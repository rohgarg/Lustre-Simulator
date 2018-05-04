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
#include "mdt.h"
int MDT::num;
mdtvec_t MDT::set;

MDT::MDT()
 : Server()
{
	id = num++;
	set.push_back(this);
	__dbg = params.debug.MDT;
	sprintf(name, "MDT%d", id);
	thnr = params.cluster.MdtThreadCount;
}


MDT::~MDT()
{
}

void MDT::OpenStateMachine(ThreadLocalData *tld)
{
	Processor *p = tld->p;
	Message *msg = tld->m;
	FileObject *obj = (FileObject *)msg->req;
	Thread *t = tld->t;

	switch (tld->state) {
	case mdt_OpenFileState:
		/* It will take 5ms to open/create a new file. Very simple... */
		t->RunAfter(params.io.OpenTicks + SignRand(params.io.OpenTicks / 10)); 
		tld->state = mdt_OpenCompleteState;
		break;
	case mdt_OpenCompleteState:
		Print(NOW "%s Finished OPEN for OBJ@%llu.\n",
			now, name, obj->GetId());
		msg->SetLength(1024);
		Send(msg);
		p->TaskCompletion(tld);
		break;
	}
}

void MDT::Handle(void *arg)
{
	ThreadLocalData *tld = (ThreadLocalData *) arg;
	MDT *mdt = (MDT *) tld->n;
	Message *msg = tld->m;
	int type = msg->GetType();

	/*if (msg->phase == 1)
		ost->StatNrs(msg, -1);
	*/

	switch(type) {
	case MSG_OPEN:
		mdt->OpenStateMachine(tld);
		break;
	}
}

int MDT::GetCount()
{
	return num;
}

int MDT::GetNid(int i)
{
	return set[i]->nid;
}

void MDT::Start()
{
	Print(NOW"%s is starting...\n", now, name);
	__stat = 1;
	Server::Start();
	handler = Handle;
}
