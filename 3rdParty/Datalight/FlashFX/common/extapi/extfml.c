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

    This module contains the FML external API functions.  These functions
    are used in a fashion identical to that of the standard FML interface.
    See the standard FML documentation for details on using these functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: extfml.c $
    Revision 1.18  2010/07/08 03:53:53Z  garyp
    Removed some unnecessary error handling code.
    Revision 1.17  2009/07/24 01:53:40Z  garyp
    Merged from the v4.0 branch.  Updated the FfxFmlCreate() function to take
    a flags parameter.  Adjusted for a number of fields which are now unsigned
    rather than D_UINT16.  Added FfxFmlDeviceRangeEnumerate().
    Revision 1.16  2009/04/09 02:58:28Z  garyp
    Renamed a structure to avoid AutoDoc naming collisions.
    Revision 1.15  2009/03/31 19:29:22Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.14  2009/02/18 18:13:17Z  billr
    Resolve bug 2426: FMSL test using external API fails debug assertion.
    Revision 1.13  2009/02/18 04:46:54Z  garyp
    Corrected FfxFmlCreate() to pass the hDisk parameter through the IOCTL
    interface.
    Revision 1.12  2009/02/09 06:58:56Z  garyp
    Merged from the v4.0 branch.  Major update to the external requestor API
    to disassociate the requestor handle from a specific Disk.  Most requests
    already include a handle inside the request packet itself.  Added the
    FlashFX_Open() and FlashFX_Close() functions.
    Revision 1.11  2009/01/16 23:48:33Z  glenns
    Fixed up literal FFXIOSTATUS initialization in one place.
    Revision 1.10  2008/01/13 07:26:40Z  keithg
    Function header updates to support autodoc.
    Revision 1.9  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.8  2006/11/10 20:43:48Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.7  2006/03/19 05:32:44Z  Garyp
    Corrected some logic to use FFX_MAX_DISKS rather than DEVICES.
    Revision 1.6  2006/03/07 00:47:04Z  Garyp
    Added FfxFmlDeviceInfo() support.
    Revision 1.5  2006/02/20 23:03:48Z  Garyp
    Modified to use the newly updated external API mechanisms.
    Revision 1.4  2006/02/13 02:58:08Z  Garyp
    Updated to new external API interface.
    Revision 1.3  2006/02/07 19:10:04Z  Garyp
    Temporarily disable the FML external interface while it is being revamped.
    Revision 1.2  2005/12/31 19:49:51Z  Garyp
    Modified various functions to not ask for a DriveNum if it is not going to
    be used.
    Revision 1.1  2005/05/02 17:38:08Z  Pauli
    Initial revision
    Revision 1.2  2005/05/02 18:38:08Z  Garyp
    Added external API support for FfxFmlWritePages().
    Revision 1.3  2004/12/30 23:03:11Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2004/03/15 22:43:29Z  garys
    Eliminated oemchanged() support.
    Updated to use renamed headers.
    Revision 1.1  2004/02/06 21:22:18Z  garyp
    Initial revision
    Revision 1.1.1.4  2004/02/06 21:22:18Z  garyp
    Improved parameter validation in oemmount().
    Revision 1.1.1.3  2003/12/05 22:35:58Z  garyp
    Modified to use pDevExtra.
    Revision 1.1.1.2  2003/11/26 20:49:22Z  billr
    Use MEMSET() (FlashFX macro) instead of memset() (Standard C library
    function).
    Revision 1.1  2003/11/22 02:58:14Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxapi.h>
#include <fxfmlapi.h>
#include <fxapireq.h>
#include <fmlreq.h>


/*-------------------------------------------------------------------
    Public: FfxFmlCreate()

    This function is used in an identical fashion to the real FML
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXFMLHANDLE FfxFmlCreate(
    FFXDISKHANDLE   hDisk,
    FFXFIMDEVHANDLE hFimDev,
    unsigned        nDiskNum,
    D_UINT32        ulStartBlock,
    D_UINT32        ulBlockCount,
    unsigned        nFlags)
{
    FFXREQHANDLE    hReq;
    REQ_FMLCREATE   create;
    FFXSTATUS       ffxStat;

    if(nDiskNum >= FFX_MAX_DISKS)
        return NULL;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return NULL;

    DclMemSet(&create, 0, sizeof(create));

    create.req.ioFunc   = FXIOFUNC_EXTFML_CREATE;
    create.req.ulReqLen = sizeof(create);
    create.hDisk        = hDisk;
    create.hFimDev      = hFimDev;
    create.nDiskNum     = nDiskNum;
    create.ulStartBlock = ulStartBlock;
    create.ulBlockCount = ulBlockCount;
    create.nFlags       = nFlags;

    ffxStat = FlashFX_DevRequest(hReq, &create.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return create.hFML;
    else
        return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxFmlDestroy()

    This function is used in an identical fashion to the real FML
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlDestroy(
    FFXFMLHANDLE    hFML)
{
    FFXREQHANDLE    hReq;
    REQ_FMLDESTROY  destroy;
    FFXSTATUS       ffxStat;

    DclAssert(hFML);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&destroy, 0, sizeof(destroy));

    destroy.req.ioFunc = FXIOFUNC_EXTFML_DESTROY;
    destroy.req.ulReqLen = sizeof(destroy);
    destroy.hFML = hFML;

    ffxStat = FlashFX_DevRequest(hReq, &destroy.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return destroy.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxFmlHandle()

    This function is used in an identical fashion to the real FML
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXFMLHANDLE FfxFmlHandle(
    unsigned        nDiskNum)
{
    FFXREQHANDLE    hReq;
    REQ_FMLHANDLE   handle;
    FFXSTATUS       ffxStat;

    if(nDiskNum >= FFX_MAX_DISKS)
        return NULL;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return NULL;

    DclMemSet(&handle, 0, sizeof(handle));

    handle.req.ioFunc   = FXIOFUNC_EXTFML_HANDLE;
    handle.req.ulReqLen = sizeof(handle);
    handle.nDiskNum     = nDiskNum;

    ffxStat = FlashFX_DevRequest(hReq, &handle.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return handle.hFML;
    else
        return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxFmlDeviceRangeEnumerate()

    This function is used in an identical fashion to the real FML
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlDeviceRangeEnumerate(
    unsigned            nDevNum,
    D_UINT32            ulBlockNum,
    FFXFMLHANDLE       *phFML,
    D_UINT32           *pulBlockCount)
{
    REQ_FMLDEVICERANGE  info;
    FFXREQHANDLE        hReq;
    FFXSTATUS           ffxStat;

    DclAssert(phFML);
    DclAssert(pulBlockCount);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&info, 0, sizeof(info));

    info.req.ioFunc     = FXIOFUNC_EXTFML_DEVICERANGE;
    info.req.ulReqLen   = sizeof(info);
    info.nDeviceNum     = nDevNum;
    info.ulBlockNum     = ulBlockNum;
    info.phFML          = phFML;
    info.pulBlockCount  = pulBlockCount;

    ffxStat = FlashFX_DevRequest(hReq, &info.req);

    FlashFX_Close(hReq);

    if(ffxStat != FFXSTAT_SUCCESS)
         return ffxStat;

    return info.ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxFmlDeviceInfo()

    This function is used in an identical fashion to the real FML
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlDeviceInfo(
    unsigned            nDeviceNum,
    FFXFMLDEVINFO      *pFDI)
{
    REQ_FMLDEVICEINFO   info;
    FFXREQHANDLE        hReq;
    FFXSTATUS           ffxStat;

    DclAssert(pFDI);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&info, 0, sizeof(info));

    info.req.ioFunc     = FXIOFUNC_EXTFML_DEVICEINFO;
    info.req.ulReqLen   = sizeof(info);
    info.nDeviceNum     = nDeviceNum;
    info.pFmlDeviceInfo = pFDI;

    ffxStat = FlashFX_DevRequest(hReq, &info.req);

    FlashFX_Close(hReq);

    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    return info.ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxFmlDiskInfo()

    This function is used in an identical fashion to the real FML
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlDiskInfo(
    FFXFMLHANDLE    hFML,
    FFXFMLINFO     *pFI)
{
    REQ_FMLINFO     info;
    FFXREQHANDLE    hReq;
    FFXSTATUS       ffxStat;

    DclAssert(hFML);
    DclAssert(pFI);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&info, 0, sizeof(info));

    info.req.ioFunc     = FXIOFUNC_EXTFML_DISKINFO;
    info.req.ulReqLen   = sizeof(info);
    info.hFML           = hFML;
    info.pFmlInfo       = pFI;

    ffxStat = FlashFX_DevRequest(hReq, &info.req);

    FlashFX_Close(hReq);

    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    return info.ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxFmlParameterGet()

    This function is used in an identical fashion to the real FML
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlParameterGet(
    FFXFMLHANDLE        hFML,
    FFXPARAM            id,
    void               *pBuffer,
    D_UINT32            ulBuffLen)
{
    REQ_FMLPARAMETERGET param;
    FFXREQHANDLE        hReq;
    FFXSTATUS           ffxStat;

    DclAssert(hFML);
    DclAssert(id);
    DclAssert(pBuffer);
    DclAssert(ulBuffLen);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&param, 0, sizeof(param));

    param.req.ioFunc    = FXIOFUNC_EXTFML_PARAMETERGET;
    param.req.ulReqLen  = sizeof(param);
    param.hFML          = hFML;
    param.id            = id;
    param.pBuffer       = pBuffer;
    param.ulBuffLen     = ulBuffLen;

    ffxStat = FlashFX_DevRequest(hReq, &param.req);

    FlashFX_Close(hReq);

    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    return param.ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxFmlParameterSet()

    This function is used in an identical fashion to the real FML
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlParameterSet(
    FFXFMLHANDLE        hFML,
    FFXPARAM            id,
    const void         *pBuffer,
    D_UINT32            ulBuffLen)
{
    REQ_FMLPARAMETERSET param;
    FFXREQHANDLE        hReq;
    FFXSTATUS           ffxStat;

    DclAssert(hFML);
    DclAssert(id);
    /*  Don't assert these -- some param code don't require any data
    DclAssert(pBuffer);
    DclAssert(ulBuffLen);
    */
    
    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&param, 0, sizeof(param));

    param.req.ioFunc    = FXIOFUNC_EXTFML_PARAMETERSET;
    param.req.ulReqLen  = sizeof(param);
    param.hFML          = hFML;
    param.id            = id;
    param.pBuffer       = pBuffer;
    param.ulBuffLen     = ulBuffLen;

    ffxStat = FlashFX_DevRequest(hReq, &param.req);

    FlashFX_Close(hReq);

    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    return param.ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxFmlIORequest()

    This function is used in an identical fashion to the real FML
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlIORequest(
    FFXFMLHANDLE        hFML,
    FFXIOREQUEST       *pIOR)
{
    REQ_FMLIOREQUEST    ioreq;
    FFXREQHANDLE        hReq;
    FFXIOSTATUS         ioStat = NULL_IO_STATUS;

    ioStat.ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        return ioStat;

    DclMemSet(&ioreq, 0, sizeof(ioreq));

    ioreq.req.ioFunc    = FXIOFUNC_EXTFML_IOREQUEST;
    ioreq.req.ulReqLen  = sizeof(ioreq);
    ioreq.hFML          = hFML;
    ioreq.pIOReq        = pIOR;

    ioStat.ffxStat = FlashFX_DevRequest(hReq, &ioreq.req);

    FlashFX_Close(hReq);

    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        return ioStat;

    return ioreq.ioStat;
}


