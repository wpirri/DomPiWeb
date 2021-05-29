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
using namespace std;

#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>

#include <wiringPi.h> // Include WiringPi library!
//#include <wiringSerial.h>

#include "gpiopin.h"

CGMServerWait *m_pServer;
void OnClose(int sig);

#define GPIO_STATUS_LED_PIN 24

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[4096];
	unsigned long message_len;
	unsigned char blink;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGKILL,         OnClose);
	signal(SIGTERM,         OnClose);
	signal(SIGSTOP,         OnClose);
	signal(SIGABRT,         OnClose);
	signal(SIGQUIT,         OnClose);
	signal(SIGINT,          OnClose);
	signal(SIGILL,          OnClose);
	signal(SIGFPE,          OnClose);
	signal(SIGSEGV,         OnClose);
	signal(SIGBUS,          OnClose);

	m_pServer = new CGMServerWait;
	m_pServer->Init("dompi_gpio");
	m_pServer->m_pLog->Add(1, "Iniciando interface GPIO");

    wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers
    pinMode(gpio_pin[GPIO_STATUS_LED_PIN], OUTPUT);

	if(( rc =  m_pServer->Suscribe("dompi_setio", GM_MSG_TYPE_CR)) != GME_OK)
	{
		m_pServer->m_pLog->Add(1, "ERROR %i al suscribir servicio dompi_status_io", rc);
		OnClose(0);
	}

	m_pServer->SetLogLevel(20);

	blink = 0;
	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 10 )) >= 0)
	{
		if(rc > 0)
		{
			m_pServer->m_pLog->Add(100, "Query recibido fn = [%s] rc = %i", fn, rc);
			if( !strcmp(fn, "dompi_setio"))
			{





				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje");
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
			blink++;
			digitalWrite(gpio_pin[GPIO_STATUS_LED_PIN], (blink&0x01)?HIGH:LOW); // Turn LED ON/OFF
		}
		
	}
	m_pServer->m_pLog->Add(50, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);
	m_pServer->UnSuscribe("dompi_setio", GM_MSG_TYPE_CR);
	delete m_pServer;
	exit(0);
}
