
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
char www_root[FILENAME_MAX+1];
CDB *pDB;
GEvent *pEV;
cJSON *json_System_Config;
int i;

time_t last_daily;

#define MAX_HW_LIST_NEW 128
#define MAX_CARD_LIST_NEW 128

void OnClose(int sig);
void LoadSystemConfig(void);
void BuildTouchConfig( void );

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
					"  download <dispositivo>, <archivo>\r\n"
					"  manten\r\n"
					"  sms <numero>, <mensaje>\r\n"
					"  habilitar <zona>, <particion>\r\n"
					"  deshabilitar <zona>, <particion>\r\n"
					"  activar alarma, <particion>\r\n"
					"  desactivar alarma, <particion>\r\n"
					"  estado alarma, <particion>\r\n"
					"  configurar <dispositivo>, <parametro=valor>\r\n"
					"  help\r\n"
					"  * tipo: dispositivos, objetos, grupos, eventos.\r\n"
					"    objeto: Nombre de un objeto existente.\r\n"
					"    dispositivo: Nombre de un dispositivo existente.\r\n"
					"    modulo: wifi, config, firmware.\r\n"
					"    segundos: duracion en segundos. Si no se especifica el default es 1.\r\n"
					"    numero: Numero de telefono destino del mensaje.\r\n"
					"    mensaje: Mensaje a enviar.\r\n"
					"    particion: Nombre de la particion.\r\n"
					"    archivo: Debe estar en el directorio download de la central.\r\n"
                    "    parametros:\r\n"
					"        ap1: Access point WiFi 1.\r\n"
					"        ap1p: Clave de Access point WiFi 1.\r\n"
					"        ap2: Access point WiFi 2.\r\n"
					"        ap2p: Clave de Access point WiFi 2.\r\n"
					"        ce1: Central de domotica 1.\r\n"
					"        ce2: Central de domotica 2.\r\n"
					"        path: Raiz para los requerimientos\r\n"
					"        ce1p: Port de la Central de domotica 1.\r\n"
					"        ce2p: Port de la Central de domotica 2.\r\n"
					"        report: Inetrvalo de actualización de estado.\r\n"
					"        brillo-max: Brillo maximo del display.\r\n"
					"        brillo-med: Brillo medio del display.\r\n"
					"        brillo-min: Brillo minimo del display.\r\n"
					"        screen-saver: Tiempo de inactividad para apagado (0 = desactivado).\r\n"
					"        debug: 0 = no, 1 = si (necesita restart).\r\n"
					"        display: 0 = MCU 1 = SPI (necesita restart).\r\n"
                    "\r\n"
					"-------------------------------------------------------------------------------\r\n"
                    "\r\n";

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[GM_COMM_MSG_LEN+1];
	char cmdline[1024];
	char db_host[32];
	char db_name[32];
	char db_user[32];
	char db_password[32];
	char query[4096];
	char listado[4096];
	unsigned long message_len;
	//time_t t;
	char s[16];
	STRFunc sf;

	char comando[1024];
	char objeto[1024];
	char parametro[1024];

	STRFunc Strf;
	//CGMServerBase::GMIOS call_resp;

    cJSON *json_Request;
    cJSON *json_un_obj;
    cJSON *json_Query_Result = NULL;
	cJSON *json_Query_Row;
    cJSON *json_query;
    cJSON *json_cmdline;

    cJSON *json_HW_Id;
	cJSON *json_Objeto;
	cJSON *json_Tipo;
	cJSON *json_Estado;
	cJSON *json_Id;
	cJSON *json_Dispositivo;
	cJSON *json_MAC;
	cJSON *json_Direccion_IP;
	cJSON *json_Command;
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
	m_pServer->Init("dompi_command");
	m_pServer->m_pLog->Add(1, "Iniciando Servidor de Linea de comandos...");

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

	strcpy(www_root, "/var/www/html");
	pConfig->GetParam("WWW-ROOT", www_root);

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
	m_pServer->Suscribe("dompi_cmdline", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_reload_config", GM_MSG_TYPE_MSG);

	m_pServer->m_pLog->Add(1, "Servicios DOMCLI inicializados.");

	//t = time(&t);

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, (-1) )) >= 0)
	{
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(90, "%s:(Q)[%s]", fn, message);
			/* ****************************************************************
			*		dompi_cmdline - input de domcli
			**************************************************************** */
			if( !strcmp(fn, "dompi_cmdline"))
			{
				json_Request = cJSON_Parse(message);
				message[0] = 0;
				/* *********************************************************** */
				json_query = cJSON_GetObjectItemCaseSensitive(json_Request, "query");
				if(json_query)
				{
					json_cmdline = cJSON_GetObjectItemCaseSensitive(json_query, "CmdLine");
					if(json_cmdline)
					{
						strcpy(cmdline, json_cmdline->valuestring);

						Strf.ParseCommand(cmdline, comando, objeto, parametro);

						m_pServer->m_pLog->Add(80, "[dompi_cmdline] Comando: %s - Objeto: %s - Parametro: %s", 
											(comando[0])?comando:"NULL", 
											(objeto[0])?objeto:"NULL", 
											(parametro[0])?parametro:"NULL");

						if( !strcmp(comando, "help") || !strcmp(comando, "?"))
						{
							sprintf(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"%s\"}}", cli_help);
						}
						else if( !strcmp(comando, "listar") || !strcmp(comando, "list"))
						{
							listado[0] = 0;
							if( !memcmp(objeto, "dis", 3))
							{
								sprintf(query, "SELECT Id, Dispositivo, MAC, Direccion_IP, Estado "
												"FROM TB_DOM_PERIF "
												"ORDER BY Dispositivo ASC;");
								m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
								json_Query_Result = cJSON_CreateArray();
								rc = pDB->Query(json_Query_Result, query);
								m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
								if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
								if(rc >= 0)
								{	/*                         11111111112222222222333333333344444444445555555555 */
									/*               012345678901234567890123456789012345678901234567890123456789  */
									/*WiFi-01-4a61af                 c8c9a34a61af 192.168.10.174  **/
					                strcpy(listado, "> Nombre                       MAC          IP          En Linea\n"); 
									/* Obtengo el primero del array del resultado del query */
									cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
									{
										if(json_Query_Row)
										{
											json_Id = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Id");
											json_Dispositivo = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Dispositivo");
											json_MAC = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "MAC");
											json_Direccion_IP = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Direccion_IP");
											json_Estado = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Estado");
											if(json_Dispositivo && json_MAC && json_Direccion_IP && json_Estado)
											{
												if(atoi(json_Id->valuestring))
												{
													sprintf(&listado[strlen(listado)], "%-30.30s %12.12s %-15.15s %c\n", 
														json_Dispositivo->valuestring, json_MAC->valuestring,
														json_Direccion_IP->valuestring, (atoi(json_Estado->valuestring)?'*':' '));
												}
											}
										}
									}
								}
								cJSON_Delete(json_Query_Result);
							}
							else if( !memcmp(objeto, "obj", 3))
							{
								listado[0] = 0;
								sprintf(query, "SELECT Id, Objeto, Tipo, Estado "
												"FROM TB_DOM_ASSIGN "
												"ORDER BY Objeto ASC;");
								m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
								json_Query_Result = cJSON_CreateArray();
								rc = pDB->Query(json_Query_Result, query);
								m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
								if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
								if(rc >= 0)
								{	/*                         11111111112222222222333333333344444444445555555555 */
									/*               012345678901234567890123456789012345678901234567890123456789  */
									strcpy(listado, "> Nombre                                  Tipo  Estado\n"); 
									/* Obtengo el primero del array del resultado del query */
									cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
									{
										if(json_Query_Row)
										{
											json_Id = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Id");
											json_Objeto = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Objeto");
											json_Tipo = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Tipo");
											json_Estado = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Estado");
											if(json_Objeto && json_Tipo && json_Estado)
											{
												if(atoi(json_Id->valuestring))
												{
													sprintf(&listado[strlen(listado)], "%-40.40s %3s  %5s\n", 
														json_Objeto->valuestring, json_Tipo->valuestring, json_Estado->valuestring);
												}
											}
												
										}
									}
								}
								cJSON_Delete(json_Query_Result);
							}
							else if( !memcmp(objeto, "gru", 3))
							{
								/* TODO: Completar comando listar grupo */
								strcat(listado, "WARNING: Objeto GRUPO sin programar");
							}
							else if( !memcmp(objeto, "eve", 3))
							{
								/* TODO: Completar comando listar eventos */
								strcat(listado, "WARNING: Objeto EVENTO sin programar");
							}
							else
							{
								sprintf(listado, "ERROR: Objeto [%s] desconocido", objeto);
							}
							sprintf(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"%s\"}}", listado);
						}
						else if( !strcmp(comando, "manten") )
						{
							/* TODO: Hacer algún mantenimiento si es necesario */
							BuildTouchConfig();
							sprintf(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"BuildTouchConfig ejecutado\"}}");
						}
						else if( !strcmp(comando, "sms") )
						{
							if(objeto[0] && parametro[0])
							{
								json_un_obj = cJSON_CreateObject();
								cJSON_AddStringToObject(json_un_obj, "SmsTo", (objeto[0])?objeto:"98765432");
								cJSON_AddStringToObject(json_un_obj, "SmsTxt", (parametro[0])?parametro:"test");
								cJSON_PrintPreallocated(json_un_obj, message, GM_COMM_MSG_LEN, 0);
								m_pServer->m_pLog->Add(90, "Enqueue [dompi_sms_output][%s]", message);
								rc = m_pServer->Enqueue("dompi_sms_output", message, strlen(message));
								if(rc != 0)
								{
									sprintf(message, "{\"response\":{\"resp_code\":\"%i\", \"resp_msg\":\"Error en envio de SMS\"}}", rc);
								}
								cJSON_Delete(json_un_obj);
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Falta un dato\"}}");
							}
						}
						/* TODO: Completar varios comandos sobre objetos */
						else if( !strcmp(comando, "encender") || !strcmp(comando, "enc"))
						{
							if(objeto[0])
							{
								pEV->ChangeAssignByName(objeto, 1, 0);
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Falta un dato\"}}");
							}
						}
						else if( !strcmp(comando, "apagar") || !strcmp(comando, "apa"))
						{
							if(objeto[0])
							{
								pEV->ChangeAssignByName(objeto, 2, 0);
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Falta un dato\"}}");
							}
						}
						else if( !strcmp(comando, "cambiar") || !strcmp(comando, "switch") || !strcmp(comando, "sw") )
						{
							if(objeto[0])
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
							if(objeto[0])
							{
								pEV->ChangeAssignByName(objeto, 4, 0);
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Falta un dato\"}}");
							}
						}
						else if( !strcmp(comando, "download"))
						{
							/*
								Indica a un dispositivo que se debe bajar un archivo
								objeto: nombre de un periferico
								parametro: nombre de archivo 
							 */
							json_arr_Perif = cJSON_CreateArray();
							sprintf(query, "SELECT * "
											"FROM TB_DOM_PERIF "
											"WHERE Dispositivo = \'%s\'; ", objeto);
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
									json_Tipo = cJSON_GetObjectItemCaseSensitive(json_Perif, "Tipo");
									json_HW_Id = cJSON_GetObjectItemCaseSensitive(json_Perif, "Id");
									json_Direccion_IP = cJSON_GetObjectItemCaseSensitive(json_Perif, "Direccion_IP");	

									if(json_MAC && json_Tipo && json_HW_Id && json_Direccion_IP)
									{
										/* Un objeto para contener a todos */
										json_Command = cJSON_CreateObject();
										/* Saco los datos que necesito */
										cJSON_AddStringToObject(json_Command, "Id", json_HW_Id->valuestring);
										cJSON_AddStringToObject(json_Command, "MAC", json_MAC->valuestring);
										cJSON_AddStringToObject(json_Command, "Direccion_IP", json_Direccion_IP->valuestring);
										cJSON_AddStringToObject(json_Command, "Tipo_HW", json_Tipo->valuestring);
										cJSON_AddStringToObject(json_Command, "Command", "download");
										cJSON_AddStringToObject(json_Command, "Filename", parametro);

										if( atoi(json_Tipo->valuestring) == TIPO_HW_WIFI ||
											atoi(json_Tipo->valuestring) == TIPO_HW_TOUCH ||
											atoi(json_Tipo->valuestring) == TIPO_HW_RBPI)
										{
											cJSON_PrintPreallocated(json_Command, message, GM_COMM_MSG_LEN, 0);
											m_pServer->m_pLog->Add(90, "Notify [dompi_hw_send_command][%s]", message);
											m_pServer->Notify("dompi_hw_send_command", message, strlen(message));
										}
										cJSON_Delete(json_Command);
									}
								}
							}
						}
						else if( !memcmp(comando, "conf", 4))
						{
							/*
								Seteo un parametro de configuración
								objeto: nombre de un periferico
								parametro: parametro=valor 
							 */
							json_arr_Perif = cJSON_CreateArray();
							sprintf(query, "SELECT * "
											"FROM TB_DOM_PERIF "
											"WHERE Dispositivo = \'%s\'; ", objeto);
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
									json_Tipo = cJSON_GetObjectItemCaseSensitive(json_Perif, "Tipo");
									json_HW_Id = cJSON_GetObjectItemCaseSensitive(json_Perif, "Id");
									json_Direccion_IP = cJSON_GetObjectItemCaseSensitive(json_Perif, "Direccion_IP");	

									if(json_MAC && json_Tipo && json_HW_Id && json_Direccion_IP)
									{
										/* Un objeto para contener a todos */
										json_Command = cJSON_CreateObject();
										/* Saco los datos que necesito */
										cJSON_AddStringToObject(json_Command, "Id", json_HW_Id->valuestring);
										cJSON_AddStringToObject(json_Command, "MAC", json_MAC->valuestring);
										cJSON_AddStringToObject(json_Command, "Direccion_IP", json_Direccion_IP->valuestring);
										cJSON_AddStringToObject(json_Command, "Tipo_HW", json_Tipo->valuestring);
										cJSON_AddStringToObject(json_Command, "Command", "config");
										cJSON_AddStringToObject(json_Command, "Parametro", parametro);

										if( atoi(json_Tipo->valuestring) == TIPO_HW_WIFI ||
											atoi(json_Tipo->valuestring) == TIPO_HW_TOUCH ||
											atoi(json_Tipo->valuestring) == TIPO_HW_RBPI)
										{
											cJSON_PrintPreallocated(json_Command, message, GM_COMM_MSG_LEN, 0);
											m_pServer->m_pLog->Add(90, "Notify [dompi_hw_send_config][%s]", message);
											m_pServer->Notify("dompi_hw_send_config", message, strlen(message));
										}
										cJSON_Delete(json_Command);
									}
								}
							}
						}
						else if( !strcmp(comando, "actualizar"))
						{
							/* Saco los datos que necesito */
							if( !memcmp(parametro, "wifi", 4))
							{
								m_pServer->m_pLog->Add(50, "Actualizar WiFi en: %s", objeto);
								sprintf(query, "UPDATE TB_DOM_PERIF "
												"SET Update_WiFi = 1 "
												"WHERE Dispositivo = \'%s\';", objeto);
								m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
								rc = pDB->Query(NULL, query);
								m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
								if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
								if(rc > 0)
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
								}
								else if(rc == 0)
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"HW Not Found in Data Base\"}}");
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
								}
							}
							else if( !memcmp(parametro, "conf", 4))
							{
								m_pServer->m_pLog->Add(50, "Actualizar configuracion de: %s", objeto);
								sprintf(query, "UPDATE TB_DOM_PERIF "
												"SET Update_Config = 1 "
												"WHERE Dispositivo = \'%s\';", objeto);
								m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
								rc = pDB->Query(NULL, query);
								m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
								if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
								if(rc > 0)
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
								}
								else if(rc == 0)
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"HW Not Found in Data Base\"}}");
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
								}
							}
							else if( !memcmp(parametro, "firm", 4))
							{
								m_pServer->m_pLog->Add(50, "Actualizar Firmware de: %s", objeto);
								sprintf(query, "UPDATE TB_DOM_PERIF "
												"SET Update_Firmware = 1 "
												"WHERE Dispositivo = \'%s\';", objeto);
								m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
								rc = pDB->Query(NULL, query);
								m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
								if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
								if(rc > 0)
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
								}
								else if(rc == 0)
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"HW Not Found in Data Base\"}}");
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
								}
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
							}
						}
						else if( !strcmp(comando, "habilitar") || !strcmp(comando, "hab"))
						{
							/*                    Zona       Partición */
							if(pEV->Habilitar_Alarma(objeto, parametro) == 0)
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
							}
						}
						else if( !strcmp(comando, "deshabilitar"))
						{
							/*                       Zona       Partición */
							if(pEV->Deshabilitar_Alarma(objeto, parametro) == 0)
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
							}
						}
						else if( !strcmp(comando, "activar"))
						{
							if( !strcmp(objeto, "alarma"))
							{
								if(pEV->Activar_Alarma(parametro, 1) == 0)
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
								}
							}
						}
						else if( !strcmp(comando, "desactivar") )
						{
							if( !strcmp(objeto, "alarma"))
							{
								if(pEV->Desactivar_Alarma(parametro) == 0)
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
								}
							}
						}
						else if( !strcmp(comando, "estado") )
						{
							if( !strcmp(objeto, "alarma"))
							{
								pEV->Estado_Alarma(parametro, message, GM_COMM_MSG_LEN);
							}
						}
					}
				}
				/* *********************************************************** */
				cJSON_Delete(json_Request);
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

		}
	}
	m_pServer->m_pLog->Add(1, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);

	m_pServer->UnSuscribe("dompi_cmdline", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_reload_config", GM_MSG_TYPE_MSG);

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


	/*
		# FONDO,[color]
		# BOTON_CUADRADO,[etiqueta],[comando:objeto],[x],[y],[w],[h],[color fondo],[color borde],[color etiqueta],[icono],[orientacion]
		# BOTON_REDONDO,[etiqueta],[comando:objeto],[x],[y],[w],[h],[color fondo],[color borde],[color etiqueta],[icono],[orientacion]
		#
		# etiqueta: Texto que aparece en el botón (opcopnal)
		# comando: SWITCH, PULSE, CONFIG, HOME, NEXT, PREV
		# objeto: 
		# x: posición X de la esquina superior derecha del botón
		# y: posición Y de la esquina superior derecha del botón
		# w: Ancho del botón
		# h: altura del botón
		# color fondo: 0 a 65535
		# color borde: 0 a 65535
		# color etiqueta: 0 a 65535
		# icono: archivo BMP de 48x48 pixel (opcopnal)
		# orientacion: 0 = Horizontal, 1 = Vertical
		# 
		FONDO,0
		BOTON_REDONDO,,SWITCH:Luz Comedor,5,5,230,250,0,768,58624,power.bmp,0
		# -----------------------------------------------------------------------------
		#BOTON_CUADRADO,,PREV,20,260,60,60,0,0,0,prev.bmp,0
		BOTON_CUADRADO,,CONFIG,20,260,60,60,0,0,0,gear.bmp,0
		#BOTON_CUADRADO,,HOME,90,260,60,60,0,0,0,home.bmp,0
		BOTON_CUADRADO,,NEXT,160,260,60,60,0,0,0,next.bmp,0
	*/
void BuildTouchConfig( void )
{
	int rc;
	char query[4096];
	int disp, pant, last_disp = (-1), last_pant = (-1);
	FILE* f = nullptr;
	FILE* f_list = nullptr;
	char display_path[FILENAME_MAX+1];
	char screen_name[FILENAME_MAX+1];
	char file_name[FILENAME_MAX+1];
	char cmd[FILENAME_MAX+1];
	char evento[256];
	char texto[256];
	char objeto[256];
	char icono[256];


	cJSON* jsQueryRes;
	cJSON* jsQueryRow;

	cJSON* jsMAC;
	cJSON* jsDestino;
	cJSON* jsDispositivo;
	cJSON* jsPantalla;
//	cJSON* jsBoton;
	cJSON* jsEvento;
	cJSON* jsObjeto;
	cJSON* jsX;
	cJSON* jsY;
	cJSON* jsW;
	cJSON* jsH;
	cJSON* jsRedondo;
	cJSON* jsTexto;
	cJSON* jsIcono;
	cJSON* jsColor_pantalla;
	cJSON* jsColor_borde;
	cJSON* jsColor_fondo;
	cJSON* jsColor_texto;
	cJSON* jsOrientacion;

	m_pServer->m_pLog->Add(20, "[BuildTouchConfig] Inicio");

	jsQueryRes = cJSON_CreateArray();
	sprintf(query, "SELECT T.*, P.MAC, A.Objeto AS Destino "
					"FROM TB_DOM_TOUCH AS T, TB_DOM_PERIF AS P, TB_DOM_ASSIGN AS A "
					"WHERE T.Dispositivo > 0 AND T.Dispositivo = P.Id AND T.Objeto = A.Id;");
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = pDB->Query(jsQueryRes, query);
	m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
	if(rc >= 0)
	{
		cJSON_ArrayForEach(jsQueryRow, jsQueryRes)
		{
			jsMAC = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "MAC");
			jsDestino = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Destino");
			jsDispositivo = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Dispositivo");
			jsPantalla = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Pantalla");
//			jsBoton = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Boton");
			jsEvento = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Evento");
			jsObjeto = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Objeto");
			jsX = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "X");
			jsY = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Y");
			jsW = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "W");
			jsH = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "H");
			jsRedondo = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Redondo");
			jsTexto = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Texto");
			jsIcono = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Icono");
			jsColor_pantalla = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Color_pantalla");
			jsColor_borde = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Color_borde");
			jsColor_fondo = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Color_fondo");
			jsColor_texto = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Color_texto");
			jsOrientacion = cJSON_GetObjectItemCaseSensitive(jsQueryRow, "Orientacion");

			disp = atoi(jsDispositivo->valuestring);
			pant = atoi(jsPantalla->valuestring);
			if(disp != last_disp)
			{
				if(f) fclose(f);
				f = nullptr;
				last_disp = disp;
				m_pServer->m_pLog->Add(20, "[BuildTouchConfig] Armando archivos de pantalla para [%s]", jsMAC->valuestring);
				sprintf(display_path, "%s/download/touch/%s", www_root, jsMAC->valuestring);
				sprintf(cmd, "mkdir -p %s", display_path);
				m_pServer->m_pLog->Add(20, "[BuildTouchConfig] Ejecutando [%s]", cmd);
				rc = system(cmd);
				if(rc)
				{
					m_pServer->m_pLog->Add(1, "[BuildTouchConfig] ERROR [%i] al ejecutar [%s]", rc, cmd);
				}

				/* Abro el listado de patallas */
				if(f_list) fclose(f_list);
				sprintf(file_name, "%s/screen.lst", display_path);
				m_pServer->m_pLog->Add(20, "[BuildTouchConfig] Generando listado de pantallas [%s]", file_name);
				f_list = fopen(file_name, "w");
				if(f_list == nullptr)
				{
					m_pServer->m_pLog->Add(1, "[BuildTouchConfig] ERROR al abrir el listado de pantallas [%s]", file_name);
					break;
				}

			}
			if(pant != last_pant)
			{
				/* Abro el archivo de pantalla */
				if(f) fclose(f);
				f = nullptr;
				last_pant = pant;
				sprintf(screen_name, "screen%i.csv", pant);
				strcpy(file_name, display_path);
				strcat(file_name, "/");
				strcat(file_name, screen_name);
				m_pServer->m_pLog->Add(20, "[BuildTouchConfig] Generando archivo de pantalla [%s]", file_name);
				f = fopen(file_name, "w");
				if(f == nullptr)
				{
					m_pServer->m_pLog->Add(1, "[BuildTouchConfig] ERROR al abrir el archivo [%s]", file_name);
					break;
				}
				/* Agrego la pantalla al listado */
				m_pServer->m_pLog->Add(20, "[BuildTouchConfig] Agregando pantalla [%s] a listado", screen_name);
				fprintf(f_list, "%s\n", screen_name);
				/* Agrego la cabecera a la pantalla */
				fprintf(f, "## Archivo: screen%i.csv\n", pant);
				fprintf(f, "# FONDO,[color]\n");
				fprintf(f, "# BOTON_CUADRADO,[etiqueta],[comando:objeto],[x],[y],[w],[h],[color fondo],[color borde],[color etiqueta],[icono],[orientacion]\n");
				fprintf(f, "# BOTON_REDONDO,[etiqueta],[comando:objeto],[x],[y],[w],[h],[color fondo],[color borde],[color etiqueta],[icono],[orientacion]\n");
				fprintf(f, "#\n");
				fprintf(f, "# etiqueta: Texto que aparece en el botón (opcopnal)\n");
				fprintf(f, "# comando: SWITCH, PULSE, CONFIG, HOME, NEXT, PREV\n");
				fprintf(f, "# objeto: \n");
				fprintf(f, "# x: posición X de la esquina superior derecha del botón\n");
				fprintf(f, "# y: posición Y de la esquina superior derecha del botón\n");
				fprintf(f, "# w: Ancho del botón\n");
				fprintf(f, "# h: altura del botón\n");
				fprintf(f, "# color fondo: 0 a 65535\n");
				fprintf(f, "# color borde: 0 a 65535\n");
				fprintf(f, "# color etiqueta: 0 a 65535\n");
				fprintf(f, "# icono: archivo BMP de 48x48 pixel (opcopnal)\n");
				fprintf(f, "# orientacion: 0 = Horizontal, 1 = Vertical\n");
				fprintf(f, "#\n");
				fprintf(f, "FONDO,%s\n", jsColor_pantalla->valuestring);
				fprintf(f, "#\n");
			}
			switch(atoi(jsEvento->valuestring))
			{
				case 0: /* Nada */
					evento[0] = 0;
					break;
				case 1: /* On */
					strcpy(evento, "ON");
					break;
				case 2: /* Off */
					strcpy(evento, "OFF");
					break;
				case 3: /* Switch */
					strcpy(evento, "SWITCH");
					break;
				case 4: /* Pulsp */
					strcpy(evento, "PULSE");
					break;
				case 10: /* Config */
					strcpy(evento, "CONFIG");
					break;
				case 11: /* Home */
					strcpy(evento, "HOME");
					break;
				case 12: /* Prev */
					strcpy(evento, "PREV");
					break;
				case 13: /* Next */
					strcpy(evento, "NEXT");
					break;
				default:
					evento[0] = 0;
					break;
			}
			/*
			# BOTON_CUADRADO,[etiqueta],[comando:objeto],[x],[y],[w],[h],[color fondo],[color borde],[color etiqueta],[icono],[orientacion]
			# BOTON_REDONDO,[etiqueta],[comando:objeto],[x],[y],[w],[h],[color fondo],[color borde],[color etiqueta],[icono],[orientacion]
			*/
			if( !strcmp(jsTexto->valuestring, "NULL"))
			{
				texto[0] = 0;
			}
			else
			{
				strcpy(texto, jsTexto->valuestring);
			}

			if(atoi(jsObjeto->valuestring))
			{
				strcpy(objeto, ":");
				strcat(objeto, jsDestino->valuestring); 
			}
			else
			{
				objeto[0] = 0;
			}

			if( !strcmp(jsIcono->valuestring, "NULL"))
			{
				icono[0] = 0;
			}
			else
			{
				strcpy(icono, jsIcono->valuestring);
			}

			if(strlen(icono))
			{
				/* Agrego el icono al listado */
				m_pServer->m_pLog->Add(20, "[BuildTouchConfig] Agregando icono [%s] a listado", icono);
				fprintf(f_list, "%s\n", icono);
				/* copio el icono al directorio del display */
				sprintf(cmd, "cp %s/download/touch/%s %s/download/touch/%s/", www_root, icono, www_root, jsMAC->valuestring);
				m_pServer->m_pLog->Add(20, "[BuildTouchConfig] Ejecutando [%s]", cmd);
				rc = system(cmd);
				if(rc)
				{
					m_pServer->m_pLog->Add(1, "[BuildTouchConfig] ERROR [%i] al ejecutar [%s]", rc, cmd);
				}
			}

			m_pServer->m_pLog->Add(1, "[BuildTouchConfig] Agregando boton [%s %s%s]", texto, evento, objeto);
			fprintf(f, "BOTON_%s,%s,%s%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
				(atoi(jsRedondo->valuestring))?"REDONDO":"CUADRADO",
				texto, evento, objeto,
				jsX->valuestring, jsY->valuestring, jsW->valuestring, jsH->valuestring,
				jsColor_fondo->valuestring, jsColor_borde->valuestring, jsColor_texto->valuestring,
				icono, jsOrientacion->valuestring);

		}
		if(f) fclose(f);
		if(f_list) fclose(f_list);
	}

	cJSON_Delete(jsQueryRes);

	m_pServer->m_pLog->Add(1, "[BuildTouchConfig] Fin");
}
