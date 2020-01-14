/*
	Structures used for $ZF callout functions

	Copyright (c) 1986-2018 by InterSystems.
	Cambridge, Massachusetts, U.S.A.  All rights reserved.

	This file contains macros to support compatibility for
	modules written for Cache to be compiled for execution
	with InterSystems IRIS.

	Last Change: $Date$
	Changelist : $Change$
*/

#ifndef	_cdzf_h_
#define	_cdzf_h_

#include "iris-cdzf.h"

#ifdef	__cplusplus
extern	"C" {
#endif

/*	Flag indicating 4-byte wchar_t support */
#if	defined(_WIN32)
#define	CACHE_ZHARR	0
#else
#define	CACHE_ZHARR	1
#endif

typedef const struct CacheCallbackTab {
   int	ZFPROTOI(ZFSTRSIZE,(void));
   int	ZFPROTOI(ZFERROR,(int,int));
   int	ZFPROTOI(CACHEMGRDIRA,(int,char *));
   int	ZFPROTOI(CACHEMGRDIRW,(int,unsigned short *));
#if	!defined(_WIN32)	/* These are only for UNIX */
   int	ZFPROTOI(SIGRTCHK,(void));
   int	ZFPROTOI(SIGRTCLR,(void));
   int	ZFPROTOI(DZFALARM,());
#endif
   int	ZFPROTOI(ZFRESJOB,(void));
#if	CACHE_ZHARR
   int	ZFPROTOI(CACHEMGRDIRH,(int,wchar_t *));
#endif	/* CACHE_ZHARR */
} CACHECBT, *CACHECBTP;

#ifndef	ZF_DLL		/*==============================*/

/*
	Direct (linked-in) Callback Function Declarations

	zfstrsize - returns max length of $ZF reply string
	zferror - used to report errors as though from within Cache
	CacheMgrDirA - obtains Cache manager's directory (narrow)
	CacheMgrDirW - obtains Cache manager's directory (wide)
	CacheMgrDirH - obtains Cache manager's directory (wchar_t)
*/
extern	int	ZFPROTOD(zfstrsize,(void));
extern	int	ZFPROTOD(zferror,(int,int));
extern	int	ZFPROTOD(zfresjob,(void));
extern	int	ZFPROTOD(CacheMgrDirA,(int,char *));
extern	int	ZFPROTOD(CacheMgrDirW,(int,unsigned short *));
#if	CACHE_ZHARR
extern	int	ZFPROTOD(CacheMgrDirH,(int,wchar_t *));
#else	/* CACHE_ZHARR */
#define CacheMgrDirH(len,namep)	CacheMgrDirW(len,namep)
#endif	/* CACHE_ZHARR */
/*
	Prototypes for signal handling helper functions in $ZF
	for UNIX systems only.
*/

/*
*  sigrtchk() checks for asynchronous events.
*      Should be called whenever a system call fails because of a
*      received signal (usually open(), close(), read(), write(),
*      ioctl() and others that may take an unpredictable amount
*      of time to complete). It returns a code indicating the action
*      the user should take:
*
*     Return code   Cause                Action
*     ----------    -----------------    -------------------------
*         -1        Not a signal         Check for I/O error.
*          1        SIGINT or SIGTERM    Exit $ZF.
*          0        Other signal         Restart operation.
*/
extern	int	ZFPROTOD(sigrtchk,(void));

/*
*  sigrtclr() clears retry flag.
*      Should be called once before using sigrtchk()
*/
extern	int	ZFPROTOD(sigrtclr,(void));

/*
*  dzfalarm() establishes new SIGALRM handler.
*      On entry to $ZF, the previous handler is saved automatically.
*      On exit, it's restored automatically.
*/
extern	int	ZFPROTOD(dzfalarm,());

#else	/* ZF_DLL */	/*==============================*/

#define	CACHEEFTP	IRISEFTP
#define	cachefunctabp	irisfunctabp
#define	cachecallbackp	iriscallbackp

#endif	/* ZF_DLL */	/*==============================*/

#if	defined(CACHE_UNICODE)
#define	CacheMgrDir(len,namep)	CacheMgrDirW(len,namep)
#elif	defined(CACHE_WCHART)	/* wchar_t character strings */
#define	CacheMgrDir(len,namep)	CacheMgrDirH(len,namep)
#else
#define	CacheMgrDir(len,namep)	CacheMgrDirA(len,namep)
#endif

#define	ZFENTRY(n,a,f)	{n,a,(zffunc)f},
#define	ZFPRIV		ZFENTRY
#define	ZFEND		{(char *) 0, (char *) 0, (zffunc) 0}};
#define	ZF_SUCCESS	0	/* Return code for success */
#define	ZF_FAILURE	1	/* Return code for failure */

#ifdef	__cplusplus
}
#endif

#include "callin.h"

#endif	/* _cdzf_h_ */
