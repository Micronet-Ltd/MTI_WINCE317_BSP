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

    This module contains the RTOS OS Services default implementations for:

       DclOsSleep()

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: ossleep.c $
    Revision 1.10  2009/04/10 20:52:11Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.9  2008/09/09 23:51:47Z  billr
    Update copyright date.
    Revision 1.8  2008/09/09 23:25:22Z  billr
    Resolve bug 1958: DclOsSleep() is wrong for rtos and loader environments.
    Revision 1.7  2007/11/07 17:25:24Z  pauli
    Made #error messages strings to prevent macro expansion.
    Revision 1.6  2007/11/03 23:31:30Z  Garyp
    Added the standard module header.
    Revision 1.5  2007/06/22 05:43:04Z  timothyj
    Added #error for building this code in thread-capable RTOS's.  See also
    BZ #995.  Ported single-threaded version that will work fine for single-
    threaded RTOS's.
    Revision 1.4  2006/02/09 23:18:19Z  Pauli
    Removed reference to FlashFX.
    Revision 1.3  2006/02/09 22:59:43Z  Pauli
    Updated comments.
    Revision 1.2  2005/12/08 20:37:05Z  Garyp
    Updated to use some renamed symbols.
    Revision 1.1  2005/10/02 03:58:12Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

/*  In a multithreaded environment, the implementation should allow
    other threads to run while a thread sleeps.
*/
#if DCL_OSFEATURE_THREADS
#error "Default (single-threaded) DclOsSleep built in OS with threading capabilities."
#endif


/*-------------------------------------------------------------------
    Public: DclOsSleep()

    Sleep for the specified number of milliseconds.  This function
    is typically used when running under a multitasking OS.  It is
    called to allow other tasks or threads to run.

    Note that the behavior for a ulMilliseconds value of 0 is
    undefined.  In some environments this may release the remainder
    of the time slice, but that behavior must not be relied upon
    when this function is called from in portable code.

    Parameters:
        ulMilliseconds - The number of milliseconds to sleep.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclOsSleep(
    D_UINT32        ulMilliseconds)
{
    if (ulMilliseconds)
    {
        DCLTIMER t;

        DclTimerSet(&t, ulMilliseconds);
        while (!DclTimerExpired(&t))
            /* spin */ ;
    }
}
