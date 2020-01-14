//**********************************************************************
//* Copyright (c) 2000 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* utlCacheList.h: Definition of LIST interface
//**********************************************************************
//* Change History:
//* Gang Wang       8/10/18     Removed exception specification (GWA002)
//* Jamie Newton    10/18/05    Added support for type CT_DOUBLE
//* Jamie Newton    10/3/01     Added support for XString
//* Mark Hanson:    1/28/00     Extensively reworked the interface to make it STL like
//* Dave McCaldon:  6/14/99     Ported to C++
//* Scott Jones:    3/19/99     Totally rewrote to fix major bugs, add error
//*                             checking, add correct handling of conversions between string
//*                             and numeric types, increase the speed (2-100 times), and
//*                             add support for BigDecimals
//*                             added support for run-time check of whether running on
//*                             JDK 1.1
//* Dave McCaldon:  3/27/98     Misc. changes and fixes
//* Dave McCaldon:  2/02/98     Return null for invalid Date's in getDate()
//* Dave McCaldon:  11/05/97    Initial Implementation.
//**********************************************************************

#ifndef __FILE__
#define __FILE__ "utlCacheList.h"
#endif

#ifndef __LINE__
#define __LINE__ 0
#endif

#ifndef _utlCacheList_h_
#define _utlCacheList_h_

#ifdef TRACE_INCLUDE
#pragma message("...Begin Include "__FILE__)
#endif

#if defined(_WIN32) || defined (_WIN64)
#pragma warning(disable:4290) // c++ exception specification ignored ( that's ok )
#pragma warning(disable:4018) // signed/unsigned mismatch ( these are ok here )
#endif

#ifndef _sysCommon_h_
#include "sysCommon.h"
#endif

// Only include the COM stuff where we need it, C++ binding does not want it for the moment
#ifndef CACHE_COM_DISABLE

#ifndef _sysCom_h_
#include "sysCom.h"
#endif

#endif  // CACHE_COM_DISABLE

#ifndef _utlCountedByteArray_h_
#include "utlCountedByteArray.h"
#endif

#include <exception>
#include <stdexcept>      // std::out_of_range

#ifdef WIN32
#pragma warning(disable: 4200) // Zero size array
#endif

// Null class for adding null elements to the list
class CacheNull {};

// Forward declarations
class CacheListItem;
class CacheListIterator;
class CacheList;

class CacheROList
{
    // Friends
public:
    friend class CacheListItem;
    friend class CacheListIterator;
    friend class CacheList;

    // Enums
public:
    enum CacheTypes {
            CT_UNDEF        = -1,
            CT_PLACEHOLDER  = 0,
            CT_ASCII        = 1,
            CT_UNICODE      = 2,
            CT_XUNICODE     = 3,
            CT_POSINT       = 4,
            CT_NEGINT       = 5,
            CT_POSREAL      = 6,
            CT_NEGREAL      = 7,
            CT_DOUBLE       = 8
    };

    // Constructors
public:
    CacheROList();
    CacheROList(const BYTE* p_pData, size_t p_iLen);
    CacheROList(const CountedByteArray& p_rData);
    CacheROList(const CacheList& p_rList);
    virtual ~CacheROList() {}

    // Operators
public:
    CacheListItem operator[](size_t nIndex); /* Exception specification is deprecated in C++11 */
    CacheROList& operator=(const CacheList& p_rList);
    CacheROList& operator=(const CacheROList& p_rList);
    CacheROList& operator=(const CountedByteArray& p_rList);

public:
    CacheList operator+(const CacheList& rhs);
    CacheList operator+(const CacheROList& rhs);
    CacheList operator+(const CountedByteArray& rhs);
    CacheList operator+(const AString& rhs);
    CacheList operator+(const WString& rhs);
    CacheList operator+(const C16String& rhs);  
    CacheList operator+(const CacheNull& rhs);
    CacheList operator+(const LONG& rhs);
    CacheList operator+(const size_t& rhs);
#ifndef LONG_EQUAL_LONGLONG
    CacheList operator+(const LONGLONG& rhs);
#endif
    CacheList operator+(const sysTypes::IEEE64& rhs);
    CacheList operator+(const int& rhs);

#ifdef WCHAR4
#ifdef USE_ISC_NAMESPACE
    CacheList operator+(const ISC::XString& rhs);
#else
    CacheList operator+(const XString& rhs);
#endif // USE_ISC_NAMESPACE
#endif

  //VisualAge C++ on AIX is not WCHAR4, so we don't use wchar_t for ISC strings.
  //It has a two byte wchar_t as a built in type.
  //Because ICU uses this as its character type, and the iKnow engine uses the ICU character
  //type (and utlCacheList!) throughout, it's necessary to support basic_string<wchar_t>.
  //On Windows we use wchar_t for ISC strings, but WCHAR4 is not
  //defined (wchar_t there is two byte, as on AIX). We can't include an additional overload
  //there because it will have already been defined above.
#if !defined(WCHAR4) && !defined(_WIN32)
  CacheList operator+(const std::basic_string<wchar_t>& rhs);
#endif

    // Accessors
public:
    BYTE*                   data()                  const;
    size_t                     datalength()            const;
    const CountedByteArray  AsCountedByteArray()    const;
    size_t                     size();
    CacheListIterator       begin()                 const;
    CacheListIterator       end()                   const;

// Manipulators
public:
    void Reset();

private:
    void    internalInit();
    size_t     countItems();

protected:
    BYTE*   m_pbData;
    size_t     m_iLength;
    size_t     m_iLastIndex;
    size_t     m_iLastOffset;
    size_t     m_iCount;
    BOOL    m_bCounted;

// Statics
private:
    static sysTypes::IEEE64 parsedblb(BYTE* dat, size_t off, size_t len);
    static sysTypes::IEEE64 parsedblu(BYTE* dat, size_t off, size_t len);
    static int parsenumb(BYTE* dat, size_t off, size_t len, LONGLONG& retnum);
    static int parsefracb(BYTE* dat, size_t off, size_t end, LONGLONG dbl, int dscale, BOOL sign, LONGLONG& retnum);
    static int parseexpb(BYTE* dat, size_t off, size_t end, LONGLONG dbl, int dscale, BOOL sign, LONGLONG& retnum);
    static int parsefracu(BYTE* dat, size_t off, size_t end, LONGLONG dbl, int dscale, BOOL sign, LONGLONG& retnum);
    static int parseexpu(BYTE* dat, size_t off, size_t end, LONGLONG dbl, int dscale, BOOL sign, LONGLONG& retnum);
    static int parsenumu(BYTE* dat, size_t off, size_t len, LONGLONG& retnum);
    static int scale(LONGLONG value, int scale, BOOL sign, LONGLONG& retnum);

    static int              dblint(int scale, LONGLONG num);
    static LONGLONG         dbllong(int scale, LONGLONG num);
    static AString          dblstr(int scale, LONGLONG num);
    static sysTypes::IEEE64 dbl2dbl(int scale, LONGLONG num);

    // Type 8 support
    static int              ieee2int(BYTE* pData,size_t p_iLen);
    static LONGLONG         ieee2long(BYTE* pData,size_t p_iLen);
    static AString          ieee2str(BYTE* pData,size_t p_iLen);
    static sysTypes::IEEE32 ieee2flt(BYTE* pData,size_t p_iLen);
    static sysTypes::IEEE64 ieee2dbl(BYTE* pData,size_t p_iLen);

    static AString lngstr(LONGLONG num);

protected:
    static int s_rgnNeedArray[];
    static LONGLONG s_rgscaletab[];
    static char s_rgtenstab[];
    static char s_rgonestab[];
    static char s_rglonglongmin[];
};

class CacheList : public CacheROList
{
// Friends
public:
    friend class CacheListItem;
    friend class CacheListIterator;
    friend class CacheROList;

// Constructors
public:
    CacheList();
    CacheList(const CacheROList& p_rList);
    CacheList(const CacheList& p_rList);
    CacheList(const BYTE* p_pData, size_t p_iLen);
    CacheList(const CountedByteArray& p_rData);
    CacheList(size_t cbInitial);
    virtual ~CacheList();

// Operations
public:
    void appendList(const CacheList& p_rList);
    void appendList(const CacheROList& p_rList);
    void appendAChars(const AChar* p_pData, size_t p_iLen);
    void appendXChars(const XChar* p_pData, size_t p_iLen);
    void appendXStringWithCompress(const XChar* p_pData, size_t p_iLen);
    //An alternate version that uses a supplied buffer for ASCII narrowing (if needed) instead of
    //allocating its own.
    void appendXStringWithCompress(const XChar* p_pData, size_t p_iLen, AChar* buf);
    void appendStringWithCompress(const WChar* p_pData, size_t p_iLen);
    void appendStringWithCompress(const AChar* p_pData, size_t p_iLen);
    void appendStringWithCompress(const AString &p_str);
    void appendStringWithCompress(const WString &p_str);
    void appendStringWithCompress(const C16String &p_str);
    void appendDoubleString(const AString &p_str);
    void appendDoubleString(const WString &p_str);
    void appendEmptyItem();
    void clear();
    void appendIEEE32(sysTypes::IEEE32);
    void appendIEEE64(sysTypes::IEEE64);
    void appendPtr(void* pv);

// Operators
public:
    CacheList& operator=(const CacheList& p_rList);
    CacheList& operator=(const CacheROList& p_rList);
    CacheList& operator=(const CountedByteArray& p_rList);

    const CacheListIterator insert(const CacheListIterator &p_Iterator, const CacheList& p_rList);
    const CacheListIterator insert(const CacheListIterator &p_Iterator, const CacheROList& p_rList);
    const CacheListIterator insert(const CacheListIterator &p_Iterator, const CountedByteArray& p_rData);
    const CacheListIterator insert(const CacheListIterator &p_Iterator, const AString& p_rStr);
    const CacheListIterator insert(const CacheListIterator &p_Iterator, const WString& p_rStr);
    const CacheListIterator insert(const CacheListIterator &p_Iterator, const LONG p_iVal);
#ifndef LONG_EQUAL_LONGLONG
    const CacheListIterator insert(const CacheListIterator &p_Iterator, const LONGLONG p_lVal);
#endif
    const CacheListIterator insert(const CacheListIterator &p_Iterator, const sysTypes::IEEE64 p_fVal);
    const CacheListIterator insert(const CacheListIterator &p_Iterator, const int p_iVal);
    const CacheListIterator insert(const CacheListIterator &p_Iterator, const CacheNull& p_rNull);
    //const CacheListIterator insert(const CacheListIterator &p_Iterator, const time_t p_tVal);

#ifdef WCHAR4
#ifdef USE_ISC_NAMESPACE
    const CacheListIterator insert(const CacheListIterator &p_Iterator, const ISC::XString& p_rStr);
#else
    const CacheListIterator insert(const CacheListIterator &p_Iterator, const XString& p_rStr);
#endif // USE_ISC_NAMESPACE
#endif

  //See comment on operator+ above.
#if !defined(WCHAR4) && !defined(_WIN32)
  const CacheListIterator insert(const CacheListIterator &p_Iterator, const std::basic_string<wchar_t>& p_rStr);
#endif

    CacheList& operator+=(const CacheList& rhs);
    CacheList& operator+=(const CacheROList& rhs);
    CacheList& operator+=(const CountedByteArray& rhs);
    CacheList& operator+=(const AString& rhs);
    CacheList& operator+=(const UAString& rhs);
    CacheList& operator+=(const WString& rhs);
    CacheList& operator+=(const C16String& rhs);
    CacheList& operator+=(const CacheNull& rhs);
    CacheList& operator+=(const LONG& rhs);
    CacheList& operator+=(const size_t& rhs);
#ifndef LONG_EQUAL_LONGLONG
    CacheList& operator+=(const LONGLONG& rhs);
#endif
    CacheList& operator+=(const sysTypes::IEEE64& rhs);
    CacheList& operator+=(const int& rhs);
    CacheList& operator+=(const char rhs);

#ifdef WCHAR4
#ifdef USE_ISC_NAMESPACE
    CacheList& operator+=(const ISC::XString& rhs);
#else
    CacheList& operator+=(const XString& rhs);
#endif // USE_ISC_NAMESPACE
#endif

  //See comment on operator+ above.
#if !defined(WCHAR4) && !defined(_WIN32)
  CacheList& operator+=(const std::basic_string<wchar_t>& rhs);
#endif

    const CacheListIterator erase(const CacheListIterator &p_Iterator);

protected:
    void setList(const BYTE* p_pData, size_t p_iLen);
    virtual void ensureBuffer(size_t cbNeeded);
    size_t  makeItem(size_t p_iOffset, size_t p_iLen);

    void stuffAscii(size_t p_iItem, const AString& s);
    void stuffAscii(size_t p_iItem, const AChar* p_psa,size_t p_nLen);

#ifdef USE_ISC_NAMESPACE
    void stuffUnicode(size_t p_iItem, const ISC::XString& s);
    void stuffUnicode(size_t p_iItem, const XChar* p_psx, size_t p_nLen);
    void stuffUnicode(size_t p_iItem, const ISC::C16String& s);
#else
    void stuffUnicode(size_t p_iItem, const XString& s);
    void stuffUnicode(size_t p_iItem, const XChar* p_psx, size_t p_nLen);
    void stuffUnicode(size_t p_iItem, const C16String& s);
#endif // USE_ISC_NAMESPACE

    void stuffByte(size_t p_iItem, const BYTE* p_pData, size_t p_iLen);
    void stuffNull(size_t p_iItem);
    void stuffNegInt(size_t p_iItem, int value);
    void stuffPosInt(size_t p_iItem, int value);
    void stuffPosLong(size_t p_iItem, LONGLONG value);
    void stuffNegLong(size_t p_iItem, LONGLONG value);
    void stuffPosNum(size_t p_iItem, int scale, LONGLONG value);
    void stuffNegNum(size_t p_iItem, int scale, LONGLONG value);
    void stuffDouble(size_t p_iItem, sysTypes::IEEE64 value);
    void stuffDouble(size_t p_iItem, const AString& s);
    void stuffIEEE32(size_t p_iItem,sysTypes::IEEE32 value);
    void stuffIEEE64(size_t p_iItem,sysTypes::IEEE64 value);

protected:
    size_t  m_cbData;
};

class CacheListItem
{
// Friends
public:
    friend class CacheListIterator;

  //Constructor, points at a byte into list
  CacheListItem(BYTE* rgbyData) : m_rgbyData(rgbyData) {}
// Accessors
    CacheList::CacheTypes   Type()  const;

    sysTypes::IEEE64        AsDouble()      const;
    LONGLONG    AsLongLong()    const;
    LONG        AsLong()        const;
    AString     AsAString()     const;
    UAString    AsUAString()    const;
    WString     AsWString()     const;
    C16String   AsC16String()   const;
    BOOL        AsBool()        const;
    CacheList   AsCacheList()   const;
    CacheROList AsCacheROList() const;
    LONG        AsDateTime()    const;
    void*       AsPtr()         const;

#ifdef _DOUBLEBYTE
    WString     AsString()      const { return AsWString(); }
#else
    AString     AsString()      const { return AsAString(); }
#endif

#ifdef USE_ISC_NAMESPACE
    ISC::XString    AsXString() const;
#else
    XString         AsXString() const;
#endif // USE_ISC_NAMESPACE

#ifndef CACHE_COM_DISABLE
    COM_BSTR    AsBSTR()        const;
#endif  // CACHE_COM_DISABLE

    CountedByteArray    AsCountedByteArray()    const   { return CountedByteArray(data(), datalength()); }

private:
    const BYTE* data()          const;  // The data within the item
    size_t        datalength()    const;  // Size of the data
    size_t         size()          const;  // Total size of the item including the overhead
    CacheList::CacheTypes   getType(BYTE* & p_pData, size_t& p_iLen) const;    // Gets the type, pointer and size in one go

    AString     getAscii(const BYTE* p_pData, const size_t p_iLen)     const;
    UAString    getUAscii(const BYTE* p_pData, const size_t p_iLen)     const;
    WString     getUnicode(const BYTE* p_pData, const size_t p_iLen)       const;
    C16String   getC16Unicode(const BYTE* p_pData, const size_t p_iLen)       const;
    LONGLONG    getPosLongLong(const BYTE* p_pData, const size_t p_iLen)   const;
    LONGLONG    getNegLongLong(const BYTE* p_pData, const size_t p_iLen)   const;

#ifdef USE_ISC_NAMESPACE
    ISC::XString getXString(const BYTE* p_pData,const size_t p_iLen) const;
#else
    XString     getXString(const BYTE* p_pData,const size_t p_iLen) const;
#endif // USE_ISC_NAMESPACE

private:
    BYTE*    m_rgbyData;
};


class CacheListIterator {
// Friends
public:
    friend class CacheList;

// Creators
public:
    CacheListIterator(const CacheROList* p_pCacheList, BYTE* pbyData);

    // Accessors
public:
    CacheListItem operator*();

// Friend helper functions
public:
    friend BOOL operator==(const CacheListIterator& lhs, const CacheListIterator& rhs);
    friend BOOL operator!=(const CacheListIterator& lhs, const CacheListIterator& rhs);

// Manipulators
public:
    CacheListIterator& operator++();        // iterator++
    CacheListIterator operator++(int);      // ++iterator

// Attributes
private:
    BYTE*               m_pbyData;
    const CacheROList*  m_pCacheList;
};

// Creators
inline CacheListIterator::CacheListIterator(const CacheROList* p_pCacheList, BYTE* pbyData) :
    m_pbyData(pbyData), m_pCacheList(p_pCacheList) {}

// Operators
inline BOOL operator==(const CacheListIterator& lhs, const CacheListIterator& rhs) {

    // Test for equivalence
    return (lhs.m_pbyData == rhs.m_pbyData);
}

inline BOOL operator!=(const CacheListIterator& lhs, const CacheListIterator& rhs) {

    // Test for non-equivalence
    return (lhs.m_pbyData != rhs.m_pbyData);
}

inline CacheListItem CacheListIterator::operator*()
{
    // Return the current list item
    if (m_pCacheList->m_pbData + m_pCacheList->m_iLength <= m_pbyData) {
#ifndef WIN32
        throw std::exception();
#else
        throw std::out_of_range("Attempt to advance iterator past end of list");
#endif
    }
    return CacheListItem(m_pbyData);
}

inline CacheListIterator& CacheListIterator::operator++()
{
    // Advance the iterator
    if (m_pCacheList->m_pbData + m_pCacheList->m_iLength <= m_pbyData) {
#ifndef WIN32
        throw std::exception();
#else
        throw std::out_of_range("Attempt to advance iterator past end of list");
#endif
    }
    m_pbyData += CacheListItem(m_pbyData).size();
    return *this;
}

inline CacheListIterator CacheListIterator::operator++(int)
{
    // Advance the iterator, return previous iterator
    if (m_pCacheList->m_pbData + m_pCacheList->m_iLength <= m_pbyData) {
#ifndef WIN32
        throw std::exception();
#else
        throw std::out_of_range("Attempt to advance iterator past end of list");
#endif
    }
    CacheListIterator ret = *this;
    m_pbyData += CacheListItem(m_pbyData).size();
    return ret;
}

#ifdef TRACE_INCLUDE
#pragma message("...End   Include "__FILE__)
#endif

#else

#ifdef TRACE_INCLUDE
#pragma message("...Redundant Include Attempt " __FILE__)
#endif

#endif // _utlCacheList_h_
