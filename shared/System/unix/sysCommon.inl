//**********************************************************************
//* Copyright (c) 1999 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* sysCommon.inl: Implementation of sysCommon functions
//**********************************************************************
//* Change History: 
//* PJN: 1999/12/01 Initial Implementation.
//**********************************************************************

// System-specific implementations for Linux platforms
#define _I64_MAX 9223372036854775807LL
#define _I64_MIN (-_I64_MAX-1)

#define _LONG64_MAX _I64_MAX
#define _LONG64_MIN _I64_MIN

inline SDWORD sysAtomicInc(SDWORD* p_pl)
{
    return ++(*p_pl);
}

inline SDWORD sysAtomicDec(SDWORD* p_pl)
{
    return --(*p_pl);
}

inline SDWORD sysAtomicAdd(SDWORD* p_p1, SDWORD p_p2)
{
    SDWORD t_tmp = *p_p1;
    (*p_p1) += p_p2;
    return t_tmp;
}

inline SDWORD sysAtomicSwap(SDWORD* p_p1, SDWORD p_p2)
{
    SDWORD t_tmp = *p_p1;
    *p_p1=p_p2;
    return t_tmp;
}

/**
 * Handy macro for making calls to system functions
 *
 * @param callspec  system function call that we should make
 */
#define SYSTEMCALL(callspec)                                                  \
    {                                                                         \
        if ((callspec) < 0)                                                   \
        {                                                                     \
            throw CISSystemException(__LINE__, __FILE__, errno);              \
        }                                                                     \
    }

/**
 * Handy macro for making calls to pthread functions.  Note pthread functions
 * are different from system calls in that they will return 0 on success and
 * non-zero indicates an the error code (rather than -1 as system calls always
 * do).
 *
 * @param callspec  Pthread function call that we should make
 */
#define PTHREADCALL(callspec)                                                 \
    {                                                                         \
        int t__iError;                                                        \
        if (0 != (t__iError = (callspec)))                                    \
        {                                                                     \
            throw CISSystemException(__LINE__, __FILE__, t__iError);          \
        }                                                                     \
    }


#if defined(__APPLE__)

#include <sys/sysctl.h>
#include <unistd.h>

/**
 * Determine if we're running under the debugger.  See technical Q&A 1361
 *   http://devworld.apple.com/qa/qa2004/qa1361.html
 */
inline int AmIBeingDebugged()
{
    int               mib[4];
    struct kinfo_proc info;
    size_t            size;
        
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = (int)getpid();
    size = sizeof(info);
    info.kp_proc.p_flag = 0;
    sysctl(mib,4,&info,&size,NULL,0);

    return ((info.kp_proc.p_flag & P_TRACED) == P_TRACED);
}

inline void APPLE_Assert(const char* file, const int line, const char* expr)
{
    fprintf(stderr,
            "ASSERT: Assertion failed at %s:%d\n%s\n", file, line, expr);

    // This is the best way to break into the debugger.  You can step right
    // over this if you see it.
    if (AmIBeingDebugged()) pthread_kill(pthread_self(), SIGINT);
}
#endif  // __APPLE__

