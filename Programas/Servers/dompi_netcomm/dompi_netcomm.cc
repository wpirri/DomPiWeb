/***************************************************************************
    Copyright (C) 2021   Walter Pirri

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/
#include <gmonitor/gmerror.h>
#include <gmonitor/gmontdb.h>
/*#include <gmonitor/gmstring.h>*/
#include <gmonitor/gmswaited.h>
#include <gmonitor/gmc.h>

#include <string>
#include <iostream>
#include <csignal>
using namespace std;

#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_BUFFER_LEN 32767

#include <cjson/cJSON.h>

#include "dom32iowifi.h"
#include "gevent.h"
#include "cdb.h"
#include "rbpiio.h"
#include "config.h"
#include "defines.h"

CGMServerWait *m_pServer;
DPConfig *pConfig;
GEvent *pEV;
CGMClient     *m_pClient;
int internal_timeout;
int external_timeout;
CDB *pDB;
#ifdef ACTIVO_ACTIVO
char sys_backup[32];
#endif

#define BT_BUF_SIZE 256

void OnClose(int sig);
int GetSAFRemoto(const char* host, int port, const char* proto, const char* saf_name, char* msg, unsigned int msg_max);
void Update_Last_Connection(const char* id, const char* data);
#ifdef ACTIVO_ACTIVO
int Check_Remote_Synch( void );
#endif
void SetIO_CallBack(const char* id, const char* data);
void SwitchIO_CallBack(const char* id, const char* data);
void PulseIO_CallBack(const char* id, const char* data);
void GetWifiConfig_CallBack(const char* id, const char* data);
void SetWifiConfig_CallBack(const char* id, const char* data);
void Pulse_GetConfig(const char* id, const char* data);
void GetConfig_CallBack(const char* id, const char* data);
void SetConfig_CallBack(const char* id, const char* data);
void GetIO_CallBack(const char* id, const char* data);
void SetTime_CallBack(const char* id, const char* data);


/* ****************************************************************************
 * MAIN
 * ***************************************************************************/

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[4096];
	unsigned long message_len;
	char s[16];
	int timer_count = 0;
	#ifdef ACTIVO_ACTIVO
	int timer_synch = 0;
	#endif

	char db_host[32];
	char db_name[32];
	char db_user[32];
	char db_password[32];

	CGMInitData gminit;

    cJSON *json_req;
    cJSON *json_resp;
	cJSON *json_Direccion_IP;
	cJSON *json_Tipo_HW;
	cJSON *json_Tipo_ASS;
	cJSON *json_Port;
	cJSON *json_Estado;
	cJSON *json_ap1;
	cJSON *json_ap1_pass;
	cJSON *json_ap2;
	cJSON *json_ap2_pass;
	cJSON *json_host1;
	cJSON *json_host2;
	cJSON *json_rqst_path;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGKILL, OnClose);
	signal(SIGTERM, OnClose);
	signal(SIGSTOP, OnClose);
	signal(SIGABRT, OnClose);
	signal(SIGQUIT, OnClose);
	signal(SIGINT,  OnClose);
	signal(SIGILL,  OnClose);
	signal(SIGFPE,  OnClose);
	signal(SIGSEGV, OnClose);
	signal(SIGBUS,  OnClose);

	m_pServer = new CGMServerWait;
	m_pServer->Init("dompi_netcomm");
	m_pServer->m_pLog->Add(1, "Iniciando interface NETCOMM...");

	m_pServer->m_pLog->Add(10, "Leyendo configuración...");
	pConfig = new DPConfig("/etc/dompiweb.config");

	//pConfig->GetParam("SQLITE_DB_FILENAME", db_filename);
	pConfig->GetParam("DBHOST", db_host);
	pConfig->GetParam("DBNAME", db_name);
	pConfig->GetParam("DBUSER", db_user);
	pConfig->GetParam("DBPASSWORD", db_password);

	internal_timeout = 1000;
	if( pConfig->GetParam("INTERNAL-TIMEOUT", s))
	{
		internal_timeout = atoi(s) * 1000;
	}

	external_timeout = 1000;
	if( pConfig->GetParam("EXTERNAL-TIMEOUT", s))
	{
		external_timeout = atoi(s) * 1000;
	}

#ifdef ACTIVO_ACTIVO
	sys_backup[0] = 0;
	pConfig->GetParam("BACKUP", sys_backup);
#endif

    Dom32IoWifi *pD32W;
    Dom32IoWifi::wifi_config_data dom32_wifi_data;
    pD32W = new Dom32IoWifi(m_pServer);

	RBPiIO *pRBPi;
    RBPiIO::rbpi_config_data rbpi_wifi_data;
    pRBPi = new RBPiIO(m_pServer);

	//m_pServer->m_pLog->Add(10, "Conectando a la base de datos %s...", db_filename);
	//pDB = new CDB(db_filename);
	m_pServer->m_pLog->Add(10, "Conectando a la base de datos %s en %s ...", db_name, db_host);
	pDB = new CDB(db_host, db_name, db_user, db_password);
	if(pDB->Open() != 0)
	{
		m_pServer->m_pLog->Add(1, "ERROR al conectar con la base de datos %s en %s.", db_name, db_host);
		OnClose(0);
	}
	else
	{
		//m_pServer->m_pLog->Add(10, "Conectado a la base de datos %s", db_filename);
		m_pServer->m_pLog->Add(10, "Conectado a la base de datos %s en %s.", db_name, db_host);
	}

	pEV = new GEvent(pDB, m_pServer);

	m_pServer->Suscribe("dompi_hw_set_port_config", GM_MSG_TYPE_NOT);	/* Sin respuesta, lo atiende el mas libre */
	m_pServer->Suscribe("dompi_hw_get_port_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_set_comm_config", GM_MSG_TYPE_NOT);	/* Sin respuesta, lo atiende el mas libre */
	m_pServer->Suscribe("dompi_hw_get_comm_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_set_time_config", GM_MSG_TYPE_NOT);	/* Sin respuesta, lo atiende el mas libre */
	m_pServer->Suscribe("dompi_hw_set_io", GM_MSG_TYPE_NOT);			/* Sin respuesta, lo atiende el mas libre */
	m_pServer->Suscribe("dompi_hw_switch_io", GM_MSG_TYPE_NOT);			/* Sin respuesta, lo atiende el mas libre */
	m_pServer->Suscribe("dompi_hw_pulse_io", GM_MSG_TYPE_NOT);			/* Sin respuesta, lo atiende el mas libre */

	m_pServer->m_pLog->Add(1, "Servicios de Comunicación con Interfaces Locales inicializados.");

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 1 )) >= 0)
	{
		json_req = NULL;
		json_resp = NULL;
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(90, "%s:(Q)[%s]", fn, message);

			json_req = cJSON_Parse(message);
			json_Direccion_IP = cJSON_GetObjectItemCaseSensitive(json_req, "Direccion_IP");
			json_Tipo_HW = cJSON_GetObjectItemCaseSensitive(json_req, "Tipo_HW");
			json_Tipo_ASS = cJSON_GetObjectItemCaseSensitive(json_req, "Tipo_ASS");
			json_Port = cJSON_GetObjectItemCaseSensitive(json_req, "Port");
			json_Estado = cJSON_GetObjectItemCaseSensitive(json_req, "Estado");
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			if( !strcmp(fn, "dompi_hw_set_port_config"))
			{
				//m_pServer->Resp(NULL, 0, GME_OK);

				if(json_Direccion_IP && json_Tipo_HW)
				{
					if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_WIFI) /* Dom32IOWiFi */
					{
						/* Envío de configuración a WiFi */
						pD32W->SetConfig(json_Direccion_IP->valuestring, message, SetConfig_CallBack);
					}
					else if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_RBPI) /* RBPi */
					{
						pRBPi->SetConfig(json_Direccion_IP->valuestring, message, SetConfig_CallBack);
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"HW no soportado\"}}");
					}
				}
				else
				{
					m_pServer->m_pLog->Add(1, "[dompi_hw_set_port_config] ERROR: Datos insuficientes");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_get_port_config"))
			{
				if(json_Direccion_IP && json_Tipo_HW)
				{
					if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_WIFI) /* Dom32IOWiFi */
					{
						rc = pD32W->GetConfig(json_Direccion_IP->valuestring, message, 1024, GetConfig_CallBack);
						m_pServer->m_pLog->Add(100, "pD32W->GetConfig(%s, ...) = %i", 
											json_Direccion_IP->valuestring,
											rc);
						if(rc != 0)
						{
							/* Otro Error */
							strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"General Error\"}}");
						}
					}
					else if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_RBPI) /* RBPi */
					{
						rc = pRBPi->GetConfig(json_Direccion_IP->valuestring, message, 1024, GetConfig_CallBack);
						m_pServer->m_pLog->Add(100, "pRBPi->GetConfig(%s, ...) = %i", 
											json_Direccion_IP->valuestring,
											rc);
						if(rc != 0)
						{
							/* Otro Error */
							strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"General Error\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"HW no soportado\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"4\", \"resp_msg\":\"Datos insuficientes\"}}");
				}
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [dompi_hw_get_port_config]");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_set_comm_config"))
			{
				//m_pServer->Resp(NULL, 0, GME_OK);

				if(json_Direccion_IP && json_Tipo_HW)
				{
					if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_WIFI) /* Dom32IOWiFi */
					{
						/* Envío de configuración a WiFi */
						memset(&dom32_wifi_data, 0, sizeof(Dom32IoWifi::wifi_config_data));

						json_ap1 = cJSON_GetObjectItemCaseSensitive(json_req, "Wifi_AP1");
						json_ap1_pass = cJSON_GetObjectItemCaseSensitive(json_req, "Wifi_AP1_Pass");
						json_ap2 = cJSON_GetObjectItemCaseSensitive(json_req, "Wifi_AP2");
						json_ap2_pass = cJSON_GetObjectItemCaseSensitive(json_req, "Wifi_AP2_Pass");
						json_host1 = cJSON_GetObjectItemCaseSensitive(json_req, "Home_Host_1_Address");
						json_host2 = cJSON_GetObjectItemCaseSensitive(json_req, "Home_Host_2_Address");
						json_rqst_path = cJSON_GetObjectItemCaseSensitive(json_req, "Rqst_Path");

						if(json_ap1 && json_ap1_pass && json_ap2 && json_ap2_pass &&
							json_host1 && json_host2 && json_rqst_path)
						{
							strcpy(dom32_wifi_data.wifi_ap1, json_ap1->valuestring);
							strcpy(dom32_wifi_data.wifi_ap1_pass, json_ap1_pass->valuestring);
							strcpy(dom32_wifi_data.wifi_ap2, json_ap2->valuestring);
							strcpy(dom32_wifi_data.wifi_ap2_pass, json_ap2_pass->valuestring);
							strcpy(dom32_wifi_data.wifi_host1, json_host1->valuestring);
							strcpy(dom32_wifi_data.wifi_host2, json_host2->valuestring);
							strcpy(dom32_wifi_data.rqst_path, json_rqst_path->valuestring);

							pD32W->SetWifiConfig(json_Direccion_IP->valuestring, &dom32_wifi_data, SetWifiConfig_CallBack);
						}
						else
						{
							m_pServer->m_pLog->Add(1, "[dompi_hw_set_comm_config] ERROR: Datos insuficientes de WiFi");
						}
					}
					else if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_RBPI) /* RBPi */
					{
						memset(&rbpi_wifi_data, 0, sizeof(RBPiIO::rbpi_config_data));

						json_ap1 = cJSON_GetObjectItemCaseSensitive(json_req, "Wifi_AP1");
						json_ap1_pass = cJSON_GetObjectItemCaseSensitive(json_req, "Wifi_AP1_Pass");
						json_ap2 = cJSON_GetObjectItemCaseSensitive(json_req, "Wifi_AP2");
						json_ap2_pass = cJSON_GetObjectItemCaseSensitive(json_req, "Wifi_AP2_Pass");
						json_host1 = cJSON_GetObjectItemCaseSensitive(json_req, "Home_Host_1_Address");
						json_host2 = cJSON_GetObjectItemCaseSensitive(json_req, "Home_Host_2_Address");
						json_rqst_path = cJSON_GetObjectItemCaseSensitive(json_req, "Rqst_Path");

						if(json_ap1 && json_ap1_pass && json_ap2 && json_ap2_pass &&
							json_host1 && json_host2 && json_rqst_path)
						{
							strcpy(rbpi_wifi_data.wifi_ap1, json_ap1->valuestring);
							strcpy(rbpi_wifi_data.wifi_ap1_pass, json_ap1_pass->valuestring);
							strcpy(rbpi_wifi_data.wifi_ap2, json_ap2->valuestring);
							strcpy(rbpi_wifi_data.wifi_ap2_pass, json_ap2_pass->valuestring);
							strcpy(rbpi_wifi_data.wifi_host1, json_host1->valuestring);
							strcpy(rbpi_wifi_data.wifi_host2, json_host2->valuestring);
							strcpy(rbpi_wifi_data.rqst_path, json_rqst_path->valuestring);

							pRBPi->SetWifiConfig(json_Direccion_IP->valuestring, &rbpi_wifi_data, NULL);
						}
						else
						{
							m_pServer->m_pLog->Add(1, "[dompi_hw_set_comm_config] ERROR: Datos insuficientes de WiFi");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"HW no soportado\"}}");
					}
				}
				else
				{
					m_pServer->m_pLog->Add(1, "[dompi_hw_set_comm_config] ERROR: Datos insuficientes");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_get_comm_config"))
			{
				if(json_Direccion_IP && json_Tipo_HW)
				{
					if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_WIFI) /* Dom32IOWiFi */
					{
						memset(&dom32_wifi_data, 0, sizeof(Dom32IoWifi::wifi_config_data));

						rc = pD32W->GetWifiConfig(json_Direccion_IP->valuestring, &dom32_wifi_data, GetWifiConfig_CallBack);
						m_pServer->m_pLog->Add(100, "pD32W->GetWifiConfig(%s, ...) = %i", 
											json_Direccion_IP->valuestring,
											rc);
						if(rc == 0)
						{
							/* OK */
							sprintf(message,
									"{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"AP1: %s(%s)\r\nAP2: %s(%s)\r\nHost1: %s:%i\r\nHost1: %s:%i\r\nRqstPath: %s\"}}", 
									dom32_wifi_data.wifi_ap1, dom32_wifi_data.wifi_ap1_pass,
									dom32_wifi_data.wifi_ap2, dom32_wifi_data.wifi_ap2_pass,
									dom32_wifi_data.wifi_host1,dom32_wifi_data.wifi_host1_port,
									dom32_wifi_data.wifi_host2,dom32_wifi_data.wifi_host2_port,
									dom32_wifi_data.rqst_path);
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
						}
					}
					else if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_RBPI) /* RBPi */
					{
						memset(&rbpi_wifi_data, 0, sizeof(RBPiIO::rbpi_config_data));

						rc = pRBPi->GetWifiConfig(json_Direccion_IP->valuestring, &rbpi_wifi_data, nullptr);
						m_pServer->m_pLog->Add(100, "pRBPi->GetWifiConfig(%s, ...) = %i", 
											json_Direccion_IP->valuestring,
											rc);
						if(rc == 0)
						{
							/* OK */
							sprintf(message,
									"{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"AP1: %s(%s)\r\nAP2: %s(%s)\r\nHost1: %s:%i\r\nHost1: %s:%i\r\nRqstPath: %s\"}}", 
									rbpi_wifi_data.wifi_ap1, rbpi_wifi_data.wifi_ap1_pass,
									rbpi_wifi_data.wifi_ap2, rbpi_wifi_data.wifi_ap2_pass,
									rbpi_wifi_data.wifi_host1,rbpi_wifi_data.wifi_host1_port,
									rbpi_wifi_data.wifi_host2,rbpi_wifi_data.wifi_host2_port,
									rbpi_wifi_data.rqst_path);
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"HW no soportado\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"4\", \"resp_msg\":\"Datos insuficientes\"}}");
				}
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [dompi_hw_get_comm_config]");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_set_time_config"))
			{
				//m_pServer->Resp(NULL, 0, GME_OK);

				if(json_Direccion_IP && json_Tipo_HW)
				{
					if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_WIFI) /* Dom32IOWiFi */
					{
						/* Interface Vía IP mensajeria HTTP */
						/* Envío de configuración a WiFi */
						pD32W->SetTime(json_Direccion_IP->valuestring, SetTime_CallBack);
					}
					else if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_RBPI) /* RBPi */
					{
						pRBPi->SetTime(json_Direccion_IP->valuestring, SetTime_CallBack);
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"HW no soportado\"}}");
					}
				}
				else
				{
					m_pServer->m_pLog->Add(1, "[dompi_hw_set_time_config] ERROR: Datos insuficientes");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_set_io"))
			{
				//m_pServer->Resp(NULL, 0, GME_OK);

				if(json_Direccion_IP && json_Tipo_HW && json_Tipo_ASS && json_Port && json_Estado )
				{
					m_pServer->m_pLog->Add(20, "[dompi_hw_set_io] IP: %s Port: %s Estado: %s",
											json_Direccion_IP->valuestring, json_Port->valuestring, json_Estado->valuestring);
					if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_WIFI) /* Dom32IOWiFi */
					{
						/* Interface Vía IP mensajeria HTTP */
						if( atoi(json_Tipo_ASS->valuestring) == 0 ||	/* Salida */
							atoi(json_Tipo_ASS->valuestring) == 3 ||	/* Salida Alarma */
							atoi(json_Tipo_ASS->valuestring) == 5  )	/* Salida pulso */
						{	
							/* Enviar mensaje al WiFi */
							pD32W->SetIO(json_Direccion_IP->valuestring, json_req, SetIO_CallBack);
						}
					}
					else if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_RBPI) /* RBPi */
					{
						if( atoi(json_Tipo_ASS->valuestring) == 0 ||	/* Salida */
							atoi(json_Tipo_ASS->valuestring) == 3 ||	/* Salida Alarma */
							atoi(json_Tipo_ASS->valuestring) == 5  )	/* Salida pulso */
						{	
							pRBPi->SetIO(json_Direccion_IP->valuestring, json_req, SetIO_CallBack);
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"HW no soportado\"}}");
					}
				}
				else
				{
					m_pServer->m_pLog->Add(1, "[dompi_hw_set_io] ERROR: Datos insuficientes");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_switch_io"))
			{
				//m_pServer->Resp(NULL, 0, GME_OK);

				if(json_Direccion_IP && json_Tipo_HW && json_Tipo_ASS && json_Port )
				{
					m_pServer->m_pLog->Add(20, "[dompi_hw_switch_io] IP: %s Port: %s",
											json_Direccion_IP->valuestring, json_Port->valuestring);
					if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_WIFI) /* Dom32IOWiFi */
					{
						/* Interface Vía IP mensajeria HTTP */
						if( atoi(json_Tipo_ASS->valuestring) == 0 ||	/* Salida */
							atoi(json_Tipo_ASS->valuestring) == 3 ||	/* Salida Alarma */
							atoi(json_Tipo_ASS->valuestring) == 5  )	/* Salida pulso */
						{
							/* Enviar mensaje al WiFi */
							pD32W->SwitchIO(json_Direccion_IP->valuestring, json_req, SwitchIO_CallBack);
						}
						else
						{
							m_pServer->m_pLog->Add(1, "[dompi_hw_switch_io] ERROR: No es salida");
						}
					}
					else if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_RBPI) /* RBPi */
					{
						if( atoi(json_Tipo_ASS->valuestring) == 0 ||	/* Salida */
							atoi(json_Tipo_ASS->valuestring) == 3 ||	/* Salida Alarma */
							atoi(json_Tipo_ASS->valuestring) == 5  )	/* Salida pulso */
						{
							pRBPi->SwitchIO(json_Direccion_IP->valuestring, json_req, SwitchIO_CallBack);
						}
						else
						{
							m_pServer->m_pLog->Add(1, "[dompi_hw_switch_io] ERROR: No es salida");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"HW no soportado\"}}");
					}
				}
				else
				{
					m_pServer->m_pLog->Add(1, "[dompi_hw_switch_io] ERROR: Datos insuficientes");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_pulse_io"))
			{
				//m_pServer->Resp(NULL, 0, GME_OK);
				
				if(json_Direccion_IP && json_Tipo_HW && json_Tipo_ASS && json_Port)
				{
					m_pServer->m_pLog->Add(20, "[dompi_hw_pulse_io] IP: %s Port: %s",
											json_Direccion_IP->valuestring, json_Port->valuestring);
					if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_WIFI) /* Dom32IOWiFi */
					{
						/* Interface Vía IP mensajeria HTTP */
						if( atoi(json_Tipo_ASS->valuestring) == 0 ||	/* Salida */
							atoi(json_Tipo_ASS->valuestring) == 3 ||	/* Salida Alarma */
							atoi(json_Tipo_ASS->valuestring) == 5  )	/* Salida pulso */
						{
							/* Enviar mensaje al WiFi */
							pD32W->PulseIO(json_Direccion_IP->valuestring, json_req, PulseIO_CallBack);
						}
						else
						{
							m_pServer->m_pLog->Add(1, "[dompi_hw_pulse_io] ERROR: No es salida");
						}
					}
					else if(atoi(json_Tipo_HW->valuestring) == TIPO_HW_RBPI) /* RBPi */
					{
						if( atoi(json_Tipo_ASS->valuestring) == 0 ||	/* Salida */
							atoi(json_Tipo_ASS->valuestring) == 3 ||	/* Salida Alarma */
							atoi(json_Tipo_ASS->valuestring) == 5  )	/* Salida pulso */
						{
							/* Enviar mensaje al WiFi */
							pRBPi->PulseIO(json_Direccion_IP->valuestring, json_req, PulseIO_CallBack);
						}
						else
						{
							m_pServer->m_pLog->Add(1, "[dompi_hw_pulse_io] ERROR: No es salida");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"HW no soportado\"}}");
					}
				}
				else
				{
					m_pServer->m_pLog->Add(1, "[dompi_hw_pulse_io] ERROR: Datos insuficientes");
				}
			}




			else
			{
				m_pServer->m_pLog->Add(90, "[%s][R][GME_SVC_NOTFOUND]", fn);
				m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
			}
		}
		else
		{
			/* expiracion del timer 1ms*/
			if(++timer_count == 10)
			{
				timer_count = 0;
				pD32W->Timer();
				pRBPi->Timer();
			}

#ifdef ACTIVO_ACTIVO
			/* expiracion del timer 10s*/
			if(++timer_synch >= 1000)
			{
				if(Check_Remote_Synch() != 1) timer_synch = 0;
			}
#endif /* ACTIVO_ACTIVO */
		}

		pD32W->Task();
		pRBPi->Task();

		if(json_req) cJSON_Delete(json_req);
		if(json_resp) cJSON_Delete(json_resp);
	}
	m_pServer->m_pLog->Add(50, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);

	m_pServer->UnSuscribe("dompi_hw_set_port_config", GM_MSG_TYPE_NOT);
	m_pServer->UnSuscribe("dompi_hw_get_port_config", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_set_comm_config", GM_MSG_TYPE_NOT);
	m_pServer->UnSuscribe("dompi_hw_get_comm_config", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_set_port", GM_MSG_TYPE_NOT);
	m_pServer->UnSuscribe("dompi_hw_get_port", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_set_io", GM_MSG_TYPE_NOT);
	m_pServer->UnSuscribe("dompi_hw_switch_io", GM_MSG_TYPE_NOT);
	m_pServer->UnSuscribe("dompi_hw_pulse_io", GM_MSG_TYPE_NOT);

	delete pEV;
	delete m_pServer;
	
	exit(0);
}

int GetSAFRemoto(const char* host, int port, const char* proto, const char* saf_name, char* msg, unsigned int msg_max)
{
    /*
    * GET
    * 1.- %s: URI
    * 2.- %s: GET
	* 3.- %s: Host
    */
    char http_get[] =   "GET %s%s HTTP/1.1\r\n"
                        "Host: %s\r\n"
                        "Connection: close\r\n"
                        "User-Agent: DomPiSrv/1.00 (RaspBerryPi;Dom32)\r\n"
                        "Accept: text/html,text/xml\r\n\r\n";

    char url_default[] = "/cgi-bin/gmonitor_get_saf.cgi";

	CTcp *pSock;
	char buffer[MAX_BUFFER_LEN+1];
	char get[256];
	char *ps;
	int rc = 0;

	buffer[0] = 0;
	sprintf(get, "?saf=%s", saf_name);
	if( proto && !strcmp(proto, "https"))
	{
		pSock = new CTcp(1, 3);
		if(port == 0) port = 443;
    		sprintf(buffer, http_get, url_default, get, host);
		rc =pSock->Query(host, port, buffer, buffer, MAX_BUFFER_LEN, external_timeout);
		delete pSock;
	}
	else
	{
		pSock = new CTcp();
		if(port == 0) port = 80;
    		sprintf(buffer, http_get, url_default, get, host);
		rc = pSock->Query(host, port, buffer, buffer, MAX_BUFFER_LEN, external_timeout);
		delete pSock;
	}
	if(msg)
	{
		*msg = 0;
		if(rc > 0 && strlen(buffer))
		{
			ps = strstr(buffer, "\r\n\r\n");
			if(ps)
			{
				ps += 4;

				/* Está viniendo algo raro al princiìo de la parte de datos que no la puedo sacar */
				while(*ps && *ps != '{') ps++;
				strncpy(msg, ps, msg_max);
			}
		}
		rc = strlen(msg);
	}
	return rc;
}

#ifdef ACTIVO_ACTIVO
int Check_Remote_Synch( void )
{
	int rc;
	char saf_message[MAX_BUFFER_LEN];

	if(strlen(sys_backup) == 0) return (-1);
	m_pServer->m_pLog->Add(100, "[Check_Remote_Synch]");

	rc = GetSAFRemoto(sys_backup, 80, "http", "dompi_infoio_synch", saf_message, MAX_BUFFER_LEN);
	m_pServer->m_pLog->Add(100, "[SYNCH] INFO rc=%i [%s]", rc, saf_message);
	if(rc > 100)
	{
		pEV->SyncIO(saf_message);
		rc = 1;
	}
	else
	{
		rc = GetSAFRemoto(sys_backup, 80, "http", "dompi_changeio_synch", saf_message, MAX_BUFFER_LEN);
		m_pServer->m_pLog->Add(100, "[SYNCH] CHANGE rc=%i [%s]", rc, saf_message);
		if(rc > 100)
		{
			pEV->ChangeIO(saf_message);
			rc = 1;
		}
		else
		{
			rc = 0;
		}
	}
	return rc;
}
#endif /* ACTIVO_ACTIVO */

/*  */
void Update_Last_Connection(const char* id, const char* data)
{
	int rc;
	char query[4096];
    time_t t;

	t = time(&t);
	sprintf(query, "UPDATE TB_DOM_PERIF "
						"SET Ultimo_Ok = %lu, "
						"Estado = 1 "
						"WHERE Direccion_IP = \'%s\' ;",
						t, id);
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(NULL, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);

	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[Update_Last_Connection] Id: %s Data: [%s]", id, data);
}

/* Callback para SetIO */
void SetIO_CallBack(const char* id, const char* data)
{
	m_pServer->m_pLog->Add(20, "[SetIO_CallBack] ID: %s", id);
	m_pServer->m_pLog->Add(100, "[SetIO_CallBack] Data: [%s]", data);
	Update_Last_Connection(id,data);

}

/* Callback para SwitchIO */
void SwitchIO_CallBack(const char* id, const char* data)
{
	m_pServer->m_pLog->Add(20, "[SwitchIO_CallBack] ID: %s", id);
	m_pServer->m_pLog->Add(100, "[SwitchIO_CallBack] Data: [%s]", data);
	Update_Last_Connection(id,data);

}

/* Callback para PulseIO */
void PulseIO_CallBack(const char* id, const char* data)
{
	m_pServer->m_pLog->Add(20, "[PulseIO_CallBack] ID: %s", id);
	m_pServer->m_pLog->Add(100, "[PulseIO_CallBack] Data: [%s]", data);
	Update_Last_Connection(id,data);

}

/* Callback para GetWifiConfig */
void GetWifiConfig_CallBack(const char* id, const char* data)
{
	m_pServer->m_pLog->Add(20, "[GetWifiConfig_CallBack] ID: %s", id);
	m_pServer->m_pLog->Add(100, "[GetWifiConfig_CallBack] Data: [%s]", data);
	Update_Last_Connection(id,data);

}

/* Callback para SetWifiConfig */
void SetWifiConfig_CallBack(const char* id, const char* data)
{
	m_pServer->m_pLog->Add(20, "[SetWifiConfig_CallBack] ID: %s", id);
	m_pServer->m_pLog->Add(100, "[SetWifiConfig_CallBack] Data: [%s]", data);
	Update_Last_Connection(id,data);

}

/* Callback para GetConfig */
void Pulse_GetConfig(const char* id, const char* data)
{
	m_pServer->m_pLog->Add(20, "[Pulse_GetConfig] ID: %s", id);
	m_pServer->m_pLog->Add(100, "[Pulse_GetConfig] Data: [%s]", data);
	Update_Last_Connection(id,data);

}

/* Callback para GetConfig */
void GetConfig_CallBack(const char* id, const char* data)
{
	m_pServer->m_pLog->Add(20, "[GetConfig_CallBack] ID: %s", id);
	m_pServer->m_pLog->Add(100, "[GetConfig_CallBack] Data: [%s]", data);
	Update_Last_Connection(id,data);

}

/* Callback para SetConfig */
void SetConfig_CallBack(const char* id, const char* data)
{
	m_pServer->m_pLog->Add(20, "[SetConfig_CallBack] ID: %s", id);
	m_pServer->m_pLog->Add(100, "[SetConfig_CallBack] Data: [%s]", data);
	Update_Last_Connection(id,data);

}

/* Callback para GetIO */
void GetIO_CallBack(const char* id, const char* data)
{
	m_pServer->m_pLog->Add(20, "[GetIO_CallBack] ID: %s", id);
	m_pServer->m_pLog->Add(100, "[GetIO_CallBack] Data: [%s]", data);
	Update_Last_Connection(id,data);

}

/* Callback para SetTime */
void SetTime_CallBack(const char* id, const char* data)
{
	m_pServer->m_pLog->Add(20, "[SetTime_CallBack] ID: %s", id);
	m_pServer->m_pLog->Add(100, "[SetTime_CallBack] Data: [%s]", data);
	Update_Last_Connection(id,data);

}
