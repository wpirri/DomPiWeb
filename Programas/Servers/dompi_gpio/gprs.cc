/* ************************************************************************** *
 *  Project:            uCom
 *  File:				cgsm.c
 *  Date:               1 de Agosto de 2019
 *  Author:             Walter Pirri
 *                      (walter***AT***pirri***DOT***com***DOT***ar)
 *  Company:            SIIA
 * ************************************************************************** */
/*  BG96 - tips https://en.na4.teamsupport.com/knowledgeBase/18027787
 *  AT command presedence
 *
 *     ATZ0
 *     AT
 *     ATE0
 *     AT+CGREG=0
 *     AT+CMEE=0
 *     AT&W
 *     AT+QCFG=”nwscanseq”
 *     AT+CPIN?
 *     AT+COPS=0
 *     AT+CGDCONT
 *     AT+CCGREG?
 *     AT+CFUN=1,0
 *     AT+CPIN?
 *     AT+CGREG?
 *     AT+COPS=0
 *     AT+CTZU=1
 *
 */


#include <gprs.h>
#include "main.h"

#define RCV_BUFFER_LEN 256
#define CMD_LEN 160
#define GPRS_BLINKS_ON_SEND	9
#define GPRS_BLINKS_ON_ACK 3

/* CatM + NB todo LATAM */
#define BGxx_EXTRA_INIT_COMMANDS_1
/* CatM, en Argentina con Fallback a 2G haríamos lo siguiente */
//#define BGxx_EXTRA_INIT_COMMANDS_2
/* CatM en Argentina sin Fallback */
//#define BGxx_EXTRA_INIT_COMMANDS_3


/* Definicion de estados de la maquina de estados */
typedef enum
{
    GPRS_INVALID_STATE = 0u,
	GPRS_POWER_OFF,
	GPRS_DISABLE,
	GPRS_POWER_ON,
	GPRS_KEY_PRESSED,
	GPRS_KEY_RELEASSED,
	GPRS_WAIT_START,
	GPRS_CONFIG_MODEM,
	GPRS_WAIT_MODEM_MODELL,
	GPRS_CONFIG_MODEM_BANDS,
	GPRS_CONFIG_MODEM_WAIT,
	GPRS_CONFIG_GPRS,
	GPRS_CONFIG_GPRS_WAIT,
	GPRS_CONFIG_TCPIP,
	GPRS_CONFIG_TCPIP_WAIT,
	GPRS_CONFIG_DNS,
	GPRS_INIT_OK,
	GPRS_LOOP,
	GPRS_SEND_ERROR,
	GPRS_TIMEOUT,
	GPRS_ERROR,
	GPRS_RESTART
} GPRS_STM;

typedef enum
{
	MODEM_NONE = 0u,
	MODEM_M95,
	MODEM_UGxx,
	MODEM_BGxx
} MODEM_MODELL;

typedef enum
{
	GPRS_MSGSEND_STATUS_NONE = 0u,
	GPRS_MSGSEND_STATUS_OPENING,
	GPRS_MSGSEND_STATUS_OPEN,
	GPRS_MSGSEND_STATUS_SENDING,
	GPRS_MSGSEND_STATUS_SEND,
	GPRS_MSGSEND_STATUS_WAITING_RESP,
	GPRS_MSGSEND_STATUS_RESP_OK,
	GPRS_MSGSEND_STATUS_RESP_ERROR,
	GPRS_MSGSEND_STATUS_RESP_TIMEOUT,
	GPRS_MSGSEND_STATUS_CLOSING,
	GPRS_MSGSEND_STATUS_CLOSE
} GPRS_MSGSEND_STATUS;

GPRS_STM gGprsSt;
GPRS_STM gGprsLastSt;
MODEM_MODELL gModemModell;
GPRS_MSGSEND_STATUS gGprsMsgSendStatus;
GPRS_MSGSEND_STATUS gGprsMsgSendLastStatus;

int gGprsStTo;
int gGprsMsgSendTimer;
unsigned char gGprsMsgSenRetry;
unsigned char gGprsMsgSendChannel;

unsigned char gGprsBlinkOnSend;
unsigned char gGprsBlinkOnAck;

void GprsBlinkSend( void )
{
	gGprsBlinkOnSend = GPRS_BLINKS_ON_SEND;
}

void GprsBlinkAck( void )
{
	gGprsBlinkOnAck = GPRS_BLINKS_ON_ACK;
}

typedef struct _gsm_cmd
{
    char cmd[CMD_LEN];
    char wait_for[17];
    unsigned char on_ok;
    unsigned char on_error;
    unsigned char time_out;
    unsigned char item_used;
} gsm_cmd;

typedef struct
{
	char *name;
	char *apn;
	char *user;
	char *pass;
	unsigned char auth;
}provider;

provider provider_tbl[] = {
		{0,0,0,0,0},
		{"SIERRA", "internet.swir", 0, 0, 0},
		{"MOVISTAR", "wap.gprs.unifon.com.ar", "wap", "wap", 1},
		{"PERSONAL", "datos.personal.com", "datos", "datos", 1},
//		{"CLARO", "igprs.claro.com.ar", "claroigprs", "claroigprs999", 1},
		{"CLARO", "iot.claro.com.ar", "datos", "datos", 1},
		{"CTI", "internet.ctimovil.com.ar", "gprs", "gprs", 1},
		{0,0,0,0}
};

char gGprsRcvBuffer[RCV_BUFFER_LEN];
int gGprsRcvBufferLen;
int gGprsRcvStringTo;

int gGprsRxQueue;
int gGprsTxQueue;
int gGprsMsgQueue;

int gGprsTaskDelay;
int gGprsRssiX100;
int gGprsRssiFlashCount;
char gGsmRegStatus;
int gGprsCmdTimeout;
int gGprsTcpTimeout;
int gGprsCheckStatus;
int gGprsSignalProvider;
bool gSIMReady;
bool gModemReady;
bool gGprsReady;
bool gGsmReady;
bool gAttachReady;
bool gTcpIpReady;
unsigned char gGprsNroSec;

extern UART_HandleTypeDef huart1;
extern char chUART1;

int GprsSend(const char* cmd, const char* wait_for, int onError, int onOk, int timeOut);
void CheckModemStatus( void );

void GprsProcessReceiveQueue( void );
void GprsProcessSendQueue( void );
void GprsProcessMsgQueue( void );
void GprsControlLed1();
void GprsControlLed2();

void GprsInit( void )
{
    LogMessage("SYSTEM", "Interface GPRS (Quectel)");
    /* Seteo los valores iniciales de las lineas de control */
	HAL_GPIO_WritePin(GSM_KEY_GPIO_Port, GSM_KEY_Pin, 1); /* Boton suelto */
	HAL_GPIO_WritePin(GSM_POWER_GPIO_Port, GSM_POWER_Pin, 0);  /* Modem sin alimentacion */
	/* Apago los leds */
	HAL_GPIO_WritePin(RED1_GPIO_Port, RED1_Pin, 0);
	HAL_GPIO_WritePin(GREEN1_GPIO_Port, GREEN1_Pin, 0);
	HAL_GPIO_WritePin(RED2_GPIO_Port, RED2_Pin, 0);
	HAL_GPIO_WritePin(GREEN2_GPIO_Port, GREEN2_Pin, 0);
	gGprsSt = GPRS_INVALID_STATE;
	gGprsLastSt = GPRS_INVALID_STATE;
	gModemModell = MODEM_NONE;
	gGprsMsgSendStatus = GPRS_MSGSEND_STATUS_NONE;
	gGprsMsgSendLastStatus = GPRS_MSGSEND_STATUS_NONE;
	gGprsMsgSendTimer = 0;
    gGprsRcvBufferLen = 0;
    gGprsRcvStringTo = 0;
    gGprsCmdTimeout = 0;
	gGprsStTo = 0;
	gGprsCheckStatus = 0;
	gGprsRssiFlashCount = 0;
	gGprsRssiX100 = 0;
	gGsmRegStatus = 0;
	gSIMReady = false;
	gModemReady = false;
	gGprsReady = false;
	gGsmReady = false;
	gAttachReady = false;
	gTcpIpReady = false;
	gGprsSignalProvider = 0;
	gGprsTcpTimeout = 0;
	gGprsNroSec = 1;
	gGprsMsgSenRetry = 0;
	gGprsMsgSendChannel = 0;
	gGprsBlinkOnSend = 0;
	gGprsBlinkOnAck = 0;

    gGprsRxQueue = QueueOpen(5, RCV_BUFFER_LEN);
    if(gGprsRxQueue == INVALID_QUEUE)
    {
        LogMessage("GPRS", "Error al crear cola RxQueue.");
        return;
    }

    gGprsTxQueue = QueueOpen(10, sizeof(gsm_cmd));
    if(gGprsTxQueue == INVALID_QUEUE)
    {
        LogMessage("GPRS", "Error al crear cola TxQueue.");
        return;
    }

    gGprsMsgQueue = QueueOpen(5, CMD_LEN);
    if(gGprsMsgQueue == INVALID_QUEUE)
    {
        LogMessage("GPRS", "Error al crear cola MsgQueue.");
        return;
    }

    HAL_UART_Receive_IT(&huart1, (uint8_t *)&chUART1, 1);
	gGprsSt = GPRS_POWER_OFF;
}

void GprsTasks( void )
{
	char cmd[CMD_LEN];

	/* Proceso I/O */
	if(g_uConfig.enable_gsm)
	{
		GprsProcessSendQueue();
		GprsProcessReceiveQueue();
	}

	if( !g_uConfig.enable_gsm && gGprsSt != GPRS_DISABLE) gGprsSt = GPRS_POWER_OFF;

	if(gGprsTaskDelay) return;
	if(gGprsSt != gGprsLastSt)
	{
		LogMessage("GPRS", "STM %d -> %d", gGprsLastSt, gGprsSt);
		gGprsLastSt = gGprsSt;
	}
	switch(gGprsSt)
	{
		case GPRS_INVALID_STATE:
			break;
		case GPRS_POWER_OFF:
			/* Desalimento el módulo GSM */
			LogMessage("GPRS", "POWER OFF.");
			/* Desalimento el módulo GSM */
			HAL_GPIO_WritePin(GSM_POWER_GPIO_Port, GSM_POWER_Pin, 0);  /* Modem sin alimentacion */
			HAL_GPIO_WritePin(GSM_KEY_GPIO_Port, GSM_KEY_Pin, 1); /* Boton suelto */
			QueueClean(gGprsRxQueue);
			QueueClean(gGprsTxQueue);
			gModemModell = MODEM_NONE;
			gGprsMsgSendStatus = GPRS_MSGSEND_STATUS_NONE;
			gGprsMsgSendLastStatus = GPRS_MSGSEND_STATUS_NONE;
			gGprsCmdTimeout = 0;
			gGprsRcvBufferLen = 0;
			gGprsRcvStringTo = 0;
			gGprsStTo = 0;
			gGprsCheckStatus = 0;
			gGprsRssiFlashCount = 0;
			gGprsRssiX100 = 0;
			gGsmRegStatus = 0;
			gSIMReady = false;
			gModemReady = false;
			gGprsReady = false;
			gGsmReady = false;
			gAttachReady = false;
			gTcpIpReady = false;
			gGprsSignalProvider = 0;
			gModemModell = MODEM_NONE;
			gGprsMsgSenRetry = 0;
			gGprsMsgSendChannel = 0;
			/* Espero 1,5 seg para que se estabilice la fuente */
			gGprsTaskDelay = 15;
			gGprsSt++;
			break;
		case GPRS_DISABLE:
			if(g_uConfig.enable_gsm) gGprsSt++;
			break;
		case GPRS_POWER_ON:
			/* Levanto el Reset */
			LogMessage("GPRS", "POWER ON.");
			HAL_GPIO_WritePin(GSM_POWER_GPIO_Port, GSM_POWER_Pin, 1);  /* Alimento el Modem */
			/* Espero 1 segundo para apretar el botón */
			gGprsTaskDelay = 10;
			gGprsSt++;
			break;
		case GPRS_KEY_PRESSED:
			LogMessage("GPRS", "KEY PRESSED.");
			HAL_GPIO_WritePin(GSM_KEY_GPIO_Port, GSM_KEY_Pin, 0); /* Boton presionado */
			/* Mantengo presionado el boton 1,5 seg. */
			gGprsTaskDelay = 15;
			gGprsSt++;
			break;
		case GPRS_KEY_RELEASSED:
			LogMessage("GPRS", "KEY RELEASED.");
			HAL_GPIO_WritePin(GSM_KEY_GPIO_Port, GSM_KEY_Pin, 1); /* Boton suelto */
			/* Seteo un time-out de 10 segundos para el siguiente estado */
			gGprsStTo = 100;
			gGprsSt++;
			break;
		case GPRS_WAIT_START:
			/* Espero que encienda */
			if(HAL_GPIO_ReadPin(GSM_STATUS_GPIO_Port, GSM_STATUS_Pin) == GPIO_PIN_SET)
			{
				gGprsTaskDelay = 50; /* Esperar de 2 a 3 segundos antes de enviar comandos (5 seg) */
				gGprsStTo = 0;
				gGprsSt++;
			}
			break;
		case GPRS_CONFIG_MODEM:
			if(gUcomStatus.gprs_xcom)
			{
				gGprsSt = GPRS_LOOP;
				break;
			}
			LogMessage("GPRS", "CONFIG MODEM.");
			gModemReady = false;
			gGprsReady = false;
			gGsmReady = false;
			gAttachReady = false;
			gTcpIpReady = false;
			QueueClean(gGprsTxQueue);
			gGprsCmdTimeout = 0;
			/* Para el autobaud Quectel */
			GprsSend("AT\r", "OK", GPRS_POWER_OFF, 0, 3);
			/* Reset a configuración en firmware */
			GprsSend("ATZ\r", "OK", GPRS_TIMEOUT, 0, 3);
			/* Echo off */
			GprsSend("ATE0\r", "OK", GPRS_TIMEOUT, 0, 3);
			/* No atender telefono */
			GprsSend("ATS0=0\r", "OK", GPRS_TIMEOUT, 0, 3);
			/* PII - Identificación del producto BGxx, UGxx, etc. + Revision*/
			GprsSend("ATI\r", "OK", GPRS_TIMEOUT, 0, 3);
			/* Request International Mobile Equipment Identity (IMEI) */
			GprsSend("AT+GSN\r", "OK", GPRS_TIMEOUT, 0, 3);
			/* Request TA Model Identification
			 * Quectel_M95
			 * BG96 */
			GprsSend("AT+GMM\r", "OK", GPRS_TIMEOUT, 0, 3);
			/* Seteo un time-out de 30 segundos para el siguiente estado */
			gGprsStTo = 300;
			gGprsSt++;
			break;
		case GPRS_WAIT_MODEM_MODELL:
			/* Esperando respuesta de AT+GMM */
			if(gModemModell == MODEM_NONE) break;
			gGprsSt++;
			break;
		case GPRS_CONFIG_MODEM_BANDS:
			/* Comando de inicialización rápida para BG95/96 */
			/* Hacemos un detach y reattach de la red de manera de aplicar la configuración cargada */
			GprsSend("AT+CFUN=0,0\r", "OK", GPRS_CONFIG_MODEM_BANDS, 0, 16);
			if(gModemModell == MODEM_BGxx)
			{
#ifdef BGxx_EXTRA_INIT_COMMANDS_1
				/* 2G-> todas las bandas, CatM -> B28, B5, B4, B3, B2, NB -> B28, B5, B4, B3, B2 */
				GprsSend("AT+QCFG=\"band\",F,800001E,800001E\r", "OK", GPRS_CONFIG_MODEM_BANDS, 0, 3);
				/* Modo automático LTE/2G (fallback a 2G) */
				GprsSend("AT+QCFG=\"nwscanmode\",0\r", "OK", GPRS_CONFIG_MODEM_BANDS, 0, 3);
				/* Prioridad CatM->NB->2G */
				GprsSend("AT+QCFG=\"nwscanseq\",020301\r", "OK", GPRS_CONFIG_MODEM_BANDS, 0, 3);
				/* Modo automático CatM luego NB */
				GprsSend("AT+QCFG=\"iotopmode\",2\r", "OK", GPRS_CONFIG_MODEM_BANDS, 0, 3);
#endif /* BGxx_EXTRA_INIT_COMMANDS */
#ifdef BGxx_EXTRA_INIT_COMMANDS_2
				/* 2G-> todas las bandas, CatM -> B28 y B4, NB -> no me importa */
				GprsSend("AT+QCFG=\"band\",f,8000008,0\r", "OK", GPRS_CONFIG_MODEM_BANDS, 0, 3);
				/* Modo automático LTE/2G (fallback a 2G) */
				GprsSend("AT+QCFG=\"nwscanmode\",0\r", "OK", GPRS_CONFIG_MODEM_BANDS, 0, 3);
				/* Prioridad LTE->2G */
				GprsSend("AT+QCFG=\"nwscanseq\",0201\r", "OK", GPRS_CONFIG_MODEM_BANDS, 0, 3);
				/* Modo LTE, solo CatM1 */
				GprsSend("AT+QCFG=\"iotopmode\",0\r", "OK", GPRS_CONFIG_MODEM_BANDS, 0, 3);
#endif /* BGxx_EXTRA_INIT_COMMANDS */
#ifdef BGxx_EXTRA_INIT_COMMANDS_3
				/* 2G-> don’t care, CatM -> B28 y B4, NB -> no me importa */
				GprsSend("AT+QCFG=\"band\",0,8000008,0\r", "OK", GPRS_CONFIG_MODEM_BANDS, 0, 3);
				/* Modo solo LTE */
				GprsSend("AT+QCFG=\"nwscanmode\",3\r", "OK", GPRS_CONFIG_MODEM_BANDS, 0, 3);
				/* Prioridad LTE->2G */
				GprsSend("AT+QCFG=\"nwscanseq\",02\r", "OK", GPRS_CONFIG_MODEM_BANDS, 0, 3);
				/* Modo LTE, solo CatM1 */
				GprsSend("AT+QCFG=\"iotopmode\",0\r", "OK", GPRS_CONFIG_MODEM_BANDS, 0, 3);
#endif /* BGxx_EXTRA_INIT_COMMANDS */
			}
			GprsSend("AT+CFUN=1,0\r", "OK", GPRS_CONFIG_MODEM_BANDS, 0, 16);
			/* Seteo un time-out de 30 segundos para el siguiente estado */
			gGprsStTo = 300;
			gGprsSt++;
			break;
		case GPRS_CONFIG_MODEM_WAIT:
			if(QueueCount(gGprsTxQueue) == 0)
			{
				/* AT+CPIN?    --> +CPIN: READY
				 * AT+QINISTAT --> +QINISTAT: 3 */
				if(gModemReady)
				{
					if(QueueCount(gGprsTxQueue) == 0)
					{
						gGprsStTo = 0;
						gGprsSt++;
					}
				}
				else
				{
					CheckModemStatus();
				}
			}
			break;
		case GPRS_CONFIG_GPRS:
			LogMessage("GPRS", "CONFIG GPRS.");
			gGprsReady = false;
			gGsmReady = false;
			gAttachReady = false;
			gTcpIpReady = false;
			QueueClean(gGprsTxQueue);
			gGprsCmdTimeout = 0;
			GprsSend("AT+CNMI=2,1,0,0,0\r", "OK", GPRS_CONFIG_GPRS, 0, 3);
			GprsSend("AT+CMGF=1\r", "OK", GPRS_CONFIG_GPRS, 0, 3);
			GprsSend("AT+CREG=0\r", "OK", GPRS_CONFIG_GPRS, 0, 3);
			/* Seteo un time-out de 4 minutos para el siguiente estado */
			gGprsStTo = 2400;
			gGprsSt++;
			break;
		case GPRS_CONFIG_GPRS_WAIT:
			if(QueueCount(gGprsTxQueue) == 0)
			{
				/*	--> AT+CSQ --> +CSQ, 30,0
					--> AT+CREG --> +CREG: 0,1

					--> AT+COPS? --> +COPS 0,0,"Movistar"
					--> AT+CGREG? --> +CGREG: 0,1

					--> AT+CGATT? --> +CGATT: 1 */
				if(gAttachReady)
				{
					gGprsStTo = 0;
					gGprsSt++;
				}
				else
				{
					CheckModemStatus();
				}
			}
			break;
		case GPRS_CONFIG_TCPIP:
			LogMessage("GPRS", "CONFIG TCP/IP.");
			gTcpIpReady = false;
			QueueClean(gGprsTxQueue);
			gGprsCmdTimeout = 0;
			/* Uso defaults para APN? */
			if( g_uConfig.gprs_apn_auto != 0 && gGprsSignalProvider > 0 )
			{
				strcpy(g_uConfig.gprs_apn, provider_tbl[gGprsSignalProvider].apn);
				strcpy(g_uConfig.gprs_user, provider_tbl[gGprsSignalProvider].user);
				strcpy(g_uConfig.gprs_pass, provider_tbl[gGprsSignalProvider].pass);
				g_uConfig.gprs_auth = provider_tbl[gGprsSignalProvider].auth;
			}
			switch(gModemModell)
			{
				case MODEM_M95:
					GprsSend("AT+QIMUX=1\r", "OK", GPRS_CONFIG_TCPIP, 0, 3);
					sprintf(cmd, "AT+QICSGP=1,\"%s\",\"%s\",\"%s\"\r", g_uConfig.gprs_apn, g_uConfig.gprs_user, g_uConfig.gprs_pass);
					GprsSend(cmd, "OK", GPRS_CONFIG_TCPIP, 0, 3);
					break;
				case MODEM_UGxx:
				case MODEM_BGxx:
					/* Configure Context */
					/* AT+QICSGP=<contextID>,<context_type>,[<apn>[,<username>,<password>)[,<authentication>]]] */
					if( strlen(g_uConfig.gprs_user) && strlen(g_uConfig.gprs_pass))
					{
						sprintf(cmd, "AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",%d\r",
								g_uConfig.gprs_apn, g_uConfig.gprs_user, g_uConfig.gprs_pass, g_uConfig.gprs_auth);
					}
					else
					{
						sprintf(cmd, "AT+QICSGP=1,1,\"%s\"\r", g_uConfig.gprs_apn);
					}
					GprsSend(cmd, "OK", GPRS_ERROR, 0, 3);
					/* Activate PDP Context #1 */
					GprsSend("AT+QIACT=1\r", "OK", GPRS_ERROR, 0, 300);
					break;
				default:
					gGprsSt = GPRS_ERROR;
					break;
			}
			gGprsStTo = 3500; /* Seteo un time-out */
			gGprsSt++;
			break;
		case GPRS_CONFIG_TCPIP_WAIT:
			if(QueueCount(gGprsTxQueue) == 0)
			{
				/*
				 * M95:  AT+QILOCIP
				 * UG96: AT+QIACT?
				 */
				if(gTcpIpReady)
				{
					gGprsStTo = 0;
					gGprsSt++;
				}
				else
				{
					CheckModemStatus();
				}
			}
			break;
		case GPRS_CONFIG_DNS:
			if(g_uConfig.gprs_dns1[0] != '0')
			{
				if(g_uConfig.gprs_dns2[0] != '0')
				{
					sprintf(cmd, "AT+QIDNSCFG=1,\"%s\",\"%s\"\r", g_uConfig.gprs_dns1, g_uConfig.gprs_dns2);
				}
				else
				{
					sprintf(cmd, "AT+QIDNSCFG=1,\"%s\"\r", g_uConfig.gprs_dns1);
				}
				GprsSend(cmd, "OK", 0, 0, 3);
			}
			gGprsSt++;
			break;
		case GPRS_INIT_OK:
			LogMessage("GPRS", "INIT Ok.");
			gGprsSt++;
			break;
		case GPRS_LOOP:
			CheckModemStatus();
			GprsProcessMsgQueue();
			if( !gUcomStatus.gprs_xcom)
			{
				if( !gModemReady)
				{
					gGprsSt = GPRS_ERROR;
					break;
				}
				else if( !gGprsReady || !gGsmReady || !gAttachReady)
				{
					gUcomStatus.gprs_host1 = false;
					gUcomStatus.gprs_host2 = false;
					gGprsSt = GPRS_CONFIG_GPRS;
					break;
				}
				else if( !gTcpIpReady)
				{
					gGprsSt = GPRS_ERROR;
					break;
				}
			}
			break;
		case GPRS_SEND_ERROR:
			QueueClean(gGprsTxQueue);
			gGprsCmdTimeout = 0;
			gGprsMsgSendStatus = GPRS_MSGSEND_STATUS_CLOSING;
			gGprsSt = GPRS_LOOP;
			break;
		case GPRS_TIMEOUT:
		case GPRS_ERROR:
			QueueClean(gGprsTxQueue);
			gGprsCmdTimeout = 0;
			GprsSend("AT+QIDEACT=1\r", "OK", 0, 0, 15);
			gGprsTaskDelay = 50;		/* No vuelvo a la maquina de estados por 5 seg */
			gUcomStatus.gprs_host1 = false;
			gUcomStatus.gprs_host2 = false;
		default:
			gGprsSt = GPRS_POWER_OFF;
			break;
	}
}

/* Se llama 10 veces x segundo */
void GprsTimer( void )
{
    gsm_cmd *c;

    if(gGprsTaskDelay) gGprsTaskDelay--;

    /* Controlo time-out entre caracteres */
	if(gGprsRcvStringTo)
	{
		gGprsRcvStringTo--;
		if(gGprsRcvStringTo == 0)
		{
			/* Fin del mensaje por time-out - Mando lo que haya */
			gGprsRcvBuffer[gGprsRcvBufferLen] = 0;
			if(gGprsRcvBufferLen)
			{
	            /* Encolo la linea recibida */
	            if( !QueueAdd(gGprsRxQueue, &gGprsRcvBuffer[0]))
	            {
	                LogMessage("GPRS", "ERROR: GprsRx Queue overflow");
	            }
			}
			/* reseteo el buffer */
			gGprsRcvBufferLen = 0;
			return;
		}
	}
    /* Controlo time-out de comandos al modulo */
	if(gGprsCmdTimeout)
	{
		gGprsCmdTimeout--;
		if(gGprsCmdTimeout == 0)
		{
			/* Se produjo un time-out de un comando al gsm */
		    if(QueueView(gGprsTxQueue, (void**)&c))
		    {
				LogMessage("GPRS", "TIMEOUT esperando [%s] de [%s].", c->wait_for, c->cmd);
				if(c->on_error != GPRS_INVALID_STATE)
				{
					gGprsSt = c->on_error;
				}
				//else
				//{
				//	gGprsSt = GPRS_ERROR;
				//}
				QueueDel(gGprsTxQueue);
		    }
		}
	}
    /* Controlo time-out de la máquina de estados */
	if(gGprsStTo)
	{
		gGprsStTo--;
		if(gGprsStTo == 0)
		{
    		gGprsSt = GPRS_TIMEOUT;
		}
	}
	/* Consulta de estado del modem */
	if( gGprsCheckStatus )
	{
		gGprsCheckStatus--;
	}
	/* ================ Direccion IP =================== */
	if(gGprsTcpTimeout)
	{
		gGprsTcpTimeout--;
		if(gGprsTcpTimeout == 0)
		{
			/* Time out esperando dirección IP - Respuesta de AT+QILOCIP o AT+QIACT? */
			LogMessage("GPRS","TIMEOUT Esperando direccion IP");
	    	gUcomStatus.gprs_ip[0] = 0;
	    	gTcpIpReady = false;
		}
	}

	/* ================ Mensaje UDP =================== */
	if(gGprsMsgSendTimer)
	{
		gGprsMsgSendTimer--;
		if(gGprsMsgSendTimer == 0)
		{
			LogMessage("GPRS","TIMEOUT Esperando respuesta UDP");
			gGprsMsgSendStatus = GPRS_MSGSEND_STATUS_RESP_TIMEOUT;
			if(gGprsMsgSendChannel == 0)
			{
				gUcomStatus.gprs_host1 = false;
			}
			else
			{
				gUcomStatus.gprs_host2 = false;
			}
		}
	}

	GprsControlLed1();
	GprsControlLed2();
}

int GprsReady( void )
{
	return (gTcpIpReady)?1:0;
}

int GprsAlarma(const char* event, int part, int zone, const char *t)
{
	CONTAC_UDP msg;
	char cmd[CMD_LEN];
	char d[20];

	if( !GprsReady() ) return (-1);
	if(gUcomStatus.gprs_xcom) return 0;

	if( !strcmp(event, "0000"))
	{
		strcpy(msg.cid.msg_typ, "00");
		strcpy(msg.cid.event_cde, "0000");
		msg.cid.part_num = 0;
		msg.cid.zone_num = 0;
		strcpy(msg.cid.acct_num, "0000");
	}
	else
	{
		strcpy(msg.cid.msg_typ, "18");
		strncpy(msg.cid.event_cde, event, 4);
		msg.cid.part_num = part;
		msg.cid.zone_num = zone;
		strncpy(msg.cid.acct_num, g_uConfig.user_account, 4);
	}
	strncpy(msg.acct, g_uConfig.user_account, 4);

    /* 01234567890123456789 */
    /* dd/mm/yyyy-hh:mm:ss */
    do
    {
    	strncpy(d, t, 19);
        if( d[2] == '/') d[2] = 0;
        else break;
        if( d[5] == '/') d[5] = 0;
        else break;
        if( d[10] == '-') d[10] = 0;
        else break;
        if( d[13] == ':') d[13] = 0;
        else break;
        if( d[16] == ':') d[16] = 0;
        else break;
        d[19] = 0;

        msg.date.dd = atoi(d);
        msg.date.mm = atoi(&d[3]);
        msg.date.yyyy = atoi(&d[6]);
        msg.date.HH = atoi(&d[11]);
        msg.date.MM = atoi(&d[14]);
        msg.date.SS = atoi(&d[17]);
    } while (0);

	msg.frec = g_uConfig.gprs_report;
	strcpy(msg.ver, VERSION_SISTEMA);
	strcpy(msg.id, g_uConfig.gprs_sys_id);
	msg.rssi = gUcomStatus.gprs_rssi;
	msg.seq = gGprsNroSec++;
	if(gGprsNroSec > 99) gGprsNroSec = 1;

	UDP_MessageMK(&msg, cmd);
	/* Al mensaje le agrego el CTRL+Z */
	cmd[strlen(cmd)+1] = 0;
	cmd[strlen(cmd)] = 0x1A;

	QueueAdd(gGprsMsgQueue, cmd);
	return 0;
}

void GprsGetTime( void )
{
	if( !GprsReady() ) return;
	if(gUcomStatus.gprs_xcom) return;

	GprsSend("AT+QNTP=1,\"pool.ntp.org\"\r", 0, 0, 0, 5);
}

void GprsGetRSSI( void )
{
	if( !GprsReady() ) return;
	if(gUcomStatus.gprs_xcom) return;

	GprsSend("AT+CSQ\r", "OK", 0, 0, 5);
}

/* Llamada desde Interrupción */
void GprsReceive( void )
{
	gGprsRcvStringTo = 15;
	if(chUART1 == 0x0A || chUART1 == 0x0D)
	{
		/* Fin del mensaje */
		gGprsRcvBuffer[gGprsRcvBufferLen] = 0;
		if(gGprsRcvBufferLen)
		{
            /* Encolo la linea recibida */
            if( !QueueAdd(gGprsRxQueue, &gGprsRcvBuffer[0]))
            {
                LogMessage("GPRS", "ERROR: Gprs RxQueue overflow.");
            }
		}
		/* reseteo el buffer */
		gGprsRcvBufferLen = 0;
		gGprsRcvStringTo = 0;
	}
	else if(chUART1 > 0 && chUART1 < 127)
	{
		gGprsRcvBuffer[gGprsRcvBufferLen] = chUART1;
		if(gGprsRcvBufferLen < (RCV_BUFFER_LEN-1)) gGprsRcvBufferLen++;
	}
}

void CheckModemStatus( void )
{
	if(gGprsCheckStatus) return;
	
	if(gTcpIpReady)
	{
		gGprsCheckStatus = 300; /* De vuelta en 300 seg. */
	}
	else
	{
		gGprsCheckStatus = 50; /* De vuelta en 5 seg. */
	}

	/* Nada de check si estoy en modo comando */
	if(gUcomStatus.gprs_xcom) return;

	/* Si estoy en medio de un envío no hago consultas */
	if(gGprsMsgSendStatus >= GPRS_MSGSEND_STATUS_OPEN )
	{
		return;
	}

	/* Consulta sobre socket abierto */
	if(gGprsMsgSendStatus == GPRS_MSGSEND_STATUS_OPENING)
	{
		GprsSend("AT+QISTATE=1,0\r", "OK", 0, 0, 5);
	}

	if(gAttachReady)
	{
		/* Get Local IP */
		gGprsTcpTimeout = 35;	/* Si en 3,5 seg no me devuelve la IP invalido la que tengo */
		switch(gModemModell)
		{
		case MODEM_M95:
			GprsSend("AT+QILOCIP\r", 0, 0, 0, 5);
			break;
		case MODEM_UGxx:
		case MODEM_BGxx:
			GprsSend("AT+QIACT?\r", "OK", 0, 0, 5);
			break;
		default:
			break;
		}
		/* Signal Strength */
		GprsSend("AT+CSQ\r", "OK", GPRS_ERROR, 0, 5);
	}
	else if(gGprsReady)
	{
		/* Signal Strength */
		GprsSend("AT+CSQ\r", "OK", 0, 0, 5);
		/* GPRS network attached? */
		GprsSend("AT+CGATT?\r", "OK", GPRS_CONFIG_GPRS, 0, 5);
	}
	else if(gGsmReady)
	{
		/* Signal Strength */
		GprsSend("AT+CSQ\r", "OK", 0, 0, 5);
		/* Operador */
		GprsSend("AT+COPS?\r", "OK", GPRS_CONFIG_GPRS, 0, 5);
		/* Query register state of GPRS network */
		GprsSend("AT+CGREG?\r", "OK", GPRS_CONFIG_GPRS, 0, 5);
	}
	else if(gModemReady)
	{
		/* Signal Strength */
		GprsSend("AT+CSQ\r", "OK", 0, 0, 5);
		/* Query register state of GSM network */
		GprsSend("AT+CREG?\r", "OK", 0, 0, 5);
	}
	else if(gSIMReady)
	{
		/* Get SIM init Status */
		GprsSend("AT+QINISTAT\r", "OK", 0, 0, 20);
	}
	else
	{
		/* Get SIM Status */
		GprsSend("AT+CPIN?\r", "OK", 0, 0, 5);
	}
}

int GprsSend(const char* cmd, const char* wait_for, int onError, int onOk, int timeOut)
{
    gsm_cmd c;
    
    /* Cargo el comando en la cola */
    strcpy(c.cmd, cmd);
    if(wait_for)
    {
        strcpy(c.wait_for, wait_for);
    }
    else c.wait_for[0] = '\0';
    c.on_ok = onOk;
    c.on_error = onError;
    if(timeOut > 255) timeOut = 255;
    c.time_out = timeOut;
    c.item_used = 0;

    if( !QueueAdd(gGprsTxQueue, &c))
    {
        LogMessage("GPRS", "ERROR: GprsTx Queue overflow.");
        return (-1);
    }
    LogMessage("GPRS", "Encolando [%s].", cmd);
    return 0;	/* Ok */
}

void GprsProcessReceiveQueue( void )
{
    gsm_cmd *c;
    char str[RCV_BUFFER_LEN];
    char lastCmd[20];
    char *p;
    bool waiting;
    RESPONSE_UDP cid;

    lastCmd[0] = 0;

    /* Me fijo si tengo algo en la cola */
    if( !QueueGet(gGprsRxQueue, &str)) return;
    /* Me fijo si estaba esperando algo */
    waiting = QueueView(gGprsTxQueue, (void**)&c);
    if(waiting && c->item_used)
	{
    	strncpy(lastCmd, c->cmd, 19);
        lastCmd[19] = 0;
        if(strchr(lastCmd, ':')) *(strchr(lastCmd, ':')) = 0;
        else if(strchr(lastCmd, '?')) *(strchr(lastCmd, '?')) = 0;
        else if(strchr(lastCmd, '=')) *(strchr(lastCmd, '=')) = 0;
        else if(strchr(lastCmd, '\r')) *(strchr(lastCmd, '\r')) = 0;

		/* Controlo si lo recibido empieza con lo esperado */
		if( !memcmp(str, c->wait_for, strlen(c->wait_for)))
		{
			LogMessage("GPRS", "Recibido [%s] de [%s].", str, c->cmd);
			/* Manejo el cambio de estado */
			if(c->on_ok != GPRS_INVALID_STATE)
			{
				gGprsSt = c->on_ok;
			}
			gGprsCmdTimeout = 0;
			lastCmd[0] = 0;
			/* Avanzo la cola */
			QueueDel(gGprsTxQueue);
			//return;
		}
		else
		{
			LogMessage("GPRS", "Recibido [%s], esperando [%s].", str, c->wait_for);
		}
    }
    else
    {
    	LogMessage("GPRS", "Recibido [%s]", str);
    }

    /* No proceso respuestas si estoy en modo comando */
    if(gUcomStatus.gprs_xcom)
    {
    	CONOutput(str);
    	return;
    }

    if( !memcmp("+CPIN:", str, 6))
    {
    	/* +CPIN: READY */
    	if(str[7] == 'R')
    	{
    		gSIMReady = true;
    	}
    }
    else if( !memcmp("+QINISTAT:", str, 10))
    {
		/* 	[+QINISTAT: 3]
		 *  0 No initialization
			1 Ready to execute AT command
			2 Phonebook has finished initialization
			3 SMS has finished initialization
		*/
        if(str[11] == '3') gModemReady = true;
        else gModemReady = false;
    }
    else if( !memcmp("+CSQ:", str, 5))
    {
        /* Respuesta de AT+CSQ [+CSQ: 10,3]
         * viene con info de nivel de senal - +CSQ: 12,0
    	                                            ^  ^
													|  \- Bit error Rate
													\--- Signal Streng Indicator
		*/
        /*
            Wording 	Blocks 	Percent/RSSI/DB
            Excellent 	[][][][][] 	       100  31 >-51
                                            97 	30 	-53
                                            94 	29 	-55
                                            90 	28 	-57
                                            87 	27 	-59
                                            84 	26 	-61
            Good            [][][][]	    81 	25 	-63
                                            77 	24 	-65
                                            74 	23 	-67
                                            71 	22 	-69
                                            68 	21 	-71
                                            65 	20 	-73
            Fair            [][][]		    61 	19 	-75
                                            58 	18 	-77
                                            55 	17 	-79
                                            52 	16 	-81
                                            48 	15 	-83
                                            45 	14 	-85
            Poor            [][]		    42 	13 	-87
                                            39 	12 	-89
                                            35 	11 	-91
                                            32 	10 	-93
                                            29 	9 	-95
                                            26 	8 	-97
            Very Poor 	[] 		            23 	7 	-99
                                            19 	6 	-101
                                            16 	5 	-103
                                            13 	4 	-105
                                            10 	3 	-107
                                            6 	2 	-109
            No Signal 			            3 	1 	-111
                                            0 	0 	<-113
         */

    	if(strchr(str, ','))
    	{
    		*(strchr(str, ',')) = 0;
        	gUcomStatus.gprs_rssi = atoi(&str[6]);
        	if(gUcomStatus.gprs_rssi > 31) gUcomStatus.gprs_rssi = 0;
        	gGprsRssiX100 = gUcomStatus.gprs_rssi * 100 / 31;
    	}
    }
    else if( !memcmp("+CREG:", str, 6))
    {
        /*  GSM Network Registration status
         *  [+CREG: 0,3]
			+CREG: <stat>
			+CREG: <n>,<stat>
            <stat>
            0   not registered, MT is not currently searching a new
                operator to register to
            1   registered, home network
            2   not registered, but MT is currently searching a new
                operator to register to
            3   registration denied
            4   unknown
            5   registered, roaming
            <lac>
            string type(string should be included in quotation marks);
            two byte location area code in hexadecimal format
            < ci >
            string type(string should be included in quotation marks);
            two byte cell ID in hexadecimal format
         */
		if(strchr(str, ','))
		{
			if(str[9] != gGsmRegStatus)
			{
				gGsmRegStatus = str[9];
				if(gGsmRegStatus == '0')
					LogMessage("GPRS", "Registration Status: Not registered.");
				else if(gGsmRegStatus == '1')
					LogMessage("GPRS", "Registration Status: Registered.");
				else if(gGsmRegStatus == '2')
					LogMessage("GPRS", "Registration Status: Searching...");
				else if(gGsmRegStatus == '3')
					LogMessage("GPRS", "Registration Status: Denied.");
				else if(gGsmRegStatus == '4')
					LogMessage("GPRS", "Registration Status: Unknown.");
				else if(gGsmRegStatus == '5')
					LogMessage("GPRS", "Registration Status: Roaming.");
			}
		}
		else
		{
			if(str[7] != gGsmRegStatus)
			{
				gGsmRegStatus = str[7];
				if(gGsmRegStatus == '0')
					LogMessage("GPRS", "Registration Status: Not registered.");
				else if(gGsmRegStatus == '1')
					LogMessage("GPRS", "Registration Status: Registered.");
				else if(gGsmRegStatus == '2')
					LogMessage("GPRS", "Registration Status: Searching...");
				else if(gGsmRegStatus == '3')
					LogMessage("GPRS", "Registration Status: Denied.");
				else if(gGsmRegStatus == '4')
					LogMessage("GPRS", "Registration Status: Unknown.");
				else if(gGsmRegStatus == '5')
					LogMessage("GPRS", "Registration Status: Roaming.");
			}
		}
		
		if(gGsmRegStatus == '1' || gGsmRegStatus == '5') gGsmReady = true;
		else gGsmReady = false;
    }
    else if( !memcmp("+CGREG:", str, 7))
    {
		/* GPRS Network registration status +CGREG: 0,1 */
    	if(str[10] == '1')	gGprsReady = true;
    	else	gGprsReady = false;

    }
    else if( !memcmp("+COPS:", str, 6))
    {
		/* Operador +COPS: 0,0,"CHINA MOBILE"]
		 * 			+COPS: 0,0,"Movistar",6*/
    	if(strchr(&str[12], '\"'))
    	{
    		*(strchr(&str[12], '\"')) = 0;
    		ToUp(gUcomStatus.gprs_provider, &str[12]);
    		for(gGprsSignalProvider = 1; provider_tbl[gGprsSignalProvider].name; gGprsSignalProvider++)
    		{
    			if( strstr(gUcomStatus.gprs_provider, provider_tbl[gGprsSignalProvider].name))
    			{
    				break;
    			}
    		}
    		if( !provider_tbl[gGprsSignalProvider].name) gGprsSignalProvider = 0;
    		if(gGprsSignalProvider)
    		{
    			LogMessage("GPRS", "PROVIDER: %s", gUcomStatus.gprs_provider);
    		}
    		else
    		{
    			if(g_uConfig.gprs_apn_auto)
    			{
        			LogMessage("GPRS", "PROVIDER no identificado");
    				gGprsSt = GPRS_ERROR;

    			}
    		}
    	}
    }
    else if( !memcmp("+CGATT:", str, 7))
    {
		/* GPRS attach +CGATT: 1 */
    	if(str[8] == '1')	gAttachReady = true;
    	else	gAttachReady = false;
    }
    else if( !memcmp("+QIACT:", str, 7))
    {
		/* GPRS attach +QIACT:20,<context_state>,<context_type>,<IP_address>
		 * +QIACT: 1,1,1,"10.173.62.119" */
    	p = strchr(str, '\"');
    	if(p)
    	{
        	p++;
    		if(strchr(p, '\"')) *(strchr(p, '\"')) = 0;
    		if(strcmp(p, gUcomStatus.gprs_ip) != 0)
    		{
    			strcpy(gUcomStatus.gprs_ip, p);
    			LogMessage("GPRS","GPRS IP: %s", gUcomStatus.gprs_ip);
    		}
    	}
    	if( !gTcpIpReady)
    	{
    		gGprsAliveTimer = 0;
    	}
    	gTcpIpReady = true;
    	gGprsTcpTimeout = 0;
    }
    /* Me fijo si es un mensaje no solicitado */
    else if( !memcmp("+CMTI:", str, 6))
    {
        /* Aviso de mensaje entrante - viene con numero de mensaje - +CMTI: "SM",1 */


    }
    else if( !memcmp("+CMGR:", str, 6) || !memcmp("+CMGL:", str, 6))
    {
        /* Lectura de mensaje entrante - viene con el numero del originante */
        /*
            +CMGR: "REC UNREAD","+8613918186089", ,"02/01/30,20:40:31+00"
            This is a test
        */
        /* Listado de mensajes entrantes - vienen con el numero del originante y el de orden adelante */
        /*  +CMGL:1,"RECREAD","+8613918186089",, "02/01/30,20:40:31+00"
            This is a test

         */
    }
    else if( !memcmp("+QISTATE:", str, 9))
    {
    	/* [+QISTATE: 0,"UDP","186.134.168.60",8030,2773,3,1,0,1,"uart1"] */
    	/* Espero la configrmacion del OPEN */
    	if(strstr(str, "uart1"))
    	{
			gGprsMsgSendStatus = GPRS_MSGSEND_STATUS_OPEN;
		}
    }
    else if( !memcmp("+QNTP:", str, 6))
    {
    	/* +QNTP: <err>,<time>
    	 * +QNTP: 0,"2020/04/11,02:02:36-12"
    	 */
    	if(str[7] == '0')
    	{
        	RTC_TimeTypeDef RtcTime;
        	RTC_DateTypeDef RtcDate;

			RtcDate.Year = GetInt(&str[10], 4) % 100;
			RtcDate.Month = GetInt(&str[15], 2);
			RtcDate.Date = GetInt(&str[18], 2);
			RtcDate.WeekDay = 0;

			if(RtcDate.Date > 0 && RtcDate.Month > 0 && RtcDate.Year > 0)
			{
				RtcTime.Hours = GetInt(&str[21], 2);
				RtcTime.Minutes = GetInt(&str[24], 2);
				RtcTime.Seconds = GetInt(&str[27], 2);
				/* En el Modem BG96 la la hora viene en UTC */
				if(gModemModell == MODEM_BGxx)
				{
					/* Tengo que restar 3 Hs. */
					if(RtcTime.Hours >= 3)
					{
						RtcTime.Hours -= 3;
					}
					else
					{
						RtcTime.Hours = 24 - 3 + RtcTime.Hours;
						/* Le tengo que restar 1 día */
						if(RtcDate.Date > 1)
						{
							RtcDate.Date -= 1;
						}
						else
						{
							/* Tengo que ver de cuantos dias es el mes anterior */
							if(RtcDate.Month == 12 || RtcDate.Month == 5 || RtcDate.Month == 7 || RtcDate.Month == 10)
							{
								/* Meses de 30 días (11, 4, 6, 9) */
								RtcDate.Date = 30;
							}
							else if(RtcDate.Month == 3)
							{
								/* Meses de 28/29 días (2) */
								if( !(RtcDate.Year % 4) && ( !((RtcDate.Year % 100 )?false:true) || ((RtcDate.Year % 400)?false:true) ) )
								{
									/* Febreros de 29 días */
									RtcDate.Date = 29;
								}
								else
								{
									/* Febreros de 28 días */
									RtcDate.Date = 28;
								}

							}
							else
							{
								RtcDate.Date = 31;
							}
							/* Le tengo que restar un mes */
							if(RtcDate.Month > 1)
							{
								RtcDate.Month -= 1;
							}
							else
							{
								RtcDate.Month = 12;
								/* Le tengo que restar 1 año */
								RtcDate.Year -= 1;
							}
						}
					}
				}
				HAL_RTC_SetTime(&hrtc, &RtcTime, RTC_FORMAT_BIN);
				HAL_RTC_SetDate(&hrtc, &RtcDate, RTC_FORMAT_BIN);
				gUComGetTimeCount = 172800; /* 24 Hs. */

				/* Ahora tengo una fecha y hora válida */
				gUcomStatus.system_clock = true;
			}
    	}
    }
    else if( !memcmp("+QIND", str, 5))
    {


    }
	else if( !memcmp("$D,", str, 3))
    {
    	/* Receive data in direct push access mode
    	 * [+QIURC: "recv",<connectID>,<currentrecvlength>,<remoteIP>,<remote_port><CR><lF><data>]
    	 *
    	 */
		/* Proceso el mensaje recibido */
    	if(UDP_MessageGT(str, &cid) != 100)
    	{
			/* Si todo ok */
			gGprsMsgSendStatus = GPRS_MSGSEND_STATUS_RESP_OK;
    	}
    }
    else if( !memcmp("SEND OK", str, 7))
    {
    	gGprsMsgSendStatus = GPRS_MSGSEND_STATUS_SEND;
    }
    else
    {
    	/* No vino nada conocido, busco alguna de las cadenas que identifican modem */
    	if( !strcmp(lastCmd, "AT+GSN") )
    	{
    		/* viene el IMEI */
    		strncpy(g_uConfig.gprs_imei, str, 17);
    		g_uConfig.gprs_imei[17] = 0;
    		LogMessage("GPRS", "IMEI: %s", g_uConfig.gprs_imei);
    		/* Me fijo si tengo que cargar el ID del sistema o ya estaba cargado */
    		if( strncmp(g_uConfig.gprs_sys_id, &g_uConfig.gprs_imei[strlen(g_uConfig.gprs_imei)-7], 7))
    		{
				strncpy(g_uConfig.gprs_sys_id, &g_uConfig.gprs_imei[strlen(g_uConfig.gprs_imei)-7], 7);
				g_uConfig.gprs_sys_id[7] = 0;
				gConfigWiFiCount = 10;
    		}
    	}
    	else if( !strcmp(lastCmd, "AT+GMM") )
    	{
    		gModemModell = MODEM_NONE;
    		if( !strcmp(str, "Quectel_M95"))
    		{
    			gModemModell = MODEM_M95;
    		}
    		else if( !memcmp(str, "UG", 2))
    		{
    			gModemModell = MODEM_UGxx;
    		}
    		else if( !memcmp(str, "BG", 2))
    		{
    			gModemModell = MODEM_BGxx;
    		}

    		if(gModemModell == MODEM_NONE)
    		{
    			LogMessage("GPRS", "MODEM: DESCONOCIDO");
				gGprsSt = GPRS_ERROR;
    		}
    		else
    		{
    			LogMessage("GPRS", "MODEM: %s", str);
    		}
    	}
    	else if( !strcmp(lastCmd, "AT+QILOCIP") )
    	{
        	strcpy(gUcomStatus.gprs_ip, str);
        	gTcpIpReady = true;
        	gGprsTcpTimeout = 0;
    	}

    }

}

void GprsProcessSendQueue( void )
{
    gsm_cmd *c;

	if(gGprsCmdTimeout || gGprsSt == GPRS_ERROR) return;
    if(QueueView(gGprsTxQueue, (void**)&c))
    {
		LogMessage("GPRS", "Enviando [%s].", c->cmd);
    	HAL_UART_Transmit(&huart1, (uint8_t *)c->cmd, strlen(c->cmd), strlen(c->cmd));
    	if(c->time_out == 0 || c->wait_for[0] == 0)
    	{
    		QueueDel(gGprsTxQueue);
    		gGprsCmdTimeout = 0;
    	}
    	else
    	{
    		c->item_used = 1;
    		gGprsCmdTimeout = c->time_out * 10;
    	}
    	HAL_UART_Receive_IT(&huart1, (uint8_t *)&chUART1, 1);
    }
}

void GprsSendConfig( void )
{
	/* Si estoy en modo comando lo ignoro */
	if( !gUcomStatus.gprs_xcom)
		gGprsSt = GPRS_RESTART;
}

void GprsProcessMsgQueue( void )
{
	char cmd[CMD_LEN];
	char *msg;

	/* Si está en modo comando me olvido de los mensajes UDP */
	if( gUcomStatus.gprs_xcom ) return;

	if(gGprsMsgSendLastStatus != gGprsMsgSendStatus)
	{
		LogMessage("GPRS", "MSG-STM: %d -> %d", gGprsMsgSendLastStatus, gGprsMsgSendStatus);
		gGprsMsgSendLastStatus = gGprsMsgSendStatus;
	}
	switch(gGprsMsgSendStatus)
	{
	case GPRS_MSGSEND_STATUS_NONE:
		if(QueueView(gGprsMsgQueue, (void**)&msg))
		{
			GprsBlinkSend();
			/*
			 * AT+QIOPEN=<contextID>,<connectID>,<service_type>,<IP_address>/<domain_name>,<remote_port>[,<local_port>,<access_mode>]
			 */
			sprintf(cmd, "AT+QIOPEN=1,0,\"UDP\",\"%s\",%d,%d,1\r",
					(gGprsMsgSendChannel)?g_uConfig.gprs_host2:g_uConfig.gprs_host1,
					(gGprsMsgSendChannel)?g_uConfig.gprs_host2_port:g_uConfig.gprs_host1_port,
					g_uConfig.gprs_local_port);
			GprsSend(cmd, "OK", GPRS_SEND_ERROR, 0, 10);
			gGprsMsgSendTimer = 150;	/* Espero 15 seg la respuesta */
			gGprsMsgSendStatus++;
		}
		break;
	case GPRS_MSGSEND_STATUS_OPENING:
		/* Esperando recibir un [+QISTATE: ...] con datos de la conexion */
		break;
	case GPRS_MSGSEND_STATUS_OPEN:
		if(QueueView(gGprsMsgQueue, (void**)&msg))
		{
			GprsSend("AT+QISEND=0\r", ">", GPRS_SEND_ERROR, 0, 10);
			GprsSend(msg, "SEND OK", GPRS_SEND_ERROR, 0, 10);
			gGprsMsgSendStatus++;
		}
		else
		{
			gGprsMsgSendStatus = GPRS_MSGSEND_STATUS_CLOSING;
		}
		break;
	case GPRS_MSGSEND_STATUS_SENDING:
		/* Esperando [SEND OK] */
		break;
	case GPRS_MSGSEND_STATUS_SEND:
		gGprsMsgSendStatus++;
		break;
	case GPRS_MSGSEND_STATUS_WAITING_RESP:
		break;
	case GPRS_MSGSEND_STATUS_RESP_OK:
		LogMessage("GPRS", "ACK de Mensaje por GPRS");
		GprsBlinkAck();
		if(gGprsMsgSendChannel == 0)
		{
			gUcomStatus.gprs_host1 = true;
		}
		else
		{
			gUcomStatus.gprs_host2 = true;
		}
		gGprsMsgSendStatus = GPRS_MSGSEND_STATUS_CLOSING;
		gGprsMsgSendTimer = 0;
		gGprsMsgSendChannel = 0;
		gGprsMsgSenRetry = 0;
		QueueDel(gGprsMsgQueue);
		NotiAlarmGprsOk();
		break;
	case GPRS_MSGSEND_STATUS_RESP_ERROR:
	case GPRS_MSGSEND_STATUS_RESP_TIMEOUT:
		gGprsMsgSendStatus = GPRS_MSGSEND_STATUS_CLOSING;
		gGprsMsgSendTimer = 0;
		gGprsMsgSenRetry++;
		if(gGprsMsgSenRetry >= GPRS_SEND_RETRY)
		{
			if(gGprsMsgSendChannel == 0)
			{
				gUcomStatus.gprs_host1 = false;
			}
			else
			{
				gUcomStatus.gprs_host2 = false;
			}

			if(gGprsMsgSendChannel == 0)
			{
				gGprsMsgSendChannel = 1;
				gGprsMsgSenRetry = 0;
			}
			else
			{
				gGprsMsgSendChannel = 0;
				gGprsMsgSenRetry = 0;
				NotiAlarmGprsError();
			}
		}
		break;
	case GPRS_MSGSEND_STATUS_CLOSING:
		/* AT+QICLOSE= */
		GprsSend("AT+QICLOSE=0\r", "OK", 0, 0, 5);
		gGprsMsgSendStatus++;
		break;
	case GPRS_MSGSEND_STATUS_CLOSE:
		gGprsMsgSendStatus = GPRS_MSGSEND_STATUS_NONE;
		break;
	}

}

void GprsControlLed1()
{
	/* ================ LED 1 =================== */
	if( !gUcomStatus.info_leds)
	{
		HAL_GPIO_WritePin(RED1_GPIO_Port, RED1_Pin, 0);
		HAL_GPIO_WritePin(GREEN1_GPIO_Port, GREEN1_Pin, 0);
	}
	/* Fijo en rojo al principio */
	else if(gGprsSt <= GPRS_WAIT_START)
	{
		HAL_GPIO_WritePin(RED1_GPIO_Port, RED1_Pin, 1);
		HAL_GPIO_WritePin(GREEN1_GPIO_Port, GREEN1_Pin, 0);
	}
	/* Rojo titilando hasta que enganche GPRS */
	else if(gGprsSt <= GPRS_CONFIG_GPRS_WAIT)
	{
		HAL_GPIO_TogglePin(RED1_GPIO_Port, RED1_Pin);
		HAL_GPIO_WritePin(GREEN1_GPIO_Port, GREEN1_Pin, 0);
	}
	/* Verde una vez que engancha GPRS */
	else if(gGprsSt < GPRS_LOOP)
	{
		HAL_GPIO_WritePin(RED1_GPIO_Port, RED1_Pin, 0);
		HAL_GPIO_WritePin(GREEN1_GPIO_Port, GREEN1_Pin, 1);
	}
	/* Indicador de nivel de señal Rojo sobre verde fijo cuando esta listo */
	else if(gGprsSt == GPRS_LOOP)
	{
		if( ( (gGprsRssiX100 >= 10) && (gGprsRssiFlashCount == GPRS_SIGNAL_DELTA) ) ||
			( (gGprsRssiX100 >= 30) && (gGprsRssiFlashCount == (GPRS_SIGNAL_DELTA*2)) ) ||
			( (gGprsRssiX100 >= 50) && (gGprsRssiFlashCount == (GPRS_SIGNAL_DELTA*3)) ) ||
			( (gGprsRssiX100 >= 70) && (gGprsRssiFlashCount == (GPRS_SIGNAL_DELTA*4)) ) ||
			( (gGprsRssiX100 >= 90) && (gGprsRssiFlashCount == (GPRS_SIGNAL_DELTA*5)) )  )
		{
			HAL_GPIO_WritePin(RED1_GPIO_Port, RED1_Pin, 1);
			HAL_GPIO_WritePin(GREEN1_GPIO_Port, GREEN1_Pin, 0);
		}
		else
		{
			HAL_GPIO_WritePin(RED1_GPIO_Port, RED1_Pin, 0);
			HAL_GPIO_WritePin(GREEN1_GPIO_Port, GREEN1_Pin, 1);
		}
		gGprsRssiFlashCount++;
		if(gGprsRssiFlashCount > GPRS_SIGNAL_PERIOD)
		{
			gGprsRssiFlashCount = 1;
		}
	}
	else
	{
		HAL_GPIO_WritePin(RED1_GPIO_Port, RED1_Pin, 1);
		HAL_GPIO_WritePin(GREEN1_GPIO_Port, GREEN1_Pin, 0);
	}
}

void GprsControlLed2()
{
	/* ================ LED 2 =================== */
	if( !gUcomStatus.info_leds)
	{
		HAL_GPIO_WritePin(RED2_GPIO_Port, RED2_Pin, 0);
		HAL_GPIO_WritePin(GREEN2_GPIO_Port, GREEN2_Pin, 0);
	}
	/* Fijo en rojo al principio */
	else if( !gSIMReady)
	{
		HAL_GPIO_WritePin(RED2_GPIO_Port, RED2_Pin, 1);
		HAL_GPIO_WritePin(GREEN2_GPIO_Port, GREEN2_Pin, 0);
	}
	/* Rojo titilando hasta que detecte la SIM */
	else if( !gGprsReady)
	{
		HAL_GPIO_TogglePin(RED2_GPIO_Port, RED2_Pin);
		HAL_GPIO_WritePin(GREEN2_GPIO_Port, GREEN2_Pin, 0);
	}
	/* Verde una vez que engancha GPRS */
	else if( !gTcpIpReady)
	{
		HAL_GPIO_WritePin(RED2_GPIO_Port, RED2_Pin, 0);
		HAL_GPIO_WritePin(GREEN2_GPIO_Port, GREEN2_Pin, 1);
	}
	/* Indicador de Transmisión y ACKo */
	else /* gTcpIpReady = true */
	{
		if(gGprsBlinkOnAck)
		{
			gGprsBlinkOnSend = 0;
			if( !(gGprsBlinkOnSend & 0x01))
			{
				HAL_GPIO_WritePin(RED2_GPIO_Port, RED2_Pin, 1);
				HAL_GPIO_WritePin(GREEN2_GPIO_Port, GREEN2_Pin, 1);
			}
			else
			{
				HAL_GPIO_WritePin(RED2_GPIO_Port, RED2_Pin, 0);
				HAL_GPIO_WritePin(GREEN2_GPIO_Port, GREEN2_Pin, 1);
			}
			gGprsBlinkOnAck--;
		}
		else if(gGprsBlinkOnSend)
		{
			if( !(gGprsBlinkOnSend & 0x01))
			{
				HAL_GPIO_WritePin(RED2_GPIO_Port, RED2_Pin, 1);
				HAL_GPIO_WritePin(GREEN2_GPIO_Port, GREEN2_Pin, 0);
			}
			else
			{
				HAL_GPIO_WritePin(RED2_GPIO_Port, RED2_Pin, 0);
				HAL_GPIO_WritePin(GREEN2_GPIO_Port, GREEN2_Pin, 1);
			}
			gGprsBlinkOnSend--;
		}
		else
		{
			HAL_GPIO_WritePin(RED2_GPIO_Port, RED2_Pin, 0);
			HAL_GPIO_WritePin(GREEN2_GPIO_Port, GREEN2_Pin, 1);
		}
	}
}

void GprsCommand( const char *cmd )
{
    if( !memcmp("EXIT", cmd, 4))
    {
    	/* recupero los valores guardados */
    	g_uConfig.enable_wifi = gUcomStatus.orig_enable_wifi;
    	g_uConfig.enable_gsm = gUcomStatus.orig_enable_gprs;
    	g_uConfig.enable_bus = gUcomStatus.orig_enable_bus;
    	g_uConfig.trace = gUcomStatus.orig_trace;
    	/* Activo los comandos directos */
    	gUcomStatus.gprs_xcom = false;
    	/* Reinicio el modem */
    	gGprsSt = GPRS_ERROR;
    }
    else if( !memcmp("RESET", cmd, 5))
    {
    	/* Reinicio el modem */
    	gGprsSt = GPRS_ERROR;
    }
    else
    {
    	/* Mando los comandos directo al modem */
    	GprsSend(cmd, "OK", 0, 0, 5);
    }
}
