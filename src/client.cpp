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
#include "client.h"
#include "lov.h"
#include "mdc.h"

int Client::finished;
List Client::rwQ;
int Client::num;
int Client::waitnr;
clivec_t Client::vec;

#define RW_READ 1
#define RW_WRITE 2
Client::Client()
		: Node()
{
	id = num++;
	sprintf(name, "Client%d", id);
	vec.push_back(this);
	__dbg = params.debug.Client;
	dt = NULL;
	mt = NULL;
	set = NULL;
//	state = user_StartState;
	rw.rbytes = rw.wbytes = 0;
	rw.rticks = rw.wticks = 0;
	rw.rtime = rw.wtime = 0;
	rw.wperf = rw.rperf = 0;
	rw.type = 0;
}


Client::~Client()
{}

/* Launch the I/O */
void Client::UserStateMachine()
{
	FileObject *obj = &rw.obj;
	IO *io = &rw.io;

	switch (state) {
	case user_FirstState:
	case user_OpenFileState:
		/* FIXME: The file should be created by metadata device.*/
		obj->waiter = &user;
		dt->Create(obj);
		state = user_InitRWState;
		if (params.cluster.MDT) { 
			mt->Open(obj);
			break;
		}
	case user_InitRWState:	
		io->obj = obj;
		io->waiter = &user;
		io->ref = 0;
		io->fid = obj->GetId();

		if (params.io.Mode == ACM_SHARE && params.io.TestRead &&
		        set->rdnr > set->wtnr && id >= set->wtnr) {
			state = user_ReadStartState;
			assert(params.io.WaitAllWrites);
			user.Insert(&rwQ);
			waitnr++;
			break;
		}

		if (params.io.Mode == ACM_FPP) {
			io->off = 0;
			io->count = params.io.IOCount;
		} else {
			obd_count step;

			step = set->fsz / set->wtnr;
			io->count = step * params.SizeUnit;
			io->off = step * id * params.SizeUnit;
			if (id == params.io.WriterCount - 1)
				io->count += (set->fsz % set->wtnr) * params.SizeUnit;
			io->count;
		}
		rw.off = io->off;
		rw.count = io->count;
		rw.wbytes = io->count;
	case user_WriteStartState:
		rw.type |= RW_WRITE;
		io->cmd = WRITE;
		if (params.io.Async || params.io.XferSize == 0) {
			Print(NOW"%s WRITE file (fid@%llu,%llu:%llu).\n",
		      now, name, io->fid, io->off, io->count);
			state = user_WriteCompletionState;
			dt->Write(io);
			break;
		}
		
		Print(NOW "%s WRITE file (fid@%llu) mode: SYNC, xfersize: %llu\n",
			now, name, io->fid, params.io.XferSize);
	case user_WriteXferState:
		/* Direct I/O. (O_DIRECT) */
		if (rw.count != 0) {
			io->count = min(rw.count, params.io.XferSize);
			io->off = rw.off;
			rw.count -= io->count;
			rw.off += io->count;
			state = user_WriteXferState;
			dt->Write(io);
			break;
		}
	case user_WriteCompletionState:
		Print(NOW"%s FINI ALL write.\n", now, name);
		rw.wticks = now;
		if (!params.io.TestRead) {
			state = user_LastState;
			UserStateMachine();
			break;
		} else if (params.io.WaitAllWrites) {
			if (params.io.Mode == ACM_SHARE && 
         set->wtnr > set->rdnr && id >= set->rdnr) {
				state = user_LastState;
			} else {
				state = user_ReadStartState;
				user.Insert(&rwQ);
				waitnr++;
			}
			if (id < set->wtnr && ++finished >= set->wtnr) {
				Print(NOW "Wakeup %d clients to start reading...\n", now, waitnr);
				WakeupAll(&rwQ);
			}
			rw.rtime = now;
			if (state == user_LastState)
				UserStateMachine();
			break;
		}
	case user_ReadStartState:
		if (params.io.Mode == ACM_SHARE) {
			obd_count step;

			step = set->fsz / set->rdnr;
			io->count = step * params.SizeUnit;
			io->off = step * id * params.SizeUnit;
		} else { /* Mode FPP */
			io->off = 0;
			io->count = params.io.IOCount;
		}
		rw.type |= RW_READ;
		rw.rbytes = io->count;
		rw.off = io->off;
		rw.count = io->count;
		io->cmd = READ;

		if (params.io.Async || params.io.XferSize == 0) {
			Print(NOW"%s READ file (fid@%llu, %llu:%llu) .\n",
		      now, name, io->fid, io->off, io->count);
			
			state = user_ReadCompletionState;
			dt->Read(io);
			break;
		}
	
		Print(NOW "%s WRITE file (fid@%llu) mode: SYNC, xfersize: %llu\n",
			now, name, io->fid, params.io.XferSize);
	case user_ReadXferState:
		if (rw.count != 0) {
			io->count = min(rw.count, params.io.XferSize);
			io->off = rw.off;
			rw.count -= io->count;
			rw.off += io->count;
			state = user_ReadXferState;
			dt->Read(io);
			break;
		}
	case user_ReadCompletionState:
		rw.rticks = now - rw.rtime;
		Print(NOW"%s FINI all the READ.\n", now, name);
	case user_LastState:
		Print(NOW"%s FINI all the test.\n", now, name);
		dt->Destroy(obj);
		break;
	}
}

void Client::OpenStateMachine(ThreadLocalData *tld)
{
	Message *msg = tld->m;
	Thread *t = tld->t;

	switch (tld->state) {
	case cl_OpenStartState:
		msg->SetWorker(t);
		msg->SetLength(1024);
		Send(msg);
		tld->state = cl_OpenLastState;
		break;
	case cl_OpenLastState: {
		Thread *w;

		w = ((Object *)msg->req)->waiter;
		w->Signal();
		if (!(tld->flags & TLD_FROM_POOL)) {
			delete t;
			delete tld;
		}
		break;
	}	
	}
}

void Client::WriteStateMachine(ThreadLocalData *tld)
{
	Processor *p = tld->p;
	Message *msg = tld->m;
	Thread *t = tld->t;
	IO *io = (IO *) msg->req;

	switch (tld->state) {
	case cl_WriteStartState:
		msg->cid = id;
		msg->SetWorker(t);
		Print(NOW"%s SEND write REQ@%d FID@%llu (%llu:%llu) to %s\n",
		      now, name, msg->GetId(), io->fid, io->off, io->count, GetNodeName(msg->dst));
		msg->SetLength(1024);
		Send(msg);
		tld->state = cl_WriteRecvGetState;
		break;
	case cl_WriteRecvGetState:
		Print(NOW"%s RECV bulk GET REQ@%d, tranfering bulk data.\n",
		      now, name, msg->GetId());
		msg->SetLength(io->count);
		Send(msg);
		tld->state = cl_WriteCompletionState;
		break;
	case cl_WriteCompletionState: {
		IO *p = (IO *) io->parent;

		Print(NOW"%s FINI write REQ@%d (%llu:%llu).\n",
		      now, name, msg->GetId(), io->off, io->count);

		StatSetPerformance(io);

		if (--p->ref == 0)
			p->waiter->Signal();
	}
	case cl_WriteLastState:
		delete msg;
		delete io;
		p->TaskCompletion(tld);
		break;
	}
}

void Client::ReadStateMachine(ThreadLocalData *tld)
{
	Processor *p = tld->p;
	Message *msg = tld->m;
	IO *io = (IO *)msg->req;
	Thread *t = tld->t;

	switch (tld->state) {
	case cl_ReadStartState:
		msg->cid = id;
		msg->SetWorker(t);
		Print(NOW"%s SEND read REQ@%d FID@%llu %llu:%llu to %s.\n",
		      now, name, msg->GetId(), io->fid, io->off, io->count, GetNodeName(msg->dst));
		msg->SetLength(1024);
		Send(msg);
		tld->state = cl_ReadRecvPutState;
		break;
	case cl_ReadRecvPutState:
		Print(NOW "%s RECV PUT REQ@%d, SEND PUT ACK.\n",
		      now, name, msg->GetId());
		msg->SetLength(1024);
		Send(msg);
		tld->state = cl_ReadCompletionState;
		break;
	case cl_ReadCompletionState: {
		IO *p = io->parent;
		Print(NOW "%s FINI read REQ@%d (%llu:%llu).\n",
		      now, name, msg->GetId(), io->off, io->count);

		StatSetPerformance(io);
		if (--p->ref == 0)
			p->waiter->Signal();
	}
	case cl_ReadLastState:
		delete msg;
		delete io;
		p->TaskCompletion(tld);
		break;
	}
}

void Client::UserProcess(void *arg)
{
	Client *cl = (Client *) arg;

	cl->UserStateMachine();
}

void Client::Handle(void *arg)
{
	ThreadLocalData *tld = (ThreadLocalData *) arg;
	Client *cl = (Client *) tld->n;
	Message *msg = tld->m;

	switch (msg->GetType()) {
	case MSG_OPEN:
		cl->OpenStateMachine(tld);
		break;
	case MSG_READ:
		cl->ReadStateMachine(tld);
		break;
	case MSG_WRITE:
		cl->WriteStateMachine(tld);
		break;
	}
}

int Client::GetId()
{
	return id;
}

void Client::Init(setctl *sctl)
{
	set = sctl;
}

void Client::StatSetPerformance(IO *io)
{
	cfs_duration_t ticks;
	int rw = io->cmd;

	ticks = now - set->stime[rw];
	set->tot[rw] += io->count;
	set->bytes[rw] += io->count;
	if (ticks > params.TimeUnit) {
		obd_count bw;
		bw = set->bytes[rw] * params.TimeUnit / ticks;
		set->stat->Record("%llu.%09llu	%llu.%03llu\n",
		   	now / params.TimeUnit, now % params.TimeUnit,
		   	bw / params.SizeUnit, (bw % params.SizeUnit) * 1000 / params.SizeUnit);
		set->bytes[rw] = 0;
		set->stime[rw] = now;
	}
}

obd_count Client::GetBandwidth(int type)
{
	if (type == READ && (rw.type & RW_READ))
		return (rw.rbytes * params.TimeUnit / rw.rticks);
	if (type == WRITE && (rw.type & RW_WRITE))
		return (rw.wbytes * params.TimeUnit / rw.wticks);

	return 0;
}

cfs_duration_t Client::GetRWTime(int type)
{
	return type ? rw.wticks : rw.rticks;
}

void Client::RecordAggregateBandwidth()
{
	Stat *st;
	obd_count bw;

	st = new Stat("agv.bw");

	st->Record("# Parameters:\n"
		   "# client count: %d\n"
		   "# ost count: %d\n"
		   "# mdt count: %d\n"
		   "# allocator: %s\n"
		   "# time of block allocatoin: %llu\n"
		   "# disk read bandwidth: ~%llu MB/s\n"
		   "# disk write bandwidth: ~%llu MB/s\n"
		   "# disk seek time: %llu\n"
		   "# disk seek random: %llu\n"
		   "# unplug delay: %llu\n"
		   "# access mode: %s\n"
		   "# test read: %s\n"
		   "# stripe count: %lu\n"
		   "# stripe size: %llu\n"
		   "# I/O count per client: %lluM\n"
		   "# xfersize: %llu\n"
		   "# nrs algo: %s\n",
		   params.cluster.ClientCount,
		   params.cluster.OstCount,
		   params.cluster.MdtCount,
		   params.fs.AllocALGO ? "mballoc" : "streamalloc",
		   params.fs.AllocateBlockTime,
		   params.disk.ReadBandwidth * 10,
		   params.disk.WriteBandwidth * 10,
		   params.disk.SeekTicks,
		   params.disk.SeekRandom,
		   params.disk.UnplugDelay,
		   params.io.Mode ? "FPP access mode" : "Shared access mode",
		   params.io.TestRead ? "yes" : "no",
		   params.io.StripeCount,
		   params.io.StripeSize,
		   params.io.IOCount / 1048576,
		   params.io.XferSize,
		   params.nrs.ByArrivalTime ? "FCFS" : "RR");

	for (int i = WRITE; i >= 0; i--) {
		bw = 0;
		for (int j = 0; j < num; j++) {
			bw += vec[j]->GetBandwidth(i);
		}
		st->Record("# Aggregate %s bandwidth: %llu.%03llu MB/sec\n",
			i ? "WRITE" : "READ", bw / params.SizeUnit, 
			(bw % params.SizeUnit) * 1000 / params.SizeUnit);
	}

	for (int i = WRITE; i >= 0; i--) {
		st->Record("# %s bandwidh: [ID, bw, used time]\n", i ? "WRITE" : "READ");
		for (int j = 0; j < num; j++) {
			bw = vec[j]->GetBandwidth(i);
			st->Record("%d	%llu.%03llu	%llu.%03llu\n", j, 
				bw / params.SizeUnit, (bw % params.SizeUnit) * 1000 / params.SizeUnit,
				vec[j]->GetRWTime(i) / params.TimeUnit,
				vec[j]->GetRWTime(i) % params.TimeUnit / 1000000);
		}
	}
	delete st;
}

void Client::Start()
{
	Print(NOW"%s is Starting...\n", now, name);
	Node::Start();
	
	/* Setup and connect to data device */
	dt = new LOV;
	dt->Attach(this);
	dt->Setup();

	/* Setup and connect to metadata Device */
	if (params.cluster.MDT) {
		mt = new MDC;
		mt->Attach(this);
		mt->Setup();
	}

	/* User I/O */
	user.CreateThread(UserProcess, this);
	user.RunAfter(1);

	handler = Handle;
}
