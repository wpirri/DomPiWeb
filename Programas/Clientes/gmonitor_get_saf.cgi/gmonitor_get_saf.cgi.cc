/***************************************************************************
 * Copyright (C) 2021 Walter Pirri
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 ***************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <syslog.h>
#include <time.h>
#include <cjson/cJSON.h>

#include <gmonitor/gmc.h>

#include "config.h"
#include "strfunc.h"

int trace;
int timeout;

int main(int /*argc*/, char** /*argv*/, char** env)
{
  CGMInitData gminit;
  CGMClient *pClient;
  CGMClient::GMIOS resp;
  CGMError gmerror;
  STRFunc Str;
  DPConfig *pConfig;
  int i;
  
  char server_address[16];
  char s[16];
  
  char remote_addr[16];
  char request_uri[4096];
  char request_method[8];
  char buffer[4096];
  char label[64];
  char value[1024];
  char saf_name[64];
  int rc;

  signal(SIGALRM, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);

  remote_addr[0] = 0;
  request_uri[0] = 0;
  request_method[0] = 0;
  trace = 0;

  pConfig = new DPConfig("/etc/dompiweb.config");

  if( !pConfig->GetParam("DOMPIWEB_SERVER", server_address))
  {
    fputs("{ \"rc\":\"01\", \"msg\":\"Error de configuracion\" }\r\n", stdout);
    return 0;
  }

  if( pConfig->GetParam("TRACE-GETSAF.CGI", s))
  {
    trace = atoi(s);
  }

  timeout = 1000;
  if( pConfig->GetParam("CGI-TIMEOUT", s))
  {
    timeout = atoi(s) * 1000;
  }

    for(i = 0; env[i]; i++)
  {
    if( !memcmp(env[i], "REMOTE_ADDR=", 12))
    {
      strncpy(remote_addr, env[i]+12, 15);
    }
    else if( !memcmp(env[i], "REQUEST_URI=", 12))
    {
      strncpy(request_uri, env[i]+12, 4095);
    }
    else if( !memcmp(env[i], "REQUEST_METHOD=", 15))
    {
      strncpy(request_method, env[i]+15, 7);
    }
  }

  fputs("Connection: close\r\n", stdout);
  fputs("Content-Type: text/html\r\n", stdout);
  fputs("Cache-Control: no-cache\r\n\r\n", stdout);

  Str.EscapeHttp(request_uri, request_uri);

  if(trace)
  {
    openlog("gmonitor_get_saf.cgi", 0, LOG_USER);
    syslog(LOG_DEBUG, "REMOTE_ADDR=%s REQUEST_URI=%s REQUEST_METHOD=%s", 
              remote_addr, request_uri, request_method);
  }

  gminit.m_host = server_address;
  gminit.m_port = 5533;

  pClient = new CGMClient(&gminit);

  if(strchr(request_uri, '?'))
  {
    strcpy(buffer, strchr(request_uri, '?')+1);
    if(trace) syslog(LOG_DEBUG, "GET DATA: [%s]", buffer);
    /* Recorro los parametros del GET */
    for(i = 0; Str.ParseDataIdx(buffer, label, value, i); i++)
    {
      /* El parametro funcion lo uso para el mensaje */
      if( !strcmp(label, "saf"))
      {
        strcpy(saf_name, value);
      }
    }
  }

  if(trace) syslog(LOG_DEBUG, "Dequeue %s", saf_name); 
  rc = pClient->Dequeue(saf_name, &resp);
  if(rc == 0)
  {
    fprintf(stdout, "%s\r\n", (char*)resp.data);
  }
  else
  {
    fprintf(stdout, "{ \"rc\":\"%02i\", \"msg\":\"%s\" }\r\n", rc, gmerror.Message(rc).c_str());
  }
  pClient->Free(resp);

  delete pClient;
  return 0;
}
