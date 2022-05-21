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
    int ConfigIO(const char *raddr, int ioconfig, int *config);
    int ConfigOut(const char *raddr, int ioconfig, int *config);
    int ConfigEX(const char *raddr, int exconfig, int *config);
    int SetIO(const char *raddr, int mask, int *iostatus);
    int SetOut(const char *raddr, int mask, int *ostatus);
    int SetEX(const char *raddr, int mask, int *exstatus);
    int ResetIO(const char *raddr, int mask, int *iostatus);
    int ResetOut(const char *raddr, int mask, int *ostatus);
    int ResetEX(const char *raddr, int mask, int *exstatus);
    int SwitchIO(const char *raddr, int mask, int *iostatus);
    int SwitchOut(const char *raddr, int mask, int *ostatus);
    int SwitchEX(const char *raddr, int mask, int *exstatus);
    int PulseIO(const char *raddr, int mask, int sec, int *iostatus);
    int PulseOut(const char *raddr, int mask, int sec, int *ostatus);
    int PulseEX(const char *raddr, int mask, int sec, int *exstatus);
    int GetWifi(const char *raddr, wifi_config_data *config);
    int SetWifi(const char *raddr, wifi_config_data *config);

    int m_timeout;

protected:
    const char *http_post;
    const char *http_get;

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

    int IO2Int(const char* str);
    int Out2Int(const char* str);
    int EXP2Int(const char* str);
    int HttpRespCode(const char* http);

    CGLog *m_pLog;
};

#endif /* _DOM32IOWIFI_H_ */