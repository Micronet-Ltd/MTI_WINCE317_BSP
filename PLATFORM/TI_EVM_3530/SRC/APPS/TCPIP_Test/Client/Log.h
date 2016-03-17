#pragma once

#include "Utils.h"
#include "Locker.h"

#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include <fstream>

using namespace std;

#define LOG_FORMAT AfxGetLog().Format

#define ENSURE_INITIALIZED	\
	if(!m_bInitialized)		\
		Initialize()		\

#define PASS_PER_ITER 5

class Log : public Singleton<Log>
{
public:
	Log() : m_bInitialized(false){};
	~Log(){};

	void Initialize();
	void Initialize(string &fileName);
	void Finalize();

	bool HasLines();

	void operator<<(string &str);
	void operator<<(char *ln);
	void operator>>(string &str);
	void operator>>(fstream &fs);

	void FlushLine();	
	void FlushAllLines();

	string Format(char *fmt, ...);
	wstring Format(wchar_t *fmt, ...);

private:
	CRITICAL_SECTION m_Lock;
	string m_strFileName;
	queue<string> m_logLines;
	bool m_bInitialized;
	HANDLE m_hLogThread;
	HANDLE m_hWaitEvent;
	tm m_LastTimestamp;
};

inline Log& AfxGetLog() { return Log::Instance(); };