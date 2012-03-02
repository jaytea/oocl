/*
Object Oriented Communication Library
Copyright (c) 2011 J�rgen Lorenz and J�rn Teuber

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
// This file was written by J�rgen Lorenz

#ifndef SERVERSOCKET_H_INCLUDED
#define SERVERSOCKET_H_INCLUDED

#include "oocl_import_export.h"

#include "Socket.h"

namespace oocl
{
	/**
	 * @class	ServerSocket
	 *
	 * @brief	Specialized server socket for receiving incoming connections.
	 *
	 * @author	J�rn Teuber
	 * @date	3/1/2012
	 */
	class OOCL_EXPORTIMPORT ServerSocket : public SocketStub
	{
	public:
		ServerSocket();
		~ServerSocket();

		bool bind( unsigned short port );

		Socket * accept();

		bool isValid() { return bValid; }
		int getCSocket() { return sockfd; }
		
	private:
		struct sockaddr_in addr;
		//struct sockaddr_in6 addr6;
		int sockfd;

		bool bValid;
		bool bBound;
	};

}

#endif // SERVERSOCKET_H_INCLUDED
