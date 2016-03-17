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

    This module contains NAND specific Device Manager functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: devnand.c $
    Revision 1.21  2009/08/04 17:59:42Z  garyp
    Merged from the v4.0 branch.  Updated to work with the new FfxDevIOProlog()
    function which now returns a meaningful FFXSTATUS error value, rather 
    than a D_BOOL.
    Revision 1.20  2009/08/04 16:27:01Z  keithg
    Corrected remaining erroneous range checking uncovered by bug 2818.
    Revision 1.19  2009/08/04 06:47:48Z  keithg
    Fixed bug 2818 - Corrected assert range check that was not accounting
    for reserved space in FfxDevNandGetBlockInfo().  Moved adjustment for
    reserved space in to complete proper range checking in the function
    FfxDevNandGetRawBlockInfo().
    Revision 1.18  2009/04/22 13:43:48Z  keithg
    Fixed bug 2641, raw access functions now complete range checking.
    Revision 1.17  2009/04/01 18:18:24Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.16  2009/02/23 05:26:14Z  billr
    Resolve bug 2334: BBM ignores read errors during block replacement.
    Resolve bug 2409: Block Replacement does not preserve page status and
    EDC information.
    Revision 1.15  2009/02/19 06:26:51Z  glenns
    Updated FfxDevNandRetireRawBlock to initialize ioStat, preventing possible
    use of an uninitialized variable.
    Revision 1.14  2009/02/19 04:55:49Z  keithg
    Added support for the block retirement to retry until success or no more
    reserved blocks are available for use.
    Revision 1.13  2009/02/14 00:00:15Z  glenns
    Fixed math typo introduced in Rev. 1.10
    Revision 1.12  2009/02/13 00:44:26Z  keithg
    Completed changes to fix bug 2394 - reserved space.
    Revision 1.11  2009/02/12 22:47:33Z  keithg
    Fixes for Bug 2394 to properly handle reserved space.
    Revision 1.10  2009/02/12 07:36:05Z  keithg
    Fixed bug 2394 - The device manager now maintains a private member
    to describe reserved space and does not use the obsolete FIM/NTM
    reserved space attributes.
    Revision 1.9  2009/01/18 08:43:26Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.8  2009/01/12 21:38:52Z  michaelm
    RetireRawBlock now copies up to and including PagesPerBlock pages
    Revision 1.7  2008/12/18 08:40:44Z  keithg
    Conditioned some BBM specific code in GetRawBlockStatus to build
    and link when BBMSUPPORT is disabled.
    Revision 1.6  2008/12/18 05:16:58Z  keithg
    Added RetireRawBlock API, removed unused functions, corrected
    functionality of querying and mapping raw blocks.
    Revision 1.5  2008/12/12 07:29:08Z  keithg
    Re-added set block status functionality; Fixes to block/page info functions.
    Revision 1.4  2008/12/02 23:08:49Z  keithg
    Removed unnecessary include and unused local.
    Revision 1.3  2008/10/24 04:47:12Z  keithg
    Added device get block and page info functions.
    Revision 1.2  2008/09/02 05:01:44Z  keithg
    Raw block status functionality is no longer dependant upon the
    inclusion of BBM functionality.
    Revision 1.1  2008/01/27 00:26:58Z  Garyp
    Initial revision
 ---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdevapi.h>
#include <fimdev.h>
#include "devmgr.h"
#include "devbbm.h"

#if FFXCONF_NANDSUPPORT

#if FFXCONF_BBMSUPPORT

/*-------------------------------------------------------------------
    Private: FfxDevNandGetBlockInfo()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS  FfxDevNandGetBlockInfo(
    FFXFIMDEVHANDLE                 hFimDev,
    FFXIOR_DEV_GET_BLOCK_INFO      *pDevReq)
{
    D_UINT32                        ulCount;
    FFXIOSTATUS                     ioStat;
    FFXIOR_DEV_GET_RAW_BLOCK_INFO   req;

    DclMemSet(&req, 0, sizeof(req));
    req.ior.ulReqLen = sizeof(req);
    req.ior.ioFunc = FXIOFUNC_DEV_GET_RAW_BLOCK_INFO;

    ulCount = 1;
    pDevReq->ulRawMapping  = pDevReq->ulBlock;
    pDevReq->ulRawMapping += hFimDev->ulReservedBlocks;
    DclAssert(pDevReq->ulRawMapping < (hFimDev->ulRawBlocks + hFimDev->ulReservedBlocks));

    FfxDevBbmRemapBlock(hFimDev, &pDevReq->ulRawMapping, &ulCount);
    pDevReq->ulRawMapping -= hFimDev->ulReservedBlocks;
    DclAssert(pDevReq->ulRawMapping < hFimDev->ulRawBlocks);

    req.ulBlock = pDevReq->ulRawMapping;
    ioStat = FfxDevNandGetRawBlockInfo(hFimDev, &req);

    /*  Copy the returned block information, adjust out reserved blocks
    */
    pDevReq->ulBlockInfo = req.ulBlockInfo;
    pDevReq->ulBlockStatus = req.ulBlockStatus;

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: FfxDevNandGetRawBlockInfo()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS  FfxDevNandGetRawBlockInfo(
    FFXFIMDEVHANDLE                     hFimDev,
    FFXIOR_DEV_GET_RAW_BLOCK_INFO      *pDevReq)
{
    FFXIOSTATUS                         ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_OUTOFRANGE);
    FFXIOR_FIM_GET_BLOCK_STATUS         req;

    DclAssert(pDevReq);
    DclAssert(pDevReq->ior.ulReqLen == sizeof(*pDevReq));

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEINDENT),
        "FfxDevNandGetRawBlockInfo() hFimDev=%P Block=%lX\n", hFimDev, pDevReq->ulBlock));

    DclProfilerEnter("FfxDevNandGetRawBlockInfo", 0, 0);

    if(pDevReq->ulBlock < hFimDev->ulRawBlocks)
    {
        ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_READS);
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        {
            /*  Account for reserved blocks.

                Note: It seems like a bad practice to temporarily modify
                      the ulBlock field in the calling structure, then
                      restore it on exit.
            */
            pDevReq->ulBlock += hFimDev->ulReservedBlocks;

            FfxDevBbmQueryRawBlock(hFimDev, pDevReq);

            req.ior.ioFunc = FXIOFUNC_FIM_GET_BLOCK_STATUS;
            req.ior.ulReqLen = sizeof(req);
            req.ulBlock = pDevReq->ulBlock;

            ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &req.ior);
            pDevReq->ulBlockStatus = ioStat.op.ulBlockStatus;

            /*  Adjust the reserved blocks out
            */
            pDevReq->ulBlock -= hFimDev->ulReservedBlocks;
            pDevReq->ulRawMapping -= hFimDev->ulReservedBlocks;

            FfxDevIOEpilog(hFimDev, ACCESS_READS);
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEUNDENT),
        "FfxDevNandGetBlockInfo() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: FfxDevNandRetireRawBlock()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS  FfxDevNandRetireRawBlock(
    FFXFIMDEVHANDLE                     hFimDev,
    FFXIOR_DEV_RETIRE_RAW_BLOCK        *pDevReq)
{
    FFXIOSTATUS                         ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADPARAMETER);

    if(pDevReq->ulRawBlock >= hFimDev->ulRawBlocks
        || pDevReq->ulPageCount > hFimDev->pFimInfo->uPagesPerBlock)
    {
        ioStat.ffxStat = FFXSTAT_OUTOFRANGE;
        goto Cleanup;
    }

    if(pDevReq->ulReason != BLOCKINFO_RETIRED)
    {
        ioStat.ffxStat = FFXSTAT_BADPARAMETER;
        goto Cleanup;
    }

    ioStat = FfxDevBbmRelocateBlock(hFimDev,
                pDevReq->ulRawBlock + hFimDev->ulReservedBlocks,
                pDevReq->ulPageCount);

  Cleanup:

    return ioStat;
}

#endif


/*-------------------------------------------------------------------
    Private: FfxDevNandGetBlockStatus()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS  FfxDevNandGetBlockStatus(
    FFXFIMDEVHANDLE                     hFimDev,
    FFXIOR_DEV_GET_BLOCK_STATUS        *pDevReq)
{
    FFXIOSTATUS                         ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFimDev);
    DclAssert(pDevReq);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEINDENT),
        "FfxDevNandGetBlockStatus() hFimDev=%P Block=%lX\n",
        hFimDev, pDevReq->ulBlock));

    DclProfilerEnter("FfxDevNandGetBlockStatus", 0, 0);

    if(pDevReq->ior.ulReqLen == sizeof *pDevReq)
    {
        ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_READS);
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        {
            FFXIOR_FIM_GET_BLOCK_STATUS FimReq;

            FimReq.ior.ioFunc   = FXIOFUNC_FIM_GET_BLOCK_STATUS;
            FimReq.ior.ulReqLen = sizeof FimReq;
            FimReq.ulBlock      = pDevReq->ulBlock + hFimDev->ulReservedBlocks;

          #if FFXCONF_BBMSUPPORT
            {
                D_UINT32 ulCount = 1;
                FfxDevBbmRemapBlock(hFimDev, &FimReq.ulBlock, &ulCount);
            }
            DclAssert(FimReq.ulBlock < (hFimDev->ulRawBlocks + hFimDev->ulReservedBlocks));
          #endif

            ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &FimReq.ior);

            FfxDevIOEpilog(hFimDev, ACCESS_READS);
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEUNDENT),
        "FfxDevNandGetBlockStatus() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: FfxDevNandGetRawBlockStatus()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS  FfxDevNandGetRawBlockStatus(
    FFXFIMDEVHANDLE                     hFimDev,
    FFXIOR_DEV_GET_RAW_BLOCK_STATUS    *pDevReq)
{
    FFXIOSTATUS                         ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFimDev);
    DclAssert(pDevReq);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEINDENT),
        "FfxDevNandGetRawBlockStatus() hFimDev=%P Block=%lX\n",
        hFimDev, pDevReq->ulBlock));

    DclProfilerEnter("FfxDevNandGetRawBlockStatus", 0, 0);

    if(pDevReq->ior.ulReqLen == sizeof *pDevReq)
    {
        if(pDevReq->ulBlock >= hFimDev->ulRawBlocks)
        {
            ioStat.ffxStat = FFXSTAT_OUTOFRANGE;
        }
        else
		{
            ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_READS);
            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            {
                FFXIOR_FIM_GET_BLOCK_STATUS     FimReq;

                FimReq.ior.ioFunc   = FXIOFUNC_FIM_GET_BLOCK_STATUS;
                FimReq.ior.ulReqLen = sizeof FimReq;
                FimReq.ulBlock      = pDevReq->ulBlock + hFimDev->ulReservedBlocks;

                ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &FimReq.ior);

                FfxDevIOEpilog(hFimDev, ACCESS_READS);
            }
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEUNDENT),
        "FfxDevNandGetRawBlockStatus() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: FfxDevNandSetRawBlockStatus()

    Parameters:

    Return Value:
 -------------------------------------------------------------------*/
FFXIOSTATUS  FfxDevNandSetRawBlockStatus(
    FFXFIMDEVHANDLE                     hFimDev,
    FFXIOR_DEV_SET_RAW_BLOCK_STATUS    *pDevReq)
{
    FFXIOSTATUS ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFimDev);
    DclAssert(pDevReq);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEINDENT),
        "FfxDevNandSetRawBlockStatus() hFimDev=%P Block=%lX BlockStat=%lX\n",
        hFimDev, pDevReq->ulBlock, pDevReq->ulBlockStatus));

    DclProfilerEnter("FfxDevNandSetRawBlockStatus", 0, 0);

    if(pDevReq->ior.ulReqLen == sizeof *pDevReq)
    {
        if(pDevReq->ulBlock >= hFimDev->ulRawBlocks)
        {
            ioStat.ffxStat = FFXSTAT_OUTOFRANGE;
        }
  		else
		{
            ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_WRITES);
            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            {
                FFXIOR_FIM_SET_BLOCK_STATUS    FimReq;

                FimReq.ior.ioFunc       = FXIOFUNC_FIM_SET_BLOCK_STATUS;
                FimReq.ior.ulReqLen     = sizeof FimReq;
                FimReq.ulBlock          = pDevReq->ulBlock + hFimDev->ulReservedBlocks;
                FimReq.ulBlockStatus    = pDevReq->ulBlockStatus;

                ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &FimReq.ior);

                FfxDevIOEpilog(hFimDev, ACCESS_WRITES);
            }
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEUNDENT),
        "FfxDevNandSetRawBlockStatus() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: FfxDevNandGetRawPageStatus()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS  FfxDevNandGetRawPageStatus(
    FFXFIMDEVHANDLE                     hFimDev,
    FFXIOR_DEV_GET_RAW_PAGE_STATUS     *pDevReq)
{
    FFXIOSTATUS                         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFimDev);
    DclAssert(pDevReq);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEINDENT),
        "FfxDevNandGetRawPageStatus() hFimDev=%P Page=%lX\n",
        hFimDev, pDevReq->ulPage));

    DclProfilerEnter("FfxDevNandGetRawPageStatus", 0, 0);

    if(pDevReq->ior.ulReqLen == sizeof *pDevReq)
    {
        if(pDevReq->ulPage >= hFimDev->ulRawTotalPages)
        {
            ioStat.ffxStat = FFXSTAT_OUTOFRANGE;
        }
        else
        {
            ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_READS);
            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            {
                FFXIOR_FIM_GET_PAGE_STATUS      FimReq;

                FimReq.ior.ioFunc   = FXIOFUNC_FIM_GET_PAGE_STATUS;
                FimReq.ior.ulReqLen = sizeof FimReq;
                FimReq.ulPage       = pDevReq->ulPage + (hFimDev->ulReservedBlocks * hFimDev->pFimInfo->uPagesPerBlock);

                ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &FimReq.ior);

                FfxDevIOEpilog(hFimDev, ACCESS_READS);
            }
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEUNDENT),
        "FfxDevNandGetRawPageStatus() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;

}


#endif


