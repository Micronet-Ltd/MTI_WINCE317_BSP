/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module contains functions for manipulating disk MBRs and partitions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlmbr.c $
    Revision 1.1  2011/02/09 00:40:16Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlmbr.h>


/*-----------------------------------------------
    Byte offsets for each field in a partition
    table entry.  These defines are private to
    this module, since ALL code which reads or
    writes MBRs should be using these functions
    to load and store the data.
-----------------------------------------------*/
#define DCLPTE_OFFSET_BOOTABLE      (0)
#define DCLPTE_OFFSET_BEGINHEAD     (1)
#define DCLPTE_OFFSET_BEGINSECTOR   (2)
#define DCLPTE_OFFSET_BEGINCYLINDER (3)
#define DCLPTE_OFFSET_FILESYSID     (4)
#define DCLPTE_OFFSET_ENDHEAD       (5)
#define DCLPTE_OFFSET_ENDSECTOR     (6)
#define DCLPTE_OFFSET_ENDCYLINDER   (7)
#define DCLPTE_OFFSET_STARTSECTOR   (8)
#define DCLPTE_OFFSET_SECTORCOUNT  (12)


/*-------------------------------------------------------------------
    Protected: DclMBRPartitionLoad()

    Move a partition table entry from the packed MBR format, into
    a DCLDISKPARTITION structure.

    Parameters:
        pPart - A pointer to the DCLDISKPARTITION structure to use
        pData - A pointer to the packed partition entry information.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclMBRPartitionLoad(
    DCLDISKPARTITION   *pPart,
    const D_BUFFER     *pData)
{
    DclAssertWritePtr(pPart, sizeof(*pPart));
    DclAssertReadPtr(pData, DCLMBR_PARTITION_SIZE);

    /*  The on-media partition entry has unaligned data.  To avoid
        address exceptions or faults on CPUs that cannot address 
        unaligned data, we need to move each element using memcpy().

        Additionally we must change from the little endian format
        used in the raw data, to the native format.
    */
    DCLLE2NE((D_BUFFER*)&pPart->bIsBootable,   &pData[DCLPTE_OFFSET_BOOTABLE],      sizeof(pPart->bIsBootable));
    DCLLE2NE((D_BUFFER*)&pPart->bStartHead,    &pData[DCLPTE_OFFSET_BEGINHEAD],     sizeof(pPart->bStartHead));
    DCLLE2NE((D_BUFFER*)&pPart->bStartSector,  &pData[DCLPTE_OFFSET_BEGINSECTOR],   sizeof(pPart->bStartSector));
    DCLLE2NE((D_BUFFER*)&pPart->bStartTrack,   &pData[DCLPTE_OFFSET_BEGINCYLINDER], sizeof(pPart->bStartTrack));
    DCLLE2NE((D_BUFFER*)&pPart->bFileSysID,    &pData[DCLPTE_OFFSET_FILESYSID],     sizeof(pPart->bFileSysID));
    DCLLE2NE((D_BUFFER*)&pPart->bEndHead,      &pData[DCLPTE_OFFSET_ENDHEAD],       sizeof(pPart->bEndHead));
    DCLLE2NE((D_BUFFER*)&pPart->bEndSector,    &pData[DCLPTE_OFFSET_ENDSECTOR],     sizeof(pPart->bEndSector));
    DCLLE2NE((D_BUFFER*)&pPart->bEndTrack,     &pData[DCLPTE_OFFSET_ENDCYLINDER],   sizeof(pPart->bEndTrack));
    DCLLE2NE((D_BUFFER*)&pPart->ulStartSector, &pData[DCLPTE_OFFSET_STARTSECTOR],   sizeof(pPart->ulStartSector));
    DCLLE2NE((D_BUFFER*)&pPart->ulSectorCount, &pData[DCLPTE_OFFSET_SECTORCOUNT],   sizeof(pPart->ulSectorCount));

    return;
}


/*-------------------------------------------------------------------
    Protected: DclMBRPartitionStore()

    Move partition table information from a DCLDISKPARTITION structure
    into a packed MBR format, as required on the media.

    Parameters:
        pData - A pointer to the packed partition entry to fill.
        pPart - A pointer to the DCLDISKPARTITION data to copy.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclMBRPartitionStore(
    D_BUFFER               *pData,
    const DCLDISKPARTITION *pPart)
{
    DclAssertWritePtr(pData, DCLMBR_PARTITION_SIZE);
    DclAssertReadPtr(pPart, sizeof(*pPart));

    /*  Now copy all elements into the correct location in the actual
        boot sector, and change the data to little endian format.
    */
    DCLNE2LE(pData + DCLPTE_OFFSET_BOOTABLE,      (D_BUFFER*)&pPart->bIsBootable,   sizeof(pPart->bIsBootable));
    DCLNE2LE(pData + DCLPTE_OFFSET_BEGINHEAD,     (D_BUFFER*)&pPart->bStartHead,    sizeof(pPart->bStartHead));
    DCLNE2LE(pData + DCLPTE_OFFSET_BEGINSECTOR,   (D_BUFFER*)&pPart->bStartSector,  sizeof(pPart->bStartSector));
    DCLNE2LE(pData + DCLPTE_OFFSET_BEGINCYLINDER, (D_BUFFER*)&pPart->bStartTrack,   sizeof(pPart->bStartTrack));
    DCLNE2LE(pData + DCLPTE_OFFSET_FILESYSID,     (D_BUFFER*)&pPart->bFileSysID,    sizeof(pPart->bFileSysID));
    DCLNE2LE(pData + DCLPTE_OFFSET_ENDHEAD,       (D_BUFFER*)&pPart->bEndHead,      sizeof(pPart->bEndHead));
    DCLNE2LE(pData + DCLPTE_OFFSET_ENDSECTOR,     (D_BUFFER*)&pPart->bEndSector,    sizeof(pPart->bEndSector));
    DCLNE2LE(pData + DCLPTE_OFFSET_ENDCYLINDER,   (D_BUFFER*)&pPart->bEndTrack,     sizeof(pPart->bEndTrack));
    DCLNE2LE(pData + DCLPTE_OFFSET_STARTSECTOR,   (D_BUFFER*)&pPart->ulStartSector, sizeof(pPart->ulStartSector));
    DCLNE2LE(pData + DCLPTE_OFFSET_SECTORCOUNT,   (D_BUFFER*)&pPart->ulSectorCount, sizeof(pPart->ulSectorCount));
 
    return;
}



/*-------------------------------------------------------------------
    Protected: DclMBRPartitionLoadAll()

    Validate an MBR and load the packed partition table entries into
    an array of DCLDISKPARTITION structures.  

    *Note* -- This function assumes that an MBR is 512 bytes long,
              even if a physical block device sector is longer.  

    Parameters:
        paPart - A pointer to the first of nCount DCLDISKPARTITION
                 structures.
        pMBR   - A pointer to the raw MBR data, at least 512 bytes
                 in length.
        nCount - The number of partition entries to read, typically 4.
 
    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclMBRPartitionLoadAll(
    DCLDISKPARTITION   *paPart,      
    const D_BUFFER     *pMBR,
    unsigned            nCount)  
{
    unsigned            nOffset;
    unsigned            nPart;
    D_UINT16            uSignature;
    
    DclAssert(nCount);
    DclAssertWritePtr(paPart, nCount * sizeof(*paPart));
    DclAssertReadPtr(pMBR, DCLMBR_LENGTH);
    DclAssert(DCLMBR_SIGNATURE_SIZE == sizeof(uSignature));

    nOffset = DCLMBR_LENGTH - DCLMBR_SIGNATURE_SIZE;

    DCLLE2NE(&uSignature, &pMBR[nOffset], DCLMBR_SIGNATURE_SIZE);

    if(uSignature != DCLMBR_SIGNATURE)
        return DCLSTAT_MBR_SIGNATUREINVALID;

    nOffset -= (DCLMBR_PARTITION_SIZE * nCount);

    for(nPart = 0; nPart < nCount; nPart++)
    {
        DclMBRPartitionLoad(paPart, &pMBR[nOffset]);

        nOffset += DCLMBR_PARTITION_SIZE;
        paPart++;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Protected: DclMBRPartitionDisplay()

    Display disk partition information.

    Parameters:
        pPTE     - A pointer to the DCLDISKPARTITION structure to
                   display
        fVerbose - A flag indicating whether the CHS values should
                   be displayed or not.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclMBRPartitionDisplay(
    const DCLDISKPARTITION *pPart,
    D_BOOL                  fVerbose)
{
    DclAssertReadPtr(pPart, sizeof(*pPart));

    DclPrintf("Partition Entry Bootable=%02X FileSysID=%02X StartSec=%lX SecCount=%lX\n",
         pPart->bIsBootable, pPart->bFileSysID, pPart->ulStartSector, pPart->ulSectorCount);

    if(fVerbose)
    {
        DclPrintf("          Beginning Head=%02X Sector=%02X Cylinder=%02X\n", pPart->bStartHead, pPart->bStartSector, pPart->bStartTrack);
        DclPrintf("             Ending Head=%02X Sector=%02X Cylinder=%02X\n", pPart->bEndHead, pPart->bEndSector, pPart->bEndTrack);
    }

    return;
}


/*----------------------------------------------------------------------
    Protected: DclMBRPartitionDisplayTable()

    Display a disk partition table contained in the specified MBR.

    Parameters:
        pMBR     - A pointer to the buffer containing the MBR in the
                   packed, on-disk format.
        fVerbose - A flag indicating whether the CHS values should be
                   displayed or not.

    Return Value:
        None.
----------------------------------------------------------------------*/
void DclMBRPartitionDisplayTable(
    const D_BUFFER *pMBR,
    D_BOOL          fVerbose)
{
    unsigned        nn;
    unsigned        nOffset;

    DclAssertReadPtr(pMBR, DCLMBR_LENGTH);

    nOffset = DCLMBR_PARTITION_OFFSET;

    for(nn = 0; nn < DCLMBR_PARTITION_COUNT; nn++)
    {
        DCLDISKPARTITION  pte;

        /*  Move the data from the packed buffer into the standard structure
        */
        DclMBRPartitionLoad(&pte, &pMBR[nOffset]);
        DclMBRPartitionDisplay(&pte, fVerbose);

        nOffset += DCLMBR_PARTITION_SIZE;
    }

    return;
}




