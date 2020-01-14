//**********************************************************************
//* Copyright (c) 1999 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* utlAutoArray.h: Definition of Auto Array
//**********************************************************************
//* Change History:
//* PJN: 1999/12/01 Initial Implementation.
//**********************************************************************

#ifndef __FILE__
#define __FILE__ "utlAutoArray.h"
#endif

#ifndef __LINE__
#define __LINE__ 0
#endif

#ifndef _utlAutoArray_h_
#define _utlAutoArray_h_

#ifdef TRACE_INCLUDE
#pragma message("...Begin Include "__FILE__)
#endif

// Auto array
template<class T> class utlAutoArray {

// Creators
public:
    utlAutoArray(T* const pArray) : m_pArray(pArray) {}
    ~utlAutoArray() { delete[] m_pArray; }

private:
    // Prohibited
    utlAutoArray(const utlAutoArray<T>&);
    utlAutoArray& operator=(const utlAutoArray<T>&);

// Accessors
public:
    const T* get() const { return m_pArray; }
    T* get() { return m_pArray; }

private:
    T* const m_pArray;
};

// -----------------------------------------------------------
// Functors for use with utlAutoPtrArray

// Heap Destroyer

template <class T> class CHeapDestroyer {
public:
    static void Destroy(T* p_p) { delete p_p; }
};

// AutoPtrArray
template<class T, class D = CHeapDestroyer<T> > class utlAutoPtrArray {

// Creators
public:
    utlAutoPtrArray(T* pArray, int p_cElements, BOOL p_bDeleteArray = FALSE) :
        m_pArray(pArray), m_cElements(p_cElements), m_bDeleteArray(p_bDeleteArray) {}

    ~utlAutoPtrArray() {
        if (m_pArray) {
            try {
                for (int t_i = 0; t_i < m_cElements; t_i++) {
                    D::Destroy(m_pArray[t_i]);
                }
                if (TRUE == m_bDeleteArray) {
                    delete[] m_pArray; m_pArray = 0;
                }
            } catch (...) {
            }
        }
    }

private:
    // Prohibited
    utlAutoPtrArray(const utlAutoPtrArray<T,D>&) {}
    utlAutoPtrArray& operator=(const utlAutoPtrArray<T,D>&) {}

private:
    T*       m_pArray;
    int      m_cElements;
    BOOL     m_bDeleteArray;
};

#ifdef TRACE_INCLUDE
#pragma message("...End   Include "__FILE__)
#endif

#else

#ifdef TRACE_INCLUDE
#pragma message("...Redundant Include Attempt " __FILE__)
#endif

#endif // _utlAutoArray_h_

