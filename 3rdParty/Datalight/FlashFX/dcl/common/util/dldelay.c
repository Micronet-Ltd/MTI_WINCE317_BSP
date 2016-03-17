/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

    This module implements delay functions at the nanosecond resolution.
    These functionality is only useable when high resolution timers are
    available.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dldelay.c $
    Revision 1.9  2010/07/31 19:47:15Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.8  2010/04/21 17:31:13Z  garyp
    Updated DclNanosecondDelay() to use the system ticker when dealing
    with long delays to better accommodate hardware where the HighRes
    ticker gets horribly skewed when queried repeatedly for very long
    periods.
    Revision 1.7  2010/04/17 21:43:54Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.6  2009/07/07 17:50:12Z  keithg
    Corrected return error code to use generic out of range value.
    Revision 1.5  2008/05/03 19:57:53Z  garyp
    Documentation fixes.
    Revision 1.4  2008/04/30 19:42:58Z  billr
    Convert picoseconds to nanoseconds.
    Revision 1.3  2007/12/18 04:51:50Z  brandont
    Updated function headers.
    Revision 1.2  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/10/15 18:20:48Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#define ACCURACY_SCALE                     (8)  /* Scale factor to get ~25% accuracy */
#define MAXIMUM_TICK_RESOLUTION  (100000000UL)  /* 100 us per tick maximum */
#define PICOS_PER_NANO                (1000UL)

/*  Ticker characteristics
*/
static D_UINT32 ulSystemTickNS;         /* Nanoseconds per standard system tick */
static D_UINT64 ullHRTickModulus;       /* HiRes range of tick values, 0 if it wraps */
static D_UINT32 ulHRTickResolution;     /* HiRes picoseconds per tick */
static D_UINT32 ulMinAllowableDelayNS;  /* Min delay accurate to within 25% */

static DCLSTATUS Initialize(void);


/*-------------------------------------------------------------------
    Public: DclNanosecondDelay()

    Delay until the specified number of nanoseconds have elapsed.
    Note that while the interval is specified in nanoseconds, many
    current high resolution timers are only going to be accurate
    in the microsecond range.

    This function will return failure if the interface cannot
    support at least a 100 microsecond resolution.

    Parameters:
        ulNanoSeconds - Number of nanoseconds to delay, which must
                        be greater than or equal to the value
                        returned by DclNanosecondDelayMinimum().

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclNanosecondDelay(
    D_UINT32        ulNanoseconds)
{
    D_UINT64        ullStartTick;
    D_UINT64        ullDelta;
    D_UINT64        ullTicks;
    static D_BOOL   fInited = FALSE;

    /*  Get the current tick right away.  All of the other processing
        in this function counts as part of the delay.
    */
    ullStartTick = DclOsHighResTickCount();

    /*  Initialize the ticker characteristics on the first call.
    */
    if(!fInited)
    {
        DCLSTATUS dclStat = Initialize();
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;

        fInited = TRUE;
    }

    /*  Otherwise we ensure that we should be able to satisfy
        the request to within ~25% accuracy.
    */
    if(ulNanoseconds < ulMinAllowableDelayNS)
        return DCLSTAT_OUTOFRANGE;

    /*  If the requested delay time is > 20x the length of a system tick
        just use a standard timer delay rather than the high res timer.
        Some hardware/BSP implementations (TX4938 for example) end up
        with time being horribly skewed if the high-res ticker is queried
        for long periods of time in a tight loop.  The problem does not
        happen when the standard system tick is used, which is what the
        DCL "Timer" abstraction uses.  The choice of '20' is somewhat
        arbitrary.
    */  
    if(ulNanoseconds > (ulSystemTickNS * 20))
    {
        DCLSTATUS   dclStat;
        DCLTIMER    timer;

        dclStat = DclTimerSet(&timer, ulNanoseconds / 1000000);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            while(!DclTimerExpired(&timer));
        }
        
        return dclStat;        
    }

    /*  Calculate the number of HighRes ticks to delay:

                 (nanos to delay * PICOS_PER_NANO) + (picos per tick - 1)
        ticks =  --------------------------------------------------------  + 1
                                     picos per tick

        An extra tick is added because the first tick encountered will only
        be a partial tick.  The Initialize() function has ensured that the
        ulHRTickResolution value is in a range which will not cause these
        calculations to overflow.
    */
    ullTicks = ulNanoseconds;
    DclUint64MulUint32(&ullTicks, PICOS_PER_NANO);
    ullTicks += (ulHRTickResolution - 1);
    DclUint64DivUint32(&ullTicks, ulHRTickResolution);
    ullTicks++;

    DclAssert(ullTicks > 0);

    /*  Accumulate elapsed ticks until done, counting down the number
        just calculated.  Avoid underflowing the unsigned value.
    */
    ullDelta = 0;

    while(ullTicks > ullDelta)
    {
        D_UINT64 ullNow = DclOsHighResTickCount();

        ullTicks -= ullDelta;

        /*  The HighRes ticker could have reached its period and wrapped
            back to zero.  Note that if the ticker runs the full range of
            its 64-bit value, ullHRTickModulus is zero, which works fine.
        */
        if(ullNow < ullStartTick)
            ullNow += ullHRTickModulus;

        ullDelta = ullNow;
        ullDelta -= ullStartTick;
        ullStartTick = ullNow;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclNanosecondDelayMinimum()

    Return the minimum allowable nanosecond count which may be passed
    to DclNanosecondDelay().

    Parameters:
        None.

    Return Value:
        If successful, returns the minimum nanosecond count,
        otherwise returns 0, indicating that the nanosecond
        delay functionality is not available.
-------------------------------------------------------------------*/
D_UINT32 DclNanosecondDelayMinimum(void)
{
    DCLSTATUS   dclStat;

    /*  Calling this function ensures that the nanosecond delay
        functionality is both available and initialized.
    */
    dclStat = DclNanosecondDelay(0);

    if(dclStat == DCLSTAT_SUCCESS || dclStat == DCLSTAT_OUTOFRANGE)
        return ulMinAllowableDelayNS;
    else
        return 0;
}


/*-------------------------------------------------------------------
    Local: Initialize()

    Perform one-time initialization of the nanosecond delay functionality.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS Initialize(void)
{
    D_UINT64    ullTickResolution;

    /*  Obtain the HighRes tick characteristics.
    */
    ullHRTickModulus = DclOsHighResTickModulus();
    ullTickResolution = DclOsHighResTickResolution();
    ulSystemTickNS = DclOsTickResolution() * 1000;

    DCLPRINTF(1, ("DclNanosecondDelay() HighResTickResolution %llU ps per tick, modulus %llU, SysTickResolution=%lU ns\n",
        VA64BUG(ullTickResolution), VA64BUG(ullHRTickModulus), ulSystemTickNS));

    /*  If the time stamp frequency won't provide adequate resolution,
        return failure.
    */
    if(ullTickResolution > MAXIMUM_TICK_RESOLUTION)
        return DCLSTAT_OUTOFRANGE;

    ulHRTickResolution = (D_UINT32)ullTickResolution;

    /*  The essence of the delay algorithm is looping based on the high
        res ticker.  The minimum amount that we can accurately delay must
        be significantly larger than the tick resolution.  Ensuring that
        it is 8 times larger means our delays should be accurate to within
        (approximately) 25% (accounts for inaccuracy at both the start and
        stop times, as well as general interface overhead).
    */
    DclAssert(ulHRTickResolution < (D_UINT32_MAX / ACCURACY_SCALE));
    ulMinAllowableDelayNS = (ulHRTickResolution * ACCURACY_SCALE) / PICOS_PER_NANO;

    DCLPRINTF(1, ("                     Minimum allowable delay: %lU ns\n", ulMinAllowableDelayNS));

    return DCLSTAT_SUCCESS;
}


