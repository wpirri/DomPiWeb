/***************************************************************************
  Copyright (C) 2020   Walter Pirri
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
#ifndef _RBPIIO_H_
#define _RBPIIO_H_

#include <gmonitor/gmswaited.h>
#include <cjson/cJSON.h>


#define RBPI_MSG_MAX_LEN    4096
#define RBPI_MSG_MAX_QUEUE  8

class RBPiIO
{
public:
    typedef struct
    {
        char wifi_ap1[33];
        char wifi_ap1_pass[33];
        char wifi_ap2[33];
        char wifi_ap2_pass[33];
        char wifi_host1[33];
        char wifi_host2[33];
        unsigned int wifi_host1_port;
        unsigned int wifi_host2_port;
        char rqst_path[33];
    } rbpi_config_data;

	RBPiIO(CGMServerWait *pServer = NULL);
	virtual ~RBPiIO();

    int GetWifiConfig(const char *raddr, rbpi_config_data *config, void(*fcn)(const char* id, const char* data));
    int SetWifiConfig(const char *raddr, rbpi_config_data *config, void(*fcn)(const char* id, const char* data));

    int GetConfig(const char *raddr, char *msg, int max_msg_len, void(*fcn)(const char* id, const char* data));
    int SetConfig(const char *raddr, char *msg, void(*fcn)(const char* id, const char* data));
    int SetTime(const char *raddr, void(*fcn)(const char* id, const char* data));
    int GetIO(const char *raddr, char *msg, int max_msg_len, void(*fcn)(const char* id, const char* data));
    int SetIO(const char *raddr, char *msg, void(*fcn)(const char* id, const char* data));
    int SwitchIO(const char *raddr, char *msg, void(*fcn)(const char* id, const char* data));
    int PulseIO(const char *raddr, char *msg, void(*fcn)(const char* id, const char* data));

    int GetConfig(const char *raddr, cJSON *json, void(*fcn)(const char* id, const char* data));
    int SetConfig(const char *raddr, cJSON *json, void(*fcn)(const char* id, const char* data));
    int GetIO(const char *raddr, cJSON *json, void(*fcn)(const char* id, const char* data));
    int SetIO(const char *raddr, cJSON *json, void(*fcn)(const char* id, const char* data));
    int SwitchIO(const char *raddr, cJSON *json, void(*fcn)(const char* id, const char* data));
    int PulseIO(const char *raddr, cJSON *json, void(*fcn)(const char* id, const char* data));

    void Task( void );
    void Timer( void );

    int m_timeout;

protected:
    typedef struct _queue_data
    {
        char buffer[RBPI_MSG_MAX_LEN];
        void(*fcn)(const char* id, const char* data);
    } queue_data;
    
    typedef struct _queue_list
    {
        char addr[16];
        unsigned char id;
        char buffer[sizeof(queue_data)*RBPI_MSG_MAX_QUEUE];
        unsigned int delay;
        unsigned int retry;
    } queue_list;

    queue_list m_queue_list[256];

    const char *http_post;
    const char *http_get;
    int m_port;

    const char *url_get_iostatus;
    const char *url_get_ostatus;
    const char *url_set_iostatus;
    const char *url_set_ostatus;
    const char *url_switch_iostatus;
    const char *url_switch_ostatus;
    const char *url_pulse_iostatus;
    const char *url_pulse_ostatus;
    const char *url_set_ioconfig;
    const char *url_get_exstatus;
    const char *url_set_exstatus;
    const char *url_switch_exstatus;
    const char *url_pulse_exstatus;
    const char *url_set_exconfig;
    const char *url_get_config;
    const char *url_get_ioconfig;
    const char *url_get_exconfig;
    const char *url_get_wifi;
    const char *url_set_wifi;

    CGLog *m_pLog;
    CGMServerWait *m_pServer;

    int HttpRespCode(const char* http);
    int HttpData(const char* http, char* data);
    int RequestEnqueue(const char* dest, const char* data, void(*fcn)(const char* id, const char* data));
    int RequestDequeue(const char* dest, queue_data* data, unsigned int retry);

};

#endif /* _RBPIIO_H_ */