//**********************************************************************
//* Copyright (c) 1999 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* sysCommon.h: Definition of sysCommon
//**********************************************************************
//* Change History:
//* PJN: 1999/12/01 Initial Implementation.
//**********************************************************************

#ifndef __FILE__
#define __FILE__ "sysCommon.h"
#endif

#ifndef __LINE__
#define __LINE__ 0
#endif

#ifndef _sysCommon_h_
#define _sysCommon_h_

#ifdef TRACE_INCLUDE
#pragma message("...Begin Include "__FILE__)
#endif

#if !defined(_SINGLEBYTE) && !defined(_DOUBLEBYTE)
#define _DOUBLEBYTE     // Doublebyte is defined by default
#endif

#include <assert.h>

#ifndef _INC_TCHAR
#include "tchar.h"
#endif

#include <sysTypes.h>

// Common types
typedef sysTypes::UINT8     BYTE;           // unsigned 8-bit quantity
typedef sysTypes::UINT16    WORD;           // unsigned 16-bit quantity
typedef sysTypes::UINT16    USHORT;         // unsigned 16-bit quantity
typedef sysTypes::UINT32    DWORD;          // unsigned 32-bit quantity
typedef sysTypes::UINT64    ULONGLONG;      // unsigned 64-bit quantity
typedef sysTypes::INT       INT;            // signed platform integer
typedef sysTypes::INT8      SBYTE;          // signed 8-bit quantity
typedef sysTypes::INT16     SWORD;          // signed 16-bit quantity
typedef sysTypes::INT16     SHORT;          // signed 16-bit quantity
typedef sysTypes::INT32     SDWORD;         // signed 32-bit quantity
typedef sysTypes::INT64     LONGLONG;       // signed 64-bit quantity
typedef sysTypes::LONG      LONG;           // signed CPU register-size quantity
typedef sysTypes::ULONG     ULONG;          // unsigned CPU register-size quantity
typedef sysTypes::IEEE32    FLOAT;          // Floating point number
typedef sysTypes::UINTPTR   UINTPTR;        // Pointer sized unsigned integer

//typedef sysTypes::IEEE64  DOUBLE;         // Double precision float
// use sysTypes::DOUBLE instead from now on

typedef sysTypes::SIZE_TYP  SIZE_TYP;       // Magnitude
typedef sysTypes::BOOL      BOOL;           // Truth value
typedef sysTypes::UINT      UINT;           // Unsigned int

// Character types
typedef sysTypes::AChar     AChar;          // Ansi character
typedef sysTypes::UAChar    UAChar;         // Unsigned Ansi character
typedef sysTypes::WChar     WChar;          // Wide character
typedef sysTypes::UWChar    UWChar;         // Unsigned wide character
typedef sysTypes::XChar     XChar;          // XML Character (16 bits)
#ifdef UNICODE
typedef sysTypes::WChar     TChar;          // TChar as a wide character
#else
typedef sysTypes::AChar     TChar;          // TChar as a narrow character
#endif

// String types
typedef sysTypes::ASTRING   AString;        // std::basic_string<char>
typedef sysTypes::UASTRING  UAString;       // std::basic_string<unsigned char>
typedef sysTypes::WSTRING   WString;        // std::basic_string<WChar>
typedef sysTypes::C16STRING C16String;      // std::basic_string<char16_t>
#ifdef UNICODE
typedef sysTypes::WSTRING   TString;        // TString as a wide string
#else
typedef sysTypes::ASTRING   TString;        // TString as a narrow string
#endif

typedef sysTypes::ERRORCODE         sysErrorCode;         // Error code type
typedef sysTypes::CRITICALSECTION   sysCriticalSection;   // Critical section
typedef sysTypes::SEMAPHORE         sysSemaphore;         // Semaphore type
typedef sysTypes::CONDITIONVARIABLE sysConditionVariable; // Condition Variable

// Required to avoid clash with XALAN XSLT XString typedef
#ifdef USE_ISC_NAMESPACE
namespace ISC {
#endif // USE_ISC_NAMESPACE

typedef sysTypes::XSTRING   XString;        // std::basic_string<XChar>

#ifdef USE_ISC_NAMESPACE
};
#endif // USE_ISC_NAMESPACE

// Common type manipulators
inline WORD sysLOWORD(DWORD p_dwVal) {
    return static_cast<WORD>(p_dwVal);
}

inline WORD sysHIWORD(DWORD p_dwVal) {
    return static_cast<WORD>(p_dwVal >> 16);
}

inline BYTE sysLOBYTE(WORD p_wVal) {
    return static_cast<BYTE>(p_wVal);
}

inline BYTE sysHIBYTE(WORD p_wVal) {
    return static_cast<BYTE>(p_wVal >> 8);
}

template <typename _Type>
inline _Type sysRoundUpToMultiple(_Type Value, _Type Boundary)
{
    return ((Value + (Boundary - 1)) / Boundary) * Boundary;
}

// Synchronization primitives
inline SDWORD sysAtomicInc(SDWORD* p_pl);
inline SDWORD sysAtomicDec(SDWORD* p_pl);
inline SDWORD sysAtomicAdd(SDWORD* p_p1, SDWORD p_p2);
inline SDWORD sysAtomicSwap(SDWORD* p_p1, SDWORD p_p2);

// Debugging support
#if defined(_DEBUG) || defined(DEBUG)   // If compiler in debug mode
#define M_DEBUG                         // set M debug switch

// Ensure that if one of _DEBUG or DEBUG is defined, so is the other.
#if !defined(_DEBUG)
#define _DEBUG
#elif !defined(DEBUG)
#define DEBUG
#endif

#endif // _DEBUG or DEBUG

#if defined(__GNUC__) && ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4) || (__GNUC__ > 3))
// Seems that gcc-3.3 has this, but g++3.3 doesn't -- so we only use it for gcc/g++ 3.4 or better
#define UN_USED __attribute__((unused))
#else
#define UN_USED
#endif

#define NYI     assert(0)
#define SYNTAX  assert(0)

#ifdef M_DEBUG
#if defined(__APPLE__)
// On the Mac, we redefine M_ASSERT not to call assert() because this simply
// calls abort(), which isn't very useful when working with a debug build!
void APPLE_Assert(const char* file, const int line, const char* expr);
#define M_ASSERT(x) if (!(x)) { APPLE_Assert(__FILE__, __LINE__, #x); }
#else
#define M_ASSERT(x) assert(x)
#endif
#define MDBG(x)     x

// In debug mode if we have run time type information on we can dynamic_cast
#ifdef _CPPRTTI
#define SAFE_CAST   dynamic_cast
#else
#define SAFE_CAST   static_cast
#endif  //_CPPRTTI

#else

#define M_ASSERT(x)
#define MDBG(x)     x
#define SAFE_CAST   static_cast

#endif // M_DEBUG

#define NODEFAULT M_ASSERT(0)

#ifndef FALSE
#define FALSE 0
#else
#if FALSE != 0
#error "False must have value 0"
#endif
#endif

#ifndef TRUE
#define TRUE 1
#else
#if TRUE != 1
#error "True must have value 1"
#endif
#endif

#ifndef NULL
#define NULL 0
#else
#if NULL != 0
#error "Null must have value 0"
#endif
#endif

// For parameter list declarations
#ifndef IN
#define IN
#endif // IN

#ifndef OUT
#define OUT
#endif // OUT

#ifndef IN_OUT
#define IN_OUT
#endif // IN_OUT

// Computes the number of entries in an array
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)   (sizeof(x)/sizeof((x)[0]))
#endif

// Values for bit flags
enum BIT_VAL { OFF, ON };


#ifdef _DOUBLEBYTE
#ifndef NO_UCHAR_TYPE
typedef WChar   Char;                       // Unit of memory for a character
typedef UWChar  UChar;                 // Unit of memory for an unsigned character
typedef WString String;                     // Wide stl string
#endif //NO_UCHAR_TYPE


#else // _DOUBLEBYTE

typedef AChar   Char; // Unit of memory for a character
#ifndef NO_UCHAR_TYPE
typedef UAChar  UChar;                      // Unit of memory for an unsigned character
typedef AString String;                     // Ansi stl string
#endif //NO_UCHAR_TYPE

#endif  // _DOUBLEBYTE

// Macros for inline functions
#ifndef M_INLINE
#ifdef M_DEBUG
#define M_INLINE
#else
#define M_INLINE inline
#endif
#endif


// *******************************
// Locale-related Definitions
// *******************************
#ifndef LOCALE_NEUTRAL
#define LOCALE_NEUTRAL 0
#endif

// *******************************
// System-specific implementations
// *******************************

#include "sysCommon.inl"

#ifdef TRACE_INCLUDE
#pragma message("...End   Include "__FILE__)
#endif

#else

#ifdef TRACE_INCLUDE
#pragma message("...Redundant Include Attempt " __FILE__)
#endif

#endif // _sysCommon_h_
