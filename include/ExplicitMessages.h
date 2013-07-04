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

#ifndef EXPLICITMESSAGES_H_INCLUDED
#define EXPLICITMESSAGES_H_INCLUDED


/**
 * @file ExplicitMessages.h
 * 		 
 *	The classes in this file are all messages that are needed for the DirectConNetwork and Peer2PeerNetwork
 */

#include "Message.h"

namespace oocl
{
	
#define MT_StandardMessage 1
	/**
	 * @brief	A basic message which just holds a string as body.
	 *
	 * @author	Jörn Teuber
	 * @date	1.3.2012
	 */
	class OOCL_EXPORTIMPORT StandardMessage : public Message
	{
	public:
		/** @brief registers the message type with the message system
		 *
		 *  @note must be present in every implementation of Message!
		 */
		static void registerMsg(){
			Message::registerMsg( MT_StandardMessage, StandardMessage::create );
		}

		StandardMessage( const char * cMsgBody, unsigned short length );
		StandardMessage( std::string strMsgBody );

		virtual std::string getMsgString() const;
		virtual unsigned short getBodyLength() const;

		std::string getBody() const { return m_strMsgBody; }
		void setProtocoll( int iProtocoll ) { m_iProtocoll = iProtocoll; }

	protected:
		static Message* create(const char * in);

	private:
		std::string m_strMsgBody;
	};

	
#define MT_SubscribeMessage 2
	/**
	 * @brief	Message for subscribing a given message type at the receiver.
	 *
	 * @author	Jörn Teuber
	 * @date	3/1/2012
	 */
	class OOCL_EXPORTIMPORT SubscribeMessage : public Message
	{
	public:
		/** @brief registers the message type with the message system
		 *
		 *  @note must be present in every implementation of Message!
		 */
		static void registerMsg(){
			Message::registerMsg( MT_SubscribeMessage, SubscribeMessage::create );
		}

		SubscribeMessage( unsigned short usTypeToSubscribe );

		virtual std::string getMsgString() const;
		virtual unsigned short getBodyLength() const;

		// getter
		unsigned short getTypeToSubscribe() const { return m_usTypeToSubscribe; }

	protected:
		static Message* create(const char * in);

	private:
		unsigned short m_usTypeToSubscribe;
	};
	
	
#define MT_ConnectMessage 3
	/**
	 * @brief	Message that contains all necessary information for connecting.
	 *
	 * @author	Jörn Teuber
	 * @date	3/1/2012
	 */
	class OOCL_EXPORTIMPORT ConnectMessage : public Message
	{
	public:
		/** @brief registers the message type with the message system
		 *
		 *  @note must be present in every implementation of Message!
		 */
		static void registerMsg(){
			Message::registerMsg( MT_ConnectMessage, ConnectMessage::create );
		}

		ConnectMessage( unsigned short usMyPort, unsigned int uiPeerID = 0 );

		virtual std::string getMsgString() const;
		virtual unsigned short getBodyLength() const;

		unsigned short getPort() const { return m_usPort; }
		unsigned int getPeerID() const { return m_uiPeerID; }

	protected:
		static Message* create(const char * in);

	private:
		unsigned short	m_usPort;
		unsigned int	m_uiPeerID;
	};
	
	
#define MT_DisconnectMessage 4
	/**
	 * @brief	Say goodbye to the receiver.
	 *
	 * @author	Jörn Teuber
	 * @date	3/1/2012
	 */
	class OOCL_EXPORTIMPORT DisconnectMessage : public Message
	{
	public:
		/** @brief registers the message type with the message system
		 *
		 *  @note must be present in every implementation of Message!
		 */
		static void registerMsg(){
			Message::registerMsg( MT_DisconnectMessage, DisconnectMessage::create );
		}

		DisconnectMessage();

		virtual std::string getMsgString() const;
		virtual unsigned short getBodyLength() const;

	protected:
		static Message* create(const char * in);
	};
	
	class Peer;
#define MT_NewPeerMessage 5
	/**
	 * @brief	In client message that gets sent whenever a new peer has connected.
	 *
	 * @author	Jörn Teuber
	 * @date	3/1/2012
	 */
	class OOCL_EXPORTIMPORT NewPeerMessage : public Message
	{
	public:
		/** @brief registers the message type with the message system
		 *
		 *  @note must be present in every implementation of Message!
		 */
		static void registerMsg(){
			Message::registerMsg( MT_NewPeerMessage, NewPeerMessage::create );
		}

		NewPeerMessage( Peer* pPeer );
		
		Peer* getPeer() const { return m_pPeer; }

	protected:
		static Message* create(const char * in);

	private:
		Peer* m_pPeer;
	};
}

#endif // EXPLICITMESSAGES_H_INCLUDED
