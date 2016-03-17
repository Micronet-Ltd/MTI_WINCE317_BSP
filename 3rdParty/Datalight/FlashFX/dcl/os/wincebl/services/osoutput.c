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

    This module contains the Windows CE BootLoader OS Services default
    implementations for:

        DclOsOutputString()

    Should these routines need customization for your project, copy this
    module into your project directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: osoutput.c $
    Revision 1.4  2009/04/10 20:52:22Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.3  2008/08/10 06:21:50Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.2  2007/11/03 23:31:38Z  Garyp
    Added the standard module header.
    Revision 1.1  2005/10/22 05:55:48Z  Pauli
    Initial revision
    Revision 1.2  2005/10/22 06:55:47Z  garyp
    Updated DclOsOutputString() to take a new pUserData parameter.
    Revision 1.1  2005/10/02 04:58:00Z  Garyp
    Initial revision
    Revision 1.3  2005/08/21 11:39:15Z  garyp
    Documentation update.
    Revision 1.2  2005/08/03 19:14:28Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/04/10 03:23:44Z  pauli
    Initial revision
    Revision 1.1  2005/04/10 03:23:44Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <dcl.h>

/*  Serial output function for the MS Ethernet boot loader...
*/
extern void EdbgOutputDebugString(const char *, ...);


/*-------------------------------------------------------------------
    Public: DclOsOutputString()

    Display a string on the output device.

    Parameters:
        pUserData - Must be NULL at this time
        pszStr    - A pointer to the null-terminated string to display

    Return Value:
        None.
-------------------------------------------------------------------*/
#define BUFFLEN 256
void DclOsOutputString(
    void           *pUserData,
    const char     *pszStr)
{
    (void)pUserData;

    DclAssert(pszStr);

    EdbgOutputDebugString(pszStr);

#if 0

    /*  This logic may be used in an OAL implementation that
        has a functional RETAILMSG() macro.
    */
    TCHAR           awcBuffer[BUFFLEN];
    int             i;

    /*  This code converts an ANSI string into a Unicode string.
        It is necessary because MultiByteToWideChar() is not available
        at this stage in the CE boot process, and there is apparently
        not any other kernel function available to do this.
    */
    for(i = 0; i < BUFFLEN; i++)
    {
        awcBuffer[i] = (TCHAR) pszStr[i];

        if(!pszStr[i])
            break;
    }

    RETAILMSG(1, (awcBuffer));
#endif

    return;
}
