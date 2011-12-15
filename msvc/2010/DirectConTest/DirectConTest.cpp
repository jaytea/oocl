// DirectConTest.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"

#include <DirectConNetwork.h>
#include <ExplicitMessages.h>

#include <iostream>

class DirectConTest : public oocl::MessageListener
{
public:

	void run()
	{
		int iChoice = 0;
		m_pDirectConNet = new oocl::DirectConNetwork();
		oocl::StandardMessage::registerMsg();
		oocl::DisconnectMessage::registerMsg();

		while( iChoice != 1 && iChoice != 2 )
		{
			std::cout << "should I wait for another process to connect (1) or should I connect to another process (2)?" << std::endl;
			std::cin >> iChoice;
		}

		if( iChoice == 1 )
		{
			unsigned short usPort = 0;
			std::cout << "On which port should I listen?" << std::endl;
			std::cin >> usPort;
			
			std::cout << "TCP (1) or UDP (2)?" << std::endl;
			std::cin >> iChoice;

			m_pDirectConNet->listen( usPort, iChoice );

			m_pDirectConNet->registerListener( this );

			// listen is a non blocking call as it starts a thread, so wait until another process has connected
			while( !m_pDirectConNet->isConnected() )
				Sleep(0);

		}
		else if( iChoice == 2 )
		{
			std::string strHostname;
			std::cout << "Specify the host to connect to!" << std::endl;
			std::cin >> strHostname;

			unsigned short usPort = 0;
			std::cout << "On which port should i connect?" << std::endl;
			std::cin >> usPort;

			std::cout << "TCP (1) or UDP (2)?" << std::endl;
			std::cin >> iChoice;

			m_pDirectConNet->connect( strHostname, usPort, iChoice );

			m_pDirectConNet->registerListener( this );
		}

		while( m_pDirectConNet->isConnected() )
		{
			std::cout << "send (1) or wait for (2) a message or disconnect (3)?" << std::endl;
			std::cin >> iChoice;

			if( iChoice == 1 )
			{
				std::string strMessage;
				std::cout << "What do you want to send?" << std::endl;
				std::cin >> strMessage;

				oocl::StandardMessage* pMsg = new oocl::StandardMessage( strMessage );
				m_pDirectConNet->sendMsg( pMsg );

				std::cout << "Message sent!" << std::endl;
			}
			else if( iChoice == 2 )
			{
				m_bWaitForMessage = true;
				while( m_bWaitForMessage )
					Sleep(500);
			}
			else if( iChoice == 3 )
			{
				m_pDirectConNet->disconnect();
			} 
		}
	}

	bool cbMessage( oocl::Message* pMessage )
	{
		std::cout << "A message was received!" << std::endl;

		unsigned short usType = pMessage->getType();

		if( usType == MT_StandardMessage )
		{
			std::cout << ((oocl::StandardMessage*)pMessage)->getBody() << std::endl;
		}
		else if( usType == MT_DisconnectMessage )
		{
			std::cout << "Peer disconnected" << std::endl;
		}
		else
		{
			std::cout << "unknown message type!" << std::endl;
		}

		m_bWaitForMessage = false;

		return true;
	}
	
private:
	oocl::DirectConNetwork* m_pDirectConNet;

	bool m_bWaitForMessage;
};


int _tmain(int argc, _TCHAR* argv[])
{
	DirectConTest* pTest = new DirectConTest();

	pTest->run();

	return 0;
}

