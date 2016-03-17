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

    This header defines the interface between the various NAND specific
    components of FMSLTEST.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmslnand.h $
    Revision 1.9  2010/09/24 21:48:03Z  glenns
    Fix bug 3276- update parameter name; no functional changes.
    Revision 1.8  2010/07/07 19:34:34Z  garyp
    Added BBM tests.
    Revision 1.7  2009/04/09 03:52:30Z  garyp
    Renamed a structure to avoid AutoDoc naming collisions.
    Revision 1.6  2008/05/09 03:09:14Z  garyp
    Updated to pass "BigBlockBuffer" to the NAND test so it is available.
    Revision 1.5  2008/03/24 21:11:22Z  garyp
    Updated to accommodate variable length tags.
    Revision 1.4  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2007/10/22 23:05:01Z  pauli
    Added this file back to the project and updated for current NAND tests.
    Revision 1.2  2006/02/06 21:44:34Z  Garyp
    Updated to use the new FML interface.
    Revision 1.1  2005/11/04 10:05:46Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

typedef struct FMSLNANDTESTINFO
{
    FFXFMLHANDLE    hFML;
    D_UINT32        ulStartPage;
    D_UINT32       *pulRandomSeed;
    D_UINT32        ulSerial;
    FFXFMLINFO      FmlInfo;
    FFXFMLDEVINFO   FmlDevInfo;
    D_BUFFER       *pBuffer;        
    D_BUFFER       *pBuffer2;       
    D_BUFFER       *pBigBuffer;
    D_UINT32        ulBufferSize;   /* Size (in bytes) of pBuffer and pBuffer2 */
    D_UINT32        ulBigBufferSize;
    D_BUFFER       *pSpareArea;     /* Exactly the size of the NAND spare area */
    D_BUFFER       *pSpareArea2;    /* Exactly the size of the NAND spare area */
    D_BOOL          fExtensive;
    unsigned        nVerbosity;
    D_UINT32        ulPagesPerBlock;
} FMSLNANDTESTINFO;

typedef enum
{
    WITH_ECC = 'W',
    NO_ECC = 'N'
} PAGE_VERIFY_MODE;


D_BOOL   FfxFmslNANDTestEcc(FMSLNANDTESTINFO *pTI);
D_BOOL   FfxFmslNANDTestBBM(FMSLNANDTESTINFO *pTI);


/*  FMSL NAND Test utility functions.
*/
D_BOOL   FfxFmslNANDFindTestBlocks(FMSLNANDTESTINFO *pTI, D_UINT32 ulNumBlocks, D_UINT32 *pulBlockNum, D_BOOL fMayBeUsedRaw);
void     FfxFmslNANDPageFill(      FMSLNANDTESTINFO *pTI, D_BUFFER *pBuffer, D_UINT32 ulPageCount, D_UINT32 ulSerial, D_UINT32 ulStartWritePage, const char *szTestName);
D_BOOL   FfxFmslNANDPageVerify(    FMSLNANDTESTINFO *pTI, D_BUFFER *pBuffer, D_UINT32 ulPageCount, D_UINT32 ulSerial, D_UINT32 ulStartWritePage, const char *szTestName, D_UINT32 ulOrigFirstPage, D_UINT32 ulOrigPageCount, PAGE_VERIFY_MODE ecc);


