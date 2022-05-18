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

    void LoadConfig( void );

    int GetIOStatus(int *iostatus);
    int GetEXStatus(int *exstatus);
    int GetConfig(int *ioconfig, int *exconfig);
    int ConfigIO(int ioconfig, int *config);
    int ConfigEX(int exconfig, int *config);
    int SetIO(int mask, int *iostatus);
    int SetEX(int mask, int *exstatus);
    int ResetIO(int mask, int *iostatus);
    int ResetEX(int mask, int *exstatus);
    int SwitchIO(int mask, int *iostatus);
    int SwitchEX(int mask, int *exstatus);
    int PulseIO(int mask, int sec, int *iostatus);
    int PulseEX(int mask, int sec, int *exstatus);

    void SetStatusLed(int status);

    void InitSerial(int baud);
    void TaskSerial(void);
    void TimerSerial(void);
    void SendSerial(const char* cmd, const char* wait, int on_ok, int on_error, int time_out);
    int ModemPower(int OnOff, int wait);
    int ModemPowerCheck( void );

protected:
    void SetDefaultConfig( void );
    void SaveConfig( void );

    int m_sfd;
    char m_pi_config_io_file_data[256]; 

};

#endif /* _DOM32IOPI_H_ */