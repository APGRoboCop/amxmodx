// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

#ifndef _INCLUDE_WINTHREADS_H_
#define _INCLUDE_WINTHREADS_H_

#include <Windows.h>
#include "IThreader.h"

using namespace SourceMod;

DWORD WINAPI Win32_ThreadGate(LPVOID param);

class WinThreader : public IThreader
{
public:
	class ThreadHandle : public IThreadHandle
	{
		friend class WinThreader;
		friend DWORD WINAPI Win32_ThreadGate(LPVOID param);
	public:
		ThreadHandle(IThreader *parent, HANDLE hthread, IThread *run, const ThreadParams *params);
		~ThreadHandle() override;
	public:
		bool WaitForThread() override;
		void DestroyThis() override;
		IThreadCreator *Parent() override;
		void GetParams(ThreadParams *ptparams) override;
		ThreadPriority GetPriority() override;
		bool SetPriority(ThreadPriority prio) override;
		ThreadState GetState() override;
		bool Unpause() override;
	protected:
		IThreader *m_parent;		//Parent handle
		HANDLE m_thread;			//Windows HANDLE	
		ThreadParams m_params;		//Current Parameters
		IThread *m_run;	//Runnable context
		ThreadState m_state;		//internal state
		CRITICAL_SECTION m_crit;
	};
	class WinMutex : public IMutex
	{
	public:
		WinMutex(HANDLE mutex) : m_mutex(mutex)
		{
		}

		~WinMutex() override;
	public:
		bool TryLock() override;
		void Lock() override;
		void Unlock() override;
		void DestroyThis() override;
	protected:
		HANDLE m_mutex;
	};
	class WinEvent : public IEventSignal
	{
	public:
		WinEvent(HANDLE event) : m_event(event)
		{
		}

		~WinEvent() override;
	public:
		void Wait() override;
		void Signal() override;
		void DestroyThis() override;
	public:
		HANDLE m_event;
	};
public:
	IMutex *MakeMutex() override;
	void MakeThread(IThread *pThread) override;
	IThreadHandle *MakeThread(IThread *pThread, ThreadFlags flags) override;
	IThreadHandle *MakeThread(IThread *pThread, const ThreadParams *params) override;
	void GetPriorityBounds(ThreadPriority &max, ThreadPriority &min) override;
	void ThreadSleep(unsigned int ms) override;
	IEventSignal *MakeEventSignal() override;
};

#if defined SM_DEFAULT_THREADER && !defined SM_MAIN_THREADER
#define SM_MAIN_THREADER WinThreader;
typedef class WinThreader MainThreader;
#endif

#endif //_INCLUDE_WINTHREADS_H_
