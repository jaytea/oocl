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

#ifndef LOG_H
#define LOG_H

#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <iostream>
#include <time.h>

#include "oocl_import_export.h"

namespace oocl
{	
	/**
	 * @brief	a simple logger class with multiple log support.
	 *
	 * @author	Jörn Teuber
	 * @date	14.9.2011
	 */
	class Log
	{
		friend Log& endl(Log& log);
		
	public:
		/**
		 * @enum	EErrorLevel
		 *
		 * @brief	Values that represent the importance of the logged message (aka the error level ;) ).
		 */
		enum EErrorLevel
		{
			EL_INFO = 0,	///< Information 
			EL_WARNING,		///< Problems or errors that were catched and do not have any impact on the execution of the program
			EL_ERROR,		///< Errors which could not be dealt with and likely altered the programs behaviour
			EL_FATAL_ERROR	///< Errors that crashes the programm
		};

		static Log* getLog( std::string strLogName );
		static Log& getLogRef( std::string strLogName );
		
		static Log* getDefaultLog();
		static Log& getDefaultLogRef();

		static void setDefaultLog( const std::string strDefaultLogName );

		bool logMessage( const std::string strMessage, EErrorLevel elErrorLevel = EL_INFO );

		bool logInfo( const std::string strMessage );
		bool logWarning( const std::string strMessage );
		bool logError( const std::string strMessage );
		bool logFatalError( const std::string strMessage );
		
		
		Log& operator << (const EErrorLevel eLvl);
		Log& operator << (Log& (*logmanipulator)(Log&) );
		
		Log& operator << (const char* pc);
		Log& operator << (const std::string str);
		
		Log& operator << (const bool b);
		Log& operator << (const float f);
		Log& operator << (const double d);
		
		Log& operator << (const short int i);
		Log& operator << (const unsigned short int i);
		Log& operator << (const int i);
		Log& operator << (const unsigned int i);
		Log& operator << (const long long i);
		Log& operator << (const unsigned long long i);

		void flush();

		void setLogLevel( EErrorLevel elLowestLoggedLevel );

	private:
		Log( std::string strLogName );
		~Log(void);

		std::string getTime();

	private:
		EErrorLevel m_elLowestLoggedLevel;
		EErrorLevel m_elLastStreamLogLvl;

		std::string m_strLogText;
		std::stringstream m_ssLogStream;
		std::ofstream m_fsLogFile;

		bool m_bFlushing;
		
		
		static std::map<std::string, Log*> sm_mapLogs;
		static Log* sm_pDefaultLog;
		
		static std::string sm_astrLogLevelToPrefix[];
		static const unsigned int sm_uiMaxLogLevel = 3;
	};
	
	
	Log& endl(Log& log);

}

#endif //LOG_H
