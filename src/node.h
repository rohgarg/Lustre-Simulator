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
#ifndef NODE_H
#define NODE_H

#include <nic.h>
#include <processor.h>

/**
	@author yingjin.qian <yingjin.qian@sun.com>
*/
class Node : public NIC
{
protected:
	fun_t handler;
public:
	Node();
	~Node();

	fun_t GetHandler();
	void SetHandler(fun_t h);
	virtual void Recv(Message *msg);
};

class Server : public Node
{
protected:
	int algo;
	int thnr;
	Scheduler *nrs;
	Processor cpu;

	virtual void Enqueue(Message *msg);
	virtual obd_count GetNrsPendingNum() { return nrs->Size(); }
	static int TaskCompare(void *a, void *b);

public:
	Server();
	~Server();

	static void Handle(void *arg);
	virtual void SetScheduler(int algo);
	virtual void Recv(Message *msg);
	virtual void Start();
};

#if 0
class MDT : public Server
{
public:
	MDT();
	~MDT();

	void Recv(Message *msg);
	void Start();
};
#endif
#endif
