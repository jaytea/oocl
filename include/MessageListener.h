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
/// This file was written by Jürgen Lorenz and Jörn Teuber

#ifndef MESSAGELISTENER_H_INCLUDED
#define MESSAGELISTENER_H_INCLUDED

#include "oocl_import_export.h"

#include "Message.h"

namespace oocl
{
	/**
	 * @class	MessageListener
	 *
	 * @brief	Message listener.
	 *
	 * @author	Jörn Teuber
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
		 * @author	Jörn Teuber
		 * @date	11/25/2011
		 */
		MessageListener(void) {}

		/**
		 * @fn	virtual MessageListener::~MessageListener(void)
		 *
		 * @brief	Destructor.
		 *
		 * @author	Jörn Teuber
		 * @date	11/25/2011
		 */
		virtual ~MessageListener(void) {};

		/**
		 * @fn	virtual bool MessageListener::cbMessage( Message* pMessage ) = 0;
		 *
		 * @brief	This is the callback method for the messages.
		 * 			
		 * @note	in the standard implementation this method will be called at the same time to be thread-safe. 
		 * 			If you do not want this behaviour see the notes at requestMutex().
		 *
		 * @author	Jörn Teuber
		 * @date	11/25/2011
		 *
		 * @param [in,out]	pMessage	If non-null, the message.
		 *
		 * @return	true if it succeeds, false if it fails.
		 */
		virtual bool cbMessage( Message* pMessage ) = 0;

		/**
		 * @fn	virtual bool MessageListener::requestMutex()
		 *
		 * @brief	will always be called before cbMessage() to be thread-safe.
		 * 			
		 * @note	you can implement this for your listeners if you do not need this strikt thread safety.
		 *
		 * @author	Jörn Teuber
		 * @date	11/25/2011
		 *
		 * @return	true if cbMessage can be called safely, false if not
		 */
		virtual bool requestMutex() 
		{ 
			if( m_bMutex )
			{
				m_bMutex = false;
				return true;
			}

			return false;
		}

		/**
		 * @fn	virtual void MessageListener::returnMutex()
		 *
		 * @brief	Returns the mutex.
		 *
		 * @author	Jörn Teuber
		 * @date	11/25/2011
		 */
		virtual void returnMutex()
		{
			m_bMutex = true;
		}

	protected:

		///< true if the mutex is free
		bool m_bMutex; 
	};

}

#endif