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
#include "qtcp.h"

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

QTcp::QTcp()
{
    m_socket = NULL;

}

QTcp::~QTcp()
{
    if(m_socket) delete m_socket;
}

int QTcp::Query(const char* raddr, const char* snd, char* rcv, int rcv_max_len)
{
    int rc = (-1); 

    if(m_socket) delete m_socket;
    m_socket = NULL;

    m_socket = new CTcp();
    if( !m_socket) return rc;

    rc--;
    if(m_socket->Connect(NULL, raddr, 80) != 0)
    {
        delete m_socket;
        m_socket = NULL;
        return rc;
    }

    rc--;
    if(m_socket->Send(snd, strlen(snd)) == 0)
    {
        rc = m_socket->Receive(rcv, rcv_max_len, 3000);
    }

    delete m_socket;
    m_socket = NULL;

    return rc;
}
