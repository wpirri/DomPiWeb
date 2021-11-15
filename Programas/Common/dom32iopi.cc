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
//#include <wiringSerial.h>

#include "gpiopin.h"

#define GPIO_STATUS_LED_PIN 24


Dom32IoPi::Dom32IoPi()
{
    wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers
    pinMode(gpio_pin[GPIO_STATUS_LED_PIN], OUTPUT);
}

Dom32IoPi::~Dom32IoPi()
{

}

int Dom32IoPi::GetIOStatus(const char *raddr, int *iostatus)
{
    return (-1);
}

int Dom32IoPi::GetEXStatus(const char *raddr, int *exstatus)
{
    return (-1);
}

int Dom32IoPi::GetConfig(const char *raddr, int *ioconfig, int *exconfig)
{
    return (-1);
}

int Dom32IoPi::ConfigIO(const char *raddr, int ioconfig, int *config)
{
    return (-1);
}

int Dom32IoPi::ConfigEX(const char *raddr, int exconfig, int *config)
{
    return (-1);
}

int Dom32IoPi::SetIO(const char *raddr, int mask, int *iostatus)
{
    return (-1);
}

int Dom32IoPi::SetEX(const char *raddr, int mask, int *exstatus)
{
    return (-1);
}

int Dom32IoPi::ResetIO(const char *raddr, int mask, int *iostatus)
{
    return (-1);
}

int Dom32IoPi::ResetEX(const char *raddr, int mask, int *exstatus)
{
    return (-1);
}

int Dom32IoPi::SwitchIO(const char *raddr, int mask, int *iostatus)
{
    return (-1);
}

int Dom32IoPi::SwitchEX(const char *raddr, int mask, int *exstatus)
{
    return (-1);
}

int Dom32IoPi::PulseIO(const char *raddr, int mask, int sec, int *iostatus)
{
    return (-1);
}

int Dom32IoPi::PulseEX(const char *raddr, int mask, int sec, int *exstatus)
{
    return (-1);
}

void Dom32IoPi::SetStatusLed(int status)
{
    digitalWrite(gpio_pin[GPIO_STATUS_LED_PIN], (status)?HIGH:LOW); // Turn LED ON/OFF
}
