#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <wchar.h>

#include <string>

using namespace std;

class Socket
{
public:
	Socket();
	Socket(int bufferSize, HWND hCallbackWnd = NULL);
	~Socket();

	bool IsSocketOK() { return m_socket == INVALID_SOCKET; };
	bool IsWSAInitialized() { return m_bWSAInitialized; };

	bool Connect(string address, int port);
	void Disconnect();
	bool IsConnected();
	void SetConfig(int bufferSize, HWND hCallbackWnd = NULL);

	bool Send(void *data, DWORD size, bool bReportProgress = false);
	void *Receive(DWORD size, bool bReportProgress = false);

	int SetSOLOption(int optName, void *optValue, unsigned int optLen);
	int GetSOLOption(int optName, void *optValue, int optLen);

protected:
	bool CreateSocket();

// Static methods
public:
	static bool StartWSA();

protected:
	SOCKET	m_socket;
	int		m_bufferSize;
	HWND	m_hCallbackWnd;

protected:
	static bool m_bWSAInitialized;	
};