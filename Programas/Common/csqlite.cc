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

/*
void CSQLite_Dump( sql_result *r )
{
  sql_result_data *d;
  printf("CSQLite::Dump() - Begin\n");
  while(r)
  {
    printf("CSQLite::Dump() -> sql_result\n");
    d = r->data;
    while(d)
    {
      printf("CSQLite::Dump() -> sql_result_data\n");
      printf("CSQLite::Dump() -> query_rslt->data->name : %s\n", (d->name)?d->name:"NULL");
      printf("CSQLite::Dump() -> query_rslt->data->value: %s\n", (d->value)?d->value:"NULL");
      d = d->next;
    }
    r = r->next;
  }
  printf("CSQLite::Dump() - End\n");
}
*/

int CSQLite_Query_Callback(void *rslt, int argc, char **argv, char **azColName)
{
  int i;
  sql_result *r;
  sql_result_data *d, *e;

  if( !rslt) return 0;

  r = (sql_result*)rslt;

  while(r->next) r = r->next;

  r->data = (sql_result_data*)calloc(1, sizeof(sql_result_data));

  d = r->data;
  e = NULL;

  for(i = 0; i < argc; i++)
  {
    if(azColName[i])
    {
      d->name = (char*)calloc(strlen(azColName[i])+1, sizeof(char));
      strcpy(d->name, azColName[i]);
      if(argv[i])
      {
        d->value = (char*)calloc(strlen(argv[i])+1, sizeof(char));
        strcpy(d->value, argv[i]);
      }
    }
    e = d;  /* me guardo el puntero anterior */
    d->next = (sql_result_data*)calloc(1, sizeof(sql_result_data));
    d = d->next;
  }
  if(e && e->next)
  {
    /* Libero el último next data no usado */
    free(e->next);
    e->next = NULL;
  }
  /* asigno el proximo next result */
  r->next = (sql_result*)calloc(1, sizeof(sql_result));

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

int CSQLite::Query(sql_result **query_rslt, const char *query_fmt, ...)
{
  int rc;
  char query[1024];
  va_list arg;
  char *msg;

  m_last_error_text[0] = 0;

  va_start(arg, query_fmt);
  vsprintf(query, query_fmt, arg);
  va_end(arg);

  if(query_rslt)
  {
    (*query_rslt) = (sql_result*)calloc(1, sizeof(sql_result));
  }

  rc = sqlite3_exec(m_db, query, CSQLite_Query_Callback, (query_rslt)?(*query_rslt):NULL, &msg);
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
    if(query_rslt)
    {
      FreeResult(*query_rslt);
      (*query_rslt) = NULL;
    }
    strncpy(m_last_error_text, msg, CSQLITE_MAX_ERROR_TEXT);
    sqlite3_free(msg);
    rc = (-1);
  }
  return rc;
}

void CSQLite::FreeResult(sql_result *query_rslt)
{
  if(query_rslt)
  {
    if(query_rslt->next) FreeResult(query_rslt->next);
    if(query_rslt->data) FreeResultData(query_rslt->data);
    free(query_rslt);
  }
}

void CSQLite::FreeResultData(sql_result_data *rslt_data)
{
  if(rslt_data)
  {
    FreeResultData(rslt_data->next);
    if(rslt_data->name)  free(rslt_data->name);
    if(rslt_data->value) free(rslt_data->value);
    free(rslt_data);
  }
}
