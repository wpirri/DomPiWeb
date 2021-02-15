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

STRFunc::STRFunc()
{

}

STRFunc::~STRFunc()
{

}

/*  Separa en campos segun el separador y devuelve el número de campo solicitado
    en count. El primer campo es el 1.
    Si count e 0 devuelve lo que hay antes del separador
    El dato devuelto se termina al encntrar un separador o cualquier caracter que no se número o letra
*/
int STRFunc::Section(char *in, char sep, unsigned int count, char *out)
{
    char *p;
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

int STRFunc::ParseData(char *buffer, const char *label, char *value)
{
    char *p;
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

