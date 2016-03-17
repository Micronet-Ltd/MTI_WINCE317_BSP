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

    This module contains the Windows CE OS Services default implementations
    for mananging high resolution timers.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    the master product's make file to build the new module.

    NOTE:  The code in this file is virtually identical to that in the Win32
           OS Layer module.  Any maintenance done in this module should very
           likely be done in the Win32 module as well.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: oshrtick.c $
    Revision 1.7  2009/06/27 23:02:43Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.6  2009/04/10 17:54:42Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.5  2009/04/10 02:01:32Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.4  2007/11/03 23:31:36Z  Garyp
    Added the standard module header.
    Revision 1.3  2007/05/11 03:06:08Z  Garyp
    Changed to to use different 64-bit macros and make no assumptions that 
    native 64-bit support is enabled.  Allows us to force emulation on for 
    testing purposes.
    Revision 1.2  2006/10/05 23:43:16Z  Garyp
    Modified to use a corrected macro name.
    Revision 1.1  2006/07/02 05:28:42Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>

#define NANOS_PER_SECOND    (1000000000UL)  /* must fit into a D_UINT32 */
#define PICOS_PER_NANO            (1000UL)  /* must fit into a D_UINT32 */

static void Initialize(void);

static D_UINT64 ullResolution;

#define ONE_TIME_INITIALIZATION()                   \
    if(!DclUint64GreaterUint32(&ullResolution, 0))  \
        Initialize();


/*-------------------------------------------------------------------
    Public: DclOsHighResTickCount()

    Return the current high resolution tick count.  The tick resolution
    can be determined by calling DclOsHighResTickResolution().  It must
    be assumed that the tick count will roll over at some point.

    Parameters:
        None.

    Return Value:
        Returns the current high resolution tick value.
-------------------------------------------------------------------*/
D_UINT64 DclOsHighResTickCount(void)
{
    LARGE_INTEGER   now;
    D_UINT64        ullTick;

    if(!QueryPerformanceCounter(&now))
    {
        DclError();

        DclUint64AssignUint32(&ullTick, 0);

        return ullTick;
    }

    DclUint64AssignHiLo(&ullTick, now.HighPart, now.LowPart);

    return ullTick;
}


/*-------------------------------------------------------------------
    Public: DclOsHighResTickModulus()

    Return the high resolution tick modulus.  This is the number
    of legitimate tick values.  If the tick rolls over, then the
    tick modulus must be zero.

    Parameters:
        None.

    Return Value:
        Returns the high resolution tick modulus.
-------------------------------------------------------------------*/
D_UINT64 DclOsHighResTickModulus(void)
{
    D_UINT64    ullModulus;

    /*  Presumably the performance counters wrap...
    */
    DclUint64AssignUint32(&ullModulus, 0);

    return ullModulus;
}


/*-------------------------------------------------------------------
    Public: DclOsHighResTickResolution()

    This function calculates the high-res tick resolution in
    picoseconds per tick (one trillionth of a second).

    Parameters:
        None.

    Return Value:
        Returns the high res tick resolution in picoseconds per tick.
-------------------------------------------------------------------*/
D_UINT64 DclOsHighResTickResolution(void)
{
    ONE_TIME_INITIALIZATION();

    return ullResolution;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void Initialize(void)
{
    LARGE_INTEGER   perfFreq;
    D_UINT64        ullTemp;

    if(!QueryPerformanceFrequency(&perfFreq))
    {
        DclError();
        return;
    }

    DCLPRINTF(1, ("DclOsHighResTickResolution() Frequency is %llU ticks per second\n", perfFreq.QuadPart));

    /*  We need to use picoseconds for resolution to accomodate ultra fast
        desktop machines where the timestamps are directly based off the
        instruction clock.  This means a 3.4Ghz system will report 0ns
        per tick, which plainly will not do, but 294 picoseconds per tick,
        which is fine.

        This complicates things because on slower machines, picoseconds
        per tick might be a 64-bit number and our emulated 64-bit math
        macros cannot multiply or divide by a 64-bit number.  Of course
        on a Windows CE, in which this particular code is always going
        to run, we will always be able to do real 64-bit math.  However
        for testing purposes we like to force the emulated stuff to be
        used now and then.  Therefore, the most robust choice is to choose
        to enforce the same rules regarding 64-bit math in this code that
        we do elsewhere.

        To solve this problem, if our frequency is larger than D_UINT32_MAX,
        divide both sides of our equation by 2 until it is small enough.
    */

    DclUint64AssignHiLo(&ullTemp, perfFreq.HighPart, perfFreq.LowPart);
    DclUint64AssignUint32(&ullResolution, NANOS_PER_SECOND);
    DclUint64MulUint32(&ullResolution, PICOS_PER_NANO); /* convert nanos to picos */

    while(DclUint64GreaterUint32(&ullTemp, D_UINT32_MAX))
    {
        DclUint64DivUint32(&ullTemp, 2);
        DclUint64DivUint32(&ullResolution, 2);
    }

    DclAssert(DclUint64GreaterUint32(&ullTemp, 0));

    /*  Now that we have (if necessary) scaled things to the point where we
        can do this division, calculate a final picoseconds per tick value.
    */
    DclUint64DivUint32(&ullResolution, DclUint32CastUint64(&ullTemp));

    DCLPRINTF(1, ("DclOsHighResTickResolution() Resolution is %llU picoseconds per tick\n", ullResolution));

    /*  Better not be zero...
    */
    DclAssert(DclUint64GreaterUint32(&ullResolution, 0));

    return;
}



