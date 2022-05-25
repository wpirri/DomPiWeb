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
    XML: http://xmlparselib.sourceforge.net/
         http://xmlparselib.sourceforge.net/examp_xml_token_traverser.html
*/
#include "dom32iowifi.h"

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

#include "ctcp.h"
#include "strfunc.h"
#include "queue.h"

#define BUFFER_LEN 32767

Dom32IoWifi::Dom32IoWifi(CGLog *pLog)
{
    m_pLog = pLog;
    /* POST
    * 1.- %s: URI
    * 2.- %s: Host
    * 3.- %d: Content-Length
    * 4.- %s: datos
    */
    http_post =     "POST %s HTTP/1.1\r\n"
                    "Host: %s\r\n"
                    "Connection: keep-alive\r\n"
                    "Content-Length: %d\r\n"
                    "User-Agent: DomPiSrv/1.00 (RaspBerryPi;Dom32)\r\n"
                    "Accept: text/html,text/xml\r\n"
                    "Content-Type: application/x-www-form-urlencoded\r\n\r\n%s";

    url_set_iostatus = "/iostatus.cgi";
    url_set_ostatus = "/iostatus.cgi";
    url_set_exstatus = "/exstatus.cgi";
    url_switch_iostatus = "/ioswitch.cgi";
    url_switch_ostatus = "/ioswitch.cgi";
    url_switch_exstatus = "/exswitch.cgi";
    url_pulse_iostatus = "/iopulse.cgi";
    url_pulse_ostatus = "/iopulse.cgi";
    url_pulse_exstatus = "/expulse.cgi";

    url_set_ioconfig = "/ioconfig.cgi";
    url_set_exconfig = "/exconfig.cgi";

    /*
    * GET
    * 1.- %s: URI
    * 2.- %s: Host
    */
    http_get =     "GET %s HTTP/1.1\r\n"
                        "Host: %s\r\n"
                        "Connection: close\r\n"
                        "User-Agent: DomPiSrv/1.00 (RaspBerryPi;Dom32)\r\n"
                        "Accept: text/html,text/xml\r\n\r\n";

    url_get_iostatus = "/iostatus.htm";
    url_get_ostatus = "/iostatus.htm";
    url_get_exstatus = "/exstatus.htm";

    url_get_config = "/config.htm";
    url_get_ioconfig = "/ioconfig.htm";
    url_get_exconfig = "/exconfig.htm";

    url_get_wifi = "/wifi.cgi";
    url_set_wifi = "/wifi.cgi";

    m_timeout = 1500;
    m_port = 80;

    memset(m_queue_list, 0, sizeof(queue_list));
    QueueInit();

}

Dom32IoWifi::~Dom32IoWifi()
{

}

int Dom32IoWifi::GetIOStatus(const char *raddr, int *iostatus)
{
    char buffer[BUFFER_LEN+1];
    char *p;
    CTcp q;
    int rc;

    sprintf(buffer, http_get, url_get_iostatus, raddr);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Send [%s]", buffer);
    *iostatus = 0;
    if(q.Query(raddr, 80, buffer, buffer, BUFFER_LEN, m_timeout) > 0)
    {
        if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Receive [%s]", buffer);
        rc = HttpRespCode(buffer);
        if(rc != 0) return rc;
        /* Me posiciono al final de la cabecera HTTP, al principio de los datos */
        p = strstr(buffer, "\r\n\r\n");
        if(p)
        {
            /* Salteo CR/LF CR/LF */
            p += 4;
            /* Interpreto los datos */
            if(iostatus)
            {
                *iostatus = IO2Int(p);
            }
        }
        return 0;
    }
    return (-1);
}

int Dom32IoWifi::GetOutStatus(const char *raddr, int *ostatus)
{
    char buffer[BUFFER_LEN+1];
    char *p;
    CTcp q;
    int rc;

    sprintf(buffer, http_get, url_get_ostatus, raddr);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Send [%s]", buffer);
    *ostatus = 0;
    if(q.Query(raddr, 80, buffer, buffer, BUFFER_LEN, m_timeout) > 0)
    {
        if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Receive [%s]", buffer);
        rc = HttpRespCode(buffer);
        if(rc != 0) return rc;
        /* Me posiciono al final de la cabecera HTTP, al principio de los datos */
        p = strstr(buffer, "\r\n\r\n");
        if(p)
        {
            /* Salteo CR/LF CR/LF */
            p += 4;
            /* Interpreto los datos */
            if(ostatus)
            {
                *ostatus = Out2Int(p);
            }
        }
        return 0;
    }
    return (-1);
}

int Dom32IoWifi::GetEXStatus(const char *raddr, int *exstatus)
{
    char buffer[4096];
    char *p;
    CTcp q;
    int rc;

    sprintf(buffer, http_get, url_get_exstatus, raddr);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Send [%s]", buffer);
    *exstatus = 0;
    if(q.Query(raddr, 80, buffer, buffer, BUFFER_LEN, m_timeout) > 0)
    {
        if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Receive [%s]", buffer);
        rc = HttpRespCode(buffer);
        if(rc != 0) return rc;
        /* Me posiciono al final de la cabecera HTTP, al principio de los datos */
        p = strstr(buffer, "\r\n\r\n");
        if(p)
        {
            /* Salteo CR/LF CR/LF */
            p += 4;
            /* Interpreto los datos */
            if(exstatus)
            {
                *exstatus = EXP2Int(p);
            }
        }
        return 0;
    }
    return (-1);
}

int Dom32IoWifi::GetConfig(const char *raddr, int *ioconfig, int *exconfig)
{
    char buffer[BUFFER_LEN+1];
    char *p;
    CTcp q;
    int rc;

    sprintf(buffer, http_get, url_get_config, raddr);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Send [%s]", buffer);
    if(ioconfig) *ioconfig = 0;
    if(exconfig) *exconfig = 0;

    if(q.Query(raddr, 80, buffer, buffer, BUFFER_LEN, m_timeout) > 0)
    {
        if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Receive [%s]", buffer);
        rc = HttpRespCode(buffer);
        if(rc != 0) return rc;
        /* Me posiciono al final de la cabecera HTTP, al principio de los datos */
        p = strstr(buffer, "\r\n\r\n");
        if(p)
        {
            /* Salteo CR/LF CR/LF */
            p += 4;
            /* Interpreto los datos */
            if(ioconfig)
            {
                *ioconfig = IO2Int(p);
            }
            if(exconfig)
            {
                *exconfig = EXP2Int(p);
            }
        }
        return 0;
    }
    return (-1);
}

int Dom32IoWifi::ConfigIO(const char *raddr, int ioconfig)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    sprintf(data, "IO1=%s&IO2=%s&IO3=%s&IO4=%s&IO5=%s&IO6=%s&IO7=%s&IO8=%s",
            (ioconfig&0x01)?"in":"out",
            (ioconfig&0x02)?"in":"out",
            (ioconfig&0x04)?"in":"out",
            (ioconfig&0x08)?"in":"out",
            (ioconfig&0x10)?"in":"out",
            (ioconfig&0x20)?"in":"out",
            (ioconfig&0x40)?"in":"out",
            (ioconfig&0x80)?"in":"out");
    sprintf(buffer, http_post, url_set_ioconfig, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::ConfigEX(const char *raddr, int exconfig)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    sprintf(data, "EXP1=%s&EXP2=%s&EXP3=%s&EXP4=%s&EXP5=%s&EXP6=%s&EXP7=%s&EXP8=%s",
            (exconfig&0x01)?"in":"out",
            (exconfig&0x02)?"in":"out",
            (exconfig&0x04)?"in":"out",
            (exconfig&0x08)?"in":"out",
            (exconfig&0x10)?"in":"out",
            (exconfig&0x20)?"in":"out",
            (exconfig&0x40)?"in":"out",
            (exconfig&0x80)?"in":"out");
    sprintf(buffer, http_post, url_set_exconfig, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::ConfigFlags(const char *raddr, int flags)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    sprintf(data, "HTTPS=%s&WIEGAND=%s&DHT11=%s",
            (flags&FLAG_HTTPS_ENABLE)?"yes":"no",
            (flags&FLAG_WIEGAND_ENABLE)?"yes":"no",
            (flags&FLAG_DHT11_ENABLE)?"yes":"no");
    sprintf(buffer, http_post, url_set_ioconfig, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::SetIO(const char *raddr, int mask)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    data[0] = 0;

    if(mask & 0x01)
    {
        strcat(data, "IO1=on");
    }
    if(mask & 0x02)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO2=on");
    }
    if(mask & 0x04)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO3=on");
    }
    if(mask & 0x08)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO4=on");
    }
    if(mask & 0x10)
    {
        strcat(data, "IO5=on");
    }
    if(mask & 0x20)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO6=on");
    }
    if(mask & 0x40)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO7=on");
    }
    if(mask & 0x80)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO8=on");
    }
    sprintf(buffer, http_post, url_set_iostatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::SetOut(const char *raddr, int mask)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    data[0] = 0;

    if(mask & 0x01)
    {
        strcat(data, "OUT1=on");
    }
    if(mask & 0x02)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT2=on");
    }
    if(mask & 0x04)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT3=on");
    }
    if(mask & 0x08)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT4=on");
    }
    if(mask & 0x10)
    {
        strcat(data, "OUT5=on");
    }
    if(mask & 0x20)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT6=on");
    }
    if(mask & 0x40)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT7=on");
    }
    if(mask & 0x80)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT8=on");
    }
    sprintf(buffer, http_post, url_set_ostatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::SetEX(const char *raddr, int mask)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    data[0] = 0;

    if(mask & 0x01)
    {
        strcat(data, "EXP1=on");
    }
    if(mask & 0x02)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP3=on");
    }
    if(mask & 0x04)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP3=on");
    }
    if(mask & 0x08)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP4=on");
    }
    if(mask & 0x10)
    {
        strcat(data, "EXP5=on");
    }
    if(mask & 0x20)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP6=on");
    }
    if(mask & 0x40)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP7=on");
    }
    if(mask & 0x80)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP8=on");
    }

    sprintf(buffer, http_post, url_set_exstatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::ResetIO(const char *raddr, int mask)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    data[0] = 0;

    if(mask & 0x01)
    {
        strcat(data, "IO1=off");
    }
    if(mask & 0x02)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO2=off");
    }
    if(mask & 0x04)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO3=off");
    }
    if(mask & 0x08)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO4=off");
    }
    if(mask & 0x10)
    {
        strcat(data, "IO5=off");
    }
    if(mask & 0x20)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO6=off");
    }
    if(mask & 0x40)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO7=off");
    }
    if(mask & 0x80)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO8=off");
    }

    sprintf(buffer, http_post, url_set_iostatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::ResetOut(const char *raddr, int mask)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    data[0] = 0;

    if(mask & 0x01)
    {
        strcat(data, "OUT1=off");
    }
    if(mask & 0x02)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT2=off");
    }
    if(mask & 0x04)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT3=off");
    }
    if(mask & 0x08)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT4=off");
    }
    if(mask & 0x10)
    {
        strcat(data, "OUT5=off");
    }
    if(mask & 0x20)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT6=off");
    }
    if(mask & 0x40)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT7=off");
    }
    if(mask & 0x80)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT8=off");
    }

    sprintf(buffer, http_post, url_set_ostatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::ResetEX(const char *raddr, int mask)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    data[0] = 0;

    if(mask & 0x01)
    {
        strcat(data, "EXP1=off");
    }
    if(mask & 0x02)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP3=off");
    }
    if(mask & 0x04)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP4=off");
    }
    if(mask & 0x08)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP5=off");
    }
    if(mask & 0x10)
    {
        strcat(data, "EXP7=off");
    }
    if(mask & 0x20)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP8=off");
    }
    if(mask & 0x40)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP9=off");
    }

    sprintf(buffer, http_post, url_set_exstatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::SwitchIO(const char *raddr, int mask)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    data[0] = 0;

    if(mask & 0x01)
    {
        strcat(data, "IO1=on");
    }
    if(mask & 0x02)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO2=on");
    }
    if(mask & 0x04)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO3=on");
    }
    if(mask & 0x08)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO4=on");
    }
    sprintf(buffer, http_post, url_switch_iostatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::SwitchOut(const char *raddr, int mask)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    data[0] = 0;

    if(mask & 0x01)
    {
        strcat(data, "OUT1=on");
    }
    if(mask & 0x02)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT2=on");
    }
    if(mask & 0x04)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT3=on");
    }
    if(mask & 0x08)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT4=on");
    }
    sprintf(buffer, http_post, url_switch_ostatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::SwitchEX(const char *raddr, int mask)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    data[0] = 0;

    if(mask & 0x01)
    {
        strcat(data, "EXP1=on");
    }
    if(mask & 0x02)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP3=on");
    }
    if(mask & 0x04)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP4=on");
    }
    if(mask & 0x08)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP5=on");
    }
    if(mask & 0x10)
    {
        strcat(data, "EXP7=on");
    }
    if(mask & 0x20)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP8=on");
    }
    if(mask & 0x40)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP9=on");
    }

    sprintf(buffer, http_post, url_switch_exstatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::PulseIO(const char *raddr, int mask, int sec)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    data[0] = 0;

    sprintf(data, "PULSE=%i", sec);

    if(mask & 0x01)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO1=on");
    }
    if(mask & 0x02)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO2=on");
    }
    if(mask & 0x04)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO3=on");
    }
    if(mask & 0x08)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO4=on");
    }
    sprintf(buffer, http_post, url_pulse_iostatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::PulseOut(const char *raddr, int mask, int sec)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    data[0] = 0;

    sprintf(data, "PULSE=%i", sec);

    if(mask & 0x01)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT1=on");
    }
    if(mask & 0x02)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT2=on");
    }
    if(mask & 0x04)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT3=on");
    }
    if(mask & 0x08)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT4=on");
    }
    if(mask & 0x10)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT5=on");
    }
    if(mask & 0x20)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT6=on");
    }
    if(mask & 0x40)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT3=on");
    }
    if(mask & 0x80)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "OUT4=on");
    }
    sprintf(buffer, http_post, url_pulse_ostatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::PulseEX(const char *raddr, int mask, int sec)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    data[0] = 0;

    sprintf(data, "PULSE=%i", sec);

    if(mask & 0x01)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "IO1=on");
    }
    if(mask & 0x02)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP3=on");
    }
    if(mask & 0x04)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP4=on");
    }
    if(mask & 0x08)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP5=on");
    }
    if(mask & 0x10)
    {
        strcat(data, "EXP7=on");
    }
    if(mask & 0x20)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP8=on");
    }
    if(mask & 0x40)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "EXP9=on");
    }

    sprintf(buffer, http_post, url_pulse_exstatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::GetWifi(const char *raddr, wifi_config_data *config)
{
    char buffer[BUFFER_LEN+1], tmp[16];
    char *p;
    CTcp q;
    STRFunc Str;
    int rc;

    sprintf(buffer, http_post, url_get_wifi, raddr, 0, " ");
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Send [%s]", buffer);
    if(q.Query(raddr, 80, buffer, buffer, BUFFER_LEN, m_timeout) > 0)
    {
        if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Receive [%s]", buffer);
        rc = HttpRespCode(buffer);
        if(rc != 0) return rc;
        /* Me posiciono al final de la cabecera HTTP, al principio de los datos */
        p = strstr(buffer, "\r\n\r\n");
        if(p)
        {
            /* Salteo CR/LF CR/LF */
            p += 4;
            Str.ParseData(p, "ap1", config->wifi_ap1);
            Str.ParseData(p, "ap1p", config->wifi_ap1_pass);
            Str.ParseData(p, "ap2", config->wifi_ap2);
            Str.ParseData(p, "ap2p", config->wifi_ap2_pass);
            Str.ParseData(p, "ce1", config->wifi_host1);
            Str.ParseData(p, "ce2", config->wifi_host2);
            Str.ParseData(p, "ce1p", tmp);
            config->wifi_host1_port = atoi(tmp);
            Str.ParseData(p, "ce2p", tmp);
            config->wifi_host2_port = atoi(tmp);
            Str.ParseData(p, "rqst", config->rqst_path);
        }
        return 0;
    }
    return (-1);
}

int Dom32IoWifi::SetWifi(const char *raddr, wifi_config_data *config)
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    data[0] = 0;
    if(config->wifi_ap1[0])
    {
        strcpy(data, "ap1=");
        strcat(data, config->wifi_ap1);
    }
    if(config->wifi_ap1_pass[0])
    {
        if(data[0]) strcat(data, "&");
        strcat(data, "ap1p=");
        strcat(data, config->wifi_ap1_pass);
    }
    if(config->wifi_ap2[0])
    {
        if(data[0]) strcat(data, "&");
        strcat(data, "ap2=");
        strcat(data, config->wifi_ap2);
    }
    if(config->wifi_ap2_pass[0])
    {
        if(data[0]) strcat(data, "&");
        strcat(data, "ap2p=");
        strcat(data, config->wifi_ap2_pass);
    }
    if(config->wifi_host1[0])
    {
        if(data[0]) strcat(data, "&");
        strcat(data, "ce1=");
        strcat(data, config->wifi_host1);
    }
    if(config->wifi_host2[0])
    {
        if(data[0]) strcat(data, "&");
        strcat(data, "ce2=");
        strcat(data, config->wifi_host2);
    }
    sprintf(buffer, http_post, url_set_wifi, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::IO2Int(const char* str)
{
    int rc = 0;
    char tmp[16];
    STRFunc Str;

    /* Interpreto los datos */
    if(Str.ParseData(str, "IO1", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x01;
        }
    }
    if(Str.ParseData(str, "IO2", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x02;
        }
    }
    if(Str.ParseData(str, "IO3", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x04;
        }
    }
    if(Str.ParseData(str, "IO4", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x08;
        }
    }
    if(Str.ParseData(str, "IO5", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x10;
        }
    }
    if(Str.ParseData(str, "IO6", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x20;
        }
    }
    if(Str.ParseData(str, "IO7", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x40;
        }
    }
    if(Str.ParseData(str, "IO8", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x80;
        }
    }
    return rc;
}

int Dom32IoWifi::Out2Int(const char* str)
{
    int rc = 0;
    char tmp[16];
    STRFunc Str;

    /* Interpreto los datos */
    if(Str.ParseData(str, "OUT1", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x01;
        }
    }
    if(Str.ParseData(str, "OUT2", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x02;
        }
    }
    if(Str.ParseData(str, "OUT3", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x04;
        }
    }
    if(Str.ParseData(str, "OUT4", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x08;
        }
    }
    if(Str.ParseData(str, "OUT5", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x10;
        }
    }
    if(Str.ParseData(str, "OUT6", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x20;
        }
    }
    if(Str.ParseData(str, "OUT7", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x40;
        }
    }
    if(Str.ParseData(str, "OUT8", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x80;
        }
    }
    return rc;
}

int Dom32IoWifi::EXP2Int(const char* str)
{
    int rc = 0;
    char tmp[16];
    STRFunc Str;

    if(Str.ParseData(str, "EXP1", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x01;
        }
    }
    if(Str.ParseData(str, "EXP2", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x02;
        }
    }
    if(Str.ParseData(str, "EXP3", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x04;
        }
    }
    if(Str.ParseData(str, "EXP4", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x08;
        }
    }
    if(Str.ParseData(str, "EXP5", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x10;
        }
    }
    if(Str.ParseData(str, "EXP6", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x20;
        }
    }
    if(Str.ParseData(str, "EXP7", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x40;
        }
    }
    if(Str.ParseData(str, "EXP8", tmp))
    {
        if( !strcmp("on", tmp) || !strcmp("in", tmp))
        {
            rc += 0x80;
        }
    }
    return rc;
}

int Dom32IoWifi::HttpRespCode(const char* http)
{
    char tmp[16];
    int rc;
    STRFunc Str;

    Str.Section(http, ' ', 1, tmp);

    rc = atoi(tmp);
    return (rc == 200)?0:rc;
}

void Dom32IoWifi::Task( void )
{
    int i;
    char *p;

    for(i = 0; i < MAX_QUEUE_COUNT && m_queue_list[i].addr[0] != 0; i++)
    {
        if( m_queue_list[i].delay == 0 )
        {
            if(QueueView(m_queue_list[i].id, (void**)&p))
            {
                if(RequestDequeue(m_queue_list[i].addr, p) == 0)
                {
                    QueueDel(m_queue_list[i].id);
                    m_queue_list[i].delay = 3;
                }
                else
                {
                    m_queue_list[i].delay = 30;
                }
            }
        }
    }
}

void Dom32IoWifi::Timer( void )
{
    int i;

    for(i = 0; i < MAX_QUEUE_COUNT && m_queue_list[i].addr[0] != 0; i++)
    {
        if( m_queue_list[i].delay ) m_queue_list[i].delay--;
    }
}

int Dom32IoWifi::RequestEnqueue(const char* dest, const char* data)
{
    int i;

    for(i = 0; i < MAX_QUEUE_COUNT && m_queue_list[i].addr[0] != 0; i++)
    {
        /* Busco la cola ´para este destinatario */
        if( !strcmp(m_queue_list[i].addr, dest)) break;
    }
    if(i < MAX_QUEUE_COUNT)
    {
        if(m_queue_list[i].addr[0] == 0)
        {
            /* Cola Nueva */
            m_queue_list[i].id = QueueOpen(WIFI_MSG_MAX_QUEUE, WIFI_MSG_MAX_LEN, m_queue_list[i].buffer);
            if(m_queue_list[i].id == INVALID_QUEUE) return (-1);
            strcpy(m_queue_list[i].addr, dest);
        }
        QueueAdd(m_queue_list[i].id, (void*)data);
        return 0;
    }
    return (-1);
}

int Dom32IoWifi::RequestDequeue(const char* dest, const char* data)
{
    CTcp q;
    char buffer[WIFI_MSG_MAX_LEN];

    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Send [%s]", data);
    if(q.Query(dest, m_port, data, buffer, WIFI_MSG_MAX_LEN, 3000) > 0)
    {
        if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Receive [%s]", buffer);
        return HttpRespCode(buffer);
    }
    return (-1);
}
