
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
#include "cdb.h"
#include "gevent.h"
#include "alarma.h"
#include "strfunc.h"
#include "defines.h"

#define MAX_BUFFER_LEN 32767
#define BT_BUF_SIZE 256


CGMServerWait *m_pServer;
DPConfig *pConfig;
int internal_timeout;
CDB *pDB;
GEvent *pEV;
CAlarma *pAlarma;
cJSON *json_System_Config;

time_t last_daily;

void OnClose(int sig);
void CheckHWOffline( void );
void GroupTask( void );
void AssignTask( void );
void CheckTask();
void CheckUpdateHWConfig();
void RunDaily( void );
void CheckDaily();
void LoadSystemConfig(void);
int CheckWirelessCard( const char* card );
void CheckWiegandData(void);


/*                            11111111112222222222333333333344444444445555555555666666666677777777778
                     12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
char cli_help[] = 	"-------------------------------------------------------------------------------\r\n"
                    "Sistema de Domotica - Consola de comandos\r\n"
                    "\r\n"
                    "DomPiWeb Server V 0.1\r\n"
                    "\r\n"
					"Comandos disponibles:\r\n"
					"  listar <tipo>\r\n"
					"  encender <objeto>\r\n"
					"  apagar <objeto>\r\n"
					"  cambiar <objeto>\r\n"
					"  pulso <objeto>, [segundos]\r\n"
					"  estado <objeto>\r\n"
					"  actualizar <dispositivo>, <modulo>\r\n"
					"  manten\r\n"
					"  sms <numero>, <mensaje>\r\n"
					"  help\r\n"
					"  * tipo: dispositivos, objetos, grupos, eventos.\r\n"
					"    objeto: Nombre de un objeto existente.\r\n"
					"    dispositivo: MAC de un dispositivo existente.\r\n"
					"    modulo: wifi, ports.\r\n"
					"    segundos: duracion en segundos. Si no se especifica el default es 1.\r\n"
					"    numero: Numero de telefono destino del mensaje.\r\n"
					"    mensaje: Mensaje a enviar.\r\n"
                    "\r\n"
					"-------------------------------------------------------------------------------\r\n"
                    "\r\n";


int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[MAX_BUFFER_LEN+1];
	char cmdline[1024];
	char db_host[32];
	char db_user[32];
	char db_password[32];
	char query[4096];
	unsigned long message_len;
	time_t t;
	time_t next_t;
	int delta_t;
	char s[16];

	char comando[1024];
	char objeto[1024];
	char parametro[1024];

	char update_hw_config_mac[16];
	
	STRFunc Strf;
	CGMServerBase::GMIOS call_resp;

    cJSON *json_Message;
    cJSON *json_obj;
    cJSON *json_un_obj;
    cJSON *json_Query_Result = NULL;
	cJSON *json_Query_Row;
    cJSON *json_query;
    cJSON *json_cmdline;

    cJSON *json_HW_Id;
	cJSON *json_Objeto;
	cJSON *json_Accion;
	cJSON *json_Segundos;
	cJSON *json_Planta;
	cJSON *json_Id;
	cJSON *json_Nombre;
	cJSON *json_Config;
	
	last_daily = 0;

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

	pEV = new GEvent(pDB, m_pServer);
	pAlarma = new CAlarma(pDB, m_pServer);

	m_pServer->Suscribe("dompi_infoio", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_status", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_info", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_on", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_off", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_switch", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_pulse", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_cmdline", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_status", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_info", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_enable", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_disable", GM_MSG_TYPE_CR);

    m_pServer->Suscribe("dompi_alarm_part_info", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_part_status", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_part_on_total", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_part_on_parcial", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_part_off", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_zona_enable", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_zona_disable", GM_MSG_TYPE_CR);

	m_pServer->Suscribe("dompi_reload_config", GM_MSG_TYPE_MSG);		/* Sin respuesta, llega a todos */
	m_pServer->Suscribe("dompi_cloud_notification", GM_MSG_TYPE_NOT);	/* Sin respuesta, lo atiende el mas libre */

	m_pServer->m_pLog->Add(1, "Servicios de Domotica inicializados.");

	t = time(&t);
	next_t = t + 10;
	delta_t = 500;

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, delta_t )) >= 0)
	{
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(90, "%s:(Q)[%s]", fn, message);
			/* ****************************************************************
			*		dompi_infoio - Notificacion de estado y/o cambio de I/O
			**************************************************************** */
			if( !strcmp(fn, "dompi_infoio"))
			{
				json_Message = cJSON_Parse(message);
				//message[0] = 0;

				json_HW_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "ID");
				if(json_HW_Id)
				{
					rc = pEV->ExtIOEvent(message);
					//message[0] = 0;
					if(rc == 1)
					{
						pAlarma->ExtIOEvent(message);
						message[0] = 0;
						/* OK */
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
						/* Si está todo bien me fijo si pidio enviar configuracion */
						json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Message, "GETCONF");
						if(json_un_obj)
						{
							if( atoi(json_un_obj->valuestring) > 0 )
							{
								m_pServer->m_pLog->Add(50, "[HW] %s Solicita configuracion", json_HW_Id->valuestring);
								strcpy(update_hw_config_mac, json_HW_Id->valuestring);
							}
						}
					}
					else if(rc == 0)
					{
						/* NOT FOUND */
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
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}

				cJSON_Delete(json_Message);

			}
			/* ****************************************************************
			*		dompi_ass_status
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_status"))				
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				json_Query_Result = cJSON_CreateArray();
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				json_Planta = cJSON_GetObjectItemCaseSensitive(json_Message, "Planta");
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
					if(json_Message) cJSON_Delete(json_Message);
					json_Message = cJSON_CreateObject();
					cJSON_AddItemToObject(json_Message, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_Message, message, MAX_BUFFER_LEN, 0);
				}
				if(json_Message) cJSON_Delete(json_Message);
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
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				json_Query_Result = cJSON_CreateArray();
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				json_Planta = cJSON_GetObjectItemCaseSensitive(json_Message, "Planta");
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
					if(json_Message) cJSON_Delete(json_Message);
					json_Message = cJSON_CreateObject();
					cJSON_AddItemToObject(json_Message, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_Message, message, MAX_BUFFER_LEN, 0);
				}
				if(json_Message) cJSON_Delete(json_Message);
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ass_on
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_on"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Message, "Objeto");
				if(json_un_obj)
				{
					rc = pEV->ChangeAssignByName(json_un_obj->valuestring, 1, 0);
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
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ass_off
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_off"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Message, "Objeto");
				if(json_un_obj)
				{
					rc = pEV->ChangeAssignByName(json_un_obj->valuestring, 2, 0);
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
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ass_switch
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_switch"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Message, "Objeto");
				if(json_un_obj)
				{
					rc = pEV->ChangeAssignByName(json_un_obj->valuestring, 3, 0);
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
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_ass_pulse
			**************************************************************** */
			else if( !strcmp(fn, "dompi_ass_pulse"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Message, "Objeto");
				if(json_un_obj)
				{
					json_Segundos = cJSON_GetObjectItemCaseSensitive(json_Message, "Segundos");
					/* Actualizo el estado en la base */
					rc = pEV->ChangeAssignByName(json_un_obj->valuestring, 4, (json_Segundos)?stoi(json_Segundos->valuestring):0);
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
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_cmdline - input de domcli
			**************************************************************** */
			else if( !strcmp(fn, "dompi_cmdline"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				/* *********************************************************** */
				json_query = cJSON_GetObjectItemCaseSensitive(json_Message, "query");
				if(json_query)
				{
					json_cmdline = cJSON_GetObjectItemCaseSensitive(json_query, "CmdLine");
					if(json_cmdline)
					{
						strcpy(cmdline, json_cmdline->valuestring);

						Strf.ParseCommand(cmdline, comando, objeto, parametro);

						m_pServer->m_pLog->Add(80, "[dompi_cmdline] Comando: %s - Objeto: %s - Parametro: %s", 
											(comando)?comando:"NULL", 
											(objeto)?objeto:"NULL", 
											(parametro)?parametro:"NULL");

						if( !strcmp(comando, "help") || !strcmp(comando, "?"))
						{
							sprintf(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"%s\"}}", cli_help);
						}
						else if( !strcmp(comando, "listar") )
						{
							/* TODO: Completar comando listar */
							if( !memcmp(objeto, "dis", 3))
							{
								
							}
							else if( !memcmp(objeto, "obj", 3))
							{
								
							}
							else if( !memcmp(objeto, "gru", 3))
							{
								
							}
							else if( !memcmp(objeto, "eve", 3))
							{
								
							}
							
						}
						else if( !strcmp(comando, "manten") )
						{
							/* TODO: Hacer algún mantenimiento si es necesario */
							m_pServer->m_pLog->Add(100, "[manten] Mantenimiento de la base de datos.");
							if(pDB) pDB->Manten();

						}
						else if( !strcmp(comando, "sms") )
						{
							if(objeto && parametro)
							{
								json_un_obj = cJSON_CreateObject();
								cJSON_AddStringToObject(json_un_obj, "SmsTo", (objeto)?objeto:"98765432");
								cJSON_AddStringToObject(json_un_obj, "SmsTxt", (parametro)?parametro:"test");
								cJSON_PrintPreallocated(json_un_obj, message, MAX_BUFFER_LEN, 0);
								m_pServer->m_pLog->Add(90, "Call [dompi_send_sms][%s]", message);
								rc = m_pServer->Call("dompi_send_sms", message, strlen(message), &call_resp, internal_timeout);
								if(rc == 0)
								{
									m_pServer->m_pLog->Add(90, "Resp [dompi_send_sms][%s]", (const char*)call_resp.data);
									strcpy(message, (const char*)call_resp.data);
									m_pServer->Free(call_resp);
								}
								else
								{
									sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error en envio de SMS\"}}", rc);
								}
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Falta un dato\"}}");
							}
						}
						/* TODO: Completar varios comandos sobre objetos */
						else if( !strcmp(comando, "encender") )
						{
							if(objeto)
							{
								pEV->ChangeAssignByName(objeto, 1, 0);
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Falta un dato\"}}");
							}
						}
						else if( !strcmp(comando, "apagar") )
						{
							if(objeto)
							{
								pEV->ChangeAssignByName(objeto, 2, 0);
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Falta un dato\"}}");
							}
						}
						else if( !strcmp(comando, "cambiar") || !strcmp(comando, "switch") )
						{
							if(objeto)
							{
								pEV->ChangeAssignByName(objeto, 3, 0);
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Falta un dato\"}}");
							}
						}
						else if( !strcmp(comando, "pulso") )
						{
							if(objeto)
							{
								pEV->ChangeAssignByName(objeto, 4, 0);
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Falta un dato\"}}");
							}
						}
						else if( !strcmp(comando, "estado") )
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"No implementado\"}}");
						}
						else if( !strcmp(comando, "actualizar") )
						{
							/* Saco los datos que necesito */
							if( !memcmp(parametro, "wifi", 4))
							{
								json_Query_Result = cJSON_CreateArray();
								sprintf(query, "SELECT Id, MAC, Direccion_IP, Tipo AS Tipo_HW "
												"FROM TB_DOM_PERIF "
												"WHERE UPPER(MAC) = UPPER(\'%s\');", objeto);
								m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
								rc = pDB->Query(json_Query_Result, query);
								m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
								if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
								if(rc >= 0)
								{
									/* Obtengo el primero del array del resultado del query */
									cJSON_ArrayForEach(json_Query_Row, json_Query_Result) { break; }
									if(json_Query_Row)
									{
										/* Obtengo una copia del primer item del array de configuracion del sistema */
										cJSON_ArrayForEach(json_obj, json_System_Config) { break; }
										cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
										json_obj = nullptr;
										json_Config = cJSON_Parse(message);
										/* Le agrego los datos de la placa */
										cJSON_AddStringToObject(json_Config, "Id", 	cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Id")->valuestring);
										cJSON_AddStringToObject(json_Config, "MAC", cJSON_GetObjectItemCaseSensitive(json_Query_Row, "MAC")->valuestring);
										cJSON_AddStringToObject(json_Config, "Direccion_IP", cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Direccion_IP")->valuestring);
										cJSON_AddStringToObject(json_Config, "Tipo_HW", cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Tipo_HW")->valuestring);

										cJSON_PrintPreallocated(json_Config, message, MAX_BUFFER_LEN, 0);
										m_pServer->m_pLog->Add(90, "Notify [dompi_hw_set_comm_config][%s]", message);
										/* Se envía a todos */
										m_pServer->Notify("dompi_hw_set_comm_config", message, strlen(message));
										cJSON_Delete(json_Config);
									}
								}
								cJSON_Delete(json_Query_Result);
							}
							else if( !memcmp(objeto, "port", 4))
							{
								/* Actualizar I/O */

							}
						}
					}
				}
				/* *********************************************************** */
				cJSON_Delete(json_Message);
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_status
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_status"))				
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				json_Query_Result = cJSON_CreateArray();
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				json_Planta = cJSON_GetObjectItemCaseSensitive(json_Message, "Planta");
				if(json_Id)
				{
					sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono0,Icono1,Estado,Estado_Sensor,Estado_Salida "
									"FROM TB_DOM_AUTO "
									"WHERE Id = %s;", json_Id->valuestring);
				}
				else
				{
					if(json_Planta)
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono0,Icono1,Estado,Estado_Sensor,Estado_Salida "
										"FROM TB_DOM_AUTO "
										"WHERE Planta = %s;", json_Planta->valuestring);
					}
					else
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono0,Icono1,Estado,Estado_Sensor,Estado_Salida "
										"FROM TB_DOM_AUTO;");
					}
				}
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					if(json_Message) cJSON_Delete(json_Message);
					json_Message = cJSON_CreateObject();
					cJSON_AddItemToObject(json_Message, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_Message, message, MAX_BUFFER_LEN, 0);
				}
				if(json_Message) cJSON_Delete(json_Message);
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_info
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_info"))				
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				json_Query_Result = cJSON_CreateArray();
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				json_Planta = cJSON_GetObjectItemCaseSensitive(json_Message, "Planta");
				if(json_Id)
				{
					sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono0,Icono1,Grupo_Visual,Planta,Cord_x,Cord_y "
									"FROM TB_DOM_AUTO WHERE Id = %s;", json_Id->valuestring);
				}
				else
				{
					if(json_Planta)
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono0,Icono1,Grupo_Visual,Planta,Cord_x,Cord_y "
										"FROM TB_DOM_AUTO WHERE Planta = %s;", json_Planta->valuestring);
					}
					else
					{
						strcpy(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono0,Icono1,Grupo_Visual,Planta,Cord_x,Cord_y "
										"FROM TB_DOM_AUTO;");
					}
				}
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					if(json_Message) cJSON_Delete(json_Message);
					json_Message = cJSON_CreateObject();
					cJSON_AddItemToObject(json_Message, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_Message, message, MAX_BUFFER_LEN, 0);
				}
				if(json_Message) cJSON_Delete(json_Message);
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_auto_enable
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_enable"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Message, "Objeto");
				if(json_un_obj)
				{
					/* Actualizo el estado en la base */
					sprintf(query, 	"UPDATE TB_DOM_AUTO "
									"SET Estado = 1 "
									"WHERE UPPER(Objeto) = UPPER(\'%s\');", json_un_obj->valuestring);
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
						strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"Error en update a TB_DOM_ASSIGN\"}}");
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
			*		dompi_auto_disable
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_disable"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Message, "Objeto");
				if(json_un_obj)
				{
					/* Actualizo el estado en la base */
					sprintf(query, 	"UPDATE TB_DOM_AUTO "
									"SET Estado = 0 "
									"WHERE UPPER(Objeto) = UPPER(\'%s\')", json_un_obj->valuestring);
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
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Error en UPDATE a TB_DOM_ASSIGN\"}}");
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
			*		dompi_alarm_part_info
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_part_info"))
			{

			}
			/* ****************************************************************
			*		dompi_alarm_part_on_total
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_part_on_total"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				json_Nombre = cJSON_GetObjectItemCaseSensitive(json_Message, "Nombre");

				if(json_Id)
				{
					sprintf(query,  "UPDATE TB_DOM_ALARM_PARTICION "
									"SET ActStatus = 2 "
									"WHERE Id = %s;", json_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc <= 0)
					{
						m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				else if(json_Nombre)
				{
					sprintf(query,  "UPDATE TB_DOM_ALARM_PARTICION "
									"SET ActStatus = 2 "
									"WHERE Id = %s;", json_Nombre->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc <= 0)
					{
						m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}

				cJSON_Delete(json_Message);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_alarm_part_on_parcial
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_part_on_parcial"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				json_Nombre = cJSON_GetObjectItemCaseSensitive(json_Message, "Nombre");

				if(json_Id)
				{
					sprintf(query,  "UPDATE TB_DOM_ALARM_PARTICION "
									"SET ActStatus = 1 "
									"WHERE Id = %s;", json_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc <= 0)
					{
						m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				else if(json_Nombre)
				{
					sprintf(query,  "UPDATE TB_DOM_ALARM_PARTICION "
									"SET ActStatus = 1 "
									"WHERE Id = %s;", json_Nombre->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc <= 0)
					{
						m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}

				cJSON_Delete(json_Message);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_alarm_part_off
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_part_off"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				json_Nombre = cJSON_GetObjectItemCaseSensitive(json_Message, "Nombre");

				if(json_Id)
				{
					sprintf(query,  "UPDATE TB_DOM_ALARM_PARTICION "
									"SET ActStatus = 0 "
									"WHERE Id = %s;", json_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc <= 0)
					{
						m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				else if(json_Nombre)
				{
					sprintf(query,  "UPDATE TB_DOM_ALARM_PARTICION "
									"SET ActStatus = 0 "
									"WHERE Id = %s;", json_Nombre->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc <= 0)
					{
						m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}

				cJSON_Delete(json_Message);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_alarm_zona_enable
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_zona_enable"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");

				if(json_Id)
				{
					sprintf(query,  "UPDATE TB_DOM_ALARM_ZONA "
									"SET Activa = 1 "
									"WHERE Id = %s;", json_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc <= 0)
					{
						m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}

				cJSON_Delete(json_Message);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_alarm_zona_disable
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_zona_disable"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;

				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");

				if(json_Id)
				{
					sprintf(query,  "UPDATE TB_DOM_ALARM_ZONA "
									"SET Activa = 0 "
									"WHERE Id = %s;", json_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc <= 0)
					{
						m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}

				cJSON_Delete(json_Message);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_cloud_notification
			**************************************************************** */
			else if( !strcmp(fn, "dompi_cloud_notification"))
			{
				m_pServer->Resp(NULL, 0, GME_OK);
				/* Un array de acciones sobre objetos */
				json_Query_Result = cJSON_Parse(message);
				if(cJSON_IsArray(json_Query_Result))
				{
					cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
					{
						json_Objeto = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Objeto");
						json_Accion = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Accion");

						if(json_Objeto && json_Accion)
						{
							m_pServer->m_pLog->Add(100, "[dompi_cloud_notification] Objeto: %s - Accion: %s", 
								json_Objeto->valuestring, json_Accion->valuestring);

							if( !strcmp(json_Accion->valuestring, "on"))
							{
								pEV->ChangeAssignByName(json_Objeto->valuestring, 1, 0);
							}
							else if( !strcmp(json_Accion->valuestring, "off"))
							{
								pEV->ChangeAssignByName(json_Objeto->valuestring, 2, 0);
							}
							else if( !strcmp(json_Accion->valuestring, "switch"))
							{
								pEV->ChangeAssignByName(json_Objeto->valuestring, 3, 0);
							}
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
				m_pServer->Resp(NULL, 0, GME_OK);

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

		/* ********************************************************************
		 *   Actualizaciones de estados
		 *
		 *
		 * *******************************************************************/
		CheckWiegandData();

		GroupTask();

		AssignTask();

		/* Marcar para actualizar configuracion todos los assign de un periferico por MAC */
		if(update_hw_config_mac[0])
		{
			m_pServer->m_pLog->Add(50, "Actualizar configuracion de HW MAC: %s", update_hw_config_mac);
			sprintf(query, "UPDATE TB_DOM_PERIF "
							"SET Actualizar = 1 "
							"WHERE UPPER(MAC) = UPPER(\'%s\');", update_hw_config_mac);
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(NULL, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			update_hw_config_mac[0] = 0;
		}

		/* ********************************************************************
		 *   Timer
		 *
		 *
		 * *******************************************************************/
		/* Recalculo del timer */
		t = time(&t);
		if(next_t > t)
		{
			/* Todavía no se venció el timer */
			delta_t = (next_t-t)*100;
		}
		else
		{
			/* El timer ya está vencido */
			m_pServer->m_pLog->Add(100, "[TIMER] Tareas dentro del timer");

			/* Tareas diarias */
			CheckTask();

			CheckDaily();

			CheckUpdateHWConfig();

			pEV->CheckAuto(0, nullptr, 0);

			CheckHWOffline();

			/* Controles del modulo de alarma */

			/* Tareas programadas en TB_DOM_AT */

			/*  */
			delta_t = 1000;
			next_t = t + 10;
		}
	}
	m_pServer->m_pLog->Add(1, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);

	m_pServer->UnSuscribe("dompi_infoio", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_status", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_info", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_on", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_off", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_switch", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_pulse", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_cmdline", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_status", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_info", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_enable", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_disable", GM_MSG_TYPE_CR);

    m_pServer->UnSuscribe("dompi_alarm_part_info", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_part_status", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_part_on_total", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_part_on_parcial", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_part_off", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_zona_enable", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_zona_disable", GM_MSG_TYPE_CR);

	m_pServer->UnSuscribe("dompi_reload_config", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_cloud_notification", GM_MSG_TYPE_NOT);

	delete pEV;
	delete pConfig;
	delete pDB;

	exit(0);
}

void CheckHWOffline( void )
{
	char query[4096];
	int rc;
	time_t t;
	cJSON *json_QueryArray;
	cJSON *json_QueryRow;
	cJSON *json_HW_Id;
	cJSON *json_MAC;
	cJSON *json_Direccion_IP;

	m_pServer->m_pLog->Add(100, "[CheckHWOffline]");

	/* Dispositivos offline */
	t = time(&t);
	json_QueryArray = cJSON_CreateArray();
	sprintf(query, "SELECT Id, MAC, Direccion_IP "
					"FROM TB_DOM_PERIF "
					"WHERE Estado <> 0 AND Ultimo_Ok < %lu;", t-180);
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_QueryArray, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc >= 0)
	{
		/* Recorro el array */
		cJSON_ArrayForEach(json_QueryRow, json_QueryArray)
		{
			/* Saco los datos que necesito */
			json_HW_Id = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Id");
			json_MAC = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "MAC");
			json_Direccion_IP = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Direccion_IP");

			m_pServer->m_pLog->Add(10, "[HW] %s %s Estado: OFF LINE", json_MAC->valuestring,json_Direccion_IP->valuestring );

			sprintf(query, "UPDATE TB_DOM_PERIF "
							"SET Estado = 0 "
							"WHERE Id = %s;", json_HW_Id->valuestring);
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(NULL, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
		}
	}
	cJSON_Delete(json_QueryArray);
}

void GroupTask( void )
{
	char query[4096];
	int rc;
	char *id, *p;
	int accion;
	cJSON *json_QueryArray;
	cJSON *json_QueryRow;
	cJSON *json_Id;
	cJSON *json_Grupo;
	cJSON *json_Listado_Objetos;
	cJSON *json_Estado;

	m_pServer->m_pLog->Add(100, "[GroupTask]");

	json_QueryArray = cJSON_CreateArray();
	strcpy(query, "SELECT * FROM TB_DOM_GROUP WHERE ACTUALIZAR = 1;");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_QueryArray, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc >= 0)
	{
		cJSON_ArrayForEach(json_QueryRow, json_QueryArray)
		{
			json_Id = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Id");
			json_Grupo = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Grupo");
			json_Listado_Objetos = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Listado_Objetos");
			json_Estado = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Estado");

			m_pServer->m_pLog->Add(20, "[GroupTask] Actualizando estado de I/O de Grupo [%s]", json_Grupo->valuestring);
			/* Encender o apagar */
			if(atoi(json_Estado->valuestring))
			{
				/* Grupo encendido -> mando a encender los miembros */
				accion = 1;
			}
			else
			{
				/* Grupo apagado -> mando a apagar los miembros */
				accion = 2;
			}
			/* Recorro el listado de id separados por , */
			p = json_Listado_Objetos->valuestring;
			while(*p)
			{
				id = p;
				while(*p && *p != ',') p++;
				if(*p)
				{
					*p = 0;
					p++;
				}
				pEV->ChangeAssignById(atoi(id), accion, 0);
			}
			sprintf(query, "UPDATE TB_DOM_GROUP "
							"SET Actualizar = 0 "
							"WHERE Id = %s;", json_Id->valuestring);
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(NULL, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
		}
	}
	cJSON_Delete(json_QueryArray);
}

void AssignTask( void )
{
	char query[4096];
	char message[4096];
	int rc;
	cJSON *json_QueryArray;
	cJSON *json_QueryRow;
	cJSON *json_Objeto;
	cJSON *json_ASS_Id;
	cJSON *json_Estado;
	cJSON *json_Tipo_ASS;
	int iEstado;


	m_pServer->m_pLog->Add(100, "[AssignTask]");

	/* Controlo si hay que actualizar estados de Assign de dispositivos que estén en linea */
	json_QueryArray = cJSON_CreateArray();
	sprintf(query, "SELECT MAC, PERIF.Tipo AS Tipo_HW, Direccion_IP, Objeto, "
							"ASS.Id AS ASS_Id, ASS.Tipo AS Tipo_ASS, Port, ASS.Estado "
					"FROM TB_DOM_PERIF AS PERIF, TB_DOM_ASSIGN AS ASS "
					"WHERE ASS.Dispositivo = PERIF.Id AND "
					     "PERIF.Estado = 1 AND "
					     "(ASS.Tipo = 0 OR ASS.Tipo = 3 OR ASS.Tipo = 5) AND "
					     "( (ASS.Estado <> ASS.Estado_HW) OR (ASS.Actualizar <> 0) );");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_QueryArray, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc >= 0)
	{
		/* Recorro el array */
		cJSON_ArrayForEach(json_QueryRow, json_QueryArray)
		{
			json_Objeto = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Objeto");
			json_ASS_Id = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "ASS_Id");
			json_Estado = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Estado");
			json_Tipo_ASS = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Tipo_ASS");
			m_pServer->m_pLog->Add(50, "Actualizar estado de Assign [%s]", json_Objeto->valuestring);
			cJSON_PrintPreallocated(json_QueryRow, message, MAX_BUFFER_LEN, 0);
			/* Me fijo si es estado o pulso */
			if(atoi(json_Estado->valuestring) >= 2 && ( atoi(json_Tipo_ASS->valuestring) == 0 || atoi(json_Tipo_ASS->valuestring) == 5 ) )
			{
				m_pServer->m_pLog->Add(90, "Notify [dompi_hw_pulse_io][%s]", message);
				/* Se envía a todos */
				rc = m_pServer->Notify("dompi_hw_pulse_io", message, strlen(message));
			}
			else
			{
				m_pServer->m_pLog->Add(90, "Notify [dompi_hw_set_io][%s]", message);
				/* Se envía a todos */
				rc = m_pServer->Notify("dompi_hw_set_io", message, strlen(message));

				m_pServer->m_pLog->Add(90, "Notify [dompi_ass_change][%s]", message);
				/* Se envía a todos */
				m_pServer->Notify("dompi_ass_change", message, strlen(message));
			}

			if(rc == 0)
			{
				/* Borro la diferencia */
				iEstado = atoi(json_Estado->valuestring);
				if(iEstado != 1) iEstado = 0;
				sprintf(query, "UPDATE TB_DOM_ASSIGN "
								"SET Estado = %i, Estado_HW = %i, Actualizar = 0 "
								"WHERE Id = %s;", iEstado, iEstado, json_ASS_Id->valuestring);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(NULL, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			}
		}
	}
	cJSON_Delete(json_QueryArray);
}

void CheckTask()
{
	int rc;
	char query[4096];
	time_t t;
	struct tm *now;
	char dia_semana[3];

    cJSON *json_QueryArray;
    cJSON *json_QueryRow;
	cJSON *json_Id;
	cJSON *json_Agenda;
	cJSON *json_Objeto_Destino;
	cJSON *json_Grupo_Destino;
	cJSON *json_Funcion_Destino;
	cJSON *json_Variable_Destino;
	cJSON *json_Evento;
	cJSON *json_Parametro_Evento;
	cJSON *json_Condicion_Variable;
	cJSON *json_Condicion_Igualdad;
	cJSON *json_Condicion_Valor;
	
	m_pServer->m_pLog->Add(100, "[CheckTask]");

	t = time(&t);
	now = localtime(&t);

	switch(now->tm_wday)
	{
		case 0:
			strcpy(dia_semana, "Do");
			break;
		case 1:
			strcpy(dia_semana, "Lu");
			break;
		case 2:
			strcpy(dia_semana, "Ma");
			break;
		case 3:
			strcpy(dia_semana, "Mi");
			break;
		case 4:
			strcpy(dia_semana, "Ju");
			break;
		case 5:
			strcpy(dia_semana, "Vi");
			break;
		case 6:
			strcpy(dia_semana, "Sa");
			break;
		default:
			strcpy(dia_semana, "XX");
			break;
	}

	json_QueryArray = cJSON_CreateArray();
	sprintf(query, "SELECT * "
					"FROM TB_DOM_AT "
					"WHERE ((Mes = 0) OR (Mes = %i)) AND "
					      "((Dia = 0) OR (Dia = %i)) AND  "
					      "((Hora > 23) OR (Hora = %i)) AND  "
					      "((Minuto > 59) OR (Minuto = %i)) AND  "
							"( (Ultimo_Mes <> %i) OR "
							  "(Ultimo_Dia <> %i) OR "
							  "(Ultima_Hora <> %i) OR "
							  "(Ultimo_Minuto <> %i) ) AND "
						  /*"CHARINDEX(\'%s\', Dias_Semana) > 0 "*/
						  "INSTR(Dias_Semana, \'%s\') > 0 "
					"ORDER BY Id;",
					now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min,
					now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min,
					dia_semana);
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_QueryArray, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc >= 0)
	{
		/* Recorro el array */
		cJSON_ArrayForEach(json_QueryRow, json_QueryArray)
		{
			json_Id = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Id");
			json_Agenda = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Agenda");
			json_Objeto_Destino = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Objeto_Destino");
			json_Grupo_Destino = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Grupo_Destino");
			json_Funcion_Destino = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Funcion_Destino");
			json_Variable_Destino = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Variable_Destino");
			json_Evento = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Evento");
			json_Parametro_Evento = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Parametro_Evento");
			json_Condicion_Variable = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Condicion_Variable");
			json_Condicion_Igualdad = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Condicion_Igualdad");
			json_Condicion_Valor = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Condicion_Valor");

			if(json_Agenda && json_Objeto_Destino && json_Grupo_Destino && json_Funcion_Destino &&
			   json_Variable_Destino && json_Evento && json_Parametro_Evento && 
			   json_Condicion_Variable && json_Condicion_Igualdad && json_Condicion_Valor)
			{
				m_pServer->m_pLog->Add(10, "[TASK] Ejecutando tarea: %s", json_Agenda->valuestring);
				
				if(atoi(json_Objeto_Destino->valuestring) != 0)
				{
					pEV->ChangeAssignById(atoi(json_Objeto_Destino->valuestring), atoi(json_Evento->valuestring), atoi(json_Parametro_Evento->valuestring));
				}
				else if(atoi(json_Grupo_Destino->valuestring) != 0)
				{
					pEV->ChangeGroupById(atoi(json_Grupo_Destino->valuestring), atoi(json_Evento->valuestring), atoi(json_Parametro_Evento->valuestring));
				}
				else if(atoi(json_Funcion_Destino->valuestring) != 0)
				{
					pEV->ChangeFcnById(atoi(json_Funcion_Destino->valuestring), atoi(json_Evento->valuestring), atoi(json_Parametro_Evento->valuestring));
				}
				else if(atoi(json_Condicion_Variable->valuestring) != 0)
				{
					pEV->ChangeVarById(atoi(json_Condicion_Variable->valuestring), atoi(json_Evento->valuestring), atoi(json_Parametro_Evento->valuestring));
				}
				/* Actualizo la ejecución */
				sprintf(query, "UPDATE TB_DOM_AT "
								"SET  Ultimo_Mes = %i, "
									 "Ultimo_Dia = %i, "
									 "Ultima_Hora = %i, "
									 "Ultimo_Minuto = %i "
								"WHERE Id = %s;",
								now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, json_Id->valuestring);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(NULL, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			}
		}
	}
	cJSON_Delete(json_QueryArray);
}

void CheckUpdateHWConfig()
{
	int rc;
	char query[4096];
	char message[4096];

    cJSON *json_Config;
    cJSON *json_arr_Perif;
    cJSON *json_Perif;
    cJSON *json_arr_Assign;
    cJSON *json_HW_Id;
    cJSON *json_MAC;
    cJSON *json_Tipo;
    cJSON *json_Direccion_IP;
    cJSON *json_Flags;


	m_pServer->m_pLog->Add(100, "[CheckUpdateHWConfig]");

	/* Controlo si hay que actualizar configuración de dispositivo */
	json_arr_Perif = cJSON_CreateArray();
	sprintf(query, "SELECT * "
					"FROM TB_DOM_PERIF "
					"WHERE Actualizar <> 0 AND Estado <> 0 "
					"ORDER BY Id;");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_arr_Perif, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc >= 0)
	{
		/* Recorro el array */
		cJSON_ArrayForEach(json_Perif, json_arr_Perif)
		{
			json_MAC = cJSON_GetObjectItemCaseSensitive(json_Perif, "MAC");
			json_Tipo = cJSON_GetObjectItemCaseSensitive(json_Perif, "Tipo");
			json_HW_Id = cJSON_GetObjectItemCaseSensitive(json_Perif, "Id");
			json_Flags = cJSON_GetObjectItemCaseSensitive(json_Perif, "Flags");
			json_Direccion_IP = cJSON_GetObjectItemCaseSensitive(json_Perif, "Direccion_IP");	

			if(json_MAC && json_Tipo && json_HW_Id && json_Flags && json_Direccion_IP)
			{
				m_pServer->m_pLog->Add(10, "[CheckUpdateHWConfig] Actualizar HW [%s]", json_MAC->valuestring);

				/* Un objeto para contener a todos */
				json_Config = cJSON_CreateObject();
				/* Saco los datos que necesito */
				cJSON_AddStringToObject(json_Config, "Id", json_HW_Id->valuestring);
				cJSON_AddStringToObject(json_Config, "MAC", json_MAC->valuestring);
				cJSON_AddStringToObject(json_Config, "Direccion_IP", json_Direccion_IP->valuestring);
				cJSON_AddStringToObject(json_Config, "Tipo_HW", json_Tipo->valuestring);

				if(json_Flags)
				{
					if(atoi(json_Flags->valuestring) & 0x01)
					{
						cJSON_AddStringToObject(json_Config, "HTTPS", "yes");
					}
					else
					{
						cJSON_AddStringToObject(json_Config, "HTTPS", "no");
					}
					if(atoi(json_Flags->valuestring) & 0x02)
					{
						cJSON_AddStringToObject(json_Config, "WIEGAND", "yes");
					}
					else
					{
						cJSON_AddStringToObject(json_Config, "WIEGAND", "no");
					}
					//if(atoi(json_Flags->valuestring) & 0x04)
					//{
					//	cJSON_AddStringToObject(json_Config, "DHT2x", "yes");
					//}
					//else
					//{
					//	cJSON_AddStringToObject(json_Config, "DHT2x", "no");
					//}
				}

				if(atoi(json_Tipo->valuestring) == TIPO_HW_WIFI || atoi(json_Tipo->valuestring) == TIPO_HW_RBPI)
				{
					/* Actualizar I/O de Dom32IOWiFi y RBPi*/
					json_arr_Assign = cJSON_AddArrayToObject(json_Config, "Ports");
					sprintf(query, "SELECT Objeto, ASS.Id AS ASS_Id, ASS.Tipo AS Tipo_ASS, Port "
									"FROM TB_DOM_ASSIGN AS ASS "
									"WHERE Dispositivo = %s;", json_HW_Id->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr_Assign, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc >= 0)
					{
						cJSON_PrintPreallocated(json_Config, message, MAX_BUFFER_LEN, 0);
						m_pServer->m_pLog->Add(90, "Notify [dompi_hw_set_port_config][%s]", message);
						/* Se envía a todos */
						m_pServer->Notify("dompi_hw_set_port_config", message, strlen(message));
					}
				}
				/* Borro la marca */
				sprintf(query, "UPDATE TB_DOM_PERIF "
								"SET Actualizar = 0 "
								"WHERE Id = %s;", json_HW_Id->valuestring);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(NULL, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);

				cJSON_Delete(json_Config);
			}
		}
	}
	//cJSON_Delete(json_arr_Perif);
}

void RunDaily( void )
{
	m_pServer->m_pLog->Add(10, "[Daily] Mantenimiento de la base de datos.");

	if(pDB) pDB->Manten();

}

void CheckDaily()
{
	time_t now;
	struct tm *tmNow, *tmLastDaily;
	int day_now, day_last;

	m_pServer->m_pLog->Add(100, "[CheckDaily]");

	now = time(&now);
	tmNow = localtime(&now);
	day_now = tmNow->tm_mday;
	tmLastDaily = localtime(&last_daily);
	day_last = tmLastDaily->tm_mday;

	if(day_now != day_last)
	{
		RunDaily();
		last_daily = now;
	}
}

void LoadSystemConfig(void)
{
	char query[4096];
	int rc;

	if(pDB == NULL) return;

	m_pServer->m_pLog->Add(100, "[LoadSystemConfig]");

	if(json_System_Config) cJSON_Delete(json_System_Config);
	json_System_Config = cJSON_CreateArray();
	strcpy(query, "SELECT * FROM TB_DOM_CONFIG ORDER BY Id DESC LIMIT 1;");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_System_Config, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc >= 0) m_pServer->m_pLog->Add(1, "[LoadSystemConfig] Lectura de configuracion OK.");
}

int CheckWirelessCard( const char* card )
{
	int rc;
	char query[4096];
	int auth = 0;
	time_t t;
	struct tm *now;
	char dia_semana[3];
	
    cJSON *QueryResult;
    cJSON *QueryRow;
	cJSON *Nombre_Completo;
	cJSON *Permisos;
	cJSON *Dias_Semana;
	cJSON *Hora_Desde;
	cJSON *Minuto_Desde;
	cJSON *Hora_Hasta;
	cJSON *Minuto_Hasta;
	cJSON *Estado;

	int i_now, i_desde, i_hasta;

	m_pServer->m_pLog->Add(100, "[CheckWirelessCard]");

	t = time(&t);
	now = localtime(&t);
	i_now = (now->tm_hour * 100) + now->tm_min;

	switch(now->tm_wday)
	{
		case 0:
			strcpy(dia_semana, "Do");
			break;
		case 1:
			strcpy(dia_semana, "Lu");
			break;
		case 2:
			strcpy(dia_semana, "Ma");
			break;
		case 3:
			strcpy(dia_semana, "Mi");
			break;
		case 4:
			strcpy(dia_semana, "Ju");
			break;
		case 5:
			strcpy(dia_semana, "Vi");
			break;
		case 6:
			strcpy(dia_semana, "Sa");
			break;
		default:
			strcpy(dia_semana, "XX");
			break;
	}

	if(!card) return 0;
	if(strlen(card) == 0) return 0;

	m_pServer->m_pLog->Add(100, "[CheckWirelessCard] Tarjeta: %s", card);

	QueryResult = cJSON_CreateArray();
	sprintf(query, "SELECT Nombre_Completo, Permisos, Dias_Semana, Hora_Desde, Minuto_Desde, Hora_Hasta, Minuto_Hasta, Estado "
					"FROM TB_DOM_USER "
					"WHERE UPPER(Tarjeta) = UPPER(\'%s\');", card);
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(QueryResult, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc >= 0)
	{
		/* Recorro el array */
		cJSON_ArrayForEach(QueryRow, QueryResult)
		{
			Nombre_Completo = cJSON_GetObjectItemCaseSensitive(QueryRow, "Nombre_Completo");
			Permisos = cJSON_GetObjectItemCaseSensitive(QueryRow, "Permisos");
			Dias_Semana = cJSON_GetObjectItemCaseSensitive(QueryRow, "Dias_Semana");
			Hora_Desde = cJSON_GetObjectItemCaseSensitive(QueryRow, "Hora_Desde");
			Minuto_Desde = cJSON_GetObjectItemCaseSensitive(QueryRow, "Minuto_Desde");
			Hora_Hasta = cJSON_GetObjectItemCaseSensitive(QueryRow, "Hora_Hasta");
			Minuto_Hasta = cJSON_GetObjectItemCaseSensitive(QueryRow, "Minuto_Hasta");
			Estado = cJSON_GetObjectItemCaseSensitive(QueryRow, "Estado");

			m_pServer->m_pLog->Add(20, "[CheckWirelessCard] Verificando Tarjeta: %s de Usuario: %s", card, Nombre_Completo->valuestring);

			/* Controlo día de la semana */
			if(strstr(Dias_Semana->valuestring, dia_semana))
			{
				i_desde = (atoi(Hora_Desde->valuestring) * 100) + atoi(Minuto_Desde->valuestring);
				i_hasta = (atoi(Hora_Hasta->valuestring) * 100) + atoi(Minuto_Hasta->valuestring);

				m_pServer->m_pLog->Add(100, "[CheckWirelessCard] Hora %i <= %i <= %i", i_desde, i_now, i_hasta);
				/* Controlo horario */
				if(i_desde == i_hasta)
				{
					auth = atoi(Estado->valuestring);
				}
				else if( i_desde <= i_hasta && i_now >= i_desde && i_now <= i_hasta )
				{
					auth = atoi(Estado->valuestring);
				}
				else if( i_desde > i_hasta && ( i_now >= i_desde || i_now <= i_hasta ) )
				{
					auth = atoi(Estado->valuestring);
				}

				if(auth)
				{
					m_pServer->m_pLog->Add(10, "[CheckWirelessCard] Aceptada Tarjeta: %s de Usuario: %s", card, Nombre_Completo->valuestring);
				}
				else
				{
					m_pServer->m_pLog->Add(1, "[CheckWirelessCard] Tarjeta: %s de Usuario: %s Acceso denegado", card, Nombre_Completo->valuestring);
				}
			}
		}
	}
	cJSON_Delete(QueryResult);

	return auth;
}

void CheckWiegandData( void )
{
	int rc;
	char query[4096];

    cJSON *QueryResult;
    cJSON *QueryRow;
    cJSON *Id;
    cJSON *Dispositivo;
    cJSON *Perif_Data;

	m_pServer->m_pLog->Add(100, "[CheckWiegandData]");

	QueryResult = cJSON_CreateArray();
	sprintf(query, "SELECT Id, Dispositivo, Perif_Data "
					"FROM TB_DOM_ASSIGN "
					"WHERE Port = \'CARD\' AND NOT ISNULL(Perif_Data);");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(QueryResult, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc >= 0)
	{
		/* Recorro el array */
		cJSON_ArrayForEach(QueryRow, QueryResult)
		{
			Id = cJSON_GetObjectItemCaseSensitive(QueryRow, "Id");
			Dispositivo = cJSON_GetObjectItemCaseSensitive(QueryRow, "Dispositivo");
			Perif_Data = cJSON_GetObjectItemCaseSensitive(QueryRow, "Perif_Data");
			/* Verifico si el código el válido */
			rc = CheckWirelessCard( Perif_Data->valuestring );
			/* Ejecuto el Evento */
			pEV->CheckEvent(atoi(Dispositivo->valuestring), "CARD", rc);
			/* Borro el dato */
			sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
							"SET Perif_Data = NULL "
							"WHERE Id = %s;", Id->valuestring);
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(NULL, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li", rc, pDB->LastQueryTime());
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
		}
	}
	cJSON_Delete(QueryResult);
}

