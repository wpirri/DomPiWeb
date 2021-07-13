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
#ifndef _GEVENT_H_
#define _GEVENT_H_

#include "csqlite.h"
#include <gmonitor/gmswaited.h>

class GEvent
{
public:
    GEvent(CSQLite *pDB, CGMServerWait *pServer);
    virtual ~GEvent();

    int ExtIOEvent(const char* json_evt);

    int CheckEvent(const char *hw_id, int port, int e_s, int estado);

    int SendEventObj(int id, int ev, int val);
    int SendEventGrp(int id, int ev, int val);
    int SendEventFun(int id, int ev, int val);
    int SendEventVar(int id, int ev, int val);

private:
    CSQLite *m_pDB;
    CGMServerWait *m_pServer;

};
#endif /* _GEVENT_H_ */
