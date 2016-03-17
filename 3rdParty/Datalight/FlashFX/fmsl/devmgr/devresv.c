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

    This module contains logic for determining and applying the array bounds
    to use with respect to any reserved space which may be configured by the
    developer.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: devresv.c $
    Revision 1.16  2009/08/04 17:56:12Z  garyp
    Merged from the v4.0 branch.  Updated to support the new FFX_DEVCHIP()
    and FFX_CHIP_REMAINING features.
    Revision 1.15  2009/04/03 20:26:23Z  davidh
    Function header error corrections for AutoDoc.
    Revision 1.14  2009/04/01 20:04:03Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.13  2009/03/19 05:13:34Z  keithg
    Fixed buyg 2475, ulMaxArraySizeKB may be used while still set to -1;
    Removed some usused code.
    Revision 1.12  2009/02/09 02:24:44Z  garyp
    Merged from the v4.0 branch.  Cleaned up the logic to make the code
    more readable -- no functional changes.  Documentation updated.
    Revision 1.11  2008/03/22 17:15:56Z  Garyp
    Updated debug code.
    Revision 1.10  2008/01/13 07:28:02Z  keithg
    Function header updates to support autodoc.
    Revision 1.9  2007/11/03 23:49:35Z  Garyp
    Updated to use the standard module header.
    Revision 1.8  2007/08/02 22:18:54Z  timothyj
    Changed units of reserved space and maximum size to be in terms of KB
    instead of bytes.
    Revision 1.7  2007/08/01 21:52:07Z  timothyj
    Removed obsolete FFX_FLASHOFFSET.
    Revision 1.6  2007/03/01 19:59:37Z  timothyj
    Changed some bounds checking for LFA support to use width-nonspecific
    FFX_FLASHOFFSET_MAX in lieu of 32-bit specific D_UINT32_MAX.  Modified
    FfxDevApplyArrayBounds() to use take and return block index in lieu of
    byte offset.
    Revision 1.5  2006/09/08 22:08:49Z  Garyp
    Fixed a lame bug introduced in the previous rev.
    Revision 1.4  2006/09/07 00:41:43Z  Garyp
    Updated a function declaration and corrected some logic in
    FfxDevApplyArrayBounds() to avoid overflow.
    Revision 1.3  2006/08/21 22:41:26Z  Garyp
    Updated so that ulMaxScanTotal is set to D_UINT32_MAX if the total
    array size plus reserved size exceeds the size of a D_UINT32.
    Revision 1.2  2006/02/14 22:18:28Z  Garyp
    Removed dead code.
    Revision 1.1  2006/02/08 18:13:56Z  Garyp
    Initial revision
    Revision 1.3  2005/08/05 17:40:55Z  Garyp
    Updated to use revamped reserved space options which now allow reserved
    space at the top of the array.
    Revision 1.2  2005/08/03 19:17:20Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/28 16:07:54Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriverfwapi.h>
#include <fxdevapi.h>
#include <fimdev.h>


/*-------------------------------------------------------------------
    Protected: FfxDevGetArrayBounds()

    This function queries the options interface to get the bottom
    reserved space, top reserved space, and the maximum array size.

    Note that the various values must be modulo EraseZoneSize,
    however the values are not validated within this function
    because this function is typically called from within a FIM
    before it has ID'd the part and knows what flash is being
    used.

    Parameters:
        hDev    - The device handle
        pBounds - A pointer to the FFXFIMBOUNDS structure to fill

    Return Value:
        None
 -------------------------------------------------------------------*/
void FfxDevGetArrayBounds(
    FFXDEVHANDLE    hDev,
    FFXFIMBOUNDS   *pBounds)
{
    FFXDEVSETTINGS  DevSettings;

    DclAssert(hDev);
    DclAssert(pBounds);

    if(!FfxHookOptionGet(FFXOPT_DEVICE_SETTINGS, hDev, &DevSettings, sizeof DevSettings))
    {
        DevSettings.ulReservedBottomKB = 0;
        DevSettings.ulReservedTopKB = 0;
        DevSettings.ulMaxArraySizeKB = D_UINT32_MAX;
    }

    pBounds->ulReservedBottomKB = DevSettings.ulReservedBottomKB;
    pBounds->ulReservedTopKB    = DevSettings.ulReservedTopKB;
    pBounds->ulMaxArraySizeKB   = DevSettings.ulMaxArraySizeKB;

    /*  Calculate the maximum amount of flash to scan
    */
    pBounds->ulMaxScanTotalKB = pBounds->ulMaxArraySizeKB;

    /*  If the BottomReservedKB value is a magic chip number, then the
        MaxScanTotal feature cannot be used.
    */
    if(!FFX_ISDEVCHIP(pBounds->ulReservedBottomKB))
    {
    /*  Adjust max scan total to be the sum of MaxArraySize, ReservedLow,
        and ReservedHigh, so long as it does not exceed D_UINT32_MAX.
    */
        if( pBounds->ulMaxScanTotalKB != FFX_REMAINING &&
            pBounds->ulMaxScanTotalKB != FFX_CHIP_REMAINING &&
            D_UINT32_MAX - pBounds->ulMaxScanTotalKB > pBounds->ulReservedBottomKB)
        {
            pBounds->ulMaxScanTotalKB += pBounds->ulReservedBottomKB;
        }
        else
        {
            pBounds->ulMaxScanTotalKB = D_UINT32_MAX;
        }
    }

    /*  Should we really include the upper reserved space in the MaxScanTotal?
        If it is going to be reserved space, perhaps we should not even scan
        it at mount time...
    */
    if(D_UINT32_MAX - pBounds->ulMaxScanTotalKB > pBounds->ulReservedTopKB)
        pBounds->ulMaxScanTotalKB += pBounds->ulReservedTopKB;
    else
        pBounds->ulMaxScanTotalKB = D_UINT32_MAX;

    FFXPRINTF(1, ("FfxDevGetArrayBounds() ReservedLow =%lX KB MaxArraySize=%lX KB\n",
        pBounds->ulReservedBottomKB, pBounds->ulMaxArraySizeKB));

    FFXPRINTF(1, ("                       ReservedHigh=%lX KB ScanTotal   =%lX KB\n",
        pBounds->ulReservedTopKB, pBounds->ulMaxScanTotalKB));

    return;
}


/*-------------------------------------------------------------------
    Protected: FfxDevApplyArrayBounds()

    This function applies the specified flash array bounds to
    the physical flash array.

    TotalSize is reduced first by the bottom reserved space
    value, if any, and then by the top reserved space value.
    If the remaining space is still larger than the max array
    size value, it is reduced to max array size.

    Note that all the various size values must be evenly
    divisible by ulBlockSize, or an error will be returned.

    Note also that it is technically possible for this function
    to return a zero length, if the sum of the bottom and top
    reserved space exactly matches the flash size.  If it
    exceeds the flash size D_UINT32_MAX will be returned.

    Parameters:
        ulTotalBlocks - The total amount of flash found, in blocks.
        ulBlockSize   - The flash erase block size, must be an
                        integer multiple of 1024 bytes.
        pBounds       - A pointer to the FFXFIMBOUNDS structure
                        to use.

    Return Value:
        Returns the updated total array size if successful (which
        may be zero), or D_UINT32_MAX if a bounds error occurred.
-------------------------------------------------------------------*/
D_UINT32 FfxDevApplyArrayBounds(
    D_UINT32            ulTotalBlocks,
    D_UINT32            ulBlockSize,
    const FFXFIMBOUNDS *pBounds)
{
    D_UINT32            ulUsableBlocks;
    D_UINT32            ulBlockSizeKB = ulBlockSize / 1024UL;

    DclAssert(pBounds);
    DclAssert((ulBlockSize % 1024UL) == 0);
    DclAssert(!FFX_ISDEVCHIP(pBounds->ulReservedBottomKB));

    if(!ulBlockSize)
        return D_UINT32_MAX;

    if(pBounds->ulReservedBottomKB &&
        pBounds->ulReservedBottomKB % ulBlockSizeKB)
    {
        FFXPRINTF(1, ("Low reserved space must be evenly divisible by the erase block size\n"));
        return D_UINT32_MAX;
    }

    if(pBounds->ulReservedTopKB &&
        pBounds->ulReservedTopKB % ulBlockSizeKB)
    {
        FFXPRINTF(1, ("High reserved space must be evenly divisible by the erase block size\n"));
        return D_UINT32_MAX;
    }


    if(pBounds->ulMaxArraySizeKB &&
        pBounds->ulMaxArraySizeKB != FFX_REMAINING &&
        pBounds->ulMaxArraySizeKB != FFX_CHIP_REMAINING &&
        pBounds->ulMaxArraySizeKB % ulBlockSizeKB)
    {
        FFXPRINTF(1, ("Max array size must be FFX_REMAINING, FFX_CHIP_REMAINING, or\n"));
        FFXPRINTF(1, ("be evenly divisible by the erase block size\n"));
        return D_UINT32_MAX;
    }

    if(pBounds->ulReservedBottomKB / ulBlockSizeKB > ulTotalBlocks)
    {
        FFXPRINTF(1, ("Low reserved space is greater than the array size\n"));
        return D_UINT32_MAX;
    }

    if(pBounds->ulReservedTopKB / ulBlockSizeKB > ulTotalBlocks)
    {
        FFXPRINTF(1, ("High reserved space is greater than the array size\n"));
        return D_UINT32_MAX;
    }

    if(ulTotalBlocks - (pBounds->ulReservedBottomKB / ulBlockSizeKB) < pBounds->ulReservedTopKB / ulBlockSizeKB)
    {
        FFXPRINTF(1, ("Low plus high reserved space is greater than the array size\n"));
        return D_UINT32_MAX;
    }

    ulUsableBlocks = ulTotalBlocks;

    /*  The checks above ensure that there will be no underflow.
    */
    ulUsableBlocks -= (D_UINT32)(pBounds->ulReservedBottomKB / ulBlockSizeKB);
    ulUsableBlocks -= (D_UINT32)(pBounds->ulReservedTopKB / ulBlockSizeKB);

    /*  If after removing the reserved space, the remaining size is still
        larger than what the configuration parameters specify, reduce the
        usable size further.
    */
    if(pBounds->ulMaxArraySizeKB &&
        pBounds->ulMaxArraySizeKB != FFX_REMAINING &&
        pBounds->ulMaxArraySizeKB != FFX_CHIP_REMAINING &&
        (ulUsableBlocks > pBounds->ulMaxArraySizeKB / ulBlockSizeKB))
    {
        ulUsableBlocks = (D_UINT32)(pBounds->ulMaxArraySizeKB / ulBlockSizeKB);
    }

    FFXPRINTF(1, ("FfxDevApplyArrayBounds() OriginalBlocks=%lX ReservedLowKB=%lX UsableBlocks=%lX\n",
        ulTotalBlocks, pBounds->ulReservedBottomKB, ulUsableBlocks));

    if(!ulUsableBlocks)
        DclPrintf("FFX: NOTE: FfxDevApplyArrayBounds() found zero usable blocks!\n");

    return ulUsableBlocks;
}

