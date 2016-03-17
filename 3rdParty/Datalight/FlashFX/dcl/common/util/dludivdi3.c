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

    This file contains a program for doing 64/64 ==> 64 division, on a
    machine that does not have that instruction but that does have
    instructions for "long division" (64/32 ==> 32).
    These programs are useful in implementing "long long" (64-bit)
    arithmetic on a machine that has the long division instruction. It will
    work on 64- and 32-bit machines, provided the compiler implements long
    long's (64-bit integers). It is desirable that the machine have the
    Count Leading Zeros instruction.
    In the GNU world, these programs are known as __divdi3 and __udivdi3,
    and similar names are used here.

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: dludivdi3.c $
    Revision 1.9  2009/03/03 17:27:22Z  thomd
    Use ULL and LL macros so all toolsets build.
    Revision 1.8  2008/11/18 22:52:40Z  jimmb
    Changed the 'll' to unsigned ll 'ull'
    Revision 1.7  2008/11/14 18:49:43Z  jimmb
    Updated the divu to correctly reflect the appropriated division
    Revision 1.6  2008/11/05 12:40:59Z  jimmb
    Added 64 bit unsigned modulo operator
    DclUint64ModUint64
    Revision 1.5  2008/11/05 14:46:06Z  jimmb
    Conditioned 64 bit divide using DCL_NATIVE_64BIT_SUPPORT
    Revision 1.4  2008/10/31 13:39:22Z  jimmb
    Commented out for Build process
    Revision 1.3  2008/10/30 21:36:11Z  jimmb
    Replaced GNU MACRO extensions with the appropriate function
    calls.
    Revision 1.2  2008/10/30 00:39:46Z  brandont
    Disabled this module until the module dependencies are resolved.
    Revision 1.1  2008/10/27 20:23:46Z  jimmb
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

/* These routine are not applicable for non-native 64 bit support */
#if DCL_NATIVE_64BIT_SUPPORT

/* ----------------------------- nlz64 ------------------------------ */

int nlz64(D_UINT64 x)
{
    int n;

    if ( x == 0 ) return(64);
    n = 0;
    if ( x <= UINT64SUFFIX(0x00000000FFFFFFFF) )
    {
        n = n + 32; x = x << 32;
    }
    if ( x <= UINT64SUFFIX(0x0000FFFFFFFFFFFF) )
    {
        n = n + 16; x = x << 16;
    }
    if ( x <= UINT64SUFFIX(0x00FFFFFFFFFFFFFF) )
    {
        n = n +  8; x = x <<  8;
    }
    if ( x <= UINT64SUFFIX(0x0FFFFFFFFFFFFFFF) )
    {
        n = n +  4; x = x <<  4;
    }
    if ( x <= UINT64SUFFIX(0x3FFFFFFFFFFFFFFF) )
    {
        n = n +  2; x = x <<  2;
    }
    if ( x <= UINT64SUFFIX(0x7FFFFFFFFFFFFFFF) )
    {
        n = n +  1;
    }
    return( n );
}

/*  The variables u0, u1, etc. take on only 32-bit values, but they
    are declared long long to avoid some compiler warning messages and to
    avoid some unnecessary EXTRs that the compiler would put in, to
    convert long longs to ints.

    First the procedure takes care of the case in which the divisor is a
    32-bit quantity. There are two subcases: (1) If the left half of the
    dividend is less than the divisor, one execution of DIVU is all that
    is required (overflow is not possible). (2) Otherwise it does two
    divisions, using the grade school method, with variables used as
    suggested below.

    q1 q0
    ________
    v)  u1 u0
    q1*v
    ____
    k u0
*/

/*  These macros must be used with arguments of the appropriate type
    (unsigned long long for DIVU and long long for DIVS. They are
    simulations of the presumed machines ops. i.e., they look at only the
    low-order 32 bits of the divisor, they return garbage if the division
    overflows, and they return garbage in the high-order half of the
    quotient doubleword.
    In practice, these would be replaced with uses of the machine's DIVU
    and DIVS instructions (e.g., by using the GNU "asm" facility).
*/

/*
    These MACROS use gnu extenstions which are NOT portable
*/

/*
#define DIVU(u, v) ({unsigned long long __v = (v) & 0xFFFFFFFF; \
   long long __q = (u)/(__v); \
   __q > 0xFFFFFFFF ? UINT64SUFFIX(0xdeaddeadbeefbeef) : \
   __q | UINT64SUFFIX(0xdeadbeef00000000);})

#define DIVS(u, v) ({long long __v = (v) << 32 >> 32; \
   long long __q = (u)/(__v); \
   __q < (long long)INT64SUFFIX(0xFFFFFFFF80000000) || \
   __q > (long long)INT64SUFFIX(0x000000007FFFFFFF) ? \
   INT64SUFFIX(0xfadefadedeafdeaf) : __q | INT64SUFFIX(0xfadedeaf00000000);})
*/

static D_UINT64 divu( D_UINT64 u, D_UINT64 v )
{
    D_UINT64  __v = v & 0xFFFFFFFF;
    D_INT64   __q = u;

    DclUint64DivUint32(&__q, __v);

    return( (__q > 0xFFFFFFFF) ? UINT64SUFFIX(0xdeaddeadbeefbeef)
            : (__q | UINT64SUFFIX(0xdeadbeef00000000)) );

}


D_UINT64 DclUdivdi3(D_UINT64 u, D_UINT64 v)
{

    D_UINT64 u0, u1, v1, q0, q1, k, n;

    if ( v >> 32 == 0 )
    {                                                   /* If v < 2**32: */
        if ( u >> 32 < v )                              /* If u/v cannot overflow, */
            return( divu(u, v) & 0xFFFFFFFF );          /* just do one division. */
        else
        {                                               /* If u/v would overflow: */
            u1 = u >> 32;                               /* Break u up into two */
            u0 = u & 0xFFFFFFFF;                        /* halves. */
            q1 = divu(u1, v) & 0xFFFFFFFF;              /* First quotient digit. */
            k = u1 - q1*v;                              /* First remainder, < v. */
            q0 = divu((k << 32) + u0, v) & 0xFFFFFFFF;  /* 2nd quot. digit. */
            return( (q1 << 32) + q0 );
        }
    }
    /* Here v >= 2**32. */
    n = nlz64(v);                   /* 0 <= n <= 31. */
    v1 = (v << n) >> 32;            /* Normalize the divisor */
                                    /* so its MSB is 1. */
    u1 = u >> 1;                    /* To ensure no overflow. */
    q1 = divu(u1, v1) & 0xFFFFFFFF; /* Get quotient from divide unsigned insn. */
    q0 = (q1 << n) >> 31;           /* Undo normalization and */
                                    /* division of u by 2. */
    if ( q0 != 0 )                  /* Make q0 correct or */
        q0 = q0 - 1;                /* too small by 1. */
    if ( (u - q0*v) >= v )
        q0 = q0 + 1;                /* Now q0 is correct. */
    return( q0 );

}



D_UINT64 DclUmoddi3(D_UINT64 u, D_UINT64 v)
{

    D_UINT64 qu;

    qu = DclUdivdi3(u, v);

    return ( u - (qu * v) );

}


#endif

