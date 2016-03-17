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

    This module contains functions for manipulating FAT boot records.

    ToDo: Should consider scrapping the hard-coded x86 style boot record
          code included from dlfatbootrec.h, and simply construct the few
          necessary fields manually.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlfatbootrec.c $
    Revision 1.1  2009/01/10 03:37:22Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlfatapi.h>
#include <dlpartid.h>
#include <dlfat.h>
#include "dlfatbootrec.h"


/*-------------------------------------------------------------------
    Public: DclFatBootRecordBuild()

    This function creates a standard FAT boot record and fills
    it in with the specified BPB information.

    Parameters:
       pSector - The data buffer to build the BPB data into.
                 The buffer <must> be at least as long as the
                 uBytesPerSector field in the supplied DCLFATBPB
                 structure.
       pBPB    - The DCLFATBPB structure to use.

    Returns:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclFatBootRecordBuild(
    D_BUFFER           *pSector,
    const DCLFATBPB    *pBPB)
{
    D_BYTE              bFSID;

    DCLPRINTF(1, ("DclFatBootRecordBuild()\n"));

    if(pBPB->uBytesPerSector < sizeof(abFATBootRecord))
    {
        DCLPRINTF(1, ("DclFatBootRecordBuild(): BytesPerSector value is too small\n"));
        return DCLSTAT_FAT_UNSUPPORTEDBYTESPERSEC;
    }

    bFSID = DclFatBPBDeriveFSID(pBPB, NULL);

    if(bFSID == FSID_NONE)
    {
        DCLPRINTF(1, ("DclFatBootRecordBuild(): Unrecognizable file system in boot record\n"));
        return DCLSTAT_FAT_UNRECOGNIZEDSYSID;
    }

    /*  Make certain is returned a result we know how to handle
    */
    DclAssert(bFSID == FSID_FAT12 || bFSID == FSID_FAT16 || bFSID == FSID_FAT32);

    DclMemCpy(pSector, abFATBootRecord, sizeof(abFATBootRecord));

    if(pBPB->uBytesPerSector > sizeof(abFATBootRecord))
    {
        /*  In the event that the boot record is longer than a normal
            512-byte sector, place the 0x55AA signature at the very end,
            in addition to the one that will be at the 510-byte offset.
        */
        pSector[pBPB->uBytesPerSector - 2] = 0x55;
        pSector[pBPB->uBytesPerSector - 1] = 0xAA;
    }

    /*  Now move all the elements of the BPB into the boot sector!
    */
    DCLNE2LE(pSector + BT_BPS_OFF,  (D_BUFFER*)&pBPB->uBytesPerSector,  sizeof(pBPB->uBytesPerSector));
    DCLNE2LE(pSector + BT_SPC_OFF,  (D_BUFFER*)&pBPB->bSecPerCluster,   sizeof(pBPB->bSecPerCluster));
    DCLNE2LE(pSector + BT_RES_OFF,  (D_BUFFER*)&pBPB->uReservedSectors, sizeof(pBPB->uReservedSectors));
    DCLNE2LE(pSector + BT_NFAT_OFF, (D_BUFFER*)&pBPB->bNumFats,         sizeof(pBPB->bNumFats));
    DCLNE2LE(pSector + BT_MAXD_OFF, (D_BUFFER*)&pBPB->uMaxDirs,         sizeof(pBPB->uMaxDirs));
    DCLNE2LE(pSector + BT_UTSC_OFF, (D_BUFFER*)&pBPB->uTotalSecs,       sizeof(pBPB->uTotalSecs));
    DCLNE2LE(pSector + BT_MDID_OFF, (D_BUFFER*)&pBPB->bMediaDescrip,    sizeof(pBPB->bMediaDescrip));
    DCLNE2LE(pSector + BT_SPF_OFF,  (D_BUFFER*)&pBPB->uSecPerFAT,       sizeof(pBPB->uSecPerFAT));
    DCLNE2LE(pSector + BT_SPT_OFF,  (D_BUFFER*)&pBPB->uSecPerTrack,     sizeof(pBPB->uSecPerTrack));
    DCLNE2LE(pSector + BT_SIDS_OFF, (D_BUFFER*)&pBPB->uSides,           sizeof(pBPB->uSides));
    DCLNE2LE(pSector + BT_HIDN_OFF, (D_BUFFER*)&pBPB->ulHidden,         sizeof(pBPB->ulHidden));
    DCLNE2LE(pSector + BT_LTSC_OFF, (D_BUFFER*)&pBPB->ulTotalSecs,      sizeof(pBPB->ulTotalSecs));

	if (bFSID == FSID_FAT12 || bFSID == FSID_FAT16)
	{
	    DCLNE2LE(pSector + BT16_PDRV_OFF, (D_BUFFER*)&pBPB->ext.bpb16.bPhysDrive,     sizeof(pBPB->ext.bpb16.bPhysDrive));
	    DCLNE2LE(pSector + BT16_UNSD_OFF, (D_BUFFER*)&pBPB->ext.bpb16.bUnused,        sizeof(pBPB->ext.bpb16.bUnused));
	    DCLNE2LE(pSector + BT16_SIGN_OFF, (D_BUFFER*)&pBPB->ext.bpb16.bSignature,     sizeof(pBPB->ext.bpb16.bSignature));
	    DCLNE2LE(pSector + BT16_SRNM_OFF, (D_BUFFER*)&pBPB->ext.bpb16.ulSerialNumber, sizeof(pBPB->ext.bpb16.ulSerialNumber));
	    DclMemCpy(pSector + BT16_LABL_OFF, &pBPB->ext.bpb16.acLabel,                  sizeof(pBPB->ext.bpb16.acLabel));
	    DclMemCpy(pSector + BT16_SSID_OFF, &pBPB->ext.bpb16.acSysID,                  sizeof(pBPB->ext.bpb16.acSysID));
	}
	else if (bFSID == FSID_FAT32)
	{
	    DCLNE2LE(pSector + BT32_PDRV_OFF, (D_BUFFER*)&pBPB->ext.bpb32.bPhysDrive,     sizeof(pBPB->ext.bpb32.bPhysDrive));
	    DCLNE2LE(pSector + BT32_UNSD_OFF, (D_BUFFER*)&pBPB->ext.bpb32.bUnused,        sizeof(pBPB->ext.bpb32.bUnused));
	    DCLNE2LE(pSector + BT32_SIGN_OFF, (D_BUFFER*)&pBPB->ext.bpb32.bSignature,     sizeof(pBPB->ext.bpb32.bSignature));
	    DCLNE2LE(pSector + BT32_SRNM_OFF, (D_BUFFER*)&pBPB->ext.bpb32.ulSerialNumber, sizeof(pBPB->ext.bpb32.ulSerialNumber));
	    DclMemCpy(pSector + BT32_LABL_OFF, &pBPB->ext.bpb32.acLabel,                  sizeof(pBPB->ext.bpb32.acLabel));
	    DclMemCpy(pSector + BT32_SSID_OFF, &pBPB->ext.bpb32.acSysID,                  sizeof(pBPB->ext.bpb32.acSysID));
	}

    return DCLSTAT_SUCCESS;
}


