/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

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

    This module contains a Windows CE commands for manipulating disk stores
    and partitions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlcepart.c $
    Revision 1.12  2011/05/18 22:41:09Z  garyp
    Fixed to correctly use DCLDIMENSIONOF() whree necessary.
    Revision 1.11  2010/12/10 02:53:59Z  garyp
    Expanded the previous rev to condition everything pertaining to the
    WIPE option.
    Revision 1.10  2010/12/10 00:36:53Z  deanw
    Conditioned out FATUTIL_SECURE_WIPE on CE50 builds.
    Revision 1.9  2010/12/01 23:37:08Z  garyp
    Factored volume information display out to DclCeVolumeDisplayInfo().
    Revision 1.8  2010/11/08 15:52:50Z  garyp
    Added the /FULL and /WIPE format options.
    Revision 1.7  2010/04/17 22:14:07Z  garyp
    Minor tweaks to some messages -- no functional changes.
    Revision 1.6  2010/01/23 19:32:30Z  garyp
    Added more detailed information about the store/partition.
    Revision 1.5  2009/03/09 16:53:48Z  billr
    Resolve bug 2474: CE5.0 build fails link with unresolved
    external: CeGetVolumeInfo
    Revision 1.4  2009/02/08 01:26:05Z  garyp
    Merged from the v4.0 branch.  Added the ability to specify the logical 
    block size for the FORMAT option.  Updated to display the various flags
    bits in a more readable format.  Modified to display accurate class, type,
    and flags values.  Modified to use the predefined name sizes.
    Revision 1.3  2008/01/02 19:50:32Z  Garyp
    Minor update to a console message.
    Revision 1.2  2007/12/14 17:27:00Z  Garyp
    Minor syntax fixes.
    Revision 1.1  2007/12/14 02:12:20Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <storemgr.h>
#include <fatutil.h>
#include <bootpart.h>

#include <dcl.h>
#include <dltools.h>
#include <dlerrlev.h>
#include <dlcetools.h>
#include <dlpartid.h>

#define MAX_ARG_LEN             (128)


static DCLSTATUS DoCommandFileSystemFormat(char *pszCmdLine);
static D_BOOL    ParseStoreAndPartitionNames(const char *pszCmdLine, char *pszStoreBuff, char *pszPartBuff);
static HANDLE    LocalOpenStore(const char *pszStoreName);
static HANDLE    LocalOpenPartition(HANDLE hStore, const char *pszPartName);
static D_BOOL    OpenStoreAndPartition(const char *pszStoreName, const char *pszPartName, HANDLE *phStore, HANDLE *phPart);


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
DCLSTATUS DclCePartition(
    const char     *pszCmdName,
    char           *pszCmdLine)
{
    HANDLE          hStoreSrch;
    HANDLE          hStore;
    HANDLE          hPart;
    STOREINFO       si;
    D_UINT16        uArgNum;
    D_UINT16        uArgCount;
    char            achBuffer[MAX_ARG_LEN];
    DCLSTATUS       dclStat;

    uArgCount = DclArgCount(pszCmdLine);

    for(uArgNum=1; uArgNum<=uArgCount; uArgNum++)
    {
        char    szStoreBuff[STORENAMESIZE];
        char    szPartBuff[PARTITIONNAMESIZE];

        if(!DclArgRetrieve(pszCmdLine, uArgNum, sizeof(achBuffer), achBuffer))
        {
            DclProductionError();
            return DCLSTAT_FAILURE;
        }

        if((DclStrICmp(achBuffer, "?") == 0) || (DclStrICmp(achBuffer, "/?") == 0))
        {
            DclPrintf("This command manipulates disk stores and partitions.\n\n");
            DclPrintf("Syntax:  %s [Command] [DSKn:[PartName]] [options]\n\n", pszCmdName);
            DclPrintf("Where 'Command' is one of the following:\n");
            DclPrintf("         Create  Create a partition on the specified store, with the\n");
            DclPrintf("                 specified partition name.  The /Secs and /Type options\n");
            DclPrintf("                 may be used to supply additional characteristics.\n");
            DclPrintf("         Delete  Delete the specified partition.\n");
            DclPrintf("          Mount  Mount the specified partition.\n");
            DclPrintf("       Dismount  Dismount the specified partition, or all partitions on\n");
            DclPrintf("                 the store, if only a store name is specified.\n");
            DclPrintf("            Zap  Wipe out any previous format on the specified store\n");
            DclPrintf("                 or partition (zeros out the first sector).\n");
/*          DclPrintf(" Rename:newname  Rename the specified partition to \"newname\".\n");   */
            DclPrintf("    Format:type  Format a file system on the specified partition.  'type'\n");
            DclPrintf("                  must be one of the following values:\n");
            DclPrintf("                     FAT12, FAT16, or FAT32.\n");
          #if _WIN32_WCE >= 600
            DclPrintf("                 CE 6.0 or later also supports:\n");
            DclPrintf("                     (T)EXFAT12, (T)EXFAT16, and (T)EXFAT32.\n");
          #else
            /*  Only versions prior to CE 6.0 support TFAT12, 16, or 32.
                CE 6.0 only supports TFAT when used in combination with
                EXFAT, regardless what the docs say.
            */
            DclPrintf("                     TFAT12, TFAT16, and TFAT32 are also supported.\n");
          #endif
            DclPrintf("\nAnd 'Options' may be the following:\n");
            DclPrintf("       /LB:size  Specifies the 'Format' commands's Logical Block (cluster)\n");
            DclPrintf("                 size in bytes or KiloBytes (KB suffix).\n");
            DclPrintf("          /Full  Specifies that a full format is to be performed.\n");
          #if _WIN32_WCE >= 502
            DclPrintf("          /Wipe  Specifies that a secure wipe is to be done during format.\n");
          #endif
            DclPrintf("     /Type:name  Specifies the partition type to use with the 'Create'\n");
            DclPrintf("                 command.  May be FAT12, FAT16, FAT32, BIGFAT, FAT32LBA,\n");
            DclPrintf("                 BIGFATLBA, or RELIANCE.  If the type is not specified,\n");
            DclPrintf("                 then the a default type will be used.\n");
            DclPrintf("    /Secs:count  Specifies the number of sectors to use with the 'Create'\n");
            DclPrintf("                 command.  If this is not specified, then the largest\n");
            DclPrintf("                 allowable size will be used.\n\n");
            DclPrintf("If no command is specified, information will be displayed for the\n");
            DclPrintf("specified store and/or partition.  If no parameters are specified at all,\n");
            DclPrintf("then information will be displayed for all stores and partitions.\n\n");
            DclPrintf("Store names are always specified in the form \"DSKn:\", where 'n' is a\n");
            DclPrintf("single numerical digit from 1 to 9.  The normal prefix is \"DSK\", though\n");
            DclPrintf("it is possible to have other prefixes.\n\n");

            return DCLSTAT_HELPREQUEST;
        }
        else if(DclStrICmp(achBuffer, "CREATE") == 0)
        {
            TCHAR       awcPart[PARTITIONNAMESIZE];
            SECTORNUM   ullSectors = 0;
            BYTE        bPartID = 0;

            DclArgClear(pszCmdLine, 1);
            uArgCount--;

            if(!uArgCount)
            {
                DclPrintf("A store and partition name must be specified\n");
                return DCLSTAT_FAILURE;
            }

            if(!ParseStoreAndPartitionNames(pszCmdLine, szStoreBuff, szPartBuff))
                return DCLSTAT_FAILURE;

            DclArgClear(pszCmdLine, 1);
            uArgCount--;

            while(uArgCount)
            {
                char        achTempBuff[MAX_ARG_LEN];
                const char *pChr;

                if(!DclArgRetrieve(pszCmdLine, 1, sizeof(achTempBuff), achTempBuff))
                {
                    DclProductionError();
                    return DCLSTAT_FAILURE;
                }

                if(DclStrNICmp(achTempBuff, "/Secs:", 6) == 0)
                {
                    D_UINT32    ulSectors;

                    pChr = DclNtoUL(&achTempBuff[6], &ulSectors);
                    if(!pChr || *pChr != 0)
                    {
                        DclPrintf("Bad syntax: \"%s\"\n", achTempBuff);
                        return DCLSTAT_FAILURE;
                    }

                    ullSectors = ulSectors;

                    DclArgClear(pszCmdLine, 1);
                    uArgCount--;

                    continue;
                }

                if(DclStrNICmp(achTempBuff, "/Type:", 6) == 0)
                {
                    if(DclStrICmp(&achTempBuff[6], "FAT12") == 0)
                    {
                        bPartID = PART_DOS2_FAT;
                    }
                    else if(DclStrICmp(&achTempBuff[6], "FAT16") == 0)
                    {
                        bPartID = PART_DOS3_FAT;
                    }
                    else if(DclStrICmp(&achTempBuff[6], "FAT32") == 0)
                    {
                        bPartID = PART_DOS32;
                    }
                    else if(DclStrICmp(&achTempBuff[6], "BIGFAT") == 0)
                    {
                        bPartID = PART_DOS4_FAT;
                    }
                    else if(DclStrICmp(&achTempBuff[6], "FAT32LBA") == 0)
                    {
                        bPartID = PART_DOS32X13;
                    }
                    else if(DclStrICmp(&achTempBuff[6], "BIGFATLBA") == 0)
                    {
                        bPartID = PART_DOSX13;
                    }
                    else if(DclStrICmp(&achTempBuff[6], "RELIANCE") == 0)
                    {
                        bPartID = FSID_RELIANCE;
                    }
                    else
                    {
                        DclPrintf("Unrecognized partition type: \"%s\"\n", &achTempBuff[6]);
                        return DCLSTAT_FAILURE;
                    }

                    DclArgClear(pszCmdLine, 1);
                    uArgCount--;

                    continue;
                }

                DclPrintf("Unrecognized option: \"%s\"\n", achTempBuff);
                return DCLSTAT_FAILURE;
            }

            hStore = LocalOpenStore(szStoreBuff);
            if(hStore == INVALID_HANDLE_VALUE)
                return DCLSTAT_FAILURE;

            if(!ullSectors)
            {
                si.cbSize = sizeof(si);
                if(!GetStoreInfo(hStore, &si))
                {
                    DclPrintf("Error getting store info, error code %lD\n", GetLastError());

                    CloseHandle(hStore);

                    return DCLSTAT_FAILURE;
                }

                ullSectors = si.snBiggestPartCreatable;
            }

            if(!MultiByteToWideChar(CP_ACP, 0, szPartBuff, -1, awcPart, sizeof(awcPart)))
            {
                DclPrintf("Illegal partition name \"%s\"\n", szPartBuff);

                CloseHandle(hStore);

                return DCLSTAT_FAILURE;
            }

            dclStat = DCLSTAT_FAILURE;
            if(bPartID)
            {
                if(CreatePartitionEx(hStore, awcPart, bPartID, ullSectors))
                    dclStat = DCLSTAT_SUCCESS;
            }
            else
            {
                if(CreatePartition(hStore, awcPart, ullSectors))
                    dclStat = DCLSTAT_SUCCESS;
            }

            if(dclStat == DCLSTAT_SUCCESS)
            {
                    DclPrintf("Partition successfully created\n");
            }
            else
            {
                DclPrintf("Partition create failed with error code %lD\n", GetLastError());

                dclStat = DCLSTAT_FAILURE;
            }

            CloseHandle(hStore);

            return dclStat;
        }
        else if(DclStrICmp(achBuffer, "DELETE") == 0)
        {
            TCHAR   awcPart[PARTITIONNAMESIZE];

            DclArgClear(pszCmdLine, 1);
            uArgCount--;

            if(!uArgCount)
            {
                DclPrintf("A store and partition name must be specified\n");
                return DCLSTAT_FAILURE;
            }

            if(!ParseStoreAndPartitionNames(pszCmdLine, szStoreBuff, szPartBuff))
                return DCLSTAT_FAILURE;

            DclArgClear(pszCmdLine, 1);
            uArgCount--;

            if(uArgCount)
                goto UnsupportedOptions;

            hStore = LocalOpenStore(szStoreBuff);
            if(hStore == INVALID_HANDLE_VALUE)
                return DCLSTAT_FAILURE;

            if(!MultiByteToWideChar(CP_ACP, 0, szPartBuff, -1, awcPart, sizeof(awcPart)))
            {
                DclPrintf("Illegal partition name \"%s\"\n", szPartBuff);

                CloseHandle(hStore);

                return DCLSTAT_FAILURE;
            }

            if(DeletePartition(hStore, awcPart))
            {
                DclPrintf("Partition successfully deleted\n");

                dclStat = DCLSTAT_SUCCESS;
            }
            else
            {
                DclPrintf("Partition delete failed with error code %lD\n", GetLastError());

                dclStat = DCLSTAT_FAILURE;
            }

            CloseHandle(hStore);

            return dclStat;
        }
        else if(DclStrICmp(achBuffer, "MOUNT") == 0)
        {
            DclArgClear(pszCmdLine, 1);
            uArgCount--;

            if(!uArgCount)
            {
                DclPrintf("A store and partition name must be specified\n");
                return DCLSTAT_FAILURE;
            }

            if(!ParseStoreAndPartitionNames(pszCmdLine, szStoreBuff, szPartBuff))
                return DCLSTAT_FAILURE;

            DclArgClear(pszCmdLine, 1);
            uArgCount--;

            if(uArgCount)
                goto UnsupportedOptions;

            if(!OpenStoreAndPartition(szStoreBuff, szPartBuff, &hStore, &hPart))
                return DCLSTAT_FAILURE;

            if(MountPartition(hPart))
            {
                DclPrintf("Partition successfully mounted\n");

                dclStat = DCLSTAT_SUCCESS;
            }
            else
            {
                DclPrintf("Partition mount failed with error code %lD\n", GetLastError());

                dclStat = DCLSTAT_FAILURE;
            }

            CloseHandle(hPart);
            CloseHandle(hStore);

            return dclStat;
        }
        else if(DclStrICmp(achBuffer, "DISMOUNT") == 0)
        {
            unsigned    fIsPart = FALSE;

            /*  Note that this option dismounts either a store or a partition,
                based on whether just a store name is specified, or whether
                both a store and partition are specified.
            */

            DclArgClear(pszCmdLine, 1);
            uArgCount--;

            if(!uArgCount)
            {
                DclPrintf("Either a store or a store and partition name must be specified\n");
                return DCLSTAT_FAILURE;
            }

            if(!DclArgRetrieve(pszCmdLine, 1, sizeof(szStoreBuff), szStoreBuff))
            {
                DclProductionError();
                return DCLSTAT_FAILURE;
            }

            if(DclStrLen(szStoreBuff) > 5)
            {
                /*  If the name is longer than "DSK1:" then a partition name
                    must have been specified as well.
                */
                if(!ParseStoreAndPartitionNames(pszCmdLine, szStoreBuff, szPartBuff))
                    return DCLSTAT_FAILURE;

                fIsPart = TRUE;
            }

            DclArgClear(pszCmdLine, 1);
            uArgCount--;

            if(uArgCount)
                goto UnsupportedOptions;

            hStore = LocalOpenStore(szStoreBuff);
            if(hStore == INVALID_HANDLE_VALUE)
                return DCLSTAT_FAILURE;

            if(fIsPart)
            {
                hPart = LocalOpenPartition(hStore, szPartBuff);

                if(DismountPartition(hPart))
                {
                    DclPrintf("Partition successfully dismounted\n");

                    dclStat = DCLSTAT_SUCCESS;
                }
                else
                {
                    DclPrintf("Partition dismount failed with error code %lD\n", GetLastError());

                    dclStat = DCLSTAT_FAILURE;
                }

                CloseHandle(hPart);
            }
            else
            {
                if(DismountStore(hStore))
                {
                    DclPrintf("Store successfully dismounted\n");

                    dclStat = DCLSTAT_SUCCESS;
                }
                else
                {
                    DclPrintf("Store dismount failed with error code %lD\n", GetLastError());

                    dclStat = DCLSTAT_FAILURE;
                }
            }

            CloseHandle(hStore);

            return dclStat;
        }
        else if(DclStrNICmp(achBuffer, "FORMAT:", 7) == 0)
        {
            return DoCommandFileSystemFormat(pszCmdLine);
        }
        else if(DclStrICmp(achBuffer, "ZAP") == 0)
        {
            unsigned    fIsPart = FALSE;

            /*  Note that this option zaps either a store or a partition,
                based on whether just a store name is specified, or whether
                both a store and partition are specified.
            */

            DclArgClear(pszCmdLine, 1);
            uArgCount--;

            if(!uArgCount)
            {
                DclPrintf("Either a store or a store and partition name must be specified\n");
                return DCLSTAT_FAILURE;
            }

            if(!DclArgRetrieve(pszCmdLine, 1, sizeof(szStoreBuff), szStoreBuff))
            {
                DclProductionError();
                return DCLSTAT_FAILURE;
            }

            if(DclStrLen(szStoreBuff) > 5)
            {
                /*  If the name is longer than "DSK1:" then a partition name
                    must have been specified as well.
                */
                if(!ParseStoreAndPartitionNames(pszCmdLine, szStoreBuff, szPartBuff))
                    return DCLSTAT_FAILURE;

                fIsPart = TRUE;
            }

            DclArgClear(pszCmdLine, 1);
            uArgCount--;

            if(uArgCount)
                goto UnsupportedOptions;

            hStore = LocalOpenStore(szStoreBuff);
            if(hStore == INVALID_HANDLE_VALUE)
                return DCLSTAT_FAILURE;

            if(fIsPart)
            {
                hPart = LocalOpenPartition(hStore, szPartBuff);
                if(hPart == INVALID_HANDLE_VALUE)
                {
                    dclStat = DCLSTAT_FAILURE;
                }
                else
                {
                    if(!FormatPartitionEx(hPart, 0, FALSE))
                    {
                        DclPrintf("Partition zap failed with error code %lD\n", GetLastError());

                        dclStat = DCLSTAT_FAILURE;
                    }
                    else
                    {
                        DclPrintf("Partition successfully zapped\n");

                        dclStat = DCLSTAT_SUCCESS;
                    }

                    CloseHandle(hPart);
                }
            }
            else
            {
                if(!FormatStore(hStore))
                {
                    DclPrintf("Store zap failed with error code %lD\n", GetLastError());

                    dclStat = DCLSTAT_FAILURE;
                }
                else
                {
                    DclPrintf("Store successfully zapped\n");

                    dclStat = DCLSTAT_SUCCESS;
                }
            }

            CloseHandle(hStore);

            return dclStat;
        }
        else
        {
          UnsupportedOptions:
            DclPrintf("Unrecognized command \"%s\"\n", pszCmdLine);

            return DCLSTAT_FAILURE;
        }
    }

    /*  Iterate through all the stores...
    */
    si.cbSize = sizeof(si);
    hStoreSrch = FindFirstStore(&si);
    if(hStoreSrch == INVALID_HANDLE_VALUE)
    {
        DclPrintf("No stores were found\n");
    }
    else
    {
        while(hStoreSrch != INVALID_HANDLE_VALUE)
        {
            HANDLE  hStore;
            char    szTempBuff[128]; /* long enough for the worst case combination */

            DclPrintf("Store \"%W\"\n",                              si.szDeviceName);
            DclPrintf("  Name       %32W\n",                         si.szStoreName);
            DclPrintf("  Class                            %lX\n",    si.sdi.dwDeviceClass);
            DclPrintf("  Type                             %lX\n",    si.sdi.dwDeviceType);
            DclPrintf("  Flags                            %lX\n",    si.sdi.dwDeviceFlags);
            DclPrintf("  Attributes                       %lX\n",    si.dwAttributes);

            if(si.dwAttributes)
            {
                static const char  *apszName[] = 
                {
                    "READONLY",     /* STORE_ATTRIBUTE_READONLY     */
                    "REMOVABLE",    /* STORE_ATTRIBUTE_REMOVABLE    */
                    "UNFORMATTED",  /* STORE_ATTRIBUTE_UNFORMATTED  */
                    "AUTOFORMAT",   /* STORE_ATTRIBUTE_AUTOFORMAT   */
                    "AUTOPART",     /* STORE_ATTRIBUTE_AUTOPART     */
                    "AUTOMOUNT"     /* STORE_ATTRIBUTE_AUTOMOUNT    */
                };      
                
                DclStrCpy(szTempBuff, "(");

                DclFormatBitFlags(szTempBuff, DCLDIMENSIONOF(szTempBuff), 
                                  apszName, DCLDIMENSIONOF(apszName), si.dwAttributes);
                                                   
                szTempBuff[DclStrLen(szTempBuff) - 1] = ')';

                DclPrintf("  %43s\n", szTempBuff);
            }
 
            DclPrintf("  Bytes per Sector                 %10lU\n",  si.dwBytesPerSector);
            DclPrintf("  Partitions: Total/Mounted          %3lU /%3lU\n", si.dwPartitionCount, si.dwMountCount);
            DclPrintf("  Total Sectors               %15llU  %8s\n", si.snNumSectors,  DclScaleItems(si.snNumSectors,  si.dwBytesPerSector, szTempBuff, sizeof(szTempBuff)));
            DclPrintf("  Free Sectors                %15llU  %8s\n", si.snFreeSectors, DclScaleItems(si.snFreeSectors, si.dwBytesPerSector, szTempBuff, sizeof(szTempBuff)));
            DclPrintf("  Biggest Creatable Partition %15llU  %8s\n", si.snBiggestPartCreatable, DclScaleItems(si.snBiggestPartCreatable, si.dwBytesPerSector, szTempBuff, sizeof(szTempBuff)));

            hStore = OpenStore(si.szDeviceName);
            if(hStore == INVALID_HANDLE_VALUE)
            {
                DCLPRINTF(1, ("Error opening store!\n"));
            }
            else
            {
                HANDLE      hPart;
                PARTINFO    pi;

                /*  Iterate through all the partitions on this store...
                */
                pi.cbSize = sizeof(pi);
                hPart = FindFirstPartition(hStore, &pi);

                while(hPart != INVALID_HANDLE_VALUE)
                {
                    DclPrintf("  Partition \"%W\"\n",                        pi.szPartitionName);
                    DclPrintf("    File System  %28W\n",                     pi.szFileSys);
                    DclPrintf("    Volume Name  %28W\n",                     pi.szVolumeName);
                    DclPrintf("    Total Sectors             %15llU  %8s\n", pi.snNumSectors, DclScaleItems(pi.snNumSectors, si.dwBytesPerSector, szTempBuff, sizeof(szTempBuff)));
                    DclPrintf("    Partition ID Type              %10u\n",   pi.bPartType);
                    DclPrintf("    Attributes                     %lX\n",    pi.dwAttributes);

                    if(pi.dwAttributes)
                    {
                        static const char  *apszName[] = 
                        {
                            "EXPENDABLE",   /* PARTITION_ATTRIBUTE_EXPENDABLE */
                            "READONLY",     /* PARTITION_ATTRIBUTE_READONLY   */
                            "AUTOFORMAT",   /* PARTITION_ATTRIBUTE_AUTOFORMAT */
                            "BOOT",         /* PARTITION_ATTRIBUTE_BOOT       */  
                            "MOUNTED"       /* PARTITION_ATTRIBUTE_MOUNTED    */
                        };      
                        
                        DclStrCpy(szTempBuff, "(");

                        DclFormatBitFlags(szTempBuff, DCLDIMENSIONOF(szTempBuff), 
                                          apszName, DCLDIMENSIONOF(apszName), pi.dwAttributes);
                                                           
                        szTempBuff[DclStrLen(szTempBuff) - 1] = ')';

                        DclPrintf("    %41s\n", szTempBuff);
                    }

                  #if _WIN32_WCE >= 500 /* CeGetVolumeInfo() was introduced in 5.0 */

                    /*  A NULL volume name is possible (and allowed in the call
                        to CeGetVolumeInfo()), however there is no good way to
                        distinguish that from the XIP partition, which also has
                        a NULL volume name.  Therefore check that the partition
                        is mounted and that it has a low partition ID number.
                    */
                    if(pi.szVolumeName[0] ||
                        ((pi.dwAttributes & PARTITION_ATTRIBUTE_MOUNTED) && (pi.bPartType < PART_CE_HIDDEN)))
                    {
                        DclCeVolumeDisplayInfo(pi.szVolumeName);
                    }
                  #endif /* WinCE 5.00 or later */

                    if(!FindNextPartition(hPart, &pi))
                    {
                        FindClosePartition(hPart);
                        break;
                    }
                }

                CloseHandle(hStore);
            }

            if(!FindNextStore(hStoreSrch, &si))
            {
                FindCloseStore(hStoreSrch);
                break;
            }
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS DoCommandFileSystemFormat(
    char               *pszCmdLine)
{
    DWORD               dwResult;
    PFN_FORMATVOLUMEEX  pfnFormatVolumeEx;
    HINSTANCE           hLib;
    FORMAT_PARAMS       fp = {0};
    D_UINT16            uArgCount;
    char                achBuffer[MAX_ARG_LEN];
    DCLSTATUS           dclStat;
    char                szStoreBuff[STORENAMESIZE];
    char                szPartBuff[PARTITIONNAMESIZE];
    HANDLE              hStore;
    HANDLE              hPart;

    DclAssert(pszCmdLine);

    uArgCount = DclArgCount(pszCmdLine);

    if(!DclArgRetrieve(pszCmdLine, 1, sizeof(achBuffer), achBuffer))
    {
        DclProductionError();
        return DCLSTAT_FAILURE;
    }

    fp.cbSize = sizeof(fp);
    fp.fo.dwNumFats = 2;
    fp.fo.dwRootEntries = 512;  /* <must> be specified -- CE will not choose a default value */

    if(DclStrICmp(&achBuffer[7], "FAT12") == 0)
    {
        fp.fo.dwFatVersion = 12;
    }
    else if(DclStrICmp(&achBuffer[7], "FAT16") == 0)
    {
        fp.fo.dwFatVersion = 16;
    }
    else if(DclStrICmp(&achBuffer[7], "FAT32") == 0)
    {
        fp.fo.dwFatVersion = 32;
    }
  #if _WIN32_WCE >= 600
    else if(DclStrICmp(&achBuffer[7], "EXFAT12") == 0)
    {
        fp.fo.dwFlags |= FATUTIL_FORMAT_EXFAT;
        fp.fo.dwFatVersion = 12;
    }
    else if(DclStrICmp(&achBuffer[7], "EXFAT16") == 0)
    {
        fp.fo.dwFlags |= FATUTIL_FORMAT_EXFAT;
        fp.fo.dwFatVersion = 16;
    }
    else if(DclStrICmp(&achBuffer[7], "EXFAT32") == 0)
    {
        fp.fo.dwFlags |= FATUTIL_FORMAT_EXFAT;
        fp.fo.dwFatVersion = 32;
    }
    else if(DclStrICmp(&achBuffer[7], "TEXFAT12") == 0)
    {
        fp.fo.dwFlags |= FATUTIL_FORMAT_TFAT;
        fp.fo.dwFlags |= FATUTIL_FORMAT_EXFAT;
        fp.fo.dwFatVersion = 12;
    }
    else if(DclStrICmp(&achBuffer[7], "TEXFAT16") == 0)
    {
        fp.fo.dwFlags |= FATUTIL_FORMAT_TFAT;
        fp.fo.dwFlags |= FATUTIL_FORMAT_EXFAT;
        fp.fo.dwFatVersion = 16;
    }
    else if(DclStrICmp(&achBuffer[7], "TEXFAT32") == 0)
    {
        fp.fo.dwFlags |= FATUTIL_FORMAT_TFAT;
        fp.fo.dwFlags |= FATUTIL_FORMAT_EXFAT;
        fp.fo.dwFatVersion = 32;
    }
  #else
    /*  Only versions prior to CE 6.0 support TFAT12, 16, or 32.
        CE 6.0 only supports TFAT when used in combination with
        EXFAT, regardless what the docs say.
    */
    else if(DclStrICmp(&achBuffer[7], "TFAT12") == 0)
    {
        fp.fo.dwFlags |= FATUTIL_FORMAT_TFAT;
        fp.fo.dwFatVersion = 12;
    }
    else if(DclStrICmp(&achBuffer[7], "TFAT16") == 0)
    {
        fp.fo.dwFlags |= FATUTIL_FORMAT_TFAT;
        fp.fo.dwFatVersion = 16;
    }
    else if(DclStrICmp(&achBuffer[7], "TFAT32") == 0)
    {
        fp.fo.dwFlags |= FATUTIL_FORMAT_TFAT;
        fp.fo.dwFatVersion = 32;
    }
  #endif
    else
    {
        DclPrintf("Unrecognized file system type \"%s\"\n", &achBuffer[7]);
        return DCLSTAT_FAILURE;
    }

    DclArgClear(pszCmdLine, 1);
    uArgCount--;

    if(!uArgCount)
    {
        DclPrintf("A store and partition name must be specified\n");
        return DCLSTAT_FAILURE;
    }

    if(!ParseStoreAndPartitionNames(pszCmdLine, szStoreBuff, szPartBuff))
        return DCLSTAT_FAILURE;

    DclArgClear(pszCmdLine, 1);
    uArgCount--;

    while(uArgCount--)
    {
        if(!DclArgRetrieve(pszCmdLine, 1, sizeof(achBuffer), achBuffer))
        {
            DclProductionError();
            return DCLSTAT_FAILURE;
        }

        if(DclStrNICmp(achBuffer, "/LB:", 4) == 0)
    {
        D_UINT32    ulLogicalBlock;
        const char *pChr;

        pChr = DclNtoUL(&achBuffer[4], &ulLogicalBlock);
        if(pChr)
        {
            if(DclStrICmp(pChr, "KB") == 0)
            {
                ulLogicalBlock *= 1024;
            }
            else if(*pChr != 0)
            {
                DclPrintf("Bad syntax: \"%s\"\n", achBuffer);
                return DCLSTAT_FAILURE;
            }
        }
        else
        {
            DclPrintf("Bad syntax: \"%s\"\n", achBuffer);
            return DCLSTAT_FAILURE;
        }

        fp.fo.dwClusSize = ulLogicalBlock;
        }
        else if(DclStrICmp(achBuffer, "/Full") == 0)
        {
            fp.fo.dwFlags |= FATUTIL_FULL_FORMAT;
        }
      #if _WIN32_WCE >= 502
        else if(DclStrICmp(achBuffer, "/Wipe") == 0)
        {
            fp.fo.dwFlags |= FATUTIL_SECURE_WIPE;
        }
      #endif
        else
        {
            DclPrintf("Unrecognized option \"%s\"\n", achBuffer);
        return DCLSTAT_FAILURE;
        }
        
        DclArgClear(pszCmdLine, 1);
    }

    if(!OpenStoreAndPartition(szStoreBuff, szPartBuff, &hStore, &hPart))
        return DCLSTAT_FAILURE;

    hLib = LoadLibrary(L"fatutil.dll");
    if(hLib == NULL)
    {
        DclPrintf("LoadLibrary failed\n");

        dclStat = DCLSTAT_FAILURE;
    }
    else
    {
        pfnFormatVolumeEx = (PFN_FORMATVOLUMEEX)GetProcAddress(hLib,L"FormatVolumeEx");
        if(pfnFormatVolumeEx == NULL)
        {
            DclPrintf("GetProcAddress failed\n");

            dclStat = DCLSTAT_FAILURE;
        }
        else
        {
            dwResult = pfnFormatVolumeEx(hPart, &fp);
            if(dwResult != ERROR_SUCCESS)
            {
                DclPrintf("FormatVolumeEx() failed with error code %lU.  If the partition is mounted,\n", dwResult);
                DclPrintf("it must first be dismounted before formatting, then remounted afterword.\n");

                dclStat = DCLSTAT_FAILURE;
            }
            else
            {
                DclPrintf("Format Results: FAT%lU NumFATs=%lU SecPerFAT=%lU SecPerCluster=%lU\n",
                    fp.fr.dwFatVersion, fp.fr.dwNumFats, fp.fr.dwSectorsPerFat, fp.fr.dwSectorsPerCluster);
                DclPrintf("                ReservedSecs=%lU RootDirSecs=%lU TotalSecs=%lU\n",
                    fp.fr.dwReservedSectors, fp.fr.dwRootSectors, fp.fr.dwTotalSectors);

                dclStat = DCLSTAT_SUCCESS;
            }
        }
    }

/*            MountPartition(hPart);
*/
    CloseHandle(hPart);
    CloseHandle(hStore);

    if(hLib)
        FreeLibrary(hLib);

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static D_BOOL ParseStoreAndPartitionNames(
    const char *pszCmdLine,
    char       *pszStoreBuff,
    char       *pszPartBuff)
{
    char        achBuffer[MAX_ARG_LEN];

    DclAssert(pszCmdLine);
    DclAssert(pszStoreBuff);
    DclAssert(pszPartBuff);

    if(!DclArgRetrieve(pszCmdLine, 1, sizeof(achBuffer), achBuffer))
        return FALSE;

    if(DclStrLen(achBuffer) < 6 || achBuffer[3] < '1' || achBuffer[3] > '9' || achBuffer[4] != ':')
    {
        DclPrintf("The store and partition name must be specified in the form \"DSK1:PartName\",\n");
        DclPrintf("where the store name is a 5 digit sequence ending with a number from 1 to 9,\n");
        DclPrintf("followed by a colon.  The partition name then follows the 5 digit store name.\n");

        return FALSE;
    }

    DclStrNCpy(pszStoreBuff, achBuffer, STORENAMESIZE);
    pszStoreBuff[5] = 0;

    if(DclStrLen(&achBuffer[5]) > PARTITIONNAMESIZE)
    {
        DclPrintf("The partition name is too long.\n");
        return FALSE;
    }

    DclStrNCpy(pszPartBuff, &achBuffer[5], PARTITIONNAMESIZE);

    return TRUE;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static HANDLE LocalOpenStore(
    const char *pszStoreName)
{
    TCHAR       awcStore[STORENAMESIZE];
    HANDLE      hStore;

    DclAssert(pszStoreName);

    if(!MultiByteToWideChar(CP_ACP, 0, pszStoreName, -1, awcStore, sizeof(awcStore)))
    {
        DclPrintf("Illegal store name \"%s\"\n", pszStoreName);
        return INVALID_HANDLE_VALUE;
    }

    hStore = OpenStore(awcStore);
    if(hStore == INVALID_HANDLE_VALUE)
    {
        DclPrintf("Unable to open store \"%W\", error code %lD\n", awcStore, GetLastError());
    }

    return hStore;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static HANDLE LocalOpenPartition(
    HANDLE      hStore,
    const char *pszPartName)
{
    TCHAR       awcPart[PARTITIONNAMESIZE];
    HANDLE      hPart;

    DclAssert(hStore != INVALID_HANDLE_VALUE);
    DclAssert(pszPartName);

    if(!MultiByteToWideChar(CP_ACP, 0, pszPartName, -1, awcPart, sizeof(awcPart)))
    {
        DclPrintf("Illegal partition name \"%s\"\n", pszPartName);
        return INVALID_HANDLE_VALUE;
    }

    hPart = OpenPartition(hStore, awcPart);
    if(hPart == INVALID_HANDLE_VALUE)
    {
        DclPrintf("Unable to open partition \"%W\", error code %lD\n", awcPart, GetLastError());
    }

    return hPart;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static D_BOOL OpenStoreAndPartition(
    const char     *pszStoreName,
    const char     *pszPartName,
    HANDLE         *phStore,
    HANDLE         *phPart)
{
    DclAssert(pszStoreName);
    DclAssert(pszPartName);
    DclAssert(phStore);
    DclAssert(phPart);

    *phStore = LocalOpenStore(pszStoreName);
    if(*phStore == INVALID_HANDLE_VALUE)
        return FALSE;

    *phPart = LocalOpenPartition(*phStore, pszPartName);
    if(*phPart == INVALID_HANDLE_VALUE)
    {
        CloseHandle(*phStore);

        return FALSE;
    }

    return TRUE;
}




