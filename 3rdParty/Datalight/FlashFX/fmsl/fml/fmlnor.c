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

    This module contains the handlers for dispatching I/O requests for NOR
    and Sibley (ISWF) flash.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmlnor.c $
    Revision 1.11  2009/08/04 19:41:20Z  garyp
    Merged from the v4.0 branch.  Major update to support the concept of a
    Disk which spans multiple Devices.
    Revision 1.10  2009/04/01 20:25:22Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.9  2009/01/18 04:16:55Z  keithg
    Checked function labels to private.
    Revision 1.8  2009/01/16 23:53:35Z  glenns
    - Fixed up literal FFXIOSTATUS initialization in three places.
    Revision 1.7  2008/01/13 07:36:43Z  keithg
    Function header updates to support autodoc.
    Revision 1.6  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2007/08/02 05:26:26Z  timothyj
    Fixed addition of offset (was incorrectly adding length).
    Revision 1.4  2007/08/01 15:08:06Z  timothyj
    Removed access to obsolete byte offset and length.
    Revision 1.3  2006/11/10 20:21:10Z  Garyp
    Updated to support new statistics gathering capabilities.
    Revision 1.2  2006/05/19 17:01:38Z  Garyp
    Added support for erasing NOR boot blocks.
    Revision 1.1  2006/02/09 02:13:12Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdevapi.h>
#include <fxfmlapi.h>
#include "fml.h"


#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT

/*-------------------------------------------------------------------
    Private: FfxFmlNorReadControlData()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNorReadControlData(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_READ_CONTROLDATA    *pFmlReq)
{
    FFXIOSTATUS                     ioTotal = INITIAL_BYTEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        DclError();
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNorReadControlData() hFML=%P Offset=%lX Length=%lX pData=%P\n",
        hFML, pFmlReq->ulOffset, pFmlReq->ulLength, pFmlReq->pData));

    DclProfilerEnter("FfxFmlNorReadControlData", 0, pFmlReq->ulLength);

    /*  Validate the range
    */
    DclAssert(hFML->ulTotalBlocks > pFmlReq->ulOffset / hFML->ulBlockSize);
    DclAssert(hFML->ulTotalBlocks - pFmlReq->ulOffset / hFML->ulBlockSize >= pFmlReq->ulLength / hFML->ulBlockSize);

  #if FFXCONF_STATS_FML
    hFML->stats.ulControlByteReadRequests++;
    hFML->stats.ulControlByteReadTotal += pFmlReq->ulLength;
    if(hFML->stats.ulControlByteReadMax < pFmlReq->ulLength)
        hFML->stats.ulControlByteReadMax = pFmlReq->ulLength;
  #endif

    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_READ_CONTROLDATA DevReq;
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulOffset = pFmlReq->ulOffset;
        D_UINT32                    ulLengthRemaining = pFmlReq->ulLength;

        /*  Iterate through the layout structures until we find the
            one which contains the first byte we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulOffset >= pLayout->ulBlockCount * hFML->ulBlockSize)
        {
            ulOffset -= pLayout->ulBlockCount * hFML->ulBlockSize;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DclMemSet(&DevReq, 0, sizeof DevReq);

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_READ_CONTROLDATA;
        DevReq.pData        = pFmlReq->pData;

        while(ulLengthRemaining)
        {
            D_UINT32    ulDevOffset = pLayout->ulBlockOffset * hFML->ulBlockSize;

            DevReq.ulOffset = ulOffset + ulDevOffset;
            DevReq.ulLength = DCLMIN((pLayout->ulBlockCount * hFML->ulBlockSize) - ulOffset, ulLengthRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulByteStatus |= ioStat.op.ulByteStatus;

            ulLengthRemaining       -= ioStat.ulCount;

            if(!ulLengthRemaining || !IOSUCCESS(ioStat, DevReq.ulLength))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pData            += ioStat.ulCount;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulOffset = 0;
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNorReadControlData() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNorWriteControlData()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNorWriteControlData(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_WRITE_CONTROLDATA   *pFmlReq)
{
    FFXIOSTATUS                     ioTotal = INITIAL_BYTEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        DclError();
        return ioTotal;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNorWriteControlData() hFML=%P Offset=%lX Length=%lX pData=%P\n",
        hFML, pFmlReq->ulOffset, pFmlReq->ulLength, pFmlReq->pData));

    DclProfilerEnter("FfxFmlNorWriteControlData", 0, pFmlReq->ulLength);

    /*  Validate the range
    */
    DclAssert(hFML->ulTotalBlocks > pFmlReq->ulOffset / hFML->ulBlockSize);
    DclAssert(hFML->ulTotalBlocks - pFmlReq->ulOffset / hFML->ulBlockSize >= pFmlReq->ulLength / hFML->ulBlockSize);

  #if FFXCONF_STATS_FML
    hFML->stats.ulControlByteWriteRequests++;
    hFML->stats.ulControlByteWriteTotal += pFmlReq->ulLength;
    if(hFML->stats.ulControlByteWriteMax < pFmlReq->ulLength)
        hFML->stats.ulControlByteWriteMax = pFmlReq->ulLength;
  #endif

    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_WRITE_CONTROLDATA DevReq;
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulOffset = pFmlReq->ulOffset;
        D_UINT32                    ulLengthRemaining = pFmlReq->ulLength;

        /*  Iterate through the layout structures until we find the
            one which contains the first byte we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulOffset >= pLayout->ulBlockCount * hFML->ulBlockSize)
        {
            ulOffset -= pLayout->ulBlockCount * hFML->ulBlockSize;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DclMemSet(&DevReq, 0, sizeof DevReq);

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_WRITE_CONTROLDATA;
        DevReq.pData        = pFmlReq->pData;

        while(ulLengthRemaining)
        {
            D_UINT32    ulDevOffset = pLayout->ulBlockOffset * hFML->ulBlockSize;

            DevReq.ulOffset = ulOffset + ulDevOffset;
            DevReq.ulLength = DCLMIN((pLayout->ulBlockCount * hFML->ulBlockSize) - ulOffset, ulLengthRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulByteStatus |= ioStat.op.ulByteStatus;

            ulLengthRemaining       -= ioStat.ulCount;

            if(!ulLengthRemaining || !IOSUCCESS(ioStat, DevReq.ulLength))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pData            += ioStat.ulCount;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulOffset = 0;
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNorWriteControlData() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Private: FfxFmlNorEraseBootBlocks()

    This function erases boot block areas of the flash.  It is
    designed to take a starting block number and block count
    as if the entire flash array consists of boot blocks.
    However it is only legal to use this function on the actual
    areas of the flash which are in the boot block areas.

    To accurately calculate this, you must take into account
    any reserved space at the Device Manager level, as well as
    the location of the FML window.

    It is the responsibility of the developer to ensure that
    this function is only used in the proper range.

    Boot blocks may exist at the bottom, top, or both in the
    flash array.  Additionally, any linearly arranged chips
    will have boot block areas which appear in the middle of
    the array.

    The FfxFmlDeviceInfo() function may be used to query the
    number of boot blocks at the bottom and top of the array,
    as well as the boot block size.

    Parameters:
        hFML    - The FML handle
        pFmlReq - A pointer to the FML request packet

    Return Value:
        Returns an FFXIOSTATUS value indicating the state of the
        operation.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlNorEraseBootBlocks(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_ERASE_BOOT_BLOCKS   *pFmlReq)
{
    FFXIOR_DEV_ERASE_BOOT_BLOCKS    DevReq;
    FFXIOSTATUS                     ioStat = {0, FFXSTAT_BADSTRUCLEN, IOFLAGS_BLOCK};
    D_UINT32                        ulBlockStart;
    D_UINT32                        ulBlockCount;

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        DclError();
        return ioStat;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlNorEraseBootBlocks() hFML=%P StartBB=%lX Count=%lX\n",
        hFML, pFmlReq->ulStartBB, pFmlReq->ulBBCount));

    DclProfilerEnter("FfxFmlNorEraseBootBlocks", 0, pFmlReq->ulBBCount);

    DclAssert(hFML->uDiskFlags & DISK_HASBOOTBLOCKS);
    DclAssert(hFML->ulBBsPerBlock);
    DclAssert(pFmlReq->ulBBCount);

    /*  Calculate where in the range of regular blocks for this window
        the request lies.
    */
    ulBlockStart = pFmlReq->ulStartBB / hFML->ulBBsPerBlock;
    ulBlockCount = ((pFmlReq->ulBBCount-1) / hFML->ulBBsPerBlock) + 1;

    DclAssert(ulBlockCount);

    /*  Validate the range for the FML window.  Valication that the
        areas being access really are boot blocks will be done at
        a lower level.
    */
    if(hFML->ulTotalBlocks <= ulBlockStart)
    {
        DclError();
        ioStat.ffxStat = FFXSTAT_FML_OUTOFRANGE;
        goto Cleanup;
    }

    if(hFML->ulTotalBlocks - ulBlockStart < ulBlockCount)
    {
        DclError();
        ioStat.ffxStat = FFXSTAT_FML_OUTOFRANGE;
        goto Cleanup;
    }

    /*  These structures are the same for now.
    */
    DevReq               = *pFmlReq;
    DevReq.ior.ioFunc    = FXIOFUNC_DEV_ERASE_BOOT_BLOCKS;
    DevReq.ulStartBB    += (hFML->pLayout->ulBlockOffset * hFML->ulBBsPerBlock);

    ioStat = FfxDevIORequest(hFML->pLayout->hFimDev, &DevReq.ior);

  Cleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlNorEraseBootBlocks() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}



#endif
