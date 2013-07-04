/*
Object Oriented Communication Library
Copyright (c) 2011 J��rgen Lorenz and J��rn Teuber

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

#include "Message.h"

namespace oocl
{

	// ******************** Message *********************
	
	std::vector<Message* (*)(const char*)> Message::sm_msgTypeList;


	Message::Message()
		: m_type( 0 )
		, m_iProtocoll( 0 )
		, m_uiSenderID( 0 )
		, m_bIncoming( false )
	{
	}


	/**
	 * @brief return a ready-to-send string representation of the message.
	 *  | Type  |Length | Messagebody
	 *  |2 byte |2 byte |   |   ....
	 * => Length = length of the messageBody in bytes (as returned by getBodyLength() )
	 *
	 * @return	Empty string, as this is not meant to be sent directly.
	 */
	std::string	Message::getMsgString() const
	{
		return std::string();
	}

	/**
	 * @brief	Return the length of the body of this message.
	 *
	 * @return	0, as this message has no body.
	 */
	unsigned short Message::getBodyLength() const
	{
		return 0;
	}

	/**
	 * @brief	Set the PeerID of the original sender of this message, used to differentiate messages in P2P context.
	 *
	 * @param uiSenderID	The PeerID of the original sender.
	 */
	void Message::setSenderID( unsigned int uiSenderID )
	{
		m_uiSenderID = uiSenderID;
	}

	/**
	 * @brief	Set the protocol used to send this message over the network.
	 *
	 * @param iProtocol	The protocol to use (SOCK_STREAM, SOCK_DGRAM or 0).
	 */
	void Message::setProtocoll( int iProtocol )
	{
		m_iProtocoll = iProtocol;
	}

	/**
	 * @brief 	Return the type ID of the message object.
	 *
	 * @return 	The message's type.
	 */
	unsigned short  Message::getType() const
	{
		return m_type;
	}

	/**
	 * @brief 	Get the protocol that will be used if sending this message over the network.
	 *
	 * @return	The protocol (SOCK_STREAM, SOCK_DGRAM or 0).
	 */
	int Message::getProtocoll() const
	{
		return m_iProtocoll;
	}

	/**
	 * @brief	Get the PeerID of the original sender of this message.
	 *
	 * @return	The senders PeerID.
	 */
	unsigned int Message::getSenderID() const
	{
		return m_uiSenderID;
	}

	/**
	 * @brief	Check whether this message was received from a network.
	 *
	 * @return	True, if this message was received over a network, false if it was created on this client.
	 */
	bool Message::isIncoming() const
	{
		return m_bIncoming;
	}


	/**
	 * @brief	Creates an object of an implementation of message from a received byte buffer.
	 *
	 * @param	cMsg	The message as byte buffer.
	 *
	 * @return	null if it fails, else a pointer to the created message object.
	 */
	Message* Message::createFromString( const char* cMsg )
	{
		unsigned short usType = ((unsigned short*)cMsg)[0];
		Message* pReturn = NULL;

		if( usType < sm_msgTypeList.size() && sm_msgTypeList[usType] != NULL )
		{
			pReturn = sm_msgTypeList[usType]( cMsg );
			pReturn->m_bIncoming = true;
		}
		else
		{
			Log::getLogRef("oocl") << Log::EL_ERROR << "No message class for message type " << usType << " registered" << oocl::endl;
		}

		return pReturn;
	}


	/**
	 * @brief	registers a message imlementation with the system, so that it can be created when received
	 * 			
	 * @note	has to be called from the message implementation before it can be recognized by
	 * 			the network.
	 *
	 * @param	usType		  	The type.
	 * @param	create [in]		a pointer to a function that receives a byte buffer and returns a pointer to a newly created object of your message implementation.
	 */
	void Message::registerMsg( unsigned short usType, Message* (*create)(const char*)  )
	{
		if( usType >= sm_msgTypeList.size() )
		{
			std::ostringstream os;
			os << "The message with ID " << usType << " was successfully registered.";
			Log::getLog("oocl")->logInfo( os.str() );

			sm_msgTypeList.resize( usType+1, NULL );
			sm_msgTypeList[usType] = create;
		}
		else if( sm_msgTypeList[usType] == NULL )
		{
			std::ostringstream os;
			os << "The message with ID " << usType << " was successfully registered.";
			Log::getLog("oocl")->logInfo( os.str() );

			sm_msgTypeList[usType] = create;
		}
		else
		{
			std::ostringstream os;
			os << "You tried to register the message with ID " << usType << ", but it is already registered";
			Log::getLog("oocl")->logInfo( os.str() );
		}
	}

}
