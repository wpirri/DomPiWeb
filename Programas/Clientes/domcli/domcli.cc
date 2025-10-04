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
#include <time.h>
#include <cjson/cJSON.h>

#include <gmonitor/gmc.h>

#include "config.h"
#include "strfunc.h"

#define MAX_INPUT_BUFFER_LEN 1024

#define PROMPT "DOMCLI > "

void Clear( void )
{
  char version[255];
  char clear[12];

  clear[0] = 0x1b;
  clear[1] = 0x5b;
  clear[2] = 0x48;
  clear[3] = 0x1b;
  clear[4] = 0x5b;
  clear[5] = 0x32;
  clear[6] = 0x4a;
  clear[7] = 0x1b;
  clear[8] = 0x5b;
  clear[9] = 0x33;
  clear[10] = 0x4a;
  clear[11] = 0x00;

  fputs(clear, stdout);

  strcpy(version, "          << Intercafe de linea de comandos de sistema de domotica >>\r\n"
                  "                              ..:: DOMCLI 0.1 ::..\r\n");
  fputs(clear, stdout);
  fputs(version, stdout);
}


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
  cJSON *json_query;
  cJSON *json_response;
  cJSON *json_return_message;
  char input_buffer[MAX_INPUT_BUFFER_LEN+1];

  int i;
  char server_address[16];
  int rc;

  signal(SIGALRM, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);

  for(i = 0; env[i]; i++)
  {

  }

  pConfig = new DPConfig("/etc/dompiweb.config");

  if( !pConfig->GetParam("DOMPIWEB_SERVER", server_address))
  {
    fputs("*** Error: 01 - Error de configuracion\r\n", stdout);
    return 0;
  }

  gminit.m_host = server_address;
  gminit.m_port = 5533;

  pClient = new CGMClient(&gminit);

  Clear();
  fputs(PROMPT, stdout);

  while( fgets(input_buffer, MAX_INPUT_BUFFER_LEN, stdin) )
  {
    if(strchr(input_buffer, '\r')) *(strchr(input_buffer, '\r')) = 0;
    if(strchr(input_buffer, '\n')) *(strchr(input_buffer, '\n')) = 0;

    if( !strcmp(input_buffer, "exit")) break;
    if( !strcmp(input_buffer, "clear") || !strcmp(input_buffer, "cls") )
    {
      Clear();
    }
    else if(strlen(input_buffer))
    {
      json_obj = cJSON_CreateObject();

      cJSON_AddStringToObject(json_obj, "UserName", "general");
      cJSON_AddStringToObject(json_obj, "UserPass", "********");
      cJSON_AddStringToObject(json_obj, "CmdLine", input_buffer);

      json_query = cJSON_CreateObject();
      cJSON_AddItemToObject(json_query, "query", json_obj);

      query.Clear();
      response.Clear();

      cJSON_PrintPreallocated(json_query, input_buffer, MAX_INPUT_BUFFER_LEN, 0);
      cJSON_Delete(json_query);
      query = input_buffer;
      rc = pClient->Call("dompi_cmdline", query, response, 500);
      if(rc == 0)
      {
        json_obj = cJSON_Parse(response.Data());
        if(json_obj)
        {
          json_response = cJSON_GetObjectItemCaseSensitive(json_obj, "response");
          if(json_response)
          {
            json_return_message = cJSON_GetObjectItemCaseSensitive(json_response, "resp_msg");
            if(json_return_message)
            {
              fprintf(stdout, "%s\r\n", json_return_message->valuestring);
            }
            else
            {
              if(response.Data())
              {
                fprintf(stdout, "[raw]\r\n%s\r\n", response.Data());
              }
            }
          }
          else
          {
            if(response.Data())
            {
              fprintf(stdout, "[raw]\r\n%s\r\n", response.Data());
            }
          }
          cJSON_Delete(json_obj);
        }
        else
        {
          if(response.Data())
          {
            fprintf(stdout, "[raw]\r\n%s\r\n", response.Data());
          }
        }
      }
      else
      {
        fprintf(stdout, "*** Error: %02i - %s\r\n", rc, gmerror.Message(rc).c_str());
      }
    }
    input_buffer[0] = 0;
    fputs(PROMPT, stdout);
  }

  delete pClient;
  return 0;
}
