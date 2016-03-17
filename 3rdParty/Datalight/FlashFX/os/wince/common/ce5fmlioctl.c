/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

    This module contains IOCTL dispatch routines for FlashFX specific IOCTLs
    for the FML (and any lower) layers.  This module is separately abstracted
    so that it can be linked into environments where only the FML part of
    FlashFX is being used.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ce5fmlioctl.c $
    Revision 1.6  2010/01/23 21:45:25Z  garyp
    Fixed so that for CE versions prior to v6, stats requests work properly.
    Revision 1.5  2009/07/22 00:44:07Z  garyp
    Merged from the v4.0 branch.  Added OTP support.  Added support for the
    "Read/WriteUncorrectedPages" IOCTLs and removed support for the
    "ReadCorrectedPages" IOCTL.
    Revision 1.4  2009/04/09 03:52:33Z  garyp
    Renamed a structure to avoid AutoDoc naming collisions.
    Revision 1.3  2008/05/07 01:20:23Z  garyp
    Updated to accommodate the new format for FML block statistics.
    Revision 1.2  2008/03/27 18:28:55Z  Garyp
    Separated the read/write tags and tagged pages from the other handlers.
    Revision 1.1  2007/12/26 22:31:40Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <windev.h>
#include <devload.h>
#include <diskio.h>

#if _WIN32_WCE < 600

#include <flashfx.h>
#include <fxdriver.h>
#include <fxfmlapi.h>
#include <fxapireq.h>
#include <fxperflog.h>
#include <fmlreq.h>
#include <drvfwreq.h>
#include <dlstats.h>
#include <fxconfig.h>
#include <ffxwce.h>

static void   CopyToClientBuffer(void *pDest, void *pSource, D_UINT32 ulLength);


/*-------------------------------------------------------------------
    NOTE:  These IOCTL handlers are virtually identical to the
           generic handlers, with the exception that they do
           MapPtrToProcess() as needed.  If not for this
           requirement, the default handlers would be used.

    NOTE:  This function is only required for CE 5 or earlier.

    Return:
        Returns an FFXSTATUS value indicating the request status.
        If the value is FFXSTAT_APIUNHANDLEDFUNCTION, then the API
        requires no special handling and the default dispatcher
        interface should be used.  If FFXSTAT_SUCCESS is returned
        then the request was successfully processed and the results
        are reported in the pReqHdr structure.  If any other value
        is returned, then it indicates an interface error.
-------------------------------------------------------------------*/
FFXSTATUS FfxCe5DispatchFmlIoctl(
    FFXDISKHANDLE   hDisk,
    FFXIOREQUEST   *pReqHdr,
    DWORD           dwLen)
{
    FFXDISKINFO    *pDisk = *hDisk;
    FFXSTATUS       ffxStat = FFXSTAT_APIBADREQUESTBLOCKLEN;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 2, 0),
        "FfxCe5DispatchFmlIoctl() hDisk=%P pReq=%P dwLen=%lX\n",
        hDisk, pReqHdr, dwLen));

    switch (pReqHdr->ioFunc)
    {
        case FXIOFUNC_EXTFML_IOREQUEST:
        {
            REQ_FMLIOREQUEST *pReq = (REQ_FMLIOREQUEST*)pReqHdr;

            DclAssert(pReq->pIOReq);

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "CE Dispatch FfxFmlIORequest() hFML=%P pIOReq=%P Func=%x\n",
                pReq->hFML, pReq->pIOReq, pReq->pIOReq->ioFunc));

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                goto DispatchIoctlCleanup;
            }

            switch(pReq->pIOReq->ioFunc)
            {
              #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
                case FXIOFUNC_FML_READ_CONTROLDATA:
                case FXIOFUNC_FML_WRITE_CONTROLDATA:
                    DclAssert(sizeof(FFXIOR_FML_READ_CONTROLDATA)   == sizeof(FFXIOR_FML_READ_PAGES));
                    DclAssert(sizeof(FFXIOR_FML_WRITE_CONTROLDATA)  == sizeof(FFXIOR_FML_READ_PAGES));
                    /*  Fall through...
                    */
              #endif
              #if FFXCONF_NANDSUPPORT
                case FXIOFUNC_FML_READ_SPARES:
                case FXIOFUNC_FML_WRITE_SPARES:
                    DclAssert(sizeof(FFXIOR_FML_READ_SPARES)        == sizeof(FFXIOR_FML_READ_PAGES));
                    DclAssert(sizeof(FFXIOR_FML_WRITE_SPARES)       == sizeof(FFXIOR_FML_READ_PAGES));
                    /*  Fall through...
                    */
              #endif
                case FXIOFUNC_FML_WRITE_PAGES:
                    DclAssert(sizeof(FFXIOR_FML_WRITE_PAGES)        == sizeof(FFXIOR_FML_READ_PAGES));
                    /*  Fall through...
                    */
                case FXIOFUNC_FML_READ_PAGES:
                {
                    FFXIOR_FML_READ_PAGES NewReq = *(FFXIOR_FML_READ_PAGES*)pReq->pIOReq;

                    /*  All the cases above use the same structure layout and
                        length as "READ_PAGES", so handle them together.
                    */

                    NewReq.pPageData = MapPtrToProcess((LPVOID)NewReq.pPageData, GetCallerProcess());
                    DclAssert(NewReq.pPageData);

                    pReq->ioStat = FfxFmlIORequest(pReq->hFML, &NewReq.ior);

                    UnMapPtr(NewReq.pPageData);

                    ffxStat = FFXSTAT_SUCCESS;
                    break;
                }

               #if FFXCONF_NANDSUPPORT
                case FXIOFUNC_FML_WRITE_TAGGEDPAGES:
                    DclAssert(sizeof(FFXIOR_FML_WRITE_TAGGEDPAGES) == sizeof(FFXIOR_FML_READ_TAGGEDPAGES));
                    /*  Fall through...
                    */
                case FXIOFUNC_FML_READ_TAGGEDPAGES:
                {
                    FFXIOR_FML_READ_TAGGEDPAGES NewReq = *(FFXIOR_FML_READ_TAGGEDPAGES*)pReq->pIOReq;

                    /*  All the cases above use the same structure layout and
                        length as "READ_TAGGEDPAGES", so handle them together.
                    */

                    NewReq.pPageData = MapPtrToProcess((LPVOID)NewReq.pPageData, GetCallerProcess());
                    DclAssert(NewReq.pPageData);

                    /*  pTag is optional, only map it if it's supplied.
                    */
                    if(NewReq.pTag)
                    {
                        NewReq.pTag = MapPtrToProcess((LPVOID)NewReq.pTag, GetCallerProcess());
                        DclAssert(NewReq.pTag);
                    }

                    pReq->ioStat = FfxFmlIORequest(pReq->hFML, &NewReq.ior);

                    UnMapPtr(NewReq.pPageData);

                    /*  pTag is optional, only unmap it if it's supplied.
                    */
                    if(NewReq.pTag)
                    {
                        UnMapPtr(NewReq.pTag);
                    }

                    ffxStat = FFXSTAT_SUCCESS;
                    break;
                }

              #if FFXCONF_NANDSUPPORT
                case FXIOFUNC_FML_READ_UNCORRECTEDPAGES:
                case FXIOFUNC_FML_WRITE_UNCORRECTEDPAGES:
                    DclAssert(sizeof(FFXIOR_FML_READ_UNCORRECTEDPAGES)  == sizeof(FFXIOR_FML_READ_RAWPAGES));
                    DclAssert(sizeof(FFXIOR_FML_WRITE_UNCORRECTEDPAGES) == sizeof(FFXIOR_FML_READ_RAWPAGES));
                    /*  Fall through...
                    */
              #endif
                case FXIOFUNC_FML_WRITE_RAWPAGES:
                    DclAssert(sizeof(FFXIOR_FML_WRITE_RAWPAGES) == sizeof(FFXIOR_FML_READ_RAWPAGES));
                    /*  Fall through...
                    */
                case FXIOFUNC_FML_READ_RAWPAGES:
                {
                    FFXIOR_FML_READ_RAWPAGES NewReq = *(FFXIOR_FML_READ_RAWPAGES*)pReq->pIOReq;

                    /*  All the cases above use the same structure layout and
                        length as "READ_RAWPAGES", so handle them together.
                    */

                    NewReq.pPageData = MapPtrToProcess((LPVOID)NewReq.pPageData, GetCallerProcess());
                    DclAssert(NewReq.pPageData);

                    /*  pSpare is optional, only map it if it's supplied.
                    */
                    if (NewReq.pSpare)
                    {
                        NewReq.pSpare = MapPtrToProcess((LPVOID)NewReq.pSpare, GetCallerProcess());
                        DclAssert(NewReq.pSpare);
                    }

                    pReq->ioStat = FfxFmlIORequest(pReq->hFML, &NewReq.ior);

                    UnMapPtr(NewReq.pPageData);

                    /*  pSpare is optional, only unmap it if it's supplied.
                    */
                    if (NewReq.pSpare)
                    {
                        UnMapPtr(NewReq.pSpare);
                    }

                    ffxStat = FFXSTAT_SUCCESS;
                    break;
                }

                case FXIOFUNC_FML_WRITE_TAGS:
                    DclAssert(sizeof(FFXIOR_FML_WRITE_TAGS) == sizeof(FFXIOR_FML_READ_TAGS));
                    /*  Fall through...
                    */

                case FXIOFUNC_FML_READ_TAGS:
                {
                    FFXIOR_FML_READ_TAGS NewReq = *(FFXIOR_FML_READ_TAGS*)pReq->pIOReq;

                    NewReq.pTag = MapPtrToProcess((LPVOID)NewReq.pTag, GetCallerProcess());
                    DclAssert(NewReq.pTag);

                    pReq->ioStat = FfxFmlIORequest(pReq->hFML, &NewReq.ior);

                    UnMapPtr(NewReq.pTag);

                    ffxStat = FFXSTAT_SUCCESS;
                    break;
                }

              #if FFXCONF_OTPSUPPORT
                case FXIOFUNC_FML_OTP_READPAGES:
                {
                    FFXIOR_FML_READ_OTPPAGES NewReq = *(FFXIOR_FML_READ_OTPPAGES*)pReq->pIOReq;

                    NewReq.pPageData = MapPtrToProcess((LPVOID)NewReq.pPageData, GetCallerProcess());
                    DclAssert(NewReq.pPageData);

                    pReq->ioStat = FfxFmlIORequest(pReq->hFML, &NewReq.ior);

                    UnMapPtr(NewReq.pPageData);

                    ffxStat = FFXSTAT_SUCCESS;
                    break;
                }
                case FXIOFUNC_FML_OTP_WRITEPAGES:
                {
                    FFXIOR_FML_WRITE_OTPPAGES NewReq = *(FFXIOR_FML_WRITE_OTPPAGES*)pReq->pIOReq;

                    NewReq.pPageData = MapPtrToProcess((LPVOID)NewReq.pPageData, GetCallerProcess());
                    DclAssert(NewReq.pPageData);

                    pReq->ioStat = FfxFmlIORequest(pReq->hFML, &NewReq.ior);

                    UnMapPtr(NewReq.pPageData);

                    ffxStat = FFXSTAT_SUCCESS;
                    break;
                }
              #endif  /* FFXCONF_OTPSUPPORT */

              #endif  /* FFXCONF_NANDSUPPORT */

                default:
                {
                    pReq->ioStat = FfxFmlIORequest(pReq->hFML, pReq->pIOReq);

                    ffxStat = FFXSTAT_SUCCESS;
                    break;
                }
            }

            break;
        }

        case FXIOFUNC_EXTFML_DEVICEINFO:
        {
            REQ_FMLDEVICEINFO  *pReq = (REQ_FMLDEVICEINFO*)pReqHdr;
            FFXFMLDEVINFO       FmlDevInfo;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "CE Dispatch FfxFmlDeviceInfo() DEV%u pBuff=%P\n",
                pReq->nDeviceNum, pReq->pFmlDeviceInfo));

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                goto DispatchIoctlCleanup;
            }

            pReq->ffxStat = FfxFmlDeviceInfo(pReq->nDeviceNum, &FmlDevInfo);

            CopyToClientBuffer(pReq->pFmlDeviceInfo, &FmlDevInfo, sizeof(*pReq->pFmlDeviceInfo));

            ffxStat = FFXSTAT_SUCCESS;

            break;
        }

        case FXIOFUNC_EXTFML_DISKINFO:
        {
            REQ_FMLINFO        *pReq = (REQ_FMLINFO*)pReqHdr;
            FFXFMLINFO          FmlInfo;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "CE Dispatch FfxFmlDiskInfo(), hFML=%P pBuff=%P\n",
                pReq->hFML, pReq->pFmlInfo));

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                goto DispatchIoctlCleanup;
            }

            pReq->ffxStat = FfxFmlDiskInfo(pReq->hFML, &FmlInfo);

            CopyToClientBuffer(pReq->pFmlInfo, &FmlInfo, sizeof(*pReq->pFmlInfo));

            ffxStat = FFXSTAT_SUCCESS;

            break;
        }

        case DCLIOFUNC_STATS_REQUEST:
        {
          #if DCLCONF_OUTPUT_ENABLED
            FFXFMLHANDLE        hFML;
            unsigned            nDiskNum;
            DCLREQ_STATS       *pReq = (DCLREQ_STATS*)pReqHdr;
            DCLREQ_STATS        Req = *pReq;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch DCLIOFUNC_STATS_REQUEST\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                goto DispatchIoctlCleanup;
            }

            nDiskNum = FfxOsDeviceNameToDiskNumber(pReq->szDeviceName);
            if(nDiskNum == UINT_MAX)
            {
                ffxStat = DCLSTAT_UNKNOWNDEVICE;
                break;
            }

            hFML = FfxFmlHandle(nDiskNum);
            if(!hFML)
            {
                ffxStat = DCLSTAT_DISKNUMBERINVALID;
                break;
            }

            Req.pDescBuffer = MapPtrToProcess((LPVOID)pReq->pDescBuffer, GetCallerProcess());
            Req.pDataBuffer = MapPtrToProcess((LPVOID)pReq->pDataBuffer, GetCallerProcess());

            pReq->ResultStatus = FfxFmlPerfLogDispatch(hFML, &Req);

            UnMapPtr(Req.pDataBuffer);
            UnMapPtr(Req.pDescBuffer);
            
            ffxStat = DCLSTAT_SUCCESS;
            break;
            
          #else
          
            ffxStat = DCLSTAT_FEATUREDISABLED;
            break;
            
          #endif
        }

        case FXIOFUNC_EXTDRVRFW_DISKGETPARAM:
        {
            REQ_DRVRFW_DISKPARAMGET    *pReq = (REQ_DRVRFW_DISKPARAMGET*)pReqHdr;
            void                       *pTmp;

            /*  Normally Driver Framework requests are not handled at this
                level, however if it is a general request to manipulate FML
                statistics when erase counts are being reported or reset,
                specifically handle that request only.
            */

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                goto DispatchIoctlCleanup;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverGetParameter()\n"));

            pTmp = MapPtrToProcess((LPVOID)pReq->pBuffer, GetCallerProcess());

            if(((pReq->id == FFXPARAM_STATS_FML) ||
                (pReq->id == FFXPARAM_STATS_FMLRESET)) && pTmp)
            {
                FFXFMLSTATS    *pFMLStats = pTmp;

                if(pFMLStats->pBlockStats)
                {
                    void   *pOrigStats = pFMLStats->pBlockStats;

                    /*  Temporarily patch the pointer in the client buffer
                    */
                    pFMLStats->pBlockStats = MapPtrToProcess((LPVOID)pOrigStats, GetCallerProcess());
/* This code needs attention */
                    pReq->ffxStat = FfxFmlParameterGet((*pReq->hDisk)->hFML,
                        pReq->id, pFMLStats, pReq->ulBuffLen);

/*
                    pReq->ffxStat = FfxDriverDiskParameterGet(pReq->hDisk,
                        pReq->id, pFMLStats, pReq->ulBuffLen);
*/
                    UnMapPtr(pFMLStats->pBlockStats);

                    /*  Restore the original pointer in the client buffer
                    */
                    pFMLStats->pBlockStats = pOrigStats;

                    UnMapPtr(pTmp);

                    ffxStat = FFXSTAT_SUCCESS;

                    break;
                }

                /*  If pBlockStats was NULL, we will fall out of this
                    IF conditional, and the default handler in the calling
                    function will take care of things.
                */
            }

            UnMapPtr(pTmp);

            /*  Fall through
                        .
                        .
                        .
            */
        }

        default:
        {
            ffxStat = FFXSTAT_APIUNHANDLEDFUNCTION;
        }
    }

  DispatchIoctlCleanup:

    return ffxStat;
}


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

