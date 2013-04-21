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
	Log* Log::sm_pDefaultLog = NULL;
	
	std::string Log::sm_astrLogLevelToPrefix[] = { "INFO       : ", "WARNING    : ", "ERROR      : ", "FATAL ERROR: " };


	/**
	 * @fn	Log::Log( std::string strLogName )
	 *
	 * @brief	Constructor.
	 *
	 * @param	strLogName	Name of the log.
	 */
	Log::Log( std::string strLogName ) 
		: m_elLowestLoggedLevel( EL_INFO )
		, m_elLastStreamLogLvl( EL_INFO )
	{
		m_fsLogFile.open( std::string(strLogName+std::string(".log")).c_str() );
	}


	/**
	 * @fn	Log::~Log(void)
	 *
	 * @brief	Destructor.
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
	 * @fn	Log* Log::getLog( std::string strLogName )
	 *
	 * @brief	get a reference to the log with the given name.
	 *
	 * @param	strLogName	Name of the log.
	 *
	 * @return	null if it fails, else the log.
	 */
	Log& Log::getLogRef( std::string strLogName )
	{	
		return *getLog( strLogName );
	}


	/**
	 * @fn	Log* Log::getDefaultLog()
	 *
	 * @brief	Gets the default log.
	 *
	 * @return	null if it fails, else the default log.
	 */
	Log* Log::getDefaultLog()
	{
		return sm_pDefaultLog;
	}

	/**
	 * @fn	Log* Log::getDefaultLogRef()
	 *
	 * @brief	Get a reference to the default log.
	 *
	 * @return	null if it fails, else the default log.
	 */
	Log& Log::getDefaultLogRef()
	{
		return *sm_pDefaultLog;
	}


	/**
	 * @fn	void Log::setDefaultLogName( const std::string strDefaultLogName )
	 *
	 * @brief	Sets the name of the default log.
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
	 * @param	strMessage 	Message to log
	 * @param	iErrorLevel	(optional) error level.
	 *
	 * @return	true if the message was logged, false if not.
	 */
	bool Log::logMessage( const std::string strMessage, EErrorLevel elErrorLevel )
	{
		if( elErrorLevel < m_elLowestLoggedLevel || elErrorLevel > sm_uiMaxLogLevel )
			return false;

		std::string strPrefix = sm_astrLogLevelToPrefix[elErrorLevel];

		m_strLogText += strPrefix + strMessage + "\n";

		std::cout << m_strLogText;
		flush();

		return true;
	}


	/**
	 * @fn	bool Log::logInfo( const std::string strInfo )
	 *
	 * @brief	Same as logMessage(strInfo) but possibly a little bit faster.
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

		std::cout << m_strLogText;
		flush();

		return true;
	}


	/**
	 * @fn	inline bool Log::logWarning( const std::string strWarning )
	 *
	 * @brief	Same as logMessage(..., EL_WARNING), but faster and lazier.
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

		std::cout << m_strLogText;
		flush();

		return true;
	}
	

	/**
	 * @fn	inline bool Log::logError( const std::string strError )
	 *
	 * @brief	Same as logMessage(..., EL_ERROR), but faster and lazier.
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

		std::cout << m_strLogText;
		flush();

		return true;
	}
	

	/**
	 * @fn	inline bool Log::logFatalError( const std::string strError )
	 *
	 * @brief	Same as logMessage(..., EL_FATAL_ERROR), but faster and lazier.
	 *
	 * @param	strError	The error message.
	 *
	 * @return	true if the message was logged, false if not.
	 */
	bool Log::logFatalError( const std::string strError )
	{
		if( m_elLowestLoggedLevel > EL_FATAL_ERROR )
			return false;

		m_strLogText += "FATAL ERROR: " + strError + "\n";

		std::cout << m_strLogText;
		flush();

		return true;
	}


	/**
	 * @fn	bool Log::flush()
	 *
	 * @brief	writes the current log to the file so that it is safe.
	 */
	void Log::flush()
	{
		m_fsLogFile << m_strLogText;
		m_fsLogFile << m_ssLogStream.str();
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
	 * @param	elLowestLoggedLevel	The lowest logged message level.
	 */
	void Log::setLogLevel( EErrorLevel elLowestLoggedLevel ) 
	{
		m_elLowestLoggedLevel = elLowestLoggedLevel; 
	}
	
	/**
	 * @brief	Inserts the error level prefix into the log "stream"
	 * 
     * @param	eLvl	error level to log in the subsequent stream operator calls
	 * 
     * @return	a reference to this log
     */
	Log& Log::operator << (const EErrorLevel eLvl)
	{
		if( eLvl <= sm_uiMaxLogLevel )
		{
			m_ssLogStream << sm_astrLogLevelToPrefix[eLvl];
		}
		
		m_elLastStreamLogLvl = eLvl;
		
		return *this;
	}
	
	/**
	 * @brief	Stream operator for function pointers for the manipulators
	 * 
     * @param	logmanipulator	function pointer to the manipulator, i.e. oocl::endl
	 * 
     * @return	a reference to this log
     */
	Log& Log::operator << (Log& (*logmanipulator)(Log&) ) 
	{ 
		return logmanipulator(*this);
	}
	
		
	Log& Log::operator << (const char* pc)
	{
		m_ssLogStream << pc;
		return *this;
	}
	
	Log& Log::operator << (const std::string str)
	{
		m_ssLogStream << str;
		return *this;
	}
	

	Log& Log::operator << (const bool b)
	{
		m_ssLogStream << b;
		return *this;
	}
	
	Log& Log::operator << (const float f)
	{
		m_ssLogStream << f;
		return *this;
	}
	
	Log& Log::operator << (const double d)
	{
		m_ssLogStream << d;
		return *this;
	}
	

	Log& Log::operator << (const short int i)
	{
		m_ssLogStream << i;
		return *this;
	}
	
	Log& Log::operator << (const unsigned short int i)
	{
		m_ssLogStream << i;
		return *this;
	}
	
	Log& Log::operator << (const int i)
	{
		m_ssLogStream << i;
		return *this;
	}
	
	Log& Log::operator << (const unsigned int i)
	{
		m_ssLogStream << i;
		return *this;
	}
	
	Log& Log::operator << (const long long i)
	{
		m_ssLogStream << i;
		return *this;
	}
	
	Log& Log::operator << (const unsigned long long i)
	{
		m_ssLogStream << i;
		return *this;
	}
	
	
	Log& endl(Log& log)
	{ 
		if( log.m_elLastStreamLogLvl >= log.m_elLowestLoggedLevel )
		{
			std::cout << log.m_ssLogStream.str() << std::endl;
			log.m_ssLogStream << '\n';

			log.flush();
		}
		
		log.m_ssLogStream.str("");
		log.m_ssLogStream.clear();
		
		return log; 
	}
}