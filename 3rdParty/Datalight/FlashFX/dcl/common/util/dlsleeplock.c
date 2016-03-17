/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight,
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This module contains code which implements "SleepLock" functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlsleeplock.c $
    Revision 1.3  2011/03/10 00:26:42Z  daniel.lewis
    Added a cast to silence a compiler warning about volatile qualifiers.
    Revision 1.2  2010/09/19 14:28:30Z  garyp
    Corrected to work properly in DEBUG mode.
    Revision 1.1  2010/09/19 03:03:58Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Protected: DclSleepLock()

    Sleep while waiting for an atomic field to be updated to the
    desired target value, and optionally place a new value into
    that field.

    If ulMaxMS is non-zero, ulIntervalMS must be less than or equal
    to ulMaxMS.  Additionally, when ulMaxMS is non-zero, and if
    ulIntervalMS is larger than the maximum timer sample period
    returned by DclTimerSamplePeriod(), it will be reduced to that
    level.  In most use cases, ulIntervalMS should be significantly
    less than both values, as there is no guarantee that length of
    a DclOsSleep() call will be limited to the specified time, and
    if the actual sleep time exceeds the maximum timer sample period,
    the timer could undetectably overflow.

    Parameters:
        pulAtomicFlag - A pointer to the D_ATOMIC32 field to use.
        ulCompare     - The target value for which to wait.
        ulExchange    - The optional new value for the field.  If
                        this value is the same as ulCompare, no
                        change to the field will be made, and a
                        more optimal atomic operation will be used.
        ulMaxMS       - The maximum number of milliseconds to wait
                        before timing out.  If this value is 0, the
                        function will wait forever.
        ulIntervalMS  - The number of milliseconds to sleep between
                        each check.

    Return Value:
        Returns DCLSTAT_SUCCESS if the operation succeeds.  Returns
        DCLSTAT_SLEEPLOCK_TIMEOUT if the operation times out.  May
        return other status codes in the event of an error condition.
-------------------------------------------------------------------*/
DCLSTATUS DclSleepLock(
    D_ATOMIC32     *pulAtomicFlag,
    D_UINT32        ulCompare,
    D_UINT32        ulExchange,
    D_UINT32        ulMaxMS,
    D_UINT32        ulIntervalMS)
{
    DCLTIMER        timer;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;

    DclAssertWritePtr((void *)pulAtomicFlag, sizeof(*pulAtomicFlag));
    DclAssert(ulMaxMS == 0 || ulMaxMS >= ulIntervalMS);

    if(ulMaxMS)
    {
        D_UINT32 ulTickPeriodMS = DclTimerSamplePeriod();

        DclAssert(ulTickPeriodMS > 1);

        /*  If necessary, reduce the interval to something reasonable
            with respect to the maximum timer sample period.
        */
        if(ulIntervalMS > ulTickPeriodMS >> 1)
            ulIntervalMS = ulTickPeriodMS >> 1;

        /*  Note that we are using Timers rather than TimeStamps because
            the former can have a period of up to 49.7 days, so long as
            they are checked periodically, while TimeStamps are limited
            to modulus of the system ticker.
        */
        dclStat = DclTimerSet(&timer, ulMaxMS);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclError();
            return dclStat;
        }
    }
  #if D_DEBUG
    else
    {
        /*  Set the timer even when ulMaxMS is 0, so that the debug output
            below will function properly -- but only in DEBUG mode.
        */
        dclStat = DclTimerSet(&timer, D_UINT32_MAX);
        DclAssert(dclStat == DCLSTAT_SUCCESS);
    }
  #endif

    while(TRUE)
    {
        /*  Ensure that we are doing an optimal examination of the field
            here.  If ulCompare and ulExchange are identical, then we're
            not updating the field, so use the "AtomicRetrieve" function
            rather than the more expensive "AtomicCompareAndExchange".
        */
        if(ulCompare == ulExchange)
        {
            if(DclOsAtomic32Retrieve(pulAtomicFlag) == ulCompare)
                break;
        }
        else
        {
            if(DclOsAtomic32CompareExchange(pulAtomicFlag, ulCompare, ulExchange) == ulCompare)
                break;
        }

        if(ulMaxMS && DclTimerExpired(&timer))
        {
            dclStat = DCLSTAT_SLEEPLOCK_TIMEOUT;
            break;
        }

        DclOsSleep(ulIntervalMS);
    };

    if(dclStat == DCLSTAT_SUCCESS)
    {
        DCLPRINTF(2, ("DclSleepLock() Field=%P Cmp=%lU Xchg=%lU MaxMS=%lU IntMS=%lU returned after %lU ms\n",
            pulAtomicFlag,
            ulCompare, ulExchange, ulMaxMS, ulIntervalMS,
            DclTimerElapsed(&timer)));
    }
    else
    {
        DCLPRINTF(1, ("DclSleepLock() Field=%lU Cmp=%lU Xchg=%lU MaxMS=%lU IntMS=%lU timed out!\n",
            DclOsAtomic32Retrieve(pulAtomicFlag),
            ulCompare, ulExchange, ulMaxMS, ulIntervalMS));
    }

    return dclStat;
}
