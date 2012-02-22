
#include "ExplicitMessages.h"

namespace oocl
{

	// ******************** StandardMessage *********************

	StandardMessage::StandardMessage( const char * cMsgBody, unsigned short length  ) :
		m_strMsgBody( std::string(cMsgBody, length) )
	{
		m_iProtocoll = SOCK_STREAM;
		m_type = MT_StandardMessage;
	}

	StandardMessage::StandardMessage( std::string strMsgBody  ) :
		m_strMsgBody( strMsgBody )
	{
		m_iProtocoll = SOCK_STREAM;
		m_type = MT_StandardMessage;
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
		return new StandardMessage( &(in[4]), ((unsigned short*)in)[1] );
	}
	

	// ******************** SubscribeMessage *********************

	SubscribeMessage::SubscribeMessage( unsigned short usTypeToSubscribe ) :
		m_usTypeToSubscribe( usTypeToSubscribe )
	{
		m_iProtocoll = SOCK_STREAM;
		m_type = MT_SubscribeMessage;
	}

	std::string SubscribeMessage::getMsgString()
	{
		unsigned short usMsg[3];
		usMsg[0] = MT_SubscribeMessage;
		usMsg[1] = sizeof(unsigned short);
		usMsg[2] = m_usTypeToSubscribe;

		return std::string( (char*)usMsg, 6 );
	}

	Message* SubscribeMessage::create(const char * in)
	{
		return new SubscribeMessage( ((unsigned short*)in)[2] );
	}
	

	// ******************** ConnectMessage *********************

	ConnectMessage::ConnectMessage( unsigned short usMyPort, unsigned int uiPeerID ) :
		m_usPort( usMyPort ),
		m_uiPeerID( uiPeerID )
	{
		m_iProtocoll = SOCK_STREAM;
		m_type = MT_ConnectMessage;
	}

	std::string ConnectMessage::getMsgString()
	{
		unsigned short usMsg[3];
		usMsg[0] = MT_ConnectMessage;
		usMsg[1] = sizeof(short)+sizeof(int);
		usMsg[2] = m_usPort;

		return std::string( (char*)usMsg, 6 ) + std::string( (char*)&m_uiPeerID, sizeof(int) );
	}

	Message* ConnectMessage::create(const char * in)
	{
		unsigned short usMyPort = ((unsigned short*)in)[2];
		unsigned int uiPeerID = *((unsigned int*)&in[6]);

		return new ConnectMessage( usMyPort, uiPeerID );
	}



	// ******************** DisconnectMessage *********************

	DisconnectMessage::DisconnectMessage()
	{
		m_iProtocoll = SOCK_STREAM;
		m_type = MT_DisconnectMessage;
	}

	std::string DisconnectMessage::getMsgString()
	{
		unsigned short usMsg[2];
		usMsg[0] = MT_DisconnectMessage;
		usMsg[1] = 0;

		return std::string( (char*)usMsg, 4 );
	}

	Message* DisconnectMessage::create(const char * in)
	{
		return new DisconnectMessage();
	}



	// ******************** NewPeerMessage *********************

	NewPeerMessage::NewPeerMessage( Peer* pPeer ) :
		m_pPeer( pPeer )
	{
		m_iProtocoll = SOCK_STREAM;
		m_type = MT_NewPeerMessage;
	}

	std::string NewPeerMessage::getMsgString()
	{
		// return an invalid message as this message is only for intra client use
		unsigned short usMsg[2];
		usMsg[0] = 0;
		usMsg[1] = 0;

		return std::string( (char*)usMsg, 4 );
	}

	Message* NewPeerMessage::create(const char * in)
	{
		// return an invalid message as this message is only for intra client use
		return NULL;
	}

}