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

CGMServerWait *m_pServer;
CGMClient     *m_pClient;

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
	int return_int1; 
	int return_int2; 

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

	CGMInitData gminit;

    cJSON *json_req;
    cJSON *json_resp;
	cJSON *json_Direccion_IP;
	cJSON *json_Tipo_HW;
	cJSON *json_Tipo_ASS;
	cJSON *json_Port;
	cJSON *json_E_S;
	cJSON *json_Estado;
	cJSON *json_IO_Config;
	cJSON *json_AN_Config;
	cJSON *json_Segundos;

    Dom32IoWifi *pD32W;
    Dom32IoWifi::wifi_config_data wifi_data;
    pD32W = new Dom32IoWifi();

	CGMServerBase::GMIOS call_server_resp;
	CGMClient::GMIOS call_client_resp;


	m_pServer->Suscribe("dompi_hw_set_port_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_get_port_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_set_comm_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_get_comm_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_set_port", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_get_port", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_set_io", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_switch_io", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_pulse_io", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_get_io", GM_MSG_TYPE_CR);

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 10 )) >= 0)
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
			json_E_S = cJSON_GetObjectItemCaseSensitive(json_req, "E_S");
			json_Estado = cJSON_GetObjectItemCaseSensitive(json_req, "Estado");
			json_IO_Config = cJSON_GetObjectItemCaseSensitive(json_req, "IO_Config");
			json_AN_Config = cJSON_GetObjectItemCaseSensitive(json_req, "AN_Config");
			json_Segundos = cJSON_GetObjectItemCaseSensitive(json_req, "Segundos");
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			if( !strcmp(fn, "dompi_hw_set_port_config"))
			{
				if(json_Direccion_IP && json_Tipo_HW && json_Port )
				{
					if(atoi(json_Tipo_HW->valuestring) == 0)
					{
						if( (strlen(json_Direccion_IP->valuestring) == 0 ) || !strcmp(json_Direccion_IP->valuestring, "0.0.0.0") )
						{
							/* local */
							m_pServer->m_pLog->Add(50, "[dompi_pi_set_port_config][%s]", message);
							rc = m_pServer->Call("dompi_pi_set_port_config", message, strlen(message), &call_server_resp, 500);
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
							gminit.m_host = json_Direccion_IP->valuestring;
							gminit.m_port = 5533;

							m_pClient = new CGMClient(&gminit);

							rc = m_pClient->Call("dompi_pi_set_port_config", message, strlen(message), &call_client_resp, 500);
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
						if(json_IO_Config)
						{
							if( atoi(json_Port->valuestring) == 1 )
							{
								rc = pD32W->ConfigIO(json_Direccion_IP->valuestring, 
												atol(json_IO_Config->valuestring),
												NULL);
								m_pServer->m_pLog->Add(100, "pD32W->ConfigIO(%s, %s) = %i", 
													json_Direccion_IP->valuestring,
													json_IO_Config->valuestring,
													rc);
								if(rc == 0)
								{
									/* OK */
									strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
								}
							}
							else if( atoi(json_Port->valuestring) == 2 )
							{
								rc = pD32W->ConfigEX(json_Direccion_IP->valuestring, 
												atol(json_IO_Config->valuestring),
												NULL);
								m_pServer->m_pLog->Add(100, "pD32W->ConfigEX(%s, %s) = %i", 
													json_Direccion_IP->valuestring,
													json_IO_Config->valuestring,
													rc);
								if(rc == 0)
								{
									/* OK */
									strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
								}
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid Port\"}}");
							}
						}
						else if(json_AN_Config)
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"5\", \"resp_msg\":\"No Implementado\"}}");
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"4\", \"resp_msg\":\"Datos insuficientes\"}}");
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
							rc = m_pServer->Call("dompi_pi_get_port_config", message, strlen(message), &call_server_resp, 500);
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
							gminit.m_host = json_Direccion_IP->valuestring;
							gminit.m_port = 5533;

							m_pClient = new CGMClient(&gminit);

							rc = m_pClient->Call("dompi_pi_get_port_config", message, strlen(message), &call_client_resp, 500);
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
						if( !json_Port)
						{
							rc = pD32W->GetConfig(json_Direccion_IP->valuestring, &return_int1, &return_int2);
							m_pServer->m_pLog->Add(100, "pD32W->GetConfig(%s, ...) = %i", 
												json_Direccion_IP->valuestring,
												rc);
							if(rc == 0)
							{
								/* OK */
								sprintf(message,
										"{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Port A: 0x%02X - Port B: 0x%02X\"}}", return_int1, return_int2);
							}
							else
							{
								/* Otro Error */
								strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"General Error\"}}");
							}
						}
						else if( atoi(json_Port->valuestring) == 1 )
						{
							rc = pD32W->GetConfig(json_Direccion_IP->valuestring, &return_int1, NULL);
							m_pServer->m_pLog->Add(100, "pD32W->GetConfig(%s, ...) = %i", 
												json_Direccion_IP->valuestring,
												rc);
							if(rc == 0)
							{
								/* OK */
								sprintf(message,
										"{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"0x%02X\"}}", return_int1);
							}
							else
							{
								/* Otro Error */
								strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"General Error\"}}");
							}
						}
						else if( atoi(json_Port->valuestring) == 2 )
						{
							rc = pD32W->GetConfig(json_Direccion_IP->valuestring, NULL, &return_int1);
							m_pServer->m_pLog->Add(100, "pD32W->GetConfig(%s, ...) = %i", 
												json_Direccion_IP->valuestring,
												rc);
							if(rc == 0)
							{
								/* OK */
								sprintf(message,
										"{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"0x%02X\"}}", return_int1);
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
							}
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid Port\"}}");
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
							rc = m_pServer->Call("dompi_pi_get_comm_config", message, strlen(message), &call_server_resp, 500);
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
							gminit.m_host = json_Direccion_IP->valuestring;
							gminit.m_port = 5533;

							m_pClient = new CGMClient(&gminit);

							rc = m_pClient->Call("dompi_pi_get_comm_config", message, strlen(message), &call_client_resp, 500);
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
						rc = pD32W->GetWifi(json_Direccion_IP->valuestring, 
											&wifi_data);
						m_pServer->m_pLog->Add(100, "pD32W->GetWifi(%s, ...) = %i", 
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
			else if( !strcmp(fn, "dompi_hw_set_port"))
			{
				/* TODO: Envio de estado de port completo */


				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(10, "ERROR al responder mensaje [dompi_hw_set_port]");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_get_port"))
			{
				if(json_Direccion_IP && json_Tipo_HW)
				{
					if(atoi(json_Tipo_HW->valuestring) == 0)
					{
						if( (strlen(json_Direccion_IP->valuestring) == 0 ) || !strcmp(json_Direccion_IP->valuestring, "0.0.0.0") )
						{
							/* local */
							m_pServer->m_pLog->Add(50, "[dompi_pi_get_port][%s]", message);
							rc = m_pServer->Call("dompi_pi_get_port", message, strlen(message), &call_server_resp, 500);
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
							gminit.m_host = json_Direccion_IP->valuestring;
							gminit.m_port = 5533;

							m_pClient = new CGMClient(&gminit);

							rc = m_pClient->Call("dompi_pi_get_port", message, strlen(message), &call_client_resp, 500);
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
						if(json_Port)
						{
							if( atoi(json_Port->valuestring) == 1 )
							{	/* Puerto1 */
								rc = pD32W->GetIOStatus(json_Direccion_IP->valuestring, 
													&return_int1);
								m_pServer->m_pLog->Add(100, "pD32W->GetIOStatus(%s, 0x%02X) = %i", 
													json_Direccion_IP->valuestring,
													return_int1,
													rc);
								if(rc == 0)
								{
									/* OK */
									sprintf(message,
											"{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"0x%02X\"}}", return_int1);
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
								}
							}
							else if( atoi(json_Port->valuestring) == 2 )
							{	/* Puerto2 */
								rc = pD32W->GetEXStatus(json_Direccion_IP->valuestring, 
													&return_int1);
								m_pServer->m_pLog->Add(100, "pD32W->GetIEXtatus(%s, 0x%02X) = %i", 
													json_Direccion_IP->valuestring,
													return_int1,
													rc);
								if(rc == 0)
								{
									/* OK */
									sprintf(message,
											"{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"0x%02X\"}}", return_int1);
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
								}
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid Port\"}}");
							}
						}
						else
						{
							/* Pido los dos puertos de este tipo de hw */
							rc = pD32W->GetIOStatus(json_Direccion_IP->valuestring, 
												&return_int1);
							m_pServer->m_pLog->Add(100, "pD32W->GetIOStatus(%s, 0x%02X) = %i", 
												json_Direccion_IP->valuestring,
												return_int1,
												rc);
							rc |= pD32W->GetEXStatus(json_Direccion_IP->valuestring, 
												&return_int2);
							m_pServer->m_pLog->Add(100, "pD32W->GetIOStatus(%s, 0x%02X) = %i", 
												json_Direccion_IP->valuestring,
												return_int2,
												rc);
							if(rc == 0)
							{
								/* OK */
								sprintf(message,
										"{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Port A: 0x%02X\r\nPort B: 0x%02X\"}}", return_int1, return_int2);
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
							}
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
					m_pServer->m_pLog->Add(10, "ERROR al responder mensaje [dompi_hw_get_port]");
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_set_io"))
			{
				if(json_Direccion_IP && json_Tipo_HW && json_Tipo_ASS && json_Port && json_E_S && json_Estado )
				{
					if(atoi(json_Tipo_HW->valuestring) == 0)
					{
						if( (strlen(json_Direccion_IP->valuestring) == 0 ) || !strcmp(json_Direccion_IP->valuestring, "0.0.0.0") )
						{
							/* local */
							m_pServer->m_pLog->Add(50, "[dompi_pi_set_io][%s]", message);
							rc = m_pServer->Call("dompi_pi_set_io", message, strlen(message), &call_server_resp, 500);
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
							gminit.m_host = json_Direccion_IP->valuestring;
							gminit.m_port = 5533;

							m_pClient = new CGMClient(&gminit);

							rc = m_pClient->Call("dompi_pi_set_io", message, strlen(message), &call_client_resp, 500);
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
						if(atoi(json_Tipo_ASS->valuestring) == 0)
						{	/* Salida */
							if( atoi(json_Port->valuestring) == 1 )
							{	/* Puerto1 */
								if( atoi(json_Estado->valuestring) == 1 )
								{	/* Encender */
									rc = pD32W->SetIO(json_Direccion_IP->valuestring, 
														power2(atol(json_E_S->valuestring)-1),
														&return_int1);
									m_pServer->m_pLog->Add(100, "pD32W->SetIO(%s, 0x%02X, 0x%02X) = %i", 
														json_Direccion_IP->valuestring,
														power2(atol(json_E_S->valuestring)-1),
														return_int1,
														rc);
								}
								else
								{	/* Apagar */
									rc = pD32W->ResetIO(json_Direccion_IP->valuestring, 
														power2(atol(json_E_S->valuestring)-1),
														&return_int1);
									m_pServer->m_pLog->Add(100, "pD32W->ResetIO(%s, 0x%02X, 0x%02X) = %i", 
														json_Direccion_IP->valuestring,
														power2(atol(json_E_S->valuestring)-1),
														return_int1,
														rc);
								}
								if(rc == 0)
								{
									/* OK */
									sprintf(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"0x%02X\"}}", return_int1);
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
								}
							}
							else if( atoi(json_Port->valuestring) == 2 )
							{	/* Puerto2 */
								if( atoi(json_Estado->valuestring) == 1 )
								{	/* Encender */
									rc = pD32W->SetEX(json_Direccion_IP->valuestring, 
														power2(atol(json_E_S->valuestring)-1),
														&return_int1);
									m_pServer->m_pLog->Add(100, "pD32W->SetEX(%s, 0x%02X, 0x%02X) = %i", 
														json_Direccion_IP->valuestring,
														power2(atol(json_E_S->valuestring)-1),
														return_int1,
														rc);
								}
								else
								{	/* Apagar */
									rc = pD32W->ResetEX(json_Direccion_IP->valuestring, 
														power2(atol(json_E_S->valuestring)-1),
														&return_int1);
									m_pServer->m_pLog->Add(100, "pD32W->ResetEX(%s, 0x%02X, 0x%02X) = %i", 
														json_Direccion_IP->valuestring,
														power2(atol(json_E_S->valuestring)-1),
														return_int1,
														rc);
								}

								if(rc == 0)
								{
									/* OK */
									sprintf(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"0x%02X\"}}", return_int1);
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
								}
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid Port\"}}");
							}
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
				if(json_Direccion_IP && json_Tipo_HW && json_Tipo_ASS && json_Port && json_E_S )
				{
					if(atoi(json_Tipo_HW->valuestring) == 0)
					{
						if( (strlen(json_Direccion_IP->valuestring) == 0 ) || !strcmp(json_Direccion_IP->valuestring, "0.0.0.0") )
						{
							/* local */
							m_pServer->m_pLog->Add(50, "[dompi_pi_switch_io][%s]", message);
							rc = m_pServer->Call("dompi_pi_switch_io", message, strlen(message), &call_server_resp, 500);
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
							gminit.m_host = json_Direccion_IP->valuestring;
							gminit.m_port = 5533;

							m_pClient = new CGMClient(&gminit);

							rc = m_pClient->Call("dompi_pi_switch_io", message, strlen(message), &call_client_resp, 500);
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
						if(atoi(json_Tipo_ASS->valuestring) == 0)
						{	/* Salida */
							if( atoi(json_Port->valuestring) == 1 )
							{	/* Puerto1 */
								rc = pD32W->SwitchIO(json_Direccion_IP->valuestring, 
													power2(atol(json_E_S->valuestring)-1),
													&return_int1);
								m_pServer->m_pLog->Add(100, "pD32W->SwitchIO(%s, 0x%02X, 0x%02X) = %i", 
													json_Direccion_IP->valuestring,
													power2(atol(json_E_S->valuestring)-1),
													return_int1,
													rc);
								if(rc == 0)
								{
									/* OK */
									sprintf(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"0x%02X\"}}", return_int1);
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
								}
							}
							else if( atoi(json_Port->valuestring) == 2 )
							{	/* Puerto2 */
								rc = pD32W->SwitchEX(json_Direccion_IP->valuestring, 
													power2(atol(json_E_S->valuestring)-1),
													&return_int1);
								m_pServer->m_pLog->Add(100, "pD32W->SwitchEX(%s, 0x%02X, 0x%02X) = %i", 
													json_Direccion_IP->valuestring,
													power2(atol(json_E_S->valuestring)-1),
													return_int1,
													rc);
								if(rc == 0)
								{
									/* OK */
									sprintf(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"0x%02X\"}}", return_int1);
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
								}
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid Port\"}}");
							}
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
				if(json_Direccion_IP && json_Tipo_HW && json_Tipo_ASS && json_Port && json_E_S && json_Segundos )
				{
					if(atoi(json_Tipo_HW->valuestring) == 0)
					{
						if( (strlen(json_Direccion_IP->valuestring) == 0 ) || !strcmp(json_Direccion_IP->valuestring, "0.0.0.0") )
						{
							/* local */
							m_pServer->m_pLog->Add(50, "[dompi_pi_pulse_io][%s]", message);
							rc = m_pServer->Call("dompi_pi_pulse_io", message, strlen(message), &call_server_resp, 500);
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
							gminit.m_host = json_Direccion_IP->valuestring;
							gminit.m_port = 5533;

							m_pClient = new CGMClient(&gminit);

							rc = m_pClient->Call("dompi_pi_pulse_io", message, strlen(message), &call_client_resp, 500);
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
						if(atoi(json_Tipo_ASS->valuestring) == 0)
						{	/* Salida */
							if( atoi(json_Port->valuestring) == 1 )
							{	/* Puerto1 */
								rc = pD32W->PulseIO(json_Direccion_IP->valuestring, 
													power2(atol(json_E_S->valuestring)-1),
													atoi(json_Segundos->valuestring),
													&return_int1);
								m_pServer->m_pLog->Add(100, "pD32W->PulseIO(%s, 0x%02X, %i, 0x%02X) = %i", 
													json_Direccion_IP->valuestring,
													power2(atol(json_E_S->valuestring)-1),
													atoi(json_Segundos->valuestring),
													return_int1,
													rc);
								if(rc == 0)
								{
									/* OK */
									sprintf(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"0x%02X\"}}", return_int1);
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
								}
							}
							else if( atoi(json_Port->valuestring) == 2 )
							{	/* Puerto2 */
								rc = pD32W->PulseEX(json_Direccion_IP->valuestring, 
													power2(atol(json_E_S->valuestring)-1),
													atoi(json_Segundos->valuestring),
													&return_int1);
								m_pServer->m_pLog->Add(100, "pD32W->PulseEX(%s, 0x%02X, %i, 0x%02X) = %i", 
													json_Direccion_IP->valuestring,
													power2(atol(json_E_S->valuestring)-1),
													atoi(json_Segundos->valuestring),
													return_int1,
													rc);
								if(rc == 0)
								{
									/* OK */
									sprintf(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"0x%02X\"}}", return_int1);
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
								}
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid Port\"}}");
							}
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
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_get_io"))
			{
				if(json_Direccion_IP && json_Tipo_HW && json_Tipo_ASS && json_Port && json_E_S )
				{
					if(atoi(json_Tipo_HW->valuestring) == 0)
					{
						if( (strlen(json_Direccion_IP->valuestring) == 0 ) || !strcmp(json_Direccion_IP->valuestring, "0.0.0.0") )
						{
							/* local */
							m_pServer->m_pLog->Add(50, "[dompi_pi_get_io][%s]", message);
							rc = m_pServer->Call("dompi_pi_get_io", message, strlen(message), &call_server_resp, 500);
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
							gminit.m_host = json_Direccion_IP->valuestring;
							gminit.m_port = 5533;

							m_pClient = new CGMClient(&gminit);

							rc = m_pClient->Call("dompi_pi_get_io", message, strlen(message), &call_client_resp, 500);
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
						if( atoi(json_Port->valuestring) == 1 )
						{	/* Puerto1 */
							rc = pD32W->GetIOStatus(json_Direccion_IP->valuestring, 
												&return_int1);
							m_pServer->m_pLog->Add(100, "pD32W->GetIOStatus(%s, 0x%02X) = %i", 
												json_Direccion_IP->valuestring,
												return_int1,
												rc);
							if(rc == 0)
							{
								/* OK */
								sprintf(message,
										"{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"%i\"}}",
										(return_int1&power2(atol(json_E_S->valuestring)-1)?1:0));
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
							}
						}
						else if( atoi(json_Port->valuestring) == 2 )
						{	/* Puerto2 */
							rc = pD32W->GetEXStatus(json_Direccion_IP->valuestring, 
												&return_int1);
							m_pServer->m_pLog->Add(100, "pD32W->GetIOStatus(%s, 0x%02X) = %i", 
												json_Direccion_IP->valuestring,
												return_int1,
												rc);
							if(rc == 0)
							{
								/* OK */
								sprintf(message,
										"{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"%i\"}}",
										(return_int1&power2(atol(json_E_S->valuestring)-1)?1:0));
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error on Send\"}}");
							}
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid Port\"}}");
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
					m_pServer->m_pLog->Add(10, "ERROR al responder mensaje [dompi_hw_get_io]");
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
	m_pServer->UnSuscribe("dompi_hw_get_io", GM_MSG_TYPE_CR);


	delete m_pServer;
	exit(0);
}
