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
// This file was written by Jörn Teuber

#ifndef MUTEX_H_
#define MUTEX_H_

#ifdef USE_CPP11
#	include <mutex>
#else
#	include "Thread.h"
#endif

namespace oocl
{
	/**
	 * @brief	Wrapper class for mutexes on different platforms.
	 *
	 * @note	If USE_CPP11 is defined it uses the c++11 std::mutex, else the platform specific implementation.
	 *
	 * @author	Jörn Teuber
	 * @date	20.5.2013
	 */
	class Mutex
	{
	public:
		Mutex();
		~Mutex();

		void lock();
		bool try_lock();

		void unlock();

	private:
		Mutex( Mutex& m );
		Mutex& operator=(const Mutex&);

	private:
		bool m_bLocked;
#if defined USE_CPP11
		std::mutex m_mutex;
#elif defined USE_PTHREADS
		pthread_mutex_t m_mutex;
#elif defined USE_WINTHREADS
		HANDLE m_hMutex;
#endif
	};

} /* namespace oocl */

#endif /* MUTEX_H_ */
