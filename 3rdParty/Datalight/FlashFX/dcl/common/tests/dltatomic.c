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

    This module contains code for testing the OS Atomic operations
    abstraction.

    ToDo:
    - Update this test to verify atomicity of operations.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltatomic.c $
    Revision 1.6  2010/08/04 00:10:31Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.5  2010/01/23 21:27:43Z  garyp
    Added a test for DclOsAtomic32ExchangeAdd().
    Revision 1.4  2009/11/09 18:08:06Z  garyp
    Updated the performance timings to account for test overhead.
    Revision 1.3  2009/07/01 19:34:41Z  garyp
    Fixed to initialize a local.
    Revision 1.2  2009/06/29 01:00:27Z  garyp
    Fixed to build cleanly with GNU compilers.
    Revision 1.1  2009/06/24 19:30:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include "dltests.h"

#define PERFSECS                 (2)    /* 4 max! */
#define NANOS_PER_SEC   (1000000000)


/*-------------------------------------------------------------------
    Private: DclTestAtomicOperations()

    Invoke the unit tests for excercising the atomic operations API.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclTestAtomicOperations(void)
{
    DCLSTATUS       dclStat;
    D_ATOMIC32      ulAtomic = 0;
    D_ATOMICPTR     pAtomic = NULL;
    D_UINT32        ulCount = 0;
    D_UINT64        ullOverheadNS;
    D_UINT64        ullNS;
    D_UINT64        ullAbs;
    D_UINT32        ulOld;
    D_UINT32        ulAdd;
    D_UINT32        ulPrev;
    void           *pOld;
    DCLTIMESTAMP    ts;

    DclPrintf("Testing Atomic Operations...\n");

    DclPrintf("    Counting atomic operations performed in %u seconds \n", PERFSECS);
    DclPrintf("                                                 Absolute   No Overhead\n");
    DclPrintf("         Operation Type                Count    Average NS   Average NS\n");
 
    /*  ------->  Calibration  <--------   */

    /*  Note that this overhead calibration is approximate, and therefore
        when we display the results below, we'll simply floor the results
        with the overhead value, to ensure that we're not displaying
        negative numbers.
    */
    ts = DclTimeStamp();
    ulAtomic = 0;

    while(DclTimePassedNS(ts) < PERFSECS * NANOS_PER_SEC)
    {
        if(ulCount != ulAtomic)
        {
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }
        ulCount++;
        ulAtomic++;
    }

    ullOverheadNS = DclTimePassedNS(ts);
    DclAssert(ulCount);
    DclUint64DivUint32(&ullOverheadNS, ulCount);

    DclPrintf("    Overhead Calibration            %9lU %9llU          n/a\n", 
        ulCount, VA64BUG(ullOverheadNS));

    /*  ------->  Test READ  <--------   */

    ts = DclTimeStamp();
    ulAtomic = 0;

    while(DclTimePassedNS(ts) < PERFSECS * NANOS_PER_SEC)
    {
        ulCount = DclOsAtomic32Retrieve(&ulAtomic);
        if(ulCount != ulAtomic)
        {
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }
        ulAtomic++;
    }

    ullNS = DclTimePassedNS(ts);
    DclAssert(ulCount);
    DclUint64DivUint32(&ullNS, ulCount);
    ullNS = DCLMAX(ullNS, ullOverheadNS);
    ullAbs = ullNS - ullOverheadNS;

    DclPrintf("    Atomic32 Read:                  %9lU %9llU    %9llU\n", 
        ulCount, VA64BUG(ullNS), VA64BUG(ullAbs));

    /*  ------->  Test INCREMENT  <--------   */

    ts = DclTimeStamp();
    ulAtomic = 0;

    while(DclTimePassedNS(ts) < PERFSECS * NANOS_PER_SEC)
    {
        ulOld = ulAtomic;
        ulCount = DclOsAtomic32Increment(&ulAtomic);
        if(ulCount != ulOld + 1)
        {
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }
    }

    ullNS = DclTimePassedNS(ts);
    DclAssert(ulCount);
    DclUint64DivUint32(&ullNS, ulCount);
    ullNS = DCLMAX(ullNS, ullOverheadNS);
    ullAbs = ullNS - ullOverheadNS;

    DclPrintf("    Atomic32 Increment              %9lU %9llU    %9llU\n", 
        ulCount, VA64BUG(ullNS), VA64BUG(ullAbs));
 
    /*  ------->  Test DECREMENT  <--------   */

    ts = DclTimeStamp();
    ulAtomic = D_UINT32_MAX;

    while(DclTimePassedNS(ts) < PERFSECS * NANOS_PER_SEC)
    {
        ulOld = ulAtomic;
        ulCount = DclOsAtomic32Decrement(&ulAtomic);
        if(ulCount != ulOld - 1)
        {
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }
    }

    ullNS = DclTimePassedNS(ts);
    ulCount = D_UINT32_MAX - ulCount;
    DclAssert(ulCount);
    DclUint64DivUint32(&ullNS, ulCount);
    ullNS = DCLMAX(ullNS, ullOverheadNS);
    ullAbs = ullNS - ullOverheadNS;

    DclPrintf("    Atomic32 Decrement              %9lU %9llU    %9llU\n", 
        ulCount, VA64BUG(ullNS), VA64BUG(ullAbs));
 
    /*  ------->  Test EXCHANGE  <--------   */

    ts = DclTimeStamp();
    ulAtomic = 0;

    while(DclTimePassedNS(ts) < PERFSECS * NANOS_PER_SEC)
    {
        ulOld = DclOsAtomic32Exchange(&ulAtomic, ulAtomic+1);
        if(ulOld != ulAtomic - 1)
        {
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }
    }

    ullNS = DclTimePassedNS(ts);
    DclAssert(ulAtomic);
    DclUint64DivUint32(&ullNS, ulAtomic);
    ullNS = DCLMAX(ullNS, ullOverheadNS);
    ullAbs = ullNS - ullOverheadNS;

    DclPrintf("    Atomic32 Exchange               %9lU %9llU    %9llU\n", 
        ulCount, VA64BUG(ullNS), VA64BUG(ullAbs));
 
    /*  ------->  Test EXCHANGEADD  <--------   */

    ts = DclTimeStamp();
    ulAtomic = 0;
    ulAdd = DclRand(NULL);
    ulPrev = ulAtomic;

    while(DclTimePassedNS(ts) < PERFSECS * NANOS_PER_SEC)
    {
        ulOld = DclOsAtomic32ExchangeAdd(&ulAtomic, ulAdd);
        if(ulOld != ulPrev)
        {
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }

        ulPrev += ulAdd;
    }

    ullNS = DclTimePassedNS(ts);
    DclAssert(ulAtomic);
    DclUint64DivUint32(&ullNS, ulAtomic);
    ullNS = DCLMAX(ullNS, ullOverheadNS);
    ullAbs = ullNS - ullOverheadNS;

    DclPrintf("    Atomic32 ExchangeAdd            %9lU %9llU    %9llU\n", 
        ulCount, VA64BUG(ullNS), VA64BUG(ullAbs));
 
    /*  ------->  Test COMPARE & EXCHANGE (FALSE)  <--------   */

    ts = DclTimeStamp();
    ulAtomic = 0;

    while(DclTimePassedNS(ts) < PERFSECS * NANOS_PER_SEC)
    {
        ulOld = DclOsAtomic32CompareExchange(&ulAtomic, D_UINT32_MAX, ulAtomic);
        if(ulOld != ulAtomic)
        {
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }
        ulAtomic++;
        DclAssert(ulAtomic != D_UINT32_MAX);
    }

    ullNS = DclTimePassedNS(ts);
    DclAssert(ulAtomic);
    DclUint64DivUint32(&ullNS, ulAtomic);
    ullNS = DCLMAX(ullNS, ullOverheadNS);
    ullAbs = ullNS - ullOverheadNS;

    DclPrintf("    Atomic32 CompareExchange(NOP)   %9lU %9llU    %9llU\n", 
        ulCount, VA64BUG(ullNS), VA64BUG(ullAbs));
 
    /*  ------->  Test COMPARE & EXCHANGE (TRUE)  <--------   */

    ts = DclTimeStamp();
    ulAtomic = 0;

    while(DclTimePassedNS(ts) < PERFSECS * NANOS_PER_SEC)
    {
        ulOld = DclOsAtomic32CompareExchange(&ulAtomic, ulAtomic, ulAtomic+1);
        if(ulOld != ulAtomic - 1)
        {
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }
    }

    ullNS = DclTimePassedNS(ts);
    DclAssert(ulAtomic);
    DclUint64DivUint32(&ullNS, ulAtomic);
    ullNS = DCLMAX(ullNS, ullOverheadNS);
    ullAbs = ullNS - ullOverheadNS;

    DclPrintf("    Atomic32 CompareExchange(OP)    %9lU %9llU    %9llU\n", 
        ulCount, VA64BUG(ullNS), VA64BUG(ullAbs));

    /*  ------->  Test PTR READ <--------   */

    ts = DclTimeStamp();
    pAtomic = NULL;
    ulCount = 0;

    while(DclTimePassedNS(ts) < PERFSECS * NANOS_PER_SEC)
    {
        pOld = DclOsAtomicPtrRetrieve(&pAtomic);
        if(pOld != pAtomic)
        {
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }
        pAtomic = ((char*)pAtomic)+1;
        ulCount++;
    }

    ullNS = DclTimePassedNS(ts);
    DclAssert(ulCount);
    DclUint64DivUint32(&ullNS, ulCount);
    ullNS = DCLMAX(ullNS, ullOverheadNS);
    ullAbs = ullNS - ullOverheadNS;

    DclPrintf("    AtomicPtr Read                  %9lU %9llU    %9llU\n", 
        ulCount, VA64BUG(ullNS), VA64BUG(ullAbs));
 
    /*  ------->  Test PTR EXCHANGE  <--------   */

    ts = DclTimeStamp();
    pAtomic = NULL;
    ulCount = 0;

    while(DclTimePassedNS(ts) < PERFSECS * NANOS_PER_SEC)
    {
        pOld = DclOsAtomicPtrExchange(&pAtomic, ((char*)pAtomic)+1);
        if(pOld != ((char*)pAtomic) - 1)
        {
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }
        ulCount++;
    }

    ullNS = DclTimePassedNS(ts);
    DclAssert(ulCount);
    DclUint64DivUint32(&ullNS, ulCount);
    ullNS = DCLMAX(ullNS, ullOverheadNS);
    ullAbs = ullNS - ullOverheadNS;

    DclPrintf("    AtomicPtr Exchange              %9lU %9llU    %9llU\n", 
        ulCount, VA64BUG(ullNS), VA64BUG(ullAbs));
 
    /*  ------->  Test PTR COMPARE & EXCHANGE (FALSE)  <--------   */

    ts = DclTimeStamp();
    pAtomic = NULL;
    ulCount = 0;

    while(DclTimePassedNS(ts) < PERFSECS * NANOS_PER_SEC)
    {
        pOld = DclOsAtomicPtrCompareExchange(&pAtomic, (void*)(D_INTPTR)-1, pAtomic);
        if(pOld != pAtomic)
        {
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }
        ulCount++;
        DclAssert(ulCount != D_UINT32_MAX);
    }

    ullNS = DclTimePassedNS(ts);
    DclAssert(ulCount);
    DclUint64DivUint32(&ullNS, ulCount);
    ullNS = DCLMAX(ullNS, ullOverheadNS);
    ullAbs = ullNS - ullOverheadNS;

    DclPrintf("    AtomicPtr CompareExchange(NOP)  %9lU %9llU    %9llU\n", 
        ulCount, VA64BUG(ullNS), VA64BUG(ullAbs));
 
    /*  ------->  Test COMPARE & EXCHANGE (TRUE)  <--------   */

    ts = DclTimeStamp();
    pAtomic = NULL;
    ulCount = 0;

    while(DclTimePassedNS(ts) < PERFSECS * NANOS_PER_SEC)
    {
        pOld = DclOsAtomicPtrCompareExchange(&pAtomic, pAtomic, ((char*)pAtomic)+1);
        if(pOld != ((char*)pAtomic) - 1)
        {
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }
        ulCount++;
        DclAssert(ulCount != D_UINT32_MAX);
    }

    ullNS = DclTimePassedNS(ts);
    DclAssert(ulCount);
    DclUint64DivUint32(&ullNS, ulCount);
    ullNS = DCLMAX(ullNS, ullOverheadNS);
    ullAbs = ullNS - ullOverheadNS;

    DclPrintf("    AtomicPtr CompareExchange(OP)   %9lU %9llU    %9llU\n", 
        ulCount, VA64BUG(ullNS), VA64BUG(ullAbs));

    dclStat = DCLSTAT_SUCCESS;

  Cleanup:
    if(dclStat == DCLSTAT_SUCCESS)
        DclPrintf("    OK\n");
    else
        DclPrintf("    FAILED\n");

    return dclStat;
}



