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

    This module contains routines to manipulate FlashFX PerfLog data.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmlperflog.c $
    Revision 1.7  2010/07/16 15:08:03Z  garyp
    Display NAND I/O stats only if the Device really is NAND.
    Revision 1.6  2009/04/01 20:25:43Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.5  2009/02/09 00:45:56Z  garyp
    Minor status code updates.
    Revision 1.4  2008/05/08 01:13:37Z  garyp
    Updated to accommodate the new format for FML block statistics.
    Revision 1.3  2008/04/14 21:14:44Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.2  2008/01/13 07:36:44Z  keithg
    Function header updates to support autodoc.
    Revision 1.1  2007/12/26 02:40:58Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if DCLCONF_OUTPUT_ENABLED

#include <fxfmlapi.h>
#include <fxstats.h>
#include <fxperflog.h>



/*-------------------------------------------------------------------
    Public: FfxFmlPerfLogDispatch()

    This function dispatches requests for FML and lower layers
    to retrieve PerfLog information and record it using an
    external interface.

    Parameters:
        hFML - The FML handle
        pReq - A pointer to the DCLREQ_STATS structure to use

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlPerfLogDispatch(
    FFXFMLHANDLE        hFML,
    DCLREQ_STATS       *pReq)
{
    FFXSTATUS           ffxStat;
    DCLPERFLOGHANDLE    hPerfLog;
 
    DclAssert(hFML);
    DclAssert(pReq);
    DclAssert(pReq->nBufferLen);
    DclAssert(pReq->pDescBuffer);
    DclAssert(pReq->pDataBuffer);
 
    switch(pReq->nType)
    {
        case FFXSTATS_FMLIO:
        {
            FFXFMLSTATS  FmlIOStats = {0};

            if(pReq->nSubType == 0)
            {
                ffxStat = FfxFmlParameterGet(hFML, FFXPARAM_STATS_FML, &FmlIOStats, sizeof(FmlIOStats));
                if(ffxStat == FFXSTAT_SUCCESS)
                {
                    hPerfLog = FfxPerfLogFormatFmlIOStats(&FmlIOStats, NULL, pReq->pszCatSuffix, pReq->pszBldSuffix);
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
                    DclStrCpy(pReq->pDescBuffer, "Block Statistics Data (BlockNum Erases Reads Writes)\n");
                    *pReq->pDataBuffer = 0;

                    DclAssert(DclStrLen(pReq->pDescBuffer) <= DCLOUTPUT_BUFFERLEN);

                    ffxStat = FFXSTAT_SUCCESS;
                }
                else
                {
                    /* 40 chars per element
                    */
                    #define MAX_BLOCKS_PER_REQUEST (DCLOUTPUT_BUFFERLEN / 40)
                    FFXFMLBLOCKSTATS    aBS[MAX_BLOCKS_PER_REQUEST];

                    FmlIOStats.pBlockStats = aBS;
                    FmlIOStats.ulBlockStatsCount = MAX_BLOCKS_PER_REQUEST;
                    FmlIOStats.ulBlockStatsStart = (pReq->nSubType-2) * MAX_BLOCKS_PER_REQUEST;

                    ffxStat = FfxFmlParameterGet(hFML, FFXPARAM_STATS_FML, &FmlIOStats, sizeof(FmlIOStats));
                    if(ffxStat == FFXSTAT_SUCCESS)
                    {
                        if(FmlIOStats.ulBlockStatsCount)
                        {
                            unsigned            nLen = 0;
                            unsigned            nn;
                            char               *pBuff = pReq->pDescBuffer;
                            FFXFMLBLOCKSTATS   *pBS = aBS;

                            /*  Zero this just in case there is no data to
                                place in it.
                            */
                            *pReq->pDataBuffer = 0;

                            for(nn=0; nn<FmlIOStats.ulBlockStatsCount; nn++)
                            {
                                DclSNPrintf(pBuff+nLen, DCLOUTPUT_BUFFERLEN-nLen, "%5lU %9lU %9lU %9lU ",
                                    nn+FmlIOStats.ulBlockStatsStart, pBS->ulErases, pBS->ulReads, pBS->ulWrites);

                                nLen += DclStrLen(pBuff);

                                if((nn+1) % (MAX_BLOCKS_PER_REQUEST/2))
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

                                pBS++;
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

        case FFXSTATS_FMLIONAND:
        {
            FFXFMLSTATS  FmlIOStats = {0};
            FFXFMLINFO   FmlInfo;

            ffxStat = FfxFmlDiskInfo(hFML, &FmlInfo);
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                if(FmlInfo.uDeviceType != DEVTYPE_NAND)
                {
                    ffxStat = FFXSTAT_CATEGORYDISABLED;
                    break;
                }
            }
            else
            {
                DclError();
                break;
            }

            if(pReq->nSubType)
            {
                /*  No sub-type for this category
                */
                ffxStat = FFXSTAT_SUBTYPEUNUSED;
                break;
            }

            ffxStat = FfxFmlParameterGet(hFML, FFXPARAM_STATS_FML, &FmlIOStats, sizeof(FmlIOStats));
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                hPerfLog = FfxPerfLogFormatFmlIONandStats(&FmlIOStats, NULL, pReq->pszCatSuffix, pReq->pszBldSuffix);
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

        case FFXSTATS_BBM:
        {
            FFXBBMSTATS  BbmStats = {0};

            if(pReq->nSubType)
            {
                /*  No sub-type for this category
                */
                ffxStat = FFXSTAT_SUBTYPEUNUSED;
                break;
            }

            ffxStat = FfxFmlParameterGet(hFML, FFXPARAM_STATS_BBM, &BbmStats, sizeof(BbmStats));
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                hPerfLog = FfxPerfLogFormatBbmStats(&BbmStats, NULL, pReq->pszCatSuffix, pReq->pszBldSuffix);
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

        case FFXSTATS_LIMIT:
            /*  Returning this status value indicates that there are no
                higher type values, and allows an enumeration process
                to quit.
            */
            ffxStat = DCLSTAT_LIMITREACHED;
            break;

        default:
            ffxStat = FFXSTAT_UNSUPPORTEDREQUEST;
            break;
    }

    return ffxStat;
}


#endif  /* DCLCONF_OUTPUT_ENABLED */

