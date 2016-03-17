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

    This module contains helper routines that are used by the OS level
    device driver.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvvbf.c $
    Revision 1.28  2010/12/12 05:38:11Z  garyp
    Renamed some functions for interface consistency -- no functional
    changes.
    Revision 1.27  2010/11/22 16:20:19Z  glenns
    Modified SizeChange procedure to account for disks formatted
    using the QuickMount feature, which can make a VBF-formatted
    disk appear smaller than the FML disk on which it resides.
    Revision 1.26  2009/07/31 20:06:19Z  garyp
    Eliminated the ReadAhead feature.
    Revision 1.25  2009/07/22 18:09:06Z  garyp
    Corrected a misnamed variable.
    Revision 1.24  2009/07/21 21:47:18Z  garyp
    Merged from the v4.0 branch.  Updated so that VBF instances are created
    in compaction suspend mode and have to be explicitly resumed.  Modified
    the shutdown processes to take a mode parameter.  Some variables
    renamed for consistency.  Documentation updated.
    Revision 1.23  2009/04/16 23:05:13Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.22  2009/04/15 22:05:38Z  thomd
    Avoid calling FixVbfSizeChange on fEvaluation drive
    Revision 1.21  2009/04/09 21:26:38Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.20  2009/04/07 19:45:47Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.19  2009/03/31 19:33:28Z  davidh
    Function Hearders updated for AutoDoc.
    Revision 1.18  2008/12/09 20:51:31Z  keithg
    No longer pass a serial number to VbfFormat() - it is handled internally.
    Revision 1.17  2008/05/23 13:28:10Z  thomd
    Added paramter to format - fFormatNoErase = FALSE
    Revision 1.16  2008/05/06 16:11:49Z  keithg
    Updated to use new FXSTAT_VBF_xxx codes.
    Revision 1.15  2008/03/24 19:00:53Z  Garyp
    Updated to function properly if allocator support is disabled or compiled
    out.  Added FfxDriverDiskCalcParams() from drvparam.c.
    Revision 1.14  2008/01/13 07:26:36Z  keithg
    Function header updates to support autodoc.
    Revision 1.13  2007/12/01 03:32:53Z  Garyp
    Modified FfxFatFormat() to return a meaningful status code rather than
    D_BOOL, and adjusted the calling code to make use of it as appropriate.
    Revision 1.12  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.11  2007/09/26 23:54:01Z  jeremys
    Renamed a header file.
    Revision 1.10  2007/09/12 19:51:01Z  Garyp
    Resolved Bug 1348 where uSectorToPageShift could end up wrong in some cases.
    Revision 1.9  2007/08/29 22:26:43Z  pauli
    Renamed USE_DEFAULT to FFX_USE_DEFAULT.
    Revision 1.8  2007/07/26 19:05:15Z  timothyj
    Changed VBF re-format on size change criteria from mismatch of FMSL and VBF
    byte-wise disk sizes to mismatch of number of VBF erase units (resolves
    BZ #973).
    Revision 1.7  2007/03/20 21:43:08Z  Garyp
    Eliminated the obsolete ulMaxTransferSize field.
    Revision 1.6  2006/10/14 21:00:38Z  Garyp
    Fixed a logic error in the decision as to whether or not to call the VBF
    size change fix function.
    Revision 1.5  2006/10/12 04:22:52Z  Garyp
    Major refactoring to split out the high level formatting functionality.
    Updated to reformat VBF to accommodate the "downsizing" problem when the
    configuration has changed such that the disk is made smaller.
    Revision 1.4  2006/07/07 22:39:44Z  Garyp
    Modified FfxDriverVbfCreate() to take a fForceFormat parameter to allow
    the disk to be forcibly formatted at mount time, regardless what the driver
    auto-format settings are configured for.
    Revision 1.3  2006/06/12 12:17:31Z  Garyp
    Cleaned up debug messages.  Updated to use a renamed function.
    Revision 1.2  2006/02/21 02:02:41Z  Garyp
    Updated to work with the new VBF API.
    Revision 1.1  2006/02/07 22:15:18Z  Garyp
    Initial revision
    Revision 1.4  2006/01/12 03:57:09Z  Garyp
    Updated to handle (or gracefully fail) in cases where non-standard
    block sizes are being used.
    Revision 1.3  2006/01/08 14:14:48Z  Garyp
    Modified to initialize uAllocBlockSize and ulMaxTransferSize in pDev,
    immediately after mounting VBF.
    Revision 1.2  2005/12/28 00:48:32Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.1  2005/12/04 21:07:06Z  Pauli
    Initial revision
    Revision 1.2  2005/12/04 21:07:06Z  Garyp
    Modified the compaction model to be specified as a tri-state value, which is
    one of the following FFX_COMPACT_SYNCHRONOUS, FFX_COMPACT_BACKGROUNDIDLE, or
    FFX_COMPACT_BACKGROUNDTHREAD.
    Revision 1.1  2005/10/06 06:50:54Z  Garyp
    Initial revision
    Revision 1.23  2004/12/30 17:32:42Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.22  2004/11/19 20:23:49Z  GaryP
    Modified to accomodate nested compaction suspends.
    Revision 1.21  2004/11/09 02:46:07Z  GaryP
    Modified to use the updated background compaction API.
    Revision 1.20  2004/10/29 00:57:49Z  GaryP
    Changed garbage collection terminology to compaction.
    Revision 1.19  2004/09/23 03:36:54Z  GaryP
    Changed to use "compaction" rather than "garbage collection".
    Revision 1.18  2004/09/16 23:50:48Z  GaryP
    Fixed a function that should have been declared static.
    Revision 1.17  2004/09/10 18:11:50Z  jaredw
    Some style changes
    Revision 1.16  2004/09/10 17:23:30Z  jaredw
    Fixed return of AllocatorMount to use ffxStat to fix compiler warnings
    about ffxStat not being used.
    Revision 1.15  2004/09/09 20:36:38Z  jaredw
    Added an FFXSTATUS variable to the AllocatorMount function to allow control
    to get past AutoFormatCheck.
    Revision 1.14  2004/08/25 07:16:20Z  GaryP
    Added support for background garbage collection.
    Revision 1.13  2004/08/13 22:53:20Z  GaryP
    Moved the FAT Monitor init/deinit logic so it happens after the allocator is
    mounted.
    Revision 1.12  2004/08/09 19:38:14Z  GaryP
    Eliminated FAT specific logic.
    Revision 1.11  2004/07/03 02:36:03Z  GaryP
    Eliminated FfxDriverSetDeviceInfo() and FfxDriverSetDeviceFATInfo() usage.
    Revision 1.10  2004/04/30 02:27:40Z  garyp
    Minor parameter changes to the Driver Framework.  Eliminated the use of
    a common sector buffer.
    Revision 1.9  2004/01/25 02:47:28Z  garys
    Merge from FlashFXMT
    Revision 1.6.1.4  2004/01/25 02:47:28  garyp
    Updated to generate code based on the FFXCONF_DRIVERAUTOFORMAT setting.
    Revision 1.6.1.3  2003/12/04 22:51:42Z  garyp
    Changed FfxDriverMountDevice() to FfxDriverAllocatorMount() and
    Added a corollary unmount function.
    Revision 1.6.1.2  2003/11/05 03:20:38Z  garyp
    Re-checked into variant sandbox.
    Revision 1.7  2003/11/05 03:20:38Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.6  2003/07/22 05:22:12Z  garyp
    Added debugging messages.
    Revision 1.5  2003/07/09 19:48:52Z  garyp
    Added FfxDriverSetDeviceInfo() and FfxDriverSetDeviceFATInfo().
    Revision 1.4  2003/05/08 01:39:34Z  garyp
    Eliminated changes made in the previous rev.
    Revision 1.3  2003/05/01 21:39:56Z  garyp
    Modified to reduce the total disk size in the mediainfo structure to the
    raw size if necessary.
    Revision 1.2  2003/04/16 01:59:54Z  garyp
    Minor logic refactoring.
    Revision 1.1  2003/04/11 22:27:06Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxfmlapi.h>

#if FFXCONF_ALLOCATORSUPPORT
#if FFXCONF_VBFSUPPORT

#include <vbf.h>

#if FFXCONF_FATMONITORSUPPORT
  #include <fatmon.h>
#endif

#if FFXCONF_DRIVERAUTOFORMAT
  static FFXSTATUS SizeChange(FFXDISKINFO *pDisk);
#endif


/*-------------------------------------------------------------------
    ToDo:
        When the allocator interface is completely abstracted, and
        we can support multiple allocators, the public functions
        named in this module should be moved to a module such as
        drvalloc.c, and implemented in a generic fashion to work
        with any allocator.  Then the functions contained in this
        module should be renamed with VBF specific names.
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
    Public: FfxDriverAllocatorCreate()

    Create an Allocator instance for a Disk.

    Parameters:
        hDisk        - The Disk handle

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverAllocatorCreate(
    FFXDISKHANDLE       hDisk)
{
    FFXDISKINFO        *pDisk = *hDisk;
    FFXSTATUS           ffxStat = FFXSTAT_SUCCESS;
    VBFDISKINFO         di;
    FFXALLOCSETTINGS    as;
  #if FFXCONF_DRIVERAUTOFORMAT
    D_BOOL              fDidFormat = FALSE;
    FFXFORMATSETTINGS   fs;
  #endif

    FFXPRINTF(1, ("FfxDriverAllocatorCreate() hDisk=%P\n", hDisk));

    DclAssert(pDisk);
    DclAssert(pDisk->hFML);
    DclAssert(pDisk->hVBF == NULL);

    if(!FfxHookOptionGet(FFXOPT_DISK_ALLOCSETTINGS, hDisk, &as, sizeof(as)))
    {
        DclError();
        return FFXSTAT_FAILURE;
    }

  #if FFXCONF_DRIVERAUTOFORMAT
    /*  While it is true that the pDisk structure contains a FFXFORMATSETTINGS
        structure, which will likely be the exact information returned here,
        we query through the options interface, so that format settings can
        be modified at run-time by project code.
    */
    if(!FfxHookOptionGet(FFXOPT_DISK_FORMATSETTINGS, hDisk, &fs, sizeof(fs)))
    {
        DclError();
        return FFXSTAT_FAILURE;
    }

    if(fs.uFormatState == FFX_FORMAT_ALWAYS)
    {
        ffxStat = FfxDriverAllocatorFormat(hDisk, as.nAllocator);
        if(ffxStat != FFXSTAT_SUCCESS)
            goto Cleanup;

        fDidFormat = TRUE;
    }
  #endif

    /*  Special case if the allocator we are using is "No Allocator".
        We execute up to this point to allow the format function to
        run (which in the case of "No Allocator" will simply do an
        FML format of the flash).  Now ensure we have some basic
        physical characteristics in place, and exit early.
    */
    if(as.nAllocator == FFX_ALLOC_NONE)
    {
        FFXFMLINFO      FmlInfo;

        ffxStat = FfxFmlDiskInfo(pDisk->hFML, &FmlInfo);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;

        FFXPRINTF(1, ("The DISK is configured for \"no allocator\".  Using physical disk characteristics.\n"));

        pDisk->ulSectorLength = FmlInfo.uPageSize;
        pDisk->uPageSize = FmlInfo.uPageSize;
        pDisk->uSectorToPageShift = 0;

        if(!FfxCalcPhysicalDiskParams(&pDisk->fpdi, (FmlInfo.ulBlockSize / FmlInfo.uPageSize) * FmlInfo.ulTotalBlocks))
            return FFXSTAT_BADDISKGEOMETRY;

        /*  All good...
        */
        return FFXSTAT_SUCCESS;
    }

    pDisk->hVBF = FfxVbfCreate(pDisk->hFML);
    if(!pDisk->hVBF)
    {
      #if FFXCONF_DRIVERAUTOFORMAT
        {
            if(fDidFormat || pDisk->Conf.FormatSettings.uFormatState == FFX_FORMAT_NEVER)
            {
                DclPrintf("FFX: VBF init failed, check the flash start address and length.\n");
                DclPrintf("     The drive may need to be formatted (FormatState=%U).\n", pDisk->Conf.FormatSettings.uFormatState);

                /*  If FfxVbfCreate() fails and we're not allowed to reformat
                    the flash, we can go no further, so fail.
                */
                ffxStat = FFXSTAT_VBF_MOUNTFAILED;
                goto Cleanup;
            }

            /*  If we get to this point, we must be using the only real
                allocator we support (now) -- VBF.
            */
            DclAssert(as.nAllocator == FFX_ALLOC_VBF);

            ffxStat = FfxDriverAllocatorFormat(hDisk, as.nAllocator);
            if(ffxStat != FFXSTAT_SUCCESS)
                goto Cleanup;

            fDidFormat = TRUE;

            /*  Should be able to load VBF now...
            */
            pDisk->hVBF = FfxVbfCreate(pDisk->hFML);
            if(!pDisk->hVBF)
            {
                DclPrintf("FFX: VBF init failed even after formatting!\n");
                ffxStat = FFXSTAT_VBF_MOUNTFAILED;
                goto Cleanup;
            }
        }
      #else
        {
            ffxStat = FFXSTAT_VBF_MOUNTFAILED;
            goto Cleanup;
        }
      #endif
    }

   /*---------------------------------------------------
       At this point we know that VBF has been mounted.
   ---------------------------------------------------*/
    ffxStat = FfxVbfDiskInfo(pDisk->hVBF, &di);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("FfxVbfDiskInfo failed!\n"));
        goto Cleanup;
    }


  #if FFXCONF_DRIVERAUTOFORMAT
    if(!fDidFormat && !di.fEvaluation)
    {
        /*  If we did not just finish a format, check for the "VBF size change"
            problem...
        */
        ffxStat = SizeChange(pDisk);
        if(ffxStat != FFXSTAT_SUCCESS)
            goto Cleanup;

        /*  Update the disk info structure for new values
        */
        ffxStat = FfxVbfDiskInfo(pDisk->hVBF, &di);
        if(ffxStat != FFXSTAT_SUCCESS)
        {
            FFXPRINTF(1, ("FfxVbfDiskInfo failed!\n"));
            goto Cleanup;
        }
    }
  #endif

    /*  Make sure the device's emulated sector size is a power of two
        multiple of the VBF allocation block size.
    */
    if(pDisk->ulSectorLength < di.uPageSize)
    {
        FFXPRINTF(1, ("Increasing disk sector size from %lU to %U to match VBF page size\n",
            pDisk->ulSectorLength, di.uPageSize));

        pDisk->ulSectorLength = di.uPageSize;
    }

    if((pDisk->ulSectorLength % di.uPageSize != 0) ||
        !DCLISPOWEROF2(pDisk->ulSectorLength / di.uPageSize))
    {
        FFXPRINTF(1, ("Emulated sector size %lU incompatible with VBF page size %U\n",
                     pDisk->ulSectorLength, di.uPageSize));
        ffxStat = FFXSTAT_BADSECTORSIZE;
        goto Cleanup;
    }

    pDisk->uPageSize = di.uPageSize;

    pDisk->uSectorToPageShift = 0;
    while(pDisk->ulSectorLength >> pDisk->uSectorToPageShift != pDisk->uPageSize)
        pDisk->uSectorToPageShift++;

  #if FFXCONF_DRIVERAUTOFORMAT
    if(fDidFormat)
    {
        /*  If we just did an allocator format, we know there is nothing yet
            on the Disk, so try to do a file system level format here, before
            we get to the "CalcParams" call below.

            In addition to the "success" status code, there are a couple
            status codes which we will not consider to be a failure.  If
            a default file system was not specified, or if the disk uses
            a sector size which is unsupported by our limited internal
            FAT format functionality, continue on.
        */
        ffxStat = FfxDriverFormatFileSystem(hDisk, &fs);
        if(ffxStat != FFXSTAT_SUCCESS
            && ffxStat != FFXSTAT_DRV_NOFILESYSTOFORMAT
            && ffxStat != FFXSTAT_FATFMT_UNSUPPORTEDSECTORSIZE)
        {
            goto Cleanup;
        }

        DclPrintf("FlashFX: Format complete\n");
    }
  #endif

    if(!FfxDriverDiskCalcParams(hDisk))
    {
        /*  If we still can't get the device params, something
            really bad is going on.
        */
        FFXPRINTF(1, ("FfxDriverDiskCalcParams failed!\n"));

        ffxStat = FFXSTAT_DEVPARAMSFAILED;
        goto Cleanup;
    }

    /*----------------------------------------------------------------
        Once we get to this point, we know that VBF is mounted and
        that we have valid device parameters.  We may or may not
        have valid partitions on the disk, but we do know the raw
        size and have built pseudo CHS values.
    ----------------------------------------------------------------*/

  #if FFXCONF_DRIVERAUTOFORMAT
    /*  If we're instructed to format ONCE, and we didn't just get done
        doing a format, and we can't find any valid partitions, do a
        file system level format.
    */
    if((pDisk->Conf.FormatSettings.uFormatState == FFX_FORMAT_ONCE)
        && !fDidFormat
        && !pDisk->fpdi.fFoundValidPartitions)
    {
        ffxStat = FfxDriverFormatFileSystem(hDisk, &fs);
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            DclPrintf("FlashFX: Format complete\n");

            if(!FfxDriverDiskCalcParams(hDisk))
            {
                FFXPRINTF(1, ("FfxDriverDiskCalcParams failed!\n"));

                ffxStat = FFXSTAT_DEVPARAMSFAILED;
                goto Cleanup;
            }
        }
        else if(ffxStat != FFXSTAT_DRV_NOFILESYSTOFORMAT
            && ffxStat != FFXSTAT_FATFMT_UNSUPPORTEDSECTORSIZE)
        {
            /*  We could be configured such that the "file system" level
                format doesn't do anything, so allow this.  We could also
                be configured such that our internal FAT format cannot
                format the flash -- allow this also.

                If any other status value is returned, it is an unhandled
                condition.
            */
            goto Cleanup;
        }
    }
  #endif

  #if FFXCONF_FATMONITORSUPPORT
    FfxFatMonInit(hDisk);
  #endif

  #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
    /*  The VBF instance is created with idle-time compaction
        suspended.  It must be explicitly enabled.
    */
    FfxVbfCompactIdleResume(pDisk->hVBF);

  #if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
    FfxDriverDiskCompactCreate(hDisk);
  #endif
  #endif

    ffxStat = FFXSTAT_SUCCESS;

  Cleanup:

    if(ffxStat != FFXSTAT_SUCCESS)
    {
        if(pDisk->hVBF)
        {
            FfxVbfDestroy(pDisk->hVBF, FFX_SHUTDOWNFLAGS_TIMECRITICAL);

            pDisk->hVBF = NULL;
        }
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverAllocatorDestroy()

    Destroy the allocator instance which was created with
    FfxDriverAllocatorCreate().

    Parameters:
        hDisk   - The Disk handle
        ulFlags - The destroy flags to use, which may be one of
                  the following:
          FFX_SHUTDOWNFLAGS_NORMAL       - Perform a normal shutdown.
          FFX_SHUTDOWNFLAGS_FAST         - Perform a fast shutdown.
          FFX_SHUTDOWNFLAGS_TIMECRITICAL - Perform a time-critical shutdown.

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverAllocatorDestroy(
    FFXDISKHANDLE   hDisk,
    D_UINT32        ulFlags)
{
    FFXDISKINFO    *pDisk = *hDisk;

    FFXPRINTF(1, ("FfxDriverAllocatorDestroy() hDisk=%P Flags=%lU\n", hDisk, ulFlags));

    DclAssert(!(ulFlags & FFX_SHUTDOWNFLAGS_RESERVEDMASK));

    if(!FfxDriverDiskHandleValidate(hDisk))
        return FFXSTAT_BADHANDLE;

  #if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
    if(pDisk->pFCI)
    {
        FFXSTATUS       ffxStat;

        /*  We MUST be able to suspend the background compaction thread
            before unmounting the allocator.  If that fails, just return
            failure in ignominy.
        */
        ffxStat = FfxDriverDiskCompactSuspend(pDisk);
        if(ffxStat != FFXSTAT_SUCCESS && ffxStat != FFXSTAT_DRVCOMPACTALREADYSUSPENDED)
        {
            DclError();
            return ffxStat;
        }

        ffxStat = FfxDriverDiskCompactDestroy(pDisk, 5000L);
        if(ffxStat != FFXSTAT_SUCCESS)
        {
            DclError();
        }
    }
  #endif

  #if FFXCONF_FATMONITORSUPPORT
    FfxFatMonDeinit(hDisk);
  #endif

    if(pDisk->hVBF)
    {
        FfxVbfDestroy(pDisk->hVBF, ulFlags);

        pDisk->hVBF = NULL;
    }

    return FFXSTAT_SUCCESS;
}


#if FFXCONF_FORMATSUPPORT

/*-------------------------------------------------------------------
    Public: FfxDriverAllocatorFormat()

    Format a Disk using the specific Allocator.

    Parameters:
        hDisk      - The Disk handle
        nAllocator - The allocator to use, which must be one of
                     the following:
                     - FFX_ALLOC_NONE
                     - FFX_ALLOC_VBF

    Return Value:
        Returns an FFXSTATUS code indicating the state of the
        operation.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverAllocatorFormat(
    FFXDISKHANDLE       hDisk,
    unsigned            nAllocator)
{
    FFXDISKINFO        *pDisk;
    FFXSTATUS           ffxStat;

    DclAssert(hDisk);

    pDisk = *hDisk;

    DclAssert(pDisk->hVBF == NULL);
    DclAssert(pDisk->hFML);

    DclPrintf("FlashFX: Formatting... One moment please\n");

    switch(nAllocator)
    {
        case FFX_ALLOC_NONE:
            /*  Whatever emulated physical disk information we had
                recorded is going away...
            */
            DclMemSet(&pDisk->fpdi, 0, sizeof pDisk->fpdi);

            ffxStat = FfxFmlFormat(pDisk->hFML);
            break;

        case FFX_ALLOC_VBF:
            /*  Whatever emulated physical disk information we had
                recorded is going away...
            */
            DclMemSet(&pDisk->fpdi, 0, sizeof pDisk->fpdi);

            ffxStat = FfxVbfFormat(pDisk->hFML, FFX_USE_DEFAULT, FFX_USE_DEFAULT, FALSE);
            if(ffxStat != FFXSTAT_SUCCESS)
            {
                DclPrintf("FFX: Format failed, check the flash start address and length, Status=%lX\n", ffxStat);
            }
            break;

        default:
            DclError();
            ffxStat = FFXSTAT_DRV_UNKNOWNALLOCATOR;
    }

    return ffxStat;
}


#if FFXCONF_DRIVERAUTOFORMAT

/*-------------------------------------------------------------------
    Local: SizeChange()

    This routine exists solely to correct a deficiency which
    occurs when the developer specified configuration has
    changed such that the disk size has been increased, yet
    the media still contains a valid smaller VBF format.

    If the format state is set to "FFX_FORMAT_ONCE", VBF will
    successfully mount using the smaller format and the new
    configuration parameters will not take effect.

    This function detects this and solves the problem in brute
    force fashion, by FML formatting (erasing) the disk and
    thereby allowing VBF to format the full size.

    Parameters:
        pDisk - A pointer to the FFXDISKINFO structure to use

    Return Value:
        Returns an FFXSTATUS code indicating the state of the
        operation.
-------------------------------------------------------------------*/
static FFXSTATUS SizeChange(
    FFXDISKINFO        *pDisk)
{
    FFXSTATUS           ffxStat = FFXSTAT_SUCCESS;
    VBFDISKMETRICS      dm;
    FFXFMLINFO          fi;
    D_UINT32            ulTotalUnits;

    /*  Retrieve the VBF disk information
    */
    ffxStat = FfxVbfDiskMetrics(pDisk->hVBF, &dm);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    /*  Return the FML disk information
    */
    ffxStat = FfxFmlDiskInfo(pDisk->hFML, &fi);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    /*  If the two geometries add up to be the same integer multiple of units, all is good
        They can differ by a number of erase blocks less than the number of blocks per unit.
    */
    {
        D_UINT32 ulPagesPerBlock = fi.ulBlockSize / dm.uPageSize;

        DclAssert((fi.ulBlockSize % dm.uPageSize) == 0);
        DclAssert(dm.ulTotalUnits <= (D_UINT32_MAX / dm.uUnitTotalPages));
        DclAssert(fi.ulTotalBlocks <= (D_UINT32_MAX / ulPagesPerBlock));

        /*  Note this truncates to a unit boundry, by design
        */
        ulTotalUnits = (fi.ulTotalBlocks * ulPagesPerBlock) / dm.uUnitTotalPages;
        
      #if FFXCONF_QUICKMOUNTSUPPORT

        {
            VBFDISKINFO         di;
            
            /*  If this disk appears to be formatted for QuickMount, adjust
                the number of units since the QuickMount feature will make
                the number of formatted units in the VBF disk appear less
                than the number of units available in the FML disk.
            */
            if (FfxVbfDiskInfo(pDisk->hVBF, &di) != FFXSTAT_SUCCESS)
            {
                FFXPRINTF(1, ("FfxVbfDiskInfo failed!\n"));
                return ffxStat;
            }
            if (di.fUsesQuickMount)
            {
                D_UINT32 ulUnitSize;
                D_UINT32 ulStateInfoSize = 0;
                D_UINT32 ulSetAsideUnits = 0;

                ulUnitSize = dm.uUnitTotalPages * fi.uPageSize;
                FfxVbfCalcDiskStateSize(pDisk->hFML, ulUnitSize, &ulStateInfoSize, &ulSetAsideUnits);
                ulTotalUnits -= ulSetAsideUnits;
            }
        }
      #endif
      
        if(dm.ulTotalUnits == ulTotalUnits)
            return FFXSTAT_SUCCESS;

        DclPrintf("FFX: Forcibly reformatting VBF again due to a configuration change.\n");
        DclPrintf("     (Disk size increased, but had a smaller, yet valid VBF format.)\n");

        DclAssert((dm.ulTotalUnits * dm.uUnitTotalPages) < (fi.ulTotalBlocks * ulPagesPerBlock));

    }

    ffxStat = FfxVbfDestroy(pDisk->hVBF, FFX_SHUTDOWNFLAGS_TIMECRITICAL);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    /*  This will trash everything on the disk
    */
    ffxStat = FfxFmlFormat(pDisk->hFML);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    ffxStat = FfxVbfFormat(pDisk->hFML, FFX_USE_DEFAULT, FFX_USE_DEFAULT, FALSE);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("FFX: Format failed, check the flash start address and length, Status=%lX\n", ffxStat);
        return ffxStat;
    }

    /*  Darn well better be able to load VBF now...
    */
    pDisk->hVBF = FfxVbfCreate(pDisk->hFML);
    if(!pDisk->hVBF)
    {
        DclPrintf("FFX: VBF init failed even after formatting!\n");
        ffxStat = FFXSTAT_VBF_MOUNTFAILED;
    }

    return FFXSTAT_SUCCESS;
}


#endif  /* FFXCONF_DRIVERAUTOFORMAT */
#endif  /* FFXCONF_FORMATSUPPORT */
#endif  /* FFXCONF_VBFSUPPORT */
#endif  /* FFXCONF_ALLOCATORSUPPORT */


/*-------------------------------------------------------------------
    Protected: FfxDriverDiskCalcParams()

    This routine determines the physical disk parameters, or
    calculates them if necessary.  If the disk does not need
    formatting, the FAT disk parameters for the first FAT
    partition are read as well.

    Parameters:
        hDisk - The Disk handle

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxDriverDiskCalcParams(
    FFXDISKHANDLE           hDisk)
{
    FFXPHYSICALDISKINFO    *pFPDI;
    D_BOOL                  fSuccess = FALSE;
    FFXDISKINFO            *pDisk;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverDiskCalcParams() hDisk=%P\n", hDisk));

    DclAssert(hDisk);

    pDisk = *hDisk;

    DclAssert(pDisk);

    pFPDI = &pDisk->fpdi;

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    {
        D_BUFFER               *pBuffer = NULL;
        FFXIOSTATUS             ioStat;

        DclAssert(pDisk->hVBF);
        DclAssert(pDisk->uPageSize);

        /*  The compiled-in disk sector length must be a multiple of the
            partition's allocation block size.
        */
    /*    if(DSK_SECTORLEN % pDisk->uPageSize != 0)
        {
            DclError();
            goto Cleanup;
        }
    */
        pBuffer = DclMemAlloc(pDisk->uPageSize);
        if(!pBuffer)
        {
            DclError();
            goto Cleanup;
        }

        /*  DSK_SECTORLEN is presumed to be the length of an MBR.
        */
        ioStat = FfxVbfReadPages(pDisk->hVBF, 0, 1, pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclError();
            goto Cleanup;
        }

        if (!FfxGetPhysicalDiskParams(pFPDI, pBuffer, pDisk->uPageSize))
        {
            DclError();
            goto Cleanup;
        }

        if(!pFPDI->ulTotalSectors)
        {
            VBFDISKINFO vdi;

            /*  This flag should always have been turned off if we
                couldn't calculate the total sectors.
            */
            DclAssert(!pFPDI->fFoundValidPartitions);

            if(FfxVbfDiskInfo(pDisk->hVBF, &vdi) != FFXSTAT_SUCCESS)
            {
                DclError();
                goto Cleanup;
            }

            /*  Since there's nothing on the disk that we can use to determine
                the size, use the VBF client size (since we know that VBF is
                mounted).
            */
            if(!FfxCalcPhysicalDiskParams(pFPDI, vdi.ulTotalPages))
            {
                DclError();
                goto Cleanup;
            }

            /*  After successfully having executed the code above, the
                total sectors value MUST now be initialized
            */
            DclAssert(pFPDI->ulTotalSectors);
        }

        fSuccess = TRUE;

      Cleanup:

        if(pBuffer)
            DclMemFree(pBuffer);
    }

  #else

    {
        if(!pFPDI->ulTotalSectors)
        {
            FFXFMLINFO  fi;

            /*  This flag should always have been turned off if we
                couldn't calculate the total sectors.
            */
            DclAssert(!pFPDI->fFoundValidPartitions);
            DclAssert(pDisk->hFML);

            if(FfxFmlDiskInfo(pDisk->hFML, &fi) == FFXSTAT_SUCCESS)
            {
                D_UINT32 ulTotalPages = (fi.ulBlockSize / fi.uPageSize) * fi.ulTotalBlocks;

                /*  Since we're not using an allocator, use the Disk
                    characteristics to mock up some CHS values and
                    from that a virtual total sector count.
                */
                if(FfxCalcPhysicalDiskParams(pFPDI, ulTotalPages))
                {
                    /*  After successfully having executed the code above, the
                        total sectors value MUST now be initialized
                    */
                    DclAssert(pFPDI->ulTotalSectors);

                    fSuccess = TRUE;
                }
                else
                {
                    DclError();
                }
            }
            else
            {
                DclError();
            }
        }
    }
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 2, TRACEUNDENT),
        "FfxDriverDiskCalcParams() returning %U\n", fSuccess));

    return fSuccess;
}


