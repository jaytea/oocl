/*
Object Oriented Communication Library
Copyright (c) 2011 Jürgen Lorenz and Jörn Teuber

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
/// This file was written by Jürgen Lorenz and Jörn Teuber

#include "Thread.h"

namespace oocl
{

	int Thread::sm_iThreadCount = 0;

	Thread::Thread(){
		m_bRuns = false;
	}

	void Thread::join()
	{
#ifdef linux
		pthread_join(m_iThreadID, NULL);
#else
		WaitForSingleObject(m_hThread,INFINITE);
#endif
	}

	bool Thread::isAlive(){
		return m_bRuns;
	}

	int Thread::activeCount(){
		return sm_iThreadCount;
	}

	bool Thread::start()
	{
#ifdef linux
		return !pthread_create(&m_iThreadID, NULL, Thread::entryPoint, this);
#else
		m_hThread = CreateThread( NULL, 0, Thread::entryPoint, this, 0, &m_iThreadID);
		return m_hThread;
#endif
	}

#ifdef linux
	void * Thread::entryPoint(void * pthis)
#else
	DWORD WINAPI Thread::entryPoint(LPVOID pthis)
#endif
	{
		((Thread*)pthis)->run();
		return 0;
	}

	void Thread::setPriority( EPriority iPriority )
	{
		m_iThreadPriority = iPriority;
	
#ifdef linux
		struct sched_param sp;
		memset(&sp, 0, sizeof(struct sched_param));
	
		int iMin = sched_get_priority_min();
		int iMax = sched_get_priority_max();
		int iAvg = (iMax + iMin) / 2;

		sp.sched_priority = iAvg + ( iPriority * (iMax - iMin) ) / 4;

		pthread_setschedparam(m_iThreadID, SCHED_RR, &sp);
#else
		SetThreadPriority(&m_hThread, iPriority);
#endif
	}

}
