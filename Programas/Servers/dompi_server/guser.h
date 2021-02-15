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
#ifndef _GUSER_H_
#define _GUSER_H_

#include "csqlite.h"

typedef struct _user_data
{
    char id[16];
    char name[64];
    char pin_keypad[32];
    char pin_sms[32];
    char pin_web[32];
    char phone_call[32];
    char phone_sms[32];
    char email[64];
    char access_mask[128];
    char days_of_week[8];
    char hours_of_day[32];
    int access_error_count;
    long last_access_ok;
    long last_access_error;
    int user_flags;
} user_data;

typedef struct _user_list
{
    user_data data;
    struct _user_list *next;
} user_list;

class GUser
{
public:
    GUser(CSQLite *pDB);
    virtual ~GUser();

    int List(user_data *filter, user_list *list);
    int Add(user_data *data);
    int Remove(user_data *data);
    int Update(user_data *data);

    void FreeList(user_list *p);
private:
    CSQLite *m_pDB;

};
#endif /* _GEVENT_H_ */
