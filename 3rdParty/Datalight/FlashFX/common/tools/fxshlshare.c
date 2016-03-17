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

    This module contains functionality which is shared between the general
    FlashFX command shell, and the sub-shell created by the "FXDebug" command.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxshlshare.c $
    Revision 1.9  2010/09/13 16:25:44Z  garyp
    Fixed the Device destruction process to avoid potentially getting into
    and endless loop.
    Revision 1.8  2010/04/29 00:04:21Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.7  2009/11/28 00:31:48Z  garyp
    Cleaned up some messages -- no functional changes.
    Revision 1.6  2009/11/19 02:05:48Z  garyp
    Updated the "Disk Create" command to support the /Span option.
    Revision 1.5  2009/07/21 22:34:18Z  garyp
    Merged from the v4.0 branch.  Modified the shutdown processes to take a
    mode parameter.  Updated the Device enumeration to provide more details.
    Revision 1.4  2009/04/01 15:09:17Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.3  2009/02/17 07:52:55Z  keithg
    Added explicit void to unused function parameters.
    Revision 1.2  2009/02/09 08:41:16Z  garyp
    Updated for MKS reverse checkin problem.
    Revision 1.1.1.2  2008/12/17 18:26:39Z  garyp
    Minor datatype changes from D_UINT16 to unsigned.  No other functional
    changes.
    Revision 1.1  2008/09/06 18:14:26Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxtools.h>
#include <fxdriver.h>
#include <fxfmlapi.h>
#include <dlshell.h>
#include "fxshl.h"

#if DCLCONF_COMMAND_SHELL


static FFXSTATUS DestroyDisk(  unsigned nDiskNum);
static FFXSTATUS DestroyDevice(unsigned nDevNum);


/*-------------------------------------------------------------------
    Public: FfxShellCmdDevice()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
DCLSTATUS FfxShellCmdDevice(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FFXSTATUS       ffxStat;
    FFXDEVHANDLE    hDev = NULL;
    D_UINT32        ulDevNum;
    const char     *pStr;

    (void)hShell;
    if(ISHELPREQUEST())
        goto DisplayHelp;

    /*  If no arguments are supplied, then simply enumerate the DEVICEs
    */
    if(argc == 1)
    {
        unsigned    nCount = 0;
        unsigned    nDisks = 0;
        unsigned    nUnmapped = 0;

        do
        {
            hDev = FfxDriverDeviceEnumerate(NULL, hDev);
            if(hDev)
            {
                D_UINT32        ulBlock;
                D_UINT32        ulCount;
                FFXDEVGEOMETRY  geo;

                ffxStat = FfxDriverDeviceGeometry(hDev, &geo);
                if(ffxStat != FFXSTAT_SUCCESS)
                {
                    DclError();
                    break;
                }

                DclPrintf("DEV%u: Type=%u ResvBlocks=%lU UsableBlocks=%lU BlockSize=%lUKB PageSize=%u%s\n",
                    geo.nDevNum,          geo.nDevType,
                    geo.ulReservedBlocks, geo.ulTotalBlocks,
                    geo.ulBlockSize/1024, geo.nPageSize,
                    (!geo.fBBMLoaded && geo.nDevType == DEVTYPE_NAND) ? " (BBM not loaded)" : "");

                ulBlock = 0;
                ulCount = 0;

                while(ulCount < geo.ulTotalBlocks)
                {
                    FFXFMLHANDLE    hFML;
                    D_UINT32        ulBlockCount;

                    ffxStat = FfxFmlDeviceRangeEnumerate(geo.nDevNum, ulBlock, &hFML, &ulBlockCount);
                    if(ffxStat != FFXSTAT_SUCCESS)
                        break;

                    if(hFML)
                    {
                        FFXFMLINFO  fi;

                        ffxStat = FfxFmlDiskInfo(hFML, &fi);
                        if(ffxStat != FFXSTAT_SUCCESS)
                        {
                            FFXPRINTF(1, ("Error getting FML info, Status=%lX\n", ffxStat));
                            break;
                        }

                        if(ulBlockCount == 1)
                            DclPrintf("    Block %lU is mapped to DISK%u\n", ulBlock, fi.nDiskNum);
                        else
                            DclPrintf("    Blocks %lU to %lU are mapped to DISK%u\n", ulBlock, ulBlock+ulBlockCount-1, fi.nDiskNum);

                        nDisks++;
                    }
                    else
                    {
                        if(ulBlockCount == 1)
                            DclPrintf("    Block %lU is not mapped to any Disk\n", ulBlock);
                        else
                            DclPrintf("    Blocks %lU to %lU are not mapped to any Disk\n", ulBlock, ulBlock+ulBlockCount-1);

                        nUnmapped++;
                    }

                    ulBlock += ulBlockCount;
                    ulCount += ulBlockCount;
                }

                nCount++;
            }
        }
        while(hDev);

        DclPrintf("Enumerated %u Device(s) containing %u Disk(s) and %u unmapped range(s)\n",
            nCount, nDisks, nUnmapped);

        return FFXSTAT_SUCCESS;
    }

    if(DclStrICmp(argv[1], "destroy") == 0)
    {
        if(argc != 3)
        {
            DclPrintf("Syntax error:  Too many or too few arguments for \"Destroy\".\n");
            return FFXSTAT_BADSYNTAX;
        }

        if(DclStrNICmp(argv[2], "DEV", 3) == 0)
        {
            pStr = DclNtoUL(&argv[2][3], &ulDevNum);
            if(pStr && *pStr == 0)
            {
                ffxStat = DestroyDevice((unsigned)ulDevNum);
                if(ffxStat != FFXSTAT_SUCCESS)
                    DclPrintf("Error destroying device, Status=%lX\n", ffxStat);

                return ffxStat;
            }
        }

        DclPrintf("Syntax error in %s\n", argv[2]);
        return FFXSTAT_BADSYNTAX;
    }

    if(DclStrICmp(argv[1], "create") == 0)
    {
        if(argc < 4 || argc > 5)
        {
            DclPrintf("Syntax error:  Too many or too few arguments for \"Create\".\n");
            return FFXSTAT_BADSYNTAX;
        }

        if(DclStrNICmp(argv[2], "DEV", 3) == 0)
        {
            pStr = DclNtoUL(&argv[2][3], &ulDevNum);
            if(pStr && *pStr == 0)
            {
                FFXDEVCONFIG        conf;
                D_UINT32            ulMaxKB;
                D_UINT32            ulResvLoKB = 0;
                D_UINT32            ulResvHiKB = 0;
              #if FFXCONF_NANDSUPPORT
                unsigned            fUseBBM = FFXCONF_BBMSUPPORT;
              #else
                unsigned            fUseBBM = FALSE;
              #endif
                D_UINT32            ulTemp;

                if(ulDevNum >= FFX_MAX_DEVICES)
                {
                    FFXPRINTF(1, ("DEV%lU out of range (max=%u)\n", ulDevNum, FFX_MAX_DEVICES-1));
                    goto BadDevSyntax;
                }

                pStr = DclNtoUL(argv[3], &ulMaxKB);
                if(!pStr)
                    goto BadDevSyntax;

                if(DclStrNICmp(pStr, "KB", 2) == 0)
                    ulTemp = ulMaxKB;
                else if(DclStrNICmp(pStr, "MB", 2) == 0)
                    ulTemp = ulMaxKB * 1024;
                else
                    goto BadDevSyntax;

                /*  Make sure the above calculations did not
                    cause the value to wrap.
                */
                if(ulTemp < ulMaxKB)
                    ulMaxKB = FFX_REMAINING;
                else
                    ulMaxKB = ulTemp;

                pStr += 2;

                if(*pStr == ':')
                {

                    pStr++;

                    pStr = DclNtoUL(pStr, &ulResvLoKB);
                    if(!pStr)
                        goto BadDevSyntax;

                    if(DclStrNICmp(pStr, "KB", 2) == 0)
                        ulTemp = ulResvLoKB;
                    else if(DclStrNICmp(pStr, "MB", 2) == 0)
                        ulTemp = ulResvLoKB * 1024;
                    else
                        goto BadDevSyntax;

                    /*  Make sure the above calculations did not
                        cause the value to wrap.
                    */
                    if(ulTemp < ulResvLoKB)
                        ulResvLoKB = FFX_REMAINING;
                    else
                        ulResvLoKB = ulTemp;

                    pStr += 2;

                    if(*pStr == ':')
                    {
                        pStr++;

                        pStr = DclNtoUL(pStr, &ulResvHiKB);
                        if(!pStr)
                            goto BadDevSyntax;

                        if(DclStrNICmp(pStr, "KB", 2) == 0)
                            ulTemp = ulResvHiKB;
                        else if(DclStrNICmp(pStr, "MB", 2) == 0)
                            ulTemp = ulResvHiKB * 1024;
                        else
                            goto BadDevSyntax;

                        /*  Make sure the above calculations did not
                            cause the value to wrap.
                        */
                        if(ulTemp < ulResvHiKB)
                            ulResvHiKB = FFX_REMAINING;
                        else
                            ulResvHiKB = ulTemp;

                        pStr += 2;

                        if(*pStr)
                            goto BadDevSyntax;
                    }
                }

                if((argc == 5) && (DclStrICmp(argv[4], "/NoBBM") == 0))
                    fUseBBM = FALSE;

                DclMemSet(&conf, 0, sizeof(conf));

                conf.nDevNum = (unsigned)ulDevNum;

                /*  Query the standard device settings from the ffxconf.h.
                    We must do this because the pBaseFlashAddress field
                    must be initialized, and we don't have a mechanism for
                    initing it in this interface.
                */
                ffxStat = FfxDriverDeviceSettings(&conf);
                if(ffxStat != FFXSTAT_SUCCESS)
                {
                    DclPrintf("Error querying Device settings, Status=%lX\n", ffxStat);
                    return ffxStat;
                }

                /*  Overlay some of the settings we obtained from the
                    ffxconf.h file with those supplied in this command.
                */
                conf.DevSettings.ulReservedBottomKB = ulResvLoKB;
                conf.DevSettings.ulReservedTopKB    = ulResvHiKB;
                conf.DevSettings.ulMaxArraySizeKB   = ulMaxKB;

              #if FFXCONF_NANDSUPPORT && FFXCONF_BBMSUPPORT
                /*  Query the standard BBM settings from the ffxconf.h
                    file, and overlay the settings to either use or not
                    use BBM.
                */
                ffxStat = FfxDriverDeviceBbmSettings(&conf);
                if(ffxStat != FFXSTAT_SUCCESS)
                {
                    DclPrintf("Error querying Device BBM settings, Status=%lX\n", ffxStat);
                    return ffxStat;
                }

                conf.BbmSettings.fEnableBBM = fUseBBM;
                
              #else
              
                (void)fUseBBM;
              #endif

                hDev = FfxDriverDeviceCreateParam(NULL, &conf, NULL);
                if(hDev)
                {
                    DclPrintf("Successfully created DEV%lU\n", ulDevNum);
                    return FFXSTAT_SUCCESS;
                }

                ffxStat = FFXSTAT_DEVICE_CREATEFAILED;

                DclPrintf("DEV creation failed with Status=%lX\n", ffxStat);
                return ffxStat;
            }
        }

        DclPrintf("Syntax error in %s\n", argv[2]);
        return FFXSTAT_BADSYNTAX;

      BadDevSyntax:
        DclPrintf("Syntax error in %s\n", argv[3]);
        return FFXSTAT_BADSYNTAX;
    }

    DclPrintf("Unrecognized option \"%s\"\n", argv[1]);

    return FFXSTAT_BADSYNTAX;

  DisplayHelp:

    DclPrintf("This command enumerates, creates, and destroys DEVICEs.\n\n");
    DclPrintf("Enumerate Syntax:  %s\n", argv[0]);
    DclPrintf("   Create Syntax:  %s Create DEVn MaxLen[:ResvLo[:ResvHi]] [/NoBBM]\n", argv[0]);
    DclPrintf("  Destroy Syntax:  %s Destroy DEVn\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("     DEVn  The Device to create or destroy, where 'n' is a number from 0 to %u.\n", FFX_MAX_DEVICES-1);
    DclPrintf("   MaxLen  The maximum size of the Device to create.  This may be specified as\n");
    DclPrintf("           hex (0x) or decimal, and must contain a \"KB\" or \"MB\" suffix.\n");
    DclPrintf("           If this value is larger than the available flash, after having\n");
    DclPrintf("           subtracted any low and high reserved space, all the remaining flash\n");
    DclPrintf("           will be used.\n");
    DclPrintf("   ResvLo  The amount of space to reserve at the beginning of the flash array.\n");
    DclPrintf("           This may be specified in hex (0x) or decimal, and must contain a\n");
    DclPrintf("           \"KB\" or \"MB\" suffix.\n");
    DclPrintf("   ResvHi  The amount of space to reserve at the end of the flash array.  This\n");
    DclPrintf("           may be specified in hex (0x) or decimal, and must contain a \"KB\" or\n");
    DclPrintf("           \"MB\" suffix.\n");
    DclPrintf("   /NoBBM  This option instructs the \"Create\" command to NOT use BBM for the\n");
    DclPrintf("           Device.  This is meaningful only when NAND is being used.\n\n");
    DclPrintf("NOTE: The reserved low and high space, as well as the maximum Device length must\n");
    DclPrintf("      be evenly divisible by the erase block size.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Public: FfxShellCmdDisk()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
DCLSTATUS FfxShellCmdDisk(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FFXSTATUS       ffxStat;
    FFXDISKHANDLE   hDisk = NULL;
    D_UINT32        ulDiskNum;
    const char     *pStr;
    int             iArgNum = 1;

    (void)hShell;
    
    if(ISHELPREQUEST())
        goto DisplayHelp;

    /*                      ------------ ENUMERATE ------------
    */
    if(argc == 1)
    {
        unsigned    nCount = 0;

        /*  No arguments are supplied, simply enumerate the Disks
        */
        do
        {
            hDisk = FfxDriverDiskEnumerate(NULL, hDisk);
            if(hDisk)
            {
                FFXDISKGEOMETRY geo;
                FFXFMLHANDLE    hFML;
                FFXFMLINFO      fi;

                ffxStat = FfxDriverDiskGeometry(hDisk, &geo);
                if(ffxStat != FFXSTAT_SUCCESS)
                {
                    DclError();
                    break;
                }

                hFML = FfxFmlHandle(geo.nDiskNum);
                if(!hFML)
                {
                    DclError();
                    break;
                }

                ffxStat = FfxFmlDiskInfo(hFML, &fi);
                if(ffxStat != FFXSTAT_SUCCESS)
                {
                    DclError();
                    break;
                }

                DclPrintf("DISK%u: Starts on DEV%u at block offset %4lU for %4lU blocks%s%s\n",
                    geo.nDiskNum, geo.nDevNum, geo.ulBlockOffset, geo.ulBlockCount,
                    fi.nDeviceCount <= 1 ? "" : " (spans multiple Devices)",
                    geo.fAllocatorLoaded ? "" : " (no allocator loaded)");

                nCount++;
            }
        }
        while(hDisk);

        DclPrintf("Enumerated %u Disk(s)\n", nCount);

        return FFXSTAT_SUCCESS;
    }

    /*                      ------------ DESTROY ------------
    */
    if(DclStrICmp(argv[iArgNum], "destroy") == 0)
    {
        if(argc != 3)
        {
            DclPrintf("Syntax error:  Too many or too few arguments for \"Destroy\".\n");
            return FFXSTAT_BADSYNTAX;
        }

        if(DclStrNICmp(argv[2], "DISK", 4) == 0)
        {
            pStr = DclNtoUL(&argv[2][4], &ulDiskNum);
            if(pStr && *pStr == 0)
            {
                return DestroyDisk((unsigned)ulDiskNum);
            }
        }

        DclPrintf("Syntax error in %s\n", argv[2]);
        return FFXSTAT_BADSYNTAX;
    }

    /*                      ------------ CREATE ------------
    */
    if(DclStrICmp(argv[iArgNum], "create") == 0)
    {
        FFXDISKCONFIG   conf = {0};
        D_UINT32        ulDevNum;
        D_UINT32        ulOffsetKB = 0;
        D_UINT32        ulLengthKB = FFX_REMAINING;
        D_BOOL          fUseAllocator = TRUE;
        
        if(argc < 4 || argc > 6)
        {
            DclPrintf("Syntax error:  Too many or too few arguments for \"Create\".\n");
            return FFXSTAT_BADSYNTAX;
        }

        /*  Process positional arguments 2 and 3 first
        */

        iArgNum++;

        if(DclStrNICmp(argv[iArgNum], "DISK", 4) != 0)
            goto BadDiskSyntax;
        
        pStr = DclNtoUL(&argv[iArgNum][4], &ulDiskNum);
        if(!pStr || *pStr != 0)
            goto BadDiskSyntax;

        if(ulDiskNum >= FFX_MAX_DISKS)
        {
            DclPrintf("DISK%lU out of range (max=%u)\n", ulDiskNum, FFX_MAX_DISKS-1);
            return FFXSTAT_DISKNUMBERINVALID;
        }

        iArgNum++;

        if(DclStrNICmp(argv[iArgNum], "DEV", 3) != 0)
            goto BadDiskSyntax;

        pStr = DclNtoUL(&argv[iArgNum][3], &ulDevNum);
        if(!pStr || ((*pStr != 0) && (*pStr != ':')))
            goto BadDiskSyntax;

        if(*pStr == ':')
        {
            D_UINT32    ulTemp;
            
            pStr++;

            pStr = DclNtoUL(pStr, &ulOffsetKB);
            if(!pStr)
                goto BadDiskSyntax;

            if(DclStrNICmp(pStr, "KB", 2) == 0)
                ulTemp = ulOffsetKB;
            else if(DclStrNICmp(pStr, "MB", 2) == 0)
                ulTemp = ulOffsetKB * 1024;
            else
                goto BadDiskSyntax;

            /*  Make sure the above calculations did not
                cause the value to wrap.
            */
            if(ulTemp < ulOffsetKB)
                ulOffsetKB = FFX_REMAINING;
            else
                ulOffsetKB = ulTemp;

            pStr += 2;

            if(*pStr == ':')
            {
                pStr++;

                pStr = DclNtoUL(pStr, &ulLengthKB);
                if(!pStr)
                    goto BadDiskSyntax;

                if(DclStrNICmp(pStr, "KB", 2) == 0)
                    ulTemp = ulLengthKB;
                else if(DclStrNICmp(pStr, "MB", 2) == 0)
                    ulTemp = ulLengthKB * 1024;
                else
                    goto BadDiskSyntax;

                /*  Make sure the above calculations did not
                    cause the value to wrap.
                */
                if(ulTemp < ulLengthKB)
                    ulLengthKB = FFX_REMAINING;
                else
                    ulLengthKB = ulTemp;

                pStr += 2;

                if(*pStr)
                    goto BadDiskSyntax;
            }
        }

        iArgNum++;

        /*  Process any remaining non-positional arguments
        */

        for(; iArgNum < argc; iArgNum++)
        {
            if(DclStrICmp(argv[iArgNum], "/NoAlloc") == 0)
            {
                fUseAllocator = FALSE;
                continue;
            }

            if(DclStrICmp(argv[iArgNum], "/Span") == 0)
            {
                conf.DiskSettings.nFlags |= FFX_DISK_SPANDEVICES;
                continue;
            }

            goto BadOption;
        }

        conf.nDiskNum                = (unsigned)ulDiskNum;
        conf.DiskSettings.nDevNum    = (unsigned)ulDevNum;
        conf.DiskSettings.ulOffsetKB = ulOffsetKB;
        conf.DiskSettings.ulLengthKB = ulLengthKB;
        conf.fInitedDiskSettings     = TRUE;

        /*  Init this now in the event we follow a code path
            which does not set it.
        */
        ffxStat = FFXSTAT_DISK_CREATEFAILED;

        hDisk = FfxDriverDiskCreateParam(NULL, &conf, NULL);
        if(hDisk)
        {
          #if FFXCONF_ALLOCATORSUPPORT
            if(fUseAllocator)
            {
                ffxStat = FfxDriverAllocatorCreate(hDisk);
                if(ffxStat != FFXSTAT_SUCCESS)
                    FfxDriverDiskDestroy(hDisk);
            }
            else
            {
                ffxStat = FFXSTAT_SUCCESS;
            }

          #else

            if(fUseAllocator)
            {
                DclPrintf("Allocator functionality is not available... skipping\n");
            }

            ffxStat = FFXSTAT_SUCCESS;
          #endif

            if(ffxStat == FFXSTAT_SUCCESS)
            {
                DclPrintf("Successfully created DISK%lU\n", ulDiskNum);
                return ffxStat;
            }
        }

        DclPrintf("Disk creation failed with Status=%lX\n", ffxStat);
        return ffxStat;

      BadDiskSyntax:
        DclPrintf("Syntax error in %s\n", argv[iArgNum]);
        return FFXSTAT_BADSYNTAX;
    }

  BadOption:
    DclPrintf("Unrecognized option \"%s\"\n", argv[iArgNum]);
    return FFXSTAT_BADSYNTAX;

  DisplayHelp:

    DclPrintf("This command enumerates, creates, and destroys Disks.\n\n");
    DclPrintf("Enumerate Syntax:  %s\n", argv[0]);
    DclPrintf("   Create Syntax:  %s Create DISKn DEVn[:Offset[:Len]] [/Span] [/NoAlloc]\n", argv[0]);
    DclPrintf("  Destroy Syntax:  %s Destroy DISKn\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("    DISKn  The Disk to create or destroy, where 'n' is a number from 0 to %u.\n", FFX_MAX_DISKS-1);
    DclPrintf("     DEVn  The Device upon which to create the Disk, where 'n' is a number from\n");
    DclPrintf("           0 to %u.\n", FFX_MAX_DEVICES-1);
    DclPrintf("   Offset  The offset within DEVn where the Disk should be created.  This may\n");
    DclPrintf("           be specified as hex (0x) or decimal, and must contain a \"KB\" or \"MB\"\n");
    DclPrintf("           suffix.  If this value is omitted, then the Disk will start at\n");
    DclPrintf("           offset 0 in the Device, and use the entire Device.\n");
    DclPrintf("   Length  The Disk length.  This may be specified as hex (0x) or decimal, and\n");
    DclPrintf("           must contain a \"KB\" or \"MB\" suffix.  If this value is omitted, the\n");
    DclPrintf("           Disk will use all the remaining space in the Device.\n");
    DclPrintf(" /Span     Allow the Disk to span multiple Devices, if the length dictates, and\n");
    DclPrintf("           the Device characteristics are similar.\n");
    DclPrintf(" /NoAlloc  This option instructs the \"Create\" command to NOT initialize the\n");
    DclPrintf("           allocator once the Disk is created.\n\n");
    DclPrintf("NOTE: You cannot create a Disk which overlaps another Disk.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Private: DestroyDisk()

    This function destroys a Disk.

    Parameters:
        nDiskNum - The Disk number to destroy.

    Return Value:
        Returns a standard FFXSTATUS code.
-------------------------------------------------------------------*/
static FFXSTATUS DestroyDisk(
    unsigned        nDiskNum)
{
    FFXSTATUS       ffxStat;
    FFXDISKHANDLE   hDisk;
    FFXDISKGEOMETRY geo;

    hDisk = FfxDriverDiskHandle(NULL, nDiskNum);
    if(!hDisk)
    {
        DclPrintf("DISK%u is not valid or not initialized\n", nDiskNum);
        return FFXSTAT_DISKNUMBERINVALID;
    }

    ffxStat = FfxDriverDiskGeometry(hDisk, &geo);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    if(geo.fAllocatorLoaded)
    {
        ffxStat = FfxDriverAllocatorDestroy(hDisk, FFX_SHUTDOWNFLAGS_NORMAL);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;
    }
  #endif

    DclPrintf("Destroying DISK%u\n", nDiskNum);

    FfxDriverDiskDestroy(hDisk);

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Private: DestroyDevice()

    This function destroys a Device.

    Parameters:
        nDevNum - The Device number to destroy.

    Return Value:
        Returns a standard FFXSTATUS code.
-------------------------------------------------------------------*/
static FFXSTATUS DestroyDevice(
    unsigned        nDevNum)
{
    FFXSTATUS       ffxStat;
    FFXDISKHANDLE   hDisk = NULL;
    FFXDEVHANDLE    hDev;

    hDev = FfxDriverDeviceHandle(NULL, nDevNum);
    if(!hDev)
    {
        DclPrintf("DEV%u is not valid or not initialized\n", nDevNum);
        return FFXSTAT_DEVICENUMBERINVALID;
    }

    /*  Before destroying the Device we need to destroy any Disks on it.
    */
    while(TRUE)
    {
        FFXDISKGEOMETRY geo;
        
        hDisk = FfxDriverDiskEnumerate(NULL, hDisk);
        if(!hDisk)
            break;

        ffxStat = FfxDriverDiskGeometry(hDisk, &geo);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;

        if(geo.nDevNum == nDevNum)
        {
            ffxStat = DestroyDisk(geo.nDiskNum);
            if(ffxStat != FFXSTAT_SUCCESS)
            {
                DclError();
                return ffxStat;
            }

            /*  Since we just destroyed the currently enumerated Disk, we
                must reset hDisk to NULL and restart the enumeration.
            */    
            hDisk = NULL;
        }
    };

    DclPrintf("Destroying DEV%u\n", nDevNum);

    FfxDriverDeviceDestroy(hDev);

    return FFXSTAT_SUCCESS;
}




#endif  /* DCLCONF_COMMAND_SHELL */

