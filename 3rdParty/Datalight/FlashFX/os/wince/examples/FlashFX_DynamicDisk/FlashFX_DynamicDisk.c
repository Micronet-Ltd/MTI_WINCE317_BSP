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
    $Log: FlashFX_DynamicDisk.c $
    Revision 1.4  2009/08/02 16:59:17Z  garyp
    Fixed a broken initializer.
    Revision 1.3  2009/07/24 00:20:24Z  garyp
    Updated to work with the new version number format.
    Revision 1.2  2009/03/22 21:37:30Z  garyp
    Merged from the v4.0 branch.
    Revision 1.1.1.3  2009/03/22 21:37:30Z  garyp
    Updated to use a structure which was renamed to accommodate the
    auto-documentation system.
    Revision 1.1.1.2  2009/01/18 20:57:33Z  garyp
    Updated to use FlashFX_Open/Close() at the outermost layer, which
    internally creates/destroys a DCL instance.
    Revision 1.1  2009/01/14 00:42:48Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <string.h>

#include <flashfx_wince.h>
#include <fxdriverfwapi.h>
#include <fxfmlapi.h>


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
int WINAPI WinMain(
    HINSTANCE               hInstance,
    HINSTANCE               hPrevInstance,
    LPTSTR                  ptzCmdLine,
    int                     nCmdShow)
{
    FFXVERSION              fv;
    FFXSTATUS               ffxStat;
    FFXFMLHANDLE            hFML;
    FFXFMLDEVINFO           fmlDevInfo;
    FFXFMLINFO              fmlInfo;
    FFXIOSTATUS             ioStat;
    FFXDISKCONFIG           conf;
    FFXDISKHANDLE           hDisk;
    FFXREQHANDLE            hReq = 0;
    unsigned char          *pPageBuff;
    unsigned                nDisk;
    unsigned                nDev;
    unsigned long           ulBlockOffset;
    unsigned long           ulBlockCount;
    unsigned                nn;
    int                     iReturn = 0;
    char                    cRevLetter[2] = {0};

    if( (wcslen(ptzCmdLine) == 0) ||
        (_wcsicmp(ptzCmdLine, TEXT("?")) == 0) ||
        (_wcsicmp(ptzCmdLine, TEXT("/?")) == 0) )
    {
        goto DisplayHelp;
    }

    ffxStat = FlashFX_Open(UINT_MAX, NULL, &hReq);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        printf("Error opening the FlashFX requestor interface, Status=0x%08lX\n", ffxStat);
        return __LINE__;
    }

    /*  Retrieve the FlashFX version number and display it
    */
    if(FlashFX_VersionCheck(&fv, FALSE) != FFXSTAT_SUCCESS)
    {
        printf("Error retrieving the FlashFX version information\n");
        iReturn = __LINE__;
        goto Cleanup;
    }

    if((fv.ulVersion & 0xFF) != 0x0)
        cRevLetter[0] = (char)(fv.ulVersion & 0xFF);

    printf("FlashFX driver version %u.%u.%u%s Build %s\n",
        fv.ulVersion >> 24,
        (fv.ulVersion >> 16) & 0xFF,
        (fv.ulVersion >> 8) & 0xFF,
        &cRevLetter[0],
        fv.szBuildNum);

    memset(&conf, 0, sizeof(conf));

    /*  Skip white space
    */
    while(ptzCmdLine[0] && ptzCmdLine[0] == L' ')
        ptzCmdLine++;

    if(!ptzCmdLine[0])
        goto BadSyntax;

    /*  Parse out the DISKn number
    */
    if(_wcsnicmp(ptzCmdLine, L"DISK", 4) != 0)
        goto BadSyntax;

    ptzCmdLine += 4;

    if(ptzCmdLine[0] < L'0' || ptzCmdLine[0] > L'9' || ptzCmdLine[1] != L' ')
        goto BadSyntax;

    nDisk = ptzCmdLine[0] - L'0';

    ptzCmdLine++;

    /*  Skip white space
    */
    while(ptzCmdLine[0] && ptzCmdLine[0] == L' ')
        ptzCmdLine++;

    if(!ptzCmdLine[0])
        goto BadSyntax;

    /*  Parse out the DEVn number
    */
    if(_wcsnicmp(ptzCmdLine, L"DEV", 3) != 0)
        goto BadSyntax;

    ptzCmdLine += 3;

    if(ptzCmdLine[0] < L'0' || ptzCmdLine[0] > L'9' || ptzCmdLine[1] != L' ')
        goto BadSyntax;

    nDev = ptzCmdLine[0] - L'0';

    ptzCmdLine++;

    /*  Skip white space
    */
    while(ptzCmdLine[0] && ptzCmdLine[0] == L' ')
        ptzCmdLine++;

    if(!ptzCmdLine[0])
        goto BadSyntax;

    if(ptzCmdLine[0] < L'0' || ptzCmdLine[0] > L'9')
        goto BadSyntax;

    ulBlockOffset = _wtol(ptzCmdLine);

    while(ptzCmdLine[0] >= L'0' && ptzCmdLine[0] <= L'9')
        ptzCmdLine++;

    /*  Skip white space
    */
    while(ptzCmdLine[0] && ptzCmdLine[0] == L' ')
        ptzCmdLine++;

    if(!ptzCmdLine[0])
        goto BadSyntax;

    if(ptzCmdLine[0] < L'0' || ptzCmdLine[0] > L'9')
        goto BadSyntax;

    ulBlockCount = _wtol(ptzCmdLine);

    while(ptzCmdLine[0] >= L'0' && ptzCmdLine[0] <= L'9')
        ptzCmdLine++;

    /*  Skip white space
    */
    while(ptzCmdLine[0] && ptzCmdLine[0] == L' ')
        ptzCmdLine++;

    if(ptzCmdLine[0])
        goto BadSyntax;

    if(FfxFmlDeviceInfo(nDev, &fmlDevInfo) != FFXSTAT_SUCCESS)
    {
        printf("Error retrieving FML information for DEV%u\n", conf.nDiskNum);
        iReturn = __LINE__;
        goto Cleanup;
    }

    conf.nDiskNum = nDisk;
    conf.DiskSettings.nDevNum = nDev;
    conf.DiskSettings.ulOffsetKB = ulBlockOffset * fmlDevInfo.ulBlockSize / 1024;
    conf.DiskSettings.ulLengthKB = ulBlockCount * fmlDevInfo.ulBlockSize / 1024;
    conf.DiskSettings.nFlags = 0;
    conf.fInitedDiskSettings = TRUE;
    conf.AllocSettings.nAllocator = FFX_ALLOC_NONE;
    conf.fInitedAllocatorSettings = TRUE;

    hDisk = FfxDriverDiskCreateParam(NULL, &conf, NULL);
    if(!hDisk)
    {
        printf("Error creating DISK%u\n", conf.nDiskNum);
        iReturn = __LINE__;
        goto Cleanup;
    }

    /*  Get the FML handle so we can access this Disk using the FML API
    */
    hFML = FfxFmlHandle(conf.nDiskNum);
    if(!hFML)
    {
        printf("Error retrieving FML handle for DISK%u\n", conf.nDiskNum);
        iReturn = __LINE__;
        goto Cleanup;
    }

    ffxStat = FfxFmlDiskInfo(hFML, &fmlInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        printf("Error %lX retrieving FML information for DISK%u\n", ffxStat, conf.nDiskNum);
        iReturn = __LINE__;
        goto Cleanup;
    }

    printf("FlashFX DISK%u characteristics:\n", conf.nDiskNum);
    printf("    Total Erase Blocks:  %9lu\n",    fmlInfo.ulTotalBlocks);
    printf("    Erase Block Size:    %9lu KB\n", fmlInfo.ulBlockSize / 1024);
    printf("    Bytes per Page:      %9u\n",     fmlInfo.uPageSize);

    /*  Allocate memory for a page buffer.  We lazily don't bother
        free'ing this buffer since we're a Windows application, and
        Windows will take care of it.
    */
    pPageBuff = malloc(fmlInfo.uPageSize);
    if(!pPageBuff)
    {
        printf("Error allocating memory for the page buffer\n");
        iReturn = __LINE__;
        goto Cleanup;
    }

    /*  Read the first page of the first block and dump it out.
    */
    FMLREAD_PAGES(hFML, 0, 1, pPageBuff, ioStat);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
    {
        printf("Error %lX reading page 0\n", ioStat.ffxStat);
        iReturn = __LINE__;
        goto Cleanup;
    }

    for(nn=0; nn<fmlInfo.uPageSize; nn+=16)
    {
        printf("Offset %04u: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
            nn,
            *(pPageBuff+0),  *(pPageBuff+1),  *(pPageBuff+2),  *(pPageBuff+3),
            *(pPageBuff+4),  *(pPageBuff+5),  *(pPageBuff+6),  *(pPageBuff+7),
            *(pPageBuff+8),  *(pPageBuff+9),  *(pPageBuff+10), *(pPageBuff+11),
            *(pPageBuff+12), *(pPageBuff+13), *(pPageBuff+14), *(pPageBuff+15));

        pPageBuff += 16;
    }

    ffxStat = FfxDriverDiskDestroy(hDisk);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        printf("Error %lX destroying the Diskn", ffxStat);
        iReturn = __LINE__;
    }
    else
    {
        iReturn = 0;
    }

  Cleanup:
    if(hReq)
        FlashFX_Close(hReq);

    /*  Returning 0 indicates that everything worked.
    */
    return iReturn;

  BadSyntax:

    if(hReq)
        FlashFX_Close(hReq);

    printf("Syntax error in \"%S\"\n\n", ptzCmdLine);

  DisplayHelp:
    printf("This sample application demonstrates how to dynamically create and destroy a\n");
    printf("FlashFX Disk.\n\n");
    printf("Syntax:  DynamicDisk DISKn DEVn BlockOffset BlockCount\n\n");
    printf("Where:\n");
    printf("        DISKn - Specifies the Disk number to use from 0 to FFX_MAX_DISKS.\n");
    printf("         DEVn - Specifies the Device upon which the Disk will be created,\n");
    printf("                where the Device number is a value from 0 to FFX_MAX_DEVICES.\n");
    printf("  BlockOffset - Specifies the erase block offset into the Device where the\n");
    printf("                Disk is to start.\n");
    printf("   BlockCount - Specifies the length of the Disk in erase blocks.\n\n");
    printf("Notes:\n");
    printf(" - The Device must already be created, and there must be at least one pre-\n");
    printf("   existing Disk already in use.\n");
    printf(" - The new Disk definition cannot overlap any existing Disks.\n\n");

    return __LINE__;
}


