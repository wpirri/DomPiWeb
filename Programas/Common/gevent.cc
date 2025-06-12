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

#include "strfunc.h"

char *numcpy(char* dst, const char *src)
{
    if(!src || !dst) return nullptr;
    while(*src)
    {
        if(*src >= '0' && *src <= '9')
        {
            *dst = *src;
            dst++;
        }
        src++;
    }
    *dst = 0;
    return dst;
}

GEvent::GEvent(CDB *pDB, CGMServerWait *pServer)
{
    m_pDB = pDB;
    m_pServer = pServer;
    m_last_time_task = time((time_t*)&m_last_time_task);
    m_alarm_need_update = 0;
}

GEvent::~GEvent()
{

}

/*
Devuelve (en caso de cambios realizados como efecto de lo informado en el campo CHG):
    0   Si no hubo cambios
    1   Si cambió un assign
    2   Si cambió un grupo
    3   Si cambió una particion de alarma
    4   Si llamó una función
    5   Si cambió una variable
    (-1) Si no se encuentra el dispositivo
*/

int GEvent::ExtIOEvent(const char* json_evt)
{
    int i;
    char s[256];
    time_t t;
    int rc;
    unsigned int ival;
    char query[4096];
    cJSON *json_obj;
    cJSON *json_QueryArray;
    cJSON *json_QueryRow;
    cJSON *json_hw_id;
    cJSON *json_hw_mac;
    cJSON *json_raddr;
    cJSON *json_chg;
    cJSON *json_status;
    cJSON *json_un_obj;
    cJSON *json_tmp;
    STRFunc str;
    char extra_info[1024];

    m_pServer->m_pLog->Add(100, "[GEvent::ExtIOEvent] json_evt: %s", json_evt);

    json_obj = cJSON_Parse(json_evt);
    if(json_obj)
    {
        json_hw_mac = cJSON_GetObjectItemCaseSensitive(json_obj, "ID");
        json_raddr = cJSON_GetObjectItemCaseSensitive(json_obj, "REMOTE_ADDR");
        if(json_hw_mac && cJSON_IsString(json_hw_mac))
        {
            t = time(&t);

            /* Busco el ID para relacionar con la tabla de assigns */
            sprintf(query, "SELECT Id, Estado FROM TB_DOM_PERIF WHERE UPPER(MAC) = UPPER(\'%s\');", json_hw_mac->valuestring);
            m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
            json_QueryArray = cJSON_CreateArray();
            rc = m_pDB->Query(json_QueryArray, query);
            m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
            if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
            if(rc > 0)
            {
                cJSON_ArrayForEach(json_QueryRow, json_QueryArray)
                {
                    json_hw_id = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Id");
                    json_status = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Estado");
                    if(json_raddr)
                    {
                        if(atoi(json_status->valuestring) == 0)
                        {
                            m_pServer->m_pLog->Add(10, "[HW] %s %s Estado: ON LINE", json_hw_mac->valuestring, json_raddr->valuestring);
                        }
                        /* Me fijo si tiene info de FW, HW, etc. */
                        extra_info[0] = 0;
                        json_tmp = cJSON_GetObjectItemCaseSensitive(json_obj, "HW");
                        if(json_tmp)
                        {
                            strcat(extra_info, "HW: ");
                            strcat(extra_info, json_tmp->valuestring);
                            strcat(extra_info, "\n");
                        }
                        json_tmp = cJSON_GetObjectItemCaseSensitive(json_obj, "SO");
                        if(json_tmp)
                        {
                            strcat(extra_info, "SO: ");
                            strcat(extra_info, json_tmp->valuestring);
                            strcat(extra_info, "\n");
                        }
                        json_tmp = cJSON_GetObjectItemCaseSensitive(json_obj, "FW");
                        if(json_tmp)
                        {
                            strcat(extra_info, "FW: ");
                            strcat(extra_info, json_tmp->valuestring);
                            strcat(extra_info, "\n");
                        }
                        json_tmp = cJSON_GetObjectItemCaseSensitive(json_obj, "SDK");
                        if(json_tmp)
                        {
                            strcat(extra_info, "SDK: ");
                            strcat(extra_info, json_tmp->valuestring);
                            strcat(extra_info, "\n");
                        }
                        json_tmp = cJSON_GetObjectItemCaseSensitive(json_obj, "AT");
                        if(json_tmp)
                        {
                            strcat(extra_info, "AT: ");
                            strcat(extra_info, json_tmp->valuestring);
                            strcat(extra_info, "\n");
                        }
                        json_tmp = cJSON_GetObjectItemCaseSensitive(json_obj, "SSL");
                        if(json_tmp)
                        {
                            strcat(extra_info, "SSL: ");
                            strcat(extra_info, json_tmp->valuestring);
                            strcat(extra_info, "\n");
                        }
                        /* Actualizo la tabla de Dispositivos */
                        sprintf(query, "UPDATE TB_DOM_PERIF "
                                            "SET Ultimo_Ok = %lu, "
                                            "Direccion_IP = \'%s\', "
                                            "Informacion  = \'%s\', "
                                            "Estado = 1 "
                                            "WHERE UPPER(MAC) = UPPER(\'%s\');",
                                            t,
                                            json_raddr->valuestring,
                                            extra_info,
                                            json_hw_mac->valuestring);
                        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                        rc = m_pDB->Query(NULL, query);
                        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                    }
                }
                /* Actualizo los assign que vengan  en el mensaje */
                json_un_obj = json_obj;
                while( json_un_obj )
                {
                    /* Voy hasta el elemento con datos */
                    if(json_un_obj->type == cJSON_Object)
                    {
                        json_un_obj = json_un_obj->child;
                    }
                    else
                    {
                        if(json_un_obj->type == cJSON_String)
                        {
                            if(json_un_obj->string && json_un_obj->valuestring)
                            {
                                if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
                                {
                                    if( !memcmp(json_un_obj->string, "IO", 2) || !memcmp(json_un_obj->string, "OUT", 3) )
                                    {
                                        ival = atoi(json_un_obj->valuestring);

                                        /* Actualizo el estado del HW */
                                        sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Estado_HW = %i "
                                                        "WHERE Dispositivo = \'%s\' AND Port = \'%s\';",
                                                        ival,
                                                        json_hw_id->valuestring,
                                                        json_un_obj->string);
                                        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                                        rc = m_pDB->Query(NULL, query);
                                        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                                        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                                        /* En las entradas actualizo también el estado a mostrar */
                                        if(!memcmp(json_un_obj->string, "IO", 2))
                                        {
                                            sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Estado = %i "
                                                            "WHERE Dispositivo = \'%s\' AND Port = \'%s\' AND "
                                                            "(Tipo = 1 OR Tipo = 4);",
                                                            ival,
                                                            json_hw_id->valuestring,
                                                            json_un_obj->string);
                                            m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                                            rc = m_pDB->Query(NULL, query);
                                            m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                                            if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                                            /* Si es entrada analogica completo el dato en perifdata */
                                            sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Estado = %i, Perif_Data = \'%i\' "
                                                            "WHERE Dispositivo = \'%s\' AND Port = \'%s\' AND Tipo = 2;",
                                                            ival, ival,
                                                            json_hw_id->valuestring,
                                                            json_un_obj->string);
                                            m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                                            rc = m_pDB->Query(NULL, query);
                                            m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                                            if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                                            /* Si existe un assign me fijo si hay alguna automatización */
                                            if(rc > 0)
                                            {
                                                CheckAuto(atoi(json_hw_id->valuestring), json_un_obj->string, ival);
                                            }
                                        }
                                    }
                                    else if( !memcmp(json_un_obj->string, "CARD", 4))
                                    {
                                        sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Perif_Data = \'%s\' "
                                                        "WHERE Dispositivo = \'%s\' AND Port = \'CARD\' AND "
                                                        "Tipo = 6;",
                                                        json_un_obj->valuestring,
                                                        json_hw_id->valuestring);
                                        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                                        rc = m_pDB->Query(NULL, query);
                                        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                                        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                                    }
                                    else if( !memcmp(json_un_obj->string, "TEMP", 4))
                                    {
                                        ival = atoi(numcpy(s, json_un_obj->valuestring));

                                        sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Perif_Data = \'%s\', Estado = %i "
                                                        "WHERE Dispositivo = \'%s\' AND Port = \'TEMP\' AND "
                                                        "Tipo = 6;",
                                                        json_un_obj->valuestring, ival,
                                                        json_hw_id->valuestring);
                                        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                                        rc = m_pDB->Query(NULL, query);
                                        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                                        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                                        if(rc > 0)
                                        {
                                            CheckAuto(atoi(json_hw_id->valuestring), json_un_obj->string, ival);
                                        }
                                    }
                                    else if( !memcmp(json_un_obj->string, "HUM", 3))
                                    {
                                        ival = atoi(numcpy(s, json_un_obj->valuestring));

                                        sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Perif_Data = \'%s\', Estado = %i "
                                                        "WHERE Dispositivo = \'%s\' AND Port = \'HUM\' AND "
                                                        "Tipo = 6;",
                                                        json_un_obj->valuestring, ival,
                                                        json_hw_id->valuestring);
                                        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                                        rc = m_pDB->Query(NULL, query);
                                        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                                        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                                        if(rc > 0)
                                        {
                                            CheckAuto(atoi(json_hw_id->valuestring), json_un_obj->string, ival);
                                        }
                                    }
                                }
                            }
                        }
                        json_un_obj = json_un_obj->next;
                    }
                }

                /* Controlo eventos con los cambios informados */
                json_chg = cJSON_GetObjectItemCaseSensitive(json_obj, "CHG");
                if(json_chg)
                {
                    m_pServer->m_pLog->Add(50, "[ExtIOEvent] El mensaje informa cabios de estado.");
                    /* json_chg->valuestring: lista separada por comas de cambios  */
                    i = 0;
                    while(str.Section(json_chg->valuestring, ',', i, s))
                    {
                        /* Me fijo el estado actual de ese puerto */
                        json_status = cJSON_GetObjectItemCaseSensitive(json_obj, s);
                        if(json_status)
                        {
                            CheckEvent(atoi(json_hw_id->valuestring), s, atoi(json_status->valuestring));
                        }
                        else
                        {
                            CheckEvent(atoi(json_hw_id->valuestring), s, 1);
                        }
                        i++;
                    }
                }
                else
                {
                    /* TODO: Si no se informaron cambios busco diferencia con la base */



                }

                /**/
                cJSON_Delete(json_QueryArray);
                cJSON_Delete(json_obj);
                return 1;
            }
            else
            {
                /* Desconocido o error */
                cJSON_Delete(json_QueryArray);
                cJSON_Delete(json_obj);
                return rc;
            }
        }
        cJSON_Delete(json_obj);
        return (-1);
    }
    else
    {
        /* No se pudo parsear el parámetro JSon */
        return (-1);
    }
}

int GEvent::SyncIO(const char* json_evt)
{
    char s[256];
    time_t t;
    int rc;
    unsigned int ival;
    char query[4096];
    cJSON *json_obj;
    cJSON *json_QueryArray;
    cJSON *json_hw_id;
    cJSON *json_hw_mac;
    cJSON *json_raddr;
    cJSON *json_status;
    cJSON *json_un_obj;
    STRFunc str;

    m_pServer->m_pLog->Add(100, "[GEvent::SyncIO] json_evt: %s", json_evt);

    json_obj = cJSON_Parse(json_evt);
    if(json_obj)
    {
        json_hw_mac = cJSON_GetObjectItemCaseSensitive(json_obj, "ID");
        json_raddr = cJSON_GetObjectItemCaseSensitive(json_obj, "REMOTE_ADDR");
        if(json_hw_mac && cJSON_IsString(json_hw_mac))
        {
            t = time(&t);

            /* Busco el ID para relacionar con la tabla de assigns */
            sprintf(query, "SELECT Id, Estado FROM TB_DOM_PERIF WHERE UPPER(MAC) = UPPER(\'%s\');", json_hw_mac->valuestring);
            m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
            json_QueryArray = cJSON_CreateArray();
            rc = m_pDB->Query(json_QueryArray, query);
            m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
            if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
            if(rc >= 0 && json_QueryArray->child)
            {
                json_hw_id = cJSON_GetObjectItemCaseSensitive(json_QueryArray->child, "Id");
                json_status = cJSON_GetObjectItemCaseSensitive(json_QueryArray->child, "Estado");
                if(json_raddr)
                {
                    if(atoi(json_status->valuestring) == 0)
                    {
                        m_pServer->m_pLog->Add(10, "[HW] %s %s Estado: ON LINE", json_hw_mac->valuestring, json_raddr->valuestring);
                    }
                    /* Actualizo la tabla de Dispositivos */
                    sprintf(query, "UPDATE TB_DOM_PERIF "
                                        "SET Ultimo_Ok = %lu, "
                                        "Direccion_IP = \'%s\', "
                                        "Estado = 1 "
                                        "WHERE UPPER(MAC) = UPPER(\'%s\');",
                                        t,
                                        json_raddr->valuestring,
                                        json_hw_mac->valuestring);
                    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                    rc = m_pDB->Query(NULL, query);
                    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                }
                /* Actualizo los assign que vengan  en el mensaje */
                json_un_obj = json_obj;
                while( json_un_obj )
                {
                    /* Voy hasta el elemento con datos */
                    if(json_un_obj->type == cJSON_Object)
                    {
                        json_un_obj = json_un_obj->child;
                    }
                    else
                    {
                        if(json_un_obj->type == cJSON_String)
                        {
                            if(json_un_obj->string && json_un_obj->valuestring)
                            {
                                if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
                                {
                                    if( !memcmp(json_un_obj->string, "IO", 2) || !memcmp(json_un_obj->string, "OUT", 3) )
                                    {
                                        ival = atoi(json_un_obj->valuestring);

                                        /* Actualizo el estado del HW */
                                        sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Estado = %i, Estado_HW = %i "
                                                        "WHERE Dispositivo = \'%s\' AND Port = \'%s\';",
                                                        ival, ival,
                                                        json_hw_id->valuestring,
                                                        json_un_obj->string);
                                        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                                        rc = m_pDB->Query(NULL, query);
                                        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                                        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                                    }
                                    else if( !memcmp(json_un_obj->string, "TEMP", 4))
                                    {
                                        ival = atoi(numcpy(s, json_un_obj->valuestring));

                                        sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Perif_Data = \'%s\', Estado = %i "
                                                        "WHERE Dispositivo = \'%s\' AND Port = \'TEMP\' AND "
                                                        "Tipo = 6;",
                                                        json_un_obj->valuestring, ival,
                                                        json_hw_id->valuestring);
                                        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                                        rc = m_pDB->Query(NULL, query);
                                        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                                        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                                    }
                                    else if( !memcmp(json_un_obj->string, "HUM", 3))
                                    {
                                        ival = atoi(numcpy(s, json_un_obj->valuestring));

                                        sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Perif_Data = \'%s\', Estado = %i "
                                                        "WHERE Dispositivo = \'%s\' AND Port = \'HUM\' AND "
                                                        "Tipo = 6;",
                                                        json_un_obj->valuestring, ival,
                                                        json_hw_id->valuestring);
                                        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                                        rc = m_pDB->Query(NULL, query);
                                        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                                        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                                    }
                                }
                            }
                        }
                        json_un_obj = json_un_obj->next;
                    }
                }

                /**/
                cJSON_Delete(json_QueryArray);
                cJSON_Delete(json_obj);
                return 1;
            }
            else
            {
                m_pServer->m_pLog->Add(10, "[HW] %s %s Desconocido", json_hw_mac->valuestring, (json_raddr)?json_raddr->valuestring:"-");
                cJSON_Delete(json_QueryArray);
                cJSON_Delete(json_obj);
                return 0;
            }
        }
        cJSON_Delete(json_obj);
        return 0;
    }
    return 0;
}

int GEvent::ChangeIO(const char* json_evt)
{
    int rc;
    char query[4096];
    cJSON *json_obj;
    cJSON *json_ASS_Id;
    cJSON *json_Estado;

    m_pServer->m_pLog->Add(100, "[GEvent::ChangeIO] json_evt: %s", json_evt);

    json_obj = cJSON_Parse(json_evt);
    if(json_obj)
    {
        json_ASS_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "ASS_Id");
        json_Estado = cJSON_GetObjectItemCaseSensitive(json_obj, "Estado");
        if(json_ASS_Id && json_Estado)
        {
            /* Actualizo el estado del HW */
            sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Estado = %s, Estado_HW = %s "
                            "WHERE Id = \'%s\';",
                            json_Estado->valuestring, json_Estado->valuestring,
                            json_ASS_Id->valuestring);
            m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
            rc = m_pDB->Query(NULL, query);
            m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
            if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
        }
        cJSON_Delete(json_obj);
    }
    return 0;
}

/*
Devuelve:
    0   Si no hubo cambios
    1   Si cambió un assign
    2   Si cambió un grupo
    3   Si cambió una particion de alarma
    4   Si llamó una función
    5   Si cambió una variable
*/
int GEvent::CheckEvent(int hw_id, const char* port, int estado)
{
	char query[4096];
    int rc;
    int cambios = 0;
    time_t time_now;
    cJSON *json_AssignArray;
    cJSON *json_AssignRow;
    cJSON *json_EventArray;
    cJSON *json_EventRow;
    cJSON *Assign_Id;
    cJSON *Assign_Nombre;
    cJSON *Assign_Tipo;
    cJSON *Objeto_Destino;
    cJSON *Grupo_Destino;
    cJSON *Particion_Destino;
    cJSON *Funcion_Destino;
    cJSON *Variable_Destino;
    cJSON *Enviar;
    cJSON *Parametro_Evento;
    cJSON *Condicion_Variable;
    cJSON *Condicion_Igualdad;
    cJSON *Condicion_Valor;
    cJSON *Evento_Id;
    cJSON *Filtro_Repeticion;
    cJSON *Ultimo_Evento;

    m_pServer->m_pLog->Add(100, "[GEvent::CheckEvent] hw_id: %i port %s estado %i", hw_id, port, estado);

    time_now = time(&time_now);

    sprintf(query, "SELECT Id, Objeto, Tipo "
                    "FROM TB_DOM_ASSIGN "
                    "WHERE Dispositivo = %i AND Port = \'%s\'; ", hw_id, port);
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    json_AssignArray = cJSON_CreateArray();
    rc = m_pDB->Query(json_AssignArray, query);
	m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0)
    {
        cJSON_ArrayForEach(json_AssignRow, json_AssignArray)
        {
            Assign_Id = cJSON_GetObjectItemCaseSensitive(json_AssignRow, "Id");
            Assign_Nombre = cJSON_GetObjectItemCaseSensitive(json_AssignRow, "Objeto");
            Assign_Tipo = cJSON_GetObjectItemCaseSensitive(json_AssignRow, "Tipo");

            if(!Assign_Id || !Assign_Nombre || !Assign_Tipo)
            {
                m_pServer->m_pLog->Add(1, "[CheckEvent] Error obteniendo datos de: HW: %i Port: %s", hw_id, port);
                cJSON_Delete(json_AssignArray);
                return (-1);
            }
            m_pServer->m_pLog->Add(20, "[CheckEvent] Evento: Ass: %s Typ: %s Estado: %i", Assign_Nombre->valuestring, Assign_Tipo->valuestring, estado);

            if( atoi(Assign_Tipo->valuestring) == 3 || atoi(Assign_Tipo->valuestring) == 4 )
            {
                /* Evento de alarma */
                m_pServer->m_pLog->Add(20, "[CheckEvent] Evento de Alarma");
                ExtIOEvent_Alarma(atoi(Assign_Id->valuestring), estado);
            }
            else
            {
                /* Evento de dommotica */
                m_pServer->m_pLog->Add(20, "[CheckEvent] Evento de Domotica");
                /* Busco si hay un evento para este cambio */
                sprintf(query, "SELECT * "
                                "FROM TB_DOM_EVENT "
                                "WHERE Objeto_Origen = %s AND %s;",
                                Assign_Id->valuestring, (estado)?"OFF_a_ON = 1":"ON_a_OFF = 1");
                m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                json_EventArray = cJSON_CreateArray();
                rc = m_pDB->Query(json_EventArray, query);
                m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                if(rc > 0)
                {
                    /* Recorro el array */
                    cJSON_ArrayForEach(json_EventRow, json_EventArray)
                    {
                        cJSON_PrintPreallocated(json_EventRow, query, 4095, 0);
                        m_pServer->m_pLog->Add(50, "[EVENTO]: %s", query); 

                        Evento_Id = cJSON_GetObjectItemCaseSensitive(json_EventRow, "Id");
                        Objeto_Destino = cJSON_GetObjectItemCaseSensitive(json_EventRow, "Objeto_Destino");
                        Grupo_Destino = cJSON_GetObjectItemCaseSensitive(json_EventRow, "Grupo_Destino");
                        Particion_Destino = cJSON_GetObjectItemCaseSensitive(json_EventRow, "Particion_Destino");
                        Funcion_Destino = cJSON_GetObjectItemCaseSensitive(json_EventRow, "Funcion_Destino");
                        Variable_Destino = cJSON_GetObjectItemCaseSensitive(json_EventRow, "Variable_Destino");
                        Enviar = cJSON_GetObjectItemCaseSensitive(json_EventRow, "Enviar");
                        Parametro_Evento = cJSON_GetObjectItemCaseSensitive(json_EventRow, "Parametro_Evento");
                        Condicion_Variable = cJSON_GetObjectItemCaseSensitive(json_EventRow, "Condicion_Variable");
                        Condicion_Igualdad = cJSON_GetObjectItemCaseSensitive(json_EventRow, "Condicion_Igualdad");
                        Condicion_Valor = cJSON_GetObjectItemCaseSensitive(json_EventRow, "Condicion_Valor");
                        Filtro_Repeticion = cJSON_GetObjectItemCaseSensitive(json_EventRow, "Filtro_Repeticion");
                        Ultimo_Evento = cJSON_GetObjectItemCaseSensitive(json_EventRow, "Ultimo_Evento");
                        //Flags = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Flags");

                        if(Filtro_Repeticion && Ultimo_Evento)
                        {
                            if(atol(Filtro_Repeticion->valuestring) > 0 )
                            {
                                if(time_now < (atol(Ultimo_Evento->valuestring) + atol(Filtro_Repeticion->valuestring)))
                                {
                                    /* Evento repetido con proximidad */
                                    break;
                                }
                            }
                        }

                        /* TODO: Evaluar condiciones */
                        if(Condicion_Variable && Condicion_Igualdad && Condicion_Valor)
                        {



                        }

                        /* Actualizo la hora del evento */
                        sprintf(query, "UPDATE TB_DOM_EVENT "
                                        "SET Ultimo_Evento = %li "
                                        "WHERE Id = %s;", time_now, Evento_Id->valuestring);
                        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                        rc = m_pDB->Query(nullptr, query);
                        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);

                        /* Si la condicion lo permite ejecuto según corresponda */
                        if( rc >= 0 && Enviar )
                        {
                            if(Objeto_Destino &&  atoi(Objeto_Destino->valuestring) > 0 )
                            {
                                cambios = 1;
                                ChangeAssignById(   atoi(Objeto_Destino->valuestring), 
                                                atoi(Enviar->valuestring),
                                                (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                            }
                            else if(Grupo_Destino &&  atoi(Grupo_Destino->valuestring) > 0 )
                            {
                                cambios = 2;
                                ChangeGroupById(   atoi(Grupo_Destino->valuestring), 
                                                atoi(Enviar->valuestring),
                                                (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                            }
                            else if(Particion_Destino &&  atoi(Particion_Destino->valuestring) > 0 )
                            {
                                cambios = 3;
                                ChangeParticionById(   atoi(Particion_Destino->valuestring), 
                                                atoi(Enviar->valuestring),
                                                (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                            }
                            else if(Funcion_Destino &&  atoi(Funcion_Destino->valuestring) > 0 )
                            {
                                cambios = 4;
                                ChangeFcnById(   atoi(Funcion_Destino->valuestring), 
                                                atoi(Enviar->valuestring),
                                                (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                            }
                            else if(Variable_Destino &&  atoi(Variable_Destino->valuestring) > 0 )
                            {
                                cambios = 5;
                                ChangeVarById(   atoi(Variable_Destino->valuestring), 
                                                atoi(Enviar->valuestring),
                                                (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                            }
                        }
                    }
                }
                cJSON_Delete(json_EventArray);
            }
        }
    }
    cJSON_Delete(json_AssignArray);
    return cambios;
}

int GEvent::CheckAuto(int hw_id, const char* port, int estado_sensor)
{
	char query[4096];
    int rc;
	time_t t;
	struct tm *lt;
	char dia[3];
    int enviar;
    int set_estado;
    int cambios = 0;

    cJSON *json_QueryArray;
    cJSON *json_QueryRow;
	
    cJSON *Id;
    cJSON *Objeto;
    cJSON *Objeto_Salida;
    cJSON *Objeto_Sensor;
    cJSON *Grupo_Salida;
    cJSON *Particion_Salida;
    cJSON *Funcion_Salida;
    cJSON *Variable_Salida;
    cJSON *Enviar_Max;
    cJSON *Enviar_Min;
    cJSON *Parametro_Evento;
    cJSON *Estado;
    cJSON *Estado_Sensor;
    cJSON *Min_Sensor;
    cJSON *Max_Sensor;
    cJSON *Hora_Inicio;
    cJSON *Minuto_Inicio;
    cJSON *Hora_Fin;
    cJSON *Minuto_Fin;
    cJSON *Dias_Semana;
    cJSON *Habilitado;

    const char *tablaAccion[] = {
        "Nada",
        "encendiendo",
        "apagando",
        "cambiando",
        "enviando pulso"
    };

    m_pServer->m_pLog->Add(100, "[GEvent::CheckAuto] HW: %i Port: %s Estado: %i", hw_id, (port)?port:"NULL", estado_sensor);

    /* Busco si hay un evento para este cambio */
	t = time(&t);
	lt = localtime(&t);

	switch(lt->tm_wday)
	{
		case 0:
			strcpy(dia, "Do");
			break;
		case 1:
			strcpy(dia, "Lu");
			break;
		case 2:
			strcpy(dia, "Ma");
			break;
		case 3:
			strcpy(dia, "Mi");
			break;
		case 4:
			strcpy(dia, "Ju");
			break;
		case 5:
			strcpy(dia, "Vi");
			break;
		case 6:
			strcpy(dia, "Sa");
			break;
		default:
			strcpy(dia, "XX");
			break;
	}

    if(hw_id > 0 && port)
    {
        sprintf(query, "SELECT AU.*, ASS.Estado AS Estado_Sensor "
                        "FROM TB_DOM_AUTO AS AU, TB_DOM_ASSIGN AS ASS "
                        "WHERE AU.Objeto_Sensor = ASS.Id AND AU.Id > 0 AND "
                            "ASS.Dispositivo = %i AND ASS.Port = \'%s\' AND "
                            "((ASS.Estado >= Max_Sensor AND AU.Estado = 0) OR (ASS.Estado <= Min_Sensor AND AU.Estado = 1));",
                        hw_id, port);
    }
    else
    {
        sprintf(query, "SELECT AU.*, ASS.Estado AS Estado_Sensor "
                        "FROM TB_DOM_AUTO AS AU, TB_DOM_ASSIGN AS ASS "
                        "WHERE AU.Objeto_Sensor = ASS.Id AND AU.Id > 0;");
    }
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    json_QueryArray = cJSON_CreateArray();
    rc = m_pDB->Query(json_QueryArray, query);
	m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0)
    {
        /* Recorro el array */
        cJSON_ArrayForEach(json_QueryRow, json_QueryArray)
        {
            cJSON_PrintPreallocated(json_QueryRow, query, 4095, 0);
            m_pServer->m_pLog->Add(100, "[CheckAuto]: %s", query); 

            Id = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Id");
            Objeto = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Objeto");
            Objeto_Salida = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Objeto_Salida");
            Objeto_Sensor = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Objeto_Sensor");
            Grupo_Salida = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Grupo_Salida");
            Particion_Salida = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Particion_Salida");
            Funcion_Salida = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Funcion_Salida");
            Variable_Salida = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Variable_Salida");
            Parametro_Evento = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Parametro_Evento");
            Estado = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Estado");
            Estado_Sensor = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Estado_Sensor");
            Min_Sensor = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Min_Sensor");
            Max_Sensor = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Max_Sensor");
            Habilitado = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Habilitado");
            Hora_Inicio = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Hora_Inicio");
            Minuto_Inicio = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Minuto_Inicio");
            Hora_Fin = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Hora_Fin");
            Minuto_Fin = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Minuto_Fin");
            Dias_Semana = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Dias_Semana");

            Enviar_Max = nullptr;
            Enviar_Min = nullptr;
            enviar = 0;
            set_estado = 0;

            do
            {
                /* Controlo servicio Habilitado 
                    Habiltado   0 - Apagado
                                1 - Encendido
                                2 - Automático
                */
                if(atoi(Habilitado->valuestring) == 0)
                {
                    m_pServer->m_pLog->Add(100, "[CheckAuto] Sistema [%s] desactivado.", Objeto->valuestring);
                    if(atoi(Estado->valuestring) == 1)
                    {
                        /* Desabilitado y Encendido -> Mando a apagar */
                        m_pServer->m_pLog->Add(20, "[CheckAuto] Sistema [%s] apagado.", Objeto->valuestring);
                        enviar = 2; /* apagar */
                    }
                    break;
                }
                else if(atoi(Habilitado->valuestring) == 1)
                {
                    m_pServer->m_pLog->Add(100, "[CheckAuto] Sistema [%s] activado.", Objeto->valuestring);
                    if(atoi(Estado->valuestring) == 0)
                    {
                        /* Habilitado y Apagado */
                        m_pServer->m_pLog->Add(20, "[CheckAuto] Sistema [%s] encendido.", Objeto->valuestring);
                        enviar = 1; /* encender */
                    }
                    break;
                }
                else if(atoi(Habilitado->valuestring) == 2) /* Automático */
                {
                    m_pServer->m_pLog->Add(100, "[CheckAuto] Sistema [%s] automático.", Objeto->valuestring);
                    /* Controlo día de la semana vigente */
                    if( !strstr(Dias_Semana->valuestring, dia))
                    {
                        if(atoi(Estado->valuestring) == 1)
                        {
                            /* Fuera de día y encendido -> mando a apagar */
                            m_pServer->m_pLog->Add(20, "[CheckAuto] Sistema [%s] apagando por día de la semana.", Objeto->valuestring);;
                            enviar = 2; /* apagar */
                        }
                        break;
                    }

                    /* Si hay valores validos en el horario */
                    if( (atoi(Hora_Inicio->valuestring) != atoi(Hora_Fin->valuestring)) || (atoi(Minuto_Inicio->valuestring) != atoi(Minuto_Fin->valuestring)) )
                    {
                        /* Controlo horario de funcionamiento */
                        if( (atoi(Hora_Inicio->valuestring) > atoi(Hora_Fin->valuestring)) ||
                            ( (atoi(Hora_Inicio->valuestring) == atoi(Hora_Fin->valuestring)) &&
                            ( atoi(Minuto_Inicio->valuestring) > atoi(Minuto_Fin->valuestring)) ) )
                        {
                            /* El período de actividad termina al dia siguiente de que empieza - Inicio > Fin   */
                            if( (lt->tm_hour < atoi(Hora_Inicio->valuestring)  ||
                                (lt->tm_hour == atoi(Hora_Inicio->valuestring) && lt->tm_min < atoi(Minuto_Inicio->valuestring)) ) 
                                &&
                                (lt->tm_hour > atoi(Hora_Fin->valuestring)  ||
                                (lt->tm_hour == atoi(Hora_Fin->valuestring) && lt->tm_min > atoi(Minuto_Fin->valuestring)) ) )
                            {
                                /* Fuera de horario */
                                if(atoi(Estado->valuestring) == 1)
                                {
                                    /* Fuera de horario y encendido -> mando a apagar */
                                    m_pServer->m_pLog->Add(20, "[CheckAuto] Sistema [%s] apagando por horario.", Objeto->valuestring);
                                    enviar = 2; /* apagar */
                                }
                                break;
                            }
                        }
                        else
                        {
                            /* El período de actividad empieza y termina el mismo día - Fin > Inicio */
                            if( (lt->tm_hour < atoi(Hora_Inicio->valuestring)  ||
                                (lt->tm_hour == atoi(Hora_Inicio->valuestring) && lt->tm_min < atoi(Minuto_Inicio->valuestring)) ) 
                                ||
                                (lt->tm_hour > atoi(Hora_Fin->valuestring)  ||
                                (lt->tm_hour == atoi(Hora_Fin->valuestring) && lt->tm_min > atoi(Minuto_Fin->valuestring)) ) )
                            {
                                /* Fuera de horario */
                                if(atoi(Estado->valuestring) == 1)
                                {
                                    /* Fuera de horario y encendido -> mando a apagar */
                                    m_pServer->m_pLog->Add(20, "[CheckAuto] Sistema [%s] apagando por horario.", Objeto->valuestring);
                                    enviar = 2; /* apagar */
                                }
                                break;
                            }
                        }
                    }

                    /* Si hay sensor definido evalúo el estado del sensor */
                    if(atoi(Objeto_Sensor->valuestring) > 0)
                    {
                        if(atoi(Estado->valuestring) == 0 && atoi(Estado_Sensor->valuestring) >= atoi(Max_Sensor->valuestring))
                        {
                            Enviar_Max = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Enviar_Max");
                            enviar = atoi(Enviar_Max->valuestring);
                            m_pServer->m_pLog->Add(20, "[CheckAuto] Sistema [%s] %s por sensor (MAX).", Objeto->valuestring, tablaAccion[enviar]);
                            break;
                        }

                        if(atoi(Estado->valuestring) == 1 && atoi(Estado_Sensor->valuestring) <= atoi(Min_Sensor->valuestring))
                        {
                            Enviar_Min = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Enviar_Min");
                            enviar = atoi(Enviar_Min->valuestring);
                            m_pServer->m_pLog->Add(20, "[CheckAuto] Sistema [%s] %s por sensor (MIN).", Objeto->valuestring, tablaAccion[enviar]);
                            break;
                        }
                    }
                    else
                    {
                        /* Si no hay sensor definido */
                        if(atoi(Estado->valuestring) == 0)
                        {
                            m_pServer->m_pLog->Add(20, "[CheckAuto] Sistema [%s] encendiendo sin sensor.", Objeto->valuestring);
                            enviar = 1;
                            break;
                        }
                    }

                }
                break;
            } while(1);

            /* Si la condicion lo permite ejecuto según corresponda */
            if( enviar > 0 )
            {

                if(Objeto_Salida &&  atoi(Objeto_Salida->valuestring) > 0 )
                {
                    cambios = 1;
                    ChangeAssignById(   atoi(Objeto_Salida->valuestring), enviar,
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);

                }
                else if(Grupo_Salida &&  atoi(Grupo_Salida->valuestring) > 0 )
                {
                    cambios = 2;
                    ChangeGroupById(   atoi(Grupo_Salida->valuestring), enviar,
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                }
                else if(Particion_Salida &&  atoi(Particion_Salida->valuestring) > 0 )
                {
                    cambios = 3;
                    ChangeParticionById(   atoi(Particion_Salida->valuestring), enviar,
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                }
                else if(Funcion_Salida &&  atoi(Funcion_Salida->valuestring) > 0 )
                {
                    cambios = 4;
                    ChangeFcnById(   atoi(Funcion_Salida->valuestring), enviar,
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                }
                else if(Variable_Salida &&  atoi(Variable_Salida->valuestring) > 0 )
                {
                    cambios = 5;
                    ChangeVarById(   atoi(Variable_Salida->valuestring), enviar,
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                }

                if(enviar == 1) set_estado = 1;
                else set_estado = 0;

                sprintf(query, "UPDATE TB_DOM_AUTO SET Estado = %i WHERE Id = %s;", set_estado, Id->valuestring);
                m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                rc = m_pDB->Query(nullptr, query);
                m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);

				m_pServer->m_pLog->Add(90, "Notify [dompi_auto_change]");
				m_pServer->Notify("dompi_auto_change", nullptr, 0);
            }
        }
    }

    cJSON_Delete(json_QueryArray);
    return cambios;
}

int GEvent::ChangeAssignByName(const char* name, int accion, int param)
{
	int rc = 0;
	char query[4096];

	m_pServer->m_pLog->Add(20, "[GEvent::ChangeAssignByName] name= %s, accion= %i param= %i", name, accion, param);
	switch(accion)
	{
		case 1: /* Encender */
			sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
							"SET Estado = 1, Actualizar = 1 "
							"WHERE UPPER(Objeto) = UPPER(\'%s\');", name);
			break;
		case 2: /* Apagar */
			sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
							"SET Estado = 0, Actualizar = 1 "
							"WHERE UPPER(Objeto) = UPPER(\'%s\');", name);
			break;
		case 3:	/* Switch */
			sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
							"SET Estado = (1 - Estado), Actualizar = 1 "
							"WHERE UPPER(Objeto) = UPPER(\'%s\');", name);
			break;
		case 4: /* Pulso */
            if(param > 0)
            {
                sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
                                "SET Estado = %i, Actualizar = 1 "
                                "WHERE UPPER(Objeto) = UPPER(\'%s\');", param, name);
            }
            else
            {
                sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
                                "SET Estado = Analog_Mult_Div_Valor, Actualizar = 1 "
                                "WHERE UPPER(Objeto) = UPPER(\'%s\');", name);
            }
			break;
		default:
			break;
	}
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = m_pDB->Query(NULL, query);
	m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
	return rc;
}

int GEvent::ChangeAssignById(int id, int accion, int param)
{
	int rc = 0;
	char query[4096];

	m_pServer->m_pLog->Add(20, "[GEvent::ChangeAssignById] id= %i, accion= %i param= %i", id, accion, param);
	switch(accion)
	{
		case 1: /* Encender */
			sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
							"SET Estado = 1, Actualizar = 1 "
							"WHERE Id = %i;", id);
			break;
		case 2: /* Apagar */
			sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
							"SET Estado = 0, Actualizar = 1 "
							"WHERE Id = %i;", id);
			break;
		case 3:	/* Switch */
			sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
							"SET Estado = (1 - Estado), Actualizar = 1 "
							"WHERE Id = %i;", id);
			break;
		case 4: /* Pulso */
            if(param > 1)
            {
                sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
                                "SET Estado = %i, Actualizar = 1 "
                                "WHERE Id = %i;", param, id);
            }
            else
            {
                sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
                                "SET Estado = Analog_Mult_Div_Valor, Actualizar = 1 "
                                "WHERE Id = %i;", id);
            }
			break;
		default:
			break;
	}
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = m_pDB->Query(NULL, query);
	m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
	return rc;
}

int GEvent::ChangeGroupByName(const char* name, int accion, int param)
{
	int rc = 0;
	char query[4096];

	m_pServer->m_pLog->Add(20, "[GEvent::ChangeGroupByName] name= %s, accion= %i param= %i", name, accion, param);

	switch(accion)
	{
		case 1: /* Encender */
			sprintf(query, 	"UPDATE TB_DOM_GROUP "
							"SET Estado = 1, Actualizar = 1 "
							"WHERE UPPER(Grupo) = UPPER(\'%s\');", name);
			break;
		case 2: /* Apagar */
			sprintf(query, 	"UPDATE TB_DOM_GROUP "
							"SET Estado = 0, Actualizar = 1 "
							"WHERE UPPER(Grupo) = UPPER(\'%s\');", name);
			break;
		case 3:	/* Switch */
			sprintf(query, 	"UPDATE TB_DOM_GROUP "
							"SET Estado = (1 - Estado), Actualizar = 1 "
							"WHERE UPPER(Grupo) = UPPER(\'%s\');", name);
			break;
		case 4: /* Pulso */
            m_pServer->m_pLog->Add(1, "[ChangeGroupByName]  ERROR: Envio de pulso a Grupo [%s]", name);
            return (-1);
			break;
		default:
			break;
	}
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = m_pDB->Query(NULL, query);
	m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
	return rc;
}

int GEvent::ChangeGroupById(int id, int accion, int param)
{
	int rc = 0;
	char query[4096];

	m_pServer->m_pLog->Add(20, "[GEvent::ChangeGroupById] id= %i, accion= %i param= %i", id, accion, param);

	switch(accion)
	{
		case 1: /* Encender */
			sprintf(query, 	"UPDATE TB_DOM_GROUP "
							"SET Estado = 1, Actualizar = 1 "
							"WHERE Id = %i;", id);
			break;
		case 2: /* Apagar */
			sprintf(query, 	"UPDATE TB_DOM_GROUP "
							"SET Estado = 0, Actualizar = 1 "
							"WHERE Id = %i;", id);
			break;
		case 3:	/* Switch */
			sprintf(query, 	"UPDATE TB_DOM_GROUP "
							"SET Estado = (1 - Estado), Actualizar = 1 "
							"WHERE Id = %i;", id);
			break;
		case 4: /* Pulso */
            m_pServer->m_pLog->Add(1, "[ChangeGroupByName]  ERROR: Envio de pulso a Grupo [%i]", id);
            return (-1);
			break;
		default:
			break;
	}
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = m_pDB->Query(NULL, query);
	m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
	return rc;
}

int GEvent::ChangeParticionById(int id, int accion, int param)
{
	int rc = 0;

	m_pServer->m_pLog->Add(20, "[GEvent::ChangeParticionById] id= %i, accion= %i param= %i", id, accion, param);

	switch(accion)
	{
		case 1: /* Encender */
            Activar_Alarma(id, param);
			break;
		case 2: /* Apagar */
            Desactivar_Alarma(id);
			break;
		case 3:	/* Switch */
            Switch_Alarma(id);
			break;
		case 4: /* Pulso */
            m_pServer->m_pLog->Add(1, "[ChangeParticionById]  ERROR: Envio de pulso a Particion [%i]", id);
            return (-1);
			break;
		default:
			break;
	}
	return rc;
}

int GEvent::ChangeFcnByName(const char* /*name*/, int /*accion*/, int /*param*/)
{
    return (-1);
}

int GEvent::ChangeFcnById(int /*id*/, int /*accion*/, int /*param*/)
{
    return (-1);
}

int GEvent::ChangeVarByName(const char* /*name*/, int /*accion*/, int /*param*/)
{
    return (-1);
}

int GEvent::ChangeVarById(int /*id*/, int /*accion*/, int /*param*/)
{
    return (-1);
}

int GEvent::ChangeAutoByName(const char* name, int accion, int param)
{
	int rc = 0;
	char query[4096];

	m_pServer->m_pLog->Add(20, "[GEvent::ChangeAutoByName] name= %s, accion= %i param= %i", name, accion, param);
	switch(accion)
	{
		case 1: /* Encender */
			sprintf(query, 	"UPDATE TB_DOM_AUTO "
							"SET Habilitado = 1, Actualizar = 1 "
							"WHERE UPPER(Objeto) = UPPER(\'%s\');", name);
			break;
		case 2: /* Apagar */
			sprintf(query, 	"UPDATE TB_DOM_AUTO "
							"SET Habilitado = 0, Actualizar = 1 "
							"WHERE UPPER(Objeto) = UPPER(\'%s\');", name);
			break;
		case 3:	/* Switch */
			sprintf(query, 	"UPDATE TB_DOM_AUTO "
							"SET Habilitado = MOD( (Habilitado+1),3 ), Actualizar = 1 "
							"WHERE UPPER(Objeto) = UPPER(\'%s\');", name);
			break;
		case 5: /* Automatico */
			sprintf(query, 	"UPDATE TB_DOM_AUTO "
							"SET Habilitado = 2, Actualizar = 1 "
							"WHERE UPPER(Objeto) = UPPER(\'%s\');", name);
			break;
		default:
			break;
	}
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = m_pDB->Query(NULL, query);
	m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
	return rc;
}

int GEvent::ChangeAutoById(int id, int accion, int param)
{
	int rc = 0;
	char query[4096];

	m_pServer->m_pLog->Add(20, "[GEvent::ChangeAutoById] id= %i, accion= %i param= %i", id, accion, param);
	switch(accion)
	{
		case 1: /* Encender */
			sprintf(query, 	"UPDATE TB_DOM_AUTO "
							"SET Habilitado = 1, Actualizar = 1 "
							"WHERE Id = %i;", id);
			break;
		case 2: /* Apagar */
			sprintf(query, 	"UPDATE TB_DOM_AUTO "
							"SET Habilitado = 0, Actualizar = 1 "
							"WHERE Id = %i;", id);
			break;
		case 3:	/* Switch */
			sprintf(query, 	"UPDATE TB_DOM_AUTO "
							"SET Habilitado = MOD( (Habilitado+1),3 ), Actualizar = 1 "
							"WHERE Id = %i;", id);
			break;
		case 5: /* Automatico */
			sprintf(query, 	"UPDATE TB_DOM_AUTO "
							"SET Habilitado = 2, Actualizar = 1 "
							"WHERE Id = %i;", id);
			break;
		default:
			break;
	}
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = m_pDB->Query(NULL, query);
	m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
	return rc;
}

int GEvent::Habilitar_Alarma(const char* /*zona*/, const char* /*particion*/)
{

    m_alarm_need_update = 1;
    return 0;
}

int GEvent::Deshabilitar_Alarma(const char* /*zona*/, const char* /*particion*/)
{

    m_alarm_need_update = 1;
    return 0;
}

int GEvent::Activar_Alarma(int particion, int total)
{
    int rc;
    char query[4096];

    cJSON *json_Query_Result;
    cJSON *json_Query_Row;
    cJSON *json_Zona_Nombre;
    //cJSON *json_Zona_Tipo;
    cJSON *json_Zona_Activa;
    cJSON *json_Zona_Estado;
    cJSON *json_Testigo_Activacion;

    int zonas_abiertas = 0;

    m_pServer->m_pLog->Add(20, "[GEvent::Activar_Alarma] Part: %i tot: %i", particion, total);

    /* Busco zonas abiertas */
    sprintf(query, "SELECT A.Objeto, Z.Tipo_Zona, Z.Activa, A.Estado "
                    "FROM TB_DOM_ALARM_PARTICION AS P, TB_DOM_ALARM_ZONA AS Z, TB_DOM_ASSIGN AS A "
                    "WHERE P.Id = Z.Particion AND Z.Objeto_Zona = A.Id AND "
                        "P.Id = %i);", particion);
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    json_Query_Result = cJSON_CreateArray();
    rc = m_pDB->Query(json_Query_Result, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0)
    {
        cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
        {
            json_Zona_Nombre = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Objeto");
            //json_Zona_Tipo = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Tipo_Zona");
            json_Zona_Activa = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Activa");
            json_Zona_Estado = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Estado");
            m_pServer->m_pLog->Add(20, "[ALARMA] Particion %i zona %s %s %s", 
                particion, json_Zona_Nombre->valuestring, 
                (atoi(json_Zona_Activa->valuestring)>0)?"HABILITADA":"INHABILITADA",
                (atoi(json_Zona_Estado->valuestring)>0)?"ABIERTA":"CERRADA");

            if( atoi(json_Zona_Activa->valuestring) > 0 && atoi(json_Zona_Estado->valuestring) > 0)
            {
                zonas_abiertas++;
            }
        }
    }
    cJSON_Delete(json_Query_Result);

    if(zonas_abiertas > 0)
    {
        m_pServer->m_pLog->Add(20, "[ALARMA] Particion %i no se activa, %i zonas abiertas", particion, zonas_abiertas);
        return zonas_abiertas;
    }

    sprintf(query, "UPDATE TB_DOM_ALARM_PARTICION "
                    "SET Estado_Activacion = %i, Estado_Memoria = 0 "
                    "WHERE Id = %i;", (total)?2:1, particion);
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    rc = m_pDB->Query(nullptr, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0)
    {
        /* Actualizo los indicadores */
        sprintf(query, "SELECT Testigo_Activacion "
                        "FROM TB_DOM_ALARM_PARTICION "
                        "WHERE Id = %i;", particion);
        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
        json_Query_Result = cJSON_CreateArray();
        rc = m_pDB->Query(json_Query_Result, query);
        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
        if(rc > 0)
        {
            cJSON_ArrayForEach(json_Query_Row, json_Query_Result) { break; }
            json_Testigo_Activacion = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Testigo_Activacion");
            if(json_Testigo_Activacion)
            {
                ChangeAssignById(atoi(json_Testigo_Activacion->valuestring), 1, 0);
            }
        }
        cJSON_Delete(json_Query_Result);

        m_pServer->m_pLog->Add(1, "[ALARMA] Particion %i activada en forma %s", particion, (total)?"Total":"Parcial");
        m_alarm_need_update = 1;
    }
    else
    {
        m_pServer->m_pLog->Add(1, "[ALARMA] Error: No se encontro la particion %i", particion);
    }
    return 0;
}

int GEvent::Activar_Alarma(const char* particion, int total)
{
    int rc;
    char query[4096];

    cJSON *json_Query_Result;
    cJSON *json_Query_Row;
    cJSON *json_Zona_Nombre;
    //cJSON *json_Zona_Tipo;
    cJSON *json_Zona_Activa;
    cJSON *json_Zona_Estado;
    cJSON *json_Testigo_Activacion;

    int zonas_abiertas = 0;

    m_pServer->m_pLog->Add(20, "[GEvent::Activar_Alarma] Part: %s tot: %i", particion, total);

    /* Busco zonas abiertas */
    sprintf(query, "SELECT A.Objeto, Z.Tipo_Zona, Z.Activa, A.Estado "
                    "FROM TB_DOM_ALARM_PARTICION AS P, TB_DOM_ALARM_ZONA AS Z, TB_DOM_ASSIGN AS A "
                    "WHERE P.Id = Z.Particion AND Z.Objeto_Zona = A.Id AND "
                        "UPPER(P.Nombre) = UPPER(\'%s\');", particion);
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    json_Query_Result = cJSON_CreateArray();
    rc = m_pDB->Query(json_Query_Result, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0)
    {
        cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
        {
            json_Zona_Nombre = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Objeto");
            //json_Zona_Tipo = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Tipo_Zona");
            json_Zona_Activa = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Activa");
            json_Zona_Estado = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Estado");
            m_pServer->m_pLog->Add(20, "[ALARMA] Particion %s zona %s %s %s", 
                particion, json_Zona_Nombre->valuestring, 
                (atoi(json_Zona_Activa->valuestring)>0)?"HABILITADA":"INHABILITADA",
                (atoi(json_Zona_Estado->valuestring)>0)?"ABIERTA":"CERRADA");

            if( atoi(json_Zona_Activa->valuestring) > 0 && atoi(json_Zona_Estado->valuestring) > 0)
            {
                zonas_abiertas++;
            }
        }
    }
    cJSON_Delete(json_Query_Result);

    if(zonas_abiertas > 0)
    {
        m_pServer->m_pLog->Add(20, "[ALARMA] Particion %s no se activa, %i zonas abiertas", particion, zonas_abiertas);
        return zonas_abiertas;
    }

    sprintf(query, "UPDATE TB_DOM_ALARM_PARTICION "
                    "SET Estado_Activacion = %i, Estado_Memoria = 0 "
                    "WHERE UPPER(Nombre) = UPPER(\'%s\');", (total)?2:1, particion);
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    rc = m_pDB->Query(nullptr, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0)
    {
        /* Actualizo los indicadores */
        sprintf(query, "SELECT Testigo_Activacion "
                        "FROM TB_DOM_ALARM_PARTICION "
                        "WHERE UPPER(Nombre) = UPPER(\'%s\');", particion);
        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
        json_Query_Result = cJSON_CreateArray();
        rc = m_pDB->Query(json_Query_Result, query);
        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
        if(rc > 0)
        {
            cJSON_ArrayForEach(json_Query_Row, json_Query_Result) { break; }
            json_Testigo_Activacion = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Testigo_Activacion");
            if(json_Testigo_Activacion)
            {
                ChangeAssignById(atoi(json_Testigo_Activacion->valuestring), 1, 0);
            }
        }
        cJSON_Delete(json_Query_Result);

        m_pServer->m_pLog->Add(1, "[ALARMA] Particion %s activada en forma %s", particion, (total)?"Total":"Parcial");
        m_alarm_need_update = 1;
    }
    else
    {
        m_pServer->m_pLog->Add(1, "[ALARMA] Error: No se encontro la particion %s", particion);
    }
    return 0;
}

int GEvent::Desactivar_Alarma(int particion)
{
    int rc;
    char query[4096];

    cJSON *json_Query_Result;
    cJSON *json_Query_Row;
    cJSON *json_Objeto_Salida;
    cJSON *json_Testigo_Activacion;

    m_pServer->m_pLog->Add(20, "[GEvent::Desactivar_Alarma] Part: %i", particion);

    /* Desactivo la particion */
    sprintf(query, "UPDATE TB_DOM_ALARM_PARTICION "
                    "SET Estado_Activacion = 0, Estado_Alarma = 0 "
                    "WHERE Id = %i;", particion);
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    rc = m_pDB->Query(nullptr, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0)
    {
        /* Busco salida para apagar */
        sprintf(query, "SELECT S.Objeto_Salida, P.Testigo_Activacion "
                        "FROM TB_DOM_ALARM_PARTICION AS P, TB_DOM_ALARM_SALIDA AS S "
                        "WHERE P.Id = S.Particion AND "
                            "P.Id = %i;", particion);
        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
        json_Query_Result = cJSON_CreateArray();
        rc = m_pDB->Query(json_Query_Result, query);
        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
        if(rc > 0)
        {
            cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
            {
                json_Objeto_Salida = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Objeto_Salida");
                if(json_Objeto_Salida)
                {
                    /* Apago la salida de alarma */
                    ChangeAssignById(atoi(json_Objeto_Salida->valuestring), 2, 0);
                }
                json_Testigo_Activacion = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Testigo_Activacion");
                if(json_Testigo_Activacion)
                {
                    ChangeAssignById(atoi(json_Testigo_Activacion->valuestring), 2, 0);
                }
            } /* cJSON_ArrayForEach */
        }
        cJSON_Delete(json_Query_Result);

        m_pServer->m_pLog->Add(1, "[ALARMA] Particion %i desactivada", particion);
        m_alarm_need_update = 1;
    }
    else
    {
        m_pServer->m_pLog->Add(1, "[ALARMA] Error: No se encontro la particion %i", particion);
        return (-1);
    }

    return 0;
}

int GEvent::Desactivar_Alarma(const char* particion)
{
    int rc;
    char query[4096];

    cJSON *json_Query_Result;
    cJSON *json_Query_Row;
    cJSON *json_Objeto_Salida;
    cJSON *json_Testigo_Activacion;

    m_pServer->m_pLog->Add(20, "[GEvent::Desactivar_Alarma] Part: %s", particion);

    /* Desactivo la particion */
    sprintf(query, "UPDATE TB_DOM_ALARM_PARTICION "
                    "SET Estado_Activacion = 0, Estado_Alarma = 0 "
                    "WHERE UPPER(Nombre) = UPPER(\'%s\');", particion);
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    rc = m_pDB->Query(nullptr, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0)
    {
        /* Busco salida para apagar */
        sprintf(query, "SELECT S.Objeto_Salida, P.Testigo_Activacion "
                        "FROM TB_DOM_ALARM_PARTICION AS P, TB_DOM_ALARM_SALIDA AS S "
                        "WHERE P.Id = S.Particion AND "
                            "UPPER(P.Nombre) = UPPER(\'%s\');", particion);
        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
        json_Query_Result = cJSON_CreateArray();
        rc = m_pDB->Query(json_Query_Result, query);
        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
        if(rc > 0)
        {
            cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
            {
                json_Objeto_Salida = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Objeto_Salida");
                if(json_Objeto_Salida)
                {
                    /* Apago la salida de alarma */
                    ChangeAssignById(atoi(json_Objeto_Salida->valuestring), 2, 0);
                }
                json_Testigo_Activacion = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Testigo_Activacion");
                if(json_Testigo_Activacion)
                {
                    ChangeAssignById(atoi(json_Testigo_Activacion->valuestring), 2, 0);
                }
            } /* cJSON_ArrayForEach */
        }
        cJSON_Delete(json_Query_Result);

        m_pServer->m_pLog->Add(1, "[ALARMA] Particion %s desactivada", particion);
        m_alarm_need_update = 1;
    }
    else
    {
        m_pServer->m_pLog->Add(1, "[ALARMA] Error: No se encontro la particion %s", particion);
        return (-1);
    }

    return 0;
}

int GEvent::ExtIOEvent_Alarma(int assign, int status)
{
    int rc;
    char query[4096];

    cJSON *json_Part_Result;
    cJSON *json_Part_Row;

    cJSON *json_Join_Result;
    cJSON *json_Join_Row;

    cJSON *json_Query_Result;
    cJSON *json_Query_Row;

    //cJSON *json_Part_Id;
    cJSON *json_Particion;
    cJSON *json_Particion_Activada;
    cJSON *json_Entrada_Act_Total;
    cJSON *json_Entrada_Act_Parcial;

    cJSON *json_Ass_Id;
    cJSON *json_Ass_Nombre;
    //cJSON *json_Tipo_Zona;
    cJSON *json_Grupo_Zona;
    cJSON *json_Zona_Activa;
    cJSON *json_Part_Nombre;
    cJSON *json_Tiempo_De_Alerta;

    cJSON *json_Objeto_Salida;

    m_pServer->m_pLog->Add(20, "[GEvent::ExtIOEvent_Alarma] Ass: %i Status: %i", assign, status);
    
    /* Me fijo si corresponde con una entrada de Activación / Desactivación */
    sprintf(query, "SELECT Id, Nombre, Estado_Activacion, Entrada_Act_Total, Entrada_Act_Parcial "
                    "FROM TB_DOM_ALARM_PARTICION "
                    "WHERE Id > 0;");
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    json_Part_Result = cJSON_CreateArray();
    rc = m_pDB->Query(json_Part_Result, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0)
    {
        m_alarm_need_update = 1;
        cJSON_ArrayForEach(json_Part_Row, json_Part_Result)
        {
            //json_Part_Id = cJSON_GetObjectItemCaseSensitive(json_Part_Row, "Id");
            json_Particion = cJSON_GetObjectItemCaseSensitive(json_Part_Row, "Nombre");
            json_Particion_Activada = cJSON_GetObjectItemCaseSensitive(json_Part_Row, "Estado_Activacion");
            json_Entrada_Act_Total = cJSON_GetObjectItemCaseSensitive(json_Part_Row, "Entrada_Act_Total");
            json_Entrada_Act_Parcial = cJSON_GetObjectItemCaseSensitive(json_Part_Row, "Entrada_Act_Parcial");

            /* Si es Activación / Desactivación Total*/
            if(assign == atoi(json_Entrada_Act_Total->valuestring) && status == 0)
            {
                /* Entrada de Act o Des */
                if(atoi(json_Particion_Activada->valuestring) == 0)
                {
                    Activar_Alarma(json_Particion->valuestring, 1);
                }
                else
                {
                    Desactivar_Alarma(json_Particion->valuestring);
                }
            }
            /* Si es Activación / Desactivación Parcial */
            else if(assign == atoi(json_Entrada_Act_Parcial->valuestring) && status == 0)
            {
                /* Entrada de Act o Des */
                if(atoi(json_Particion_Activada->valuestring) == 0)
                {
                    Activar_Alarma(json_Particion->valuestring, 0);
                }
                else
                {
                    Desactivar_Alarma(json_Particion->valuestring);
                }
            }
        }
    }
    cJSON_Delete(json_Part_Result);

    /* Me fijo si corresponde con una Zona */
    sprintf(query, "SELECT A.Id, A.Objeto, Z.Tipo_Zona, Z.Grupo, Z.Activa, P.Nombre, P.Estado_Activacion, P.Tiempo_De_Alerta "
                    "FROM TB_DOM_ALARM_PARTICION AS P, TB_DOM_ALARM_ZONA AS Z, TB_DOM_ASSIGN AS A "
                    "WHERE P.Id > 0 AND P.Id = Z.Particion AND Z.Objeto_Zona = A.Id;");
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    json_Join_Result = cJSON_CreateArray();
    rc = m_pDB->Query(json_Join_Result, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0)
    {
        cJSON_ArrayForEach(json_Join_Row, json_Join_Result)
        {

            json_Ass_Id = cJSON_GetObjectItemCaseSensitive(json_Join_Row, "Id");
            json_Ass_Nombre = cJSON_GetObjectItemCaseSensitive(json_Join_Row, "Objeto");
            //json_Tipo_Zona = cJSON_GetObjectItemCaseSensitive(json_Join_Row, "Tipo_Zona");
            json_Grupo_Zona = cJSON_GetObjectItemCaseSensitive(json_Join_Row, "Grupo");
            json_Zona_Activa = cJSON_GetObjectItemCaseSensitive(json_Join_Row, "Activa");
            json_Part_Nombre = cJSON_GetObjectItemCaseSensitive(json_Join_Row, "Nombre");
            json_Particion_Activada = cJSON_GetObjectItemCaseSensitive(json_Join_Row, "Estado_Activacion");
            json_Tiempo_De_Alerta = cJSON_GetObjectItemCaseSensitive(json_Join_Row, "Tiempo_De_Alerta");

            /* Cuando la zona se abre y está activa */
            if( atoi(json_Ass_Id->valuestring) == assign && atoi(json_Zona_Activa->valuestring) && status)
            {
                if( atoi(json_Particion_Activada->valuestring) == 2 || 
                    (atoi(json_Particion_Activada->valuestring) == 1 && atoi(json_Grupo_Zona->valuestring) == 1) )
                {
                    m_pServer->m_pLog->Add(20, "[ALARMA] Particion: %s Zona: %s", json_Part_Nombre->valuestring, json_Ass_Nombre->valuestring);

                    /* Actualizo el estado de alarma de la particion */
                    sprintf(query, "UPDATE TB_DOM_ALARM_PARTICION "
                                    "SET Estado_Alarma = %s , Estado_Memoria = (Estado_Memoria+1) "
                                    "WHERE UPPER(Nombre) = UPPER(\'%s\');", json_Tiempo_De_Alerta->valuestring, json_Part_Nombre->valuestring);
                    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                    rc = m_pDB->Query(nullptr, query);
                    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);

                    /* Enciendo la salida de alarma */
                    sprintf(query, "SELECT S.Objeto_Salida "
                                    "FROM TB_DOM_ALARM_PARTICION AS P, TB_DOM_ALARM_SALIDA AS S "
                                    "WHERE P.Id = S.Particion AND "
                                        "UPPER(P.Nombre) = UPPER(\'%s\');", json_Part_Nombre->valuestring);
                    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                    json_Query_Result = cJSON_CreateArray();
                    rc = m_pDB->Query(json_Query_Result, query);
                    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                    if(rc > 0)
                    {
                        cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
                        {
                            json_Objeto_Salida = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Objeto_Salida");
                            if(json_Objeto_Salida)
                            {
                                /* Apago la salida de alarma */
                                ChangeAssignById(atoi(json_Objeto_Salida->valuestring), 1, 0);
                            }
                        } /* cJSON_ArrayForEach */
                    }
                    cJSON_Delete(json_Query_Result);

                }
            }
        }
    }
    cJSON_Delete(json_Join_Result);


    return 0;
}

void GEvent::Task_Alarma( void )
{
    int rc;
    char query[4096];
    int delta_time;
    int iTemp;
    unsigned long now;

    cJSON *json_Query_Result;
    cJSON *json_Query_Row;

    cJSON *json_Out_Result;
    cJSON *json_Out_Row;

    cJSON *json_Part_Id;
    cJSON *json_Particion;
    cJSON *json_Particion_Activada;
    cJSON *json_Estado_Alarma;
    cJSON *json_Tiempo_De_Salida;
    cJSON *json_Tiempo_De_Entrada;
    cJSON *json_Tiempo_De_Alerta;
    cJSON *json_Objeto_Salida;
    //cJSON *json_Tipo_Salida;

    now = time((time_t*)&now);
    delta_time = (int)(now - m_last_time_task);
    m_last_time_task = now;

    sprintf(query, "SELECT * "
                    "FROM TB_DOM_ALARM_PARTICION "
                    "WHERE Id > 0;");
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    json_Query_Result = cJSON_CreateArray();
    rc = m_pDB->Query(json_Query_Result, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0)
    {
        cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
        {
            json_Part_Id = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Id");
            json_Particion = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Nombre");
            json_Particion_Activada = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Estado_Activacion");
            json_Estado_Alarma = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Estado_Alarma");
            json_Tiempo_De_Salida = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Tiempo_De_Salida");
            json_Tiempo_De_Entrada = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Tiempo_De_Entrada");
            json_Tiempo_De_Alerta = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Tiempo_De_Alerta");

            m_pServer->m_pLog->Add(100, "[GEvent::Task_Alarma] Id: %s Nom: %s Act: %s Alarm: %s Sal: %s Ent: %s Alert: %s", 
            json_Part_Id->valuestring,json_Particion->valuestring,json_Particion_Activada->valuestring,json_Estado_Alarma->valuestring,
            json_Tiempo_De_Salida->valuestring,json_Tiempo_De_Entrada->valuestring,json_Tiempo_De_Alerta->valuestring);

            /* Control de Tiempo de alerta */
            iTemp = atoi(json_Estado_Alarma->valuestring);
            if( iTemp > 0 && delta_time > 0 )
            {
                iTemp -= delta_time;
                if(iTemp < 0) iTemp = 0;
                sprintf(query, "UPDATE TB_DOM_ALARM_PARTICION "
                                    "SET Estado_Alarma = %i "
                                    "WHERE UPPER(Nombre) = UPPER(\'%s\');", iTemp, json_Particion->valuestring);
                m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                rc = m_pDB->Query(nullptr, query);
                m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);

                if(iTemp == 0)
                {
                    /* Hay que apagar la salida */
                    sprintf(query, "SELECT S.Objeto_Salida "
                                    "FROM TB_DOM_ALARM_PARTICION AS P, TB_DOM_ALARM_SALIDA AS S "
                                    "WHERE P.Id = S.Particion AND "
                                        "UPPER(P.Nombre) = UPPER(\'%s\');", json_Particion->valuestring);
                    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                    json_Out_Result = cJSON_CreateArray();
                    rc = m_pDB->Query(json_Out_Result, query);
                    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                    if(rc > 0)
                    {
                        cJSON_ArrayForEach(json_Out_Row, json_Out_Result)
                        {
                            json_Objeto_Salida = cJSON_GetObjectItemCaseSensitive(json_Out_Row, "Objeto_Salida");
                            if(json_Objeto_Salida)
                            {
                                /* Apago la salida de alarma */
                                ChangeAssignById(atoi(json_Objeto_Salida->valuestring), 2, 0);
                                m_alarm_need_update = 1;
                            }
                        } /* cJSON_ArrayForEach */
                    }
                    cJSON_Delete(json_Out_Result);
                }
            }
        }

    }
    cJSON_Delete(json_Query_Result);
}

int GEvent::Estado_Alarma(int particion, char* json_estado, int json_max)
{
    int rc;
    char query[4096];
    int estado_act;

    cJSON *json_Query_Result;
    cJSON *json_EstadoPart = nullptr;
    cJSON *json_EstadoZona;
    cJSON *json_EstadoSalida;
    cJSON *json_obj;
    cJSON *json_EstadoAct;

    m_pServer->m_pLog->Add(100, "[GEvent::Estado_Alarma] Part: %d", particion);

    json_Query_Result = cJSON_CreateArray();
    sprintf(query, "SELECT  Id, Nombre, Estado_Activacion, Estado_Memoria, Estado_Alarma "
                    "FROM TB_DOM_ALARM_PARTICION "
                    "WHERE Id = %i;", particion);
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    rc = m_pDB->Query(json_Query_Result, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0)
    {
        /* Paso el primero y único */
        cJSON_ArrayForEach(json_EstadoPart, json_Query_Result) { break; }
    }

    if(rc > 0 && json_estado)
    {
        /* Información de Zonas */
        json_EstadoZona = cJSON_CreateArray();
        sprintf(query, "SELECT Z.Id, A.Objeto, Z.Tipo_Zona, Z.Grupo, Z.Activa, A.Estado "
                        "FROM TB_DOM_ALARM_PARTICION AS P, TB_DOM_ALARM_ZONA AS Z, TB_DOM_ASSIGN AS A "
                        "WHERE P.Id = Z.Particion AND A.Id = Z.Objeto_Zona AND "
                        "P.Id = %i;", particion);
        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
        rc = m_pDB->Query(json_EstadoZona, query);
        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
        if(rc >= 0)
        {
            cJSON_AddItemToObject(json_EstadoPart, "Zonas", json_EstadoZona);
        }

        json_EstadoSalida = cJSON_CreateArray();
        sprintf(query, "SELECT S.Id, A.Objeto, S.Tipo_Salida, A.Estado "
                        "FROM TB_DOM_ALARM_PARTICION AS P, TB_DOM_ALARM_SALIDA AS S, TB_DOM_ASSIGN AS A "
                        "WHERE P.Id = S.Particion AND A.Id = S.Objeto_Salida AND "
                        "P.Id = %i;", particion);
        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
        rc = m_pDB->Query(json_EstadoSalida, query);
        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
        if(rc >= 0)
        {
            cJSON_AddItemToObject(json_EstadoPart, "Salidas", json_EstadoSalida);
        }
    }

    if(json_EstadoPart)
    {
        json_EstadoAct = cJSON_GetObjectItemCaseSensitive(json_EstadoPart, "Estado_Activacion");
        estado_act = atoi(json_EstadoAct->valuestring);
        if(json_estado)
        {
            json_obj = cJSON_CreateObject();
            cJSON_AddItemToObject(json_obj, "response", json_EstadoPart);
            cJSON_PrintPreallocated(json_obj, json_estado, json_max, 0);
            cJSON_Delete(json_obj);
        }
        return estado_act;
    }

    return (-1);
}

int GEvent::Estado_Alarma(const char* particion, char* json_estado, int json_max)
{
    int rc;
    char query[4096];
    int estado_act;

    cJSON *json_Query_Result;
    cJSON *json_EstadoPart = nullptr;
    cJSON *json_EstadoZona;
    cJSON *json_EstadoSalida;
    cJSON *json_obj;
    cJSON *json_EstadoAct;

    m_pServer->m_pLog->Add(100, "[GEvent::Estado_Alarma] Part: %s", particion);

    json_Query_Result = cJSON_CreateArray();
    sprintf(query, "SELECT  Id, Nombre, Estado_Activacion, Estado_Memoria, Estado_Alarma "
                    "FROM TB_DOM_ALARM_PARTICION "
                    "WHERE Nombre = \'%s\';", particion);
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    rc = m_pDB->Query(json_Query_Result, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0)
    {
        /* Paso el primero y único */
        cJSON_ArrayForEach(json_EstadoPart, json_Query_Result) { break; }
    }

    if(rc > 0 && json_estado)
    {
        /* Información de Zonas */
        json_EstadoZona = cJSON_CreateArray();
        sprintf(query, "SELECT Z.Id, A.Objeto, Z.Tipo_Zona, Z.Grupo, Z.Activa, A.Estado "
                        "FROM TB_DOM_ALARM_PARTICION AS P, TB_DOM_ALARM_ZONA AS Z, TB_DOM_ASSIGN AS A "
                        "WHERE P.Id = Z.Particion AND A.Id = Z.Objeto_Zona AND "
                        "P.Nombre = \'%s\';", particion);
        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
        rc = m_pDB->Query(json_EstadoZona, query);
        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
        if(rc >= 0)
        {
            cJSON_AddItemToObject(json_EstadoPart, "Zonas", json_EstadoZona);
        }

        json_EstadoSalida = cJSON_CreateArray();
        sprintf(query, "SELECT S.Id, A.Objeto, S.Tipo_Salida, A.Estado "
                        "FROM TB_DOM_ALARM_PARTICION AS P, TB_DOM_ALARM_SALIDA AS S, TB_DOM_ASSIGN AS A "
                        "WHERE P.Id = S.Particion AND A.Id = S.Objeto_Salida AND "
                        "P.Nombre = \'%s\';", particion);
        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
        rc = m_pDB->Query(json_EstadoSalida, query);
        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
        if(rc >= 0)
        {
            cJSON_AddItemToObject(json_EstadoPart, "Salidas", json_EstadoSalida);
        }
    }

    if(json_EstadoPart)
    {
        json_EstadoAct = cJSON_GetObjectItemCaseSensitive(json_EstadoPart, "Estado_Activacion");
        estado_act = atoi(json_EstadoAct->valuestring);
        if(json_estado)
        {
            json_obj = cJSON_CreateObject();
            cJSON_AddItemToObject(json_obj, "response", json_EstadoPart);
            cJSON_PrintPreallocated(json_obj, json_estado, json_max, 0);
            cJSON_Delete(json_obj);
        }
        return estado_act;
    }

    return (-1);
}

int GEvent::Switch_Alarma(int particion)
{
    int estado = Estado_Alarma(particion, nullptr, 0);

    m_pServer->m_pLog->Add(20, "[GEvent::Switch_Alarma] Part: %i", particion);

    if(estado < 0) return (-1);

    if(estado == 0)
    {
        if(Activar_Alarma(particion, 1) != 0)
        {
            return (-1);
        }
        else
        {
            return 2;
        }
    }
    else
    {
        if(Desactivar_Alarma(particion) != 0)
        {
            return (-1);
        }
        else
        {
            return 0;
        }
    }
}

int GEvent::Switch_Alarma(const char* particion)
{
    int estado = Estado_Alarma(particion, nullptr, 0);

    m_pServer->m_pLog->Add(20, "[GEvent::Switch_Alarma] Part: %s", particion);

    if(estado < 0) return (-1);

    if(estado == 0)
    {
        if(Activar_Alarma(particion, 1) != 0)
        {
            return (-1);
        }
        else
        {
            return 2;
        }
    }
    else
    {
        if(Desactivar_Alarma(particion) != 0)
        {
            return (-1);
        }
        else
        {
            return 0;
        }
    }
}

int GEvent::Switch_Zona_Alarma(const char* particion, const char* zona)
{
    int rc;
    char query[4096];
    int estado_act;
    int id_part;
    int id_ass;

    cJSON *json_Query_Result;
    cJSON *json_Query_Row = nullptr;
    cJSON *json_EstadoAct;
    cJSON *json_PartId;
    cJSON *json_AssId;

    m_pServer->m_pLog->Add(20, "[GEvent::Switch_Zona_Alarma] Part: %s Zona: %s", particion, zona);

    json_Query_Result = cJSON_CreateArray();
    sprintf(query, "SELECT  Id, Estado_Activacion, Estado_Alarma "
                    "FROM TB_DOM_ALARM_PARTICION "
                    "WHERE Nombre = \'%s\';", particion);
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    rc = m_pDB->Query(json_Query_Result, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc >= 0)
    {
        /* Paso el primero y único */
        cJSON_ArrayForEach(json_Query_Row, json_Query_Result) { break; }
    }
    if( !json_Query_Row) return (-1);
    json_EstadoAct = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Estado_Activacion");
    json_PartId = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Id");
    /* Esto no se puede hacer con la alarma activada */
    estado_act = atoi(json_EstadoAct->valuestring);
    id_part = atoi(json_PartId->valuestring);
    cJSON_Delete(json_Query_Result);
    if(estado_act != 0) return (-1);

    /* Busco el Id del assign de la zona */
    json_Query_Result = cJSON_CreateArray();
    sprintf(query, "SELECT Id "
                    "FROM TB_DOM_ASSIGN "
                    "WHERE UPPER(Objeto) = UPPER(\'%s\');", zona);
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    rc = m_pDB->Query(json_Query_Result, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc >= 0)
    {
        /* Paso el primero y único */
        cJSON_ArrayForEach(json_Query_Row, json_Query_Result) { break; }
    }
    if( !json_Query_Row) return (-1);
    json_AssId = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Id");
    id_ass = atoi(json_AssId->valuestring);
    cJSON_Delete(json_Query_Result);

    /* Habilito / Desabilito la zona */
    sprintf(query, "UPDATE TB_DOM_ALARM_ZONA "
                    "SET Activa = (1 - Activa) "
                    "WHERE Particion = %i AND Objeto_Zona = %i;", id_part, id_ass);
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    rc = m_pDB->Query(nullptr, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0) m_alarm_need_update = 1;
    return rc;
}

int GEvent::Pulse_Salida_Alarma(const char* particion, const char* salida)
{
    int rc;
    char query[4096];
    //int estado_act;

    cJSON *json_Query_Result;
    cJSON *json_EstadoPart = nullptr;
    cJSON *json_EstadoAct;

    m_pServer->m_pLog->Add(20, "[GEvent::Pulse_Salida_Alarma] Part: %s Zona: %s", particion, salida);

    json_Query_Result = cJSON_CreateArray();
    sprintf(query, "SELECT  Id, Estado_Activacion, Estado_Alarma "
                    "FROM TB_DOM_ALARM_PARTICION "
                    "WHERE Nombre = \'%s\';", particion);
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    rc = m_pDB->Query(json_Query_Result, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc >= 0)
    {
        /* Paso el primero y único */
        cJSON_ArrayForEach(json_EstadoPart, json_Query_Result) { break; }
    }
    if( !json_EstadoPart) return (-1);
    json_EstadoAct = cJSON_GetObjectItemCaseSensitive(json_EstadoPart, "Estado_Activacion");
    /* Esto no se puede hacer con la alarma activada */
    if(atoi(json_EstadoAct->valuestring) != 0) return (-1);

    ChangeAssignByName(salida, 4, 30);

    return 0;
}

int GEvent::Estado_Alarma_General(char* json, int max_len)
{
    int rc;
    char query[4096];
    int part = 0;
 
    cJSON *json_Result;
    cJSON *json_Result_Item;
    cJSON *json_Result_Array;
    cJSON *json_Part_Result;
    cJSON *json_Part_Row;
    cJSON *json_Zona_Result;
    cJSON *json_Salida_Result;
    cJSON *json_Part_Id;
    
    m_pServer->m_pLog->Add(100, "[GEvent::Estado_Alarma_General]");
    json_Result_Array = cJSON_CreateArray();

    json_Part_Result = cJSON_CreateArray();
    sprintf(query, "SELECT  Id, Nombre, Estado_Activacion, Estado_Memoria, Estado_Alarma "
                    "FROM TB_DOM_ALARM_PARTICION "
                    "WHERE Id > 0;");
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    rc = m_pDB->Query(json_Part_Result, query);
    m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
    if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc > 0)
    {
        cJSON_ArrayForEach(json_Part_Row, json_Part_Result)
        {
            part++;
            json_Part_Id = cJSON_GetObjectItemCaseSensitive(json_Part_Row, "Id");
            json_Result_Item =  cJSON_Duplicate(json_Part_Row, true);
            /* Información de Zonas */
            json_Zona_Result = cJSON_CreateArray();
            sprintf(query, "SELECT Z.Id, A.Objeto, Z.Tipo_Zona, Z.Grupo, Z.Activa, A.Estado "
                            "FROM TB_DOM_ALARM_ZONA AS Z, TB_DOM_ASSIGN AS A "
                            "WHERE A.Id = Z.Objeto_Zona AND "
                            "Z.Particion = %s;", json_Part_Id->valuestring);
            m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
            rc = m_pDB->Query(json_Zona_Result, query);
            m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
            if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
            if(rc > 0)
            {
                cJSON_AddItemToObject(json_Result_Item, "Zonas", json_Zona_Result);
            }

            /* Información de Salidas */
            json_Salida_Result = cJSON_CreateArray();
            sprintf(query, "SELECT S.Id, A.Objeto, S.Tipo_Salida, A.Estado "
                            "FROM TB_DOM_ALARM_SALIDA AS S, TB_DOM_ASSIGN AS A "
                            "WHERE A.Id = S.Objeto_Salida AND "
                            "S.Particion = %s;", json_Part_Id->valuestring);
            m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
            rc = m_pDB->Query(json_Salida_Result, query);
            m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
            if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
            if(rc > 0)
            {
                cJSON_AddItemToObject(json_Result_Item, "Salidas", json_Salida_Result);
            }

            /* Agrego todo al array de salida */
            cJSON_AddItemToArray(json_Result_Array, json_Result_Item);
        }
    }
    json_Result = cJSON_CreateObject();
    cJSON_AddItemToObject(json_Result, "Alarma", json_Result_Array);
    cJSON_PrintPreallocated(json_Result, json, max_len, 0);
    cJSON_Delete(json_Result);
    return part;
}

int GEvent::AlarmNeedUpdate( void )
{
    if(m_alarm_need_update)
    {
        m_alarm_need_update = 0;
        return 1;
    }
    else return 0;
}
