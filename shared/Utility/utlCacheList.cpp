//**********************************************************************
//* Copyright (c) 1997-2006 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* utlCacheList.cpp: Implementation of LIST interface
//**********************************************************************
//* Change History:
//*
//* Gang Wang       8/10/18     Removed exception specification (GWA002)
//* Steven Hobbs     6/30/10	SOH216 fix stuffing negative 0 literals
//* Chuck Sorenson  10/12/06	CDS906 fix double scaling
//* Keith Knowles   05/30/06	Support long string elements in lists. (KMK470)
//* Jamie Newton    10/18/05    Added support for type CT_DOUBLE
//* Dave McCaldon   12/13/03    Added gcvt() for Mac OS X
//* Scott Jones:    3/29/02	    Fixed bugs introduced by port to C++
//* Chuck Sorenson: 6/02/00     Allow lower case 'e' for exponent
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

#include "utlCacheList.h"
#include "utlAutoArray.h"
#include "sysMemory.h"
#include "sysCharConversion.h"

#include <stdlib.h>
#include <limits.h>

// LL suffix is required by Linux, prohibited by WIN32
#ifndef WIN32
#define MAKELONGLONG(N) N##LL
#else
#define MAKELONGLONG(N) N##L
#endif

//const size_t       CHUNKSIZE   = 32;
//const size_t       LONGDIGIT10 = 18;

const LONGLONG  CONGNEGD5   = MAKELONGLONG(0x1999999999999999);
const LONGLONG  CONGNEGD10  = MAKELONGLONG(0x0CCCCCCCCCCCCCCC);

const int       GINT        = 2147483647;
const int       GNEGI       = 0x80000000; // this replaces the line below because of a compiler bug in VC++ which generates a warning
// const int GNEGI = -2147483648;

inline sysTypes::UINT64 SWAP64(sysTypes::UINT64 x)
{
    return  (sysTypes::UINT64(x) & (sysTypes::UINT64(MAKELONGLONG(0x00000000000000ffU))) << 56) |
            (sysTypes::UINT64(x) & (sysTypes::UINT64(MAKELONGLONG(0x000000000000ff00U))) << 40) |
            (sysTypes::UINT64(x) & (sysTypes::UINT64(MAKELONGLONG(0x0000000000ff0000U))) << 24) |
            (sysTypes::UINT64(x) & (sysTypes::UINT64(MAKELONGLONG(0x00000000ff000000U))) <<  8) |
            (sysTypes::UINT64(x) & (sysTypes::UINT64(MAKELONGLONG(0x000000ff00000000U))) >>  8) |
            (sysTypes::UINT64(x) & (sysTypes::UINT64(MAKELONGLONG(0x0000ff0000000000U))) >> 24) |
            (sysTypes::UINT64(x) & (sysTypes::UINT64(MAKELONGLONG(0x00ff000000000000U))) >> 40) |
            (sysTypes::UINT64(x) & (sysTypes::UINT64(MAKELONGLONG(0xff00000000000000U))) >> 56);
    }

inline sysTypes::UINT32 SWAP32(sysTypes::UINT32 x)
{
    return  (sysTypes::UINT32(x) & (sysTypes::UINT32(0x000000ffU)) << 24) |
            (sysTypes::UINT32(x) & (sysTypes::UINT32(0x0000ff00U)) <<  8) |
            (sysTypes::UINT32(x) & (sysTypes::UINT32(0x00ff0000U)) >>  8) |
            (sysTypes::UINT32(x) & (sysTypes::UINT32(0xff000000U)) >> 24);
}

union Align {
    BYTE                m_rgcBuffer[8];
    sysTypes::UINT64    m_uint64;
    sysTypes::UINT32    m_uint32;
    sysTypes::IEEE64    m_ieee64;
    sysTypes::IEEE32    m_ieee32;
    int                 m_int;
    LONGLONG            m_longlong;
};

int CacheROList::s_rgnNeedArray[] = {
    0,                  // 0
    32,                 // 1..32
    64,                 // 33..64
    128,128,            // 65..128
    256,256,256,256,    // 129..256
    512,512,512,512,    // 257..416
    512,512,512,512     // 417..512
};

/**
 * Array of powers of 10
 */

LONGLONG CacheROList::s_rgscaletab[] = {
    1,10,100,1000,10000,
    100000,1000000,10000000,100000000,1000000000,
    MAKELONGLONG(10000000000),
    MAKELONGLONG(100000000000),
    MAKELONGLONG(1000000000000),
    MAKELONGLONG(10000000000000),
    MAKELONGLONG(100000000000000),
    MAKELONGLONG(1000000000000000),
    MAKELONGLONG(10000000000000000),
    MAKELONGLONG(100000000000000000),
    MAKELONGLONG(1000000000000000000),
};

/**
 * Array of chars to lookup the char for the digit in the tenth's
 * place for a two digit, base ten number.  The char can be got by
 * using the number as the index.
 */
char CacheROList::s_rgtenstab[] = {
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '1', '1', '1', '1', '1', '1', '1', '1', '1', '1',
    '2', '2', '2', '2', '2', '2', '2', '2', '2', '2',
    '3', '3', '3', '3', '3', '3', '3', '3', '3', '3',
    '4', '4', '4', '4', '4', '4', '4', '4', '4', '4',
    '5', '5', '5', '5', '5', '5', '5', '5', '5', '5',
    '6', '6', '6', '6', '6', '6', '6', '6', '6', '6',
    '7', '7', '7', '7', '7', '7', '7', '7', '7', '7',
    '8', '8', '8', '8', '8', '8', '8', '8', '8', '8',
    '9', '9', '9', '9', '9', '9', '9', '9', '9', '9'
};

/**
 * Array of chars to lookup the char for the digit in the unit's
 * place for a two digit, base ten number.  The char can be got by
 * using the number as the index.
 */
char CacheROList::s_rgonestab[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
};

#define PASTE(x)    #x
char CacheROList::s_rglonglongmin[] = PASTE(_LONG64_MIN);

#if defined (__APPLE__)
/**
 * Implement a gcvt() emulator for the Mac OS X ... (as of 10.3)
 *
 * @param value    Value to convert
 * @param ndigits  Number of digits to max out at
 * @param buf      Character buffer to work in
 * @returns Converted character string
 */
inline char* gcvt(double p_fValue, size_t p_cDigits, char* p_pchBuffer)
{
    snprintf(p_pchBuffer, p_cDigits+1, "%f", p_fValue);

    return p_pchBuffer;
}
#endif

// CacheROList - Read only list

// Private constructor
CacheROList::
CacheROList()
{
    internalInit();
}

CacheROList::
CacheROList(const BYTE* p_pData, size_t p_iLen)
{
    internalInit();
    m_pbData = const_cast<BYTE*>(p_pData);
    m_iLength = p_iLen;
}

CacheROList::
CacheROList(const CountedByteArray& p_rData)
{
    internalInit();
    m_pbData = p_rData.data();
    m_iLength = p_rData.length();
}

CacheROList::
CacheROList(const CacheList& p_rList)
{
    internalInit();
    m_pbData = p_rList.data();
    m_iLength = p_rList.datalength();
}

CacheROList& CacheROList::
operator=(const CacheList& p_rList)
{
    internalInit();
    m_pbData = p_rList.data();
    m_iLength = p_rList.datalength();
    return *this;
}

CacheROList& CacheROList::
operator=(const CacheROList& p_rList)
{
    internalInit();
    m_pbData = p_rList.data();
    m_iLength = p_rList.datalength();
    return *this;
}

CacheROList& CacheROList::
operator=(const CountedByteArray& p_rData)
{
    internalInit();
    m_pbData = p_rData.data();
    m_iLength = p_rData.length();
    return *this;
}

size_t CacheROList::
datalength() const
{
    return m_iLength;
}

BYTE* CacheROList::
data() const
{
    return m_pbData;
}

size_t CacheROList::
size()
{
    if (!m_bCounted) {
        countItems();
    }

    return m_iCount;
}

const CountedByteArray CacheROList::
AsCountedByteArray() const
{
    return CountedByteArray(reinterpret_cast<const BYTE*>(m_pbData), m_iLength);
}

void CacheROList::internalInit()
{
    m_iLastIndex = 0;
    m_iLastOffset = 0;
    m_iCount = 0;
    m_iLength = 0;
    m_bCounted = FALSE;
    m_pbData = NULL;
}

void CacheROList::Reset()
{
    // Don't free the data or zero data count
    m_iLastIndex = 0;
    m_iLastOffset = 0;
    m_iCount = 0;
    m_iLength = 0;
    m_bCounted = FALSE;
}

size_t CacheROList::
countItems()
{
    size_t i;

    // If we have information available about the last access, use it
    m_iCount = m_iLastIndex;
    i = m_iLastOffset;

    while (i < m_iLength) {

        // Skip to the next item
        if (m_pbData[i] != 0) { // One BYTE length

            i += (m_pbData[i]&255);

        } else {    // First BYTE is 0, then 2-BYTE length
            if (i + 3 > m_iLength) {
                break;
            }

            size_t t_n = (m_pbData[i+1]&255) | ((m_pbData[i+2]&255) << 8);

            if (t_n != 0) { // 2 Byte length
                i += t_n + 3;

            } else { // 4 Byte length

                if (i + 7 > m_iLength) {
                	break;
            	}

                i += ((m_pbData[i+3]&255) | ((m_pbData[i+4]&255) << 8) | ((m_pbData[i+5]&255) << 16) | ((m_pbData[i+6]&255) << 24)) + 7;
            }
        }
        m_iCount++;
    }

    if (i != m_iLength) {
#ifndef WIN32
        throw std::exception();
#else
        throw std::invalid_argument("List Format Error");
#endif
    }
    return m_iCount;
}

CacheListItem CacheROList::
operator[](size_t nIndex) /* Exception specification is deprecated in C++11 */
{
    // If we are just resetting to the beginning of the list,
    // all we need to do is set the offset and index to 0
    if (nIndex == 0) {
        m_iLastIndex = m_iLastOffset = 0;
        return CacheListItem(m_pbData);
    }

    // If we are looking at the last item we visited, just return it
    if (nIndex == m_iLastIndex) {
        return CacheListItem(m_pbData + m_iLastOffset);
    }

    size_t i;
    size_t p;

    // If we are looking at (or past) the end of the list,
    // start at the end of the list, and extend it
    if (nIndex >= m_iCount && m_bCounted) {
        p = m_iCount;
        i = m_iLength;
        // If we're looking past the last item we visited,
        // then start there instead of at zero.
    } else {
        if (nIndex > m_iLastIndex) {
            p = m_iLastIndex;
            i = m_iLastOffset;
        } else {    // Otherwise, just start at beginning of list
            p = 0;
            i = 0;
        }
        // Now, we advance to the position requested by the user
        while (i < m_iLength) {

            if (m_pbData[ i ] == 0) { // 2 byte length

                if (i+3 > m_iLength) {
#ifndef WIN32
                    throw std::exception();
#else
                    throw std::invalid_argument("List Format Error");
#endif
                }

                size_t t_n = ((m_pbData[i+1]&255) | ((m_pbData[i+2]&255)<<8));

                if (t_n != 0) {

                    i += t_n + 3;
                } else { // 4 Byte length

                    if (i + 7 > m_iLength) {
#ifndef WIN32
                        throw std::exception();
#else
                        throw std::invalid_argument("List Format Error");
#endif
                    }

                    i += ((m_pbData[i+3]&255) | ((m_pbData[i+4]&255) << 8) | ((m_pbData[i+5]&255) << 16) | ((m_pbData[i+6]&255) << 24)) + 7;
                }

            } else {
                i += (m_pbData[i]&255); // One BYTE length
            }

            if (++p == nIndex) {    // We've found position

                if (i > m_iLength) {
#ifndef WIN32
                    throw std::exception();
#else
                    throw std::invalid_argument("List Format Error");
#endif
                }

                goto retpos;
            }
        }
        // Make sure we haven't gone past the end of the list
        if (i != m_iLength) {
#ifndef WIN32
            throw std::exception();
#else
            throw std::invalid_argument("List Format Error");
#endif
        }
        // Update counter if it wasn't already set
        if (!m_bCounted) {
            m_iCount = p;
            m_bCounted = TRUE;
        }
    }
    // Are we asking for a point after the end of the list
    if (p < nIndex) {
#ifndef WIN32
        throw std::exception();
#else
        throw std::out_of_range("Index out of bounds");
#endif
    }

retpos:

    // Remember the new position
    m_iLastIndex = p;
    m_iLastOffset = i;
    return CacheListItem(m_pbData + i);
}

// Return a CacheListIterator positioned at the first element of the list
CacheListIterator CacheROList::
begin() const
{
    return CacheListIterator(this, m_pbData);
}

// Return a CacheListIterator positioned one past the end of the list
CacheListIterator CacheROList::
end() const
{
    return CacheListIterator(this, m_pbData + m_iLength);
}

// Operator +
CacheList CacheROList::
operator+(const CacheList& rhs)
{
  CacheList out(*this);
  out += rhs;
  return out;
}

CacheList CacheROList::
operator+(const CacheROList& rhs)
{
  CacheList out(*this);
  out += rhs;
  return out;
}

CacheList CacheROList::
operator+(const CountedByteArray& rhs)
{
  CacheList out(*this);
  out += rhs;
  return out;
}

CacheList CacheROList::
operator+(const AString& rhs)
{
  CacheList out(*this);
  out += rhs;
  return out;
}

CacheList CacheROList::
operator+(const WString& rhs)
{
  CacheList out(*this);
  out += rhs;
  return out;
}

CacheList CacheROList::
operator+(const C16String& rhs)
{
  CacheList out(*this);
  out += rhs;
  return out;
}

CacheList CacheROList::
operator+(const CacheNull& rhs)
{
  CacheList out(*this);
  out += rhs;
  return out;
}

CacheList CacheROList::
operator+(const LONG& rhs)
{
  CacheList out(*this);
  out += rhs;
  return out;
}

CacheList CacheROList::
operator+(const size_t& rhs)
{
  CacheList out(*this);
  out += rhs;
  return out;
}

#ifndef LONG_EQUAL_LONGLONG
CacheList CacheROList::
operator+(const LONGLONG& rhs)
{
  CacheList out(*this);
  out += rhs;
  return out;
}
#endif
CacheList CacheROList::
operator+(const sysTypes::IEEE64& rhs)
{
  CacheList out(*this);
  out += rhs;
  return out;
}

CacheList CacheROList::
operator+(const int& rhs)
{
  CacheList out(*this);
  out += rhs;
  return out;
}

#if defined(WCHAR4)
#ifdef USE_ISC_NAMESPACE
CacheList CacheROList::operator+(const ISC::XString& rhs)
#else
CacheList CacheROList::operator+(const XString& rhs)
#endif // USE_ISC_NAMESPACE
{
  CacheList out(*this);
  out += rhs;
  return out;
}
#endif

  //See comment on operator+ in header.
#if !defined(WCHAR4) && !defined(_WIN32)
CacheList CacheROList::operator+(const std::basic_string<wchar_t>& rhs) {
  CacheList out(*this);
  out += rhs;
  return out;
}
#endif


// CacheList, one that has its own buffer and you can add items to it
CacheList::
CacheList()
{
    m_cbData = 0;
}

CacheList::
CacheList(const CountedByteArray& p_rData)
{
    m_cbData = 0;
    setList(p_rData.data(), p_rData.length());
}

CacheList::
CacheList(const BYTE* p_pData, size_t p_iLen)
{
    m_cbData = 0;
    setList(p_pData, p_iLen);
}

CacheList::CacheList(size_t cbInitial)
{
    m_cbData = 0;
    ensureBuffer( cbInitial );
}

CacheList::
CacheList(const CacheList& p_rList) : CacheROList()
{
    m_cbData = 0;
    setList(p_rList.m_pbData, p_rList.m_iLength);
}

CacheList::
CacheList(const CacheROList& p_rList)
{
    m_cbData = 0;
    setList(p_rList.m_pbData, p_rList.m_iLength);
}

CacheList::~CacheList()
{
    sysFree(m_pbData);
}

CacheList& CacheList::
operator=(const CacheList& p_rList)
{
    if (this == &p_rList) {
        return *this;
    }
    clear();
    setList(p_rList.m_pbData, p_rList.m_iLength);
    return *this;
}

CacheList& CacheList::
operator=(const CacheROList& p_rList)
{
    if (this == &p_rList) {
        return *this;
    }
    clear();
    setList(p_rList.m_pbData, p_rList.m_iLength);
    return *this;
}

void CacheList::setList(const BYTE* p_pData, size_t p_iLen)
{
    // Allocate list data
    ensureBuffer(p_iLen);

    // We must copy data from the list
    sysMemCpy(m_pbData, p_pData, p_iLen);
    m_iLength = p_iLen;
}

void CacheList::appendList(const CacheList& p_rList)
{
    ensureBuffer(p_rList.datalength() + m_iLength);
    sysMemCpy(&m_pbData[m_iLength], p_rList.data(), p_rList.datalength() );
    m_iLength += p_rList.datalength();

    if (m_bCounted) {
        if (p_rList.m_bCounted) {
            m_iCount += p_rList.m_iCount;
        } else {
            // Can't rely on combined count value
            m_bCounted = FALSE;
        }
    }
}

void CacheList::appendList(const CacheROList& p_rList)
{
    ensureBuffer(p_rList.datalength() + m_iLength);
    sysMemCpy(&m_pbData[m_iLength], p_rList.data(), p_rList.datalength() );
    m_iLength += p_rList.datalength();

    if (m_bCounted) {
        if (p_rList.m_bCounted) {
            m_iCount += p_rList.m_iCount;
        } else {
            // Can't rely on combined count value
            m_bCounted = FALSE;
        }
    }
}

void CacheList::appendEmptyItem()
{
    makeItem(m_iLength, 0);
}

void CacheList::
appendStringWithCompress(const WString &p_rwstr)
{
    appendStringWithCompress(p_rwstr.data(),static_cast<size_t>(p_rwstr.length()));
}

void CacheList::
appendStringWithCompress(const C16String &p_rwstr)
{
  appendStringWithCompress(reinterpret_cast<const WChar*>(p_rwstr.data()),static_cast<size_t>(p_rwstr.length()));
}

void CacheList::
appendXChars(const XChar* p_pData,size_t p_iLen)
{
    stuffUnicode(m_iLength, p_pData, p_iLen);
}

void CacheList::
appendAChars(const AChar* p_pData,size_t p_iLen)
{
    stuffAscii(m_iLength, p_pData, p_iLen);
}

void CacheList::
appendXStringWithCompress(const XChar* p_pData, size_t p_iLen, AChar* buf)
{
  // scan to see if it is really a wide string
  size_t i;
  for (i = 0; i < p_iLen; ++i) {
    if (static_cast<BYTE>(sysHIBYTE(p_pData[i])) != 0)
      break;
  }

  if (i == p_iLen) {
    // concatenate the low bytes of the wide-string to the byte array
    for (i = 0; i < p_iLen; ++i) {
      buf[i] = static_cast<AChar>(sysLOBYTE(p_pData[i]));
    }
    stuffAscii(m_iLength, buf, p_iLen);
  } else {
    stuffUnicode(m_iLength, p_pData,p_iLen);
  }
}

void CacheList::
appendXStringWithCompress(const XChar* p_pData,size_t p_iLen)
{
  // scan to see if it is really a wide string
  size_t i;
  for (i = 0; i < p_iLen; ++i) {
    if (static_cast<BYTE>(sysHIBYTE(p_pData[i])) != 0)
      break;
  }

  if (i == p_iLen) {
    AString t_str;
    t_str.reserve(p_iLen);
    // concatenate the low bytes of the wide-string to the byte array
    for (i = 0; i < p_iLen; ++i) {
      t_str += static_cast<AChar>(sysLOBYTE(p_pData[i]));
    }
    stuffAscii(m_iLength, t_str);
  } else {
    stuffUnicode(m_iLength, p_pData,p_iLen);
  }
}

void CacheList::
appendStringWithCompress(const WChar* p_pData,size_t p_iLen)
{
#ifdef WCHAR4

    // Need to convert to XString (2 byte unicode string)
#ifdef USE_ISC_NAMESPACE
    ISC::XString t_xstr = w2xStr(p_pData,p_iLen);
#else
    XString t_xstr = w2xStr(p_pData,p_iLen);
#endif  // USE_ISC_NAMESPACE
    appendXStringWithCompress(t_xstr.data(), p_iLen);
#else
    appendXStringWithCompress(p_pData,p_iLen);
#endif
}

void CacheList::
appendStringWithCompress(const AString &p_str)
{
    stuffAscii(m_iLength, p_str);
}

void CacheList::appendStringWithCompress(const AChar* p_pData, size_t p_iLen)
{
    stuffAscii(m_iLength,p_pData, p_iLen);
}

void CacheList::
appendDoubleString(const WString &p_str)
{
#ifdef WCHAR4

    // Need to convert from 4 byte to an single byte string
    AString t_astr = wStr2aStr(p_str);

    stuffDouble(m_iLength, t_astr);
#else
    AString t_str; AString::size_type i;
    t_str.reserve(p_str.size());

    // convert to an Astring
    for (i = 0; i < p_str.size(); ++i) {
        t_str += static_cast<AChar>(sysLOBYTE(p_str[i]));
    }

    stuffDouble(m_iLength, t_str);
#endif
}

void CacheList::
appendDoubleString(const AString &p_str)
{
    stuffDouble(m_iLength, p_str);
}

void CacheList::
appendIEEE32(sysTypes::IEEE32 p_d)
{
    stuffIEEE32(m_iLength,p_d);
}

void CacheList::
appendIEEE64(sysTypes::IEEE64 p_d)
{
    stuffIEEE64(m_iLength,p_d);
}

void CacheList::clear()
{
    sysFree(m_pbData);
    m_cbData = 0;
    internalInit();
}

const CacheListIterator CacheList::
erase(const CacheListIterator &p_Iterator)
{
    M_ASSERT(p_Iterator.m_pCacheList == this);  // Iterator is for this list!

    size_t l;
    size_t i = static_cast<size_t>(p_Iterator.m_pbyData - m_pbData);

    // We can only delete items if we're not at the end of the list
    if (i < m_iLength) {
        // Compute the length of the next item
        if (0 == (l = (m_pbData[i]&255))) {

            // First BYTE is 0, length is in following 2 BYTEs
            size_t t_n = ((m_pbData[i+1]&255)|((m_pbData[i+2]&255)<<8));
            if (t_n != 0) {
                l = t_n + 3;
            } else {
                // 4 Byte length
                l = ((m_pbData[i+3]&255) | ((m_pbData[i+4]&255) << 8) | ((m_pbData[i+5]&255) << 16) | ((m_pbData[i+6]&255) << 24)) + 7;
            }
        }

        // Now, shuffle the remainder down to fill the gap
        m_iLength -= l;
        sysMemMoveLeft( &m_pbData[i], &m_pbData[i+l], m_iLength-i );
        if (m_bCounted) {
            m_iCount--;
        }
    }
    return p_Iterator;
}

void CacheList::ensureBuffer( size_t p_cNeeded )
{
    if (p_cNeeded > m_cbData) {
        // Allocate a larger array and copy the data into it
        m_cbData = (p_cNeeded <= 512)
                   ? s_rgnNeedArray[(p_cNeeded+31)>>5]  // >>>
                   : ((p_cNeeded + 1023) & ~1023);

        m_pbData = reinterpret_cast<BYTE*>(sysRealloc(m_pbData, m_cbData));
    }
}

size_t CacheList::makeItem(size_t p_iOffset, size_t p_iLen)
{
    if (p_iLen < 255) { // One BYTE length
        p_iLen++;
        // Now, make room. First, we ensure the array is big enough
        ensureBuffer(m_iLength + p_iLen);
        if (m_iLength != p_iOffset) {
            // Move elements down to make room
            sysMemMoveRight( &m_pbData[ p_iOffset + p_iLen ],
                             &m_pbData[ p_iOffset ],
                             m_iLength - p_iOffset );
        }

        m_iLength += p_iLen;
	m_iCount++;


        // Store the length
        m_pbData[p_iOffset] = (BYTE) p_iLen;
        m_iLastIndex++;
        m_iLastOffset += p_iLen;


        return p_iOffset + 1;
    } else if (p_iLen < 65536) {				// KMK470

	// Three BYTEs to store length
	ensureBuffer(m_iLength + p_iLen + 3);
	if (m_iLength != p_iOffset) {
	    // Move elements down to make room
	    sysMemMoveRight( &m_pbData[ p_iOffset + p_iLen + 3 ],
			     &m_pbData[ p_iOffset ],
			     m_iLength - p_iOffset );
	}

	m_iLength += p_iLen + 3;
	m_iCount++;

	m_pbData[p_iOffset] = 0;
	m_pbData[p_iOffset+1] = (BYTE) p_iLen;
	m_pbData[p_iOffset+2] = (BYTE) (p_iLen>>8);
	m_iLastIndex++;
	m_iLastOffset += (p_iLen+3);
	return p_iOffset + 3;
    } else {							// KMK470+
        // Seven bytes to store the length.
	ensureBuffer(m_iLength + p_iLen + 7);
	if (m_iLength != p_iOffset) {
	    sysMemMoveRight(
		&m_pbData[p_iOffset + p_iLen + 7],
		&m_pbData[p_iOffset],
		m_iLength - p_iOffset);
	}
	m_iLength += p_iLen + 7;
	m_iCount++;

	m_pbData[p_iOffset] = 0;
	m_pbData[p_iOffset + 1] = 0;
	m_pbData[p_iOffset + 2] = 0;
	m_pbData[p_iOffset + 3] = (BYTE) p_iLen;
	m_pbData[p_iOffset + 4] = (BYTE) (p_iLen >> 8);
	m_pbData[p_iOffset + 5] = (BYTE) (p_iLen >> 16);
	m_pbData[p_iOffset + 6] = (BYTE) (p_iLen >> 24);
	m_iLastIndex++;
	m_iLastOffset += p_iLen + 7;
	return p_iOffset + 7;
    }
}

void CacheList::stuffAscii( size_t p_iItem, const AString& s)
{
   stuffAscii(p_iItem,s.data(),static_cast<size_t>(s.length()));
}

void CacheList::stuffAscii(size_t p_iItem,const AChar* p_psa,size_t p_nLen)
{
    // Make room for the string plus the type BYTE
    size_t t_i = makeItem(p_iItem,(size_t)(p_nLen + 1));

    // Fill in the type
    m_pbData[t_i++] = CT_ASCII;

    // Copy in the data
    sysMemCpy(&m_pbData[t_i], reinterpret_cast<const BYTE *>(p_psa), p_nLen);
}

#ifdef USE_ISC_NAMESPACE
void CacheList::stuffUnicode(size_t p_iItem, const ISC::XString& p_rstr)
#else
void CacheList::stuffUnicode(size_t p_iItem, const XString& p_rstr)
#endif // USE_ISC_NAMESPACE
{
    stuffUnicode(p_iItem,p_rstr.data(),static_cast<size_t>(p_rstr.length()));
}

#ifdef USE_ISC_NAMESPACE
void CacheList::stuffUnicode(size_t p_iItem, const ISC::C16String& p_rstr)
#else
void CacheList::stuffUnicode(size_t p_iItem, const C16String& p_rstr)
#endif // USE_ISC_NAMESPACE
{
  stuffUnicode(p_iItem,reinterpret_cast<const XChar*>(p_rstr.data()),static_cast<size_t>(p_rstr.length()));
}

#ifdef USE_ISC_NAMESPACE
void CacheList::stuffUnicode(size_t p_iItem, const XChar* p_psx, size_t p_nLen)
#else
void CacheList::stuffUnicode(size_t p_iItem, const XChar* p_psx, size_t p_nLen)
#endif
{
    // Make room for the string plus the type BYTE
    size_t t_i = makeItem(p_iItem, (size_t)((p_nLen * 2) + 1));

    // Fill in the unicode flag
    m_pbData[t_i++] = CT_UNICODE;

#if (defined(_ISC_BIGENDIAN) && _ISC_BIGENDIAN)
    size_t t_j;
    for (t_j = 0; t_j < p_nLen; t_j++) {
        m_pbData[t_i++] = p_psx[t_j] & 0xFF;
        m_pbData[t_i++] = p_psx[t_j] >> 8;
    }
#else
    sysMemCpy(&m_pbData[t_i], reinterpret_cast<const BYTE*>(p_psx), p_nLen * 2);
#endif
}

void CacheList::stuffNull(size_t p_iItem)
{
    // Make room for the null value.  Note that a null value has a length of 1
    // and no data.
	makeItem(p_iItem, 0);
}

void CacheList::stuffByte(size_t p_iItem, const BYTE* p_pData, size_t p_iLen)
{
    size_t t_i;

    // Make room for the string plus the type BYTE
    t_i = makeItem(p_iItem, p_iLen + 1);

    m_pbData[t_i++] = CT_ASCII;
    sysMemCpy(&m_pbData[t_i], p_pData, p_iLen );
}

void CacheList::stuffNegInt(size_t i, int value)
{
    if (value < -0x10000) {
        if (value < -0x1000000) {
            i = makeItem(i,5);
            m_pbData[i+4] = (BYTE) (value>>24);
        } else {
            i = makeItem(i,4);
        }
        m_pbData[i+3] = (BYTE) (value>>16);
        m_pbData[i+2] = (BYTE) (value>>8);
        m_pbData[i+1] = (BYTE) value;
    } else if (value < -0x100) {
        i = makeItem(i,3);
        m_pbData[i+2] = (BYTE) (value>>8);
        m_pbData[i+1] = (BYTE) value;
    } else if (value < -1) {					// SOH216+
        i = makeItem(i,2);
        m_pbData[i+1] = (BYTE) value;
    } else if (value == 0) {
        // Special case -0 as +0
        stuffPosInt(i, value);
	return;							// SOH216-
    } else {
        i = makeItem(i,1);
    }

    m_pbData[i] = CT_NEGINT;
}

void CacheList::stuffPosInt(size_t i, int value)
{
    if (value > 0xFFFF) {
        if (value > 0xFFFFFF) {
            i = makeItem(i,5);
            m_pbData[i+4] = (BYTE) (value>>24);
        } else {
            i = makeItem(i,4);
        }
        m_pbData[i+3] = (BYTE) (value>>16);
        m_pbData[i+2] = (BYTE) (value>>8);
        m_pbData[i+1] = (BYTE) value;
    } else if (value > 0xFF) {
        i = makeItem(i,3);
        m_pbData[i+2] = (BYTE) (value>>8);
        m_pbData[i+1] = (BYTE) value;
    } else if (value != 0) {
        i = makeItem(i,2);
        m_pbData[i+1] = (BYTE) value;
    } else {
        i = makeItem(i,1);
    }
    m_pbData[i] = CT_POSINT;
}

void CacheList::stuffPosLong(size_t i, LONGLONG value)
{
    size_t len;

#ifndef WIN32
    // Do a kind a binary search to find the length
    // i.e. only 3 or 4 comparisons
    len = (value > 0xFFFFFFFFL)
          ? ((value >  0xFFFFFFFFFFFFLL)
         ? ((value > 0xFFFFFFFFFFFFFFLL) ? 8 : 7)
         : ((value > 0xFFFFFFFFFFLL) ? 6 : 5))
          : ((value > 0xFFFF)
         ? ((value > 0xFFFFFF) ? 4 : 3)
         : ((value > 0xFF) ? 2 : ((value > 0) ? 1 : 0)));
#else   // WIN32
    len = (value > 0xFFFFFFFFL)
          ? ((value >  0xFFFFFFFFFFFFL)
         ? ((value > 0xFFFFFFFFFFFFFFL) ? 8 : 7)
         : ((value > 0xFFFFFFFFFFL) ? 6 : 5))
          : ((value > 0xFFFF)
         ? ((value > 0xFFFFFF) ? 4 : 3)
         : ((value > 0xFF) ? 2 : ((value > 0) ? 1 : 0)));
#endif  // WIN32

    i = makeItem(i,len+1);
    switch (len) {
    case 8: m_pbData[i+8] = (BYTE) (value>>56);
    case 7: m_pbData[i+7] = (BYTE) (value>>48);
    case 6: m_pbData[i+6] = (BYTE) (value>>40);
    case 5: m_pbData[i+5] = (BYTE) (value>>32);
    case 4: m_pbData[i+4] = (BYTE) (value>>24);
    case 3: m_pbData[i+3] = (BYTE) (value>>16);
    case 2: m_pbData[i+2] = (BYTE) (value>>8);
    case 1: m_pbData[i+1] = (BYTE) value;
    }
    m_pbData[i] = CT_POSINT;
}

void CacheList::stuffNegLong(size_t i, LONGLONG value)
{
    // Special case -0 and treat as +0				// SOH216+
    if (value == 0) {
        stuffPosLong(i, value);
	return;
    }								// SOH216-

    size_t len;
    // Do a kind a binary search to find the length
    // i.e. only 3 or 4 comparisons

#ifndef WIN32
    len = (value < -0x100000000LL)
          ? ((value < -0x1000000000000LL)
         ? ((value < -0x100000000000000LL) ? 8 : 7)
         : ((value < -0x10000000000LL) ? 6 : 5))
          : ((value < -0x10000)
         ? ((value < -0x100000) ? 4 : 3)
         : ((value < -0x100) ? 2 : ((value < -1) ? 1 : 0)));
#else   // WIN32
    len = (value < -0x100000000L)
          ? ((value < -0x1000000000000L)
         ? ((value < -0x100000000000000L) ? 8 : 7)
         : ((value < -0x10000000000L) ? 6 : 5))
          : ((value < -0x10000)
         ? ((value < -0x100000) ? 4 : 3)
         : ((value < -0x100) ? 2 : ((value < -1) ? 1 : 0)));
#endif  // WIN32

    i = makeItem(i,len+1);
    switch (len) {
    case 8: m_pbData[i+8] = (BYTE) (value>>56);
    case 7: m_pbData[i+7] = (BYTE) (value>>48);
    case 6: m_pbData[i+6] = (BYTE) (value>>40);
    case 5: m_pbData[i+5] = (BYTE) (value>>32);
    case 4: m_pbData[i+4] = (BYTE) (value>>24);
    case 3: m_pbData[i+3] = (BYTE) (value>>16);
    case 2: m_pbData[i+2] = (BYTE) (value>>8);
    case 1: m_pbData[i+1] = (BYTE) value;
    }
    m_pbData[i] = CT_NEGINT;
}

void CacheList::stuffPosNum(size_t i, int scale, LONGLONG value)
{
    size_t len;
    // Do a kind a binary search to find the length
    // i.e. only 3 or 4 comparisons

#ifndef WIN32
    len = (value > 0xFFFFFFFFL)
          ? ((value >  0xFFFFFFFFFFFFLL)
         ? ((value > 0xFFFFFFFFFFFFFFLL) ? 8 : 7)
         : ((value > 0xFFFFFFFFFFLL) ? 6 : 5))
          : ((value > 0xFFFF)
         ? ((value > 0xFFFFFF) ? 4 : 3)
         : ((value > 0xFF) ? 2 : ((value > 0) ? 1 : 0)));
#else   // WIN32
    len = (value > 0xFFFFFFFFL)
          ? ((value >  0xFFFFFFFFFFFFL)
         ? ((value > 0xFFFFFFFFFFFFFFL) ? 8 : 7)
         : ((value > 0xFFFFFFFFFFL) ? 6 : 5))
          : ((value > 0xFFFF)
         ? ((value > 0xFFFFFF) ? 4 : 3)
         : ((value > 0xFF) ? 2 : ((value > 0) ? 1 : 0)));
#endif  // WIN32

    i = makeItem(i,len+2);
    m_pbData[i++] = CT_POSREAL;
    m_pbData[i] = (BYTE) scale;
    switch (len) {
    case 8: m_pbData[i+8] = (BYTE) (value>>56);
    case 7: m_pbData[i+7] = (BYTE) (value>>48);
    case 6: m_pbData[i+6] = (BYTE) (value>>40);
    case 5: m_pbData[i+5] = (BYTE) (value>>32);
    case 4: m_pbData[i+4] = (BYTE) (value>>24);
    case 3: m_pbData[i+3] = (BYTE) (value>>16);
    case 2: m_pbData[i+2] = (BYTE) (value>>8);
    case 1: m_pbData[i+1] = (BYTE) value;
    }
}

void CacheList::stuffNegNum(size_t i, int scale, LONGLONG value) {
    // Special case -0 and treat as +0				// SOH216+
    if (value == 0) {
        stuffPosNum(i, scale, value);
	return;
    }								// SOH216-

    size_t len;
    // Do a kind a binary search to find the length
    // i.e. only 3 or 4 comparisons

#ifndef WIN32
    len = (value < -0x100000000LL)
          ? ((value < -0x1000000000000LL)
         ? ((value < -0x100000000000000LL) ? 8 : 7)
         : ((value < -0x10000000000LL) ? 6 : 5))
          : ((value < -0x10000)
         ? ((value < -0x100000) ? 4 : 3)
         : ((value < -0x100) ? 2 : ((value < -1) ? 1 : 0)));
#else   // WIN32
    len = (value < -0x100000000L)
          ? ((value < -0x1000000000000L)
         ? ((value < -0x100000000000000L) ? 8 : 7)
         : ((value < -0x10000000000L) ? 6 : 5))
          : ((value < -0x10000)
         ? ((value < -0x100000) ? 4 : 3)
         : ((value < -0x100) ? 2 : ((value < -1) ? 1 : 0)));
#endif  // WIN32

    i = makeItem(i,len+2);
    m_pbData[i++] = CT_NEGREAL;
    m_pbData[i] = (BYTE) scale;
    switch(len) {
    case 8: m_pbData[i+8] = (BYTE) (value>>56);
    case 7: m_pbData[i+7] = (BYTE) (value>>48);
    case 6: m_pbData[i+6] = (BYTE) (value>>40);
    case 5: m_pbData[i+5] = (BYTE) (value>>32);
    case 4: m_pbData[i+4] = (BYTE) (value>>24);
    case 3: m_pbData[i+3] = (BYTE) (value>>16);
    case 2: m_pbData[i+2] = (BYTE) (value>>8);
    case 1: m_pbData[i+1] = (BYTE) value;
    }
}



/**
 * Inserts a DOUBLE (positive/negative). IEEE 754 DOUBLE format is
 * as follows:
 *
 *  int s = ((bits >> 63) == 0) ? 1 : -1;
 *  int e = (int)((bits >> 52) & 0x7ffL);
 *  LONG m = (e == 0) ? (bits & 0xfffffffffffffL) << 1 :
 *      (bits & 0xfffffffffffffL) | 0x10000000000000L;
 *
 * Then the floating-point result equals the value of the mathematical
 * expression:
 *
 *      s * m * (2 ** (e - 1075))
 *
 * If e is 0x7ff, then the DOUBLE is either +/- Infinity, or a
 * "NaN" (Not-a-Number)
 * See 'Double.LONGBitsToDouble()' in the Java Language Specification
 * for more details.
 *
 * @param   offset  Offset in the array to beginning of the element
 * @param   value   IEEE DOUBLE to insert
 */

void CacheList::stuffDouble(size_t offset, sysTypes::IEEE64 value) {
    BOOL        	sign;
    LONGLONG    	bits;
    AString     	s;
    int     		scale = 0;
    AString::size_type  len = 0;
    AString::size_type  dec = 0;
    AString::size_type  exploc;
    int     exp;
    size_t     i;

    bits = *(reinterpret_cast<LONGLONG *>(&value));

    sign = (bits < 0);
    exp = (int)((bits >> 52) & 0x7ffL);

    bits &= MAKELONGLONG(0xfffffffffffff);
    if (exp == (0x7ff)) {
#ifndef WIN32
        throw std::exception();
#else
        throw std::invalid_argument(bits == 0 ? (sign ? "-Infinity" : "Infinity") : sign ? "NaN: -" : "NaN: ");
#endif
    }
    if (0 == exp) {
        if (0 == bits) {    // Zero
            offset = makeItem(offset, 1);
            m_pbData[offset] = CT_POSINT;
            return;
        }
        bits <<= 1;
    } else {
        bits |= MAKELONGLONG(0x10000000000000);
    }
    exp -= 1075;
    if (exp < 0) {
        // Don't bother if there is no chance we can handle it by scaling
        if (exp >= -77) {
            // We can chop off at most 51 bits
            // (there must be at least 1 non-zero bit)
            // First, see if we can chop off 32 bits
            if (0 == (bits & 0xffffffffL)) {
                if (exp <= -32) {
                    bits >>= 32;    // >>>
                    exp += 32;
                } else {
                    bits >>= -exp;  // >>>
                    exp = 0;
                }
            }
            // See if we can chop off 16 bits
            if (0 == (bits & 0xffff)) {
                if (exp <= -16) {
                    bits >>= 16;    // >>>
                    exp += 16;
                } else if (exp != 0) {
                    bits >>= -exp;  // >>>
                    exp = 0;
                }
            }
            // See if we can chop off 8 bits
            if (0 == (bits & 0xff)) {
                if (exp <= -8) {
                    bits >>= 8;     // >>>
                    exp += 8;
                } else if (exp != 0) {
                    bits >>= -exp;  // >>>
                    exp = 0;
                }
            }
            // See if we can chop off 4 bits
            if (0 == (bits & 15)) {
                if (exp <= -4) {
                    bits >>= 4;     // >>>
                    exp += 4;
                } else if (exp != 0) {
                    bits >>= -exp;  // >>>
                    exp = 0;
                }
            }
            // See if we can chop off 2 bits
            if (exp < -1 && 0 == (bits & 3)) {
                bits >>= 2;     // >>>
                exp += 2;
            }
            // See if we can chop off the last bit
            if (0 != exp && 0 == (bits & 1)) {
                bits >>= 1;     // >>>
                ++exp;
            }
            // See if we can multiply by 5s, incrementing
            // exp each time, decrementing the scale also
            while (exp != 0 && bits <= CONGNEGD5) {
                bits *= 5;
                --scale;
                ++exp;
            }
        }
    } else if (exp > 0) {
        // We know here that we can shift left at most 10 times
        if (exp > 10) {
            // Only try scaling if it looks like it could help
            if (exp <= 76) {
                // Adjust as much as possible
                bits <<= 10;
                exp -= 10;
                // If we can divide by fives evenly, we can adjust
                // the decimal scale, as well as shift up by 4
                while ((bits % 5) == 0) {
                    bits /= 5;
                    scale++;
                    if (--exp == 0) {
                        break;
                    }
                }
                while (exp > 0 && 0 == (bits & MAKELONGLONG(0x4000000000000000))) {
                    bits <<= 1;
                    exp--;
                }
            }
        } else {
            bits <<= exp;
            exp = 0;
        }
    }
    if (exp != 0) {
        // We will use C++'s DOUBLE to string converter,
        // and then see if we can put it into a Cache DOUBLE

        // The original buffer size of 60 was inadequate to support conversion of ALL values
        // _CVTBUFSIZE guarantees enough buffer space

#ifndef _CVTBUFSIZE
#define _CVTBUFSIZE 512
#endif
        char t_rgchWork[ _CVTBUFSIZE ];
        s = gcvt( value, 60, t_rgchWork );

        len = s.length();
        scale = 0;
        bits = 0;
        // We assume that these always have a .
        // and at least 1 digit before and after the .
        dec = s.find('.');
        // There may or may not be an E
        if (((exploc = s.find('E', dec)) != AString::npos) || ((exploc = s.find('e', dec)) != AString::npos)) {
            BOOL    expsign = FALSE;
            int     savend = (int)exploc;
            if (s.at(exploc+1) == '-') {
                expsign = TRUE;
                ++exploc;
            }
            // We assume there is only 1-3 digits of exponent
            while (++exploc < len) {
                scale = scale*10 + (s.at(exploc) - '0');
            }
            if (expsign) {
                scale = -scale;
            }
            // The 'E' is now the end
            len = savend;
        }
        if (dec + 2 == len && s.at(dec+1) == '0') {
            len = dec;
        } else {
            // Adjust scale for decimal point
            scale -= (int)(len - dec - 1);
        }
        // If it is at all possible that we can adjust the
        // scale to fit in the range -128 to 127,
        // read in value as LONG and try to adjust
        if (scale <= 127+19 && scale >= (-128-19)) {
            // Parse digits - we assume that it will always fit in
            // a LONG, having come from a IEEE DOUBLE originally
            bits = 0;
            for (i = (sign ? 1 : 0); i < dec; i++) {
                bits = bits*10 + (s.at(i) - '0');
            }
            // Read in fractional part
            for (i = (int)(dec+1); i < len; i++) {
                bits = bits*10 + (s.at(i) - '0');
            }
            if (scale > 127) {
                do {
                    if (bits > CONGNEGD10) {
                        break;
                    }
                    bits *= 10;
                } while (--scale > 127);
            } else if (scale < -128) {
                // Reduce scale by 100's, then by a single 1
                do {
                    if (bits % 10 != 0) {
                        break;
                    }
                    bits /= 10;
                } while (++scale < -128);
            }
        }
    }
    if (scale > 127) {
        // Make room for the string plus the type BYTE
        offset = makeItem(offset, (int)(((dec == len) ? 1 : 0) + len + scale));
        m_pbData[offset++] = CT_ASCII;
        // Copy string (not including decimal point)
        for (i = 0; i < dec; i++) {
            m_pbData[offset++] = (BYTE) s.at(i);
        }
        while (++i < len) {
            m_pbData[offset++] = (BYTE) s.at(i);
        }
        // Pad with zeros
        for (i = 0; i < (size_t)scale; i++) {
            m_pbData[offset++] = (BYTE) '0';
        }
    } else if (scale < -128) {
        // [-].[0]digits
        // Make room for the string plus the type BYTE
        exp = (int)(-scale - (len - (sign ? 1 : 0) - ((dec == len) ? 0 : 1)));
        offset = makeItem(offset, (sign ? 3 : 2) - scale);
        m_pbData[offset++] = CT_ASCII;
        if (sign) {
            m_pbData[offset++] = (BYTE) '-';
        }
        m_pbData[offset++] = (BYTE) '.';
        for (i = 0; i < (size_t)exp; i++) {
            m_pbData[offset++] = (BYTE) '0';
        }
        // Copy string (not including decimal point)
        for (i = (sign ? 1 : 0); i < dec; i++) {
            m_pbData[offset++] = (BYTE) s.at(i);
        }
        while (++i < len) {
            m_pbData[offset++] = (BYTE) s.at(i);
        }
    } else if (scale != 0) {
        if (sign) {
            stuffNegNum(offset, scale, -bits);
        } else {
            stuffPosNum(offset, scale, bits);
        }
    } else if (sign) {
        stuffNegLong(offset, -bits);
    } else {
        stuffPosLong(offset, bits);
    }
}

// DoubleString := [ Digit ]* [.] [ Digit ]* ['E'] [{+|-} Digit [Digit]*]
void CacheList::stuffDouble(size_t offset, const AString& s) {

    AString::size_type len = s.length();

    // Detect sign
    BOOL sign = (s.at(0) == '-');

    int         scale = 0;
    LONGLONG    bits = 0;

    // There may or may not be an {E | e}
    AString::size_type exp;
    if (((exp = s.find('E')) != AString::npos) || ((exp = s.find('e')) != AString::npos)) {

        BOOL    expsign = FALSE;
        int     savend = (int)exp;

        if ((s.at(exp+1) == '-') || (s.at(exp+1) == '+')) {
            if (s.at(exp+1) == '-') expsign = TRUE;
            ++exp;
        }

        // There may be multiple digits of exponent
        while (++exp < len) {
            scale = scale*10 + (s.at(exp) - '0');
        }
        if (expsign) {
            scale = -scale;
        }
        // The 'E' is now the end
        len = savend;
    }

    AString::size_type dec = s.find('.');
    if (dec != AString::npos) {

        if (dec + 2 == len && s.at(dec+1) == '0') {

            // trailing zero, ignore it
            len = dec;
        } else {
            // Adjust scale for decimal point
            scale -= (int)(len - dec - 1);
        }
    } else {
        // if we got here then there is no decimal point so lets put it at the end
        dec = len;
    }

    // If it is at all possible that we can adjust the
    // scale to fit in the range -128 to 127,
    // read in value as LONG and try to adjust
    size_t i;
    if (scale <= 127+19 && scale >= (-128-19)) {
        // Parse digits - we assume that it will always fit in
        // a LONG, having come from a IEEE DOUBLE originally
        LONGLONG btest;
        bits = 0;

        for (i = (sign ? 1 : 0); i < dec; i++) {
#if 0
            bits = bits*10 + (s.at(i) - '0');
#else       // check for overflow
            btest = bits*10 + (s.at(i) - '0');

            if ((btest > CONGNEGD10) || (btest < bits)) {
                scale = scale + (int)(dec - i);
                break;
            }
            bits = btest;
#endif
        }
        // Read in fractional part
        for (i = (int)(dec+1); i < len; i++) {
#if 0
            bits = bits*10 + (s.at(i) - '0');
#else       // check for overflow
            btest = bits*10 + (s.at(i) - '0');

            if ((btest > CONGNEGD10) || (btest < bits)) {
                scale = scale + (int)(len - i);
                break;
            }
            bits = btest;
#endif
        }
        if (scale > 127) {
	    // See if we can scale it to fit
	    int tscale = scale;					// CDS906
            do {
                if (bits > CONGNEGD10) {
                    break;
                }
                bits *= 10;
            } while (--tscale > 127);
	    if (tscale <= 127)		// If it fits
		scale = tscale;		// use the new scale
        } else if (scale < -128) {
	    // See if we can scale it to fit
	    int tscale = scale;					// CDS906
            do {
                if (bits % 10 != 0) {
                    break;
                }
                bits /= 10;
            } while (++tscale < -128);
	    if (tscale >= -128)		// If it fits
		scale = tscale;		// use the new scale
        }
    }
    if (scale > 127) {
        // Make room for the string plus the type BYTE
        offset = makeItem(offset, (int)(((dec == len) ? 1 : 0) + len + scale));
        m_pbData[offset++] = CT_ASCII;
        // Copy string (not including decimal point)
        for (i = 0; i < dec; i++) {
            m_pbData[offset++] = (BYTE) s.at(i);
        }
        while (++i < len) {
            m_pbData[offset++] = (BYTE) s.at(i);
        }
        // Pad with zeros
        for (i = 0; i < (size_t)scale; i++) {
            m_pbData[offset++] = (BYTE) '0';
        }
    } else if (scale < -128) {
        // [-].[0]digits
        // Make room for the string plus the type BYTE
        exp = -scale - (len - (sign ? 1 : 0) - ((dec == len) ? 0 : 1));
        offset = makeItem(offset, (sign ? 3 : 2) - scale);
        m_pbData[offset++] = CT_ASCII;
        if (sign) {
            m_pbData[offset++] = (BYTE) '-';
        }
        m_pbData[offset++] = (BYTE) '.';
        for (i = 0; i < exp; i++) {
            m_pbData[offset++] = (BYTE) '0';
        }
        // Copy string (not including decimal point)
        for (i = (sign ? 1 : 0); i < dec; i++) {
            m_pbData[offset++] = (BYTE) s.at(i);
        }
        while (++i < len) {
            m_pbData[offset++] = (BYTE) s.at(i);
        }
    } else if (scale != 0) {
        if (sign) {
            stuffNegNum(offset, scale, -bits);
        } else {
            stuffPosNum(offset, scale, bits);
        }
    } else if (sign) {
        stuffNegLong(offset, -bits);
    } else {
        stuffPosLong(offset, bits);
    }
}

void CacheList::stuffIEEE64(size_t p_iItem,sysTypes::IEEE64 value)
{
#if (defined(_ISC_BIGENDIAN) && _ISC_BIGENDIAN)
    value = static_cast<sysTypes::IEEE64>(SWAP64(static_cast<sysTypes::UINT64>(value)));
#endif
    // Make room for the string plus the type DOUBLE
    size_t t_i = makeItem(p_iItem, sizeof(sysTypes::IEEE64) + 1);

    m_pbData[t_i++] = CT_DOUBLE;
    sysMemCpy(&m_pbData[t_i], reinterpret_cast<BYTE*>(&value), sizeof(sysTypes::IEEE64));
}

void CacheList::stuffIEEE32(size_t p_iItem,sysTypes::IEEE32 value)
{
#if (defined(_ISC_BIGENDIAN) && _ISC_BIGENDIAN)
    value = static_cast<sysTypes::IEEE32>(SWAP32(static_cast<sysTypes::UINT32>(value)));
#endif
    // Make room for the string plus the type DOUBLE
    size_t t_i = makeItem(p_iItem, sizeof(sysTypes::IEEE32) + 1);

    m_pbData[t_i++] = CT_DOUBLE;
    sysMemCpy(&m_pbData[t_i], reinterpret_cast<BYTE*>(&value), sizeof(sysTypes::IEEE32));
}

const CacheListIterator CacheList::
insert(const CacheListIterator &p_Iterator, const CacheList& p_rList)
{
    M_ASSERT(p_Iterator.m_pCacheList == this);  // Iterator is for this list!
    stuffByte(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData), p_rList.m_pbData, p_rList.m_iLength);
    return p_Iterator;
}

const CacheListIterator CacheList::
insert(const CacheListIterator &p_Iterator, const CacheROList& p_rList)
{
    M_ASSERT(p_Iterator.m_pCacheList == this);  // Iterator is for this list!
    stuffByte(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData), p_rList.m_pbData, p_rList.m_iLength);
    return p_Iterator;
}

const CacheListIterator CacheList::
insert(const CacheListIterator &p_Iterator, const CountedByteArray& p_rData)
{
    M_ASSERT(p_Iterator.m_pCacheList == this);  // Iterator is for this list!
    stuffByte(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData), p_rData.data(), p_rData.length());
    return p_Iterator;
}

const CacheListIterator CacheList::
insert(const CacheListIterator &p_Iterator, const AString& p_rStr)
{
    M_ASSERT(p_Iterator.m_pCacheList == this);  // Iterator is for this list!
    stuffAscii(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData), p_rStr);
    return p_Iterator;
}

const CacheListIterator CacheList::
insert(const CacheListIterator &p_Iterator, const WString& p_rStr)
{
     M_ASSERT(p_Iterator.m_pCacheList == this);  // Iterator is for this list!
#ifdef WCHAR4
#ifdef USE_ISC_NAMESPACE
    ISC::XString t_xstr = wStr2xStr(p_rStr);
#else
    XString t_xstr = wStr2xStr(p_rStr);
#endif // USE_ISC_NAMESPACE
    stuffUnicode(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData), t_xstr );
#else
    stuffUnicode(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData), p_rStr );
#endif // WCHAR4
    return p_Iterator;
}

//See comment on operator+ in header.
#if !defined(WCHAR4) && !defined(_WIN32)
const CacheListIterator CacheList::
insert(const CacheListIterator &p_Iterator, const std::basic_string<wchar_t>& p_rStr) {
  M_ASSERT(p_Iterator.m_pCacheList == this);  // Iterator is for this list!
  //This overload exists only on platforms where wchar_t is the same size as unsigned short *and* WString is
  //a basic_string of unsigned short, so this cast should be fine.
  stuffUnicode(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData), reinterpret_cast<const WString&>(p_rStr));
  return p_Iterator;
}
#endif

const CacheListIterator CacheList::
insert(const CacheListIterator &p_Iterator, const LONG p_iVal)
{
    M_ASSERT(p_Iterator.m_pCacheList == this);  // Iterator is for this list!
    if (p_iVal < 0) {
        stuffNegLong(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData), p_iVal);
    } else {
        stuffPosLong(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData), p_iVal);
    }
    return p_Iterator;
}
#ifndef LONG_EQUAL_LONGLONG
const CacheListIterator CacheList::
insert(const CacheListIterator &p_Iterator, const LONGLONG p_lVal)
{
    M_ASSERT(p_Iterator.m_pCacheList == this);  // Iterator is for this list!
    if (p_lVal < 0) {
        stuffNegLong(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData), p_lVal);
    } else {
        stuffPosLong(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData), p_lVal);
    }
    return p_Iterator;
}
#endif
const CacheListIterator CacheList::
insert(const CacheListIterator &p_Iterator, const sysTypes::IEEE64 p_fVal)
{
    M_ASSERT(p_Iterator.m_pCacheList == this);  // Iterator is for this list!
    stuffDouble(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData), p_fVal);
    return p_Iterator;
}

const CacheListIterator CacheList::
insert(const CacheListIterator &p_Iterator, const int p_iVal)
{
    M_ASSERT(p_Iterator.m_pCacheList == this);  // Iterator is for this list!
    if (p_iVal < 0) {
        stuffNegInt(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData), p_iVal);
    } else {
        stuffPosInt(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData), p_iVal);
    }
    return p_Iterator;
}

const CacheListIterator CacheList::
insert(const CacheListIterator &p_Iterator, const CacheNull&)
{
    M_ASSERT(p_Iterator.m_pCacheList == this);  // Iterator is for this list!
    stuffNull(static_cast<size_t>(p_Iterator.m_pbyData - m_pbData));
    return p_Iterator;
}

#ifdef WCHAR4
const CacheListIterator CacheList::
#ifdef USE_ISC_NAMESPACE
insert(const CacheListIterator &p_Iterator, const ISC::XString& p_rStr)
#else
insert(const CacheListIterator &p_Iterator, const XString& p_rStr)
#endif // USE_ISC_NAMESPACE
{
    M_ASSERT(p_Iterator.m_pCacheList == this);  // Iterator is for this list!
    stuffUnicode(p_Iterator.m_pbyData - m_pbData, p_rStr );
    return p_Iterator;
}
#endif // WCHAR4

// += operators
void CacheList::
appendPtr(void* p_pv)
{
    stuffPosLong(m_iLength,reinterpret_cast<UINTPTR>(p_pv));
}


CacheList& CacheList::
operator+=(const CacheList& rhs)
{
    stuffByte(m_iLength, rhs.m_pbData, rhs.m_iLength);
    return *this;
}

CacheList& CacheList::
operator+=(const CacheROList& rhs)
{
    stuffByte(m_iLength, rhs.m_pbData, rhs.m_iLength);
    return *this;
}

CacheList& CacheList::
operator+=(const CountedByteArray& rhs)
{
    stuffByte(m_iLength, rhs.data(), rhs.length());
    return *this;
}

CacheList& CacheList::
operator+=(const AString& rhs)
{
    stuffAscii(m_iLength, rhs);
    return *this;
}

CacheList& CacheList::
operator+=(const UAString& rhs)
{
    stuffAscii(m_iLength,reinterpret_cast<const AChar*>(rhs.data()),static_cast<size_t>(rhs.length()));
    return *this;
}

CacheList& CacheList::
operator+=(const WString& rhs)
{
#ifdef WCHAR4
    // need to convert from 4 byte unicode to 2 byte unicode
#ifdef USE_ISC_NAMESPACE
    ISC::XString t_xstr = wStr2xStr(rhs);
#else
    XString t_xstr = wStr2xStr(rhs);
#endif // USE_ISC_NAMESPACE
    stuffUnicode(m_iLength,t_xstr);
#else
    stuffUnicode(m_iLength, rhs);
#endif
    return *this;
}

CacheList& CacheList::
operator+=(const C16String& rhs)
{
#ifdef WCHAR4
    // need to convert from 4 byte unicode to 2 byte unicode
#ifdef USE_ISC_NAMESPACE
    ISC::XString t_xstr = c16Str2xStr(rhs);
#else
    XString t_xstr = c16Str2xStr(rhs);
#endif // USE_ISC_NAMESPACE
    stuffUnicode(m_iLength, t_xstr);
#else
    stuffUnicode(m_iLength, rhs);
#endif
    return *this;
}

//See comment on operator+ in header.
#if !defined(WCHAR4) && !defined(_WIN32)
CacheList& CacheList::
operator+=(const std::basic_string<wchar_t>& rhs) {
  //This overload exists only on platforms where wchar_t is the same size as unsigned short *and* WString is
  //a basic_string of unsigned short, so this cast should be fine.
  stuffUnicode(m_iLength, reinterpret_cast<const WString&>(rhs));
  return *this;
}
#endif

CacheList& CacheList::
operator+=(const CacheNull&)
{
    stuffNull(m_iLength);
    return *this;
}

CacheList& CacheList::
operator+=(const LONG& rhs)
{
    if (rhs < 0) {
        stuffNegLong(m_iLength, rhs);
    } else {
        stuffPosLong(m_iLength, rhs);
    }
    return *this;
}

CacheList& CacheList::
operator+=(const size_t& rhs)
{
  stuffPosLong(m_iLength, rhs);
  return *this;
}

#ifndef LONG_EQUAL_LONGLONG
CacheList& CacheList::
operator+=(const LONGLONG& rhs)
{
    if (rhs < 0) {
        stuffNegLong(m_iLength, rhs);
    } else {
        stuffPosLong(m_iLength, rhs);
    }
    return *this;
}
#endif
CacheList& CacheList::
operator+=(const sysTypes::IEEE64& rhs)
{
    stuffDouble(m_iLength, rhs);
    return *this;
}

CacheList& CacheList::
operator+=(const int& rhs)
{
    if (rhs < 0) {
        stuffNegInt(m_iLength, rhs);
    } else {
        stuffPosInt(m_iLength, rhs);
    }
    return *this;
}

CacheList& CacheList::
operator+=(const char rhs)
{
    BYTE t_rgby[1]; t_rgby[0]=rhs;
    stuffByte(m_iLength, t_rgby,1);
    return *this;
}

#if defined(WCHAR4)
CacheList& CacheList::
#ifdef USE_ISC_NAMESPACE
operator+=(const ISC::XString& rhs)
#else
operator+=(const XString& rhs)
#endif // USE_ISC_NAMESPACE
{
    stuffUnicode(m_iLength, rhs);
    return *this;
}
#endif

#if 0
const CacheListIterator CacheList::
insert(const CacheListIterator &p_Iterator, const time_t p_tVal)
{
    char        t_rgchWork[ 32 ];
    std::String t_s;
    LONG        t_lSecs;


    // Convert to $Horolog format, adjusting for the timezone
    t_lSecs = p_tVal / 1000;
    t_s = _ltoa( (t_lSecs / 86400) + 47117, t_rgchWork, 10 );
    t_s += _ltoa( t_lSecs % 86400, t_rgchWork, 10 );

    // Store as string
    insert(p_Iterator, t_s);
    return p_Iterator;
}
#endif




//////////////////////////////////////////////////////////////////////////////

sysTypes::IEEE64 CacheROList::parsedblb(BYTE* dat, size_t off, size_t len)
{
    char    digit;      // Holds digit 0-9 under consideration
    BOOL    sign;       // Sign of value
    size_t     decpnt = 0; // Location + 1 of decimal point
    int     scale = 0;

    len += off;     // Put final offset into len
    size_t end = len;      // Save it for later
    sign = FALSE;
    // Skip over + and -
    while (off < end) {
        if ((char)((digit = (char)dat[off++])-'0') <= 9) {
            goto notsgn;
        } else if (digit == '-') {
            sign = !sign;
        } else if (digit == '.') {
            decpnt = off;
            goto notsgn;
        } else if (digit != '+') {
            break;
        }
    }
    return 0.0;

notsgn:

    // Here, value is either '0'-'9', or '.', skip over any leading zeros
    if (digit == '0' || digit == '.') {
        while (off < end) {
            if ((digit = (char)dat[off++]) == '.') {
                if (decpnt != 0) {  // We've already seen a '.'
                break;
                }
                decpnt = off;
            } else if (digit != '0') {
                if ((char)(digit-'0') <= 9) {
                    goto notzer;
                } else {
                    break;
                }
            }
        }
        return 0.0;
    }

notzer:

    AString buf;
    if (sign) {
        buf += '-';
    }
    // Save first non-zero digit
    buf += digit;
    // Now, process digits, and possibly a '.' or 'E'
    while (off < end) {
        if ((char)((digit = (char)dat[off++]) - '0') <= 9) {
            buf += digit;
        } else {
            if (digit == '.') {
                if (decpnt == 0) {
                    decpnt = off;
                    continue;
                }
                end = off-1;
            } else {
                end = off-1;
                if ((digit == 'E' || digit == 'e') && off < len) {
                    BOOL expsign = FALSE;
                    if ((digit = (char)(dat[off++] - '0')) <= 9) {
                        scale = digit;
                    } else if (digit == '-' - '0') {
                        expsign = TRUE;
                    } else if (digit != '+' - '0') {
                        break;
                    }
                    while (off < len
                           && (digit = (char)(dat[off++] - '0')) <= 9) {
		      if (scale > static_cast<int>((GINT/10))
                        || (scale = scale * 10 + digit) < 0) {
#ifndef WIN32
                            throw std::exception();
#else
                            throw std::overflow_error("Double too large");
#endif
                        }
                    }
                    if (expsign) {
                        scale = -scale;
                    }
                }
            }
            break;
        }
    }
    // We need to calculate scale, and build BigDecimal
    if (decpnt != 0) {
      scale -= static_cast<int>((end - decpnt));
    }

    if (scale != 0) {
        utlAutoArray<AChar> t_rgchWork(l2a(scale));
        buf += 'E';
        buf += t_rgchWork.get();
    }


    return atof( buf.c_str() );
}

sysTypes::IEEE64 CacheROList::parsedblu(BYTE* dat, size_t off, size_t len)
{
    char    digit;      // Holds digit 0-9 under consideration
    BOOL    sign;       // Sign of value
    size_t     decpnt = 0; // Location + 1 of decimal point
    int     scale = 0;

    len += off;     // Put final offset into len
    size_t end = len;      // Save it for later
    sign = FALSE;
    // Skip over + and -
    while (off < end && dat[off+1] == 0) {
        digit = (char)dat[off];
        off += 2;
        if ((char)(digit-'0') <= 9) {
            goto notsgn;
        } else if (digit == '-') {
            sign = !sign;
        } else if (digit == '.') {
            decpnt = off;
            goto notsgn;
        } else if (digit != '+') {
            break;
        }
    }
    return 0.0;

notsgn:

    // Here, value is either '0'-'9', or '.', skip over any leading zeros
    if (digit == '0' || digit == '.') {
        while (off < end && dat[off+1] == 0) {
            digit = (char)dat[off];
            off += 2;
            if (digit == '.') {
                if (decpnt != 0) {  // We've already seen a '.'
                    break;
                }
                decpnt = off;
            } else if (digit != '0') {
                if ((char)(digit-'0') <= 9) {
                    goto notzer;
                } else {
                    break;
                }
            }
        }
        return 0.0;
    }

notzer:

    AString buf;

    if (sign) {
        buf += '-';
    }
    // Save first non-zero digit
    buf += digit;
    // Now, process digits, and possibly a '.' or 'E'
    while (off < end) {
        if (dat[off+1] != 0) {
            end = off;
            break;
        }
        digit = (char)dat[off];
        off += 2;
        if ((char)(digit - '0') <= 9) {
            buf += digit;
        } else {
            if (digit == '.') {
                if (decpnt == 0) {
                    decpnt = off;
                    continue;
                }
                end = off-2;
            } else {
                end = off-2;
                if ((digit == 'E' || digit == 'e') && off < len) {
                    BOOL expsign = FALSE;
                    if (dat[off+1] != 0) {
                        break;
                    }
                    digit = (char)(dat[off] - '0');
                    off += 2;
                    if (digit <= 9) {
                        scale = digit;
                    } else if (digit == '-' - '0') {
                        expsign = TRUE;
                    } else if (digit != '+' - '0') {
                        break;
                    }
                    while (off < len
                           && dat[off+1] == 0
                           && (digit = (char)(dat[off] - '0')) <= 9) {
                        off += 2;
                        if (scale > static_cast<int>((GINT/10))
                        || (scale = scale * 10 + digit) < 0) {
#ifndef WIN32
                            throw std::exception();
#else
                            throw std::overflow_error("Double too large");
#endif
                        }
                    }
                    if (expsign) {
                        scale = -scale;
                    }
                }
            }
            break;
        }
    }
    // We need to calculate scale
    if (decpnt != 0) {
      scale -= static_cast<int>((end - decpnt));
    }

    if (scale != 0) {
        utlAutoArray<AChar> t_rgchWork(l2a(scale));
        buf += 'E';
        buf += t_rgchWork.get();
    }


    return atof( buf.c_str() );
}


int CacheROList::parsenumb(BYTE* dat, size_t off, size_t len, LONGLONG &retnum)
{
    unsigned char    digit;      // Holds digit 0-9 under consideration
    int     value;      // Holds current integer value
    BOOL    sign;       // Sign of value

    if (len == 0) {
        retnum = 0; // Return a zero
        return 0;
    }
    len += off;     // Put final offset into len
    sign = FALSE;
    // Loop over +,- signs, look for . or digit
    for (;;) {
        if ((unsigned char)(value = dat[off++] - '0') <= 9) { // Branch if digit
            while (off < len) { // Process additional integer digits
                if ((digit = (unsigned char)(dat[off++] - '0')) > 9) {
                    // No more integers, done if no more chars
                    if (off >= len) {
                        break;
                    }
                    if (digit == (unsigned char)('.' - '0')) {
                        // Have 1 word integer & just scanned a .
                        // Convert to DOUBLE precision & process decimal digits
                        return parsefracb(dat,off,len,value,0,sign,retnum);
                    }
                    if (digit == (unsigned char)('E' - '0') ||
                        digit == (unsigned char)('e' - '0')) {
                        // Have exponent w/ single precision #
                        return parseexpb(dat,off,len,value,0,sign,retnum);
                    }
                    break;
                }
                if (value >= (0x7FFFFFFF/10)) { // Check for overflow
                    // Overflowed single precision while scanning integer digits
                    LONGLONG dbl = (LONG)value * 10 + digit;
                    int dscale = 0;
                    while (off < len
                           && (digit = (unsigned char)(dat[off++] - '0')) <= 9) {
                        if (dbl > CONGNEGD10
                        || (dbl == CONGNEGD10 && digit > 7)) {
                            // Overflowed dbl precision
                            do {
                                ++dscale;
                            } while (off < len
                                 && (digit = (unsigned char)(dat[off++]-'0')) <= 9);
                            break;
                        }
                        dbl = dbl * 10 + digit;
                    }
                    if (off < len) {
                        // Finished integer digits w/ DOUBLE fraction
                        if (digit == (unsigned char)('.' - '0')) {
                            // Convert to DOUBLE precision
                            // and process decimal digits
                            return parsefracb(dat,off,len,dbl,dscale,sign,retnum);
                        }
                        // Handle exponent
                        if (digit == (unsigned char)('E' - '0') ||
                            digit == (unsigned char)('e' - '0')) {
                            return parseexpb(dat,off,len,dbl,dscale,sign,retnum);
                        }
                    }
                    // Return DOUBLE precision #
                    return scale(dbl,dscale,sign,retnum);
                }
                value = value * 10 + digit;
            }   // End of while
            retnum = sign ? -value : value; // Return an integer
            return 0;
        }   // End of if
        if (off == len) {
            break;
        } else if (value == ('-' - '0')) {
            sign = !sign;
        } else if (value == ('.' - '0')) {
            return parsefracb(dat,off,len,0,0,sign,retnum);
        } else if (value != ('+' - '0')) {
            break;
        }
    }
    retnum = 0; // Return a zero


    return 0;
}


// Process fractional digits
int CacheROList::parsefracb(BYTE* dat, size_t off, size_t end, LONGLONG dbl, int dscale,
                            BOOL sign, LONGLONG &retnum)
{
    char    digit;      // Holds digit 0-9 under consideration

    do {
        if ((digit = (char)(dat[off++] - '0')) > 9) {
            if ((digit != (char)('E'-'0') && digit != (char)('e'-'0')) || off >= end) {
                break;
            }
            return parseexpb(dat,off,end,dbl,dscale,sign,retnum);
        }
        if (dbl > CONGNEGD10 || (dbl == CONGNEGD10 && digit > 7)) {
            // Overflow of dbl prec # while scanning decimal digits
            while (off < end) {
                if ((digit = (char)(dat[off++] - '0')) > 9) {
                    if (off >= end ||
                       (digit != (char)('E'-'0') && digit != (char)('e'-'0'))) {
                        break;
                    }
                    return parseexpb(dat,off,end,dbl,dscale,sign,retnum);
                }
            }
            break;
        }
        dbl = dbl * 10 + digit;
        --dscale;       // Account for digit in scale
    } while (off < end);

    // Return DOUBLE precision #
    return scale(dbl,dscale,sign,retnum);
}

// Process exponent
int CacheROList::parseexpb(BYTE* dat, size_t off, size_t end, LONGLONG dbl, int dscale,
                           BOOL sign, LONGLONG &retnum)
{
    int exp;
    BOOL    expsign;
    char    digit;

    // Don't bother parsing any more if DOUBLE is 0
    if (dbl == 0) {
        retnum = 0;
        return 0;
    }
    expsign = FALSE;        // Sign of exponent
    if ((char)(exp = dat[off++] - '0') > 9) {
        if (exp == '-' - '0') {
            expsign = TRUE;
        } else if (exp != '+' - '0') {
            off = end;
        }
        exp = 0;
    }
    while (off < end) { // Process additional exponent digits
        if ((digit = (char)(dat[off++] - '0')) > 9) {
            break;
        }
        if (exp > static_cast<int>((GINT/10)) || (exp = exp * 10 + digit) < 0) {
            break;
        }
    }
    if (exp > (128+(19*2))) {
        if (!expsign) {
#ifndef WIN32
            throw std::exception();
#else
            throw std::overflow_error("Exponent too large");
#endif
        }
        retnum = 0; // Return 0
        return 0;
    } else {
        return scale(dbl, dscale + (expsign ? -exp : exp), sign, retnum);
    }
}


// Process fractional digits
int CacheROList::parsefracu(BYTE* dat, size_t off, size_t end, LONGLONG dbl, int dscale,
                            BOOL sign, LONGLONG &retnum)
{
    char    digit;      // Holds digit 0-9 under consideration

    do {
        if (dat[off+1] != 0) {
            break;
        }
        if ((digit = (char)(dat[off] - '0')) > 9) {
            off += 2;
            if ((digit != (char)('E'-'0') && digit != (char)('e'-'0')) || off >= end) {
                break;
            }
            return parseexpu(dat,off,end,dbl,dscale,sign,retnum);
        }
        off += 2;
        if (dbl > CONGNEGD10 || (dbl == CONGNEGD10 && digit > 7)) {
            // Overflow of dbl prec # while scanning decimal digits
            while (off < end && dat[off+1] == 0) {
                if ((digit = (char)(dat[off] - '0')) > 9) {
                    off += 2;
                    if (off >= end ||
                       (digit != (char)('E'-'0') && digit != (char)('e'-'0'))) {
                        break;
                    }
                    return parseexpu(dat,off,end,dbl,dscale,sign,retnum);
                }
                off += 2;
            }
            break;
        }
        dbl = dbl * 10 + digit;
        --dscale;       // Account for digit in scale
    } while (off < end);
    // Return DOUBLE precision #
    return scale(dbl,dscale,sign,retnum);
}

// Process exponent
int CacheROList::parseexpu(BYTE* dat, size_t off, size_t end, LONGLONG dbl, int dscale,
                           BOOL sign, LONGLONG &retnum)
{
    int exp = 0;
    BOOL    expsign;
    char    digit;

    // Don't bother parsing any more if DOUBLE is 0
    if (dbl == 0) {
        retnum = 0;
        return 0;
    }
    expsign = FALSE;        // Sign of exponent
    if (dat[off+1] != 0) {
        off = end;  // Found Unicode character
    } else if ((char)(exp = dat[off] - '0') > 9) {
        // Check for - or +
        off += 2;
        if (exp == '-' - '0') {
            expsign = TRUE;
        } else if (exp != '+' - '0') {
            off = end;
        }
        exp = 0;
    } else off += 2;
    // Process additional exponent digits
    while (off < end
           && (digit = (char)(dat[off] - '0')) <= 9
           && dat[off+1] == 0
           && exp <= static_cast<int>((GINT/10))
           && (exp = exp * 10 + digit) > 0) {
        off += 2;
    }
    if (exp > (128+(19*2))) {
        if (!expsign) {
#ifndef WIN32
            throw std::exception();
#else
            throw std::out_of_range("Exponent too large");
#endif
        }
        retnum = 0; // Return 0
        return 0;
    } else {
        return scale(dbl, dscale + (expsign ? -exp : exp), sign, retnum);
    }
}

// Assumes that len is always even!
int CacheROList::parsenumu(BYTE* dat, size_t off, size_t len, LONGLONG &retnum)
{
    char    digit;      // Holds digit 0-9 under consideration
    int     value;      // Holds current integer value
    BOOL    sign;       // Sign of value

    if (len == 0) {
        retnum = 0; // Return a zero
        return 0;
    }
    len += off;     // Put final offset into len
    sign = FALSE;
    // Loop over +,- signs, look for . or digit
    for (;;) {
        if (dat[off+1] != 0) {
            break;
        }
        if ((char)(value = dat[off] - '0') <= 9) {
            off += 2;
            // Process additional integer digits
            while (off < len && dat[off+1] == 0) {
                if ((digit = (char)(dat[off] - '0')) > 9) {
                    off += 2;
                    // No more integers, done if no more chars
                    if (digit == (char)('.' - '0')) {
                        // Have 1 word integer & just scanned a .
                        // Convert to DOUBLE precision & process decimal digits
                        return parsefracu(dat,off,len,value,0,sign,retnum);
                    }
                    if (digit == (char)('E' - '0') ||
                        digit == (char)('e' - '0')) {
                        // Have exponent w/ single precision #
                        return parseexpu(dat,off,len,value,0,sign,retnum);
                    }
                break;
                }
                off += 2;
                if (value >= (0x7FFFFFFF/10)) { // Check for overflow
                    // Overflowed single precision while scanning digits
                    LONGLONG dbl = (LONG)value*10+digit;
                    int dscale = 0;
                    while (off < len
                           && (digit = (char)(dat[off] - '0')) <= 9
                           && dat[off+1]==0) {
                        if (dbl > CONGNEGD10
                        || (dbl == CONGNEGD10 && digit > 7)) {
                            // Overflowed dbl precision while on digit
                            do {
                                ++dscale;
                                off += 2;
                            } while (off < len
                                 && (digit=(char)(dat[off]-'0')) <= 9
                                 && dat[off+1]==0);
                            break;
                        }
                        off += 2;
                        dbl = dbl * 10 + digit;
                    }
                    if (off < len && dat[off+1] == 0) {
                        off += 2;
                        // Finished integer digits w/ DOUBLE fraction
                        if (digit == (char)('.' - '0')) {
                            // Convert to DOUBLE precision & process decimal digits
                            return parsefracu(dat,off,len,dbl,dscale,sign,retnum);
                        }
                        // Handle exponent
                        if (digit == (char)('E' - '0') ||
                            digit == (char)('e' - '0')) {
                            return parseexpu(dat,off,len,dbl,dscale,sign,retnum);
                        }
                    }
                    // Return DOUBLE precision #
                    return scale(dbl,dscale,sign,retnum);
                }
                value = value * 10 + digit;
            }   // End of while
            retnum = sign ? -value : value; // Return an integer
            return 0;
        }
        if ((off += 2) >= len) {
            break;
        } else if (value == ('-' - '0')) {
            sign = !sign;
        } else if (value == ('.' - '0')) {
            return parsefracu(dat,off,len,0,0,sign,retnum);
        } else if (value != ('+' - '0')) {
            break;
        }
    }
    retnum = 0; // Return a zero
    return 0;
}

int CacheROList::scale(LONGLONG value, int scale, BOOL sign, LONGLONG& retnum)
{
    if (value == 0) {
        scale = 0;
    } else if (scale > 127) {       // scale too large
        do {
            if (value > CONGNEGD10) {
#ifndef WIN32
                throw std::exception();
#else
                throw std::out_of_range("Exponent too large");
#endif
            }
            value *= 10;
        } while (--scale > 127);
    } else if (scale < -128) {  // scale too negative
        if (scale < (-128-19)) {
            value = 0;
            scale = 0;
        } else {
            if (value < 0) {
                value = CONGNEGD10;
                while (++scale < -128) {
                    value /= 10;
                }
            } else {
                // Reduce scale by 100's, then by a single 1
                do {
                    value /= 10;
                } while (++scale < -128 && value != 0);
            }
            if (value == 0) {
                scale = 0;
            }
        }
    }
    retnum = sign ? -value : value;
    return static_cast<int>(scale);
}

int CacheROList::ieee2int(BYTE* pData,size_t p_iLen)
{
    // Avoid alignment issues
    Align t_Align;
    sysMemCpy(t_Align.m_rgcBuffer,pData,p_iLen);

#if (defined(_ISC_BIGENDIAN) && _ISC_BIGENDIAN)
    if (sizeof(sysTypes::UINT64) == p_iLen) {
        t_Align.m_uint64 = SWAP64(t_Align.m_uint64);
    } else {
        t_Align.m_uint32 = SWAP32(t_Align.m_uint32);
    }
#endif

    return t_Align.m_int;
}

int CacheROList::dblint(int scale, LONGLONG num)
{
    if (scale < 0) {
        if (scale < -18) {
            return 0;
        }
        num /= s_rgscaletab[-scale];
        return (num != (int)num) ? ((num >= 0) ? GINT : GNEGI) : (int)num;
    }
    // Make sure # can be represented as integer
    if (num != (int)num) {
        return (num >= 0) ? GINT : GNEGI;
    }
    // scale >= 0
    if (scale > 0 && num != 0) {
        do {
	  if (num > static_cast<int>((GINT/10))) {
                return GINT;
	  } else if (num < static_cast<int>((GNEGI/10))) {
                return GNEGI;
            }
            num *= 10;
        } while (--scale != 0);
    }
    return (int)num;
}

LONGLONG CacheROList::ieee2long(BYTE* pData,size_t p_iLen)
{
    // Avoid alignment issues
    Align t_Align;
    sysMemCpy(t_Align.m_rgcBuffer,pData,p_iLen);

#if (defined(_ISC_BIGENDIAN) && _ISC_BIGENDIAN)
    if (sizeof(sysTypes::UINT64) == p_iLen) {
        t_Align.m_uint64 = SWAP64(t_Align.m_uint64);
    } else {
        t_Align.m_uint32 = SWAP32(t_Align.m_uint32);
    }
#endif
    return t_Align.m_longlong;
}

LONGLONG CacheROList::dbllong(int scale, LONGLONG num)
{
    if (scale < 0) {
        return (scale < -18) ? 0 : (num / s_rgscaletab[-scale]);
    }
    // scale >= 0
    if (scale > 0 && num != 0) {
        do {
            if (num > (MAKELONGLONG(0x7FFFFFFFFFFFFFFF)/10)) {
                return MAKELONGLONG(0x7FFFFFFFFFFFFFFF);
            } else if (num < (LONGLONG)(MAKELONGLONG(0x8000000000000000)/10)) {
                return MAKELONGLONG(0x8000000000000000);
            }
            num *= 10;
        } while (--scale != 0);
    }
    return num;
}

AString CacheROList::ieee2str(BYTE* pData, size_t p_iLen)
{
    char t_rgchWork[64];
    return AString(gcvt(ieee2dbl(pData,p_iLen), 60, t_rgchWork ));
}

AString CacheROList::dblstr(int scale, LONGLONG num)
{
    int pos;
    int end;
    int len;
    char    buf[150];
    BOOL    negative = (num < 0);

    pos = end = 150;
    if (num == 0) {
        return AString("0");
    }
    if (num == _LONG64_MIN) {
        // -2 to remove the '-' and the 'L'
        pos = (int)(end-(sysMCHARzStrLen(s_rglonglongmin)-2));
        sysMemCpy(reinterpret_cast<BYTE*>(&buf[pos]), reinterpret_cast<BYTE*>(&s_rglonglongmin[1]), sysMCHARzStrLen(s_rglonglongmin)-2 );
    } else {
        if (negative) {
            num = -num;
        }
        int val = (int)num;
        if (val == num) {
            do {
                int digit = (int)(val % 100);
                buf[--pos] = s_rgonestab[digit];
                buf[--pos] = s_rgtenstab[digit];
                val /= 100;
            } while (val != 0);
        } else {
            do {
                int digit = (int)(num % 100);
                buf[--pos] = s_rgonestab[digit];
                buf[--pos] = s_rgtenstab[digit];
                num /= 100;
            } while (num != 0);
        }
        if (buf[pos] == '0') {
            pos++;
        }
    }
    if (scale > 0) {
        // Shuffle down array, add trailing zeros
        sysMemMove(reinterpret_cast<BYTE*>(&buf[1]), reinterpret_cast<BYTE*>(&buf[pos]), end -= pos );
        pos = 1;
        end++;
        do {
            buf[end++] = '0';
        } while (--scale > 0);
    } else if (scale < 0) {
        // Soak up trailing zeros
        scale = -scale;
        do {
            if (buf[--end] != '0') {
                // No more zeros - need to replace
                if ((len = (end + 1 - pos)) > scale) {
                    // [len-scale].[scale]
                    sysMemMoveLeft(reinterpret_cast<BYTE*>(&buf[pos-1]), reinterpret_cast<BYTE*>(&buf[pos]), len-scale);
                    buf[end-scale] = '.';
                    --pos;
                } else {
                    // .[scale-len 0's][len digits]
                    if ((scale -= len) != 0) {
                        do {
                        buf[--pos] = '0';
                        } while (--scale != 0);
                    }
                    buf[--pos] = '.';
                }
                end++;
                break;
            }
        } while (--scale != 0);
    }
    if (negative) {
        buf[--pos] = '-';
    }

    return AString(&buf[pos], end - pos);
}

sysTypes::IEEE32 CacheROList::ieee2flt(BYTE* pData, size_t p_iLen)
{
    // Avoid alignment issues
    Align t_Align;
    sysMemCpy(t_Align.m_rgcBuffer,pData,p_iLen);

#if (defined(_ISC_BIGENDIAN) && _ISC_BIGENDIAN)
    if (sizeof(sysTypes::UINT64) == p_iLen) {
        t_Align.m_uint64 = SWAP64(t_Align.m_uint64);
    } else {
        t_Align.m_uint32 = SWAP32(t_Align.m_uint32);
    }
#endif

    return t_Align.m_ieee32;
}

sysTypes::IEEE64 CacheROList::ieee2dbl(BYTE* pData, size_t p_iLen)
{
    // Avoid alignment issues
    Align t_Align;
    sysMemCpy(t_Align.m_rgcBuffer,pData,p_iLen);

#if (defined(_ISC_BIGENDIAN) && _ISC_BIGENDIAN)
    if (sizeof(sysTypes::UINT64) == p_iLen) {
        t_Align.m_uint64 = SWAP64(t_Align.m_uint64);
    } else {
        t_Align.m_uint32 = SWAP32(t_Align.m_uint32);
    }
#endif

    return t_Align.m_ieee64;
}

sysTypes::IEEE64 CacheROList::dbl2dbl(int scale, LONGLONG num)
{
    int pos;
    int end;
    char    buf[32];	/* -<19>E-<3>0 */

    if (!num) return 0.0;
    pos = end = 26;
    if (num < 0) {
	num = -num;
	if (num < 0) {	/* If still negative, greatest negative # */
           // -2 to remove the '-' and the 'L'
	   pos = (int)(end-(sysMCHARzStrLen(s_rglonglongmin)-2));
	   sysMemCpy(reinterpret_cast<BYTE*>(&buf[pos]),
		     reinterpret_cast<BYTE*>(&s_rglonglongmin[1]),
		     sysMCHARzStrLen(s_rglonglongmin)-2 );
	} else {
	   while (num > INT_MAX) {
              int digit = (int)(num % 100);
	      buf[--pos] = s_rgonestab[digit];
	      buf[--pos] = s_rgtenstab[digit];
	      num /= 100;
	   }
	   int val = (int)num;
	   do {
	      int digit = (int)(val % 100);
	      buf[--pos] = s_rgonestab[digit];
	      buf[--pos] = s_rgtenstab[digit];
	      val /= 100;
	   } while (val != 0);
	   if (buf[pos] == '0') pos++;
	}
	buf[--pos] = '-';
    } else {
        while (num > INT_MAX) {
            int digit = (int)(num % 100);
            buf[--pos] = s_rgonestab[digit];
            buf[--pos] = s_rgtenstab[digit];
            num /= 100;
        }
        int val = (int)num;
        do {
            int digit = (int)(val % 100);
            buf[--pos] = s_rgonestab[digit];
            buf[--pos] = s_rgtenstab[digit];
            val /= 100;
        } while (val != 0);
        if (buf[pos] == '0') pos++;
    }
    // Output scale (in range -128 to 127)
    if (scale) {
        buf[end++] = 'E';
        if (scale < 0) {
            buf[end++] = '-';
            scale = -scale;
        }
        if (scale >= 100) {
            buf[end++] = '1';
            scale -= 100;
        }
        if (scale >= 10) buf[end++] = s_rgtenstab[scale];
        buf[end++] = s_rgonestab[scale];
    }
    buf[end] = 0;

    return atof( buf + pos );
}

AString CacheROList::lngstr(LONGLONG num)
{

    char buf[20];

    if ((int)num == num) {
        utlAutoArray<AChar> t_buf(i2a((int) num));
        return AString( t_buf.get() );
    }
    if (num == _I64_MIN) {
        // -2 to remove the '-' and the 'L'
        return AString(&s_rglonglongmin[1], sysMCHARzStrLen(s_rglonglongmin)-2);
    }
    int pos = 20;

    if (num < 0) {
        num = -num;
        do {
            int digit = (int)(num % 100);
            buf[--pos] = s_rgonestab[digit];
            buf[--pos] = s_rgtenstab[digit];
            num /= 100;
        } while (num != 0);
        if (buf[pos] != '0') {
            --pos;
        }
        buf[pos] = '-';
    } else {
        do {
            int digit = (int)(num % 100);
            buf[--pos] = s_rgonestab[digit];
            buf[--pos] = s_rgtenstab[digit];
            num /= 100;
        } while (num != 0);
        if (buf[pos] == '0') {
            pos++;
        }
    }
    return AString(&buf[pos], 20 - pos);
}

// Return a pointer to the actual data
const BYTE* CacheListItem::
data() const
{
    if (m_rgbyData[0]) return &m_rgbyData[2];			// KMK470+
    else if (m_rgbyData[1] || m_rgbyData[2]) return &m_rgbyData[4];
    else return &m_rgbyData[8];					// KMK470-
}

// Return the length of the data item
size_t CacheListItem::
datalength() const
{
    size_t nLen = m_rgbyData[0];
    if (nLen) {

        // Special case for a list element
        // with a <null value>
        if (nLen == 1) {
            // this should really be an error,
            // a <null value> has no length
            return 0;
        }
        return nLen - 2;
    }
    // Get the 2-byte length.
    nLen = m_rgbyData[1] | m_rgbyData[2] << 8;			// KMK470+
    if (!nLen) {
	nLen =
	    m_rgbyData[3] |
	    m_rgbyData[4] << 8 |
	    m_rgbyData[5] << 16 |
	    m_rgbyData[6] << 24;
    }
    return nLen - 1;						// KMK470-
}

// Return the actual size of the list element including data len
// and type info
size_t CacheListItem::
size() const
{
    if (m_rgbyData[0]) return m_rgbyData[0];
    size_t len = m_rgbyData[1] | m_rgbyData[2] << 8;		// KMK470+
    if (len) return len + 3;
    return (m_rgbyData[3]
	| m_rgbyData[4] << 8
	| m_rgbyData[5] << 16
	| m_rgbyData[6] << 24)
      + 7;							// KMK470-
}

// Return the type of the item
CacheList::CacheTypes CacheListItem::
Type() const
{
    size_t nLen = m_rgbyData[0];
    if (nLen) {
        if (nLen == 1) {
            return CacheList::CT_UNDEF;
        }
        return CacheList::CacheTypes(m_rgbyData[1]);
    }
    if (m_rgbyData[1] || m_rgbyData[2])				// KMK470+
        return CacheList::CacheTypes(m_rgbyData[3]);
    return CacheList::CacheTypes(m_rgbyData[7]);		// KMK470-
}

// Private function to return the type and set the start position and
// length of the data.
CacheList::CacheTypes CacheListItem::
getType(BYTE* & p_pData, size_t& p_iLen) const
{
    size_t nLen = m_rgbyData[0];
    if (nLen) {
        p_pData = const_cast<BYTE*>(&m_rgbyData[2]);
        if (nLen == 1) {
            p_iLen = 0;
            return CacheList::CT_UNDEF;
        }
        p_iLen = nLen - 2;
        return CacheList::CacheTypes(m_rgbyData[1]);
    }
    nLen = m_rgbyData[1] | m_rgbyData[2] << 8;			// KMK470+
    if (nLen) {
	p_iLen = nLen - 1;
	p_pData = const_cast<BYTE*>(&m_rgbyData[4]);
	return CacheList::CacheTypes(m_rgbyData[3]);
    } else {
	nLen = m_rgbyData[3]
	    | m_rgbyData[4] << 8
	    | m_rgbyData[5] << 16 |
	    m_rgbyData[6] << 24;
	p_iLen = nLen - 1;
	p_pData = const_cast<BYTE *>(&m_rgbyData[8]);
	return CacheList::CacheTypes(m_rgbyData[7]);
    }
}


AString CacheListItem::
AsAString() const
{
    BYTE* t_pData;
    size_t t_iLen;

    switch (getType(t_pData, t_iLen)) {
    case CacheList::CT_UNDEF:
        return AString();
    case CacheList::CT_ASCII:
        return getAscii(t_pData, t_iLen);

    case CacheList::CT_POSINT:
        return CacheList::lngstr(getPosLongLong(t_pData, t_iLen));

    case CacheList::CT_NEGINT:
        return CacheList::lngstr(getNegLongLong(t_pData, t_iLen));

    case CacheList::CT_POSREAL:
        return CacheList::dblstr(*(char *)t_pData, getPosLongLong(t_pData + 1, t_iLen - 1));

    case CacheList::CT_NEGREAL:
        return CacheList::dblstr(*(char *)t_pData, getNegLongLong(t_pData + 1, t_iLen -1 ));

    case CacheList::CT_UNICODE:
        return xStr2aStr(getXString(t_pData,t_iLen));

    case CacheList::CT_DOUBLE:
        return CacheList::ieee2str(t_pData,t_iLen);

    default:
#ifndef WIN32
        throw std::exception();
#else
        throw std::invalid_argument("List Format Error");
#endif
    }
}


inline static UAString ToUAString(const AString& p_cras) {
    return UAString(reinterpret_cast<const UAChar*>(p_cras.c_str()));
}

UAString CacheListItem::
AsUAString() const
{
    BYTE* t_pData;
    size_t t_iLen;

    switch (getType(t_pData, t_iLen)) {

    case CacheList::CT_UNDEF:
        return UAString();

    case CacheList::CT_ASCII:
        return getUAscii(t_pData, t_iLen);

    case CacheList::CT_POSINT:
        return ToUAString(CacheList::lngstr(getPosLongLong(t_pData, t_iLen)));

    case CacheList::CT_NEGINT:
        return ToUAString(CacheList::lngstr(getNegLongLong(t_pData, t_iLen)));

    case CacheList::CT_POSREAL:
        return ToUAString(CacheList::dblstr(*(char *)t_pData, getPosLongLong(t_pData + 1, t_iLen - 1)));

    case CacheList::CT_NEGREAL:
        return ToUAString(CacheList::dblstr(*(char *)t_pData, getNegLongLong(t_pData + 1, t_iLen -1 )));

    case CacheList::CT_UNICODE:
        return ToUAString(xStr2aStr(getXString(t_pData,t_iLen)));

    case CacheList::CT_DOUBLE:
        return ToUAString(CacheList::ieee2str(t_pData,t_iLen));

    default:
#ifndef WIN32
        throw std::exception();
#else
        throw std::invalid_argument("List Format Error");
#endif
    }
}

WString CacheListItem::
AsWString() const
{
    BYTE*   t_pData;
    size_t     t_iLen;
    AString t_str;

    switch (getType(t_pData, t_iLen)) {
    case CacheList::CT_UNDEF:
        return WString();

    case CacheList::CT_ASCII:
        t_str = getAscii(t_pData, t_iLen);
        break;

    case CacheList::CT_UNICODE:
        return getUnicode(t_pData, t_iLen);

    case CacheList::CT_POSINT:
        t_str = CacheList::lngstr(getPosLongLong(t_pData, t_iLen));
        break;

    case CacheList::CT_NEGINT:
        t_str = CacheList::lngstr(getNegLongLong(t_pData, t_iLen));
        break;

    case CacheList::CT_POSREAL:
        t_str = CacheList::dblstr(*(char *)t_pData, getPosLongLong(t_pData + 1, t_iLen - 1));
        break;

    case CacheList::CT_NEGREAL:
        t_str = CacheList::dblstr(*(char *)t_pData, getNegLongLong(t_pData + 1, t_iLen - 1));
        break;

    case CacheList::CT_DOUBLE:
        t_str = CacheList::ieee2str(t_pData,t_iLen);
        break;

    default:
#ifndef WIN32
        throw std::exception();
#else
        throw std::invalid_argument("List Format Error");
#endif
    }

    return a2wStr(t_str.c_str(), t_str.size());
}

C16String CacheListItem::
AsC16String() const
{
    BYTE*   t_pData;
    size_t     t_iLen;
    AString t_str;

    switch (getType(t_pData, t_iLen)) {
    case CacheList::CT_UNDEF:
        return C16String();

    case CacheList::CT_ASCII:
        t_str = getAscii(t_pData, t_iLen);
        break;

    case CacheList::CT_UNICODE:
        return getC16Unicode(t_pData, t_iLen);

    case CacheList::CT_POSINT:
        t_str = CacheList::lngstr(getPosLongLong(t_pData, t_iLen));
        break;

    case CacheList::CT_NEGINT:
        t_str = CacheList::lngstr(getNegLongLong(t_pData, t_iLen));
        break;

    case CacheList::CT_POSREAL:
        t_str = CacheList::dblstr(*(char *)t_pData, getPosLongLong(t_pData + 1, t_iLen - 1));
        break;

    case CacheList::CT_NEGREAL:
        t_str = CacheList::dblstr(*(char *)t_pData, getNegLongLong(t_pData + 1, t_iLen - 1));
        break;

    case CacheList::CT_DOUBLE:
        t_str = CacheList::ieee2str(t_pData,t_iLen);
        break;

    default:
#ifndef WIN32
        throw std::exception();
#else
        throw std::invalid_argument("List Format Error");
#endif
    }

    return a2c16Str(t_str.c_str(), t_str.size());
}

#ifndef CACHE_COM_DISABLE
COM_BSTR CacheListItem::
AsBSTR() const
{
    WString t_str = AsWString();

#if defined(_WIN32) | defined (_WIN64)
    return SysAllocStringLen(t_str.c_str(), static_cast<UINT>(t_str.size()));
#else
    return COM_SysAllocStringLen(t_str.c_str(), t_str.size());
#endif
}
#endif  // CACHE_COM_DISABLE

LONG CacheListItem::
AsLong() const
{
    BYTE*       t_pData;
    size_t         t_iLen;
    LONGLONG    t_iNum;
    size_t         t_i;

    switch (getType(t_pData, t_iLen)) {
    case CacheList::CT_UNDEF:
        return 0;
    case CacheList::CT_ASCII:   // Parse an integer from the ASCII data in the buffer
        t_i = CacheList::parsenumb(t_pData, 0, t_iLen, t_iNum);
        return CacheList::dblint(static_cast<int>(t_i), t_iNum);
    case CacheList::CT_UNICODE: // Parse an integer from the Unicode data in the buffer
        if ((t_iLen & 1) != 0) {
#ifndef WIN32
            throw std::exception();
#else
            throw std::invalid_argument("List Format Error");
#endif
        }
        t_i = CacheList::parsenumu(t_pData, 0, t_iLen, t_iNum);
        return CacheList::dblint(static_cast<int>(t_i), t_iNum);
    case CacheList::CT_POSINT:  // Positive integer
        return CacheList::dblint(0, getPosLongLong(t_pData, t_iLen));
    case CacheList::CT_NEGINT:  // Negative integer
        return CacheList::dblint(0, getNegLongLong(t_pData, t_iLen));
    case CacheList::CT_POSREAL: // Positive floating point number
        return CacheList::dblint(*(char *)t_pData, getPosLongLong(t_pData + 1, t_iLen - 1));
    case CacheList::CT_NEGREAL: // Negative floating point number
        return CacheList::dblint(*(char *)t_pData, getNegLongLong(t_pData + 1, t_iLen - 1));
    case CacheList::CT_DOUBLE:
        return static_cast<LONG>(CacheList::ieee2long(t_pData,t_iLen));
    default:
#ifndef WIN32
        throw std::exception();
#else
        throw std::invalid_argument("List Format Error");
#endif
    }
}

LONGLONG CacheListItem::
AsLongLong() const
{
    BYTE*       t_pData;
    size_t         t_iLen;
    LONGLONG    t_iNum;
    size_t         t_i;

    switch (getType(t_pData, t_iLen)) {
    case CacheList::CT_UNDEF:
        return 0;
    case CacheList::CT_ASCII:   // Parse an integer from the ASCII data in the buffer
        t_i = CacheList::parsenumb(t_pData, 0, t_iLen, t_iNum);
        return CacheList::dbllong(static_cast<int>(t_i), t_iNum);
    case CacheList::CT_UNICODE: // Parse an integer from the Unicode data in the buffer
        if ((t_iLen & 1) != 0) {
#ifndef WIN32
            throw std::exception();
#else
            throw std::invalid_argument("List Format Error");
#endif
        }
        t_i = CacheList::parsenumu(t_pData, 0, t_iLen, t_iNum);
        return CacheList::dbllong(static_cast<int>(t_i), t_iNum);
    case CacheList::CT_POSINT:  // Positive integer
        return CacheList::dbllong(0, getPosLongLong(t_pData, t_iLen));
    case CacheList::CT_NEGINT:  // Negative integer
        return CacheList::dbllong(0, getNegLongLong(t_pData, t_iLen));
    case CacheList::CT_POSREAL: // Positive floating point number
        return CacheList::dbllong(*(char *)t_pData, getPosLongLong(t_pData + 1, t_iLen - 1));
    case CacheList::CT_NEGREAL: // Negative floating point number
        return CacheList::dbllong(*(char *)t_pData, getNegLongLong(t_pData + 1, t_iLen - 1));
    case CacheList::CT_DOUBLE:
        return CacheList::ieee2long(t_pData,t_iLen);

    default:
#ifndef WIN32
        throw std::exception();
#else
        throw std::invalid_argument("List Format Error");
#endif
    }
}

BOOL CacheListItem::
AsBool() const
{
    return static_cast<BOOL>(AsLong());
}

sysTypes::IEEE64 CacheListItem::
AsDouble() const
{
    BYTE*       t_pData;
    size_t         t_iLen;

    switch (getType(t_pData, t_iLen)) {
    case CacheList::CT_UNDEF:
        return 0;
    case CacheList::CT_ASCII:   // Parse a DOUBLE from the ASCII data
        return CacheList::parsedblb(t_pData, 0, t_iLen);
    case CacheList::CT_UNICODE: // Parse a DOUBLE from the Unicode data
        if ((t_iLen & 1) != 0) {
#ifndef WIN32
            throw std::exception();
#else
            throw std::invalid_argument("List Format Error");
#endif
        }
        return CacheList::parsedblu(t_pData, 0, t_iLen);
    case CacheList::CT_POSINT:  // Positive integer
        return (sysTypes::IEEE64)getPosLongLong(t_pData, t_iLen);
    case CacheList::CT_NEGINT:  // Negative integer
        return (sysTypes::IEEE64)getNegLongLong(t_pData, t_iLen);
    case CacheList::CT_POSREAL: // Positive floating point number
        return CacheList::dbl2dbl(*(char *)t_pData, getPosLongLong(t_pData + 1, t_iLen - 1));
    case CacheList::CT_NEGREAL: // Negative floating point number
        return CacheList::dbl2dbl(*(char *)t_pData, getNegLongLong(t_pData + 1, t_iLen - 1));
    case CacheList::CT_DOUBLE:
        return CacheList::ieee2dbl(t_pData,t_iLen);
    default:
        {
            // CT_PLACEHOLDER and CT_XUNICODE are not used
        }
    }

    // To prevent C4715 warnings ...
    return 0.0;
}

LONG CacheListItem::
AsDateTime() const
{
    BYTE*       t_pData;
    size_t         t_iLen;
    int         scale;
    LONGLONG    d = 0;
    LONGLONG    t = 0;
    LONGLONG    t_iNum;

    switch (getType(t_pData, t_iLen)) {
    case CacheList::CT_UNDEF:
        return 0;
    case CacheList::CT_ASCII:   // A string consists of 'ddddd[,sssss]'
        if (t_iLen != 0) {
            scale = CacheList::parsenumb(t_pData, 0, t_iLen, t_iNum);
            d = CacheList::dbllong(scale, t_iNum);
            // Look for comma
            while (--t_iLen != 0 && *t_pData++ != (BYTE)',') ;
            scale = CacheList::parsenumb(t_pData, 0, t_iLen, t_iNum);
            t = CacheList::dbllong(scale, t_iNum);
        }
        break;
    case CacheList::CT_UNICODE: // A UNICODE string consists of 'ddddd[,sssss]'
        if (t_iLen != 0) {
            if ((t_iLen & 1) != 0) {
#ifndef WIN32
                throw std::exception();
#else
                throw std::invalid_argument("List Format Error");
#endif
            }
            scale = CacheList::parsenumu(t_pData, 0, t_iLen, t_iNum);
            d = CacheList::dbllong(scale, t_iNum);
            // Look for comma
            while ((t_iLen -= 2) != 0
                && !(*t_pData++ == (BYTE)','
                && *t_pData++ == 0)) {
            }
            scale = CacheList::parsenumu(t_pData, 2, t_iLen, t_iNum);
            t = CacheList::dbllong(scale, t_iNum);
        }
        break;
    case CacheList::CT_POSINT:  // Positive integer
        d = getPosLongLong(t_pData, t_iLen);
        break;
    case CacheList::CT_NEGINT:  // Negative integer
        d = getNegLongLong(t_pData, t_iLen);
        break;
    case CacheList::CT_POSREAL: // Positive floating point number
        d = CacheList::dbllong(*(char *)t_pData, getPosLongLong(t_pData + 1, t_iLen - 1));
        break;
    case CacheList::CT_NEGREAL: // Negative floating point number
        d = CacheList::dbllong(*(char *)t_pData, getNegLongLong(t_pData + 1, t_iLen - 1));
        break;
    case CacheList::CT_DOUBLE:
        d = CacheList::ieee2long(t_pData,t_iLen);
	break;
    default:
#ifndef WIN32
        throw std::exception();
#else
        throw std::invalid_argument("List Format Error");
#endif
    }

    return static_cast<LONG>(((d - 47117) * 86400) + t);
}

CacheList CacheListItem::
AsCacheList() const
{
    return CacheList(data(), datalength());
}

CacheROList CacheListItem::
AsCacheROList() const
{
    return CacheROList(data(), datalength());
}

void* CacheListItem::
AsPtr() const
{
#ifdef PTR8 // Pointers are 64 bit
    return reinterpret_cast<void*>(AsLongLong());
#else // Pointers are 32 bit
    return reinterpret_cast<void*>(AsLong());
#endif
}
///////////////////////////////////////////////////////////////////
AString CacheListItem::
getAscii(const BYTE* p_pData, const size_t p_iLen) const
{
    return AString( reinterpret_cast<const AChar*>(p_pData), p_iLen);
}

UAString CacheListItem::
getUAscii(const BYTE* p_pData, const size_t p_iLen) const
{
    return UAString( reinterpret_cast<const UAChar*>(p_pData), p_iLen);
}

C16String CacheListItem::
getC16Unicode(const BYTE* p_pData, const size_t p_iLen) const
{
#if (defined(_ISC_BIGENDIAN) && _ISC_BIGENDIAN)
    size_t     t_iLen = p_iLen;
    size_t     t_i = 0;
    BYTE*   t_pBuffer = new BYTE[t_iLen];
    if (!t_pBuffer) {
#ifndef WIN32
        throw std::exception();
#else
        throw std::bad_alloc();
#endif  // WIN32
    }

    for (size_t t_j = 0; t_j < t_iLen; t_j += 2) {
        t_pBuffer[t_i++] = p_pData[t_j + 1];
        t_pBuffer[t_i++] = p_pData[t_j];
    }

#ifdef USE_ISC_NAMESPACE
    ISC::C16String t_xstr = ISC::C16String(reinterpret_cast<char16_t*>(t_pBuffer), t_iLen );
#else
    C16String t_xstr = C16String(reinterpret_cast<char16_t*>(t_pBuffer), t_iLen );
#endif // USE_ISC_NAMESPACE

    delete[] t_pBuffer;

#ifdef WCHAR4
    return xStr2c16Str(t_xstr);
#else
    return t_xstr;
#endif // WCHAR4

#else // _ISC_BIGENDIAN

#ifdef USE_ISC_NAMESPACE
    ISC::XString t_xstr(reinterpret_cast<const XChar*>(p_pData), p_iLen >> 1);
#else
    XString t_xstr(reinterpret_cast<const XChar*>(p_pData), p_iLen >> 1);
#endif // USE_ISC_NAMESPACE

#ifdef WCHAR4
    return xStr2c16Str(t_xstr);
#else
    return xStr2c16Str(t_xstr);
#endif // WCHAR4

#endif // _ISC_BIGENDIAN
}

WString CacheListItem::
getUnicode(const BYTE* p_pData, const size_t p_iLen) const
{
#if (defined(_ISC_BIGENDIAN) && _ISC_BIGENDIAN)
    size_t     t_iLen = p_iLen;
    size_t     t_i = 0;
    BYTE*   t_pBuffer = new BYTE[t_iLen];
    if (!t_pBuffer) {
#ifndef WIN32
        throw std::exception();
#else
        throw std::bad_alloc();
#endif  // WIN32
    }

    for (size_t t_j = 0; t_j < t_iLen; t_j += 2) {
        t_pBuffer[t_i++] = p_pData[t_j + 1];
        t_pBuffer[t_i++] = p_pData[t_j];
    }

#ifdef USE_ISC_NAMESPACE
    ISC::XString t_xstr = ISC::XString(reinterpret_cast<XChar*>(t_pBuffer), t_iLen );
#else
    XString t_xstr = XString(reinterpret_cast<XChar*>(t_pBuffer), t_iLen );
#endif // USE_ISC_NAMESPACE

    delete[] t_pBuffer;

#ifdef WCHAR4
    return xStr2wStr(t_xstr);
#else
    return t_xstr;
#endif // WCHAR4

#else // _ISC_BIGENDIAN

#ifdef USE_ISC_NAMESPACE
    ISC::XString t_xstr(reinterpret_cast<const XChar*>(p_pData), p_iLen >> 1);
#else
    XString t_xstr(reinterpret_cast<const XChar*>(p_pData), p_iLen >> 1);
#endif // USE_ISC_NAMESPACE

#ifdef WCHAR4
    return xStr2wStr(t_xstr);
#else
    return t_xstr;
#endif // WCHAR4

#endif // _ISC_BIGENDIAN
}

#ifdef USE_ISC_NAMESPACE
ISC::XString CacheListItem:: getXString(const BYTE* p_pData, const size_t p_iLen) const
#else
XString CacheListItem:: getXString(const BYTE* p_pData, const size_t p_iLen) const
#endif // USE_ISC_NAMESPACE

{
#if (defined(_ISC_BIGENDIAN) && _ISC_BIGENDIAN)
    size_t     t_i = 0;
    BYTE*   t_pBuffer = new BYTE[p_iLen];
    if (!t_pBuffer) {
#ifndef WIN32
        throw std::exception();
#else
        throw std::bad_alloc();
#endif  // WIN32
    }

    for (size_t t_j = 0; t_j < p_iLen; t_j += 2) {
        t_pBuffer[t_i++] = p_pData[t_j + 1];
        t_pBuffer[t_i++] = p_pData[t_j];

    }
#ifdef USE_ISC_NAMESPACE
    ISC::XString t_xstr = ISC::XString(reinterpret_cast<XChar*>(t_pBuffer), p_iLen >> 1);
#else
    XString t_xstr = XString(reinterpret_cast<XChar*>(t_pBuffer), p_iLen >> 1);
#endif // USE_ISC_NAMESPACE

    delete[] t_pBuffer;

#else // _ISC_BIGENDIAN

#ifdef USE_ISC_NAMESPACE
    ISC::XString t_xstr(reinterpret_cast<const XChar*>(p_pData), p_iLen >> 1);
#else
    XString t_xstr(reinterpret_cast<const XChar*>(p_pData), p_iLen >> 1);
#endif // USE_ISC_NAMESPACE

#endif // _ISC_BIGENDIAN
    return t_xstr;
}

LONGLONG CacheListItem::
getPosLongLong(const BYTE* p_pData, const size_t p_iLen) const
{
    LONGLONG    t_lNum;

    t_lNum = 0;
    switch (p_iLen) {
    case 8: if ((t_lNum = ((LONGLONG)p_pData[7])<<56) < 0) {
        break;
            }
    case 7: t_lNum |= (((LONGLONG)p_pData[6])&255)<<48;
    case 6: t_lNum |= (((LONGLONG)p_pData[5])&255)<<40;
    case 5: t_lNum |= (((LONGLONG)p_pData[4])&255)<<32;
    case 4: t_lNum |= (((LONGLONG)p_pData[3])&255)<<24;
    case 3: t_lNum |= (((LONGLONG)p_pData[2])&255)<<16;
    case 2: t_lNum |= (((LONGLONG)p_pData[1])&255)<<8;
    case 1: t_lNum |= (p_pData[0]&255);
        return t_lNum;
    case 0: return 0;
    }
#ifndef WIN32
    throw std::exception();
#else
    throw std::out_of_range("LONGLONG too large");
#endif
}

LONGLONG CacheListItem::
getNegLongLong(const BYTE* p_pData, const size_t p_iLen) const
{
    LONGLONG    t_lNum;
    size_t         t_i;

    switch (p_iLen) {
    case 8: t_lNum = ((p_pData[7])<<24) | ((p_pData[6]&255)<<16)
                | ((p_pData[5]&255)<<8) |  (p_pData[4]&255);
        if (t_lNum >= 0) {
#ifndef WIN32
            throw std::exception();
#else
            throw std::out_of_range("LONGLONG too large");
#endif
        }
        break;
    case 7: t_lNum = 0xFF000000 | ((p_pData[6]&255)<<16)
                | ((p_pData[5]&255)<<8) | (p_pData[4]&255);
        break;
    case 6: t_lNum = 0xFFFF0000 | ((p_pData[5]&255)<<8)
                | (p_pData[4]&255);
        break;
    case 5: t_lNum = 0xFFFFFF00 | (p_pData[4]&255);
        break;
    case 4: return (((p_pData[3]&255)<<24)
                | ((p_pData[2]&255)<<16)
                | ((p_pData[1]&255)<<8)
                | (p_pData[0]&255)) | MAKELONGLONG(0xFFFFFFFF00000000);
    case 3: return (((p_pData[2]&255)<<16)
                | ((p_pData[1]&255)<<8)
                | (p_pData[0]&255)) | MAKELONGLONG(0xFFFFFFFFFF000000);
    case 2: return (((p_pData[1]&255)<<8)
                | (p_pData[0]&255)) | MAKELONGLONG(0xFFFFFFFFFFFF0000);
    case 1: return (p_pData[0]&255) | MAKELONGLONG(0xFFFFFFFFFFFFFF00);
    case 0: return -1;
#ifndef WIN32
    default: throw std::exception();
#else
    default: throw std::out_of_range("LONGLONG too large");
#endif
    }

    t_i = ((p_pData[3]&255)<<24) | ((p_pData[2]&255)<<16)
        | ((p_pData[1]&255)<<8) | (p_pData[0]&255);

    return (((LONGLONG)t_lNum)<<32) | (((LONGLONG)t_i) & 0xFFFFFFFFL);
}

#ifdef USE_ISC_NAMESPACE
ISC::XString CacheListItem::AsXString() const
#else
XString CacheListItem::AsXString() const
#endif // USE_ISC_NAMESPACE
{
    BYTE*   t_pData;
    size_t     t_iLen;
    AString t_str;

    switch (getType(t_pData, t_iLen)) {
    case CacheList::CT_UNDEF:
#ifdef USE_ISC_NAMESPACE
        return ISC::XString();
#else
        return XString();
#endif

    case CacheList::CT_ASCII:
        t_str = getAscii(t_pData, t_iLen);
        break;

    case CacheList::CT_UNICODE:
        return getXString(t_pData, t_iLen);

    case CacheList::CT_POSINT:
        t_str = CacheList::lngstr(getPosLongLong(t_pData, t_iLen));
        break;

    case CacheList::CT_NEGINT:
        t_str = CacheList::lngstr(getNegLongLong(t_pData, t_iLen));
        break;

    case CacheList::CT_POSREAL:
        t_str = CacheList::dblstr(*(char *)t_pData, getPosLongLong(t_pData + 1, t_iLen - 1));
        break;

    case CacheList::CT_NEGREAL:
        t_str = CacheList::dblstr(*(char *)t_pData, getNegLongLong(t_pData + 1, t_iLen - 1));
        break;

    case CacheList::CT_DOUBLE:
        t_str = CacheList::ieee2str(t_pData,t_iLen);
        break;

    default:
#ifndef WIN32
        throw std::exception();
#else
        throw std::invalid_argument("List Format Error");
#endif // WIN32
    }

    return a2xStr(t_str.c_str(), t_str.size());
}
