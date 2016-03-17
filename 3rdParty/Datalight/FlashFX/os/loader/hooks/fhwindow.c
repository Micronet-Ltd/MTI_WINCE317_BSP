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

    This module contains the default OS Layer implementations for:

       FfxHookMapWindow()

    Should these routines need customization for your project, copy this
    module into your project directory, make your changes, and modify
    ffxproj.mak to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhwindow.c $
    Revision 1.5  2009/07/18 01:24:20Z  garyp
    Merged from the v4.0 branch.  Headers updated.
    Revision 1.4  2009/04/08 19:50:40Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.3  2008/03/23 04:25:27Z  Garyp
    Modified to use some slightly relocated DEVICE and DISK configuration info
    Revision 1.2  2007/11/03 23:49:54Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2006/08/29 00:07:52Z  brandont
    Initial revision
    Revision 1.5  2006/08/29 00:07:52Z  pauli
    Added the new hooks function FfxHookMapWindow to replace the
    old WindowMap/WindowSize routines.
    Revision 1.4  2006/02/14 01:11:44Z  Pauli
    Updated headers.
    Revision 1.3  2006/02/10 04:13:46Z  Garyp
    No longer use the EXTMEDIAINFO structure.
    Revision 1.2  2006/02/09 19:15:45Z  Pauli
    Modified to do ptr math rather than assume 32-bit pointer.
    Revision 1.1  2005/10/06 05:47:08Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <ffxdrv.h>


/*-------------------------------------------------------------------
    Public: FfxHookMapWindow()

    This function is used by NOR FIMs to map a window into the
    flash array.  The window may be anywhere in the flash array
    up the the length specified by ulMaxLen.  The function must
    return a valid pointer which may be used to access the
    flash, as well as a maximum length in bytes which may be
    accessed using the returned pointer.

    If the flash array is linearly addressed and requires no
    special mapping, the entire length, minus the supplied
    offset can be returned, along with an appropriately
    constructed pointer.

    If the flash hardware has particular windowing requirements,
    the returned window size must be adjusted accordingly.

    Note that this function may be asked to map areas within
    the reserved space in the flash array (during the mount
    process).  This is expected and no special considerations
    need to be made.

    Parameters:
        hDev     - The device handle
        ulOffset - The byte offset into the flash array
        ulMaxLen - The maximum amount of flash which is addressable.
                   The supplied offset, plus the returned window
                   length may not exceed this value.
        ppMedia  - A pointer to the location in which to store the
                   flash window pointer.

    Return Value:
        Returns the window length in bytes which is accessible
        using the returned pointer, or zero if an error occurred.
-------------------------------------------------------------------*/
D_UINT32 FfxHookMapWindow(
    FFXDEVHANDLE        hDev,
    D_UINT32            ulOffset,
    D_UINT32            ulMaxLen,
    volatile void     **ppMedia)
{
    D_BUFFER           *pFlashMedia;

    DclAssert(hDev);
    DclAssert(ppMedia);
    DclAssert(ulMaxLen);
    DclAssert(ulMaxLen > ulOffset);

    /*  Perform any custom mapping code here if needed.
    */
    pFlashMedia = (*hDev)->Conf.DevSettings.pBaseFlashAddress;
    pFlashMedia += ulOffset;

    /*  Stuff the pointer to the flash window
    */
    *ppMedia = pFlashMedia;

    /*  Return the amount of flash which can be accessed from
        the returned flash window pointer.
    */
    return ulMaxLen - ulOffset;
}


