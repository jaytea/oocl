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
/// This file was written by J�rgen Lorenz and J�rn Teuber

#ifndef MESSAGELISTENER_H_INCLUDED
#define MESSAGELISTENER_H_INCLUDED

#include "oocl_import_export.h"

#include "Message.h"

namespace oocl
{
	/**
	 * @class	MessageListener
	 *
	 * @brief	Interface for all classes that need to receive messages of any type.
	 *
	 * @author	J�rn Teuber
	 * @date	11/25/2011
	 */
	class OOCL_EXPORTIMPORT MessageListener
	{
	public:

		/**
		 * @fn	MessageListener::MessageListener(void)
		 *
		 * @brief	Default constructor.
		 *
		 * @author	J�rn Teuber
		 * @date	11/25/2011
		 */
		MessageListener(void) {}

		/**
		 * @fn	virtual MessageListener::~MessageListener(void)
		 *
		 * @brief	Destructor.
		 *
		 * @author	J�rn Teuber
		 * @date	11/25/2011
		 */
		virtual ~MessageListener(void) {};

		/**
		 * @fn	virtual bool MessageListener::cbMessage( Message* pMessage ) = 0;
		 *
		 * @brief	This is the callback method for the messages.
		 * 			
		 * @note	Please note that every message type has its own thread to deliver the messages.
		 * 			So be sure to either only process thread-safe data here or make the data thread-safe.
		 * 			If you do not want to handle the message now you can return false to let the message delivery thread return to you with this messsage later.
		 *
		 * @author	J�rn Teuber
		 * @date	11/25/2011
		 *
		 * @param [in,out]	pMessage	If non-null, the message.
		 *
		 * @return	return true if you have processed the data, false if you want to be called later with the same message.
		 */
		virtual bool cbMessage( Message* pMessage ) = 0;

	protected:

		///< true if the mutex is free
		bool m_bMutex; 
	};

}

#endif