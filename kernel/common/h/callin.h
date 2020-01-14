/*
	Definitions for the Call-In Interface to IRIS

	Copyright (c) 1997-2018 by InterSystems.
	Cambridge, Massachusetts, U.S.A.  All rights reserved.

	This file contains macros to support compatibility for
	modules written for Cache to be compiled for execution
	with InterSystems IRIS.

	NOTE - because this file is included in C programs and the
	C compiler on some platforms may not support new style
	comments by default, the // style of comments should not
	be used here.

	Last Change: $Date$
	Changelist : $Change$
*/

#ifndef	_callin_h_
#define	_callin_h_

#if	defined(CACHE_UNICODE)	/* Unicode character strings */
#define	IRIS_UNICODE
#elif	defined(CACHE_WCHART)	/* wchar_t character strings */
#define	IRIS_WCHART
#endif

#if	defined(CACHE_INT64)
#define	IRIS_INT64 CACHE_INT64
#endif

#include "cdzf.h"
#include "iris-callin.h"


#define	CACHE_MAXSTRLEN IRIS_MAXSTRLEN
#define	CACHE_MAXLOSTSZ IRIS_MAXLOSTSZ
#define	CACHE_HSTR	IRIS_HSTR

/*		Data Structures
*/

#define	CACHE_ASTR	IRIS_ASTR
#define	CACHE_ASTRP	IRIS_ASTRP
#define	CACHEWSTR	IRISWSTR
#define	CACHEWSTRP	IRISWSTRP
#if !defined(CACHE_INT64)
#define	CACHE_INT64	IRIS_INT64
#endif
#define	CACHEHSTR	IRISHSTR
#define	CACHEHSTRP	IRISHSTRP
#define	CACHE_EXSTR	IRIS_EXSTR
#define	CACHE_EXSTRP	IRIS_EXSTRP
#define	CACHESTR	IRISSTR
#define	CACHE_STR	IRIS_STR
#define	CACHESTRP	IRISSTRP
#define	CACHE_STRP	IRIS_STRP
#define	CACHE_STRING	IRIS_STRING
#define	CACHE_IKNOWDATA		IRIS_IKNOWDATA
#define	CACHE_IKNOWDATAP	IRIS_IKNOWDATAP
#define	CACHE_IKNOWSTRING	IRIS_IKNOWSTRING
#define	CACHE_IKNOWSTRINGP	IRIS_IKNOWSTRINGP
#define	CACHE_IKNOWLOADFUNC	IRIS_IKNOWLOADFUNC
#define	CACHE_IKNOWRELEASEFUNC	IRIS_IKNOWRELEASEFUNC
#define	CACHE_IKNOWFUNCS	IRIS_IKNOWFUNCS
#define	CACHE_IKNOWFUNCSP	IRIS_IKNOWFUNCSP

/* 		Constants
*/

#define	CACHE_TTALL	IRIS_TTALL
#define	CACHE_TTCALLIN	IRIS_TTCALLIN
#define	CACHE_TTSTART	IRIS_TTSTART
#define	CACHE_TTNEVER	IRIS_TTNEVER
#define	CACHE_TTNONE	IRIS_TTNONE
#define	CACHE_TTMASK	IRIS_TTMASK
#define	CACHE_PROGMODE	IRIS_PROGMODE
#define	CACHE_TTNOUSE	IRIS_TTNOUSE
#define	CACHE_DISACTRLC	IRIS_DISACTRLC
#define	CACHE_ENABCTRLC	IRIS_ENABCTRLC
#define	CACHE_CTRLC	IRIS_CTRLC
#define	CACHE_RESJOB	IRIS_RESJOB

#define	CACHE_INCREMENTAL_LOCK	IRIS_INCREMENTAL_LOCK
#define	CACHE_SHARED_LOCK	IRIS_SHARED_LOCK
#define	CACHE_IMMEDIATE_RELEASE	IRIS_IMMEDIATE_RELEASE

#define	CACHE_INT	IRIS_INT
#define	CACHE_DOUBLE	IRIS_DOUBLE
#define	CACHE_ASTRING	IRIS_ASTRING
#define	CACHE_CHAR	IRIS_CHAR
#define	CACHE_INT2	IRIS_INT2
#define	CACHE_INT4	IRIS_INT4
#define	CACHE_INT8	IRIS_INT8
#define	CACHE_UCHAR	IRIS_UCHAR
#define	CACHE_UINT2	IRIS_UINT2
#define	CACHE_UINT4	IRIS_UINT4
#define	CACHE_UINT8	IRIS_UINT8
#define	CACHE_FLOAT	IRIS_FLOAT
#define	CACHE_HFLOAT	IRIS_HFLOAT
#define	CACHE_UINT	IRIS_UINT
#define	CACHE_WSTRING	IRIS_WSTRING
#define	CACHE_OREF	IRIS_OREF
#define	CACHE_LASTRING	IRIS_LASTRING
#define	CACHE_LWSTRING	IRIS_LWSTRING
#define	CACHE_IEEE_DBL	IRIS_IEEE_DBL
#define	CACHE_HSTRING	IRIS_HSTRING
#define	CACHE_UNDEF	IRIS_UNDEF

#define	CACHE_INVALID_INSTANCE	IRIS_INVALID_INSTANCE
#define	CACHE_CHANGEPASSWORD	IRIS_CHANGEPASSWORD
#define	CACHE_ACCESSDENIED	IRIS_ACCESSDENIED
#define	CACHE_EXSTR_INUSE	IRIS_EXSTR_INUSE
#define	CACHE_NORES		IRIS_NORES
#define	CACHE_BADARG		IRIS_BADARG
#define	CACHE_NOTINCACHE	IRIS_NOTINIRIS
#define	CACHE_RETTRUNC		IRIS_RETTRUNC
#define	CACHE_ERUNKNOWN		IRIS_ERUNKNOWN
#define	CACHE_RETTOOSMALL	IRIS_RETTOOSMALL
#define	CACHE_NOCON		IRIS_NOCON
#define	CACHE_INTERRUPT		IRIS_INTERRUPT
#define	CACHE_CONBROKEN		IRIS_CONBROKEN
#define	CACHE_STRTOOLONG	IRIS_STRTOOLONG
#define	CACHE_ALREADYCON	IRIS_ALREADYCON

#define	CACHE_FAILURE	IRIS_FAILURE
#define	CACHE_SUCCESS	IRIS_SUCCESS

#define	CACHE_ERMAXNUM	IRIS_ERMAXNUM
#define	CACHE_ERSELECT	IRIS_ERSELECT
#define	CACHE_ERSYNTAX	IRIS_ERSYNTAX
#define	CACHE_ERLABELREDEF	IRIS_ERLABELREDEF
#define	CACHE_ERMXSTR	IRIS_ERMXSTR
#define	CACHE_ERFUNCTION	IRIS_ERFUNCTION
#define	CACHE_ERINTERRUPT	IRIS_ERINTERRUPT
#define	CACHE_ERNOLINE	IRIS_ERNOLINE
#define	CACHE_ERUNDEF	IRIS_ERUNDEF
#define	CACHE_ERSYSTEM	IRIS_ERSYSTEM
#define	CACHE_EREDITED	IRIS_EREDITED
#define	CACHE_ERCOMMAND IRIS_ERCOMMAND
#define	CACHE_ERMAXARRAY	IRIS_ERMAXARRAY
#define	CACHE_ERNAME	IRIS_ERNAME
#define	CACHE_ERSTORE	IRIS_ERSTORE
#define	CACHE_ERSUBSCR	IRIS_ERSUBSCR
#define	CACHE_ERNOROUTINE	IRIS_ERNOROUTINE
#define	CACHE_ERDIVIDE	IRIS_ERDIVIDE
#define	CACHE_ERSWIZZLEFAIL	IRIS_ERSWIZZLEFAIL
#define	CACHE_ERSTRINGSTACK	IRIS_ERSTRINGSTACK
#define	CACHE_ERFRAMESTACK	IRIS_ERFRAMESTACK
#define	CACHE_ERUNIMPLEMENTED	IRIS_ERUNIMPLEMENTED
#define	CACHE_EREXQUOTA	IRIS_EREXQUOTA
#define	CACHE_ERNOTOPEN	IRIS_ERNOTOPEN
#define	CACHE_ERARGSTACK	IRIS_ERARGSTACK
#define	CACHE_ERM11TAPE	IRIS_ERM11TAPE
#define	CACHE_ERPROTECT	IRIS_ERPROTECT
#define	CACHE_ERDATABASE	IRIS_ERDATABASE
#define	CACHE_ERFILEFULL	IRIS_ERFILEFULL
#define	CACHE_ERNAKED	IRIS_ERNAKED
#define	CACHE_ERROUTINELOAD	IRIS_ERROUTINELOAD
#define	CACHE_ERTERMINATOR	IRIS_ERTERMINATOR
#define	CACHE_ERDISKHARD	IRIS_ERDISKHARD
#define	CACHE_ERBLOCKNUM	IRIS_ERBLOCKNUM
#define	CACHE_ERDEVNAME	IRIS_ERDEVNAME
#define	CACHE_ERNOJOB	IRIS_ERNOJOB
#define	CACHE_ERREAD	IRIS_ERREAD
#define	CACHE_ERWRITE	IRIS_ERWRITE
#define	CACHE_EREOF	IRIS_EREOF
#define	CACHE_ERPARAMETER	IRIS_ERPARAMETER
#define	CACHE_ERNETWORK	IRIS_ERNETWORK
#define	CACHE_ERRECOMPILE	IRIS_ERRECOMPILE
#define	CACHE_ERDIR	IRIS_ERDIR
#define	CACHE_ERMAXSCOPE	IRIS_ERMAXSCOPE
#define	CACHE_ERCLUSTERFAIL	IRIS_ERCLUSTERFAIL
#define	CACHE_ERMAXERROR	IRIS_ERMAXERROR
#define	CACHE_ERCONBROKEN	IRIS_ERCONBROKEN
#define	CACHE_ERNOMBX	IRIS_ERNOMBX
#define	CACHE_ERNOCP	IRIS_ERNOCP
#define	CACHE_ERNOWD	IRIS_ERNOWD
#define	CACHE_ERNOGC	IRIS_ERNOGC
#define	CACHE_ERMTERR	IRIS_ERMTERR
#define	CACHE_ERDOWN	IRIS_ERDOWN
#define	CACHE_ERCLNUP	IRIS_ERCLNUP
#define	CACHE_ERMAXPID	IRIS_ERMAXPID
#define	CACHE_ERNOSIGNON	IRIS_ERNOSIGNON
#define	CACHE_ERHALT	IRIS_ERHALT
#define	CACHE_ERWAITINIT	IRIS_ERWAITINIT
#define	CACHE_ERPRIMARY	IRIS_ERPRIMARY
#define	CACHE_ERCORRUPT	IRIS_ERCORRUPT
#define	CACHE_ERMAPS	IRIS_ERMAPS
#define	CACHE_ERVOLSET1	IRIS_ERVOLSET1
#define	CACHE_ERFORMAT	IRIS_ERFORMAT
#define	CACHE_ERMAPRSV	IRIS_ERMAPRSV
#define	CACHE_ERCOLLATE	IRIS_ERCOLLATE
#define	CACHE_ERZTRP	IRIS_ERZTRP
#define	CACHE_ERNODEV	IRIS_ERNODEV
#define	CACHE_ERLANG	IRIS_ERLANG
#define	CACHE_ERNOSYS	IRIS_ERNOSYS
#define	CACHE_ERGIMPL	IRIS_ERGIMPL
#define	CACHE_ERNGLOREF	IRIS_ERNGLOREF
#define	CACHE_ERNFORMAT	IRIS_ERNFORMAT
#define	CACHE_ERDSCON	IRIS_ERDSCON
#define	CACHE_ERNVERSION	IRIS_ERNVERSION
#define	CACHE_ERNJOBMAX	IRIS_ERNJOBMAX
#define	CACHE_ERNRETRY	IRIS_ERNRETRY
#define	CACHE_ERNAMADD	IRIS_ERNAMADD
#define	CACHE_ERNETLOCK	IRIS_ERNETLOCK
#define	CACHE_ERVALUE	IRIS_ERVALUE
#define	CACHE_ERNUMFIL	IRIS_ERNUMFIL
#define	CACHE_ERLIST	IRIS_ERLIST
#define	CACHE_ERNAMSP	IRIS_ERNAMSP
#define	CACHE_ERCMTFAIL	IRIS_ERCMTFAIL
#define	CACHE_ERROLLFAIL	IRIS_ERROLLFAIL
#define	CACHE_ERNSRVFAIL	IRIS_ERNSRVFAIL
#define	CACHE_ERCOLMISMAT	IRIS_ERCOLMISMAT
#define	CACHE_ERSLMSPAN	IRIS_ERSLMSPAN
#define	CACHE_ERWIDECHAR	IRIS_ERWIDECHAR
#define	CACHE_ERLINELEV	IRIS_ERLINELEV
#define	CACHE_ERARGDUP	IRIS_ERARGDUP
#define	CACHE_ERGLODEF	IRIS_ERGLODEF
#define	CACHE_ERCOLCHG	IRIS_ERCOLCHG
#define	CACHE_ERRESTART	IRIS_ERRESTART
#define	CACHE_ERROUDEF	IRIS_ERROUDEF
#define	CACHE_ERMNESPACE	IRIS_ERMNESPACE
#define	CACHE_ERTRANSLOCK	IRIS_ERTRANSLOCK
#define	CACHE_ERTRANSQUIT	IRIS_ERTRANSQUIT
#define	CACHE_ERECODETRAP	IRIS_ERECODETRAP
#define	CACHE_LOCKLOST	IRIS_LOCKLOST
#define	CACHE_DATALOST	IRIS_DATALOST
#define	CACHE_TRANSLOST	IRIS_TRANSLOST
#define	CACHE_ERZOLE	IRIS_ERZOLE
#define	CACHE_ERLICENSE	IRIS_ERLICENSE
#define	CACHE_ERNOLICSERV	IRIS_ERNOLICSERV
#define	CACHE_ERCOLNSUP	IRIS_ERCOLNSUP
#define	CACHE_ERNLSTAB	IRIS_ERNLSTAB
#define	CACHE_ERXLATE	IRIS_ERXLATE
#define	CACHE_ERNULLVAL	IRIS_ERNULLVAL
#define	CACHE_ERNOVAL	IRIS_ERNOVAL
#define	CACHE_ERUNKTYPE	IRIS_ERUNKTYPE
#define	CACHE_ERVALRANGE	IRIS_ERVALRANGE
#define	CACHE_ERRANGE	IRIS_ERRANGE
#define	CACHE_ERDOMAINSPACERETRY IRIS_ERDOMAINSPACERETRY
#define	CACHE_ERCSMISMATCH	IRIS_ERCSMISMATCH
#define	CACHE_ERSTRMISMATCH	IRIS_ERSTRMISMATCH
#define	CACHE_ERBADCLASS	IRIS_ERBADCLASS
#define	CACHE_ERIOBJECT	IRIS_ERIOBJECT
#define	CACHE_ERBADOREF	IRIS_ERBADOREF
#define	CACHE_ERNOMETHOD	IRIS_ERNOMETHOD
#define	CACHE_ERNOPROPERTY	IRIS_ERNOPROPERTY
#define	CACHE_ERNOCLASS	IRIS_ERNOCLASS
#define	CACHE_ERCLASSDESC	IRIS_ERCLASSDESC
#define	CACHE_ERCLASSCOMP	IRIS_ERCLASSCOMP
#define	CACHE_ERCLASSRECOMP	IRIS_ERCLASSRECOMP
#define	CACHE_ERCLASSMEMORY	IRIS_ERCLASSMEMORY
#define	CACHE_ERNUMCLASS	IRIS_ERNUMCLASS
#define	CACHE_ERNUMOREF	IRIS_ERNUMOREF
#define	CACHE_ERGETINVALID	IRIS_ERGETINVALID
#define	CACHE_ERSETINVALID	IRIS_ERSETINVALID
#define	CACHE_ERREMCLASSRECOMP	IRIS_ERREMCLASSRECOMP
#define	CACHE_ERREMCLASSEDITED	IRIS_ERREMCLASSEDITED
#define	CACHE_ERCLASSEDITED	IRIS_ERCLASSEDITED
#define	CACHE_ERCLASSLOAD	IRIS_ERCLASSLOAD
#define	CACHE_ERCLASSSIZE	IRIS_ERCLASSSIZE
#define	CACHE_ERCORRUPTOBJECT	IRIS_ERCORRUPTOBJECT
#define	CACHE_ERDISCONNECT	IRIS_ERDISCONNECT
#define	CACHE_ERNOCURRENTOBJECT	IRIS_ERNOCURRENTOBJECT
#define	CACHE_ERZFHEAPSPACE	IRIS_ERZFHEAPSPACE
#define	CACHE_ERTOOMANYCLASSES	IRIS_ERTOOMANYCLASSES
#define	CACHE_ERNETSTRMISMATCH IRIS_ERNETSTRMISMATCH
#define	CACHE_ERNETSSTACK IRIS_ERNETSSTACK
#define	CACHE_ERNETDIR IRIS_ERNETDIR
#define	CACHE_ERNETDKHER IRIS_ERNETDKHER
#define	CACHE_ERNETDBDGD IRIS_ERNETDBDGD
#define	CACHE_ERNETCLUSTER IRIS_ERNETCLUSTER
#define	CACHE_ERNETCSMISMATCH IRIS_ERNETCSMISMATCH
#define	CACHE_ERNETPROT IRIS_ERNETPROT
#define	CACHE_ERNETDKSER IRIS_ERNETDKSER
#define	CACHE_ERNETDKFUL IRIS_ERNETDKFUL
#define	CACHE_ERNETMXSTR IRIS_ERNETMXSTR
#define	CACHE_ERNETSBSCR IRIS_ERNETSBSCR
#define	CACHE_ERNETSYSTM IRIS_ERNETSYSTM
#define	CACHE_ERNETWIDECHAR IRIS_ERNETWIDECHAR
#define	CACHE_ERNETNVERSION	IRIS_ERNETNVERSION
#define	CACHE_ERNETNFORMAT	IRIS_ERNETNFORMAT
#define	CACHE_ERNETNGLOREF	IRIS_ERNETNGLOREF
#define	CACHE_ERBADBITSTR	IRIS_ERBADBITSTR
#define	CACHE_ERROUTINESIZE	IRIS_ERROUTINESIZE
#define	CACHE_ERNOSOURCE	IRIS_ERNOSOURCE
#define	CACHE_ERMALLOC	IRIS_ERMALLOC
#define	CACHE_EROREFNO	IRIS_EROREFNO
#define	CACHE_ERNOOREF	IRIS_ERNOOREF
#define	CACHE_ERINVARG	IRIS_ERINVARG
#define	CACHE_ERDYNLOAD	IRIS_ERDYNLOAD
#define	CACHE_NOJVM	IRIS_NOJVM
#define	CACHE_ERPRIVPROP	IRIS_ERPRIVPROP
#define	CACHE_ERPRIVMETH	IRIS_ERPRIVMETH
#define	CACHE_ERJAVAEXCEPTION	IRIS_ERJAVAEXCEPTION
#define	CACHE_ERNETLICENSE	IRIS_ERNETLICENSE
#define	CACHE_ERDDPJOBOVFL	IRIS_ERDDPJOBOVFL
#define	CACHE_ERLICENSEALLOC	IRIS_ERLICENSEALLOC
#define	CACHE_ERTRANLEVEL	IRIS_ERTRANLEVEL
#define	CACHE_ERRESJOB	IRIS_ERRESJOB
#define	CACHE_ERALARM	IRIS_ERALARM
#define	CACHE_ERLOSTTABOVER	IRIS_ERLOSTTABOVER
#define	CACHE_EREXTINT	IRIS_EREXTINT
#define	CACHE_ERNOFLOAT	IRIS_ERNOFLOAT
#define	CACHE_ERMVWRAPUP	IRIS_ERMVWRAPUP
#define	CACHE_ERUNLICENSED	IRIS_ERUNLICENSED
#define	CACHE_ERPROTDB	IRIS_ERPROTDB
#define	CACHE_ERARRDIM	IRIS_ERARRDIM
#define	CACHE_ERTHROW	IRIS_ERTHROW
#define	CACHE_ERNOTFILVAR	IRIS_ERNOTFILVAR
#define	CACHE_ERNOTSELLST	IRIS_ERNOTSELLST
#define	CACHE_ERCMPLXPAT	IRIS_ERCMPLXPAT
#define	CACHE_ERCLASSPROP	IRIS_ERCLASSPROP
#define	CACHE_ERMAXLOCKS	IRIS_ERMAXLOCKS
#define	CACHE_ERLOCKFULL	IRIS_ERLOCKFULL
#define	CACHE_ERSVRLOCKFULL	IRIS_ERSVRLOCKFULL
#define	CACHE_ERMAXROUTINES	IRIS_ERMAXROUTINES
#define	CACHE_EROUTOFHEAP	IRIS_EROUTOFHEAP
#define	CACHE_ERMAXINCR	IRIS_ERMAXINCR
#define	CACHE_ERWRONGNS	IRIS_ERWRONGNS
#define	CACHE_ERCALLBKSYN	IRIS_ERCALLBKSYN
#define	CACHE_EROBJDISP	IRIS_EROBJDISP
#define	CACHE_ERCOMMATCH	IRIS_ERCOMMATCH
#define	CACHE_ERRTNLIMIT	IRIS_ERRTNLIMIT
#define	CACHE_ERZFMAXSTR	IRIS_ERZFMAXSTR
#define	CACHE_ERTOOCOMPLEX	IRIS_ERTOOCOMPLEX
#define	CACHE_ERINVALGLOBALREF IRIS_ERINVALGLOBALREF
#define	CACHE_ERTCPWRITE	IRIS_ERTCPWRITE
#define	CACHE_ERUNSUPPMETHOD	IRIS_ERUNSUPPMETHOD
#define	CACHE_ERMNSSUPPORT	IRIS_ERMNSSUPPORT
#define	CACHE_ERTIMEOUT	IRIS_ERTIMEOUT
#define	CACHE_ERREXREAD	IRIS_ERREXREAD
#define	CACHE_ERREXWRITE	IRIS_ERREXWRITE
#define	CACHE_ERMAX	IRIS_ERMAX

#define	CachePopIEEEDbl	IrisPopIEEEDbl
#define	CACHEPOPIEEEDBL	IRISPOPIEEEDBL

#define	CACHEEFT IRISEFT

#ifndef	ZF_DLL		/*==============================*/

/*
			Function Prototypes

	These entry points are used when the user module is linked
	together with the Cache object module.
*/
#define	CacheStartA	IrisStartA
#define	CacheStartW	IrisStartW
#define	CacheSecureStartA	IrisSecureStartA
#define	CacheSecureStartW	IrisSecureStartW
#define	CacheChangePasswordA	IrisChangePasswordA
#define	CacheChangePasswordW	IrisChangePasswordW
#define	CacheExecuteA	IrisExecuteA
#define	CacheExecuteW	IrisExecuteW
#define	CacheEvalA	IrisEvalA
#define	CacheEvalW	IrisEvalW
#define	CacheContext	IrisContext
#define	CacheCvtInA	IrisCvtInA
#define	CacheCvtInW	IrisCvtInW
#define	CacheCvtOutA	IrisCvtOutA
#define	CacheCvtOutW	IrisCvtOutW
#define	CacheCvtExStrInA	IrisCvtExStrInA
#define	CacheCvtExStrInW	IrisCvtExStrInW
#define	CacheCvtExStrOutA	IrisCvtExStrOutA
#define	CacheCvtExStrOutW	IrisCvtExStrOutW
#define	CachePromptA	IrisPromptA
#define	CachePromptW	IrisPromptW
#define	CacheErrxlateA	IrisErrxlateA
#define	CacheErrxlateW	IrisErrxlateW
#define	CacheCtrl	IrisCtrl
#define	CacheConvert	IrisConvert
#define	CacheType	IrisType
#define	CacheAbort	IrisAbort
#define	CacheEnd	IrisEnd
#define	CacheOflush	IrisOflush
#define	CacheSignal	IrisSignal
#define	CacheErrorA	IrisErrorA
#define	CacheErrorW	IrisErrorW
#define	CachePushInt	IrisPushInt
#define	CachePushDbl	IrisPushDbl
#define	CachePushUndef	IrisPushUndef
#define	CachePushIEEEDbl	IrisPushIEEEDbl
#define	CachePushInt64	IrisPushInt64
#define	CachePushOref	IrisPushOref
#define	CachePushStr	IrisPushStr
#define	CachePushStrW	IrisPushStrW
#define	CachePushExStr	IrisPushExStr
#define	CachePushExStrW	IrisPushExStrW
#define	CachePushRtn	IrisPushRtn
#define	CachePushRtnX	IrisPushRtnX
#define	CachePushRtnW	IrisPushRtnW
#define	CachePushRtnXW	IrisPushRtnXW
#define	CachePushFunc	IrisPushFunc
#define	CachePushFuncX	IrisPushFuncX
#define	CachePushFuncW	IrisPushFuncW
#define	CachePushFuncXW	IrisPushFuncXW
#define	CachePushGlobal	IrisPushGlobal
#define	CachePushGlobalX	IrisPushGlobalX
#define	CachePushGlobalW	IrisPushGlobalW
#define	CachePushGlobalXW	IrisPushGlobalXW
#define	CacheUnPop	IrisUnPop
#define	CachePop	IrisPop
#define	CachePopInt	IrisPopInt
#define	CachePopInt64	IrisPopInt64
#define	CachePopDbl	IrisPopDbl
#define	CachePopOref	IrisPopOref
#define	CachePopStr	IrisPopStr
#define	CachePopStrW	IrisPopStrW
#define	CachePopExStr	IrisPopExStr
#define	CachePopExStrW	IrisPopExStrW
#define	CacheSetVar	IrisSetVar
#define	CacheGetVar	IrisGetVar
#define	CacheDoRtn	IrisDoRtn
#define	CacheDoFun	IrisDoFun
#define	CacheExtFun	IrisExtFun
#define CacheExecuteArgs	IrisExecuteArgs
#define CachePushExecuteFuncA	IrisPushExecuteFuncA
#define CachePushExecuteFuncW	IrisPushExecuteFuncW
#define CacheCallExecuteFunc	IrisCallExecuteFunc
#define	CacheGlobalGet	IrisGlobalGet
#define	CacheGlobalGetBinary	IrisGlobalGetBinary
#define	CacheGlobalSet		IrisGlobalSet
#define	CacheGlobalIncrement	IrisGlobalIncrement
#define	CachePushClassMethod	IrisPushClassMethod
#define	CachePushClassMethodW	IrisPushClassMethodW
#define	CachePushMethod		IrisPushMethod
#define	CachePushMethodW	IrisPushMethodW
#define	CacheInvokeClassMethod	IrisInvokeClassMethod
#define	CacheInvokeMethod	IrisInvokeMethod
#define	CachePushProperty	IrisPushProperty
#define	CachePushPropertyW	IrisPushPropertyW
#define	CacheGetProperty	IrisGetProperty
#define	CacheSetProperty	IrisSetProperty
#define	CacheIncrementCountOref	IrisIncrementCountOref
#define	CacheCloseOref	IrisCloseOref
#define	CachePushCvtW	IrisPushCvtW
#define	CachePopCvtW	IrisPopCvtW
#define	CachePushExStrCvtW	IrisPushExStrCvtW
#define	CachePopExStrCvtW	IrisPopExStrCvtW
#define	CachePushList	IrisPushList
#define	CachePopList	IrisPopList
#define	CachePushPtr	IrisPushPtr
#define	CachePopPtr	IrisPopPtr
#define	CacheGlobalQuery	IrisGlobalQuery
#define	CacheGlobalOrder	IrisGlobalOrder
#define	CacheGlobalData		IrisGlobalData
#define	CacheGlobalKill		IrisGlobalKill
#define	CacheGlobalRelease	IrisGlobalRelease
#define	CachePushLock	IrisPushLock
#define	CachePushLockX	IrisPushLockX
#define	CachePushLockW	IrisPushLockW
#define	CachePushLockXW	IrisPushLockXW
#define	CacheAcquireLock	IrisAcquireLock
#define	CacheReleaseLock	IrisReleaseLock
#define	CacheReleaseAllLocks	IrisReleaseAllLocks
#define	CacheTStart	IrisTStart
#define	CacheTCommit	IrisTCommit
#define	CacheTRollback	IrisTRollback
#define	CacheTLevel	IrisTLevel
#define	CacheSetDir	IrisSetDir

#if	CACHE_HSTR
#define	CacheStartH		IrisStartH
#define	CacheSecureStartH	IrisSecureStartH
#define	CacheChangePasswordH	IrisChangePasswordH
#define	CacheExecuteH	IrisExecuteH
#define	CacheEvalH	IrisEvalH
#define	CachePromptH	IrisPromptH
#define	CacheErrxlateH	IrisErrxlateH
#define	CacheErrorH	IrisErrorH
#define	CacheCvtInH	IrisCvtInH
#define	CacheCvtOutH	IrisCvtOutH
#define	CachePushStrH	IrisPushStrH
#define	CachePushExStrH	IrisPushExStrH
#define	CachePushExStrCvtH	IrisPushExStrCvtH
#define	CachePushRtnH	IrisPushRtnH
#define	CachePushRtnXH	IrisPushRtnXH
#define	CachePushFuncH	IrisPushFuncH
#define	CachePushFuncXH	IrisPushFuncXH
#define	CachePushGlobalH	IrisPushGlobalH
#define	CachePushGlobalXH	IrisPushGlobalXH
#define	CachePushClassMethodH	IrisPushClassMethodH
#define	CachePushMethodH	IrisPushMethodH
#define	CachePushPropertyH	IrisPushPropertyH
#define	CachePopStrH	IrisPopStrH
#define	CachePopExStrH	IrisPopExStrH
#define	CachePopExStrCvtH	IrisPopExStrCvtH
#define	CachePushCvtH	IrisPushCvtH
#define	CachePopCvtH	IrisPopCvtH
#define	CachePushLockH	IrisPushLockH
#define	CachePushLockXH	IrisPushLockXH
#define	CacheCvtExStrInH	IrisCvtExStrInH
#define	CacheCvtExStrOutH	IrisCvtExStrOutH
#define CachePushExecuteFuncH	IrisPushExecuteFuncH
#endif	/* CACHE_HSTR */

#define	CacheSPCSend	IrisSPCSend
#define	CacheSPCReceive	IrisSPCReceive
#define	CacheExStrKill	IrisExStrKill
#define	CacheExStrNew	IrisExStrNew
#define	CacheExStrNewW	IrisExStrNewW
#define	CacheExStrNewH	IrisExStrNewH
#define	CachePushExList	IrisPushExList
#define	CachePopExList	IrisPopExList
#define	CacheBitFind	IrisBitFind
#define	CacheBitFindB	IrisBitFindB
#define	CacheEndAll	IrisEndAll
#define	CachePushSSVN	IrisPushSSVN
#define	CachePushSSVNX	IrisPushSSVNX
#define	CachePushSSVNW	IrisPushSSVNW
#define	CachePushSSVNXW	IrisPushSSVNXW
#define	CacheAddSSVN	IrisAddSSVN
#define	CacheAddSSVNX	IrisAddSSVNX
#define	CacheAddSSVNW	IrisAddSSVNW
#define	CacheAddSSVNXW	IrisAddSSVNXW

#if	CACHE_HSTR
#define	CachePushSSVNH	IrisPushSSVNH
#define	CachePushSSVNXH	IrisPushSSVNXH
#define	CacheAddSSVNH	IrisAddSSVNH
#define	CacheAddSSVNXH	IrisAddSSVNXH
#define	CacheAddGlobalH	IrisAddGlobalH
#define	CacheAddGlobalXH	IrisAddGlobalXH
#endif	/* CACHE_HSTR */

#define	CacheAddGlobal	IrisAddGlobal
#define	CacheAddGlobalX	IrisAddGlobalX
#define	CacheAddGlobalW	IrisAddGlobalW
#define	CacheAddGlobalXW	IrisAddGlobalXW
#define	CacheAddSSVNDescriptor	IrisAddSSVNDescriptor
#define	CacheAddGlobalDescriptor	IrisAddGlobalDescriptor
#define	CacheAddLocalDescriptor	IrisAddLocalDescriptor
#define	CacheSSVNGet	IrisSSVNGet
#define	CacheSSVNKill	IrisSSVNKill
#define	CacheSSVNOrder	IrisSSVNOrder
#define	CacheSSVNQuery	IrisSSVNQuery
#define	CacheSSVNData	IrisSSVNData
#define	CacheMerge	IrisMerge
#define	CacheEnableMultiThread	IrisEnableMultiThread
#define	CacheiKnowFuncs	IrisiKnowFuncs

#define	CACHESTARTA(flags,tout,prinp,prout) CacheStartA(flags,tout,prinp,prout)
#define	CACHESTARTW(flags,tout,prinp,prout) CacheStartW(flags,tout,prinp,prout)
#define	CACHESECURESTARTA(username,password,exename,flags,tout,prinp,prout) CacheSecureStartA(username,password,exename,flags,tout,prinp,prout)
#define	CACHESECURESTARTW(username,password,exename,flags,tout,prinp,prout) CacheSecureStartW(username,password,exename,flags,tout,prinp,prout)
#define	CACHECHANGEPASSWORDA(username,oldpassword,newpassword) CacheChangePasswordA(username,oldpassword,newpassword)
#define	CACHECHANGEPASSWORDW(username,oldpassword,newpassword) CacheChangePasswordW(username,oldpassword,newpassword)
#define	CACHEEXECUTEA(cmd)	CacheExecuteA(cmd)
#define	CACHEEXECUTEW(cmd)	CacheExecuteW(cmd)
#define	CACHEEVALA(expr)	CacheEvalA(expr)
#define	CACHEEVALW(expr)	CacheEvalW(expr)
#define	CACHECONTEXT()		CacheContext()
#define	CACHEPROMPTA(rbuf)	CachePromptA(rbuf)
#define	CACHEPROMPTW(rbuf)	CachePromptW(rbuf)
#define	CACHEERRXLATEA(code,rbuf) CacheErrxlateA(code,rbuf)
#define	CACHEERRXLATEW(code,rbuf) CacheErrxlateW(code,rbuf)
#define	CACHEERRORA(msg,src,offp) CacheErrorA(msg,src,offp)
#define	CACHEERRORW(msg,src,offp) CacheErrorW(msg,src,offp)
#define	CACHECTRL(flags)	CacheCtrl(flags)
#define	CACHECONVERT(type,rbuf)	CacheConvert(type,rbuf)
#define	CACHETYPE()		CacheType()
#define	CACHEABORT(type)	CacheAbort(type)
#define	CACHEEND()		CacheEnd()
#define	CACHEENDALL()		CacheEndAll()
#define	CACHEOFLUSH()		CacheOflush()
#define	CACHECVTINA(src,tbl,res)  CacheCvtInA(src,tbl,res)
#define	CACHECVTINW(src,tbl,res)  CacheCvtInW(src,tbl,res)
#define	CACHECVTOUTA(src,tbl,res) CacheCvtOutA(src,tbl,res)
#define	CACHECVTOUTW(src,tbl,res) CacheCvtOutW(src,tbl,res)
#define	CACHECVTEXSTRINA(src,tbl,res)  CacheCvtExStrInA(src,tbl,res)
#define	CACHECVTEXSTRINW(src,tbl,res)  CacheCvtExStrInW(src,tbl,res)
#define	CACHECVTEXSTROUTA(src,tbl,res) CacheCvtExStrOutA(src,tbl,res)
#define	CACHECVTEXSTROUTW(src,tbl,res) CacheCvtExStrOutW(src,tbl,res)
#define	CACHESIGNAL(signal)	CacheSignal(signal)
#define	CACHEPUSHINT(num)	CachePushInt(num)
#define	CACHEPUSHDBL(num)	CachePushDbl(num)
#define	CACHEPUSHIEEEDBL(num)	CachePushIEEEDbl(num)
#define	CACHEPUSHOREF(oref)	CachePushOref(oref)
#define	CACHEPUSHUNDEF()	CachePushUndef()
#define	CACHEPUSHINT64(num)	CachePushInt64(num)
#define	CACHEPUSHSTRA(len,ptr)	(CachePushStr)(len,ptr)
#define	CACHEPUSHSTRW(len,ptr)	CachePushStrW(len,ptr)
#define	CACHEPUSHEXSTRA(sptr)	(CachePushExStr)(sptr)
#define	CACHEPUSHEXSTRW(sptr)	CachePushExStrW(sptr)
#define	CACHEPUSHRTNA(rflag,tlen,tptr,nlen,nptr)			\
	(CachePushRtn)(rflag,tlen,tptr,nlen,nptr)
#define	CACHEPUSHRTNXA(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)		\
	(CachePushRtnX)(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	CACHEPUSHRTNW(rflag,tlen,tptr,nlen,nptr)			\
	CachePushRtnW(rflag,tlen,tptr,nlen,nptr)
#define	CACHEPUSHRTNXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)		\
	CachePushRtnXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	CACHEPUSHFUNCA(rflag,tlen,tptr,nlen,nptr)			\
	(CachePushFunc)(rflag,tlen,tptr,nlen,nptr)
#define	CACHEPUSHFUNCXA(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	\
	(CachePushFuncX)(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	CACHEPUSHFUNCW(rflag,tlen,tptr,nlen,nptr)			\
	CachePushFuncW(rflag,tlen,tptr,nlen,nptr)
#define	CACHEPUSHFUNCXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	\
	CachePushFuncXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	CACHEPUSHGLOBALA(nlen,nptr) (CachePushGlobal)(nlen,nptr)
#define	CACHEPUSHGLOBALXA(nlen,nptr,elen,eptr)				\
	      (CachePushGlobalX)(nlen,nptr,elen,eptr)
#define	CACHEPUSHGLOBALW(nlen,nptr) CachePushGlobalW(nlen,nptr)
#define	CACHEPUSHGLOBALXW(nlen,nptr,elen,eptr)				\
	      CachePushGlobalXW(nlen,nptr,elen,eptr)
#define	CACHEUNPOP()		CacheUnPop()
#define	CACHEPOP(arg)		CachePop(arg)
#define	CACHEPOPINT(nump)	CachePopInt(nump)
#define	CACHEPOPDBL(nump)	CachePopDbl(nump)
#define	CACHEPOPOREF(orefp)	CachePopOref(orefp)
#define	CACHEPOPINT64(num)	CachePopInt64(num)
#define	CACHEPOPSTRA(lenp,strp)	(CachePopStr)(lenp,strp)
#define	CACHEPOPSTRW(lenp,strp)	CachePopStrW(lenp,strp)
#define	CACHEPOPEXSTRA(sstrp)	(CachePopExStr)(sstrp)
#define	CACHEPOPEXSTRW(sstrp)	CachePopExStrW(sstrp)
#define	CACHESETVAR(len,namep)	CacheSetVar(len,namep)
#define	CACHEGETVAR(len,namep)	CacheGetVar(len,namep)
#define	CACHEDORTN(flags,narg)	CacheDoRtn(flags,narg)
#define	CACHEEXTFUN(flags,narg)	CacheExtFun(flags,narg)
#define CACHEEXECUTEARGS(narg)	CacheExecuteArgs(narg)
#define CACHEPUSHEXECUTEFUNCA(len,ptr) CachePushExecuteFuncA(len,ptr)
#define CACHEPUSHEXECUTEFUNCW(len,ptr) CachePushExecuteFuncW(len,ptr)
#define CACHECALLEXECUTEFUNC(narg) CacheCallExecuteFunc(narg)
#define	CACHEGLOBALGET(narg,flag) CacheGlobalGet(narg,flag)
#define	CACHEGLOBALSET(narg)	CacheGlobalSet(narg)
#define	CACHEGLOBALINCREMENT(narg) CacheGlobalIncrement(narg)
#define	CACHEGLOBALQUERY(narg,dir,valueflag) \
		 CacheGlobalQuery(narg,dir,valueflag)
#define	CACHEGLOBALORDER(narg,dir,valueflag) \
		 CacheGlobalOrder(narg,dir,valueflag)
#define	CACHEGLOBALDATA(narg,valueflag) CacheGlobalData(narg,valueflag)
#define	CACHEGLOBALKILL(narg,nodeonly) CacheGlobalKill(narg,nodeonly)
#define	CACHEGLOBALRELEASE() CacheGlobalRelease()
#define	CACHEPUSHLOCKA(nlen,nptr) (CachePushLock)(nlen,nptr)
#define	CACHEPUSHLOCKXA(nlen,nptr,elen,eptr)				\
    (CachePushLockX)(nlen,nptr,elen,eptr)
#define	CACHEPUSHLOCKW(nlen,nptr) CachePushLockW(nlen,nptr)
#define	CACHEPUSHLOCKXW(nlen,nptr,elen,eptr)				\
    CachePushLockXW(nlen,nptr,elen,eptr)
#define	CACHEDOFUN(flags,narg)	CacheDoFun(flags,narg)
#define	CACHEPUSHCLASSMETHODA(clen,cptr,mlen,mptr,flg)			\
		(CachePushClassMethod)(clen,cptr,mlen,mptr,flg)
#define	CACHEPUSHCLASSMETHODW(clen,cptr,mlen,mptr,flg)			\
	       CachePushClassMethodW(clen,cptr,mlen,mptr,flg)
#define	CACHEPUSHMETHODA(oref,mlen,mptr,flg)				\
		(CachePushMethod)(oref,mlen,mptr,flg)
#define	CACHEPUSHMETHODW(oref,mlen,mptr,flg)				\
		CachePushMethodW(oref,mlen,mptr,flg)
#define	CACHEINVOKECLASSMETHOD(narg) CacheInvokeClassMethod(narg)
#define	CACHEINVOKEMETHOD(narg)	 CacheInvokeMethod(narg)
#define	CACHEPUSHPROPERTYA(oref,plen,pptr)				\
		(CachePushProperty)(oref,plen,pptr)
#define	CACHEPUSHPROPERTYW(oref,plen,pptr) CachePushPropertyW(oref,plen,pptr)
#define	CACHEGETPROPERTY()	CacheGetProperty()
#define	CACHESETPROPERTY()	CacheSetProperty()
#define	CACHEINCREMENTCOUNTOREF(oref) CacheIncrementCountOref(oref)
#define	CACHECLOSEOREF(oref)	CacheCloseOref(oref)
#define	CACHEPUSHCVTW(len,ptr)	CachePushCvtW(len,ptr)
#define	CACHEPOPCVTW(lenp,strp)	CachePopCvtW(lenp,strp)
#define	CACHEPUSHEXSTRCVTW(sstrp)	CachePushExStrCvtW(sstrp)
#define	CACHEPOPEXSTRCVTW(sstrp)	CachePopExStrCvtW(sstrp)
#define	CACHEPUSHLIST(len,ptr)	CachePushList(len,ptr)
#define	CACHEPOPLIST(lenp,strp)	CachePopList(lenp,strp)
#define	CACHEPUSHPTR(ptr)	CachePushPtr(ptr)
#define	CACHEPOPPTR(ptrp)	CachePopPtr(ptrp)
#define	CACHEACQUIRELOCK(nsub,flg,tout,rval) CacheAcquireLock(nsub,flg,tout,rval)
#define	CACHERELEASELOCK(nsub,flg) CacheReleaseLock(nsub,flg)
#define	CACHERELEASEALLLOCKS() CacheReleaseAllLocks()
#define	CACHETSTART() CacheTStart()
#define	CACHETCOMMIT() CacheTCommit()
#define	CACHETROLLBACK(nlev) CacheTRollback(nlev)
#define	CACHETLEVEL() CacheTLevel()
#define	CACHESPCSEND(len,ptr) CacheSPCSend(len,ptr)
#define	CACHESPCRECEIVE(lenp,ptr) CacheSPCReceive(lenp,ptr)
#define	CACHESETDIR(dir) CacheSetDir(dir)

#if	CACHE_HSTR
#define	CACHESTARTH(flags,tout,prinp,prout) CacheStartH(flags,tout,prinp,prout)
#define	CACHESECURESTARTH(username,password,exename,flags,tout,prinp,prout) CacheSecureStartH(username,password,exename,flags,tout,prinp,prout)
#define	CACHECHANGEPASSWORDH(username,oldpassword,newpassword) CacheChangePasswordH(username,oldpassword,newpassword)
#define	CACHEEXECUTEH(cmd)	CacheExecuteH(cmd)
#define	CACHEEVALH(expr)	CacheEvalH(expr)
#define	CACHEPROMPTH(rbuf)	CachePromptH(rbuf)
#define	CACHEERRXLATEH(code,rbuf) CacheErrxlateH(code,rbuf)
#define	CACHEERRORH(msg,src,offp) CacheErrorH(msg,src,offp)
#define	CACHECVTINH(src,tbl,res) CacheCvtInH(src,tbl,res)
#define	CACHECVTOUTH(src,tbl,res) CacheCvtOutH(src,tbl,res)
#define	CACHEPUSHSTRH(len,ptr)	CachePushStrH(len,ptr)
#define	CACHEPUSHEXSTRH(sptr)	CachePushExStrH(sptr)
#define	CACHEPUSHEXSTRCVTH(sptr) CachePushExStrCvtH(sptr)
#define	CACHEPUSHRTNH(rflag,tlen,tptr,nlen,nptr)			\
	CachePushRtnH(rflag,tlen,tptr,nlen,nptr)
#define	CACHEPUSHRTNXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)		\
	CachePushRtnXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	CACHEPUSHFUNCH(rflag,tlen,tptr,nlen,nptr)			\
	CachePushFuncH(rflag,tlen,tptr,nlen,nptr)
#define	CACHEPUSHFUNCXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	\
	CachePushFuncXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	CACHEPUSHGLOBALH(nlen,nptr) CachePushGlobalH(nlen,nptr)
#define	CACHEPUSHGLOBALXH(nlen,nptr,elen,eptr)				\
	CachePushGlobalXH(nlen,nptr,elen,eptr)
#define	CACHEPUSHCLASSMETHODH(clen,cptr,mlen,mptr,flg)			\
	CachePushClassMethodH(clen,cptr,mlen,mptr,flg)
#define	CACHEPUSHMETHODH(oref,mlen,mptr,flg)				\
	CachePushMethodH(oref,mlen,mptr,flg)
#define	CACHEPUSHPROPERTYH(oref,plen,pptr) CachePushPropertyH(oref,plen,pptr)
#define	CACHEPOPSTRH(lenp,strp)	CachePopStrH(lenp,strp)
#define	CACHEPOPEXSTRH(sstrp)	CachePopExStrH(sstrp)
#define	CACHEPOPEXSTRCVTH(sstrp) CachePopExStrCvtH(sstrp)
#define	CACHEPUSHCVTH(len,ptr)	CachePushCvtH(len,ptr)
#define	CACHEPOPCVTH(lenp,strp)	CachePopCvtH(lenp,strp)
#define	CACHEPUSHLOCKH(nlen,nptr) CachePushLockH(nlen,nptr)
#define	CACHEPUSHLOCKXH(nlen,nptr,elen,eptr) CachePushLockXH(nlen,nptr,elen,eptr)
#define	CACHECVTEXSTRINH(src,tbl,res) CacheCvtExStrInH(src,tbl,res)
#define	CACHECVTEXSTROUTH(src,tbl,res) CacheCvtExStrOutH(src,tbl,res)
#define CACHEPUSHEXECUTEFUNCH(len,ptr) CachePushExecuteFuncH(len,ptr)
#else	/* CACHE_HSTR */
#define	CACHESTARTH(flags,tout,prinp,prout) CacheStartW(flags,tout,prinp,prout)
#define	CACHESECURESTARTH(username,password,exename,flags,tout,prinp,prout) CacheSecureStartW(username,password,exename,flags,tout,prinp,prout)
#define	CACHECHANGEPASSWORDH(username,oldpassword,newpassword) CacheChangePasswordW(username,oldpassword,newpassword)
#define	CACHEEXECUTEH(cmd)	CacheExecuteW(cmd)
#define	CACHEEVALH(expr)	CacheEvalW(expr)
#define	CACHEPROMPTH(rbuf)	CachePromptW(rbuf)
#define	CACHEERRXLATEH(code,rbuf) CacheErrxlateW(code,rbuf)
#define	CACHEERRORH(msg,src,offp) CacheErrorW(msg,src,offp)
#define	CACHECVTINH(src,tbl,res) CacheCvtInW(src,tbl,res)
#define	CACHECVTOUTH(src,tbl,res) CacheCvtOutW(src,tbl,res)
#define	CACHEPUSHSTRH(len,ptr)	CachePushStrW(len,ptr)
#define	CACHEPUSHEXSTRH(sptr)	CachePushExStrW(sptr)
#define	CACHEPUSHEXSTRCVTH(sptr) CachePushExStrCvtW(sptr)
#define	CACHEPUSHRTNH(rflag,tlen,tptr,nlen,nptr)			\
	CachePushRtnW(rflag,tlen,tptr,nlen,nptr)
#define	CACHEPUSHRTNXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)		\
	CachePushRtnXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	CACHEPUSHFUNCH(rflag,tlen,tptr,nlen,nptr)			\
	CachePushFuncW(rflag,tlen,tptr,nlen,nptr)
#define	CACHEPUSHFUNCXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	\
	CachePushFuncXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	CACHEPUSHGLOBALH(nlen,nptr) CachePushGlobalW(nlen,nptr)
#define	CACHEPUSHGLOBALXH(nlen,nptr,elen,eptr)				\
	CachePushGlobalXW(nlen,nptr,elen,eptr)
#define	CACHEPUSHCLASSMETHODH(clen,cptr,mlen,mptr,flg)			\
	CachePushClassMethodW(clen,cptr,mlen,mptr,flg)
#define	CACHEPUSHMETHODH(oref,mlen,mptr,flg)				\
	CachePushMethodW(oref,mlen,mptr,flg)
#define	CACHEPUSHPROPERTYH(oref,plen,pptr) CachePushPropertyW(oref,plen,pptr)
#define	CACHEPOPSTRH(lenp,strp)	CachePopStrW(lenp,strp)
#define	CACHEPOPEXSTRH(sstrp)	CachePopExStrW(sstrp)
#define	CACHEPOPEXSTRCVTH(sstrp) CachePopExStrW(sstrp)
#define	CACHEPUSHCVTH(len,ptr)	CachePushCvtW(len,ptr)
#define	CACHEPOPCVTH(lenp,strp)	CachePopCvtW(lenp,strp)
#define	CACHEPUSHLOCKH(nlen,nptr) CachePushLockW(nlen,nptr)
#define	CACHEPUSHLOCKXH(nlen,nptr,elen,eptr)				\
	CachePushLockXW(nlen,nptr,elen,eptr)
#define	CACHECVTEXSTRINH(src,tbl,res) CacheCvtExStrInW(src,tbl,res)
#define	CACHECVTEXSTROUTH(src,tbl,res) CacheCvtExStrOutW(src,tbl,res)
#define CACHEPUSHEXECUTEFUNCH(len,ptr) CachePushExecuteFuncW(len,ptr)
#endif	/* CACHE_HSTR */

#define	CACHEEXSTRKILL(obj) CacheExStrKill(obj)
#define	CACHEEXSTRNEWA(zstr,size) (CacheExStrNew)(zstr,size)
#define	CACHEEXSTRNEWW(zstr,size) CacheExStrNewW(zstr,size)
#define	CACHEEXSTRNEWH(zstr,size) CacheExStrNewH(zstr,size)
#define	CACHEPUSHEXLIST(sstr) CachePushExList(sstr)
#define	CACHEPOPEXLIST(sstr) CachePopExList(sstr)
#define	CACHEPUSHSSVNA(nlen,nptr) (CachePushSSVN)(nlen,nptr)
#define	CACHEPUSHSSVNXA(nlen,nptr,elen,eptr)				\
	(CachePushSSVNX)(nlen,nptr,elen,eptr)
#define	CACHEPUSHSSVNW(nlen,nptr) CachePushSSVNW(nlen,nptr)
#define	CACHEPUSHSSVNXW(nlen,nptr,elen,eptr)				\
	CachePushSSVNXW(nlen,nptr,elen,eptr)
#define	CACHEADDSSVNA(nlen,nptr) (CacheAddSSVN)(nlen,nptr)
#define	CACHEADDSSVNXA(nlen,nptr,elen,eptr)				\
	(CacheAddSSVNX)(nlen,nptr,elen,eptr)
#define	CACHEADDSSVNW(nlen,nptr) CacheAddSSVNW(nlen,nptr)
#define	CACHEADDSSVNXW(nlen,nptr,elen,eptr)				\
	CacheAddSSVNXW(nlen,nptr,elen,eptr)

#if	CACHE_HSTR
#define	CACHEPUSHSSVNH(nlen,nptr) CachePushSSVNH(nlen,nptr)
#define	CACHEPUSHSSVNXH(nlen,nptr,elen,eptr)				\
	CachePushSSVNXH(nlen,nptr,elen,eptr)
#define	CACHEADDSSVNH(nlen,nptr) CacheAddSSVNH(nlen,nptr)
#define	CACHEADDSSVNXH(nlen,nptr,elen,eptr)				\
	CacheAddSSVNXH(nlen,nptr,elen,eptr)
#define	CACHEADDGLOBALH(nlen,nptr) CacheAddGlobalH(nlen,nptr)
#define	CACHEADDGLOBALXH(nlen,nptr,elen,eptr)				\
	CacheAddGlobalXH(nlen,nptr,elen,eptr)
#else	/* CACHE_HSTR */
#define	CACHEPUSHSSVNH(nlen,nptr) CachePushSSVNW(nlen,nptr)
#define	CACHEPUSHSSVNXH(nlen,nptr,elen,eptr)				\
	CachePushSSVNXW(nlen,nptr,elen,eptr)
#define	CACHEADDSSVNH(nlen,nptr) CacheAddSSVNW(nlen,nptr)
#define	CACHEADDSSVNXH(nlen,nptr,elen,eptr)				\
	CacheAddSSVNXW(nlen,nptr,elen,eptr)
#define	CACHEADDGLOBALH(nlen,nptr) CacheAddGlobalW(nlen,nptr)
#define	CACHEADDGLOBALXH(nlen,nptr,elen,eptr)				\
	CacheAddGlobalXW(nlen,nptr,elen,eptr)
#endif	/* CACHE_HSTR */
#define	CACHEADDGLOBALA(nlen,nptr) (CacheAddGlobal)(nlen,nptr)
#define	CACHEADDGLOBALXA(nlen,nptr,elen,eptr)				\
	(CacheAddGlobalX)(nlen,nptr,elen,eptr)
#define	CACHEADDGLOBALW(nlen,nptr) CacheAddGlobalW(nlen,nptr)
#define	CACHEADDGLOBALXW(nlen,nptr,elen,eptr)				\
	CacheAddGlobalXW(nlen,nptr,elen,eptr)
#define	CACHEADDSSVNDESCRIPTOR(num)	CacheAddSSVNDescriptor(num)
#define	CACHEADDGLOBALDESCRIPTOR(num)	CacheAddGlobalDescriptor(num)
#define	CACHEADDLOCALDESCRIPTOR(num)	CacheAddLocalDescriptor(num)
#define	CACHESSVNGET(num)		CacheSSVNGet(num)
#define	CACHESSVNKILL(num,val)		CacheSSVNKill(num,val)
#define	CACHESSVNORDER(num,val)		CacheSSVNOrder(num,val)
#define	CACHESSVNQUERY(num,val)		CacheSSVNQuery(num,val)
#define	CACHESSVNDATA(num)		CacheSSVNData(num)
#define	CACHEMERGE()			CacheMerge()
#define	CACHEENABLEMULTITHREAD()	CacheEnableMultiThread()
#define	CACHEIKNOWFUNCS(funcsp)		CacheiKnowFuncs(funcsp)

#else	/* ZF_DLL */	/*==============================*/
/*
		Interface Function Definitions

	The export function table entries are used for calling back
	into Cache from a dynamically loaded user library module,
	i.e. one loaded using $ZF(-n).
*/
#define	CACHEEXECUTEA(cmd)	 IRISEXECUTEA(cmd)
#define	CACHEEXECUTEW(cmd)	 IRISEXECUTEW(cmd)
#define	CACHEEVALA(expr)	 IRISEVALA(expr)
#define	CACHEEVALW(expr)	 IRISEVALW(expr)
#define	CACHEPROMPTA(rbuf)	 IRISPROMPTA(rbuf)
#define	CACHEPROMPTW(rbuf)	 IRISPROMPTW(rbuf)
#define	CACHEERRXLATEA(code,rbuf)  IRISERRXLATEA(code,rbuf)
#define	CACHEERRXLATEW(code,rbuf)  IRISERRXLATEW(code,rbuf)
#define	CACHEERRORA(msg,src,offp)  IRISERRORA(msg,src,offp)
#define	CACHEERRORW(msg,src,offp)  IRISERRORW(msg,src,offp)
#define	CACHECONVERT(type,rbuf)	 IRISCONVERT(type,rbuf)
#define	CACHETYPE()		 IRISTYPE()
#define	CACHEABORT(type)	 IRISABORT(type)
#define	CACHESIGNAL(signal)	 IRISSIGNAL(signal)
#define	CACHEPUSHINT(num)	 IRISPUSHINT(num)
#define	CACHEPUSHDBL(num)	 IRISPUSHDBL(num)
#define	CACHEPUSHIEEEDBL(num)	 IRISPUSHIEEEDBL(num)
#define	CACHEPUSHOREF(oref)	 IRISPUSHOREF(oref)
#define	CACHEPUSHUNDEF()	 IRISPUSHUNDEF()
#define	CACHEPUSHINT64(num)	 IRISPUSHINT64(num)
#define	CACHEPUSHSTRA(len,ptr)	 IRISPUSHSTRA(len,ptr)
#define	CACHEPUSHSTRW(len,ptr)	 IRISPUSHSTRW(len,ptr)
#define	CACHEPUSHEXSTRA(sptr)	 IRISPUSHEXSTRA(sptr)
#define	CACHEPUSHEXSTRW(sptr)	 IRISPUSHEXSTRW(sptr)
#define	CACHEPUSHRTNA(rflag,tlen,tptr,nlen,nptr)	 IRISPUSHRTNA(rflag,tlen,tptr,nlen,nptr)
#define	CACHEPUSHRTNXA(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	 IRISPUSHRTNXA(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	CACHEPUSHRTNW(rflag,tlen,tptr,nlen,nptr)	 IRISPUSHRTNW(rflag,tlen,tptr,nlen,nptr)
#define	CACHEPUSHRTNXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	 IRISPUSHRTNXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	CACHEPUSHFUNCA(rflag,tlen,tptr,nlen,nptr)	 IRISPUSHFUNCA(rflag,tlen,tptr,nlen,nptr)
#define	CACHEPUSHFUNCXA(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	 IRISPUSHFUNCXA(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	CACHEPUSHFUNCW(rflag,tlen,tptr,nlen,nptr)	 IRISPUSHFUNCW(rflag,tlen,tptr,nlen,nptr)
#define	CACHEPUSHFUNCXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	 IRISPUSHFUNCXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	CACHEPUSHGLOBALA(nlen,nptr)  IRISPUSHGLOBALA(nlen,nptr)
#define	CACHEPUSHGLOBALXA(nlen,nptr,elen,eptr)	 IRISPUSHGLOBALXA(nlen,nptr,elen,eptr)
#define	CACHEPUSHGLOBALW(nlen,nptr)  IRISPUSHGLOBALW(nlen,nptr)
#define	CACHEPUSHGLOBALXW(nlen,nptr,elen,eptr)	 IRISPUSHGLOBALXW(nlen,nptr,elen,eptr)
#define	CACHEUNPOP()	 IRISUNPOP()
#define	CACHEPOP(arg)	 IRISPOP(arg)
#define	CACHEPOPINT(nump)	 IRISPOPINT(nump)
#define	CACHEPOPDBL(nump)	 IRISPOPDBL(nump)
#define	CACHEPOPOREF(orefp)	 IRISPOPOREF(orefp)
#define	CACHEPOPINT64(num)	 IRISPOPINT64(num)
#define	CACHEPOPSTRA(lenp,strp)	 IRISPOPSTRA(lenp,strp)
#define	CACHEPOPSTRW(lenp,strp)	 IRISPOPSTRW(lenp,strp)
#define	CACHEPOPEXSTRA(sstrp)	 IRISPOPEXSTRA(sstrp)
#define	CACHEPOPEXSTRW(sstrp)	 IRISPOPEXSTRW(sstrp)
#define	CACHESETVAR(len,namep)   IRISSETVAR(len,namep)
#define	CACHEGETVAR(len,namep)   IRISGETVAR(len,namep)
#define	CACHEDORTN(flags,narg)	 IRISDORTN(flags,narg)
#define	CACHEEXTFUN(flags,narg)	 IRISEXTFUN(flags,narg)
#define CACHEEXECUTEARGS(narg)	 IRISEXECUTEARGS(narg)
#define CACHEPUSHEXECUTEFUNCA(len,ptr) IRISPUSHEXECUTEFUNCA(len,ptr)
#define CACHEPUSHEXECUTEFUNCW(len,ptr) IRISPUSHEXECUTEFUNCW(len,ptr)
#define CACHECALLEXECUTEFUNC(narg)     IRISCALLEXECUTEFUNC(narg)
#define	CACHEGLOBALGET(narg,flag)  IRISGLOBALGET(narg,flag)
#define	CACHEGLOBALSET(narg)	   IRISGLOBALSET(narg)
#define	CACHEGLOBALINCREMENT(narg)  IRISGLOBALINCREMENT(narg)
#define	CACHEGLOBALQUERY(narg,dir,valueflag)  IRISGLOBALQUERY(narg,dir,valueflag)
#define	CACHEGLOBALORDER(narg,dir,valueflag)  IRISGLOBALORDER(narg,dir,valueflag)
#define	CACHEGLOBALDATA(narg,valueflag)  IRISGLOBALDATA(narg,valueflag)
#define	CACHEGLOBALKILL(narg,nodeonly)  IRISGLOBALKILL(narg,nodeonly)
#define	CACHEGLOBALRELEASE()  IRISGLOBALRELEASE()
#define	CACHEDOFUN(flags,narg)	 IRISDOFUN(flags,narg)
#define	CACHEPUSHCLASSMETHODA(clen,cptr,mlen,mptr,flg)	 IRISPUSHCLASSMETHODA(clen,cptr,mlen,mptr,flg)
#define	CACHEPUSHCLASSMETHODW(clen,cptr,mlen,mptr,flg)	 IRISPUSHCLASSMETHODW(clen,cptr,mlen,mptr,flg)
#define	CACHEPUSHMETHODA(oref,mlen,mptr,flg)	 IRISPUSHMETHODA(oref,mlen,mptr,flg)
#define	CACHEPUSHMETHODW(oref,mlen,mptr,flg)	 IRISPUSHMETHODW(oref,mlen,mptr,flg)
#define	CACHEINVOKECLASSMETHOD(narg)	 IRISINVOKECLASSMETHOD(narg)
#define	CACHEINVOKEMETHOD(narg)	 IRISINVOKEMETHOD(narg)
#define	CACHEPUSHPROPERTYA(oref,plen,pptr)	 IRISPUSHPROPERTYA(oref,plen,pptr)
#define	CACHEPUSHPROPERTYW(oref,plen,pptr)	 IRISPUSHPROPERTYW(oref,plen,pptr)
#define	CACHEGETPROPERTY()	 IRISGETPROPERTY()
#define	CACHESETPROPERTY()	 IRISSETPROPERTY()
#define	CACHEINCREMENTCOUNTOREF(oref)	 IRISINCREMENTCOUNTOREF(oref)
#define	CACHECLOSEOREF(oref)	 IRISCLOSEOREF(oref)
#define	CACHEPUSHCVTW(len,ptr)	 IRISPUSHCVTW(len,ptr)
#define	CACHEPOPCVTW(lenp,strp)	 IRISPOPCVTW(lenp,strp)
#define	CACHEPUSHLIST(len,ptr)	 IRISPUSHLIST(len,ptr)
#define	CACHEPOPLIST(lenp,strp)	 IRISPOPLIST(lenp,strp)
#define	CACHEPUSHPTR(ptr)	 IRISPUSHPTR(ptr)
#define	CACHEPOPPTR(ptrp)	 IRISPOPPTR(ptrp)
#define	CACHECONTEXT()	 IRISCONTEXT()
#define	CACHECTRL(flags)	 IRISCTRL(flags)
#define	CACHECVTINA(src,tbl,res)  IRISCVTINA(src,tbl,res)
#define	CACHECVTINW(src,tbl,res)  IRISCVTINW(src,tbl,res)
#define	CACHECVTOUTA(src,tbl,res)  IRISCVTOUTA(src,tbl,res)
#define	CACHECVTOUTW(src,tbl,res)  IRISCVTOUTW(src,tbl,res)
#define	CACHECVTEXSTRINA(src,tbl,res)  IRISCVTEXSTRINA(src,tbl,res)
#define	CACHECVTEXSTRINW(src,tbl,res)  IRISCVTEXSTRINW(src,tbl,res)
#define	CACHECVTEXSTROUTA(src,tbl,res)  IRISCVTEXSTROUTA(src,tbl,res)
#define	CACHECVTEXSTROUTW(src,tbl,res)  IRISCVTEXSTROUTW(src,tbl,res)
#define	CACHEPUSHLOCKA(nlen,nptr)  IRISPUSHLOCKA(nlen,nptr)
#define	CACHEPUSHLOCKXA(nlen,nptr,elen,eptr)	 IRISPUSHLOCKXA(nlen,nptr,elen,eptr)
#define	CACHEPUSHLOCKW(nlen,nptr)  IRISPUSHLOCKW(nlen,nptr)
#define	CACHEPUSHLOCKXW(nlen,nptr,elen,eptr)	 IRISPUSHLOCKXW(nlen,nptr,elen,eptr)
#define	CACHEACQUIRELOCK(nsub,flg,tout,rval)  IRISACQUIRELOCK(nsub,flg,tout,rval)
#define	CACHERELEASELOCK(nsub,flg)  IRISRELEASELOCK(nsub,flg)
#define	CACHERELEASEALLLOCKS()  IRISRELEASEALLLOCKS()
#define	CACHETSTART()  IRISTSTART()
#define	CACHETCOMMIT()  IRISTCOMMIT()
#define	CACHETROLLBACK(nlev)  IRISTROLLBACK(nlev)
#define	CACHETLEVEL()  IRISTLEVEL()
#define	CACHESPCSEND(len,ptr)  IRISSPCSEND(len,ptr)
#define	CACHESPCRECEIVE(lenp,ptr)  IRISSPCRECEIVE(lenp,ptr)
#define	CACHEEXECUTEH(cmd)	 IRISEXECUTEH(cmd)
#define	CACHEEVALH(expr)	 IRISEVALH(expr)
#define	CACHEPROMPTH(rbuf)	 IRISPROMPTH(rbuf)
#define	CACHEERRXLATEH(code,rbuf)  IRISERRXLATEH(code,rbuf)
#define	CACHEERRORH(msg,src,offp)  IRISERRORH(msg,src,offp)
#define	CACHECVTINH(src,tbl,res)  IRISCVTINH(src,tbl,res)
#define	CACHECVTOUTH(src,tbl,res)  IRISCVTOUTH(src,tbl,res)
#define	CACHEPUSHSTRH(len,ptr)	 IRISPUSHSTRH(len,ptr)
#define	CACHEPUSHEXSTRH(sptr)	 IRISPUSHEXSTRH(sptr)
#define	CACHEPUSHEXSTRCVTH(sptr)  IRISPUSHEXSTRCVTH(sptr)
#define	CACHEPUSHRTNH(rflag,tlen,tptr,nlen,nptr)	 IRISPUSHRTNH(rflag,tlen,tptr,nlen,nptr)
#define	CACHEPUSHRTNXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	 IRISPUSHRTNXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	CACHEPUSHFUNCH(rflag,tlen,tptr,nlen,nptr)	 IRISPUSHFUNCH(rflag,tlen,tptr,nlen,nptr)
#define	CACHEPUSHFUNCXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	 IRISPUSHFUNCXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	CACHEPUSHGLOBALH(nlen,nptr)  IRISPUSHGLOBALH(nlen,nptr)
#define	CACHEPUSHGLOBALXH(nlen,nptr,elen,eptr)	 IRISPUSHGLOBALXH(nlen,nptr,elen,eptr)
#define	CACHEPUSHCLASSMETHODH(clen,cptr,mlen,mptr,flg)	 IRISPUSHCLASSMETHODH(clen,cptr,mlen,mptr,flg)
#define	CACHEPUSHMETHODH(oref,mlen,mptr,flg)	 IRISPUSHMETHODH(oref,mlen,mptr,flg)
#define	CACHEPUSHPROPERTYH(oref,plen,pptr)	 IRISPUSHPROPERTYH(oref,plen,pptr)
#define	CACHEPOPSTRH(lenp,strp)	 IRISPOPSTRH(lenp,strp)
#define	CACHEPOPEXSTRH(sstrp)	 IRISPOPEXSTRH(sstrp)
#define	CACHEPOPEXSTRCVTH(sstrp)  IRISPOPEXSTRCVTH(sstrp)
#define	CACHEPUSHCVTH(len,ptr)	 IRISPUSHCVTH(len,ptr)
#define	CACHEPOPCVTH(lenp,strp)	 IRISPOPCVTH(lenp,strp)
#define	CACHEPUSHLOCKH(nlen,nptr)  IRISPUSHLOCKH(nlen,nptr)
#define	CACHEPUSHLOCKXH(nlen,nptr,elen,eptr)	 IRISPUSHLOCKXH(nlen,nptr,elen,eptr)
#define	CACHECVTEXSTRINH(src,tbl,res)  IRISCVTEXSTRINH(src,tbl,res)
#define	CACHECVTEXSTROUTH(src,tbl,res) IRISCVTEXSTROUTH(src,tbl,res)
#define CACHEPUSHEXECUTEFUNCH(len,ptr) IRISPUSHEXECUTEFUNCH(len,ptr)
#define	CACHEEXSTRKILL(obj)  IRISEXSTRKILL(obj)
#define	CACHEEXSTRNEWA(zstr,size)  IRISEXSTRNEWA(zstr,size)
#define	CACHEEXSTRNEWW(zstr,size)  IRISEXSTRNEWW(zstr,size)
#define	CACHEEXSTRNEWH(zstr,size)  IRISEXSTRNEWH(zstr,size)
#define	CACHEPUSHEXLIST(sstr)  IRISPUSHEXLIST(sstr)
#define	CACHEPOPEXLIST(sstr)  IRISPOPEXLIST(sstr)
#define	CACHEPUSHEXSTRCVTW(sstrp)	 IRISPUSHEXSTRCVTW(sstrp)
#define	CACHEPOPEXSTRCVTW(sstrp)	 IRISPOPEXSTRCVTW(sstrp)
#define	CACHEPUSHSSVNA(nlen,nptr)  IRISPUSHSSVNA(nlen,nptr)
#define	CACHEPUSHSSVNXA(nlen,nptr,elen,eptr)	 IRISPUSHSSVNXA(nlen,nptr,elen,eptr)
#define	CACHEPUSHSSVNW(nlen,nptr)  IRISPUSHSSVNW(nlen,nptr)
#define	CACHEPUSHSSVNXW(nlen,nptr,elen,eptr)	 IRISPUSHSSVNXW(nlen,nptr,elen,eptr)
#define	CACHEADDSSVNA(nlen,nptr)  IRISADDSSVNA(nlen,nptr)
#define	CACHEADDSSVNXA(nlen,nptr,elen,eptr)	 IRISADDSSVNXA(nlen,nptr,elen,eptr)
#define	CACHEADDSSVNW(nlen,nptr)  IRISADDSSVNW(nlen,nptr)
#define	CACHEADDSSVNXW(nlen,nptr,elen,eptr)	 IRISADDSSVNXW(nlen,nptr,elen,eptr)
#define	CACHEPUSHSSVNH(nlen,nptr)  IRISPUSHSSVNH(nlen,nptr)
#define	CACHEPUSHSSVNXH(nlen,nptr,elen,eptr)	 IRISPUSHSSVNXH(nlen,nptr,elen,eptr)
#define	CACHEADDSSVNH(nlen,nptr)  IRISADDSSVNH(nlen,nptr)
#define	CACHEADDSSVNXH(nlen,nptr,elen,eptr)	 IRISADDSSVNXH(nlen,nptr,elen,eptr)
#define	CACHEADDGLOBALH(nlen,nptr)  IRISADDGLOBALH(nlen,nptr)
#define	CACHEADDGLOBALXH(nlen,nptr,elen,eptr)	 IRISADDGLOBALXH(nlen,nptr,elen,eptr)
#define	CACHEADDGLOBALA(nlen,nptr)  IRISADDGLOBALA(nlen,nptr)
#define	CACHEADDGLOBALXA(nlen,nptr,elen,eptr)	 IRISADDGLOBALXA(nlen,nptr,elen,eptr)
#define	CACHEADDGLOBALW(nlen,nptr)  IRISADDGLOBALW(nlen,nptr)
#define	CACHEADDGLOBALXW(nlen,nptr,elen,eptr)	 IRISADDGLOBALXW(nlen,nptr,elen,eptr)
#define	CACHEADDSSVNDESCRIPTOR(num)	 IRISADDSSVNDESCRIPTOR(num)
#define	CACHEADDGLOBALDESCRIPTOR(num)	 IRISADDGLOBALDESCRIPTOR(num)
#define	CACHEADDLOCALDESCRIPTOR(num)	 IRISADDLOCALDESCRIPTOR(num)
#define	CACHESSVNGET(num)	 IRISSSVNGET(num)
#define	CACHESSVNKILL(num,val)	 IRISSSVNKILL(num,val)
#define	CACHESSVNORDER(num,val)	 IRISSSVNORDER(num,val)
#define	CACHESSVNQUERY(num,val)	 IRISSSVNQUERY(num,val)
#define	CACHESSVNDATA(num)	 IRISSSVNDATA(num)
#define	CACHEMERGE()	 IRISMERGE()
#define	CACHEENABLEMULTITHREAD() IRISENABLEMULTITHREAD()
#define	CACHEIKNOWFUNCS(funcsp)	 IRISIKNOWFUNCS(funcsp)
#endif	/* ZF_DLL */	/*==============================*/

#ifndef	ZF_DLL		/*==============================*/

/*	Default function definitions (ASCII/Unicode/wchar_t)
*/
#if	defined(CACHE_UNICODE)	/* Unicode character strings */
#define	CacheCvtIn		CacheCvtInW
#define	CacheCvtOut		CacheCvtOutW
#define	CacheCvtExStrIn		CacheCvtExStrInW
#define	CacheCvtExStrOut	CacheCvtExStrOutW
#define	CacheError		CacheErrorW
#define	CacheErrxlate		CacheErrxlateW
#define	CacheEval		CacheEvalW
#define	CacheExecute		CacheExecuteW
#define	CachePrompt		CachePromptW
#define	CacheStart		CacheStartW
#define	CacheSecureStart	CacheSecureStartW
#define	CacheChangePassword	CacheChangePasswordW
#define	CACHECVTIN		CACHECVTINW
#define	CACHECVTOUT		CACHECVTOUTW
#define	CACHECVTEXSTRIN		CACHECVTEXSTRINW
#define	CACHECVTEXSTROUT	CACHECVTEXSTROUTW
#define	CACHEERROR		CACHEERRORW
#define	CACHEERRXLATE		CACHEERRXLATEW
#define	CACHEEVAL		CACHEEVALW
#define	CACHEEXECUTE		CACHEEXECUTEW
#define	CACHEPROMPT		CACHEPROMPTW
#define	CACHESTART		CACHESTARTW
#define	CACHESECURESTART	CACHESECURESTARTW
#define	CACHEPOPCVT		CACHEPOPCVTW
#define	CACHEPUSHCVT		CACHEPUSHCVTW
#define	CACHEPOPSTR		CACHEPOPSTRW
#define	CACHEPOPEXSTR		CACHEPOPEXSTRW
#define	CACHEPUSHCLASSMETHOD	CACHEPUSHCLASSMETHODW
#define	CACHEPUSHFUNC		CACHEPUSHFUNCW
#define	CACHEPUSHFUNCX		CACHEPUSHFUNCXW
#define	CACHEPUSHGLOBAL		CACHEPUSHGLOBALW
#define	CACHEPUSHGLOBALX	CACHEPUSHGLOBALXW
#define	CACHEPUSHMETHOD		CACHEPUSHMETHODW
#define	CACHEPUSHPROPERTY	CACHEPUSHPROPERTYW
#define	CACHEPUSHRTN		CACHEPUSHRTNW
#define	CACHEPUSHRTNX		CACHEPUSHRTNXW
#define	CACHEPUSHSTR		CACHEPUSHSTRW
#define	CACHEPUSHEXSTR		CACHEPUSHEXSTRW
#define	CACHEPUSHLOCK		CACHEPUSHLOCKW
#define	CACHEPUSHLOCKX		CACHEPUSHLOCKXW
#define	CACHEEXSTRNEW		CACHEEXSTRNEWW
#define	CACHEPUSHSSVN		CACHEPUSHSSVNW
#define	CACHEPUSHSSVNX		CACHEPUSHSSVNXW
#define	CACHEADDGLOBAL		CACHEADDGLOBALW
#define	CACHEADDGLOBALX		CACHEADDGLOBALXW
#define	CACHEADDSSVN		CACHEADDSSVNW
#define	CACHEADDSSVNX		CACHEADDSSVNXW
#define	CACHEPUSHEXECUTEFUNC	CACHEPUSHEXECUTEFUNCW
#elif	defined(CACHE_WCHART)	/* wchar_t character strings */
#define	CacheCvtIn		CacheCvtInH
#define	CacheCvtOut		CacheCvtOutH
#define	CacheCvtExStrIn		CacheCvtExStrInW
#define	CacheCvtExStrOut		CacheCvtExStrOutW
#define	CacheError		CacheErrorH
#define	CacheErrxlate		CacheErrxlateH
#define	CacheEval		CacheEvalH
#define	CacheExecute		CacheExecuteH
#define	CachePrompt		CachePromptH
#define	CacheStart		CacheStartH
#define	CacheSecureStart	CacheSecureStartH
#define	CacheChangePassword	CacheChangePasswordH
#define	CACHECVTIN		CACHECVTINH
#define	CACHECVTOUT		CACHECVTOUTH
#define	CACHECVTEXSTRIN		CACHECVTEXSTRINH
#define	CACHECVTEXSTROUT		CACHECVTEXSTROUTH
#define	CACHEERROR		CACHEERRORH
#define	CACHEERRXLATE		CACHEERRXLATEH
#define	CACHEEVAL		CACHEEVALH
#define	CACHEEXECUTE		CACHEEXECUTEH
#define	CACHEPROMPT		CACHEPROMPTH
#define	CACHESTART		CACHESTARTH
#define	CACHESECURESTART	CACHESECURESTARTH
#define	CACHECHANGEPASSWORD	CACHECHANGEPASSWORDH
#define	CACHEPOPCVT		CACHEPOPCVTH
#define	CACHEPUSHCVT		CACHEPUSHCVTH
#define	CACHEPOPSTR		CACHEPOPSTRH
#define	CACHEPOPEXSTR		CACHEPOPEXSTRH
#define	CACHEPUSHCLASSMETHOD	CACHEPUSHCLASSMETHODH
#define	CACHEPUSHFUNC		CACHEPUSHFUNCH
#define	CACHEPUSHFUNCX		CACHEPUSHFUNCXH
#define	CACHEPUSHGLOBAL		CACHEPUSHGLOBALH
#define	CACHEPUSHGLOBALX	CACHEPUSHGLOBALXH
#define	CACHEPUSHMETHOD		CACHEPUSHMETHODH
#define	CACHEPUSHPROPERTY	CACHEPUSHPROPERTYH
#define	CACHEPUSHRTN		CACHEPUSHRTNH
#define	CACHEPUSHRTNX		CACHEPUSHRTNXH
#define	CACHEPUSHSTR		CACHEPUSHSTRH
#define	CACHEPUSHEXSTR		CACHEPUSHEXSTRH
#define	CACHEPUSHLOCK		CACHEPUSHLOCKH
#define	CACHEPUSHLOCKX		CACHEPUSHLOCKXH
#define	CACHEEXSTRNEW		CACHEEXSTRNEWH
#define	CACHEPUSHSSVN		CACHEPUSHSSVNH
#define	CACHEPUSHSSVNX		CACHEPUSHSSVNXH
#define	CACHEADDGLOBAL		CACHEADDGLOBALH
#define	CACHEADDGLOBALX		CACHEADDGLOBALXH
#define	CACHEADDSSVN		CACHEADDSSVNH
#define	CACHEADDSSVNX		CACHEADDSSVNXH
#define	CACHEPUSHEXECUTEFUNC	CACHEPUSHEXECUTEFUNCH
#else				/* ASCII character strings */
#define	CacheCvtIn		CacheCvtInA
#define	CacheCvtOut		CacheCvtOutA
#define	CacheCvtExStrIn		CacheCvtExStrInA
#define	CacheCvtExStrOut	CacheCvtExStrOutA
#define	CacheError		CacheErrorA
#define	CacheErrxlate		CacheErrxlateA
#define	CacheEval		CacheEvalA
#define	CacheExecute		CacheExecuteA
#define	CachePrompt		CachePromptA
#define	CacheStart		CacheStartA
#define	CacheSecureStart	CacheSecureStartA
#define	CacheChangePassword	CacheChangePasswordA
#define	CACHECVTIN		CACHECVTINA
#define	CACHECVTOUT		CACHECVTOUTA
#define	CACHECVTEXSTRIN		CACHECVTEXSTRINA
#define	CACHECVTEXSTROUT	CACHECVTEXSTROUTA
#define	CACHEERROR		CACHEERRORA
#define	CACHEERRXLATE		CACHEERRXLATEA
#define	CACHEEVAL		CACHEEVALA
#define	CACHEEXECUTE		CACHEEXECUTEA
#define	CACHEPROMPT		CACHEPROMPTA
#define	CACHESTART		CACHESTARTA
#define	CACHESECURESTART	CACHESECURESTARTA
#define	CACHECHANGEPASSWORD	CACHECHANGEPASSWORDA
#define	CACHEPOPSTR		CACHEPOPSTRA
#define	CACHEPOPEXSTR		CACHEPOPEXSTRA
#define	CACHEPUSHCLASSMETHOD	CACHEPUSHCLASSMETHODA
#define	CACHEPUSHFUNC		CACHEPUSHFUNCA
#define	CACHEPUSHFUNCX		CACHEPUSHFUNCXA
#define	CACHEPUSHGLOBAL		CACHEPUSHGLOBALA
#define	CACHEPUSHGLOBALX	CACHEPUSHGLOBALXA
#define	CACHEPUSHMETHOD		CACHEPUSHMETHODA
#define	CACHEPUSHPROPERTY	CACHEPUSHPROPERTYA
#define	CACHEPUSHRTN		CACHEPUSHRTNA
#define	CACHEPUSHRTNX		CACHEPUSHRTNXA
#define	CACHEPUSHSTR		CACHEPUSHSTRA
#define	CACHEPUSHEXSTR		CACHEPUSHEXSTRA
#define	CACHEPUSHLOCK		CACHEPUSHLOCKA
#define	CACHEPUSHLOCKX		CACHEPUSHLOCKXA
#define	CACHEEXSTRNEW		CACHEEXSTRNEWA
#define	CACHEPUSHSSVN		CACHEPUSHSSVNA
#define	CACHEPUSHSSVNX		CACHEPUSHSSVNXA
#define	CACHEADDGLOBAL		CACHEADDGLOBALA
#define	CACHEADDGLOBALX		CACHEADDGLOBALXA
#define	CACHEADDSSVN		CACHEADDSSVNA
#define	CACHEADDSSVNX		CACHEADDSSVNXA
#define	CACHEPUSHEXECUTEFUNC	CACHEPUSHEXECUTEFUNCA
#endif	/* CACHE_UNICODE */

#else	/* ZF_DLL */	/*==============================*/

#define	CacheCvtIn	IrisCvtIn
#define	CacheCvtOut	IrisCvtOut
#define	CacheCvtExStrIn	IrisCvtExStrIn
#define	CacheCvtExStrOut	IrisCvtExStrOut
#define	CacheError	IrisError
#define	CacheErrxlate	IrisErrxlate
#define	CacheEval	IrisEval
#define	CacheExecute	IrisExecute
#define	CachePrompt	IrisPrompt
#define	CacheStart	IrisStart
#define	CacheSecureStart	IrisSecureStart
#define	CacheChangePassword	IrisChangePassword
#define	CACHECVTIN	IRISCVTIN
#define	CACHECVTOUT	IRISCVTOUT
#define	CACHECVTEXSTRIN	IRISCVTEXSTRIN
#define	CACHECVTEXSTROUT	IRISCVTEXSTROUT
#define	CACHEERROR	IRISERROR
#define	CACHEERRXLATE	IRISERRXLATE
#define	CACHEEVAL	IRISEVAL
#define	CACHEEXECUTE	IRISEXECUTE
#define	CACHEPROMPT	IRISPROMPT
#define	CACHESTART	IRISSTART
#define	CACHESECURESTART	IRISSECURESTART
#define	CACHECHANGEPASSWORD	IRISCHANGEPASSWORD
#define	CACHEPOPCVT	IRISPOPCVT
#define	CACHEPUSHCVT	IRISPUSHCVT
#define	CACHEPOPSTR	IRISPOPSTR
#define	CACHEPOPEXSTR	IRISPOPEXSTR
#define	CACHEPUSHCLASSMETHOD	IRISPUSHCLASSMETHOD
#define	CACHEPUSHFUNC	IRISPUSHFUNC
#define	CACHEPUSHFUNCX	IRISPUSHFUNCX
#define	CACHEPUSHGLOBAL	IRISPUSHGLOBAL
#define	CACHEPUSHGLOBALX	IRISPUSHGLOBALX
#define	CACHEPUSHMETHOD	IRISPUSHMETHOD
#define	CACHEPUSHPROPERTY	IRISPUSHPROPERTY
#define	CACHEPUSHRTN	IRISPUSHRTN
#define	CACHEPUSHRTNX	IRISPUSHRTNX
#define	CACHEPUSHSTR	IRISPUSHSTR
#define	CACHEPUSHEXSTR	IRISPUSHEXSTR
#define	CACHEPUSHLOCK	IRISPUSHLOCK
#define	CACHEPUSHLOCKX	IRISPUSHLOCKX
#define	CACHEEXSTRNEW	IRISEXSTRNEW
#define	CACHEPUSHSSVN	IRISPUSHSSVN
#define	CACHEPUSHSSVNX	IRISPUSHSSVNX
#define	CACHEADDGLOBAL	IRISADDGLOBAL
#define	CACHEADDGLOBALX	IRISADDGLOBALX
#define	CACHEADDSSVN	IRISADDSSVN
#define	CACHEADDSSVNX	IRISADDSSVNX
#endif	/* ZF_DLL */	/*==============================*/

#endif	/* _callin_h_ */
