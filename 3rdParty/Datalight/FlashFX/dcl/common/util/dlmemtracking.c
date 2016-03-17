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
  jurisdictions.  Patents may be pending.

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

    This module contain general memory management routines built upon the
    primitives provided in the OS Services Layer.

    Note that the memory tracking code uses production messages and asserts
    because we want it to be function even if running in release mode.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlmemtracking.c $
    Revision 1.7  2010/04/17 21:51:27Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.6  2010/01/07 02:46:43Z  garyp
    Updated so that at service shutdown time, the pool (if any) is verified,
    and any remaining memory allocations are reported as memory leaks.
    Revision 1.5  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.4  2009/11/11 16:10:44Z  garyp
    Fixed to avoid using a NULL pointer when the profiler is disabled.
    Revision 1.3  2009/11/09 05:34:42Z  garyp
    Updated so the entire module is excluded if memory tracking is not enabled.
    Revision 1.2  2009/06/27 00:35:37Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.4  2009/03/30 02:49:35Z  garyp
    Documentation fixes -- no functional changes.
    Revision 1.1.1.3  2009/02/04 03:06:07Z  garyp
    Modified the way DCLIOFUNC_MEMTRACK_POOLDUMP is handled to decode the
    packet at a higher level.
    Revision 1.1.1.2  2009/01/16 03:19:01Z  garyp
    Fixed to compile with some picky compilers.
    Revision 1.1  2009/01/05 20:57:22Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#if DCLCONF_MEMORYTRACKING

#include <dlapiprv.h>
#include <dlstats.h>
#include <dlservice.h>
#include <dlinstance.h>
#include "dlmemtracking.h"

/*  The memory tracking system keeps track of each memory allocation.
    To do this, it allocates additional space for a control structure.
    The system also allocates space to place sentinel values both before
    and after the client memory space.  These are used both to ensure
    proper alignment and to detect underruns and overruns.

    The individual values and minimum sizes of the sentinels can be
    controlled with the settings below.  The minimum sentinel sizes
    must be multiples of DCL_ALIGNSIZE.  Additional bytes will be added
    to both the head and tail sentinels as needed to keep everything
    aligned.  The client address returned must be aligned and the overall
    total size of the allocation (including all overhead) must be aligned.

    The size of the head and tail sentinels may not be aligned.

    ------------------------------------------------------------------
    | Control Header | Head Sentinel | Client Memory | Tail Sentinel |
    ------------------------------------------------------------------
*/
#define ALLOC_FILL_VALUE    '*' /* fill value for memory allocations */
#define FREE_FILL_VALUE     '#' /* fill value for freed memory */
#define ALLOC_HEAD_SENTINEL_VAL ((D_BUFFER)~ALLOC_FILL_VALUE)
#define ALLOC_TAIL_SENTINEL_VAL ((D_BUFFER)~FREE_FILL_VALUE)
#define ALLOC_HEAD_SENTINEL_MIN (DCL_ALIGNSIZE)
#define ALLOC_TAIL_SENTINEL_MIN (DCL_ALIGNSIZE)

typedef struct sALLOC_HDR
{
    struct sALLOC_HDR  *pNext;
    D_UINT32            ulSize;   /* original size */
    const char         *pszOwner;
    D_BUFFER           *pHeadSentinel;
    D_BUFFER           *pTailSentinel;
} ALLOC_HDR;

/*  Macros for calculating the size of the head and tail sentinels.
    The head sentinel size is fixed.  The tail sentinel size varies based
    on the requested allocation size.
*/
#define ALLOC_HEAD_SENTINEL_SIZE    (ALLOC_HEAD_SENTINEL_MIN + DCLALIGNPAD(sizeof(ALLOC_HDR)))
#define ALLOC_TAIL_SENTINEL_SIZE    (ALLOC_TAIL_SENTINEL_MIN + DCLALIGNPAD(pAlloc->ulSize))

/*  Macro to calculate the overall size of the header, including the head
    sentinel.  This must be evenly divisible by DCL_ALIGNSIZE to ensure
    that the address returned to the caller is properly aligned.
*/
#define ALLOC_HDR_SIZE      (sizeof(ALLOC_HDR) + ALLOC_HEAD_SENTINEL_SIZE)

/*  This is the fixed amount of additional memory needed by the memory
    tracking mechanism for each allocation.
*/
#define ALLOC_EXTRAMEMORY   (ALLOC_HDR_SIZE + ALLOC_TAIL_SENTINEL_MIN)

/*  Static data
*/
typedef struct
{
    DCLSERVICEHEADER   *pService;
    D_ATOMIC32          ulBusyFlag;
    unsigned            nErrorMode;
    ALLOC_HDR          *pListHead;
    D_UINT32            ulMaxConcurrentBytes;
    D_UINT32            ulMaxConcurrentBlocks;
    D_UINT32            ulBytesCurrentlyAllocated;
    D_UINT32            ulBlocksCurrentlyAllocated;
    D_UINT32            ulTotalAllocCalls;
    D_UINT32            ulLargestAllocation;
} DCLMEMTRACKINGINFO;

/*  Prototypes
*/
static DCLSTATUS    ServiceIoctl(DCLSERVICEHANDLE hService, DCLREQ_SERVICE *pReq, void *pMTI);
static DCLSTATUS    LinkMemoryBlock(    DCLMEMTRACKINGINFO *pMTI, void **ppClient, D_BUFFER *pBlock, D_UINT32 ulSize, const char **ppszOwner);
static DCLSTATUS    UnlinkMemoryBlock(  DCLMEMTRACKINGINFO *pMTI, void **ppBlock, D_BUFFER *pClient, D_UINT32 *pulSize);
static DCLSTATUS    ValidateBlock(      DCLMEMTRACKINGINFO *pMTI, const ALLOC_HDR *pAlloc, D_BOOL fAssert, D_BOOL fQuiet);
static DCLSTATUS    BlockVerify(        DCLMEMTRACKINGINFO *pMTI, const D_BUFFER *pMem, D_BOOL fQuiet);
static DCLSTATUS    PoolVerify(         DCLMEMTRACKINGINFO *pMTI, D_BOOL fQuiet);
static DCLSTATUS    PoolDump(           DCLMEMTRACKINGINFO *pMTI, unsigned nLength);
static DCLSTATUS    StatsQuery(         DCLMEMTRACKINGINFO *pMTI, DCLMEMSTATS *pDMS, D_BOOL fVerbose, D_BOOL fReset);



                /*---------------------------------*\
                 *                                 *
                 *        Public Functions         *
                 *                                 *
                \*---------------------------------*/



/*-------------------------------------------------------------------
    Protected: DclMemTrackServiceInit()

    Initialize the Memory Tracking service.

    Parameters:
        hDclInst - The DCL instance handle.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclMemTrackServiceInit(
    DCLINSTANCEHANDLE           hDclInst)
{
    static DCLSERVICEHEADER     SrvMemTrack[DCL_MAX_INSTANCES];
    static DCLMEMTRACKINGINFO   MTI[DCL_MAX_INSTANCES];
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
    DclMemSet(&MTI[nInstNum], 0, sizeof(MTI[0]));

    pSrv = &SrvMemTrack[nInstNum];
    dclStat = DclServiceHeaderInit(&pSrv, "DLMEMTRACK", DCLSERVICE_MEMTRACK, ServiceIoctl, &MTI[nInstNum], 0);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        dclStat = DclServiceCreate(hDclInst, pSrv);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            /*  Save a back-pointer into the service structure so we can
                get to the mutex directly.
            */
            MTI[nInstNum].pService = pSrv;
        }
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Protected: DclMemTrackErrorMode()

    Set the error mode for the Memory Tracking service, and return
    the original mode.

    Parameters:
        nNewMode - The new error mode value, which must be
                   DCLMEM_ERROR_FAIL or DCLMEM_ERROR_ASSERT.

    Return Value:
        Returns an unsigned int containing the original error mode,
        or DCLMEM_ERROR_INVALID if the call failed.
-------------------------------------------------------------------*/
unsigned DclMemTrackErrorMode(
    DCLINSTANCEHANDLE           hDclInst,
    unsigned                    nNewMode)
{
    DCLDECLAREREQUESTPACKET     (MEMTRACK, ERRORMODE, mode); /* DCLREQ_MEMTRACK_ERRORMODE */
    DCLSTATUS                   dclStat;

    DclAssert(nNewMode == DCLMEM_ERROR_FAIL || nNewMode == DCLMEM_ERROR_ASSERT);

    mode.nNewMode = nNewMode;
    dclStat = DclServiceIoctl(hDclInst, DCLSERVICE_MEMTRACK, &mode.ior);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(dclStat != DCLSTAT_SERVICE_NOTREGISTERED)
        {
            DCLPRINTF(1, ("DclMemTrackErrorMode() unexpected error %lX\n", dclStat));
        }

        return DCLMEM_ERROR_INVALID;
    }

    return mode.nOldMode;
}


/*-------------------------------------------------------------------
    Protected: DclMemTrackPoolVerify()

    Validate the memory pool.

    Parameters:
        hDclInst - The DCL instance handle.
        fQuiet   - TRUE to disable verbose error messages.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclMemTrackPoolVerify(
    DCLINSTANCEHANDLE           hDclInst,
    D_BOOL                      fQuiet)
{
    DCLDECLAREREQUESTPACKET     (MEMTRACK, POOLVERIFY, verify); /* DCLREQ_MEMTRACK_POOLVERIFY */
    DCLSTATUS                   dclStat;

    verify.fQuiet = fQuiet;
    dclStat = DclServiceIoctl(hDclInst, DCLSERVICE_MEMTRACK, &verify.ior);
    if(dclStat != DCLSTAT_SUCCESS && dclStat != DCLSTAT_SERVICE_NOTREGISTERED)
    {
        DCLPRINTF(1, ("DclMemTrackPoolVerify() unexpected error %lX\n", dclStat));
    }

    return dclStat;
}



                /*---------------------------------*\
                 *                                 *
                 *       Private Functions         *
                 *                                 *
                \*---------------------------------*/


/*-------------------------------------------------------------------
    Private: DclMemTrackAlloc()

    Allocate a block of memory.  All buffers returned by this 
    function are required to be aligned on DCL_ALIGNSIZE
    boundaries.

    All sizes <should> be on DCL_ALIGNSIZE boundaries.  If not,
    a debug warning will be displayed, and the buffer size will
    be automatically increased as necessary.

    Parameters:
        ulSize - The number of bytes to allocate

    Return Value:
        Returns a pointer to allocated memory, or NULL if failure.
-------------------------------------------------------------------*/
void * DclMemTrackAlloc(
    D_UINT32    ulSize)
{
    void       *pMem;

    pMem = DclOsMemAlloc(ulSize + ALLOC_EXTRAMEMORY);

  #if DCLCONF_MEMORYVALIDATION
    if(pMem)
        DclMemValBufferAdd(NULL, pMem, ulSize + ALLOC_EXTRAMEMORY, DCL_MVFLAG_TYPEOSMEMPOOL);
  #endif

    return pMem;
}


/*-------------------------------------------------------------------
    Private: DclMemTrackAllocLink()

    Link a block of memory into the pool.

    Parameters:
        pMem   - A pointer to the memory to link
        ulSize - The number of bytes to allocate

    Return Value:
        Returns a pointer to allocated memory, or NULL if failure.
-------------------------------------------------------------------*/
void * DclMemTrackAllocLink(
    void               *pMem,
    D_UINT32            ulSize)
{
    char               *pszOwner = NULL;
    DCLMEMTRACKINGINFO *pMTI;
    DCLSTATUS           dclStat;

    dclStat = DclServiceData(0, DCLSERVICE_MEMTRACK, (void**)&pMTI);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DCLPRINTF(2, ("DclMemTrackAllocLink() Memory tracking not available, Status=%lX\n", dclStat));

        /*  Allow the link request to fail.  This is allows memory
            allocates to succeed even when the memory tracking system
            is not working (so long as the alloc itself worked).  This
            is necessary at startup because the memory tracking system
            uses a mutex, and a mutex allocates memory, ...
        */
        return pMem;
    }

    if(pMTI->pService->pServiceMutex)
        DclMutexAcquire(pMTI->pService->pServiceMutex);

    pMTI->ulTotalAllocCalls++;

    if(pMTI->ulLargestAllocation < ulSize)
        pMTI->ulLargestAllocation = ulSize;

    dclStat = LinkMemoryBlock(pMTI, &pMem, pMem, ulSize, (const char**)&pszOwner);
    if(dclStat != DCLSTAT_SUCCESS)
    {
      #if D_DEBUG
        if(pMTI->nErrorMode == DCLMEM_ERROR_ASSERT)
            DclError();
      #endif

        DclMemTrackFree(pMem);
        pMem = NULL;
    }

    if(pMTI->pService->pServiceMutex)
        DclMutexRelease(pMTI->pService->pServiceMutex);

    /*  And the pointer better still be aligned now...
    */
    DclAssert(DCLISALIGNED((D_UINTPTR)pMem, DCL_ALIGNSIZE));

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MEM, 1, 0),
        "DclMemTrackAllocLink(%lU) Owner=%s, returning %P\n", ulSize, pszOwner, pMem));

    return pMem;
}


/*-------------------------------------------------------------------
    Private: DclMemTrackFree()

    Release a block of memory that was allocated with DclMemTrackAlloc().

    Parameters:
        pMem - A pointer to the allocated memory.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclMemTrackFree(
    void       *pMem)
{
    DCLSTATUS   dclStat;

  #if DCLCONF_MEMORYVALIDATION
    dclStat = DclMemValBufferRemove(NULL, pMem);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;
  #endif

    dclStat = DclOsMemFree(pMem);

    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclMemTrackFreeUnlink()

    Unlink a block of memory from the pool.

    Parameters:
        ppMem - A pointer to a location in which the originally
                allocated memory pointer will be stored.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclMemTrackFreeUnlink(
    void              **ppMem)
{
    DCLSTATUS           dclStat;
    D_UINT32            ulSize;
    void               *pBlk;
    DCLMEMTRACKINGINFO *pMTI;

    dclStat = DclServiceData(0, DCLSERVICE_MEMTRACK, (void**)&pMTI);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    if(pMTI->pService->pServiceMutex)
        DclMutexAcquire(pMTI->pService->pServiceMutex);

    dclStat = UnlinkMemoryBlock(pMTI, &pBlk, *ppMem, &ulSize);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MEM, 1, 0),
            "DclMemTrackFreeUnlink(%P) freeing %lU bytes, Block=%P\n", *ppMem, ulSize, pBlk));

        *ppMem = pBlk;
    }

    if(pMTI->pService->pServiceMutex)
        DclMutexRelease(pMTI->pService->pServiceMutex);

    return dclStat;
}



                /*---------------------------------*\
                 *                                 *
                 *    Static Internal Functions    *
                 *                                 *
                \*---------------------------------*/



/*-------------------------------------------------------------------
    Local: ServiceIoctl()

    Dispatch Memory Tracking service requests.

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
    DCLMEMTRACKINGINFO *pMTI = pPriv;

    DclAssert(hService);
    DclAssert(pReq);
    DclAssert(pMTI);

    if(pReq->ior.ulReqLen != sizeof(*pReq))
        return DCLSTAT_BADSTRUCLEN;

    switch(pReq->ior.ioFunc)
    {
        case DCLIOFUNC_SERVICE_CREATE:
            DclAssert(pReq->pSubRequest == NULL);

            /*  Output memory tracking information.
            */
            DCLPRINTF(2, ("Memory Tracking Information\n"));
            DCLPRINTF(2, ("  DCL_ALIGNSIZE            = %4u\n",   DCL_ALIGNSIZE));
            DCLPRINTF(2, ("  ALLOC_HEAD_SENTINEL_MIN  = %4u\n",   ALLOC_HEAD_SENTINEL_MIN));
            DCLPRINTF(2, ("  ALLOC_TAIL_SENTINEL_MIN  = %4u\n",   ALLOC_TAIL_SENTINEL_MIN));
            DCLPRINTF(2, ("  ALLOC_HEAD_SENTINEL_VAL  = 0x%2x\n", ALLOC_HEAD_SENTINEL_VAL));
            DCLPRINTF(2, ("  ALLOC_TAIL_SENTINEL_VAL  = 0x%2x\n", ALLOC_TAIL_SENTINEL_VAL));
            DCLPRINTF(2, ("  sizeof(ALLOC_HDR)        = %4u\n",   sizeof(ALLOC_HDR)));
            DCLPRINTF(2, ("  ALLOC_HEAD_SENTINEL_SIZE = %4u\n",   ALLOC_HEAD_SENTINEL_SIZE));
            DCLPRINTF(2, ("  ALLOC_HDR_SIZE           = %4u\n",   ALLOC_HDR_SIZE));
            DCLPRINTF(2, ("  ALLOC_EXTRAMEMORY        = %4u\n",   ALLOC_EXTRAMEMORY));

            DclAssert(DCLISALIGNED(ALLOC_HEAD_SENTINEL_MIN, DCL_ALIGNSIZE));
            DclAssert(DCLISALIGNED(ALLOC_TAIL_SENTINEL_MIN, DCL_ALIGNSIZE));
            DclAssert(DCLISALIGNED(ALLOC_EXTRAMEMORY, DCL_ALIGNSIZE));
            DclAssert(DCLISALIGNED(ALLOC_HDR_SIZE, DCL_ALIGNSIZE));

            return DCLSTAT_SUCCESS;

        case DCLIOFUNC_SERVICE_DESTROY:
        {
            DCLSTATUS   dclStat;
            ALLOC_HDR  *pAllocation;
            
            DclAssert(pReq->pSubRequest == NULL);

            dclStat = PoolVerify(pMTI, FALSE);
            if(dclStat != DCLSTAT_SUCCESS)
                DclPrintf("DLMEMTRACK Service: Detected a pool verification error (%lX) at shutdown\n", dclStat);

            /*  Everything we allocated should have been freed by now,
                therefore anything remaining should be considered to be
                a memory leak.
            */
            pAllocation = pMTI->pListHead;
            while(pAllocation != NULL)
            {
                DclPrintf("DLMEMTRACK Service: Memory leak! pMem=%P Size=%lX Owner=\"%s\"\n",
                    (D_BYTE*)pAllocation+ALLOC_HDR_SIZE, pAllocation->ulSize, pAllocation->pszOwner);

                pAllocation = pAllocation->pNext;
            }

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MEM, 1, 0),
                "DLMEMTRACK Service uninitialized\n"));

            return DCLSTAT_SUCCESS;
        }
        
        case DCLIOFUNC_SERVICE_DISPATCH:
        {
            if(pReq->pSubRequest == NULL)
                return DCLSTAT_SERVICE_BADSUBREQUEST;

            if(pReq->pSubRequest->ulReqLen < sizeof(pReq->pSubRequest))
                return DCLSTAT_BADSTRUCLEN;

            switch(pReq->pSubRequest->ioFunc)
            {
                case DCLIOFUNC_MEMTRACK_BLOCKVERIFY:
                {
                    DCLREQ_MEMTRACK_BLOCKVERIFY *pSubReq = (DCLREQ_MEMTRACK_BLOCKVERIFY*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    pSubReq->dclStat = BlockVerify(pMTI, pSubReq->pBuffer, pSubReq->fQuiet);

                    return DCLSTAT_SUCCESS;
                }

                case DCLIOFUNC_MEMTRACK_POOLVERIFY:
                {
                    DCLREQ_MEMTRACK_POOLVERIFY *pSubReq = (DCLREQ_MEMTRACK_POOLVERIFY*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    return PoolVerify(pMTI, pSubReq->fQuiet);
                }
                
                case DCLIOFUNC_MEMTRACK_POOLDUMP:
                {
                    DCLREQ_MEMTRACK_POOLDUMP *pSubReq = (DCLREQ_MEMTRACK_POOLDUMP*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    pSubReq->dclStat = PoolDump(pMTI, pSubReq->nLength);

                    return DCLSTAT_SUCCESS;
                }

                case DCLIOFUNC_MEMTRACK_STATS:
                {
                    DCLREQ_MEMTRACK_STATS *pSubReq = (DCLREQ_MEMTRACK_STATS*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    return StatsQuery(pMTI, &pSubReq->dms, pSubReq->fVerbose, pSubReq->fReset);
                }
                
                case DCLIOFUNC_MEMTRACK_ERRORMODE:
                {
                    DCLREQ_MEMTRACK_ERRORMODE  *pSubReq = (DCLREQ_MEMTRACK_ERRORMODE*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    DclAssert(pSubReq->nNewMode == DCLMEM_ERROR_ASSERT || pSubReq->nNewMode == DCLMEM_ERROR_FAIL);
                    DclAssert(pMTI->nErrorMode == DCLMEM_ERROR_ASSERT || pMTI->nErrorMode == DCLMEM_ERROR_FAIL);

                    pSubReq->nOldMode = pMTI->nErrorMode;
                    pMTI->nErrorMode = pSubReq->nNewMode;

                    return DCLSTAT_SUCCESS;
                }

                default:
                    DCLPRINTF(1, ("DLMEMTRACK:ServiceIoctl() Unsupported subfunction %x\n", pReq->pSubRequest->ioFunc));
                    return DCLSTAT_SERVICE_UNSUPPORTEDREQUEST;
            }
        }

        case DCLIOFUNC_SERVICE_REGISTER:
        case DCLIOFUNC_SERVICE_DEREGISTER:
            DclAssert(pReq->pSubRequest == NULL);

            /*  Nothing to do
            */
            return DCLSTAT_SUCCESS;

        default:
            DCLPRINTF(1, ("DLMEMTRACK:ServiceIoctl() Unhandled request %x\n", pReq->ior.ioFunc));
            return DCLSTAT_SERVICE_UNHANDLEDREQUEST;
    }
}


/*-------------------------------------------------------------------
    Local: LinkMemoryBlock()

    This function adds tracking information to a block of memory
    an inserts it into a linked list.

    Parameters:
        pMTI      - A pointer to the DCLMEMTRACKINGINFO structure.
        ppClient  - A pointer to a buffer into which a pointer to the
                    usable client portion of the memory block.  This
                    buffer will only be modified if the function
                    returns successfully, so it is safe for ppClient
                    to point to pBlock.
        pBlock    - A pointer to the allocated memory block, which
                    includes the requested memory and the extra space
                    for the header and tail.
        ulSize    - The number of bytes originally requested.  This
                    is NOT the total number of bytes in the
                    allocation.
        ppszOwner - A pointer to a location in which a pointer to
                    the memory owner string will be stored.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS LinkMemoryBlock(
    DCLMEMTRACKINGINFO *pMTI,
    void              **ppClient,
    D_BUFFER           *pBlock,
    D_UINT32            ulSize,
    const char        **ppszOwner)
{
    ALLOC_HDR          *pAlloc = (ALLOC_HDR *)pBlock;
    ALLOC_HDR          *pLastAlloc;
    static const char   szUnknown[] = "Unknown";
    DCLSTATUS           dclStat;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MEM, 2, 0),
        "LinkMemoryBlock() pBlock=%P Len=%lU\n", pBlock, ulSize));

    DclAssert(ppClient);
    DclAssert(pBlock);
    DclAssert(ulSize);
    DclAssert(ppszOwner);

    /*  Point to the block of memory that we are going to return.
    */
    pBlock = pBlock + ALLOC_HDR_SIZE;

    /*  Fill in the allocation header
    */
    pAlloc->pNext = NULL;
    pAlloc->ulSize = ulSize;
    pAlloc->pHeadSentinel = (D_BUFFER*)pAlloc + sizeof(ALLOC_HDR);
    pAlloc->pTailSentinel = (D_BUFFER*)pBlock + pAlloc->ulSize;

    /*  If the profiler is enabled, use the current context as the owner of
        this memory block, otherwise just mark it as unknown.
    */
    pAlloc->pszOwner = DclProfContextName();
    if(!pAlloc->pszOwner)
        pAlloc->pszOwner = &szUnknown[0];

    /*  Place the header sentinel at the beginning of the allocation to detect
        underruns.  This will include the extra space needed to make the
        overall size of the header to be of an aligned size.
    */
    DclMemSet(pAlloc->pHeadSentinel, ALLOC_HEAD_SENTINEL_VAL, ALLOC_HEAD_SENTINEL_SIZE);

    /*  Place the tail sentinel at the end of the allocation to detect overruns.
        This will include the extra space required to make the total memory
        allocated to be of an aligned size.
    */
    DclMemSet(pAlloc->pTailSentinel, ALLOC_TAIL_SENTINEL_VAL, ALLOC_TAIL_SENTINEL_SIZE);

    /*  Fill with a known, non-zero value to be able to tell if any memory
        is used before it is initialized.
    */
    DclMemSet(pBlock, ALLOC_FILL_VALUE, pAlloc->ulSize);

    /*  Validate the block we just set up
    */
    dclStat = ValidateBlock(pMTI, pAlloc, pMTI->nErrorMode, FALSE);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    /*  Append this allocation to the list of allocations
    */
    if(pMTI->pListHead == NULL)
    {
        /*  Set the first allocation to the current allocation
        */
        pMTI->pListHead = pAlloc;
    }
    else
    {
        /*  Find the last allocation and append the current allocation to
            the last allocation.
        */
        pLastAlloc = pMTI->pListHead;

        dclStat = ValidateBlock(pMTI, pLastAlloc, pMTI->nErrorMode, FALSE);
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;

        while(pLastAlloc->pNext != NULL)
        {
            pLastAlloc = pLastAlloc->pNext;
            dclStat = ValidateBlock(pMTI, pLastAlloc, pMTI->nErrorMode, FALSE);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;
        }
        pLastAlloc->pNext = pAlloc;
    }

    /*  Update internal counts of blocks and bytes allocated
    */
    pMTI->ulBlocksCurrentlyAllocated++;
    if(pMTI->ulMaxConcurrentBlocks < pMTI->ulBlocksCurrentlyAllocated)
        pMTI->ulMaxConcurrentBlocks = pMTI->ulBlocksCurrentlyAllocated;

    pMTI->ulBytesCurrentlyAllocated += pAlloc->ulSize;
    if(pMTI->ulMaxConcurrentBytes < pMTI->ulBytesCurrentlyAllocated)
        pMTI->ulMaxConcurrentBytes = pMTI->ulBytesCurrentlyAllocated;

    *ppszOwner = pAlloc->pszOwner;

    *ppClient = (void *)pBlock;

    /*  dclStat will be DCLSTAT_SUCCESS if we get here...
    */
    return dclStat;
}


/*-------------------------------------------------------------------
    Local: UnlinkMemoryBlock()

    Removes an allocated memory block from the memory tracking system.

    Parameters:
        pMTI    - A pointer to the DCLMEMTRACKINGINFO structure.
        ppBlock - A pointer to a buffer into which a pointer to the
                  real memory block will be returned (the actual
                  value returned by DclOsMemAlloc().  This buffer
                  will only be modified if the function returns
                  successfully, so it is safe for ppBlock to point
                  to pClient.
        pClient - A pointer to the memory block to unlink, which is
                  a value returned by DclMemAlloc().
        pulSize - A pointer to a variable which receives the size of
                  the block.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS UnlinkMemoryBlock(
    DCLMEMTRACKINGINFO *pMTI,
    void              **ppBlock,
    D_BUFFER           *pClient,
    D_UINT32           *pulSize)
{
    ALLOC_HDR          *pThisAlloc;
    ALLOC_HDR          *pAllocation;
    DCLSTATUS           dclStat;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MEM, 2, 0),
        "UnlinkMemoryBlock() pClient=%P\n", pClient));

    DclAssert(ppBlock);
    DclAssert(pClient);
    DclAssert(pulSize);

    pThisAlloc = (ALLOC_HDR*)(pClient - ALLOC_HDR_SIZE);

    dclStat = ValidateBlock(pMTI, pThisAlloc, pMTI->nErrorMode, FALSE);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    /*  Fill with a known, non-zero value to be able to tell if any memory
        is used after it has been freed.
    */
    DclMemSet(pClient, FREE_FILL_VALUE, pThisAlloc->ulSize);

    *pulSize = pThisAlloc->ulSize;

    /*  Find this allocation in the list and then remove it from the list
    */
    if(pMTI->pListHead == pThisAlloc)
    {
        pMTI->pListHead = pThisAlloc->pNext;
        pAllocation = pMTI->pListHead;
    }
    else
    {
        pAllocation = pMTI->pListHead;

        dclStat = ValidateBlock(pMTI, pAllocation, pMTI->nErrorMode, FALSE);
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;

        while(pAllocation->pNext != pThisAlloc)
        {
            DclAssert(pAllocation->pNext != NULL);

            pAllocation = pAllocation->pNext;

            dclStat = ValidateBlock(pMTI, pAllocation, pMTI->nErrorMode, FALSE);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;
        }
        DclAssert(pAllocation->pNext == pThisAlloc);
        pAllocation->pNext = pAllocation->pNext->pNext;
    }

    /*  Finish validating the remainder of the list
    */
    while(pAllocation != NULL)
    {
        dclStat = ValidateBlock(pMTI, pAllocation, pMTI->nErrorMode, FALSE);
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;

        pAllocation = pAllocation->pNext;
    }

    /*  Update internal count of memory freed
    */
    DclAssert(pMTI->ulBytesCurrentlyAllocated >= pThisAlloc->ulSize);
    DclAssert(pMTI->ulBlocksCurrentlyAllocated);

    pMTI->ulBlocksCurrentlyAllocated--;

    pMTI->ulBytesCurrentlyAllocated -= pThisAlloc->ulSize;

    /*  Return a pointer to the real (originally allocated) block
    */
    *ppBlock = pThisAlloc;

    /*  dclStat will be DCLSTAT_SUCCESS if we get here...
    */
    return dclStat;
}


/*-------------------------------------------------------------------
    Local: BlockVerify()

    Validate the specified memory block.

    Parameters:
        pMTI    - A pointer to the DCLMEMTRACKINGINFO structure.
        pMem    - A pointer to the client memory block to validate.
        fQuiet  - A flag, which if TRUE, indicates that a message
                  should NOT be displayed if an error is encountered.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS BlockVerify(
    DCLMEMTRACKINGINFO *pMTI,
    const D_BUFFER     *pMem,
    D_BOOL              fQuiet)
{
    ALLOC_HDR          *pThisAlloc;
    DCLSTATUS           dclStat;

    DclAssert(pMTI);

    if(DclOsAtomic32Retrieve(&pMTI->ulBusyFlag) != 0)
        return DCLSTAT_BUSY;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MEM, 3, 0),
        "DclMemBlockVerify() pBuffer=%P\n", pMem));

    pThisAlloc = (ALLOC_HDR*)(pMem - ALLOC_HDR_SIZE);
    dclStat = ValidateBlock(pMTI, pThisAlloc, TRUE, fQuiet);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: PoolVerify()

    Iterate through the memory pool examining the list for consistency
    and memory overruns.

    Parameters:
        pMTI    - A pointer to the DCLMEMTRACKINGINFO structure.
        fQuiet  - A flag, which if TRUE, indicates that a message
                  should NOT be displayed if an error is encountered.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS PoolVerify(
    DCLMEMTRACKINGINFO         *pMTI,
    D_BOOL                      fQuiet)
{
    ALLOC_HDR                  *pAllocation;
    D_UINT32                    ulCount = 0;
    DCLSTATUS                   dclStat = DCLSTAT_SUCCESS;

    DclAssert(pMTI);

    if(DclOsAtomic32Retrieve(&pMTI->ulBusyFlag) != 0)
        return DCLSTAT_BUSY;

    /*  verify each allocation in the list
    */
    pAllocation = pMTI->pListHead;
    while(pAllocation != NULL)
    {
        dclStat = ValidateBlock(pMTI, pAllocation, FALSE, fQuiet);
        if(dclStat != DCLSTAT_SUCCESS)
            break;

        ulCount++;

        pAllocation = pAllocation->pNext;
    }

    if(dclStat == DCLSTAT_SUCCESS)
        DclProductionAssert(ulCount == pMTI->ulBlocksCurrentlyAllocated);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: PoolDump()

    Display the memory pool chain.

    Parameters:
        pMTI    - A pointer to the DCLMEMTRACKINGINFO structure.
        nLength - The length to dump from each block, or 0 to
                  disable.  This length will automatically be
                  truncated to the actual block length if necessary.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS PoolDump(
    DCLMEMTRACKINGINFO         *pMTI,
    unsigned                    nLength)
{
    ALLOC_HDR                  *pAlloc;
    D_UINT32                    ulCount = 0;
    D_UINT32                    ulBytes = 0;
    D_UINT32                    ulAlignOverhead = 0;
    D_UINT32                    ulAvgAlign = 0;

    DclAssert(pMTI);

    DclPrintf("DLMEMTRACK Service: Memory Pool Dump\n");

    pAlloc = pMTI->pListHead;
    while(pAlloc != NULL)
    {
        DclPrintf("  Address:%P  Length:%lX  Owner:%s\n",
                  (D_BYTE*)pAlloc+ALLOC_HDR_SIZE, pAlloc->ulSize, pAlloc->pszOwner);

        if(nLength)
        {
            DclHexDump(NULL, HEXDUMP_UINT8, 16,
                       nLength < pAlloc->ulSize ? nLength : pAlloc->ulSize,
                       (D_BYTE*)pAlloc+ALLOC_HDR_SIZE);
        }
        ulCount++;
        ulBytes += pAlloc->ulSize;
        ulAlignOverhead += DCLALIGNPAD(pAlloc->ulSize);

        pAlloc = pAlloc->pNext;
    }

    DclPrintf("Memory pool contains %lX bytes in %lU blocks.\n", ulBytes, ulCount);
    if(ulCount)
        ulAvgAlign = ulAlignOverhead / ulCount;
    DclPrintf("Alignment overhead is %lX bytes (~%u bytes per block).\n", ulAlignOverhead, ulAvgAlign);
    DclPrintf("Memory tracker overhead is %lX bytes (%u bytes per block).\n", ulCount * ALLOC_EXTRAMEMORY, ALLOC_EXTRAMEMORY);
    DclPrintf("Total memory allocated is %lX bytes.\n", ulBytes + ulAlignOverhead + (ulCount * ALLOC_EXTRAMEMORY));
  #if !DCLCONF_PROFILERENABLED
    DclPrintf("Memory owner information is only available if the DCL profiler is included.\n");
  #endif

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: StatsQuery()

    Fill the supplied DCLMEMSTATS structure.

    Parameters:
        pMTI     - A pointer to the DCLMEMTRACKINGINFO structure.
        pDMS     - A pointer to the DCLMEMSTATS structure to fill.
                   On entry, the uStrucLen field must be set to the
                   size of the structure.
        fVerbose - Return verbose statistics, if available.
        fReset   - Reset the reset-able counts to zero.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS StatsQuery(
    DCLMEMTRACKINGINFO     *pMTI,
    DCLMEMSTATS            *pDMS,
    D_BOOL                  fVerbose,
    D_BOOL                  fReset)
{
    DclAssert(pMTI);
    DclAssert(pDMS);

    if(pDMS->uStrucLen != sizeof(*pDMS))
        return DCLSTAT_BADSTRUCLEN;

    DclMemSet(pDMS, 0, sizeof(*pDMS));

    pDMS->uStrucLen                   = sizeof(*pDMS);
    pDMS->ulBytesCurrentlyAllocated   = pMTI->ulBytesCurrentlyAllocated;
    pDMS->ulBlocksCurrentlyAllocated  = pMTI->ulBlocksCurrentlyAllocated;
    pDMS->ulMaxConcurrentBytes        = pMTI->ulMaxConcurrentBytes;
    pDMS->ulMaxConcurrentBlocks       = pMTI->ulMaxConcurrentBlocks;
    pDMS->ulTotalAllocCalls           = pMTI->ulTotalAllocCalls;
    pDMS->ulLargestAllocation         = pMTI->ulLargestAllocation;
    pDMS->uOverhead                   = ALLOC_EXTRAMEMORY;

    if(fReset)
    {
        DCLPRINTF(1, ("DLMEMTRACK Service: Resetting memory statistics\n"));
        
        pMTI->ulMaxConcurrentBytes = 0;
        pMTI->ulMaxConcurrentBlocks = 0;
        pMTI->ulTotalAllocCalls = 0;
        pMTI->ulLargestAllocation = 0;
    }

    return DCLSTAT_SUCCESS;

}


/*-------------------------------------------------------------------
    Local: ValidateBlock()

    Validate an individual memory block.

    Note that in this function, the printf and assert statements
    are not DEBUG only, as we want to be able to enable the
    memory tracking code even in release mode, if we so choose.

    Parameters:
        pMTI    - A pointer to the DCLMEMTRACKINGINFO structure.
        pAlloc  - A pointer memory block's ALLOC_HDR structure.
        fAssert - A flag to indicate whether an assert should be
                  issued if a problem is found.
        fQuiet  - A flag, which if TRUE, indicates that a message
                  should NOT be displayed if an error is encountered.

    Return Value:
        Returns a DCLSTATUS value indicating if the block is valid
        or not.
-------------------------------------------------------------------*/
static DCLSTATUS ValidateBlock(
    DCLMEMTRACKINGINFO *pMTI,
    const ALLOC_HDR    *pAlloc,
    D_BOOL              fAssert,
    D_BOOL              fQuiet)
{
    D_UINT8             uIndex;
    D_UINT8             uSize;
    const D_BUFFER     *pExpect;
    const D_BUFFER     *pClient;
    DCLSTATUS           dclStat;

    /*  Prevent being recursively entered!  The mutex we are currently
        under will not necessarily prevent that.
    */
    DclOsAtomic32Increment(&pMTI->ulBusyFlag);

    pClient = (D_BUFFER*)pAlloc + ALLOC_HDR_SIZE;

    /*  Validate the allocation structure.
    */
    pExpect = (D_BUFFER*)pAlloc + sizeof(ALLOC_HDR);
    if(pAlloc->pHeadSentinel != pExpect)
    {
        if(!fQuiet)
        {
            DclPrintf("DLMEMTRACK Service: Pool corruption: pAlloc=%P pClient=%P: Bad head sentinel: expected %P, got %P\n",
                pAlloc, pClient, pExpect, pAlloc->pHeadSentinel);
        }

        if(fAssert)
            DclProductionError();

        dclStat = DCLSTAT_MEMPOOLCORRUPTED;
        goto ValidateCleanup;
    }

    pExpect = (D_BUFFER*)pAlloc + ALLOC_HDR_SIZE + pAlloc->ulSize;
    if(pAlloc->pTailSentinel != pExpect)
    {
        if(!fQuiet)
        {
            DclPrintf("DLMEMTRACK Service: Pool corruption: pAlloc=%P pClient=%P: Bad tail sentinel: expected %P, got %P\n",
                pAlloc, pClient, pExpect, pAlloc->pTailSentinel);
        }

        if(fAssert)
            DclProductionError();

        dclStat = DCLSTAT_MEMPOOLCORRUPTED;
        goto ValidateCleanup;
    }

    /*  Validate the header sentinel.
    */
    uSize = (D_UINT8)ALLOC_HEAD_SENTINEL_SIZE;
    for(uIndex = 0; uIndex < uSize; uIndex++)
    {
        if(pAlloc->pHeadSentinel[uIndex] != ALLOC_HEAD_SENTINEL_VAL)
        {
            if(!fQuiet)
            {
                DclPrintf("DLMEMTRACK Service: Underrun detected: pAlloc=%P pClient=%P\n",
                    pAlloc, pClient);
            }

            if(fAssert)
                DclProductionError();

            dclStat = DCLSTAT_MEMUNDERRUN;
            goto ValidateCleanup;
        }
    }

    /*  Validate the tail sentinel.
    */
    uSize = (D_UINT8)ALLOC_TAIL_SENTINEL_SIZE;
    for(uIndex = 0; uIndex < uSize; uIndex++)
    {
        if(pAlloc->pTailSentinel[uIndex] != ALLOC_TAIL_SENTINEL_VAL)
        {
            if(!fQuiet)
            {
                DclPrintf("DLMEMTRACK Service: Overrun detected: pAlloc=%P pClient=%P Owner=%s Len=%lU\n",
                    pAlloc, pClient, pAlloc->pszOwner, pAlloc->ulSize);
            }

            if(fAssert)
                DclProductionError();

            dclStat = DCLSTAT_MEMOVERRUN;
            goto ValidateCleanup;
        }
    }

    dclStat = DCLSTAT_SUCCESS;

  ValidateCleanup:

    DclOsAtomic32Decrement(&pMTI->ulBusyFlag);

    return dclStat;
}



#endif  /* DCLCONF_MEMORYTRACKING */

