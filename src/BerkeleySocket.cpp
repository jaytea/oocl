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
// This file was written by J�rgen Lorenz and J�rn Teuber
#include "BerkeleySocket.h"

namespace oocl
{

	int BerkeleySocket::sm_iSocketCounter = 0;

	/**
	 * @fn	BerkeleySocket::BerkeleySocket(int socket, struct sockaddr_in sock_addr)
	 *
	 * @brief	Private socket constructor for the server socket, uses an existing c socket.
	 *
	 * @param	socket   	The c socket.
	 * @param	sock_addr	The address as returned by accept.
	 */
	BerkeleySocket::BerkeleySocket( int socket, struct sockaddr_in sock_addr )
		: 	Socket()
		, m_addrData( sock_addr )
		, m_iSockFD( socket )
		, m_iSockType( 0 )
		, m_bValid( true )
		, m_bConnected( true )
	{
#ifdef WIN32
		int iLength = sizeof(int);
#else
		unsigned int iLength = sizeof(int);
#endif
		getsockopt( m_iSockFD, SOL_SOCKET, SO_TYPE, (char*) &m_iSockType, &iLength );
	}

	/**
	 * @fn	BerkeleySocket::BerkeleySocket( int iSockType )
	 *
	 * @brief	Public constructor for a new, clean and unconnected socket.
	 *
	 * @param	iSockType	Protocoll used by the socket, TCP = SOCK_STREAM, UDP = SOCK_DGRAM.
	 */
	BerkeleySocket::BerkeleySocket( int iSockType )
		: 	Socket()
		, m_iSockType( iSockType )
		, m_bValid( true )
		, m_bConnected( false )
	{

		if( iSockType != SOCK_STREAM && iSockType != SOCK_DGRAM )
		{
			Log::getLog( "oocl" )->logWarning( "Socket got invalid protocoll type; defaults to TCP" );
			iSockType = SOCK_STREAM;
		}

		m_iSockFD = socket( AF_INET, iSockType, 0 );
#ifdef linux
		if( m_iSockFD < 0 )
#else
		if( m_iSockFD == INVALID_SOCKET )
#endif
		{
			m_bValid = false;
			Log::getLog( "oocl" )->logError( "Creating socket failed" );
		}
	}

	/**
	 * @fn	bool BerkeleySocket::connect( std::string host, unsigned short usPort )
	 *
	 * @brief	Connects the socket to the given Peer.
	 *
	 * @param	host  	The host.
	 * @param	usPort	The port.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool BerkeleySocket::connect( std::string host, unsigned short usPort )
	{
		return connect( getAddrFromString( host.c_str() ), usPort );
	}

	/**
	 * @fn	bool BerkeleySocket::connect( unsigned int uiHostIP, unsigned short usPort )
	 *
	 * @brief	Connects the socket to the given Peer.
	 *
	 * @param	uiHostIP	The host ip.
	 * @param	usPort  	The port.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool BerkeleySocket::connect( unsigned int uiHostIP, unsigned short usPort )
	{
		if( m_bValid && !m_bConnected )
		{
			m_addrData.sin_addr.s_addr = uiHostIP;
			m_addrData.sin_family = AF_INET;
			m_addrData.sin_port = htons( usPort );

			int result = ::connect( m_iSockFD, (struct sockaddr *) &m_addrData, sizeof(m_addrData) );
#ifdef linux
			if( result < 0 )
			{
				std::ostringstream os;
				os << "Connecting socket to " << m_addrData.sin_addr.s_addr << ":" << usPort << " failed";
#else
				if( result == SOCKET_ERROR )
				{
					std::ostringstream os;
					os << "Connecting socket to " << (int)m_addrData.sin_addr.s_net << "." << (int)m_addrData.sin_addr.s_host << "." << (int)m_addrData.sin_addr.s_lh << "." << (int)m_addrData.sin_addr.s_impno << ":" << usPort << " failed";
#endif
				Log::getLog( "oocl" )->logError( os.str() );

				close();
				return false;
			}

			m_bConnected = true;

			return true;
		}

		std::ostringstream os;
#ifdef linux
		os << "Connecting to " << m_addrData.sin_addr.s_addr << ":" << usPort << " failed due to invalid socket";
#else
		os << "Connecting to " << (int)m_addrData.sin_addr.s_net << "." << (int)m_addrData.sin_addr.s_host << "." << (int)m_addrData.sin_addr.s_lh << "." << (int)m_addrData.sin_addr.s_impno << ":" << usPort << " failed due to invalid socket";
#endif
		Log::getLog( "oocl" )->logError( os.str() );

		return false;
	}

	/**
	 * @fn	bool BerkeleySocket::bind( unsigned short usPort )
	 *
	 * @brief	Binds the socket to the given port, used for UDP listening.
	 *
	 * @param	usPort	The port.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool BerkeleySocket::bind( unsigned short usPort )
	{
		if( m_bValid && !m_bConnected )
		{
			m_addrData.sin_family = AF_INET;
			m_addrData.sin_port = htons( usPort );
			m_addrData.sin_addr.s_addr = INADDR_ANY;

			int result = ::bind( m_iSockFD, (struct sockaddr *) &m_addrData, sizeof(m_addrData) );
#ifdef linux
			if( result < 0 )
#else
			if( result == SOCKET_ERROR )
#endif
			{
				std::ostringstream os;
				os << "Binding socket to port " << usPort << " failed";
				Log::getLog( "oocl" )->logError( os.str() );

				close();
				return false;
			}

			m_bConnected = true;

			return true;
		}

		return false;
	}

	/**
	 * @fn	BerkeleySocket::~BerkeleySocket()
	 *
	 * @brief	Destructor.
	 */
	BerkeleySocket::~BerkeleySocket()
	{
		close();
	}

	/**
	 * @fn	bool BerkeleySocket::isValid()
	 *
	 * @brief	Query if this socket is valid, i.e. the C socket could be created.
	 *
	 * @return	true if valid, false if not.
	 */
	bool BerkeleySocket::isValid()
	{
		return m_bValid;
	}

	/**
	 * @fn	bool BerkeleySocket::isConnected()
	 *
	 * @brief	Query if this socket is connected.
	 *
	 * @return	true if connected, false if not.
	 */
	bool BerkeleySocket::isConnected()
	{
		// if this socket seems to be connected and is a tcp socket, check it on the c socket
		if( m_bConnected && m_iSockType == SOCK_STREAM )
		{
			int iStatus = 0;
#ifdef WIN32
			int iLength = sizeof(int);
#else
			unsigned int iLength = sizeof(int);
#endif
			getsockopt( m_iSockFD, SOL_SOCKET, SO_ERROR, (char*) &iStatus, &iLength );

			if( iStatus > 0 )
			{
				Log::getLogRef( "oocl" ) << Log::EL_ERROR << "Socket discovered an error. ErrNo: " << iStatus << endl;
				close();
			}
		}

		return m_bConnected;
	}

	/**
	 * @fn	bool BerkeleySocket::read( std::string& str, int count )
	 *
	 * @brief	Receives a package of maximum length count and stores it in a given string.
	 *
	 * @param	str		The string to write the received bytes into.
	 * @param	count	The number of bytes to receive.
	 *
	 * @return	false if encountered any errors, else true.
	 */
	bool BerkeleySocket::read( std::string& str, int count )
	{
		if( count == 0 )
			count = MAX_BUFFER_SIZE;

		int readCount = count;
		char* in = new char[count];

		bool bRet = read( in, readCount );
		if( bRet )
			str = std::string( in, readCount );

		delete[] in;
		return bRet;
	}

	/**
	 * @fn	bool BerkeleySocket::read( char& c )
	 *
	 * @brief	Receive exactly one byte.
	 *
	 * @param	c	The reference to the read byte.
	 *
	 * @return	false if encountered any errors, else true.
	 */
	bool BerkeleySocket::read( char& c )
	{
		int count = 1;
		return read( &c, count );
	}

	/**
	 * @brief	Receives a package with max count size, returns a char array and stores the number of actually received bytes in readCount.
	 *
	 * @param	pcBuf	Pre-allocated buffer in which the received data will be written.
	 * @param  	count	The size of the buffer, will be set to the number of actually received bytes.
	 *
	 * @return	false if encountered any errors, else true.
	 */
	bool BerkeleySocket::read( char* pcBuf, int& count )
	{
		if( m_bConnected && count > 0 )
		{
			int rc = ::recv( m_iSockFD, pcBuf, count, 0 );
			count = rc;

			if( rc > 0 )
			{
				return true;
			}
			else if( rc < 0 )
			{
				Log::getLog( "oocl" )->logError( "receiving on connected socket failed" );
				close();
			}
		}
		return false;
	}

	/**
	 * @brief	Receives from an unconnected, i.e. UDP socket.
	 *
	 * @param	str		[out] The received bytes as string.
	 * @param	count	Maximum number of bytes to read.
	 * @param 	hostIP	If non-null, the ip of the peer that sent the package.
	 *
	 * @return	false if encountered any errors, else true.
	 */
	bool BerkeleySocket::readFrom( std::string& str, int count, unsigned int* hostIP )
	{
		if( m_bValid && !m_bConnected )
		{
			if( count == 0 )
				count = MAX_BUFFER_SIZE;

			char * buffer = new char[count];
#ifdef WIN32
			int fromlen = sizeof( sockaddr_in );
#else
			socklen_t fromlen = sizeof(sockaddr_in);
#endif
			struct sockaddr_in addr;

			int rc = ::recvfrom( m_iSockFD, buffer, count, 0, (sockaddr*) &addr, &fromlen );

			if( rc > 0 )
			{
				if( hostIP != NULL )
					*hostIP = addr.sin_addr.s_addr;
				str = std::string( buffer, rc );
				return true;
			}
			else if( rc < 0 )
			{
				Log::getLog( "oocl" )->logError( "receiving on connectionless socket failed" );
				close();
				return false;
			}
		}

		return false;
	}

	/**
	 * @fn	bool BerkeleySocket::write(std::string in)
	 *
	 * @brief	Send a package to the connected process.
	 *
	 * @param	in	The package as string.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool BerkeleySocket::write( std::string in )
	{
		return write( in.c_str(), in.length() );
	}

	/**
	 * @fn	bool BerkeleySocket::write(char in)
	 *
	 * @brief	Sends one byte to the connected process.
	 *
	 * @param	in	The byte to send.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool BerkeleySocket::write( char in )
	{
		return write( &in, 1 );
	}

	/**
	 * @fn	bool BerkeleySocket::write(const char * in, int count)
	 *
	 * @brief	Sends a byte array to the connected process.
	 *
	 * @param	in   	The byte array.
	 * @param	count	Number of bytes to send.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool BerkeleySocket::write( const char * in, int count )
	{
		if( m_bConnected && count > 0 )
		{
			unsigned int iBytesSent = 0;
			while( iBytesSent < count )
			{
				int rc = ::send( m_iSockFD, in + iBytesSent, count - iBytesSent, 0 );
				if( rc < 0 )
				{
					std::ostringstream os;
					os << "Sending failed: " << count << ":" << rc << ":" << in;
					Log::getLog( "oocl" )->logError( os.str() );
					close();
					// TODO: implement fail-count, close connection after n failed sends

					return false;
				}

				iBytesSent += rc;
			}

			return true;
		}

		Log::getLog( "oocl" )->logWarning( "Sending failed due to unconnected socket" );

		return false;
	}

	/**
	 * @fn	bool BerkeleySocket::writeTo( std::string in, std::string host, unsigned short port )
	 *
	 * @brief	Sends a package .
	 *
	 * @param	in  	The package to send as string.
	 * @param	host	The host to send to.
	 * @param	port	The port of the host.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool BerkeleySocket::writeTo( std::string in, std::string host, unsigned short port )
	{
		if( connect( host, port ) )
		{
			bool ret = write( in );
			close();
			return ret;
		}

		return false;
	}

	/**
	 * @fn	void BerkeleySocket::close()
	 *
	 * @brief	Closes this socket.
	 */
	void BerkeleySocket::close()
	{
		m_bConnected = false;
#ifdef linux
		shutdown( m_iSockFD, 2 );
//		::close(sockfd);
#else
		::closesocket(m_iSockFD);
#endif
	}


	/**
	 * @fn	int BerkeleySocket::getCSocket()
	 *
	 * @brief	Get the sockets underlying C or Berkeley socket.
	 *
	 * @return	The Sockets underlying CSocket.
	 */
	int BerkeleySocket::getCSocket()
	{
		return m_iSockFD;
	}

	/**
	 * @fn	unsigned int BerkeleySocket::getConnectedIP()
	 *
	 * @brief	Get the IP this socket is connected to.
	 *
	 * @return	The IP this socket is connected to.
	 */
	unsigned int BerkeleySocket::getConnectedIP()
	{
		return m_addrData.sin_addr.s_addr;
	}


	/**
	 * @fn	unsigned int BerkeleySocket::getAddrFromString(const char* hostnameOrIp)
	 *
	 * @brief	Gets the IP from a string, this can be an IP as string or a domain name.
	 *
	 * @param	hostnameOrIp	The hostname or ip.
	 *
	 * @return	The address from string.
	 */
	unsigned int BerkeleySocket::getAddrFromString( const char* hostnameOrIp )
	{
		unsigned long ip;
		hostent* he;

		/* Parameter pr�fen */
		if( hostnameOrIp == NULL )
			return 0;

		/* eine IP in hostnameOrIp ? */
		ip = inet_addr( hostnameOrIp );

		/* bei einem fehler liefert inet_addr den R�ckgabewert INADDR_NONE */
		if( ip != INADDR_NONE )
		{
			return ip;
		}
		else
		{
			struct sockaddr_in addr;

			/* Hostname in hostnameOrIp auflösen */
			he = gethostbyname( hostnameOrIp );
			if( he == NULL )
			{
				return 0;
			}
			else
			{
				/*die 4 Bytes der IP von he nach addr kopieren */
				std::memcpy( &(addr.sin_addr), he->h_addr_list[0], 4 );
			}

			return addr.sin_addr.s_addr;
		}
	}

}
