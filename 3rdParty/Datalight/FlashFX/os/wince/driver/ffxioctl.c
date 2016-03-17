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

    This module contains IOCTL dispatch routines for FlashFX specific IOCTLs.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ffxioctl.c $
    Revision 1.25  2010/04/19 19:18:45Z  garyp
    Updated so the QUEUE_PENDING_DISCARDS feature is configurable at
    compile time.  Added profiler instrumentation.
    Revision 1.24  2010/01/23 21:45:25Z  garyp
    Fixed so that for CE versions prior to v6, stats requests work properly.
    Revision 1.23  2009/11/02 23:47:47Z  garyp
    Added profiler instrumentation.  Cleaned up some error handling logic.
    Revision 1.22  2009/02/09 01:05:47Z  garyp
    Removed some unnecessary headers.
    Revision 1.21  2008/05/07 01:20:23Z  garyp
    Updated to accommodate the new format for FML block statistics.
    Revision 1.20  2008/03/29 19:57:53Z  garyp
    Fixed some broken return logic.
    Revision 1.19  2008/03/26 01:10:15Z  Garyp
    Updated to support more Driver Framework functions.  Modified to compile
    when allocator support is turned off.
    Revision 1.18  2007/12/26 02:04:00Z  Garyp
    Moved the special IOCTL handlers for the FML and lower interfaces into
    the function FfxCe5DispatchFmlIoctl(), to allow that functionality to be
    separately linked in when the higher layers of FlashFX are not being used.
    Revision 1.17  2007/12/13 20:46:46Z  Garyp
    Updated so the FML stats sub-type data (erase counts) is properly
    handled with regard to mapping pointers.
    Revision 1.16  2007/11/03 23:50:10Z  Garyp
    Updated to use the standard module header.
    Revision 1.15  2007/08/06 23:44:27Z  timothyj
    Made mapping and unmapping of pointers conditional for Raw accesses.
    Revision 1.14  2007/06/07 22:45:04Z  rickc
    Removed deprecated ioctls for vbfread() and vbfwrite()
    Revision 1.13  2007/04/07 01:43:50Z  Garyp
    Improved parameter validation.
    Revision 1.12  2007/04/06 03:19:45Z  Garyp
    Fixed some cut-and-paste errors.
    Revision 1.11  2007/03/30 19:09:46Z  Garyp
    Updated to support mutex and read/write semaphore statistics.
    Revision 1.10  2007/03/04 02:59:10Z  Garyp
    Removed CE 3.0 support.  Added CE 6.0 support.
    Revision 1.9  2007/01/10 01:54:40Z  Garyp
    Minor function renaming exercise.
    Revision 1.8  2006/11/10 20:05:14Z  Garyp
    Added ParameterGet() external API support.
    Revision 1.7  2006/08/21 22:05:12Z  Garyp
    Fixed the default case for FfxCeDispatchIoctl() to properly pass the request
    to the next level.
    Revision 1.6  2006/03/06 19:18:51Z  Garyp
    Added FfxFmlDeviceInfo() support.
    Revision 1.5  2006/02/21 02:02:40Z  Garyp
    Modified to use the newly updated external API mechanisms.
    Revision 1.4  2006/02/15 05:46:14Z  Garyp
    Commented out the ioctl stuff (work-in-progress).
    Revision 1.3  2006/02/13 09:26:36Z  Garyp
    Temporarily disabled -- work-in-progress.
    Revision 1.2  2006/01/02 06:42:23Z  Garyp
    Added an external interface for FfxDriverGetConfigInfo().
    Revision 1.1  2005/10/04 23:22:36Z  Pauli
    Initial revision
    Revision 1.3  2004/12/30 23:08:32Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2004/01/14 20:08:20Z  garys
    Revision 1.1.1.3  2004/01/14 20:08:20  garyp
    Modified to work with the updated vbfgetpartitioninfo() which returns
    a success/fail code.
    Revision 1.1.1.2  2004/01/03 01:14:58Z  garyp
    Include changes.
    Revision 1.1  2003/12/05 23:02:40Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <windev.h>
#include <devload.h>
#include <diskio.h>

#include <flashfx.h>
#include <fxdriver.h>
#include <fxfmlapi.h>
#include <fxapireq.h>
#include <fxperflog.h>
#include <fmlreq.h>
#include <drvfwreq.h>
#include <dlstats.h>
#include <fxconfig.h>
#include <oecommon.h>

#if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
#include <vbf.h>
#include <vbfreq.h>
#endif

#include <ffxwce.h>
#include "ffxdrv.h"

#if _WIN32_WCE < 600
static void   CopyToClientBuffer(void *pDest, void *pSource, D_UINT32 ulLength);
#endif

/*-------------------------------------------------------------------
    NOTE:  These IOCTL handlers are virtually identical to the
           generic handlers, with the exception that they do
           MapPtrToProcess() as needed.  If not for this
           requirement, the default handlers would be used.
-------------------------------------------------------------------*/
D_BOOL FfxCeDispatchIoctl(
    FFXDISKHANDLE   hDisk,
    FFXIOREQUEST   *pReqHdr,
    DWORD           dwLen)
{
    FFXDISKINFO    *pDisk = *hDisk;
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 2, 0),
        "FfxCeDispatchIoctl() hDisk=%P pReq=%P dwLen=%lX\n",
        hDisk, pReqHdr, dwLen));

    DclProfilerEnter("FfxCeDispatchIoctl", 0, 0);

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT && QUEUE_PENDING_DISCARDS
    FfxCeFlushPendingDiscards(hDisk);
  #endif

    switch(pReqHdr->ioFunc)
    {
      #if _WIN32_WCE < 600
      #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
        case FXIOFUNC_EXTVBF_DISKINFO:
        {
            REQ_VBFDISKINFO        *pReq = (REQ_VBFDISKINFO*)pReqHdr;
            VBFDISKINFO             di;

            if(pReq->req.ulReqLen != sizeof(*pReq))
                goto BadPacketLength;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "CE Dispatch FfxVbfDiskInfo(), hVBF=%P pDiskInfo=%P\n",
                pReq->hVBF, pReq->pDiskInfo));

            pReq->ffxStat = FfxVbfDiskInfo(pReq->hVBF, &di);

            if(pReq->ffxStat == FFXSTAT_SUCCESS)
                CopyToClientBuffer(pReq->pDiskInfo, &di, sizeof *pReq->pDiskInfo);

            break;
        }

        case FXIOFUNC_EXTVBF_DISKMETRICS:
        {
            REQ_VBFDISKMETRICS     *pReq = (REQ_VBFDISKMETRICS*)pReqHdr;
            VBFDISKMETRICS          dm;

            if(pReq->req.ulReqLen != sizeof(*pReq))
                goto BadPacketLength;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "CE Dispatch FfxVbfDiskMetrics(), hVBF=%P pDiskMets=%P\n",
                pReq->hVBF, pReq->pDiskMets));

            pReq->ffxStat = FfxVbfDiskMetrics(pReq->hVBF, &dm);

            if(pReq->ffxStat == FFXSTAT_SUCCESS)
                CopyToClientBuffer(pReq->pDiskMets, &dm, sizeof *pReq->pDiskMets);

            break;
        }

        case FXIOFUNC_EXTVBF_UNITMETRICS:
        {
            REQ_VBFUNITMETRICS     *pReq = (REQ_VBFUNITMETRICS*)pReqHdr;
            VBFUNITMETRICS          ui;
            void                   *pUI;

            if(pReq->req.ulReqLen != sizeof(*pReq))
                goto BadPacketLength;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "CE Dispatch FfxVbfUnitMetrics(), hVBF=%P pUnitMets=%P\n",
                pReq->hVBF, pReq->pUnitMets));

            pUI = MapPtrToProcess((LPVOID) pReq->pUnitMets, GetCallerProcess());
            DclAssert(pUI);

            DclMemCpy(&ui, pUI, sizeof ui);

            UnMapPtr(pUI);

            pReq->ffxStat = FfxVbfUnitMetrics(pReq->hVBF, &ui);

            if(pReq->ffxStat == FFXSTAT_SUCCESS)
                CopyToClientBuffer(pReq->pUnitMets, &ui, sizeof *pReq->pUnitMets);

            break;
        }

        case FXIOFUNC_EXTVBF_READPAGES:
        {
            REQ_VBFREAD        *pReq = (REQ_VBFREAD*)pReqHdr;
            void               *pBuff;

            if(pReq->req.ulReqLen != sizeof(*pReq))
                goto BadPacketLength;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "CE Dispatch FfxVbfReadPages(), hVBF=%P StartPage=%lX Count=%lX pBuff=%P\n",
                pReq->hVBF, pReq->ulStartPage, pReq->ulPageCount, pReq->pBuffer));

            pBuff = MapPtrToProcess((LPVOID)pReq->pBuffer, GetCallerProcess());
            DclAssert(pBuff);

            pReq->ioStat = FfxVbfReadPages(pReq->hVBF, pReq->ulStartPage, pReq->ulPageCount, pBuff);

            UnMapPtr(pBuff);

            break;
        }

        case FXIOFUNC_EXTVBF_WRITEPAGES:
        {
            REQ_VBFWRITE       *pReq = (REQ_VBFWRITE*)pReqHdr;
            void               *pBuff;

            if(pReq->req.ulReqLen != sizeof(*pReq))
                goto BadPacketLength;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "CE Dispatch FfxVbfWritePages(), hVBF=%P StartPage=%lX Count=%lX pBuff=%P\n",
                pReq->hVBF, pReq->ulStartPage, pReq->ulPageCount, pReq->pBuffer));

            pBuff = MapPtrToProcess((LPVOID)pReq->pBuffer, GetCallerProcess());
            DclAssert(pBuff);

            pReq->ioStat = FfxVbfWritePages(pReq->hVBF, pReq->ulStartPage, pReq->ulPageCount, pBuff);

            UnMapPtr(pBuff);

            break;
        }
      #endif /* FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT */

        case FXIOFUNC_EXTDRVRFW_DEVCREATEPARAM:
        {
            REQ_DRVRFW_DEVCREATEPARAM       *pReq = (REQ_DRVRFW_DEVCREATEPARAM*)pReqHdr;
            void               *pBuff;

            if(pReq->req.ulReqLen != sizeof(*pReq))
                goto BadPacketLength;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "CE Dispatch FfxDriverDeviceCreateParam(), pDrv=%P pConf=%P pHook=%P\n",
                pReq->pFDI, pReq->pConf, pReq->pHook));

            pBuff = MapPtrToProcess((LPVOID)pReq->pConf, GetCallerProcess());
            DclAssert(pBuff);

            pReq->hDev = FfxDriverDeviceCreateParam(pReq->pFDI, pBuff, pReq->pHook);

            UnMapPtr(pBuff);

            break;
        }

        case FXIOFUNC_EXTDRVRFW_DEVGEOMETRY:
        {
            REQ_DRVRFW_DEVGEOMETRY  *pReq = (REQ_DRVRFW_DEVGEOMETRY*)pReqHdr;
            void                    *pBuff;

            if(pReq->req.ulReqLen != sizeof(*pReq))
                goto BadPacketLength;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "CE Dispatch FfxDriverDeviceGeometry() hDev=%P pGeometry=%P\n",
                pReq->hDev, pReq->pGeometry));

            pBuff = MapPtrToProcess((LPVOID)pReq->pGeometry, GetCallerProcess());
            DclAssert(pBuff);

            pReq->ffxStat = FfxDriverDeviceGeometry(pReq->hDev, pBuff);

            UnMapPtr(pBuff);

            break;
        }

        case FXIOFUNC_EXTDRVRFW_DEVSETTINGS:
        {
            REQ_DRVRFW_DEVSETTINGS *pReq = (REQ_DRVRFW_DEVSETTINGS*)pReqHdr;
            void                   *pBuff;

            if(pReq->req.ulReqLen != sizeof(*pReq))
                goto BadPacketLength;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "CE Dispatch FfxDriverDeviceSettings() pConf=%P\n", pReq->pConf));

            pBuff = MapPtrToProcess((LPVOID)pReq->pConf, GetCallerProcess());
            DclAssert(pBuff);

            pReq->ffxStat = FfxDriverDeviceSettings(pBuff);

            UnMapPtr(pBuff);

            break;
        }

      #if FFXCONF_BBMSUPPORT

        case FXIOFUNC_EXTDRVRFW_DEVBBMSETTINGS:
        {
            REQ_DRVRFW_DEVBBMSETTINGS  *pReq = (REQ_DRVRFW_DEVBBMSETTINGS*)pReqHdr;
            void                       *pBuff;

            if(pReq->req.ulReqLen != sizeof(*pReq))
                goto BadPacketLength;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "CE Dispatch FfxDriverDeviceBbmSettings() pConf=%P\n", pReq->pConf));

            pBuff = MapPtrToProcess((LPVOID)pReq->pConf, GetCallerProcess());
            DclAssert(pBuff);

            pReq->ffxStat = FfxDriverDeviceBbmSettings(pBuff);

            UnMapPtr(pBuff);

            break;
        }

      #endif

        case FXIOFUNC_EXTDRVRFW_DISKCREATEPARAM:
        {
            REQ_DRVRFW_DISKCREATEPARAM  *pReq = (REQ_DRVRFW_DISKCREATEPARAM*)pReqHdr;
            void                        *pBuff;

            if(pReq->req.ulReqLen != sizeof(*pReq))
                goto BadPacketLength;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "CE Dispatch FfxDriverDiskCreateParam(), pDrv=%P pConf=%P pHook=%P\n",
                pReq->pFDI, pReq->pConf, pReq->pHook));

            pBuff = MapPtrToProcess((LPVOID)pReq->pConf, GetCallerProcess());
            DclAssert(pBuff);

            pReq->hDisk = FfxDriverDiskCreateParam(pReq->pFDI, pBuff, pReq->pHook);

            UnMapPtr(pBuff);

            break;
        }

        case FXIOFUNC_EXTDRVRFW_DISKGEOMETRY:
        {
            REQ_DRVRFW_DISKGEOMETRY  *pReq = (REQ_DRVRFW_DISKGEOMETRY*)pReqHdr;
            void                    *pBuff;

            if(pReq->req.ulReqLen != sizeof(*pReq))
                goto BadPacketLength;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "CE Dispatch FfxDriverDiskGeometry() hDisk=%P pGeometry=%P\n",
                pReq->hDisk, pReq->pGeometry));

            pBuff = MapPtrToProcess((LPVOID)pReq->pGeometry, GetCallerProcess());
            DclAssert(pBuff);

            pReq->ffxStat = FfxDriverDiskGeometry(pReq->hDisk, pBuff);

            UnMapPtr(pBuff);

            break;
        }

        case FXIOFUNC_EXTDRVRFW_DISKGETPARAM:
        {
            REQ_DRVRFW_DISKPARAMGET *pReq = (REQ_DRVRFW_DISKPARAMGET*)pReqHdr;
            void                   *pTmp;

            if(pReq->req.ulReqLen != sizeof(*pReq))
                goto BadPacketLength;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverGetParameter()\n"));

            pTmp = MapPtrToProcess((LPVOID)pReq->pBuffer, GetCallerProcess());

            if(((pReq->id == FFXPARAM_STATS_VBFREGION) ||
                (pReq->id == FFXPARAM_STATS_VBFREGIONRESET)) && pTmp)
            {
                VBFREGIONSTATS     *pVBFStats = pTmp;

                if(pVBFStats->pMountData)
                {
                    void   *pMountData = pVBFStats->pMountData;

                    /*  Temporarily patch the pointer in the client buffer
                    */
                    pVBFStats->pMountData = MapPtrToProcess((LPVOID)pMountData, GetCallerProcess());

                    pReq->ffxStat = FfxDriverDiskParameterGet(pReq->hDisk,
                        pReq->id, pVBFStats, pReq->ulBuffLen);

                    UnMapPtr(pVBFStats->pMountData);

                    /*  Restore the original pointer in the client buffer
                    */
                    pVBFStats->pMountData = pMountData;

                    UnMapPtr(pTmp);

                    break;
                }

                /*  If pMountData was NULL, we will fall out of this IF
                    conditional, and the default handler will take care
                    of things.
                */
            }
            else if(((pReq->id == FFXPARAM_STATS_FML) ||
                (pReq->id == FFXPARAM_STATS_FMLRESET)) && pTmp)
            {
                FFXFMLSTATS    *pFMLStats = pTmp;

                /*  The FML decoder will only handle things if a pBlockStats
                    pointer is specified.  We can only know that if we decode
                    a bunch of stuff early on.  Even though we have done half
                    the work be decoding things, if we find that it really
                    should be handled in the FML decoder, pass it though at
                    this point, and avoid duplicating even more code.
                */
                if(pFMLStats->pBlockStats)
                {
                    ffxStat = FfxCe5DispatchFmlIoctl(hDisk, pReqHdr, dwLen);

                    UnMapPtr(pTmp);

                    break;
                }

                /*  If pBlockStats was NULL, we will fall out of this IF
                    conditional, and the default handler will take care
                    of things.
                */
            }

            pReq->ffxStat = FfxDriverDiskParameterGet(pReq->hDisk,
                pReq->id, pTmp, pReq->ulBuffLen);

            UnMapPtr(pTmp);

            break;
        }

        case FXIOFUNC_EXTDRVRFW_GETCONFIGINFO:
        {
            REQ_DRIVERGETCONFIGINFO    *pReq = (REQ_DRIVERGETCONFIGINFO*)pReqHdr;
            DCLMUTEXINFO               *pDMS = NULL;
            DCLSEMAPHOREINFO           *pDSS = NULL;
            DCLRDWRSEMAPHOREINFO       *pRWS = NULL;

            if(pReq->req.ulReqLen != sizeof(*pReq))
                goto BadPacketLength;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxConfigurationInfo()\n"));

            if(pReq->pCI->MutStats.pDMI)
            {
                pDMS = pReq->pCI->MutStats.pDMI;
                pReq->pCI->MutStats.pDMI = MapPtrToProcess((LPVOID)pReq->pCI->MutStats.pDMI, GetCallerProcess());
            }

            if(pReq->pCI->SemStats.pDSI)
            {
                pDSS = pReq->pCI->SemStats.pDSI;
                pReq->pCI->SemStats.pDSI = MapPtrToProcess((LPVOID)pReq->pCI->SemStats.pDSI, GetCallerProcess());
            }

            if(pReq->pCI->RWSemStats.pRWI)
            {
                pRWS = pReq->pCI->RWSemStats.pRWI;
                pReq->pCI->RWSemStats.pRWI = MapPtrToProcess((LPVOID)pReq->pCI->RWSemStats.pRWI, GetCallerProcess());
            }

            pReq->ffxStat = FfxConfigurationInfo(pReq->pCI, pReq->fVerbose, pReq->fReset);

            if(pReq->pCI->MutStats.pDMI)
            {
                UnMapPtr(pReq->pCI->MutStats.pDMI);
                pReq->pCI->MutStats.pDMI = pDMS;
            }

            if(pReq->pCI->SemStats.pDSI)
            {
                UnMapPtr(pReq->pCI->SemStats.pDSI);
                pReq->pCI->SemStats.pDSI = pDSS;
            }

            if(pReq->pCI->RWSemStats.pRWI)
            {
                UnMapPtr(pReq->pCI->RWSemStats.pRWI);
                pReq->pCI->RWSemStats.pRWI = pRWS;
            }

            break;
        }

        case DCLIOFUNC_STATS_REQUEST:
        {
          #if DCLCONF_OUTPUT_ENABLED
            FFXDISKHANDLE       hDisk;
            unsigned            nDiskNum;
            DCLREQ_STATS       *pReq = (DCLREQ_STATS*)pReqHdr;
            DCLREQ_STATS        Req = *pReq;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch DCLIOFUNC_STATS_REQUEST\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
                goto BadPacketLength;
 
            nDiskNum = FfxOsDeviceNameToDiskNumber(pReq->szDeviceName);
            if(nDiskNum == UINT_MAX)
            {
                ffxStat = DCLSTAT_UNKNOWNDEVICE;
                break;
            }

            hDisk = FfxDriverDiskHandle(FfxDriverHandle(1), nDiskNum);
            if(!hDisk)
            {
                ffxStat = DCLSTAT_DISKNUMBERINVALID;
                break;
            }

            Req.pDescBuffer = MapPtrToProcess((LPVOID)pReq->pDescBuffer, GetCallerProcess());
            Req.pDataBuffer = MapPtrToProcess((LPVOID)pReq->pDataBuffer, GetCallerProcess());

            pReq->ResultStatus = FfxDriverPerfLogDispatch(hDisk, &Req);

            UnMapPtr(Req.pDataBuffer);
            UnMapPtr(Req.pDescBuffer);

            break;            
            
          #else
          
            ffxStat = DCLSTAT_FEATUREDISABLED;
            break;
            
          #endif
        }
        
      #endif /* _WIN32_WCE < 600 */

        default:
        {
          #if _WIN32_WCE < 600
            ffxStat = FfxCe5DispatchFmlIoctl(hDisk, pReqHdr, dwLen);

            /*  If we are using CE 5 or earlier, and the function above did
                not handle the request, pass it to the general Driver
                Framework dispatcher.  However if the function above did
                handle it (whether it worked or failed), don't try calling
                into the Driver Framework.
            */
            if(ffxStat == FFXSTAT_APIUNHANDLEDFUNCTION)
                ffxStat = FfxDriverIoctlMasterDispatch(pReqHdr);

          #else

            ffxStat = FfxDriverIoctlMasterDispatch(pReqHdr);

          #endif
        }
    }

    goto Cleanup;
    
#if _WIN32_WCE < 600
  BadPacketLength:
    FFXPRINTF(1, ("FfxCeDispatchIoctl() Bad I/O request packet length for function 0x%x\n", pReqHdr->ioFunc));
    ffxStat = FFXSTAT_APIBADREQUESTBLOCKLEN;
#endif    

  Cleanup:

    DclProfilerLeave(0);

    return (ffxStat == FFXSTAT_SUCCESS);
}


#if _WIN32_WCE < 600
/*-------------------------------------------------------------------
    CopyToClientBuffer()

    Description
        This function copies data into the specified client buffer,
        first performing the appropriate pointer mapping.

    Parameters
        pDest    - A pointer to the client buffer.
        pSource  - A pointer to the data to copy.
        ulLength - The length to copy.

    Return Value
        None
-------------------------------------------------------------------*/
static void CopyToClientBuffer(
    void       *pDest,
    void       *pSource,
    D_UINT32    ulLength)
{
    DclAssert(pSource);
    DclAssert(pDest);
    DclAssert(ulLength);

    pDest = MapPtrToProcess((LPVOID) pDest, GetCallerProcess());
    DclAssert(pDest);

    DclMemCpy(pDest, pSource, ulLength);

    UnMapPtr(pDest);

    return;
}
#endif



