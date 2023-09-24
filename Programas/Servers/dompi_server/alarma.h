#ifdef DALARMA_INTEGRADA

#ifndef _ALARMA_H_
#define _ALARMA_H_

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

#include "cdb.h"
#include "gevent.h"
#include <gmonitor/gmswaited.h>

class CAlarma
{
public:
    CAlarma(CDB *pDB, GEvent *pEV, CGMServerWait *pServer);
    virtual ~CAlarma();

    int Habilitar(const char* zona, const char* particion);
    int Deshabilitar(const char* zona, const char* particion);
    int Activar(const char* particion);
    int Desactivar(const char* particion);
    int Estado(const char* particion, char* json_estado, int max);

    int ExtIOEvent(const char* json_evt);


    void Task( void );

private:
    CDB *m_pDB;
    GEvent *m_pEV;
    CGMServerWait *m_pServer;

};
#endif /* _ALARMA_H_ */

#endif /* DALARMA_INTEGRADA */
