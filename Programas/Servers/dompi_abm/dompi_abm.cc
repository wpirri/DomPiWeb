
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

#include <cjson/cJSON.h>

#include "config.h"
#include "cdb.h"
#include "strfunc.h"

#define MAX_BUFFER_LEN 32767
#define BT_BUF_SIZE 256

CGMServerWait *m_pServer;
DPConfig *pConfig;
int internal_timeout;
CDB *pDB;
cJSON *json_System_Config;

void LoadSystemConfig(void);
int ExcluirDeABM(const char* label);
void OnClose(int sig);

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[MAX_BUFFER_LEN+1];
	char db_host[32];
	char db_user[32];
	char db_password[32];
	char query[4096];
	char query_into[1024];
	char query_values[2048];
	char query_where[512];
	unsigned long message_len;
	int update_hw_config_id;
	time_t t;
	struct tm *lt;
	int checked;
	char hw_id[16];
	long temp_l;
	char temp_s[64];
	char s[16];
	int load_system_config;

	STRFunc Strf;

    cJSON *json_obj;
    cJSON *json_un_obj;
    cJSON *json_Query_Result = NULL;
    cJSON *json_user;
    cJSON *json_pass;
    cJSON *json_channel;
    cJSON *json_query_result;
	cJSON *json_Planta;
	cJSON *json_Id;
	

	load_system_config = 0;
	update_hw_config_id = 0;

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
	m_pServer->Init("dompi_abm");
	m_pServer->m_pLog->Add(1, "Iniciando servicios de ABM...");

	m_pServer->m_pLog->Add(10, "Leyendo configuración...");
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

	m_pServer->Suscribe("dompi_db_struct", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_user_list", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_user_list_all", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_user_get", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_user_add", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_user_delete", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_user_update", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_user_check", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_list", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_list_all", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_get", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_add", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_delete", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_update", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_list", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_list_all", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_get", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_add", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_delete", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_update", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_status", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_info", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_on", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_off", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_switch", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_pulse", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_add_to_planta", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_list", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_list_all", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_get", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_add", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_delete", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_update", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_task_list", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_task_list_all", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_task_get", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_task_add", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_task_delete", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_task_update", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_group_list", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_group_list_all", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_group_get", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_group_add", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_group_delete", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_group_update", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_sysconf_list", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_sysconf_get", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_sysconf_get_current", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_sysconf_add", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_list", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_list_all", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_get", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_add", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_delete", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_update", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_status", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_info", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_enable", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_disable", GM_MSG_TYPE_CR);

	m_pServer->m_pLog->Add(1, "ABM de domotica inicializado.");

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, (-1) )) >= 0)
	{
		if(rc > 0)
		{
			json_query_result = NULL;
			message[message_len] = 0;
			m_pServer->m_pLog->Add(90, "%s:(Q)[%s]", fn, message);
			/* ****************************************************************
			*		dompi_db_struct
			**************************************************************** */
			if( !strcmp(fn, "dompi_db_struct"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "table");
				if(json_un_obj)
				{
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, ".schema %s;", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc == 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}

			}
			/* ****************************************************************
			*		dompi_user_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_list"))
			{
				message[0] = 0;

				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT Id, Usuario, Nombre_Completo, Estado, Ultimo_Acceso "
								"FROM TB_DOM_USER;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_Query_Result);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_user_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_list_all"))
			{
				message[0] = 0;

				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_USER;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_Query_Result);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_user_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_get"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_USER WHERE Id = %s;", json_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc >= 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_user_add
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_add"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;

				/* Obtengo un ID para el elemento nuevo y lo cambio en el dato recibido */
				temp_l = pDB->NextId("TB_DOM_USER", "Id");
				sprintf(temp_s, "%li", temp_l);
				cJSON_DeleteItemFromObjectCaseSensitive(json_obj, "Id");
				cJSON_AddStringToObject(json_obj, "Id", temp_s);

				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
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
										strcat(query_into, json_un_obj->string);
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
										strcat(query_values, json_un_obj->valuestring);
										strcat(query_values, "'");
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);

				strcat(query_into, ")");
				strcat(query_values, ")");

				sprintf(query, "INSERT INTO TB_DOM_USER %s VALUES %s;", query_into, query_values);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(NULL, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc != 1)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_user_delete
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_delete"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					if( strcmp(json_un_obj->valuestring, "admin") )
					{
						sprintf(query, "DELETE FROM TB_DOM_USER WHERE Id = %s;", json_un_obj->valuestring);
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						if(rc != 0)
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Usuario Invalido\"}}");
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_user_update
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_update"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;
				query_where[0] = 0;

				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
									{
										if( !strcmp(json_un_obj->string, "Id") )
										{
											strcpy(query_where, json_un_obj->string);
											strcat(query_where, "='");
											strcat(query_where, json_un_obj->valuestring);
											strcat(query_where, "'");
										}
										else
										{
											/* Dato = Valor */
											if(strlen(query_values) > 0)
											{
												strcat(query_values, ",");
											}
											strcat(query_values, json_un_obj->string);
											strcat(query_values, "='");
											strcat(query_values, json_un_obj->valuestring);
											strcat(query_values, "'");
										}
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);
				if(strlen(query_where))
				{
					sprintf(query, "UPDATE TB_DOM_USER SET %s WHERE %s;", query_values, query_where);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Form Data Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_user_check
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_check"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"99\", \"resp_msg\":\"General Error\"}}");
				checked = 0;
				json_user = cJSON_GetObjectItemCaseSensitive(json_obj, "user_id");
				json_pass = cJSON_GetObjectItemCaseSensitive(json_obj, "password");
				json_channel = cJSON_GetObjectItemCaseSensitive(json_obj, "channel");

				if(json_user && json_pass && json_channel)
				{
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT Permisos,Dias,Horas,Estado,Contador_Error,"
					"Pin_Teclado,Pin_SMS,Pin_WEB FROM TB_DOM_USER WHERE UPPER(Usuario) = UPPER(\'%s\');", json_user->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc >= 0)
					{
						//json_response = cJSON_GetObjectItemCaseSensitive(json_Query_Result, "response");

						if( !strcmp(json_channel->valuestring, "web"))
						{
							//json_response_password = cJSON_GetObjectItemCaseSensitive(json_Query_Result, "response");
						}
						else if( !strcmp(json_channel->valuestring, "sms"))
						{

						}
						else if( !strcmp(json_channel->valuestring, "pad"))
						{

						}
						else
						{

						}


						if(checked == 1)
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
						}
						else
						{

						}

					}
				}
				cJSON_Delete(json_obj);
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_hw_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_hw_list"))
			{
				message[0] = 0;

				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT Id, Dispositivo, Tipo, Estado FROM TB_DOM_PERIF;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_Query_Result);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_hw_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_hw_list_all"))
			{
				message[0] = 0;

				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_PERIF;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_Query_Result);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_hw_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_hw_get"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_PERIF WHERE Id = %s;", json_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc >= 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_hw_add
			**************************************************************** */
			else if( !strcmp(fn, "dompi_hw_add"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;

				/* Obtengo un ID para el elemento nuevo y lo cambio en el dato recibido */
				temp_l = pDB->NextId("TB_DOM_PERIF", "Id");
				sprintf(temp_s, "%li", temp_l);
				cJSON_DeleteItemFromObjectCaseSensitive(json_obj, "Id");
				cJSON_AddStringToObject(json_obj, "Id", temp_s);

				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
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
										strcat(query_into, json_un_obj->string);
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
										strcat(query_values, json_un_obj->valuestring);
										strcat(query_values, "'");
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);

				strcat(query_into, ")");
				strcat(query_values, ")");

				sprintf(query, "INSERT INTO TB_DOM_PERIF %s VALUES %s;", query_into, query_values);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(NULL, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc != 1)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_hw_delete
			**************************************************************** */
			else if( !strcmp(fn, "dompi_hw_delete"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					if( memcmp(json_Id->valuestring, "00", 2) )
					{
						sprintf(query, "DELETE FROM TB_DOM_PERIF WHERE Id = %s;", json_Id->valuestring);
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						if(rc != 0)
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalis User\"}}");
					}
				}

				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_hw_update
			**************************************************************** */
			else if( !strcmp(fn, "dompi_hw_update"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;
				query_where[0] = 0;
				hw_id[0] = 0;

				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
									{
										if( !strcmp(json_un_obj->string, "Id") )
										{
											strcpy(query_where, json_un_obj->string);
											strcat(query_where, "='");
											strcat(query_where, json_un_obj->valuestring);
											strcat(query_where, "'");

											strcpy(hw_id, json_un_obj->valuestring);
										}
										else
										{
											/* Dato = Valor */
											if(strlen(query_values) > 0)
											{
												strcat(query_values, ",");
											}
											strcat(query_values, json_un_obj->string);
											strcat(query_values, "='");
											strcat(query_values, json_un_obj->valuestring);
											strcat(query_values, "'");
										}
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);
				if(strlen(query_where) && strlen(hw_id))
				{
					strcat(query_values, ",Actualizar = 1");
					sprintf(query, "UPDATE TB_DOM_PERIF SET %s WHERE %s;", query_values, query_where);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Form Data Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}

			}
			/* ****************************************************************
			*		dompi_ass_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_list"))
			{
				message[0] = 0;

				json_query_result = cJSON_CreateArray();
				strcpy(query, "SELECT ASS.Id, ASS.Objeto, HW.Dispositivo, ASS.Port, ASS.Tipo "
								"FROM TB_DOM_ASSIGN AS ASS, TB_DOM_PERIF AS HW "
								"WHERE ASS.Dispositivo = HW.Id;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_query_result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_query_result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_query_result);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ass_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_list_all"))
			{
				message[0] = 0;

				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_ASSIGN;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_Query_Result);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ass_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_get"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_ASSIGN WHERE Id = %s;", json_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc >= 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ass_add
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_add"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;

				/* Obtengo un ID para el elemento nuevo y lo cambio en el dato recibido */
				temp_l = pDB->NextId("TB_DOM_ASSIGN", "Id");
				sprintf(temp_s, "%li", temp_l);
				cJSON_DeleteItemFromObjectCaseSensitive(json_obj, "Id");
				cJSON_AddStringToObject(json_obj, "Id", temp_s);
  
				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
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
										strcat(query_into, json_un_obj->string);
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
										strcat(query_values, json_un_obj->valuestring);
										strcat(query_values, "'");
										/* Recopilo algunos datos para actualizar la tabla de HW */
										if( !strcmp(json_un_obj->string, "Dispositivo"))
										{
											/* Mando a actualiza la configuración del HW */
											update_hw_config_id = atoi(json_un_obj->valuestring);
										}
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);

				strcat(query_into, ")");
				strcat(query_values, ")");

				sprintf(query, "INSERT INTO TB_DOM_ASSIGN %s VALUES %s;", query_into, query_values);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(NULL, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc != 1)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ass_delete
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_delete"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					if( atoi(json_Id->valuestring) != 0 )
					{
						sprintf(query, "DELETE FROM TB_DOM_ASSIGN WHERE Id = %s;", json_Id->valuestring);
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						if(rc != 0)
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid User\"}}");
					}
				}

				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ass_update
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_update"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;
				query_where[0] = 0;

				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
									{
										if( !strcmp(json_un_obj->string, "Id") )
										{
											strcpy(query_where, json_un_obj->string);
											strcat(query_where, "='");
											strcat(query_where, json_un_obj->valuestring);
											strcat(query_where, "'");

											strcpy(hw_id, json_un_obj->valuestring);
										}
										else
										{
											/* Dato = Valor */
											if(strlen(query_values) > 0)
											{
												strcat(query_values, ",");
											}
											strcat(query_values, json_un_obj->string);
											strcat(query_values, "='");
											strcat(query_values, json_un_obj->valuestring);
											strcat(query_values, "'");
										}
										/* Recopilo algunos datos para actualizar la tabla de HW */
										if( !strcmp(json_un_obj->string, "Dispositivo"))
										{
											/* Mando a actualiza la configuración del HW */
											update_hw_config_id = atoi(json_un_obj->valuestring);
										}
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);
				if(strlen(query_where))
				{
					sprintf(query, "UPDATE TB_DOM_ASSIGN SET %s WHERE %s;", query_values, query_where);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc > 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Form Data Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ass_status
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_status"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_Query_Result = cJSON_CreateArray();
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				json_Planta = cJSON_GetObjectItemCaseSensitive(json_obj, "Planta");
				if(json_Id)
				{
					sprintf(query, "SELECT Id,Objeto,Tipo,Port,Icono0,Icono1,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Estado,Perif_Data FROM TB_DOM_ASSIGN WHERE Id = %s;", json_Id->valuestring);
				}
				else
				{
					if(json_Planta)
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Port,Icono0,Icono1,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Estado,Perif_Data FROM TB_DOM_ASSIGN WHERE Planta = %s;", json_Planta->valuestring);
					}
					else
					{
						strcpy(query, "SELECT Id,Objeto,Tipo,Port,Icono0,Icono1,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Estado,Perif_Data FROM TB_DOM_ASSIGN;");
					}
				}
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					if(json_obj) cJSON_Delete(json_obj);
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
				}
				if(json_obj) cJSON_Delete(json_obj);
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ass_info
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_info"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_Query_Result = cJSON_CreateArray();
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				json_Planta = cJSON_GetObjectItemCaseSensitive(json_obj, "Planta");
				if(json_Id)
				{
					sprintf(query, "SELECT Id,Objeto,Tipo,Icono0,Icono1,Grupo_Visual,Planta,Cord_x,Cord_y FROM TB_DOM_ASSIGN WHERE Id = %s;", json_Id->valuestring);
				}
				else
				{
					if(json_Planta)
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Icono0,Icono1,Grupo_Visual,Planta,Cord_x,Cord_y FROM TB_DOM_ASSIGN WHERE Planta = %s;", json_Planta->valuestring);
					}
					else
					{
						strcpy(query, "SELECT Id,Objeto,Tipo,Icono0,Icono1,Grupo_Visual,Planta,Cord_x,Cord_y FROM TB_DOM_ASSIGN;");
					}
				}
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					if(json_obj) cJSON_Delete(json_obj);
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
				}
				if(json_obj) cJSON_Delete(json_obj);
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ass_add_to_planta
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_add_to_planta"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					sprintf(query, "UPDATE TB_DOM_ASSIGN "
									"SET Icono0 = \"lamp0.png\",Icono1 = \"lamp1.png\",Planta = 1,Cord_x = 200,Cord_y = 50 "
									"WHERE Id = %s;", json_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid Object\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ev_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ev_list"))
			{
				message[0] = 0;

				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT EV.Id, EV.Evento, EV.ON_a_OFF AS \'OFF\', EV.OFF_a_ON AS \'ON\', ASS.Objeto AS Origen "
				               "FROM TB_DOM_EVENT AS EV, TB_DOM_ASSIGN AS ASS "
							   "WHERE EV.Objeto_Origen = ASS.Id;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_Query_Result);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ev_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ev_list_all"))
			{
				message[0] = 0;

				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_EVENT;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_Query_Result);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ev_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ev_get"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_EVENT WHERE Id = %s;", json_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc >= 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ev_add
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ev_add"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;

				/* Obtengo un ID para el elemento nuevo y lo cambio en el dato recibido */
				temp_l = pDB->NextId("TB_DOM_EVENT", "Id");
				sprintf(temp_s, "%li", temp_l);
				cJSON_DeleteItemFromObjectCaseSensitive(json_obj, "Id");
				cJSON_AddStringToObject(json_obj, "Id", temp_s);
  
				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
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
										strcat(query_into, json_un_obj->string);
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
										strcat(query_values, json_un_obj->valuestring);
										strcat(query_values, "'");
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);
				/* Cierro la sentencia */
				strcat(query_into, ")");
				strcat(query_values, ")");

				sprintf(query, "INSERT INTO TB_DOM_EVENT %s VALUES %s;", query_into, query_values);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(NULL, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc != 1)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ev_delete
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ev_delete"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					if( atoi(json_Id->valuestring) != 0 )
					{
						sprintf(query, "DELETE FROM TB_DOM_EVENT WHERE Id = %s;", json_Id->valuestring);
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						if(rc != 0)
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalis User\"}}");
					}
				}

				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ev_update
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ev_update"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;
				query_where[0] = 0;

				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
									{
										if( !strcmp(json_un_obj->string, "Id") )
										{
											strcpy(query_where, json_un_obj->string);
											strcat(query_where, "='");
											strcat(query_where, json_un_obj->valuestring);
											strcat(query_where, "'");

											strcpy(hw_id, json_un_obj->valuestring);
										}
										else
										{
											/* Dato = Valor */
											if(strlen(query_values) > 0)
											{
												strcat(query_values, ",");
											}
											strcat(query_values, json_un_obj->string);
											strcat(query_values, "='");
											strcat(query_values, json_un_obj->valuestring);
											strcat(query_values, "'");

										}
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);
				if(strlen(query_where))
				{
					sprintf(query, "UPDATE TB_DOM_EVENT SET %s WHERE %s;", query_values, query_where);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Form Data Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_task_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_task_list"))
			{
				message[0] = 0;

				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT TASK.Id, Agenda, ASS.Objeto "
				               "FROM TB_DOM_AT AS TASK, TB_DOM_ASSIGN AS ASS "
							   "WHERE TASK.Objeto_Destino = ASS.Id;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_Query_Result);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_task_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_task_list_all"))
			{
				message[0] = 0;

				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_AT;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_Query_Result);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_task_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_task_get"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_AT WHERE Id = %s;", json_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc >= 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_task_add
			**************************************************************** */
			else if( !strcmp(fn, "dompi_task_add"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;

				/* Obtengo un ID para el elemento nuevo y lo cambio en el dato recibido */
				temp_l = pDB->NextId("TB_DOM_AT", "Id");
				sprintf(temp_s, "%li", temp_l);
				cJSON_DeleteItemFromObjectCaseSensitive(json_obj, "Id");
				cJSON_AddStringToObject(json_obj, "Id", temp_s);
  
				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
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
										strcat(query_into, json_un_obj->string);
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
										strcat(query_values, json_un_obj->valuestring);
										strcat(query_values, "'");
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);


				/* Cierro la sentencia */
				strcat(query_into, ")");
				strcat(query_values, ")");

				sprintf(query, "INSERT INTO TB_DOM_AT %s VALUES %s;", query_into, query_values);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(NULL, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc != 1)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_task_delete
			**************************************************************** */
			else if( !strcmp(fn, "dompi_task_delete"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					if( atoi(json_Id->valuestring) != 0 )
					{
						sprintf(query, "DELETE FROM TB_DOM_AT WHERE Id = %s;", json_Id->valuestring);
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						if(rc != 0)
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalis User\"}}");
					}
				}

				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_task_update
			**************************************************************** */
			else if( !strcmp(fn, "dompi_task_update"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;
				query_where[0] = 0;

				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
									{
										if( !strcmp(json_un_obj->string, "Id") )
										{
											strcpy(query_where, json_un_obj->string);
											strcat(query_where, "='");
											strcat(query_where, json_un_obj->valuestring);
											strcat(query_where, "'");

											strcpy(hw_id, json_un_obj->valuestring);
										}
										else
										{
											/* Dato = Valor */
											if(strlen(query_values) > 0)
											{
												strcat(query_values, ",");
											}
											strcat(query_values, json_un_obj->string);
											strcat(query_values, "='");
											strcat(query_values, json_un_obj->valuestring);
											strcat(query_values, "'");

										}
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);
				if(strlen(query_where))
				{
					sprintf(query, "UPDATE TB_DOM_AT SET %s WHERE %s;", query_values, query_where);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Form Data Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_group_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_group_list"))
			{
				message[0] = 0;

				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT Id, Grupo "
				               "FROM TB_DOM_GROUP;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_Query_Result);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_group_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_group_list_all"))
			{
				message[0] = 0;

				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_GROUP;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_Query_Result);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_group_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_group_get"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_GROUP WHERE Id = %s;", json_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc >= 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_group_add
			**************************************************************** */
			else if( !strcmp(fn, "dompi_group_add"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;

				/* Obtengo un ID para el elemento nuevo y lo cambio en el dato recibido */
				temp_l = pDB->NextId("TB_DOM_GROUP", "Id");
				sprintf(temp_s, "%li", temp_l);
				cJSON_DeleteItemFromObjectCaseSensitive(json_obj, "Id");
				cJSON_AddStringToObject(json_obj, "Id", temp_s);
  
				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
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
										strcat(query_into, json_un_obj->string);
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
										strcat(query_values, json_un_obj->valuestring);
										strcat(query_values, "'");
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);


				/* Cierro la sentencia */
				strcat(query_into, ")");
				strcat(query_values, ")");
				
				sprintf(query, "INSERT INTO TB_DOM_GROUP %s VALUES %s;", query_into, query_values);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(NULL, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc != 1)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_group_delete
			**************************************************************** */
			else if( !strcmp(fn, "dompi_group_delete"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					if( atoi(json_Id->valuestring) != 0 )
					{
						sprintf(query, "DELETE FROM TB_DOM_GROUP WHERE Id = %s;", json_Id->valuestring);
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						if(rc != 0)
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalis User\"}}");
					}
				}

				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_group_update
			**************************************************************** */
			else if( !strcmp(fn, "dompi_group_update"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;
				query_where[0] = 0;

				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
									{
										if( !strcmp(json_un_obj->string, "Id") )
										{
											strcpy(query_where, json_un_obj->string);
											strcat(query_where, "='");
											strcat(query_where, json_un_obj->valuestring);
											strcat(query_where, "'");

											strcpy(hw_id, json_un_obj->valuestring);
										}
										else
										{
											/* Dato = Valor */
											if(strlen(query_values) > 0)
											{
												strcat(query_values, ",");
											}
											strcat(query_values, json_un_obj->string);
											strcat(query_values, "='");
											strcat(query_values, json_un_obj->valuestring);
											strcat(query_values, "'");

										}
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);
				if(strlen(query_where))
				{
					sprintf(query, "UPDATE TB_DOM_GROUP SET %s WHERE %s;", query_values, query_where);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Form Data Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_sysconf_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_sysconf_list"))
			{
				message[0] = 0;

				json_Query_Result = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_CONFIG;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_Query_Result);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_sysconf_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_sysconf_get"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_CONFIG WHERE Id = %s;", json_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc >= 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_sysconf_get_current
			**************************************************************** */
			else if( !strcmp(fn, "dompi_sysconf_get_current"))				
			{
				json_obj = cJSON_CreateObject();
				cJSON_AddItemReferenceToObject(json_obj, "response", json_System_Config);
				cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
				cJSON_Delete(json_obj);
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_sysconf_add
			**************************************************************** */
			else if( !strcmp(fn, "dompi_sysconf_add"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;

				/* Obtengo un ID para el elemento nuevo y lo cambio en el dato recibido */
				temp_l = pDB->NextId("TB_DOM_CONFIG", "Id");
				sprintf(temp_s, "%li", temp_l);
				cJSON_DeleteItemFromObjectCaseSensitive(json_obj, "Id");
				cJSON_AddStringToObject(json_obj, "Id", temp_s);
  
				t = time(&t);
				lt = localtime(&t);

				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
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
										strcat(query_into, json_un_obj->string);
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
										if( !strcmp(json_un_obj->string, "Creacion"))
										{
											sprintf(&query_values[strlen(query_values)], "%04i/%02i/%02i %02i:%02i:%02i", 
												lt->tm_year + 1900, lt->tm_mon+1, lt->tm_mday,
												lt->tm_hour, lt->tm_min, lt->tm_sec);
										}
										else
										{
											strcat(query_values, json_un_obj->valuestring);
										}
										strcat(query_values, "'");
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);

				strcat(query_into, ")");
				strcat(query_values, ")");

				load_system_config = 1;
				sprintf(query, "INSERT INTO TB_DOM_CONFIG %s VALUES %s;", query_into, query_values);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(NULL, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc != 1)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_list"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Tipo");
				if(json_un_obj)
				{
					json_query_result = cJSON_CreateArray();
					sprintf(query, "SELECT AU.Id AS Id, AU.Objeto AS Grupo, ASS.Objeto AS Salida, AU.Estado AS Estado "
									"FROM TB_DOM_AUTO AS AU, TB_DOM_ASSIGN AS ASS "
									"WHERE (AU.Objeto_Salida = ASS.Id AND AU.Id = 0) OR "
										"(AU.Objeto_Salida = ASS.Id AND AU.Tipo = %s);", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_query_result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc >= 0)
					{
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_query_result);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
						cJSON_Delete(json_obj);
					}
					else
					{
						cJSON_Delete(json_query_result);
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Falta dato Tipo\"}}");					
				}
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_list_all"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Tipo");
				if(json_un_obj)
				{
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT * "
									"FROM TB_DOM_AUTO "
									"WHERE Id = 0 OR Tipo = %s;", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc >= 0)
					{
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
						cJSON_Delete(json_obj);
					}
					else
					{
						cJSON_Delete(json_Query_Result);
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Falta dato Tipo\"}}");					
				}
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_get"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_AUTO WHERE Id = %s;", json_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc >= 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_add
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_add"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;

				/* Obtengo un ID para el elemento nuevo y lo cambio en el dato recibido */
				temp_l = pDB->NextId("TB_DOM_AUTO", "Id");
				sprintf(temp_s, "%li", temp_l);
				cJSON_DeleteItemFromObjectCaseSensitive(json_obj, "Id");
				cJSON_AddStringToObject(json_obj, "Id", temp_s);
  
				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
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
										strcat(query_into, json_un_obj->string);
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
										if( !strcmp(json_un_obj->string, "Actualizar"))
										{
											strcat(query_values, "1");
										}
										else
										{ 
											strcat(query_values, json_un_obj->valuestring);
										}
										strcat(query_values, "'");
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);

				strcat(query_into, ")");
				strcat(query_values, ")");

				sprintf(query, "INSERT INTO TB_DOM_AUTO %s VALUES %s;", query_into, query_values);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(NULL, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc != 1)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_delete
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_delete"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					if( atoi(json_Id->valuestring) != 0 )
					{
						sprintf(query, "DELETE FROM TB_DOM_AUTO WHERE Id = %s;", json_Id->valuestring);
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						if(rc != 0)
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid User\"}}");
					}
				}

				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_update
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_update"))
			{
				json_obj = cJSON_Parse(message);

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;
				query_where[0] = 0;

				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExcluirDeABM(json_un_obj->string) == 0)
									{
										if( !strcmp(json_un_obj->string, "Id") )
										{
											strcpy(query_where, json_un_obj->string);
											strcat(query_where, "='");
											strcat(query_where, json_un_obj->valuestring);
											strcat(query_where, "'");

											strcpy(hw_id, json_un_obj->valuestring);
										}
										else
										{
											/* Dato = Valor */
											if(strlen(query_values) > 0)
											{
												strcat(query_values, ",");
											}
											strcat(query_values, json_un_obj->string);
											strcat(query_values, "='");
											if( !strcmp(json_un_obj->string, "Actualizar"))
											{
												strcat(query_values, "1");
											}
											else
											{ 
												strcat(query_values, json_un_obj->valuestring);
											}
											strcat(query_values, "'");
										}
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);
				if(strlen(query_where))
				{
					sprintf(query, "UPDATE TB_DOM_AUTO SET %s WHERE %s;", query_values, query_where);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc > 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Form Data Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
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


		/* Marcar para actualizar configuracion todos los assign de un periferico por Id */
		if(update_hw_config_id)
		{
			m_pServer->m_pLog->Add(50, "Actualizar configuracion de HW Id: %i", update_hw_config_id);
			sprintf(query, "UPDATE TB_DOM_PERIF "
							"SET Actualizar = 1 "
							"WHERE Id = %i;", update_hw_config_id);
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(NULL, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			update_hw_config_id = 0;
		}

		if(load_system_config)
		{
			LoadSystemConfig();
			/* Mando a todos a releer la configuración */
			m_pServer->Post("dompi_reload_config", nullptr, 0);
		}






	}
	m_pServer->m_pLog->Add(1, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);

	m_pServer->UnSuscribe("dompi_db_struct", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_user_list", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_user_list_all", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_user_get", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_user_add", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_user_delete", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_user_update", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_user_check", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_list", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_list_all", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_get", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_add", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_delete", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_hw_update", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_list", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_list_all", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_get", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_add", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_delete", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_update", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_status", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_add_to_planta", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_list", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_list_all", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_get", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_add", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_delete", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_update", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_task_list", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_task_list_all", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_task_get", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_task_add", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_task_delete", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_task_update", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_group_list", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_group_list_all", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_group_get", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_group_add", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_group_delete", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_group_update", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_sysconf_list", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_sysconf_get", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_sysconf_get_current", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_sysconf_add", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_list", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_list_all", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_get", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_add", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_delete", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_update", GM_MSG_TYPE_CR);

	delete pConfig;
	delete pDB;

	exit(0);
}

int ExcluirDeABM(const char* label)
{
	if( !strcmp(label, "REMOTE_ADDR")) return 1;
	if( !strcmp(label, "REQUEST_URI")) return 1;
	if( !strcmp(label, "REQUEST_METHOD")) return 1;
	if( !strcmp(label, "CONTENT_LENGTH")) return 1;
	if( !strcmp(label, "GET")) return 1;
	if( !strcmp(label, "POST")) return 1;
	return 0;
}

void LoadSystemConfig(void)
{
	char query[4096];
	int rc;

	if(pDB == NULL) return;

	if(json_System_Config) cJSON_Delete(json_System_Config);
	json_System_Config = cJSON_CreateArray();
	strcpy(query, "SELECT * FROM TB_DOM_CONFIG ORDER BY Id DESC LIMIT 1;");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_System_Config, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc >= 0) m_pServer->m_pLog->Add(1, "[LoadSystemConfig] Lectura de configuracion OK.");
}