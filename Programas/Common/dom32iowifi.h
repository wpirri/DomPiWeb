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

	Dom32IoWifi();
	virtual ~Dom32IoWifi();

    int GetIOStatus(const char *raddr, int *iostatus);
    int GetEXStatus(const char *raddr, int *exstatus);
    int GetConfig(const char *raddr, int *ioconfig, int *exconfig);
    int ConfigIO(const char *raddr, int ioconfig, int *config);
    int ConfigEX(const char *raddr, int exconfig, int *config);
    int SetIO(const char *raddr, int mask, int *iostatus);
    int SetEX(const char *raddr, int mask, int *exstatus);
    int ResetIO(const char *raddr, int mask, int *iostatus);
    int ResetEX(const char *raddr, int mask, int *exstatus);
    int GetWifi(const char *raddr, wifi_config_data *config);
    int SetWifi(const char *raddr, wifi_config_data *config);

    int m_verbose;

protected:
    const char *http_post;
    const char *http_get;

    const char *url_get_iostatus;
    const char *url_set_iostatus;
    const char *url_set_ioconfig;
    const char *url_get_exstatus;
    const char *url_set_exstatus;
    const char *url_set_exconfig;
    const char *url_get_config;
    const char *url_get_ioconfig;
    const char *url_get_exconfig;
    const char *url_get_wifi;
    const char *url_set_wifi;

    int IO2Int(const char* str);
    int EXP2Int(const char* str);
    int HttpRespCode(const char* http);

};

#endif /* _DOM32IOWIFI_H_ */