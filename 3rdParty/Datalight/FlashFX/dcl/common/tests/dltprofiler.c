/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module contains code for testing the profiler.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltprofiler.c $
    Revision 1.4  2011/08/16 19:26:01Z  daniel.lewis
    Eliminated use of the non-standard __FUNCTION__ macro.
    Revision 1.3  2010/04/27 23:42:53Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.2  2010/04/17 22:29:53Z  garyp
    Updated the profiler summary command to support an "AdjustOverhead"
    flag.
    Revision 1.1  2009/11/18 01:48:18Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include "dltests.h"

static void         DclTestProf_NoArgsNoLocals_Level_1(void);
static void         DclTestProf_NoArgsNoLocals_Level_2(void);
static void         DclTestProf_NoArgsNoLocals_Level_3(void);
static void         DclTestProf_NoArgsNoLocals_Level_4(void);
static void         DclTestProf_NoArgsNoLocals_Level_5(void);
static D_UINT32     DclTestProf_Args4NoLocals_Level_1(D_UINT32 ulTest);
static D_UINT32     DclTestProf_Args4NoLocals_Level_2(D_UINT32 ulTest);
static D_UINT32     DclTestProf_Args4NoLocals_Level_3(D_UINT32 ulTest);
static D_UINT32     DclTestProf_Locals_UINT32_NoInit(void);
static D_UINT32     DclTestProf_Locals_UINT32_Init(void);
static char         DclTestProf_Locals_BUFFER32(void);
static void         DclTestProf_Locals_ScopeAtFunction(void);
static void         DclTestProf_Locals_ScopeAtClause(void);
static void         DclTestProf_MissingLeave_Level_1(void);
static void         DclTestProf_MissingLeave_Level_2(void);


/*-------------------------------------------------------------------
    Protected: DclTestProfiler()

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclTestProfiler(void)
{
    DCLSTATUS   dclStat;
    D_UINT32    ulTest = 0;
    D_UINT32    ulTest2;
    D_UINT32    ulTest3;
    char        chr;

    DclPrintf("Testing the Profiler...\n");

    dclStat = DclProfEnable();
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("    Attempting to enable the profiler returned status %lX\n", dclStat);
        DclPrintf("    FAILED\n");
        return dclStat;
    }

    DclProfilerEnter("DclTestProf_Master", 0, 0);

    DclTestProf_NoArgsNoLocals_Level_1();

    ulTest = DclTestProf_Args4NoLocals_Level_1(ulTest);

    ulTest2 = DclTestProf_Locals_UINT32_NoInit();
    ulTest3 = DclTestProf_Locals_UINT32_Init();
    chr = DclTestProf_Locals_BUFFER32();

    (void)ulTest2;
    (void)ulTest3;
    (void)chr;

    /*  The following two functions do exactly the same thing, the first
        with locals declared at "function scope", and the second with
        locals declared at "clause scope".  Profiler instrumentation
        records stack depth at various points.
    */
    DclTestProf_Locals_ScopeAtFunction();
    DclTestProf_Locals_ScopeAtClause();

    DclTestProf_MissingLeave_Level_1();

    DclProfilerLeave(ulTest);

    dclStat = DclProfSummary(TRUE, FALSE, TRUE);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("    Attempting to display the profiler summary returned status %lX\n", dclStat);
        DclPrintf("    FAILED\n");
        return dclStat;
    }

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    NoArgsNoLocals call tree
-------------------------------------------------------------------*/
static void DclTestProf_NoArgsNoLocals_Level_1(void)
{
    DclProfilerEnter("DclTestProf_NoArgsNoLocals_Level_1", 0, 0);

    DclTestProf_NoArgsNoLocals_Level_2();

    DclProfilerLeave(0);
    return;
}

static void DclTestProf_NoArgsNoLocals_Level_2(void)
{
    DclProfilerEnter("DclTestProf_NoArgsNoLocals_Level_2", 0, 0);

    DclTestProf_NoArgsNoLocals_Level_3();

    DclProfilerLeave(0);
    return;
}

static void DclTestProf_NoArgsNoLocals_Level_3(void)
{
    DclProfilerEnter("DclTestProf_NoArgsNoLocals_Level_3", 0, 0);

    DclTestProf_NoArgsNoLocals_Level_4();

    DclProfilerLeave(0);
    return;
}


static void DclTestProf_NoArgsNoLocals_Level_4(void)
{
    DclProfilerEnter("DclTestProf_NoArgsNoLocals_Level_4", 0, 0);

    DclTestProf_NoArgsNoLocals_Level_5();

    DclProfilerLeave(0);
    return;
}

static void DclTestProf_NoArgsNoLocals_Level_5(void)
{
    DclProfilerEnter("DclTestProf_NoArgsNoLocals_Level_5", 0, 0);

    DclProfilerLeave(0);
    return;
}


/*-------------------------------------------------------------------
    Args4NoLocals call tree
-------------------------------------------------------------------*/
static D_UINT32 DclTestProf_Args4NoLocals_Level_1(D_UINT32 ulTest)
{
    DclProfilerEnter("DclTestProf_Args4NoLocals_Level_1", 0, 0);

    DclTestProf_Args4NoLocals_Level_2(ulTest);

    DclProfilerLeave((D_UINT32)&ulTest);
    return ulTest;
}

static D_UINT32 DclTestProf_Args4NoLocals_Level_2(D_UINT32 ulTest)
{
    DclProfilerEnter("DclTestProf_Args4NoLocals_Level_2", 0, 0);

    DclTestProf_Args4NoLocals_Level_3(ulTest);

    DclProfilerLeave((D_UINT32)&ulTest);
    return ulTest;
}

static D_UINT32 DclTestProf_Args4NoLocals_Level_3(D_UINT32 ulTest)
{
    DclProfilerEnter("DclTestProf_Args4NoLocals_Level_3", 0, 0);

    DclProfilerLeave((D_UINT32)&ulTest);
    return ulTest;
}


/*-------------------------------------------------------------------
    NoArgs Misc Locals call tree
-------------------------------------------------------------------*/
static D_UINT32 DclTestProf_Locals_UINT32_NoInit(void)
{
    D_UINT32    ulTest;

    DclProfilerEnter("DclTestProf_Locals_UINT32_NoInit", 0, 0);

    ulTest = 4356;

    DclProfilerLeave((D_UINT32)&ulTest);
    return ulTest;
}

static D_UINT32 DclTestProf_Locals_UINT32_Init(void)
{
    D_UINT32    ulTest = 1234;

    DclProfilerEnter("DclTestProf_Locals_UINT32_Init", 0, 0);

    DclProfilerLeave((D_UINT32)&ulTest);
    return ulTest;
}

static char DclTestProf_Locals_BUFFER32(void)
{
    char        achBuff[32] = {0};

    DclProfilerEnter("DclTestProf_Locals_BUFFER32", 0, 0);

    DclMemSet(achBuff, 1, sizeof(achBuff));

    DclProfilerLeave((D_UINT32)&achBuff[0]);
    return achBuff[0];
}


static void DclTestProf_Locals_ScopeAtFunction(void)
{
    D_BUFFER    abBuff1[10];
    D_BUFFER    abBuff2[100];
    D_BUFFER    abBuff3[200];

    DclProfilerEnter("DclTestProf_Locals_ScopeAtFunction", 0, 0);

    {
        DclMemSet(abBuff1, 0, sizeof(abBuff1));
        DclProfilerRecord("ScopeFunc: 10 byte local buffer", 0, 0);
        DclMemSet(abBuff1, 1, sizeof(abBuff1));
    }

    {
        DclMemSet(abBuff2, 0, sizeof(abBuff2));
        DclProfilerRecord("ScopeFunc: 100 byte local buffer", 0, 0);
        DclMemSet(abBuff2, 1, sizeof(abBuff2));
    }

    {
        DclMemSet(abBuff3, 0, sizeof(abBuff3));
        DclProfilerRecord("ScopeFunc: 200 byte local buffer", 0, 0);
        DclMemSet(abBuff3, 1, sizeof(abBuff3));
    }

    DclProfilerLeave(0);
    return;
}


static void DclTestProf_Locals_ScopeAtClause(void)
{

    DclProfilerEnter("DclTestProf_Locals_ScopeAtClause", 0, 0);

    {
        D_BUFFER    abBuff1[10];
        DclMemSet(abBuff1, 0, sizeof(abBuff1));
        DclProfilerRecord("ScopeClause: 10 byte local buffer", 0, 0);
        DclMemSet(abBuff1, 1, sizeof(abBuff1));
    }

    {
        D_BUFFER    abBuff2[100];
        DclMemSet(abBuff2, 0, sizeof(abBuff2));
        DclProfilerRecord("ScopeClause: 100 byte local buffer", 0, 0);
        DclMemSet(abBuff2, 1, sizeof(abBuff2));
    }

    {
        D_BUFFER    abBuff3[200];
        DclMemSet(abBuff3, 0, sizeof(abBuff3));
        DclProfilerRecord("ScopeClause: 200 byte local buffer", 0, 0);
        DclMemSet(abBuff3, 1, sizeof(abBuff3));
    }

    DclProfilerLeave(0);
    return;
}


/*-------------------------------------------------------------------
    Missing "leave" instrumentation
-------------------------------------------------------------------*/
static void DclTestProf_MissingLeave_Level_1(void)
{
    DclProfilerEnter("DclTestProf_MissingLeave_Level_1", 0, 0);

    /*  Do the real test at "level 2"
    */
    DclTestProf_MissingLeave_Level_2();

    DclProfilerLeave(0);
    return;
}

static void DclTestProf_MissingLeave_Level_2(void)
{
    D_UINT32    ulTemp = 1234;

    DclProfilerEnter("DclTestProf_MissingLeave_Level_2", 0, ulTemp);

    /*  Bad early return -- DclProfilerLeave() is not executed...
    */
    if(ulTemp == 1234)
        return;

    DclProfilerLeave(0);
    return;
}



