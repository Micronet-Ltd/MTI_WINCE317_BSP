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

    This module implements the FlashFX Device Manager (DevMgr) code which
    deals with latency.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: devlatency.c $
    Revision 1.6  2009/04/01 20:18:36Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.5  2008/01/13 07:27:59Z  keithg
    Function header updates to support autodoc.
    Revision 1.4  2007/11/03 23:49:35Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2007/04/08 02:19:29Z  Garyp
    Updated so that cycling of the DEV mutex can be controlled at run-time
    whereas before it was done at compile time only.
    Revision 1.2  2007/02/03 23:17:21Z  Garyp
    Updated debug messages.
    Revision 1.1  2007/01/31 22:05:52Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdevapi.h>
#include <fimdev.h>
#include "devmgr.h"


/*  If the project level latency configuration settings are not
    enabled, use default values for the erase poll interval.
    Polling is done more infrequently for NOR since it erases
    much slower than NAND.
*/
#define DEFAULT_NAND_ERASE_POLL_INTERVAL    (1) /* milliseconds */
#define DEFAULT_NOR_ERASE_POLL_INTERVAL     (5) /* milliseconds */
#define ERASE_POLL_MAX                   (3000) /* milliseconds */
#define ERASE_POLL_SHIFT                    (5) /* 1/32nd of block erase time */

#if FFXCONF_LATENCYREDUCTIONENABLED
static void InitErasePollInterval(FFXFIMDEVHANDLE hFimDev, D_UINT32 ulEraseTimeUS);
#endif


/*-------------------------------------------------------------------
    Public: FfxDevLatencyInit()

    This function initializes Device latency characteristics,
    based on the latency settings specified for the project,
    and the performance characteristics as defined by the FIM.

    Parameters:
        pFIM - A pointer to the FFXFIMDEVICE structure to use

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDevLatencyInit(
    FFXFIMDEVICE   *pFIM)
{
  #if FFXCONF_LATENCYREDUCTIONENABLED
    FFXDEVTIMING    Timing;

    DclAssert(pFIM);

    /*  Default to cycling the mutex during I/O to allow other threads to
        get a chance to execute.
    */
    pFIM->fMutexCycle = TRUE;

    /*  Get the project latency settings for this Device
    */
    if(!FfxHookOptionGet(FFXOPT_DEVICE_LATENCY, pFIM->hDev, &pFIM->Latency, sizeof pFIM->Latency))
    {
        FFXPRINTF(1, ("Unable to retrieve DEV%U latency settings, using defaults.\n", pFIM->uDeviceNum));
        {
            FFXDEVLATENCY   dl = {FFX_DEFAULT_LATENCY_SETTINGS};

            pFIM->Latency = dl;
        }
    }

    if(!FfxHookOptionGet(FFXOPT_DEVICE_TIMING, pFIM->hDev, &Timing, sizeof Timing))
    {
        FFXPRINTF(1, ("Unable to retrieve DEV%U timing settings, using defaults.\n", pFIM->uDeviceNum));

        {
            FFXDEVTIMING    dt = {FFX_DEFAULT_TIMING_SETTINGS};

            Timing = dt;
        }
    }

    /*  Based on the configuration information queried above, attempt to
        initialize the various latency characteristics for each type of
        I/O we perform.  Any one of these may be completed upon return
        from these function calls, or if the "Auto-Tune" feature is
        enabled, the initialization may be postponed until we've measured
        enough I/O for us to dynamically configure the values.
    */
    pFIM->ulMaxReadCount =  FfxDevCalcMaxReadCount( pFIM, Timing.ulPageReadUS);
    pFIM->ulMaxWriteCount = FfxDevCalcMaxWriteCount(pFIM, Timing.ulPageWriteUS);
    pFIM->ulMaxEraseCount = FfxDevCalcMaxEraseCount(pFIM, Timing.ulBlockEraseUS);

  #else

    DclAssert(pFIM);

    /*  Latency features are compiled out, so default to a "max-perf"
        configuration.
    */
    pFIM->ulMaxReadCount = D_UINT32_MAX;
    pFIM->ulMaxWriteCount = D_UINT32_MAX;
    pFIM->ulMaxEraseCount = D_UINT32_MAX;

    FFXPRINTF(1, ("Latency reduction features are disabled; using \"max-perf\" settings\n"));

  #endif

    return FFXSTAT_SUCCESS;
}


#if FFXCONF_LATENCYREDUCTIONENABLED

/*-------------------------------------------------------------------
    Public: FfxDevCalcMaxReadCount()

    This function calculates the maximum number of pages to
    read in one operation, based on the read performance, and
    the desired latency settings configured in the project.

    Parameters:
        hFimDev      - The handle for the FIM device to use
        ulReadTimeUS - The number of microseconds to read one page

    Return Value:
        Returns the maximum number of pages to read in any single
        operation.
-------------------------------------------------------------------*/
D_UINT32 FfxDevCalcMaxReadCount(
    FFXFIMDEVHANDLE     hFimDev,
    D_UINT32            ulReadTimeUS)
{
    D_UINT32            ulReadSize;

    DclAssert(hFimDev);

    if(hFimDev->Latency.lReadLatencyUS == 0)
    {
        FFXPRINTF(1, ("Read Latency:  Configured for \"min-latency\"\n"));

        /*  Use the most conservative setting...
        */
        return 1;
    }
    else if(hFimDev->Latency.lReadLatencyUS < 0)
    {
        DclAssert(hFimDev->Latency.lReadLatencyUS == -1);

        FFXPRINTF(1, ("Read Latency:  Configured for \"max-perf\"\n"));

        ulReadSize = D_UINT32_MAX;
    }
    else
    {
        if(ulReadTimeUS)
        {
            /*  The read time is specified, so we have everything we need
                to complete the latency calculation.
            */
            ulReadSize = hFimDev->Latency.lReadLatencyUS / ulReadTimeUS;
            if(!ulReadSize)
                ulReadSize++;

          #if (D_DEBUG == 0) && FFXCONF_LATENCYAUTOTUNE
            /*  This is NOT a DEBUG printf() because we want it to display
                in RELEASE mode so the value can be plugged back into the
                ffxconf.h file, and the "auto-tune" feature then disabled.
            */
            DclPrintf("AutoTune FFX_DEV%U_LATENCY: Read performance is %lU microseconds per page read\n",
                hFimDev->uDeviceNum, ulReadTimeUS);
          #endif

            FFXPRINTF(1, ("Read Latency:  Time/page=%8lUus  DesiredMaxTime=%7ldus  MaxPagesToRead=%5lU\n",
                ulReadTimeUS, hFimDev->Latency.lReadLatencyUS, ulReadSize));
        }
        else
        {
          #if FFXCONF_LATENCYAUTOTUNE
            FFXPRINTF(1, ("Read Latency:  Auto-tuning, temporarily using the \"min-latency\" setting\n"));

            hFimDev->fReadAutoTune = TRUE;
            ulReadSize = 1;

          #else

            FFXPRINTF(1, ("Read Latency:  Read time unknown, defaulting to the \"max-perf\" setting\n"));

            ulReadSize = D_UINT32_MAX;
          #endif
        }
    }

  #if FFXCONF_LATENCYAUTOTUNE
    /*  We could have already determined a max read value if we are using
        an artificial 64KB limit to support an old style FIM.  Make sure we
        don't accidentally increase a value which was already determined.
    */
    if(hFimDev->fReduceOnly && (ulReadSize > hFimDev->ulMaxReadCount))
        ulReadSize = hFimDev->ulMaxReadCount;
  #endif

    return ulReadSize;
}


/*-------------------------------------------------------------------
    Public: FfxDevCalcMaxWriteCount()

    This function calculates the maximum number of pages to
    write in one operation, based on the write performance, and
    the desired latency settings configured in the project.

    Parameters:
        hFimDev       - The handle for the FIM device to use
        ulWriteTimeUS - The number of microseconds to write one page

    Return Value:
        Returns the maximum number of pages to write in any single
        operation.
-------------------------------------------------------------------*/
D_UINT32 FfxDevCalcMaxWriteCount(
    FFXFIMDEVHANDLE     hFimDev,
    D_UINT32            ulWriteTimeUS)
{
    D_UINT32            ulWriteSize;

    DclAssert(hFimDev);

    if(hFimDev->Latency.lWriteLatencyUS == 0)
    {
        FFXPRINTF(1, ("Write Latency: Configured for \"min-latency\"\n"));

        /*  Use the most conservative setting...
        */
        return 1;
    }
    else if(hFimDev->Latency.lWriteLatencyUS < 0)
    {
        DclAssert(hFimDev->Latency.lWriteLatencyUS == -1);

        FFXPRINTF(1, ("Write Latency: Configured for \"max-perf\"\n"));

        ulWriteSize = D_UINT32_MAX;
    }
    else
    {
        if(ulWriteTimeUS)
        {
            /*  The write time is specified, so we have everything we need
                to complete the latency calculation.
            */
            ulWriteSize = hFimDev->Latency.lWriteLatencyUS / ulWriteTimeUS;
            if(!ulWriteSize)
                ulWriteSize++;

          #if (D_DEBUG == 0) && FFXCONF_LATENCYAUTOTUNE
            /*  This is NOT a DEBUG printf() because we want it to display
                in RELEASE mode so the value can be plugged back into the
                ffxconf.h file, and the "auto-tune" feature then disabled.
            */
            DclPrintf("AutoTune FFX_DEV%U_LATENCY: Write performance is %lU microseconds per page written\n",
                hFimDev->uDeviceNum, ulWriteTimeUS);
          #endif

            FFXPRINTF(1, ("Write Latency: Time/page=%8lUus  DesiredMaxTime=%7ldus MaxPagesToWrite=%5lU\n",
                ulWriteTimeUS, hFimDev->Latency.lWriteLatencyUS, ulWriteSize));
        }
        else
        {
          #if FFXCONF_LATENCYAUTOTUNE
            FFXPRINTF(1, ("Write Latency: Auto-tuning, temporarily using the \"min-latency\" setting\n"));

            hFimDev->fWriteAutoTune = TRUE;

            ulWriteSize = 1;

          #else

            FFXPRINTF(1, ("Write Latency: Write time unknown, defaulting to the \"max-perf\" setting\n"));

            ulWriteSize = D_UINT32_MAX;
          #endif
        }
    }

  #if FFXCONF_LATENCYAUTOTUNE
    /*  We could have already determined a max write value if we are using
        an artificial 64KB limit to support an old style FIM.  Make sure we
        don't accidentally increase a value which was already determined.
    */
    if(hFimDev->fReduceOnly && (ulWriteSize > hFimDev->ulMaxWriteCount))
        ulWriteSize = hFimDev->ulMaxWriteCount;
  #endif

    return ulWriteSize;
}


/*-------------------------------------------------------------------
    Public: FfxDevCalcMaxEraseCount()

    This function calculates the maximum number of blocks to
    erase in one operation, based on the erase performance, and
    the desired latency settings configured in the project.

    Parameters:
        hFimDev       - The handle for the FIM device to use
        ulEraseTimeUS - The number of microseconds to erase one block

    Return Value:
        Returns the maximum number of blocks to erase in any single
        operation.
-------------------------------------------------------------------*/
D_UINT32 FfxDevCalcMaxEraseCount(
    FFXFIMDEVHANDLE     hFimDev,
    D_UINT32            ulEraseTimeUS)
{
    D_UINT32            ulEraseCount;

    DclAssert(hFimDev);

    if(hFimDev->Latency.lEraseLatencyUS == 0)
    {
        FFXPRINTF(1, ("Erase Latency: Configured for \"min-latency\"\n"));

        /*  Use the most conservative setting...
        */
        ulEraseCount = 1;
    }
    else if(hFimDev->Latency.lEraseLatencyUS < 0)
    {
        DclAssert(hFimDev->Latency.lEraseLatencyUS == -1);

        FFXPRINTF(1, ("Erase Latency: Configured for \"max-perf\"\n"));

        ulEraseCount = D_UINT32_MAX;
    }
    else
    {
        if(ulEraseTimeUS)
        {
            /*  The erase time is specified, so we have everything we need
                to complete the latency calculation.
            */
            ulEraseCount = hFimDev->Latency.lEraseLatencyUS / ulEraseTimeUS;
            if(!ulEraseCount)
                ulEraseCount++;

          #if (D_DEBUG == 0) && FFXCONF_LATENCYAUTOTUNE
            /*  This is NOT a DEBUG printf() because we want it to display
                in RELEASE mode so the value can be plugged back into the
                ffxconf.h file, and the "auto-tune" feature then disabled.
            */
            DclPrintf("AutoTune FFX_DEV%U_LATENCY: Erase performance is %lU microseconds per block erased\n",
                hFimDev->uDeviceNum, ulEraseTimeUS);
          #endif

            FFXPRINTF(1, ("Erase Latency: Time/block=%8lUus DesiredMaxTime=%7ldus MaxBlocksToErase=%5lU\n",
                ulEraseTimeUS, hFimDev->Latency.lEraseLatencyUS, ulEraseCount));
        }
        else
        {
          #if FFXCONF_LATENCYAUTOTUNE
            FFXPRINTF(1, ("Erase Latency: Auto-tuning, temporarily using the \"min-latency\" setting\n"));

            hFimDev->fEraseAutoTune = TRUE;
            ulEraseCount = 1;

          #else

            FFXPRINTF(1, ("Erase Latency: Erase time unknown, defaulting to the \"max-perf\" setting\n"));

            ulEraseCount = D_UINT32_MAX;
          #endif
        }
    }

    /*  Kind of sucks to initialize this from this point, however being able
        to initialize the erase poll interval on the fly depends on being
        able to first dynamically determine the erase timings.
    */
    InitErasePollInterval(hFimDev, ulEraseTimeUS);

    return ulEraseCount;
}


/*-------------------------------------------------------------------
    Private: InitErasePollInterval()

    This function calculates the erase poll interval to use
    when polling for the completion of erase operations.

    Parameters:
        hFimDev       - The handle for the FIM device to use
        ulEraseTimeUS - The number of microseconds to erase one block

    Return Value:
        None
-------------------------------------------------------------------*/
static void InitErasePollInterval(
    FFXFIMDEVHANDLE     hFimDev,
    D_UINT32            ulEraseTimeUS)
{
    DclAssert(hFimDev);

    /*  Handle the simple cases first.  If the specified value is zero,
        then we are to disable sleeping while polling for erase completion.
    */
    if(hFimDev->Latency.lErasePollInterval == 0)
    {
        hFimDev->uErasePollInterval = 0;
    }
    else if(hFimDev->Latency.lErasePollInterval > 0)
    {
        /*  If an explicit, positive value was specified, convert it from
            microseconds to milliseconds, and verify that it is in a
            reasonable range.
        */
        if(hFimDev->Latency.lErasePollInterval < 1000)
        {
            /*  The value is specified in microseconds, however we currently
                only can sleep in millisecond increments.  If the value was
                greater than 0 and less than 1000, treat it as 1 millisecond.
            */
            hFimDev->uErasePollInterval = 1;
        }
        else
        {
            /*  Use the project specified value, but keep it a reasonable max
            */
            hFimDev->uErasePollInterval =
                (D_UINT16)DCLMIN(ERASE_POLL_MAX, (hFimDev->Latency.lErasePollInterval+500)/1000);
        }
    }
    else if((hFimDev->Latency.lErasePollInterval < 0) && ulEraseTimeUS)
    {
        /*  If is less than zero (should be -1), and the erase time is a
            known quantity, calculate the value to use.

            Set the erase poll interval to 1/32th the amount of time to
            erase a block (in MS), but not less than 1, and not more than
            some a reasonable maximum value.
        */
        hFimDev->uErasePollInterval =
            (D_UINT16)DCLMIN(ERASE_POLL_MAX, (((ulEraseTimeUS+500)/1000) >> ERASE_POLL_SHIFT));

        if(!hFimDev->uErasePollInterval)
            hFimDev->uErasePollInterval++;
    }
  #if FFXCONF_LATENCYAUTOTUNE
    else if(hFimDev->fEraseAutoTune)
    {
        /*  If we get to this point, we have been told to automatically
            determine the erase poll value to use, however the actual
            time to erase a block is either unspecified or unknown.

            Since we are auto-calculating the erase time, we can simply
            delay setting the erase poll interval until that calculation
            is complete, at which time, one of the clauses above will
            handle things.
        */
        return;
    }
  #endif
    else
    {
        /*  The erase times were not specified, and we're not auto-calculating
            them, so simply pick an erase poll interval based on whether this
            device is NAND or NOR.
        */
        if(hFimDev->pFimInfo->uDeviceType == DEVTYPE_NAND)
            hFimDev->uErasePollInterval = DEFAULT_NAND_ERASE_POLL_INTERVAL;
        else
            hFimDev->uErasePollInterval = DEFAULT_NOR_ERASE_POLL_INTERVAL;
    }

    FFXPRINTF(1, ("Erase Poll Interval: %Ums\n", hFimDev->uErasePollInterval));

    return;
}


#endif  /* FFXCONF_LATENCYREDUCTIONENABLED */

