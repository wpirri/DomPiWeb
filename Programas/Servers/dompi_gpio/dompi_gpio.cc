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

#include <string>
#include <iostream>
#include <csignal>
using namespace std;

#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>

#include "dom32iopi.h"

CGMServerWait *m_pServer;
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
	unsigned char blink;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGKILL,         OnClose);
	signal(SIGTERM,         OnClose);
	signal(SIGSTOP,         OnClose);
	signal(SIGABRT,         OnClose);
	signal(SIGQUIT,         OnClose);
	signal(SIGINT,          OnClose);
	signal(SIGILL,          OnClose);
	signal(SIGFPE,          OnClose);
	signal(SIGSEGV,         OnClose);
	signal(SIGBUS,          OnClose);

	m_pServer = new CGMServerWait;
	m_pServer->Init("dompi_gpio");
	m_pServer->m_pLog->Add(1, "Iniciando interface GPIO");

    cJSON *json_req;
    //cJSON *json_resp;
	cJSON *json_Direccion_IP;
	cJSON *json_Tipo_HW;
	cJSON *json_Tipo_ASS;
	cJSON *json_Port;
	cJSON *json_E_S;
	cJSON *json_Estado;
	cJSON *json_IO_Config;
	cJSON *json_AN_Config;
	cJSON *json_Segundos;

	int return_int1;
	int return_int2;

    Dom32IoPi *pPI;
    //Dom32IoPi::pi_config_data pi_data;
    pPI = new Dom32IoPi();

	m_pServer->Suscribe("dompi_pi_set_port_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_get_port_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_set_comm_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_get_comm_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_set_port", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_get_port", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_set_io", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_switch_io", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_pulse_io", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_get_io", GM_MSG_TYPE_CR);

	blink = 0;
	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 10 )) >= 0)
	{
		json_req = NULL;
		//json_resp = NULL;
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
			if( !strcmp(fn, "dompi_pi_set_port_config"))
			{
				if(json_Direccion_IP && json_Tipo_HW && json_Port )
				{
					if(atoi(json_Tipo_HW->valuestring) == 1)
					{
						if(json_IO_Config)
						{
							if( atoi(json_Port->valuestring) == 1 )
							{
								rc = pPI->ConfigIO(
												atol(json_IO_Config->valuestring),
												NULL);
								m_pServer->m_pLog->Add(100, "pPI->ConfigIO(%s, %s) = %i", 
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
								rc = pPI->ConfigEX(
												atol(json_IO_Config->valuestring),
												NULL);
								m_pServer->m_pLog->Add(100, "pPI->ConfigEX(%s, %s) = %i", 
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
					if(atoi(json_Tipo_HW->valuestring) == 1)
					{
						if( !json_Port)
						{
							rc = pPI->GetConfig(&return_int1, &return_int2);
							m_pServer->m_pLog->Add(100, "pPI->GetConfig(%s, ...) = %i", 
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
							rc = pPI->GetConfig(&return_int1, NULL);
							m_pServer->m_pLog->Add(100, "pPI->GetConfig(%s, ...) = %i", 
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
							rc = pPI->GetConfig(NULL, &return_int1);
							m_pServer->m_pLog->Add(100, "pPI->GetConfig(%s, ...) = %i", 
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
					if(atoi(json_Tipo_HW->valuestring) == 1)
					{	/* Interface Vía IP con dos puertos */
						if(json_Port)
						{
							if( atoi(json_Port->valuestring) == 1 )
							{	/* Puerto1 */
								rc = pPI->GetIOStatus(
													&return_int1);
								m_pServer->m_pLog->Add(100, "pPI->GetIOStatus(%s, 0x%02X) = %i", 
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
								rc = pPI->GetEXStatus(
													&return_int1);
								m_pServer->m_pLog->Add(100, "pPI->GetIEXtatus(%s, 0x%02X) = %i", 
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
							rc = pPI->GetIOStatus(
												&return_int1);
							m_pServer->m_pLog->Add(100, "pPI->GetIOStatus(%s, 0x%02X) = %i", 
												json_Direccion_IP->valuestring,
												return_int1,
												rc);
							rc |= pPI->GetEXStatus(
												&return_int2);
							m_pServer->m_pLog->Add(100, "pPI->GetIOStatus(%s, 0x%02X) = %i", 
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
					if(atoi(json_Tipo_HW->valuestring) == 1)
					{	/* Interface Vía IP con dos puertos */
						if(atoi(json_Tipo_ASS->valuestring) == 0)
						{	/* Salida */
							if( atoi(json_Port->valuestring) == 1 )
							{	/* Puerto1 */
								if( atoi(json_Estado->valuestring) == 1 )
								{	/* Encender */
									rc = pPI->SetIO(
														power2(atol(json_E_S->valuestring)-1),
														&return_int1);
									m_pServer->m_pLog->Add(100, "pPI->SetIO(%s, 0x%02X, 0x%02X) = %i", 
														json_Direccion_IP->valuestring,
														power2(atol(json_E_S->valuestring)-1),
														return_int1,
														rc);
								}
								else
								{	/* Apagar */
									rc = pPI->ResetIO(
														power2(atol(json_E_S->valuestring)-1),
														&return_int1);
									m_pServer->m_pLog->Add(100, "pPI->ResetIO(%s, 0x%02X, 0x%02X) = %i", 
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
									rc = pPI->SetEX(
														power2(atol(json_E_S->valuestring)-1),
														&return_int1);
									m_pServer->m_pLog->Add(100, "pPI->SetEX(%s, 0x%02X, 0x%02X) = %i", 
														json_Direccion_IP->valuestring,
														power2(atol(json_E_S->valuestring)-1),
														return_int1,
														rc);
								}
								else
								{	/* Apagar */
									rc = pPI->ResetEX(
														power2(atol(json_E_S->valuestring)-1),
														&return_int1);
									m_pServer->m_pLog->Add(100, "pPI->ResetEX(%s, 0x%02X, 0x%02X) = %i", 
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
					if(atoi(json_Tipo_HW->valuestring) == 1)
					{	/* Interface Vía IP con dos puertos */
						if(atoi(json_Tipo_ASS->valuestring) == 0)
						{	/* Salida */
							if( atoi(json_Port->valuestring) == 1 )
							{	/* Puerto1 */
								rc = pPI->SwitchIO(
													power2(atol(json_E_S->valuestring)-1),
													&return_int1);
								m_pServer->m_pLog->Add(100, "pPI->SwitchIO(%s, 0x%02X, 0x%02X) = %i", 
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
								rc = pPI->SwitchEX(
													power2(atol(json_E_S->valuestring)-1),
													&return_int1);
								m_pServer->m_pLog->Add(100, "pPI->SwitchEX(%s, 0x%02X, 0x%02X) = %i", 
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
					if(atoi(json_Tipo_HW->valuestring) == 1)
					{	/* Interface Vía IP con dos puertos */
						if(atoi(json_Tipo_ASS->valuestring) == 0)
						{	/* Salida */
							if( atoi(json_Port->valuestring) == 1 )
							{	/* Puerto1 */
								rc = pPI->PulseIO(
													power2(atol(json_E_S->valuestring)-1),
													atoi(json_Segundos->valuestring),
													&return_int1);
								m_pServer->m_pLog->Add(100, "pPI->PulseIO(%s, 0x%02X, %i, 0x%02X) = %i", 
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
								rc = pPI->PulseEX(
													power2(atol(json_E_S->valuestring)-1),
													atoi(json_Segundos->valuestring),
													&return_int1);
								m_pServer->m_pLog->Add(100, "pPI->PulseEX(%s, 0x%02X, %i, 0x%02X) = %i", 
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
					if(atoi(json_Tipo_HW->valuestring) == 1)
					{	/* Interface Vía IP con dos puertos */
						if( atoi(json_Port->valuestring) == 1 )
						{	/* Puerto1 */
							rc = pPI->GetIOStatus(
												&return_int1);
							m_pServer->m_pLog->Add(100, "pPI->GetIOStatus(%s, 0x%02X) = %i", 
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
							rc = pPI->GetEXStatus(
												&return_int1);
							m_pServer->m_pLog->Add(100, "pPI->GetIOStatus(%s, 0x%02X) = %i", 
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
			blink++;
			pPI->SetStatusLed(blink&0x01);
		}
		
	}
	m_pServer->m_pLog->Add(50, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);
	m_pServer->UnSuscribe("dompi_pi_set_port_config", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_get_port_config", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_set_comm_config", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_get_comm_config", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_set_port", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_get_port", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_set_io", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_switch_io", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_pulse_io", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_get_io", GM_MSG_TYPE_CR);
	delete m_pServer;
	exit(0);
}
