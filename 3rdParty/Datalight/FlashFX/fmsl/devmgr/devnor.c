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

    This module contains NOR/Sibley specific Device Manager functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: devnor.c $
    Revision 1.19  2011/12/20 21:33:30Z  johnb
    Fixed to properly handle reserved space.
    Revision 1.18  2009/08/04 19:32:43Z  garyp
    Merged from the v4.0 branch.  Corrected some misleading variable names.
    Updated to work with the new FfxDevIOProlog() and FfxDevEraseProlog()
    functions which now return a meaningful FFXSTATUS error value, rather than
    a D_BOOL.  Updated to use standard initializor macros for FFXIOSTATUS
    where possible.
    Revision 1.17  2009/04/03 20:21:42Z  davidh
    Function header error corrected for AutoDoc.
    Revision 1.16  2009/04/01 20:03:34Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.15  2009/01/16 23:53:20Z  glenns
    - Fixed up literal FFXIOSTATUS initialization in four places.
    Revision 1.14  2008/06/16 22:05:56Z  jimmb
    We definitely require that the pointer be aligned,
    but the control data offset and length do not need to be aligned.
    Revision 1.13  2008/05/27 16:34:45Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.12  2008/01/13 07:28:01Z  keithg
    Function header updates to support autodoc.
    Revision 1.11  2007/11/03 23:49:35Z  Garyp
    Updated to use the standard module header.
    Revision 1.10  2007/10/01 21:47:32Z  pauli
    Resolved Bug 355: Added checks to ensure that data buffers meet
    the alignement requirements of the FIM.
    Revision 1.9  2007/02/08 01:07:40Z  Garyp
    Updated to eliminate a lame syntax error when using the VxWorks 5.5 GNU
    MIPS compiler.
    Revision 1.8  2007/02/01 03:47:38Z  Garyp
    Added boot-block init code which was factored out of devmgr.c.  Cleaned
    up boot-block erase logic to match that used for regular blocks.  Updated
    control read/write to avoid overflow when determining the maximum size
    to process at a time.
    Revision 1.7  2007/01/04 04:07:05Z  Garyp
    Fixed some bogus preprocessor code which could cause the disk to be trashed
    if both Sibley and NOR support are enabled, a NOR FIM is being used, and
    the DevMgr latency settings have been custom configured to only allow very
    small writes.
    Revision 1.6  2006/06/23 15:37:41Z  Garyp
    Corrected profiler macro.  Documentation fixes.
    Revision 1.5  2006/05/20 20:42:35Z  Garyp
    Added support for erasing NOR boot blocks.
    Revision 1.4  2006/03/04 18:02:23Z  Garyp
    Updated debug code.
    Revision 1.3  2006/02/15 04:28:11Z  Garyp
    Added reserved space support for NOR FIMs.
    Revision 1.2  2006/02/10 23:32:38Z  Garyp
    Fixed Sibley support.
    Revision 1.1  2006/02/08 21:48:56Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdevapi.h>
#include <fimdev.h>
#include "devmgr.h"


#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT

#if FFXCONF_ISWFSUPPORT
static D_UINT32 UpdateISWFAddress(FFXFIMDEVHANDLE hFimDev, D_UINT32 ulStart, D_UINT32 ulLength);
#endif


/*-------------------------------------------------------------------
    Private: FfxDevBootBlockInit()

    This function is called by FfxDevCreate() to initialize the
    boot block characteristics (if any).

    Parameters:
        pFIM     - A pointer to the FFXFIMDEVICE structure.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxDevBootBlockInit(
    FFXFIMDEVICE   *pFIM)
{
    DclAssert(pFIM);

    /*  Initialize NOR boot block configuration
    */
    if(pFIM->pFimInfo->uBootBlockCountLow || pFIM->pFimInfo->uBootBlockCountHigh)
    {
        DclAssert(pFIM->pFimInfo->uDeviceType != DEVTYPE_NAND);
        DclAssert(pFIM->pFimInfo->ulBootBlockSize);

        pFIM->ulBBsPerBlock = pFIM->pFimInfo->ulBlockSize / pFIM->pFimInfo->ulBootBlockSize;

        if(pFIM->pFimInfo->uBootBlockCountLow)
        {
            DclAssert((pFIM->pFimInfo->ulBootBlockSize *
                   pFIM->pFimInfo->uBootBlockCountLow) %
                   pFIM->pFimInfo->ulBlockSize == 0);

            pFIM->ulLowBootBlockCount = pFIM->pFimInfo->uBootBlockCountLow;
        }

        if(pFIM->pFimInfo->uBootBlockCountHigh)
        {
            DclAssert((pFIM->pFimInfo->ulBootBlockSize *
                       pFIM->pFimInfo->uBootBlockCountHigh) %
                       pFIM->pFimInfo->ulBlockSize == 0);

            /*  Calculate the starting block at the high end
            */
            pFIM->ulHighBootBlockStart =
                ((pFIM->pFimInfo->ulChipBlocks * pFIM->ulBBsPerBlock) - pFIM->pFimInfo->uBootBlockCountHigh);

            DclAssert(pFIM->ulHighBootBlockStart != D_UINT32_MAX);
        }
        else
        {
            /*  If no high boot blocks, this keeps the on the fly math simple,
                since no block can start this high (yet).
            */
            pFIM->ulHighBootBlockStart = D_UINT32_MAX;
        }
    }
    else
    {
        DclAssert(pFIM->pFimInfo->ulBootBlockSize == 0);
    }

    return;
}


/*-------------------------------------------------------------------
    Private: FfxDevReadControlData()

    This function implements the "Control Read/Write" logic necessary
    when NOR or Sibley (ISWF) is being used.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxDevReadControlData(
    FFXFIMDEVHANDLE                 hFimDev,
    FFXIOR_DEV_READ_CONTROLDATA    *pDevReq)
{
    FFXIOR_FIM_READ_CONTROLDATA     FimReq;
    FFXIOSTATUS                     ioTotal = INITIAL_BYTEIO_STATUS(FFXSTAT_BADSTRUCLEN);
    D_UINT32                        ulRemaining;
    D_UINT32                        ulMaxReadLength;

    DclAssert(hFimDev);
    DclAssert(pDevReq);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEINDENT),
        "FfxDevReadControlData() hFimDev=%P Offset=%lX Length=%lX pData=%P\n",
        hFimDev, pDevReq->ulOffset, pDevReq->ulLength, pDevReq->pData));

    DclProfilerEnter("FfxDevReadControlData", 0, pDevReq->ulLength);

    if(pDevReq->ior.ulReqLen != sizeof *pDevReq)
    {
        DclError();
        goto Cleanup;
    }

    /*  Make sure the data pointer, offset and length are aligned appropriately.
    */
    if(!DCLISALIGNED((D_UINTPTR)pDevReq->pData, hFimDev->pFimInfo->uAlignSize))
    {
        FFXPRINTF(1, ("FfxDevReadControlData() pData is misaligned: %P\n",
                      pDevReq->pData));
        DclError();
        ioTotal.ffxStat = FFXSTAT_DEVMGR_MISALIGNED;
        goto Cleanup;
    }

    ioTotal.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_READS);
    if(ioTotal.ffxStat != FFXSTAT_SUCCESS)
        goto Cleanup;

    /*  Since we are reading a count of bytes rather than pages, calculate
        the maximum we can read in one call, maintaining the same ratio.
    */
    if(hFimDev->ulMaxReadCount > D_UINT32_MAX / hFimDev->pFimInfo->uPageSize)
        ulMaxReadLength = D_UINT32_MAX;
    else
        ulMaxReadLength = hFimDev->ulMaxReadCount * hFimDev->pFimInfo->uPageSize;

    ulRemaining = pDevReq->ulLength;

    /*  For the moment these structures are identical
    */
    FimReq = *pDevReq;
    FimReq.ior.ioFunc = FXIOFUNC_FIM_READ_CONTROLDATA;
    FimReq.ulOffset += (hFimDev->ulReservedBlocks * hFimDev->pFimInfo->ulBlockSize);

    while(ulRemaining)
    {
        FFXIOSTATUS     ioStat;

        /*  Break transfers into "reasonable length" chunks, and allow
            other FIM accesses to preempt this one between them.
        */
        FimReq.ulLength = DCLMIN(ulRemaining, ulMaxReadLength);

        ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &FimReq.ior);

        ioTotal.ulCount         += ioStat.ulCount;
        ioTotal.ffxStat          = ioStat.ffxStat;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

        if(!IOSUCCESS(ioStat, FimReq.ulLength))
            break;

        /*  Reduce the count, and advance the start offset and buffer
            pointers as appropriate.
        */
        ulRemaining         -= ioStat.ulCount;
        if(!ulRemaining)
            break;

        FimReq.pData        += ioStat.ulCount;

      #if FFXCONF_NORSUPPORT && FFXCONF_ISWFSUPPORT
        if(hFimDev->pFimInfo->uDeviceType == DEVTYPE_ISWF)
            FimReq.ulOffset = UpdateISWFAddress(hFimDev, FimReq.ulOffset, ioStat.ulCount);
        else
            FimReq.ulOffset += ioStat.ulCount;
      #elif FFXCONF_ISWFSUPPORT
            FimReq.ulOffset = UpdateISWFAddress(hFimDev, FimReq.ulOffset, ioStat.ulCount);
      #else
            FimReq.ulOffset += ioStat.ulCount;
      #endif

        /*  Release and acquire the mutex, allowing higher-priority
            accesses to preempt.
        */
        DclMutexRelease(hFimDev->pMutex);
        DclMutexAcquire(hFimDev->pMutex);
    }

    FfxDevIOEpilog(hFimDev, ACCESS_READS);

  Cleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEUNDENT),
        "FfxDevReadControlData() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxDevWriteControlData()

    This function implements the "Control Read/Write" logic necessary
    when NOR or Sibley (ISWF) is being used.
    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxDevWriteControlData(
    FFXFIMDEVHANDLE                 hFimDev,
    FFXIOR_DEV_WRITE_CONTROLDATA   *pDevReq)
{
    FFXIOR_FIM_WRITE_CONTROLDATA    FimReq;
    FFXIOSTATUS                     ioTotal = INITIAL_BYTEIO_STATUS(FFXSTAT_BADSTRUCLEN);
    D_UINT32                        ulRemaining;
    D_UINT32                        ulMaxWriteLength;

    DclAssert(hFimDev);
    DclAssert(pDevReq);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEINDENT),
        "FfxDevWriteControlData() hFimDev=%P Offset=%lX Length=%lX pData=%P\n",
        hFimDev, pDevReq->ulOffset, pDevReq->ulLength, pDevReq->pData));

    DclProfilerEnter("FfxDevWriteControlData", 0, pDevReq->ulLength);

    if(pDevReq->ior.ulReqLen != sizeof *pDevReq)
    {
        DclError();
        goto Cleanup;
    }

    /*  Make sure the data pointer, offset and length are aligned appropriately.
    */
    if(!DCLISALIGNED((D_UINTPTR)pDevReq->pData, hFimDev->pFimInfo->uAlignSize))
    {
        FFXPRINTF(1, ("FfxDevWriteControlData() pData is misaligned: %P\n",
                      pDevReq->pData));
        DclError();
        ioTotal.ffxStat = FFXSTAT_DEVMGR_MISALIGNED;
        goto Cleanup;
    }

    ioTotal.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_WRITES);
    if(ioTotal.ffxStat != FFXSTAT_SUCCESS)
        goto Cleanup;

    /*  Since we are writing a count of bytes rather than pages, calculate
        the maximum we can write in one call, maintaining the same ratio.
    */
    if(hFimDev->ulMaxWriteCount > D_UINT32_MAX / hFimDev->pFimInfo->uPageSize)
        ulMaxWriteLength = D_UINT32_MAX;
    else
        ulMaxWriteLength = hFimDev->ulMaxWriteCount * hFimDev->pFimInfo->uPageSize;

    ulRemaining = pDevReq->ulLength;

    /*  For the moment these structures are identical
    */
    FimReq = *pDevReq;
    FimReq.ior.ioFunc = FXIOFUNC_FIM_WRITE_CONTROLDATA;
    FimReq.ulOffset += (hFimDev->ulReservedBlocks * hFimDev->pFimInfo->ulBlockSize);

    while(ulRemaining)
    {
        FFXIOSTATUS     ioStat;

        /*  Break transfers into "reasonable length" chunks, and allow
            other FIM accesses to preempt this one between them.
        */
        FimReq.ulLength = DCLMIN(ulRemaining, ulMaxWriteLength);

        ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &FimReq.ior);

        ioTotal.ulCount         += ioStat.ulCount;
        ioTotal.ffxStat          = ioStat.ffxStat;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

        if(!IOSUCCESS(ioStat, FimReq.ulLength))
            break;

        /*  Reduce the count, and advance the start offset and buffer
            pointers as appropriate.
        */
        ulRemaining         -= ioStat.ulCount;
        if(!ulRemaining)
            break;

        FimReq.pData        += ioStat.ulCount;

      #if FFXCONF_NORSUPPORT && FFXCONF_ISWFSUPPORT
        if(hFimDev->pFimInfo->uDeviceType == DEVTYPE_ISWF)
            FimReq.ulOffset = UpdateISWFAddress(hFimDev, FimReq.ulOffset, ioStat.ulCount);
        else
            FimReq.ulOffset += ioStat.ulCount;
      #elif FFXCONF_ISWFSUPPORT
            FimReq.ulOffset = UpdateISWFAddress(hFimDev, FimReq.ulOffset, ioStat.ulCount);
      #else
            FimReq.ulOffset += ioStat.ulCount;
      #endif

        /*  Release and acquire the mutex, allowing higher-priority
            accesses to preempt.
        */
        DclMutexRelease(hFimDev->pMutex);
        DclMutexAcquire(hFimDev->pMutex);
    }

    FfxDevIOEpilog(hFimDev, ACCESS_WRITES);

  Cleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEUNDENT),
        "FfxDevWriteControlData() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}



#if FFXCONF_ISWFSUPPORT

/*-------------------------------------------------------------------
    Local: UpdateISWFAddress()

    Parameters:
        hFimDev  - The FFXFIMDEVHANDLE
        ulStart  - An address suitable for use as an argument to
                   oemread() or oemwrite().
        ulLength - Length of the request in bytes.  This will always
                   be aligned on 32 bit (4 byte) boundaries.

    Return Value:
        Update the address to one that is suitable for use as an
        argument to oemread() or oemwrite().
-------------------------------------------------------------------*/
static D_UINT32 UpdateISWFAddress(
    FFXFIMDEVHANDLE hFimDev,
    D_UINT32        ulStart,
    D_UINT32        ulLength)
{
    D_UINT32        ulRetAddress = ~(D_UINT32) 0;
    D_UINT32        ulPage = 0;
    D_UINT32        ulOffset = 0;
    D_UINT32        ulControlOffsetMask = 0;
    D_UINT32        ulPageMask = 0;

    DclAssert(hFimDev);

    /* control mode offset mask
    */
    ulControlOffsetMask = (hFimDev->pFimInfo->uPageSize >> 1) - 1;

    /* start address of page mask
    */
    ulPageMask = ~(D_UINT32)(hFimDev->pFimInfo->uPageSize - 1);

    /* calc new address
    */
    ulRetAddress = ulStart + ulLength;

    /* find start of page
    */
    ulPage = ulRetAddress & ulPageMask;

    /* find offset into page
    */
    ulOffset = ulRetAddress & ulControlOffsetMask;

    /* starting address +
       + page if sitting on page boundary +
       offset in range
    */
    ulRetAddress = (ulPage
            + ((ulOffset & ~ulControlOffsetMask) << 1)
            + (ulOffset & ulControlOffsetMask));

    /* double check still in range
    */
    DclAssert(ulRetAddress < hFimDev->pFimInfo->ulTotalBlocks * hFimDev->pFimInfo->ulBlockSize);

    return ulRetAddress;
}

#endif


/*-------------------------------------------------------------------
    Private: FfxDevEraseBootBlocks()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxDevEraseBootBlocks(
    FFXFIMDEVHANDLE                     hFimDev,
    FFXIOR_DEV_ERASE_BOOT_BLOCKS       *pDevReq)
{
    FFXIOSTATUS                         ioTotal = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);
    FFXIOR_FIM_ERASE_BOOT_BLOCK_START   ReqStart;
    D_UINT32                            ulStartBB;
    D_UINT32                            ulChipBB;   /* block within the chip */

    DclAssert(pDevReq);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEINDENT),
        "FfxDevEraseBootBlocks() hFimDev=%P StartBB=%lX Count=%lX\n",
        hFimDev, pDevReq->ulStartBB, pDevReq->ulBBCount));

    DclProfilerEnter("FfxDevEraseBootBlocks", 0, pDevReq->ulBBCount);

    if(pDevReq->ior.ulReqLen != sizeof *pDevReq)
    {
        DclError();
        goto Cleanup;
    }

    /*  Make sure the device actually has boot blocks
    */
    if(!hFimDev->pFimInfo->uBootBlockCountLow && !hFimDev->pFimInfo->uBootBlockCountHigh)
    {
        DclError();
        ioTotal.ffxStat = FFXSTAT_DEVMGR_NOBOOTBLOCKS;
        goto Cleanup;
    }

    DclAssert(hFimDev->pFimInfo->ulChipBlocks);

    /*  Adjust the start boot block for reserved space
    */
    ulStartBB = pDevReq->ulStartBB + (hFimDev->ulReservedBlocks * hFimDev->ulBBsPerBlock);

    /*  Find the block within the chip
    */
    ulChipBB = ulStartBB % (hFimDev->pFimInfo->ulChipBlocks * hFimDev->ulBBsPerBlock);

    if(ulChipBB < hFimDev->ulLowBootBlockCount)
    {
        if(pDevReq->ulBBCount > hFimDev->ulLowBootBlockCount - ulChipBB)
        {
            DclError();
            ioTotal.ffxStat = FFXSTAT_DEVMGR_BOOTBLOCKRANGE;
            goto Cleanup;
        }
    }
    else if(ulChipBB >= hFimDev->ulHighBootBlockStart)
    {
        if(pDevReq->ulBBCount >
            (hFimDev->pFimInfo->ulChipBlocks * hFimDev->ulBBsPerBlock) - ulChipBB)
        {
            DclError();
            ioTotal.ffxStat = FFXSTAT_DEVMGR_BOOTBLOCKRANGE;
            goto Cleanup;
        }
    }
    else
    {
        DclError();
        ioTotal.ffxStat = FFXSTAT_DEVMGR_BOOTBLOCKRANGE;
        goto Cleanup;
    }

    ioTotal.ffxStat = FfxDevEraseProlog(hFimDev);
    if(ioTotal.ffxStat == FFXSTAT_SUCCESS)
    {
        ReqStart.ior.ioFunc     = FXIOFUNC_FIM_ERASE_BOOT_BLOCK_START;
        ReqStart.ior.ulReqLen   = sizeof ReqStart;
        ReqStart.ulStartBB      = ulStartBB;
        ReqStart.ulBBCount      = pDevReq->ulBBCount;

        ioTotal = FfxDevEraseBootBlockSequence(hFimDev, &ReqStart);

        FfxDevEraseEpilog(hFimDev);
    }

  Cleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEUNDENT),
        "FfxDevEraseBootBlocks() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxDevEraseBootBlockSequence()

    This function erases a range of boot blocks, as specified in
    the supplied packet.  Validation that the boot blocks are
    in range must have already been performed.  The "EraseProlog"
    function must have already been called when this function is
    invoked.

    Note that the contents of the supplied request packet WILL be
    modified during the course of this call.

    Parameters:

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of the
        operation.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxDevEraseBootBlockSequence(
    FFXFIMDEVHANDLE                     hFimDev,
    FFXIOR_FIM_ERASE_BOOT_BLOCK_START  *pReq)
{
    FFXIOSTATUS                         ioTotal = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFimDev);
    DclAssert(pReq);
    DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEINDENT),
        "FfxDevEraseBootBlockSequence() hFimDev=%P StartBB=%lX Count=%lX\n",
        hFimDev, pReq->ulStartBB, pReq->ulBBCount));

    while(pReq->ulBBCount)
    {
        FFXIOSTATUS ioStat;

        /*  Attempt to start an erase operation.
        */
        ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &pReq->ior);
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        {
            ioStat = FfxDevErasePoll(hFimDev);
        }

        /*  Accumulate our counts for what was successfully erased.
        */
        ioTotal.ulCount             += ioStat.ulCount;
        ioTotal.op.ulBlockStatus    |= ioStat.op.ulBlockStatus;
        ioTotal.ffxStat              = ioStat.ffxStat;

        /*  Give up if this erase operation failed.
        */
        if((ioStat.ffxStat != FFXSTAT_SUCCESS) || (ioStat.ulCount == 0))
            break;

        /*  The operation succeeded.  The FIM reported how many blocks
            were erased.  Adjust the starting block and count accordingly,
            and go around again to erase any remaining.
        */
        pReq->ulBBCount             -= ioStat.ulCount;
        if(!pReq->ulBBCount)
            break;

        pReq->ulStartBB             += ioStat.ulCount;

#if 0
This is the paradigm we use for full-size erase blocks, but technically we
probably do not need to do this for boot blocks since the latency configuration
settings are set up based on full-sized erase blocks.

      #if FFXCONF_LATENCYREDUCTIONENABLED
        /*  Release and acquire the mutex, allowing higher-priority
            accesses to preempt.
        */
        DclMutexRelease(hFimDev->pMutex);
        DclMutexAcquire(hFimDev->pMutex);
      #endif
#endif
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEUNDENT),
        "FfxDevEraseBootBlockSequence() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxDevIsInBootBlocks()

    This function returns TRUE if the specified block number is at
    the beginning of a range of boot blocks.  The block number MUST
    have already been adjusted for reserved space, if any.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_BOOL FfxDevIsInBootBlocks(
    FFXFIMDEVHANDLE     hFimDev,
    D_UINT32            ulStartBlock)
{
    D_UINT32            ulChipBlock;

    DclAssert(hFimDev);
    DclAssert(hFimDev->pFimInfo->ulChipBlocks);

    /*  Make sure the device actually has boot blocks
    */
    if(!hFimDev->pFimInfo->uBootBlockCountLow && !hFimDev->pFimInfo->uBootBlockCountHigh)
        return FALSE;

    DclAssert(hFimDev->ulBBsPerBlock);

    /*  Make the start block relative to a chip
    */
    ulChipBlock = ulStartBlock % hFimDev->pFimInfo->ulChipBlocks;

    if(ulChipBlock < hFimDev->ulLowBootBlockCount / hFimDev->ulBBsPerBlock)
        return TRUE;

    if(ulChipBlock >= hFimDev->ulHighBootBlockStart / hFimDev->ulBBsPerBlock)
        return TRUE;

    return FALSE;
}





#endif



