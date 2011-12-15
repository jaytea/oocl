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

#include "Message.h"

namespace oocl
{

	// ******************** Message *********************
	
	//unsigned short Message::sm_type = MT_InvalidMessage; // this is an invalid type
	std::vector<Message* (*)(const char*)> Message::sm_msgTypeList;

	Message* Message::createFromString( const char* cMsg )
	{
		unsigned short usType = ((unsigned short*)cMsg)[0];

		if( usType < sm_msgTypeList.size() && sm_msgTypeList[usType] != NULL )
		{
			return sm_msgTypeList[usType]( cMsg );
		}
		else
		{
			std::ostringstream os;
			os << "No message class for message type " << usType << " registered";
			Log::getLog("oocl")->logMessage( os.str(), Log::EL_ERROR );
		}

		return NULL;
	}

	void Message::registerMsg( unsigned short usType, Message* (*create)(const char*)  )
	{
		if( usType >= sm_msgTypeList.size() )
		{
			std::ostringstream os;
			os << "The message with ID " << usType << " was successfully registered.";
			Log::getLog("oocl")->logMessage( os.str(), Log::EL_INFO );

			sm_msgTypeList.resize( usType+1, NULL );
			sm_msgTypeList[usType] = create;
		}
		else if( sm_msgTypeList[usType] == NULL )
		{
			std::ostringstream os;
			os << "The message with ID " << usType << " was successfully registered.";
			Log::getLog("oocl")->logMessage( os.str(), Log::EL_INFO );

			sm_msgTypeList[usType] = create;
		}
		else
		{
			std::ostringstream os;
			os << "You tried to register the message with ID " << usType << ", but it is already registered";
			Log::getLog("oocl")->logMessage( os.str(), Log::EL_INFO );
		}
	}

}