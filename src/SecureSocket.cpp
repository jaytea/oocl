/* 
 * File:   SecureSocket.cpp
 * Author: jay
 * 
 * Created on May 17, 2012, 1:34 AM
 */
#include "SecureSocket.h"

namespace oocl
{
	unsigned int SecureSocket::sm_uiSocketCounter = 0;
	SSL_CTX* SecureSocket::sm_pCTX = NULL;

	SecureSocket::SecureSocket( bool bSecure ) :
		m_pBio( NULL ),
		m_bSecure( bSecure ),
		m_bValid( true ),
		m_bConnected( false )
	{
		if( sm_pCTX == NULL )
		{
			SSL_library_init();
			SSL_load_error_strings();
			ERR_load_BIO_strings();
			OpenSSL_add_all_algorithms();
			
			sm_pCTX = SSL_CTX_new(SSLv23_client_method());
			if( !SSL_CTX_load_verify_locations(sm_pCTX, NULL, "certs") )
			{
				/* Handle failed load here */
				sm_pCTX = NULL;
				Log::getLog("oocl")->logFatalError( "OpenSSL TrustStore could not be loaded!" );
			}
		}
		sm_uiSocketCounter++;
	}

	SecureSocket::~SecureSocket() 
	{
		sm_uiSocketCounter--;
		BIO_free_all(m_pBio);
		
		if( sm_uiSocketCounter == 0 )
		{
			SSL_CTX_free(sm_pCTX);
		}
	}

	bool SecureSocket::connect( std::string host, unsigned short usPort )
	{
		std::stringstream ss;
		ss << host << ":" << usPort;
		
		return connect( ss.str() );
	}
	
	bool SecureSocket::connect( unsigned int uiHostIP, unsigned short usPort )
	{
		std::stringstream ss;
		char* cp = (char*)&uiHostIP;
		ss << cp[0] << "." << cp[1] << "." << cp[2] << "." << cp[3] << ":" << usPort;
		std::cout << ss << std::endl;
		
		return connect( ss.str() );
	}
	
	bool SecureSocket::connect( std::string strHostPort )
	{
		m_strHostPort = strHostPort;
		
		if( m_bSecure )
		{
			m_pBio = BIO_new_ssl_connect(sm_pCTX);
			m_pSSL = SSL_new(sm_pCTX);
			SSL_set_mode(m_pSSL, SSL_MODE_AUTO_RETRY);
			
			char* pTemp = new char[m_strHostPort.size()];
			memcpy( pTemp, m_strHostPort.c_str(), m_strHostPort.size()+1 );
			
			BIO_set_conn_hostname( m_pBio, pTemp );
		} 
		else
		{
			char* pTemp = new char[m_strHostPort.size()];
			memcpy( pTemp, m_strHostPort.c_str(), m_strHostPort.size()+1 );
			
			m_pBio = BIO_new_connect( pTemp );
		}
		
		if(m_pBio == NULL)
		{
			/* Handle the failure */
			Log::getLog("oocl")->logError( "connecting failed!" );
			Log::getLog("oocl")->logError( ERR_error_string(ERR_get_error(), NULL) );
			m_bValid = false;
			return false;
		}

		if(BIO_do_connect(m_pBio) <= 0)
		{
			/* Handle failed connection */
			Log::getLog("oocl")->logError( "connecting failed!" );
			Log::getLog("oocl")->logError( ERR_error_string(ERR_get_error(), NULL) );
			m_bValid = false;
			return false;
		}
		
		if( m_bSecure )
		{
			if(BIO_do_handshake(m_pBio) <= 0) 
			{
				Log::getLog("oocl")->logError( "Error establishing SSL connection" );
				return false;
			}
			if(SSL_get_verify_result(m_pSSL) != X509_V_OK)
			{
				/* Handle the failed verification */
				Log::getLog("oocl")->logError( "SSL certificate verification failed!" );
			}
		}
		
		m_bConnected = true;
		return true;
	}
	
	bool SecureSocket::bind( unsigned short usPort )
	{
	}
	

	bool SecureSocket::isValid()
	{
		return m_bValid;
	}
	
	bool SecureSocket::isConnected()
	{
		return m_bConnected;
	}
	

	std::string SecureSocket::read(int count)
	{
		if(count==0){
			count = 10240;
		}
		
		int readCount;
		char* cpBuffer = readCA( count, &readCount );
		
		if( cpBuffer == NULL )
			return "";
		
		return std::string( cpBuffer, readCount );
	}
	
	char SecureSocket::readC()
	{
		char* c;
		c = readCA( 1 );
		
		if( c == NULL )
			return 0;
		
		return *c;
	}
	
	char* SecureSocket::readCA(int count, int * readCount)
	{
		if(count==0){
			count = 1024;
		}
		char* cpBuffer = new char[count];
		
		int x = BIO_read(m_pBio, cpBuffer, count);
		if(x == 0)
		{
			if( connect( m_strHostPort ) )
			{
				x = BIO_read(m_pBio, cpBuffer, count);
				if( x <= 0 )
				{
					m_bConnected = false;
					return NULL;
				}
			}
			else
			{
				m_bConnected = false;
				return NULL;
			}
		}
		else if(x < 0)
		{
			if(! BIO_should_retry(m_pBio))
			{
				/* Handle failed read here */
				m_bConnected = false;
				return NULL;
			}

			/* Do something to handle the retry */
			if( connect( m_strHostPort ) )
			{
				x = BIO_read(m_pBio, cpBuffer, count);
				if( x <= 0 ) 
				{
					m_bConnected = false;
					return NULL;
				}
			}
			else
			{
				m_bConnected = false;
				return NULL;
			}
		}
		
		if( readCount != NULL )
		{
			*readCount = x;
		}
		
		return cpBuffer;
	}
	

	std::string SecureSocket::readFrom( int count, unsigned int* hostIP )
	{
	}


	bool SecureSocket::write(std::string in)
	{
		return writeCA( in.c_str(), in.length() );
	}
	
	bool SecureSocket::writeC(char in)
	{ 
		return writeCA( &in, 1 );
	}
	
	bool SecureSocket::writeCA(const char * in, int count)
	{
		if( BIO_write(m_pBio, in, count) <= 0 )
		{
			if(! BIO_should_retry(m_pBio))
			{
				/* Handle failed write here */
				m_bConnected = false;
				return false;
			}

			/* Do something to handle the retry */
			if( connect( m_strHostPort ) )
			{
				if( BIO_write(m_pBio, in, count) <= 0 ) 
				{
					m_bConnected = false;
					return false;
				}
			}
			else
			{
				m_bConnected = false;
				return false;
			}
		}
		
		return true;
	}
	

	bool SecureSocket::writeTo( std::string in, std::string host, unsigned short port )
	{
	}
	

	void SecureSocket::close()
	{
		BIO_reset(m_pBio);
		m_bConnected = false;
	}
}