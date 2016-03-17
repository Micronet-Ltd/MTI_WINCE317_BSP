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

    This module contains general memory management routines built upon the
    primitives provided in the OS Services Layer.

    Note that the memory tracking code uses production messages and asserts
    because we want it to be function even if running in release mode.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlmem.c $
    Revision 1.32  2009/09/28 17:39:17Z  keithg
    Removed asserts on handled error conditions.
    Revision 1.31  2009/09/09 00:03:49Z  billr
    Attempting to allocate zero length is not permitted.
    Revision 1.30  2009/06/28 00:59:17Z  garyp
    Moved the meat of the "MemTracking" logic into dlmemtracking.c.
    Revision 1.29  2009/04/09 22:05:37Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.28  2009/02/23 23:47:43Z  keithg
    Added debug condition around debug output conditioned in last checkin.
    Revision 1.27  2009/02/23 22:13:07Z  keithg
    Made debug output dependent upon DCLCONF_OUTPUT_ENABLED.
    Revision 1.26  2009/02/18 07:50:23Z  garyp
    Voided some unused parameters.
    Revision 1.25  2008/05/27 16:34:33Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.24  2008/01/13 01:15:16Z  Garyp
    Modified DclMemAllocZero() to internally pad the specified length if
    necessary, so that DclMemSetAligned() may be used.
    Revision 1.23  2007/12/18 04:51:50Z  brandont
    Updated function headers.
    Revision 1.22  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.21  2007/10/15 17:28:29Z  Garyp
    Modified DclMemPoolVerify() to take an fQuiet parameter.
    Revision 1.20  2007/10/07 05:30:19Z  pauli
    Replaced ALIGNPAD with DCLALIGNPAD which is defined even when memory
    tracking is disabled.
    Revision 1.19  2007/10/06 03:54:00Z  pauli
    Reworked the memory tracking mechanism to no longer depend upon
    DCLALIGNEDSTRUCT to make the size of the structure aligned.
    Additionally, all of the padding added to ensure alignment is
    checked for memory underruns and overruns.
    Revision 1.18  2007/05/17 18:44:34Z  garyp
    Removed a deprecated and unused function.
    Revision 1.17  2007/04/12 20:23:11Z  billr
    Resolve bug 1016.
    Revision 1.16  2007/01/17 01:29:12Z  Garyp
    Added the DclMemPoolDump() function.
    Revision 1.15  2006/12/21 19:41:54Z  Pauli
    Changed a global declaration of ALIGNED_ALLOC_HDR to a type definition as
    it is only used with sizeof.  Corrected an error message.
    Revision 1.14  2006/10/07 03:32:37Z  Garyp
    Updated to eliminate warnings generated by the RealView tools.
    Revision 1.13  2006/10/03 20:30:53Z  Garyp
    Updated to track the maximum number of concurrent blocks allocated.
    Revision 1.12  2006/08/21 23:19:41Z  Garyp
    Fixed to handle being recursively entered in the event that the profiler
    is being used.
    Revision 1.11  2006/08/07 23:44:28Z  Garyp
    Updated to use DCLALIGNEDSTRUCT() to ensure structure alignment.
    Revision 1.10  2006/07/23 02:44:34Z  Garyp
    Enabled code to display the memory owner if available.  Updated debug
    messages to make it easier to track memory leaks.
    Revision 1.9  2006/05/28 21:16:13Z  Garyp
    Fixed so that if memory tracking is enabled, resources are still released
    in reverse order.
    Revision 1.8  2006/04/26 20:31:13Z  billr
    Support for pointers wider than 32 bits.
    Revision 1.7  2006/03/09 02:12:35Z  Garyp
    Moved an assert inside mutex protected code.  Added some commented
    out code to track memory owners.
    Revision 1.6  2006/02/26 20:27:31Z  Garyp
    Updated some debug levels.
    Revision 1.5  2006/02/23 02:55:08Z  Garyp
    Added a mutex to protect the memory tracking structures.  Modified to
    automatically increase the allocation size if it is not evenly divisible by
    DCL_ALIGNSIZE.
    Revision 1.4  2006/02/22 23:55:59Z  Pauli
    Added function to allocate a buffer and zero initialize it.
    Revision 1.3  2006/01/02 02:21:57Z  Garyp
    Modified the stats interface to query and return the stats in a structure,
    rather than display them.
    Revision 1.2  2005/12/18 22:53:30Z  garyp
    Tweaked some debug messages.
    Revision 1.1  2005/12/06 01:15:40Z  Pauli
    Initial revision
    Revision 1.5  2005/12/06 01:15:39Z  Pauli
    Updated comments
    Revision 1.4  2005/12/02 23:02:40Z  Pauli
    Merge with 2.0 product line, build 173.
    Revision 1.3  2005/11/13 02:33:28Z  Garyp
    Updated the statistics interface functions to quietly return FALSE if the
    particular stat interface is disabled.
    Revision 1.2  2005/11/06 03:31:36Z  Garyp
    Cleaned up the memory usage display.
    Revision 1.1  2005/10/02 04:57:04Z  Garyp
    Initial revision
    Revision 1.5  2005/09/15 23:45:33Z  garyp
    Added DclMemStatistics().
    Revision 1.4  2005/09/13 07:57:14Z  garyp
    Straightened up some debugging code.
    Revision 1.3  2005/09/12 05:09:18Z  garyp
    Added debugging code.
    Revision 1.2  2005/08/03 19:17:46Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/06 03:47:28Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>
#include <dlstats.h>

#if DCLCONF_MEMORYTRACKING
#include "dlmemtracking.h"
#endif


static void *    AllocOsMem(D_UINT32 ulSize);
static DCLSTATUS FreeOsMem(void *pMem);


/*-------------------------------------------------------------------
    Public: DclMemAlloc()

    Allocate a block of memory.  All buffers returned by this
    function are required to be aligned on DCL_ALIGNSIZE
    boundaries.

    All sizes <should> be on DCL_ALIGNSIZE boundaries.  If not,
    a debug warning will be displayed, and the buffer size will
    be automatically increased as necessary.

    Parameters:
        ulSize - The number of bytes to allocate.  Must be nonzero.

    Return Value:
        Returns a pointer to allocated memory, or NULL if failure.
-------------------------------------------------------------------*/
void * DclMemAlloc(
    D_UINT32        ulSize)
{
    void           *pMem;
    D_UINT32        ulAlignPadLen;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MEM, 2, 0),
        "DclMemAlloc() Len=%lU\n", ulSize));

    if (ulSize == 0)
    {
        return NULL;
    }

    ulAlignPadLen = DCLALIGNPAD(ulSize);

  #if D_DEBUG > 1
    if(ulAlignPadLen)
        DCLPRINTF(2, ("DclMemAlloc() allocating a buffer of a non-aligned length (%lU), adjusting...\n", ulSize));
  #endif

    if(ulSize <= D_UINT32_MAX - ulAlignPadLen)
    {
        /*  Allocate the memory from the OS Memory Service.
        */
        DclAssert(DCLISALIGNED(ulSize + ulAlignPadLen, DCL_ALIGNSIZE));

        pMem = AllocOsMem(ulSize + ulAlignPadLen);

        /*  Pointers returned from the OS Services allocator always have
            to be properly aligned.
        */
        if(!DCLISALIGNED((D_UINTPTR)pMem, DCL_ALIGNSIZE))
        {
            DCLPRINTF(1, ("DclOsMemAlloc() returned an improperly aligned address: %P\n", pMem));
            DclError();

            FreeOsMem(pMem);
            pMem = NULL;
        }
    }
    else
    {
        DCLPRINTF(1, ("DclMemAlloc() trying to allocate an impossibly large buffer %lX\n", ulSize));
        pMem = NULL;
    }

    if(!pMem)
    {
         DCLPRINTF(1, ("DclMemAlloc(%lU) failed\n", ulSize));
         return pMem;
    }

  #if DCLCONF_MEMORYTRACKING

    pMem = DclMemTrackAllocLink(pMem, ulSize);

  #else

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MEM, 1, 0),
        "DclMemAlloc(%lU) returning %P\n", ulSize, pMem));
  #endif

  #if DCLCONF_MEMORYVALIDATION
    if(pMem)
        DclMemValBufferAdd(NULL, pMem, ulSize, DCL_MVFLAG_TYPEDLMEMMGR);
  #endif

    return pMem;
}


/*-------------------------------------------------------------------
    Public: DclMemFree()

    Release a block of memory that was allocated with DclMemAlloc().

    Parameters:
        pMem - A pointer to the allocated memory.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclMemFree(
    void       *pMem)
{
    DCLSTATUS   dclStat;

  #if DCLCONF_MEMORYVALIDATION
    dclStat = DclMemValBufferRemove(NULL, pMem);
    if(dclStat != DCLSTAT_SUCCESS && dclStat != DCLSTAT_SERVICE_NOTREGISTERED)
    {
        DclPrintf("DclMemValBufferRemove() pMem=%P failed with status %lX\n", pMem, dclStat);
/*        DclProductionError();
        return dclStat; */
    }
  #endif

  #if DCLCONF_MEMORYTRACKING

    dclStat = DclMemTrackFreeUnlink(&pMem);
    if(dclStat != DCLSTAT_SUCCESS && dclStat != DCLSTAT_SERVICE_NOTREGISTERED)
        return dclStat;

  #else

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MEM, 1, 0), "DclMemFree(%P)\n", pMem));

  #endif

    if(!pMem)
        return DCLSTAT_MEM_NULLPOINTER;

    dclStat = FreeOsMem(pMem);

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclMemAllocZero()

    Allocate a block of memory and initialize it with zeros.
    All buffers returned by this function are required to be
    aligned on DCL_ALIGNSIZE boundaries.

    Parameters:
        ulSize - The number of bytes to allocate.  Must be nonzero.

    Return Value:
        Returns a pointer to allocated memory, or NULL if failure.
-------------------------------------------------------------------*/
void * DclMemAllocZero(
    D_UINT32    ulSize)
{
    void       *pMem;
    D_UINT32    ulAlignPadLen = DCLALIGNPAD(ulSize);

    if (ulSize == 0)
    {
        return NULL;
    }

  #if D_DEBUG > 1
    if(ulAlignPadLen)
    {
        DCLPRINTF(2, ("DclMemAllocZero() allocating a buffer of a non-aligned length (%lU), adjusting...\n", ulSize));
    }
  #endif

    /*  In order to use DclMemSetAligned(), the length as well as the
        pointer must be properly aligned.  Adjust it the length here
        if necessary.
    */
    ulSize += ulAlignPadLen;

    pMem = DclMemAlloc(ulSize);
    if(pMem)
        DclMemSetAligned(pMem, 0, ulSize);

    return pMem;
}


/*-------------------------------------------------------------------
    Local: AllocOsMem()

    Parameters:
        ulSize - number of bytes to allocate

    Return Value:
        Returns a pointer to allocated memory, or NULL if failure.
-------------------------------------------------------------------*/
static void * AllocOsMem(
    D_UINT32    ulSize)
{
    void       *pMem;

  #if DCLCONF_MEMORYTRACKING
    pMem = DclMemTrackAlloc(ulSize);

  #else

    pMem = DclOsMemAlloc(ulSize);

  #if DCLCONF_MEMORYVALIDATION
    if(pMem)
        DclMemValBufferAdd(NULL, pMem, ulSize, DCL_MVFLAG_TYPEOSMEMPOOL);
  #endif

  #endif

    return pMem;
}


/*-------------------------------------------------------------------
    Local: FreeOsMem()

    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the results
-------------------------------------------------------------------*/
static DCLSTATUS FreeOsMem(
    void       *pMem)
{
    DCLSTATUS   dclStat;

  #if DCLCONF_MEMORYTRACKING
    dclStat = DclMemTrackFree(pMem);
  #else

  #if DCLCONF_MEMORYVALIDATION
    dclStat = DclMemValBufferRemove(NULL, pMem);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;
  #endif

    dclStat = DclOsMemFree(pMem);
  #endif

    return dclStat;
}

