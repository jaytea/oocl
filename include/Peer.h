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

#ifndef PEER
#define PEER

#include <time.h>
#include <stdlib.h>

#include "MessageBroker.h"
#include "ExplicitMessages.h"
#include "Socket.h"

namespace oocl
{
	/**
	 * @class	Peer
	 *
	 * @brief	this class manages one peer in the peer 2 peer network.
	 * 			
	 * 
	 *
	 * @author	Jörn Teuber
	 * @date	12/9/2011
	 */
	class OOCL_EXPORTIMPORT Peer : public MessageListener
	{
		friend class Peer2PeerNetwork;

	public:
		Peer( unsigned int uiIP, unsigned short usPort );
		virtual ~Peer(void);

		bool connect( unsigned short usMyPort );
		bool setExistingSockets( Socket* pTCPSocket, Socket* pUDPSocket );

		virtual bool cbMessage( Message* pMessage );

		// will be send to the peer
		bool sendMessage( Message* pMessage );
		bool subscribe( unsigned short usType );

		// was sent by the peer
		bool receiveMessage( Message* pMessage );

		bool isConnected() { return m_bConnected; }

	private:
		bool m_bConnected;

		std::list<unsigned short> m_lusSubscribedMsgTypes;

		Socket* m_pTCPSocket;
		Socket* m_pUDPSocket;

		unsigned int	m_uiIP;
		unsigned short	m_usPort;
	};

}

#endif // PEER