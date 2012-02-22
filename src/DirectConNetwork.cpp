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

#include "DirectConNetwork.h"

namespace oocl
{
	/**
	 * @fn	DirectConNetwork::DirectConNetwork()
	 *
	 * @brief	Default constructor.
	 *
	 * @author	Jörn Teuber
	 * @date	9/14/2011
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
	 *
	 * @author	Jörn Teuber
	 * @date	9/14/2011
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
	 * 		int iProtocoll )
	 *
	 * @brief	Connects with another process.
	 *
	 * @author	Jörn Teuber
	 * @date	9/14/2011
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
			
			m_pSocketUDPIn = new Socket( SOCK_DGRAM );
			m_pSocketUDPIn->bind( m_usListeningPort );

			m_pSocketUDPOut = new Socket( SOCK_DGRAM );
			m_pSocketUDPOut->connect( strHostname, m_usHostPort );

			m_pSocketTCP = new Socket( SOCK_STREAM );
			m_pSocketTCP->connect( strHostname, m_usHostPort );
			
			m_bConnected = true;

			sendMessage( new ConnectMessage( m_usListeningPort ) );

			start();

			return true;
		}

		return false;
	}

	/**
	 * @fn	bool DirectConNetwork::listen( unsigned short usListeningPort )
	 *
	 * @brief	Waits for a client to connect.
	 *
	 * @author	Jörn Teuber
	 * @date	12/15/2011
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
			m_pSocketUDPIn = new Socket( SOCK_DGRAM );
			m_pSocketUDPOut = new Socket( SOCK_DGRAM );

			m_pServerSocket->bind( usListeningPort );
			m_pSocketUDPIn->bind( usListeningPort );

			if( bBlocking )
			{
				m_pSocketTCP = m_pServerSocket->accept();

				std::string strMsg = m_pSocketTCP->read();

				Message* pMsg = Message::createFromString( strMsg.c_str() );

				if( pMsg->getType() != MT_ConnectMessage )
				{
					Log::getLog("oocl")->logMessage("the first received message was not a connectMessage!", Log::EL_ERROR );
					return false;
				}

				m_usHostPort = ((ConnectMessage*)pMsg)->getPort();
				m_pSocketUDPOut->connect( m_pSocketTCP->getConnectedIP(), m_usHostPort );

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
	 * @author	Jörn Teuber
	 * @date	9/14/2011
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
	 * @author	Jörn Teuber
	 * @date	9/14/2011
	 *
	 * @param [in]	pMessage	The message that you want to send.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool DirectConNetwork::sendMessage( Message* pMessage )
	{
		if( pMessage && m_bConnected )
		{
			if( pMessage->getProtocoll() == SOCK_DGRAM )
				m_pSocketUDPOut->write( pMessage->getMsgString() );
			else if( pMessage->getProtocoll() == SOCK_STREAM )
				m_pSocketTCP->write( pMessage->getMsgString() );
			else
				Log::getLog("oocl")->logMessage("You tried to send a message over network that was not intended for that", Log::EL_WARNING );
			
			return true;
		}
		
		return false;
	}

	/**
	 * @fn	bool DirectConNetwork::registerListener( MessageListener* pListener )
	 *
	 * @brief	Registers the listener described by pListener to this network so that the listener will receive all further messages.
	 *
	 * @author	Jörn Teuber
	 * @date	9/15/2011
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
	 * @author	Jörn Teuber
	 * @date	9/15/2011
	 *
	 * @param [in]	pListener	The listener that you want to unregister.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool DirectConNetwork::unregisterListener( MessageListener* pListener )
	{
		if( pListener )
		{
			for( std::list< MessageListener* >::iterator it = m_lListeners.begin(); it != m_lListeners.end(); it++ )
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
	 *
	 * @author	Jörn Teuber
	 * @date	11/25/2011
	 */
	void DirectConNetwork::run()
	{
		if( !m_bConnected )
		{
			m_pSocketTCP = m_pServerSocket->accept();

			std::string strMsg = m_pSocketTCP->read();

			Message* pMsg = Message::createFromString( strMsg.c_str() );

			if( pMsg->getType() != MT_ConnectMessage )
			{
				Log::getLog("oocl")->logMessage("the first received message was not a connectMessage!", Log::EL_ERROR );
				return;
			}

			m_usHostPort = ((ConnectMessage*)pMsg)->getPort();
			m_pSocketUDPOut->connect( m_pSocketTCP->getConnectedIP(), m_usHostPort );

			m_bConnected = true;
		}


		int iBiggestSocket = max( m_pSocketUDPIn->getCSocket(), m_pSocketTCP->getCSocket() );

		fd_set selectSet;
		FD_ZERO( &selectSet );

		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		while( m_bConnected )
		{
			FD_SET( m_pSocketUDPIn->getCSocket(), &selectSet );
			FD_SET( m_pSocketTCP->getCSocket(), &selectSet );

			int iRet = select( iBiggestSocket+1, &selectSet, NULL, NULL, &tv );
			if( iRet == SOCKET_ERROR )
			{
				Log::getLog("oocl")->logMessage( "Selecting failed", Log::EL_ERROR );
			}

			if( FD_ISSET( m_pSocketTCP->getCSocket(), &selectSet ) )
			{
				std::string strMsg = m_pSocketTCP->read();
				Message* pMsg = Message::createFromString( strMsg.c_str() );
				if( !pMsg )
					continue;

				if( pMsg->getType() == MT_DisconnectMessage )
				{
					m_bConnected = false;

					m_pSocketUDPIn->close();
					m_pSocketUDPOut->close();
					m_pSocketTCP->close();
				}

				for( std::list<MessageListener*>::iterator it = m_lListeners.begin(); it != m_lListeners.end(); it++ )
				{
					if( (*it)->requestMutex() )
					{
						(*it)->cbMessage( pMsg );
						(*it)->returnMutex();
					}
				}
			}

			if( FD_ISSET( m_pSocketUDPIn->getCSocket(), &selectSet ) )
			{
				std::string strMsg = m_pSocketUDPIn->read();
				Message* pMsg = Message::createFromString( strMsg.c_str() );
				if( !pMsg )
					continue;

				for( std::list<MessageListener*>::iterator it = m_lListeners.begin(); it != m_lListeners.end(); it++ )
				{
					if( (*it)->requestMutex() )
					{
						(*it)->cbMessage( pMsg );
						(*it)->returnMutex();
					}
				}
			}
		}
	}

}
