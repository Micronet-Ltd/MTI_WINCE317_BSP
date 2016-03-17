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

    This module contains the FlashFX Hooks portion of the ntpageio NTM.

    To customize this for a particular project, copy this module into
    the Project Directory, make your changes, and modify ffxproj.mak to
    build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhpageio.c $
    Revision 1.11  2009/04/08 15:08:09Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.10  2009/04/02 20:21:45Z  glenns
    - Fix Bugzilla #2468: Removed usage of ill-defined FXSTATUS
      values and replaced with FFXSTAT_UNSUPPORTEDFUNCTION.
      This will make it clearer that customers should not use the other
      status values.
    Revision 1.9  2009/02/17 08:01:01Z  keithg
    Added explicit void to unused function parameters.
    Revision 1.8  2009/01/17 00:04:10Z  glenns
    - Fixed up literal FFXIOSTATUS initialization in several places.
    Revision 1.7  2008/10/10 05:07:44Z  keithg
    Fixed bug 2172 - changed default erase failure.   xxxIOERROR
    is required for the NAND FIM to properly handle erase failures.
    Revision 1.6  2008/01/13 07:29:22Z  keithg
    Function header updates to support autodoc.
    Revision 1.5  2007/11/03 23:50:01Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2007/02/16 05:50:21Z  Garyp
    Fixed a conversion spot missed in the previous rev.
    Revision 1.3  2007/02/14 00:19:41Z  timothyj
    Changed D_UINT16 uCount parameters to D_UIN32 ulCount, to allow the call
    tree all the way up through the IoRequest to avoid having to range check
    (and/or split) requests.  Removed corresponding casts.
    Revision 1.2  2006/03/12 01:28:59Z  Garyp
    Updated to the current ntpageio NTM model.
    Revision 1.1  2005/12/08 17:32:44Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/
#include <flashfx.h>
#include <fxiosys.h>
#include <nandconf.h>
#include <nandctl.h>

typedef struct nand_ctl
{
    unsigned    fReading : 1;
    unsigned    fWriting : 1;
} NANDCTL;


/*-------------------------------------------------------------------
    Public: FfxHookNTPageCreate()

    This function creates a NAND Control Module instance which
    is associated with the ntpageio NTM.

    Parameters:
        hDev    - The Device handle
        pNFI    - A pointer to the NANDFLASHINFO structure to fill
        pBounds - A pointer to the FFXFIMBOUNDS structure to use

    Return Value:
        Returns a pointer to the NANDCTL structure to use if
        successful, otherwise NULL.
 -------------------------------------------------------------------*/
PNANDCTL FfxHookNTPageCreate(
    FFXDEVHANDLE        hDev,
    NANDFLASHINFO      *pNFI,
    FFXFIMBOUNDS       *pBounds)
{
    (void)hDev;
    (void)pBounds;
    (void)pNFI;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageCreate()\n"));

    /*  Replace this section with code to create the instance.
    */
    DclProductionError();

    return NULL;

#if 0
    /*  ID the flash and return the information in the NANDFLASHINFO
        structure.
    */

    ulID = GetFlashID();
    if(ulID == 0)
        return NULL;

    pNC = DclMemAllocZero(sizeof(*pNC));
    if(!pNC)
        return NULL;

    pNFI->abID[0] = (D_UINT8)((ulID >> 8) & 0xFF);  /* Mfg ID   */
    pNFI->abID[1] = (D_UINT8)(ulID & 0xFF);         /* Dev ID   */
    pNFI->abID[2] = 0;
    pNFI->abID[3] = 0;
    pNFI->abID[4] = 0;
    pNFI->abID[5] = 0;

    /*  Only one chip
    */
    pNFI->uDevices = 1;

    return pNC;
#endif
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageDestroy()

    This function destroys a NAND Control Module instance.

    Parameters:
        pNC - A pointer to the NANDCTL structure

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTPageDestroy(
    NANDCTL        *pNC)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageDestroy() pNC=%P\n", pNC));

    DclAssert(pNC);

    /*  Replace this section with code destroy the instance.
    */
    DclProductionError();
/*
    DclMemFree(pNC);
*/
    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageRead()

    This function reads one or more pages, and their associated
    spare areas, into the specified buffers.

    Either pPageBuff or pSpareBuff may be NULL if either
    respective areas are not required.  They cannot both be
    NULL as their would be no purpose for calling this function.

    Parameters:
        pNC         - A pointer to the NANDCTL structure
        ulStartPage - The starting page to read (relative to zero)
        uCount      - The number of pages to read
        pPageBuff   - The buffer to fill with page data.  May be
                      NULL if the page data is not required.
        pSpareBuff  - The buffer to fill with spare data.  May be
                      NULL if the spare data is not required.

    Return Value:
        Returns an FFXIOSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxHookNTPageRead(
    NANDCTL        *pNC,
    D_UINT32        ulStartPage,
    D_UINT16        uCount,
    D_BUFFER       *pPageBuff,
    D_BUFFER       *pSpareBuff)
{
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMIOERROR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageRead() Start=%lX Count=%U pPageBuff=%P pSpareBuff=%P\n",
        ulStartPage, uCount, pPageBuff, pSpareBuff));

    DclProfilerEnter("FfxHookNTPageRead", 0, uCount);

    DclAssert(pNC);
    DclAssert(pPageBuff || pSpareBuff);
    DclAssert(uCount);
    DclAssert(!pNC->fReading);
    DclAssert(!pNC->fWriting);

    /*  Replace this section with code to read pages and/or spare areas.
    */
    (void)pNC;
    (void)ulStartPage;
    (void)uCount;
    (void)pPageBuff;
    (void)pSpareBuff;
    DclProductionError();

    DclProfilerLeave(0);

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageReadStart()

    This function starts a page read operation which must be
    completed by calling FfxHookReadPageComplete().

    Parameters:
        pNC       - A pointer to the NANDCTL structure
        ulPageNum - The page to read

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTPageReadStart(
    NANDCTL        *pNC,
    D_UINT32        ulPageNum)
{
    FFXSTATUS       ffxStat = FFXSTAT_UNSUPPORTEDFUNCTION;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageReadStart() Page=%lX\n", ulPageNum));

    DclAssert(pNC);
    DclAssert(!pNC->fReading);
    DclAssert(!pNC->fWriting);

    DclProfilerEnter("FfxHookNTPageReadStart", 0, 1);

    /*  Replace this section with code to start a read page operation.
    */
    (void)pNC;
    (void)ulPageNum;
    DclProductionError();

    DclProfilerLeave(0);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageReadComplete()

    This function completes a page read operation which was
    started with FfxHookReadPageStart().

    Parameters:
        pNC        - A pointer to the NANDCTL structure
        pPageBuff  - The buffer to fill.  If this value is NULL, a
                     previously started readahead operation must be
                     completed, but no data returned.
        pSpareBuff - Must be NULL if pPageBuff is NULL.

    Return Value:
        Returns an FFXIOSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxHookNTPageReadComplete(
    NANDCTL        *pNC,
    D_BUFFER       *pPageBuff,
    D_BUFFER       *pSpareBuff)
{
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_UNSUPPORTEDFUNCTION);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 2, 0),
        "FfxHookNTPageReadComplete() pPageBuff=%P pSpareBuff=%P\n", pPageBuff, pSpareBuff));

    DclAssert(pNC);
    DclAssert(pNC->fReading);
    DclAssert(!pNC->fWriting);

    DclProfilerEnter("FfxHookNTPageReadComplete", 0, 1);

    /*  Replace this section with code to complete a read page operation.
    */
    (void)pNC;
    (void)pPageBuff;
    (void)pSpareBuff;
    DclProductionError();

    DclProfilerLeave(0);

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageReadHidden()

    This function reads the hidden areas from one or more pages
    into the specified buffer.

    Parameters:
        pNC         - A pointer to the NANDCTL structure
        ulStartPage - The starting page to read (relative to zero)
        ulCount     - The number of hidden areas to read
        uScale      - The scaling factor (1=every page, 2=every
                      2nd page, 4=every 4th page, etc).
        pBuffer     - The buffer to fill

    Return Value:
        Returns an FFXIOSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxHookNTPageReadHidden(
    NANDCTL        *pNC,
    D_UINT32        ulStartPage,
    D_UINT32        ulCount,
    D_UINT16        uScale,
    D_BUFFER       *pBuffer)
{
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMIOERROR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageReadHidden() Start=%lX Count=%lU Scale=%U pBuffer=%P\n",
        ulStartPage, ulCount, uScale, pBuffer));

    DclAssert(pNC);
    DclAssert(!pNC->fReading);
    DclAssert(!pNC->fWriting);

    DclProfilerEnter("FfxHookNTPageReadHidden", 0, ulCount);

    /*  Replace this section with code to read tags.
    */
    (void)pNC;
    (void)ulStartPage;
    (void)ulCount;
    (void)uScale;
    (void)pBuffer;
    DclProductionError();

    DclProfilerLeave(0);

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageWrite()

    This function writes data to one or more pages.

    Either pPageBuff or pSpareBuff may be NULL if either
    respective area is not to be modified.  They cannot both be
    NULL as their would be no purpose for calling this function.

    Parameters:
        pNC         - A pointer to the NANDCTL structure
        ulStartPage - The starting page to write (relative to zero)
        ulCount     - The number of pages to write
        pPageBuff   - The buffer to fill with page data.  May be
                      NULL if the page data is not to be modified.
        pSpareBuff  - The buffer to fill with spare data.  May be
                      NULL if the spare data is not to be modified.

    Return Value:
        Returns an FFXIOSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxHookNTPageWrite(
    NANDCTL        *pNC,
    D_UINT32        ulStartPage,
    D_UINT32        ulCount,
    const D_BUFFER *pPageBuff,
    const D_BUFFER *pSpareBuff)
{
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMIOERROR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageWrite() Start=%lX Count=%U pPageBuff=%P pSpareBuff=%P\n",
        ulStartPage, ulCount, pPageBuff, pSpareBuff));

    DclAssert(pNC);
    DclAssert(!pNC->fReading);
    DclAssert(!pNC->fWriting);
    DclAssert(pPageBuff || pSpareBuff);

    DclProfilerEnter("FfxHookNTPageWrite", 0, ulCount);

    /*  Replace this section with code to write pages and/or spare areas.
    */
    (void)pNC;
    (void)ulStartPage;
    (void)ulCount;
    (void)pPageBuff;
    (void)pSpareBuff;
    DclProductionError();

    DclProfilerLeave(0);

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageWriteStart()

    This function starts a page write operation which must be
    completed by calling FfxHookWritePageComplete().

    Parameters:
        pNC        - A pointer to the NANDCTL structure
        ulPageNum  - The page to write
        pPageBuff  - A pointer to the buffer containing page data
        pSpareBuff - A pointer to the buffer containing spare data

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTPageWriteStart(
    NANDCTL        *pNC,
    D_UINT32        ulPageNum,
    const D_BUFFER *pPageBuff,
    const D_BUFFER *pSpareBuff)
{
    FFXSTATUS       ffxStat = FFXSTAT_UNSUPPORTEDFUNCTION;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageWriteStart() Page=%lX pPageBuff=%P pSpareBuff=%P\n",
        ulPageNum, pPageBuff, pSpareBuff));

    DclAssert(pNC);
    DclAssert(!pNC->fReading);
    DclAssert(!pNC->fWriting);
    DclAssert(pPageBuff);
    DclAssert(pSpareBuff);

    DclProfilerEnter("FfxHookNTPageWriteStart", 0, 1);

    /*  Replace this section with code to start a write page operation.
    */
    (void)pNC;
    (void)ulPageNum;
    (void)pPageBuff;
    (void)pSpareBuff;
    DclProductionError();

    DclProfilerLeave(0);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageWriteComplete()

    This function completes a page write operation which was
    started with FfxHookWritePageStart().

    Parameters:
        pNC - A pointer to the NANDCTL structure

    Return Value:
        Returns an FFXIOSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxHookNTPageWriteComplete(
    NANDCTL        *pNC)
{
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_UNSUPPORTEDFUNCTION);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageWriteComplete()\n"));

    DclAssert(pNC);
    DclAssert(!pNC->fReading);
    DclAssert(pNC->fWriting);

    DclProfilerEnter("FfxHookNTPageWriteComplete", 0, 1);

    /*  Replace this section with code to complete a write page operation.
    */
    (void)pNC;
    DclProductionError();

    DclProfilerLeave(0);

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageEraseBlock()

    This function erases one flash block.

    Parameters:
        pNC        - A pointer to the NANDCTL structure
        ulBlockNum - The block to erase

    Return Value:
        Returns an FFXIOSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxHookNTPageEraseBlock(
    NANDCTL        *pNC,
    D_UINT32        ulBlockNum)
{
    FFXIOSTATUS     ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_FIMIOERROR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageEraseBlock() Block=%lX\n", ulBlockNum));

    DclProfilerEnter("FfxHookNTPageEraseBlock", 0, 1);

    /*  Replace this section with code to erase a block.
    */
    (void)pNC;
    (void)ulBlockNum;
    DclProductionError();

    DclProfilerLeave(0);

    return ioStat;
}



