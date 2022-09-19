/***************************************************************************
    Copyright (C) 2007   Walter Pirri

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
#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <string>
using namespace std;

#include <termios.h>

class CSerial
{
public:
  CSerial();
  virtual ~CSerial();

  int Open(int port);
  int Open(const char* port);
  void Close();

  int Send(const char* fmt, ...);
  int Recv(char* data, unsigned int max_data_len, int time_out);

protected:
  int m_sfd;

private:
  struct termios m_old_settings;

};
#endif /* _SERIAL_H_ */
