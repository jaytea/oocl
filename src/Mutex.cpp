/*
 * Mutex.cpp
 *
 *  Created on: 14 Jun 2013
 *      Author: jay
 */

#include "Mutex.h"

namespace oocl
{
	/**
	 * @fn	Mutex::Mutex()
	 *
	 * @brief	Constructor.
	 */
	Mutex::Mutex()
		: m_bLocked( false )
	{
#if defined USE_PTHREADS
		m_mutex = PTHREAD_MUTEX_INITIALIZER;
#elif defined USE_WINTHREADS
		m_hMutex = CreateMutex( NULL, FALSE, NULL);
#endif
	}


	/**
	 * @fn	Mutex::~Mutex()
	 *
	 * @brief	Destructor.
	 */
	Mutex::~Mutex()
	{
		m_bLocked = false;
		unlock();
	}


	/**
	 * @fn	Mutex::lock()
	 *
	 * @brief	Locks the mutex, blocks if it is already locked at the moment.
	 */
	void Mutex::lock()
	{
#if defined USE_CPP11
		m_mutex.lock();
#elif defined USE_PTHREADS
		pthread_mutex_lock( &m_mutex );
#elif defined USE_WINTHREADS
		WaitForSingleObject( m_hMutex, 0 );
#endif
		m_bLocked = true;
	}

	/**
	 * @fn	Mutex::try_lock()
	 *
	 * @brief	If the mutex is not locked, locks the mutex and returns true, else returns immediatly without locking and returns false.
	 *
	 * @return 	True if the mutex is successfully locked, true if it was already locked.
	 */
	bool Mutex::try_lock()
	{
		if( !m_bLocked )
			m_bLocked = true;
#ifdef USE_CPP11
		return m_mutex.try_lock();
#elif defined USE_PTHREADS
		return pthread_mutex_trylock( &m_mutex ) == 0;
#elif defined USE_WINTHREADS
		return WaitForSingleObject( m_hMutex, 0 ) == WAIT_OBJECT_0;
#endif
	}

	/**
	 * @fn	Mutex::unlock()
	 *
	 * @brief	Unlocks the mutex.
	 */
	void Mutex::unlock()
	{
#if defined USE_CPP11
		m_mutex.unlock();
#elif defined USE_PTHREADS
		pthread_mutex_unlock( &m_mutex );
#elif defined USE_WINTHREADS
		ReleaseMutex( m_hMutex );
#endif
		m_bLocked = false;
	}

} /* namespace oocl */
