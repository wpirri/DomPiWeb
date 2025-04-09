
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

#define ACTIVE_UPDATE_INTERVAL 3600

const char *TB_DOM_CONFIG[] = {
	"Id",
	"Creacion",
	"System_Key",
	"Cloud_Host_1_Address",
	"Cloud_Host_1_Port",
	"Cloud_Host_1_Proto",
	"Cloud_Host_2_Address",
	"Cloud_Host_2_Port",
	"Cloud_Host_2_Proto",
	"Wifi_AP1",
	"Wifi_AP1_Pass",
	"Wifi_AP2",
	"Wifi_AP2_Pass",
	"Home_Host_1_Address",
	"Home_Host_2_Address",
	"Rqst_Path",
	"Wifi_Report",
	"Gprs_APN_Auto",
	"Gprs_APN",
	"Gprs_DNS1",
	"Gprs_DNS2",
	"Gprs_User",
	"Gprs_Pass",
	"Gprs_Auth",
	"Send_Method",
	"Planta1",
	"Planta2",
	"Planta3",
	"Planta4",
	"Planta5",
	"Flags",
	0};

const char *TB_DOM_USER[] = {
	"Id",
	"Usuario",
	"Nombre_Completo",
	"Pin_Teclado",
	"Pin_SMS",
	"Pin_WEB",
	"Telefono_Voz",
	"Telefono_SMS",
	"Usuario_Cloud",
	"Clave_Cloud",
	"Amazon_Key",
	"Google_Key",
	"Apple_Key",
	"Other_Key",
	"Tarjeta",
	"Acceso_Fisico",
	"Acceso_Web",
	"Acceso_Clowd",
	"Dias_Semana",
	"Hora_Desde",
	"Minuto_Desde",
	"Hora_Hasta",
	"Minuto_Hasta",
	"Estado",
	"Contador_Error",
	"Ultimo_Acceso",
	"Ultimo_Error",
	"Flags",
	0};

const char *TB_DOM_PERIF[] = {
	"Id",
	"MAC",
	"Dispositivo",
	"Tipo",
	/*"Estado",*/
	"Direccion_IP",
	/*"Ultimo_Ok",*/
	"Usar_Https",
	"Habilitar_Wiegand",
	"Update_Firmware",
	"Update_WiFi",
	"Update_Config",
	"Informacion",
	0};
	
const char *TB_DOM_ASSIGN[] = {
	"Id",
	"Objeto",
	"Dispositivo",
	"Port",
	"Tipo",
	"Estado",
	"Estado_HW",
	"Perif_Data",
	"Icono_Apagado",
	"Icono_Encendido",
	"Grupo_Visual",
	"Planta",
	"Cord_x",
	"Cord_y",
	"Coeficiente",
	"Analog_Mult_Div",
	"Analog_Mult_Div_Valor",
	"Actualizar",
	"Flags",
	0};
	
const char *TB_DOM_GROUP[] = {
	"Id",
	"Grupo",
	"Listado_Objetos",
	"Estado",
	"Icono_Apagado",
	"Icono_Encendido",
	"Grupo_Visual",
	"Planta",
	"Cord_x",
	"Cord_y",
	"Actualizar",
	0};

const char *TB_DOM_FLAG[] = {
	"Id",
	"Variable",
	"Valor",
	0};
	
const char *TB_DOM_ALARM_PARTICION[] = {
	"Id",
	"Nombre",
	"Entrada_Act_Total",
	"Entrada_Act_Parcial",
	"Testigo_Activacion",
	"Estado_Activacion",
	"Estado_Memoria",
	"Estado_Alarma",
	"Delay_Activacion",
	"Delay_Alarma",
	"Tiempo_De_Salida",
	"Tiempo_De_Entrada",
	"Tiempo_De_Alerta",
	"Notificar_SMS_Activacion",
	"Notificar_SMS_Alerta",
	0};
	
const char *TB_DOM_ALARM_ZONA[] = {
	"Id",
	"Particion",
	"Objeto_Zona",
	"Tipo_Zona",
	"Grupo",
	"Activa",
	0};
	
const char *TB_DOM_ALARM_SALIDA[] = {
	"Id",
	"Particion",
	"Objeto_Salida",
	"Tipo_Salida",
	0};
	
const char *TB_DOM_CAMARA[] = {
	"Id",
	"Nombre",
	"Direccion_IP",
	"Usuario",
	"Clave",
	"Protocolo",
	"Requerimiento",
	"Flags",
	0};
	
const char *TB_DOM_EVENT[] = {
	"Id",
	"Evento",
	"Objeto_Origen",
	"Objeto_Destino",
	"Grupo_Destino",
	"Particion_Destino",
	"Variable_Destino",
	"ON_a_OFF",
	"OFF_a_ON",
	"Enviar",
	"Parametro_Evento",
	"Condicion_Variable",
	"Condicion_Igualdad",
	"Condicion_Valor",
	"Filtro_Repeticion",
	"Ultimo_Evento",
	"Flags",
	0};
	
const char *TB_DOM_AT[] = {
	"Id",
	"Agenda",
	"Mes",
	"Dia",
	"Hora",
	"Minuto",
	"Dias_Semana",
	"Objeto_Destino",
	"Grupo_Destino",
	"Variable_Destino",
	"Evento",
	"Parametro_Evento",
	"Condicion_Variable",
	"Condicion_Igualdad",
	"Condicion_Valor",
	"Ultimo_Mes",
	"Ultimo_Dia",
	"Ultima_Hora",
	"Ultimo_Minuto",
	"Flags",
	0};
	
const char *TB_DOM_AUTO[] = {
	"Id",
	"Objeto",
	"Tipo",
	"Objeto_Sensor",
	"Objeto_Salida",
	"Grupo_Salida",
	"Particion_Salida",
	"Variable_Salida",
	"Parametro_Evento",
	"Min_Sensor",
	"Enviar_Min",
	"Max_Sensor",
	"Enviar_Max",
	"Hora_Inicio",
	"Minuto_Inicio",
	"Hora_Fin",
	"Minuto_Fin",
	"Dias_Semana",
	"Condicion_Variable",
	"Condicion_Igualdad",
	"Condicion_Valor",
	"Estado",
	"Habilitado",
	"Icono_Apagado",
	"Icono_Encendido",
	"Icono_Auto",
	"Grupo_Visual",
	"Planta",
	"Cord_x",
	"Cord_y",
	"Actualizar",
	"Flags",
	0};
	
const char *TB_DOM_TOUCH[] = {
	"Dispositivo",
	"Screen",
	"Line",
	"Button",
	"Evento",
	"Objeto",
	"X",
	"Y",
	"W",
	"H",
	"Redondo",
	"texto",
	"icono",
	"color_borde",
	"color_fondo",
	"color_texto",
	"orientacion",
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

int DBInsert(CDB* db, const char* tabla, const char** columnas, cJSON* jdata);
int DBUpdate(CDB* db, const char* tabla, const char *key, const char** columnas, cJSON* jdata);

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
	time_t t;
	time_t time_last_config;
	time_t time_last_user;
	time_t time_last_perif;
	time_t time_last_assign;
	time_t time_last_group;
	time_t time_last_flag;
	time_t time_last_particion;
	time_t time_last_zona;
	time_t time_last_salida;
	time_t time_last_camara;
	time_t time_last_event;
	time_t time_last_at;
	time_t time_last_auto;
	time_t time_last_touch;

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

	time_last_config = 0;
	time_last_user = 0;
	time_last_perif = 0;
	time_last_assign = 0;
	time_last_group = 0;
	time_last_flag = 0;
	time_last_particion = 0;
	time_last_zona = 0;
	time_last_salida = 0;
	time_last_camara = 0;
	time_last_event = 0;
	time_last_at = 0;
	time_last_auto = 0;
	time_last_touch = 0;

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 1000 )) >= 0)
	{
		t = time(&t);
		json_Id = nullptr;

		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(90, "%s:(Q)[%s]", fn, message);

			json_Message = cJSON_Parse(message);
			message[0] = 0;
			if(json_Message)
			{
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
			}
			
			/* ****************************************************************
			*		dompi_infoio - Notificacion de estado y/o cambio de I/O
			**************************************************************** */
			if( !strcmp(fn, "dompi_full_change"))
			{
				json_Message = nullptr;
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
				send_user = true;
			}
			else if( !strcmp(fn, "dompi_perif_change"))
			{
				send_perif = true;
			}
			else if( !strcmp(fn, "dompi_assign_change"))
			{
				if(json_Id == nullptr && json_Message)
				{
					json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "ASS_Id");
				}
				send_assign = true;
			}
			else if( !strcmp(fn, "dompi_group_change"))
			{
				if(json_Id == nullptr && json_Message)
				{
					json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "GRP_Id");
				}
				send_group = true;
			}
			else if( !strcmp(fn, "dompi_flag_change"))
			{
				send_flag = true;
			}
			else if( !strcmp(fn, "dompi_partition_change"))
			{
				send_partition = true;
			}
			else if( !strcmp(fn, "dompi_alarm_zona_change"))
			{
				send_alarm_zona = true;
			}
			else if( !strcmp(fn, "dompi_alarm_salida_change"))
			{
				send_alarm_salida = true;
			}
			else if( !strcmp(fn, "dompi_camara_change"))
			{
				send_camara = true;
			}
			else if( !strcmp(fn, "dompi_event_change"))
			{
				send_event = true;
			}
			else if( !strcmp(fn, "dompi_at_change"))
			{
				send_at = true;
			}
			else if( !strcmp(fn, "dompi_auto_change"))
			{
				send_auto = true;
			}
			else if( !strcmp(fn, "dompi_touch_change"))
			{
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

		/* Control de sincronización periódica */
		/*
		if(t > (time_last_config + ACTIVE_UPDATE_INTERVAL)) send_config = true;
		if(t > (time_last_user + ACTIVE_UPDATE_INTERVAL)) send_user = true;
		if(t > (time_last_perif + ACTIVE_UPDATE_INTERVAL)) send_perif = true;
		if(t > (time_last_assign + ACTIVE_UPDATE_INTERVAL)) send_assign = true;
		if(t > (time_last_group + ACTIVE_UPDATE_INTERVAL)) send_group = true;
		if(t > (time_last_flag + ACTIVE_UPDATE_INTERVAL)) send_flag = true;
		if(t > (time_last_particion + ACTIVE_UPDATE_INTERVAL)) send_partition = true;
		if(t > (time_last_zona + ACTIVE_UPDATE_INTERVAL)) send_alarm_zona = true;
		if(t > (time_last_salida + ACTIVE_UPDATE_INTERVAL)) send_alarm_salida = true;
		if(t > (time_last_camara + ACTIVE_UPDATE_INTERVAL)) send_camara = true;
		if(t > (time_last_event + ACTIVE_UPDATE_INTERVAL)) send_event = true;
		if(t > (time_last_at + ACTIVE_UPDATE_INTERVAL)) send_at = true;
		if(t > (time_last_auto + ACTIVE_UPDATE_INTERVAL)) send_auto = true;
		if(t > (time_last_touch + ACTIVE_UPDATE_INTERVAL)) send_touch = true;
		*/

		/* Encolo datos para el sistema remoto */
		if(send_config)
		{
			send_config = false;
			time_last_config = t;
			/* Configuración */
			json_Query_Result = cJSON_CreateArray();
			m_pServer->m_pLog->Add(10, "Generando sincronizacion de configuracion");
			strcpy(query, "SELECT * FROM TB_DOM_CONFIG ORDER BY Id DESC LIMIT 1;");
			m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
			rc = pDB->Query(json_Query_Result, query);
			m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
			if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
			if(rc > 0)
			{
				cJSON_PrintPreallocated(json_Query_Result, message, GM_COMM_MSG_LEN, 0);
				m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_config]", message);
				if(m_pServer->Enqueue("dompi_config_change", message, strlen(message)) != GME_OK)
				{
					m_pServer->m_pLog->Add(1, "ERROR: Encolando en SAF dompi_config [%s]", message);
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_user)
		{
			send_user = false;
			time_last_user = t;
			/* Usuarios */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de usuario [%s]", json_Id->valuestring);
				sprintf(query, "SELECT * FROM TB_DOM_USER WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de usuarios");
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
					if(m_pServer->Enqueue("dompi_user_change", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "ERROR: Encolando en SAF dompi_user [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_perif)
		{
			send_perif = false;
			time_last_perif = t;
			/* Perifericos */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de periferico [%s]", json_Id->valuestring);
				sprintf(query, "SELECT * FROM TB_DOM_PERIF WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de perifericos");
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
					if(m_pServer->Enqueue("dompi_perif_change", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "ERROR: Encolando en SAF dompi_perif [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_assign)
		{
			send_assign = false;
			time_last_assign = t;
			/* Objetos */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de objeto [%s]", json_Id->valuestring);
				sprintf(query, "SELECT * FROM TB_DOM_ASSIGN WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de objetos");
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
					m_pServer->m_pLog->Add(100, "[SYNCH] [%s] -> [dompi_assign_change]", message);
					if(m_pServer->Enqueue("dompi_assign_change", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "ERROR: Encolando en SAF dompi_assign_change [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_group)
		{
			send_group = false;
			time_last_group = t;
			/* Grupos */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de grupo [%s]", json_Id->valuestring);
				sprintf(query, "SELECT * FROM TB_DOM_GROUP WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de grupos");
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
					if(m_pServer->Enqueue("dompi_group_change", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "ERROR: Encolando en SAF dompi_group [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_flag)
		{
			send_flag = false;
			time_last_flag = t;
			/* Flags */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de flag [%s]", json_Id->valuestring);
				sprintf(query, "SELECT * FROM TB_DOM_FLAG WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de usuario flags");
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
					if(m_pServer->Enqueue("dompi_flag_change", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "ERROR: Encolando en SAF dompi_flag [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_partition)
		{
			send_partition = false;
			time_last_particion = t;
			/* Particiones */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de particion [%s]", json_Id->valuestring);
				sprintf(query, "SELECT * FROM TB_DOM_ALARM_PARTICION WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de particiones");
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
					if(m_pServer->Enqueue("dompi_particion_change", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "ERROR: Encolando en SAF dompi_particion [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_alarm_zona)
		{
			send_alarm_zona = false;
			time_last_zona = t;
			/* Zonas */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de zona [%s]", json_Id->valuestring);
				sprintf(query, "SELECT * FROM TB_DOM_ALARM_ZONA WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de usuario zonas");
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
					if(m_pServer->Enqueue("dompi_alarm_zona_change", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "ERROR: Encolando en SAF dompi_alarm_zona [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_alarm_salida)
		{
			send_alarm_salida = false;
			time_last_salida = t;
			/* Salidas de Alarma */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de salida [%s]", json_Id->valuestring);
				sprintf(query, "SELECT * FROM TB_DOM_ALARM_SALIDA WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de salidas");
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
					if(m_pServer->Enqueue("dompi_alarm_salida_change", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "ERROR: Encolando en SAF dompi_alarm_salida [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_camara)
		{
			send_camara = false;
			time_last_camara = t;
			/* Camaras */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de camara [%s]", json_Id->valuestring);
				sprintf(query, "SELECT * FROM TB_DOM_CAMARA WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de usuario camaras");
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
					if(m_pServer->Enqueue("dompi_camara_change", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "ERROR: Encolando en SAF dompi_camara [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_event)
		{
			send_event = false;
			time_last_event = t;
			/* Eventos */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de evento [%s]", json_Id->valuestring);
				sprintf(query, "SELECT * FROM TB_DOM_EVENT WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de eventos");
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
					if(m_pServer->Enqueue("dompi_event_change", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "ERROR: Encolando en SAF dompi_event [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_at)
		{
			send_at = false;
			time_last_at = t;
			/* AT */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de tarea programada [%s]", json_Id->valuestring);
				sprintf(query, "SELECT * FROM TB_DOM_AT WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de tareas programadas");
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
					if(m_pServer->Enqueue("dompi_at_change", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "ERROR: Encolando en SAF dompi_at [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_auto)
		{
			send_auto = false;
			time_last_auto = t;
			/* Auto */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de automatismo [%s]", json_Id->valuestring);
				sprintf(query, "SELECT * FROM TB_DOM_AUTO WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de automatosmos");
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
					if(m_pServer->Enqueue("dompi_auto_change", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "ERROR: Encolando en SAF dompi_auto [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}

		if(send_touch)
		{
			send_touch = false;
			time_last_touch = t;
			/* Touch */
			json_Query_Result = cJSON_CreateArray();
			if(json_Id)
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de panel tactil [%s]", json_Id->valuestring);
				sprintf(query, "SELECT * FROM TB_DOM_TOUCH WHERE Id = %s;", json_Id->valuestring);
			}
			else
			{
				m_pServer->m_pLog->Add(10, "Generando sincronizacion de paneles tactiles");
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
					if(m_pServer->Enqueue("dompi_touch_change", message, strlen(message)) != GME_OK)
					{
						m_pServer->m_pLog->Add(1, "ERROR: Encolando en SAF dompi_touch [%s]", message);
					}
				}
			}
			cJSON_Delete(json_Query_Result);
		}


		/* Traigo datos del sistema remoto */
		if(host_backup[0] != 0)
		{
			/* Configuracion */
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_config_change", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_config] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				m_pServer->m_pLog->Add(10, "Sincronizando configuracion");
				if( DBUpdate(pDB, "TB_DOM_CONFIG", "Id", TB_DOM_CONFIG, json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_CONFIG", TB_DOM_CONFIG, json_Message);
				}
			}

			while(GetSAFRemoto(host_backup, 0, "http", "dompi_user_change", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_user] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				m_pServer->m_pLog->Add(10, "Sincronizando usuario [%s]", json_Id->valuestring);
				if( DBUpdate(pDB, "TB_DOM_USER", "Id", TB_DOM_USER, json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_USER", TB_DOM_USER, json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_perif_change", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_perif] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				m_pServer->m_pLog->Add(10, "Sincronizando periferico [%s]", json_Id->valuestring);
				if( DBUpdate(pDB, "TB_DOM_PERIF", "Id", TB_DOM_PERIF, json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_PERIF", TB_DOM_PERIF, json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_assign_change", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_assign_change] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				m_pServer->m_pLog->Add(10, "Sincronizando objeto [%s]", json_Id->valuestring);
				if( DBUpdate(pDB, "TB_DOM_ASSIGN", "Id", TB_DOM_ASSIGN, json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_ASSIGN", TB_DOM_ASSIGN, json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_group_change", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_group] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				m_pServer->m_pLog->Add(10, "Sincronizando grupo [%s]", json_Id->valuestring);
				if( DBUpdate(pDB, "TB_DOM_GROUP", "Id", TB_DOM_GROUP, json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_GROUP", TB_DOM_GROUP, json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_flag_change", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_flag] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				m_pServer->m_pLog->Add(10, "Sincronizando flag [%s]", json_Id->valuestring);
				if( DBUpdate(pDB, "TB_DOM_FLAG", "Id", TB_DOM_FLAG, json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_FLAG", TB_DOM_FLAG, json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_particion_change", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_particion] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				m_pServer->m_pLog->Add(10, "Sincronizando particion [%s]", json_Id->valuestring);
				if( DBUpdate(pDB, "TB_DOM_ALARM_PARTICION", "Id", TB_DOM_ALARM_PARTICION, json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_ALARM_PARTICION", TB_DOM_ALARM_PARTICION, json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_alarm_zona_change", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_alarm_zona] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				m_pServer->m_pLog->Add(10, "Sincronizando zona [%s]", json_Id->valuestring);
				if( DBUpdate(pDB, "TB_DOM_ALARM_ZONA", "Id", TB_DOM_ALARM_ZONA, json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_ALARM_ZONA", TB_DOM_ALARM_ZONA, json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_alarm_salida_change", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_alarm_salida] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				m_pServer->m_pLog->Add(10, "Sincronizando salida [%s]", json_Id->valuestring);
				if( DBUpdate(pDB, "TB_DOM_ALARM_SALIDA", "Id", TB_DOM_ALARM_SALIDA, json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_ALARM_SALIDA", TB_DOM_ALARM_SALIDA, json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_camara_change", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_camara] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				m_pServer->m_pLog->Add(10, "Sincronizando camara [%s]", json_Id->valuestring);
				if( DBUpdate(pDB, "TB_DOM_CAMARA", "Id", TB_DOM_CAMARA, json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_CAMARA", TB_DOM_CAMARA, json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_event_change", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_event] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				m_pServer->m_pLog->Add(10, "Sincronizando evento [%s]", json_Id->valuestring);
				if( DBUpdate(pDB, "TB_DOM_EVENT", "Id", TB_DOM_EVENT, json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_EVENT", TB_DOM_EVENT, json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_at_change", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_at] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				m_pServer->m_pLog->Add(10, "Sincronizando tarea programada [%s]", json_Id->valuestring);
				if( DBUpdate(pDB, "TB_DOM_AT", "Id", TB_DOM_AT, json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_AT", TB_DOM_AT, json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_auto_change", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_auto] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				m_pServer->m_pLog->Add(10, "Sincronizando automatismo [%s]", json_Id->valuestring);
				if( DBUpdate(pDB, "TB_DOM_AUTO", "Id", TB_DOM_AUTO, json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_AUTO", TB_DOM_AUTO, json_Message);
				}
			}
			
			while(GetSAFRemoto(host_backup, 0, "http", "dompi_touch_change", message, GM_COMM_MSG_LEN) > 0)
			{
				m_pServer->m_pLog->Add(100, "[SYNCH] [dompi_touch] -> [%s]", message);
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				if(!json_Message) break;
				json_Id = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				if(!json_Id) break;
				m_pServer->m_pLog->Add(10, "Sincronizando panel tactil [%s]", json_Id->valuestring);
				if( DBUpdate(pDB, "TB_DOM_TOUCH", "Id", TB_DOM_TOUCH, json_Message) <= 0)
				{
					DBInsert(pDB, "TB_DOM_TOUCH", TB_DOM_TOUCH, json_Message);
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
	m_pServer->Notify(".create-queue", "dompi_config_change", 20);	
	m_pServer->Notify(".create-queue", "dompi_user_change", 18);	
	m_pServer->Notify(".create-queue", "dompi_perif_change", 19);	
	m_pServer->Notify(".create-queue", "dompi_assign_change", 20);	
	m_pServer->Notify(".create-queue", "dompi_group_change", 19);	
	m_pServer->Notify(".create-queue", "dompi_flag_change", 18);	
	m_pServer->Notify(".create-queue", "dompi_particion_change", 23);	
	m_pServer->Notify(".create-queue", "dompi_alarm_zona_change", 24);	
	m_pServer->Notify(".create-queue", "dompi_alarm_salida_change", 26);	
	m_pServer->Notify(".create-queue", "dompi_camara_change", 20);	
	m_pServer->Notify(".create-queue", "dompi_event_change", 19);	
	m_pServer->Notify(".create-queue", "dompi_at_change", 16);	
	m_pServer->Notify(".create-queue", "dompi_auto_change", 18);	
	m_pServer->Notify(".create-queue", "dompi_touch_change", 19);	
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

int DBInsert(CDB* db, const char* tabla, const char** columnas, cJSON* jdata)
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
					if(ExisteColumna(j->string, columnas))
					{
						if(strlen(j->string) /*&& strlen(j->valuestring)*/)
						{
							if(strcmp(j->valuestring, "NULL"))
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
	return rc;
}

int DBUpdate(CDB* db, const char* tabla, const char *key, const char** columnas, cJSON* jdata)
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
						if(ExisteColumna(j->string, columnas) && strcmp(j->valuestring, "NULL"))
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
		return rc;
	}
	return (-1);
}