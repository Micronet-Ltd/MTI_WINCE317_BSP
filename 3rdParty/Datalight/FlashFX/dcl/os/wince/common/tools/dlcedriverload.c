/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2008 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

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

    This program loads the DLL specified in the supplied registry file, using
    the settings contained in that registry file.  ActivateDevice() loads the
    DLL, then adds it to the active driver list.

    ToDo: The AddRegistryKeys function should be abstracted out and placed
          in a CE specific common library in DCL so that it can be shared by
          other apps, such as FSIT.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlcedriverload.c $
    Revision 1.5  2008/05/27 16:34:41Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.4  2008/03/22 20:23:38Z  Garyp
    Modified to use DclHtoUL() rather than sscanf(), as the latter is not available
    in all CE environments (bootloaders).
    Revision 1.3  2007/12/15 20:32:46Z  Garyp
    Fixed so the /quiet option properly spits out an unload command suitable
    for redirection to a batch file.
    Revision 1.2  2007/12/14 17:26:58Z  Garyp
    Commented out some non-functional code.
    Revision 1.1  2007/12/07 00:15:44Z  Garyp
    Initial revision
    Revision 1.5  2007/11/03 23:31:37Z  Garyp
    Added the standard module header.
    Revision 1.4  2007/07/18 21:54:38Z  keithg
    Now treat the handle passes as unsigned to avoid overflow.
    This tool now works with Win CE 6.0
    Revision 1.3  2007/05/09 16:26:52Z  keithg
    Fixed quiet option to work with the unload option
    Revision 1.2  2007/04/26 22:44:49Z  keithg
    Added support to unload a driver and a /quiet option - improved help screen.
    Revision 1.1  2007/01/21 19:07:30Z  Garyp
    Initial revision
    Revision 1.2  2006/12/14 00:52:05Z  Garyp
    Added a standard header and updated the code formatting to current
    standards -- no functional changes.
    11/06/01 TWQ Created from Savereg.cpp
---------------------------------------------------------------------------*/

#include <windows.h>
#include <stdio.h>
#include <assert.h>

#include <dcl.h>
#include <dlcetools.h>

BOOL AddRegistryKeys(char *pszFileName, TCHAR *ptzDriverString, char *pszDriverName);

#define MAX_DRIVERNAME_LEN      (64)
#define MAX_DRIVERSTRING_LEN   (128)


/*-------------------------------------------------------------------
    Description

    Parameters

    Return Value
-------------------------------------------------------------------*/
DCLSTATUS DclCeDriverLoad(
    const char     *pszCmdName,
    char           *pszCmdLine)
{
    HANDLE          hDLL = NULL;
    TCHAR           tzDriverString[MAX_DRIVERSTRING_LEN];
    char            szDriverName[MAX_DRIVERNAME_LEN];
    unsigned        fUnloadDriver = FALSE;
    unsigned        fWaitForUnload = FALSE;
    unsigned        fQuiet = FALSE;

    /*  Set the flags based on the command line parameters
    */
    if(_strnicmp(pszCmdLine, "/quiet", 6)==0)
    {
        fQuiet = TRUE;
        pszCmdLine += 6;
    }
    else
    {
        DclPrintf("%s v2.1\n", pszCmdName);
        DclPrintf("Copyright (c) 2001-2007 Datalight, Inc.  All Rights Reserved Worldwide.\n\n");
    }

    /*  Skip any preceding white space before the command line option
    */
    while(*pszCmdLine && isspace(*pszCmdLine))
        ++pszCmdLine;

    if(!*pszCmdLine || (strcmp(pszCmdLine, "/?") == 0))
    {
        DclPrintf("%s loads a device driver into memory and activates it, using the\n", pszCmdName);
        DclPrintf("settings in the specified registry file.  This command may be used to unload\n");
        DclPrintf("the driver as well.\n\n");
        DclPrintf("Syntax:  %s [/Quiet>] [/Wait] /Load:RegFile || /Unload:Handle\n\n", pszCmdName);
        DclPrintf("Where:\n");
        DclPrintf("             /? - This help information.\n");
        DclPrintf("         /Quiet - Suppresses all output except the command to unload (and error\n");
        DclPrintf("                  messages).  The output is suitable to be redirected to a batch\n");
        DclPrintf("                  which may be run later to unload the driver.\n");
        DclPrintf("          /Wait - After loading, displays an \"Unload Driver?\" dialog.\n");
        DclPrintf("  /Load:RegFile - Creates the registry entries in the specified file, and loads\n");
        DclPrintf("                  the DLL specified in that file.\n");
        DclPrintf(" /Unload:Handle - Unloads a previously loaded driver, with the specified handle.\n\n");
        DclPrintf("The registry text file may contain multiple sections, however the first section\n");
        DclPrintf("is used to derive the device path.  Also it is from the first section that the\n");
        DclPrintf("actual device driver file name is derived, using the \"DLL\" setting.\n\n");
        DclPrintf("Note that the /Quiet and /Wait options are order-sensitive.\n\n");
        DclPrintf("Returns zero if successful, or non-zero otherwise.\n\n");

        return DCLSTAT_HELPREQUEST;
    }

    /*  Set the flags based on the command line parameters
    */
    if(_strnicmp(pszCmdLine, "/wait", 5)==0)
    {
        fWaitForUnload = TRUE;
        pszCmdLine += 5;
    }

    /*  Skip any preceding white space before the command line option
    */
    while(*pszCmdLine && isspace(*pszCmdLine))
        ++pszCmdLine;

    /*  Set the flags based on the command line parameters
    */
    if(_strnicmp(pszCmdLine, "/load:", 6)==0)
    {
        pszCmdLine += 6;

        /*  Load the driver registry keys and the driver itself
        */
        if(!AddRegistryKeys(pszCmdLine, tzDriverString, szDriverName))
        {
            DclPrintf("\nFailed to load registry!  Aborting.\n");
            return DCLSTAT_FAILURE;
        }

        if(!szDriverName[0])
        {
            DclPrintf("\nUnable to find a \"DLL\"=\"FileName\" setting in the specified registry file.\n\n");
            return DCLSTAT_FAILURE;
        }

        if(!fQuiet)
        {
            DclPrintf("Created registry settings for \"%W\"\n", tzDriverString);
            DclPrintf("Loading \"%s\"...\n", szDriverName);
        }

        hDLL = ActivateDevice(tzDriverString, 0);
        if(!hDLL || hDLL == INVALID_HANDLE_VALUE)
        {
            DclPrintf("\nFailed to load driver \"%W\".  Error=%lD\n", tzDriverString, GetLastError());
            return DCLSTAT_FAILURE;
        }

        if(fWaitForUnload)
        {
            TCHAR   tzTempBuff[MAX_DRIVERNAME_LEN+64];

            if(!fQuiet)
                DclPrintf("Waiting for dialog confirmation to unload the driver...\n");

            swprintf(tzTempBuff, TEXT("Press OK to unload %S, or Close\nto leave the driver loaded..."), szDriverName);

            if(MessageBox(NULL, tzTempBuff, TEXT("Driver Unload"), MB_OKCANCEL|MB_ICONINFORMATION) == IDOK)
            {
                fUnloadDriver = TRUE;
            }

            /*  Since the reg file loaded, we know there were no trailing,
                invalid parameters.  Simply point to the end of the string
                to ensure that the bad parameter check does not trigger.
            */
            pszCmdLine += DclStrLen(pszCmdLine);
        }
        else
        {
            /*  You would think that this should work to allow the module
                handle to be determined automatically, but this always
                seems to return 0.  Maybe it's a CE thing...

                TCHAR   tzTmp[32];
                MultiByteToWideChar(CP_ACP, 0, szDriverName, -1, tzTmp, sizeof(tzTmp));
                DclPrintf("module handle is %lX\n", GetModuleHandle(tzTmp));
            */

            /*  Show how to unload the driver
            */
            if(!fQuiet)
                DclPrintf("\nRun the following command to unload the driver:\n");

            DclPrintf("    %s /unload:%lX\n", pszCmdName, (D_UINT32)hDLL);

            return DCLSTAT_SUCCESS;
        }
    }
    else if(_strnicmp(pszCmdLine, "/unload:", 8)==0)
    {
        char   *pszEnd;

        hDLL = (HANDLE)strtoul(pszCmdLine+8, &pszEnd, 16);

        DclStrNCpy(szDriverName, pszCmdLine+8, sizeof(szDriverName));
        fUnloadDriver = TRUE;

        while(*pszEnd && isspace(*pszEnd))
            pszEnd++;

        if(*pszEnd)
        {
            DclPrintf("Unrecognized option \"%s\"... aborting\n", pszEnd);
            return DCLSTAT_BADPARAMETER;
        }
    }
    else
    {
        if(*pszCmdLine)
            DclPrintf("Unrecognized option \"%s\"... aborting\n", pszCmdLine);
        else
            DclPrintf("A /Load or /Unload parameter must be specified\n");

        return DCLSTAT_BADPARAMETER;
    }

    if(fUnloadDriver)
    {
        if(!fQuiet)
            DclPrintf("Unloading \"%s\"...\n", szDriverName);

        if(!DeactivateDevice(hDLL))
        {
            DclPrintf("Error unloading \"%s\".  Error=%lD\n", szDriverName, GetLastError());
            return DCLSTAT_FAILURE;
        }

        if(!fQuiet)
            DclPrintf("\"%s\" successfully unloaded\n", szDriverName);
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Description

    Parameters

    Return Value
-------------------------------------------------------------------*/
BOOL ConvertToUnicode(
    char   *pInputStr,
    WCHAR  *pOutputStr,
    int     OutputBufferSize)
{
    return MultiByteToWideChar(CP_ACP, 0, pInputStr, -1, pOutputStr, OutputBufferSize);
}


/*-------------------------------------------------------------------
    Description

    Parameters

    Return Value
-------------------------------------------------------------------*/
BOOL ReadLine(
    HANDLE      hFile,
    char       *pLine,
    int        *pNumChars)
{
    int         ii;
    BOOL        success = FALSE;
    char        inChar;
    DWORD       bytesRead;

    for(ii = 0; ii < *pNumChars; ii++)
    {
        if(!ReadFile(hFile, &inChar, 1, &bytesRead, NULL))
            break;

        pLine[ii] = inChar;
        if(inChar == '\n')
        {
            success = TRUE;
            break;
        }
    }

    *pNumChars = ii;
    if((0 == ii) || success)
        return TRUE;
    else
        return FALSE;
}


/*-------------------------------------------------------------------
    Description

    Parameters

    Return Value
-------------------------------------------------------------------*/
BOOL CreateValue(
    char       *pLine,
    HKEY        hKey)
{
    int         ii;
    char       *pRemainder;
    char       *pValueName;
    void       *pValue;
    DWORD       dwordVal;
    DWORD       type;
    WCHAR       wideValueName[512];
    WCHAR       wideValue[512];
    DWORD       size;

    /*  First, parse out the name of the key
    */
    for(ii = 1; pLine[ii] && (pLine[ii] != '\"'); ii++);
    if(0 == pLine[ii])
        return FALSE;

    pLine[ii] = 0;
    pRemainder = &pLine[ii + 1];
    pValueName = &pLine[1];

    /*  Now, check what comes after the equals sign
    */
    if(pRemainder[0] != '=')
        return FALSE;

    switch(pRemainder[1])
    {
        case '\"':
            /*  String value.  Parse out the full string
            */
            for(ii = 2; pRemainder[ii] && (pRemainder[ii] != '\"'); ii++);
            if(0 == pRemainder[ii])
                return FALSE;

            pRemainder[ii] = 0;
            type = REG_SZ;

            /*  Convert the value name to unicode
            */
            if(!ConvertToUnicode(&pRemainder[2], wideValue, sizeof(wideValue) / sizeof(WCHAR)))
                return FALSE;

            /*  Record the value and the size
            */
            pValue = (void *) wideValue;
            size = (wcslen(wideValue) + 1) * sizeof(WCHAR);
            break;

        case 'd':
            /*  Dword value.  Make sure it starts with "dword:"
            */
            if((strlen(&pRemainder[1]) < (strlen("dword:") + 1)) ||
                (strncmp(&pRemainder[1], "dword:", strlen("dword:")) != 0))
                return FALSE;

            /*  Convert it to a dword value
            */
            if(!DclHtoUL(&pRemainder[1 + strlen("dword:")], &dwordVal))
                return FALSE;

            pValue = (void *) &dwordVal;
            type = REG_DWORD;
            size = sizeof(DWORD);
            break;

        default:
            return FALSE;
    }

    /*  Convert the value name to unicode
    */
    if(!ConvertToUnicode(pValueName, wideValueName, sizeof(wideValueName) / sizeof(WCHAR)))
        return FALSE;

    /*  Create the value
    */
    if(RegSetValueEx(hKey, wideValueName, 0, type, pValue, size) != ERROR_SUCCESS)
        return FALSE;

    return TRUE;
}


/*-------------------------------------------------------------------
    AddRegistryKeys()

    Description
        This function parses the specified text file, and creates
        the specified registry sections and key values.

    Parameters
        pszFileName     - A pointer to the null-terminated file name
        ptzDriverString - A pointer to a buffer in which to store
                          the driver path.
        pszDriverName   - A pointer to a buffer in which to store
                          the driver file name.

    Return Value
        Returns TRUE if successful or FALSE otherwise.
-------------------------------------------------------------------*/
static BOOL AddRegistryKeys(
    char       *pszFileName,
    TCHAR      *ptzDriverString,
    char       *pszDriverName)
{
    BOOL        success = FALSE;
    HANDLE      hFile;
    char        line[512];
    WCHAR       wideSubkeyName[512];
    int         numChars;
    int         ii;
    int         jj;
    HKEY        hMasterKey = NULL;
    HKEY        hKey = NULL;
    DWORD       disposition;
    BOOL        fFirstGroup = FALSE;
    TCHAR       tzFileName[MAX_DRIVERSTRING_LEN];

    assert(pszFileName);
    assert(ptzDriverString);
    assert(pszDriverName);

    ConvertToUnicode(pszFileName, tzFileName, sizeof(tzFileName));

    /*  Default return values.
    */
    *ptzDriverString = 0;
    *pszDriverName = 0;

    /*  Open up the file
    */
    hFile = CreateFile(tzFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile)
    {
        DclPrintf("Unable to open registry file \"%W\"\n", tzFileName);
        return FALSE;
    }

    /*  Loop until the end
    */
    while(TRUE)
    {
        numChars = (sizeof(line) / sizeof(line[0])) - 1;

        /*  Read one line
        */
        if(!ReadLine(hFile, line, &numChars))
        {
            success = TRUE;
            break;
        }

        /*  Did we run out of file
        */
        if(0 == numChars)
        {
            success = TRUE;
            break;
        }

        /*  Null terminate the string
        */
        line[numChars] = 0;

        /*  Skip whitespace
        */
        for(ii = 0; line[ii]; ii++)
        {
            if(!isspace(line[ii]))
                break;
        }

        /*  See what type of line this is
        */
        if(line[ii] == '[')
        {
            /*  New registry location.  Replace the right bracket with a NULL.
            */
            for (jj = ii + 1; line[jj] && (line[jj] != ']'); jj++);
            if(0 == line[jj])
            {
                /*  no right bracket, skip this line
                */
                continue;
            }

            line[jj] = 0;

            /*  Get the first part of the registry path (the main key)
            */
            for(jj = ii + 1; line[jj] && (line[jj] != '\\'); jj++);
            if(0 == line[jj])
            {
                /*  No backslash, skip this line
                */
                continue;
            }

            line[jj] = 0;

            /*  See which key is being opened
            */
            if(strcmp(&line[ii + 1], "HKEY_LOCAL_MACHINE") == 0)
                hMasterKey = HKEY_LOCAL_MACHINE;
            else if(strcmp(&line[ii + 1], "HKEY_CURRENT_USER") == 0)
                hMasterKey = HKEY_LOCAL_MACHINE;
            else if(strcmp(&line[ii + 1], "HKEY_CLASSES_ROOT") == 0)
                hMasterKey = HKEY_LOCAL_MACHINE;
            else if(strcmp(&line[ii + 1], "HKEY_USERS") == 0)
                hMasterKey = HKEY_LOCAL_MACHINE;
            else
                continue;           /* Unknown key, skip this line */

            /*  Convert the name to unicode
            */
            if(!ConvertToUnicode(&line[jj + 1], wideSubkeyName, sizeof(wideSubkeyName) / sizeof(WCHAR)))
                break;

            /*  Close the previous registry key, if any
            */
            if(hKey)
            {
                RegCloseKey(hKey);
                hKey = NULL;
                fFirstGroup = FALSE;
            }
            else
            {
                if(hMasterKey)
                {
                    wcscpy(ptzDriverString, wideSubkeyName);

                    fFirstGroup = TRUE;
                }
            }

            /*  Open up the new key
            */
            if(RegCreateKeyEx(hMasterKey, wideSubkeyName, 0, NULL, REG_OPTION_NON_VOLATILE,
                               0, NULL, &hKey, &disposition) != ERROR_SUCCESS)
            {
                break;
            }
        }
        else if(line[ii] == '\"')
        {
            /*  If we are in the first section of the registry, find a "DLL"
                entry and return its value in pszDriverName.
            */
            if(fFirstGroup && (_strnicmp(&line[ii], "\"dll\"", 5) == 0))
            {
                int qq = ii+5;
                int rr;

                /*  The line must have the form "DLL"="DriverName.ext", where
                    DriverName is the name of the DLL load (white space is OK).
                */

                /*  Using a do...while contruct simply to avoid using goto
                */
                do
                {
                    /*  Skip whitespace
                    */
                    for(; line[qq]; qq++)
                    {
                        if(!isspace(line[qq]))
                            break;
                    }

                    if(!line[qq])
                        break;

                    if(line[qq] != '=')
                        break;

                    qq++;

                    /*  Skip whitespace
                    */
                    for(; line[qq]; qq++)
                    {
                        if(!isspace(line[qq]))
                            break;
                    }

                    if(!line[qq])
                        break;

                    if(line[qq] != '"')
                        break;

                    qq++;

                    for(rr = qq; line[rr] && (line[rr] != '"'); rr++);
                    if(!line[rr])
                    {
                        /*  No right quote, skip this line
                        */
                        break;
                    }

                    /*  Temporarily change the '"' to a NULL.
                    */
                    line[rr] = 0;

                    strcpy(pszDriverName, &line[qq]);

                    /*  Restore the quote.
                    */
                    line[rr] = '"';
                }while(FALSE);
            }

            if(!CreateValue(&line[ii], hKey))
                break;
        }
    }

    /*  Close our previous registry key
    */
    if(hKey)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }

    CloseHandle(hFile);
    return success;
}


