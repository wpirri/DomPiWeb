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
Informaciòn de desarrollo sobre Raspberry Pi

https://pinout.xyz/
http://diymakers.es/usando-el-puerto-gpio/
http://wiringpi.com/reference/


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

#include <string>
#include <iostream>
#include <csignal>
using namespace std;

#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

#include "csqlite.h"



int main(/*int argc, char** argv, char** env*/void)
{
    int rc;
    CSQLite *pDB;
    sql_result *q = NULL, *p;
    cJSON *json_obj;
	cJSON *json_reg;
    cJSON *json_arr = NULL;

    pDB = new CSQLite("/var/lib/DomPiWeb/.DomPiWebDB.sqll3");

    pDB->Open();

    /*
    Ej.
    { "departamento":8,
    "nombredepto":"Ventas",
    "director": "Juan Rodríguez",
    "empleados":[ { "nombre":"Pedro", "apellido":"Fernández" },
                    { "nombre":"Jacinto", "apellido":"Benavente" } ]
    }
    */
    rc = pDB->Query(&q, "SELECT * FROM TB_DOM_USER;");
    if(rc == 0)
    {
        p = q;
        json_arr = cJSON_CreateArray();
        while(p && p->data)
        {
            rc++;
            json_reg = cJSON_CreateObject();
            while(p->data && p->data->name)
            {
                cJSON_AddStringToObject(json_reg, p->data->name, p->data->value);
                p->data = p->data->next;
            }
            cJSON_AddItemToArray(json_arr, json_reg);
            p = p->next;
        }
    }
    if(json_arr)
    {
        json_obj = cJSON_CreateObject();
        cJSON_AddItemToObject(json_obj, "usuarios", json_arr);
        printf("JSON: |%s|\n", cJSON_PrintUnformatted(json_obj));
        cJSON_Delete(json_obj);
    }
    pDB->FreeResult(q);
    return 0;
}
