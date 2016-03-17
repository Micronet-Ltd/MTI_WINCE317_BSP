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

    This header contains the prototypes, structures, and types necessary
    to use the FAT Monitor related API functions in FlashFX.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fatmon.h $
    Revision 1.8  2009/08/04 02:45:04Z  garyp
    Merged from the v4.0 branch.  Updated to use some reorganized FAT 
    functionality.
    Revision 1.7  2008/03/23 02:39:33Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.6  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2006/10/16 19:49:00Z  Garyp
    Minor type changes.
    Revision 1.4  2006/10/13 01:52:27Z  Garyp
    Updated to use some renamed symbols.
    Revision 1.3  2006/02/20 02:42:52Z  Garyp
    Minor type changes.
    Revision 1.2  2006/02/06 19:37:09Z  Garyp
    Updated to use the FFXDEVHANDLE and FFXDISKHANDLE model.
    Revision 1.1  2005/10/02 01:58:32Z  Pauli
    Initial revision
    Revision 1.3  2004/12/30 23:08:33Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2004/08/10 06:56:15Z  garyp
    Added FfxFatMonGetBPB().
    Revision 1.1  2004/08/09 23:32:56Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FATMON_H_INCLUDED
#define FATMON_H_INCLUDED

#if FFXCONF_ALLOCATORSUPPORT
#if FFXCONF_VBFSUPPORT
#if FFXCONF_FATMONITORSUPPORT

#include <dlfatapi.h>

FFXSTATUS   FfxFatMonInit(  FFXDISKHANDLE hDisk);
FFXSTATUS   FfxFatMonDeinit(FFXDISKHANDLE hDisk);
D_BOOL      FfxFatMonWrite( FFXDISKHANDLE hDisk, D_UINT32 ulStartPage, D_UINT32 ulPageCount, const D_BUFFER *pBuffer);
FFXSTATUS   FfxFatMonGetBPB(FFXDISKHANDLE hDisk, D_UINT16 uPartNum, DCLFATBPB *pBPB);

#endif  /* FFXCONF_FATMONITORSUPPORT */
#endif  /* FFXCONF_VBFSUPPORT */
#endif  /* FFXCONF_ALLOCATORSUPPORT */

#endif  /* FATMON_H_INCLUDED */



