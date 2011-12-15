#include "Peer.h"


namespace oocl
{

	Peer::Peer( unsigned int uiIP, unsigned short usPort ) :
		m_uiIP( uiIP ),
		m_usPort( usPort ),
		m_bConnected( false ),
		m_pTCPSocket( NULL ),
		m_pUDPSocket( NULL )
	{
	}

	Peer::~Peer(void)
	{
		for( std::list<unsigned short>::iterator it = m_lusSubscribedMsgTypes.begin(); it != m_lusSubscribedMsgTypes.end(); it++ )
			MessageBroker::getBrokerFor( (*it) )->unregisterListener( this );

		if( m_pTCPSocket )
			delete m_pTCPSocket;
		if( m_pUDPSocket )
			delete m_pUDPSocket;
	}

	bool Peer::connect( unsigned short usMyPort )
	{
		m_pTCPSocket = new Socket( SOCK_STREAM );
		m_pUDPSocket = new Socket( SOCK_DGRAM );

		if( !m_pUDPSocket->connect( m_uiIP, m_usPort ) || !m_pTCPSocket->connect( m_uiIP, m_usPort ) )
		{
			delete m_pTCPSocket;
			delete m_pUDPSocket;

			return false;
		}

		ConnectMessage* pMsg = new ConnectMessage( usMyPort );

		// try to contact the peer twice
		if( !m_pTCPSocket->write( pMsg->getMsgString() ) )
		{
			if( !m_pTCPSocket->write( pMsg->getMsgString() ) )
			{
				delete m_pTCPSocket;
				delete m_pUDPSocket;

				return false;
			}
		}

		m_bConnected = true;

		return true;
	}

	bool Peer::setExistingSockets( Socket* pTCPSocket, Socket* pUDPSocket )
	{
		if( pTCPSocket->isConnected() && pUDPSocket->isConnected() )
		{
			m_pTCPSocket = pTCPSocket;
			m_pUDPSocket = pUDPSocket;

			m_bConnected = true;
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
			bool bReturn;
			if( pMessage->getProtocoll() == SOCK_DGRAM )
				bReturn = m_pUDPSocket->write( pMessage->getMsgString() );
			else
				bReturn = m_pTCPSocket->write( pMessage->getMsgString() );

			return bReturn;
		}

		Log::getLog("oocl")->logMessage( "Message was dropped because there is no connection to the peer", Log::EL_INFO );

		return false;
	}

	bool Peer::subscribe( unsigned short usType )
	{
		SubscribeMessage* pMsg = new SubscribeMessage( usType );

		return sendMessage( pMsg );
	}

	bool Peer::receiveMessage( Message* pMessage )
	{
		switch( pMessage->getProtocoll() )
		{
		case MT_SubscribeMessage:
			{
				unsigned short usType = ((SubscribeMessage*)pMessage)->getTypeToSubscribe();
				if( MessageBroker::getBrokerFor( usType )->registerListener( this ) )
					m_lusSubscribedMsgTypes.push_back( usType );

				break;
			}
		case MT_DisconnectMessage:
			{

			}
		}

		return true;
	}
}
