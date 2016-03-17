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

    This module provide the prototypes and definitions necessary to use the
    OS Layer API.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: oeslapi.h $
    Revision 1.17  2009/10/06 18:56:18Z  garyp
    Removed ECC related stuff.
    Revision 1.16  2009/08/04 03:14:55Z  garyp
    Merged from the v4.0 branch.  Added support for the power suspend/resume API.
    Revision 1.15  2008/06/02 17:47:32Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.14  2008/05/23 02:15:51Z  garyp
    Merged from the WinMobile branch.
    Revision 1.13.1.2  2008/05/23 02:15:51Z  garyp
    Added a prototype.
    Revision 1.13  2008/03/20 19:31:14Z  Garyp
    Prototype updates.
    Revision 1.12  2008/01/31 23:48:49Z  Garyp
    Removed obsolete prototypes.
    Revision 1.11  2007/11/03 23:49:39Z  Garyp
    Updated to use the standard module header.
    Revision 1.10  2007/03/01 22:01:51Z  timothyj
    Changed interface to take a block index in lieu of a linear byte offset.
    Revision 1.9  2006/08/29 00:07:50Z  pauli
    Removed prototypes of deprecated WindowMap/WindowSize functions.
    Revision 1.8  2006/08/21 22:37:50Z  Garyp
    Added a prototype for FfxHookMapWindow().
    Revision 1.7  2006/02/17 22:16:05Z  Garyp
    Modified the ECC hook functions to take an hDev parameter.
    Revision 1.6  2006/02/12 18:01:38Z  Garyp
    Eliminated invalid stuff.
    Revision 1.5  2006/02/12 01:15:07Z  Garyp
    Prototype updates.
    Revision 1.4  2006/02/10 10:44:06Z  Garyp
    Updated to use the FFXDEVHANDLE and FFXDISKHANDLE model.
    Revision 1.3  2006/01/25 19:55:08Z  Garyp
    Added Sibley support.
    Revision 1.2  2005/12/30 17:52:22Z  Garyp
    Removed thread related stuff (now in DCL).
    Revision 1.1  2005/12/02 01:46:06Z  Pauli
    Initial revision
    Revision 1.3  2005/12/02 01:46:05Z  Garyp
    Prototype updates.
    Revision 1.2  2005/10/26 07:43:10Z  Garyp
    Eliminated obsolete stuff now handled elsewhere.
    Revision 1.1  2005/10/06 06:51:30Z  Garyp
    Initial revision
    Revision 1.3  2005/09/20 19:01:08Z  pauli
    Removed prototype for _syssleep().
    Revision 1.2  2005/09/17 23:51:23Z  garyp
    Modified so that _sysdelay() and related code is only built if
    FFXCONF_NORSUPPORT is TRUE.
    Revision 1.1  2005/07/06 05:53:04Z  pauli
    Initial revision
    Revision 1.27  2005/06/12 04:40:30Z  PaulI
    Removed obsolete _sysdisplaytext prototype.  This has been replaced in DCL.
    Revision 1.26  2005/05/11 18:54:03Z  garyp
    Cleaned up the prototypes.
    Revision 1.25  2004/12/30 21:38:42Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.24  2004/11/20 04:57:19Z  GaryP
    Added the PFFXDRIVERINFO opaque type.
    Revision 1.23  2004/09/23 22:34:27Z  billr
    Old FIMs are built whether or not FFXCONF_ENABLEOLDSTYLEFIMS is true, so
    _syssettimeout() and _systimeout() need to be declared to avoid warnings.
    Revision 1.22  2004/09/22 23:29:34Z  GaryP
    Made the _systimeout() functions conditional on FFXCONF_ENABLEOLDSTYLEFIMS.
    Revision 1.21  2004/08/30 21:43:38Z  GaryP
    Moved PFFXMUTEX and PFFXSEMAPHORE type declarations to fxtypes.h.
    Revision 1.20  2004/08/29 20:50:39Z  GaryP
    Moved memory management info to fxheap.h.
    Revision 1.19  2004/08/23 16:41:31Z  GaryP
    Added several new thread manipulation functions.
    Revision 1.18  2004/07/17 04:56:57Z  GaryP
    Added prototypes for FfxHookDiskCreate/Destroy().
    Revision 1.17  2004/07/08 20:44:13Z  garys
    moved FFXOPTION enumeration from oeslapi.h to fxoption.h
    Revision 1.16  2004/07/03 02:03:13  GaryP
    Added more FFXOPT_... values.
    Revision 1.15  2004/06/28 19:24:45Z  BillR
    Fix build failure on WinCE.
    Revision 1.14  2004/06/25 20:12:11Z  BillR
    New project hooks for initialization and configuration.
    Revision 1.13  2004/04/14 01:29:22Z  garyp
    Added the PDCLTHREAD type.  Added a thread name parameter to
    DclOsThreadCreate().
    Revision 1.12  2004/02/22 21:28:24Z  garys
    Merge from FlashFXMT
    Revision 1.8.1.10  2004/02/22 21:28:24  garyp
    Removed prototypes for the old-style HW Services routines, and added
    prototypes for more Project Hooks functions.
    Revision 1.8.1.9  2004/01/25 06:45:34Z  garyp
    Updated to use the new FFXCONF_... style configuration settings.  Added
    support for more memory management functions.
    Revision 1.8.1.8  2003/12/17 17:55:38Z  garyp
    Added some new prototypes for Project Hooks functions.
    Revision 1.8.1.7  2003/12/15 23:13:00Z  billr
    Initial implementation of OESL support for threads.
    Revision 1.8.1.6  2003/12/05 06:38:22Z  garyp
    Default to no FatMon if the setting is not defined.
    Revision 1.8.1.5  2003/11/22 04:41:20Z  garyp
    Eliminated a duplicate structure definition.
    Revision 1.8.1.4  2003/11/19 16:40:12Z  billr
    Merged from trunk build 744.
    Revision 1.11  2003/11/08 00:28:18Z  billr
    Implement multiple region caches.
    Revision 1.10  2003/04/15 23:38:18Z  garyp
    Reverted to previous revision to fix checkin problem.
    Revision 1.8.1.3  2003/11/06 22:55:26Z  garyp
    Added the Project Hooks prototypes.
    Revision 1.8.1.2  2003/11/03 06:02:18Z  garyp
    Re-checked into variant sandbox.
    Revision 1.9  2003/11/03 06:02:18Z  garyp
    Updated to initialize default value for FFX_MAX_FML_DEVICES,
    FFX_MAX_DISKS, and FFX_ALLOW_SINGLETHREADED_FIMS.
    Revision 1.8  2003/04/15 23:38:18Z  garyp
    Added FFXMUTEX type.  Changed _sysmalloc() to take a size_t.
    Revision 1.7  2003/04/15 17:49:12Z  garyp
    Added a number of new prototypes.
    Revision 1.6  2003/03/27 17:49:14Z  garyp
    Added a number of new prototypes.
    Revision 1.5  2002/12/02 22:16:52Z  garyp
    Eliminated _sysgetc() and _sysputc() prototypes.
    Revision 1.4  2002/11/13 09:20:56Z  garyp
    Updated prototypes for the new sysinterrupts... functions.
    Revision 1.3  2002/11/11 20:50:30Z  garyp
    Changed the prototype for FfxProjMain()
    Revision 1.2  2002/11/07 11:07:44Z  garyp
    Added prototypes for Unicode support.
---------------------------------------------------------------------------*/

#ifndef OESLAPI_H_INCLUDED
#define OESLAPI_H_INCLUDED


/*-------------------------------------------------------------------
    Prototypes for OS Services functions
-------------------------------------------------------------------*/
/* #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT

    The _sysdelay function is only use by NOR FIMs, however because the NOR
    FIMs currently build even when FFXCONF_NORSUPPORT is FALSE, we just keep
    these prototypes in place so compile errors are avoided.
*/
    void        _sysdelay(D_UINT32 ulMicroSeconds);
/* #endif */
D_UINT32        _sysinterruptdisable(void);
void            _sysinterruptrestore(D_UINT32 ulOldState);


/*-------------------------------------------------------------------
    Prototypes for new-style Project Hooks functions.

    (The NAND Control Project Hooks functions are
    dealt with in nandctl.h.)
-------------------------------------------------------------------*/

D_BOOL          FfxProjMain(void);
D_BOOL          FfxHookDriverCreate(    FFXDRIVERINFO *pFDI);
void            FfxHookDriverDestroy(   FFXDRIVERINFO *pFDI);
FFXDISKHOOK    *FfxHookDiskCreate(      const FFXDRIVERINFO *pDI, const FFXDISKINITDATA *pDiskData, FFXDISKCONFIG *pConf);
void            FfxHookDiskDestroy(     const FFXDRIVERINFO *pDI, FFXDISKHOOK *pHook);
FFXDEVHOOK     *FfxHookDeviceCreate(    const FFXDRIVERINFO *pDI, const FFXDEVINITDATA *pDevData, FFXDEVCONFIG *pConf);
void            FfxHookDeviceDestroy(   const FFXDRIVERINFO *pDI, FFXDEVHOOK *pHook);
FFXSTATUS       FfxHookAccessBegin(     FFXDEVHANDLE hDev, D_UINT16 uType);
FFXSTATUS       FfxHookAccessEnd(       FFXDEVHANDLE hDev, D_UINT16 uType);
D_UINT32        FfxHookMapWindow(       FFXDEVHANDLE hDev, D_UINT32 ulOffset, D_UINT32 ulMaxLen, volatile void **ppMedia);
D_UINT32        FfxHookBbmFormatBlock(  FFXDEVHANDLE hDev, D_UINT32 ulBlock, D_UINT32 ulBlockStatus);
D_BOOL          FfxHookOptionGet(       FFXOPTION opt, void *handle, void *pBuffer, D_UINT32 ulBuffLen);
D_BOOL          FfxOsOptionGet(         FFXOPTION opt, void *handle, void *pBuffer, D_UINT32 ulBuffLen);
unsigned        FfxOsDeviceNameToDiskNumber(const char *pszDevName);


/*-------------------------------------------------------------------
    The following settings define the access type values passed
    to FfxHookAccessBegin/End()
-------------------------------------------------------------------*/
#define ACCESS_READS       0
#define ACCESS_WRITES      1
#define ACCESS_ERASES      2
#define ACCESS_MOUNTS      3
#define ACCESS_UNMOUNTS    4
#define ACCESS_SUSPEND     5
#define ACCESS_RESUME      6



#endif  /* OESLAPI_H_INCLUDED */

