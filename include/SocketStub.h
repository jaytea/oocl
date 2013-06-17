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

#ifndef SOCKETSTUB_H_INCLUDED
#define SOCKETSTUB_H_INCLUDED

#include <string>
#include <stdio.h>
#include <sstream>

#include "oocl_import_export.h"

#include "Log.h"

#ifdef linux
#	include <netinet/in.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <arpa/inet.h>
#	include <netinet/tcp.h>
#	include <netdb.h>
#else
#	include <windows.h>
#	include <WinSock.h>
#endif

namespace oocl
{
	/**
	 * @class	SocketStub
	 *
	 * @brief	Socket stub that holds features needed by Socket and ServerSocket.
	 *
	 * @author	Jörn Teuber
	 * @date	3/2/2012
	 */
	class OOCL_EXPORTIMPORT SocketStub
	{
	public:
		SocketStub();
		virtual ~SocketStub();

		virtual int getCSocket() = 0;
		
	private:
		static int iSocketCounter;

	};

}

#endif // SOCKETSTUB_H_INCLUDED
