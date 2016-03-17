// All rights reserved ADENEO EMBEDDED 2010
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/*
 * Enhanced Critical Section API
 *
 * This module defines four functions that parallel the Win32 critical section
 * APIs but that provide some additional functionality.
 *
 * void InitCritSec_Ex (CritSec_Ex *this)
 *      Call this to initialize an enhanced critical section structure.
 *      The caller is responsible for allocating the structure. No other
 *      routines should be called on the structure until it has been
 *      initialized by this function. Do not initialize a structure more
 *      than once (excepting when it has been de-initialized first).
 *      Failure to abide by these rules will yield unpredicatable chaos.
 *
 * void DeleteCritSec_Ex (CritSec_Ex *this)
 *      Deletes the enhanced critical section structure (but doesn't free
 *      it - caller is responsible for that), deallocating all internally
 *      used system resources. Once this call returns, no threads are allowed
 *      to pass the structure to any of these calls except Init.
 *      !!!!!
 *      This call will block until the critical section can be safely deleted.
 *      !!!!!
 *      Once this call is made, all further attempts to Enter the critical
 *      section will fail. All pending Enter requests will be woken from their
 *      blocked states and be forced to fail. If another thread already owns
 *      the critical section then this function will block until that thread
 *      relinquishes control.
 *      The calling thread may or may not own the critical section when it
 *      calls this function. If it does own the critical section, it must
 *      not call Leave once the Delete operation returns, no matter how many
 *      nested Enter calls there were.
 *      If the caller of this function did not already hold the critical
 *      section for some other reason, it is more efficient for it to simply
 *      call Delete than for it to Enter the critical section first. This is
 *      because Delete forces other competing threads to abort whereas with
 *      Enter, a thread must wait its proper turn before initiating the Delete.
 *
 * void PrepareDeleteCritSec_Ex (CritSec_Ex *this)
 *      Initiates the destruction sequence. Using this call is optional.
 *      If used, it should be called only once and must then be followed
 *      up with a call to DeleteCritSec_Ex. This call has a bounded duration
 *      and when it returns other threads will behave as described for
 *      DeleteCritSec_Ex above. It does not wait for any such threads to finish,
 *      however, and so also does not clean up internal resources. That's why
 *      this must be followed up with a call to DeleteCritSec_Ex. Use this call
 *      when the delete operation has to be split into two phases.
 *
 * CritSec_Status EnterCritSec_Ex (CritSec_Ex *this, ULONG ulTimeout)
 *      Attempts to enter the enhanced critical section. The second argument,
 *      ulTimeout, specifies a maximum time (in milliseconds) to wait (as per
 *      the Win32 call WaitForSingleObject). Like Win32 critical sections,
 *      enhanced critical sections may be Enter'd recursively, in which case,
 *      the critical section is not relinquished until Leave has been called
 *      once for each time Enter was called.
 *      Possible return values are:
 *      o CSS_SUCCESS - caller now owns the enhanced critical section.
 *      o CSS_TIMEOUT - the non-INFINITE timeout expired before we gained
 *                      ownership; caller does not own the enhanced critical
 *                      section but may try to Enter it again.
 *      o CSS_DESTROYED - someone has Delete'd the enhanced critical section
 *                      and, as of the time at which this value is first
 *                      returned, it is no longer usable.
 *
 * void LeaveCritSec_Ex (CritSec_Ex *this)
 *      Relinquishes a previously Enter'd enhanced critical section.
 *      This call CAN block and CAN cause the scheduler to run. While the
 *      Win32 critical section it attempts to acquire can only be held for a
 *      bounded period of time, there is no bound to the time we may have to
 *      wait before being granted access to it.
 *      See also notes for Enter and Delete.
 *
 * NOTES:
 * o Using ulTimeout values other than 0 or INFINITE in the Enter call may have
 *   somewhat misleading results due to the implementation. Since it is possible
 *   for a thread to be woken but not be able to acquire the critical section
 *   (because the Win32 API does not give us a way to atomically grab a Win32
 *   critical section while waking up), we can wake up and block one or more
 *   times during the timeout interval, losing the race each time and, depending
 *   upon scheduler details, possibly reset any kernel fairness indications each
 *   time and cause us to timeout even though the section became available
 *   sometime during our interval. For the same reasons, an INFINITE timeout
 *   also leaves us vulnerable to starvation despite any in-kernel remedies.
 *
 * o Since the caller of these functions allocates the memory for the enhanced
 *   critical section structure (though not the internally used resources), we
 *   can guarantee that any number of attempted Enter's after a Delete will
 *   return CSS_DESTROYED, until the caller actually frees said memory. This is
 *   a potentially useful feature in that it prevents the need for an additional
 *   external flag while waiting for a module's auxilliary threads to finish
 *   shutting down.
 */

#pragma warning(push)
#pragma warning(disable : 4510 4512 4610)
#include <globals.hpp>
#include "Sync.hpp"
#pragma warning(pop)

/* - copied from globals.hpp:
typedef enum e_CritSec_Status {
    CSS_SUCCESS, CSS_DESTROYED, CSS_TIMEOUT
} CritSec_Status;

typedef struct s_CritSec_Ex
{
    DWORD m_hOwnerThread;
    UINT m_cOwnerReferences;
    UINT m_cWaitingThreads;
    HANDLE m_hev;
    CRITICAL_SECTION m_cs;
    BOOL m_fClosing;
} CritSec_Ex;

void InitCritSec_Ex (CritSec_Ex *this);
void DeleteCritSec_Ex (CritSec_Ex *this);
CritSec_Status EnterCritSec_Ex (CritSec_Ex *this, ULONG ulTimeout);
void LeaveCritSec_Ex (CritSec_Ex *this);
*/

CritSec_Status CritSec_Ex::EnterCritSec_Ex (ULONG ulTimeout)
{
    DWORD r;
    ULONG tStart, tNow, tLeft;
    CritSec_Status retval = CSS_SUCCESS;
    BOOL fWaiting;
    DWORD me;

    tStart = GetTickCount();
    r = ! WAIT_TIMEOUT;  // anything other than WAIT_TIMEOUT will suffice
    me = GetCurrentThreadId();

    // Help destruction proceed more quickly by preventing anyone new from using
    // the IPC objects when we're pending destruction. Do this here for speed
    // and again within the critical section for correctness.
    if (m_fClosing)
        return CSS_DESTROYED;

    EnterCriticalSection(&m_cs);
    do {
        fWaiting = FALSE;

        if (m_fClosing) {
            SetEvent(m_hev);
            retval = CSS_DESTROYED;
        }
        else if (r == WAIT_TIMEOUT) {
            retval = CSS_TIMEOUT;
        }
        else if (m_hOwnerThread == 0) {
            m_hOwnerThread = me;
            m_cOwnerReferences = 1;
            retval = CSS_SUCCESS;
        }
        else if (m_hOwnerThread == me) {
            ++m_cOwnerReferences;
            retval = CSS_SUCCESS;
        }
        else {
            // Oh well, we've got to wait.
            ++m_cWaitingThreads;
            fWaiting = TRUE;
        }
        LeaveCriticalSection(&m_cs);

        if (fWaiting) {
            if (ulTimeout != INFINITE) {
                tNow = GetTickCount();
                if (tNow - tStart < ulTimeout)
                    tLeft = ulTimeout - (tNow - tStart);
                else
                    tLeft = 0; // poll one more time
            } else {
                tLeft = INFINITE;
            }
            r = WaitForSingleObject(m_hev, tLeft);
            EnterCriticalSection(&m_cs);
            --m_cWaitingThreads;
        }
    } while (fWaiting);

    return retval;
}

void CritSec_Ex::LeaveCritSec_Ex ()
{
#ifdef POLITE // but not by default // default is to fail so the debugger knows you botched
    if (m_fClosing && m_hOwnerThread == NULL)
        // our caller violated protocol but we'll ignore it
        return;
#endif
    ASSERT(m_hOwnerThread == GetCurrentThreadId());
    
    EnterCriticalSection(&m_cs);

    // this would be symptomatic of a logic error in the caller
    ASSERT(m_cOwnerReferences > 0);

    if (--m_cOwnerReferences == 0) {
        m_hOwnerThread = 0;
        SetEvent(m_hev);
    }

    LeaveCriticalSection(&m_cs);
}

CritSec_Ex::CritSec_Ex()
{
    m_hev = CreateEvent(NULL, FALSE, TRUE, 0);   // initially set!
    if (m_hev == NULL) {
        // simulate InitializeCriticalSection - see docs
        RaiseException((DWORD)STATUS_NO_MEMORY, 0, 0, NULL);
    }
    InitializeCriticalSection(&m_cs);
    Initialize();
}
void CritSec_Ex::Initialize( )
{
    EnterCriticalSection(&m_cs);
    m_hOwnerThread = 0;
    m_cOwnerReferences = 0;
    m_cWaitingThreads = 0;
    m_fClosing = FALSE;
    LeaveCriticalSection(&m_cs);
}

void CritSec_Ex::PrepareDeleteCritSec_Ex ()
{
    DWORD me = GetCurrentThreadId();
    
    EnterCriticalSection(&m_cs);
    m_fClosing = TRUE;
    if (m_hOwnerThread == me) {
        // m_cOwnerReferences is >=1 but the caller had better not Leave after this...
        m_hOwnerThread = 0;
    }
    LeaveCriticalSection(&m_cs);

    // start waking up threads blocked on this critsec;
    // each thread woken this way will set the event again.
    SetEvent(m_hev);
}

CritSec_Ex::~CritSec_Ex()
{
    BOOL bDone;

    // In case it wasn't already done:
    PrepareDeleteCritSec_Ex();

    EnterCriticalSection(&m_cs);
    bDone = m_hOwnerThread == 0 && m_cWaitingThreads == 0;
    LeaveCriticalSection(&m_cs);

    while (!bDone) {
        // force someone to wake up, leaving the event reset
        // so that the next statement can actually block.
        PulseEvent(m_hev);

        // and wait for the next waiting thread to set the event again
        // which will happen when they relinquish or when they note the
        // m_fClosing flag.
        WaitForSingleObject(m_hev, INFINITE);

        EnterCriticalSection(&m_cs);
        bDone = m_hOwnerThread == 0 && m_cWaitingThreads == 0;
        LeaveCriticalSection(&m_cs);
    }

    CloseHandle(m_hev);
    DeleteCriticalSection(&m_cs);
}


Countdown::Countdown( DWORD cInitial)
{
    count = cInitial;
    hev = CreateEvent(NULL, TRUE, cInitial ? FALSE : TRUE, NULL);
    InitializeCriticalSection(&cs);
    lock = FALSE;
}

BOOL Countdown::IncrCountdown ()
{
    BOOL r = TRUE;

    EnterCriticalSection(&cs);
    if (lock)
        r = FALSE;
    else
        if (count++ == 0)
            ResetEvent(hev);
    LeaveCriticalSection(&cs);

    return r;
}

void Countdown::DecrCountdown ()
{
    EnterCriticalSection(&cs);
    ASSERT(count > 0);
    if (--count == 0)
        SetEvent(hev);
    LeaveCriticalSection(&cs);
}

void Countdown::LockCountdown ()
{
    EnterCriticalSection(&cs);
    lock = TRUE;
    LeaveCriticalSection(&cs);
}


void Countdown::WaitForCountdown (BOOL keepLocked)
{
    LockCountdown();
    
    WaitForSingleObject(hev, INFINITE);
    ASSERT(count == 0);

    lock = keepLocked;
}

Countdown::~Countdown()
{
    WaitForCountdown( TRUE);

    CloseHandle(hev);
    DeleteCriticalSection(&cs);
}


