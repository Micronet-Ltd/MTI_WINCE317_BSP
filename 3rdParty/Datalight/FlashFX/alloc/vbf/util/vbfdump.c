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
    $Log: vbfdump.c $
    Revision 1.18  2010/12/22 20:52:34Z  garyp
    Updated to build cleanly in release mode.
    Revision 1.17  2010/12/14 13:09:55Z  glenns
    Fix typo in previous checkin
    Revision 1.16  2010/12/14 04:47:22Z  glenns
    Added typecast to fix compiler warning.
    Revision 1.15  2010/12/12 07:21:36Z  garyp
    Added support for OffsetZero style flash.  Now use standard verbosity
    levels.
    Revision 1.14  2009/12/23 23:49:08Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.13  2009/12/22 22:49:11Z  billr
    Correct previous checkin: don't include a header that doesn't exist yet.
    Revision 1.12  2009/12/21 17:44:43Z  billr
    Fix one-byte buffer overrun (thank you, gcc!).
    Revision 1.11  2009/03/31 16:39:56Z  davidh
    Function headers updated for AutoDoc
    Revision 1.10  2009/02/18 04:11:17Z  keithg
    Added explicit void of unused formal parameters.
    Revision 1.9  2008/04/03 23:30:51Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.8  2008/01/13 07:20:42Z  keithg
    Function header updates to support autodoc.
    Revision 1.7  2007/11/29 20:02:05Z  jimmb
    Initialized pointer in FfxVbfDump.
    This removed GNU compiler warning/error
    Revision 1.6  2007/11/03 23:49:29Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2007/07/20 18:01:21Z  Garyp
    Fixed a problem in the unit header dump code which was not properly
    handling both the old and new style of specifying the region number.
    Revision 1.4  2007/04/17 22:25:06Z  timothyj
    Changed to use large flash array compatible regionNumber instead of byte
    offset clientAddress.
    Revision 1.3  2007/03/15 03:42:03Z  Garyp
    Now use DclOsFileRead() rather than DclOsFileGetChar().
    Revision 1.2  2006/08/30 23:31:19Z  pauli
    Initialized a variable potentially used uninitialized.
    Revision 1.1  2006/05/08 07:40:14Z  Garyp
    Initial revision
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
    Updated to use datalight's _sysmalloc and _sysfree wrappers
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
#include <fxnandapi.h>
#include <vbfdump.h>
#include <vbfunit.h>

#define STATUS_MASK         0x0000F000U 
#define STATUS_FREE         0x0000F000U 
#define STATUS_VALID        0x00004000U 
#define STATUS_INVALID      0x00008000U 
#define STATUS_EUH          0x00002000U     /* EUH signature found */
#define STATUS_METATAG      0x00001000U     /* NAND metatag found */
#define STATUS_DISCARDED    0x00000000U 
#define ADDRESS_MASK        0x00000FFFU 
#define OLDEUHSIGNATURE     0x165545E2U     /* pre v3.0 EUH signature */
#define NEWEUHSIGNATURE         0x48E2U     /* v3.0 or later EUH signature */
#define NANDMETATAG         0x19505FF0U     /* NAND metadata tag */

/*  Size and contents of the signature found at the start of each unit
*/
#define VBF_SIGNATURE ("\xCC\xDD" "DL_FS4.00\xFF\xFF\xFF\xFF\xFF")

/*  This is the unit header structure that will exist at the
    start of each unit.  It describes information about the
    entire partition and the specific unit it resides in.
*/
typedef struct
{
    D_UCHAR     signature[VBF_SIGNATURE_SIZE];
    EUH         euh;
    D_UINT16    checksum;
} FULLHEADER;

#define FH_SIZE     (VBF_SIGNATURE_SIZE + VBF_DISK_EUH_SIZE + sizeof(D_UINT16))

/*  this structure contains all the data the prints as the summary
*/
typedef struct
{
    D_UINT32    ulTotalValidAllocs;
    D_UINT32    ulTotalSystemAllocs;
    D_UINT32    ulTotalDiscardedAllocs;
    D_UINT32    ulTotalFreeAllocs;
    D_UINT32    ulTotalMetaTags;
    D_UINT32    ulTotalEUHeaders;
    D_UINT32    ulTotalInvalidAllocs;
    D_UINT32    ulTotalInvalidHeaders;
    D_UINT32    serialNumber;       /* the partition serial number */
    D_UINT32    lnuTotal;           /* number of units in this partition */
    D_UINT16    numSpareUnits;      /* number of spare units available */
    D_UINT16    uAllocBlockSize;    /* allocation block size (must be fixed) */
    D_UINT16    lnuPerRegion;       /* units in a region */
    D_UINT16    uReserved;          /* reserved, used to be partition start LNU (pre v3.0) */
    D_UINT16    uUnitTotalBlocks;   /* size in blocks of an erase unit */
    D_UINT16    unitClientBlocks;   /* client space in unit (# of blocks */
    D_UINT16    unitDataBlocks;     /* # of blocks available for allocation */
} SUMMARYINFO;

typedef struct
{
    unsigned        nVerbosity;         /* verbosity level */
    D_BOOL          fIsNand;            /* is this a NAND image */
    D_BOOL          fOffsetZero;        /* NAND is OFFSETZERO rather than SSFDC */
    D_UINT16        fShowAllocations;   /* determine which allocation to use */
    D_UINT32        address;            /* keeps track of the linear address being read */
    /*  TBD: Not LFA compliant
    */
    D_UINT32        ulOffset;           /* keeps track of where flash array begins */
    DCLFSFILEHANDLE hFile;
    char           *pszCmdLine;
    SUMMARYINFO     sum;
} DUMPINFO;

static D_BOOL       GetVbfUnitHeader(    DUMPINFO *pDI, FULLHEADER *pFH, D_UINT32 **pAllocs);
static D_UINT16     Checksum(            const void *pData, ptrdiff_t length);
static FFXSTATUS    RetrieveEUH(         FULLHEADER *pFH, DCLFSFILEHANDLE hFile);
static void         PrintVbfEuh(         DUMPINFO *pDI, FULLHEADER *pFH);
static void         SetStats(            DUMPINFO *pDI, FULLHEADER *pFH);
static void         PrintSummary(        DUMPINFO *pDI);
static void         PrintNANDAllocs(     DUMPINFO *pDI, D_UINT32 *pAllocs);
static void         PrintNORAllocs(      DUMPINFO *pDI, D_UINT32 *pAllocs);
static void         RetrieveAllocations( DUMPINFO *pDI, D_UINT32 *pAllocs, FULLHEADER *pFH);
static FFXSTATUS    SetSwitches(         DUMPINFO *pDI, char *pszCmdLine);
static int          GetChar(             DCLFSFILEHANDLE hFile);
static FFXSTATUS    Read16(              DCLFSFILEHANDLE hFile, D_UINT16 *puResult);
static FFXSTATUS    Read32(              DCLFSFILEHANDLE hFile, D_UINT32 *pulResult);


/*-------------------------------------------------------------------
    Protected: FfxVbfDump()

    Dump VBF information from a FlashFX image file.

    WARNING! This function ONLY works with images which use the 
             FlashFX standard "SSFDC" or "OFFSETZERO" style spare
             areas.  Any other spare area format will result in 
             invalid output being displayed.

    Parameters:
        hFile      - The file handle
        ulOffsetKB - The offset into the file in KB (not currently used)
        ulLengthKB - The length to examine in KB (not currently used)
        fIsNand    - Must be TRUE if the image is NAND
        nVerbosity - The information verbosity level to use
        pszCmdLine - A pointer to the command-line string.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
/*  TBD: Not LFA Compliant
*/
FFXSTATUS FfxVbfDump(
    DCLFSFILEHANDLE hFile,
    D_UINT32        ulOffsetKB,
    D_UINT32        ulLengthKB,
    D_BOOL          fIsNand,
    D_BOOL          fOffsetZero,
    unsigned        nVerbosity,
    char           *pszCmdLine)
{
    FULLHEADER      fh;
    D_UINT32       *pAllocs =  NULL;
    DUMPINFO        di = {0};
    FFXSTATUS       ffxStat;

    (void) ulLengthKB;

    DclAssert(hFile);
    DclAssert(pszCmdLine);

    di.hFile = hFile;
    di.ulOffset = ulOffsetKB * 1024;
/*    di.ulLengthKB = ulLengthKB; */
    di.fIsNand = fIsNand;
    di.fOffsetZero = fOffsetZero;
    di.nVerbosity = nVerbosity;
    di.pszCmdLine = pszCmdLine;

    /*  put the signature into header buffer
    */
    DclAssert(sizeof(fh.signature) == DclStrLen(VBF_SIGNATURE));
    DclMemCpy(fh.signature, VBF_SIGNATURE, sizeof(fh.signature));

    ffxStat = SetSwitches(&di, di.pszCmdLine);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    /*  seek to the start of the image
    */
/*  if(di.ulOffset)
        DclFsFileSeek(di.hFile, di.ulOffset, DCLFSFILESEEK_SET);
*/
    while(GetVbfUnitHeader(&di, &fh, &pAllocs))
    {
        PrintVbfEuh(&di, &fh);

        SetStats(&di, &fh);

        if(di.fIsNand)
            PrintNANDAllocs(&di, pAllocs);
        else
            PrintNORAllocs(&di, pAllocs);

        DclMemFree(pAllocs);
    };

    PrintSummary(&di);

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: GetVbfUnitHeader()

    Attempt to find a FULLHEADER and if found, fill in the values
    erase unit header structure.   If found validate checksum and
    retrieve allocations.

    Parameters:
        pDI     - A pointer to the DUMPINFO structure to use
        pFH     - A pointer to the FULLHEADER structure to fill
        pAllocs - A pointer to a pointer to an array of 32 bit
                  allocations.

    Return Value:
        Returns TRUE if successful, FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL GetVbfUnitHeader(
    DUMPINFO   *pDI,
    FULLHEADER *pFH,
    D_UINT32  **pAllocs)
{
    D_UCHAR     temp, i;
    D_BOOL      fFoundEUH = FALSE;

    /*  search until the end of the file
    */
    while(!DclOsFileEOF(pDI->hFile))
    {
        /*  get a byte
        */
        temp = (D_UCHAR)GetChar(pDI->hFile);
        pDI->address++;

        /*  check if this byte is the start of a signature
        */
        if(temp == pFH->signature[0])
        {
            /*  check next 15 bytes for a signature
            */
            for(i = 1; i < VBF_SIGNATURE_SIZE; i++)
            {
                temp = (D_UCHAR) GetChar(pDI->hFile);
                pDI->address++;
                if(!(temp == pFH->signature[i]))
                    break;

                /* all 16 bytes have been checked
                */
                if(i == VBF_SIGNATURE_SIZE-1)
                    fFoundEUH = TRUE;
            }
        }

        if(fFoundEUH)
        {
            FFXSTATUS   ffxStat;
            
            /*  we found a header
            */

            /*  pack the header structure from the data in the file
            */
            ffxStat = RetrieveEUH(pFH, pDI->hFile);
            DclAssert(ffxStat == FFXSTAT_SUCCESS);
            (void)ffxStat;

            pDI->address += (FH_SIZE - VBF_SIGNATURE_SIZE);

            /*  Only get allocations if checksum is valid.  We are using
                unitDataBlocks as number of allocations so we need to make
                sure this value is correct to avoid trashing memory.
            */
            if(pFH->checksum != Checksum(pFH, (FH_SIZE - 2)))
            {
                DclPrintf("ERROR: checksum mismatch for EUH at offset %lX. Expected %X Found %X\n",
                    pDI->address - FH_SIZE, pFH->checksum, Checksum(pFH, FH_SIZE-2));

                fFoundEUH = FALSE;

                continue;
            }

            /*  Allocate enough space for all the allocations plus one
                because the first entry will store the length.
            */
            *pAllocs = DclMemAlloc(sizeof(*pAllocs) * (pFH->euh.uUnitTotalBlocks + 1));
            if(!*pAllocs)
            {
                DclPrintf("Out of memory!\n");
                return FALSE;
            }

            /*  fill allocation structure
            */
            RetrieveAllocations(pDI, *pAllocs, pFH);

            return TRUE;
        }
    }

    return FALSE;
}


/*-------------------------------------------------------------------
    Local: Checksum()

    Calculates a sixteen bit sum of an object.

    Parameters:
        pData  - A pointer to some object
        length - The size of the object

    Return Value:
       The sum of the unsigned chars comprising the object.
-------------------------------------------------------------------*/
static D_UINT16 Checksum(
    const void     *pData,
    ptrdiff_t       length)
{
    D_UINT16        sum = 0;
    unsigned char  *p = (unsigned char*)pData;

    while(length--)
    {
        sum += *p++;
    }

    return sum;
}


/*-------------------------------------------------------------------
    Local: RetrieveEUH()

    This function reads the EUH from the image into the supplied
    FULLHEADER structure.

    Parameters:
        pFH   - A pointer to Header to fill
        hFile - The file handle

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS RetrieveEUH(
    FULLHEADER     *pFH,
    DCLFSFILEHANDLE hFile)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;
    
    ffxStat |= Read32(hFile, &pFH->euh.regionNumber);
    ffxStat |= Read32(hFile, (D_UINT32 *)(&pFH->euh.eraseCount));
    ffxStat |= Read32(hFile, &pFH->euh.serialNumber);
    ffxStat |= Read32(hFile, &pFH->euh.ulSequenceNumber);
    ffxStat |= Read32(hFile, &pFH->euh.lnuTotal);
    ffxStat |= Read32(hFile, &pFH->euh.lnuTag);
    
    ffxStat |= Read16(hFile, &pFH->euh.numSpareUnits);
    ffxStat |= Read16(hFile, &pFH->euh.uAllocBlockSize);
    ffxStat |= Read16(hFile, &pFH->euh.lnuPerRegion);
    ffxStat |= Read16(hFile, &pFH->euh.uReserved);
    ffxStat |= Read16(hFile, &pFH->euh.uUnitTotalBlocks);
    ffxStat |= Read16(hFile, &pFH->euh.unitClientBlocks);
    ffxStat |= Read16(hFile, &pFH->euh.unitDataBlocks);
    ffxStat |= Read16(hFile, &pFH->checksum);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: PrintVbfEuh()

    Display the contents of the EUH, depending on the verbosity
    level.

    Parameters:
        pDI - A pointer to the DUMPINFO structure to use
        pFH - A pointer to the FULLHEADER structure to print

    Return Value:
        None
-------------------------------------------------------------------*/
static void PrintVbfEuh(
    DUMPINFO       *pDI,
    FULLHEADER     *pFH)
{
    static D_UINT32 ulUnits = 0;
    D_UINT16        uSpareLen = 0;

    if(pDI->fIsNand)
        uSpareLen = 16;

    if(pDI->nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        if(ulUnits == 0 || pDI->fShowAllocations)
        {
            if(!pDI->fShowAllocations)
                DclPrintf("\nVBF Unit Information");

            DclPrintf("\nLinUnit ClientAddr Region Erases   LnuTag   SeqNum ");

            if(pDI->nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("SerialNum   LnuTotal  Spares AlcLen lnu/Rg Resrvd TotBlk CliBlk DatBlk ChkSum \n");
            else
                DclPrintf("\n");
        }
    }
    ulUnits++;

    if(pDI->nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        #define REGBUFF (6)
        char    szRegion[REGBUFF+1];

        if(pFH->euh.regionNumber == D_UINT32_MAX)
        {
            DclStrNCpy(szRegion, "  None", REGBUFF+1);
        }
        else
        {
            if(pFH->euh.regionNumber & 0x80000000)
            {
                /*  If the high bit is set then the remainder of the field is
                    the region number.
                */
                DclSNPrintf(szRegion, REGBUFF+1, "%6lU", pFH->euh.regionNumber & ~0x80000000);
            }
            else
            {
                /*  High bit is not set, so convert the client address to
                    a region number.
                */
                DclSNPrintf(szRegion, REGBUFF+1, "%6lU", pFH->euh.regionNumber /
                    (pFH->euh.unitClientBlocks * pFH->euh.uAllocBlockSize * pFH->euh.lnuPerRegion));
            }
        }

        DclPrintf("%7lU %lX %s %6lU %lX %6lD ",
            (pDI->address - VBF_SIGNATURE_SIZE) / (pFH->euh.uUnitTotalBlocks * (pFH->euh.uAllocBlockSize + uSpareLen)),
            pFH->euh.regionNumber,
            szRegion,
            pFH->euh.eraseCount,
            pFH->euh.lnuTag,
            pFH->euh.ulSequenceNumber);

        if(pDI->nVerbosity >= DCL_VERBOSE_LOUD)
        {
            DclPrintf("%lX %lX %X %X %X %X %X %X %X %X\n",
                pFH->euh.serialNumber,
                pFH->euh.lnuTotal,
                pFH->euh.numSpareUnits,
                pFH->euh.uAllocBlockSize,
                pFH->euh.lnuPerRegion,
                pFH->euh.uReserved,
                pFH->euh.uUnitTotalBlocks,
                pFH->euh.unitClientBlocks,
                pFH->euh.unitDataBlocks,
                pFH->checksum);
        }
        else
        {
            DclPrintf("\n");
        }

    }
}


/*-------------------------------------------------------------------
    Local: SetStats()

    When first called it initializes the summary information
    to what is contained in the FULLHEADER.  After the first
    call it compares each value in the header with what was
    saved earlier and depending on verbose level it warns if
    there are differences.

    Parameters:
        pDI - A pointer to the DUMPINFO structure to use
        pFH - A pointer to the FULLHEADER structure to use

    Return Value:
        None
-------------------------------------------------------------------*/
static void SetStats(
    DUMPINFO   *pDI,
    FULLHEADER *pFH)
{
    D_UCHAR     fError = FALSE;

    if(pDI->sum.serialNumber == 0)
    {
        /*  assume the first entry has good data (if checksum matches)
        */
        pDI->sum.serialNumber       = pFH->euh.serialNumber;
        pDI->sum.lnuTotal           = pFH->euh.lnuTotal;
        pDI->sum.numSpareUnits      = pFH->euh.numSpareUnits;
        pDI->sum.uAllocBlockSize    = pFH->euh.uAllocBlockSize;
        pDI->sum.lnuPerRegion       = pFH->euh.lnuPerRegion;
        pDI->sum.uReserved          = pFH->euh.uReserved;
        pDI->sum.uUnitTotalBlocks   = pFH->euh.uUnitTotalBlocks;
        pDI->sum.unitClientBlocks   = pFH->euh.unitClientBlocks;
        pDI->sum.unitDataBlocks     = pFH->euh.unitDataBlocks;
    }
    else
    {
        if(pDI->sum.serialNumber != pFH->euh.serialNumber)
        {
            DclPrintf("ERROR: serial number %lX doesn't match %lX\n",
                      pDI->sum.serialNumber, pFH->euh.serialNumber);
            fError = TRUE;
        }
        if(pDI->sum.lnuTotal != pFH->euh.lnuTotal)
        {
            DclPrintf("ERROR: lnuTotal %lX doesn't match %lX\n",
                      pDI->sum.lnuTotal, pFH->euh.lnuTotal);
            fError = TRUE;
        }
        if(pDI->sum.numSpareUnits != pFH->euh.numSpareUnits)
        {
            DclPrintf("ERROR: number of spare units %lX doesn't match %lX\n",
                 pDI->sum.numSpareUnits, pFH->euh.numSpareUnits);
            fError = TRUE;
        }
        if(pDI->sum.uAllocBlockSize != pFH->euh.uAllocBlockSize)
        {
            DclPrintf("ERROR: Alloc Block Size %lX doesn't match %lX\n",
                      pDI->sum.uAllocBlockSize, pFH->euh.uAllocBlockSize);
            fError = TRUE;
        }
        if(pDI->sum.lnuPerRegion != pFH->euh.lnuPerRegion)
        {
            DclPrintf("ERROR: LNU per Region %lX doesn't match %lX\n",
                      pDI->sum.lnuPerRegion, pFH->euh.lnuPerRegion);
            fError = TRUE;
        }
        if(pDI->sum.uReserved != pFH->euh.uReserved)
        {
            DclPrintf("ERROR: Reserved field %lX doesn't match %lX\n",
                 pDI->sum.uReserved, pFH->euh.uReserved);
            fError = TRUE;
        }
        if(pDI->sum.uUnitTotalBlocks != pFH->euh.uUnitTotalBlocks)
        {
            DclPrintf("ERROR: Unit TotalBlocks %lX doesn't match %lX\n",
                 pDI->sum.uUnitTotalBlocks, pFH->euh.uUnitTotalBlocks);
            fError = TRUE;
        }
        if(pDI->sum.unitClientBlocks != pFH->euh.unitClientBlocks)
        {
            DclPrintf("ERROR: Unit Client Blocks %lX doesn't match %lX\n",
                 pDI->sum.unitClientBlocks, pFH->euh.unitClientBlocks);
            fError = TRUE;
        }
        if(pDI->sum.unitDataBlocks != pFH->euh.unitDataBlocks)
        {
            DclPrintf("ERROR: Unit Data Blocks %lX doesn't match %lX\n",
                 pDI->sum.unitDataBlocks, pFH->euh.unitDataBlocks);
            fError = TRUE;
        }
        if(fError)
            pDI->sum.ulTotalInvalidHeaders++;
    }

    return;
}


/*-------------------------------------------------------------------
    Local: PrintSummary()

    This function displays summary information.

    Parameters:

    Return Value:

-------------------------------------------------------------------*/
static void PrintSummary(
    DUMPINFO   *pDI)
{
    DclPrintf("\nVBF Disk Summary\n");
    DclPrintf("  Total Invalid Headers:  %10lU\n",     pDI->sum.ulTotalInvalidHeaders);
    DclPrintf("  Serial Number:          %lX\n",       pDI->sum.serialNumber);
    DclPrintf("  Unit Information\n");
    DclPrintf("    Total:                %10lU\n",     pDI->sum.lnuTotal);
    DclPrintf("    Spares:                   %6U\n",   pDI->sum.numSpareUnits);
    DclPrintf("    Units Per Region:         %6U\n",   pDI->sum.lnuPerRegion);
    DclPrintf("    Reserved (start unit):    %6U\n",   pDI->sum.uReserved);
    DclPrintf("    Total Blocks:             %6U\n",   pDI->sum.uUnitTotalBlocks);
    DclPrintf("    Client Blocks:            %6U\n",   pDI->sum.unitClientBlocks);
    DclPrintf("    Data Blocks:              %6U\n",   pDI->sum.unitDataBlocks);
    DclPrintf("  Allocation Information\n");
    DclPrintf("    Alloc Block Size:         %6U\n",   pDI->sum.uAllocBlockSize);
    DclPrintf("    Total:                %10lU\n",     pDI->sum.ulTotalSystemAllocs);
    DclPrintf("      Free:               %10lU\n",     pDI->sum.ulTotalFreeAllocs);
    DclPrintf("      Valid:              %10lU\n",     pDI->sum.ulTotalValidAllocs);
    DclPrintf("      Invalid:            %10lU\n",     pDI->sum.ulTotalInvalidAllocs);
    if(pDI->fIsNand)
    {
        DclPrintf("      EU Headers:         %10lU\n", pDI->sum.ulTotalEUHeaders);
        DclPrintf("      MetaData:           %10lU\n", pDI->sum.ulTotalMetaTags);
        DclPrintf("      Expended:           %10lU\n", pDI->sum.ulTotalDiscardedAllocs);
    }
    else
    {
        DclPrintf("      Discarded:          %10lU\n", pDI->sum.ulTotalDiscardedAllocs);
    }

    return;
}


/*-------------------------------------------------------------------
    Local: RetrieveAllocations()

    Retrieve 32-bit allocation values and place them in the pAllocs
    array.  Note that the very first entry will be a count of the
    number of allocations which follow.

    Parameters:
        pDI     - A pointer to the DUMPINFO structure to use
        pAllocs - A pointer to the allocs array
        pFH     - A pointer to the FULLHEADER structure to use

    Return Value:
        None.
-------------------------------------------------------------------*/
static void RetrieveAllocations(
    DUMPINFO   *pDI,
    D_UINT32   *pAllocs,
    FULLHEADER *pFH)
{
    unsigned    ii;
    D_UINT32    ulAllocCount;
    FFXSTATUS   ffxStat;

    if(pDI->fIsNand)
    {
        /*  Inherent assumption that a NAND spare size is always 1/32nd
            of the page size.
        */
        size_t  nSpareSize = pFH->euh.uAllocBlockSize / 32;
            
        DclAssert(NSSSFDC_TAG_LENGTH == NSOFFSETZERO_TAG_LENGTH);
        
        ulAllocCount = pFH->euh.uUnitTotalBlocks;

        *pAllocs = ulAllocCount;

        /*  seek to the next VBF block
        */
        DclFsFileSeek(pDI->hFile, (pFH->euh.uAllocBlockSize - FH_SIZE), DCLFSFILESEEK_CUR);
        pDI->address += (pFH->euh.uAllocBlockSize - FH_SIZE);

        for(ii = 0; ii < ulAllocCount; ii++)
        {
            size_t  nAdjust = NSSSFDC_TAG_LENGTH;
            
            pAllocs++;

            if(pDI->fOffsetZero)
            {
                DclFsFileSeek(pDI->hFile, NSOFFSETZERO_TAG_OFFSET, DCLFSFILESEEK_CUR);
                nAdjust += NSOFFSETZERO_TAG_OFFSET;
            }
            else
            {
                DclFsFileSeek(pDI->hFile, NSSSFDC_TAG_OFFSET, DCLFSFILESEEK_CUR);
                nAdjust += NSSSFDC_TAG_OFFSET;
            }

            ffxStat = Read32(pDI->hFile, pAllocs);            
            DclAssert(ffxStat == FFXSTAT_SUCCESS);

            /*  seek to the next vbf block
            */
            if(ii < ulAllocCount-1)
            {
                DclFsFileSeek(pDI->hFile, pFH->euh.uAllocBlockSize + (nSpareSize - nAdjust), DCLFSFILESEEK_CUR);
                pDI->address += (pFH->euh.uAllocBlockSize + nSpareSize);
            }
        }
    }
    else
    {
        ulAllocCount = pFH->euh.unitDataBlocks;

        /*  seek to the next VBF block
        */
        DclFsFileSeek(pDI->hFile, (pFH->euh.uAllocBlockSize - FH_SIZE), DCLFSFILESEEK_CUR);
        pDI->address += (pFH->euh.uAllocBlockSize - FH_SIZE);

        *pAllocs = ulAllocCount;
        for(ii = 0; ii < ulAllocCount; ii++)
        {
            pAllocs++;

            ffxStat = Read32(pDI->hFile, pAllocs);      
            DclAssert(ffxStat == FFXSTAT_SUCCESS);

            pDI->address += sizeof(*pAllocs);
        }
    }

    /*  Unused in release mode.
    */
    (void)ffxStat;

    return;
}


/*-------------------------------------------------------------------
    Local: PrintNORAllocs()

    Decodes and prints each NOR allocation depending on verbosity
    level.  The first element in the array is the number of
    allocations to print.

    Parameters:
        pDI     - A pointer to the DUMPINFO structure to use
        pAllocs - A pointer to the allocs array

    Return Value:
        None.
-------------------------------------------------------------------*/
static void PrintNORAllocs(
    DUMPINFO   *pDI,
    D_UINT32   *pAllocs)
{
    D_UINT32    i;
    D_UINT32    addressOffset, status, ulNumAllocations;
    char        toPrint;

    DclAssert(!pDI->fIsNand);

    if(pDI->fShowAllocations)
        DclPrintf("I=Invalid D=Discarded V=Valid F=Free\n");

    ulNumAllocations = *pAllocs;

    if(pDI->fShowAllocations)
        DclPrintf("    ");

    for(i = 1; i <= ulNumAllocations; i++)
    {
        pAllocs++;

        addressOffset = (*pAllocs & ADDRESS_MASK);

        if(((*pAllocs & 0x0000FFFFL) ^ 0x00003865L) ==
           ((*pAllocs & 0xFFFF0000L) >> 16))
        {
            /*  if this xor is not true the allocation is not valid
            */
            status = *pAllocs & STATUS_MASK;
        }
        else
        {
            /* check for free block
            */
            if(*pAllocs == 0xFFFFFFFFL)
                status = STATUS_FREE;
            else if(*pAllocs == 0xBFFFBFFFL)
                status = STATUS_DISCARDED;
            else
                status = STATUS_INVALID;
        }

        pDI->sum.ulTotalSystemAllocs++;

        switch (status)
        {
            case STATUS_VALID:
                pDI->sum.ulTotalValidAllocs++;
                toPrint = 'V';
                break;
            case STATUS_DISCARDED:
                pDI->sum.ulTotalDiscardedAllocs++;
                toPrint = 'D';
                break;
            case STATUS_FREE:
                pDI->sum.ulTotalFreeAllocs++;
                toPrint = 'F';
                break;
            default:
                toPrint = 'I';
                pDI->sum.ulTotalInvalidAllocs++;
                break;
        };

        if(pDI->fShowAllocations)
        {
            /*  print the entire entry if invalid
            */
            if(toPrint == 'I')
                DclPrintf("%c,%lX ", toPrint, *pAllocs);
            else
                DclPrintf("%c,0x%03lX  ", toPrint, addressOffset);

            if(i % 8 == 0)
                DclPrintf("\n    ");

            if(i == (ulNumAllocations))
                DclPrintf("\n");
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Local: PrintNANDAllocs()

    Decode and print each NAND allocation depending on verbosity
    level.  The first element in the array is the number of 
    allocations to print.

    Parameters:
        pDI     - A pointer to the DUMPINFO structure to use
        pAllocs - A pointer to the allocs array

    Return Value:
        None.
-------------------------------------------------------------------*/
static void PrintNANDAllocs(
    DUMPINFO   *pDI,
    D_UINT32   *pAllocs)
{
    D_UINT32    i;
    D_UINT32    addressOffset = 0;
    D_UINT32    status;
    D_UINT32    ulNumAllocations;

    DclAssert(pDI->fIsNand);

    ulNumAllocations = *pAllocs;

    if(pDI->fShowAllocations)
        DclPrintf("    ");

    for(i = 1; i <= ulNumAllocations; i++)
    {
        pAllocs++;

        if(i == 1 && *pAllocs == OLDEUHSIGNATURE)
        {
            status = STATUS_EUH;
        }
        else if(*pAllocs == 0xFFFFFFFFL)
        {
            status = STATUS_FREE;
        }
        else if(((((~*pAllocs) & 0x0000FF00L) >> 8) ^ (*pAllocs & 0xFF)) != ((*pAllocs & 0x00FF0000L) >> 16))
        {
            status = STATUS_INVALID;
        }
        else if((*pAllocs & 0x0000FFFF) == 0)
        {
            status = STATUS_DISCARDED;      /* really EXPENDED */
        }
        else if((*pAllocs & STATUS_VALID) == 0)
        {
            status = STATUS_INVALID;
        }
        else if(*pAllocs & 0x00008000)      /* bit only set if the block is free */
        {
            status = STATUS_INVALID;
        }
        else
        {
            addressOffset = *pAllocs & 0x00003FFF;

            status = STATUS_VALID;

            if(i == 1 && (*pAllocs & 0x0000FFFF) == NEWEUHSIGNATURE)
                status = STATUS_EUH;

            if(*pAllocs == NANDMETATAG)
                status = STATUS_METATAG;
        }

        pDI->sum.ulTotalSystemAllocs++;

        switch (status)
        {
            case STATUS_VALID:
                pDI->sum.ulTotalValidAllocs++;

                if(pDI->fShowAllocations)
                    DclPrintf("Valid:%04lX   ", addressOffset);

                break;

            case STATUS_EUH:
                pDI->sum.ulTotalEUHeaders++;

                if(pDI->fShowAllocations)
                    DclPrintf("EUH:%08lX ", *pAllocs);

                break;

            case STATUS_DISCARDED:
                pDI->sum.ulTotalDiscardedAllocs++;

                if(pDI->fShowAllocations)
                    DclPrintf("Expended     ");

                break;

            case STATUS_FREE:
                pDI->sum.ulTotalFreeAllocs++;
                if(pDI->fShowAllocations)
                    DclPrintf("Free         ");

                break;

            case STATUS_INVALID:
                pDI->sum.ulTotalInvalidAllocs++;

                if(pDI->fShowAllocations)
                    DclPrintf("BAD:%08lX ", *pAllocs);

                break;

            case STATUS_METATAG:
                pDI->sum.ulTotalMetaTags++;

                if(pDI->fShowAllocations)
                    DclPrintf("Meta: %04lX   ", (D_UINT16)*pAllocs);

                break;
        };

        if(pDI->fShowAllocations)
        {
            if(i % 8 == 0)
                DclPrintf("\n    ");
            else if(i == (ulNumAllocations))
                DclPrintf("\n");
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Local: SetSwitches()

    Parses the command line arguments and sets any globals needed
    for the tests.

    Parameters:
        pDI        - A pointer to the DUMPINFO structure to use
        pszCmdLine - A pointer to the command line string.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS SetSwitches(
    DUMPINFO       *pDI,
    char           *pszCmdLine)
{
    #define         ARGBUFFLEN 128
    D_UINT16        uIndex;
    char            achArgBuff[ARGBUFFLEN];
    D_UINT16        uArgCount;

    uArgCount = DclArgCount(pszCmdLine);

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

        if(DclStrICmp(achArgBuff, "/ALLOCS") == 0)
        {
            pDI->fShowAllocations = TRUE;
            continue;
        }
    }

    /*  All worked fine
    */
    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Protected: FfxVbfDumpUsage()

    Display usage information.

    Parameters:
        None.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxVbfDumpUsage(void)
{
    DclPrintf("  /ALLOCS       Display the allocation entry details\n");

    return;
}


/*-------------------------------------------------------------------
    Local: GetChar()

    Read a character from a file.

    Parameters:
        hFile - The DCLFSFILEHANDLE value

    Return Value:
        Returns the character read, or DCLFILE_EOF if an error
        occurred or the end of the file was reached.
-------------------------------------------------------------------*/
static int GetChar(
    DCLFSFILEHANDLE hFile)
{
    unsigned char   chr;

    DclAssert(hFile);

    if(DclOsFileRead(&chr, 1, 1, hFile) == 1)
        return (int)chr;
    else
        return DCLFILE_EOF;
}


/*-------------------------------------------------------------------
    Local: Read16()

    Parameters:
        hFile - The DCLFSFILEHANDLE value
        puResult - The location in which to store the result.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS Read16(
    DCLFSFILEHANDLE hFile,
    D_UINT16       *puResult)
{
    FFXSTATUS       ffxStat;
    D_UINT32        ulTransferred;
    D_UINT16        uResult;
    
    DclAssert(hFile);
    DclAssertWritePtr(puResult, sizeof(*puResult));

    ffxStat = DclFsFileRead(hFile, &uResult, sizeof(uResult), &ulTransferred);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    if(ulTransferred != sizeof(*puResult))
        return DCLSTAT_FS_EOF;

    DCLLITTLE(&uResult, sizeof(uResult));

    *puResult = uResult;

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: Read32()

    Parameters:
        hFile     - The DCLFSFILEHANDLE value
        pulResult - The location in which to store the result.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS Read32(
    DCLFSFILEHANDLE hFile,
    D_UINT32       *pulResult)
{
    FFXSTATUS       ffxStat;
    D_UINT32        ulTransferred;
    D_UINT32        ulResult;
    
    DclAssert(hFile);
    DclAssertWritePtr(pulResult, sizeof(*pulResult));

    ffxStat = DclFsFileRead(hFile, &ulResult, sizeof(ulResult), &ulTransferred);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    if(ulTransferred != sizeof(*pulResult))
        return DCLSTAT_FS_EOF;

    DCLLITTLE(&ulResult, sizeof(ulResult));

    *pulResult = ulResult;

    return FFXSTAT_SUCCESS;
}




