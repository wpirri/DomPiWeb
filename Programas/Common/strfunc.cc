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
#include "strfunc.h"

#include <string>
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
using namespace std;

#include <unistd.h>
#include <sys/msg.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

STRFunc::STRFunc()
{

}

STRFunc::~STRFunc()
{

}

int STRFunc::Section(const char *in, char sep, unsigned int count, char *out)
{
    const char *p;
    int len = 0;
    
    p = in;
    while(count && *p)
    {
        if(*p == sep) count--;
        p++;
    }
    if(*p == 0 || count) return 0;
    /* p apunta al inicio del campo, copio hasta el final */
    while(*p && ( (*p >= '0' && *p <= '9') || 
                  (*p >= 'a' && *p <= 'z') || 
                  (*p >= 'A' && *p <= 'Z')  ) && *p != sep )
    {
        *out = *p;
        p++;
        out++;
        len++;
    }
    *out = 0;
    return len;
}

int STRFunc::ParseData(const char *buffer, const char *label, char *value)
{
    const char *p;
    int len = 0;
    
    p = buffer;
    do
    {
        p = strstr(p, label);
        if(p)
        {
            p += strlen(label);
            if(*p == '=')
            {
                p++;
                while(*p && *p != '&')
                {
                    *value = *p;
                    p++;
                    value++;
                    len++;
                }
                *value = 0;
            }
        }
    } while(p);
    
    return len;
}

int STRFunc::ParseDataIdx(const char *buffer, char *label, char *value, int idx)
{
    const char *p;
    int found = 0;
    
    p = buffer;
    /* Avanzo hasta el indice */
    while(*p && idx)
    {
        if(*p == '&') idx--;
        p++;
    }
    /* Copio el label */
    while(*p && *p != '=')
    {
        *label = *p;
        label++;
        p++;
    }
    *label = 0;
    if(*p == '=')
    {
        p++;
        /* copio el value */
        while(*p && *p != '&')
        {
            *value = *p;
            value++;
            p++;
        }
        found = 1;
    }
    *value = 0;
    
    return found;
}

int STRFunc::EscapeHttp(const char* in, char* out)
{
    int len = 0;

    if(!in || !out) return (-1);
    while(*in)
    {
        if(*in == '%')
        {
            in++;
            *out = (unsigned char)StrHex2Int(in);
            in++;
        }
        else if(in != out) /* Por si se usa el mismo buffer de entradad y salida */
        {
            *out = *in;
        }
        in++;
        out++;
        len++;
    }
    *out = 0;
    return len;
}

int STRFunc::StrHex2Int(const char *str_hex)
{
    int rc = 0;

    if(*str_hex >= '0' && *str_hex <= '9')
    {
        rc += (*str_hex)-'0';    
    }
    else if(*str_hex >= 'A' && *str_hex <= 'F')
    {
        rc += (*str_hex)-'A'+10;    
    }
    rc *= 16;
    str_hex++;
    if(*str_hex >= '0' && *str_hex <= '9')
    {
        rc += (*str_hex)-'0';    
    }
    else if(*str_hex >= 'A' && *str_hex <= 'F')
    {
        rc += (*str_hex)-'A'+10;    
    }
    return rc;
}

int STRFunc::ParseCommand(const char *buffer, char *comando, char *objeto, char *parametro)
{
   
    if(!buffer) return (-1);
    if(!comando) return (-1);

    while(*buffer && *buffer != ' ' && *buffer != '\r' && *buffer != '\n')
    {
        *comando = *buffer;
        comando++;
        buffer++;
    }
    *comando = 0;

    while(*buffer && (*buffer == ' ' || *buffer == '\r' || *buffer == '\n')) buffer++;

    while(*buffer && *buffer != ',' && *buffer != '\r' && *buffer != '\n')
    {
        *objeto = *buffer;
        objeto++;
        buffer++;
    }
    *objeto = 0;

    while(*buffer && (*buffer == ' ' || *buffer == ',' || *buffer == '\r' || *buffer == '\n')) buffer++;

    while(*buffer && *buffer != '\r' && *buffer != '\n')
    {
        *parametro = *buffer;
        parametro++;
        buffer++;
    }
    *parametro = 0;

    return 0;
}

void STRFunc::ToUpper(const char* in, char* out)
{
    while(*in)
    {
        *out = toupper(*in);
        in++;
        out++;
    }
    *out = 0;
}

void STRFunc::ToLower(const char* in, char* out)
{
    while(*in)
    {
        *out = tolower(*in);
        in++;
        out++;
    }
    *out = 0;
}

/*
    Convierte "Feb 14 2023 19:48:17" en unix time
    
*/
unsigned long STRFunc::Fecha2Timestamp(const char* fecha)
{
    struct tm stm;
    char s[8];

    /* Mes */
    s[0] = *fecha++;
    s[1] = *fecha++;
    s[2] = *fecha++;
    s[3] = 0;
    if( !strcmp(s, "Ene")) { stm.tm_mon = 0; }
    else if( !strcmp(s, "Feb")) { stm.tm_mon = 1; }
    else if( !strcmp(s, "Mar")) { stm.tm_mon = 2; }
    else if( !strcmp(s, "Abr")) { stm.tm_mon = 3; }
    else if( !strcmp(s, "May")) { stm.tm_mon = 4; }
    else if( !strcmp(s, "Jun")) { stm.tm_mon = 5; }
    else if( !strcmp(s, "Jul")) { stm.tm_mon = 6; }
    else if( !strcmp(s, "Ago")) { stm.tm_mon = 7; }
    else if( !strcmp(s, "Sep")) { stm.tm_mon = 8; }
    else if( !strcmp(s, "Oct")) { stm.tm_mon = 9; }
    else if( !strcmp(s, "Nov")) { stm.tm_mon = 10; }
    else if( !strcmp(s, "Dic")) { stm.tm_mon = 11; }

    /* Día */
    fecha++;
    if(*fecha == ' ')
    {
        fecha++; 
        s[0] = *fecha++;
    }
    else
    {
        s[0] = *fecha++;
        s[1] = *fecha++;
        s[2] = 0;
    }
    stm.tm_mday = atoi(s);

    /*Año*/
    fecha++;
    s[0] = *fecha++;
    s[1] = *fecha++;
    s[2] = *fecha++;
    s[3] = *fecha++;
    s[4] = 0;
    stm.tm_year = atoi(s) - 1900;

    /*Hora*/
    fecha++;
    s[0] = *fecha++;
    s[1] = *fecha++;
    s[2] = 0;
    stm.tm_hour = atoi(s);
    /*Minuto*/
    fecha++;
    s[0] = *fecha++;
    s[1] = *fecha++;
    s[2] = 0;
    stm.tm_min = atoi(s);
    /*Segundo*/
    fecha++;
    s[0] = *fecha++;
    s[1] = *fecha++;
    s[2] = 0;
    stm.tm_sec = atoi(s);

    return mktime(&stm);
}