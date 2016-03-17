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

    This module contains tests for 64-bit division and modulus operations.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltudivdi3.c $
    Revision 1.10  2009/09/15 23:20:15Z  garyp
    Fixed to use the correct macro to initialize the 64-bit values (related to Bug
    #2845).  General code cleanup.
    Revision 1.9  2009/06/28 22:48:21Z  garyp
    Updated to include dltests.h.
    Revision 1.8  2009/03/03 14:08:33Z  thomd
    Use ULL and LL macros so all toolsets build.
    Revision 1.7  2009/02/21 02:15:10Z  brandont
    Renamed Testudivdi3 to DclTestudivdi3.
    Revision 1.6  2008/11/05 21:28:28Z  jimmb
    Added 64 bit unsigned modulo operator DclUint64ModUint64.
    Revision 1.5  2008/11/05 14:42:48Z  jimmb
    Conditioned 64 bit divide using DCL_NATIVE_64BIT_SUPPORT.
    Revision 1.4  2008/10/31 13:19:29Z  jimmb
    Comment out the test for builds.
    Revision 1.3  2008/10/30 21:29:22Z  jimmb
    Replaced GNU MACRO extensions with the appropriate function calls.
    Revision 1.2  2008/10/30 00:39:45Z  brandont
    Disabled this module until the module dependencies are resolved.
    Revision 1.1  2008/10/27 20:06:16Z  jimmb
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include "dltests.h"

DCLSTATUS DclTestudivdi3(void)
{
    /*  This test is not applicable for non-native 64 bit support
    */
#if DCL_NATIVE_64BIT_SUPPORT

    /*  First test unsigned division.

        The entries in this table are used in all combinations.
    */
    static D_UINT64 tabu[] =
    {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15, 16, 1000, 2003,
        32765, 32766, 32767, 32768, 32769, 32760,
        65533, 65534, 65535, 65536, 65537, 65538,
        0x7FFFFFFE, 0x7FFFFFFF, 0x80000000, 0x80000001,
        UINT64SUFFIX(0x7000000000000000), UINT64SUFFIX(0x7000000080000000),
        UINT64SUFFIX(0x7000000080000001), UINT64SUFFIX(0x7FFFFFFFFFFFFFFF),
        UINT64SUFFIX(0x7FFFFFFF8FFFFFFF), UINT64SUFFIX(0x7FFFFFFF8FFFFFF1),
        UINT64SUFFIX(0x7FFFFFFF00000000), UINT64SUFFIX(0x7FFFFFFF80000000),
        UINT64SUFFIX(0x7FFFFFFF00000001), UINT64SUFFIX(0x8000000000000000),
        UINT64SUFFIX(0x8000000080000000), UINT64SUFFIX(0x8000000080000001),
        UINT64SUFFIX(0xC000000000000000), UINT64SUFFIX(0xC000000080000000),
        UINT64SUFFIX(0xC000000080000001), UINT64SUFFIX(0xFFFFFFFFFFFFFFFD),
        UINT64SUFFIX(0xFFFFFFFFFFFFFFFE), UINT64SUFFIX(0xFFFFFFFFFFFFFFFF)
    };

    int n, i, j, errs = 0;
    D_UINT64 uu, vu, qu, ru;

    DclPrintf("    Testing 64-bit unsigned division\n");
    n = sizeof(tabu)/sizeof(tabu[0]);
    for ( i = 0; i < n; i++ )
    {
        for ( j = 1; j < n; j++ )
        {                                       /* Skip tabu[0], which is 0. */
            uu = tabu[i];
            vu = tabu[j];
            qu = DclUint64DivUint64(&uu, &vu);  /* Call the program being tested. */
            ru = uu - qu*vu;
            if ( qu > uu || ru >= vu )
            {
                DclPrintf("        Error for %llX/%llX, got %llX rem %llX\n", uu, vu, qu, ru);
                errs = errs + 1;
            }
        }
    }
    if ( errs != 0 )
    {
        DclPrintf("        Failed %d cases (unsigned)\n", errs);
        return DCLSTAT_FAILURE;
    }

    DclPrintf("        passed all %d unsigned tests \n", n*(n-1));

    DclPrintf("    Testing 64-bit unsigned modulo\n");
    for ( i = 0; i < n; i++ )
    {
        for ( j = 1; j < n; j++ )
        {                                       /* Skip tabu[0], which is 0. */
            uu = tabu[i];
            vu = tabu[j];
            qu = DclUint64DivUint64(&uu, &vu);  /* Call the program being tested. */
            ru = uu - qu*vu;
            if ( ru != DclUint64ModUint64(&uu, &vu) )
            {
                DclPrintf("        Error for %llX/%llX, rem %llX\n", uu, vu, ru);
                errs = errs + 1;
            }
        }
    }
    if ( errs != 0 )
    {
        DclPrintf("        Failed %d cases (modulo)\n", errs);
        return DCLSTAT_FAILURE;
    }

    DclPrintf("        passed all %d modulo tests \n", n*(n-1));

#endif

    return DCLSTAT_SUCCESS;
}

