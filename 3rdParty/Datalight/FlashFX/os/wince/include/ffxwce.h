/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This header contains structures and symbols used by the FlashFX driver
    for Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ffxwce.h $
    Revision 1.15  2010/09/22 20:43:46Z  garyp
    Added ulInitCount to the DRIVERDATA structure, rather than having
    it as a discrete global.
    Revision 1.14  2010/09/19 03:21:56Z  garyp
    Added an "Autoload Workaround" for WinMobile, which resolves an
    apparent bug in WinMobile, whereby PowerDown requests for 
    autoloaded drivers are not handled properly.
    Revision 1.13  2010/01/23 22:58:31Z  garyp
    Added a nDefaultFS field.
    Revision 1.12  2009/07/22 01:14:27Z  garyp
    Merged from the v4.0 branch.  Added the DiskFlags field.  Updated to 
    support the concept of a CE store which is split across two FlashFX Disks.
    Revision 1.11  2009/02/09 03:54:48Z  garyp
    Merged from the v4.0 branch.  No longer use FFX_IOCTL_CODE, but 
    rather use the general "BlockDev" IOCTL code defined in DCL, which
    allows for more flexible cross-product, cross-code-base communication.
    Revision 1.10  2008/05/23 02:15:51Z  garyp
    Merged from the WinMobile branch.
    Revision 1.9.1.2  2008/05/23 02:15:51Z  garyp
    Added the index field.
    Revision 1.9  2008/03/26 02:34:54Z  Garyp
    Minor datatype updates.
    Revision 1.8  2007/12/26 02:51:34Z  Garyp
    Removed some obsolete settings.
    Revision 1.7  2007/11/03 23:50:11Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2007/08/01 00:10:41Z  timothyj
    Changed offset and length to be in KB.
    Revision 1.5  2007/06/20 21:30:35Z  Garyp
    Updated to use the generic external requestor logic now implemented in DCL.
    Revision 1.4  2007/03/02 00:26:29Z  Garyp
    Removed CE 3 support.
    Revision 1.3  2006/03/20 04:39:11Z  Garyp
    Per Bill, fixed to store a vpp count so we work properly when suspending
    erases.
    Revision 1.2  2006/02/07 04:05:45Z  Garyp
    Started the process of factoring things into Devices and Disks.
    Work-in-progress, not completed yet.
    Revision 1.1  2005/10/02 01:39:54Z  Pauli
    Initial revision
    Revision 1.17  2005/04/03 01:29:35Z  GaryP
    Minor documentation and code formatting cleanup.
    Revision 1.16  2004/12/30 23:08:32Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.15  2004/08/09 23:04:16Z  GaryP
    Minor tweaks to support refactoring the file system specific code.
    Revision 1.14  2004/07/22 04:02:04Z  GaryP
    Eliminated the uFormatBBMState support which is implemented elsewhere.
    Revision 1.13  2004/07/19 20:27:13Z  GaryP
    Added an FFXDISKDATA structure.
    Revision 1.12  2004/06/18 23:37:26Z  BillR
    New interface to multi-threaded FIMs.  New project hooks for initialization
    and configuration.
    Revision 1.11  2004/05/26 16:12:35Z  garyp
    Minor parameter changes to the Driver Framework.  Eliminated the use of
    a common sector buffer.
    Revision 1.10  2004/01/05 18:11:22Z  garys
    Merge from FlashFXMT
    Revision 1.9.1.4  2004/01/05 18:11:22  garyp
    Added DRIVERPREFIX.  Moved the DEVEXTRA structure into this header.
    Revision 1.9.1.3  2003/12/05 06:53:48Z  garyp
    Renamed FFX_DISK to DRIVERDATA and moved the device specific information to
    the DEVEXTRA structure.
    Revision 1.9.1.2  2003/11/11 21:57:56Z  garyp
    Changed the IOCTL code name.
    Revision 1.9  2003/04/15 17:49:14Z  garyp
    Updated to use the new Driver Framework.
    Revision 1.8  2003/03/27 21:47:54Z  garyp
    Added the phys and FAT disk info structures to FFXDISK.
    Revision 1.7  2003/02/13 23:17:14Z  garyp
    Added the iWelcome field to FFXDISK, and defined the WELCOME_... states.
    Revision 1.6  2003/02/11 20:05:38Z  garyp
    Minor formatting changes.
    Revision 1.5  2002/12/20 21:19:42Z  garyp
    Eliminated the use of the FFX_DRK and ISDRK settings.
    Revision 1.4  2002/11/13 07:03:38Z  garyp
    Fixed a typo in the previous rev.
    Revision 1.3  2002/11/11 19:53:26Z  garyp
    Changed to define ISDRK and ISDRKEVAL.
    Revision 1.2  2002/11/07 11:22:32Z  garyp
    Eliminated DBGPRINTF (now defined in FXASSERT.H).
    05/15/02 gp  Moved this file back into the WINCE tree from the INCLUDE
                 directory.  No other changes.
    05/10/02 gp  Added the uFormat field to FFX_DISK.
    05/01/02 gp  Eliminated the dwTotalSectors field from FFX_DISK.
    02/07/02 DE  Updated copyright notice for 2002.
    02/06/02 gp  Added several fields to the FFX_DISK structure.
    08/08/01 gp  Removed the DEV_INFO structure.
    07/31/01 gp  Added the uOpenCount field to the FFX_DISK structure.
    07/26/01 gp  Added the DEV_INFO structure to support suspend/resume.
    06/19/01 gp  Added the DBGPRINTF() macros.
    09/14/99 PKG Added active key copy feild in disk structure.
    10/26/98 PKG Changed FFX_EXTENSION to more acurrate FFX_DISK
    08/20/98 PKG Corrected alignment of internal structures
    02/11/98 PKG Extensive modifications, removal of un-used code.
    09/10/97 PJG Original.
---------------------------------------------------------------------------*/

#ifndef FFXWCE_H_INCLUDED
#define FFXWCE_H_INCLUDED

#include "oecommon.h"

/*  The driver prefix is used to refer to FlashFX storage devices,
    typically in the form "DSK1:".  If this value is changed from
    "DSK", the FFXDRV.C and FLASHFX.DEF files must be modified so
    the entry points for the standard functions use the new name.
*/
#define DRIVERPREFIX "DSK"


/*  The FAT format must always have 240 root entries in order
    to provide compatibility with floppy disks.
*/
#define  ROOT_DIR_SIZE 0xF0


/*  The following states are used by the FlashFX driver to keep track
    of where the access to the media is.  We do not want to allow the
    disk to be opened more than once, it should not be initilized more
    than once, etc...
*/
#define STATE_INITIALIZED  0x1
#define STATE_OPENED       0x2
#define STATE_DEAD         0x4
#define STATE_REMOVED      0x8


/*  Maximum size of the registry key
*/
#define MAX_KEY_LEN 256

/*  This structure holds information needed for access to the flash disk.
*/
typedef struct
{
    D_UINT32        ulInitCount;
    D_BOOL          fUseFatMon;
    D_BOOL          fDelSectorsBroken;
    OSVERSIONINFO   OSVI;
    FFXDRIVERINFO  *pDI;
} DRIVERDATA, * PDRIVERDATA;

typedef struct tagFFXDEVHOOK
{
    unsigned        nDevNum;
    D_INT16         iVppCount;      /* Number of threads needing Vpp to be on */
    BYTE           *pbFlashMedia;
} FFXDEVHOOK;

typedef struct tagFFXDISKHOOK
{
    DISK_INFO       CeDiskInfo;     /* Disk info structure used by WinCE (FATFS) */
    TCHAR           tzFolderName[MAX_KEY_LEN];
    TCHAR           tzProfileName[MAX_KEY_LEN];
    DWORD           dwFlashOffsetKB;
    DWORD           dwFlashLengthKB;
    DWORD           dwFlags;
    DWORD           dwDeviceNum;
    DWORD           dwDiskNum;
    DWORD           dwDiskFlags;
    DWORD           dwIndex;
    D_UINT32        ulOpenCount;
    D_UINT32        ulPendingDiscardStart;
    D_UINT32        ulPendingDiscardCount;
    D_UINT32        ulNextDiskSector;
    FFXDISKHANDLE   hNextDisk;
    unsigned        nDiskNum;
    unsigned        nFormatState;
    unsigned        nDefaultFS;
} FFXDISKHOOK;


/*  The parameter defining a drive is the registry key passed to the
    driver initialization entry point.
*/
typedef struct tagFFXDISKINITDATA
{
    LPCTSTR         lptzActiveKey;
} FFXDISKINITDATA;

struct tagFFXDEVINITDATA
{
    unsigned        nDevNum;
};


extern DRIVERDATA dd;


#endif /* FFXWCE_H_INCLUDED */


/*---------------------------------------------------------
    File pruned by DL-Prune v1.04

    Pruned/grafted 2/0 instances containing 6 lines.
    Modified 0 lines.
---------------------------------------------------------*/
