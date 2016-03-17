#include "stdafx.h"
#include "bench2.h"
#include <windows.h>

__int64 qw_read(char *buf,int count)
{
	register unsigned __int64 dw = 0;
	volatile unsigned __int64 *wBuf=(unsigned __int64 *)buf;
	count=count/8;

	do
	{

		dw += *wBuf++ ;//= (WORD)0xABAB;

	}while(--count);

	return dw;
}

__inline int dw_copy(UINT32 *src, UINT32 *dst, int count)
{
	count=count/sizeof(UINT32);

//	if(dst > src)
//	{
//		do
//		{
//			*dst++ = *src++ ;
//		}while(--count);
//	}
//	else
//	{
		do
		{

			*dst++ = *src++ ;

		}while(--count);
//	}

	return count;
}

int w_copy(WORD *src, WORD *dst, int count)
{
	count=count/sizeof(WORD);

	do
	{

		*dst++ = *src++ ;

	}while(--count);

	return count;
}

int b_copy(BYTE *src, BYTE *dst, int count)
{
	count=count/sizeof(char);

	do
	{

		*dst++ = *src++ ;

	}while(--count);

	return count;
}

int buf_check(char *buf,int count)
{

	DWORD *dwBuf=(DWORD*)buf;
	count=count/4;

	do
	{


		if ( *dwBuf++ != (DWORD)0xABABABAB )
			return 0 ;

	}while(--count);

	return 1;  
} 

