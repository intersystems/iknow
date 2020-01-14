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

#include <string>

#ifndef _WINDOWS_
#include <windows.h>
#endif

#ifdef _WIN64
// Pointers are 64 bit
#define PTR8
//#define BASETYPES
#endif

class CPosixConditionVariable;

class sysTypes {

public:
    // Types
    typedef wchar_t             	    WChar;      // Wide character
    typedef wchar_t              	    UWChar;     // Unsigned Wide char

    typedef wchar_t              	    XChar;	    // 2-byte wide character
    typedef wchar_t              	    UXChar;     // unsinged 2-byte wide character

    typedef char                        AChar;      // Ansi char
    typedef unsigned char               UAChar;     // Unsigned Ansi char

    typedef signed char                 INT8;       // 8-bit signed integer
    typedef short                       INT16;      // 16-bit signed integer
    typedef long                        INT32;      // 32-bit signed integer
    typedef __int64                     INT64;      // 64-bit signed integer
    typedef unsigned char               UINT8;      // 8-bit unsigned integer
    typedef unsigned short              UINT16;     // 16-bit unsigned integer
    typedef unsigned long               UINT32;     // 32-bit unsigned integer
    typedef unsigned __int64            UINT64;     // 64-bit unsigned integer

    typedef int                         BOOL;       // Boolean value
    typedef size_t						SIZE_TYP;   // Magnitude

    typedef int                         INT;        // Integer
    typedef unsigned int                UINT;       // Unsigned Integer

    typedef std::basic_string<AChar>    ASTRING;    // Stl ansi string
    typedef std::basic_string<UAChar>   UASTRING;   // Stl ansi string (unsigned characters)
    typedef std::basic_string<WChar>    WSTRING;    // Stl wide string
    typedef std::basic_string<char16_t> C16STRING;
    typedef std::basic_string<XChar>    XSTRING;    // XML String

#ifdef PTR8
    typedef UINT64                      UINTPTR;    // Pointer sized unsigned integer
#else
    typedef UINT32                      UINTPTR;    // Pointer sized unsigned integer
#endif

    // Synonyms
    typedef INT32                       LONG;       // 32-bit signed integer
    typedef UINT32                      ULONG;      // 32-bit unsigned integer

    typedef float                       IEEE32;     // IEEE32 floating point number
    typedef double                      IEEE64;     // IEEE64 floating point number

    typedef IEEE64                      DOUBLE;     // what used to be declared as ::DOUBLE

    typedef DWORD                       ERRORCODE;         // Error type
    typedef CRITICAL_SECTION            CRITICALSECTION;   // Critical section
    typedef HANDLE                      SEMAPHORE;         // Semaphore
    typedef CPosixConditionVariable     CONDITIONVARIABLE; // Condition Variable

    // NOTE: The CPosixConditionVariable class is implemented in sysCommon.inl
};


#ifdef TRACE_INCLUDE
#pragma message("...End   Include "__FILE__)
#endif

#else

#ifdef TRACE_INCLUDE
#pragma message("...Redundant Include Attempt " __FILE__)
#endif

#endif // _sysTypes_h_
