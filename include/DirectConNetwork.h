/*
Object Oriented Communication Library
Copyright (c) 2011 J�rgen Lorenz and J�rn Teuber

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
/// This file was written by J�rn Teuber

#ifndef DIRECTCONNETWORK_H
#define DIRECTCONNETWORK_H

#include <list>

#include "oocl_import_export.h"

#include "MessageListener.h"
#include "ServerSocket.h"
#include "Socket.h"
#include "Thread.h"
#include "ExplicitMessages.h"

namespace oocl
{
	/**
	 * @class	DirectConNetwork
	 *
	 * @brief	class for connecting with one other process and sending and receiving messages.
	 *
	 * @author	J�rn Teuber
	 * @date	9/14/2011
	 */
	class OOCL_EXPORTIMPORT DirectConNetwork : public Thread
	{
	public:
		DirectConNetwork();
		~DirectConNetwork();

		bool connect( std::string strHostname, unsigned short usPort, int iProtocoll );
		bool listen( unsigned short usPort, int iProtocoll );
		bool disconnect();

		bool sendMsg( Message* pMessage );

		bool registerListener( MessageListener* pListener );
		bool unregisterListener( MessageListener* pListener );

		// getter
		bool isConnected() { return m_bConnected; }

	protected:
		virtual void run();

	private:
		Socket* m_pSocketIn;
		Socket* m_pSocketOut;
		ServerSocket* m_pServerSocket;

		int     m_iProtocoll;
		unsigned short	m_usPort;
		std::list<MessageListener*> m_lListeners;

		bool m_bConnected;
	};

}

#endif // DIRECTCONNETWORK_H
