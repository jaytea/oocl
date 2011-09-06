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

	unsigned short Message::sm_type = 0;
	std::map<unsigned short, Message* (*)(const char*)> Message::sm_msgTypeMap;

	Message* Message::createFromString( const char* cMsg )
	{
		unsigned short iType = ((unsigned short*)cMsg)[0];

		std::map<unsigned short, Message* (*)(const char*)>::iterator it = sm_msgTypeMap.find( iType );
		if( it != sm_msgTypeMap.end() )
		{
			return (*it).second( cMsg );
		}
		else
		{
			std::cout << "MessageType with id " << iType << " not found!" << std::endl;
		}

		return NULL;
	}

	void Message::registerMsg( unsigned short type, Message* (*create)(const char*)  )
	{
		sm_msgTypeMap.insert( std::pair<unsigned short, Message* (*)(const char*)>(type, create) );
	}


	// ******************** StandardMessage *********************

	unsigned short StandardMessage::sm_type = 1;

	StandardMessage::StandardMessage( const char * cMsgBody, unsigned short length  )
	{
		StandardMessage( std::string(cMsgBody, length) );
	}

	StandardMessage::StandardMessage( std::string strMsgBody  ) :
		m_type( 0 ),
		m_strMsgBody( strMsgBody ),
		m_iProtocoll( SOCK_STREAM )
	{
	}

	std::string StandardMessage::getMsgString()
	{
		unsigned short usTemp[2];
		usTemp[0] = m_type;
		usTemp[1] = m_strMsgBody.length();
		std::string strHeader( (char*)usTemp, 4 );

		return strHeader + m_strMsgBody;
	}

	Message* StandardMessage::create(const char * in)
	{
		return &StandardMessage(  &(in[4]), ((unsigned short*)in)[1] );
	}

}