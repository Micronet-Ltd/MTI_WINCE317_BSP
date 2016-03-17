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

    This module contains definitions for things that are shared between the
    DCL tests.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltests.h $
    Revision 1.3  2010/09/06 18:58:46Z  garyp
    Added a mutex test.
    Revision 1.2  2009/11/14 02:09:51Z  garyp
    Added prototypes.
    Revision 1.1  2009/06/27 22:31:42Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLTESTS_H_INCLUDED
#define DLTESTS_H_INCLUDED


/*-------------------------------------------------------------------
    Prototypes for unit tests that are called by DclTestMain.
-------------------------------------------------------------------*/
#define DclTest64BitMath            DCLFUNC(DclTest64BitMath)
#define DclTestAssert               DCLFUNC(DclTestAssert)
#define DclTestAtomicOperations     DCLFUNC(DclTestAtomicOperations)
#define DclTestByteOrderConversion  DCLFUNC(DclTestByteOrderConversion)
#define DclTestCompilerBugs         DCLFUNC(DclTestCompilerBugs)
#define DclTestDateTime             DCLFUNC(DclTestDateTime)
#define DclTestMBWCConv             DCLFUNC(DclTestMBWCConv)
#define DclTestMemFuncs             DCLFUNC(DclTestMemFuncs)
#define DclTestMemoryManagement     DCLFUNC(DclTestMemoryManagement)
#define DclTestMemoryProtection     DCLFUNC(DclTestMemoryProtection)
#define DclTestMemoryValidation     DCLFUNC(DclTestMemoryValidation)
#define DclTestMutexes              DCLFUNC(DclTestMutexes)
#define DclTestPath                 DCLFUNC(DclTestPath)
#define DclTestProfiler             DCLFUNC(DclTestProfiler)
#define DclTestThreads              DCLFUNC(DclTestThreads)
#define DclTestudivdi3              DCLFUNC(DclTestudivdi3)
#define DclTestUtf8                 DCLFUNC(DclTestUtf8)

DCLSTATUS DclTest64BitMath(void);
DCLSTATUS DclTestAssert(void);
DCLSTATUS DclTestAtomicOperations(void);
DCLSTATUS DclTestByteOrderConversion(void);
DCLSTATUS DclTestCompilerBugs(void);
DCLSTATUS DclTestDateTime(void);
DCLSTATUS DclTestMBWCConv(void);
DCLSTATUS DclTestMemFuncs(D_BOOL fTestClib, D_BOOL fPerf, D_BOOL fPerfLog, const char *pszPerfLogSuffix);
DCLSTATUS DclTestMemoryManagement(D_BOOL fStress);
DCLSTATUS DclTestMemoryProtection(D_BOOL fForce);
DCLSTATUS DclTestMemoryValidation(void);
DCLSTATUS DclTestMutexes(unsigned nVerbosity);
DCLSTATUS DclTestPath(void);
DCLSTATUS DclTestProfiler(void);
DCLSTATUS DclTestThreads(void);
DCLSTATUS DclTestudivdi3(void);
DCLSTATUS DclTestUtf8(void);



#endif /* DLTESTS_H_INCLUDED */


