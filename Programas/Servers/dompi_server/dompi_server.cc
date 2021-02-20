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
#include <syslog.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>

#include "config.h"
#include "csqlite.h"
#include "gevent.h"

CGMServerWait *m_pServer;
DPConfig *pConfig;
CSQLite *pDB;
GEvent *pEV;

void OnClose(int sig);

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[4096];
	unsigned long message_len;
	char db_filename[FILENAME_MAX+1];
    cJSON *json_obj;
    cJSON *json_arr = NULL;

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

#ifdef __DEBUG__
  openlog("dompi_server", 0, LOG_USER);
#endif

	m_pServer = new CGMServerWait;
	m_pServer->Init("dompi_server");
	m_pServer->m_pLog->Add(1, "Iniciando Servidor de Domotica...");
#ifdef __DEBUG__
    syslog(LOG_DEBUG, "Iniciando Servidor de Domotica...");
#endif  

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

	pEV = new GEvent(pDB);

	m_pServer->m_pLog->Add(1, "Registrando Servicios: dompi_infoio");
	if(( rc =  m_pServer->Suscribe("dompi_infoio", GM_MSG_TYPE_CR)) != GME_OK)
	{
		m_pServer->m_pLog->Add(1, "ERROR %i al suscribir servicio dompi_infoio", rc);
		OnClose(0);
	}
	if(( rc =  m_pServer->Suscribe("dompi_user_list", GM_MSG_TYPE_CR)) != GME_OK)
	{
		m_pServer->m_pLog->Add(1, "ERROR %i al suscribir servicio dompi_user_list", rc);
		OnClose(0);
	}
	if(( rc =  m_pServer->Suscribe("dompi_user_add", GM_MSG_TYPE_CR)) != GME_OK)
	{
		m_pServer->m_pLog->Add(1, "ERROR %i al suscribir servicio dompi_user_add", rc);
		OnClose(0);
	}
	if(( rc =  m_pServer->Suscribe("dompi_user_delete", GM_MSG_TYPE_CR)) != GME_OK)
	{
		m_pServer->m_pLog->Add(1, "ERROR %i al suscribir servicio dompi_user_delete", rc);
		OnClose(0);
	}
	if(( rc =  m_pServer->Suscribe("dompi_user_update", GM_MSG_TYPE_CR)) != GME_OK)
	{
		m_pServer->m_pLog->Add(1, "ERROR %i al suscribir servicio dompi_user_update", rc);
		OnClose(0);
	}

	m_pServer->m_pLog->Add(1, "Inicializacion OK");
	m_pServer->SetLogLevel(20);
	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, (-1) )) >= 0)
	{
		if(rc > 0)
		{
#ifdef __DEBUG__
		    syslog(LOG_DEBUG, "Query recibido fn = [%s] rc = %i", fn, rc);
#endif  
			if( !strcmp(fn, "dompi_infoio"))
			{
				rc = pEV->ExtIOEvent(message);
				if(rc != 0)
				{
					m_pServer->m_pLog->Add(100, "Error %i en ExtIOEvent()", rc);
				}
				if(rc == 1)
				{
					/* OK */
					strcpy(message, "{RC=0}");
				}
				else if(rc == 0)
				{
					/* NOT FOUND */
					strcpy(message, "{RC=2}");
				}
				else
				{
					/* Otro Error */
					strcpy(message, "{RC=1}");
				}
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			else if( !strcmp(fn, "dompi_user_list"))
			{
				message[0] = 0;

				/*
				Ej.
				{ "departamento":8,
				"nombredepto":"Ventas",
				"director": "Juan Rodríguez",
				"empleados":[ { "nombre":"Pedro", "apellido":"Fernández" },
								{ "nombre":"Jacinto", "apellido":"Benavente" } ]
				}
				*/
				json_arr = cJSON_CreateArray();
				rc = pDB->Query(json_arr, "SELECT user_id, name, last_access_ok, access_error_count, last_access_error FROM TB_DOM_USER;");
				if(rc == 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "usuarios", json_arr);
					cJSON_PrintPreallocated(json_obj, message, 4095, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_arr);
				}
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			else if( !strcmp(fn, "dompi_user_add"))
			{




				
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			else if( !strcmp(fn, "dompi_user_delete"))
			{




				
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			else if( !strcmp(fn, "dompi_user_update"))
			{




				
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
				}
			}
			else
			{
				m_pServer->m_pLog->Add(50, "GME_SVC_NOTFOUND");
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
	m_pServer->UnSuscribe("dompi_user_add", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_user_delete", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_user_update", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_user_list", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_infoio", GM_MSG_TYPE_CR);
	delete pEV;
	delete pConfig;
	delete pDB;
	delete m_pServer;
	exit(0);
}
