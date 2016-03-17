/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2008 Datalight, Inc.
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

    This header contains VBF symbols and types which relate to units.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: vbfunit.h $
    Revision 1.4  2008/01/30 23:50:01Z  Garyp
    Added VBF_EUH_BUFFER_SIZE.
    Revision 1.3  2007/11/03 23:49:29Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/04/19 23:55:51Z  timothyj
    Changed clientAddress (byte offset) to regionNumber (index of
    region) in the UnitHeader.
    Revision 1.1  2006/04/29 18:49:20Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef VBFUNIT_H_INCLUDED
#define VBFUNIT_H_INCLUDED

#define VBF_SIGNATURE_SIZE 0x10

/*  This is the unit header structure that will exist at the
    start of each unit.  It describes information about the
    entire partition and the specific unit it resides in.
*/
typedef struct
{
    D_UINT32    regionNumber;       /* region to which this unit belongs */
    D_INT32     eraseCount;         /* this units erase count */
    D_UINT32    serialNumber;       /* the partition serial number */
    D_UINT32    ulSequenceNumber;   /* partition sequence number of this unit */
    D_UINT32    lnuTotal;           /* number of units in this partition */
    D_UINT32    lnuTag;             /* unit number of queued unit */
    D_UINT16    numSpareUnits;      /* number of spare units available */
    D_UINT16    uAllocBlockSize;    /* allocation block size (must be fixed) */
    D_UINT16    lnuPerRegion;       /* units in a region */
    D_UINT16    uReserved;          /* reserved, was old partition start LNU */
    D_UINT16    uUnitTotalBlocks;   /* size in allocation blocks of an erase unit */
    D_UINT16    unitClientBlocks;   /* client space in unit (# of blocks */
    D_UINT16    unitDataBlocks;     /* # of blocks available for allocation */
} UnitHeader, EUH;


#define VBF_DISK_EUH_SIZE   (38)
#define VBF_EUH_BUFFER_SIZE (64)

#endif /* VBFUNIT_H_INCLUDED */

