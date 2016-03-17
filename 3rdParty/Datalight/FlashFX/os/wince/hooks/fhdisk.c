/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module contains the Windows CE Hooks Layer default implementations
    for:

       FfxHookDiskCreate()
       FfxHookDiskDestroy()

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    ffxproj.mak to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhdisk.c $
    Revision 1.16  2011/02/09 00:53:03Z  garyp
    Modified to use some refactored disk partition manipulation code which
    has moved from FlashFX into DCL.  Not functionally changed otherwise.
    Revision 1.15  2010/01/23 22:39:36Z  garyp
    Updated to support auto-assigned Disk numbers.  Now get the default
    file system from the registry, rather than the ffxconf.h file.
    Revision 1.14  2009/11/25 23:33:42Z  garyp
    Added conditional test code to test auto-lock/unlock at init time.
    Revision 1.13  2009/07/22 01:09:52Z  garyp
    Merged from the v4.0 branch.  Added general logic to support automatically
    calculating the Disk offset and length.  Added WinMobile specific support
    for dynamically calculating the DISK0 parameters from the MBR and FLS, as
    well as recording compaction block information.  Updated to process the
    DiskFlags value.
    Revision 1.12  2009/04/08 20:26:23Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.11  2009/02/18 06:13:01Z  garyp
    Updated to deal with a renamed structure member.
    Revision 1.10  2008/03/23 04:18:39Z  Garyp
    Updated to the new-and-improved method of DISK creation, where the hook
    function fills in a FFXDISKCONFIG structure, rather than touching the
    FFXDISKINFO structure directly.  Any configuration options not supplied
    will be automatically filled in with the default values.
    Revision 1.9  2007/12/03 02:48:43Z  Garyp
    Corrected some error handling problems in FfxHookDiskCreate().
    Revision 1.8  2007/11/03 23:50:11Z  Garyp
    Updated to use the standard module header.
    Revision 1.7  2007/07/12 22:35:02Z  timothyj
    Updated to use the new FfxDriverDiskAdjust() helper.
    Revision 1.6  2007/01/23 23:13:53Z  Garyp
    Corrected some faultty but benign debug code.
    Revision 1.5  2006/10/13 01:39:39Z  Garyp
    Updated to initialize the disk format settings.
    Revision 1.4  2006/10/04 01:07:29Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.3  2006/02/15 02:59:10Z  Garyp
    Split out the compaction settings.
    Revision 1.2  2006/02/15 00:07:48Z  Garyp
    Updated comments.
    Revision 1.1  2006/02/10 02:15:32Z  Garyp
    Initial revision
    Revision 1.7  2005/04/03 01:29:36Z  GaryP
    Added debugging code.
    Revision 1.6  2005/03/29 06:05:52Z  GaryP
    Modified to skip calling FfxCeMapFlash() if the length is valid, but the
    start address is BAD_FLASH_ADDRESS.  Presumably the flash is NAND and
    we don't need to map anything.
    Revision 1.5  2004/12/30 17:33:26Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.4  2004/11/20 04:36:04Z  GaryP
    Updated to use refactored header files.
    Revision 1.3  2004/08/24 06:05:40Z  GaryP
    Modified to use common helper functions in cereg.c and cemapmem.c.
    Revision 1.2  2004/07/22 04:00:33Z  GaryP
    Updated to use the newly revamped FfxHookDiskCreate() function.
    Revision 1.1  2004/06/25 20:47:50Z  BillR
    Initial revision
    ---------------------
    Bill Roman 2004-06-18
---------------------------------------------------------------------------*/

#include <windows.h>
#include <diskio.h>

#if _WIN32_WCE >= 502
#include <fls.h>
#include <bootpart.h>
#endif

#include <flashfx.h>
#include <oecommon.h>
#include <fxdriver.h>
#include <ffxwce.h>
#include <fxfmlapi.h>
#include <dlmbr.h>




/*-------------------------------------------------------------------
    Public: FfxHookDiskCreate()

    Hook the creation of a Disk, allowing project specific, run-time
    customization of the Disk configuration parameters.

    Upon successful completion, this function must fill in the
    pConf->nDiskNum field with the appropriate DISKn number.

    Other configuration parameters may be filled in as well.  Any
    parameters which are not set by this function will use default
    values.

    Parameters:
        pDI       - A pointer to the FFXDRIVERINFO structure to use
        pDiskData - A pointer to the FFXDISKINITDATA structure to use
        pConf     - A pointer to the FFXDISKCONFIG structure to use

    Return Value:
        If successful, returns an opaque pointer/handle for the
        newly created FFXDISKHOOK object, or NULL otherwise.
-------------------------------------------------------------------*/
FFXDISKHOOK *FfxHookDiskCreate(
    const FFXDRIVERINFO    *pDI,
    const FFXDISKINITDATA  *pDiskData,
    FFXDISKCONFIG          *pConf)
{
    FFXDISKHOOK            *pHook = NULL;
    LPCTSTR                 lptzActiveKey;
    FFXSTATUS               ffxStat = FFXSTAT_SUCCESS;

    /*  Not currently used.
    */
    (void)pDI;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxHookDiskCreate() pDrv=%P pDiskInit=%P pConf=%P\n", pDI, pDiskData, pConf));

    DclAssert(pDI);
    DclAssert(pDiskData);
    DclAssert(pConf);

    lptzActiveKey = pDiskData->lptzActiveKey;

    pHook = DclMemAllocZero(sizeof *pHook);
    if(!pHook)
    {
        ffxStat = DCLSTAT_MEMALLOCFAILED;
        goto Cleanup;
    }

    /*  Remember that for CE, we don't even know which Disk we are
        initializing until after we read these registry settings.
    */

    if(!FfxCeGetRegistrySettings(lptzActiveKey, pHook))
    {
        FFXPRINTF(1, ("Error reading registry settings for \"%W\"\n", lptzActiveKey));
        goto Cleanup;
    }

    if(pHook->dwDiskNum == D_UINT32_MAX)
    {
        unsigned    nn;
        
        for(nn=0; nn<FFX_MAX_DISKS; nn++)
        {
            if(!FfxFmlHandle(nn))
                break;
        }

        if(nn == FFX_MAX_DISKS)
        {
            FFXPRINTF(1, ("Unable to auto-determine the Disk number -- all FFX_MAX_DISKS(%u) are used\n", FFX_MAX_DISKS));
            ffxStat = FFXSTAT_DISK_OUTOFSLOTS;
            goto Cleanup;
        }

        pHook->dwDiskNum = (DWORD)nn;
    }

    if(pHook->dwDiskNum >= FFX_MAX_DISKS)
    {
        FFXPRINTF(1, ("DISK%lU is out of range (max=%u)\n", pHook->dwDiskNum, FFX_MAX_DISKS-1));
        ffxStat = FFXSTAT_DISKNUMBERINVALID;
        goto Cleanup;
    }

    if(pHook->dwDeviceNum >= FFX_MAX_DEVICES)
    {
        FFXPRINTF(1, ("DEV%lU is out of range (max=%u)\n", pHook->dwDeviceNum, FFX_MAX_DEVICES-1));
        ffxStat = FFXSTAT_DEVICENUMBERINVALID;
        goto Cleanup;
    }

    if(pHook->dwDiskFlags & ~FFX_DISK_VALIDMASK)
    {
        FFXPRINTF(1, ("DiskFlags value of %lX is not valid\n", pHook->dwDiskFlags));
        ffxStat = FFXSTAT_DISK_FLAGSINVALID;
        goto Cleanup;
    }


    /*  For the Disk offset is FFX_NEXT, start it immediately following the
        numerically previous Disk.  If this is DISK0, or if there if the
        numerically previous Disk does not exist, start at offset 0.
    */
    if(pHook->dwFlashOffsetKB == FFX_NEXT)
    {
        FFXFMLHANDLE    hFML;

        if( pHook->dwDiskNum &&
            ((hFML = FfxFmlHandle((unsigned)pHook->dwDiskNum-1)) != NULL))
        {
            FFXFMLINFO  fi;

            ffxStat = FfxFmlDiskInfo(hFML, &fi);
            if(ffxStat != FFXSTAT_SUCCESS)
                goto Cleanup;

            if(fi.nDeviceNum == pHook->dwDeviceNum)
            {
                /*  If we found the numerically previous Disk, and it was on
                    the same Device, then just start the new Disk immediately
                    following.
                */
                pHook->dwFlashOffsetKB = (fi.ulBlockSize / 1024) * (fi.ulStartBlock + fi.ulTotalBlocks);
            }
            else
            {
                /*  However if the numerically previous Disk was on a
                    different Device, then just start this Disk at the
                    start of the Device.

                    Assumes a sequential initialization of Disks!
                */
                pHook->dwFlashOffsetKB = 0;
            }
        }
        else
        {
            /*  Either we are the first Disk, or the numerically previous
                Disk is not initialized -- in either case, start at offset
                zero in the Device.
            */
            pHook->dwFlashOffsetKB = 0;
        }
    }

    FFXPRINTF(1, ("DISK%lU OffsetKB=%lX LengthKB=%lX\n",
        pHook->dwDiskNum, pHook->dwFlashOffsetKB, pHook->dwFlashLengthKB));

    pConf->nDiskNum                 = (unsigned)pHook->dwDiskNum;
    pConf->DiskSettings.nDevNum     = (unsigned)pHook->dwDeviceNum;
    pConf->DiskSettings.nFlags      = (unsigned)pHook->dwDiskFlags;
    pConf->DiskSettings.ulOffsetKB  = pHook->dwFlashOffsetKB;
    pConf->DiskSettings.ulLengthKB  = pHook->dwFlashLengthKB;
    pConf->fInitedDiskSettings      = TRUE;

    pHook->nDiskNum = pConf->nDiskNum;

  #if FFXCONF_FORMATSUPPORT
    pConf->FormatSettings.uFormatState = (D_UINT16)pHook->nFormatState;
    pConf->FormatSettings.uFileSystem = (D_UINT16)pHook->nDefaultFS;

    if(pHook->nDefaultFS == DCL_FILESYS_UNKNOWN)
        pConf->FormatSettings.fUseMBR = FALSE;
    else
        pConf->FormatSettings.fUseMBR = TRUE;

    pConf->fInitedFormatSettings = TRUE;
    
    ffxStat = FFXSTAT_SUCCESS;
  #endif

  Cleanup:

    if(ffxStat != FFXSTAT_SUCCESS && pHook)
    {

        DclMemFree(pHook);

        pHook = NULL;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEUNDENT),
        "FfxHookDiskCreate() returning pHook=%P, Status=%lX\n", pHook, ffxStat));

    return pHook;
}


/*-------------------------------------------------------------------
    Public: FfxHookDiskDestroy()

    Hook the destruction of a Disk.  This function should release
    any resources which were allocated by FfxHookDiskCreate().

    Parameters:
        pDI     - A pointer to the FFXDRIVERINFO structure to use
        pHook   - The opaque pointer/handle for the FFXDISKHOOK
                  object to destroy

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookDiskDestroy(
    const FFXDRIVERINFO    *pDI,
    FFXDISKHOOK            *pHook)
{
    DclAssert(pDI);
    DclAssert(pHook);

    (void)pDI;


    DclMemFree(pHook);

    return;
}





/*---------------------------------------------------------
    File pruned by DL-Prune v1.04

    Pruned/grafted 4/0 instances containing 315 lines.
    Modified 0 lines.
---------------------------------------------------------*/
