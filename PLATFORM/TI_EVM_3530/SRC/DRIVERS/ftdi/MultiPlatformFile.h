/*++

Copyright (c) 2004  Future Technology Devices International Ltd.

Module Name:

    MultiPlatformFile.h

Abstract:

    Class to allow cross platfrom file access (initially for Windows CE and Windows).

Environment:

   user mode

Revision History:

    26/07/04    st     Created.
	
--*/

#ifndef __MULTI_PLATFORM_FILE_H
#define __MULTI_PLATFORM_FILE_H
#ifdef WINCE
#ifndef strupr
#define strupr _strupr
#endif
#endif

#ifdef LINUX
#include "WinTypes.h"
#else
#include <windows.h>
#endif

class MultiPlatformFile
{
private:
#if defined(WINCE) || defined(LINUX)
  FILE * hFile;
#else
  int hFile;
#endif

public:
	INT Open(LPSTR lpFileName, DWORD Flags);
	BOOL IsOpen();
	INT Close();
#ifdef LINUX
	INT Read(void * pBuffer, int size, int count);
	INT Write(void * pBuffer, int size, int count);
#else
	INT Read(void * pBuffer, size_t size, size_t count);
	INT Write(void * pBuffer, size_t size, size_t count);
#endif
	INT Access(LPSTR lpFileNameAndPath, DWORD Flags);
	INT Seek(LONG offset, INT origin);
	INT EOFile();
	MultiPlatformFile(void);
	~MultiPlatformFile(void);

protected:
};


#ifdef WINCE
/* For general use in debug output */
void WcharToString(char *pDest, PWCHAR pSrc, ULONG Len);
void StringToWchar(PWCHAR pDest, char * pSrc, ULONG Len);
#endif

#endif  /* __MULTI_PLATFORM_FILE_H */
