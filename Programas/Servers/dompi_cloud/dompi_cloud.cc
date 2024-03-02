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
#include <gmonitor/svcstru.h>

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
#include "gevent.h"

CGMServerWait *m_pServer;
DPConfig *pConfig;
int internal_timeout;
int external_timeout;
CDB *pDB;
GEvent *pEV;
cJSON *json_System_Config;

char m_SystemKey[256];
char m_CloudHost1Address[64];
int  m_CloudHost1Port;
char m_CloudHost1Proto[8];
char m_CloudHost2Address[64];
int  m_CloudHost2Port;
char m_CloudHost2Proto[8];
time_t update_ass_t;
time_t update_alarm_t;
time_t update_user_t;

int m_cloud_status;
char *m_host_actual;
int m_port_actual;
char *m_proto_actual;

void OnClose(int sig);
int KeepAliveCloud( void );
int SendToCloud( void );
int DompiCloud_Notificar(const char* host, int port, const char* proto, const char* send_msg, char* receive_msg);
void LoadSystemConfig(void);
void AddSaf( void );

void CheckUpdateAssignCloud( void );
void CheckUpdateAlarmCloud( void );
void CheckUpdateUserCloud( void );

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[GM_COMM_MSG_LEN+1];
	char db_host[32];
	char db_name[32];
	char db_user[32];
	char db_password[32];
	unsigned long message_len;
	char s[16];
	int wait;

	cJSON *json_Message;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGKILL, OnClose);
	signal(SIGTERM, OnClose);
	/* Dejo de capturar interrupciones para permitir Core Dumps */
	//signal(SIGSTOP, OnClose);
	//signal(SIGABRT, OnClose);
	//signal(SIGQUIT, OnClose);
	//signal(SIGINT,  OnClose);
	//signal(SIGILL,  OnClose);
	//signal(SIGFPE,  OnClose);
	//signal(SIGSEGV, OnClose);
	//signal(SIGBUS,  OnClose);

	m_CloudHost1Address[0] = 0;
	m_CloudHost2Address[0] = 0;
	update_ass_t = 0;
	update_alarm_t = 0;
	update_user_t = 0;
	m_cloud_status = 0;

	m_pServer = new CGMServerWait;
	m_pServer->Init("dompi_cloud");
	m_pServer->m_pLog->Add(1, "Iniciando interface con la nube...");

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

	json_System_Config = NULL;
	LoadSystemConfig();

	pEV = new GEvent(pDB, m_pServer);

	m_pServer->Suscribe("dompi_ass_change", GM_MSG_TYPE_NOT);	  		/* Sin respuesta, lo atiende el mas libre */
	m_pServer->Suscribe("dompi_user_change", GM_MSG_TYPE_NOT);	  		/* Sin respuesta, lo atiende el mas libre */
	m_pServer->Suscribe("dompi_alarm_change", GM_MSG_TYPE_NOT);	  		/* Sin respuesta, lo atiende el mas libre */
	m_pServer->Suscribe("dompi_reload_config", GM_MSG_TYPE_MSG);		/* Sin respuesta, llega a todos */

	AddSaf();

	m_pServer->m_pLog->Add(1, "Interface con la nube inicializada.");

	wait = 250;
	while((rc = m_pServer->Wait(fn, typ, message, GM_COMM_MSG_LEN, &message_len, wait )) >= 0)
	{
		wait = 250;
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(50, "%s:(Q)[%s]", fn, message);

			/* ************************************************************* *
			 *
			 * ************************************************************* */
			if( !strcmp(fn, "dompi_ass_change")) /* Tipo NOT */
			{
				//m_pServer->Resp(NULL, 0, GME_OK);

				json_Message = cJSON_Parse(message);
				message[0] = 0;

				m_pServer->m_pLog->Add(20, "[dompi_ass_change] Encolando actualizacion con datos de assign");
				if(m_cloud_status)
				{
					cJSON_AddStringToObject(json_Message, "System_Key", m_SystemKey);
					cJSON_PrintPreallocated(json_Message, message, GM_COMM_MSG_LEN, 0);
					if(m_pServer->Enqueue("dompi_msg_to_cloud", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_ass_change] ERROR: Encolando en SAF dompi_msg_to_cloud [%s]", message);
					}
				}
				else
				{
					m_pServer->m_pLog->Add(1, "[dompi_ass_change] OFFLINE: Encolando actualizacion con datos de assign [%s]", message);
				}
				cJSON_Delete(json_Message);
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_user_change")) /* Tipo NOT */
			{
				//m_pServer->Resp(NULL, 0, GME_OK);
				update_user_t = 0;
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_alarm_change")) /* Tipo NOT */
			{
				//m_pServer->Resp(NULL, 0, GME_OK);
				update_alarm_t = 0;

			}
			/* ****************************************************************
			*		dompi_reload_config
			**************************************************************** */
			else if( !strcmp(fn, "dompi_reload_config")) /* Tipo MSG */
			{
				//m_pServer->Resp(NULL, 0, GME_OK);
				
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


		}
		/* Después de recibir un mensaje o expirar el timer */
		CheckUpdateAssignCloud();
		CheckUpdateAlarmCloud();
		CheckUpdateUserCloud();

		if(SendToCloud())
		{
			wait = 1;
		}
		else
		{
			if(KeepAliveCloud())
			{
				wait = 25;
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

	m_pServer->UnSuscribe("dompi_ass_change", GM_MSG_TYPE_NOT);
	m_pServer->UnSuscribe("dompi_user_change", GM_MSG_TYPE_NOT);
	m_pServer->UnSuscribe("dompi_alarm_change", GM_MSG_TYPE_NOT);
	m_pServer->UnSuscribe("dompi_reload_config", GM_MSG_TYPE_MSG);

	delete m_pServer;
	delete pEV;
	delete pConfig;
	delete pDB;

	exit(0);
}

int KeepAliveCloud( void )
{
	int rc;
	char message[GM_COMM_MSG_LEN+1];
	cJSON *json_Message;
	cJSON *json_arr;

	if(m_host_actual)
	{
		json_Message = cJSON_CreateObject();

		cJSON_AddStringToObject(json_Message, "System_Key", m_SystemKey);

		/* Si hay que agragar mas objetos */

		cJSON_PrintPreallocated(json_Message, message, GM_COMM_MSG_LEN, 0);
		cJSON_Delete(json_Message);

		m_pServer->m_pLog->Add(100, "[CLOUD] << [%s]", message);

		if(m_cloud_status == 0)
		{
			/* Switch de host */
			if(m_host_actual == &m_CloudHost1Address[0] && m_CloudHost2Address[0])
			{
				m_host_actual = &m_CloudHost2Address[0];
				m_port_actual = m_CloudHost2Port;
				m_proto_actual = &m_CloudHost2Proto[0];
			}
			else if(m_host_actual == &m_CloudHost2Address[0] && m_CloudHost1Address[0])
			{
				m_host_actual = &m_CloudHost1Address[0];
				m_port_actual = m_CloudHost1Port;
				m_proto_actual = &m_CloudHost1Proto[0];
			}
			else
			{
				m_host_actual = nullptr;
				m_port_actual = 0;
				m_proto_actual = nullptr;
			}
		}
		rc = DompiCloud_Notificar(m_host_actual, m_port_actual, m_proto_actual, message, message);
		if(rc < 0)
		{
			if(rc == (-10))
			{
				m_pServer->m_pLog->Add(100, "[CLOUD] ERROR en send a %s", m_host_actual);
			}
			else
			{
				m_pServer->m_pLog->Add(100, "[CLOUD] ERROR en recv de %s", m_host_actual);
			}
			if(m_cloud_status)
			{
				m_pServer->m_pLog->Add(1, "[CLOUD] Conexión perdida con %s", m_host_actual);
				m_cloud_status = 0;
			}
		}
		else
		{
			if(m_cloud_status == 0)
			{
				m_pServer->m_pLog->Add(1, "[CLOUD] Conexión restablecida con %s", m_host_actual);
				m_cloud_status = 1;
			}
		}
		if( rc > 0 && strlen(message) )
		{
			/* La respuesta de la nube puede venir con un array de acciones luego de la cabecera HTTP*/
			m_pServer->m_pLog->Add(100, "[CLOUD] >> [%s]", message);
			json_Message = cJSON_Parse(message);
			//json_resp_code = cJSON_GetObjectItemCaseSensitive(json_Message, "resp_code");
			//json_resp_msg = cJSON_GetObjectItemCaseSensitive(json_Message, "resp_msg");
			json_arr = cJSON_GetObjectItemCaseSensitive(json_Message, "response");
			if(json_arr && cJSON_IsArray(json_arr))
			{
				cJSON_PrintPreallocated(json_arr, message, GM_COMM_MSG_LEN, 0);
				m_pServer->m_pLog->Add(50, "[dompi_cloud_notification][%s]", message);
				m_pServer->Notify("dompi_cloud_notification", message, strlen(message));
				return 1;
			}
			cJSON_Delete(json_Message);
		}
		else
		{
			m_pServer->m_pLog->Add(100, "[CLOUD] >> Respuesta vacia");
		}
	}
	else
	{
		m_pServer->m_pLog->Add(1, "No hay servidores definodos para reporte a la nube");
	}
	return 0;
}

int SendToCloud( void )
{
    CGMServerBase::GMIOS resp;
	char message[GM_COMM_MSG_LEN+1];
	cJSON *json_Message;
	cJSON *json_arr;
	int rc = 0;

	if(m_cloud_status == 0) return 0;

    if(m_pServer->Dequeue("dompi_msg_to_cloud", &resp) == 0)
    {
		m_pServer->m_pLog->Add(100, "[CLOUD] << [%s]", (char*)resp.data);
		rc = DompiCloud_Notificar(m_host_actual, m_port_actual, m_proto_actual, (char*)resp.data, message);
		if(rc < 0)
		{
			if(rc == (-10))
			{
				m_pServer->m_pLog->Add(100, "[CLOUD] ERROR en send a %s", m_host_actual);
			}
			else
			{
				m_pServer->m_pLog->Add(100, "[CLOUD] ERROR en recv de %s", m_host_actual);
			}
			m_pServer->m_pLog->Add(1, "[CLOUD] Conexión perdida con %s", m_host_actual);
			m_cloud_status = 0;
		}
		else if( rc > 0 && strlen(message) )
		{
			/* La respuesta de la nube puede venir con un array de acciones luego de la cabecera HTTP*/
			m_pServer->m_pLog->Add(100, "[CLOUD] >> [%s]", message);
			json_Message = cJSON_Parse(message);
			//json_resp_code = cJSON_GetObjectItemCaseSensitive(json_Message, "resp_code");
			//json_resp_msg = cJSON_GetObjectItemCaseSensitive(json_Message, "resp_msg");
			json_arr = cJSON_GetObjectItemCaseSensitive(json_Message, "response");
			if(json_arr && cJSON_IsArray(json_arr))
			{
				cJSON_PrintPreallocated(json_arr, message, GM_COMM_MSG_LEN, 0);
				m_pServer->m_pLog->Add(50, "[dompi_cloud_notification][%s]", message);
				m_pServer->Notify("dompi_cloud_notification", message, strlen(message));
			}
			cJSON_Delete(json_Message);
		}
		else
		{
			m_pServer->m_pLog->Add(100, "[CLOUD] >> Respuesta vacia");
		}
		rc = 1;
    }
	return rc;
}

void CheckUpdateAssignCloud( void )
{
	char query[4096];
	char message[4096];
	int rc;
	cJSON *json_QueryArray;
	cJSON *json_QueryRow;
	cJSON *json_Id;
	cJSON *json_Tipo;
	cJSON *json_Estado;

	int i_tipo;
	char s_tipo[12];

	time_t t;

	t = time(&t);

	if(t >= update_ass_t && m_cloud_status && m_host_actual)
	{
		/* Actualizacion de objetos en la nube cada 10 min */
		update_ass_t = t + 600;

		m_pServer->m_pLog->Add(10, "[CheckUpdateAssignCloud] Actualizando estado general de objetos en la nube.");
		/* Genero un listado de los objetos con su estado para subir a la nube */
		json_QueryArray = cJSON_CreateArray();
		strcpy(query, "SELECT * FROM TB_DOM_ASSIGN WHERE Id > 0;");
		m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
		rc = pDB->Query(json_QueryArray, query);
		m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
		if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
		if(rc >= 0)
		{
			cJSON_ArrayForEach(json_QueryRow, json_QueryArray)
			{
				/* Agrego datos del sistema */
				cJSON_AddStringToObject(json_QueryRow, "System_Key", m_SystemKey);
				cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
				if(m_pServer->Enqueue("dompi_msg_to_cloud", message, strlen(message)) != GME_OK)
				{
					m_pServer->m_pLog->Add(1, "[CheckUpdateAssignCloud] ERROR: Encolando en SAF dompi_msg_to_cloud [%s]", message);
				}
			}
		}
		cJSON_Delete(json_QueryArray);

		/* Genero un listado de los automatismos con su estado para subir a la nube */
		json_QueryArray = cJSON_CreateArray();
		strcpy(query, "SELECT * FROM TB_DOM_AUTO WHERE Id > 0;");
		m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
		rc = pDB->Query(json_QueryArray, query);
		m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
		if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
		if(rc >= 0)
		{
			cJSON_ArrayForEach(json_QueryRow, json_QueryArray)
			{
				/* Cambio el Id (le sumo 10000) */
				json_Id = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Id");
				i_tipo = atoi(json_Id->valuestring);
				sprintf(s_tipo, "%d", i_tipo + 10000);
				cJSON_DeleteItemFromObjectCaseSensitive(json_QueryRow, "Id");
				cJSON_AddStringToObject(json_QueryRow, "Id", s_tipo);

				/* Cambio el tipo (le sumo 10) */
				json_Tipo = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Tipo");
				i_tipo = atoi(json_Tipo->valuestring);
				sprintf(s_tipo, "%d", i_tipo + 10);
				cJSON_DeleteItemFromObjectCaseSensitive(json_QueryRow, "Tipo");
				cJSON_AddStringToObject(json_QueryRow, "Tipo", s_tipo);

				/* Cambio el estado por el valor de Habilitado */
				json_Estado = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Habilitado");
				cJSON_DeleteItemFromObjectCaseSensitive(json_QueryRow, "Estado");
				cJSON_AddStringToObject(json_QueryRow, "Estado", json_Estado->valuestring);

				/* Agrego datos del sistema */
				cJSON_AddStringToObject(json_QueryRow, "System_Key", m_SystemKey);
				cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);

				if(m_pServer->Enqueue("dompi_msg_to_cloud", message, strlen(message)) != GME_OK)
				{
					m_pServer->m_pLog->Add(1, "[CheckUpdateAssignCloud] ERROR: Encolando en SAF dompi_msg_to_cloud [%s]", message);
				}
			}
		}
		cJSON_Delete(json_QueryArray);

	}
}

void CheckUpdateAlarmCloud( void )
{
	char message[GM_COMM_MSG_LEN+1];
	time_t t;
	cJSON *json_Message;

	t = time(&t);

	if(t >= update_alarm_t && m_cloud_status && m_host_actual)
	{
		/* Actualizacion de objetos en la nube cada 10 min */
		update_alarm_t = t + 600;

		m_pServer->m_pLog->Add(10, "[CheckUpdateAlarmCloud] Actualizando estado general de alarma en la nube.");
		pEV->Estado_Alarma_General(message, GM_COMM_MSG_LEN);
		json_Message = cJSON_Parse(message);
		cJSON_AddStringToObject(json_Message, "System_Key", m_SystemKey);
		cJSON_PrintPreallocated(json_Message, message, GM_COMM_MSG_LEN, 0);
		cJSON_Delete(json_Message);
		if(m_pServer->Enqueue("dompi_msg_to_cloud", message, strlen(message)) != GME_OK)
		{
			m_pServer->m_pLog->Add(1, "[CheckUpdateAlarmCloud] ERROR: Encolando en SAF dompi_msg_to_cloud [%s]", message);
		}
	}
}

void CheckUpdateUserCloud( void )
{
	int rc;
	time_t t;
	char query[4096];
	char message[4096];
	cJSON *json_QueryArray;
	cJSON *json_QueryRow;
	cJSON *json_Usuario_Cloud;
	cJSON *json_Clave_Cloud;

	t = time(&t);

	if(t >= update_user_t && m_cloud_status && m_host_actual)
	{
		/* TODO: Actualizacion de usuarios en la nube cada 10 min */
		update_user_t = t + 600;

		m_pServer->m_pLog->Add(10, "[CheckUpdateUserCloud] Actualizando usuarios en la nube.");
		/* Genero un listado de los objetos con su estado para subir a la nube */
		json_QueryArray = cJSON_CreateArray();
		strcpy(query, "SELECT * FROM TB_DOM_USER WHERE Id > 0;");
		m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
		rc = pDB->Query(json_QueryArray, query);
		m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
		if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
		if(rc >= 0)
		{
			cJSON_ArrayForEach(json_QueryRow, json_QueryArray)
			{
				json_Usuario_Cloud = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Usuario_Cloud");
				json_Clave_Cloud = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Clave_Cloud");
				/* Lo envío a la nube solo si tiene usuario y clave definidos */
				if(json_Usuario_Cloud && json_Clave_Cloud)
				{
					if(strlen(json_Usuario_Cloud->valuestring) > 0 && strcmp(json_Usuario_Cloud->valuestring, "NULL") &&
						strlen(json_Clave_Cloud->valuestring) > 0 && strcmp(json_Clave_Cloud->valuestring, "NULL"))
					{
						/* Agrego datos del sistema */
						cJSON_AddStringToObject(json_QueryRow, "System_Key", m_SystemKey);
						cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
						if(m_pServer->Enqueue("dompi_msg_to_cloud", message, strlen(message)) != GME_OK)
						{
							m_pServer->m_pLog->Add(1, "[CheckUpdateUserCloud] ERROR: Encolando en SAF dompi_msg_to_cloud [%s]", message);
						}
					}
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
	char buffer[GM_COMM_MSG_LEN+1];
	char *ps;
	int rc = 0;

	buffer[0] = 0;
	if( proto && !strcmp(proto, "https"))
	{
		pSock = new CTcp(1, 3);
		if(port == 0) port = 443;
    		sprintf(buffer, http_post, url_default, host, strlen(send_msg), send_msg);
		rc =pSock->Query(host, port, buffer, buffer, GM_COMM_MSG_LEN, external_timeout);
		delete pSock;
	}
	else
	{
		pSock = new CTcp();
		if(port == 0) port = 80;
    		sprintf(buffer, http_post, url_default, host, strlen(send_msg), send_msg);
		rc = pSock->Query(host, port, buffer, buffer, GM_COMM_MSG_LEN, external_timeout);
		delete pSock;
	}

	if(receive_msg && rc > 0)
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
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
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

	if(m_CloudHost2Address[0])
	{
		m_host_actual = &m_CloudHost2Address[0];
		m_port_actual = m_CloudHost2Port;
		m_proto_actual = &m_CloudHost2Proto[0];
	}
	else if(m_CloudHost1Address[0])
	{
		m_host_actual = &m_CloudHost1Address[0];
		m_port_actual = m_CloudHost1Port;
		m_proto_actual = &m_CloudHost1Proto[0];
	}
	else
	{
		m_host_actual = nullptr;
		m_port_actual = 0;
		m_proto_actual = nullptr;
	}

	m_cloud_status = 0;


	if(rc >= 0) m_pServer->m_pLog->Add(1, "[LoadSystemConfig] Lectura de configuracion OK.");
}

void AddSaf( void )
{
	m_pServer->Notify(".create-queue", "dompi_msg_to_cloud", 19);	
}
