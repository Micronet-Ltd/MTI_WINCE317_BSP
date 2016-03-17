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

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: oecommon.h $
    Revision 1.8  2009/07/22 01:16:33Z  garyp
    Merged from the v4.0 branch.  Added the COMPBLOCKINFO structure,
    as well as function prototypes to support WinMobile.
    Revision 1.7  2008/05/27 21:39:36Z  garyp
    Merged from the WinMobile branch.
    Revision 1.6.1.2  2008/05/27 21:39:36Z  garyp
    Added a missing const declaration.
    Revision 1.6  2008/03/27 17:05:27Z  Garyp
    Hacked the prototype for FfxCe5DispatchFmlIoctl() -- needs to be refactored!
    Revision 1.5  2007/12/26 01:46:54Z  Garyp
    Added a prototype.
    Revision 1.4  2007/12/01 01:49:49Z  Garyp
    Removed an obsolete prototype.
    Revision 1.3  2007/11/03 23:50:11Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/02/07 03:16:01Z  Garyp
    Prototype update.
    Revision 1.1  2005/10/03 12:49:10Z  Pauli
    Initial revision
    Revision 1.6  2005/06/12 04:40:30Z  PaulI
    Added new prototype for extapi output function.
    Revision 1.5  2005/03/29 03:36:45Z  GaryP
    Minor code formatting cleanup.
    Revision 1.4  2004/12/30 23:22:07Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.3  2004/08/23 19:26:16Z  GaryP
    Eliminated obsolete prototypes, and added new ones.
    Revision 1.2  2003/12/30 04:52:10Z  garys
    Merge from FlashFXMT
    Revision 1.1.1.3  2003/12/30 04:52:10  garyp
    New prototypes.
    Revision 1.1.1.2  2003/12/02 20:54:40Z  garyp
    Added prototypes.
    Revision 1.1  2003/02/17 01:48:50Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef OECOMMON_H_INCLUDED
#define OECOMMON_H_INCLUDED

#define BAD_FLASH_ADDRESS 0xFFFFFFFFUL
#define BAD_FLASH_LENGTH  0xFFFFFFFFUL


D_BOOL      FfxParseDriveNumber(const char *pszString, D_UINT16 *puDriveNumber, D_UINT16 *puStringLen);
const char *FfxGetDriveFormatString(void);
D_BOOL      FfxCeGetRegistrySettings(LPCTSTR lptzActiveKey, FFXDISKHOOK *pHook);
BYTE *      FfxCeMapFlash(D_BUFFER *pFlashAddress, DWORD dwFlashLength);
void        FfxCeUnmapFlash(BYTE * pbFlashMedia);


#if _WIN32_WCE < 600
/*  ToDo:  This needs to be refactored.
*/
#ifdef FLASHFX_H_INCLUDED
#include <fxdriver.h>
FFXSTATUS FfxCe5DispatchFmlIoctl(FFXDISKHANDLE hDisk, FFXIOREQUEST *pReqHdr, DWORD dwLen);
#endif
#endif

#endif  /* OECOMMON_H_INCLUDED */



/*---------------------------------------------------------
    File pruned by DL-Prune v1.04

    Pruned/grafted 1/0 instances containing 11 lines.
    Modified 0 lines.
---------------------------------------------------------*/
