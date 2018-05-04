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
#include "message.h"

uint32_t Message::num;

Message::Message()
 : Packet()
{
	id  = num++;
	Init(MSG_NONE, -1, -1);
}

Message::Message(int t, int s, int d)
	: Packet(s, d)
{
	Init(t, s, d);
}

Message::~Message()
{}

void Message::Init(int t, int s, int d)
{
	type = t;
	src = s;
	dst = d;
	r = 0;
	nt = 0;
	phase = 0;
	left = 0;
	slice = 0;
	waiter = NULL;
	worker[0] = worker[1] = NULL;

	memset(&io, 0, sizeof(io));
	io.data = this;

	req = NULL;
	key = 0;
	flags = 0;
	sid = cid = 0;
}

void Message::SetLength(obd_count len)
{
	msz = left = len;
}

void Message::SetRecvTime(cfs_time_t t)
{
	runTime = t;
	if (params.nrs.ByArrivalTime)
		key = t;
}

cfs_time_t Message::GetRecvTime()
{
	return runTime;
}

obd_count Message::GetKey()
{
	return key;
}

int Message::GetType()
{
	return type;
}

int Message::GetId()
{
	return id;
}

Packet *Message::GetPacket()
{
	Packet *pkt;
	
	pkt = new Packet;
	if (r) {
		pkt->src = dst;
		pkt->dst = src;
	} else {
		pkt->src = src;
		pkt->dst = dst;
	}
	pkt->size = min(params.network.PacketSize, left);
	left -= pkt->size;
	if (left == 0) {
		pkt->state |= NET_PKT_MSG;
	}
	pkt->data = this;
	pkt->id = slice++;

	return pkt;
}

void Message::SetIO(obd_id id, int cmd, obd_off offset, obd_count len)
{
	io.fid = id;
	io.off = offset;
	io.count = len;
	io.cmd = cmd;
}

obd_count Message::GetIOCount()
{
	return io.count;
}

void Message::SetWorker(Thread *t)
{
	worker[r] = t;
}

void Message::ReverseChannel()
{
	r = !r;
}

void Message::Notify()
{
	worker[r]->Signal();
}
