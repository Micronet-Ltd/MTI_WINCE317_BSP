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

    This module contains the VBF external API functions.  These functions
    are used in a fashion identical to that of the standard VBF interface.
    See the standard VBF documentation for details on using these functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: extvbf.c $
    Revision 1.25  2010/11/30 17:37:58Z  glenns
    Add external API for disabling the QuickMount state save upon
    the next disk dismount.
    Revision 1.24  2009/12/18 17:17:46Z  garyp
    Resolved Bug 2955: Write interruption test failure.
    Revision 1.23  2009/08/04 01:23:23Z  garyp
    Merged from the v4.0 branch.  Added support for FfxVbfRegionMetrics() to 
    the external API.  Updated the compaction functions to take a compaction 
    level.  Modified the shutdown processes to take a mode parameter.  Added 
    support for compaction suspend/resume.  Minor datatype changes from 
    D_UINT16 to unsigned.  Updated for new compaction functions which now
    return an FFXIOSTATUS value rather than a D_BOOL.
    Revision 1.22  2009/03/31 19:32:28Z  davidh
    Function Hearders updated for AutoDoc.
    Revision 1.21  2009/03/28 19:55:49Z  glenns
    Fix parameter qualified mismatch causing compiler warning.
    Revision 1.20  2009/02/09 06:59:28Z  garyp
    Merged from the v4.0 branch.  Major update to the external requestor API
    to disassociate the requestor handle from a specific Disk.  Most requests
    already include a handle inside the request packet itself.  Added the
    FlashFX_Open() and FlashFX_Close() functions.
    Revision 1.19  2009/01/16 23:48:49Z  glenns
    Fixed up literal FFXIOSTATUS initialization in three places.
    Revision 1.18  2008/12/09 20:51:31Z  keithg
    No longer pass a serial number to VbfFormat() - it is handled internally.
    Revision 1.17  2008/06/03 20:49:02Z  thomd
    Added fFormatNoErase.
    Revision 1.16  2008/05/27 19:29:48Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.15  2008/05/06 16:12:53Z  keithg
    Updated to use new FXSTAT_VBF_xxx codes.
    Revision 1.14  2008/04/14 18:38:05Z  garyp
    Removed an unnecessary preprocessor symbol check.
    Revision 1.13  2008/02/06 20:07:55Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.12  2008/01/17 03:19:54Z  Garyp
    Corrected some broken return values in FfxVbfFormat().
    Revision 1.11  2008/01/13 07:26:42Z  keithg
    Function header updates to support autodoc.
    Revision 1.10  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.9  2007/06/07 22:41:20Z  rickc
    Removed deprecated functions vbfread(), vbfwrite(), vbfdiscard(),
    and vbfclientsize()
    Revision 1.8  2006/12/16 02:02:29Z  Garyp
    Modified FfxVbfTestRegionMountPerf() to return the performance result
    rather than displaying it, which is not very useful in non-monolithic
    environments.
    Revision 1.7  2006/05/08 17:18:02Z  Garyp
    Updated to use the new write interruption test interfaces.  Removed dead
    code.
    Revision 1.6  2006/03/19 05:32:44Z  Garyp
    Corrected some logic to use FFX_MAX_DISKS rather than DEVICES.
    Revision 1.5  2006/02/21 02:23:33Z  Garyp
    Modified to use the newly updated external API mechanisms.
    Revision 1.4  2006/02/12 21:33:47Z  Garyp
    Temporarily disabled -- work-in-progress.
    Revision 1.3  2006/02/08 00:22:31Z  Garyp
    Renamed FFXDEVHANDLE to FFXREQHANDLE to avoid conflicts with the
    new meaning of FFXDEVHANDLE.
    Revision 1.2  2006/01/05 03:36:33Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.1  2005/11/06 09:02:08Z  Pauli
    Initial revision
    Revision 1.2  2005/11/06 09:02:07Z  Garyp
    Added FfxVbfStatistics().. Updated so that the uDriveNum parameter is not
    returned if it is not needed.
    Revision 1.1  2005/10/02 03:03:22Z  Garyp
    Initial revision
    Revision 1.2  2005/09/18 05:55:21Z  garyp
    Renamed vbfcompact() to FfxVbfCompact() and added FfxVbfCompactIfIdle().
    Revision 1.1  2005/08/03 19:31:00Z  pauli
    Initial revision
    Revision 1.9  2005/05/05 23:50:00Z  garyp
    Added some prototypes to mollify what appears to be a compiler
    issue with the DIAB SH compiler in VxWorks 6.x.  (See compiler.txt.)
    Revision 1.8  2004/12/30 17:32:42Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.7  2004/11/30 19:10:24Z  GaryP
    Modified the previous rev to return failure codes that indicate the external
    API is not functioning, as necessary.
    Revision 1.6  2004/11/29 23:43:26Z  garys
    Changed return values for FfxVbfCompactionSuspend/Restore
    Revision 1.5  2004/11/20 00:40:32Z  GaryP
    Added external API support for compaction suspend/restore.
    Revision 1.4  2004/09/23 08:34:28Z  GaryP
    Semantic change from "garbage collection" to "compaction".
    Revision 1.3  2004/09/15 23:28:58Z  garys
    fAggressive parameter to FfxVbfCompact() to support background G.C.
    Revision 1.2  2004/03/15 22:49:08  garys
    Header updates.
    Revision 1.1  2003/12/29 23:01:20Z  garyp
    Initial revision
    Revision 1.1.1.4  2003/12/29 23:01:19Z  billr
    Fix interface to vbfgetpartitioninfo(), it now returns a status.
    Revision 1.1.1.3  2003/12/25 00:04:22Z  garyp
    Changed all uses of VBF_API_BLOCK_SIZE to VBF_BLOCK_SIZE and eliminated
    any conditional code based on different values for those settings.
    Revision 1.1.1.2  2003/11/26 20:50:18Z  billr
    Use MEMSET() (FlashFX macro) instead of memset() (Standard C library
    function).
    Revision 1.1  2003/11/22 02:58:14Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_VBFSUPPORT

#include <vbf.h>
#include <fxapi.h>
#include <fxapireq.h>

#include <vbfreq.h>


/*-------------------------------------------------------------------
    Public: FfxVbfCreate()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
VBFHANDLE FfxVbfCreate(
    FFXFMLHANDLE    hFML)
{
    FFXSTATUS       ffxStat;
    FFXREQHANDLE    hReq;
    REQ_VBFCREATE   create;

    DclAssert(hFML);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
         return NULL;

    DclMemSet(&create, 0, sizeof(create));

    create.req.ioFunc   = FXIOFUNC_EXTVBF_CREATE;
    create.req.ulReqLen = sizeof(create);
    create.hFML         = hFML;

    ffxStat = FlashFX_DevRequest(hReq, &create.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return create.hVBF;
    else
        return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxVbfDestroy()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxVbfDestroy(
    VBFHANDLE       hVBF,
    D_UINT32        ulFlags)
{
    FFXREQHANDLE    hReq;
    FFXSTATUS       ffxStat;
    REQ_VBFDESTROY  destroy;

    DclAssert(hVBF);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&destroy, 0, sizeof(destroy));

    destroy.req.ioFunc = FXIOFUNC_EXTVBF_DESTROY;
    destroy.req.ulReqLen = sizeof(destroy);
    destroy.hVBF = hVBF;
    destroy.ulFlags           = ulFlags;

    ffxStat = FlashFX_DevRequest(hReq, &destroy.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return destroy.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxVbfHandle()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
VBFHANDLE FfxVbfHandle(
    unsigned        nDiskNum)
{
    FFXREQHANDLE    hReq;
    REQ_VBFHANDLE   handle;
    FFXSTATUS       ffxStat;

    if(nDiskNum >= FFX_MAX_DISKS)
        return NULL;

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return NULL;

    DclMemSet(&handle, 0, sizeof(handle));

    handle.req.ioFunc   = FXIOFUNC_EXTVBF_HANDLE;
    handle.req.ulReqLen = sizeof(handle);
    handle.nDiskNum     = nDiskNum;

    ffxStat = FlashFX_DevRequest(hReq, &handle.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return handle.hVBF;
    else
        return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxVbfDiskInfo()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxVbfDiskInfo(
    VBFHANDLE           hVBF,
    VBFDISKINFO        *pInfo)
{
    FFXREQHANDLE        hReq;
    REQ_VBFDISKINFO     DiskInfo;
    FFXSTATUS           ffxStat;

    DclAssert(hVBF);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&DiskInfo, 0, sizeof(DiskInfo));

    DiskInfo.req.ioFunc     = FXIOFUNC_EXTVBF_DISKINFO;
    DiskInfo.req.ulReqLen   = sizeof(DiskInfo);
    DiskInfo.hVBF           = hVBF;
    DiskInfo.pDiskInfo      = pInfo;

    ffxStat = FlashFX_DevRequest(hReq, &DiskInfo.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return DiskInfo.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxVbfDiskMetrics()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxVbfDiskMetrics(
    VBFHANDLE           hVBF,
    VBFDISKMETRICS     *pDiskMets)
{
    FFXREQHANDLE        hReq;
    REQ_VBFDISKMETRICS  DiskMets;
    FFXSTATUS           ffxStat;

    DclAssert(hVBF);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&DiskMets, 0, sizeof(DiskMets));

    DiskMets.req.ioFunc     = FXIOFUNC_EXTVBF_DISKMETRICS;
    DiskMets.req.ulReqLen   = sizeof(DiskMets);
    DiskMets.hVBF           = hVBF;
    DiskMets.pDiskMets      = pDiskMets;

    ffxStat = FlashFX_DevRequest(hReq, &DiskMets.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return DiskMets.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxVbfRegionMetrics()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxVbfRegionMetrics(
    VBFHANDLE               hVBF,
    D_UINT32                ulRegionNum,
    VBFREGIONMETRICS       *pRegionMets)
{
    FFXREQHANDLE            hReq;
    REQ_VBFREGIONMETRICS    RgnMets;
    FFXSTATUS               ffxStat;

    DclAssert(hVBF);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&RgnMets, 0, sizeof(RgnMets));

    RgnMets.req.ioFunc     = FXIOFUNC_EXTVBF_REGIONMETRICS;
    RgnMets.req.ulReqLen   = sizeof(RgnMets);
    RgnMets.hVBF           = hVBF;
    RgnMets.ulRegionNum    = ulRegionNum;
    RgnMets.pRegionMets    = pRegionMets;

    ffxStat = FlashFX_DevRequest(hReq, &RgnMets.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return RgnMets.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxVbfUnitMetrics()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxVbfUnitMetrics(
    VBFHANDLE           hVBF,
    VBFUNITMETRICS     *pUnitMets)
{
    FFXREQHANDLE        hReq;
    REQ_VBFUNITMETRICS  UnitMets;
    FFXSTATUS           ffxStat;

    DclAssert(hVBF);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&UnitMets, 0, sizeof(UnitMets));

    UnitMets.req.ioFunc     = FXIOFUNC_EXTVBF_UNITMETRICS;
    UnitMets.req.ulReqLen   = sizeof(UnitMets);
    UnitMets.hVBF           = hVBF;
    UnitMets.pUnitMets      = pUnitMets;

    ffxStat = FlashFX_DevRequest(hReq, &UnitMets.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return UnitMets.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxVbfReadPages()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxVbfReadPages(
    VBFHANDLE       hVBF,
    D_UINT32        ulStartPage,
    D_UINT32        ulPageCount,
    D_BUFFER       *pBuffer)
{
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_BADHANDLE);
    FFXREQHANDLE    hReq;
    REQ_VBFREAD     Read;

    DclAssert(hVBF);
    DclAssert(pBuffer);
    DclAssert(ulPageCount);

    ioStat.ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        return ioStat;

    DclMemSet(&Read, 0, sizeof(Read));

    Read.req.ioFunc     = FXIOFUNC_EXTVBF_READPAGES;
    Read.req.ulReqLen   = sizeof(Read);
    Read.hVBF           = hVBF;
    Read.ulStartPage    = ulStartPage;
    Read.ulPageCount    = ulPageCount;
    Read.pBuffer        = pBuffer;

    ioStat.ffxStat = FlashFX_DevRequest(hReq, &Read.req);

    FlashFX_Close(hReq);

    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        return Read.ioStat;
    else
        return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxVbfWritePages()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxVbfWritePages(
    VBFHANDLE       hVBF,
    D_UINT32        ulStartPage,
    D_UINT32        ulPageCount,
    const D_BUFFER *pBuffer)
{
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_BADHANDLE);
    FFXREQHANDLE    hReq;
    REQ_VBFWRITE    Write;

    DclAssert(hVBF);
    DclAssert(pBuffer);
    DclAssert(ulPageCount);

    ioStat.ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        return ioStat;

    DclMemSet(&Write, 0, sizeof(Write));

    Write.req.ioFunc    = FXIOFUNC_EXTVBF_WRITEPAGES;
    Write.req.ulReqLen  = sizeof(Write);
    Write.hVBF          = hVBF;
    Write.ulStartPage   = ulStartPage;
    Write.ulPageCount   = ulPageCount;
    Write.pBuffer       = pBuffer;

    ioStat.ffxStat = FlashFX_DevRequest(hReq, &Write.req);

    FlashFX_Close(hReq);

    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        return Write.ioStat;
    else
        return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxVbfDiscardPages()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxVbfDiscardPages(
    VBFHANDLE       hVBF,
    D_UINT32        ulStartPage,
    D_UINT32        ulPageCount)
{
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_BADHANDLE);
    FFXREQHANDLE    hReq;
    REQ_VBFDISCARD  Discard;

    DclAssert(hVBF);
    DclAssert(ulPageCount);

    ioStat.ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        return ioStat;

    DclMemSet(&Discard, 0, sizeof(Discard));

    Discard.req.ioFunc    = FXIOFUNC_EXTVBF_DISCARDPAGES;
    Discard.req.ulReqLen  = sizeof(Discard);
    Discard.hVBF          = hVBF;
    Discard.ulStartPage   = ulStartPage;
    Discard.ulPageCount   = ulPageCount;

    ioStat.ffxStat = FlashFX_DevRequest(hReq, &Discard.req);

    FlashFX_Close(hReq);

    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        return Discard.ioStat;
    else
        return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxVbfCompact()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxVbfCompact(
    VBFHANDLE       hVBF,
    D_UINT32        ulFlags)
{
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_BADHANDLE);
    FFXREQHANDLE    hReq;
    REQ_VBFCOMPACT  Compact;

    DclAssert(hVBF);

    ioStat.ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        return ioStat;

    DclMemSet(&Compact, 0, sizeof(Compact));

    Compact.req.ioFunc      = FXIOFUNC_EXTVBF_COMPACT;
    Compact.req.ulReqLen    = sizeof(Compact);
    Compact.hVBF            = hVBF;
    Compact.ulFlags         = ulFlags;

    ioStat.ffxStat = FlashFX_DevRequest(hReq, &Compact.req);

    FlashFX_Close(hReq);

    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        return Compact.ioStat;
    else
        return ioStat;
}


#if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE

/*-------------------------------------------------------------------
    Public: FfxVbfCompactIdleSuspend()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_UINT32 FfxVbfCompactIdleSuspend(
    VBFHANDLE           hVBF)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_BADHANDLE);
    FFXREQHANDLE        hReq;
    REQ_VBFCOMPSUSPEND  Suspend;

    DclAssert(hVBF);

    ioStat.ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        return D_UINT32_MAX;

    DclMemSet(&Suspend, 0, sizeof(Suspend));

    Suspend.req.ioFunc      = FXIOFUNC_EXTVBF_COMPACTSUSPEND;
    Suspend.req.ulReqLen    = sizeof(Suspend);
    Suspend.hVBF            = hVBF;

    ioStat.ffxStat = FlashFX_DevRequest(hReq, &Suspend.req);

    FlashFX_Close(hReq);

    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        return Suspend.ulCount;
    else
        return D_UINT32_MAX;
}


/*-------------------------------------------------------------------
    Public: FfxVbfCompactIdleResume()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_UINT32 FfxVbfCompactIdleResume(
    VBFHANDLE           hVBF)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_BADHANDLE);
    FFXREQHANDLE        hReq;
    REQ_VBFCOMPRESUME   Resume;

    DclAssert(hVBF);

    ioStat.ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        return D_UINT32_MAX;

    DclMemSet(&Resume, 0, sizeof(Resume));

    Resume.req.ioFunc      = FXIOFUNC_EXTVBF_COMPACTRESUME;
    Resume.req.ulReqLen    = sizeof(Resume);
    Resume.hVBF            = hVBF;

    ioStat.ffxStat = FlashFX_DevRequest(hReq, &Resume.req);

    FlashFX_Close(hReq);

    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        return Resume.ulCount;
    else
        return D_UINT32_MAX;
}

#endif


/*-------------------------------------------------------------------
    Public: FfxVbfFormat()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxVbfFormat(
    FFXFMLHANDLE    hFML,
    D_UINT32        ulCushion,
    D_UINT32        ulSpareUnits,
    D_BOOL          fFormatNoErase)
{
    FFXSTATUS       ffxStat;
    FFXREQHANDLE    hReq;
    REQ_VBFFORMAT   Format;

    DclAssert(hFML);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclMemSet(&Format, 0, sizeof(Format));

    Format.req.ioFunc       = FXIOFUNC_EXTVBF_FORMAT;
    Format.req.ulReqLen     = sizeof(Format);
    Format.hFML             = hFML;
    Format.ulCushion        = ulCushion;
    Format.ulSpareUnits     = ulSpareUnits;
    Format.fFormatNoErase   = fFormatNoErase;

    ffxStat = FlashFX_DevRequest(hReq, &Format.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return Format.ffxStat;
    else
        return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxVbfTestRegionMountPerf()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_UINT32 FfxVbfTestRegionMountPerf(
    VBFHANDLE               hVBF)
{
    FFXREQHANDLE            hReq;
    REQ_VBFTESTRGNMOUNTPERF RegionMountPerf;
    FFXSTATUS               ffxStat;

    DclAssert(hVBF);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return 0;

    DclMemSet(&RegionMountPerf, 0, sizeof(RegionMountPerf));

    RegionMountPerf.req.ioFunc   = FXIOFUNC_EXTVBF_TESTRGNMOUNTPERF;
    RegionMountPerf.req.ulReqLen = sizeof(RegionMountPerf);
    RegionMountPerf.hVBF         = hVBF;

    ffxStat = FlashFX_DevRequest(hReq, &RegionMountPerf.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return RegionMountPerf.ulResult;
    else
        return 0;
}


/*-------------------------------------------------------------------
    Public: FfxVbfTestWriteInterruptions()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_INT16 FfxVbfTestWriteInterruptions(
    volatile VBFHANDLE      hVBF,
    D_UINT32                ulSeed,
    unsigned                nVerbosity)
{
    FFXREQHANDLE            hReq;
    REQ_VBFTESTWRITEINTS    WriteInts;
    FFXSTATUS               ffxStat;

    DclAssert(hVBF);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return -1;

    DclMemSet(&WriteInts, 0, sizeof(WriteInts));

    WriteInts.req.ioFunc    = FXIOFUNC_EXTVBF_TESTWRITEINTS;
    WriteInts.req.ulReqLen  = sizeof(WriteInts);
    WriteInts.hVBF          = hVBF;
    WriteInts.ulSeed        = ulSeed;
    WriteInts.nVerbosity    = nVerbosity;

    ffxStat = FlashFX_DevRequest(hReq, &WriteInts.req);

    FlashFX_Close(hReq);

    if(ffxStat == FFXSTAT_SUCCESS)
        return WriteInts.nReturn;
    else
        return -1;
}

#if FFXCONF_QUICKMOUNTSUPPORT

/*-------------------------------------------------------------------
    Public: FfxVbfDisableStateSave()

    This function is used in an identical fashion to the real VBF
    function of the same name.  See the official documentation for
    details on using this function.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
void FfxVbfDisableStateSave(VBFHANDLE hVBF)
{
    FFXREQHANDLE            hReq;
    REQ_VBFDISABLESAVESTATE DisableStateSave;
    FFXSTATUS               ffxStat;

    DclAssert(hVBF);

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
        return;

    DclMemSet(&DisableStateSave, 0, sizeof(DisableStateSave));

    DisableStateSave.req.ioFunc    = FXIOFUNC_EXTVBF_DISABLESTATSAVE;
    DisableStateSave.req.ulReqLen  = sizeof(DisableStateSave);
    DisableStateSave.hVBF          = hVBF;

    ffxStat = FlashFX_DevRequest(hReq, &DisableStateSave.req);

    FlashFX_Close(hReq);

}

#endif  /* FFXCONF_QUICKMOUNTSUPPORT */

#endif  /* FFXCONF_VBFSUPPORT */



