#pragma once

#include <string>
#include <map>
#include <fstream>
#include <iostream>

namespace oocl
{
	/**
	 * @class	Log
	 *
	 * @brief	a simple logger class with multiple log support.
	 *
	 * @author	Jörn Teuber
	 * @date	9/14/2011
	 */
	class Log
	{
	public:
		enum EErrorLevel
		{
			EL_INFO,		// Information 
			EL_WARNING,		// Problems or errors that were catched and do not have any impact on the execution of the program
			EL_ERROR,		// Errors which could not be dealt with and likely altered the programs behaviour
			EL_FATAL_ERROR	// Errors that crashes the programm
		};

		static Log* getLog( std::string strLogName );

		bool logMessage( const std::string strMessage, EErrorLevel iErrorLevel = EL_INFO );
		void flush();

		void setLogLevel( unsigned int iLowestLoggedLevel ) { m_iLowestLoggedLevel = iLowestLoggedLevel; }

	private:
		Log( std::string strLogName );
		~Log(void);

	private:
		static std::map<std::string, Log*> sm_mapLogs;

		unsigned int m_iLowestLoggedLevel;

		std::string m_strLogText;
		std::ofstream m_fsLogFile;
	};

}

