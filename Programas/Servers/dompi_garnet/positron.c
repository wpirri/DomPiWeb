/* ************************************************************************** *
 *  Project:            uCom
 *  File:				positron.c
 *  Date:               20 de Agosto de 2019
 *  Author:             Walter Pirri
 *                      (walter***AT***pirri***DOT***com***DOT***ar)
 *  Company:            SIIA
 * ************************************************************************** */
/*
 *  Mensajeria con central de alarmas Positron
 */
#include "main.h"
#ifdef POSITRON
#include "positron.h"

#define NO_DEBUG_BUS

#define PSTN_BASE_ID		0x02
//#define PSTN_TERMINAL_ID	0x03
#define PSTN_BROADCAST_ID	0xFF

#define PSTN_START_MESSAGE 0x28
#define PSTN_END_MESSAGE   0x29
#define PSTN_ESC_CHAR      0x27
#define PSTN_ESC_OPER      0x40

#define PSTN_LIVE_CONTROL_TIME 1200

#define PSTM_MESSAGE_BUFFER_LEN 256

#define IS_BUS_OK ((gc485LiveCount != 0)?true:false)

/* Definicion de estados de la maquina de estados */
typedef enum
{
    C485_INVALID_STATE = 0u,
	C485_RESET,
	C485_INIT,
	C485_LOOP,
	C485_CONFIG,
	C485_CONFIG_OK,
	C485_CONFIG_ERROR,
	C485_TIMEOUT,
	C485_ERROR
} C485_STM;

C485_STM gC485St;
C485_STM gC845LastSt;
unsigned char gC485NroSec;
unsigned char gC485NroVer;
unsigned char gC485NroTer;

typedef struct
{
	int len;
	char msg[PSTM_MESSAGE_BUFFER_LEN];
} c485_msg;

typedef struct _c485_cmd
{
	uint8_t dsta;		// Dest address - len segun adcr
	uint8_t srca;		// Src address - len segun adcr
	uint8_t sqn;			// Sequence number
	uint8_t vrs;			// Protocol version
	uint8_t len;			// App data len (len typ + len data)
	uint8_t data[8];		// Data area (typ included)
	uint8_t on_ok;
	uint8_t on_error;
	uint8_t time_out;
	uint8_t* result;
    uint16_t result_len;
} c485_cmd;

int gC458RcvTo;
int gC485ConfigIdx;

int gC485RxQueue;
int gC485TxQueue;
int gC485RespQueue;

int gC485TaskDelay;
int gC485SendDelay;
int gC485CmdTimeout;
unsigned char gC485ConfigErrCount;
int gc485LiveCount;
bool gc485Live;

bool gPSTNStart;
bool gPSTNEnd;
bool gPSTNEsc;

c485_msg gPSTNMessageIn;

extern UART_HandleTypeDef huart3;
extern char chUART3;


#define ACK_LEN 5
const char msg_ack[] = {0x00,0x00,0x01};
#define BUS_LOCK_LEN 5
const char bus_lock[] = {0x02,0x00,0x0F,0x01,0x00};
#define CMD_KEY_LEN 3
const char key_cancell[] = {0x14,0x00,0x43};
const char key_programa[] = {0x14,0x00,0x50};
const char key_enter[] = {0x14,0x00,0x45};
const char key_num[][3] = {	{0x14,0x00,0x30},
							{0x14,0x00,0x31},
							{0x14,0x00,0x32},
							{0x14,0x00,0x33},
							{0x14,0x00,0x34},
							{0x14,0x00,0x35},
							{0x14,0x00,0x36},
							{0x14,0x00,0x37},
							{0x14,0x00,0x38},
							{0x14,0x00,0x39}};

int PSTNSend(const char* msg, int msg_len, int onError, int onOk, int timeOut, char** result, unsigned int result_len);
int PSTNSendResp(const char* msg, int msg_len, unsigned char sec);
void PSTNProcessReceiveQueue( void );
void PSTNProcessSendQueue( void );
void PSTNSendConfig( unsigned int cfgid );

void PSTNInit( void )
{
    /* Seteo los valores iniciales de las lineas de control */
    LogMessage("SYSTEM", "Interface Panel (POSITRON)");
	gC485St = C485_INVALID_STATE;
	gC845LastSt = C485_INVALID_STATE;
    //gC485RcvBufferLen = 0;
    gC458RcvTo = 0;
    gC485CmdTimeout = 0;
	gC485NroSec = 1;
	gC485SendDelay = 0;
	gC485ConfigIdx = 0;
	gC485ConfigErrCount = 0;
	gPSTNStart = false;
	gPSTNEnd = false;
	gPSTNEsc = false;
	gPSTNMessageIn.len = 0;
	gC485NroTer = 0;
	gc485LiveCount = 0;
	gc485Live = false;

    gC485RxQueue = QueueOpen(15, sizeof(c485_msg));
    if(gC485RxQueue == INVALID_QUEUE)
    {
        LogMessage("PSTN", "Error al crear Rx Queue.");
        return;
    }
    gC485TxQueue = QueueOpen(20, sizeof(c485_cmd));
    if(gC485TxQueue == INVALID_QUEUE)
    {
        LogMessage("PSTN", "Error al crear Tx Queue.");
        return;
    }
    gC485RespQueue = QueueOpen(10, sizeof(c485_cmd));
    if(gC485RespQueue == INVALID_QUEUE)
    {
        LogMessage("PSTN", "Error al crear Resp Queue.");
        return;
    }
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&chUART3, 1);
	gC485St = C485_RESET;
}

/* Se llama dentro del loop del main */
void PSTNTasks( void )
{
	if(gC485TaskDelay) return;

	if(gC485St != gC845LastSt)
	{
        LogMessage("PSTN", "STM %d -> %d", gC845LastSt, gC485St);
		gC845LastSt = gC485St;
	}

	switch(gC485St)
	{
		case C485_INVALID_STATE:
			break;
		case C485_RESET:
	        LogMessage("PSTN", "RESET.");
		    gC485CmdTimeout = 0;
			gC485TaskDelay = 5; /* seg. antes de seguir */
		    gC458RcvTo = 0;
			gC485St++;
			break;
		case C485_INIT:
			gC485ConfigIdx = 0;
			if(g_uConfig.enable_bus)
			{
		    	QueueClean(gC485RxQueue);
		    	QueueClean(gC485TxQueue);
				/* Reseteo el buffer y todos los flags de mensaje */
		    	gPSTNStart = false;
		    	gPSTNEnd = false;
		    	gPSTNEsc = false;
		    	gPSTNMessageIn.len = 0;
		    	/* */
				gC485St++;
			}
			break;
		case C485_LOOP:
			if( !g_uConfig.enable_bus)
			{
				gC485St = C485_INIT;
				break;
			}
			PSTNProcessSendQueue();
			PSTNProcessReceiveQueue();
			break;
		case C485_CONFIG:
			gC485St = C485_LOOP;
			/* Cuento la cantidad de veces que reinicia la configuración */
			if(gC485ConfigIdx == 0 && gC485ConfigErrCount > 0)
			{
				gC485ConfigErrCount--;
				if(gC485ConfigErrCount == 0)
				{
					LogMessage("PSTN", "Config idx: %i ERROR con %d intentos.",
							gC485ConfigIdx, PANEL_CONFIG_RETRY);
					gUComGetTimeCount = 120; /* Vuelve a pedir la hora en 1 min */
					break;
				}
			}
			PSTNSendConfig(gC485ConfigIdx);
			break;
		case C485_CONFIG_OK:
			LogMessage("PSTN", "Config idx: %i OK en %d intentos.",
					gC485ConfigIdx, PANEL_CONFIG_RETRY - gC485ConfigErrCount + 1);
			gC485ConfigIdx++;
			gC485ConfigErrCount = PANEL_CONFIG_RETRY;
			gC485St = C485_CONFIG;
			break;
		case C485_CONFIG_ERROR:
			QueueClean(gC485TxQueue);
		    gC485CmdTimeout = 0;
			gC485St = C485_CONFIG;
			break;
		case C485_TIMEOUT:
		case C485_ERROR:
		default:
			gC485St = C485_RESET;
			break;
	}
}

/* Se llama 2 veces por segundo */
void PSTNTimer( void )
{
    c485_cmd *c;

    if(gC485TaskDelay) gC485TaskDelay--;

    /* Controlo time-out entre caracteres */
	if(gC458RcvTo)
	{
		gC458RcvTo--;
		if(gC458RcvTo == 0)
		{
			/* Reseteo el buffer y todos los flags de mensaje */
			gPSTNStart = false;
			gPSTNEnd = false;
			gPSTNEsc = false;
			gPSTNMessageIn.len = 0;
			return;
		}
	}

    /* Controlo time-out de comandos al modulo */
	if(gC485CmdTimeout)
	{
		gC485CmdTimeout--;
		if(gC485CmdTimeout == 0)
		{
			/* Se produjo un time-out de un comando al 485 */
		    if(QueueView(gC485TxQueue, (void**)&c))
		    {
		    	LogMessage("PSTN", "Time Out de %02X %02X %02X sec %02X",
		    			c->data[0], c->data[1], c->data[2], c->sqn);
				/* Si no estaba esperando nada en especial manejo el cambio de estado */
				if(c->on_error != C485_INVALID_STATE)
				{
					gC485St = c->on_error;
				}
				QueueDel(gC485TxQueue);
		    }
		}
	}

	/* Tiempo para detectar que el panel esta vivo */
	if(gc485LiveCount)
	{
		gc485LiveCount--;
		if(gc485LiveCount == 0)
		{
			/* El panel esta muerto */
			LogMessage("PSTN", "BUS POSITRON DESCONECTADO");
			gc485Live = false;
			NotiPanel(0);
		}

	}

	/* Delay para la cola de despacho de mensajes */
	if(gC485SendDelay) gC485SendDelay--;
}

void PSTNReceive( void )
{
	gC458RcvTo = 3;
	//c485_cmd c;		 TODO: SOLO PARA PRUEBAS

	if(gPSTNStart) /* Caracter START ya recibido */
	{
		if(chUART3 == PSTN_END_MESSAGE) /* Me fijo si es un END */
		{
			/* Ya tengo el mensaje completo en el buffer, los dos últimos bytes son el checksum */
			if(gPSTNMessageIn.len)
			{
				/* Encolo la linea recibida */
				if( !QueueAdd(gC485RxQueue, &gPSTNMessageIn))
				{
					LogMessage("PSTN", "ERROR: RX Queue overflow.");
				}
			}
			/* Reseteo el buffer y todos los flags de mensaje */
			gPSTNStart = false;
			gPSTNEnd = false;
			gPSTNEsc = false;
			gPSTNMessageIn.len = 0;
		}
		else
		{
			if(gPSTNEsc) /* Caracter de escape recibido antes */
			{
				gPSTNEsc = false;
				chUART3 ^= PSTN_ESC_OPER;
			}
			else if(chUART3 == PSTN_ESC_CHAR)
			{
				gPSTNEsc = true;
			}

			if( chUART3 != PSTN_ESC_CHAR && gPSTNMessageIn.len < (PSTM_MESSAGE_BUFFER_LEN-1) )
			{
				gPSTNMessageIn.msg[gPSTNMessageIn.len++] = chUART3;
				gPSTNMessageIn.msg[gPSTNMessageIn.len] = 0;
			}
		}
	}
	else if(chUART3 == PSTN_START_MESSAGE) /* Me fijo si es un START */
	{
		gPSTNStart = true;
	}
}

void PSTNSendConfig( unsigned int cfgid )
{
	int i;
	RTC_DateTypeDef RtcDate;
	RTC_TimeTypeDef RtcTime;

	switch(cfgid)
	{
		case 0:
			LogMessage("PSTN", "Set Panel Date.");
			/* Bus LOCK */
			// Comentado, falta la parte del manual dond está este comando y no lo puedo desactivar (Tabla 7)
			//if(PSTNSend(bus_lock, BUS_LOCK_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			/* Tecla CANCEL */
			if(PSTNSend(key_cancell, CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			/* Fecha */
			HAL_RTC_GetDate(&hrtc, &RtcDate, RTC_FORMAT_BIN);
			/* Prog */
			if(PSTNSend(key_programa, CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			/* Clave */
			for(i = 0; i < 17;i++)
			{
				if(g_uConfig.key_1[i] < '0' || g_uConfig.key_1[i] > '9') break;
				if(PSTNSend(key_num[(int)(g_uConfig.key_1[i]-'0')], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			}
			/* Cmd 006 */
			if(PSTNSend(key_num[0], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			if(PSTNSend(key_num[0], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			if(PSTNSend(key_num[6], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			/*Fecha*/
			if(PSTNSend(key_num[RtcDate.Date/10], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			if(PSTNSend(key_num[RtcDate.Date%10], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			if(PSTNSend(key_num[RtcDate.Month/10], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			if(PSTNSend(key_num[RtcDate.Month%10], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			if(PSTNSend(key_num[(RtcDate.Year%100)/10], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			if(PSTNSend(key_num[RtcDate.Year%10], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			/* Enter */
			if(PSTNSend(key_enter, CMD_KEY_LEN, C485_CONFIG_ERROR, C485_CONFIG_OK, 3, NULL, 0) != 0) return;
			break;
		case 1:
			LogMessage("PSTN", "Set Panel Time.");
			/* Bus LOCK */
			//if(PSTNSend(bus_lock, BUS_LOCK_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			/* Tecla CANCEL */
			if(PSTNSend(key_cancell, CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			/* Hora */
			HAL_RTC_GetTime(&hrtc, &RtcTime, RTC_FORMAT_BIN);
			/* Prog */
			if(PSTNSend(key_programa, CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			/* Clave */
			for(i = 0; i < 17;i++)
			{
				if(g_uConfig.key_1[i] < '0' || g_uConfig.key_1[i] > '9') break;
				if(PSTNSend(key_num[(int)(g_uConfig.key_1[i]-'0')], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			}
			/* Cmd 007 */
			if(PSTNSend(key_num[0], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			if(PSTNSend(key_num[0], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			if(PSTNSend(key_num[7], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			/*Hora*/
			if(PSTNSend(key_num[RtcTime.Hours/10], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			if(PSTNSend(key_num[RtcTime.Hours%10], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			if(PSTNSend(key_num[RtcTime.Minutes/10], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			if(PSTNSend(key_num[RtcTime.Minutes%10], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			if(PSTNSend(key_num[RtcTime.Seconds/10], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			if(PSTNSend(key_num[RtcTime.Seconds%10], CMD_KEY_LEN, C485_CONFIG_ERROR, 0, 3, NULL, 0) != 0) return;
			/* Enter */
			if(PSTNSend(key_enter, CMD_KEY_LEN, C485_CONFIG_ERROR, C485_CONFIG_OK, 3, NULL, 0) != 0) return;
			break;

		default:
			break;
	}
}

void PSTNConfig( void )
{
	if(IS_BUS_OK)
	{
		gC485ConfigIdx = 0;
		gC485ConfigErrCount = PANEL_CONFIG_RETRY;
		gC485St = C485_CONFIG;
	}
}

int PSTNSend(const char* msg, int msg_len, int onError, int onOk, int timeOut, char** result, unsigned int result_len)
{
    c485_cmd c;

    /* Cargo el comando en la cola */
    memset(&c, 0, sizeof(c485_cmd));
    c.dsta = PSTN_BASE_ID;	/* Destino busmaster */
    c.srca = gC485NroTer;  /* Origen el primer teclado TODO: registrar como teclado nuevo */
    c.sqn = gC485NroSec++;
    c.vrs = gC485NroVer;
    memcpy(c.data, msg, msg_len);
    c.len = msg_len;
    c.on_ok = onOk;
    c.on_error = onError;
    c.time_out = timeOut * 2;
    if(result)
    {
        c.result = (unsigned char*)*result;
        c.result_len = result_len;
    }
    else
    {
        c.result = NULL;
        c.result_len = 0;
    }
    //LogMessage("PSTN", "Encolando: %02X->%02X len: %02X msg:%02X %02X %02X.", c.srca, c.dsta, c.len, c.data[0], c.data[1], c.data[2]);
    if( !QueueAdd(gC485TxQueue, &c))
    {
        LogMessage("PSTN", "ERROR: TX Queue overflow.");
        return (-1);
    }
    return 0;	/* Ok */
}

int PSTNSendResp(const char* msg, int msg_len, unsigned char sec)
{
    c485_cmd c;

    /* Cargo el comando en la cola */
    memset(&c, 0, sizeof(c485_cmd));
    c.dsta = PSTN_BASE_ID;	/* Destino busmaster */
    c.srca = gC485NroTer;  /* Origen el primer teclado TODO: registrar como teclado nuevo */
    c.sqn = sec;
    c.vrs = gC485NroVer;
    memcpy(c.data, msg, msg_len);
    c.len = msg_len;
    c.on_ok = 0;
    c.on_error = 0;
    c.time_out = 0;
    //LogMessage("PSTN", "Encolando: %02X->%02X len: %02X msg:%02X %02X %02X.", c.srca, c.dsta, c.len, c.data[0], c.data[1], c.data[2]);
    if( !QueueAdd(gC485RespQueue, &c))
    {
        LogMessage("PSTN", "ERROR: Resp Queue Overflow.");
        return (-1);
    }
    return 0;	/* Ok */
}

/*
 * Address		1 = Dispositivo sin registrar
 * 				2 = Central
 * 				3..FE = Perifericos (teclados, modulos de expansion)
 *
 */
void PSTNProcessReceiveQueue( void )
{
    c485_msg m;
    c485_cmd *c;

    int addr_len;
    int ver = 0;
    int data_len;
    char data[256];
    int chk_sum;
    int loc_sum;
    int pass_len;
    char pass[16];
    int typ = 0;
    char *msg;
    long src = 0;
    long dst = 0;
    int sec = 0;

    /* Me fijo si tengo algo en la cola */
    if( !QueueGet(gC485RxQueue, &m)) return;
    /* Cuando recibo algo avito enviar en seguida */
    gC485SendDelay = 2;
    /* copio el checksum del mensaje */
    chk_sum = (m.msg[m.len-2]*256)+m.msg[m.len-1];
    loc_sum = crc_ccitt_ffff((unsigned char*)m.msg, m.len-2);
    /* Parseo mensaje POSITRON */
    addr_len = m.msg[0];
    if(addr_len == 0)
    {
    	dst = m.msg[1];
    	src = m.msg[2];
        sec = m.msg[3];
        ver = m.msg[4];
        data_len = m.msg[5];
        if(data_len) memcpy(data, &m.msg[6], data_len);
    }
    else if(addr_len == 1)
    {
    	dst = (m.msg[1]*256)+m.msg[2];
    	src = (m.msg[3]*256)+m.msg[4];
        sec = m.msg[5];
        ver = m.msg[6];
        data_len = m.msg[7];
        if(data_len) memcpy(data, &m.msg[8], data_len);
    }
    else if(addr_len == 2)
    {
    	dst = (m.msg[1]*16777216)+(m.msg[2]*65536)+(m.msg[3]*256)+m.msg[4];
    	src = (m.msg[5]*16777216)+(m.msg[6]*65536)+(m.msg[7]*256)+m.msg[8];
        sec = m.msg[9];
        ver = m.msg[10];
        data_len = m.msg[11];
        if(data_len) memcpy(data, &m.msg[12], data_len);
    }

    pass_len = data[0];
    if(pass_len)
    {
    	memcpy(pass, &data[1], min(pass_len, 16));
    }
    typ = data[pass_len+1];
    msg = &data[pass_len+2];
    /* No controlo el checksum en ACK y respuestas */
    if(typ != 0x00 && typ != 0x15 && typ != 0x16)
    {
		if(chk_sum != loc_sum)
		{
			LogMessage("PSTN", "BUS CHKSUM ERROR");
			return;
		}
    }
    /* Busco un mensaje del teclado para hacerme pasar por él */
    if(gC485NroTer == 0 && src > PSTN_BASE_ID && src < PSTN_BROADCAST_ID)
    {
    	gC485NroTer = src;
    }
    /* Si el mensaje viene del teclado que estoy emulando me
     * copio el nro de secuencia y la version para no pisar
     * secuencias si me estoy haciendo pasar por él */
    if(gC485NroTer && gC485NroTer == src)
    {
    	gC485NroSec = sec - 127;
    	gC485NroVer = ver;
    }
    /* Controlo que cada tanto reciba algo de la base */
    if(src > 0 && src < PSTN_BROADCAST_ID)
    {
		/* El bus está vivo */
		gc485LiveCount = PSTN_LIVE_CONTROL_TIME;
		if(gc485Live == false)
		{
			LogMessage("PSTN", "BUS POSITRON CONECTADO");
			gc485Live = true;
			NotiPanel(1);
		}
    }

    switch(typ)
    {
		case 0x00:	/* ACK */
#ifdef DEBUG_BUS
			LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X ACK", src, dst, sec, typ);
#endif
		    /* Verifico si en la cola de salida tengo que eliminar un mensaje */
		    if(QueueView(gC485TxQueue, (void**)&c))
		    {
		    	if( c->dsta == src && c->srca == dst && c->sqn == sec &&
		    			/* Los que se contestan con ACK */
		    		(c->data[0] == 0x01 || c->data[0] == 0x07 || c->data[0] == 0x0a || c->data[0] == 0x0d) )
		    	{
		    		LogMessage("PSTN", "ACK de %02X", sec);
		    		gC485CmdTimeout = 0;
		    		if(c->on_ok != 0) gC485St = c->on_ok;
		    		QueueDel(gC485TxQueue);
		    	}
		    }
		    break;
		case 0x01:	/* Log Message (Resp: Ack) */
			msg[2+msg[1]] = 0;
#ifdef DEBUG_BUS
			LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X MSG=%s", src, dst, sec, typ, &msg[2]);
#endif
		    break;
		case 0x02:	/* Command (Resp: Ack) */
#ifdef DEBUG_BUS
			LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X CMD=0x%02X%02X %s",
					src, dst, sec, typ, msg[0], msg[1], (!msg[0] && !msg[1])?"Keypad Alive.":".");
#endif
#ifdef __NO_CONTESTO__
			if(dst == gC485NroTer)
			{
				/* Al mensaje 0x02 el teclado contesta ACK 0x00 (si es al revez la central contesta 0x16) */
				/* Parece que me hablan a mi... */
				PSTNSendResp(msg_ack, ACK_LEN, sec);
			}
#endif
			break;
		case 0x03:	/* Parameter Write */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X PARAM Write sect:%02X reg:%02X%02X param:%02X len:%02X",
		    		   src, dst, sec, typ, msg[0], msg[1], msg[2], msg[3], msg[4]);
#endif
			break;
		case 0x04:	/* Parameter Write Resp */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X PARAM Write RESP sect:%02X reg:%02X%02X param:%02X err:%02X%02X",
		    		   src, dst, sec, typ, msg[0], msg[1], msg[2], msg[3], msg[4], msg[5]);
#endif
			break;
		case 0x05:	/* Parameter Read */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X PARAM Read idx:%02X%02X,%02X%02X",
		    		   src, dst, sec, typ, msg[0], msg[1], msg[2], msg[3]);
#endif
			break;
		case 0x06:	/* Parameter Read Resp */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X PARAM Read RESP: idx:%02X%02X,%02X%02X len:%02X",
		    		   src, dst, sec, typ, msg[0], msg[1], msg[2], msg[3], msg[4]);
#endif
			break;
		case 0x07:	/* Event Report (Resp: Ack) */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X EVENT:%02X%02X %02d/%02d/%02d %02d:%02d:%02d code-idx:%02X qual:%02X code:%02X%02X part:%02X zone:%02X%02X",
		    		src, dst, sec, typ, msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7], msg[8], msg[9], msg[10], msg[11], msg[12], msg[13], msg[14]);
#endif
			break;
		case 0x08:	/* RF Signal Message */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X RF MSG: len:%02X", src, dst, sec, typ, msg[0]);
#endif
			break;
		case 0x09:	/* Config mode commands */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X CONFIG: len:%02X", src, dst, sec, typ, msg[0]);
#endif
			break;
		case 0x0a:	/* System status (Resp: Ack) */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X STATUS: sub:%02X %02X%02X", src, dst, sec, typ, msg[0], msg[2], msg[3]);
#endif
#ifdef __NO_CONTESTO__
			if(dst == gC485NroTer)
			{
				/* Al mensaje 0x0a se contesta con ACK */
				/* Parece que me hablan a mi... */
				PSTNSendResp(msg_ack, ACK_LEN, sec);
			}
#endif
			if(msg[0] == 0x01)	/* Partition status */
		    {
		    	/*	msg1 -> data_len
		    	 *  msg2 -> Part1
		    	 *  msg3 -> Part2
		    	 *  bit0 -> enable
		    	 *  bit1 -> armed	*
		    	 *  bit2 -> ready
		    	 *  bit3 -> transition
		    	 *  bit4 -> stay	*
		    	 */
		    	/* Part1 */
		    	if( msg[2] & 0x10 )
		    	{
		    		gUcomStatus.part[0] = NOTI_PART_STATUS_STAY;
		    	}
		    	else if( msg[2] & 0x02 )
		    	{
		    		gUcomStatus.part[0] = NOTI_PART_STATUS_ARMED;
		    	}
		    	else if( msg[2] & 0x04 || msg[2] & 0x01 )
		    	{
		    		gUcomStatus.part[0] = NOTI_PART_STATUS_READY;
		    	}
		    	else
		    	{
		    		gUcomStatus.part[0] = NOTI_PART_STATUS_DISABLE;
		    	}
		    	/* Part 2 */
		    	if( (msg[3] & 0x02) || (msg[3] & 0x10) )
		    	{
		    		gUcomStatus.part[1] = 3;
		    	}
		    	else if( (msg[3] & 0x04) )
		    	{
		    		gUcomStatus.part[1] = 2;
		    	}
		    	else if( (msg[3] & 0x01) )
		    	{
		    		gUcomStatus.part[1] = 1;
		    	}
		    	else
		    	{
		    		gUcomStatus.part[1] = 0;
		    	}
		    }
		    else if(msg[0] == 0x04)	/* system troubles */
		    {
		    	/*
		    	 * msg[2] -> MSB
		    	 * 		0
		    	 * 		1
		    	 * 		2 - Power Fail
		    	 * 		3
		    	 * 		4
		    	 * 		5
		    	 * 		6
		    	 * 		7
		    	 * msg[3] -> LSB
		    	 * 		0
		    	 * 		1 - Bat Low
		    	 * 		2
		    	 * 		3
		    	 * 		4
		    	 * 		5
		    	 * 		6
		    	 * 		7
		    	 */
		    	if(msg[2] & 0x04)
		    	{
		    		/* Power Fail */
		    		gUcomStatus.power_fail = true;
		    	}
		    	if(msg[3] & 0x02)
		    	{
		    		/* Low Battery */
		    		gUcomStatus.low_bat = true;
		    	}

		    }
		    else if(msg[0] == 0x06)	/* Memory violation status, no hay informacion de particion */
		    {
		    	if(msg[2] != 0x00)
		    	{
					gUcomStatus.alarm_zone = 1;	/* TODO: Colocar el numero corecto de zona */
		    	}
		    }
			break;
		case 0x0b:	/* Test Command */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X TEST", src, dst, sec, typ);
#endif
			break;
		case 0x0c:	/* Test Command Resp */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X TEST RESP", src, dst, sec, typ);
#endif
			break;
		case 0x0d:	/* Display Message (Resp: Ack) */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X DISPLAY", src, dst, sec, typ);
#endif
			break;
		case 0x0e:	/* Config & Control (Slave devices) */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X CONFIG SLAVE", src, dst, sec, typ);
#endif
			break;
		case 0x0f:	/* Config & Control Resp (Slave devices) */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X CONFIG SLAVE RESP", src, dst, sec, typ);
#endif
			break;
		case 0x10:	/* Remote programming Status */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X REMOTE PROG STAT", src, dst, sec, typ);
#endif
			break;
		case 0x11:	/* Request Event */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X RQ EVENT: 0x%02X%02X", src, dst, sec, typ, msg[0], msg[1]);
#endif
			break;
		case 0x12:	/* Zone Values (Resp: Ack) */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X Z:%02X V:%02X", src, dst, sec, typ, msg[0], msg[1]);
#endif
			break;
		case 0x13:	/* Request Last Event Log */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X REQUEST LAST EVENT", src, dst, sec, typ);
#endif
			break;
		case 0x14:	/* Key Pressed */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X KEY PRESSED: 0x%02X%02X", src, dst, sec, typ, msg[0], msg[1]);
#endif
		    /*
				* Tecla 0
												      vv vv
				[C485] ->[12][00 02 03 35 00 04 00 14 00 30 EF F7 ]
				[C485] [3->2](53) KEY PRESSED: 0x00
				[C485] ->[12][00 03 02 35 00 04 00 15 01 30 47 06 ]
				[C485] [2->3](53) KEY PRESSED RESP

				Tecla 1
												      vv vv
				[C485] ->[12][00 02 03 25 00 04 00 14 00 31 C8 AD ]
				[C485] [3->2](37) KEY PRESSED: 0x00
				[C485] ->[12][00 03 02 25 00 04 00 15 01 31 60 5C ]
				[C485] [2->3](37) KEY PRESSED RESP

				Tecla 2
												      vv vv
				[C485] ->[12][00 02 03 1D 00 04 00 14 00 32 32 EE ]
				[C485] [3->2](29) KEY PRESSED: 0x00
				[C485] ->[12][00 03 02 1D 00 04 00 15 01 32 9A 1F ]
				[C485] [2->3](29) KEY PRESSED RESP

				Tecla 3
												      vv vv
				[C485] ->[12][00 02 03 2B 00 04 00 14 00 33 DA 67 ]
				[C485] [3->2](43) KEY PRESSED: 0x00
				[C485] ->[12][00 03 02 2B 00 04 00 15 01 33 72 96 ]
				[C485] [2->3](43) KEY PRESSED RESP
		     * */
		    if(msg[0] == 0x00 && msg[1] == 0x6C)
		    {
		    	/* Panic */
		    	gUcomStatus.panic = true;

		    }
		    else if(msg[0] == 0x00 && msg[1] == 0x66)
		    {
		    	/* Fire */
		    	gUcomStatus.fire = true;
		    }
		    else if(msg[0] == 0x00 && msg[1] == 0x6D)
		    {
		    	/* Emergency */
		    	gUcomStatus.emergency = true;
		    }
			break;
		case 0x15:	/* Key Pressed Resp */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X KEY PRESSED RESP", src, dst, sec, typ);
#endif
		    /* Verifico si en la cola de salida tengo que eliminar un mensaje de Key Press */
		    if(QueueView(gC485TxQueue, (void**)&c))
		    {
		    	if( c->dsta == src && c->srca == dst && c->sqn == sec && c->data[0] == 0x14 )
		    	{
		    		LogMessage("PSTN", "KEY ok %02X", sec);
		    		gC485CmdTimeout = 0;
		    		if(c->on_ok != 0) gC485St = c->on_ok;
		    		QueueDel(gC485TxQueue);
		    	}
		    }

			break;
		case 0x16:	/* Command Response */
#ifdef DEBUG_BUS
		    LogMessage("PSTN", "%02X->%02X sec:%02X typ:%02X COMMAND RESP: 0x%02X%02X", src, dst, sec, typ, msg[0], msg[1]);
#endif
		    /* Verifico si en la cola de salida tengo que eliminar un mensaje de Key Press */
		    if(QueueView(gC485TxQueue, (void**)&c))
		    {
		    	if( c->dsta == src && c->srca == dst && c->sqn == sec && c->data[0] == 0x02 )
		    	{
		    		LogMessage("PSTN", "ACK de %02X", sec);
		    		gC485CmdTimeout = 0;
		    		if(c->on_ok != 0) gC485St = c->on_ok;
		    		QueueDel(gC485TxQueue);
		    	}
		    }

			break;
    }
}

void PSTNProcessSendQueue( void )
{
    c485_cmd *c, C;
    unsigned char tx_pstn_len;
    unsigned char tx_pstn_temp[32];
    unsigned char tx_pstn_buffer[32];
    unsigned char i, j;
    unsigned int chk_sum;
    unsigned char resp;

    c = &C;

	if(gC485St == C485_ERROR) return;
	/* Me fijo si tengo alguna respuesta para enviar */
	resp = 1;
	if( !QueueGet(gC485RespQueue, c))
	{
		if(gC485CmdTimeout || gC485SendDelay) return;
		/* Me fijo si tengo algún mensaje para enviar */
		if( !QueueView(gC485TxQueue, (void**)&c) ) return;
		resp = 0;
	}
	/* Armo el mensaje en formato POSITRON */
	tx_pstn_len = 0;
	tx_pstn_temp[tx_pstn_len++] = PSTN_START_MESSAGE;
	tx_pstn_temp[tx_pstn_len++] = 0;				/* ADDR LEN = 0 -> Address de 1 byte */
	tx_pstn_temp[tx_pstn_len++] = c->dsta;			/* Destino */
	tx_pstn_temp[tx_pstn_len++] = c->srca;			/* Origen */
	tx_pstn_temp[tx_pstn_len++] = c->sqn;			/* Secuencia */
	tx_pstn_temp[tx_pstn_len++] = c->vrs;			/* Version */
	tx_pstn_temp[tx_pstn_len++] = c->len + 1;		/* Data Len, le sumo el tamaño del passlen mas el passlen que es cero */
	tx_pstn_temp[tx_pstn_len++] = 0;				/* Pass Len */
	memcpy(&tx_pstn_temp[tx_pstn_len], c->data, c->len);
	tx_pstn_len += c->len;
	/* Calculo y cargo el checksum salteando el start sentinell */
	chk_sum = crc_ccitt_ffff((unsigned char*)&tx_pstn_temp[1], tx_pstn_len-1);
	tx_pstn_temp[tx_pstn_len++] = chk_sum / 256;
	tx_pstn_temp[tx_pstn_len++] = chk_sum % 256;
	/* ESCapeo los caracteres especial */
	for(i = 0, j = 0; i < tx_pstn_len; i++, j++)
	{
		/* Si en el mensaje hay algún caracter similar al end sentinel se aplica una secuencia de escape */
		if(tx_pstn_temp[i] == PSTN_END_MESSAGE || tx_pstn_temp[i] == PSTN_ESC_CHAR)
		{
			tx_pstn_buffer[j] = PSTN_ESC_CHAR;
			j++;
			tx_pstn_buffer[j] = tx_pstn_temp[i]^PSTN_ESC_OPER;
		}
		else
		{
			tx_pstn_buffer[j] = tx_pstn_temp[i];
		}
	}
	tx_pstn_buffer[j++] = PSTN_END_MESSAGE;
	tx_pstn_buffer[j++] = 0xFF;
	tx_pstn_len = j;
	/* Envío */
	LogMessage("PSTN", "Enviando %02X %02X %02X sec %02X", c->data[0], c->data[1], c->data[2], c->sqn);
	HAL_UART_Transmit(&huart3, (uint8_t *)tx_pstn_buffer, tx_pstn_len, tx_pstn_len);
	if(c->time_out == 0)
	{
		if(resp)
			QueueDel(gC485RespQueue);
		else
			QueueDel(gC485TxQueue);
	}
	gC485CmdTimeout = c->time_out;
	HAL_UART_Receive_IT(&huart3, (uint8_t *)&chUART3, 1);
	gC485SendDelay = 1;
}

int PSTNReady( void )
{
	return (gc485Live && gC485NroTer)?1:0;
}

#endif /* POSITRON */
