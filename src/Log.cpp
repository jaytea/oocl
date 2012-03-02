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

#include "Log.h"

namespace oocl
{
	std::map<std::string, Log*> Log::sm_mapLogs;
	Log* Log::sm_pDefaultLog = getLog("default");


	/**
	 * @fn	Log::Log( std::string strLogName )
	 *
	 * @brief	Constructor.
	 *
	 * @author	Jörn Teuber
	 * @date	11/25/2011
	 *
	 * @param	strLogName	Name of the log.
	 */
	Log::Log( std::string strLogName ) :
		m_elLowestLoggedLevel( EL_INFO )
	{
		m_fsLogFile.open( strLogName+std::string(".log") );
	}


	/**
	 * @fn	Log::~Log(void)
	 *
	 * @brief	Destructor.
	 *
	 * @author	Jörn Teuber
	 * @date	11/25/2011
	 */
	Log::~Log(void)
	{
		flush();
		m_fsLogFile.close();
	}


	/**
	 * @fn	Log* Log::getLog( std::string strLogName )
	 *
	 * @brief	get the log with the given name.
	 *
	 * @author	Jörn Teuber
	 * @date	11/25/2011
	 *
	 * @param	strLogName	Name of the log.
	 *
	 * @return	null if it fails, else the log.
	 */
	Log* Log::getLog( std::string strLogName )
	{
		std::map< std::string, Log* >::iterator it = sm_mapLogs.find( strLogName );
		if( it == sm_mapLogs.end() )
		{
			Log* pLog = new Log(strLogName);
			sm_mapLogs.insert( std::pair<std::string, Log* >( strLogName, pLog ) );
			return pLog;
		}
		else
			return it->second;
	}


	/**
	 * @fn	inline Log* Log::getDefaultLog()
	 *
	 * @brief	Gets the default log.
	 *
	 * @author	Jörn Teuber
	 * @date	1/27/2012
	 *
	 * @return	null if it fails, else the default log.
	 */
	Log* Log::getDefaultLog()
	{
		return sm_pDefaultLog;
	}


	/**
	 * @fn	void Log::setDefaultLogName( const std::string strDefaultLogName )
	 *
	 * @brief	Sets the name of the default log.
	 *
	 * @author	Jörn Teuber
	 * @date	1/27/2012
	 *
	 * @param	strDefaultLogName	The name of the default log.
	 */
	void Log::setDefaultLog( const std::string strDefaultLogName )
	{
		sm_pDefaultLog = getLog( strDefaultLogName );
	}


	/**
	 * @fn	bool Log::logMessage( const std::string strMessage, int iErrorLevel = 0 )
	 *
	 * @brief	Logs a message as info if no second parameter is given.
	 * 			
	 * @note	in debug mode every message will be written to the standard output and directly to the logfile.
	 *
	 * @author	Jörn Teuber
	 * @date	11/25/2011
	 *
	 * @param	strMessage 	Message to log
	 * @param	iErrorLevel	(optional) error level.
	 *
	 * @return	true if the message was logged, false if not.
	 */
	bool Log::logMessage( const std::string strMessage, EErrorLevel elErrorLevel )
	{
		if( elErrorLevel < m_elLowestLoggedLevel )
			return false;

		std::string strPrefix;

		switch(elErrorLevel)
		{
		case EL_INFO:			strPrefix = "INFO       : "; break;
		case EL_WARNING:		strPrefix = "WARNING    : "; break;
		case EL_ERROR:			strPrefix = "ERROR      : "; break;
		case EL_FATAL_ERROR:	strPrefix = "FATAL ERROR: "; break;
		default:	return false;
		}

		m_strLogText += strPrefix + strMessage + "\n";

#ifdef _DEBUG
		std::cout << m_strLogText;
		flush();
#endif

		return true;
	}


	/**
	 * @fn	bool Log::logInfo( const std::string strInfo )
	 *
	 * @brief	Same as logMessage(strInfo) but possibly a little bit faster.
	 *
	 * @author	Jörn Teuber
	 * @date	1/26/2012
	 *
	 * @param	strInfo	The info.
	 *
	 * @return	true if the message was logged, false if not.
	 */
	bool Log::logInfo( const std::string strInfo )
	{
		if( m_elLowestLoggedLevel > EL_INFO )
			return false;

		m_strLogText += "INFO       : " + strInfo + "\n";

#ifdef _DEBUG
		std::cout << m_strLogText;
		flush();
#endif

		return true;
	}


	/**
	 * @fn	inline bool Log::logWarning( const std::string strWarning )
	 *
	 * @brief	Same as logMessage(..., EL_WARNING), but faster and lazzier.
	 *
	 * @author	Jörn Teuber
	 * @date	1/26/2012
	 *
	 * @param	strWarning	The warning.
	 *
	 * @return	true if the message was logged, false if not.
	 */
	bool Log::logWarning( const std::string strWarning )
	{
		if( m_elLowestLoggedLevel > EL_WARNING )
			return false;

		m_strLogText += "WARNING    : " + strWarning + "\n";

#ifdef _DEBUG
		std::cout << m_strLogText;
		flush();
#endif

		return true;
	}
	

	/**
	 * @fn	inline bool Log::logError( const std::string strError )
	 *
	 * @brief	Same as logMessage(..., EL_ERROR), but faster and lazzier.
	 *
	 * @author	Jörn Teuber
	 * @date	1/26/2012
	 *
	 * @param	strError	The error message.
	 *
	 * @return	true if the message was logged, false if not.
	 */
	bool Log::logError( const std::string strError )
	{
		if( m_elLowestLoggedLevel > EL_ERROR )
			return false;

		m_strLogText += "ERROR      : " + strError + "\n";

#ifdef _DEBUG
		std::cout << m_strLogText;
		flush();
#endif

		return true;
	}
	

	/**
	 * @fn	inline bool Log::logFatalError( const std::string strError )
	 *
	 * @brief	Same as logMessage(..., EL_FATAL_ERROR), but faster and lazzier.
	 *
	 * @author	Jörn Teuber
	 * @date	1/26/2012
	 *
	 * @param	strError	The error message.
	 *
	 * @return	true if the message was logged, false if not.
	 */
	bool Log::logFatalError( const std::string strError )
	{
		if( m_elLowestLoggedLevel > EL_FATAL_ERROR )
			return false;

		m_strLogText += "ERROR      : " + strError + "\n";

#ifdef _DEBUG
		std::cout << m_strLogText;
		flush();
#endif

		return true;
	}


	/**
	 * @fn	bool Log::flush()
	 *
	 * @brief	writes the current log to the file so that it is safe.
	 *
	 * @author	Jörn Teuber
	 * @date	11/25/2011
	 */
	void Log::flush()
	{
		m_fsLogFile << m_strLogText;
		m_fsLogFile.flush();

		m_strLogText.clear();
	}


	/**
	 * @fn	void Log::setLogLevel( EErrorLevel elLowestLoggedLevel )
	 *
	 * @brief	Sets the lowest message level that will still be logged.
	 * 			
	 * @note	This call effects all subsequent calls of the logging methods. 
	 * 			If you do not want any logging pass 4 (EL_FATAL_ERROR+1).
	 *
	 * @author	Jörn Teuber
	 * @date	2/22/2012
	 *
	 * @param	elLowestLoggedLevel	The lowest logged message level.
	 */
	void Log::setLogLevel( EErrorLevel elLowestLoggedLevel ) 
	{
		m_elLowestLoggedLevel = elLowestLoggedLevel; 
	}

}