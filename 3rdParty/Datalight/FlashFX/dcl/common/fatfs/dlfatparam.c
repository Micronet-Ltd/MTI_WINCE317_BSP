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
    $Log: dlfatparam.c $
    Revision 1.5  2009/06/28 00:23:15Z  garyp
    Final merge from the v4.0 branch --  Updated to use the cleaned up and
    refactored BPB structures.
    Revision 1.4  2008/12/12 17:22:27Z  thomd
    Modify to allow larger BytesPerSector values in BPB
    Revision 1.3  2008/05/05 00:11:53Z  garyp
    Updated debug code.
    Revision 1.2  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/09/27 01:17:06Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#include <dlfatapi.h>
#include <dlpartid.h>
#include <dlfat.h>


/*-------------------------------------------------------------------
    Public: DclFatBootRecordDeriveFSID()

    Given a boot record, return the type of file system identified
    therein.

    Currently only supports FSID_FAT12, FSID_FAT16, and FSID_FAT32.
    Anything that cannot be determined to be one of the above will
    be identified as FSID_NONE.

    If the boot record is TFAT specific the pfIsTFAT value will be
    set to TRUE.

    Parameters:
        pcBR       - A pointer to a raw sector buffer containing a
                     potential FAT boot record.
        pfIsTFAT   - A pointer to a D_BOOL variable to receive a
                     TRUE/FALSE indicator.  May be NULL.

    Return Value:
        Returns the file system ID (FSID_*) byte.
-------------------------------------------------------------------*/
D_BYTE DclFatBootRecordDeriveFSID(
    const D_BUFFER *pcBR,
    D_BOOL         *pfIsTFAT)
{
    D_BYTE          bFSID = FSID_NONE;
    D_CHAR          achFSName[BT_SSID_LENGTH+1];
    D_UINT32        ulBytesPerSector;
    D_BOOL          fIsTFAT = FALSE;

    DclAssert(pcBR);

    /*  Known FAT disk Bytes per Sector is always a power of 2, starting at 512.
        If this looks proper, we can get the name, then the FSID from it.
    */
    ulBytesPerSector = pcBR[BT_BPB_OFF] + (pcBR[BT_BPB_OFF+1] << 8);
    if( DclBitCount(ulBytesPerSector) == 1 && ulBytesPerSector >= 512 )
    {
        /*  Verify the sector ID code
        */
        if(pcBR[BOOTREC_SIGNATURE_OFFSET] == 0x55 &&
            pcBR[BOOTREC_SIGNATURE_OFFSET + 1] == 0xAA)
        {
            DclMemCpy(&achFSName[0], &pcBR[BT16_SSID_OFF], BT_SSID_LENGTH);
            achFSName[BT_SSID_LENGTH] = 0;

            DCLPRINTF(2, ("FSID Name is \"%s\"\n", achFSName));

            if(!DclMemCmp(&achFSName[0], "FAT12   ", BT_SSID_LENGTH))
            {
                bFSID = FSID_FAT12;
            }
            else if(!DclMemCmp(&achFSName[0], "FAT16   ", BT_SSID_LENGTH))
            {
                bFSID = FSID_FAT16;
            }
            else if(!DclMemCmp(&achFSName[0], "TFAT12  ", BT_SSID_LENGTH))
            {
                fIsTFAT = TRUE;
                bFSID = FSID_FAT12;
            }
            else if(!DclMemCmp(&achFSName[0], "TFAT16  ", BT_SSID_LENGTH))
            {
                fIsTFAT = TRUE;
                bFSID = FSID_FAT16;
            }
            else
            {
                /*  Try the FAT32 file system name
                */
                DclMemCpy(&achFSName[0], &pcBR[BT32_SSID_OFF], BT_SSID_LENGTH);

                if(!DclMemCmp(&achFSName[0], "FAT32   ", BT_SSID_LENGTH))
                {
                    bFSID = FSID_FAT32;
                }
                else if(!DclMemCmp(&achFSName[0], "TFAT32  ", BT_SSID_LENGTH))
                {
                    fIsTFAT = TRUE;
                    bFSID = FSID_FAT32;
                }
                else
                {
                    DCLPRINTF(1, ("Unrecognizable file system in boot record\n"));
                }

              #if D_DEBUG
                if(bFSID == FSID_FAT32)
                {
                    /*  Reality Check -- RootDirEntries and SectorsPerFat
                        are zero on FAT32 drives.
                    */
                    DclAssert(pcBR[BT_MAXD_OFF] == 0);
                    DclAssert(pcBR[BT_MAXD_OFF + 1] == 0);
                    DclAssert(pcBR[BT_SPF_OFF] == 0);
                    DclAssert(pcBR[BT_SPF_OFF + 1] == 0);
                }
              #endif
            }
        }
    }

    if(pfIsTFAT)
        *pfIsTFAT = fIsTFAT;

    DCLPRINTF(2, ("DlFatDeriveFSIDFromBootRecord() FSID=%u IsTFAT=%U\n", bFSID, fIsTFAT));

    return bFSID;
}


/*-------------------------------------------------------------------
    Public: DclFatBPBDeriveFSID()

    Given a BPB, return the type of file system identified therein.

    Currently only supports FSID_FAT12, FSID_FAT16, and FSID_FAT32.
    Anything that cannot be determined to be one of the above will
    be identified as FSID_NONE.

    If the boot record is TFAT specific the pfIsTFAT value will be
    set to TRUE.

    Parameters:
        pBPB       - A pointer to the DCLFATBPB structure to use.
        pfIsTFAT   - A pointer to a D_BOOL variable to receive a
                     TRUE/FALSE indicator.  May be NULL.

    Return Value:
        Returns the file system ID (FSID_*) byte.
-------------------------------------------------------------------*/
D_BYTE DclFatBPBDeriveFSID(
    const DCLFATBPB    *pBPB,
    D_BOOL             *pfIsTFAT)
{
    D_BYTE              bFSID = FSID_NONE;
    D_BOOL              fIsTFAT = FALSE;

    DclAssert(pBPB);

    if(!DclMemCmp(pBPB->ext.bpb16.acSysID, "FAT12   ", BT_SSID_LENGTH))
    {
        bFSID = FSID_FAT12;
    }
    else if(!DclMemCmp(pBPB->ext.bpb16.acSysID, "FAT16   ", BT_SSID_LENGTH))
    {
        bFSID = FSID_FAT16;
    }
    else if(!DclMemCmp(pBPB->ext.bpb16.acSysID, "TFAT12  ", BT_SSID_LENGTH))
    {
        fIsTFAT = TRUE;
        bFSID = FSID_FAT12;
    }
    else if(!DclMemCmp(pBPB->ext.bpb16.acSysID, "TFAT16  ", BT_SSID_LENGTH))
    {
        fIsTFAT = TRUE;
        bFSID = FSID_FAT16;
    }
    else
    {
        /*  Try the FAT32 file system name
        */

        if(!DclMemCmp(pBPB->ext.bpb32.acSysID, "FAT32   ", BT_SSID_LENGTH))
        {
            bFSID = FSID_FAT32;
        }
        else if(!DclMemCmp(pBPB->ext.bpb32.acSysID, "TFAT32  ", BT_SSID_LENGTH))
        {
            fIsTFAT = TRUE;
            bFSID = FSID_FAT32;
        }
        else
        {
            DCLPRINTF(1, ("Unrecognizable file system in boot record\n"));
        }

      #if D_DEBUG
        if(bFSID == FSID_FAT32)
        {
            /*  Reality Check -- RootDirEntries and SectorsPerFat
                are zero on FAT32 drives.
            */
            DclAssert(pBPB->uMaxDirs == 0);
            DclAssert(pBPB->uSecPerFAT == 0);

        }
      #endif
    }

    if(pfIsTFAT)
        *pfIsTFAT = fIsTFAT;

    DCLPRINTF(1, ("DclFatBPBDeriveFSID() FSID=%u IsTFAT=%U\n", bFSID, fIsTFAT));

    return bFSID;
}

