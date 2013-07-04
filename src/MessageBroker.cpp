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
	std::map< unsigned int, MessageBroker* > MessageBroker::sm_vBroker;


	/**
	 * @brief	Default constructor.
	 */
	MessageBroker::MessageBroker(void)
		: m_pExclusiveListener( NULL )
		, m_bRunThread( false )
		, m_bRunContinuously( false )
		, m_bSynchronous( false )
	{
	}


	/**
	 * @brief	Destructor.
	 */
	MessageBroker::~MessageBroker(void)
	{
		m_bRunThread = false;
	}


	/**
	 * @brief	Returns the broker for the given message type.
	 *
	 * @param	usMessageType	Message type for which you need the broker.
	 *
	 * @return	The requested broker.
	 */
	MessageBroker* MessageBroker::getBrokerFor( unsigned short usMessageType )
	{
		if( sm_vBroker.find( usMessageType ) == sm_vBroker.end() )
		{
			MessageBroker* pBroker = new MessageBroker();
			sm_vBroker[usMessageType] = pBroker;
			return pBroker;
		}
		else
			return sm_vBroker[usMessageType];
	}


	/**
	 * @brief	Registers the listener described by pListener.
	 *
	 * @param [in]	pListener	The listener to register with this brocker.
	 */
	void MessageBroker::registerListener( MessageListener* pListener )
	{
		if( pListener != NULL )
		{
			m_mxListener.lock();
			m_lListeners.push_back( pListener );
			m_mxListener.unlock();
		}
	}


	/**
	 * @brief	Unregisters the listener given by pListener.
	 *
	 * @param [in]	pListener	The listener to unregister.
	 */
	void MessageBroker::unregisterListener( MessageListener* pListener )
	{
		if( pListener != NULL )
		{
			m_mxListener.lock();
			m_lListeners.remove( pListener );
			m_mxListener.unlock();
		}
	}


	/**
	 * @brief	Send a message to all registered listeners.
	 *
	 * @param [in]	pMessage	If non-null, the message to send.
	 */
	void MessageBroker::pumpMessage( Message* pMessage )
	{
		if( !m_lListeners.empty() && pMessage != NULL )
		{
			if( m_bSynchronous )
			{
				deliverMessage( pMessage );
			}
			else
			{
				m_mxQueue.lock();
				m_lMessageQueue.push( pMessage );
				m_mxQueue.unlock();
		
				if( !m_bRunThread )
				{
					m_bRunThread = true;
					start();
				}
			}
		}
	}


	/**
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
			m_mxExclusiveListener.lock();
			m_pExclusiveListener = pListener;
			m_mxExclusiveListener.unlock();

			return true;
		}

		return false;
	}


	/**
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
			m_mxExclusiveListener.lock();
			m_pExclusiveListener = NULL;
			m_mxExclusiveListener.unlock();

			return true;
		}

		return false;
	}


	/**
	 * @brief	Enables continuous processing of messages in the message passing thread.
	 * 			
	 * @note	synchronous messaging will be disabled when you enable continuous processing.
	 */
	void MessageBroker::enableContinuousProcessing()
	{
		m_bRunContinuously = true;
		m_bSynchronous = false;
	}


	/**
	 * @brief	Disables continuous processing.
	 */
	void MessageBroker::disableContinuousProcessing()
	{
		m_bRunContinuously = false;
	}


	/**
	 * @brief	Enables synchronous messaging, i.e. pumpMessage() will be blocking until all listeners got the message.
	 * 			
	 * @note	continuous processing will be disabled when you enable synchronous messaging.
	 */
	void MessageBroker::enableSynchronousMessaging()
	{
		m_bRunContinuously = false;
		m_bSynchronous = true;
	}


	/**
	 * @brief	Disables synchronous messaging.
	 */
	void MessageBroker::disableSynchronousMessaging()
	{
		m_bSynchronous = false;
	}


	/**
	 * @brief	Distributes the messages in the message queue of this broker.
	 */
	void MessageBroker::run()
	{
		m_bRunThread = true;

		while( m_bRunThread )
		{
			// wait for a new message
			while( m_lMessageQueue.empty() && m_bRunThread )
				Thread::sleep(0);

			// get the queues first element and instantly remove it from the queue so that it is not used twice
			m_mxQueue.lock();
			Message* pMessage = m_lMessageQueue.front();

			deliverMessage( pMessage );
			
			m_lMessageQueue.pop();
			m_mxQueue.unlock();

			// delete the message to prevent memory holes, the listeners should have finished processing the data
			delete pMessage;
			
			// when the thread is not set to run the whole time, quit the thread if the message queue is empty
			if( !m_bRunContinuously )
				m_bRunThread = !m_lMessageQueue.empty();
		}
	}


	/**
	 * @brief 	Helper methods to deliver the given message to all listeners.
	 *
	 * @param pMessage	The message to deliver.
	 */
	void MessageBroker::deliverMessage( Message* pMessage )
	{
		// if no listener requested exclusive delivery, deliver the message to all listeners
		if( m_pExclusiveListener == NULL && !m_lListeners.empty() )
		{
			m_mxListener.lock();

			// build a stack of listeners that have not received the message yet
			std::list< MessageListener* > lWaitList( m_lListeners.begin(), m_lListeners.end() );

			for( std::list< MessageListener* >::iterator it = lWaitList.begin(); it != lWaitList.end(); it = lWaitList.erase( it ) )
			{
				// if the message can not be dealt with now, the listener should return false and will be pushed to the end of the listener stack
				if( !(*it)->cbMessage( pMessage ) )
					lWaitList.push_back( (*it) );
			}

			m_mxListener.unlock();
		}
		// else deliver the message only to the current exclusive listener
		else if( m_pExclusiveListener != NULL )
		{
			m_mxExclusiveListener.lock();

			bool bRet = false;
			do {
				bRet = m_pExclusiveListener->cbMessage( pMessage );
			} while( !bRet );

			m_mxExclusiveListener.unlock();
		}
	}

}
