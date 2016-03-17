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

    This module contains the mapping functions for NOR flash.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: normap.c $
    Revision 1.9  2011/12/20 21:34:33Z  johnb
    Fixed to properly handle reserved space.
    Revision 1.8  2009/07/18 01:08:28Z  garyp
    Merged from the v4.0 branch.  Headers updated.
    Revision 1.7  2009/04/06 14:28:20Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.6  2008/02/03 01:56:20Z  keithg
    comment updates to support autodoc.
    Revision 1.5  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2007/08/02 23:15:36Z  timothyj
    Changed units of reserved space and maximum size to be in terms
    of KB instead of bytes.
    Revision 1.3  2007/03/09 21:35:51Z  billr
    Use forward slashes in #include directives.
    Revision 1.2  2006/08/22 18:47:55Z  Garyp
    Updated to reference the proper reserved blocks field.
    Revision 1.1  2006/08/20 02:41:00Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdevapi.h>
#include <fxdriver.h>
#include <fimdev.h>
#include <../devmgr/devmgr.h>


/*-------------------------------------------------------------------
    Public: FfxFimNorWindowCreate

    This function is used by NOR FIMs to map a window into the
    flash array during the FIM mount process.

    During the FIM mount process, this function allows windows
    to be mapped into the flash up the the length specified
    by the FFXFIMBOUNDS.ulMaxScanTotalKB value returned by the
    function FfxDevGetArrayBounds().  Windows may be mapped into
    reserved areas, however the mount process may not write
    anything to the flash.

    NOTE: This function uses the "create" verb because it is
          called exclusively from the FIM "create" functions
          ("mount" for old-style NOR FIMs).

    Parameters:
        hDev     - The device handle.
        ulOffset - The byte offset into the flash array.
        pBounds  - A pointer to the FFXFIMBOUNDS structure to use.
        ppMedia  - A pointer to the location in which to store the
                   flash window pointer.

    Return:
        Returns the window length in bytes which is accessible
        using the returned pointer, or zero if an error occurred.
-------------------------------------------------------------------*/
D_UINT32 FfxFimNorWindowCreate(
    FFXDEVHANDLE        hDev,
    D_UINT32            ulOffset,
    const FFXFIMBOUNDS *pBounds,
    volatile void     **ppMedia)
{
    D_UINT32            ulSize;

    DclAssert(hDev);
    DclAssert(pBounds);
    DclAssert(ppMedia);

    DclAssert(pBounds->ulMaxScanTotalKB);

    /*  Mapping is not allowed past the maximum size we are allowed
        to scan.  FIM creation will attempt this, so fail gracefully...
    */
    if(ulOffset / 1024UL >= pBounds->ulMaxScanTotalKB)
        return 0UL;

    ulSize = FfxHookMapWindow(hDev, ulOffset, pBounds->ulMaxScanTotalKB * 1024UL, ppMedia);

    /*  Validate that the project hook function returned a legitimate
        size to us.
    */
    DclAssert(ulSize);
    DclAssert((ulOffset + ulSize) / 1024UL <= pBounds->ulMaxScanTotalKB);

    return ulSize;
}


/*-------------------------------------------------------------------
    Public: FfxFimNorWindowMap()

    This function is used by NOR FIMs to map a window into the
    flash array.  This function may only be used after the FIM
    device has been created.  During the FIM creation process,
    the function FfxFimNorWindowCreate() should be used.

    After the FIM has mounted, this function allows windows
    to be mapped only onto areas of the flash which are a part
    of the given device.  Windows cannot be mapped into
    reserved areas.

    Parameters:
        hDev     - The device handle
        ulOffset - The byte offset into the flash array
        ppMedia  - A pointer to the location in which to store the
                   flash window pointer.

    Return Value:
        Returns the window length in bytes which is accessible
        using the returned pointer, or zero if an error occurred.
-------------------------------------------------------------------*/
D_UINT32 FfxFimNorWindowMap(
    FFXDEVHANDLE        hDev,
    D_UINT32            ulOffset,
    volatile void     **ppMedia)
{
    D_UINT32            ulReservedLow;
    D_UINT32            ulDevLength;
    D_UINT32            ulSize;
    D_UINT32            ulMaxLen;
    FFXFIMDEVICE       *pFimDev;

    DclAssert(hDev);
    DclAssert((*hDev)->hFimDev);
    DclAssert(ppMedia);

    /*  For readability...
    */
    pFimDev = (*hDev)->hFimDev;

    ulReservedLow = pFimDev->ulReservedBlocks * pFimDev->pFimInfo->ulBlockSize;
    ulDevLength = pFimDev->ulTotalBlocks * pFimDev->pFimInfo->ulBlockSize;

    DclAssert(ulDevLength);

    /*  Mapping is not allowed in the reserved area.  After the FIM has
        been created, all offsets which the Device Manager passes into
        the FIM will have been adjusted for any low reserved space.
        Therefore, this should never happen...
    */
    DclAssert(ulOffset >= ulReservedLow);

    ulMaxLen = ulReservedLow + ulDevLength;

    /*  Mapping is not allowed past the end of the device
    */
    DclAssert(ulOffset < ulMaxLen);

    ulSize = FfxHookMapWindow(hDev, ulOffset, ulMaxLen, ppMedia);

    /*  Validate that the project hook function returned a legitimate
        size to us.
    */
    DclAssert(ulSize);
    DclAssert(ulOffset + ulSize <= ulMaxLen);

    return ulSize;
}


