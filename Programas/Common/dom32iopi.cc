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

#define DOMPI_IO_CONFIG "/var/gmonitor/dompi_io.config"

Dom32IoPi::Dom32IoPi()
{
    wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers
    m_sfd = (-1);
}

Dom32IoPi::~Dom32IoPi()
{

}

void Dom32IoPi::SetDefaultConfig()
{
    memset(m_pi_config_io_file_data, 0, sizeof(m_pi_config_io_file_data));

    m_pi_config_io_file_data[GPIO_IO1] = INPUT;
    m_pi_config_io_file_data[GPIO_IO2] = INPUT;
    m_pi_config_io_file_data[GPIO_IO3] = INPUT;
    m_pi_config_io_file_data[GPIO_IO4] = INPUT;
    m_pi_config_io_file_data[GPIO_IO5] = INPUT;
    m_pi_config_io_file_data[GPIO_IO6] = INPUT;
    m_pi_config_io_file_data[GPIO_IO7] = INPUT;
    m_pi_config_io_file_data[GPIO_IO8] = INPUT;
    m_pi_config_io_file_data[GPIO_EX1] = OUTPUT;
    m_pi_config_io_file_data[GPIO_EX2] = OUTPUT;
    m_pi_config_io_file_data[GPIO_EX3] = OUTPUT;
    m_pi_config_io_file_data[GPIO_EX4] = OUTPUT;
    m_pi_config_io_file_data[GPIO_EX5] = OUTPUT;
    m_pi_config_io_file_data[GPIO_EX6] = OUTPUT;
    m_pi_config_io_file_data[GPIO_EX7] = OUTPUT;
    m_pi_config_io_file_data[GPIO_EX8] = OUTPUT;
    m_pi_config_io_file_data[GPIO_POWER_5V] = OUTPUT;    /* OUTPUT OC - Se emula pasando a input al HIGH*/
    m_pi_config_io_file_data[GPIO_STATUS_LED] = OUTPUT;
    m_pi_config_io_file_data[GPIO_TX_MODEM_RX] = OUTPUT;
    m_pi_config_io_file_data[GPIO_RX_MODEM_TX] = INPUT;
    m_pi_config_io_file_data[GPIO_MODEM_POWER_SET] = OUTPUT;
    m_pi_config_io_file_data[GPIO_MODEM_POWER_GET] = INPUT;
    m_pi_config_io_file_data[GPIO_MODEM_RESET] = OUTPUT;    /* OUTPUT - OC */
    m_pi_config_io_file_data[GPIO_MODEM_RING ] = INPUT;
    m_pi_config_io_file_data[GPIO_MODEM_PWRKEY] = OUTPUT;    /* OUTPUT - OC */

}

void Dom32IoPi::LoadConfig( void )
{
    FILE *fd;

    fd = fopen(DOMPI_IO_CONFIG, "r");
    if(fd)
    {
        if(fread(&m_pi_config_io_file_data, sizeof(m_pi_config_io_file_data), 1, fd))
        {
            fclose(fd);
        }
        else
        {
            fclose(fd);
            SetDefaultConfig();
            SaveConfig();
        }
    }
    else
    {
        SetDefaultConfig();
        SaveConfig();
    }
    pinMode(gpio_pin[GPIO_IO1], m_pi_config_io_file_data[GPIO_IO1]);
    pinMode(gpio_pin[GPIO_IO2], m_pi_config_io_file_data[GPIO_IO2]);
    pinMode(gpio_pin[GPIO_IO3], m_pi_config_io_file_data[GPIO_IO3]);
    pinMode(gpio_pin[GPIO_IO4], m_pi_config_io_file_data[GPIO_IO4]);
    pinMode(gpio_pin[GPIO_IO5], m_pi_config_io_file_data[GPIO_IO5]);
    pinMode(gpio_pin[GPIO_IO6], m_pi_config_io_file_data[GPIO_IO6]);
    pinMode(gpio_pin[GPIO_IO7], m_pi_config_io_file_data[GPIO_IO7]);
    pinMode(gpio_pin[GPIO_IO8], m_pi_config_io_file_data[GPIO_IO8]);
    pinMode(gpio_pin[GPIO_EX1], m_pi_config_io_file_data[GPIO_EX1]);
    pinMode(gpio_pin[GPIO_EX2], m_pi_config_io_file_data[GPIO_EX2]);
    pinMode(gpio_pin[GPIO_EX3], m_pi_config_io_file_data[GPIO_EX3]);
    pinMode(gpio_pin[GPIO_EX4], m_pi_config_io_file_data[GPIO_EX4]);
    pinMode(gpio_pin[GPIO_EX5], m_pi_config_io_file_data[GPIO_EX5]);
    pinMode(gpio_pin[GPIO_EX6], m_pi_config_io_file_data[GPIO_EX6]);
    pinMode(gpio_pin[GPIO_EX7], m_pi_config_io_file_data[GPIO_EX7]);
    pinMode(gpio_pin[GPIO_EX8], m_pi_config_io_file_data[GPIO_EX8]);
    pinMode(gpio_pin[GPIO_POWER_5V], m_pi_config_io_file_data[GPIO_POWER_5V]);    /* OUTPUT - OC */
    pinMode(gpio_pin[GPIO_STATUS_LED], m_pi_config_io_file_data[GPIO_STATUS_LED]);
    pinMode(gpio_pin[GPIO_TX_MODEM_RX], m_pi_config_io_file_data[GPIO_TX_MODEM_RX]);
    pinMode(gpio_pin[GPIO_RX_MODEM_TX], m_pi_config_io_file_data[GPIO_RX_MODEM_TX]);
    pinMode(gpio_pin[GPIO_MODEM_POWER_SET], m_pi_config_io_file_data[GPIO_MODEM_POWER_SET]);
    pinMode(gpio_pin[GPIO_MODEM_POWER_GET], m_pi_config_io_file_data[GPIO_MODEM_POWER_GET]);
    pinMode(gpio_pin[GPIO_MODEM_RESET], m_pi_config_io_file_data[GPIO_MODEM_RESET]);    /* OUTPUT - OC */
    pinMode(gpio_pin[GPIO_MODEM_RING ], m_pi_config_io_file_data[GPIO_MODEM_RING]);
    pinMode(gpio_pin[GPIO_MODEM_PWRKEY], m_pi_config_io_file_data[GPIO_MODEM_PWRKEY]);    /* OUTPUT - OC */
}

void Dom32IoPi::SaveConfig( void )
{
    FILE *fd;

    fd = fopen(DOMPI_IO_CONFIG, "w");
    if(fd)
    {
        fwrite(&m_pi_config_io_file_data, sizeof(m_pi_config_io_file_data), 1, fd);
        fclose(fd);
    }
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
    pinMode(gpio_pin[GPIO_IO2], (ioconfig & 0x02)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_IO3], (ioconfig & 0x04)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_IO4], (ioconfig & 0x08)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_IO5], (ioconfig & 0x10)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_IO6], (ioconfig & 0x20)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_IO7], (ioconfig & 0x40)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_IO8], (ioconfig & 0x80)?INPUT:OUTPUT);

    m_pi_config_io_file_data[GPIO_IO1] = (ioconfig & 0x01)?INPUT:OUTPUT;
    m_pi_config_io_file_data[GPIO_IO2] = (ioconfig & 0x02)?INPUT:OUTPUT;
    m_pi_config_io_file_data[GPIO_IO3] = (ioconfig & 0x04)?INPUT:OUTPUT;
    m_pi_config_io_file_data[GPIO_IO4] = (ioconfig & 0x08)?INPUT:OUTPUT;
    m_pi_config_io_file_data[GPIO_IO5] = (ioconfig & 0x10)?INPUT:OUTPUT;
    m_pi_config_io_file_data[GPIO_IO6] = (ioconfig & 0x20)?INPUT:OUTPUT;
    m_pi_config_io_file_data[GPIO_IO7] = (ioconfig & 0x40)?INPUT:OUTPUT;
    m_pi_config_io_file_data[GPIO_IO8] = (ioconfig & 0x80)?INPUT:OUTPUT;

    SaveConfig();

    return (*config);
}

int Dom32IoPi::ConfigEX(int exconfig, int *config)
{
    (*config) = exconfig & 0xff;
    pinMode(gpio_pin[GPIO_EX1], (exconfig & 0x01)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_EX2], (exconfig & 0x02)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_EX3], (exconfig & 0x04)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_EX4], (exconfig & 0x08)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_EX5], (exconfig & 0x10)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_EX6], (exconfig & 0x20)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_EX7], (exconfig & 0x40)?INPUT:OUTPUT);
    pinMode(gpio_pin[GPIO_EX8], (exconfig & 0x80)?INPUT:OUTPUT);

    m_pi_config_io_file_data[GPIO_EX1] = (exconfig & 0x01)?INPUT:OUTPUT;
    m_pi_config_io_file_data[GPIO_EX2] = (exconfig & 0x02)?INPUT:OUTPUT;
    m_pi_config_io_file_data[GPIO_EX3] = (exconfig & 0x04)?INPUT:OUTPUT;
    m_pi_config_io_file_data[GPIO_EX4] = (exconfig & 0x08)?INPUT:OUTPUT;
    m_pi_config_io_file_data[GPIO_EX5] = (exconfig & 0x10)?INPUT:OUTPUT;
    m_pi_config_io_file_data[GPIO_EX6] = (exconfig & 0x20)?INPUT:OUTPUT;
    m_pi_config_io_file_data[GPIO_EX7] = (exconfig & 0x40)?INPUT:OUTPUT;
    m_pi_config_io_file_data[GPIO_EX8] = (exconfig & 0x80)?INPUT:OUTPUT;

    SaveConfig();

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
