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

    This module contains code used during driver and device initalization.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ffxinit.c $
    Revision 1.14  2010/09/23 00:21:57Z  garyp
    Enhanced the startup and shutdown logic to provide more useful
    error information.
    Revision 1.13  2010/07/06 02:00:29Z  garyp
    Fixed FfxCeDriverInit() to handle the initialization count properly when
    already initialized.
    Revision 1.12  2010/01/07 23:51:43Z  garyp
    Modified to use the updated DriverAutoTest interface.
    Revision 1.11  2009/07/22 00:55:12Z  garyp
    Merged from the v4.0 branch.  Fixed to FfxCeDriverInit() to decrement the
    init counter properly.  Updated to initialize a DCL instance for the driver.
    Updated to use the revised FfxSignOn() function, which now takes an 
    fQuiet parameter.
    Revision 1.10  2008/03/27 03:23:29Z  Garyp
    Modified to use updated driver initialization functions.  Minor datatype
    changes.
    Revision 1.9  2007/11/03 23:50:10Z  Garyp
    Updated to use the standard module header.
    Revision 1.8  2007/09/28 22:48:47Z  jeremys
    Renamed a header file.
    Revision 1.7  2007/03/02 00:26:29Z  Garyp
    Removed CE 3 support.
    Revision 1.6  2006/10/16 20:32:12Z  Garyp
    Minor header changes.
    Revision 1.5  2006/10/13 02:20:56Z  Garyp
    Updated to use some renamed symbols.
    Revision 1.4  2006/02/21 02:02:34Z  Garyp
    Removed an obsolete header.
    Revision 1.3  2006/02/12 03:59:29Z  Garyp
    Continuing updates to the new Device and Disk model.
    Revision 1.2  2006/02/07 21:56:36Z  Garyp
    Updated to use the new DeviceNum/DiskNum concepts.
    Revision 1.1  2005/10/06 05:50:12Z  Pauli
    Initial revision
    Revision 1.2  2005/08/03 19:31:08Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/06/20 12:40:28Z  pauli
    Initial revision
    Revision 1.12  2005/04/07 01:05:44Z  GaryP
    Changed FfxCeDiskDestroy() to dismount the allocator, since the Init
    function was not the guy that loaded it in the first place.  Documentation
    cleanup.
    Revision 1.11  2004/12/30 17:33:25Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.10  2004/12/05 07:02:12Z  garys
    Accept the discard IOCTL under CE 4.1 with Reliance
    Revision 1.9  2004/11/20 04:36:07  GaryP
    Updated to use refactored header files.
    Revision 1.8  2004/08/15 04:16:40Z  GaryP
    Minor function renaming exersize
    Revision 1.7  2004/08/09 22:59:29Z  GaryP
    Eliminated an unnecessary header.
    Revision 1.6  2004/07/22 04:34:05Z  GaryP
    Fixed a syntax error in the previous rev.
    Revision 1.5  2004/07/17 05:27:39Z  GaryP
    Modified to use the updated FfxHookDiskCreate() function.
    Revision 1.4  2004/06/25 22:36:36Z  BillR
    New interface to multi-threaded FIMs.  New project hooks for
    initialization and configuration.
    Revision 1.3  2004/05/26 16:12:35Z  garyp
    Minor parameter changes to the Driver Framework.  Eliminated the use of
    a common sector buffer.
    Revision 1.2  2004/01/24 23:29:12Z  garys
    Revision 1.1.1.3  2004/01/24 23:29:12  garyp
    Updated to use the new FFXCONF_... style configuration settings.
    Revision 1.1.1.2  2004/01/03 01:14:46Z  garyp
    Include changes.
    Revision 1.1  2003/12/05 23:01:54Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <diskio.h>
#include <devload.h>

#include <flashfx.h>
#include <fxver.h>
#include <fxdriver.h>
#include <diskapi.h>
#include <fxfmlapi.h>
#include <fxapireq.h>
#include <fmlreq.h>
#include <oecommon.h>
#include <dlinstance.h>

#include <ffxwce.h>
#include "ffxdrv.h"

#if FFXCONF_FATSUPPORT
#include <dlfatapi.h>
#endif


/*-------------------------------------------------------------------
    Private: FfxCeDriverInit()

    This function provides the initial driver startup logic.

    Parameters:
        pDD - A pointer to the DRIVERDATA structure.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxCeDriverInit(
    DRIVERDATA         *pDD)
{
    FFXSTATUS           ffxStat;
    DCLINSTANCEHANDLE   hDclInst = NULL;

    /*  This routine is for one time initialization stuff.  If we are
        already initialized, simply return SUCCESS.

        This routine WILL get called multiple times as each disk is
        inited by CE, so we must maintain a running init count so that
        we know when it is appropriate to deinitialize the driver.
    */
    if(InterlockedIncrement(&pDD->ulInitCount) > 1)
        return FFXSTAT_SUCCESS;

    ffxStat = DclInstanceCreate(0, DCLFLAG_DRIVER, &hDclInst);
    if(ffxStat != DCLSTAT_SUCCESS)
        goto ErrorCleanup;

    DclAssert(pDD->pDI == NULL);

    if(!FfxProjMain())
    {
        FFXPRINTF(1, ("Driver load cancelled by FfxProjMain()\n"));
        ffxStat = DCLSTAT_PROJ_INITCANCELLED;
        goto ErrorCleanup;
    }

    FfxSignOn(FALSE);

  #if FFXCONF_DRIVERAUTOTEST
    FfxDriverUnitTestDCL(hDclInst);
  #endif

    /*  Validate the OS version we're running on
    */
    pDD->OSVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if(!GetVersionEx(&pDD->OSVI))
    {
        DclProductionError();
        ffxStat = FFXSTAT_FAILURE;
        goto ErrorCleanup;
    }

    DclPrintf("FFX: CE Version %lU.%lU Build %lU\n",
              pDD->OSVI.dwMajorVersion,
              pDD->OSVI.dwMinorVersion,
              pDD->OSVI.dwBuildNumber);

    if((pDD->OSVI.dwMajorVersion < 2) ||
       (pDD->OSVI.dwMajorVersion == 2 && pDD->OSVI.dwMinorVersion < 10))
    {
        DclPrintf("FlashFX requires Windows CE version 2.10 or later\n");
        ffxStat = DCLSTAT_UNSUPPORTEDVERSION;
        goto ErrorCleanup;
    }

    /*  The DISK_DELETE_SECTORS IOCTL may discard the wrong sectors
        in CE 4.0 due to the partition manager, so avoid it there,
        regardless of the file system.
    */
    if(pDD->OSVI.dwMajorVersion == 4 && pDD->OSVI.dwMinorVersion < 10)
    {
        pDD->fDelSectorsBroken = TRUE;
    }
    else if(pDD->OSVI.dwMajorVersion == 4 && pDD->OSVI.dwMinorVersion < 20)
    {
        /*  In CE 4.1, FATFS will stop sending the DISK_DELETE_SECTORS IOCTL
            after the entire disk is discarded once.  But discards work fine
            with Reliance.
        */
      #if FFXCONF_RELIANCESUPPORT
        pDD->fDelSectorsBroken = FALSE;
      #else
        pDD->fDelSectorsBroken = TRUE;
      #endif
    }
    else
    {
        pDD->fDelSectorsBroken = FALSE;
    }

  #if FFXCONF_FATMONITORSUPPORT
    {
        /*  Default to using FATMON if enabled
        */
        pDD->fUseFatMon = TRUE;

        /*  However, if running on CE 4.2 or later, don't use it
            as the IOCTL_DISK_DELETE_SECTORS ioctl will be used.
        */
        if((pDD->OSVI.dwMajorVersion > 4) ||
           (pDD->OSVI.dwMajorVersion == 4 && pDD->OSVI.dwMinorVersion >= 20))
        {
            pDD->fUseFatMon = FALSE;
        }
    }
  #else
    {
        pDD->fUseFatMon = FALSE;
    }
  #endif

    FFXPRINTF(1, ("FATMON is %s\n", pDD->fUseFatMon ? "enabled" : "disabled"));

    pDD->pDI = FfxDriverCreate(1, hDclInst);
    if(!pDD->pDI)
    {
        ffxStat = FFXSTAT_DRVINITFAILED;
        goto ErrorCleanup;
    }

    if(FfxDriverLock(pDD->pDI))
    {
        if(!FfxCeCreateDevices(pDD->pDI))
        {
            FFXPRINTF(1, ("Error creating devices\n"));

            FfxDriverUnlock(pDD->pDI);

            ffxStat = FFXSTAT_DEVINITFAILED;
            goto ErrorCleanup;
        }

        FfxDriverUnlock(pDD->pDI);
    }
    else
    {
        ffxStat = FFXSTAT_DRV_LOCKFAILED;
        goto ErrorCleanup;
    }

    return FFXSTAT_SUCCESS;

  ErrorCleanup:
    if(pDD->pDI)
    {
        FfxDriverDestroy(pDD->pDI);
        pDD->pDI = NULL;
    }

    if(hDclInst)
        DclInstanceDestroy(hDclInst);

    InterlockedDecrement(&pDD->ulInitCount);

    return ffxStat;
}


/*-------------------------------------------------------------------
    FfxCeDriverDeinit()

    Undo everything done by FfxCeDriverInit().

    Parameters:
        pDD - A pointer to the DRIVERDATA structure.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxCeDriverDeinit(
    PDRIVERDATA         pDD)
{
    FFXSTATUS           ffxStat;

    DclAssertWritePtr(pDD, sizeof(*pDD));

    if(InterlockedDecrement(&pDD->ulInitCount) > 0)
        return FFXSTAT_SUCCESS;

    ffxStat = FfxCeDestroyDevices(pDD->pDI);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        DCLINSTANCEHANDLE hDclInst = pDD->pDI->hDclInst;

        ffxStat = FfxDriverDestroy(pDD->pDI);
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            pDD->pDI = NULL;

            ffxStat = DclInstanceDestroy(hDclInst);
        }
    }

    FFXPRINTF(1, ("FfxCeDriverDeinit() returning %lX\n", ffxStat));
    
    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: FfxCeCreateDevices()

    Parameters:
        pFDI - A pointer to the FFXDRIVERINFO structure.

    Return Value:
        Returns TRUE if at least one device was created, otherwise
        FALSE.
-------------------------------------------------------------------*/
static D_BOOL FfxCeCreateDevices(
    FFXDRIVERINFO      *pFDI)
{
    unsigned            nDevNum;
    unsigned            nDevCount = 0;
    FFXDEVINITDATA      DevInit;

    for(nDevNum = 0; nDevNum < FFX_MAX_DEVICES; nDevNum++)
    {
#if FFX_MAX_DEVICES > 8
#error "FlashFX CE device initialization maxed out at 8"
#endif
        if(TRUE == FALSE) {}
      #if defined(FFX_DEV0_FIMS)
        else if(nDevNum == 0) {}
      #endif
      #if defined(FFX_DEV1_FIMS)
        else if(nDevNum == 1) {}
      #endif
      #if defined(FFX_DEV2_FIMS)
        else if(nDevNum == 2) {}
      #endif
      #if defined(FFX_DEV3_FIMS)
        else if(nDevNum == 3) {}
      #endif
      #if defined(FFX_DEV4_FIMS)
        else if(nDevNum == 4) {}
      #endif
      #if defined(FFX_DEV5_FIMS)
        else if(nDevNum == 5) {}
      #endif
      #if defined(FFX_DEV6_FIMS)
        else if(nDevNum == 6) {}
      #endif
      #if defined(FFX_DEV7_FIMS)
        else if(nDevNum == 7) {}
      #endif
        else
            continue;

        DclMemSet(&DevInit, 0, sizeof DevInit);

        DevInit.nDevNum = nDevNum;

        if(FfxDriverDeviceCreate(pFDI, &DevInit))
            nDevCount++;
    }

    return nDevCount > 0 ? TRUE : FALSE;
}


/*-------------------------------------------------------------------
    Local: FfxCeDestroyDevices()

    Parameters:
        pFDI - A pointer to the FFXDRIVERINFO structure.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS FfxCeDestroyDevices(
    FFXDRIVERINFO  *pFDI)
{
    unsigned        nDevNum;
    FFXSTATUS       ffxStat;

    for(nDevNum = FFX_MAX_DEVICES-1; nDevNum != UINT_MAX; nDevNum--)
    {
        if(pFDI->apDev[nDevNum])
        {
            ffxStat = FfxDriverDeviceDestroy(&pFDI->apDev[nDevNum]);
            if(ffxStat != FFXSTAT_SUCCESS)
            {
                DclPrintf("Error destroying DEV%u, Status=%lX\n", nDevNum, ffxStat);
                break;
            }
        }
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Private: FfxCeDiskCreate()

    This function creates a disk object.

    Parameters:
        pDD            - A pointer to the DRIVERDATA structure to use
        ptzActiveKey   - A pointer to the active key to use

    Return Value:
        A pointer to the FFXDISKINFO structure if successful, else NULL
-------------------------------------------------------------------*/
FFXDISKHANDLE FfxCeDiskCreate(
    PDRIVERDATA     pDD,
    LPCTSTR         ptzActiveKey)
{
    FFXDISKINITDATA fdd;
    FFXDISKHANDLE   hDsk;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEINDENT),
        "FfxCeDiskCreate() ActiveKey='%W'\n", ptzActiveKey));

    DclMemSet(&fdd, 0, sizeof fdd);

    fdd.lptzActiveKey = ptzActiveKey;

    hDsk = FfxDriverDiskCreate(pDD->pDI, &fdd);
    if(!hDsk)
    {
        FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEUNDENT),
            "FfxCeDiskCreate() failed\n"));

        return NULL;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEUNDENT),
        "FfxCeDiskCreate() DISK%U returning hDsk=%P\n", (*hDsk)->Conf.nDiskNum, hDsk));

    return hDsk;
}


/*-------------------------------------------------------------------
    Private: FfxCeDiskDestroy()

    Deinitialize a device that was set up with FfxCeDiskCreate().

    Parameters:
        pDev    - A pointer to the PFFXDEVINFO structure to use

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxCeDiskDestroy(
    FFXDISKHANDLE   hDsk)
{
    FFXSTATUS       ffxStat;

    ffxStat = FfxDriverDiskDestroy(hDsk);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("Error destroying DISK%u, Status=%lX\n", (*hDsk)->Conf.nDiskNum, ffxStat);
    }

    return ffxStat;
}

