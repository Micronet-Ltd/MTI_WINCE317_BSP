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
    $Log: dlfatbpbcr.c $
    Revision 1.2  2009/06/28 03:51:49Z  garyp
    Merged in BillR's changes from the trunk FFX revision of fatbpbcr.c, where
    bug 2275 was addressed.
    Revision 1.1  2009/01/10 03:39:36Z  garyp
    Initial revision
    Revision 1.7.1.2  2008/10/09 21:01:40Z  garyp
    Modified to use the cleaned up and refactored BPB structures.
    Revision 1.7  2008/01/13 07:26:43Z  keithg
    Function header updates to support autodoc.
    Revision 1.6  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2007/09/26 23:55:37Z  jeremys
    Renamed a header file and a function name.
    Revision 1.4  2006/10/18 23:04:03Z  Garyp
    Minor type changes.
    Revision 1.3  2006/06/02 21:07:36Z  Pauli
    Removed the incomplete FAT-32 support.
    Revision 1.2  2006/02/03 16:07:05Z  johnb
    Added FATMON FAT32 Support
    Revision 1.1  2005/10/03 19:37:22Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlfatapi.h>
#include <dlfat.h>


/*-------------------------------------------------------------------
    Public: DclFatBPBBuild()

    Build a FAT compatible BPB structure.  Fills in the entire
    DCLFATBPB from the given information.

    Parameters:
       pBPB     - The data buffer in which to build the DCLFATBPB
       pFFP     - A pointer to the DCLFATPARAMS structure to use

    Returns:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclFatBPBBuild(
    DCLFATBPB              *pBPB,
    const DCLFATPARAMS     *pFFP)
{
    D_UINT32                ulFatSize;
    D_UINT32                ulNumClusters;
    D_UINT16                uSpc;

    DclAssert(pFFP);

    DCLPRINTF(1, ("DclFatBPBBuild() BPS=%U HiddenSecs=%lU TotalSecs=%lU RootSize=%U Heads=%U SPT=%U FATs=%U\n",
               pFFP->uBytesPerSector, pFFP->ulHiddenSectors, pFFP->ulTotalSectors,
               pFFP->uRootEntries, pFFP->uHeads, pFFP->uSPT, pFFP->uNumFATs));

    DclAssert(pBPB);
    DclAssert(pFFP->uBytesPerSector);
    DclAssert(pFFP->uNumFATs);
    DclAssert(pFFP->ulTotalSectors);
    DclAssert(pFFP->uRootEntries);
    DclAssert(pFFP->uRootEntries % 16 == 0);

    DclMemSet(pBPB, 0, sizeof *pBPB);

    pBPB->uSides                = pFFP->uHeads;
    pBPB->uSecPerTrack          = pFFP->uSPT;
    pBPB->ulHidden              = pFFP->ulHiddenSectors;
    pBPB->uMaxDirs              = pFFP->uRootEntries;
    pBPB->bNumFats              = (D_BYTE) pFFP->uNumFATs;
    pBPB->uBytesPerSector       = pFFP->uBytesPerSector;
    pBPB->uReservedSectors      = 1;
    pBPB->bMediaDescrip         = FAT_MEDIAID_HD;
    pBPB->ext.bpb16.bPhysDrive  = 0x80;
    pBPB->ext.bpb16.bUnused     = 0x00;
    pBPB->ext.bpb16.bSignature  = 0x29;

    /*  Copy in the label, we only have 11 bytes in the BPB
    */
    DclMemCpy(&pBPB->ext.bpb16.acLabel[0], pFFP->szVolumeLabel, sizeof(pBPB->ext.bpb16.acLabel));

    /*  We must use DclMemCpy() here to avoid any 32 bit alignment
        problems.  ulSize is on an odd boundary and some platforms
        cannot store a 32 bit value into an odd offset.
    */
    DclMemCpy(&pBPB->ext.bpb16.ulSerialNumber, &pFFP->ulSerialNumber, sizeof pBPB->ext.bpb16.ulSerialNumber);

    /*  Determine the system ID
    */
    if (pFFP->ulTotalSectors <= FAT12_MAXSECTORS)
        DclMemCpy(pBPB->ext.bpb16.acSysID, "FAT12   ", 8);
    else
        DclMemCpy(pBPB->ext.bpb16.acSysID, "FAT16   ", 8);

    /*  Set the number of sectors in the partition.  If it fits in
        sixteen bits, it gets stored in the 16-bit sector count field,
        and the 32-bit sector count is zero; if not, it's the other
        way around.  This same boundary is used to set the partition
        type ID when building the partition table in the MBR.
    */
    if(pFFP->ulTotalSectors < MIN_DOSBIG_SECS)
    {
        pBPB->uTotalSecs = (D_UINT16) pFFP->ulTotalSectors;
        pBPB->ulTotalSecs = 0L;
    }
    else
    {
        pBPB->uTotalSecs = 0;
        pBPB->ulTotalSecs = pFFP->ulTotalSectors;
    }

    /*  Find the best fit for the number of clusters

        NOTE: these calculations are an approximation that errs on the
        safe side.  The actual number of clusters when the partition
        is finally formatted may be fewer than calculated here because
        the space allocated to data storage (and calculated in
        clusters) is decreased by the reserved space (typically the
        boot block), the FATs, and the root directory.  This means
        that the chosen cluster size will be larger than necessary at
        disk sizes just below the proper boundary.  Oh, well.
    */
    if(pFFP->ulTotalSectors > FAT12_MAXSECTORS)
    {
        if (pFFP->ulTotalSectors <= FAT16_SMALL_CLUSTER_LIMIT)
        {
            /*  Disks smaller than about 16 MB use 2 sectors/cluster
                for FAT16 (Microsoft recommendation).
            */
            uSpc = 2;
            ulNumClusters = pFFP->ulTotalSectors / uSpc;
        }
        else
        {
            /*  For larger disks, start at 4 sectors/cluster for FAT16
                (Microsoft recommendation).
            */
            uSpc = 4;
            ulNumClusters = pFFP->ulTotalSectors / uSpc;

            /*  We need to be sure the SPC will fit into our FAT
             */
            while (ulNumClusters > FAT16_MAXCLUSTERS)
            {
                uSpc *= 2;
                ulNumClusters /= 2;
            }
        }
        ulFatSize = (ulNumClusters + 2) * 2;
    }
    else
    {
        uSpc = 1;
        ulNumClusters = pFFP->ulTotalSectors / uSpc;

        /*  We need to be sure the SPC will fit into our FAT
        */
        while(ulNumClusters > FAT12_MAXCLUSTERS)
        {
            uSpc *= 2;
            ulNumClusters /= 2;
        }
        ulFatSize = ((ulNumClusters + 2) * 3 + 1) / 2;
    }

    /*  Record the size of the FAT and the SPC
    */
    pBPB->bSecPerCluster = (D_UCHAR) uSpc;
    pBPB->uSecPerFAT = (D_UINT16) ((ulFatSize + pFFP->uBytesPerSector - 1)
                                         / pFFP->uBytesPerSector);

  #if D_DEBUG > 1
    DclFatBPBDisplay(pBPB);
  #endif

    /*  Validate the BPB that was created
    */
    {
        D_UINT32 ulDataStart;

        /*  Calculate the start of the data area
        */
        ulDataStart = pBPB->uReservedSectors;
        ulDataStart += (D_UINT32)pBPB->uSecPerFAT * pBPB->bNumFats;
        ulDataStart += (D_UINT32)pBPB->uMaxDirs / (pFFP->uBytesPerSector / FAT_DIRENTRYSIZE);

        /*  Make sure that the FAT media layout seems reasonable
        */
        if((ulDataStart + pBPB->bSecPerCluster) > pFFP->ulTotalSectors)
        {
            /*  The FAT layout is not valid since there is not room
                on the media for at least one cluster
            */
            return FALSE;
        }
    }

    return TRUE;
}


