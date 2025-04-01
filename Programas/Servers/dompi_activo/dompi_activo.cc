
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
		GM_MSG_TYPE_NOT		- Sin respuesta (Notify) Lo atiende uno
		GM_MSG_TYPE_INT		- Mensaje particionado con continuación
	Se envía a todos los suscriptos
		GM_MSG_TYPE_MSG		- Sin respuesta (Post) Lo atienden todos
	*/
	m_pServer->Suscribe("dompi_reload_config", GM_MSG_TYPE_MSG);	/* Sin respuesta, llega a todos */
	m_pServer->Suscribe("dompi_aa_full_synch", GM_MSG_TYPE_NOT);	/* Sin respuesta, lo atiende el mas libre */

	AddSaf();

	m_pServer->m_pLog->Add(1, "Servicios de Sincronizacion inicializados.");

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 1000 )) >= 0)
	{
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(90, "%s:(Q)[%s]", fn, message);
			/* ****************************************************************
			*		dompi_infoio - Notificacion de estado y/o cambio de I/O
			**************************************************************** */
			if( !strcmp(fn, "dompi_aa_full_synch"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;

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
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_aa_config]", message);
					if(m_pServer->Enqueue("dompi_aa_config", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "[dompi_aa_full_synch] ERROR: Encolando en SAF dompi_aa_config [%s]", message);
					}
				}
				cJSON_Delete(json_Query_Result);

				/* Usuarios */
				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_USER;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc > 0)
				{
					cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
					{
						cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
						m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_aa_user]", message);
						if(m_pServer->Enqueue("dompi_aa_user", message, strlen(message)) != GME_OK)
						{
							m_pServer->m_pLog->Add(1, "[dompi_aa_full_synch] ERROR: Encolando en SAF dompi_aa_user [%s]", message);
						}
					}
				}
				cJSON_Delete(json_Query_Result);

				/* Perifericos */
				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_PERIF;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc > 0)
				{
					cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
					{
						cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
						m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_aa_perif]", message);
						if(m_pServer->Enqueue("dompi_aa_perif", message, strlen(message)) != GME_OK)
						{
							m_pServer->m_pLog->Add(1, "[dompi_aa_full_synch] ERROR: Encolando en SAF dompi_aa_perif [%s]", message);
						}
					}
				}
				cJSON_Delete(json_Query_Result);
				
				/* Objetos */
				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_ASSIGN;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc > 0)
				{
					cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
					{
						cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
						m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_aa_assign]", message);
						if(m_pServer->Enqueue("dompi_aa_assign", message, strlen(message)) != GME_OK)
						{
							m_pServer->m_pLog->Add(1, "[dompi_aa_full_synch] ERROR: Encolando en SAF dompi_aa_assign [%s]", message);
						}
					}
				}
				cJSON_Delete(json_Query_Result);

				/* Grupos */
				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_GROUP;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc > 0)
				{
					cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
					{
						cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
						m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_aa_group]", message);
						if(m_pServer->Enqueue("dompi_aa_group", message, strlen(message)) != GME_OK)
						{
							m_pServer->m_pLog->Add(1, "[dompi_aa_full_synch] ERROR: Encolando en SAF dompi_aa_group [%s]", message);
						}
					}
				}
				cJSON_Delete(json_Query_Result);

				/* Flags */
				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_FLAG;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc > 0)
				{
					cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
					{
						cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
						m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_aa_flag]", message);
						if(m_pServer->Enqueue("dompi_aa_flag", message, strlen(message)) != GME_OK)
						{
							m_pServer->m_pLog->Add(1, "[dompi_aa_full_synch] ERROR: Encolando en SAF dompi_aa_flag [%s]", message);
						}
					}
				}
				cJSON_Delete(json_Query_Result);

				/* Particiones */
				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_ALARM_PARTICION;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc > 0)
				{
					cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
					{
						cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
						m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_aa_particion]", message);
						if(m_pServer->Enqueue("dompi_aa_particion", message, strlen(message)) != GME_OK)
						{
							m_pServer->m_pLog->Add(1, "[dompi_aa_full_synch] ERROR: Encolando en SAF dompi_aa_particion [%s]", message);
						}
					}
				}
				cJSON_Delete(json_Query_Result);

				/* Zonas */
				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_ALARM_ZONA;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc > 0)
				{
					cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
					{
						cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
						m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_aa_alarm_zona]", message);
						if(m_pServer->Enqueue("dompi_aa_alarm_zona", message, strlen(message)) != GME_OK)
						{
							m_pServer->m_pLog->Add(1, "[dompi_aa_full_synch] ERROR: Encolando en SAF dompi_aa_alarm_zona [%s]", message);
						}
					}
				}
				cJSON_Delete(json_Query_Result);

				/* Salidas de Alarma */
				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_ALARM_SALIDA;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc > 0)
				{
					cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
					{
						cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
						m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_aa_alarm_salida]", message);
						if(m_pServer->Enqueue("dompi_aa_alarm_salida", message, strlen(message)) != GME_OK)
						{
							m_pServer->m_pLog->Add(1, "[dompi_aa_full_synch] ERROR: Encolando en SAF dompi_aa_alarm_salida [%s]", message);
						}
					}
				}
				cJSON_Delete(json_Query_Result);

				/* Camaras */
				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_CAMARA;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc > 0)
				{
					cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
					{
						cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
						m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_aa_camara]", message);
						if(m_pServer->Enqueue("dompi_aa_camara", message, strlen(message)) != GME_OK)
						{
							m_pServer->m_pLog->Add(1, "[dompi_aa_full_synch] ERROR: Encolando en SAF dompi_aa_camara [%s]", message);
						}
					}
				}
				cJSON_Delete(json_Query_Result);

				/* Eventos */
				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_EVENT;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc > 0)
				{
					cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
					{
						cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
						m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_aa_event]", message);
						if(m_pServer->Enqueue("dompi_aa_event", message, strlen(message)) != GME_OK)
						{
							m_pServer->m_pLog->Add(1, "[dompi_aa_full_synch] ERROR: Encolando en SAF dompi_aa_event [%s]", message);
						}
					}
				}
				cJSON_Delete(json_Query_Result);

				/* AT */
				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_AT;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc > 0)
				{
					cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
					{
						cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
						m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_aa_at]", message);
						if(m_pServer->Enqueue("dompi_aa_at", message, strlen(message)) != GME_OK)
						{
							m_pServer->m_pLog->Add(1, "[dompi_aa_full_synch] ERROR: Encolando en SAF dompi_aa_at [%s]", message);
						}
					}
				}
				cJSON_Delete(json_Query_Result);

				/* Auto */
				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_AUTO;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc > 0)
				{
					cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
					{
						cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
						m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_aa_auto]", message);
						if(m_pServer->Enqueue("dompi_aa_auto", message, strlen(message)) != GME_OK)
						{
							m_pServer->m_pLog->Add(1, "[dompi_aa_full_synch] ERROR: Encolando en SAF dompi_aa_auto [%s]", message);
						}
					}
				}
				cJSON_Delete(json_Query_Result);

				/* Touch */
				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_TOUCH;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc > 0)
				{
					cJSON_ArrayForEach(json_QueryRow, json_Query_Result)
					{
						cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
						m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_aa_touch]", message);
						if(m_pServer->Enqueue("dompi_aa_touch", message, strlen(message)) != GME_OK)
						{
							m_pServer->m_pLog->Add(1, "[dompi_aa_full_synch] ERROR: Encolando en SAF dompi_aa_touch [%s]", message);
						}
					}
				}
				cJSON_Delete(json_Query_Result);

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

		/* Configuracion */
		if(host_backup[0] != 0)
		{
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_aa_config", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_aa_config] -> [%s]", message);
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

			while(GetSAFRemoto(host_backup, 0, "http", "dompi_aa_user", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_aa_user] -> [%s]", message);
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
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_aa_perif", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_aa_perif] -> [%s]", message);
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
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_aa_assign", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_aa_assign] -> [%s]", message);
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
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_aa_group", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_aa_group] -> [%s]", message);
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
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_aa_flag", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_aa_flag] -> [%s]", message);
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
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_aa_particion", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_aa_particion] -> [%s]", message);
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
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_aa_alarm_zona", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_aa_alarm_zona] -> [%s]", message);
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
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_aa_alarm_salida", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_aa_alarm_salida] -> [%s]", message);
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
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_aa_camara", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_aa_camara] -> [%s]", message);
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
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_aa_event", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_aa_event] -> [%s]", message);
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
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_aa_at", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_aa_at] -> [%s]", message);
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
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_aa_auto", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_aa_auto] -> [%s]", message);
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
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_aa_touch", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_aa_touch] -> [%s]", message);
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
	m_pServer->UnSuscribe("dompi_aa_full_synch", GM_MSG_TYPE_NOT);

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
	m_pServer->Notify(".create-queue", "dompi_aa_config", 16);	
	m_pServer->Notify(".create-queue", "dompi_aa_user", 14);	
	m_pServer->Notify(".create-queue", "dompi_aa_perif", 15);	
	m_pServer->Notify(".create-queue", "dompi_aa_assign", 16);	
	m_pServer->Notify(".create-queue", "dompi_aa_group", 15);	
	m_pServer->Notify(".create-queue", "dompi_aa_flag", 14);	
	m_pServer->Notify(".create-queue", "dompi_aa_particion", 19);	
	m_pServer->Notify(".create-queue", "dompi_aa_alarm_zona", 20);	
	m_pServer->Notify(".create-queue", "dompi_aa_alarm_salida", 22);	
	m_pServer->Notify(".create-queue", "dompi_aa_camara", 16);	
	m_pServer->Notify(".create-queue", "dompi_aa_event", 15);	
	m_pServer->Notify(".create-queue", "dompi_aa_at", 12);	
	m_pServer->Notify(".create-queue", "dompi_aa_auto", 14);	
	m_pServer->Notify(".create-queue", "dompi_aa_touch", 15);	
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