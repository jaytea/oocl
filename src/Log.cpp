#include "Log.h"


namespace oocl
{
	///< The sm map logs
	std::map<std::string, Log*> Log::sm_mapLogs;

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
		m_iLowestLoggedLevel( 0 )
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
	 * @fn	bool Log::logMessage( const std::string strMessage, int iErrorLevel = 0 )
	 *
	 * @brief	Logs a message.
	 * 			
	 * @note	in debug mode after every message will be written to the log and it will be flushed.
	 * 			In release mode only messages with error level "Warning" or higher will be logged and the user has to flush the log himself
	 *
	 * @author	Jörn Teuber
	 * @date	11/25/2011
	 *
	 * @param	strMessage 	Message to log
	 * @param	iErrorLevel	(optional) error level.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */

	bool Log::logMessage( const std::string strMessage, EErrorLevel iErrorLevel )
	{
		if( iErrorLevel < m_iLowestLoggedLevel )
			return false;

		switch(iErrorLevel)
		{
		case EL_INFO:			m_strLogText += "INFO: "; break;
		case EL_WARNING:		m_strLogText += "WARNING: "; break;
		case EL_ERROR:			m_strLogText += "ERROR:	"; break;
		case EL_FATAL_ERROR:	m_strLogText += "FATAL ERROR: "; break;
		}

		m_strLogText += strMessage;
		m_strLogText += "\n";

#ifdef _DEBUG
		flush();
		std::cout << strMessage << std::endl;
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

}