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

    This header contains information pertaining to disk partition types.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlpartid.h $
    Revision 1.3  2009/02/08 01:19:47Z  garyp
    Merged from the v4.0 branch.  Added partition types for FAT LBA partitions.
    Revision 1.2  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/09/27 01:26:40Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLPARTID_H_INCLUDED
#define DLPARTID_H_INCLUDED


#define BOOTREC_SIGNATURE_OFFSET    (0x01FE) /* offest of the signature word */


/*  File System ID values
*/
#define FSID_NONE        (0x00)
#define FSID_FAT12       (0x01) /* FAT 12 partition                             */
#define FSID_FAT16       (0x04) /* FAT 16 partition                             */
#define FSID_EXTENDED    (0x05) /* Extended partition                           */
#define FSID_BIGFAT      (0x06) /* FAT partition with more than 64K sectors     */
#define FSID_RELIANCE    (0x07) /* Datalight Reliance partition (and NTFS/HPFS) */
#define FSID_FAT32       (0x0B) /* FAT 32 partition                             */
#define FSID_FAT32LBA    (0x0C) /* FAT 32 partition using LBA                   */
#define FSID_BIGFATLBA   (0x0E) /* BIFFAT partition using LBA                   */
#define FSID_EXTENDEDLBA (0x0F) /* EXTENDED partition using LBA                 */
#define FSID_QNX40_1     (0x4D) /* QNX v4.0 partition 1                         */
#define FSID_QNX40_2     (0x4E) /* QNX v4.0 partition 2                         */
#define FSID_QNX40_3     (0x4F) /* QNX v4.0 partition 3                         */
#define FSID_UNIX        (0x63) /* Unix partition                               */


#endif  /* DLPARTID_H_INCLUDED */

