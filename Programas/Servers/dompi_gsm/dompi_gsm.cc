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

#include <cjson/cJSON.h>
#include "config.h"

CGMServerWait *m_pServer;
DPConfig *pConfig;

int internal_timeout;
int external_timeout;
char str[256];
char gsm_port[256];
char sms_pool_files[256];

void OnClose(int sig);

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[4096];
	unsigned long message_len;
	time_t t;
	FILE *f;

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
	m_pServer->m_pLog->Add(1, "Iniciando interface de masajeria SMS");

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

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 1 )) >= 0)
	{

		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(50, "%s:(Q)[%s]", fn, message);

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
					sprintf(str, "%ssms-%10lu", sms_pool_files, t);
					f = fopen(str, "w");
					if(f)
					{
						sprintf(str, "SMS:%s:%s", json_msg_to->valuestring, json_msg_txt->valuestring);
						fwrite(str, sizeof(char), strlen(str), f);
						fclose(f);
					}
					/* OK */
					strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				}
				else
				{
					/* El mensaje vino sin HWID */
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Faltan SmsTo y/o SmsTxt\"}}");
				}
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(10, "ERROR al responder mensaje [dompi_hw_get_port_config]");
				}
			}






			else
			{
				m_pServer->m_pLog->Add(50, "GME_SVC_NOTFOUND");
				m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
			}
		}
		else
		{
			/* expiracion del timer */




		}
	}
	m_pServer->m_pLog->Add(50, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);

	m_pServer->UnSuscribe("dompi_send_sms", GM_MSG_TYPE_CR);

	delete m_pServer;
	exit(0);
}
