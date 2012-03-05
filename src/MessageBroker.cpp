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

#include "MessageBroker.h"

namespace oocl
{

	///< The list of all registered MessageBroker
	std::vector< MessageBroker* > MessageBroker::sm_vBroker;


	/**
	 * @fn	MessageBroker::MessageBroker(void)
	 *
	 * @brief	Default constructor.
	 */
	MessageBroker::MessageBroker(void)
		: m_pExclusiveListener( NULL ),
		m_iSequenceNumber( 0 ),
		m_bRunThread( true )
	{
		start();
	}


	/**
	 * @fn	MessageBroker::~MessageBroker(void)
	 *
	 * @brief	Destructor.
	 */
	MessageBroker::~MessageBroker(void)
	{
		m_bRunThread = false;
	}


	/**
	 * @fn	MessageBroker* MessageBroker::getBrokerFor( unsigned short usMessageType )
	 *
	 * @brief	Returns the broker for the given message type.
	 *
	 * @param	usMessageType	Message type for which you need the broker.
	 *
	 * @return	The broker you need.
	 */
	MessageBroker* MessageBroker::getBrokerFor( unsigned short usMessageType )
	{
		if( usMessageType >= sm_vBroker.size() )
		{
			sm_vBroker.resize( usMessageType+1, NULL );

			MessageBroker* pBroker = new MessageBroker();
			sm_vBroker[usMessageType] = pBroker;
			return pBroker;
		}
		else if( sm_vBroker[usMessageType] == NULL )
		{
			MessageBroker* pBroker = new MessageBroker();
			sm_vBroker[usMessageType] = pBroker;
			return pBroker;
		}
		else
			return sm_vBroker[usMessageType];
	}


	/**
	 * @fn	void MessageBroker::registerListener( MessageListener* pListener )
	 *
	 * @brief	Registers the listener described by pListener.
	 *
	 * @param [in]	pListener	The listener you want to register with this brocker.
	 */
	void MessageBroker::registerListener( MessageListener* pListener )
	{
		if( pListener )
			m_lListeners.push_back( pListener );
	}


	/**
	 * @fn	void MessageBroker::unregisterListener( MessageListener* pListener )
	 *
	 * @brief	Unregisters the listener described by pListener.
	 *
	 * @param [in]	pListener	The listener you want to unregister.
	 */
	void MessageBroker::unregisterListener( MessageListener* pListener )
	{
		if( pListener )
			m_lListeners.remove( pListener );
	}


	/**
	 * @fn	void MessageBroker::pumpMessage( Message* pMessage )
	 *
	 * @brief	Pump message.
	 *
	 * @param [in,out]	pMessage	If non-null, the message.
	 */
	void MessageBroker::pumpMessage( Message* pMessage )
	{
		if( pMessage )
			m_lMessageQueue.push_back( pMessage );
	}


	/**
	 * @fn	bool MessageBroker::requestExclusiveMessaging( MessageListener* pListener )
	 *
	 * @brief	Request exclusive messaging, so that only the given MessageListener gets messages until discarded.
	 *
	 * @param [in]	pListener	The listener that needs full attention.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool MessageBroker::requestExclusiveMessaging( MessageListener* pListener )
	{
		if( !m_pExclusiveListener )
		{
			m_pExclusiveListener = pListener;
			return true;
		}

		return false;
	}


	/**
	 * @fn	bool MessageBroker::discardExclusiveMessaging( MessageListener* pListener )
	 *
	 * @brief	Discard exclusive messaging.
	 *
	 * @param [in]	pListener	The listener.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool MessageBroker::discardExclusiveMessaging( MessageListener* pListener )
	{
		if( m_pExclusiveListener == pListener )
		{
			m_pExclusiveListener = NULL;
			return true;
		}

		return false;
	}


	/**
	 * @fn	void MessageBroker::run()
	 *
	 * @brief	Distributes the messages in the message queue of this broker.
	 */
	void MessageBroker::run()
	{
		// TODO: This really needs an overhaul. In practice this wastes way to much time inside the idle loop. 
		//			A simple heuristic could decide whether we stay in the thread or close it and open another on demand.
		while( m_bRunThread )
		{
			while( m_lMessageQueue.size() == 0 )
				sleep(0);

			Message* pMessage = m_lMessageQueue.front();

			if( !m_pExclusiveListener )
			{
				std::list< MessageListener* > lWaitList( m_lListeners.begin(), m_lListeners.end() );

				for( std::list< MessageListener* >::iterator it = lWaitList.begin(); it != lWaitList.end(); it = lWaitList.erase( it ) )
				{
					if( !(*it)->cbMessage( pMessage ) )
						lWaitList.push_back( (*it) );
				}
			}
			else
			{
				bool bRet = false;
				do {
					bRet = m_pExclusiveListener->cbMessage( pMessage );
				} while( !bRet );
			}

			m_lMessageQueue.pop_front();
		}
	}

}