//**********************************************************************
//* Copyright (c) 1999 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* sysMemory.h: Definition of sysMemory
//**********************************************************************
//* Change History:
//* PJN: 1999/12/01 Initial Implementation.
//* PJN: 2004/04/21 Migrate certain functions to templates
//**********************************************************************

#ifndef __FILE__
#define __FILE__ "sysMemory.h"
#endif

#ifndef __LINE__
#define __LINE__ 0
#endif

#ifndef _sysMemory_h_
#define _sysMemory_h_

#ifdef TRACE_INCLUDE
#pragma message("...Begin Include "__FILE__)
#endif

#ifndef _sysCommon_h_
#include "sysCommon.h"
#endif

#ifdef _DOUBLEBYTE
#include <wctype.h>
#if (__GNUG__==3)
#include <ctype.h>
#endif
#else
#include <ctype.h>
#endif

#include <stdlib.h>
#include <limits.h>

#ifndef _NEW_
#include <new> // for std::bad_alloc
#endif

// *****************************
// System-independent prototypes
// *****************************

// Raw memory manipulation routines

// Allocate, Free and Reallocate memory
inline void* sysAlloc(SIZE_TYP size);
inline void  sysFree(void *memblock);
inline void* sysRealloc(void *memblock, SIZE_TYP size);

// sysMemSet - fill memory with value
template <typename IscType>
#if defined(DECUNIX) || defined(__VMS)
void sysMemSet(IscType* const p_pTo,IscType p_IscType,SIZE_TYP p_nCount)
#else
inline void sysMemSet(IscType* const p_pTo,IscType p_IscType,SIZE_TYP p_nCount)
#endif
{
    SIZE_TYP t_nCount;
    for (t_nCount = 0; t_nCount < p_nCount; t_nCount++) {
        p_pTo[t_nCount]= p_IscType;
    }
}

// sysMemCpy - copy with no overlap
template <typename IscType>
inline void sysMemCpy(IscType* const p_pTo,const IscType* const p_pFrom, SIZE_TYP p_nCount) {
    ::memcpy(p_pTo,p_pFrom,p_nCount * sizeof(IscType));
}

// sysMemCmp - compare memory
template <typename IscType>
inline int sysMemCmp(const IscType* const p_pLHS,const IscType* const p_pRHS,SIZE_TYP p_nCount) {

    // Results :-

    // -1 - pLHS < pwRHS
    // 0  - both strings are equal
    // 1  - pLHS > pwRHS

    SIZE_TYP t_nCount;
    int t_nResult = 0;
    for (t_nCount = 0; t_nCount < p_nCount ; t_nCount++) {
        if (p_pLHS[t_nCount] < p_pRHS[t_nCount]) {
            t_nResult = -1;
            break;
        } else if (p_pLHS[t_nCount] > p_pRHS[t_nCount]) {
            t_nResult = 1;
            break;
        }
    }
    return t_nResult;
}


// sysMemMove - Move for when you don't know if you are coming or going
template <typename IscType>
inline void sysMemMove(IscType* const p_pTo, const IscType* const p_pFrom, SIZE_TYP p_nCount) {
    ::memmove(p_pTo,p_pFrom,p_nCount * sizeof(IscType));
}

// sysMemMoveLeft - Move with overlap to the left
template <typename IscType>
inline void sysMemMoveLeft(IscType* const p_pTo, const IscType* const p_pFrom, SIZE_TYP p_nCount) {
    sysMemMove(p_pTo,p_pFrom,p_nCount);
}

// sysMemMoveRight - Move with overlap to the right
template <typename IscType>
inline void sysMemMoveRight(IscType* const p_pTo, const IscType* const p_pFrom, SIZE_TYP p_nCount) {
    sysMemMove(p_pTo,p_pFrom,p_nCount);
}

// character manipulation routines

// sysMCHARzStrLen - length of zero terminated string
template <typename IscType>
inline SIZE_TYP sysMCHARzStrLen(const IscType* const p_psz)
{
    SIZE_TYP t_n=0; while(p_psz[t_n++]) { }; return --t_n;
}

// sysMCHARzStrChr - Scan for a character in a null terminated string
template <typename IscType>
inline const IscType* sysMCHARzStrChr(const IscType* t_pStr,IscType c)
{
    while (*t_pStr != '\0' && *t_pStr != c) {
        t_pStr++;
    }
    return ((*t_pStr == c) ? t_pStr : NULL );
}

// sysMemClear - clear memory
inline void sysMemClear(void* pvTo, SIZE_TYP stCountBytes);

// **********************************
// System-independent implementations
// **********************************

template <typename IscType>
inline IscType* sysMCHARtoCHARcpy(const IscType* const pTo, const IscType* const pFrom,SIZE_TYP len)
{
    while (--len > 0) {
        *pTo++ = static_cast<IscType>(*pFrom++);
    }
    return pTo;
}

// sysMCHARcmp - compare M strings same length
template <typename IscType>
inline int sysMCHARcmp(const IscType* const pmstrLHS, const IscType* const pmstrRHS, SIZE_TYP stCountMc) {

    // Force to BYTE* for fast comparison
    return sysMemCmp(reinterpret_cast<const BYTE* const>(pmstrLHS), reinterpret_cast<const BYTE* const>(pmstrRHS), stCountMc * sizeof(IscType));
}

// compare M strings different length
// - LHS<RHS => -1
// - LHS=RHS => 0
// - LHS>RHS => +1
template <typename IscType>
inline int sysMCHARcmp(const IscType* const pmstrLHS, SIZE_TYP stCountLHS, const IscType* const pmstrRHS, SIZE_TYP stCountRHS) {

    // if the LHS is shorter than or equal to the right-hand side
    if (stCountLHS <= stCountRHS) {

        // compare up to the LHS length
        int cmp = sysMemCmp(reinterpret_cast<const BYTE*>(pmstrLHS),reinterpret_cast<const BYTE*>(pmstrRHS), stCountLHS * sizeof(IscType));

        // if they weren't equal ..
        if (cmp != 0) {
            return (cmp < 0) ? (-1) : (1);
        }

        // .. they were equal ..
        else {

            // return result based on the lengths
            return (stCountLHS < stCountRHS) ? (-1) : (0);
        }
    }

    // .. LHS is longer than the RHS ..
    else {

        // compare up to the RHS length
        int cmp = sysMemCmp(reinterpret_cast<const BYTE*>(pmstrLHS),reinterpret_cast<const BYTE*>(pmstrRHS), stCountRHS * sizeof(IscType));

        // if they weren't equal ..
        if (cmp != 0) {
            return (cmp < 0) ? (-1) : (1);
        }

        // return result based on the lengths
        return +1;
    }
}
// Locate first occurrence of null terminated substring within a null terminated string
template <typename IscType>
inline const IscType* sysMCHARzStr(const IscType* p_pStr,const IscType* p_pStrSub)
{
    if (*p_pStrSub == '\0') {
        return p_pStr;
    }
    
    SIZE_TYP t_nSubLen = sysMCHARzStrLen(p_pStrSub);
    for (; (p_pStr = sysMCHARzStrChr(p_pStr,*p_pStrSub)) != NULL ; p_pStr++) {
        if (0 == sysMCHARcmp(p_pStr,p_pStrSub,t_nSubLen)) {
            return p_pStr;
        }
    }
    return NULL;
}

// return whether the given strings are equal up to the given lengths
template<typename IscType>
inline BOOL sysMCHARisEqual(const IscType* const pmstrLHS, SIZE_TYP stCountLHS, const IscType* const pmstrRHS, SIZE_TYP stCountRHS)
{
    // (this is an optimization)
    // if the lengths are different ..
    if (stCountLHS != stCountRHS) {
        return 0; // can't be equal
    }

    // delegate
    return sysMCHARisEqual(pmstrLHS,pmstrRHS,stCountRHS);
}

// return whether the given strings are equal up to the given common length
template<typename IscType>
inline BOOL sysMCHARisEqual(const IscType* const pmstrLHS, const IscType* const pmstrRHS, SIZE_TYP stCountMc)
{
    // delegate to sysMemCmp ( force to BYTE* to take advantage of specialization )
    return sysMCHARcmp(pmstrLHS,pmstrRHS,stCountMc) == 0;
}

// return whether the given character is upper-case
// note carefully: assumes A..Z is contiguous, ignores international character set issues
inline BOOL sysCharIsUpper(WChar p_cChar) {
    return iswupper(static_cast<WChar>(p_cChar));
}


// return whether the given character is lower-case
// note carefully: assumes a..z is contiguous, ignores international character set issues
inline BOOL sysCharIsLower(WChar p_cChar) {
    return iswlower(static_cast<WChar>(p_cChar));
}

// return whether the given character is a digit
inline BOOL sysCharIsDigit(WChar p_cChar) {
    return iswdigit(static_cast<WChar>(p_cChar));
}

// return the given character folded to upper case (or unchanged if it was not lower case to start with)
// note carefully assumes A..Z and a..z are contiguous, ignores international character set issues
inline BOOL sysCharToUpper(WChar p_cChar) {
    return towupper(static_cast<WChar>(p_cChar));
}

// return the given character folded to lower case (or unchanged if it was not upper case to start with)
// note carefully assumes A..Z and a..z are contiguous, ignores international character set issues
inline BOOL sysCharToLower(WChar p_cChar) {
    return towlower(static_cast<WChar>(p_cChar));
}


// return whether the given strings are equal, ignoring letter case
// note carefully assumes A..Z and a..z are contiguous, ignores international character set issues
inline BOOL sysMCHARisEqualIgnoreCase(const WChar* pmstrLHS,SIZE_TYP stCountLHS, const WChar* pmstrRHS, SIZE_TYP stCountRHS) {

    // if the counts are not equal ..
    if (stCountLHS != stCountRHS) {
        return FALSE;
    }

    // .. the counts are equal ..
    else {

        for (unsigned int t_i = 0; t_i < stCountLHS; ++t_i) {
            if (sysCharToUpper(pmstrLHS[t_i]) != sysCharToUpper(pmstrRHS[t_i])) {
                return FALSE;
            }
        }
        return TRUE;
    }
}

// convert digit to hex ascii
inline WChar DigitToHexAscii(int p_nDigit, bool p_bLowerCase) {
    if (p_nDigit < 10) {
        return (WChar) ('0' + p_nDigit);
    }
    else {
        return (WChar)((p_bLowerCase?'a':'A') + (p_nDigit - 10));
    }
}


// convert number to hex ascii
// convert p_n, writing result to p_s
// if p_nWidth is 0 the minimum required width is used
// .. otherwise the number is right-justified within the given width
// .. if it is too large for the given width the width is simply ignored
// if p_bLeadingZeroes is true then zeroes are used instead of spaces when p_nWidth exceeds the number's natural width
// if p_bLowerCase is true then lower-case hex digits are used for digits > 10
inline void NumberToHexAscii(ULONG p_n, WString& p_s, USHORT p_uWidth = 0, bool p_bLeadingZeroes = false, bool p_bLowerCase = false) {

    // - perform the basic conversion -

    WString t_s;

    // if the number is zero ..
    if (0 == p_n) {
        t_s = '0';
    }

    // .. the number isn't zero ..
    else {
        ULONG t_n = p_n;
        while (t_n != 0) {
            t_s.insert(WString::size_type(0), WString::size_type(1), DigitToHexAscii(static_cast<int>(t_n % 16), p_bLowerCase));
            t_n /= 16;
        }
    }

    // - pad to width if appropriate -

    // if the width is not zero ..
    if (p_uWidth != 0) {

        // if the string is smaller than the width ..
        if (t_s.length() < p_uWidth) {

            // select padding character
            const WChar p_cPad = (p_bLeadingZeroes?'0':' ');

            // pad
            t_s.insert(WString::size_type(0), WString::size_type(p_uWidth - t_s.length()), WChar(p_cPad));
        }
    }

    // set the result
    p_s += t_s;
}



// convert signed number to decimal ascii
// convert p_n, writing result to p_s
// if p_nWidth is 0 the minimum required width is used
// .. otherwise the number is right-justified within the given width
// .. if it is too large for the given width the width is simply ignored
// if p_bLeadingZeroes is true then zeroes are used instead of spaces when p_nWidth exceeds the number's natural width
// if the number is positive no sign is added, if the number is negative a "-" is put on the front
inline void NumberToDecimalAscii(LONG p_n, WString& p_s, USHORT p_uWidth = 0, bool p_bLeadingZeroes = false) {

    // - perform the basic conversion -

    WString t_s;

    // if the number is zero ..
    if (0 == p_n) {
        t_s = '0';
    }

    // .. the number isn't zero ..
    else {

        LONG t_LONG_MIN;
#ifdef LONG_MIN
        t_LONG_MIN = LONG_MIN;
#else
        switch (sizeof(p_n)) {
        case 4: t_LONG_MIN = -2147483648L; break;
        case 8: t_LONG_MIN = _LONG64_MIN; break;
        default: throw 0x8000FFFFL /* COM_E_UNEXPECTED */;
        }
#endif

        // convert the absolute value of the number to decimal ascii
        ULONG t_ul = labs(p_n);
        while (t_ul != 0UL) {
            const WChar t_cDigit = '0' + static_cast<WChar>(t_ul % 10UL);
            // - the static cast is required to avoid a warning with VC7
            //   (the warning of loss of data in the conversion is over-zealous in this case because a%b cannot exceed b-1)
            t_s.insert(WString::size_type(0), WString::size_type(1), t_cDigit);
            t_ul /= 10UL;
        }

        // if it's negative ..
        if (p_n<0 || t_LONG_MIN==p_n) {

            // put a '-' on the front
            t_s.insert(WString::size_type(0), WString::size_type(1), '-');
        }
    }

    // - pad to width if appropriate -

    // if the width is not zero ..
    if (p_uWidth != 0) {

        // if the string is smaller than the width ..
        if (t_s.length() < p_uWidth) {

            // select padding character
            const WChar p_cPad = (p_bLeadingZeroes?'0':' ');

            // pad
            t_s.insert(WString::size_type(0), WString::size_type(p_uWidth - t_s.length()), WChar(p_cPad));
        }
    }

    // set the result
    p_s += t_s;
}



// ***********************************************
// System-specific implementations/specializations
// ***********************************************
#include "sysMemory.inl"

// These follow the include because they use specializations of templates previously defined

// return whether the given strings are equal, ignoring letter case
// note carefully assumes A..Z and a..z are contiguous, ignores international character set issues
inline BOOL sysMCHARzIsEqualIgnoreCase(const WChar* pmstrLHS,const WChar* pmstrRHS) {
    return sysMCHARisEqualIgnoreCase(pmstrLHS,sysMCHARzStrLen(pmstrLHS),pmstrRHS,sysMCHARzStrLen(pmstrRHS));
}

// sysMemClear - clear memory
inline void sysMemClear(void* pvTo, SIZE_TYP stCountBytes)
{
    sysMemSet(static_cast<BYTE*>(pvTo), static_cast<BYTE>(0), stCountBytes);
}

// Convenience function for comparing zero terminated strings
template <typename IscType>
inline int sysMCHARzCmp(const IscType* const pzLHS,const IscType* const pzRHS) {
    return sysMCHARcmp(pzLHS, sysMCHARzStrLen(pzLHS), pzRHS, sysMCHARzStrLen(pzRHS));
}

template <typename IscType>
inline IscType* sysMCHARzStrCat(IscType* pDst,IscType pSrc) {

    SIZE_TYP nLenDst = sysMCHARzStrLen(pDst);
    SIZE_TYP nLenSrc = sysMCHARzStrLen(pSrc);

    sysMemCpy(pDst + nLenDst,pSrc,nLenSrc);
    pDst[nLenDst + nLenSrc + 1] = '\0';

    return pDst;
}

// sysMCHARzDup - duplicate a zero terminated string
template<typename IscType>
inline IscType* sysMCHARzDup(const IscType* const p_pStrFrom) {

    SIZE_TYP t_nLen = sysMCHARzStrLen(p_pStrFrom) + 1;

    IscType* t_pStrTo = new IscType[t_nLen];
    if (0 == t_pStrTo) {
        return 0;
    }

    sysMemCpy(t_pStrTo,p_pStrFrom,t_nLen);

    return t_pStrTo;
}


// avoid conversion to a more-strictly-aligned type
#define ALIGNDATA(org_,destvar_) \
    UINTPTR destvar_; \
    unsigned char* t_ptrALIGNDATA = reinterpret_cast<unsigned char*>(&destvar_); \
    unsigned char* t_pOrgALIGNDATA = reinterpret_cast<unsigned char*>(&(org_)); \
    for (int t_iALIGNDATA = 0; t_iALIGNDATA < sizeof(t_ptrALIGNDATA); ++t_iALIGNDATA) { \
        t_ptrALIGNDATA[t_iALIGNDATA] = static_cast<unsigned char>(t_pOrgALIGNDATA[t_iALIGNDATA]); \
    }



#ifdef TRACE_INCLUDE
#pragma message("...End   Include "__FILE__)
#endif

#else

#ifdef TRACE_INCLUDE
#pragma message("...Redundant Include Attempt " __FILE__)
#endif

#endif // _sysMemory_h_

