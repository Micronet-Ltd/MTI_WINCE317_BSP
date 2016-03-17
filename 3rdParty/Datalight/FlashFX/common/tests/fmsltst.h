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
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

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

    This header defines the internal interfaces for FMSLTEST.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmsltst.h $
    Revision 1.19  2010/07/07 19:34:57Z  garyp
    Added BBM tests.
    Revision 1.18  2009/11/17 18:58:06Z  garyp
    Added a first cut at some block locking tests.
    Revision 1.17  2009/08/04 18:58:20Z  garyp
    Merged from the v4.0 branch.  Prototype updates.
    Revision 1.16  2009/02/06 05:21:00Z  michaelm
    Renamed Read_Pages() to FfxFmslTestReadPages() and moved function
    description to fmsltst.c.
    Revision 1.15  2009/02/03 16:30:10Z  michaelm
    Made calls to READ_CORRECTEDPAGES and READ_SPARES conditional on
    FFXCONF_NANDSUPPORT.
    Revision 1.14  2009/02/03 15:47:09Z  michaelm
    Moved fxfmlapi.h include into fmsltst.h.
    Revision 1.13  2009/02/03 00:00:57Z  michaelm
    Moved Read_Pages into fmsltst.h.  Now using Read_Pages() so test will
    function normally in the case of a correctable read error.
    Revision 1.12  2009/01/14 22:28:05Z  deanw
    Removed use of global buffers. Functions now use dynamic memory allocation.
    Revision 1.11  2009/01/06 22:45:13Z  michaelm
    Added command-line switch /R to determine whether to run RetireBlockTest
    Revision 1.10  2008/05/09 03:24:37Z  garyp
    Updated to pass "BigBlockBuffer" to the NAND test so it is available.
    Revision 1.9  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.8  2007/10/27 18:19:15Z  Garyp
    Added FMSL level flash stress tests.
    Revision 1.7  2007/10/22 21:32:19Z  pauli
    Added verbose level.
    Revision 1.6  2007/08/24 20:21:59Z  pauli
    Removed FfxFmslNORControlReadWrite.  It is now part of the NOR specific
    tests.
    Revision 1.5  2007/07/16 22:41:10Z  rickc
    Updated for LFA
    Revision 1.4  2007/03/03 20:00:45Z  Garyp
    Added NAND specific FMSL performance tests which measure page reads
    and write using ECC.
    Revision 1.3  2007/02/27 21:01:58Z  Garyp
    Prototype updates.
    Revision 1.2  2006/12/29 18:37:20Z  Garyp
    Minor type changes -- nothing functional.
    Revision 1.1  2006/05/18 23:21:04Z  Garyp
    Initial revision
    Revision 1.2  2006/02/06 21:44:34Z  Garyp
    Updated to use the new FML interface.
    Revision 1.1  2005/11/04 10:05:46Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <fxfmlapi.h>

/*  Values for Read_Pages()
*/
#define READ_PAGES          1
#define READ_CORRECTEDPAGES 2
#define READ_SPARES         3

D_BOOL      FfxFmslStress(FFXFMLHANDLE hFML, D_UINT32 ulTestMinutes, D_UINT32 *pulRandomSeed, D_BOOL fExtensive, unsigned nVerbosity);
FFXIOSTATUS FfxFmslTestReadPages(FFXFMLHANDLE hFML, D_UINT32 ulStartPage, D_UINT32 ulNumberOfPagesToRead, D_BUFFER *pcDataReadBackBuffer, D_UINT32 ulOperation);

#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
D_BOOL      FfxFmslNORTest(FFXFMLHANDLE hFML, D_UINT32 *pulRandomSeed, D_BOOL fExtensive);
D_BOOL      FfxFmslNORSysDelay(void);
#endif

#if FFXCONF_NANDSUPPORT
D_BOOL      FfxFmslNANDTest(FFXFMLHANDLE hFML, D_UINT32 ulStartPage, D_BUFFER *pBigBuffer, D_UINT32 ulBufferSize, D_UINT32 *pulRandomSeed, D_BOOL fExtensive, D_BOOL fRetireBlockTest, D_BOOL fBBMTests, unsigned nVerbosity);
D_UINT32    FfxFmslNANDTestReadPerform(FFXFMLHANDLE hFML, D_BUFFER *pBuffer, D_UINT32 ulBufferSize, D_UINT32 ulMaxPages, D_UINT32 ulSeconds, unsigned nCount, unsigned nVerbosity);
D_UINT32    FfxFmslNANDTestWritePerform(FFXFMLHANDLE hFML, D_BUFFER *pBuffer, D_UINT32 ulBufferSize, D_UINT32 *pulPagesWritten, D_UINT32 ulSeconds, unsigned nCount, unsigned nVerbosity);
FFXSTATUS   FfxFmslLockTest(FFXFMLHANDLE hFML, D_UINT32 *pulRandomSeed, unsigned nVerbosity);
#endif

#if FFXCONF_ISWFSUPPORT
D_UINT32    FfxFmslISWFCalcControlAddress(D_UINT32 ulAddress, D_UINT16 uPageSize);
#endif


