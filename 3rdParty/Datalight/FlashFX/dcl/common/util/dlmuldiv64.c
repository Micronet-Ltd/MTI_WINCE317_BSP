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

    This module contains functions for performing 64-bit multiply and
    division.

    ToDo: Implement some 128-bit math functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlmuldiv64.c $
    Revision 1.5  2010/08/04 00:10:31Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.4  2009/10/09 17:47:55Z  garyp
    Corrected comments -- no functional changes.
    Revision 1.3  2009/10/01 16:00:22Z  garyp
    Fixed to build cleanly when output is disabled.
    Revision 1.2  2009/10/01 01:34:37Z  garyp
    Updated to build cleanly.
    Revision 1.1  2009/09/30 02:43:04Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclMulDiv64()

    Multiply 64-bit and 32-bit numbers, and divide by a 64-bit number,
    returning a 64-bit result.

    *Warning* -- This function may return an approximate value if
                 multiplying ullBase and ulMultplier results in a
                 number larger than 64-bits <and> this cannot be
                 avoided by scaling.

    Parameters:
        ullBase      - The base 64-bit number number.
        ulMultiplier - The 32-bit number by which to multiply.
        ullDivisor   - The 64-bit number by which to divide.

    Return Value:
        Returns the 64-bit unsigned integer result.  Always returns
        zero if either ullBase or ulMultiplier are zero (regardless
        what ullDivisor is).  Returns D_UINT64_MAX if an overflow
        condition occurred, or if ullDivisor is zero.
-------------------------------------------------------------------*/
D_UINT64 DclMulDiv64(
    D_UINT64    ullBase,
    D_UINT32    ulMultiplier,
    D_UINT64    ullDivisor)
{
    D_UINT64    ullTemp;
  #if D_DEBUG && DCLCONF_OUTPUT_ENABLED
    D_UINT64    ullOriginalBase = ullBase;
    D_UINT32    ullOriginalMultiplier = ulMultiplier;
    D_UINT64    ullOriginalDivisor = ullDivisor;
  #endif

    /*  Result would always be zero if either of these are zero.
        Specifically test this case before looking for a zero
        divisor.
    */
    if(!ullBase || !ulMultiplier)
        return 0;

    if(!ullDivisor)
    {
        DCLPRINTF(1, ("DclMulDiv32(%llU %lU %llU) Warning! attempted divide-by-zero\n",
            VA64BUG(ullBase), ulMultiplier, VA64BUG(ullDivisor)));

        return D_UINT64_MAX;
    }

    /*  Since we don't have the ability (yet) to use 128-bit numbers, we
        jump through the following hoops (in order) to try to determine
        the proper results without losing precision:

        1) Shift the divisor and one of the multiplicands as many times
           as is necessary to reduce the scale -- only if it can be done
           without losing precision.
        2) Divide one of the multiplicands by the divisor first, but only
           if it divides evenly, preserving precision.
        3) Same as #2, but try it for the other multiplicand.
        4) Last ditch, divide the larger multiplicand by the divisor first,
           then do the multiply.  This <WILL> lose precision.

        These solutions are identified as CODE-PATHs #1-4 which are used
        to identify the matching tests in dltmain.c.

        Note that execution might partially include CODE-PATH #1 up until
        shifting can no longer be done without losing precision.  In that
        case, one of the three remaining options will be used.
    */

    ullTemp = D_UINT64_MAX;
    DclUint64DivUint32(&ullTemp, ulMultiplier);
    while(ullBase > ullTemp)
    {
        D_UINT64    ullMod;

        /*  CODE-PATH #1
        */
        /*  So long as ulDivisor, and at least one of the other numbers,
            are evenly divisible by 2, we can scale the numbers so the
            result does not overflow the intermediate 64-bit value.
        */
        if(!(ullDivisor & 1))
        {
            if(!(ullBase & 1))
            {
                /*  CODE-PATH #1a
                */
                ullDivisor >>= 1;
                ullBase >>= 1;
                continue;
            }
            else if(!(ulMultiplier & 1) && !(ullTemp & UINT64SUFFIX(0x8000000000000000)))
            {
                /*  CODE-PATH #1b
                */
                ullDivisor >>= 1;
                ulMultiplier >>= 1;
                ullTemp <<= 1;
                continue;
            }
        }

        /*  If we get to this point, the above method (#1) cannot be used
            because not enough of the numbers are even long enough to scale
            the operands down.  We'll see if either multiplicand is evenly
            divisble by ulDivisor, and if so, do the divide first, then the
            multiply.  (Note that once we get to this point, we will never
            exercise the while{} loop anymore.)
        */

        /*  CODE-PATH #2
        */
        ullMod = DclUint64ModUint64(&ullBase, &ullDivisor);
        if(ullMod == 0)
        {
            /*  Evenly divides, so check that we won't overflow, and finish up.
            */
            ullBase = DclUint64DivUint64(&ullBase, &ullDivisor);
            if(ullBase > ullTemp)
            {
                DCLPRINTF(1, ("DclMulDiv32(%llU %llU %llU) Warning! would overflow (#2)\n",
                    VA64BUG(ullOriginalBase), VA64BUG(ullOriginalMultiplier), VA64BUG(ullOriginalDivisor)));

                return D_UINT64_MAX;
            }
            else
            {
                /*  We've validated that this will not overflow.
                */
                DclUint64MulUint32(&ullBase, ulMultiplier);
                return ullBase;
            }
        }

        /*  CODE-PATH #3
        */
        ullMod = DclUint64ModUint64(&ulMultiplier, &ullDivisor);
        if(ullMod == 0)
        {
            D_UINT64 ullWideMultiplier = ulMultiplier; /* 32 to 64 bit conversion */

            /*  Evenly divides, so check that we won't overflow, and
                finish up.
            */
            ullWideMultiplier = DclUint64DivUint64(&ullWideMultiplier, &ullDivisor);

            /*  Must recalculate ullTemp relative to ullBase
            */
            ullTemp = D_UINT64_MAX;
            ullTemp = DclUint64DivUint64(&ullTemp, &ullBase);

            if(ullWideMultiplier > ullTemp)
            {
                DCLPRINTF(1, ("DclMulDiv32(%llU %llU %llU) Warning! would overflow (#3)\n",
                    VA64BUG(ullOriginalBase), VA64BUG(ullOriginalMultiplier), VA64BUG(ullOriginalDivisor)));

                return D_UINT64_MAX;
            }
            else
            {
                D_UINT32    ulNarrowMultiplier = (D_UINT32)ullWideMultiplier;

                /*  We've validated that this will not overflow.
                */
                DclUint64MulUint32(&ullBase, ulNarrowMultiplier);
                return ullBase;
            }
        }

        /*  CODE-PATH #4

            Neither of the multipliers is evenly divisible by the divisor,
            so just punt and divide the larger number first, then do the
            final multiply.

            All the other attempts above would preserve precision -- this
            is the only case where precision may be lost.
        */

        /*  If necessary reverse the ullBase and ulMultiplier operands
            so that ullBase contains the larger of the two values.
        */
        if(ullBase < ulMultiplier)
        {
            D_UINT32    ulTemp = ulMultiplier;

            ulMultiplier = (D_UINT32)ullBase;
            ullBase = ulTemp;
        }

        ullBase = DclUint64DivUint64(&ullBase, &ullDivisor);

        ullTemp = D_UINT64_MAX;
        DclUint64DivUint32(&ullTemp, ulMultiplier);
        if(ullBase > ullTemp)
        {
            DCLPRINTF(1, ("DclMulDiv32(%llU %llU %llU) Warning! would overflow (#4)\n",
                VA64BUG(ullOriginalBase), VA64BUG(ullOriginalMultiplier), VA64BUG(ullOriginalDivisor)));

            return D_UINT64_MAX;
        }
        else
        {
            DclUint64MulUint32(&ullBase, ulMultiplier);
            return ullBase;
        }
    }

    /*  We only get to this point if either there was never any chance of
        overflow, or if the pure shifting mechanism succeeded in reducing
        the scale so overflow is not a problem.
    */

    DclUint64MulUint32(&ullBase, ulMultiplier);
    ullBase = DclUint64DivUint64(&ullBase, &ullDivisor);

    return ullBase;
}

