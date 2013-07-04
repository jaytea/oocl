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

#ifndef THREAD_H_INCLUDED
#define THREAD_H_INCLUDED

#if defined USE_CPP11
#	include <thread>
#elif defined linux
#	include <pthread.h>
#	include <unistd.h>
#	include <string.h>
#	define USE_PTHREADS
#else
#	define USE_WINTHREADS
#endif

#ifdef WIN32
#	include <windows.h>
#endif

#include "oocl_import_export.h"
#include "Log.h"

namespace oocl
{
	/**
	 * @class	Thread
	 *
	 * @brief	Wrapper class for threads on different platforms.
	 *
	 * @note	If USE_CPP11 is defined it uses the c++11 std::thread, else a platform specific implementation.
	 *
	 * @author	Jörn Teuber
	 * @date	23.11.2011
	 */
	class OOCL_EXPORTIMPORT Thread
	{
	public:

		/**
		 * @enum	EPriority
		 *
		 * @brief	Values that represent the thread priority.
		 */
		enum EPriority
		{
			TP_Lowest = -2,
			TP_Low,
			TP_Normal,
			TP_High,
			TP_Highest
		};

		Thread();
		virtual ~Thread();

		bool start();
		void join();

		// setter
		void setPriority( EPriority iPriority );

		// getter
		EPriority  getPriority();
		bool isAlive();
		
		// for calling from inside the thread
		static void sleep( int iMilliseconds = 0 );
		 
	protected:
		

		/**
		 * @fn	void Thread::run()
		 *
		 * @brief	This is the method called inside the thread.
		 *
		 * @note	Overwrite this with your thread code.
		 */
		virtual void run() = 0;
		
#if defined USE_CPP11
		static int entryPoint(Thread* pthis);
#elif defined linux
		static void * entryPoint(void* pthis);
#else
		static DWORD WINAPI entryPoint(LPVOID runnableInstance);
#endif

	private:
#if defined USE_CPP11
		std::thread* m_pThread;
#elif defined linux
		///< Identifier for the thread
		pthread_t m_iThreadID;
#else
		///< Identifier for the thread
		DWORD m_iThreadID;
		///< Handle of the thread
		HANDLE m_hThread;
#endif

		///< The thread priority
		EPriority m_iThreadPriority;
		///< Number of threads
		static int sm_iThreadCount;

		bool m_bActive;
	};

}

#endif // THREAD_H_INCLUDED
