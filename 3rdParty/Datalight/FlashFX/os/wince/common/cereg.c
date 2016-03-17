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

    This module contains common CE code for reading FlashFX settings from
    the registry.  It is typically called from FfxHookDiskCreate().
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: cereg.c $
    Revision 1.13  2010/07/31 19:23:01Z  garyp
    Updated to no longer requires a "DefaultFileSystem" value of "NONE",
    but rather to treat any unknown value as "no file system", so that we
    don't conflict with CE's expected usage of this registry setting.
    Revision 1.12  2010/01/23 22:58:17Z  garyp
    Updated to read the DefaultFileSystem value from the profile section.
    Revision 1.11  2009/07/22 00:48:21Z  garyp
    Merged from the v4.0 branch.  Updated to grab the DiskFlags value from the
    registry.  Modified so the lack of a folder name is not a critical error.
    Cleaned up the messages to be DEBUG messages.
    Revision 1.10  2009/02/09 07:43:49Z  garyp
    Some variables renamed.
    Revision 1.9  2008/05/23 02:15:51Z  garyp
    Merged from the WinMobile branch.
    Revision 1.8.1.2  2008/05/23 02:15:51Z  garyp
    Updated to grab the Index value out of the registry.
    Revision 1.8  2008/05/09 21:08:02Z  garyp
    Updated to allow "Profile" or "ProfileName".
    Revision 1.7  2008/03/26 03:04:00Z  Garyp
    Minor datatype updates to mollify WinMobile.
    Revision 1.6  2007/12/25 21:19:11Z  Garyp
    Removed some obsolete code and cleaned up some messages.
    Revision 1.5  2007/11/03 23:50:09Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2007/07/24 17:59:58Z  timothyj
    Updated to use the new FfxDriverDiskAdjust() helper.  Changed offset and
    length to be in KB.
    Revision 1.3  2006/02/22 02:59:10Z  Garyp
    Updated to default to device 0 and disk 0 if the registry settings are not
    found.
    Revision 1.2  2006/02/11 00:46:50Z  Garyp
    Updated to read the DeviceNum and DiskNum settings.
    Revision 1.1  2005/10/06 05:48:56Z  Pauli
    Initial revision
    Revision 1.4  2005/03/29 03:36:45Z  GaryP
    Minor code formatting cleanup.
    Revision 1.3  2004/12/30 23:00:12Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2004/08/24 05:50:15Z  GaryP
    Re-implemented using new initialization model.
---------------------------------------------------------------------------*/

#include <windows.h>
#include <diskio.h>
#include <devload.h>

#include <flashfx.h>
#include <ffxwce.h>
#include <oecommon.h>


static D_BOOL   ReadRegistryParameters(HKEY hDevKey, FFXDISKHOOK *pHook);


/*-------------------------------------------------------------------
    Protected: FfxCeGetRegistrySettings()

    Read all the parameters for this device from the registry.
    Fill in appropriate defaults for optional parameters.

    Parameters:
        lptzActiveKey - the registry key for the parameters
        pHook - the DEVEXTRA structure for the device

    Return Value:
        Returns TRUE if all required parameters were read
        successfully, else FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxCeGetRegistrySettings(
    LPCTSTR         lptzActiveKey,
    FFXDISKHOOK    *pHook)
{
    DWORD           dwValLen, dwValType;
    LONG            lError;
    TCHAR           tzDevKey[MAX_KEY_LEN];
    HKEY            hActiveKey = 0;
    HKEY            hDevKey = 0;
    D_BOOL          fSuccess;

    lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lptzActiveKey, 0, 0, &hActiveKey);
    if(lError != ERROR_SUCCESS)
    {
        FFXPRINTF(1, ("FfxCeGetRegistrySettings() failed to open the active key, Error=%ld\n", lError));
        return FALSE;
    }

    dwValLen = sizeof(tzDevKey);
    lError = RegQueryValueEx(hActiveKey, DEVLOAD_DEVKEY_VALNAME,
                             NULL, &dwValType, (PUCHAR) tzDevKey, &dwValLen);
    RegCloseKey(hActiveKey);
    if(lError != ERROR_SUCCESS)
    {
        FFXPRINTF(1, ("FfxCeGetRegistrySettings() failed to find the device key name, error=%ld\n", lError));
        return FALSE;
    }

    FFXPRINTF(1, ("DevKey='%W'\n", tzDevKey));

    lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, tzDevKey, 0, 0, &hDevKey);
    if(lError != ERROR_SUCCESS)
    {
        FFXPRINTF(1, ("FfxCeGetRegistrySettings() failed to open the device key, Error=%ld\n", lError));
        return FALSE;
    }

    fSuccess = ReadRegistryParameters(hDevKey, pHook);
    RegCloseKey(hDevKey);

    if(!fSuccess)
        return fSuccess;

    /*-----------------------------------------------------------
        Get stuff from the "profiles" section
    -----------------------------------------------------------*/
    {
        TCHAR               tzTempName[MAX_KEY_LEN];
        static const TCHAR  tzPrefix[] = TEXT("System\\StorageManager\\Profiles\\");
        HKEY                hProfKey = 0;

        DclAssert(sizeof tzPrefix < sizeof tzTempName);

        wcscpy(tzTempName, tzPrefix);

        /*  The entire profile key name (including the prefix) must fit
            within MAX_KEY_LEN.
        */
        if(wcslen(tzTempName) + wcslen(pHook->tzProfileName) >= MAX_KEY_LEN)
        {
            FFXPRINTF(1, ("FfxCeGetRegistrySettings() profile key is too long.\n"));
            return FALSE;
        }
        
        wcscat(tzTempName, pHook->tzProfileName);

        FFXPRINTF(1, ("ProfKey='%W'\n", tzTempName));

        lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, tzTempName, 0, 0, &hProfKey);
        if(lError != ERROR_SUCCESS)
        {
            FFXPRINTF(1, ("FfxCeGetRegistrySettings() failed to open the profile key, Error=%ld\n", lError));
            return FALSE;
        }

        /*-----------------------------------------------------------
            Get the Folder value
        -----------------------------------------------------------*/
        dwValLen = sizeof(pHook->tzFolderName);

        lError = RegQueryValueEx(hProfKey, TEXT("Folder"), NULL, &dwValType,
                            (PUCHAR) pHook->tzFolderName, &dwValLen);
        if(lError != ERROR_SUCCESS)
        {
            FFXPRINTF(1, ("FfxCeGetRegistrySettings() failed to find the folder name, will use CE default, Error=%ld\n", lError));

            /*  A null folder name will cause CE to use a default name
            */
            pHook->tzFolderName[0] = 0;
        }

        /*-----------------------------------------------------------
            Get the DefaultFileSystem value
        -----------------------------------------------------------*/
        dwValLen = sizeof(tzTempName);

        lError = RegQueryValueEx(hProfKey, TEXT("DefaultFileSystem"), NULL, 
                                &dwValType, (PUCHAR)tzTempName, &dwValLen);
        if(lError != ERROR_SUCCESS)
        {
            FFXPRINTF(1, ("FfxCeGetRegistrySettings() failed to find the 'DefaultFileSystem' value, assuming FATFS, Error=%ld\n", lError));

            pHook->nDefaultFS = DCL_FILESYS_FAT;
        }
        else
        {
            if(wcsicmp(tzTempName, TEXT("FATFS")) == 0)
            {
                pHook->nDefaultFS = DCL_FILESYS_FAT;
            }
            else if(wcsicmp(tzTempName, TEXT("RELIANCE")) == 0)
            {
                pHook->nDefaultFS = DCL_FILESYS_RELIANCE;
            }
            else
            {
                FFXPRINTF(1, ("FfxCeGetRegistrySettings() unrecognized 'DefaultFileSystem' value of \"%W\", using DCL_FILESYS_UNKNOWN\n", tzTempName));
                pHook->nDefaultFS = DCL_FILESYS_UNKNOWN;
            }
        }

      #if D_DEBUG
        if(fSuccess)
            FFXPRINTF(1, ("DefaultFileSystem=%u\n", pHook->nDefaultFS));
      #endif

        RegCloseKey(hProfKey);
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Local: ReadRegistryParameters()

    Read all of the parameters stored under the device key from the
    registry.  Fill in appropriate defaults for optional parameters.

    Parameters:
        hDevKey - the open device key
        pHook     - the DEVEXTRA structure for the device

    Return Value:
        Returns TRUE if all required parameters were read
        successfully, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL ReadRegistryParameters(
    HKEY            hDevKey,
    FFXDISKHOOK    *pHook)
{
    DWORD           dwValLen, dwValType;
    LONG            lError;

    /*-----------------------------------------------------------
        Get the Index value
    -----------------------------------------------------------*/
    {
        dwValLen = sizeof(pHook->dwIndex);

        lError = RegQueryValueEx(hDevKey, TEXT("Index"), NULL,
                             &dwValType, (PUCHAR)&pHook->dwIndex, &dwValLen);

        if(lError != ERROR_SUCCESS)
        {
            FFXPRINTF(1, ("ReadRegistryParameters() failed to find the Index value, defaulting to 1\n"));
            pHook->dwIndex = 1;
        }

        FFXPRINTF(1, ("Index=%lU\n", pHook->dwIndex));
    }

    /*-----------------------------------------------------------
        Get the DeviceNum value
    -----------------------------------------------------------*/
    {
        dwValLen = sizeof(pHook->dwDeviceNum);

        lError = RegQueryValueEx(hDevKey, TEXT("DeviceNum"), NULL,
                             &dwValType, (PUCHAR)&pHook->dwDeviceNum, &dwValLen);

        if(lError != ERROR_SUCCESS)
        {
            FFXPRINTF(1, ("ReadRegistryParameters() failed to find the DeviceNum value, defaulting to DEV0\n"));
            pHook->dwDeviceNum = 0;
        }

        FFXPRINTF(1, ("DeviceNum=%lU\n", pHook->dwDeviceNum));
    }

    /*-----------------------------------------------------------
        Get the DiskNum value
    -----------------------------------------------------------*/
    {
        dwValLen = sizeof(pHook->dwDiskNum);

        lError = RegQueryValueEx(hDevKey, TEXT("DiskNum"), NULL,
                             &dwValType, (PUCHAR)&pHook->dwDiskNum, &dwValLen);

        if(lError != ERROR_SUCCESS)
        {
            FFXPRINTF(1, ("ReadRegistryParameters() failed to find the DiskNum value, defaulting to \"first available\"\n"));
            pHook->dwDiskNum = D_UINT32_MAX;
        }

      #if D_DEBUG
        if(pHook->dwDiskNum == D_UINT32_MAX)
            FFXPRINTF(1, ("DiskNum=FirstAvailable\n"));
        else
            FFXPRINTF(1, ("DiskNum=%lU\n", pHook->dwDiskNum));
      #endif
    }

    /*-----------------------------------------------------------
        Get the DiskFlags value
    -----------------------------------------------------------*/
    {
        dwValLen = sizeof(pHook->dwDiskFlags);

        lError = RegQueryValueEx(hDevKey, TEXT("DiskFlags"), NULL,
                             &dwValType, (PUCHAR)&pHook->dwDiskFlags, &dwValLen);

        if(lError != ERROR_SUCCESS)
        {
            FFXPRINTF(1, ("ReadRegistryParameters() failed to find the DiskFlags value, defaulting to 0x0\n"));
            pHook->dwDiskFlags = 0;
        }

        FFXPRINTF(1, ("DiskFlags=%lX\n", pHook->dwDiskFlags));
    }

    /*-----------------------------------------------------------
        Get the Format value (optional, default to FORMAT_NEVER)
    -----------------------------------------------------------*/
    {
        DWORD           dwFormat;

        dwValLen = sizeof(dwFormat);
        lError = RegQueryValueEx(hDevKey, TEXT("Format"), NULL,
                                 &dwValType, (PUCHAR) & dwFormat, &dwValLen);
        if(lError == ERROR_SUCCESS)
        {
            switch (dwFormat)
            {
                case FFX_FORMAT_NEVER:
                case FFX_FORMAT_ONCE:
                case FFX_FORMAT_ALWAYS:
                    break;
                default:
                    FFXPRINTF(1, ("ReadRegistryParameters() Invalid Format value, using default\n"));
                    dwFormat = FFX_FORMAT_NEVER;
                    break;
            }
        }
        else
        {
            FFXPRINTF(1, ("ReadRegistryParameters() failed to read Format value, using default. Error=%ld\n", lError));
            dwFormat = FFX_FORMAT_NEVER;
        }

        pHook->nFormatState = (unsigned) dwFormat;
        FFXPRINTF(1, ("FormatState=%u\n", pHook->nFormatState));
    }

    /*-----------------------------------------------------------
        Get the Start value (optional, default to BAD_FLASH_ADDRESS)
    -----------------------------------------------------------*/
    {
        /*  TBD: Should we change 'Start' to 'StartKB'?
        */
        dwValLen = sizeof(pHook->dwFlashOffsetKB);
        if(RegQueryValueEx(hDevKey, TEXT("Start"), NULL, &dwValType,
                   (PUCHAR)&pHook->dwFlashOffsetKB, &dwValLen) != ERROR_SUCCESS)
        {
            FFXPRINTF(1, ("ReadRegistryParameters(Start) - FAILED!\n"));

            pHook->dwFlashOffsetKB = BAD_FLASH_ADDRESS;
        }

        FFXPRINTF(1, ("Start=%lX KB\n", pHook->dwFlashOffsetKB));
    }

    /*-----------------------------------------------------------
        Get the Length value (optional, default to BAD_FLASH_LENGTH)
    -----------------------------------------------------------*/
    {
        /*  TBD: Should we change 'Length' to 'LengthKB?'
        */
        dwValLen = sizeof(pHook->dwFlashLengthKB);
        if(RegQueryValueEx(hDevKey, TEXT("Length"), NULL, &dwValType,
                     (PUCHAR)&pHook->dwFlashLengthKB, &dwValLen) != ERROR_SUCCESS)
        {
            FFXPRINTF(1, ("ReadRegistryParameters(Length) - FAILED!\n"));

            pHook->dwFlashLengthKB = BAD_FLASH_LENGTH;
        }

        FFXPRINTF(1, ("Length=%lX KB\n", pHook->dwFlashLengthKB));
    }

    /*-----------------------------------------------------------
        Get the ProfileName value
    -----------------------------------------------------------*/
    {
        dwValLen = sizeof(pHook->tzProfileName);
        lError = RegQueryValueEx(hDevKey, TEXT("ProfileName"), NULL,
                            &dwValType, (PUCHAR) pHook->tzProfileName, &dwValLen);
        if(lError != ERROR_SUCCESS)
        {
            /*  Later versions of CE seem to use "Profile" rather than
                "ProfileName".  Allow either to be used for backwards
                compatibility.
            */
            dwValLen = sizeof(pHook->tzProfileName);
            lError = RegQueryValueEx(hDevKey, TEXT("Profile"), NULL,
                            &dwValType, (PUCHAR) pHook->tzProfileName, &dwValLen);
            if(lError != ERROR_SUCCESS)
            {
                FFXPRINTF(1, ("ReadRegistryParameters() failed to find the profile name, Error=%ld\n", lError));
                return FALSE;
            }
        }
    }

    return TRUE;
}
