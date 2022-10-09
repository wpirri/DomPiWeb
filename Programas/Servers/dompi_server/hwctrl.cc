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
#include "hwctrl.h"

#include <string>
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
using namespace std;

#include <unistd.h>
#include <sys/msg.h>
#include <time.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <math.h>

HWCtrl::HWCtrl(CDB *pDB, CGMServerWait *pServer)
{
    m_pDB = pDB;
    m_pServer = pServer;
}

HWCtrl::~HWCtrl()
{

}

int HWCtrl::GetHWId(const char* mac)
{

    return 0;
}

int HWCtrl::GetHWIP(const char* mac, char *ip)
{

    return 0;
}

int HWCtrl::GetHWStatus(const char* mac, char *ip)
{

    return 0;
}

int HWCtrl::SetHWStatus(const char* mac, char *ip)
{

    return 0;
}

int HWCtrl::GetHWConf(const char* mac, char *ip)
{

    return 0;
}

int HWCtrl::SetHWConf(const char* mac, char *ip)
{

    return 0;
}

int HWCtrl::SetObject(int id)
{

    return 0;
}

int HWCtrl::SetObject(const char* name)
{

    return 0;
}

int HWCtrl::ResetObject(int id)
{

    return 0;
}

int HWCtrl::ResetObject(const char* name)
{

    return 0;
}

int HWCtrl::SwitchObject(int id)
{

    return 0;
}

int HWCtrl::SwitchObject(const char* name)
{

    return 0;
}
