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

    This sample application for Windows CE demonstrates how to use the FlashFX
    FML API to read, write, and erase raw flash.  This application is designed
    to interface with an already installed FlashFX device driver, using the
    CE IOCTL interface.

    The output from this console mode application will typically go to the
    command prompt window from which the program was started.  If there is
    no command prompt capability, this application can be run from the
    Target Control window (using the 's' command), and the output will go
    to the Platform Builder debugger window.

    See the comments included in the code for the syntax to this program.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: FlashFX_FML.c $
    Revision 1.1  2009/01/14 00:42:46Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <flashfx_wince.h>
#include <fxfmlapi.h>

#define TEST_BLOCKS   (4)


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
int WINAPI WinMain(
    HINSTANCE               hInstance,
    HINSTANCE               hPrevInstance,
    LPTSTR                  ptzCmdLine,
    int                     nCmdShow)
{
    FFXVERSION              fv;
    FFXFMLHANDLE            hFML;
    FFXFMLINFO              fmlInfo;
    FFXIOSTATUS             ioStat;
    unsigned                nBlockNum;
    unsigned                nPagesPerBlock;
    unsigned char          *pPageBuff;
    unsigned char           chFill;

    /*  If any parameters at all are specified, just display the help text
    */
    if(wcslen(ptzCmdLine) != 0)
    {
        printf("This sample application demonstrates how to use the low level FlashFX FML interface\n");
        printf("to read, write, and erase raw flash.  This application is hard-coded to use the\n");
        printf("last %u erase blocks in FlashFX DISK0.\n\n", TEST_BLOCKS);
        printf("This application does not have any command-line options.  It returns 0 if all\n");
        printf("worked, or non-zero on failure.\n\n");

        return __LINE__;
    }

    /*  Retrieve the FlashFX version number and display it
    */
    if(FlashFX_VersionCheck(&fv, FALSE) != FFXSTAT_SUCCESS)
    {
        printf("Error retrieving the FlashFX version information\n");
        return __LINE__;
    }

    printf("FlashFX driver version %u.%u.%u Build %s\n",
        fv.ulVersion >> 24,
        (fv.ulVersion >> 16) & 0xFF,
        (fv.ulVersion >> 8) & 0xFF,
        fv.szBuildNum);

    if((fv.ulVersion & 0xFF) == 0x0A)
        printf("    (Driver is in an ALPHA release state)\n");
    else if((fv.ulVersion & 0xFF) == 0x0B)
        printf("    (Driver is in an BETA release state)\n");

    /*  Hard-coded to always use FlashFX DISK0
    */
    hFML = FfxFmlHandle(0);
    if(!hFML)
    {
        printf("Error retrieving FML handle for DISK0\n");
        return __LINE__;
    }

    if(FfxFmlDiskInfo(hFML, &fmlInfo) != FFXSTAT_SUCCESS)
    {
        printf("Error retrieving FML information for DISK0\n");
        return __LINE__;
    }

    printf("FlashFX DISK0 characteristics:\n");
    printf("    Total Erase Blocks:  %9lu\n",    fmlInfo.ulTotalBlocks);
    printf("    Erase Block Size:    %9lu KB\n", fmlInfo.ulBlockSize / 1024);
    printf("    Bytes per Page:      %9u\n",     fmlInfo.uPageSize);

    nPagesPerBlock = fmlInfo.ulBlockSize / fmlInfo.uPageSize;

    /*  Allocate memory for a page buffer.  We lazily don't bother
        free'ing this buffer since we're a Windows application, and
        Windows will take care of it.
    */
    pPageBuff = malloc(fmlInfo.uPageSize);
    if(!pPageBuff)
    {
        printf("Error allocating memory for the page buffer\n");
        return __LINE__;
    }

    /*  First examine and display the existing contents (first 16 bytes)
        of each of the pages in our example case.  We do this to prove
        that the results of any previous test run are preserved.
    */
    printf("Displaying the pre-existing first 16 bytes of the %u pages:\n", TEST_BLOCKS);

    for(nBlockNum = fmlInfo.ulTotalBlocks - TEST_BLOCKS;
        nBlockNum < fmlInfo.ulTotalBlocks;
        nBlockNum++)
    {
        FMLREAD_PAGES(hFML, nBlockNum * nPagesPerBlock, 1, pPageBuff, ioStat);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        {
            printf("Error %lX reading page %u\n", ioStat.ffxStat, nBlockNum * nPagesPerBlock);
            return __LINE__;
        }

        printf("  Block %u (DISK0 page %u): %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
            nBlockNum, nBlockNum * nPagesPerBlock,
            *(pPageBuff+0),  *(pPageBuff+1),  *(pPageBuff+2),  *(pPageBuff+3),
            *(pPageBuff+4),  *(pPageBuff+5),  *(pPageBuff+6),  *(pPageBuff+7),
            *(pPageBuff+8),  *(pPageBuff+9),  *(pPageBuff+10), *(pPageBuff+11),
            *(pPageBuff+12), *(pPageBuff+13), *(pPageBuff+14), *(pPageBuff+15));
    }

    /*  Use a starting fill character based off the previous contents
        of the last page in the range.
    */
    chFill = *pPageBuff;

    /*  In turn, erase each block, and reprogram the first page with the
        new fill character.
    */
    for(nBlockNum = fmlInfo.ulTotalBlocks - TEST_BLOCKS;
        nBlockNum < fmlInfo.ulTotalBlocks;
        nBlockNum++)
    {
        FMLERASE_BLOCKS(hFML, nBlockNum, 1, ioStat);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        {
            printf("Error %lX erasing block %u\n", ioStat.ffxStat, nBlockNum);
            return __LINE__;
        }

        /*  Adjust the fill character to use, but never fill with 0xFF
        */
        if(chFill == 0xFF)
            chFill = 0x11;
        else if(chFill == 0xEE)
            chFill = 0x00;
        else
            chFill += 0x11;

        memset(pPageBuff, chFill, fmlInfo.uPageSize);

        FMLWRITE_PAGES(hFML, nBlockNum * nPagesPerBlock, 1, pPageBuff, ioStat);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        {
            printf("Error %lX writing page %u\n", ioStat.ffxStat, nBlockNum * nPagesPerBlock);
            return __LINE__;
        }

        printf("Wrote 0x%02X to the first page in block %u (DISK0 page %u)\n",
            chFill, nBlockNum, nBlockNum * nPagesPerBlock);
    }

    return 0;
}


