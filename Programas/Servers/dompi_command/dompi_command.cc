
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
int i;

time_t last_daily;

#define MAX_HW_LIST_NEW 128
#define MAX_CARD_LIST_NEW 128

void OnClose(int sig);
void LoadSystemConfig(void);

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
					"  sincronizar, <nombre tabla|todo>\r\n"
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
							/* TODO: Completar comando listar */
							if( !memcmp(objeto, "dis", 3))
							{
								listado[0] = 0;
								sprintf(query, "SELECT Id, Dispositivo, MAC, Direccion_IP "
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
									/*               _______________________________ c8c9a34a61a0 000.000.000.000  */
									strcpy(listado, " Nombre MAC IP\n"); 
									/* Obtengo el primero del array del resultado del query */
									cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
									{
										if(json_Query_Row)
										{
											json_Id = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Id");
											json_Dispositivo = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Dispositivo");
											json_MAC = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "MAC");
											json_Direccion_IP = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Direccion_IP");
											if(json_Dispositivo && json_MAC && json_Direccion_IP)
											{
												if(atoi(json_Id->valuestring))
												{
													sprintf(&listado[strlen(listado)], "%-30.30s %12.12s %-15.15s\n", 
														json_Dispositivo->valuestring, json_MAC->valuestring, json_Direccion_IP->valuestring);
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
									strcpy(listado, "             Nombre                     Tipo Estado\n"); 
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
													sprintf(&listado[strlen(listado)], "%-40.40s %3s %5s\n", 
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
								
							}
							else if( !memcmp(objeto, "eve", 3))
							{
								
							}
							sprintf(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"%s\"}}", listado);
						}
						else if( !strcmp(comando, "manten") )
						{
							/* TODO: Hacer algún mantenimiento si es necesario */
							m_pServer->m_pLog->Add(100, "[manten] Mantenimiento de la base de datos.");
							if(pDB) pDB->Manten();

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
						else if( !memcmp(comando, "sinc", 4))
						{
							/* Saco los datos que necesito */
							if( !strcmp(objeto, "todo"))
							{
								m_pServer->Notify("dompi_aa_full_synch", nullptr, 0);
								strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Error\"}}");
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
