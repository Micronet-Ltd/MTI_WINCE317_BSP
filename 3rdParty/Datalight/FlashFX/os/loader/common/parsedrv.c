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

    This module contains routines used to parse drive specification strings.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: parsedrv.c $
    Revision 1.5  2009/07/18 18:43:59Z  garyp
    Merged from the v4.0 branch.  Added FfxOsDeviceNameToDiskNumber().
    Revision 1.4  2009/04/06 18:01:09Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.3  2008/05/29 20:41:53Z  garyp
    Merged from the WinMobile branch.
    Revision 1.2.1.2  2008/05/29 20:41:53Z  garyp
    Added a missing const declaration.
    Revision 1.2  2007/11/03 23:49:53Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2006/01/25 17:05:28Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <oecommon.h>


static const char szDriveNumberFormats[] = "C:, /D2, 2";


/*-------------------------------------------------------------------
    Protected: FfxGetDriveFormatString()

    Returns a string listing the possible drive letter formats for
    this OS abstraction.

    Parameters:
        None

    Return Value:
        A pointer to the null terminated string.
-------------------------------------------------------------------*/
const char * FfxGetDriveFormatString(void)
{
    return &szDriveNumberFormats[0];
}


/*-------------------------------------------------------------------
    Protected: FfxParseDriveNumber()

    Determines if the specified drive designation is valid for the
    given OS abstraction.

    Parameters:
        pszString     - A pointer to the null-terminated drive.
        puDriveNumber - A pointer to a location in which to store the
                        drive number.
        puStringLen   - A pointer to location in which to store the
                        string length.  Supply a null pointer if this
                        value is not needed.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL FfxParseDriveNumber(
    const char     *pszString,
    D_UINT16       *puDriveNumber,
    D_UINT16       *puStringLen)
{
    D_UINT16        uThisUnit;
    D_UINT16        uStringLen = 0;

    *puDriveNumber = D_UINT16_MAX;

    /*  Get the drive letter or number
    */
    if((pszString[0] <= '9') && (pszString[0] >= '0'))
    {
        if(pszString[0] == '8')
        {
            uThisUnit = 0x80 + (pszString[1] - '0');

            uStringLen = 2;
        }
        else
        {
            if(pszString[1] != 0 &&
               pszString[1] != ' ' &&
               pszString[1] != '/' && pszString[1] != '\t')
                return FALSE;

            uStringLen = 1;

            uThisUnit = DclAtoI(pszString);
        }
    }
    else if(pszString[0] == '/')
    {
        if(pszString[1] != 'd' && pszString[1] != 'D')
            return FALSE;

        if((pszString[2] > '9') || (pszString[2] < '0'))
            return FALSE;

        if(pszString[3] != 0 &&
           pszString[3] != ' ' && pszString[3] != '/' && pszString[3] != '\t')
            return FALSE;

        uStringLen = 3;

        uThisUnit = DclAtoI(&pszString[2]);
    }
    else
    {
        D_CHAR          ch;

        /*  Must be in X: form
        */
        if(pszString[1] != ':')
            return FALSE;

        /*  Make sure it is a valid character
        */
        ch = pszString[0];
        if(ch >= 'a')
            ch += 'A' - 'a';
        if(ch < 'A' || ch > 'Z')
            return FALSE;

        uStringLen = 2;

        /*  Make it zero based
        */
        uThisUnit = ch - 'A';
    }


    /*  A little more validity checking
    */
    if(uThisUnit > 25)
        return FALSE;

    if(puStringLen)
        *puStringLen = uStringLen;

    *puDriveNumber = uThisUnit;

    return TRUE;
}


/*----------------------------------------------------------------------
    Protected: FfxOsDeviceNameToDiskNumber()

    This function determines the Disk number given an OS specific
    device name.

    Parameters:
        pszDevName  - The null-terminated device name.

    Return Value:
        Returns the FlashFX Disk number if successful, or UINT_MAX if
        the name is invalid or the Disk does not exist.
----------------------------------------------------------------------*/
unsigned FfxOsDeviceNameToDiskNumber(
    const char *pszDevName)
{
    D_UINT16    uDiskNum;

    DclAssert(pszDevName);

    if(!FfxParseDriveNumber(pszDevName, &uDiskNum, NULL))
        return UINT_MAX;

    return (unsigned)uDiskNum;
}



