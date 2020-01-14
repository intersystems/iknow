//**********************************************************************
//* Copyright (c) 1999 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* sysCom.h: Definition of sysCOM
//**********************************************************************
//* Change History:
//* CDS917 11/07/06 Make exports visible on macosx
//* PJN: 1999/12/01 Initial Implementation.
//**********************************************************************

#ifndef __FILE__
#define __FILE__ "sysCom.h"
#endif

#ifndef __LINE__
#define __LINE__ 0
#endif

#ifndef _sysCom_h_
#define _sysCom_h_

#ifdef TRACE_INCLUDE
#pragma message("...Begin Include "__FILE__)
#endif


#ifndef _sysCommon_h_
#include "sysCommon.h"
#endif

#ifndef _sysMemory_h_
#include "sysMemory.h"
#endif

#ifndef _sysComTypes_h_
#include "sysComTypes.h"
#endif

#ifndef _sysComError_h_
#include "sysComError.h"
#endif

#ifndef _utlAutoArray_h_
#include "utlAutoArray.h"
#endif

// Bogus hack for Visual Assist problem expanding macros
// They only expand macros with () after them
#if 0
#define COM_INTERFACE()     struct
#define COM_IMPLEMENTS()    public
#define COM_OVERRIDES()     virtual
#define COM_EXTERN_C()      extern "C"
#define COM_STDMETHODIMP()  COM_HRESULT COM_STDMETHODCALLTYPE
#endif
// End bogus hack

#define COM_INTERFACE   struct
#define COM_IMPLEMENTS  public
#define COM_OVERRIDES   virtual

#ifdef __APPLE__
#define COM_EXTERN_C    extern "C" __attribute__((visibility("default")))
#else
#define COM_EXTERN_C    extern "C"
#endif

#define COM_STDMETHODIMP            COM_HRESULT COM_STDMETHODCALLTYPE
#define COM_STDMETHODIMP_(type)     type COM_STDMETHODCALLTYPE
#define COM_STDMETHOD(method)       virtual COM_HRESULT COM_STDMETHODCALLTYPE method
#define COM_STDMETHOD_(type,method) virtual type COM_STDMETHODCALLTYPE method

#ifndef COM_GUID_DEFINED
#define COM_GUID_DEFINED

typedef struct  _COM_GUID {
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[ 8 ];
} COM_GUID;

#endif // COM_GUID_DEFINED

typedef const   _COM_GUID&   COM_REFGUID;
typedef const   _COM_GUID&   COM_REFIID;
typedef         _COM_GUID    COM_IID;
typedef const   _COM_GUID&   COM_REFCLSID;
typedef         _COM_GUID    COM_CLSID;
typedef const   _COM_GUID&   COM_REFCATID;
typedef         _COM_GUID    COM_CATID;


// GUID definitions
#ifndef COM_INITGUID
#define COM_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        COM_EXTERN_C const _COM_GUID name
#else
#define COM_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        COM_EXTERN_C const _COM_GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#endif // COM_INITGUID

// Test for equality of GUIDs
inline BOOL IsEqualCOM_GUID(const COM_GUID& rGuid1, const COM_GUID& rGuid2) {
    return !sysMemCmp(reinterpret_cast<const BYTE*>(&rGuid1),
        reinterpret_cast<const BYTE*>(&rGuid2), sizeof(COM_GUID));
}

// Override ==
inline BOOL operator==(const COM_GUID& rGuid1, const COM_GUID& rGuid2) {
    return IsEqualCOM_GUID(rGuid1,rGuid2);
}

// Test for less than of GUIDs
inline BOOL IsLessCOM_GUID(const COM_GUID& rGuid1, const COM_GUID& rGuid2) {
    return (sysMemCmp(reinterpret_cast<const BYTE*>(&rGuid1),
        reinterpret_cast<const BYTE*>(&rGuid2), sizeof(COM_GUID)) == -1);
}

// Override <
inline BOOL operator<(const COM_GUID& rGuid1, const COM_GUID& rGuid2) {
    return IsLessCOM_GUID(rGuid1,rGuid2);
}

// Common Types
typedef WChar*          COM_BSTR;
typedef SDWORD          COM_HRESULT;
typedef WChar           COM_CHAR;
typedef WChar*          COM_LPWSTR;
typedef const WChar*    COM_LPCWSTR;
typedef AChar*          COM_LPASTR;
typedef const AChar*    COM_LPCASTR;

typedef DWORD           COM_LCID;
typedef LONG            COM_DISPID;
typedef LONG            COM_SCODE;
typedef sysTypes::DOUBLE COM_DATE;
typedef void*           COM_PVOID;
typedef DWORD           COM_LCID;
typedef LONG            COM_DISPID;
typedef SHORT           COM_VARIANT_BOOL;
typedef USHORT          COM_VARTYPE;

// Dispatch IDs

#define COM_DISPID_VALUE             0
#define COM_DISPID_UNKNOWN          -1
#define COM_DISPID_PROPERTYPUT      -3
#define COM_DISPID_NEWENUM          -4
#define COM_DISPID_EVALUATE         -5
#define COM_DISPID_CONSTRUCTOR      -6
#define COM_DISPID_DESTRUCTOR       -7
#define COM_DISPID_COLLECT          -8

// Flags for COM_IDispatch Invoke
#define COM_DISPATCH_METHOD         0x1
#define COM_DISPATCH_PROPERTYGET    0x2
#define COM_DISPATCH_PROPERTYPUT    0x4
#define COM_DISPATCH_PROPERTYPUTREF 0x8

// Exception handling support
struct COM_HRESULT_EXCEPTION
{
    COM_HRESULT_EXCEPTION() : m_hr(COM_S_OK) {}

    COM_HRESULT_EXCEPTION(COM_HRESULT p_hr) : m_hr(p_hr) {
        if (COM_FAILED(p_hr)) {
            throw p_hr;
        }
    }

    COM_HRESULT_EXCEPTION& operator = (COM_HRESULT p_hr) {
        m_hr = p_hr;
        if (COM_FAILED(p_hr)) {
            throw p_hr;
        }
        return *this;
    }

    // Prefer AsComHResult()
    operator COM_HRESULT() const {
        return m_hr;
    }

    COM_HRESULT AsComHResult() const {
        return m_hr;
    }

    COM_HRESULT m_hr;
};

// Wrap a Cache error - or a *negative* Cache call-in error - in a COM_HRESULT
inline COM_HRESULT MAKE_CACHE_HRESULT(int p_nCResultCode) {
    if (p_nCResultCode < 0) {
        return COM_MAKE_HRESULT(COM_SEVERITY_ERROR, COM_FACILITY_CACHECALLIN, (-(p_nCResultCode)) & 0xFFFF);
    }
    else {
        return COM_MAKE_HRESULT(COM_SEVERITY_ERROR, COM_FACILITY_CACHE, p_nCResultCode);
    }
}

#ifndef NO_UCHAR_TYPE
// Return a message corresponding to a COM_HRESULT - doesn't handle Cache or Cache call-in COM_HRESULTs
String COM_HRESULT_Expansion(COM_HRESULT p_hRes);
#endif //NO_UCHAR_TYPE

// MACRO to provide type safety for QI
#define IID_PPV_ARG(Type,Expr) COM_IID_##Type, \
    reinterpret_cast<void**>(static_cast<COM_##Type**>(Expr))

#ifndef _ISCDLL
#define COM_LINKAGE     COM_IMPORT
#else   // _ISCDLL
#define COM_LINKAGE     COM_EXPORT
#endif  // _ISCDLL

// The JCOM API
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_GetClassFactory(COM_REFCLSID p_rClsid,void** p_ppvObject);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_CreateInstance(COM_REFCLSID p_rClsid, DWORD p_dwContext, COM_REFIID p_riid, void ** p_ppvObject);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_GetCLSIDFromProgID(COM_LPWSTR p_lpwszProgID,COM_CLSID* p_pClsid);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_GetProgIDFromClsid(COM_REFCLSID p_rClsid, COM_BSTR* p_pbstrProgId);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_GetDisplayNameFromClsid(COM_REFCLSID p_rClsid, COM_BSTR* p_pbstrDisplayName);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_GetDescriptionFromClsid(COM_REFCLSID p_rClsid, COM_BSTR* p_pbstrDescription);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_MkParseName(COM_LPWSTR p_pwszName,ULONG* p_ulEaten, COM_INTERFACE COM_IMoniker** p_ppIMoniker);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_GetService(COM_REFCLSID p_rClsid, DWORD p_dwContext, COM_REFIID p_riid, void ** p_ppvObject);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_Initialize();
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_Uninitialize();

COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_ClearErrorInfo();
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_CreateErrorInfo(OUT COM_INTERFACE COM_IErrorInfo** p_ppIEI);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_GetErrorInfo(OUT COM_INTERFACE COM_IErrorInfo** p_ppIEI);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_SetErrorInfo(IN COM_INTERFACE COM_IErrorInfo* p_pIEI);

COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_TaskMemAlloc(IN SIZE_TYP cb,void** ppv);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_TaskMemFree(IN void* pv);

COM_EXTERN_C COM_BSTR    COM_LINKAGE COM_SysAllocString(const WChar* p_pwsz);
COM_EXTERN_C BOOL        COM_LINKAGE COM_SysReAllocString(COM_BSTR* p_pbstr, const WChar* p_pwsz);
COM_EXTERN_C COM_BSTR    COM_LINKAGE COM_SysAllocStringLen(const WChar* p_pwsz, SIZE_TYP p_nLen);
COM_EXTERN_C COM_BSTR    COM_LINKAGE COM_SysAllocStringByteLen(const BYTE* p_pwsz, SIZE_TYP p_nLen);
COM_EXTERN_C BOOL        COM_LINKAGE COM_SysReAllocStringLen(COM_BSTR* p_pbstr, const WChar* p_pwsz, SIZE_TYP p_nLen);
COM_EXTERN_C BOOL        COM_LINKAGE COM_SysFreeString(COM_BSTR p_bstr);
COM_EXTERN_C UINT        COM_LINKAGE COM_SysStringLen(COM_BSTR p_bstr);
COM_EXTERN_C UINT        COM_LINKAGE COM_SysStringByteLen(COM_BSTR p_bstr);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_GetSysStats(LONG* p_nAllocs,LONG* p_nFrees);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_ResetSysStats();

// Miscellaneous utilities
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_CLSIDToString(COM_REFCLSID p_rclsid,COM_BSTR* p_pbstr);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_StringToCLSID(COM_LPCWSTR p_pswclsid, COM_CLSID* p_pclsid);
COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_GenerateCLSID(COM_CLSID* p_pCLSID);

COM_EXTERN_C COM_HRESULT COM_LINKAGE COM_DecodeHRESULT(COM_HRESULT p_hRes,DWORD p_dwLangID,COM_BSTR* p_ppbstrExpansion);

// BSTR Destroyer - Functor for use with utlAutoArray
class CBSTRDestroyer {
public:
    static void Destroy(COM_BSTR p_p) { 
#if defined(_WIN32) | defined(_WIN64)
        SysFreeString(p_p); 
#else
        COM_SysFreeString(p_p); 
#endif
    }
};

#include "sysCOM.inl"

#ifdef TRACE_INCLUDE
#pragma message("...End   Include "__FILE__)
#endif

#else

#ifdef TRACE_INCLUDE
#pragma message("...Redundant Include Attempt " __FILE__)
#endif

#endif // _sysCom_h_
