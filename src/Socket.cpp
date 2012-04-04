/*
Object Oriented Communication Library
Copyright (c) 2011 J�rgen Lorenz and J�rn Teuber

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
// This file was written by J�rgen Lorenz and J�rn Teuber

#include "Socket.h"

namespace oocl
{

	int Socket::iSocketCounter = 0;


	/**
	 * @fn	Socket::Socket(int socket, struct sockaddr_in sock_addr)
	 *
	 * @brief	Private socket constructor for the server socket, uses an existing c socket.
	 *
	 * @author	J�rn Teuber
	 * @date	3/2/2012
	 *
	 * @param	socket   	The c socket.
	 * @param	sock_addr	The address as returned by accept.
	 */
	Socket::Socket(int socket, struct sockaddr_in sock_addr)
		: SocketStub(),
		bValid( true ),
		bConnected( true ),
		bufferSize( 1024 ),
		sockfd( socket ),
		m_addrData( sock_addr )
	{
	}


	/**
	 * @fn	Socket::Socket( int iSockType )
	 *
	 * @brief	Public constructor for a new, clean and unconnected socket.
	 *
	 * @author	J�rn Teuber
	 * @date	3/2/2012
	 *
	 * @param	iSockType	Protocoll used by the socket, TCP = SOCK_STREAM, UDP = SOCK_DGRAM.
	 */
	Socket::Socket( int iSockType )
		: SocketStub(),
		bConnected( false ),
		bValid( true ),
		bufferSize( 1024 )
	{

		if( iSockType != SOCK_STREAM && iSockType != SOCK_DGRAM )
		{
			Log::getLog("oocl")->logWarning("Socket gets invalid protocoll type; defaults to TCP" );
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
			Log::getLog("oocl")->logError("Creating socket failed");
		}
	}


	/**
	 * @fn	bool Socket::connect( std::string host, unsigned short usPort )
	 *
	 * @brief	Connects the socket to the given Peer.
	 *
	 * @param	host  	The host.
	 * @param	usPort	The port.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Socket::connect( std::string host, unsigned short usPort )
	{
		return connect( getAddrFromString( host.c_str() ), usPort );
	}


	/**
	 * @fn	bool Socket::connect( unsigned int uiHostIP, unsigned short usPort )
	 *
	 * @brief	Connects the socket to the given Peer.
	 *
	 * @param	uiHostIP	The host ip.
	 * @param	usPort  	The port.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
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
			{
				std::ostringstream os;
				os << "Connecting socket to " << m_addrData.sin_addr.s_addr << ":" << usPort << " failed";
#else
			if( result == SOCKET_ERROR )
			{
				std::ostringstream os;
				os << "Connecting socket to " << m_addrData.sin_addr.s_net << "." << m_addrData.sin_addr.s_host << "." << m_addrData.sin_addr.s_lh << "." << m_addrData.sin_addr.s_impno << ":" << usPort << " failed";
#endif
				Log::getLog("oocl")->logError( os.str() );

    			close();
				return false;
			}

			bConnected = true;

			return true;
		}
		
		std::ostringstream os;
#ifdef linux
		os << "Connecting to " << m_addrData.sin_addr.s_addr << ":" << usPort << " failed due to invalid socket";
#else
		os << "Connecting to " << m_addrData.sin_addr.s_net << "." << m_addrData.sin_addr.s_host << "." << m_addrData.sin_addr.s_lh << "." << m_addrData.sin_addr.s_impno << ":" << usPort << " failed due to invalid socket";
#endif
		Log::getLog("oocl")->logError( os.str() );

		return false;
	}


	/**
	 * @fn	bool Socket::bind( unsigned short usPort )
	 *
	 * @brief	Binds the socket to the given port, used for UDP listening.
	 *
	 * @param	usPort	The port.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
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
				Log::getLog("oocl")->logError( os.str() );

    			close();
				return false;
			}

			bConnected = true;

			return true;
		}

		return false;
	}


	/**
	 * @fn	Socket::~Socket()
	 *
	 * @brief	Destructor.
	 */
	Socket::~Socket()
	{
		close();
	}


	/**
	 * @fn	bool Socket::isValid()
	 *
	 * @brief	Query if this socket is valid, i.e. the C socket could not be created.
	 *
	 * @return	true if valid, false if not.
	 */
	bool Socket::isValid()
	{
		return bValid;
	}


	/**
	 * @fn	bool Socket::isConnected()
	 *
	 * @brief	Query if this socket is connected.
	 *
	 * @return	true if connected, false if not.
	 */
	bool Socket::isConnected()
	{
		return bConnected;
	}


	/**
	 * @fn	std::string Socket::read(int count)
	 *
	 * @brief	Receives a package of count length at max and returns it as string.
	 *
	 * @param	count	The number of bytes to receive.
	 *
	 * @return	The received bytes as string.
	 */
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


	/**
	 * @fn	char Socket::readC()
	 *
	 * @brief	Receive exactly one byte.
	 *
	 * @return	The received byte.
	 */
	char Socket::readC()
	{
		char * in = readCA(1);
		if(in!=NULL){
			return in[0];
		}

		return 0;
	}


	/**
	 * @fn	char * Socket::readCA(int count, int * readCount)
	 *
	 * @brief	Receives a package with max count size, returns a char array and stores the number of actually received bytes in readCount.
	 *
	 * @param	count			 	Maximum number of bytes to read.
	 * @param [out]		readCount	If non-null, contains the number of actually received bytes.
	 *
	 * @return	null if it fails, else the ca.
	 */
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
				Log::getLog("oocl")->logError( "receiving on connected socket failed" );
				close();
			}
		}
		return NULL;
	}


	/**
	 * @fn	std::string Socket::readFrom( int count, unsigned int* hostIP )
	 *
	 * @brief	Receives from an unconnected, i.e. UDP socket.
	 *
	 * @param	count		  	Maximum number of bytes to read.
	 * @param [out]		hostIP	If non-null, the ip of the peer that sent the package.
	 *
	 * @return	The received bytes as string.
	 */
	std::string Socket::readFrom( int count, unsigned int* hostIP )
	{
		if( bValid && !bConnected )
		{
			if(count==0){
				count = bufferSize;
			}

			char * buffer = new char[count];
#ifdef _MSC_VER
			int fromlen = sizeof( sockaddr_in );
#else
			socklen_t fromlen = sizeof( sockaddr_in );
#endif
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
				Log::getLog("oocl")->logError( "receiving on connectionless socket failed" );
				close();
				return std::string();
			}
		}

		return std::string();
	}


	/**
	 * @fn	bool Socket::write(std::string in)
	 *
	 * @brief	Send a package to the connected process.
	 *
	 * @param	in	The package as string.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Socket::write(std::string in)
	{
		return writeCA(in.c_str(),in.length());
	}


	/**
	 * @fn	bool Socket::writeC(char in)
	 *
	 * @brief	Sends one byte to the connected process.
	 *
	 * @param	in	The byte to send.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Socket::writeC(char in)
	{
		return writeCA(&in,1);
	}


	/**
	 * @fn	bool Socket::writeCA(const char * in, int count)
	 *
	 * @brief	Sends a byte array to the connected process.
	 *
	 * @param	in   	The byte array.
	 * @param	count	Number of bytes to send.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Socket::writeCA(const char * in, int count)
	{
		if( bConnected && count>0 )
		{
			int rc = ::send(sockfd,in,count,0);
			if(rc < 0)
			{
				std::ostringstream os;
				os << "Sending failed: " << count << ":" << rc << ":" << in;
				Log::getLog("oocl")->logError( os.str() );
				close();
				// TODO: implement failcount, close connection after n failed sendings

				return false;
			}

			return true;
		}

		Log::getLog("oocl")->logWarning( "Sending failed due to unconnected socket" );

		return false;
	}


	/**
	 * @fn	bool Socket::writeTo( std::string in, std::string host, unsigned short port )
	 *
	 * @brief	Sends a package .
	 *
	 * @param	in  	The package to send as string.
	 * @param	host	The host to send to.
	 * @param	port	The port of the host.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool Socket::writeTo( std::string in, std::string host, unsigned short port )
	{
		// TODO!: this is totally wrong! It can only connect once, the second call of this won't send anything
		if( connect( host, port ) )
		{
			return write( in );
		}

		return false;
	}


	/**
	 * @fn	void Socket::close()
	 *
	 * @brief	Closes this socket.
	 */
	void Socket::close()
	{
		bConnected = false;
#ifdef linux
		::close(sockfd);
#else
		::closesocket(sockfd);
#endif
	}


	/**
	 * @fn	unsigned int Socket::getAddrFromString(const char* hostnameOrIp)
	 *
	 * @brief	Gets the IP from a string, this can be an IP as string or a domain name.
	 *
	 * @param	hostnameOrIp	The hostname or ip.
	 *
	 * @return	The address from string.
	 */
	unsigned int Socket::getAddrFromString(const char* hostnameOrIp)
	{
		unsigned long ip;
		hostent* he;

		/* Parameter pr�fen */
		if(hostnameOrIp==NULL)
			return 0;

		/* eine IP in hostnameOrIp ? */
		ip=inet_addr(hostnameOrIp);

		/* bei einem fehler liefert inet_addr den R�ckgabewert INADDR_NONE */
		if(ip!=INADDR_NONE)
		{
			return ip;
		}
		else
		{
			struct sockaddr_in addr;

			/* Hostname in hostnameOrIp aufl�sen */
			he = gethostbyname(hostnameOrIp);
			if(he==NULL)
			{
				return 0;
			}
			else
			{
				/*die 4 Bytes der IP von he nach addr kopieren */
				std::memcpy(&(addr.sin_addr),he->h_addr_list[0],4);
			}

			return addr.sin_addr.s_addr;
		}
	}

}