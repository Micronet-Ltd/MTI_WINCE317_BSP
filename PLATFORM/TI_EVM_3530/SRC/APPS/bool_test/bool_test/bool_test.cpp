// bool_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>

BOOL child(void)
{
	static int i = 0;

	i++;

	if(i/2)
		return i;

	else
		return -1;
}

BOOL parent(void)
{
	return child();
}

int _tmain(int argc, _TCHAR* argv[])
{
	UINT32 u;

	u = parent();

	return 0;
}

