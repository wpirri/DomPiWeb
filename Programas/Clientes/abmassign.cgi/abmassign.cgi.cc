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
  CGMError gmerror;
  CGMBuffer query;
  CGMBuffer response;
  DPConfig *pConfig;
  STRFunc Str;
  cJSON *json_request;
  cJSON *json_response;
  cJSON *json_obj;
  
  char server_address[16];
  char s[16];
  
  char remote_addr[16];
  char request_uri[4096];
  char request_method[8];
  int content_length;
  char s_content_length[8];
  char post_data[4096];
  char label[64];
  char value[1024];

  int i;
  int rc;

  char funcion[64];
  char funcion_call[64];
  char buffer[4096];

  signal(SIGALRM, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);

  remote_addr[0] = 0;
  request_uri[0] = 0;
  request_method[0] = 0;
  post_data[0] = 0;
  content_length = 0;
  s_content_length[0] = 0;
  trace = 0;

  pConfig = new DPConfig("/etc/dompiweb.config");

  if( !pConfig->GetParam("DOMPIWEB_SERVER", server_address))
  {
    fputs("error=99&msg=Config Error\r\n", stdout);
    return 0;
  }

  if( pConfig->GetParam("TRACE-ABMASSIGN.CGI", s))
  {
    trace = atoi(s);
  }

  timeout = 1000;
  if( pConfig->GetParam("CGI-TIMEOUT", s))
  {
    timeout = atoi(s) * 1000;
  }

  json_request = cJSON_CreateObject();

  for(i = 0; env[i]; i++)
  {
    if( !memcmp(env[i], "REMOTE_ADDR=", 12))
    {
      strncpy(remote_addr, env[i]+12, 15);
      cJSON_AddStringToObject(json_request, "REMOTE_ADDR", remote_addr);
    }
    else if( !memcmp(env[i], "REQUEST_URI=", 12))
    {
      strncpy(request_uri, env[i]+12, 4095);
      cJSON_AddStringToObject(json_request, "REQUEST_URI", request_uri);
    }
    else if( !memcmp(env[i], "REQUEST_METHOD=", 15))
    {
      strncpy(request_method, env[i]+15, 7);
      cJSON_AddStringToObject(json_request, "REQUEST_METHOD", request_method);
    }
    else if( !memcmp(env[i], "CONTENT_LENGTH=", 15))
    {
      strncpy(s_content_length, env[i]+15, 7);
      content_length = atoi(s_content_length);
      cJSON_AddStringToObject(json_request, "CONTENT_LENGTH", s_content_length);
    }
  }

  if(content_length)
  {
    fgets(post_data, ((content_length+1)<4096)?(content_length+1):4095, stdin);
  }

  fputs("Connection: close\r\n", stdout);
  fputs("Content-Type: text/html\r\n", stdout);
  fputs("Cache-Control: no-cache\r\n\r\n", stdout);

  if(trace)
  {
    openlog("abmassign.cgi", 0, LOG_USER);

    syslog(LOG_DEBUG, "REMOTE_ADDR: %s",remote_addr);
    syslog(LOG_DEBUG, "REQUEST_URI: [%s]",request_uri);
    syslog(LOG_DEBUG, "REQUEST_METHOD: %s",request_method);
    syslog(LOG_DEBUG, "CONTENT_LENGTH: %s",s_content_length);
    syslog(LOG_DEBUG, "CONFIG_FILE: /etc/dompiweb.config");
    syslog(LOG_DEBUG, "DOMPIWEB_SERVER: [%s]",server_address);
  }

  Str.EscapeHttp(request_uri, request_uri);
  Str.EscapeHttp(post_data, post_data);

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
      if( !strcmp(label, "funcion"))
      {
        strcpy(funcion, value);
      }
      else /* El resto lo paso a json y va como dato */
      {
        cJSON_AddStringToObject(json_request, label, value);
      }
    }
  }

  if(content_length > 0)
  {
    /* Recorro los datos del POST */
    for(i = 0; Str.ParseDataIdx(post_data, label, value, i); i++)
    {
      /* lo agrego al JSon */
      cJSON_AddStringToObject(json_request, label, value);
    }
  }

  if( !strcmp(funcion, "get"))
  {
    strcpy(funcion_call, "dompi_ass_get");
  }
  else if( !strcmp(funcion, "add"))
  {
    strcpy(funcion_call, "dompi_ass_add");
  }
  else if( !strcmp(funcion, "update"))
  {
    strcpy(funcion_call, "dompi_ass_update");
  }
  else if( !strcmp(funcion, "delete"))
  {
    strcpy(funcion_call, "dompi_ass_delete");
  }
  else if( !strcmp(funcion, "struct"))
  {
    strcpy(funcion_call, "dompi_db_struct");
  }
  else if( !strcmp(funcion, "status"))
  {
    strcpy(funcion_call, "dompi_ass_status");
  }
  else if( !strcmp(funcion, "info"))
  {
    strcpy(funcion_call, "dompi_ass_info");
  }
  else if( !strcmp(funcion, "on"))
  {
    strcpy(funcion_call, "dompi_ass_on");
  }
  else if( !strcmp(funcion, "off"))
  {
    strcpy(funcion_call, "dompi_ass_off");
  }
  else if( !strcmp(funcion, "switch"))
  {
    strcpy(funcion_call, "dompi_ass_switch");
  }
  else if( !strcmp(funcion, "pulse"))
  {
    strcpy(funcion_call, "dompi_ass_pulse");
  }
  else if( !strcmp(funcion, "addassigntoplanta"))
  {
    strcpy(funcion_call, "dompi_ass_add_to_planta");
  }
  else
  {
    strcpy(funcion_call, "dompi_ass_list");
  }

  query.Clear();
  response.Clear();

  query = cJSON_PrintUnformatted(json_request);

  cJSON_Delete(json_request);

  if(trace)
  {
    syslog(LOG_DEBUG, "Call Q: %s [%s]", funcion_call, query.C_Str());
  }

  rc = pClient->Call(funcion_call, query, response, timeout);
  if(rc == 0)
  {
    if(trace)
    {
      syslog(LOG_DEBUG, "Call R: [%s]", response.C_Str());
    }

    if( !strcmp(funcion, "on")      ||
        !strcmp(funcion, "off")     ||
        !strcmp(funcion, "switch")  ||
        !strcmp(funcion, "pulse")    )
    {
      /* Esto se contesta en formato formulario */
      json_response = cJSON_Parse(response.C_Str());
      if(json_response)
      {
        /* Armar respuesta en formato de formulario con datos de response en formato JSON */
        post_data[0] = 0;
        json_obj = json_response;
        while( json_obj )
        {
            /* Voy hasta el elemento con datos */
            if(json_obj->type == cJSON_Object)
            {
                json_obj = json_obj->child;
            }
            else
            {
                if(json_obj->type == cJSON_String)
                {
                    if(json_obj->string && json_obj->valuestring)
                    {
                        if(strlen(json_obj->string))
                        {
                          strcpy(label, json_obj->string);
                          strcpy(value, json_obj->valuestring);
                          /* Algunas sustitucions */
                          if( !strcmp(label, "resp_code")) strcpy(label, "error");
                          else if( !strcmp(label, "resp_msg")) strcpy(label, "msg");
                          /* */
                          if(post_data[0] != 0) strcat(post_data, "&");
                          strcat(post_data, label);
                          strcat(post_data, "=");
                          strcat(post_data, value);
                        }
                    }
                }
                json_obj = json_obj->next;
            }
        }
        fprintf(stdout, "%s\r\n", post_data);
        if(trace)
        {
          syslog(LOG_DEBUG, "%s\r\n", post_data);
        }
        cJSON_Delete(json_response);
      }
      else
      {
        /* Supongo que la respuesta ya estaba en formato de formulario y la mando directamente */
        fprintf(stdout, "%s\r\n", response.Data());
        if(trace)
        {
          syslog(LOG_DEBUG, "%s\r\n", response.Data());
        }
      }
    }
    else
    {
      /* Esto se contesta como viene */
      fprintf(stdout, "%s\r\n", response.Data());
      if(trace)
      {
        syslog(LOG_DEBUG, "%s\r\n", response.Data());
      }
    }
  }
  else
  {
    if(trace)
    {
      syslog(LOG_DEBUG, "Call R: [** Error **]");
    }
    fprintf(stdout, "error=%02i&msg=%s\r\n", rc, gmerror.Message(rc).c_str());
    if(trace)
    {
      syslog(LOG_DEBUG, "error=%02i&msg=%s\r\n", rc, gmerror.Message(rc).c_str());
    }
  }
  delete pClient;
  return 0;
}
