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
/// This file was written by Jörn Teuber

#ifndef DIRECTCONNETWORK_H
#define DIRECTCONNETWORK_H

#include <list>

#include "oocl_import_export.h"

#include "MessageListener.h"
#include "Socket.h"
#include "Thread.h"

namespace oocl
{
	/// class for connecting with one other process and sending and receiving messages.
	/**
	 * this can be the client side of a client-server-network
	 */
	class OOCL_EXPORTIMPORT DirectConNetwork : public Thread
	{
	public:
		DirectConNetwork();
		~DirectConNetwork();

		bool connect( std::string strHostname, unsigned short usPort, int iProtocoll = 0 );
		bool disconnect();

		bool sendMsg( Message* pMessage );

		bool registerListener( MessageListener* pListener );
		bool unregisterListener( MessageListener* pListener );

		// getter
		bool isConnected() { return m_bConnected; }

	private:
		Socket* m_pUDPSocket;
		Socket* m_pTCPSocket;
		Socket* m_pStdSocket;

		int     m_iStdProtocoll;
		std::list<MessageListener*> m_lListeners;

		bool m_bConnected;
	};

}

#endif // DIRECTCONNETWORK_H
