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
#ifndef _MODULO_GSM_H_
#define _MODULO_GSM_H_

#include <gmonitor/gmswaited.h>
#include "serial.h"

class ModGSM
{
public:

    typedef enum
    {
        MODEM_STATUS_ERROR = (-1),
        MODEM_STATUS_NOT_INIT = 0,
        MODEM_STATUS_CLOSE,
        MODEM_STATUS_OPEN,
        MODEM_STATUS_SMS_READY,
        MODEM_STATUS_GSM_READY
    } MODEM_STATUS;

    unsigned int m_time_out;

    ModGSM(const char* tempdir);
    ModGSM(const char* tempdir, CGMServerWait *pServer);
    ModGSM(const char* tempdir, CGMServerWait *pServer, const char* port);
    virtual ~ModGSM();

    int Open();
    int Open(const char* port);
    void Close();

    int ReadySMS( void );
    int ReadyTCP( void );
    int ReadyUDP( void );

    int SendSMS(const char* dest, const char* msg);
    int SendTCP(const char* host, unsigned port, const char* msg);
    int SendUDP(const char* host, unsigned port, const char* msg);

    void Task( void );
    
private:
    CGMServerWait* m_pServer;
    CSerial* m_pSerial;
    MODEM_STATUS m_modem_status;
    char m_temp_dir[FILENAME_MAX+1];
    char m_port[256];
    char m_imei[32];
    long m_next_task_time;
    long m_get_status_time;

    int QueryModem(const char* wait_for, const char* fmt, ...);
    int QueryModem(const char* wait_for, char* recv, int recv_max, const char* fmt, ...);
    void CheckUnsol(void);
    void CheckSMS( void );
    void GetSMS(int id);
    void GetModemStatus(void);
    void SMSDelRead(void);
    void SMSDelSent(void);

    void ModemError(void);
    int SaveRecvSMS(const char* from, const char* msg);

};

#endif /* _MODULO_GSM_H_ */