#if !defined(WINCE)
#include "stdafx.h"
#endif
//Winbase.h
#include <stdio.h>
#include "MultiPlatformFile.h"
#if !defined(WINCE) && !defined(LINUX)
#include <io.h> // access
#include <fcntl.h> // open
#endif

#ifdef LINUX
#include <unistd.h>
#endif

MultiPlatformFile::MultiPlatformFile(void)
{
#if defined(WINCE) || defined(LINUX)
	hFile = NULL;
#else
	hFile = 0;
#endif
}

MultiPlatformFile::~MultiPlatformFile(void)
{

}

/*
	Flags isnt used at the moment as the Scope dll only used binary and read only 
	- in here for future if required
*/
INT MultiPlatformFile::Open(LPSTR lpFileName, DWORD Flags)
{
#if defined(WINCE) || defined(LINUX)
	if((hFile = fopen(lpFileName, "r+b")) != NULL )
#else
	if ((hFile = open(lpFileName, _O_RDONLY | _O_BINARY)) != -1)
#endif
		return 0;
	return -1;
}

BOOL MultiPlatformFile::IsOpen()
{
#if defined(WINCE) || defined(LINUX)
	if(hFile != NULL)
		return TRUE;
#else
	if(hFile != 0)
		return TRUE;
#endif
	return FALSE;
}

INT MultiPlatformFile::Close()
{
	int ret;
#if defined(WINCE) || defined(LINUX)
	ret = fclose(hFile);
	hFile = NULL;
	return ret;
#else
	ret = close(hFile);
	hFile = 0;
	return ret;
#endif
}

#ifdef LINUX
INT MultiPlatformFile::Read(void * pBuffer, int size, int count)
#else
INT MultiPlatformFile::Read(void * pBuffer, size_t size, size_t count)
#endif
{
#if defined(WINCE) || defined(LINUX)
	int iReadCount;

	iReadCount = fread(pBuffer, size, count, hFile);

	if(iReadCount < (int)count) {
		if(ferror(hFile))
			return -1;
	}

	return iReadCount;
#else
	return read(hFile, pBuffer, count);
#endif
}

#ifdef LINUX
INT MultiPlatformFile::Write(void * pBuffer, int size, int count)
#else
INT MultiPlatformFile::Write(void * pBuffer, size_t size, size_t count)
#endif
{
#if defined(WINCE) || defined(LINUX)
	return fwrite(pBuffer, size, count, hFile);
#else
	return write(hFile, pBuffer, count);
#endif
}

INT MultiPlatformFile::Seek(LONG offset, INT origin)
{
#if defined(WINCE) || defined(LINUX)
	if (fseek(hFile, offset, origin) == 0)
#else
	if (lseek(hFile, offset, origin) != -1L)
#endif
		return 0;
	return -1;
}

INT MultiPlatformFile::EOFile()
{
#if defined(WINCE) || defined(LINUX)
	return feof(hFile);
#else
	return eof(hFile);
#endif
}

INT MultiPlatformFile::Access(LPSTR lpFileNameAndPath, DWORD Flags)
{
#ifdef WINCE
	TCHAR tTemp[MAX_PATH];
	
	StringToWchar(tTemp, lpFileNameAndPath, MAX_PATH);
	if(GetFileAttributes(tTemp) == -1 && GetLastError() == ERROR_FILE_NOT_FOUND)
		return -1;
	return 0;
#else
	return access(lpFileNameAndPath, Flags);
#endif
}

/*
	C functions to do the string manipulation - WINCE is UNICODE which causes most of the problems.
 */
#ifdef WINCE
void WcharToString(char *pDest, PWCHAR pSrc, ULONG Len)
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

void StringToWchar(PWCHAR pDest, char * pSrc, ULONG Len)
{
    WCHAR *d = pDest;
    WCHAR w;
    char c;

    while (Len--) {

        c = *pSrc++;
        w = (WCHAR)c;

        *d++ = w;

        if (w == 0)
            break;

    }

}
#endif
