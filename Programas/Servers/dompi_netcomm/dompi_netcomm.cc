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

#include <cjson/cJSON.h>

#include "dom32iowifi.h"
#include "config.h"

CGMServerWait *m_pServer;
DPConfig *pConfig;
CGMClient     *m_pClient;
int internal_timeout;
int external_timeout;

void OnClose(int sig);

int power2(int exp)
{
	switch(exp)
	{
		case 0x00: return 0x01;
		case 0x01: return 0x02;
		case 0x02: return 0x04;
		case 0x03: return 0x08;
		case 0x04: return 0x10;
		case 0x05: return 0x20;
		case 0x06: return 0x40;
		case 0x07: return 0x80;
		default:   return 0x00;
	}
}


int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[4096];
	unsigned long message_len;
	char s[16];
	int timer_count = 0;

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
	m_pServer->m_pLog->Add(1, "Iniciando interface NETCOMM");

	pConfig = new DPConfig("/etc/dompiweb.config");

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

	CGMInitData gminit;

    cJSON *json_req;
    cJSON *json_resp;
	cJSON *json_Direccion_IP;
	cJSON *json_Tipo_HW;
	cJSON *json_Tipo_ASS;
	cJSON *json_Port;
	cJSON *json_Estado;

    Dom32IoWifi *pD32W;
    Dom32IoWifi::wifi_config_data wifi_data;
    pD32W = new Dom32IoWifi(m_pServer->m_pLog);

	CGMServerBase::GMIOS call_server_resp;
	CGMClient::GMIOS call_client_resp;


	m_pServer->Suscribe("dompi_hw_set_port_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_get_port_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_set_comm_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_get_comm_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_set_io", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_switch_io", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_pulse_io", GM_MSG_TYPE_CR);

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 1 )) >= 0)
	{
		json_req = NULL;
		json_resp = NULL;
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(50, "%s:(Q)[%s]", fn, message);

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
				if(json_Direccion_IP && json_Tipo_HW && json_Port )
				{
					if(atoi(json_Tipo_HW->valuestring) == 0) /* RBPi Local */
					{
						if( (strlen(json_Direccion_IP->valuestring) == 0 ) || !strcmp(json_Direccion_IP->valuestring, "0.0.0.0") )
						{
							/* local */
							m_pServer->m_pLog->Add(50, "[dompi_pi_set_port_config][%s]", message);
							rc = m_pServer->Call("dompi_pi_set_port_config", message, strlen(message), &call_server_resp, internal_timeout);
							if(rc == 0)
							{
								strcpy(message, (const char*)call_server_resp.data);
							}
							else
							{
								sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
							}
							m_pServer->Free(call_server_resp);
						}
						else
						{
							/* remoto */
							m_pServer->m_pLog->Add(50, "[dompi_pi_set_port_config][%s]", message);
							gminit.m_host = json_Direccion_IP->valuestring;
							gminit.m_port = 5533;

							m_pClient = new CGMClient(&gminit);

							rc = m_pClient->Call("dompi_pi_set_port_config", message, strlen(message), &call_client_resp, external_timeout);
							if(rc == 0)
							{
								strcpy(message, (const char*)call_client_resp.data);
							}
							else
							{
								sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
							}
							m_pClient->Free(call_client_resp);
							delete(m_pClient);
						}

					}
					else if(atoi(json_Tipo_HW->valuestring) == 1) /* Dom32IOWiFi */
					{
						/* Envío de configuración a WiFi */
						if(pD32W->SetConfig(json_Direccion_IP->valuestring, message) == 0)
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error al encolar mensaje\"}}");
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
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(10, "ERROR al responder mensaje [dompi_hw_get_port_config]");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_get_port_config"))
			{
				if(json_Direccion_IP && json_Tipo_HW)
				{
					if(atoi(json_Tipo_HW->valuestring) == 0)
					{
						if( (strlen(json_Direccion_IP->valuestring) == 0 ) || !strcmp(json_Direccion_IP->valuestring, "0.0.0.0") )
						{
							/* local */
							m_pServer->m_pLog->Add(50, "[dompi_pi_get_port_config][%s]", message);
							rc = m_pServer->Call("dompi_pi_get_port_config", message, strlen(message), &call_server_resp, internal_timeout);
							if(rc == 0)
							{
								strcpy(message, (const char*)call_server_resp.data);
							}
							else
							{
								sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
							}
							m_pServer->Free(call_server_resp);
						}
						else
						{
							/* remoto */
							m_pServer->m_pLog->Add(50, "[dompi_pi_get_port_config][%s]", message);
							gminit.m_host = json_Direccion_IP->valuestring;
							gminit.m_port = 5533;

							m_pClient = new CGMClient(&gminit);

							rc = m_pClient->Call("dompi_pi_get_port_config", message, strlen(message), &call_client_resp, external_timeout);
							if(rc == 0)
							{
								strcpy(message, (const char*)call_client_resp.data);
							}
							else
							{
								sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
							}
							m_pClient->Free(call_client_resp);
							delete(m_pClient);
						}

					}
					else if(atoi(json_Tipo_HW->valuestring) == 1)
					{
						rc = pD32W->GetConfig(json_Direccion_IP->valuestring, message, 1024);
						m_pServer->m_pLog->Add(100, "pD32W->GetConfig(%s, ...) = %i", 
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
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(10, "ERROR al responder mensaje [dompi_hw_get_port_config]");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_set_comm_config"))
			{
				memset(&wifi_data, 0, sizeof(Dom32IoWifi::wifi_config_data));

				/* TODO: Enviar configuracion de WiFFi */

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(10, "ERROR al responder mensaje [dompi_hw_set_comm_config]");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_get_comm_config"))
			{
				memset(&wifi_data, 0, sizeof(Dom32IoWifi::wifi_config_data));

				if(json_Direccion_IP && json_Tipo_HW)
				{
					if(atoi(json_Tipo_HW->valuestring) == 0)
					{
						if( (strlen(json_Direccion_IP->valuestring) == 0 ) || !strcmp(json_Direccion_IP->valuestring, "0.0.0.0") )
						{
							/* local */
							m_pServer->m_pLog->Add(50, "[dompi_pi_get_comm_config][%s]", message);
							rc = m_pServer->Call("dompi_pi_get_comm_config", message, strlen(message), &call_server_resp, internal_timeout);
							if(rc == 0)
							{
								strcpy(message, (const char*)call_server_resp.data);
							}
							else
							{
								sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
							}
							m_pServer->Free(call_server_resp);
						}
						else
						{
							/* remoto */
							m_pServer->m_pLog->Add(50, "[dompi_pi_get_comm_config][%s]", message);
							gminit.m_host = json_Direccion_IP->valuestring;
							gminit.m_port = 5533;

							m_pClient = new CGMClient(&gminit);

							rc = m_pClient->Call("dompi_pi_get_comm_config", message, strlen(message), &call_client_resp, external_timeout);
							if(rc == 0)
							{
								strcpy(message, (const char*)call_client_resp.data);
							}
							else
							{
								sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
							}
							m_pClient->Free(call_client_resp);
							delete(m_pClient);
						}

					}
					else if(atoi(json_Tipo_HW->valuestring) == 1)
					{	/* Interface Vía IP con dos puertos */
						rc = pD32W->GetWifiConfig(json_Direccion_IP->valuestring, &wifi_data);
						m_pServer->m_pLog->Add(100, "pD32W->GetWifiConfig(%s, ...) = %i", 
											json_Direccion_IP->valuestring,
											rc);
						if(rc == 0)
						{
							/* OK */
							sprintf(message,
									"{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"AP1: %s(%s)\r\nAP2: %s(%s)\r\nHost1: %s:%i\r\nHost1: %s:%i\r\nRqstPath: %s\"}}", 
									wifi_data.wifi_ap1, wifi_data.wifi_ap1_pass,
									wifi_data.wifi_ap2, wifi_data.wifi_ap2_pass,
									wifi_data.wifi_host1,wifi_data.wifi_host1_port,
									wifi_data.wifi_host2,wifi_data.wifi_host2_port,
									wifi_data.rqst_path);
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
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(10, "ERROR al responder mensaje [dompi_hw_get_comm_config]");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_set_io"))
			{
				if(json_Direccion_IP && json_Tipo_HW && json_Tipo_ASS && json_Port && json_Estado )
				{
					if(atoi(json_Tipo_HW->valuestring) == 0)
					{
						if( (strlen(json_Direccion_IP->valuestring) == 0 ) || !strcmp(json_Direccion_IP->valuestring, "0.0.0.0") )
						{
							/* local */
							m_pServer->m_pLog->Add(50, "[dompi_pi_set_io][%s]", message);
							rc = m_pServer->Call("dompi_pi_set_io", message, strlen(message), &call_server_resp, internal_timeout);
							if(rc == 0)
							{
								strcpy(message, (const char*)call_server_resp.data);
							}
							else
							{
								sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
							}
							m_pServer->Free(call_server_resp);
						}
						else
						{
							/* remoto */
							m_pServer->m_pLog->Add(50, "[dompi_pi_set_io][%s]", message);
							gminit.m_host = json_Direccion_IP->valuestring;
							gminit.m_port = 5533;

							m_pClient = new CGMClient(&gminit);

							rc = m_pClient->Call("dompi_pi_set_io", message, strlen(message), &call_client_resp, external_timeout);
							if(rc == 0)
							{
								strcpy(message, (const char*)call_client_resp.data);
							}
							else
							{
								sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
							}
							m_pClient->Free(call_client_resp);
							delete(m_pClient);
						}

					}
					else if(atoi(json_Tipo_HW->valuestring) == 1)
					{	
						/* Interface Vía IP mensajeria HTTP */
						if( atoi(json_Tipo_ASS->valuestring) == 0 ||	/* Salida */
							atoi(json_Tipo_ASS->valuestring) == 3 ||	/* Salida Alarma */
							atoi(json_Tipo_ASS->valuestring) == 5  )	/* Salida pulso */
						{	
							/* Enviar mensaje al WiFi */
							pD32W->SetIO(json_Direccion_IP->valuestring, json_req);
							strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"No es salida\"}}");
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
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(10, "ERROR al responder mensaje [dompi_hw_set_io]");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_switch_io"))
			{
				if(json_Direccion_IP && json_Tipo_HW && json_Tipo_ASS && json_Port )
				{
					if(atoi(json_Tipo_HW->valuestring) == 0)
					{
						if( (strlen(json_Direccion_IP->valuestring) == 0 ) || !strcmp(json_Direccion_IP->valuestring, "0.0.0.0") )
						{
							/* local */
							m_pServer->m_pLog->Add(50, "[dompi_pi_switch_io][%s]", message);
							rc = m_pServer->Call("dompi_pi_switch_io", message, strlen(message), &call_server_resp, internal_timeout);
							if(rc == 0)
							{
								strcpy(message, (const char*)call_server_resp.data);
							}
							else
							{
								sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
							}
							m_pServer->Free(call_server_resp);
						}
						else
						{
							/* remoto */
							m_pServer->m_pLog->Add(50, "[dompi_pi_switch_io][%s]", message);
							gminit.m_host = json_Direccion_IP->valuestring;
							gminit.m_port = 5533;

							m_pClient = new CGMClient(&gminit);

							rc = m_pClient->Call("dompi_pi_switch_io", message, strlen(message), &call_client_resp, external_timeout);
							if(rc == 0)
							{
								strcpy(message, (const char*)call_client_resp.data);
							}
							else
							{
								sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
							}
							m_pClient->Free(call_client_resp);
							delete(m_pClient);
						}

					}
					else if(atoi(json_Tipo_HW->valuestring) == 1)
					{	
						/* Interface Vía IP mensajeria HTTP */
						if( atoi(json_Tipo_ASS->valuestring) == 0 ||	/* Salida */
							atoi(json_Tipo_ASS->valuestring) == 3 ||	/* Salida Alarma */
							atoi(json_Tipo_ASS->valuestring) == 5  )	/* Salida pulso */
						{
							/* Enviar mensaje al WiFi */
							pD32W->SwitchIO(json_Direccion_IP->valuestring, json_req);
							strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"No es salida\"}}");
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
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(10, "ERROR al responder mensaje [dompi_hw_switch_io]");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_pulse_io"))
			{
				if(json_Direccion_IP && json_Tipo_HW && json_Tipo_ASS && json_Port)
				{
					if(atoi(json_Tipo_HW->valuestring) == 0)
					{
						if( (strlen(json_Direccion_IP->valuestring) == 0 ) || !strcmp(json_Direccion_IP->valuestring, "0.0.0.0") )
						{
							/* local */
							m_pServer->m_pLog->Add(50, "[dompi_pi_pulse_io][%s]", message);
							rc = m_pServer->Call("dompi_pi_pulse_io", message, strlen(message), &call_server_resp, internal_timeout);
							if(rc == 0)
							{
								strcpy(message, (const char*)call_server_resp.data);
							}
							else
							{
								sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
							}
							m_pServer->Free(call_server_resp);
						}
						else
						{
							/* remoto */
							m_pServer->m_pLog->Add(50, "[dompi_pi_pulse_io][%s]", message);
							gminit.m_host = json_Direccion_IP->valuestring;
							gminit.m_port = 5533;

							m_pClient = new CGMClient(&gminit);

							rc = m_pClient->Call("dompi_pi_pulse_io", message, strlen(message), &call_client_resp, external_timeout);
							if(rc == 0)
							{
								strcpy(message, (const char*)call_client_resp.data);
							}
							else
							{
								sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
							}
							m_pClient->Free(call_client_resp);
							delete(m_pClient);
						}

					}
					else if(atoi(json_Tipo_HW->valuestring) == 1)
					{	
						/* Interface Vía IP mensajeria HTTP */
						if( atoi(json_Tipo_ASS->valuestring) == 0 ||	/* Salida */
							atoi(json_Tipo_ASS->valuestring) == 3 ||	/* Salida Alarma */
							atoi(json_Tipo_ASS->valuestring) == 5  )	/* Salida pulso */
						{
							/* Enviar mensaje al WiFi */
							pD32W->PulseIO(json_Direccion_IP->valuestring, json_req);
							strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"No es salida\"}}");
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
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(10, "ERROR al responder mensaje [dompi_hw_pulse_io]");
				}
			}




			else
			{
				m_pServer->m_pLog->Add(50, "GME_SVC_NOTFOUND");
				m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
			}
		}
		else
		{
			/* expiracion del timer */
			timer_count++;
			if(timer_count == 100)
			{
				timer_count = 0;
				pD32W->Timer();
			}
			pD32W->Task();
		}
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

	m_pServer->UnSuscribe("dompi_hw_set_port_config", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_get_port_config", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_set_comm_config", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_get_comm_config", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_set_port", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_get_port", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_set_io", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_switch_io", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_pulse_io", GM_MSG_TYPE_CR);


	delete m_pServer;
	exit(0);
}
