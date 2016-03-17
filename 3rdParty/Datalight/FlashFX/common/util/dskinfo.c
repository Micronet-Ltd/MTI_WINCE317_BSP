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

    This module contains functions for determining the format of a disk.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dskinfo.c $
    Revision 1.17  2011/02/09 00:57:55Z  garyp
    Modified to use some refactored disk partition manipulation code which
    has moved from FlashFX into DCL.  Not functionally changed otherwise.
    Revision 1.16  2009/07/22 00:08:04Z  garyp
    Merged from the v4.0 branch.  Updated to use some reorganized FAT 
    functionality.  Eliminated an unused parameter for FfxMBRLoadPartitions().
    Modified to use the cleaned up and refactored BPB structures.
    Revision 1.15  2009/04/01 15:35:40Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.14  2009/02/09 01:20:34Z  garyp
    Updated to include dlpartid.h.
    Revision 1.13  2008/03/24 19:00:53Z  Garyp
    Documentation fixes.
    Revision 1.12  2008/01/13 07:27:11Z  keithg
    Function header updates to support autodoc.
    Revision 1.11  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.10  2007/09/27 00:03:11Z  jeremys
    Renamed a header files and function names.
    Revision 1.9  2006/10/16 21:06:21Z  Garyp
    Modified so MBR support can be stripped out of the product.
    Revision 1.8  2006/03/21 17:47:48Z  Pauli
    Added FAT support condition around locals only used by FAT code.
    Revision 1.7  2006/03/18 02:21:29Z  Garyp
    Fixed to no longer query the driver code for the desired physical sector
    size when a reliance boot sector is found.
    Revision 1.6  2006/03/17 22:41:30Z  Garyp
    Modified to allow FAT boot records for non-512-byte disks to be recognized.
    Revision 1.5  2006/02/23 04:26:06Z  Garyp
    Debug code updated.
    Revision 1.4  2006/02/20 02:53:44Z  Garyp
    Minor type changes.
    Revision 1.3  2006/01/11 02:05:09Z  Garyp
    Documentation and debug code changes only -- nothing functional.
    Revision 1.2  2006/01/05 03:36:33Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.1  2005/10/02 01:58:26Z  Pauli
    Initial revision
    Revision 1.20  2005/05/20 03:04:22Z  garyp
    Fixed so that if our last resort in deriving disk parameters is to use the
    Reliance logical block size, scale as necessary to use the desired emulated
    physical block size, while keeping the same overall size.
    Revision 1.19  2005/04/18 18:02:28Z  GaryP
    Updated to use a renamed header.
    Revision 1.18  2005/02/26 02:38:06Z  GaryP
    Code formatting cleanup -- no functional changes.
    Revision 1.17  2004/12/30 17:32:45Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.16  2004/12/19 00:51:08Z  GaryP
    Modified to accomodate MBRs that manage disks with sector sizes other than
    512 bytes.  Added a FlashFX proprietary field into the MBR to denote the
    sector size.
    Revision 1.15  2004/09/23 08:06:29Z  GaryP
    Updated to work when VBF_BLOCK_SIZE is different than DSK_SECTORLEN.
    Revision 1.14  2004/09/17 02:27:26Z  GaryP
    Moved the vbfread() call into a higher level function.  Split out the MBR
    processing code into a different module.
    Revision 1.13  2004/09/15 02:23:43Z  GaryP
    Changed a DclProductionError() to DclError();
    Revision 1.12  2004/08/30 22:57:28Z  GaryP
    Modified FfxRelGetLogicalDiskSize() to return the logical block size.
    Revision 1.11  2004/08/07 04:37:29Z  GaryP
    Made all FAT and Reliance support dependent on their respective FFXCONF_*
    settings.
    Revision 1.10  2004/04/30 02:27:40Z  garyp
    Updated to eliminate passing sector buffers around.
    Revision 1.9  2004/01/25 06:05:58Z  garys
    Merge from FlashFXMT
    Revision 1.3.1.7  2004/01/25 06:05:58  garyp
    Moved the functions FfxMovePTE() and FfxDisplayPTE() to other modules
    for better granularity.
    Revision 1.3.1.6  2003/12/25 00:07:04Z  garyp
    Changed all uses of VBF_API_BLOCK_SIZE to VBF_BLOCK_SIZE and eliminated any
    conditional code based on different values for those settings.
    Revision 1.3.1.5  2003/12/17 18:56:17Z  garys
    Typecast to D_UINT32 * instead of D_UINT32 * for consistency
    Revision 1.3.1.4  2003/12/12 00:04:46  garys
    Added support for Reliance in VxWorks.
    Revision 1.3.1.3  2003/12/05 19:01:13  garys
    Added support for Reliance without MBR (like in CE 3.0).
    Revision 1.3.1.2  2003/11/03 04:54:16  garyp
    Re-checked into variant sandbox.
    Revision 1.4  2003/11/03 04:54:16Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.3  2003/04/16 01:59:54Z  garyp
    Minor variable renaming.
    Revision 1.2  2003/04/13 02:27:52Z  garyp
    Header file changes.
    Revision 1.1  2003/03/26 08:36:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <diskapi.h>
#include <dlpartid.h>

#if FFXCONF_FATSUPPORT
#include <dlfat.h>
#include <fxfatapi.h>
#endif

#if FFXCONF_RELIANCESUPPORT
#include <dlrelapi.h>
#endif

/*-------------------------------------------------------------------
    Protected: FfxGetPhysicalDiskParams()

    This routine attempts to determine the physical disk parameters
    by examining the supplied data to see if is an MBR or a boot
    record.  If it is a recognized FAT or Reliance boot record, the
    disk parameters are pulled from the BPB fields.  If it is an
    MBR, the partitions are scanned to determine the extent of the
    disk, and CHS values are derived.

    The data pointed to by pBuffer must be at least 512 bytes long,
    and is typically one physical page long, however nothing beyond
    the 512 byte boundary is examined.

    ulSectorLen is the LOGICAL sector length and may be longer than
    the data pointed to by pBuffer.  In any event, the function never
    examines anything beyond 512 bytes in pBuffer.

    Parameters:
        pPDI        - A pointer to the FFXPHYSICALDISKINFO structure
        pBuffer     - A pointer to the buffer containing the data
                      to examine.
        ulSectorLen - The logical sector length (not necessarily the
                      length of pBuffer).

    Return Value:
        Always returns TRUE (currently)
 -------------------------------------------------------------------*/
D_BOOL FfxGetPhysicalDiskParams(
    FFXPHYSICALDISKINFO    *pPDI,
    const D_BUFFER         *pBuffer,
    D_UINT32                ulSectorLen)
{
    D_UINT32                ulTotalSecs = 0;
    D_BOOL                  fTryFATandMBR = TRUE;
  #if FFXCONF_FATSUPPORT
    D_UINT8                 bSecLow = (D_UINT8)ulSectorLen;
    D_UINT8                 bSecHigh = (D_UINT8)(ulSectorLen >> 8);
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxGetPhysicalDiskParams() LogicalSectorLen=%lU\n", ulSectorLen));

    DclAssert(pPDI);
    DclAssert(pBuffer);

    /*  If the sector length is less than a standard disk sector
        size (512B), this can't possible be a FAT format, or have
        and MBR on it.
    */
    if(ulSectorLen < DSK_SECTORLEN)
        fTryFATandMBR = FALSE;

    DclMemSet(pPDI, 0, sizeof(*pPDI));

    /*  These are implied by the preceding DclMemSet().

    pPDI->fUsingMBR = FALSE;
    pPDI->fIsFFXMBR = FALSE;
    pPDI->fFoundValidPartitions = FALSE;
    */

    /*  Default to using the specified sector length, but hopefully we
        will find something on the disk that will conclusively tell us
        what sector size it was formatted with...
    */
    pPDI->ulBytesPerSector = ulSectorLen;

    /*  Verify the signature
    */
    if(fTryFATandMBR &&
        (pBuffer[BOOTREC_SIGNATURE_OFFSET] == 0x55 &&
        pBuffer[BOOTREC_SIGNATURE_OFFSET + 1] == 0xAA))
    {
      #if FFXCONF_FATSUPPORT
        /*  We want to be able to support existing disks with and without MBRs
            however there is no simple way to determine what's already on the
            disk since both MBRs and boot records use the same 0x55AA signature.

            We check that the BytesPerSector value matches the passed in
            sector size, and that the media ID byte is at least the minimum
            value, and that the number of FATs is 0, 1, or 2 (allowing for
            TFAT compatibility).
        */
        if((pBuffer[BT_BPS_OFF + 0] == bSecLow) &&
           (pBuffer[BT_BPS_OFF + 1] == bSecHigh) &&
           (pBuffer[BT_MDID_OFF] >= FAT_MEDIAID_MIN) &&
           ((pBuffer[BT_NFAT_OFF] == 0) ||
            (pBuffer[BT_NFAT_OFF] == 1) ||
            (pBuffer[BT_NFAT_OFF] == 2)))
        {
            D_BYTE  bFSID;

            bFSID = FfxFatGetDiskParamsFromBootRecord(pBuffer, pPDI);
            if(bFSID == FSID_NONE)
            {
                FFXPRINTF(1, ("No MBR or FAT boot record found\n"));
                goto GetPhysDiskParamsCleanup;
            }

            /*  Create a virtual partition entry at slot zero since there is no
                MBR on the disk.
            */
            pPDI->fpi[0].bFSID = bFSID;
            pPDI->fpi[0].ulStartSector = 0;
            pPDI->fpi[0].ulSectorCount = pPDI->ulTotalSectors;
            pPDI->fpi[0].bBootable = DCLMBR_PARTITION_BOOTABLE;

            pPDI->fFoundValidPartitions = TRUE;

            FFXPRINTF(1, ("Found FAT Boot Record, MBR not in use\n"));

            goto GetPhysDiskParamsCleanup;
        }

        FFXPRINTF(2, ("No FAT boot record, probably an MBR\n"));
      #endif

      #if FFXCONF_MBRSUPPORT
        /*-------------------------------------------------------------
            If we've gotten to this point, we've determined that there
            is not a recognized FAT boot record in the first physical
            sector, so it's probably an MBR, since the 55AA is there.
        -------------------------------------------------------------*/

        /*  Load the partition table entries and try to derive the
            total disk size.
        */
        ulTotalSecs = FfxMBRLoadPartitions(pPDI, pBuffer);
      #endif
    }

  #if FFXCONF_RELIANCESUPPORT
    else
    {
        D_UINT32       ulBlockSize;

        FFXPRINTF(2, ("0xAA55 signature not found\n"));

        /*  We've now gone through all sorts of contortions to see if there
            is any recognizable FAT boot records or MBR information on the
            disk from which we can derive the emulated physical disk
            characteristics.  Our last-ditch effort is to find a Reliance
            MasterBlock that has the logical block size and count.
        */
        ulTotalSecs = DclRelGetLogicalDiskSize(pBuffer, &ulBlockSize, ulSectorLen);
        if(ulTotalSecs)
        {
            /*  If the logical block size is larger than the desired physical
                block size, scale as needed to get the size we want, and keep
                the total disk size the same.
            */
            ulTotalSecs *= (ulBlockSize / ulSectorLen);

            pPDI->ulBytesPerSector = ulSectorLen;

            /*  Create a virtual partition entry at slot zero since there
                is no MBR on the disk.
            */
            pPDI->fpi[0].bFSID = FSID_RELIANCE;
            pPDI->fpi[0].ulStartSector = 0;
            pPDI->fpi[0].ulSectorCount = ulTotalSecs;
            pPDI->fpi[0].bBootable = DCLMBR_PARTITION_BOOTABLE;
        }
    }
  #endif

    if(ulTotalSecs)
    {
        if(FfxCalcPhysicalDiskParams(pPDI, ulTotalSecs))
        {
            pPDI->fFoundValidPartitions = TRUE;
        }
        else
        {
            DclError();
        }

        /*  This goto is coded solely to avoid a compiler warning about an
            unreferenced label, should both FAT and Reliance support be
            turned off.
        */
        goto GetPhysDiskParamsCleanup;
    }

  GetPhysDiskParamsCleanup:

    FFXPRINTF(1, ("PhysDiskParams:   BPS = %lX\n",         pPDI->ulBytesPerSector));
    FFXPRINTF(1, ("         TotalSectors = %lX\n",         pPDI->ulTotalSectors));
    FFXPRINTF(1, ("                Heads =     %X\n",      pPDI->uHeads));
    FFXPRINTF(1, ("      SectorsPerTrack =     %X\n",      pPDI->uSectorsPerTrack));
    FFXPRINTF(1, ("            Cylinders =     %X\n",      pPDI->uCylinders));
    FFXPRINTF(1, ("            fUsingMBR =          %U\n", pPDI->fUsingMBR));
    FFXPRINTF(1, ("            fIsFFXMBR =          %U\n", pPDI->fIsFFXMBR));
    FFXPRINTF(1, ("fFoundValidPartitions =          %U\n", pPDI->fFoundValidPartitions));

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEUNDENT),
        "FfxGetPhysicalDiskParams() returning TRUE\n"));

    return TRUE;
}


/*-------------------------------------------------------------------
    Protected: FfxCalcPhysicalDiskParams()

    This routine calculates the disk CHS values from the given
    total sectors value, and fills in the FFXPHYSICALDISKINFO
    structure.

    Parameters:
        pPDI        - A pointer to the FFXPHYSICALDISKINFO structure
        ulSectors   - The sector count

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxCalcPhysicalDiskParams(
    FFXPHYSICALDISKINFO    *pPDI,
    D_UINT32                ulSectors)
{
    D_UINT32                ulMaxSize = ulSectors;
    D_UINT16                uHeads;
    D_UINT16                uSecPerTrack;
    D_UINT16                uCylinders;

    FFXPRINTF(1, ("FfxCalcPhysicalDiskParams() TotalSectors=%lU\n", ulSectors));

    DclAssert(pPDI);
    DclAssert(ulSectors);

    if(FfxCalculateCHS(&ulMaxSize, &uHeads, &uSecPerTrack, &uCylinders))
    {
        pPDI->ulTotalSectors = ulMaxSize;
        pPDI->uHeads = uHeads;
        pPDI->uSectorsPerTrack = uSecPerTrack;
        pPDI->uCylinders = uCylinders;

        return TRUE;
    }

    return FALSE;
}


