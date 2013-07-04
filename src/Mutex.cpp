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

#include "Mutex.h"

namespace oocl
{
	/**
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
	 * @brief	Destructor.
	 */
	Mutex::~Mutex()
	{
		m_bLocked = false;
		unlock();
	}


	/**
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
