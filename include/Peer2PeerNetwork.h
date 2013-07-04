/*
Object Oriented Communication Library
Copyright (c) 2011 J��rgen Lorenz and J��rn Teuber

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

#ifndef PEER2PEERNETWORK_H
#define PEER2PEERNETWORK_H

#include <list>
#include <map>

#include "Peer.h"
#include "Thread.h"
#include "MessageBroker.h"
#include "ExplicitMessages.h"
#include "ServerSocket.h"

namespace oocl
{
	/**
	 * @class	Peer2PeerNetwork
	 *
	 * @brief	Manager class for a message based peer2peer network.
	 * 			
	 * @note	sends message: NewPeerMessage
	 *
	 * @author	Jörn Teuber
	 * @date	12/8/2011
	 */
	class OOCL_EXPORTIMPORT Peer2PeerNetwork : public Thread
	{
	public:
		Peer2PeerNetwork( unsigned short usListeningPort, PeerID uiPeerID ); 
		virtual ~Peer2PeerNetwork(void);

		Peer* addPeer( std::string strHostname, unsigned short usPeerPort );
		Peer* addPeer( unsigned int uiIP, unsigned short usPeerPort );

		void subPeer( PeerID uiPeerID );
		void disconnect();

		// getter
		Peer*				getPeerByID( PeerID uiPeerID );
		unsigned int		getPeersIP( PeerID uiPeerID );
		std::list<Peer*>*	getPeerList();
		unsigned int		getUserID();
		
	protected:
		virtual void run();

	private:
		bool connectAndInsertPeer( Peer* pPeer );

	private:
		std::list<Peer*> m_lpPeers;
		std::map<PeerID, Peer*> m_mapPeersByID;
		std::list<Socket*> m_lpSocketsWithoutPeers;

		Socket*			m_pServerSocketUDP;
		ServerSocket*	m_pServerSocketTCP;

		Mutex		m_mxPeers;

		bool			m_bActive;
		unsigned short	m_usListeningPort;
		PeerID			m_uiUserID;
	};

}

#endif // PEER2PEERNETWORK_H
