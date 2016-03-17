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

    This module contains the Driver Framework external API functions.  These
    functions are used in a fashion identical to that of the standard Driver
    Framework interface.  See the standard Driver Framework documentation for
    details on using these functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: extdrvfw.c $
    Revision 1.19  2010/04/29 00:04:20Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.18  2009/07/24 02:00:31Z  garyp
    Merged from the v4.0 branch.  Added debug code.  Modified the shutdown
    processes to take a mode parameter.
    Revision 1.17  2009/03/31 19:28:01Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.16  2009/02/18 04:46:55Z  garyp
    Corrected FfxDriverDeviceHandle() to pass the FFXDRIVERINFO pointer
    through the IOCTL interface.
    Revision 1.15  2009/02/09 06:58:38Z  garyp
    Merged from the v4.0 branch.  Major update to the external requestor API
    to disassociate the requestor handle from a specific Disk.  Most requests
    already include a handle inside the request packet itself.  Added the
    FlashFX_Open() and FlashFX_Close() functions.
    Revision 1.14  2008/04/28 13:58:23Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.13  2008/03/22 21:31:07Z  Garyp
    Updated to support more Driver Framework functions.
    Revision 1.12  2008/01/13 07:26:39Z  keithg
    Function header updates to support autodoc.
    Revision 1.11  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.10  2007/04/12 22:06:04Z  jimmb
    Added const to match the new prototype for FfxDriverDiskParameterSet
    Revision 1.9  2007/04/07 03:36:31Z  Garyp
    Added support for "ParameterSet".
    Revision 1.8  2007/01/10 01:45:00Z  Garyp
    Minor function renaming exercise.
    Revision 1.7  2006/11/10 20:18:52Z  Garyp
    Added "ParameterGet" and "Handle" support to the external API and
    removed "Stats" support.
    Revision 1.6  2006/05/08 00:31:00Z  Garyp
    Finalized the statistics interfaces.
    Revision 1.5  2006/02/20 22:22:15Z  Garyp
    Modified to use the newly updated external API mechanisms.
    Revision 1.4  2006/02/12 21:31:11Z  Garyp
    Temporarily disabled -- work-in-progress.
    Revision 1.3  2006/02/07 07:04:45Z  Garyp
    Renamed FFXDEVHANDLE to FFXREQHANDLE to avoid conflicts with the new
    meaning of FFXDEVHANDLE.
    Revision 1.2  2006/01/02 02:04:10Z  Garyp
    Cosmetic changes only.
    Revision 1.1  2005/11/06 09:02:06Z  Pauli
    Initial revision
    Revision 1.2  2005/11/06 09:02:06Z  Garyp
    Added the fVerbose and fReset parameters.
    Revision 1.1  2005/10/01 12:04:06Z  Garyp
    Initial revision
    Revision 1.2  2004/12/30 17:32:42Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2004/11/30 18:29:14Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxapi.h>
#include <fxdriver.h>
#include <fxapireq.h>
#include <drvfwreq.h>


/*-------------------------------------------------------------------
    Public: FfxConfigurationInfo()

    This function is used in an identical fashion to the real
    Driver Framework function of the same name.  See the official
    documentation for details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxConfigurationInfo(
    FFXCONFIGINFO          *pCI,
    D_BOOL                  fVerbose,
    D_BOOL                  fReset)
{
    FFXSTATUS               ffxStat;
    FFXREQHANDLE            hReq;
    REQ_DRIVERGETCONFIGINFO req;

    DclAssert(pCI);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_GETCONFIGINFO;
    req.req.ulReqLen    = sizeof(req);
    req.pCI             = pCI;
    req.fVerbose        = fVerbose;
    req.fReset          = fReset;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDeviceCreateParam()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXDEVHANDLE FfxDriverDeviceCreateParam(
    FFXDRIVERINFO              *pFDI,
    FFXDEVCONFIG               *pConf,
    FFXDEVHOOK                 *pHook)
{
    REQ_DRVRFW_DEVCREATEPARAM   req;
    FFXREQHANDLE                hReq;
    FFXSTATUS                   ffxStat;

    if(pConf->nDevNum >= FFX_MAX_DEVICES)
        return NULL;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return NULL;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_DEVCREATEPARAM;
    req.req.ulReqLen    = sizeof(req);
    req.pFDI            = pFDI;
    req.pConf           = pConf;
    req.pHook           = pHook;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.hDev;
    else
        return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDeviceDestroy()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDeviceDestroy(
    FFXDEVHANDLE            hDev)
{
    REQ_DRVRFW_DEVDESTROY   req;
    FFXREQHANDLE            hReq;
    FFXSTATUS               ffxStat;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_DEVDESTROY;
    req.req.ulReqLen    = sizeof(req);
    req.hDev            = hDev;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDeviceGeometry()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDeviceGeometry(
    FFXDEVHANDLE            hDev,
    FFXDEVGEOMETRY         *pGeometry)
{
    REQ_DRVRFW_DEVGEOMETRY  req;
    FFXREQHANDLE            hReq;
    FFXSTATUS               ffxStat;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_DEVGEOMETRY;
    req.req.ulReqLen    = sizeof(req);
    req.hDev            = hDev;
    req.pGeometry       = pGeometry;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDeviceHandle()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXDEVHANDLE FfxDriverDeviceHandle(
    FFXDRIVERINFO          *pFDI,
    unsigned                nDevNum)
{
    REQ_DRVRFW_DEVHANDLE    req;
    FFXREQHANDLE            hReq;
    FFXSTATUS               ffxStat;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return NULL;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_DEVHANDLE;
    req.req.ulReqLen    = sizeof(req);
    req.pFDI            = pFDI;
    req.nDevNum         = nDevNum;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.hDev;
    else
        return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDeviceEnumerate()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXDEVHANDLE FfxDriverDeviceEnumerate(
    FFXDRIVERINFO          *pFDI,
    FFXDEVHANDLE            hDev)
{
    REQ_DRVRFW_DEVENUMERATE req;
    FFXREQHANDLE            hReq;
    FFXSTATUS               ffxStat;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return NULL;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_DEVENUMERATE;
    req.req.ulReqLen    = sizeof(req);
    req.pFDI            = pFDI;
    req.hDev            = hDev;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.hDev;
    else
        return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDeviceSettings()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDeviceSettings(
    FFXDEVCONFIG           *pConf)
{
    REQ_DRVRFW_DEVSETTINGS  req;
    FFXREQHANDLE            hReq;
    FFXSTATUS               ffxStat;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_DEVSETTINGS;
    req.req.ulReqLen    = sizeof(req);
    req.pConf           = pConf;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.ffxStat;
    else
        return ffxStat;
}


#if FFXCONF_BBMSUPPORT

/*-------------------------------------------------------------------
    Public: FfxDriverDeviceBbmSettings()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDeviceBbmSettings(
    FFXDEVCONFIG               *pConf)
{
    REQ_DRVRFW_DEVBBMSETTINGS   req;
    FFXREQHANDLE                hReq;
    FFXSTATUS                   ffxStat;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_DEVBBMSETTINGS;
    req.req.ulReqLen    = sizeof(req);
    req.pConf           = pConf;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.ffxStat;
    else
        return ffxStat;
}

#endif


/*-------------------------------------------------------------------
    Public: FfxDriverDiskCreateParam()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXDISKHANDLE FfxDriverDiskCreateParam(
    FFXDRIVERINFO              *pFDI,
    FFXDISKCONFIG              *pConf,
    FFXDISKHOOK                *pHook)
{
    REQ_DRVRFW_DISKCREATEPARAM  req;
    FFXREQHANDLE                hReq;
    FFXSTATUS                   ffxStat;

    if(pConf->nDiskNum >= FFX_MAX_DISKS)
        return NULL;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return NULL;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_DISKCREATEPARAM;
    req.req.ulReqLen    = sizeof(req);
    req.pFDI            = pFDI;
    req.pConf           = pConf;
    req.pHook           = pHook;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.hDisk;
    else
        return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskDestroy()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskDestroy(
    FFXDISKHANDLE           hDisk)
{
    REQ_DRVRFW_DISKDESTROY  req;
    FFXREQHANDLE            hReq;
    FFXSTATUS               ffxStat;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_DISKDESTROY;
    req.req.ulReqLen    = sizeof(req);
    req.hDisk           = hDisk;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskGeometry()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskGeometry(
    FFXDISKHANDLE           hDisk,
    FFXDISKGEOMETRY        *pGeo)
{
    REQ_DRVRFW_DISKGEOMETRY req;
    FFXREQHANDLE            hReq;
    FFXSTATUS               ffxStat;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_DISKGEOMETRY;
    req.req.ulReqLen    = sizeof(req);
    req.hDisk           = hDisk;
    req.pGeometry       = pGeo;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskHandle()

    This function is used in an identical fashion to the real
    Driver Framework function of the same name.  See the official
    documentation for details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXDISKHANDLE FfxDriverDiskHandle(
    FFXDRIVERINFO          *pFDI,
    unsigned                nDiskNum)
{
    FFXSTATUS               ffxStat;
    FFXREQHANDLE            hReq;
    REQ_DRVRFW_DISKHANDLE   req;

    DclAssert(pFDI == NULL);

    if(nDiskNum >= FFX_MAX_DISKS)
        return NULL;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return NULL;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc   = FXIOFUNC_EXTDRVRFW_DISKHANDLE;
    req.req.ulReqLen = sizeof(req);
    req.pFDI         = pFDI;
    req.nDiskNum     = nDiskNum;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat != FFXSTAT_SUCCESS)
        return NULL;

    return req.hDisk;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskEnumerate()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXDISKHANDLE FfxDriverDiskEnumerate(
    FFXDRIVERINFO              *pFDI,
    FFXDISKHANDLE               hDisk)
{
    REQ_DRVRFW_DISKENUMERATE    req;
    FFXREQHANDLE                hReq;
    FFXSTATUS                   ffxStat;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return NULL;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_DISKENUMERATE;
    req.req.ulReqLen    = sizeof(req);
    req.pFDI            = pFDI;
    req.hDisk           = hDisk;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.hDisk;
    else
        return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskParameterGet()

    This function is used in an identical fashion to the real
    Driver Framework function of the same name.  See the official
    documentation for details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskParameterGet(
    FFXDISKHANDLE           hDisk,
    FFXPARAM                id,
    void                   *pBuffer,
    D_UINT32                ulBuffLen)
{
    FFXSTATUS               ffxStat;
    FFXREQHANDLE            hReq;
    REQ_DRVRFW_DISKPARAMGET req;

    DclAssert(id > FFXPARAM_STARTOFLIST);
    DclAssert(id < FFXPARAM_ENDOFLIST);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc     = FXIOFUNC_EXTDRVRFW_DISKGETPARAM;
    req.req.ulReqLen   = sizeof(req);
    req.hDisk          = hDisk;
    req.id             = id;
    req.pBuffer        = pBuffer;
    req.ulBuffLen      = ulBuffLen;

    ffxStat  = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskParameterSet()

    This function is used in an identical fashion to the real
    Driver Framework function of the same name.  See the official
    documentation for details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskParameterSet(
    FFXDISKHANDLE           hDisk,
    FFXPARAM                id,
    const void             *pBuffer,
    D_UINT32                ulBuffLen)
{
    FFXSTATUS               ffxStat;
    FFXREQHANDLE            hReq;
    REQ_DRVRFW_DISKPARAMSET req = {{0}};

    DclAssert(id);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    /*  Special case in the event we want to set a parameter for
        both the code calling the external API, as well as the
        actual FlashFX device driver.
    */
    if(!hDisk)
    {
        /*  This switch statement handles only those requests which
            can be satisfied without requiring a Disk handle.
        */
        switch(id)
        {
          #if D_DEBUG && DCLCONF_OUTPUT_ENABLED
            /*  When setting the trace mask, we want to do so for
                both the external API client, and the FlashFX device
                driver -- however the latter is considered to be
                more important, so return its trace mask, rather
                than the one used by the external API program.
            */
            case FFXPARAM_DEBUG_TRACEMASKSWAP:
                if(pBuffer && (ulBuffLen == sizeof(D_UINT32)))
                    FfxTraceMaskSwap(*(D_UINT32*)pBuffer);

                break;
          #endif

            default:
                break;
        }
    }

    req.req.ioFunc     = FXIOFUNC_EXTDRVRFW_DISKSETPARAM;
    req.req.ulReqLen   = sizeof(req);
    req.hDisk          = hDisk;
    req.id             = id;
    req.pBuffer        = pBuffer;
    req.ulBuffLen      = ulBuffLen;

    ffxStat  = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverAllocatorCreate()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverAllocatorCreate(
    FFXDISKHANDLE               hDisk)
{
    REQ_DRVRFW_ALLOCATORCREATE  req;
    FFXREQHANDLE                hReq;
    FFXSTATUS                   ffxStat;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_ALLOCATORCREATE;
    req.req.ulReqLen    = sizeof(req);
    req.hDisk           = hDisk;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverAllocatorDestroy()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverAllocatorDestroy(
    FFXDISKHANDLE               hDisk,
    D_UINT32                    ulFlags)
{
    REQ_DRVRFW_ALLOCATORDESTROY req;
    FFXREQHANDLE                hReq;
    FFXSTATUS                   ffxStat;

    DclAssert(!(ulFlags & FFX_SHUTDOWNFLAGS_RESERVEDMASK));

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_ALLOCATORDESTROY;
    req.req.ulReqLen    = sizeof(req);
    req.hDisk           = hDisk;
    req.ulFlags         = ulFlags;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverOptionGet()

    This function is used in an identical fashion to the real
    Driver Framework function of the same name.  See the official
    documentation for details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_BOOL FfxDriverOptionGet(
    FFXOPTION                   opt,
    void                       *handle,
    void                       *pBuffer,
    D_UINT32                    ulBuffLen)
{
    REQ_DRVRFW_OPTIONGET        req;
    FFXREQHANDLE                hReq;
    FFXSTATUS                   ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "FfxDriverOptionGet() handle=%P Type=%x pBuff=%P Len=%lU\n",
        handle, opt, pBuffer, ulBuffLen));

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&req, 0, sizeof(req));

    req.req.ioFunc      = FXIOFUNC_EXTDRVRFW_OPTIONGET;
    req.req.ulReqLen    = sizeof(req);
    req.option          = opt;
    req.handle          = handle;
    req.pBuffer         = pBuffer;
    req.ulBuffLen       = ulBuffLen;

    ffxStat = FlashFX_DevRequest(hReq, &req.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return req.fResult;
    else
        return FALSE;
}


