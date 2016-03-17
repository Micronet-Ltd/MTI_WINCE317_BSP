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

    This module contains the handlers for dispatching I/O requests for NAND
    flash.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmlnand.c $
    Revision 1.28  2011/11/17 23:47:21Z  garyp
    Updated to let the compiler do the work rather than using DclMemSet().
    No functional changes.
    Revision 1.27  2010/12/18 04:01:06Z  glenns
    Fix function header comment blocks to meet Datalight coding standards. 
    Remove compiled-out code. Fix GetRawBlockInfo function to compensate for
    FML disk offset into a device.
    Revision 1.26  2010/12/15 05:16:06Z  glenns
    Major revision to support FML disk spanning of devices with BBMv5. See B
    ug 3280.
    Revision 1.25  2010/12/08 06:25:09Z  glenns
    For the moment disable some range-checking and FML-disk device spanning code
    that can cause raw APIs to fail in certain block ranges.
    Revision 1.24  2009/08/07 01:47:42Z  garyp
    Temporarily hacked up several of the "raw" functions to allow the test
    suite to pass.  These functions will not work with Disks that span Devices.
    Revision 1.23  2009/08/01 00:46:55Z  garyp
    Merged from the v4.0 branch.  Added the "Read/WriteUncorrectedPages"
    functions and eliminated the "ReadCorrectedPages" function.  Major update
    to support the concept of a Disk which spans multiple Devices.  Updated
    the error handling to avoid asserting for out-of-range requests.
    Revision 1.22  2009/04/03 20:30:51Z  davidh
    Function header error corrections for AutoDoc.
    Revision 1.21  2009/04/01 20:24:26Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.20  2009/01/19 04:34:41Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.19  2008/12/31 18:38:20Z  keithg
    Added bounds checking for Raw FML disks; Raw access within an
    FML disk remains bound only by the device.
    Revision 1.18  2008/12/31 00:04:48Z  keithg
    Replaced calls to add stats on raw access - the called functions
    will now ignore out of bounds requests.
    Revision 1.17  2008/12/25 06:25:31Z  keithg
    Added support new IOCTL support for raw erase operations; added
    asserts for parameters and debug messages for failure conditions;
    Removed per block statistics for raw functions.
    Revision 1.16  2008/12/22 04:19:45Z  keithg
    Removed parameter validation that must be completed in the devmgr.
    Revision 1.15  2008/12/18 17:20:19Z  keithg
    Replace the WriteNativePages functionality which as incorrectly
    removed in the previous check in.
    Revision 1.14  2008/12/18 05:04:44Z  keithg
    Corrected compile time conditions and removed unused IOCTLs.
    Removed asserts on handled conditions, fixed issues with mapping
    between raw and normal aand remapped addresses.
    Revision 1.13  2008/12/11 23:47:16Z  keithg
    Added support for read/write native pages and setting the block status.
    Revision 1.12  2008/12/02 23:38:03Z  keithg
    Removed raw block mapping calls into the BBM left to lower levels.
    Revision 1.11  2008/10/23 23:32:33Z  keithg
    Added get block and page info functions and conditioned
    obsolescent status functions.
    Revision 1.10  2008/05/24 05:47:55Z  keithg
    Raw block status functionality is no longer dependant upon the
    inclusion of BBM functionality.
    Revision 1.9  2008/05/07 01:20:24Z  garyp
    Updated to track read and write stats on a per-block basis.
    Revision 1.8  2008/03/15 20:47:18Z  Garyp
    Updated to support variable width tags.  Expanded the various Get/Set
    Page and Block Status functions.
    Revision 1.7  2008/01/13 07:36:42Z  keithg
    Function header updates to support autodoc.
    Revision 1.6  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2007/08/28 03:35:38Z  Garyp
    Fixed documentation, debug message, and return value problems in
    FfxFmlNandGetPhysicalBlockStatus().
    Revision 1.4  2007/06/29 21:13:02Z  rickc
    Added FfxFmlNandGetPhysicalBlockStatus()
    Revision 1.3  2006/11/10 20:20:59Z  Garyp
    Updated to support new statistics gathering capabilities.
    Revision 1.2  2006/02/10 23:34:27Z  Garyp
    Updated debugging code.
    Revision 1.1  2006/02/08 01:49:28Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxfmlapi.h>
#include <fxdevapi.h>
#include "fml.h"


#if FFXCONF_NANDSUPPORT

/*-------------------------------------------------------------------
    Private: FfxFmlNandReadUncorrectedPages()

    Reads pages from normally-mapped FML disk blocks while telling
    underlying device manager not to apply EDC to the data. All other
    normal remappings apply.

    NOTE: This interface is only used by FlashFX for testing EDC 
    functionality. Some NTMs or chips may not be able to actually
    turn the EDC off. If they are configured correctly, this will
    be reported correctly to the FML layer in the EDC capability
    and requirement fields, and the FMSL EDC test will know not to
    run EDC tests on such NTMs/chips.

    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandReadUncorrectedPages(
    FFXFMLHANDLE                        hFML,
    FFXIOR_FML_READ_UNCORRECTEDPAGES   *pFmlReq)
{
    FFXIOSTATUS                         ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        DclError();
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandReadUncorrectedPages() hFML=%P StartPage=%lX Count=%lX pData=%P\n",
        hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount, pFmlReq->pPageData));

    DclProfilerEnter("FfxFmlNandReadUncorrectedPages", 0, pFmlReq->ulPageCount);

    /*  Validate the range
    */
    if((pFmlReq->ulStartPage >= hFML->ulTotalPages) ||
        (pFmlReq->ulPageCount > hFML->ulTotalPages - pFmlReq->ulStartPage))
    {
        ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_READ_GENERIC     DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulPage = pFmlReq->ulStartPage;
        D_UINT32                    ulPagesRemaining = pFmlReq->ulPageCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first page we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulPage >= pLayout->ulPageCount)
        {
            ulPage -= pLayout->ulPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_READ_GENERIC;
        DevReq.ioSubFunc    = FXIOSUBFUNC_DEV_READ_UNCORRECTEDPAGES;
        DevReq.pPageData    = pFmlReq->pPageData;
        DevReq.pAltData     = pFmlReq->pSpare;

        while(ulPagesRemaining)
        {
            DevReq.ulStartPage = ulPage + pLayout->ulPageOffset;
            DevReq.ulCount = DCLMIN(pLayout->ulPageCount - ulPage, ulPagesRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            ulPagesRemaining        -= ioStat.ulCount;

            if(!ulPagesRemaining || !IOSUCCESS(ioStat, DevReq.ulCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pPageData        += ioStat.ulCount << hFML->nBytePageShift;
            if(DevReq.pAltData)
                DevReq.pAltData     += ioStat.ulCount * hFML->nSpareSize;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulPage = 0;
        }
    }

    DclProfilerLeave(0UL);

  #if FFXCONF_STATS_FML
    hFML->stats.ulUncorrectedPageReadRequests++;
    hFML->stats.ulUncorrectedPageReadTotal += ioTotal.ulCount;
    if(hFML->stats.ulUncorrectedPageReadMax < ioTotal.ulCount)
        hFML->stats.ulUncorrectedPageReadMax = ioTotal.ulCount;

    FfxFmlBlockStatsAddReads(hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount);
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandReadUncorrectedPages() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandWriteUncorrectedPages()

    Writes pages to normally-mapped FML disk blocks while telling
    underlying device manager not to apply EDC to the data. All other
    normal remappings apply.

    NOTE: This interface is only used by FlashFX for testing EDC 
    functionality. Some NTMs or chips may not be able to actually
    turn the EDC off. If they are configured correctly, this will
    be reported correctly to the FML layer in the EDC capability
    and requirement fields, and the FMSL EDC test will know not to
    run EDC tests on such NTMs/chips.


    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandWriteUncorrectedPages(
    FFXFMLHANDLE                        hFML,
    FFXIOR_FML_WRITE_UNCORRECTEDPAGES  *pFmlReq)
{
    FFXIOSTATUS                         ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        DclError();
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandWriteUncorrectedPages() hFML=%P StartPage=%lX Count=%lX pData=%P\n",
        hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount, pFmlReq->pPageData));

    DclProfilerEnter("FfxFmlNandWriteUncorrectedPages", 0, pFmlReq->ulPageCount);

    /*  Validate the range
    */
    if((pFmlReq->ulStartPage >= hFML->ulTotalPages) ||
        (pFmlReq->ulPageCount > hFML->ulTotalPages - pFmlReq->ulStartPage))
    {
        ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_WRITE_GENERIC    DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulPage = pFmlReq->ulStartPage;
        D_UINT32                    ulPagesRemaining = pFmlReq->ulPageCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first page we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulPage >= pLayout->ulPageCount)
        {
            ulPage -= pLayout->ulPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_WRITE_GENERIC;
        DevReq.ioSubFunc    = FXIOSUBFUNC_DEV_WRITE_UNCORRECTEDPAGES;
        DevReq.pPageData    = pFmlReq->pPageData;
        DevReq.pAltData     = pFmlReq->pSpare;

        while(ulPagesRemaining)
        {
            DevReq.ulStartPage = ulPage + pLayout->ulPageOffset;
            DevReq.ulCount = DCLMIN(pLayout->ulPageCount - ulPage, ulPagesRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            ulPagesRemaining        -= ioStat.ulCount;

            if(!ulPagesRemaining || !IOSUCCESS(ioStat, DevReq.ulCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pPageData        += ioStat.ulCount << hFML->nBytePageShift;
            if(DevReq.pAltData)
                DevReq.pAltData     += ioStat.ulCount * hFML->nSpareSize;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulPage = 0;
        }
    }

    DclProfilerLeave(0UL);

  #if FFXCONF_STATS_FML
    hFML->stats.ulUncorrectedPageWriteRequests++;
    hFML->stats.ulUncorrectedPageWriteTotal += ioTotal.ulCount;
    if(hFML->stats.ulUncorrectedPageWriteMax < ioTotal.ulCount)
        hFML->stats.ulUncorrectedPageWriteMax = ioTotal.ulCount;

    FfxFmlBlockStatsAddWrites(hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount);
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandWriteUncorrectedPages() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandReadTaggedPages()

    Reads pages and tags from the device. Subject to standard
    remappings by the device.

    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandReadTaggedPages(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_READ_TAGGEDPAGES    *pFmlReq)
{
    FFXIOSTATUS                     ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandReadTaggedPages() Structure length invalid\n" ));
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandReadTaggedPages() hFML=%P StartPage=%lX Count=%lX pData=%P pTag=%P TagSize=%u\n",
        hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount, pFmlReq->pPageData, pFmlReq->pTag, pFmlReq->nTagSize));

    DclProfilerEnter("FfxFmlNandReadTaggedPages", 0, pFmlReq->ulPageCount);

    /*  Validate the range
    */
    if((pFmlReq->ulStartPage >= hFML->ulTotalPages) ||
        (pFmlReq->ulPageCount > hFML->ulTotalPages - pFmlReq->ulStartPage))
    {
        ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_READ_GENERIC     DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulPage = pFmlReq->ulStartPage;
        D_UINT32                    ulPagesRemaining = pFmlReq->ulPageCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first page we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulPage >= pLayout->ulPageCount)
        {
            ulPage -= pLayout->ulPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_READ_GENERIC;
        DevReq.ioSubFunc    = FXIOSUBFUNC_DEV_READ_TAGGEDPAGES;
        DevReq.pPageData    = pFmlReq->pPageData;
        DevReq.pAltData     = pFmlReq->pTag;
        DevReq.nAltDataSize = pFmlReq->nTagSize;

        while(ulPagesRemaining)
        {
            DevReq.ulStartPage = ulPage + pLayout->ulPageOffset;
            DevReq.ulCount = DCLMIN(pLayout->ulPageCount - ulPage, ulPagesRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            ulPagesRemaining        -= ioStat.ulCount;

            if(!ulPagesRemaining || !IOSUCCESS(ioStat, DevReq.ulCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pPageData        += ioStat.ulCount << hFML->nBytePageShift;
            if(DevReq.pAltData)
                DevReq.pAltData     += ioStat.ulCount * pFmlReq->nTagSize;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulPage = 0;
        }
    }

    DclProfilerLeave(0UL);

  #if FFXCONF_STATS_FML
    hFML->stats.ulTaggedPageReadRequests++;
    hFML->stats.ulTaggedPageReadTotal += ioTotal.ulCount;
    if(hFML->stats.ulTaggedPageReadMax < ioTotal.ulCount)
        hFML->stats.ulTaggedPageReadMax = ioTotal.ulCount;

    FfxFmlBlockStatsAddReads(hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount);
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandReadTaggedPages() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandWriteTaggedPages()

    Writes pages and tags to the device. Subject to standard
    remappings by the device.

    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandWriteTaggedPages(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_WRITE_TAGGEDPAGES   *pFmlReq)
{
    FFXIOSTATUS                     ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandWriteTaggedPages() Structure length invalid\n" ));
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandWriteTaggedPages() hFML=%P StartPage=%lX Count=%lX pData=%P pTag=%P TagSize=%u\n",
        hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount, pFmlReq->pPageData, pFmlReq->pTag, pFmlReq->nTagSize));

    DclProfilerEnter("FfxFmlNandWriteTaggedPages", 0, pFmlReq->ulPageCount);

    /*  Validate the range
    */
    if((pFmlReq->ulStartPage >= hFML->ulTotalPages) ||
        (pFmlReq->ulPageCount > hFML->ulTotalPages - pFmlReq->ulStartPage))
    {
        ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_WRITE_GENERIC    DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulPage = pFmlReq->ulStartPage;
        D_UINT32                    ulPagesRemaining = pFmlReq->ulPageCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first page we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulPage >= pLayout->ulPageCount)
        {
            ulPage -= pLayout->ulPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_WRITE_GENERIC;
        DevReq.ioSubFunc    = FXIOSUBFUNC_DEV_WRITE_TAGGEDPAGES;
        DevReq.pPageData    = pFmlReq->pPageData;
        DevReq.pAltData     = pFmlReq->pTag;
        DevReq.nAltDataSize = pFmlReq->nTagSize;

        while(ulPagesRemaining)
        {
            DevReq.ulStartPage = ulPage + pLayout->ulPageOffset;
            DevReq.ulCount = DCLMIN(pLayout->ulPageCount - ulPage, ulPagesRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            ulPagesRemaining        -= ioStat.ulCount;

            if(!ulPagesRemaining || !IOSUCCESS(ioStat, DevReq.ulCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pPageData        += ioStat.ulCount << hFML->nBytePageShift;
            if(DevReq.pAltData)
                DevReq.pAltData     += ioStat.ulCount * pFmlReq->nTagSize;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulPage = 0;
        }
    }

    DclProfilerLeave(0UL);

  #if FFXCONF_STATS_FML
    hFML->stats.ulTaggedPageWriteRequests++;
    hFML->stats.ulTaggedPageWriteTotal += ioTotal.ulCount;
    if(hFML->stats.ulTaggedPageWriteMax < ioTotal.ulCount)
        hFML->stats.ulTaggedPageWriteMax = ioTotal.ulCount;

    FfxFmlBlockStatsAddWrites(hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount);
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandWriteTaggedPages() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandReadRawPages()

    Reads page and possibly spare area data from the device. The 
    input page number is assumed to be in a raw block selected
    from a range which may or may not span device-reserved segments
    of blocks. Normal device remappings do not apply. This function
    will assert on raw block numbers that are not spanned by the FML
    disk. This is particularly dangerous when using the GET_PAGE_INFO
    interface to retrieve the raw block number of a block that was
    remapped by BBM, because the BBM table where the substitute block
    is located may reside outside that range.

    This function should only be used by those who really know
    what they are doing.
    
    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandReadRawPages(
    FFXFMLHANDLE                hFML,
    FFXIOR_FML_READ_RAWPAGES   *pFmlReq)
{
    FFXIOSTATUS                 ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandReadRawPages() Structure length invalid\n" ));
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandReadRawPages() hFML=%P StartPage=%lX Count=%lX pData=%P pSpare=%P\n",
        hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount, pFmlReq->pPageData, pFmlReq->pSpare));

    DclProfilerEnter("FfxFmlNandReadRawPages", 0, pFmlReq->ulPageCount);

    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_READ_GENERIC     DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulPage = pFmlReq->ulStartPage;
        D_UINT32                    ulPagesRemaining = pFmlReq->ulPageCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first page we want to access.
        */
        pLayout = hFML->pLayout;

        while(ulPage >= pLayout->ulRawPageCount)
        {
            ulPage -= pLayout->ulRawPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_READ_GENERIC;
        DevReq.ioSubFunc    = FXIOSUBFUNC_DEV_READ_RAWPAGES;
        DevReq.pPageData    = pFmlReq->pPageData;
        DevReq.pAltData     = pFmlReq->pSpare;

        while(ulPagesRemaining)
        {
            DevReq.ulStartPage = ulPage + pLayout->ulRawPageOffset;
            DevReq.ulCount = DCLMIN(pLayout->ulRawPageCount - ulPage, ulPagesRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            ulPagesRemaining        -= ioStat.ulCount;

            if(!ulPagesRemaining || !IOSUCCESS(ioStat, DevReq.ulCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pPageData        += ioStat.ulCount << hFML->nBytePageShift;
            if(DevReq.pAltData)
                DevReq.pAltData     += ioStat.ulCount * hFML->nSpareSize;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulPage = 0;
        }
    }

    DclProfilerLeave(0UL);

  #if FFXCONF_STATS_FML
    hFML->stats.ulRawPageReadRequests++;
    hFML->stats.ulRawPageReadTotal += ioTotal.ulCount;
    if(hFML->stats.ulRawPageReadMax < ioTotal.ulCount)
        hFML->stats.ulRawPageReadMax = ioTotal.ulCount;

    FfxFmlBlockStatsAddReads(hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount);
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandReadRawPages() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandWriteRawPages()

    Writes page and possibly spare area data to the device. The 
    input page number is assumed to be in a raw block selected
    from a range which may or may not span device-reserved segments
    of blocks. Normal device remappings do not apply. This function
    will assert on raw block numbers that are not spanned by the FML
    disk. This is particularly dangerous when using the GET_PAGE_INFO
    interface to retrieve the raw block number of a block that was
    remapped by BBM, because the BBM table where the substitute block
    is located may reside outside that range.

    This function should only be used by those who really know
    what they are doing.
    
    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandWriteRawPages(
    FFXFMLHANDLE                hFML,
    FFXIOR_FML_WRITE_RAWPAGES  *pFmlReq)
{
    FFXIOSTATUS                 ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandWriteRawPages() Structure length invalid\n" ));
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandWriteRawPages() hFML=%P StartPage=%lX Count=%lX pData=%P pSpare=%P\n",
        hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount, pFmlReq->pPageData, pFmlReq->pSpare));

    DclProfilerEnter("FfxFmlNandWriteRawPages", 0, pFmlReq->ulPageCount);

    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_WRITE_GENERIC    DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulPage = pFmlReq->ulStartPage;
        D_UINT32                    ulPagesRemaining = pFmlReq->ulPageCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first page we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulPage >= pLayout->ulRawPageCount)
        {
            ulPage -= pLayout->ulRawPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_WRITE_GENERIC;
        DevReq.ioSubFunc    = FXIOSUBFUNC_DEV_WRITE_RAWPAGES;
        DevReq.pPageData    = pFmlReq->pPageData;
        DevReq.pAltData     = pFmlReq->pSpare;

        while(ulPagesRemaining)
        {
            DevReq.ulStartPage = ulPage + pLayout->ulRawPageOffset;
            DevReq.ulCount = DCLMIN(pLayout->ulRawPageCount - ulPage, ulPagesRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            ulPagesRemaining        -= ioStat.ulCount;

            if(!ulPagesRemaining || !IOSUCCESS(ioStat, DevReq.ulCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pPageData        += ioStat.ulCount << hFML->nBytePageShift;
            if(DevReq.pAltData)
                DevReq.pAltData     += ioStat.ulCount * hFML->nSpareSize;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulPage = 0;
        }
    }

    DclProfilerLeave(0UL);

  #if FFXCONF_STATS_FML
    hFML->stats.ulRawPageWriteRequests++;
    hFML->stats.ulRawPageWriteTotal += ioTotal.ulCount;
    if(hFML->stats.ulRawPageWriteMax < ioTotal.ulCount)
        hFML->stats.ulRawPageWriteMax = ioTotal.ulCount;

    FfxFmlBlockStatsAddWrites(hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount);
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandWriteRawPages() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandEraseRawBlocks()

    Erases blocks in the device. The input block number is assumed
    to be a raw block number selected from a range which may or may
    not span device-reserved segments of blocks. Normal device
    remappings do not apply. This function will assert on raw block
    numbers that are not spanned by the FML disk. This is particularly
    dangerous when using the GET_PAGE_INFO  interface to retrieve the
    raw block number of a block that was remapped by BBM, because 
    outside that range.

    This function should only be used by those who really know
    what they are doing.
    
    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandEraseRawBlocks(
    FFXFMLHANDLE                hFML,
    FFXIOR_FML_ERASE_RAWBLOCKS *pFmlReq)
{
    FFXIOSTATUS                 ioTotal = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);
    
    /*  These structures must be the same.
    */
    DclAssert( sizeof(FFXIOR_FML_ERASE_RAWBLOCKS)
            == sizeof(FFXIOR_FML_ERASE_BLOCKS) );

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandEraseRawBlocks() hFML=%P StartBlock=%lX Count=%lX\n",
        hFML, pFmlReq->ulStartBlock, pFmlReq->ulBlockCount));

    DclProfilerEnter("FfxFmlNandEraseRawBlocks", 0, pFmlReq->ulBlockCount);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandEraseRawBlocks() Structure length invalid\n" ));
        goto ErrorCleanup;
    }

    DclAssert(pFmlReq->ulBlockCount);

    /*  Limit access on a raw disk to the bounds established by
        the FML disk definition.  Raw access to any other
        disk is limited by the Device Manager.
    */
    if(hFML->uDiskFlags & DISK_RAWACCESSONLY)
    {
        if((hFML->ulTotalBlocks > pFmlReq->ulStartBlock)
            || (hFML->ulTotalBlocks - pFmlReq->ulStartBlock >= pFmlReq->ulBlockCount))
        {
            ioTotal.ffxStat = FFXSTAT_OUTOFRANGE;
            FFXPRINTF(2, ("Warning: out of bounds request.\n" ));
            goto ErrorCleanup;

        }
    }
    else
    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_ERASE_BLOCKS     DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulBlock = pFmlReq->ulStartBlock;
        D_UINT32                    ulBlocksRemaining = pFmlReq->ulBlockCount;
        
        pLayout = hFML->pLayout;
        while(ulBlock >= pLayout->ulRawBlockCount)
        {
            ulBlock -= pLayout->ulRawBlockCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc = FXIOFUNC_DEV_ERASE_RAWBLOCKS;

        while(ulBlocksRemaining)
        {
            DevReq.ulStartBlock = ulBlock + pLayout->ulRawBlockOffset;
            DevReq.ulBlockCount = DCLMIN(pLayout->ulRawBlockCount - ulBlock, ulBlocksRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount          += ioStat.ulCount;
            ioTotal.ffxStat           = ioStat.ffxStat;
            ioTotal.ulFlags           = ioStat.ulFlags;
            ioTotal.op.ulBlockStatus |= ioStat.op.ulBlockStatus;

            ulBlocksRemaining        -= ioStat.ulCount;

            if(!ulBlocksRemaining || !IOSUCCESS(ioStat, DevReq.ulBlockCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulBlock = 0;
        }
    }

  ErrorCleanup:
    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandEraseRawBlocks() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandReadSpares()

    Read spare area from the selected device page. Normal device
    remappings apply.
    
    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandReadSpares(
    FFXFMLHANDLE            hFML,
    FFXIOR_FML_READ_SPARES *pFmlReq)
{
    FFXIOSTATUS             ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandReadSpares() Structure length invalid\n" ));
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandReadSpares() hFML=%P StartPage=%lX Count=%lX pSpare=%P\n",
        hFML, pFmlReq->ulStartPage, pFmlReq->ulSpareCount, pFmlReq->pSpare));

    DclProfilerEnter("FfxFmlNandReadSpares", 0, pFmlReq->ulSpareCount);

    /*  Validate the range
    */
    if((pFmlReq->ulStartPage >= hFML->ulTotalPages) ||
        (pFmlReq->ulSpareCount > hFML->ulTotalPages - pFmlReq->ulStartPage))
    {
        ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_READ_GENERIC     DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulPage = pFmlReq->ulStartPage;
        D_UINT32                    ulPagesRemaining = pFmlReq->ulSpareCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first page we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulPage >= pLayout->ulPageCount)
        {
            ulPage -= pLayout->ulPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_READ_GENERIC;
        DevReq.ioSubFunc    = FXIOSUBFUNC_DEV_READ_SPARES;
        DevReq.pAltData     = pFmlReq->pSpare;

        while(ulPagesRemaining)
        {
            DevReq.ulStartPage = ulPage + pLayout->ulPageOffset;
            DevReq.ulCount = DCLMIN(pLayout->ulPageCount - ulPage, ulPagesRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            ulPagesRemaining        -= ioStat.ulCount;

            if(!ulPagesRemaining || !IOSUCCESS(ioStat, DevReq.ulCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pAltData         += ioStat.ulCount * hFML->nSpareSize;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulPage = 0;
        }
    }

    DclProfilerLeave(0UL);

  #if FFXCONF_STATS_FML
    hFML->stats.ulSpareAreaReadRequests++;
    hFML->stats.ulSpareAreaReadTotal += ioTotal.ulCount;
    if(hFML->stats.ulSpareAreaReadMax < ioTotal.ulCount)
        hFML->stats.ulSpareAreaReadMax = ioTotal.ulCount;
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandReadSpares() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandWriteSpares()

    Write data to the spare area of the selected device page. Normal
    device remappings apply.
    
    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandWriteSpares(
    FFXFMLHANDLE                hFML,
    FFXIOR_FML_WRITE_SPARES    *pFmlReq)
{
    FFXIOSTATUS                 ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandWriteSpares() Structure length invalid\n" ));
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandWriteSpares() hFML=%P StartPage=%lX Count=%lX pSpare=%P\n",
        hFML, pFmlReq->ulStartPage, pFmlReq->ulSpareCount, pFmlReq->pSpare));

    DclProfilerEnter("FfxFmlNandWriteSpares", 0, pFmlReq->ulSpareCount);

    /*  Validate the range
    */
    if((pFmlReq->ulStartPage >= hFML->ulTotalPages) ||
        (pFmlReq->ulSpareCount > hFML->ulTotalPages - pFmlReq->ulStartPage))
    {
        ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_WRITE_GENERIC    DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulPage = pFmlReq->ulStartPage;
        D_UINT32                    ulPagesRemaining = pFmlReq->ulSpareCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first page we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulPage >= pLayout->ulPageCount)
        {
            ulPage -= pLayout->ulPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_WRITE_GENERIC;
        DevReq.ioSubFunc    = FXIOSUBFUNC_DEV_WRITE_SPARES;
        DevReq.pAltData     = pFmlReq->pSpare;

        while(ulPagesRemaining)
        {
            DevReq.ulStartPage = ulPage + pLayout->ulPageOffset;
            DevReq.ulCount = DCLMIN(pLayout->ulPageCount - ulPage, ulPagesRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            ulPagesRemaining        -= ioStat.ulCount;

            if(!ulPagesRemaining || !IOSUCCESS(ioStat, DevReq.ulCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pAltData         += ioStat.ulCount * hFML->nSpareSize;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulPage = 0;
        }
    }

    DclProfilerLeave(0UL);

  #if FFXCONF_STATS_FML
    hFML->stats.ulSpareAreaWriteRequests++;
    hFML->stats.ulSpareAreaWriteTotal += ioTotal.ulCount;
    if(hFML->stats.ulSpareAreaWriteMax < ioTotal.ulCount)
        hFML->stats.ulSpareAreaWriteMax = ioTotal.ulCount;
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandWriteSpares() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandReadTags()

    Read tags from the selected device page. Normal device remappings
    apply.
    
    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandReadTags(
    FFXFMLHANDLE            hFML,
    FFXIOR_FML_READ_TAGS   *pFmlReq)
{
    FFXIOSTATUS             ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandReadTags() Structure length invalid\n" ));
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandReadTags() hFML=%P StartPage=%lX Count=%lX pTag=%P TagSize=%u\n",
        hFML, pFmlReq->ulStartPage, pFmlReq->ulTagCount, pFmlReq->pTag, pFmlReq->nTagSize));

    DclProfilerEnter("FfxFmlNandReadTags", 0, pFmlReq->ulTagCount);

    /*  Validate the range
    */
    if((pFmlReq->ulStartPage >= hFML->ulTotalPages) ||
        (pFmlReq->ulTagCount > hFML->ulTotalPages - pFmlReq->ulStartPage))
    {
        ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_READ_GENERIC     DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulPage = pFmlReq->ulStartPage;
        D_UINT32                    ulPagesRemaining = pFmlReq->ulTagCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first page we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulPage >= pLayout->ulPageCount)
        {
            ulPage -= pLayout->ulPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_READ_GENERIC;
        DevReq.ioSubFunc    = FXIOSUBFUNC_DEV_READ_TAGS;
        DevReq.pAltData     = pFmlReq->pTag;
        DevReq.nAltDataSize = pFmlReq->nTagSize;

        while(ulPagesRemaining)
        {
            DevReq.ulStartPage = ulPage + pLayout->ulPageOffset;
            DevReq.ulCount = DCLMIN(pLayout->ulPageCount - ulPage, ulPagesRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            ulPagesRemaining        -= ioStat.ulCount;

            if(!ulPagesRemaining || !IOSUCCESS(ioStat, DevReq.ulCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pAltData         += ioStat.ulCount * pFmlReq->nTagSize;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulPage = 0;
        }
    }

    DclProfilerLeave(0UL);

  #if FFXCONF_STATS_FML
    hFML->stats.ulTagReadRequests++;
    hFML->stats.ulTagReadTotal += ioTotal.ulCount;
    if(hFML->stats.ulTagReadMax < ioTotal.ulCount)
        hFML->stats.ulTagReadMax = ioTotal.ulCount;
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandReadTags() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandWriteTags()

    Write tags to the selected device page. Normal device remappings
    apply.
    
    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandWriteTags(
    FFXFMLHANDLE                hFML,
    FFXIOR_FML_WRITE_TAGS      *pFmlReq)
{
    FFXIOSTATUS                 ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandWriteTags() Structure length invalid\n" ));
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandWriteTags() hFML=%P StartPage=%lX Count=%lX pTags=%P TagSize=%u\n",
        hFML, pFmlReq->ulStartPage, pFmlReq->ulTagCount, pFmlReq->pTag, pFmlReq->nTagSize));

    DclProfilerEnter("FfxFmlNandWriteTags", 0, pFmlReq->ulTagCount);

    /*  Validate the range
    */
    if((pFmlReq->ulStartPage >= hFML->ulTotalPages) ||
        (pFmlReq->ulTagCount > hFML->ulTotalPages - pFmlReq->ulStartPage))
    {
        ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_WRITE_GENERIC    DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulPage = pFmlReq->ulStartPage;
        D_UINT32                    ulPagesRemaining = pFmlReq->ulTagCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first page we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulPage >= pLayout->ulPageCount)
        {
            ulPage -= pLayout->ulPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_WRITE_GENERIC;
        DevReq.ioSubFunc    = FXIOSUBFUNC_DEV_WRITE_TAGS;
        DevReq.pAltData     = pFmlReq->pTag;
        DevReq.nAltDataSize = pFmlReq->nTagSize;

        while(ulPagesRemaining)
        {
            DevReq.ulStartPage = ulPage + pLayout->ulPageOffset;
            DevReq.ulCount = DCLMIN(pLayout->ulPageCount - ulPage, ulPagesRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            ulPagesRemaining        -= ioStat.ulCount;

            if(!ulPagesRemaining || !IOSUCCESS(ioStat, DevReq.ulCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pAltData         += ioStat.ulCount * pFmlReq->nTagSize;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulPage = 0;
        }
    }

    DclProfilerLeave(0UL);

  #if FFXCONF_STATS_FML
    hFML->stats.ulTagWriteRequests++;
    hFML->stats.ulTagWriteTotal += ioTotal.ulCount;
    if(hFML->stats.ulTagWriteMax < ioTotal.ulCount)
        hFML->stats.ulTagWriteMax = ioTotal.ulCount;
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandWriteTags() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandReadNativePages()

    Read page and spare data from the selected device page as in
    FfxFmlNandReadRawPages, except normal device remappings and 
    EDC apply.
    
    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandReadNativePages(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_READ_NATIVEPAGES    *pFmlReq)
{
    FFXIOSTATUS                     ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandReadNativePages() Structure length invalid\n" ));
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandReadNativePages() hFML=%P StartPage=%lX Count=%lX pData=%P pSpare=%P\n",
        hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount, pFmlReq->pPageData, pFmlReq->pSpare));

    DclProfilerEnter("FfxFmlNandReadNativePages", 0, pFmlReq->ulPageCount);

    /*  Validate the range
    */
    if((pFmlReq->ulStartPage >= hFML->ulTotalPages) ||
        (pFmlReq->ulPageCount > hFML->ulTotalPages - pFmlReq->ulStartPage))
    {
        ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOR_DEV_READ_GENERIC     DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulPage = pFmlReq->ulStartPage;
        D_UINT32                    ulPagesRemaining = pFmlReq->ulPageCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first page we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulPage >= pLayout->ulPageCount)
        {
            ulPage -= pLayout->ulPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_READ_GENERIC;
        DevReq.ioSubFunc    = FXIOSUBFUNC_DEV_READ_NATIVEPAGES;
        DevReq.pPageData    = pFmlReq->pPageData;
        DevReq.pAltData     = pFmlReq->pSpare;

        while(ulPagesRemaining)
        {
            FFXIOSTATUS ioStat;

            DevReq.ulStartPage = ulPage + pLayout->ulPageOffset;
            DevReq.ulCount = DCLMIN(pLayout->ulPageCount - ulPage, ulPagesRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            ulPagesRemaining        -= ioStat.ulCount;

            if(!ulPagesRemaining || !IOSUCCESS(ioStat, DevReq.ulCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pPageData        += ioStat.ulCount << hFML->nBytePageShift;
            if(DevReq.pAltData)
                DevReq.pAltData     += ioStat.ulCount * hFML->nSpareSize;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulPage = 0;
        }
    }

  #if FFXCONF_STATS_FML
    hFML->stats.ulNativePageReadRequests++;
    hFML->stats.ulNativePageReadTotal += ioTotal.ulCount;
    if(hFML->stats.ulNativePageReadMax < ioTotal.ulCount)
        hFML->stats.ulNativePageReadMax = ioTotal.ulCount;

    FfxFmlBlockStatsAddReads(hFML, pFmlReq->ulStartPage, ioTotal.ulCount);
  #endif

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandReadNativePages() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandWriteNativePages()

    Write page and spare data to the selected device page as in
    FfxFmlNandwriteRawPages, except normal device remappings and 
    EDC apply.
    
    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandWriteNativePages(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_WRITE_NATIVEPAGES   *pFmlReq)
{
    FFXIOSTATUS                     ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandWriteNativePages() Structure length invalid\n" ));
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandWriteNativePages() hFML=%P StartPage=%lX Count=%lX pData=%P pSpare=%P\n",
        hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount, pFmlReq->pPageData, pFmlReq->pSpare));

    DclProfilerEnter("FfxFmlNandWriteNativePages", 0, pFmlReq->ulPageCount);

    /*  Validate the range
    */
    if((pFmlReq->ulStartPage >= hFML->ulTotalPages) ||
        (pFmlReq->ulPageCount > hFML->ulTotalPages - pFmlReq->ulStartPage))
    {
        ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOR_DEV_WRITE_GENERIC    DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulPage = pFmlReq->ulStartPage;
        D_UINT32                    ulPagesRemaining = pFmlReq->ulPageCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first page we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulPage >= pLayout->ulPageCount)
        {
            ulPage -= pLayout->ulPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_WRITE_GENERIC;
        DevReq.ioSubFunc    = FXIOSUBFUNC_DEV_WRITE_NATIVEPAGES;
        DevReq.pPageData    = pFmlReq->pPageData;
        DevReq.pAltData     = pFmlReq->pSpare;

        while(ulPagesRemaining)
        {
            FFXIOSTATUS ioStat;

            DevReq.ulStartPage = ulPage + pLayout->ulPageOffset;
            DevReq.ulCount = DCLMIN(pLayout->ulPageCount - ulPage, ulPagesRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            ulPagesRemaining        -= ioStat.ulCount;

            if(!ulPagesRemaining || !IOSUCCESS(ioStat, DevReq.ulCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pPageData        += ioStat.ulCount << hFML->nBytePageShift;
            if(DevReq.pAltData)
                DevReq.pAltData     += ioStat.ulCount * hFML->nSpareSize;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulPage = 0;
        }
    }

  #if FFXCONF_STATS_FML
    hFML->stats.ulNativePageWriteRequests++;
    hFML->stats.ulNativePageWriteTotal += ioTotal.ulCount;
    if(hFML->stats.ulNativePageWriteMax < ioTotal.ulCount)
        hFML->stats.ulNativePageWriteMax = ioTotal.ulCount;

    FfxFmlBlockStatsAddWrites(hFML, pFmlReq->ulStartPage, ioTotal.ulCount);
  #endif

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandWriteNativePages() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


#if FFXCONF_BBMSUPPORT

/*-------------------------------------------------------------------
    Private: FfxFmlNandGetBlockInfo()

    Get block information for the selected normally-mapped FML block.
    returns the block type as well as any normal device remapping.
    
    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandGetBlockInfo(
    FFXFMLHANDLE                        hFML,
    FFXIOR_FML_GET_BLOCK_INFO          *pFmlReq)
{
    FFXIOSTATUS                         ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandGetBlockInfo() Structure length invalid\n" ));
        return ioStat;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandGetBlockInfo() hFML=%P Block=%lX\n",
        hFML, pFmlReq->ulBlock));

    DclProfilerEnter("FfxFmlNandGetBlockInfo", 0, 0);

    if(pFmlReq->ulBlock >= hFML->ulTotalBlocks)
    {
        ioStat.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOR_DEV_GET_BLOCK_INFO   DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulBlock = pFmlReq->ulBlock;
        D_UINT32                    ulOffset = 0;

        /*  Initialize the raw mapping. We must keep track of this
            as we iterate through the layouts to account for the
            fact that BBM tables will cause given blocks within
            each layout to have different standard and raw mappings
            relative to the overal FML disk mapping.
        */
        pFmlReq->ulRawMapping = 0;
        
        /*  Iterate through the layout structures until we find the
            one which contains the block we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulBlock >= pLayout->ulBlockCount)
        {
            pFmlReq->ulRawMapping += pLayout->ulRawBlockCount - pLayout->ulBlockCount;
            ulOffset += pLayout->ulBlockCount;
            ulBlock -= pLayout->ulBlockCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen     = sizeof DevReq;
        DevReq.ior.ioFunc       = FXIOFUNC_DEV_GET_BLOCK_INFO;
        DevReq.ulBlock          = ulBlock + pLayout->ulBlockOffset;

        ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

        pFmlReq->ulBlockInfo    = DevReq.ulBlockInfo;
        pFmlReq->ulBlockStatus  = DevReq.ulBlockStatus;
        pFmlReq->ulRawMapping  += DevReq.ulRawMapping + ulOffset;
        
        /*  Remember that if there is an offset into the device, the
            returned mappings have to be shifted by that offset, including
            the raw mappings.
        */
        pFmlReq->ulRawMapping  -= pLayout->ulBlockOffset;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandGetBlockInfo() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandGetRawBlockInfo()

    Get block info about the selected raw FML block. the info
    returned includes both the block type, but no information about
    normal device remappings. The block is assumed to be a raw FML
    block number and must exist in the raw range spanned by the FML
    disk. This is particularly dangerous when using the GET_PAGE_INFO
    interface to retrieve the raw block number of a block that was
    remapped by BBM, because the BBM table where the substitute block
    is located may reside outside that range.

    This function should only be used by those who really know
    what they are doing.
    
    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandGetRawBlockInfo(
    FFXFMLHANDLE                        hFML,
    FFXIOR_FML_GET_RAW_BLOCK_INFO       *pFmlReq)
{
    FFXIOSTATUS                     ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);
    FFXIOR_DEV_GET_RAW_BLOCK_INFO   DevReq = {{0}};
    FFXFMLLAYOUT                   *pLayout;
    D_UINT32                        ulBlock = pFmlReq->ulBlock;
    D_UINT32                        ulOffset = 0;

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandGetRawBlockInfo() Structure length invalid\n" ));
        return ioStat;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandGetRawBlockInfo() hFML=%P Block=%lX\n",
        hFML, pFmlReq->ulBlock));

    DclProfilerEnter("FfxFmlNandGetRawBlockInfo", 0, 0);

    /*  Iterate through the layout structures until we find the
        one which contains the block we want to access.
    */
    pLayout = hFML->pLayout;
    while(ulBlock >= pLayout->ulRawBlockCount)
    {
        ulOffset += pLayout->ulRawBlockCount;
        ulBlock -= pLayout->ulRawBlockCount;
        pLayout = pLayout->pNext;
        DclAssert(pLayout);
    }
    
    DevReq.ior.ulReqLen     = sizeof(DevReq);
    DevReq.ior.ioFunc       = FXIOFUNC_DEV_GET_RAW_BLOCK_INFO;
    DevReq.ulBlock          = ulBlock + pLayout->ulRawBlockOffset;
    
    ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

    pFmlReq->ulBlockInfo = DevReq.ulBlockInfo;
    pFmlReq->ulBlockStatus = DevReq.ulBlockStatus;
    pFmlReq->ulRawMapping = DevReq.ulRawMapping + ulOffset;

    /*  Remember that if there is an offset into the device, the
        returned mappings have to be shifted by that offset, including
        the raw mappings.
    */
    pFmlReq->ulRawMapping  -= pLayout->ulBlockOffset;
    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandGetRawBlockInfo() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandRetireRawBlock()

    Retires blocks in the device. The input block number is assumed
    to be a raw block number selected from a range which may or may
    not span device-reserved segments of blocks. Normal device
    remappings do not apply. This function will assert on raw block
    numbers that are not spanned by the FML disk. This is particularly
    dangerous when using the GET_PAGE_INFO  interface to retrieve the
    raw block number of a block that was remapped by BBM, because 
    outside that range. In addition, retiring blocks is a very
    inadvisable thing to do outside of a simulation environment.
    Once retired, a block will no longer be available until BBM is
    reformatted.

    This function should only be used by those who really know
    what they are doing.
    
    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlRetireRawBlock(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_RETIRE_RAW_BLOCK    *pFmlReq)
{
    FFXIOR_DEV_RETIRE_RAW_BLOCK     DevReq = {{0}};
    FFXIOSTATUS                     ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);
    FFXFMLLAYOUT                   *pLayout;
    D_UINT32                        ulBlock = pFmlReq->ulRawBlock;

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandRetireRawBlock() Structure length invalid\n" ));
        return ioStat;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandRetireRawBlock() hFML=%P Block=%lX, Reason=%lX\n",
        hFML, pFmlReq->ulRawBlock, pFmlReq->ulReason));

    DclProfilerEnter("FfxFmlNandRetireRawBlock", 0, 0);

    DevReq.ior.ulReqLen     = sizeof(DevReq);
    DevReq.ior.ioFunc       = FXIOFUNC_DEV_RETIRE_RAW_BLOCK;

    /*  Iterate through the layout structures until we find the
        one which contains the block we want to retire.
    */
    pLayout = hFML->pLayout;
    while(ulBlock >= pLayout->ulRawBlockCount)
    {
        ulBlock -= pLayout->ulRawBlockCount;
        pLayout = pLayout->pNext;
        DclAssert(pLayout);
    }
    DevReq.ulRawBlock  = ulBlock + hFML->pLayout->ulRawBlockOffset;
    DevReq.ulPageCount = pFmlReq->ulPageCount;
    DevReq.ulReason    = pFmlReq->ulReason;

    ioStat = FfxDevIORequest(hFML->pLayout->hFimDev, &DevReq.ior);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandRetireRawBlock() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}

#endif


/*-------------------------------------------------------------------
    Private: FfxFmlNandGetBlockStatus()

    Gets the block status of a normally-mapped FML block. Normal
    device remappings apply.
    
    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandGetBlockStatus(
    FFXFMLHANDLE                        hFML,
    FFXIOR_FML_GET_BLOCK_STATUS        *pFmlReq)
{
    FFXIOSTATUS                         ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandGetBlockStatus() Structure length invalid\n" ));
        return ioStat;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandGetBlockStatus() hFML=%P Block=%lX\n",
        hFML, pFmlReq->ulBlock));

    DclProfilerEnter("FfxFmlNandGetBlockStatus", 0, 0);

    if(pFmlReq->ulBlock >= hFML->ulTotalBlocks)
    {
        ioStat.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOR_DEV_GET_BLOCK_STATUS DevReq = {{0}};
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulBlock = pFmlReq->ulBlock;

        /*  Iterate through the layout structures until we find the
            one which contains the first block we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulBlock >= pLayout->ulBlockCount)
        {
            ulBlock -= pLayout->ulBlockCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen     = sizeof DevReq;
        DevReq.ior.ioFunc       = FXIOFUNC_DEV_GET_BLOCK_STATUS;
        DevReq.ulBlock          = ulBlock + pLayout->ulBlockOffset;

        ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandGetBlockStatus() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandGetRawBlockStatus()

    Gets the block status of the selected raw FML block. The block is
    assumed to be a raw FML block number and must exist in the raw
    range spanned by the FML disk. This is particularly dangerous when
    using the GET_PAGE_INFO interface to retrieve the raw block number
    of a block that was remapped by BBM, because the BBM table where
    the substitute block is located may reside outside that range.

    This function should only be used by those who really know
    what they are doing.

    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandGetRawBlockStatus(
    FFXFMLHANDLE                        hFML,
    FFXIOR_FML_GET_RAW_BLOCK_STATUS    *pFmlReq)
{
    FFXIOSTATUS                         ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandGetRawBlockStatus() Structure length invalid\n" ));
        return ioStat;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandGetRawBlockStatus() hFML=%P Block=%lX\n",
        hFML, pFmlReq->ulBlock));

    DclProfilerEnter("FfxFmlNandGetRawBlockStatus", 0, 0);

    {
        FFXIOR_DEV_GET_RAW_BLOCK_STATUS DevReq = {{0}};
        FFXFMLLAYOUT                   *pLayout;
        D_UINT32                        ulBlock = pFmlReq->ulBlock;

        /*  Iterate through the layout structures until we find the
            one which contains the first block we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulBlock >= pLayout->ulRawBlockCount)
        {
            ulBlock -= pLayout->ulRawBlockCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen     = sizeof DevReq;
        DevReq.ior.ioFunc       = FXIOFUNC_DEV_GET_RAW_BLOCK_STATUS;
        DevReq.ulBlock          = ulBlock + pLayout->ulRawBlockOffset;

        ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandGetRawBlockStatus() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandSetRawBlockStatus()

    Sets the block status of the selected raw FML block. The block is
    assumed to be a raw FML block number and must exist in the raw
    range spanned by the FML disk. This is particularly dangerous when
    using the GET_PAGE_INFO interface to retrieve the raw block number
    of a block that was remapped by BBM, because the BBM table where
    the substitute block is located may reside outside that range.

    Since setting block status was originally used for block 
    retirement, and is no longer used for that purpose, this
    interface may soon be deprecated.
    
    This function should only be used by those who really know
    what they are doing.

    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandSetRawBlockStatus(
    FFXFMLHANDLE                        hFML,
    FFXIOR_FML_SET_RAW_BLOCK_STATUS    *pFmlReq)
{
    FFXIOSTATUS                         ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandSetRawBlockStatus() Structure length invalid\n" ));
        return ioStat;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandSetRawBlockStatus() hFML=%P Block=%lX BlockStat=%lX\n",
        hFML, pFmlReq->ulBlock, pFmlReq->ulBlockStatus));

    DclProfilerEnter("FfxFmlNandSetRawBlockStatus", 0, 0);

    {
        FFXIOR_DEV_SET_RAW_BLOCK_STATUS DevReq = {{0}};
        FFXFMLLAYOUT                   *pLayout;
        D_UINT32                        ulBlock = pFmlReq->ulBlock;

        /*  Iterate through the layout structures until we find the
            one which contains the first block we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulBlock >= pLayout->ulRawBlockCount)
        {
            ulBlock -= pLayout->ulRawBlockCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen     = sizeof DevReq;
        DevReq.ior.ioFunc       = FXIOFUNC_DEV_SET_RAW_BLOCK_STATUS;
        DevReq.ulBlock          = ulBlock + pLayout->ulRawBlockOffset;
        DevReq.ulBlockStatus    = pFmlReq->ulBlockStatus;

        ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandSetRawBlockStatus() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNandGetRawPageStatus()

    Gets the page status of the selected raw FML page. The page is
    assumed to be in a raw FML block number and must exist in the raw
    range spanned by the FML disk. This is particularly dangerous when
    using the GET_PAGE_INFO interface to retrieve the raw block number
    of a block that was remapped by BBM, because the BBM table where
    the substitute block is located may reside outside that range.

    This function should only be used by those who really know
    what they are doing.

    Parameters:
        hFML      - handle of the FML disk for this request.
        pFmlReq   - pointer to the request structure containing
                    necessary parameters for this requiest.

    Return Value:
        An FXIOSTATUS structure containing the results of the
        request in addition to any necessary block or page status
        and count of successful operations.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNandGetRawPageStatus(
    FFXFMLHANDLE                        hFML,
    FFXIOR_FML_GET_RAW_PAGE_STATUS     *pFmlReq)
{
    FFXIOSTATUS                         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFmlNandGetRawPageStatus() Structure length invalid\n" ));
        return ioStat;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNandGetRawPageStatus() hFML=%P Page=%lX\n",
        hFML, pFmlReq->ulPage));

    DclProfilerEnter("FfxFmlNandGetRawPageStatus", 0, 0);

    {
        FFXIOR_DEV_GET_RAW_PAGE_STATUS  DevReq = {{0}};
        FFXFMLLAYOUT                   *pLayout;
        D_UINT32                        ulPage = pFmlReq->ulPage;

        /*  Iterate through the layout structures until we find the
            one which contains the first block we want to access.
        */
        pLayout = hFML->pLayout;

        while(ulPage >= pLayout->ulRawPageCount)
        {
            ulPage -= pLayout->ulRawPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DevReq.ior.ulReqLen     = sizeof DevReq;
        DevReq.ior.ioFunc       = FXIOFUNC_DEV_GET_RAW_PAGE_STATUS;
        DevReq.ulPage           = ulPage + pLayout->ulRawPageOffset;

        ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNandGetRawPageStatus() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}

#endif



