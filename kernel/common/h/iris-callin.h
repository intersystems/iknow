/*
	Definitions for the Call-In Interface to IRIS

	Copyright (c) 1997-2018 by InterSystems.
	Cambridge, Massachusetts, U.S.A.  All rights reserved.
	Confidential, unpublished property of InterSystems.

	This module contains the definitions necessary to build a 'C'
	language program that utilizes the InterSystems IRIS call-in
	interface.

	NOTE - because this file is included in C programs and the
	C compiler on some platforms may not support new style
	comments by default, the // style of comments should not
	be used here.

	Last Change: $Date$
	Changelist : $Change$
*/

#ifndef	_iriscallin_h_
#define	_iriscallin_h_

#include "iris-cdzf.h"

/*
	By default, 8-bit strings and arrays used in the Callin interface
	are declared as type "unsigned char". Many C and C++ compilers
	expect string literals and arguments to string handling libraries
	to be of the type "char". Some compilers have option switches to
	specify the default type for "char", for example -signed and
	-unsigned. If you want the function argument prototypes in this
	header file to declare 8-bit string quantities as "char" instead of
	"unsigned char", to avoid compiler warnings on your system, define
	macro USE_CALLIN_CHAR before including it in your program. Doing so
	will not affect the way character strings are handled inside IRIS.
*/

#ifdef	USE_CALLIN_CHAR
typedef	char		Callin_char_t;
#else
typedef	unsigned char	Callin_char_t;
#endif

/*		Data Structures
*/

/*	The following typedefs produce pointers to counted strings of the type
	the IRIS interface routines expect as input and return as output.
	The length element for a string being passed to an interface routine
	is set to the actual length of the string.  When a string is to be
	passed back to the caller, the caller provides a counted string buffer
	with the length element set to the length of the buffer (i.e. the
	maximum possible length of the returned string).  Failure to set the
	length element can result in either errors from the interface if the
	length field is out of range, or overwriting other memory structures
	if a string larger than the allocated space is returned.
*/

#ifdef	__cplusplus
extern "C" {
#endif

/*	Flag indicating 4-byte wchar_t support */
#if	defined(_WIN32)
#define	IRIS_HSTR	0
#else
#define	IRIS_HSTR	1
#endif

#define	IRIS_MAXSTRLEN	32767U
#define	IRIS_MAXLOSTSZ	3641144U

/*	Structure to pass ASCII (one byte) character strings */
typedef struct {
	unsigned short	len;
	Callin_char_t	str[IRIS_MAXSTRLEN];
} IRIS_ASTR, *IRIS_ASTRP;

/*	Structure to pass Unicode (two byte) character strings */
typedef struct {
	unsigned short	len;
	unsigned short	str[IRIS_MAXSTRLEN];
} IRISWSTR, *IRISWSTRP;

#ifndef IRIS_INT64
  /* long long is a 64-bit integer type on all supported platforms */
  /* long long conforms to the C99 standard and is mostly portable */
#define IRIS_INT64 long long
#endif

#if	IRIS_HSTR
#include <wchar.h>
/*	Structure to pass extended (four byte) character strings */
typedef struct {
	unsigned int	len;
	wchar_t		str[IRIS_MAXSTRLEN];
} IRISHSTR, *IRISHSTRP;
#else
typedef	IRISWSTR	IRISHSTR, *IRISHSTRP;
#endif	/* IRIS_HSTR */

/*	IRIS is able to manipulate "long strings", much longer than the
	former, traditional limit of 32,767 characters.  The IRIS_EXSTR
	descriptor is used when strings may exceed this old limit.

	Note that the str field is actually a pointer to the string.
	These strings are managed internally with a reference count. The
	structure must be allocated with IRISEXSTRNEW() and released with
	IRISEXSTRKILL(). After IRISEXSTRKILL() the memory pointed to by
	str should be considered no longer valid.
*/

typedef struct {
	unsigned int	len;		/* length of string */
	union {
	   Callin_char_t * ch;		/* text of the 8-bit string */
	   unsigned short *wch;		/* text of the 16-bit string */
#if	IRIS_ZHARR
	   wchar_t 	  *lch;		/* text of the wchar_t string */
#else
	   unsigned short *lch;
#endif
	}		str;
} IRIS_EXSTR, *IRIS_EXSTRP;

/* A block of data used by the iKnow engine to store Knowledgebases and
   Languagebases */
typedef struct {
	int		data_len;
	unsigned char*	data;
} IRIS_IKNOWDATA, *IRIS_IKNOWDATAP;

/* An iKnow string, which is always UTF-16 with unsigned shorts */
typedef struct {
	int			string_len;
	const unsigned short*	string;
} IRIS_IKNOWSTRING, *IRIS_IKNOWSTRINGP;

/* Callbacks for a callin client (currently only the iKnow engine) to allocate and free iKnow knowledge bases */
/* All return error codes */
typedef int (*IRIS_IKNOWLOADFUNC)(void* udata, IRIS_IKNOWDATAP loaded_data);
typedef int (*IRIS_IKNOWRELEASEFUNC)(IRIS_IKNOWDATAP data);
typedef struct {
	/* Loads data if needed by calling load_func with user data udata, populates loaded_data with data description,
	   gives client the opportunity to release resources from the load via release_func */
	int (*LoadData)(IRIS_IKNOWSTRINGP id, void* udata, IRIS_IKNOWLOADFUNC load_func, IRIS_IKNOWRELEASEFUNC release_func, IRIS_IKNOWDATAP loaded_data);
	/* Release data that is no longer needed, takes the data pointer from the loaded_data struct of the LoadData function */
	int (*UnloadData)(unsigned char* data);
	/* Point at null terminated strings of the error message ID
	   (from Errors.xml) and a single error parameter for the last
	   iKnow data failure for this process. */
	void (*GetError)(const char** message_id, const char** message_val);
	/* Set the current error ID and parameter value */
	void (*SetError)(const char* message_id, const char* message_val);
} IRIS_IKNOWFUNCS, *IRIS_IKNOWFUNCSP;


/* 			Character type interface definitions
*/
#if	defined(IRIS_UNICODE)	/* Unicode character strings */
#define	IRISSTR		IRISWSTR
#define	IRIS_STR	IRISWSTR
#define	IRISSTRP	IRISWSTRP
#define IRIS_STRP	IRISWSTRP
#define	IRIS_STRING	IRIS_WSTRING
#elif	defined(IRIS_WCHART)	/* wchar_t character strings */
#define	IRISSTR		IRISHSTR
#define	IRIS_STR	IRISHSTR
#define	IRISSTRP	IRISHSTRP
#define IRIS_STRP	IRISHSTRP
#define	IRIS_STRING	IRIS_HSTRING
#else			/* ASCII character strings */
#define	IRISSTR		IRIS_ASTR
#define	IRIS_STR	IRIS_ASTR
#define	IRISSTRP	IRIS_ASTRP
#define IRIS_STRP	IRIS_ASTRP
#define	IRIS_STRING	IRIS_ASTRING
#endif

/* 			Constants
*/

/*	The following are the definitions that are used to form the flags
	argument to IrisStart() to control how IRIS treats, or doesn't
	treat, the principal input device.

	Setting more than one of these flags (with the exception of the
	modifiers) will cause unpredictable results
*/
#define	IRIS_TTALL	1	/* Indicates that IRIS should initialize the
				   terminal's settings and restore them across
				   each call into, and return from, the
				   interface.  */
#define IRIS_TTCALLIN	2	/* Indicates that IRIS should initialize the
				   terminal each time it is called but should
				   restore it only when the connection is
				   closed (when IrisEnd() is called or the
				   connection is broken). */
#define IRIS_TTSTART	4	/* Indicates that IRIS should initialize the
				   terminal when the connection is formed and
				   reset it when the connection is terminated. */
#define	IRIS_TTNEVER	8	/* Indicates that IRIS should not alter the
				   terminal's settings. */
#define IRIS_TTNONE	16	/* Indicates that IRIS should not do any
				   output or input from the principle input
				   device. This is equivilent to specifying
				   the null device for princin and princout.
				   Output will be discarded and reads will
				   complete with end-of-file. */
#define IRIS_TTMASK	(IRIS_TTALL|IRIS_TTCALLIN|IRIS_TTSTART| \
			 IRIS_TTNEVER|IRIS_TTNONE)
#define IRIS_PROGMODE	32	/* IRIS errors do not cause halt in the
				   absence of an error handler */
/*	The following modifier is allowed in conjunction with
	IRIS_TTALL, IRIS_TTCALLIN and IRIS_TTSTART.
*/
#define IRIS_TTNOUSE	0x10000 /* If set this indicates that IRIS OPEN and
				   USE commands are not allowed to alter the
				   terminal. This is implicitly set by
				   IRIS_TTNEVER and IRIS_TTNONE. The default
				   is to allow IRIS OPEN and USE commands to
				   alter the terminal in the other modes. */

/*	The following are the definitions that can be used to form the
	arguments to IrisCtrl().  These flags are in pairs and if both
	members of a set are specified the results will be unpredictable.
*/
#define IRIS_DISACTRLC	 1	/* CTRL/C is ignored by IRIS */
#define IRIS_ENABCTRLC	 2	/* CTRL/C in IRIS generates an <INTERRUPT> */

/*	The following are the arguments to IrisAbort() that control the
	mechanism used to terminate IRIS processing.

	IRIS_CTRLC simulates a CTRL/C at the keyboard and takes effect even
	if IrisCtrl() specified that CTRL/C is to be ignored. The IRIS
	call that is interrupted returns IRIS_INTERRUPT.

	IRIS_RESJOB simulates a resjob. This results in the IRIS connection
	being broken and the IRIS call that is interrupted completes with
	a status of IRIS_CONBROKEN.
*/
#define	IRIS_CTRLC	 1
#define IRIS_RESJOB	 2

/*	The following are the flag definitions for the flag argument
 *	to the IrisAcquireLock and/or IrisReleaseLock functions.
 *	These may be OR'd together to specify multiple flags for a
 *	single request.
 *
 *	IRIS_INCREMENTAL_LOCK is passed to IrisAcquireLock and indicates
 *	that this is a L + style request. Any locks currently held remain
 *	held. If this is omitted then all existing locks for the process
 *	are released before attempting to acquire the newly requested lock.
 *
 *	IRIS_SHARED_LOCK is passed to IrisAcquireLock and indicates that
 *	the requested lock should be acquired in SHARED access mode. If this
 *	is omitted the lock is acquired in EXCLUSIVE mode. This is equivilent
 *	to the #"S" argument to the lock command.
 *
 *	IRIS_IMMEDIATE_RELEASE is passed to IrisReleaseLock and indicates
 *	that the lock being released should be unlocked immediatly even if
 *	there is an open transaction. Normally within a transaction the
 *	actual releasing of a lock is delayed until the transaction ends.
 *	This is equivilent to the #"I" flag on an unlock command.
*/
#define	IRIS_INCREMENTAL_LOCK	1
#define	IRIS_SHARED_LOCK	2
#define IRIS_IMMEDIATE_RELEASE	4

/* 	The following are the return values from IrisType() that describe
	the internal datatype of the result from the call to IrisEval()
*/
#define	IRIS_INT	 1	/* native integer */
#define IRIS_DOUBLE	 2	/* IRIS 64-bit floating point */
#define IRIS_ASTRING	 3	/* ASCII character string */

/*	In addition to the above the following can be used as the type
	argument to the IrisConvert() function

	Note that floating point types (IRIS_FLOAT, IRIS_HFLOAT and
	IRIS_DOUBLE) that have decimal parts (including negative exponents),
	as well as the 64 bit integer types IRIS_INT8 and IRIS_UINT8,
	involve division when calculating the return value. Therefore the
	results may not be 100% accurate.  IRIS_STRINT, IRIS_INT8 and
	IRIS_UINT8 can return the status IRIS_RETTRUNC if the result of the
	last IrisEval() call will not fit in the space allocated in
	IrisConvert(). In the case of IRIS_INT8 and IRIS_UINT8 this means
	that the expression resulted in a floating point value that could not
	be normalized to fit within 64 bits.
*/
#define	IRIS_CHAR	 4	/*   8 bit signed integer */
#define	IRIS_INT2	 5	/*  16 bit signed integer */
#define	IRIS_INT4	 6	/*  32 bit signed integer */
#define	IRIS_INT8	 7	/*  64 bit signed integer */
#define	IRIS_UCHAR	 8	/*   8 bit unsigned integer */
#define	IRIS_UINT2	 9	/*  16 bit unsigned integer */
#define	IRIS_UINT4	10	/*  32 bit unsigned integer */
#define	IRIS_UINT8	11	/*  64 bit unsigned integer */
#define IRIS_FLOAT	12	/*  32 bit floating point */
#define IRIS_HFLOAT	13	/* 128 bit floating point */
#define	IRIS_UINT	14	/* native size unsigned integer */
#define IRIS_WSTRING	15	/* Unicode character string */
#define	IRIS_OREF	16	/* object reference */
#define	IRIS_LASTRING	17	/* ASCII long string */
#define	IRIS_LWSTRING	18	/* Unicode long string */
#define	IRIS_IEEE_DBL	19	/* IEEE 64-bit floating point */
#define IRIS_HSTRING	20	/* wchar_t character string */
#define	IRIS_UNDEF	21	/* an undefined argument */


/*	The following are error codes that are generated by the callin
	interface as opposed to error codes that are generated by
	IRIS and returned by the callin interface. */

#define IRIS_INVALID_INSTANCE  -17 /* invalid shdir */
#define	IRIS_CHANGEPASSWORD -16 /* Password change required */
#define	IRIS_ACCESSDENIED -15	/* Process not authenticated */
#define	IRIS_EXSTR_INUSE  -14	/* an output EXSTR must be null */
#define IRIS_NORES	  -13	/* IrisConvert() was called to get a result
				   but there is no result to return */
#define IRIS_BADARG	  -12	/* A function was called with an argument
				   that was out of range or otherwise
				   invalid */
#define IRIS_NOTINIRIS	  -11	/* Returned from IrisAbort() if the process
			  	   was not executing on the IRIS side */
#define IRIS_RETTRUNC 	  -10	/* The return buffer was too small so only
				   part of the requested item was returned */
#define IRIS_ERUNKNOWN	   -9	/* Unknown error code */
#define IRIS_RETTOOSMALL   -8	/* return buffer too small, nothing returned */
#define IRIS_NOCON 	   -7	/* not connected to IRIS */
#define IRIS_INTERRUPT	   -6	/* Failed due to CTRL/C (special case of
				   IrisError() */
#define IRIS_CONBROKEN	   -4	/* The connection is broken */
#define IRIS_STRTOOLONG	   -3	/* A string argument is too long */
#define IRIS_ALREADYCON	   -2	/* Already connected to IRIS */
#define IRIS_FAILURE	   -1
#define IRIS_SUCCESS 	    0

/*	The following are possible errors that IRIS could return. To get
	the text associated with a particular error pass one of the following
	codes to IrisErrxlate().  After a call to one of the IRIS interface
	routines returns one of these errors, calling IrisError() will return
	the contents of $ZE.

	Calling IrisErrxlate() with an error code out of range results in an
	IRIS_ERUNKNOWN return status.

	Some IRIS errors (IRIS_ERINTERRUPT and IRIS_ERCONBROKEN) are of
	special interest and are translated to one of the negative codes
	above.
*/

#define	IRIS_ERMAXNUM		  1	/* max number */
#define	IRIS_ERSELECT		  2	/* select */
#define	IRIS_ERSYNTAX		  3	/* syntax */
#define	IRIS_ERLABELREDEF	  4	/* labelredef */
#define IRIS_ERMXSTR		  5	/* max string */
#define	IRIS_ERFUNCTION		  6	/* function */
#define	IRIS_ERINTERRUPT	  7	/* interrupt */
#define	IRIS_ERNOLINE		  8	/* noline */
#define	IRIS_ERUNDEF		  9	/* undefined */
#define	IRIS_ERSYSTEM		 10	/* system */
#define	IRIS_EREDITED		 11	/* edited */
#define	IRIS_ERCOMMAND  	 12	/* command */
#define	IRIS_ERMAXARRAY		 13	/* maxarray */
#define	IRIS_ERNAME		 14	/* name */
#define	IRIS_ERSTORE		 15	/* store */
#define	IRIS_ERSUBSCR		 16	/* subscript */
#define	IRIS_ERNOROUTINE	 17	/* noroutine */
#define	IRIS_ERDIVIDE		 18	/* divide */
#define	IRIS_ERSWIZZLEFAIL	 19	/* <SWIZZLE FAIL> */
#define	IRIS_ERSTRINGSTACK	 20	/* string stack */
#define	IRIS_ERFRAMESTACK	 21	/* framestack */
#define	IRIS_ERUNIMPLEMENTED	 22	/* unimplemented */
#define	IRIS_EREXQUOTA		 23	/* exceeded quota (wsextent) */
#define	IRIS_ERNOTOPEN		 24	/* not open */
#define	IRIS_ERARGSTACK		 25	/* argstack */
#define	IRIS_ERM11TAPE		 26	/* m11 tape error */
#define	IRIS_ERPROTECT		 27	/* protect */
#define	IRIS_ERDATABASE		 28	/* database */
#define	IRIS_ERFILEFULL		 29	/* file full */
#define	IRIS_ERNAKED		 30	/* naked */
#define	IRIS_ERROUTINELOAD	 31 	/* routineload */
#define	IRIS_ERTERMINATOR	 32	/* terminator */
#define	IRIS_ERDISKHARD		 33	/* disk hard */
#define	IRIS_ERBLOCKNUM		 34	/* block # */
#define	IRIS_ERDEVNAME		 35	/* device name */
#define	IRIS_ERNOJOB		 36	/* nojob */
#define	IRIS_ERREAD		 37	/* read */
#define	IRIS_ERWRITE		 38	/* write */
#define	IRIS_EREOF		 39	/* end of file */
#define	IRIS_ERPARAMETER	 40	/* parameter */
#define	IRIS_ERNETWORK		 41	/* network */
#define	IRIS_ERRECOMPILE	 42	/* recompile */
#define	IRIS_ERDIR		 43	/* directory */
#define	IRIS_ERMAXSCOPE		 44	/* max scope */
#define	IRIS_ERCLUSTERFAIL	 45	/* cluster failed */
#define	IRIS_ERMAXERROR		 46	/* MAXERROR (internal problem) */
#define	IRIS_ERCONBROKEN	 47	/* lost an external connection */
#define	IRIS_ERNOMBX		 48	/* no mailbox */
#define	IRIS_ERNOCP		 49	/* unused */
#define	IRIS_ERNOWD		 50	/* unused */
#define	IRIS_ERNOGC		 51	/* unused */
#define	IRIS_ERMTERR		 52	/* <MAGTAPE> */
#define	IRIS_ERDOWN		 54	/* unused */
#define	IRIS_ERCLNUP		 55	/* unused */
#define	IRIS_ERMAXPID		 56	/* unused */
#define	IRIS_ERNOSIGNON		 57	/* <SIGNONS INHIBITED> */
#define	IRIS_ERHALT		 58	/* <HALTED> */
#define	IRIS_ERWAITINIT		 59	/* <WAIT FOR M/xx INIT.. TO COMPLT> */
#define	IRIS_ERPRIMARY		 60	/* <NOT PRIMARY VOLUME>	  */
#define	IRIS_ERCORRUPT		 61	/* <CORRUPT VOLUME SET>	  */
#define	IRIS_ERMAPS		 62	/* <INVALID MAP NUMBER>	  */
#define	IRIS_ERVOLSET1		 63	/* <VOLUME SET ALREADY CREATED> */
#define	IRIS_ERFORMAT		 64	/* <VOLUME IS NOT FORMATTED>	   */
#define	IRIS_ERMAPRSV		 65	/* <CONFLICTING BLOCK NUMBERS>  */
#define	IRIS_ERCOLLATE		 66	/* <COLLATION> */
#define	IRIS_ERZTRP		 67	/* <ZTRAP> */
#define	IRIS_ERNODEV		 68	/* <NODEV> */
#define	IRIS_ERLANG		 69	/* <LANGUAGE MISMATCH> */
#define	IRIS_ERNOSYS		 70	/* <NOSYS> */
#define	IRIS_ERGIMPL		 71	/* <BADIMPLICIT> */
#define	IRIS_ERNGLOREF		 72	/* <NETGLOREF> */
#define	IRIS_ERNFORMAT		 73	/* <NETFORMAT> */
#define	IRIS_ERDSCON		 74	/* <DSCON> */
#define	IRIS_ERNVERSION		 75	/* <NETVERSION> */
#define IRIS_ERNJOBMAX		 76	/* <NETJOBMAX> */
#define IRIS_ERNRETRY		 77	/* <NETRETRY> */
#define	IRIS_ERNAMADD		 78	/* <NAMEADD> */
#define	IRIS_ERNETLOCK		 79	/* <NETLOCK> */
#define	IRIS_ERVALUE		 80	/* <ILLEGAL VALUE> */
#define	IRIS_ERNUMFIL		 81	/* <TOOMANYFILES> */
#define	IRIS_ERLIST		 82	/* <LIST> */
#define	IRIS_ERNAMSP		 83	/* <NAMESPACE> */
#define	IRIS_ERCMTFAIL		 84	/* <COMMITFAIL> */
#define	IRIS_ERROLLFAIL		 85	/* <ROLLFAIL> */
#define	IRIS_ERNSRVFAIL		 86	/* <NETSRVFAIL> */
#define	IRIS_ERCOLMISMAT	 87	/* <COLLATEMISMATCH> */
#define	IRIS_ERSLMSPAN		 88	/* <SLMSPAN> */
#define	IRIS_ERWIDECHAR		 89	/* <WIDE CHAR> */
#define	IRIS_ERLINELEV		 90	/* <LINELEVEL> */
#define	IRIS_ERARGDUP		 91	/* <DUPLICATEARG> */
#define	IRIS_ERGLODEF		 92	/* <GLODEFCHANGE> */
#define	IRIS_ERCOLCHG		 93	/* <COLLATECHANGE> */
#define	IRIS_ERRESTART		 94	/* <NORESTART> */
#define	IRIS_ERROUDEF		 95	/* <ROUDEFCHANGE> */
#define	IRIS_ERMNESPACE		 96	/* <MNEMONICSPACE> */
#define	IRIS_ERTRANSLOCK	 97	/* <INVTRANSLOCK> */
#define	IRIS_ERTRANSQUIT	 98	/* <INVTRANSQUIT> */
#define	IRIS_ERECODETRAP	 99	/* <ECODETRAP> */
#define	IRIS_LOCKLOST		100	/* <LOCKLOST> */
#define	IRIS_DATALOST		101	/* <DATALOST> */
#define	IRIS_TRANSLOST		102	/* <TRANSLOST> */
#define	IRIS_ERZOLE		103	/* <ZOLE> */
#define	IRIS_ERLICENSE		104	/* <LICENSE LIMIT EXCEEDED> */
#define	IRIS_ERNOLICSERV	105 	/* <LICENSE SERVER UNAVAILABLE> */
#define	IRIS_ERCOLNSUP		106	/* <COLLATION NOT SUPPORTED> */
#define	IRIS_ERNLSTAB		107	/* <NLS TABLE> */
#define	IRIS_ERXLATE		108	/* <TRANSLATE> */
#define	IRIS_ERNULLVAL		109	/* <NULL VALUE> */
#define	IRIS_ERNOVAL		110	/* <MISSING VALUE> */
#define	IRIS_ERUNKTYPE		111	/* <UNKNOWN TYPE> */
#define	IRIS_ERVALRANGE		112	/* <VALUE OUT OF RANGE> */
#define	IRIS_ERRANGE		113	/* <RANGE> */
#define	IRIS_ERDOMAINSPACERETRY 114	/* <DOMAINSPACERETRY> */
#define IRIS_ERCSMISMATCH	115	/* <CLIENT-SERVER MISMATCH> */
#define IRIS_ERSTRMISMATCH	116	/* <STRMISMATCH> */
#define IRIS_ERBADCLASS		117	/* <INVALID CLASS> */
#define IRIS_ERIOBJECT		118	/* <INTERNAL OBJECT ERROR> */
#define IRIS_ERBADOREF		119	/* <INVALID OREF> */
#define IRIS_ERNOMETHOD		120	/* <METHOD DOES NOT EXIST> */
#define IRIS_ERNOPROPERTY	121	/* <PROPERTY DOES NOT EXIST> */
#define IRIS_ERNOCLASS		122	/* <CLASS DOES NOT EXIST> */
#define IRIS_ERCLASSDESC	123	/* <CLASS DESCRIPTOR> */
#define IRIS_ERCLASSCOMP	124	/* <CLASS COMPILING> */
#define IRIS_ERCLASSRECOMP	125	/* <CLASS RECOMPILED> */
#define	IRIS_ERCLASSMEMORY	126	/* <INSUFFICIENT CLASS MEMORY> */
#define	IRIS_ERNUMCLASS		127	/* <TOO MANY USERS OF CLASS> */
#define	IRIS_ERNUMOREF		128	/* <TOO MANY OREFS> */
#define	IRIS_ERGETINVALID	129	/* <CANNOT GET THIS PROPERTY> */
#define	IRIS_ERSETINVALID	130	/* <CANNOT SET THIS PROPERTY> */
#define IRIS_ERREMCLASSRECOMP	131	/* <REMOTE CLASS RECOMPILED> */
#define IRIS_ERREMCLASSEDITED	132	/* <REMOTE CLASS EDITED> */
#define IRIS_ERCLASSEDITED	133	/* <CLASS EDITED> */
#define IRIS_ERCLASSLOAD	134	/* <CLASS TOO BIG TO LOAD> */
#define IRIS_ERCLASSSIZE	135	/* <CLASS TOO BIG TO SAVE> */
#define IRIS_ERCORRUPTOBJECT	136	/* <CORRUPT OBJECT> */
#define IRIS_ERDISCONNECT	137	/* <DISCONNECT> */
#define IRIS_ERNOCURRENTOBJECT	138	/* <NO CURRENT OBJECT> */
#define IRIS_ERZFHEAPSPACE	139	/* <OUT OF $ZF HEAP SPACE> */
#define IRIS_ERTOOMANYCLASSES	140	/* <TOO MANY CLASSES> */
#define IRIS_ERNETSTRMISMATCH	141 /* <NETWORK DATA UPDATE FAILED - STRMISMATCH> */
#define IRIS_ERNETSSTACK	142 /* <NETWORK DATA UPDATE FAILED - STRINGSTACK> */
#define IRIS_ERNETDIR		143 /* <NETWORK DATA UPDATE FAILED - DIRECTORY> */
#define	IRIS_ERNETDKHER		144 /* <NETWORK DATA UPDATE FAILED - DISKHARD> */
#define IRIS_ERNETDBDGD		145 /* <NETWORK DATA UPDATE FAILED - DATABASE> */
#define	IRIS_ERNETCLUSTER	146 /* <NETWORK DATA UPDATE FAILED - CLUSTERFAILED> */
#define	IRIS_ERNETCSMISMATCH	147 /* <NETWORK DATA .... - CLIENT-SERVER MISMATCH> */
#define IRIS_ERNETPROT		148 /* <NETWORK DATA UPDATE FAILED - PROTECT> */
#define	IRIS_ERNETDKSER		149 /* <NETWORK DATA UPDATE FAILED - BLOCKNUMBER> */
#define	IRIS_ERNETDKFUL		150 /* <NETWORK DATA UPDATE FAILED - DISKFULL> */
#define	IRIS_ERNETMXSTR		151 /* <NETWORK DATA UPDATE FAILED - MAXSTRING> */
#define	IRIS_ERNETSBSCR		152 /* <NETWORK DATA UPDATE FAILED - SUBSCRIPT> */
#define	IRIS_ERNETSYSTM		153 /* <NETWORK DATA UPDATE FAILED - SYSTEM> */
#define	IRIS_ERNETWIDECHAR	154 /* <NETWORK DATA UPDATE FAILED - WIDECHAR> */
#define IRIS_ERNETNVERSION	155 /* <NETWORK DATA UPDATE FAILED - NETVERSION> */
#define IRIS_ERNETNFORMAT	156 /* <NETWORK DATA UPDATE FAILED - NETFORMAT> */
#define	IRIS_ERNETNGLOREF	157 /* <NETWORK DATA UPDATE FAILED - NETGLOREF>  */
#define IRIS_ERBADBITSTR	158	/* <INVALID BIT STRING> */
#define IRIS_ERROUTINESIZE	159	/* <ROUTINE TOO BIG TO SAVE> */
#define IRIS_ERNOSOURCE		160	/* <NO SOURCE> */
#define IRIS_ERMALLOC		161	/* <STORE> - malloc failed */
#define IRIS_EROREFNO		162	/* <INVALID TYPE> */
#define IRIS_ERNOOREF		163	/* <INVALID OREF> */
#define	IRIS_ERINVARG		164	/* <INVALID ARGUMENT> */
#define	IRIS_ERDYNLOAD		165	/* <DYNAMIC LIBRARY LOAD> */
#define IRIS_NOJVM		166	/* <Java VM not loaded> */
#define IRIS_ERPRIVPROP		167	/* <PRIVATE PROPERTY> */
#define IRIS_ERPRIVMETH		168	/* <PRIVATE METHOD> */
#define IRIS_ERJAVAEXCEPTION	169	/* <Java Exception> */
#define	IRIS_ERNETLICENSE	170	/* <NETWORK UNLICENSED> */
#define IRIS_ERDDPJOBOVFL	171	/* <DDP JOB OVERFLOW> */
#define	IRIS_ERLICENSEALLOC	172	/* <LICENSE ALLOCATION EXCEEDED> */
#define	IRIS_ERTRANLEVEL	173	/* <TRANSACTION LEVEL> */
#define IRIS_ERRESJOB		174	/* <RESJOB> */
#define IRIS_ERALARM		175	/* <ALARM> */
#define	IRIS_ERLOSTTABOVER	176	/* <TOO MANY LONG STRINGS> */
#define	IRIS_EREXTINT		177	/* <EXTERNAL INTERRUPT> */
#define	IRIS_ERNOFLOAT		178	/* <UNIMPLEMENTED FLOAT> */
#define	IRIS_ERMVWRAPUP		179	/* <MV WRAPUP> */
#define IRIS_ERUNLICENSED	180	/* <UNLICENSED> */
#define	IRIS_ERPROTDB		181	/* <PROTECT> */
#define	IRIS_ERARRDIM		182	/* <ARRAY DIMENSION> */
#define	IRIS_ERTHROW		183	/* <THROW> */
#define	IRIS_ERNOTFILVAR	184	/* <INVALID FILE VARIABLE> */
#define	IRIS_ERNOTSELLST	185	/* <INVALID SELECT LIST> */
#define	IRIS_ERCMPLXPAT		186	/* <COMPLEX PATTERN> */
#define	IRIS_ERCLASSPROP	187	/* <CLASS PROPERTY> */
#define	IRIS_ERMAXLOCKS		188	/* <MAX LOCKS> */
#define	IRIS_ERLOCKFULL		189	/* <LOCK TABLE FULL> */
#define	IRIS_ERSVRLOCKFULL	190	/* <SERVER LOCK TABLE FULL> */
#define IRIS_ERMAXROUTINES	191	/* <MAX ROUTINES> */
#define IRIS_EROUTOFHEAP	192	/* <SHARED MEM HEAP> */
#define	IRIS_ERMAXINCR		193	/* <MAXINCREMENT> */
#define IRIS_ERWRONGNS		194	/* <WRONG NAMESPACE> */
#define	IRIS_ERCALLBKSYN	195	/* <_CALLBACK SYNTAX> */
#define IRIS_EROBJDISP		196	/* <OBJECT DISPATCH> */
#define	IRIS_ERCOMMATCH		197	/* <COMMON MISMATCH> */
#define	IRIS_ERRTNLIMIT		198	/* <ROUTINE LIMIT> */
#define	IRIS_ERZFMAXSTR		199	/* <MAX $ZF STRING> */
#define IRIS_ERTOOCOMPLEX	200	/* <ROUTINE TOO COMPLEX> */
#define IRIS_ERINVALGLOBALREF	201	/* <INVALID GLOBAL REFERENCE> */
#define IRIS_ERTCPWRITE		202	/* <TCPWRITE> SML1761 */
#define IRIS_ERUNSUPPMETHOD	203	/* <METHOD NOT SUPPORTED> */
#define IRIS_ERMNSSUPPORT	204	/* <MULTI NS NOT SUPPORTED> */
#define	IRIS_ERTIMEOUT		205	/* <TIMEOUT> */
#define	IRIS_ERREXREAD		206	/* <REMOTE EXECUTE INVALID READ>  */
#define	IRIS_ERREXWRITE		207	/* <REMOTE EXECUTE INVALID WRITE> */

#define IRIS_ERMAX		207	/* Maximum error number */
/* Note: do not use // style comments in this file! */


/*		Name mappings to internal entrypoints
*/

/* Popping a double works the same way regardless of the format */
#define	IrisPopIEEEDbl	IrisPopDbl
#define	IRISPOPIEEEDBL	IRISPOPDBL

#if	defined(_WIN32)
#define	CFCALL			__cdecl
#else
#define	CFCALL
#endif

#if	defined(__STDC__) || defined(__cplusplus)
#define	CFPROTOD(func,args)	CFCALL func args
#define	CFPROTOI(func,args)	(CFCALL *func) args
#else
#define	CFPROTOD(func,args)	CFCALL func ()
#define	CFPROTOI(func,args)	(CFCALL *func) ()
#endif

typedef const struct  IrisExportFuncTab {
        int  CFPROTOI(IrisExecuteA,(IRIS_ASTRP volatile));
	int  CFPROTOI(IrisExecuteW,(IRISWSTRP volatile));
	int  CFPROTOI(IrisEvalA,(IRIS_ASTRP volatile));
	int  CFPROTOI(IrisEvalW,(IRISWSTRP volatile));
	int  CFPROTOI(IrisPromptA,(IRIS_ASTRP));
	int  CFPROTOI(IrisPromptW,(IRISWSTRP));
	int  CFPROTOI(IrisErrxlateA,(int,IRIS_ASTRP));
	int  CFPROTOI(IrisErrxlateW,(int,IRISWSTRP));
	int  CFPROTOI(IrisErrorA,(IRIS_ASTRP,IRIS_ASTRP,int *));
	int  CFPROTOI(IrisErrorW,(IRISWSTRP,IRISWSTRP,int *));
	int  CFPROTOI(IrisConvert,(unsigned long,void *));
	int  CFPROTOI(IrisType,(void));
	int  CFPROTOI(IrisAbort,(unsigned long));
	int  CFPROTOI(IrisSignal,(int));
	int  CFPROTOI(IrisPushInt,(int));
	int  CFPROTOI(IrisPushDbl,(double));
	int  CFPROTOI(IrisPushStr,(int,const Callin_char_t *));
	int  CFPROTOI(IrisPushStrW,(int,const unsigned short *));
	int  CFPROTOI(IrisPushRtn,(unsigned int *,int,const Callin_char_t *,int,const Callin_char_t *));
	int  CFPROTOI(IrisPushRtnX,(unsigned int *,int,const Callin_char_t *,int,int,const Callin_char_t*,int,const Callin_char_t *));
	int  CFPROTOI(IrisPushRtnW,(unsigned int *,int,const unsigned short *,int,const unsigned short *));
	int  CFPROTOI(IrisPushRtnXW,(unsigned int *,int,const unsigned short *,int,int,const unsigned short *,int,const unsigned short *));
	int  CFPROTOI(IrisPushFunc,(unsigned int *,int,const Callin_char_t *,int,const Callin_char_t *));
	int  CFPROTOI(IrisPushFuncX,(unsigned int *,int,const Callin_char_t *,int,int,const Callin_char_t*,int,const Callin_char_t *));
	int  CFPROTOI(IrisPushFuncW,(unsigned int *,int,const unsigned short *,int,const unsigned short *));
	int  CFPROTOI(IrisPushFuncXW,(unsigned int *,int,const unsigned short *,int,int,const unsigned short*,int,const unsigned short *));
	int  CFPROTOI(IrisPushGlobal,(int,const Callin_char_t *));
	int  CFPROTOI(IrisPushGlobalX,(int,const Callin_char_t *,int,const Callin_char_t *));
	int  CFPROTOI(IrisPushGlobalW,(int,const unsigned short *));
	int  CFPROTOI(IrisPushGlobalXW,(int,const unsigned short *,int,const unsigned short *));
	int  CFPROTOI(IrisUnPop,(void));
	int  CFPROTOI(IrisPop,(void **));
	int  CFPROTOI(IrisPopInt,(int *));
	int  CFPROTOI(IrisPopDbl,(double *));
	int  CFPROTOI(IrisPopStr,(int *,Callin_char_t **));
	int  CFPROTOI(IrisPopStrW,(int *,unsigned short **));
  	int  CFPROTOI(IrisSetVar,(int,const Callin_char_t *));
 	int  CFPROTOI(IrisGetVar,(int,const Callin_char_t *));
 	int  CFPROTOI(IrisDoRtn,(unsigned int,int));
	int  CFPROTOI(IrisExtFun,(unsigned int,int));
	int  CFPROTOI(IrisGlobalGet,(int,int));
	int  CFPROTOI(IrisGlobalSet,(int));
	int  CFPROTOI(IrisDoFun,(unsigned int,int));
	int  CFPROTOI(IrisPushClassMethod,(int,const Callin_char_t *,int,const Callin_char_t *,int));
	int  CFPROTOI(IrisPushClassMethodW,(int,const unsigned short *,int,const unsigned short *,int));
	int  CFPROTOI(IrisPushMethod,(unsigned int,int,const Callin_char_t *,int));
	int  CFPROTOI(IrisPushMethodW,(unsigned int,int,const unsigned short *,int));
	int  CFPROTOI(IrisInvokeClassMethod,(int));
	int  CFPROTOI(IrisInvokeMethod,(int));
	int  CFPROTOI(IrisPushProperty,(unsigned int,int,const Callin_char_t *));
	int  CFPROTOI(IrisPushPropertyW,(unsigned int,int,const unsigned short *));
	int  CFPROTOI(IrisGetProperty,(void));
	int  CFPROTOI(IrisSetProperty,(void));
	int  CFPROTOI(IrisPushOref,(unsigned int));
	int  CFPROTOI(IrisPopOref,(unsigned int *));
	int  CFPROTOI(IrisIncrementCountOref,(unsigned int));
	int  CFPROTOI(IrisCloseOref,(unsigned int));
	int  CFPROTOI(IrisPushCvtW,(int,const unsigned short *));
	int  CFPROTOI(IrisPopCvtW,(int *,unsigned short **));
	int  CFPROTOI(IrisPushList,(int,const Callin_char_t *));
	int  CFPROTOI(IrisPopList,(int *,Callin_char_t **));
	int  CFPROTOI(IrisPushPtr,(void *));
	int  CFPROTOI(IrisPopPtr,(void **));
	int  CFPROTOI(IrisGlobalIncrement,(int));
	int  CFPROTOI(IrisContext,(void));
	int  CFPROTOI(IrisCtrl,(unsigned long));
	int  CFPROTOI(IrisCvtInA,(IRIS_ASTRP,IRIS_ASTRP,IRIS_ASTRP));
	int  CFPROTOI(IrisCvtInW,(IRIS_ASTRP,IRISWSTRP,IRISWSTRP));
	int  CFPROTOI(IrisCvtOutA,(IRIS_ASTRP,IRIS_ASTRP,IRIS_ASTRP));
	int  CFPROTOI(IrisCvtOutW,(IRISWSTRP,IRISWSTRP,IRIS_ASTRP));
	int  CFPROTOI(IrisGlobalQuery,(int,int,int));
	int  CFPROTOI(IrisGlobalOrder,(int,int,int));
	int  CFPROTOI(IrisGlobalData,(int,int));
	int  CFPROTOI(IrisGlobalKill,(int,int));
	int  CFPROTOI(IrisGlobalRelease,(void));
	int  CFPROTOI(IrisPushLock,(int,const Callin_char_t *));
	int  CFPROTOI(IrisPushLockX,(int,const Callin_char_t *,int,const Callin_char_t *));
	int  CFPROTOI(IrisPushLockW,(int,const unsigned short *));
	int  CFPROTOI(IrisPushLockXW,(int,const unsigned short *,int,const unsigned short *));
	int  CFPROTOI(IrisAcquireLock,(int,int,int,int *));
	int  CFPROTOI(IrisReleaseLock,(int,int));
	int  CFPROTOI(IrisReleaseAllLocks,(void));
	int  CFPROTOI(IrisTStart,(void));
	int  CFPROTOI(IrisTCommit,(void));
	int  CFPROTOI(IrisTRollback,(int));
	int  CFPROTOI(IrisTLevel,(void));
	int  CFPROTOI(IrisCvtExStrInA,(IRIS_EXSTRP, IRIS_ASTRP, IRIS_EXSTRP));
	int  CFPROTOI(IrisCvtExStrInW,(IRIS_EXSTRP, IRISWSTRP, IRIS_EXSTRP));
	int  CFPROTOI(IrisCvtExStrOutA,(IRIS_EXSTRP, IRIS_ASTRP, IRIS_EXSTRP));
	int  CFPROTOI(IrisCvtExStrOutW,(IRIS_EXSTRP, IRISWSTRP, IRIS_EXSTRP));
	int  CFPROTOI(IrisPushIEEEDbl,(double));
#if	IRIS_HSTR
	int  CFPROTOI(IrisExecuteH,(IRISHSTRP volatile));
	int  CFPROTOI(IrisEvalH,(IRISHSTRP volatile));
	int  CFPROTOI(IrisPromptH,(IRISHSTRP));
	int  CFPROTOI(IrisErrxlateH,(int,IRISHSTRP));
	int  CFPROTOI(IrisErrorH,(IRISHSTRP,IRISHSTRP,int *));
	int  CFPROTOI(IrisCvtInH,(IRIS_ASTRP,IRISHSTRP,IRISHSTRP));
	int  CFPROTOI(IrisCvtOutH,(IRISHSTRP,IRISHSTRP,IRIS_ASTRP));
	int  CFPROTOI(IrisPushStrH,(int,const wchar_t *));
	int  CFPROTOI(IrisPushRtnH,(unsigned int *,int,const wchar_t *,int,const wchar_t *));
	int  CFPROTOI(IrisPushRtnXH,(unsigned int *,int,const wchar_t *,int,int,const wchar_t *,int,const wchar_t *));
	int  CFPROTOI(IrisPushFuncH,(unsigned int *,int,const wchar_t *,int,const wchar_t *));
	int  CFPROTOI(IrisPushFuncXH,(unsigned int *,int,const wchar_t *,int,int,const wchar_t*,int,const wchar_t *));
	int  CFPROTOI(IrisPushGlobalH,(int,const wchar_t *));
	int  CFPROTOI(IrisPushGlobalXH,(int,const wchar_t *,int,const wchar_t *));
	int  CFPROTOI(IrisPushClassMethodH,(int,const wchar_t *,int,const wchar_t *,int));
	int  CFPROTOI(IrisPushMethodH,(unsigned int,int,const wchar_t *,int));
	int  CFPROTOI(IrisPushPropertyH,(unsigned int,int,const wchar_t *));
	int  CFPROTOI(IrisPopStrH,(int *,wchar_t **));
	int  CFPROTOI(IrisPushCvtH,(int,const wchar_t *));
	int  CFPROTOI(IrisPopCvtH,(int *,wchar_t **));
	int  CFPROTOI(IrisPushLockH,(int,const wchar_t *));
	int  CFPROTOI(IrisPushLockXH,(int,const wchar_t *,int,const wchar_t *));
	int  CFPROTOI(IrisCvtExStrInH,(IRIS_EXSTRP,IRISHSTRP,IRIS_EXSTRP));
	int  CFPROTOI(IrisCvtExStrOutH,(IRIS_EXSTRP,IRISHSTRP,IRIS_EXSTRP));
#endif	/* IRIS_HSTR */
        int  CFPROTOI(IrisSPCSend,(int,const Callin_char_t *));
        int  CFPROTOI(IrisSPCReceive,(int *,Callin_char_t *));
	int  CFPROTOI(IrisExStrKill,(IRIS_EXSTRP));
	unsigned char *CFPROTOI(IrisExStrNew,(IRIS_EXSTRP,int));
	unsigned short *CFPROTOI(IrisExStrNewW,(IRIS_EXSTRP,int));
#if IRIS_ZHARR
	wchar_t *CFPROTOI(IrisExStrNewH,(IRIS_EXSTRP,int));
#else
	unsigned short *CFPROTOI(IrisExStrNewH,(IRIS_EXSTRP,int));
#endif
	int  CFPROTOI(IrisPopExStr,(IRIS_EXSTRP));
	int  CFPROTOI(IrisPopExStrW,(IRIS_EXSTRP));
	int  CFPROTOI(IrisPushExStr,(IRIS_EXSTRP));
	int  CFPROTOI(IrisPushExStrW,(IRIS_EXSTRP));
#if IRIS_HSTR
	int  CFPROTOI(IrisPushExStrH,(IRIS_EXSTRP));
	int  CFPROTOI(IrisPopExStrH,(IRIS_EXSTRP));
	int  CFPROTOI(IrisPushExStrCvtH,(IRIS_EXSTRP));
	int  CFPROTOI(IrisPopExStrCvtH,(IRIS_EXSTRP));
#endif	/* IRIS_HSTR */
	int  CFPROTOI(IrisPushExList,(IRIS_EXSTRP));
	int  CFPROTOI(IrisPopExList,(IRIS_EXSTRP));
	int  CFPROTOI(IrisPushExStrCvtW,(IRIS_EXSTRP));
	int  CFPROTOI(IrisPopExStrCvtW,(IRIS_EXSTRP));
	int  CFPROTOI(IrisPushUndef,(void));
	int  CFPROTOI(IrisPushInt64,(IRIS_INT64));
	int  CFPROTOI(IrisPopInt64,(IRIS_INT64 *));
	int  CFPROTOI(IrisPushSSVN,(int,const Callin_char_t *));
	int  CFPROTOI(IrisPushSSVNX,(int,const Callin_char_t *,int,const Callin_char_t *));
	int  CFPROTOI(IrisPushSSVNW,(int,const unsigned short *));
	int  CFPROTOI(IrisPushSSVNXW,(int,const unsigned short *,int,const unsigned short *));
	int  CFPROTOI(IrisAddSSVN,(int,const Callin_char_t *));
	int  CFPROTOI(IrisAddSSVNX,(int,const Callin_char_t *,int,const Callin_char_t *));
	int  CFPROTOI(IrisAddSSVNW,(int,const unsigned short *));
	int  CFPROTOI(IrisAddSSVNXW,(int,const unsigned short *,int,const unsigned short *));
#if IRIS_HSTR
	int  CFPROTOI(IrisPushSSVNH,(int,const wchar_t *));
	int  CFPROTOI(IrisPushSSVNXH,(int,const wchar_t *,int,const wchar_t *));
	int  CFPROTOI(IrisAddSSVNH,(int,const wchar_t *));
	int  CFPROTOI(IrisAddSSVNXH,(int,const wchar_t *,int,const wchar_t *));
	int  CFPROTOI(IrisAddGlobalH,(int,const wchar_t *));
	int  CFPROTOI(IrisAddGlobalXH,(int,const wchar_t *,int,const wchar_t *));
#endif	/* IRIS_HSTR */
	int  CFPROTOI(IrisAddGlobal,(int,const Callin_char_t *));
	int  CFPROTOI(IrisAddGlobalX,(int,const Callin_char_t *,int,const Callin_char_t *));
	int  CFPROTOI(IrisAddGlobalW,(int,const unsigned short *));
	int  CFPROTOI(IrisAddGlobalXW,(int,const unsigned short *,int,const unsigned short *));
	int  CFPROTOI(IrisAddSSVNDescriptor,(int));
	int  CFPROTOI(IrisAddGlobalDescriptor,(int));
	int  CFPROTOI(IrisAddLocalDescriptor,(int));
	int  CFPROTOI(IrisSSVNGet,(int));
	int  CFPROTOI(IrisSSVNKill,(int,int));
	int  CFPROTOI(IrisSSVNOrder,(int,int));
	int  CFPROTOI(IrisSSVNQuery,(int,int));
	int  CFPROTOI(IrisSSVNData,(int));
	int  CFPROTOI(IrisMerge,(void));
	int  CFPROTOI(IrisEnableMultiThread,(void));
	int  CFPROTOI(IrisiKnowFuncs,(IRIS_IKNOWFUNCSP));
	int  CFPROTOI(IrisExecuteArgs,(int));
	int  CFPROTOI(IrisPushExecuteFuncA,(int,const Callin_char_t *));
	int  CFPROTOI(IrisPushExecuteFuncW,(int,const unsigned short *));
#if IRIS_HSTR
	int  CFPROTOI(IrisPushExecuteFuncH,(int,const wchar_t *));
#endif	/* IRIS_HSTR */
	int  CFPROTOI(IrisCallExecuteFunc,(int));
} IRISEFT;

#ifndef	ZF_DLL		/*==============================*/

/*
			Function Prototypes

	These entry points are used when the user module is linked
	together with the Iris object module.
*/
extern	int  CFPROTOD(IrisStartA,(unsigned long,int,IRIS_ASTRP,IRIS_ASTRP));
extern	int  CFPROTOD(IrisStartW,(unsigned long,int,IRISWSTRP,IRISWSTRP));
								/* STC1114+ */
extern	int  CFPROTOD(IrisSecureStartA,(IRIS_ASTRP,IRIS_ASTRP,IRIS_ASTRP,unsigned long,int,IRIS_ASTRP,IRIS_ASTRP));
extern	int  CFPROTOD(IrisSecureStartW,(IRISWSTRP,IRISWSTRP,IRISWSTRP,unsigned long,int,IRISWSTRP,IRISWSTRP));
extern	int  CFPROTOD(IrisChangePasswordA,(IRIS_ASTRP,IRIS_ASTRP,IRIS_ASTRP));
extern	int  CFPROTOD(IrisChangePasswordW,(IRISWSTRP,IRISWSTRP,IRISWSTRP));
								/* STC1114- */
extern	int  CFPROTOD(IrisExecuteA,(IRIS_ASTRP volatile));
extern	int  CFPROTOD(IrisExecuteW,(IRISWSTRP volatile));
extern	int  CFPROTOD(IrisEvalA,(IRIS_ASTRP volatile));
extern	int  CFPROTOD(IrisEvalW,(IRISWSTRP volatile));
extern	int  CFPROTOD(IrisContext,(void));
extern	int  CFPROTOD(IrisCvtInA,(IRIS_ASTRP,IRIS_ASTRP,IRIS_ASTRP));
extern	int  CFPROTOD(IrisCvtInW,(IRIS_ASTRP,IRISWSTRP,IRISWSTRP));
extern	int  CFPROTOD(IrisCvtOutA,(IRIS_ASTRP,IRIS_ASTRP,IRIS_ASTRP));
extern	int  CFPROTOD(IrisCvtOutW,(IRISWSTRP,IRISWSTRP,IRIS_ASTRP));
extern	int  CFPROTOD(IrisCvtExStrInA,(IRIS_EXSTRP, IRIS_ASTRP, IRIS_EXSTRP));
extern	int  CFPROTOD(IrisCvtExStrInW,(IRIS_EXSTRP, IRISWSTRP, IRIS_EXSTRP));
extern	int  CFPROTOD(IrisCvtExStrOutA,(IRIS_EXSTRP, IRIS_ASTRP, IRIS_EXSTRP));
extern	int  CFPROTOD(IrisCvtExStrOutW,(IRIS_EXSTRP, IRISWSTRP, IRIS_EXSTRP));
extern	int  CFPROTOD(IrisPromptA,(IRIS_ASTRP));
extern	int  CFPROTOD(IrisPromptW,(IRISWSTRP));
extern	int  CFPROTOD(IrisErrxlateA,(int,IRIS_ASTRP));
extern	int  CFPROTOD(IrisErrxlateW,(int,IRISWSTRP));
extern	int  CFPROTOD(IrisCtrl,(unsigned long));
extern	int  CFPROTOD(IrisConvert,(unsigned long,void *));
extern	int  CFPROTOD(IrisType,(void));
extern	int  CFPROTOD(IrisAbort,(unsigned long));
extern	int  CFPROTOD(IrisEnd,(void));
extern	void CFPROTOD(IrisOflush,(void));
extern	int  CFPROTOD(IrisSignal,(int));
extern	int  CFPROTOD(IrisErrorA,(IRIS_ASTRP,IRIS_ASTRP,int *));
extern	int  CFPROTOD(IrisErrorW,(IRISWSTRP,IRISWSTRP,int *));
extern	int  CFPROTOD(IrisPushInt,(int));
extern	int  CFPROTOD(IrisPushDbl,(double));
extern	int  CFPROTOD(IrisPushUndef,(void));			/* CDS1472 */
extern	int  CFPROTOD(IrisPushIEEEDbl,(double));
extern	int  CFPROTOD(IrisPushInt64,(IRIS_INT64));	/* SAP1134, SOH106 */
extern	int  CFPROTOD(IrisPushOref,(unsigned int));
extern	int  CFPROTOD(IrisPushStr,(int,const Callin_char_t *));
extern	int  CFPROTOD(IrisPushStrW,(int,const unsigned short *));
extern	int  CFPROTOD(IrisPushExStr,(IRIS_EXSTRP));		/* KMK497+ */
extern	int  CFPROTOD(IrisPushExStrW,(IRIS_EXSTRP));		/* KMK497- */
extern	int  CFPROTOD(IrisPushRtn,(unsigned int *,int,const Callin_char_t *,int,const Callin_char_t *));
extern	int  CFPROTOD(IrisPushRtnX,(unsigned int *,int,const Callin_char_t *,int,int,const Callin_char_t*,int,const Callin_char_t *));
extern	int  CFPROTOD(IrisPushRtnW,(unsigned int *,int,const unsigned short *,int,const unsigned short *));
extern	int  CFPROTOD(IrisPushRtnXW,(unsigned int *,int,const unsigned short *,int,int,const unsigned short *,int,const unsigned short *));
extern	int  CFPROTOD(IrisPushFunc,(unsigned int *,int,const Callin_char_t *,int,const Callin_char_t *));
extern	int  CFPROTOD(IrisPushFuncX,(unsigned int *,int,const Callin_char_t *,int,int,const Callin_char_t*,int,const Callin_char_t *));
extern	int  CFPROTOD(IrisPushFuncW,(unsigned int *,int,const unsigned short *,int,const unsigned short *));
extern	int  CFPROTOD(IrisPushFuncXW,(unsigned int *,int,const unsigned short *,int,int,const unsigned short*,int,const unsigned short *));
extern	int  CFPROTOD(IrisPushGlobal,(int,const Callin_char_t *));
extern	int  CFPROTOD(IrisPushGlobalX,(int,const Callin_char_t *,int,const Callin_char_t *));
extern	int  CFPROTOD(IrisPushGlobalW,(int,const unsigned short *));
extern	int  CFPROTOD(IrisPushGlobalXW,(int,const unsigned short *,int,const unsigned short *));
extern	int  CFPROTOD(IrisUnPop,(void));
extern	int  CFPROTOD(IrisPop,(void **));
extern	int  CFPROTOD(IrisPopInt,(int *));
extern	int  CFPROTOD(IrisPopInt64,(IRIS_INT64 *));	/* SAP1134, SOH106 */
extern	int  CFPROTOD(IrisPopDbl,(double *));
extern	int  CFPROTOD(IrisPopOref,(unsigned int *));
extern	int  CFPROTOD(IrisPopStr,(int *,Callin_char_t **));
extern	int  CFPROTOD(IrisPopStrW,(int *,unsigned short **));
extern  int  CFPROTOD(IrisPopExStr,(IRIS_EXSTRP));		/* KMK497+ */
extern  int  CFPROTOD(IrisPopExStrW,(IRIS_EXSTRP));		/* KMK497- */
extern	int  CFPROTOD(IrisSetVar,(int,const Callin_char_t *));
extern	int  CFPROTOD(IrisGetVar,(int,const Callin_char_t *));
extern	int  CFPROTOD(IrisDoRtn,(unsigned int,int));
extern	int  CFPROTOD(IrisDoFun,(unsigned int,int));
extern	int  CFPROTOD(IrisExtFun,(unsigned int,int));
extern	int  CFPROTOD(IrisExecuteArgs,(int));
extern	int  CFPROTOD(IrisPushExecuteFuncA,(int,const Callin_char_t *));
extern	int  CFPROTOD(IrisPushExecuteFuncW,(int,const unsigned short *));
extern	int  CFPROTOD(IrisCallExecuteFunc,(int));
extern	int  CFPROTOD(IrisGlobalGet,(int,int));
extern	int  CFPROTOD(IrisGlobalGetBinary,(int,int,int *,Callin_char_t **));
extern	int  CFPROTOD(IrisGlobalSet,(int));
extern	int  CFPROTOD(IrisGlobalIncrement,(int));
extern	int  CFPROTOD(IrisPushClassMethod,(int,const Callin_char_t *,int,const Callin_char_t *,int));
extern	int  CFPROTOD(IrisPushClassMethodW,(int,const unsigned short *,int,const unsigned short *,int));
extern	int  CFPROTOD(IrisPushMethod,(unsigned int,int,const Callin_char_t *,int));
extern	int  CFPROTOD(IrisPushMethodW,(unsigned int,int,const unsigned short *,int));
extern	int  CFPROTOD(IrisInvokeClassMethod,(int));
extern	int  CFPROTOD(IrisInvokeMethod,(int));
extern	int  CFPROTOD(IrisPushProperty,(unsigned int,int,const Callin_char_t *));
extern	int  CFPROTOD(IrisPushPropertyW,(unsigned int,int,const unsigned short *));
extern	int  CFPROTOD(IrisGetProperty,(void));
extern	int  CFPROTOD(IrisSetProperty,(void));
extern	int  CFPROTOD(IrisIncrementCountOref,(unsigned int));
extern	int  CFPROTOD(IrisCloseOref,(unsigned int));
extern	int  CFPROTOD(IrisPushCvtW,(int,const unsigned short *));
extern	int  CFPROTOD(IrisPopCvtW,(int *,unsigned short **));
extern	int  CFPROTOD(IrisPushExStrCvtW,(IRIS_EXSTRP));		/* SAP1121 */
extern	int  CFPROTOD(IrisPopExStrCvtW,(IRIS_EXSTRP));		/* SAP1121 */
extern	int  CFPROTOD(IrisPushList,(int,const Callin_char_t *));
extern	int  CFPROTOD(IrisPopList,(int *,Callin_char_t **));
extern	int  CFPROTOD(IrisPushPtr,(void *));
extern	int  CFPROTOD(IrisPopPtr,(void **));
extern  int  CFPROTOD(IrisGlobalQuery,(int,int,int));		/* JO1874+ */
extern  int  CFPROTOD(IrisGlobalOrder,(int,int,int));
extern  int  CFPROTOD(IrisGlobalData,(int,int));
extern  int  CFPROTOD(IrisGlobalKill,(int,int));
extern  int  CFPROTOD(IrisGlobalRelease,(void));		/* JO1874- */
extern	int  CFPROTOD(IrisPushLock,(int,const Callin_char_t *)); /* JO1924+ */
extern	int  CFPROTOD(IrisPushLockX,(int,const Callin_char_t *,int,const Callin_char_t *));
extern	int  CFPROTOD(IrisPushLockW,(int,const unsigned short *));
extern	int  CFPROTOD(IrisPushLockXW,(int,const unsigned short *,int,const unsigned short *));
extern  int  CFPROTOD(IrisAcquireLock,(int,int,int,int *));
extern  int  CFPROTOD(IrisReleaseLock,(int,int));
extern  int  CFPROTOD(IrisReleaseAllLocks,(void));
extern  int  CFPROTOD(IrisTStart,(void));
extern  int  CFPROTOD(IrisTCommit,(void));
extern  int  CFPROTOD(IrisTRollback,(int));
extern  int  CFPROTOD(IrisTLevel,(void));		/* JO1924-,LRS1013- */
extern  int  CFPROTOD(IrisSetDir,(char *));			/* SAP609 */
#if	IRIS_HSTR						/* LRS1034+ */
extern	int  CFPROTOD(IrisStartH,(unsigned long,int,IRISHSTRP,IRISHSTRP));
								/* STC1114+ */
extern	int  CFPROTOD(IrisSecureStartH,(IRISHSTRP,IRISHSTRP,IRISHSTRP,unsigned long,int,IRISHSTRP,IRISHSTRP));
extern	int  CFPROTOD(IrisChangePasswordH,(IRISHSTRP,IRISHSTRP,IRISHSTRP));
								/* STC1114- */
extern	int  CFPROTOD(IrisExecuteH,(IRISHSTRP volatile));
extern	int  CFPROTOD(IrisEvalH,(IRISHSTRP volatile));
extern	int  CFPROTOD(IrisPromptH,(IRISHSTRP));
extern	int  CFPROTOD(IrisErrxlateH,(int,IRISHSTRP));
extern	int  CFPROTOD(IrisErrorH,(IRISHSTRP,IRISHSTRP,int *));
extern	int  CFPROTOD(IrisCvtInH,(IRIS_ASTRP,IRISHSTRP,IRISHSTRP));
extern	int  CFPROTOD(IrisCvtOutH,(IRISHSTRP,IRISHSTRP,IRIS_ASTRP));
extern	int  CFPROTOD(IrisPushStrH,(int,const wchar_t *));
extern	int  CFPROTOD(IrisPushExStrH,(IRIS_EXSTRP));		/* KMK497 */
extern	int  CFPROTOD(IrisPushExStrCvtH,(IRIS_EXSTRP));		/* KMK497 */
extern	int  CFPROTOD(IrisPushRtnH,(unsigned int *,int,const wchar_t *,int,const wchar_t *));
extern	int  CFPROTOD(IrisPushRtnXH,(unsigned int *,int,const wchar_t *,int,int,const wchar_t *,int,const wchar_t *));
extern	int  CFPROTOD(IrisPushFuncH,(unsigned int *,int,const wchar_t *,int,const wchar_t *));
extern	int  CFPROTOD(IrisPushFuncXH,(unsigned int *,int,const wchar_t *,int,int,const wchar_t*,int,const wchar_t *));
extern	int  CFPROTOD(IrisPushGlobalH,(int,const wchar_t *));
extern	int  CFPROTOD(IrisPushGlobalXH,(int,const wchar_t *,int,const wchar_t *));
extern	int  CFPROTOD(IrisPushClassMethodH,(int,const wchar_t *,int,const wchar_t *,int));
extern	int  CFPROTOD(IrisPushMethodH,(unsigned int,int,const wchar_t *,int));
extern	int  CFPROTOD(IrisPushPropertyH,(unsigned int,int,const wchar_t *));
extern	int  CFPROTOD(IrisPopStrH,(int *,wchar_t **));
extern	int  CFPROTOD(IrisPopExStrH,(IRIS_EXSTRP));		/* KMK497 */
extern	int  CFPROTOD(IrisPopExStrCvtH,(IRIS_EXSTRP));		/* KMK497 */
extern	int  CFPROTOD(IrisPushCvtH,(int,const wchar_t *));
extern	int  CFPROTOD(IrisPopCvtH,(int *,wchar_t **));
extern	int  CFPROTOD(IrisPushLockH,(int,const wchar_t *));
extern	int  CFPROTOD(IrisPushLockXH,(int,const wchar_t *,int,const wchar_t *));
extern	int  CFPROTOD(IrisCvtExStrInH,(IRIS_EXSTRP,IRISHSTRP,IRIS_EXSTRP));
extern	int  CFPROTOD(IrisCvtExStrOutH,(IRIS_EXSTRP,IRISHSTRP,IRIS_EXSTRP));
extern	int  CFPROTOD(IrisPushExecuteFuncH,(int,const wchar_t *));
#endif	/* IRIS_HSTR */						/* LRS1034- */

extern  int  CFPROTOD(IrisSPCSend,(int,const Callin_char_t *));  /* JMM072+ */
extern  int  CFPROTOD(IrisSPCReceive,(int *,Callin_char_t *));   /* JMM072+ */
extern	int CFPROTOD(IrisExStrKill,(IRIS_EXSTRP));	/* KMK409+ KMK498 */
extern	unsigned char *CFPROTOD(IrisExStrNew,(IRIS_EXSTRP,int));
extern	unsigned short *CFPROTOD(IrisExStrNewW,(IRIS_EXSTRP,int));
#if IRIS_ZHARR
extern	wchar_t *CFPROTOD(IrisExStrNewH,(IRIS_EXSTRP,int));
#else
extern	unsigned short *CFPROTOD(IrisExStrNewH,(IRIS_EXSTRP,int));
#endif								/* KMK409- */
extern	int  CFPROTOD(IrisPushExList,(IRIS_EXSTRP));		/* KMK523+ */
extern	int  CFPROTOD(IrisPopExList,(IRIS_EXSTRP));		/* KMK523- */
#ifdef	IRIS_UNICODE
extern	int  CFPROTOD(IrisBitFind,(int,unsigned short *,int,int,int));
#else
extern	int  CFPROTOD(IrisBitFind,(int,unsigned char *,int,int,int));
#endif
extern	int  CFPROTOD(IrisBitFindB,(int,unsigned char *,int,int,int));
extern	int  CFPROTOD(IrisEndAll,(void));
								/* JO2405+ */
extern	int  CFPROTOD(IrisPushSSVN,(int,const Callin_char_t *));
extern	int  CFPROTOD(IrisPushSSVNX,(int,const Callin_char_t *,int,const Callin_char_t *));
extern	int  CFPROTOD(IrisPushSSVNW,(int,const unsigned short *));
extern	int  CFPROTOD(IrisPushSSVNXW,(int,const unsigned short *,int,const unsigned short *));
extern	int  CFPROTOD(IrisAddSSVN,(int,const Callin_char_t *));
extern	int  CFPROTOD(IrisAddSSVNX,(int,const Callin_char_t *,int,const Callin_char_t *));
extern	int  CFPROTOD(IrisAddSSVNW,(int,const unsigned short *));
extern	int  CFPROTOD(IrisAddSSVNXW,(int,const unsigned short *,int,const unsigned short *));
#if	IRIS_HSTR
extern	int  CFPROTOD(IrisPushSSVNH,(int,const wchar_t *));
extern	int  CFPROTOD(IrisPushSSVNXH,(int,const wchar_t *,int,const wchar_t *));
extern	int  CFPROTOD(IrisAddSSVNH,(int,const wchar_t *));
extern	int  CFPROTOD(IrisAddSSVNXH,(int,const wchar_t *,int,const wchar_t *));
extern	int  CFPROTOD(IrisAddGlobalH,(int,const wchar_t *));
extern	int  CFPROTOD(IrisAddGlobalXH,(int,const wchar_t *,int,const wchar_t *));
#endif	/* IRIS_HSTR */
extern	int  CFPROTOD(IrisAddGlobal,(int,const Callin_char_t *));
extern	int  CFPROTOD(IrisAddGlobalX,(int,const Callin_char_t *,int,const Callin_char_t *));
extern	int  CFPROTOD(IrisAddGlobalW,(int,const unsigned short *));
extern	int  CFPROTOD(IrisAddGlobalXW,(int,const unsigned short *,int,const unsigned short *));
extern	int  CFPROTOD(IrisAddSSVNDescriptor,(int));
extern	int  CFPROTOD(IrisAddGlobalDescriptor,(int));
extern	int  CFPROTOD(IrisAddLocalDescriptor,(int));
extern	int  CFPROTOD(IrisSSVNGet,(int));
extern	int  CFPROTOD(IrisSSVNKill,(int,int));
extern	int  CFPROTOD(IrisSSVNOrder,(int,int));
extern	int  CFPROTOD(IrisSSVNQuery,(int,int));
extern	int  CFPROTOD(IrisSSVNData,(int));
extern	int  CFPROTOD(IrisMerge,(void));			/* JO2405- */
extern  int  CFPROTOD(IrisEnableMultiThread,(void));		/* JO2410 */
extern	int  CFPROTOD(IrisiKnowFuncs,(IRIS_IKNOWFUNCSP));


#define	IRISSTARTA(flags,tout,prinp,prout) IrisStartA(flags,tout,prinp,prout)
#define	IRISSTARTW(flags,tout,prinp,prout) IrisStartW(flags,tout,prinp,prout)
								/* STC1114+ */
#define	IRISSECURESTARTA(username,password,exename,flags,tout,prinp,prout) IrisSecureStartA(username,password,exename,flags,tout,prinp,prout)
#define	IRISSECURESTARTW(username,password,exename,flags,tout,prinp,prout) IrisSecureStartW(username,password,exename,flags,tout,prinp,prout)
#define	IRISCHANGEPASSWORDA(username,oldpassword,newpassword) IrisChangePasswordA(username,oldpassword,newpassword)
#define	IRISCHANGEPASSWORDW(username,oldpassword,newpassword) IrisChangePasswordW(username,oldpassword,newpassword)
								/* STC1114- */
#define	IRISEXECUTEA(cmd)	IrisExecuteA(cmd)
#define	IRISEXECUTEW(cmd)	IrisExecuteW(cmd)
#define	IRISEVALA(expr)	IrisEvalA(expr)
#define	IRISEVALW(expr)	IrisEvalW(expr)
#define	IRISCONTEXT()		IrisContext()
#define	IRISPROMPTA(rbuf)	IrisPromptA(rbuf)
#define	IRISPROMPTW(rbuf)	IrisPromptW(rbuf)
#define	IRISERRXLATEA(code,rbuf) IrisErrxlateA(code,rbuf)
#define	IRISERRXLATEW(code,rbuf) IrisErrxlateW(code,rbuf)
#define IRISERRORA(msg,src,offp) IrisErrorA(msg,src,offp)
#define IRISERRORW(msg,src,offp) IrisErrorW(msg,src,offp)
#define	IRISCTRL(flags)	IrisCtrl(flags)
#define	IRISCONVERT(type,rbuf)	IrisConvert(type,rbuf)
#define	IRISTYPE()		IrisType()
#define	IRISABORT(type)	IrisAbort(type)
#define	IRISEND()		IrisEnd()
#define	IRISENDALL()		IrisEndAll()
#define	IRISOFLUSH()		IrisOflush()
#define	IRISCVTINA(src,tbl,res) IrisCvtInA(src,tbl,res)
#define	IRISCVTINW(src,tbl,res) IrisCvtInW(src,tbl,res)
#define	IRISCVTOUTA(src,tbl,res) IrisCvtOutA(src,tbl,res)
#define	IRISCVTOUTW(src,tbl,res) IrisCvtOutW(src,tbl,res)
#define	IRISCVTEXSTRINA(src,tbl,res) IrisCvtExStrInA(src,tbl,res)
#define	IRISCVTEXSTRINW(src,tbl,res) IrisCvtExStrInW(src,tbl,res)
#define	IRISCVTEXSTROUTA(src,tbl,res) IrisCvtExStrOutA(src,tbl,res)
#define	IRISCVTEXSTROUTW(src,tbl,res) IrisCvtExStrOutW(src,tbl,res)
#define	IRISSIGNAL(signal)	IrisSignal(signal)
#define	IRISPUSHINT(num)	IrisPushInt(num)
#define	IRISPUSHDBL(num)	IrisPushDbl(num)
#define	IRISPUSHIEEEDBL(num)	IrisPushIEEEDbl(num)
#define	IRISPUSHOREF(oref)	IrisPushOref(oref)
#define	IRISPUSHUNDEF()		IrisPushUndef()			/* CDS1472 */
#define IRISPUSHINT64(num)	IrisPushInt64(num)		/* SAP1134 */
#define	IRISPUSHSTRA(len,ptr)	(IrisPushStr)(len,ptr)		/* SOH393 */
#define	IRISPUSHSTRW(len,ptr)	IrisPushStrW(len,ptr)
#define	IRISPUSHEXSTRA(sptr)	(IrisPushExStr)(sptr)		/*KMK497+,SOH393*/
#define	IRISPUSHEXSTRW(sptr)	IrisPushExStrW(sptr)		/* KMK497- */
#define	IRISPUSHRTNA(rflag,tlen,tptr,nlen,nptr)			\
	(IrisPushRtn)(rflag,tlen,tptr,nlen,nptr)		/* SOH393 */
#define	IRISPUSHRTNXA(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)		\
	(IrisPushRtnX)(rflag,tlen,tptr,off,elen,eptr,nlen,nptr) /* SOH393 */
#define	IRISPUSHRTNW(rflag,tlen,tptr,nlen,nptr)			\
	IrisPushRtnW(rflag,tlen,tptr,nlen,nptr)
#define	IRISPUSHRTNXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)		\
	IrisPushRtnXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	IRISPUSHFUNCA(rflag,tlen,tptr,nlen,nptr)			\
	(IrisPushFunc)(rflag,tlen,tptr,nlen,nptr)		/* SOH393 */
#define	IRISPUSHFUNCXA(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	\
	(IrisPushFuncX)(rflag,tlen,tptr,off,elen,eptr,nlen,nptr) /* SOH393 */
#define	IRISPUSHFUNCW(rflag,tlen,tptr,nlen,nptr)			\
	IrisPushFuncW(rflag,tlen,tptr,nlen,nptr)
#define	IRISPUSHFUNCXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	\
	IrisPushFuncXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	IRISPUSHGLOBALA(nlen,nptr) (IrisPushGlobal)(nlen,nptr) /* SOH393 */
#define	IRISPUSHGLOBALXA(nlen,nptr,elen,eptr)				\
	      (IrisPushGlobalX)(nlen,nptr,elen,eptr)		/* SOH393 */
#define	IRISPUSHGLOBALW(nlen,nptr) IrisPushGlobalW(nlen,nptr)
#define	IRISPUSHGLOBALXW(nlen,nptr,elen,eptr)				\
	      IrisPushGlobalXW(nlen,nptr,elen,eptr)
#define IRISUNPOP()		IrisUnPop()
#define IRISPOP(arg)		IrisPop(arg)
#define IRISPOPINT(nump)	IrisPopInt(nump)
#define IRISPOPDBL(nump)	IrisPopDbl(nump)
#define IRISPOPOREF(orefp)	IrisPopOref(orefp)
#define IRISPOPINT64(num)	IrisPopInt64(num)		/* SAP1134 */
#define IRISPOPSTRA(lenp,strp)	(IrisPopStr)(lenp,strp)	/* SOH393 */
#define IRISPOPSTRW(lenp,strp)	IrisPopStrW(lenp,strp)
#define	IRISPOPEXSTRA(sstrp)	(IrisPopExStr)(sstrp)		/*KMK497+,SOH393*/
#define	IRISPOPEXSTRW(sstrp)	IrisPopExStrW(sstrp)		/* KMK497- */
#define IRISSETVAR(len,namep)	IrisSetVar(len,namep)		/* SOH392 */
#define IRISGETVAR(len,namep)	IrisGetVar(len,namep)		/* SOH392 */
#define IRISDORTN(flags,narg)	IrisDoRtn(flags,narg)
#define IRISEXTFUN(flags,narg)	IrisExtFun(flags,narg)
#define IRISEXECUTEARGS(narg)	IrisExecuteArgs(narg)
#define IRISPUSHEXECUTEFUNCA(len,ptr) IrisPushExecuteFuncA(len,ptr)
#define IRISPUSHEXECUTEFUNCW(len,ptr) IrisPushExecuteFuncW(len,ptr)
#define IRISCALLEXECUTEFUNC(narg) IrisCallExecuteFunc(narg)
#define IRISGLOBALGET(narg,flag) IrisGlobalGet(narg,flag)
#define IRISGLOBALSET(narg)	IrisGlobalSet(narg)
#define IRISGLOBALINCREMENT(narg) IrisGlobalIncrement(narg)
								/* JO1874+ */
#define IRISGLOBALQUERY(narg,dir,valueflag) \
		 IrisGlobalQuery(narg,dir,valueflag)
#define IRISGLOBALORDER(narg,dir,valueflag) \
		 IrisGlobalOrder(narg,dir,valueflag)
#define IRISGLOBALDATA(narg,valueflag) IrisGlobalData(narg,valueflag)
#define IRISGLOBALKILL(narg,nodeonly) IrisGlobalKill(narg,nodeonly)
#define IRISGLOBALRELEASE() IrisGlobalRelease()
								/* JO1874- */
								/* JO1924+ */
#define	IRISPUSHLOCKA(nlen,nptr) (IrisPushLock)(nlen,nptr)	/* SOH393 */
#define	IRISPUSHLOCKXA(nlen,nptr,elen,eptr)				\
    (IrisPushLockX)(nlen,nptr,elen,eptr)			/* SOH393 */
#define	IRISPUSHLOCKW(nlen,nptr) IrisPushLockW(nlen,nptr)
#define	IRISPUSHLOCKXW(nlen,nptr,elen,eptr)				\
    IrisPushLockXW(nlen,nptr,elen,eptr)
								/* JO1924- */
#define IRISDOFUN(flags,narg)	IrisDoFun(flags,narg)
#define	IRISPUSHCLASSMETHODA(clen,cptr,mlen,mptr,flg)			\
		(IrisPushClassMethod)(clen,cptr,mlen,mptr,flg) /* SOH393 */
#define	IRISPUSHCLASSMETHODW(clen,cptr,mlen,mptr,flg)			\
	       IrisPushClassMethodW(clen,cptr,mlen,mptr,flg)
#define	IRISPUSHMETHODA(oref,mlen,mptr,flg)				\
		(IrisPushMethod)(oref,mlen,mptr,flg)		/* SOH393 */
#define	IRISPUSHMETHODW(oref,mlen,mptr,flg)				\
		IrisPushMethodW(oref,mlen,mptr,flg)
#define IRISINVOKECLASSMETHOD(narg) IrisInvokeClassMethod(narg)
#define IRISINVOKEMETHOD(narg)	 IrisInvokeMethod(narg)
#define	IRISPUSHPROPERTYA(oref,plen,pptr)				\
		(IrisPushProperty)(oref,plen,pptr)		/* SOH393 */
#define	IRISPUSHPROPERTYW(oref,plen,pptr) IrisPushPropertyW(oref,plen,pptr)
#define	IRISGETPROPERTY()	IrisGetProperty()
#define IRISSETPROPERTY()	IrisSetProperty()
#define	IRISINCREMENTCOUNTOREF(oref) IrisIncrementCountOref(oref)
#define	IRISCLOSEOREF(oref)	IrisCloseOref(oref)
#define	IRISPUSHCVTW(len,ptr)	IrisPushCvtW(len,ptr)
#define IRISPOPCVTW(lenp,strp)	IrisPopCvtW(lenp,strp)
#define	IRISPUSHEXSTRCVTW(sstrp)	IrisPushExStrCvtW(sstrp)
#define IRISPOPEXSTRCVTW(sstrp)	IrisPopExStrCvtW(sstrp)
#define	IRISPUSHLIST(len,ptr)	IrisPushList(len,ptr)
#define IRISPOPLIST(lenp,strp)	IrisPopList(lenp,strp)
#define IRISPUSHPTR(ptr)	IrisPushPtr(ptr)
#define IRISPOPPTR(ptrp)	IrisPopPtr(ptrp)
								/* JO1924+ */
#define IRISACQUIRELOCK(nsub,flg,tout,rval) IrisAcquireLock(nsub,flg,tout,rval)
#define IRISRELEASELOCK(nsub,flg) IrisReleaseLock(nsub,flg)
#define IRISRELEASEALLLOCKS() IrisReleaseAllLocks()
#define IRISTSTART() IrisTStart()
#define IRISTCOMMIT() IrisTCommit()
#define IRISTROLLBACK(nlev) IrisTRollback(nlev)
#define IRISTLEVEL() IrisTLevel()
								/* JO1924- */
#define IRISSPCSEND(len,ptr) IrisSPCSend(len,ptr)
#define IRISSPCRECEIVE(lenp,ptr) IrisSPCReceive(lenp,ptr)
#define IRISSETDIR(dir) IrisSetDir(dir)

#if	IRIS_HSTR						/* LRS1034+ */
#define	IRISSTARTH(flags,tout,prinp,prout) IrisStartH(flags,tout,prinp,prout)
								/* STC1114+ */
#define	IRISSECURESTARTH(username,password,exename,flags,tout,prinp,prout) IrisSecureStartH(username,password,exename,flags,tout,prinp,prout)
#define	IRISCHANGEPASSWORDH(username,oldpassword,newpassword) IrisChangePasswordH(username,oldpassword,newpassword)
								/* STC1114- */
#define	IRISEXECUTEH(cmd)	IrisExecuteH(cmd)
#define	IRISEVALH(expr)	IrisEvalH(expr)
#define	IRISPROMPTH(rbuf)	IrisPromptH(rbuf)
#define	IRISERRXLATEH(code,rbuf) IrisErrxlateH(code,rbuf)
#define IRISERRORH(msg,src,offp) IrisErrorH(msg,src,offp)
#define	IRISCVTINH(src,tbl,res) IrisCvtInH(src,tbl,res)
#define	IRISCVTOUTH(src,tbl,res) IrisCvtOutH(src,tbl,res)
#define	IRISPUSHSTRH(len,ptr)	IrisPushStrH(len,ptr)
#define	IRISPUSHEXSTRH(sptr)	IrisPushExStrH(sptr)		/* KMK497 */
#define	IRISPUSHEXSTRCVTH(sptr) IrisPushExStrCvtH(sptr)		/* KMK497 */
#define	IRISPUSHRTNH(rflag,tlen,tptr,nlen,nptr)			\
	IrisPushRtnH(rflag,tlen,tptr,nlen,nptr)
#define	IRISPUSHRTNXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)		\
	IrisPushRtnXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	IRISPUSHFUNCH(rflag,tlen,tptr,nlen,nptr)			\
	IrisPushFuncH(rflag,tlen,tptr,nlen,nptr)
#define	IRISPUSHFUNCXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	\
	IrisPushFuncXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	IRISPUSHGLOBALH(nlen,nptr) IrisPushGlobalH(nlen,nptr)
#define	IRISPUSHGLOBALXH(nlen,nptr,elen,eptr)				\
	IrisPushGlobalXH(nlen,nptr,elen,eptr)
#define	IRISPUSHCLASSMETHODH(clen,cptr,mlen,mptr,flg)			\
	IrisPushClassMethodH(clen,cptr,mlen,mptr,flg)
#define	IRISPUSHMETHODH(oref,mlen,mptr,flg)				\
	IrisPushMethodH(oref,mlen,mptr,flg)
#define	IRISPUSHPROPERTYH(oref,plen,pptr) IrisPushPropertyH(oref,plen,pptr)
#define IRISPOPSTRH(lenp,strp)	IrisPopStrH(lenp,strp)
#define	IRISPOPEXSTRH(sstrp)	IrisPopExStrH(sstrp)		/* KMK497 */
#define	IRISPOPEXSTRCVTH(sstrp) IrisPopExStrCvtH(sstrp)	/* KMK497 */
#define	IRISPUSHCVTH(len,ptr)	IrisPushCvtH(len,ptr)
#define IRISPOPCVTH(lenp,strp)	IrisPopCvtH(lenp,strp)
#define	IRISPUSHLOCKH(nlen,nptr) IrisPushLockH(nlen,nptr)
#define	IRISPUSHLOCKXH(nlen,nptr,elen,eptr) IrisPushLockXH(nlen,nptr,elen,eptr)
#define	IRISCVTEXSTRINH(src,tbl,res) IrisCvtExStrInH(src,tbl,res)
#define	IRISCVTEXSTROUTH(src,tbl,res) IrisCvtExStrOutH(src,tbl,res)
#define IRISPUSHEXECUTEFUNCH(len,ptr) IrisPushExecuteFuncH(len,ptr)
#else	/* IRIS_HSTR */
#define	IRISSTARTH(flags,tout,prinp,prout) IrisStartW(flags,tout,prinp,prout)
								/* STC1114+ */
#define	IRISSECURESTARTH(username,password,exename,flags,tout,prinp,prout) IrisSecureStartW(username,password,exename,flags,tout,prinp,prout)
#define	IRISCHANGEPASSWORDH(username,oldpassword,newpassword) IrisChangePasswordW(username,oldpassword,newpassword)
								/* STC1114- */
#define	IRISEXECUTEH(cmd)	IrisExecuteW(cmd)
#define	IRISEVALH(expr)	IrisEvalW(expr)
#define	IRISPROMPTH(rbuf)	IrisPromptW(rbuf)
#define	IRISERRXLATEH(code,rbuf) IrisErrxlateW(code,rbuf)
#define IRISERRORH(msg,src,offp) IrisErrorW(msg,src,offp)
#define	IRISCVTINH(src,tbl,res) IrisCvtInW(src,tbl,res)
#define	IRISCVTOUTH(src,tbl,res) IrisCvtOutW(src,tbl,res)
#define	IRISPUSHSTRH(len,ptr)	IrisPushStrW(len,ptr)
#define	IRISPUSHEXSTRH(sptr)	IrisPushExStrW(sptr)		/* KMK497 */
#define	IRISPUSHEXSTRCVTH(sptr) IrisPushExStrCvtW(sptr)		/* SAP1806 */
#define	IRISPUSHRTNH(rflag,tlen,tptr,nlen,nptr)			\
	IrisPushRtnW(rflag,tlen,tptr,nlen,nptr)
#define	IRISPUSHRTNXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)		\
	IrisPushRtnXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	IRISPUSHFUNCH(rflag,tlen,tptr,nlen,nptr)			\
	IrisPushFuncW(rflag,tlen,tptr,nlen,nptr)
#define	IRISPUSHFUNCXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	\
	IrisPushFuncXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	IRISPUSHGLOBALH(nlen,nptr) IrisPushGlobalW(nlen,nptr)
#define	IRISPUSHGLOBALXH(nlen,nptr,elen,eptr)				\
	IrisPushGlobalXW(nlen,nptr,elen,eptr)
#define	IRISPUSHCLASSMETHODH(clen,cptr,mlen,mptr,flg)			\
	IrisPushClassMethodW(clen,cptr,mlen,mptr,flg)
#define	IRISPUSHMETHODH(oref,mlen,mptr,flg)				\
	IrisPushMethodW(oref,mlen,mptr,flg)
#define	IRISPUSHPROPERTYH(oref,plen,pptr) IrisPushPropertyW(oref,plen,pptr)
#define IRISPOPSTRH(lenp,strp)	IrisPopStrW(lenp,strp)
#define	IRISPOPEXSTRH(sstrp)	IrisPopExStrW(sstrp)		/* KMK497 */
#define	IRISPOPEXSTRCVTH(sstrp) IrisPopExStrW(sstrp)		/* SAP829 */
#define	IRISPUSHCVTH(len,ptr)	IrisPushCvtW(len,ptr)
#define IRISPOPCVTH(lenp,strp)	IrisPopCvtW(lenp,strp)
#define	IRISPUSHLOCKH(nlen,nptr) IrisPushLockW(nlen,nptr)
#define	IRISPUSHLOCKXH(nlen,nptr,elen,eptr)				\
	IrisPushLockXW(nlen,nptr,elen,eptr)
#define	IRISCVTEXSTRINH(src,tbl,res) IrisCvtExStrInW(src,tbl,res)
#define	IRISCVTEXSTROUTH(src,tbl,res) IrisCvtExStrOutW(src,tbl,res)
#define IRISPUSHEXECUTEFUNCH(len,ptr) IrisPushExecuteFuncW(len,ptr)
#endif	/* IRIS_HSTR */						/* LRS1034- */

#define	IRISEXSTRKILL(obj) IrisExStrKill(obj)			/* KMK409+ */
#define	IRISEXSTRNEWA(zstr,size) (IrisExStrNew)(zstr,size)	/*KMK495+,SOH393*/
#define	IRISEXSTRNEWW(zstr,size) IrisExStrNewW(zstr,size)
#define	IRISEXSTRNEWH(zstr,size) IrisExStrNewH(zstr,size)	/* KMK409- *//* KMK495- */
#define	IRISPUSHEXLIST(sstr) IrisPushExList(sstr)		/* KMK523+ */
#define IRISPOPEXLIST(sstr) IrisPopExList(sstr)			/* KMK523- */
								/* JO2405+ */
#define	IRISPUSHSSVNA(nlen,nptr) (IrisPushSSVN)(nlen,nptr)	/* SOH393 */
#define	IRISPUSHSSVNXA(nlen,nptr,elen,eptr)				\
	(IrisPushSSVNX)(nlen,nptr,elen,eptr)			/* SOH393 */
#define	IRISPUSHSSVNW(nlen,nptr) IrisPushSSVNW(nlen,nptr)
#define	IRISPUSHSSVNXW(nlen,nptr,elen,eptr)				\
	IrisPushSSVNXW(nlen,nptr,elen,eptr)
#define	IRISADDSSVNA(nlen,nptr) (IrisAddSSVN)(nlen,nptr)	/* SOH393 */
#define	IRISADDSSVNXA(nlen,nptr,elen,eptr)				\
	(IrisAddSSVNX)(nlen,nptr,elen,eptr)			/* SOH393 */
#define	IRISADDSSVNW(nlen,nptr) IrisAddSSVNW(nlen,nptr)
#define	IRISADDSSVNXW(nlen,nptr,elen,eptr)				\
	IrisAddSSVNXW(nlen,nptr,elen,eptr)

#if	IRIS_HSTR
#define	IRISPUSHSSVNH(nlen,nptr) IrisPushSSVNH(nlen,nptr)
#define	IRISPUSHSSVNXH(nlen,nptr,elen,eptr)				\
	IrisPushSSVNXH(nlen,nptr,elen,eptr)
#define	IRISADDSSVNH(nlen,nptr) IrisAddSSVNH(nlen,nptr)
#define	IRISADDSSVNXH(nlen,nptr,elen,eptr)				\
	IrisAddSSVNXH(nlen,nptr,elen,eptr)
#define	IRISADDGLOBALH(nlen,nptr) IrisAddGlobalH(nlen,nptr)
#define	IRISADDGLOBALXH(nlen,nptr,elen,eptr)				\
	IrisAddGlobalXH(nlen,nptr,elen,eptr)
#else	/* IRIS_HSTR */
#define	IRISPUSHSSVNH(nlen,nptr) IrisPushSSVNW(nlen,nptr)
#define	IRISPUSHSSVNXH(nlen,nptr,elen,eptr)				\
	IrisPushSSVNXW(nlen,nptr,elen,eptr)
#define	IRISADDSSVNH(nlen,nptr) IrisAddSSVNW(nlen,nptr)
#define	IRISADDSSVNXH(nlen,nptr,elen,eptr)				\
	IrisAddSSVNXW(nlen,nptr,elen,eptr)
#define	IRISADDGLOBALH(nlen,nptr) IrisAddGlobalW(nlen,nptr)
#define	IRISADDGLOBALXH(nlen,nptr,elen,eptr)				\
	IrisAddGlobalXW(nlen,nptr,elen,eptr)
#endif	/* IRIS_HSTR */

#define	IRISADDGLOBALA(nlen,nptr) (IrisAddGlobal)(nlen,nptr)	/* SOH393 */
#define	IRISADDGLOBALXA(nlen,nptr,elen,eptr)				\
	(IrisAddGlobalX)(nlen,nptr,elen,eptr)			/* SOH393 */
#define	IRISADDGLOBALW(nlen,nptr) IrisAddGlobalW(nlen,nptr)
#define	IRISADDGLOBALXW(nlen,nptr,elen,eptr)				\
	IrisAddGlobalXW(nlen,nptr,elen,eptr)
#define	IRISADDSSVNDESCRIPTOR(num)	IrisAddSSVNDescriptor(num)
#define	IRISADDGLOBALDESCRIPTOR(num)	IrisAddGlobalDescriptor(num)
#define	IRISADDLOCALDESCRIPTOR(num)	IrisAddLocalDescriptor(num)
#define IRISSSVNGET(num)		IrisSSVNGet(num)
#define IRISSSVNKILL(num,val)		IrisSSVNKill(num,val)
#define IRISSSVNORDER(num,val)		IrisSSVNOrder(num,val)
#define IRISSSVNQUERY(num,val)		IrisSSVNQuery(num,val)
#define IRISSSVNDATA(num)		IrisSSVNData(num)
#define IRISMERGE()			IrisMerge()		/* JO2405- */
#define IRISENABLEMULTITHREAD()	IrisEnableMultiThread()		/* JO2410 */
#define IRISIKNOWFUNCS(funcsp)		IrisiKnowFuncs(funcsp)

#else	/* ZF_DLL */	/*==============================*/

/*
			Interface Function Definitions

	The export function table entries are used for calling back
	into IRIS from a dynamically loaded user library module,
	i.e. one loaded using $ZF(-n).
*/
#define	IRISEXECUTEA(cmd)	(*irisfunctabp->IrisExecuteA)(cmd)
#define	IRISEXECUTEW(cmd)	(*irisfunctabp->IrisExecuteW)(cmd)
#define	IRISEVALA(expr)	(*irisfunctabp->IrisEvalA)(expr)
#define	IRISEVALW(expr)	(*irisfunctabp->IrisEvalW)(expr)
#define	IRISPROMPTA(rbuf)	(*irisfunctabp->IrisPromptA)(rbuf)
#define	IRISPROMPTW(rbuf)	(*irisfunctabp->IrisPromptW)(rbuf)
#define	IRISERRXLATEA(code,rbuf) (*irisfunctabp->IrisErrxlateA)(code,rbuf)
#define	IRISERRXLATEW(code,rbuf) (*irisfunctabp->IrisErrxlateW)(code,rbuf)
#define IRISERRORA(msg,src,offp) (*irisfunctabp->IrisErrorA)(msg,src,offp)
#define IRISERRORW(msg,src,offp) (*irisfunctabp->IrisErrorW)(msg,src,offp)
#define	IRISCONVERT(type,rbuf)	(*irisfunctabp->IrisConvert)(type,rbuf)
#define	IRISTYPE()		(*irisfunctabp->IrisType)()
#define	IRISABORT(type)	(*irisfunctabp->IrisAbort)(type)
#define	IRISSIGNAL(signal)	(*irisfunctabp->IrisSignal)(signal)
#define	IRISPUSHINT(num)	(*irisfunctabp->IrisPushInt)(num)
#define	IRISPUSHDBL(num)	(*irisfunctabp->IrisPushDbl)(num)
#define	IRISPUSHIEEEDBL(num)	(*irisfunctabp->IrisPushIEEEDbl)(num)
#define	IRISPUSHOREF(oref)	(*irisfunctabp->IrisPushOref)(oref)
#define	IRISPUSHUNDEF()	(*irisfunctabp->IrisPushUndef)()
#define IRISPUSHINT64(num)	(*irisfunctabp->IrisPushInt64)(num)
#define	IRISPUSHSTRA(len,ptr)	(*irisfunctabp->IrisPushStr)(len,ptr)
#define	IRISPUSHSTRW(len,ptr)	(*irisfunctabp->IrisPushStrW)(len,ptr)
#define	IRISPUSHEXSTRA(sptr)	(*irisfunctabp->IrisPushExStr)(sptr)
#define	IRISPUSHEXSTRW(sptr)	(*irisfunctabp->IrisPushExStrW)(sptr)
#define	IRISPUSHRTNA(rflag,tlen,tptr,nlen,nptr)			\
	(*irisfunctabp->IrisPushRtn)(rflag,tlen,tptr,nlen,nptr)
#define	IRISPUSHRTNXA(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)		\
	(*irisfunctabp->IrisPushRtnX)(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	IRISPUSHRTNW(rflag,tlen,tptr,nlen,nptr)			\
	(*irisfunctabp->IrisPushRtnW)(rflag,tlen,tptr,nlen,nptr)
#define	IRISPUSHRTNXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)		\
	(*irisfunctabp->IrisPushRtnXW)(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	IRISPUSHFUNCA(rflag,tlen,tptr,nlen,nptr)			\
	(*irisfunctabp->IrisPushFunc)(rflag,tlen,tptr,nlen,nptr)
#define	IRISPUSHFUNCXA(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	\
	(*irisfunctabp->IrisPushFuncX)(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	IRISPUSHFUNCW(rflag,tlen,tptr,nlen,nptr)			\
	(*irisfunctabp->IrisPushFuncW)(rflag,tlen,tptr,nlen,nptr)
#define	IRISPUSHFUNCXW(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	\
	(*irisfunctabp->IrisPushFuncXW)(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	IRISPUSHGLOBALA(nlen,nptr) (*irisfunctabp->IrisPushGlobal)(nlen,nptr)
#define	IRISPUSHGLOBALXA(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisPushGlobalX)(nlen,nptr,elen,eptr)
#define	IRISPUSHGLOBALW(nlen,nptr) (*irisfunctabp->IrisPushGlobalW)(nlen,nptr)
#define	IRISPUSHGLOBALXW(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisPushGlobalXW)(nlen,nptr,elen,eptr)
#define IRISUNPOP()		(*irisfunctabp->IrisUnPop)()
#define IRISPOP(arg)		(*irisfunctabp->IrisPop)(arg)
#define IRISPOPINT(nump)	(*irisfunctabp->IrisPopInt)(nump)
#define IRISPOPDBL(nump)	(*irisfunctabp->IrisPopDbl)(nump)
#define IRISPOPOREF(orefp)	(*irisfunctabp->IrisPopOref)(orefp)
#define IRISPOPINT64(num)	(*irisfunctabp->IrisPopInt64)(num)
#define IRISPOPSTRA(lenp,strp)	(*irisfunctabp->IrisPopStr)(lenp,strp)
#define IRISPOPSTRW(lenp,strp)	(*irisfunctabp->IrisPopStrW)(lenp,strp)
#define IRISPOPEXSTRA(sstrp)	(*irisfunctabp->IrisPopExStr)(sstrp)
#define IRISPOPEXSTRW(sstrp)	(*irisfunctabp->IrisPopExStrW)(sstrp)
#define IRISSETVAR(len,namep) 	(*irisfunctabp->IrisSetVar)(len,namep)
#define IRISGETVAR(len,namep) 	(*irisfunctabp->IrisGetVar)(len,namep)
#define IRISDORTN(flags,narg)	(*irisfunctabp->IrisDoRtn)(flags,narg)
#define IRISEXTFUN(flags,narg)	(*irisfunctabp->IrisExtFun)(flags,narg)
#define IRISEXECUTEARGS(narg)	(*irisfunctabp->IrisExecuteArgs)(narg)
#define IRISPUSHEXECUTEFUNCA(len,ptr) (*irisfunctabp->IrisPushExecuteFuncA)(len,ptr)
#define IRISPUSHEXECUTEFUNCW(len,ptr) (*irisfunctabp->IrisPushExecuteFuncW)(len,ptr)
#define IRISCALLEXECUTEFUNC(narg) (*irisfunctabp->IrisCallExecuteFunc)(narg)
#define IRISGLOBALGET(narg,flag) (*irisfunctabp->IrisGlobalGet)(narg,flag)
#define IRISGLOBALSET(narg)	(*irisfunctabp->IrisGlobalSet)(narg)
#define IRISGLOBALINCREMENT(narg) (*irisfunctabp->IrisGlobalIncrement)(narg)
#define IRISGLOBALQUERY(narg,dir,valueflag) \
	(*irisfunctabp->IrisGlobalQuery)(narg,dir,valueflag)
#define IRISGLOBALORDER(narg,dir,valueflag) \
	(*irisfunctabp->IrisGlobalOrder)(narg,dir,valueflag)
#define IRISGLOBALDATA(narg,valueflag) \
	(*irisfunctabp->IrisGlobalData)(narg,valueflag)
#define IRISGLOBALKILL(narg,nodeonly) \
	(*irisfunctabp->IrisGlobalKill)(narg,nodeonly)
#define IRISGLOBALRELEASE() (*irisfunctabp->IrisGlobalRelease)()
#define IRISDOFUN(flags,narg)	(*irisfunctabp->IrisDoFun)(flags,narg)
#define	IRISPUSHCLASSMETHODA(clen,cptr,mlen,mptr,flg)			\
	(*irisfunctabp->IrisPushClassMethod)(clen,cptr,mlen,mptr,flg)
#define	IRISPUSHCLASSMETHODW(clen,cptr,mlen,mptr,flg)			\
	(*irisfunctabp->IrisPushClassMethodW)(clen,cptr,mlen,mptr,flg)
#define	IRISPUSHMETHODA(oref,mlen,mptr,flg)				\
	(*irisfunctabp->IrisPushMethod)(oref,mlen,mptr,flg)
#define	IRISPUSHMETHODW(oref,mlen,mptr,flg)				\
	(*irisfunctabp->IrisPushMethodW)(oref,mlen,mptr,flg)
#define IRISINVOKECLASSMETHOD(narg)					\
	(*irisfunctabp->IrisInvokeClassMethod)(narg)
#define IRISINVOKEMETHOD(narg)	(*irisfunctabp->IrisInvokeMethod)(narg)
#define	IRISPUSHPROPERTYA(oref,plen,pptr)				\
	(*irisfunctabp->IrisPushProperty)(oref,plen,pptr)
#define	IRISPUSHPROPERTYW(oref,plen,pptr)				\
	(*irisfunctabp->IrisPushPropertyW)(oref,plen,pptr)
#define IRISGETPROPERTY()	(*irisfunctabp->IrisGetProperty)()
#define IRISSETPROPERTY()	(*irisfunctabp->IrisSetProperty)()
#define	IRISINCREMENTCOUNTOREF(oref)					\
	(*irisfunctabp->IrisIncrementCountOref)(oref)
#define	IRISCLOSEOREF(oref)	(*irisfunctabp->IrisCloseOref)(oref)
#define	IRISPUSHCVTW(len,ptr)	(*irisfunctabp->IrisPushCvtW)(len,ptr)
#define IRISPOPCVTW(lenp,strp)	(*irisfunctabp->IrisPopCvtW)(lenp,strp)
#define	IRISPUSHLIST(len,ptr)	(*irisfunctabp->IrisPushList)(len,ptr)
#define IRISPOPLIST(lenp,strp)	(*irisfunctabp->IrisPopList)(lenp,strp)
#define IRISPUSHPTR(ptr)	(*irisfunctabp->IrisPushPtr)(ptr)
#define IRISPOPPTR(ptrp)	(*irisfunctabp->IrisPopPtr)(ptrp)
#define	IRISCONTEXT()		(*irisfunctabp->IrisContext)()
#define	IRISCTRL(flags)	(*irisfunctabp->IrisCtrl)(flags)
#define	IRISCVTINA(src,tbl,res) (*irisfunctabp->IrisCvtInA)(src,tbl,res)
#define	IRISCVTINW(src,tbl,res) (*irisfunctabp->IrisCvtInW)(src,tbl,res)
#define	IRISCVTOUTA(src,tbl,res) (*irisfunctabp->IrisCvtOutA)(src,tbl,res)
#define	IRISCVTOUTW(src,tbl,res) (*irisfunctabp->IrisCvtOutW)(src,tbl,res)
#define	IRISCVTEXSTRINA(src,tbl,res) (*irisfunctabp->IrisCvtExStrInA)(src,tbl,res)
#define	IRISCVTEXSTRINW(src,tbl,res) (*irisfunctabp->IrisCvtExStrInW)(src,tbl,res)
#define	IRISCVTEXSTROUTA(src,tbl,res) (*irisfunctabp->IrisCvtExStrOutA)(src,tbl,res)
#define	IRISCVTEXSTROUTW(src,tbl,res) (*irisfunctabp->IrisCvtExStrOutW)(src,tbl,res)
#define	IRISPUSHLOCKA(nlen,nptr) (*irisfunctabp->IrisPushLock)(nlen,nptr)
#define	IRISPUSHLOCKXA(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisPushLockX)(nlen,nptr,elen,eptr)
#define	IRISPUSHLOCKW(nlen,nptr) (*irisfunctabp->IrisPushLockW)(nlen,nptr)
#define	IRISPUSHLOCKXW(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisPushLockXW)(nlen,nptr,elen,eptr)
#define IRISACQUIRELOCK(nsub,flg,tout,rval) \
	(*irisfunctabp->IrisAcquireLock)(nsub,flg,tout,rval)
#define IRISRELEASELOCK(nsub,flg) (*irisfunctabp->IrisReleaseLock)(nsub,flg)
#define IRISRELEASEALLLOCKS() (*irisfunctabp->IrisReleaseAllLocks)()
#define IRISTSTART() (*irisfunctabp->IrisTStart)()
#define IRISTCOMMIT() (*irisfunctabp->IrisTCommit)()
#define IRISTROLLBACK(nlev) (*irisfunctabp->IrisTRollback)(nlev)
#define IRISTLEVEL() (*irisfunctabp->IrisTLevel)()
#define IRISSPCSEND(len,ptr) (*irisfunctabp->IrisSPCSend(len,ptr)
#define IRISSPCRECEIVE(lenp,ptr) (*irisfunctabp->IrisSPCReceive(lenp,ptr)

#if	IRIS_HSTR
#define	IRISEXECUTEH(cmd)	(*irisfunctabp->IrisExecuteH)(cmd)
#define	IRISEVALH(expr)	(*irisfunctabp->IrisEvalH)(expr)
#define	IRISPROMPTH(rbuf)	(*irisfunctabp->IrisPromptH)(rbuf)
#define	IRISERRXLATEH(code,rbuf) (*irisfunctabp->IrisErrxlateH)(code,rbuf)
#define IRISERRORH(msg,src,offp) (*irisfunctabp->IrisErrorH)(msg,src,offp)
#define	IRISCVTINH(src,tbl,res) (*irisfunctabp->IrisCvtInH)(src,tbl,res)
#define	IRISCVTOUTH(src,tbl,res) (*irisfunctabp->IrisCvtOutH)(src,tbl,res)
#define	IRISPUSHSTRH(len,ptr)	(*irisfunctabp->IrisPushStrH)(len,ptr)
#define	IRISPUSHEXSTRH(sptr)	(*irisfunctabp->IrisPushExStrH)(sptr)
#define	IRISPUSHEXSTRCVTH(sptr) (*irisfunctabp->IrisPushExStrCvtH)(sptr)
#define	IRISPUSHRTNH(rflag,tlen,tptr,nlen,nptr)			\
	(*irisfunctabp->IrisPushRtnH)(rflag,tlen,tptr,nlen,nptr)
#define	IRISPUSHRTNXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)		\
	(*irisfunctabp->IrisPushRtnXH)(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	IRISPUSHFUNCH(rflag,tlen,tptr,nlen,nptr)			\
	(*irisfunctabp->IrisPushFuncH)(rflag,tlen,tptr,nlen,nptr)
#define	IRISPUSHFUNCXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	\
	(*irisfunctabp->IrisPushFuncXH)(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	IRISPUSHGLOBALH(nlen,nptr) (*irisfunctabp->IrisPushGlobalH)(nlen,nptr)
#define	IRISPUSHGLOBALXH(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisPushGlobalXH)(nlen,nptr,elen,eptr)
#define	IRISPUSHCLASSMETHODH(clen,cptr,mlen,mptr,flg)			\
	(*irisfunctabp->IrisPushClassMethodH)(clen,cptr,mlen,mptr,flg)
#define	IRISPUSHMETHODH(oref,mlen,mptr,flg)				\
	(*irisfunctabp->IrisPushMethodH)(oref,mlen,mptr,flg)
#define	IRISPUSHPROPERTYH(oref,plen,pptr)				\
	(*irisfunctabp->IrisPushPropertyH)(oref,plen,pptr)
#define IRISPOPSTRH(lenp,strp)	(*irisfunctabp->IrisPopStrH)(lenp,strp)
#define	IRISPOPEXSTRH(sstrp)(*irisfunctabp->IrisPopExStrH)(sstrp)
#define	IRISPOPEXSTRCVTH(sstrp)(*irisfunctabp->IrisPopExStrCvtH)(sstrp)
#define	IRISPUSHCVTH(len,ptr)	(*irisfunctabp->IrisPushCvtH)(len,ptr)
#define IRISPOPCVTH(lenp,strp)	(*irisfunctabp->IrisPopCvtH)(lenp,strp)
#define	IRISPUSHLOCKH(nlen,nptr) (*irisfunctabp->IrisPushLockH)(nlen,nptr)
#define	IRISPUSHLOCKXH(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisPushLockXH)(nlen,nptr,elen,eptr)
#define	IRISCVTEXSTRINH(src,tbl,res) (*irisfunctabp->IrisCvtExStrInH)(src,tbl,res)
#define	IRISCVTEXSTROUTH(src,tbl,res) (*irisfunctabp->IrisCvtExStrOutH)(src,tbl,res)
#define IRISPUSHEXECUTEFUNCH(len,ptr) (*irisfunctabp->IrisPushExecuteFuncH)(len,ptr)
#else	/* IRIS_HSTR */
#define	IRISEXECUTEH(cmd)	(*irisfunctabp->IrisExecuteW)(cmd)
#define	IRISEVALH(expr)	(*irisfunctabp->IrisEvalW)(expr)
#define	IRISPROMPTH(rbuf)	(*irisfunctabp->IrisPromptW)(rbuf)
#define	IRISERRXLATEH(code,rbuf) (*irisfunctabp->IrisErrxlateW)(code,rbuf)
#define IRISERRORH(msg,src,offp) (*irisfunctabp->IrisErrorW)(msg,src,offp)
#define	IRISCVTINH(src,tbl,res) (*irisfunctabp->IrisCvtInW)(src,tbl,res)
#define	IRISCVTOUTH(src,tbl,res) (*irisfunctabp->IrisCvtOutW)(src,tbl,res)
#define	IRISPUSHSTRH(len,ptr)	(*irisfunctabp->IrisPushStrW)(len,ptr)
#define	IRISPUSHEXSTRH(sptr)	(*irisfunctabp->IrisPushExStrW)(sptr)
#define	IRISPUSHEXSTRCVTH(sptr) (*irisfunctabp->IrisPushExStrCvtW)(sptr)
#define	IRISPUSHRTNH(rflag,tlen,tptr,nlen,nptr)			\
	(*irisfunctabp->IrisPushRtnW)(rflag,tlen,tptr,nlen,nptr)
#define	IRISPUSHRTNXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)		\
	(*irisfunctabp->IrisPushRtnXW)(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	IRISPUSHFUNCH(rflag,tlen,tptr,nlen,nptr)			\
	(*irisfunctabp->IrisPushFuncW)(rflag,tlen,tptr,nlen,nptr)
#define	IRISPUSHFUNCXH(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)	\
	(*irisfunctabp->IrisPushFuncXW)(rflag,tlen,tptr,off,elen,eptr,nlen,nptr)
#define	IRISPUSHGLOBALH(nlen,nptr) (*irisfunctabp->IrisPushGlobalW)(nlen,nptr)
#define	IRISPUSHGLOBALXH(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisPushGlobalXW)(nlen,nptr,elen,eptr)
#define	IRISPUSHCLASSMETHODH(clen,cptr,mlen,mptr,flg)			\
	(*irisfunctabp->IrisPushClassMethodW)(clen,cptr,mlen,mptr,flg)
#define	IRISPUSHMETHODH(oref,mlen,mptr,flg)				\
	(*irisfunctabp->IrisPushMethodW)(oref,mlen,mptr,flg)
#define	IRISPUSHPROPERTYH(oref,plen,pptr)				\
	(*irisfunctabp->IrisPushPropertyW)(oref,plen,pptr)
#define IRISPOPSTRH(lenp,strp)	(*irisfunctabp->IrisPopStrW)(lenp,strp)
#define IRISPOPEXSTRH(sstrp)	(*irisfunctabp->IrisPopExStrW)(sstrp)
#define IRISPOPEXSTRCVTH(sstrp) (*irisfunctabp->IrisPopExStrW)(sstrp)
#define	IRISPUSHCVTH(len,ptr)	(*irisfunctabp->IrisPushCvtW)(len,ptr)
#define IRISPOPCVTH(lenp,strp)	(*irisfunctabp->IrisPopCvtW)(lenp,strp)
#define	IRISPUSHLOCKH(nlen,nptr) (*irisfunctabp->IrisPushLockW)(nlen,nptr)
#define	IRISPUSHLOCKXH(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisPushLockXW)(nlen,nptr,elen,eptr)
#define	IRISCVTEXSTRINH(src,tbl,res) (*irisfunctabp->IrisCvtExStrInW)(src,tbl,res)
#define	IRISCVTEXSTROUTH(src,tbl,res) (*irisfunctabp->IrisCvtExStrOutW)(src,tbl,res)
#define IRISPUSHEXECUTEFUNCH(len,ptr) (*irisfunctabp->IrisPushExecuteFuncW)(len,ptr)
#endif	/* IRIS_HSTR */

#define	IRISEXSTRKILL(obj) (*irisfunctabp->IrisExStrKill)(obj)
#define	IRISEXSTRNEWA(zstr,size) (*irisfunctabp->IrisExStrNew)(zstr,size)
#define	IRISEXSTRNEWW(zstr,size) (*irisfunctabp->IrisExStrNewW)(zstr,size)
#define	IRISEXSTRNEWH(zstr,size) (*irisfunctabp->IrisExStrNewH)(zstr,size)
#define	IRISPUSHEXLIST(sstr) (*irisfunctabp->IrisPushExList)(sstr)
#define IRISPOPEXLIST(sstr) (*irisfunctabp->IrisPopExList)(sstr)
#define	IRISPUSHEXSTRCVTW(sstrp)	(*irisfunctabp->IrisPushExStrCvtW)(sstrp)
#define IRISPOPEXSTRCVTW(sstrp)	(*irisfunctabp->IrisPopExStrCvtW)(sstrp)
#define	IRISPUSHSSVNA(nlen,nptr) (*irisfunctabp->IrisPushSSVN)(nlen,nptr)
#define	IRISPUSHSSVNXA(nlen,nptr,elen,eptr)				\
		(*irisfunctabp->IrisPushSSVNX)(nlen,nptr,elen,eptr)
#define	IRISPUSHSSVNW(nlen,nptr) (*irisfunctabp->IrisPushSSVNW)(nlen,nptr)
#define	IRISPUSHSSVNXW(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisPushSSVNXW)(nlen,nptr,elen,eptr)
#define	IRISADDSSVNA(nlen,nptr) (*irisfunctabp->IrisAddSSVN)(nlen,nptr)
#define	IRISADDSSVNXA(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisAddSSVNX)(nlen,nptr,elen,eptr)
#define	IRISADDSSVNW(nlen,nptr) (*irisfunctabp->IrisAddSSVNW)(nlen,nptr)
#define	IRISADDSSVNXW(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisAddSSVNXW)(nlen,nptr,elen,eptr)

#ifdef	IRIS_HSTR
#define	IRISPUSHSSVNH(nlen,nptr) (*irisfunctabp->IrisPushSSVNH)(nlen,nptr)
#define	IRISPUSHSSVNXH(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisPushSSVNXH)(nlen,nptr,elen,eptr)
#define	IRISADDSSVNH(nlen,nptr) (*irisfunctabp->IrisAddSSVNH)(nlen,nptr)
#define	IRISADDSSVNXH(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisAddSSVNXH)(nlen,nptr,elen,eptr)
#define	IRISADDGLOBALH(nlen,nptr) (*irisfunctabp->IrisAddGlobalH)(nlen,nptr)
#define	IRISADDGLOBALXH(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisAddGlobalXH)(nlen,nptr,elen,eptr)
#else	/* IRIS_HSTR */
#define	IRISPUSHSSVNH(nlen,nptr) (*irisfunctabp->IrisPushSSVNW)(nlen,nptr)
#define	IRISPUSHSSVNXH(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisPushSSVNXW)(nlen,nptr,elen,eptr)
#define	IRISADDSSVNH(nlen,nptr) (*irisfunctabp->IrisAddSSVNW)(nlen,nptr)
#define	IRISADDSSVNXH(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisAddSSVNXW)(nlen,nptr,elen,eptr)
#define	IRISADDGLOBALH(nlen,nptr) (*irisfunctabp->IrisAddGlobalW)(nlen,nptr)
#define	IRISADDGLOBALXH(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisAddGlobalXW)(nlen,nptr,elen,eptr)
#endif	/* IRIS_HSTR */

#define	IRISADDGLOBALA(nlen,nptr) (*irisfunctabp->IrisAddGlobal)(nlen,nptr)
#define	IRISADDGLOBALXA(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisAddGlobalX)(nlen,nptr,elen,eptr)
#define	IRISADDGLOBALW(nlen,nptr) (*irisfunctabp->IrisAddGlobalW)(nlen,nptr)
#define	IRISADDGLOBALXW(nlen,nptr,elen,eptr)				\
	(*irisfunctabp->IrisAddGlobalXW)(nlen,nptr,elen,eptr)
#define	IRISADDSSVNDESCRIPTOR(num)					\
	(*irisfunctabp->IrisAddSSVNDescriptor)(num)
#define	IRISADDGLOBALDESCRIPTOR(num)					\
	(*irisfunctabp->IrisAddGlobalDescriptor)(num)
#define	IRISADDLOCALDESCRIPTOR(num)					\
	(*irisfunctabp->IrisAddLocalDescriptor)(num)
#define IRISSSVNGET(num)	(*irisfunctabp->IrisSSVNGet)(num)
#define IRISSSVNKILL(num,val)	(*irisfunctabp->IrisSSVNKill)(num,val)
#define IRISSSVNORDER(num,val)	(*irisfunctabp->IrisSSVNOrder)(num,val)
#define IRISSSVNQUERY(num,val)	(*irisfunctabp->IrisSSVNQuery)(num,val)
#define IRISSSVNDATA(num)	(*irisfunctabp->IrisSSVNData)(num)
#define IRISMERGE()		(*irisfunctabp->IrisMerge)()
#define IRISENABLEMULTITHREAD()				\
	(*irisfunctabp->IrisEnableMultiThread)()
#define IRISIKNOWFUNCS(funcsp)		     	\
	(*irisfunctabp->IrisiKnowFuncs)(funcsp)

#endif	/* ZF_DLL */	/*==============================*/

/*	Default function definitions (ASCII/Unicode/wchar_t)
*/
#if	defined(IRIS_UNICODE)	/* Unicode character strings */
#define	IrisCvtIn		IrisCvtInW
#define	IrisCvtOut		IrisCvtOutW
#define	IrisCvtExStrIn		IrisCvtExStrInW
#define	IrisCvtExStrOut		IrisCvtExStrOutW
#define	IrisError		IrisErrorW
#define	IrisErrxlate		IrisErrxlateW
#define	IrisEval		IrisEvalW
#define	IrisExecute		IrisExecuteW
#define	IrisPrompt		IrisPromptW
#define	IrisStart		IrisStartW
#define	IrisSecureStart		IrisSecureStartW
#define	IrisChangePassword	IrisChangePasswordW
#define	IRISCVTIN		IRISCVTINW
#define	IRISCVTOUT		IRISCVTOUTW
#define	IRISCVTEXSTRIN		IRISCVTEXSTRINW
#define	IRISCVTEXSTROUT		IRISCVTEXSTROUTW
#define	IRISERROR		IRISERRORW
#define	IRISERRXLATE		IRISERRXLATEW
#define	IRISEVAL		IRISEVALW
#define	IRISEXECUTE		IRISEXECUTEW
#define	IRISPROMPT		IRISPROMPTW
#define	IRISSTART		IRISSTARTW
#define	IRISSECURESTART		IRISSECURESTARTW
#define	IRISCHANGEPASSWORD	IRISCHANGEPASSWORDH
#define IRISPOPCVT		IRISPOPCVTW
#define	IRISPUSHCVT		IRISPUSHCVTW
#define IRISPOPSTR		IRISPOPSTRW
#define	IRISPOPEXSTR		IRISPOPEXSTRW
#define	IRISPUSHCLASSMETHOD	IRISPUSHCLASSMETHODW
#define	IRISPUSHFUNC		IRISPUSHFUNCW
#define	IRISPUSHFUNCX		IRISPUSHFUNCXW
#define	IRISPUSHGLOBAL		IRISPUSHGLOBALW
#define	IRISPUSHGLOBALX		IRISPUSHGLOBALXW
#define	IRISPUSHMETHOD		IRISPUSHMETHODW
#define	IRISPUSHPROPERTY	IRISPUSHPROPERTYW
#define	IRISPUSHRTN		IRISPUSHRTNW
#define	IRISPUSHRTNX		IRISPUSHRTNXW
#define	IRISPUSHSTR		IRISPUSHSTRW
#define	IRISPUSHEXSTR		IRISPUSHEXSTRW
#define	IRISPUSHLOCK		IRISPUSHLOCKW
#define	IRISPUSHLOCKX		IRISPUSHLOCKXW
#define IRISEXSTRNEW		IRISEXSTRNEWW
#define	IRISPUSHSSVN		IRISPUSHSSVNW
#define	IRISPUSHSSVNX		IRISPUSHSSVNXW
#define	IRISADDGLOBAL		IRISADDGLOBALW
#define	IRISADDGLOBALX		IRISADDGLOBALXW
#define	IRISADDSSVN		IRISADDSSVNW
#define	IRISADDSSVNX		IRISADDSSVNXW
#define	IRISPUSHEXECUTEFUNC	IRISPUSHEXECUTEFUNCW
#elif	defined(IRIS_WCHART)	/* wchar_t character strings */
#define	IrisCvtIn		IrisCvtInH
#define	IrisCvtOut		IrisCvtOutH
#define	IrisCvtExStrIn		IrisCvtExStrInW
#define	IrisCvtExStrOut		IrisCvtExStrOutW
#define	IrisError		IrisErrorH
#define	IrisErrxlate		IrisErrxlateH
#define	IrisEval		IrisEvalH
#define	IrisExecute		IrisExecuteH
#define	IrisPrompt		IrisPromptH
#define	IrisStart		IrisStartH
#define	IrisSecureStart		IrisSecureStartH
#define	IrisChangePassword	IrisChangePasswordH
#define	IRISCVTIN		IRISCVTINH
#define	IRISCVTOUT		IRISCVTOUTH
#define	IRISCVTEXSTRIN		IRISCVTEXSTRINH
#define	IRISCVTEXSTROUT		IRISCVTEXSTROUTH
#define	IRISERROR		IRISERRORH
#define	IRISERRXLATE		IRISERRXLATEH
#define	IRISEVAL		IRISEVALH
#define	IRISEXECUTE		IRISEXECUTEH
#define	IRISPROMPT		IRISPROMPTH
#define	IRISSTART		IRISSTARTH
#define	IRISSECURESTART		IRISSECURESTARTH
#define	IRISCHANGEPASSWORD	IRISCHANGEPASSWORDH
#define IRISPOPCVT		IRISPOPCVTH
#define	IRISPUSHCVT		IRISPUSHCVTH
#define IRISPOPSTR		IRISPOPSTRH
#define	IRISPOPEXSTR		IRISPOPEXSTRH
#define	IRISPUSHCLASSMETHOD	IRISPUSHCLASSMETHODH
#define	IRISPUSHFUNC		IRISPUSHFUNCH
#define	IRISPUSHFUNCX		IRISPUSHFUNCXH
#define	IRISPUSHGLOBAL		IRISPUSHGLOBALH
#define	IRISPUSHGLOBALX		IRISPUSHGLOBALXH
#define	IRISPUSHMETHOD		IRISPUSHMETHODH
#define	IRISPUSHPROPERTY	IRISPUSHPROPERTYH
#define	IRISPUSHRTN		IRISPUSHRTNH
#define	IRISPUSHRTNX		IRISPUSHRTNXH
#define	IRISPUSHSTR		IRISPUSHSTRH
#define	IRISPUSHEXSTR		IRISPUSHEXSTRH
#define	IRISPUSHLOCK		IRISPUSHLOCKH
#define	IRISPUSHLOCKX		IRISPUSHLOCKXH
#define IRISEXSTRNEW		IRISEXSTRNEWH
#define	IRISPUSHSSVN		IRISPUSHSSVNH
#define	IRISPUSHSSVNX		IRISPUSHSSVNXH
#define	IRISADDGLOBAL		IRISADDGLOBALH
#define	IRISADDGLOBALX		IRISADDGLOBALXH
#define	IRISADDSSVN		IRISADDSSVNH
#define	IRISADDSSVNX		IRISADDSSVNXH
#define	IRISPUSHEXECUTEFUNC	IRISPUSHEXECUTEFUNCH
#else				/* ASCII character strings */
#define	IrisCvtIn		IrisCvtInA
#define	IrisCvtOut		IrisCvtOutA
#define	IrisCvtExStrIn		IrisCvtExStrInA
#define	IrisCvtExStrOut		IrisCvtExStrOutA
#define	IrisError		IrisErrorA
#define	IrisErrxlate		IrisErrxlateA
#define	IrisEval		IrisEvalA
#define	IrisExecute		IrisExecuteA
#define	IrisPrompt		IrisPromptA
#define	IrisStart		IrisStartA
#define	IrisSecureStart		IrisSecureStartA
#define	IrisChangePassword	IrisChangePasswordA
#define	IRISCVTIN		IRISCVTINA
#define	IRISCVTOUT		IRISCVTOUTA
#define	IRISCVTEXSTRIN		IRISCVTEXSTRINA
#define	IRISCVTEXSTROUT		IRISCVTEXSTROUTA
#define	IRISERROR		IRISERRORA
#define	IRISERRXLATE		IRISERRXLATEA
#define	IRISEVAL		IRISEVALA
#define	IRISEXECUTE		IRISEXECUTEA
#define	IRISPROMPT		IRISPROMPTA
#define	IRISSTART		IRISSTARTA
#define	IRISSECURESTART		IRISSECURESTARTA
#define	IRISCHANGEPASSWORD	IRISCHANGEPASSWORDA
#define IRISPOPSTR		IRISPOPSTRA
#define	IRISPOPEXSTR		IRISPOPEXSTRA
#define	IRISPUSHCLASSMETHOD	IRISPUSHCLASSMETHODA
#define	IRISPUSHFUNC		IRISPUSHFUNCA
#define	IRISPUSHFUNCX		IRISPUSHFUNCXA
#define	IRISPUSHGLOBAL		IRISPUSHGLOBALA
#define	IRISPUSHGLOBALX		IRISPUSHGLOBALXA
#define	IRISPUSHMETHOD		IRISPUSHMETHODA
#define	IRISPUSHPROPERTY	IRISPUSHPROPERTYA
#define	IRISPUSHRTN		IRISPUSHRTNA
#define	IRISPUSHRTNX		IRISPUSHRTNXA
#define	IRISPUSHSTR		IRISPUSHSTRA
#define	IRISPUSHEXSTR		IRISPUSHEXSTRA
#define	IRISPUSHLOCK		IRISPUSHLOCKA
#define	IRISPUSHLOCKX		IRISPUSHLOCKXA
#define	IRISEXSTRNEW		IRISEXSTRNEWA
#define	IRISPUSHSSVN		IRISPUSHSSVNA
#define	IRISPUSHSSVNX		IRISPUSHSSVNXA
#define	IRISADDGLOBAL		IRISADDGLOBALA
#define	IRISADDGLOBALX		IRISADDGLOBALXA
#define	IRISADDSSVN		IRISADDSSVNA
#define	IRISADDSSVNX		IRISADDSSVNXA
#define	IRISPUSHEXECUTEFUNC	IRISPUSHEXECUTEFUNCA
#endif	/* IRIS_UNICODE */

#ifdef __cplusplus
}
#endif

#endif	/* _iriscallin_h_ */
