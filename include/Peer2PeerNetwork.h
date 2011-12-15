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
/// This file was written by Jörn Teuber

#ifndef PEER2PEERNETWORK_H
#define PEER2PEERNETWORK_H

#include <list>

#include "Thread.h"
#include "MessageBroker.h"
#include "ExplicitMessages.h"
#include "Peer.h"
#include "ServerSocket.h"

namespace oocl
{
	/**
	 * @class	Peer2PeerNetwork
	 *
	 * @brief	Peer 2 peer network.
	 * 			
	 * @note	sends message: NewPeerMessage
	 *
	 * @author	Jörn Teuber
	 * @date	12/8/2011
	 */
	class OOCL_EXPORTIMPORT Peer2PeerNetwork : public Thread
	{
	public:
		Peer2PeerNetwork( unsigned short usListeningPort ); 
		~Peer2PeerNetwork(void);

		bool addPeer( Peer* pPeer );
		
	protected:
		virtual void run();

	private:
		std::list<Peer*> m_lpPeers;
		std::list<Socket*> m_lpSocketsWithoutPeers;

		bool m_bActive;
		unsigned short m_usListeningPort;
	};

}

#endif // PEER2PEERNETWORK_H