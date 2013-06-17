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

	SecureSocket::SecureSocket( bool bSecure )
			: 	m_pBio( NULL ),
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

			sm_pCTX = SSL_CTX_new( SSLv23_client_method() );
			if( !SSL_CTX_load_verify_locations( sm_pCTX, NULL, "certs" ) )
			{
				/* Handle failed load here */
				sm_pCTX = NULL;
				Log::getLog( "oocl" )->logFatalError( "OpenSSL TrustStore could not be loaded!" );
			}
		}
		sm_uiSocketCounter++;
	}

	SecureSocket::~SecureSocket()
	{
		sm_uiSocketCounter--;
		BIO_free_all( m_pBio );

		if( sm_uiSocketCounter == 0 )
		{
			SSL_CTX_free( sm_pCTX );
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
		char* cp = (char*) &uiHostIP;
		ss << cp[0] << "." << cp[1] << "." << cp[2] << "." << cp[3] << ":" << usPort;

		Log::getLog( "oocl" )->logInfo( "SecureSocket: built address string out of IP: " + ss.str() );

		return connect( ss.str() );
	}

	bool SecureSocket::connect( std::string strHostPort )
	{
		m_strHostPort = strHostPort;

		if( m_bSecure )
		{
			m_pBio = BIO_new_ssl_connect( sm_pCTX );
			m_pSSL = SSL_new( sm_pCTX );
			SSL_set_mode( m_pSSL, SSL_MODE_AUTO_RETRY );

			char* pTemp = new char[m_strHostPort.size() + 1];
			memcpy( pTemp, m_strHostPort.c_str(), m_strHostPort.size() + 1 );

			BIO_set_conn_hostname( m_pBio, pTemp );
		}
		else
		{
			char* pTemp = new char[m_strHostPort.size() + 1];
			memcpy( pTemp, m_strHostPort.c_str(), m_strHostPort.size() + 1 );

			m_pBio = BIO_new_connect( pTemp );
		}

		if( m_pBio == NULL )
		{
			/* Handle the failure */
			Log::getLog( "oocl" )->logError( "connecting failed!" );
			Log::getLog( "oocl" )->logError( ERR_error_string( ERR_get_error(), NULL ) );
			m_bValid = false;
			return false;
		}

		if( BIO_do_connect(m_pBio) <= 0 )
		{
			/* Handle failed connection */
			Log::getLog( "oocl" )->logError( "connecting failed!" );
			Log::getLog( "oocl" )->logError( ERR_error_string( ERR_get_error(), NULL ) );
			m_bValid = false;
			return false;
		}

		if( m_bSecure )
		{
			if( BIO_do_handshake(m_pBio) <= 0 )
			{
				Log::getLog( "oocl" )->logError( "Error establishing SSL connection" );
				return false;
			}
			if( SSL_get_verify_result( m_pSSL ) != X509_V_OK )
			{
				/* Handle the failed verification */
				Log::getLog( "oocl" )->logError( "SSL certificate verification failed!" );
			}
		}

		m_bConnected = true;
		return true;
	}

	bool SecureSocket::bind( unsigned short usPort )
	{
		return false;
	}

	bool SecureSocket::isValid()
	{
		return m_bValid;
	}

	bool SecureSocket::isConnected()
	{
		return m_bConnected;
	}

	bool SecureSocket::read( std::string& str, int count )
	{
		if( m_bConnected )
		{
			if( count == 0 )
				count = 1024;

			int readCount = count;
			char* pcBuffer = new char[count];

			if( !read( pcBuffer, readCount ) )
			{
				delete[] pcBuffer;
				return false;
			}

			str = std::string( pcBuffer, readCount );
			delete[] pcBuffer;
			return true;
		}

		return false;
	}

	bool SecureSocket::read( char& c )
	{
		if( m_bConnected )
		{
			int count = 1;

			return read( &c, count );
		}

		return false;
	}

	bool SecureSocket::read( char* pcBuf, int& count )
	{
		if( m_bConnected )
		{
			int x = 0;
			try
			{
				x = BIO_read( m_pBio, pcBuf, count );
			}
			catch( ... )
			{
				Log::getLog( "oocl" )->logWarning( "read failed, aborting" );
				return false;
			}

			if( x == 0 )
			{
				//			close();
				//			if( connect( m_strHostPort ) )
				//			{
				//				x = BIO_read(m_pBio, cpBuffer, count);
				//
				//				if( x <= 0 )
				//				{
				//					m_bConnected = false;
				//					return NULL;
				//				}
				//			}
				//			else
				//			{
				//				m_bConnected = false;
				return false;
				//			}
			}
			else if( x == -1 )
			{
				if( !BIO_should_retry(m_pBio) )
				{
					/* Handle failed read here */
					m_bConnected = false;
					return false;
				}

				/* Do something to handle the retry */
				if( connect( m_strHostPort ) )
				{
					x = BIO_read( m_pBio, pcBuf, count );
					if( x <= 0 )
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
			else if( x == -2 )
			{
				return false;
			}

			count = x;


			return true;
		}

		return false;
	}

	bool SecureSocket::readFrom( std::string& str, int count, unsigned int* hostIP )
	{
		return true;
	}

	bool SecureSocket::write( std::string in )
	{
		return write( in.c_str(), in.length() );
	}

	bool SecureSocket::write( char in )
	{
		return write( &in, 1 );
	}

	bool SecureSocket::write( const char * in, int count )
	{
		if( m_bConnected )
		{
			if( BIO_write( m_pBio, in, count ) <= 0 )
			{
				if( !BIO_should_retry(m_pBio) )
				{
					/* Handle failed write here */
					m_bConnected = false;
					return false;
				}

				/* Do something to handle the retry */
				if( connect( m_strHostPort ) )
				{
					if( BIO_write( m_pBio, in, count ) <= 0 )
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

		return false;
	}

	bool SecureSocket::writeTo( std::string in, std::string host, unsigned short port )
	{
		return false;
	}

	void SecureSocket::close()
	{
		try
		{
			BIO_reset( m_pBio );
		}
		catch( ... )
		{
			Log::getLog( "oocl" )->logWarning( "OpenSSL reset threw an exception" );
		}
		m_bConnected = false;
	}
}
