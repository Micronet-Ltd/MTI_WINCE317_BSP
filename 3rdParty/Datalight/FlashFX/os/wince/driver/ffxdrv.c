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

    This module contains the FlashFX device driver for Windows CE.

    ToDo:
    - Write a unit test for the ScatterGatherWrite() code in the case
      that the SG buffers are not sized on multiples of 512.  The
      CETK in CE.net tests this functionality, but for reading only.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ffxdrv.c $
    Revision 1.33  2010/12/15 21:43:59Z  garyp
    Conditioned IOCTL_DISK_SET_SECURE_WIPE_FLAG and IOCTL_DISK_SECURE_WIPE
    on being version 502 or later.
    Revision 1.32  2010/12/12 02:25:09Z  garyp
    Cleaned up error handling in the IOCTL function.  Added support for
    IOCTL_DISK_SECURE_WIPE.
    Revision 1.31  2010/09/23 20:24:39Z  garyp
    Modified DSK_PowerDown() to detect if the driver is not initialized,
    and do nothing if so.
    Revision 1.30  2010/09/23 07:23:57Z  garyp
    Reverted a portion of rev 1.28 which was preventing all DCL_IOCTL_BLOCKDEV
    calls from working.  Updated the error handling logic in the startup and
    shutdown processes.  Added IOCTL_POWER_* handlers.
    Revision 1.29  2010/09/18 02:54:56Z  garyp
    Added an "Autoload Workaround" for WinMobile, which resolves an apparent 
    bug in WinMobile, whereby PowerDown requests for autoloaded drivers are 
    not handled properly.
    Revision 1.28  2010/09/09 15:37:27Z  johnb
    Added various checks to validate parameters in order to get
    the CETK to fully pass.
    Revision 1.27  2010/04/19 19:18:07Z  garyp
    Updated so the QUEUE_PENDING_DISCARDS feature is configurable at
    compile time.  Added profiler instrumentation.
    Revision 1.26  2010/01/21 19:31:10Z  garyp
    Updated to support flush.
    Revision 1.25  2010/01/07 23:51:43Z  garyp
    Modified to use the updated DriverAutoTest interface.
    Revision 1.24  2009/12/13 01:38:58Z  garyp
    Updated so that SetLastError() is only called in the event that there
    actually IS and error.  Essentially SetLastError(0) is never done.
    Revision 1.23  2009/08/04 17:15:43Z  garyp
    Merged from the v4.0 branch.  Modified the shutdown processes to take a
    mode parameter.  Conditioned some code on DCL_OSTARGET_WINMOBILE.  Handle
    the changed return value from FfxCeDriverInit().  Updated to support the
    concept of a CE store which is split across two FlashFX Disks.  Updated
    documentation.  Modified to set the STORAGE_DEVICE_FLAG_TRANSACTED for
    read-only (no allocator) Disks.  Updated so that unhandled IOCTL calls are
    passed to the FfxCeEmulatedFmdIoControl() function.  Added support for the
    power suspend/resume API.
    Revision 1.22  2009/03/09 19:53:18Z  thomd
    Cast comparison for compiler.
    Revision 1.21  2009/02/09 07:06:45Z  garyp
    Merged from the v4.0 branch.  No longer use FFX_IOCTL_CODE, but rather
    use the general "BlockDev" IOCTL code defined in DCL, which allows for
    more flexible cross-product, cross-code-base communication.  Conditioned
    some code on DCL_OSTARGET_WINMOBILE.  Updated documentation.  Renamed
    FlashFXDriverEntry() to DllMain().  Eliminated the obsolete DetectDSKDisk()
    function.  Removed unnecessary headers.  Added logic to watch for NULL
    pointers in the IOCTL interface.
    Revision 1.20  2008/05/23 06:55:51Z  garyp
    Merged from the WinMobile branch.
    Revision 1.19.1.2  2008/05/23 06:55:51Z  garyp
    Added FfxOsDeviceNameToDiskNumber().
    Revision 1.19  2008/03/29 19:50:36Z  garyp
    Updated to build cleanly with CE 4.2.
    Revision 1.18  2008/03/25 07:23:56Z  Garyp
    Implemented IOCTL_DISK_GET_STORAGEID.  Removed DRK remnants.  Updated to
    work when allocator support is disabled.
    Revision 1.17  2008/01/31 03:52:06Z  Garyp
    Cleaned up some debug messages.
    Revision 1.16  2007/11/03 23:50:10Z  Garyp
    Updated to use the standard module header.
    Revision 1.15  2007/09/28 22:47:25Z  jeremys
    Renamed a header file.
    Revision 1.14  2007/06/26 23:00:19Z  timothyj
    Changed offset and length to be in KB.
    Revision 1.13  2007/04/02 16:18:26Z  keithg
    FSINTTEST has been removed from FFX, it is now part of DCL.
    Revision 1.12  2007/03/04 02:33:39Z  Garyp
    Removed CE 3 support.
    Revision 1.11  2007/01/23 23:31:17Z  Garyp
    Added FSIntTest support.
    Revision 1.10  2006/10/16 20:32:12Z  Garyp
    Minor header changes.
    Revision 1.9  2006/10/13 02:20:03Z  Garyp
    Updated so an MBR is always used.
    Revision 1.8  2006/07/01 18:57:47Z  Garyp
    Updated to use the new FfxDriverVbfCreate() function.
    Revision 1.7  2006/05/06 21:42:05Z  Garyp
    Updated debugging code.
    Revision 1.6  2006/03/24 06:14:14Z  Garyp
    Documentation updates.
    Revision 1.5  2006/02/21 02:02:43Z  Garyp
    Removed an obsolete header.
    Revision 1.4  2006/02/13 03:10:40Z  Garyp
    Updated to new external API requestor interface.
    Revision 1.3  2006/02/07 21:56:37Z  Garyp
    Updated to use the new DeviceNum/DiskNum concepts.
    Revision 1.2  2006/01/11 18:41:59Z  Garyp
    Documentation, debug code, and general cleanup -- no functional changes.
    Revision 1.1  2005/10/02 01:33:10Z  Pauli
    Initial revision
    Revision 1.42  2005/05/17 19:21:53Z  garyp
    Added IOCTL_DISK_FLUSH_CACHE support (CE 5.0 only).
    Revision 1.41  2005/04/12 08:06:42Z  Rickc
    added variable declaration
    Revision 1.40  2005/04/03 01:29:36  GaryP
    Minor init code logic cleanup.
    Revision 1.39  2004/12/30 17:33:25Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.38  2004/11/19 20:59:32Z  GaryP
    Updated to use refactored header files.
    Revision 1.37  2004/10/04 22:50:24Z  jaredw
    Fixed spelling error: FfxDriverUnitTest was FfxDriverUnitTests.
    Revision 1.36  2004/09/29 02:24:27Z  GaryP
    Modified to use the driver framework unit test function.
    Revision 1.35  2004/09/22 16:56:18Z  GaryP
    Changed the device sector length field to be a 32-bit value.
    Revision 1.34  2004/08/13 19:23:22Z  GaryP
    Modified to use the updated FfxDriverSectorRead/Write() functions.  Updated
    to use FfxDriverSectorDiscard() rather than discrete vbfdiscard() calls.
    Revision 1.33  2004/08/07 00:55:36Z  GaryP
    In-progress checkin.  Commented out some apparently unnecessary functions
    now that FATMON has been refactored.
    Revision 1.32  2004/07/29 20:23:20Z  jaredw
    Added IOCTL_PSL_NOTIFY to the handled IOCTLs list.  It currently
    just prints a debug message indicating we recieved this ioctl.
    Revision 1.31  2004/07/14 01:03:50Z  jaredw
    drive now loads even if flash isnt' formatted
    Revision 1.30  2004/07/02 23:51:01Z  GaryP
    Eliminated FfxDriverSetDeviceInfo() and FfxDriverSetDeviceFATInfo() usage.
    Revision 1.29  2004/06/04 18:52:48Z  jaredw
    added FMLHANDLE parameter to FfxPerformUnitTests to fix problem with needed
    struct not defined.  This allows running of unit tests from the driver
    Revision 1.28  2004/05/28 17:18:12Z  garyp
    Minor parameter changes to the Driver Framework.  Eliminated the use of
    a common sector buffer.
    Revision 1.27  2004/01/05 18:19:28Z  garys
    Merge from FlashFXMT
    Revision 1.26.1.6  2004/01/05 18:19:28  garyp
    Renamed DSKDiskEntry() to FlashFXDriverEntry().
    Revision 1.26.1.5  2003/12/17 00:13:34Z  garyp
    Eliminated the RAMDISK special case to set the array size.
    Revision 1.26.1.4  2003/12/05 17:47:46Z  garyp
    Major revamping to work in a multi-threaded environment.  Modified to
    work with multiple devices.  Split out large chunks of functionality
    into other modules.
    Revision 1.26.1.3  2003/11/23 21:09:16Z  garyp
    Changed the driver mutex name to meet our new length standards.
    Revision 1.26.1.2  2003/11/22 04:28:46Z  garyp
    Updated to work with FlashFX-MT.  Total revamping of the external API.
    Revision 1.26  2003/09/26 18:05:05Z  garys
    CE 4.1 is no longer documented as supporting IOCTL_DISK_DELETE_SECTORS
    Revision 1.25  2003/06/24 21:49:36  garys
    updated comment: the CETK benchmark was fixed in CE 4.2
    Revision 1.24  2003/06/05 18:00:26  garyp
    Renamed FFX_UNIT_TEST to FFXCONF_DRIVERAUTOTEST.  Fixed to use NUM_DRIVES
    rather than MAX_DRIVES.
    Revision 1.23  2003/05/29 17:48:06Z  garyp
    Updated to use VBF_MAX_TRANSFER_SIZE.
    Revision 1.22  2003/05/29 06:20:16Z  garyp
    Fixed to build right under CE3.  Fixed the write interruption IOCTL.
    Revision 1.21  2003/05/23 20:27:52Z  tonyq
    Update to only do vbftestwriteinterruptions during debug build
    Revision 1.20  2003/05/22 17:37:46Z  garyp
    Added external API support for vbftestwriteinterruptions() and
    vbftestregionmountperf().
    Revision 1.19  2003/05/08 18:08:26Z  garyp
    Removed the unit test drive size limitation hack implemented in the
    previous rev.  Turned of write interruption tests by default.
    Revision 1.18  2003/05/02 17:32:18Z  garyp
    Removed obsolete code.  Modified FfxPerformUnitTests to hack the total disk
    size in the mediainfo structure prior to starting the tests.  Updated to do
    the write interruption and extensive tests by default.
    Revision 1.17  2003/04/30 21:57:23Z  garys
    Added support for vbfgetpartitioninfo() in DoIOCtl_FxAPI()
    Revision 1.16  2003/04/29 22:30:50  garys
    changed two instances of RAMEmul to RAMDisk
    Revision 1.15  2003/04/24 23:29:15  billr
    External API changes: add vbfgetunitinfo() and vbfgetpartitioninfo(),
    remove MountEUH(), ReadEUH(), and ReadAllocationList().
    Revision 1.14  2003/04/21 11:05:36Z  garyp
    Updated to cache pending discards.
    Revision 1.13  2003/04/15 21:17:40Z  garyp
    Updated to include FXVER.H.
    Revision 1.12  2003/04/15 17:49:12Z  garyp
    Updated to use the new Driver Framework.  Added IOCTLs to support the tools.
    Revision 1.11  2003/03/26 21:16:02Z  garyp
    Updated to use new helper routines.
    Revision 1.10  2003/02/13 23:14:26Z  garyp
    Updated to retrieve the welcome state from the registry.  Added logic to
    support the new CE DRK which uses a disk size restriction.
    Revision 1.9  2001/07/01 18:54:00Z  garys
    Made IOCTL_DISK_DELETE_SECTORS conditional on CE.net
    Revision 1.8  2003/02/12 01:01:42Z  garyp
    Added support for IOCTL_DISK_DELETE_SECTORS.  Replaced MountBPB() with
    GetDiskParams(), which reads the MBR and properly locates the disk.  Added
    support for logical drives within an extended partition, which is the
    standard method used when CE formats a disk.  Eliminated use of the FSD
    registry setting.
    Revision 1.7  2003/01/24 22:15:16Z  garyp
    Major cleanup of the debugging messages.  Streamlined the code
    flow through the IOCTL routine.
    Revision 1.6  2002/12/20 21:19:42Z  garyp
    Eliminated the use of ISDRK.  Moved the eval DRK signon message
    into FfxProjMain().
    Revision 1.5  2002/11/13 09:28:42Z  garyp
    Commented out the call the FormatDisk() in DISK_IOCTL_FORMAT_MEDIA.
    Revision 1.4  2002/11/11 21:18:26Z  garyp
    Changed to use the new FfxProjMain() functionality.  Changed the FFX_DRK
    and FFX_DRKEVAL usage.  Changed #ifdef UNIT_TEST to #if FFX_UNIT_TEST.
    Revision 1.3  2002/11/07 11:22:32Z  garyp
    Modified to place the standard driver configuration options in OEMCONF.H.
    Changed all RETAILMSGs to FfxPrintfs.  Modified to use the new FFXPRINTF
    which uses an ANSI formatting string.
    Revision 1.2  2002/10/18 19:05:44Z  garyp
    Tweaked the version string to include the ALPHABETA value, if any.
    08/26/02 gp  Merged in DRKEVAL functionality from the DRK_CE project.
    08/26/02 gp  Updated the version display to use the new symbols.
    08/19/02 gp  Added SmartOemMount() and modified all oemmount() calls to
                 use this instead.  Commented out the code in DSK_PowerUp()
                 and DSK_PowerDown().  Minor debug code changes.  Commented
                 out the bogus code to allow the CE 4 Test Kit to pass.
    05/15/02 gp  Cleaned up various debug and retail messages for the DRK.
    05/10/02 gp  Eliminated the FORMAT_MEDIA setting and changed so we format
                 based on a registry setting.  Added a signon copyright
                 message.  Modified the behavior under CE.net so that we force
                 the hidden sectors value to be correct, but we report it as
                 zero to CE.net so that the Storage Manager applet works
                 correctly.
    05/01/02 gp  Fixed DEVICE_NAME to be a unicode string.  Fixed DetectDSKDisk
                 to use the wcs functions.  Fixed several IOCTLs to fall out
                 out the end of the function if successful, so the proper
                 debugging information is displayed.  Modified MountBPB() to
                 avoid using the hidden sectors value since it is not reliable
                 in Windows CE.  Fixed IOCTL_DISK_READ to avoid setting the
                 "bytes read" value if the pointer is NULL.  Fixed the unit
                 test code to fail properly if vbfformat() fails.
    02/18/02 gp  Minor documentation and comment fixes.
    02/18/02 gjs ifdef'd several IOCTL switches that aren't in CE 2.0.
    02/13/02 gp  Added support for IOCTL_DISK_FORMAT_MEDIA.  Added support for
                 scatter/gather buffers that are not sized on multiples of 512.
    02/07/02 DE  Updated copyright notice for 2002.
    02/06/02 gp  Major updates for CE.net compatibility.  Change the device
                 prefix from FFX to DSK.  Updated to avoid calling InitFSD()/
                 LoadFSD() under CE.net.  Modified to use StorageManager
                 profiles.  Added the FFX_USEMBR setting to allow changes to
                 the driver behavior.  Added support for the IOCTL_DISK_READ
                 IOCTL_DISK_WRITE, IOCTL_DISK_GETINFO, IOCTL_DISK_DEVICE_INFO,
                 and IOCTL_DISK_GETNAME ioctls.  Eliminated the use of the
                 FATFS_SCAN_DELAY feature under CE.net.  Abstracted the
                 disk parameter calculations into the routine InitDiskParams().
                 Updated to detect when the BPB is being rewritten, even if
                 it is at a new location due to changes in whether the MBR
                 is reserving just a sector (our default behavior) or whether
                 the disk is being reformatted to reserve the entire first
                 track for the MBR (as FATFS apparently does).
    08/08/01 gp  Finished removing the old CE 2.1 suspend/resume code
    08/07/01 gp  Removed the old CE 2.1 suspend/resume code added in the 7/26
                 revision.  Added CE 3.0 compatible suspend/resume code.
    07/31/01 gp  Modified FFX_Open() and FFX_Close() to allow multiple opens
                 for a given device.
    07/31/01 de  Now use registry values to control size of vbfformat.
    07/30/01 gp  Modified the previous rev to not worry about misaligned
                 lengths, and asserted in the Read/Write code that all the
                 scatter/gather buffer lengths are multiples of the sector
                 size.
    07/26/01 gp  Added support for misaligned buffers in the IO routines.
                 Integrated the NMI suspend/resume code, though it does not
                 seem to be working right, and is currently disabled.
    07/06/01 de  Changed FFX_Init to actually set the STATE_INITIALIZED flag.
    06/20/01 gp  Added the FFX_USEFATMONITOR symbol to control whether the FAT
                 monitor stuff is compiled in.  Modified the FAT monitor stuff
                 so we determine at run-time whether its use is appropriate,
                 since it will only work with FAT12 and FAT16 disks.  Modified
                 to detect writes to the BPB and re-save our copy of the BPB
                 so that the FAT monitor will work (or be avoided) correctly.
                 Changed the FORMAT_MEDIA state to be undefined by default.
                 Modified to be FAT32 aware, though performance will be poor
                 due to no FAT monitor support.  Added asserts to FFX_Open()
                 and FFX_Close() in hopes of ferretting out a possible design
                 problem (see the comments within).  Updated to use FFXPRINTF()
                 rather than DEBUGMSG().  Cleaned up the debug code to make it
                 less voluminous while more useful.
    06/12/01 gp  Cleaned up debug output from FFXDiskEntry().  Fixed several
                 faulty debug messages.
    12/23/99 GJS Changed UNIT_TEST output from FFXPRINTF to RETAILMSG
    10/15/99 TWQ Enabled DISK_INFO_FLAG_PAGEABLE bit in di_flags for the
                 FlashFX disk.  This corrected a problem with CeMountDBVol()
                 returning an "invalid handle" message when trying to create
                 or open a database on a FlashFX disk.
    09/16/99 TWQ Corrected the length returned by DISK_IOCTL_GETNAME.
                 Microsoft Support Article ID: Q233020 describes
                 inconsistencies within it's file system implementaiton.
                 Without this fix, Explorer would not update it's caches and
                 appear to not delete files and incorrectly report read-only
                 files.  A refresh would be required to resync the FATFS.
    09/15/99 PKG Added registry query of the folder type instead of relying
                 on the Windows CE FATFS from trying to get the right
                 information.
    01/20/99 PKG Removed conditional delay in IOCTL call GETNAME in order
                 to allow the needed delay under WinCE v2.10.  This code
                 is not executed in WinCE v2.0 preventing more than one delay
                 under either WinCE v2.0 or 2.10.
    12/11/98 PKG Added delay code in access routines to allow WinCE FATFS v2.0
                 to work when a fault is detected in the FAT file system.
                 Without this delay the FATFS can call into GWES before GWES
                 is loaded causing an exception and disallowing access to
                 the flash disk.  Note that this fix prevents disk access for
                 up to 5 seconds after system startup.  It is only needed in
                 systems using WinCE v2.0 but will always be enabled in order
                 to work with WinCE v2.0.
    11/11/98 PKG Corrected debug output and added index display
    10/26/98 PKG Added GetRegistryInfo to set the correct disk number
                 based on the index value in the registry.  Also now save
                 the device key and pass it to the OEM layer.
    10/19/98 TWQ Change DosGetPartitionStart() to GetPartitionStart()
    09/21/98 PKG Due to the implementation of the FATFS code, a small delay
                 is required in the IOCTL_GETNAME call.  this is due to the
                 fact that the FATFS shiped with WinCE v2.10 makes API calls
                 into GWES.  Since GWES has not finished loading at the time
                 that the FATFS is exectuting, we must pause the FATFS thread
                 to wait for GWES to be fully loaded.
    09/21/98 PKG Now define IOCTL_GETNAME if its not already set
                 This is done for WinCE v2.10 compatibility.
    09/15/98 PKG Increased delay in IOCTL_GETNAME and changed use of
                 VBF_BLOCK_SIZE to VBF_API_BLOCK_SIZE, added local
                 declaration of VBF_API_BLOCK_SIZE if needed.
    09/10/98 PKG Made all code use the BPB for disk information instead of
                 calculating it.  This was causing from small discrepancies
                 that the FATFS was complaining about.
                 - Added calls to MountBPB during xxx_Open and the Init to
                 ensure all BPB information is upto date.
                 - Added the IOCTL_GETNAME call.  Just fail since FATFS will
                 correctly look up the registry entry if it's unsupported.
                 - Corrected call to fatformat() to allow a partially
                 formatted disk to be fully formatted.
                 - Added more debug code.
    08/20/98 PKG Updated debug text output
    07/20/98 PKG Changed all DEBUG to D_DEBUG to avoid clashes
    03/09/98 PKG Added UNIT_TEST support to allow developers to step
                 through the OEM and VBF unit tests
    03/02/98 PKG Made modifications to the handling of the external API to
                 return the correct number of bytes and to map the calling
                 process's pointers into our process.
    02/28/98 PKG Added check in the xxx_Open to disallow more than
                 one process to access the media at the same time.
    02/27/98 PKG Made all retail messages into DEBUG messages
    02/25/98 PKG Fixed the acSectorBuffer declaration, it was missing the
                 type which should have been D_UCHAR, the compiler assumed
                 it was an integer and caused all the offsets to be too big
    02/12/98 PKG Added a FAT monitor and IOCTL code processing
    01/28/98 PKG Extensive modifications and code clean up
    11/25/97 HDS Changed oemsetunit() call to oemsetcurrentdrive()
                 and vbfsetunit() call to vbfsetcurrentdrive().
    11/19/97 HDS Changed include file search to use predefined path.
    09/10/97 PJG Original derived from WinCE ATADISK.C
---------------------------------------------------------------------------*/

#include <windows.h>
#include <windev.h>
#include <devload.h>
#include <diskio.h>
#include <storemgr.h>
#include <pm.h>

#if _WIN32_WCE >= 502
#include <fsioctl.h>    /* Needed for FSCTL_SET_EXTENDED_FLAGS */
#endif

#include <flashfx.h>
#include <fxver.h>
#include <fxdriver.h>
#include <diskapi.h>
#include <fxfmlapi.h>
#include <fxapireq.h>
#include <fmlreq.h>
#include <dlwinutil.h>
#include <dlceutil.h>

#include <ffxwce.h>
#include "ffxdrv.h"

#if FFXCONF_FATSUPPORT
#include <dlfatapi.h>
#endif

#if WINVER >= 0x500
/*  CE 3.0 or later...
*/
HANDLE          ghDevHandle;
#endif


    /*  Always FALSE if not using WinMobile.
    */
    #define ENABLE_WINMOBILE_AUTOLOAD_WORKAROUND    FALSE

/*  Globals
*/
DRIVERDATA      dd;


static void         DbgDumpDiskInfo(PDISK_INFO pDI);
static FFXSTATUS    FfxCeInitDiskParamsFromPhysInfo(FFXDISKINFO *pDisk);
#if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
static void         DiscardDeletedSectors(FFXDISKHANDLE hDisk, D_UINT32 ulStartSec, D_UINT32 ulCount);
static void         IssueDiscards(FFXDISKHANDLE hDisk, D_UINT32 ulStartSec, D_UINT32 ulCount);
#endif



/*-------------------------------------------------------------------
    Entry points for a WinCE disk device driver.

    DLLMain
    DSK_Init           Initializes your device.
    DSK_Deinit         De-initializes your device.
    DSK_Open           Opens a device for reading and/or writing.
    DSK_Close          Closes the device context identified by hDiskContext.
    DSK_Read           Reads data from the device identified by the context.
    DSK_Write          Writes data to the device.
    DSK_Seek           Moves the data pointer in the device.
    DSK_PowerUp        Restores power to a device.
    DSK_PowerDown      Suspends power to the device.
    DSK_IOControl      Sends a command to the device.
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
    Protected: DllMain()

    Parameters:
        hInstance  - Instance of the device driver loaded.
        iReason    - DLL_PROCESS_ATTACH or DLL_PROCESS_DETACH are passed
                     before and after being loaded by the file system
                     driver.
        lpReserved - Just that.

    Return Value:
        Always returns TRUE.
-------------------------------------------------------------------*/
BOOL WINAPI DllMain(
    HINSTANCE   hInstance,
    DWORD       dwReason,
    LPVOID      lpReserved)
{
    DclProfilerEnter("FfxCe:DllMain", 0, 0);
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            FFXPRINTF(1, ("DllMain: DLL_PROCESS_ATTACH hInst=%lX\n", hInstance));
            break;

        case DLL_PROCESS_DETACH:
            FFXPRINTF(1, ("DllMain: DLL_PROCESS_DETACH hInst=%lX\n", hInstance));
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:

            /*  Ignore these notifications since they happen all the time and
                we really don't care.
            */
            break;

        default:
            FFXPRINTF(1, ("DllMain: hInst=%lX dwReason=%d\n", hInstance, dwReason));

            /*  Friendly reminder if we get some new notification.
            */
            DclError();
            break;
    }

    DclProfilerLeave(0UL);
    return TRUE;
}


/*-------------------------------------------------------------------
    Protected: DSK_Init()

    The DSK_Init function initializes the device driver.  When the
    user starts using a device, such as when a PC Card is inserted,
    the Device Manager calls this function to initialize the device.

    This function is not called by applications.  The Device Manager
    specifies a pointer to a string containing the registry path to
    the active key of the specific PC Card device in the dwContext
    parameter.  For example, the string contains
    "HKEY_LOCAL_MACHINE\Drivers\Active\nn" where nn is the device
    number assigned to the device.

    The initialization function might use this information to
      Access data stored in the registry
      Initialize the installed device.
      Allocate resources used globally by the device driver.
      Register a status callback function with Card services.

    When the status of the device changes, the operating system calls
    this status callback function.  For example, the status of a PC
    Card changes when you insert it or remove it.  Map system memory
    and I/O space to a PC Card devices memory and I/O space.  Request
    notification on specific callback events.

    For an interrupt-driven device, register an interrupt callback
    function with Card Services. When the device generates an
    interrupt, the operating system calls this interrupt callback
    function.  After this function returns, the Device Manager
    optionally calls xxx­_IOControl.  Your driver can use this
    function to finish initializing itself after it has been installed.
    For example, a driver might load additional modules that require
    the underlying driver to be installed before they can load.

    Parameters:
        dwContext - Pointer to a string containing the registry path
                    to the active key for the installed device.
    Return Value:
        Handle to the device context created. This handle is passed
        to the DSK_Open, DSK_PowerDown, DSK_PowerUp, and DSK_Deinit
        functions.
-------------------------------------------------------------------*/

// Vladimir: temporrary for debugging purposes only
FFXDISKHANDLE g_dsk;
DWORD DSK_Init(
    DWORD           dwContext)
{
    FFXSTATUS       ffxStat;
    FFXDISKHANDLE   hDsk;

    FFXPRINTF(1, ("DSK_Init() dwContext=%lX\n", dwContext));

    /*  Do one-time driver initialization, as well as device initialization
    */
    ffxStat = FfxCeDriverInit(&dd);
    if(ffxStat != FFXSTAT_SUCCESS)
        return 0;

    FfxDriverLock(dd.pDI);

    hDsk = FfxCeDiskCreate(&dd, (LPCTSTR)dwContext);
    if(!hDsk)
    {
        FFXPRINTF(1, ("Error creating Disk\n"));

        FfxDriverUnlock(dd.pDI);

        FfxCeDriverDeinit(&dd);

        FFXPRINTF(1, ("DSK_Init Failed\n"));

        return 0;
    }

    DclPrintf("Flash memory start block = %lX, count = %lX\n", (*hDsk)->ulBlockOffset, (*hDsk)->ulBlockCount);

  #if FFXCONF_DRIVERAUTOTEST
    FfxDriverUnitTest(NULL, hDsk);
  #endif


  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    ffxStat = FfxDriverAllocatorCreate(hDsk);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        /*  Even though the allocator mount failed, we return success
            anyway so that the driver will load and we can format later.
        */
        DclPrintf("Disk create failed, status=%lX\n", ffxStat);
        DclPrintf("The Disk may need to be formatted\n");
    }
    else
  #endif
    {
        ffxStat = FfxCeInitDiskParamsFromPhysInfo(*hDsk);
        if(ffxStat != FFXSTAT_SUCCESS)
        {
            DclPrintf("Critical error intializing FlashFX, status=%lX\n", ffxStat);
            DclError();
        }
    }

    FfxDriverUnlock(dd.pDI);

// Vladimir: temporrary for debugging purposes only
	g_dsk = hDsk;
    FFXPRINTF(1, ("DSK_Init returning hDsk=%P\n", hDsk));

    return (DWORD)hDsk;
}


/*-------------------------------------------------------------------
    Protected: DSK_Deinit()

    Deinit function called to de-initialize the device driver.

    When the user stops using a device, such as when a PC Card is
    removed from its socket, the operating system Device Manager
    calls this function. This function is not called by applications.

    Free any resources that it has allocated and terminate.

    Parameters:
        hDiskContext - Device context created during Init

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
BOOL DSK_Deinit(
    DWORD           hDiskContext)
{
    FFXSTATUS       ffxStat;
    FFXDISKHANDLE   hDsk = (FFXDISKHANDLE)hDiskContext;

    FFXPRINTF(1, ("DSK_Deinit hDsk=%P\n", hDsk));

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT && QUEUE_PENDING_DISCARDS
    FfxCeFlushPendingDiscards(hDsk);
  #endif

    FfxDriverLock(dd.pDI);

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    ffxStat = FfxDriverAllocatorDestroy(hDsk, FFX_SHUTDOWNFLAGS_NORMAL);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("Error destroying the allocator instance for DISK%u, Status=%lX\n", (*hDsk)->Conf.nDiskNum, ffxStat);
        FfxDriverUnlock(dd.pDI);
        return FALSE;
    }
  #endif

    ffxStat = FfxCeDiskDestroy(hDsk);

    FfxDriverUnlock(dd.pDI);

    if(ffxStat == FFXSTAT_SUCCESS)
        ffxStat = FfxCeDriverDeinit(&dd);

    FFXPRINTF(1, ("DSK_Deinit() completed with Status %lX\n", ffxStat));

    if(ffxStat == FFXSTAT_SUCCESS)
        return TRUE;
    else
        return FALSE;
}


/*-------------------------------------------------------------------
    Protected: DSK_Open()

    The Open function opens a device for reading and/or writing.
    An application indirectly invokes this function when it opens
    a device.  An application calls CreateFile("xxx#:", ...) to open
    a device.  The operating system, in turn, invokes Open.  xxx# is a
    string that consists of two parts.  The first part is the prefix
    string found in the operating system registry identifying the
    device type.  The second part indicates the number of the device
    type that you want to open.  For example, all serial devices have
    a "COM" prefix, and each is labeled with a number, such as "COM1:"
    and "COM2:".

    Allocate resources needed for each open context and prepare for
    operation.  This might involve preparing the device for reading or
    writing, and initializing data structures it uses for operation.

    Parameters:
        hDiskContext - Device context handle. Created by DSK_Init.
        dwAccessCode - Specifies the requested access code of the device.
                       The access is a combination of read and write.
        dwShareMode  - Specifies the requested file share mode of the
                       PC Card.  The share mode is a combination of
                       file read and write sharing.

    Return Value:
        Handle that identifies the open context of the device.  If your
        device can be opened multiple times, use this handle to identify
        each open context.  This identifier is passed into the DSK_Read,
        DSK_Write, DSK_Seek, and DSK_IOControl functions as a handle for
        the PC Card device.  If the device cannot be opened, the function
        returns NULL.
-------------------------------------------------------------------*/
DWORD DSK_Open(
    DWORD           hDiskContext,
    DWORD           dwAccessCode,
    DWORD           dwShareMode)
{
    FFXDISKHANDLE   hDsk = (FFXDISKHANDLE)hDiskContext;

    FFXPRINTF(1, ("DSK_Open hDsk=%P\n", hDsk));

    DclProfilerEnter("FfxCe:DSK_Open", 0, 0);
    FfxDriverLock(dd.pDI);
    {
        if(!((*hDsk)->pDiskHook->dwFlags & STATE_OPENED))
        {
            DclAssert(!(*hDsk)->pDiskHook->ulOpenCount);

            (*hDsk)->pDiskHook->dwFlags |= STATE_OPENED;
        }
    }

    (*hDsk)->pDiskHook->ulOpenCount++;

    FfxDriverUnlock(dd.pDI);

    DclProfilerLeave(0UL);
    FFXPRINTF(1, ("DSK_Open returning hContext %lX, ulOpenCount=%lU\n",
               hDiskContext, (*hDsk)->pDiskHook->ulOpenCount));

    /*  Just return the device context created by the Init
    */
    return hDiskContext;
}


/*-------------------------------------------------------------------
    Protected: DSK_Close()

    The Close function closes the device context identified by
    hDiskContext and previously created by Open.

    An application uses CloseHandle(hFile) to close the context. The
    hFile argument specifies the handle associated with the device
    context.  In response to CloseHandle, the operating system invokes
    Close.  The file handle specified for hDiskContext is invalid after
    this function returns.

    Parameters:
        hDiskContext - Device open context handle created by DSK_Open.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
BOOL DSK_Close(
    DWORD           hDiskContext)
{
    FFXDISKHANDLE   hDsk = (FFXDISKHANDLE)hDiskContext;

    FFXPRINTF(1, ("DSK_Close hDsk=%P\n", hDsk));

    DclProfilerEnter("FfxCe:DSK_Close", 0, 0);
    DclAssert((*hDsk)->pDiskHook->dwFlags & STATE_OPENED);
    DclAssert((*hDsk)->pDiskHook->ulOpenCount);

    FfxDriverLock(dd.pDI);
    {
      #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT && QUEUE_PENDING_DISCARDS
        FfxCeFlushPendingDiscards(hDsk);
      #endif

        (*hDsk)->pDiskHook->ulOpenCount--;

        if((*hDsk)->pDiskHook->ulOpenCount == 0)
        {
            /*  Clear the open flag
            */
            (*hDsk)->pDiskHook->dwFlags &= ~STATE_OPENED;
        }
    }
    FfxDriverUnlock(dd.pDI);

    DclProfilerLeave(0UL);
    FFXPRINTF(1, ("DSK_Close, OpenCount=%lU\n", (*hDsk)->pDiskHook->ulOpenCount));

    return TRUE;
}


/*-------------------------------------------------------------------
    Protected: DSK_Read()

    The Read function reads data from the device identified by the
    open context.  An application calls ReadFile to read from the
    device.  The operating system, in turn, invokes Read.  The hFile
    argument is a handle for your device.  The pData argument points
    to the buffer that contains the data read from the device.  The
    Size argument indicates the number of bytes you want to read from
    the device.  The pSizeRead argument is the address of a value
    that holds the number of bytes  actually read from the device into
    the buffer.

    The value returned from DSK_Read is equal to the value contained
    in pSizeRead, unless Read returns -1 for an error. If Read returns
    an error, then pSizeRead will contain 0.

    NOTE: This function is not implemented!

    Parameters:
        hDiskContext - Device open context handle created by DSK_Open.
        pBuffer      - Points to the buffer which stores the data read
                       from the device.  This buffer should be at least
                       Count bytes long.
        dwCount      - Specifies the number of bytes to read from the
                       device into pBuffer.

    Return Value:
        The number of bytes read for success, OR 0 for the end-of-file,
        OR -1 for an error.
-------------------------------------------------------------------*/
DWORD DSK_Read(
    DWORD       hDiskContext,
    LPVOID      pBuffer,
    DWORD       dwCount)
{
    FFXPRINTF(1, ("DSK_Read hContext=%lX pBuff=%P len=%lX\n",
                  hDiskContext, pBuffer, dwCount));

  #if D_DEBUG
    DebugBreak();
  #endif

    /*  Do nothing for now
    */
    return (DWORD) 0;
}


/*-------------------------------------------------------------------
    Protected: DSK_Write()

    The Write function writes data to the device.  An application uses
    WriteFile to write to the PC Card device.  The operating system,
    in turn, invokes Write.  The hFile argument is a handle for your
    device.  The pData argument points to the buffer that contains the
    data to transmit to the device.  The Size argument indicates the
    number of bytes you want to transmit.  The pSizeWritten argument
    is the address of a value that holds the number of bytes actually
    transmitted from the buffer into the device.  The value returned
    from Write is equal to the value contained in pSizeWritten,
    unless Write returns -1 for an error. In that case, pSizeWritten
    will contain 0.

    NOTE: This function is not implemented!

    Parameters:
        hDiskContext   - Device open context handle created by DSK_Open.
        pSourceBytes   - Specifies the address of the buffer from which
                         the PC Card device gets data.
       dwNumberOfBytes - Specifies the number of bytes to write from the
                         pSourceBytes buffer into the PC Card device.

    Return Value:
       The number of bytes read for success, OR 0 for the end-of-file,
       OR -1 for an error.
-------------------------------------------------------------------*/
DWORD DSK_Write(
    DWORD       hDiskContext,
    LPCVOID     pSourcebytes,
    DWORD       dwNumberOfBytes)
{
    FFXPRINTF(1, ("DSK_Write hContext=%lX pBuff=%P len=%lX\n",
                  hDiskContext, pSourcebytes, dwNumberOfBytes));

  #if D_DEBUG
    DebugBreak();
  #endif

    /*  Do nothing for now
    */
    return (DWORD) 0;
}


/*-------------------------------------------------------------------
    Protected: DSK_Seek()

    The Seek function moves the data pointer in the device.  An
    application calls SetFilePointer to move the data pointer in
    the device.  The operating system, in turn, invokes Seek.
    The hFile argument is a handle for your device.  The MoveDis
    argument specifies the number of bytes you want to move the
    device data pointer.

    The dwType argument specifies the starting point of the move
    and can be one of the following:
       FILE_BEGIN - Indicates the starting point is zero or the
          beginning of the file.
        FILE_CURRENT - Indicates the current value of the file pointer
          is the starting point.
       FILE_END - Indicates the current end-of-file position is the
          starting point.

    If your device is capable of being opened more than once at a
    time, Seek should only modify the data pointer for the instance
    specified by hDiskContext

    Parameters:
        hDiskContext - Device open context handle created by DSK_Open.
        lpos         - Specifies the number of bytes to move the data
                       pointer in the device.  A positive value moves
                       the data pointer forward and a negative value
                       moves it backward.
        dwtype       - Specifies the starting point for the data
                       pointer move.

    Return Value:
        -1 for an error, or the device's new data pointer for success.
-------------------------------------------------------------------*/
DWORD DSK_Seek(
    DWORD       hDiskContext,
    long        lpos,
    DWORD       dwType)
{
    FFXPRINTF(1, ("DSK_Seek hContext=%lX pos=%P type=%lX\n",
                  hDiskContext, lpos, dwType));

  #if D_DEBUG
    DebugBreak();
  #endif

    return (DWORD) 0;
}


/*-------------------------------------------------------------------
    Protected: DSK_PowerUp()

    The xxx_PowerUp function restores power to a device.

    The operating system invokes this function to restore power to
    a device.  The operating system might call this function as it
    is leaving its power saving mode.  This function should not call
    any functions that may cause it to block and should return as
    quickly as possible.  This function should set a global variable
    to indicate that power was restored and perform any necessary
    processing later.

    Note Some Windows CE platforms provide a subset of the
    specifications for PC Card power.  If you are writing a driver
    for a PC Card device, consult with the platform's manufacturer
    to ensure that it can support your PC Card.

    Parameters:
        hDiskContext - Device open context handle created by DSK_Open.

    Return Value:
        None.
-------------------------------------------------------------------*/
VOID DSK_PowerUp(void)
//VOID DSK_PowerUp(
//    DWORD           hDiskContext)
{
//    FFXDISKHANDLE   hDisk = (FFXDISKHANDLE)hDiskContext;

    //FFXPRINTF(1,  ("DSK_PowerUp hContext=%lX\n", hDiskContext));
    FFXPRINTF(1, ("DSK_PowerUp hContext=%lX\n", g_dsk));

  #if FFXCONF_POWERSUSPENDRESUME
// Vladimir: temporrary for debugging purposes only
//    FfxDriverDiskPowerResume(hDisk, 0);
    FfxDriverDiskPowerResume(g_dsk, 0);
  #endif

#if WINVER >= 0x500
    {
        /*  CE 3.0 or later...
        */
//      BOOL fSuccess;

//      DclAssert(ghDevHandle);

//      fSuccess = CeResyncFilesys(ghDevHandle);

//      DclAssert(fSuccess);
    }
#endif

    return;
}


/*-------------------------------------------------------------------
    Protected: DSK_PowerDown()

    The PowerDown function suspends power to the device.  This is
    only useful with devices that can be shut off under software
    control.  Usually such devices are PC Card devices, but not
    exclusively.

    The operating system invokes this function to suspend power to
    the device.  The operating system might call this function when
    it is about to enter the power saving mode.

    This function should never call any functions that might cause
    it to block and it should return as quickly as possible.
    One strategy for returning quickly is to have this function set
    a global variable to indicate that a power loss occurred and
    perform any necessary processing later.

    Parameters:
        hDiskContext - Device open context handle created by DSK_Open.

    Return Value:
        None.
-------------------------------------------------------------------*/
VOID DSK_PowerDown(void)
// Vladimir
//VOID DSK_PowerDown(
//    DWORD           hDiskContext)
{
//    FFXDISKHANDLE   hDisk = (FFXDISKHANDLE)hDiskContext;

    if(!dd.ulInitCount)
    {
//        DclPrintf("DSK_PowerDown(%lX) called while the driver is not loaded!\n", hDisk);
        DclPrintf("DSK_PowerDown(%lX) called while the driver is not loaded!\n", g_dsk);
        return;
    }

  
//    FFXPRINTF(1, ("DSK_PowerDown hContext=%lX\n", hDiskContext));
    FFXPRINTF(1, ("DSK_PowerDown hContext=%lX\n", g_dsk));


  #if FFXCONF_POWERSUSPENDRESUME
// Vladimir: temporrary for debugging purposes only
//    FfxDriverDiskPowerSuspend(hDisk, 0);
    FfxDriverDiskPowerSuspend(g_dsk, 0);
  #endif

    return;
}


/*-------------------------------------------------------------------
    Proected: DSK_IOControl()

    The IOControl function sends a command to the device.

    If the Ctrl registry value is defined for your device driver,
    the Device Manager calls this function right after it calls Init.
    The Device Manager also specifies NULL for the pBufIn and pBufOut
    arguments.  Your device might use this option to load other
    modules that require the basic device driver to be installed.

    An application uses DeviceIOControl to specify an operation to
    be performed.  The operating system, in turn, invokes IOControl.
    The hFile argument is the handle created for your device driver's
    Open function.  The dwCode argument contains the input or output
    operation requested.  The pDataIn argument points to the buffer
    containing the data the application supplies.  The pDataOut
    argument points to the buffer that stores the data the application
    receives.  The pByteCnt argument is the address of a value that
    specifies the number of bytes stored in the pDataOut buffer.

    Other IOCTL codes defined for Datalight FlashFX entry points
    are described in the FXCEIO.H.

    Parameters:
        hDiskContext - Device open context handle created by DSK_Open.
        dwIOCTLCode  - Specifies a value indicating the I/O control
                       operation to perform.  These codes are device
                       specific, and are usually exposed to application
                       programmers by means of a header file.
        pBufIn       - Points to the buffer containing data to be
                       transferred to the device.
        dwLenIn      - Specifies the number of bytes of data in the
                       buffer specified for pBufIn.
        pBufOut      - Points to the buffer used to transfer the
                       output data from the device.
        dwLenOut     - Specifies the maximum number of bytes in the
                       buffer specified by pBufOut
        pdwActualOut - Points to DWORD buffer the function uses to
                       return the actual number of bytes received
                       from the device.

    Return Value:
        Returns TRUE if the device successfully completed its
        specified I/O control operation, or FALSE otherwise.
-------------------------------------------------------------------*/
BOOL DSK_IOControl(
    DWORD           hDiskContext,
    DWORD           dwIOCTLCode,
    PBYTE           pBufIn,
    DWORD           dwLenIn,
    PBYTE           pBufOut,
    DWORD           dwLenOut,
    PDWORD          pdwActualOut)
{
    FFXDISKHANDLE   hDsk = (FFXDISKHANDLE)hDiskContext;
    DWORD           dwLastError = ERROR_INVALID_PARAMETER;

    /*  The disk to access has already been setup and described in a
        call to the DSK_Open function.  Only one process per access.
    */
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEINDENT),
        "DSK_IOControl() hContext: %lX  dwIOCTLCode: %lX\n", hDiskContext, dwIOCTLCode));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 2, 0),
        "pBufIn=%P LenIn=%lX pBuffOut=%P LenOut=%lX\n", pBufIn, dwLenIn, pBufOut, dwLenOut));

    DclProfilerEnter("FfxCe:DSK_IOControl", 0, 0);

    switch (dwIOCTLCode)
    {
        case IOCTL_DISK_READ:
        case DISK_IOCTL_READ:
        {
            DWORD           dwBytesRead;
            SG_REQ         *pSG = (SG_REQ*)pBufIn;

            FFXPRINTF(3, ("IOCTL_DISK_READ\n"));

            /*  If buffer pointer is invalid, report it now
            */
            if (pSG == NULL || dwLenIn < sizeof(SG_REQ) ||
                  dwLenIn > (sizeof(SG_REQ) + ((MAX_SG_BUF - 1) * sizeof(SG_BUF))))
            {
                goto IOCTL_Cleanup;
            }

            dwBytesRead = FfxCeIoctlReadWrite(hDsk, DISK_IOCTL_READ, pSG);
            if(pSG->sr_status != ERROR_SUCCESS)
            {
                /*  Let the caller know they can use GetLastError
                */
                dwLastError = pSG->sr_status;

                goto IOCTL_Cleanup;
            }

            /*  When using the Scandisk funtion from within the Storage
                Manager applet on CE.net, this function is called with a
                null pdwActualOut value.
                DclProductionAssert(pdwActualOut);
            */
            if(pdwActualOut)
                *pdwActualOut = dwBytesRead;

            break;
        }

        case IOCTL_DISK_WRITE:
        case DISK_IOCTL_WRITE:
        {
            SG_REQ         *pSG = (SG_REQ*)pBufIn;

            FFXPRINTF(3, ("IOCTL_DISK_WRITE\n"));

            /*  if buffer pointer is invalid, report it now
            */
            if (pSG == NULL || dwLenIn < sizeof(SG_REQ) ||
                  dwLenIn > (sizeof(SG_REQ) + ((MAX_SG_BUF - 1) * sizeof(SG_BUF))))
            {
                goto IOCTL_Cleanup;
            }

            FfxCeIoctlReadWrite(hDsk, DISK_IOCTL_WRITE, pSG);
            if(pSG->sr_status != ERROR_SUCCESS)
            {
                /*  Let the caller know they can use GetLastError
                */
                dwLastError = pSG->sr_status;

                goto IOCTL_Cleanup;
            }

            break;
        }

      #if _WIN32_WCE >= 500
        case IOCTL_DISK_FLUSH_CACHE:
        {
            FFXSTATUS   ffxStat;

            FFXPRINTF(2, ("IOCTL_DISK_FLUSH_CACHE\n"));

            ffxStat = FfxCeIoctlFlush(hDsk);
            if(ffxStat != FFXSTAT_SUCCESS)
            {
                /*  Let the caller know they can use GetLastError
                */
                dwLastError = ERROR_GEN_FAILURE;

                goto IOCTL_Cleanup;
            }

            break;
        }
      #endif

        /*  Should already be correct from the xxx_Open call!
        */
      #ifdef IOCTL_DISK_GETINFO       /* not defined for CE 2.0 */

        /*  Note that the code for this IOCTL is subtly different than
            that for DISK_IOCTL_GETINFO.
        */
        case IOCTL_DISK_GETINFO:
            FFXPRINTF(1, ("IOCTL_DISK_GETINFO\n"));

            DclAssert(dwLenOut == sizeof(DISK_INFO));

            /*  if buffer pointer is invalid, report it now
            */
            if (!pBufOut || dwLenOut != sizeof(DISK_INFO))
                goto IOCTL_Cleanup;

            /*  We have an MBR out there and clear the unformatted flag
            */
            *((PDISK_INFO) pBufOut) = (*hDsk)->pDiskHook->CeDiskInfo;

            ((PDISK_INFO) pBufOut)->di_flags &= ~DISK_INFO_FLAG_UNFORMATTED;

            ((PDISK_INFO) pBufOut)->di_flags |= DISK_INFO_FLAG_PAGEABLE;

            /*  Always use an MBR with CE
            */
            ((PDISK_INFO) pBufOut)->di_flags |= DISK_INFO_FLAG_MBR;

            DbgDumpDiskInfo((PDISK_INFO) pBufOut);

            if(pdwActualOut)
                *pdwActualOut = sizeof(DISK_INFO);

            break;
      #endif

        case DISK_IOCTL_GETINFO:
            FFXPRINTF(1, ("DISK_IOCTL_GETINFO\n"));

            DclAssert(dwLenIn == sizeof(DISK_INFO));

            /*  if buffer pointer is invalid, report it now
            */
            if (!pBufIn || dwLenIn != sizeof(DISK_INFO))
                goto IOCTL_Cleanup;

            /*  We have an MBR out there and clear the unformatted flag
            */
            *((PDISK_INFO) pBufIn) = (*hDsk)->pDiskHook->CeDiskInfo;

            ((PDISK_INFO) pBufIn)->di_flags &= ~DISK_INFO_FLAG_UNFORMATTED;

            ((PDISK_INFO) pBufIn)->di_flags |= DISK_INFO_FLAG_PAGEABLE;

            /*  Always use an MBR with CE
            */
            ((PDISK_INFO) pBufIn)->di_flags |= DISK_INFO_FLAG_MBR;

            DbgDumpDiskInfo((PDISK_INFO) pBufIn);

            break;

        case DISK_IOCTL_SETINFO:
            FFXPRINTF(1, ("DISK_IOCTL_SETINFO stubbed\n"));

            /*  if buffer pointer is invalid, report it now
            */
            if (!pBufIn || dwLenIn != sizeof(DISK_INFO))
                goto IOCTL_Cleanup;

            DbgDumpDiskInfo((DISK_INFO*)pBufIn);

            break;

      #ifdef IOCTL_DISK_GET_STORAGEID   /* not defined for CE 2.0 */
        case IOCTL_DISK_GET_STORAGEID:
        {
            STORAGE_IDENTIFICATION *pStorageID = (STORAGE_IDENTIFICATION*)pBufOut;

            FFXPRINTF(1, ("IOCTL_DISK_GET_STORAGEID\n"));

            /*  if buffer pointer is invalid, report it now
            */
            if (!pBufOut || dwLenOut < sizeof(*pStorageID))
                goto IOCTL_Cleanup;

            pStorageID->dwSize = sizeof(*pStorageID);
            pStorageID->dwFlags = MANUFACTUREID_INVALID | SERIALNUM_INVALID;
            pStorageID->dwManufactureIDOffset = 0;
            pStorageID->dwSerialNumOffset = 0;

            break;
        }
       #endif

        case IOCTL_DISK_DEVICE_INFO:
        {
            STORAGEDEVICEINFO *pSDI = (STORAGEDEVICEINFO*)pBufIn;

            FFXPRINTF(1, ("IOCTL_DISK_DEVICE_INFO\n"));

            /*  if buffer pointer is invalid, report it now
            */
            if (!pSDI || dwLenIn != sizeof(*pSDI))
                goto IOCTL_Cleanup;

            wcscpy(pSDI->szProfile, (*hDsk)->pDiskHook->tzProfileName);

            pSDI->cbSize = dwLenIn;
            pSDI->dwDeviceClass = STORAGE_DEVICE_CLASS_BLOCK;
            pSDI->dwDeviceType = STORAGE_DEVICE_TYPE_FLASH;
            pSDI->dwDeviceFlags = STORAGE_DEVICE_FLAG_READWRITE;

          #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
            if(!(*hDsk)->hVBF)
            {
                /*  The readonly XIP partition in WinMobile has the
                    "transacted" flag set.  Amazingly it does <not>
                    have the "readonly", but is marked read-write
                    like the other stores.
                */
                pSDI->dwDeviceFlags |= STORAGE_DEVICE_FLAG_TRANSACTED;
            }

          #else

            pSDI->dwDeviceFlags |= STORAGE_DEVICE_FLAG_TRANSACTED;
          #endif

            break;
        }

        case DISK_IOCTL_INITIALIZED:
            FFXPRINTF(1, ("DISK_IOCTL_INITIALIZED\n"));

          #if WINVER >= 0x500

            /*  CE 3.0 or later...
            */

            /*  if buffer pointer is invalid, report it now
            */
            if (!pBufIn)
                goto IOCTL_Cleanup;

            ghDevHandle = ((PPOST_INIT_BUF) pBufIn)->p_hDevice;
          #endif

            break;

        case IOCTL_DISK_GETNAME:
        case DISK_IOCTL_GETNAME:
            FFXPRINTF(1, ("IOCTL_DISK_GETNAME\n"));

            if(pBufOut == NULL || pdwActualOut == NULL)
                goto IOCTL_Cleanup;

            DclAssert(pdwActualOut);

            if(!(*(*hDsk)->pDiskHook->tzFolderName))
            {
                *pdwActualOut = 0;
            }
            else
            {
                /*  Give the caller the name of the folder
                */
                wcscpy((LPWSTR)pBufOut, (LPWSTR)(*hDsk)->pDiskHook->tzFolderName);

                /*  Save the size and account for terminating 0 (2 bytes)
                    and one more unknown unicode character (2 bytes).  This
                    extra unknown character is documented in Microsoft's
                    Knowledge base, Article Q233020 (PRB: Explorer Shell Does
                    Not Refresh After File Deletion)

                    Note: wcslen() is returning a number of unicode
                    characters not single byte characters.
                */
                *pdwActualOut = (wcslen((LPWSTR)(*hDsk)->pDiskHook->tzFolderName) * 2) + 4;

                FFXPRINTF(1, ("IOCTL_DISK_GETNAME() FolderName = %W, length = %lD\n",
                           pBufOut, *pdwActualOut));
            }

            break;

        case IOCTL_DISK_FORMAT_MEDIA:
        case DISK_IOCTL_FORMAT_MEDIA:
            FFXPRINTF(1, ("IOCTL_DISK_FORMAT_MEDIA stubbed\n"));

            /*  Note that the CE 4.0 documentation for this IOCTL is
                incorrect.  it has been corrected in CE 4.1 and later.
            */

            /*  This IOCTL is used when CE does the equivalent of a low-
                level format on the disk.

                With CE 4.0 and 4.1, immediately following this ioctl, the
                disk is completely written with 0's.  This results in poor
                FFX performance since VBF thinks the disk is being filled.
                We really need to do a FfxVbfFormat() AFTER the disk is
                written with 0's.  Fortunately with CE 4.2, only the FAT
                and root directory is wiped.
            */

            DclAssert(pdwActualOut);

            /*  check for invalid parameters and report error if
                necessary.
            */
            if (pdwActualOut == NULL)
                goto IOCTL_Cleanup;

            *pdwActualOut = 0;

            break;

        case IOCTL_DISK_DELETE_CLUSTER:
            FFXPRINTF(1, ("IOCTL_DISK_DELETE_CLUSTER stubbed\n"));
            goto IOCTL_Cleanup;

        case IOCTL_DISK_DELETE_SECTORS:
        {
            DELETE_SECTOR_INFO *pDSI;

            FFXPRINTF(2, ("IOCTL_DISK_DELETE_SECTORS\n"));

            pDSI = (DELETE_SECTOR_INFO*)pBufIn;

            /*  if buffer pointer is invalid, report it now
            */
            if (!pDSI || dwLenIn != sizeof(*pDSI))
                goto IOCTL_Cleanup;

            DclAssert(pDSI->cbSize == sizeof(*pDSI));

          #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
            /*  We never want to do this if we have FATMON enabled, or if
                the delsectors ioctl is broken as it is in CE 4.0.
            */
            if(!dd.fUseFatMon && !dd.fDelSectorsBroken)
                DiscardDeletedSectors(hDsk, pDSI->startsector, pDSI->numsectors);
          #endif

            break;
        }

      #ifdef IOCTL_PSL_NOTIFY
        /*  This IOCTL is used by device drivers if a process is exiting, that
            is, its main thread terminates, while one or more other threads are
            running.  This IOCTL can also be used when secondary threads that
            are blocked in a system call do not exit cleanly when requested by
            the scheduler.  This IOCTL provides a mechanism for the system call
            implementers to unblock threads belonging to processes that have
            been terminated.

            Device drivers use this IOCTL to perform custom processing not
            handled by the XXX_Close function.

            If threads owned by disappearing processes are blocked in a call
            to a driver, the OS uses IOCTL_PS_NOTIFY to notify the driver to
            unblock those threads.  The OS invokes IOCTL_PSL_NOTIFY when the
            all of the following conditions are true:

                The application's main thread exits.
                The application has other threads still running.
                The application has open file handles that refer to the device.(from MSDN)
        */
        case IOCTL_PSL_NOTIFY:
        {
            DEVICE_PSL_NOTIFY *pDPN = (DEVICE_PSL_NOTIFY*)pBufIn;

            /*  if buffer pointer is invalid, report it now
            */
            if(!pDPN)
                goto IOCTL_Cleanup;

            FFXPRINTF(1, ("IOCTL_PSL_NOTIFY, Process handle: %lX Thread handle:%lX\n",
                       pDPN->hProc, pDPN->hThread));
            break;
        }
      #endif

        case DCL_IOCTL_BLOCKDEV:
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 3, 0),
                "DCL_IOCTL_BLOCKDEV %lX\n", dwIOCTLCode));

            DclAssert(pBufIn == pBufOut);
            DclAssert(dwLenIn == dwLenOut);

            if(pBufIn == NULL)
                goto IOCTL_Cleanup;

            FfxCeDispatchIoctl(hDsk, (FFXIOREQUEST*)pBufIn, dwLenIn);

            DclAssert(pdwActualOut);

            *pdwActualOut = dwLenIn;

            break;


        case 0x91F80:
            /*  Received this once when switching between Reliance and FAT
                using the Control Panel utilities.  Can't find any docs
                for it other than it is in the FILE_DEVICE_FILE_SYSTEM
                class.

                Got it when pressing "scan" in the Reliance Tools Applet,
                and the disk was already formatted with FAT.

                NOTE: This is FSCTL_USER, subfunction 0.  See fsioctl.h in
                      WinMobile 6.
            */
            FFXPRINTF(1, ("Invalid IOCTL = %lX\n", dwIOCTLCode));

            goto IOCTL_Cleanup;


        /*  Called at init to determine what states the device supports, 
            as well as power requirements.
        */    
		case IOCTL_POWER_CAPABILITIES:
            FFXPRINTF(1, ("IOCTL_POWER_CAPABILITIES"));

			if(!pBufOut || dwLenOut < sizeof(POWER_CAPABILITIES) || !pdwActualOut)
			{
                goto IOCTL_Cleanup;
			}
			else
			{
				POWER_CAPABILITIES  SafePowerCapabilities = {0};
				DWORD               dwSafeBytesReturned = sizeof(POWER_CAPABILITIES);

                SafePowerCapabilities.DeviceDx = DX_MASK(D0) | DX_MASK(D4);

				SafePowerCapabilities.Power[D0] = PwrDeviceUnspecified;
				SafePowerCapabilities.Power[D1] = PwrDeviceUnspecified;
				SafePowerCapabilities.Power[D2] = PwrDeviceUnspecified;
				SafePowerCapabilities.Power[D3] = PwrDeviceUnspecified;
				SafePowerCapabilities.Power[D4] = PwrDeviceUnspecified;

				if(!CeSafeCopyMemory((LPVOID)pBufOut, (LPVOID)&SafePowerCapabilities, sizeof(POWER_CAPABILITIES)))
				{
                    goto IOCTL_Cleanup;
				}
				else if(!CeSafeCopyMemory((LPVOID)pdwActualOut, (LPVOID)&dwSafeBytesReturned, sizeof(DWORD)))
				{
                    goto IOCTL_Cleanup;
				}
			}
            
			break;

		/*  Called at various times to change the power state to a specific
		    Dx-type state.  Unlike DSK_PowerUp() and DSK_PowerDown(), this can
		    use any driver level APIs.
        */		    
		case IOCTL_POWER_SET:
        {
            TCHAR   tzStateName[32];
            DWORD   dwResult;
            DWORD   dwFlags;

            FFXPRINTF(1, ("IOCTL_POWER_SET, Level=D%lU", *(CEDEVICE_POWER_STATE*)pBufOut));

            dwResult = GetSystemPowerState(tzStateName, DCLDIMENSIONOF(tzStateName), &dwFlags);
            if(dwResult == ERROR_SUCCESS)
                FFXPRINTF(1, ("New Power State is \"%W\", (%lX)\n", tzStateName, dwFlags));
            else
                FFXPRINTF(1, ("GetSystemPowerState() returned error %lU\n", dwResult));
            
            if((*(CEDEVICE_POWER_STATE*)pBufOut == D4) && (dwFlags == POWER_STATE_RESET))
            {
                /*  Only in the event that the power state is D4 and we are
                    doing a "reset" do we want to do this.  This will be called
                    for each Disk, and once the final Disk is completed, the
                    driver will de-initialize all Devices, and effectively
                    become "unloaded".

                    For standard "suspend" operations, do nothing here, and
                    simply let the legacy DSK_PowerDown() function do the
                    work.
                */                    
                DSK_Deinit(hDiskContext);
           	}
            
			break;
		}
        
        /*  Determines the current Dx-type state.
        */
		case IOCTL_POWER_GET:
            FFXPRINTF(1, ("IOCTL_POWER_GET"));
            
            if(!pBufOut || dwLenOut < sizeof(POWER_CAPABILITIES) || !pdwActualOut)
			{
                goto IOCTL_Cleanup;
			}
			else
			{
                /*  ToDo: The current power state should be maintained outside
                          this scope, possibly in the Disk context.  For now
                          we are going to always return D0, assuming that we
                          won't get this call between the time we go to D4 and
                          the system reboots.
                */
                CEDEVICE_POWER_STATE    power_state = D0;
                DWORD                   dwSafeBytesReturned = sizeof(POWER_CAPABILITIES);

				if(!CeSafeCopyMemory((LPVOID)pBufOut, (LPVOID)&power_state, sizeof(CEDEVICE_POWER_STATE)))
				{
                    goto IOCTL_Cleanup;
				}
				else if(!CeSafeCopyMemory((LPVOID)pdwActualOut, (LPVOID)&dwSafeBytesReturned, sizeof(DWORD)))
				{
                    goto IOCTL_Cleanup;
				}
			}
            
			break;

		case IOCTL_POWER_QUERY:
            FFXPRINTF(1, ("IOCTL_POWER_QUERY is stubbed"));
			break;

		case IOCTL_REGISTER_POWER_RELATIONSHIP:
            FFXPRINTF(1, ("IOCTL_REGISTER_POWER_RELATIONSHIP is stubbed"));
			break;

      #if _WIN32_WCE >= 502
		case IOCTL_DISK_SET_SECURE_WIPE_FLAG:
            FFXPRINTF(1, ("IOCTL_DISK_SET_SECURE_WIPE_FLAG is not supported"));
            dwLastError = ERROR_NOT_SUPPORTED;
            goto IOCTL_Cleanup;

		case IOCTL_DISK_SECURE_WIPE:
        {
            DELETE_SECTOR_INFO *pDSI;

            FFXPRINTF(1, ("IOCTL_DISK_SECURE_WIPE\n"));

            pDSI = (DELETE_SECTOR_INFO*)pBufIn;

            /*  If buffer pointer is invalid, report it now
            */
            if (!pDSI || dwLenIn != sizeof(*pDSI))
                goto IOCTL_Cleanup;

            DclAssert(pDSI->cbSize == sizeof(*pDSI));

            FFXPRINTF(1, ("Wiping SectorStart=%lX Count=%lX\n", pDSI->startsector, pDSI->numsectors));

          #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
            /*  We never want to do this if we have FATMON enabled, or if
                the delsectors ioctl is broken as it is in CE 4.0.
            */
            if(!dd.fUseFatMon && !dd.fDelSectorsBroken)
            {
                DiscardDeletedSectors(hDsk, pDSI->startsector, pDSI->numsectors);

              #if QUEUE_PENDING_DISCARDS
                FfxCeFlushPendingDiscards(hDsk);
              #endif
            }

            FfxDriverDiskWipe(hDsk);
            
          #else

            dwLastError = ERROR_NOT_SUPPORTED;
            goto IOCTL_Cleanup;

          #endif

            break;
        }
      #endif
           
        default:

            FFXPRINTF(1, ("DSK_IOControl() Invalid IOCTL %lX\n", dwIOCTLCode));

                /* DclError(); */

                dwLastError = ERROR_NOT_SUPPORTED;
                goto IOCTL_Cleanup;
    }

    /*  Success if we get to this point!
    */
    dwLastError = ERROR_SUCCESS;

  IOCTL_Cleanup:

    DclProfilerLeave(0UL);
    
    if(dwLastError == ERROR_SUCCESS)
    {
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 2, TRACEUNDENT),
            "DSK_IOControl() returning SUCCESS\n"));

        return TRUE;
    }
    else
    {
        /*  Call SetLastError() only in the event that there actually 
            WAS an error.  Never do SetLastError(ERROR_SUCCESS).
        */            
        SetLastError(dwLastError); 

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEUNDENT),
            "DSK_IOControl() for %lX failed, dwLastError=%lX\n", dwIOCTLCode, dwLastError));

        return FALSE;
    }
}


/*-------------------------------------------------------------------
    Local: DbgDumpDiskInfo()

    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DbgDumpDiskInfo(
    PDISK_INFO pDI)
{
    FFXPRINTF(1, ("DISK_INFO:          \n"));
    FFXPRINTF(1, ("  Total Secs:   %lX\n", pDI->di_total_sectors));
    FFXPRINTF(1, ("  Heads:        %lX\n", pDI->di_heads));
    FFXPRINTF(1, ("  Sec/Track:    %lX\n", pDI->di_sectors));
    FFXPRINTF(1, ("  Cylinders:    %lX\n", pDI->di_cylinders));
    FFXPRINTF(1, ("  Bytes/Sector: %lX\n", pDI->di_bytes_per_sect));
    FFXPRINTF(1, ("  Flags:        %lX\n", pDI->di_flags));

    return;
}


#if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT

/*-------------------------------------------------------------------
    Local: DiscardDeletedSectors()

    This routine is used by the DELETE_SECTORS ioctl.  When this
    is used, there is no need to use our FAT monitor.

    Parameters:
        hDisk      - The FlashFX Disk handle.
        ulStartSec - This is the starting sector to delete.  This
                     value is relative to the start of the FAT,
                     therefore the hidden sectors and the reserved
                     sectors must be added in when calculating the
                     VBF relative client address.
        ulCount    - The number of sectors to delete.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DiscardDeletedSectors(
    FFXDISKHANDLE   hDisk,
    D_UINT32        ulStartSec,
    D_UINT32        ulCount)
{
    FFXDISKINFO    *pDisk = *hDisk;

    FFXPRINTF(2, ("FfxCe:DiscardDeletedSectors start %lU num %lU\n", ulStartSec, ulCount));

    DclAssert(ulCount);
    DclAssert(!dd.fDelSectorsBroken);
    DclAssert((dd.OSVI.dwMajorVersion > 4) ||
              (dd.OSVI.dwMajorVersion == 4 && dd.OSVI.dwMinorVersion >= 10));

  #if QUEUE_PENDING_DISCARDS

    /*  We might have one CE Store which is split across two FlashFX
        Disks.  Adjust the starting sector and the Disk handle if
        necessary.  (In this configuration, only the second Disk will
        have an allocator on it which can do discards, however the
        pending discard information will be stored in the pDiskHook
        structure for the first Disk -- needs to be refactored!).
    */
    FfxCeAdjustIODiskAndOffset(hDisk, &ulStartSec);

    if(!pDisk->pDiskHook->ulPendingDiscardCount)
    {
        pDisk->pDiskHook->ulPendingDiscardStart = ulStartSec;
        pDisk->pDiskHook->ulPendingDiscardCount = ulCount;
        return;
    }

    if(pDisk->pDiskHook->ulPendingDiscardStart +
       pDisk->pDiskHook->ulPendingDiscardCount == ulStartSec)
    {
        pDisk->pDiskHook->ulPendingDiscardCount += ulCount;
        return;
    }

    /*  Discard the previously cached block
    */
    FfxCeFlushPendingDiscards(hDisk);

    /*  Start recording a new cached block
    */
    pDisk->pDiskHook->ulPendingDiscardStart = ulStartSec;
    pDisk->pDiskHook->ulPendingDiscardCount = ulCount;

  #else

    IssueDiscards(hDisk, ulStartSec, ulCount);

  #endif

    return;
}


/*-------------------------------------------------------------------
    Private: IssueDiscards()

    This routine discards any sectors that have been cached.

    Parameters:
        hDisk      - The FlashFX Disk handle.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void IssueDiscards(
    FFXDISKHANDLE   hDisk,
    D_UINT32        ulStartSec,
    D_UINT32        ulCount)
{
    FFXDISKINFO    *pDisk = *hDisk;

    DclAssert(hDisk);
    DclAssert(pDisk);
    DclAssert(pDisk->pDiskHook);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 2, 0),
        "FfxCe:IssueDiscards start %lU num %lU\n", ulStartSec, ulCount));

    DclAssert(!dd.fDelSectorsBroken);
    DclAssert((dd.OSVI.dwMajorVersion > 4) ||
              (dd.OSVI.dwMajorVersion == 4 && dd.OSVI.dwMinorVersion >= 10));

    /*  We might have one CE Store which is split across two FlashFX
        Disks.  However in this configuration we should only have an
        allocator (and hence discards) on the second Disk.
    */
    if(pDisk->hVBF)
    {
        /*  Assert that there is no second Disk.
        */
        DclAssert(!pDisk->pDiskHook->hNextDisk);

        FfxDriverSectorDiscard(hDisk, ulStartSec, ulCount);
    }
    else if(pDisk->pDiskHook->hNextDisk)
    {
        /*  If there is a second Disk, it must have an allocator
        */
        DclAssert((*pDisk->pDiskHook->hNextDisk)->hVBF);

        /*  Note that even when using a second Disk, the pending discard
            information is still stored in the pDiskHook structure for
            the first Disk.
        */
        FfxDriverSectorDiscard(pDisk->pDiskHook->hNextDisk, ulStartSec, ulCount);
    }

    return;
}


#if QUEUE_PENDING_DISCARDS

/*-------------------------------------------------------------------
    Private: FfxCeFlushPendingDiscards()

    This routine discards any sectors that have been cached.

    Parameters:
        hDisk      - The FlashFX Disk handle.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxCeFlushPendingDiscards(
    FFXDISKHANDLE   hDisk)
{
    FFXDISKINFO    *pDisk = *hDisk;

    DclAssert(hDisk);
    DclAssert(pDisk);
    DclAssert(pDisk->pDiskHook);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 2, 0),
        "FfxCeFlushPendingDiscards start %lU num %lU\n",
                  pDisk->pDiskHook->ulPendingDiscardStart,
                  pDisk->pDiskHook->ulPendingDiscardCount));

    DclAssert(!dd.fDelSectorsBroken);
    DclAssert((dd.OSVI.dwMajorVersion > 4) ||
              (dd.OSVI.dwMajorVersion == 4 && dd.OSVI.dwMinorVersion >= 10));


    if(!pDisk->pDiskHook->ulPendingDiscardCount)
        return;

    IssueDiscards(hDisk, pDisk->pDiskHook->ulPendingDiscardStart, pDisk->pDiskHook->ulPendingDiscardCount);

    pDisk->pDiskHook->ulPendingDiscardStart = 0;
    pDisk->pDiskHook->ulPendingDiscardCount = 0;

    return;
}

#endif
#endif


/*-------------------------------------------------------------------
    Local: FfxCeInitDiskParamsFromPhysInfo()

    Parameters:

    Return Value:
        Returns an FFXSTATUS code indicating the results.
-------------------------------------------------------------------*/
static FFXSTATUS FfxCeInitDiskParamsFromPhysInfo(
    FFXDISKINFO    *pDisk)
{
    FFXSTATUS       ffxStat;
    PDISK_INFO      pDSK;

    DclAssert(pDisk);
    DclAssert(pDisk->fpdi.ulBytesPerSector <= D_UINT16_MAX);

    if(!pDisk->fpdi.ulBytesPerSector || !pDisk->fpdi.ulTotalSectors)
    {
        FFXFMLINFO      FmlInfo;

        DclAssert(pDisk->hFML);

        ffxStat = FfxFmlDiskInfo(pDisk->hFML, &FmlInfo);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;

        pDisk->fpdi.ulBytesPerSector = FmlInfo.uPageSize;

        if(!FfxCalcPhysicalDiskParams(&pDisk->fpdi, (FmlInfo.ulBlockSize / FmlInfo.uPageSize) * FmlInfo.ulTotalBlocks))
            return FFXSTAT_BADDISKGEOMETRY;
    }

    DclAssert(pDisk->fpdi.ulBytesPerSector);
    DclAssert(pDisk->fpdi.ulTotalSectors);
    DclAssert(pDisk->fpdi.uHeads);
    DclAssert(pDisk->fpdi.uSectorsPerTrack);

    pDSK = &pDisk->pDiskHook->CeDiskInfo;

    pDSK->di_bytes_per_sect = pDisk->fpdi.ulBytesPerSector;
    pDSK->di_total_sectors  = pDisk->fpdi.ulTotalSectors;
    pDSK->di_heads          = pDisk->fpdi.uHeads;
    pDSK->di_sectors        = pDisk->fpdi.uSectorsPerTrack;
    pDSK->di_cylinders      = (pDSK->di_total_sectors / (pDSK->di_heads * pDSK->di_sectors));

    if(pDisk->pDiskHook->ulNextDiskSector)
    {
        FFXDISKGEOMETRY geo;

        DclAssert(pDisk->pDiskHook->hNextDisk);

        ffxStat = FfxDriverDiskGeometry(pDisk->pDiskHook->hNextDisk, &geo);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;

        DclAssert(geo.fAllocatorLoaded);

        DclPrintf("CE Store is split across two FlashFX Disks.\n");
        DclPrintf("Increasing Store size from %lU to %lU logical sectors.\n",
            pDSK->di_total_sectors, pDSK->di_total_sectors + geo.ulLogicalSectorCount);

        pDSK->di_total_sectors += geo.ulLogicalSectorCount;
        pDSK->di_heads          = 0;
        pDSK->di_sectors        = 0;
        pDSK->di_cylinders      = 0;

        pDSK->di_flags |= DISK_INFO_FLAG_CHS_UNCERTAIN;
    }

    DbgDumpDiskInfo(pDSK);

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Protected: FfxOsDeviceNameToDiskNumber()

    This function determines the Disk number given an OS specific
    device name.

    Parameters:
        pszDevName  - The null-terminated device name.

    Return Value:
        Returns the FlashFX Disk number if successful, or UINT_MAX if
        the name is invalid or the Disk does not exist.
-------------------------------------------------------------------*/
unsigned FfxOsDeviceNameToDiskNumber(
    const char *pszDevName)
{
    unsigned    nn;
    unsigned    nLen = DclStrLen(DRIVERPREFIX);
    long        lDevNum;

    DclAssert(pszDevName);

    if(DclStrNICmp(pszDevName, DRIVERPREFIX, nLen) != 0)
        return UINT_MAX;

    pszDevName += nLen;

    if(!DclIsDigit(*pszDevName))
        return UINT_MAX;

    lDevNum = DclAtoL(pszDevName);

    /*  Has to be in the form "DSK1:"
    */
    if((pszDevName[1] != ':') && (pszDevName[2] != 0))
        return UINT_MAX;

    for(nn=0; nn<FFX_MAX_DISKS; nn++)
    {
        FFXDISKHANDLE   hDisk;

        hDisk = FfxDriverDiskHandle(NULL, nn);
        if(hDisk)
        {
            FFXDISKINFO    *pDisk;

            pDisk = FfxDriverDiskPtr(hDisk);
            if(pDisk)
            {
                if((long)pDisk->pDiskHook->dwIndex == lDevNum)
                {
                    DclAssert(pDisk->pDiskHook->dwDiskNum == nn);
                    return nn;
                }
            }
        }
    }

    return UINT_MAX;
}




/*---------------------------------------------------------
    File pruned by DL-Prune v1.04

    Pruned/grafted 5/0 instances containing 59 lines.
    Modified 0 lines.
---------------------------------------------------------*/
