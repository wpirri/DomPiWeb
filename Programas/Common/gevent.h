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
#ifndef _GEVENT_H_
#define _GEVENT_H_

#include "cdb.h"
#include <gmonitor/gmswaited.h>

class GEvent
{
public:
    GEvent(CDB *pDB, CGMServerWait *pServer);
    virtual ~GEvent();

    int ExtIOEvent(const char* json_evt);
    int SyncIO(const char* json_evt);
    int ChangeIO(const char* json_evt);

    void CheckEvent(int hw_id, const char* port, int estado);
    void CheckAuto(int hw_id, const char* port, int estado);

/*
    accion  1 = Encender
            2 = Apagar
            3 = Switch
            4 = Pulso   -> param = largo del pulso en seg
*/

    int ChangeAssignByName(const char* name, int accion, int param);
    int ChangeAssignById(int id, int accion, int param);
    int ChangeGroupByName(const char* name, int accion, int param);
    int ChangeGroupById(int id, int accion, int param);
    int ChangeFcnByName(const char* name, int accion, int param);
    int ChangeFcnById(int id, int accion, int param);
    int ChangeVarByName(const char* name, int accion, int param);
    int ChangeVarById(int id, int accion, int param);
    int ChangeAutoByName(const char* name, int accion, int param);
    int ChangeAutoById(int id, int accion, int param);


    int Habilitar_Alarma(const char* zona, const char* particion);
    int Deshabilitar_Alarma(const char* zona, const char* particion);
    int Activar_Alarma(const char* particion, int total);
    int Desactivar_Alarma(const char* particion);
    int Estado_Alarma(const char* particion, char* json_estado, int max);

    int Switch_Alarma(const char* particion);
    int Switch_Zona_Alarma(const char* particion, const char* zona);
    int Pulse_Salida_Alarma(const char* particion, const char* salida);

    int ExtIOEvent_Alarma(int assign, int status);

    void Task_Alarma( void );

private:
    CDB *m_pDB;
    CGMServerWait *m_pServer;
    unsigned long m_last_time_task;

};
#endif /* _GEVENT_H_ */
