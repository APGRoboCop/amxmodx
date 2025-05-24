// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

#ifndef _INCLUDE_SOURCEMOD_BASEWORKER_H
#define _INCLUDE_SOURCEMOD_BASEWORKER_H

#include <amtl/am-linkedlist.h>
#include "ThreadSupport.h"

#define SM_DEFAULT_THREADS_PER_FRAME	1

class BaseWorker;

//SW = Simple Wrapper
class SWThreadHandle : public IThreadHandle
{
	friend class BaseWorker;
public:
	SWThreadHandle(IThreadCreator *parent, const ThreadParams *p, IThread *thread);
	IThread *GetThread();
public:
	//NOTE: We don't support this by default.
	//It's specific usage that'd require many mutexes
	bool WaitForThread() override;
public:
	void DestroyThis() override;
	IThreadCreator *Parent() override;
	void GetParams(ThreadParams *ptparams) override;
public:
	//Priorities not supported by default.
	ThreadPriority GetPriority() override;
	bool SetPriority(ThreadPriority prio) override;
public:
	ThreadState GetState() override;
	bool Unpause() override;
private:
	ThreadState m_state;
	ThreadParams m_params;
	IThreadCreator *m_parent;
	IThread *pThread;
};

class BaseWorker : public IWorker
{
public:
	BaseWorker();
	~BaseWorker() override;
public:	//IWorker
	unsigned int RunFrame() override;
	//Controls the worker
	bool Pause() override;
	bool Unpause() override;
	bool Start() override;
	bool Stop(bool flush_cancel) override;
	//Flushes out any remaining threads
	unsigned int Flush(bool flush_cancel) override;
	//returns status and number of threads in queue
	WorkerState GetStatus(unsigned int *numThreads) override;
public:	//IThreadCreator
	void MakeThread(IThread *pThread) override;
	IThreadHandle *MakeThread(IThread *pThread, ThreadFlags flags) override;
	IThreadHandle *MakeThread(IThread *pThread, const ThreadParams *params) override;
	void GetPriorityBounds(ThreadPriority &max, ThreadPriority &min) override;
public:	//BaseWorker
	virtual void AddThreadToQueue(SWThreadHandle *pHandle);
	virtual SWThreadHandle *PopThreadFromQueue();
	virtual void SetMaxThreadsPerFrame(unsigned int threads);
	virtual unsigned int GetMaxThreadsPerFrame();
protected:
	ke::LinkedList<SWThreadHandle *> m_ThreadQueue;
	unsigned int m_perFrame;
	volatile WorkerState m_state;
};

#endif //_INCLUDE_SOURCEMOD_BASEWORKER_H
