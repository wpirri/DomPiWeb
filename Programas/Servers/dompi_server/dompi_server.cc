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
/*
Informaciòn de desarrollo sobre Raspberry Pi

https://pinout.xyz/
http://diymakers.es/usando-el-puerto-gpio/
http://wiringpi.com/reference/


instalar libcjson-dev (https://github.com/DaveGamble/cJSON)
#include <cjson/cJSON.h>
-lcjson

char *p;
cJSON *obj;
cJSON *str;
cJSON *num;
cJSON *arr;
obj = cJSON_CreateObject();


-- String ---------------------------------------------------------------------
obj = cJSON_Parse(const char *value);

-- String ---------------------------------------------------------------------
str = cJSON_CreateString("un string");
cJSON_AddItemToObject(obj, "nombre", str);
o
cJSON_AddStringToObject(obj, "nombre", "un string")

-- Number ---------------------------------------------------------------------
num = cJSON_CreateNumber(50);
cJSON_AddItemToObject(obj, "edad", num);
o
cJSON_AddNumberToObject(obj, edad, 50);

-- Array ----------------------------------------------------------------------
arr = cJSON_CreateArray();
cJSON_AddItemToArray(arr, obj);
cJSON_AddItemToObject(obj, "nombre_array", arr);
o
arr = cJSON_AddArrayToObject(obj, "nombre_arr");

-- Generado -------------------------------------------------------------------
p = cJSON_Print(obj);
p = cJSON_PrintUnformatted(obj)
cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format);

-- Obtener --------------------------------------------------------------------
un_obj = cJSON_GetObjectItemCaseSensitive(obj, "nombre");
un_obj = cJSON_GetObjectItemCaseSensitive(obj, "edad");
    cJSON_IsString(un_obj)
    name->valuestring

    cJSON_IsNumber
    width->valuedouble

-- Free -----------------------------------------------------------------------
cJSON_Delete(obj);


#define cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

*/

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
#include "csqlite.h"
#include "gevent.h"
#include "strfunc.h"

CGMServerWait *m_pServer;
DPConfig *pConfig;
CSQLite *pDB;
GEvent *pEV;

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

/*                            11111111112222222222333333333344444444445555555555666666666677777777778
                     12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
char cli_help[] = 	"-------------------------------------------------------------------------------\r\n"
					"Comandos disponibles:\r\n"
					"  encender <objeto>\r\n"
					"  apagar <objeto>\r\n"
					"  pulso <objeto>\r\n"
					"  estado <objeto>\r\n"
					"  actualizar <dispositivo>, [modulo]\r\n"
					"  modulo <dispositivo>, [modulo]\r\n"
					"  help\r\n"
					"  * objeto: Nombre de un objeto existente.\r\n"
					"    dispositivo: Nombre de un dispositivo existente.\r\n"
					"    modulo: config, wifi, porta, portb o portc\r\n"
					"-------------------------------------------------------------------------------\r\n";

void OnClose(int sig);

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[4096];
	char cmdline[1024];
	char query[4096];
	char query_into[1024];
	char query_values[2048];
	char query_where[512];
	unsigned long message_len;
	char db_filename[FILENAME_MAX+1];
	int checked;
	char hw_id[16];
	char update_hw_config[16];
	char update_hw_status[16];
	long temp_l;
	char temp_s[64];

	char comando[1024];
	char objeto[1024];
	char parametro[1024];

	STRFunc Strf;
	CGMServerBase::GMIOS call_resp;

    cJSON *json_obj;
    cJSON *json_un_obj;
    cJSON *json_arr = NULL;
    cJSON *json_user;
    cJSON *json_pass;
    cJSON *json_channel;
    cJSON *json_query;
    cJSON *json_query_result;
    //cJSON *json_response;
    //cJSON *json_response_password;
    cJSON *json_hw_id;
    cJSON *json_cmdline;

	char ass_s_disp[128];
	int ass_i_port;
	int ass_i_e_s;
	int ass_i_tipo;

	update_hw_config[0] = 0;
	update_hw_status[0] = 0;

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
	m_pServer->Init("dompi_server");
	m_pServer->m_pLog->Add(1, "Iniciando Servidor de Domotica...");

	m_pServer->m_pLog->Add(1, "Leyendo configuración...");
	pConfig = new DPConfig("/etc/dompiweb.config");
	pConfig->GetParam("SQLITE_DB_FILENAME", db_filename);

	m_pServer->m_pLog->Add(1, "Conectando a la base de datos %s...", db_filename);
	pDB = new CSQLite(db_filename);
	if(pDB->Open() != 0)
	{
		m_pServer->m_pLog->Add(1, "ERROR al conectar con la base de datos");
		OnClose(0);
	}
	else
	{
		m_pServer->m_pLog->Add(1, "Conectado a la base de datos %s", db_filename);
	}

	pEV = new GEvent(pDB, m_pServer);

	m_pServer->Suscribe("dompi_infoio", GM_MSG_TYPE_CR);
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
	m_pServer->Suscribe("dompi_ev_list", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_list_all", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_get", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_add", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_delete", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_update", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_cmdline", GM_MSG_TYPE_CR);

	m_pServer->m_pLog->Add(1, "Servicios de Domotica inicializados.");

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, (-1) )) >= 0)
	{
		if(rc > 0)
		{
			json_query_result = NULL;
			message[message_len] = 0;
			m_pServer->m_pLog->Add(50, "%s:(Q)[%s]", fn, message);
			/* ****************************************************************
			*		dompi_infoio - Notificacion de estado y/o cambio de I/O
			**************************************************************** */
			if( !strcmp(fn, "dompi_infoio"))
			{
				json_obj = cJSON_Parse(message);

				json_hw_id = cJSON_GetObjectItemCaseSensitive(json_obj, "HW_ID");
				if(json_hw_id)
				{
					rc = pEV->ExtIOEvent(message);
					if(rc != 1)
					{
						m_pServer->m_pLog->Add(100, "Error %i en ExtIOEvent()", rc);
					}
					if(rc == 1)
					{
						/* OK */
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
						/* Si está todo bien me fijo si pidio enviar configuracion */
						json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "GETCONF");
						if(json_un_obj)
						{
							if( atoi(json_un_obj->valuestring) > 0 )
							{
								strcpy(update_hw_config, json_hw_id->valuestring);
							}
						}
					}
					else if(rc == 0)
					{
						/* NOT FOUND */
						m_pServer->m_pLog->Add(10, "HW: %s No encontrado en la base", json_hw_id->valuestring);
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Not Found\"}}");
					}
					else
					{
						/* Otro Error */
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"General Error\"}}");
					}
				}
				else
				{
					/* El mensaje vino sin HWID */
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Not Found\"}}");
				}
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}

				cJSON_Delete(json_obj);

			}
			/* ****************************************************************
			*		dompi_db_struct
			**************************************************************** */
			else if( !strcmp(fn, "dompi_db_struct"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "table");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, ".schema %s", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc == 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, 4095, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}

			}
			/* ****************************************************************
			*		dompi_user_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_list"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT Nombre, Usuario, Estado, Ultimo_Acceso FROM TB_DOM_USER;");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, 4095, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_arr);
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_user_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_list_all"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_USER;");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, 4095, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_arr);
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_user_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_get"))				/* ***** dompi_user_get ***** */
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Nombre");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_USER WHERE Nombre = \'%s\';", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc == 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, 4095, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
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
// cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)
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
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);

				strcat(query_into, ")");
				strcat(query_values, ")");
				sprintf(query, "INSERT INTO TB_DOM_USER %s VALUES %s;", query_into, query_values);
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);

				rc = pDB->Query(NULL, query);
				if(rc != 0)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_user_delete
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_delete"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Nombre");
				if(json_un_obj)
				{
					if( strcmp(json_un_obj->valuestring, "admin") )
					{
						sprintf(query, "DELETE FROM TB_DOM_USER WHERE Nombre = \'%s\';", json_un_obj->valuestring);
						m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
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

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
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
// cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)
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
									if( !strcmp(json_un_obj->string, "Nombre") )
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
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);
				if(strlen(query_where))
				{
					sprintf(query, "UPDATE TB_DOM_USER SET %s WHERE %s;", query_values, query_where);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					if(rc == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Form Data Error\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
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
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT Permisos,Dias,Horas,Estado,Contador_Error,"
					"Pin_Teclado,Pin_SMS,Pin_WEB FROM TB_DOM_USER WHERE Nombre = \'%s\';", json_user->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc == 0)
					{
						//json_response = cJSON_GetObjectItemCaseSensitive(json_arr, "response");

						if( !strcmp(json_channel->valuestring, "web"))
						{
							//json_response_password = cJSON_GetObjectItemCaseSensitive(json_arr, "response");
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
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_hw_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_hw_list"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT Id, Dispositivo, Tipo, Estado FROM TB_DOM_PERIF;");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, 4095, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_arr);
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_hw_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_hw_list_all"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_PERIF;");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, 4095, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_arr);
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_hw_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_hw_get"))				/* ***** dompi_hw_get ***** */
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_PERIF WHERE Id = \'%s\';", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc == 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, 4095, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
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
// cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)
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
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);

				strcat(query_into, ")");
				strcat(query_values, ")");
				sprintf(query, "INSERT INTO TB_DOM_PERIF %s VALUES %s;", query_into, query_values);
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);

				rc = pDB->Query(NULL, query);
				if(rc != 0)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_hw_delete
			**************************************************************** */
			else if( !strcmp(fn, "dompi_hw_delete"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					if( memcmp(json_un_obj->valuestring, "00", 2) )
					{
						sprintf(query, "DELETE FROM TB_DOM_PERIF WHERE Id = \'%s\';", json_un_obj->valuestring);
						m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
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

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
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

										if( !memcmp("Config_", json_un_obj->string, 7))
										{
											strcpy(update_hw_config, hw_id);
										}
										if( !memcmp("Estado_", json_un_obj->string, 7))
										{
											strcpy(update_hw_status, hw_id);
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
					sprintf(query, "UPDATE TB_DOM_PERIF SET %s WHERE %s;", query_values, query_where);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					if(rc == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Form Data Error\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}

			}
			/* ****************************************************************
			*		dompi_ass_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_list"))
			{
				message[0] = 0;

				json_query_result = cJSON_CreateArray();
				strcpy(query, "SELECT ASS.Id, ASS.Objeto, HW.Dispositivo, ASS.Port, ASS.E_S, ASS.Tipo "
								"FROM TB_DOM_ASSIGN AS ASS, TB_DOM_PERIF AS HW "
								"WHERE ASS.Dispositivo = HW.Id;");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_query_result, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_query_result);
					cJSON_PrintPreallocated(json_obj, message, 4095, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_query_result);
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_ass_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_list_all"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_ASSIGN;");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, 4095, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_arr);
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_ass_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_get"))				/* ***** dompi_hw_get ***** */
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_ASSIGN WHERE Id = \'%s\';", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc == 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, 4095, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
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
// cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)
				json_un_obj = json_obj;

				/* Obtengo un ID para el elemento nuevo y lo cambio en el dato recibido */
				temp_l = pDB->NextId("TB_DOM_ASSIGN", "Id");
				sprintf(temp_s, "%li", temp_l);
				cJSON_AddStringToObject(json_un_obj, "Id", temp_s);
  
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
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);

				strcat(query_into, ")");
				strcat(query_values, ")");
				sprintf(query, "INSERT INTO TB_DOM_ASSIGN %s VALUES %s;", query_into, query_values);
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);

				rc = pDB->Query(NULL, query);
				if(rc != 0)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_ass_delete
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_delete"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					if( atoi(json_un_obj->valuestring) != 0 )
					{
						sprintf(query, "DELETE FROM TB_DOM_ASSIGN WHERE Id = \'%s\';", json_un_obj->valuestring);
						m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
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

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_ass_update
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_update"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;
				query_where[0] = 0;

				json_un_obj = json_obj;
				ass_s_disp[0] = 0;
				ass_i_e_s = 0;
				ass_i_port = 0;
				ass_i_tipo = 0;
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
										strcpy(ass_s_disp, json_un_obj->valuestring);
									}
									else if( !strcmp(json_un_obj->string, "Port"))
									{
										ass_i_port = atoi(json_un_obj->valuestring);
									}
									else if( !strcmp(json_un_obj->string, "E_S"))
									{
										ass_i_e_s = atoi(json_un_obj->valuestring);
									}
									else if( !strcmp(json_un_obj->string, "Tipo"))
									{
										ass_i_tipo = atoi(json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					if(rc == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
					else
					{
						/* Actualizo la tabla de HW */
						if(strlen(ass_s_disp) && ass_i_port && ass_i_e_s )
						{
							if(ass_i_tipo == 1)
							{ 
								sprintf(query, "UPDATE TB_DOM_PERIF "
											"SET Actualizar = 1, "
											    "Config_PORT_%c_E_S = (SELECT Config_PORT_%c_E_S "
																	 "FROM TB_DOM_PERIF WHERE Id = '%s') | %i "
											"WHERE Id = '%s';",
											(ass_i_port == 1)?'A':(ass_i_port == 2)?'B':'C',
											(ass_i_port == 1)?'A':(ass_i_port == 2)?'B':'C',
											ass_s_disp,
											power2(ass_i_e_s-1),
											ass_s_disp);
							}
							else /* ass_i_tipo == 0 */
							{
								sprintf(query, "UPDATE TB_DOM_PERIF "
											"SET Actualizar = 1, "
											    "Config_PORT_%c_E_S = (SELECT Config_PORT_%c_E_S "
																	 "FROM TB_DOM_PERIF WHERE Id = '%s') & %i "
											"WHERE Id = '%s';",
											(ass_i_port == 1)?'A':(ass_i_port == 2)?'B':'C',
											(ass_i_port == 1)?'A':(ass_i_port == 2)?'B':'C',
											ass_s_disp,
											power2(ass_i_e_s-1)^0xFF,
											ass_s_disp);
							}
							m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
							rc = pDB->Query(NULL, query);
							if(rc == 0)
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
							}
						}
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Form Data Error\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}

			}
			/* ****************************************************************
			*		dompi_ev_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ev_list"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT EV.Id, EV.Evento, EV.ON_a_OFF AS \'OFF\', EV.OFF_a_ON AS \'ON\', ASS.Objeto AS Origen "
				               "FROM TB_DOM_EVENT AS EV, TB_DOM_ASSIGN AS ASS "
							   "WHERE EV.Objeto_Origen = ASS.Id;");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, 4095, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_arr);
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_ev_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ev_list_all"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_EVENT;");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, 4095, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_arr);
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_ev_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ev_get"))				/* ***** dompi_hw_get ***** */
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_EVENT WHERE Id = \'%s\';", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc == 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, 4095, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
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
// cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)
				json_un_obj = json_obj;

				/* Obtengo un ID para el elemento nuevo y lo cambio en el dato recibido */
				temp_l = pDB->NextId("TB_DOM_EVENT", "Id");
				sprintf(temp_s, "%li", temp_l);
				cJSON_AddStringToObject(json_un_obj, "Id", temp_s);
  
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
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);

				strcat(query_into, ")");
				strcat(query_values, ")");
				sprintf(query, "INSERT INTO TB_DOM_EVENT %s VALUES %s;", query_into, query_values);
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);

				rc = pDB->Query(NULL, query);
				if(rc != 0)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_ev_delete
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ev_delete"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					if( atoi(json_un_obj->valuestring) != 0 )
					{
						sprintf(query, "DELETE FROM TB_DOM_EVENT WHERE Id = \'%s\';", json_un_obj->valuestring);
						m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
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

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
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
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);
				if(strlen(query_where))
				{
					sprintf(query, "UPDATE TB_DOM_EVENT SET %s WHERE %s;", query_values, query_where);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					if(rc == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Form Data Error\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			/* ****************************************************************
			*		dompi_cmdline
			**************************************************************** */
			else if( !strcmp(fn, "dompi_cmdline"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				/* *********************************************************** */
				json_query = cJSON_GetObjectItemCaseSensitive(json_obj, "query");
				if(json_query)
				{
					json_cmdline = cJSON_GetObjectItemCaseSensitive(json_query, "CmdLine");
					if(json_cmdline)
					{
						strcpy(cmdline, json_cmdline->valuestring);

						Strf.ParseCommand(cmdline, comando, objeto, parametro);

						m_pServer->m_pLog->Add(100, "[dompi_cmdline] Comando: %s - Objeto: %s - Parametro: %s", 
											(comando)?comando:"NULL", 
											(objeto)?objeto:"NULL", 
											(parametro)?parametro:"NULL");

						if( !strcmp(comando, "help") || !strcmp(comando, "?"))
						{
							sprintf(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"%s\"}}", cli_help);
						}
						else if( !strcmp(comando, "encender") )
						{
							json_arr = cJSON_CreateArray();
							sprintf(query, "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port, ASS.E_S "
											"FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
											"WHERE HW.Id = ASS.Dispositivo AND "
											"ASS.Objeto =  \'%s\';", objeto);
							m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
							rc = pDB->Query(json_arr, query);
							if(rc == 0)
							{
								/* Creo un objeto con el primer item del array */
								json_un_obj = json_arr->child;
								cJSON_AddStringToObject(json_un_obj, "Estado", "1");
								cJSON_PrintPreallocated(json_un_obj, message, 4096, 0);
								m_pServer->m_pLog->Add(50, "[dompi_hw_set_io][%s]", message);
								m_pServer->Call("dompi_hw_set_io", message, strlen(message), &call_resp, 500);
								strcpy(message, (const char*)call_resp.data);
								m_pServer->Free(call_resp);
							}
						}
						else if( !strcmp(comando, "apagar") )
						{
							json_arr = cJSON_CreateArray();
							sprintf(query, "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port, ASS.E_S "
											"FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
											"WHERE HW.Id = ASS.Dispositivo AND "
											"ASS.Objeto =  \'%s\';", objeto);
							m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
							rc = pDB->Query(json_arr, query);
							if(rc == 0)
							{
								/* Creo un objeto con el primer item del array */
								json_un_obj = json_arr->child;
								cJSON_AddStringToObject(json_un_obj, "Estado", "0");
								cJSON_PrintPreallocated(json_un_obj, message, 4096, 0);
								m_pServer->m_pLog->Add(50, "[dompi_hw_set_io][%s]", message);
								m_pServer->Call("dompi_hw_set_io", message, strlen(message), &call_resp, 500);
								strcpy(message, (const char*)call_resp.data);
								m_pServer->Free(call_resp);
							}
						}
						else if( !strcmp(comando, "estado") )
						{
							json_arr = cJSON_CreateArray();
							sprintf(query, "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port, ASS.E_S "
											"FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
											"WHERE HW.Id = ASS.Dispositivo AND "
											"ASS.Objeto =  \'%s\';", objeto);
							m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
							rc = pDB->Query(json_arr, query);
							if(rc == 0)
							{
								/* Creo un objeto con el primer item del array */
								json_un_obj = json_arr->child;
								cJSON_PrintPreallocated(json_un_obj, message, 4096, 0);
								m_pServer->m_pLog->Add(50, "[dompi_hw_set_io][%s]", message);
								m_pServer->Call("dompi_hw_get_io", message, strlen(message), &call_resp, 500);
								strcpy(message, (const char*)call_resp.data);
								m_pServer->Free(call_resp);
							}
						}
						else if( !strcmp(comando, "actualizar") )
						{
							if( !memcmp(parametro, "conf", 4))
							{
								json_arr = cJSON_CreateArray();
								sprintf(query, "SELECT Direccion_IP, Tipo AS Tipo_HW, "
												"Config_PORT_A_Analog, Config_PORT_A_E_S, "
												"Config_PORT_B_Analog, Config_PORT_B_E_S, "
												"Config_PORT_C_Analog, Config_PORT_C_E_S "
												"FROM TB_DOM_PERIF "
												"WHERE Dispositivo =  \'%s\';", objeto);
								m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
								rc = pDB->Query(json_arr, query);
								if(rc == 0)
								{
									/* Creo un objeto con el primer item del array */
									json_un_obj = json_arr->child;
									cJSON_PrintPreallocated(json_un_obj, message, 4096, 0);
									m_pServer->m_pLog->Add(50, "[dompi_hw_set_port_config][%s]", message);
									m_pServer->Call("dompi_hw_set_port_config", message, strlen(message), &call_resp, 500);
									strcpy(message, (const char*)call_resp.data);
									m_pServer->Free(call_resp);
								}
							}
							else if( !strcmp(parametro, "wifi"))
							{

							}
							else if( !strcmp(parametro, "estado"))
							{
								json_arr = cJSON_CreateArray();
								sprintf(query, "SELECT Direccion_IP, Tipo AS Tipo_HW, "
												"Estado_PORT_A, Estado_PORT_B, Estado_PORT_C "
												"FROM TB_DOM_PERIF "
												"WHERE Dispositivo =  \'%s\';", objeto);
								m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
								rc = pDB->Query(json_arr, query);
								if(rc == 0)
								{
									/* Creo un objeto con el primer item del array */
									json_un_obj = json_arr->child;
									cJSON_PrintPreallocated(json_un_obj, message, 4096, 0);
									m_pServer->m_pLog->Add(50, "[dompi_hw_set_port][%s]", message);
									m_pServer->Call("dompi_hw_set_port", message, strlen(message), &call_resp, 500);
									strcpy(message, (const char*)call_resp.data);
									m_pServer->Free(call_resp);
								}
							}
						}
						else if( !strcmp(comando, "modulo") )
						{
							if( !memcmp(parametro, "conf", 4))
							{
								json_arr = cJSON_CreateArray();
								sprintf(query, "SELECT Direccion_IP, Tipo AS Tipo_HW "
												"FROM TB_DOM_PERIF "
												"WHERE Dispositivo =  \'%s\';", objeto);
								m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
								rc = pDB->Query(json_arr, query);
								if(rc == 0)
								{
									/* Creo un objeto con el primer item del array */
									json_un_obj = json_arr->child;
									cJSON_PrintPreallocated(json_un_obj, message, 4096, 0);
									m_pServer->m_pLog->Add(50, "[dompi_hw_get_port_config][%s]", message);
									m_pServer->Call("dompi_hw_get_port_config", message, strlen(message), &call_resp, 500);
									strcpy(message, (const char*)call_resp.data);
									m_pServer->Free(call_resp);
								}
							}
							else if( !strcmp(parametro, "wifi"))
							{
								json_arr = cJSON_CreateArray();
								sprintf(query, "SELECT Direccion_IP, Tipo AS Tipo_HW "
												"FROM TB_DOM_PERIF "
												"WHERE Dispositivo =  \'%s\';", objeto);
								m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
								rc = pDB->Query(json_arr, query);
								if(rc == 0)
								{
									/* Creo un objeto con el primer item del array */
									json_un_obj = json_arr->child;
									cJSON_PrintPreallocated(json_un_obj, message, 4096, 0);
									m_pServer->m_pLog->Add(50, "[dompi_hw_get_comm_config][%s]", message);
									m_pServer->Call("dompi_hw_get_comm_config", message, strlen(message), &call_resp, 500);
									strcpy(message, (const char*)call_resp.data);
									m_pServer->Free(call_resp);
								}
							}
							else if( !strcmp(parametro, "estado"))
							{
								json_arr = cJSON_CreateArray();
								sprintf(query, "SELECT Direccion_IP, Tipo AS Tipo_HW "
												"FROM TB_DOM_PERIF "
												"WHERE Dispositivo =  \'%s\';", objeto);
								m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
								rc = pDB->Query(json_arr, query);
								if(rc == 0)
								{
									/* Creo un objeto con el primer item del array */
									json_un_obj = json_arr->child;
									cJSON_PrintPreallocated(json_un_obj, message, 4096, 0);
									m_pServer->m_pLog->Add(50, "[dompi_hw_get_port][%s]", message);
									m_pServer->Call("dompi_hw_get_port", message, strlen(message), &call_resp, 500);
									strcpy(message, (const char*)call_resp.data);
									m_pServer->Free(call_resp);
								}
							}
						}










						//json_response = cJSON_CreateObject();
						//cJSON_AddStringToObject(json_response, "ReturnMessage", message);
						//message[0] = 0;
						//cJSON_AddItemToObject(json_obj, "response", json_response);
						//cJSON_PrintPreallocated(json_obj, message, 4096, 0);
					}
				}
				/* *********************************************************** */
				cJSON_Delete(json_obj);
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}

			else
			{
				m_pServer->m_pLog->Add(50, "GME_SVC_NOTFOUND");
				m_pServer->m_pLog->Add(50, "[%s][R][GME_SVC_NOTFOUND]");
				m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
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

	m_pServer->UnSuscribe("dompi_infoio", GM_MSG_TYPE_CR);
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
	m_pServer->UnSuscribe("dompi_ev_list", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_list_all", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_get", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_add", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_delete", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_update", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_cmdline", GM_MSG_TYPE_CR);

	delete pEV;
	delete pConfig;
	delete pDB;
	delete m_pServer;
	exit(0);
}
