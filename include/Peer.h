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

#ifndef PEER_H
#define PEER_H

#include <time.h>
#include <stdlib.h>

#include "MessageBroker.h"
#include "ExplicitMessages.h"
#include "BerkeleySocket.h"
#include "Mutex.h"

// #define SIM_DELAY

namespace oocl
{

	/**
	 * @brief	typedef for the unique IDs of peers.
	 */
	typedef unsigned int PeerID;

	/**
	 * @brief	This class manages one peer in the peer 2 peer network.
	 * 			
	 * @note	To create a peer call Peer2PeerNetwork::addPeer(...).
	 *
	 * @author	Jörn Teuber
	 * @date	9.12.2011
	 */
	class OOCL_EXPORTIMPORT Peer : public MessageListener
	{
		friend class Peer2PeerNetwork;

	public:
		// will be send to the peer
		bool sendMessage( Message const * const pMessage );
		bool subscribe( unsigned short usType );

		// was sent by the peer
		bool receiveMessage( Message* pMessage );

		// getter
		bool	isConnected();
		PeerID	getPeerID();

		unsigned int	getIP();
		unsigned short 	getListeningPort();

		virtual bool cbMessage( Message const * const pMessage );

	private:
		Peer( std::string strHostname, unsigned short usPeerPort );
		Peer( unsigned int uiIP, unsigned short usPeerPort );
		virtual ~Peer(void);
		
		bool connect( unsigned short usListeningPort, PeerID uiUserID  );
		bool connected( Socket* pTCPSocket, ConnectMessage* pMsg, unsigned short usListeningPort, PeerID uiUserID );
		bool disconnect( bool bSendMessage = true );
		bool connectSockets();

		void deactivate();

#ifdef SIM_DELAY
		class MessageDelayer : public Thread
		{
		public:
			MessageDelayer( Message* pMessage );

		protected:
			void run();

		private:
			Message* m_pMessage;
		};
#endif

	private:
		unsigned char m_ucConnectStatus;

		std::list<unsigned short> m_lusSubscribedMsgTypes;

		Socket* m_pSocketTCP;
		Socket* m_pSocketUDPOut;

		std::string		m_strHostname;
		unsigned int	m_uiIP;
		unsigned short	m_usPort;
		PeerID			m_uiPeerID; ///< the ID of the peer this object refers to
		PeerID			m_uiUserID; ///< the ID of the peer running this instance
		bool 			m_bActive;

		Mutex	m_mxSockets;

		static unsigned int sm_uiNumPeers;
	};

}

#endif // PEER_H
