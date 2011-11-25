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
/// This file was written by Jürgen Lorenz and Jörn Teuber

#include "Socket.h"

namespace oocl
{

	int Socket::iSocketCounter = 0;


	Socket::Socket(int socket, struct sockaddr_in sock_addr)
		: SocketStub(),
		bValid( true )
	{
		bConnected = true;
		bufferSize = 1024;
		sockfd = socket;
		addr = sock_addr;
	}

	Socket::Socket( int iSockType )
		: SocketStub(),
		bConnected( false )
	{
		bValid = true;
		bufferSize = 1024;

		sockfd = socket(AF_INET, iSockType, 0);
#ifdef linux
		if (sockfd < 0)
		{
    		bValid = false;
			perror("ERROR opening socket");
		}
#else
		if( sockfd == INVALID_SOCKET )
		{
    		bValid = false;
			printf("ERROR opening socket! Error code: %d\n", WSAGetLastError() );
		}
#endif
	}

	bool Socket::connect( std::string host, unsigned short port )
	{
		return connect( getAddrFromString( host.c_str() ), port );
	}

	bool Socket::connect( unsigned int uiHostIP, unsigned short port )
	{
		if( bValid && !bConnected )
		{
			addr.sin_addr.s_addr = uiHostIP;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);

			int result = ::connect(sockfd,(struct sockaddr *) &addr,sizeof(addr));
#ifdef linux
			if( result < 0){
    			perror("ERROR connecting");
    			close();
				return false;
			}
#else
			if( result == SOCKET_ERROR )
			{
				printf("ERROR connecting! Error code: %d\n", WSAGetLastError() );
    			close();
				return false;
			}
#endif

			bConnected = true;

			return true;
		}

		return false;
	}

	bool Socket::bind( unsigned short usPort )
	{
		if( bValid && !bConnected )
		{
			addr.sin_family = AF_INET;
			addr.sin_port = htons(usPort);
			addr.sin_addr.s_addr = INADDR_ANY;

			int result = ::bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
#ifdef linux
			if( result < 0){
    			perror("ERROR binding socket!");
    			close();
				return false;
			}
#else
			if( result == SOCKET_ERROR )
			{
				printf("ERROR binding socket! Error code: %d\n", WSAGetLastError() );
    			close();
				return false;
			}
#endif
			bConnected = true;

			return true;
		}

		return false;
	}

	Socket::~Socket()
	{
		close();
	}

	bool Socket::isValid()
	{
		return bValid;
	}

	bool Socket::isConnected()
	{
		return bConnected;
	}


	std::string Socket::read(int count)
	{
		if(count==0){
			count = bufferSize;
		}
		int readCount = 0;
		char * in = readCA(count,&readCount);
		if(in!=NULL){
			return std::string(in,readCount);
		}

		return std::string();
	}

	char Socket::readC()
	{
		char * in = readCA(1);
		if(in!=NULL){
			return in[0];
		}

		return 0;
	}

	char * Socket::readCA(int count, int * readCount)
	{
		if(bConnected && count>0){
    		char * buffer = new char[count];
			int rc = ::recv(sockfd,buffer,count,0);
			if(readCount != NULL){
				*readCount = rc;
			}

			if(rc > 0){
				return buffer;
			}
			else if(rc < 0){
				//printf("recv:%d:%s\n", rc, buffer); ?? du gibts einen Buffer aus, der nur Müll enthält!
#ifdef linux
				perror("ERROR receiving");
#else
				printf("ERROR receiving! Error code: %d\n", WSAGetLastError() );
#endif
				close();
			}
		}
		return NULL;
	}

	std::string Socket::readFrom( int count, unsigned int* hostIP )
	{
		if( bValid && !bConnected )
		{
			if(count==0){
				count = bufferSize;
			}

			char * buffer = new char[count];
			int fromlen;
			int rc = ::recvfrom( sockfd,buffer,count,0, (sockaddr*)&addr, &fromlen);

			if(rc > 0)
			{
				if( hostIP != NULL )
					*hostIP = addr.sin_addr.s_addr;
				return std::string(buffer, rc);
			}
			else if(rc < 0)
			{
#ifdef linux
				perror("ERROR receiving");
#else
				printf("ERROR receiving! Error code: %d\n", WSAGetLastError() );
#endif
				close();
			}
		}
	}


	void Socket::write(std::string in)
	{
		writeCA(in.c_str(),in.length());
	}

	void Socket::writeC(char in)
	{
		writeCA(&in,1);
	}

	void Socket::writeCA(const char * in, int count)
	{
		if( bConnected && count>0 ){
			int rc = ::send(sockfd,in,count,0);
			if(rc < 0){
				printf("send:%d:%d:%s\n",count, rc, in);
#ifdef linux
				perror("ERROR sending");
#else
				printf("ERROR sending! Error code: %d\n", WSAGetLastError() );
#endif
				close();
			}
		}
	}

	void Socket::writeTo( std::string in, std::string host, unsigned short port )
	{
		if( bValid && !bConnected )
		{
			connect( host, port );
			write( in );
		}
	}


	void Socket::close()
	{
		bConnected = false;
#ifdef linux
		::close(sockfd);
#else
		::closesocket(sockfd);
#endif
	}

	unsigned int Socket::getAddrFromString(const char* hostnameOrIp)
	{
		long rc;
		unsigned long ip;
		HOSTENT* he;

		/* Parameter prüfen */
		if(hostnameOrIp==NULL)
			return SOCKET_ERROR;

		/* eine IP in hostnameOrIp ? */
		ip=inet_addr(hostnameOrIp);

		/* bei einem fehler liefert inet_addr den Rückgabewert INADDR_NONE */
		if(ip!=INADDR_NONE)
		{
			return ip;
		}
		else
		{
			/* Hostname in hostnameOrIp auflösen */
			he = gethostbyname(hostnameOrIp);
			if(he==NULL)
			{
				return SOCKET_ERROR;
			}
			else
			{
				/*die 4 Bytes der IP von he nach addr kopieren */
				memcpy(&(addr.sin_addr),he->h_addr_list[0],4);
			}

			return addr.sin_addr.s_addr;
		}
	}

}