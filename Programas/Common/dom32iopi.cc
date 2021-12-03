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
#include "dom32iopi.h"

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

#include "strfunc.h"

#include <wiringPi.h> // Include WiringPi library!
#include <wiringSerial.h>

#include "gpiopin.h"

Dom32IoPi::Dom32IoPi()
{
    wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers

    pinMode(gpio_pin[GPIO_IO1], INPUT);
    pinMode(gpio_pin[GPIO_IO2], INPUT);
    pinMode(gpio_pin[GPIO_IO3], INPUT);
    pinMode(gpio_pin[GPIO_IO4], INPUT);
    pinMode(gpio_pin[GPIO_IO5], INPUT);
    pinMode(gpio_pin[GPIO_IO6], INPUT);
    pinMode(gpio_pin[GPIO_IO7], INPUT);
    pinMode(gpio_pin[GPIO_IO8], INPUT);
    pinMode(gpio_pin[GPIO_EX1], OUTPUT);
    pinMode(gpio_pin[GPIO_EX2], OUTPUT);
    pinMode(gpio_pin[GPIO_EX3], OUTPUT);
    pinMode(gpio_pin[GPIO_EX4], OUTPUT);
    pinMode(gpio_pin[GPIO_EX5], OUTPUT);
    pinMode(gpio_pin[GPIO_EX6], OUTPUT);
    pinMode(gpio_pin[GPIO_EX7], OUTPUT);
    pinMode(gpio_pin[GPIO_EX8], OUTPUT);

    pinMode(gpio_pin[GPIO_POWER_5V], INPUT);    /* OUTPUT - OC */
    pinMode(gpio_pin[GPIO_STATUS_LED], OUTPUT);

    pinMode(gpio_pin[GPIO_TX_MODEM_RX], OUTPUT);
    pinMode(gpio_pin[GPIO_RX_MODEM_TX], INPUT);
    pinMode(gpio_pin[GPIO_MODEM_POWER_SET], OUTPUT);
    pinMode(gpio_pin[GPIO_MODEM_POWER_GET], INPUT);
    pinMode(gpio_pin[GPIO_MODEM_RESET], INPUT);    /* OUTPUT - OC */
    pinMode(gpio_pin[GPIO_MODEM_RING ], INPUT);
    pinMode(gpio_pin[GPIO_MODEM_PWRKEY], INPUT);    /* OUTPUT - OC */

    m_sfd = (-1);

}

Dom32IoPi::~Dom32IoPi()
{

}

int Dom32IoPi::GetIOStatus(int *iostatus)
{
    *iostatus = 0;

    if(digitalRead(gpio_pin[GPIO_IO1])>0) (*iostatus) += 0x01;
    if(digitalRead(gpio_pin[GPIO_IO2])>0) (*iostatus) += 0x02;
    if(digitalRead(gpio_pin[GPIO_IO3])>0) (*iostatus) += 0x04;
    if(digitalRead(gpio_pin[GPIO_IO4])>0) (*iostatus) += 0x08;
    if(digitalRead(gpio_pin[GPIO_IO5])>0) (*iostatus) += 0x10;
    if(digitalRead(gpio_pin[GPIO_IO6])>0) (*iostatus) += 0x20;
    if(digitalRead(gpio_pin[GPIO_IO7])>0) (*iostatus) += 0x40;
    if(digitalRead(gpio_pin[GPIO_IO8])>0) (*iostatus) += 0x80;
    return (*iostatus);
}

int Dom32IoPi::GetEXStatus(int *exstatus)
{
    *exstatus = 0;

    if(digitalRead(gpio_pin[GPIO_EX1])>0) (*exstatus) += 0x01;
    if(digitalRead(gpio_pin[GPIO_EX2])>0) (*exstatus) += 0x02;
    if(digitalRead(gpio_pin[GPIO_EX3])>0) (*exstatus) += 0x04;
    if(digitalRead(gpio_pin[GPIO_EX4])>0) (*exstatus) += 0x08;
    if(digitalRead(gpio_pin[GPIO_EX5])>0) (*exstatus) += 0x10;
    if(digitalRead(gpio_pin[GPIO_EX6])>0) (*exstatus) += 0x20;
    if(digitalRead(gpio_pin[GPIO_EX7])>0) (*exstatus) += 0x40;
    if(digitalRead(gpio_pin[GPIO_EX8])>0) (*exstatus) += 0x80;
    return (*exstatus);
}

int Dom32IoPi::GetConfig(int */*ioconfig*/, int */*exconfig*/)
{
    return (-1);
}

int Dom32IoPi::ConfigIO(int ioconfig, int *config)
{
    (*config) = ioconfig & 0xff;
    pinMode(gpio_pin[GPIO_IO1], (ioconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_IO2], (ioconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_IO3], (ioconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_IO4], (ioconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_IO5], (ioconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_IO6], (ioconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_IO7], (ioconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_IO8], (ioconfig & 0x01)?INPUT:OUTPUT);
    return (*config);
}

int Dom32IoPi::ConfigEX(int exconfig, int *config)
{
    (*config) = exconfig & 0xff;
    pinMode(gpio_pin[GPIO_EX1], (exconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_EX2], (exconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_EX3], (exconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_EX4], (exconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_EX5], (exconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_EX6], (exconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_EX7], (exconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_EX8], (exconfig & 0x01)?INPUT:OUTPUT);
    return (*config);
}

int Dom32IoPi::SetIO(int mask, int *iostatus)
{
    if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_IO1], HIGH); }
    if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_IO2], HIGH); }
    if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_IO3], HIGH); }
    if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_IO4], HIGH); }
    if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_IO5], HIGH); }
    if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_IO6], HIGH); }
    if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_IO7], HIGH); }
    if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_IO8], HIGH); }
    return GetIOStatus(iostatus);
}

int Dom32IoPi::SetEX(int mask, int *exstatus)
{
    if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_EX1], HIGH); }
    if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_EX2], HIGH); }
    if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_EX3], HIGH); }
    if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_EX4], HIGH); }
    if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_EX5], HIGH); }
    if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_EX6], HIGH); }
    if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_EX7], HIGH); }
    if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_EX8], HIGH); }
    return GetEXStatus(exstatus);
}

int Dom32IoPi::ResetIO(int mask, int *iostatus)
{
    if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_IO1], LOW); }
    if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_IO2], LOW); }
    if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_IO3], LOW); }
    if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_IO4], LOW); }
    if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_IO5], LOW); }
    if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_IO6], LOW); }
    if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_IO7], LOW); }
    if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_IO8], LOW); }
    return GetIOStatus(iostatus);
}

int Dom32IoPi::ResetEX(int mask, int *exstatus)
{
    if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_EX1], LOW); }
    if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_EX2], LOW); }
    if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_EX3], LOW); }
    if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_EX4], LOW); }
    if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_EX5], LOW); }
    if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_EX6], LOW); }
    if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_EX7], LOW); }
    if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_EX8], LOW); }
    return GetEXStatus(exstatus);
}

int Dom32IoPi::SwitchIO(int mask, int *iostatus)
{
    (*iostatus) = GetIOStatus(iostatus);
    if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_IO1], ((*iostatus)&0x01)?LOW:HIGH); }
    if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_IO2], ((*iostatus)&0x02)?LOW:HIGH); }
    if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_IO3], ((*iostatus)&0x04)?LOW:HIGH); }
    if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_IO4], ((*iostatus)&0x08)?LOW:HIGH); }
    if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_IO5], ((*iostatus)&0x01)?LOW:HIGH); }
    if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_IO6], ((*iostatus)&0x02)?LOW:HIGH); }
    if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_IO7], ((*iostatus)&0x04)?LOW:HIGH); }
    if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_IO8], ((*iostatus)&0x08)?LOW:HIGH); }
    return GetIOStatus(iostatus);
}

int Dom32IoPi::SwitchEX(int mask, int *exstatus)
{
    (*exstatus) = GetIOStatus(exstatus);
    if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_EX1], ((*exstatus)&0x01)?LOW:HIGH); }
    if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_EX2], ((*exstatus)&0x02)?LOW:HIGH); }
    if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_EX3], ((*exstatus)&0x04)?LOW:HIGH); }
    if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_EX4], ((*exstatus)&0x08)?LOW:HIGH); }
    if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_EX5], ((*exstatus)&0x01)?LOW:HIGH); }
    if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_EX6], ((*exstatus)&0x02)?LOW:HIGH); }
    if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_EX7], ((*exstatus)&0x04)?LOW:HIGH); }
    if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_EX8], ((*exstatus)&0x08)?LOW:HIGH); }
    return GetIOStatus(exstatus);
}

int Dom32IoPi::PulseIO(int /*mask*/, int /*sec*/, int * /*iostatus*/)
{
    return (-1);
}

int Dom32IoPi::PulseEX(int /*mask*/, int /*sec*/, int * /*exstatus*/)
{
    return (-1);
}

void Dom32IoPi::SetStatusLed(int status)
{
    digitalWrite(gpio_pin[GPIO_STATUS_LED], (status)?HIGH:LOW); // Turn LED ON/OFF
}

void Dom32IoPi::InitSerial(int baud)
{
    if(m_sfd >= 0)
    {
        serialClose(m_sfd);
    }
    m_sfd = serialOpen("/dev/ttyAMA0", baud);
}

void Dom32IoPi::TaskSerial(void)
{

}

void Dom32IoPi::TimerSerial(void)
{

}

void Dom32IoPi::SendSerial(const char* /*cmd*/, const char* /*wait*/, int /*on_ok*/, int /*on_error*/, int /*time_out*/)
{

}

