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
// This file was written by Jürgen Lorenz and Jörn Teuber

#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#include "oocl_import_export.h"

#include "SocketStub.h"

namespace oocl
{
	/**
	 * @brief	Interface for all sockets.
	 *
	 * @author	Jörn Teuber
	 * @date	02.03.2012
	 */
	class OOCL_EXPORTIMPORT Socket : public SocketStub
	{
		friend class ServerSocket;
		
	public:
		Socket();
		virtual ~Socket() {}

		/**
		 * @brief	Connect the socket to the host with given hostname on given port.
		 *
		 * @return	True if it succeeds, false if not.
		 */
		virtual bool connect( std::string host, unsigned short usPort ) = 0;

		/**
		 * @brief	Connect the socket to the host with given IP on given port.
		 *
		 * @return	True if it succeeds, false if not.
		 */
		virtual bool connect( unsigned int uiHostIP, unsigned short usPort ) = 0;

		/**
		 * @brief	Binds the socket to the given port.
		 *
		 * @param	usPort	The port.
		 *
		 * @return	true if it succeeds, false if it fails.
		 */
		virtual bool bind( unsigned short usPort ) = 0;


		/**
		 * @brief	Check whether the socket was created successfully.
		 *
		 * @return	True if this socket is valid, i.e. was created successfully, false if not.
		 */
		virtual bool isValid() = 0;

		/**
		 * @brief	Check whether the socket was successfully connected.
		 *
		 * @return	True if this socket is connected, false if not.
		 */
		virtual bool isConnected() = 0;

		/**
		 * @brief	Receives a package of maximum length count and stores it in a given string.
		 *
		 * @param	str		The string to write the received bytes into.
		 * @param	count	The number of bytes to receive.
		 *
		 * @return	false if encountered any errors, else true.
		 */
		virtual bool read( std::string& str, int count = 0 ) = 0;

		/**
		 * @brief	Receive exactly one byte.
		 *
		 * @param	c	The reference to the read byte.
		 *
		 * @return	false if encountered any errors, else true.
		 */
		virtual bool read( char& c ) = 0;

		/**
		 * @brief	Receives a package with max count size, returns a char array and stores the number of actually received bytes in readCount.
		 *
		 * @param	pcBuf	Pre-allocated buffer in which the received data will be written.
		 * @param 	count	The size of the buffer, will be set to the number of actually received bytes.
		 *
		 * @return	false if encountered any errors, else true.
		 */
		virtual bool read( char* pcBuf, int& count ) = 0;

		/**
		 * @brief	Receives from an unconnected socket, i.e. UDP.
		 *
		 * @param 	str		The received bytes as string.
		 * @param	count	Maximum number of bytes to read.
		 * @param	hostIP	If non-null, the ip of the peer that sent the package.
		 *
		 * @return	false if encountered any errors, else true.
		 */
		virtual bool readFrom( std::string& str, int count = 0, unsigned int* hostIP = NULL ) = 0;


		/**
		 * @brief	Send a package to the connected process.
		 *
		 * @param	in	The package as string.
		 *
		 * @return	true if it succeeds, false if it fails.
		 */
		virtual bool write(std::string in) = 0;

		/**
		 * @brief	Sends one byte to the connected process.
		 *
		 * @param	in	The byte to send.
		 *
		 * @return	true if it succeeds, false if it fails.
		 */
		virtual bool write(char in) = 0;

		/**
		 * @brief	Sends a byte array to the connected process.
		 *
		 * @param	in   	The byte array.
		 * @param	count	Number of bytes to send.
		 *
		 * @return	true if it succeeds, false if it fails.
		 */
		virtual bool write(const char * in, int count) = 0;

		/**
		 * @brief	Sends a package .
		 *
		 * @param	in  	The package to send as string.
		 * @param	host	The host to send to.
		 * @param	port	The port of the host.
		 *
		 * @return	true if it succeeds, false if it fails.
		 */
		virtual bool writeTo( std::string in, std::string host, unsigned short port ) = 0;


		/**
		 * @brief	Closes this socket.
		 */
		virtual void close() = 0;

		/**
		 * @brief	Get the IP this socket is connected to.
		 *
		 * @return	The IP this socket is connected to.
		 */
		virtual unsigned int getConnectedIP() = 0;
	};

}

#endif // SOCKET_H_INCLUDED
