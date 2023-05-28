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
    cJSON *json_QueryArray;
    cJSON *json_hw_id;
    cJSON *json_hw_mac;
    cJSON *json_raddr;
    cJSON *json_chg;
    cJSON *json_status;
    cJSON *json_un_obj;
    STRFunc str;

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

                                        CheckAuto(atoi(json_hw_id->valuestring), json_un_obj->string, ival);
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

                                        CheckAuto(atoi(json_hw_id->valuestring), json_un_obj->string, ival);
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
                    /* TODO: Si no se informaron cambios */



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

void GEvent::CheckEvent(int hw_id, const char* port, int estado)
{
	char query[4096];
    int rc;
    cJSON *json_QueryArray;
    cJSON *json_QueryRow;

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
    json_QueryArray = cJSON_CreateArray();
    sprintf(query, "SELECT EV.Evento, EV.Objeto_Destino, EV.Grupo_Destino, EV.Funcion_Destino, EV.Variable_Destino, "
                    "EV.Enviar, EV.Parametro_Evento, EV.Condicion_Variable, EV.Condicion_Igualdad, EV.Condicion_Valor, EV.Flags "
                    "FROM TB_DOM_EVENT AS EV, TB_DOM_ASSIGN AS ASS "
                    "WHERE EV.Objeto_Origen = ASS.Id AND "
                    "ASS.Dispositivo = %i AND ASS.Port = \'%s\' AND %s",
                    hw_id, port, (estado)?"OFF_a_ON = 1;":"ON_a_OFF = 1;");
    m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
    rc = m_pDB->Query(json_QueryArray, query);
	m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc >= 0)
    {
        /* Recorro el array */
        cJSON_ArrayForEach(json_QueryRow, json_QueryArray)
        {
            cJSON_PrintPreallocated(json_QueryRow, query, 4095, 0);
            m_pServer->m_pLog->Add(50, "[EVENTO]: %s", query); 

            Objeto_Destino = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Objeto_Destino");
            Grupo_Destino = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Grupo_Destino");
            Funcion_Destino = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Funcion_Destino");
            Variable_Destino = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Variable_Destino");
            Enviar = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Enviar");
            Parametro_Evento = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Parametro_Evento");
            Condicion_Variable = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Condicion_Variable");
            Condicion_Igualdad = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Condicion_Igualdad");
            Condicion_Valor = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Condicion_Valor");
            //Flags = cJSON_GetObjectItemCaseSensitive(json_QueryRow, "Flags");

            /* TODO: Evaluar condiciones */
            if(Condicion_Variable && Condicion_Igualdad && Condicion_Valor)
            {



            }

            /* Si la condicion lo permite ejecuto según corresponda */
            if( rc >= 0 && Enviar )
            {
                if(Objeto_Destino &&  atoi(Objeto_Destino->valuestring) > 0 )
                {
                    ChangeAssignById(   atoi(Objeto_Destino->valuestring), 
                                    atoi(Enviar->valuestring),
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                }
                else if(Grupo_Destino &&  atoi(Grupo_Destino->valuestring) > 0 )
                {
                    ChangeGroupById(   atoi(Grupo_Destino->valuestring), 
                                    atoi(Enviar->valuestring),
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                }
                else if(Funcion_Destino &&  atoi(Funcion_Destino->valuestring) > 0 )
                {
                    ChangeFcnById(   atoi(Funcion_Destino->valuestring), 
                                    atoi(Enviar->valuestring),
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                }
                else if(Variable_Destino &&  atoi(Variable_Destino->valuestring) > 0 )
                {
                    ChangeVarById(   atoi(Variable_Destino->valuestring), 
                                    atoi(Enviar->valuestring),
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                }
            }
        }
    }
    cJSON_Delete(json_QueryArray);
}

void GEvent::CheckAuto(int hw_id, const char* port, int estado_sensor)
{
	char query[4096];
    int rc;
	time_t t;
	struct tm *lt;
	char dia[3];
    int enviar;
    int set_estado;

    cJSON *json_QueryArray;
    cJSON *json_QueryRow;
	
    cJSON *Id;
    cJSON *Objeto;
    cJSON *Objeto_Salida;
    cJSON *Objeto_Sensor;
    cJSON *Grupo_Salida;
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

    m_pServer->m_pLog->Add(100, "[CheckAuto] HW: %i Port: %s Estado: %i", hw_id, (port)?port:"NULL", estado_sensor);

    /* Busco si hay un evento para este cambio */
    json_QueryArray = cJSON_CreateArray();
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
    rc = m_pDB->Query(json_QueryArray, query);
	m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
	if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
    if(rc >= 0)
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
                    ChangeAssignById(   atoi(Objeto_Salida->valuestring), enviar,
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);

                }
                else if(Grupo_Salida &&  atoi(Grupo_Salida->valuestring) > 0 )
                {
                    ChangeGroupById(   atoi(Grupo_Salida->valuestring), enviar,
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                }
                else if(Funcion_Salida &&  atoi(Funcion_Salida->valuestring) > 0 )
                {
                    ChangeFcnById(   atoi(Funcion_Salida->valuestring), enviar,
                                    (Parametro_Evento)?atoi(Parametro_Evento->valuestring):0);
                }
                else if(Variable_Salida &&  atoi(Variable_Salida->valuestring) > 0 )
                {
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
            }
        }
    }

    cJSON_Delete(json_QueryArray);
}

int GEvent::ChangeAssignByName(const char* name, int accion, int param)
{
	int rc = 0;
	char query[4096];

	m_pServer->m_pLog->Add(20, "[ChangeAssignByName] name= %s, accion= %i param= %i", name, accion, param);
	switch(accion)
	{
		case 1: /* Encender */
			sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
							"SET Estado = 1 "
							"WHERE UPPER(Objeto) = UPPER(\'%s\');", name);
			break;
		case 2: /* Apagar */
			sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
							"SET Estado = 0 "
							"WHERE UPPER(Objeto) = UPPER(\'%s\');", name);
			break;
		case 3:	/* Switch */
			sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
							"SET Estado = (1 - Estado) "
							"WHERE UPPER(Objeto) = UPPER(\'%s\');", name);
			break;
		case 4: /* Pulso */
			if(param > 0)
			{
				sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
								"SET Estado = %i "
								"WHERE UPPER(Objeto) = UPPER(\'%s\');", max(2, param), name);
			}
			else
			{
				sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
								"SET Estado = Analog_Mult_Div_Valor "
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

	m_pServer->m_pLog->Add(20, "[ChangeAssignById] id= %i, accion= %i param= %i", id, accion, param);
	switch(accion)
	{
		case 1: /* Encender */
			sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
							"SET Estado = 1 "
							"WHERE Id = %i;", id);
			break;
		case 2: /* Apagar */
			sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
							"SET Estado = 0 "
							"WHERE Id = %i;", id);
			break;
		case 3:	/* Switch */
			sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
							"SET Estado = (1 - Estado) "
							"WHERE Id = %i;", id);
			break;
		case 4: /* Pulso */
			if(param > 0)
			{
				sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
								"SET Estado = %i "
							    "WHERE Id = %i;", max(2, param), id);
			}
			else
			{
				sprintf(query, 	"UPDATE TB_DOM_ASSIGN "
								"SET Estado = MAX(2, Analog_Mult_Div_Valor) "
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

	m_pServer->m_pLog->Add(20, "[ChangeGroupByName] name= %s, accion= %i param= %i", name, accion, param);

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
			if(param > 0)
			{
				sprintf(query, 	"UPDATE TB_DOM_GROUP "
								"SET Estado = %i, Actualizar = 1 "
                                "WHERE UPPER(Grupo) = UPPER(\'%s\');", max(2, param), name);
			}
			else
			{
				sprintf(query, 	"UPDATE TB_DOM_GROUP "
								"SET Estado = MAX(2, Analog_Mult_Div_Valor), Actualizar = 1" 
                                "WHERE UPPER(Grupo) = UPPER(\'%s\');", name);
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

int GEvent::ChangeGroupById(int id, int accion, int param)
{
	int rc = 0;
	char query[4096];

	m_pServer->m_pLog->Add(20, "[ChangeGroupById] id= %i, accion= %i param= %i", id, accion, param);

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
			if(param > 0)
			{
				sprintf(query, 	"UPDATE TB_DOM_GROUP "
								"SET Estado = %i, Actualizar = 1 "
							    "WHERE Id = %i;", max(2, param), id);
			}
			else
			{
				sprintf(query, 	"UPDATE TB_DOM_GROUP "
								"SET Estado = MAX(2, Analog_Mult_Div_Valor), Actualizar = 1" 
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

	m_pServer->m_pLog->Add(20, "[ChangeAutoByName] name= %s, accion= %i param= %i", name, accion, param);
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

	m_pServer->m_pLog->Add(20, "[ChangeAutoById] id= %i, accion= %i param= %i", id, accion, param);
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
