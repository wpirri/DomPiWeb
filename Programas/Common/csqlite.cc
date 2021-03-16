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
#include "csqlite.h"

#include <string>
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
using namespace std;

#include <unistd.h>
#include <sys/msg.h>
#include <time.h>
#include <string.h>
#ifdef __DEBUG__
#include <syslog.h>
#endif


int CSQLite_Query_Callback(void *arg4, int argc, char **argv, char **azColName)
{
  int i;
  cJSON *json_array = (cJSON*)arg4;
  cJSON *json_record;

  if( !json_array) return 0;

  json_record = cJSON_CreateObject();
  for(i = 0; i < argc; i++)
  {
    cJSON_AddStringToObject(json_record, azColName[i], (argv[i])?argv[i]:"NULL");
  }
  cJSON_AddItemToArray(json_array, json_record);
  return 0; 
}

CSQLite::CSQLite()
{
  m_db_file[0] = 0;   
  m_db = NULL;
}

CSQLite::CSQLite(const char *filename)
{
  strncpy(m_db_file, filename, FILENAME_MAX);
  m_db_file[FILENAME_MAX] = 0;
  m_db = NULL;
}

CSQLite::~CSQLite()
{
  Close();
}

int CSQLite::Open( void )
{
  return sqlite3_open(m_db_file, &m_db);
}

int CSQLite::Open(const char *filename)
{
  strncpy(m_db_file, filename, FILENAME_MAX);
  m_db_file[FILENAME_MAX] = 0;
  return this->Open();
}

void CSQLite::Close( void )
{
  if(m_db)
  {
    sqlite3_close(m_db);
  }
}

int CSQLite::Query(cJSON *json_array, const char *query_fmt, ...)
{
  int rc;
  char query[1024];
  va_list arg;
  char *msg;

  m_last_error_text[0] = 0;

  va_start(arg, query_fmt);
  vsprintf(query, query_fmt, arg);
  va_end(arg);

#ifdef __DEBUG__
    syslog(LOG_DEBUG, "QUERY: [%s]", query);
#endif  

  rc = sqlite3_exec(m_db, query, CSQLite_Query_Callback, json_array, &msg);
  if(rc == SQLITE_OK)
  {
    if(query[0] != 's' && query[0] != 'S')
    {
      rc = sqlite3_changes(m_db);
    }
    else
    {
      rc = 0;
    }
  }
  else
  {
    strncpy(m_last_error_text, msg, CSQLITE_MAX_ERROR_TEXT);
    sqlite3_free(msg);
    rc = (-1);
  }
  return rc;
}
