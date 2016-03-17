/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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
    $Log: ffxdrv.c $
    Revision 1.9  2010/01/07 23:51:42Z  garyp
    Modified to use the updated DriverAutoTest interface.
    Revision 1.8  2009/07/29 17:53:48Z  garyp
    Merged from the v4.0 branch.  Updated to use new calling conventions
    for several functions.
    Revision 1.7  2009/05/21 17:40:33Z  garyp
    Updated documentation.  Updated to use standard type names.  Cleaned
    up some code formatting.  No functional changes.
    Revision 1.6  2009/04/06 18:01:10Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.5  2008/03/25 00:03:57Z  Garyp
    Updated to use some slightly modified driver initialization functions.
    Revision 1.4  2008/03/23 04:52:29Z  Garyp
    Minor data type changes.
    Revision 1.3  2007/11/03 23:49:54Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/10/10 05:27:52Z  brandont
    Changed define for driver support for multiple instances to
    DRIVER_SUPPORT_MULTIPLE_INSTANCES.
    Revision 1.1  2007/10/10 05:18:08Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <vbf.h>
#include <fxfmlapi.h>
#include <fxdriver.h>
#include <dlreaderio.h>
#include <ffxdrv.h>

#define DRIVER_SUPPORT_MULTIPLE_INSTANCES FALSE

typedef struct
{
    FFXDRIVERINFO      *pDI;
    FFXDEVHANDLE        hDev;
    FFXDISKHANDLE       hDisk;
    unsigned short      uPartNum;
    D_BOOL              fDriverLocked;
    D_BOOL              fAllocatorMounted;
} READERIOINSTANCE;


#if DRIVER_SUPPORT_MULTIPLE_INSTANCES
    FFXDRIVERINFO  *gpDI = NULL;
    unsigned        guOpenReaderIOs = 0;
#endif


/*-------------------------------------------------------------------
    Local: ReaderIoGetParameters()

    Return the sector size and total sectors of this block device

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS ReaderIoGetParameters(
    DCLREADERIO            *pReaderDevice,
    D_UINT32               *pulBlockSize,
    D_UINT32               *pulTotalBlocks)
{
    DCLSTATUS               dclStat;
    READERIOINSTANCE       *pRIO;
    VBFDISKINFO             di;

    DclAssert(pReaderDevice);
    DclAssert(pReaderDevice->pDeviceSpecificInfo);
    DclAssert(pulBlockSize);
    DclAssert(pulTotalBlocks);

    pRIO = pReaderDevice->pDeviceSpecificInfo;

    dclStat = FfxVbfDiskInfo((*pRIO->hDisk)->hVBF, &di);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("ReaderIoGetParameters() FfxVbfDiskInfo() failed with status %lX\n", dclStat);
    }
    else
    {
        *pulBlockSize = di.uPageSize;
        *pulTotalBlocks = di.ulTotalPages;
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: ReaderIoRead()

    Read from the block device

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS ReaderIoRead(
    DCLREADERIO            *pReaderDevice,
    D_UINT32                ulBlockStart,
    D_UINT32                ulBlockLength,
    void                   *pBuffer)
{
    DCLSTATUS               dclStat;
    FFXIOSTATUS             ioStat;
    READERIOINSTANCE       *pRIO;

    DclAssert(pReaderDevice);
    DclAssert(pBuffer);

    pRIO = pReaderDevice->pDeviceSpecificInfo;

    ulBlockStart += (*pRIO->hDisk)->fpdi.fpi[pRIO->uPartNum].ulStartSector;

    ioStat = FfxVbfReadPages((*pRIO->hDisk)->hVBF, ulBlockStart, ulBlockLength, pBuffer);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("ReaderIoRead() Error FfxVbfReadPages() %lU %lU failed with status %lX\n",
            ulBlockStart, ulBlockLength, ioStat.ffxStat);
    }

    dclStat = ioStat.ffxStat;

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: ReaderIoDestroy()

    Destroy this instance of the block device.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS ReaderIoDestroy(
    DCLREADERIO            *pReaderDevice)
{
    READERIOINSTANCE       *pRIO;

    if(pReaderDevice)
    {
        pRIO = pReaderDevice->pDeviceSpecificInfo;
        if(pRIO)
        {
            if(!pRIO->fDriverLocked)
                FfxDriverLock(pRIO->pDI);


            /*  Unmount the FlashFX allocator
            */
            if(pRIO->fAllocatorMounted)
                FfxDriverAllocatorDestroy(pRIO->hDisk, FFX_SHUTDOWNFLAGS_NORMAL);


            /*  Destroy the instance of the FlashFX disk
            */
            if(pRIO->hDisk)
                FfxDriverDiskDestroy(pRIO->hDisk);


            /*  Destroy the instance of the FlashFX device
            */
            if(pRIO->hDev)
                FfxDriverDeviceDestroy(pRIO->hDev);


            FfxDriverUnlock(pRIO->pDI);


            /*  Destroy the instance of FlashFX
            */
            if(pRIO->pDI)
            {
              #if DRIVER_SUPPORT_MULTIPLE_INSTANCES
                if(guOpenReaderIOs == 1)
                {
                    FfxDriverDestroy(pRIO->pDI);
                }
                if(guOpenReaderIOs > 0)
                {
                    guOpenReaderIOs--;
                }
              #else
                FfxDriverDestroy(pRIO->pDI);
              #endif
            }


            /*  Free the memory allocated to manage this instance of the driver
            */
            DclMemFree(pRIO);
        }
        DclMemSet(pReaderDevice, 0, sizeof(*pReaderDevice));
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxReaderIoCreate()

    Create an instance of the FlashFX Reader.

    Parameters:
        nDeviceNum    - The Device number
        nDiskNum      - The Disk number
        uPartNum      - The partition number
        pReaderDevice - A pointer to the DCLREADERIO structure to use.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS FfxReaderIoCreate(
    unsigned                nDeviceNum,
    unsigned                nDiskNum,
    unsigned short          uPartNum,
    DCLREADERIO            *pReaderDevice)
{
    READERIOINSTANCE       *pRIO;
    FFXDEVINITDATA          DevInitData;
    FFXDISKINITDATA         DiskInitData;
    FFXSTATUS               ffxStat;

    DclAssert(pReaderDevice);

    DevInitData.nDevNum = nDeviceNum;
    DiskInitData.nDiskNum = nDiskNum;

    /*  Allocate a structure to manage this instance of the driver
    */
    DclMemSet(pReaderDevice, 0, sizeof(*pReaderDevice));

    pRIO = DclMemAllocZero(sizeof *pRIO);
    if(!pRIO)
    {
        DclPrintf("FfxReaderIoCreate() Out of memory\n");
        return DCLSTAT_FAILURE;
    }
    pReaderDevice->pDeviceSpecificInfo = pRIO;
    pReaderDevice->fnIoDestroy = ReaderIoDestroy;
    pReaderDevice->fnIoGetParameters = ReaderIoGetParameters;
    pReaderDevice->fnIoRead = ReaderIoRead;

  #if DRIVER_SUPPORT_MULTIPLE_INSTANCES
    if(guOpenReaderIOs == 0)
    {
        /*  Create an instance of FlashFX
        */
        gpDI = FfxDriverCreate(1, NULL);
        if(!gpDI)
        {
            DclPrintf("FfxReaderIoCreate() Initialization failed\n");
            goto InitFailure;
        }
    }
    pRIO->pDI = gpDI;
    guOpenReaderIOs++;

  #else

    /*  Create an instance of FlashFX
    */
    pRIO->pDI = FfxDriverCreate(1, NULL);
    if(!pRIO->pDI)
    {
        DclPrintf("FfxReaderIoCreate() Initialization failed\n");
        goto InitFailure;
    }
  #endif

  #if FFXCONF_DRIVERAUTOTEST
    FfxDriverUnitTestDCL(NULL);
  #endif

    pRIO->uPartNum = uPartNum;

    FfxDriverLock(pRIO->pDI);
    pRIO->fDriverLocked = TRUE;

    /*  Create an instance of a FlashFX device
    */
    pRIO->hDev = FfxDriverDeviceCreate(pRIO->pDI, &DevInitData);
    if(!pRIO->hDev)
    {
        DclPrintf("FfxReaderIoCreate() DEVn initialization failed\n");
        goto InitFailure;
    }

    /*  Create an instance of a FlashFX disk
    */
    pRIO->hDisk = FfxDriverDiskCreate(pRIO->pDI, &DiskInitData);
    if(!pRIO->hDisk)
    {
        DclPrintf("FfxReaderIoCreate() DISKn initialization failed\n");
        goto InitFailure;
    }

  #if FFXCONF_DRIVERAUTOTEST
    FfxDriverUnitTest(NULL, pRIO->hDisk);
  #endif

    /*  Mount the FlashFX allocator
    */
    ffxStat = FfxDriverAllocatorCreate(pRIO->hDisk);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("FfxReaderIoCreate() VBF initialization failed\n");
        goto InitFailure;
    }
    pRIO->fAllocatorMounted = TRUE;

    FfxDriverUnlock(pRIO->pDI);
    pRIO->fDriverLocked = FALSE;

    DclPrintf("FfxReaderIoCreate() Initialization complete\n");
    return DCLSTAT_SUCCESS;

  InitFailure:

    ReaderIoDestroy(pReaderDevice);
    DclPrintf("FfxReaderIoCreate() Init Failed\n");

    return DCLSTAT_FAILURE;
}


