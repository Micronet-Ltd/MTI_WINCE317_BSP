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

    This module contains the function dispatcher for the IOCTL interface
    to the FML.  The sister module for this code, (which builds the requests),
    is found in COMMON\EXTAPI\APIFML.C.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmldisp.c $
    Revision 1.18  2010/08/04 00:35:16Z  garyp
    Updated to use the DCLREQ_VERSIONCHECK structure defined in DCL.
    Revision 1.17  2009/07/24 02:36:09Z  garyp
    Merged from the v4.0 branch.  Added debug code.  Added support for
    FXIOFUNC_EXTFML_DEVICERANGE.
    Revision 1.16  2009/04/08 21:39:08Z  garyp
    Updated for AutoDoc -- no functional changes.
    Revision 1.15  2009/04/01 20:20:01Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.14  2009/03/09 19:47:00Z  thomd
    Cast parameter of FfxFmlHandle()
    Revision 1.13  2009/02/08 21:09:57Z  garyp
    Merged from the v4.0 branch.  Cleaned up the order in which things are
    dispatched to better support generic IOCTLs.
    Revision 1.11.1.7  2009/01/19 00:20:05Z  garyp
    Debug code updated -- no functional changes.
    Revision 1.11.1.6  2009/01/16 08:52:09Z  garyp
    Updated debug messages -- no functional changes.
    Revision 1.11.1.5  2009/01/14 05:07:25Z  garyp
    Reorganized how things are dispatched to better accommodate the DCL
    dispatcher.  Updated to use some simplified stats related status codes.
    Revision 1.11.1.4  2008/12/15 22:06:22Z  garyp
    Updated the FfxFmlCreate() function to take a flags parameter.  Adjusted
    for a number of fields which are now unsigned rather than D_UINT16.
    Revision 1.11.1.3  2008/09/06 18:33:44Z  garyp
    Added FfxFmlDeviceRangeEnumerate().
    Revision 1.11.1.2  2008/05/23 02:15:51Z  garyp
    Modified the stats interfaces to use a device name rather than a DISK
    number.
    Revision 1.11  2008/04/14 21:08:05Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.10  2008/03/25 19:59:31Z  Garyp
    Minor datatype changes.
    Revision 1.9  2008/01/13 07:36:40Z  keithg
    Function header updates to support autodoc.
    Revision 1.8  2007/12/26 02:36:01Z  Garyp
    Added FfxFmlIoctlMasterDispatch() to support dispatching everything for
    the FML and lower layers.
    Revision 1.7  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2006/11/08 17:46:43Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.5  2006/03/06 19:18:03Z  Garyp
    Added FfxFmlDeviceInfo() support.
    Revision 1.4  2006/02/20 06:15:39Z  Garyp
    Updated debugging code.
    Revision 1.3  2006/02/13 03:18:43Z  Garyp
    Updated to new external API interface.
    Revision 1.2  2006/02/01 20:57:02Z  Garyp
    Refactored such that the FML is literally just the flash mapping layer,
    other functionality is moved into the Device Manager layer.
    Revision 1.1  2005/05/02 17:38:08Z  Pauli
    Initial revision
    Revision 1.2  2005/05/02 18:38:07Z  Garyp
    Added external API support for FfxFmlWritePages.
    Revision 1.1  2005/10/02 02:29:38Z  Garyp
    Initial revision
    Revision 1.3  2004/12/30 23:08:28Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2004/02/22 19:11:52Z  garys
    Revision 1.1.1.4  2004/02/22 19:11:52  garyp
    Eliminated oemchanged() support.
    Revision 1.1.1.3  2004/01/25 08:43:46Z  garyp
    Debugging code updated.
    Revision 1.1.1.2  2003/12/04 23:22:04Z  garyp
    Modified to use the new oemmount() function.
    Revision 1.1  2003/11/22 03:26:12Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxver.h>
#include <fxapireq.h>
#include <fxfmlapi.h>
#include <fxstats.h>
#include <fxperflog.h>
#include <fmlreq.h>


static FFXSTATUS GenericIoctlDispatch(FFXIOREQUEST *pExt);


/*-------------------------------------------------------------------
    Protected: FfxFmlIoctlDispatch()

    This function dispatches API function requests.

    Parameters:
        pReq - A pointer to the request block for the function.

    Return Value:
       Returns an FFXSTATUS value indicating the status of the
       request.  Note that this status value indicates wheter the
       IOCTL request and interface are working properly, NOT whether
       the actual function call was successful.  If this function
       return FFXSTAT_SUCCESS, then the returned request structure
       must be examined to see if the actual API function was
       successful.
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlIoctlDispatch(
    FFXIOREQUEST   *pExt)
{
    FXIOFUNC        nFunc;
    FFXSTATUS       ffxStat;

    DclAssert(pExt);

    if(!pExt)
        return FFXSTAT_APIBADREQUESTPOINTER;

    nFunc = pExt->ioFunc;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 2, 0),
        "FfxFmlIoctlDispatch() Function 0x%x\n", nFunc));

    /*  It <could> be that the FML is the top-most layer of FlashFX which
        is being used.  In that case, we want to process certain generic
        requests at this level.  In the event that we are <not> the top-
        most layer, these requests will have already been filtered out.
    */
    ffxStat = GenericIoctlDispatch(pExt);
    if(ffxStat != DCLSTAT_UNSUPPORTEDFUNCTION)
        return ffxStat;

    switch(nFunc)
    {
        case FXIOFUNC_EXTFML_VERSIONCHECK:
        {
            DCLREQ_VERSIONCHECK *pVC = (DCLREQ_VERSIONCHECK*)pExt;

            /*  NOTE: The VersionCheck API is handled as part of the FML
                      interface because that is the highest layer of
                      FlashFX which is always guaranteed to be used.  VBF
                      and the Driver Framework are not always linked in.
            */

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FXIOFUNC_EXTFML_VERSIONCHECK"));

            if(pVC->ior.ulReqLen != sizeof(*pVC))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            DclAssert(pVC->ulSignature == FFXAPI_REQUESTSIGNATURE);

            if(pVC->ulSignature != FFXAPI_REQUESTSIGNATURE)
                return FFXSTAT_APIBADREQUESTSIGNATURE;

            pVC->ulVersion = VERSIONVAL;
            pVC->ulSignature = FFXAPI_REPLYSIGNATURE;

            DclAssert(DclStrLen(FFXBUILDNUM) < sizeof(pVC->szBuildNum));
            DclStrNCpy(pVC->szBuildNum, FFXBUILDNUM, sizeof(pVC->szBuildNum));

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTFML_CREATE:
        {
            REQ_FMLCREATE  *pReq = (REQ_FMLCREATE*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxFmlCreate(), DiskNum=%u StartBlock=%lX Count=%lX Flags=0x%x\n",
                pReq->nDiskNum, pReq->ulStartBlock, pReq->ulBlockCount, pReq->nFlags));

            pReq->hFML = FfxFmlCreate(pReq->hDisk, pReq->hFimDev, pReq->nDiskNum,
                                      pReq->ulStartBlock, pReq->ulBlockCount, pReq->nFlags);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTFML_DESTROY:
        {
            REQ_FMLDESTROY *pReq = (REQ_FMLDESTROY*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxFmlDestroy(), hFML=%P\n", pReq->hFML));

            pReq->ffxStat = FfxFmlDestroy(pReq->hFML);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTFML_HANDLE:
        {
            REQ_FMLHANDLE *pReq = (REQ_FMLHANDLE*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxFmlHandle() DiskNum=%u\n", pReq->nDiskNum));

            pReq->hFML = FfxFmlHandle(pReq->nDiskNum);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTFML_DEVICERANGE:
        {
            REQ_FMLDEVICERANGE  *pReq = (REQ_FMLDEVICERANGE*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxFmlDeviceRangeEnumerate() DEV%u ulBlock=%lU phFML=%P pulCount=%P\n",
                pReq->nDeviceNum, pReq->ulBlockNum, pReq->phFML, pReq->pulBlockCount));

            pReq->ffxStat = FfxFmlDeviceRangeEnumerate(pReq->nDeviceNum,
                        pReq->ulBlockNum, pReq->phFML, pReq->pulBlockCount);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTFML_DEVICEINFO:
        {
            REQ_FMLDEVICEINFO  *pReq = (REQ_FMLDEVICEINFO*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxFmlDeviceInfo() DEV%u pBuff=%P\n",
                pReq->nDeviceNum, pReq->pFmlDeviceInfo));

            pReq->ffxStat = FfxFmlDeviceInfo(pReq->nDeviceNum, pReq->pFmlDeviceInfo);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTFML_DISKINFO:
        {
            REQ_FMLINFO *pReq = (REQ_FMLINFO*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxFmlDiskInfo() hFML=%P pBuff=%P\n", pReq->hFML, pReq->pFmlInfo));

            pReq->ffxStat = FfxFmlDiskInfo(pReq->hFML, pReq->pFmlInfo);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTFML_PARAMETERGET:
        {
            REQ_FMLPARAMETERGET *pReq = (REQ_FMLPARAMETERGET*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxFmlParameterGet() hFML=%P ID=%u pBuff=%P BuffLen=%lU\n",
                pReq->hFML, pReq->id, pReq->pBuffer, pReq->ulBuffLen));

            pReq->ffxStat = FfxFmlParameterGet(pReq->hFML, pReq->id, pReq->pBuffer, pReq->ulBuffLen);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTFML_PARAMETERSET:
        {
            REQ_FMLPARAMETERSET *pReq = (REQ_FMLPARAMETERSET*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxFmlParameterSet() hFML=%P ID=%u pBuff=%P BuffLen=%lU\n",
                pReq->hFML, pReq->id, pReq->pBuffer, pReq->ulBuffLen));

            pReq->ffxStat = FfxFmlParameterSet(pReq->hFML, pReq->id, pReq->pBuffer, pReq->ulBuffLen);

            return FFXSTAT_SUCCESS;
        }

        case FXIOFUNC_EXTFML_IOREQUEST:
        {
            REQ_FMLIOREQUEST *pReq = (REQ_FMLIOREQUEST*)pExt;

            if(pReq->req.ulReqLen != sizeof(*pReq))
            {
                DclError();
                return FFXSTAT_APIBADREQUESTBLOCKLEN;
            }

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "Dispatch FfxFmlIORequest() hFML=%P pIOReq=%P\n",
                pReq->hFML, pReq->pIOReq));

            pReq->ioStat = FfxFmlIORequest(pReq->hFML, pReq->pIOReq);

            return FFXSTAT_SUCCESS;
        }

        default:
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 1, 0),
                "FfxFmlIoctlDispatch() Function 0x%x unknown\n", nFunc));

            DclProductionError();

            return FFXSTAT_APIUNKNOWNFMLFUNCTION;
        }
    }
}


/*-------------------------------------------------------------------
    Local: GenericIoctlDispatch()

    This function dispatches generic function requests which
    came from outside FlashFX.

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
static FFXSTATUS GenericIoctlDispatch(
    FFXIOREQUEST   *pExt)
{
    if(!pExt)
    {
        DclError();
        return DCLSTAT_BADPARAMETER;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 2, 0),
        "FfxFml:GenericIoctlDispatch() Function 0x%x\n", pExt->ioFunc));

    /*  Handle any DCL specific requests first.  These could have
        originated in generic DCL code such as FSIOTEST.

        Note that unless there is an interface error, we continue
        on and pass these requests to the FlashFX specific handlers
        as well.  This is to allow requests such as "stats reset"
        to apply at all levels of the product.
    */
    if(DCLIO_GETPRODUCT(pExt->ioFunc) == PRODUCTNUM_DCL)
    {
        FFXSTATUS   ffxStat;

        ffxStat = DclIoctlDispatch(pExt);
        if(ffxStat != DCLSTAT_SUCCESS && ffxStat != DCLSTAT_FEATUREDISABLED)
            return ffxStat;
    }

    switch(pExt->ioFunc)
    {
        FFXFMLHANDLE        hFML;

        case DCLIOFUNC_STATS_RESET:
        {
            DCLREQ_STATSRESET  *pReq = (DCLREQ_STATSRESET*)pExt;
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

            hFML = FfxFmlHandle(nDiskNum);
            if(!hFML)
                return DCLSTAT_DISKNUMBERINVALID;

            FfxFmlParameterGet(hFML, FFXPARAM_STATS_RESETALL, NULL, 0);

            return DCLSTAT_SUCCESS;
        }

        case DCLIOFUNC_STATS_REQUEST:
        {
          #if DCLCONF_OUTPUT_ENABLED
            DCLREQ_STATS       *pReq = (DCLREQ_STATS*)pExt;
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

            hFML = FfxFmlHandle(nDiskNum);
            if(!hFML)
                return DCLSTAT_DISKNUMBERINVALID;

            pReq->ResultStatus = FfxFmlPerfLogDispatch(hFML, pReq);

            return DCLSTAT_SUCCESS;
          #else
            return DCLSTAT_FEATUREDISABLED;
          #endif
        }

        default:
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 2, 0),
                "FfxFml:GenericIoctlDispatch() Function %x unknown\n", pExt->ioFunc));

            return DCLSTAT_UNSUPPORTEDFUNCTION;
        }
    }
}





