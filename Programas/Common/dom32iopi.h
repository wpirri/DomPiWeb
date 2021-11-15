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
#ifndef _DOM32IOPI_H_
#define _DOM32IOPI_H_

class Dom32IoPi
{
public:
    typedef struct
    {
        char ap1[33];
        char ap1_pass[33];
        char ap2[33];
        char ap2_pass[33];
        char host1[33];
        char host2[33];
        unsigned int host1_port;
        unsigned int host2_port;
        char rqst_path[33];
    } pi_config_data;

	Dom32IoPi();
	virtual ~Dom32IoPi();

    int GetIOStatus(const char *raddr, int *iostatus);
    int GetEXStatus(const char *raddr, int *exstatus);
    int GetConfig(const char *raddr, int *ioconfig, int *exconfig);
    int ConfigIO(const char *raddr, int ioconfig, int *config);
    int ConfigEX(const char *raddr, int exconfig, int *config);
    int SetIO(const char *raddr, int mask, int *iostatus);
    int SetEX(const char *raddr, int mask, int *exstatus);
    int ResetIO(const char *raddr, int mask, int *iostatus);
    int ResetEX(const char *raddr, int mask, int *exstatus);
    int SwitchIO(const char *raddr, int mask, int *iostatus);
    int SwitchEX(const char *raddr, int mask, int *exstatus);
    int PulseIO(const char *raddr, int mask, int sec, int *iostatus);
    int PulseEX(const char *raddr, int mask, int sec, int *exstatus);

    void SetStatusLed(int status);

protected:


};

#endif /* _DOM32IOPI_H_ */