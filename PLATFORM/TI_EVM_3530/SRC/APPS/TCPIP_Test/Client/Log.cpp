#include "stdafx.h"
#include "Log.h"

#include <time.h>
#include <wchar.h>
#include <sstream>

LRESULT WINAPI LogThread(LPVOID params);

void Log::Initialize()
{
	wchar_t buffer[80];
	struct tm m_LastTimestamp;

	__time64_t long_time;
	_time64(&long_time);                
	_localtime64_s(&m_LastTimestamp, &long_time); 

	wcsftime(buffer, 80, TEXT("%d-%m-%Y %H.%M.%S.log"), &m_LastTimestamp);	
	
	Initialize(AfxGetUtils().WStringToString(buffer));
}

void Log::Initialize(std::string &fileName)
{
	TCHAR szPath[MAX_PATH] = {0}, szAppDirectory[MAX_PATH] = {0};

	GetModuleFileName(NULL, szPath, MAX_PATH - sizeof(TCHAR));
	_tcsncpy(szAppDirectory, szPath, _tcsrchr(szPath, '\\') - szPath);
	wcscat(szAppDirectory, TEXT("\\Logs"));
	::CreateDirectory(szAppDirectory, NULL);

	wsprintf(szPath, _T("%s\\"), szAppDirectory);
	m_strFileName = AfxGetUtils().WStringToString(szPath);
	m_strFileName.append(fileName);

	InitializeCriticalSection(&m_Lock);

	m_hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	DWORD threadID;
	m_hLogThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &LogThread, (LPVOID) m_hWaitEvent, 0, &threadID);

	m_bInitialized = true;
}

void Log::Finalize()
{
	ENSURE_INITIALIZED;

	SetEvent(m_hWaitEvent);
	while(WaitForSingleObject(m_hWaitEvent, 500) == WAIT_OBJECT_0);

	m_bInitialized = false;

	CloseHandle(m_hWaitEvent);
	CloseHandle(m_hLogThread);

	DeleteCriticalSection(&m_Lock);
}

void Log::FlushAllLines()
{
	ENSURE_INITIALIZED;

	fstream fs;
	fs.open(m_strFileName.c_str(), ios::app);

	if(fs.is_open())
	{
		while(AfxGetLog().HasLines())
		{
			AfxGetLog() >> fs;
		};
	}
}

void Log::FlushLine()
{
	ENSURE_INITIALIZED;

	fstream fs;
	fs.open(m_strFileName.c_str(), ios::app);

	if(fs.is_open())
	{
		AfxGetLog() >> fs;
	}
}

bool Log::HasLines()
{
	ENSURE_INITIALIZED;

	Locker lock(m_Lock);
	return !m_logLines.empty();
}

void Log::operator>>(string &str)
{
	ENSURE_INITIALIZED;

	Locker lock(m_Lock);
	if(m_logLines.empty())
		return;

	str = m_logLines.front();
	m_logLines.pop();
}

void Log::operator>>(fstream &fs)
{
	ENSURE_INITIALIZED;

	string line;
	AfxGetLog() >> line;

	fs << line;
}

void Log::operator<<(char *ln)
{
	string line = ln;
	AfxGetLog() << line;
}

void Log::operator<<(string &str)
{
	ENSURE_INITIALIZED;

	wchar_t buffer[80], duration_buffer[80];
	tm timeinfo;

	__time64_t long_time;
	_time64(&long_time);                
	_localtime64_s(&timeinfo, &long_time); 

	if((m_LastTimestamp.tm_hour == 0) && (m_LastTimestamp.tm_min == 0) && (m_LastTimestamp.tm_sec == 0))
	{
		memcpy(&m_LastTimestamp, &timeinfo, sizeof(tm));
	}

	tm time_during = AfxGetUtils().CalculateDuration(m_LastTimestamp, timeinfo);
	wcsftime(buffer, 80, TEXT("%x %X"), &timeinfo);	
	wcsftime(duration_buffer, 80, TEXT("%M:%S"), &time_during);	
	
	stringstream ss;
	ss << AfxGetUtils().WStringToString(buffer) << ", during(" << AfxGetUtils().WStringToString(duration_buffer) << "),  " << str << endl;

	memcpy(&m_LastTimestamp, &timeinfo, sizeof(tm));

	Locker lock(m_Lock);
	m_logLines.push(ss.str());
}

wstring Format(wchar_t *fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);

	wchar_t buffer[MAX_PATH];
	vswprintf(buffer, fmt, vl);
	wstring temp = buffer;

	va_end(vl);

	return temp;
}

string Log::Format(char *fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);

	char buffer[MAX_PATH];
	vsprintf(buffer, fmt, vl);
	string temp = buffer;

	va_end(vl);

	return temp;
}

LRESULT WINAPI LogThread(LPVOID params)
{
	HANDLE hWaitEvent = (HANDLE) params;
	Log& log = AfxGetLog();
	string line;
	int counter = PASS_PER_ITER;

	while(WaitForSingleObject(hWaitEvent, 1000) != WAIT_OBJECT_0)
	{
		while(log.HasLines() && (counter-- > 0))
		{
			log.FlushLine();
			Sleep(0);
		}

		counter = PASS_PER_ITER;
	}

	if(log.HasLines())
		log.FlushAllLines();

	ResetEvent(hWaitEvent);

	return 0;
}