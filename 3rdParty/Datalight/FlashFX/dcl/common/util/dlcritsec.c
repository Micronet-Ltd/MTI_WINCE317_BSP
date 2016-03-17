/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

  In addition to civil penalties for infringement of copyright under appli-
  cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
  of (a) the restrictions on circumvention of copyright protection systems
  found in 17 U.S.C. 1201 and (b) the protections for the integrity of
  copyright management information found in 17 U.S.C. 1202.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
  CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
  IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
  OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
  BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
  PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.

  IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
  CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
  SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This module contains a synchronization mechanism for managing critical
    sections.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlcritsec.c $
    Revision 1.2  2009/12/11 20:27:51Z  garyp
    Updated DclCriticalSectionEnter() to add a warning message in the event
    that ulTimeoutMS is zero and fSleepOK is FALSE.  Docs updated.
    Revision 1.1  2009/11/24 16:45:14Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#define SLEEPTHRESHOLD      (50)
#define WARNTHRESHOLD     (1000)


/*-------------------------------------------------------------------
    Public: DclCriticalSectionEnter()

    Enter a critical section.  Code must not attempt to recursively
    enter the section.

    The value pointed to by pulAtomicSectionGate must be initialized
    to zero prior to calling this function, and thereafter should be
    considered to be private to the critical section functions.

    *Note* -- Critical sections are for serializing access to a
              section of code, and are typically used to facilitate
              atomically updating a group of variables.  Critical 
              sections are designed to be held for very short periods
              of time.  Generally software should *not* be calling out
              to other functions while executing inside a critical 
              section -- a mutex is more appropriate for that.

    *Note* -- Generally this function should not be used with both a
              ulTimeoutMS value of 0 (forever) and a fSleepOK value
              of FALSE, as a deadlock could result if the critical
              section requestor ends up blocked by a lower priority
              thread.  If the OS thread scheduler is not time-slice
              based, the higher priority blocked thread may end up 
              spinning forever.

    Parameters:
        pulAtomicSectionGate - A pointer to a D_ATOMIC32 variable
                               which controls access to the section.
        ulTimeoutMS          - The maximum number of milliseconds to
                               wait, or 0 to wait forever.
        fSleepOK             - TRUE if it is OK to sleep while waiting.

    Return Value:
        Returns TRUE if the section was entered, or FALSE if the
        operation timed out.
-------------------------------------------------------------------*/
D_BOOL DclCriticalSectionEnter(
    D_ATOMIC32     *pulAtomicSectionGate,
    D_UINT32        ulTimeoutMS,
    D_BOOL          fSleepOK)
{
    D_BOOL          fSuccess = TRUE;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_CRITSEC, 1, 0),
        "Entering critical section %P Timeout=%lU ms Sleep=%u\n", 
        pulAtomicSectionGate, ulTimeoutMS, fSleepOK));

    DclProfilerEnter("DclCriticalSectionEnter", 0, 0);

    DclAssert(pulAtomicSectionGate);

    /*  Note that the following is NOT a DEBUG-only message.  If you
        really want to do this and not see this message, set ulTimeoutMS
        to 0xFFFFFFFF, which is still a REALLY LONG TIME (however, this
        is still not recommended -- per the notes above...)
    */
    if(!ulTimeoutMS && !fSleepOK)
        DclPrintf("WARNING! Deadlock potential in DclCriticalSectionEnter()\n");

  #if DCL_OSFEATURE_THREADS
    {
        DCLTIMER        timer;
        unsigned        nRetryCount = 0;
        
        if(ulTimeoutMS)
            DclTimerSet(&timer, ulTimeoutMS);

        while(TRUE)
        {
            if(DclOsAtomic32SectionEnter(pulAtomicSectionGate))
                break;

            nRetryCount++;      /* (could wrap -- don't care) */
            
          #if D_DEBUG
            if(nRetryCount % WARNTHRESHOLD == 0)
                DCLPRINTF(1, ("DclCriticalSectionEnter() trying to enter critical section %P, retry count at %u\n", pulAtomicSectionGate, nRetryCount));
          #endif        

            /*  If sleeping is OK, sleep for 1 ms for every SLEEPTHRESHOLD 
                times we try to enter the section.
            */            
            if(fSleepOK && (nRetryCount % SLEEPTHRESHOLD == 0))
            {
                DclOsSleep(1);
                continue;
            }
            
            if(ulTimeoutMS && DclTimerExpired(&timer))
            {
                DCLPRINTF(1, ("DclCriticalSectionEnter() timed out trying to enter critical section %P\n", pulAtomicSectionGate));
                fSuccess = FALSE;
                break;
            }
        };

        DCLTRACEPRINTF((
            MAKETRACEFLAGS((!fSuccess || nRetryCount > SLEEPTHRESHOLD) ? DCLTRACE_ALWAYS : DCLTRACE_CRITSEC, 1, 0),
            "Entering critical section %P returning %u (retried %u times)\n", 
            pulAtomicSectionGate, fSuccess, nRetryCount));
    }
  
  #else
  
    *pulAtomicSectionGate++;
    DclAssert(*pulAtomicSectionGate == 1);
    
  #endif
  
    DclProfilerLeave(0);

    return fSuccess;
}


/*-------------------------------------------------------------------
    Public: DclCriticalSectionLeave()

    Leave a critical section.

    Parameters:
        pulAtomicSectionGate - A pointer to a D_ATOMIC32 variable
                               which controls access to the section.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclCriticalSectionLeave(
    D_ATOMIC32     *pulAtomicSectionGate)
{
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_CRITSEC, 1, 0),
        "Leaving critical section %P\n", pulAtomicSectionGate));

    DclProfilerEnter("DclCriticalSectionLeave", 0, 0);

    DclAssert(pulAtomicSectionGate);

  #if DCL_OSFEATURE_THREADS
  
    DclOsAtomic32SectionLeave(pulAtomicSectionGate);

  #else
  
    *pulAtomicSectionGate--;
    DclAssert(*pulAtomicSectionGate == 0);

  #endif

    DclProfilerLeave(0);

    return;
}


