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

    Dumps raw information about each of the FlashFX erase units, region
    information, and allocations lists in a readable form.  The data is
    in a human readable format, where possible data is interpreted, but
    *no* validatation is completed on the data contents.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxdmp.c $
    Revision 1.15  2010/12/12 07:20:22Z  garyp
    Added support for OffsetZero style flash.  Now use standard verbosity
    levels.
    Revision 1.14  2010/01/27 04:27:33Z  glenns
    Repair issues exposed by turning on a compiler option to warn of 
    possible data loss resulting from implicit typecasts between
    integer data types.
    Revision 1.13  2009/12/31 17:24:42Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.12  2009/07/21 21:32:03Z  garyp
    Merged from the v4.0 branch.  Updated to use the revised FfxSignOn()
    function, which now takes an fQuiet parameter.  Documentation updated.
    Revision 1.11  2009/04/01 14:44:08Z  davidh
    Function hearders updated for AutoDoc.
    Revision 1.10  2009/02/09 01:38:07Z  garyp
    Merged from the v4.0 branch.  Updated to use the new FFXTOOLPARAMS
    structure.
    Revision 1.9  2008/04/07 22:41:07Z  brandont
    Updated to use the new DCL file system services.
    Revision 1.8  2008/04/03 23:30:41Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.7  2008/01/31 01:23:14Z  Garyp
    Added the /WRITTEN option to cause only those pages which have been
    written to be displayed.
    Revision 1.6  2008/01/13 07:27:00Z  keithg
    Function header updates to support autodoc.
    Revision 1.5  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2006/10/03 23:56:32Z  Garyp
    Updated to use the new style printf macros and functions.
    Revision 1.3  2006/05/05 02:40:44Z  Garyp
    Major update to support NAND, among other things.  Added several new
    display modes, including the ability to dump BBM information.
    Revision 1.2  2006/02/12 18:15:14Z  Garyp
    Minor type changes.
    Revision 1.1  2005/10/20 02:33:00Z  Pauli
    Initial revision
    Revision 1.2  2005/10/20 03:32:59Z  garyp
    Changed some D_CHAR buffers to be plain old char.
    Revision 1.1  2005/10/02 03:24:26Z  Garyp
    Initial revision
    Revision 1.8  2005/01/28 22:20:40Z  GaryP
    Tweaked the syntax screen.
    Revision 1.7  2004/12/30 17:32:44Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.6  2004/12/18 08:57:25Z  GaryP
    General cleanup.  Removed the STDIN support.  Allow the filename to be
    specified with single quotes.
    Revision 1.5  2004/12/02 01:27:41Z  GaryP
    Modified to use the standard signon message.
    Revision 1.4  2004/12/01 21:58:20Z  tonyq
    Updated to use Datalight's _sysmalloc and _sysfree wrappers
    Revision 1.3  2004/11/30 00:11:25Z  garys
    Removed check for negative value from FfxAtoI(), which returns uint
    Revision 1.2  2004/10/31 00:50:18  GaryP
    Updated to build cleanly with MSVC6 tools.
    Revision 1.1  2004/10/27 23:26:38Z  jaredw
    Initial revision
    -----------------------
    Jared Wilkin 10/18/2004
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxtools.h>
#include <fxver.h>
#include <vbfdump.h>
#include "fxdmpbbm.h"

#define MAX_FILESPEC_LEN    (256)
#define DEFAULT_PAGE_SIZE   (512)
#define DEFAULT_VERBOSITY   (1)

typedef struct
{
    unsigned        nVerbosity;         /* verbosity level */
    D_UINT16        uSpareSize;
    D_UINT32        ulPageSize;
    D_UINT32        ulOffsetKB;         /* offset in KB */
    D_UINT32        ulPageTruncLen;
    D_UINT32        ulAbsoluteOffset;
    D_BUFFER       *pPageBuff;
    DCLFSFILEHANDLE hFile;
    char            szPath[MAX_FILESPEC_LEN];
    unsigned        fIsNand      : 1;   /* is this a NAND image */
    unsigned        fRawPages    : 1;   /* dump raw pages */
    unsigned        fRawSpares   : 1;   /* dump spare areas */
    unsigned        fDumpVBF     : 1;
    unsigned        fDumpBBM     : 1;
    unsigned        fWrittenOnly : 1;
    unsigned        fOffsetZero  : 1;   /* NAND is OFFSETZERO rather than SSFDC */
} FXDUMPINFO;

static D_BOOL       ShowUsage(     FFXTOOLPARAMS *pTP);
static FFXSTATUS    SetSwitches(   FXDUMPINFO *pDI, const char * pszCmdLine);
static void         DumpRawSpares( FXDUMPINFO *pDI);
static void         DumpRawPages(  FXDUMPINFO *pDI);
static D_BOOL       VerifyFileSize(FXDUMPINFO *pDI);


/*-------------------------------------------------------------------
    Protected: FfxDump()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDump(
    FFXTOOLPARAMS  *pTP)
{
    FXDUMPINFO      di = {0};
    FFXSTATUS       ffxStat;

    DclPrintf("\nFlashFX Image Dump Utility\n");
    FfxSignOn(FALSE);

    di.nVerbosity = DCL_VERBOSE_NORMAL;

    ffxStat = SetSwitches(&di, pTP->dtp.pszCmdLine);
    if(ffxStat == DCLSTAT_HELPREQUEST)
        ShowUsage(pTP);

    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    if(!di.ulPageSize)
        di.ulPageSize = DEFAULT_PAGE_SIZE;

    if(!di.ulPageTruncLen)
        di.ulPageTruncLen = di.ulPageSize;

    if(di.fIsNand)
    {
        DclAssert(di.ulPageSize);

        /*  Inherent assumption that a NAND spare size is always 1/32nd
            of the page size.
        */
        di.uSpareSize = (D_UINT16)di.ulPageSize / 32;
    }

    di.ulAbsoluteOffset = (di.ulOffsetKB * 1024) +
                         ((di.ulOffsetKB * 1024 / di.ulPageSize) * di.uSpareSize);

    di.pPageBuff = DclMemAlloc(di.ulPageSize);
    if(!di.pPageBuff)
    {
        ffxStat = DCLSTAT_OUTOFMEMORY;
        goto DumpCleanup;
    }

    /*  attempt to open the file given
    */
    ffxStat = DclFsFileOpen(di.szPath, "rb", &di.hFile);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("Error %lX attempting to open image file \"%s\".\n", ffxStat, di.szPath);
        goto DumpCleanup;
    }

    if(di.fRawPages)
        DumpRawPages(&di);

    /*  If "pages" was done, it internally handles spares if specified,
        so skip it here if need be.
    */
    if(di.fRawSpares && !di.fRawPages)
        DumpRawSpares(&di);

    if(di.fDumpBBM)
    {
        ffxStat = DclFsFileSeek(di.hFile, di.ulAbsoluteOffset, DCLFSFILESEEK_SET);
        if(ffxStat != FFXSTAT_SUCCESS)
            goto DumpCleanup;
        
        if(!FfxBbmDump(di.hFile, di.nVerbosity))
        {
            ffxStat = FFXSTAT_FAILURE;
            goto DumpCleanup;
        }
    }

    if(di.fDumpVBF)
    {
        ffxStat = DclFsFileSeek(di.hFile, di.ulAbsoluteOffset, DCLFSFILESEEK_SET);
        if(ffxStat != FFXSTAT_SUCCESS)
            goto DumpCleanup;

        ffxStat = FfxVbfDump(di.hFile, di.ulOffsetKB, 0, di.fIsNand, di.fOffsetZero, di.nVerbosity, (char*)pTP->dtp.pszCmdLine);
    }
    else
    {
        ffxStat = FFXSTAT_SUCCESS;
    }

  DumpCleanup:

    if(di.hFile)
        DclFsFileClose(di.hFile);

    if(di.pPageBuff)
        DclMemFree(di.pPageBuff);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: DumpRawPages()

    Parameters:

    Return Value:  
        None.
-------------------------------------------------------------------*/
static void DumpRawPages(
    FXDUMPINFO *pDI)
{
    D_UINT32    ulPage = 0;

    DclAssert(pDI);

    DclPrintf("\nRaw page dump at offset %lU KB.  PageSize=%lU SpareSize=%U\n",
        pDI->ulOffsetKB, pDI->ulPageSize, pDI->uSpareSize);

    if(pDI->ulAbsoluteOffset)
        DclPrintf("Starting at absolute file offset %lU\n", pDI->ulAbsoluteOffset);

    if(pDI->ulPageTruncLen != pDI->ulPageSize)
        DclPrintf("Truncating page data output after the first %lU bytes\n", pDI->ulPageTruncLen);

    if(pDI->fWrittenOnly)
        DclPrintf("Displaying data only for those pages which have been written\n");

    DclPrintf("\n");

    if(!VerifyFileSize(pDI))
        return;

    DclFsFileSeek(pDI->hFile, pDI->ulAbsoluteOffset, DCLFSFILESEEK_SET);

    while(!DclOsFileEOF(pDI->hFile))
    {
        D_BOOL  fIsWritten = TRUE;  /* Assume the page has been written */

        if(DclOsFileRead(pDI->pPageBuff, 1, pDI->ulPageSize, pDI->hFile) != pDI->ulPageSize)
            break;

        if(pDI->fWrittenOnly)
        {
            unsigned    ii;

            for(ii=0; ii<pDI->ulPageSize; ii++)
            {
                if(pDI->pPageBuff[ii] != 0xFF)
                    break;
            }

            if(ii == pDI->ulPageSize)
                fIsWritten = FALSE;
        }

        if(fIsWritten)
        {
            DclPrintf("Page %lU:\n", ulPage);

            DclHexDump(NULL, HEXDUMP_UINT8, 32, pDI->ulPageTruncLen, pDI->pPageBuff);
        }

        if(pDI->fIsNand)
        {
            if(pDI->fRawSpares)
            {
                if(DclOsFileRead(pDI->pPageBuff, 1, pDI->uSpareSize, pDI->hFile) != pDI->uSpareSize)
                    break;

                DclHexDump("Spare:", HEXDUMP_UINT8|HEXDUMP_NOOFFSET,
                    pDI->uSpareSize, pDI->uSpareSize, pDI->pPageBuff);
            }
            else
            {
                /*  If we don't want to display the spare data, we still
                    need to advance the pointer past the it.
                */
                DclFsFileSeek(pDI->hFile, pDI->uSpareSize, DCLFSFILESEEK_CUR);
            }
        }

        ulPage++;
    }

    return;
}


/*-------------------------------------------------------------------
    Local: DumpRawSpares()

    Parameters:

    Return Value:	
        None.
-------------------------------------------------------------------*/
static void DumpRawSpares(
    FXDUMPINFO *pDI)
{
    D_UINT32    ulPage = 0;

    DclAssert(pDI);
    DclAssert(pDI->fIsNand);
    DclAssert(pDI->ulPageSize);

    DclPrintf("\nRaw spare area dump at offset %lU KB.  PageSize=%lU SpareSize=%U\n",
        pDI->ulOffsetKB, pDI->ulPageSize, pDI->uSpareSize);

    if(pDI->ulAbsoluteOffset)
        DclPrintf("Starting at absolute file offset %lU\n", pDI->ulAbsoluteOffset);

    DclPrintf("\n");

    if(!VerifyFileSize(pDI))
        return;

    DclFsFileSeek(pDI->hFile, pDI->ulAbsoluteOffset, DCLFSFILESEEK_SET);

    while(!DclOsFileEOF(pDI->hFile))
    {
        char    szLine[12];

        DclFsFileSeek(pDI->hFile, pDI->ulPageSize, DCLFSFILESEEK_CUR);

        if(DclOsFileRead(pDI->pPageBuff, 1, pDI->uSpareSize, pDI->hFile) != pDI->uSpareSize)
            break;

        if(pDI->uSpareSize <= 16)
        {
            DclSNPrintf(szLine, sizeof(szLine), "%6lU: ", ulPage);
            DclHexDump(szLine, HEXDUMP_UINT8|HEXDUMP_NOOFFSET, pDI->uSpareSize, pDI->uSpareSize, pDI->pPageBuff);
        }
        else
        {
            DclSNPrintf(szLine, sizeof(szLine), "%6lU:\n", ulPage);
            DclHexDump(szLine, HEXDUMP_UINT8|HEXDUMP_NOOFFSET, 16, pDI->uSpareSize, pDI->pPageBuff);
        }

        ulPage++;
    }

    return;
}


/*-------------------------------------------------------------------
    Local: VerifyFileSize()

    This function verifies that the file size is evenly divisible
    by the page size (inluding the spare area if NAND).

    The file pointer will be reset to the beginning of the file.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL VerifyFileSize(
    FXDUMPINFO *pDI)
{
    long        lSize;

    DclAssert(pDI);

    DclFsFileSeek(pDI->hFile, 0, DCLFSFILESEEK_END);

    lSize = DclOsFileTell(pDI->hFile);

    /*  Set the pointer to the beginning of the file
    */
    DclFsFileSeek(pDI->hFile, 0, DCLFSFILESEEK_SET);

    if(lSize % (pDI->ulPageSize+pDI->uSpareSize))
    {
        DclPrintf("The file size %ld is not evenly divisible by the page+spare size (%lU).\n",
            lSize, pDI->ulPageSize+pDI->uSpareSize);

        return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: SetSwitches()

    Parses the command line arguments and sets any globals needed
    for the tests.

    Parameters:
        argc - number of command line arguments
        argv - pointer to array of command line argument strings

    Return Value:
        ERROR_BAD_OPTION if any problems are encountered.
        ERROR_PASSED if the command line arguments are valid and recorded.
-------------------------------------------------------------------*/
static FFXSTATUS SetSwitches(
    FXDUMPINFO     *pDI,
    const char     *pszCmdLine)
{
    #define         ARGBUFFLEN 128
    D_UINT16        uIndex;
    D_UINT16        uSwitch;
    char            achArgBuff[ARGBUFFLEN];
    D_UINT16        uArgCount;

    uArgCount = DclArgCount(pszCmdLine);
    if(!uArgCount)
        return DCLSTAT_HELPREQUEST;

    /*  Start with argument number 1, argv[0] is the command line string
        for the program that is being executed.
    */
    for(uIndex = 1; uIndex <= uArgCount; uIndex++)
    {
        if(!DclArgRetrieve(pszCmdLine, uIndex, ARGBUFFLEN, achArgBuff))
        {
            DclPrintf("Bad argument!\n");
            return FFXSTAT_BADSYNTAX;
        }

        if(DclStrICmp(achArgBuff, "/BBM") == 0)
        {
            pDI->fDumpBBM = TRUE;
            pDI->fIsNand = TRUE;
            continue;
        }

        if(DclStrICmp(achArgBuff, "/VBF") == 0)
        {
            pDI->fDumpVBF = TRUE;
            continue;
        }

        if(DclStrNICmp(achArgBuff, "/PAGEDATA", 9) == 0)
        {
            pDI->fRawPages = TRUE;

            if(achArgBuff[9] == ':')
                pDI->ulPageTruncLen = DclAtoL(&achArgBuff[10]);

            continue;
        }

        if(DclStrICmp(achArgBuff, "/SPAREDATA") == 0)
        {
            pDI->fRawSpares = TRUE;
            continue;
        }

        if(DclStrNICmp(achArgBuff, "/NAND", 5) == 0)
        {
            pDI->fIsNand = TRUE;

            if(DclStrICmp(&achArgBuff[5], ":SSFDC") == 0)
            {
            }
            else if(DclStrICmp(&achArgBuff[5], ":OFFSETZERO") == 0)
            {
                pDI->fOffsetZero = TRUE;
            }
            else if(achArgBuff[5] != 0)
            {
                DclPrintf("Bad syntax in \"%s\"\n", achArgBuff);
                return FFXSTAT_BADSYNTAX;
            }
            
            continue;
        }

        if(DclStrICmp(achArgBuff, "/WRITTEN") == 0)
        {
            pDI->fWrittenOnly = TRUE;
            continue;
        }

        if(DclStrNICmp(achArgBuff, "/PAGESIZE:", 10) == 0)
        {
            pDI->ulPageSize = DclAtoL(&achArgBuff[10]);

            if(!DCLISPOWEROF2(pDI->ulPageSize) || pDI->ulPageSize < 512)
            {
                DclPrintf("The page size must be a power-of-two value at least 512 bytes in size\n\n");
                return FFXSTAT_BADSYNTAX;
            }

            continue;
        }

        if(DclStrNICmp(achArgBuff, "/OFFSET:", 8) == 0)
        {
            pDI->ulOffsetKB = DclAtoL(&achArgBuff[8]);
            continue;
        }

        uSwitch = 0;
        while(achArgBuff[uSwitch] == '/')
        {
            /*  Go to the next character to test
            */
            uSwitch++;

            switch (achArgBuff[uSwitch])
            {
                case 'f':
                case 'F':
                    if(achArgBuff[uSwitch + 1] != ':')
                        return FFXSTAT_BADSYNTAX;
                    if((achArgBuff[uSwitch + 2] != '"') && (achArgBuff[uSwitch + 2] != '\''))
                        return FFXSTAT_BADSYNTAX;

                    DclStrNCpy(pDI->szPath, &achArgBuff[uSwitch + 3],
                               DclStrLen(&achArgBuff[uSwitch + 3]) - 1);
                    uSwitch += 2;
                    uSwitch += (DclStrLen(&achArgBuff[uSwitch + 1]) + 1);
                    break;

                case '?':
                    return DCLSTAT_HELPREQUEST;

                case 'v':
                case 'V':
                    if(achArgBuff[uSwitch + 1] != ':')
                        return FFXSTAT_BADSYNTAX;

                    /*  FfxAtoI() returns an unsigned value, so we don't need
                        to check for negative.  In fact, a check for negative
                        values will cause compiler warnings.
                    */
                    if(DCL_VERBOSE_OBNOXIOUS >= DclAtoI(&achArgBuff[uSwitch + 2]))
                    {
                        /*  Type conversion OK, we know value is 2 or less
                        */
                        pDI->nVerbosity = (unsigned)DclAtoI(&achArgBuff[uSwitch + 2]);
                        uSwitch += 3;
                    }
                    else
                    {
                        DclPrintf("Verbose setting out of range (%u-%u): %s\n", 
                            DCL_VERBOSE_QUIET, DCL_VERBOSE_OBNOXIOUS, &achArgBuff[uSwitch + 2]);
                        
                        return FFXSTAT_BADSYNTAX;
                    }
                    break;
            }
        }
    }

    if(!pDI->szPath[0])
    {
        DclPrintf("\nA filename must be specified.\n");
        return FFXSTAT_BADSYNTAX;
    }

    if(!pDI->fRawPages && !pDI->fRawSpares && !pDI->fDumpBBM && !pDI->fDumpVBF)
    {
        DclPrintf("\nAt least one of the modes /PAGEDATA, /SPAREDATA, /BBM, or /VBF must be specified.\n");
        return FFXSTAT_BADSYNTAX;
    }

    if(pDI->fRawSpares && !pDI->fIsNand)
    {
        DclPrintf("\nDisplaying spare data is only valid when using NAND.\n");
        return FFXSTAT_BADSYNTAX;
    }

    if((pDI->fRawPages || pDI->fRawSpares) && pDI->fIsNand && pDI->ulPageSize == 0)
    {
        DclPrintf("\nA page size must be specified.\n");
        return FFXSTAT_BADSYNTAX;
    }

    /*  All worked fine
    */
    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: ShowUsage()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL ShowUsage(
    FFXTOOLPARAMS *pTP)
{
    DclPrintf("\nThis command dumps information derived from a FlashFX disk image.\n\n");
    DclPrintf("Usage: %s /F:'imagefile' mode [options]\n\n", pTP->dtp.pszCmdName);
    DclPrintf("Where 'mode' is one or more of the following:\n");
    DclPrintf("  /PAGEDATA[:n] Dump raw page data, display only the first 'n' bytes\n");
    DclPrintf("  /SPAREDATA    Dump raw spare area data (NAND only)\n");
    DclPrintf("  /BBM          Dump BBM information\n");
    DclPrintf("  /VBF          Dump VBF information\n");
    DclPrintf("And [options] are:\n");
    DclPrintf("  /?            This help information\n");
    DclPrintf("  /V:n          Verbosity level from 0 to 3, with 1 being the default.\n");
    DclPrintf("  /NAND[:type]  Indicate that the image is NAND rather than NOR.  The 'type'\n");
    DclPrintf("                value may be 'SSFDC' or 'OFFSETZERO' to indicate the format\n");
    DclPrintf("                of the spare area.  SSFDC is the default.\n");
    DclPrintf("  /OFFSET:n     Start offset at 'n' KB into the image\n");
    DclPrintf("  /PAGESIZE:n   The page size in bytes (required if NAND)\n");
    DclPrintf("  /WRITTEN      Display only those pages which have been written to (that\n");
    DclPrintf("                contain something other than 0xFFs).\n");
    if(pTP->dtp.pszExtraHelp)
        DclPrintf(pTP->dtp.pszExtraHelp);
    DclPrintf("VBF specific options:\n");
    FfxVbfDumpUsage();
    DclPrintf("\nNote: The filename must be specified with single or double quotes.\n\n");

    return 1;
}

