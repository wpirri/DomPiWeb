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

#define MAX_BUFFER_LEN 32767

CGMServerWait *m_pServer;
DPConfig *pConfig;
CSQLite *pDB;
GEvent *pEV;
cJSON *json_System_Config;
int load_system_config;
int update_system_config;
int internal_timeout;

bool run_dbmant;
void DBMant( char* msg );

void LoadSystemConfig(void)
{
	char query[4096];
	int rc;

	if(pDB == NULL) return;

	if(json_System_Config) cJSON_Delete(json_System_Config);
	json_System_Config = cJSON_CreateArray();
	strcpy(query, "SELECT * FROM TB_DOM_CONFIG ORDER BY Id DESC LIMIT 1");
	m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
	rc = pDB->Query(json_System_Config, query);
	if(rc == 0)
	{
		load_system_config = 0;
		update_system_config = 1;
	}
}

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
					"  cambiar <objeto>\r\n"
					"  pulso <objeto>, [segundos]\r\n"
					"  estado <objeto>\r\n"
					"  actualizar <dispositivo>, [modulo]\r\n"
					"  modulo <dispositivo>, [modulo]\r\n"
					"  manten\r\n"
					"  help\r\n"
					"  * objeto: Nombre de un objeto existente.\r\n"
					"    dispositivo: Nombre de un dispositivo existente.\r\n"
					"    modulo: config, wifi, porta, portb o portc\r\n"
					"    segundos: duracion en segundos. Si no se especifica el default es 1.\r\n"
					"-------------------------------------------------------------------------------\r\n";

void OnClose(int sig);

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[MAX_BUFFER_LEN+1];
	char cmdline[1024];
	char query[4096];
	char query_into[1024];
	char query_values[2048];
	char query_where[512];
	unsigned long message_len;
	char db_filename[FILENAME_MAX+1];
	int checked;
	char hw_id[16];
	char update_hw_config_id[8];
	char update_hw_config_mac[16];
	char update_hw_status[16];
	long temp_l;
	char temp_s[64];
	time_t t;
	time_t next_t;
	int delta_t;
	time_t update_ass_t;
	char s[16];

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
    cJSON *json_cmdline;
    //cJSON *json_resp_code;

    cJSON *json_HW_Id;
    cJSON *json_MAC;
	cJSON *json_Direccion_IP;
	cJSON *json_Tipo_HW;
	//cJSON *json_Tipo_ASS;
	//cJSON *json_Port;
	//cJSON *json_E_S;
	//cJSON *json_Estado;
	//cJSON *json_AN_Config;
	//cJSON *json_IO_Config;
	cJSON *json_Config_PORT_A_Analog;
	cJSON *json_Config_PORT_A_E_S;
	//cJSON *json_Config_PORT_B_Analog;
	cJSON *json_Config_PORT_B_E_S;
	//cJSON *json_Config_PORT_C_Analog;
	//cJSON *json_Config_PORT_C_E_S;
	cJSON *json_Flags;
	cJSON *json_Objeto;
	cJSON *json_Accion;

	char ass_s_disp[128];
	int ass_i_port;
	int ass_i_e_s;
	int ass_i_tipo;

	update_hw_config_id[0] = 0;
	update_hw_config_mac[0] = 0;
	update_hw_status[0] = 0;
	load_system_config = 1;
	update_system_config = 0;
	run_dbmant = false;

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

	internal_timeout = 1000;
	if( pConfig->GetParam("INTERNAL-TIMEOUT", s))
	{
		internal_timeout = atoi(s) * 1000;
	}

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

	json_System_Config = NULL;
	LoadSystemConfig();

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
	m_pServer->Suscribe("dompi_ass_status", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_info", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_on", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_off", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_switch", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_pulse", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_add_to_planta", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_cmd", GM_MSG_TYPE_MSG);
	m_pServer->Suscribe("dompi_ev_list", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_list_all", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_get", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_add", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_delete", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ev_update", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_cmdline", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_sysconf_list", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_sysconf_get", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_sysconf_get_current", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_sysconf_add", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_cloud_notification", GM_MSG_TYPE_MSG);

	m_pServer->m_pLog->Add(1, "Servicios de Domotica inicializados.");

	t = time(&t);
	next_t = t + 10;
	delta_t = 500;
	update_ass_t = t + 600;

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, delta_t )) >= 0)
	{
		t = time(&t);
		next_t = t + 10;
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
				//message[0] = 0;

				json_HW_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "HW_ID");
				if(json_HW_Id)
				{
					rc = pEV->ExtIOEvent(message);
					message[0] = 0;
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
								m_pServer->m_pLog->Add(10, "HW %s Solicita configuracion", json_HW_Id->valuestring);
								strcpy(update_hw_config_mac, json_HW_Id->valuestring);
							}
						}
					}
					else if(rc == 0)
					{
						/* NOT FOUND */
						m_pServer->m_pLog->Add(10, "HW: %s No encontrado en la base", json_HW_Id->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_infoio]");
				}

				cJSON_Delete(json_obj);

			}
			/* ****************************************************************
			*		dompi_statusio
			**************************************************************** */
			else if( !strcmp(fn, "dompi_statusio"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;





				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_statusio]");
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
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_db_struct]");
				}

			}
			/* ****************************************************************
			*		dompi_user_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_list"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT Id, Usuario, Nombre_Completo, Estado, Ultimo_Acceso FROM TB_DOM_USER");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_user_list]");
				}
			}
			/* ****************************************************************
			*		dompi_user_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_list_all"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_USER");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_user_list_all]");
				}
			}
			/* ****************************************************************
			*		dompi_user_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_get"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_USER WHERE Id = \'%s\'", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc == 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_user_get]");
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

				/* Obtengo un ID para el elemento nuevo y lo cambio en el dato recibido */
				temp_l = pDB->NextId("TB_DOM_USER", "Id");
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
				sprintf(query, "INSERT INTO TB_DOM_USER %s VALUES %s", query_into, query_values);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_user_add]");
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
						sprintf(query, "DELETE FROM TB_DOM_USER WHERE Nombre = \'%s\'", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_user_delete]");
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
					sprintf(query, "UPDATE TB_DOM_USER SET %s WHERE %s", query_values, query_where);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_user_update]");
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
					"Pin_Teclado,Pin_SMS,Pin_WEB FROM TB_DOM_USER WHERE Nombre = \'%s\'", json_user->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_user_check]");
				}
			}
			/* ****************************************************************
			*		dompi_hw_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_hw_list"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT Id, Dispositivo, Tipo, Estado FROM TB_DOM_PERIF");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_hw_list]");
				}
			}
			/* ****************************************************************
			*		dompi_hw_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_hw_list_all"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_PERIF");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_hw_list_all]");
				}
			}
			/* ****************************************************************
			*		dompi_hw_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_hw_get"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_PERIF WHERE Id = \'%s\'", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc == 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_hw_get]");
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

				/* Obtengo un ID para el elemento nuevo y lo cambio en el dato recibido */
				temp_l = pDB->NextId("TB_DOM_PERIF", "Id");
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
				sprintf(query, "INSERT INTO TB_DOM_PERIF %s VALUES %s", query_into, query_values);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_hw_add]");
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
						sprintf(query, "DELETE FROM TB_DOM_PERIF WHERE Id = \'%s\'", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_hw_delete]");
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
											strcpy(update_hw_config_id, hw_id);
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
					sprintf(query, "UPDATE TB_DOM_PERIF SET %s WHERE %s", query_values, query_where);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_hw_update]");
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
								"WHERE ASS.Dispositivo = HW.Id");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_query_result, query);
				if(rc == 0)
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

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ass_list]");
				}
			}
			/* ****************************************************************
			*		dompi_ass_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_list_all"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_ASSIGN");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ass_list_all]");
				}
			}
			/* ****************************************************************
			*		dompi_ass_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_get"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_ASSIGN WHERE Id = \'%s\'", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc == 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ass_get]");
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
				sprintf(query, "INSERT INTO TB_DOM_ASSIGN %s VALUES %s", query_into, query_values);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ass_add]");
				}
				run_dbmant = true;
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
						sprintf(query, "DELETE FROM TB_DOM_ASSIGN WHERE Id = \'%s\'", json_un_obj->valuestring);
						m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
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

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ass_delete]");
				}
				run_dbmant = true;
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
					sprintf(query, "UPDATE TB_DOM_ASSIGN SET %s WHERE %s", query_values, query_where);
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
											"WHERE Id = '%s'",
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
											"WHERE Id = '%s'",
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ass_update]");
				}
				run_dbmant = true;
			}
			/* ****************************************************************
			*		dompi_ass_status
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_status"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_arr = cJSON_CreateArray();
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					sprintf(query, "SELECT Id,Objeto,Tipo,Icono0,Icono1,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Estado FROM TB_DOM_ASSIGN WHERE Id = \'%s\'", json_un_obj->valuestring);
				}
				else
				{
					json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Planta");
					if(json_un_obj)
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Icono0,Icono1,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Estado FROM TB_DOM_ASSIGN WHERE Planta = %s", json_un_obj->valuestring);
					}
					else
					{
						strcpy(query, "SELECT Id,Objeto,Tipo,Icono0,Icono1,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Estado FROM TB_DOM_ASSIGN");
					}
				}
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					if(json_obj) cJSON_Delete(json_obj);
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
				}
				if(json_obj) cJSON_Delete(json_obj);
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ass_status]");
				}
			}
			/* ****************************************************************
			*		dompi_ass_info
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_info"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_arr = cJSON_CreateArray();
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					sprintf(query, "SELECT Id,Objeto,Tipo,Icono0,Icono1,Grupo_Visual,Planta,Cord_x,Cord_y FROM TB_DOM_ASSIGN WHERE Id = \'%s\'", json_un_obj->valuestring);
				}
				else
				{
					json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Planta");
					if(json_un_obj)
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Icono0,Icono1,Grupo_Visual,Planta,Cord_x,Cord_y FROM TB_DOM_ASSIGN WHERE Planta = %s", json_un_obj->valuestring);
					}
					else
					{
						strcpy(query, "SELECT Id,Objeto,Tipo,Icono0,Icono1,Grupo_Visual,Planta,Cord_x,Cord_y FROM TB_DOM_ASSIGN");
					}
				}
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					if(json_obj) cJSON_Delete(json_obj);
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
				}
				if(json_obj) cJSON_Delete(json_obj);
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ass_info]");
				}
			}
			/* ****************************************************************
			*		dompi_ass_on
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_on"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Objeto");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port, ASS.E_S "
									"FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
									"WHERE HW.Id = ASS.Dispositivo AND "
									"ASS.Objeto =  \'%s\'", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc == 0)
					{
						/* Actualizo el estado en la base */
						sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
										"SET Estado = 1 "
										"WHERE Objeto = \'%s\'", json_un_obj->valuestring);
						m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
						pDB->Query(NULL, query);
						/* Creo un objeto con el primer item del array */
						json_un_obj = json_arr->child;
						cJSON_AddStringToObject(json_un_obj, "Estado", "1");
						cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
						m_pServer->m_pLog->Add(50, "[dompi_hw_set_io][%s]", message);
						rc = m_pServer->Call("dompi_hw_set_io", message, strlen(message), &call_resp, internal_timeout);
						if(rc == 0)
						{
							strcpy(message, (const char*)call_resp.data);
						}
						else
						{
							sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
						}
						m_pServer->Free(call_resp);
					}
					strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid Object\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ass_on]");
				}
			}
			/* ****************************************************************
			*		dompi_ass_off
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_off"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Objeto");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port, ASS.E_S "
									"FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
									"WHERE HW.Id = ASS.Dispositivo AND "
									"ASS.Objeto =  \'%s\'", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc == 0)
					{
						/* Actualizo el estado en la base */
						sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
										"SET Estado = 0 "
										"WHERE Objeto = \'%s\'", json_un_obj->valuestring);
						m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
						pDB->Query(NULL, query);
						/* Creo un objeto con el primer item del array */
						json_un_obj = json_arr->child;
						cJSON_AddStringToObject(json_un_obj, "Estado", "0");
						cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
						m_pServer->m_pLog->Add(50, "[dompi_hw_set_io][%s]", message);
						rc = m_pServer->Call("dompi_hw_set_io", message, strlen(message), &call_resp, internal_timeout);
						if(rc == 0)
						{
							strcpy(message, (const char*)call_resp.data);
						}
						else
						{
							sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
						}
						m_pServer->Free(call_resp);
					}
					strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid Object\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ass_off]");
				}
			}
			/* ****************************************************************
			*		dompi_ass_switch
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_switch"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Objeto");
				if(json_un_obj)
				{
					/* Actualizo el estado en la base */
					sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
									"SET Estado = (1 - Estado) "
									"WHERE Objeto = \'%s\'", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					if(rc > 0)
					{
						json_arr = cJSON_CreateArray();
						sprintf(query, "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port, ASS.E_S, ASS.Estado "
										"FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
										"WHERE HW.Id = ASS.Dispositivo AND "
										"ASS.Objeto =  \'%s\'", json_un_obj->valuestring);
						m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
						rc = pDB->Query(json_arr, query);
						if(rc == 0)
						{
							/* Creo un objeto con el primer item del array */
							json_un_obj = json_arr->child;
							cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
							m_pServer->m_pLog->Add(50, "[dompi_hw_set_io][%s]", message);
							rc = m_pServer->Call("dompi_hw_set_io", message, strlen(message), &call_resp, internal_timeout);
							if(rc == 0)
							{
								strcpy(message, (const char*)call_resp.data);
							}
							else
							{
								sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
							}
							m_pServer->Free(call_resp);
						}
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid Object\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid Object\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ass_switch]");
				}
			}
			/* ****************************************************************
			*		dompi_ass_pulse
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_pulse"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Objeto");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port, ASS.E_S "
									"FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
									"WHERE HW.Id = ASS.Dispositivo AND "
									"ASS.Objeto =  \'%s\'", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc == 0)
					{
						/* Actualizo el estado en la base */
						sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
										"SET Estado = 1 "
										"WHERE Objeto = \'%s\'", json_un_obj->valuestring);
						m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
						pDB->Query(NULL, query);
						/* Creo un objeto con el primer item del array */
						json_un_obj = json_arr->child;
						/* TODO: Implementar parametro de tiempo */
						cJSON_AddStringToObject(json_un_obj, "Segundos", "1");
						cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
						m_pServer->m_pLog->Add(50, "[dompi_hw_pulse_io][%s]", message);
						rc = m_pServer->Call("dompi_hw_pulse_io", message, strlen(message), &call_resp, internal_timeout);
						if(rc == 0)
						{
							strcpy(message, (const char*)call_resp.data);
						}
						else
						{
							sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
						}
						m_pServer->Free(call_resp);
					}
					strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Invalid Object\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ass_pulse]");
				}
			}
			/* ****************************************************************
			*		dompi_ass_add_to_planta
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_add_to_planta"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					sprintf(query, "UPDATE TB_DOM_ASSIGN "
									"SET Icono0 = \"lamp0.png\",Icono1 = \"lamp1.png\",Planta = 1,Cord_x = 100,Cord_y = 100 "
									"WHERE Id = %s", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
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

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ass_add_to_planta]");
				}
			}
			/* ****************************************************************
			*		dompi_ass_cmd
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_cmd"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				json_Objeto = cJSON_GetObjectItemCaseSensitive(json_obj, "Objeto");
				json_Accion = cJSON_GetObjectItemCaseSensitive(json_obj, "Accion");

				json_arr = cJSON_CreateArray();
				sprintf(query, "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port, ASS.E_S "
								"FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
								"WHERE HW.Id = ASS.Dispositivo AND "
								"ASS.Objeto =  \'%s\'", json_Objeto->valuestring);
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					/* Creo un objeto con el primer item del array */
					json_un_obj = json_arr->child;
					if( !strcmp(json_Accion->valuestring, "on"))
					{
						cJSON_AddStringToObject(json_un_obj, "Estado", "1");
					}
					else if( !strcmp(json_Accion->valuestring, "off"))
					{
						cJSON_AddStringToObject(json_un_obj, "Estado", "0");
					}
					cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
					if( !strcmp(json_Accion->valuestring, "switch"))
					{
						m_pServer->m_pLog->Add(50, "[dompi_hw_set_io][%s]", message);
						rc = m_pServer->Call("dompi_hw_set_io", message, strlen(message), &call_resp, internal_timeout);
					}
					else
					{
						m_pServer->m_pLog->Add(50, "[dompi_hw_switch_io][%s]", message);
						rc = m_pServer->Call("dompi_hw_switch_io", message, strlen(message), &call_resp, internal_timeout);
					}
					if(rc == 0)
					{
						strcpy(message, (const char*)call_resp.data);
					}
					else
					{
						sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
					}
					m_pServer->Free(call_resp);
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
							   "WHERE EV.Objeto_Origen = ASS.Id");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ev_list]");
				}
			}
			/* ****************************************************************
			*		dompi_ev_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ev_list_all"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_EVENT");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ev_list_all]");
				}
			}
			/* ****************************************************************
			*		dompi_ev_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ev_get"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_EVENT WHERE Id = \'%s\'", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc == 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ev_get]");
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
				sprintf(query, "INSERT INTO TB_DOM_EVENT %s VALUES %s", query_into, query_values);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ev_add]");
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
						sprintf(query, "DELETE FROM TB_DOM_EVENT WHERE Id = \'%s\'", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ev_delete]");
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
					sprintf(query, "UPDATE TB_DOM_EVENT SET %s WHERE %s", query_values, query_where);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_ev_update]");
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
						else if( !strcmp(comando, "manten") )
						{
							DBMant( message );
						}
						else if( !strcmp(comando, "sms") )
						{
							json_un_obj = cJSON_CreateObject();
							cJSON_AddStringToObject(json_un_obj, "SmsTo", (objeto)?objeto:"98765432");
							cJSON_AddStringToObject(json_un_obj, "SmsTxt", (parametro)?parametro:"test");
							cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
							m_pServer->m_pLog->Add(50, "[dompi_send_sms][%s]", message);
							rc = m_pServer->Call("dompi_send_sms", message, strlen(message), &call_resp, internal_timeout);
							if(rc == 0)
							{
								strcpy(message, (const char*)call_resp.data);
							}
							else
							{
								sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
							}
							m_pServer->Free(call_resp);
						}
						else if( !strcmp(comando, "encender") )
						{
							json_arr = cJSON_CreateArray();
							sprintf(query, "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port, ASS.E_S "
											"FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
											"WHERE HW.Id = ASS.Dispositivo AND "
											"ASS.Objeto =  \'%s\'", objeto);
							m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
							rc = pDB->Query(json_arr, query);
							if(rc == 0)
							{
								/* Creo un objeto con el primer item del array */
								json_un_obj = json_arr->child;
								cJSON_AddStringToObject(json_un_obj, "Estado", "1");
								cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
								m_pServer->m_pLog->Add(50, "[dompi_hw_set_io][%s]", message);
								rc = m_pServer->Call("dompi_hw_set_io", message, strlen(message), &call_resp, internal_timeout);
								if(rc == 0)
								{
									strcpy(message, (const char*)call_resp.data);
								}
								else
								{
									sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
								}
								m_pServer->Free(call_resp);
							}
						}
						else if( !strcmp(comando, "apagar") )
						{
							json_arr = cJSON_CreateArray();
							sprintf(query, "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port, ASS.E_S "
											"FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
											"WHERE HW.Id = ASS.Dispositivo AND "
											"ASS.Objeto =  \'%s\'", objeto);
							m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
							rc = pDB->Query(json_arr, query);
							if(rc == 0)
							{
								/* Creo un objeto con el primer item del array */
								json_un_obj = json_arr->child;
								cJSON_AddStringToObject(json_un_obj, "Estado", "0");
								cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
								m_pServer->m_pLog->Add(50, "[dompi_hw_set_io][%s]", message);
								rc = m_pServer->Call("dompi_hw_set_io", message, strlen(message), &call_resp, internal_timeout);
								if(rc == 0)
								{
									strcpy(message, (const char*)call_resp.data);
								}
								else
								{
									sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
								}
								m_pServer->Free(call_resp);
							}
						}
						else if( !strcmp(comando, "cambiar") )
						{
							json_arr = cJSON_CreateArray();
							sprintf(query, "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port, ASS.E_S "
											"FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
											"WHERE HW.Id = ASS.Dispositivo AND "
											"ASS.Objeto =  \'%s\'", objeto);
							m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
							rc = pDB->Query(json_arr, query);
							if(rc == 0)
							{
								/* Creo un objeto con el primer item del array */
								json_un_obj = json_arr->child;
								cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
								m_pServer->m_pLog->Add(50, "[dompi_hw_switch_io][%s]", message);
								rc = m_pServer->Call("dompi_hw_switch_io", message, strlen(message), &call_resp, internal_timeout);
								if(rc == 0)
								{
									strcpy(message, (const char*)call_resp.data);
								}
								else
								{
									sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
								}
								m_pServer->Free(call_resp);
							}
						}
						else if( !strcmp(comando, "pulso") )
						{
							json_arr = cJSON_CreateArray();
							sprintf(query, "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port, ASS.E_S "
											"FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
											"WHERE HW.Id = ASS.Dispositivo AND "
											"ASS.Objeto =  \'%s\'", objeto);
							m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
							rc = pDB->Query(json_arr, query);
							if(rc == 0)
							{
								/* Creo un objeto con el primer item del array */
								json_un_obj = json_arr->child;
								cJSON_AddStringToObject(json_un_obj, "Segundos", (parametro)?parametro:"1");
								cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
								m_pServer->m_pLog->Add(50, "[dompi_hw_pulse_io][%s]", message);
								rc = m_pServer->Call("dompi_hw_pulse_io", message, strlen(message), &call_resp, internal_timeout);
								if(rc == 0)
								{
									strcpy(message, (const char*)call_resp.data);
								}
								else
								{
									sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
								}
								m_pServer->Free(call_resp);
							}
						}
						else if( !strcmp(comando, "estado") )
						{
							json_arr = cJSON_CreateArray();
							sprintf(query, "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port, ASS.E_S "
											"FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
											"WHERE HW.Id = ASS.Dispositivo AND "
											"ASS.Objeto =  \'%s\'", objeto);
							m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
							rc = pDB->Query(json_arr, query);
							if(rc == 0)
							{
								/* Creo un objeto con el primer item del array */
								json_un_obj = json_arr->child;
								cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
								m_pServer->m_pLog->Add(50, "[dompi_hw_get_io][%s]", message);
								rc = m_pServer->Call("dompi_hw_get_io", message, strlen(message), &call_resp, internal_timeout);
								if(rc == 0)
								{
									strcpy(message, (const char*)call_resp.data);
								}
								else
								{
									sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
								}
								m_pServer->Free(call_resp);
							}
						}
						else if( !strcmp(comando, "actualizar") )
						{
							if( !memcmp(parametro, "conf", 4))
							{
								/* PORT A */
								json_arr = cJSON_CreateArray();
								sprintf(query, "SELECT Direccion_IP, Tipo AS Tipo_HW, "
												"Port = 1, Config_PORT_A_Analog AS AN_Config, Config_PORT_A_E_S AS IO_Config "
												"FROM TB_DOM_PERIF "
												"WHERE Dispositivo =  \'%s\'", objeto);
								m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
								rc = pDB->Query(json_arr, query);
								if(rc == 0)
								{
									/* Creo un objeto con el primer item del array */
									json_un_obj = json_arr->child;
									cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
									m_pServer->m_pLog->Add(50, "[dompi_hw_set_port_config][%s]", message);
									rc = m_pServer->Call("dompi_hw_set_port_config", message, strlen(message), &call_resp, internal_timeout);
									if(rc == 0)
									{
										strcpy(message, (const char*)call_resp.data);
									}
									else
									{
										sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
									}
									m_pServer->Free(call_resp);
								}
								cJSON_Delete(json_arr);
								/* Flags */
								json_arr = cJSON_CreateArray();
								sprintf(query, "SELECT Direccion_IP, Tipo AS Tipo_HW, "
												"Port = 0, Flags AS IO_Config "
												"FROM TB_DOM_PERIF "
												"WHERE Dispositivo =  \'%s\'", objeto);
								m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
								rc = pDB->Query(json_arr, query);
								if(rc == 0)
								{
									/* Creo un objeto con el primer item del array */
									json_un_obj = json_arr->child;
									cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
									m_pServer->m_pLog->Add(50, "[dompi_hw_set_port_config][%s]", message);
									rc = m_pServer->Call("dompi_hw_set_port_config", message, strlen(message), &call_resp, internal_timeout);
									if(rc == 0)
									{
										strcpy(message, (const char*)call_resp.data);
									}
									else
									{
										sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
									}
									m_pServer->Free(call_resp);
								}
								cJSON_Delete(json_arr);
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
												"WHERE Dispositivo =  \'%s\'", objeto);
								m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
								rc = pDB->Query(json_arr, query);
								if(rc == 0)
								{
									/* Creo un objeto con el primer item del array */
									json_un_obj = json_arr->child;
									cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
									m_pServer->m_pLog->Add(50, "[dompi_hw_set_port][%s]", message);
									rc = m_pServer->Call("dompi_hw_set_port", message, strlen(message), &call_resp, internal_timeout);
									if(rc == 0)
									{
										strcpy(message, (const char*)call_resp.data);
									}
									else
									{
										sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
									}
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
												"WHERE Dispositivo =  \'%s\'", objeto);
								m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
								rc = pDB->Query(json_arr, query);
								if(rc == 0)
								{
									/* Creo un objeto con el primer item del array */
									json_un_obj = json_arr->child;
									cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
									m_pServer->m_pLog->Add(50, "[dompi_hw_get_port_config][%s]", message);
									rc = m_pServer->Call("dompi_hw_get_port_config", message, strlen(message), &call_resp, internal_timeout);
									if(rc == 0)
									{
										strcpy(message, (const char*)call_resp.data);
									}
									else
									{
										sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
									}
									m_pServer->Free(call_resp);
								}
							}
							else if( !strcmp(parametro, "wifi"))
							{
								json_arr = cJSON_CreateArray();
								sprintf(query, "SELECT Direccion_IP, Tipo AS Tipo_HW "
												"FROM TB_DOM_PERIF "
												"WHERE Dispositivo =  \'%s\'", objeto);
								m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
								rc = pDB->Query(json_arr, query);
								if(rc == 0)
								{
									/* Creo un objeto con el primer item del array */
									json_un_obj = json_arr->child;
									cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
									m_pServer->m_pLog->Add(50, "[dompi_hw_get_comm_config][%s]", message);
									rc = m_pServer->Call("dompi_hw_get_comm_config", message, strlen(message), &call_resp, internal_timeout);
									if(rc == 0)
									{
										strcpy(message, (const char*)call_resp.data);
									}
									else
									{
										sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
									}
									m_pServer->Free(call_resp);
								}
							}
							else if( !strcmp(parametro, "estado"))
							{
								json_arr = cJSON_CreateArray();
								sprintf(query, "SELECT Direccion_IP, Tipo AS Tipo_HW "
												"FROM TB_DOM_PERIF "
												"WHERE Dispositivo =  \'%s\'", objeto);
								m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
								rc = pDB->Query(json_arr, query);
								if(rc == 0)
								{
									/* Creo un objeto con el primer item del array */
									json_un_obj = json_arr->child;
									cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
									m_pServer->m_pLog->Add(50, "[dompi_hw_get_port][%s]", message);
									rc = m_pServer->Call("dompi_hw_get_port", message, strlen(message), &call_resp, internal_timeout);
									if(rc == 0)
									{
										strcpy(message, (const char*)call_resp.data);
									}
									else
									{
										sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error\"}}", rc);
									}
									m_pServer->Free(call_resp);
								}
							}
						}
					}
				}
				/* *********************************************************** */
				cJSON_Delete(json_obj);
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_cmdline]");
				}
			}


			/* ****************************************************************
			*		dompi_sysconf_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_sysconf_list"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOM_CONFIG");
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_sysconf_list]");
				}
			}
			/* ****************************************************************
			*		dompi_sysconf_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_sysconf_get"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_CONFIG WHERE Id = \'%s\'", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc == 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_sysconf_get]");
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
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_sysconf_get_current]");
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
// cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)
				json_un_obj = json_obj;

				/* Obtengo un ID para el elemento nuevo y lo cambio en el dato recibido */
				temp_l = pDB->NextId("TB_DOM_CONFIG", "Id");
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
				sprintf(query, "INSERT INTO TB_DOM_CONFIG %s VALUES %s", query_into, query_values);
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);

				load_system_config = 1;

				rc = pDB->Query(NULL, query);
				if(rc != 0)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_sysconf_add]");
				}
			}
			/* ****************************************************************
			*		dompi_cloud_notification
			**************************************************************** */
			else if( !strcmp(fn, "dompi_cloud_notification"))
			{
				/* Un array de acciones sobre objetos */
				json_arr = cJSON_Parse(message);
				if(cJSON_IsArray(json_arr))
				{
					cJSON_ArrayForEach(json_un_obj, json_arr)
					{
						/* Llamo dompi_ass_cmd */
						cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
						m_pServer->m_pLog->Add(50, "[dompi_ass_cmd][%s]", message);
						m_pServer->Post("dompi_ass_cmd", message, strlen(message));
					}
				}
				cJSON_Delete(json_arr);
			}



			else
			{
				m_pServer->m_pLog->Add(10, "[%s][R][GME_SVC_NOTFOUND]", fn);
				m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
			}

		}
		/*
		* Tareas asincrónicas
		*
		*
		*/

		if(run_dbmant)
		{
			run_dbmant = false;
			DBMant(NULL);
		}

		m_pServer->m_pLog->Add(100, "Control de tareas programadas");
		/* Control de tareas pendientes */
		json_arr = cJSON_CreateArray();
		sprintf(query, "SELECT Id, MAC, Direccion_IP, Tipo, "
						"Config_PORT_A_Analog, Config_PORT_A_E_S, "
						"Config_PORT_B_Analog, Config_PORT_B_E_S, "
						"Config_PORT_C_Analog, Config_PORT_C_E_S, "
						"Flags "
						"FROM TB_DOM_PERIF "
						"WHERE Actualizar <> 0");
		m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
		rc = pDB->Query(json_arr, query);
		if(rc == 0)
		{
			/* Recorro el array */
			cJSON_ArrayForEach(json_un_obj, json_arr)
			{
				/* Saco los datos que necesito */
				json_HW_Id = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Id");
				json_MAC = cJSON_GetObjectItemCaseSensitive(json_un_obj, "MAC");
				json_Direccion_IP = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Direccion_IP");
				json_Tipo_HW = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Tipo");
				json_Config_PORT_A_E_S = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Config_PORT_A_E_S");
				json_Config_PORT_B_E_S = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Config_PORT_B_E_S");
				json_Flags = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Flags");
				//json_Config_PORT_C_E_S = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Config_PORT_C_E_S");
				json_Config_PORT_A_Analog = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Config_PORT_A_Analog");
				//json_Config_PORT_B_Analog = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Config_PORT_B_Analog");
				//json_Config_PORT_C_Analog = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Config_PORT_C_Analog");
				m_pServer->m_pLog->Add(100, "Actualizar [%s]", json_MAC->valuestring);
				if(atoi(json_Tipo_HW->valuestring) == 0) /* RBPi Local */
				{
					/* Armo los mensajes para cada port */
					/*PORT A*/
					json_obj = cJSON_CreateObject();
					cJSON_AddStringToObject(json_obj, json_Direccion_IP->string, json_Direccion_IP->valuestring);
					cJSON_AddStringToObject(json_obj, "Tipo_HW", json_Tipo_HW->valuestring);
					cJSON_AddStringToObject(json_obj, "IO_Config", json_Config_PORT_A_E_S->valuestring);
					if(json_Config_PORT_A_Analog)
					{
						cJSON_AddStringToObject(json_obj, "AN_Config", json_Config_PORT_A_Analog->valuestring);
					}
					cJSON_AddStringToObject(json_obj, "Port", "1");
					/* Envio la configuracion de cada port por separado */
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
					m_pServer->m_pLog->Add(50, "[dompi_pi_set_port_config][%s]", message);
					m_pServer->Call("dompi_pi_set_port_config", message, strlen(message), NULL, internal_timeout);
					/*  */
					/*PORT B*/
					json_obj = cJSON_CreateObject();
					cJSON_AddStringToObject(json_obj, json_Direccion_IP->string, json_Direccion_IP->valuestring);
					cJSON_AddStringToObject(json_obj, "Tipo_HW", json_Tipo_HW->valuestring);
					cJSON_AddStringToObject(json_obj, "IO_Config", json_Config_PORT_B_E_S->valuestring);
					cJSON_AddStringToObject(json_obj, "Port", "2");
					/* Envio la configuracion de cada port por separado */
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
					m_pServer->m_pLog->Add(50, "[dompi_pi_set_port_config][%s]", message);
					m_pServer->Call("dompi_pi_set_port_config", message, strlen(message), NULL, internal_timeout);
					/*  */
				}
				else if(atoi(json_Tipo_HW->valuestring) == 1) /* Dom32IOWiFi */
				{
					/* Armo los mensajes para cada port */
					/*PORT A*/
					json_obj = cJSON_CreateObject();
					cJSON_AddStringToObject(json_obj, json_Direccion_IP->string, json_Direccion_IP->valuestring);
					cJSON_AddStringToObject(json_obj, "Tipo_HW", json_Tipo_HW->valuestring);
					cJSON_AddStringToObject(json_obj, "IO_Config", json_Config_PORT_A_E_S->valuestring);
					if(json_Config_PORT_A_Analog)
					{
						cJSON_AddStringToObject(json_obj, "AN_Config", json_Config_PORT_A_Analog->valuestring);
					}
					cJSON_AddStringToObject(json_obj, "Port", "1");
					/* Envio la configuracion de cada port por separado */
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
					m_pServer->m_pLog->Add(50, "[dompi_hw_set_port_config][%s]", message);
					m_pServer->Call("dompi_hw_set_port_config", message, strlen(message), NULL, internal_timeout);
					/*  */
					/*Flags*/
					json_obj = cJSON_CreateObject();
					cJSON_AddStringToObject(json_obj, json_Direccion_IP->string, json_Direccion_IP->valuestring);
					cJSON_AddStringToObject(json_obj, "Tipo_HW", json_Tipo_HW->valuestring);
					cJSON_AddStringToObject(json_obj, "IO_Config", json_Flags->valuestring);
					cJSON_AddStringToObject(json_obj, "Port", "0");
					/* Envio la configuracion de cada port por separado */
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
					m_pServer->m_pLog->Add(50, "[dompi_hw_set_port_config][%s]", message);
					m_pServer->Call("dompi_hw_set_port_config", message, strlen(message), NULL, internal_timeout);
					/*  */
				}
				/* Borro la marca */
				sprintf(query, "UPDATE TB_DOM_PERIF "
								"SET Actualizar = 0 "
								"WHERE Id = \'%s\'", json_HW_Id->valuestring);
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				pDB->Query(NULL, query);
			}
		}
		cJSON_Delete(json_arr);
		/*  */
		if(update_hw_config_mac[0])
		{
			sprintf(query, "UPDATE TB_DOM_PERIF "
							"SET Actualizar = 1 "
							"WHERE MAC = \'%s\'", update_hw_config_mac);
			m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
			pDB->Query(NULL, query);
			update_hw_config_mac[0] = 0;
		}
		/*  */
		if(update_hw_config_id[0])
		{
			sprintf(query, "UPDATE TB_DOM_PERIF "
							"SET Actualizar = 1 "
							"WHERE Id = \'%s\'", update_hw_config_id);
			m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
			pDB->Query(NULL, query);
			update_hw_config_id[0] = 0;
		}

		if(load_system_config)
		{
			LoadSystemConfig();
		}

		if(update_system_config && json_System_Config)
		{
			cJSON_PrintPreallocated(json_System_Config->child, message, MAX_BUFFER_LEN, 0);
			m_pServer->m_pLog->Add(50, "[dompi_cloud_config][%s]", message);
			rc = m_pServer->Call("dompi_cloud_config", message, strlen(message), &call_resp, internal_timeout);
			if(rc == 0)
			{
				update_system_config = 0;
				update_ass_t = 0;
			}
			m_pServer->Free(call_resp);
		}
		/* Tomo la hora para los cálculos de abajo */
		t = time(&t);
		/* Actualizacion de objetos en la nube */
		if(t >= update_ass_t)
		{
			update_ass_t = t + 600;
			/* Genero un listado de los objetos con su estado para subir a la nube */
			json_query_result = cJSON_CreateArray();
			strcpy(query, "SELECT * FROM TB_DOM_ASSIGN WHERE Id > 0");
			m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
			rc = pDB->Query(json_query_result, query);
			if(rc == 0)
			{
				json_obj = cJSON_CreateObject();
				cJSON_AddItemToObject(json_obj, "Objetos", json_query_result);
				cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
				cJSON_Delete(json_obj);
				m_pServer->Post("dompi_ass_status_update", message, strlen(message));
			}
			else
			{
				cJSON_Delete(json_query_result);
			}






		}
		/* Controles del modulo de alarma */





		
		/* Tareas programadas en TB_DOM_AT */


		/* Recalculo del timer */
		delta_t = (next_t > t)?(next_t-t)*100:(1);
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
	m_pServer->UnSuscribe("dompi_ass_status", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_info", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_on", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_off", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_switch", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_pulse", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_add_to_planta", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_cmd", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_ev_list", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_list_all", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_get", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_add", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_delete", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ev_update", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_cmdline", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_sysconf_list", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_sysconf_get", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_sysconf_get_current", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_sysconf_add", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_cloud_notification", GM_MSG_TYPE_MSG);

	delete pEV;
	delete pConfig;
	delete pDB;
	delete m_pServer;
	exit(0);
}

/*
	Recorre los assign para setear la configuración correcta de E/S en cada dispositivo
*/
void DBMant( char* msg )
{
	int rc;
	char query[4096];
    cJSON *json_obj_hw;
    cJSON *json_arr_hw;
    cJSON *json_obj_ass;
    cJSON *json_arr_ass;

    cJSON *json_HW_Id;
    cJSON *json_MAC;
	cJSON *json_Tipo_HW;

	cJSON *json_Objeto;
	cJSON *json_Port;
	cJSON *json_E_S;
	cJSON *json_Tipo_ASS;

	int i_PORT_A_Analog;
	int i_PORT_A_E_S;
	int i_PORT_B_Analog;
	int i_PORT_B_E_S;
	int i_PORT_C_Analog;
	int i_PORT_C_E_S;

	if(msg) strcpy(msg, "Mantenimiento de la base de datos...\r\n");
	m_pServer->m_pLog->Add(50, "Iniciando mantenimiento de la base de datos");

	json_arr_hw = cJSON_CreateArray();
	sprintf(query, "SELECT Id, MAC, Tipo "
					"FROM TB_DOM_PERIF "
					"WHERE Id > 0 ");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_arr_hw, query);
	if(rc == 0)
	{
		/* Recorro el array */
		cJSON_ArrayForEach(json_obj_hw, json_arr_hw)
		{
			/* Saco los datos que necesito */
			json_HW_Id = cJSON_GetObjectItemCaseSensitive(json_obj_hw, "Id");
			json_MAC = cJSON_GetObjectItemCaseSensitive(json_obj_hw, "MAC");
			json_Tipo_HW = cJSON_GetObjectItemCaseSensitive(json_obj_hw, "Tipo");
			if(msg)
			{
				strcat(msg, "    Procesando ");
				strcat(msg, json_MAC->valuestring);
				strcat(msg, "\r\n");
			}
			m_pServer->m_pLog->Add(100, "Procesando [%s]...", json_MAC->valuestring);

			if(atoi(json_Tipo_HW->valuestring) == 1)
			{
				json_arr_ass = cJSON_CreateArray();
				sprintf(query, "SELECT Id, Objeto, Port, E_S, Tipo "
								"FROM TB_DOM_ASSIGN "
								"WHERE Dispositivo = %s ", json_HW_Id->valuestring);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr_ass, query);
				if(rc == 0)
				{
					i_PORT_A_Analog = 0;
					i_PORT_A_E_S = 0xFF;
					i_PORT_B_Analog = 0;
					i_PORT_B_E_S = 0x00;
					i_PORT_C_Analog = 0;
					i_PORT_C_E_S = 0xFF;
					/* Recorro el array */
					cJSON_ArrayForEach(json_obj_ass, json_arr_ass)
					{
						json_Objeto = cJSON_GetObjectItemCaseSensitive(json_obj_ass, "Objeto");
						json_Port = cJSON_GetObjectItemCaseSensitive(json_obj_ass, "Port");
						json_E_S = cJSON_GetObjectItemCaseSensitive(json_obj_ass, "E_S");
						json_Tipo_ASS = cJSON_GetObjectItemCaseSensitive(json_obj_ass, "Tipo");

						m_pServer->m_pLog->Add(100, "    Objeto [%s]...", json_Objeto->valuestring);

						if(atoi(json_Port->valuestring) == 1)
						{
							/* PORT A */
							/*	0=Output, 
								1=Input, 
								2=Analog, 
								3=Output Alarma, 
								4=Input Alarma, 
								5=Output Pulse/Analog_Mult_Div_Valor=Pulse Param
							*/
							if(	atoi(json_Tipo_ASS->valuestring) == 1 ||
								atoi(json_Tipo_ASS->valuestring) == 2 ||
								atoi(json_Tipo_ASS->valuestring) == 4  )
							{
								/* Entrada */
								i_PORT_A_E_S |= power2(atoi(json_E_S->valuestring)-1);  
								if(atoi(json_Tipo_ASS->valuestring) == 2)
								{
									i_PORT_A_Analog |= power2(atoi(json_E_S->valuestring)-1);  
								}
							}
							else
							{
								/* Salida */
								i_PORT_A_E_S &= (power2(atoi(json_E_S->valuestring)-1)^0xFF);  
								i_PORT_A_Analog &= (power2(atoi(json_E_S->valuestring)-1)^0xFF);  
							}
						}
						else if(atoi(json_Port->valuestring) == 2)
						{
							/* PORT B */
							if(	atoi(json_Tipo_ASS->valuestring) == 1 ||
								atoi(json_Tipo_ASS->valuestring) == 2 ||
								atoi(json_Tipo_ASS->valuestring) == 4  )
							{
								/* Entrada */
								i_PORT_B_E_S |= power2(atoi(json_E_S->valuestring)-1);  
								if(atoi(json_Tipo_ASS->valuestring) == 2)
								{
									i_PORT_B_Analog |= power2(atoi(json_E_S->valuestring)-1);  
								}
							}
							else
							{
								/* Salida */
								i_PORT_B_E_S &= (power2(atoi(json_E_S->valuestring)-1)^0xFF);  
								i_PORT_B_Analog &= (power2(atoi(json_E_S->valuestring)-1)^0xFF);  
							}
						}
						else if(atoi(json_Port->valuestring) == 2)
						{
							/* PORT C */
							if(	atoi(json_Tipo_ASS->valuestring) == 1 ||
								atoi(json_Tipo_ASS->valuestring) == 2 ||
								atoi(json_Tipo_ASS->valuestring) == 4  )
							{
								/* Entrada */
								i_PORT_C_E_S |= power2(atoi(json_E_S->valuestring)-1);  
								if(atoi(json_Tipo_ASS->valuestring) == 2)
								{
									i_PORT_C_Analog |= power2(atoi(json_E_S->valuestring)-1);  
								}
							}
							else
							{
								/* Salida */
								i_PORT_C_E_S &= (power2(atoi(json_E_S->valuestring)-1)^0xFF);  
								i_PORT_C_Analog &= (power2(atoi(json_E_S->valuestring)-1)^0xFF);  
							}
						}
					}
					sprintf(query, "UPDATE TB_DOM_PERIF "
									"Config_PORT_A_Analog = %i, "
									"Config_PORT_A_E_S = %i, "
									"Config_PORT_B_Analog = %i, "
									"Config_PORT_B_E_S = %i, "
									"Config_PORT_C_Analog = %i, "
									"Config_PORT_C_E_S = %i, "
									"Actualizar = 1 "
									"WHERE Id = \'%s\'",
									i_PORT_A_Analog,
									i_PORT_A_E_S,
									i_PORT_B_Analog,
									i_PORT_B_E_S,
									i_PORT_C_Analog,
									i_PORT_C_E_S,
									json_HW_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					pDB->Query(NULL, query);
				}
				cJSON_Delete(json_arr_ass);
			}
			/* Levanto el flag para que mande configuracion a la placa */
			sprintf(query, "UPDATE TB_DOM_PERIF "
							"SET Actualizar = 1 "
							"WHERE Id = \'%s\'", json_HW_Id->valuestring);
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			pDB->Query(NULL, query);
		}
	}
	m_pServer->m_pLog->Add(50, "Finalizando mantenimiento de la base de datos");
	cJSON_Delete(json_arr_hw);
}
