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

    This module implements functions for block locking and unlocking at the
    FML level.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmllock.c $
    Revision 1.4  2010/07/06 01:24:09Z  garyp
    Minor debug code / datatype updates -- no functional changes.
    Revision 1.3  2009/11/25 21:18:19Z  garyp
    Enhanced the UnlockChipBlocks() function to accommodate Disks
    which span multiple Devices.
    Revision 1.2  2009/03/19 00:10:09Z  garyp
    Merged from the v4.0 branch.
    Revision 1.1.1.6  2009/03/19 00:10:09Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.1.1.5  2009/02/18 04:26:08Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.1.1.4  2008/12/17 01:18:54Z  garyp
    Major update to support the concept of a Disk which spans multiple Devices.
    Revision 1.1.1.3  2008/10/23 21:50:52Z  garyp
    Enhanced the lock operation to use an inverted unlock command, if that
    is available, and better meets the request.
    Revision 1.1.1.2  2008/10/22 19:36:34Z  garyp
    Split the lock-freeze functionality out so it is separate from the lock
    function.  Updated so the fInvert option works if unlocking at the Device
    level.
    Revision 1.1  2008/09/05 22:56:56Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdevapi.h>
#include <fxfmlapi.h>
#include "fml.h"


static FFXIOSTATUS  UnlockChipBlocks(FFXFMLHANDLE hFML, FFXIOR_FML_UNLOCK_BLOCKS *pFmlReq);
static FFXSTATUS    LockUnlockWholeDevice(FFXFMLLAYOUT *pLayout, D_BOOL fUnlock);


/*-------------------------------------------------------------------
    Private: FfxFmlBlockLockFreeze()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlBlockLockFreeze(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_LOCK_FREEZE         *pFmlReq)
{
    FFXIOR_DEV_LOCK_FREEZE          DevReq;
    FFXIOSTATUS                     ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);
    FFXFMLLAYOUT                   *pLayout;

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        DclError();
        return ioStat;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlBlockLockFreeze() hFML=%P\n", hFML));

    DclProfilerEnter("FfxFmlBlockLockFreeze", 0, 0);

    /*  These structures are the same for now.
    */
    DevReq = *pFmlReq;
    DevReq.ior.ioFunc = FXIOFUNC_DEV_LOCK_FREEZE;

    pLayout = hFML->pLayout;
    while(pLayout)
    {
        ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        pLayout = pLayout->pNext;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlBlockLockFreeze() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: FfxFmlBlockLock()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlBlockLock(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_LOCK_BLOCKS         *pFmlReq)
{
    FFXIOSTATUS                     ioTotal = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        DclError();
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlBlockLock() hFML=%P StartBlock=%lX Count=%lX\n",
        hFML, pFmlReq->ulStartBlock, pFmlReq->ulBlockCount));

    DclProfilerEnter("FfxFmlBlockLock", 0, pFmlReq->ulBlockCount);

    DclAssert(pFmlReq->ulBlockCount);

#if 0

At this point, unlike the unlock functionality, we need to do nothing
special in the event that the hardware locks things at the chip level
and does not support locking a range of blocks.  The user is responsible
for detecting and knowing this.

    if(hFML->uLockFlags & FFXLOCKFLAGS_LOCKALLBLOCKS)
    {
        ioTotal = LockChipBlocks(hFML, pFmlReq);
    }
    else
#endif
    {
        /*  Validate the range
        */
        if((pFmlReq->ulStartBlock >= hFML->ulTotalBlocks) ||
            (pFmlReq->ulBlockCount > hFML->ulTotalBlocks - pFmlReq->ulStartBlock))
        {
            ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
        }
        else
        {
            FFXIOSTATUS     ioStat;
            FFXFMLLAYOUT   *pLayout;
            D_UINT32        ulBlock = pFmlReq->ulStartBlock;
            D_UINT32        ulBlocksRemaining = pFmlReq->ulBlockCount;

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

            while(ulBlocksRemaining)
            {
                D_UINT32    ulRequested;

                /*  If a lock command applies to the entire Device, however the
                    inverted unlock command is supported, use that instead to
                    effectively lock just the range we want -- everything else
                    will become unlocked.
                */
                if( (hFML->uLockFlags & FFXLOCKFLAGS_LOCKALLBLOCKS) &&
                    (hFML->uLockFlags & FFXLOCKFLAGS_UNLOCKINVERTEDRANGE) )
                {
                    FFXIOR_DEV_UNLOCK_BLOCKS    DevReq;

                    DclMemSet(&DevReq, 0, sizeof DevReq);

                    DevReq.ior.ulReqLen = sizeof DevReq;
                    DevReq.ior.ioFunc   = FXIOFUNC_DEV_UNLOCK_BLOCKS;
                    DevReq.ulStartBlock = ulBlock + pLayout->ulBlockOffset;
                    DevReq.ulBlockCount = DCLMIN(pLayout->ulBlockCount - ulBlock, ulBlocksRemaining);
                    DevReq.fInvert      = TRUE;

                    ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

                    ulRequested = DevReq.ulBlockCount;
                }
                else
                {
                    FFXIOR_DEV_LOCK_BLOCKS      DevReq;

                    DclMemSet(&DevReq, 0, sizeof DevReq);

                    DevReq.ior.ioFunc   = FXIOFUNC_DEV_LOCK_BLOCKS;
                    DevReq.ior.ulReqLen = sizeof DevReq;
                    DevReq.ulStartBlock = ulBlock + pLayout->ulBlockOffset;
                    DevReq.ulBlockCount = DCLMIN(pLayout->ulBlockCount - ulBlock, ulBlocksRemaining);

                    ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

                    ulRequested = DevReq.ulBlockCount;
                }

                ioTotal.ulCount          += ioStat.ulCount;
                ioTotal.ffxStat           = ioStat.ffxStat;
                ioTotal.ulFlags           = ioStat.ulFlags;
                ioTotal.op.ulBlockStatus |= ioStat.op.ulBlockStatus;

                ulBlocksRemaining        -= ioStat.ulCount;

                if(!ulBlocksRemaining || !IOSUCCESS(ioStat, ulRequested))
                    break;

                pLayout = pLayout->pNext;
                DclAssert(pLayout);

                /*  Accesses in subsequent Devices will always be at offset 0
                */
                ulBlock = 0;
            }
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlBlockLock() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlBlockUnlock()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlBlockUnlock(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_UNLOCK_BLOCKS       *pFmlReq)
{
    FFXIOSTATUS                     ioTotal = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        DclError();
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlBlockUnlock() hFML=%P StartBlock=%lX Count=%lX Invert=%u\n",
        hFML, pFmlReq->ulStartBlock, pFmlReq->ulBlockCount, pFmlReq->fInvert));

    DclProfilerEnter("FfxFmlBlockUnlock", 0, pFmlReq->ulBlockCount);

    DclAssert(pFmlReq->ulBlockCount);

    /*  If unlocking operations affect all the flash, we must take special
        measures to deal with BBM.
    */
    if(hFML->uLockFlags & (FFXLOCKFLAGS_UNLOCKSINGLERANGE | FFXLOCKFLAGS_UNLOCKINVERTEDRANGE))
    {
        ioTotal = UnlockChipBlocks(hFML, pFmlReq);
    }
    else
    {
        FFXIOR_DEV_UNLOCK_BLOCKS    DevReq;
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulBlock = pFmlReq->ulStartBlock;
        D_UINT32                    ulBlocksRemaining = pFmlReq->ulBlockCount;

        /*  Validate the range
        */
        if((pFmlReq->ulStartBlock >= hFML->ulTotalBlocks) ||
            (pFmlReq->ulBlockCount > hFML->ulTotalBlocks - pFmlReq->ulStartBlock))
        {
            ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
            goto UnlockCleanup;
        }

        /*  We do not currently handle the fInvert option when operating
            in a Disk oriented mode.  (And in any case, the invert flag
            should only be used when the FFXLOCKFLAGS_UNLOCKINVERTEDRANGE
            flag is specified, and that is handled above.)
        */
        DclAssert(!pFmlReq->fInvert);

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

        DclMemSet(&DevReq, 0, sizeof DevReq);

        DevReq.ior.ioFunc = FXIOFUNC_DEV_UNLOCK_BLOCKS;
        DevReq.ior.ulReqLen = sizeof DevReq;

        while(ulBlocksRemaining)
        {
            FFXIOSTATUS ioStat;

            DevReq.ulStartBlock = ulBlock + pLayout->ulBlockOffset;
            DevReq.ulBlockCount = DCLMIN(pLayout->ulBlockCount - ulBlock, ulBlocksRemaining);

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

  UnlockCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlBlockUnlock() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Local: UnlockChipBlocks()

    This function performs unlocking from a Device/Chip perspective,
    rather than a Disk perspective.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS UnlockChipBlocks(
    FFXFMLHANDLE                hFML,
    FFXIOR_FML_UNLOCK_BLOCKS   *pFmlReq)
{
    FFXIOR_DEV_UNLOCK_BLOCKS    DevReq = {{0}};
    FFXIOSTATUS                 ioTotal = DEFAULT_BLOCKIO_STATUS;
    FFXFMLLAYOUT               *pLayout;
    D_UINT32                    ulBlock;
    D_UINT32                    ulBlocksRemaining;

    DclAssert(hFML);
    DclAssert(pFmlReq);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFml:UnlockChipBlocks() hFML=%P StartBlock=%lX Count=%lX Invert=%u\n",
        hFML, pFmlReq->ulStartBlock, pFmlReq->ulBlockCount, pFmlReq->fInvert));

    /*  Validate the range.  It must be valid relative to the Disk definition,
        even though we're going to physically affect every Device upon which
        this Disk resides (irrespective of the actual range).
    */
    if((pFmlReq->ulStartBlock >= hFML->ulTotalBlocks) ||
        (pFmlReq->ulBlockCount > hFML->ulTotalBlocks - pFmlReq->ulStartBlock))
    {
        ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
        goto ChipUnlockCleanup;
    }

    /*  Cannot use the "invert" flag if it is not a supported feature
    */
    if(pFmlReq->fInvert && !(hFML->uLockFlags & FFXLOCKFLAGS_UNLOCKINVERTEDRANGE))
    {
        ioTotal.ffxStat = FFXSTAT_BADPARAMETER;
        goto ChipUnlockCleanup;
    }

    ulBlock = pFmlReq->ulStartBlock;
    ulBlocksRemaining = pFmlReq->ulBlockCount;

    /*  Iterate through the layout structures until we find the one which
        contains the first block we want to access.  Any Devices which
        fully precede the desired range must have their lock states 
        adjusted according to the request.
    */
    pLayout = hFML->pLayout;
    while(ulBlock >= pLayout->ulBlockCount)
    {
        ioTotal.ffxStat = LockUnlockWholeDevice(pLayout, pFmlReq->fInvert);
        if(ioTotal.ffxStat != FFXSTAT_SUCCESS)
            goto ChipUnlockCleanup;
        
        ulBlock -= pLayout->ulBlockCount;
        pLayout = pLayout->pNext;
        DclAssert(pLayout);
    }

    DevReq.ior.ioFunc = FXIOFUNC_DEV_UNLOCK_BLOCKS;
    DevReq.ior.ulReqLen = sizeof DevReq;

    while(ulBlocksRemaining)
    {
        FFXFIMDEVINFO   di;
        FFXIOSTATUS     ioStat;
        D_UINT32        ulOriginalCount;

        if(!FfxDevInfo(pLayout->hFimDev, &di))
        {
            ioTotal.ffxStat = FFXSTAT_DEVMGR_INFOFAILED;
            goto ChipUnlockCleanup;
        }

        DevReq.ulStartBlock = ulBlock + pLayout->ulBlockOffset;
        DevReq.ulBlockCount = DCLMIN(pLayout->ulBlockCount - ulBlock, ulBlocksRemaining);

        ulOriginalCount = DevReq.ulBlockCount;

        if(di.ulTotalBlocks == di.ulRawBlocks)
        {
            /*  Reality check, if "total" and "raw" block counts are the
                same, BBM can't be used.
            */
            DclAssert(!(di.uDeviceFlags & DEV_USING_BBM));
        }
        else
        {
            /*  Reality check, if "total" and "raw" block counts are not the
                same, it must be because BBM is being used.
            */
            DclAssert(di.uDeviceFlags & DEV_USING_BBM);

            if(pFmlReq->fInvert)
            {
                DevReq.fInvert = TRUE;
                
                /*  If we are inverting, it must be done in such a fashion
                    that BBM remains unlocked <if> there are any other
                    unlocked blocks in the Device.

                    In other words, if there is any writeable block in the main
                    part of the Device, then all of BBM must be writeable.  If
                    all the main part of the Device is write-protected, then
                    any, all, or none of BBM may be write-protected.
                */
                if((DevReq.ulStartBlock > 0) &&
                    (DevReq.ulStartBlock + DevReq.ulBlockCount > di.ulTotalBlocks))
                {
                    ioTotal.ffxStat = FFXSTAT_FML_UNSUPPORTEDLOCKRANGE;
                    goto ChipUnlockCleanup;
                }
            }
            else
            {
                /*  If BBM is being used, and we are unlocking "some" of the
                    Device, we must ensure that the BBM area is unlocked as
                    well so that it can operate properly.  Since the locking
                    capabilities have already been determined to be chip-wide,
                    we must simply extend the count so that it includes the
                    BBM area.
                */
                DevReq.ulBlockCount = di.ulRawBlocks - DevReq.ulStartBlock;
            }
        }

        ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

        ioTotal.ulCount          += DCLMIN(ioStat.ulCount, ulOriginalCount);
        ioTotal.ffxStat           = ioStat.ffxStat;
        ioTotal.ulFlags           = ioStat.ulFlags;
        ioTotal.op.ulBlockStatus |= ioStat.op.ulBlockStatus;

        ulBlocksRemaining        -= DCLMIN(ioStat.ulCount, ulOriginalCount);

        if(!IOSUCCESS(ioStat, DevReq.ulBlockCount))
            goto ChipUnlockCleanup;

        pLayout = pLayout->pNext;
        
        DclAssert(pLayout || ulBlocksRemaining == 0);

        /*  Accesses in subsequent Devices will always be at offset 0
        */
        ulBlock = 0;
    }

    /*  Iterate through any remaining layout structures for this Disk, 
        unlocking any trailing blocks.
    */
    while(pLayout)
    {
        ioTotal.ffxStat = LockUnlockWholeDevice(pLayout, TRUE);
        if(ioTotal.ffxStat != FFXSTAT_SUCCESS)
            goto ChipUnlockCleanup;
        
        pLayout = pLayout->pNext;
    }

  ChipUnlockCleanup:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFml:UnlockChipBlocks() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Local: LockUnlockWholeDevice()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXSTATUS LockUnlockWholeDevice(
    FFXFMLLAYOUT       *pLayout,
    D_BOOL              fUnlock)
{
    FFXIOSTATUS         ioStat;
    FFXFIMDEVINFO       di;

    DclAssertReadPtr(pLayout, sizeof(*pLayout));

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_ALWAYS, 1, 0),
        "FfxFml:LockUnlockWholeDevice() fUnlock=%u\n", fUnlock));

    if(!FfxDevInfo(pLayout->hFimDev, &di))
        return FFXSTAT_DEVMGR_INFOFAILED;
    
    /*  Even if the request does not directly touch any blocks on this
        Device, we must still lock or unlock the Device, since these are
        chip-wide operations, or in the case of Disk which is spanning
        Devices -- virtual chip-wide operations.
    */            
    if(fUnlock)
    {
        FFXIOR_DEV_UNLOCK_BLOCKS    DevReqUnlock = {{0}};
        
        DevReqUnlock.ior.ioFunc   = FXIOFUNC_DEV_UNLOCK_BLOCKS;
        DevReqUnlock.ior.ulReqLen = sizeof DevReqUnlock;
        DevReqUnlock.ulStartBlock = 0;
        DevReqUnlock.ulBlockCount = di.ulRawBlocks;
        DevReqUnlock.fInvert      = FALSE;

        ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReqUnlock.ior);
    }
    else
    {
        FFXIOR_DEV_LOCK_BLOCKS      DevReqLock = {{0}};
        
        DevReqLock.ior.ioFunc   = FXIOFUNC_DEV_LOCK_BLOCKS;
        DevReqLock.ior.ulReqLen = sizeof DevReqLock;
        DevReqLock.ulStartBlock = 0;
        DevReqLock.ulBlockCount = di.ulRawBlocks;

        ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReqLock.ior);
    }

    return ioStat.ffxStat;
}
