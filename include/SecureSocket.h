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
	
	class SecureSocket : public Socket
	{
	public:
		SecureSocket( bool bSecure = true );
		virtual ~SecureSocket();
		
		bool connect( std::string host, unsigned short usPort );
		bool connect( unsigned int uiHostIP, unsigned short usPort );
		bool connect( std::string strHostPort );
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
		
		int getCSocket() { return 0; }
		
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

