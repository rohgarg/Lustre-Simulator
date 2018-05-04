/***************************************************************************
 *   Copyright (C) 2008 by qian                                            *
 *   yingjin.qian@sun.com                                                  *
 *                                                                         *
 *   This file is part of Lustre, http://www.lustre.org                    *
 *                                                                         *
 *   Lustre is free software; you can redistribute it and/or               *
 *   modify it under the terms of version 2 of the GNU General Public      *
 *   License as published by the Free Software Foundation.                 *
 *                                                                         *
 *   Lustre is distributed in the hope that it will be useful,             *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with Lustre; if not, write to the Free Software                 *
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.             *
 ***************************************************************************/
#include "osd.h"

OSD::OSD()
 : OBD()
{
	__dbg = params.debug.OSD;
}


OSD::~OSD()
{}

int OSD::Read(IO *io)
{
	fs.Read(io);
	return 0;
}

int OSD::Write(IO *io)
{
	fs.Write(io);
	return 0;
}

void OSD::Setup()
{
	Print(NOW"%s is starting...\n", now, name);
	
	fs.Attach((Server *)site);
	fs.Start();
}
