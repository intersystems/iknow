//**********************************************************************
//* Copyright (c) 1999 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* sysComTypes.h: Definition of sysComTypes
//**********************************************************************
//* Change History: 
//* PJN: 1999/12/01 Initial Implementation.
//**********************************************************************

#ifndef __FILE__
#define __FILE__ "sysComTypes.h"
#endif

#ifndef __LINE__
#define __LINE__ 0
#endif

#ifndef _sysComTypes_h_
#define _sysComTypes_h_

#ifdef TRACE_INCLUDE
#pragma message("...Begin Include "__FILE__)
#endif

#define COM_STDMETHODCALLTYPE
#define COM_EXPORT
#define COM_IMPORT
#define COM_NO_VTABLE

#ifdef TRACE_INCLUDE
#pragma message("...End   Include "__FILE__)
#endif

#else

#ifdef TRACE_INCLUDE
#pragma message("...Redundant Include Attempt " __FILE__)
#endif

#endif // _sysComTypes_h_
