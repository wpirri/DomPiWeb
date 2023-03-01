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
#ifndef _CDB_H_
#define _CDB_H_

/*
#include "csqlite.h"

class CDB : public CSQLite
{
    public:
        CDB(); 
        CDB(const char *filename) : CSQLite { filename } {}

};
*/

#include "cmydb.h"

class CDB : public CMyDB
{
    public:
        CDB(); 
        CDB(const char* host, const char* dbname, const char* username, const char* userpass) : CMyDB { host,dbname,username,userpass } {}
};


#endif /* _CDB_H_ */
