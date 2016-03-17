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

    This module contains functionality to allow ranges of mapped or unmapped
    blocks on a Device to be enumerated.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmlrange.c $
    Revision 1.2  2009/03/22 21:37:29Z  garyp
    Merged from the v4.0 branch.
    Revision 1.1.1.6  2009/03/22 21:37:29Z  garyp
    Updated to use a structure which was renamed to accommodate the
    auto-documentation system.
    Revision 1.1.1.5  2009/02/17 04:03:15Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.1.1.4  2009/01/14 04:15:58Z  garyp
    Fixed a bug in FfxFmlDeviceRangeEnumerate() introduced in the previous
    rev which would likely cause the system to hang.
    Revision 1.1.1.3  2008/12/16 21:25:52Z  garyp
    Major update to support the concept of a Disk which spans multiple Devices.
    Revision 1.1.1.2  2008/11/26 18:47:31Z  garyp
    Removed an unused variable.
    Revision 1.1  2008/09/03 23:15:08Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxfmlapi.h>
#include "fml.h"


/*-------------------------------------------------------------------
    Public: FfxFmlDeviceRangeEnumerate()

    Enumerate ranges of blocks on a given Device.

    The enumeration returns the number of contiguous blocks
    remaining, both inside or outside existing Disk definitions.

    Parameters:
        nDevNum       - The Device number to examine.
        ulBlockNum    - The block which denotes the start of the
                        range to examine.
        phFML         - A pointer to an FFXFMLHANDLE which will be
                        set on exit if the given ulBlockNum is
                        within a Disk.  Will be NULL if it is not,
                        or if an error occurs.
        pulBlockCount - A pointer to a D_UINT32 which will return
                        the number of contiguous blocks until the
                        end of the current Disk, the start of the
                        next Disk, or the end of the Device.  In
                        the event of an error, this field will be
                        set to zero.

    Return Value:
        Returns an FFXSTATUS code indicating the state of the
        operation.  Additionally the *phFML and *pulBlockCount
        values will be returned.
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlDeviceRangeEnumerate(
    unsigned            nDevNum,
    D_UINT32            ulBlockNum,
    FFXFMLHANDLE       *phFML,
    D_UINT32           *pulBlockCount)
{
    FFXFMLDEVINFO       fdi;
    FFXSTATUS           ffxStat;
    FFXFMLHANDLE        hFML;
    FFXFMLINFO          fi;
    FFXFMLLAYOUT       *pLayout = NULL;
    D_UINT32            ulNextDisk = D_UINT32_MAX;
    unsigned            nn;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlDeviceRangeEnumerate() DEV%u Block=%lU phFML=%P pulBlockCount=%P\n",
        nDevNum, ulBlockNum, phFML, pulBlockCount));

    DclAssert(phFML);
    DclAssert(pulBlockCount);

    /*  Initialize default results...
    */
    *phFML = NULL;
    *pulBlockCount = 0;

    if(nDevNum >= FFX_MAX_DEVICES)
    {
        FFXPRINTF(1, ("FfxFmlDeviceRangeEnumerate() DEV%u out of range (%u)\n", nDevNum, FFX_MAX_DEVICES));
        ffxStat = FFXSTAT_DEVICENUMBERINVALID;
        goto RangeCleanup;
    }

    ffxStat =  FfxFmlDeviceInfo(nDevNum, &fdi);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("FfxFmlDeviceRangeEnumerate() Error getting DEV%u info, Status=%lX\n", nDevNum, ffxStat));
        goto RangeCleanup;
    }

    if(ulBlockNum >= fdi.ulTotalBlocks)
    {
        FFXPRINTF(1, ("FfxFmlDeviceRangeEnumerate() DEV%u block number %lU is out of range\n", nDevNum, ulBlockNum));
        ffxStat = FFXSTAT_OUTOFRANGE;
        goto RangeCleanup;
    }

    for(nn=0; nn<FFX_MAX_DISKS; nn++)
    {
        hFML = FfxFmlHandle(nn);
        if(hFML)
        {
            ffxStat = FfxFmlDiskInfo(hFML, &fi);
            if(ffxStat != FFXSTAT_SUCCESS)
            {
                FFXPRINTF(1, ("FfxFmlDeviceRangeEnumerate() Error getting DISK%u info, Status=%lX\n", nn, ffxStat));
                goto RangeCleanup;
            }

            if( (nDevNum >= fi.nDeviceNum) &&
                (nDevNum <= fi.nDeviceNum + fi.nDeviceCount - 1) )
            {
                unsigned    dd;

                pLayout = hFML->pLayout;

                for(dd=0; dd < nDevNum - fi.nDeviceNum; dd++)
                    pLayout = pLayout->pNext;

                /*  If we found a Disk which contains this block, break out
                    of the for() loop.  hFML and fi will be initialized with
                    information for the Disk.
                */
                if( (ulBlockNum >= pLayout->ulBlockOffset) &&
                    (ulBlockNum < pLayout->ulBlockOffset + pLayout->ulBlockCount))
                {
                    break;
                }

                /*  Keep track of the first block after the target block number
                    so that we can determine how many unmapped blocks there are
                    in one contiguous chunk.
                */
                if((pLayout->ulBlockOffset > ulBlockNum) && (pLayout->ulBlockOffset < ulNextDisk))
                    ulNextDisk = pLayout->ulBlockOffset;
            }

            /*  Set this to null in case the loop finishes without finding
                a Disk which contains the block.
            */
            hFML = NULL;
        }
    }

    if(hFML)
    {
        DclAssert(pLayout);

        /*  We found a Disk which contains the block
        */
        DclAssert(ulBlockNum >= pLayout->ulBlockOffset);

        /*  Calculate the remaining blocks in this Disk and return
            the count.
        */
        ulBlockNum -= pLayout->ulBlockOffset;
        *pulBlockCount = pLayout->ulBlockCount - ulBlockNum;

        *phFML = hFML;
    }
    else
    {
        /*  No Disk was found which contains the block.  Return the number
            of blocks until the next Disk starts, or the end of the Device.
        */
        if(ulNextDisk == D_UINT32_MAX)
            *pulBlockCount = fdi.ulTotalBlocks - ulBlockNum;
        else
            *pulBlockCount = ulNextDisk - ulBlockNum;
    }

    ffxStat = FFXSTAT_SUCCESS;

  RangeCleanup:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlDeviceRangeEnumerate() returning hFML=%lX BlockCount=%lU Status=%lX\n",
        *phFML, *pulBlockCount, ffxStat));

    return ffxStat;
}


