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

       DclOsHalt()

    Should this routine need customization for your project, copy this
    module into the Project Directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: oshalt.c $
    Revision 1.5  2009/04/10 02:21:00Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.4  2007/11/03 23:50:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2007/04/01 22:07:47Z  Garyp
    Removed "DebugBreak" logic which is now implemented in osdebug.c.
    Revision 1.2  2006/12/30 18:51:26Z  Garyp
    Updated to only attempt to enter the debugger if the user presses 'D'
    within 10 seconds.  Otherwise the program will be terminated.
    Revision 1.1  2006/02/09 23:10:14Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclOsHalt()

    Halt the system.

    This function provides the back-end part of the DclAssert()
    functionality that halts execution.

    Depending on the OS implementation, this function may only
    halt execution of the currently executing thread.

    This function must not return.

    Parameters:
        pszMessage  - A pointer to a null-terminated message string
        ulError     - The error code

    Return Value:
        This function does not return.
-------------------------------------------------------------------*/
void DclOsHalt(
    const char *pszMessage,
    D_UINT32    ulError)
{
    void(*pNull)(void) = NULL;

    DclPrintf("Program Terminated: \"%s-%lU\"\n", pszMessage, ulError);

    /*  Exit the application
    */
    exit(ulError);

    /*  We should not ever get to get to this point, but if we do,
        cause a fault.
    */
    pNull();
}

