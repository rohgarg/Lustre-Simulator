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
#include "event.h"
#include "nrsrbtree.h"
#include "nrsbinheap.h"

//Event

cfs_time_t Event::now;
cfs_time_t Event::maxRunTicks = ~0;
Scheduler *Event::runQ = new NrsRbtree(RunTimeCompare);
int Event::algo = ALGO_RBTREE;
FILE *Event::printer = stdout;
Event *Event::current;

Event::Event()
{
	runTime = 0;
	arg = NULL;
	fn = NULL;
	run = 0;
	memset(&rbnode, 0, sizeof(rbnode));
	rbnode.rb_entry = this;
}


Event::~Event()
{}

void Event::InitEvent(fun_t f, void *data)
{
	fn = f;
	arg = data;
}

void Event::InsertRunQ()
{
	assert(!run);

	//printf(NOW"Insert event %p runTime %llu.\n", now, this, runTime);
	runQ->Enqueue(this);
	run = 1;
}

void Event::RemoveFromRunQ()
{
	assert(run);
	runQ->Erase(this);
	run = 0;
}

void Event::Run()
{
	current = this;
	fn(arg);
}

void Event::RunAfter(cfs_duration_t ticks)
{
	assert(!run);
	runTime = now + ticks;
	InsertRunQ();
}

void Event::RunAt(cfs_time_t time)
{
	assert(!run && (time > now));
	runTime = time;
	InsertRunQ();
}

Event *Event::NextRunEvent()
{
	Event *ev;

	ev = (Event *)runQ->Dequeue();
	if (ev)
		ev->run = 0;

	return ev;
}

void Event::Schedule()
{
	Event *e;

	now = 0;
	srand((int)time(0));

	while (e = NextRunEvent()) {
		/*if (now > e->runTime)
			printf(NOW"Event %p runTime %llu.\n", now, e, e->runTime);*/
		assert(now <= e->runTime);
		now = e->runTime;
		if (now == 502235345)
			printf("Hit the time!\n");
		e->Run();
		if (now > maxRunTicks) {
			printf("Reached max run ticks %llu (%llus).\n", 
				maxRunTicks, maxRunTicks / params.TimeUnit);
			break;
		}
	}
	
	while (NextRunEvent() != NULL);
		
	printf("Timer is stopped.\n");
}

cfs_time_t Event::Clock()
{
	return now;
}

void Event::SetRunTicks(cfs_duration_t ticks)
{
	maxRunTicks = ticks ? : maxRunTicks;
}

/*rb_node *Event::GetRbnode(void *e)
{
	return &(((Event *)e)->rbnode);
}*/

int Event::RunTimeCompare(void *a, void *b)
{
	Event *e1, *e2;
	
	e1 = (Event *) a;
	e2 = (Event *) b;
	return (((Event *)a)->runTime > ((Event *)b)->runTime) ? 1: -1;
}

int Event::BinheapRunTimeCompare(void *a, void *b)
{
	return (((Event *)b)->runTime >= ((Event *)a)->runTime);
}

void Event::SetAlgorithm(int a)
{
	if (algo > 0)
		delete runQ;

	algo = a;
	if (algo == ALGO_RBTREE)
		runQ = new NrsRbtree(RunTimeCompare);
	else if (algo == ALGO_BINHEAP)
		runQ = new NrsBinheap(BinheapRunTimeCompare);
}

cfs_duration_t Event::Rand(cfs_duration_t max)
{
	return params.debug.Rand ?
		(1 + (cfs_duration_t) (max * rand() / (RAND_MAX + 1.0))) : 0;
}

int Event::SignRand(cfs_duration_t max)
{
	return params.debug.Rand ?
		(rand() % (2 * max + 1) - max) : 0;
}

void Event::DumpEvents()
{
	Event *e;

	while (e = NextRunEvent()) {
		printf(NOW"run event %p.\n", e->runTime);
	}
}

/**************************************************************************
 * class Thread.                                                          *
 **************************************************************************/
#include "processor.h"
uint32_t Thread::num;

Thread::Thread()
{
	pid = num++;
}

Thread::~Thread()
{}

void Thread::CreateThread(fun_t f, void *data)
{
	InitEvent(f, data);
}

void Thread::Signal()
{
	assert(!run);
	runTime = now + Rand(params.thread.CtxSwitchTicks);
	InsertRunQ();
}

void Thread::Sleep(cfs_duration_t timeout)
{
	assert(!run);
	RunAfter(timeout);
}

int Thread::GetPid()
{
	return pid;
}

void Thread::PushContext(int state, fun_t f)
{
	ThreadLocalData *tld = (ThreadLocalData *)arg;
	
	assert((tld != NULL) && (tld->flags & TLD_FROM_POOL) &&
		(tld->deep == 0));
	tld->ctx.f = tld->f;
	tld->ctx.state = tld->state;
	tld->f = f;
	tld->state = state;
	tld->deep++;
}

void Thread::PopContext()
{
	ThreadLocalData *tld = (ThreadLocalData *)arg;
	
	assert((tld != NULL) && (tld->flags & TLD_FROM_POOL) &&
		(tld->deep == 1));
	tld->f = tld->ctx.f;
	tld->state = tld->ctx.state;
	tld->deep--;
}

void Thread::WakeupAll(List *waitq)
{
	Thread *t;
	
	while (!waitq->Empty()) {
		t = (Thread *)(waitq->suc);
		t->Remove();
		t->Signal();
	}
}

ThreadLocalData *Thread::GetTLD()
{
	ThreadLocalData *tld = (ThreadLocalData *)arg;
	
	assert((tld != NULL) && (tld->flags & TLD_FROM_POOL));

	return tld;
}

Thread *Thread::Current()
{
	return (Thread *)current;
}

ThreadLocalData *Thread::CurrentTLD()
{
	return Current()->GetTLD();
}

void Thread::PushCurCtxt(int state, fun_t f)
{
	return Current()->PushContext(state, f);
}

void Thread::PopCurCtxt()
{
	return Current()->PopContext();
}