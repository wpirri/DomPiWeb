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

GEvent::GEvent(CDB *pDB, CGMServerWait *pServer)
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
    char s[256];
    time_t t;
    int rc;
    unsigned int ival;
    char query[4096];
    cJSON *json_obj;
    cJSON *json_arr;
    cJSON *json_hw_id;
    cJSON *json_hw_mac;
    cJSON *json_raddr;
    cJSON *json_chg;
    cJSON *json_status;
    cJSON *json_un_obj;
    STRFunc str;

    m_pServer->m_pLog->Add(100, "[ExtIOEvent] json_evt: %s", json_evt);

    json_obj = cJSON_Parse(json_evt);
    if(json_obj)
    {
        json_hw_mac = cJSON_GetObjectItemCaseSensitive(json_obj, "ID");
        json_raddr = cJSON_GetObjectItemCaseSensitive(json_obj, "REMOTE_ADDR");
        if(json_hw_mac && cJSON_IsString(json_hw_mac))
        {
            t = time(&t);

            /* Busco el ID para relacionar con la tabla de assigns */
            sprintf(query, "SELECT Id, Estado FROM TB_DOM_PERIF WHERE MAC = \"%s\"", json_hw_mac->valuestring);
            m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
            json_arr = cJSON_CreateArray();
            rc = m_pDB->Query(json_arr, query);
            if(rc == 0)
            {
                json_hw_id = cJSON_GetObjectItemCaseSensitive(json_arr->child, "Id");
                json_status = cJSON_GetObjectItemCaseSensitive(json_arr->child, "Estado");
                if(json_raddr)
                {
                    if(atoi(json_status->valuestring) == 0)
                    {
                        m_pServer->m_pLog->Add(10, "[HW] %s %s Estado: ON LINE", json_hw_mac->valuestring, json_raddr->valuestring);
                    }
                    /* Actualizo la tabla de Dispositivos */
                    sprintf(query, "UPDATE TB_DOM_PERIF "
                                        "SET Ultimo_Ok = %lu, "
                                        "Direccion_IP = \"%s\", "
                                        "Estado = 1 "
                                        "WHERE MAC = \"%s\"",
                                        t,
                                        json_raddr->valuestring,
                                        json_hw_mac->valuestring);
                    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                    m_pDB->Query(NULL, query);
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
                                                        "WHERE Dispositivo = \"%s\" AND Port = \"%s\"",
                                                        ival,
                                                        json_hw_id->valuestring,
                                                        json_un_obj->string);
                                        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                                        m_pDB->Query(NULL, query);
                                        /* En las entradas actualizo también el estado a mostrar */
                                        sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Estado = %i "
                                                        "WHERE Dispositivo = \"%s\" AND Port = \"%s\" AND "
                                                        "(Tipo = 1 OR Tipo = 2 OR Tipo = 4)",
                                                        ival,
                                                        json_hw_id->valuestring,
                                                        json_un_obj->string);
                                        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                                        m_pDB->Query(NULL, query);
                                    }
                                    else if( !memcmp(json_un_obj->string, "WIEGAND", 7))
                                    {
                                        /* TODO: Tratamiento de tarjeta inalambrica */


                                    }
                                    else if( !memcmp(json_un_obj->string, "TEMP", 4))
                                    {
                                        sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Estado = (%s * 100) "
                                                        "WHERE Dispositivo = \"%s\" AND Port = \"%s\"",
                                                        json_un_obj->valuestring,
                                                        json_hw_id->valuestring,
                                                        json_un_obj->string);
                                        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                                        m_pDB->Query(NULL, query);
                                    }
                                    else if( !memcmp(json_un_obj->string, "HUM", 3))
                                    {
                                        sprintf(query,  "UPDATE TB_DOM_ASSIGN SET Estado = (%s * 100) "
                                                        "WHERE Dispositivo = \"%s\" AND Port = \"%s\"",
                                                        json_un_obj->valuestring,
                                                        json_hw_id->valuestring,
                                                        json_un_obj->string);
                                        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                                        m_pDB->Query(NULL, query);
                                    }
                                }
                            }
                        }
                        json_un_obj = json_un_obj->next;
                    }
                }

                /* Cambios informados */
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
                    /* TODO: Si no se informaron cambios */



                }
                cJSON_Delete(json_arr);
                cJSON_Delete(json_obj);
                return 1;
            }
            else
            {
                m_pServer->m_pLog->Add(10, "[HW] %s %s Desconocido", json_hw_mac->valuestring, (json_raddr)?json_raddr->valuestring:"-");
                cJSON_Delete(json_arr);
                cJSON_Delete(json_obj);
                return 0;
            }
        }
        cJSON_Delete(json_obj);
        return 0;
    }
    return 0;
}

int GEvent::CheckEvent(int hw_id, const char* port, int estado)
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
//    cJSON *Flags;

    m_pServer->m_pLog->Add(50, "[CheckEvent] HW: %i Port: %s Estado: %s", 
                                hw_id, port, (estado)?"ON":"OFF");

    /* Busco si hay un evento para este cambio */
    json_arr = cJSON_CreateArray();
    sprintf(query, "SELECT EV.Evento, EV.Objeto_Destino, EV.Grupo_Destino, EV.Funcion_Destino, EV.Variable_Destino, "
                    "EV.Enviar, EV.Parametro_Evento, EV.Condicion_Variable, EV.Condicion_Igualdad, EV.Condicion_Valor, EV.Flags "
                    "FROM TB_DOM_EVENT AS EV, TB_DOM_ASSIGN AS ASS "
                    "WHERE EV.Objeto_Origen = ASS.Id AND "
                    "ASS.Dispositivo = %i AND ASS.Port = \"%s\" AND %s",
                    hw_id, port, (estado)?"OFF_a_ON = 1":"ON_a_OFF = 1");
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
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
            //Flags = cJSON_GetObjectItemCaseSensitive(json_obj, "Flags");

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

    m_pServer->m_pLog->Add(90, "[SendEventObj] ass id: %i - ev: %i - val: %i", id, ev, val);

    json_arr = cJSON_CreateArray();
    sprintf(query,  "SELECT HW.Direccion_IP, HW.Tipo AS Tipo_HW, ASS.Tipo AS Tipo_ASS, ASS.Port "
                    "FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS ASS "
                    "WHERE HW.Id = ASS.Dispositivo AND "
                    "ASS.Id = %i", id);
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    rc = m_pDB->Query(json_arr, query);
    if(rc == 0)
    {
        cJSON_ArrayForEach(json_obj, json_arr)
        {
            cJSON_PrintPreallocated(json_obj, query, 4095, 0);
            m_pServer->m_pLog->Add(100, "[RESULT]: %s", query); 

            switch(ev)
            {
                case 1:     /* On */
                    /* Actualizo el estado en la base */
                    sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
                                    "SET Estado = 1 "
                                    "WHERE Id = %i", id);
                    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                    m_pDB->Query(NULL, query);
                    break;
                case 2:     /* Off */
                    /* Actualizo el estado en la base */
                    sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
                                    "SET Estado = 0 "
                                    "WHERE Id = %i", id);
                    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                    m_pDB->Query(NULL, query);
                    break;
                case 3:     /* Switch */
                    /* Actualizo el estado en la base */
                    sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
                                    "SET Estado = (1 - Estado) "
                                    "WHERE Id = %i", id);
                    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                    m_pDB->Query(NULL, query);
                    break;
                case 4:     /* Pulso */
                    /* Actualizo el estado en la base */
                    sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
                                    "SET Estado = %i "
                                    "WHERE Id = %i", 2 + val, id);
                    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                    m_pDB->Query(NULL, query);
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
    m_pServer->m_pLog->Add(90, "[SendEventGrp] id: %i - ev: %i - val: %i", id, ev, val);
    m_pServer->m_pLog->Add(1, "[SendEventGrp] ERROR - Falta desarrollar");

    return 0;
}

int GEvent::SendEventFun(int id, int ev, int val)
{
    m_pServer->m_pLog->Add(90, "[SendEventFun] id: %i - ev: %i - val: %i", id, ev, val);
    m_pServer->m_pLog->Add(1, "[SendEventFun] ERROR - Falta desarrollar");

    return 0;
}

int GEvent::SendEventVar(int id, int ev, int val)
{
    m_pServer->m_pLog->Add(90, "[SendEventVar] id: %i - ev: %i - val: %i", id, ev, val);
    m_pServer->m_pLog->Add(1, "[SendEventVar] ERROR - Falta desarrollar");

    return 0;
}
