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
		bValid( true ),
		bConnected( true ),
		bufferSize( 1024 ),
		sockfd( socket ),
		m_addrData( sock_addr )
	{
	}

	Socket::Socket( int iSockType )
		: SocketStub(),
		bConnected( false ),
		bValid( true ),
		bufferSize( 1024 )
	{

		if( iSockType != SOCK_STREAM && iSockType != SOCK_DGRAM )
		{
			Log::getLog("oocl")->logMessage("Socket gets invalid protocoll type; defaults to TCP", Log::EL_WARNING );
			iSockType = SOCK_STREAM;
		}

		sockfd = socket(AF_INET, iSockType, 0);
#ifdef linux
		if (sockfd < 0)
#else
		if( sockfd == INVALID_SOCKET )
#endif
		{
    		bValid = false;
			Log::getLog("oocl")->logMessage("Creating socket failed", Log::EL_ERROR);
		}
	}

	bool Socket::connect( std::string host, unsigned short usPort )
	{
		return connect( getAddrFromString( host.c_str() ), usPort );
	}

	bool Socket::connect( unsigned int uiHostIP, unsigned short usPort )
	{
		if( bValid && !bConnected )
		{
			m_addrData.sin_addr.s_addr = uiHostIP;
			m_addrData.sin_family = AF_INET;
			m_addrData.sin_port = htons(usPort);

			int result = ::connect(sockfd,(struct sockaddr *) &m_addrData, sizeof(m_addrData));
#ifdef linux
			if( result < 0)
#else
			if( result == SOCKET_ERROR )
#endif
			{
				std::ostringstream os;
				os << "Connecting socket to " << m_addrData.sin_addr.s_net << "." << m_addrData.sin_addr.s_host << "." << m_addrData.sin_addr.s_lh << "." << m_addrData.sin_addr.s_impno << ":" << usPort << " failed";
				Log::getLog("oocl")->logMessage( os.str(), Log::EL_ERROR );

    			close();
				return false;
			}

			bConnected = true;

			return true;
		}
		
		std::ostringstream os;
		os << "Connecting to " << m_addrData.sin_addr.s_net << "." << m_addrData.sin_addr.s_host << "." << m_addrData.sin_addr.s_lh << "." << m_addrData.sin_addr.s_impno << ":" << usPort << " failed due to invalid socket";
		Log::getLog("oocl")->logMessage( os.str(), Log::EL_ERROR );

		return false;
	}

	bool Socket::bind( unsigned short usPort )
	{
		if( bValid && !bConnected )
		{
			m_addrData.sin_family = AF_INET;
			m_addrData.sin_port = htons(usPort);
			m_addrData.sin_addr.s_addr = INADDR_ANY;

			int result = ::bind(sockfd, (struct sockaddr *)&m_addrData, sizeof(m_addrData));
#ifdef linux
			if( result < 0)
#else
			if( result == SOCKET_ERROR )
#endif
			{
				std::ostringstream os;
				os << "Binding socket to port " << usPort << " failed";
				Log::getLog("oocl")->logMessage( os.str(), Log::EL_ERROR );

    			close();
				return false;
			}

			return true;
		}

		return false;
	}

	Socket::~Socket()
	{
		close();
		SocketStub::~SocketStub();
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
		if(bConnected && count>0)
		{
    		char * buffer = new char[count];
			int rc = ::recv(sockfd,buffer,count,0);
			if(readCount != NULL)
			{
				*readCount = rc;
			}

			if(rc > 0)
			{
				return buffer;
			}
			else if(rc < 0)
			{
				Log::getLog("oocl")->logMessage( "receiving on connected socket failed", Log::EL_ERROR );
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
			int fromlen = sizeof( sockaddr_in );
			struct sockaddr_in addr;

			int rc = ::recvfrom( sockfd,buffer,count,0, (sockaddr*)&addr, &fromlen);

			if(rc > 0)
			{
				if( hostIP != NULL )
					*hostIP = addr.sin_addr.s_addr;
				return std::string(buffer, rc);
			}
			else if(rc < 0)
			{
				rc = WSAGetLastError();
				Log::getLog("oocl")->logMessage( "receiving on connectionless socket failed", Log::EL_ERROR );
				close();
				return std::string();
			}
		}

		return std::string();
	}


	bool Socket::write(std::string in)
	{
		return writeCA(in.c_str(),in.length());
	}

	bool Socket::writeC(char in)
	{
		return writeCA(&in,1);
	}

	bool Socket::writeCA(const char * in, int count)
	{
		if( bConnected && count>0 )
		{
			int rc = ::send(sockfd,in,count,0);
			if(rc < 0)
			{
				std::ostringstream os;
				os << "Sending failed: " << count << ":" << rc << ":" << in;
				Log::getLog("oocl")->logMessage( os.str(), Log::EL_ERROR );
				close();
				// TODO: implement failcount, close connection after n failed sendings

				return false;
			}

			return true;
		}

		Log::getLog("oocl")->logMessage( "Sending failed due to unconnected socket", Log::EL_WARNING );

		return false;
	}

	bool Socket::writeTo( std::string in, std::string host, unsigned short port )
	{
		if( connect( host, port ) )
		{
			return write( in );
		}

		return false;
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
			struct sockaddr_in addr;

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