// Peer2PeerTest.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <sstream>
#include <map>

#include <Peer2PeerNetwork.h>

#include "ChatMessage.h"
#include "IntroductionMessage.h"

// code copied from http://www.spieleprogrammierer.de/18-c-cplusplus-csharp-delphi-java-python-und-lua/15218-cplusplus-konsolen-chat/

// this code writes the given string to a specified position inside the console
#ifdef linux
	inline void PortableOutput(const std::string &str, int y=0, int x=0)
	{
		mvaddwstr(y, x, str.c_str());
		refresh();
	}
#else
#include <Windows.h>

	inline void PortableOutput(const std::string &str, int y=0, int x=0)
	{
		COORD pos = { x, y };
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
		std::wcout << str.c_str();
	}
#endif

inline void clearLine( int y )
{
	PortableOutput( "                                                                 ", y );
}


/**
 * @class	Peer2PeerTest
 *
 * @brief	Test and demo for the Peer2PeerNetwork, implements a simple commandline chat.
 *
 * @author	Jörn Teuber
 * @date	2/20/2012
 */
class Peer2PeerTest : public oocl::MessageListener
{
public:

	/**
	 * @fn	void :::run()
	 *
	 * @brief	Main function that initializes the network and processes the user input.
	 *
	 * @author	Jörn Teuber
	 * @date	2/20/2012
	 */
	void run()
	{
		m_iLines = 20;
		int iChoice = 0;
		ChatMessage::registerMsg();
		IntroductionMessage::registerMsg();
	
		unsigned short usListeningPort = 0;
		std::cout << "On which port should I listen?" << std::endl;
		std::cin >> usListeningPort;

		std::cout << "What is your UserID?" << std::endl;
		std::cin >> m_uiUserID;

		std::cout << "What is your Username?" << std::endl;
		std::cin >> m_strName;

		m_mapIDtoName.insert( std::pair<unsigned int, std::string>( m_uiUserID, m_strName ) );

		m_pPeer2PeerNet = new oocl::Peer2PeerNetwork(usListeningPort, m_uiUserID);
		
		oocl::MessageBroker::getBrokerFor( MT_IntroductionMessage )->registerListener( this );
		oocl::MessageBroker::getBrokerFor( MT_ChatMessage )->registerListener( this );
		oocl::MessageBroker::getBrokerFor( MT_DisconnectMessage )->registerListener( this );
		oocl::MessageBroker::getBrokerFor( MT_NewPeerMessage )->registerListener( this );

		PortableOutput( "Instructions: type \"+IP:Port\" to add another Peer", m_iLines+2 );
		PortableOutput( "Instructions: type \"exit\" to close the chat", m_iLines+3 );

		clearLine( m_iLines );
		PortableOutput( ">", m_iLines );

		while( 1 )
		{
			std::string strInput;
			std::cin >> strInput;

			if( strInput[0] == '+' )
			{
				std::string strHostname = strInput.substr( 1, strInput.find(':')-1 );
				std::stringstream ssPort( strInput.substr( strInput.find(':')+1 ) );
				unsigned short usPort;
				ssPort >> usPort;

				oocl::Peer* pPeer = m_pPeer2PeerNet->addPeer( strHostname, usPort );
				//pPeer->subscribe( MT_ChatMessage );
				//pPeer->sendMessage( new IntroductionMessage( m_uiUserID, m_strName ) );

				clearLine( m_iLines );
				PortableOutput( ">", m_iLines );
			}
			else if( strInput == "exit" )
			{
				break;
			}
			else if( strInput == "connected" )
			{
				std::cout << m_pPeer2PeerNet->getPeerList()->size() << " Peers connected" << std::endl;
				clearLine( m_iLines );
				PortableOutput( ">", m_iLines );
			}
			else
			{
				oocl::MessageBroker::getBrokerFor( MT_ChatMessage )->pumpMessage( new ChatMessage( m_uiUserID, strInput )  );
			}
		}

		m_lChatLines.clear();
		m_mapIDtoName.clear();

		delete m_pPeer2PeerNet;
	}


	bool cbMessage( oocl::Message* pMessage )
	{
		unsigned short usType = pMessage->getType();

		if( usType == MT_ChatMessage )
		{
			std::string strUsername = m_mapIDtoName.find( ((ChatMessage*)pMessage)->getPeerID() )->second;

			m_lChatLines.push_back( "<"+strUsername+"> " + ((ChatMessage*)pMessage)->getMessage() );
		}
		else if( usType == MT_NewPeerMessage )
		{
			oocl::Peer* pPeer = ((oocl::NewPeerMessage*)pMessage)->getPeer();
			pPeer->subscribe( MT_ChatMessage );
			pPeer->sendMessage( new IntroductionMessage( m_uiUserID, m_strName ) );
		}
		else if( usType == MT_IntroductionMessage )
		{
			m_mapIDtoName.insert( std::pair<unsigned int, std::string>( ((IntroductionMessage*)pMessage)->getPeerID(),  ((IntroductionMessage*)pMessage)->getUsername() ) );
			PortableOutput( ((IntroductionMessage*)pMessage)->getUsername(), m_iLines+1, (m_mapIDtoName.size()-1)*12 );
			PortableOutput( ">", m_iLines );
			m_lChatLines.push_back( ((IntroductionMessage*)pMessage)->getUsername()+" connected!" );
		}
		else
		{
			std::cout << "unknown message type!" << std::endl;
		}
		
		if( m_lChatLines.size() > m_iLines )
			m_lChatLines.pop_front();

		int i=0;
		for( std::list<std::string>::iterator it = m_lChatLines.begin(); it != m_lChatLines.end(); it++ )
		{
			clearLine(i);
			PortableOutput( (*it), i );
			i++;
		}
			
		clearLine( m_iLines );
		PortableOutput( ">", m_iLines );

		return true;
	}
	
private:
	oocl::Peer2PeerNetwork* m_pPeer2PeerNet;

	std::list<std::string> m_lChatLines;
	int m_iLines;

	std::map<unsigned int, std::string> m_mapIDtoName;

	std::string		m_strName;
	unsigned int	m_uiUserID;
};


int _tmain(int argc, _TCHAR* argv[])
{
	Peer2PeerTest* pTest = new Peer2PeerTest();

	pTest->run();

	return 0;
}

