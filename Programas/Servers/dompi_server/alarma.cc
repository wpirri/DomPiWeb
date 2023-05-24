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

CAlarma::CAlarma(CDB *pDB, CGMServerWait *pServer)
{
    m_pDB = pDB;
    m_pServer = pServer;
}

CAlarma::~CAlarma()
{

}

int CAlarma::Activar(int particion)
{

  return 0;
}

int CAlarma::Desactivar(int particion)
{

  return 0;
}

int CAlarma::Activar(const char* particion)
{

  return 0;
}

int CAlarma::Desactivar(const char* particion)
{

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
  cJSON *json_QueryArray;

  cJSON *json_Zona;
  cJSON *json_Estado_Zona;
  cJSON *json_Tipo_Zona;
  cJSON *json_Zona_Habilitada;
  cJSON *json_Particion;
  cJSON *json_Particion_Activada;

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
                  sprintf(query, "SELECT A.Objeto AS Zona, A.Estado AS Estado_Zona, P.Nombre AS Particion, P.Estado_Activacion AS Particion_Activada, Z.Tipo_Zona AS Tipo_Zona, Z.Grupo, Z.Activa AS Zona_Habilitada "
                                  "FROM TB_DOM_PERIF AS HW, TB_DOM_ASSIGN AS A, TB_DOM_ALARM_PARTICION AS P, TB_DOM_ALARM_ZONA AS Z "
                                  "WHERE ( HW.Id = A.Dispositivo AND A.Id = Z.Objeto_Zona AND P.Id = Z.Particion ) "
                                    "AND UPPER(HW.MAC) = UPPER(\'%s\') AND A.Port = \'%s\';", json_hw_mac->valuestring, json_un_obj->string);
                  m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
                  json_QueryArray = cJSON_CreateArray();
                  rc = m_pDB->Query(json_QueryArray, query);
                  m_pServer->m_pLog->Add((m_pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, m_pDB->LastQueryTime(), query);
                  if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", m_pDB->m_last_error_text, query);
                  if(rc > 0)
                  {
                    /* trae un solo resultado, no necesito recorrer el array */
                    json_Zona = cJSON_GetObjectItemCaseSensitive(json_QueryArray->child, "Zona");
                    json_Estado_Zona = cJSON_GetObjectItemCaseSensitive(json_QueryArray->child, "Estado_Zona");
                    json_Tipo_Zona = cJSON_GetObjectItemCaseSensitive(json_QueryArray->child, "Tipo_Zona");
                    json_Zona_Habilitada = cJSON_GetObjectItemCaseSensitive(json_QueryArray->child, "Zona_Habilitada");
                    json_Particion = cJSON_GetObjectItemCaseSensitive(json_QueryArray->child, "Particion");
                    json_Particion_Activada = cJSON_GetObjectItemCaseSensitive(json_QueryArray->child, "Particion_Activada");

                    m_pServer->m_pLog->Add(100, "[ALARMA] Evento: Zona (%s, %s) de Particion (%s, %s) Estado (%s) -> (%i)",
                                          json_Zona->valuestring, (atoi(json_Zona_Habilitada->valuestring))?"Habilitada":"No Habilitada", json_Particion->valuestring, (atoi(json_Particion_Activada->valuestring))?"Activada":"Desactivada", json_Estado_Zona->valuestring, ival);




                  }

                }
              }
            }
          }
        }
        json_un_obj = json_un_obj->next;
      }
    }
  }

  return 0;
}
