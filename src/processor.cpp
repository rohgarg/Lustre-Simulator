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
#include "processor.h"
#include "node.h"

Processor::Processor()
    : Thread()
{
	num = 0;
	nrs = NULL;
	idle = 0;
	tlds = NULL;
	poolState = pool_StartState;
}

Processor::~Processor()
{
	if (tlds) {
		while (NextIdleThreadFromPool() != NULL);
		for (int i = 0; i < num; i++)
			delete tlds[i].t;
		delete [] tlds;
	}
}

Thread *Processor::NextIdleThreadFromPool()
{
	Thread *t;
	
	if (idleQ.Empty())
			return NULL;
	
	t = (Thread *)(idleQ.suc);
	t->Remove();
	idle--;
	if (idle == 0)
		poolState = pool_FullLoadState;
	return t;
}

int Processor::AddNewTask(Message *msg)
{
	if (nrs->Enqueue(msg)) {
		printf(NOW "Enqueue message failed.\n", now);
		exit(1);
	}
	
	if (poolState <= pool_IdleState) {
		if (poolState == pool_IdleState)
			Signal();
		poolState = pool_RunState;
	}
	return 0;
}

int Processor::TaskCompletion(ThreadLocalData *tld)
{
	tld->f = NULL;
	tld->m = NULL;
	tld->v = NULL;
	tld->state = 0;
	tld->t->Insert(&idleQ);
	idle++;
	if (nrs && (poolState == pool_FullLoadState)) {
		Signal();
		poolState = pool_RunState;
		//PoolStateMachine();
	}
}

void Processor::ProcessOneTask(void *arg)
{
	ThreadLocalData *tld = (ThreadLocalData *)arg;
	
	tld->f(tld);
}

void Processor::PoolStateMachine()
{
	switch (poolState) {
	case pool_RunState: {
		Message *msg;
		Thread *t;
		ThreadLocalData *tld; 
		
		/* Initialize the working thread to handle
		 * various tasks and start it in the next tick. */
		msg = (Message *)nrs->Dequeue();
		if (msg == NULL) {
			poolState = pool_IdleState;
			return;
		}
		
		t = NextIdleThreadFromPool();
		assert(t != NULL);
		tld = t->GetTLD();
		tld->m = msg;
		//dev->InitThreadLocalData(tld);
		tld->f = site->GetHandler();
		t->Run();
		/* Check for left queued tasks. */
		PoolStateMachine();
		break;
	}
	case pool_StartState:
		poolState = pool_IdleState;
	case pool_IdleState:
	case pool_FullLoadState:
		break;
	}
}

void Processor::ThreadPoolManger(void *arg)
{
	Processor *p = (Processor *) arg;
	
	p->PoolStateMachine();
}

void Processor::Attach(Node *n, int threads, Scheduler *s)
{
	site = n;
	num = threads;
	nrs = s;
}

ThreadLocalData *Processor::GetTLDFromPool(int tid)
{
	assert(num >= tid);
	return &tlds[tid];
}

void Processor::RunThreadPool()
{
	Thread *t;
	
	for (int i = 0; i < num; i++) {
		t = tlds[i].t;
		t->Remove();
		t->RunAfter(1);
		idle--;
		if (idle == 0)
			poolState = pool_FullLoadState;
	}
}

int Processor::InitThreadPool()
{
	/* Create @num threads for the thread pool. */
	assert(num > 0);
	tlds = new ThreadLocalData [num];
	for (int i = 0; i < num; i++) {
		Thread *t;
		ThreadLocalData *tld;

		t = new Thread;
		tld = &tlds[i];
		memset(tld, 0, sizeof(*tld));
		tld->m = NULL;
		tld->p = this;
		tld->n = site;
		tld->t = t;
		tld->id = i;
		tld->flags = TLD_FROM_POOL;

		t->CreateThread(ProcessOneTask, tld);
		t->Insert(&idleQ);
		if (site)
			printf(NOW "Start TID@%d PID@%d for %s\n", 
				now, i, t->GetPid(), site->GetDeviceName());
	}
	idle = num;
}

void Processor::Start()
{
	InitThreadPool();
	CreateThread(ThreadPoolManger, this);
	RunAfter(1);
}
