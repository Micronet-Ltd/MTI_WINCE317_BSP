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
  jurisdictions.  Patents may be pending.

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
    $Log: dlrelparam.c $
    Revision 1.4  2011/08/05 01:46:30Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.3  2007/11/03 23:31:12Z  Garyp
    Added the standard module header.
    Revision 1.2  2007/09/27 01:58:41Z  jeremys
    Fixed a typo in a string.
    Revision 1.1  2007/09/27 01:24:54Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#include <dlrelfs.h>
#include <dlrelapi.h>


/*-------------------------------------------------------------------
    Protected: DclRelGetLogicalDiskSize() 

    Analyze the specified block to determine if it is a Reliance
    MasterBlock, and if so, returns the number of logical blocks
    in the logical disk, as well as the logical block size.

    Parameters:
        pMasterBlock - A pointer to a byte array containing one
                       sector's worth of data.
        pulBlockSize - A pointer to a location in which to store the
                       logical block size.

    Return Value:
        Returns the number of logical blocks in the disk if it is a
        Reliance disk, or zero if it is not a Reliance disk.
-------------------------------------------------------------------*/
D_UINT32 DclRelGetLogicalDiskSize(
    const D_BUFFER *pMasterBlock,
    D_UINT32       *pulBlockSize,
    D_UINT32        ulSectorLen)
{
    D_UINT32       ulTotalBlocks = 0L;
    D_UINT32       ulBlockSize;
    D_UINT32       ulID;

    DCLPRINTF(1, ("DclRelGetLogicalDiskSize() SectorLen=%lU\n", ulSectorLen));

    if((ulSectorLen < (RELIANCE_ID_OFFSET + sizeof ulID)) ||
        (ulSectorLen < (RELIANCE_SECTOR_SIZE_OFFSET + sizeof ulID)) ||
        (ulSectorLen < (RELIANCE_TOTAL_SECTORS_OFFSET + sizeof ulID)))
    {
        /*  If the physical sector length is not long enough to
            contain the Reliance fields, this can't be valid.
        */
        return ulTotalBlocks;
    }

    DCLLE2NE((D_BUFFER*)&ulID, &pMasterBlock[RELIANCE_ID_OFFSET], sizeof ulID);

    if(ulID == RELIANCE_ID)
    {
        DCLLE2NE((D_BUFFER*)&ulBlockSize,   &pMasterBlock[RELIANCE_SECTOR_SIZE_OFFSET], sizeof ulBlockSize);
        DCLLE2NE((D_BUFFER*)&ulTotalBlocks, &pMasterBlock[RELIANCE_TOTAL_SECTORS_OFFSET], sizeof ulTotalBlocks);

        /*  If the Reliance sector size is smaller than the physical sector
            size, something is drastically wrong -- perhaps an old format
            on the disk.  Return failure.
        */
        if(ulBlockSize < ulSectorLen)
            return 0;

        /*  store the block size
        */
        *pulBlockSize = ulBlockSize;

        DCLPRINTF(1, ("Found Reliance signature, LogicalBlockSize=%lU TotalBlocks=%lU\n",
            ulBlockSize, ulTotalBlocks));
    }

    /*  Return the total number of blocks
    */
    return ulTotalBlocks;
}

