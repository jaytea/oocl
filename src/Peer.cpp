#include "Peer.h"


namespace oocl
{

	Peer::Peer( std::string strHostname, unsigned short usPeerPort ) :
		m_uiIP( 0 ),
		m_uiPeerID( 0 ),
		m_strHostname( strHostname ),
		m_usPort( usPeerPort ),
		m_ucConnectStatus( 0 ),
		m_pSocketTCP( NULL ),
		m_pSocketUDPOut( NULL )
	{
	}
		
	Peer::Peer( unsigned int uiIP, unsigned short usPeerPort ) :
		m_uiIP( uiIP ),
		m_uiPeerID( 0 ),
		m_strHostname(),
		m_usPort( usPeerPort ),
		m_ucConnectStatus( 0 ),
		m_pSocketTCP( NULL ),
		m_pSocketUDPOut( NULL )
	{
	}

	Peer::~Peer(void)
	{
		if( m_ucConnectStatus > 0 )
			disconnect();

		if( m_pSocketTCP )
			delete m_pSocketTCP;
		if( m_pSocketUDPOut )
			delete m_pSocketUDPOut;
	}

	bool Peer::connect( unsigned short usListeningPort, unsigned int uiPeerID )
	{
		if( m_ucConnectStatus == 0 )
		{
			m_pSocketTCP = new Socket( SOCK_STREAM );
			m_pSocketUDPOut = new Socket( SOCK_DGRAM );

			bool bUDPConnected = false, bTCPConnected = false;

			if( m_uiIP != 0 )
			{
				bUDPConnected = m_pSocketUDPOut->connect( m_uiIP, m_usPort );
				bTCPConnected = m_pSocketTCP->connect( m_uiIP, m_usPort );
			}
			else
			{
				bUDPConnected = m_pSocketUDPOut->connect( m_strHostname, m_usPort );
				bTCPConnected = m_pSocketTCP->connect( m_strHostname, m_usPort );
			}
			
			if( !bUDPConnected || !bTCPConnected )
			{
				delete m_pSocketTCP;
				delete m_pSocketUDPOut;

				return false;
			}

			ConnectMessage* pMsg = new ConnectMessage( usListeningPort, uiPeerID );

			// try to contact the peer twice
			if( !m_pSocketTCP->write( pMsg->getMsgString() ) )
			{
				if( !m_pSocketTCP->write( pMsg->getMsgString() ) )
				{
					delete m_pSocketTCP;
					delete m_pSocketUDPOut;

					return false;
				}
			}

			m_ucConnectStatus = 1;
			
			std::string strMsg = m_pSocketTCP->read( );
			Message* pMsg2 = Message::createFromString( strMsg.c_str() );

			if( pMsg2->getType() == MT_ConnectMessage )
			{
				m_uiPeerID = ((ConnectMessage*)pMsg2)->getPeerID();
				m_usPort = ((ConnectMessage*)pMsg2)->getPort();
				m_ucConnectStatus = 2;
			}
			else
			{
				Log::getLog("oocl")->logError( "The first message from a peer was not a ConnectMessage" );
			}

			return true;
		}

		Log::getLog("oocl")->logWarning( "You tried to connect to a peer that is already connected" );

		return false;
	}

	bool Peer::connected( Socket* pTCPSocket, ConnectMessage* pMsg, unsigned short usListeningPort, PeerID uiUserID )
	{
		if( m_ucConnectStatus == 0 )
		{
			if( !pTCPSocket->isValid() || !pTCPSocket->isConnected() ) // this would be very very strange, but better be safe
			{
				Log::getLog("oocl")->logError( "A peer connected but the tcp socket is not connected or invalid" );
				return false;
			}

			m_uiPeerID = pMsg->getPeerID();
			m_pSocketTCP = pTCPSocket;

			m_pSocketUDPOut = new Socket( SOCK_DGRAM );
			m_pSocketUDPOut->connect( m_uiIP, m_usPort );

			ConnectMessage* pMsg = new ConnectMessage( usListeningPort, uiUserID );

			// try to contact the peer twice
			if( !m_pSocketTCP->write( pMsg->getMsgString() ) )
			{
				if( !m_pSocketTCP->write( pMsg->getMsgString() ) )
				{
					delete m_pSocketTCP;
					delete m_pSocketUDPOut;

					return false;
				}
			}

			m_ucConnectStatus = 2;

			return true;
		}

		Log::getLog("oocl")->logWarning( "A peer tried to connect to you but you are already connected to each other" );

		return false;
	}
	
	bool Peer::disconnect()
	{
		if( m_ucConnectStatus > 0 )
		{
			sendMessage( new DisconnectMessage() );

			for( std::list<unsigned short>::iterator it = m_lusSubscribedMsgTypes.begin(); it != m_lusSubscribedMsgTypes.end(); it++ )
				MessageBroker::getBrokerFor( (*it) )->unregisterListener( this );

			m_pSocketTCP->close();
			delete m_pSocketTCP;
			m_pSocketTCP = NULL;

			m_pSocketUDPOut->close();
			delete m_pSocketUDPOut;
			m_pSocketUDPOut = NULL;

			m_ucConnectStatus = 0;

			return true;
		}

		return false;
	}

	bool Peer::cbMessage( Message* pMessage )
	{
		sendMessage( pMessage );

		return true;
	}

	bool Peer::sendMessage( Message* pMessage )
	{
		if( m_ucConnectStatus == 2 )
		{
			if( m_uiPeerID>0 && pMessage->getSenderID() == m_uiPeerID )
			{
				Log::getLog("oocl")->logInfo("stopped message from beeing send back to the sender" );
				return true;
			}

			bool bReturn = false;
			if( pMessage->getProtocoll() == SOCK_DGRAM )
				bReturn = m_pSocketUDPOut->write( pMessage->getMsgString()+std::string( (char*)&m_uiPeerID, 4 ) );
			else if( pMessage->getProtocoll() == SOCK_STREAM )
				bReturn = m_pSocketTCP->write( pMessage->getMsgString() );
			else
				Log::getLog("oocl")->logWarning("You tried to send a message over network that was not intended for that" );

			return bReturn;
		}
		else if( m_ucConnectStatus == 1 )
		{
			Log::getLog("oocl")->logInfo( "Message was dropped because the peer is not yet fully connected" );
			return false;
		}
		else
		{
			Log::getLog("oocl")->logInfo( "Message was dropped because the peer is not connected" );
			return false;
		}
	}

	bool Peer::subscribe( unsigned short usType )
	{
		return sendMessage( new SubscribeMessage( usType ) );
	}

	bool Peer::receiveMessage( Message* pMessage )
	{
		pMessage->setSenderID( m_uiPeerID );

		switch( pMessage->getType() )
		{
		case MT_SubscribeMessage:
			{
				unsigned short usType = ((SubscribeMessage*)pMessage)->getTypeToSubscribe();
				MessageBroker::getBrokerFor( usType )->registerListener( this );
				m_lusSubscribedMsgTypes.push_back( usType );

				break;
			}
		case MT_DisconnectMessage:
			{
				m_pSocketTCP->close();
				delete m_pSocketTCP;
				m_pSocketTCP = NULL;

				m_pSocketUDPOut->close();
				delete m_pSocketUDPOut;
				m_pSocketUDPOut = NULL;

				m_ucConnectStatus = 0;
				// no break, let the client know when a peer disconnected
			}
		default:
			MessageBroker::getBrokerFor( pMessage->getType() )->pumpMessage( pMessage );
		}

		return true;
	}
}
