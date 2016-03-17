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

    This module contains code for testing the memory management subsystem.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltmemmgr.c $
    Revision 1.10  2010/04/28 23:31:29Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.9  2010/01/23 16:47:08Z  garyp
    Initialized a local to satisfy an overly picky compiler.
    Revision 1.8  2009/12/11 03:50:17Z  garyp
    Moved the "max allocatable memory" tests into "stress" category of 
    tests, so they only run when the /MEMSTRESS option is used (Bug 2938).
    Added more memory tests.
    Revision 1.7  2009/11/16 02:28:28Z  garyp
    Added profiler instrumentation to allow verification that the memory manager
    can accurately track memory owners via the profiler.  Updated to dynamically
    disable the profiler's memory pool validation while running the test, to 
    avoid a failure when testing error conditions.
    Revision 1.6  2009/11/11 03:54:14Z  garyp
    Updated to build cleanly with GNU compilers when memory tracking
    is turned off.
    Revision 1.5  2009/11/03 00:09:27Z  billr
    Resolve bug 2882: DCLTest seg faults on embedded linux platform.
    Revision 1.4  2009/10/03 00:42:51Z  garyp
    Reduced MAX_MILLISECONDS from 5000 to 2000.
    Revision 1.3  2009/07/01 19:34:38Z  garyp
    Added local initializations to satisfy picky compilers.
    Revision 1.2  2009/06/22 19:17:32Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.3  2009/01/10 02:50:38Z  garyp
    Updated to use a Service IOCTL to talk to the MemTracking system.
    Revision 1.1.1.2  2008/11/05 03:49:30Z  garyp
    Fixed the memory protection test to set the error mode to "fail" while
    running the test.
    Revision 1.1  2008/11/05 01:57:20Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include <dlperflog.h>
#include <dlapiprv.h>
#include <dlservice.h>
#include "dltests.h"

#define MAX_SMALL_ALLOCATION_SIZE   (64)
#define MAX_LARGE_ALLOCATION_SIZE   (2 * 1024UL * 1024UL * 1024UL)  /* 2GB */
#define START_SCALE                 (1024)  /* 1024 or 1KB minimum */
#define OVERHEAD_DIVISOR            (32 * 1024)
#define MAX_MILLISECONDS            (10000)
#define DISPLAY_INTERVAL            (500)
#define VMM_DIVISOR                 (10)
#define VMM_BLOCK_MULTIPLIER        (3)

static DCLSTATUS TestMemTracking(D_UINT32 ulAllocSize);
static DCLSTATUS TestMemProtection(D_BOOL fMemTrackingOn, D_UINT32 ulAllocSize);
static DCLSTATUS TestMemStress(unsigned nAlignBytes);


/*-------------------------------------------------------------------
    Protected: DclTestMemoryManagement()

    This function invokes the unit tests for the general memory
    management system.

    Parameters:
        fStress - A flag indicating whether the memory management
                  stress tests should be run.  These tests may
                  take a long time to complete.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclTestMemoryManagement(
    D_BOOL          fStress)
{
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS; /* (Unnecessary init for a dump compiler) */
    unsigned        nAlignBytes;
    D_UINT32        ulSize;
    D_UINT32        ulIndex;
    D_BUFFER       *pMem;
    unsigned        nOldMemTrackingErrorMode;
    unsigned        nOldMemValErrorMode;

    DclProfilerEnter(__FUNCTION__, 0, 0);

    DclPrintf("Testing Memory Management...\n");

    /*  Set the error mode so that we simply fail a call, rather than
        asserting on errors.  Note that setting this mode is of primary
        benefit when DCLCONF_MEMORYTRACKING is enabled.  This mode has
        no control over what the OS does when memory management errors
        occur.
    */
    nOldMemTrackingErrorMode = DclMemTrackErrorMode(0, DCLMEM_ERROR_FAIL);
    nOldMemValErrorMode = DclMemValErrorMode(0, DCLMEM_ERROR_FAIL);

    DclPrintf("  Determining the native OS memory alignment boundary...\n");
    {
        D_BUFFER   *apBuff[32];
        unsigned    nn;
        D_UINTPTR   nPtrORed = 0;

        for(nn=0; nn<DCLDIMENSIONOF(apBuff); nn++)
        {
            apBuff[nn] = DclOsMemAlloc(DCL_ALIGNSIZE+nn);
            if(!apBuff[nn])
            {
                DclPrintf("OS Services allocation of %u bytes failed, iteration %u\n", DCL_ALIGNSIZE+nn, nn);
                break;
            }

            nPtrORed |= (D_UINTPTR)apBuff[nn];
        }

        /*  If anything was allocated, free it all
        */
        if(nn)
        {
            unsigned yy;

            for(yy=nn; yy>0; yy--)
            {
                dclStat = DclOsMemFree(apBuff[yy-1]);
                if(dclStat != DCLSTAT_SUCCESS)
                {
                    DclPrintf("OS Services free of pMem=%P failed with error code %lX\n", apBuff[yy-1], dclStat);
                    goto Cleanup;
                }
            }
        }

        /*  If for some reason everything did not allocate, exit and get
            out.  We already displayed an error message and released the
            memory.
        */
        if(nn != DCLDIMENSIONOF(apBuff))
        {
            dclStat = DCLSTAT_MEMALLOCFAILED;
            goto Cleanup;
        }

        nAlignBytes = 1;
        while(!(nPtrORed & 1))
        {
            nPtrORed >>= 1;
            nAlignBytes <<= 1;
        }

        DclPrintf("    OS Services memory appears to be %u byte aligned\n", nAlignBytes);
        if(nAlignBytes < DCL_ALIGNSIZE)
        {
            DclPrintf("   %u is less than %u -- test failed\n", nAlignBytes, DCL_ALIGNSIZE);
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }
    }

    DclPrintf("  Determining the DLMEM subsystem alignment boundary...\n");
    {
        D_BUFFER   *apBuff[32];
        unsigned    nn;
        D_UINTPTR   nPtrORed = 0;

        for(nn=0; nn<DCLDIMENSIONOF(apBuff); nn++)
        {
            apBuff[nn] = DclMemAlloc(DCL_ALIGNSIZE+nn);
            if(!apBuff[nn])
            {
                DclPrintf("Memory allocation of %u bytes failed, iteration %u\n", DCL_ALIGNSIZE+nn, nn);
                break;
            }

            nPtrORed |= (D_UINTPTR)apBuff[nn];
        }

        /*  If anything was allocated, free it all
        */
        if(nn)
        {
            unsigned yy;

            for(yy=nn; yy>0; yy--)
            {
                dclStat = DclMemFree(apBuff[yy-1]);
                if(dclStat != DCLSTAT_SUCCESS)
                {
                    DclPrintf("Memory free of pMem=%P failed with error code %lX\n", apBuff[yy-1], dclStat);
                    goto Cleanup;
                }
            }
        }

        /*  If for some reason everything did not allocate, exit and get
            out.  We already displayed an error message and released the
            memory.
        */
        if(nn != DCLDIMENSIONOF(apBuff))
        {
            dclStat = DCLSTAT_MEMALLOCFAILED;
            goto Cleanup;
        }

        nAlignBytes = 1;
        while(!(nPtrORed & 1))
        {
            nPtrORed >>= 1;
            nAlignBytes <<= 1;
        }

        DclPrintf("    DLMEM subsystem memory appears to be %u byte aligned\n", nAlignBytes);
        if(nAlignBytes < DCL_ALIGNSIZE)
        {
            DclPrintf("   %u is less than %u -- test failed\n", nAlignBytes, DCL_ALIGNSIZE);
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }
    }

    DclPrintf("  Testing small allocations...\n");
    {
        for(ulSize=1; ulSize<=MAX_SMALL_ALLOCATION_SIZE; ulSize++)
        {
            pMem = DclMemAlloc(ulSize);
            if(!pMem)
            {
                DclPrintf("Basic allocation of %lU bytes failed\n", ulSize);
                dclStat = DCLSTAT_MEMALLOCFAILED;
                goto Cleanup;
            }

            /*  Ensure that each allocated byte is accessible
            */
            for(ulIndex=0; ulIndex<ulSize; ulIndex++)
                pMem[0] += pMem[ulIndex];

            dclStat = DclMemFree(pMem);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Free of pMem=%P size=%lU failed with error code %lX\n", pMem, ulSize, dclStat);
                goto Cleanup;
            }
        }
    }

    DclPrintf("  Testing zeroed small allocations...\n");
    {
        for(ulSize=1; ulSize<=MAX_SMALL_ALLOCATION_SIZE; ulSize++)
        {
            pMem = DclMemAllocZero(ulSize);
            if(!pMem)
            {
                DclPrintf("Basic allocation of %lU bytes failed\n", ulSize);
                dclStat = DCLSTAT_MEMALLOCFAILED;
                goto Cleanup;
            }

            /*  Ensure that each allocated byte is accessible
            */
            for(ulIndex=0; ulIndex<ulSize; ulIndex++)
            {
                if(pMem[ulIndex] != 0)
                {
                    DclPrintf("For buffer at %P, offset %lU in a %lU zeroed allocation is non-zero\n", pMem, ulIndex, ulSize);
                    dclStat = DCLSTAT_MEM_INITFAILURE;
                    goto Cleanup;
                }
            }

            dclStat = DclMemFree(pMem);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Free of pMem=%P size=%lU failed with error code %lX\n", pMem, ulSize, dclStat);
                goto Cleanup;
            }
        }
    }

    if(fStress)
        dclStat = TestMemStress(nAlignBytes);

  Cleanup:

    /* Restore the original error modes
    */
    if(nOldMemTrackingErrorMode != DCLMEM_ERROR_INVALID)
        (void)DclMemTrackErrorMode(0, nOldMemTrackingErrorMode);

    if(nOldMemValErrorMode != DCLMEM_ERROR_INVALID)
        (void)DclMemValErrorMode(0, nOldMemValErrorMode);

    DclProfilerLeave(0);

    if(dclStat == DCLSTAT_SUCCESS)
        DclPrintf("    OK\n");
    else
        DclPrintf("    FAILED\n");

    return dclStat;
}


/*-------------------------------------------------------------------
    Protected: DclTestMemoryProtection()

    This function verifies that the memory tracking mechanism can
    detect memory overruns and underruns.

    Parameters:
        fForce - TRUE to force the memory protection tests to run
                 even if memory tracking is disabled (may FAULT!)

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclTestMemoryProtection(
    D_BOOL                  fForce)
{
    DCLSTATUS               dclStat;
    D_UINT32                ulAlign;
    D_UINT32                ulSize;
    D_BOOL                  fMemTrackingOn = TRUE;
    unsigned                nOldMemTrackingErrorMode = DCLMEM_ERROR_INVALID; /* Init'ed for a picky compiler only */
    unsigned                nOldMemValErrorMode = DCLMEM_ERROR_INVALID;
    DCLDECLAREREQUESTPACKET (PROFILER, MEMPOOLSTATE, mempoolstate);  /* DCLREQ_PROFILER_MEMPOOLSTATE */
    D_BOOL                  fMemPoolStateValid = FALSE;

    DclProfilerEnter(__FUNCTION__, 0, 0);

    DclPrintf("Testing Memory Tracking and Protection...\n");

    /*  If the profiler is in use, it will internally validate the memory
        pool.  Since this test is intentionally causing underruns and 
        overruns, we don't want the profiler to catch these.  Temporarily
        disable that feature of the profiler.
    */        
    mempoolstate.fNewValidationEnabled = FALSE;
    dclStat = DclServiceIoctl(NULL, DCLSERVICE_PROFILER, &mempoolstate.ior);
    if(dclStat == DCLSTAT_SUCCESS)
        fMemPoolStateValid = TRUE;

    dclStat = DclMemTrackPoolVerify(0, FALSE);
    if(dclStat == DCLSTAT_SERVICE_NOTREGISTERED || dclStat == DCLSTAT_FEATUREDISABLED)
    {
        fMemTrackingOn = FALSE;
        
        if(!fForce)
        {
            DclPrintf("  Memory tracking is disabled.  Use the /MemProtect switch to force this\n");
            DclPrintf("  test to run even when DCLCONF_MEMTRACKING is FALSE.\n");

            dclStat = DCLSTAT_SUCCESS;
            goto Cleanup;
        }
    }
    else if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("  Memory pool verification failed: %lX\n", dclStat);
        goto Cleanup;
    }

    if(fMemTrackingOn)
    {
        /*  Set the error mode so that we simply fail a call, rather than
            asserting on errors.  Note that setting this mode is of primary
            benefit when DCLCONF_MEMORYTRACKING is enabled.  This mode has
            no control over what the OS does when memory management errors
            occur.
        */
        nOldMemTrackingErrorMode = DclMemTrackErrorMode(0, DCLMEM_ERROR_FAIL);
    }

    nOldMemValErrorMode = DclMemValErrorMode(0, DCLMEM_ERROR_FAIL);

    ulAlign = DCLMAX(DCL_ALIGNSIZE+1, 33);

    DclPrintf("  Testing Memory Tracking...\n");
    {
        if(fMemTrackingOn)
        {
            for(ulSize = 1; ulSize <= ulAlign; ulSize++)
            {
                dclStat = TestMemTracking(ulSize);
                if(dclStat != DCLSTAT_SUCCESS)
                    goto Cleanup;
            }
        }
        else
        {
            DclPrintf("    Memory tracking is disabled\n");
        }
    }

    if(fMemTrackingOn || fForce)
    {
        DclPrintf("  Testing Memory Protection...\n");

        if(!fMemTrackingOn)
        {
            DclPrintf("    WARNING! Running this test with the memory tracking feature disabled\n");
            DclPrintf("             may cause a fault or system crash.  Even if the test appears\n");
            DclPrintf("             to succeed, the memory pool may be corrupted but not manifest\n");
            DclPrintf("             any problems until some later point in time.\n");
        }

        for(ulSize = 1; ulSize <= ulAlign; ulSize++)
        {
            dclStat = TestMemProtection(fMemTrackingOn, ulSize);
            if(dclStat != DCLSTAT_SUCCESS)
                break;
        }
    }

  Cleanup:

    if(fMemTrackingOn)
    {
        /*  Restore the original error mode
        */
        (void)DclMemTrackErrorMode(0, nOldMemTrackingErrorMode);

        (void)nOldMemTrackingErrorMode;
    }

    if(nOldMemValErrorMode != DCLMEM_ERROR_INVALID)
        (void)DclMemValErrorMode(0, nOldMemValErrorMode);

    if(fMemPoolStateValid)
    {
        /*  If we were able to successfully disable the profiler's memory 
            validation stuff on entry, restore the original state.
        */        
        mempoolstate.fNewValidationEnabled = mempoolstate.fOldValidationEnabled;
        dclStat = DclServiceIoctl(NULL, DCLSERVICE_PROFILER, &mempoolstate.ior);
        DclAssert(dclStat == DCLSTAT_SUCCESS);
    }

    DclProfilerLeave(0);

    if(dclStat == DCLSTAT_SUCCESS)
        DclPrintf("    OK\n");
    else
        DclPrintf("    FAILED\n");

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: TestMemTracking()

    This function tests the memory tracking system which is enabled
    when DCLCONF_MEMTRACKING is TRUE.  It specifically is testing
    for memory overrun and underrun error handling.

    Parameters:

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestMemTracking(
    D_UINT32                ulAllocSize)
{
    DCLSTATUS               dclStat;
    DCLSTATUS               dclStat2;
    D_BUFFER               *pData;
    D_BUFFER               *pTest;
    D_BUFFER                ucSave;
    DCLDECLAREREQUESTPACKET (MEMTRACK, POOLVERIFY, verify);

    DclProfilerEnter(__FUNCTION__, 0, 0);

    DclPrintf("    Testing %lU byte allocation...\n", ulAllocSize);

    /*  Allocate memory for the test.
    */
    pData = DclMemAlloc(ulAllocSize);
    if(!pData)
    {
        DclPrintf("      Memory allocation failed\n");
        dclStat = DCLSTAT_MEMALLOCFAILED;
        goto TrackingCleanup;
    }

    /*  Cause an underrun by corrupting the memory immediately preceeding
        the allocation.  Save and restore the data at the memory address
        that we are corrupting.
    */
    DclPrintf("      Testing underrun...\n");
    pTest = pData - 1;
    ucSave = *pTest;
    *pTest = ~ucSave;
    verify.fQuiet = TRUE;
    dclStat = DclServiceIoctl(0, DCLSERVICE_MEMTRACK, &verify.ior);
    *pTest = ucSave;
    if(dclStat != DCLSTAT_MEMUNDERRUN)
    {
        DclPrintf("FAILED: Mem tracking system failed to report an underrun, Status=%lX\n", dclStat);

        /*  Make sure we are not returning DCLSTAT_SUCCESS...
        */
        if(dclStat == DCLSTAT_SUCCESS)
            dclStat = DCLSTAT_FAILURE;

        goto TrackingCleanup;
    }

    /*  Cause an overrun by corrupting the memory immediately following
        the allocation.  Save and restore the data at the memory address
        that we are corrupting.
    */
    DclPrintf("      Testing overrun...\n");
    pTest = pData + ulAllocSize + 1;
    ucSave = *pTest;
    *pTest = ~ucSave;

    verify.fQuiet = TRUE;
    dclStat = DclServiceIoctl(0, DCLSERVICE_MEMTRACK, &verify.ior);
    *pTest = ucSave;
    if(dclStat != DCLSTAT_MEMOVERRUN)
    {
        DclPrintf("FAILED: Mem tracking system failed to report an overrun, Status=%lX\n", dclStat);

        /*  Make sure we are not returning DCLSTAT_SUCCESS...
        */
        if(dclStat == DCLSTAT_SUCCESS)
            dclStat = DCLSTAT_FAILURE;

        goto TrackingCleanup;
    }

    /*  It is not possible to safely cause or test for the
        DCLSTAT_MEMPOOLCORRUPTED condition without knowing
        exactly where the memory pool control structures are
        located.  We are not supposed to know or have access
        to that information.
    */

    /*  If we get here, everything is good.
    */
    dclStat = DCLSTAT_SUCCESS;

  TrackingCleanup:

    if(pData)
    {
        dclStat2 = DclMemFree(pData);
        if(dclStat2 != DCLSTAT_SUCCESS)
        {
            DclPrintf("      Free failed even after correcting the problem, Status=%lX\n", dclStat2);

            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = dclStat2;
        }
    }
    
    DclProfilerLeave(0);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: TestMemProtection()

    Parameters:

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestMemProtection(
    D_BOOL          fMemTrackingOn,
    D_UINT32        ulAllocSize)
{
    DCLSTATUS       dclStat;
    D_BUFFER       *pData;
    D_BUFFER       *pTest;
    D_BUFFER       *pTemp;
    D_BUFFER        ucSave;

    DclProfilerEnter(__FUNCTION__, 0, 0);

    DclPrintf("    Testing %lU byte allocation...\n", ulAllocSize);

    /*  Cause an underrun by corrupting the memory immediately preceeding
        the allocation.  Save and restore the data at the memory address
        that we are corrupting.
    */
    DclPrintf("      Testing underrun...\n");

    pData = DclMemAlloc(ulAllocSize);
    if(!pData)
    {
        DclPrintf("      Memory allocation failed\n");
        dclStat = DCLSTAT_MEMALLOCFAILED;
        goto ProtectCleanup;
    }

    pTest = pData - 1;
    ucSave = *pTest;
    *pTest = ~ucSave;

    pTemp = DclMemAlloc(ulAllocSize);
    if(pTemp)
    {
        DclPrintf("        Allocating a subsequent block worked\n");

        dclStat = DclMemFree(pTemp);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("        Freeing the subsequent block failed, Status %lX\n", dclStat);

            /*  Note that this could result in a memory leak, but there is
                nothing we can do about it, and we've warned the user that
                this test is dangerous.
            */
        }
        else
        {
            DclPrintf("        Freeing the subsequent block worked\n");
        }
    }
    else
    {
        DclPrintf("        Allocating a subsequent block failed\n");
    }

    dclStat = DclMemFree(pData);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("        Freeing the block with the underrun failed, Status %lX\n", dclStat);

        /*  Correct the underrun so the free will hopefully work...
        */
        *pTest = ucSave;

        dclStat = DclMemFree(pData);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("        Freeing the block with the underrun still failed, Status %lX\n", dclStat);

            /*  Things are unrecoverable at this point, just fail the test
            */
            goto ProtectCleanup;
        }
        else
        {
            DclPrintf("        Freeing the block after correcting the underrun worked\n");
        }
    }
    else
    {
        DclPrintf("        Freeing the block with the underrun worked\n");
    }

    /*  Cause an overrun by corrupting the memory immediately following
        the allocation.  Save and restore the data at the memory address
        that we are corrupting.
    */
    DclPrintf("      Testing overrun...\n");

    pData = DclMemAlloc(ulAllocSize);
    if(!pData)
    {
        DclPrintf("      Memory allocation failed\n");
        dclStat = DCLSTAT_MEMALLOCFAILED;
        goto ProtectCleanup;
    }

    pTest = pData + ulAllocSize + 1;
    ucSave = *pTest;
    *pTest = ~ucSave;

    pTemp = DclMemAlloc(ulAllocSize);
    if(pTemp)
    {
        DclPrintf("        Allocating a subsequent block worked\n");

        dclStat = DclMemFree(pTemp);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("        Freeing the subsequent block failed, Status %lX\n", dclStat);

            /*  Note that this could result in a memory leak, but there is
                nothing we can do about it, and we've warned the user that
                this test is dangerous.
            */
        }
        else
        {
            DclPrintf("        Freeing the subsequent block worked\n");
        }
    }
    else
    {
        DclPrintf("        Allocating a subsequent block failed\n");
    }

    dclStat = DclMemFree(pData);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("        Freeing the block with the overrun failed, Status %lX\n", dclStat);

        /*  Correct the overrun so the free will hopefully work...
        */
        *pTest = ucSave;

        dclStat = DclMemFree(pData);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("        Freeing the block with the overrun still failed, Status %lX\n", dclStat);

            /*  Things are unrecoverable at this point, just fail the test
            */
            goto ProtectCleanup;
        }
        else
        {
            DclPrintf("        Freeing the block after correcting the overrun worked\n");
        }
    }
    else
    {
        DclPrintf("        Freeing the block with the overrun worked\n");
    }

  ProtectCleanup:

    DclProfilerLeave(0);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: TestMemStress()

    Parameters:

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestMemStress(
    unsigned        nAlignBytes)
{
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    D_UINT32        ulSize;
    D_UINT32        ulMaxBlockSize = 0;
    D_BUFFER       *pMem;
    DCLTIMESTAMP    ts;

    DclProfilerEnter(__FUNCTION__, 0, 0);

    DclPrintf("  Determining the maximum allocatable block size...\n");
    {
        D_UINT32    ulBase = 0;
        D_BOOL      fTooSlow = FALSE;

        /*  Start a 1KB and double until the allocation request fails, or we
            reach MAX_LARGE_ALLOCATION_SIZE.
        */
        for(ulSize = START_SCALE;
            ulBase+ulSize <= MAX_LARGE_ALLOCATION_SIZE;
            ulSize <<= 1)
        {
            D_UINT32    ulMS;

            ts = DclTimeStamp();

            pMem = DclMemAlloc(ulBase+ulSize);
            if(!pMem)
            {
                DclPrintf("    Attempted allocation of %lU KB failed, resetting scale\n", (ulBase+ulSize)/1024);

                if(ulSize > START_SCALE)
                {
                    /*  Adjust the new base and reset the size so we can
                        continue to try to allocate larger blocks
                    */
                    ulBase += ulSize >> 1;
                    ulSize = START_SCALE >> 1;

                    continue;
                }
                else
                {
                    ulMaxBlockSize = ulBase + (ulSize >> 1);

                    DclPrintf("    Largest allocatable block size is %lU KB\n", ulMaxBlockSize / 1024);

                    break;
                }
            }

            /*  Read and write the first and last bytes...
            */
            pMem[0] += pMem[ulBase+ulSize-1];
            pMem[ulBase+ulSize-1] += pMem[0];

            dclStat = DclMemFree(pMem);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Free of pMem=%P size=%lU failed with error code %lX\n", pMem, ulBase+ulSize, dclStat);
                goto StressCleanup;
            }

            ulMS = DclTimePassed(ts);

            DclPrintf("    Allocated and freed %7lU KB in %4lU ms\n", (ulBase+ulSize) / 1024, ulMS);

            if(ulMS > MAX_MILLISECONDS)
            {
                DclPrintf("    Quitting early due to slow performance\n");
                fTooSlow = TRUE;
                break;
            }
        }

        if(ulMaxBlockSize == 0 && !fTooSlow)
        {
            DclPrintf("    Maxed out the %lU MB test limit, largest allocatable size is <really big>\n",
                MAX_LARGE_ALLOCATION_SIZE / (1024 * 1024));
        }
    }

    if(ulMaxBlockSize)
    {
        D_UINT32    ulBlockSizeKB = (ulMaxBlockSize / VMM_DIVISOR) / 1024;
        D_BUFFER   *apMem[VMM_DIVISOR*2];
        D_BUFFER   *pTestBlock;
        unsigned    nn;
        unsigned    nBlocks = 0;

        ulBlockSizeKB = DCLMAX(ulBlockSizeKB, 1);

        DclPrintf("  Testing for the existence of movable memory...\n");
        DclPrintf("    Allocating as many (up to %u) blocks of %lU KB each, as we can...\n",
            DCLDIMENSIONOF(apMem), ulBlockSizeKB);

        for (nn = 0; nn < DCLDIMENSIONOF(apMem); nn++)
        {
            apMem[nn] = DclMemAlloc(ulBlockSizeKB * 1024);
            if(!apMem[nn])
                break;

            nBlocks++;
        }

        DclPrintf("    Sucessfully allocated %u blocks\n", nBlocks);
        DclPrintf("    Freeing the odd numbered allocations...\n");

        for(nn=1; nn<nBlocks; nn+=2)
        {
            dclStat = DclMemFree(apMem[nn]);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Free of pMem=%P block %u of %u failed with error code %lX\n",
                    apMem[nn], nn, nBlocks, dclStat);
                goto StressCleanup;
            }
        }

        DclPrintf("    Attempting to allocate a %lU KB block...\n", ulBlockSizeKB * VMM_BLOCK_MULTIPLIER);

        pTestBlock = DclMemAlloc((ulBlockSizeKB * 1024) * VMM_BLOCK_MULTIPLIER);
        if(!pTestBlock)
        {
            DclPrintf("      Unable to allocate the memory, even when it is known that the space is\n");
            DclPrintf("      available.  Memory does not appear to be movable, and is subject to\n");
            DclPrintf("      fragmentation.\n");
        }
        else
        {
            DclPrintf("      Block allocated successfully -- the system appears to support movable\n");
            DclPrintf("      memory and is resistant to fragmentation.\n");

            dclStat = DclMemFree(pTestBlock);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Free of pMem=%P block failed with error code %lX\n",
                    pTestBlock, dclStat);
                goto StressCleanup;
            }
        }

        DclPrintf("    Freeing the even numbered allocations...\n");

        for(nn=0; nn<nBlocks; nn+=2)
        {
            dclStat = DclMemFree(apMem[nn]);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Free of pMem=%P block %u of %u failed with error code %lX\n",
                    apMem[nn], nn, nBlocks, dclStat);
                goto StressCleanup;
            }
        }
    }

    if(ulMaxBlockSize)
    {
        D_UINT32        ulAllocSize;
        char           *pHead = NULL;
        D_UINT32        ulCount = 0;

        DclPrintf("  Testing allocation performance and limits...\n");

        ulAllocSize = DCLMAX(nAlignBytes, ulMaxBlockSize / OVERHEAD_DIVISOR);

        ulAllocSize &= ~(nAlignBytes-1);

        DclPrintf("    Allocating %lU byte blocks...\n", ulAllocSize);

        ts = DclTimeStamp();

        while(TRUE)
        {
            char   *pNew;

            pNew = DclMemAlloc(ulAllocSize);
            if(!pNew)
            {
                DclPrintf("      %lU allocations of %lU took %lU ms\n",
                    ulCount % DISPLAY_INTERVAL, ulCount, DclTimePassed(ts));

                break;
            }

            DclMemCpy(pNew, &pHead, sizeof(pHead));
            pHead = pNew;

            ulCount++;

            if(!(ulCount % DISPLAY_INTERVAL))
            {
                DclPrintf("      %u allocations of %lU took %lU ms\n",
                    DISPLAY_INTERVAL, ulCount, DclTimePassed(ts));

                ts = DclTimeStamp();
            }
        }

        DclPrintf("    Allocated %lU blocks, containing approximately %lU KB\n",
            ulCount, (ulAllocSize * ulCount) / 1024);

        if(ulAllocSize * ulCount > ulMaxBlockSize)
            DclPrintf("    Per allocation overhead cannot be accurately calculated\n");
        else
            DclPrintf("    Per allocation overhead is approximately %lU bytes\n",
                (ulMaxBlockSize - (ulAllocSize * ulCount)) / ulCount);

        ts = DclTimeStamp();

        ulCount = 0;
        while(pHead)
        {
            char *pNext;

            DclMemCpy(&pNext, pHead, sizeof(pHead));

            dclStat = DclMemFree(pHead);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Free of pMem=%P failed with error code %lX\n", pHead, dclStat);
                goto StressCleanup;
            }

            pHead = pNext;

            ulCount++;

            if(!(ulCount % DISPLAY_INTERVAL))
            {
                DclPrintf("      %u frees took %lU ms\n", DISPLAY_INTERVAL, DclTimePassed(ts));
                ts = DclTimeStamp();
            }
        }
    }

  StressCleanup:

    DclProfilerLeave(0);

    return dclStat;
}

