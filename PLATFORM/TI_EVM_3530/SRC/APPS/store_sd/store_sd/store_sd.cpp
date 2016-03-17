// store_sd.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "store_sd.h"
#include <windows.h>
#include <commctrl.h>

int set_store_power(CEDEVICE_POWER_STATE *ps)
{
	UINT32 ret, code;
	HANDLE h, hs;
	TCHAR buf[512];
    STOREINFO si = {0};
	PARTINFO  pi = {0};

	_tprintf(TEXT("Enamerate all stores\r\n"));
	si.cbSize = sizeof(STOREINFO);
    h = FindFirstStore(&si);

	if((HANDLE)-1 != h)
	{
		do
		{
			wsprintf(buf, L"{8DD679CE-8AB4-43c8-A14A-EA4963FAA715}\\%s", si.szDeviceName);
			ret = SetDevicePower(buf, POWER_NAME, *ps);
			if(ERROR_SUCCESS != ret)
			{
				hs = OpenStore(si.szDeviceName);
				if((HANDLE)-1 != hs)
				{
					ret = DeviceIoControl(hs, IOCTL_POWER_SET, 0, 0, ps, sizeof(CEDEVICE_POWER_STATE), (DWORD *)&code, 0);
					CloseHandle(hs);
					ret = (ret)?ERROR_SUCCESS:-1;
				}
			}
			_tprintf(TEXT("'%s' is%sset to D%d\r\n"), buf, (ret == ERROR_SUCCESS)?L" ":L" failure to ", *ps);

		}while(FindNextStore(h, &si));

		FindCloseStore(h);
	}

	return 0;
}
int is_dp_exist(TCHAR *v_name, unsigned long vn_len, TCHAR *dp_name, unsigned long dpn_len)
{
	int found = 0;
	HANDLE h, hs, hp;
    STOREINFO si = {0};
	PARTINFO  pi = {0};

	if(!dp_name)
		return found;

//	_tprintf(TEXT("Enamerate all stores\r\n"));
	si.cbSize = sizeof(STOREINFO);
    h = FindFirstStore(&si);

	if((HANDLE)-1 != h)
	{
		do
		{
//			_tprintf(TEXT("%d partition(s) is(are) found on '%s'\r\n"),si.dwPartitionCount, si.szDeviceName);
//			if(0 != _tcsnicmp(si.szDeviceName, v_name, vn_len))
//				continue;
//			if(si.dwPartitionCount < 2)
//				break;

			hs = OpenStore(si.szDeviceName);
			if((HANDLE)-1 == hs)
				continue;

//			_tprintf(TEXT("Enamerate all partition on '%s'\r\n"), si.szDeviceName);
			pi.cbSize = sizeof(pi);
			hp = FindFirstPartition(hs, &pi);
			if((HANDLE)-1 != h)
			{
				do
				{
//					_tprintf(TEXT("'%s' partition is found\r\n"), pi.szPartitionName);
//					_tprintf(TEXT("\t'%s' partition of %d sectors\r\n"),
//								(pi.bPartType == PART_BOOTSECTION)?TEXT("Binary"):
//								(pi.bPartType == PART_DOS32)?TEXT("FAT 32"):TEXT("Other"), pi.snNumSectors);
					if(pi.bPartType == PART_BOOTSECTION)
					{
						HANDLE h = OpenPartition(hs, pi.szPartitionName);
						if((HANDLE)-1 != h)
						{
							int ret = SetPartitionAttributes(h, PARTITION_ATTRIBUTE_BOOT | PARTITION_ATTRIBUTE_READONLY);
//							int ret = SetPartitionAttributes(h, PARTITION_ATTRIBUTE_READONLY);
							if(!ret)
								ret = GetLastError();
							CloseHandle(h);
						}
						continue;
					}
					if(pi.bPartType != PART_DOS32)
						continue;

					if(0 == _tcsnicmp(pi.szPartitionName, dp_name, dpn_len))
						found = 1;
				}while(FindNextPartition(hp, &pi) && (si.dwPartitionCount > 1) && !found);
				FindClosePartition(hp);
			}

			CloseHandle(hs);
		}while(FindNextStore(h, &si) && !found);

		FindCloseStore(h);
	}

//	_tprintf(TEXT("Data partition is%sfound\r\n"), (found)?TEXT(" "):TEXT(" not "));
	if(!found)
		MessageBox(0, L"Data partition doesn't exist or corrupted", L"Storage fatal error", MB_OK);

	return found;
}

int create_dp(TCHAR *v_name, unsigned long vn_len, TCHAR *dp_name, unsigned long dpn_len, unsigned long force)
{
	unsigned long ret = 0;
	HANDLE hs;

	if(!force)
	{
		if(!v_name)
		{
			v_name = TEXT("Flash Disk");
			vn_len = _tcslen(v_name);
		}

		if(!dp_name)
		{
			dp_name = TEXT("Part01");
			dpn_len = _tcslen(dp_name);
		}

		if(!is_dp_exist(v_name, vn_len, dp_name, dpn_len))
			force = 1;
	}

	if(force)
	{
		if(!dp_name)
			return ret;
		if(!v_name)
			return ret;

//		_tprintf(TEXT("Open %s store\r\n"), v_name);
		hs = OpenStore(v_name);
		if((HANDLE)-1 != hs)
		{
			ret = CreatePartitionEx(hs, dp_name, PART_DOS32, 0);
			CloseHandle(hs);
		}
	}

//	_tprintf(TEXT("FAT 32 partition is%screated\r\n"), (ret)?TEXT(" "):TEXT(" not "));
	if(!ret)
		MessageBox(0, L"Data partition hasn't created", L"Storage fatal error", MB_OK);

	return ret;
}

void FormatProgress(DWORD per)
{
	RETAILMSG(1, (_T("Format: %d%% complete\r\n"), per));

	_tprintf(TEXT("Format: %d%% complete\r\n"), per);
}

int FormatMessage(LPTSTR szMessage, LPTSTR szCaption, BOOL fYesNo)
{
	RETAILMSG(1, (_T("Finally: %s %s %d complete\r\n"), szMessage, szCaption, fYesNo));

	_tprintf(TEXT("Finally: %s %s %d complete\r\n"), szMessage, szCaption, fYesNo);

	return 1;
}

int format_dp(TCHAR *v_name, TCHAR *p_name)
{
	HANDLE hs;
	HANDLE hp;
	PARTINFO  pi = {0};
	DISK_INFO di = {0};

	if(!v_name)
		return 0;
	if(!p_name)
		return 0;

	_tprintf(TEXT("Open %s store\r\n"), v_name);

	hs = OpenStore(v_name);

	if((HANDLE)-1 != hs)
	{
		_tprintf(TEXT("Open '%s' partition\r\n"), p_name);
		hp = OpenPartition(hs, p_name);
		if((HANDLE)-1 != hs)
		{
			pi.cbSize = sizeof(pi);
			GetPartitionInfo(hp, &pi);

			if(pi.dwAttributes & PARTITION_ATTRIBUTE_MOUNTED)
			{
				_tprintf(TEXT("Dismount partition\r\n"));
				DismountPartition(hp);
			}

			PFN_FORMATVOLUME pfnFormatVolume = 0;

			HMODULE hm = LoadLibrary(TEXT("fatutil.dll"));
			if(hm)
			{
				FORMAT_OPTIONS fo;

				pfnFormatVolume = (PFN_FORMATVOLUME)GetProcAddress(hm, TEXT("FormatVolume"));
				if(pfnFormatVolume)
				{
					ULONG ret;
					if(DeviceIoControl(hp, DISK_IOCTL_GETINFO, &di, sizeof(di), &di, sizeof(di), &ret, 0))
					{
						fo.dwClusSize = di.di_bytes_per_sect;//*512;
						fo.dwRootEntries = 512;
						fo.dwFatVersion = 64;
						fo.dwNumFats = 2;
						fo.dwFlags = FATUTIL_FORMAT_TFAT | FATUTIL_FORMAT_EXFAT | FATUTIL_FULL_FORMAT;

						_tprintf(TEXT("Preparing to Transactional FAT format:\r\n\tFAT version: exFAT\r\n\tClaster size: %d\r\n\tRoot entries %d\r\n\tFat copies %d\r\n"),
							fo.dwClusSize, fo.dwRootEntries, fo.dwNumFats);
						pfnFormatVolume(hp, &di, &fo, FormatProgress, FormatMessage);
					}
				}
				FreeLibrary(hm);
			}

			MountPartition(hp);
			CloseHandle(hp);
		}
		CloseHandle(hs);
	}

	return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
//	UINT32 ret;
	CEDEVICE_POWER_STATE ps = PwrDeviceUnspecified;

	ps = D4;
	set_store_power(&ps);
	int c = getc(stdin);
	ps = PwrDeviceUnspecified;
	set_store_power(&ps);
	c = getc(stdin);


	return 0;
}
