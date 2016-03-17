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
    $Log: dskpart.c $
    Revision 1.16  2011/12/08 18:56:28Z  garyp
    Removed some legacy FAT code which assumed 512-byte sectors
    in the event that FlashFX had NOT written the MBR itself. 
    Revision 1.15  2011/02/09 00:57:55Z  garyp
    Modified to use some refactored disk partition manipulation code which
    has moved from FlashFX into DCL.  Not functionally changed otherwise.
    Revision 1.14  2009/07/22 00:12:31Z  garyp
    Merged from the v4.0 branch.  Modified to use the cleaned up and refactored
    BPB structures.  Eliminated an unused parameter for FfxMBRLoadPartitions().
    Revision 1.13  2009/04/01 15:36:46Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.12  2009/02/09 01:20:35Z  garyp
    Updated to include dlpartid.h.
    Revision 1.11  2008/01/13 07:27:12Z  keithg
    Function header updates to support autodoc.
    Revision 1.10  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.9  2007/09/27 00:04:06Z  jeremys
    Renamed a header file.
    Revision 1.8  2006/10/16 19:49:02Z  Garyp
    Modified so MBR support can be stripped out of the product.
    Revision 1.7  2006/10/06 00:16:59Z  Garyp
    Modified to use the DCL specific byte-ordering macros.
    Revision 1.6  2006/06/21 23:11:24Z  Garyp
    Documentation updates -- no functional changes.
    Revision 1.5  2006/03/17 20:13:49Z  Garyp
    Modified to allow FAT boot records for non-512-byte disks to be recognized.
    Revision 1.4  2006/02/21 23:18:16Z  Garyp
    Minor type changes.
    Revision 1.3  2006/02/20 03:27:49Z  Garyp
    Minor type changes.
    Revision 1.2  2006/01/11 02:06:35Z  Garyp
    Documentation and debug code changes only -- nothing functional.
    Revision 1.1  2005/10/02 01:58:26Z  Pauli
    Initial revision
    Revision 1.6  2004/12/30 21:37:57Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.5  2004/12/19 04:09:20Z  GaryP
    Fixed some syntax problems with the previous rev.
    Revision 1.4  2004/12/19 00:12:50Z  GaryP
    Modified to accomodate MBRs that manage disks with sector sizes other than
    512 bytes.  Added a FlashFX proprietary field into the MBR to denote the
    sector size.
    Revision 1.3  2004/09/17 02:45:12Z  GaryP
    Added FfxMBRLoadPartitions().
    Revision 1.2  2004/08/09 22:02:50Z  GaryP
    Eliminated some unnecessary headers.
    Revision 1.1  2004/01/25 06:06:38Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_MBRSUPPORT

#include <diskapi.h>
#include <dlpartid.h>
#include <dlmbr.h>

#if FFXCONF_FATSUPPORT
#include <dlfat.h>
#include <fxfatapi.h>
#endif


/*-------------------------------------------------------------------
    Protected: FfxMBRLoadPartitions()

    Determine if the specified buffer contains a recognized MBR,
    and if so, load the partition table entries into the specified
    FFXPHYSICALDISKINFO structure.

    From this information attempt to determine the disk's bytes per
    sector value, and the total number of sectors on the disk.

    Note that the pPDI->ulBytesPerSector value will only be modified
    if it is determined that a FlashFX specific MBR is being used.

    Parameters:
        pPDI  - A pointer to the FFXPHYSICALDISKINFO structure to use.
        pData - A pointer to a byte array containing the potential MBR.

    Return Value:
        If successful, returns the total number of sectors on the
        disk, otherwise it returns 0.
-------------------------------------------------------------------*/
D_UINT32 FfxMBRLoadPartitions(
    FFXPHYSICALDISKINFO    *pPDI,
    const D_BUFFER         *pData)
{
    D_UINT32                ulTotalSecs = 0;
    unsigned                kk;
    DCLDISKPARTITION        pt;
    D_UINT16                uBootAccumulator = 0;

    FFXPRINTF(1, ("FfxMBRLoadPartitions()\n"));

    /*  This function should be called only after the pPDI 
        structure is zero'd.
    */
    DclAssertWritePtr(pPDI, sizeof(*pPDI));
    DclAssert(pPDI->fUsingMBR == FALSE);
    DclAssert(pPDI->fFoundValidPartitions == FALSE);

    if(DclMemCmp(&pData[MBR_OFFSET_FFXSIG], MBR_FFX_SIGNATURE, 4) == 0)
    {
        DCLLE2NE((D_BUFFER*)&pPDI->ulBytesPerSector, &pData[MBR_OFFSET_FFXBPS],
                          sizeof(pPDI->ulBytesPerSector));

        pPDI->fIsFFXMBR = TRUE;

        FFXPRINTF(1, ("FlashFX MBR signature found, BPS=%lU\n", pPDI->ulBytesPerSector));
    }

    for(kk = 0; kk < DCLDIMENSIONOF(pPDI->fpi); kk++)
    {
        DclMBRPartitionLoad(&pt, &pData[DCLMBR_PARTITION_OFFSET + (DCLMBR_PARTITION_SIZE * kk)]);

        uBootAccumulator += (D_UINT16)pt.bIsBootable;

        if(pt.bFileSysID == FSID_NONE)
            continue;

        FFXPRINTF(2, ("Found PTE #%u in MBR\n", kk));

      #if D_DEBUG
        DclMBRPartitionDisplay(&pt, FALSE);
      #endif

        pPDI->fpi[kk].bFSID = pt.bFileSysID;
        pPDI->fpi[kk].ulStartSector = pt.ulStartSector;
        pPDI->fpi[kk].ulSectorCount = pt.ulSectorCount;
        pPDI->fpi[kk].bBootable = pt.bIsBootable;

        if(ulTotalSecs < pt.ulStartSector + pt.ulSectorCount)
            ulTotalSecs = pt.ulStartSector + pt.ulSectorCount;

        pPDI->fUsingMBR = TRUE;
    }

    /*  We must be able to distinguish an MBR from garbage or a boot
        record (in the event that FFXCONF_FATSUPPORT==FALSE).  The
        0x55AA at the end is not good enough because that is used
        by boot records as well.

        Therefore this algorithm accumulates the sum total of the
        "IsBootable" field, and if the result is evenly divisible
        by the DCLMBR_PARTITION_BOOTABLE value (0x80), then we assume
        that this really is a boot record.  (Most desktop systems will
        have only a single valid entry in one of these fields, but we
        really can't count on that.)
    */
    if(uBootAccumulator % DCLMBR_PARTITION_BOOTABLE)
    {
        /*  The test failed, so that must not be an MBR after all.
        */
        for(kk = 0; kk < DCLDIMENSIONOF(pPDI->fpi); kk++)
            DclMemSet(&pPDI->fpi[kk], 0, sizeof pPDI->fpi[kk]);

        pPDI->fUsingMBR = FALSE;

        return 0;
    }

    return ulTotalSecs;
}


#endif  /* FFXCONF_MBRSUPPORT */


