/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module contains a simple heap implementation that may be used by
    OS environments that do not support dynamic memory allocation.

    This is a primitive implementation which may be suitable for systems
    with no memory management available.  As such, it is susceptible to
    fragmentation.  It is recommended that the host provide plenty of extra
    memory to eliminate the effects of fragmenting the memory pool.

    These contraints are usually not a problem in the environments where this
    code is typically used, which is inside boot loaders, or other "no-OS"
    environments.

    WARNING!  This code is *NOT* thread-safe and should only be used in
              scenarios where it can be assured that multiple threads will
              not be allocating and freeing memory.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlheap.c $
    Revision 1.9  2011/08/22 23:54:11Z  daniel.lewis
    Updated to use soft tabs -- no functional changes.
    Revision 1.8  2009/06/27 23:26:13Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.7  2009/04/09 22:08:05Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.6  2009/02/08 00:32:02Z  garyp
    Merged from the v4.0 branch.  Complete rewrite.  No longer requires that
    memory be released in reverse order of allocation.
    Revision 1.5  2007/12/18 04:03:34Z  brandont
    Updated function headers.
    Revision 1.4  2007/11/24 18:19:41Z  Garyp
    Updated to use DCLISALIGNED().  Removed the use of the PMEMHDR type.
    Revision 1.3  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.2  2006/05/18 01:20:06Z  Garyp
    Documentation fixes.
    Revision 1.1  2005/10/02 03:57:04Z  Pauli
    Initial revision
    Revision 1.2  2005/08/03 19:17:18Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/06 02:59:42Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlheap.h>

#define DCL_ALIGN_MASK  (DCL_ALIGNSIZE-1)
#define BLOCK_FREE      (0)
#define BLOCK_ALLOC     (1)
#define END_SENTINEL    D_UINT32_MAX

typedef struct
{
    D_BUFFER   *pPoolBase;      /* original pool base, never changes */
    D_UINT32    ulPoolSize;     /* original pool size, never changes */
} DCLHEAPINFO;

static DCLHEAPINFO  mi;

typedef struct
{
    unsigned    nStatus;
    D_UINT32    ulBlockSize;
} DCLHEAPHDR;

#if D_DEBUG && 0
static void DclHeapDumpHeaders(unsigned fVerbose);
#endif


/*-------------------------------------------------------------------
    ToDo: Could/should consider an allocation scheme where small
          blocks are allocated from one end of the pool and large
          blocks are allocated from the other -- to reduce
          fragmentation.
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
    Public: DclHeapInit()

    Initialize the memory heap subsystem.  It must be called early
    in the driver initialization process, before any other functions
    are invoked that may attempt to allocate memory.

    Parameters:
        pMemBase  - The address of the base of the memory pool.  This
                    must be aligned on an DCL_ALIGNSIZE boundary.
        ulMemSize - The size of the memory pool.  This value must be
                    evenly divided by DCL_ALIGNSIZE.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclHeapInit(
    void       *pMemBase,
    D_UINT32    ulMemSize)
{
    DCLHEAPHDR *pHead;

    DCLPRINTF(1, ("DclHeapInit() base=%P size=%lX\n", pMemBase, ulMemSize));

    DclAssert(pMemBase);
    DclAssert(DCLISALIGNED((D_UINTPTR)pMemBase, DCL_ALIGNSIZE));
    DclAssert(ulMemSize);
    DclAssert(ulMemSize % DCL_ALIGNSIZE == 0);

    DclMemSet(&mi, 0, sizeof(mi));

    mi.pPoolBase    = (D_BUFFER*)pMemBase;
    mi.ulPoolSize   = ulMemSize;

    /*  Make the first header
    */
    pHead = (DCLHEAPHDR*)mi.pPoolBase;
    pHead->nStatus = BLOCK_FREE;
    pHead->ulBlockSize = ulMemSize - (2 * sizeof(DCLHEAPHDR));

    /*  Now make the terminating header
    */
    pHead = (DCLHEAPHDR*)(mi.pPoolBase + (mi.ulPoolSize - sizeof(DCLHEAPHDR)));
    pHead->nStatus = BLOCK_ALLOC;
    pHead->ulBlockSize = END_SENTINEL;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclHeapAlloc()

    Allocate a block of memory.  All buffers returned by this
    function are required to be aligned on DCL_ALIGNSIZE
    boundaries.

    Parameters:
        ulSize - number of bytes to allocate

    Return Value:
        Returns a pointer to allocated memory, or NULL if failure.
-------------------------------------------------------------------*/
void * DclHeapAlloc(
    D_UINT32    ulSize)
{
    D_BUFFER   *pMem;

    DclAssert(ulSize);

    /*  ulSize must be aligned
    */
    if(ulSize & DCL_ALIGN_MASK)
        ulSize += DCL_ALIGN_MASK - (ulSize & DCL_ALIGN_MASK) + 1;

    pMem = mi.pPoolBase;

/*  DclHeapDumpHeaders(FALSE); */

    while(((D_UINTPTR)pMem - (D_UINTPTR)mi.pPoolBase) < (mi.ulPoolSize - sizeof(DCLHEAPHDR)))
    {
        DCLHEAPHDR *pHead = (DCLHEAPHDR*)pMem;

        /*  Found a spot to allocate?
        */
        if(pHead->nStatus == BLOCK_FREE && pHead->ulBlockSize >= ulSize)
        {
            DCLHEAPHDR  OldHead = *pHead;
            DCLHEAPHDR *pNext = (DCLHEAPHDR*)(pMem + OldHead.ulBlockSize + sizeof(DCLHEAPHDR));

            /*  Change the old header to the desired dimensions
            */
            pHead->nStatus = BLOCK_ALLOC;
            pHead->ulBlockSize = ulSize;

            /*  We need to add the left overs to this block and back up
                this header
            */
            if(pNext->nStatus == BLOCK_FREE)
            {
                DCLHEAPHDR *pNew = (DCLHEAPHDR*)(pMem + pHead->ulBlockSize + sizeof(DCLHEAPHDR));

                /*  It seems like this case should never happen, since we
                    have logic in the free function to ensure that adjacent
                    free blocks are merged...
                */

                pNew->nStatus = BLOCK_FREE;
                pNew->ulBlockSize = pNext->ulBlockSize + (OldHead.ulBlockSize - ulSize);
            }
            else if( pNext->nStatus == BLOCK_ALLOC  && (OldHead.ulBlockSize - ulSize) < (2 * sizeof(DCLHEAPHDR)))
            {
                /*  Size of blocks is too similar, keep the block size the
                    same as the old size.
                */
                pHead->ulBlockSize = OldHead.ulBlockSize;
            }
            else if(pNext->nStatus == BLOCK_ALLOC)
            {
                /*  We must allocate a new header in the remaining space
                */
                DCLHEAPHDR *pNew = (DCLHEAPHDR*)(pMem + pHead->ulBlockSize + sizeof(DCLHEAPHDR));

                pNew->nStatus = BLOCK_FREE;
                pNew->ulBlockSize = (OldHead.ulBlockSize - ulSize) - sizeof(DCLHEAPHDR);
            }
            else
            {
                DCLPRINTF(1, ("DclHeapAlloc(A) Corrupted heap, pHead=%P\n", pHead));
                return NULL;
            }

            return pMem + sizeof(DCLHEAPHDR);

        }
        else if(pHead->nStatus != BLOCK_ALLOC && pHead->nStatus != BLOCK_FREE)
        {
            DCLPRINTF(1, ("DclHeapAlloc(B) Corrupted heap, pHead=%P\n", pHead));
            return NULL;
        }

        pMem += pHead->ulBlockSize + sizeof(DCLHEAPHDR);
    }

    DclPrintf("DclHeapAlloc() Heap is full!\n");

    return NULL;
}


/*-------------------------------------------------------------------
    Public: DclHeapFree()

    Release a block of memory that was allocated with DclHeapAlloc().

    Parameters:
        pMem - A pointer to the allocated memory block.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclHeapFree(
    void       *pBuffer)
{
    D_BUFFER   *pMem = pBuffer;
    DCLHEAPHDR *pHead;

/*  DclHeapDumpHeaders(TRUE); */

    DclAssert(pMem);

    pMem -= sizeof(DCLHEAPHDR);

    pHead = (DCLHEAPHDR*)pMem;

    if(pHead->nStatus != BLOCK_ALLOC && pHead->nStatus != BLOCK_FREE)
    {
        DCLPRINTF(1, ("DclHeapFree() Corrupted heap, pHead=%P\n", pHead));
        DclError();
        return DCLSTAT_MEMPOOLCORRUPTED;
    }

    if(pHead->nStatus == BLOCK_FREE)
    {
        DCLPRINTF(1, ("DclHeapFree() Double free! pHead=%P\n", pHead));
        DclError();
        return DCLSTAT_MEMFREEFAILED;
    }

    pHead->nStatus = BLOCK_FREE;

    /*  This is not efficient.  We will now go through all the
        headers a merge things up.
    */

    pMem = mi.pPoolBase;

    while(((D_UINTPTR)pMem - (D_UINTPTR)mi.pPoolBase) < (mi.ulPoolSize - sizeof(DCLHEAPHDR)))
    {
        DCLHEAPHDR *pHead = (DCLHEAPHDR*)pMem;
        DCLHEAPHDR *pNext = (DCLHEAPHDR*)(pMem + pHead->ulBlockSize + sizeof(DCLHEAPHDR));

        if(pHead->nStatus != BLOCK_ALLOC && pHead->nStatus != BLOCK_FREE)
        {
            DCLPRINTF(1, ("DclHeapFree() Corrupted heap: bad linked list head, pHead=%P\n", pHead));
            DclError();
            return DCLSTAT_MEMPOOLCORRUPTED;
        }

        if(pNext->nStatus != BLOCK_ALLOC && pNext->nStatus != BLOCK_FREE)
        {
            DCLPRINTF(1, ("DclHeapFree() Corrupted heap: bad linked list next, pHead=%P\n", pHead));
            DclError();
            return DCLSTAT_MEMPOOLCORRUPTED;
        }

        if(pNext->ulBlockSize == END_SENTINEL)
            break;

        if(pHead->nStatus == BLOCK_FREE  && pNext->nStatus == BLOCK_FREE)
        {
            /*  Combine the two blocks
            */
            pHead->ulBlockSize +=  pNext->ulBlockSize + sizeof(DCLHEAPHDR);

            /*  (Don't increment pMem in this case)
            */
        }
        else
        {
            pMem += pHead->ulBlockSize + sizeof(DCLHEAPHDR);
        }
    }

    return DCLSTAT_SUCCESS;
}


#if D_DEBUG && 0

/*-------------------------------------------------------------------
    Local: DclHeapDumpHeaders()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void DclHeapDumpHeaders(
    unsigned    fVerbose)
{
    D_BUFFER   *pMem;
    unsigned    nAllocated = 0;
    unsigned    nFree = 0;
    D_UINT32    ulBytesFree = 0;
    D_UINT32    ulBytesAllocated = 0;

    pMem = mi.pPoolBase;

    while(TRUE)
    {
        DCLHEAPHDR* pHead;

        pHead = (DCLHEAPHDR*)pMem;

        if(fVerbose)
            DclPrintf("Heap: Address=%P Status=%u Size=%lX\n", pMem, pHead->nStatus, pHead->ulBlockSize);

        if(pHead->ulBlockSize == END_SENTINEL)
            break;

        if(pHead->nStatus == BLOCK_ALLOC)
        {
            nAllocated++;
            ulBytesAllocated += pHead->ulBlockSize;
        }
        else if(pHead->nStatus == BLOCK_FREE)
        {
            nFree++;
            ulBytesFree += pHead->ulBlockSize;
        }
        else
        {
            DclPrintf("DclHeapDumpHeaders() Corrupted heap, pHead=%P\n", pHead);
            DclError();
        }

        pMem += pHead->ulBlockSize + sizeof(DCLHEAPHDR);
    }

    DclPrintf("Heap Summary: BlocksAllocated=%3u BlocksFree=%3u BytesAllocated=%6lU BytesFree=%6lU\n",
        nAllocated, nFree, ulBytesAllocated, ulBytesFree);

    return;
}

#endif


