/* 
 * File:   SecureSocket.h
 * Author: jay
 *
 * Created on May 17, 2012, 1:34 AM
 */

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
	 * @date	7/4/2013
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

