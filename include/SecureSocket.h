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
		
		virtual bool connect( std::string host, unsigned short usPort );
		virtual bool connect( unsigned int uiHostIP, unsigned short usPort );
		bool connect( std::string strHostPort );
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
		
		virtual int getCSocket() { return 0; }
		
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

