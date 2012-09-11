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

#ifndef HTTPSERVER_H
#define	HTTPSERVER_H

#include <string>
#include <sstream>
#include <map>
#include <signal.h>

#include "oocl_import_export.h"

#include "SecureSocket.h"
#include "Thread.h"

#define NEWLINE std::string("\x0D\x0A")

namespace oocl
{
	
	/**
	 * @class	HttpClient
	 * 
	 * @brief	A simple http client class to send the most common http requests and receive either synchronous or asynchronous.
	 * 
	 * @note	At the moment there can only be one concurrent asynchronous request as there is only one connection to the server per object.
	 * 
	 * @author	Jörn Teuber
	 * @date	09.09.2012
     */
	class OOCL_EXPORTIMPORT HttpClient : public Thread
	{
	public:
		HttpClient();
		~HttpClient();

		bool connect( std::string strHostname );
		void disconnect();
		
		static std::string oneGET( std::string strURL );
		
		// synchronous/blocking calls
		std::string GET( std::string strURL );
		std::string POST( std::string strURL, std::string strBody );
		std::string DELETE( std::string strURL, std::string strBody );
		std::string PUT( std::string strURL, std::string strBody );
		
		// asynchronous version (returns a pointer to a stream to which all incoming data is written)
		std::stringbuf* GETstream( std::string strURL );
		std::stringbuf* POSTstream( std::string strURL, std::string strBody );
		std::stringbuf* DELETEstream( std::string strURL, std::string strBody );
		std::stringbuf* PUTstream( std::string strURL, std::string strBody );
		
		// modifier for custom header parameters
		void addHeaderParameter( std::string strParamName, std::string strValue );
		void removeHeaderParameter( std::string strParamName );
		void editHeaderParameter( std::string strParamName, std::string strNewValue );
		
		
		void terminateAllStreaming();

		// getter
		bool isConnected() { return m_bConnected; }
		bool isStreaming() { return m_bStreaming; }
		std::string getConnectedTo() { return m_strHost; }
		
		// setter
		void setHTTPVersion( std::string strVersion ) { m_strHttpVersion = strVersion; }
		
	protected:
		void run();
		
	private:
		std::string buildHeader( std::string strMethod, std::string strURL );
		bool sendRequest( std::string strRequest );
		int verifyAnswerLength( std::string strAnswer );
		
		static void handleBrokenPipe( int iSignal );

	private:
		bool m_bConnected;
		bool m_bStreaming;
		bool m_bSecure;
		
		std::string m_strHttpVersion;
		std::string m_strHost;
		std::string m_strHostname;
		std::string m_strHostPort;
		oocl::SecureSocket* m_pSocket;
		
		std::stringbuf* m_psbBuffer;
		
//		timespec* m_ptLastConnect;
		time_t m_tLastConnect;
		
		std::map< std::string, std::string > m_mapHeaderParametersByName;
	};
	
}

#endif	/* HTTPSERVER_H */

