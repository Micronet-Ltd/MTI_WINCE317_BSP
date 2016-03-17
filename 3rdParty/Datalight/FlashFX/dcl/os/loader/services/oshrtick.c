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

    This module contains the RTOS OS Services Layer default implementations
    for:

        DclOsHighResTickCount()
        DclOsHighResTickModulus()
        DclOsHighResTickResolution()

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    the master product's make file to build the new module.

    NOTE:  This is a stubbed implementation in which the HighRes timers
           use the standard system tick.  For systems in which a high
           resolution tick source is available, this code can be modified
           to use that functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: oshrtick.c $
    Revision 1.3  2009/04/10 02:01:09Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.2  2007/11/03 23:31:25Z  Garyp
    Added the standard module header.
    Revision 1.1  2006/07/06 00:49:44Z  brandont
    Initial revision
    Revision 1.1  2006/07/06 00:49:44Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#define PICOS_PER_MICRO             (1000000UL)


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
    D_UINT64    ullTick;

    DclUint64AssignUint32(&ullTick, DclOsTickCount());

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
    D_UINT32    ulMod;
    D_UINT64    ullModulus;

    ulMod = DclOsTickModulus();

    if(ulMod != 0)
    {
        /*  If the system tick wraps a a certain point other than
            the max, use that value.
        */
        DclUint64AssignUint32(&ullModulus, ulMod);
    }
    else
    {
        /*  The 32-bit mod value is zero, meaning that the ticker wraps
            at the 32-bit max value, so use D_UINT32_MAX+1.
        */
        DclUint64AssignUint32(&ullModulus, D_UINT32_MAX);
        DclUint64AddUint32(&ullModulus, 1UL);
    }

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
    D_UINT64    ullRes;

    DclUint64AssignUint32(&ullRes, DclOsTickResolution());
    DclUint64MulUint32(&ullRes, PICOS_PER_MICRO);

    return ullRes;
}


