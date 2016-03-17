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

    This module implements thread safe timers based on the system clock tick.
    A timer may be set to expire after a time period has elapsed.  It can be
    tested for whether it has expired; the time remaining until expiration
    can be determined; or the time since it was set can be determined.

    Timeouts are specified in milliseconds, but are measured to the
    resolution of the system clock tick.  Timeouts are rounded up to the
    next full tick period, and may be as much as a tick longer than that.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltimer.c $
    Revision 1.15  2010/09/19 03:06:13Z  garyp
    Added DclTimerSamplePeriod().
    Revision 1.14  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.13  2009/11/12 01:06:47Z  garyp
    Added a missing typecast stemming from the previous checkin.
    Revision 1.12  2009/11/10 23:08:28Z  garyp
    Eliminated the on-the-fly service initialization.  Modified DclTimerSet()
    to return a DCLSTATUS code.  Removed those 64-bit macros which
    are no longer necessary.
    Revision 1.11  2009/06/28 00:45:55Z  garyp
    Updated to use this feature as a service.
    Revision 1.10  2009/02/08 00:30:54Z  garyp
    Merged from the v4.0 branch.  Added DclTimePassedUS() and DclTimePassedNS().
    Revision 1.9  2008/05/27 17:09:54Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.8  2008/03/19 19:04:19Z  Garyp
    Added asserts.
    Revision 1.7  2007/12/18 04:51:50Z  brandont
    Updated function headers.
    Revision 1.6  2007/11/29 23:23:11Z  Glenns
    Modified DCLTimer structure and member functions to account for system
    timer granularity.  See Bugzilla #1624.
    Revision 1.5  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.4  2007/02/21 19:47:10Z  Garyp
    Modified the timestamp services to use 64-bit math, allowing them to
    measure periods longer than 71 minutes.
    Revision 1.3  2006/10/26 02:52:12Z  Garyp
    Updated to allow the timestamp functions to be compiled out in favor of
    the high-res timestamp functions.  General cleanup.
    Revision 1.2  2005/12/13 22:37:21Z  billr
    Correct assertion (and comment) to match interface of DclTimerSet().
    Revision 1.1  2005/10/02 05:20:02Z  Pauli
    Initial revision
    Revision 1.2  2005/08/03 19:14:00Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/06 11:04:16Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlservice.h>
#include <dlinstance.h>

static void UpdateTimer(DCLTIMER *pT);

#define MICROSEC_PER_MILLISEC (1000)    /* Microseconds per millisecond */

static D_UINT32 ulTickResolutionUS;     /* Microseconds per tick        */
static D_UINT32 ulTickModulus;          /* Tick count rolls over; or 0  */
static D_UINT32 ulTickPeriodMS;         /* Maximum sample period MS     */


/*-------------------------------------------------------------------
    Public: DclTimerServiceInit()

    Initialize the Timer subsystem.  Typically this is called one 
    time, very early in the startup process.  This service does not
    allocate any system resources, and therefore does not require
    any shutdown operations.

    Parameters:
        hDclInst - The DCL instance handle.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclTimerServiceInit(
    DCLINSTANCEHANDLE       hDclInst)
{
    static DCLSERVICEHEADER SrvTimer[DCL_MAX_INSTANCES];
    DCLSERVICEHEADER       *pSrv;
    DCLSTATUS               dclStat;
    D_UINT64                ullTickPeriodUS;

    /*  REMINDER! Because some environments are capable of restarting,
                  without reinitializing static data, this function must
                  explicitly ensure that any static data is manually
                  zero'd, rather than just expecting it to already be in
                  that state.
    */

    /*  The "resolution" field is the only field for which the above
        REMINDER matters...
    */
    ulTickResolutionUS = 0;

    if(!hDclInst)
    {
        hDclInst = DclInstanceHandle(0);
        if(!hDclInst)
            return DCLSTAT_TIMER_INSTANCEHANDLEFAILED;
    }

    /*  There are potential thread safety issues with the following
        initialization process, however this function should be
        called early before any such issues can occur.
    */
    ulTickModulus = DclOsTickModulus();
    ulTickResolutionUS = DclOsTickResolution();

    if(ulTickResolutionUS == 0)
        return DCLSTAT_TIMER_SERVICEINITFAILED;
         
    if(ulTickModulus)
        ullTickPeriodUS = ((D_UINT64)ulTickModulus) * ulTickResolutionUS;
    else
        ullTickPeriodUS = UINT64SUFFIX(0x100000000) * ulTickResolutionUS;

    if(ullTickPeriodUS / 1000 < D_UINT32_MAX)
        ulTickPeriodMS = (D_UINT32)(ullTickPeriodUS / 1000);
    else
        ulTickPeriodMS = D_UINT32_MAX;

    pSrv = &SrvTimer[DclInstanceNumber(hDclInst)];
    dclStat = DclServiceHeaderInit(&pSrv, "DLTIMER", DCLSERVICE_TIMER, NULL, NULL, 0);
    if(dclStat == DCLSTAT_SUCCESS)
        dclStat = DclServiceCreate(hDclInst, pSrv);

    if(dclStat == DCLSTAT_SUCCESS)
    {
        /*  Warn if the period is so small that various tests and such may
            not function properly.  One hour should be OK for most things.
        */        
        if(ulTickPeriodMS < (60*60*1000))
            DclPrintf("Warning: The timer period of %lU ms is unusually small!\n", ulTickPeriodMS);
    }
    
    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclTimerSet()

    Set a timer to indicate timeout after the specified number of
    milliseconds.  The timeout will occur no sooner than specified,
    and may occur up to two ticks later.

    The maximum permitted value of ulMillisec is platform dependent
    and can be determined by calling DclTimerSamplePeriod().
    
    Converted to microseconds, it must not exceed the largest value
    that fits in a D_UINT32 minus the number of microseconds in a 
    clock tick.  Practically speaking, this will always be in excess
    of an hour.

    Parameters:
        pT         - A pointer to the DCLTIMER object to use
        ulMillisec - The number of milliseconds for the timeout

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclTimerSet(
    DCLTIMER       *pT,
    D_UINT32        ulMillisec)
{
    DclAssert(pT);

    if(!ulTickResolutionUS)
    {
        DCLPRINTF(1, ("DclTimerSet() Timer service is not initialized!\n"));
        DclError();
        return DCLSTAT_SERVICE_NOTINITIALIZED;
    }

    /*  Set the timer right now.
    */
    pT->ulLastTick = DclOsTickCount();

    /*  Calculate the number of ticks until the timer expires.  Round up
        for any fraction of a tick represented by ulMillisec.  Note that
        the first tick could be very short; this is handled by calculating
        the timer granularity in UpdateTimer().
    */
    DclAssert(ulMillisec <= ((D_UINT32_MAX - (ulTickResolutionUS - 1)) / MICROSEC_PER_MILLISEC));
    
    pT->ulEndTicks = ((ulMillisec * MICROSEC_PER_MILLISEC + ulTickResolutionUS - 1) / ulTickResolutionUS);
    
    pT->ulElapsed = 0;
    pT->uGranularity = 0;
    pT->fExpired = FALSE;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclTimerExpired()

    Test a timer previously set with DclTimerSet() to determine
    whether the specified time has elapsed.

    Parameters:
        pT         - A pointer to the DCLTIMER object to use

    Return Value:
        Returns TRUE if the timer has expired, FALSE if it has not.
-------------------------------------------------------------------*/
D_BOOL DclTimerExpired(
    DCLTIMER       *pT)
{
    DclAssert(pT);

    UpdateTimer(pT);

    return pT->fExpired ? TRUE : FALSE;
}


/*-------------------------------------------------------------------
    Public: DclTimerRemaining()

    Determine how long it will be until a timer previously set
    with DclTimerSet() expires.

    Parameters:
        pT         - A pointer to the DCLTIMER object to use

    Return Value:
        Returns the number of milliseconds until the timer expires.
        If it has already expired, returns zero.
-------------------------------------------------------------------*/
D_UINT32 DclTimerRemaining(
    DCLTIMER       *pT)
{
    DclAssert(pT);

    UpdateTimer(pT);

    if(!pT->fExpired)
    {
        /*  Convert the number of ticks remaining to milliseconds: multiply
            by the number of microseconds per tick, then divide by 1000
            microseconds per millisecond, rounding up.
        */
        return (((pT->ulEndTicks - pT->ulElapsed) * ulTickResolutionUS
              + MICROSEC_PER_MILLISEC - 1) / MICROSEC_PER_MILLISEC);
    }
    else
    {
        /*  Expired timers have no time remaining.
        */
        return 0;
    }
}


/*-------------------------------------------------------------------
    Public: DclTimerElapsed()

    Determine how long it has been since a timer was set with
    DclTimerSet().  This count keeps accumulating even after the
    timer has expired.

    Parameters:
        pT         - A pointer to the DCLTIMER object to use

    Return Value:
        Returns the number of milliseconds that have passed since the
        timer was last set.
-------------------------------------------------------------------*/
D_UINT32 DclTimerElapsed(
    DCLTIMER       *pT)
{
    DclAssert(pT);

    UpdateTimer(pT);

    /*  Convert the number of ticks elapsed to milliseconds: multiply
        by the number of microseconds per tick, then divide by 1000
        microseconds per millisecond, truncating the result.
    */
    return (pT->ulElapsed * ulTickResolutionUS) / MICROSEC_PER_MILLISEC;
}


/*-------------------------------------------------------------------
    Public: DclTimerSamplePeriod()

    Get the length in milliseconds, of the maximum sample period
    which the Timer API is capable of supporting.  Sampling the 
    timer too infrequently will result in timer innaccuracy.

    Parameters:
        None.

    Return Value:
        Returns the maximum timer period in milliseconds.
-------------------------------------------------------------------*/
D_UINT32 DclTimerSamplePeriod(void)
{
    return ulTickPeriodMS;
}   


#if !DCLCONF_HIGHRESTIMESTAMP
/*  If so designated, do not compile these functions, because we
    want to use the high-res timestamp rather than the standard
    system tick based timestamp.  Note that this may ONLY be done
    if the high-res timer code is implemented (in oshrtick.c),
    and has a period long enough to time something useful (some
    high-res timers wrap REALLY fast).
*/

/*-------------------------------------------------------------------
    Public: DclTimeStamp()

    Get a timestamp representing the current time suitable for
    use as an argument to DclTimePassed(), DclTimePassedUS(), and
    DclTimePassedNS().

    Parameters:
        None.

    Return Value:
        Returns a timestamp value.
-------------------------------------------------------------------*/
DCLTIMESTAMP DclTimeStamp(void)
{
    return DclOsTickCount();
}


/*-------------------------------------------------------------------
    Public: DclTimePassed()

    Determine the elapsed time in milliseconds since the timestamp
    was marked.

    The maximum number of milliseconds that can be measured is
    never greater than the equivalent of ~49.7 days, however it
    is also dependent on the system tick modulus value, which is 
    platform dependent.

    If tTimestamp is zero, this function will effectively return
    the elapsed time since system startup, with the same caveats
    regarding the maximum timeable period.

    Parameters:
        tTimestamp - A timestamp value returned by DclTimeStamp()

    Return Value:
        Returns the number of milliseconds that have elapsed.  If
        the time period is longer than the system tick modulus, the
        return value is undefined.
-------------------------------------------------------------------*/
D_UINT32 DclTimePassed(
    DCLTIMESTAMP    tTimestamp)
{
    DCLTIMESTAMP    tNow = DclOsTickCount();
    D_UINT32        ulElapsedTicks = tNow - tTimestamp;
    D_UINT64        ullElapsed64;

    ullElapsed64 = ulElapsedTicks;

    /*  Check for reaching the tick modulus.  Note that there is
        no way to know whether it has actually been reached multiple
        times.
    */
    if(tNow < tTimestamp)
        ullElapsed64 += ulTickModulus;

    if(!ulTickResolutionUS)
    {
        DCLPRINTF(1, ("DclTimePassed() Timer service is not initialized!\n"));

        /*  Don't assert here, as it is <possible>, that this will happen
            in normal execution (but typically only if the services are
            initialized in a bad order.

            Fall through -- will end up returning zero...
        */
    }
    
    DclUint64MulUint32(&ullElapsed64, ulTickResolutionUS);
    DclUint64DivUint32(&ullElapsed64, MICROSEC_PER_MILLISEC);

    return (D_UINT32)ullElapsed64;
}


/*-------------------------------------------------------------------
    Public: DclTimePassedUS()

    Determine the elapsed time in microseconds since the timestamp
    was marked.

    The maximum number of microseconds that can be measured is
    never greater than the equivalent of ~71.5 minutes, however
    it is also dependent on the system tick modulus value, which
    is platform dependent.

    If tTimestamp is zero, this function will effectively return
    the elapsed time since system startup, with the same caveats
    regarding the maximum timeable period.

    Parameters:
        tTimestamp - A timestamp value returned by DclTimeStamp()

    Return Value:
        Returns the number of microseconds that have elapsed.  If
        the time period is longer than the system tick modulus, the
        return value is undefined.
-------------------------------------------------------------------*/
D_UINT32 DclTimePassedUS(
    DCLTIMESTAMP    tTimestamp)
{
    D_UINT32        ulMS = DclTimePassed(tTimestamp);

    if(ulMS > D_UINT32_MAX / 1000)
        return D_UINT32_MAX;
    else
        return ulMS * 1000;
}


/*-------------------------------------------------------------------
    Public: DclTimePassedNS()

    Determine the elapsed time in nanoseconds since the timestamp
    was marked.

    The maximum number of nanoseconds that can be measured is
    never greater than the equivalent of ~584 years, however it
    is also dependent on the system tick modulus value, which
    is platform dependent.

    If tTimestamp is zero, this function will effectively return
    the elapsed time since system startup, with the same caveats
    regarding the maximum timeable period.

    Parameters:
        tTimestamp - A timestamp value returned by DclTimeStamp()

    Return Value:
        Returns the number of nanoseconds that have elapsed.  If
        the time period is longer than the system tick modulus, the
        return value is undefined.
-------------------------------------------------------------------*/
D_UINT64 DclTimePassedNS(
    DCLTIMESTAMP    tTimestamp)
{
    D_UINT32        ulMS = DclTimePassed(tTimestamp);
    D_UINT64        ullNS;

    ullNS = ulMS;
    DclUint64MulUint32(&ullNS, 1000*1000);

    return ullNS;
}

#endif  /* !DCLCONF_HIGHRESTIMESTAMP */


/*-------------------------------------------------------------------
    Local: UpdateTimer()

    Accumulate the ticks that have passed since the last call, and
    test whether it has reached the expiration count.

    Parameters:
        pT         - A pointer to the DCLTIMER object to use

    Return Value:
        None.
-------------------------------------------------------------------*/
static void UpdateTimer(
    DCLTIMER       *pT)
{
    D_UINT32        ulCurrentTick = DclOsTickCount();
    D_UINT32        ulElapsed;

    DclAssert(pT);

    /*  If the current tick is less than the last one, the tick count
        wrapped around at the modulus value.  Adjust the current tick
        accordingly.
    */
    if(ulCurrentTick < pT->ulLastTick)
        ulCurrentTick += ulTickModulus;

    /*  Accumulate the ticks that have passed since the last call.
    */
    ulElapsed = ulCurrentTick - pT->ulLastTick;

    if(ulElapsed)
    {
        pT->ulElapsed += ulElapsed;

        if(pT->uGranularity == 0 || ulElapsed < pT->uGranularity)
        {
            DclAssert(ulElapsed <= D_UINT16_MAX);
            pT->uGranularity = (D_UINT16)ulElapsed;
        }

        /*  Only test for expiration once.  That way a timer is always
            expired until it is reset, no matter how many ticks elapse.
        */
        if(!pT->fExpired && pT->ulElapsed >= pT->ulEndTicks + pT->uGranularity)
            pT->fExpired = TRUE;

        /*  Record the most recent tick count for the next call.
        */
        pT->ulLastTick = ulCurrentTick;
    }

    return;
}

