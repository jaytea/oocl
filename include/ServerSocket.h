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
// This file was written by Jürgen Lorenz

#ifndef SERVERSOCKET_H_INCLUDED
#define SERVERSOCKET_H_INCLUDED

#include "oocl_import_export.h"

#include "BerkeleySocket.h"

namespace oocl
{
	/**
	 * @brief	Specialized server socket for receiving incoming connections.
	 *
	 * @author	Jörn Teuber
	 * @date	1.3.2012
	 */
	class OOCL_EXPORTIMPORT ServerSocket : public SocketStub
	{
	public:
		ServerSocket();
		~ServerSocket();

		bool bind( unsigned short port );

		Socket * accept();

		bool isValid();
		int getCSocket();
		
	private:
		struct sockaddr_in m_sSockAddr;
		//struct sockaddr_in6 addr6;
		int m_iSockFD;

		bool m_bValid;
		bool m_bBound;
	};

}

#endif // SERVERSOCKET_H_INCLUDED
