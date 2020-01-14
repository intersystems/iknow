//**********************************************************************
//* Copyright (c) 1999 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* sysMemory.inl: Implementation of sysMemory
//**********************************************************************
//* Change History:
//*
//* CDS1049  5/07/07 Chuck Sorenson, work around Apple bug with string visibility
//* PJN: 1999/12/01 Initial Implementation.
//* PJN: 2000/04/24 Port to Linux
//**********************************************************************

// System-specific implementations for Linux (INTEL)
#ifdef __APPLE__						// CDS1049+
#pragma GCC visibility push(default)
#endif								// CDS1049-

#include <string.h>
#include <wchar.h>

#ifdef __APPLE__						// CDS1049+
#pragma GCC visibility pop
#endif								// CDS1049-

// Allocate, Free and Reallocate memory
inline void* sysAlloc(SIZE_TYP size) {
    void* t_pv = malloc(size);
    if (!t_pv) {
        throw std::bad_alloc();
    }
    return t_pv;
}

inline void sysFree(void *memblock) {
    free(memblock);
}

inline void* sysRealloc(void *memblock, SIZE_TYP size) {
    void* t_pv = realloc(memblock, size);
    if (!t_pv) {
        throw std::bad_alloc();
    }
    return t_pv;
}

// Specializations

// sysMemCmp - compare memory
template<>
inline int sysMemCmp(const BYTE* const pbyLHS,const BYTE* const pbyRHS,SIZE_TYP stCountBytes) {
    return ::memcmp(pbyLHS,pbyRHS,stCountBytes);
}

// sysMemSet - fill memory with value
template <>
inline void sysMemSet(BYTE* const p_pTo,BYTE p_IscType,SIZE_TYP p_nCount)
{
    ::memset(p_pTo,p_IscType,p_nCount);
}

// sysMCHARzStrLen - length of the string
template<>
inline SIZE_TYP sysMCHARzStrLen(const char* const p_psz) {
    return ::strlen(p_psz);
}

template<>
inline SIZE_TYP sysMCHARzStrLen(const wchar_t* const p_psz) {
    return ::wcslen(reinterpret_cast<const wchar_t*>(p_psz));
}

template<>
inline const AChar* sysMCHARzStr(const char* t_pStr,const AChar* t_pStrSub) {
    return ::strstr(t_pStr,t_pStrSub);
}


