#include "stdafx.h"
#include "IniFile.h"

#include <string>
#include <iostream>
#include <fstream>

void IniFile::Read(wchar_t *file_name)
{
	string line;
	ifstream iniFile(file_name);

	if(iniFile.is_open())
	{
		map<string, string> section;
		string section_name;

		while(iniFile.good())
		{
			getline (iniFile,line);

			if(line[0] == '[')
			{
				if(section.size() != 0)
				{
					m_entries[section_name] = section;
				}

				section.clear();	

				line.replace(0, 1, "");
				line.replace(line.length()-1, 1, "");
				section_name = line; 
			}
			else
			{
				string left, right;
				AfxGetUtils().Split(line, "=", left, right);

				section[left] = right;
			}
		}

		if(section.size() != 0)
		{
			m_entries[section_name] = section;
		}

		iniFile.close();
	}
}

void IniFile::Save(wchar_t *file_name)
{
	ofstream iniFile(file_name);

	if(iniFile.is_open())
	{
		bool bFirstTime = true;

		map<string, map<string, string>>::const_iterator itr;
		for(itr = m_entries.begin(); itr != m_entries.end(); ++itr)
		{
			if(bFirstTime)
			{
				bFirstTime = false;
			}
			else
			{
				iniFile << "\n";
			}

			iniFile << "[" << (*itr).first.c_str() << "]\n";

			map<string, string>::const_iterator entry_itr;			
			for(entry_itr = (*itr).second.begin(); entry_itr != (*itr).second.end(); ++entry_itr)
			{
				if((*entry_itr).first.empty())
					continue;

				iniFile << (*entry_itr).first.c_str() << "=" << (*entry_itr).second.c_str() << "\n";
			}
		}
	}
}

string& IniFile::GetValue(char *section, char *key)
{
	return m_entries[section][key];
}

void IniFile::SetValue(char *section, char *key, const char *value)
{
	m_entries[section][key] = value;
}