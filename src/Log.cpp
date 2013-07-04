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
	
	std::string Log::sm_astrLogLevelToPrefix[] = { " INFO       : ", " WARNING    : ", " ERROR      : ", " FATAL ERROR: " };


	/**
	 * @brief	Constructor.
	 *
	 * @param	strLogName	Name of the log.
	 */
	Log::Log( std::string strLogName ) 
		: m_elLowestLoggedLevel( EL_INFO )
		, m_elLastStreamLogLvl( EL_INFO )
		, m_bFlushing( false )
	{
		m_fsLogFile.open( std::string(strLogName+std::string(".log")).c_str() );
	}


	/**
	 * @brief	Destructor.
	 */
	Log::~Log(void)
	{
		flush();
		m_fsLogFile.close();
	}


	/**
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
	 * @brief	Gets the default log.
	 *
	 * @return	null if it fails, else the default log.
	 */
	Log* Log::getDefaultLog()
	{
		return sm_pDefaultLog;
	}

	/**
	 * @brief	Get a reference to the default log.
	 *
	 * @return	null if it fails, else the default log.
	 */
	Log& Log::getDefaultLogRef()
	{
		return *sm_pDefaultLog;
	}


	/**
	 * @brief	Sets the name of the default log.
	 *
	 * @param	strDefaultLogName	The name of the default log.
	 */
	void Log::setDefaultLog( const std::string strDefaultLogName )
	{
		sm_pDefaultLog = getLog( strDefaultLogName );
	}


	/**
	 * @brief	Logs a message as info if no second parameter is given.
	 * 			
	 * @note	in debug mode every message will be written to the standard output and directly to the logfile.
	 *
	 * @param	strMessage 		Message to log
	 * @param	elErrorLevel	(optional) error level.
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

		m_strLogText += getTime() + " INFO       : " + strInfo + "\n";

		std::cout << m_strLogText;
		flush();

		return true;
	}


	/**
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

		m_strLogText += getTime() + " WARNING    : " + strWarning + "\n";

		std::cout << m_strLogText;
		flush();

		return true;
	}
	

	/**
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

		m_strLogText += getTime() + " ERROR      : " + strError + "\n";

		std::cout << m_strLogText;
		flush();

		return true;
	}
	

	/**
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

		m_strLogText += getTime() + " FATAL ERROR: " + strError + "\n";

		std::cout << m_strLogText;
		flush();

		return true;
	}


	/**
	 * @brief	writes the current log to the file so that it is safe.
	 */
	void Log::flush()
	{
		std::cout.flush();

		m_fsLogFile << m_strLogText;
		m_fsLogFile.flush();

		m_strLogText.clear();
	}


	/**
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
			m_ssLogStream << getTime() + sm_astrLogLevelToPrefix[eLvl];
		}
		
		m_elLastStreamLogLvl = eLvl;
		
		return *this;
	}
	
	/**
	 * @brief	Stream operator for function pointers for the manipulators.
	 * 
     * @param	logmanipulator	function pointer to the manipulator, i.e. oocl::endl.
	 * 
     * @return	a reference to this log.
     */
	Log& Log::operator << (Log& (*logmanipulator)(Log&) ) 
	{ 
		return logmanipulator(*this);
	}
	

	/**
	 * @brief	Stream operator for inserting char arrays into the log.
	 *
     * @param	pc	pointer to a char array to insert into the log.
	 *
     * @return	a reference to this log.
     */
	Log& Log::operator << (const char* pc)
	{
		if( m_elLastStreamLogLvl <= sm_uiMaxLogLevel )
			m_ssLogStream << pc;
		return *this;
	}

	/**
	 * @brief	Stream operator for inserting strings into the log.
	 *
     * @param	str		a string to insert into the log.
	 *
     * @return	a reference to this log.
     */
	Log& Log::operator << (const std::string str)
	{
		if( m_elLastStreamLogLvl <= sm_uiMaxLogLevel )
			m_ssLogStream << str;
		return *this;
	}
	

	/**
	 * @brief	Stream operator for inserting boolean values into the log.
	 *
     * @param	b	a bool to insert into the log.
	 *
     * @return	a reference to this log.
     */
	Log& Log::operator << (const bool b)
	{
		if( m_elLastStreamLogLvl <= sm_uiMaxLogLevel )
			m_ssLogStream << b;
		return *this;
	}

	/**
	 * @brief	Stream operator for inserting floating point values into the log.
	 *
     * @param	f	a float to insert into the log.
	 *
     * @return	a reference to this log.
     */
	Log& Log::operator << (const float f)
	{
		if( m_elLastStreamLogLvl <= sm_uiMaxLogLevel )
			m_ssLogStream << f;
		return *this;
	}

	/**
	 * @brief	Stream operator for inserting double precision floats into the log.
	 *
     * @param	d	a double to insert into the log.
	 *
     * @return	a reference to this log.
     */
	Log& Log::operator << (const double d)
	{
		if( m_elLastStreamLogLvl <= sm_uiMaxLogLevel )
			m_ssLogStream << d;
		return *this;
	}
	

	/**
	 * @brief	Stream operator for inserting short integers into the log.
	 *
     * @param	i	a short int to insert into the log.
	 *
     * @return	a reference to this log.
     */
	Log& Log::operator << (const short int i)
	{
		if( m_elLastStreamLogLvl <= sm_uiMaxLogLevel )
			m_ssLogStream << i;
		return *this;
	}

	/**
	 * @brief	Stream operator for inserting short unsigned integers into the log.
	 *
     * @param	i	a short unsigned int to insert into the log.
	 *
     * @return	a reference to this log.
     */
	Log& Log::operator << (const unsigned short int i)
	{
		if( m_elLastStreamLogLvl <= sm_uiMaxLogLevel )
			m_ssLogStream << i;
		return *this;
	}

	/**
	 * @brief	Stream operator for inserting signed integers into the log.
	 *
     * @param	i	a signed integer to insert into the log.
	 *
     * @return	a reference to this log.
     */
	Log& Log::operator << (const int i)
	{
		if( m_elLastStreamLogLvl <= sm_uiMaxLogLevel )
			m_ssLogStream << i;
		return *this;
	}

	/**
	 * @brief	Stream operator for inserting unsigned integers into the log.
	 *
     * @param	i	an unsigned integer to insert into the log.
	 *
     * @return	a reference to this log.
     */
	Log& Log::operator << (const unsigned int i)
	{
		if( m_elLastStreamLogLvl <= sm_uiMaxLogLevel )
			m_ssLogStream << i;
		return *this;
	}

	/**
	 * @brief	Stream operator for inserting long (64bit) integers into the log.
	 *
     * @param	i	a long integer to insert into the log.
	 *
     * @return	a reference to this log.
     */
	Log& Log::operator << (const long long i)
	{
		if( m_elLastStreamLogLvl <= sm_uiMaxLogLevel )
			m_ssLogStream << i;
		return *this;
	}

	/**
	 * @brief	Stream operator for inserting long unsigned integers into the log.
	 *
     * @param	i	a long unsigned integers to insert into the log.
	 *
     * @return	a reference to this log.
     */
	Log& Log::operator << (const unsigned long long i)
	{
		if( m_elLastStreamLogLvl <= sm_uiMaxLogLevel )
			m_ssLogStream << i;
		return *this;
	}
	

	/**
	 * @brief	Get the current time stamp to insert into the log.
	 *
     * @return	A string containing the current time stamp.
     */
	std::string Log::getTime()
	{
		char acTime[9];
		time_t timeval = time(NULL);
		tm* pTM = gmtime( &timeval );
		strftime( acTime, 9, "%H:%M:%S", pTM );
		return std::string( acTime );
	}


	/**
	 * @brief	Inserts a new-line into the current log stream and flushes it.
	 *
	 * @param log	Affected log. As this is a manipulator, it is designed to be used without any arguments in conjunction with the "<<" operators.
	 *
	 * @return	The log.
	 */
	Log& endl(Log& log)
	{ 
		while( log.m_bFlushing );
		log.m_bFlushing = true;

		if( log.m_elLastStreamLogLvl >= log.m_elLowestLoggedLevel )
		{
			log.m_ssLogStream << '\n';
			std::cout << log.m_ssLogStream.str();
			log.m_strLogText += log.m_ssLogStream.str();

			log.m_ssLogStream.str("");
			log.m_ssLogStream.clear();

			log.flush();
		}

		log.m_bFlushing = false;
		
		return log; 
	}
}
