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

    This module contains the Ioctl dispatcher routine used by the Driver
    Framework to dispatch Driver Framework specific functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvdisp.c $
    Revision 1.21  2009/07/21 21:07:12Z  garyp
    Merged from the v4.0 branch.  Modified the shutdown processes to take
    a mode parameter.
    Revision 1.20  2009/04/09 21:26:37Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.19  2009/03/31 18:06:51Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.18  2009/02/09 02:41:39Z  garyp
    Merged from the v4.0 branch.  Moved DCL specific IOCTLs into their own
    function in DCL.  Reworked the stats related status codes so cross-product
    translations are not necessary.  Added support for FfxDriverOptionGet().
    Revision 1.17  2008/07/23 20:04:32Z  keithg
    Added type cast to placate older MSVS tools.
    Revision 1.16  2008/05/23 02:15:52Z  garyp
    Merged from the WinMobile branch.
    Revision 1.15.1.2  2008/05/23 02:15:52Z  garyp
    Updated the stats functions to use a device name rather than a DISK number.
    Added handlers for the profiler requests.
    Revision 1.15  2008/04/26 02:09:19Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.14  2008/03/22 21:31:08Z  Garyp
    Updated to support more Driver Framework functions.
    Revision 1.13  2008/01/13 07:26:24Z  keithg
    Function header updates to support autodoc.
    Revision 1.12  2007/12/26 02:11:08Z  Garyp
    Function renamed for clarity.
    Revision 1.11  2007/12/14 21:58:13Z  Garyp
    Updated the stats functionality to support querying for sub-type
    information.
    Revision 1.10  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.9  2007/06/22 20:44:00Z  Garyp
    Added support for dispatching generic IOCTL requests.
    Revision 1.8  2007/04/07 03:40:07Z  Garyp
    Added support for "ParameterSet".
    Revision 1.7  2007/01/10 01:44:48Z  Garyp
    Minor function renaming exercise.
    Revision 1.6  2006/11/10 20:16:18Z  Garyp
    Added "ParameterGet" and "Handle" support to the external API and removed
    "Stats" support.
    Revision 1.5  2006/05/08 01:48:17Z  Garyp
    Finalized the statistics interfaces.
    Revision 1.4  2006/02/20 22:44:14Z  Garyp
    Modified to use the newly updated external API mechanisms.
    Revision 1.3  2006/02/12 21:29:53Z  Garyp
    Temporarily disabled -- work-in-progress.
    Revision 1.2  2006/01/02 06:42:22Z  Garyp
    Added/updated debugging code -- no functional changes.
    Revision 1.1  2005/11/06 09:11:40Z  Pauli
    Initial revision
    Revision 1.2  2005/11/06 09:11:40Z  Garyp
    Modified to pass the fVerbose and fReset parameters.
    Revision 1.4  2004/12/30 17:32:41Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.3  2004/11/29 18:51:39Z  GaryP
    Moved the general ioctl dispatcher code to drvioctl.c, and use this module
    for dispatching Driver Framework specific functions only.
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxapireq.h>
#include <drvfwreq.h>
#include <fxstats.h>
#include <fxperflog.h>


/*-------------------------------------------------------------------
    Protected: FfxDriverIoctlDispatch()

    This function dispatches Driver Framework specific API
    function requests.

    Parameters:
        pExt - A pointer to the request block for the function.

    Return Value:
        Returns an FFXSTATUS value indicating the status of the
        request.  Note that this status value indicates whether the
        IOCTL request and interface are working properly, NOT whether
        the actual function call was successful.  If this function
        returns FFXSTAT_SUCCESS, then the returned request structure
        must be examined to see if the actual API function was
        successful.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverIoctlDispatch(
    FFXIOREQUEST   *pExt)
{
    if(!pExt)
    {
        DclError();
        return FFXSTAT_APIBADREQUESTPOINTER;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 2, 0),
        "FfxDriverIoctlDispatch() Function 0x%x\n", pExt->ioFunc));

    switch(pExt->ioFunc)
    {
        case FXIOFUNC_EXTDRVRFW_GETCONFIGINFO:
        {
            REQ_DRIVERGETCONFIGINFO *pReq = (REQ_DRIVERGETCONFIGINFO*)pExt;

            if(pReq->req.ulReqLen != sizeof(REQ_DRIVERGETCONFIGINFO))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxConfigurationInfo()\n"));

            pReq->ffxStat = FfxConfigurationInfo(pReq->pCI, pReq->fVerbose, pReq->fReset);

            return FFXSTAT_SUCCESS;
        }

                /*******************************************************\
                 *                                                     *
                 *              Dispatch DISK Functions                *
                 *                                                     *
                \*******************************************************/


        case FXIOFUNC_EXTDRVRFW_DISKCREATEPARAM:
        {
            REQ_DRVRFW_DISKCREATEPARAM *pReq = (REQ_DRVRFW_DISKCREATEPARAM*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverDiskCreateParam()\n"));

            pReq->hDisk = FfxDriverDiskCreateParam(pReq->pFDI, pReq->pConf, pReq->pHook);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTDRVRFW_DISKDESTROY:
        {
            REQ_DRVRFW_DISKDESTROY *pReq = (REQ_DRVRFW_DISKDESTROY*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverDiskDestroy()\n"));

            pReq->ffxStat = FfxDriverDiskDestroy(pReq->hDisk);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTDRVRFW_DISKHANDLE:
        {
            REQ_DRVRFW_DISKHANDLE *pReq = (REQ_DRVRFW_DISKHANDLE*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverDiskHandle()\n"));

            pReq->hDisk = FfxDriverDiskHandle(pReq->pFDI, pReq->nDiskNum);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTDRVRFW_DISKGEOMETRY:
        {
            REQ_DRVRFW_DISKGEOMETRY *pReq = (REQ_DRVRFW_DISKGEOMETRY*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverDiskGeometry()\n"));

            pReq->ffxStat = FfxDriverDiskGeometry(pReq->hDisk, pReq->pGeometry);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTDRVRFW_DISKENUMERATE:
        {
            REQ_DRVRFW_DISKENUMERATE *pReq = (REQ_DRVRFW_DISKENUMERATE*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverDiskEnumerate()\n"));

            pReq->hDisk = FfxDriverDiskEnumerate(pReq->pFDI, pReq->hDisk);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTDRVRFW_DISKGETPARAM:
        {
            REQ_DRVRFW_DISKPARAMGET *pReq = (REQ_DRVRFW_DISKPARAMGET*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverDiskParameterGet()\n"));

            pReq->ffxStat = FfxDriverDiskParameterGet(pReq->hDisk, pReq->id, pReq->pBuffer, pReq->ulBuffLen);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTDRVRFW_DISKSETPARAM:
        {
            REQ_DRVRFW_DISKPARAMSET *pReq = (REQ_DRVRFW_DISKPARAMSET*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverDiskParameterSet()\n"));

            pReq->ffxStat = FfxDriverDiskParameterSet(pReq->hDisk, pReq->id, pReq->pBuffer, pReq->ulBuffLen);

            return FFXSTAT_SUCCESS;
        }

                /*******************************************************\
                 *                                                     *
                 *             Dispatch DEVICE Functions               *
                 *                                                     *
                \*******************************************************/

        case FXIOFUNC_EXTDRVRFW_DEVCREATEPARAM:
        {
            REQ_DRVRFW_DEVCREATEPARAM *pReq = (REQ_DRVRFW_DEVCREATEPARAM*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverDeviceCreateParam()\n"));

            pReq->hDev = FfxDriverDeviceCreateParam(pReq->pFDI, pReq->pConf, pReq->pHook);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTDRVRFW_DEVDESTROY:
        {
            REQ_DRVRFW_DEVDESTROY *pReq = (REQ_DRVRFW_DEVDESTROY*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverDeviceDestroy()\n"));

            pReq->ffxStat = FfxDriverDeviceDestroy(pReq->hDev);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTDRVRFW_DEVHANDLE:
        {
            REQ_DRVRFW_DEVHANDLE *pReq = (REQ_DRVRFW_DEVHANDLE*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverDeviceHandle()\n"));

            pReq->hDev = FfxDriverDeviceHandle(pReq->pFDI, pReq->nDevNum);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTDRVRFW_DEVGEOMETRY:
        {
            REQ_DRVRFW_DEVGEOMETRY *pReq = (REQ_DRVRFW_DEVGEOMETRY*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverDeviceGeometry()\n"));

            pReq->ffxStat = FfxDriverDeviceGeometry(pReq->hDev, pReq->pGeometry);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTDRVRFW_DEVENUMERATE:
        {
            REQ_DRVRFW_DEVENUMERATE *pReq = (REQ_DRVRFW_DEVENUMERATE*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverDeviceEnumerate()\n"));

            pReq->hDev = FfxDriverDeviceEnumerate(pReq->pFDI, pReq->hDev);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTDRVRFW_DEVSETTINGS:
        {
            REQ_DRVRFW_DEVSETTINGS *pReq = (REQ_DRVRFW_DEVSETTINGS*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverDeviceSettings() pConf=%P\n", pReq->pConf));

            pReq->ffxStat = FfxDriverDeviceSettings(pReq->pConf);

            return FFXSTAT_SUCCESS;
        }

      #if FFXCONF_BBMSUPPORT

        case FXIOFUNC_EXTDRVRFW_DEVBBMSETTINGS:
        {
            REQ_DRVRFW_DEVBBMSETTINGS *pReq = (REQ_DRVRFW_DEVBBMSETTINGS*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverDeviceBbmSettings() pConf=%P\n", pReq->pConf));

            pReq->ffxStat = FfxDriverDeviceBbmSettings(pReq->pConf);

            return FFXSTAT_SUCCESS;
        }

      #endif


      #if FFXCONF_ALLOCATORSUPPORT

                /*******************************************************\
                 *                                                     *
                 *           Dispatch ALLOCATOR Functions              *
                 *                                                     *
                \*******************************************************/

        case FXIOFUNC_EXTDRVRFW_ALLOCATORCREATE:
        {
            REQ_DRVRFW_ALLOCATORCREATE*pReq = (REQ_DRVRFW_ALLOCATORCREATE*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverAllocatorCreate()\n"));

            pReq->ffxStat = FfxDriverAllocatorCreate(pReq->hDisk);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTDRVRFW_ALLOCATORDESTROY:
        {
            REQ_DRVRFW_ALLOCATORDESTROY *pReq = (REQ_DRVRFW_ALLOCATORDESTROY*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverAllocatorDestroy() hDisk=%P Flags=%lU\n",
                pReq->hDisk, pReq->ulFlags));

            pReq->ffxStat = FfxDriverAllocatorDestroy(pReq->hDisk, pReq->ulFlags);

            return FFXSTAT_SUCCESS;
        }
      #endif

                /*******************************************************\
                 *                                                     *
                 *          Dispatch MISCELLANOUS Functions            *
                 *                                                     *
                \*******************************************************/

        case FXIOFUNC_EXTDRVRFW_OPTIONGET:
        {
            REQ_DRVRFW_OPTIONGET *pReq = (REQ_DRVRFW_OPTIONGET*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxDriverOptionGet()\n"));

            pReq->fResult = FfxDriverOptionGet(pReq->option, pReq->handle, pReq->pBuffer, pReq->ulBuffLen);

            return FFXSTAT_SUCCESS;
        }

        default:
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "FfxDriverIoctlDispatch() Function 0x%x unknown\n", pExt->ioFunc));

            DclProductionError();

            return FFXSTAT_APIUNKNOWNDRVRFWFUNCTION;
        }
    }
}


/*-------------------------------------------------------------------
    Protected: FfxDriverGenericIoctlDispatch()

    This function dispatches generic function requests which came
    from outside FlashFX, but still may be processed by FlashFX.

    Parameters:
        pReqHdr - A pointer to the request block for the function.

    Return Value:
        Returns an FFXSTATUS value indicating the status of the
        request.  Note that this status value indicates whether the
        IOCTL request and interface are working properly, NOT whether
        the actual function call was successful.  If this function
        returns FFXSTAT_SUCCESS, then the returned request structure
        must be examined to see if the actual API function was
        successful.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverGenericIoctlDispatch(
    FFXIOREQUEST   *pReqHdr)
{
    if(!pReqHdr)
    {
        DclError();
        return DCLSTAT_BADPARAMETER;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 2, 0),
        "FfxDriverGenericIoctlDispatch() Function 0x%x\n", pReqHdr->ioFunc));

    /*  Handle any DCL specific requests first.  These could have
        originated in generic DCL code such as FSIOTEST.

        Note that unless there is an interface error, we continue
        on and pass these requests to the FlashFX specific handlers
        as well.  This is to allow requests such as "stats reset"
        to apply at all levels of the product.
    */
    if(DCLIO_GETPRODUCT(pReqHdr->ioFunc) == PRODUCTNUM_DCL)
    {
        FFXSTATUS   ffxStat;

        ffxStat = DclIoctlDispatch(pReqHdr);
        if(ffxStat != DCLSTAT_SUCCESS && ffxStat != DCLSTAT_FEATUREDISABLED)
            return ffxStat;
    }

    switch(pReqHdr->ioFunc)
    {
        case DCLIOFUNC_STATS_RESET:
        {
            DCLREQ_STATSRESET  *pReq = (DCLREQ_STATSRESET*)pReqHdr;
            FFXDISKHANDLE       hDisk;
            unsigned            nDiskNum;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch DCLIOFUNC_STATS_RESET\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            nDiskNum = FfxOsDeviceNameToDiskNumber(pReq->szDeviceName);
            if(nDiskNum == UINT_MAX)
                return DCLSTAT_UNKNOWNDEVICE;

            hDisk = FfxDriverDiskHandle(FfxDriverHandle(1), nDiskNum);
            if(!hDisk)
                return DCLSTAT_DISKNUMBERINVALID;

            FfxDriverDiskParameterGet(hDisk, FFXPARAM_STATS_RESETALL, NULL, 0);

            return DCLSTAT_SUCCESS;
        }

        case DCLIOFUNC_STATS_REQUEST:
        {
          #if DCLCONF_OUTPUT_ENABLED
            DCLREQ_STATS       *pReq = (DCLREQ_STATS*)pReqHdr;
            FFXDISKHANDLE       hDisk;
            unsigned            nDiskNum;

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch DCLIOFUNC_STATS_REQUEST\n"));

            if(pReq->ior.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return DCLSTAT_BADSTRUCLEN;
            }

            nDiskNum = FfxOsDeviceNameToDiskNumber(pReq->szDeviceName);
            if(nDiskNum == UINT_MAX)
                return DCLSTAT_UNKNOWNDEVICE;

            hDisk = FfxDriverDiskHandle(FfxDriverHandle(1), nDiskNum);
            if(!hDisk)
                return DCLSTAT_DISKNUMBERINVALID;

            pReq->ResultStatus = FfxDriverPerfLogDispatch(hDisk, pReq);

            return DCLSTAT_SUCCESS;
          #else
            return DCLSTAT_FEATUREDISABLED;
          #endif
        }

        default:
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 2, 0),
                "FfxDriverGenericIoctlDispatch() Function %x unknown\n", pReqHdr->ioFunc));

            return DCLSTAT_UNSUPPORTEDFUNCTION;
        }
    }
}




