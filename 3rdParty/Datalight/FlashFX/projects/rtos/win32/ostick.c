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

    This module contains the Win32 OS Services default implementations for:

        DclOsTickCount()
        DclOsTickModulus()
        DclOsTickResolution()

    Should these routines need customization for your project, copy this
    module into the Project Directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ostick.c $
    Revision 1.4  2009/04/10 20:55:03Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.3  2008/01/30 03:05:55Z  Garyp
    Documentation updated -- no functional changes.
    Revision 1.2  2007/11/03 23:50:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2006/02/09 22:26:54Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclOsTickCount()

    Get the current system tick count.  The tick resolution can be
    determined by calling DclOsTickResolution().  It must be assumed
    that the tick count will roll over at some point.

    Parameters:
        None.

    Return Value:
        Returns the current system tick value.
-------------------------------------------------------------------*/
D_UINT32 DclOsTickCount(void)
{
    return GetTickCount();
}


/*-------------------------------------------------------------------
    Public: DclOsTickModulus()

    Get the system tick modulus.  This is the number of legitimate
    tick values.  If the tick rolls over, then the tick modulus must
    be zero.

    Parameters:
        None.

    Return Value:
        Returns the system tick modulus.
-------------------------------------------------------------------*/
D_UINT32 DclOsTickModulus(void)
{
    /*  Ticker wraps so return a modulus of 0
    */
    return 0L;
}


/*-------------------------------------------------------------------
    Public: DclOsTickResolution()

    Get the system tick resolution in microseconds per tick.

    Parameters:
        None.

    Return Value:
        Returns the system tick resolution.
-------------------------------------------------------------------*/
D_UINT32 DclOsTickResolution(void)
{
    /*  Ticks on Win 32 are in milliseconds
    */
    return 1000UL;
}

