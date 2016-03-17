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

    This module contains unit tests for 64-bit math operations.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlt64bit.c $
    Revision 1.12  2010/08/04 00:10:31Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.11  2009/06/25 22:49:22Z  garyp
    Updated to include dltests.h.
    Revision 1.10  2009/02/26 21:57:19Z  keithg
    Changed fResult from unsigned to D_BOOL to match it's use and
    eliminate compiler warnings about signed/unsigned mismatches.
    Revision 1.9  2009/02/21 02:14:38Z  brandont
    Renamed Testudivdi3 to DclTestudivdi3.
    Revision 1.8  2008/11/05 15:42:32Z  jimmb
    Added unsigned 64 bit divide DclTestudivdi3 to DCL tests
    Revision 1.7  2007/11/03 23:31:12Z  Garyp
    Added the standard module header.
    Revision 1.6  2007/05/13 16:33:50Z  garyp
    Eliminated the use of recreated structure types to avoid confusing the
    MS debugger.  Changed the Boolean type used in comparisons from
    int back to unsigned, while also (in dl64bit.h) casting the result of the
    "less than" and "greater than" macros to an unsigned value -- so we can
    actually work with MSVC6 and MSVS8 at the same time!
    Revision 1.5  2007/05/11 17:14:49Z  Garyp
    Fixed to hopefully build cleanly.
    Revision 1.4  2007/05/11 02:56:54Z  Garyp
    Added tests for the various "greater than" and "less than" macros.  Cleaned
    up the test initialization logic.  Added standard function headers.
    Revision 1.3  2006/06/01 23:50:22Z  Garyp
    Added a test for U64 MOD U32.
    Revision 1.2  2006/02/26 20:57:14Z  Garyp
    Minor message cleanup -- nothing functional.
    Revision 1.1  2006/01/03 16:31:36Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include "dltests.h"

typedef struct
{
    D_UINT32    ulVal1High;
    D_UINT32    ulVal1Low;
} TESTCASE_U64;

typedef struct
{
    D_UINT32    ulVal1High;
    D_UINT32    ulVal1Low;
    D_UINT32    ulVal2High;
    D_UINT32    ulVal2Low;
    D_BOOL      fResult;
} TESTCASE_U64U64;

typedef struct
{
    D_UINT32    ulVal1High;
    D_UINT32    ulVal1Low;
    D_UINT16    uValue2;
    D_UINT32    ulResultHigh;
    D_UINT32    ulResultLow;
} TESTCASE_U64U16U64;

typedef struct
{
    D_UINT32    ulVal1High;
    D_UINT32    ulVal1Low;
    D_UINT32    ulValue2;
    D_UINT32    ulResultHigh;
    D_UINT32    ulResultLow;
} TESTCASE_U64U32U64;

typedef struct
{
    D_UINT32    ulVal1High;
    D_UINT32    ulVal1Low;
    D_UINT32    ulVal2High;
    D_UINT32    ulVal2Low;
    D_UINT32    ulResultHigh;
    D_UINT32    ulResultLow;
} TESTCASE_U64U64U64;

typedef struct
{
    D_UINT32    ulVal1High;
    D_UINT32    ulVal1Low;
    D_UINT32    ulValue2;
    D_BOOL      fResult;
} TESTCASE_U64U32;


static DCLSTATUS TestUint64IsEqualUint64(void);
static DCLSTATUS TestUint64AssignUint16(void);
static DCLSTATUS TestUint64AssignUint32(void);
static DCLSTATUS TestUint64AssignUint64(void);
static DCLSTATUS TestUint64AddUint16(void);
static DCLSTATUS TestUint64AddUint32(void);
static DCLSTATUS TestUint64AddUint64(void);
static DCLSTATUS TestUint64SubUint16(void);
static DCLSTATUS TestUint64SubUint32(void);
static DCLSTATUS TestUint64SubUint64(void);
static DCLSTATUS TestUint64MulUint16(void);
static DCLSTATUS TestUint64MulUint32(void);
static DCLSTATUS TestUint64DivUint32(void);
static DCLSTATUS TestUint64ModUint32(void);
static DCLSTATUS TestUint64GreaterUint32(void);
static DCLSTATUS TestUint64GreaterUint64(void);
static DCLSTATUS TestUint64LessUint32(void);
static DCLSTATUS TestUint64LessUint64(void);


/*-------------------------------------------------------------------
    DclTest64BitMath()

    Description
        This function invokes the unit tests for the 64-bit math
        routines.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclTest64BitMath(void)
{
    DCLSTATUS       dclStat;

    DclPrintf("Testing 64-bit math functions/macros...\n");

    /*  Validate the 64-bit compare equal functions/macros
    */
    if((dclStat = TestUint64IsEqualUint64()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  Validate the 64-bit assigning a 16-bit value functions/macros
    */
    if((dclStat = TestUint64AssignUint16()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  Validate the 64-bit assigning a 32-bit value functions/macros
    */
    if((dclStat = TestUint64AssignUint32()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  Validate the 64-bit assigning a 64-bit value functions/macros
    */
    if((dclStat = TestUint64AssignUint64()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  Validate the 64-bit adding a 16-bit math functions/macros
    */
    if((dclStat = TestUint64AddUint16()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  Validate the 64-bit adding a 32-bit math functions/macros
    */
    if((dclStat = TestUint64AddUint32()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  Validate the 64-bit adding a 64-bit math functions/macros
    */
    if((dclStat = TestUint64AddUint64()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  Validate the 64-bit subtracting a 16-bit math functions/macros
    */
    if((dclStat = TestUint64SubUint16()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  Validate the 64-bit subtracting a 32-bit math functions/macros
    */
    if((dclStat = TestUint64SubUint32()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  Validate the 64-bit subtracting a 64-bit math functions/macros
    */
    if((dclStat = TestUint64SubUint64()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  Validate the 64-bit multiply by 16-bit math functions/macros
    */
    if((dclStat = TestUint64MulUint16()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  Validate the 64-bit multiply by 32-bit math functions/macros
    */
    if((dclStat = TestUint64MulUint32()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  Validate the 64-bit dividing by 32-bit math functions/macros
    */
    if((dclStat = TestUint64DivUint32()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  Validate the 64-bit mod 32-bit math functions/macros
    */
    if((dclStat = TestUint64ModUint32()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestUint64GreaterUint32()) != DCLSTAT_SUCCESS)
        goto Failed;
    if((dclStat = TestUint64GreaterUint64()) != DCLSTAT_SUCCESS)
        goto Failed;
    if((dclStat = TestUint64LessUint32()) != DCLSTAT_SUCCESS)
        goto Failed;
    if((dclStat = TestUint64LessUint64()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  Validate that 64 bit unsigned division and signed division
    */
    if((dclStat = DclTestudivdi3()) != DCLSTAT_SUCCESS)
        goto Failed;


    DclPrintf("    OK\n");
    return DCLSTAT_SUCCESS;

  Failed:
    DclPrintf("    FAILED\n", dclStat);
    return dclStat;
}


/*-------------------------------------------------------------------
    TestUint64IsEqualUint64()

    Description
        Test the 64-bit equality functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64IsEqualUint64(void)
{
    TESTCASE_U64U64  TestCase[] =
    {
        {0, 0, 0, 0, TRUE},
        {0, 0, 0, 1, FALSE},
        {0, 0, 1, 0, FALSE},
        {0, 1, 0, 0, FALSE},
        {1, 0, 0, 0, FALSE},
        {0, 1, 0, 1, TRUE},
        {1, 0, 1, 0, TRUE},
        {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, TRUE},
    };
    D_BOOL          fIsEqual;
    unsigned        uIndex;
    D_UINT64        ullValue1;
    D_UINT64        ullValue2;

    /*  Validate the 64-bit compare equal functions/macros for each of the
        test cases.
    */
    DclPrintf("    Compare for equality\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);
        DclUint64AssignHiLo(&ullValue2, TestCase[uIndex].ulVal2High, TestCase[uIndex].ulVal2Low);

        /*  Perform the operation and validate the result
        */
        fIsEqual = DclUint64IsEqualUint64(&ullValue1, &ullValue2);
        if(fIsEqual != TestCase[uIndex].fResult)
        {
            DclPrintf("Error: DclUint64IsEqualUint64 uIndex=%u", uIndex);
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64AssignUint16()

    Description
        Test the 16-bit to 64-bit assignment functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64AssignUint16(void)
{
    D_UINT16        TestCase[] = {0, 1, 0x5327, 0xFFFF};
    D_BOOL          fIsEqual;
    unsigned        uIndex;
    D_UINT64        ullValue1;
    D_UINT64        ullValue2;

    /*  Validate the 64-bit assigning a 16-bit value functions/macros for each
        of the test cases.
    */
    DclPrintf("    Assign a 16-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, 0, (D_UINT32)TestCase[uIndex]);

        DclMemSet(&ullValue2, 0x77, sizeof(ullValue2));

        /*  Perform the operation and validate the result
        */
        DclUint64AssignUint16(&ullValue2, TestCase[uIndex]);
        fIsEqual = DclUint64IsEqualUint64(&ullValue1, &ullValue2);
        if(!fIsEqual)
        {
            DclPrintf("Error: DclUint64AssignUint16 uIndex=%u", uIndex);
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64AssignUint32()

    Description
        Test the 32-bit to 64-bit assignment functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64AssignUint32(void)
{
    D_UINT32    TestCase[] = {0, 1, 0x5327, 0xffff, 0x10000, 0x98761234, 0xffffffff};
    D_BOOL      fIsEqual;
    unsigned    uIndex;
    D_UINT64    ullValue1;
    D_UINT64    ullValue2;

    /*  Validate the 64-bit assigning a 32-bit value functions/macros for each
        of the test cases.
    */
    DclPrintf("    Assign a 32-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, 0, TestCase[uIndex]);

        DclMemSet(&ullValue2, 0x77, sizeof(ullValue2));

        /*  Perform the operation and validate the result
        */
        DclUint64AssignUint32(&ullValue2, TestCase[uIndex]);
        fIsEqual = DclUint64IsEqualUint64(&ullValue1, &ullValue2);
        if(!fIsEqual)
        {
            DclPrintf("Error: DclUint64AssignUint32 uIndex=%u", uIndex);
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64AssignUint64()

    Description
        Test the 64-bit to 64-bit assignment functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64AssignUint64(void)
{
    D_BOOL          fIsEqual;
    unsigned        uIndex;
    D_UINT64        ullValue1;
    D_UINT64        ullValue2;
    TESTCASE_U64    TestCase[] =
    {
        {0x00000000, 0x00000000},
        {0x00000000, 0x00000001},
        {0x00000001, 0x00000000},
        {0x98761234, 0x54327891},
        {0xffffffff, 0xffffffff},
    };

    /*  Validate the 64-bit assigning a 64-bit value functions/macros for each
        of the test cases.
    */
    DclPrintf("    Assign a 64-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);

        DclMemSet(&ullValue2, 0x77, sizeof(ullValue2));

        /*  Perform the operation and validate the result
        */
        DclUint64AssignUint64(&ullValue2, &ullValue1);
        fIsEqual = DclUint64IsEqualUint64(&ullValue1, &ullValue2);
        if(!fIsEqual)
        {
            DclPrintf("Error: DclUint64AssignUint64 uIndex=%u", uIndex);
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64AddUint16()

    Description
        Test the add a 16-bit value to a 64-bit value functions/
        macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64AddUint16(void)
{
    D_BOOL              fIsEqual;
    unsigned            uIndex;
    D_UINT64            ullValue1;
    D_UINT64            ullResult;
    TESTCASE_U64U16U64  TestCase[] =
    {
        {0x00000000, 0x00000000, 0x0000, 0x00000000, 0x00000000},
        {0xffffffff, 0xffffffff, 0x0000, 0xffffffff, 0xffffffff},
        {0x00000000, 0x00000000, 0xffff, 0x00000000, 0x0000ffff},
        {0x00000000, 0x00000001, 0xffff, 0x00000000, 0x00010000},
        {0x00000000, 0xffffffff, 0x0001, 0x00000001, 0x00000000},
        {0xefffffff, 0xfffffff1, 0x000f, 0xf0000000, 0x00000000},
        {0xffffffff, 0xfffffff1, 0x000f, 0x00000000, 0x00000000},
        {0xffffffff, 0xfffffff2, 0x000f, 0x00000000, 0x00000001},
        {0xffffffff, 0xffffffff, 0xffff, 0x00000000, 0x0000fffe},
        {0x67894567, 0x87542357, 0x7593, 0x67894567, 0x875498ea},
    };

    /*  Validate the 64-bit adding a 16-bit math functions/macros for each of
        the test cases
    */
    DclPrintf("    Add a 16-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);
        DclUint64AssignHiLo(&ullResult, TestCase[uIndex].ulResultHigh, TestCase[uIndex].ulResultLow);

        /*  Perform the operation and validate the result
        */
        DclUint64AddUint16(&ullValue1, TestCase[uIndex].uValue2);
        fIsEqual = DclUint64IsEqualUint64(&ullValue1, &ullResult);
        if(!fIsEqual)
        {
            DclPrintf("Error: DclUint64AddUint16 uIndex=%u", uIndex);
            DclPrintf("ullValue1: %llx\n", VA64BUG(ullValue1));
            DclPrintf("ullResult: %llx\n", VA64BUG(ullResult));
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64AddUint32()

    Description
        Test the add a 32-bit value to a 64-bit value functions/
        macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64AddUint32(void)
{
    D_BOOL              fIsEqual;
    unsigned            uIndex;
    D_UINT64            ullValue1;
    D_UINT64            ullResult;
    TESTCASE_U64U32U64  TestCase[] =
    {
        {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
        {0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff},
        {0x00000000, 0x00000000, 0x0000ffff, 0x00000000, 0x0000ffff},
        {0x00000000, 0x00000001, 0x0000ffff, 0x00000000, 0x00010000},
        {0x00000000, 0xffffffff, 0x00000001, 0x00000001, 0x00000000},
        {0xefffffff, 0xfffffff1, 0x0000000f, 0xf0000000, 0x00000000},
        {0xffffffff, 0xfffffff1, 0x0000000f, 0x00000000, 0x00000000},
        {0xffffffff, 0xfffffff2, 0x0000000f, 0x00000000, 0x00000001},
        {0xffffffff, 0xffffffff, 0x0000ffff, 0x00000000, 0x0000fffe},
        {0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0xfffffffe},
        {0x84577964, 0x29474687, 0x84324891, 0x84577964, 0xad798f18},
    };

    /*  Validate the 64-bit adding a 32-bit math functions/macros for each of
        the test cases
    */
    DclPrintf("    Add a 32-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);
        DclUint64AssignHiLo(&ullResult, TestCase[uIndex].ulResultHigh, TestCase[uIndex].ulResultLow);

        /*  Perform the operation and validate the result
        */
        DclUint64AddUint32(&ullValue1, TestCase[uIndex].ulValue2);
        fIsEqual = DclUint64IsEqualUint64(&ullValue1, &ullResult);
        if(!fIsEqual)
        {
            DclPrintf("Error: DclUint64AddUint32 uIndex=%u", uIndex);
            DclPrintf("ullValue1: %llx\n", VA64BUG(ullValue1));
            DclPrintf("ullResult: %llx\n", VA64BUG(ullResult));
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64AddUint64()

    Description
        Test the add a 64-bit value to a 64-bit value functions/
        macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64AddUint64(void)
{
    D_BOOL              fIsEqual;
    unsigned            uIndex;
    D_UINT64            ullValue1;
    D_UINT64            ullValue2;
    D_UINT64            ullResult;
    TESTCASE_U64U64U64  TestCase[] =
    {
        {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
        {0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff},
        {0x00000000, 0x00000000, 0x00000000, 0x0000ffff, 0x00000000, 0x0000ffff},
        {0x00000000, 0x00000001, 0x00000000, 0x0000ffff, 0x00000000, 0x00010000},
        {0x00000000, 0xffffffff, 0x00000000, 0x00000001, 0x00000001, 0x00000000},
        {0xefffffff, 0xfffffff1, 0x00000000, 0x0000000f, 0xf0000000, 0x00000000},
        {0xffffffff, 0xfffffff1, 0x00000000, 0x0000000f, 0x00000000, 0x00000000},
        {0xffffffff, 0xfffffff2, 0x00000000, 0x0000000f, 0x00000000, 0x00000001},
        {0xffffffff, 0xffffffff, 0x00000000, 0x0000ffff, 0x00000000, 0x0000fffe},
        {0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0x00000000, 0xfffffffe},
        {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffffffe},
        {0x84577964, 0x29474687, 0x84324891, 0x84791573, 0x0889c1f5, 0xadc05bfa},
    };

    /*  Validate the 64-bit adding a 64-bit math functions/macros for each of
        the test cases
    */
    DclPrintf("    Add a 64-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);
        DclUint64AssignHiLo(&ullValue2, TestCase[uIndex].ulVal2High, TestCase[uIndex].ulVal2Low);
        DclUint64AssignHiLo(&ullResult, TestCase[uIndex].ulResultHigh, TestCase[uIndex].ulResultLow);

        /*  Perform the operation and validate the result
        */
        DclUint64AddUint64(&ullValue1, &ullValue2);
        fIsEqual = DclUint64IsEqualUint64(&ullValue1, &ullResult);
        if(!fIsEqual)
        {
            DclPrintf("Error: DclUint64AddUint64 uIndex=%u", uIndex);
            DclPrintf("ullValue1: %llx\n", VA64BUG(ullValue1));
            DclPrintf("ullResult: %llx\n", VA64BUG(ullResult));
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64SubUint16()

    Description
        Test the subtract a 16-bit value from a 64-bit value
        functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64SubUint16(void)
{
    D_BOOL              fIsEqual;
    unsigned            uIndex;
    D_UINT64            ullValue1;
    D_UINT64            ullResult;
    TESTCASE_U64U16U64  TestCase[] =
    {
        {0x00000000, 0x00000000, 0x0000, 0x00000000, 0x00000000},
        {0xffffffff, 0xffffffff, 0x0000, 0xffffffff, 0xffffffff},
        {0x00000000, 0x00000000, 0x0001, 0xffffffff, 0xffffffff},
        {0x00000000, 0x00000000, 0xffff, 0xffffffff, 0xffff0001},
        {0xffffffff, 0xffffffff, 0x0001, 0xffffffff, 0xfffffffe},
        {0xffffffff, 0xffffffff, 0xffff, 0xffffffff, 0xffff0000},
        {0x67894567, 0x87542357, 0x7593, 0x67894567, 0x8753adc4},
    };

    /*  Validate the 64-bit subtracting a 16-bit math functions/macros for each of
        the test cases
    */
    DclPrintf("    Subtract a 16-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);
        DclUint64AssignHiLo(&ullResult, TestCase[uIndex].ulResultHigh, TestCase[uIndex].ulResultLow);

        /*  Perform the operation and validate the result
        */
        DclUint64SubUint16(&ullValue1, TestCase[uIndex].uValue2);
        fIsEqual = DclUint64IsEqualUint64(&ullValue1, &ullResult);
        if(!fIsEqual)
        {
            DclPrintf("Error: DclUint64SubUint16 uIndex=%u", uIndex);
            DclPrintf("ullValue1: %llx\n", VA64BUG(ullValue1));
            DclPrintf("ullResult: %llx\n", VA64BUG(ullResult));
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64SubUint32()

    Description
        Test the subtract a 32-bit value from a 64-bit value
        functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64SubUint32(void)
{
    D_BOOL              fIsEqual;
    unsigned            uIndex;
    D_UINT64            ullValue1;
    D_UINT64            ullResult;
    TESTCASE_U64U32U64  TestCase[] =
    {
        {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
        {0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff},
        {0x00000000, 0x00000000, 0x00000001, 0xffffffff, 0xffffffff},
        {0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x00000001},
        {0xffffffff, 0xffffffff, 0x00000001, 0xffffffff, 0xfffffffe},
        {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000},
        {0x84577964, 0x29474687, 0x84324891, 0x84577963, 0xa514fdf6},
    };

    /*  Validate the 64-bit subtracting a 32-bit math functions/macros for each of
        the test cases
    */
    DclPrintf("    Subtract a 32-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);
        DclUint64AssignHiLo(&ullResult, TestCase[uIndex].ulResultHigh, TestCase[uIndex].ulResultLow);

        /*  Perform the operation and validate the result
        */
        DclUint64SubUint32(&ullValue1, TestCase[uIndex].ulValue2);
        fIsEqual = DclUint64IsEqualUint64(&ullValue1, &ullResult);
        if(!fIsEqual)
        {
            DclPrintf("Error: DclUint64SubUint32 uIndex=%u", uIndex);
            DclPrintf("ullValue1: %llx\n", VA64BUG(ullValue1));
            DclPrintf("ullResult: %llx\n", VA64BUG(ullResult));
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64SubUint64()

    Description
        Test the subtract a 64-bit value from a 64-bit value
        functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64SubUint64(void)
{
    D_BOOL              fIsEqual;
    unsigned            uIndex;
    D_UINT64            ullValue1;
    D_UINT64            ullValue2;
    D_UINT64            ullResult;
    TESTCASE_U64U64U64  TestCase[] =
    {
        {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
        {0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff},
        {0x00000000, 0x00000000, 0x00000000, 0x00000001, 0xffffffff, 0xffffffff},
        {0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x00000000, 0x00000001},
        {0xffffffff, 0xffffffff, 0x00000000, 0x00000001, 0xffffffff, 0xfffffffe},
        {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000},
        {0x84577964, 0x29474687, 0x84324891, 0x84791573, 0x002530d2, 0xa4ce3114},
    };

    /*  Validate the 64-bit subtracting a 64-bit math functions/macros for
        each of the test cases
    */
    DclPrintf("    Subtract a 64-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);
        DclUint64AssignHiLo(&ullValue2, TestCase[uIndex].ulVal2High, TestCase[uIndex].ulVal2Low);
        DclUint64AssignHiLo(&ullResult, TestCase[uIndex].ulResultHigh, TestCase[uIndex].ulResultLow);

        /*  Perform the operation and validate the result
        */
        DclUint64SubUint64(&ullValue1, &ullValue2);
        fIsEqual = DclUint64IsEqualUint64(&ullValue1, &ullResult);
        if(!fIsEqual)
        {
            DclPrintf("Error: DclUint64SubUint64 uIndex=%u", uIndex);
            DclPrintf("ullValue1: %llx\n", VA64BUG(ullValue1));
            DclPrintf("ullResult: %llx\n", VA64BUG(ullResult));
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64MulUint16()

    Description
        Test the multiply a 16-bit value by a 64-bit value
        functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64MulUint16(void)
{
    D_BOOL              fIsEqual;
    unsigned            uIndex;
    D_UINT64            ullValue1;
    D_UINT64            ullResult;
    TESTCASE_U64U16U64  TestCase[] =
    {
        {0x00000000, 0x00000000, 0x0000, 0x00000000, 0x00000000},
        {0xffffffff, 0xffffffff, 0x0000, 0x00000000, 0x00000000},
        {0x00000000, 0x00000000, 0xffff, 0x00000000, 0x00000000},
        {0x00000000, 0x00000001, 0xffff, 0x00000000, 0x0000ffff},
        {0x00000000, 0xffffffff, 0x0001, 0x00000000, 0xffffffff},
        {0x00000000, 0x0000ffff, 0xffff, 0x00000000, 0xfffe0001},
        {0x00000000, 0xffffffff, 0xffff, 0x0000fffe, 0xffff0001},
        {0x0000ffff, 0xffffffff, 0xffff, 0xfffeffff, 0xffff0001},
        {0xffffffff, 0xffffffff, 0xffff, 0xffffffff, 0xffff0001},
        {0x67894567, 0x87542357, 0x7593, 0x308b2b4c, 0x29770df5},
    };

    /*  Validate the 64-bit multiply by 16-bit math functions/macros for each
        of the test cases.
    */
    DclPrintf("    Multiply by a 16-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);
        DclUint64AssignHiLo(&ullResult, TestCase[uIndex].ulResultHigh, TestCase[uIndex].ulResultLow);

        /*  Perform the operation and validate the result
        */
        DclUint64MulUint16(&ullValue1, TestCase[uIndex].uValue2);
        fIsEqual = DclUint64IsEqualUint64(&ullValue1, &ullResult);
        if(!fIsEqual)
        {
            DclPrintf("Error: DclUint64MulUint16 uIndex=%u", uIndex);
            DclPrintf("ullValue1: %llx\n", VA64BUG(ullValue1));
            DclPrintf("ullResult: %llx\n", VA64BUG(ullResult));
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64MulUint32()

    Description
        Test the multiply a 32-bit value by a 64-bit value
        functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64MulUint32(void)
{
    D_BOOL              fIsEqual;
    unsigned            uIndex;
    D_UINT64            ullValue1;
    D_UINT64            ullResult;
    TESTCASE_U64U32U64  TestCase[] =
    {
        {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
        {0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000},
        {0x00000000, 0x00000000, 0x0000ffff, 0x00000000, 0x00000000},
        {0x00000000, 0x00000001, 0x0000ffff, 0x00000000, 0x0000ffff},
        {0x00000000, 0xffffffff, 0x00000001, 0x00000000, 0xffffffff},
        {0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0001},
        {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000001},
        {0x84577964, 0x29474687, 0x84324891, 0xea88bda0, 0xcf92ea77},
    };

    /*  Validate the 64-bit multiply by 32-bit math functions/macros for each
        of the test cases.
    */
    DclPrintf("    Multiply by a 32-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);
        DclUint64AssignHiLo(&ullResult, TestCase[uIndex].ulResultHigh, TestCase[uIndex].ulResultLow);

        /*  Perform the operation and validate the result
        */
        DclUint64MulUint32(&ullValue1, TestCase[uIndex].ulValue2);
        fIsEqual = DclUint64IsEqualUint64(&ullValue1, &ullResult);
        if(!fIsEqual)
        {
            DclPrintf("Error: DclUint64MulUint32 uIndex=%u", uIndex);
            DclPrintf("ullValue1: %llx\n", VA64BUG(ullValue1));
            DclPrintf("ullResult: %llx\n", VA64BUG(ullResult));
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64DivUint32()

    Description
        Test the divide a 64-bit value by a 32-bit value
        functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64DivUint32(void)
{
    D_BOOL              fIsEqual;
    unsigned            uIndex;
    D_UINT64            ullValue1;
    D_UINT64            ullResult;
    TESTCASE_U64U32U64  TestCase[] =
    {
        {0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x00000000},
        {0xffffffff, 0xffffffff, 0x00000001, 0xffffffff, 0xffffffff},
        {0xffffffff, 0xffffffff, 0x000000ff, 0x01010101, 0x01010101},
        {0xffffffff, 0xffffffff, 0x0000ffff, 0x00010001, 0x00010001},
        {0xffffffff, 0xffffffff, 0x00ffffff, 0x00000100, 0x00010000},
        {0xffffffff, 0xffffffff, 0xffffffff, 0x00000001, 0x00000001},
        {0xff010101, 0x01010101, 0xffffffff, 0x00000000, 0xff010102},
        {0x84577964, 0x29474687, 0x84324891, 0x00000001, 0x00480531},
    };

    /*  Validate the 64-bit dividing by 32-bit math functions/macros for each
        of the test cases.
    */
    DclPrintf("    Divide by a 32-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);
        DclUint64AssignHiLo(&ullResult, TestCase[uIndex].ulResultHigh, TestCase[uIndex].ulResultLow);

        /*  Perform the operation and validate the result
        */
        DclUint64DivUint32(&ullValue1, TestCase[uIndex].ulValue2);
        fIsEqual = DclUint64IsEqualUint64(&ullValue1, &ullResult);
        if(!fIsEqual)
        {
            DclPrintf("Error: DclUint64DivUint32 uIndex=%u", uIndex);
            DclPrintf("ullValue1: %llx\n", VA64BUG(ullValue1));
            DclPrintf("ullResult: %llx\n", VA64BUG(ullResult));
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64ModUint32()

    Description
        Test the 64-bit mod 32-bit math functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64ModUint32(void)
{
    D_BOOL              fIsEqual;
    unsigned            uIndex;
    D_UINT64            ullValue1;
    D_UINT64            ullResult;
    TESTCASE_U64U32U64  TestCase[] =
    {
        {0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x00000000},
        {0xffffffff, 0xffffffff, 0x00000001, 0x00000000, 0x00000000},
        {0xffffffff, 0xffffffff, 0x00000002, 0x00000000, 0x00000001},
        {0xffffffff, 0xffffffff, 0x000000ff, 0x00000000, 0x00000000},
        {0xff010101, 0x01010101, 0xffffffff, 0x00000000, 0x00020203},
        {0x84577964, 0x29474687, 0x84324891, 0x00000000, 0x77748DC6},
    };

    /*  Validate the 64-bit dividing by 32-bit math functions/macros for each
        of the test cases.
    */
    DclPrintf("    Modulo by a 32-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);
        DclUint64AssignHiLo(&ullResult, TestCase[uIndex].ulResultHigh, TestCase[uIndex].ulResultLow);

        /*  Perform the operation and validate the result
        */
        DclUint64ModUint32(&ullValue1, TestCase[uIndex].ulValue2);
        fIsEqual = DclUint64IsEqualUint64(&ullValue1, &ullResult);
        if(!fIsEqual)
        {
            DclPrintf("Error: DclUint64ModUint32 uIndex=%u\n", uIndex);
            DclPrintf("ullValue1: %llx\n", VA64BUG(ullValue1));
            DclPrintf("ullResult: %llx\n", VA64BUG(ullResult));
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64GreaterUint32()

    Description
        Test the 64-bit value greater than a 32-bit value
        functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64GreaterUint32(void)
{
    unsigned        uIndex;
    D_UINT64        ullValue1;
    TESTCASE_U64U32 TestCase[] =
    {
        {0x00000000, 0x00000000, 0x00000000, FALSE},
        {0x00000000, 0x00000001, 0x00000000,  TRUE},
        {0x00000001, 0x00000000, 0x00000000,  TRUE},
        {0x00000000, 0x00000000, 0xFFFFFFFF, FALSE},
        {0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, FALSE},
        {0x00000001, 0x00000000, 0xFFFFFFFF,  TRUE},
        {0x80000000, 0x00000000, 0xFFFFFFFF,  TRUE}
    };

    /*  Iterate through each of the test cases
    */
    DclPrintf("    Greater than a 32-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);

        /*  Perform the operation and validate the result
        */
        if(DclUint64GreaterUint32(&ullValue1, TestCase[uIndex].ulValue2) != TestCase[uIndex].fResult)
        {
            DclPrintf("Error: DclUint64GreaterUint32 uIndex=%u\n", uIndex);
            DclPrintf("ullValue1: %llx\n", VA64BUG(ullValue1));
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64GreaterUint64()

    Description
        Test the 64-bit value greater than a 64-bit value
        functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64GreaterUint64(void)
{
    unsigned        uIndex;
    D_UINT64        ullValue1;
    D_UINT64        ullValue2;
    TESTCASE_U64U64 TestCase[] =
    {
        {0x00000000, 0x00000000, 0x00000000, 0x00000000, FALSE},
        {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, FALSE},

        {0x00000001, 0x00000000, 0x00000000, 0x00000000,  TRUE},
        {0x00000000, 0x00000001, 0x00000000, 0x00000000,  TRUE},
        {0x00000000, 0x00000000, 0x00000001, 0x00000000, FALSE},
        {0x00000000, 0x00000000, 0x00000000, 0x00000001, FALSE},

        {0x80000000, 0x00000000, 0x00000000, 0x00000000,  TRUE},
        {0x00000000, 0x80000000, 0x00000000, 0x00000000,  TRUE},
        {0x00000000, 0x00000000, 0x80000000, 0x00000000, FALSE},
        {0x00000000, 0x00000000, 0x00000000, 0x80000000, FALSE},

        {0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000,  TRUE},
        {0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000,  TRUE},
        {0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000, FALSE},
        {0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, FALSE},

        {0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, FALSE},
        {0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, FALSE},
        {0xFFFFFFFF, 0x00000000, 0x00000000, 0xFFFFFFFF,  TRUE},
        {0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, FALSE}
    };

    /*  Iterate through each of the test cases
    */
    DclPrintf("    Greater than a 64-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);
        DclUint64AssignHiLo(&ullValue2, TestCase[uIndex].ulVal2High, TestCase[uIndex].ulVal2Low);

        /*  Perform the operation and validate the result
        */
        if(DclUint64GreaterUint64(&ullValue1, &ullValue2) != TestCase[uIndex].fResult)
        {
            DclPrintf("Error: DclUint64GreaterUint64 uIndex=%u\n", uIndex);
            DclPrintf("ullValue1: %llx ullValue2: %llx\n", VA64BUG(ullValue1), VA64BUG(ullValue2));
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64LessUint32()

    Description
        Test the 64-bit value less than a 32-bit value
        functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64LessUint32(void)
{
    unsigned        uIndex;
    D_UINT64        ullValue1;
    TESTCASE_U64U32 TestCase[] =
    {
        {0x00000000, 0x00000000, 0x00000000, FALSE},
        {0x00000000, 0x00000001, 0x00000000, FALSE},
        {0x00000001, 0x00000000, 0x00000000, FALSE},
        {0x00000000, 0x00000000, 0x00000001,  TRUE},
        {0x00000000, 0x00000001, 0x80000000,  TRUE},
        {0x00000001, 0x00000000, 0x80000000, FALSE},
        {0x00000000, 0x00000000, 0xFFFFFFFF,  TRUE},
        {0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, FALSE},
        {0x00000001, 0x00000000, 0xFFFFFFFF, FALSE},
        {0x80000000, 0x00000000, 0xFFFFFFFF, FALSE}
    };

    /*  Iterate through each of the test cases
    */
    DclPrintf("    Less than a 32-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);

        /*  Perform the operation and validate the result
        */
        if(DclUint64LessUint32(&ullValue1, TestCase[uIndex].ulValue2) != TestCase[uIndex].fResult)
        {
            DclPrintf("Error: DclUint64LessUint32 uIndex=%u\n", uIndex);
            DclPrintf("ullValue1: %llx\n", VA64BUG(ullValue1));
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestUint64LessUint64()

    Description
        Test the 64-bit value less than a 64-bit value
        functions/macros.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestUint64LessUint64(void)
{
    unsigned        uIndex;
    D_UINT64        ullValue1;
    D_UINT64        ullValue2;
    TESTCASE_U64U64 TestCase[] =
    {
        {0x00000000, 0x00000000, 0x00000000, 0x00000000, FALSE},
        {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, FALSE},

        {0x00000001, 0x00000000, 0x00000000, 0x00000000, FALSE},
        {0x00000000, 0x00000001, 0x00000000, 0x00000000, FALSE},
        {0x00000000, 0x00000000, 0x00000001, 0x00000000,  TRUE},
        {0x00000000, 0x00000000, 0x00000000, 0x00000001,  TRUE},

        {0x80000000, 0x00000000, 0x00000000, 0x00000000, FALSE},
        {0x00000000, 0x80000000, 0x00000000, 0x00000000, FALSE},
        {0x00000000, 0x00000000, 0x80000000, 0x00000000,  TRUE},
        {0x00000000, 0x00000000, 0x00000000, 0x80000000,  TRUE},

        {0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, FALSE},
        {0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000, FALSE},
        {0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000,  TRUE},
        {0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,  TRUE},

        {0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, FALSE},
        {0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, FALSE},
        {0xFFFFFFFF, 0x00000000, 0x00000000, 0xFFFFFFFF, FALSE},
        {0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,  TRUE}
    };

    /*  Iterate through each of the test cases
    */
    DclPrintf("    Less than a 64-bit value\n");
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(TestCase); uIndex++)
    {
        /*  Set up the test case
        */
        DclUint64AssignHiLo(&ullValue1, TestCase[uIndex].ulVal1High, TestCase[uIndex].ulVal1Low);
        DclUint64AssignHiLo(&ullValue2, TestCase[uIndex].ulVal2High, TestCase[uIndex].ulVal2Low);

        /*  Perform the operation and validate the result
        */
        if(DclUint64LessUint64(&ullValue1, &ullValue2) != TestCase[uIndex].fResult)
        {
            DclPrintf("Error: DclUint64LessUint64 uIndex=%u\n", uIndex);
            DclPrintf("ullValue1: %llx ullValue2: %llx\n", VA64BUG(ullValue1), VA64BUG(ullValue2));
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}



