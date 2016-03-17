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
    to use the MBR related API functions in DCL.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlmbr.h $
    Revision 1.1  2011/02/09 00:40:14Z  garyp
    Initial revision
---------------------------------------------------------------------------*/
#ifndef DLMBR_H_INCLUDED
#define DLMBR_H_INCLUDED


/*-------------------------------------------------------------------
-------------                                             -----------
-------------  Structures and Types for Public Functions  -----------
-------------                                             -----------
-------------------------------------------------------------------*/

#define DCLMBR_LENGTH               512 /* Standard MBR length, regardless of the block device sector size */
#define DCLMBR_TWOBYTEJUMP         0xEB /* The first byte of an x86 MBR     */
#define DCLMBR_THREEBYTEJUMP       0xE9 /* is one of these two values.      */
#define DCLMBR_PARTITION_OFFSET  0x01BE /* Offset of first partition table entry  */
#define DCLMBR_PARTITION_COUNT        4 /* Partition table entries per MBR  */
#define DCLMBR_PARTITION_SIZE        16 /* Size of each partition table entry (packed) */     
#define DCLMBR_PARTITION_BOOTABLE  0x80 /* Indicates that a given partition is bootable */     
#define DCLMBR_SIGNATURE         0xAA55 /* Native-endian MBR and boot record signature */
#define DCLMBR_SIGNATURE_SIZE         2


/*-------------------------------------------------------------------
    Disk partition information.   
-------------------------------------------------------------------*/
typedef struct
{
    /*  Note that the field order in this structure is intentionally 
        different from the on-media order to prevent unsafe use in 
        assuming the structure fields are packed like they are on
        the media.
    */        
    D_UINT32    ulStartSector;  /* logical start of partition     */
    D_UINT32    ulSectorCount;  /* size of partition in sectors   */
    D_BYTE      bIsBootable;    /* 00H=non, 80H=bootable          */
    D_BYTE      bStartHead;     /* starting head of partition     */
    D_BYTE      bStartSector;   /* starting sector of partition   */
    D_BYTE      bStartTrack;    /* starting cylinder of partition */
    D_BYTE      bFileSysID;     /* 0=unused, 1,4,5,6=FAT          */
    D_BYTE      bEndHead;       /* ending head of partition       */
    D_BYTE      bEndSector;     /* ending sector of partition     */
    D_BYTE      bEndTrack;      /* ending cylinder of partition   */
} DCLDISKPARTITION;


/*-------------------------------------------------------------------
-------------                                            ------------
-------------       Prototypes for Public Functions      ------------
-------------                                            ------------
-------------------------------------------------------------------*/

#define     DclMBRPartitionDisplay       DCLFUNC(DclMBRPartitionDisplay)
#define     DclMBRPartitionDisplayTable  DCLFUNC(DclMBRPartitionDisplayTable)
#define     DclMBRPartitionLoad          DCLFUNC(DclMBRPartitionLoad)
#define     DclMBRPartitionLoadAll       DCLFUNC(DclMBRPartitionLoadAll)
#define     DclMBRPartitionStore         DCLFUNC(DclMBRPartitionStore)

void        DclMBRPartitionDisplay(const DCLDISKPARTITION *pPart, D_BOOL fVerbose);
void        DclMBRPartitionDisplayTable(const D_BUFFER *pMBR, D_BOOL fVerbose);
void        DclMBRPartitionLoad(         DCLDISKPARTITION *pPart, const D_BUFFER *pData);
DCLSTATUS   DclMBRPartitionLoadAll(      DCLDISKPARTITION *paPart, const D_BUFFER *pMBR, unsigned nCount);
void        DclMBRPartitionStore(D_BUFFER *pData, const DCLDISKPARTITION *pPart);


#endif /* DLMBR_H_INCLUDED */


