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

#include "Peer2PeerNetwork.h"

namespace oocl
{

	Peer2PeerNetwork::Peer2PeerNetwork( unsigned short usListeningPort ) :
		m_usListeningPort( usListeningPort ),
		m_bActive( true )
	{
		SubscribeMessage::registerMsg();
		ConnectMessage::registerMsg();
		DisconnectMessage::registerMsg();
		NewPeerMessage::registerMsg();

		start();
	}


	Peer2PeerNetwork::~Peer2PeerNetwork(void)
	{
		m_bActive = false;
	}


	bool Peer2PeerNetwork::addPeer( Peer* pPeer )
	{
		if( pPeer )
		{
			if( !pPeer->isConnected() )
				if( !pPeer->connect( m_usListeningPort ) )
					return false;

			m_lpPeers.push_back( pPeer );

			return true;
		}

		return false;
	}
		

	void Peer2PeerNetwork::run()
	{
		ServerSocket* pTCPServerSock = new ServerSocket();

		pTCPServerSock->bind( m_usListeningPort );

		while( m_bActive )
		{
			int iBiggestSocket = pTCPServerSock->getCSocket();
			fd_set selectSet;
			FD_ZERO( &selectSet );

			FD_SET( pTCPServerSock->getCSocket(), &selectSet );

			for( std::list<Peer*>::iterator it = m_lpPeers.begin(); it != m_lpPeers.end(); it++ )
			{
				if( (*it)->isConnected() )
				{
					int iTCP = (*it)->m_pTCPSocket->getCSocket();
					if( iTCP > iBiggestSocket )
						iBiggestSocket = iTCP;
					FD_SET( iTCP, &selectSet );

					int iUDP = (*it)->m_pUDPSocket->getCSocket();
					if( iUDP > iBiggestSocket )
						iBiggestSocket = iUDP;
					FD_SET( iUDP, &selectSet );
				}
			}

			for( std::list<Socket*>::iterator it = m_lpSocketsWithoutPeers.begin(); it != m_lpSocketsWithoutPeers.end(); it++ )
			{
				if( (*it)->isConnected() )
				{
					if( (*it)->getCSocket() > iBiggestSocket )
						iBiggestSocket = (*it)->getCSocket();
					FD_SET( (*it)->getCSocket(), &selectSet );
				}
			}

			timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 500000;

			int iRet = select( iBiggestSocket, &selectSet, NULL, NULL, &tv );
			if( iRet == SOCKET_ERROR )
			{
				Log::getLog("oocl")->logMessage( "Selecting in the peernetwork failed", Log::EL_ERROR );
			}

			if( FD_ISSET( pTCPServerSock->getCSocket(), &selectSet ) )
			{
				m_lpSocketsWithoutPeers.push_back( pTCPServerSock->accept() );
			}

			for( std::list<Peer*>::iterator it = m_lpPeers.begin(); it != m_lpPeers.end(); it++ )
			{
				if( FD_ISSET( (*it)->m_pTCPSocket->getCSocket(), &selectSet ) )
				{
					std::string strHeader = (*it)->m_pTCPSocket->read( 4 );

					unsigned short usMsgLength = ((unsigned short*)strHeader.c_str())[1];
					strHeader.append( (*it)->m_pTCPSocket->read( usMsgLength ) );

					Message* pMsg = Message::createFromString( strHeader.c_str() );
					(*it)->receiveMessage( pMsg );
				}
				
				if( FD_ISSET( (*it)->m_pUDPSocket->getCSocket(), &selectSet ) )
				{
					std::string strHeader = (*it)->m_pUDPSocket->read( 4 );

					unsigned short usMsgLength = ((unsigned short*)strHeader.c_str())[1];
					strHeader.append( (*it)->m_pUDPSocket->read( usMsgLength ) );

					Message* pMsg = Message::createFromString( strHeader.c_str() );
					(*it)->receiveMessage( pMsg );
				}
			}
			
			for( std::list<Socket*>::iterator it = m_lpSocketsWithoutPeers.begin(); it != m_lpSocketsWithoutPeers.end(); it++ )
			{
				if( FD_ISSET( (*it)->getCSocket(), &selectSet ) )
				{
					std::string strHeader = (*it)->read( 4 );

					unsigned short usMsgLength = ((unsigned short*)strHeader.c_str())[1];
					strHeader.append( (*it)->read( usMsgLength ) );

					Message* pMsg = Message::createFromString( strHeader.c_str() );

					if( ((unsigned short*)strHeader.c_str())[1] == MT_ConnectMessage )
					{
						unsigned short usPort = ((ConnectMessage*)pMsg)->getProtocoll();

						Socket* pUDPSocket = new Socket( SOCK_DGRAM );
						pUDPSocket->connect( (*it)->getConnectedIP(), usPort );

						Peer* pPeer = new Peer( (*it)->getConnectedIP(), usPort );
						pPeer->setExistingSockets( (*it), pUDPSocket );

						addPeer( pPeer );
						MessageBroker::getBrokerFor( MT_NewPeerMessage )->pumpMessage( new NewPeerMessage( pPeer ) );
						m_lpSocketsWithoutPeers.erase( it );
					}
					else
					{
						Log::getLog("oocl")->logMessage("A message from an unknown peer was received", Log::EL_INFO );
					}
				}
			}
		}
	}
}