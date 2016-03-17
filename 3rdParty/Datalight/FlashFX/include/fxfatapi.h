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
    to use the FAT related API functions in FlashFX.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxfatapi.h $
    Revision 1.7  2009/08/04 02:55:15Z  garyp
    Merged from the v4.0 branch.  Moved BPB and boot record creation 
    functions into DCL.
    Revision 1.6  2008/12/09 21:33:57Z  keithg
    No longer pass a serial number to VbfFormat() - it is handled internally.
    Revision 1.5  2008/03/23 02:39:35Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.4  2007/12/01 03:32:54Z  Garyp
    Modified FfxFatFormat() to return a meaningful status code rather than
    D_BOOL, and adjusted the calling code to make use of it as appropriate.
    Revision 1.3  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/09/28 19:56:55Z  jeremys
    The code in this header was conditioned by the wrong define.
    Revision 1.1  2007/09/26 23:52:22Z  jeremys
    Initial revision
    Revision 1.6  2006/10/16 20:32:14Z  Garyp
    Removed an obsolete structure.
    Revision 1.5  2006/02/21 03:29:03Z  Garyp
    Minor type changes.
    Revision 1.4  2006/02/09 23:33:01Z  Garyp
    Updated to use the FFXDEVHANDLE and FFXDISKHANDLE model.
    Revision 1.3  2006/02/03 16:19:16Z  johnb
    Added FATMON FAT32 Support
    Revision 1.2  2006/01/12 01:25:20Z  Garyp
    Minor prototype updates.
    Revision 1.1  2005/10/03 19:28:02Z  Pauli
    Initial revision
    Revision 1.10  2004/12/30 23:08:34Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.9  2004/08/09 20:35:56Z  GaryP
    Miscellaneous changes to support isoating the file system specific support
    from generic FlashFX functionality.
    Revision 1.8  2004/05/06 00:42:06Z  garyp
    Updated to eliminate passing sector buffers around.
    Revision 1.7  2004/01/13 04:15:50Z  garys
    Merge from FlashFXMT
    Revision 1.4.1.3  2004/01/13 04:15:50  garyp
    Eliminated some obsolete prototypes.
    Revision 1.4.1.2  2003/11/01 02:22:28Z  garyp
    Re-checked into variant sandbox.
    Revision 1.5  2003/11/01 02:22:28Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.4  2003/04/16 06:18:22Z  garyp
    Prototype changes.
    Revision 1.3  2003/04/10 19:39:16Z  garyp
    Added the FFXFATFMTPARAMS structure.
    Revision 1.2  2003/03/26 21:15:32Z  garyp
    Renamed helper functions to use the "Ffx" prefix.
---------------------------------------------------------------------------*/


#ifndef FFXFAT_H_INCLUDED
#define FFXFAT_H_INCLUDED

#if !FFXCONF_FATSUPPORT
#error "fxfatapi.h is being included when FFXCONF_FATSUPPORT is FALSE"
#endif

/*-------------------------------------------------------------------
-------------                                             -----------
-------------  Structures and Types for Public Functions  -----------
-------------                                             -----------
-------------------------------------------------------------------*/

#include <dlfatapi.h>
#include <diskapi.h>


#if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
FFXSTATUS   FfxFatFormat(VBFHANDLE hVBF, DCLFATPARAMS *pFFP);
#endif

D_BYTE      FfxFatGetDiskParamsFromBootRecord(const D_BUFFER *pBuffer, FFXPHYSICALDISKINFO *pPDI);

#endif /* FFXFAT_H_INCLUDED */


