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
  jurisdictions.  Patents may be pending.

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

    The memory validation service is used to track detailed memory usage.
    Memory is tracked based on a number of different factors which are:

    1) The memory buffer type -- OSMemPool, DLMemMgr, Explicit, and Automatic.
    2) The memory buffer address and length
    3) The memory access type -- ReadOnly or ReadWrite.

    The memory validation service is tied directly into the specialized assert
    macros DclAssertReadPtr() and DclAssertWritePtr().  These macros take a
    pointer and a length, and call the memory validation service (if enabled)
    to validate and record information about the requested access.

    In addition to validation, the service may be queried to dump out summary
    statistical information.

    OSMemPool Buffers

    These buffers are those which are allocated from the operating system using
    the OS Services.

    DLMemMgr Buffers

    These buffers are those which are allocated and freed using DclMemAlloc()
    and DclMemFree().  Buffers of this style are automatically added and
    removed by the memory management functions.  Add and remove functions for
    these buffers must be equally matched.

    Explicit Buffers (AKA Client Buffers)

    These buffers are typically client memory buffers which must be explicitly
    added and removed, typically in the high-level driver code.  For example,
    high level block device driver code which takes a client buffer pointer
    containing data for a ReadSectors call, would explicitly add the buffer
    at the start of the call, and remove the buffer at the end of the call.
    Add and remove functions for these buffers must be equally matched.

    Automatic Buffers

    Automatic buffers are dynamically added as they are encountered during
    execution.  Anytime the "Validate" serice request is performed for a
    buffer which is not already being tracked, it will be added as an
    Automatic buffer.  Therefore, buffers on the stack, or constant string
    pointers are typical examples of automatic buffers.  Automatic buffers
    are never removed from the tracking list -- since they are not explicitly
    added, they cannot be explicitly removed.  Also, unlike DLMemMgr and
    Explicit buffers, Automatic buffers will be automatically coalesced as
    they are added, if they overlap or are adjacent.

    The Buffers are stored in two linked lists -- the Active list and the
    Inactive List.  When a DLMemMgr or Exlicit buffer is removed from being
    actively tracked, it will be added to an "Inactive" list, which will be
    part of the statistical summary information.  The usage count field in
    the Inactive list will show the number of times that buffer has been
    cycled.  Automatic buffers never end up on the Inactive list, because
    they are never explicitly removed from the Active list.

    Validation Algorithm

    When the validate service is invoked, the specified buffer is examined to
    see if it is currently being tracked in the Active buffer list.  If so,
    then the type of access is validated, based on the type of buffer --
    DLMemMgr, Explicit, or Automatic, and the type of request.  An attempt
    to write to a Read-Only Explicit buffer, for example, will yield an error.
    Likewise, and attempt to read past the end of a DLMemMgr or Explicit
    buffer will fail.

    For buffers of the type DLMemMgr, if the Memory Tracking service is
    enabled, it will be called to verify that the sentinels for the buffer
    in question are intact.

    Access to Automatic buffers are treated differently -- since they are
    automatically added, it may be completely valid that a given Automatic
    buffer is initially added in Read-Only mode, and then written to later.
    Similarly the length of these buffers may dynamically change as different
    operations are performed.  Therefore, Automatic buffers are tracked and
    adjusted on the fly to reflect the actual usage of the memory.  Summary
    information can be statistically analyzed to find anomalies, for example,
    a random access out in the apparent middle of nowhere, or a write access
    in the middle of an area which migh appear to be constant string data.

    Summary Information

    The summary information shows the current contents of the Active and
    Inactive lists, in detail, which includes (among other things), the
    number of times each buffer is read from or written to.

    Final Notes

    While this service functions most effectively when used in combination
    with the Memory Tracking service, they are independent of each other.
    The Memory Tracking serivice only deals with memory allocated through
    the DCL memory management functions.

    This service is designed solely for testing and debugging purposes -- it
    will dramatically slow things down!

    This service is only as good as the instrumentation which makes it work.
    Ensuring that client buffers are properly added and removed as Explicit
    buffers is important, as is ensuring that the DclAssertReadPtr() and
    DclAssertWritePtr() macros are properly used in every location possible.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlmemvalservice.c $
    Revision 1.2  2010/01/04 22:08:14Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.1  2009/06/27 02:40:00Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlmemval.h>
#include <dlservice.h>
#include <dlinstance.h>


#if DCLCONF_MEMORYVALIDATION

/*  This defines the number of free nodes to add each time the
    free list runs out of entries.
*/
#define NODE_EXPAND_COUNT       (256)

#define DCL_MVFLAG_INACTIVE     (0x80000000U)


typedef struct sMEMVALPOOLHDR
{
    struct sMEMVALPOOLHDR  *pNextPool;
    unsigned                nEntries;
} MEMVALPOOLHDR;

typedef struct sMEMVALINFO
{
    struct sMEMVALINFO *pNext;
    const D_BUFFER     *pBuffer;
    D_UINT32            ulBuffLen;
    D_UINT32            ulReadCount;
    D_UINT32            ulWriteCount;
    unsigned            nUsageCount;
    unsigned            nFlags;
} MEMVALINFO;

typedef struct
{
    DCLINSTANCEHANDLE   hDclInst;
    MEMVALPOOLHDR      *pPoolList;      /* Linked list of each memory allocation for internal use */
    MEMVALINFO         *pActiveList;
    MEMVALINFO         *pInactiveList;
    MEMVALINFO         *pFreeList;
    unsigned            nTotalNodes;
    unsigned            nErrorMode;
} MEMVALSERVICEDATA;

/*  Handy-dandy macros for examining the node flags.
*/
#define ISAUTOMATIC(flg) (((flg) & DCL_MVFLAG_TYPEMASK) == DCL_MVFLAG_TYPEAUTOMATIC)
#define ISREADONLY(flg)   ((flg) & DCL_MVFLAG_READONLY)


static DCLSTATUS    ServiceIoctl(DCLSERVICEHANDLE hService, DCLREQ_SERVICE *pReq, void *pPriv);
static DCLSTATUS    Add(             MEMVALSERVICEDATA *pMVD, const void *pBuffer, D_UINT32 ulBuffLen, unsigned nFlags);
static DCLSTATUS    Remove(          MEMVALSERVICEDATA *pMVD, const void *pBuffer);
static DCLSTATUS    Validate(        MEMVALSERVICEDATA *pMVD, const void *pBuffer, D_UINT32 ulBuffLen, unsigned nFlags);
static MEMVALINFO * Lookup(          MEMVALSERVICEDATA *pMVD, const void *pBuffer, MEMVALINFO **ppPrevMVD);
static DCLSTATUS    Summary(         MEMVALSERVICEDATA *pMVD, DCLMEMVALSUMMARY *pVal, D_BOOL fQuiet, D_BOOL fReset);
static DCLSTATUS    ExpandFreeList(  MEMVALSERVICEDATA *pMVD, unsigned nCount);
static DCLSTATUS    AddAutomaticNode(MEMVALSERVICEDATA *pMVD, MEMVALINFO *pMVI, MEMVALINFO *pPrevMVI, const void *pBuffer, D_UINT32 ulBuffLen, unsigned nFlags);
static D_BOOL       TryCoalesceNodes(MEMVALSERVICEDATA *pMVD, MEMVALINFO *pMVI);

#endif


/*-------------------------------------------------------------------
    Protected: DclMemValServiceInit()

    Parameters:
        hDclInst - The DCL instance handle.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclMemValServiceInit(
    DCLINSTANCEHANDLE           hDclInst)
{
  #if DCLCONF_MEMORYVALIDATION
    static DCLSERVICEHEADER     SrvMemVal[DCL_MAX_INSTANCES];
    static MEMVALSERVICEDATA    MVD[DCL_MAX_INSTANCES];
    DCLSERVICEHEADER           *pSrv;
    DCLSTATUS                   dclStat;
    unsigned                    nInstNum;

    /*  REMINDER! Because some environments are capable of restarting,
                  without reinitializing static data, this function must
                  explicitly ensure that any static data is manually
                  zero'd, rather than just expecting it to already be in
                  that state.
    */

    if(!hDclInst)
        return DCLSTAT_SERVICE_BADHANDLE;

    nInstNum = DclInstanceNumber(hDclInst);
    DclMemSet(&MVD[nInstNum], 0, sizeof(MVD[0]));

    MVD[nInstNum].hDclInst = hDclInst;

    pSrv = &SrvMemVal[nInstNum];
    dclStat = DclServiceHeaderInit(&pSrv, "DLMEMVAL", DCLSERVICE_MEMVALIDATION, ServiceIoctl, &MVD[nInstNum], 0);
    if(dclStat == DCLSTAT_SUCCESS)
        dclStat = DclServiceCreate(hDclInst, pSrv);

    return dclStat;

  #else

    /*  Even if a feature is disabled at compile-time, a given project may
        still be using a default set of services which tries to initialize
        it.  Therefore, to avoid link errors, this "Init" function must
        exist.  Simply politely decline to initialize if this is the case.
        The DCLSTAT_FEATUREDISABLED status code is explicitly accommodated
        in the project creation process, and will not abort the process.
    */
    (void)hDclInst;
    return DCLSTAT_FEATUREDISABLED;

  #endif
}



#if DCLCONF_MEMORYVALIDATION

                /*---------------------------------*\
                 *                                 *
                 *     Static Local Functions      *
                 *                                 *
                \*---------------------------------*/



/*-------------------------------------------------------------------
    Local: ServiceIoctl()

    Parameters:
        hService - The service handle
        pReq     - A pointer to the DCLREQ_SERVICE structure
        pPriv    - A pointer to any private data to pass

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS ServiceIoctl(
    DCLSERVICEHANDLE    hService,
    DCLREQ_SERVICE     *pReq,
    void               *pPriv)
{
    MEMVALSERVICEDATA  *pMVD = pPriv;
    DCLSTATUS           dclStat;

    DclAssert(hService);
    DclAssert(pReq);
    DclAssert(pMVD);

    if(pReq->ior.ulReqLen != sizeof(*pReq))
        return DCLSTAT_BADSTRUCLEN;

    switch(pReq->ior.ioFunc)
    {
        case DCLIOFUNC_SERVICE_CREATE:
        case DCLIOFUNC_SERVICE_REGISTER:
        case DCLIOFUNC_SERVICE_DEREGISTER:
            DclAssert(pReq->pSubRequest == NULL);

            /*  Nothing to do
            */
            return DCLSTAT_SUCCESS;

/*
        case DCLIOFUNC_SERVICE_CREATE:
        {
            DclAssert(pReq->pSubRequest == NULL);

            pMVD->pActiveList = NULL;

            return ExpandFreeList(pMVD, NODE_EXPAND_COUNT);
        }
*/
        case DCLIOFUNC_SERVICE_DESTROY:
            DclAssert(pReq->pSubRequest == NULL);
/*            DclAssert(pMVD->pPoolList);*/

            /*  Dump out final summary information
            */
            dclStat = Summary(pMVD, NULL, FALSE, FALSE);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;

            /*  Free up however many pools we've allocated
            */
            while(pMVD->pPoolList)
            {
                MEMVALPOOLHDR *pPrev = pMVD->pPoolList->pNextPool;

                pMVD->nTotalNodes -= pMVD->pPoolList->nEntries;

                dclStat = DclMemFree(pMVD->pPoolList);
                if(dclStat != DCLSTAT_SUCCESS)
                    return dclStat;

                pMVD->pPoolList = pPrev;
            }

            return DCLSTAT_SUCCESS;

        case DCLIOFUNC_SERVICE_DISPATCH:
        {
            if(pReq->pSubRequest == NULL)
                return DCLSTAT_SERVICE_BADSUBREQUEST;

            if(pReq->pSubRequest->ulReqLen < sizeof(pReq->pSubRequest))
                return DCLSTAT_BADSTRUCLEN;

            switch(pReq->pSubRequest->ioFunc)
            {
                case DCLIOFUNC_MEMVAL_ERRORMODE:
                {
                    DCLREQ_MEMVAL_ERRORMODE  *pSubReq = (DCLREQ_MEMVAL_ERRORMODE*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    DclAssert(pSubReq->nNewMode == DCLMEM_ERROR_ASSERT || pSubReq->nNewMode == DCLMEM_ERROR_FAIL);
                    DclAssert(pMVD->nErrorMode == DCLMEM_ERROR_ASSERT || pMVD->nErrorMode == DCLMEM_ERROR_FAIL);

                    pSubReq->nOldMode = pMVD->nErrorMode;
                    pMVD->nErrorMode = pSubReq->nNewMode;

                    return DCLSTAT_SUCCESS;
                }

                case DCLIOFUNC_MEMVAL_BUFFERADD:
                {
                    DCLREQ_MEMVAL_BUFFERADD  *pSubReq = (DCLREQ_MEMVAL_BUFFERADD*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    return Add(pMVD, pSubReq->pBuffer, pSubReq->ulBuffLen, pSubReq->nFlags);
                }

                case DCLIOFUNC_MEMVAL_BUFFERREMOVE:
                {
                    DCLREQ_MEMVAL_BUFFERREMOVE  *pSubReq = (DCLREQ_MEMVAL_BUFFERREMOVE*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    return Remove(pMVD, pSubReq->pBuffer);
                }

                case DCLIOFUNC_MEMVAL_BUFFERVALIDATE:
                {
                    DCLREQ_MEMVAL_BUFFERVALIDATE  *pSubReq = (DCLREQ_MEMVAL_BUFFERVALIDATE*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    return Validate(pMVD, pSubReq->pBuffer, pSubReq->ulBuffLen, pSubReq->nFlags);
                }

                case DCLIOFUNC_MEMVAL_SUMMARY:
                {
                    DCLREQ_MEMVAL_SUMMARY  *pSubReq = (DCLREQ_MEMVAL_SUMMARY*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    return Summary(pMVD, &pSubReq->MVS, pSubReq->fQuiet, pSubReq->fReset);
                }

                default:
                    DCLPRINTF(1, ("MemVal:ServiceIoctl() Unsupported subfunction %x\n", pReq->pSubRequest->ioFunc));
                    return DCLSTAT_SERVICE_UNSUPPORTEDREQUEST;
            }
        }

        default:
            DCLPRINTF(1, ("MemVal:ServiceIoctl() Unhandled request %x\n", pReq->ior.ioFunc));
            return DCLSTAT_SERVICE_UNHANDLEDREQUEST;
    }
}


/*-------------------------------------------------------------------
    Local: Add()

    Parameters:
        pMVD      - A pointer to the MEMVALSERVICEDATA structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Add(
    MEMVALSERVICEDATA  *pMVD,
    const void         *pBuffer,
    D_UINT32            ulBuffLen,
    unsigned            nFlags)
{
/*    const D_BUFFER     *pBuff = pBuffer; */
    MEMVALINFO         *pMVI;
    MEMVALINFO         *pPrevMVI;
    DCLSTATUS           dclStat = DCLSTAT_SUCCESS;

    DclAssert(pMVD);
    DclAssert(pBuffer);
    DclAssert(!ISAUTOMATIC(nFlags));

    /*  Reminder that "Add" is never used for Automatic type buffers.
    */
    pMVI = Lookup(pMVD, pBuffer, &pPrevMVI);
    if(pMVI && ((nFlags & DCL_MVFLAG_TYPEMASK) == (pMVI->nFlags & DCL_MVFLAG_TYPEMASK)))
    {

 /*  Seems like this should never happen...
 */
 DclProductionError();
        /*  If we found an entry, everything had better match up.
            Verify that, increment the usage count and we're done.
        */
        DclAssert(pMVI->nUsageCount);
        DclAssert(pBuffer == pMVI->pBuffer);
        DclAssert(ulBuffLen == pMVI->ulBuffLen);

        pMVI->nUsageCount++;
        DclAssert(pMVI->nUsageCount);
    }
    else
    {
        if(!pMVD->pFreeList)
        {
            dclStat = ExpandFreeList(pMVD, NODE_EXPAND_COUNT);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;

            DclAssert(pMVD->pFreeList);
        }

        /*  Remove the item from the head of the free list
        */
        pMVI = pMVD->pFreeList;
        pMVD->pFreeList = pMVI->pNext;

        pMVI->pBuffer = pBuffer;
        pMVI->ulBuffLen = ulBuffLen;
        pMVI->nFlags = nFlags;
        pMVI->nUsageCount = 1;

        if(pPrevMVI)
        {
            /*  Cannot have overlapping entries!
            */
/*            DclAssert(pPrevMVI->pBuffer + pPrevMVI->ulBuffLen <= pBuff);
            DclAssert(pPrevMVI->pNext == NULL || pBuff + ulBuffLen <= pPrevMVI->pNext->pBuffer);
*/
            /*  Hook the new entry into proper spot in the "used" list
            */
            pMVI->pNext = pPrevMVI->pNext;
            pPrevMVI->pNext = pMVI;
        }
        else
        {
            /*  Cannot have overlapping entries!
            */
/*            DclAssert(pMVD->pActiveList == NULL || pMVD->pActiveList->pNext == NULL ||
                      pBuff + ulBuffLen <= pMVD->pActiveList->pNext->pBuffer);
*/
            /*  Hook the new entry into the head of the "used" list
            */
            pMVI->pNext = pMVD->pActiveList;
            pMVD->pActiveList = pMVI;
        }
    }

    if(pMVI && dclStat == DCLSTAT_SUCCESS)
    {
        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACEBIT_MEM, 1, 0),
            "MEMVAL Add     : Buffer=%P Len=%lX Flags=%x Usage=%u\n",
            pMVI->pBuffer, pMVI->ulBuffLen, pMVI->nFlags, pMVI->nUsageCount));
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: Remove()

    Parameters:
        pMVD      - A pointer to the MEMVALSERVICEDATA structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Remove(
    MEMVALSERVICEDATA  *pMVD,
    const void         *pBuffer)
{
    MEMVALINFO         *pMVI;
    MEMVALINFO         *pPrevMVI;

    DclAssert(pBuffer);

    /*  Reminder that "Remove" is never used for Automatic type buffers.
    */

    pMVI = Lookup(pMVD, pBuffer, &pPrevMVI);
    if(!pMVI)
    {
        if(pMVD->nErrorMode == DCLMEM_ERROR_ASSERT)
        {
            DclProductionError();
        }

        return DCLSTAT_MEMVAL_INVALIDBUFFER;
    }

    if(pMVI->pBuffer != pBuffer)
    {
        DCLPRINTF(2, ("DclMemVal:Remove() buffer %P not found\n", pBuffer));
        return DCLSTAT_MEMVAL_INVALIDBUFFER;
    }

    DclAssert(pMVI->ulBuffLen);
    DclAssert(pMVI->nUsageCount);
    DclAssert(!ISAUTOMATIC(pMVI->nFlags));

    pMVI->nUsageCount--;

    if(!pMVI->nUsageCount)
    {
        MEMVALINFO         *pCurr = pMVD->pInactiveList;
        MEMVALINFO         *pPrev = NULL;

        /*  Remove the node from the "Active" list
        */
        if(pPrevMVI)
        {
            pPrevMVI->pNext = pMVI->pNext;
        }
        else
        {
            DclAssert(pMVD->pActiveList == pMVI);
            pMVD->pActiveList = pMVI->pNext;
        }

        /*  Find the right spot in the "Inactive" list
        */
        while(pCurr)
        {
            if(pMVI->pBuffer < pCurr->pBuffer)
                break;

            if(pMVI->pBuffer == pCurr->pBuffer && pMVI->ulBuffLen <= pCurr->ulBuffLen)
                break;

            pPrev = pCurr;
            pCurr = pCurr->pNext;
        }

        /*  If the characteristics of these buffers are <exactly> the same,
            increment the usage count and accumulate the statistics.
        */
        if(pCurr &&
            pCurr->pBuffer == pMVI->pBuffer &&
            pCurr->ulBuffLen == pMVI->ulBuffLen &&
            pCurr->nFlags == pMVI->nFlags)
        {
            pCurr->ulReadCount += pMVI->ulReadCount;
            pCurr->ulWriteCount += pMVI->ulWriteCount;
            pCurr->nUsageCount++;

            /*  Added the freed up node to the start of the "free" list
            */
            pMVI->pNext = pMVD->pFreeList;
            pMVD->pFreeList = pMVI;
        }
        else
        {
            /*  Hook the node into the "Inactive" list
            */
            if(pPrev)
            {
                pMVI->pNext = pPrev->pNext;
                pPrev->pNext = pMVI;
            }
            else
            {
                pMVI->pNext = pMVD->pInactiveList;
                pMVD->pInactiveList = pMVI;
            }

            /*  Initial usage count to 1
            */
            pMVI->nUsageCount++;
        }

        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACEBIT_MEM, 1, 0),
            "MEMVAL Remove  : Buffer=%P Len=%lX Flags=%x Reads=%lU Writes=%lU\n",
            pMVI->pBuffer, pMVI->ulBuffLen, pMVI->nFlags, pMVI->ulReadCount, pMVI->ulWriteCount));
    }
    else
    {

 /* Never happens, right?
 */
 DclProductionError();

        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACEBIT_MEM, 1, 0),
            "MEMVAL Deref   : Buffer=%P Len=%lX Flags=%x Usage=%u\n",
            pMVI->pBuffer, pMVI->ulBuffLen, pMVI->nFlags, pMVI->nUsageCount));
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: Validate()

    Parameters:
        pMVD      - A pointer to the MEMVALSERVICEDATA structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Validate(
    MEMVALSERVICEDATA  *pMVD,
    const void         *pBuffer,
    D_UINT32            ulBuffLen,
    unsigned            nFlags)
{
    MEMVALINFO         *pMVI;
    MEMVALINFO         *pPrevMVI;
    DCLSTATUS           dclStat;
    unsigned            nType;

    /*  Validate calls should never have any type set...
    */
    DclAssert(!(nFlags & DCL_MVFLAG_TYPEMASK));

    pMVI = Lookup(pMVD, pBuffer, &pPrevMVI);
    if(!pMVI || ISAUTOMATIC(pMVI->nFlags))
    {
        /*  If we found no matching buffer, or the buffer we did find is
            Automatic, we need to do a bunch of extra work to deal with
            adding the new Automatic node and possibly coalescing stuff,
            so go do that and be done.
        */
        return AddAutomaticNode(pMVD, pMVI, pPrevMVI, pBuffer, ulBuffLen, nFlags | DCL_MVFLAG_TYPEAUTOMATIC);
    }

    /*  At this point we know we are dealing with either a DLMemMgr or
        Explicit node.
    */

    if(nFlags & DCL_MVFLAG_READONLY)
        pMVI->ulReadCount++;
    else
        pMVI->ulWriteCount++;

    nType = pMVI->nFlags & DCL_MVFLAG_TYPEMASK;

    /*  Any buffer which is of the type "automatic" will get automatically
        expanded as necessary.
    */
/*    if((nType == DCL_MVFLAG_TYPEAUTOMATIC) &&
        ((const D_BUFFER*)pBuffer + ulBuffLen > pMVI->pBuffer + pMVI->ulBuffLen))
    {
        pMVI->ulBuffLen += (((const D_BUFFER*)pBuffer + ulBuffLen) - (pMVI->pBuffer + pMVI->ulBuffLen));
    }
*/

    /*  Verify that we are accessing in range...
    */
    if((const D_BUFFER*)pBuffer + ulBuffLen > pMVI->pBuffer + pMVI->ulBuffLen)
        return DCLSTAT_MEMVAL_INVALIDLENGTH;

    /*  Make sure we're not trying to write into a readonly buffer
    */
    if(!(nFlags & DCL_MVFLAG_READONLY) && (pMVI->nFlags & DCL_MVFLAG_READONLY))
        return DCLSTAT_MEMVAL_READONLY;

    /*  For any buffer which was allocated from the DCL memory manager,
        validate the block via the memory tracking service (if enabled).
    */
    if(nType == DCL_MVFLAG_TYPEDLMEMMGR)
    {
        DCLDECLAREREQUESTPACKET (MEMTRACK, BLOCKVERIFY, verify); /* DCLREQ_MEMTRACK_BLOCKVERIFY */

        verify.pBuffer = pMVI->pBuffer;
        verify.fQuiet = FALSE;
        dclStat = DclServiceIoctl(pMVD->hDclInst, DCLSERVICE_MEMTRACK, &verify.ior);
        if(dclStat == DCLSTAT_SUCCESS)
            dclStat = verify.dclStat;
        else if(dclStat == DCLSTAT_SERVICE_NOTREGISTERED || dclStat == DCLSTAT_FEATUREDISABLED)
            dclStat = DCLSTAT_SUCCESS;
    }
    else
    {
        dclStat = DCLSTAT_SUCCESS;
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: Lookup()

    Lookup the specified address in the linked list.  If a record is
    found which contains the address, a pointer to the MEMVALINFO
    structure is returned.

    If the record is not found, NULL will be returned.

    Additionally, in either case, ppPrevMVI will point to the
    preceding record if there is one, or will be NULL if there
    is no preceding record.

    Parameters:
        pMVD      - A pointer to the MEMVALSERVICEDATA structure.
        pBuffer   - The address to lookup.
        ppPrevMVI - A pointer to a buffer in which to store a pointer
                    to the previous MEMVALINFO structure.  This
                    value may be NULL if the information is not
                    desired.

    Return Value:
        Returns a pointer to the encapsulating MEMVALINFO record
        if found, or NULL otherwise.
-------------------------------------------------------------------*/
static MEMVALINFO * Lookup(
    MEMVALSERVICEDATA  *pMVD,
    const void         *pBuffer,
    MEMVALINFO        **ppPrevMVI)
{
    const D_BUFFER     *pBuff = pBuffer;
    MEMVALINFO         *pCurr = pMVD->pActiveList;
    MEMVALINFO         *pPrev = NULL;

    DclAssert(pMVD);
    DclAssert(pBuffer);

    while(pCurr)
    {
        /*  The desired buffer precedes this, just break out and return NULL.
        */
        if(pBuff < pCurr->pBuffer)
            break;

        if(pBuff == pCurr->pBuffer)
        {
            /*  If there are multiple entries with the same matching
                buffer address, use the one with the highest type.
            */
            while(pCurr->pNext && pCurr->pNext->pBuffer == pBuff &&
                ((pCurr->pNext->nFlags & DCL_MVFLAG_TYPEMASK) > (pCurr->nFlags & DCL_MVFLAG_TYPEMASK)))
            {
                pPrev = pCurr;
                pCurr = pCurr->pNext;
            }

            /*  Found an entry which contains this address
            */
            if(ppPrevMVI)
                *ppPrevMVI = pPrev;

            return pCurr;
        }

        if(pBuff < pCurr->pBuffer + pCurr->ulBuffLen)
        {
            /*  If there are multiple entries which contain this address
                use the highest one.
            */
            while(pCurr->pNext && pBuff >= pCurr->pNext->pBuffer &&
                pBuff <= pCurr->pNext->pBuffer + pCurr->pNext->ulBuffLen)
            {
                pPrev = pCurr;
                pCurr = pCurr->pNext;
            }

            /*  Found an entry which contains this address
            */
            if(ppPrevMVI)
                *ppPrevMVI = pPrev;

            return pCurr;
        }

        pPrev = pCurr;
        pCurr = pCurr->pNext;
    }

    if(ppPrevMVI)
        *ppPrevMVI = pPrev;

    return NULL;
}


/*-------------------------------------------------------------------
    Local: Summary()

    Parameters:
        pMVD      - A pointer to the MEMVALSERVICEDATA structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Summary(
    MEMVALSERVICEDATA  *pMVD,
    DCLMEMVALSUMMARY   *pVal,
    D_BOOL              fQuiet,
    D_BOOL              fReset)
{
    MEMVALINFO         *pMVI;
    char                szType[16];
    DCLMEMVALSUMMARY    summary;

    DclAssert(pMVD);

    if(!pVal)
        pVal = &summary;

    DclMemSet(pVal, 0, sizeof(*pVal));

    if(!fQuiet)
    {
        DclPrintf("Active Node Memory Validation Summary:\n");
        DclPrintf("  Type     Address     Length   Usage      Reads     Writes     Flags\n");
    }

    pMVI = pMVD->pActiveList;
    while(pMVI)
    {
        switch(pMVI->nFlags & DCL_MVFLAG_TYPEMASK)
        {
            case DCL_MVFLAG_TYPEAUTOMATIC:
                pVal->ulAutomaticBytes += pMVI->ulBuffLen;
                pVal->nAutomaticBlocks++;
                DclStrCpy(szType, "Automatic");
                break;
            case DCL_MVFLAG_TYPEOSMEMPOOL:
                pVal->ulOSMemPoolBytes += pMVI->ulBuffLen;
                pVal->nOSMemPoolBlocks++;
                DclStrCpy(szType, "OSMemPool");
                break;
            case DCL_MVFLAG_TYPEDLMEMMGR:
                pVal->ulDLMemMgrBytes += pMVI->ulBuffLen;
                pVal->nDLMemMgrBlocks++;
                DclStrCpy(szType, "DLMemMgr ");
                break;
            case DCL_MVFLAG_TYPEEXPLICIT:
                pVal->ulExplicitBytes += pMVI->ulBuffLen;
                pVal->nExplicitBlocks++;
                DclStrCpy(szType, "Explicit ");
                break;
            default:
                DclProductionError();
        }

        if(!fQuiet)
        {
            DclPrintf("%s %P %10lU %7u %10lU %10lU %x %s\n",
                szType, pMVI->pBuffer, pMVI->ulBuffLen, pMVI->nUsageCount,
                pMVI->ulReadCount, pMVI->ulWriteCount,
                pMVI->nFlags, pMVI->nFlags & DCL_MVFLAG_READONLY ? "ReadOnly " : "ReadWrite");
        }

        pMVI = pMVI->pNext;
    }

    if(!fQuiet)
    {
        DclPrintf("%u Automatic memory buffers containing %lU bytes\n", pVal->nAutomaticBlocks,  pVal->ulAutomaticBytes);
        DclPrintf("%u OSMemPool memory buffers containing %lU bytes\n", pVal->nOSMemPoolBlocks, pVal->ulOSMemPoolBytes);
        DclPrintf("%u DLMemMgr memory buffers containing %lU bytes\n", pVal->nDLMemMgrBlocks, pVal->ulDLMemMgrBytes);
        DclPrintf("%u Explicit memory buffers containing %lU bytes\n",  pVal->nExplicitBlocks, pVal->ulExplicitBytes);
    }

    if(!fQuiet)
    {
        DclPrintf("Inactive Node Memory Validation Summary:\n");
        DclPrintf("  Type     Address     Length   Usage      Reads     Writes     Flags\n");
    }

    pMVI = pMVD->pInactiveList;
    while(pMVI)
    {
        switch(pMVI->nFlags & DCL_MVFLAG_TYPEMASK)
        {
/*            case DCL_MVFLAG_TYPEAUTOMATIC:
                pVal->ulAutomaticBytes += pMVI->ulBuffLen;
                pVal->nAutomaticBlocks++;
                DclStrCpy(szType, "Automatic");
                break;*/
            case DCL_MVFLAG_TYPEOSMEMPOOL:
                pVal->ulInactiveOSMemPoolBytes += pMVI->ulBuffLen;
                pVal->nInactiveOSMemPoolBlocks++;
                DclStrCpy(szType, "OSMemPool");
                break;
            case DCL_MVFLAG_TYPEDLMEMMGR:
                pVal->ulInactiveDLMemMgrBytes += pMVI->ulBuffLen;
                pVal->nInactiveDLMemMgrBlocks++;
                DclStrCpy(szType, "DLMemMgr ");
                break;
            case DCL_MVFLAG_TYPEEXPLICIT:
                pVal->ulInactiveExplicitBytes += pMVI->ulBuffLen;
                pVal->nInactiveExplicitBlocks++;
                DclStrCpy(szType, "Explicit ");
                break;
            default:
                DclProductionError();
        }

        if(!fQuiet)
        {
            DclPrintf("%s %P %10lU %7u %10lU %10lU %x %s\n",
                szType, pMVI->pBuffer, pMVI->ulBuffLen, pMVI->nUsageCount,
                pMVI->ulReadCount, pMVI->ulWriteCount,
                pMVI->nFlags, pMVI->nFlags & DCL_MVFLAG_READONLY ? "ReadOnly " : "ReadWrite");
        }

        pMVI = pMVI->pNext;
    }

    if(!fQuiet)
    {
/*        DclPrintf("%u Automatic memory buffers containing %lU bytes\n", pVal->nAutomaticBlocks,  pVal->ulAutomaticBytes); */
        DclPrintf("%u OSMemPool memory buffers containing %lU bytes\n", pVal->nInactiveOSMemPoolBlocks, pVal->ulInactiveOSMemPoolBytes);
        DclPrintf("%u DLMemMgr memory buffers containing %lU bytes\n", pVal->nInactiveDLMemMgrBlocks, pVal->ulInactiveDLMemMgrBytes);
        DclPrintf("%u Explicit memory buffers containing %lU bytes\n",  pVal->nInactiveExplicitBlocks, pVal->ulInactiveExplicitBytes);
    }

    if(fReset)
    {
        DclPrintf("The RESET option is not yet implemented\n");
        return DCLSTAT_UNSUPPORTEDREQUEST;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: ExpandFreeList()

    Parameters:
        pMVD      - A pointer to the MEMVALSERVICEDATA structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS ExpandFreeList(
    MEMVALSERVICEDATA  *pMVD,
    unsigned            nCount)
{
    MEMVALPOOLHDR      *pHdr;
    MEMVALINFO         *paMVI;
    unsigned            nn;

    DclAssert(pMVD);
    DclAssert(nCount);

    pHdr = DclMemAllocZero(sizeof(*pHdr) + (sizeof(*paMVI) * nCount));
    if(!pHdr)
        return DCLSTAT_MEMALLOCFAILED;

    paMVI = (MEMVALINFO*)(pHdr + 1);

    /*  Place everything in the "free" list.
    */
    for(nn = 0; nn < nCount - 1; nn++)
    {
        paMVI[nn].pNext = &paMVI[nn + 1];
    }

    /*  Link the new pool into the free list
    */
    paMVI[nn].pNext = pMVD->pFreeList;
    pMVD->pFreeList = paMVI;

    /*  Link the new pool into the list of pools
    */
    pHdr->pNextPool = pMVD->pPoolList;
    pMVD->pPoolList = pHdr;

    pHdr->nEntries = nCount;
    pMVD->nTotalNodes += nCount;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: AddAutomaticNode()

    Parameters:
        pMVD      - A pointer to the MEMVALSERVICEDATA structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS AddAutomaticNode(
    MEMVALSERVICEDATA  *pMVD,
    MEMVALINFO         *pMVI,
    MEMVALINFO         *pPrevMVI,
    const void         *pBuffer,
    D_UINT32            ulBuffLen,
    unsigned            nFlags)
{
    const D_BUFFER     *pBuff = pBuffer;
    DCLSTATUS           dclStat = DCLSTAT_SUCCESS;

    DclAssert(pMVD);
    DclAssert(pBuffer);

    if(pMVI && pBuffer == pMVI->pBuffer)
    {
        DclAssert(ISAUTOMATIC(pMVI->nFlags));
        DclAssert(ISAUTOMATIC(nFlags));
        DclAssert(pBuffer == pMVI->pBuffer);
        DclAssert(pMVI->nUsageCount);

        pMVI->nUsageCount++;
        DclAssert(pMVI->nUsageCount);

        /*  Turn off the READONLY flag if the newer access is READWRITE
        */
        if(ISREADONLY(pMVI->nFlags) && !ISREADONLY(nFlags))
            pMVI->nFlags &= ~DCL_MVFLAG_READONLY;

        /*  Increase the buffer size if necessary.  Doing this requires
            that we check for nodes which can be coalesced.
        */
        if(pMVI->ulBuffLen < ulBuffLen)
        {
            if(pMVI->pNext && !ISAUTOMATIC(pMVI->pNext->nFlags) &&
                pMVI->pBuffer + pMVI->ulBuffLen > pMVI->pNext->pBuffer)
            {
                /*  Not legal to increase an Automatic buffer's length so
                    far that infringes on a buffer which is NOT Automatic.
                */
                return DCLSTAT_MEMVAL_INVALIDLENGTH;
            }

            pMVI->ulBuffLen = ulBuffLen;

            /*  Loop coalescing nodes until we can do no more
            */
            while(TryCoalesceNodes(pMVD, pMVI));
        }

        if(nFlags & DCL_MVFLAG_READONLY)
            pMVI->ulReadCount++;
        else
            pMVI->ulWriteCount++;

        return DCLSTAT_SUCCESS;
    }

    if(pMVI)
        pPrevMVI = pMVI;

    /*  If the new buffer falls partly or completely within a previous
        entry...
    */
    if(pPrevMVI && pPrevMVI->pBuffer + pPrevMVI->ulBuffLen > pBuff)
    {
        /*  If that previous entry is not AUTOMATIC, then the new access
            MUST conform to the contraints of the original buffer.
        */
        if(!ISAUTOMATIC(pPrevMVI->nFlags))
        {
            if(pBuff + ulBuffLen >  pPrevMVI->pBuffer + pPrevMVI->ulBuffLen)
                return DCLSTAT_MEMVAL_INVALIDLENGTH;

            if(ISREADONLY(pPrevMVI->nFlags) && !ISREADONLY(nFlags))
                return DCLSTAT_MEMVAL_READONLY;

            if(nFlags & DCL_MVFLAG_READONLY)
                pPrevMVI->ulReadCount++;
            else
                pPrevMVI->ulWriteCount++;

            return DCLSTAT_SUCCESS;
        }
    }

    /*  We want to create a new Automatic node, however it may not overlap
        the next node if that node is not also Automatic.
    */
    if(pPrevMVI && pPrevMVI->pNext && !ISAUTOMATIC(pPrevMVI->pNext->nFlags) &&
        pBuff + ulBuffLen > pPrevMVI->pNext->pBuffer)
    {
        /*  Not legal to increase an Automatic buffer's length so
            far that infringes on a buffer which is NOT Automatic.
        */
        return DCLSTAT_MEMVAL_INVALIDLENGTH;
    }

    /*  At this point we know that there either is no previous buffer, or
        if there is, it is either automatic, or it is known to not overlap
        the new buffer.  We also know that we will not improperly overlap
        a subsequent buffer (if so it is Automatic, and we will coalesce).

        Regardless which case it is, we'll add a new entry for the new
        buffer, link it into the chain, and let the standard coalesce
        function deal with combining things if necessary.
    */

    if(!pMVD->pFreeList)
    {
        dclStat = ExpandFreeList(pMVD, NODE_EXPAND_COUNT);
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;

        DclAssert(pMVD->pFreeList);
    }

    /*  Remove the item from the head of the free list
    */
    pMVI = pMVD->pFreeList;
    pMVD->pFreeList = pMVI->pNext;

    pMVI->pBuffer = pBuff;
    pMVI->ulBuffLen = ulBuffLen;
    pMVI->nFlags = nFlags;
    pMVI->nUsageCount = 1;

    if(nFlags & DCL_MVFLAG_READONLY)
        pMVI->ulReadCount++;
    else
        pMVI->ulWriteCount++;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACEBIT_MEM, 1, 0),
        "MEMVAL AddAuto : Buffer=%P Len=%lX Flags=%x Usage=%u\n",
        pMVI->pBuffer, pMVI->ulBuffLen, pMVI->nFlags, pMVI->nUsageCount));

    if(pPrevMVI)
    {
        /*  Link the new node into the proper point in the list
        */
        pMVI->pNext = pPrevMVI->pNext;
        pPrevMVI->pNext = pMVI;

        if(ISAUTOMATIC(pPrevMVI->nFlags))
        {
            /*  Loop coalescing nodes until we can do no more
            */
            while(TryCoalesceNodes(pMVD, pPrevMVI));

            /*  The node we just created could have disappeared -- clear out
                the pointer so we don't accidentally try to coalesce it below.
            */
            if(pPrevMVI->pNext != pMVI)
                pMVI = NULL;
        }
    }
    else
    {
        /*  No previous node, so the new one goes at the head of the list
        */
        pMVI->pNext = pMVD->pActiveList;
        pMVD->pActiveList = pMVI;
    }

    /*  Loop coalescing nodes until we can do no more
    */
    if(pMVI)
        while(TryCoalesceNodes(pMVD, pMVI));

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: TryCoalesceNodes()

    Attempt to coalesce the current node with its next node.  This
    is only done with Automatic nodes.

    Parameters:
        pMVD    - A pointer to the MEMVALSERVICEDATA structure.
        pMVI    - The node to start with.

    Return Value:
        Returns TRUE if the next node was coalesced, or FALSE if not.
-------------------------------------------------------------------*/
static D_BOOL TryCoalesceNodes(
    MEMVALSERVICEDATA  *pMVD,
    MEMVALINFO         *pMVI)
{
    MEMVALINFO         *pNext;

    DclAssert(pMVD);
    DclAssert(pMVI);
    DclAssert(ISAUTOMATIC(pMVI->nFlags));

    if(!pMVI->pNext)
        return FALSE;
    else
        pNext = pMVI->pNext;

    if(ISAUTOMATIC(pNext->nFlags) &&
        pMVI->pBuffer + pMVI->ulBuffLen >= pNext->pBuffer)
    {
        if(pMVI->pBuffer + pMVI->ulBuffLen == pNext->pBuffer &&
            ISREADONLY(pMVI->nFlags) != ISREADONLY(pNext->nFlags))
        {
            /*  If the length of the first buffer runs directly up to the
                second buffer, but they do not overlap, and if the READONLY
                flag is different between the two buffers, don't coalesce
                them -- one is READONLY, one is READWRITE, and there's no
                overlap
            */
            return FALSE;
        }

        /*  Grow the first buffer's length if the second buffer is longer
            than what the first buffer includes.  (The first buffer still
            may be longer than the whole second buffer, in which case we
            will need to do a coalesce check again, but that is done at
            a higher level.)
        */
        if(pMVI->pBuffer + pMVI->ulBuffLen < pNext->pBuffer + pNext->ulBuffLen)
            pMVI->ulBuffLen += (pNext->ulBuffLen - ((pMVI->pBuffer + pMVI->ulBuffLen) - pNext->pBuffer));

        /*  Combine the stats
        */
        pMVI->nUsageCount += pNext->nUsageCount;
        pMVI->ulReadCount += pNext->ulReadCount;
        pMVI->ulWriteCount += pNext->ulWriteCount;

        /*  If the trailing mode is not Read-Only, then the
            updated node may not be Read-Only either.
        */
        if(ISREADONLY(pMVI->nFlags) && !ISREADONLY(pNext->nFlags))
            pMVI->nFlags &= ~DCL_MVFLAG_READONLY;

        /*  Remove the trailing node from the active list, and
            add it to the free list.
        */
        pMVI->pNext = pNext->pNext;
        pNext->pNext = pMVD->pFreeList;
        pMVD->pFreeList = pNext;

        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACEBIT_MEM, 1, 0),
            "MEMVAL Combine : Buffer %P into Buffer %P, NewLen=%lU\n",
            pNext->pBuffer, pMVI->pBuffer, pMVI->ulBuffLen));

        return TRUE;
    }

    return FALSE;
}







#endif  /* DCLCONF_MEMORYVALIDATION */


