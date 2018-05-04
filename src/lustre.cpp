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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <lustre.h>

simulate_params_t params = {
	thread: {
		CtxSwitchTicks: 50,
	},

	network: {
		PacketSize: 4096,
		Bandwidth: 1,
		NetLatency: 1000, /* 1 us */
		InterruptLatency: 25000,
		N: 1,
	},
	
	fs: {
		/*
		 * 0: stream allocation algorithm;
		 * 1: mballoc allocation algorithm;
		 */
		AllocALGO: 1,
		PreallocOrder: 2,
		ReservePA: 1,
		PreallocWind: 4 * 1048576ULL,
		StreamRequestSize: 64 * 1024ULL,
		AllocateBlockTime: 800000, /* 30 us */ 
		FilesPerFileSystem: 1000,
	},

	disk: {
		ElvHashShift: 6,
		ElvNoop: 1,
		ElvDeadline: 0,
		ElvUnplugThreshold: 4,
		ElvMaxReqSize: 4ULL * 1048576ULL, /* 4M */
		TimeUnit: 100,
		Latency: 20000, //100000,
		LatencyRandom: 6000, //30000,
		ReadBandwidth: 30, /* 2 bytes/10ns ~ 200M/s */
		WriteBandwidth: 30,
		UnplugDelay: 3000000, /* 3 ms */
		SeekTicks: 800000, /* 5 ms */
		SeekRandom: 200000, /* 0.8 ms */
		/*
		 * 0: single Disk;
		 * 1: raid0 device;
		 */
		BlockDeviceType: 0,
		raid0: {
			DiskCount: 20,
			DiskSize: 0,
			ChunkSize: 32 * 1048576ULL,
		},
	},

	handle: {
		PingHandleTicks: 1000000ULL,
		IntervalPerPing: 30000000000ULL,
	},

	cluster: {
		MDT: 0,
		CMD: 0,
		MdtCount: 0,
		OstCount: 72,
		ClientCount: 1000,
		PingON: 0,
		OstThreadCount: 32,
		MdtThreadCount: 2,
		MaxRpcsInflight: 8,
		ClientSet: 1,
		Scale: 0,
	},

	debug: {
		Nrs: 0,
		NIC: 0,
		Elv: 0,
		Disk: 0,
		Raid0: 0,
		FS: 0,
		Client: 0,
		LOV: 0,
		OSC: 0,
		OST: 1,
		OSD: 0,
		MDC: 0,
		MDT: 0,
		Rand: 1, 
	},

	stat: {
		Nrs: 1,
		NIC: 1,
		DiskReqSize: 1,
		DiskBandwidth: 1,
	},

	test: {
		NetTestTicks: 10000000000ULL,
		NetLatency: 1,
		NetBandwidth: 0,
		NicPerformance: 0,
		Network: 0,
		NetworkNodisk: 0,
		DiskLatency: 0,
		DiskRandPerf: 0,
		DiskSeqPerf: 1,
		FsElv: 1,
		FsLatency: 0,
		FsPerformance: 0,
	},

	io: {
		/*
		 * ACM_SHARE 0: shared access mode;
		 * ACM_FPP 1: File Per Processor (FPP) access mode;
		 */
		Mode: ACM_FPP,
		TestRead: 1,
		WaitAllWrites: 1,
		StripeCount: 1,
		StripePattern: 0,
		StripeSize: 1048576,
		IOCount: 32 * 1048576ULL, //(32ULL * 1048576ULL), /* 32M */
		WriterCount: 100,
		ReaderCount: 100,
		OpenTicks: 500000000, /* 10 ms */
		XferSize: 64 * 1024ULL,
		Async: 0,
	},

	nrs: {
		ByArrivalTime: 1, /* Default algorithm used by Lustre: FCFS */
		ByKeyVaule: 0,
		GreedByObjid: 0,
		AlgoEpoch: 0,
	},

	MaxRunTicks: 0, //10000000000ULL,
	TimeUnit: 1000000000ULL, /* 1s = 10 ^ 9 tick */
	SizeUnit: 1048576,
};

#include <disk.h>
#include <filesystem.h>
#include <nic.h>
#include <cluster.h>
#include <nrsrbtree.h>
#include <raid0device.h>
int main(int argc, char *argv[])
{
	Cluster lustre("lustre");

	//Disk::SelfBenchmark();
	//NIC::SelfBenchmark();
	//NrsRbtree::SelfTest();
	//FileSystem::SelfTest();
	//Raid0Device::SelfTest();
	lustre.Start();
	return EXIT_SUCCESS;
}
