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

    This module contains test code which demonstrates compiler bugs or
    anomalies.

    This code is divided into two sections, one containing code that
    causes compile-time errors, and one containing code which causes
    run-time errors.

    The compile-time error section is disabled by default, since the
    module needs to be able to compile.  To excercise this code, it must
    be manually enabled.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltcompiler.c $
    Revision 1.4  2009/06/24 22:45:58Z  garyp
    Updated to include dltests.h.
    Revision 1.3  2009/02/21 04:16:10Z  brandont
    Added static declaration to local functions.
    Revision 1.2  2007/11/03 23:31:12Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/05/13 17:28:24Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include "dltests.h"

/*  Enable this at your peril.  Depending on what compiler
    is being used you may not get far...
*/
#define COMPILE_TIME_BUGS_ENABLED   TRUE        /* FALSE for checkin!  */
#define WILL_NOT_COMPILE           FALSE        /* Set to FALSE to choose code which WILL compile */


static DCLSTATUS TestCompileTimeBugs(void);
static DCLSTATUS TestRunTimeBugs(void);


/*-------------------------------------------------------------------
    DclTestCompilerBugs()

    Description
        Test for known compiler bugs and anomalies, both compile-
        time and run-time.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclTestCompilerBugs(void)
{
    DclPrintf("Testing Compiler Bugs and Anomalies\n");

    TestCompileTimeBugs();

    return TestRunTimeBugs();
}


/*-------------------------------------------------------------------
    TestCompileTimeBugs()

    Description

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestCompileTimeBugs(void)
{
  #if !COMPILE_TIME_BUGS_ENABLED

    DclPrintf("    Testing of compile-time bugs is disabled\n");
    return DCLSTAT_SUCCESS;

  #else

    DclPrintf("    Testing compile-time bugs\n");

    /**********************************************************\
     *  MSVC6 apparently requires that the result returned    *
     *  by a comparison operation is of the type "int", and   *
     *  comparing that against an "unsigned" type will result *
     *  in a "Warning C4018: '!=' signed/unsigned mismatch"   *
     *  warning.                                              *
     *                                                        *
     *  This does not occur with MSVC8 (Visual Studio 2005)   *
    \**********************************************************/
    DclPrintf("      MSVC6 comparison result type\n");
    {
      #if WILL_NOT_COMPILE
        unsigned    fResult = FALSE;
      #else
        int         fResult = FALSE;
      #endif
        unsigned    aa = 1;
        unsigned    bb = 2;

        if((aa < bb) != fResult)
        {
            DclPrintf("        Compile and run OK\n");
        }
        else
        {
            DclPrintf("        Compile OK but run-time failed!\n");
            return DCLSTAT_FAILURE;
        }
    }

    return DCLSTAT_SUCCESS;

  #endif
}


/*-------------------------------------------------------------------
    TestRunTimeBugs()

    Description

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestRunTimeBugs(void)
{
    DclPrintf("    Testing run-time bugs\n");

    return DCLSTAT_SUCCESS;
}
