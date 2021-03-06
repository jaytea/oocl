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

#ifndef MESSAGEBROKER_H_INCLUDED
#define MESSAGEBROKER_H_INCLUDED

#include <map>
#include <list>
#include <queue>

#include "oocl_import_export.h"

#include "Thread.h"
#include "Mutex.h"
#include "MessageListener.h"

namespace oocl
{
	class MessageBroker;
}

EXPIMP_TEMPLATE template class OOCL_EXPORTIMPORT std::allocator<oocl::MessageBroker*>;
EXPIMP_TEMPLATE template class OOCL_EXPORTIMPORT std::vector<oocl::MessageBroker*>;

namespace oocl
{
	/**
	 * @brief	Manages the delivery of messages to all listeners who subscribed for it.
	 * 			
	 * @note	To subscribe for a specific message type call MessageBroker::getBrokerFor(messageType)->registerListener(this). 
	 * 			Now you will get all messages that are sent through MessageBroker::pumpMessage().
	 *
	 * @author	Jörn Teuber
	 * @date	22.02.2012
	 */
	class OOCL_EXPORTIMPORT MessageBroker : public Thread
	{
	public:
		static MessageBroker* getBrokerFor( unsigned short usMessageType );

		void registerListener( MessageListener* pListener );
		void unregisterListener( MessageListener* pListener );

		void pumpMessage( Message* pMessage );

		bool requestExclusiveMessaging( MessageListener* pListener );
		bool discardExclusiveMessaging( MessageListener* pListener );
		
		void enableContinuousProcessing();
		void disableContinuousProcessing();

		void enableSynchronousMessaging();
		void disableSynchronousMessaging();

	private:
		virtual void run();

		void deliverMessage( Message* pMessage );

		MessageBroker(void);
		~MessageBroker(void);

	private:
		MessageListener*	m_pExclusiveListener;
		std::list< MessageListener* > m_lListeners;
		std::queue< Message* > m_lMessageQueue;

		bool m_bRunThread;
		bool m_bRunContinuously;
		bool m_bSynchronous;

		Mutex m_mxQueue;
		Mutex m_mxListener;
		Mutex m_mxExclusiveListener;

		static std::map< unsigned int, MessageBroker* > sm_vBroker;
	};

}

#endif
