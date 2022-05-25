/***************************************************************************
    Copyright (C) 2013   Walter Pirri

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

#ifndef _QUEUE_H_
#define	_QUEUE_H_

#include <stdbool.h>

#define MAX_QUEUE_COUNT 128
#define INVALID_QUEUE   MAX_QUEUE_COUNT

void QueueInit( void );
unsigned char QueueOpen(unsigned char elements, unsigned int len, void* buffer);
void QueueClose(unsigned char id);
bool QueueAdd(unsigned char id, void* data);
bool QueueView(unsigned char id, void** data);
bool QueueGet(unsigned char id, void* data);
bool QueueDel(unsigned char id);
unsigned char QueueCount(unsigned char id);

#endif	/* _QUEUE_H_ */

