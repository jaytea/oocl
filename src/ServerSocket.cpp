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
	 * @fn	ServerSocket::ServerSocket()
	 *
	 * @brief	Default constructor.
	 */
	ServerSocket::ServerSocket()
		: SocketStub(),
		bValid( true ),
		bBound( false )
	{
		// create socket
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef linux
		if(sockfd < 0) {
			perror("ERROR opening socket");
			bValid = false;
		}
#else
		if(sockfd == INVALID_SOCKET) {
			printf("ERROR opening socket! Error code: %d\n", WSAGetLastError() );
			bValid = false;
		}
#endif
	}


	/**
	 * @fn	bool ServerSocket::bind( unsigned short port )
	 *
	 * @brief	Binds the socket to the given port.
	 *
	 * @param	port	The port.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool ServerSocket::bind( unsigned short port )
	{
		if( bValid && !bBound )
		{
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = INADDR_ANY;
			addr.sin_port = htons(port);

			// bind socket to port
			int result = ::bind( sockfd, (struct sockaddr *) &addr, sizeof(addr) );
#ifdef linux
			if( result < 0 ){
				perror("ERROR on binding");
				return false;
			}
#else
			if( result == SOCKET_ERROR )
			{
				printf("ERROR on binding! Error code: %d\n", WSAGetLastError() );
				return false;
			}
#endif
		
			// start listening
			listen(sockfd,10);

			bBound = true;
			return true;
		}

		return false;
	}


	/**
	 * @fn	ServerSocket::~ServerSocket()
	 *
	 * @brief	Destructor.
	 */
	ServerSocket::~ServerSocket()
	{
		bBound = false;
#ifdef linux
		::close(sockfd);
#else
		::closesocket(sockfd);
#endif
		SocketStub::~SocketStub();
	}


	/**
	 * @fn	Socket * ServerSocket::accept()
	 *
	 * @brief	Waits for another process to connect.
	 *
	 * @return	A connected TCP socket.
	 */
	Socket * ServerSocket::accept()
	{
		if( bBound )
		{
			struct sockaddr_in cli_addr;
#ifdef linux
			socklen_t clilen = sizeof(cli_addr);
#else
			int clilen = sizeof(cli_addr);
#endif

			int newsockfd = ::accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
#ifdef linux
			if(newsockfd < 0) {
				perror("ERROR on accept");
				return NULL;
			}
#else
			if(newsockfd == INVALID_SOCKET) {
				printf("ERROR on accept! Error code: %d\n", WSAGetLastError() );
				return NULL;
			}
#endif
			return new Socket(newsockfd, cli_addr);
		}

		return NULL;
	}

}