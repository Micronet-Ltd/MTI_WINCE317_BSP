#include <windows.h>
#include <keypad_user.h>
#include <keypad.h>

BOOL MIC_SwitchKeysMap(KEYPAD_KEYS_MAP table)
{
	BOOL ret = 0;
	UINT32 map_idx = table;
	HANDLE kpd = CreateFile(L"KPD1:", GENERIC_WRITE | GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);

	if(INVALID_HANDLE_VALUE != kpd)
	{
		ret = DeviceIoControl(kpd, IOCTL_KEYPAD_SWITCH_MAP, &map_idx, sizeof(map_idx), 0, 0, 0, 0);
		CloseHandle(kpd);
	}

	return ret;
}