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
#ifndef _CSQLITE_H_
#define _CSQLITE_H_

#include <cstdio>

#include <sqlite3.h>

#define CSQLITE_MAX_ERROR_TEXT 256

typedef struct _sql_result_data
{
    char *name;
    char *value;
    struct _sql_result_data *next;
} sql_result_data;

typedef struct _sql_result
{
    sql_result_data *data;
    struct _sql_result *next;
} sql_result;

class CSQLite
{
public:
    CSQLite();
    CSQLite(const char *filename);
    virtual ~CSQLite();

    int Open( void );
    int Open(const char *filename);
    void Close( void );

    int Query(sql_result **query_rslt, const char *query_fmt, ...);

    void FreeResult(sql_result *query_rslt);

    char m_last_error_text[CSQLITE_MAX_ERROR_TEXT+1];
private:
    sqlite3 *m_db;
    char m_db_file[FILENAME_MAX+1];

    void FreeResultData(sql_result_data *rslt_data);

};
#endif /* _CSQLITE_H_ */
