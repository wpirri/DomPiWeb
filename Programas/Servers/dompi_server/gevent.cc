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
#include <math.h>

GEvent::GEvent(CSQLite *pDB, CGMServerWait *pServer)
{
    m_pDB = pDB;
    m_pServer = pServer;
}

GEvent::~GEvent()
{

}

int GEvent::ExtIOEvent(const char* json_evt)
{
    int i;
    int mask;
    char hw_id[16];
    int status_a;
    int status_b;
    int status_c;
    int delta_a;
    int delta_b;
    int delta_c;
    char remote_addr[16];
    time_t t;
    struct tm *p_tm;
    int rc;
    char sql_set[4096];
    cJSON *json_obj;
    cJSON *json_hw_id;
    cJSON *json_status_a;
    cJSON *json_status_b;
    cJSON *json_status_c;
    cJSON *json_delta_a;
    cJSON *json_delta_b;
    cJSON *json_delta_c;
    cJSON *json_raddr;

    status_a = (-1);
    status_b = (-1);
    status_c = (-1);
    delta_a = (-1);
    delta_b = (-1);
    delta_c = (-1);
    sql_set[0] = 0;

    rc = 0;

    json_obj = cJSON_Parse(json_evt);
    if(json_obj)
    {
        json_hw_id = cJSON_GetObjectItemCaseSensitive(json_obj, "HW_ID");
        if(json_hw_id)
        {
            t = time(&t);
            p_tm = localtime(&t);
            json_status_a = cJSON_GetObjectItemCaseSensitive(json_obj, "STATUS_PORTA");
            json_status_b = cJSON_GetObjectItemCaseSensitive(json_obj, "STATUS_PORTB");
            json_status_c = cJSON_GetObjectItemCaseSensitive(json_obj, "STATUS_PORTC");
            json_delta_a = cJSON_GetObjectItemCaseSensitive(json_obj, "DELTA_PORTA");
            json_delta_b = cJSON_GetObjectItemCaseSensitive(json_obj, "DELTA_PORTB");
            json_delta_c = cJSON_GetObjectItemCaseSensitive(json_obj, "DELTA_PORTC");
            json_raddr = cJSON_GetObjectItemCaseSensitive(json_obj, "REMOTE_ADDR");
            if(json_hw_id && cJSON_IsString(json_hw_id))
            {
                strcpy(hw_id, json_hw_id->valuestring);
            }
            if(json_status_a && cJSON_IsString(json_status_a))
            {
                status_a = atoi(json_status_a->valuestring);
                strcat(sql_set, ",Estado_PORT_A=\"");
                strcat(sql_set, json_status_a->valuestring);
                strcat(sql_set, "\"");
            }
            if(json_status_b && cJSON_IsString(json_status_b))
            {
                status_b = atoi(json_status_b->valuestring);
                strcat(sql_set, ",Estado_PORT_B=\"");
                strcat(sql_set, json_status_b->valuestring);
                strcat(sql_set, "\"");
            }
            if(json_status_c && cJSON_IsString(json_status_c))
            {
                status_b = atoi(json_status_b->valuestring);
                strcat(sql_set, ",Estado_PORT_C=\"");
                strcat(sql_set, json_status_b->valuestring);
                strcat(sql_set, "\"");
            }
            if(json_delta_a && cJSON_IsString(json_delta_a))
            {
                delta_a = atoi(json_delta_a->valuestring);
            }
            if(json_delta_b && cJSON_IsString(json_delta_b))
            {
                delta_b = atoi(json_delta_b->valuestring);
            }
            if(json_delta_c && cJSON_IsString(json_delta_c))
            {
                delta_c = atoi(json_delta_c->valuestring);
            }
            if(json_raddr && cJSON_IsString(json_raddr))
            {
                strcpy(remote_addr, json_raddr->valuestring);
            }

            m_pServer->m_pLog->Add(10, "Reporte de HW: %s", hw_id);
            rc = m_pDB->Query(NULL, "UPDATE TB_DOM_PERIF "
                                "SET Ultimo_Ok  = \"%04i-%02i-%02i %02i:%02i:%02i\", "
                                  "Direccion_IP = \"%s\""
                                  "%s "
                                "WHERE Id = \"%s\";",
                                p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
                                p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec, 
                                remote_addr,
                                sql_set,
                                hw_id);
            if(delta_a >= 0)
            {
                /* Para los bits 0 a 15 */
                for(i = 0; i < 16; i++)
                {
                    mask = pow(2, i);   /* Armo la mascara */
                    if(delta_a & mask)  /* Si cambió .... */
                    {
                        /* Busco si hay evento */
                        CheckEvent(hw_id, 1 /* PORT A */, i, (status_a & mask)?1:0);
                    }
                }
            }
            if(delta_b >= 0)
            {
                /* Para los bits 0 a 15 */
                for(i = 0; i < 16; i++)
                {
                    mask = pow(2, i);   /* Armo la mascara */
                    if(delta_b & mask)  /* Si cambió .... */
                    {
                        /* Busco si hay evento */
                        CheckEvent(hw_id, 2 /* PORT B */, i, (status_b & mask)?1:0);
                    }
                }
            }
            if(delta_c >= 0)
            {
                /* Para los bits 0 a 15 */
                for(i = 0; i < 16; i++)
                {
                    mask = pow(2, i);   /* Armo la mascara */
                    if(delta_b & mask)  /* Si cambió .... */
                    {
                        /* Busco si hay evento */
                        CheckEvent(hw_id, 3 /* PORT C */, i, (status_c & mask)?1:0);
                    }
                }
            }






        }
        cJSON_Delete(json_obj);
    }
    return rc;
}

int GEvent::CheckEvent(const char *hw_id, int port, int e_s, int estado)
{
    cJSON *json_arr;
    cJSON *json_obj;
	char query[4096];
    int rc;

    m_pServer->m_pLog->Add(10, "Cambio de estado - CheckEvent: HW: %s Port: %s E/S: %i Estado: %s", 
                                hw_id, (port==1)?"A":(port==2)?"B":(port==3)?"C":"?",
                                e_s, (estado)?"ON":"OFF");
    /* Busco si hay un assign */

    /* Busco si hay evento para ese assign */

    json_arr = cJSON_CreateArray();
    sprintf(query, "SELECT EV.Evento, EV.Objeto_Destino, EV.Grupo_Destino, EV.Funcion_Destino, EV.Variable_Destino "
                    "FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS, TB_DOM_EVENT AS EV "
                    "WHERE EV.Objeto_Origen = ASS.Id AND ASS.Dispositivo = HW.Id AND "
                    "HW.Id = \'%s\' AND ASS.Port = %i AND ASS.E_S = %i AND %s;",
                    hw_id, port, e_s, (estado)?"OFF_a_ON = 1":"ON_a_OFF = 1");
    m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
    rc = m_pDB->Query(json_arr, query);
    if(rc == 0)
    {
        json_obj = cJSON_CreateObject();
        cJSON_AddItemToObject(json_obj, "response", json_arr);
        cJSON_PrintPreallocated(json_obj, query, 4095, 0);
        cJSON_Delete(json_obj);



        m_pServer->m_pLog->Add(10, "Eventos: %s", query); 




    }
    else
    {
        cJSON_Delete(json_arr);
    }
    return 0;
}
