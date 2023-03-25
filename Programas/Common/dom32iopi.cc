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

    m_pi_config_io_file_data[GPIO_A01] = INPUT;
    m_pi_config_io_file_data[GPIO_A02] = INPUT;
    m_pi_config_io_file_data[GPIO_A03] = INPUT;
    m_pi_config_io_file_data[GPIO_A04] = INPUT;
    m_pi_config_io_file_data[GPIO_A05] = INPUT;
    m_pi_config_io_file_data[GPIO_A06] = INPUT;
    m_pi_config_io_file_data[GPIO_A07] = INPUT;
    m_pi_config_io_file_data[GPIO_A08] = INPUT;
    m_pi_config_io_file_data[GPIO_B01] = OUTPUT;
    m_pi_config_io_file_data[GPIO_B02] = OUTPUT;
    m_pi_config_io_file_data[GPIO_B03] = OUTPUT;
    m_pi_config_io_file_data[GPIO_B04] = OUTPUT;
    m_pi_config_io_file_data[GPIO_B05] = OUTPUT;
    m_pi_config_io_file_data[GPIO_B06] = OUTPUT;
    m_pi_config_io_file_data[GPIO_B07] = OUTPUT;
    m_pi_config_io_file_data[GPIO_B08] = OUTPUT;
    m_pi_config_io_file_data[GPIO_C01] = OUTPUT;
    m_pi_config_io_file_data[GPIO_C02] = OUTPUT;
    m_pi_config_io_file_data[GPIO_C03] = OUTPUT;
    m_pi_config_io_file_data[GPIO_C04] = OUTPUT;
    m_pi_config_io_file_data[GPIO_C05] = OUTPUT;
    m_pi_config_io_file_data[GPIO_C06] = OUTPUT;
    m_pi_config_io_file_data[GPIO_C07] = OUTPUT;
    m_pi_config_io_file_data[GPIO_C08] = OUTPUT;
    m_pi_config_io_file_data[GPIO_STATUS_LED] = OUTPUT;
    m_pi_config_io_file_data[GPIO_MODE_LED] = OUTPUT;
    m_pi_config_io_file_data[GPIO_TX] = OUTPUT;
    m_pi_config_io_file_data[GPIO_RX] = INPUT;

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
    pinMode(gpio_pin[GPIO_A01], m_pi_config_io_file_data[GPIO_A01]);
    pinMode(gpio_pin[GPIO_A02], m_pi_config_io_file_data[GPIO_A02]);
    pinMode(gpio_pin[GPIO_A03], m_pi_config_io_file_data[GPIO_A03]);
    pinMode(gpio_pin[GPIO_A04], m_pi_config_io_file_data[GPIO_A04]);
    pinMode(gpio_pin[GPIO_A05], m_pi_config_io_file_data[GPIO_A05]);
    pinMode(gpio_pin[GPIO_A06], m_pi_config_io_file_data[GPIO_A06]);
    pinMode(gpio_pin[GPIO_A07], m_pi_config_io_file_data[GPIO_A07]);
    pinMode(gpio_pin[GPIO_A08], m_pi_config_io_file_data[GPIO_A08]);
    pinMode(gpio_pin[GPIO_B01], m_pi_config_io_file_data[GPIO_B01]);
    pinMode(gpio_pin[GPIO_B02], m_pi_config_io_file_data[GPIO_B02]);
    pinMode(gpio_pin[GPIO_B03], m_pi_config_io_file_data[GPIO_B03]);
    pinMode(gpio_pin[GPIO_B04], m_pi_config_io_file_data[GPIO_B04]);
    pinMode(gpio_pin[GPIO_B05], m_pi_config_io_file_data[GPIO_B05]);
    pinMode(gpio_pin[GPIO_B06], m_pi_config_io_file_data[GPIO_B06]);
    pinMode(gpio_pin[GPIO_B07], m_pi_config_io_file_data[GPIO_B07]);
    pinMode(gpio_pin[GPIO_B08], m_pi_config_io_file_data[GPIO_B08]);
    pinMode(gpio_pin[GPIO_C01], m_pi_config_io_file_data[GPIO_C01]);
    pinMode(gpio_pin[GPIO_C02], m_pi_config_io_file_data[GPIO_C02]);
    pinMode(gpio_pin[GPIO_C03], m_pi_config_io_file_data[GPIO_C03]);
    pinMode(gpio_pin[GPIO_C04], m_pi_config_io_file_data[GPIO_C04]);
    pinMode(gpio_pin[GPIO_C05], m_pi_config_io_file_data[GPIO_C05]);
    pinMode(gpio_pin[GPIO_C06], m_pi_config_io_file_data[GPIO_C06]);
    pinMode(gpio_pin[GPIO_C07], m_pi_config_io_file_data[GPIO_C07]);
    pinMode(gpio_pin[GPIO_C08], m_pi_config_io_file_data[GPIO_C08]);
    pinMode(gpio_pin[GPIO_MODE_LED], m_pi_config_io_file_data[GPIO_MODE_LED]);
    pinMode(gpio_pin[GPIO_STATUS_LED], m_pi_config_io_file_data[GPIO_STATUS_LED]);
    pinMode(gpio_pin[GPIO_TX], m_pi_config_io_file_data[GPIO_TX]);
    pinMode(gpio_pin[GPIO_RX], m_pi_config_io_file_data[GPIO_RX]);
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

int Dom32IoPi::GetIOStatus(int port, int *iostatus)
{
    *iostatus = 0;

    switch (port)
    {
        case 1:
            if(digitalRead(gpio_pin[GPIO_A01])>0) (*iostatus) += 0x01;
            if(digitalRead(gpio_pin[GPIO_A02])>0) (*iostatus) += 0x02;
            if(digitalRead(gpio_pin[GPIO_A03])>0) (*iostatus) += 0x04;
            if(digitalRead(gpio_pin[GPIO_A04])>0) (*iostatus) += 0x08;
            if(digitalRead(gpio_pin[GPIO_A05])>0) (*iostatus) += 0x10;
            if(digitalRead(gpio_pin[GPIO_A06])>0) (*iostatus) += 0x20;
            if(digitalRead(gpio_pin[GPIO_A07])>0) (*iostatus) += 0x40;
            if(digitalRead(gpio_pin[GPIO_A08])>0) (*iostatus) += 0x80;
            break;
        case 2:
            if(digitalRead(gpio_pin[GPIO_B01])>0) (*iostatus) += 0x01;
            if(digitalRead(gpio_pin[GPIO_B02])>0) (*iostatus) += 0x02;
            if(digitalRead(gpio_pin[GPIO_B03])>0) (*iostatus) += 0x04;
            if(digitalRead(gpio_pin[GPIO_B04])>0) (*iostatus) += 0x08;
            if(digitalRead(gpio_pin[GPIO_B05])>0) (*iostatus) += 0x10;
            if(digitalRead(gpio_pin[GPIO_B06])>0) (*iostatus) += 0x20;
            if(digitalRead(gpio_pin[GPIO_B07])>0) (*iostatus) += 0x40;
            if(digitalRead(gpio_pin[GPIO_B08])>0) (*iostatus) += 0x80;
            break;
        case 3:
            if(digitalRead(gpio_pin[GPIO_C01])>0) (*iostatus) += 0x01;
            if(digitalRead(gpio_pin[GPIO_C02])>0) (*iostatus) += 0x02;
            if(digitalRead(gpio_pin[GPIO_C03])>0) (*iostatus) += 0x04;
            if(digitalRead(gpio_pin[GPIO_C04])>0) (*iostatus) += 0x08;
            if(digitalRead(gpio_pin[GPIO_C05])>0) (*iostatus) += 0x10;
            if(digitalRead(gpio_pin[GPIO_C06])>0) (*iostatus) += 0x20;
            if(digitalRead(gpio_pin[GPIO_C07])>0) (*iostatus) += 0x40;
            if(digitalRead(gpio_pin[GPIO_C08])>0) (*iostatus) += 0x80;
            break;
    }
    return (*iostatus);
}

int Dom32IoPi::GetConfig(int /*port*/, int */*ioconfig*/)
{
    return (-1);
}

int Dom32IoPi::ConfigIO(int port, int ioconfig, int *config)
{
    (*config) = ioconfig & 0xff;

    switch(port)
    {
        case 1:
            pinMode(gpio_pin[GPIO_A01], (ioconfig & 0x01)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_A02], (ioconfig & 0x02)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_A03], (ioconfig & 0x04)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_A04], (ioconfig & 0x08)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_A05], (ioconfig & 0x10)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_A06], (ioconfig & 0x20)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_A07], (ioconfig & 0x40)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_A08], (ioconfig & 0x80)?INPUT:OUTPUT);

            m_pi_config_io_file_data[GPIO_A01] = (ioconfig & 0x01)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_A02] = (ioconfig & 0x02)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_A03] = (ioconfig & 0x04)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_A04] = (ioconfig & 0x08)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_A05] = (ioconfig & 0x10)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_A06] = (ioconfig & 0x20)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_A07] = (ioconfig & 0x40)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_A08] = (ioconfig & 0x80)?INPUT:OUTPUT;
            break;
        case 2:
            pinMode(gpio_pin[GPIO_B01], (ioconfig & 0x01)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_B02], (ioconfig & 0x02)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_B03], (ioconfig & 0x04)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_B04], (ioconfig & 0x08)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_B05], (ioconfig & 0x10)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_B06], (ioconfig & 0x20)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_B07], (ioconfig & 0x40)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_B08], (ioconfig & 0x80)?INPUT:OUTPUT);

            m_pi_config_io_file_data[GPIO_B01] = (ioconfig & 0x01)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_B02] = (ioconfig & 0x02)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_B03] = (ioconfig & 0x04)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_B04] = (ioconfig & 0x08)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_B05] = (ioconfig & 0x10)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_B06] = (ioconfig & 0x20)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_B07] = (ioconfig & 0x40)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_B08] = (ioconfig & 0x80)?INPUT:OUTPUT;
            break;
        case 3:
            pinMode(gpio_pin[GPIO_C01], (ioconfig & 0x01)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_C02], (ioconfig & 0x02)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_C03], (ioconfig & 0x04)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_C04], (ioconfig & 0x08)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_C05], (ioconfig & 0x10)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_C06], (ioconfig & 0x20)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_C07], (ioconfig & 0x40)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_C08], (ioconfig & 0x80)?INPUT:OUTPUT);

            m_pi_config_io_file_data[GPIO_C01] = (ioconfig & 0x01)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_C02] = (ioconfig & 0x02)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_C03] = (ioconfig & 0x04)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_C04] = (ioconfig & 0x08)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_C05] = (ioconfig & 0x10)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_C06] = (ioconfig & 0x20)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_C07] = (ioconfig & 0x40)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_C08] = (ioconfig & 0x80)?INPUT:OUTPUT;
            break;
    }

    SaveConfig();

    return (*config);
}

int Dom32IoPi::SetIO(int mask, int port, int *iostatus)
{
    SWITCH(port)
    {
        case 1:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_A01], HIGH); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_A02], HIGH); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_A03], HIGH); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_A04], HIGH); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_A05], HIGH); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_A06], HIGH); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_A07], HIGH); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_A08], HIGH); }
            break;
        case 2:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_B01], HIGH); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_B02], HIGH); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_B03], HIGH); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_B04], HIGH); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_B05], HIGH); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_B06], HIGH); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_B07], HIGH); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_B08], HIGH); }
            break;
        case 3:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_C01], HIGH); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_C02], HIGH); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_C03], HIGH); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_C04], HIGH); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_C05], HIGH); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_C06], HIGH); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_C07], HIGH); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_C08], HIGH); }
            break;
    }

    return GetIOStatus(port, iostatus);
}

int Dom32IoPi::ResetIO(int mask, int port, int *iostatus)
{
    switch(port)
    {
        case 1:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_A01], LOW); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_A02], LOW); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_A03], LOW); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_A04], LOW); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_A05], LOW); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_A06], LOW); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_A07], LOW); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_A08], LOW); }
            break;
        case 2:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_B01], LOW); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_B02], LOW); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_B03], LOW); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_B04], LOW); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_B05], LOW); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_B06], LOW); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_B07], LOW); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_B08], LOW); }
            break;
        case 3:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_C01], LOW); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_C02], LOW); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_C03], LOW); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_C04], LOW); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_C05], LOW); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_C06], LOW); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_C07], LOW); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_C08], LOW); }
            break;
    }

    return GetIOStatus(port, iostatus);
}

int Dom32IoPi::SwitchIO(int mask, int *iostatus)
{
    (*iostatus) = GetIOStatus(port, iostatus);

    switch(port)
    {
        case 1:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_A01], ((*iostatus)&0x01)?LOW:HIGH); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_A02], ((*iostatus)&0x02)?LOW:HIGH); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_A03], ((*iostatus)&0x04)?LOW:HIGH); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_A04], ((*iostatus)&0x08)?LOW:HIGH); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_A05], ((*iostatus)&0x01)?LOW:HIGH); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_A06], ((*iostatus)&0x02)?LOW:HIGH); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_A07], ((*iostatus)&0x04)?LOW:HIGH); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_A08], ((*iostatus)&0x08)?LOW:HIGH); }
            break;
        case 2:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_B01], ((*iostatus)&0x01)?LOW:HIGH); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_B02], ((*iostatus)&0x02)?LOW:HIGH); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_B03], ((*iostatus)&0x04)?LOW:HIGH); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_B04], ((*iostatus)&0x08)?LOW:HIGH); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_B05], ((*iostatus)&0x01)?LOW:HIGH); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_B06], ((*iostatus)&0x02)?LOW:HIGH); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_B07], ((*iostatus)&0x04)?LOW:HIGH); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_B08], ((*iostatus)&0x08)?LOW:HIGH); }
            break;
        case 3:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_C01], ((*iostatus)&0x01)?LOW:HIGH); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_C02], ((*iostatus)&0x02)?LOW:HIGH); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_C03], ((*iostatus)&0x04)?LOW:HIGH); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_C04], ((*iostatus)&0x08)?LOW:HIGH); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_C05], ((*iostatus)&0x01)?LOW:HIGH); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_C06], ((*iostatus)&0x02)?LOW:HIGH); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_C07], ((*iostatus)&0x04)?LOW:HIGH); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_C08], ((*iostatus)&0x08)?LOW:HIGH); }
            break;
    }

    return GetIOStatus(iostatus);
}

int Dom32IoPi::PulseIO(int /*mask*/, int /*sec*/, int * /*iostatus*/)
{
    return (-1);
}

void Dom32IoPi::SetStatusLed(int status)
{
    digitalWrite(gpio_pin[GPIO_STATUS_LED], (status)?HIGH:LOW); // Turn LED ON/OFF
}
