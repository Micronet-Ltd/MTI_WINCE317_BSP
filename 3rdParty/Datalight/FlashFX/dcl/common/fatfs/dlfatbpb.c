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

    This module contains functions for manipulating FAT boot records
    and BPBs.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlfatbpb.c $
    Revision 1.6  2009/06/28 00:21:37Z  garyp
    Final merge from the v4.0 branch --  Updated to use the cleaned up and
    refactored BPB structures.
    Revision 1.5  2009/03/20 22:48:10Z  billr
    Use correct format specification to print 32-bit values.
    Revision 1.4  2009/02/13 20:19:07Z  garyp
    Partial merge from the v4.0 branches.  Renamed the functions.  Modified
    DclFatBPBMove() to return the FS ID.  Updated so that DclFATBPBDisplay()
    is available in RELEASE mode.
    Revision 1.3  2008/12/12 19:05:39Z  jeremys
    Testing MKS quirks.
    Revision 1.2  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/09/27 01:24:24Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#include <dlfatapi.h>
#include <dlpartid.h>
#include <dlfat.h>


/*-------------------------------------------------------------------
    Public: DclFatBPBMove()

    This function moves a BPB from raw disk sector to a native C
    structure.

    Parameters:
        pBPB    - A pointer to the DCLFATBPB structure to fill.
        pSector - A pointer to the packed data stream.

    Return Value:
        Returns the FSID_* value as determined from the raw data.
        Returns FSID_NONE if the data does not appear to contain a
        valid FAT BPB (and nothing will be moved).
-------------------------------------------------------------------*/
D_BYTE DclFatBPBMove(
    DCLFATBPB      *pBPB,
    const D_BUFFER *pSector)
{
    D_BYTE          bFSID;

    DclAssert(pBPB);
    DclAssert(pSector);

    bFSID = DclFatBootRecordDeriveFSID(pSector, NULL);

    if(bFSID != FSID_FAT12 && bFSID != FSID_FAT16 && bFSID != FSID_FAT32)
        return bFSID;

    /*  The BPB has words and long words that are on odd boundaries.
        In order to avoid address exceptions or faults on CPUs that
        cannot address words on odd boundaries, we need to move each
        element of the BPB one at a time with MEMMOVE.

        Additionally we must change from the little endian format
        used in the raw data, to the native format.
    */

    DCLLE2NE((D_BUFFER*)&pBPB->uBytesPerSector,  &pSector[BT_BPS_OFF],  sizeof(pBPB->uBytesPerSector));
    DCLLE2NE((D_BUFFER*)&pBPB->bSecPerCluster,   &pSector[BT_SPC_OFF],  sizeof(pBPB->bSecPerCluster));
    DCLLE2NE((D_BUFFER*)&pBPB->uReservedSectors, &pSector[BT_RES_OFF],  sizeof(pBPB->uReservedSectors));
    DCLLE2NE((D_BUFFER*)&pBPB->bNumFats,         &pSector[BT_NFAT_OFF], sizeof(pBPB->bNumFats));
    DCLLE2NE((D_BUFFER*)&pBPB->uMaxDirs,         &pSector[BT_MAXD_OFF], sizeof(pBPB->uMaxDirs));
    DCLLE2NE((D_BUFFER*)&pBPB->uTotalSecs,       &pSector[BT_UTSC_OFF], sizeof(pBPB->uTotalSecs));
    DCLLE2NE((D_BUFFER*)&pBPB->bMediaDescrip,    &pSector[BT_MDID_OFF], sizeof(pBPB->bMediaDescrip));
    DCLLE2NE((D_BUFFER*)&pBPB->uSecPerFAT,       &pSector[BT_SPF_OFF],  sizeof(pBPB->uSecPerFAT));
    DCLLE2NE((D_BUFFER*)&pBPB->uSecPerTrack,     &pSector[BT_SPT_OFF],  sizeof(pBPB->uSecPerTrack));
    DCLLE2NE((D_BUFFER*)&pBPB->uSides,           &pSector[BT_SIDS_OFF], sizeof(pBPB->uSides));
    DCLLE2NE((D_BUFFER*)&pBPB->ulHidden,         &pSector[BT_HIDN_OFF], sizeof(pBPB->ulHidden));
    DCLLE2NE((D_BUFFER*)&pBPB->ulTotalSecs,      &pSector[BT_LTSC_OFF], sizeof(pBPB->ulTotalSecs));

    if(bFSID == FSID_FAT12 || bFSID == FSID_FAT16)
    {
        DCLLE2NE((D_BUFFER*)&pBPB->ext.bpb16.bPhysDrive,     &pSector[BT16_PDRV_OFF], sizeof(pBPB->ext.bpb16.bPhysDrive));
        DCLLE2NE((D_BUFFER*)&pBPB->ext.bpb16.bUnused,        &pSector[BT16_UNSD_OFF], sizeof(pBPB->ext.bpb16.bUnused));
        DCLLE2NE((D_BUFFER*)&pBPB->ext.bpb16.bSignature,     &pSector[BT16_SIGN_OFF], sizeof(pBPB->ext.bpb16.bSignature));
        DCLLE2NE((D_BUFFER*)&pBPB->ext.bpb16.ulSerialNumber, &pSector[BT16_SRNM_OFF], sizeof(pBPB->ext.bpb16.ulSerialNumber));
        DclMemCpy(&pBPB->ext.bpb16.acLabel,                  &pSector[BT16_LABL_OFF], sizeof(pBPB->ext.bpb16.acLabel));
        DclMemCpy(&pBPB->ext.bpb16.acSysID,                  &pSector[BT16_SSID_OFF], sizeof(pBPB->ext.bpb16.acSysID));
    }
    else
    {
        DCLLE2NE((D_BUFFER*)&pBPB->ext.bpb32.ulSectorsPerFat,       &pSector[BT32_SPRF_OFF], sizeof(pBPB->ext.bpb32.ulSectorsPerFat));
        DCLLE2NE((D_BUFFER*)&pBPB->ext.bpb32.uExtendedFlags,        &pSector[BT32_EXFL_OFF], sizeof(pBPB->ext.bpb32.uExtendedFlags));
        DCLLE2NE((D_BUFFER*)&pBPB->ext.bpb32.uFileSystemVersion,    &pSector[BT32_FSVR_OFF], sizeof(pBPB->ext.bpb32.uFileSystemVersion));
        DCLLE2NE((D_BUFFER*)&pBPB->ext.bpb32.ulRootDirStartCluster, &pSector[BT32_RDSC_OFF], sizeof(pBPB->ext.bpb32.ulRootDirStartCluster));
        DCLLE2NE((D_BUFFER*)&pBPB->ext.bpb32.uFSInfoSector,         &pSector[BT32_FSIS_OFF], sizeof(pBPB->ext.bpb32.uFSInfoSector));
        DCLLE2NE((D_BUFFER*)&pBPB->ext.bpb32.uBackupBootSector,     &pSector[BT32_BUBS_OFF], sizeof(pBPB->ext.bpb32.uBackupBootSector));
        DclMemCpy(&pBPB->ext.bpb32.abReserved,                      &pSector[BT32_RESV_OFF], sizeof(pBPB->ext.bpb32.abReserved));
        DCLLE2NE((D_BUFFER*)&pBPB->ext.bpb32.bPhysDrive,            &pSector[BT32_PDRV_OFF], sizeof(pBPB->ext.bpb32.bPhysDrive));
        DCLLE2NE((D_BUFFER*)&pBPB->ext.bpb32.bUnused,               &pSector[BT32_UNSD_OFF], sizeof(pBPB->ext.bpb32.bUnused));
        DCLLE2NE((D_BUFFER*)&pBPB->ext.bpb32.bSignature,            &pSector[BT32_SIGN_OFF], sizeof(pBPB->ext.bpb32.bSignature));
        DCLLE2NE((D_BUFFER*)&pBPB->ext.bpb32.ulSerialNumber,        &pSector[BT32_SRNM_OFF], sizeof(pBPB->ext.bpb32.ulSerialNumber));
        DclMemCpy(&pBPB->ext.bpb32.acLabel,                         &pSector[BT32_LABL_OFF], sizeof(pBPB->ext.bpb32.acLabel));
        DclMemCpy(&pBPB->ext.bpb32.acSysID,                         &pSector[BT32_SSID_OFF], sizeof(pBPB->ext.bpb32.acSysID));
    }

    return bFSID;
}


/*-------------------------------------------------------------------
    Public: DclFatBPBDisplay()

    This function displays the contents of a BPB.

    Parameters:
        pBPB points to a BPB structure.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclFatBPBDisplay(
    const DCLFATBPB    *pBPB)
{
    D_BYTE              bFSID;
    char                szLabel[FAT_VOLUME_LEN+1];
    char                szSysID[FAT_SYSID_LEN+1];

    DclAssert(pBPB);

    bFSID = DclFatBPBDeriveFSID(pBPB, NULL);

    DclPrintf("BPB: BytesPerSector:           %X\n",       pBPB->uBytesPerSector);
    DclPrintf("     SectorsPerCluster:             %u\n",  pBPB->bSecPerCluster);
    DclPrintf("     ReservedSectors:          %X\n",       pBPB->uReservedSectors);
    DclPrintf("     FATCount:                      %u\n",  pBPB->bNumFats);
    DclPrintf("     RootDirEntries:           %X\n",       pBPB->uMaxDirs);
    DclPrintf("     SectorCount:              %X\n",       pBPB->uTotalSecs);
    DclPrintf("     MediaDescriptor:            0x%02x\n", pBPB->bMediaDescrip);
    DclPrintf("     SectorsPerFAT:            %X\n",       pBPB->uSecPerFAT);
    DclPrintf("     SectorsPerTrack:          %X\n",       pBPB->uSecPerTrack);
    DclPrintf("     Heads:                    %X\n",       pBPB->uSides);
    DclPrintf("     HiddenSectors:        %lX\n",          pBPB->ulHidden);
    DclPrintf("     HugeSectorCount:      %lX\n",          pBPB->ulTotalSecs);

    if(bFSID == FSID_FAT12 || bFSID == FSID_FAT16)
    {
        DclStrNCpy(szLabel, pBPB->ext.bpb16.acLabel, sizeof(szLabel));
        szLabel[sizeof(szLabel)-1] = 0;
        DclStrNCpy(szSysID, pBPB->ext.bpb16.acSysID, sizeof(szSysID));
        szSysID[sizeof(szSysID)-1] = 0;

        DclPrintf("     PhysDriveNum:               0x%02x\n", pBPB->ext.bpb16.bPhysDrive);
        DclPrintf("     DirtyFlag:                  0x%02x\n", pBPB->ext.bpb16.bUnused);
        DclPrintf("     Signature:                  0x%02x\n", pBPB->ext.bpb16.bSignature);
        DclPrintf("     SerialNumber:         %lX\n",          pBPB->ext.bpb16.ulSerialNumber);
        DclPrintf("     VolumeLabel:         %11s\n",          szLabel);
        DclPrintf("     FileSystemName:      %8s\n",           szSysID);
    }
    else if(bFSID == FSID_FAT32)
    {
        DclStrNCpy(szLabel, pBPB->ext.bpb32.acLabel, sizeof(szLabel));
        szLabel[sizeof(szLabel)-1] = 0;
        DclStrNCpy(szSysID, pBPB->ext.bpb32.acSysID, sizeof(szSysID));
        szSysID[sizeof(szSysID)-1] = 0;

        DclPrintf("     SectorsPerFat:        %lX\n",          pBPB->ext.bpb32.ulSectorsPerFat);
        DclPrintf("     ExtendedFlags:            %X\n",       pBPB->ext.bpb32.uExtendedFlags);
        DclPrintf("     FileSystemVersion:        %X\n",       pBPB->ext.bpb32.uFileSystemVersion);
        DclPrintf("     RootDirStartCluster:  %lX\n",          pBPB->ext.bpb32.ulRootDirStartCluster);
        DclPrintf("     FSInfoSector:             %X\n",       pBPB->ext.bpb32.uFSInfoSector);
        DclPrintf("     BackUpBootSector:         %X\n",       pBPB->ext.bpb32.uBackupBootSector);
        DclPrintf("     PhysDriveNum:               0x%02x\n", pBPB->ext.bpb32.bPhysDrive);
        DclPrintf("     DirtyFlag:                  0x%02x\n", pBPB->ext.bpb32.bUnused);
        DclPrintf("     Signature:                  0x%02x\n", pBPB->ext.bpb32.bSignature);
        DclPrintf("     SerialNumber:         %lX\n",          pBPB->ext.bpb32.ulSerialNumber);
        DclPrintf("     VolumeLabel:         %11s\n",          szLabel);
        DclPrintf("     FileSystemName:      %8s\n",           szSysID);
    }
    else
    {
        DclPrintf("The BPB does not contain a recognized file system ID -- %u\n", bFSID);
    }

    return;
}


