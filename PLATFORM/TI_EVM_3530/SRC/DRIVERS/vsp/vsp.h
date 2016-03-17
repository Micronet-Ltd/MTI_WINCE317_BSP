#include <windows.h>
#include "ftdi_ioctl.h"
#include "ObjectList.h"
#include "RegistryHelper.h"

#define VSP_RETAILS	0

class InitContext;
class VirtualSerialPort
{
private:
	InitContext *m_init;
	HANDLE m_port;

public:
	VirtualSerialPort(InitContext *init);
	InitContext *GetInitContext() { return m_init; };
	BOOL IsOpen() { return m_port != INVALID_HANDLE_VALUE; };

	VOID Open(TCHAR *PortName, DWORD AccessCode, DWORD ShareMode);
	VOID Close();
	ULONG Read(PUCHAR pTargetBuffer, ULONG BufferLength);
	ULONG Write(PUCHAR pSourceBytes, ULONG NumberOfBytes);
	BOOL IOControl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);
};

class InitContext
{
private:
	TCHAR m_portName[8];
	VirtualSerialPort *m_owner;
	ObjectList *m_list;

public:
	InitContext() : m_owner(NULL)
	{
		memset(m_portName, 0, sizeof(m_portName));
		m_list = new ObjectList(8);
	}

	~InitContext()
	{
		VirtualSerialPort *vsp;
		while (vsp = (VirtualSerialPort *)m_list->GetFirst())
			Close(vsp);

		delete(m_list);
	}

	BOOL Init(ULONG Identifier, LPCVOID lpvBusContext)
	{
		PFTDI_DEVICE pUsbFTDI = (PFTDI_DEVICE)lpvBusContext;
		RegistryKey reg;
		TCHAR value[8];
		if (reg.Open(Registry::LocalMachine, (LPCWSTR)Identifier))
		{
			if (reg.GetStringValue(_T("Name"), value, sizeof(value)))
			{
				RETAILMSG(VSP_RETAILS, (L"VSP:%S, Name = %s\r\n", __FUNCTION__, value));
				wcscpy(pUsbFTDI->usbDeviceInfo.PortName, value);
				if (reg.GetStringValue(_T("PortName"), value, sizeof(value)))
				{
					RETAILMSG(VSP_RETAILS, (L"VSP:%S, PortName = %s\r\n", __FUNCTION__, value));
					wcscpy(m_portName, value);
					return TRUE;
				}
			}
		}

		return FALSE;
	}

	BOOL IsOwner(VirtualSerialPort *vsp)
	{
		return m_owner == vsp;
	}

	VirtualSerialPort *Open(DWORD AccessCode, DWORD ShareMode)
	{
		if ((AccessCode & (GENERIC_READ | GENERIC_WRITE)) && m_owner)
		{
			SetLastError(ERROR_INVALID_ACCESS);
			return NULL;
        }

		VirtualSerialPort *vsp = new VirtualSerialPort(this);
		if (AccessCode & (GENERIC_READ | GENERIC_WRITE))
			m_owner = vsp;

		vsp->Open(m_portName, AccessCode, ShareMode);
		if (!vsp->IsOpen())
		{
			delete(vsp);
			SetLastError(ERROR_NOT_READY);
			return NULL;
		}

		m_list->Add(vsp);
		return vsp;
	}

	BOOL Close(VirtualSerialPort *vsp)
	{
		if (IsOwner(vsp))
			m_owner = NULL;

		vsp->Close();
		m_list->Delete(vsp);
		delete(vsp);
		return TRUE;
	}
};

VirtualSerialPort::VirtualSerialPort(InitContext *init) : m_port(INVALID_HANDLE_VALUE), m_init(init)
{
}

VOID VirtualSerialPort::Open(TCHAR *PortName, DWORD AccessCode, DWORD ShareMode)
{
	m_port = CreateFile(PortName, AccessCode, ShareMode, 0, OPEN_EXISTING, 0, NULL);
}

VOID VirtualSerialPort::Close()
{
	if (IsOpen())
	{
		CloseHandle(m_port);
		m_port = INVALID_HANDLE_VALUE;
	}
}

ULONG VirtualSerialPort::Read(PUCHAR pTargetBuffer, ULONG BufferLength)
{
	if (!m_init->IsOwner(this))
		SetLastError(ERROR_INVALID_ACCESS);
	else
	{
		DWORD BytesRead;
		if (ReadFile(m_port, pTargetBuffer, BufferLength, &BytesRead, 0))
			return (ULONG)BytesRead;
	}
	return (ULONG)-1;
}

ULONG VirtualSerialPort::Write(PUCHAR pSourceBytes, ULONG NumberOfBytes)
{
	if (!m_init->IsOwner(this))
		SetLastError(ERROR_INVALID_ACCESS);
	else
	{
		DWORD TotalWritten;
		if (WriteFile(m_port, pSourceBytes, NumberOfBytes, &TotalWritten, 0))
			return (ULONG)TotalWritten;
	}
	return (ULONG)-1;
}

BOOL VirtualSerialPort::IOControl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
	return DeviceIoControl(m_port, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut, 0);
}
