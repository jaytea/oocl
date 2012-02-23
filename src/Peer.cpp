#include "Peer.h"


namespace oocl
{

	Peer::Peer( std::string strHostname, unsigned short usPeerPort ) :
		m_uiIP( 0 ),
		m_uiPeerID( 0 ),
		m_strHostname( strHostname ),
		m_usPort( usPeerPort ),
		m_bConnected( false ),
		m_pSocketTCP( NULL ),
		m_pSocketUDPOut( NULL )
	{
	}
		
	Peer::Peer( unsigned int uiIP, unsigned short usPeerPort ) :
		m_uiIP( uiIP ),
		m_uiPeerID( 0 ),
		m_strHostname(),
		m_usPort( usPeerPort ),
		m_bConnected( false ),
		m_pSocketTCP( NULL ),
		m_pSocketUDPOut( NULL )
	{
	}

	Peer::~Peer(void)
	{
		for( std::list<unsigned short>::iterator it = m_lusSubscribedMsgTypes.begin(); it != m_lusSubscribedMsgTypes.end(); it++ )
			MessageBroker::getBrokerFor( (*it) )->unregisterListener( this );

		if( m_pSocketTCP )
			delete m_pSocketTCP;
		if( m_pSocketUDPOut )
			delete m_pSocketUDPOut;
	}

	bool Peer::connect( unsigned short usListeningPort, unsigned int uiPeerID )
	{
		if( !m_bConnected )
		{
			m_pSocketTCP = new Socket( SOCK_STREAM );
			m_pSocketUDPOut = new Socket( SOCK_DGRAM );

			if( m_uiIP )
			{
				if( !m_pSocketUDPOut->connect( m_uiIP, m_usPort ) || !m_pSocketTCP->connect( m_uiIP, m_usPort ) )
				{
					delete m_pSocketTCP;
					delete m_pSocketUDPOut;

					return false;
				}
			}
			else
			{
				if( !m_pSocketUDPOut->connect( m_strHostname, m_usPort ) || !m_pSocketTCP->connect( m_strHostname, m_usPort ) )
				{
					delete m_pSocketTCP;
					delete m_pSocketUDPOut;

					return false;
				}
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

			m_bConnected = true;

			return true;
		}

		return false;
	}

	bool Peer::createWithExistingSockets( Socket* pTCPSocket, Socket* pUDPSocket, PeerID peerID )
	{
		if( !m_bConnected && pTCPSocket->isConnected() && pUDPSocket->isConnected() )
		{
			m_uiIP = pTCPSocket->getConnectedIP();

			m_pSocketTCP = pTCPSocket;
			m_pSocketUDPOut = pUDPSocket;

			m_bConnected = true;
			m_uiPeerID = peerID;
			return true;
		}

		return false;
	}
	
	bool Peer::disconnect()
	{
		if( m_bConnected )
		{
			sendMessage( new DisconnectMessage() );

			m_pSocketTCP->close();
			delete m_pSocketTCP;
			m_pSocketTCP = NULL;

			m_pSocketUDPOut->close();
			delete m_pSocketUDPOut;
			m_pSocketUDPOut = NULL;

			m_bConnected = false;

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
		if( m_bConnected )
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

		Log::getLog("oocl")->logInfo( "Message was dropped because the peer is not connected" );

		return false;
	}

	bool Peer::subscribe( unsigned short usType )
	{
		SubscribeMessage* pMsg = new SubscribeMessage( usType );

		return sendMessage( pMsg );
	}

	bool Peer::receiveMessage( Message* pMessage )
	{
		pMessage->setSenderID( m_uiPeerID );

		switch( pMessage->getType() )
		{
		case MT_ConnectMessage:
			{
				m_uiPeerID = ((ConnectMessage*)pMessage)->getPeerID();
				break;
			}
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

				m_bConnected = false;
				break;
			}
		default:
			MessageBroker::getBrokerFor( pMessage->getType() )->pumpMessage( pMessage );
		}

		return true;
	}
}
