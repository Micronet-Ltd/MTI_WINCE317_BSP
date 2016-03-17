#include "usbser_api.h"
#include "ftdi_ioctl.h"

BOOL MIC_EnumUsbSerialDevices(USB_SER_ENUM_PROC lpEnumFunc, LPVOID pUserData)
{
	BOOL res = TRUE, fOK;
	TCHAR portName[8];
	HANDLE hPort;
	USB_SERIAL_DEVICE_INFO info;
	USB_SERIAL_DEVICE_TYPE deviceType;

	if (!lpEnumFunc)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	for (int i = 1; i <= 9 && res; i++)
	{
		wsprintf(portName, _T("MOM%d:"), i);
		hPort = CreateFile(portName, DEVACCESS_BUSNAMESPACE, 0, 0, OPEN_EXISTING, 0, NULL);
		if (hPort != INVALID_HANDLE_VALUE)
		{
			fOK = DeviceIoControl(hPort, IOCTL_FT_GET_USB_DEVICE_EXT_INFO, 0, 0, &info, sizeof(info), 0, 0);
			CloseHandle(hPort);
			if (fOK)
			{
				deviceType = info.DeviceType;
				if (deviceType == USB_SERIAL_DEVICE_TYPE_UNKNOWN || deviceType >= USB_SERIAL_DEVICE_TYPE_MODEM)
				{
					// Call application callback
					res = lpEnumFunc(&info, pUserData);

					// Has application changed the device type?
					if (info.DeviceType != deviceType && (info.DeviceType == USB_SERIAL_DEVICE_TYPE_UNKNOWN || info.DeviceType >= USB_SERIAL_DEVICE_TYPE_MODEM))
					{
						hPort = CreateFile(portName, DEVACCESS_BUSNAMESPACE, 0, 0, OPEN_EXISTING, 0, NULL);
						if (hPort != INVALID_HANDLE_VALUE)
						{
							DeviceIoControl(hPort, IOCTL_FT_GET_SET_DEVICE_TYPE, &info.DeviceType, sizeof(info.DeviceType), 0, 0, 0, 0);
							CloseHandle(hPort);
						}
					}
				}
			}
		}
	}

	return res;
}