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

    This module contains unit tests for path operations.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltpath.c $
    Revision 1.3  2009/06/27 22:31:01Z  garyp
    Updated to include dltests.h.
    Revision 1.2  2007/11/03 23:31:12Z  Garyp
    Added the standard module header.
    Revision 1.1  2006/03/16 02:33:32Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include "dltests.h"


typedef struct _path_test_case
{
    char       *szTest;
    char       *szCWD;
    char       *szExpect;
    D_BOOL      fResult;
} PATH_TEST_CASE;


DCLSTATUS DclTestPath(
    void)
{
    D_BOOL          fResult;
    D_BOOL          fPassed = TRUE;
    char            szAbs[128];
    char            szSeparators[] = "/\\";
    PATH_TEST_CASE *pTest;
    PATH_TEST_CASE  asTests[] = {
                        {"..", "/", NULL, FALSE},
                        {"./..", "/", NULL, FALSE},
                        {"/..", "/", NULL, FALSE},
                        {".", "/", "/", TRUE},
                        {"./", "/", "/", TRUE},
                        {"/.", "/", "/", TRUE},
                        {".", NULL, "/", TRUE},
                        {"./", NULL, "/", TRUE},
                        {"/.", NULL, "/", TRUE},
                        {"test/./../test/dir", "/", "/test/dir", TRUE},
                        {"test/./../../test/dir", "/", NULL, FALSE},
                        {"test/./../test/dir/..", "/", "/test", TRUE},
                        {"test/./../test/dir/", "/", "/test/dir", TRUE},
                        {"test/./../test/dir/.", "/", "/test/dir", TRUE},
                        {"test/./../test/.dir", "/", "/test/.dir", TRUE},
                        {"test/./../test/.dir.", "/", "/test/.dir.", TRUE},
                        {"test/./../test/..dir.", "/", "/test/..dir.", TRUE},
                        {"test/./../test/..dir..", "/", "/test/..dir..", TRUE},
                        {"../test/dir", "/test", "/test/dir", TRUE},
                        {"../../test/dir", "/test", NULL, FALSE},
                        {"../..", "/test/dir", "/", TRUE} };
    D_UINT16        uTestCases = DCLDIMENSIONOF(asTests);
    D_UINT16        uIndex;


    DclPrintf("Testing path utilities...\n");

    for(uIndex = 0; uIndex < uTestCases; uIndex++)
    {
        pTest = &asTests[uIndex];
        fResult = DclAbsolutePath(pTest->szTest, pTest->szCWD, szSeparators, szAbs);
        if(fResult != pTest->fResult)
        {
            fPassed = FALSE;
        }
        if(pTest->fResult && (DclStrICmp(szAbs, pTest->szExpect) != 0))
        {
            fPassed = FALSE;
        }
        if(!fPassed)
        {
            DclPrintf("Test %u Failed\n", uIndex);
            return DCLSTAT_CURRENTLINE;
        }
    }

    /*  All tests pass.
    */
    DclPrintf("    OK\n");
    return DCLSTAT_SUCCESS;
}
