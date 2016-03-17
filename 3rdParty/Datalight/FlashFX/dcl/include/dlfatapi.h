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

    This header contains the prototypes, structures, and types necessary
    to use the FAT related API functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlfatapi.h $
    Revision 1.6  2009/06/28 01:24:07Z  garyp
    Finished the merge from the v4.0 branch.  Moved BPB definitions into this
    header from dlfat.h.
    Revision 1.5  2009/02/13 20:19:07Z  garyp
    Partial merge from the v4.0 branches.  Renamed the functions.  Modified
    DclFatBPBMove() to return the FS ID.  Updated so that DclFATBPBDisplay()
    is available in RELEASE mode.
    Revision 1.4  2008/05/03 20:02:29Z  garyp
    Fixed to declare the prototype for DclFatDisplayBPB() only if running in
    debug mode.
    Revision 1.3  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.2  2007/09/27 19:54:30Z  jeremys
    Removed a #error condition that no longer applies to this header.
    Revision 1.1  2007/09/27 01:26:04Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLFATAPI_H_INCLUDED
#define DLFATAPI_H_INCLUDED

#define FAT_VOLUME_LEN      (11)    /* size of the volume label field */
#define FAT_SYSID_LEN        (8)    /* size of the system ID field */


/*-------------------------------------------------------------------
    This structure supplies FAT specific information about a disk.
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT32        ulHiddenSectors;
    D_UINT32        ulTotalSectors;
    D_UINT32        ulSerialNumber;
    char            szVolumeLabel[12];
    D_UINT16        uHeads;
    D_UINT16        uSPT;
    D_UINT16        uCylinders;
    D_UINT16        uRootEntries;
    D_UINT16        uNumFATs;
    D_UINT16        uBytesPerSector;
} DCLFATPARAMS;


/*-------------------------------------------------------------------
    Standard BIOS Parameter Block, Version 4.

    WARNING: The offsets in the comments below apply only to the
             packed form of the structures on disk, and are relative
             to start of the boot record.  They must NOT be applied
             to the in memory representation of the structure.
-------------------------------------------------------------------*/
typedef struct
{
    D_BYTE      bPhysDrive;             /* 24h - Physical drive number          */
    D_BYTE      bUnused;                /* 25h - 0x00 unused byte, reserved     */
    D_BYTE      bSignature;             /* 26h - signature                      */
    D_UINT32    ulSerialNumber;         /* 27h - Unique serial number           */
    char        acLabel[FAT_VOLUME_LEN];/* 2Bh - Volume label                   */
    char        acSysID[FAT_SYSID_LEN]; /* 36h - FAT12, FAT16, ...              */
} DCLFATBPB16;

/*-------------------------------------------------------------------
    Standard BIOS Parameter Block, Version 4 with added FAT32 fields.

    WARNING: The offsets in the comments below apply only to the
             packed form of the structures on disk, and are relative
             to start of the boot record.  They must NOT be applied
             to the in memory representation of the structure.
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT32    ulSectorsPerFat;        /* 24h - FAT32 sectors per FAT            */
    D_UINT16    uExtendedFlags;         /* 28h - FAT32 extended flags (defined below) */
    D_UINT16    uFileSystemVersion;     /* 2Ah - FAT32 File system version number */
    D_UINT32    ulRootDirStartCluster;  /* 2Ch - FAT32 Root dir start (can move)  */
    D_UINT16    uFSInfoSector;          /* 30h - FAT32 sector # of FSINFO struc  (-1 if none) */
    D_UINT16    uBackupBootSector;      /* 32h - FAT32 sector # of backup boot sector (-1 if none) */
    D_BYTE      abReserved[12];         /* 34h - FAT32 ???                      */
    D_BYTE      bPhysDrive;             /* 40h - Physical drive number          */
    D_BYTE      bUnused;                /* 41h - 0x00 unused byte, reserved     */
    D_BYTE      bSignature;             /* 42h - signature                      */
    D_UINT32    ulSerialNumber;         /* 43h - Unique serial number           */
    char        acLabel[FAT_VOLUME_LEN];/* 47h - Volume label                   */
    char        acSysID[FAT_SYSID_LEN]; /* 52h - FAT32, ...                     */
} DCLFATBPB32;

/*-------------------------------------------------------------------
    Unified BPB Structure

    WARNING: The offsets in the comments below apply only to the
             packed form of the structures on disk, and are relative
             to start of the boot record.  They must NOT be applied
             to the in memory representation of the structure.
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT16    uBytesPerSector;        /* 0Bh -                                  */
    D_BYTE      bSecPerCluster;         /* 0Dh -                                  */
    D_UINT16    uReservedSectors;       /* 0Eh -                                  */
    D_BYTE      bNumFats;               /* 10h -                                  */
    D_UINT16    uMaxDirs;               /* 11h - Floppies < 256, Always % 16      */
    D_UINT16    uTotalSecs;             /* 13h - If less than 32MB, 0 otherwise   */
    D_BYTE      bMediaDescrip;          /* 15h -                                  */
    D_UINT16    uSecPerFAT;             /* 16h - Calculated at format time        */
    D_UINT16    uSecPerTrack;           /* 18h - Number of sectors per track      */
    D_UINT16    uSides;                 /* 1Ah - Number of sides in the media     */
    D_UINT32    ulHidden;               /* 1Ch - Hidden sectors at start of disk  */
    D_UINT32    ulTotalSecs;            /* 20h - Valid if uTotalSecs is zero      */
    union
    {
        DCLFATBPB16 bpb16;
        DCLFATBPB32 bpb32;
    } ext;
} DCLFATBPB;


/*-------------------------------------------------------------------
---------------------------------------------------------------------
-----------------  Prototypes for Public Functions  -----------------
---------------------------------------------------------------------
-------------------------------------------------------------------*/

#define DclFatBootRecordBuild           DCLFUNC(DclFatBootRecordBuild)
#define DclFatBootRecordDeriveFSID      DCLFUNC(DclFatBootRecordDeriveFSID)
#define DclFatBPBBuild                  DCLFUNC(DclFatBPBBuild)
#define DclFatBPBDeriveFSID             DCLFUNC(DclFatBPBDeriveFSID)
#define DclFatBPBDisplay                DCLFUNC(DclFatBPBDisplay)
#define DclFatBPBMove                   DCLFUNC(DclFatBPBMove)

DCLSTATUS   DclFatBootRecordBuild(D_BUFFER *pSector, const DCLFATBPB *pBPB);
D_BYTE      DclFatBootRecordDeriveFSID(const D_BUFFER *pbBR, D_BOOL *pfIsTFAT);
DCLSTATUS   DclFatBPBBuild(DCLFATBPB *pBPB, const DCLFATPARAMS *pFatParams);
D_BYTE      DclFatBPBDeriveFSID(const DCLFATBPB *pBPB, D_BOOL *pfIsTFAT);
void        DclFatBPBDisplay(const DCLFATBPB *pBPB);
D_BYTE      DclFatBPBMove(DCLFATBPB *pBPB, const D_BUFFER *pSector);


#endif  /* DLFATAPI_H_INCLUDED */


