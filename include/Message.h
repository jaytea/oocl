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
// This file was written by Jörn Teuber

#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#ifdef linux
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
#else
	#include <windows.h>
#endif


#include "oocl_import_export.h"
#include "Log.h"


namespace oocl
{
	class Message;
}

EXPIMP_TEMPLATE template class OOCL_EXPORTIMPORT std::allocator<oocl::Message* (*)(const char*)>;
EXPIMP_TEMPLATE template class OOCL_EXPORTIMPORT std::vector<oocl::Message* (*)(const char*)>;

namespace oocl
{

#define MT_InvalidMessage 0
	/**
	 * @class	Message
	 *
	 * @brief	Interface and manager for all message classes.
	 *
	 * @author	Jörn Teuber
	 * @date	3/1/2012
	 */
	class OOCL_EXPORTIMPORT Message
	{
		friend class MessageBroker;

	public:
		static Message* createFromString( const char* cMsg );

		// *********** getter *************
		/**
		 * @brief return a ready-to-send string representation of the message
		 *  | Type  |Length | Messagebody
		 *  |2 byte |2 byte |   |   .... 
		 * => Length = length of the messageBody in bytes (as returned by getBodyLength() )
		 */
		virtual std::string		getMsgString()  const	{ int i=0; return std::string( (char*)&i, 4 ); }
		virtual unsigned short	getBodyLength() const	{ return 0; }

		void setSenderID( unsigned int uiSenderID ) { m_uiSenderID = uiSenderID; }
		void setProtocoll( int iProtocoll ) { m_iProtocoll = iProtocoll; }

		/// @brief return the type id of the message object
		virtual unsigned short  getType()		const	{ return m_type; }
		virtual int             getProtocoll() 	const	{ return m_iProtocoll; }
		unsigned int			getSenderID() 	const	{ return m_uiSenderID; }
		bool					isIncoming() 	const	{ return m_bIncoming; }

	protected:
		Message();
		virtual ~Message(void) {}

		static void registerMsg( unsigned short type, Message* (*create)(const char*) );

	protected:
		unsigned short 	m_type;
		int             m_iProtocoll;
		unsigned int	m_uiSenderID;
		bool			m_bIncoming; ///< true if message came from a network, false if this client is sending it

	private:
		
		static std::vector<oocl::Message* (*)(const char*)> sm_msgTypeList;
	};
}

#endif
