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

    This module provides a test harness for implementing tests.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltestfw.h $
    Revision 1.13  2010/09/10 22:10:50Z  garyp
    Added some extra braces to avoid compiler warnings.
    Revision 1.12  2010/09/01 01:14:38Z  garyp
    Added the CHECK_STATUS_FAIL() macro.
    Revision 1.11  2010/08/30 00:36:58Z  garyp
    Major update to support trapping asserts, quitting when the first failure
    occurs, allow popping into the debugger, and allow multiple verbosity
    levels.  Along with a general code cleanup.
    Revision 1.10  2010/08/27 17:50:35Z  garyp
    Added some const keywords where appropriate.
    Revision 1.9  2009/07/24 20:17:57Z  johnbr
    Changes for the power cycling tests.
    Revision 1.8  2009/04/14 20:55:00Z  garyp
    Removed AutoDoc tags.
    Revision 1.7  2009/02/08 01:11:14Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.6  2009/02/03 23:37:02Z  johnbr
    Moving the new #defines from the rlcore tests to global space.
    Revision 1.5  2009/01/12 11:35:19  johnbr
    Added test results ERROR and NOTIMPLEMENTED to the
    eDCLTESTFWSTATUS enum for the Monkey test tool.
    Revision 1.4  2007/12/18 05:07:05Z  brandont
    Updated comments.
    Revision 1.3  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.2  2007/10/17 21:50:28Z  brandont
    Added DCL name masquerading for public functions.
    Revision 1.1  2007/10/17 02:08:24Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLTESTFW_H_INCLUDED
#define DLTESTFW_H_INCLUDED

#define DCL_TESTFWFLAG_QUITONFAILURE        (0x00000001)
#define DCL_TESTFWFLAG_DEBUGERRORS          (0x00000002)

#define DCL_TESTFW_STRING_INTERNAL          "*** Internal Error ***"
#define DCL_TESTFW_STRING_PASSED            "Passed"
#define DCL_TESTFW_STRING_FAILED            "*** FAILED ***"
#define DCL_TESTFW_STRING_SKIPPED           "*** SKIPPED ***"
#define DCL_TESTFW_STRING_ERROR             "*** Test Error ***"
#define DCL_TESTFW_STRING_NOTIMPLEMENTED    "Not Implemented"


/*-------------------------------------------------------------------
    The following macros are used to display the results for a check.
    Typically, the results will be a function return code, so the
    default method of display will be %lX.

    These definitions imply a naming convention in the test cases.
    That is:

    hTestFw     - The pointer to the test framework globals.  The
                  variable is passed into each test case by the 
                  framework.  Most existing tests use this name.  If
                  you're going to use the #defines below, they require
                  that the globals parameter use this name.
    fTestPassed - This variable is local to each test case and is
                  assumed to be set to true on entry to the test case.
                  It contains the test result, TRUE for a passed test
                  and FALSE for a failure.  These #defines update this
                  value according to the results of the compares and
                  checks.
-------------------------------------------------------------------*/
#define SUITE_NAME( name )                                                      \
    DclTestFwPrintf(hTestFw, "\tStarting Suite: %s\n", #name);

#define SUITE_END( name )                                                       \
    DclTestFwPrintf(hTestFw, "\tEnding Suite:   %s\n", #name);

#define MARK_FAILED()                                                           \
    fTestPassed = FALSE;                                                        \
    if(DclTestFwFlags(hTestFw) & DCL_TESTFWFLAG_DEBUGERRORS)                    \
        DclOsDebugBreak(DCLDBGCMD_BREAK);

#define CHECK_BRANCH(name, actual, expected, nextLabel)                         \
    {                                                                           \
        if ( (actual) != (expected) )                                           \
        {                                                                       \
            DclTestFwInternalPrintf(hTestFw, DCL_VERBOSE_NORMAL,                \
                "FAIL\t%s\tActual value %lX mismatches expected %lX\t, skipping\t%s: %d\n", \
                #name, (D_UINT32)(actual), (D_UINT32)(expected), __FILE__, __LINE__); \
            DclTestFwCheckAsserts(hTestFw, FALSE);                              \
            MARK_FAILED();                                                      \
            goto nextLabel;                                                     \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            DclTestFwInternalPrintf(hTestFw, DCL_VERBOSE_OBNOXIOUS,             \
                "Pass\t%s\tActual value %lX matches expected %lX\t%s: %d\n",    \
                #name, (D_UINT32)(actual), (D_UINT32)(expected), __FILE__, __LINE__); \
            if(DclTestFwCheckAsserts(hTestFw, TRUE))                            \
            {                                                                   \
                MARK_FAILED();                                                  \
                goto nextLabel;                                                 \
            }                                                                   \
        }                                                                       \
    }

#define CHECK_MATCH(name, actual, expected )                                    \
    {                                                                           \
        if ( (actual) != (expected) )                                           \
        {                                                                       \
            DclTestFwInternalPrintf(hTestFw, DCL_VERBOSE_NORMAL,                \
                "FAIL\t%s\tActual value %lX mismatches expected %lX\t%s: %d\n", \
                #name, (D_UINT32)(actual), (D_UINT32)(expected), __FILE__, __LINE__); \
            DclTestFwCheckAsserts(hTestFw, FALSE);                              \
            MARK_FAILED();                                                      \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            DclTestFwInternalPrintf(hTestFw, DCL_VERBOSE_OBNOXIOUS,             \
                "Pass\t%s\tActual value %lX matches expected %lX\t%s: %d\n",    \
                #name, (D_UINT32)(actual), (D_UINT32)(expected), __FILE__, __LINE__); \
            if(DclTestFwCheckAsserts(hTestFw, TRUE))                            \
            {                                                                   \
                MARK_FAILED();                                                  \
            }                                                                   \
        }                                                                       \
    }

#define CHECK_MATCH_ULL(name, actual, expected )                                \
    {                                                                           \
        if ( (actual) != (expected) )                                           \
        {                                                                       \
            DclTestFwInternalPrintf(hTestFw, DCL_VERBOSE_NORMAL,                \
                "FAIL\t%s\tActual value %llX mismatches expected %llX\t%s: %d\n", \
                #name, (D_UINT64)(actual), (D_UINT64)(expected), __FILE__, __LINE__); \
            DclTestFwCheckAsserts(hTestFw, FALSE);                              \
            MARK_FAILED();                                                      \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            DclTestFwInternalPrintf(hTestFw, DCL_VERBOSE_OBNOXIOUS,             \
                "Pass\t%s\tActual value %llX matches expected %llX\t%s: %d\n",  \
                #name, (D_UINT64)(actual), (D_UINT64)(expected), __FILE__, __LINE__); \
            if(DclTestFwCheckAsserts(hTestFw, TRUE))                            \
            {                                                                   \
                MARK_FAILED();                                                  \
            }                                                                   \
        }                                                                       \
    }

#define CHECK_TRUE(name, actual )                                               \
    {                                                                           \
        if(!(actual))                                                           \
        {                                                                       \
            DclTestFwInternalPrintf(hTestFw, DCL_VERBOSE_NORMAL,                \
                "FAIL\t%s\tActual value '%s' is FALSE\t%s: %d\n",               \
                #name, #actual, __FILE__, __LINE__);                            \
            DclTestFwCheckAsserts(hTestFw, FALSE);                              \
            MARK_FAILED();                                                      \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            DclTestFwInternalPrintf(hTestFw, DCL_VERBOSE_OBNOXIOUS,             \
                "Pass\t%s\tActual value '%s' is TRUE\t%s: %d\n",                \
                #name, #actual, __FILE__, __LINE__);                            \
            if(DclTestFwCheckAsserts(hTestFw, TRUE))                            \
            {                                                                   \
                MARK_FAILED();                                                  \
            }                                                                   \
        }                                                                       \
    }

#define CHECK_STATUS_FAIL(name, stat)                                           \
    {                                                                           \
        if(stat == DCLSTAT_SUCCESS)                                             \
        {                                                                       \
            DclTestFwInternalPrintf(hTestFw, DCL_VERBOSE_NORMAL,                \
                "FAIL\t%s\tActual status %lX does not fail as expected\t%s: %d\n", \
                #name, stat, __FILE__, __LINE__);                               \
            DclTestFwCheckAsserts(hTestFw, FALSE);                              \
            MARK_FAILED();                                                      \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            DclTestFwInternalPrintf(hTestFw, DCL_VERBOSE_OBNOXIOUS,             \
                "Pass\t%s\tActual status %lX fails as expected\t%s: %d\n",      \
                #name, stat, __FILE__, __LINE__);                               \
            DclTestFwCheckAsserts(hTestFw, TRUE);                               \
        }                                                                       \
    }

/*-------------------------------------------------------------------
    tDclTestSpecificData

    Data that is specific to a test.
 -------------------------------------------------------------------*/
typedef struct _tDclTestSpecificData tDclTestSpecificData;


/*-------------------------------------------------------------------
    tDclTestFwHandle

    Test framework handle.
 -------------------------------------------------------------------*/
typedef struct sDCLTESTFWDATA * tDclTestFwHandle;


/*-------------------------------------------------------------------
    eDCLTESTFWSTATUS

    Possible results from a test.
 -------------------------------------------------------------------*/
typedef enum 
{
    DCL_TESTFW_INTERNAL_ERR = 0,
    DCL_TESTFW_PASSED = 3,
    DCL_TESTFW_FAILED = 4,
    DCL_TESTFW_SKIPPED = 5,
    DCL_TESTFW_ERROR = 6,
    DCL_TESTFW_NOTIMPLEMENTED = 7
} eDCLTESTFWSTATUS;


/*-------------------------------------------------------------------
    tDclTestFwCase

    Function pointer type for a test case.
 -------------------------------------------------------------------*/
typedef eDCLTESTFWSTATUS (*tDclTestFwCase)(tDclTestFwHandle hTestFw, tDclTestSpecificData *pTestData);


/*-------------------------------------------------------------------
    tDclTestFwTestSection

    Arrays of test cases (called a section).
 -------------------------------------------------------------------*/
typedef struct
{
    D_BOOL          fIsCategory;
    tDclTestFwCase  TestPtr;
    const char     *pszTestName;
} tDclTestFwTestSection;


#define DclTestFwFlags              DCLFUNC(DclTestFwFlags)
#define DclTestFwTestExecute        DCLFUNC(DclTestFwTestExecute)
#define DclTestFwSectionExecute     DCLFUNC(DclTestFwSectionExecute)
#define DclTestFwPrintf             DCLFUNC(DclTestFwPrintf)
#define DclTestFwInternalPrintf     DCLFUNC(DclTestFwInternalPrintf)
#define DclTestFwCheckAsserts       DCLFUNC(DclTestFwCheckAsserts)
#define DclTestFwLogNotImplemented  DCLFUNC(DclTestFwLogNotImplemented)
#define DclTestGetResultString      DCLFUNC(DclTestGetResultString)
#define DclTestGetLongResultString  DCLFUNC(DclTestGetLongResultString)

D_UINT32        DclTestFwFlags(            tDclTestFwHandle hTestFw);
void            DclTestFwPrintf(           tDclTestFwHandle hTestFw, const char *pszMsg, ...);
void            DclTestFwInternalPrintf(   tDclTestFwHandle hTestFw, unsigned nVerbosity, const char *pszMsg, ...);
unsigned        DclTestFwCheckAsserts(     tDclTestFwHandle hTestFw, D_BOOL fPassed);
void            DclTestFwLogNotImplemented(tDclTestFwHandle hTestFw, const char *pszMsg, ...);
const char *    DclTestGetResultString(    eDCLTESTFWSTATUS eResult);
const char *    DclTestGetLongResultString(eDCLTESTFWSTATUS eResult);

eDCLTESTFWSTATUS DclTestFwTestExecute(
    D_UINT32                        ulFlags,
    unsigned                        nVerbosity,
    const char                     *pszTitle,
    D_UINT32                        ulVERSIONVAL,
    const char                     *pszBUILDNUM,
    const char                     *pszCOPYRIGHT,
    const char                     *pszLogFile,
    tDclTestSpecificData           *pTestData,
    tDclTestFwCase                  TestHeaderCallback,
    const tDclTestFwTestSection    *TestSection,
    D_UINT32                        ulTestEntries);

eDCLTESTFWSTATUS DclTestFwSectionExecute(
    tDclTestFwHandle                hTestFw, 
    tDclTestSpecificData           *pTestData, 
    const tDclTestFwTestSection    *TestSection, 
    D_UINT32                        ulTestEntries);


#endif /* #ifndef DLTESTFW_H_INCLUDED */
