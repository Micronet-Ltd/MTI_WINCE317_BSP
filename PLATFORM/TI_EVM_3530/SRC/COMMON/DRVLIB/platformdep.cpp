#include <windows.h>
#include <bsp.h>
#include "swupdate_api.h"

BOOL IsCe300()
{
	BspVersionExt ver;
	DWORD size;
	if (MIC_GetBspSwVersionExt(BSP_VERSION, (VOID*)&ver, sizeof(ver), &size))
	{
		DWORD series = (ver.dwVersionIncremental / 100) * 100;
		if (series == 300)
			return TRUE;
	}
	else
	{
		RETAILMSG (1, (TEXT("IsCe300: MIC_GetBspSwVersionExt failed\r\n")));
	}

	return FALSE;
}