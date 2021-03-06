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

#include "SocketStub.h"

namespace oocl
{

	int SocketStub::iSocketCounter = 0;


	/**
	 * @fn	SocketStub::SocketStub()
	 *
	 * @brief	Default constructor.
	 */
	SocketStub::SocketStub()
	{
	#ifndef linux
		if( iSocketCounter == 0 )
		{
			WSADATA wsaData;
			int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult != 0) {
				std::ostringstream os;
				os << "WSAStartup failed: " << WSAGetLastError();
				Log::getLog("oocl")->logError( os.str() );
			}
		}
	#endif
		iSocketCounter++;
	}


	/**
	 * @fn	SocketStub::~SocketStub()
	 *
	 * @brief	Destructor.
	 */
	SocketStub::~SocketStub()
	{
		iSocketCounter--;
	#ifndef linux
		if( iSocketCounter == 0 )
			WSACleanup();
	#endif
	}

}