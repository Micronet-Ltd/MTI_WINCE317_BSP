/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

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

    This is a generic delay function based on a counted loop.  While
    this is far from an ideal way to provide accurate delays, it may
    be the best available in some operating environments.

    The implementation uses a calibrated loop count: a known number of
    passes around a loop takes a known amount of time.  The basic
    calculation to find the loop count given an arbitrary amount of
    time is:

       loop count = time * (loops per unit time)

    For this function, time is in microseconds, so:

       loop count = microseconds * (loops per microsecond)

    A completely general implementation must be able to handle loop rates
    that range from less than 1 (multiple microseconds per loop, possible
    on older or very low power implementations) to thousands (fast modern
    or future processors).  Using floating point calculations is out of
    the question, so scaled integer arithmetic must be used:

                    microseconds * ((scale factor) * (loops per microsecond))
       loop count = ---------------------------------------------------------
                                       (scale factor)

    Precision is maintained by evaluating the numerator of the fraction
    before dividing by the scale factor.  Note that the expression
    (scale factor) * (loops per microsecond) represents the loop count
    that produces an execution time of (scale factor) microseconds.

    Avoiding integer overflow takes some care.  A larger scale factor
    produces a more precise result, but decreases the maximum delay that
    can be handled without overflow.

    Instead of placing a (platform-dependent) limitation on the maximum
    delay that can be handled, the delay is performed in units of
    (scale factor) microseconds, each of which takes (scale factor) *
    (loops per microseconds) loops.  The loop count for any remaining
    amount (less than (scale factor) microseconds) is calculated using
    the formula above.

    Thus, the condition required to avoid overflow is that the product
    of the scale factor (representing a time in microseconds) times the
    number of loops executed in that many microseconds must not overflow.

    Division is a costly operation.  The cost is drastically reduced by
    using a scale factor that is a power of two, and shifting instead
    of dividing.

    This implementation makes some reasonable assumptions.  The clock
    tick period must be constant.  The period must be a reasonable
    fraction of a second (roughly 1 msec. to 100 msec.).  Overhead due
    to clock tick processing must be negligible.  Other threads of
    execution scheduled during calibration must either consume
    negligible CPU time or in excess of a clock tick (this latter case
    is accounted for during calibration).

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: delay.c $
    Revision 1.9  2010/01/12 18:28:17Z  billr
    Resolve bug 2964: header refactoring requires including fxdelay.h
    for prototypes of FfxDelay(), FfxDelayCalibrate().
    Revision 1.8  2009/04/01 15:38:30Z  davidh
    Function Headers Updated for AutoDoc.
    Revision 1.7  2008/01/13 07:27:08Z  keithg
    Function header updates to support autodoc.
    Revision 1.6  2007/11/07 00:42:14Z  pauli
    Made #error message strings to prevent macro expansion.
    Revision 1.5  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2007/10/27 16:43:54Z  Garyp
    Updated to use DclBitHighest(), rather than the now obsolete FlashFX
    specific function.
    Revision 1.3  2006/10/03 23:57:41Z  Garyp
    Updated to use the new style printf macros and functions.
    Revision 1.2  2006/02/01 03:28:19Z  Garyp
    Added a missing Sibley conditiional check.
    Revision 1.1  2005/10/02 01:32:48Z  Pauli
    Initial revision
    Revision 1.1  2005/10/02 02:32:48Z  Garyp
    Initial revision
    Revision 1.3  2005/09/20 19:01:10Z  pauli
    Replaced _syssleep with DclOsSleep.
    Revision 1.2  2005/09/17 23:57:14Z  garyp
    Modified so that _sysdelay() and related code is only built if
    FFXCONF_NORSUPPORT is TRUE.
    Revision 1.1  2005/08/03 19:30:40Z  pauli
    Initial revision
    Revision 1.2  2005/08/03 19:30:40Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/03 09:26:40Z  pauli
    Initial revision
    Revision 1.7  2005/01/23 00:21:11Z  billr
    No longer needs to include limits.h.
    Revision 1.6  2004/12/30 23:00:10Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.5  2004/09/16 00:17:48Z  GaryP
    Tweaked the output message.
    Revision 1.4  2004/08/30 00:45:17Z  GaryP
    Changed to use FFX standard data types, eliminated unnecessary casts, and
    added debugging code.
    Revision 1.3  2004/08/12 20:06:00Z  Pauli
    Corrected inequality on compile time callibration settings test.
    Revision 1.2  2004/08/10 06:14:35Z  garyp
    Updated to build with the ARM Developer's Suite.
    Revision 1.1  2004/07/19 23:53:38Z  BillR
    Initial revision
    ---------------------
    Bill Roman 2004-07-19
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdelay.h>

#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT

/*  It's possible to supply all of the timing parameters at compile
    time.  If any one is omitted, all of them must be determined at
    run time.
*/
#if defined(FFX_DELAY_COUNT) && defined(FFX_DELAY_MICROSEC) && defined(FFX_DELAY_SHIFT)

/*  If timing parameters were supplied, make sure they make sense.

    The delay microseconds must be a power of two, as represented by
    the delay shift count.
*/
#if (1UL << FFX_DELAY_SHIFT) != FFX_DELAY_MICROSEC
#error "FFX_DELAY_SHIFT does not match FFX_DELAY_MICROSEC"
#endif

/*  The delay count and delay microseconds must be such that calculations
    involving them will fit in an unsigned long.
*/
#if (((D_UINT32_MAX - (FFX_DELAY_MICROSEC - 1)) /  FFX_DELAY_COUNT)  \
     < FFX_DELAY_MICROSEC)
#error "FFX_DELAY_COUNT and FFX_DELAY_MICROSEC are too large"
#endif

#else /* Not all of the timing parameters were supplied. */

#undef FFX_DELAY_COUNT
#define FFX_DELAY_COUNT 0

#undef FFX_DELAY_MICROSEC
#define FFX_DELAY_MICROSEC 0

#undef FFX_DELAY_SHIFT
#define FFX_DELAY_SHIFT 0

#endif


/*  IMPORTANT NOTE: the following defines give meaningful names to some
    important constants.  They are emphatically NOT configuration
    parameters to be tweaked blindly!  They may be changed, but only
    with a complete understanding of their purpose and relationship.
*/

/*  The minimum number of microsends of delay to produce per pass of
    the loop in FfxDelay().  This needs to be large enough that the
    loop overhead is insignificant.  Note that the maximum loop rate
    depends on this: it is D_UINT32_MAX / MIN_MICROSEC MHz.
*/
#define MIN_MICROSEC (8192)

/*  The maximum length FfxDelay() to use during calibration.  If the
    clock tick period is short enough, a shorter delay is used.
*/
#define MAX_CALIBRATION_MICROSEC (500 * 1000UL)

/*  The maximum number of clock tick periods to FfxDelay() during
    calibration.  If the tick period is too long, fewer tick periods
    will be used.
*/
#define MAX_CALIBRATION_TICKS (100)

/*  The minimum number of clock tick periods for calibration.  If there
    are fewer than this, it won't be accurate enough.
*/
#define MIN_CALIBRATION_TICK_PERIODS (5)

/*  The maximum clock tick period in microseconds that can be expected
    to work reasonably well.
*/
#define MAX_TICK_PERIOD (100 * 1000UL)

/*  The number of times a delay count must produce a nonzero tick count
    before it's considered to be valid.
*/
#define TICK_PERIOD_TRIES (3)

/*  The number of times to try refining the calibration.
*/
#define CALIBRATION_TRIES (5)

/*  The fraction by which to speed up the estimated loop rate when
    adjusting it -- the adjustment is divided by this value.
*/
#define ADJUSTMENT_FRACTION (2)

/*  Private variables
*/
static D_UINT32 ulDelayCount = FFX_DELAY_COUNT;
static D_UINT32 ulDelayMicrosec = FFX_DELAY_MICROSEC;
static D_UINT16   uDelayShift = FFX_DELAY_SHIFT;


/*  Local functions.
*/
static void     delayloop(
    D_UINT32 ulCount);
#if !FFX_DELAY_COUNT
static D_BOOL   normalize(
    D_UINT32 ulCount,
    D_UINT32 ulMicrosec);
#endif


/*-------------------------------------------------------------------
    Public: FfxDelay()

    Delay execution a specified number of microseconds.

    FfxDelay delays execution for the specified number of
    microseconds, not returning until at least that time has
    elapsed. The implementation must be relatively accurate:
    insufficient delay can cause malfunction by violating flash chip
    specifications, while excessive delay is likely to decrease
    performance.

    FfxDelay does not attempt to yield the processor to other tasks
    in a multitasking system, as this could have a serious effect on
    performance. If a longer delay (multiple system clock ticks) is
    needed, DclOsSleep should be used instead.

    Parameters:
        ulMicroSeconds - Number of microseconds (1/1,000,000) to delay.

    Return Value:
        none
-------------------------------------------------------------------*/
void FfxDelay(
    D_UINT32 ulMicroSeconds)
{
    DclAssert(ulDelayCount != 0);
    DclAssert(ulDelayMicrosec != 0);

    while(ulMicroSeconds >= ulDelayMicrosec)
    {
        delayloop(ulDelayCount);
        ulMicroSeconds -= ulDelayMicrosec;
    }

    delayloop((ulMicroSeconds * ulDelayCount + ulDelayMicrosec - 1)
              >> uDelayShift);
}


/*-------------------------------------------------------------------
    Public: FfxDelayCalibrate()

    Calibrate the FfxDelay delay loop:

    Initialize the timing Parameters: for an implementation of
    FfxDelay() that uses a simple delay loop.

    Calibration parameters may be set at compile time, in which
    case this function does nothing.

    If both ulMicrosec and ulCount are non-zero, they are taken
    as specifying the ratio of loops per microsecond to be used
    by FfxDelay().

    If either parameter is zero, the loop counter is calibrated by
    using _sysgettickcount() and _sysgettickresolution() as a time
    reference.

    Parameters:
        ulMicrosec - the number of microseconds needed for ulCount loops.
        ulCount    - the number of loops that takes ulMicrosec microseconds.

    Return Value:
        TRUE if calibration succeeded, FALSE if not.

        Calibration can fail because of obviously ridiculous and
        unusable values of the parameters, or because the attempt
        to calibrate to the system tick produced inconsistent results,
        possibly because of interference from task scheduling.
        In the latter case, retrying this function may succeed.
-------------------------------------------------------------------*/
D_BOOL FfxDelayCalibrate(
    D_UINT32 ulMicrosec,
    D_UINT32 ulCount)
{
    FFXPRINTF(1, ("FfxDelayCalibrate() ulMicroSec=%lU ulCount=%lU\n",
                  ulMicrosec, ulCount));

#if !FFX_DELAY_COUNT

    if(ulMicrosec && ulCount)
    {
        /*  Calibration values were provided at run time.
        */
        if(!normalize(ulCount, ulMicrosec))
            return FALSE;
    }
    else
    {
        D_UINT32
            ulStartTick, ulEndTick, ulTotalElapsed, ulLeastTicks, ulCalTicks,
            ulElapsedTicks, ulCalDelay,
            ulTickResolution = DclOsTickResolution(),
            ulTickModulus = DclOsTickModulus();
        int             tries;

        /*  Set an initial estimate for the number of loops in one clock
            tick.  Lowball this so as not to waste a lot of time on slow
            processors.  Start by assuming that the processor performs
            one loop per microsecond and that each clock tick increments
            the tick count by one (this is not true on all platforms).
            Verify that the initial loop count produces a delay of less
            than a tick (it's okay if it is much less); decrease it if
            necessary.
        */
        ulCount = ulTickResolution;
        for(ulStartTick = DclOsTickCount();
            delayloop(ulCount), ulStartTick != DclOsTickCount();
            ulStartTick = DclOsTickCount())
        {
            ulCount /= 2;
            if(ulCount == 0)
            {
                DclError();
                return FALSE;
            }
        }

        /*  The count now produces a delay of less than one clock tick
            period.  Increase it slowly until it produces a delay of at
            least a full tick period.  If the tick count changes
            during a delay, sample it again.
        */
        ulTotalElapsed = ulLeastTicks = 0;
        for(tries = 1; tries <= TICK_PERIOD_TRIES; ++tries)
        {
            ulStartTick = DclOsTickCount();
            delayloop(ulCount);
            ulEndTick = DclOsTickCount();
            if(ulEndTick < ulStartTick)
            {
                ulEndTick += ulTickModulus;
            }
            ulElapsedTicks = ulEndTick - ulStartTick;

            /*  If the tick count changed during the delay, a clock tick
                occurred.  Perhaps the delay count is now sufficient to
                to delay for a clock tick period.
            */
            if(ulElapsedTicks != 0)
            {
                ulTotalElapsed += ulElapsedTicks;

                /*  Keep track of the lowest non-zero change in tick count
                    observed.  Usually this will be one, but some systems
                    (notably, WinCE) may lie about the tick resolution.
                */
                if(ulLeastTicks == 0 || ulElapsedTicks < ulLeastTicks)
                    ulLeastTicks = ulElapsedTicks;
            }
            else
            {
                /*  No tick occurred during the delay, so it must have been
                    less than a tick period.  Increase it by 50% and start
                    over with this new count (making sure the increase
                    won't overflow).
                */
                if(ulCount > D_UINT32_MAX / 2)
                {
                    DclError();
                    return FALSE;
                }
                ulCount += (ulCount + 1) / 2;
                tries = 0;
                ulTotalElapsed = 0;
            }
        }

        /*  ulCount should now represent between one and two tick periods.
            ulLeastTicks is the number of reported ticks in a tick period.
            ulTotalElapsed is the number of reported ticks corresponding
            to approximately ulCount * tries loops.

            Use these values as an initial estimate.  It doesn't have
            to be particularly close.
        */
        if(ulLeastTicks * ulTickResolution > MAX_TICK_PERIOD)
        {
            DclError();
            return FALSE;
        }

        if(!normalize(ulCount, ((ulTotalElapsed * ulTickResolution)
                                / TICK_PERIOD_TRIES)))
        {
            return FALSE;
        }

        /*  Decide how long a delay to use for calibration.  Use as many
            tick periods as possible without exceeding a reasonable
            maximum delay.  Quietly assume that the tick period is a fraction
            of a second.
        */
        if(ulLeastTicks * ulTickResolution * MAX_CALIBRATION_TICKS
           <= MAX_CALIBRATION_MICROSEC)
        {
            ulCalTicks = ulLeastTicks * MAX_CALIBRATION_TICKS;
        }
        else
        {
            ulCalTicks = (MAX_CALIBRATION_MICROSEC
                          / (ulLeastTicks * ulTickResolution));
        }

        FFXPRINTF(2, ("Calibration ticks: %lU\n", ulCalTicks));

        if(ulCalTicks / ulLeastTicks < MIN_CALIBRATION_TICK_PERIODS)
        {
            DclError();
            return FALSE;
        }
        ulCalDelay = ulCalTicks * ulTickResolution;

        for(tries = 0; tries < CALIBRATION_TRIES; ++tries)
        {
            ulStartTick = DclOsTickCount();
            FfxDelay(ulCalDelay);
            ulEndTick = DclOsTickCount();
            if(ulEndTick < ulStartTick)
                ulEndTick += ulTickModulus;
            ulElapsedTicks = ulEndTick - ulStartTick;
            if(ulElapsedTicks < ulLeastTicks)
            {
                DclError();
                return FALSE;
            }
            DclAssert(ulElapsedTicks % ulLeastTicks == 0);

            FFXPRINTF(2, ("Elapsed ticks: %lU\n", ulElapsedTicks));

            /*  if (ulElapsedTicks == ulCalTicks)
                break;
            */

            /*  Adjust the number of loops proportionally to the error.
                The calculation for the total error is:

                                      ulCalTicks
                ulCount = ulCount * --------------
                                    ulElapsedTicks

                                    ulElapsedTicks + ulCalTicks - ulElapsedTicks
                        = ulCount * --------------------------------------------
                                                 ulElapsedTicks

                                              ulCalTicks - ulElapsedTicks
                        = ulCount + ulCount * ---------------------------
                                                     ulElapsedTicks

                That's fine algebraically, but be careful when subtracting
                unsigned values.
            */
            ulCount = ulDelayCount;
            if(ulElapsedTicks < ulCalTicks)
            {
                /*  It definitely takes more counts to produce the desired
                    delay.
                */
                ulCount += ((ulCount * (ulCalTicks - ulElapsedTicks))
                            / ulElapsedTicks);
            }
            else
            {
                /*  Because the measurement is noisy (by one tick period),
                    don't adjust by the full amount when decreasing the
                    number of counts.
                */
                ulCount -= ((ulCount * (ulElapsedTicks - ulCalTicks))
                            / ulElapsedTicks) / ADJUSTMENT_FRACTION;
            }
            if(!normalize(ulCount, ulDelayMicrosec))
                return FALSE;
        }

        /*  Perform a final calibration to err on the safe side of the
            uncertainty: the actual time interval of the delay could be
            less than ulCalTicks by a tick period (ulLeastTicks) if the
            delay began just before a tick and ended immediately after
            one.

            See "Adjust the number of loops" comment above for derivation
            of the adjustment.
        */
        ulCount =
            ulDelayCount
            + ((ulDelayCount * ulLeastTicks + ulCalTicks - ulLeastTicks - 1)
               / (ulCalTicks - ulLeastTicks));

        if(!normalize(ulCount, ulDelayMicrosec))
            return FALSE;
    }

#endif

    /*  ulDelayMicrosec must be a power of two, which is uDelayShift.
    */
    DclAssert(ulDelayMicrosec == (1UL << uDelayShift));

    /*  Verify that no overflow will occur by testing:

          ulDelayMicrosec * ulDelayCount + (ulDelayMicrosec - 1) <= D_UINT32_MAX

        Of course this test must be done without overflowing, so
        rearrange it.
    */
    DclAssert(ulDelayCount != 0);
    DclAssert((D_UINT32_MAX - (ulDelayMicrosec - 1)) / ulDelayCount >=
                ulDelayMicrosec);

    DclPrintf("Detected FfxDelay() parameters: Count=%lU MicroSec=%lU Shift=%U\n",
         ulDelayCount, ulDelayMicrosec, uDelayShift);

    return TRUE;                /* successful calibration */
}


/*-------------------------------------------------------------------
    Public: delayloop()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void delayloop(
    D_UINT32 ulCount)
{
    volatile D_UINT32 u = ulCount;

    while(u != 0)
        --u;
}


#if !FFX_DELAY_COUNT

/*-------------------------------------------------------------------
    Public: normalize()

    Set ulDelayMicrosec, uDelayShift, and ulDelayCount such that
    ulDelayMicrosec is a power of two equal to 1 << uDelayShift, and
    ulDelayCount/ulDelayMicrosec) is approximately equal to ulCount/ulMicrosec

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL normalize(
    D_UINT32 ulCount,
    D_UINT32 ulMicrosec)
{
    FFXPRINTF(2, ("normalize: ulCount = %lU, ulMicrosec = %lU\n",
                  ulCount, ulMicrosec));

    /*  Check for reasonable parameters.  Obviously, neither the loop
        count nor the corresponding time can be zero.  Additionally, to
        allow scaling the time up to a reasonable value (to minimize
        loop overhead in FfxDelay), there must be a limit on the ratio
        of ulCount/ulMicrosec such that:

          (ulCount/ulMicrosec) * MIN_MICROSEC <= D_UINT32_MAX

        Or, taking into account the rounding and truncation in the
        actual calculation used, and avoiding overflow in the test:

           ulCount <= D_UINT32_MAX / ((MIN_MICROSEC + ulMicrosec - 1) / ulMicrosec)

        Note that this limits the maximum allowable loop rate to
        D_UINT32_MAX/MIN_MICROSEC MHz.
    */
    if(ulCount == 0 || ulMicrosec == 0
       || ulCount >
       D_UINT32_MAX / ((MIN_MICROSEC + ulMicrosec - 1) / ulMicrosec))
    {
        DclError();
        return FALSE;
    }

    if(ulMicrosec < MIN_MICROSEC)
    {
        ulCount *= ((MIN_MICROSEC + ulMicrosec - 1) / ulMicrosec);
        ulMicrosec *= ((MIN_MICROSEC + ulMicrosec - 1) / ulMicrosec);
    }

    /*  Find the largest power of two less than ulMicrosec.
    */
    uDelayShift = DclBitHighest(ulMicrosec) - 1;
    ulDelayMicrosec = 1UL << uDelayShift;

    /*  ulCount now has to be scaled down correspondingly, which involves
        multiplying by ulDelayMicrosec/ulMicrosec (a fraction), rounding
        up.  To do this without overflow requires that:

           ulCount * ulDelayMicrosec + (ulMicrosec - 1) <= D_UINT32_MAX

        Rearranging this condition to one that can be evaluated safely:

                      D_UINT32_MAX - (ulMicrosec - 1)
           ulCount <= ----------------------------
                            ulDelayMicrosec

    */

    while(ulCount > (D_UINT32_MAX - (ulMicrosec - 1)) / ulDelayMicrosec)
    {
        ulCount = (ulCount + 1) / 2;
        ulMicrosec /= 2;
        uDelayShift = DclBitHighest(ulMicrosec) - 1;
        ulDelayMicrosec = 1UL << uDelayShift;
    }
    ulDelayCount = (ulCount * ulDelayMicrosec + ulMicrosec - 1) / ulMicrosec;

    FFXPRINTF(2, ("normalize() ulDelayCount = %lU, ulDelayMicrosec = %lU\n",
               ulDelayCount, ulDelayMicrosec));
    return TRUE;
}

#endif


#endif
