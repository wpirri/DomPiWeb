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
  
  int i;
  char server_address[16];
  int rc;

  signal(SIGALRM, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);

  for(i = 0; env[i]; i++)
  {

  }

  pConfig = new DPConfig("/etc/dompi_manager.config");

  if( !pConfig->GetParam("DOMPIWEB_SERVER", server_address))
  {
    fputs("{ \"rc\":\"01\", \"msg\":\"Error de configuracion\" }\r\n", stdout);
    return 0;
  }

  gminit.m_host = server_address;
  gminit.m_port = 5533;

  pClient = new CGMClient(&gminit);

  json_obj = cJSON_CreateObject();

  
  


  query.Clear();
  response.Clear();
  query = buffer;
  syslog(LOG_DEBUG, "Call %s [%s]", funcion_call, buffer); 
  rc = pClient->Call(funcion_call, query, response, 100);
  if(rc == 0)
  {
    fprintf(stdout, "%s\r\n", response.Data());
  }
  else
  {
    fprintf(stdout, "{ \"rc\":\"%02i\", \"msg\":\"%s\" }\r\n", rc, gmerror.Message(rc).c_str());
  }
  delete pClient;
  return 0;
}
