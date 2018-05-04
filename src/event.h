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
#ifndef EVENT_H
#define EVENT_H

#include <lustre.h>
#include <rbtree.h>
#include <scheduler.h>
#include <string.h>
/**
	@author yingjin.qian <yingjin.qian@sun.com>
*/

struct List
{
	List *suc;
	List *pred;
	List()
		{	suc = pred = this;}
	~List()
		{ assert(suc == this && pred == this);}

	int Empty()
		{
			return (suc == this);
		}

	void Insert(List *other)
	 	{
			suc = other;
			pred = other->pred;
			suc->pred = pred->suc = this;
		}
		
	void InsertTail(List *head)
		{
			suc = head->suc;
			pred = head;
			head->suc = head->suc->pred = this;
			
		}
	void Remove()
	 	{
			suc->pred = pred; 
			pred->suc = suc; 
			suc = pred = this;
		}
		
	int  Size()
		{
			int n = 0; 
			for (List *l = suc; l != this; l = l->suc)
				n++;
			return (n);
		}

	int  Contains(List *other)
		{
			for (List *l = suc; l != this; l = l->suc)
				if (l == other)
					return (1); 
			return (0);
		}
};

typedef cfs_time_t order_t;
#define LAST_ORDER (order_t(0x7fffffff))

struct OrderList : public List
{
	order_t *order;
 	OrderList()
		{order = NULL;}
		
	OrderList(order_t *o)
		{order = o;}
		
	void InsertInorder (OrderList *other)
		{
			OrderList *o = (OrderList *)other->suc;
			while (o != other && (*order) >= (*o->order))
				o = (OrderList *)o->suc;
	  		Insert (o);
		}
		
   order_t NextOrder()
	 	{return (Empty() ? LAST_ORDER : (*((OrderList *)suc)->order));}
};

#define ALGO_FIFO 1
#define ALGO_ORDERLIST 2
#define ALGO_BINHEAP 3
#define ALGO_RBTREE 4

typedef void (*fun_t)(void *);
class Event
{
	rb_node rbnode; /* It must be the first member. */

	static int algo;
	static Scheduler *runQ;

	static Event *NextRunEvent();
	static int RunTimeCompare(void *a, void *b);
	static int BinheapRunTimeCompare(void *a, void *b);
protected:
	int run;
	void *arg;
	fun_t fn;
	cfs_time_t runTime;

	static cfs_time_t now;
	static cfs_time_t maxRunTicks;
	static FILE *printer;

	static Event *current;

	void InsertRunQ();
	void RemoveFromRunQ();

	//static rb_node *GetRbnode(void *e);
public:
	Event();

	~Event();
	
	void InitEvent(fun_t f, void *data);
	void Run();
	void RunAfter(cfs_duration_t ticks);
	void RunAt(cfs_time_t	time);

	static cfs_time_t Clock();
	static void Schedule();
	static void SetAlgorithm(int a);
	static void SetRunTicks(cfs_duration_t ticks);
	static cfs_duration_t Rand(cfs_duration_t max);
	static int SignRand(cfs_duration_t max);
	static void DumpEvents();
};

class ThreadLocalData;
class Thread: public Event, public OrderList
{
	int pid;

	static uint32_t num;
public:
	Thread();
	~Thread();
	
	void CreateThread(fun_t f, void *data);
	ThreadLocalData *GetTLD();
	void PushContext(int state, fun_t f);
	void PopContext();
	void Signal();
	void Sleep(cfs_duration_t to);
	void Suspend();
	int GetPid();
	static Thread *Current();
	static ThreadLocalData *CurrentTLD();
	static void PushCurCtxt(int state, fun_t f);
	static void PopCurCtxt(); 
	static void WakeupAll(List *waitq);
};

struct Layout {
	obd_count stripe_size;
	uint32_t stripe_count;
	uint32_t stripe_pattern;
	int StripeAlign(obd_off off)
		{
			return !(off & (stripe_size - 1));
		}
	int Check(obd_count count)
		{
			return stripe_size % count == 0 || count % stripe_size == 0;
		}
};

class Object
{
	obd_id id;

public:
	Thread *waiter;
public:
	Object() { id = 0;}
	~Object() {}
	obd_id GetId() { return id; }
	void SetId(obd_id oid) { id = oid; }
};

class ChunkObject : public Object
{
public:
	obd_id ost_idx;
	obd_id ost_gen;
	obd_count kms;

public:
	ChunkObject()
		: Object()
		{
			ost_idx = ost_gen = 0;
			kms = 0;
		}
	~ChunkObject()
		{}
};

#define FILE_HAS_LAYOUT 1

class FileObject : public Object
{
public:
	int flags;
	obd_id gr;
	ChunkObject *cobj; /* children objects */
	Layout layout;

public:
	FileObject() : Object()
		{
			flags = 0;
			gr = 0;
			cobj = NULL;
			waiter = NULL;
			memset(&layout, 0, sizeof(layout));
		}
	~FileObject() {}
	Layout *GetLayout()
		{
			return &layout;
		}
	void SetLayout(Layout *lyt)
		{
			memcpy(&layout, lyt, sizeof(*lyt));
		}
	uint32_t StripeCount()
		{
			return layout.stripe_count;
		}
};

struct IO {
	int cmd;
	int ref;
	obd_id fid;
	obd_off off;
	obd_count count;
	obd_count left;
	Object *obj;
	Thread *waiter;
	IO *parent;
	void *data;
	
	void Refer(IO *io)
		{
			ref++;
			io->cmd = cmd;
		}
};

struct Request : public List
{
	int type;
	int ref;

	union {
		IO io;
	} u;

	Thread *worker;
};

#endif
