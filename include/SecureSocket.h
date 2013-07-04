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
// This file was written by Jörn Teuber

#ifndef SECURESOCKET_H
#define	SECURESOCKET_H

/* OpenSSL headers */

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "Socket.h"

namespace oocl
{
	/**
	 * @brief	Socket that uses OpenSSL to establish a secure connection.
	 *
	 * @author	Jörn Teuber
	 * @date	4.7.2013
	 */
	class SecureSocket : public Socket
	{
	public:
		SecureSocket( bool bSecure = true );
		virtual ~SecureSocket();
		
		virtual bool connect( std::string host, unsigned short usPort );
		virtual bool connect( unsigned int uiHostIP, unsigned short usPort );
		bool connect( std::string strHostPort );
		virtual bool bind( unsigned short usPort );

		virtual bool isValid();
		virtual bool isConnected();

		virtual bool read( std::string& str, int count = 0 );
		virtual bool read( char& c );
		virtual bool read( char* pcBuf, int& count );

		virtual bool readFrom( std::string& str, int count = 0, unsigned int* hostIP = NULL );


		virtual bool write(std::string in);
		virtual bool write(char in);
		virtual bool write(const char * in, int count);

		virtual bool writeTo( std::string in, std::string host, unsigned short port );

		virtual void close();
		
		virtual int getCSocket() { return 0; }
		
		virtual unsigned int getConnectedIP() { return *((unsigned int*)BIO_get_conn_ip(m_pBio)); }

	private:
		BIO* m_pBio;
		SSL* m_pSSL;
		bool m_bSecure;
		
		bool m_bValid;
		bool m_bConnected;
		
		std::string m_strHostPort;
		
		static SSL_CTX* sm_pCTX;
		static unsigned int sm_uiSocketCounter;
	};
	
}


#endif	/* SECURESOCKET_H */

