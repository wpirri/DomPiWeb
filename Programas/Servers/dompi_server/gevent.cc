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
    m_change_ass_count = 0;
}

GEvent::~GEvent()
{

}

int GEvent::ExtIOEvent(const char* json_evt)
{
    int i;
    int mask;
    char hw_mac[16];
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
    int hw_count;
    char query[4096];
    char sql_set[4096];
    cJSON *json_obj;
    cJSON *json_arr;
    cJSON *json_hw_id;
    cJSON *json_hw_mac;
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
    hw_count = 0;

    m_pServer->m_pLog->Add(100, "[ExtIOEvent] json_evt: %s", json_evt);

    json_obj = cJSON_Parse(json_evt);
    if(json_obj)
    {
        json_hw_mac = cJSON_GetObjectItemCaseSensitive(json_obj, "HW_ID");
        if(json_hw_mac)
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
            if(json_hw_mac && cJSON_IsString(json_hw_mac))
            {
                strcpy(hw_mac, json_hw_mac->valuestring);
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

            m_pServer->m_pLog->Add(10, "[HW] %s %s", hw_mac, remote_addr);

            /* Actualizo la tabla de Dispositivos */
            sprintf(query, "UPDATE TB_DOM_PERIF "
                                "SET Ultimo_Ok  = \"%04i-%02i-%02i %02i:%02i:%02i\", "
                                  "Direccion_IP = \"%s\""
                                  "%s "
                                "WHERE MAC = \"%s\";",
                                p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
                                p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec, 
                                remote_addr,
                                sql_set,
                                hw_mac);
            m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
            hw_count = m_pDB->Query(NULL, query);


            if(hw_count > 0)
            {
                /* Busco el ID para relacionar con la tabla de assigns */
                sprintf(query, "SELECT ID FROM TB_DOM_PERIF WHERE MAC = \"%s\";", hw_mac);
                m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
                json_arr = cJSON_CreateArray();
                rc = m_pDB->Query(json_arr, query);
                if(rc == 0)
                {
                    json_hw_id = cJSON_GetObjectItemCaseSensitive(json_arr->child, "Id");

                    /* Actualizo los assign correspondientes */
                    if(json_status_a && cJSON_IsString(json_status_a))
                    {
                        /* Para los bits 0 a 15 */
                        for(i = 0; i < 16; i++)
                        {
                            mask = pow(2, i);   /* Armo la mascara */
                            sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Estado = %i "
                                            "WHERE Dispositivo = \"%s\" AND Port = 1 AND E_S = %i",
                                            (status_a & mask)?1:0,
                                            json_hw_id->valuestring,
                                            i+1);
                            m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
                            m_pDB->Query(NULL, query);
                        }
                    }
                    if(json_status_b && cJSON_IsString(json_status_b))
                    {
                        /* Para los bits 0 a 15 */
                        for(i = 0; i < 16; i++)
                        {
                            mask = pow(2, i);   /* Armo la mascara */
                            sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Estado = %i "
                                            "WHERE Dispositivo = \"%s\" AND Port = 2 AND E_S = %i",
                                            (status_b & mask)?1:0,
                                            json_hw_id->valuestring,
                                            i+1);
                            m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
                            m_pDB->Query(NULL, query);
                        }
                    }
                    if(json_status_c && cJSON_IsString(json_status_c))
                    {
                        /* Para los bits 0 a 15 */
                        for(i = 0; i < 16; i++)
                        {
                            mask = pow(2, i);   /* Armo la mascara */
                            sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Estado = %i "
                                            "WHERE Dispositivo = \"%s\" AND Port = 3 AND E_S = %i",
                                            (status_c & mask)?1:0,
                                            json_hw_id->valuestring,
                                            i+1);
                            m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
                            m_pDB->Query(NULL, query);
                        }
                    }


                    /* Si hay cambios busco si hay que enviar eventos */
                    if(delta_a >= 0)
                    {
                        /* Para los bits 0 a 15 */
                        for(i = 0; i < 16; i++)
                        {
                            mask = pow(2, i);   /* Armo la mascara */
                            if(delta_a & mask)  /* Si cambió .... */
                            {
                                /* Busco si hay evento  - Port A=1, B=2, C=3 - Entrada 1 a 16 */
                                CheckEvent(hw_mac, 1 /* PORT A */, i+1, (status_a & mask)?1:0);
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
                                /* Busco si hay evento  - Port A=1, B=2, C=3 - Entrada 1 a 16 */
                                CheckEvent(hw_mac, 2 /* PORT B */, i+1, (status_b & mask)?1:0);
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
                                /* Busco si hay evento  - Port A=1, B=2, C=3 - Entrada 1 a 16 */
                                CheckEvent(hw_mac, 3 /* PORT C */, i+1, (status_c & mask)?1:0);
                            }
                        }
                    }
                }
                cJSON_Delete(json_arr);
            }
        }
        cJSON_Delete(json_obj);
    }
    return hw_count;
}

int GEvent::CheckEvent(const char *hw_mac, int port, int e_s, int estado)
{
	char query[4096];
    int rc;
    cJSON *json_arr;
    cJSON *json_obj;

    //cJSON *Evento;
    cJSON *Objeto_Destino;
    cJSON *Grupo_Destino;
    cJSON *Funcion_Destino;
    cJSON *Variable_Destino;
    cJSON *Enviar;
    cJSON *Parametro_Evento;
    cJSON *Condicion_Variable;
    cJSON *Condicion_Igualdad;
    cJSON *Condicion_Valor;
    cJSON *Flags;

    m_pServer->m_pLog->Add(10, "Cambio de estado - CheckEvent: HW: %s Port: %s E/S: %i Estado: %s", 
                                hw_mac, (port==1)?"A":(port==2)?"B":(port==3)?"C":"?",
                                e_s, (estado)?"ON":"OFF");

    m_change_ass_count++;

    if(m_change_ass_count > 10)
    {
        /* Notifico el estado de todos los objetos */


        m_change_ass_count = 0;
    }
    else
    {
        /* Notifico el cambio de estado si corresponde a un assign */
        json_arr = cJSON_CreateArray();
        sprintf(query,  "SELECT ASS.Id, ASS.Objeto, ASS.Tipo, ASS.Estado "
                        "FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
                        "WHERE ASS.Dispositivo = HW.Id AND "
                        "HW.MAC = \"%s\" AND ASS.Port = %i AND ASS.E_S = %i;",
                        hw_mac, port, e_s);
        m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
        rc = m_pDB->Query(json_arr, query);
        if(rc == 0)
        {
            json_obj = json_arr->child;

            /* si hay que agregar algo mas */

            cJSON_PrintPreallocated(json_obj, query, 4095, 0);
            m_pServer->m_pLog->Add(50, "[Post]: %s", query); 
            m_pServer->Post("dompi_ass_change", query, strlen(query));
        }
        cJSON_Delete(json_arr);
    }


    /* Busco si hay un assign y si hay evento para ese assign */
    json_arr = cJSON_CreateArray();
    sprintf(query, "SELECT EV.Evento, EV.Objeto_Destino, EV.Grupo_Destino, EV.Funcion_Destino, EV.Variable_Destino, "
                    "EV.Enviar, EV.Parametro_Evento, EV.Condicion_Variable, EV.Condicion_Igualdad, EV.Condicion_Valor, EV.Flags "
                    "FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS, TB_DOM_EVENT AS EV "
                    "WHERE EV.Objeto_Origen = ASS.Id AND ASS.Dispositivo = HW.Id AND "
                    "HW.MAC = \"%s\" AND ASS.Port = %i AND ASS.E_S = %i AND %s;",
                    hw_mac, port, e_s, (estado)?"OFF_a_ON = 1":"ON_a_OFF = 1");
    m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
    rc = m_pDB->Query(json_arr, query);
    if(rc == 0)
    {
        /* Recorro el array */
        cJSON_ArrayForEach(json_obj, json_arr)
        {
            cJSON_PrintPreallocated(json_obj, query, 4095, 0);
            m_pServer->m_pLog->Add(50, "[EVENTO]: %s", query); 

            Objeto_Destino = cJSON_GetObjectItemCaseSensitive(json_obj, "Objeto_Destino");
            Grupo_Destino = cJSON_GetObjectItemCaseSensitive(json_obj, "Grupo_Destino");
            Funcion_Destino = cJSON_GetObjectItemCaseSensitive(json_obj, "Funcion_Destino");
            Variable_Destino = cJSON_GetObjectItemCaseSensitive(json_obj, "Variable_Destino");
            Enviar = cJSON_GetObjectItemCaseSensitive(json_obj, "Enviar");
            Parametro_Evento = cJSON_GetObjectItemCaseSensitive(json_obj, "Parametro_Evento");
            Condicion_Variable = cJSON_GetObjectItemCaseSensitive(json_obj, "Condicion_Variable");
            Condicion_Igualdad = cJSON_GetObjectItemCaseSensitive(json_obj, "Condicion_Igualdad");
            Condicion_Valor = cJSON_GetObjectItemCaseSensitive(json_obj, "Condicion_Valor");
            Flags = cJSON_GetObjectItemCaseSensitive(json_obj, "Flags");

            /* TODO: Evaluar condiciones */
            if(Condicion_Variable && Condicion_Igualdad && Condicion_Valor)
            {



            }

            /* Si la condicion lo permite ejecuto según corresponda */
            if( rc == 0 && Enviar )
            {
                if(Objeto_Destino &&  atoi(Objeto_Destino->valuestring) > 0 )
                {
                    SendEventObj(   atoi(Objeto_Destino->valuestring), 
                                    atoi(Enviar->valuestring),
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                }
                else if(Grupo_Destino &&  atoi(Grupo_Destino->valuestring) > 0 )
                {
                    SendEventGrp(   atoi(Objeto_Destino->valuestring), 
                                    atoi(Enviar->valuestring),
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                }
                else if(Funcion_Destino &&  atoi(Funcion_Destino->valuestring) > 0 )
                {
                    SendEventFun(   atoi(Objeto_Destino->valuestring), 
                                    atoi(Enviar->valuestring),
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                }
                else if(Variable_Destino &&  atoi(Variable_Destino->valuestring) > 0 )
                {
                    SendEventVar(   atoi(Objeto_Destino->valuestring), 
                                    atoi(Enviar->valuestring),
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                }
            }
        }
    }

    cJSON_Delete(json_arr);

    return 0;
}

int GEvent::SendEventObj(int id, int ev, int val)
{
	char query[4096];
    int rc;
    cJSON *json_arr;
    cJSON *json_obj;
    CGMServerBase::GMIOS call_resp;


    m_pServer->m_pLog->Add(100, "[SendEventObj] id: %i - ev: %i - val: %i", id, ev, val);

    json_arr = cJSON_CreateArray();
    sprintf(query,  "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port, ASS.E_S "
                    "FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
                    "WHERE HW.Id = ASS.Dispositivo AND "
                    "ASS.Id = %i", id);
    m_pServer->m_pLog->Add(50, "[QUERY][%s]", query);
    rc = m_pDB->Query(json_arr, query);
    if(rc == 0)
    {
        cJSON_ArrayForEach(json_obj, json_arr)
        {
            cJSON_PrintPreallocated(json_obj, query, 4095, 0);
            m_pServer->m_pLog->Add(50, "[RESULT]: %s", query); 

            switch(ev)
            {
                case 1:     /* On */
                    cJSON_AddStringToObject(json_obj, "Estado", "1");
                    cJSON_PrintPreallocated(json_obj, query, 4096, 0);
                    m_pServer->m_pLog->Add(50, "[dompi_hw_set_io]>>[%s]", query);
                    rc = m_pServer->Call("dompi_hw_set_io", query, strlen(query), &call_resp, 500);
                    if(rc == 0)
                    {
                        m_pServer->m_pLog->Add(50, "[dompi_hw_set_io]<<[%s]", (const char*)call_resp.data);
                    }
                    else
                    {
                        m_pServer->m_pLog->Add(50, "[dompi_hw_set_io]<<[Error]");
                    }
                    m_pServer->Free(call_resp);
                    break;
                case 2:     /* Off */
                    cJSON_AddStringToObject(json_obj, "Estado", "0");
                    cJSON_PrintPreallocated(json_obj, query, 4096, 0);
                    m_pServer->m_pLog->Add(50, "[dompi_hw_set_io]>>[%s]", query);
                    rc = m_pServer->Call("dompi_hw_set_io", query, strlen(query), &call_resp, 500);
                    if(rc == 0)
                    {
                        m_pServer->m_pLog->Add(50, "[dompi_hw_set_io]<<[%s]", (const char*)call_resp.data);
                    }
                    else
                    {
                        m_pServer->m_pLog->Add(50, "[dompi_hw_set_io]<<[Error]");
                    }
                    m_pServer->Free(call_resp);
                    break;
                case 3:     /* Switch */
                    cJSON_PrintPreallocated(json_obj, query, 4096, 0);
                    m_pServer->m_pLog->Add(50, "[dompi_hw_switch_io]>>[%s]", query);
                    rc = m_pServer->Call("dompi_hw_switch_io", query, strlen(query), &call_resp, 500);
                    if(rc == 0)
                    {
                        m_pServer->m_pLog->Add(50, "[dompi_hw_switch_io]<<[%s]", (const char*)call_resp.data);
                    }
                    else
                    {
                        m_pServer->m_pLog->Add(50, "[dompi_hw_switch_io]<<[Error]");
                    }
                    m_pServer->Free(call_resp);
                    break;
                case 4:     /* Pulso */
                    sprintf(query, "%i", (val > 0)?val:1);
                    cJSON_AddStringToObject(json_obj, "Segundos", query);
                    cJSON_PrintPreallocated(json_obj, query, 4096, 0);
                    m_pServer->m_pLog->Add(50, "[dompi_hw_pulse_io][%s]", query);
                    rc = m_pServer->Call("dompi_hw_pulse_io", query, strlen(query), &call_resp, 500);
                    if(rc == 0)
                    {
                        m_pServer->m_pLog->Add(50, "[dompi_hw_pulse_io]<<[%s]", (const char*)call_resp.data);
                    }
                    else
                    {
                        m_pServer->m_pLog->Add(50, "[dompi_hw_pulse_io]<<[Error]");
                    }
                    m_pServer->Free(call_resp);
                    break;
                default:
                    break;
            }
        }
    }
    cJSON_Delete(json_arr);
    return 0;
}

int GEvent::SendEventGrp(int id, int ev, int val)
{
    m_pServer->m_pLog->Add(100, "[SendEventGrp] id: %i - ev: %i - val: %i", id, ev, val);

    return 0;
}

int GEvent::SendEventFun(int id, int ev, int val)
{
    m_pServer->m_pLog->Add(100, "[SendEventFun] id: %i - ev: %i - val: %i", id, ev, val);

    return 0;
}

int GEvent::SendEventVar(int id, int ev, int val)
{
    m_pServer->m_pLog->Add(100, "[SendEventVar] id: %i - ev: %i - val: %i", id, ev, val);

    return 0;
}
