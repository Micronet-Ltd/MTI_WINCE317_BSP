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
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltestfwp.h $
    Revision 1.5  2010/08/30 00:38:52Z  garyp
    Major update to support trapping asserts, quitting when the first failure
    occurs, allow popping into the debugger, and allow multiple verbosity
    levels.  Along with a general code cleanup.
    Revision 1.4  2009/01/06 01:07:44Z  brandont
    Updated to display lists of failed, skipped, and not implemented tests
    in the test summary.
    Revision 1.3  2008/05/29 19:47:53Z  garyp
    Merged from the WinMobile branch.
    Revision 1.2.1.2  2008/05/29 19:47:53Z  garyp
    Updated to correctly use DclTimeStamp() and DclTimePassed().
    Revision 1.2  2007/11/03 23:31:12Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/10/17 01:04:44Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLTESTFWP_H_INCLUDED
#define DLTESTFWP_H_INCLUDED


#define DCL_TESTFW_SUMMARY_LIST_STRING     100


typedef struct sDCLTESTFWSUMMARYLIST
{
    D_UINT32                        ulCategoryNumber;   /* 0 means "not used" */
    char                            szString[DCL_TESTFW_SUMMARY_LIST_STRING];
    struct sDCLTESTFWSUMMARYLIST   *pNext;
} DCLTESTFWSUMMARYLIST;


/*  sDCLTESTFWDATA

    Test data used by each test
*/
struct sDCLTESTFWDATA
{
    D_UINT32                ulFlags;
    D_UINT32                ulTotalTests;
    D_UINT32                ulTotalPassed;
    D_UINT32                ulTotalSkipped;
    D_UINT32                ulTotalFailed;
    D_UINT32                ulTotalNotImplemented;
    DCLTIMESTAMP            tsTestStart;
    DCLTIMESTAMP            tsSectionStart;
    D_UINT32                ulTestNumber;
    D_UINT32                ulFirstTestNumber;
    D_UINT32                ulLastTestNumber;
    tDclTestFwCase          TestHeaderCallback;
    unsigned                nIndentCount;
    unsigned                nAssertCount;
    unsigned                nVerbosity;
    D_BOOL                  fDoIndent;
    DCLTESTFWSUMMARYLIST   *pFailedList;
    D_BOOL                  fFailedListIncomplete;
    DCLTESTFWSUMMARYLIST   *pSkippedList;
    D_BOOL                  fSkippedListIncomplete;
    DCLTESTFWSUMMARYLIST   *pNIList;
    D_BOOL                  fNIListIncomplete;
};


#endif /* #ifndef DLTESTFWP_H_INCLUDED */
