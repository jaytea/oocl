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
/// This file was written by J�rn Teuber
#include "IntroductionMessage.h"


IntroductionMessage::IntroductionMessage( unsigned int uiUserID, std::string strUsername ) :
	m_uiUserID( uiUserID ),
	m_strUsername( strUsername )
{
	m_iProtocoll = SOCK_STREAM;
	m_type = MT_IntroductionMessage;
}
	
std::string IntroductionMessage::getMsgString()
{
	unsigned short usMsg[4];
	usMsg[0] = MT_IntroductionMessage;
	usMsg[1] = getBodyLength();
	usMsg[2] = ((unsigned short*)&m_uiUserID)[0];
	usMsg[3] = ((unsigned short*)&m_uiUserID)[1];

	std::string strHeader( (char*)usMsg, 8 );

	return strHeader+m_strUsername;
}

unsigned short IntroductionMessage::getBodyLength()
{
	return 4 + m_strUsername.length();
}

oocl::Message* IntroductionMessage::create(const char * in)
{
	unsigned int uiUserID = ((unsigned int*)in)[1];
	std::string strUsername( &in[8], ((unsigned short*)in)[1]-4 );

	return new IntroductionMessage( uiUserID, strUsername );
}