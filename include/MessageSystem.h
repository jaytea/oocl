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
/// This file was written by Jürgen Lorenz and Jörn Teuber

#ifndef MESSAGESYSTEM_H_INCLUDED
#define MESSAGESYSTEM_H_INCLUDED

#include "oocl_import_export.h"

#include "MessagePump.h"

namespace oocl
{

	class OOCL_EXPORTIMPORT MessageSystem :
		public MessageListener
	{
	public:
		MessageSystem(void);
		~MessageSystem(void);

		bool registerForMessage( int iType, MessageListener* pListener );
		bool pumpMessage( Message* pMessage );

		bool cbMessage( Message* pMessage );

	private:
		MessagePump* m_pPumps;
	};

}

#endif