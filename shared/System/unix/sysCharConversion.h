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

#include <new>
#include <stdio.h>
#include <stdlib.h>

#ifndef _sysCommon_h_
#include "sysCommon.h"
#endif

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

// Wide to Ansi / Ansi to Wide conversions

// Convert A to W
inline WChar* a2w(const AChar* pas, size_t paslen,int = 0 /* cp */) {

#ifdef WCHAR4
    WChar* pws = new WChar[paslen];
    size_t lenleft;
    int numconverted;
    WChar* pwsc = pws;
    if (pws) {
        if (!paslen) {
            return pws;
        }
        lenleft = paslen;
        while (lenleft) {
            numconverted =  mbtowc((wchar_t*)pwsc, pas, lenleft);
            if (numconverted == -1) {
                delete[] pws;
                throw std::bad_alloc();
            }
            if (numconverted == 0) {
                // pwsc must point to zero
                numconverted = 1;
                *pwsc = 0;
                break;
            }
            lenleft -= numconverted;
            pas += numconverted;
            pwsc++;
        }
        return pws;
    }
    throw std::bad_alloc();
#else
  WChar* pws = new WChar[paslen];
  if (pws) {
       size_t t_i;
       for (t_i = 0; t_i < paslen; ++t_i) {
           pws[t_i] = WChar(pas[t_i] & 0xFF); // Mask to prevent sign extension
       }
       return pws;
  }
  throw std::bad_alloc();
#endif
}


// Convert a A to X
#ifdef WCHAR4
inline XChar* a2x(const AChar* pas,size_t paslen,int /* cp */) {

  XChar* pxs = new XChar[paslen];
  if (pxs) {
       size_t t_i;
       for (t_i = 0; t_i < paslen; ++t_i) {
     pxs[t_i] = XChar(pas[t_i] & 0xFF);       // Mask to prevent sign extension
       }
       return pxs;
  }
  throw std::bad_alloc();
}
#else
inline XChar* a2x(const AChar* pas,size_t paslen,int cp = CP_ACP) {

    // An X is a W , so just call existing conversion
    return a2w(pas,paslen,cp);
}
#endif // WCHAR4

// Convert wide string to ansi string
inline AChar* w2a(const WChar* pws, size_t pwslen,int = 0 /* cp */) {

#ifdef WCHAR4
    // At most we need MB_CUR_MAX multibyte characters per wide character

    // compute size of required buffer
    size_t lenleft;
    int numconverted;
    size_t bufsiz = 0;
    AChar atemp[6]; // this should be big enough, we hope
    const WChar *pwc;

    lenleft = pwslen;
    pwc = pws;
    while (lenleft--) {
        numconverted = wctomb(atemp, *pwc);
        if (numconverted == -1) {
            throw std::bad_alloc();
        }
        if (numconverted == 0)
            break;
        bufsiz += numconverted;
        pwc++;
    }

    AChar* pas = new AChar[bufsiz];
    AChar* pacs = pas;
    if (pas) {
        if (!pwslen) {
            return pas;
        }
        lenleft = pwslen;
        pwc = pws;
        while (lenleft--) {
            numconverted = wctomb(pacs, *pwc);
            if (numconverted == -1) {
                delete[] pas;
                throw std::bad_alloc();
            }
            if (numconverted == 0)
                break;
            pacs += numconverted;
            pwc++;

        }
        return pas;

    }
    throw std::bad_alloc();
#else
    AChar* pas = new AChar[pwslen];
    if (pas) {
       size_t t_i;
       for (t_i = 0; t_i < pwslen; ++t_i) {
           pas[t_i] = AChar(pws[t_i] & 0xFF);
       }
       return pas;
    }
    throw std::bad_alloc();
#endif
}


// Convert a X to A
#ifdef WCHAR4
inline AChar* x2a(const XChar* pxs,size_t pxslen,int cp = CP_ACP) {

  AChar* pas = new AChar[pxslen];
  if (pas) {
       size_t t_i;
       for (t_i = 0; t_i < pxslen; ++t_i) {
     pas[t_i] = AChar(pxs[t_i] & 0xFF);
       }
       return pas;
  }
  throw std::bad_alloc();
}
#else
inline AChar* x2a(const XChar* pxs,size_t pxslen,int cp = CP_ACP) {

    // an X IS a W so just call existing conversion
    return w2a(pxs,pxslen,cp);
}
#endif // WCHAR4

// Convert W to X
#ifdef WCHAR4
inline XChar* w2x(const WChar* pws, size_t pwslen, int cp = CP_ACP) {

    XChar* pxs = new XChar[pwslen];
    if (pxs) {
        size_t t_i;
        for (t_i = 0; t_i < pwslen; ++t_i) {
            pxs[t_i] = XChar(pws[t_i] & 0xFFFF);
        }
        return pxs;
    }
    throw std::bad_alloc();
}
#else
inline XChar* w2x(const WChar* pws,size_t pwslen, int cp = CP_ACP) {

    // an X IS a W, but MUST copy as
    // caller expects a return string that they can delete
    XChar* pxs = new XChar[pwslen];
    if (pxs) {
        sysMemCpy(pxs,pws,pwslen);
        return pxs;
    }
    throw std::bad_alloc();
}
#endif // WCHAR4

// Convert X to W
#ifdef WCHAR4
inline WChar* x2w(const XChar* pxs, size_t pxslen, int cp = CP_ACP) {

  WChar* pws = new WChar[pxslen];
  if (pws) {
    size_t t_i;
    for (t_i = 0; t_i < pxslen; ++t_i) {
      pws[t_i] = WChar(pxs[t_i]);
    }
    return pws;
  }
  throw std::bad_alloc();
}
#else
inline WChar* x2w(const WChar* pws,size_t pwslen,int cp = CP_ACP) {

    // a W IS a X
    return w2x(pws,pwslen,cp);
}
#endif // WCHAR4

// Convert W to XString
#ifdef USE_ISC_NAMESPACE
#ifdef WCHAR4
inline ISC::XString w2xStr(const WChar* pws,size_t pwslen,int cp = CP_ACP) {

    utlAutoArray<XChar> pxs(w2x(pws,pwslen,cp));
    return ISC::XString(pxs.get(),pwslen);
}
#else
inline ISC::XString w2xStr(const WChar* pws, size_t pwslen, int cp = CP_ACP) {

    // a W is a X
    return ISC::XString(pws,pwslen);
}
#endif // WCHAR4
#else
#ifdef WCHAR4
inline XString w2xStr(const WChar* pws,size_t pwslen,int cp = CP_ACP) {

    utlAutoArray<XChar> pxs(w2x(pws,pwslen,cp));
    return XString(pxs.get(),pwslen);
}
#else
inline XString w2xStr(const WChar* pws, size_t pwslen, int cp = CP_ACP) {

    // a W is a X
    return XString(pws,pwslen);
}
#endif // WCHAR4
#endif // USE_ISC_NAMESPACE

// Convert X to WString
#ifdef WCHAR4
inline WString x2wStr(const XChar* pxs, size_t pxslen, int cp = CP_ACP) {

    utlAutoArray<WChar> pws(x2w(pxs,pxslen,cp));
    return WString(pws.get(),pxslen);
}
#else
inline WString x2wStr(const XChar* pxs, size_t pxslen, int cp = CP_ACP) {

    // a X is a W
    return WString(pxs,pxslen);
}
#endif // WCHAR4

// Convert X to C16String
#ifdef WCHAR4
inline C16String x2c16Str(const XChar* pxs, size_t pxslen, int cp = CP_ACP) {

  utlAutoArray<char16_t> pws(reinterpret_cast<char16_t*>(x2w(pxs,pxslen,cp)));
    return C16String(pws.get(),pxslen);
}
#else
inline C16String x2c16Str(const XChar* pxs, size_t pxslen, int cp = CP_ACP) {

    // a X is a W
  return C16String(reinterpret_cast<const char16_t*>(pxs),pxslen);
}
#endif // WCHAR4

inline WString a2wStr(const AChar* pas,size_t paslen,int cp = CP_ACP) {

    utlAutoArray<WChar> t_Auto(a2w(pas,paslen,cp));

    return WString(t_Auto.get(),paslen);
}

inline C16String a2c16Str(const AChar* pas,size_t paslen,int cp = CP_ACP) {

  utlAutoArray<char16_t> t_Auto(reinterpret_cast<char16_t*>(a2w(pas,paslen,cp)));

    return C16String(reinterpret_cast<const char16_t*>(t_Auto.get()),paslen);
}
inline AString w2aStr(const WChar* pws,size_t pwslen,int cp = CP_ACP) {

    utlAutoArray<AChar> t_Auto(w2a(pws,pwslen,cp));

    return AString(t_Auto.get(),pwslen);
}

// Convert X to AString
#ifdef WCHAR4
inline AString x2aStr(const XChar* pxs, size_t pxslen, int cp = CP_ACP) {

  utlAutoArray<AChar> t_Auto(x2a(pxs,pxslen,cp));

  return AString(t_Auto.get(),pxslen);
}
#else
inline AString x2aStr(const XChar* pxs, size_t pxslen, int cp = CP_ACP) {

    // a X is a W
    return w2aStr(pxs,pxslen,cp);
}
#endif // WCHAR4

// Convert A to XString
#ifdef USE_ISC_NAMESPACE
#ifdef WCHAR4
inline ISC::XString a2xStr(const AChar* pas,size_t paslen, int cp = CP_ACP) {

  utlAutoArray<XChar> t_Auto(a2x(pas,paslen,cp));

  return ISC::XString(t_Auto.get(),paslen);
}
#else
inline ISC::XString a2xStr(const AChar* pas, size_t paslen, int cp = CP_ACP) {

    // a X is a W
    return a2wStr(pas,paslen,cp);
}
#endif // WCHAR4
#else
#ifdef WCHAR4
inline XString a2xStr(const AChar* pas,size_t paslen, int cp = CP_ACP) {

  utlAutoArray<XChar> t_Auto(a2x(pas,paslen,cp));

  return XString(t_Auto.get(),paslen);
}
#else
inline XString a2xStr(const AChar* pas, size_t paslen, int cp = CP_ACP) {

    // a X is a W
    return a2wStr(pas,paslen,cp);
}
#endif // WCHAR4
#endif // USE_ISC_NAMESPACE

inline WString aStr2wStr(const AString& p_rstr,int cp = CP_ACP) {
    return a2wStr(p_rstr.c_str(),static_cast<size_t>(p_rstr.size()),cp);
}

inline AString wStr2aStr(const WString& p_rwstr,int cp = CP_ACP) {
    return w2aStr(p_rwstr.c_str(),static_cast<size_t>(p_rwstr.size()),cp);
}

// Convert XString to WString
#ifdef USE_ISC_NAMESPACE
inline WString xStr2wStr(const ISC::XString& p_rxstr, int cp = CP_ACP) {
#else
inline WString xStr2wStr(const XString& p_rxstr, int cp = CP_ACP) {
#endif
  return x2wStr(p_rxstr.c_str(),static_cast<size_t>(p_rxstr.size()),cp);
}

 // Convert XString to C16String
#ifdef USE_ISC_NAMESPACE
inline C16String xStr2c16Str(const ISC::XString& p_rxstr, int cp = CP_ACP) {
#else
inline C16String xStr2c16Str(const XString& p_rxstr, int cp = CP_ACP) {
#endif
  return x2c16Str(p_rxstr.c_str(),static_cast<size_t>(p_rxstr.size()),cp);
}

// Convert WString to XString
#ifdef USE_ISC_NAMESPACE
inline ISC::XString wStr2xStr(const WString& p_rwstr, int cp = CP_ACP) {
#else
inline XString wStr2xStr(const WString& p_rwstr, int cp = CP_ACP) {
#endif
  return w2xStr(p_rwstr.c_str(),static_cast<size_t>(p_rwstr.size()),cp);
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
inline AString xStr2aStr(const ISC::XString& p_rxstr,int cp = CP_ACP) {
#else
inline AString xStr2aStr(const XString& p_rxstr,int cp = CP_ACP) {
#endif
  return x2aStr(p_rxstr.c_str(),static_cast<size_t>(p_rxstr.size()),cp);
}

// Convert AString to XString
#ifdef USE_ISC_NAMESPACE
inline ISC::XString aStr2xStr(const AString& p_rastr,int cp = CP_ACP) {
#else
inline XString aStr2xStr(const AString& p_rastr,int cp = CP_ACP) {
#endif
  return a2xStr(p_rastr.c_str(),static_cast<size_t>(p_rastr.size()),cp);
}

// Numeric conversion routines

// Convert an ansi string to an integer
inline INT a2i(const AChar* p_psz) {
    return atoi(p_psz);
}

// Convert a wide string to an integer
inline INT w2i(const WChar* p_psz) {

    utlAutoArray<AChar> t_Auto(w2a(p_psz,static_cast<size_t>(sysMCHARzStrLen(p_psz)) + 1));

    return a2i(t_Auto.get());
}

// Convert an ansi string to a long integer
inline LONG a2l(const AChar* p_psz) {
    return atol(p_psz);
}

// Convert a wide string to a long integer
inline LONG w2l(const WChar* p_pwsz) {

    utlAutoArray<AChar> t_Auto(w2a(p_pwsz,static_cast<size_t>(sysMCHARzStrLen(p_pwsz)) + 1));

    return a2l(t_Auto.get());
}

// Note that these functions allocate memory which must be freed by the caller

// Convert an integer to an ansi string
inline AChar* i2a(INT p_n,INT p_r UN_USED = 10) {

    // ! no itoa on unix !

    AChar* t_pBuffer = new AChar[20];
    if (! t_pBuffer) {
		return NULL;
	}

	switch (p_r) {
	case 10:
        sprintf(t_pBuffer,"%d",p_n);
		break;
	case 16:
		sprintf(t_pBuffer,"%X",p_n);
		break;
	default:
		M_ASSERT(!"i2a: only bases 10 and 16 are supported on unix platforms");
		return NULL;
	}

    return t_pBuffer;
}

// Convert an integer to a wide string
inline WChar* i2w(INT p_n, INT p_r = 10) {

    utlAutoArray<AChar> t_Auto(i2a(p_n,p_r));

    if (t_Auto.get()) {
        return a2w(t_Auto.get(),static_cast<size_t>(sysMCHARzStrLen(t_Auto.get()))+1);
    }
    return NULL;
}

// Convert a long integer to an ansi string
inline AChar* l2a(LONG p_n,INT p_r = 10) {

    // ! no ltoa on unix !

    AChar* t_pBuffer = new AChar[20];
    if (! t_pBuffer) {
		return NULL;
	}

	switch (p_r) {
	case 10:
        sprintf(t_pBuffer,"%li",p_n);
		break;
	case 16:
		sprintf(t_pBuffer,"%lX",p_n);
		break;
	default:
		M_ASSERT(!"l2a: only bases 10 and 16 are supported on unix platforms");
		return NULL;
	}

    return t_pBuffer;
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

    utlAutoArray<AChar> t_Auto(l2a(p_n,p_r));

    if (t_Auto.get()) {
        return a2w(t_Auto.get(),sysMCHARzStrLen(t_Auto.get())+1);
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
