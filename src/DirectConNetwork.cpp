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

	/// Constructor
	DirectConNetwork::DirectConNetwork() :
		m_bConnected(false)
	{
		m_pUDPSocket = new Socket( SOCK_DGRAM );
		m_pTCPSocket = new Socket();
	}

	/// Destructor
	DirectConNetwork::~DirectConNetwork()
	{
		disconnect();

		delete m_pUDPSocket;
		m_pUDPSocket = NULL;
		delete m_pTCPSocket;
		m_pTCPSocket = NULL;
	}

	/// connect with the other process
	/**
	 *
	 **/
	bool DirectConNetwork::connect( std::string strHostname, unsigned short usPort, int iProtocoll )
	{
		if( !m_bConnected )
		{
			m_iStdProtocoll = iProtocoll;

			if( iProtocoll == 0 )
			{
				m_pUDPSocket->connect( strHostname, usPort );
				m_pTCPSocket->connect( strHostname, usPort );
			}
			else if( iProtocoll == SOCK_DGRAM )
			{
				m_pUDPSocket->connect( strHostname, usPort );
				m_pStdSocket = m_pUDPSocket;
			}
			else if( iProtocoll == SOCK_STREAM )
			{
				m_pTCPSocket->connect( strHostname, usPort );
				m_pStdSocket = m_pTCPSocket;
			}

			m_bConnected = true;

			return true;
		}

		return false;
	}

	bool DirectConNetwork::disconnect()
	{
		m_pUDPSocket->close();
		m_pTCPSocket->close();

		m_bConnected = false;

		return true;
	}

	bool DirectConNetwork::sendMsg( Message* pMessage )
	{
		if( m_iStdProtocoll == 0 )
		{
			if( pMessage->getProtocoll() == SOCK_DGRAM )
				m_pUDPSocket->write( pMessage->getMsgString() );
			else if( pMessage->getProtocoll() == SOCK_STREAM )
				m_pTCPSocket->write( pMessage->getMsgString() );
		}
		else
		{
			m_pStdSocket->write( pMessage->getMsgString() );
		}
		return true;
	}

	bool DirectConNetwork::registerListener( MessageListener* pListener )
	{
		m_lListeners.push_back( pListener );
		return true;
	}

	bool DirectConNetwork::unregisterListener( MessageListener* pListener )
	{
		for( std::list< MessageListener* >::iterator it = m_lListeners.begin(); it != m_lListeners.end(); it++ )
		{
			if( (*it) == pListener )
			{
				m_lListeners.erase( it );
				return true;
			}
		}

		return false;
	}

}
