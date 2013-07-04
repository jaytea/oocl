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
	unsigned int Peer::sm_uiNumPeers = 0;

	/**
	 * @brief	Constructor.
	 *
	 * @param	strHostname	The hostname.
	 * @param	usPeerPort 	The peer port.
	 */
	Peer::Peer( std::string strHostname, unsigned short usPeerPort ) :
		m_ucConnectStatus( 0 ),
		m_pSocketTCP( NULL ),
		m_pSocketUDPOut( NULL ),
		m_strHostname( strHostname ),
		m_uiIP( 0 ),
		m_usPort( usPeerPort ),
		m_uiPeerID( ++sm_uiNumPeers ),
		m_uiUserID( 0 ),
		m_bActive( true )
	{
	}


	/**
	 * @brief	Constructor.
	 *
	 * @param	uiIP	  	The ip.
	 * @param	usPeerPort	The peer port.
	 */
	Peer::Peer( unsigned int uiIP, unsigned short usPeerPort ) :
		m_ucConnectStatus( 0 ),
		m_pSocketTCP( NULL ),
		m_pSocketUDPOut( NULL ),
		m_strHostname(),
		m_uiIP( uiIP ),
		m_usPort( usPeerPort ),
		m_uiPeerID( ++sm_uiNumPeers ),
		m_uiUserID( 0 ),
		m_bActive( true )
	{
	}


	/**
	 * @brief	Destructor.
	 */
	Peer::~Peer(void)
	{
		if( m_ucConnectStatus > 0 )
			disconnect();

		m_mxSockets.lock();
		if( m_pSocketTCP != NULL )
			delete m_pSocketTCP;
		if( m_pSocketUDPOut != NULL )
			delete m_pSocketUDPOut;
		m_mxSockets.unlock();
	}


	/**
	 * @brief	Connects.
	 *
	 * @param	usListeningPort	The listening port.
	 * @param	uiUserID	   	Identifier for the peer.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Peer::connect( unsigned short usListeningPort, unsigned int uiUserID )
	{
		if( m_ucConnectStatus == 0 )
		{
			m_pSocketTCP = new BerkeleySocket( SOCK_STREAM );
			m_pSocketUDPOut = new BerkeleySocket( SOCK_DGRAM );

			m_uiUserID = uiUserID;

			if( !connectSockets() )
				return false;

			ConnectMessage* pMsg = new ConnectMessage( usListeningPort, uiUserID );

			m_mxSockets.lock();

			// try to contact the peer twice
			if( !m_pSocketTCP->write( pMsg->getMsgString() ) )
			{
				if( !m_pSocketTCP->write( pMsg->getMsgString() ) )
				{
					delete m_pSocketTCP;
					delete m_pSocketUDPOut;

					m_mxSockets.unlock();
					return false;
				}
			}

			m_ucConnectStatus = 1;
			
			std::string strMsg;
			m_pSocketTCP->read( strMsg );
			Message* pMsg2 = Message::createFromString( strMsg.c_str() );

			if( pMsg2->getType() == MT_ConnectMessage )
			{
				if( ((ConnectMessage*)pMsg2)->getPeerID() > 0 )
					m_uiPeerID = ((ConnectMessage*)pMsg2)->getPeerID();
				m_usPort = ((ConnectMessage*)pMsg2)->getPort();
				m_ucConnectStatus = 2;
			}
			else
			{
				Log::getLog("oocl")->logError( "The first message from a peer was not a ConnectMessage" );
			}

			m_mxSockets.unlock();

			return true;
		}

		Log::getLog("oocl")->logWarning( "You tried to connect to a peer that is already connected" );

		return false;
	}


	/**
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

			m_mxSockets.lock();

			m_uiUserID = uiUserID;

			if( pMsg->getPeerID() > 0 )
				m_uiPeerID = pMsg->getPeerID();
			m_pSocketTCP = pTCPSocket;

			m_pSocketUDPOut = new BerkeleySocket( SOCK_DGRAM );
			m_pSocketUDPOut->connect( m_uiIP, m_usPort );

			ConnectMessage* pMsg = new ConnectMessage( usListeningPort, uiUserID );

			// try to contact the peer twice
			if( !m_pSocketTCP->write( pMsg->getMsgString() ) )
			{
				if( !m_pSocketTCP->write( pMsg->getMsgString() ) )
				{
					delete m_pSocketTCP;
					delete m_pSocketUDPOut;

					m_mxSockets.unlock();
					return false;
				}
			}

			m_mxSockets.unlock();

			m_ucConnectStatus = 2;

			return true;
		}

		Log::getLog("oocl")->logWarning( "A peer tried to connect to you but you are already connected to each other" );

		return false;
	}


	/**
	 * @brief	Disconnects this peer.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Peer::disconnect( bool bSendMessage )
	{
		if( bSendMessage && m_ucConnectStatus > 0 )
			sendMessage( new DisconnectMessage() );

		for( std::list<unsigned short>::iterator it = m_lusSubscribedMsgTypes.begin(); it != m_lusSubscribedMsgTypes.end(); ++it )
			MessageBroker::getBrokerFor( (*it) )->unregisterListener( this );

		m_mxSockets.lock();

		m_ucConnectStatus = 0;
		m_bActive = false;

		delete m_pSocketTCP;
		m_pSocketTCP = NULL;

		delete m_pSocketUDPOut;
		m_pSocketUDPOut = NULL;

		m_mxSockets.unlock();

		return true;
	}

	/**
	 * @brief	Connects the sockets with the data known from calls to one of the connect methods.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Peer::connectSockets()
	{
		bool bUDPConnected = false, bTCPConnected = false;

		if( m_pSocketUDPOut == NULL || m_pSocketTCP == NULL )
			return false;

		if( m_uiIP != 0 )
		{
			m_mxSockets.lock();
			bUDPConnected = m_pSocketUDPOut->connect( m_uiIP, m_usPort );
			bTCPConnected = m_pSocketTCP->connect( m_uiIP, m_usPort );
			m_mxSockets.unlock();
		}
		else
		{
			m_mxSockets.lock();
			bUDPConnected = m_pSocketUDPOut->connect( m_strHostname, m_usPort );
			bTCPConnected = m_pSocketTCP->connect( m_strHostname, m_usPort );
			m_mxSockets.unlock();
		}

		if( !bUDPConnected || !bTCPConnected )
		{
			Log::getLogRef("oocl") << Log::EL_WARNING << "tried to connectSockets to peer " << m_uiPeerID << " but failed" << endl;
			m_ucConnectStatus = 0;
			return false;
		}

		return true;
	}


	/**
	 * @brief	Gets the messages to which the peer has subscribed.
	 *
	 * @param	pMessage [in]	The incoming message.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Peer::cbMessage( Message const * const pMessage )
	{
		if( pMessage->isIncoming() || !m_bActive )
		{
//			Log::getLog("oocl")->logWarning("stopped incoming message from being sent back into the network");
			return true;
		}

		sendMessage( pMessage );

		return true;
	}


	/**
	 * @brief	Sends a message to the connected peer.
	 *
	 * @param	pMessage [in]	The message to send.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Peer::sendMessage( Message const * const pMessage )
	{
		if( !m_bActive )
			return false;

		if( !isConnected() && m_ucConnectStatus == 1 )
		{
			connectSockets();
		}

		if( isConnected() )
		{
			if( pMessage->isIncoming() ) //m_uiPeerID > 0 && pMessage->getSenderID() == m_uiPeerID )
			{
				return true;
			}

			m_mxSockets.lock();
			if( !m_bActive )
				return false;

			bool bReturn = false;
			if( pMessage->getProtocoll() == SOCK_DGRAM && m_pSocketUDPOut != NULL )
				bReturn = m_pSocketUDPOut->write( pMessage->getMsgString()+std::string( (char*)&m_uiUserID, 4 ) );
			else if( pMessage->getProtocoll() == SOCK_STREAM && m_pSocketTCP != NULL )
				bReturn = m_pSocketTCP->write( pMessage->getMsgString() );
			else
				Log::getLog("oocl")->logWarning("attempted to send a message over the network that was not intended for that (protocol 0)" );

			if( !bReturn )
				Log::getLogRef("oocl") << Log::EL_ERROR << "failed to send a message to peer " << m_uiPeerID << endl;

			m_mxSockets.unlock();

			return bReturn;
		}
		else if( m_ucConnectStatus == 1 )
		{
			Log::getLogRef("oocl") << Log::EL_WARNING << "Message was dropped because peer " << m_uiPeerID << " is not fully connected" << endl;
		}
		else
		{
			Log::getLogRef("oocl") << Log::EL_WARNING << "Message was dropped because peer " << m_uiPeerID << " is not connected" << endl;
		}

		return false;
	}


	/**
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
	 * @brief	Called when a message of the connected peer was received.
	 *
	 * @param	pMessage [in]	The received message.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Peer::receiveMessage( Message* pMessage )
	{
		if( !m_bActive )
			return false;

		pMessage->setSenderID( m_uiPeerID );

		switch( pMessage->getType() )
		{
		case MT_SubscribeMessage:
			{
				unsigned short usType = ((SubscribeMessage*)pMessage)->getTypeToSubscribe();
				MessageBroker::getBrokerFor( usType )->registerListener( this );
				m_lusSubscribedMsgTypes.push_back( usType );

				Log::getLogRef( "oocl" ) << Log::EL_INFO << "Peer " << m_uiPeerID << " subscribed message " << usType << endl;

				break;
			}
		case MT_DisconnectMessage:
			{
#ifdef SIM_DELAY
				MessageDelayer* pMD = new MessageDelayer( pMessage );
#else
				MessageBroker::getBrokerFor( MT_DisconnectMessage )->pumpMessage( pMessage );
#endif

				disconnect( false );

				break;
			}
		default:
#ifdef SIM_DELAY
			MessageDelayer* pMD = new MessageDelayer( pMessage );
#else
			MessageBroker::getBrokerFor( pMessage->getType() )->pumpMessage( pMessage );
#endif
			break;
		}

		return true;
	}

	/**
	 * @brief	Checks whether the peer is truly connected and returns the status.
	 *
	 * @return	True if the peer is connected, false if not.
	 */
	bool Peer::isConnected()
	{
		if( m_ucConnectStatus == 2 )
		{
			if( m_pSocketTCP != NULL && m_pSocketTCP->isConnected() && m_pSocketUDPOut != NULL && m_pSocketUDPOut->isConnected() )
			{
				return true;
			}

			m_ucConnectStatus = 1;
		}

		return false;
	}

	/**
	 * @brief	Get this peers PeerID.
	 * @return	This peers PeerID.
	 */
	PeerID	Peer::getPeerID()
	{
		return m_uiPeerID;
	}

	/**
	 * @brief	Get the IP with which this peer is connected.
	 *
	 * @return	The peers IP.
	 */
	unsigned int Peer::getIP()
	{
		if( m_pSocketTCP != NULL )
			return m_pSocketTCP->getConnectedIP();

		return 0;
	}

	/**
	 * @brief	Get the port this peer is listening for UDP messages.
	 *
	 * @return	This peers listening port.
	 */
	unsigned short Peer::getListeningPort()
	{
		return m_usPort;
	}


	/**
	 * @brief	Deactivate this peer and prevent it from sending and receiving messages.
	 */
	void Peer::deactivate()
	{
		m_mxSockets.lock();
		m_bActive = false;
		m_mxSockets.unlock();
	}

	
#ifdef SIM_DELAY
	Peer::MessageDelayer::MessageDelayer( Message* pMessage )
		: m_pMessage( pMessage )
	{
		start();
	}

	void Peer::MessageDelayer::run()
	{
		srand( clock() );
		Thread::sleep( 100 + rand() % 100 );
		MessageBroker::getBrokerFor( m_pMessage->getType() )->pumpMessage( m_pMessage );
	}
#endif
}
