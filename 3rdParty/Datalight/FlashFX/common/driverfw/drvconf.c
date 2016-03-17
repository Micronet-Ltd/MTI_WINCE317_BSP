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
    $Log: drvconf.c $
    Revision 1.21  2010/04/18 23:20:23Z  garyp
    Updated to use some refactored DCL system info functionality.
    Revision 1.20  2009/11/04 18:55:24Z  garyp
    Minor datatype updates -- no functional changes.
    Revision 1.19  2009/07/21 20:55:06Z  garyp
    Merged from the v4.0 branch.  Updated to retrieve the MemTracking 
    stats via the Service IOCTL interface.
    Revision 1.18  2009/04/09 21:26:37Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.17  2009/03/31 18:08:23Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.16  2008/01/17 04:02:15Z  Garyp
    Eliminated the obsolete and unused FFXCONF_ENABLEEXTERNALAPI setting.
    Revision 1.15  2008/01/13 07:26:20Z  keithg
    Function header updates to support autodoc.
    Revision 1.14  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.13  2007/04/07 23:01:50Z  Garyp
    Added latency configuration flags.
    Revision 1.12  2007/03/29 23:44:04Z  Garyp
    Updated to support mutex and read/write semaphore statistics.
    Revision 1.11  2007/01/05 21:10:39Z  Garyp
    Minor function renaming exercise.
    Revision 1.10  2006/11/17 23:11:35Z  Garyp
    Updated to return the compaction model.
    Revision 1.9  2006/10/16 19:31:14Z  Garyp
    Added support for new configuration flags.
    Revision 1.8  2006/10/13 01:22:26Z  Garyp
    Updated to use some renamed symbols.
    Revision 1.7  2006/10/06 00:13:15Z  Garyp
    Modified to use the DCL specific byte-ordering macros.
    Revision 1.6  2006/05/07 04:12:07Z  Garyp
    Eliminated the obsolete reserved space field.
    Revision 1.5  2006/02/21 03:23:49Z  Garyp
    Minor type changes.
    Revision 1.4  2006/01/17 01:45:10Z  Garyp
    Updated to use new device/disk handle model.
    Revision 1.3  2006/01/04 04:18:25Z  Garyp
    Eliminated some obsolete settings and added new information.
    Revision 1.2  2005/12/17 21:54:58Z  garyp
    Removed some obsolete logic.
    Revision 1.1  2005/12/04 21:07:08Z  Pauli
    Initial revision
    Revision 1.4  2005/12/04 21:07:07Z  Garyp
    Modified the compaction model to be specified as a tri-state value, which is
    one of the following FFX_COMPACT_SYNCHRONOUS, FFX_COMPACT_BACKGROUNDIDLE, or
    FFX_COMPACT_BACKGROUNDTHREAD.
    Revision 1.3  2005/11/06 09:05:23Z  Garyp
    Updated to use DclSystemInfo().
    Revision 1.2  2005/10/09 22:54:50Z  Garyp
    Removed singlethreaded FIM support.
    Revision 1.1  2005/08/03 19:30:42Z  Garyp
    Initial revision
    Revision 1.2  2004/12/30 17:32:41Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2004/11/29 19:14:58Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/
#include <flashfx.h>
#include <fxconfig.h>
#include <dlservice.h>


/*-------------------------------------------------------------------
    Protected: FfxConfigurationInfo()

    Get FlashFX configuration information.

    NOTE: This function is NOT architecturally part of the
          Driver Framework.  It exists in this location solely
          for linkage purposes.  This function must reside
          outside the core code such that it can call through
          the external API.

    TODO: This function should be refactored.

    Parameters:
        pCI      - The FFXCONFIGINFO structure to fill
        fVerbose - Return verbose information where possible
        fReset   - Reset any statistics which are reset-able

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxConfigurationInfo(
    FFXCONFIGINFO      *pCI,
    D_BOOL              fVerbose,
    D_BOOL              fReset)
{
    DCLDECLAREREQUESTPACKET (MEMTRACK, STATS, stats);   /* DCLREQ_MEMTRACK_STATS */

    DclAssert(pCI);

    if(pCI->uStrucLen != sizeof *pCI)
    {
        DclError();
        return FFXSTAT_BADSTRUCLEN;
    }

    pCI->uStrucLen          = sizeof *pCI;
    pCI->uMaxFmlDevices     = FFX_MAX_DEVICES;
    pCI->uMaxVbfDrives      = FFX_MAX_DISKS;
    pCI->uCompactionModel   = FFX_COMPACTIONMODEL;

  #if FFXCONF_FATSUPPORT
    pCI->ulConfBits |= FFXCONFBIT_FATSUPPORT;
  #endif
  #if FFXCONF_RELIANCESUPPORT
    pCI->ulConfBits |= FFXCONFBIT_RELIANCESUPPORT;
  #endif
  #if FFXCONF_FORMATSUPPORT
    pCI->ulConfBits |= FFXCONFBIT_FORMATSUPPORT;
  #endif
  #if FFXCONF_MBRFORMAT
    pCI->ulConfBits |= FFXCONFBIT_MBRFORMAT;
  #endif
  #if FFXCONF_MBRSUPPORT
    pCI->ulConfBits |= FFXCONFBIT_MBRSUPPORT;
  #endif
  #if FFXCONF_BBMSUPPORT
    pCI->ulConfBits |= FFXCONFBIT_BBMFORMAT;
  #endif
  #if FFXCONF_DRIVERAUTOFORMAT
    pCI->ulConfBits |= FFXCONFBIT_DRIVERAUTOFORMAT;
  #endif
  #if FFXCONF_FATFORMATSUPPORT
    pCI->ulConfBits |= FFXCONFBIT_FATFORMATSUPPORT;
  #endif
  #if FFXCONF_FATMONITORSUPPORT
    pCI->ulConfBits |= FFXCONFBIT_FATMONITORSUPPORT;
  #endif
  #if FFXCONF_FORCEALIGNEDIO
    pCI->ulConfBits |= FFXCONFBIT_FORCEALIGNEDIO;
  #endif
  #if FFXCONF_LATENCYREDUCTIONENABLED
    pCI->ulConfBits |= FFXCONFBIT_LATENCYRUNTIME;
    #if FFXCONF_LATENCYAUTOTUNE
    pCI->ulConfBits |= FFXCONFBIT_LATENCYAUTOTUNE;
    #endif
    #if FFXCONF_ERASESUSPENDSUPPORT
    pCI->ulConfBits |= FFXCONFBIT_LATENCYERASESUSPEND;
    #endif
  #endif

    if(DclSystemInfoQuery(&pCI->SysInfo) != DCLSTAT_SUCCESS)
        pCI->SysInfo.nStrucLen = 0;

    stats.dms.uStrucLen = sizeof(stats.dms);
    stats.fVerbose = fVerbose;
    stats.fReset = fReset;

    if(DclServiceIoctl(0, DCLSERVICE_MEMTRACK, &stats.ior) != DCLSTAT_SUCCESS)
        pCI->MemStats.uStrucLen = 0;
    else
        pCI->MemStats = stats.dms;

    if(DclMutexStatsQuery(&pCI->MutStats, fVerbose, fReset) != DCLSTAT_SUCCESS)
        pCI->MutStats.nStrucLen = 0;

    if(DclSemaphoreStatsQuery(&pCI->SemStats, fVerbose, fReset) != DCLSTAT_SUCCESS)
        pCI->SemStats.nStrucLen = 0;

    if(DclSemaphoreRdWrStatsQuery(&pCI->RWSemStats, fVerbose, fReset) != DCLSTAT_SUCCESS)
        pCI->RWSemStats.nStrucLen = 0;

    return FFXSTAT_SUCCESS;
}



