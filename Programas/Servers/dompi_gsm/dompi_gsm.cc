/***************************************************************************
    Copyright (C) 2022   Walter Pirri

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
#include <gmonitor/gmc.h>

#include <string>
#include <iostream>
#include <csignal>
using namespace std;

#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* para readdir */
#include <sys/types.h>
#include <dirent.h>

#include <cjson/cJSON.h>
#include "config.h"
#include "modulo_gsm.h"

#define SMS_TEMP_FILE "sms.tmp"

CGMServerWait *m_pServer;
DPConfig *pConfig;
ModGSM* pModem;

int internal_timeout;
int external_timeout;
char str[256];
char gsm_port[256];
char sms_pool_files[256];
char sms_buffer[1024];

void OnClose(int sig);
void ScanSMS(const char *path);

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[4096];
	unsigned long message_len;
	time_t t;
	FILE *f;
	char filename[FILENAME_MAX+1];
	char filename_tmp[FILENAME_MAX+1];

    cJSON *json_obj;
    cJSON *json_msg_to;
    cJSON *json_msg_txt;

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
	m_pServer->Init("dompi_gsm");
	m_pServer->m_pLog->Add(1, "Iniciando interface de mensajeria SMS");

	pConfig = new DPConfig("/etc/dompiweb.config");

	internal_timeout = 1000;
	if( pConfig->GetParam("INTERNAL-TIMEOUT", str))
	{
		internal_timeout = atoi(str) * 1000;
	}

	external_timeout = 1000;
	if( pConfig->GetParam("EXTERNAL-TIMEOUT", str))
	{
		external_timeout = atoi(str) * 1000;
	}

	gsm_port[0] = 0;
	pConfig->GetParam("GSM-PORT", gsm_port);

	sms_pool_files[0] = 0;
	pConfig->GetParam("SMS_POOL_FILES", sms_pool_files);

	CGMInitData gminit;

	m_pServer->Suscribe("dompi_send_sms", GM_MSG_TYPE_CR);

	pModem = new ModGSM(sms_pool_files, m_pServer, gsm_port);

	m_pServer->m_pLog->Add(1, "Soporte SMS / GSM Inicializado.");

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 1 )) >= 0)
	{

		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(90, "%s:(Q)[%s]", fn, message);

			/* ************************************************************* *
			 *
			 * ************************************************************* */
			if( !strcmp(fn, "dompi_send_sms"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;

				t = time(&t);
				json_msg_to = cJSON_GetObjectItemCaseSensitive(json_obj, "SmsTo");
				json_msg_txt = cJSON_GetObjectItemCaseSensitive(json_obj, "SmsTxt");

				if(json_msg_to && json_msg_txt)
				{
					if( sms_pool_files[strlen(sms_pool_files) - 1] != '/' )
					{
						sprintf(filename, "%s/sms-%10lu", sms_pool_files, t);
						sprintf(filename_tmp, "%s/tmp-%10lu", sms_pool_files, t);
					}
					else
					{
						sprintf(filename, "%ssms-%10lu", sms_pool_files, t);
						sprintf(filename_tmp, "%stmp-%10lu", sms_pool_files, t);
					}
					f = fopen(filename_tmp, "w");
					if(f)
					{
						sprintf(sms_buffer, "SMS:%s:%s\n", json_msg_to->valuestring, json_msg_txt->valuestring);
						if(fwrite(sms_buffer, sizeof(char), strlen(sms_buffer), f) != strlen(sms_buffer))
						{
							/* Error */
							strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"ERROR: Al escribir archivo de mensaje.\"}}");
							m_pServer->m_pLog->Add(1, "[dompi_send_sms] ERROR Al escribir archivo de mensaje");
						}
						fclose(f);
						rename(filename_tmp, filename);
						/* OK */
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						/* Error */
						strcpy(message, "{\"response\":{\"resp_code\":\"3\", \"resp_msg\":\"ERROR: Al crear archivo de mensaje.\"}}");
						m_pServer->m_pLog->Add(1, "[dompi_send_sms] ERROR Al crear archivo de mensaje");
					}
				}
				else
				{
					/* El mensaje vino sin HWID */
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Faltan SmsTo y/o SmsTxt\"}}");
					m_pServer->m_pLog->Add(1, "[dompi_send_sms] ERROR Faltan SmsTo y/o SmsTxt");
				}
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [dompi_hw_get_port_config]");
				}
			}






			else
			{
				m_pServer->m_pLog->Add(50, "GME_SVC_NOTFOUND");
				m_pServer->Resp((void*)0, 0, GME_SVC_NOTFOUND);
			}
		}
		else
		{
			/* expiracion del timer */




		}
		pModem->Task();

		/* Busco mensajes pendientes de envío */
		if(pModem->ReadySMS()) ScanSMS(sms_pool_files);


	}
	m_pServer->m_pLog->Add(50, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);
	m_pServer->UnSuscribe("dompi_send_sms", GM_MSG_TYPE_CR);
	pModem->Close();

	delete pModem;
	delete m_pServer;
	exit(0);
}

void ScanSMS(const char *path)
{
    DIR *dir;
    struct dirent *dir_ent;
    FILE *f;
    char buffer[4096];
	char filename[FILENAME_MAX+1];
	char filename_rename[FILENAME_MAX+1];
    char *to, *msg;
	int rc;

    dir = opendir(path);
    if( !dir)
    {
        if(m_pServer) m_pServer->m_pLog->Add(1, "[ScanSMS] Error al abrir directorio [%s]", path);
        return;
    }
    while( (dir_ent = readdir(dir)) )
    {
        if(dir_ent->d_type == DT_REG && dir_ent->d_name[0] == 's' && dir_ent->d_name[1] == 'm' && dir_ent->d_name[2] == 's')
        {
            if(m_pServer) m_pServer->m_pLog->Add(90, "[ScanSMS] Procesando: [%s]", dir_ent->d_name);
			if( *(path + strlen(path) - 1) != '/' )
			{
				sprintf(filename, "%s/%s", path, dir_ent->d_name);
			}
			else
			{
				sprintf(filename, "%s%s", path, dir_ent->d_name);
			}
            if(m_pServer) m_pServer->m_pLog->Add(90, "[ScanSMS] Procesando: [%s]", filename);
			f = fopen(filename, "r");
			if( !f )
			{
				if(m_pServer) m_pServer->m_pLog->Add(1, "[ScanSMS] Error al abrir archivo [%s]", filename);
				if( *(path + strlen(path) - 1) != '/' )
				{
					sprintf(filename_rename, "%s/error-%s", path, dir_ent->d_name);
				}
				else
				{
					sprintf(filename_rename, "%serror-%s", path, dir_ent->d_name);
				}
				rename(filename, filename_rename);
				break;
			}
			rc = fread(buffer, sizeof(char), 4096, f);
			if(rc == 0)
			{
				if(m_pServer) m_pServer->m_pLog->Add(1, "[ScanSMS] Error al leer archivo [%s]", filename);
				fclose(f);
				if( *(path + strlen(path) - 1) != '/' )
				{
					sprintf(filename_rename, "%s/error-%s", path, dir_ent->d_name);
				}
				else
				{
					sprintf(filename_rename, "%serror-%s", path, dir_ent->d_name);
				}
				rename(filename, filename_rename);
				break;
			}
			fclose(f);
			if(buffer[0] != 'S' || buffer[1] != 'M' || buffer[2] != 'S' || buffer[3] != ':')
			{
				if(m_pServer) m_pServer->m_pLog->Add(1, "[ScanSMS] Error parseando archivo [%s]", filename);
				if( *(path + strlen(path) - 1) != '/' )
				{
					sprintf(filename_rename, "%s/error-%s", path, dir_ent->d_name);
				}
				else
				{
					sprintf(filename_rename, "%serror-%s", path, dir_ent->d_name);
				}
				rename(filename, filename_rename);
				break;
			}
			to = &buffer[4];
			if( !strchr(to, ':'))
			{
				if(m_pServer) m_pServer->m_pLog->Add(1, "[ScanSMS] Error parseando archivo [%s]", filename);
				if( *(path + strlen(path) - 1) != '/' )
				{
					sprintf(filename_rename, "%s/error-%s", path, dir_ent->d_name);
				}
				else
				{
					sprintf(filename_rename, "%serror-%s", path, dir_ent->d_name);
				}
				rename(filename, filename_rename);
				break;
			}
			msg = strchr(to, ':');
			*msg = 0;
			msg++;
			if(strchr(msg, '\n')) *(strchr(msg, '\n')) = 0;
			if(strchr(msg, '\r')) *(strchr(msg, '\r')) = 0;
			if(m_pServer) m_pServer->m_pLog->Add(90, "[ScanSMS] SMS To: [%s] Msg: [%s]", to, msg);
			/* Envio SMS */
			if(pModem->SendSMS(to, msg) != 0)
			{
				if(m_pServer) m_pServer->m_pLog->Add(1, "[ScanSMS] Error enviando mensaje SMS To: [%s] Msg: [%s]", to, msg);
//				if( *(path + strlen(path) - 1) != '/' )
//				{
//					sprintf(filename_rename, "%s/error-%s", path, dir_ent->d_name);
//				}
//				else
//				{
//					sprintf(filename_rename, "%serror-%s", path, dir_ent->d_name);
//				}
//				rename(filename, filename_rename);
				break;
			}
			/* Renombro archivo temporal */
			if( *(path + strlen(path) - 1) != '/' )
			{
				sprintf(filename_rename, "%s/send-%s", path, dir_ent->d_name);
			}
			else
			{
				sprintf(filename_rename, "%ssend-%s", path, dir_ent->d_name);
			}
            if(m_pServer) m_pServer->m_pLog->Add(100, "[ScanSMS] Renombrando: [%s -> %s]", filename, filename_rename);
			rename(filename, filename_rename);
        }
    }
    closedir(dir);
}