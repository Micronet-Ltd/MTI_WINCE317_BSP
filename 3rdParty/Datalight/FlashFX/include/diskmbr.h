/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2007 Datalight, Inc.
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

    This module contains MBR and disk specific information (not FAT specific).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: diskmbr.h $
    Revision 1.3  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/10/16 20:32:10Z  Garyp
    Modified so MBR support can be stripped out of the product.
    Revision 1.1  2004/12/31 02:00:10Z  Pauli
    Initial revision
    Revision 1.2  2004/12/30 23:00:09Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2003/04/13 02:57:52Z  garyp
    Initial revision
---------------------------------------------------------------------------*/
#ifndef DISKMBR_H_INCLUDED
#define DISKMBR_H_INCLUDED

#if !FFXCONF_MBRSUPPORT
#error "diskmbr.h is being included when FFXCONF_MBRSUPPORT is FALSE"
#endif

/*------------------------------------------------------------------
                       Partition Table Stuff
------------------------------------------------------------------*/

/*  The following are byte offsets for each of the partition table
    entries.  These are useful in systems that cannot access word
    on odd boundaries.
*/
#define PTE_OFFSET_BOOTABLE      (0)
#define PTE_OFFSET_BEGINHEAD     (1)
#define PTE_OFFSET_BEGINSECTOR   (2)
#define PTE_OFFSET_BEGINCYLINDER (3)
#define PTE_OFFSET_FILESYSID     (4)
#define PTE_OFFSET_ENDHEAD       (5)
#define PTE_OFFSET_ENDSECTOR     (6)
#define PTE_OFFSET_ENDCYLINDER   (7)
#define PTE_OFFSET_STARTSECTOR   (8)
#define PTE_OFFSET_SECTORCOUNT  (12)

#define PT_ISBT_OFF (MBR_OFFSET_PT + PTE_OFFSET_BOOTABLE     )
#define PT_STHD_OFF (MBR_OFFSET_PT + PTE_OFFSET_BEGINHEAD    )
#define PT_STSC_OFF (MBR_OFFSET_PT + PTE_OFFSET_BEGINSECTOR  )
#define PT_STTK_OFF (MBR_OFFSET_PT + PTE_OFFSET_BEGINCYLINDER)
#define PT_SYID_OFF (MBR_OFFSET_PT + PTE_OFFSET_FILESYSID    )
#define PT_EDHD_OFF (MBR_OFFSET_PT + PTE_OFFSET_ENDHEAD      )
#define PT_EDSC_OFF (MBR_OFFSET_PT + PTE_OFFSET_ENDSECTOR    )
#define PT_EDTK_OFF (MBR_OFFSET_PT + PTE_OFFSET_ENDCYLINDER  )
#define PT_STRT_OFF (MBR_OFFSET_PT + PTE_OFFSET_STARTSECTOR  )
#define PT_SIZE_OFF (MBR_OFFSET_PT + PTE_OFFSET_SECTORCOUNT  )


#endif /* DISKMBR_H_INCLUDED */
