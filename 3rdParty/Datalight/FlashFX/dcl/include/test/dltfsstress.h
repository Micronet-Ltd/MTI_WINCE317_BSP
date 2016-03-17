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
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltfsstress.h $
    Revision 1.6  2011/05/16 23:06:15Z  garyp
    Moved the DclTestFSStressMain() prototype from dltfsstress.h to dltools.h.
    Revision 1.5  2011/03/29 03:51:31Z  garyp
    Record the case number in DLTFSSTRESSCASEINFO.
    Revision 1.4  2010/05/24 00:16:49Z  garyp
    Added more stats.
    Revision 1.3  2010/04/12 03:23:56Z  garyp
    Added a "DelTree" test case.  Removed some dead testing code.
    Revision 1.2  2009/11/05 03:27:45Z  garyp
    Minor prototype updates.
    Revision 1.1  2009/06/11 16:49:32Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLTFSSTRESS_H_INCLUDED
#define DLTFSSTRESS_H_INCLUDED

typedef struct
{
    struct sFSSTRESSTHREADINFO *pTC;
    const char     *pszTestName;
    const char     *pszBasePath;
    const char     *pszBaseName;
    unsigned        nThreadNum;
    unsigned        nCaseNum;
    unsigned        fSupportsCWD    : 1;
} DLTFSSTRESSCASEINFO;

/*  Generic function types, used to dump functions into a table later on
*/
typedef DCLSTATUS (FNDLTFSSTRESSTESTCASE)(const DLTFSSTRESSCASEINFO *CaseInfo);

/*  structure that holds the functions for a given test
*/
typedef struct
{
    FNDLTFSSTRESSTESTCASE  *pfnInit;
    FNDLTFSSTRESSTESTCASE  *pfnTest;
    FNDLTFSSTRESSTESTCASE  *pfnCleanup;
    const char             *pszTestName;
    D_ATOMIC32            ulAtomicGate;         /* Critical Section gate for the "sample" fields below */
    unsigned                nSampleOperations;  /* per each sample */
    D_UINT64                ullSampleTotalUS;   /* per each sample */
    D_UINT32                ulSampleMaxUS;      /* per each sample */
    D_UINT32                ulTotalOperations;  /* per whole test run */
    D_UINT64                ullTotalUS;         /* per whole test run */
    D_UINT32                ulMaxUS;            /* per whole test run */
} DLTFSSTRESSTESTCASE;



#endif /* DLTFSSTRESS_H_INCLUDED */

