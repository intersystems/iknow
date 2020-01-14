//**********************************************************************
//* Copyright (c) 1999 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* sysCommon.inl: Implementation of sysCommon functions
//**********************************************************************
//* Change History:
//* DaveM: 2004/05/20 Added POSIX style condition variables
//* PJN: 1999/12/01 Initial Implementation.
//**********************************************************************

// System-specific implementations for Microsoft Windows NT (INTEL)
#include "limits.h"
#include <windows.h>

#undef NODEFAULT
#undef M_ASSERT

#ifdef M_DEBUG
#define NODEFAULT M_ASSERT(0)
#define M_ASSERT(x) assert(x)
#else
#define NODEFAULT __assume(0)
#define M_ASSERT(x)
// - we no longer define M_ASSERT(x) as __assume(x) because it is not safe to allow __assume(0) generally
//   - but NODEFAULT in "unreachable" switch branches is ok
#endif

// Wide Characters are 2 bytes on this system
#ifdef _DOUBLEBYTE
#define MCHARWIDTH 2
#define CHAR_WIDTH 2
#else
#define MCHARWIDTH 1
#define CHAR_WIDTH 1
#endif

#define _LONG64_MAX _I64_MAX
#define _LONG64_MIN _I64_MIN

// Atomic Lock implementations
inline SDWORD sysAtomicInc(SDWORD* p_pl)
{
	return ::InterlockedIncrement(p_pl);
}

inline SDWORD sysAtomicDec(SDWORD* p_pl)
{
	return ::InterlockedDecrement(p_pl);
}

inline SDWORD sysAtomicAdd(SDWORD* p_p1, SDWORD p_p2)
{
	return ::InterlockedExchangeAdd(p_p1, p_p2);
}

inline SDWORD sysAtomicSwap(SDWORD* p_p1, SDWORD p_p2)
{
	return ::InterlockedExchange(p_p1, p_p2);
}

/**
 * The <code>CPosixConditionVariable</code> class contains a Win32
 * implementation of POSIX style conditional variables.  This was
 * adapted from:
 *
 *     http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
 */
class CPosixConditionVariable
{
public:
    CPosixConditionVariable();
    ~CPosixConditionVariable();

public:
    void Initialize();
    void Destroy();
    void Notify(bool p_bAll = false);
    bool TryWait(sysTypes::CRITICALSECTION& p_rcsSection,
                 DWORD                      p_dwTimeout);

protected:
    // Internal state
    enum
    {
        SIGNAL        = 0,
        BROADCAST     = 1,

        __MAX_EVENTS  = 2
    };

    bool          m_bInitialized;             //< Initialized flag
    HANDLE        m_rgEvents[__MAX_EVENTS];   //< Array of events}
    volatile LONG m_nWaiters;                 //< Number of waiting threads
};

/**
 * Constructor for a condition variable
 */
inline CPosixConditionVariable::CPosixConditionVariable()
    : m_bInitialized(false),
      m_nWaiters(0)
{
    memset(reinterpret_cast<void*>(&m_rgEvents), 0, sizeof(m_rgEvents));
}

/**
 * Destructor for a condition variable
 */
inline CPosixConditionVariable::~CPosixConditionVariable()
{
    M_ASSERT(!m_bInitialized);
    M_ASSERT(0 == m_nWaiters);
    M_ASSERT(NULL == m_rgEvents[SIGNAL]);
    M_ASSERT(NULL == m_rgEvents[BROADCAST]);
}

/**
 * Notify anyone waiting on a condition variable
 *
 * @param p_bAll  If true, wake all waiters
 */
inline void CPosixConditionVariable::Notify(bool p_bAll)
{
	M_ASSERT(true == m_bInitialized);

	if ((m_nWaiters > 0) || (!p_bAll))
    {
		::SetEvent(m_rgEvents[p_bAll ? BROADCAST : SIGNAL]);
	}
}

/**
 * Create and initialize a condition variable
 */
inline void CPosixConditionVariable::Initialize()
{
    M_ASSERT(!m_bInitialized);

	m_nWaiters = 0;
	m_rgEvents[SIGNAL]    = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_rgEvents[BROADCAST] = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_bInitialized = true;
}

/**
 * Destroy a condition variable
 */
inline void CPosixConditionVariable::Destroy()
{
    M_ASSERT(true == m_bInitialized);

	m_bInitialized = false;
	::CloseHandle(m_rgEvents[SIGNAL]);
	::CloseHandle(m_rgEvents[BROADCAST]);

    m_rgEvents[SIGNAL]    = NULL;
    m_rgEvents[BROADCAST] = NULL;
}

/**
 * Wait for a condition variable, without blocking if it's not available.
 * You must have acquired the critical section before calling this.
 *
 * This is based on the a Win32 implementation of POSIX condition
 * variables:
 *
 *     http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
 *
 *
 * @param p_rcsSection   Associated critical section
 * @param p_dwTimeout    Timeout to wait
 * @return False if the variable was unavailable, True if it was
 */
inline bool CPosixConditionVariable::TryWait(sysCriticalSection& p_rcsSection,
                                             DWORD                p_dwTimeout)
{
    bool  t_bSuccess = false;
	DWORD t_dw       = WAIT_FAILED;


    M_ASSERT(m_bInitialized);

	// Increment the waiting thread count
	sysAtomicInc((SDWORD*)(&m_nWaiters));

    // Wait for either the BROADCAST or SIGNAL events to be set
	try
    {
        // Release the critical section
        ::LeaveCriticalSection(&p_rcsSection);

        // Reset the current error
        ::SetLastError(ERROR_SUCCESS);
        
        // Go ahead and wait, note that this will block until either one of
        // the events is set.
		t_dw = ::WaitForMultipleObjects(CPosixConditionVariable::__MAX_EVENTS,
                                        m_rgEvents,
                                        FALSE,
                                        p_dwTimeout);
	}
    catch (...)
    {
        // We must clean up since we failed ...
		sysAtomicDec((SDWORD*)&m_nWaiters);

		// Re-get the critical section (this may block)
		::EnterCriticalSection(&p_rcsSection);


		throw;
	}

    // Grab the error
    // DWORD t_dwError = ::GetLastError();

    // For a broadcast wakeup, if we're the last thread out then the event
    // needs to be reset.
	if (0 == sysAtomicDec((SDWORD*)&m_nWaiters))
    {
        // Check for broadcast ...
		if ((WAIT_OBJECT_0 + BROADCAST) == t_dw)
        {
			// We were the last waiting thread, reset the broadcast event
			::ResetEvent(m_rgEvents[BROADCAST]);
		}
	}

    // We need to get the critical section again (may block here)
    ::EnterCriticalSection(&p_rcsSection);

	switch (t_dw)
    {
    case WAIT_OBJECT_0 + SIGNAL:
    case WAIT_OBJECT_0 + BROADCAST:
        // If we were woken up via a signal or a broadcast, then we should
        // return success.
        t_bSuccess = true;
        break;

    case WAIT_TIMEOUT:
    case WAIT_ABANDONED:
        // We timed out or the wait was abandoned
        t_bSuccess = false;
        break;

	case WAIT_FAILED:
    default:
        // Error (in t_dwError)
        break;
	}


    return t_bSuccess;
}


/*
 * End-of-file
 *
 */
