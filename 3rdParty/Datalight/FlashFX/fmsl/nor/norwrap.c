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

    This module implements wrapper code so that old style NOR FIMs, which do
    not use the new ioctl interface, can function transparently in the new
    model, without upper layers knowing any different.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: norwrap.c $
    Revision 1.25  2009/04/06 14:26:36Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.24  2009/04/03 16:07:57Z  glenns
    - Fix Bugzilla #1253: Assign the ffxStat element of the FFXIOSTATUS
      return value to FFXSTAT_FIM_UNSUPPORTEDIOREQUEST
      in the default cases of the IORequest switch statement.
    Revision 1.23  2008/03/26 00:05:03Z  Garyp
    Minor datatype changes.
    Revision 1.22  2008/02/03 01:48:22Z  keithg
    comment updates to support autodoc.
    Revision 1.21  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.20  2007/10/23 17:43:36Z  billr
    Remove bogus assertions: comparison is always true due to limited range of
    data type.
    Revision 1.19  2007/10/03 21:34:19Z  Garyp
    Implement the default alignment based on DCL_CPU_ALIGNSIZE rather than
    DCL_ALIGNSIZE.
    Revision 1.18  2007/10/01 21:39:29Z  pauli
    Updated comments.
    Revision 1.17  2007/07/19 21:23:53Z  timothyj
    Added minor assertions that flash block size is evenly divisible by
    the page size.
    Revision 1.16  2007/04/30 14:07:12Z  garyp
    Fixed so that if the real FIM initializes the page size value (Sibley), the
    wrapper will initialize the proper pages per block value.
    Revision 1.15  2007/03/02 21:22:06Z  timothyj
    Added computation of simulated uPagesPerBlock to FFXPARAM_PAGE_SIZE handler.
    Revision 1.14  2007/01/31 20:24:46Z  Garyp
    Updated to allow erase-suspend support to be compiled out of the product.
    Revision 1.13  2006/12/31 21:29:56Z  Garyp
    Added the DEV_OLDSTYLE_NORFIM so we can clearly distinguish old and new
    style NOR FIMs at the DevMgr level.
    Revision 1.12  2006/11/08 18:01:28Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.11  2006/08/28 20:37:12Z  Garyp
    Removed a spurious but benign preprocessor check.
    Revision 1.10  2006/05/23 18:39:59Z  Garyp
    Updated to build properly if NOR/Sibley is disabled.
    Revision 1.9  2006/05/20 18:53:05Z  Garyp
    Added support for boot blocks.
    Revision 1.8  2006/03/16 22:51:19Z  Garyp
    Corrected to return the page size in the FimInfo structure (if any).
    Revision 1.7  2006/03/06 17:33:11Z  Garyp
    Fixed some bogus asserts.
    Revision 1.6  2006/02/15 02:00:54Z  Garyp
    Updated to build cleanly.
    Revision 1.5  2006/02/14 23:27:35Z  Garyp
    Updated to build properly if NOR and Sibley are disabled.
    Revision 1.4  2006/02/12 18:48:21Z  Garyp
    Updated to the new tools/tests model where device/disk numbers are
    used, rather than handles.
    Revision 1.3  2006/02/11 23:49:11Z  Garyp
    Tweaked to build cleanly.
    Revision 1.2  2006/02/11 00:12:59Z  Garyp
    Renamed the FIMDEVICE structure instantiation for clarity.
    Revision 1.1  2006/02/08 19:35:54Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT   /* File-wide */

#include <fxdevapi.h>
#include <fimdev.h>
#include "nor.h"


/*-------------------------------------------------------------------
    The FIM instance data structure.
-------------------------------------------------------------------*/
struct tagFIMDATA
{
    FIMINFO         FimInfo;
    EXTMEDIAINFO    EMI;
    FIMDEVICE      *pRealDev;
    unsigned        fInBootBlocks : 1;
};


static FIMHANDLE    Create(FFXDEVHANDLE hDev, const FIMINFO **ppFimInfo);
static void         Destroy(     FIMHANDLE hFim);
static FFXSTATUS    ParameterGet(FIMHANDLE hFim, FFXPARAM id, void *pBuffer, D_UINT32 ulBuffLen);
static FFXSTATUS    ParameterSet(FIMHANDLE hFim, FFXPARAM id, const void *pBuffer, D_UINT32 ulBuffLen);
static FFXIOSTATUS  IORequest(   FIMHANDLE hFim, FFXIOREQUEST *pIOR);

/*-------------------------------------------------------------------
    FIMDEVICE Declaration

    This structure declaration is used to define the entry points
    into the FIM.  This is declared at the end of the module to
    eliminate the need for what would be duplicated function
    prototypes in all the FIMs.
-------------------------------------------------------------------*/
FIMDEVICE FFXFIM_NOR_Wrapper =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    Create,
    Destroy,
    IORequest,
    ParameterGet,
    ParameterSet
};



/*-------------------------------------------------------------------
    Public: Create()

    The function creates a FIM instance.

    Parameters:
        hDev - The device handle

    Return Value:
        Returns TRUE if successful else FALSE.
-------------------------------------------------------------------*/
static FIMHANDLE Create(
    FFXDEVHANDLE        hDev,
    const FIMINFO     **ppFimInfo)
{
    FIMDATA            *pFim = NULL;
    unsigned            nDevNum;

    DclAssert(hDev);
    DclAssert(ppFimInfo);

    /*  The DevMgr stores the FIMDEVICE structure for the real
        device, which we are wrapping in our FIMDEVICE structure.
    */
    if(!FFXFIM_NOR_Wrapper.pOldFimDevice)
    {
        DclError();
        return NULL;
    }

    if(!FfxHookOptionGet(FFXOPT_DEVICE_NUMBER, hDev, &nDevNum, sizeof nDevNum))
        return NULL;

    pFim = DclMemAllocZero(sizeof *pFim);
    if(!pFim)
        return NULL;

    pFim->pRealDev = FFXFIM_NOR_Wrapper.pOldFimDevice;
    pFim->EMI.hDev = hDev;
    pFim->EMI.uDriveNum =(D_UINT16)nDevNum;

    /*  See if we can successfully mount the media
    */
    DclAssert(pFim->pRealDev->Mount);
    if(!pFim->pRealDev->Mount(&pFim->EMI))
        goto MountError;

    DclAssert(pFim->EMI.ulTotalSize);
    DclAssert(pFim->EMI.ulEraseZoneSize);

    pFim->FimInfo.uDeviceType         = pFim->EMI.uDeviceType & DEVTYPE_MASK;
    pFim->FimInfo.uDeviceFlags        = (pFim->EMI.uDeviceType & DEVFLAGS_MASK) | (D_UINT16)DEV_OLDSTYLE_NORFIM;
    pFim->FimInfo.ulPhysicalBlocks    = pFim->EMI.ulTotalSize / pFim->EMI.ulEraseZoneSize;
    pFim->FimInfo.ulChipBlocks        = pFim->EMI.ulDeviceSize / pFim->EMI.ulEraseZoneSize;
    pFim->FimInfo.ulBlockSize         = pFim->EMI.ulEraseZoneSize;
    pFim->FimInfo.ulBootBlockSize     = pFim->EMI.ulBootBlockSize;
    pFim->FimInfo.uBootBlockCountLow  = pFim->EMI.uBootBlockCountLow;
    pFim->FimInfo.uBootBlockCountHigh = pFim->EMI.uBootBlockCountHigh;
    pFim->FimInfo.uPageSize           = pFim->EMI.uPageSize;

    /*  None of the Datalight supplied old-style NOR FIMs handle alignment.
        If that changes, or to accommodate customers old-style FIMS, we will
        need a way for this new style wrapper FIM to get that information and
        pass it on appropriately.  A better approach would be to update any
        old-style FIMs to the new interface.
    */
    pFim->FimInfo.uAlignSize          = DCL_CPU_ALIGNSIZE;

    pFim->FimInfo.ulTotalBlocks       = pFim->FimInfo.ulPhysicalBlocks; /* Same for NOR */

    /*  If this device does have a page size, then we also know the number of
        pages per block.  If it does not have a fixed page size, then leave
        both values set to zero and let the DevMgr figure out what to do.
    */
    if(pFim->FimInfo.uPageSize)
    {
        DclAssert((pFim->FimInfo.ulBlockSize % pFim->FimInfo.uPageSize) == 0);
        pFim->FimInfo.uPagesPerBlock = (D_UINT16)(pFim->FimInfo.ulBlockSize / pFim->FimInfo.uPageSize);
    }

    if(pFim->FimInfo.uBootBlockCountLow)
    {
        DclAssert(pFim->FimInfo.ulBootBlockSize);
        DclAssert(((pFim->FimInfo.ulBootBlockSize * pFim->FimInfo.uBootBlockCountLow) %
            pFim->FimInfo.ulBlockSize) == 0);
    }

    if(pFim->FimInfo.uBootBlockCountHigh)
    {
        DclAssert(pFim->FimInfo.ulBootBlockSize);
        DclAssert(((pFim->FimInfo.ulBootBlockSize * pFim->FimInfo.uBootBlockCountHigh) %
            pFim->FimInfo.ulBlockSize) == 0);
    }

    if(!pFim->FimInfo.ulBootBlockSize)
    {
        DclAssert(!pFim->FimInfo.uBootBlockCountLow);
        DclAssert(!pFim->FimInfo.uBootBlockCountHigh);
    }

    DclPrintf("FFX: NORWRAP: DevSize=%lUKB ChipSize=%lUKB BlockSize=%lUKB BBSize=%lUKB BBLow=%U BBHigh=%U\n",
        pFim->EMI.ulTotalSize / 1024UL,
        pFim->FimInfo.ulChipBlocks * pFim->FimInfo.ulBlockSize / 1024UL,
        pFim->FimInfo.ulBlockSize / 1024UL,
        pFim->FimInfo.ulBootBlockSize / 1024UL,
        pFim->FimInfo.uBootBlockCountLow,
        pFim->FimInfo.uBootBlockCountHigh);

    *ppFimInfo = &pFim->FimInfo;
    return pFim;

  MountError:

    if(pFim)
        DclMemFree(pFim);

    return NULL;
}


/*-------------------------------------------------------------------
    Public: Destroy()

    Destroy a FIM instance and release any allocated resources.

    Parameters:
        hFim - The FIM instance handle

    Return Value:
        None
-------------------------------------------------------------------*/
static void Destroy(
    FIMHANDLE  hFim)
{
    DclAssert(hFim);

    DclAssert(hFim->pRealDev->Unmount);
    hFim->pRealDev->Unmount(&hFim->EMI);

    DclMemFree(hFim);

    return;
}


/*-------------------------------------------------------------------
    Public: ParameterGet()

    Get a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hFim      - The FIM instance handle
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the buffer in which to store the
                    value.  May be NULL.
        ulBuffLen - The size of buffer.  May be zero if pBuffer is
                    NULL.

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS ParameterGet(
    FIMHANDLE       hFim,
    FFXPARAM        id,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_BADPARAMETER;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "NORWRAP:ParameterGet() hFim=%P ID=%x pBuff=%P Len=%lU\n",
        hFim, id, pBuffer, ulBuffLen));

    FFXPRINTF(1, ("NORWRAP:ParameterGet() Unsupported FFXPARAM ID %u\n", id));

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "NORWRAP:ParameterGet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: Parameterset()

    Set a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hFim      - The FIM instance handle
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the parameter data.
        ulBuffLen - The parameter data length.

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS ParameterSet(
    FIMHANDLE       hFim,
    FFXPARAM        id,
    const void     *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_BADPARAMETER;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "NORWRAP:ParameterSet() hFim=%P ID=%x pBuff=%P Len=%lU\n",
        hFim, id, pBuffer, ulBuffLen));

    switch(id)
    {
        case FFXPARAM_PAGE_SIZE:
            if(ulBuffLen == sizeof hFim->FimInfo.uPageSize)
            {
                hFim->FimInfo.uPageSize = *(D_UINT16*)pBuffer;
                DclAssert((hFim->FimInfo.ulBlockSize % hFim->FimInfo.uPageSize) == 0);
                hFim->FimInfo.uPagesPerBlock = (D_UINT16)(hFim->FimInfo.ulBlockSize / hFim->FimInfo.uPageSize);
                ffxStat = FFXSTAT_SUCCESS;
            }
            break;

        default:
            FFXPRINTF(1, ("NORWRAP:ParameterSet() Unsupported FFXPARAM ID %u\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "NORWRAP:ParameterSet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: IORequest()

    Parameters:
        hFim - The FIM instance handle
        pIOR - A pointer to the FFXIOREQUEST structure

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS IORequest(
    FIMHANDLE           hFim,
    FFXIOREQUEST       *pIOR)
{
    FFXIOSTATUS         ioStat;

    DclAssert(pIOR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 2, TRACEINDENT),
        "NORWRAP:IORequest() hFim=%P Func=%x\n", hFim, pIOR->ioFunc));

    DclProfilerEnter("NORWRAP:IORequest", 0, 0);

    DclAssert(hFim);

    DclMemSet(&ioStat, 0, sizeof ioStat);

    switch(pIOR->ioFunc)
    {
        case FXIOFUNC_FIM_ERASE_BOOT_BLOCK_START:
        {
            FFXIOR_FIM_ERASE_BOOT_BLOCK_START *pReq = (FFXIOR_FIM_ERASE_BOOT_BLOCK_START*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);
            DclAssert(hFim->FimInfo.ulBootBlockSize);

            ioStat.ulFlags = IOFLAGS_BLOCK;

            hFim->fInBootBlocks = TRUE;

            if(!hFim->pRealDev->EraseStart(&hFim->EMI,
                pReq->ulStartBB * hFim->FimInfo.ulBootBlockSize,
                pReq->ulBBCount * hFim->FimInfo.ulBootBlockSize))
            {
                ioStat.ffxStat = FFXSTAT_FIM_ERASEFAILED;
            }
            else
            {
                ioStat.ffxStat = FFXSTAT_SUCCESS;
            }

            break;
        }

        case FXIOFUNC_FIM_ERASE_START:
        {
            FFXIOR_FIM_ERASE_START *pReq = (FFXIOR_FIM_ERASE_START*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat.ulFlags = IOFLAGS_BLOCK;

            hFim->fInBootBlocks = FALSE;

            if(!hFim->pRealDev->EraseStart(&hFim->EMI,
                pReq->ulStartBlock * hFim->FimInfo.ulBlockSize,
                pReq->ulCount * hFim->FimInfo.ulBlockSize))
            {
                ioStat.ffxStat = FFXSTAT_FIM_ERASEFAILED;
            }
            else
            {
                ioStat.ffxStat = FFXSTAT_SUCCESS;
            }

            break;
        }

        case FXIOFUNC_FIM_ERASE_POLL:
        {
            FFXIOR_FIM_ERASE_POLL  *pReq = (FFXIOR_FIM_ERASE_POLL*)pIOR;
            D_UINT32                ulLength;

            (void)pReq;
            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat.ulFlags = IOFLAGS_BLOCK;

            ulLength = hFim->pRealDev->ErasePoll(&hFim->EMI);
            if(ulLength == FIMMT_ERASE_FAILED)
            {
                ioStat.ffxStat = FFXSTAT_FIM_ERASEFAILED;
            }
            else
            {
                if(ulLength)
                {
                    if(hFim->fInBootBlocks)
                    {
                        DclAssert(ulLength % hFim->FimInfo.ulBootBlockSize == 0);
                        ioStat.ulCount = ulLength / hFim->FimInfo.ulBootBlockSize;
                    }
                    else
                    {
                        DclAssert(ulLength % hFim->FimInfo.ulBlockSize == 0);
                        ioStat.ulCount = ulLength / hFim->FimInfo.ulBlockSize;
                    }

                    ioStat.ffxStat = FFXSTAT_SUCCESS;
                }
                else
                {
                    ioStat.ffxStat = FFXSTAT_FIM_ERASEINPROGRESS;
                }
            }

            break;
        }

      #if FFXCONF_ERASESUSPENDSUPPORT
        case FXIOFUNC_FIM_ERASE_SUSPEND:
        {
            FFXIOR_FIM_ERASE_SUSPEND *pReq = (FFXIOR_FIM_ERASE_SUSPEND*)pIOR;

            (void)pReq;
            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat.ulFlags = IOFLAGS_BLOCK;

            if(hFim->pRealDev->EraseSuspend)
            {
                if(hFim->pRealDev->EraseSuspend(&hFim->EMI))
                    ioStat.ffxStat = FFXSTAT_SUCCESS;
                else
                    ioStat.ffxStat = FFXSTAT_FIM_ERASESUSPENDFAILED;
            }
            else
            {
                ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            }

            break;
        }

        case FXIOFUNC_FIM_ERASE_RESUME:
        {
            FFXIOR_FIM_ERASE_RESUME *pReq = (FFXIOR_FIM_ERASE_RESUME*)pIOR;

            (void)pReq;
            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat.ulFlags = IOFLAGS_BLOCK;

            if(hFim->pRealDev->EraseResume)
            {
                hFim->pRealDev->EraseResume(&hFim->EMI);
                ioStat.ffxStat = FFXSTAT_FIM_ERASESUSPENDFAILED;
            }
            else
            {
                ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            }

            break;
        }
      #endif

        case FXIOFUNC_FIM_READ_GENERIC:
        {
            FFXIOR_FIM_READ_GENERIC *pReq = (FFXIOR_FIM_READ_GENERIC*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);
            DclAssert(pReq->ulCount);
            DclAssert(hFim->FimInfo.uPageSize);

            /*  All these operations are page ops.
            */
            ioStat.ulFlags = IOFLAGS_PAGE;

            switch(pReq->ioSubFunc)
            {
                case FXIOSUBFUNC_FIM_READ_PAGES:
                {
                    DclAssert(pReq->pPageData);
                    DclAssert(pReq->ulCount * hFim->FimInfo.uPageSize < D_UINT16_MAX);

                    if(!hFim->pRealDev->Read(&hFim->EMI,
                        pReq->ulStartPage * hFim->FimInfo.uPageSize,
                        (D_UINT16)(pReq->ulCount * hFim->FimInfo.uPageSize),
                        pReq->pPageData))
                    {
                        ioStat.ffxStat = FFXSTAT_FIMIOERROR;
                    }
                    else
                    {
                        ioStat.ffxStat = FFXSTAT_SUCCESS;
                        ioStat.ulCount = pReq->ulCount;
                    }

                    break;
                }

                default:
                {
                    FFXPRINTF(1, ("NORWRAP:IORequest() Bad Read Generic SubFunction %x\n", pReq->ioSubFunc));
                    ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
                    DclError();
                    break;
                }
            }

            break;
        }

        case FXIOFUNC_FIM_WRITE_GENERIC:
        {
            FFXIOR_FIM_WRITE_GENERIC *pReq = (FFXIOR_FIM_WRITE_GENERIC*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);
            DclAssert(pReq->ulCount);
            DclAssert(hFim->FimInfo.uPageSize);

            switch(pReq->ioSubFunc)
            {
                case FXIOSUBFUNC_FIM_WRITE_PAGES:
                {
                    DclAssert(pReq->pPageData);
                    DclAssert(pReq->ulCount * hFim->FimInfo.uPageSize < D_UINT16_MAX);

                    if(!hFim->pRealDev->Write(&hFim->EMI,
                        pReq->ulStartPage * hFim->FimInfo.uPageSize,
                        (D_UINT16)(pReq->ulCount * hFim->FimInfo.uPageSize),
                        (void*)pReq->pPageData))
                    {
                        ioStat.ffxStat = FFXSTAT_FIMIOERROR;
                    }
                    else
                    {
                        ioStat.ffxStat = FFXSTAT_SUCCESS;
                        ioStat.ulCount = pReq->ulCount;
                    }

                    break;
                }

                default:
                {
                    FFXPRINTF(1, ("NORWRAP:IORequest() Bad Write Generic SubFunction %x\n", pReq->ioSubFunc));
                    ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
                    DclError();
                    break;
                }
            }

            break;
        }

        case FXIOFUNC_FIM_READ_CONTROLDATA:
        {
            FFXIOR_FIM_READ_CONTROLDATA *pReq = (FFXIOR_FIM_READ_CONTROLDATA*)pIOR;
            D_BOOL  fSuccess;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);
            DclAssert(pReq->pData);
            DclAssert(pReq->ulLength);
            DclAssert(pReq->ulLength < D_UINT16_MAX);

            ioStat.ulFlags = IOFLAGS_BYTE;

            if(hFim->pRealDev->ReadControl)
            {
                fSuccess = hFim->pRealDev->ReadControl(&hFim->EMI, pReq->ulOffset,
                    (D_UINT16)pReq->ulLength, pReq->pData);
            }
            else
            {
                fSuccess = hFim->pRealDev->Read(&hFim->EMI, pReq->ulOffset,
                    (D_UINT16)pReq->ulLength, pReq->pData);
            }

            if(!fSuccess)
            {
                ioStat.ffxStat = FFXSTAT_FIMIOERROR;
            }
            else
            {
                ioStat.ffxStat = FFXSTAT_SUCCESS;
                ioStat.ulCount = pReq->ulLength;
            }

            break;
        }

        case FXIOFUNC_FIM_WRITE_CONTROLDATA:
        {
            FFXIOR_FIM_WRITE_CONTROLDATA *pReq = (FFXIOR_FIM_WRITE_CONTROLDATA*)pIOR;
            D_BOOL  fSuccess;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);
            DclAssert(pReq->pData);
            DclAssert(pReq->ulLength);
            DclAssert(pReq->ulLength < D_UINT16_MAX);

            ioStat.ulFlags = IOFLAGS_BYTE;

            if(hFim->pRealDev->WriteControl)
            {
                fSuccess = hFim->pRealDev->WriteControl(&hFim->EMI, pReq->ulOffset,
                    (D_UINT16)pReq->ulLength, (void*)pReq->pData);
            }
            else
            {
                fSuccess = hFim->pRealDev->Write(&hFim->EMI, pReq->ulOffset,
                    (D_UINT16)pReq->ulLength, (void*)pReq->pData);
            }

            if(!fSuccess)
            {
                ioStat.ffxStat = FFXSTAT_FIMIOERROR;
            }
            else
            {
                ioStat.ffxStat = FFXSTAT_SUCCESS;
                ioStat.ulCount = pReq->ulLength;
            }

            break;
        }

        default:
        {
            FFXPRINTF(1, ("NORWRAP:IORequest() Bad Function %x\n", pIOR->ioFunc));
            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            DclError();
            break;
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 2, TRACEUNDENT),
        "NORWRAP:IORequest() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}



#endif  /* FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT */




