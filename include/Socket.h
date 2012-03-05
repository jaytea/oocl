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

#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#include <string>
#include <stdio.h>
#include <sstream>

#include "oocl_import_export.h"

#include "SocketStub.h"
#include "Log.h"

namespace oocl
{

	struct OOCL_EXPORTIMPORT socket_conf{
		unsigned short   ipVersion;
		unsigned short   bufferSize;
	};


	/**
	 * @class	Socket
	 *
	 * @brief	C++ abstraction of a C socket.
	 *
	 * @author	Jörn Teuber
	 * @date	3/2/2012
	 */
	class OOCL_EXPORTIMPORT Socket : public SocketStub
	{
		friend class ServerSocket;

	public:
		Socket( int iSockType = SOCK_STREAM );
		~Socket();

		bool connect( std::string host, unsigned short usPort );
		bool connect( unsigned int uiHostIP, unsigned short usPort );
		bool bind( unsigned short usPort );

		bool isValid();
		bool isConnected();

		std::string read(int count = 0);
		char		readC();
		char*		readCA(int count, int * readCount = NULL);

		std::string readFrom( int count = 0, unsigned int* hostIP = NULL );


		bool write(std::string in);
		bool writeC(char in);
		bool writeCA(const char * in, int count);

		bool writeTo( std::string in, std::string host, unsigned short port );

		void close();

		int getCSocket() { return sockfd; }
		unsigned int getConnectedIP() { return m_addrData.sin_addr.s_addr; }

	protected:
		Socket(int socket, struct sockaddr_in sock_addr);

		struct sockaddr_in m_addrData;
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

#endif // SOCKET_H_INCLUDED
