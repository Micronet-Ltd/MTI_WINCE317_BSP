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

    This module contains functions for obtaining the characteristics of a
    FAT formatted disk.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fatinfo.c $
    Revision 1.17  2009/07/28 18:48:41Z  garyp
    Merged from the v4.0 branch.  Updated to use some reorganized FAT 
    functionality and modified to use the cleaned up and refactored BPB 
    structures.
    Revision 1.16  2009/03/31 19:49:15Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.15  2009/03/20 20:17:16Z  billr
    Resolve bug 2275: FAT/MBR format code appears to be broken wrt FAT12,
    FAT16, BIGFAT choices.
    Revision 1.14  2009/02/13 00:00:59Z  garyp
    Updated to use the enhanced DclFatBPBMove() function.
    Revision 1.13  2009/02/09 01:20:33Z  garyp
    Updated to include dlpartid.h.
    Revision 1.12  2008/01/13 07:26:46Z  keithg
    Function header updates to support autodoc.
    Revision 1.11  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.10  2007/09/28 23:00:22Z  jeremys
    Renamed a header file.
    Revision 1.9  2007/09/26 23:59:59Z  jeremys
    Renamed a header file and two function names.
    Revision 1.8  2006/10/18 23:17:42Z  Garyp
    Minor header changes.
    Revision 1.7  2006/08/08 00:43:57Z  Garyp
    Documentation and debug code fixes.
    Revision 1.6  2006/07/31 17:32:31Z  Pauli
    Corrected to use C-style comments.
    Revision 1.5  2006/07/27 21:06:34Z  johnb
    Added code to check type of FAT16 partition and to return the
    appropriate type.
    Revision 1.4  2006/06/21 23:11:24Z  Garyp
    Documentation updates -- no functional changes.
    Revision 1.3  2006/02/21 03:25:59Z  Garyp
    Minor type changes.
    Revision 1.2  2006/02/03 16:12:00Z  johnb
    Added FATMON FAT32 Support
    Revision 1.1  2005/10/04 02:09:20Z  Pauli
    Initial revision
    Revision 1.4  2004/12/30 17:32:43Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.3  2004/09/23 05:50:01Z  GaryP
    Changed the device sector length value to be 32-bits.
    Revision 1.2  2004/08/30 20:26:46Z  GaryP
    Changed the file number.
    Revision 1.1  2004/08/09 23:18:00Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_FATSUPPORT

#include <dlfat.h>
#include <dlpartid.h>
#include <diskapi.h>
#include <fxfatapi.h>


/*-------------------------------------------------------------------
    Public: FfxFatGetDiskParamsFromBootRecord()

    This routine is used to obtain the disk characteristics (CHS
    values) from a FAT boot record.

    Parameters:
        pBuffer - A pointer to a byte array containing the potential
                  FAT boot record.
        pPDI    - A pointer to the FFXPHYSICALDISKINFO structure to
                  use.

    Return Value:
        Returns the FAT file system ID if successful, in which case
        the disk geometry fields in the FFXPHYSICALDISKINFO structure
        will be filled, or FSID_NONE otherwise, in which case the
        FFXPHYSICALDISKINFO will not be modified.
-------------------------------------------------------------------*/
D_BYTE FfxFatGetDiskParamsFromBootRecord(
    const D_BUFFER         *pBuffer,
    FFXPHYSICALDISKINFO    *pPDI)
{
    DCLFATBPB               bpb;
    D_BYTE                  bFSID;

    DclAssert(pBuffer);
    DclAssert(pPDI);

    bFSID = DclFatBPBMove(&bpb, pBuffer);

    if((bFSID != FSID_FAT12) && (bFSID != FSID_FAT16) && (bFSID != FSID_FAT32))
    {
        FFXPRINTF(1, ("Not a FAT boot record\n"));
        bFSID = FSID_NONE;
    }
    else
    {
        pPDI->ulBytesPerSector = bpb.uBytesPerSector;

        if(bpb.uTotalSecs)
            pPDI->ulTotalSectors = bpb.uTotalSecs;
        else
            pPDI->ulTotalSectors = bpb.ulTotalSecs;

        pPDI->uHeads = bpb.uSides;
        pPDI->uSectorsPerTrack = bpb.uSecPerTrack;
        pPDI->uCylinders = (D_UINT16)
            (pPDI->ulTotalSectors / ((D_UINT32)pPDI->uHeads * pPDI->uSectorsPerTrack));

        if(bFSID == FSID_FAT16)
        {
            /*  Determine if this FAT16 partition (type 4) is really a
                BIGFAT partition (type 6).
            */
            if(pPDI->ulTotalSectors >= MIN_DOSBIG_SECS)
                bFSID = FSID_BIGFAT;
        }
    }

    FFXPRINTF(1, ("FfxFatGetDiskParamsFromBootRecord() returning FSID=%u\n", bFSID));

    return bFSID;
}


#endif
