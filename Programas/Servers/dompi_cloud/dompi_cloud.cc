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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <cjson/cJSON.h>

#include "ctcp.h"
#include "config.h"

#define MAX_BUFFER_LEN 32767

int internal_timeout;
int external_timeout;

CGMServerWait *m_pServer;
DPConfig *pConfig;
void OnClose(int sig);

char m_SystemKey[256];
char m_CloudHost1Address[64];
int  m_CloudHost1Port;
char m_CloudHost1Proto[8];
char m_CloudHost2Address[64];
int  m_CloudHost2Port;
char m_CloudHost2Proto[8];

int DompiCloud_Notificar(const char* host, int port, const char* proto, const char* send_msg, char* receive_msg)
{
    /* POST
    * 1.- %s: URI
    * 2.- %s: Host
    * 3.- %d: Content-Length
    * 4.- %s: datos
    */
    char http_post[] =     "POST %s HTTP/1.1\r\n"
                    "Host: %s\r\n"
                    "Connection: keep-alive\r\n"
                    "Content-Length: %d\r\n"
                    "User-Agent: DomPiSrv/1.00 (RaspBerryPi;Dom32)\r\n"
                    "Accept: text/html,text/xml\r\n"
                    "Content-Type: application/x-www-form-urlencoded\r\n\r\n%s";

    /*
    * GET
    * 1.- %s: URI
    * 2.- %s: Host
    */
    //char http_get[] =     "GET %s HTTP/1.1\r\n"
    //                    "Host: %s\r\n"
    //                    "Connection: close\r\n"
    //                    "User-Agent: DomPiSrv/1.00 (RaspBerryPi;Dom32)\r\n"
    //                    "Accept: text/html,text/xml\r\n\r\n";

    char url_default[] = "/cgi-bin/dompi_cloud_notif.cgi";

	CTcp *pSock;
	char buffer[MAX_BUFFER_LEN+1];
	char *ps;
	int rc = 0;

	buffer[0] = 0;
	if( proto && !strcmp(proto, "https"))
	{
		pSock = new CTcp(1, 3);
		if(port == 0) port = 443;
    		sprintf(buffer, http_post, url_default, host, strlen(send_msg), send_msg);
		rc =pSock->Query(host, port, buffer, buffer, MAX_BUFFER_LEN, external_timeout);
		delete pSock;
	}
	else
	{
		pSock = new CTcp();
		if(port == 0) port = 80;
    		sprintf(buffer, http_post, url_default, host, strlen(send_msg), send_msg);
		rc = pSock->Query(host, port, buffer, buffer, MAX_BUFFER_LEN, external_timeout);
		delete pSock;
	}
	if(receive_msg)
	{
		*receive_msg = 0;
		if(rc > 0 && strlen(buffer))
		{
			ps = strstr(buffer, "\r\n\r\n");
			if(ps)
			{
				ps += 4;
				strcpy(receive_msg, ps);
			}
		}
	}
	return rc;
}

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[MAX_BUFFER_LEN+1];
	unsigned long message_len;
	char s[16];
	//CGMServerBase::GMIOS call_resp;

	//char str[256];
	cJSON *json_obj;
	cJSON *json_un_obj;
	//cJSON *json_resp_code;
	//cJSON *json_resp_msg;
	cJSON *json_arr;

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

	m_CloudHost1Address[0] = 0;
	m_CloudHost2Address[0] = 0;

	m_pServer = new CGMServerWait;
	m_pServer->Init("dompi_cloud");
	m_pServer->m_pLog->Add(1, "Iniciando interface CLOUD");

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

	m_pServer->Suscribe("dompi_cloud_config", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_change", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_ass_status_update", GM_MSG_TYPE_MSG);

	while((rc = m_pServer->Wait(fn, typ, message, MAX_BUFFER_LEN, &message_len, 3000 )) >= 0)
	{
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(50, "%s:(Q)[%s]", fn, message);

			/* ************************************************************* *
			 *
			 * ************************************************************* */
			if( !strcmp(fn, "dompi_cloud_config"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				if((json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "System_Key")) != NULL)
				{
					strcpy(m_SystemKey, json_un_obj->valuestring);
				}
				if((json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Cloud_Host_1_Address")) != NULL)
				{
					strcpy(m_CloudHost1Address, json_un_obj->valuestring);
				}
				if((json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Cloud_Host_1_Port")) != NULL)
				{
					m_CloudHost1Port = atoi(json_un_obj->valuestring);
				}
				if((json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Cloud_Host_1_Proto")) != NULL)
				{
					strcpy(m_CloudHost1Proto, json_un_obj->valuestring);
				}
				if((json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Cloud_Host_2_Address")) != NULL)
				{
					strcpy(m_CloudHost2Address, json_un_obj->valuestring);
				}
				if((json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Cloud_Host_2_Port")) != NULL)
				{
					m_CloudHost2Port = atoi(json_un_obj->valuestring);
				}
				if((json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Cloud_Host_2_Proto")) != NULL)
				{
					strcpy(m_CloudHost2Proto, json_un_obj->valuestring);
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(10, "ERROR al responder mensaje [dompi_hw_get_port_config]");
				}
			}
			else if( !strcmp(fn, "dompi_ass_change")) /* typo MSG, no se responde */
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				cJSON_AddStringToObject(json_obj, "System_Key", m_SystemKey);

				cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
				cJSON_Delete(json_obj);
				rc = 0;
				if(m_CloudHost1Address[0])
				{
					rc = DompiCloud_Notificar(m_CloudHost1Address, m_CloudHost1Port, m_CloudHost1Proto, message, message);
				}
				else /*if(m_CloudHost2Address[0])*/
				{
					rc = DompiCloud_Notificar(m_CloudHost2Address, m_CloudHost2Port, m_CloudHost2Proto, message, message);
				}

				if( rc > 0 && strlen(message) )
				{
					/* La respuesta de la nube puede venir con un array de acciones luego de la cabecera HTTP*/
					m_pServer->m_pLog->Add(100, "[CLOUD] >> [%s]", message);
					json_obj = cJSON_Parse(message);
					//json_resp_code = cJSON_GetObjectItemCaseSensitive(json_obj, "resp_code");
					//json_resp_msg = cJSON_GetObjectItemCaseSensitive(json_obj, "resp_msg");
					json_arr = cJSON_GetObjectItemCaseSensitive(json_obj, "response");
					if(json_arr && cJSON_IsArray(json_arr))
					{
						cJSON_PrintPreallocated(json_arr, message, MAX_BUFFER_LEN, 0);
						m_pServer->m_pLog->Add(50, "[dompi_cloud_notification][%s]", message);
						m_pServer->Post("dompi_cloud_notification", message, strlen(message));
					}
					strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				}
			}
			else if( !strcmp(fn, "dompi_ass_status_update")) /* typo MSG, no se responde */
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				cJSON_AddStringToObject(json_obj, "System_Key", m_SystemKey);

				cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
				cJSON_Delete(json_obj);
				rc = 0;
				if(m_CloudHost1Address[0])
				{
					rc = DompiCloud_Notificar(m_CloudHost1Address, m_CloudHost1Port, m_CloudHost1Proto, message, message);
				}
				else /*if(m_CloudHost2Address[0])*/
				{
					rc = DompiCloud_Notificar(m_CloudHost2Address, m_CloudHost2Port, m_CloudHost2Proto, message, message);
				}

				if( rc > 0 && strlen(message) )
				{
					/* La respuesta de la nube puede venir con un array de acciones luego de la cabecera HTTP*/
					m_pServer->m_pLog->Add(100, "[CLOUD] >> [%s]", message);
					json_obj = cJSON_Parse(message);
					//json_resp_code = cJSON_GetObjectItemCaseSensitive(json_obj, "resp_code");
					//json_resp_msg = cJSON_GetObjectItemCaseSensitive(json_obj, "resp_msg");
					json_arr = cJSON_GetObjectItemCaseSensitive(json_obj, "response");
					if(json_arr && cJSON_IsArray(json_arr))
					{
						cJSON_PrintPreallocated(json_arr, message, MAX_BUFFER_LEN, 0);
						m_pServer->m_pLog->Add(50, "[dompi_cloud_notification][%s]", message);
						m_pServer->Post("dompi_cloud_notification", message, strlen(message));
					}
					strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				}
			}


			else
			{
				m_pServer->m_pLog->Add(50, "GME_SVC_NOTFOUND");
				m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
			}
		}
		/* Después de un mensaje o al expirar el timer */
		if(m_CloudHost1Address[0] || m_CloudHost2Address[0])
		{
			json_obj = cJSON_CreateObject();

			cJSON_AddStringToObject(json_obj, "System_Key", m_SystemKey);

			/* Si hay que agragar mas objetos */

			cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
			cJSON_Delete(json_obj);
			m_pServer->m_pLog->Add(100, "[CLOUD] << [%s]", message);
			rc = 0;
			if(m_CloudHost1Address[0])
			{
				rc = DompiCloud_Notificar(m_CloudHost1Address, m_CloudHost1Port, m_CloudHost1Proto, message, message);
			}
			else /*if(m_CloudHost2Address[0])*/
			{
				rc = DompiCloud_Notificar(m_CloudHost2Address, m_CloudHost2Port, m_CloudHost2Proto, message, message);
			}

			if( rc > 0 && strlen(message) )
			{
				/* La respuesta de la nube puede venir con un array de acciones luego de la cabecera HTTP*/
				m_pServer->m_pLog->Add(100, "[CLOUD] >> [%s]", message);
				json_obj = cJSON_Parse(message);
				//json_resp_code = cJSON_GetObjectItemCaseSensitive(json_obj, "resp_code");
				//json_resp_msg = cJSON_GetObjectItemCaseSensitive(json_obj, "resp_msg");
				json_arr = cJSON_GetObjectItemCaseSensitive(json_obj, "response");
				if(json_arr && cJSON_IsArray(json_arr))
				{
					cJSON_PrintPreallocated(json_arr, message, MAX_BUFFER_LEN, 0);
					m_pServer->m_pLog->Add(50, "[dompi_cloud_notification][%s]", message);
					m_pServer->Post("dompi_cloud_notification", message, strlen(message));
				}
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
			}
		}
	}
	m_pServer->m_pLog->Add(50, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);

	m_pServer->UnSuscribe("dompi_cloud_config", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_ass_status_update", GM_MSG_TYPE_MSG);


	delete m_pServer;
	exit(0);
}
