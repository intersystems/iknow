//**********************************************************************
//* Copyright (c) 1999 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* sysCOM.inl: Implementation of sysCOM for Microsoft Windows NT (INTEL)
//**********************************************************************
//* Change History: 
//* PJN: 1999/12/01 Initial Implementation.
//**********************************************************************

#if defined(_MSC_VER)
// Auto Link COM Runtime
#ifndef _ISCDLL
#ifdef _WIN64
#pragma comment( lib, "IRISCOM64.LIB" )
#else
#pragma comment( lib, "IRISCOM.LIB" )
#endif
#endif
#endif

#include "sysCharConversion.h"
