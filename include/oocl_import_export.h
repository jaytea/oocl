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
/// This file was mainly copied from http://www.c-plusplus.de/forum/245780 and modified by Jörn Teuber


// @see http://gcc.gnu.org/wiki/Visibility
// @see http://people.redhat.com/drepper/dsohowto.pdf
// @see http://www.eventhelix.com/RealtimeMantra/HeaderFileIncludePatterns.htm



// Generic helper definitions for shared library support
#if defined(_WIN32) || defined(__CYGWIN__)
#    if defined(_MSC_VER)   // Windows && MS Visual C
#        if _MSC_VER < 1310    //Version < 7.1?
#            pragma message ("Compiling with a Visual C compiler version < 7.1 (2003) has not been tested!")
#        endif // Version > 7.1
#        define MYLIB_HELPER_DLL_IMPORT __declspec(dllimport)
#        define MYLIB_HELPER_DLL_EXPORT __declspec(dllexport)
#        define MYLIB_HELPER_DLL_LOCAL
#    elif defined (__GNUC__)
#        define MYLIB_HELPER_DLL_IMPORT __attribute__((dllimport))
#        define MYLIB_HELPER_DLL_EXPORT __attribute__((dllexport))
#        define MYLIB_HELPER_DLL_LOCAL
#    endif
#    define BOOST_ALL_NO_LIB  //disable the msvc automatic boost-lib selection in order to link against the static libs!
#elif defined(__linux__) || defined(linux) || defined(__linux)
#    if __GNUC__ >= 4    // TODO Makefile: add -fvisibility=hidden to compiler parameter in Linux version
#        define MYLIB_HELPER_DLL_IMPORT __attribute__ ((visibility("default")))
#        define MYLIB_HELPER_DLL_EXPORT __attribute__ ((visibility("default")))
#        define MYLIB_HELPER_DLL_LOCAL  __attribute__ ((visibility("hidden")))
#    else
#        define MYLIB_HELPER_DLL_IMPORT
#        define MYLIB_HELPER_DLL_EXPORT
#        define MYLIB_HELPER_DLL_LOCAL
#    endif
#endif

// Now we use the generic helper definitions above to define MYLIB_API and MYLIB_LOCAL.
// MYLIB_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// MYLIB_LOCAL is used for non-api symbols.

#ifndef MYLIB_DLL
#define MYLIB_DLL
#endif
// TODO Makefile: add MYLIB_DLL and EXPORTS
#ifdef MYLIB_DLL // defined if MYLIB is compiled as a DLL
#    ifdef OOCL_EXPORT // defined if we are building the MYLIB DLL (instead of using it)
#        define OOCL_EXPORTIMPORT MYLIB_HELPER_DLL_EXPORT
#		 define EXPIMP_TEMPLATE
#    else
#        define OOCL_EXPORTIMPORT MYLIB_HELPER_DLL_IMPORT
#		 define EXPIMP_TEMPLATE extern
#    endif // MYLIB_DLL_EXPORTS
#define DLL_LOCAL MYLIB_HELPER_DLL_LOCAL
#else // MYLIB_DLL is not defined: this means MYLIB is a static lib.
#	 define DLL_PUBLIC
#    define DLL_LOCAL
#endif // MYLIB_DLL



// disable some nasty MSVC Warnings
#ifdef _MSC_VER   // Windows && MS Visual C
//#        pragma warning( disable : 4996 )     // disable deprecation warnings
//#        pragma warning( disable : 4275 )    // non &#8211; DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
#        pragma warning( disable : 4251 )    // like warning above but for templates (like std::string)
#endif 

/*#if defined(_WIN32)
#	ifdef OOCL_EXPORT
#   	define OOCL_EXPORTIMPORT __declspec(dllexport)
#		define EXPIMP_TEMPLATE
#	else
#   	define OOCL_EXPORTIMPORT __declspec(dllimport)
#		define EXPIMP_TEMPLATE extern
#	endif
#else
#   define OOCL_EXPORTIMPORT
#	define EXPIMP_TEMPLATE
#endif*/
