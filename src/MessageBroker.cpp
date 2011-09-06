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

#include "MessageBroker.h"

namespace oocl
{

	std::map< unsigned short, MessageBroker* > MessageBroker::sm_mapBroker;


	MessageBroker::MessageBroker(void)
		: m_pExclusiveListener( NULL ),
		m_iSequenceNumber( 0 ),
		m_bRunThread( true )
	{
		start();
	}

	MessageBroker::~MessageBroker(void)
	{
		m_bRunThread = false;
	}

	MessageBroker* MessageBroker::getBrokerFor( unsigned short usMessageType )
	{
		std::map< unsigned short, MessageBroker* >::iterator it = sm_mapBroker.find( usMessageType );
		if( it == sm_mapBroker.end() )
		{
			MessageBroker* pBroker = new MessageBroker();
			sm_mapBroker.insert( std::pair< unsigned short, MessageBroker* >( usMessageType, pBroker ) );
			return pBroker;
		}
		else
			return it->second;
	}

	bool MessageBroker::registerListener( MessageListener* pListener )
	{
		m_lListeners.push_back( pListener );
		return true;
	}

	bool MessageBroker::unregisterListener( MessageListener* pListener )
	{
		for( std::list< MessageListener* >::iterator it = m_lListeners.begin(); it != m_lListeners.end(); it++ )
		{
			if( (*it) == pListener )
			{
				m_lListeners.erase( it );
				return true;
			}
		}

		return false;
	}

	bool MessageBroker::pumpMessage( Message* pMessage )
	{
		if( m_lMessageQueue.size() == 10 )
			m_lMessageQueue.pop_front();

		m_lMessageQueue.push_back( pMessage );

		return true;
	}

	bool MessageBroker::requestExclusiveMessaging( MessageListener* pListener )
	{
		if( !m_pExclusiveListener )
		{
			m_pExclusiveListener = pListener;
			return true;
		}

		return false;
	}

	bool MessageBroker::discardExclusiveMessaging( MessageListener* pListener )
	{
		if( m_pExclusiveListener == pListener )
		{
			m_pExclusiveListener = NULL;
			return true;
		}

		return false;
	}

	void MessageBroker::run()
	{
		while( m_bRunThread )
		{
			while( m_lMessageQueue.size() == 0 )
				Sleep(0);

			std::list< MessageListener* > lWaitList( m_lListeners );

			Message* pMessage = m_lMessageQueue.front();

			if( !m_pExclusiveListener )
			{
				for( std::list< MessageListener* >::iterator it = lWaitList.begin(); it != lWaitList.end(); it = lWaitList.erase( it ) )
				{
					if( (*it)->requestMutex() )
					{
						(*it)->cbMessage( pMessage );
						(*it)->returnMutex();
					}
					else
						lWaitList.push_back( (*it) );
				}
			}
			else
			{
				m_pExclusiveListener->cbMessage( pMessage );
			}

			m_lMessageQueue.pop_front();
		}
	}

}