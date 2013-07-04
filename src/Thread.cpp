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
// This file was written by Jürgen Lorenz and Jörn Teuber

#include "Thread.h"

namespace oocl
{
	int Thread::sm_iThreadCount = 0;

	/**
	 * @fn	Thread::Thread()
	 *
	 * @brief	Default constructor.
	 */
	Thread::Thread()
		: m_iThreadPriority( TP_Normal )
		, m_bActive( false )
#ifdef USE_CPP11
		, m_pThread( NULL )
#endif
	{
	}


	Thread::~Thread()
	{
#if defined USE_CPP11
		delete m_pThread;
#elif defined USE_WINTHREADS
		CloseHandle(m_hThread);
#endif
	}


	/**
	 * @fn	void Thread::join()
	 *
	 * @brief	blocks until the thread has finished.
	 */
	void Thread::join()
	{
		if( m_bActive )
		{
#if defined USE_CPP11
			m_pThread->join();
#elif defined linux
			pthread_join(m_iThreadID, NULL);
#else
			WaitForSingleObject(m_hThread,INFINITE);
#endif
		}
	}


	/**
	 * @fn	void Thread::sleep( int iMilliseconds )
	 *
	 * @brief	Blocks the thread for a specified time.
	 *
	 * @param	iMilliseconds	milliseconds to sleep or 0 to pass the current CPU timeslot to the next thread or process.
	 */
	void Thread::sleep( int iMilliseconds )
	{
#ifdef linux
		usleep(iMilliseconds*1000);
#else
		Sleep(iMilliseconds);
#endif
	}


	/**
	 * @fn	EPriority Thread::getPriority()
	 *
	 * @brief	Get the threads priority.
	 *
	 * @note	A Thread built with USE_CPP11 will always return TP_Normal, as std::threads do not have a priority.
	 *
	 * @return	The threads current priority.
	 */
	Thread::EPriority Thread::getPriority()
	{
		return m_iThreadPriority;
	}


	/**
	 * @fn	bool Thread::isAlive()
	 *
	 * @brief	Query whether this thread is alive.
	 *
	 * @return	true if alive, false if not.
	 */
	bool Thread::isAlive()
	{
		return m_bActive;
	}


	/**
	 * @fn	bool Thread::start()
	 *
	 * @brief	Starts the thread.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Thread::start()
	{
#if defined USE_CPP11
		try {
			m_pThread = new std::thread( Thread::entryPoint, this );
			return true;
		}
		catch( std::system_error& e )
		{
			Log::getLogRef("oocl") << Log::EL_ERROR << "Unable to start thread: " << e.what() << endl;
			return false;
		}
#elif defined linux
		return !pthread_create(&m_iThreadID, NULL, Thread::entryPoint, this);
#else
		m_hThread = CreateThread( NULL, 0, Thread::entryPoint, this, 0, &m_iThreadID);
		return (m_hThread != NULL);
#endif
	}

#if defined USE_CPP11
	int Thread::entryPoint(Thread* pthis)
#elif defined linux
	void * Thread::entryPoint(void * pthis)
#else
	DWORD WINAPI Thread::entryPoint(LPVOID pthis)
#endif
	{
		((Thread*)pthis)->m_bActive = true;
		((Thread*)pthis)->run();
		((Thread*)pthis)->m_bActive = false;

		return 0;
	}

	/**
	 * @fn	void Thread::setPriority( EPriority iPriority )
	 *
	 * @brief	Sets a new thread priority.
	 *
	 * @note 	Has no effect when USE_CPP11 is defined
	 *
	 * @param	iPriority	the new priority.
	 */
	void Thread::setPriority( EPriority iPriority )
	{
		m_iThreadPriority = iPriority;

#ifndef USE_CPP11
#	ifdef linux
		struct sched_param sp;
		memset(&sp, 0, sizeof(struct sched_param));
	
		int iMin = sched_get_priority_min(0);
		int iMax = sched_get_priority_max(0);
		int iAvg = (iMax + iMin) / 2;

		sp.sched_priority = iAvg + ( iPriority * (iMax - iMin) ) / 4;

		pthread_setschedparam(m_iThreadID, SCHED_RR, &sp);
#	else
		SetThreadPriority(&m_hThread, iPriority);
#	endif
#endif
	}

}
