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

#ifndef MESSAGEBROKER_H_INCLUDED
#define MESSAGEBROKER_H_INCLUDED

#include <map>
#include <list>

#include "oocl_import_export.h"

#include "Thread.h"
#include "MessageListener.h"

namespace oocl
{

	class OOCL_EXPORTIMPORT MessageBroker : public Thread
	{
	public:
		static MessageBroker* getBrokerFor( unsigned short usMessageType );

		bool registerListener( MessageListener* pListener );
		bool unregisterListener( MessageListener* pListener );

		bool pumpMessage( Message* pMessage );

		bool requestExclusiveMessaging( MessageListener* pListener );
		bool discardExclusiveMessaging( MessageListener* pListener );

	private:
		virtual void run();

		MessageBroker(void);
		~MessageBroker(void);

	private:
		unsigned int m_iSequenceNumber;
		MessageListener*	m_pExclusiveListener;
		std::list< MessageListener* > m_lListeners;
		std::list< Message* > m_lMessageQueue;

		bool m_bRunThread;

		static std::map< unsigned short, MessageBroker* > sm_mapBroker;
	};

}

#endif