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
#ifndef MESSAGE_H
#define MESSAGE_H

#include <event.h>

/**
	@author yingjin.qian <yingjin.qian@sun.com>
*/

#define NET_PKT_TX 1
#define NET_PKT_RX 2
#define NET_PKT_BLK 4
#define NET_PKT_TXDONE 8
#define NET_PKT_RXDONE 16
#define NET_PKT_RETX 16
#define NET_PKT_MSG 32 /* Last packet of the message */

class Packet: public Event
{
public:
	int id;
	int state;
	obd_count size;
	int src;
	int dst;
	void *data;
public:
	Packet() 
			: Event(), data(NULL), id(0), size(0), state(0)
		{ src = dst = -1; }
	Packet(int s, int d)
			: Event(), data(NULL), id(0), size(0), state(0)
		{ src = s; dst = d;}
	~Packet() {}	
};

/* Message or operation Type */
#define MSG_NONE  0
#define MSG_PING   1
#define MSG_LOCK  2
#define MSG_READ  3
#define MSG_WRITE 4

/* For network banchmark */
#define MSG_LATENCY    5
#define MSG_BANDWIDTH 6
#define MSG_NICPERF     7
#define MSG_READ_NET  8  /* Network Read */
#define MSG_WRITE_NET 9  /* Network Write */
#define MSG_NETWORK 10

#define MSG_OPEN 11

class Message : public Packet
{
	static uint32_t num;
	
	int id;
	int type;
	int flags;
	int slice; /* No of packet */
	Thread *worker[2];
	Thread *waiter;

public:
	unsigned int r:1;
	int nt:1;
	int phase;
	obd_count left;
	obd_count msz;
	obd_count key;

	IO io;
	void *req;

	int sid; /* Server ID */
	int cid; /* Client ID */

public:
 	Message();
	Message(int t, int s, int d);
 	~Message();

	void Init(int t, int s, int d);
	void SetLength(obd_count len);
	void SetIO(obd_id id, int cmd, obd_off off, obd_count count);
	void SetRecvTime(cfs_time_t t);
	void SetWorker(Thread *t);
	cfs_time_t GetRecvTime();
	int GetType();
	int GetId();
	obd_count GetIOCount();
	obd_count GetKey();
	Packet *GetPacket();

	void ReverseChannel();
	void Notify();
};

#endif
