//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//

#include <windows.h>
#include <btagpub.h>


PFN_SendATCommand g_pfnSendATCommand = NULL;


void DbgPrintATCmd(DWORD dwZone, LPSTR szCommand, int cbCommand)
{
    CHAR szDebug[512];

    for (int i = 0, j = 0; i < cbCommand; i++, j++) {
        if (szCommand[i] == '\r') {
            szDebug[j] = '<'; j++;
            szDebug[j] = 'c'; j++;
            szDebug[j] = 'r'; j++;
            szDebug[j] = '>';
        }
        else if (szCommand[i] == '\n') {
            szDebug[j] = '<'; j++;
            szDebug[j] = 'l'; j++;
            szDebug[j] = 'f'; j++;
            szDebug[j] = '>';
        }
        else {
            szDebug[j] = szCommand[i];
        }
    }

    szDebug[j] = '\0';

    DEBUGMSG(dwZone, (L"%hs", szDebug));
}



void BthAGATSetCallback(PFN_SendATCommand pfn)
{
    g_pfnSendATCommand = pfn;
}

BOOL BthAGATHandler(LPSTR szCommand, DWORD cbCommand)
{
    DEBUGMSG(1, (L"----> BTAGATEXT SAMPLE: Received command: "));
    DbgPrintATCmd(1, szCommand, cbCommand);
    DEBUGMSG(1, (L"\n"));

    if (! _strnicmp(szCommand, "AT+BVRA", 7)) {
		HANDLE h = CreateFile(L"BAG0:",0,0,NULL,OPEN_EXISTING,0,NULL);
		if (INVALID_HANDLE_VALUE == h) {
			DEBUGMSG(1, (L"BTAGATEXT: Error opening handle to AG service.\n"));
			return FALSE;
		}

		DeviceIoControl(h,IOCTL_AG_OPEN_AUDIO,NULL,0,NULL,0,NULL,NULL);
		CloseHandle(h);

		g_pfnSendATCommand("\r\nOK\r\n", 6);
		return TRUE;
    }

    return FALSE;
}


