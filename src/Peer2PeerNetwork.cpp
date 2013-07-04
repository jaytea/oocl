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
	 * @brief	Constructor.
	 *
	 * @param	usListeningPort	The port on which we are listening for new messages.
	 * @param	uiUserID	   	Identifier of this Peer, i.e. our own peerID.
	 */
	Peer2PeerNetwork::Peer2PeerNetwork( unsigned short usListeningPort, unsigned int uiUserID )
		: m_pServerSocketUDP( NULL )
		, m_pServerSocketTCP( NULL )
		, m_bActive( true )
		, m_usListeningPort( usListeningPort )
		, m_uiUserID( uiUserID )
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
	 * @brief	Destructor.
	 */
	Peer2PeerNetwork::~Peer2PeerNetwork(void)
	{
		m_bActive = false;
		join();

		disconnect();

		for( std::list<Peer*>::iterator it = m_lpPeers.begin(); it != m_lpPeers.end(); ++it )
			delete (*it);

		m_lpPeers.clear();
		m_lpSocketsWithoutPeers.clear();
	}


	/**
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
	 * @brief	Disconnects a peer and removes it from the network.
	 *
	 * @param [in]	uiPeerID	The PeerID of the peer to remove.
	 */
	void Peer2PeerNetwork::subPeer( PeerID uiPeerID )
	{
		Peer* pPeer = getPeerByID( uiPeerID );
		if( pPeer != NULL )
		{
			m_mapPeersByID.erase( uiPeerID );
			m_lpPeers.remove( pPeer );

			delete pPeer;
		}
	}


	/**
	 * @brief	Disconnects from all peers currently in the network and stops accepting incoming messages.
	 */
	void Peer2PeerNetwork::disconnect()
	{
		m_mxPeers.lock();

		for( std::list<Peer*>::iterator it = m_lpPeers.begin(); it != m_lpPeers.end(); ++it )
		{
			if( (*it) != NULL )
			{
				delete (*it);
				(*it) = NULL;
			}
		}

		m_mapPeersByID.clear();
		m_lpPeers.clear();

		m_mxPeers.unlock();
	}


	/**
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

		m_mxPeers.lock();

		m_lpPeers.push_back( pPeer );
		m_mapPeersByID.insert( std::pair<unsigned int, Peer*>( pPeer->getPeerID(), pPeer ) );

		m_mxPeers.unlock();

		MessageBroker::getBrokerFor( MT_NewPeerMessage )->pumpMessage( new NewPeerMessage( pPeer ) );

		return true;
	}


	/**
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
	 * @brief	Returns the IP of the Peer identified by the given ID.
	 *
	 * @param	uiPeerID	Identifier for the peer.
	 *
	 * @return	null if there is no such peer or the peer is not connected, else the peers IP.
	 */
	unsigned int Peer2PeerNetwork::getPeersIP( PeerID uiPeerID )
	{
		Peer* p = getPeerByID( uiPeerID );
		if( p != NULL )
			return p->getIP();

		return 0;
	}


	/**
	 * @brief	Returns the list of connected peers.
	 *
	 * @return	pointer to the list of connected peers.
	 */
	std::list<Peer*>* Peer2PeerNetwork::getPeerList()
	{
		return &m_lpPeers;
	}


	/**
	 * @brief	Returns the PeerID of this user.
	 *
	 * @return	The PeerID of this user.
	 */
	unsigned int Peer2PeerNetwork::getUserID()
	{
		return m_uiUserID;
	}


	/**
	 * @brief	Thread for managing incoming messages and connections.
	 */
	void Peer2PeerNetwork::run()
	{
		// set the MessageBroker for the disconnect messages to synchronous mode as the client might need the time to process on the peer
		MessageBroker::getBrokerFor( MT_DisconnectMessage )->enableSynchronousMessaging();

		// prepare the sockets for receiving
		m_pServerSocketTCP = new ServerSocket();
		m_pServerSocketUDP = new BerkeleySocket( SOCK_DGRAM );

		m_pServerSocketTCP->bind( m_usListeningPort );
		m_pServerSocketUDP->bind( m_usListeningPort );

		fd_set selectSet;

		// start 
		while( m_bActive )
		{
			// substitution of std::max / max as it is defined different in msvc and gnucc
			int iBiggestSocket = m_pServerSocketTCP->getCSocket();
			if( iBiggestSocket < m_pServerSocketUDP->getCSocket() )
				iBiggestSocket = m_pServerSocketUDP->getCSocket();
			
			// prepare the fd_set
			FD_ZERO( &selectSet );
			
			// add UDP and TCP listening sockets to the set
			FD_SET( m_pServerSocketTCP->getCSocket(), &selectSet );
			FD_SET( m_pServerSocketUDP->getCSocket(), &selectSet );

			m_mxPeers.lock();
			// add all peer tcp sockets to the set
			{
				std::list<Peer*>::iterator it = m_lpPeers.begin();
				while( it != m_lpPeers.end() )
				{
					if( (*it)->isConnected() && (*it)->m_pSocketTCP != NULL )
					{
						int iTCP = (*it)->m_pSocketTCP->getCSocket();
						if( iTCP > iBiggestSocket )
							iBiggestSocket = iTCP;
						FD_SET( iTCP, &selectSet );
					}
					else
					{
						delete (*it);
						it = m_lpPeers.erase( it );
					}

					++it;
				}
			}
			m_mxPeers.unlock();

			// add all tcp sockets to the set that have not yet been assigned to a peer
			{
				std::list<Socket*>::iterator it = m_lpSocketsWithoutPeers.begin();
				while( it != m_lpSocketsWithoutPeers.end() )
				{
					if( (*it)->isConnected() )
					{
						if( (*it)->getCSocket() > iBiggestSocket )
							iBiggestSocket = (*it)->getCSocket();
						FD_SET( (*it)->getCSocket(), &selectSet );
					}
					else
					{
						delete (*it);
						it = m_lpSocketsWithoutPeers.erase( it );
					}

					++it;
				}
			}

			timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 500000;

			// select!
			int iRet = select( iBiggestSocket+1, &selectSet, NULL, NULL, &tv );
#ifdef _MSC_VER
			if( iRet == SOCKET_ERROR )
#else	
			if( iRet < 0 )
#endif
			{
				Log::getLog("oocl")->logError( "Selecting in the peernetwork failed" );
				continue;
			}
			else if( iRet == 0 )
				continue;
			
			// messages from the udp receiving socket will be pushed on the appropriate MessageBroker
			if( FD_ISSET( m_pServerSocketUDP->getCSocket(), &selectSet ) )
			{
				std::string strMsg;
				if( !m_pServerSocketUDP->read( strMsg ) )
				{
					Log::getLog("oocl")->logWarning( "a message from a peer on udp could not be received" );
				}
				else
				{
					// messages that come from other peers must have the sender peerID at the end of the message
					unsigned int uiPeerID = *((unsigned int*)strMsg.substr( strMsg.length()-4 ).c_str());
					Message* pMsg = Message::createFromString( strMsg.substr(0,strMsg.length()-4).c_str() );

					Peer* pPeer = getPeerByID( uiPeerID );
					if( pPeer != NULL && pPeer->isConnected() )
						pPeer->receiveMessage( pMsg );
					else
						Log::getLog("oocl")->logWarning( "received udp-message from a no longer connected peer" );
				}
			}

			m_mxPeers.lock();

			// walk through the peer list and check for messages
			std::list<Peer*>::iterator itPeers = m_lpPeers.begin();
			while( itPeers != m_lpPeers.end() )
			{
				bool bPeerDisconnected = false;
				Peer* pPeer = *itPeers;

				if( pPeer->isConnected() && FD_ISSET( pPeer->m_pSocketTCP->getCSocket(), &selectSet ) )
				{
					std::string strMsg;
					if( !pPeer->m_pSocketTCP->read( strMsg ) )
					{
						if( !pPeer->isConnected() && !pPeer->connectSockets() )
						{
							Log::getLogRef("oocl") << Log::EL_WARNING << "removed peer " << pPeer->getPeerID() << " after error on socket" << oocl::endl;
							delete pPeer;

							itPeers = m_lpPeers.erase( itPeers );
							continue;
						}
					}

					while( !strMsg.empty() )
					{
//						Log::getLogRef("oocl") << Log::EL_INFO << "received message from peer " << pPeer->getPeerID() << oocl::endl;
						Message* pMsg = Message::createFromString( strMsg.c_str() );
						if( pMsg == NULL )
						{
							Log::getLogRef("oocl") << Log::EL_WARNING << "reading incoming message from peer " << pPeer->getPeerID() << " failed" << oocl::endl;
							break;
						}

						if( strMsg.length() < pMsg->getBodyLength() + 4 )
						{
							Log::getLogRef("oocl") << Log::EL_WARNING << "read " << (int)strMsg.length() << " bytes while expected message is " << pMsg->getBodyLength() + 4 << " bytes long" << oocl::endl;
							std::string strMsg2;
							pPeer->m_pSocketTCP->read( strMsg2 );
							strMsg += strMsg2;
						}
						else
						{
							strMsg = strMsg.substr( pMsg->getBodyLength() +4 );

							pPeer->receiveMessage( pMsg );
						}

						// some messages need special attention here
						if( pMsg->getType() == MT_DisconnectMessage ) // remove the peer from the lists when he disconnected
						{
							m_mapPeersByID.erase( pPeer->getPeerID() );
							itPeers = m_lpPeers.erase( itPeers );

							bPeerDisconnected = true;
							break;
						}
					}
				}

				if( !bPeerDisconnected )
					++itPeers;
			}

			m_mxPeers.unlock();

			// check for connect messages on the sockets without peers
			std::list<Socket*>::iterator it = m_lpSocketsWithoutPeers.begin();
			while(  it != m_lpSocketsWithoutPeers.end() )
			{
				if( FD_ISSET( (*it)->getCSocket(), &selectSet ) )
				{
					std::string strMsg;
					if( (*it)->read( strMsg ) )
					{
						Message* pMsg = Message::createFromString( strMsg.c_str() );
						if( pMsg != NULL )
						{
							if( strMsg.length() > pMsg->getBodyLength() + 4 )
								Log::getLog("oocl")->logError("ConnectMessage too long" );

							if( pMsg->getType() == MT_ConnectMessage )
							{
								Log::getLog("oocl")->logInfo( "received connect message" );

								Peer* pPeer = new Peer( (*it)->getConnectedIP(), ((ConnectMessage*)pMsg)->getPort() );
								pPeer->connected( (*it), (ConnectMessage*)pMsg, m_usListeningPort, m_uiUserID );

								m_lpPeers.push_back( pPeer );
								m_mapPeersByID.insert( std::pair<PeerID,Peer*>( pPeer->getPeerID(), pPeer ) );

								Message* pMsg = new NewPeerMessage( pPeer );
								pMsg->setSenderID( pPeer->getPeerID() );
								MessageBroker::getBrokerFor( MT_NewPeerMessage )->pumpMessage( pMsg );

								it = m_lpSocketsWithoutPeers.erase( it );
								continue;
							}
							else
							{
								Log::getLog("oocl")->logInfo("A message from an unknown peer was received" );
							}
						}
					}

				}

				++it;
			}

			// if the tcp server socket got a connection, push the tcp socket on the list
			if( FD_ISSET( m_pServerSocketTCP->getCSocket(), &selectSet ) )
			{
				Log::getLog("oocl")->logInfo( "new half connection" );
				Socket* pSocket = m_pServerSocketTCP->accept();

				if( pSocket )
					m_lpSocketsWithoutPeers.push_back( pSocket );
			}
		}
	}
}
