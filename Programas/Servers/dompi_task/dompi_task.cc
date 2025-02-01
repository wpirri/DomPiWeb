
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
#include "gevent.h"
#include "strfunc.h"
#include "defines.h"

CGMServerWait *m_pServer;
DPConfig *pConfig;
int internal_timeout;
CDB *pDB;
GEvent *pEV;
cJSON *json_System_Config;
#ifdef ACTIVO_ACTIVO
char sys_backup[32];
#endif
int i;

time_t last_daily;

#define MAX_HW_LIST_NEW 128
#define MAX_CARD_LIST_NEW 128

typedef struct _new_hw_list
{
	char mac[16];
	time_t last_info;
} new_hw_list;

new_hw_list g_dompi_server_new_hw_list[MAX_HW_LIST_NEW];

typedef struct _new_card_list
{
	char card[16];
	time_t last_info;
} new_card_list;

new_card_list g_dompi_server_new_card_list[MAX_CARD_LIST_NEW];

void OnClose(int sig);
void CheckHWOffline( void );
void GroupMaint( void );
void AssignTask( void );
void CheckTask();
void CheckUpdateHWConfig();
void RunDaily( void );
void CheckDaily();
void LoadSystemConfig(void);
void AutoChangeNotify(void);
void AddSaf( void );

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
	STRFunc sf;
	STRFunc Strf;
	//CGMServerBase::GMIOS call_resp;

	last_daily = 0;
	
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

	m_pServer = new CGMServerWait;
	m_pServer->Init("dompi_task");
	m_pServer->m_pLog->Add(1, "Iniciando Servidor de Tareas Offline...");

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

	/*
	Se distribuye equitativamente entre las colas menos cargadas
		GM_MSG_TYPE_CR		- Se espera respuesta (Call)
		GM_MSG_TYPE_NOT		- Sin respuesta (Notify)
		GM_MSG_TYPE_INT		- Mensaje particionado con continuación
	Se envía a todos los suscriptos
		GM_MSG_TYPE_MSG		- Sin respuesta (Post)
	*/

	m_pServer->Suscribe("dompi_reload_config", GM_MSG_TYPE_MSG);		/* Sin respuesta, llega a todos */
	m_pServer->Suscribe("dompi_cloud_notification", GM_MSG_TYPE_NOT);	/* Sin respuesta, lo atiende el mas libre */

	AddSaf();

	m_pServer->m_pLog->Add(1, "Servicios de Tareas Offline inicializados.");

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 500 )) >= 0)
	{
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(90, "%s:(Q)[%s]", fn, message);
			/* ****************************************************************
			*		dompi_reload_config
			**************************************************************** */
			if( !strcmp(fn, "dompi_reload_config"))
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

		/* ********************************************************************
		 *   Actualizaciones de estados
		 *
		 *
		 * *******************************************************************/
		GroupMaint();
		AssignTask();
		/* Hay que actualizar estado de alarma */
		if(pEV->AlarmNeedUpdate())
		{
			m_pServer->m_pLog->Add(20, "Actualizar estados de alarmas");
			m_pServer->Notify("dompi_alarm_change", nullptr, 0);
		}

		m_pServer->m_pLog->Add(100, "[TIMER] Tareas dentro del timer");
		/* Tareas diarias */
		CheckDaily();
		/* Controles del modulo de alarma */
		pEV->Task_Alarma();
		/* Tareas programadas en TB_DOM_AT */
		CheckTask();
		pEV->CheckAuto(0, nullptr, 0);
		/*  */
		CheckUpdateHWConfig();
		/*  */
		CheckHWOffline();
		/*  */
		AutoChangeNotify();

	}
	m_pServer->m_pLog->Add(1, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);

	m_pServer->UnSuscribe("dompi_reload_config", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_cloud_notification", GM_MSG_TYPE_NOT);

	delete m_pServer;
	delete pEV;
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

void CheckHWOffline( void )
{
	char query[4096];
	int rc;
	unsigned long tolerancia = 0;
	time_t t;
	cJSON *json_QueryArray;
	cJSON *json_QueryRow;
	cJSON *json_HW_Id;
	cJSON *json_MAC;
	cJSON *json_Direccion_IP;
	cJSON *json_Last_Config;
	cJSON *Wifi_Report;

	m_pServer->m_pLog->Add(50, "[CheckHWOffline]");

	/* Traigo el intervalo de actualizacion de la configuración para calcular la tolerancia */
	if(json_System_Config)
	{
		cJSON_ArrayForEach(json_Last_Config, json_System_Config) { break; }
		if(json_Last_Config)
		{
			Wifi_Report = cJSON_GetObjectItemCaseSensitive(json_Last_Config, "Wifi_Report");
			if(Wifi_Report)
			{
				tolerancia = 3 * atoi(Wifi_Report->valuestring);
			}
		}
	}
	if(tolerancia == 0)	tolerancia = 180;
	/* Dispositivos offline */
	t = time(&t);
	json_QueryArray = cJSON_CreateArray();
	sprintf(query, "SELECT Id, MAC, Direccion_IP "
					"FROM TB_DOM_PERIF "
					"WHERE Estado <> 0 AND Ultimo_Ok < %lu;", t-tolerancia);
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_QueryArray, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc > 0)
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
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
		}
	}
	cJSON_Delete(json_QueryArray);
}

/* Mantiene el estado del grupo según el estado de sus miembros */
void GroupMaint( void )
{
	char query[4096];
	int rc;
	char *id, *p;
	bool todos_encendidos;
	bool todos_apagados;
	cJSON *json_QueryResult_Group;
	cJSON *json_QueryResult_Assign;
	cJSON *json_QueryRow_Group;
	cJSON *json_QueryRow_Assign;
	cJSON *json_Id;
	cJSON *json_Grupo;
	cJSON *json_Listado_Objetos;
	cJSON *json_Estado;
	cJSON *json_ASS_Objeto;
	cJSON *json_ASS_Estado;

	m_pServer->m_pLog->Add(50, "[GroupMaint]");

	json_QueryResult_Group = cJSON_CreateArray();
	strcpy(query, "SELECT * FROM TB_DOM_GROUP WHERE Id > 0;");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_QueryResult_Group, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc > 0)
	{
		cJSON_ArrayForEach(json_QueryRow_Group, json_QueryResult_Group)
		{
			json_Id = cJSON_GetObjectItemCaseSensitive(json_QueryRow_Group, "Id");
			json_Grupo = cJSON_GetObjectItemCaseSensitive(json_QueryRow_Group, "Grupo");
			json_Listado_Objetos = cJSON_GetObjectItemCaseSensitive(json_QueryRow_Group, "Listado_Objetos");
			json_Estado = cJSON_GetObjectItemCaseSensitive(json_QueryRow_Group, "Estado");

			m_pServer->m_pLog->Add(50, "[GroupMaint] Analizando Grupo [%s]", json_Grupo->valuestring);
			todos_encendidos = true;
			todos_apagados = true;
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
				/* ASSIGN = atoi(id) */
				sprintf(query, "SELECT Objeto, Estado "
								"FROM TB_DOM_ASSIGN "
								"WHERE Id = %s;", id);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				json_QueryResult_Assign = cJSON_CreateArray();
				rc = pDB->Query(json_QueryResult_Assign, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc > 0)
				{
					/* Obtengo el primero del array del resultado del query */
					cJSON_ArrayForEach(json_QueryRow_Assign, json_QueryResult_Assign)
					{
						json_ASS_Objeto = cJSON_GetObjectItemCaseSensitive(json_QueryRow_Assign, "Objeto");
						json_ASS_Estado = cJSON_GetObjectItemCaseSensitive(json_QueryRow_Assign, "Estado");
						m_pServer->m_pLog->Add(50, "[GroupMaint] Mienbro: %s (%s)", json_ASS_Objeto->valuestring, json_ASS_Estado->valuestring);

						if( atoi(json_ASS_Estado->valuestring) == 0 ) todos_encendidos = false;
						else if( atoi(json_ASS_Estado->valuestring) == 1 ) todos_apagados = false;
					}
				}
				else
				{
					m_pServer->m_pLog->Add(50, "[GroupMaint] Sin mienbros");
					todos_encendidos = false;
					todos_apagados = false;
				}
				cJSON_Delete(json_QueryResult_Assign);
			}

			if(todos_encendidos)
			{
				m_pServer->m_pLog->Add(50, "[GroupMaint] Todos encendidos");
			}

			if(todos_apagados)
			{
				m_pServer->m_pLog->Add(50, "[GroupMaint] Todos apagados");
			}

			if(todos_encendidos && ( atoi(json_Estado->valuestring) == 0 ))
			{
				m_pServer->m_pLog->Add(20, "[GroupMaint] Cambiando Grupo %s a estado 1", json_Grupo->valuestring);
				sprintf(query, "UPDATE TB_DOM_GROUP "
								"SET Estado = 1 "
								"WHERE Id = %s;", json_Id->valuestring);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(NULL, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			}
			else if(todos_apagados && ( atoi(json_Estado->valuestring) == 1 ))
			{
				m_pServer->m_pLog->Add(20, "[GroupMaint] Cambiando Grupo %s a estado 0", json_Grupo->valuestring);
				sprintf(query, "UPDATE TB_DOM_GROUP "
								"SET Estado = 0 "
								"WHERE Id = %s;", json_Id->valuestring);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(NULL, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			}
		}
	}
	cJSON_Delete(json_QueryResult_Group);
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


	m_pServer->m_pLog->Add(50, "[AssignTask]");

	/* Controlo si hay que actualizar estados de Assign de dispositivos que estén en linea */
	json_QueryArray = cJSON_CreateArray();
	sprintf(query, "SELECT MAC, PERIF.Tipo AS Tipo_HW, Direccion_IP, Objeto, "
							"ASS.Id AS ASS_Id, ASS.Tipo AS Tipo_ASS, Port, ASS.Estado, ASS.Analog_Mult_Div_Valor "
					"FROM TB_DOM_PERIF AS PERIF, TB_DOM_ASSIGN AS ASS "
					"WHERE ASS.Dispositivo = PERIF.Id AND "
					     "PERIF.Estado = 1 AND "
					     "(ASS.Tipo = 0 OR ASS.Tipo = 3 OR ASS.Tipo = 5) AND "
					     "( (ASS.Estado <> ASS.Estado_HW) OR (ASS.Actualizar <> 0) );");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_QueryArray, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc > 0)
	{
		/* Recorro el array */
		cJSON_ArrayForEach(json_QueryRow, json_QueryArray)
		{
			json_Objeto = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Objeto");
			json_ASS_Id = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "ASS_Id");
			json_Estado = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Estado");
			json_Tipo_ASS = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Tipo_ASS");
			m_pServer->m_pLog->Add(20, "Actualizar estado de Assign [%s] Estado: %s",
									json_Objeto->valuestring, json_Estado->valuestring);
			cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
			/* Me fijo si es estado o pulso */
			if(atoi(json_Tipo_ASS->valuestring) == 5)
			{	/* Pulso */
				m_pServer->m_pLog->Add(90, "Notify [dompi_hw_pulse_io][%s]", message);
				m_pServer->Notify("dompi_hw_pulse_io", message, strlen(message));
				iEstado = 0;
			}
			else
			{	/* El resto de las salidas */
				m_pServer->m_pLog->Add(90, "Notify [dompi_hw_set_io][%s]", message);
				m_pServer->Notify("dompi_hw_set_io", message, strlen(message));
				m_pServer->m_pLog->Add(90, "Notify [dompi_ass_change][%s]", message);
				m_pServer->Notify("dompi_ass_change", message, strlen(message));
#ifdef ACTIVO_ACTIVO
				/* Encolo la sincronización */
				if(strlen(sys_backup)) m_pServer->Enqueue("dompi_changeio_synch", message, strlen(message));
#endif
				iEstado = atoi(json_Estado->valuestring);
				if(iEstado != 1) iEstado = 0;
			}

			/* Borro la diferencia */
			sprintf(query, "UPDATE TB_DOM_ASSIGN "
							"SET Estado = %i, Estado_HW = %i, Actualizar = 0 "
							"WHERE Id = %s;", iEstado, iEstado, json_ASS_Id->valuestring);
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(NULL, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
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
	/*cJSON *json_Funcion_Destino;*/
	cJSON *json_Variable_Destino;
	cJSON *json_Evento;
	cJSON *json_Parametro_Evento;
	cJSON *json_Condicion_Variable;
	cJSON *json_Condicion_Igualdad;
	cJSON *json_Condicion_Valor;
	
	m_pServer->m_pLog->Add(50, "[CheckTask]");

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
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc > 0)
	{
		/* Recorro el array */
		cJSON_ArrayForEach(json_QueryRow, json_QueryArray)
		{
			json_Id = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Id");
			json_Agenda = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Agenda");
			json_Objeto_Destino = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Objeto_Destino");
			json_Grupo_Destino = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Grupo_Destino");
			/*json_Funcion_Destino = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Funcion_Destino");*/
			json_Variable_Destino = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Variable_Destino");
			json_Evento = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Evento");
			json_Parametro_Evento = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Parametro_Evento");
			json_Condicion_Variable = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Condicion_Variable");
			json_Condicion_Igualdad = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Condicion_Igualdad");
			json_Condicion_Valor = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Condicion_Valor");

			if(json_Agenda && json_Objeto_Destino && json_Grupo_Destino && /*json_Funcion_Destino &&*/
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
				/*
				else if(atoi(json_Funcion_Destino->valuestring) != 0)
				{
					pEV->ChangeFcnById(atoi(json_Funcion_Destino->valuestring), atoi(json_Evento->valuestring), atoi(json_Parametro_Evento->valuestring));
				}
				*/
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
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			}
			else
			{
				if( !json_Agenda ) m_pServer->m_pLog->Add(10, "[CheckTask] ERROR: Valor NULL para Agenda");
				if( !json_Objeto_Destino ) m_pServer->m_pLog->Add(10, "[CheckTask] ERROR: Valor NULL para Objeto_Destino");
				if( !json_Grupo_Destino ) m_pServer->m_pLog->Add(10, "[CheckTask] ERROR: Valor NULL para Grupo_Destino");
				/*if( !json_Funcion_Destino ) m_pServer->m_pLog->Add(10, "[CheckTask] ERROR: Valor NULL para Funcion_Destino");*/
				if( !json_Variable_Destino ) m_pServer->m_pLog->Add(10, "[CheckTask] ERROR: Valor NULL para Variable_Destino");
				if( !json_Evento ) m_pServer->m_pLog->Add(10, "[CheckTask] ERROR: Valor NULL para Evento");
				if( !json_Parametro_Evento ) m_pServer->m_pLog->Add(10, "[CheckTask] ERROR: Valor NULL para Parametro_Evento");
				if( !json_Condicion_Variable ) m_pServer->m_pLog->Add(10, "[CheckTask] ERROR: Valor NULL para Condicion_Variable");
				if( !json_Condicion_Igualdad ) m_pServer->m_pLog->Add(10, "[CheckTask] ERROR: Valor NULL para Condicion_Igualdad");
				if( !json_Condicion_Valor ) m_pServer->m_pLog->Add(10, "[CheckTask] ERROR: Valor NULL para Condicion_Valor");
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

    cJSON *json_obj;
    cJSON *json_Config;
    cJSON *json_arr_Perif;
    cJSON *json_Perif;
    cJSON *json_arr_Assign;
    cJSON *json_HW_Id;
    cJSON *json_MAC;
    cJSON *json_Dispositivo;
    cJSON *json_Tipo;
    cJSON *json_Direccion_IP;
	cJSON *json_Usar_Https;
	cJSON *json_Habilitar_Wiegand;
	cJSON *json_Update_Config;
	cJSON *json_Update_WiFi;

	m_pServer->m_pLog->Add(50, "[CheckUpdateHWConfig]");

	/* Controlo si hay que actualizar configuración de dispositivo */
	json_arr_Perif = cJSON_CreateArray();
	sprintf(query, "SELECT * "
					"FROM TB_DOM_PERIF "
					"WHERE Estado <> 0 "
					"AND ( Update_Config <> 0 OR Update_WiFi <> 0 ) "
					"ORDER BY Id;");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_arr_Perif, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc > 0)
	{
		/* Recorro el array */
		cJSON_ArrayForEach(json_Perif, json_arr_Perif)
		{
			json_MAC = cJSON_GetObjectItemCaseSensitive(json_Perif, "MAC");
			json_Dispositivo = cJSON_GetObjectItemCaseSensitive(json_Perif, "Dispositivo");
			json_Tipo = cJSON_GetObjectItemCaseSensitive(json_Perif, "Tipo");
			json_HW_Id = cJSON_GetObjectItemCaseSensitive(json_Perif, "Id");
			json_Direccion_IP = cJSON_GetObjectItemCaseSensitive(json_Perif, "Direccion_IP");	
			json_Usar_Https = cJSON_GetObjectItemCaseSensitive(json_Perif, "Usar_Https");
			json_Habilitar_Wiegand = cJSON_GetObjectItemCaseSensitive(json_Perif, "Habilitar_Wiegand");
			json_Update_Config = cJSON_GetObjectItemCaseSensitive(json_Perif, "Update_Config");
			json_Update_WiFi = cJSON_GetObjectItemCaseSensitive(json_Perif, "Update_WiFi");

			if(json_MAC && json_Tipo && json_HW_Id && json_Direccion_IP && json_Usar_Https && json_Habilitar_Wiegand)
			{
				if(atoi(json_Update_Config->valuestring))
				{
					m_pServer->m_pLog->Add(10, "[CheckUpdateHWConfig] Actualizar configuracion de HW [%s]", json_Dispositivo->valuestring);

					if(atoi(json_Tipo->valuestring) == TIPO_HW_WIFI || atoi(json_Tipo->valuestring) == TIPO_HW_RBPI)
					{
						/* Un objeto para contener a todos */
						json_Config = cJSON_CreateObject();
						/* Saco los datos que necesito */
						cJSON_AddStringToObject(json_Config, "Id", json_HW_Id->valuestring);
						cJSON_AddStringToObject(json_Config, "MAC", json_MAC->valuestring);
						cJSON_AddStringToObject(json_Config, "Direccion_IP", json_Direccion_IP->valuestring);
						cJSON_AddStringToObject(json_Config, "Tipo_HW", json_Tipo->valuestring);
						cJSON_AddStringToObject(json_Config, "HTTPS", (atoi(json_Usar_Https->valuestring))?"yes":"no");
						cJSON_AddStringToObject(json_Config, "WIEGAND", (atoi(json_Habilitar_Wiegand->valuestring))?"yes":"no");

						/* Actualizar I/O de Dom32IOWiFi y RBPi*/
						json_arr_Assign = cJSON_AddArrayToObject(json_Config, "Ports");
						sprintf(query, "SELECT Objeto, ASS.Id AS ASS_Id, ASS.Tipo AS Tipo_ASS, Port "
										"FROM TB_DOM_ASSIGN AS ASS "
										"WHERE Dispositivo = %s;", json_HW_Id->valuestring);
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						rc = pDB->Query(json_arr_Assign, query);
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						/* Aunque no haya assigs lo mando igual para que vaya la configuración de HTTPS y Wiegand */
						if(rc >= 0)
						{
							cJSON_PrintPreallocated(json_Config, message, GM_COMM_MSG_LEN, 0);
							m_pServer->m_pLog->Add(90, "Notify [dompi_hw_set_port_config][%s]", message);
							m_pServer->Notify("dompi_hw_set_port_config", message, strlen(message));
						}

						cJSON_Delete(json_Config);
					}
				}
				if(atoi(json_Update_WiFi->valuestring))
				{
					/* Obtengo una copia del primer item del array de configuracion del sistema */
					cJSON_ArrayForEach(json_obj, json_System_Config) { break; }
					cJSON_PrintPreallocated(json_obj, message, GM_COMM_MSG_LEN, 0);
					json_obj = nullptr;
					json_Config = cJSON_Parse(message);
					/* Le agrego los datos de la placa */
					cJSON_AddStringToObject(json_Config, "Id", 	json_HW_Id->valuestring);
					cJSON_AddStringToObject(json_Config, "MAC", json_MAC->valuestring);
					cJSON_AddStringToObject(json_Config, "Direccion_IP", json_Direccion_IP->valuestring);
					cJSON_AddStringToObject(json_Config, "Tipo_HW", json_Tipo->valuestring);

					cJSON_PrintPreallocated(json_Config, message, GM_COMM_MSG_LEN, 0);
					m_pServer->m_pLog->Add(90, "Notify [dompi_hw_set_comm_config][%s]", message);
					m_pServer->Notify("dompi_hw_set_comm_config", message, strlen(message));
					cJSON_Delete(json_Config);
				}
				/* Borro las marcas */
				sprintf(query, "UPDATE TB_DOM_PERIF "
								"SET Update_Config = 0, Update_WiFi = 0 "
								"WHERE Id = %s;", json_HW_Id->valuestring);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(NULL, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			}
		}
	}
	cJSON_Delete(json_arr_Perif);
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

	m_pServer->m_pLog->Add(50, "[CheckDaily]");

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

void AutoChangeNotify( void )
{
	char query[4096];
	char message[4096];
	int rc;
	//char *id, *p;
	//int accion;
	cJSON *json_QueryArray;
	cJSON *json_QueryRow;
	cJSON *json_Id;
	cJSON *json_Objeto;
	cJSON *json_Tipo;
	cJSON *json_Estado;

	int i_tipo;
	int i_id;
	char s_tipo[16];
	char s_id[16];

	m_pServer->m_pLog->Add(50, "[AutoChangeNotify]");

	json_QueryArray = cJSON_CreateArray();
	strcpy(query, "SELECT * FROM TB_DOM_AUTO WHERE Actualizar = 1;");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_QueryArray, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc > 0)
	{
		cJSON_ArrayForEach(json_QueryRow, json_QueryArray)
		{
			json_Id = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Id");
			json_Objeto = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Objeto");
			m_pServer->m_pLog->Add(20, "[AutoChangeNotify] Actualizando estado de automatismo [%s]", json_Objeto->valuestring);

			/* Cambio el Id (le sumo 10000) */
			i_id = atoi(json_Id->valuestring);
			sprintf(s_id, "%d", i_id + 10000);
			cJSON_DeleteItemFromObjectCaseSensitive(json_QueryRow, "Id");
			cJSON_AddStringToObject(json_QueryRow, "Id", s_id);

			/* Cambio el Tipo (le sumo 10) */
			json_Tipo = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Tipo");
			i_tipo = atoi(json_Tipo->valuestring);
			sprintf(s_tipo, "%d", i_tipo + 10);
			cJSON_DeleteItemFromObjectCaseSensitive(json_QueryRow, "Tipo");
			cJSON_AddStringToObject(json_QueryRow, "Tipo", s_tipo);

			/* Cambio el estado por el valor de Habilitado */
			json_Estado = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Habilitado");
			cJSON_DeleteItemFromObjectCaseSensitive(json_QueryRow, "Estado");
			cJSON_AddStringToObject(json_QueryRow, "Estado", json_Estado->valuestring);

			cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
			m_pServer->m_pLog->Add(90, "Notify [dompi_ass_change][%s]", message);
			m_pServer->Notify("dompi_ass_change", message, strlen(message));

			sprintf(query, "UPDATE TB_DOM_AUTO "
							"SET Actualizar = 0 "
							"WHERE Id = %i;", i_id);
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(NULL, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
		}
	}
	cJSON_Delete(json_QueryArray);
}

void AddSaf( void )
{
	m_pServer->Notify(".create-queue", "dompi_infoio_synch", 19);	
	m_pServer->Notify(".create-queue", "dompi_changeio_synch", 21);	
}
