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
#include <gmonitor/gmswaited.h>

class CAlarma
{
public:
    CAlarma(CDB *pDB, CGMServerWait *pServer);
    virtual ~CAlarma();

    int Activar(int particion);
    int Desactivar(int particion);
    int Activar(const char* particion);
    int Desactivar(const char* particion);

    int ExtIOEvent(const char* json_evt);


    void Task( void );

private:
    CDB *m_pDB;
    CGMServerWait *m_pServer;

};
#endif /* _ALARMA_H_ */