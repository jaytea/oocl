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

		delete m_pStdSocket;
		m_pStdSocket = NULL;
	}

	/**
	 * @fn	bool DirectConNetwork::connect( std::string strHostname, unsigned short usPort,
	 * 		int iProtocoll )
	 *
	 * @brief	Connects with the other process.
	 *
	 * @author	Jörn Teuber
	 * @date	9/14/2011
	 *
	 * @param	strHostname	The hostname as string (URL or IP).
	 * @param	usPort	   	The port.
	 * @param	iProtocoll 	The protocoll (SOCK_DGRAM=UDP | SOCK_STREAM=TCP).
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool DirectConNetwork::connect( std::string strHostname, unsigned short usPort, int iProtocoll )
	{
		if( !m_bConnected && ( iProtocoll == SOCK_DGRAM || iProtocoll == SOCK_STREAM ) )
		{
			m_usPort = usPort;
			
			m_pStdSocket = new Socket( iProtocoll );
			m_pStdSocket->connect( strHostname, usPort );

			m_bConnected = true;

			start();

			return true;
		}

		return false;
	}

	bool DirectConNetwork::listen( unsigned short usPort, int iProtocoll )
	{
		if( !m_bConnected )
		{
			m_iStdProtocoll = iProtocoll;
			m_usPort = usPort;

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
		m_bConnected = false;

		join();

		m_pStdSocket->close();

		return true;
	}

	/**
	 * @fn	bool DirectConNetwork::sendMsg( Message* pMessage )
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
	bool DirectConNetwork::sendMsg( Message* pMessage )
	{
		if( pMessage )
		{
			m_pStdSocket->write( pMessage->getMsgString() );
			
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
		// if we are not yet connected we have to listen for a connecting client
		if( !m_bConnected )
		{
			if( m_iStdProtocoll == SOCK_STREAM )
			{
				m_pServerSocket = new ServerSocket();

				m_pServerSocket->bind( m_usPort );
				m_pStdSocket = m_pServerSocket->accept();

				m_bConnected = true;
			}
			else
			{
				m_pStdSocket->bind( m_usPort );
				unsigned int uiIP = 0;

				std::string strHeader = m_pStdSocket->readFrom( 4, &uiIP );

				m_pStdSocket->connect( uiIP, m_usPort );

				unsigned short usMsgLength = ((unsigned short*)strHeader.c_str())[1];
				strHeader.append( m_pStdSocket->read( usMsgLength ) );

				Message* pMsg = Message::createFromString( strHeader.c_str() );

				for( std::list<MessageListener*>::iterator it = m_lListeners.begin(); it != m_lListeners.end(); it++ )
				{
					if( (*it)->requestMutex() )
					{
						(*it)->cbMessage( pMsg );
						(*it)->returnMutex();
					}
				}
				
				m_bConnected = true;
			}
		}

		while( m_bConnected )
		{
			std::string strHeader = m_pStdSocket->read( 4 );

			unsigned short usMsgLength = ((unsigned short*)strHeader.c_str())[1];
			strHeader.append( m_pStdSocket->read( usMsgLength ) );

			Message* pMsg = Message::createFromString( strHeader.c_str() );

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
