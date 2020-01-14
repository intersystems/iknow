//**********************************************************************
//* Copyright (c) 1999 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* sysMemory.inl: Implementation of sysMemory
//**********************************************************************
//* Change History:
//* PJN: 1999/12/01 Initial Implementation.
//**********************************************************************

// System-specific implementations for Microsoft Windows NT (INTEL)
#include <string.h>
#include <windows.h>

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
inline SIZE_TYP sysMCHARzStrLen(const AChar* const p_psz) {
    return (SIZE_TYP)::strlen(p_psz);
}

template<>
inline SIZE_TYP sysMCHARzStrLen(const WChar* const p_psz) {
    return (SIZE_TYP)::wcslen(p_psz);
}

template<>
inline const AChar* sysMCHARzStr(const AChar* t_pStr,const AChar* t_pStrSub) {
    return ::strstr(t_pStr,t_pStrSub);
}

template<>
inline const WChar* sysMCHARzStr(const WChar* t_pStr,const WChar* t_pStrSub) {
    return ::wcsstr(t_pStr,t_pStrSub);
}
