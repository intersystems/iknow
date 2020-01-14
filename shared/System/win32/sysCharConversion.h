//**********************************************************************
//* Copyright (c) 1999 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* sysCharConversion.h: Definition of sysCharConversion
//**********************************************************************
//* Change History:
//* PJN: 1999/12/01 Initial Implementation.
//**********************************************************************

#ifndef __FILE__
#define __FILE__ "sysCharConversion.h"
#endif

#ifndef __LINE__
#define __LINE__ 0
#endif

#ifndef _sysCharConversion_h_
#define _sysCharConversion_h_

#ifdef TRACE_INCLUDE
#pragma message("...Begin Include "__FILE__)
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4996)
#pragma warning( disable : 4100)
#endif

#include <windows.h>
#include <new>

#ifndef _sysMemory_h_
#include "sysMemory.h"
#endif

#ifndef _utlAutoArray_h_
#include "utlAutoArray.h"
#endif

#define CP_ACP                    0           // default to ANSI code page
#define CP_OEMCP                  1           // default to OEM  code page
#define CP_MACCP                  2           // default to MAC  code page
#define CP_THREAD_ACP             3           // current thread's ANSI code page
#define CP_SYMBOL                 42          // SYMBOL translations

#define CP_UTF7                   65000       // UTF-7 translation
#define CP_UTF8                   65001       // UTF-8 translation

// Note: these functions allocate memory which is the caller's responsibility
// to free.



// Convert A to W
inline WChar* a2w(const AChar* pas,SIZE_TYP paslen,int cp = CP_ACP) {

    WChar* pws = new WChar[paslen];
    if (pws) {
        if (!paslen) {

            // optimization, if conversion length is zero, just return the new buffer
            return pws;
        }
		// Windows Itanium changes size_t to 64 bit but MultiByteToWideChar API uses int for length (32 bit)
        if (::MultiByteToWideChar(cp, 0, pas, (int)paslen,pws,(int)paslen)) {
            return pws;
        }
        delete[] pws;
    }
    throw std::bad_alloc();
}

// Convert a A to X
inline XChar* a2x(const AChar* pas,SIZE_TYP paslen,int cp = CP_ACP) {

    // On WIN32 An XString is a WString, so just call WString conversion
    return a2w(pas,paslen,cp);
}

// Convert W to an A
inline AChar* w2a(const WChar* pws,SIZE_TYP pwslen,int cp = CP_ACP) {

    // First determine size of buffer required
	// Windows Itanium changes size_t to 64 bit but WideCharToMultiByte API uses int for length (32 bit)
    int t_nBuflen = ::WideCharToMultiByte (cp,0,pws,(int)pwslen,0,0,0,0);

    AChar* pas = new AChar[t_nBuflen];
    if (pas) {
        if (!pwslen) {
            // optimization, if conversion length is zero, just return the new buffer
            return pas;
        }
       // Windows Itanium changes size_t to 64 bit but WideCharToMultiByte API uses int for length (32 bit)
       if (::WideCharToMultiByte(cp,0,pws,(int)pwslen,pas,t_nBuflen,0,0)) {
            return pas;
        }
        delete[] pas;
    }
    throw std::bad_alloc();
}

// Convert an X to A
inline AChar* x2a(const XChar* pxs,SIZE_TYP pxslen,int cp = CP_ACP) {

    // On WIN32 a X IS a W
    return w2a(pxs,pxslen,cp);
}

// Convert W to X
inline XChar* w2x(const WChar* pws,SIZE_TYP pwslen,int cp = CP_ACP) {

    // On WIN32 an XString IS a WString, but MUST copy as
    // caller expects a return string that they can delete
    XChar* pxs = new XChar[pwslen];
    if (pxs) {
        sysMemCpy(pxs,pws,pwslen);
        return pxs;
    }
    throw std::bad_alloc();
}

// Convert X to W
inline WChar* x2w(const WChar* pws,SIZE_TYP pwslen,int cp = CP_ACP) {

    // On WIN2 a W IS a X
    return w2x(pws,pwslen,cp);
}

// Convert X to C16String
#ifdef WCHAR4
inline C16String x2c16Str(const XChar* pxs, size_t pxslen, int cp = CP_ACP) {

    utlAutoArray<char16_t> pws(reinterpret_cast<char16_t*>(x2w(pxs, pxslen, cp)));
    return C16String(pws.get(), pxslen);
}
#else
inline C16String x2c16Str(const XChar* pxs, size_t pxslen, int cp = CP_ACP) {

    // a X is a W
    return C16String(reinterpret_cast<const char16_t*>(pxs), pxslen);
}
#endif // WCHAR4

// Convert A to WString
inline WString a2wStr(const AChar* pas,SIZE_TYP paslen,int cp = CP_ACP) {

    WChar* pws = new WChar[paslen];
    if (pws) {
        if (!paslen) {
            delete[] pws;
            return WString();
        }
       // Windows Itanium changes size_t to 64 bit but MultiByteToWideChar API uses int for length (32 bit)
       int t_nBuflen = ::MultiByteToWideChar(cp, 0, pas, (int)paslen,pws,(int)paslen);
        if (t_nBuflen) {
            WString t_str = WString(pws,t_nBuflen);
            delete[] pws;
            return t_str;
        }
        delete[] pws;
    }
    throw std::bad_alloc();
}

// Convert A to C16String
inline C16String a2c16Str(const AChar* pas, size_t paslen, int cp = CP_ACP) {

    char16_t* pws = new char16_t[paslen];
    if (pws) {
        if (!paslen) {
            delete[] pws;
            return C16String();
        }
        int t_nBuflen = ::MultiByteToWideChar(cp, 0, pas, (int)paslen, (wchar_t*) pws, (int)paslen);
        if (t_nBuflen) {
            C16String t_str = C16String(pws, t_nBuflen);
            delete[] pws;
            return t_str;
        }
        delete[] pws;
    }
    throw std::bad_alloc();
}

// Conver W to AString
inline AString w2aStr(const WChar* pws,SIZE_TYP pwslen,int cp = CP_ACP) {

    // First determine size of buffer required
    // Windows Itanium changes size_t to 64 bit but WideCharToMultiByte API uses int for length (32 bit)
    int t_nBuflen = ::WideCharToMultiByte (cp,0,pws,(int)pwslen,0,0,0,0);

    // At most we need two multibyte characters per wide character
    AChar* pas = new AChar[t_nBuflen];
    if (pas) {
        if (!pwslen) {
            delete[] pas;
            return AString();
        }
       // Windows Itanium changes size_t to 64 bit but WideCharToMultiByte API uses int for length (32 bit)
       if (::WideCharToMultiByte(cp,0,pws,(int)pwslen,pas,t_nBuflen,0,0)) {
            AString t_str = AString(pas,t_nBuflen);
            delete[] pas;
            return t_str;
        }
        delete[] pas;
    }
    throw std::bad_alloc();
}

// Convert AString to WString
inline WString aStr2wStr(const AString& p_rstr,int cp = CP_ACP) {
    return a2wStr(p_rstr.c_str(),(SIZE_TYP)p_rstr.size(),cp);
}

// Conver WString to AString
inline AString wStr2aStr(const WString& p_rwstr, int cp = CP_ACP) {
    return w2aStr(p_rwstr.c_str(),(SIZE_TYP)p_rwstr.size(),cp);
}

// Convert W to XString
#ifdef USE_ISC_NAMESPACE
inline ISC::XString w2xStr(const WChar* pws, SIZE_TYP pwslen, int cp = CP_ACP) {

    // On WIN32 W is a X
    return ISC::XString(pws,pwslen);
}
#else
inline XString w2xStr(const WChar* pws, SIZE_TYP pwslen, int cp = CP_ACP) {

    // On WIN32 W is a X
    return XString(pws,pwslen);
}
#endif // USE_ISC_NAMESPACE

// Convert X to WString
inline WString x2wStr(const XChar* pxs, SIZE_TYP pxslen, int cp = CP_ACP) {

    // On WIN32 X is a W
    return WString(pxs,pxslen);
}

// Convert X to AString
inline AString x2aStr(const XChar* pxs, SIZE_TYP pxslen, int cp = CP_ACP) {

    // On WIN32, X is a W
    return w2aStr(pxs,pxslen,cp);
}

// Convert A to XString
#ifdef USE_ISC_NAMESPACE
inline ISC::XString a2xStr(const AChar* pas, SIZE_TYP paslen, int cp = CP_ACP) {
#else
inline XString a2xStr(const AChar* pas, SIZE_TYP paslen, int cp = CP_ACP) {
#endif // USE_ISC_NAMESPACE

    // On WIN32, X is a W
    return a2wStr(pas,paslen,cp);
}

// Convert XString to WString
#ifdef USE_ISC_NAMESPACE
inline WString xStr2wStr(const ISC::XString& p_rxstr, int cp = CP_ACP)
#else
inline WString xStr2wStr(const XString& p_rxstr, int cp = CP_ACP)
#endif // USE_ISC_NAMESPACE
{
    return x2wStr(p_rxstr.c_str(),(SIZE_TYP)p_rxstr.size(),cp);
}

// Convert XString to C16String
#ifdef USE_ISC_NAMESPACE
inline C16String xStr2c16Str(const ISC::XString& p_rxstr, int cp = CP_ACP) {
#else
inline C16String xStr2c16Str(const XString & p_rxstr, int cp = CP_ACP) {
#endif
    return x2c16Str(p_rxstr.c_str(), static_cast<size_t>(p_rxstr.size()), cp);
}

// Convert WString to XString
#ifdef USE_ISC_NAMESPACE
inline ISC::XString wStr2xStr(const WString& p_rwstr, int cp = CP_ACP)
#else
inline XString wStr2xStr(const WString& p_rwstr, int cp = CP_ACP)
#endif // USE_ISC_NAMESPACE
{
    return w2xStr(p_rwstr.c_str(),(SIZE_TYP)p_rwstr.size(),cp);
}

 // Convert C16String to XString
#ifdef USE_ISC_NAMESPACE
inline ISC::XString c16Str2xStr(const C16String& p_rwstr, int cp = CP_ACP)
#else
inline XString c16Str2xStr(const C16String& p_rwstr, int cp = CP_ACP)
#endif // USE_ISC_NAMESPACE
{
  return w2xStr(reinterpret_cast<const WChar*>(p_rwstr.c_str()),(SIZE_TYP)p_rwstr.size(),cp);
}

// Convert XString to AString
#ifdef USE_ISC_NAMESPACE
inline AString xStr2aStr(const ISC::XString& p_rxstr,int cp = CP_ACP)
#else
inline AString xStr2aStr(const XString& p_rxstr,int cp = CP_ACP)
#endif // USE_ISC_NAMESPACE
{
   return x2aStr(p_rxstr.c_str(),(SIZE_TYP)p_rxstr.size(),cp);
}

// Convert AString to XString
#ifdef USE_ISC_NAMESPACE
inline ISC::XString aStr2xStr(const AString& p_rastr,int cp = CP_ACP)
#else
inline XString aStr2xStr(const AString& p_rastr,int cp = CP_ACP)
#endif // USE_ISC_NAMESPACE
{
    return a2xStr(p_rastr.c_str(),(SIZE_TYP)p_rastr.size(),cp);
}

// Numeric conversion routines

// Convert an ansi string to an integer
inline INT a2i(const AChar* p_psz) {
    return atoi(p_psz);
}

// Convert a wide string to an integer
inline INT w2i(const WChar* p_psz) {

    utlAutoArray<AChar> t_Auto(w2a(p_psz,sysMCHARzStrLen(p_psz) + 1));

    return a2i(t_Auto.get());
}

// Convert an ansi string to a long integer
inline LONG a2l(const AChar* p_psz) {
    return atol(p_psz);
}

// Convert a wide string to a long integer
inline LONG w2l(const WChar* p_pwsz) {

    utlAutoArray<AChar> t_Auto(w2a(p_pwsz,sysMCHARzStrLen(p_pwsz) + 1));

    return a2l(t_Auto.get());
}

// Note that these functions allocate memory which must be freed by the caller

// Convert an integer to an ansi string
inline AChar* i2a(INT p_n,INT p_r = 10) {

    AChar* t_pBuffer = new AChar[20];
    if (t_pBuffer) {
        return _itoa(p_n,t_pBuffer,p_r);
    }
    return NULL;
}

// Convert an integer to a wide string
inline WChar* i2w(INT p_n, INT p_r = 10) {

    WChar* t_pBuffer = new WChar[20];
    if (t_pBuffer) {
        return _itow(p_n,t_pBuffer,p_r);
    }
    return NULL;
}

// Convert a long integer to an ansi string
inline AChar* l2a(LONG p_n,INT p_r = 10) {

    AChar* t_pBuffer = new AChar[20];
    if (t_pBuffer) {
        return _ltoa(p_n,t_pBuffer,p_r);
    }
    return NULL;
}

// Convert a long integer into an AString
inline AString l2aStr(LONG p_n, INT p_r = 10)
{
    utlAutoArray<AChar> t_l(l2a(p_n,p_r));
    if (t_l.get()) {
        return AString(t_l.get());
    } else {
        return AString();
    }
}

// Convert a long integer to a wide string
inline WChar* l2w(LONG p_n, INT p_r = 10) {

    WChar* t_pBuffer = new WChar[20];
    if (t_pBuffer) {
        return _ltow(p_n,t_pBuffer,p_r);
    }
    return NULL;
}

// Convert a long integer into an WString
inline WString l2wStr(LONG p_n, INT p_r = 10)
{
    utlAutoArray<WChar> t_l(l2w(p_n,p_r));
    if (t_l.get()) {
        return WString(t_l.get());
    }
    return WString();
}

#ifdef TRACE_INCLUDE
#pragma message("...End   Include "__FILE__)
#endif

#else

#ifdef TRACE_INCLUDE
#pragma message("...Redundant Include Attempt " __FILE__)
#endif

#endif // _sysCharConversion_h_
