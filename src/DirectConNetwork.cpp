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

#include "DirectConNetwork.h"

namespace oocl
{
	/**
	 * @fn	DirectConNetwork::DirectConNetwork()
	 *
	 * @brief	Default constructor.
	 */
	DirectConNetwork::DirectConNetwork() :
		m_bConnected(false)
	{
		oocl::ConnectMessage::registerMsg();
		oocl::DisconnectMessage::registerMsg();
	}

	/**
	 * @fn	DirectConNetwork::~DirectConNetwork()
	 *
	 * @brief	Destructor.
	 */
	DirectConNetwork::~DirectConNetwork()
	{
		disconnect();

		delete m_pSocketUDPIn;
		m_pSocketUDPIn = NULL;
		delete m_pSocketUDPOut;
		m_pSocketUDPOut = NULL;
		delete m_pSocketTCP;
		m_pSocketTCP = NULL;
	}

	/**
	 * @fn	bool DirectConNetwork::connect( std::string strHostname, unsigned short usPort,
	 * 		unsigned short usListeningPort )
	 *
	 * @brief	Connects with another process.
	 *
	 * @param	strHostname		The hostname as string (URL or IP).
	 * @param	usHostPort	   	The port to connect to.
	 * @param	usListeningPort The port on which the udp socket listens.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool DirectConNetwork::connect( std::string strHostname, unsigned short usHostPort, unsigned short usListeningPort )
	{
		if( !m_bConnected )
		{
			m_usHostPort = usHostPort;
			m_usListeningPort = usListeningPort;
			
			m_bConnected = true;

			m_pSocketUDPIn = new BerkeleySocket( SOCK_DGRAM );
			m_bConnected &= m_pSocketUDPIn->bind( m_usListeningPort );

			m_pSocketUDPOut = new BerkeleySocket( SOCK_DGRAM );
			m_bConnected &= m_pSocketUDPOut->connect( strHostname, m_usHostPort );

			m_pSocketTCP = new BerkeleySocket( SOCK_STREAM );
			m_bConnected &= m_pSocketTCP->connect( strHostname, m_usHostPort );

			if( m_bConnected )
			{
				sendMessage( new ConnectMessage( m_usListeningPort ) );
				start();
			}

			return m_bConnected;
		}

		return false;
	}

	/**
	 * @fn	bool DirectConNetwork::listen( unsigned short usListeningPort )
	 *
	 * @brief	Waits for a client to connect.
	 *
	 * @param	usListeningPort	The listening port.
	 * @param	bBlocking		If true, a call of this method will be blocking.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool DirectConNetwork::listen( unsigned short usListeningPort, bool bBlocking )
	{
		if( !m_bConnected )
		{
			m_usListeningPort = usListeningPort;

			m_pServerSocket = new ServerSocket();
			m_pSocketUDPIn = new BerkeleySocket( SOCK_DGRAM );
			m_pSocketUDPOut = new BerkeleySocket( SOCK_DGRAM );

			m_pServerSocket->bind( usListeningPort );
			m_pSocketUDPIn->bind( usListeningPort );

			if( bBlocking )
			{
				m_pSocketTCP = m_pServerSocket->accept();

				std::string strMsg;
				m_pSocketTCP->read( strMsg );

				Message* pMsg = Message::createFromString( strMsg.c_str() );

				if( pMsg->getType() != MT_ConnectMessage )
				{
					Log::getLog("oocl")->logError("the first received message was not a connectMessage!" );
					return false;
				}

				m_usHostPort = ((ConnectMessage*)pMsg)->getPort();
				m_pSocketUDPOut->connect( ((BerkeleySocket*)m_pSocketTCP)->getConnectedIP(), m_usHostPort );

				sendMessage( new ConnectMessage( m_usListeningPort ) );

				m_bConnected = true;
			}

			start();

			return true;
		}

		return false;
	}

	/**
	 * @fn	bool DirectConNetwork::disconnect()
	 *
	 * @brief	Disconnects from the other process.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool DirectConNetwork::disconnect()
	{
		sendMessage( new DisconnectMessage() );

		m_pSocketUDPIn->close();
		m_pSocketUDPOut->close();
		m_pSocketTCP->close();
		
		m_bConnected = false;

		return true;
	}

	/**
	 * @fn	bool DirectConNetwork::sendMessage( Message* pMessage )
	 *
	 * @brief	Sends a message to the connected process either over the standard protocoll or the protocoll specified in the message.
	 *
	 * @param [in]	pMessage	The message that you want to send.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool DirectConNetwork::sendMessage( Message const * const pMessage )
	{
		if( pMessage && m_bConnected )
		{
			if( pMessage->getProtocoll() == SOCK_DGRAM )
			{
				m_pSocketUDPOut->write( pMessage->getMsgString() );
			}
			else if( pMessage->getProtocoll() == SOCK_STREAM )
			{
//				int flag = 0;
//				setsockopt(m_pSocketTCP->getCSocket(), IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
				m_pSocketTCP->write( pMessage->getMsgString() );
//				flag = 1;
//				setsockopt(m_pSocketTCP->getCSocket(), IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
			}
			else
				Log::getLog("oocl")->logWarning("You tried to send a message over network that was not intended for that" );
			
			return true;
		}
		
		return false;
	}

	/**
	 * @fn	bool DirectConNetwork::registerListener( MessageListener* pListener )
	 *
	 * @brief	Registers the listener described by pListener to this network so that the listener will receive all further messages.
	 *
	 * @param [in]	pListener	The object that you want to receive messages from the network.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool DirectConNetwork::registerListener( MessageListener* pListener )
	{
		if( pListener )
		{
			m_lListeners.push_back( pListener );
			return true;
		}
		else
			return false;
	}

	/**
	 * @fn	bool DirectConNetwork::unregisterListener( MessageListener* pListener )
	 *
	 * @brief	Unregisters the specified listener.
	 *
	 * @param [in]	pListener	The listener that you want to unregister.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool DirectConNetwork::unregisterListener( MessageListener* pListener )
	{
		if( pListener )
		{
			for( std::list< MessageListener* >::iterator it = m_lListeners.begin(); it != m_lListeners.end(); ++it )
			{
				if( (*it) == pListener )
				{
					m_lListeners.erase( it );
					return true;
				}
			}
		}

		return false;
	}

	/**
	 * @fn	void DirectConNetwork::run()
	 *
	 * @brief	implementation of the Thread run-method, listens for incoming messages.
	 */
	void DirectConNetwork::run()
	{
		// if the listen call was not blocking, call accept here
		if( !m_bConnected )
		{
			m_pSocketTCP = m_pServerSocket->accept();

			std::string strMsg;
			m_pSocketTCP->read( strMsg );

			Message* pMsg = Message::createFromString( strMsg.c_str() );

			if( pMsg->getType() != MT_ConnectMessage )
			{
				Log::getLog("oocl")->logError("the first received message was not a connectMessage!" );
				return;
			}

			m_usHostPort = ((ConnectMessage*)pMsg)->getPort();
			m_pSocketUDPOut->connect( m_pSocketTCP->getConnectedIP(), m_usHostPort );

			sendMessage( new ConnectMessage( m_usListeningPort ) );

			m_bConnected = true;
		}

		// substitution of std::max / max as it is defined different in msvc and gnucc
		int iBiggestSocket = m_pSocketUDPIn->getCSocket();
		if( iBiggestSocket < m_pSocketTCP->getCSocket() )
			iBiggestSocket = m_pSocketTCP->getCSocket();

		fd_set selectSet;

		while( m_bConnected )
		{
			FD_ZERO( &selectSet );
			FD_SET( m_pSocketUDPIn->getCSocket(), &selectSet );
			FD_SET( m_pSocketTCP->getCSocket(), &selectSet );

			timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 500000;

			int iRet = select( iBiggestSocket+1, &selectSet, NULL, NULL, &tv );
#ifdef _MSC_VER
			if( iRet == SOCKET_ERROR )
#else
			if( iRet < 0 )
#endif
			{
				Log::getLog("oocl")->logError( "Selecting failed" );
				continue;
			}
			else if( iRet == 0 )
				continue;

			if( FD_ISSET( m_pSocketTCP->getCSocket(), &selectSet ) )
			{
				std::string strMsg;
				m_pSocketTCP->read( strMsg );

				while( !strMsg.empty() )
				{
					Message* pMsg = Message::createFromString( strMsg.c_str() );
					if( pMsg == NULL )
						break;

					if( strMsg.length() < pMsg->getBodyLength() + 4 )
						Log::getLogRef("oocl") << Log::EL_WARNING << "read " << (int)strMsg.length() << " bytes while expected message is " << pMsg->getBodyLength() + 4 << " bytes long" << oocl::endl;

					strMsg = strMsg.substr( pMsg->getBodyLength() +4 );

					if( pMsg->getType() == MT_DisconnectMessage )
					{
						m_bConnected = false;

						m_pSocketUDPIn->close();
						m_pSocketUDPOut->close();
						m_pSocketTCP->close();
					}

					std::list< MessageListener* > lWaitList( m_lListeners );

					for( std::list<MessageListener*>::iterator it = lWaitList.begin(); it != lWaitList.end(); it = lWaitList.erase( it ) )
					{
						if( !(*it)->cbMessage( pMsg ) )
							lWaitList.push_back( (*it) );
					}

					MessageBroker::getBrokerFor(pMsg->getType())->pumpMessage( pMsg );
				}
			}

			if( FD_ISSET( m_pSocketUDPIn->getCSocket(), &selectSet ) )
			{
				std::string strMsg;
				m_pSocketUDPIn->read( strMsg );

				Message* pMsg = Message::createFromString( strMsg.c_str() );
				if( pMsg == NULL )
					continue;

				MessageBroker::getBrokerFor(pMsg->getType())->pumpMessage( pMsg );

				std::list< MessageListener* > lWaitList( m_lListeners );

				for( std::list<MessageListener*>::iterator it = lWaitList.begin(); it != lWaitList.end(); it = lWaitList.erase( it ) )
				{
					if( !(*it)->cbMessage( pMsg ) )
						lWaitList.push_back( (*it) );
				}
			}
		}
	}

}
