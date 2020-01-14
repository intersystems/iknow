/*
	File: iris-cdzf.h

	Structures used for $ZF callout functions

	Copyright (c) 1986-2018 by InterSystems.
	Cambridge, Massachusetts, U.S.A.  All rights reserved.

	NOTE - because this file is included in C programs and the
	C compiler on some platforms may not support C++ style
	comments by default, the // style of comments should not
	be used here.

	Last Change: $Date$
	Changelist : $Change$
*/

#ifndef	_iriscdzf_h_
#define	_iriscdzf_h_

#ifdef	__cplusplus
extern	"C" {
#endif

/*	Flag indicating 4-byte wchar_t support */
#if	defined(_WIN32)
#define	IRIS_ZHARR	0
#else
#define	IRIS_ZHARR	1
#endif

/*
	Counted string argument descriptors
*/
typedef struct zarray {
	unsigned short	len;
	unsigned char	data[1];	/* 1 is a dummy value */
} *ZARRAYP;

typedef struct zwarray {
	unsigned short	len;
	unsigned short	data[1];	/* 1 is a dummy value */
} *ZWARRAYP;

#if	IRIS_ZHARR
#include <wchar.h>
typedef struct zharray {
	unsigned int	len;
	wchar_t		data[1];	/* 1 is a dummy value */
} *ZHARRAYP;
#else
typedef	ZWARRAYP	ZHARRAYP;
#endif	/* IRIS_ZHARR */

/*	Argument descriptor values 

        Argument descriptor characters may be either lowercase or uppercase.
        The lowercase value is used in a switch statement to distinguish
        argument types. Uppercase counterparts indicate that the argument
        is an output argument.

*/
#define	DPBYTE	  'b'	/* Counted byte array */
#define DPCHAR	  'c'	/* Zero terminated string (8 bit characters) */
#define DPDBL	  'd'	/* Double */
#define DPFLOAT	  'f'	/* Float */
#define	DCVLLIST  'g'	/* Long string $LIST object*/
#define	DCVWSTR   'h'	/* Counted wide string with translation */
#define DINT	  'i'	/* Integer */
#define	DLSTR	  'j'	/* long string (8-bit characters, counted) */
#define DPARGSTK  'k'	/* Pointer to argstk element */
#define	DCVLIST	  'l'	/* $LIST object with translation */
#define	DTLSTR	  'm'	/* translated long string (8-bit characters, counted) */
#define	DWLSTR	  'n'	/* wide long string (counted) */
#define DOREF	  'o'	/* Object reference */
#define DPINT	  'p'	/* Pointer to integer */
#define	DCVWCHR	  'q'	/* Zero terminated string with translation */
#define	DTWLSTR	  'r'	/* translated wide long string (counted) */
#define	DPCWSTR   's'	/* Counted string (16 bit characters) */
#define	DPTSTR	  't'	/* Translated string (in byte array) */
#define DPVDESC	  'v'	/* VMS fixed length string descriptor */
#define	DPWCHAR	  'w'	/* Zero terminated string (16 bit characters) */
#define DPVOID	  'y'	/* Generic void pointer */
#define DCILINK   'z'	/* Call-in linkage vector. input only. Need */
			/* to include callin.h as well */

/* The following macros work with an argument descriptor character
   that may be either upper or lower case.
*/
#define ZFARGTYPE(x)  ((x) | 0x20)	/* Coerces to lowercase */
#define ISZFOUTARG(x) !((x) & 0x20)	/* Check for absence of bit that
					   indicates uppercase character */
/* Prefix character designating IEEE format */
#define	DIEEENUM  '#'	/* See next char for real type */

/* Maximum size of $zf(-1) command. Restricted to 8191 on windows.
   Artificially restricted to this on Unix. 
*/
#define MAXCLICOMMAND 8191

/* Expanded argument descriptor prototypes:
	Since new data types are being introduced at a faster
	rate than characters are being added to the alphabet,
	the following two-character convention (similar to that
	employed for IEEE number formats) has been adopted for
	various string data types. Both prior and new syntaxes,
	where applicable, are accepted:

	prior	new	meaning
	 'b'	'1b'	counted 8-bit character array
	 'c'	'1c'	zero-terminated 8-bit character string
	 's'	'2b'	counted 16-bit character array
	 'w'	'2c'	zero-terminated 16-bit character string
	 	'4b'	counted wchar_t array
	 	'4c'	zero-terminated wchar_t string
	 	'1h'	counted 8-bit character array w/ translation*
	 	'1q'	zero-terminated 8-bit character string w/ translation*
	 'h'	'2h'	counted 16-bit character array w/ translation
	 'q'	'2q'	zero-terminated 16-bit character string w/ translation
	 	'4h'	counted wchar_t array w/ translation
	 	'4q'	zero-terminated wchar_t string w/ translation
	 'j'	'1j'	counted 8-bit character long string array
	 'n'	'2j'	counted 16-bit character long string array
	 	'4j'	counted wchar_t long string array

	The size of wide character type wchar_t is system dependent.

	* Types '1h' and '1q' are not currently implemented.
*/

typedef	int	(*zffunc)();

/*
	Table of remote routine descriptors
*/
struct zfestr {
	const char  *zfename;	/* Address of function name string */
	const char  *zfead;	/* Address of argument descriptor string */
	zffunc	     zfeep;	/* Function entry point address */
};

/*
	Function Prototypes
*/

#ifdef	_WIN32
#define	ZFCALL			__cdecl
#define	ZFEXPO			__declspec( dllexport )
#elif defined(__APPLE__) && defined(__GNUC__) && (__GNUC__ >= 4)
#define	ZFCALL			
#define	ZFEXPO		__attribute__((visibility("default")))
#else
#define	ZFCALL
#define	ZFEXPO
#endif

#if	defined(__STDC__) || defined(__cplusplus)
#define	ZFPROTOD(func,args)	ZFCALL func args
#define	ZFPROTOI(func,args)	(ZFCALL *func) args
#else
#define	ZFPROTOD(func,args)	ZFCALL func ()
#define	ZFPROTOI(func,args)	(ZFCALL *func) ()
#endif

typedef const struct IrisCallbackTab {
   int	ZFPROTOI(ZFSTRSIZE,(void));
   int	ZFPROTOI(ZFERROR,(int,int));
   int	ZFPROTOI(IRISMGRDIRA,(int,char *));
   int	ZFPROTOI(IRISMGRDIRW,(int,unsigned short *));
#if	!defined(_WIN32)	/* These are only for UNIX */
   int	ZFPROTOI(SIGRTCHK,(void));
   int	ZFPROTOI(SIGRTCLR,(void));
   int	ZFPROTOI(DZFALARM,());
#endif
   int	ZFPROTOI(ZFRESJOB,(void));
#if	IRIS_ZHARR
   int	ZFPROTOI(IRISMGRDIRH,(int,wchar_t *));
#endif	/* IRIS_ZHARR */
} IRISCBT, *IRISCBTP;

#ifndef	ZF_DLL		/*==============================*/

/*
	Direct (linked-in) Callback Function Declarations

	zfstrsize - returns max length of $ZF reply string
	zferror - used to report errors as though from within Iris
	IrisMgrDirA - obtains Iris manager's directory (narrow)
	IrisMgrDirW - obtains Iris manager's directory (wide)
	IrisMgrDirH - obtains Iris manager's directory (wchar_t)
*/
extern	int	ZFPROTOD(zfstrsize,(void));
extern	int	ZFPROTOD(zferror,(int,int));
extern	int	ZFPROTOD(zfresjob,(void));
extern	int	ZFPROTOD(IrisMgrDirA,(int,char *));
extern	int	ZFPROTOD(IrisMgrDirW,(int,unsigned short *));
#if	IRIS_ZHARR
extern	int	ZFPROTOD(IrisMgrDirH,(int,wchar_t *));
#else	/* IRIS_ZHARR */
#define IrisMgrDirH(len,namep)	IrisMgrDirW(len,namep)
#endif	/* IRIS_ZHARR */

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

/*
	Indirect Callback Function Table
*/

typedef struct IrisExportFuncTab *IRISEFTP;

extern	IRISEFTP	irisfunctabp;
extern	IRISCBTP	iriscallbackp;

/*
	Interface Function Definition
*/

#define	zfstrsize()		(*iriscallbackp->ZFSTRSIZE)()
#define	zferror(errnum,mode)	(*iriscallbackp->ZFERROR)(errnum,mode)
#define	zfresjob()		(*iriscallbackp->ZFRESJOB)()
#if	!defined(_WIN32)
#define	sigrtchk()		(*iriscallbackp->SIGRTCHK)();
#define	sigrtclr()		(*iriscallbackp->SIGRTCLR)();
#define	dzfalarm()		(*iriscallbackp->DZFALARM)();
#endif

#define IrisMgrDirA(len,namep)	(*iriscallbackp->IRISMGRDIRA)(len,namep)
#define IrisMgrDirW(len,namep)	(*iriscallbackp->IRISMGRDIRW)(len,namep)
#if	IRIS_ZHARR
#define IrisMgrDirH(len,namep)	(*iriscallbackp->IRISMGRDIRH)(len,namep)
#else	/* IRIS_ZHARR */
#define IrisMgrDirH(len,namep)	(*iriscallbackp->IRISMGRDIRW)(len,namep)
#endif	/* IRIS_ZHARR */

#endif	/* ZF_DLL */	/*==============================*/

#if	defined(IRIS_UNICODE)
#define	IrisMgrDir(len,namep)	IrisMgrDirW(len,namep)
#elif	defined(IRIS_WCHART)	/* wchar_t character strings */
#define	IrisMgrDir(len,namep)	IrisMgrDirH(len,namep)
#else
#define	IrisMgrDir(len,namep)	IrisMgrDirA(len,namep)
#endif

#ifdef	ZF_DLL
extern struct zfestr zfedll[];
#else
extern struct zfestr zfe[];
#endif

#ifdef	ZF_DLL
#ifdef	__cplusplus
#define	ZFDECL		extern "C" ZFEXPO 
#else
#define	ZFDECL		ZFEXPO
#endif

#define	ZFBEGIN				\
	IRISCBTP	iriscallbackp;	\
	IRISEFTP	irisfunctabp;	\
ZFDECL struct zfestr * ZFCALL GetZFTable(	\
	IRISCBTP	cbtp,		\
	IRISEFTP	eftp)		\
{					\
	iriscallbackp = cbtp;		\
	irisfunctabp = eftp;		\
	return zfedll;			\
}					\
struct zfestr zfedll[] = {
#else	/* ZF_DLL */
#define	ZFBEGIN	struct zfestr zfe[] = {
#endif	/* ZF_DLL */

#define	ZFENTRY(n,a,f)	{n,a,(zffunc)f},
#define	ZFPRIV		ZFENTRY
#define	ZFEND		{(char *) 0, (char *) 0, (zffunc) 0}};
#define	ZF_SUCCESS	0	/* Return code for success */
#define	ZF_FAILURE	1	/* Return code for failure */

#ifdef	__cplusplus
}
#endif

#include "iris-callin.h"

#endif	/* _iriscdzf_h_ */
