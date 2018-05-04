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
#include "node.h"
#include "nrsfifo.h"
#include "nrsrbtree.h"
#include "nrsepoch.h"

Node::Node()
 : NIC()
{
	handler = NULL;
}


Node::~Node()
{
}

void Node::SetHandler(fun_t h)
{
	handler = h;
}

fun_t Node::GetHandler()
{
	return handler;
}

void Node::Recv(Message *msg)
{
	/*Print(NOW"%s RECV msg@%d.\n",
		now, name, msg->GetId());*/
	msg->Notify();
}

Server::Server()
 : Node()
{
	thnr = 1;
	algo = 0;
	nrs = NULL;
}

Server::~Server()
{
	if (nrs)
		delete nrs;
}

void Server::Enqueue(Message *msg)
{
	cpu.AddNewTask(msg);
}

void Server::Recv(Message *msg)
{
	msg->SetRecvTime(now);

	if (msg->phase == 1) /* New RPC. */
		Enqueue(msg);
	else /* Wake up the waiting thread */
		msg->Notify();
}

int Server::TaskCompare(void *a, void *b)
{
	return (((Message *)a)->GetKey() > ((Message *)b)->GetKey() ? 1 : -1);
}

void Server::SetScheduler(int a)
{
	if (nrs)
		delete nrs;

	algo = a ? : NRS_ALGO_FIFO;
	if (algo == NRS_ALGO_FIFO)
		nrs = new NrsFifo;
	else if (algo == NRS_ALGO_RBTREE)
		nrs = new NrsRbtree(TaskCompare);
	else if (algo == NRS_ALGO_EPOCH) {
		nrs = new NrsEpoch;
	}
}

void Server::Start()
{
	Node::Start();

	SetScheduler(algo);

	assert(nrs != NULL && thnr != 0);
	cpu.Attach(this, thnr, nrs);
	cpu.Start();
}

