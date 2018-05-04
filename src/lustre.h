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
#ifndef LUSTRE_H
#define LUSTRE_H

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <cstddef>

using namespace std;

#define GOTO(label, rc)	do { rc; goto label; } while(0)

#define LIBCFS_ALLOC(v, nob) do {               \
        (v) = (void *)calloc(nob, 1);           \
} while (0)

#define LIBCFS_FREE(v, nob) do {                \
        free((void *)v);                        \
} while (0)

#if 1
#define LASSERT(e)
#define BUG_ON(e)
#else
#define LASSERT(e) do {                                         \
        if (!(e)) {                                             \
                printf("Assertion %s failed at %s:%d\n",        \
                       #e, __FILE__, __LINE__);                 \
                abort();                                        \
        }                                                       \
} while (0)
#define BUG_ON(e)	assert(!e)
#endif

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr: the pointer to the member.
 * @type: the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({      \
	const typeof( ((type *)0)->member ) *__mptr = (ptr); \
	(type *)( (char *)__mptr - offsetof(type,member) );})


typedef uint64_t cfs_time_t;
typedef uint64_t cfs_duration_t;
typedef uint64_t obd_count;
typedef uint64_t obd_id;
typedef	uint64_t obd_off;
typedef uint64_t bandwidth_t;

#define MAX_NAME_LEN 32

struct simulate_params_t
{
	struct thread_params_t {
		const cfs_duration_t CtxSwitchTicks;
	} thread;

	struct network_params_t {
		const obd_count PacketSize;
		const obd_count Bandwidth;
		const cfs_duration_t NetLatency;
		const cfs_duration_t InterruptLatency;
		const int N;
	} network;
	
	struct fs_params_t {
		const int AllocALGO;
		const int PreallocOrder;
		const int ReservePA;
		const obd_count PreallocWind;
		const obd_count StreamRequestSize;
		const cfs_duration_t AllocateBlockTime;
		const cfs_duration_t FilesPerFileSystem;
	} fs;

	struct disk_params_t {
		const int ElvHashShift;
		const int ElvNoop;
		const int ElvDeadline;
		const int ElvUnplugThreshold;
		const obd_count ElvMaxReqSize;
		const cfs_duration_t TimeUnit;
		const cfs_duration_t Latency;
		const cfs_duration_t LatencyRandom;
		const obd_count ReadBandwidth;
		const obd_count WriteBandwidth;
		const cfs_duration_t UnplugDelay;
		const cfs_duration_t SeekTicks;
		const cfs_duration_t SeekRandom;
		const int BlockDeviceType;
		struct raid0_params_t {
			const int DiskCount;
			const obd_count DiskSize;
			const unsigned long ChunkSize;
		} raid0;
	} disk;

	struct handle_params_t {
		const cfs_duration_t PingHandleTicks;
		const cfs_duration_t IntervalPerPing;
	} handle;

	struct cluster_params_t {
		const int MDT; /* Support metadata I/O path */
		const int CMD; /* Cluster Metadata Server */
		const int MdtCount;
		const int OstCount;
		const int ClientCount;
		const int PingON;
		const int OstThreadCount;
		const int MdtThreadCount;
		const int MaxRpcsInflight;
		const int ClientSet;
		const int Scale;
	} cluster;

	struct debug_params_t {
		const int Nrs;
		const int NIC;
		const int Elv;
		const int Disk;
		const int Raid0;
		const int FS;
		const int Client;
		const int LOV;
		const int OSC;
		const int OST;
		const int OSD;
		const int MDC;
		const int MDT;
		const int Rand;
	} debug;

	struct stat_params_t {
		const int Nrs;
		const int NIC;
		const int DiskReqSize;
		const int DiskBandwidth;
	} stat;

	struct test_params_t {
		const cfs_duration_t NetTestTicks;
		const int NetLatency;
		const int NetBandwidth;
		const int NicPerformance;
		const int Network;
		const int NetworkNodisk;
		const int DiskLatency;
		const int DiskRandPerf;
		const int DiskSeqPerf;
		const int FsElv;
		const int FsLatency;
		const int FsPerformance;
	} test;

	struct io_params_t {
		/* Access Mode */
		#define ACM_SHARE 0
		#define ACM_FPP 1	
		const int Mode;
		const int TestRead;
		const int WaitAllWrites;
		const uint32_t StripeCount;
		const uint32_t StripePattern;
		const obd_count StripeSize;
		const obd_count IOCount;
		const int WriterCount;
		const int ReaderCount;
		const cfs_duration_t OpenTicks;
		const obd_count XferSize;
		const int Async;
	} io;

	struct nrs_params_t {
		const int ByArrivalTime;
		const int ByKeyVaule; /* used by round-robin algorithm */
		const int GreedByObjid; /* design to mballoc algorithm */
		const int AlgoEpoch;
	} nrs;

	const cfs_duration_t MaxRunTicks;
	const cfs_time_t TimeUnit;
	const obd_count SizeUnit;
};

extern simulate_params_t params;

typedef int (*compare_f)(void *a, void *b);

#define READ	0
#define WRITE	1

#define NOW "At %llu	"
#endif /*LUSTRE_H*/
