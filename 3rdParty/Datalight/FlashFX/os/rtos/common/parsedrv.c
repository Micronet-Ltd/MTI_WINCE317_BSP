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

    This module contains routines used to parse drive specification strings.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: parsedrv.c $
    Revision 1.5  2010/01/31 18:06:56Z  glenns
    Fix possible compiler warning about possible loss of data when
    doing implicit typecast from int to D_UINT16.
    Revision 1.4  2008/05/27 21:39:39Z  garyp
    Merged from the WinMobile branch.
    Revision 1.3.1.2  2008/05/27 21:39:39Z  garyp
    Added FfxOsDeviceNameToDiskNumber().
    Revision 1.3  2008/01/13 07:29:09Z  keithg
    Function header updates to support autodoc.
    Revision 1.2  2007/11/03 23:50:01Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2006/01/25 17:05:28Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <oecommon.h>


static const char szDriveNumberFormats[] = "C:, /D2, 2";


/*-------------------------------------------------------------------
    Public: FfxGetDriveFormatString

    Returns a string listing the possible drive letter formats.

    Parameters:
       None

    Return:
       A pointer to the null terminated string.
-------------------------------------------------------------------*/
const char * FfxGetDriveFormatString(void)
{
    return &szDriveNumberFormats[0];
}


/*-------------------------------------------------------------------
    Public: FfxParseDriveNumber

       Determines if the given drive letter, or number is a valid flash disk.
       It may be in any one of the following forms, case - insensitive:
          C: 2 /D2

    Parameters:
       pszString     - ASCIIZ string designating the device to use.
                       This is usually C:.
       puDriveNumber - A pointer to where the drive number is
                       returned.
       puStringLen   - A pointer to where the string length is
                       returned.  Supply a null pointer if this
                       value is not needed.

    Return:
       TRUE if successful, else FALSE
       *puDriveNumber - Set to the zero relative drive number or UINT
       *puStringLen   - Set to the length of the drive number string
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

            /*  Typecast is safe because the processing above makes sure that
                what is assigned to puDriveNumber is a number between 0 and 9,
                inclusive.
            */
            uThisUnit = (D_UINT16)DclAtoI(pszString);
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

        /*  Typecast is safe because the processing above makes sure that
            what is assigned to puDriveNumber is a number between 0 and 9,
            inclusive.
        */
        uThisUnit = (D_UINT16)DclAtoI(&pszString[2]);
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
    Public: FfxOsDeviceNameToDiskNumber

    This function determines the DISK number given an OS specific
    device name.

    Parameters:
        pszDevName  - The null-terminated device name.

    Returns:
        Returns the FlashFX DISK number if successful, or UINT_MAX if
        the name is invalid or the DISK does not exist.
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





