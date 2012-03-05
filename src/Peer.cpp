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

#include "Peer.h"

namespace oocl
{
	/**
	 * @fn	Peer::Peer( std::string strHostname, unsigned short usPeerPort )
	 *
	 * @brief	Constructor.
	 *
	 * @param	strHostname	The hostname.
	 * @param	usPeerPort 	The peer port.
	 */
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


	/**
	 * @fn	Peer::Peer( unsigned int uiIP, unsigned short usPeerPort )
	 *
	 * @brief	Constructor.
	 *
	 * @param	uiIP	  	The ip.
	 * @param	usPeerPort	The peer port.
	 */
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


	/**
	 * @fn	Peer::~Peer(void)
	 *
	 * @brief	Destructor.
	 */
	Peer::~Peer(void)
	{
		if( m_ucConnectStatus > 0 )
			disconnect();

		if( m_pSocketTCP )
			delete m_pSocketTCP;
		if( m_pSocketUDPOut )
			delete m_pSocketUDPOut;
	}


	/**
	 * @fn	bool Peer::connect( unsigned short usListeningPort, unsigned int uiPeerID )
	 *
	 * @brief	Connects.
	 *
	 * @param	usListeningPort	The listening port.
	 * @param	uiPeerID	   	Identifier for the peer.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
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


	/**
	 * @fn	bool Peer::connected( Socket* pTCPSocket, ConnectMessage* pMsg,
	 * 		unsigned short usListeningPort, PeerID uiUserID )
	 *
	 * @brief	Called when someone connects.
	 *
	 * @param [in]	pTCPSocket		The connected tcp socket.
	 * @param [in]	pMsg	  		The connect message sent by the connecting peer.
	 * @param	usListeningPort   	The listening port.
	 * @param	uiUserID		  	Identifier for the user.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
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


	/**
	 * @fn	bool Peer::disconnect()
	 *
	 * @brief	Disconnects this peer.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
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


	/**
	 * @fn	bool Peer::cbMessage( Message* pMessage )
	 *
	 * @brief	Gets the messages to which the peer has subscribed.
	 *
	 * @param [in]	pMessage	The incoming message.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Peer::cbMessage( Message* pMessage )
	{
		if( pMessage->isIncoming() )
		{
			Log::getLog("oocl")->logWarning("stopped incoming message from being sent back into the network");
			return true;
		}

		sendMessage( pMessage );

		return true;
	}


	/**
	 * @fn	bool Peer::sendMessage( Message* pMessage )
	 *
	 * @brief	Sends a message to the connected peer.
	 *
	 * @param [in]	pMessage	The message to send.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
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


	/**
	 * @fn	bool Peer::subscribe( unsigned short usType )
	 *
	 * @brief	Subscribe a message type at the connected peer.
	 *
	 * @param	usType	The type.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Peer::subscribe( unsigned short usType )
	{
		return sendMessage( new SubscribeMessage( usType ) );
	}


	/**
	 * @fn	bool Peer::receiveMessage( Message* pMessage )
	 *
	 * @brief	Called when a message of the connected peer was received.
	 *
	 * @param [in]	pMessage	The received message.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
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
