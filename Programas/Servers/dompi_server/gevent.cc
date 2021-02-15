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
/*

instalar libcjson-dev (https://github.com/DaveGamble/cJSON)
#include <cjson/cJSON.h>
-lcjson

char *p;
cJSON *obj;
cJSON *str;
cJSON *num;
cJSON *arr;
obj = cJSON_CreateObject();


-- String ---------------------------------------------------------------------
obj = cJSON_Parse(const char *value);

-- String ---------------------------------------------------------------------
str = cJSON_CreateString("un string");
cJSON_AddItemToObject(obj, "nombre", str);
o
cJSON_AddStringToObject(obj, "nombre", "un string")

-- Number ---------------------------------------------------------------------
num = cJSON_CreateNumber(50);
cJSON_AddItemToObject(obj, "edad", num);
o
cJSON_AddNumberToObject(obj, edad, 50);

-- Array ----------------------------------------------------------------------
arr = cJSON_CreateArray();
cJSON_AddItemToArray(arr, obj);
cJSON_AddItemToObject(obj, "nombre_array", arr);
o
arr = cJSON_AddArrayToObject(obj, "nombre_arr");

-- Generado -------------------------------------------------------------------
p = cJSON_Print(obj);
p = cJSON_PrintUnformatted(obj)
cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format);

-- Obtener --------------------------------------------------------------------
un_obj = cJSON_GetObjectItemCaseSensitive(obj, "nombre");
un_obj = cJSON_GetObjectItemCaseSensitive(obj, "edad");
    cJSON_IsString(un_obj)
    name->valuestring

    cJSON_IsNumber
    width->valuedouble

-- Free -----------------------------------------------------------------------
cJSON_Delete(obj);

*/


#include "gevent.h"

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

#ifdef __DEBUG__
#include <syslog.h>
#endif

#include "cjson.h"

GEvent::GEvent(CSQLite *pDB)
{
    m_pDB = pDB;
}

GEvent::~GEvent()
{

}

int GEvent::ExtIOEvent(const char* json_evt)
{
    char hw_id[16];
    char status_a[8];
    char status_b[8];
    char delta_a[8];
    char delta_b[8];
    char remote_addr[16];
    time_t t;
    cJSON *json_obj;
    cJSON *json_hw_id;
    cJSON *json_status_a;
    cJSON *json_status_b;
    cJSON *json_delta_a;
    cJSON *json_delta_b;
    cJSON *json_raddr;

    status_a[0] = 0;
    status_b[0] = 0;
    delta_a[0] = 0;
    delta_b[0] = 0;

#ifdef __DEBUG__
    syslog(LOG_DEBUG, "GEvent::ExtIOEvent");
#endif  

    json_obj = cJSON_Parse(json_evt);
    if(json_obj)
    {
        json_hw_id = cJSON_GetObjectItemCaseSensitive(json_obj, "HW_ID");
        if(json_hw_id)
        {
            t = time(&t);
            json_status_a = cJSON_GetObjectItemCaseSensitive(json_obj, "STATUS_PORTA");
            json_status_b = cJSON_GetObjectItemCaseSensitive(json_obj, "STATUS_PORTB");
            json_delta_a = cJSON_GetObjectItemCaseSensitive(json_obj, "DELTA_PORTA");
            json_delta_b = cJSON_GetObjectItemCaseSensitive(json_obj, "DELTA_PORTB");
            json_raddr = cJSON_GetObjectItemCaseSensitive(json_obj, "REMOTE_ADDR");
            if(json_hw_id && cJSON_IsString(json_hw_id))
            {
                strcpy(hw_id, json_hw_id->valuestring);
            }
            if(json_status_a && cJSON_IsString(json_status_a))
            {
                strcpy(status_a, json_status_a->valuestring);
            }
            if(json_status_b && cJSON_IsString(json_status_b))
            {
                strcpy(status_b, json_status_b->valuestring);
            }
            if(json_delta_a && cJSON_IsString(json_delta_a))
            {
                strcpy(delta_a, json_delta_a->valuestring);
            }
            if(json_delta_b && cJSON_IsString(json_delta_b))
            {
                strcpy(delta_b, json_delta_b->valuestring);
            }
            if(json_raddr && cJSON_IsString(json_raddr))
            {
                strcpy(remote_addr, json_raddr->valuestring);
            }

            return m_pDB->Query(NULL, "UPDATE TB_DOM_PERIF "
                                    "SET last_ok = %lu, ip_address = \"%s\" "
                                    "WHERE hw_id = \"%s\";",
                                    t, remote_addr, hw_id);







            if(json_hw_id) cJSON_Delete(json_obj);
            if(json_status_a) cJSON_Delete(json_obj);
            if(json_status_b) cJSON_Delete(json_obj);
            if(json_delta_a) cJSON_Delete(json_obj);
            if(json_delta_b) cJSON_Delete(json_obj);
            if(remote_addr) cJSON_Delete(json_obj);
        }
        cJSON_Delete(json_obj);
    }
    return (-1);
}
