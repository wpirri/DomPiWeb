
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
int wait_time;

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
void LoadSystemConfig(void);

void AssignTask( void );
void GroupTask( void );
void GroupMaint( void );
void CheckNewHWList(const char* mac);
void CheckNewCardList(const char* card);
void CheckHWOffline( void );
void CheckTask();
void CheckUpdateHWConfig();
void RunDaily( void );
void CheckDaily();
int CheckWirelessCard( const char* card );
void CheckWiegandData(void);
void AddSaf( void );

int main(/*int argc, char** argv, char** env*/void)
{
	int i;
	int rc;
	char fn[33];
	char typ[1];
	char message[GM_COMM_MSG_LEN+1];
	char alt_message[GM_COMM_MSG_LEN+1];
	char db_host[32];
	char db_name[32];
	char db_user[32];
	char db_password[32];
	char query[4096];
	unsigned long message_len;
	time_t entry_time, exit_time;
	struct tm *s_tm;
	char s[16];
	STRFunc sf;
	char extra_info[1024];
	STRFunc Strf;

    cJSON *json_Request;
    cJSON *json_Response;
    cJSON *json_obj;
    cJSON *json_un_obj;
    cJSON *json_Query_Result = NULL;
	cJSON *json_Query_Row;
	cJSON *json_Objeto;
	cJSON *json_Tipo_ASS;
	cJSON *json_Tipo_HW;
	cJSON *json_Port;
	cJSON *json_Estado;
	cJSON *json_Accion;
	cJSON *json_Segundos;
	cJSON *json_Planta;
	cJSON *json_Id;
	cJSON *json_Grupo;
	cJSON *json_Part;
	cJSON *json_Zona;
	cJSON *json_Salida;
	cJSON *json_MAC;
	cJSON *json_Direccion_IP;
    cJSON *json_arr_Perif;
    cJSON *json_Perif;
	
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
	m_pServer->Init("dompi_server");
	m_pServer->m_pLog->Add(1, "Iniciando Servidor de Domotica...");

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

	memset(g_dompi_server_new_hw_list, 0, sizeof(g_dompi_server_new_hw_list));

	/*
	Se distribuye equitativamente entre las colas menos cargadas
		GM_MSG_TYPE_CR		- Se espera respuesta (Call)
		GM_MSG_TYPE_NOT		- Sin respuesta (Notify)
		GM_MSG_TYPE_INT		- Mensaje particionado con continuación
	Se envía a todos los suscriptos
		GM_MSG_TYPE_MSG		- Sin respuesta (Post)
	*/
	m_pServer->Suscribe("dompi_infoio", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_hw_list_new", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_user_list_new_card", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_status", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_info", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_on", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_off", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_switch", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_ass_pulse", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_status", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_info", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_enable", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_auto_disable", GM_MSG_TYPE_CR);

    m_pServer->Suscribe("dompi_alarm_part_info", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_part_status", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_part_on_total", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_part_on_parcial", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_part_off", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_part_switch", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_zona_enable", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_zona_disable", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_zona_switch", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_salida_pulse", GM_MSG_TYPE_CR);

	m_pServer->Suscribe("dompi_reload_config", GM_MSG_TYPE_MSG);		/* Sin respuesta, llega a todos */
	m_pServer->Suscribe("dompi_cloud_notification", GM_MSG_TYPE_NOT);	/* Sin respuesta, lo atiende el mas libre */

	m_pServer->Suscribe("dompi_mobile_list_objects", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_mobile_touch_object", GM_MSG_TYPE_CR);

	AddSaf();

	m_pServer->m_pLog->Add(1, "Servicios de Domotica inicializados.");

	wait_time = 1;
	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, (wait_time*10) )) >= 0)
	{
		if(rc > 0)
		{
			wait_time = 1;
			entry_time = time(&entry_time);
			s_tm = localtime(&entry_time);
			message[message_len] = 0;
			m_pServer->m_pLog->Add(90, "%s:(Q)[%s]", fn, message);
			/* ****************************************************************
			*		dompi_infoio - Notificacion de estado y/o cambio de I/O
			**************************************************************** */
			if( !strcmp(fn, "dompi_infoio"))
			{
				json_Request = cJSON_Parse(message);
				json_Response = cJSON_CreateObject();
				//message[0] = 0; No blanquear, se usa mas adelante

				json_MAC = cJSON_GetObjectItemCaseSensitive(json_Request, "ID");
				json_Direccion_IP = cJSON_GetObjectItemCaseSensitive(json_Request, "REMOTE_ADDR");
				if(json_MAC)
				{
					/* Identifico las distintas placas que entran por este servicio */
					json_Tipo_HW = cJSON_GetObjectItemCaseSensitive(json_Request, "TYP");
					/* Si son placas viejas de Dom32-IO-WiFi no informan el TYP en la mensajería
						así que se la agrego para mantener compatibilidad 
						Dom32-IO-WiFi           - Typ: IO
						RBPi COn Server GPIO    - Typ: PI
						Dom32-Touch             - Typ: TOUCH
					*/
					if( !json_Tipo_HW )
					{
						cJSON_AddStringToObject(json_Request, "TYP", "IO");
						json_Tipo_HW = cJSON_GetObjectItemCaseSensitive(json_Request, "TYP");
					}

					if( !strcmp(json_Tipo_HW->valuestring, "IO") || !strcmp(json_Tipo_HW->valuestring, "PI")  )
					{
						rc = pEV->ExtIOEvent(message);
						if(rc > 0)
						{
							GroupTask();

							/* Me traigo los estados de las salidas del dispositivo
							para informar si hay cambios en la misma respuesta */
							json_Query_Result = cJSON_CreateArray();
							sprintf(query, "SELECT Objeto, A.Id AS ASS_Id, Port, A.Estado, A.Tipo AS Tipo_ASS "
											"FROM TB_DOM_PERIF AS P, TB_DOM_ASSIGN AS A "
											"WHERE A.Dispositivo = P.Id AND UPPER(P.MAC) = UPPER(\'%s\') "
												"AND ( A.Estado <> A.Estado_HW OR A.Actualizar > 0 ) "
												"AND ( A.Tipo = 0 OR A.Tipo = 3 OR A.Tipo = 5 );", json_MAC->valuestring);
							m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
							rc = pDB->Query(json_Query_Result, query);
							m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
							if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
							if(rc > 0)
							{
								m_pServer->m_pLog->Add(100, "Respondiendo con cambios de estado");
								/* Recorro el array */
								cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
								{
									/* Saco los datos que necesito */
									json_Id = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "ASS_Id");
									json_Port = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Port");
									json_Estado = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Estado");
									json_Objeto = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Objeto");
									json_Tipo_ASS = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Tipo_ASS");
									/* Armo la respuesta */
									cJSON_AddStringToObject(json_Response, json_Port->valuestring, json_Estado->valuestring);
									/* Borro el flag de update de los que ya aviso */
									if( !strcmp(json_Tipo_ASS->valuestring, "5"))
									{
										/* Salidas de pulso */
										sprintf(query, "UPDATE TB_DOM_ASSIGN "
															"SET Actualizar = 0, Estado_HW = 0, Estado = 0 "
															"WHERE Id = %s;", json_Id->valuestring);
										m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
										rc = pDB->Query(NULL, query);
										m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
										if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
									}
									else
									{
										/* Salidas que no son de pulso */
										sprintf(query, "UPDATE TB_DOM_ASSIGN "
															"SET Actualizar = 0, Estado_HW = Estado "
															"WHERE Id = %s;", json_Id->valuestring);
										m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
										rc = pDB->Query(NULL, query);
										m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
										if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);

										/* Notifico los cambios */
										m_pServer->m_pLog->Add(20, "Actualizar estado de Assign [%s] en la nube (Estado: %s)",
																json_Objeto->valuestring, json_Estado->valuestring);
										cJSON_PrintPreallocated(json_Query_Row, alt_message, GM_COMM_MSG_LEN, 0);
										m_pServer->m_pLog->Add(90, "Post [dompi_assign_change][%s]", alt_message);
										m_pServer->Post("dompi_assign_change", alt_message, strlen(alt_message));

									}
								}
							}
							else
							{
								/* Si no hay estados para responder me fijo si hay update de firmware pendiente */
								sprintf(query, "SELECT Update_firmware "
										"FROM TB_DOM_PERIF "
										"WHERE Update_firmware > 0 AND UPPER(MAC) = UPPER(\'%s\');", json_MAC->valuestring);
								m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
								rc = pDB->Query(nullptr, query);
								m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
								if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
								if(rc > 0)
								{
									cJSON_AddStringToObject(json_Response, "UPDATE-FW", "1");
									/* Borro el flag de update de firmware */
									sprintf(query, "UPDATE TB_DOM_PERIF "
										"SET Update_firmware = 0 "
										"WHERE UPPER(MAC) = UPPER(\'%s\');", json_MAC->valuestring);
									m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
									rc = pDB->Query(nullptr, query);
									m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
									if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
								}
								else
								{
									/* Si no hay nada para responder le mando la hora */
									sprintf(alt_message, "%04i/%02i/%02i %02i:%02i:%02i", 
										s_tm->tm_year+1900, s_tm->tm_mon+1, s_tm->tm_mday,
										s_tm->tm_hour, s_tm->tm_min, s_tm->tm_sec );
									cJSON_AddStringToObject(json_Response, "TIME", alt_message);
								}
							}
							cJSON_Delete(json_Query_Result);
							/* Si está todo bien me fijo si pidio enviar configuracion */
							json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Request, "GETCONF");
							if(json_un_obj)
							{
								if( atoi(json_un_obj->valuestring) > 0 )
								{
									m_pServer->m_pLog->Add(50, "[HW] %s Solicita configuracion", json_MAC->valuestring);
									sprintf(query, "UPDATE TB_DOM_PERIF "
														"SET Update_Config = 1 "
														"WHERE UPPER(MAC) = UPPER(\'%s\');", json_MAC->valuestring);
									m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
									rc = pDB->Query(NULL, query);
									m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
									if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
								}
							}
						}
						else if(rc < 0)
						{
							/* Otro Error */
							cJSON_AddStringToObject(json_Response, "resp_code", "1");
							cJSON_AddStringToObject(json_Response, "resp_msg", "General Error");
						}
						else
						{
							m_pServer->m_pLog->Add(10, "[HW] %s %s Desconocido", json_MAC->valuestring, (json_Direccion_IP)?json_Direccion_IP->valuestring:"-");
							CheckNewHWList(json_MAC->valuestring);
							/* NOT FOUND */
							cJSON_AddStringToObject(json_Response, "resp_code", "2");
							cJSON_AddStringToObject(json_Response, "resp_msg", "HW ID Not Found in Data Base");
						}
					}
					else if( !strcmp(json_Tipo_HW->valuestring, "TOUCH") )
					{
						/* Busco el ID para relacionar con la tabla de assigns */
						sprintf(query, "SELECT Id, Estado FROM TB_DOM_PERIF WHERE UPPER(MAC) = UPPER(\'%s\');", json_MAC->valuestring);
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						json_arr_Perif = cJSON_CreateArray();
						rc = pDB->Query(json_arr_Perif, query);
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						if(rc > 0)
						{
							/* Recorro el array */
							cJSON_ArrayForEach(json_Perif, json_arr_Perif)
							{
								/* Me fijo si tiene info de FW, HW, etc. */
								extra_info[0] = 0;
								json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Request, "HW");
								if(json_un_obj)
								{
									strcat(extra_info, "HW: ");
									strcat(extra_info, json_un_obj->valuestring);
									strcat(extra_info, "\n");
								}
								json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Request, "SO");
								if(json_un_obj)
								{
									strcat(extra_info, "SO: ");
									strcat(extra_info, json_un_obj->valuestring);
									strcat(extra_info, "\n");
								}
								json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Request, "FW");
								if(json_un_obj)
								{
									strcat(extra_info, "FW: ");
									strcat(extra_info, json_un_obj->valuestring);
									strcat(extra_info, "\n");
								}
								json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Request, "SDK");
								if(json_un_obj)
								{
									strcat(extra_info, "SDK: ");
									strcat(extra_info, json_un_obj->valuestring);
									strcat(extra_info, "\n");
								}
								json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Request, "AT");
								if(json_un_obj)
								{
									strcat(extra_info, "AT: ");
									strcat(extra_info, json_un_obj->valuestring);
									strcat(extra_info, "\n");
								}
								json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Request, "SSL");
								if(json_un_obj)
								{
									strcat(extra_info, "SSL: ");
									strcat(extra_info, json_un_obj->valuestring);
									strcat(extra_info, "\n");
								}
								/* Actualizo la tabla de Dispositivos */
								sprintf(query, "UPDATE TB_DOM_PERIF "
													"SET Ultimo_Ok = %lu, "
													"Direccion_IP = \'%s\', "
													"Informacion  = \'%s\', "
													"Estado = 1 "
													"WHERE UPPER(MAC) = UPPER(\'%s\');",
													entry_time,
													json_Direccion_IP->valuestring,
													extra_info,
													json_MAC->valuestring);
								m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
								rc = pDB->Query(NULL, query);
								m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
								if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);

								/* Me fijo si hay update de firmware pendiente */
								sprintf(query, "SELECT Update_firmware "
									"FROM TB_DOM_PERIF "
									"WHERE Update_firmware > 0 AND UPPER(MAC) = UPPER(\'%s\');", json_MAC->valuestring);
								m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
								rc = pDB->Query(nullptr, query);
								m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
								if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
								if(rc > 0)
								{
									json_Response = cJSON_CreateObject();
									cJSON_AddStringToObject(json_Response, "UPDATE-FW", "1");
									/* Borro el flag de update de firmware */
									sprintf(query, "UPDATE TB_DOM_PERIF "
										"SET Update_firmware = 0 "
										"WHERE UPPER(MAC) = UPPER(\'%s\');", json_MAC->valuestring);
									m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
									rc = pDB->Query(nullptr, query);
									m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
									if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
								}
								cJSON_AddStringToObject(json_Response, "resp_code", "0");
								cJSON_AddStringToObject(json_Response, "resp_msg", "Ok");
								/* Agrego la hora a la respuesta */
								sprintf(alt_message, "%04i/%02i/%02i %02i:%02i:%02i", 
									s_tm->tm_year+1900, s_tm->tm_mon+1, s_tm->tm_mday,
									s_tm->tm_hour, s_tm->tm_min, s_tm->tm_sec );
								cJSON_AddStringToObject(json_Response, "TIME", alt_message);
							}
						}
						else if(rc < 0)
						{
							/* Otro Error */
							cJSON_AddStringToObject(json_Response, "resp_code", "1");
							cJSON_AddStringToObject(json_Response, "resp_msg", "General Error");
						}
						else
						{
							m_pServer->m_pLog->Add(10, "[HW] %s %s Desconocido", json_MAC->valuestring, (json_Direccion_IP)?json_Direccion_IP->valuestring:"-");
							CheckNewHWList(json_MAC->valuestring);
							/* NOT FOUND */
							cJSON_AddStringToObject(json_Response, "resp_code", "2");
							cJSON_AddStringToObject(json_Response, "resp_msg", "HW ID Not Found in Data Base");
						}
					}
					else 
					{
						/* HW TYP Desconocido */
						cJSON_AddStringToObject(json_Response, "resp_code", "3");
						cJSON_AddStringToObject(json_Response, "resp_msg", "HW TYP Unknoun");
					}
				}
				else
				{
					/* El mensaje vino sin HWID */
					cJSON_AddStringToObject(json_Response, "resp_code", "4");
					cJSON_AddStringToObject(json_Response, "resp_msg", "HW ID Lost in message");
				}

				/* Armo la respuesta con lo que hay en el JSon */
				cJSON_PrintPreallocated(json_Response, message, GM_COMM_MSG_LEN, 0);
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
				cJSON_Delete(json_Request);
				cJSON_Delete(json_Response);

			}
			/* ****************************************************************
			*		dompi_hw_list_new - Listado de Harware recientemente descubierto
			**************************************************************** */
			else if( !strcmp(fn, "dompi_hw_list_new"))
			{
				message[0] = 0;

				for(i = 0; i < MAX_HW_LIST_NEW; i++)
				{
					if(g_dompi_server_new_hw_list[i].mac[0])
					{
						if(message[0])
						{
							/* agrego alamento */
							sprintf(&message[strlen(message)], ",{\"label\":\"%s\",\"value\":\"%s\"}",
								g_dompi_server_new_hw_list[i].mac, g_dompi_server_new_hw_list[i].mac);
						}
						else
						{
							/* Inicio del array */
							sprintf(message, "{\"response\":[{\"label\":\"%s\",\"value\":\"%s\"}",
								g_dompi_server_new_hw_list[i].mac, g_dompi_server_new_hw_list[i].mac);
						}
					}
				}
				/* */
				if(message[0])
				{
					/* Fin del array */
					strcat(message, "]}");
				}
				else
				{
					/* array vacío */
					strcat(message, "{\"response\":[]}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_user_list_new_card - Listado de Tarjetas no asignadas
			**************************************************************** */
			else if( !strcmp(fn, "dompi_user_list_new_card"))
			{
				message[0] = 0;

				for(i = 0; i < MAX_CARD_LIST_NEW; i++)
				{
					if(g_dompi_server_new_card_list[i].card[0])
					{
						if(message[0])
						{
							/* agrego alamento */
							sprintf(&message[strlen(message)], ",{\"label\":\"%s\",\"value\":\"%s\"}",
								g_dompi_server_new_card_list[i].card, g_dompi_server_new_card_list[i].card);
						}
						else
						{
							/* Inicio del array */
							sprintf(message, "{\"response\":[{\"label\":\"%s\",\"value\":\"%s\"}",
								g_dompi_server_new_card_list[i].card, g_dompi_server_new_card_list[i].card);
						}
					}
				}
				/* */
				if(message[0])
				{
					/* Fin del array */
					strcat(message, "]}");
				}
				else
				{
					/* array vacío */
					strcat(message, "{\"response\":[]}");
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
				json_Request = cJSON_Parse(message);
				message[0] = 0;
				json_Query_Result = cJSON_CreateArray();
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Request, "Id");
				json_Planta = cJSON_GetObjectItemCaseSensitive(json_Request, "Planta");
				if(json_Id)
				{
					sprintf(query, "SELECT Id,Objeto,Tipo,Port,Icono_Apagado,Icono_Encendido,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Estado,Perif_Data FROM TB_DOM_ASSIGN WHERE Id = %s;", json_Id->valuestring);
				}
				else
				{
					if(json_Planta)
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Port,Icono_Apagado,Icono_Encendido,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Estado,Perif_Data FROM TB_DOM_ASSIGN WHERE Planta = %s;", json_Planta->valuestring);
					}
					else
					{
						strcpy(query, "SELECT Id,Objeto,Tipo,Port,Icono_Apagado,Icono_Encendido,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Estado,Perif_Data FROM TB_DOM_ASSIGN;");
					}
				}
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					if(json_Id == nullptr)
					{
						if(json_Planta)
						{
							sprintf(query, "SELECT Id,Grupo,Icono_Apagado,Icono_Encendido,Estado FROM TB_DOM_GROUP WHERE Planta = %s;", json_Planta->valuestring);
						}
						else
						{
							strcpy(query, "SELECT Id,Grupo,Icono_Apagado,Icono_Encendido,Estado FROM TB_DOM_GROUP;");
						}
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						rc = pDB->Query(json_Query_Result, query);
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					}
					/* Queda en json_Query_Result los datos de los dos querys */
					if(json_Request) cJSON_Delete(json_Request);
					json_Request = cJSON_CreateObject();
					cJSON_AddItemToObject(json_Request, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_Request, message, GM_COMM_MSG_LEN, 0);
				}
				if(json_Request) cJSON_Delete(json_Request);
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
				json_Request = cJSON_Parse(message);
				message[0] = 0;
				json_Query_Result = cJSON_CreateArray();
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Request, "Id");
				json_Planta = cJSON_GetObjectItemCaseSensitive(json_Request, "Planta");
				if(json_Id)
				{
					sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y FROM TB_DOM_ASSIGN WHERE Id = %s;", json_Id->valuestring);
				}
				else
				{
					if(json_Planta)
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y FROM TB_DOM_ASSIGN WHERE Planta = %s;", json_Planta->valuestring);
					}
					else
					{
						strcpy(query, "SELECT Id,Objeto,Tipo,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y FROM TB_DOM_ASSIGN;");
					}
				}
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					if(json_Request) cJSON_Delete(json_Request);
					json_Request = cJSON_CreateObject();
					cJSON_AddItemToObject(json_Request, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_Request, message, GM_COMM_MSG_LEN, 0);
				}
				if(json_Request) cJSON_Delete(json_Request);
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
				json_Request = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Request, "Objeto");
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
				json_Request = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Request, "Objeto");
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
				json_Request = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Request, "Objeto");
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
				json_Request = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Request, "Objeto");
				if(json_un_obj)
				{
					json_Segundos = cJSON_GetObjectItemCaseSensitive(json_Request, "Segundos");
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
			*		dompi_auto_status
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_status"))				
			{
				json_Request = cJSON_Parse(message);
				message[0] = 0;
				json_Query_Result = cJSON_CreateArray();
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Request, "Id");
				json_Planta = cJSON_GetObjectItemCaseSensitive(json_Request, "Planta");
				if(json_Id)
				{
					sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono_Apagado,Icono_Encendido,Habilitado,Estado,Estado_Sensor,Estado_Salida "
									"FROM TB_DOM_AUTO "
									"WHERE Id = %s;", json_Id->valuestring);
				}
				else
				{
					if(json_Planta)
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono_Apagado,Icono_Encendido,Estado,Estado_Sensor,Estado_Salida "
										"FROM TB_DOM_AUTO "
										"WHERE Planta = %s;", json_Planta->valuestring);
					}
					else
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono_Apagado,Icono_Encendido,Estado,Estado_Sensor,Estado_Salida "
										"FROM TB_DOM_AUTO;");
					}
				}
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					if(json_Request) cJSON_Delete(json_Request);
					json_Request = cJSON_CreateObject();
					cJSON_AddItemToObject(json_Request, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_Request, message, GM_COMM_MSG_LEN, 0);
				}
				if(json_Request) cJSON_Delete(json_Request);
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
				json_Request = cJSON_Parse(message);
				message[0] = 0;
				json_Query_Result = cJSON_CreateArray();
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Request, "Id");
				json_Planta = cJSON_GetObjectItemCaseSensitive(json_Request, "Planta");
				if(json_Id)
				{
					sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono_Apagado,Icono_Encendido,Habilitado,Grupo_Visual,Planta,Cord_x,Cord_y "
									"FROM TB_DOM_AUTO WHERE Id = %s;", json_Id->valuestring);
				}
				else
				{
					if(json_Planta)
					{
						sprintf(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y "
										"FROM TB_DOM_AUTO WHERE Planta = %s;", json_Planta->valuestring);
					}
					else
					{
						strcpy(query, "SELECT Id,Objeto,Tipo,Icono_Disable,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y "
										"FROM TB_DOM_AUTO;");
					}
				}
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_Query_Result, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc >= 0)
				{
					if(json_Request) cJSON_Delete(json_Request);
					json_Request = cJSON_CreateObject();
					cJSON_AddItemToObject(json_Request, "response", json_Query_Result);
					cJSON_PrintPreallocated(json_Request, message, GM_COMM_MSG_LEN, 0);
				}
				if(json_Request) cJSON_Delete(json_Request);
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
				json_Request = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Request, "Objeto");
				if(json_un_obj)
				{
					/* Actualizo el estado en la base */
					sprintf(query, 	"UPDATE TB_DOM_AUTO "
									"SET Habilitado = 1 "
									"WHERE UPPER(Objeto) = UPPER(\'%s\');", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc > 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"Error en update a TB_DOM_AUTO\"}}");
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

				m_pServer->m_pLog->Add(90, "Notify [dompi_auto_change]");
				m_pServer->Notify("dompi_auto_change", nullptr, 0);
			}
			/* ****************************************************************
			*		dompi_auto_disable
			**************************************************************** */
			else if( !strcmp(fn, "dompi_auto_disable"))
			{
				json_Request = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Request, "Objeto");
				if(json_un_obj)
				{
					/* Actualizo el estado en la base */
					sprintf(query, 	"UPDATE TB_DOM_AUTO "
									"SET Habilitado = 0 "
									"WHERE UPPER(Objeto) = UPPER(\'%s\')", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc > 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Error en UPDATE a TB_DOM_AUTO\"}}");
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

				m_pServer->m_pLog->Add(90, "Notify [dompi_auto_change]");
				m_pServer->Notify("dompi_auto_change", nullptr, 0);
			}
			/* ****************************************************************
			*		dompi_alarm_part_info
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_part_info"))
			{

			}
			/* ****************************************************************
			*		dompi_alarm_part_status
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_part_status"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				json_Part = cJSON_GetObjectItemCaseSensitive(json_obj, "Part");

				if(json_Part)
				{
					pEV->Estado_Alarma(json_Part->valuestring, message, GM_COMM_MSG_LEN);
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Faltan Datos\"}}");					
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_alarm_part_on_total
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_part_on_total"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				json_Part = cJSON_GetObjectItemCaseSensitive(json_obj, "Part");

				if(json_Part)
				{
					if(pEV->Activar_Alarma(json_Part->valuestring, 1) == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Faltan Datos\"}}");					
				}

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
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				json_Part = cJSON_GetObjectItemCaseSensitive(json_obj, "Part");

				if(json_Part)
				{
					if(pEV->Activar_Alarma(json_Part->valuestring, 0) == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Faltan Datos\"}}");					
				}

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
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				json_Part = cJSON_GetObjectItemCaseSensitive(json_obj, "Part");

				if(json_Part)
				{
					if(pEV->Desactivar_Alarma(json_Part->valuestring) == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Faltan Datos\"}}");					
				}

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
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				json_Part = cJSON_GetObjectItemCaseSensitive(json_obj, "Part");

				if(json_Part)
				{


					strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Faltan Datos\"}}");					
				}

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
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				json_Part = cJSON_GetObjectItemCaseSensitive(json_obj, "Part");

				if(json_Part)
				{


					strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Faltan Datos\"}}");					
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_alarm_part_switch
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_part_switch"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				json_Part = cJSON_GetObjectItemCaseSensitive(json_obj, "Part");

				if(json_Part)
				{
					rc = pEV->Switch_Alarma(json_Part->valuestring);
					if(rc < 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
					}
					else if(rc == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Alarma desactivada\"}}");
					}
					else if(rc == 1)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Alarma activada parcial\"}}");
					}
					else if(rc == 2)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Alarma activada total\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Faltan Datos\"}}");					
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_alarm_zona_switch
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_zona_switch"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				json_Part = cJSON_GetObjectItemCaseSensitive(json_obj, "Part");
				json_Zona = cJSON_GetObjectItemCaseSensitive(json_obj, "Zona");

				if(json_Part && json_Zona)
				{
					if(pEV->Switch_Zona_Alarma(json_Part->valuestring, json_Zona->valuestring) >= 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Faltan Datos\"}}");					
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_alarm_salida_pulse
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_salida_pulse"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				json_Part = cJSON_GetObjectItemCaseSensitive(json_obj, "Part");
				json_Salida = cJSON_GetObjectItemCaseSensitive(json_obj, "Salida");

				if(json_Part && json_Salida)
				{
					if(pEV->Pulse_Salida_Alarma(json_Part->valuestring, json_Salida->valuestring) >= 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Faltan Datos\"}}");					
				}

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
				//m_pServer->Resp(NULL, 0, GME_OK);
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
							m_pServer->m_pLog->Add(20, "[CLOUD COMMAND] Objeto: %s - Accion: %s", 
								json_Objeto->valuestring, json_Accion->valuestring);

							if( !strcmp(json_Accion->valuestring, "on"))
							{
								if(pEV->ChangeAssignByName(json_Objeto->valuestring, 1, 0) <= 0)
								{
									/* Si no es un assign pruebo con un gupo */
									if(pEV->ChangeGroupByName(json_Objeto->valuestring, 1, 0) <= 0)
									{
										/* Si no es un grupo pruebo con un automatismo */
										pEV->ChangeAutoByName(json_Objeto->valuestring, 1, 0);
									}
								}
							}
							else if( !strcmp(json_Accion->valuestring, "off"))
							{
								if(pEV->ChangeAssignByName(json_Objeto->valuestring, 2, 0) <= 0)
								{
									/* Si no es un assign pruebo con un gupo */
									if(pEV->ChangeGroupByName(json_Objeto->valuestring, 2, 0) <= 0)
									{
										/* Si no es un grupo pruebo con un automatismo */
										pEV->ChangeAutoByName(json_Objeto->valuestring, 2, 0);
									}
								}
							}
							else if( !strcmp(json_Accion->valuestring, "switch"))
							{
								if(pEV->ChangeAssignByName(json_Objeto->valuestring, 3, 0) <= 0)
								{
									/* Si no es un assign pruebo con un gupo */
									if(pEV->ChangeGroupByName(json_Objeto->valuestring, 3, 0) <= 0)
									{
										/* Si no es un grupo pruebo con un automatismo */
										pEV->ChangeAutoByName(json_Objeto->valuestring, 3, 0);
									}
								}
							}
							else if( !strcmp(json_Accion->valuestring, "pulse"))
							{
								pEV->ChangeAssignByName(json_Objeto->valuestring, 4, 0);
							}
							else if( !strcmp(json_Accion->valuestring, "auto"))
							{
								pEV->ChangeAutoByName(json_Objeto->valuestring, 5, 0);
							}
							else if( !strcmp(json_Accion->valuestring, "parcial"))
							{
								pEV->Activar_Alarma(json_Objeto->valuestring, 0);
							}
							else if( !strcmp(json_Accion->valuestring, "total"))
							{
								pEV->Activar_Alarma(json_Objeto->valuestring, 1);
							}
							else if( !strcmp(json_Accion->valuestring, "desactivar"))
							{
								pEV->Desactivar_Alarma(json_Objeto->valuestring);
							}
							else if( !strcmp(json_Accion->valuestring, "habilitar"))
							{

							}
							else if( !strcmp(json_Accion->valuestring, "inhabilitar"))
							{
								
							}
						}
					}
				}
				cJSON_Delete(json_Query_Result);
			}
			/* ****************************************************************
			*		dompi_mobile_list_objects
			**************************************************************** */
			else if( !strcmp(fn, "dompi_mobile_list_objects"))
			{
				json_Request = cJSON_Parse(message);
				message[0] = 0;
				json_Grupo = cJSON_GetObjectItemCaseSensitive(json_Request, "Grupo");
				if(json_Grupo)
				{
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT Id,Objeto,Estado,Icono_Apagado,Icono_Encendido "
								   "FROM TB_DOM_ASSIGN "
					               "WHERE  Grupo_Visual = %s "
								   "ORDER BY Objeto ASC;",
								   json_Grupo->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc > 0)
					{
						cJSON_Delete(json_Request);
						json_Request = cJSON_CreateObject();
						cJSON_AddItemToObject(json_Request, "response", json_Query_Result);
						cJSON_PrintPreallocated(json_Request, message, GM_COMM_MSG_LEN, 0);
					}
				}
				cJSON_Delete(json_Request);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_mobile_touch_object
			**************************************************************** */
			else if( !strcmp(fn, "dompi_mobile_touch_object"))
			{
				json_Request = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_Request, "Objeto");
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
			exit_time = time(&exit_time);
			if( (exit_time - entry_time) > 1)
			{
				m_pServer->m_pLog->Add(10, "WARNING: %d segundos para procesar [%s]", (exit_time - entry_time), fn);
			}
		}
		else
		{
			/* Salida por time-out */
			if(wait_time < 100) wait_time++;
		}

		/* ********************************************************************
		 *   Actualizaciones de estados
		 *
		 *
		 * *******************************************************************/
		GroupTask();
		AssignTask();
		GroupMaint();
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
		CheckWiegandData();
		/*  */
		CheckNewHWList(NULL);
		/*  */
		CheckNewCardList(NULL);

		exit_time = time(&exit_time);
		if( (exit_time - entry_time) > 1)
		{
			m_pServer->m_pLog->Add(10, "WARNING: %d segundos de post procesamiento", (exit_time - entry_time));
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
	m_pServer->UnSuscribe("dompi_hw_list_new", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_user_list_new_card", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_status", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_info", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_on", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_off", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_switch", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_ass_pulse", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_status", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_info", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_enable", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_auto_disable", GM_MSG_TYPE_CR);

    m_pServer->UnSuscribe("dompi_alarm_part_info", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_part_status", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_part_on_total", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_part_on_parcial", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_part_off", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_part_switch", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_zona_enable", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_zona_disable", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_zona_switch", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_salida_pulse", GM_MSG_TYPE_CR);

	m_pServer->UnSuscribe("dompi_reload_config", GM_MSG_TYPE_MSG);
	m_pServer->UnSuscribe("dompi_cloud_notification", GM_MSG_TYPE_NOT);

	m_pServer->UnSuscribe("dompi_mobile_list_objects", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_mobile_touch_object", GM_MSG_TYPE_CR);

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

	m_pServer->m_pLog->Add(50, "Ejecutando [AssignTask]");

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
			m_pServer->m_pLog->Add(20, "Enviar estado de Assign [%s] Estado: %s al HW",
									json_Objeto->valuestring, json_Estado->valuestring);
			cJSON_PrintPreallocated(json_QueryRow, message, GM_COMM_MSG_LEN, 0);
			/* Me fijo si es estado o pulso */
			if( !strcmp(json_Tipo_ASS->valuestring, "5"))
			{
				/* Salidas de pulso */
				m_pServer->m_pLog->Add(90, "Notify [dompi_hw_pulse_io][%s]", message);
				m_pServer->Notify("dompi_hw_pulse_io", message, strlen(message));
				iEstado = 0;
			}
			else
			{
				/* Salidas que no son de pulso */
				m_pServer->m_pLog->Add(20, "Actualizar estado de Assign [%s] en la nube (Estado: %s)",
										json_Objeto->valuestring, json_Estado->valuestring);
				m_pServer->m_pLog->Add(90, "Notify [dompi_hw_set_io][%s]", message);
				m_pServer->Notify("dompi_hw_set_io", message, strlen(message));
				/* Notifico a la nube */
				m_pServer->m_pLog->Add(90, "Post [dompi_assign_change][%s]", message);
				m_pServer->Post("dompi_assign_change", message, strlen(message));
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

/* Actualiza el estado de los assign que pertenecen a un grupo que tiene cambios pendientes de actualizar */
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

	m_pServer->m_pLog->Add(50, "Ejecutando [GroupTask]");

	json_QueryArray = cJSON_CreateArray();
	strcpy(query, "SELECT * FROM TB_DOM_GROUP WHERE Actualizar = 1;");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(json_QueryArray, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc > 0)
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
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
		}
		m_pServer->m_pLog->Add(20, "Actualizar datos de grupos");
		m_pServer->Post("dompi_group_change", nullptr, 0);
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
	bool group_update;
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
		group_update = false;
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
				group_update = true;
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
				group_update = true;
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

		if(group_update)
		{
			m_pServer->m_pLog->Add(20, "Actualizar datos de grupos");
			m_pServer->Post("dompi_group_change", nullptr, 0);
		}
	}
	cJSON_Delete(json_QueryResult_Group);
}

void CheckNewHWList(const char* mac)
{
	int i;
	time_t t = time(&t);

	/* Busco para actualizar */
	for(i = 0; i < MAX_HW_LIST_NEW; i++)
	{
		if(mac)
		{
			if( !strcmp(mac, g_dompi_server_new_hw_list[i].mac) )
			{
				g_dompi_server_new_hw_list[i].last_info = t;
				break;
			}
		}
	}

	/* Si no existe la agrego */
	if(mac && i == MAX_HW_LIST_NEW)
	{
		for(i = 0; i < MAX_HW_LIST_NEW; i++)
		{
			if(g_dompi_server_new_hw_list[i].mac[0] == 0)
			{
				strcpy(g_dompi_server_new_hw_list[i].mac, mac);
				g_dompi_server_new_hw_list[i].last_info = t;
				break;
			}
		}

	}

	/* Busco viejas para dar de baja las que tengan mas de 2 min */
	for(i = 0; i < MAX_HW_LIST_NEW; i++)
	{
		if(g_dompi_server_new_hw_list[i].last_info && g_dompi_server_new_hw_list[i].last_info < (t-120))
		{
			g_dompi_server_new_hw_list[i].mac[0] = 0;
			g_dompi_server_new_hw_list[i].last_info = 0;
		}
	}
}

void CheckNewCardList(const char* card)
{
	int i;
	time_t t = time(&t);

	/* Busco para actualizar */
	for(i = 0; i < MAX_CARD_LIST_NEW; i++)
	{
		if(card)
		{
			if( !strcmp(card, g_dompi_server_new_card_list[i].card) )
			{
				g_dompi_server_new_card_list[i].last_info = t;
				break;
			}
		}
	}

	/* Si no existe la agrego */
	if(card && i == MAX_CARD_LIST_NEW)
	{
		for(i = 0; i < MAX_CARD_LIST_NEW; i++)
		{
			if(g_dompi_server_new_card_list[i].card[0] == 0)
			{
				strcpy(g_dompi_server_new_card_list[i].card, card);
				g_dompi_server_new_card_list[i].last_info = t;
				break;
			}
		}

	}

	/* Busco viejas para dar de baja las que tengan mas de 2 min */
	for(i = 0; i < MAX_CARD_LIST_NEW; i++)
	{
		if(g_dompi_server_new_card_list[i].last_info && g_dompi_server_new_card_list[i].last_info < (t-120))
		{
			g_dompi_server_new_card_list[i].card[0] = 0;
			g_dompi_server_new_card_list[i].last_info = 0;
		}
	}
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
	//cJSON *Permisos;
	cJSON *Dias_Semana;
	cJSON *Hora_Desde;
	cJSON *Minuto_Desde;
	cJSON *Hora_Hasta;
	cJSON *Minuto_Hasta;
	cJSON *Estado;

	int i_now, i_desde, i_hasta;

	m_pServer->m_pLog->Add(50, "[CheckWirelessCard] Tarjeta: %s", card);

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
	if(!strcmp(card, "NULL")) return 0;

	QueryResult = cJSON_CreateArray();
	sprintf(query, "SELECT Nombre_Completo, Dias_Semana, Hora_Desde, Minuto_Desde, Hora_Hasta, Minuto_Hasta, Estado "
					"FROM TB_DOM_USER "
					"WHERE UPPER(Tarjeta) = UPPER(\'%s\');", card);
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(QueryResult, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc > 0)
	{
		/* Recorro el array */
		cJSON_ArrayForEach(QueryRow, QueryResult)
		{
			Nombre_Completo = cJSON_GetObjectItemCaseSensitive(QueryRow, "Nombre_Completo");
			//Permisos = cJSON_GetObjectItemCaseSensitive(QueryRow, "Permisos");
			Dias_Semana = cJSON_GetObjectItemCaseSensitive(QueryRow, "Dias_Semana");
			Hora_Desde = cJSON_GetObjectItemCaseSensitive(QueryRow, "Hora_Desde");
			Minuto_Desde = cJSON_GetObjectItemCaseSensitive(QueryRow, "Minuto_Desde");
			Hora_Hasta = cJSON_GetObjectItemCaseSensitive(QueryRow, "Hora_Hasta");
			Minuto_Hasta = cJSON_GetObjectItemCaseSensitive(QueryRow, "Minuto_Hasta");
			Estado = cJSON_GetObjectItemCaseSensitive(QueryRow, "Estado");

			m_pServer->m_pLog->Add(50, "[CheckWirelessCard] Verificando Tarjeta: %s de Usuario: %s", card, Nombre_Completo->valuestring);

			/* Controlo día de la semana */
			if(strstr(Dias_Semana->valuestring, dia_semana))
			{
				i_desde = (atoi(Hora_Desde->valuestring) * 100) + atoi(Minuto_Desde->valuestring);
				i_hasta = (atoi(Hora_Hasta->valuestring) * 100) + atoi(Minuto_Hasta->valuestring);

				m_pServer->m_pLog->Add(50, "[CheckWirelessCard] Hora %i <= %i <= %i", i_desde, i_now, i_hasta);
				/* Controlo horario */
				if(atoi(Estado->valuestring) == 0)
				{
					m_pServer->m_pLog->Add(10, "[CheckWirelessCard] Usuario: %s INHABILITADO", Nombre_Completo->valuestring);
					auth = 0;
				}
				if(i_desde == i_hasta)
				{
					auth = 1;
				}
				else if( i_desde <= i_hasta && i_now >= i_desde && i_now <= i_hasta )
				{
					auth = 1;
				}
				else if( i_desde > i_hasta && ( i_now >= i_desde || i_now <= i_hasta ) )
				{
					auth = 1;
				}

				if(auth)
				{
					m_pServer->m_pLog->Add(10, "[CheckWirelessCard] Tarjeta: %s de Usuario: %s ACEPTADA", card, Nombre_Completo->valuestring);
				}
				else
				{
					m_pServer->m_pLog->Add(10, "[CheckWirelessCard] Tarjeta: %s de Usuario: %s DENEGADA", card, Nombre_Completo->valuestring);
				}
			}
			/* Verifico solo la primer ocurrencia */
			break;
		}
	}
	else if(rc == 0)
	{
		m_pServer->m_pLog->Add(10, "[CheckWirelessCard] Tarjeta: %s desconocida", card);
		CheckNewCardList(card);
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

	m_pServer->m_pLog->Add(50, "[CheckWiegandData]");

	QueryResult = cJSON_CreateArray();
	sprintf(query, "SELECT Id, Dispositivo, Perif_Data "
					"FROM TB_DOM_ASSIGN "
					"WHERE Port = \'CARD\' AND NOT ISNULL(Perif_Data);");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(QueryResult, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc > 0)
	{
		/* Recorro el array */
		cJSON_ArrayForEach(QueryRow, QueryResult)
		{
			Id = cJSON_GetObjectItemCaseSensitive(QueryRow, "Id");
			Dispositivo = cJSON_GetObjectItemCaseSensitive(QueryRow, "Dispositivo");
			Perif_Data = cJSON_GetObjectItemCaseSensitive(QueryRow, "Perif_Data");
			/* Verifico si el código el válido */
			rc = CheckWirelessCard( Perif_Data->valuestring );
			/* Ejecuto el Evento rc=0 Error - rc=1 Ok */
			pEV->CheckEvent(atoi(Dispositivo->valuestring), "CARD", rc);
			/* Borro el dato */
			sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
							"SET Perif_Data = NULL "
							"WHERE Id = %s;", Id->valuestring);
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(NULL, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
		}
	}
	cJSON_Delete(QueryResult);
}

void AddSaf( void )
{
	m_pServer->Notify(".create-queue", "dompi_infoio_synch", 19);	
	m_pServer->Notify(".create-queue", "dompi_changeio_synch", 21);	
}
