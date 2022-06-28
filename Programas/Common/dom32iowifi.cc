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
    url_switch_iostatus = "/ioswitch.cgi";

    url_set_ioconfig = "/ioconfig.cgi";

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

    url_get_config = "/config.htm";

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

int Dom32IoWifi::GetWifiConfig(const char *raddr, wifi_config_data *config)
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

int Dom32IoWifi::SetWifiConfig(const char *raddr, wifi_config_data *config)
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

int Dom32IoWifi::GetConfig(const char *raddr, char *msg, int max_msg_len)
{
    int rc;
    cJSON *json_obj;
    json_obj = cJSON_Parse(msg);
    rc = GetConfig(raddr, json_obj);
    cJSON_PrintPreallocated(json_obj, msg, max_msg_len, 0);
    cJSON_Delete(json_obj);
    return rc;
}

int Dom32IoWifi::SetConfig(const char *raddr, char *msg)
{
    int rc;
    cJSON *json_obj;
    json_obj = cJSON_Parse(msg);
    rc = SetConfig(raddr, json_obj);
    cJSON_Delete(json_obj);
    return rc;
}

int Dom32IoWifi::GetIO(const char *raddr, char *msg, int max_msg_len)
{
    int rc;
    cJSON *json_obj;
    json_obj = cJSON_Parse(msg);
    rc = GetIO(raddr, json_obj);
    cJSON_PrintPreallocated(json_obj, msg, max_msg_len, 0);
    cJSON_Delete(json_obj);
    return rc;
}

int Dom32IoWifi::SetIO(const char *raddr, char* msg)
{
    int rc;
    cJSON *json_obj;
    json_obj = cJSON_Parse(msg);
    rc = SetIO(raddr, json_obj);
    cJSON_Delete(json_obj);
    return rc;
}

int Dom32IoWifi::SwitchIO(const char *raddr, char* msg)
{
    int rc;
    cJSON *json_obj;
    json_obj = cJSON_Parse(msg);
    rc = SwitchIO(raddr, json_obj);
    cJSON_Delete(json_obj);
    return rc;
}

int Dom32IoWifi::PulseIO(const char *raddr, char* msg)
{
    int rc;
    cJSON *json_obj;
    json_obj = cJSON_Parse(msg);
    rc = PulseIO(raddr, json_obj);
    cJSON_Delete(json_obj);
    return rc;
}

int Dom32IoWifi::GetConfig(const char *raddr, cJSON *json_obj)
{
    char buffer[BUFFER_LEN+1];
    char *p;
    CTcp q;
    int rc;
    int i;
    STRFunc Str;
    char label[256];
    char value[256];

    sprintf(buffer, http_get, url_get_config, raddr);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] GetConfig Send [%s]", buffer);

    if(q.Query(raddr, 80, buffer, buffer, BUFFER_LEN, m_timeout) > 0)
    {
        if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] GetConfig Receive [%s]", buffer);
        rc = HttpRespCode(buffer);
        if(rc != 0) return rc;
        /* Me posiciono al final de la cabecera HTTP, al principio de los datos */
        p = strstr(buffer, "\r\n\r\n");
        if(p)
        {
            /* Salteo CR/LF CR/LF */
            p += 4;
            for(i = 0; Str.ParseDataIdx(p, label, value, i); i++)
            {
                cJSON_AddStringToObject(json_obj, label, value);                    
            }
        }
        return 0;
    }
    return (-1);
}

int Dom32IoWifi::SetConfig(const char *raddr, cJSON *json_obj)
{
    char buffer[BUFFER_LEN+1];
    char data[1024];
    char port[16];
    char estado[16];
    cJSON *json_un_obj;

    json_un_obj = json_obj;
    data[0] = 0;
    port[0] = 0;
    estado[0] = 0;
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
                        if( !strcmp(json_un_obj->string, "Port"))
                        {
                            strcpy(port, json_un_obj->valuestring);
                        }
                        else if( !strcmp(json_un_obj->string, "Tipo_ASS"))
                        {
                            strcpy(estado, json_un_obj->valuestring);
                        }
                        if(port[0] && estado[0])
                        {
                            if(data[0] != 0) strcat(data, "&");
                            strcat(data, port);
                            strcat(data, "=");
                            if( !strcmp(estado, "0"))
                            {
                                strcat(data, "out");
                            }
                            else if( !strcmp(estado, "1"))
                            {
                                strcat(data, "in");
                            }
                            else
                            {
                                strcat(data, estado);
                            }
                            port[0] = 0;
                            estado[0] = 0;
                        }
                    }
                }
            }
            json_un_obj = json_un_obj->next;
        }
    }
    sprintf(buffer, http_post, url_set_ioconfig, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] SetConfig Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::GetIO(const char *raddr, cJSON *json_obj)
{
    char buffer[BUFFER_LEN+1];
    char *p;
    CTcp q;
    int rc;
    int i;
    STRFunc Str;
    char label[256];
    char value[256];

    sprintf(buffer, http_get, url_get_iostatus, raddr);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] GetIO Send [%s]", buffer);
    if(q.Query(raddr, 80, buffer, buffer, BUFFER_LEN, m_timeout) > 0)
    {
        if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] GetIO Receive [%s]", buffer);
        rc = HttpRespCode(buffer);
        if(rc != 0) return rc;
        /* Me posiciono al final de la cabecera HTTP, al principio de los datos */
        p = strstr(buffer, "\r\n\r\n");
        if(p)
        {
            /* Salteo CR/LF CR/LF */
            p += 4;
            for(i = 0; Str.ParseDataIdx(p, label, value, i); i++)
            {
                cJSON_AddStringToObject(json_obj, label, value);                    
            }
        }
        return 0;
    }
    return (-1);

}

int Dom32IoWifi::SetIO(const char *raddr, cJSON *json_obj)
{
    char buffer[BUFFER_LEN+1];
    char data[1024];
    char port[16];
    char estado[16];
    cJSON *json_un_obj;

    json_un_obj = json_obj;
    data[0] = 0;
    port[0] = 0;
    estado[0] = 0;
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
                        if( !strcmp(json_un_obj->string, "Port"))
                        {
                            strcpy(port, json_un_obj->valuestring);
                        }
                        else if( !strcmp(json_un_obj->string, "Estado"))
                        {
                            strcpy(estado, json_un_obj->valuestring);
                        }
                        if(port[0] && estado[0])
                        {
                            if(data[0] != 0) strcat(data, "&");
                            strcat(data, port);
                            strcat(data, "=");
                            strcat(data, estado);
                            port[0] = 0;
                            estado[0] = 0;
                        }
                    }
                }
            }
            json_un_obj = json_un_obj->next;
        }
    }
    sprintf(buffer, http_post, url_set_iostatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] SetIO Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::SwitchIO(const char *raddr, cJSON *json_obj)
{
    char buffer[BUFFER_LEN+1];
    char data[1024];
    char port[16];
    char estado[16];
    cJSON *json_un_obj;

    json_un_obj = json_obj;
    data[0] = 0;
    port[0] = 0;
    estado[0] = 0;
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
                        if( !strcmp(json_un_obj->string, "Port"))
                        {
                            strcpy(port, json_un_obj->valuestring);
                        }
                        else if( !strcmp(json_un_obj->string, "Estado"))
                        {
                            strcpy(estado, json_un_obj->valuestring);
                        }
                        if(port[0] && estado[0])
                        {
                            if(data[0] != 0) strcat(data, "&");
                            strcat(data, port);
                            strcat(data, "=");
                            strcat(data, estado);
                            port[0] = 0;
                            estado[0] = 0;
                        }
                    }
                }
            }
            json_un_obj = json_un_obj->next;
        }
    }
    sprintf(buffer, http_post, url_switch_iostatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] SwitchIO Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
}

int Dom32IoWifi::PulseIO(const char *raddr, cJSON *json_obj)
{
    char buffer[BUFFER_LEN+1];
    char data[1024];
    char port[16];
    char estado[16];
    cJSON *json_un_obj;

    json_un_obj = json_obj;
    data[0] = 0;
    port[0] = 0;
    estado[0] = 0;
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
                        if( !strcmp(json_un_obj->string, "Port"))
                        {
                            strcpy(port, json_un_obj->valuestring);
                        }
                        else if( !strcmp(json_un_obj->string, "Estado"))
                        {
                            strcpy(estado, json_un_obj->valuestring);
                        }
                        if(port[0] && estado[0])
                        {
                            if(data[0] != 0) strcat(data, "&");
                            strcat(data, "PULSE-");
                            strcat(data, port);
                            strcat(data, "=");
                            strcat(data, estado);
                            port[0] = 0;
                            estado[0] = 0;
                        }
                    }
                }
            }
            json_un_obj = json_un_obj->next;
        }
    }
    sprintf(buffer, http_post, url_set_iostatus, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] PulseIO Encolando [%s]", buffer);
    return RequestEnqueue(raddr, buffer);
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
