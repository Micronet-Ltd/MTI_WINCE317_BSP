#include "StdAfx.h"
#include "Utils.h"

Utils::Utils()
{
}

Utils::~Utils()
{
}

wstring Utils::StringToWString(string str)
{
	std::wstring wStr;
	
	WCHAR wstrTemp[MAX_PATH];
	SecureZeroMemory(wstrTemp, sizeof(wstrTemp));

	int nLength = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str.c_str(), str.length(), wstrTemp, 0);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str.c_str(), str.length(), wstrTemp, nLength);

	wStr = wstrTemp;
	return wStr;
}

string Utils::WStringToString(wstring wstr)
{
	std::string str(wstr.begin(), wstr.end());

	return str;
}

void Utils::Split(string str, string sep, queue<string> &tokens)
{
	char *p = strtok((char *) str.c_str(), sep.c_str());
	while(p)
	{
		tokens.push(p);
		p = strtok(NULL, sep.c_str());
	}
}

void Utils::Split(string str, string sep, string &left, string &right)
{
	char *p = strtok((char *) str.c_str(), sep.c_str());

	if(p)
	{
		left = p;
		right = strtok(NULL, sep.c_str());
	}
}

tm Utils::GetTimestamp()
{
	struct tm timeinfo;

	__time64_t long_time;
	_time64(&long_time);                
	_localtime64_s(&timeinfo, &long_time); 

	return timeinfo;
}

tm Utils::CalculateDuration(tm &start, tm &end)
{
	int totalStartSeconds = start.tm_hour*360+start.tm_min*60+start.tm_sec;
	int totalEndSeconds = end.tm_hour*360+end.tm_min*60+end.tm_sec;

	int diff = totalEndSeconds-totalStartSeconds;

	struct tm timeinfo;
	SecureZeroMemory(&timeinfo, sizeof(tm));

	timeinfo.tm_hour = diff/3600;
	diff = diff%3600;
	timeinfo.tm_min = diff/60;
	diff = diff%60;
	timeinfo.tm_sec = diff;

	return timeinfo;
}

int Utils::GetSecondsFromTimestamp(tm &timestamp)
{
	return timestamp.tm_hour*360+timestamp.tm_min*60+timestamp.tm_sec;
}