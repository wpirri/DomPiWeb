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
#include <string.h>
#include <math.h>

#include <cjson/cJSON.h>

#include "config.h"

CGMServerWait *m_pServer;
DPConfig *pConfig;
int internal_timeout;
int external_timeout;

void OnClose(int sig);

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[4096];
	unsigned long message_len;
	char s[16];
	int timer_count = 0;

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
	m_pServer->Init("dompi_garnet");
	m_pServer->m_pLog->Add(1, "Iniciando interface con Bus Garnet");

	pConfig = new DPConfig("/etc/dompiweb.config");

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

	CGMInitData gminit;

    cJSON *json_Message;

	CGMServerBase::GMIOS call_server_resp;
	CGMClient::GMIOS call_client_resp;


	// m_pServer->Suscribe("uno / varios suscriben uno recibe no responde", GM_MSG_TYPE_NOT);
	// m_pServer->Suscribe("uno / varios suscriben uno recibe y responde", GM_MSG_TYPE_CR);
	// m_pServer->Suscribe("uno / varios suscriben todos reciben no responden", GM_MSG_TYPE_MSG);

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 1 )) >= 0)
	{
		json_Message = NULL;

		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(90, "%s:(Q)[%s]", fn, message);

			json_Message = cJSON_Parse(message);

			/* ************************************************************* *
			 *
			 * ************************************************************* */
			if( !strcmp(fn, ""))
			{
				//m_pServer->Resp(NULL, 0, GME_OK);


			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_hw_get_port_config"))
			{






				strcpy(message, "{\"response\":{\"resp_code\":\"4\", \"resp_msg\":\"Datos insuficientes\"}}");

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}




			else
			{
				m_pServer->m_pLog->Add(90, "[%s][R][GME_SVC_NOTFOUND]", fn);
				m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
			}
		}
		else
		{
			/* expiracion del timer 1ms*/




		}




	}
	m_pServer->m_pLog->Add(50, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);

	//m_pServer->UnSuscribe("", GM_MSG_TYPE_NOT CR MSG);

	delete m_pServer;
	delete pConfig;
	
	exit(0);
}
