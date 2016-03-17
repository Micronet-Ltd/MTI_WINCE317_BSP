/*++

Copyright (c) 2001-2005  Future Technology Devices International Ltd.

Module Name:

    ftdi_debug.c

Abstract:

    Native USB device driver for FTDI FT8U2XX
    Debug utility routines

Environment:

    kernel mode only

Revision History:

	15/04/05	st		Debug functions added.


--*/
#include <windows.h>


#if defined(LOG_DEBUG)
void WcharToString(char *pDest, TCHAR * pSrc, ULONG Len)
{
    CHAR *d = pDest;
    WCHAR w;
    char c;

    while (Len--) {
        w = *pSrc++;
        w &= 0x00ff;

        c = (char) w;

        *d++ = c;

        if (w == 0)
            break;
    }
}

void WriteToDebugFile(TCHAR * pBuf)
{
	char cBuffer[1024];
	ULONG len = 1024;
	static BOOL bUsed = FALSE;
	FILE * fp = NULL;

	while(bUsed) { /* wait for it to be free */};
	bUsed = TRUE;
	
	// open the file
	fp = fopen("dbug.txt", "aw");
	WcharToString(cBuffer, pBuf, len);
	if(fp != NULL) {
		fseek(fp, 0, SEEK_END);
		fwrite(cBuffer, 1, strlen(cBuffer), fp);
		fclose(fp);
	}
	bUsed = FALSE;
}
#endif