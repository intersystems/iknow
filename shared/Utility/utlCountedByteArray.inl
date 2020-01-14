// -*- C++ -*-
//**********************************************************************
//* Copyright (c) 2000 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* utlCountedByteArray.inl: Definition of Counted Binary Array
//**********************************************************************
//* Change History:
//* Dave McCaldon:  26/1/2000   Initial Implementation.
//**********************************************************************

M_INLINE CountedByteArray::CountedByteArray()
    : m_prgby(0),
      m_crgby(0),
      m_bOwn(false)
{
}

M_INLINE CountedByteArray::CountedByteArray(BYTE* p_rgby, size_t p_crgby)
    : m_prgby(p_rgby),
      m_crgby(p_crgby),
      m_bOwn(true)
{
}

M_INLINE CountedByteArray::CountedByteArray(const BYTE* p_rgby, size_t p_crgby)
    : m_prgby(const_cast<BYTE*>(p_rgby)),
      m_crgby(p_crgby),
      m_bOwn(false)
{
}

M_INLINE CountedByteArray::~CountedByteArray()
{
    if (m_bOwn && (0 != m_prgby))
    {
        delete[] m_prgby;
    }

    m_prgby = 0;
    m_crgby = 0;
}

M_INLINE BYTE* CountedByteArray::data() const
{
    return m_prgby;
}

M_INLINE size_t CountedByteArray::length() const
{
    return m_crgby;
}

M_INLINE BYTE& CountedByteArray::operator[](size_t p_nIndex) const
{
    return m_prgby[p_nIndex];
}

M_INLINE CountedByteArray::operator BYTE*() const
{
    return m_prgby;
}

M_INLINE BYTE* CountedByteArray::release()
{
    m_bOwn = false;
    return data();
}

/* ************************ GrowableCountedByteArray *********************** */

/**
 * Initialize the counted byte array with the specified size
 *
 * @param p_cInitialSize  Initial size of the array
 */
M_INLINE GrowableCountedByteArray::GrowableCountedByteArray(size_t p_cInitialSize)
    : m_cCapacity(p_cInitialSize)
{
    if (p_cInitialSize > 0)
    {
        m_prgby = new BYTE[p_cInitialSize];
        m_crgby = p_cInitialSize;
        m_bOwn = true;
    }
}

/**
 * Initialize the counted byte array
 *
 * @param p_rgby   Array of bytes to initialize this with
 * @param p_crgby  Size of the array
 */
M_INLINE GrowableCountedByteArray::GrowableCountedByteArray(BYTE* p_rgby,
                                                            size_t   p_crgby)
    : CountedByteArray(p_rgby, p_crgby),
      m_cCapacity(p_crgby)
{
}

/**
 * Initialize the counted byte array
 *
 * @param p_rgby   Array of bytes to initialize this with
 * @param p_crgby  Size of the array
 */
M_INLINE
GrowableCountedByteArray::GrowableCountedByteArray(const BYTE* p_rgby,
                                                   size_t         p_crgby)
    : CountedByteArray(p_rgby, p_crgby),
      m_cCapacity(0)
{
}

M_INLINE void GrowableCountedByteArray::resize(size_t p_cSize, size_t p_cCapacity)
{
    size_t   t_cSize     = (kDefault != p_cSize) ? p_cSize : m_crgby;
    size_t   t_cCapacity = (kDefault != p_cCapacity)
      ? p_cCapacity
      : (p_cSize > m_cCapacity ? p_cSize : m_cCapacity);
    BYTE* t_prgby     = new BYTE[t_cCapacity];
    
    
    sysMemCpy(t_prgby, m_prgby, (t_cSize < m_crgby ? t_cSize : m_crgby));
    
    if (m_bOwn) delete[] m_prgby;
    
    m_prgby = t_prgby;
    m_crgby = t_cSize;
    m_bOwn = true;
}

/**
 * Fetches the overall capacity of this CBA
 *
 * @return Capacity of this CBA
 */
M_INLINE size_t GrowableCountedByteArray::capacity() const
{
    return m_cCapacity;
}

/**
 * Append data to this <code>GrowableCountedByteArray</code>
 *
 * @param p_rgby   Data to append
 * @param p_crgby  Size of data to append
 */
M_INLINE void GrowableCountedByteArray::append(const BYTE* p_rgby, size_t p_crgby)
{
    resize(m_crgby + p_crgby);

    sysMemCpy(&m_prgby[m_crgby], p_rgby, p_crgby);
    m_crgby += p_crgby;
}

/**
 * Append data to this <code>GrowableCountedByteArray</code>
 *
 * @param p_rCBA  CountedByteArray to append
 */
M_INLINE void GrowableCountedByteArray::append(const CountedByteArray& p_rCBA)
{
    append(p_rCBA.data(), p_rCBA.length());
}

/**
 * Assign another counted byte array to this one
 *
 * @param p_rCBA  Array to copy from
 */
M_INLINE CountedByteArray&
GrowableCountedByteArray::operator= (const CountedByteArray& p_rCBA)
{
    if (m_bOwn) delete[] m_prgby;

    m_crgby = p_rCBA.length();
    m_prgby = new BYTE[m_crgby];
    m_bOwn = true;

    sysMemCpy(m_prgby, p_rCBA.data(), m_crgby);


    return *this;
}
