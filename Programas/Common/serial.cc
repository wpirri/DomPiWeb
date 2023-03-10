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
/*
https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/
*/
#include "serial.h"

#include <string>
#include <iostream>
#include <csignal>
#include <cstdarg>
using namespace std;

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <syslog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <gmonitor/glog.h>

/*
extern CGLog* pLog;
*/

CSerial::CSerial()
{
  m_sfd = -1;
}

CSerial::~CSerial()
{
  Close();
}

int CSerial::Open(int port)
{
  char dev_name[256];

  if(port < 1 || port > 4) return -1;
  /* aca hay que poner #IFDEF por SO */
  sprintf(dev_name, "/dev/ttyS%i", port - 1);

  return Open(dev_name);
}

int CSerial::Open(const char* port)
{
  struct termios settings;

  Close();

  /* abro el puerto */
  m_sfd = open( port, O_RDWR|O_NOCTTY | O_NONBLOCK);
  if( m_sfd == (-1) ) return (-1);
  /* me guardo la configuracion que tiene asi se la devuelvo antes de cerrarlo */
  tcgetattr(m_sfd, &m_old_settings);
  /* seteo la nueva configuración */
  memcpy(&settings, &m_old_settings, sizeof(struct termios));
  settings.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
  settings.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
  settings.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
  settings.c_cflag |= CS8; // 8 bits per byte (most common)
  settings.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
  settings.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  settings.c_lflag &= ~ICANON;
  settings.c_lflag &= ~ECHO; // Disable echo
  settings.c_lflag &= ~ECHOE; // Disable erasure
  settings.c_lflag &= ~ECHONL; // Disable new-line echo
  settings.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
  settings.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
  settings.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

  settings.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  settings.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  // settings.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
  // settings.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

  /*
    VMIN = 0, VTIME = 0: No blocking, return immediately with what is available
    VMIN > 0, VTIME = 0: This will make read() always wait for bytes 
                         (exactly how many is determined by VMIN), so read()
                         could block indefinitely.
    VMIN = 0, VTIME > 0: This is a blocking read of any number of chars with a 
                         maximum timeout (given by VTIME). read() will block 
                         until either any amount of data is available, or the 
                         timeout occurs. This happens to be my favourite mode 
                         (and the one I use the most).
    VMIN > 0, VTIME > 0: Block until either VMIN characters have been received, 
                         or VTIME after first character has elapsed. Note that 
                         the timeout for VTIME does not begin until the first 
                         character is received.
  */
  settings.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
  settings.c_cc[VMIN] = 0;

  // Set in/out baud rate to be 9600
  cfsetispeed(&settings, B115200);
  cfsetospeed(&settings, B115200);

  tcflush(m_sfd, TCIFLUSH);
  tcsetattr(m_sfd, TCSANOW, &settings);

  return 0;
}

void CSerial::Close()
{
  if(m_sfd == (-1)) return;
  tcsetattr(m_sfd, TCSANOW, &m_old_settings);
  close(m_sfd);
  m_sfd = (-1);
}

int CSerial::Send(const char* fmt, ...)
{
  va_list arg;
  char tx_buffer[4096];

  if(m_sfd == (-1)) return 0;

  va_start(arg, fmt);
  vsprintf(tx_buffer, fmt, arg);
  va_end(arg);

  if(write(m_sfd, tx_buffer, (int)strlen(tx_buffer)) != (int)strlen(tx_buffer))
  {
    return 0;
  }
  return strlen(tx_buffer);
}

int CSerial::Recv(char* data, unsigned int max_data_len, int time_out)
{
  time_t t, max_t;
  char ch;
  int i;

  if(m_sfd == (-1)) return 0;

  t = time(&t);
  i = 0;
  max_t = t + time_out + 1;

  while(i < (int)max_data_len && t <= max_t )
  {
    if(read(m_sfd, &ch, 1) > 0)
    {
      if(ch == 0x0D || ch == 0x0A) break;
      *(data + i) = ch;
      i++;
    }
    t = time(&t);
    usleep(250);
  }
  *(data + i) = 0;
  return i;
}

