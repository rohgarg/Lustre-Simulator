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
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <lustre.h>

/**
	@author yingjin.qian <yingjin.qian@sun.com>
*/

#define NRS_ALGO_FIFO 1
#define NRS_ALGO_BINHEAP 2
#define NRS_ALGO_RBTREE 3
#define NRS_ALGO_EPOCH 4

class Scheduler
{
protected:
	int __dbg:1;
	obd_count	size;
	void Print(const char *fmt...);
public:
  Scheduler();
  ~Scheduler();
	
	virtual int Enqueue(void *e) = 0;
	virtual void *Dequeue() = 0;
	virtual void Erase(void *e) = 0;
	virtual int Requeue(void *e) = 0;
	obd_count Size();
};

#endif
