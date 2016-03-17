#include <windows.h>

class Registry
{
public:
	static enum Hive
	{
		ClassesRoot = (DWORD)HKEY_CLASSES_ROOT,
		CurrentUser = (DWORD)HKEY_CURRENT_USER,
		LocalMachine = (DWORD)HKEY_LOCAL_MACHINE,
		Users = (DWORD)HKEY_USERS        
	};
};

class RegistryKey
{
private:
	HKEY m_hKey;

public:
	RegistryKey()
	{
		m_hKey = NULL;
	}

	~RegistryKey()
	{
		Close();
	}

	bool Open(Registry::Hive hive, LPCWSTR key)
	{
		LONG res = RegOpenKeyEx((HKEY)hive, key, 0, 0, &m_hKey); 
		return res == ERROR_SUCCESS;
	}

	void Close()
	{
		if (IsOpen())
		{
			RegCloseKey(m_hKey);
			m_hKey = NULL;
		}
	}

	void Flush()
	{
		if (IsOpen())
			RegFlushKey(m_hKey);
	}

	bool IsOpen()
	{
		return m_hKey != NULL;
	}

	bool SetValue(LPCWSTR valueName, DWORD type, BYTE* data, DWORD cbData)
	{
		if (!IsOpen())
			return false;

		LONG res = RegSetValueEx(m_hKey, valueName, 0, type, data, cbData);
		return res == ERROR_SUCCESS;
	}

	bool SetDwordValue(LPCWSTR valueName, DWORD value)
	{
		return SetValue(valueName, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
	}

	bool GetValue(LPCWSTR valueName, LPDWORD type, LPBYTE data, LPDWORD cbData)
	{
		if (!IsOpen())
			return false;

		LONG res = RegQueryValueEx(m_hKey, valueName, 0, type, data, cbData); 
		return res == ERROR_SUCCESS;
	}

	bool GetDwordValue(LPCWSTR valueName, LPDWORD value)
	{
		bool res = false;
		DWORD type, size;
		if (GetValue(valueName, &type, NULL, &size))
		{
			if (type == REG_DWORD && size == sizeof(DWORD))
				res = GetValue(valueName, &type, (LPBYTE)value, &size);
		}

		return res;
	}

	void GetDwordValue(LPCWSTR valueName, LPDWORD value, DWORD defaultValue)
	{
		if (!GetDwordValue(valueName, value))
			*value = defaultValue;
	}

	bool GetStringValue(LPCWSTR valueName, LPCWSTR value, DWORD cbData)
	{
		bool res = false;
		DWORD type = 0, size = 0;
		if (GetValue(valueName, &type, NULL, &size))
		{
			if (type == REG_SZ && size <= cbData)
				res = GetValue(valueName, &type, (LPBYTE)value, &size);
		}

		return res;
	}
};
