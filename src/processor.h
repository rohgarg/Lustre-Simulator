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
#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <event.h>

/**
	@author yingjin.qian <yingjin.qian@sun.com>
*/

/* CPU, Thread pool Model,
 * It can be used by both Client and Server.*/
#define TLD_FROM_POOL	1
class Message;
class Node;
class Processor;

struct Context {
	fun_t f;
	int state;
};

struct ThreadLocalData {
	int flags;
	Node *n;
	Processor *p;
	Thread *t;
	Message *m;
	/* Backup context */
	Context ctx;
	int state;
	fun_t f;
	void *v;
	int deep;
	int id;
};

class Processor : public Thread
{
	Scheduler *nrs;
	List idleQ; /* List of idle threads */
	int idle;  /* number of idle threads. */
	int num; /* number of threads in the pool */
	Node *site;
	ThreadLocalData *tlds;
	enum {
		pool_StartState,
		pool_IdleState,
		pool_RunState,
		pool_FullLoadState,
		pool_LastState,
	} poolState;
	
	void PoolStateMachine();
	static void ThreadPoolManger(void *arg);
	static void ProcessOneTask(void *arg);
	Thread *NextIdleThreadFromPool();
public:
	Processor();
	~Processor();

	void Start();
	void Attach(Node *site, int thnr, Scheduler *nrs);
	int AddNewTask(Message *msg);
	int TaskCompletion(ThreadLocalData *tld);
	int InitThreadPool();
	ThreadLocalData *GetTLDFromPool(int tid);
	void RunThreadPool();
};

#endif
