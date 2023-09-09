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
#include "alarma.h"

CAlarma::CAlarma(CDB *pDB, GEvent *pEV, CGMServerWait *pServer)
{
    m_pDB = pDB;
    m_pEV = pEV;
    m_pServer = pServer;
}

CAlarma::~CAlarma()
{

}

int CAlarma::Activar(int particion)
{
  int rc;
  char query[4096];

  cJSON *json_Query_Result;
  cJSON *json_Query_Row;
  cJSON *json_Objeto;

  int zona_abierta = 0;

  /* Busco zonas abiertas */
  sprintf(query, "SELECT A.Objeto "
                    "FROM TB_DOM_ALARM_ZONA AS Z, TB_DOM_ASSIGN AS A "
                    "WHERE Z.Objeto_Zona = A.Id AND Z.Activa <> 0 AND A.Estado <> 0 AND Z.Particion = %i;", particion);
  m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
  json_Query_Result = cJSON_CreateArray();
  rc = m_pDB->Query(json_Query_Result, query);
  m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
  if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
  if(rc > 0)
  {
    cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
    {
      json_Objeto = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Objeto");
      m_pServer->m_pLog->Add(10, "[ALARMA] Error al Activar. Zona %s abierta", json_Objeto->valuestring);
      zona_abierta++;
    } /* cJSON_ArrayForEach */
  }
  cJSON_Delete(json_Query_Result);

  if(zona_abierta)
  {
      m_pServer->m_pLog->Add(1, "[ALARMA] Error al Activar, %i zonas abiertas", zona_abierta);
      return zona_abierta;
  }

  /* Seteo el estado de alarma en la particion */
  sprintf(query, "UPDATE TB_DOM_ALARM_PARTICION "
                  "SET Estado_Activacion = 2, Estado_Memoria = 0 "
                  "WHERE Id = %i;",
                  particion);
  m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
  rc = m_pDB->Query(nullptr, query);
  m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
  if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);

  return 0;
}

int CAlarma::Desactivar(int particion)
{
  int rc;
  char query[4096];

  cJSON *json_Query_Result;
  cJSON *json_Query_Row;
  cJSON *json_Objeto_Salida;
  cJSON *json_Tipo_Salida;

  /* Apago la salida de alarma de la particion */
  sprintf(query, "SELECT * FROM TB_DOM_ALARM_SALIDA "
                    "WHERE Particion = %i;", particion);
  m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
  json_Query_Result = cJSON_CreateArray();
  rc = m_pDB->Query(json_Query_Result, query);
  m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
  if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
  if(rc > 0)
  {
    cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
    {
      json_Objeto_Salida = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Objeto_Salida");
      json_Tipo_Salida = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Tipo_Salida");
      if(atoi(json_Tipo_Salida->valuestring) == 0 || atoi(json_Tipo_Salida->valuestring) == 2)
      {
        /* Enciendo la alarma */
        m_pEV->ChangeAssignById(atoi(json_Objeto_Salida->valuestring), 2, 0);
      }
    } /* cJSON_ArrayForEach */
  }
  cJSON_Delete(json_Query_Result);

  /* Seteo el estado de alarma en la particion */
  sprintf(query, "UPDATE TB_DOM_ALARM_PARTICION "
                  "SET Estado_Activacion = 0, Estado_Memoria = 0 "
                  "WHERE Id = %i;",
                  particion);
  m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
  rc = m_pDB->Query(nullptr, query);
  m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
  if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);

  return 0;
}

int CAlarma::Activar(const char* particion)
{
  int rc;
  char query[4096];

  cJSON *json_Query_Result;
  cJSON *json_Query_Row;
  cJSON *json_Id;

  int zonas_abiertas = 0;

  /* Busco zonas abiertas */
  sprintf(query, "SELECT Id "
                    "FROM TB_DOM_ALARM_PARTICION "
                    "WHERE Nombre = %s ;", particion);
  m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
  json_Query_Result = cJSON_CreateArray();
  rc = m_pDB->Query(json_Query_Result, query);
  m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
  if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
  if(rc > 0)
  {
    cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
    {
      json_Id = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Id");
      zonas_abiertas += Activar(atoi(json_Id->valuestring));
    } /* cJSON_ArrayForEach */
  }
  cJSON_Delete(json_Query_Result);

  return zonas_abiertas;
}

int CAlarma::Desactivar(const char* particion)
{
  int rc;
  char query[4096];

  cJSON *json_Query_Result;
  cJSON *json_Query_Row;
  cJSON *json_Id;

  /* Busco zonas abiertas */
  sprintf(query, "SELECT Id "
                    "FROM TB_DOM_ALARM_PARTICION "
                    "WHERE Nombre = %s ;", particion);
  m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
  json_Query_Result = cJSON_CreateArray();
  rc = m_pDB->Query(json_Query_Result, query);
  m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
  if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
  if(rc > 0)
  {
    cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
    {
      json_Id = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Id");
      Desactivar(atoi(json_Id->valuestring));
    } /* cJSON_ArrayForEach */
  }
  cJSON_Delete(json_Query_Result);

  return 0;
}

int CAlarma::ExtIOEvent(const char* json_evt)
{
  int ival;
  int rc;
  char query[4096];

  cJSON *json_obj;
  cJSON *json_hw_mac;
  cJSON *json_un_obj;
  cJSON *json_Query_Result;
  cJSON *json_Query_Row;

  cJSON *json_Zona;
  cJSON *json_Estado_Zona;
  cJSON *json_Tipo_Zona;
  cJSON *json_Grupo_Zona;
  cJSON *json_Zona_Habilitada;
  cJSON *json_Part_Id;
  cJSON *json_Particion;
  cJSON *json_Particion_Activada;
  cJSON *json_Estado_Alarma;
  cJSON *json_Tiempo_De_Salida;
  cJSON *json_Tiempo_De_Entrada;
  cJSON *json_Tiempo_De_Alerta;
  cJSON *json_Objeto_Salida;
  cJSON *json_Tipo_Salida;
  cJSON *json_Notificar_SMS_Alerta;


  m_pServer->m_pLog->Add(100, "[ALARMA] Evento: %s", json_evt);

  json_obj = cJSON_Parse(json_evt);
  if(json_obj)
  {
    json_hw_mac = cJSON_GetObjectItemCaseSensitive(json_obj, "ID");
    if(json_hw_mac)
    {
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
                if( !memcmp(json_un_obj->string, "IO", 2) )
                {
                  ival = atoi(json_un_obj->valuestring);
                  /* Busco si el IO corresponde a una entrada de una particion de alarma activada */
                  sprintf(query, "SELECT P.Id AS Part_Id, A.Objeto AS Zona, A.Estado AS Estado_Zona, P.Nombre AS Particion, P.Estado_Activacion AS Particion_Activada, Z.Tipo_Zona AS Tipo_Zona, Z.Grupo, Z.Activa AS Zona_Habilitada, P.Estado_Alarma, P.Tiempo_De_Salida, P.Tiempo_De_Entrada, P.Tiempo_De_Alerta "
                                  "FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS A, TB_DOM_ALARM_PARTICION AS P, TB_DOM_ALARM_ZONA AS Z "
                                  "WHERE ( HW.Id = A.Dispositivo AND A.Id = Z.Objeto_Zona AND P.Id = Z.Particion ) "
                                    "AND UPPER(HW.MAC) = UPPER(\'%s\') AND A.Port = \'%s\';", json_hw_mac->valuestring, json_un_obj->string);
                  m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                  json_Query_Result = cJSON_CreateArray();
                  rc = m_pDB->Query(json_Query_Result, query);
                  m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                  if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                  if(rc > 0)
                  {
                    /* trae un solo resultado, no necesito recorrer el array */
                    cJSON_ArrayForEach(json_Query_Row, json_Query_Result) { break; }

                    json_Part_Id = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Part_Id");
                    json_Zona = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Zona");
                    json_Estado_Zona = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Estado_Zona");
                    json_Tipo_Zona = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Tipo_Zona");
                    json_Grupo_Zona = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Grupo");
                    json_Zona_Habilitada = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Zona_Habilitada");
                    json_Particion = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Particion");
                    json_Particion_Activada = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Particion_Activada");
                    json_Estado_Alarma = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Estado_Alarma");
                    json_Tiempo_De_Salida = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Tiempo_De_Salida");
                    json_Tiempo_De_Entrada = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Tiempo_De_Entrada");
                    json_Tiempo_De_Alerta = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Tiempo_De_Alerta");
                    json_Notificar_SMS_Alerta = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Notificar_SMS_Alerta");

                    m_pServer->m_pLog->Add(100, "[ALARMA] Evento: Zona (%s, %s) de Particion (%s, %s) Estado (%s) -> (%i)",
                                          json_Zona->valuestring, (atoi(json_Zona_Habilitada->valuestring))?"Habilitada":"No Habilitada", json_Particion->valuestring, (atoi(json_Particion_Activada->valuestring))?"Activada":"Desactivada", json_Estado_Zona->valuestring, ival);

                    if(atoi(json_Particion_Activada->valuestring))
                    {
                      /* Alarma activada */
                      if(atoi(json_Estado_Zona->valuestring) && atoi(json_Zona_Habilitada->valuestring))
                      {
                        /* Zona abierta y enable */
                        if(atoi(json_Particion_Activada->valuestring) == 2 ||  atoi(json_Grupo_Zona->valuestring) == 1)
                        {
                          /* Particion en estado de alarma */
                          if(atoi(json_Estado_Alarma->valuestring) == 0)
                          {
                            m_pServer->m_pLog->Add(10, "[ALARMA] Particion %s en estado de alarma por zona %s", 
                                                  json_Particion->valuestring, json_Zona->valuestring);
                            /* Seteo el estado de alarma en la particion */
                            sprintf(query, "UPDATE TB_DOM_ALARM_PARTICION "
                                            "SET Estado_Alarma = %s, Estado_Memoria = (Estado_Memoria+1) "
                                            "WHERE Id = %s;",
                                            json_Tiempo_De_Alerta->valuestring, json_Part_Id->valuestring);
                            m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                            rc = m_pDB->Query(nullptr, query);
                            m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                            if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                            /* Enciendo la salida de alarma de la particion */
                            sprintf(query, "SELECT * FROM TB_DOM_ALARM_SALIDA "
                                              "WHERE Particion = %s;", json_Part_Id->valuestring);
                            m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                            cJSON_Delete(json_Query_Result);
                            json_Query_Result = cJSON_CreateArray();
                            rc = m_pDB->Query(json_Query_Result, query);
                            m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                            if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                            if(rc > 0)
                            {
                              cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
                              {
                                json_Objeto_Salida = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Objeto_Salida");
                                json_Tipo_Salida = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Tipo_Salida");
                                if(atoi(json_Tipo_Salida->valuestring) == 0 || atoi(json_Tipo_Salida->valuestring) == 2)
                                {
                                  /* Enciendo la alarma */
                                  m_pEV->ChangeAssignById(atoi(json_Objeto_Salida->valuestring), 1, 0);
                                }
                              } /* cJSON_ArrayForEach */
                            }
                            /* Notifico */
                            if(atoi(json_Notificar_SMS_Alerta->valuestring))
                            {
                              /* TODO: Notificación por SMS */


                            }
                          }
                        }
                      }
                    }
                  }
                  cJSON_Delete(json_Query_Result);
                }
              }
            }
          }
        }
        json_un_obj = json_un_obj->next;
      } /* while(json_un_obj) */
    }
  }

  return 0;
}

void CAlarma::Task( void )
{
  int rc;
  char query[4096];

  cJSON *json_Query_Result;
  cJSON *json_Query_Row;

  cJSON *json_Part_Id;
  cJSON *json_Particion;
  cJSON *json_Particion_Activada;
  cJSON *json_Estado_Alarma;
  cJSON *json_Tiempo_De_Salida;
  cJSON *json_Tiempo_De_Entrada;
  cJSON *json_Tiempo_De_Alerta;
  cJSON *json_Objeto_Salida;
  cJSON *json_Tipo_Salida;

  int iEstado_Alarma;

  sprintf(query, "SELECT * "
                  "FROM TB_DOM_ALARM_PARTICION;");
  m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
  json_Query_Result = cJSON_CreateArray();
  rc = m_pDB->Query(json_Query_Result, query);
  m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
  if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
  if(rc > 0)
  {
    cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
    {
      json_Part_Id = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Id");
      json_Particion = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Nombre");
      json_Particion_Activada = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Estado_Activacion");
      json_Estado_Alarma = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Estado_Alarma");
      json_Tiempo_De_Salida = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Tiempo_De_Salida");
      json_Tiempo_De_Entrada = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Tiempo_De_Entrada");
      json_Tiempo_De_Alerta = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Tiempo_De_Alerta");

      /* Tiempo de alarma sonando */
      iEstado_Alarma = atoi(json_Estado_Alarma->valuestring);
      if(iEstado_Alarma > 0)
      {
        iEstado_Alarma--;
        if(iEstado_Alarma == 0)
        {
          /* Fin tiempo de sirena */
          /* Apago la salida de alarma de la particion */
          sprintf(query, "SELECT * FROM TB_DOM_ALARM_SALIDA "
                            "WHERE Particion = %s;", json_Part_Id->valuestring);
          m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
          cJSON_Delete(json_Query_Result);
          json_Query_Result = cJSON_CreateArray();
          rc = m_pDB->Query(json_Query_Result, query);
          m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
          if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
          if(rc > 0)
          {
            cJSON_ArrayForEach(json_Query_Row, json_Query_Result)
            {
              json_Objeto_Salida = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Objeto_Salida");
              json_Tipo_Salida = cJSON_GetObjectItemCaseSensitive(json_Query_Row->child, "Tipo_Salida");
              if(atoi(json_Tipo_Salida->valuestring) == 0 || atoi(json_Tipo_Salida->valuestring) == 2)
              {
                /* Enciendo la alarma */
                m_pEV->ChangeAssignById(atoi(json_Objeto_Salida->valuestring), 2, 0);
              }
            } /* cJSON_ArrayForEach */
          }
        }
        /* Seteo el estado de alarma en la particion */
        sprintf(query, "UPDATE TB_DOM_ALARM_PARTICION "
                        "SET Estado_Alarma = %i "
                        "WHERE Id = %s;",
                        iEstado_Alarma, json_Part_Id->valuestring);
        m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
        rc = m_pDB->Query(nullptr, query);
        m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
        if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
      }







    }

  }
  cJSON_Delete(json_Query_Result);
}
