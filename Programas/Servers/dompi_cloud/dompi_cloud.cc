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
#include "cdb.h"

#define MAX_BUFFER_LEN 32767

CGMServerWait *m_pServer;
DPConfig *pConfig;
CDB *pDB;
cJSON *json_System_Config;
int internal_timeout;
int external_timeout;

char m_SystemKey[256];
char m_CloudHost1Address[64];
int  m_CloudHost1Port;
char m_CloudHost1Proto[8];
char m_CloudHost2Address[64];
int  m_CloudHost2Port;
char m_CloudHost2Proto[8];
time_t update_ass_t;

void OnClose(int sig);
void UpdateCloud( void );
int DompiCloud_Notificar(const char* host, int port, const char* proto, const char* send_msg, char* receive_msg);
void LoadSystemConfig(void);

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[MAX_BUFFER_LEN+1];
	char db_host[32];
	char db_user[32];
	char db_password[32];
	unsigned long message_len;
	char s[16];

	cJSON *json_obj;
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
	update_ass_t = 0;

	m_pServer = new CGMServerWait;
	m_pServer->Init("dompi_cloud");
	m_pServer->m_pLog->Add(1, "Iniciando interface con la nube...");

	pConfig = new DPConfig("/etc/dompiweb.config");

	//pConfig->GetParam("SQLITE_DB_FILENAME", db_filename);
	pConfig->GetParam("DBHOST", db_host);
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

	//m_pServer->m_pLog->Add(10, "Conectando a la base de datos %s...", db_filename);
	//pDB = new CDB(db_filename);
	m_pServer->m_pLog->Add(10, "Conectado a la base de datos DB_DOMPIWEB...");
	pDB = new CDB(db_host, "DB_DOMPIWEB", db_user, db_password);
	if(pDB->Open() != 0)
	{
		m_pServer->m_pLog->Add(1, "ERROR al conectar con la base de datos");
		OnClose(0);
	}
	else
	{
		//m_pServer->m_pLog->Add(10, "Conectado a la base de datos %s", db_filename);
		m_pServer->m_pLog->Add(10, "Conectado a la base de datos DB_DOMPIWEB");
	}

	json_System_Config = NULL;
	LoadSystemConfig();

	m_pServer->Suscribe("dompi_ass_change", GM_MSG_TYPE_NOT);	  		/* Sin respuesta, lo atiende el mas libre */
	m_pServer->Suscribe("dompi_reload_config", GM_MSG_TYPE_MSG);		/* Sin respuesta, llega a todos */

	m_pServer->m_pLog->Add(1, "Interface con la nube inicializada.");

	while((rc = m_pServer->Wait(fn, typ, message, MAX_BUFFER_LEN, &message_len, 250 )) >= 0)
	{
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(50, "%s:(Q)[%s]", fn, message);

			/* ************************************************************* *
			 *
			 * ************************************************************* */
			if( !strcmp(fn, "dompi_ass_change")) /* Tipo NOT, no se responde */
			{
				m_pServer->Resp(NULL, 0, GME_OK);

				json_obj = cJSON_Parse(message);
				message[0] = 0;

				cJSON_AddStringToObject(json_obj, "System_Key", m_SystemKey);

				cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
				cJSON_Delete(json_obj);
				m_pServer->m_pLog->Add(100, "[CLOUD] << [%s]", message);
				if(m_CloudHost1Address[0])
				{
					rc = DompiCloud_Notificar(m_CloudHost1Address, m_CloudHost1Port, m_CloudHost1Proto, message, message);
				}
				else if(m_CloudHost2Address[0])
				{
					rc = DompiCloud_Notificar(m_CloudHost2Address, m_CloudHost2Port, m_CloudHost2Proto, message, message);
				}
				else
				{
					m_pServer->m_pLog->Add(1, "[dompi_ass_change] No hay servidores definodos para reporte a la nube");
					rc = (-1);
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
						m_pServer->Notify("dompi_cloud_notification", message, strlen(message));
					}
				}
			}
			/* ****************************************************************
			*		dompi_reload_config
			**************************************************************** */
			else if( !strcmp(fn, "dompi_reload_config")) /* Tipo MSG, no se responde */
			{
				m_pServer->Resp(NULL, 0, GME_OK);
				
				LoadSystemConfig();
			}





			else
			{
				m_pServer->m_pLog->Add(50, "GME_SVC_NOTFOUND");
				m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
			}
		}
		else
		{
			/* al expirar el timer */
			if(m_CloudHost1Address[0] || m_CloudHost2Address[0])
			{
				json_obj = cJSON_CreateObject();

				cJSON_AddStringToObject(json_obj, "System_Key", m_SystemKey);

				/* Si hay que agragar mas objetos */

				cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
				cJSON_Delete(json_obj);
				m_pServer->m_pLog->Add(100, "[CLOUD] << [%s]", message);
				rc = (-1);
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
						m_pServer->Notify("dompi_cloud_notification", message, strlen(message));
					}
				}
			}
			else
			{
				m_pServer->m_pLog->Add(1, "No hay servidores definodos para reporte a la nube");
			}
		}
		/* Después de recibir un mensaje o expirar el timer */


		UpdateCloud();



	}
	m_pServer->m_pLog->Add(50, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);

	m_pServer->UnSuscribe("dompi_ass_change", GM_MSG_TYPE_NOT);
	m_pServer->UnSuscribe("dompi_reload_config", GM_MSG_TYPE_MSG);

	delete m_pServer;
	exit(0);
}

void UpdateCloud( void )
{
	char query[4096];
	char message[4096];
	int rc;
	cJSON *json_QueryArray;
	cJSON *json_QueryRow;
	cJSON *json_Message;
	cJSON *json_Resp;
	time_t t;

	t = time(&t);

	if(t >= update_ass_t)
	{
		/* Actualizacion de objetos en la nube cada 10 min */
		update_ass_t = t + 600;

		json_QueryArray = cJSON_CreateArray();

		/* Genero un listado de los objetos con su estado para subir a la nube */
		strcpy(query, "SELECT * FROM TB_DOM_ASSIGN WHERE Id > 0;");
		m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
		rc = pDB->Query(json_QueryArray, query);
		m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
		if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
		if(rc >= 0)
		{
			cJSON_ArrayForEach(json_QueryRow, json_QueryArray)
			{
				cJSON_AddStringToObject(json_QueryRow, "System_Key", m_SystemKey);
				cJSON_PrintPreallocated(json_QueryRow, message, MAX_BUFFER_LEN, 0);

				m_pServer->m_pLog->Add(100, "[CLOUD] << [%s]", message);
				if(m_CloudHost1Address[0])
				{
					rc = DompiCloud_Notificar(m_CloudHost1Address, m_CloudHost1Port, m_CloudHost1Proto, message, message);
				}
				else if(m_CloudHost2Address[0])
				{
					rc = DompiCloud_Notificar(m_CloudHost2Address, m_CloudHost2Port, m_CloudHost2Proto, message, message);
				}
				else
				{
					m_pServer->m_pLog->Add(1, "[dompi_ass_status_update] No hay servidores definodos para reporte a la nube");
					rc = (-1);
				}

				if( rc > 0 && strlen(message) )
				{
					/* La respuesta de la nube puede venir con un array de acciones luego de la cabecera HTTP*/
					m_pServer->m_pLog->Add(100, "[CLOUD] >> [%s]", message);
					json_Message = cJSON_Parse(message);
					json_Resp = cJSON_GetObjectItemCaseSensitive(json_Message, "response");
					if(json_Resp && cJSON_IsArray(json_Resp))
					{
						cJSON_PrintPreallocated(json_Resp, message, MAX_BUFFER_LEN, 0);
						m_pServer->m_pLog->Add(50, "[dompi_cloud_notification][%s]", message);
						m_pServer->Notify("dompi_cloud_notification", message, strlen(message));
					}
					cJSON_Delete(json_Message);
				}

			}
		}
		cJSON_Delete(json_QueryArray);
	}
}

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

				/* Está viniendo algo raro al princiìo de la parte de datos que no la puedo sacar */
				while(*ps && *ps != '{') ps++;

				strcpy(receive_msg, ps);
			}
		}
	}
	return rc;
}

void LoadSystemConfig(void)
{
	char query[4096];
	int rc;
	cJSON *json_Config;
	cJSON *json_Obj;

	if(pDB == NULL) return;

	if(json_System_Config) cJSON_Delete(json_System_Config);
	json_System_Config = cJSON_CreateArray();
	strcpy(query, "SELECT * FROM TB_DOM_CONFIG ORDER BY Id DESC LIMIT 1;");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_System_Config, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
	if(rc < 0)
	{
		m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
		return;
	}

	cJSON_ArrayForEach(json_Config, json_System_Config){ break; }

	if((json_Obj = cJSON_GetObjectItemCaseSensitive(json_Config, "System_Key")) != NULL)
	{
		strcpy(m_SystemKey, json_Obj->valuestring);
	}
	if((json_Obj = cJSON_GetObjectItemCaseSensitive(json_Config, "Cloud_Host_1_Address")) != NULL)
	{
		strcpy(m_CloudHost1Address, json_Obj->valuestring);
	}
	if((json_Obj = cJSON_GetObjectItemCaseSensitive(json_Config, "Cloud_Host_1_Port")) != NULL)
	{
		m_CloudHost1Port = atoi(json_Obj->valuestring);
	}
	if((json_Obj = cJSON_GetObjectItemCaseSensitive(json_Config, "Cloud_Host_1_Proto")) != NULL)
	{
		strcpy(m_CloudHost1Proto, json_Obj->valuestring);
	}
	if((json_Obj = cJSON_GetObjectItemCaseSensitive(json_Config, "Cloud_Host_2_Address")) != NULL)
	{
		strcpy(m_CloudHost2Address, json_Obj->valuestring);
	}
	if((json_Obj = cJSON_GetObjectItemCaseSensitive(json_Config, "Cloud_Host_2_Port")) != NULL)
	{
		m_CloudHost2Port = atoi(json_Obj->valuestring);
	}
	if((json_Obj = cJSON_GetObjectItemCaseSensitive(json_Config, "Cloud_Host_2_Proto")) != NULL)
	{
		strcpy(m_CloudHost2Proto, json_Obj->valuestring);
	}

	if(rc >= 0) m_pServer->m_pLog->Add(1, "[LoadSystemConfig] Lectura de configuracion OK.");
}
