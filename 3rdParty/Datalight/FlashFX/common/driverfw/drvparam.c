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

    This module contains functions which implement the "Param Get/Set"
    interface at the Driver Framework level.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvparam.c $
    Revision 1.23  2011/02/17 23:26:07Z  garyp
    Fixed to use some renamed functions.
    Revision 1.22  2010/01/23 21:44:10Z  garyp
    Enhanced the driver statistics.  Now support flush.
    Revision 1.21  2009/11/25 02:02:25Z  garyp
    Updated to use critical sections to protect the stats reporting stuff.
    Revision 1.20  2009/07/21 21:25:09Z  garyp
    Merged from the v4.0 branch.  Added debug code.  Eliminated a deprecated
    header.
    Revision 1.19  2009/03/31 19:21:18Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.18  2009/02/09 00:39:07Z  garyp
    Minor status code updates.
    Revision 1.17  2008/06/02 16:45:07Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.16  2008/05/15 21:00:14Z  garyp
    Fixed a broken status code.
    Revision 1.15  2008/04/28 14:01:01Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.14  2008/03/22 18:45:28Z  Garyp
    Moved FfxDriverDiskCalcParams() to drvvbf.c.  Added ParameterGet/Set()
    handlers for TRACEMASKGET and TRACEMASKSWAP.
    Revision 1.13  2008/01/13 07:26:32Z  keithg
    Function header updates to support autodoc.
    Revision 1.12  2007/11/03 05:05:43Z  Garyp
    Removed some invalid asserts.
    Revision 1.11  2007/06/22 16:24:32Z  Garyp
    Factored out the stats reset logic so that it can be independently invoked.
    Revision 1.10  2007/06/07 19:50:26Z  rickc
    Replace deprecated vbfread() call with FfxVbfReadPages()
    Revision 1.9  2007/04/07 03:36:09Z  Garyp
    Modified the "ParameterGet" function so that the buffer size for a given
    parameter can be queried without actually retrieving the parameter.
    Revision 1.8  2007/03/30 18:45:57Z  Garyp
    Corrected some logic that would cause the compaction stats to be
    reset when they should not.
    Revision 1.7  2006/11/10 20:37:40Z  Garyp
    Added ParameterGet/Set() functionality.
    Revision 1.6  2006/06/12 11:59:17Z  Garyp
    Modified to no longer use vbfclientsize().  Renamed a function for clarity.
    Revision 1.5  2006/02/21 02:02:41Z  Garyp
    Minor type changes.
    Revision 1.4  2006/02/06 19:20:38Z  Garyp
    Updated to use new device/disk handle model.
    Revision 1.3  2006/01/07 22:23:44Z  Garyp
    Modified to dynamically allocate the sector buffer.  Moved the block
    size calculation to a higher level.  Added debugging code.
    Revision 1.2  2005/12/28 00:54:20Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.1  2005/10/01 11:04:08Z  Pauli
    Initial revision
    Revision 1.14  2004/12/30 17:32:42Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.13  2004/12/19 00:12:51Z  GaryP
    Modified to accomodate MBRs that manage disks with sector sizes other than
    512 bytes.  Added a FlashFX proprietary field into the MBR to denote the
    sector size.
    Revision 1.12  2004/11/19 20:23:50Z  GaryP
    Updated to use refactored header files.
    Revision 1.11  2004/09/25 04:52:52Z  GaryP
    Removed an invalid assert.
    Revision 1.10  2004/09/23 08:06:29Z  GaryP
    Updated to work when VBF_BLOCK_SIZE is different than DSK_SECTORLEN.
    Revision 1.9  2004/09/17 02:27:26Z  GaryP
    Moved the vbfread() call into this function from FfxGetPhysicalDiskParams().
    Revision 1.8  2004/09/15 01:57:13Z  GaryP
    Eliminated the fDiskValid variable.  Removed some DclProductionError()
    conditions.
    Revision 1.7  2004/08/09 19:40:21Z  GaryP
    Eliminated FAT specific logic.
    Revision 1.6  2004/04/30 02:27:40Z  garyp
    Updated to eliminate passing sector buffers around.
    Revision 1.5  2003/11/05 03:21:30Z  garys
    Merge from FlashFXMT
    Revision 1.2.1.2  2003/11/05 03:21:30  garyp
    Re-checked into variant sandbox.
    Revision 1.3  2003/11/05 03:21:30Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.2  2003/04/16 01:59:54Z  garyp
    Minor logic refactoring.
    Revision 1.1  2003/04/14 01:39:30Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxfmlapi.h>

#if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
#include <vbf.h>
#endif

#if FFXCONF_STATS_DRIVERIO
static FFXSTATUS GetDriverIOStats(FFXDISKINFO *pDisk, DRIVERIOSTATS *pStats, D_BOOL fReset);
#endif


/*-------------------------------------------------------------------
    Public: FfxDriverDiskParameterGet()

    Get a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hDisk     - The Disk handle.  This may be NULL for certain
                    FXPARAM_... types which do not require it.
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the buffer in which to store the
                    value.  May be NULL which causes this function
                    to return the size of buffer required to store
                    the parameter.
        ulBuffLen - The size of buffer.  May be zero if pBuffer is
                    NULL.

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
        If pBuffer is NULL, but the id is valid, the returned
        FFXSTATUS value will be decodeable using the macro
        DCLSTAT_GETUINT20(), which will return the buffer length
        required to store the parameter.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskParameterGet(
    FFXDISKHANDLE   hDisk,
    FFXPARAM        id,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "FfxDriverDiskParameterGet() hDisk=%P ID=%x pBuff=%P Len=%lU\n",
        hDisk, id, pBuffer, ulBuffLen));

    DclAssert(id > FFXPARAM_STARTOFLIST);
    DclAssert(id < FFXPARAM_ENDOFLIST);

    if(!hDisk)
    {
        /*  This switch statement handles only those requests which
            can be satisfied without requiring a Disk handle.
        */
        switch(id)
        {
          #if D_DEBUG && DCLCONF_OUTPUT_ENABLED
            case FFXPARAM_DEBUG_TRACEMASKGET:
                if(!pBuffer)
                {
                    /*  A buffer was not supplied -- return an indicator
                        that the parameter is valid and the buffer length
                        required to hold it.
                    */
                    ffxStat = DCLSTAT_SETUINT20(sizeof(D_UINT32));
                }
                else
                {
                    if(ulBuffLen == sizeof(D_UINT32))
                    {
                        *(D_UINT32*)pBuffer = FfxTraceMaskGet();

                        ffxStat = FFXSTAT_SUCCESS;
                    }
                    else
                    {
                        ffxStat = FFXSTAT_BADPARAMETERLEN;
                    }
                }

                break;
          #endif

            default:
                /*  Not a parameter ID we recognize.  Can't pass the request
                    any lower because we have no disk handle.
                */
                ffxStat = FFXSTAT_BADPARAMETER;
                break;
        }
    }
    else
    {
        /*  This switch statement handles those requests which
            can be satisfied only when a Disk handle is supplied.
        */
        switch(id)
        {
            case FFXPARAM_STATS_DRIVERCOMPACTION:
            case FFXPARAM_STATS_DRIVERCOMPACTIONRESET:
              #if FFXCONF_STATS_COMPACTION
                if(!pBuffer)
                {
                    /*  A buffer was not supplied -- return an indicator
                        that the parameter is valid and the buffer length
                        required to hold it.
                    */
                    ffxStat = DCLSTAT_SETUINT20(sizeof(FFXCOMPSTATS));
                }
                else
                {
                    if(ulBuffLen == sizeof(FFXCOMPSTATS))
                    {
                        if(FfxDriverDiskCompactStatsQuery(*hDisk, (FFXCOMPSTATS*)pBuffer,
                            (D_BOOL)((id == FFXPARAM_STATS_DRIVERCOMPACTIONRESET) ? TRUE : FALSE)))
                        {
                            ffxStat = FFXSTAT_SUCCESS;
                        }
                        else
                        {
                            ffxStat = FFXSTAT_CATEGORYDISABLED;
                        }
                    }
                    else
                    {
                        ffxStat = FFXSTAT_BADPARAMETERLEN;
                    }
                }
              #else
                ffxStat = FFXSTAT_CATEGORYDISABLED;
              #endif
                break;

            case FFXPARAM_STATS_DRIVERIO:
            case FFXPARAM_STATS_DRIVERIORESET:
              #if FFXCONF_STATS_DRIVERIO
                if(!pBuffer)
                {
                    /*  A buffer was not supplied -- return an indicator
                        that the parameter is valid and the buffer length
                        required to hold it.
                    */
                    ffxStat = DCLSTAT_SETUINT20(sizeof((*hDisk)->stats));
                }
                else
                {
                    if(ulBuffLen == sizeof((*hDisk)->stats))
                    {
                        ffxStat = GetDriverIOStats(*hDisk, (DRIVERIOSTATS*)pBuffer, 
                                                id == FFXPARAM_STATS_DRIVERIORESET);
                    }
                    else
                    {
                        ffxStat = FFXSTAT_BADPARAMETERLEN;
                    }
                }
              #else
                ffxStat = FFXSTAT_CATEGORYDISABLED;
              #endif
                break;

            case FFXPARAM_STATS_RESETALL:
              #if FFXCONF_STATS_COMPACTION
                FfxDriverDiskCompactStatsReset(*hDisk);
              #endif
              #if FFXCONF_STATS_DRIVERIO
                GetDriverIOStats(*hDisk, NULL, TRUE);
              #endif

                /*  Fall through
                        .
                        .
                        .
                */

            default:
                /*  Not a parameter ID we recognize, so pass the request to
                    the layer below us.  Depending on what style of disk we
                    are using, we may need to call either VBF or the FML.
                */
              #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
                if((*hDisk)->hVBF)
                    ffxStat = FfxVbfParameterGet((*hDisk)->hVBF, id, pBuffer, ulBuffLen);
                else
              #endif
                    ffxStat = FfxFmlParameterGet((*hDisk)->hFML, id, pBuffer, ulBuffLen);
                break;
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "FfxDriverDiskParameterGet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskParameterset()

    Set a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hDisk     - The Disk handle.  This may be NULL for certain
                    FXPARAM_... types which do not require it.
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the parameter data.
        ulBuffLen - The parameter data length.

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskParameterSet(
    FFXDISKHANDLE   hDisk,
    FFXPARAM        id,
    const void     *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat;

    /*  NOTE:  We explicitly do NOT use INDENT/UNDENT here because the
               changing the trace mask with FFXPARAM_DEBUG_TRACEMASKSWAP
               may result in mismatched indent levels, and the warnings
               that go with them.

               Just don't use that feature for this function.
    */

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, 0),
        "FfxDriverDiskParameterSet() hDisk=%P ID=%x pBuff=%P Len=%lU\n",
        hDisk, id, pBuffer, ulBuffLen));

    DclAssert(id);
    DclAssert(pBuffer);
    DclAssert(ulBuffLen);

    if(!hDisk)
    {
        /*  This switch statement handles only those requests which
            can be satisfied without requiring a Disk handle.
        */
        switch(id)
        {
          #if D_DEBUG && DCLCONF_OUTPUT_ENABLED
            case FFXPARAM_DEBUG_TRACEMASKSWAP:
                if(!pBuffer)
                {
                    /*  A buffer was not supplied -- return an indicator
                        that the parameter is valid and the buffer length
                        required to hold it.
                    */
                    ffxStat = DCLSTAT_SETUINT20(sizeof(D_UINT32));
                }
                else
                {
                    if(ulBuffLen == sizeof(D_UINT32))
                    {
                        D_UINT32    *pulTraceMask = (D_UINT32*)pBuffer;

                        *pulTraceMask = FfxTraceMaskSwap(*pulTraceMask);

                        ffxStat = FFXSTAT_SUCCESS;
                    }
                    else
                    {
                        ffxStat = FFXSTAT_BADPARAMETERLEN;
                    }
                }

                break;
          #endif

            default:
                /*  Not a parameter ID we recognize.  Can't pass the request
                    any lower because we have no disk handle.
                */
                ffxStat = FFXSTAT_BADPARAMETER;
                break;
        }
    }
    else
    {
        /*  This switch statement handles those requests which
            can be satisfied only when a Disk handle is supplied.
        */
        switch(id)
        {
            default:
                /*  Not a parameter ID we recognize, so pass the request to
                    the layer below us.  Depending on what style of disk we
                    are using, we may need to call either VBF or the FML.
                */
              #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
                if((*hDisk)->hVBF)
                    ffxStat = FfxVbfParameterSet((*hDisk)->hVBF, id, pBuffer, ulBuffLen);
                else
              #endif
                    ffxStat = FfxFmlParameterSet((*hDisk)->hFML, id, pBuffer, ulBuffLen);
                break;
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, 0),
        "FfxDriverDiskParameterSet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


#if FFXCONF_STATS_DRIVERIO

/*-------------------------------------------------------------------
    Local: GetDriverIOStats()

    Parameters:
        hDisk     - The Disk handle.  
        pStats    - A pointer to the DRIVERIOSTATS structure to fill.
                    May be NULL if the only purpose for calling this
                    function is to reset the stats.
        fReset    - TRUE to reset the stats values.                    

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS GetDriverIOStats(
    FFXDISKINFO    *pDisk,
    DRIVERIOSTATS  *pStats,
    D_BOOL          fReset)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    DclAssertWritePtr(pDisk, sizeof(*pDisk));

    if(DclCriticalSectionEnter(&pDisk->ulAtomicReadStatsGate, 10000, TRUE))
    {
        if(pStats)
            pStats->Read = pDisk->stats.Read;

        if(fReset)
        {
            DclMemSet(&pDisk->stats.Read, 0, sizeof(pDisk->stats.Read));

            pDisk->ulNextSeqReadSectorNum = 0;
        }

        DclCriticalSectionLeave(&pDisk->ulAtomicReadStatsGate);
    }
    else
    {
        /*  Really should not ever time out...
        */
        DclError();
        ffxStat = DCLSTAT_TIMEOUT;
    }
    
    if(DclCriticalSectionEnter(&pDisk->ulAtomicWriteStatsGate, 10000, TRUE))
    {
        if(pStats)
            pStats->Write = pDisk->stats.Write;

        if(fReset)
        {
            DclMemSet(&pDisk->stats.Write, 0, sizeof(pDisk->stats.Write));

            pDisk->ulNextSeqWriteSectorNum = 0;
        }

        DclCriticalSectionLeave(&pDisk->ulAtomicWriteStatsGate);
    }
    else
    {
        /*  Really should not ever time out...
        */
        DclError();
        ffxStat = DCLSTAT_TIMEOUT;
    }
    
    if(DclCriticalSectionEnter(&pDisk->ulAtomicDiscardStatsGate, 10000, TRUE))
    {
        if(pStats)
            pStats->Discard = pDisk->stats.Discard;

        if(fReset)
        {
            DclMemSet(&pDisk->stats.Discard, 0, sizeof(pDisk->stats.Discard));

            pDisk->ulNextSeqDiscardSectorNum = 0;
        }

        DclCriticalSectionLeave(&pDisk->ulAtomicDiscardStatsGate);
    }
    else
    {
        /*  Really should not ever time out...
        */
        DclError();
        ffxStat = DCLSTAT_TIMEOUT;
    }

    if(DclCriticalSectionEnter(&pDisk->ulAtomicFlushStatsGate, 10000, TRUE))
    {
        if(pStats)
        {
            pStats->Flush               = pDisk->stats.Flush;
            pStats->ulFlushActualMax    = pDisk->stats.ulFlushActualMax;
            pStats->ulFlushActualTotal  = pDisk->stats.ulFlushActualTotal;
        }
        
        if(fReset)
        {
            DclMemSet(&pDisk->stats.Flush, 0, sizeof(pDisk->stats.Flush));
            pDisk->stats.ulFlushActualMax   = 0;
            pDisk->stats.ulFlushActualTotal = 0;
        }

        DclCriticalSectionLeave(&pDisk->ulAtomicFlushStatsGate);
    }
    else
    {
        /*  Really should not ever time out...
        */
        DclError();
        ffxStat = DCLSTAT_TIMEOUT;
    }

    return ffxStat;
}    

#endif  /* FFXCONF_STATS_DRIVERIO */

