//**********************************************************************
//* Copyright (c) 1999 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* sysComError.h: Definition of sysComError
//**********************************************************************
//* Change History: 
//* PJN: 1999/12/01 Initial Implementation.
//**********************************************************************

#ifndef __FILE__
#define __FILE__ "sysComError.h"
#endif

#ifndef __LINE__
#define __LINE__ 0
#endif

#ifndef _sysComError_h_
#define _sysComError_h_

#ifdef TRACE_INCLUDE
#pragma message("...Begin Include "__FILE__)
#endif

//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//

// Macros for testing a COM_HRESULT for SUCCESS/FAIL 
#define COM_SUCCEEDED(Status) ((COM_HRESULT)(Status) >= 0)
#define COM_FAILED(Status)    ((COM_HRESULT)(Status) <  0)
#define COM_SUCCEEDED0(Status) ((COM_HRESULT)(Status) == 0)

// Macros for defining a COM_HRESULT
#define _COM_HRESULT_TYPEDEF_(X) ((COM_HRESULT)X)

// Definitions for SCODES (HRESULTS)

// Success (TRUE)
#define COM_S_OK        _COM_HRESULT_TYPEDEF_(0x00000000L)

// Success (FALSE)
#define COM_S_FALSE     _COM_HRESULT_TYPEDEF_(0x00000001L)

// Microsoft compatible facility codes
#define COM_FACILITY_NULL                    0
#define COM_FACILITY_RPC                     1
#define COM_FACILITY_DISPATCH                2
#define COM_FACILITY_STORAGE                 3
#define COM_FACILITY_ITF                     4
#define COM_FACILITY_WIN32                   7
#define COM_FACILITY_WINDOWS                 8
#define COM_FACILITY_SSPI                    9
#define COM_FACILITY_CONTROL                 10
#define COM_FACILITY_CERT                    11
#define COM_FACILITY_INTERNET                12
#define COM_FACILITY_MEDIASERVER             13
#define COM_FACILITY_MSMQ                    14
#define COM_FACILITY_SETUPAPI                15
#define COM_FACILITY_SSH					 16

#define COM_FACILITY_UNIX                    98
#define COM_FACILITY_VMS                     99

// Define your new facility codes here
// *** and consider updating sysCom.cpp ***
#define COM_FACILITY_USERBASE                100

#define COM_FACILITY_CACHEBASIC_PARSE        COM_FACILITY_USERBASE + 0
#define COM_FACILITY_CACHEBASIC_COMPILE      COM_FACILITY_USERBASE + 1
#define COM_FACILITY_CACHEOBJECT             COM_FACILITY_USERBASE + 2
#define COM_FACILITY_TNODE                   COM_FACILITY_USERBASE + 3
#define COM_FACILITY_PARSER                  COM_FACILITY_USERBASE + 4
#define COM_FACILITY_CACHEHTML_PARSE         COM_FACILITY_USERBASE + 5
#define COM_FACILITY_CACHESQL_PARSE          COM_FACILITY_USERBASE + 6
#define COM_FACILITY_UDL_PARSE               COM_FACILITY_USERBASE + 7
#define COM_FACILITY_CACHEJS_PARSE           COM_FACILITY_USERBASE + 8
#define COM_FACILITY_XERCES_C                COM_FACILITY_USERBASE + 9
#define COM_FACILITY_CACHEASM_PARSE          COM_FACILITY_USERBASE + 10
#define COM_FACILITY_CACHE                   COM_FACILITY_USERBASE + 11 // encapsulates a CACHE_XXX error
#define COM_FACILITY_CACHECALLIN             COM_FACILITY_USERBASE + 12 // encapsulates an error code from a call-in to Cache
#define COM_FACILITY_CACHECOS_PARSE          COM_FACILITY_USERBASE + 13
#define COM_FACILITY_CACHEXML_PARSE          COM_FACILITY_USERBASE + 14
#define COM_FACILITY_SCANNER                 COM_FACILITY_USERBASE + 15
#define COM_FACILITY_ACTIVATE                COM_FACILITY_USERBASE + 16 // ActiveX Client interface
#define COM_FACILITY_CACHEPEPP_PARSE         COM_FACILITY_USERBASE + 17
#define COM_FACILITY_CACHEJAVA_PARSE         COM_FACILITY_USERBASE + 18
#define COM_FACILITY_CACHEJAVASCRIPT_PARSE   COM_FACILITY_USERBASE + 19
#define COM_FACILITY_CACHETSQL_PARSE         COM_FACILITY_USERBASE + 20
#define COM_FACILITY_CACHECSS_PARSE          COM_FACILITY_USERBASE + 21
#define COM_FACILITY_OPENLDAP                COM_FACILITY_USERBASE + 22
#define COM_FACILITY_CACHEISQL_PARSE         COM_FACILITY_USERBASE + 23
#define COM_FACILITY_ZEROCDNS                COM_FACILITY_USERBASE + 24
#define COM_FACILITY_CPPBINDING              COM_FACILITY_USERBASE + 25
#define COM_FACILITY_CACHECOLORHTML          COM_FACILITY_USERBASE + 26
#define COM_FACILITY_PROVIDER                COM_FACILITY_USERBASE + 27
#define COM_FACILITY_CACHEOSQL_PARSE         COM_FACILITY_USERBASE + 28
#define COM_FACILITY_CACHEEIVX_PARSE         COM_FACILITY_USERBASE + 29

// Generic success code
#define COM_ERROR_SUCCESS       0

//
// Severity values
//

#define COM_SEVERITY_SUCCESS    0
#define COM_SEVERITY_ERROR      1

//  Return the facility

#define COM_HRESULT_FACILITY(hr)  (((hr) >> 16) & 0x1fff)

//
//  Return the severity
//

#define COM_HRESULT_SEVERITY(hr)  (((hr) >> 31) & 0x1)

// Return the error
#define COM_HRESULT_CODE(hr)      ((hr) & 0xFFFF)

//
// Create an HRESULT value from component pieces
//

#define COM_MAKE_HRESULT(sev,fac,code) \
    ((COM_HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )


//
// Error definitions follow
//

// Definitions for system wide COM_HRESULTS these map directly to 
// Microsoft defined HRESULTS

// CoClass not available
#define COM_CLASS_E_CLASSNOTAVAILABLE   _COM_HRESULT_TYPEDEF_(0x80040111L)

// Can't aggregate this object
#define COM_CLASS_E_NOAGGREGATION       _COM_HRESULT_TYPEDEF_(0x80040110L)

// Can't find property
#define COM_DISP_E_MEMBERNOTFOUND       _COM_HRESULT_TYPEDEF_(0x80020003L)

// Can't find parameter
#define COM_DISP_E_PARAMNOTFOUND        _COM_HRESULT_TYPEDEF_(0x80020004L)

// Can't find method
#define COM_DISP_E_UNKNOWNNAME          _COM_HRESULT_TYPEDEF_(0x80020006L)

// General failure
#define COM_E_FAIL                      _COM_HRESULT_TYPEDEF_(0x80004005L)

// One or more arguments are invalid
#define COM_E_INVALIDARG                _COM_HRESULT_TYPEDEF_(0x80070057L)

// Can't find specified interface
#define COM_E_NOINTERFACE               _COM_HRESULT_TYPEDEF_(0x80004002L)

// Method not implemented
#define COM_E_NOTIMPL                   _COM_HRESULT_TYPEDEF_(0x80004001L)

// Ran out of memory
#define COM_E_OUTOFMEMORY               _COM_HRESULT_TYPEDEF_(0x8007000EL)

// Catastrophic failure
#define COM_E_UNEXPECTED                _COM_HRESULT_TYPEDEF_(0x8000FFFFL)

// Couldn't find DLL
#define COM_E_DLLNOTFOUND               _COM_HRESULT_TYPEDEF_(0x800401F8L)

// Entry point not found in DLL
#define COM_E_ERRORINDLL                _COM_HRESULT_TYPEDEF_(0x800401F9L)

// Invalid Pointer
#define COM_E_POINTER                   _COM_HRESULT_TYPEDEF_(0x80004003L)

// File not found
#define COM_STG_E_FILENOTFOUND          _COM_HRESULT_TYPEDEF_(0x80030002L)

// Can't save file
#define COM_STG_E_CANTSAVE              _COM_HRESULT_TYPEDEF_(0x80030103L)

//  Could not read key from registry
#define COM_REGDB_E_READREGDB           _COM_HRESULT_TYPEDEF_(0x80040150L)

//  Could not write key to registry
#define COM_REGDB_E_WRITEREGDB          _COM_HRESULT_TYPEDEF_(0x80040151L)

//  Could not find the key in the registry
#define COM_REGDB_E_KEYMISSING          _COM_HRESULT_TYPEDEF_(0x80040152L)

//  Invalid value for registry
#define COM_REGDB_E_INVALIDVALUE        _COM_HRESULT_TYPEDEF_(0x80040153L)

// ******************
// FACILITY_DISPATCH
// ******************

// Unknown interface
#define COM_DISP_E_UNKNOWNINTERFACE     _COM_HRESULT_TYPEDEF_(0x80020001L)

// Member not found.
#define COM_DISP_E_MEMBERNOTFOUND       _COM_HRESULT_TYPEDEF_(0x80020003L)

// Parameter not found.
#define COM_DISP_E_PARAMNOTFOUND        _COM_HRESULT_TYPEDEF_(0x80020004L)

// Type mismatch.
#define COM_DISP_E_TYPEMISMATCH         _COM_HRESULT_TYPEDEF_(0x80020005L)

// Unknown name.
#define COM_DISP_E_UNKNOWNNAME          _COM_HRESULT_TYPEDEF_(0x80020006L)

// No named arguments.
#define COM_DISP_E_NONAMEDARGS          _COM_HRESULT_TYPEDEF_(0x80020007L)

// Bad variable type.
#define COM_DISP_E_BADVARTYPE           _COM_HRESULT_TYPEDEF_(0x80020008L)

// Exception occurred.
#define COM_DISP_E_EXCEPTION            _COM_HRESULT_TYPEDEF_(0x80020009L)

// Out of present range.
#define COM_DISP_E_OVERFLOW             _COM_HRESULT_TYPEDEF_(0x8002000AL)

// Bad Index
#define COM_DISP_E_BADINDEX             _COM_HRESULT_TYPEDEF_(0x8002000BL)

// Unknown language.
#define COM_DISP_E_UNKNOWNLCID          _COM_HRESULT_TYPEDEF_(0x8002000CL)

// Memory is locked.
#define COM_DISP_E_ARRAYISLOCKED        _COM_HRESULT_TYPEDEF_(0x8002000DL)

// Invalid number of parameters.
#define COM_DISP_E_BADPARAMCOUNT        _COM_HRESULT_TYPEDEF_(0x8002000EL)

// Parameter not optional.
#define COM_DISP_E_PARAMNOTOPTIONAL     _COM_HRESULT_TYPEDEF_(0x8002000FL)

// Invalid callee.
#define COM_DISP_E_BADCALLEE            _COM_HRESULT_TYPEDEF_(0x80020010L)

// Does not support a collection.
#define COM_DISP_E_NOTACOLLECTION       _COM_HRESULT_TYPEDEF_(0x80020011L)

// Division by zero.
#define COM_DISP_E_DIVBYZERO            _COM_HRESULT_TYPEDEF_(0x80020012L)

// ***********************************************************************
//
// Codes 0x00 - 0xff are reserved for Microsoft OLE
//
// ***********************************************************************

// Macro to make an interface specific error code
#define COM_MAKE_ERROR_ITF(code) \
    COM_MAKE_HRESULT(COM_SEVERITY_ERROR,COM_FACILITY_ITF,(code))

// Macro to make an Facility Specific error code
#define COM_MAKE_ERROR(facility,code) \
    COM_MAKE_HRESULT(COM_SEVERITY_ERROR,(facility),(code))

// ***********************************************************************
// Definitions for Parsers

// *** if you update this section please consider updating sysCom.cpp ***

// Error during parse
#define COM_E_PARSEFAIL                 COM_MAKE_ERROR(COM_FACILITY_PARSER,0x001)

// Error during compilation
#define COM_E_COMPILEFAIL               COM_MAKE_ERROR(COM_FACILITY_PARSER,0x002)

// Syntax Error
#define COM_E_SYNTAX                    COM_MAKE_ERROR(COM_FACILITY_PARSER,0x003)

// Unsupported moniker extension
#define COM_E_UNSUPPORTEDMONIKEREXTENSION COM_MAKE_ERROR(COM_FACILITY_PARSER,0x004)


// ***********************************************************************
// Definitions for Scanner

// attribute bookmark is either no longer valid (checkpoint restored) or completely bogus
#define COM_E_INVATTRBOOKMARK           COM_MAKE_ERROR(COM_FACILITY_SCANNER, 0x001)


// ***********************************************************************
// Definitions for COM_IResource & friends specific COM_HRESULTS

// *** if you update this section please consider updating sysCom.cpp ***

#define COM_E_NORESOURCE                COM_MAKE_ERROR_ITF(0x300)

// Already have this item
#define COM_E_HASITEM                   COM_MAKE_ERROR_ITF(0x301)

// Does not have this item
#define COM_E_NOTHASITEM                COM_MAKE_ERROR_ITF(0x302)

// ***********************************************************************
// Definitions for XML
#define COM_XML_E_INVALIDTNODE          COM_MAKE_ERROR(COM_FACILITY_CACHEXML_PARSE,0x001)

// ***********************************************************************
// Definitions for TNODES
#define COM_TNODE_E_ERROR                     COM_MAKE_ERROR(COM_FACILITY_TNODE,0x001)
#define COM_TNODE_E_SERVER                    COM_MAKE_ERROR(COM_FACILITY_TNODE,0x002)
#define COM_TNODE_E_NOCACHEEXECUTE            COM_MAKE_ERROR(COM_FACILITY_TNODE,0x003)
#define COM_TNODE_E_KEYWORDDESCRIPTOR         COM_MAKE_ERROR(COM_FACILITY_TNODE,0x004)
#define COM_TNODE_E_NAMECONFLICT              COM_MAKE_ERROR(COM_FACILITY_TNODE,0x005)
#define COM_TNODE_E_NAMEINVALID               COM_MAKE_ERROR(COM_FACILITY_TNODE,0x006)
#define COM_TNODE_E_NOACLASS                  COM_MAKE_ERROR(COM_FACILITY_TNODE,0x007)
#define COM_TNODE_E_TNODENOTFOUND             COM_MAKE_ERROR(COM_FACILITY_TNODE,0x008)
#define COM_TNODE_E_CLASSEXISTS               COM_MAKE_ERROR(COM_FACILITY_TNODE,0x009)
#define COM_TNODE_E_CLASSNAMEINVALID          COM_MAKE_ERROR(COM_FACILITY_TNODE,0x00a)
#define COM_TNODE_E_CLASSNAMETOOLONG          COM_MAKE_ERROR(COM_FACILITY_TNODE,0x00b)
#define COM_TNODE_E_CLASSMEMBERNAMETOOLONG    COM_MAKE_ERROR(COM_FACILITY_TNODE,0x00c)
#define COM_TNODE_E_CLASSDEPLOYED             COM_MAKE_ERROR(COM_FACILITY_TNODE,0x00d)
#define COM_TNODE_E_PROTOCOL                  COM_MAKE_ERROR(COM_FACILITY_TNODE,0x00e)
#define COM_TNODE_E_SERVERDISCONNECTED        COM_MAKE_ERROR(COM_FACILITY_TNODE,0x00f)


// ***********************************************************************
// Definitions for ACTIVATE
#define COM_E_ACTIVATE_RESULTTOOLARGE         COM_MAKE_ERROR(COM_FACILITY_ACTIVATE,0x001)

// ***********************************************************************
// Definitions for OPENLDAP
#define COM_OPENLDAP_E_INIT_FAILED            COM_MAKE_ERROR(COM_FACILITY_OPENLDAP,0x001)

// ***********************************************************************
// Definitions for Providers
#define COM_PROVIDER_E_CANNOTSPAWNCHILD       COM_MAKE_ERROR(COM_FACILITY_PROVIDER,0x001)
#define COM_PROVIDER_E_ERRORINSPAWNEDCHILD    COM_MAKE_ERROR(COM_FACILITY_PROVIDER,0x002)
#define COM_PROVIDER_E_SERVERERROR            COM_MAKE_ERROR(COM_FACILITY_PROVIDER,0x003)
#define COM_PROVIDER_E_XMLHTTPREQUESTERROR    COM_MAKE_ERROR(COM_FACILITY_PROVIDER,0x004)
#define COM_PROVIDER_E_PROTOCOLVERSIONERROR   COM_MAKE_ERROR(COM_FACILITY_PROVIDER,0x005)
#define COM_PROVIDER_E_DATAFORMATERROR        COM_MAKE_ERROR(COM_FACILITY_PROVIDER,0x006)

// ***********************************************************************
// Definitions for Cache SSH
#define CACHESSH_GROUP_FROM_HRESULT(hr) 	((hr >> 12) & 0xF)
#define CACHESSH_MAKE_GROUPCODE(grp,err)	((grp & 0xF) << 12) | err

#define COM_HRESULT_FROM_CACHESSH_ERROR(group,err) \
COM_MAKE_HRESULT(COM_SEVERITY_ERROR, COM_FACILITY_SSH, CACHESSH_MAKE_GROUPCODE(group,err))

#define GROUP_CACHESSH	0
#define GROUP_SSH		1
#define GROUP_SFTP		2

#define CACHESSH_ERROR(err)			COM_HRESULT_FROM_CACHESSH_ERROR(GROUP_CACHESSH,err)

#define COM_E_SSH_NOTCONNECTED		CACHESSH_ERROR(1)
#define COM_E_SSH_HOSTKEYMISMATCH	CACHESSH_ERROR(2)
#define COM_E_SSH_INVALIDHOSTNAME	CACHESSH_ERROR(3)
#define COM_E_SSH_TIMEOUT			CACHESSH_ERROR(4)
#define COM_E_SSH_CONNECTED			CACHESSH_ERROR(5)
#define COM_E_SSH_NOSESSION         CACHESSH_ERROR(6)
#define COM_E_SSH_NOCHANNEL         CACHESSH_ERROR(7)
#define COM_E_SSH_NOSFTP            CACHESSH_ERROR(8)
#define COM_E_SSH_SESSIONINUSE      CACHESSH_ERROR(9)

// ***********************************************************************

#ifdef TRACE_INCLUDE
#pragma message("...End   Include "__FILE__)
#endif

#else

#ifdef TRACE_INCLUDE
#pragma message("...Redundant Include Attempt " __FILE__)
#endif

#endif // _sysComError_h_
