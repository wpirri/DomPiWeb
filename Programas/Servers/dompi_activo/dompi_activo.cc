
/***************************************************************************
    Copyright (C) 2025   Walter Pirri

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

#include <cjson/cJSON.h>

#include "config.h"
#include "cdb.h"
#include "strfunc.h"
#include "defines.h"
#include "ctcp.h"

CGMServerWait *m_pServer;
DPConfig *pConfig;
int internal_timeout;
int external_timeout;
CDB *pDB;
cJSON *json_System_Config;
int i;

void OnClose(int sig);
void LoadSystemConfig(void);

void AddSaf( void );
int GetSAFRemoto(const char* host, int port, const char* proto, const char* saf_name, char* msg, unsigned int msg_max);

int DBInsert(CDB* db, const char* tabla, cJSON* jdata);
int DBUpdate(CDB* db, const char* tabla, const char *key, cJSON* jdata);

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[GM_COMM_MSG_LEN+1];
	unsigned long message_len;	
	char db_host[32];
	char db_name[32];
	char db_user[32];
	char db_password[32];
	char query[4096];
	char host_backup[32];
	char s[16];
	bool send_config = false;
	bool send_user = false;
	bool send_perif = false;
	bool send_assign = false;
	bool send_group = false;
	bool send_flag = false;
	bool send_partition = false;
	bool send_alarm_zona = false;
	bool send_alarm_salida = false;
	bool send_camara = false;
	bool send_event = false;
	bool send_at = false;
	bool send_auto = false;
	bool send_touch = false;

    cJSON *json_Message;
    cJSON *json_Query_Result = NULL;
    cJSON *json_QueryRow;
    cJSON *json_Id;
	
	signal(SIGPIPE, SIG_IGN);
	signal(SIGKILL, OnClose);
	signal(SIGTERM, OnClose);

	m_pServer = new CGMServerWait;
	m_pServer->Init("dompi_activo");
	m_pServer->m_pLog->Add(1, "Iniciando Servidor de Sincronizacion...");

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

	host_backup[0] = 0;
	pConfig->GetParam("BACKUP", host_backup);

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

	/*
	Se distribuye equitativamente entre las colas menos cargadas
		GM_MSG_TYPE_CR		- Se espera respuesta (Call)
		GM_MSG_TYPE_MSG		- Sin respuesta (Notify) Lo atiende uno
		GM_MSG_TYPE_INT		- Mensaje particionado con continuación
	Se envía a todos los suscriptos
		GM_MSG_TYPE_MSG		- Sin respuesta (Post) Lo atienden todos
	*/
	m_pServer->Suscribe("dompi_reload_config", GM_MSG_TYPE_MSG);	/* Sin respuesta, llega a todos */
	m_pServer->Suscribe("dompi_full_change", GM_MSG_TYPE_MSG);	/* Sin respuesta, lo atiende el mas libre */
	m_pServer->Suscribe("dompi_config_change", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_user_change", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_perif_change", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_assign_change", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_group_change", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_flag_change", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_partition_change", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_alarm_zona_change", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_alarm_salida_change", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_camara_change", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_event_change", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_at_change", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_auto_change", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_touch_change", GM_MSG_TYPE_MSG);

	AddSaf();

	m_pServer->m_pLog->Add(1, "Servicios de Sincronizacion inicializados.");

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 1000 )) >= 0)
	{
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(90, "%s:(Q)[%s]", fn, message);

			json_Message = cJSON_Parse(message);
			message[0] = 0;
			
			/* ****************************************************************
			*		dompi_infoio - Notificacion de estado y/o cambio de I/O
			**************************************************************** */
			if( !strcmp(fn, "dompi_full_change"))
			{
				json_Id = nullptr;

				send_config = true;
				send_user = true;
				send_perif = true;
				send_assign = true;
				send_group = true;
				send_flag = true;
				send_partition = true;
				send_alarm_zona = true;
				send_alarm_salida = true;
				send_camara = true;
				send_event = true;
				send_at = true;
				send_auto = true;
				send_touch = true;
			}
			else if( !strcmp(fn, "dompi_config_change"))
			{
				send_config = true;
			}
			else if( !strcmp(fn, "dompi_user_change"))
			{
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				send_user = true;
			}
			else if( !strcmp(fn, "dompi_perif_change"))
			{
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				send_perif = true;
			}
			else if( !strcmp(fn, "dompi_assign_change"))
			{
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(json_Id == nullptr)
				{
					json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "ASS_Id");
				}
				send_assign = true;
			}
			else if( !strcmp(fn, "dompi_group_change"))
			{
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(json_Id == nullptr)
				{
					json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "GRP_Id");
				}
				send_group = true;
			}
			else if( !strcmp(fn, "dompi_flag_change"))
			{
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				send_flag = true;
			}
			else if( !strcmp(fn, "dompi_partition_change"))
			{
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				send_partition = true;
			}
			else if( !strcmp(fn, "dompi_alarm_zona_change"))
			{
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				send_alarm_zona = true;
			}
			else if( !strcmp(fn, "dompi_alarm_salida_change"))
			{
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				send_alarm_salida = true;
			}
			else if( !strcmp(fn, "dompi_camara_change"))
			{
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				send_camara = true;
			}
			else if( !strcmp(fn, "dompi_event_change"))
			{
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				send_event = true;
			}
			else if( !strcmp(fn, "dompi_at_change"))
			{
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				send_at = true;
			}
			else if( !strcmp(fn, "dompi_auto_change"))
			{
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				send_auto = true;
			}
			else if( !strcmp(fn, "dompi_touch_change"))
			{
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				send_touch = true;
			}
			/* ****************************************************************
			*		dompi_reload_config
			**************************************************************** */
			else if( !strcmp(fn, "dompi_reload_config"))
			{
				//m_pServer->Resp(NULL, 0, GME_OK);

				LoadSystemConfig();
			}
			






			/* ****************************************************************
			*		FIN SERVICIOS
			**************************************************************** */
			else
			{
				m_pServer->m_pLog->Add(90, "[%s][R][GME_SVC_NOTFOUND]", fn);
				m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
			}
		}


		/* Encolo datos para el sistema remoto */
		if(send_config)
		{
			send_config = false;
			/* Configuración */
			json_Query_Result = cJSON_CreateArray();
			strcpy(query, "SELECT * FROM TB_DOM_CONFIG ORDER BY Id DESC LIMIT 1;");
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_PrintPreallocated(json_Query_Result, message, GM_COMM_MSG_LEN, 0);
				m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_config]", message);
				if(m_pServer->Enqueue("dompi_config", message, strlen(message)) != GME_OK)
				{
					m_pServer->m_pLog->Add(1, "[dompi_full_change] ERROR: Encolando en SAF dompi_config [%s]", message);
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_user)
		{
			send_user = false;
			/* Usuarios */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				sprintf(query, "SELECT * FROM TB_DOM_USER WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				strcpy(query, "SELECT * FROM TB_DOM_USER;");
			}
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
				{
					cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_user]", message);
					if(m_pServer->Enqueue("dompi_user", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_full_change] ERROR: Encolando en SAF dompi_user [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_perif)
		{
			send_perif = false;
			/* Perifericos */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				sprintf(query, "SELECT * FROM TB_DOM_PERIF WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				strcpy(query, "SELECT * FROM TB_DOM_PERIF;");
			}
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
				{
					cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_perif]", message);
					if(m_pServer->Enqueue("dompi_perif", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_full_change] ERROR: Encolando en SAF dompi_perif [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_assign)
		{
			send_assign = false;
			/* Objetos */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				sprintf(query, "SELECT * FROM TB_DOM_ASSIGN WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				strcpy(query, "SELECT * FROM TB_DOM_ASSIGN;");
			}
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
				{
					cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_assign]", message);
					if(m_pServer->Enqueue("dompi_assign", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_full_change] ERROR: Encolando en SAF dompi_assign [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_group)
		{
			send_group = false;
			/* Grupos */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				sprintf(query, "SELECT * FROM TB_DOM_GROUP WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				strcpy(query, "SELECT * FROM TB_DOM_GROUP;");
			}
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
				{
					cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_group]", message);
					if(m_pServer->Enqueue("dompi_group", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_full_change] ERROR: Encolando en SAF dompi_group [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_flag)
		{
			send_flag = false;
			/* Flags */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				sprintf(query, "SELECT * FROM TB_DOM_FLAG WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				strcpy(query, "SELECT * FROM TB_DOM_FLAG;");
			}
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
				{
					cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_flag]", message);
					if(m_pServer->Enqueue("dompi_flag", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_full_change] ERROR: Encolando en SAF dompi_flag [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_partition)
		{
			send_partition = false;
			/* Particiones */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				sprintf(query, "SELECT * FROM TB_DOM_ALARM_PARTICION WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				strcpy(query, "SELECT * FROM TB_DOM_ALARM_PARTICION;");
			}
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
				{
					cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_particion]", message);
					if(m_pServer->Enqueue("dompi_particion", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_full_change] ERROR: Encolando en SAF dompi_particion [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_alarm_zona)
		{
			send_alarm_zona = false;
			/* Zonas */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				sprintf(query, "SELECT * FROM TB_DOM_ALARM_ZONA WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				strcpy(query, "SELECT * FROM TB_DOM_ALARM_ZONA;");
			}
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
				{
					cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_alarm_zona]", message);
					if(m_pServer->Enqueue("dompi_alarm_zona", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_full_change] ERROR: Encolando en SAF dompi_alarm_zona [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_alarm_salida)
		{
			send_alarm_salida = false;
			/* Salidas de Alarma */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				sprintf(query, "SELECT * FROM TB_DOM_ALARM_SALIDA WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				strcpy(query, "SELECT * FROM TB_DOM_ALARM_SALIDA;");
			}
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
				{
					cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_alarm_salida]", message);
					if(m_pServer->Enqueue("dompi_alarm_salida", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_full_change] ERROR: Encolando en SAF dompi_alarm_salida [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_camara)
		{
			send_camara = false;
			/* Camaras */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				sprintf(query, "SELECT * FROM TB_DOM_CAMARA WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				strcpy(query, "SELECT * FROM TB_DOM_CAMARA;");
			}
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
				{
					cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_camara]", message);
					if(m_pServer->Enqueue("dompi_camara", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_full_change] ERROR: Encolando en SAF dompi_camara [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_event)
		{
			send_event = false;
			/* Eventos */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				sprintf(query, "SELECT * FROM TB_DOM_EVENT WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				strcpy(query, "SELECT * FROM TB_DOM_EVENT;");
			}
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
				{
					cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_event]", message);
					if(m_pServer->Enqueue("dompi_event", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_full_change] ERROR: Encolando en SAF dompi_event [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_at)
		{
			send_at = false;
			/* AT */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				sprintf(query, "SELECT * FROM TB_DOM_AT WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				strcpy(query, "SELECT * FROM TB_DOM_AT;");
			}
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
				{
					cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_at]", message);
					if(m_pServer->Enqueue("dompi_at", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_full_change] ERROR: Encolando en SAF dompi_at [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_auto)
		{
			send_auto = false;
			/* Auto */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				sprintf(query, "SELECT * FROM TB_DOM_AUTO WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				strcpy(query, "SELECT * FROM TB_DOM_AUTO;");
			}
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
				{
					cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_auto]", message);
					if(m_pServer->Enqueue("dompi_auto", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_full_change] ERROR: Encolando en SAF dompi_auto [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_touch)
		{
			send_touch = false;
			/* Touch */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				sprintf(query, "SELECT * FROM TB_DOM_TOUCH WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				strcpy(query, "SELECT * FROM TB_DOM_TOUCH;");
			}
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
				{
					cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_touch]", message);
					if(m_pServer->Enqueue("dompi_touch", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_full_change] ERROR: Encolando en SAF dompi_touch [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}


		/* Traigo datos del sistema remoto */
		if(host_backup[0] != 0)
		{
			/* Configuracion */
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_config", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_config] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				if( DBUpdate(pDB, "TB_DOM_CONFIG", "Id", json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_CONFIG", json_Message);
				}
			}

			while(GetSAFRemoto(host_backup, 0, "http", "dompi_user", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_user] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				if( DBUpdate(pDB, "TB_DOM_USER", "Id", json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_USER", json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_perif", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_perif] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				if( DBUpdate(pDB, "TB_DOM_PERIF", "Id", json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_PERIF", json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_assign", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_assign] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				if( DBUpdate(pDB, "TB_DOM_ASSIGN", "Id", json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_ASSIGN", json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_group", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_group] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				if( DBUpdate(pDB, "TB_DOM_GROUP", "Id", json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_GROUP", json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_flag", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_flag] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				if( DBUpdate(pDB, "TB_DOM_FLAG", "Id", json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_FLAG", json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_particion", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_particion] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				if( DBUpdate(pDB, "TB_DOM_ALARM_PARTICION", "Id", json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_ALARM_PARTICION", json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_alarm_zona", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_alarm_zona] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				if( DBUpdate(pDB, "TB_DOM_ALARM_ZONA", "Id", json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_ALARM_ZONA", json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_alarm_salida", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_alarm_salida] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				if( DBUpdate(pDB, "TB_DOM_ALARM_SALIDA", "Id", json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_ALARM_SALIDA", json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_camara", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_camara] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				if( DBUpdate(pDB, "TB_DOM_CAMARA", "Id", json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_CAMARA", json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_event", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_event] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				if( DBUpdate(pDB, "TB_DOM_EVENT", "Id", json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_EVENT", json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_at", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_at] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				if( DBUpdate(pDB, "TB_DOM_AT", "Id", json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_AT", json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_auto", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_auto] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				if( DBUpdate(pDB, "TB_DOM_AUTO", "Id", json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_AUTO", json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_touch", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_touch] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				if( DBUpdate(pDB, "TB_DOM_TOUCH", "Id", json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_TOUCH", json_Message);
				}
			}
		}



	}
	m_pServer->m_pLog->Add(1, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);

	m_pServer->UnSuscribe("dompi_reload_config", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_full_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_config_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_user_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_perif_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_assign_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_group_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_flag_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_partition_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_alarm_zona_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_alarm_salida_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_camara_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_event_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_at_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_auto_change", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_touch_change", GM_MSG_TYPE_MSG);

	delete m_pServer;
	delete pConfig;
	delete pDB;

	exit(0);
}

void LoadSystemConfig(void)
{
	char query[4096];
	int rc;

	if(pDB == NULL) return;

	m_pServer->m_pLog->Add(50, "[LoadSystemConfig]");

	if(json_System_Config) cJSON_Delete(json_System_Config);
	json_System_Config = cJSON_CreateArray();
	strcpy(query, "SELECT * FROM TB_DOM_CONFIG ORDER BY Id DESC LIMIT 1;");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_System_Config, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc >= 0) m_pServer->m_pLog->Add(1, "[LoadSystemConfig] Lectura de configuracion OK.");
}

void AddSaf( void )
{
	m_pServer->Notify(".create-queue", "dompi_config", 16);	
	m_pServer->Notify(".create-queue", "dompi_user", 14);	
	m_pServer->Notify(".create-queue", "dompi_perif", 15);	
	m_pServer->Notify(".create-queue", "dompi_assign", 16);	
	m_pServer->Notify(".create-queue", "dompi_group", 15);	
	m_pServer->Notify(".create-queue", "dompi_flag", 14);	
	m_pServer->Notify(".create-queue", "dompi_particion", 19);	
	m_pServer->Notify(".create-queue", "dompi_alarm_zona", 20);	
	m_pServer->Notify(".create-queue", "dompi_alarm_salida", 22);	
	m_pServer->Notify(".create-queue", "dompi_camara", 16);	
	m_pServer->Notify(".create-queue", "dompi_event", 15);	
	m_pServer->Notify(".create-queue", "dompi_at", 12);	
	m_pServer->Notify(".create-queue", "dompi_auto", 14);	
	m_pServer->Notify(".create-queue", "dompi_touch", 15);	
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
	char buffer[GM_COMM_MSG_LEN+1];
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
		m_pServer->m_pLog->Add(100, "[GetSAFRemoto] https://%s [%s].", host, buffer);
		rc = pSock->Query(host, port, buffer, buffer, GM_COMM_MSG_LEN, external_timeout);
		delete pSock;
	}
	else
	{
		pSock = new CTcp();
		if(port == 0) port = 80;
		sprintf(buffer, http_get, url_default, get, host);
		m_pServer->m_pLog->Add(100, "[GetSAFRemoto] http://%s [%s].", host, buffer);
		rc = pSock->Query(host, port, buffer, buffer, GM_COMM_MSG_LEN, external_timeout);
		delete pSock;
	}

	if(rc < 0)
	{
		m_pServer->m_pLog->Add(100, "[GetSAFRemoto] ERROR en CTcp::Query.");
	}

	if(msg && rc > 0)
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
				m_pServer->m_pLog->Add(100, "[GetSAFRemoto] Resp [%s].", msg);
			}
		}
		rc = strlen(msg);
	}
	return rc;
}

int DBInsert(CDB* db, const char* tabla, cJSON* jdata)
{
	cJSON *j;
	int rc;
	char query_into[4096];
	char query_values[4096];
	char query[4096];

	j = jdata;
	query[0] = 0;
	query_into[0] = 0;
	query_values[0] = 0;

	while( j )
	{
		/* Voy hasta el elemento con datos */
		if(j->type == cJSON_Object)
		{
			j = j->child;
		}
		else
		{
			if(j->type == cJSON_String)
			{
				if(j->string && j->valuestring)
				{
					if(strlen(j->string) && strlen(j->valuestring))
					{
						/* Dato */
						if(strlen(query_into) == 0)
						{
							strcpy(query_into, "(");
						}
						else
						{
							strcat(query_into, ",");
						}
						strcat(query_into, j->string);
						/* Valor */
						if(strlen(query_values) == 0)
						{
							strcpy(query_values, "(");
						}
						else
						{
							strcat(query_values, ",");
						}
						strcat(query_values, "'");
						strcat(query_values, j->valuestring);
						strcat(query_values, "'");
					}
				}
			}
			j = j->next;
		}
	}
	strcat(query_into, ")");
	strcat(query_values, ")");

	sprintf(query, "INSERT INTO %s %s VALUES %s;", tabla, query_into, query_values);
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = db->Query(NULL, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, db->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", db->m_last_error_text, query);
	return rc;
}

int DBUpdate(CDB* db, const char* tabla, const char *key, cJSON* jdata)
{
	cJSON *j;
	int rc;
	char query_values[4096];
	char query_where[4096];
	char query[4096];

	j = jdata;
	query[0] = 0;
	query_values[0] = 0;
	query_where[0] = 0;

	j = jdata;
	while( j )
	{
		/* Voy hasta el elemento con datos */
		if(j->type == cJSON_Object)
		{
			j = j->child;
		}
		else
		{
			if(j->type == cJSON_String)
			{
				if(j->string && j->valuestring)
				{
					if(strlen(j->string) /*&& strlen(j->valuestring)*/)
					{
						if( !strcmp(j->string, key) )
						{
							strcpy(query_where, j->string);
							strcat(query_where, "='");
							strcat(query_where, j->valuestring);
							strcat(query_where, "'");
						}
						else
						{
							/* Dato = Valor */
							if(strlen(query_values) > 0)
							{
								strcat(query_values, ",");
							}
							strcat(query_values, j->string);
							strcat(query_values, "='");
							strcat(query_values, j->valuestring);
							strcat(query_values, "'");
						}
					}
				}
			}
			j = j->next;
		}
	}

	if(strlen(query_where))
	{
		sprintf(query, "UPDATE %s SET %s WHERE %s;", tabla, query_values, query_where);
		m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
		rc = db->Query(NULL, query);
		m_pServer->m_pLog->Add((db->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, db->LastQueryTime(), query);
		if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", db->m_last_error_text, query);
		return rc;
	}
	return (-1);
}