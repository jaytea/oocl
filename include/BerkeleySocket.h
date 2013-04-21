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

#ifndef BERKELEYSOCKET_H
#define	BERKELEYSOCKET_H

#include <string>
#include <stdio.h>
#include <sstream>
#include <cstring>

#include <unistd.h>

/*
#ifdef linux
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <sys/socket.h>
#	include <netdb.h>
#else
#	include <Windows.h>
#	include <WinSock2.h>
#endif
*/

#include "oocl_import_export.h"

#include "Socket.h"
#include "Log.h"

namespace oocl
{

	/**
	 * @class	BerkeleySocket
	 *
	 * @brief	Socket class for simple unencrypted berkeley sockets.
	 *
	 * @author	Jörn Teuber
	 * @date	2.3.2012
	 */
	class OOCL_EXPORTIMPORT BerkeleySocket : public Socket
	{
		friend class ServerSocket;

	public:
		BerkeleySocket( int iSockType = SOCK_STREAM );
		virtual ~BerkeleySocket();

		virtual bool connect( std::string host, unsigned short usPort );
		virtual bool connect( unsigned int uiHostIP, unsigned short usPort );
		virtual bool bind( unsigned short usPort );

		virtual bool isValid();
		virtual bool isConnected();

		virtual std::string read(int count = 0);
		virtual char		readC();
		virtual char*		readCA(int count, int * readCount = NULL);

		virtual std::string readFrom( int count = 0, unsigned int* hostIP = NULL );


		virtual bool write(std::string in);
		virtual bool writeC(char in);
		virtual bool writeCA(const char * in, int count);

		virtual bool writeTo( std::string in, std::string host, unsigned short port );

		virtual void close();

		virtual int getCSocket() { return sockfd; }
		unsigned int getConnectedIP() { return m_addrData.sin_addr.s_addr; }

	protected:
		BerkeleySocket(int socket, struct sockaddr_in sock_addr);

		::sockaddr_in m_addrData;
		//struct sockaddr_in6 addr6;
		int sockfd;

		bool bValid;
		bool bConnected;

		static int iSocketCounter;

	private:
		unsigned int getAddrFromString( const char* hostnameOrIp );

		int bufferSize;
	};

}

#endif	/* BERKELEYSOCKET_H */
