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

    This module contains a special FlashFX Hooks module which is designed
    to interface with the ntpageio NTM.

    This module has functions with the exact same interface as the normal
    fhpageio.c functions, however it uses generic file I/O, to essentially
    implement and file based NTM, when used in combination with the
    ntpageio NTM.

    To cause this module to be used, you must modify ntpageio.c to set
    the USE_FILEIO_HOOKS_MODULE setting to TRUE.  This value is normally
    set to FALSE so the standard fhpageio.c functions will be linked in.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhpageio_file.c $
    Revision 1.23  2009/12/03 22:37:13Z  garyp
    Changed STMicro references to Numonyx.
    Revision 1.22  2009/10/07 17:34:23Z  garyp
    Eliminated the use of ntspare.h.  Replaced with fxnandapi.h.
    Revision 1.21  2009/07/24 18:43:10Z  garyp
    Partial merge from the v4.0 branch.  Fixed to compile properly if NAND 
    support is disabled.  If enabled, require either Samsung or STMicro 
    support to be turned on.
    Revision 1.20  2009/03/26 06:47:19Z  glenns
    Fixed Bug 2464: see documentation in the bug report.  Fixed Bug 2467:
    partial fix; be sure hooks provide accurate status to NTM.
    Revision 1.19  2009/02/06 08:30:47Z  keithg
    Updated to reflect new location of NAND header files and macros,.
    Revision 1.18  2009/01/16 15:53:53  glenns
    Fixed up literal FFXIOSTATUS initialization in five places.
    Revision 1.17  2008/10/10 05:07:52Z  keithg
    Fixed Bug 2172 - changed default erase failure.  xxxIOERROR is required for
    the NAND FIM to properly handle erase failures.
    Revision 1.16  2008/06/16 16:02:59Z  thomd
    Renamed ChipClass field to match higher levels
    Revision 1.15  2008/04/07 22:17:42Z  brandont
    Updated to use the new DCL file system services.
    Revision 1.14  2008/04/03 23:30:38Z  brandont
    Updated all defines and structures used by the DCL file system services to
    use the DCLFS prefix.
    Revision 1.13  2008/03/23 18:59:10Z  Garyp
    Corrected some error handling.
    Revision 1.12  2008/02/03 05:22:50Z  keithg
    Comment updates to support autodoc.
    Revision 1.11  2008/01/31 01:40:46Z  Garyp
    Modified FfxHookNTPageFileReadComplete() so the spare buffer pointer
    is optional.
    Revision 1.10  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.9  2007/08/02 22:22:15Z  timothyj
    Changed units of reserved space and maximum size to be in terms of KB
    instead of bytes.
    Revision 1.8  2007/08/01 21:52:05Z  timothyj
    Removed obsolete FFX_FLASHOFFSET.
    Revision 1.7  2007/03/01 00:33:21Z  timothyj
    Changed some bounds checking for LFA support to use width-nonspecific
    FFX_FLASHOFFSET_MAX in lieu of 32-bit specific D_UINT32_MAX.
    Revision 1.6  2007/02/13 22:14:30Z  timothyj
    Changed D_UINT16 uCount parameters to D_UIN32 ulCount, to allow the call
    tree all the way up through the IoRequest to avoid having to range check
    (and/or split) requests.  Removed corresponding casts.
    Revision 1.5  2007/01/03 22:40:39Z  Timothyj
    IR #777, 778, 681: Modified to use new FfxNandDecodeId() that returns a
    reference to a constant FFXNANDCHIP from the table where the ID was located.
    Removed FFXNANDMFG (replaced references with references to the constant
    FFXNANDCHIP returned, above).
    Revision 1.4  2006/08/31 21:39:43Z  Garyp
    Corrected a syntax error.
    Revision 1.3  2006/04/29 19:09:05Z  Garyp
    Updated to allow readonly files to be recognized and opened.
    Revision 1.2  2006/03/16 07:20:07Z  Garyp
    Updated debugging code.
    Revision 1.1  2006/03/13 02:30:10Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <fxiosys.h>
#include <nandconf.h>
#include <nandid.h>
#include <nandctl.h>

/*  At least one of these manufacturer types must be
    enabled for this module to compile.
*/
#if FFXCONF_NANDSUPPORT_SAMSUNG || FFXCONF_NANDSUPPORT_NUMONYX

#include "fhpageio_file.h"

#if 1
  #if FFXCONF_NANDSUPPORT_SAMSUNG
  #define PAGEFILE_CHIP_MFG (NAND_MFG_SAMSUNG)  /* A typical 512-byte per page part */
  #define PAGEFILE_CHIP_ID  (0x79)
  #elif FFXCONF_NANDSUPPORT_NUMONYX
    #define PAGEFILE_CHIP_MFG (NAND_MFG_NUMONYX)  /* A typical 512-byte per page part */
    #define PAGEFILE_CHIP_ID  (0x79)
#else
    #error "FFX: fhpageio_file.c: Can't find a NAND manufacturer which is defined"
  #endif
#else
  #if FFX_MAX_PAGESIZE < NAND2K_PAGE
  #error "FFX: fhpageio_file.c: FFX_MAX_PAGESIZE is too small for 2KB page flash parts"
  #endif
  #if FFXCONF_NANDSUPPORT_SAMSUNG
    #define PAGEFILE_CHIP_MFG (NAND_MFG_SAMSUNG)  /* A typical 2KB per page part */
    #define PAGEFILE_CHIP_ID  (0xA1)
  #elif FFXCONF_NANDSUPPORT_NUMONYX
    #define PAGEFILE_CHIP_MFG (NAND_MFG_NUMONYX)  /* A typical 2KB per page part */
  #define PAGEFILE_CHIP_ID  (0xF1)
  #elif FFXCONF_NANDSUPPORT_MICRON
    #define PAGEFILE_CHIP_MFG (NAND_MFG_MICRON)   /* A typical 2KB per page part */
    #define PAGEFILE_CHIP_ID  (0xA1)
  #else
    #error "FFX: fhpageio_file.c: Can't find a NAND manufacturer which is defined"
#endif
#endif

#define PAGEFILE_NUM_CHIPS  (1)

#if !PAGEFILE_CHIP_MFG || !PAGEFILE_CHIP_ID || !PAGEFILE_NUM_CHIPS
  #error "FFX: fhpageio_file.c: PAGEFILE_CHIP_MFG, PAGEFILE_CHIP_ID, and PAGEFILE_NUM_CHIPS must be initialized"
#endif

typedef struct nand_ctl
{
    DCLFSFILEHANDLE   hFile;
    D_UINT32        ulBlockSize;
    D_UINT32        ulTotalPages;
    D_UINT16        uPageSize;
    D_UINT16        uSpareSize;
    D_BUFFER       *pIOBuffer;
    int             nResult;
    FFXIOSTATUS     ioStat;
    unsigned        fReading  : 1;
    unsigned        fWriting  : 1;
    unsigned        fReadOnly : 1;
} NANDCTL;


/*-------------------------------------------------------------------
    Public: FfxHookNTPageFileCreate

        This function creates a NAND Control Module instance which
        is associated with the ntpageio NTM.

    Parameters:
        hDev        - The Device handle
        pNFI        - A pointer to the NANDFLASHINFO structure to fill
        pBounds     - A pointer to the FFXFIMBOUNDS structure to use

    Return:
        Returns a pointer to the NANDCTL structure to use if
        successful, otherwise NULL.
 -------------------------------------------------------------------*/
PNANDCTL FfxHookNTPageFileCreate(
    FFXDEVHANDLE        hDev,
    NANDFLASHINFO      *pNFI,
    FFXFIMBOUNDS       *pBounds)
{
    #define             BUFFLEN (16)
    char                szFilename[BUFFLEN];
    char               *pszFileName;
    unsigned            nDevNum;
    D_UINT32            ulTotalPhysSize;
    NANDCTL            *pNC = NULL;
    const FFXNANDCHIP  *pChip = NULL;
    DCLSTATUS           DclStatus;


    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageFileCreate()\n"));

    DclAssert(hDev);
    DclAssert(pNFI);
    DclAssert(pBounds);

    /*  ID the flash and return the information in the NANDFLASHINFO
        structure.
    */
    pNC = DclMemAllocZero(sizeof *pNC);
    if(!pNC)
        return NULL;

    pNFI->abID[0] = PAGEFILE_CHIP_MFG;
    pNFI->abID[1] = PAGEFILE_CHIP_ID;
    pNFI->abID[2] = 0;
    pNFI->abID[3] = 0;
    pNFI->abID[4] = 0;
    pNFI->abID[5] = 0;

    pNFI->uDevices = PAGEFILE_NUM_CHIPS;

    pChip = FfxNandDecodeID(pNFI->abID);
    if (pChip == NULL)
        goto CreateError;

    pNC->ulBlockSize = pChip->pChipClass->ulBlockSize;
    pNC->uPageSize = pChip->pChipClass->uPageSize;
    pNC->uSpareSize = pChip->pChipClass->uSpareSize;

    pNC->pIOBuffer = DclMemAlloc(pNC->uPageSize + pNC->uSpareSize);
    if(!pNC->pIOBuffer)
        goto CreateError;

    if(!FfxHookOptionGet(FFXOPT_DEVICE_NUMBER, hDev, &nDevNum, sizeof nDevNum))
        goto CreateError;

    if(!FfxHookOptionGet(FFXOPT_FIM_FILENAME, hDev, &pszFileName, sizeof pszFileName))
    {
        DclSNPrintf(szFilename, BUFFLEN, "FFXPFD%u.DAT", nDevNum);
        pszFileName = &szFilename[0];
    }

    /*  Try to open an existing file
    */
    DclStatus = DclFsFileOpen(pszFileName, "r+b", &pNC->hFile);
    if(DclStatus)
    {
        DclStatus = DclFsFileOpen(pszFileName, "rb", &pNC->hFile);
        if(!DclStatus)
        {
            pNC->fReadOnly = TRUE;

            DclPrintf("FFX: FHPAGEFILE: File opened in READONLY mode!\n");
        }
    }

    if(!DclStatus)
    {
        /*  Behave like we would on real flash -- regardless how the
            project may be configured, the real flash length supersedes
            the configuration options if it is shorter than what is
            configured.

            Get the file length, and use that as the emulated flash length.
        */
        if(DclOsFileSeek(pNC->hFile, 0, DCLFSFILESEEK_END) != 0)
            goto CreateError;

        ulTotalPhysSize = DclOsFileTell(pNC->hFile);
        pNC->ulTotalPages = ulTotalPhysSize / (pNC->uPageSize + pNC->uSpareSize);

        if((pNC->ulTotalPages * (pNC->uPageSize + pNC->uSpareSize) != ulTotalPhysSize) ||
            ((pNC->ulTotalPages * pNC->uPageSize) % pNC->ulBlockSize))
        {
            FFXPRINTF(1, ("FHPAGEFILE: File size %lX does not appear to match the chip characteristics\n",
                ulTotalPhysSize));

            goto CreateError;
        }

        FFXPRINTF(1, ("FHPAGEFILE: Using existing file %s, Length=%lUKB\n",
            pszFileName, ulTotalPhysSize / 1024UL));
    }
    else
    {
        unsigned    i;
        FFXIOSTATUS ioStat;

        if(pBounds->ulMaxArraySizeKB == D_UINT32_MAX)
        {
            ulTotalPhysSize = pChip->pChipClass->ulChipBlocks * pChip->pChipClass->ulBlockSize * pNFI->uDevices;
        }
        else
        {
            ulTotalPhysSize = (pBounds->ulMaxArraySizeKB +
                pBounds->ulReservedBottomKB + pBounds->ulReservedTopKB) * 1024UL;

            if(ulTotalPhysSize % pNC->ulBlockSize)
            {
                FFXPRINTF(1, ("FHPAGEFILE: Bounds size of %lX does not appear to match the chip characteristics\n",
                    ulTotalPhysSize));

                goto CreateError;
            }
        }

        pNC->ulTotalPages = ulTotalPhysSize / pNC->uPageSize;

        /*  The file does not exist, so create it and use the length
            specified by the configuration parameters.
        */
        DclStatus = DclFsFileOpen(pszFileName, "w+b", &pNC->hFile);
        if(DclStatus)
        {
            FFXPRINTF(1, ("FHPAGEFILE: Unable to create file '%s'\n", pszFileName));
            goto CreateError;
        }

        /*  Unlike NOR, which will get quickly formatted by VBF when a
            brand new file is created, with NAND we must initialize the
            file data to an erased state, or we will end up with a whole
            lot of bad blocks.

            This also serves the purpose of setting the file size.
        */
        for(i=0; i<ulTotalPhysSize / pNC->ulBlockSize; i++)
        {
            ioStat = FfxHookNTPageFileEraseBlock(pNC, i);
            if(!IOSUCCESS(ioStat, 1))
                goto CreateError;
        }
    }

    return pNC;

  CreateError:

    if(pNC)
    {
        if(pNC->hFile)
            DclFsFileClose(pNC->hFile);

        if(pNC->pIOBuffer)
            DclMemFree(pNC->pIOBuffer);

        DclMemFree(pNC);
    }

    return NULL;

}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageFileDestroy

        This function destroys a NAND Control Module instance.

    Parameters:
        pNC         - A pointer to the NANDCTL structure

    Return:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTPageFileDestroy(
    NANDCTL        *pNC)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageFileDestroy() pNC=%P\n", pNC));

    DclAssert(pNC);

    DclFsFileClose(pNC->hFile);

    DclMemFree(pNC->pIOBuffer);

    DclMemFree(pNC);

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageFileRead

        This function reads one or more pages, and their associated
        spare areas, into the specified buffers.

        Either pPageBuff or pSpareBuff may be NULL if either
        respective areas are not required.  They cannot both be
        NULL as their would be no purpose for calling this function.

    Parameters:
        pNC         - A pointer to the NANDCTL structure
        ulStartPage - The starting page to read (relative to zero)
        ulCount     - The number of pages to read
        pPageBuff   - The buffer to fill with page data.  May be
                      NULL if the page data is not required.
        pSpareBuff  - The buffer to fill with spare data.  May be
                      NULL if the spare data is not required.

    Return:
        Returns an FFXIOSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxHookNTPageFileRead(
    NANDCTL        *pNC,
    D_UINT32        ulStartPage,
    D_UINT32        ulCount,
    D_BUFFER       *pPageBuff,
    D_BUFFER       *pSpareBuff)
{
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMIOERROR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageFileRead() Start=%lX Count=%lX pPageBuff=%P pSpareBuff=%P\n",
        ulStartPage, ulCount, pPageBuff, pSpareBuff));

    DclProfilerEnter("FfxHookNTPageFileRead", 0, ulCount);

    DclAssert(pNC);
    DclAssert(pPageBuff || pSpareBuff);
    DclAssert(ulCount);
    DclAssert(!pNC->fReading);
    DclAssert(!pNC->fWriting);

    if(DclOsFileSeek(pNC->hFile, ulStartPage * (pNC->uPageSize + pNC->uSpareSize), DCLFSFILESEEK_SET) != 0)
        goto ReadCleanup;

    while(ulCount)
    {
        if(pPageBuff)
        {
            if(DclOsFileRead(pPageBuff, pNC->uPageSize, 1, pNC->hFile) != 1)
                goto ReadCleanup;

            pPageBuff += pNC->uPageSize;
        }
        else
        {
            if(DclOsFileSeek(pNC->hFile, pNC->uPageSize, DCLFSFILESEEK_CUR) != 0)
                goto ReadCleanup;
        }

        if(pSpareBuff)
        {
            if(DclOsFileRead(pSpareBuff, pNC->uSpareSize, 1, pNC->hFile) != 1)
                goto ReadCleanup;

            pSpareBuff += pNC->uSpareSize;
        }
        else
        {
            if(DclOsFileSeek(pNC->hFile, pNC->uSpareSize, DCLFSFILESEEK_CUR) != 0)
                goto ReadCleanup;
        }

        ioStat.ulCount++;
        ulCount--;
    }

    ioStat.ffxStat = FFXSTAT_SUCCESS;

  ReadCleanup:

    DclProfilerLeave(0);

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageFileReadStart

        This function starts a page read operation which must be
        completed by calling FfxHookReadPageComplete().

    Parameters:
        pNC         - A pointer to the NANDCTL structure
        ulPageNum   - The page to read

    Return:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTPageFileReadStart(
    NANDCTL        *pNC,
    D_UINT32        ulPageNum)
{
    FFXSTATUS       ffxStat = FFXSTAT_FIMIOERROR;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageFileReadStart() Page=%lX\n", ulPageNum));

    DclAssert(pNC);
    DclAssert(!pNC->fReading);
    DclAssert(!pNC->fWriting);

    DclProfilerEnter("FfxHookNTPageFileReadStart", 0, 1);

    /*  Even though there is not really anything such as asynchronous
        I/O in this model, we still do the PageReadStart/Complete
        sequence as separate operations so we can model as closely as
        possible what asynchronous operation would look like.
    */

    if(DclOsFileSeek(pNC->hFile, ulPageNum * (pNC->uPageSize + pNC->uSpareSize), DCLFSFILESEEK_SET) != 0)
        goto ReadStartCleanup;

    pNC->nResult = DclOsFileRead(pNC->pIOBuffer, pNC->uPageSize + pNC->uSpareSize, 1, pNC->hFile);

    /*  OK, yes the operation is already completed, but we're going to
        pretend like it is not.
    */
    pNC->fReading = TRUE;

    ffxStat = FFXSTAT_SUCCESS;

  ReadStartCleanup:

    DclProfilerLeave(0);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageFileReadComplete

        This function completes a page read operation which was
        started with FfxHookReadPageStart().

    Parameters:
        pNC         - A pointer to the NANDCTL structure
        pPageBuff   - The buffer to fill.  If this value is NULL, a
                      previously started readahead operation must be
                      completed, but no data returned.
        pSpareBuff  - Must be NULL if pPageBuff is NULL.

    Return:
        Returns an FFXIOSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxHookNTPageFileReadComplete(
    NANDCTL        *pNC,
    D_BUFFER       *pPageBuff,
    D_BUFFER       *pSpareBuff)
{
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMIOERROR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 2, 0),
        "FfxHookNTPageFileReadComplete() pPageBuff=%P pSpareBuff=%P\n", pPageBuff, pSpareBuff));

    DclAssert(pNC);
    DclAssert(pNC->fReading);
    DclAssert(!pNC->fWriting);

    DclProfilerEnter("FfxHookNTPageFileReadComplete", 0, 1);

    if(pNC->nResult != 1)
        goto ReadCompleteCleanup;

    if(pPageBuff)
    {
        DclMemCpy(pPageBuff, pNC->pIOBuffer, pNC->uPageSize);

        if(pSpareBuff)
            DclMemCpy(pSpareBuff, pNC->pIOBuffer + pNC->uPageSize, pNC->uSpareSize);

        ioStat.ulCount = 1;
    }
    else
    {
        DclAssert(pSpareBuff == NULL);
    }

    ioStat.ffxStat = FFXSTAT_SUCCESS;

    pNC->fReading = FALSE;

  ReadCompleteCleanup:

    DclProfilerLeave(0);

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageFileReadHidden

        This function reads the hidden areas from one or more pages
        into the specified buffer.

    Parameters:
        pNC         - A pointer to the NANDCTL structure
        ulStartPage - The starting page to read (relative to zero)
        ulCount     - The number of hidden areas to read
        uScale      - The scaling factor (1=every page, 2=every
                      2nd page, 4=every 4th page, etc).
        pBuffer     - The buffer to fill

    Return:
        Returns an FFXIOSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxHookNTPageFileReadHidden(
    NANDCTL        *pNC,
    D_UINT32        ulStartPage,
    D_UINT32        ulCount,
    D_UINT16        uScale,
    D_BUFFER       *pBuffer)
{
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMIOERROR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageFileReadHidden() Start=%lX Count=%lX Scale=%U pBuffer=%P\n",
        ulStartPage, ulCount, uScale, pBuffer));

    DclAssert(pNC);
    DclAssert(ulCount);
    DclAssert(pBuffer);
    DclAssert(!pNC->fReading);
    DclAssert(!pNC->fWriting);

    DclProfilerEnter("FfxHookNTPageFileReadHidden", 0, ulCount);

    if(DclOsFileSeek(pNC->hFile, (ulStartPage * (pNC->uPageSize + pNC->uSpareSize)) + pNC->uPageSize, DCLFSFILESEEK_SET) != 0)
        goto ReadHiddenCleanup;

    while(ulCount)
    {
        if(DclOsFileRead(pBuffer, LEGACY_ENCODED_TAG_SIZE, 1, pNC->hFile) != 1)
            goto ReadHiddenCleanup;

        ioStat.ulCount++;
        ulCount--;
        pBuffer += LEGACY_ENCODED_TAG_SIZE;

        if(ulCount)
        {
            DclAssert(uScale);

            if(DclOsFileSeek(pNC->hFile,
                (uScale * (pNC->uPageSize + pNC->uSpareSize)) - LEGACY_ENCODED_TAG_SIZE,
                DCLFSFILESEEK_CUR) != 0)
            {
                goto ReadHiddenCleanup;
            }
        }
    }

    ioStat.ffxStat = FFXSTAT_SUCCESS;

  ReadHiddenCleanup:

    DclProfilerLeave(0);

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageFileWrite

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

    Return:
        Returns an FFXIOSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxHookNTPageFileWrite(
    NANDCTL        *pNC,
    D_UINT32        ulStartPage,
    D_UINT32        ulCount,
    const D_BUFFER *pPageBuff,
    const D_BUFFER *pSpareBuff)
{
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMIOERROR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageFileWrite() Start=%lX Count=%U pPageBuff=%P pSpareBuff=%P\n",
        ulStartPage, ulCount, pPageBuff, pSpareBuff));

    DclAssert(pNC);
    DclAssert(ulCount);
    DclAssert(!pNC->fReading);
    DclAssert(!pNC->fWriting);
    DclAssert(pPageBuff || pSpareBuff);

    DclProfilerEnter("FfxHookNTPageFileWrite", 0, ulCount);

    if(DclOsFileSeek(pNC->hFile, ulStartPage * (pNC->uPageSize + pNC->uSpareSize), DCLFSFILESEEK_SET) != 0)
        goto WriteCleanup;

    while(ulCount)
    {
        unsigned    i;

        /*  Read the original page+spare data so we can properly AND the
            bits together.
        */
        if(DclOsFileRead(pNC->pIOBuffer, pNC->uPageSize + pNC->uSpareSize, 1, pNC->hFile) != 1)
            goto WriteCleanup;

        /*  Return to the position prior to the read.
        */
        if(DclOsFileSeek(pNC->hFile, -(pNC->uPageSize + pNC->uSpareSize), DCLFSFILESEEK_CUR) != 0)
            goto WriteCleanup;

        if(pPageBuff)
        {
            for (i = 0; i < pNC->uPageSize; i++)
                pNC->pIOBuffer[i] &= pPageBuff[i];

            pPageBuff += pNC->uPageSize;
        }

        if(pSpareBuff)
        {
            for (i = 0; i < pNC->uSpareSize; i++)
                pNC->pIOBuffer[pNC->uPageSize + i] &= pSpareBuff[i];

            pSpareBuff += pNC->uSpareSize;
        }

        /*  Write the updated page+spare data back out.
        */
        if(DclOsFileWrite(pNC->pIOBuffer, pNC->uPageSize + pNC->uSpareSize, 1, pNC->hFile) != 1)
            goto WriteCleanup;

        ioStat.ulCount++;
        ulCount--;
    }

    ioStat.ffxStat = FFXSTAT_SUCCESS;

  WriteCleanup:

    DclProfilerLeave(0);

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageFileWriteStart

        This function starts a page write operation which must be
        completed by calling FfxHookWritePageComplete().

    Parameters:
        pNC         - A pointer to the NANDCTL structure
        ulPageNum   - The page to write
        pPageBuff   - A pointer to the buffer containing page data
        pSpareBuff  - A pointer to the buffer containing spare data

    Return:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTPageFileWriteStart(
    NANDCTL        *pNC,
    D_UINT32        ulPageNum,
    const D_BUFFER *pPageBuff,
    const D_BUFFER *pSpareBuff)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageFileWriteStart() Page=%lX pPageBuff=%P pSpareBuff=%P\n",
        ulPageNum, pPageBuff, pSpareBuff));

    DclAssert(pNC);
    DclAssert(!pNC->fReading);
    DclAssert(!pNC->fWriting);
    DclAssert(pPageBuff);
    DclAssert(pSpareBuff);

    DclProfilerEnter("FfxHookNTPageFileWriteStart", 0, 1);

    pNC->ioStat = FfxHookNTPageFileWrite(pNC, ulPageNum, 1, pPageBuff, pSpareBuff);

    pNC->fWriting = TRUE;

    DclProfilerLeave(0);

    return pNC->ioStat.ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageFileWriteComplete

        This function completes a page write operation which was
        started with FfxHookWritePageStart().

    Parameters:
        pNC         - A pointer to the NANDCTL structure

    Return:
        Returns an FFXIOSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxHookNTPageFileWriteComplete(
    NANDCTL        *pNC)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageFileWriteComplete()\n"));

    DclAssert(pNC);
    DclAssert(!pNC->fReading);
    DclAssert(pNC->fWriting);

    DclProfilerEnter("FfxHookNTPageFileWriteComplete", 0, 1);

    pNC->fWriting = FALSE;

    DclProfilerLeave(0);

    return pNC->ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPageFileEraseBlock

        This function erases one flash block.

    Parameters:
        pNC         - A pointer to the NANDCTL structure
        ulBlockNum  - The block to erase

    Return:
        Returns an FFXIOSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxHookNTPageFileEraseBlock(
    NANDCTL        *pNC,
    D_UINT32        ulBlockNum)
{
    D_UINT32        ulPagesPerBlock;
    D_UINT32        ulOffset;
    FFXIOSTATUS     ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_FIMIOERROR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPageFileEraseBlock() Block=%lX\n", ulBlockNum));

    DclAssert(pNC);
    DclAssert(!pNC->fReading);
    DclAssert(!pNC->fWriting);

    DclProfilerEnter("FfxHookNTPageFileEraseBlock", 0, 1);

    DclAssert((pNC->ulBlockSize % pNC->uPageSize) == 0);
    ulPagesPerBlock = pNC->ulBlockSize / pNC->uPageSize;

    ulOffset = ulBlockNum * (ulPagesPerBlock * (pNC->uPageSize + pNC->uSpareSize));

    if(DclOsFileSeek(pNC->hFile, ulOffset, DCLFSFILESEEK_SET) != 0)
        goto EraseCleanup;

    DclMemSet(pNC->pIOBuffer, ERASED8, pNC->uPageSize + pNC->uSpareSize);

    while(ulPagesPerBlock)
    {
        /*  Write the erased page+spare data.
        */
        if(DclOsFileWrite(pNC->pIOBuffer, pNC->uPageSize + pNC->uSpareSize, 1, pNC->hFile) != 1)
            goto EraseCleanup;

        ulPagesPerBlock--;
    }

    ioStat.ulCount = 1;
    ioStat.ffxStat = FFXSTAT_SUCCESS;

  EraseCleanup:

    DclProfilerLeave(0);

    return ioStat;
}

#endif
#endif

