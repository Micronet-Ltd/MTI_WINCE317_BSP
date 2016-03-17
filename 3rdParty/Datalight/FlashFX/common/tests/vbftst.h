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
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

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

    Internal interfaces for VBF unit tests.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: vbftst.h $
    Revision 1.7  2009/07/21 22:14:49Z  garyp
    Merged from the v4.0 branch.  Added a suspend count field.
    Revision 1.6  2008/05/20 17:41:30Z  thomd
    Added timing parameter to test structure.
    Revision 1.5  2007/11/08 18:51:27Z  pauli
    Added huge operations tests.
    Revision 1.4  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2007/10/29 06:38:00Z  Garyp
    Added wear-leveling tests.
    Revision 1.2  2007/10/18 17:07:07Z  pauli
    Added verbose level.
    Revision 1.1  2007/10/12 17:27:36Z  billr
    Initial revision
    ---------------------
    Bill Roman 2007-10-12
---------------------------------------------------------------------------*/

typedef struct
{
    FFXFMLHANDLE    hFML;
    VBFHANDLE       hVBF;
    D_UINT32        ulTestMemUsage;
    D_UINT32        ulRandSeed;
    D_BUFFER       *pBuffer;
    D_BUFFER       *pPatBuff;
    D_BUFFER       *pHugeBuff;
    D_UINT32        ulTestBufferSize;
    D_UINT32        ulHugeBufferSize;
    D_UINT32        ulTestBufferBlocks;
    D_UINT32        ulHugeBufferBlocks;
    D_UINT16        uUnitsPerRegion;
    D_UINT32        ulSuspendCount;
    unsigned        nVerbosity;
    VBFDISKINFO     DiskInfo;
    VBFDISKMETRICS  DiskMets;
    char            szPerfLogSuffix[PERFLOG_MAX_SUFFIX_LEN];
    long            lQATestNum;
    D_UINT32        ulWearLevMinutes;
    D_UINT32        ulExtensiveMinutes;
    unsigned        nWearLevStaticPercent;
    unsigned        fVBFInitialized         : 1;
    unsigned        fReformat               : 1;
    unsigned        fExtensiveTests         : 1;
    unsigned        fHugeTests              : 1;
    unsigned        fRegionBoundaries       : 1;
    unsigned        fTestWriteInterruptions : 1;
    unsigned        fPerformanceTests       : 1;
    unsigned        fTestUnitInfo           : 1;
    unsigned        fPerfLog                : 1;
    unsigned        fTestWearLeveling       : 1;
} VBFTESTINFO;

/*  Successful error level to return
*/
#define SUCCESS_RETURN_CODE      0


D_INT16 FfxTestVBFQA(VBFTESTINFO *pTI);
