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

#include "HttpClient.h"

namespace oocl
{
	
	/**
	 * @brief	Simple standard constructor.
     */
	HttpClient::HttpClient() :
		m_pSocket( NULL ),
		m_strHttpVersion( "HTTP/1.1" ),
		m_bConnected( false ),
		m_bStreaming( false ),
		m_psbBuffer( NULL )
	{
		signal( SIGPIPE, HttpClient::handleBrokenPipe );
	}

	/**
	 * @brief	Destructor.
     */
	HttpClient::~HttpClient() 
	{
		disconnect();
		
		delete m_pSocket;
		m_pSocket = NULL;
	}


	/**
	 * @brief	Connects to a http server by standard web URL.
	 * 
	 * @note	This function needs the protocol identifier (http/https) at the beginning of strHostname. When given https it will connect via secure socket.
	 * 
     * @param	strHostname		URL of the server to connect to, with the protocol identifier at the beginning.
	 * 
     * @return	True if connected successfully, false if not.
     */
	bool HttpClient::connect( std::string strHostname ) 
	{
		m_bSecure = false;
		unsigned int uiProtocollEnd = strHostname.find("://");
		
		std::string strProtokoll;
		
		// when no protocol is specified in the hostname assume plain http
		if( uiProtocollEnd > strHostname.size() )
		{
			m_strHost = strHostname;
			strProtokoll = "http";
		}
		// if a protocol is given, analyze it
		else
		{
			strProtokoll = strHostname.substr( 0, uiProtocollEnd );
			m_strHost = strHostname.substr( uiProtocollEnd+3 );
		
			if( strProtokoll == "https" )
				m_bSecure = true;
		}
		
		m_pSocket = new SecureSocket( m_bSecure );
		if( m_pSocket->connect( m_strHost + ":" + strProtokoll ) )
		{
			m_bConnected = true;
			m_strHostname = strHostname;
			m_strHostPort = m_strHost + ":" + strProtokoll;
			
//			m_ptLastConnect = new timespec;
//			clock_gettime(CLOCK_MONOTONIC, m_ptLastConnect);
			m_tLastConnect = time( NULL );
		}
		else
		{
			std::stringstream ss;
			ss << "failed to connect to " + strHostname;
			if( m_bSecure )
				ss << " on a secure socket";
			Log::getLog("oocl")->logError( ss.str() );
		}
		
		return m_bConnected;
	}


	/**
	 * @brief	Disconnects the client from the server.
     */
	void HttpClient::disconnect() 
	{
		m_pSocket->close();
		m_bConnected = false;
		
		m_bStreaming = false;
	}


	/**
	 * @brief	This method takes a full web URL, connects to the corresponding server, issues a GET request, disconnects and returns the server's answer.
	 * 
     * @param	strURL	The full web URL for the GET request.
	 * 
     * @return	The server's answer if successful, else an empty string
     */
	std::string HttpClient::oneGET( std::string strURL )
	{
		unsigned int uiDivider = strURL.find('/', 8);
		if( uiDivider > strURL.size() )
			return "";
		
		std::string strHost = strURL.substr( 0, uiDivider );
		std::string strLocation = strURL.substr( uiDivider );
		
		HttpClient con;
		if( !con.connect( strHost ) )
			return "";
		
		std::string strRet = con.GET( strLocation );
		con.disconnect();
		
		return strRet;
	}
		
	
	/**
	 * @brief	Sends a http GET request to the server connected with the parameter given in strURL and returns the server's answer when it gets it (synchronous call).
	 * 
     * @param	strURL	URL and parameters for GET request
	 * 
     * @return	The server's answer to the request or an empty string when not connected or sending not successful.
     */
	std::string HttpClient::GET( std::string strURL ) 
	{
		if( m_bConnected )
		{
			// build the standard header with strURL
			editHeaderParameter( "Content-Length", "0" );
			std::string strHeader = buildHeader( "GET", strURL );

			// send the request
			if( sendRequest( strHeader ) )
			{
				std::string strAnswer = m_pSocket->read();
				if( strAnswer.empty() )
				{
					m_pSocket->close();
					m_pSocket->connect( m_strHostPort );
					sendRequest( strHeader );
					strAnswer = m_pSocket->read();
				}

				// verify that all data has been received
				while( verifyAnswerLength( strAnswer ) > 0 )
				{
					strAnswer += m_pSocket->read();
				}
				
				return strAnswer;
			}
		}
		
		return "";
	}


	/**
	 * @brief	Sends a http POST request to the server connected with the parameter given in strURL and the body given in strBody and returns the server's answer when it gets it (synchronous call).
	 * 
     * @param	strURL	URL and parameters for POST request
	 * @param	strBody	The body of the POST request.
	 * 
     * @return	The server's answer to the request or an empty string when not connected or sending not successful.
     */
	std::string HttpClient::POST( std::string strURL, std::string strBody ) 
	{
		if( m_bConnected )
		{
			// build the standard header with strURL
			std::stringstream ss; ss << strBody.size();
			editHeaderParameter( "Content-Length", ss.str() );
			std::string strHeader = buildHeader( "POST", strURL );
			
			if( !strBody.empty() )
				strHeader += strBody + NEWLINE + NEWLINE;
#ifdef _DEBUG
			std::cout << strHeader << std::endl;
#endif
			
			// send the request
			if( sendRequest( strHeader ) )
			{
				std::string strAnswer = m_pSocket->read();

				// verify that all data has been received
				while( verifyAnswerLength( strAnswer ) > 0 )
				{
					strAnswer += m_pSocket->read();
				}
				
				return strAnswer;
			}
		}
		
		return "";
	}


	/**
	 * @brief	Sends a http DELETE request to the server connected with the parameter given in strURL and the body given in strBody and returns the server's answer when it gets it (synchronous call).
	 * 
     * @param	strURL	URL and parameters for DELETE request
	 * @param	strBody	The body of the DELETE request.
	 * 
     * @return	The server's answer to the request or an empty string when not connected or sending not successful.
     */
	std::string HttpClient::DELETE( std::string strURL, std::string strBody ) 
	{
		if( m_bConnected )
		{
			// build the standard header with strURL
			std::stringstream ss; ss << strBody.size();
			editHeaderParameter( "Content-Length", ss.str() );
			std::string strHeader = buildHeader( "DELETE", strURL );
			
			if( !strBody.empty() )
				strHeader += strBody + NEWLINE + NEWLINE;
#ifdef _DEBUG
			std::cout << strHeader << std::endl;
#endif
			
			// send the request
			if( sendRequest( strHeader ) )
			{
				std::string strAnswer = m_pSocket->read();

				// verify that all data has been received
				while( verifyAnswerLength( strAnswer ) > 0 )
				{
					strAnswer += m_pSocket->read();
				}
				
				return strAnswer;
			}
		}
		
		return "";
	}


	/**
	 * @brief	Sends a http PUT request to the server connected with the parameter given in strURL and the body given in strBody and returns the server's answer when it gets it (synchronous call).
	 * 
     * @param	strURL	URL and parameters for PUT request
	 * @param	strBody	The body of the PUT request.
	 * 
     * @return	The server's answer to the request or an empty string when not connected or sending not successful.
     */
	std::string HttpClient::PUT( std::string strURL, std::string strBody ) 
	{
		if( m_bConnected )
		{
			// build the standard header with strURL
			std::stringstream ss; ss << strBody.size();
			editHeaderParameter( "Content-Length", ss.str() );
			std::string strHeader = buildHeader( "PUT", strURL );
			
			if( !strBody.empty() )
				strHeader += strBody + NEWLINE + NEWLINE;
#ifdef _DEBUG
			std::cout << strHeader << std::endl;
#endif
			
			// send the request
			if( sendRequest( strHeader ) )
			{
				std::string strAnswer = m_pSocket->read();

				// verify that all data has been received
				while( verifyAnswerLength( strAnswer ) > 0 )
				{
					strAnswer += m_pSocket->read();
				}
				
				return strAnswer;
			}
		}
		
		return "";
	}
	
	
	/**
	 * @brief	Sends a http GET request to the connected server and returns a buffer to which the answer of the server will be streamed (asynchronous call).
	 * 
     * @param	strURL	URL and parameters for GET request
	 * 
     * @return	If successful the buffer to which the server's answer will be streamed, else a NULL pointer.
     */
	std::stringbuf* HttpClient::GETstream( std::string strURL )
	{
		if( m_bConnected && !m_bStreaming )
		{	
			// build the standard header with strURL
			std::string strHeader = buildHeader( "GET", strURL );
			
#ifdef _DEBUG
			std::cout << strHeader << std::endl;
#endif

			// send the request
			if( sendRequest( strHeader ) )
			{
				// now just stream everything that will eventually be received
				m_psbBuffer = new std::stringbuf;
				start();
				return m_psbBuffer;
			}
		}
		
		return NULL;
	}
	
	/**
	 * @brief	Sends a http POST request to the connected server and returns a buffer to which the answer of the server will be streamed until (asynchronous call).
	 * 
     * @param	strURL	URL and parameters for POST request
	 * @param	strBody	The body of the POST request.
	 * 
     * @return	If successful the buffer to which the server's answer will be streamed, else a NULL pointer.
     */
	std::stringbuf* HttpClient::POSTstream( std::string strURL, std::string strBody )
	{
		if( m_bConnected && !m_bStreaming )
		{	
			// build the standard header with strURL
			std::string strHeader = buildHeader( "POST", strURL );
			strHeader += strBody + NEWLINE + NEWLINE;
			
#ifdef _DEBUG
			std::cout << strHeader << std::endl;
#endif

			// send the request
			if( sendRequest( strHeader ) )
			{
				// now just stream everything that will eventually be received
				m_psbBuffer = new std::stringbuf;
				start();
				return m_psbBuffer;
			}
		}
		
		return NULL;
	}
	
	/**
	 * @brief	Sends a http DELETE request to the connected server and returns a buffer to which the answer of the server will be streamed until (asynchronous call).
	 * 
     * @param	strURL	URL and parameters for DELETE request
	 * @param	strBody	The body of the DELETE request.
	 * 
     * @return	If successful the buffer to which the server's answer will be streamed, else a NULL pointer.
     */
	std::stringbuf* HttpClient::DELETEstream( std::string strURL, std::string strBody )
	{
		if( m_bConnected && !m_bStreaming )
		{	
			// build the standard header with strURL
			std::string strHeader = buildHeader( "DELETE", strURL );
			strHeader += strBody + NEWLINE + NEWLINE;
			
#ifdef _DEBUG
			std::cout << strHeader << std::endl;
#endif

			// send the request
			if( sendRequest( strHeader ) )
			{
				// now just stream everything that will eventually be received
				m_psbBuffer = new std::stringbuf;
				start();
				return m_psbBuffer;
			}
		}
		
		return NULL;
	}
	
	/**
	 * @brief	Sends a http PUT request to the connected server and returns a buffer to which the answer of the server will be streamed until (asynchronous call).
	 * 
     * @param	strURL	URL and parameters for PUT request
	 * @param	strBody	The body of the PUT request.
	 * 
     * @return	If successful the buffer to which the server's answer will be streamed, else a NULL pointer.
     */
	std::stringbuf* HttpClient::PUTstream( std::string strURL, std::string strBody )
	{
		if( m_bConnected && !m_bStreaming )
		{	
			// build the standard header with strURL
			std::string strHeader = buildHeader( "PUT", strURL );
			strHeader += strBody + NEWLINE + NEWLINE;
			
#ifdef _DEBUG
			std::cout << strHeader << std::endl;
#endif

			// send the request
			if( sendRequest( strHeader ) )
			{
				// now just stream everything that will eventually be received
				m_psbBuffer = new std::stringbuf;
				start();
				return m_psbBuffer;
			}
		}
		
		return NULL;
	}
		
	
	/**
	 * @brief	Add a parameter to the next header.
	 * 
     * @param	strParamName	The name of the parameter to add
     * @param	strValue		The value of the parameter
     */
	void HttpClient::addHeaderParameter( std::string strParamName, std::string strValue ) 
	{ 
		m_mapHeaderParametersByName.insert( std::pair< std::string, std::string >(strParamName, strValue) ); 
	}
	
	/**
	 * @brief	Remove the parameter with the given name.
	 * 
     * @param	strParamName	The name of the parameter to remove
     */
	void HttpClient::removeHeaderParameter( std::string strParamName ) 
	{ 
		std::map< std::string, std::string >::iterator it = m_mapHeaderParametersByName.find( strParamName );
		if( it != m_mapHeaderParametersByName.end() )
			m_mapHeaderParametersByName.erase( it ); 
	}
	
	/**
	 * @brief	Add a header parameter or edit the value if a parameter with the given name already exists.
     * @param	strParamName	The name of the parameter
     * @param	strNewValue		The value of the parameter
     */
	void HttpClient::editHeaderParameter( std::string strParamName, std::string strNewValue ) 
	{ 
		std::map< std::string, std::string >::iterator it = m_mapHeaderParametersByName.find( strParamName );
		if( it != m_mapHeaderParametersByName.end() )
			it->second = strNewValue; 
		else
			m_mapHeaderParametersByName.insert( std::pair< std::string, std::string >(strParamName, strNewValue) ); 
	}
	
	
	/**
	 * @brief	Stop all streaming currently in progress.
     */
	void HttpClient::terminateAllStreaming() 
	{ 
		m_bStreaming = false; 
		try {
			m_pSocket->close();
		} catch(...) {}
		m_pSocket->connect( m_strHostPort );
	}


	/**
	 * @brief	The thread method for streaming the answer to asynchronously called requests.
     */
	void HttpClient::run()
	{
		m_bStreaming = true;
		char c;
		while( m_bStreaming )
		{
			c = m_pSocket->readC();
			if( c != 0)
				m_psbBuffer->sputc( c );
			else
				sleep(10);
		}
	}

	
	/**
	 * @brief	Build a header out of the given method, URL and the currently set header parameters.
	 * 
     * @param	strMethod	The http request name (GET, POST, DELETE, PUT etc.)
     * @param	strURL		The URL to issue the request to
	 * 
     * @return	The ready to send http header as string.
     */
	std::string HttpClient::buildHeader( std::string strMethod, std::string strURL )
	{
		// build the standard header with strURL
		std::string strHeader = strMethod + " " + strURL + " " + m_strHttpVersion + NEWLINE + "Host: " + m_strHost + NEWLINE;

		// add the user specified header additions and close the header (two linebreaks)
		for( std::map< std::string, std::string >::iterator it = m_mapHeaderParametersByName.begin(); it != m_mapHeaderParametersByName.end(); ++it )
		{
			strHeader += it->first + ": " + it->second + NEWLINE;
		}
		strHeader += NEWLINE;
		
		return strHeader;
	}
	
	/**
	 * @brief	Send the request given to the server we are currently connected to.
	 * 
     * @param	strRequest	The request to send.
	 * 
     * @return	True if successful, false if not.
     */
	bool HttpClient::sendRequest( std::string strRequest )
	{
//		timespec* ptTempTime = new timespec;
//		clock_gettime(CLOCK_MONOTONIC, ptTempTime);
		time_t tTempTime = time( NULL );
		
//		if( ptTempTime->tv_sec - m_ptLastConnect->tv_sec  > 45 )
		if( tTempTime - m_tLastConnect > 45 )
		{
//			try {
//				m_pSocket->close();
//			} catch(...) 
//			{
//				Log::getLog("oocl")->logWarning("closing the socket threw an exception");
//			}
			m_pSocket->connect( m_strHostPort );
		}
//		delete m_ptLastConnect;
//		m_ptLastConnect = ptTempTime;
		m_tLastConnect = time( NULL );
			
		bool bSuccess = false;
		try {
			bSuccess = m_pSocket->write( strRequest );
		}
		catch(...)
		{
			Log::getLog("oocl")->logWarning("sending request failed, retrying");
		}
		
		if( !bSuccess )
		{
			try {
				// when the request fails try to reconnect once
				if( !connect( m_strHostname ) )
				{
					Log::getLog("oocl")->logError( "sending request failed two times, disconnected!" );
					disconnect();
					return false;
				}

				bSuccess = m_pSocket->write( strRequest );
			}
			catch(...)
			{
				Log::getLog("oocl")->logError("sending request failed again");
			}
		}
		
		return bSuccess;
	}
	
	
	/**
	 * @brief	Tries to verify the length of the given http answer by the Content-Length header parameter.
	 * 
     * @param	strAnswer	The server's answer to a http request.
	 * 
     * @return	0 if the answer length is correct, -1 if it is bigger than expected, 1 if it is smaller than expected and -2 if no Content-Length parameter was found in the answer.
     */
	int HttpClient::verifyAnswerLength( std::string strAnswer )
	{
		if( !strAnswer.empty() )
		{
			int iParamPos = strAnswer.find( "Content-Length: " );
			if( iParamPos < strAnswer.length() )
			{
				std::string strContLength = strAnswer.substr( iParamPos+16 );
				strContLength = strContLength.substr( 0, strContLength.find( NEWLINE ) );
				std::stringstream ss;
				ss << strContLength;

				unsigned int uiExpectedSize, uiActualSize = strAnswer.length();
				ss >> uiExpectedSize;
				uiExpectedSize += strAnswer.find( NEWLINE+NEWLINE ) + 4;
				

				if( uiActualSize < uiExpectedSize )
					return 1;
				else if( uiActualSize > uiExpectedSize )
					return -1;
				else
					return 0;
			}
			else
			{
				Log::getLog("oocl")->logError( "could not find key \"Content-Length\" inside the header" );
			}
		}
		
		return -2;
	}
	
	
	/**
	 * @brief	Handle a broken pipe.
	 * 
     * @param	iSignal	The signal
     */
	void HttpClient::handleBrokenPipe( int iSignal )
	{
		Log::getLog("oocl")->logWarning( "signal broken pipe received and ignored" );
	}
}