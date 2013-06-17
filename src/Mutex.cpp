/*
 * Mutex.cpp
 *
 *  Created on: 14 Jun 2013
 *      Author: jay
 */

#include "Mutex.h"

namespace oocl
{

	Mutex::Mutex()
		: m_bLocked( false )
	{
#if defined USE_PTHREADS
		m_mutex = PTHREAD_MUTEX_INITIALIZER;
#elif defined USE_WINTHREADS
		m_hMutex = CreateMutex( NULL, FALSE, NULL);
#endif
	}

	Mutex::~Mutex()
	{
		m_bLocked = false;
		unlock();
	}


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
