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

/*
Reset a configuración en firmware
    ATZ
    OK
Echo off
    ATE0
    OK
No atender telefono
    ATS0
    OK
PII - Identificación del producto BGxx, UGxx, etc. + Revision
    ATI
    Manufacturer: huawei
    Model: E303C
    Revision: 21.157.01.01.18
    IMEI: 861146011227773
    +GCAP: +CGSM,+DS,+ES

    OK
Request International Mobile Equipment Identity (IMEI)
    AT+GSN
    861146011227773

    OK

Request TA Model Identification
    AT+GMM
    E303C

    OK

Configuración de mensajes
    AT+CNMI=2,1,0,0,0
    AT+CMGF=1
    AT+CREG=0

Send SMS
    AT+CMGS="number"
    >
    Mensaje
    ^Z (0x1A)

Read SMS
    AT+CMGR=

AT+CMGDA="DEL READ"
AT+CMGDA="DEL SENT"
AT+CMGDA="DEL ALL"

*/

#include "modulo_gsm.h"

#include <string>
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
using namespace std;

#include <unistd.h>
#include <sys/msg.h>
#include <time.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <math.h>

ModGSM::ModGSM(const char* tempdir)
{
    strcpy(m_port_path, "/dev/ttyUSB");
    m_last_port = 0;
    strncpy(m_temp_dir, tempdir, FILENAME_MAX);
    m_pServer = nullptr;
    m_modem_status = MODEM_STATUS_NOT_INIT;
    m_pSerial = nullptr;
    m_next_task_time = 0;
    m_get_status_time = 0;
}

ModGSM::ModGSM(const char* tempdir, CGMServerWait *pServer)
{
    strcpy(m_port_path, "/dev/ttyUSB");
    m_last_port = 0;
    strncpy(m_temp_dir, tempdir, FILENAME_MAX);
    m_pServer = pServer;
    m_modem_status = MODEM_STATUS_NOT_INIT;
    m_pSerial = nullptr;
    m_next_task_time = 0;
    m_get_status_time = 0;
}

ModGSM::ModGSM(const char* tempdir, CGMServerWait *pServer, const char* port_path)
{
    if(port_path) strcpy(m_port_path, port_path);
    m_last_port = 0;
    strncpy(m_temp_dir, tempdir, FILENAME_MAX);
    m_pServer = pServer;
    m_modem_status = MODEM_STATUS_NOT_INIT;
    m_pSerial = nullptr;
    m_next_task_time = 0;
    m_get_status_time = 0;
}

ModGSM::~ModGSM()
{
    Close();
}

int ModGSM::Open()
{
    char port_name[256];
    int retry = MAX_PORT_NUMBER+1;

    if(m_pServer) m_pServer->m_pLog->Add(100, "[ModGSM] Open");


    while(retry)
    {
        Close();
        m_pSerial = new CSerial();

        sprintf(port_name, "%s%i", m_port_path, m_last_port);
        if(m_pServer) m_pServer->m_pLog->Add(100, "[ModGSM] Intentando abrir [%s]", port_name);
        if(m_pSerial->Open(port_name) != 0)
        {
            delete m_pSerial;
            m_pSerial = nullptr;

            retry--;
            m_last_port++;
            if(m_last_port > MAX_PORT_NUMBER) m_last_port = 0;

            continue;
        }

        m_modem_status = MODEM_STATUS_OPEN;
        while(1)
        {
            if(QueryModem("OK", "ATZ") < 0) break;
            if(QueryModem("OK", "ATE0") < 0) break;
            if(QueryModem("OK", "ATS0=0") < 0) break;
            if(QueryModem("OK", m_imei, 32, "AT+GSN") < 0) break;
            if(QueryModem("OK", "AT+CNMI=2,1,0,0,0") < 0) break;
            if(QueryModem("OK", "AT+CMGF=1") < 0) break;
            if(QueryModem("OK", "AT+CREG=0") < 0) break;

            m_modem_status = MODEM_STATUS_SMS_READY;
            if(m_pServer) m_pServer->m_pLog->Add(20, "[ModGSM] Open [%s] OK", port_name);
            return 0;
        }
        delete m_pSerial;
        m_pSerial = nullptr;

        retry--;
        m_last_port++;
        if(m_last_port > MAX_PORT_NUMBER) m_last_port = 0;
    }
    if(m_pServer) m_pServer->m_pLog->Add(1, "[ModGSM] Error inicialiando modem en port [%s]", port_name);
    ModemError();
    return (-1);
}

int ModGSM::Open(const char* port_path)
{
    if(port_path) strcpy(m_port_path, port_path);
    return Open();
}

void ModGSM::Close()
{
    m_modem_status = MODEM_STATUS_CLOSE;
    if(!m_pSerial) return; 
    delete m_pSerial;
    m_pSerial = nullptr;
}

int ModGSM::QueryModem(const char* wait_for, const char* fmt, ...)
{
    va_list arg;
    char buffer[4096];

    va_start(arg, fmt);
    vsnprintf(buffer, 4095, fmt, arg);
    va_end(arg);

    return QueryModem(wait_for, nullptr, 0, buffer);
}

int ModGSM::QueryModem(const char* wait_for, char* recv, int recv_max, const char* fmt, ...)
{
    va_list arg;
    char buffer[4096];
    time_t t, t_end;
    int len;

    va_start(arg, fmt);
    vsnprintf(buffer, 4095, fmt, arg);
    va_end(arg);

    if(m_modem_status < MODEM_STATUS_OPEN || !m_pSerial)
    {
        if(m_pServer) m_pServer->m_pLog->Add(1, "[ModGSM] QueryModem: Modem en estado invalido");
        return (-1);
    }

    if(m_pServer) m_pServer->m_pLog->Add(100, "[ModGSM] QueryModem: >> [%s]", buffer);

    t = time(&t);
    t_end = t + 5;
    if(recv) *recv = 0;
    strcat(buffer, "\r\n");
    if(m_pSerial->Send(buffer) == 0) return (-1);
    do
    {
        buffer[0] = 0;
        len = m_pSerial->Recv(buffer, 4096, 5);
        if(len)
        {
            if(m_pServer) m_pServer->m_pLog->Add(100, "[ModGSM] QueryModem: << [%s]", buffer);
            if(wait_for)
            {
                if( !memcmp(wait_for, buffer, strlen(wait_for)) ) break;
            }
            if(recv && recv_max && ((int)(strlen(recv)+len+2) < recv_max ) )
            {
                if(strlen(recv)) strcat(recv, ",");
                strcat(recv, buffer);
            }
        }
        else
        {
            usleep(100);
        }
        t = time(&t);
    } while ( t <= t_end );
    if(wait_for)
    {
        if( memcmp(wait_for, buffer, strlen(wait_for))) return (-1);
    }
    return (recv && recv_max)?strlen(recv):0;
}

void ModGSM::Task( void )
{
    long t;

    t = time(&t);
    if(t < m_next_task_time) return;

    switch(m_modem_status)
    {
        case MODEM_STATUS_ERROR:
        case MODEM_STATUS_NOT_INIT:
            Close();
            m_next_task_time = t + 45;
            break;
        case MODEM_STATUS_CLOSE:
            Open();
            m_next_task_time = t + 10;
            break;
        case MODEM_STATUS_OPEN:
            break;
        case MODEM_STATUS_SMS_READY:
        case MODEM_STATUS_GSM_READY:
            if(t > m_get_status_time)
            {
                m_get_status_time = t + 30;
                GetModemStatus();
                CheckSMS();
            }
            CheckUnsol();
            break;
    }

}

void ModGSM::CheckUnsol(void)
{
    char buffer[4096];
    int len;

    if(!m_pSerial) return;

    len = m_pSerial->Recv(buffer, 4096, 1);
    if(len)
    {
        if(m_pServer) m_pServer->m_pLog->Add(100, "[ModGSM] CheckUnsol: << [%s]", buffer);
        if( !memcmp("+CMTI:", buffer, 6))
        {
            /* Aviso de mensaje entrante - viene con numero de mensaje - +CMTI: "SM",1 */
            if(strchr(buffer, ','))
            {
                GetSMS( atoi(strchr(buffer, ',')+1) );
            }

        }



    }
}

void ModGSM::GetSMS(int id)
{
    char buffer[4096];
    int rc;
    if(!m_pSerial) return;

    if(m_pServer) m_pServer->m_pLog->Add(100, "[ModGSM] GetSMS Id:[%i]", id);
    /* AT+CMGR=n */
    rc = QueryModem("OK", buffer, 4096, "AT+CMGR=%i", id);
    if(rc < 0)
    {
        ModemError();
        return;
    }
    else if(rc > 0)
    {
        if(m_pServer) m_pServer->m_pLog->Add(50, "[ModGSM] GetSMS: Mesaje recibido [%s]", buffer);





    }

}

/*
AT+CMG

+CMGL: 4,"REC UNREAD","01133926320",,"22/09/22,12:47:58-12",Prueba input 1

OK
*/
void ModGSM::CheckSMS( void )
{
    char buffer[4096];
    char *p;
    char from[32];
    char msg[4096];
    int i;
    int borrar_leidos = 0;
    int rc;

    if(!m_pSerial) return;

    if(m_pServer) m_pServer->m_pLog->Add(100, "[ModGSM] CheckSMS");
    /* AT+CMGR=n */
    rc = QueryModem("OK", buffer, 4096, "AT+CMGL");
    if(rc < 0)
    {
        ModemError();
        return;
    }
    else if(rc > 0)
    {
        if(m_pServer) m_pServer->m_pLog->Add(100, "[ModGSM] CheckSMS: Mensaje recibido [%s]", buffer);
        p = strstr(buffer, "+CMGL:");
        while(p)
        {
            borrar_leidos = 1;
            /* Salto al número de origen, salto 2 ',' y la " */
            i = 2;
            while(i && *p)
            {
                if(*p == ',') i--;
                p++;
            }
            if(!(*p)) break;
            p++;
            /* Copio la parte numerica */
            i = 0;
            while(*p && ( (*p >= '0' && *p <= '9') || *p == '+' ) && i < (int)(sizeof(from)-1))
            {
                from[i] = *p;
                i++;
                p++;
            }
            from[i] = 0;
            /* Busco el inicio del mensaje, salto 4 comas */
            i = 4;
            while(i && *p)
            {
                if(*p == ',') i--;
                p++;
            }
            if(!(*p)) break;
            /* Copio el texto hasta el final */
            i = 0;
            while(*p && *p != 0x0A && *p !=0x0D && i < (int)(sizeof(msg)-1))
            {
                if(*p == ',')
                {
                    if( !memcmp(p, ",+CMGL:", 7)) break;
                }
                msg[i] = *p;
                i++;
                p++;
            }
            msg[i] = 0;

            if(m_pServer) m_pServer->m_pLog->Add(50, "[ModGSM] GetSMS: Mesaje recibido De: [%s] Mensaje: [%s]", from, msg);
            SaveRecvSMS(from, msg);



            p = strstr(p, "+CMGL:");
        }
    }

    if(borrar_leidos)
    {
        SMSDelRead();
    }
}

void ModGSM::GetModemStatus( void )
{
    char buffer[256];
    int rc;

    if(m_pServer) m_pServer->m_pLog->Add(100, "[ModGSM] GetModemStatus");

    if(!m_pSerial) return;

    rc = QueryModem("OK", "AT+CREG?");
    if(rc < 0)
    {
        ModemError();
        return;
    }
    rc = QueryModem("OK", buffer, 256, "AT+CSQ");
    if(rc < 0)
    {
        ModemError();
        return;
    }
    else if(rc > 0)
    {
        /* Respuesta de AT+CSQ - viene con info de nivel de se?al - +CSQ: 12,0 */
        if(m_pServer) m_pServer->m_pLog->Add(20, "[ModGSM] GetModemStatus: [%s]", buffer);
        /*
            Wording 	Blocks 	Percent/RSSI/DB
            Excellent 	[][][][][] 	100 31 	>-51
                                            97 	30 	-53
                                            94 	29 	-55
                                            90 	28 	-57
                                            87 	27 	-59
                                            84 	26 	-61
            Good            [][][][]	    81 	25 	-63
                                            77 	24 	-65
                                            74 	23 	-67
                                            71 	22 	-69
                                            68 	21 	-71
                                            65 	20 	-73
            Fair            [][][]		    61 	19 	-75
                                            58 	18 	-77
                                            55 	17 	-79
                                            52 	16 	-81
                                            48 	15 	-83
                                            45 	14 	-85
            Poor            [][]		    42 	13 	-87
                                            39 	12 	-89
                                            35 	11 	-91
                                            32 	10 	-93
                                            29 	9 	-95
                                            26 	8 	-97
            Very Poor 	[] 		            23 	7 	-99
                                            19 	6 	-101
                                            16 	5 	-103
                                            13 	4 	-105
                                            10 	3 	-107
                                            6 	2 	-109
            No Signal 			            3 	1 	-111
                                            0 	0 	<-113
            */




    }

}

int ModGSM::SendSMS(const char* dest, const char* msg)
{
    if(m_pServer) m_pServer->m_pLog->Add(50, "[ModGSM] SendSMS dest:[%s] msg:[%s]", dest, msg);
    SMSDelSent();
    do
    {
        if(QueryModem(">", "AT+CMGS=\"%s\"", dest) < 0) break;
        /* Mensaje + CTRL+Z              (^Z) */
        if(QueryModem("OK", "%s%c", msg, 0x1A) < 0) break;
        return 0;
    } while (0);
    if(m_pServer) m_pServer->m_pLog->Add(1, "[ModGSM] Error en SendSMS dest:[%s] msg:[%s]", dest, msg);
    ModemError();
    return (-1);
}

int ModGSM::SendTCP(const char* /*host*/, unsigned /*port*/, const char* /*msg*/)
{

    return (-1);
}

int ModGSM::SendUDP(const char* /*host*/, unsigned /*port*/, const char* /*msg*/)
{

    return (-1);
}

int ModGSM::ReadySMS( void )
{
    if(m_modem_status >= MODEM_STATUS_SMS_READY) return 1;
    return 0;
}

int ModGSM::ReadyTCP( void )
{
    if(m_modem_status >= MODEM_STATUS_GSM_READY) return 1;
    return 0;
}

int ModGSM::ReadyUDP( void )
{
    if(m_modem_status >= MODEM_STATUS_GSM_READY) return 1;
    return 0;
}

void ModGSM::SMSDelRead(void)
{
    if(m_pServer) m_pServer->m_pLog->Add(100, "[ModGSM] SMSDelRead");
    if(QueryModem("OK", "AT+CMGD=1,1") < 0) ModemError();
}

void ModGSM::SMSDelSent(void)
{
    if(m_pServer) m_pServer->m_pLog->Add(100, "[ModGSM] SMSDelSent");
    if(QueryModem("OK", "AT+CMGD=1,2") < 0) ModemError();
}

void ModGSM::ModemError(void)
{
    if(m_pServer) m_pServer->m_pLog->Add(100, "[ModGSM] ModemError");
    m_modem_status = MODEM_STATUS_ERROR;
    delete m_pSerial;
    m_pSerial = nullptr;
}

int ModGSM::SaveRecvSMS(const char* from, const char* msg)
{
    FILE *f;
    char filename[FILENAME_MAX+1];
    char filename_tmp[FILENAME_MAX+1];
    time_t t;
    char sms_buffer[1024];

    t = time(&t);

    if(m_pServer) m_pServer->m_pLog->Add(100, "[ModGSM] SaveRecvSMS");

    if( m_temp_dir[strlen(m_temp_dir) - 1] != '/' )
    {
        snprintf(filename, FILENAME_MAX, "%s/recv-sms-%10lu", m_temp_dir, t);
        snprintf(filename_tmp, FILENAME_MAX, "%s/recv-tmp-%10lu", m_temp_dir, t);
    }
    else
    {
        snprintf(filename, FILENAME_MAX, "%srecv-sms-%10lu", m_temp_dir, t);
        snprintf(filename_tmp, FILENAME_MAX, "%srecv-tmp-%10lu", m_temp_dir, t);
    }
    f = fopen(filename_tmp, "w");
    if(f)
    {
        snprintf(sms_buffer, 1023, "SMS:%s:%s\n", from, msg);
        if(fwrite(sms_buffer, sizeof(char), strlen(sms_buffer), f) != strlen(sms_buffer))
        {
            fclose(f);
            /* Error */
            if(m_pServer) m_pServer->m_pLog->Add(1, "[ModGSM] ERROR Al escribir archivo de mensaje entrante [%s]", filename_tmp);
            return (-1);
        }
        /* OK */
        fclose(f);
        rename(filename_tmp, filename);
        return 0;
    }
    else
    {
        /* Error */
        if(m_pServer) m_pServer->m_pLog->Add(1, "[ModGSM] ERROR Al crear archivo de mensaje entrante [%s]", filename_tmp);
        return (-1);
    }
}