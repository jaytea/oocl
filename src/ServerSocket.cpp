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
// This file was written by Jürgen Lorenz and Jörn Teuber

#include "ServerSocket.h"

namespace oocl
{
	/**
	 * @brief	Default constructor.
	 */
	ServerSocket::ServerSocket()
		: SocketStub(),
		m_bValid( true ),
		m_bBound( false )
	{
		// create socket
		m_iSockFD = socket(AF_INET, SOCK_STREAM, 0);
#ifdef linux
		if(m_iSockFD < 0) {
			Log::getLog("oocl")->logError("ERROR opening socket");
			m_bValid = false;
		}
#else
		if(m_iSockFD == INVALID_SOCKET) {
			std::ostringstream os;
			os << "ERROR opening socket! Error code: " << WSAGetLastError();
			Log::getLog("oocl")->logError( os.str() );
			m_bValid = false;
		}
#endif
	}


	/**
	 * @brief	Binds the socket to the given port.
	 *
	 * @param	port	The port this socket will be binded to.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool ServerSocket::bind( unsigned short port )
	{
		if( m_bValid && !m_bBound )
		{
			m_sSockAddr.sin_family = AF_INET;
			m_sSockAddr.sin_addr.s_addr = INADDR_ANY;
			m_sSockAddr.sin_port = htons(port);

			// bind socket to port
			int result = ::bind( m_iSockFD, (struct sockaddr *) &m_sSockAddr, sizeof(m_sSockAddr) );
#ifdef linux
			if( result < 0 ){
				Log::getLog("oocl")->logError("ERROR on binding");
				return false;
			}
#else
			if( result == SOCKET_ERROR )
			{
				std::ostringstream os;
				os << "ERROR on binding! Error code: " << WSAGetLastError();
				Log::getLog("oocl")->logError( os.str() );
				return false;
			}
#endif
		
			// start listening
			listen(m_iSockFD,10);

			return true;
		}

		return false;
	}


	/**
	 * @brief	Destructor.
	 */
	ServerSocket::~ServerSocket()
	{
		m_bBound = false;
#ifdef linux
		::close(m_iSockFD);
#else
		::closesocket(sockfd);
#endif
	}


	/**
	 * @brief	Waits for another process to connect.
	 *
	 * @return	A connected TCP socket.
	 */
	Socket * ServerSocket::accept()
	{
		if( m_bBound )
		{
			struct sockaddr_in cli_addr;
#ifdef linux
			socklen_t clilen = sizeof(cli_addr);
#else
			int clilen = sizeof(cli_addr);
#endif

			int newsockfd = ::accept(m_iSockFD, (struct sockaddr *) &cli_addr, &clilen);
#ifdef linux
			if(newsockfd < 0) {
				Log::getLog("oocl")->logError("ERROR on accept");
				return NULL;
			}
#else
			if(newsockfd == INVALID_SOCKET) {
				std::ostringstream os;
				os << "ERROR on accept! Error code: " << WSAGetLastError();
				Log::getLog("oocl")->logError( os.str() );
				return NULL;
			}
#endif
			return new BerkeleySocket(newsockfd, cli_addr);
		}

		return NULL;
	}


	/**
	 * @brief	Check whether this socket was created successfully.
	 *
	 * @return	True if the socket is valid, false if not.
	 */
	bool ServerSocket::isValid()
	{
		return m_bValid;
	}


	/**
	 * @brief	Get the sockets underlying C or Berkeley socket.
	 *
	 * @return	The Sockets underlying CSocket.
	 */
	int ServerSocket::getCSocket()
	{
		return m_iSockFD;
	}

}
