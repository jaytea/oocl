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

#include "ExplicitMessages.h"

namespace oocl
{

	// ******************** StandardMessage *********************

	/**
	 * @brief	Constructor for c-style strings.
	 *
	 * @param	cMsgBody	The message body.
	 * @param	length  	Length of the body.
	 */
	StandardMessage::StandardMessage( const char * cMsgBody, unsigned short length  ) :
		m_strMsgBody( std::string(cMsgBody, length) )
	{
		m_iProtocoll = SOCK_STREAM;
		m_type = MT_StandardMessage;
	}


	/**
	 * @brief	Constructor for c++ strings.
	 *
	 * @param	strMsgBody	The message body.
	 */
	StandardMessage::StandardMessage( std::string strMsgBody  ) :
		m_strMsgBody( strMsgBody )
	{
		m_iProtocoll = SOCK_STREAM;
		m_type = MT_StandardMessage;
	}


	/**
	 * @brief	Returns the message as string.
	 *
	 * @return	The message string.
	 */
	std::string StandardMessage::getMsgString() const
	{
		unsigned short usTemp[2];
		usTemp[0] = m_type;
		usTemp[1] = getBodyLength();
		std::string strHeader( (char*)usTemp, 4 );

		return strHeader + m_strMsgBody;
	}


	/**
	 * @brief	Returns the size in bytes of the message body (message without header).
	 *
	 * @return	The body length in byte.
	 */
	unsigned short StandardMessage::getBodyLength() const
	{
		return m_strMsgBody.length();
	}


	/**
	 * @brief	Creates this message out of the received byte array.
	 *
	 * @param	in	The incoming byte buffer.
	 *
	 * @return	a new StandardMessage built from in.
	 */
	Message* StandardMessage::create(const char * in)
	{
		return new StandardMessage( &(in[4]), ((unsigned short*)in)[1] );
	}
	

	// ******************** SubscribeMessage *********************

	/**
	 * @brief	Constructor.
	 *
	 * @param	usTypeToSubscribe	The type to subscribe.
	 */
	SubscribeMessage::SubscribeMessage( unsigned short usTypeToSubscribe ) :
		m_usTypeToSubscribe( usTypeToSubscribe )
	{
		m_iProtocoll = SOCK_STREAM;
		m_type = MT_SubscribeMessage;
	}


	/**
	 * @brief	Returns the message as string.
	 *
	 * @return	The message as string.
	 */
	std::string SubscribeMessage::getMsgString() const
	{
		unsigned short usMsg[3];
		usMsg[0] = MT_SubscribeMessage;
		usMsg[1] = getBodyLength();
		usMsg[2] = m_usTypeToSubscribe;

		return std::string( (char*)usMsg, 6 );
	}


	/**
	 * @brief	Returns the size in bytes of the message body (message without header).
	 *
	 * @return	The body length in byte.
	 */
	unsigned short SubscribeMessage::getBodyLength() const
	{
		return sizeof(unsigned short);
	}


	/**
	 * @brief	Creates this message out of the received byte array.
	 *
	 * @param	in	The incoming byte buffer.
	 *
	 * @return	a new SubscribeMessage built from in.
	 */
	Message* SubscribeMessage::create(const char * in)
	{
		return new SubscribeMessage( ((unsigned short*)in)[2] );
	}
	

	// ******************** ConnectMessage *********************

	/**
	 * @brief	Constructor.
	 *
	 * @param	usMyPort	my port.
	 * @param	uiPeerID	Identifier for the peer.
	 */
	ConnectMessage::ConnectMessage( unsigned short usMyPort, unsigned int uiPeerID ) :
		m_usPort( usMyPort ),
		m_uiPeerID( uiPeerID )
	{
		m_iProtocoll = SOCK_STREAM;
		m_type = MT_ConnectMessage;
	}


	/**
	 * @brief	Returns the message as string.
	 *
	 * @return	The message as string.
	 */
	std::string ConnectMessage::getMsgString() const
	{
		unsigned short usMsg[3];
		usMsg[0] = MT_ConnectMessage;
		usMsg[1] = getBodyLength();
		usMsg[2] = m_usPort;

		return std::string( (char*)usMsg, 6 ) + std::string( (char*)&m_uiPeerID, sizeof(int) );
	}


	/**
	 * @brief	Returns the size in bytes of the message body (message without header).
	 *
	 * @return	The body length in byte.
	 */
	unsigned short ConnectMessage::getBodyLength() const
	{
		return sizeof(short)+sizeof(int);
	}


	/**
	 * @brief	Creates this message out of the received byte array.
	 *
	 * @param	in	The incoming byte buffer.
	 *
	 * @return	a new ConnectMessage built from in.
	 */
	Message* ConnectMessage::create(const char * in)
	{
		unsigned short usMyPort = ((unsigned short*)in)[2];
		unsigned int uiPeerID = *((unsigned int*)&in[6]);

		return new ConnectMessage( usMyPort, uiPeerID );
	}



	// ******************** DisconnectMessage *********************

	/**
	 * @brief	Default constructor.
	 */
	DisconnectMessage::DisconnectMessage()
	{
		m_iProtocoll = SOCK_STREAM;
		m_type = MT_DisconnectMessage;
	}


	/**
	 * @brief	Returns the message as string.
	 *
	 * @return	The message as string.
	 */
	std::string DisconnectMessage::getMsgString() const
	{
		unsigned short usMsg[2];
		usMsg[0] = MT_DisconnectMessage;
		usMsg[1] = getBodyLength();

		return std::string( (char*)usMsg, 4 );
	}


	/**
	 * @brief	Returns the size in bytes of the message body (message without header).
	 *
	 * @return	The body length in byte.
	 */
	unsigned short DisconnectMessage::getBodyLength() const
	{
		return 0;
	}


	/**
	 * @brief	Creates this message out of the received byte array.
	 *
	 * @param	in	The incoming byte buffer.
	 *
	 * @return	a new DisconnectMessage built from in.
	 */
	Message* DisconnectMessage::create(const char * in)
	{
		return new DisconnectMessage();
	}



	// ******************** NewPeerMessage *********************

	/**
	 * @brief	Constructor.
	 *
	 * @param [in,out]	pPeer	If non-null, the peer.
	 */
	NewPeerMessage::NewPeerMessage( Peer* pPeer ) :
		m_pPeer( pPeer )
	{
		m_iProtocoll = 0;
		m_type = MT_NewPeerMessage;
	}


	/**
	 * @brief	Should create a new NewPeerMessage but as this message is just for internal use always returns NULL.
	 *
	 * @param	in	The incoming byte buffer.
	 *
	 * @return	NULL.
	 */
	Message* NewPeerMessage::create(const char * in)
	{
		// return an invalid message as this message is only for intra client use
		return NULL;
	}

}
