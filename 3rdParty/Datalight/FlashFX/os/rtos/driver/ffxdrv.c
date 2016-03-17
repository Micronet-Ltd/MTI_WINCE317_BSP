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

    FlashFX device driver for generic RTOS environments.

    This device driver is supplied as an example and is designed to work
    with the Datalight Reliance RTOS kit.  Depending on the device driver
    architecture for the RTOS in question, this driver may require major
    modifications or only minor changes.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ffxdrv.c $
    Revision 1.24  2011/02/10 02:17:48Z  garyp
    Eliminated the use of unsigned long and short data types.  Now use only
    our standard abstracted types, as well as plain unsigned, to avoid
    compatibility issues with some toolchains and platforms.
    Revision 1.23  2010/01/09 18:36:08Z  garyp
    Corrected a function declaration to match the prototype -- discovered since
    the previous rev included the proper header.  Documentation updated.
    Revision 1.22  2010/01/07 23:53:19Z  garyp
    Modified to use the updated DriverAutoTest interface.  Eliminated support
    for an obsolete compaction IOCTL.
    Revision 1.21  2009/08/03 23:30:15Z  garyp
    Merged from the v4.0 branch.  Modified the shutdown processes to take a mode
    parameter.  Updated the compaction functions to take a compaction level. 
    Updated to work with functions which now take a DCL instance handle.  
    Modified to support redirecting sector I/O requests to the Driver Framework
    abstraction for the FML if allocator support is disabled.  Updated to use 
    the revised FfxSignOn() function, which now takes an fQuiet parameter.
    Revision 1.20  2009/05/07 15:39:12Z  keithg
    Updated function declarations to use the Reliance expected types.
    Revision 1.19  2009/02/18 21:40:58Z  glenns
    Added code to prevent compiler warnings about unused formal parameters.
    Revision 1.18  2008/03/27 21:43:40Z  Garyp
    Modified to use updated driver initialization functions.  Minor datatype
    updates.  Updated to work when no allocator is being used.  Added the
    RELIANCE_MBR_HACK (disabled by default) to allow Reliance to work
    properly when an MBR is being used.
    Revision 1.17  2008/01/13 07:29:10Z  keithg
    Function header updates to support autodoc.
    Revision 1.16  2007/12/26 02:03:57Z  Garyp
    Function renamed for clarity.
    Revision 1.15  2007/12/12 03:55:29Z  Garyp
    Updated to support the REL_IOCTL_GENERICBLOCK ioctl function.
    Revision 1.14  2007/11/03 23:50:01Z  Garyp
    Updated to use the standard module header.
    Revision 1.13  2007/10/30 22:52:07Z  pauli
    Removed #error if Reliance support is not enabled to allow the driver
    to be used without Reliance.
    Revision 1.12  2006/10/13 02:06:37Z  Garyp
    Modified to use new allocator create/destroy functions.
    Revision 1.11  2006/08/02 18:46:58Z  Pauli
    Removed some bogus code that was trying to pass the disk number
    to the FfxDriverUnitTest function in the drive forms parameter.
    Revision 1.10  2006/07/07 22:45:50Z  Garyp
    Updated to use the new FfxDriverVbfCreate() function.
    Revision 1.9  2006/03/23 04:17:42Z  Pauli
    Removed partition offsets.  The FFX partition information is ignored.
    This is necessary because the emulated partition information that FFX
    provides is not populated until it recognizes the file system.  This
    prevented Reliance from formatting and mounting an unformatted disk.
    Revision 1.8  2006/03/16 17:49:23Z  Pauli
    Updated comments and debug output.
    Changed IO requests to offset the starting sector based on the partition info.
    Revision 1.7  2006/03/11 05:03:50Z  Pauli
    Renamed and moved a local function.
    Revision 1.6  2006/03/02 19:38:37Z  Pauli
    The Reliance rtos header is now expected to be in the include paths.
    Revision 1.5  2006/02/14 18:29:30Z  Pauli
    Updated comments.
    Revision 1.4  2006/02/14 01:28:54Z  Pauli
    Updated to fully support the new device/disk architecture.
    Revision 1.3  2006/02/12 01:01:06Z  Garyp
    Continuing updates to the new Device and Disk model.
    Revision 1.2  2006/02/09 18:55:40Z  Pauli
    Major rewrite.  This now implements the block device driver interface
    used by the Reliance RTOS kit.
    Revision 1.1  2005/10/06 05:50:10Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <flashfx_rtos.h>
#include <fxdriver.h>
#include <vbf.h>
#include <ffxdrv.h>
#include <fxapireq.h>
#include <fxfmlapi.h>
#include <oecommon.h>


/*  Conditionally include the Reliance header.
*/
#if FFXCONF_RELIANCESUPPORT

#include <reliance_rtos.h>

/*  Set this to the non-zero number of sectors to offset I/O requests
    if the logical disk as the file system sees it does not start at
    physical sector 0.  This is necessary for Reliance if an MBR is
    on the system because Reliance does not know how to handle MBRs,
    but rather expects that some OS layer is providing only a logical
    view of sectors.  There is no such layer in the RTOS kit.

    ToDo: This MBR hack may now be unnecessary now that the Reliance
          RTOS port has been updated to allow MBRs to be recognized.
*/
#define RELIANCE_MBR_HACK   (0)     /*  Must be 0 for checkin */

#else

#define RELIANCE_MBR_HACK   (0)     /*  Must be 0 for checkin */
#endif



/*-------------------------------------------------------------------
-------------------                            ----------------------
-------------------  Global Settings and Data  ----------------------
-------------------                            ----------------------
-------------------------------------------------------------------*/

FFXDRIVERINFO          *pDI;


/*-------------------------------------------------------------------
-------------------                             ---------------------
-------------------  Private Settings and Data  ---------------------
-------------------                             ---------------------
-------------------------------------------------------------------*/

   
/*-------------------------------------------------------------------
-------------------                               -------------------
-------------------      Function Prototypes      -------------------
-------------------                               -------------------
-------------------------------------------------------------------*/

static D_BOOL   CreateDisk(unsigned nDiskNum);
static D_BOOL   CreateAllDevices(FFXDRIVERINFO *pFDI);


/*-------------------------------------------------------------------
-------------------                           -----------------------
-------------------  Public Driver Functions  -----------------------
-------------------                           -----------------------
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
    Public: FlashFXDeviceOpen()

    Initialize the FlashFX device driver, create any and all FlashFX
    Devices, and create the specfied Disk.

    The first time this function is called, the FlashFX device
    driver is initialized.  This is a one-time initialization.

    Parameters:
        nDiskNum     - The logical Disk number to open.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FlashFXDeviceOpen(
    unsigned        nDiskNum)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEINDENT),
        "FlashFXDeviceOpen(%u)\n", nDiskNum));

    /*  Don't do this if we are already init'd
    */
    if(!pDI)
    {
        if(!FfxProjMain())
        {
            FFXPRINTF(1, ("Driver load cancelled by FfxProjMain\n"));
            ffxStat = FFXSTAT_DRVLOADCANCELLED;
            goto EXIT;
        }

        FfxSignOn(FALSE);

        pDI = FfxDriverCreate(1, NULL);
        if(!pDI)
        {
            FFXPRINTF(1, ("Driver initialization failed\n"));
            ffxStat = FFXSTAT_DRVINITFAILED;
            goto EXIT;
        }

      #if FFXCONF_DRIVERAUTOTEST
        FfxDriverUnitTestDCL(NULL);
      #endif

        FfxDriverLock(pDI);

        if(!CreateAllDevices(pDI))
        {
            FFXPRINTF(1, ("Error creating devices\n"));

            FfxDriverUnlock(pDI);

            FfxDriverDestroy(pDI);

            pDI = NULL;

            ffxStat = FFXSTAT_DEVINITFAILED;
            goto EXIT;
        }

        FfxDriverUnlock(pDI);

        FFXPRINTF(1, ("Driver initialization complete\n"));
    }

    if(!CreateDisk(nDiskNum))
    {
        ffxStat = FFXSTAT_DEVINITFAILED;
        goto EXIT;
    }

  EXIT:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEUNDENT),
        "FlashFXDeviceOpen() returning status %lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FlashFXDeviceClose()

    Destroy the specified FlashFX Disk.  This function does not 
    destroy any FlashFX Devices, nor does it destroy the FlashFX
    driver instance.

    Parameters:
        nDiskNum    - The logical Disk number to close.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FlashFXDeviceClose(
    unsigned        nDiskNum)
{
    FFXDISKHANDLE   hDisk;
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEINDENT),
        "FlashFXDeviceClose(%u)\n", nDiskNum));

    hDisk = FfxDriverDiskHandle(pDI, nDiskNum);
    if(!hDisk)
    {
        ffxStat = FFXSTAT_DISKNUMBERINVALID;
        goto EXIT;
    }

    FfxDriverLock(pDI);

  #if FFXCONF_ALLOCATORSUPPORT
    FfxDriverAllocatorDestroy(hDisk, FFX_SHUTDOWNFLAGS_NORMAL);
  #endif

    FfxDriverDiskDestroy(hDisk);

    FfxDriverUnlock(pDI);

  EXIT:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEUNDENT),
        "FlashFXDeviceClose() returning status %lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FlashFXDeviceIO()

    This function reads or writes data from and to the FlashFX Disk
    based on the 'fReading' parameter.

    Parameters:
        nDiskNum        - The logical disk number to operate on
        ulSectorNum     - The starting sector number to read or write
        pClientBuffer   - Pointer to the data buffer
        uSectorCount    - Number of sectors to be read or written
        fReading        - Indicates whether we are reading or writing

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FlashFXDeviceIO(
    unsigned        nDiskNum,
    D_UINT32        ulSectorNum,
    void           *pClientBuffer,
    D_UINT16        uSectorCount,
    D_BOOL          fReading)
{
    D_UINT32        ulTotal = 0;
    FFXDISKHANDLE   hDisk;
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEINDENT),
        "FlashFXDeviceIO(%u, %lU, %U, %d)\n", nDiskNum, ulSectorNum, uSectorCount, fReading));

    DclAssert(uSectorCount);

    hDisk = FfxDriverDiskHandle(pDI, nDiskNum);
    if(!hDisk)
    {
        ffxStat = FFXSTAT_DISKNUMBERINVALID;
        goto EXIT;
    }

    DclAssert((*hDisk)->fpdi.ulTotalSectors);

    /*  Make sure request doesn't exceed the disk size
    */
    if(ulSectorNum + uSectorCount + RELIANCE_MBR_HACK > (*hDisk)->fpdi.ulTotalSectors)
    {
        FFXPRINTF(1, ("Error: IO request exceeds disk size\n"));

        ffxStat = FFXSTAT_SECTOROUTOFRANGE;
        goto EXIT;
    }

  #if FFXCONF_ALLOCATORSUPPORT
    {
    if(fReading)
        ulTotal = FfxDriverSectorRead(hDisk, ulSectorNum + RELIANCE_MBR_HACK, uSectorCount, pClientBuffer);
    else
        ulTotal = FfxDriverSectorWrite(hDisk, ulSectorNum + RELIANCE_MBR_HACK, uSectorCount, pClientBuffer);
    }
  #else
    {
        FFXIOSTATUS ioStat;

        if(fReading)
            ioStat = FfxDriverFmlSectorRead(hDisk, ulSectorNum + RELIANCE_MBR_HACK, uSectorCount, pClientBuffer);
        else
            ioStat = FfxDriverFmlSectorWrite(hDisk, ulSectorNum + RELIANCE_MBR_HACK, uSectorCount, pClientBuffer, NULL);

        ulTotal = ioStat.ulCount;
    }
  #endif

    if(ulTotal != uSectorCount)
    {
        ffxStat = FFXSTAT_FAILURE;
        goto EXIT;
    }

  EXIT:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEUNDENT),
        "FlashFXDeviceIO() returning status %lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FlashFXDeviceIoctl()

    This function performs the specified ioctl function on the
    given device.

    Parameters:
        nDiskNum    - The logical disk number to operate on
        uCommand    - The ioctl command
        pBuffer     - A pointer to the data buffer for the command.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FlashFXDeviceIoctl(
    unsigned        nDiskNum,
    D_UINT16        uCommand,
    void           *pBuffer)
{
    FFXDISKHANDLE   hDisk;
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEINDENT),
        "FlashFXDeviceIoctl(%u, %U)\n", nDiskNum, uCommand));

    /*  Prevent compiler warning if FFXCONF_RELIANCESUPPORT is FALSE 
    */
    (void)pBuffer;

    hDisk = FfxDriverDiskHandle(pDI, nDiskNum);
    if(!hDisk)
    {
        ffxStat = FFXSTAT_DISKNUMBERINVALID;
        goto EXIT;
    }

    switch(uCommand)
    {
      #if FFXCONF_RELIANCESUPPORT

        case REL_IOCTL_DISCARD:
        {
            DISCARD_PARAMS *pDP = (DISCARD_PARAMS *)pBuffer;

            if(pDP)
            {
                ffxStat = FfxDriverSectorDiscard(hDisk, pDP->ulStartSector + RELIANCE_MBR_HACK, pDP->ulNumSectors);
            }
            else
            {
                ffxStat = FFXSTAT_BADPARAMETER;
            }
            break;
        }

        case REL_IOCTL_DISCARD_SUPPORT:
        {
            ffxStat = FFXSTAT_SUCCESS;
            break;
        }

        case REL_IOCTL_FLUSH:
        {
            ffxStat = FFXSTAT_SUCCESS;
            break;
        }

        case REL_IOCTL_PARAMS:
        {
            DISK_PARAMS    *pDP = (DISK_PARAMS *)pBuffer;

            if(pDP)
            {
                pDP->ulSectorSize = (*hDisk)->fpdi.ulBytesPerSector;
                pDP->ulSectorCount = (*hDisk)->fpdi.ulTotalSectors - RELIANCE_MBR_HACK;
                ffxStat = FFXSTAT_SUCCESS;
            }
            else
            {
                ffxStat = FFXSTAT_BADPARAMETER;
            }
            break;
        }

        case REL_IOCTL_GENERICBLOCK:
        {
            ffxStat = FfxDriverIoctlMasterDispatch(pBuffer);
            break;
        }

      #endif

        default:
        {
            ffxStat = FFXSTAT_BADPARAMETER;
        }
    }

  EXIT:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEUNDENT),
        "FlashFXDeviceIoctl() returning status %lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
-------------------                            ----------------------
-------------------  Private Driver Functions  ----------------------
-------------------                            ----------------------
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
    Local: CreateAllDevices()

    Create all the FlashFX Devices upon which the logical Disks
    will be mounted.  The Devices are configured in the project
    configuration file (ffxconf.h).

    Parameters:
        pFDI - A pointer to the FFXDRIVERINFO structure.

    Return Value:
        Returns TRUE if at least one device was created, otherwise
        FALSE.
-------------------------------------------------------------------*/
static D_BOOL CreateAllDevices(
    FFXDRIVERINFO      *pFDI)
{
    unsigned            nDevNum;
    unsigned            nDevCount = 0;
    FFXDEVINITDATA      DevInit;

    for(nDevNum = 0; nDevNum < FFX_MAX_DEVICES; nDevNum++)
    {
      #if FFX_MAX_DEVICES > 8
        #error "FlashFX RTOS Device initialization maxed out at 8"
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
    Local: CreateDisk()

    This function initializes a logical disk.  Disks are
    configured in the project configuration file (ffxconf.h).

    Parameters:
       nDiskNum  - Disk number to create.

    Return Value:
       Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL CreateDisk(
    unsigned            nDiskNum)
{
    FFXDISKHANDLE       hDisk = NULL;
    FFXDISKINITDATA     sDiskInit;
        
    FFXPRINTF(1, ("FlashFXDiskCreate(%u)\n", nDiskNum));

    if(!FfxDriverLock(pDI))
    {
        DclProductionError();
        return FALSE;
    }

    sDiskInit.nDiskNum = nDiskNum;
    hDisk = FfxDriverDiskCreate(pDI, &sDiskInit);
    if(!hDisk)
    {
        goto DiskCreateFailure;
    }

  #if FFXCONF_DRIVERAUTOTEST
    FfxDriverUnitTest(NULL, hDisk);
  #endif

  #if FFXCONF_ALLOCATORSUPPORT
    {
        FFXSTATUS           ffxStat;

        /*-----------------------------------------------------------
            Mount the allocator
        -----------------------------------------------------------*/
        ffxStat = FfxDriverAllocatorCreate(hDisk);
        if(ffxStat != FFXSTAT_SUCCESS)
        {
            DclPrintf("VBF creation failed, status=%lX\n", ffxStat);
            DclPrintf("The disk may need to be formatted\n");

            goto DiskCreateFailure;
        }
    }
  #else
    if(!(*hDisk)->fpdi.ulBytesPerSector || !(*hDisk)->fpdi.ulTotalSectors)
    {
        FFXSTATUS       ffxStat;
        FFXFMLINFO      FmlInfo;

        DclAssert((*hDisk)->hFML);

        ffxStat = FfxFmlDiskInfo((*hDisk)->hFML, &FmlInfo);
        if(ffxStat != FFXSTAT_SUCCESS)
            goto DiskCreateFailure;

        (*hDisk)->fpdi.ulBytesPerSector = FmlInfo.uPageSize;

        if(!FfxCalcPhysicalDiskParams(&(*hDisk)->fpdi, (FmlInfo.ulBlockSize / FmlInfo.uPageSize) * FmlInfo.ulTotalBlocks))
            goto DiskCreateFailure;
    }
  #endif

    FfxDriverUnlock(pDI);

    FFXPRINTF(1, ("CreateDisk() Successful: returning %d\n", TRUE));

    return TRUE;

  DiskCreateFailure:

    if(hDisk)
        FfxDriverDiskDestroy(hDisk);

    FfxDriverUnlock(pDI);

    FFXPRINTF(1, ("CreateDisk() Failed: returning %d\n", FALSE));

    return FALSE;
}

