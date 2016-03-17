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

    Provides defines, types, and structures used by FAT file systems.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlfat.h $
    Revision 1.5  2009/06/28 01:41:06Z  garyp
    Finished the merge from the v4.0 branch.  Moved the BPB definitions to
    dlfatapi.h.
    Revision 1.4  2009/03/20 23:09:13Z  billr
    Resolve bug 2275: FAT/MBR format code appears to be broken wrt FAT12,
    FAT16, BIGFAT choices.
    Revision 1.3  2009/02/13 20:25:56Z  garyp
    Partial merge from the v4.0 branches.  General cleanup.
    Revision 1.2  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/09/27 01:26:00Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/
#ifndef DLFAT_H_INCLUDED
#define DLFAT_H_INCLUDED

/*-------------------------------------------------------------------
                     Boot Record Specific Stuff
-------------------------------------------------------------------*/
#define FAT_SECTORLEN            (512)  /* Standard FAT sector size             */
#define FAT_DIRENTRYSIZE          (32)  /* Size of a packed directory entry     */
#define FAT_MEDIAID_MIN         (0xF0)  /* Minimum media ID byte value          */
#define FAT_MEDIAID_HD          (0xF8)  /* Hard disk media ID                   */
#define FAT12_MAXCLUSTERS      (0xFF6)  /* Max clusters for 12 bit FATs (4086 and less) */
#define FAT16_MAXCLUSTERS     (0xFFF6)  /* Max clusters for 16 bit FATs         */

#if 0
/* Obsolete stuff to be eliminated when merging is complete */
#define FAT16_MAXBYTES (64UL * 1024UL)  /* Max size in bytes of the FAT         */
#define FAT12_MAXBYTES (16UL * 1024UL)  /* Max size in bytes of the FAT         */
#define FAT12_MAXSECTORS    ((16UL * 1024UL * 1024UL / FAT_SECTORLEN) - 1) */
#define FAT16_MAXSECTORS    ((32UL * 1024UL * 1024UL / FAT_SECTORLEN) - 1)
#endif

/*  This is the largest disk that we will build with a 12 bit FAT
    Disks larger than this will be formatted with a 16 bit FAT.
    This value is recommended by Microsoft in a Hardware White Paper
    "FAT: General Overview of On-Disk Format", version 1.03.
*/
#define FAT12_MAXSECTORS     (8400U)

/*  The same Microsoft document recommends that 2 sectors/cluster
    only be used for smaller disks (~16 MB, assuming 512 byte sector).
*/
#define FAT16_SMALL_CLUSTER_LIMIT (32680U)

/*  The limit at which a FAT16 file system is considered to be "big"
    (partition type id 0x06 instead of 0x04) is defined by the sector
    count.  If it cannot be represented in 16 bits, it must be marked
    as a "big" FAT16 file system.
*/
#define MIN_DOSBIG_SECS    (((D_UINT32) 1) << 16)



/*  The following are byte offsets for each of the boot sector
    entries.  These are useful in non-little-endian systems that
    cannot access words on odd boundaries.
*/
#define BT_JMPO_OFF      0     /* 00h */
#define BT_BTID_OFF      3     /* 03h */
#define BT_BPB_OFF      11     /* 0Bh */
#define BT_BPS_OFF      11     /* 0Bh */
#define BT_SPC_OFF      13     /* 0Dh */
#define BT_RES_OFF      14     /* 0Eh */
#define BT_NFAT_OFF     16     /* 10h */
#define BT_MAXD_OFF     17     /* 11h */
#define BT_UTSC_OFF     19     /* 13h */
#define BT_MDID_OFF     21     /* 15h */
#define BT_SPF_OFF      22     /* 16h */
#define BT_SPT_OFF      24     /* 18h */
#define BT_SIDS_OFF     26     /* 1Ah */
#define BT_HIDN_OFF     28     /* 1Ch */
#define BT_LTSC_OFF     32     /* 20h */
#define BT_SIG_OFF     510


/*  Byte offset for FAT12/16 boot records
*/
#define BT16_PDRV_OFF   36     /* 24h */
#define BT16_UNSD_OFF   37     /* 25h */
#define BT16_SIGN_OFF   38     /* 26h */
#define BT16_SRNM_OFF   39     /* 27h */
#define BT16_LABL_OFF   43     /* 2Bh */
#define BT16_SSID_OFF   54     /* 36h */


/*  Byte offset for FAT32 boot records
*/
#define BT32_SPRF_OFF   36     /* 24h */
#define BT32_EXFL_OFF   40     /* 28h */
#define BT32_FSVR_OFF   42     /* 2Ah */
#define BT32_RDSC_OFF   44     /* 2Ch */
#define BT32_FSIS_OFF   48     /* 30h */
#define BT32_BUBS_OFF   50     /* 32h */
#define BT32_RESV_OFF   52 	   /* 34h */
#define BT32_PDRV_OFF   64     /* 40h */
#define BT32_UNSD_OFF   65     /* 41h */
#define BT32_SIGN_OFF   66     /* 42h */
#define BT32_SRNM_OFF   67     /* 43h */
#define BT32_LABL_OFF   71     /* 47h */
#define BT32_SSID_OFF   82     /* 52h */

/*  Length of the file system name string (always padded with spaces)
*/
#define BT_SSID_LENGTH  8


/*-------------------------------------------------------------------
                     Directory Entry Stuff
-------------------------------------------------------------------*/

#define DIR_ATTR_READONLY   0x01
#define DIR_ATTR_HIDDEN     0x02
#define DIR_ATTR_SYSTEM     0x04
#define DIR_ATTR_VOLUME     0x08
#define DIR_ATTR_SUBDIR     0x10
#define DIR_ATTR_ARCHIVE    0x20
#define DIR_ATTR_LFN        0x0F
#define DIR_ATTR_UNKNOWN    0xC0

/*  Field offsets in the FATDIRECTORYENTRY struct
*/
#define DIR_NAME_OFF        0x00
#define DIR_EXT_OFF         0x08
#define DIR_ATTR_OFF        0x0B
#define DIR_TIME_OFF        0x16
#define DIR_DATE_OFF        0x18
#define DIR_START_OFF       0x1A
#define DIR_SIZE_OFF        0x1C
#define DIR_NAME_LEN           8
#define DIR_EXT_LEN            3

/*  Shift and mask values for directory entry time and date encoding
*/
#define FAT_DIRHOURSHIFT    (11)
#define FAT_DIRMINUTESHIFT   (5)
#define FAT_DIRSECONDSHIFT   (0)
#define FAT_DIRYEARSHIFT     (9)
#define FAT_DIRMONTHSHIFT    (5)
#define FAT_DIRDAYSHIFT      (0)

#endif /* DLFAT_H_INCLUDED */


