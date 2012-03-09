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

#include "Peer2PeerNetwork.h"

namespace oocl
{
	/**
	 * @fn	Peer2PeerNetwork::Peer2PeerNetwork( unsigned short usListeningPort,
	 * 		unsigned int uiPeerID )
	 *
	 * @brief	Constructor.
	 *
	 * @param	usListeningPort	The port on which we are listening for new messages.
	 * @param	uiUserID	   	Identifier of this Peer, i.e. our own peerID.
	 */
	Peer2PeerNetwork::Peer2PeerNetwork( unsigned short usListeningPort, unsigned int uiUserID ) :
		m_usListeningPort( usListeningPort ),
		m_uiUserID( uiUserID ),
		m_bActive( true ),
		m_pServerSocketUDP( NULL ),
		m_pServerSocketTCP( NULL )
	{
		// register all message types, that the p2p network needs
		SubscribeMessage::registerMsg();
		ConnectMessage::registerMsg();
		DisconnectMessage::registerMsg();
		NewPeerMessage::registerMsg();

		// start the thread that checks for new peers and receives messages from other peers
		start();
	}


	/**
	 * @fn	Peer2PeerNetwork::~Peer2PeerNetwork(void)
	 *
	 * @brief	Destructor.
	 */
	Peer2PeerNetwork::~Peer2PeerNetwork(void)
	{
		for( std::list<Peer*>::iterator it = m_lpPeers.begin(); it != m_lpPeers.end(); it++ )
		{
			(*it)->disconnect();
			delete (*it);
		}

		m_bActive = false;

		m_mapPeersByID.clear();
		m_lpPeers.clear();
		m_lpSocketsWithoutPeers.clear();

		Log::getLog("oocl")->flush();
	}


	/**
	 * @fn	Peer* Peer2PeerNetwork::addPeer( std::string strHostname, unsigned short usPeerPort )
	 *
	 * @brief	Adds a peer to the network.
	 *
	 * @param	strHostname	The hostname.
	 * @param	usPeerPort 	The port on which the peer listens.
	 *
	 * @return	null if it fails, else.
	 */
	Peer* Peer2PeerNetwork::addPeer( std::string strHostname, unsigned short usPeerPort )
	{
		Peer* pPeer = new Peer( strHostname, usPeerPort );

		if( !connectAndInsertPeer( pPeer ) )
			pPeer = NULL;

		return pPeer;
	}


	/**
	 * @fn	Peer* Peer2PeerNetwork::addPeer( unsigned int uiIP, unsigned short usPeerPort )
	 *
	 * @brief	Adds a peer to the network.
	 *
	 * @param	uiIP	  	The ip.
	 * @param	usPeerPort	The port on which the peer listens.
	 *
	 * @return	null if it fails, else.
	 */
	Peer* Peer2PeerNetwork::addPeer( unsigned int uiIP, unsigned short usPeerPort )
	{
		Peer* pPeer = new Peer( uiIP, usPeerPort );

		if( !connectAndInsertPeer( pPeer ) )
			pPeer = NULL;

		return pPeer;
	}


	/**
	 * @fn	bool Peer2PeerNetwork::connectAndInsertPeer( Peer* pPeer )
	 *
	 * @brief	Connects an and insert peer, used by the addPeer methods.
	 *
	 * @param [in]	pPeer	The peer.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Peer2PeerNetwork::connectAndInsertPeer( Peer* pPeer )
	{
		if( !pPeer->connect( m_usListeningPort, m_uiUserID ) )
			return false;

		m_lpPeers.push_back( pPeer );
		m_mapPeersByID.insert( std::pair<unsigned int, Peer*>( pPeer->getPeerID(), pPeer ) );

		MessageBroker::getBrokerFor( MT_NewPeerMessage )->pumpMessage( new NewPeerMessage( pPeer ) );

		return true;
	}


	/**
	 * @fn	Peer* Peer2PeerNetwork::getPeerByID( unsigned int uiPeerID )
	 *
	 * @brief	Gets a peer by identifier.
	 *
	 * @param	uiPeerID	Identifier for the peer.
	 *
	 * @return	null if it fails, else the peer.
	 */
	Peer* Peer2PeerNetwork::getPeerByID( unsigned int uiPeerID )
	{ 
		std::map<unsigned int, Peer*>::iterator it = m_mapPeersByID.find( uiPeerID );
		if( it != m_mapPeersByID.end() )
			return it->second;

		return NULL; 
	}


	/**
	 * @fn	void Peer2PeerNetwork::run()
	 *
	 * @brief	Thread for managing incoming messages and connections.
	 */
	void Peer2PeerNetwork::run()
	{
		// prepare the sockets for receiving
		m_pServerSocketTCP = new ServerSocket();
		m_pServerSocketUDP = new Socket( SOCK_DGRAM );

		m_pServerSocketTCP->bind( m_usListeningPort );
		m_pServerSocketUDP->bind( m_usListeningPort );

		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 500000;

		// start 
		while( m_bActive )
		{
			// TODO: instead of building the set every frame build it once and copy it every frame. Update the source set it whenever a peer is added

			// substitution of std::max / max as it is defined different in msvc and gnucc
			int iBiggestSocket = m_pServerSocketTCP->getCSocket();
			if( iBiggestSocket < m_pServerSocketUDP->getCSocket() )
				iBiggestSocket = m_pServerSocketUDP->getCSocket();
			
			// prepare the fd_set
			fd_set selectSet;
			FD_ZERO( &selectSet );
			
			// add UDP and TCP listening sockets to the set
			FD_SET( m_pServerSocketTCP->getCSocket(), &selectSet );
			FD_SET( m_pServerSocketUDP->getCSocket(), &selectSet );

			// add all peer tcp sockets to the set
			for( std::list<Peer*>::iterator it = m_lpPeers.begin(); it != m_lpPeers.end(); it++ )
			{
				if( (*it)->isConnected() )
				{
					int iTCP = (*it)->m_pSocketTCP->getCSocket();
					if( iTCP > iBiggestSocket )
						iBiggestSocket = iTCP;
					FD_SET( iTCP, &selectSet );
				}
			}

			// add all tcp sockets to the set that have not yet been assigned to a peer
			for( std::list<Socket*>::iterator it = m_lpSocketsWithoutPeers.begin(); it != m_lpSocketsWithoutPeers.end(); it++ )
			{
				if( (*it)->isConnected() )
				{
					if( (*it)->getCSocket() > iBiggestSocket )
						iBiggestSocket = (*it)->getCSocket();
					FD_SET( (*it)->getCSocket(), &selectSet );
				}
			}

			// select!
			int iRet = select( iBiggestSocket, &selectSet, NULL, NULL, &tv );
#ifdef _MSC_VER
			if( iRet == SOCKET_ERROR )
#else	
			if( iRet < 0 )
#endif
			{
				Log::getLog("oocl")->logError( "Selecting in the peernetwork failed" );
			}

			// if the tcp server socket got a connection push the tcp socket on the list
			if( FD_ISSET( m_pServerSocketTCP->getCSocket(), &selectSet ) )
			{
				Socket* pSocket = m_pServerSocketTCP->accept();
				if( pSocket )
					m_lpSocketsWithoutPeers.push_back( pSocket );
			}
			
			// messages from the udp receiving socket will be pushed on the appropriate MessageBroker
			if( FD_ISSET( m_pServerSocketUDP->getCSocket(), &selectSet ) )
			{
				std::string strMsg = m_pServerSocketUDP->read();
				if( strMsg.empty() )
				{
					Log::getLog("oocl")->logWarning( "a message from a peer on udp could not be received" );
				}
				else
				{
					// messages that come from other peers have to have the sender peerID at the end of the message
					unsigned int uiPeerID = *((unsigned int*)strMsg.substr( strMsg.length()-4 ).c_str());
					Message* pMsg = Message::createFromString( strMsg.substr(0,strMsg.length()-4).c_str() );

					if( getPeerByID( uiPeerID ) )
						getPeerByID( uiPeerID )->receiveMessage( pMsg );
				}
			}

			// walk through the peer list and check for messages
			std::list<Peer*>::iterator itPeers = m_lpPeers.begin();
			while( itPeers != m_lpPeers.end() )
			{
				bool bPeerDisconnected = false;
				Peer* pPeer = *itPeers; // TODO: this looks like shit

				if( FD_ISSET( pPeer->m_pSocketTCP->getCSocket(), &selectSet ) )
				{
					std::string strMsg = pPeer->m_pSocketTCP->read();
					while( !strMsg.empty() )
					{
						Message* pMsg = Message::createFromString( strMsg.c_str() );
						pPeer->receiveMessage( pMsg );
						
						// some messages need special attention here
						if( pMsg->getType() == MT_DisconnectMessage ) // remove the peer from the lists when he disconnected
						{
							m_mapPeersByID.erase( pPeer->getPeerID() );
							itPeers = m_lpPeers.erase( itPeers );
							bPeerDisconnected = true;
							break;
						}

						strMsg = strMsg.substr( pMsg->getBodyLength() +4 );
					}

					if( bPeerDisconnected )
						continue;
				}

				itPeers++;
			}

			// check for connect messages on the sockets without peers
			std::list<Socket*>::iterator it = m_lpSocketsWithoutPeers.begin();
			while(  it != m_lpSocketsWithoutPeers.end() )
			{
				if( FD_ISSET( (*it)->getCSocket(), &selectSet ) )
				{
					std::string strMsg = (*it)->read( );
					Message* pMsg = Message::createFromString( strMsg.c_str() );

					if( pMsg->getType() == MT_ConnectMessage )
					{
						Peer* pPeer = new Peer( (*it)->getConnectedIP(), ((ConnectMessage*)pMsg)->getPort() );
						pPeer->connected( (*it), (ConnectMessage*)pMsg, m_usListeningPort, m_uiUserID );

						m_lpPeers.push_back( pPeer );
						m_mapPeersByID.insert( std::pair<PeerID,Peer*>( pPeer->getPeerID(), pPeer ) );

						MessageBroker::getBrokerFor( MT_NewPeerMessage )->pumpMessage( new NewPeerMessage( pPeer ) );

						it = m_lpSocketsWithoutPeers.erase( it );
						continue;
					}
					else
					{
						Log::getLog("oocl")->logInfo("A message from an unknown peer was received" );
					}
				}

				it++;
			}
		}
	}
}