//**********************************************************************
//* Copyright (c) 1999 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* sysTypes.h: Definition of sysTypes
//**********************************************************************
//* Change History:
//* PJN: 1999/12/01 Initial Implementation.
//**********************************************************************

#ifndef __FILE__
#define __FILE__ "sysTypes.h"
#endif

#ifndef __LINE__
#define __LINE__ 0
#endif

#ifndef _sysTypes_h_
#define _sysTypes_h_

#ifdef TRACE_INCLUDE
#pragma message("...Begin Include "__FILE__)
#endif

#ifdef __APPLE__
#pragma GCC visibility push(default)
#endif

#include <string>
#include <wchar.h>

#ifdef __APPLE__
#pragma GCC visibility pop
#endif

#ifdef __VMS
#include <ints>
#endif

#ifdef __alpha
#if defined(_REENTRANT)
#define ____WAS_REENTRANT 1
#else
// This must be defined for pthreads.h to be included on DEC Unix
#define _REENTRANT
#define ____WAS_REENTRANT 0
#endif
#endif
#include <pthread.h>
#ifdef __alpha
#if !____WAS_REENTRANT
#undef _REENTRANT
#endif
#endif
#include <errno.h>
#include <semaphore.h>
#ifdef __sun
#include <widec.h>
#endif

/* ***************************** Compatibility ***************************** */

#define _snprintf     snprintf
#define _vsnprintf    vsnprintf
#define _stricmp      strcasecmp
#define wcsicmp       wscasecmp

#if BIT64PLAT || defined(DECUNIX) || defined(SPARC)
// Pointers are 64 bit
#define PTR8
#endif

// HP Platform has problems with std::ctype<>
#ifdef HP
#define WCHAR4
#endif

// Solaris has crashes at process exit (prodlog 63786)
#ifdef __sun
#define WCHAR4
#endif

#ifdef __APPLE__
#define WCHAR4
#endif


class sysTypes {

public:
    // Types
#ifndef WCHAR4
    typedef unsigned short              WChar;      // Wide character
    typedef unsigned short              UWChar;     // Unsigned Wide char
#else
    typedef wchar_t                     WChar;      // Wide character
    typedef wchar_t                     UWChar;     // Unsigned Wide char
#endif

    typedef unsigned short              XChar;	    // 2-byte wide character
    typedef unsigned short              UXChar;     // unsigned 2-byte wide character

    typedef char                        AChar;      // Ansi char
    typedef unsigned char               UAChar;     // Unsigned Ansi char

    typedef char                        INT8;       // 8-bit signed integer
    typedef short                       INT16;      // 16-bit signed integer
    typedef int                         INT32;      // 32-bit signed integer
    typedef long long int               INT64;      // 64-bit signed integer
    typedef unsigned char               UINT8;      // 8-bit unsigned integer
    typedef unsigned short              UINT16;     // 16-bit unsigned integer
    typedef unsigned int                UINT32;     // 32-bit unsigned integer
    typedef unsigned long long int      UINT64;     // 64-bit unsigned integer

#ifdef __OBJC__
    // Objective-C requires a signed char for BOOL compatibility
    typedef signed char                 BOOL;       // Boolean value
#else
    typedef int                         BOOL;       // Boolean value
#endif

    typedef size_t                      SIZE_TYP;

    typedef int                         INT;        // Integer
    typedef unsigned int                UINT;       // Unsigned integer

    typedef std::basic_string<AChar>    ASTRING;    // Stl ansi string
    typedef std::basic_string<UAChar>   UASTRING;   // Stl ansi string (unsigned characters)
    typedef std::basic_string<WChar>    WSTRING;    // Stl wide string
    typedef std::basic_string<char16_t> C16STRING;
    typedef std::basic_string<XChar>	XSTRING;    // XML String (2 byte unicode)

    // Synonyms
    typedef long                        LONG;       // register size signed integer
    typedef unsigned long               ULONG;      // register size unsigned integer

    typedef float                       IEEE32;     // IEEE32 floating point number
    typedef double                      IEEE64;     // IEEE64 floating point number

    typedef IEEE64                      DOUBLE;     // what used to be declared as ::DOUBLE

    // The size of UINTPTR must be the same as a pointer for the platform
#ifdef PTR8
    typedef UINT64                      UINTPTR;    // Pointer sized unsigned integer
#else
    typedef UINT32                      UINTPTR;    // Pointer sized unsigned integer
#endif

    typedef int                         ERRORCODE;         // Error type
    typedef pthread_mutex_t             CRITICALSECTION;   // Critical section
    typedef sem_t                       SEMAPHORE;         // Semaphore
    typedef pthread_cond_t              CONDITIONVARIABLE; // Condition Variable
};

#if (__GNUG__ == 3) && (__GNUC_MINOR__ < 4)
namespace std
{
    template<>
    struct char_traits<unsigned short>
    {
        typedef unsigned short     char_type;
        typedef wint_t              int_type;
        typedef streamoff           off_type;
        typedef wstreampos          pos_type;
        typedef mbstate_t           state_type;

        /**
        * Assign one character to another
        */
        static void assign(char_type& __c1, const char_type& __c2)
        { __c1 = __c2; }

        /**
        * Compare one character with another for equality
        */
        static bool eq(const char_type& __c1, const char_type& __c2)
        { return __c1 == __c2; }

        /**
        * Compare characters to see if one is less than the other
        */
        static bool lt(const char_type& __c1, const char_type& __c2)
        { return __c1 < __c2; }

        /**
        * Compare characters
        */
        static int
            compare(const char_type* __s1, const char_type* __s2, size_t __n)
        {
            int result = 0;

            for (int i = 0; i < __n && result == 0; ++i)
            {
                if (__s1[i] != __s2[i])
                {
                    if (__s1[i] < __s2[i]) result = -1;
                    else if (__s1[i] > __s2[i]) result = 1;
                }
                else if (__s1[i] == (unsigned short) L'\0')
                {
                    i = __n;  // Force-terminate the loop.
                }

            }
            return result;

        }

        /**
        * Determine the length of a UCS-16 string
        */
        static size_t
            length(const char_type* __s)
        {
            size_t i = 0;
            if (__s != NULL)
            {
                for (i = 0; __s[i] != (unsigned short) L'\0'; ++i);
            }

            return i;
        }

        /**
        * Search for a specific character within the string
        */
        static const char_type*
            find(const char_type* __s, size_t __n, const char_type& __a)
        {
            const char_type* result = NULL;
            if (__s != NULL)
            {
                for (int i = 0;
                    i < __n && __s[i] != (unsigned short) L'\0' && result == NULL;
                    ++i)
                {
                    if (__s[i] == __a) result = __s + i;
                }
            }

            return result;
        }

        /**
        * Move characters
        */
        static char_type*
            move(char_type* __s1, const char_type* __s2, int_type __n)
        {
            return (char_type*) memmove(__s1, __s2, __n * sizeof(unsigned short));
        }

        /**
        * Copy characters
        */
        static char_type*
            copy(char_type* __s1, const char_type* __s2, size_t __n)
        {
            return (char_type*) memcpy(__s1, __s2, __n * sizeof(unsigned short));
        }

        /**
        * Assign characters to a string
        */
        static char_type*
            assign(char_type* __s, size_t __n, char_type __a)
        {
            if (__s != NULL)
            {
                for (int i = 0; i < __n && __s[i] != (unsigned short) L'\0'; ++i)
                {
                    __s[i] = __a;
                }
            }

            return __s;
        }

        /**
        * Determine the type of a character
        */
        static char_type
            to_char_type(const int_type& __c) { return char_type(__c); }

        /**
        * Determine the int type of a character
        */
        static int_type
            to_int_type(const char_type& __c) { return int_type(__c); }

        /**
        * Compare two int types
        */
        static bool
            eq_int_type(const int_type& __c1, const int_type& __c2)
            { return __c1 == __c2; }

        /**
        * Retrieve the int type of EOF
        */
        static int_type
        eof() { return static_cast<int_type>(0xFFFF); }

        /**
        * Look for a non-EOF type
        */
        static int_type
            not_eof(const int_type& __c)
        { return eq_int_type(__c, eof()) ? 0 : __c; }
    };
}
#endif

#ifdef TRACE_INCLUDE
#pragma message("...End   Include "__FILE__)
#endif

#else

#ifdef TRACE_INCLUDE
#pragma message("...Redundant Include Attempt " __FILE__)
#endif

#endif // _sysTypes_h_
