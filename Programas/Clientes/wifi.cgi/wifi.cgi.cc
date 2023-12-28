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

#define MAX_POST_DATA 4096
#define MAX_GET_DATA  1024

int main(int /*argc*/, char** /*argv*/, char** env)
{
  CGMInitData gminit;
  CGMClient *pClient;
  CGMError gmerror;
  CGMBuffer query;
  CGMBuffer response;
  DPConfig *pConfig;
  STRFunc Str;
  cJSON *json_obj;

  char server_address[16];
  char s_trace[5];
  
  char remote_addr[16];
  char request_uri[32];
  char request_method[8];
  int content_length;
  char s_content_length[8];
  char get_data[MAX_GET_DATA+1];
  char post_data[MAX_POST_DATA+1];
  char label[64];
  char value[64];

  int i;
  int rc;

  signal(SIGALRM, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);

  remote_addr[0] = 0;
  request_uri[0] = 0;
  request_method[0] = 0;
  post_data[0] = 0;
  content_length = 0;
  s_content_length[0] = 0;
  trace = 0;

  pConfig = new DPConfig("/etc/dompiio.config");

  if( !pConfig->GetParam("DOMPIIO_SERVER", server_address))
  {
    return 0;
  }

  if( pConfig->GetParam("TRACE-WIFI.CGI", s_trace))
  {
    trace = atoi(s_trace);
  }

  json_obj = cJSON_CreateObject();

  for(i = 0; env[i]; i++)
  {
    if( !memcmp(env[i], "REMOTE_ADDR=", 12))
    {
      strncpy(remote_addr, env[i]+12, 15);
      cJSON_AddStringToObject(json_obj, "REMOTE_ADDR", remote_addr);
    }
    else if( !memcmp(env[i], "REQUEST_URI=", 12))
    {
      strncpy(request_uri, env[i]+12, 31);
      cJSON_AddStringToObject(json_obj, "REQUEST_URI", request_uri);
    }
    else if( !memcmp(env[i], "REQUEST_METHOD=", 15))
    {
      strncpy(request_method, env[i]+15, 7);
      cJSON_AddStringToObject(json_obj, "REQUEST_METHOD", request_method);
    }
    else if( !memcmp(env[i], "CONTENT_LENGTH=", 15))
    {
      strncpy(s_content_length, env[i]+15, 7);
      content_length = atoi(s_content_length);
      cJSON_AddStringToObject(json_obj, "CONTENT_LENGTH", s_content_length);
    }
  }

  if(content_length)
  {
    fgets(post_data, ((content_length+1)<MAX_POST_DATA)?(content_length+1):MAX_POST_DATA, stdin);
  }

  fputs("Connection: close\r\n", stdout);
  fputs("Content-Type: text/html\r\n", stdout);
  fputs("Cache-Control: no-cache\r\n\r\n", stdout);

  if(trace)
  {
    openlog("wifi.cgi", 0, LOG_USER);

    syslog(LOG_DEBUG, "REMOTE_ADDR: %s",remote_addr);
    syslog(LOG_DEBUG, "REQUEST_URI: [%s]",request_uri);
    syslog(LOG_DEBUG, "REQUEST_METHOD: %s",request_method);
    syslog(LOG_DEBUG, "CONTENT_LENGTH: %s",s_content_length);
    syslog(LOG_DEBUG, "CONFIG_FILE: /etc/dompiio.config");
    syslog(LOG_DEBUG, "DOMPIWEB_SERVER: [%s]",server_address);
  }

  if(strchr(request_uri, '?'))
  {
    strcpy(get_data, strchr(request_uri, '?')+1);
    if(trace)
    {
        syslog(LOG_DEBUG, "GET DATA: [%s]", get_data);
    }
    /* Recorro los parametros del GET */
    for(i = 0; Str.ParseDataIdx(get_data, label, value, i); i++)
    {
      cJSON_AddStringToObject(json_obj, label, value);
    }
  }

  if(content_length)
  {
    if(trace) syslog(LOG_DEBUG, "POST_DATA: [%s]",post_data);

    /* Recorro los parametros del POST */
    for(i = 0; Str.ParseDataIdx(post_data, label, value, i); i++)
    {
      cJSON_AddStringToObject(json_obj, label, value);
    }
  }

  gminit.m_host = server_address;
  gminit.m_port = 5533;

  pClient = new CGMClient(&gminit);

  query.Clear();
  response.Clear();

  query = cJSON_PrintUnformatted(json_obj);

  cJSON_Delete(json_obj);

  if(trace)
  {
    syslog(LOG_DEBUG, "Call Q: wifi [%s]", query.C_Str());
  }

  rc = pClient->Call("dompi_pi_wifi", query, response, 100);
  if(rc == 0)
  {
    if(trace)
    {
      syslog(LOG_DEBUG, "Call R: [%s]", response.Data());
    }
    /*Armar respuesta en formato POST con datos de response.Data() en formato JSON */
    /* Contenido de la página */
    fprintf(stdout, "%s\r\n", response.Data());
  }
  else
  {
    fprintf(stdout, "{ \"rc\":\"%02i\", \"msg\":\"%s\" }\r\n", rc, gmerror.Message(rc).c_str());
    if(trace)
    {
      syslog(LOG_DEBUG, "{ \"rc\":\"%02i\", \"msg\":\"%s\" }\r\n", rc, gmerror.Message(rc).c_str());
    }
  }
  delete pClient;
  return 0;
}
