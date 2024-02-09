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

#include <string>
#include <iostream>
#include <csignal>
#include <cstdio>
using namespace std;

#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>

#include "dom32iopi.h"
#include "ctcp.h"
#include "strfunc.h"
#include "config.h"

#define MAX_BUFFER_LEN 		32767
#define KEEP_ALIVE			60
#define SEND_RETRY			10
#define SEND_RETRY_DELAY	1

CGMServerWait *m_pServer;
DPConfig *pConfig;
Dom32IoPi *pPI;


void OnClose(int sig);

int internal_timeout;
int external_timeout;
unsigned char blink_count;
int timer_count_keep_alive;
int count_notificar_retry;
int delay_notificar_retry;
int conectado_con_central;
int usando_central;

/* POST
* 1.- %s: URI
* 2.- %s: Host
* 3.- %d: Content-Length
* 4.- %s: datos
*/
char http_post[] =	"POST %s HTTP/1.1\r\n"
					"Host: %s\r\n"
					"Connection: keep-alive\r\n"
					"Content-Length: %d\r\n"
					"User-Agent: DomPiSrv/1.00 (RaspBerryPi;Dom32)\r\n"
					"Accept: text/html,text/xml\r\n"
					"Content-Type: application/x-www-form-urlencoded\r\n\r\n%s";

/*
* GET
* 1.- %s: URI
* 2.- %s: Host
*/
char http_get[] =	"GET %s HTTP/1.1\r\n"
					"Host: %s\r\n"
					"Connection: close\r\n"
					"User-Agent: DomPiSrv/1.00 (RaspBerryPi;Dom32)\r\n"
					"Accept: text/html,text/xml\r\n\r\n";

void HTTPNotificarStatus( void )
{
	char http_rqst_data[4096];
	char http_rqst[4096];
	char http_resp[4096];
	char http_rqst_uri[256];
	char label[64], value[1024];
	int first_change = 1;
	CTcp tcp;
	int rc;
	char *p;
	int i;
	STRFunc str;

	timer_count_keep_alive = 0;

    snprintf(http_rqst_data, 1024,
        "ID=%s&TYP=PI&IO1=%i&IO2=%i&IO3=%i&IO4=%i&IO5=%i&IO6=%i&IO7=%i&IO8=%i&OUT1=%i&OUT2=%i",
        pPI->m_pi_data.config.comm.hw_mac,
		pPI->m_pi_data.status.port[0].status,
		pPI->m_pi_data.status.port[1].status,
		pPI->m_pi_data.status.port[2].status,
		pPI->m_pi_data.status.port[3].status,
		pPI->m_pi_data.status.port[4].status,
		pPI->m_pi_data.status.port[5].status,
		pPI->m_pi_data.status.port[6].status,
		pPI->m_pi_data.status.port[7].status,
		pPI->m_pi_data.status.port[8].status,
		pPI->m_pi_data.status.port[9].status);
    
    if( pPI->m_pi_data.status.port[0].change ||
	    pPI->m_pi_data.status.port[1].change ||
	    pPI->m_pi_data.status.port[2].change ||
	    pPI->m_pi_data.status.port[3].change ||
	    pPI->m_pi_data.status.port[4].change ||
	    pPI->m_pi_data.status.port[5].change ||
	    pPI->m_pi_data.status.port[6].change ||
	    pPI->m_pi_data.status.port[7].change ||
	    pPI->m_pi_data.status.port[8].change ||
	    pPI->m_pi_data.status.port[9].change  )
    {
        strcat(http_rqst_data, "&CHG=");
        
        if(pPI->m_pi_data.status.port[0].change)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO1");
        }
        if(pPI->m_pi_data.status.port[1].change)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO2");
        }
        if(pPI->m_pi_data.status.port[2].change)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO3");
        }
        if(pPI->m_pi_data.status.port[3].change)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO4");
        }
        if(pPI->m_pi_data.status.port[4].change)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO5");
        }
        if(pPI->m_pi_data.status.port[5].change)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO6");
        }
        if(pPI->m_pi_data.status.port[6].change)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO7");
        }
        if(pPI->m_pi_data.status.port[7].change)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO8");
        }
        if(pPI->m_pi_data.status.port[8].change)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "OUT1");
        }
        if(pPI->m_pi_data.status.port[9].change)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "OUT2");
        }
    }
    

    /* Agrego solicitud de configuración */
    if(pPI->m_pi_data.config.default_config)
    {
        strcat(http_rqst_data, "&GETCONF=1");
    }
    
    snprintf(http_rqst_uri, 255, "%s/infoio.cgi", pPI->m_pi_data.config.comm.rqst_path);
/* POST
 * 1.- %s: URI
 * 2.- %s: Host
 * 3.- %d: Content-Length
 * 4.- %s: datos
 */
    snprintf(http_rqst, 4095, http_post,
            http_rqst_uri,
            pPI->m_pi_data.config.comm.host1,
            strlen(http_rqst_data),
            http_rqst_data);

	if(usando_central != 1 && pPI->m_pi_data.config.comm.host2[0] == 0)
	{
		usando_central = 1;
	}

    m_pServer->m_pLog->Add(100, "[HTTPNotificarStatus] - Conectando con: [%s:%i]",
							(usando_central == 1)?pPI->m_pi_data.config.comm.host1:pPI->m_pi_data.config.comm.host2,
							(usando_central == 1)?pPI->m_pi_data.config.comm.host1_port:pPI->m_pi_data.config.comm.host2_port);
    m_pServer->m_pLog->Add(100, "[HTTPNotificarStatus] - Rqst [%d][%s]", strlen(http_rqst), http_rqst);

    if(tcp.Query((usando_central == 1)?pPI->m_pi_data.config.comm.host1:pPI->m_pi_data.config.comm.host2, 
				 (usando_central == 1)?pPI->m_pi_data.config.comm.host1_port:pPI->m_pi_data.config.comm.host2_port, 
				 http_rqst, http_resp, 4095, external_timeout) > 0)
    {
        m_pServer->m_pLog->Add(100, "[HTTPNotificarStatus]  Resp [%d][%s]", strlen(http_resp), http_resp);
        rc = pPI->HttpRespCode(http_resp);
        if(rc == 200)
		{
			/* Ok */
			/* Me posiciono al final de la cabecera HTTP, al principio de los datos */
			p = strstr(http_resp, "\r\n\r\n");
			if(p)
			{
				/* Salteo CR/LF CR/LF */
				p += 4;
				for(i = 0; str.ParseDataIdx(p, label, value, i); i++)
				{
					/* Proceso la respuesta */





				}
			}
			if(conectado_con_central == 0)
			{
				m_pServer->m_pLog->Add(10, "ON-LINE con: [%s:%i]",
										(usando_central == 1)?pPI->m_pi_data.config.comm.host1:pPI->m_pi_data.config.comm.host2,
										(usando_central == 1)?pPI->m_pi_data.config.comm.host1_port:pPI->m_pi_data.config.comm.host2_port);
			}
			conectado_con_central = (usando_central)?usando_central:2;
			count_notificar_retry = 0;
			delay_notificar_retry = 0;
			/* Reseteo los cambios */
			pPI->m_pi_data.status.port[0].change = 0;
			pPI->m_pi_data.status.port[1].change = 0;
			pPI->m_pi_data.status.port[2].change = 0;
			pPI->m_pi_data.status.port[3].change = 0;
			pPI->m_pi_data.status.port[4].change = 0;
			pPI->m_pi_data.status.port[5].change = 0;
			pPI->m_pi_data.status.port[6].change = 0;
			pPI->m_pi_data.status.port[7].change = 0;
			pPI->m_pi_data.status.port[8].change = 0;
			pPI->m_pi_data.status.port[9].change = 0;
			return;
		}
	}
	m_pServer->m_pLog->Add(10, "OFF-LINE");
	conectado_con_central = 0;
	if(count_notificar_retry == 0)
	{
		usando_central = (1 - usando_central);
	}
}

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[MAX_BUFFER_LEN+1];
	unsigned long message_len;
	//unsigned long exclude_modem = 0;

	char s[16];
	char filename[FILENAME_MAX+1];

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
	m_pServer->Init("dompi_gpio");
	m_pServer->m_pLog->Add(1, "Iniciando interface GPIO");

	pConfig = new DPConfig("/etc/dompiio.config");
	internal_timeout = 1000;
	if( pConfig->GetParam("INTERNAL-TIMEOUT", s))
	{
		internal_timeout = atoi(s) * 1000;
	}
	external_timeout = 1000;
	if( pConfig->GetParam("INTERNAL-TIMEOUT", s))
	{
		external_timeout = atoi(s) * 1000;
	}

	if( pConfig->GetParam("RBPI-IO-CONFIG", filename))
	{
	    pPI = new Dom32IoPi(filename);
	}
	else
	{
	    pPI = new Dom32IoPi();
	}
	pPI->LoadConfig();

    cJSON *json_Message;
    cJSON *json_un_obj;

	m_pServer->Suscribe("dompi_pi_ioconfig", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_iostatus", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_ioswitch", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_wifi", GM_MSG_TYPE_CR);

	timer_count_keep_alive = 0;
	count_notificar_retry = 0;
	delay_notificar_retry = 0;
	conectado_con_central = 0;
	usando_central = 1;

	blink_count = 0;
	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 10 )) >= 0)
	{
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(50, "%s:(Q)[%s]", fn, message);

			json_Message = cJSON_Parse(message);
			message[0] = 0;
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			if( !strcmp(fn, "dompi_pi_ioconfig"))
			{

				json_un_obj = json_Message;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									/*                         Label               Value      */
									m_pServer->m_pLog->Add(100, "[ConfigIO] %s = %s", json_un_obj->string, json_un_obj->valuestring);
									pPI->ConfigIO(json_un_obj->string, json_un_obj->valuestring);
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}

				/* OK */
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_pi_iostatus"))
			{

				json_un_obj = json_Message;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									/*                         Label               Value      */
									m_pServer->m_pLog->Add(100, "[SetIO] %s = %s", json_un_obj->string, json_un_obj->valuestring);
									pPI->SetIO(json_un_obj->string, json_un_obj->valuestring);
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}

				/* OK */
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_pi_ioswitch"))
			{




				/* OK */
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_pi_wifi"))
			{




				/* OK */
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}




			else
			{
				m_pServer->m_pLog->Add(50, "GME_SVC_NOTFOUND");
				m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
			}

			cJSON_Delete(json_Message);

			m_pServer->m_pLog->Add(100, "[SetIOStatus]");
			pPI->SetIOStatus();

		}
		else
		{
			/* expiracion del timer */
			
			blink_count++;
			if(delay_notificar_retry) delay_notificar_retry--;

			if(conectado_con_central)
			{
				pPI->SetModeLed( 0 );
				pPI->SetStatusLed( (blink_count&4)?1:0 );
			}
			else
			{
				pPI->SetModeLed( (blink_count&1)?0:1 );
				pPI->SetStatusLed( 0 );
			}

			/* Notifico cuando hay cambios o cuando vence el timer de keep alive */
			if(pPI->GetIOStatus())
			{
				m_pServer->m_pLog->Add(100, "[GetIOStatus] Detecta cambio de estado");
				timer_count_keep_alive = 0;
				count_notificar_retry = SEND_RETRY;
				delay_notificar_retry = 0;
			}
			else
			{
				timer_count_keep_alive++;
				if( timer_count_keep_alive >= (KEEP_ALIVE*10) )
				{
					timer_count_keep_alive = 0;
					count_notificar_retry = SEND_RETRY;
					delay_notificar_retry = 0;
				}
			}





		}

		
		if(count_notificar_retry && delay_notificar_retry == 0)
		{
			count_notificar_retry--;
			delay_notificar_retry = SEND_RETRY_DELAY * 10;
			HTTPNotificarStatus();
		}
		
	}
	m_pServer->m_pLog->Add(50, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);

	m_pServer->UnSuscribe("dompi_pi_ioconfig", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_iostatus", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_ioswitch", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_wifi", GM_MSG_TYPE_CR);

	delete m_pServer;
	delete pConfig;
	delete pPI;

	exit(0);
}
