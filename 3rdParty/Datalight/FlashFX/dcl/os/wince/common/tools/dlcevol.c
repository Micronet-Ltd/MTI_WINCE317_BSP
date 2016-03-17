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

    This module contains a Windows CE commands for manipulating disk volumes.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlcevol.c $
    Revision 1.4  2011/05/18 22:41:10Z  garyp
    Fixed to correctly use DCLDIMENSIONOF() whree necessary.
    Revision 1.3  2010/12/10 02:53:57Z  garyp
    Condition the whole module on CE 5.0 or later.
    Revision 1.2  2010/12/10 00:37:46Z  deanw
    Conditioned out IOCTL_DISK_SET_SECURE_WIPE_FLAG for CE 50 builds.
    Revision 1.1  2010/11/10 02:21:16Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <storemgr.h>
#include <fatutil.h>
#include <bootpart.h>

#if _WIN32_WCE >= 500   /* CeGetVolumeInfo() was introduced in 5.0 */

#include <dcl.h>
#include <dltools.h>
#include <dlerrlev.h>
#include <dlcetools.h>

#define MAX_ARG_LEN             (128)


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
DCLSTATUS DclCeVolume(
    const char     *pszCmdName,
    char           *pszCmdLine)
{
    D_UINT16        uArgCount;
    char            achBuffer[MAX_ARG_LEN];

    uArgCount = DclArgCount(pszCmdLine);

    if(!uArgCount)
        goto VolHelp;
        
    if(!DclArgRetrieve(pszCmdLine, 1, sizeof(achBuffer), achBuffer))
    {
        DclError();
        return DCLSTAT_FAILURE;
    }

    if((DclStrICmp(achBuffer, "?") == 0) || (DclStrICmp(achBuffer, "/?") == 0))
        goto VolHelp;

  #if _WIN32_WCE >= 502
    if(DclStrICmp(achBuffer, "SETWIPEFLAG") == 0)
    {
        D_WCHAR     wzVol[MAX_ARG_LEN];
        HANDLE      hVol;

        DclArgClear(pszCmdLine, 1);
        uArgCount--;

        if(!uArgCount)
        {
            DclPrintf("A volume name must be specified\n");
            return DCLSTAT_FAILURE;
        }

        if(!DclArgRetrieve(pszCmdLine, 1, sizeof(achBuffer), achBuffer))
        {
            DclError();
            return DCLSTAT_FAILURE;
        }

        if(!MultiByteToWideChar(CP_ACP, 0, achBuffer, -1, wzVol, sizeof(wzVol)))
        {
            DclPrintf("Illegal volume name \"%s\"\n", achBuffer);
 
            return DCLSTAT_FAILURE;
        }

        if(wzVol[wcslen(wzVol) - 1] == '\\')
            wcscat(wzVol, TEXT("VOL:"));
        else
            wcscat(wzVol, TEXT("\\VOL:"));


        hVol = CreateFile(wzVol, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if(hVol == INVALID_HANDLE_VALUE)
        {
            DCLPRINTF(1, ("DclCeVolume() failed for \"%W\", error=%lU\n", wzVol, GetLastError()));
            return DCLSTAT_FAILURE;
        }
        if(!DeviceIoControl(hVol, IOCTL_DISK_SET_SECURE_WIPE_FLAG, NULL, 0, NULL, 0, NULL, NULL))
        {
            DCLPRINTF(1, ("DeviceIoControl(IOCTL_DISK_SET_SECURE_WIPE_FLAG) failed with error=%lU\n",
                GetLastError()));

            CloseHandle(hVol);
            
            return DCLSTAT_FAILURE;
        }

        CloseHandle(hVol);
            
        return DCLSTAT_SUCCESS;
    }
  #endif
  
    {
        D_WCHAR     wzVol[MAX_ARG_LEN];
        DWORD       dwError;

        DclArgClear(pszCmdLine, 1);
        uArgCount--;

        if(uArgCount)
        {
            DclPrintf("Unrecognized argument!\n");
            return DCLSTAT_FAILURE;
        }

        if(!MultiByteToWideChar(CP_ACP, 0, achBuffer, -1, wzVol, sizeof(wzVol)))
        {
            DclPrintf("Illegal volume name \"%s\"\n", achBuffer);
 
            return DCLSTAT_FAILURE;
        }

        dwError = DclCeVolumeDisplayInfo(wzVol);
        if(dwError != ERROR_SUCCESS)
        {
            DCLPRINTF(1, ("Error %lX retrieving volume information for \"%W\"\n", dwError, wzVol));

            return DCLSTAT_FAILURE;
        }

        return DCLSTAT_SUCCESS;
    }

  VolHelp:

    DclPrintf("This command manipulates disk volumes.\n\n");
  #if _WIN32_WCE >= 502
    DclPrintf("Syntax:  %s [Command] [VolumeName]\n\n", pszCmdName);
    DclPrintf("Where 'Command' is one of the following:\n");
    DclPrintf("    SetWipeFlag  Set the secure wipe flag for the volume.\n");
    DclPrintf("If no command is specified, information will be displayed for the\n");
    DclPrintf("specified volume.\n\n");
  #else
    DclPrintf("Syntax:  %s VolumeName\n\n", pszCmdName);
  #endif    

    return DCLSTAT_HELPREQUEST;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
DWORD DclCeVolumeDisplayInfo(
    const D_WCHAR  *pwzVolumeName)
{
    char            szTempBuff[128]; /* long enough for the worst case combination */
    CE_VOLUME_INFO  cvi;

    cvi.cbSize = sizeof(cvi);

    if(!CeGetVolumeInfo(pwzVolumeName, CeVolumeInfoLevelStandard, &cvi))
    {
        DWORD   dwError = GetLastError();
        
        DCLPRINTF(1, ("Error %lX retrieving volume information for \"%W\"\n", dwError, pwzVolumeName));

        return dwError;
    }
    
    DclPrintf("    Volume Information:\n");
/*  DclPrintf("      Store:Partition %W%W\n",               cvi.szStoreName, cvi.szPartitionName); */
    DclPrintf("      Logical Block/Cluster Size   %10lU\n", cvi.dwBlockSize);
    DclPrintf("      Attributes                   %lX\n",   cvi.dwAttributes);

    if(cvi.dwAttributes)
    {
        static const char  *apszName[] = 
        {
            "READONLY",     /* CE_VOLUME_ATTRIBUTE_READONLY  */
            "HIDDEN",       /* CE_VOLUME_ATTRIBUTE_HIDDEN    */
            "REMOVABLE",    /* CE_VOLUME_ATTRIBUTE_REMOVABLE */
            "SYSTEM",       /* CE_VOLUME_ATTRIBUTE_SYSTEM    */
            "BOOT"          /* CE_VOLUME_ATTRIBUTE_BOOT      */
        };      
        
        DclStrCpy(szTempBuff, "(");

        DclFormatBitFlags(szTempBuff, DCLDIMENSIONOF(szTempBuff), apszName, 
                          DCLDIMENSIONOF(apszName), cvi.dwAttributes);
                                           
        szTempBuff[DclStrLen(szTempBuff) - 1] = ')';

        DclPrintf("      %39s\n", szTempBuff);
    }

    DclPrintf("      Flags                        %lX\n", cvi.dwFlags);

    if(cvi.dwFlags)
    {
        static const char  *apszName[] = 
        {
            "TRANSMETA",    /* CE_VOLUME_TRANSACTION_SAFE        */
            "TRANSFILE",    /* CE_VOLUME_FLAG_TRANSACT_WRITE     */
            "SGIO",         /* CE_VOLUME_FLAG_WFSC_SUPPORTED -- aka scatter/gather */
            "LOCK",         /* CE_VOLUME_FLAG_LOCKFILE_SUPPORTED */
            "NET",          /* CE_VOLUME_FLAG_NETWORK            */
            "STORE",        /* CE_VOLUME_FLAG_STORE              */
          #if _WIN32_WCE >= 600
            "RAMFS",        /* CE_VOLUME_FLAG_RAMFS                   */
            "SECURITY",     /* CE_VOLUME_FLAG_FILE_SECURITY_SUPPORTED */
            "64BIT",        /* CE_VOLUME_FLAG_64BIT_FILES_SUPPORTED   */
          #endif
        };      
        
        DclStrCpy(szTempBuff, "(");

        DclFormatBitFlags(szTempBuff, DCLDIMENSIONOF(szTempBuff), apszName, 
                          DCLDIMENSIONOF(apszName), cvi.dwFlags);
                                           
        szTempBuff[DclStrLen(szTempBuff) - 1] = ')';

        DclPrintf("      %39s\n", szTempBuff);
    }

    return ERROR_SUCCESS;
}

#endif



