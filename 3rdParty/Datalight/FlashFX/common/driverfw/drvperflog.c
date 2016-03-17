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

    This module contains routines to manipulate FlashFX PerfLog data.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvperflog.c $
    Revision 1.10  2009/04/09 21:26:36Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.9  2009/03/31 19:21:40Z  davidh
    Function Hearders updated for AutoDoc.
    Revision 1.8  2009/02/09 00:39:24Z  garyp
    Minor status code updates.
    Revision 1.7  2008/04/14 21:15:42Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.6  2008/01/13 07:26:33Z  keithg
    Function header updates to support autodoc.
    Revision 1.5  2007/12/26 02:36:02Z  Garyp
    Moved the handlers for the FML and lower layers to the function
    FfxFmlPerfLogDispatch() to allow those stats interfaces to be used
    even if the higher layers of FlashFX are disabled or not included.
    Revision 1.4  2007/12/13 22:34:33Z  Garyp
    Updated to support querying for sub-type information.  Now allow region
    stats and FML erase counts to be queried in iterations.
    Revision 1.3  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/08/29 23:29:25Z  Garyp
    Changed some D_BUFFER types to char to avoid compiler warnings in
    some environments.
    Revision 1.1  2007/06/22 17:26:02Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if DCLCONF_OUTPUT_ENABLED

#include <fxdriver.h>
#include <fxstats.h>
#include <fxperflog.h>


/*-------------------------------------------------------------------
    Protected: FfxDriverPerfLogDispatch()

    This function dispatches requests to retrieve PerfLog
    information and record it using an external interface.

    Parameters:
        hDisk - The DISK handle
        pReq  - A pointer to the DCLREQ_STATS structure to use

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverPerfLogDispatch(
    FFXDISKHANDLE       hDisk,
    DCLREQ_STATS       *pReq)
{
    FFXSTATUS           ffxStat;
    DCLPERFLOGHANDLE    hPerfLog;

    DclAssert(hDisk);
    DclAssert(pReq);
    DclAssert(pReq->nBufferLen);
    DclAssert(pReq->pDescBuffer);
    DclAssert(pReq->pDataBuffer);

    switch(pReq->nType)
    {
        case FFXSTATS_DISKIO:
        {
            DRIVERIOSTATS   DrvStats;

            if(pReq->nSubType)
            {
                /*  No sub-type for this category
                */
                ffxStat = FFXSTAT_SUBTYPEUNUSED;
                break;
            }

            DclMemSet(&DrvStats, 0, sizeof(DrvStats));

            ffxStat = FfxDriverDiskParameterGet(hDisk, FFXPARAM_STATS_DRIVERIO, &DrvStats, sizeof(DrvStats));
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                hPerfLog = FfxPerfLogFormatDiskIOStats(&DrvStats, NULL, pReq->pszCatSuffix, pReq->pszBldSuffix);
                if(hPerfLog)
                {
                    ffxStat = DclPerfLogCopy(hPerfLog, pReq->nBufferLen, pReq->pDescBuffer, pReq->pDataBuffer);
                    DclPerfLogClose(hPerfLog);
                }
                else
                {
                    ffxStat = FFXSTAT_PERFLOG_OPENFAILED;
                }
            }

            break;
        }

        case FFXSTATS_DISKCOMPACTION:
        {
            FFXCOMPSTATS   CompStats;

            if(pReq->nSubType)
            {
                /*  No sub-type for this category
                */
                ffxStat = FFXSTAT_SUBTYPEUNUSED;
                break;
            }

            DclMemSet(&CompStats, 0, sizeof(CompStats));

            ffxStat = FfxDriverDiskParameterGet(hDisk, FFXPARAM_STATS_DRIVERCOMPACTION, &CompStats, sizeof(CompStats));
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                hPerfLog = FfxPerfLogFormatDiskCompactionStats(&CompStats, NULL, pReq->pszCatSuffix, pReq->pszBldSuffix);
                if(hPerfLog)
                {
                    ffxStat = DclPerfLogCopy(hPerfLog, pReq->nBufferLen, pReq->pDescBuffer, pReq->pDataBuffer);
                    DclPerfLogClose(hPerfLog);
                }
                else
                {
                    ffxStat = FFXSTAT_PERFLOG_OPENFAILED;
                }
            }

            break;
        }

        case FFXSTATS_VBFCOMPACTION:
        {
            VBFCOMPSTATS   VbfCompStats;

            if(pReq->nSubType)
            {
                /*  No sub-type for this category
                */
                ffxStat = FFXSTAT_SUBTYPEUNUSED;
                break;
            }

            DclMemSet(&VbfCompStats, 0, sizeof(VbfCompStats));

            ffxStat = FfxDriverDiskParameterGet(hDisk, FFXPARAM_STATS_VBFCOMPACTION, &VbfCompStats, sizeof(VbfCompStats));
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                hPerfLog = FfxPerfLogFormatVbfCompactionStats(&VbfCompStats, NULL, pReq->pszCatSuffix, pReq->pszBldSuffix);
                if(hPerfLog)
                {
                    ffxStat = DclPerfLogCopy(hPerfLog, pReq->nBufferLen, pReq->pDescBuffer, pReq->pDataBuffer);
                    DclPerfLogClose(hPerfLog);
                }
                else
                {
                    ffxStat = FFXSTAT_PERFLOG_OPENFAILED;
                }
            }

            break;
        }

        case FFXSTATS_VBFREGION:
        {
            VBFREGIONSTATS  VbfRegionStats;

            DclMemSet(&VbfRegionStats, 0, sizeof(VbfRegionStats));

            if(pReq->nSubType == 0)
            {
                ffxStat = FfxDriverDiskParameterGet(hDisk, FFXPARAM_STATS_VBFREGION, &VbfRegionStats, sizeof(VbfRegionStats));
                if(ffxStat == FFXSTAT_SUCCESS)
                {
                    hPerfLog = FfxPerfLogFormatVbfRegionStats(&VbfRegionStats, NULL, pReq->pszCatSuffix, pReq->pszBldSuffix);
                    if(hPerfLog)
                    {
                        ffxStat = DclPerfLogCopy(hPerfLog, pReq->nBufferLen, pReq->pDescBuffer, pReq->pDataBuffer);
                        DclPerfLogClose(hPerfLog);
                    }
                    else
                    {
                        ffxStat = FFXSTAT_PERFLOG_OPENFAILED;
                    }
                }
            }
            else
            {
                if(pReq->nSubType == 1)
                {
                    DclStrCpy(pReq->pDescBuffer, "Region Mount Data\n");
                    DclStrCpy(pReq->pDataBuffer, "Reg#:   Mounts   Concur     Hits Reg#:   Mounts   Concur     Hits Reg#:   Mounts   Concur     Hits Reg#:   Mounts   Concur     Hits\n");

                    DclAssert(DclStrLen(pReq->pDataBuffer) <= DCLOUTPUT_BUFFERLEN);

                    ffxStat = FFXSTAT_SUCCESS;
                }
                else
                {
                    #define             MAX_REGIONS_PER_REQUEST     (8)
                    VBFREGIONMOUNTDATA  aMountData[MAX_REGIONS_PER_REQUEST];

                    VbfRegionStats.pMountData = aMountData;
                    VbfRegionStats.nMountDataCount = MAX_REGIONS_PER_REQUEST;
                    VbfRegionStats.nMountDataStart = (pReq->nSubType-2) * MAX_REGIONS_PER_REQUEST;

                    ffxStat = FfxDriverDiskParameterGet(hDisk, FFXPARAM_STATS_VBFREGION, &VbfRegionStats, sizeof(VbfRegionStats));
                    if(ffxStat == FFXSTAT_SUCCESS)
                    {
                        if(VbfRegionStats.nMountDataCount)
                        {
                            unsigned            nLen = 0;
                            unsigned            nn;
                            char               *pBuff = pReq->pDescBuffer;
                            VBFREGIONMOUNTDATA *pMD = aMountData;

                            /*  Zero this just in case there is no data to
                                place in it.
                            */
                            *pReq->pDataBuffer = 0;

                            for(nn=0; nn<VbfRegionStats.nMountDataCount; nn++)
                            {
                                DclSNPrintf(pBuff+nLen, DCLOUTPUT_BUFFERLEN-nLen, "%4u: %8lU %8lU %8lU ",
                                    VbfRegionStats.nMountDataStart + nn, pMD->ulTotalMounts,
                                    pMD->ulConcurrentMounts, pMD->ulCacheHits);

                                if((nn+1) % (MAX_REGIONS_PER_REQUEST/2))
                                {
                                    nLen = DclStrLen(pBuff);
                                }
                                else
                                {
                                    /*  Properly terminate the first buffer
                                    */
                                    DclStrCat(pBuff, "\n");

                                    /*  Switch to the second output buffer
                                    */
                                    pBuff = pReq->pDataBuffer;
                                    nLen = 0;
                                }

                                pMD++;
                            }

                            /*  If there was an odd number of elements, the
                                last line will not contain the proper
                                termination -- fix it if needed.
                            */
                            if(pBuff[DclStrLen(pBuff)-1] != '\n')
                                DclStrCat(pBuff, "\n");
                        }
                        else
                        {
                            ffxStat = FFXSTAT_SUBTYPECOMPLETE;
                        }
                    }
                }
            }

            break;
        }

        case FFXSTATS_LIMIT:
            /*  Returning this status value indicates that there are no
                higher type values, and allows an enumeration process
                to quit.
            */
            ffxStat = DCLSTAT_LIMITREACHED;
            break;

        default:
        {
            FFXFMLHANDLE    hFML = (*hDisk)->hFML;

            if(hFML)
                ffxStat = FfxFmlPerfLogDispatch(hFML, pReq);
            else
                ffxStat = FFXSTAT_BADPARAMETER;

            break;
        }
    }

    return ffxStat;
}


#endif  /* DCLCONF_OUTPUT_ENABLED */

