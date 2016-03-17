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

    This module implements the meat of the FXIMAGE utility which is used to
    read and write flash images, in a variety of modes.  See the syntax
    description elsewhere in this module for more information.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fximg.c $
    Revision 1.21  2010/12/12 05:50:59Z  garyp
    Fixed to ignore processing areas of the flash which are erased.
    Revision 1.20  2010/11/22 16:21:27Z  glenns
    Fix erroneous return code.
    Revision 1.19  2009/12/31 17:24:42Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.18  2009/10/07 17:45:58Z  garyp
    Comments updated -- no functional changes.
    Revision 1.17  2009/10/07 17:29:43Z  garyp
    Eliminated the use of ntspare.h.  Replaced with fxnandapi.h.
    Revision 1.16  2009/10/06 21:00:54Z  garyp
    Updated to use re-abstracted ECC calculation and correction functions.
    Eliminated use of the FFXECC structure.  Modified to use some renamed
    functions to avoid naming conflicts.
    Revision 1.15  2009/07/21 21:32:50Z  garyp
    Merged from the v4.0 branch.  Updated to use the revised FfxSignOn()
    function, which now takes an fQuiet parameter.  Documentation updated.
    Revision 1.14  2009/04/01 14:57:36Z  davidh
    Function Headers Updated for AutoDoc.
    Revision 1.13  2009/03/09 19:41:14Z  thomd
    Cast parameter of FfxFmlHandle()
    Revision 1.12  2009/02/18 03:58:56Z  keithg
    Added explicit void of unused formal parameters.
    Revision 1.11  2009/02/09 01:35:41Z  garyp
    Merged from the v4.0 branch.  Modified so the test's "main" function still
    compiles, even if allocator support is disabled.  Updated to use the new
    FFXTOOLPARAMS structure.
    Revision 1.10  2008/05/27 19:34:32Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.9  2008/04/07 23:40:51Z  brandont
    Updated to use the new DCL file system services.
    Revision 1.8  2008/04/03 23:30:42Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.7  2008/02/26 21:43:58Z  Garyp
    Added the ability to specify that the file contain high and/or low reserved
    space.  Added the XFORM option to allow tagged pages to be transformed
    into standard SSFDC or OFFSETZERO style spare areas.  Fixed a bug that
    prevented the tool from working properly in "VBF" mode, when using NOR.
    Revision 1.6  2008/01/13 07:27:02Z  keithg
    Function header updates to support autodoc.
    Revision 1.5  2007/12/27 02:12:51Z  billr
    Resolve bug 1662: fxImage shows an I/O error on a fs-mode data cycle
    Revision 1.4  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2007/07/19 21:24:25Z  timothyj
    Minor comments and assertions.
    Revision 1.2  2006/05/21 18:05:20Z  Garyp
    Documentation fixes.
    Revision 1.1  2006/03/06 00:41:44Z  Garyp
    Initial revision
    --------------------
    Gary Palmer 3/5/2006
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxtools.h>
#include <fxtrace.h>
#include <dlerrlev.h>

#if FFXCONF_VBFSUPPORT

#include <fxfmlapi.h>
#include <vbf.h>

#if FFXCONF_NANDSUPPORT
#include <fxnandapi.h>
#include <ecc.h>
#endif

#define MAX_FILENAME_LEN    (128)

typedef enum
{
    XFORM_NONE = 0,
    XFORM_SSFDC,
    XFORM_OFFSETZERO
} XFORM;

typedef struct
{
    FFXFMLHANDLE    hFML;
    FFXFMLINFO      FmlInfo;
    VBFHANDLE       hVBF;
    VBFDISKINFO     VbfInfo;
    D_BUFFER       *pBuffer;
    D_UINT32        ulIOCount;
    D_UINT32        ulReservedLoKB;
    D_UINT32        ulReservedHiKB;
    D_UINT16        uIOLen;
    char            szFile[MAX_FILENAME_LEN];
  #if FFXCONF_NANDSUPPORT
    XFORM           nXForm;
  #endif
    unsigned        fInfo     : 1;
    unsigned        fReading  : 1;
    unsigned        fWriting  : 1;
    unsigned        fFSImage  : 1;
    unsigned        fVBFImage : 1;
    unsigned        fRaw      : 1;
} IMAGEINFO;

static D_INT16  ReadImage(FFXTOOLPARAMS *pTP, IMAGEINFO *pII);
static D_INT16  WriteImage(FFXTOOLPARAMS *pTP, IMAGEINFO *pII);
static D_BOOL   ProcessParameters(FFXTOOLPARAMS *pTP, IMAGEINFO *pII);
static D_BOOL   ShowUsage(FFXTOOLPARAMS *pTP);
#if FFXCONF_NANDSUPPORT
static void     OffsetZeroBuildSpareArea(unsigned nPageSize, const D_BUFFER *pData, D_BUFFER *pSpare, const D_BUFFER *pTag, D_BOOL fUseEcc);
static void     SSFDCBuildSpareArea(     unsigned nPageSize, const D_BUFFER *pData, D_BUFFER *pSpare, const D_BUFFER *pTag, D_BOOL fUseEcc);
static D_BOOL   IsErased(const D_BUFFER *pBuffer, size_t nBufferLen);
#endif

#endif


/*-------------------------------------------------------------------
    Protected: FfxImage()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_INT16 FfxImage(
    FFXTOOLPARAMS  *pTP)
{
  #if FFXCONF_VBFSUPPORT
    D_UINT16        uErrorLevel;
    IMAGEINFO       ii;
    D_UINT32        ulTotalPages;

    DclPrintf("\nFlashFX Image Flashing Tool\n");
    FfxSignOn(FALSE);
    DclPrintf("\n");

    DclMemSet(&ii, 0, sizeof ii);

    if(!ProcessParameters(pTP, &ii))
    {
        ShowUsage(pTP);
        return 1;
    }

    ii.hFML = FfxFmlHandle(pTP->nDiskNum);
    if(!ii.hFML)
    {
        DclPrintf("%s: DISK%u not initialized\n", pTP->dtp.pszCmdName, pTP->nDiskNum);
        return __LINE__;
    }

    if(FfxFmlDiskInfo(ii.hFML, &ii.FmlInfo) != FFXSTAT_SUCCESS)
    {
        DclError();
        return __LINE__;
    }

    ulTotalPages = (ii.FmlInfo.ulBlockSize / ii.FmlInfo.uPageSize) * ii.FmlInfo.ulTotalBlocks;

    if(ii.fInfo)
    {
        ii.hVBF = FfxVbfHandle(pTP->nDiskNum);
        if(ii.hVBF)
        {
            if(FfxVbfDiskInfo(ii.hVBF, &ii.VbfInfo) == FFXSTAT_SUCCESS)
            {
                DclPrintf("/MODE:FS  will process %lU pages of %U bytes each\n",
                    ii.VbfInfo.ulTotalPages, ii.VbfInfo.uPageSize);
            }
        }
        else
        {
            DclPrintf("/MODE:FS option not available -- VBF not loaded for this disk\n");
        }

      #if FFXCONF_NANDSUPPORT
        if(ii.FmlInfo.uDeviceType == DEVTYPE_NAND)
        {
            if(ii.FmlInfo.uDiskFlags & DISK_RAWACCESSONLY)
            {
                DclPrintf("/MODE:VBF option not available -- window mapping includes BBM area\n");
            }
            else
            {
                DclPrintf("/MODE:VBF will process %lU pages of %U+%U bytes each (page data + tag)\n",
                    ulTotalPages, ii.FmlInfo.uPageSize, FFX_NAND_TAGSIZE);
            }
        }
        else
      #endif
        {
            DclPrintf("/MODE:VBF will process %lU pages of %U bytes each\n",
                ulTotalPages, ii.FmlInfo.uPageSize);
        }

      #if FFXCONF_NANDSUPPORT
        if(ii.FmlInfo.uDeviceType == DEVTYPE_NAND)
        {
            DclPrintf("/MODE:RAW will process %lU pages of %U+%U bytes each (page data + spare)\n",
                ulTotalPages, ii.FmlInfo.uPageSize, ii.FmlInfo.uSpareSize);
        }
        else
      #endif
        {
            DclPrintf("/MODE:RAW option not available -- DISK%u is not on NAND flash\n", ii.FmlInfo.nDiskNum);
        }

        return 0;
    }

    if(ii.fFSImage)
    {
        if(ii.FmlInfo.uDiskFlags & DISK_RAWACCESSONLY)
        {
            DclPrintf("/MODE:FS option not available -- VBF can't be loaded on a RAW access only disk\n");
            return __LINE__;
        }

        ii.hVBF = FfxVbfHandle(pTP->nDiskNum);
        if(!ii.hVBF)
        {
            DclPrintf("%s: VBF is not initialized for DISK%u\n", pTP->dtp.pszCmdName, pTP->nDiskNum);
            return __LINE__;
        }

        if(FfxVbfDiskInfo(ii.hVBF, &ii.VbfInfo) != FFXSTAT_SUCCESS)
        {
            DclError();
            return __LINE__;
        }

        ii.uIOLen = ii.VbfInfo.uPageSize;
        ii.ulIOCount = ii.VbfInfo.ulTotalPages;
    }

    if(ii.fVBFImage)
    {
        if(ii.FmlInfo.uDiskFlags & DISK_RAWACCESSONLY)
        {
            DclPrintf("/MODE:VBF option not available -- Disk is in RAW access mode\n");
            return __LINE__;
        }

        ii.uIOLen = ii.FmlInfo.uPageSize;

      #if FFXCONF_NANDSUPPORT
        if(ii.FmlInfo.uDeviceType == DEVTYPE_NAND)
        {
            if(ii.nXForm == XFORM_NONE)
                ii.uIOLen += FFX_NAND_TAGSIZE;
            else
                ii.uIOLen += ii.FmlInfo.uSpareSize;
        }
      #endif

        ii.ulIOCount = ulTotalPages;
    }

    if(ii.fRaw)
    {
        if(ii.FmlInfo.uDeviceType != DEVTYPE_NAND)
        {
            DclPrintf("%s: The /MODE:RAW option is only valid on NAND flash\n", pTP->dtp.pszCmdName);
            return __LINE__;
        }

        ii.uIOLen = ii.FmlInfo.uPageSize + ii.FmlInfo.uSpareSize;
        ii.ulIOCount = ulTotalPages;
    }

    ii.pBuffer = DclMemAlloc(ii.uIOLen);
    if(!ii.pBuffer)
    {
        DclPrintf("%s: Out of memory!\n", pTP->dtp.pszCmdName);
        return __LINE__;
    }

    if(ii.fReading)
        uErrorLevel = ReadImage(pTP, &ii);
    else
        uErrorLevel = WriteImage(pTP, &ii);

    DclMemFree(ii.pBuffer);

    return uErrorLevel;

  #else

    (void)pTP;

    DclPrintf("FlashFX is configured with Allocator support disabled\n");

    return DCLERRORLEVEL_FEATUREDISABLED;

  #endif
}


#if FFXCONF_VBFSUPPORT

/*-------------------------------------------------------------------
    Local: ReadImage()

    This function reads a flash image and write it to a file.

    Parameters:
        pTP - A pointer to the FFXTOOLPARAMS structure.
        pII - A pointer to the IMAGEINFO structure.

    Return Value:
        Returns 0 if successful, otherwise a line number.
-------------------------------------------------------------------*/
static D_INT16 ReadImage(
    FFXTOOLPARAMS  *pTP,
    IMAGEINFO      *pII)
{
    DCLFSFILEHANDLE hFile = NULL;
    unsigned        uWritten;
    D_INT16         nError;
    FFXIOSTATUS     ioStat;
    D_UINT32        ulPage = 0;
    D_UINT32        ulRemaining = pII->ulIOCount;
    D_UINT32        ulLastComplete = D_UINT32_MAX;
    DCLSTATUS DclStatus;

    (void)pTP;

    /*  Create the target file
    */
    DclStatus = DclFsFileOpen(pII->szFile, "w+b", &hFile);
    if(DclStatus)
    {
        DclPrintf("Error creating the file '%s'\n", pII->szFile);
        return __LINE__;
    }

    if(pII->ulReservedLoKB)
    {
        D_UINT32    ulReservedPages;

        ulReservedPages = pII->ulReservedLoKB * 1024 / pII->FmlInfo.uPageSize;

        if(DclOsFileSeek(hFile, ulReservedPages * pII->uIOLen, DCLFSFILESEEK_SET) != 0)
        {
            nError = __LINE__;
            goto Cleanup;
        }

        DclPrintf("Output file low reserved space of %lU pages (%U+%U bytes each)\n",
            ulReservedPages, pII->FmlInfo.uPageSize, pII->uIOLen - pII->FmlInfo.uPageSize);
    }

    /*  For the Length of the read, read the flash from the starting
        offset for a length of uLength each read, then write those
        bytes to the file.
    */
    while(ulRemaining)
    {
        D_UINT32    ulComplete = ulPage / (pII->ulIOCount / 100);

        if(ulComplete != ulLastComplete)
        {
            DclPrintf("Reading flash image...%lU percent complete\r", ulComplete);

            ulLastComplete = ulComplete;
        }

        /*  Read from the flash into the buffer
        */
      #if FFXCONF_NANDSUPPORT
        if(pII->fRaw)
        {
            FMLREAD_RAWPAGES(pII->hFML, ulPage, 1, pII->pBuffer,
                pII->pBuffer+pII->FmlInfo.uPageSize, ioStat);
        }
        else
      #endif
        {
            if(pII->fFSImage)
            {
                ioStat = FfxVbfReadPages(pII->hVBF, ulPage, 1, pII->pBuffer);
            }
            else
            {
              #if FFXCONF_NANDSUPPORT
                if(pII->FmlInfo.uDeviceType == DEVTYPE_NAND)
                {
                    if(pII->nXForm == XFORM_NONE)
                    {
                        FMLREAD_TAGGEDPAGES(pII->hFML, ulPage, 1, pII->pBuffer,
                            pII->pBuffer+pII->FmlInfo.uPageSize, FFX_NAND_TAGSIZE, ioStat);
                    }
                    else
                    {
                        DCLALIGNEDBUFFER(tag, data, FFX_NAND_MAXTAGSIZE);

                        FMLREAD_TAGGEDPAGES(pII->hFML, ulPage, 1, pII->pBuffer,
                                            tag.data, FFX_NAND_TAGSIZE, ioStat);

                        if(pII->nXForm == XFORM_OFFSETZERO)
                        {
                            OffsetZeroBuildSpareArea(pII->FmlInfo.uPageSize,
                                pII->pBuffer, pII->pBuffer+pII->FmlInfo.uPageSize,
                                tag.data, TRUE);
                        }
                        else if(pII->nXForm == XFORM_SSFDC)
                        {
                            SSFDCBuildSpareArea(pII->FmlInfo.uPageSize,
                                pII->pBuffer, pII->pBuffer+pII->FmlInfo.uPageSize,
                                tag.data, TRUE);
                        }
                        else
                        {
                            DclProductionError();
                        }
                    }
                }
                else
              #endif
                {
                    FMLREAD_PAGES(pII->hFML, ulPage, 1, pII->pBuffer, ioStat);
                }
            }
        }

        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("\nError reading page %lU from flash, %s\n", ulPage, FfxDecodeIOStatus(&ioStat));
            nError = __LINE__;
            goto Cleanup;
        }

        /*  Write the buffer information to the image file
        */
        uWritten = DclOsFileWrite(pII->pBuffer, 1, pII->uIOLen, hFile);
        if(uWritten != pII->uIOLen)
        {
            DclPrintf("\nError writing page %lU to file\n", ulPage);
            nError = __LINE__;
            goto Cleanup;
        }

        ulPage++;
        ulRemaining--;
    }

    DclPrintf("Reading flash image...100 percent complete\n\n");

    DclPrintf("Read %lU pages of %U+%U bytes each\n\n", ulPage,
        pII->FmlInfo.uPageSize, pII->uIOLen - pII->FmlInfo.uPageSize);

    if(pII->ulReservedHiKB)
    {
        D_UINT32    ulReservedPages;

        ulReservedPages = pII->ulReservedHiKB * 1024 / pII->FmlInfo.uPageSize;

        if(DclOsFileSeek(hFile, (ulReservedPages * pII->uIOLen)-1, DCLFSFILESEEK_CUR) != 0)
        {
            nError = __LINE__;
            goto Cleanup;
        }

        uWritten = DclOsFileWrite(" ", 1, 1, hFile);
        if(uWritten != 1)
        {
            DclPrintf("\nError writing high reserved marker to file\n");
            nError = __LINE__;
            goto Cleanup;
        }

        DclPrintf("Output file high reserved space of %lU pages (%U+%U bytes each)\n",
            ulReservedPages, pII->FmlInfo.uPageSize, pII->uIOLen - pII->FmlInfo.uPageSize);
    }

    DclPrintf("File %s created successfully.\n", (const char *)pII->szFile);

    nError = 0;

  Cleanup:

    /*  Close the file
    */
    if(hFile)
        DclFsFileClose(hFile);

    /*  If we got to here we read the image(s) from the disk.
        Zero means no error occured.
    */
    return nError;
}


/*-------------------------------------------------------------------
    Local: WriteImage()

    This function writes a flash image from data stored in a
    file.

    Parameters:
        pTP - A pointer to the FFXTOOLPARAMS structure.
        pII - A pointer to the IMAGEINFO structure.

    Return Value:
        Returns 0 if successful, otherwise a line number.
-------------------------------------------------------------------*/
static D_INT16 WriteImage(
    FFXTOOLPARAMS  *pTP,
    IMAGEINFO      *pII)
{
    DCLFSFILEHANDLE hFile = NULL;
    D_INT16         nError;
    FFXIOSTATUS     ioStat;
    D_UINT32        ulPage = 0;
    D_UINT32        ulRemaining = pII->ulIOCount;
    D_UINT32        ulFileSize;
    D_UINT32        ulPagesPerBlock = pII->FmlInfo.ulBlockSize / pII->FmlInfo.uPageSize;
    D_UINT32        ulLastComplete = D_UINT32_MAX;
    DCLSTATUS DclStatus;

    (void)pTP;

    DclAssert((pII->FmlInfo.ulBlockSize % pII->FmlInfo.uPageSize) == 0);

    /*  Open the source file for reading in binary mode.
    */
    DclStatus = DclFsFileOpen(pII->szFile, "r+b", &hFile);
    if(DclStatus)
    {
        DclPrintf("Error opening the file '%s'\n", pII->szFile);
        return __LINE__;
    }

    /*  Tell the file pointer to seek to the end of the file.
    */
    if(DclOsFileSeek(hFile, 0, DCLFSFILESEEK_END) != 0)
    {
        nError = __LINE__;
        goto Cleanup;
    }

    /*  Record the size of the file by retrieving the offset of the
        pointer into the file.  The pointer will be at the end of the
        file, which was set by the seek function.
    */
    ulFileSize = DclOsFileTell(hFile);

    DclAssert(ulFileSize);

    /*  Set the file pointer to point to the beginning of the file.
    */
    if(DclOsFileSeek(hFile, 0, DCLFSFILESEEK_SET) != 0)
    {
        nError = __LINE__;
        goto Cleanup;
    }

    /*  The file must appear to have been created by the same process by
        which we are going to write it out.
    */
    if((ulFileSize % pII->uIOLen) != 0)
    {
        DclPrintf("File size must be evenly divisible by the page size, (%U+%U bytes)",
            pII->FmlInfo.uPageSize, pII->uIOLen - pII->FmlInfo.uPageSize);

        nError = __LINE__;
        goto Cleanup;
    }

    if(pII->ulReservedLoKB)
    {
        D_UINT32    ulReservedPages;
        D_UINT32    ulReservedFileBytes;

        ulReservedPages = pII->ulReservedLoKB * 1024 / pII->FmlInfo.uPageSize;

        ulReservedFileBytes = ulReservedPages * pII->uIOLen;

        if(DclOsFileSeek(hFile, ulReservedFileBytes, DCLFSFILESEEK_SET) != 0)
        {
            nError = __LINE__;
            goto Cleanup;
        }

        DclAssert(ulFileSize > ulReservedFileBytes);
        ulFileSize -= ulReservedFileBytes;

        DclPrintf("Input file has %lU low reserved pages of %U+%U bytes each\n",
            ulReservedPages, pII->FmlInfo.uPageSize, pII->uIOLen - pII->FmlInfo.uPageSize);
    }

    if(pII->ulReservedHiKB)
    {
        D_UINT32    ulReservedPages;
        D_UINT32    ulReservedFileBytes;

        ulReservedPages = pII->ulReservedHiKB * 1024 / pII->FmlInfo.uPageSize;

        ulReservedFileBytes = ulReservedPages * pII->uIOLen;

        DclAssert(ulFileSize > ulReservedFileBytes);
        ulFileSize -= ulReservedFileBytes;

        DclPrintf("Input file has %lU high reserved pages of %U+%U bytes each\n",
            ulReservedPages, pII->FmlInfo.uPageSize, pII->uIOLen - pII->FmlInfo.uPageSize);
    }

    if((ulFileSize / pII->uIOLen) != pII->ulIOCount)
    {
        DclPrintf("The file appears to have been created using a different /MODE and/or window\n");
        DclPrintf("mapping (or reserved space settings).\n");
        DclPrintf("DISK%u has %lU %U-byte sized pages, plus %U extra bytes each (in this mode).\n",
            pII->FmlInfo.nDiskNum, pII->ulIOCount, pII->FmlInfo.uPageSize, pII->uIOLen - pII->FmlInfo.uPageSize);
        DclPrintf("The file has %lU %U-byte sized pages, plus %U extra bytes each (in this mode),\n",
            ulFileSize / pII->uIOLen, pII->FmlInfo.uPageSize, pII->uIOLen - pII->FmlInfo.uPageSize);

        nError = __LINE__;
        goto Cleanup;
    }

    /*  For FS-mode writes, discard the entire area to be written before starting.  This
        should help it compact faster.
    */
    if (pII->fFSImage)
    {
        DclPrintf("Discarding old filesystem contents\n");
        ioStat = FfxVbfDiscardPages(pII->hVBF, 0, pII->ulIOCount);
        if (!IOSUCCESS(ioStat, pII->ulIOCount))
        {
            DclPrintf("\nError discarding before writing image.\n");
            nError = __LINE__;
            goto Cleanup;
        }
    }

    while(ulRemaining)
    {
        D_UINT32    ulComplete = ulPage / (pII->ulIOCount / 100);

        if(ulComplete != ulLastComplete)
        {
            DclPrintf("Writing flash image...%lU percent complete\r", ulComplete);

            ulLastComplete = ulComplete;
        }

        /*  Read a block of info from the file. fread() will return the number
            of bytes that are read. If no bytes are read, then it will return
            the value of 0.
        */
        if(DclOsFileRead(pII->pBuffer, 1, pII->uIOLen, hFile) != pII->uIOLen)
        {
            DclPrintf("\nError reading page %lU from the file\n", ulPage);
            nError = __LINE__;
            goto Cleanup;
        }

        /*  Write the file data to the flash.
        */
        if (pII->fFSImage)
        {
            ioStat = FfxVbfWritePages(pII->hVBF, ulPage, 1, pII->pBuffer);
        }
        else
        {
            /*  Erase the flash drive for each zone before we write to it
             */
            if((ulPage % ulPagesPerBlock) == 0)
            {
                FMLERASE_BLOCKS(pII->hFML, ulPage / ulPagesPerBlock, 1, ioStat);
                if(!IOSUCCESS(ioStat, 1))
                {
                    DclPrintf("\nError erasing block %lX, %s\n", ulPage / ulPagesPerBlock, FfxDecodeIOStatus(&ioStat));
                    nError = __LINE__;
                    goto Cleanup;
                }
            }

          #if FFXCONF_NANDSUPPORT
            if(pII->FmlInfo.uDeviceType == DEVTYPE_NAND)
            {
                if (pII->fRaw)
                {
                    FMLWRITE_RAWPAGES(pII->hFML, ulPage, 1, pII->pBuffer,
                                      pII->pBuffer+pII->FmlInfo.uPageSize, ioStat);
                }
                else
                {
                    if(pII->nXForm == XFORM_NONE)
                    {
                        FMLWRITE_TAGGEDPAGES(pII->hFML, ulPage, 1, pII->pBuffer,
                            pII->pBuffer+pII->FmlInfo.uPageSize, FFX_NAND_TAGSIZE, ioStat);
                    }
                    else
                    {
                        D_BUFFER    abTag[FFX_NAND_MAXTAGSIZE];
                        unsigned    nTagOffset =
                            pII->nXForm == XFORM_OFFSETZERO ? NSOFFSETZERO_TAG_OFFSET : NSSSFDC_TAG_OFFSET;

                        FfxNtmHelpTagDecode(abTag, &pII->pBuffer[pII->FmlInfo.uPageSize+nTagOffset]);

                        FMLWRITE_TAGGEDPAGES(pII->hFML, ulPage, 1, pII->pBuffer, abTag, FFX_NAND_TAGSIZE, ioStat);
                    }
                }
            }
            else
          #endif
            {
                FMLWRITE_PAGES(pII->hFML, ulPage, 1, pII->pBuffer, ioStat);
            }
        }

        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("\nError writing page %lU to flash, %s\n", ulPage, FfxDecodeIOStatus(&ioStat));
            nError = __LINE__;
            goto Cleanup;
        }

        ulPage++;
        ulRemaining--;
    }

    DclPrintf("Writing flash image...100 percent complete\n\n");

    DclPrintf("Wrote %lU pages of %U+%U bytes each\n\n", ulPage,
        pII->FmlInfo.uPageSize, pII->uIOLen - pII->FmlInfo.uPageSize);

    DclPrintf("Flash image written successfully.\n");

    nError = 0;

  Cleanup:

    /*  Close the file
    */
    if(hFile)
        DclFsFileClose(hFile);

    /*  If we got to here we read the image(s) from the disk.
        Zero means no error occured.
    */
    return nError;
}


/*-------------------------------------------------------------------
    Local: ProcessParameters()

    This function parses the command line items and configures
    the tool as specified.

    Parameters:
        pTP - A pointer to the FFXTOOLPARAMS structure.
        pII - A pointer to the IMAGEINFO structure.

    Return Value:
        Returns TRUE if successful, FALSE otherwise.
-------------------------------------------------------------------*/
#define     ARGBUFFLEN 128
static D_BOOL ProcessParameters(
    FFXTOOLPARAMS  *pTP,
    IMAGEINFO      *pII)
{
    D_UINT16        uIndex;
    D_UINT16        uSwitch;
    char            achArgBuff[ARGBUFFLEN];
    D_UINT16        uArgCount;

    uArgCount = DclArgCount(pTP->dtp.pszCmdLine);

    /*  Start with argument number 1, argv[0] is the command line string
        for the program that is being executed.
    */
    for(uIndex = 1; uIndex <= uArgCount; uIndex++)
    {
        if(!DclArgRetrieve(pTP->dtp.pszCmdLine, uIndex, ARGBUFFLEN, achArgBuff))
        {
            DclPrintf("Bad argument!\n");
            return FALSE;
        }

        if(DclStrICmp(achArgBuff, "/READ") == 0)
        {
            pII->fReading = TRUE;
            continue;
        }
        else if(DclStrICmp(achArgBuff, "/WRITE") == 0)
        {
            pII->fWriting = TRUE;
            continue;
        }
        else if(DclStrICmp(achArgBuff, "/INFO") == 0)
        {
            pII->fInfo = TRUE;
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/MODE:", 6) == 0)
        {
            if(DclStrICmp(&achArgBuff[6], "RAW") == 0)
            {
                pII->fRaw = TRUE;
                continue;
            }
            else if(DclStrICmp(&achArgBuff[6], "FS") == 0)
            {
                pII->fFSImage = TRUE;
                continue;
            }
            else if(DclStrNICmp(&achArgBuff[6], "VBF", 3) == 0)
            {
                pII->fVBFImage = TRUE;

                if(achArgBuff[9] == ':')
                {
                  #if FFXCONF_NANDSUPPORT
                    if(DclStrICmp(&achArgBuff[10], "SSFDC") == 0)
                    {
                        pII->nXForm = XFORM_SSFDC;
                    }
                    else if(DclStrICmp(&achArgBuff[10], "OFFSETZERO") == 0)
                    {
                        pII->nXForm = XFORM_OFFSETZERO;
                    }
                    else
                    {
                        DclPrintf("%s: Unrecognized /MODE:VBF:xform parameter '%s'\n", pTP->dtp.pszCmdName, &achArgBuff[10]);
                        return FALSE;
                    }
                  #else
                    DclPrintf("%s: The \":xform\" parameter for the /MODE:VBF option is only valid with NAND\n", pTP->dtp.pszCmdName);
                    return FALSE;
                  #endif
                }
                else if(achArgBuff[9] != 0)
                {
                    DclPrintf("%s: Syntax error in '%s'\n", pTP->dtp.pszCmdName, achArgBuff);
                    return FALSE;
                }

                continue;
            }
            else
            {
                DclPrintf("%s: Unrecognized /MODE parameter '%s'\n", pTP->dtp.pszCmdName, &achArgBuff[6]);
                return FALSE;
            }
        }
        else if(DclStrNICmp(achArgBuff, "/FILE:", 6) == 0)
        {
            if((achArgBuff[6] == '"') || (achArgBuff[6] == '\''))
            {
                DclStrNCpy(pII->szFile, &achArgBuff[7], DclStrLen(&achArgBuff[7]) - 1);
                continue;
            }
            else
            {
                DclStrNCpy(pII->szFile, &achArgBuff[6], DclStrLen(&achArgBuff[6]));
                continue;
            }
        }
        else if(DclStrNICmp(achArgBuff, "/RESERVEDLO:", 12) == 0)
        {
            const char *pStr;

            pStr = DclNtoUL(&achArgBuff[12], &pII->ulReservedLoKB);

            if(!pStr || *pStr)
            {
                DclPrintf("%s: Syntax error in \"%s\"\n", pTP->dtp.pszCmdName, achArgBuff);
                return FALSE;
            }

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/RESERVEDHI:", 12) == 0)
        {
            const char *pStr;

            pStr = DclNtoUL(&achArgBuff[12], &pII->ulReservedHiKB);

            if(!pStr || *pStr)
            {
                DclPrintf("%s: Syntax error in \"%s\"\n", pTP->dtp.pszCmdName, achArgBuff);
                return FALSE;
            }

            continue;
        }

        uSwitch = 0;

        if(achArgBuff[uSwitch] == '/')
        {
            /*  Go to the next character to test
            */
            uSwitch++;

            switch (achArgBuff[uSwitch])
            {
                case '?':
                    return FALSE;

                default:
                    DclPrintf("Bad option: %s\n", &achArgBuff[uSwitch]);
                    return FALSE;
            }
        }
        else
        {
            DclPrintf("Bad option: %s\n", &achArgBuff[uSwitch]);
            return FALSE;
        }
    }

    if(pII->fWriting + pII->fReading + pII->fInfo != TRUE)
    {
        DclPrintf("%s: Must specify one and only one of /INFO, /READ, or /WRITE\n", pTP->dtp.pszCmdName);
        return FALSE;
    }

    if(!pII->fInfo && !pII->szFile[0])
    {
        DclPrintf("%s: A filename must be specified\n", pTP->dtp.pszCmdName);
        return FALSE;
    }

    if(!pII->fInfo && (pII->fRaw + pII->fFSImage + pII->fVBFImage != TRUE))
    {
        DclPrintf("%s: Must specify one and only one /MODE:type value\n", pTP->dtp.pszCmdName);
        return FALSE;
    }

  #if !FFXCONF_NANDSUPPORT
    if(pII->fRaw)
    {
        DclPrintf("%s: The /MODE:RAW option is only valid with NAND flash\n", pTP->dtp.pszCmdName);
        return FALSE;
    }
  #endif

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: ShowUsage()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL ShowUsage(
    FFXTOOLPARAMS  *pTP)
{
    DclPrintf("\n");
    DclPrintf("This utility reads and writes flash images.\n\n");
    DclPrintf("Usage: %s drive  {/INFO /READ /WRITE}  /FILE:'filename'  /MODE:type\n", pTP->dtp.pszCmdName);
    DclPrintf(" drive         The drive designation, in the form: %s\n", pTP->pszDriveForms);
    DclPrintf(" /?            This help information\n");
    DclPrintf(" /INFO         Display geometry information about the various possible modes\n");
    DclPrintf(" /READ         Read an image from flash into a file\n");
    DclPrintf(" /WRITE        Write an image from a file into flash\n");
    DclPrintf(" /FILE:'name'  The /WRITE input, or /READ output file specification\n");
    DclPrintf(" /MODE:type    The type of flash image to use, where 'type' is:\n");
    DclPrintf("           FS  A File System image using VBF level calls.  The file consists of\n");
    DclPrintf("               an array of VBF pages, typically a power of two, such as 512,\n");
    DclPrintf("               1024, or 2048 bytes each.\n");
    DclPrintf("  VBF[:xform]  A VBF image, using FML level calls.  The file consists of an\n");
    DclPrintf("               array of FML pages.  For NOR these will each be an FML page\n");
    DclPrintf("               long, whereas for NAND these will also include a tag immediately\n");
    DclPrintf("               following the page data, typically 2 bytes long.\n");
    DclPrintf("               For NAND only, an optional 'xform' of \"SSFDC\" or \"OFFSETZERO\" may\n");
    DclPrintf("               be specified which will transform the tag into a FlashFX standard\n");
    DclPrintf("               spare area format, typically 16 or 64 bytes long, depending on\n");
    DclPrintf("               the page size.  This is useful for creating \"perfect\" images for\n");
    DclPrintf("               manufacturing.\n");
    DclPrintf("          RAW  A raw NAND image.  The file consists of an array of pages plus\n");
    DclPrintf("               spare areas.  For 512B-page NAND, each entry will be 528 bytes\n");
    DclPrintf("               long, whereas for 2KB-page NAND, each entry will be 2112 bytes\n");
    DclPrintf("               long.  In this mode, no ECC or bad block management is used, and\n");
    DclPrintf("               the input or output file image <may> include the BBM area, if the\n");
    DclPrintf("               the DISKn mapping includes it (a RAW DISK mapping).\n");
    DclPrintf(" /RESERVEDLO:n Specifies that the input or output file has 'n' kilobytes of low\n");
    DclPrintf("               reserved space.  This figure is for the main flash area only, and\n");
    DclPrintf("               must not include any tag or spare area data.  This option is most\n");
    DclPrintf("               often used when the file is being used with the NAND Simulator,\n");
    DclPrintf("               will usually exactly match the ReservedLoKB setting for the DEVn\n");
    DclPrintf("               device definition in ffxconf.h.\n");
    DclPrintf(" /RESERVEDHI:n Same as /RESERVEDLO, but applies to the high end of the flash\n");
    DclPrintf("               array.\n");
    DclPrintf("Notes:\n");
    DclPrintf(" - Images must be read and written using the same /MODE setting.\n");
    DclPrintf(" - The number of pages processed always matches the size of the DISKn window\n");
    DclPrintf("   mapped onto the DEVn Device.\n");
    DclPrintf(" - For NAND, if the Disk window is mapped such that it includes the BBM area,\n");
    DclPrintf("   only the RAW access mode is allowed.\n\n");

    if(pTP->dtp.pszExtraHelp)
        DclPrintf(pTP->dtp.pszExtraHelp);

    return 1;
}


#if FFXCONF_NANDSUPPORT


/*-------------------------------------------------------------------
    Local: OffsetZeroBuildSpareArea()

        Initializes the spare area with the fixed values and the
        ECC generated from the data given using the offset zero
        format.

        Note that this function is used to build arrays of spare
        areas when emulating larger page sizes with small-block
        NAND.

        Likewise when emulating larger page sizes, we only record
        the tag value in the first one, and therefore the pTag
        pointer may be NULL if we do not want to record the tag for
        this given spare area.

        NOTE:  This function is only appropriate for use where
               sofware ECC is being used.

    Parameters
        uPageSize  - The page size to use
        uSpareSize - The spare area size to use
        pData      - A pointer to the data for which ECCs are to be
                     calculated.
        pSpare     - A pointer to the buffer in which to build the
                     spare data.
        pTag       - A pointer to the tag data to store in the
                     spare area.  This pointer may be NULL if the
                     tag value is to remain unset.

    Return Value
        None.
-------------------------------------------------------------------*/
static void OffsetZeroBuildSpareArea(
    unsigned            nPageSize,
    const D_BUFFER     *pData,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pTag,
    D_BOOL              fUseEcc)
{
    unsigned            nSpareRemaining = nPageSize / 32;

    DclAssert(pData);
    DclAssert(pSpare);
    DclAssert(nPageSize);
    DclAssert(nPageSize % DATA_BYTES_PER_ECC == 0);

    DclMemSet(pSpare, ERASED8, nSpareRemaining);

    if(pTag && !IsErased(pTag, FFX_NAND_TAGSIZE))
        FfxNtmHelpTagEncode(&pSpare[NSOFFSETZERO_TAG_OFFSET], pTag);

    if(fUseEcc && !IsErased(pData, nPageSize))
    {
        D_BUFFER    abECC[MAX_ECC_BYTES_PER_PAGE];
        D_BUFFER   *pabECC = abECC;
        unsigned    nn = 0;

        while(nPageSize)
        {
            FfxEccCalculate(pData, pabECC);

            nPageSize   -= DATA_BYTES_PER_ECC;
            pData       += DATA_BYTES_PER_ECC;
            pabECC      += BYTES_PER_ECC;
        }

        /*  NOTE: This could probably be done more efficiently with a single
            while() loop -- structured like this because this is how the
            original function in the NAND stuff works.
        */
        while(nSpareRemaining)
        {
            /*  Build the ECC values
            */
            pSpare[NSOFFSETZERO_FLAGS_OFFSET] = (D_UINT8)LEGACY_WRITTEN_WITH_ECC;

            pSpare[NSOFFSETZERO_ECC1_OFFSET + 0] = abECC[nn++];
            pSpare[NSOFFSETZERO_ECC1_OFFSET + 1] = abECC[nn++];
            pSpare[NSOFFSETZERO_ECC1_OFFSET + 2] = abECC[nn++];

            pSpare[NSOFFSETZERO_ECC2_OFFSET + 0] = abECC[nn++];
            pSpare[NSOFFSETZERO_ECC2_OFFSET + 1] = abECC[nn++];
            pSpare[NSOFFSETZERO_ECC2_OFFSET + 2] = abECC[nn++];

            nSpareRemaining -= NAND512_SPARE;
            pSpare          += NAND512_SPARE;
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Local: SSFDCBuildSpareArea()

        Initializes the spare area with the fixed values and the
        ECC generated from the data given using the SSFDC format.

        Note that this function is used to build arrays of spare
        areas when emulating larger page sizes with small-block
        NAND.

        Likewise when emulating larger page sizes, we only record
        the tag value in the first one, and therefore the pTag
        pointer may be NULL if we do not want to record the tag for
        this given spare area.

        NOTE: This function is only appropriate for use where
              sofware ECC is being used.

    Parameters:
        uPageSize  - The page size to use
        uSpareSize - The spare area size to use
        pData      - A pointer to the data for which ECCs are to be
                     calculated.
        pSpare     - A pointer to the buffer in which to build the
                     spare data.
        pTag       - A pointer to the tag data to store in the
                     spare area.  This pointer may be NULL if the
                     tag value is to remain unset.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void SSFDCBuildSpareArea(
    unsigned            nPageSize,
    const D_BUFFER     *pData,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pTag,
    D_BOOL              fUseEcc)
{
    unsigned            nSpareRemaining = nPageSize / 32;

    DclAssert(pData);
    DclAssert(pSpare);
    DclAssert(nPageSize);
    DclAssert(nPageSize % DATA_BYTES_PER_ECC == 0);

    DclMemSet(pSpare, ERASED8, nSpareRemaining);

    if(pTag && !IsErased(pTag, FFX_NAND_TAGSIZE))
        FfxNtmHelpTagEncode(&pSpare[NSSSFDC_TAG_OFFSET], pTag);

    if(fUseEcc && !IsErased(pData, nPageSize))
    {
        D_BUFFER    abECC[MAX_ECC_BYTES_PER_PAGE];
        D_BUFFER   *pabECC = abECC;
        unsigned    nn = 0;

        while(nPageSize)
        {
            FfxEccCalculate(pData, pabECC);

            nPageSize   -= DATA_BYTES_PER_ECC;
            pData       += DATA_BYTES_PER_ECC;
            pabECC      += BYTES_PER_ECC;
        }

        /*  NOTE: This could probably be done more efficiently with a single
            while() loop -- structured like this because this is how the
            original function in the NAND stuff works.
        */
        while(nSpareRemaining)
        {
            /*  Build the ECC values
            */
            pSpare[NSSSFDC_FLAGS_OFFSET] = (D_UINT8)LEGACY_WRITTEN_WITH_ECC;

            pSpare[NSSSFDC_ECC1_OFFSET + 0] = abECC[nn++];
            pSpare[NSSSFDC_ECC1_OFFSET + 1] = abECC[nn++];
            pSpare[NSSSFDC_ECC1_OFFSET + 2] = abECC[nn++];

            pSpare[NSSSFDC_ECC2_OFFSET + 0] = abECC[nn++];
            pSpare[NSSSFDC_ECC2_OFFSET + 1] = abECC[nn++];
            pSpare[NSSSFDC_ECC2_OFFSET + 2] = abECC[nn++];

            nSpareRemaining -= NAND512_SPARE;
            pSpare          += NAND512_SPARE;
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Local: IsErased()

    Parameters:
        pBuffer     - The data to examine
        nBufferLen  - The data length

    Return Value:
        Returns TRUE if the buffer is erased and FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL IsErased(
    const D_BUFFER     *pBuffer,
    size_t              nBufferLen)
{
    while(nBufferLen && (*pBuffer == ERASED8))
    {
        nBufferLen--;
        pBuffer++;
    }

    return (nBufferLen == 0);
}




#endif


#endif  /* FFXCONF_VBFSUPPORT */



