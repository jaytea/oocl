/*
 * Mutex.h
 *
 *  Created on: 14 Jun 2013
 *      Author: jay
 */

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
	 * @class	Mutex
	 *
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
