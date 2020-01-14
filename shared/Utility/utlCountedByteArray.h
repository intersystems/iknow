// -*- C++ -*-
//**********************************************************************
//* Copyright (c) 2000 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* utlCountedByteArray.h: Definition of Counted Binary Array
//**********************************************************************
//* Change History:
//* Dave McCaldon:  26/1/2000   Initial Implementation.
//**********************************************************************

#ifndef __FILE__
#define __FILE__ "utlCountedByteArray.h"
#endif

#ifndef __LINE__
#define __LINE__ 0
#endif

#ifndef _utlCountedByteArray_h_
#define _utlCountedByteArray_h_

#ifdef TRACE_INCLUDE
#pragma message("...Begin Include "__FILE__)
#endif

#ifndef _sysCommon_h_
#include "sysCommon.h"
#endif

#ifndef _sysMemory_h_
#include "sysMemory.h"
#endif

class CountedByteArray
{
public:
    CountedByteArray();
    CountedByteArray(BYTE* p_rgby, size_t p_crgby);
    CountedByteArray(const BYTE* p_rgby, size_t p_crgby);
    virtual ~CountedByteArray();

    BYTE* data() const;
    size_t length() const;

    BYTE& operator[](size_t p_nIndex) const;
    operator BYTE*() const;

    BYTE* release();
    
protected:
    BYTE*   m_prgby;
    size_t     m_crgby;
    BOOL    m_bOwn;
};

/**
 * An expandable version of the <code>CountedByteArray</code> class
 */
class GrowableCountedByteArray : public CountedByteArray
{
public:
    GrowableCountedByteArray(size_t p_cInitialSize = 0);
    GrowableCountedByteArray(BYTE* p_rgby, size_t p_crgby);
    GrowableCountedByteArray(const BYTE* p_rgby, size_t p_crgby);

public:
  static const size_t kDefault = static_cast<size_t>(-1);
    void resize(size_t p_cSize = kDefault, size_t p_cCapacity = kDefault);
    size_t  capacity() const;

    CountedByteArray& operator = (const CountedByteArray& p_rByteArray);

    void append(const BYTE* p_rgby, size_t p_crgby);
    void append(const CountedByteArray& p_rCBA);

protected:
    size_t m_cCapacity;
};

#ifndef NO_UCHAR_TYPE
/**
 * A handy function for dumping a byte array out to an STL string
 *
 * @param p_pbData    Pointer to the byte array to dump
 * @param p_cbData    Size of the byte array
 * @param p_rsOutput  STL String output buffer (to be appended to)
 */
void DumpByteArray(const BYTE* p_pbData, size_t p_cbData, String& p_rsOutput);
#endif //NO_UCHAR_TYPE

#ifndef M_DEBUG
#include "utlCountedByteArray.inl"
#endif

#ifdef TRACE_INCLUDE
#pragma message("...End   Include "__FILE__)
#endif

#else

#ifdef TRACE_INCLUDE
#pragma message("...Redundant Include Attempt " __FILE__)
#endif

#endif // _utlCountedByteArray_h_
