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

    This module contains code which allows a Disk to be wiped of any stale
    data.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvwipe.c $
    Revision 1.2  2010/12/22 01:25:53Z  garyp
    Corrected documentation.  Eliminated a compiler warning.
    Revision 1.1  2010/11/09 14:10:48Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxstats.h>

#if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
#include <vbf.h>
#endif


/*-------------------------------------------------------------------
    Public: FfxDriverDiskWipe()

    Ensure that all sectors on a FlashFX Disk, which do not contain 
    valid data, are erased.

    Parameters:
        hDisk - The handle of the Disk to wipe.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskWipe(
    FFXDISKHANDLE   hDisk)
{
    FFXDISKINFO    *pDsk = *hDisk;
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverDiskWipe() hDisk=%P\n", hDisk));

    DclProfilerEnter("FfxDriverDiskWipe", 0, 0);

    DclAssert(pDsk);
    DclAssert(pDsk->ulSectorLength);

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    /*  No assert here.  The driver may be configured with no allocator
    */
    if(pDsk->hVBF)
    {
        FFXIOSTATUS ioStat;

        ioStat =  FfxDriverDiskCompactAggressive(hDisk);

        /*  Intentionally ignoring errors for now...
        */
        (void)ioStat;

        ffxStat = FFXSTAT_SUCCESS;
    }
    else
    {
        ffxStat = FFXSTAT_DRV_NOALLOCATOR;
    }

  #else
  
    (void)pDsk; 
  
    ffxStat = FFXSTAT_DRV_NOALLOCATOR;
  #endif

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEUNDENT),
        "FfxDriverDiskWipe() returning Status %lX\n", ffxStat));

    return ffxStat;
}


