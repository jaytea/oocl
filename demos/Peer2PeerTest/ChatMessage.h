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
#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <Message.h>

#define MT_ChatMessage 6
class ChatMessage : public oocl::Message
{
public:
	static void registerMsg(){
		Message::registerMsg( MT_ChatMessage, ChatMessage::create );
	}

	ChatMessage( unsigned int uiUserID, std::string strMessage );
	
	virtual std::string getMsgString();
	virtual unsigned short getBodyLength();
	
	unsigned int	getPeerID() { return m_uiUserID; }
	std::string		getMessage() { return m_strMessage; }

protected:
	static Message* create(const char * in);

private:
	unsigned int	m_uiUserID;
	std::string		m_strMessage;
};

#endif // CHATMESSAGE_H