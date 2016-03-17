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

        DclOsInputChar()

    Should this routine need customization for your project, copy this
    module into the Project Directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osinput.c $
    Revision 1.6  2010/02/01 22:35:04Z  brandont
    Corrected warning for truncated variable.
    Revision 1.5  2009/02/07 23:54:45Z  garyp
    Merge from the v4.0 branch.  Renamed DclOsGetChar() to DclOsInputChar(),
    and extended the interface to accommodate more sophisticated input
    systems.
    Revision 1.4  2007/11/03 23:31:30Z  Garyp
    Added the standard module header.
    Revision 1.3  2006/12/12 20:57:37Z  Garyp
    Modified the existence of the DclOsGetChar() function to be dependent
    on the DCL_OSFEATURE_CONSOLEINPUT setting.
    Revision 1.2  2006/02/09 18:27:59Z  Pauli
    Updated comments, provided a functional default implementation.
    Revision 1.1  2005/10/02 03:57:38Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <stdio.h>

#include <dcl.h>

#if DCL_OSFEATURE_CONSOLEINPUT


/*-------------------------------------------------------------------
    Public: DclOsInputChar()

    This is the workhorse function for obtaining character input.

    When this function is used by common, OS independent code, it
    <must> be assumed that the OS interface may wait for a character,
    or even the [Enter] key before returning from this function.

    When this function is used from OS specific code, where the OS
    characteristics are known, it may return immediately with a
    DCLSTAT_INPUT_IDLE status.

    Parameters:
        pContext - A pointer to any context data which may have been
                   specified when DclInputFunction() was called.
        puChar   - A pointer to a buffer to receive the character.
                   The contents of this field will only be modified
                   if DCLSTAT_SUCCESS is returned.
        nFlags   - This parameter is reserved for future use and must
                   be 0.

    Return Value:
        Returns one of the following DCLSTATUS values:

        DCLSTAT_SUCCESS         - A character was returned
        DCLSTAT_INPUT_DISABLED  - The input interface is disabled
        DCLSTAT_INPUT_IDLE      - There is no input ready
        DCLSTAT_INPUT_TERMINATE - The process/program is terminating
-------------------------------------------------------------------*/
DCLSTATUS DclOsInputChar(
    void       *pContext,
    D_UCHAR    *puChar,
    unsigned    nFlags)
{
    (void)pContext;
    (void)nFlags;

    DclAssert(puChar);
    DclAssert(nFlags == 0);

    *puChar = (D_UCHAR)getchar();

    return DCLSTAT_SUCCESS;
}


#endif  /* DCL_OSFEATURE_CONSOLEINPUT */


