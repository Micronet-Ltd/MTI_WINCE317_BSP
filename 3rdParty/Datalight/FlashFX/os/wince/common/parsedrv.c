/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2008 Datalight, Inc.
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

    This module contains routines used to parse drive specification strings.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: parsedrv.c $
    Revision 1.3  2008/05/29 20:42:06Z  garyp
    Merged from the WinMobile branch.
    Revision 1.2.1.2  2008/05/29 20:42:06Z  garyp
    Added a missing const declaration.
    Revision 1.2  2007/11/03 23:50:10Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2005/10/20 02:49:12Z  Pauli
    Initial revision
    Revision 1.2  2005/10/20 03:49:11Z  garyp
    Changed some D_CHAR buffers to be plain old char.
    Revision 1.1  2005/10/02 03:11:46Z  Garyp
    Initial revision
    Revision 1.2  2005/08/03 17:19:34Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/04 01:20:56Z  pauli
    Initial revision
    Revision 1.3  2004/12/30 23:34:38Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2004/01/03 02:13:52Z  garys
    Merge from FlashFXMT
    Revision 1.1.1.3  2004/01/03 02:13:52  garyp
    Modified to use DRIVERPREFIX rather than hardcoding "DSK".
    Revision 1.1.1.2  2003/12/02 19:59:24Z  garyp
    Fixed the file number.
    Revision 1.1  2003/02/17 02:54:26Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <diskio.h>
#include <flashfx.h>
#include <fxdriver.h>
#include <ffxwce.h>
#include <oecommon.h>

static const char szDriveNumberFormats[] = DRIVERPREFIX "1:";


/*-------------------------------------------------------------------
    Parameters:
       None

    Notes:
       Returns a string listing the possible drive letter formats.

    Returns:
       A pointer to the null terminated string.
-------------------------------------------------------------------*/
const char * FfxGetDriveFormatString(void)
{
    return &szDriveNumberFormats[0];
}


/*-------------------------------------------------------------------
    Parameters:
       pszString     - ASCIIZ string designating the device to use.
                       This is usually DSK1: or DSK2:.
       puDriveNumber - A pointer to where the drive number is
                       returned.
       puStringLen   - A pointer to where the string length is
                       returned.  Supply a null pointer if this
                       value is not needed.

    Notes:
       Determines if the given drive designation is valid.  It may
       be in any one of the following forms, case - insensitive:
          DSK1:

    Returns:
       TRUE if successful, else FALSE
       *puDriveNumber - Set to the zero relative drive number or D_UINT16_MAX
       *puStringLen   - Set to the length of the drive number string
-------------------------------------------------------------------*/
D_BOOL FfxParseDriveNumber(
    const char *pszString,
    D_UINT16   *puDriveNumber,
    D_UINT16   *puStringLen)
{
    char        cBuffer[6];

    *puDriveNumber = D_UINT16_MAX;

    DclStrNCpy(cBuffer, pszString, 6);

    if(cBuffer[5] != 0 &&
       cBuffer[5] != ' ' && cBuffer[5] != '/' && cBuffer[5] != '\t')
        return FALSE;

    if((cBuffer[3] < '1') || (cBuffer[3] > '9'))
        return FALSE;

    cBuffer[3] = ' ';           /* zap the number so the comparison works */
    cBuffer[5] = 0;             /* ensure null termination */

    if(DclStrLen(cBuffer) != 5)
        return FALSE;

    if(DclStrICmp(cBuffer, DRIVERPREFIX " :") != 0)
        return FALSE;

    /*  The DSKn: values range from 1 to 9, however we return
        values that are relative to 0.
    */
    *puDriveNumber = DclAtoI(&pszString[3]) - 1;

    if(puStringLen)
        *puStringLen = 5;

    return TRUE;
}
