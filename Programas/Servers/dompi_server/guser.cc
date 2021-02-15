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
#include "guser.h"

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

#ifdef __DEBUG__
#include <syslog.h>
#endif

GUser::GUser(CSQLite *pDB)
{
    m_pDB = pDB;
}

GUser::~GUser()
{

}

int GUser::List(user_data *filter, user_list *list)
{
    int rc;
    sql_result *q = NULL, *p;

    user_list *l;

    rc = m_pDB->Query(&q, "SELECT * FROM TB_DOM_USER;");
    if(rc == 0)
    {
        p = q;
        l = list;
        while(p)
        {
            rc++;
            while(p->data)
            {
                if(p->data->value)
                {
                    if( !strcmp(p->data->name, "user_id"))
                    {
                        strcpy(l->data.id, p->data->value);
                    }
                    else if( !strcmp(p->data->name, "name"))
                    {
                        strcpy(l->data.name, p->data->value);
                    }
                    else if( !strcmp(p->data->name, "pin_keypad"))
                    {
                        strcpy(l->data.pin_keypad, p->data->value);
                    }
                    else if( !strcmp(p->data->name, "pin_sms"))
                    {
                        strcpy(l->data.pin_sms, p->data->value);
                    }
                    else if( !strcmp(p->data->name, "pin_web"))
                    {
                        strcpy(l->data.pin_web, p->data->value);
                    }
                    else if( !strcmp(p->data->name, "phone_call"))
                    {
                        strcpy(l->data.phone_call, p->data->value);
                    }
                    else if( !strcmp(p->data->name, "phone_sms"))
                    {
                        strcpy(l->data.phone_sms, p->data->value);
                    }
                    else if( !strcmp(p->data->name, "email"))
                    {
                        strcpy(l->data.email, p->data->value);
                    }
                    else if( !strcmp(p->data->name, "access_mask"))
                    {
                        strcpy(l->data.access_mask, p->data->value);
                    }
                    else if( !strcmp(p->data->name, "days_of_week"))
                    {
                        strcpy(l->data.days_of_week, p->data->value);
                    }
                    else if( !strcmp(p->data->name, "hours_of_day"))
                    {
                        strcpy(l->data.hours_of_day, p->data->value);
                    }
                    else if( !strcmp(p->data->name, "access_error_count"))
                    {
                        l->data.access_error_count = atoi(p->data->value);
                    }
                    else if( !strcmp(p->data->name, "last_access_error"))
                    {
                        l->data.last_access_error = atol(p->data->value);
                    }
                    else if( !strcmp(p->data->name, "last_access_ok"))
                    {
                        l->data.last_access_ok = atol(p->data->value);
                    }
                    else if( !strcmp(p->data->name, "user_flags"))
                    {
                        l->data.user_flags = atoi(p->data->value);
                    }
                }
                p->data = p->data->next;
            }
            p = p->next;
            if(p && p->data)
            {
                l->next = (user_list*)calloc(1, sizeof(user_list));
                l = l->next;
            }
        }
        m_pDB->FreeResult(q);
        return rc;
    }
    else
    {
        return (-1);
    }
    
}



int GUser::Add(user_data *data)
{

}

int GUser::Remove(user_data *data)
{

}

int GUser::Update(user_data *data)
{

}

void GUser::FreeList(user_list *p)
{
    if(p)
    {
        if(p->next) FreeList(p->next);
        free(p);
    }
}
