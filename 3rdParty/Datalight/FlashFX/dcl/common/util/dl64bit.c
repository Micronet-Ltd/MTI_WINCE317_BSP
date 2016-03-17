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

    This module contains 64-bit math operations which are used when the
    native environment does not support 64-bit operations, and therefore
    they must be emulated.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dl64bit.c $
    Revision 1.5  2009/03/30 03:22:19Z  garyp
    Documenation updates only -- no functional changes.
    Revision 1.4  2007/12/18 04:51:51Z  brandont
    Updated function headers.
    Revision 1.3  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.2  2006/06/01 21:16:37Z  Garyp
    Added support for U64 MOD U32.
    Revision 1.1  2005/10/02 03:57:02Z  Pauli
    Initial revision
    Revision 1.2  2005/08/03 18:39:48Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/06/21 16:33:20Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

/*  This module is only needed if native 64-bit support is disabled.
*/
#if !DCL_NATIVE_64BIT_SUPPORT


/*-------------------------------------------------------------------
    Local: AddCarryUint32()

    Internal function for this module.  This function adds two 32-bit
    values and returns TRUE if there is a carry.  This function also
    has an input parameter of a carry.

    Parameters:

    Return Value:
        Returns TRUE if there is a carry, or FALSE if not.
-------------------------------------------------------------------*/
static D_BOOL AddCarryUint32(
    D_UINT32       *pulValue1,
    D_UINT32        ulValue2,
    D_BOOL          fCarry)
{
    D_UINT32        ulOrgValue1;

    /*  Save the orginal value to check for carry
    */
    ulOrgValue1 = *pulValue1;

    /*  Handle passed in carry operation first
    */
    if(fCarry)
    {
        *pulValue1 += 1;
    }

    /*  Perform the addition
    */
    *pulValue1 += ulValue2;

    /*  Return whether there is a carry
    */
    return ((*pulValue1 < ulValue2) && (*pulValue1 < ulOrgValue1));
}


/*-------------------------------------------------------------------
    Local: SubBorrowUint32()

    Internal function for this module.  This function subtracts two
    32-bit values and returns TRUE if there is a borrow.  This
    function also has an input parameter of a borrow.

    Parameters:

    Return Value:
        Returns TRUE if there is a borrow, and FALSE if not.
-------------------------------------------------------------------*/
static D_BOOL SubBorrowUint32(
    D_UINT32       *pulValue1,
    D_UINT32        ulValue2,
    D_BOOL          fBorrow)
{
    D_UINT32        ulOrgValue1;

    /*  Save the orginal value to check for borrow
    */
    ulOrgValue1 = *pulValue1;

    /*  Handle passed in borrow operation first
    */
    if(fBorrow)
    {
        *pulValue1 -= 1;
    }

    /*  Perform the subtraction
    */
    *pulValue1 -= ulValue2;

    /*  Return whether there is a borrow
    */
    return (*pulValue1 > ulOrgValue1);
}


/*-------------------------------------------------------------------
    Public: DclUint64AddUint16()

    Add a 16-bit value to a 64-bit value.

    Parameters:
        pullValue64 - A pointer to the D_UINT64 target value.
        uValue16    - The 16-bit value to add.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclUint64AddUint16(
    D_UINT64       *pullValue64,
    D_UINT16        uValue16)
{
    DclUint64AddUint32(pullValue64, uValue16);
}


/*-------------------------------------------------------------------
    Public: DclUint64AddUint32()

    Add a 32-bit value to a 64-bit value.

    Parameters:
        pullValue64 - A pointer to the D_UINT64 target value.
        ulValue32   - The 32-bit value to add.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclUint64AddUint32(
    D_UINT64       *pullValue64,
    D_UINT32        ulValue32)
{
    D_UINT64        ullTemp;

    DclUint64AssignUint32(&ullTemp, ulValue32);
    DclUint64AddUint64(pullValue64, &ullTemp);
}


/*-------------------------------------------------------------------
    Public: DclUint64AddUint64()

    Add a 64-bit value to a 64-bit value.

    Parameters:
        pullValue1  - A pointer to the D_UINT64 target value.
        pullValue2  - A pointer to the D_UINT64 target value to add.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclUint64AddUint64(
    D_UINT64       *pullValue1,
    D_UINT64       *pullValue2)
{
    D_BOOL          fHasCarry = FALSE;

    fHasCarry = AddCarryUint32(&pullValue1->ulLowDword, pullValue2->ulLowDword, fHasCarry);
    fHasCarry = AddCarryUint32(&pullValue1->ulHighDword, pullValue2->ulHighDword, fHasCarry);
}


/*-------------------------------------------------------------------
    Public: DclUint64SubUint16()

    Subtract a 16-bit value from a 64-bit value.

    Parameters:
        pullValue64 - A pointer to the D_UINT64 target value.
        uValue16    - The 16-bit value to subtract.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclUint64SubUint16(
    D_UINT64       *pullValue64,
    D_UINT16        uValue16)
{
    DclUint64SubUint32(pullValue64, uValue16);
}


/*-------------------------------------------------------------------
    Public: DclUint64SubUint32()

    Subtract a 32-bit value from a 64-bit value.

    Parameters:
        pullValue64 - A pointer to the D_UINT64 target value.
        ulValue32   - The 32-bit value to subtract.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclUint64SubUint32(
    D_UINT64        *pullValue64,
    D_UINT32        ulValue32)
{
    D_UINT64        ullTemp;

    DclUint64AssignUint32(&ullTemp, ulValue32);
    DclUint64SubUint64(pullValue64, &ullTemp);
}


/*-------------------------------------------------------------------
    Public: DclUint64SubUint64()

    Subtract a 64-bit value from a 64-bit value.

    Parameters:
        pullValue1  - A pointer to the D_UINT64 target value.
        pullValue2  - A pointer to the D_UINT64 target value to subtract.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclUint64SubUint64(
    D_UINT64       *pullValue1,
    D_UINT64       *pullValue2)
{
    D_BOOL          fHasBorrow = FALSE;

    fHasBorrow = SubBorrowUint32(&pullValue1->ulLowDword, pullValue2->ulLowDword, fHasBorrow);
    fHasBorrow = SubBorrowUint32(&pullValue1->ulHighDword, pullValue2->ulHighDword, fHasBorrow);
}


/*-------------------------------------------------------------------
    Public: DclUint64MulUint16()

    Multiply a 64-bit value by a 16-bit value.

    Parameters:
        pullValue64 - A pointer to the 64-bit target operand.
        uValue16    - The value to multiply by.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclUint64MulUint16(
    D_UINT64       *pullValue64,
    D_UINT16        uValue16)
{
    DclUint64MulUint32(pullValue64, uValue16);
}


/*-------------------------------------------------------------------
    Local: Assign64AtWordShiftCount()

    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
static void Assign64AtWordShiftCount(
    D_UINT64       *pullValue64,
    D_UINT32        ulValue32,
    D_UINT16        uWordCount)
{
    switch (uWordCount)
    {
        case 0:
            pullValue64->ulLowDword = ulValue32;
            pullValue64->ulHighDword = 0;
            break;

        case 1:
            pullValue64->ulLowDword = (ulValue32 & 0x0000ffffUL) << 16;
            pullValue64->ulHighDword = (ulValue32 & 0xffff0000UL) >> 16;
            break;

        case 2:
            pullValue64->ulLowDword = 0;
            pullValue64->ulHighDword = ulValue32;
            break;

        case 3:
            pullValue64->ulLowDword = 0;
            pullValue64->ulHighDword = (ulValue32 & 0x0000ffffUL) << 16;
            break;

        default:
            pullValue64->ulLowDword = 0;
            pullValue64->ulHighDword = 0;
            break;
    }
}


/*-------------------------------------------------------------------
    Public: DclUint64MulUint32()

    Multiply a 64-bit value by a 32-bit value.

    Parameters:
        pullValue64 - A pointer to the 64-bit target operand.
        uValue32    - The value to multiply by.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclUint64MulUint32(
    D_UINT64       *pullValue64,
    D_UINT32        ulValue32)
{
    D_UINT32        ulFactor1A;
    D_UINT32        ulFactor1B;
    D_UINT32        ulFactor1C;
    D_UINT32        ulFactor1D;
    D_UINT32        ulFactor2A;
    D_UINT32        ulFactor2B;
    D_UINT64        ullIntermediate;

    /*  Implementation overview

        {1A}{1B}{1C}{1D}
        x        {2A}{2B}
        -----------------       Shift count in 16-bit words
        {2B}*{1D} << 0
        {2B}*{1C} << 1
        {2B}*{1B} << 2
        {2B}*{1A} << 3
        {2A}*{1D} << 1
        {2A}*{1C} << 2
        {2A}*{1B} << 3
        {2A}*{1A} << 4
        -----------------
    */

    /*  Get all the factors
    */
    ulFactor1A = (pullValue64->ulHighDword & 0xFFFF0000UL) >> 16;
    ulFactor1B = pullValue64->ulHighDword & 0x0000FFFFUL;
    ulFactor1C = (pullValue64->ulLowDword & 0xFFFF0000UL) >> 16;
    ulFactor1D = pullValue64->ulLowDword & 0x0000FFFFUL;
    ulFactor2A = (ulValue32 & 0xFFFF0000UL) >> 16;
    ulFactor2B = ulValue32 & 0x0000FFFFUL;

    /*  Initialize the return value to zero
    */
    DclUint64AssignUint16(pullValue64, 0);

    /*  Perform the multiplication of two unsigned longs
        (the quad word addition is required to catch overflows!)
    */
    Assign64AtWordShiftCount(pullValue64, ulFactor2B * ulFactor1D, 0);
    Assign64AtWordShiftCount(&ullIntermediate, ulFactor2B * ulFactor1C, 1);
    DclUint64AddUint64(pullValue64, &ullIntermediate);
    Assign64AtWordShiftCount(&ullIntermediate, ulFactor2B * ulFactor1B, 2);
    DclUint64AddUint64(pullValue64, &ullIntermediate);
    Assign64AtWordShiftCount(&ullIntermediate, ulFactor2B * ulFactor1A, 3);
    DclUint64AddUint64(pullValue64, &ullIntermediate);
    Assign64AtWordShiftCount(&ullIntermediate, ulFactor2A * ulFactor1D, 1);
    DclUint64AddUint64(pullValue64, &ullIntermediate);
    Assign64AtWordShiftCount(&ullIntermediate, ulFactor2A * ulFactor1C, 2);
    DclUint64AddUint64(pullValue64, &ullIntermediate);
    Assign64AtWordShiftCount(&ullIntermediate, ulFactor2A * ulFactor1B, 3);
    DclUint64AddUint64(pullValue64, &ullIntermediate);
    Assign64AtWordShiftCount(&ullIntermediate, ulFactor2A * ulFactor1A, 4);
    DclUint64AddUint64(pullValue64, &ullIntermediate);
}


/*-------------------------------------------------------------------
    Public: DclUint64DivUint32()

    Divide a 64-bit value by a 32-bit value.

    Parameters:
        pullValue64 - A pointer to the 64-bit target operand.
        ulValue32   - The 32-bit divisor.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclUint64DivUint32(
    D_UINT64       *pullValue64,
    D_UINT32        ulValue32)
{
    D_UINT64        ullResult;
    D_UINT32        ulRemainder;
    D_UINT16        uIndex;
    D_UINT32        ulThisDivision;
    D_UINT32        ulMask;
    D_UINT8         ucNextValue;
    D_UINT64        ulInterum64;


    /*  Check for divide by zero
    */
    if(ulValue32 == 0)
    {
        pullValue64->ulHighDword = 0xffffffffUL;
        pullValue64->ulLowDword = 0xffffffffUL;
        return;
    }


    /*  Compute the high part and get the remainder
    */
    ullResult.ulHighDword = pullValue64->ulHighDword / ulValue32;
    ullResult.ulLowDword = 0;
    ulRemainder = pullValue64->ulHighDword % ulValue32;


    /*  Compute the low part
    */
    ulMask = 0xff000000UL;
    for(uIndex = 0; uIndex < sizeof(D_UINT32); uIndex++)
    {
        ucNextValue = (D_UINT8) ((pullValue64->ulLowDword & ulMask) >>
                                  ((sizeof(D_UINT32) - 1 - uIndex) * 8));
        ulInterum64.ulHighDword = ulRemainder >> 24;
        ulInterum64.ulLowDword = (ulRemainder << 8) | ucNextValue;
        ulThisDivision = 0;
        while(ulInterum64.ulHighDword != 0)
        {
            DclUint64SubUint32(&ulInterum64, ulValue32);
            ulThisDivision++;
        }
        ulThisDivision += ulInterum64.ulLowDword / ulValue32;
        ulRemainder = ulInterum64.ulLowDword % ulValue32;
        ullResult.ulLowDword <<= 8;
        ullResult.ulLowDword += ulThisDivision;
        ulMask >>= 8;
    }


    /*  Set the return value
    */
    DclUint64AssignUint64(pullValue64, &ullResult);
}


/*-------------------------------------------------------------------
    Public: DclUint64ModUint32()

    Calculate the modulus of a 64-bit number and a 32-bit number.

    Parameters:
        pullValue64 - A pointer to the 64-bit value
        ulValue32   - The 32-bit value

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclUint64ModUint32(
    D_UINT64       *pullValue64,
    D_UINT32        ulValue32)
{
    D_UINT64        ullTemp;

    DclAssert(pullValue64);
    DclAssert(ulValue32);

    ullTemp = *pullValue64;

    /*  This is a really inefficient, but quick way to get this number
    */
    DclUint64DivUint32(&ullTemp, ulValue32);
    DclUint64MulUint32(&ullTemp, ulValue32);

    /*  Subtrace the recalculated number from the original to get the
        modulus value.
    */
    DclUint64SubUint64(pullValue64, &ullTemp);

    return;
}

#endif
