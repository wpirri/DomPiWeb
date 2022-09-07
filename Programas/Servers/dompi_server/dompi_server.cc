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

const char *system_columns[] = {
	"Id",
	"Creacion",
	"System_Key",
	"Cloud_Host_1_Address",
	"Cloud_Host_1_Port",
	"Cloud_Host_1_Proto",
	"Cloud_Host_2_Address",
	"Cloud_Host_2_Port",
	"Cloud_Host_2_Proto",
	"Planta1",
	"Planta2",
	"Planta3",
	"Planta4",
	"Planta5",
	"Modem_port",
	"Flags",
	0};

const char *user_columns[] = {
	"Id integer primary key",
	"Usuario",
	"Nombre_Completo",
	"Pin_Teclado",
	"Pin_WEB",
	"Telefono_Voz",
	"Telefono_SMS",
	"eMail",
	"Permisos",
	"Dias",
	"Horas",
	"Estado",
	"Contador_Error",
	"Ultimo_Acceso",
	"Ultimo_Error",
	"Flags",
	0};

const char *perif_columns[] = {
	"Id",
	"MAC",
	"Dispositivo",
	"Tipo",
	"Estado",
	"Direccion_IP",
	"Ultimo_Ok",
	"Actualizar",
	"Flags",
	0};

const char *assign_columns[] = { 
	"Id",
	"Objeto",
	"Dispositivo",
	"Port",
	"Tipo",
	"Estado",
	"Icono0",
	"Icono1",
	"Grupo_Visual",
	"Planta",
	"Cord_x",
	"Cord_y",
	"Coeficiente",
	"Analog_Mult_Div",
	"Analog_Mult_Div_Valor",
	"Actualizar",
	"Flag",
	0};

const char *event_columns[] = {
		"Id",
		"Evento",
		"Objeto_Origen",
		"Objeto_Destino",
		"Grupo_Destino",
		"Funcion_Destino",
		"Variable_Destino",
		"ON_a_OFF",
		"OFF_a_ON",
		"Enviar",
		"Parametro_Evento",
		"Condicion_Variable",
		"Condicion_Igualdad",
		"Condicion_Valor",
		"Flags",
		0};

const char *auto_columns[] = {
		"Id",
		"Objeto",
		"Tipo",
		"Objeto_Sensor",
		"Min_Sensor",
		"Max_Sensor",
		"Hora_Inicio",
		"Hora_Fin",
		"Dias_Semana",
		"Estado",
		"Icono0",
		"Icono1",
		"Grupo_Visual",
		"Planta",
		"Cord_x",
		"Cord_y",
		"Actualizar",
		"Flags",
		0};

int ExisteColumna(const char* columna, const char** lista)
{
	int i = 0;

	if(!columna || !lista) return 0;
	while(lista[i])
	{
		if( !strcmp(columna, lista[i])) return 1;
		i++;
	}
	return 0;
}

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
	long temp_l;
	char temp_s[64];
	time_t t;
	time_t next_t;
	int delta_t;
	time_t update_ass_t;
	char s[16];
	int iEstado;

	char comando[1024];
	char objeto[1024];
	char parametro[1024];

	int update_hw_config_id;
	char update_hw_config_mac[16];
	int update_ass_status_id;
	int update_ass_status_hwid;
	char update_ass_status_name[256];

	STRFunc Strf;
	CGMServerBase::GMIOS call_resp;

    cJSON *json_obj;
    cJSON *json_un_obj;
    cJSON *json_un_obj2;
    cJSON *json_arr = NULL;
    cJSON *json_arr2 = NULL;
    cJSON *json_user;
    cJSON *json_pass;
    cJSON *json_channel;
    cJSON *json_query;
    cJSON *json_query_result;
    cJSON *json_cmdline;

    cJSON *json_HW_Id;
    cJSON *json_ASS_Id;
    cJSON *json_AUTO_Id;
    cJSON *json_MAC;
	cJSON *json_Direccion_IP;
	cJSON *json_Tipo;
	cJSON *json_Tipo_HW;
	cJSON *json_Tipo_ASS;
	//cJSON *json_Port;
	cJSON *json_Estado;
	//cJSON *json_Flags;
	cJSON *json_Objeto;
	cJSON *json_Accion;
	cJSON *json_Segundos;
	cJSON *json_Min;
	cJSON *json_Max;
	cJSON *json_Hora_Inicio;
	cJSON *json_Hora_Fin;
	cJSON *json_Dias_Semana;
	cJSON *json_ASS_Estado;
	

	update_hw_config_id = 0;
	update_hw_config_mac[0] = 0;
	update_ass_status_id = 0;
	update_ass_status_hwid = 0;
	update_ass_status_name[0] = 0;
	load_system_config = 1;
	update_system_config = 0;

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

				json_HW_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "ID");
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
								m_pServer->m_pLog->Add(10, "[HW] %s Solicita configuracion", json_HW_Id->valuestring);
								strcpy(update_hw_config_mac, json_HW_Id->valuestring);
							}
						}
					}
					else if(rc == 0)
					{
						/* NOT FOUND */
						m_pServer->m_pLog->Add(10, "[HW] %s No encontrado en la base", json_HW_Id->valuestring);
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"HW ID Not Found in Data Base\"}}");
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
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"HW ID Not Found in message\"}}");
				}
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
				}

			}
			/* ****************************************************************
			*		dompi_user_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_list"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT Id, Usuario, Nombre_Completo, Estado, Ultimo_Acceso "
								"FROM TB_DOM_USER");
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					sprintf(query, "SELECT * FROM TB_DOM_USER WHERE Id = %s", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
									if(ExisteColumna(json_un_obj->string, user_columns))
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
									if(ExisteColumna(json_un_obj->string, user_columns))
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					sprintf(query, "SELECT * FROM TB_DOM_PERIF WHERE Id = %s", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
									if(ExisteColumna(json_un_obj->string, perif_columns))
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
						sprintf(query, "DELETE FROM TB_DOM_PERIF WHERE Id = %s", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
									if(ExisteColumna(json_un_obj->string, perif_columns))
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					sprintf(query, "SELECT * FROM TB_DOM_ASSIGN WHERE Id = %s", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
									if(ExisteColumna(json_un_obj->string, assign_columns))
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
						sprintf(query, "DELETE FROM TB_DOM_ASSIGN WHERE Id = %s", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ass_update
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_update"))
			{
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;
				query_where[0] = 0;

				json_obj = cJSON_Parse(message);
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
									if(ExisteColumna(json_un_obj->string, assign_columns))
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
					sprintf(query, "UPDATE TB_DOM_ASSIGN SET %s WHERE %s", query_values, query_where);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
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

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
				}
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
					sprintf(query, "SELECT Id,Objeto,Tipo,Icono0,Icono1,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Estado FROM TB_DOM_ASSIGN WHERE Id = %s", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					sprintf(query, "SELECT Id,Objeto,Tipo,Icono0,Icono1,Grupo_Visual,Planta,Cord_x,Cord_y FROM TB_DOM_ASSIGN WHERE Id = %s", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					/* Actualizo el estado en la base */
					sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
									"SET Estado = 1 "
									"WHERE Objeto = \'%s\'", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					if(rc > 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"Error en update a TB_DOM_ASSIGN\"}}");
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					/* Actualizo el estado en la base */
					sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
									"SET Estado = 0 "
									"WHERE Objeto = \'%s\'", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					if(rc > 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Error en UPDATE a TB_DOM_ASSIGN\"}}");
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Error en UPDATE a TB_DOM_ASSIGN\"}}");
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					json_Segundos = cJSON_GetObjectItemCaseSensitive(json_obj, "Segundos");
					/* Actualizo el estado en la base */
					if(json_Segundos)
					{
						sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
										"SET Estado = %i "
										"WHERE Objeto = \'%s\'",
										atoi(json_Segundos->valuestring)+1,
										json_un_obj->valuestring);
					}
					else
					{
						sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
										"SET Estado = 2 "
										"WHERE Objeto = \'%s\'",
										json_un_obj->valuestring);
					}
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					if(rc > 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Error en UPDATE a TB_DOM_ASSIGN\"}}");
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ass_cmd
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_cmd"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				/* Viene: [{"System_Key":"D3S4RR0LL0","Time_Stamp":"1655813951","Objeto":"Luz Cocina","Accion":"switch"}] */
				json_Objeto = cJSON_GetObjectItemCaseSensitive(json_obj, "Objeto");
				json_Accion = cJSON_GetObjectItemCaseSensitive(json_obj, "Accion");
				if(json_Objeto && json_Accion)
				{
					/* TODO: Completar accion que viene desde la nube */
					if( !strcmp(json_Accion->valuestring, "on"))
					{
						/* Actualizo el estado en la base */
						sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
										"SET Estado = 1 "
										"WHERE Objeto = \'%s\'", json_Objeto->valuestring);
						m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
						pDB->Query(NULL, query);
					}
					else if( !strcmp(json_Accion->valuestring, "off"))
					{
						/* Actualizo el estado en la base */
						sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
										"SET Estado = 0 "
										"WHERE Objeto = \'%s\'", json_Objeto->valuestring);
						m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
						pDB->Query(NULL, query);
					}
					else if( !strcmp(json_Accion->valuestring, "switch"))
					{
						/* Actualizo el estado en la base */
						sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
										"SET Estado = (1 - Estado) "
										"WHERE Objeto = \'%s\'", json_Objeto->valuestring);
						m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
						pDB->Query(NULL, query);
					}
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					sprintf(query, "SELECT * FROM TB_DOM_EVENT WHERE Id = %s", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
									if(ExisteColumna(json_un_obj->string, event_columns))
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
						sprintf(query, "DELETE FROM TB_DOM_EVENT WHERE Id = %s", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
									if(ExisteColumna(json_un_obj->string, event_columns))
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
							/* TODO: Hacer algún mantenimiento si es necesario */

						}
						else if( !strcmp(comando, "sms") )
						{
							json_un_obj = cJSON_CreateObject();
							cJSON_AddStringToObject(json_un_obj, "SmsTo", (objeto)?objeto:"98765432");
							cJSON_AddStringToObject(json_un_obj, "SmsTxt", (parametro)?parametro:"test");
							cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
							m_pServer->m_pLog->Add(50, "Call [dompi_send_sms][%s]", message);
							rc = m_pServer->Call("dompi_send_sms", message, strlen(message), &call_resp, internal_timeout);
							m_pServer->m_pLog->Add(50, "Resp [dompi_send_sms][%s]", (const char*)call_resp.data);
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

						}
						else if( !strcmp(comando, "apagar") )
						{

						}
						else if( !strcmp(comando, "cambiar") )
						{

						}
						else if( !strcmp(comando, "pulso") )
						{

						}
						else if( !strcmp(comando, "estado") )
						{

						}
						else if( !strcmp(comando, "actualizar") )
						{

						}
					}
				}
				/* *********************************************************** */
				cJSON_Delete(json_obj);
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					sprintf(query, "SELECT * FROM TB_DOM_CONFIG WHERE Id = %s", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
									if(ExisteColumna(json_un_obj->string, system_columns))
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
										"(AU.Objeto_Salida = ASS.Id AND AU.Tipo = %s)", json_un_obj->valuestring);
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
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Falta dato Tipo\"}}");					
				}
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT * "
									"FROM TB_DOM_AUTO "
									"WHERE Id = 0 OR Tipo = %s", json_un_obj->valuestring);
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
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Falta dato Tipo\"}}");					
				}
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_get"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOM_AUTO WHERE Id = %s", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
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
// cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)
				json_un_obj = json_obj;

				/* Obtengo un ID para el elemento nuevo y lo cambio en el dato recibido */
				temp_l = pDB->NextId("TB_DOM_AUTO", "Id");
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
									if(ExisteColumna(json_un_obj->string, auto_columns))
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
				sprintf(query, "INSERT INTO TB_DOM_AUTO %s VALUES %s", query_into, query_values);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_delete
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_delete"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					if( atoi(json_un_obj->valuestring) != 0 )
					{
						sprintf(query, "DELETE FROM TB_DOM_AUTO WHERE Id = %s", json_un_obj->valuestring);
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_update
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_update"))
			{
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;
				query_where[0] = 0;

				json_obj = cJSON_Parse(message);
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
									if(ExisteColumna(json_un_obj->string, auto_columns))
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
					sprintf(query, "UPDATE TB_DOM_AUTO SET %s WHERE %s", query_values, query_where);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
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

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_status
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_status"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_arr = cJSON_CreateArray();
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono0,Icono1,Estado,Estado_Sensor,Estado_Salida "
									"FROM TB_DOM_AUTO "
									"WHERE Id = %s", json_un_obj->valuestring);
				}
				else
				{
					json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Planta");
					if(json_un_obj)
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono0,Icono1,Estado,Estado_Sensor,Estado_Salida "
										"FROM TB_DOM_AUTO "
										"WHERE Planta = %s", json_un_obj->valuestring);
					}
					else
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono0,Icono1,Estado,Estado_Sensor,Estado_Salida "
										"FROM TB_DOM_AUTO");
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_info
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_info"))				
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_arr = cJSON_CreateArray();
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_un_obj)
				{
					sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono0,Icono1,Grupo_Visual,Planta,Cord_x,Cord_y "
									"FROM TB_DOM_AUTO WHERE Id = %s", json_un_obj->valuestring);
				}
				else
				{
					json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Planta");
					if(json_un_obj)
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono0,Icono1,Grupo_Visual,Planta,Cord_x,Cord_y "
										"FROM TB_DOM_AUTO WHERE Planta = %s", json_un_obj->valuestring);
					}
					else
					{
						strcpy(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono0,Icono1,Grupo_Visual,Planta,Cord_x,Cord_y "
										"FROM TB_DOM_AUTO");
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_enable
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_enable"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Objeto");
				if(json_un_obj)
				{
					/* Actualizo el estado en la base */
					sprintf(query, 	"UPDATE TB_DOM_AUTO "
									"SET Estado = 1 "
									"WHERE Objeto = \'%s\'", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					if(rc > 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"Error en update a TB_DOM_ASSIGN\"}}");
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_disable
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_disable"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Objeto");
				if(json_un_obj)
				{
					/* Actualizo el estado en la base */
					sprintf(query, 	"UPDATE TB_DOM_AUTO "
									"SET Estado = 0 "
									"WHERE Objeto = \'%s\'", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					if(rc > 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Error en UPDATE a TB_DOM_ASSIGN\"}}");
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
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [%s]", fn);
				}
			}



















			/* ****************************************************************
			*		FIN SERVICIOS
			**************************************************************** */
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

		m_pServer->m_pLog->Add(100, "Control de tareas programadas");

		/* Controlo si hay que actualizar configuración de dispositivo */
		json_arr = cJSON_CreateArray();
		sprintf(query, "SELECT * "
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

				m_pServer->m_pLog->Add(100, "Actualizar [%s]", json_MAC->valuestring);
				if(atoi(json_Tipo_HW->valuestring) == 0)
				{
					/* RBPi Local */

					/* TODO: Actualizar I/O de RBPi  */
				}
				else if(atoi(json_Tipo_HW->valuestring) == 1)
				{
					/* Actualizar I/O de Dom32IOWiFi */
					json_arr2 = cJSON_CreateArray();
					sprintf(query, "SELECT Objeto, ASS.Id AS ASS_Id, ASS.Tipo AS Tipo_ASS, Port "
									"FROM TB_DOM_ASSIGN AS ASS "
									"WHERE Dispositivo = %s", json_HW_Id->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr2, query);
					if(rc == 0)
					{
						/* Recorro el array */
						cJSON_ArrayForEach(json_un_obj2, json_arr2)
						{
							json_Objeto = cJSON_GetObjectItemCaseSensitive(json_un_obj2, "Objeto");
							m_pServer->m_pLog->Add(100, "Actualizar estado de Assign [%s]", json_Objeto->valuestring);
							/* Le agrego los datos del dispositivo */
							cJSON_AddStringToObject(json_un_obj2, "Direccion_IP", json_Direccion_IP->valuestring);
							cJSON_AddStringToObject(json_un_obj2, "MAC", json_MAC->valuestring);
							cJSON_AddStringToObject(json_un_obj2, "Tipo_HW", json_Tipo_HW->valuestring);
							cJSON_PrintPreallocated(json_un_obj2, message, MAX_BUFFER_LEN, 0);
							m_pServer->m_pLog->Add(50, "Call [dompi_hw_set_port_config][%s]", message);
							m_pServer->Call("dompi_hw_set_port_config", message, strlen(message), &call_resp, internal_timeout);
							m_pServer->m_pLog->Add(50, "Resp [dompi_hw_set_port_config] [%s]", (const char*)call_resp.data);
							m_pServer->Free(call_resp);
						}
					}
					cJSON_Delete(json_arr2);
				}
				/* Borro la marca */
				sprintf(query, "UPDATE TB_DOM_PERIF "
								"SET Actualizar = 0 "
								"WHERE Id = %s", json_HW_Id->valuestring);
				m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
				pDB->Query(NULL, query);
			}
		}
		cJSON_Delete(json_arr);

		/* Controlo si hay que actualizar estados de Assign */
		json_arr = cJSON_CreateArray();
		sprintf(query, "SELECT MAC, PERIF.Tipo AS Tipo_HW, Direccion_IP, Objeto, "
								"ASS.Id AS ASS_Id, ASS.Tipo AS Tipo_ASS, Port, ASS.Estado "
						"FROM TB_DOM_PERIF AS PERIF, TB_DOM_ASSIGN AS ASS "
						"WHERE ASS.Dispositivo = PERIF.Id AND (ASS.Tipo = 0 OR ASS.Tipo = 3 OR ASS.Tipo = 5) AND "
						"( (ASS.Estado <> ASS.Estado_HW) OR (ASS.Actualizar <> 0) )");
		m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
		rc = pDB->Query(json_arr, query);
		if(rc == 0)
		{
			/* Recorro el array */
			cJSON_ArrayForEach(json_un_obj, json_arr)
			{
				json_Objeto = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Objeto");
				json_ASS_Id = cJSON_GetObjectItemCaseSensitive(json_un_obj, "ASS_Id");
				json_Estado = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Estado");
				json_Tipo_ASS = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Tipo_ASS");
				m_pServer->m_pLog->Add(100, "Actualizar estado de Assign [%s]", json_Objeto->valuestring);
				cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
				/* Me fijo si es estado o pulso */
				if(atoi(json_Estado->valuestring) >= 2 && ( atoi(json_Tipo_ASS->valuestring) == 0 || atoi(json_Tipo_ASS->valuestring) == 5 ) )
				{
					m_pServer->m_pLog->Add(50, "Call [dompi_hw_pulse_io][%s]", message);
					rc = m_pServer->Call("dompi_hw_pulse_io", message, strlen(message), &call_resp, internal_timeout);
					m_pServer->m_pLog->Add(50, "Resp [dompi_hw_pulse_io] [%s]", (const char*)call_resp.data);
				}
				else
				{
					m_pServer->m_pLog->Add(50, "Call [dompi_hw_set_io][%s]", message);
					rc = m_pServer->Call("dompi_hw_set_io", message, strlen(message), &call_resp, internal_timeout);
					m_pServer->m_pLog->Add(50, "Resp [dompi_hw_set_io] [%s]", (const char*)call_resp.data);

					/* Notifico a la nube */
					m_pServer->m_pLog->Add(50, "Post [dompi_ass_change][%s]", message);
					m_pServer->Post("dompi_ass_change", message, strlen(message));
				}

				if(rc == 0)
				{
					/* Borro la diferencia */
					iEstado = atoi(json_Estado->valuestring);
					if(iEstado != 1) iEstado = 0;
					sprintf(query, "UPDATE TB_DOM_ASSIGN "
									"SET Estado = %i, Estado_HW = %i, Actualizar = 0 "
									"WHERE Id = %s", iEstado, iEstado, json_ASS_Id->valuestring);
					m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
					pDB->Query(NULL, query);
				}
				m_pServer->Free(call_resp);
			}
		}
		cJSON_Delete(json_arr);

		/* Marcar para actualizar configuracion todos los assign de un periferico por MAC */
		if(update_hw_config_mac[0])
		{
			sprintf(query, "UPDATE TB_DOM_PERIF "
							"SET Actualizar = 1 "
							"WHERE MAC = \'%s\'", update_hw_config_mac);
			m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
			pDB->Query(NULL, query);
			update_hw_config_mac[0] = 0;
		}

		/* Marcar para actualizar configuracion todos los assign de un periferico por Id */
		if(update_hw_config_id)
		{
			sprintf(query, "UPDATE TB_DOM_PERIF "
							"SET Actualizar = 1 "
							"WHERE Id = %i", update_hw_config_id);
			m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
			pDB->Query(NULL, query);
			update_hw_config_id = 0;
		}

		/* Marcar para actualizar estado todos los assign por id de HW */
		if(update_ass_status_hwid)
		{
			sprintf(query, "UPDATE TB_DOM_ASSIGN "
							"SET Actualizar = 1 "
							"WHERE Dispositivo = %i", update_ass_status_hwid);
			m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
			pDB->Query(NULL, query);
			update_ass_status_hwid = 0;
		}

		/* Marcar para actualizar estado un assign por id */
		if(update_ass_status_id)
		{
			sprintf(query, "UPDATE TB_DOM_ASSIGN "
							"SET Actualizar = 1 "
							"WHERE Id = %i", update_ass_status_id);
			m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
			pDB->Query(NULL, query);
			update_ass_status_id = 0;
		}

		/* Marcar para actualizar estado un assign por nombre */
		if(update_ass_status_name[0])
		{
			sprintf(query, "UPDATE TB_DOM_ASSIGN "
							"SET Actualizar = 1 "
							"WHERE Id = %s", update_ass_status_name);
			m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
			pDB->Query(NULL, query);
			update_ass_status_name[0] = 0;
		}

		if(load_system_config)
		{
			LoadSystemConfig();
		}

		if(update_system_config && json_System_Config)
		{
			cJSON_PrintPreallocated(json_System_Config->child, message, MAX_BUFFER_LEN, 0);
			m_pServer->m_pLog->Add(50, "Call [dompi_cloud_config][%s]", message);
			rc = m_pServer->Call("dompi_cloud_config", message, strlen(message), &call_resp, internal_timeout);
			m_pServer->m_pLog->Add(50, "Resp [dompi_cloud_config][%s]", (const char*)call_resp.data);
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
			update_ass_t = t + 60;
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
				m_pServer->m_pLog->Add(50, "Post [dompi_ass_status_update][%s]", message);
				m_pServer->Post("dompi_ass_status_update", message, strlen(message));
			}
			else
			{
				cJSON_Delete(json_query_result);
			}
		}

		/* Control del modulo de automatización */
		json_arr = cJSON_CreateArray();
		sprintf(query, "SELECT AU.Id AS AUTO_Id, AU.Tipo, AU.Min_Sensor, AU.Max_Sensor, AU.Hora_Inicio, AU.Hora_Fin, AU.Dias_Semana, AU.Estado AUTO_Estado "
							"ASS.Id AS ASS_Id, ASS.Estado AS ASS_Estado "
							"ASS.Dispositivo AS PERIF_Id "
						"FROM TB_DOM_AUTO AS AU, TB_DOM_ASSIGN AS ASS "
						"WHERE AU.Objeto_Sensor = ASS.Id AND AU.Id > 0");
		m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
		rc = pDB->Query(json_arr, query);
		if(rc == 0)
		{
			/* Recorro el array */
			cJSON_ArrayForEach(json_un_obj, json_arr)
			{
				json_AUTO_Id = cJSON_GetObjectItemCaseSensitive(json_un_obj, "AUTO_Id");
				json_Tipo = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Tipo");
				json_Min = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Min_Sensor");
				json_Max = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Max_Sensor");
				json_Hora_Inicio = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Hora_Inicio");
				json_Hora_Fin = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Hora_Fin");
				json_Dias_Semana = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Hora_Fin");
				json_Estado = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Estado");
				json_ASS_Id = cJSON_GetObjectItemCaseSensitive(json_un_obj, "ASS_Id");
				json_ASS_Estado = cJSON_GetObjectItemCaseSensitive(json_un_obj, "ASS_Estado");
				json_HW_Id = cJSON_GetObjectItemCaseSensitive(json_un_obj, "PERIF_Id");


			}
		}
		cJSON_Delete(json_arr);

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
	m_pServer->UnSuscribe("dompi_auto_list", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_list_all", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_get", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_add", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_delete", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_update", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_status", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_info", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_enable", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_disable", GM_MSG_TYPE_CR);

	delete pEV;
	delete pConfig;
	delete pDB;
	delete m_pServer;
	exit(0);
}

