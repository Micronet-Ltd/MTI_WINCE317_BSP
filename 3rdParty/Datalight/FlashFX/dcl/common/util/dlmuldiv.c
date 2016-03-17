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

    This module contains functions for performing multiply and division.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlmuldiv.c $
    Revision 1.2  2009/09/30 21:29:59Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.1  2009/09/15 23:44:24Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclMulDiv()

    Multiply two 32-bit numbers and divide by another 32-bit number,
    returning a 64-bit result.

    Parameters:
        ulBase       - The base number.
        ulMultiplier - The number by which to multiply.
        ulDivisor    - The number by which to divide.

    Return Value:
        Returns the 64-bit unsigned integer result.  Always returns
        zero if either ulBase or ulMultiplier are zero (regardless
        what ulDivisor is).  Returns D_UINT64_MAX if if ullDivisor
        is zero, with the other operands being non-zero.
-------------------------------------------------------------------*/
D_UINT64 DclMulDiv(
    D_UINT32    ulBase,
    D_UINT32    ulMultiplier,
    D_UINT32    ulDivisor)
{
    D_UINT64    ullTemp;

    /*  Result would always be zero if either of these are zero.
        Specifically test this case before looking for a zero
        divisor.
    */
    if(!ulBase || !ulMultiplier)
        return 0;

    if(!ulDivisor)
    {
        DCLPRINTF(1, ("DclMulDiv(%lU %lU %lU) Warning! attempted divide-by-zero\n",
            ulBase, ulMultiplier, ulDivisor));

        return D_UINT64_MAX;
    }

    ullTemp = ulBase;

    DclUint64MulUint32(&ullTemp, ulMultiplier);
    DclUint64DivUint32(&ullTemp, ulDivisor);

    return ullTemp;
}

