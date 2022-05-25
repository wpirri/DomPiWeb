/***************************************************************************
  Copyright (C) 2022   Walter Pirri
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
#ifndef _HWCTRL_H_
#define _HWCTRL_H_

#include "csqlite.h"
#include <gmonitor/gmswaited.h>

class HWCtrl
{
public:
    HWCtrl(CSQLite *pDB, CGMServerWait *pServer);
    virtual ~HWCtrl();

    int GetHWId(const char* mac);
    int GetHWIP(const char* mac, char *ip);
    int GetHWStatus(const char* mac, char *ip);
    int SetHWStatus(const char* mac, char *ip);
    int GetHWConf(const char* mac, char *ip);
    int SetHWConf(const char* mac, char *ip);
    int SetObject(int id);
    int SetObject(const char* name);
    int ResetObject(int id);
    int ResetObject(const char* name);
    int SwitchObject(int id);
    int SwitchObject(const char* name);

private:
    CSQLite *m_pDB;
    CGMServerWait *m_pServer;

};

#endif /* _HWCTRL_H_ */
