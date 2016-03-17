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

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxdmpbbm.c $
    Revision 1.8  2010/12/12 07:22:10Z  garyp
    Use standard verbosity levels.
    Revision 1.7  2009/04/01 14:47:00Z  davidh
    Function hearders updated for AutoDoc.
    Revision 1.6  2008/04/03 23:30:41Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.5  2008/01/25 17:54:31Z  Garyp
    Factored out the DisplayHeader() function.
    Revision 1.4  2008/01/13 07:27:01Z  keithg
    Function header updates to support autodoc.
    Revision 1.3  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/03/15 03:42:03Z  Garyp
    Now use DclOsFileRead() rather than DclOsFileGetChar().
    Revision 1.1  2006/04/28 18:49:50Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include "fxdmpbbm.h"

#define BBM_SIGNATURE_SIZE  8
#define BBM_SIGNATURE       {0xDB, 0xC0, 0x95, 0x77, 0x7A, 0x5C, 0xF7, 0x2C}

typedef struct
{
    D_UCHAR     acSignature[BBM_SIGNATURE_SIZE];
    D_UINT32    ulDataBlocks;
    D_UINT16    uBlockSize;
    D_UINT16    uStatus;
    D_UINT32    ulDataBlocksCopy;
    D_UINT16    uBlockSizeCopy;
    D_UINT16    uMapSize;
    D_UINT16    uInProgressIndex;
    D_UINT16    uSpareBlocks;       /* Number of spare (replacement) blocks */
    D_UINT16    uSpareBlocksCopy;
    D_UINT16    uReserved;
} DUMP_BBMMAPHEADER;


static D_BOOL   DisplayHeader(DCLFSFILEHANDLE hFile, unsigned nVerbosity, D_UINT32 ulOffset);
static void     RetrieveHeader(DCLFSFILEHANDLE hFile, DUMP_BBMMAPHEADER *pBMH);
static int      GetChar(DCLFSFILEHANDLE hFile);


/*-------------------------------------------------------------------
    Public: FfxBbmDump()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_BOOL FfxBbmDump(
    DCLFSFILEHANDLE     hFile,
    unsigned            nVerbosity)
{
    D_UCHAR             temp, i;
    D_UINT32            ulOffset = 0;
    D_UCHAR             achSig[] = BBM_SIGNATURE;

    /*  search until the end of the file
    */
    while(!DclOsFileEOF(hFile))
    {
        D_UINT32    ulHdrOffset = ulOffset;

        /*  get a byte
        */
        temp = (D_UCHAR)GetChar(hFile);
        ulOffset++;

        /*  check if this byte is the start of a signature
        */
        if(temp == achSig[0])
        {
            /*  check next n bytes for a signature
            */
            for(i = 1; i < BBM_SIGNATURE_SIZE; i++)
            {
                temp = (D_UCHAR) GetChar(hFile);
                ulOffset++;
                if(!(temp == achSig[i]))
                    break;

                /* all bytes have been checked
                */
                if(i == BBM_SIGNATURE_SIZE-1)
                {
                    DclPrintf("\nBBM Header at offset %lX:\n", ulHdrOffset);

                    return DisplayHeader(hFile, nVerbosity, ulOffset);
                }

            }
        }
    }

    DclPrintf("The BBM signature bytes were not found\n");

    return FALSE;
}


/*-------------------------------------------------------------------
    Public: RetrieveHeader()

    Byte by Byte this function fills the FULLHEADER with
    the appropriate values

    Parameters:
        pFH - pointer to Header to fill
        hFile - handle of file being scanned

    Return Value:
        none
-------------------------------------------------------------------*/
static void RetrieveHeader(
    DCLFSFILEHANDLE       hFile,
    DUMP_BBMMAPHEADER  *pBMH)
{
    DclMemSet(pBMH, 0, sizeof(*pBMH));

    pBMH->ulDataBlocks      = (((GetChar(hFile)) | (GetChar(hFile) << 8)) | (GetChar(hFile) << 16)) | (GetChar(hFile) << 24);
    pBMH->uBlockSize        =  ((GetChar(hFile)) | (GetChar(hFile) << 8));
    pBMH->uStatus           =  ((GetChar(hFile)) | (GetChar(hFile) << 8));
    pBMH->ulDataBlocksCopy  = (((GetChar(hFile)) | (GetChar(hFile) << 8)) | (GetChar(hFile) << 16)) | (GetChar(hFile) << 24);
    pBMH->uBlockSizeCopy    =  ((GetChar(hFile)) | (GetChar(hFile) << 8));
    pBMH->uMapSize          =  ((GetChar(hFile)) | (GetChar(hFile) << 8));
    pBMH->uInProgressIndex  =  ((GetChar(hFile)) | (GetChar(hFile) << 8));
    pBMH->uSpareBlocks      =  ((GetChar(hFile)) | (GetChar(hFile) << 8));
    pBMH->uSpareBlocksCopy  =  ((GetChar(hFile)) | (GetChar(hFile) << 8));
    pBMH->uReserved         =  ((GetChar(hFile)) | (GetChar(hFile) << 8));

    return;
}


/*-------------------------------------------------------------------
    Public: DisplayHeader()

    Parameters:
        hFile - handle of file being scanned

    Return Value:
        Nothing
-------------------------------------------------------------------*/
static D_BOOL DisplayHeader(
    DCLFSFILEHANDLE     hFile,
    unsigned            nVerbosity,
    D_UINT32            ulOffset)
{
    D_UINT16            uEntries;
    D_BUFFER           *pBuffer = NULL;
    D_BOOL              fProbablyBad = FALSE;
    DUMP_BBMMAPHEADER   bmh;

    /*  pack the header structure from the data in the file
    */
    RetrieveHeader(hFile, &bmh);

    DclPrintf("    Data Block Count:        %lX\n",    bmh.ulDataBlocks);
    DclPrintf("    Block Size:                  %X\n", bmh.uBlockSize);
    DclPrintf("    Status:                      %X\n", bmh.uStatus);
    DclPrintf("    Data Block Count (copy): %lX\n",    bmh.ulDataBlocksCopy);
    DclPrintf("    Block Size (copy):           %X\n", bmh.uBlockSizeCopy);
    DclPrintf("    Map Size:                    %X\n", bmh.uMapSize);
    DclPrintf("    Progress Index:              %X\n", bmh.uInProgressIndex);
    DclPrintf("    Spare Blocks:                %X\n", bmh.uSpareBlocks);
    DclPrintf("    Spare Blocks (copy):         %X\n", bmh.uSpareBlocksCopy);
    DclPrintf("    Reserved:                    %X\n", bmh.uReserved);

    ulOffset += (sizeof(bmh) - BBM_SIGNATURE_SIZE);

    uEntries = (bmh.uMapSize - sizeof(bmh)) / sizeof(D_UINT32);

    /*  Reality check...
    */
    if(!DCLISPOWEROF2(bmh.uBlockSize) ||
       !DCLISPOWEROF2(bmh.uBlockSizeCopy) ||
       !DCLISPOWEROF2(bmh.uMapSize) ||
       bmh.uReserved != 0xFFFF)
    {
        fProbablyBad = TRUE;

        DclPrintf("NOTE: This may be a bad BBM header.\n");

        if(nVerbosity == DCL_VERBOSE_NORMAL)
            DclPrintf("      Use verbosity level 2 to display the blockmap.\n");
    }

    if(nVerbosity > DCL_VERBOSE_NORMAL || (nVerbosity == DCL_VERBOSE_NORMAL && fProbablyBad == FALSE))
    {
        pBuffer = DclMemAlloc(uEntries * sizeof(D_UINT32));
        if(!*pBuffer)
        {
            DclPrintf("Out of memory!\n");
            return FALSE;
        }

        if(DclOsFileRead(pBuffer, sizeof(D_UINT32), uEntries, hFile) != uEntries)
        {
            DclPrintf("I/O error reading file\n");
            DclMemFree(pBuffer);
            return FALSE;
        }

        ulOffset += uEntries * sizeof(D_UINT32);

        DclHexDump("    BlockMap:\n", HEXDUMP_UINT32, 8, uEntries, pBuffer);

        DclMemFree(pBuffer);
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: GetChar()

    Read a character from a file.

    Parameters:
        hFile - The DCLFSFILEHANDLE value

    Return Value:
        Returns the character read, or DCLFILE_EOF if an error
        occurred or the end of the file was reached.
-------------------------------------------------------------------*/
static int GetChar(
    DCLFSFILEHANDLE   hFile)
{
    unsigned char   chr;

    DclAssert(hFile);

    if(DclOsFileRead(&chr, 1, 1, hFile) == 1)
        return (int)chr;
    else
        return DCLFILE_EOF;
}




