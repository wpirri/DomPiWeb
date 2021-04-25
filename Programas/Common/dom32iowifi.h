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
#ifndef _DOM32IOWIFI_H_
#define _DOM32IOWIFI_H_

class Dom32IoWifi
{
public:
	Dom32IoWifi();
	virtual ~Dom32IoWifi();

    int GetIOStatus(const char *raddr, int *iostatus);

protected:
    char *http_post;
    char *http_get;

    const char *url_get_iostatus;
    const char *url_set_iostatus;
    const char *url_get_exstatus;
    const char *url_set_exstatus;

};

#endif /* _DOM32IOWIFI_H_ */