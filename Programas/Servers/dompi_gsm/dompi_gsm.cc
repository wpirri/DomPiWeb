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
#include <gmonitor/svcstru.h>	/*ST_STIMER*/
#include <gmonitor/gmontdb.h>
/*#include <gmonitor/gmstring.h>*/
#include <gmonitor/gmswaited.h>
#include <gmonitor/gmc.h>
#include <gmonitor/svcstru.h>

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
ST_STIMER st_timer;

int internal_timeout;
int external_timeout;
char str[256];
char gsm_port[256];
char sms_buffer[1024];

void OnClose(int sig);
void AddSaf( void );

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[4096];
	unsigned long message_len;
	char from[32];
	char msg[4096];
	CGMServerBase::GMIOS resp;

    /*cJSON *json_Message;*/

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

	pModem = new ModGSM("dompi_sms_output", "dompi_sms_input", m_pServer);

	m_pServer->Suscribe("timer_check_sms_input", GM_MSG_TYPE_NOT);

	/* Suscribo un timer */
	strcpy(st_timer.set_timer.servicio, "timer_check_sms_input");
	st_timer.set_timer.modo_servicio = GM_MSG_TYPE_NOT;
	st_timer.set_timer.delay = 5;
	st_timer.set_timer.tipo_timer = 'R'; /* Repetitivo */
	st_timer.set_timer.at = 0;
	st_timer.set_timer.len = 0;
	st_timer.set_timer.data[0] = 0;
	m_pServer->Call(".set_timer", &st_timer, sizeof(ST_STIMER), &resp, 1000);
	memcpy(&st_timer, resp.data, sizeof(ST_STIMER));

	AddSaf();

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
			if( !strcmp(fn, "timer_check_sms_input"))
			{
				/*m_pServer->Resp(nullptr, 0, GME_OK);*/

				if(pModem->GetSMS(from, msg))
				{
					sprintf(message, "{\"from\":\"%s\", \"msg\":\"%s\"}", from, msg);
					m_pServer->m_pLog->Add(90, "Post [dompi_sms_msg][%s]", message);
					/* Se envía a todos los suscriptos */
					m_pServer->Notify("dompi_sms_msg", message, strlen(message));
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
	}
	m_pServer->m_pLog->Add(50, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);
	/*m_pServer->UnSuscribe("xxxx", GM_MSG_TYPE_CR);*/
	pModem->Close();

	st_timer.kill_timer.id = st_timer.set_timer.id;
	m_pServer->Notify(".kill_timer", &st_timer, sizeof(ST_STIMER));

	delete pModem;
	delete m_pServer;
	exit(0);
}

void AddSaf( void )
{
	ST_SQUEUE sq;

	sq.len = 0;
	strcpy(sq.saf_name, "dompi_sms_output");
	m_pServer->Notify(".create-queue", &sq, sizeof(ST_SQUEUE));	
	strcpy(sq.saf_name, "dompi_sms_input");
	m_pServer->Notify(".create-queue", &sq, sizeof(ST_SQUEUE));	
}
