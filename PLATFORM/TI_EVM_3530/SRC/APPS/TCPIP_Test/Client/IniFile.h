#pragma once
#include "Utils.h"
#include <map>

using namespace std;

class IniFile : public Singleton<IniFile>
{
public:
	IniFile(){}; 
	~IniFile(){}; 

	void Read(wchar_t *file_name);
	void Save(wchar_t *file_name);

	string& GetValue(char *section, char *key);
	void SetValue(char *section, char *key, const char *value);

private:
	map<string, map<string, string>> m_entries;
};

inline IniFile& AfxGetIniFile() { return IniFile::Instance(); };