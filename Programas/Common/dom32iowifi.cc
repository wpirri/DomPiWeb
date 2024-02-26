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

Dom32IoWifi::Dom32IoWifi(CGMServerWait *pServer)
{
    if(pServer)
    {
        m_pLog = pServer->m_pLog;
        m_pServer = pServer;
    }
    else
    {
        m_pLog = nullptr;
        m_pServer = nullptr;
    }
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

int Dom32IoWifi::GetWifiConfig(const char *raddr, wifi_config_data *config, void(*fcn)(const char* id, const char* data))
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
        if(rc != 200) return rc;
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
            if(Str.ParseData(p, "ce1p", tmp))
            {
                config->wifi_host1_port = atoi(tmp);
            }
            if(Str.ParseData(p, "ce2p", tmp))
            {
                config->wifi_host2_port = atoi(tmp);
            }
            if(Str.ParseData(p, "report", tmp))
            {
                config->report = atoi(tmp);
            }
            Str.ParseData(p, "rqst", config->rqst_path);

            if(fcn)
            {
                (fcn)(raddr, p);
            }
        }
        return 0;
    }
    return (-1);
}

int Dom32IoWifi::SetWifiConfig(const char *raddr, wifi_config_data *config, void(*fcn)(const char* id, const char* data))
{
    char buffer[BUFFER_LEN+1];
    char data[256];

    strcpy(data, "ap1=");
    if( config->wifi_ap1[0] && strcmp(config->wifi_ap1, "NULL") ) 
    {
        strcat(data, config->wifi_ap1);
    }
    strcat(data, "&ap1p=");
    if(config->wifi_ap1_pass[0] && strcmp(config->wifi_ap1_pass, "NULL") ) 
    {
        strcat(data, config->wifi_ap1_pass);
    }
    strcat(data, "&ap2=");
    if(config->wifi_ap2[0] && strcmp(config->wifi_ap2, "NULL") ) 
    {
        strcat(data, config->wifi_ap2);
    }
    strcat(data, "&ap2p=");
    if(config->wifi_ap2_pass[0] && strcmp(config->wifi_ap2_pass, "NULL") ) 
    {
        strcat(data, config->wifi_ap2_pass);
    }
    strcat(data, "&ce1=");
    if(config->wifi_host1[0] && strcmp(config->wifi_host1, "NULL") ) 
    {
        strcat(data, config->wifi_host1);
    }
    strcat(data, "&ce1p=");
    sprintf(&data[strlen(data)], "%u", config->wifi_host1_port);
    strcat(data, "&ce2=");
    if(config->wifi_host2[0] && strcmp(config->wifi_host2, "NULL") ) 
    {
        strcat(data, config->wifi_host2);
    }
    strcat(data, "&ce2p=");
    sprintf(&data[strlen(data)], "%u", config->wifi_host2_port);
    strcat(data, "&report=");
    sprintf(&data[strlen(data)], "%u", config->report);
    strcat(data, "&path=");
    if(config->rqst_path[0] && strcmp(config->rqst_path, "NULL") ) 
    {
        strcat(data, config->rqst_path);
    }

    sprintf(buffer, http_post, url_set_wifi, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(20, "[Dom32IoWifi] Encolando configuracion WiFi para %s", raddr);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] [%s]", buffer);
    return RequestEnqueue(raddr, buffer, fcn);
}

int Dom32IoWifi::GetConfig(const char *raddr, char *msg, int max_msg_len, void(*fcn)(const char* id, const char* data))
{
    int rc;
    cJSON *json_obj;
    json_obj = cJSON_Parse(msg);
    rc = GetConfig(raddr, json_obj, fcn);
    cJSON_PrintPreallocated(json_obj, msg, max_msg_len, 0);
    cJSON_Delete(json_obj);
    return rc;
}

int Dom32IoWifi::SetConfig(const char *raddr, char *msg, void(*fcn)(const char* id, const char* data))
{
    int rc;
    cJSON *json_obj;
    json_obj = cJSON_Parse(msg);
    rc = SetConfig(raddr, json_obj, fcn);
    cJSON_Delete(json_obj);
    return rc;
}

int Dom32IoWifi::GetIO(const char *raddr, char *msg, int max_msg_len, void(*fcn)(const char* id, const char* data))
{
    int rc;
    cJSON *json_obj;
    json_obj = cJSON_Parse(msg);
    rc = GetIO(raddr, json_obj, fcn);
    cJSON_PrintPreallocated(json_obj, msg, max_msg_len, 0);
    cJSON_Delete(json_obj);
    return rc;
}

int Dom32IoWifi::SetIO(const char *raddr, char* msg, void(*fcn)(const char* id, const char* data))
{
    int rc;
    cJSON *json_obj;
    json_obj = cJSON_Parse(msg);
    rc = SetIO(raddr, json_obj, fcn);
    cJSON_Delete(json_obj);
    return rc;
}

int Dom32IoWifi::SwitchIO(const char *raddr, char* msg, void(*fcn)(const char* id, const char* data))
{
    int rc;
    cJSON *json_obj;
    json_obj = cJSON_Parse(msg);
    rc = SwitchIO(raddr, json_obj, fcn);
    cJSON_Delete(json_obj);
    return rc;
}

int Dom32IoWifi::PulseIO(const char *raddr, char* msg, void(*fcn)(const char* id, const char* data))
{
    int rc;
    cJSON *json_obj;
    json_obj = cJSON_Parse(msg);
    rc = PulseIO(raddr, json_obj, fcn);
    cJSON_Delete(json_obj);
    return rc;
}

int Dom32IoWifi::GetConfig(const char *raddr, cJSON *json_obj, void(*fcn)(const char* id, const char* data))
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
        if(rc != 200) return rc;
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
            if(fcn)
            {
                (fcn)(raddr, p);
            }
        }
        return 0;
    }
    return (-1);
}

int Dom32IoWifi::SetConfig(const char *raddr, cJSON *json_obj, void(*fcn)(const char* id, const char* data))
{
    char buffer[BUFFER_LEN+1];
    char data[1024];
    cJSON *json_Ports, *json_un_obj;
    cJSON *json_Port;
    cJSON *json_Tipo_ASS;
    cJSON *json_flag_HTTPS;
    cJSON *json_flag_WIEGAND;
 //   cJSON *json_flag_DHT2x;

    data[0] = 0;

    if(m_pLog) m_pLog->Add(10, "[Dom32IoWifi] Enviando Config a: %s", raddr);

    cJSON_PrintPreallocated(json_obj, buffer, BUFFER_LEN, 0);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi::SetConfig] raddr: %s json: %s", raddr, buffer);

    json_flag_HTTPS = cJSON_GetObjectItemCaseSensitive(json_obj, "HTTPS");
    if(json_flag_HTTPS)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "HTTPS");
        strcat(data, "=");
        strcat(data, json_flag_HTTPS->valuestring);
    }
    json_flag_WIEGAND = cJSON_GetObjectItemCaseSensitive(json_obj, "WIEGAND");
    if(json_flag_WIEGAND)
    {
        if(data[0] != 0) strcat(data, "&");
        strcat(data, "WIEGAND");
        strcat(data, "=");
        strcat(data, json_flag_WIEGAND->valuestring);
    }
//    json_flag_DHT2x = cJSON_GetObjectItemCaseSensitive(json_obj, "DHT2x");
//    if(json_flag_DHT2x)
//    {
//        if(data[0] != 0) strcat(data, "&");
//        strcat(data, "DHT2x");
//        strcat(data, "=");
//        strcat(data, json_flag_DHT2x->valuestring);
//    }

    json_Ports = cJSON_GetObjectItemCaseSensitive(json_obj, "Ports");

    cJSON_PrintPreallocated(json_Ports, buffer, BUFFER_LEN, 0);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi::SetConfig] Ports: %s", buffer);

    cJSON_ArrayForEach(json_un_obj, json_Ports)
    {
        json_Port = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Port");
        json_Tipo_ASS = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Tipo_ASS");

        if(json_Port && json_Tipo_ASS)
        {
            /* Para tipos conocidos de port */
            if( !memcmp(json_Port->valuestring, "IO", 2) || !memcmp(json_Port->valuestring, "OUT", 3))
            {
                if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi::SetConfig] %s = %s", json_Port->valuestring, json_Tipo_ASS->valuestring);

                if(data[0] != 0) strcat(data, "&");
                strcat(data, json_Port->valuestring);
                strcat(data, "=");
                if( !strcmp(json_Tipo_ASS->valuestring, "0") || !strcmp(json_Tipo_ASS->valuestring, "3") || !strcmp(json_Tipo_ASS->valuestring, "5"))
                {
                    strcat(data, "out");
                }
                else if( !strcmp(json_Tipo_ASS->valuestring, "1") || !strcmp(json_Tipo_ASS->valuestring, "4"))
                {
                    strcat(data, "in");
                }
                else if( !strcmp(json_Tipo_ASS->valuestring, "2"))
                {
                    strcat(data, "ana");
                }
                else
                {
                    strcat(data, "err");
                }
            }
        }
    }
    sprintf(buffer, http_post, url_set_ioconfig, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(20, "[Dom32IoWifi] Encolando configuracion de I/O para %s", raddr);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] [%s]", buffer);
    return RequestEnqueue(raddr, buffer, fcn);
}

int Dom32IoWifi::SetTime(const char *raddr, void(*fcn)(const char* id, const char* data))
{
    time_t t;
    struct tm *tm_time;
    char buffer[BUFFER_LEN+1];
    char data[1024];

    t = time(&t);
    tm_time = localtime(&t);

    sprintf(data, "TIME=%04i/%02i/%02i %02i:%02i:%02i",
        tm_time->tm_year, tm_time->tm_mon, tm_time->tm_mday,
        tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);

    sprintf(buffer, http_post, url_set_ioconfig, raddr, strlen(data), data);
    if(m_pLog) m_pLog->Add(20, "[Dom32IoWifi] Encolando configuracion de fecha y hora para %s", raddr);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] [%s]", buffer);
    return RequestEnqueue(raddr, buffer, fcn);
}

int Dom32IoWifi::GetIO(const char *raddr, cJSON *json_obj, void(*fcn)(const char* id, const char* data))
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
        if(rc != 200) return rc;
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
            if(fcn)
            {
                (fcn)(raddr, p);
            }
        }
        return 0;
    }
    return (-1);

}

int Dom32IoWifi::SetIO(const char *raddr, cJSON *json_obj, void(*fcn)(const char* id, const char* data))
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
    if(m_pLog) m_pLog->Add(20, "[Dom32IoWifi] Encolando estado de I/O para %s", raddr);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] [%s]", buffer);
    return RequestEnqueue(raddr, buffer, fcn);
}

int Dom32IoWifi::SwitchIO(const char *raddr, cJSON *json_obj, void(*fcn)(const char* id, const char* data))
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
    if(m_pLog) m_pLog->Add(20, "[Dom32IoWifi] Encolando switch de I/O para %s", raddr);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] [%s]", buffer);
    return RequestEnqueue(raddr, buffer, fcn);
}

int Dom32IoWifi::PulseIO(const char *raddr, cJSON *json_obj, void(*fcn)(const char* id, const char* data))
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
                        else if( !strcmp(json_un_obj->string, "Analog_Mult_Div_Valor"))
                        {
                            if(atoi(json_un_obj->valuestring) > 0)
                            {
                                strcpy(estado, json_un_obj->valuestring);
                            }
                            else
                            {
                                strcpy(estado, "1");
                            }
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
    if(m_pLog) m_pLog->Add(20, "[Dom32IoWifi] Encolando pulso de I/O para %s", raddr);
    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] [%s]", buffer);
    return RequestEnqueue(raddr, buffer, fcn);
}

int Dom32IoWifi::HttpRespCode(const char* http)
{
    char tmp[16];
    STRFunc Str;

    Str.Section(http, ' ', 1, tmp);

    return atoi(tmp);
}

int Dom32IoWifi::HttpData(const char* http, char* data)
{
    char* p;

    *data = 0;

    p = strstr((char*)http, "\r\n\r\n");
    if(p)
    {
        strcpy(data, p+4);
    }

    return strlen(data);
}


void Dom32IoWifi::Task( void )
{
    int i;
    queue_data *p;

    for(i = 0; i < MAX_QUEUE_COUNT && m_queue_list[i].addr[0] != 0; i++)
    {
        if( m_queue_list[i].delay == 0 )
        {
            if(QueueView(m_queue_list[i].id, (void**)&p))
            {
                if(RequestDequeue(m_queue_list[i].addr, p, m_queue_list[i].retry) == 0)
                {
                    QueueDel(m_queue_list[i].id);
                    m_queue_list[i].retry = 0;
                    m_queue_list[i].delay = 2;
                }
                else
                {
                    if(++m_queue_list[i].retry == 100)
                    {
                        QueueDel(m_queue_list[i].id);
                        m_queue_list[i].retry = 0;
                        m_queue_list[i].delay = 0;
                    }
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

int Dom32IoWifi::RequestEnqueue(const char* dest, const char* data, void(*fcn)(const char* id, const char* data))
{
    int i;
    queue_data qd;

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
            m_queue_list[i].id = QueueOpen(WIFI_MSG_MAX_QUEUE, sizeof(queue_data), m_queue_list[i].buffer);
            if(m_queue_list[i].id == INVALID_QUEUE) return (-1);
            strcpy(m_queue_list[i].addr, dest);
        }
        qd.fcn = fcn;
        strcpy(qd.buffer, data);
        QueueAdd(m_queue_list[i].id, (void*)&qd);
        m_queue_list[i].delay = 0;
        m_queue_list[i].retry = 0;
        return 0;
    }
    return (-1);
}

int Dom32IoWifi::RequestDequeue(const char* dest, queue_data* qdata, unsigned int retry)
{
    CTcp q;
    char buffer[WIFI_MSG_MAX_LEN];
    char msg[WIFI_MSG_MAX_LEN];
    int rc;
    int i;
    STRFunc Str;
    char label[256];
    char value[256];
    cJSON *json_request;

    if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Send [%s] retry: %u", qdata->buffer, retry);
    if(q.Query(dest, m_port, qdata->buffer, buffer, WIFI_MSG_MAX_LEN, 1500) > 0)
    {
        if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] Receive [%s]", buffer);
        rc = HttpRespCode(buffer);
        if(rc == 200)
        {
            /* Hay que evaluar los datos de la respuesta */
            if(HttpData(buffer, msg) > 0)
            {
                /*
                ID=ecfabc3b6690&IO1=in&IO2=in&IO3=in&IO4=in&IO5=in&IO6=in

                {"ID":"ecfabc3b6642","IO1":"1","IO2":"0","IO3":"1","IO4":"1","IO5":"1","IO6":"1","OUT1":"0","OUT2":"1","OUT3":"0","OUT4":"0","GETCONF":"1"}
                
                */
                json_request = cJSON_CreateObject();
                for(i = 0; Str.ParseDataIdx(msg, label, value, i); i++)
                {
                    cJSON_AddStringToObject(json_request, label, value);                    
                }
                cJSON_PrintPreallocated(json_request, msg, 255, 0);
                /* Si trae datos son de status de la interface */
                if(qdata->fcn)
                {
                    (qdata->fcn)(dest, msg);
                }
                cJSON_Delete(json_request);
            }
            rc = 0;
        }
        return rc;
    }
    return (-1);
}
