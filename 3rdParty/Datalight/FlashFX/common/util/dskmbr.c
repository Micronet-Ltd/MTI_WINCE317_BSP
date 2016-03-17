/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This module contains code for build Master Boot Records.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dskmbr.c $
    Revision 1.15  2011/02/09 00:57:55Z  garyp
    Modified to use some refactored disk partition manipulation code which
    has moved from FlashFX into DCL.  Not functionally changed otherwise.
    Revision 1.14  2009/07/22 00:10:41Z  garyp
    Merged from the v4.0 branch.  Minor updates to deal with some refactored
    FAT code.
    Revision 1.13  2009/04/01 15:36:09Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.12  2009/03/20 20:25:23Z  billr
    Resolve bug 2275: FAT/MBR format code appears to be broken wrt FAT12,
    FAT16, BIGFAT choices.
    Revision 1.11  2009/02/09 01:20:35Z  garyp
    Updated to include dlpartid.h.
    Revision 1.10  2008/01/13 07:27:12Z  keithg
    Function header updates to support autodoc.
    Revision 1.9  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.8  2007/09/27 00:03:34Z  jeremys
    Renamed a header file.
    Revision 1.7  2006/10/18 00:05:44Z  Garyp
    Modified so MBR support can be stripped out of the product.
    Revision 1.6  2006/10/11 19:11:06Z  Garyp
    Updated to use some renamed symbols.
    Revision 1.5  2006/10/08 20:32:20Z  Garyp
    Variable renamed.
    Revision 1.4  2006/10/06 00:16:42Z  Garyp
    Modified to use the DCL specific byte-ordering macros.
    Revision 1.3  2006/02/21 23:18:16Z  Garyp
    Minor type changes.
    Revision 1.2  2006/01/11 02:06:01Z  Garyp
    Documentation and debug code changes only -- nothing functional.
    Revision 1.1  2005/10/02 01:58:26Z  Pauli
    Initial revision
    Revision 1.9  2004/12/30 17:32:45Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.8  2004/12/19 01:12:07Z  GaryP
    Updated to encode a FlashFX signature and sector length in the MBR.
    Revision 1.7  2004/09/23 23:09:34Z  billr
    Fix compiler warning.
    Revision 1.6  2004/09/22 17:52:18Z  GaryP
    Updated to use DSK_SECTORLEN.
    Revision 1.5  2004/09/17 02:27:25Z  GaryP
    Minor function renaming exercise.
    Revision 1.4  2004/08/06 03:13:27Z  GaryP
    Updated to only include fat.h if it is necessary.
    Revision 1.3  2004/07/23 23:57:42Z  GaryP
    Added support for specifying a default file system.
    Revision 1.2  2003/04/13 02:38:38Z  garyp
    Header file changes.
    Revision 1.1  2003/03/26 01:02:54Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_MBRFORMAT

#include <diskapi.h>
#include <dlpartid.h>
#include <dlmbr.h>

#if FFXCONF_FATSUPPORT
#include <dlfat.h>
#endif

#include "mbr.h"

/*  Set RESERVEMBRTRACK to FALSE to revert to the old FlashFX behavior
    where only the first sector was reserved for the MBR rather than the
    first track as is traditionally done (doing so will break CE).
*/
#define  RESERVEMBRTRACK   TRUE


/*-------------------------------------------------------------------
    Public: FfxMBRBuild()

    This function builds an MBR using the specified characteristics.

    Parameters:
        pSector         - The data buffer in which MBR will be built.
                          This buffer must be at least ulSectorLength
                          bytes long.
        uSides          - Number of sides in the media
        uSpt            - Number of sectors per track
        uCylinders      - Number of cylinders
        ulSectorLength - Number of bytes in a sector
        uFileSys       - The file system to use

    Return Value:
        Returns the starting sector value (the number of hidden
        sectors)
-------------------------------------------------------------------*/
D_UINT32 FfxMBRBuild(
    D_BUFFER           *pSector,
    D_UINT16            uSides,
    D_UINT16            uSpt,
    D_UINT16            uCylinders,
    D_UINT32            ulSectorLength,
    D_UINT16            uFileSys)
{
    DCLDISKPARTITION    pt;
    D_UINT32            ulStartSector;
    D_UINT32            ulTotalSectors;

    FFXPRINTF(1, ("FfxMBRBuild() Cylinders=%U Heads=%U SPT=%U BPS=%lU FileSys=%U\n",
               uCylinders, uSides, uSpt, ulSectorLength, uFileSys));

    DclAssert(uSides);
    DclAssert(uSpt);
    DclAssert(pSector);
    DclAssert(ulSectorLength >= DSK_SECTORLEN);

    /*  Move the default MBR code into the destination buffer
    */
    DclMemCpy(pSector, abMBRSector, sizeof(abMBRSector));

    ulTotalSectors = (D_UINT32)uSides * (D_UINT32)uSpt * (D_UINT32)uCylinders;

    /*  Set the hard coded fields
    */
    pt.bIsBootable = DCLMBR_PARTITION_BOOTABLE;

    /*  heads and cylinders are relative to zero
    */
    uSides--;
    uCylinders--;

  #if RESERVEMBRTRACK
    /*  Reserve the first track for the MBR
    */
    pt.bStartHead = 1;
    pt.bStartSector = 1;

    ulStartSector = uSpt;
  #else
    /*  Reserve the first sector for the MBR
    */
    pt.bStartHead =  0;
    pt.bStartSector =  2;

    ulStartSector = 1;
  #endif

    pt.bStartTrack =  0;
    pt.bEndHead = (D_BYTE) uSides;
    pt.bEndSector = (D_BYTE) uSpt;
    pt.bEndTrack = (D_BYTE) uCylinders;
    pt.bEndSector |= (D_BYTE) ((uCylinders & 0x0300) >> 2);

    /*  don't include the sector/track containing the MBR
    */
    ulTotalSectors -= ulStartSector;

    pt.bFileSysID = FSID_NONE;

  #if FFXCONF_FATSUPPORT
    if(uFileSys == FFX_FILESYS_FAT)
    {
        /*  Set the system ID
        */
        if (ulTotalSectors <= FAT12_MAXSECTORS)
            pt.bFileSysID = FSID_FAT12;
        else if (ulTotalSectors < MIN_DOSBIG_SECS)
            pt.bFileSysID = FSID_FAT16;
        else
            pt.bFileSysID = FSID_BIGFAT;
    }
  #endif

  #if FFXCONF_RELIANCESUPPORT
    if(uFileSys == FFX_FILESYS_RELIANCE)
    {
        pt.bFileSysID = FSID_RELIANCE;
    }
  #endif

  #if !(FFXCONF_FATSUPPORT || FFXCONF_RELIANCESUPPORT)
    (void)uFileSys;
  #endif

    DclAssert(pt.bFileSysID != FSID_NONE);

    pt.ulStartSector = ulStartSector;
    pt.ulSectorCount = ulTotalSectors;

    /*  Copy in the FFX specific signature and BytesPerSector values.
    */
    DclMemCpy(&pSector[MBR_OFFSET_FFXSIG], MBR_FFX_SIGNATURE, 4);
    DCLNE2LE(pSector + MBR_OFFSET_FFXBPS, (D_BUFFER*)&ulSectorLength, sizeof(ulSectorLength));

  #if D_DEBUG
    DclMBRPartitionDisplay(&pt, TRUE);
  #endif

    DclMBRPartitionStore(pSector + DCLMBR_PARTITION_OFFSET, &pt);

    FFXPRINTF(1, ("FfxMBRBuild() returning %lU hidden sectors\n", ulStartSector));

    /*  Return the number of hidden sectors preceding the partition.
    */
    return ulStartSector;
}


#endif  /* FFXCONF_MBRFORMAT */


