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
#ifndef _DOM32IOWIFI_H_
#define _DOM32IOWIFI_H_

#include <gmonitor/glog.h>
#include <cjson/cJSON.h>

/* Definiciones para los bits de flag */
#define FLAG_HTTPS_ENABLE    0x01
#define FLAG_WIEGAND_ENABLE  0x02
#define FLAG_DHT2x_ENABLE    0x04
#define FLAG_HTTPS_DISABLE    0x01^0xFF
#define FLAG_WIEGAND_DISABLE  0x02^0xFF
#define FLAG_DHT2x_DISABLE    0x04^0xFF

#define WIFI_MSG_MAX_LEN    4096
#define WIFI_MSG_MAX_QUEUE  8

class Dom32IoWifi
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
    } wifi_config_data;

	Dom32IoWifi(CGLog *pLog = NULL);
	virtual ~Dom32IoWifi();

    int GetIOStatus(const char *raddr, int *iostatus);
    int GetOutStatus(const char *raddr, int *ostatus);
    int GetEXStatus(const char *raddr, int *exstatus);
    int GetConfig(const char *raddr, int *ioconfig, int *exconfig);
    int ConfigIO(const char *raddr, int ioconfig, int anconfig);
    int ConfigOut(const char *raddr, int ioconfig);
    int ConfigEX(const char *raddr, int exconfig);
    int ConfigFlags(const char *raddr, int flags);
    int GetWifi(const char *raddr, wifi_config_data *config);
    int SetWifi(const char *raddr, wifi_config_data *config);

    int SetIO(const char *raddr, const char *msg);
    int SwitchIO(const char *raddr, const char *msg);
    int PulseIO(const char *raddr, const char *msg);

    int SetIO(const char *raddr, cJSON *json);
    int SwitchIO(const char *raddr, cJSON *json);
    int PulseIO(const char *raddr, cJSON *json);

    void Task( void );
    void Timer( void );

    int m_timeout;

protected:
    typedef struct _queue_list
    {
        char addr[16];
        unsigned char id;
        char buffer[WIFI_MSG_MAX_LEN*WIFI_MSG_MAX_QUEUE];
        unsigned int delay;
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

    int IO2Int(const char* str);
    int Out2Int(const char* str);
    int EXP2Int(const char* str);
    int HttpRespCode(const char* http);
    int RequestEnqueue(const char* dest, const char* data);
    int RequestDequeue(const char* dest, const char* data);

};

#endif /* _DOM32IOWIFI_H_ */