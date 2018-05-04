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
#ifndef CLIENT_H
#define CLIENT_H

#include <node.h>
#include <obd.h>

/**
	@author yingjin.qian <yingjin.qian@sun.com>
*/

struct setctl
{
	int setnr; /* number of sets */
	int type;
	int c;

	/*used by client component */

	/* shared access mode */
	obd_count fsz;
	obd_id fid;
	int rdnr;
	int wtnr;

	cfs_time_t stime[2];
	obd_count bytes[2];
	//obd_count bw[2];
	obd_count tot[2];
	Stat *stat;
};

class Client;
typedef vector<Client *> clivec_t;
class Client : public Node
{
	static int finished;
	static int waitnr;
	static List rwQ;
	static int num;
	static clivec_t vec;

	int id;
	DataDevice *dt;
	MetadataDevice *mt;

	struct rw_t
	{
		int type;
		int rw;
		IO io;
		FileObject obj;
		Request req;
	
		/* I/O information */
		obd_off off;
		obd_count count;
		/* stat information. */
		cfs_time_t rtime; /* time when start reading */
		cfs_time_t wtime; /* time when start writing */
		cfs_duration_t rticks;
		cfs_duration_t wticks;
		obd_count rbytes;
		obd_count wbytes;
		obd_count rperf;
		obd_count wperf;
	};

	rw_t rw;
	Thread user;

	setctl *set;

	/* Application state machine */
	enum {
		user_FirstState,
		user_OpenFileState,
		user_InitRWState,
		user_WriteStartState,
		user_WriteXferState,
		user_WriteCompletionState,
		user_WriteWaitAllFinishState,
		user_ReadStartState,
		user_ReadXferState,
		user_ReadCompletionState,
		user_LastState,
	} state;

	enum {
		cl_OpenStartState,
		cl_OpenLastState,
	};
	enum {
		cl_WriteStartState,
		cl_WriteRecvGetState,
		cl_WriteCompletionState,
		cl_WriteLastState,
	};
	enum {
		cl_ReadStartState,
		cl_ReadRecvPutState,
		cl_ReadCompletionState,
		cl_ReadLastState,
	};
	
	void OpenStateMachine(ThreadLocalData *tld);
	void ReadStateMachine(ThreadLocalData *tld);
	void WriteStateMachine(ThreadLocalData *tld);
	void UserStateMachine();

	static void UserProcess(void *arg);
	static void Handle(void *arg);

	/* statistics */
	void StatSetPerformance(IO *io);

public:
	Client();
	~Client();

	int GetId();
	void Init(setctl *sctl);
	void Start();

	obd_count GetBandwidth(int rw);
	cfs_duration_t GetRWTime(int rw);
	static void RecordAggregateBandwidth();
};

#endif
