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
// This file was written by Jürgen Lorenz and Jörn Teuber

#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#include "oocl_import_export.h"

#include "SocketStub.h"

namespace oocl
{

	struct OOCL_EXPORTIMPORT socket_conf{
		unsigned short   ipVersion;
		unsigned short   bufferSize;
	};


	/**
	 * @class	Socket
	 *
	 * @brief	C++ abstraction of a C socket.
	 *
	 * @author	Jörn Teuber
	 * @date	02.03.2012
	 */
	class OOCL_EXPORTIMPORT Socket : public SocketStub
	{
		friend class ServerSocket;
		
	public:
		Socket();
		virtual ~Socket() {}

		virtual bool connect( std::string host, unsigned short usPort ) = 0;
		virtual bool connect( unsigned int uiHostIP, unsigned short usPort ) = 0;
		virtual bool bind( unsigned short usPort ) = 0;

		virtual bool isValid() = 0;
		virtual bool isConnected() = 0;

		virtual bool read( std::string& str, int count = 0 ) = 0;
		virtual bool read( char& c ) = 0;
		virtual bool read( char* pcBuf, int& count ) = 0;

		virtual bool readFrom( std::string& str, int count = 0, unsigned int* hostIP = NULL ) = 0;


		virtual bool write(std::string in) = 0;
		virtual bool write(char in) = 0;
		virtual bool write(const char * in, int count) = 0;

		virtual bool writeTo( std::string in, std::string host, unsigned short port ) = 0;

		virtual void close() = 0;

		virtual unsigned int getConnectedIP() = 0;
	};

}

#endif // SOCKET_H_INCLUDED
