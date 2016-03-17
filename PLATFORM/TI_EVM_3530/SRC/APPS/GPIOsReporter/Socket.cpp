#include "stdafx.h"
#include "Socket.h"

using namespace std;

bool Socket::m_bWSAInitialized = FALSE;

Socket::Socket(int bufferSize, HWND hCallbackWnd) : m_socket(INVALID_SOCKET), m_bufferSize(bufferSize), m_hCallbackWnd(hCallbackWnd)
{
	CreateSocket();
}

Socket::Socket() : m_socket(INVALID_SOCKET), m_bufferSize(8096), m_hCallbackWnd(NULL)
{
	CreateSocket();
}

Socket::~Socket()
{
}

void Socket::SetConfig(int bufferSize, HWND hCallbackWnd)
{
	m_bufferSize = bufferSize;
	m_hCallbackWnd = hCallbackWnd;
}

int Socket::SetSOLOption(int optName, void *optValue, unsigned int optLen)
{
	return setsockopt(m_socket, SOL_SOCKET, optName, (const char *)optValue, optLen);
}

int Socket::GetSOLOption(int optName, void *optValue, int optLen)
{
	return getsockopt(m_socket, SOL_SOCKET, optName, (char *)optValue, &optLen);
}

bool Socket::CreateSocket()
{
	if(!m_bWSAInitialized)
		Socket::StartWSA();

	if((m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		//AfxGetLog() << LOG_FORMAT("Failed to create socket with error: %d.", WSAGetLastError());
		return false;
	};

	//AfxGetLog() << "Socket created.";

	return true;
}

void Socket::Disconnect()
{
	shutdown(m_socket, SD_BOTH);
	closesocket(m_socket);
	WSACleanup();

	m_bWSAInitialized = false;
}

bool Socket::Connect(string address, int port)
{
	if(!m_bWSAInitialized)
		CreateSocket();

	SOCKADDR_IN connection_address;
	connection_address.sin_family = AF_INET;
	connection_address.sin_port = htons(port);

	int hostIP = inet_addr(address.c_str());
	if(hostIP == INADDR_NONE)
	{
		HOSTENT *hostName;
		hostName = gethostbyname(address.c_str());
		
		if(hostName == NULL)
		{
			//AfxGetLog() << LOG_FORMAT("Failed to connect, error: %d.", WSAGetLastError());
			return false;
		}
		
		connection_address.sin_addr = *((IN_ADDR*)hostName->h_addr_list[0]);
	}
	else
		connection_address.sin_addr.s_addr = hostIP;

	if(connect(m_socket, (SOCKADDR*) &connection_address, sizeof(SOCKADDR_IN)) != 0)
	{
		//AfxGetLog() << LOG_FORMAT("Failed to connect, error: %d.", WSAGetLastError());
		return false;
	}

	//AfxGetLog() << LOG_FORMAT("Socket connected on %s:%d.", address.c_str(), port);

	return true;
}

void *Socket::Receive(DWORD size, bool bReportProgress)
{
	size_t bytes_read = 0;
	size_t bytes_received = 0;
	fd_set fd_read;
	
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 1000;

	ULONG percent_received = 0;
	char *buffer = (char *) malloc(m_bufferSize);
	char *bufferPtr = buffer;
	ULONG total_received = 0;
	DWORD ticks = GetTickCount() + timeout.tv_sec * 1000;

	do
	{
		if (ticks < GetTickCount())
			return NULL;

		FD_ZERO(&fd_read);		
		FD_SET(m_socket, &fd_read);

		if(select(m_socket, &fd_read, NULL, NULL, &timeout) == SOCKET_ERROR)
		{
			//AfxGetLog() << LOG_FORMAT("Select socket failed with error %d", WSAGetLastError());
			free(buffer);

			return NULL;
		}

		if(FD_ISSET(m_socket, &fd_read))
		{
			bytes_received = recv(m_socket, bufferPtr, size-total_received, 0);
			if(bytes_received == SOCKET_ERROR)
			{
				//AfxGetLog() << LOG_FORMAT("Receive failed, error: %d", WSAGetLastError());
				free(buffer);

				return NULL;
			}

			bufferPtr+=bytes_received;
			total_received += bytes_received;
		}
	}
	while(total_received != size);
	
	////AfxGetLog() << LOG_FORMAT("Receive completed, %d bytes received.", total_received);

	return buffer;
}

bool Socket::Send(void *data, DWORD size, bool bReportProgress)
{
	size_t bytes_sent = 5;
	fd_set fd_write;
	fd_set fd_error;
	
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 1000;

	FD_ZERO(&fd_write);		
	FD_SET(m_socket, &fd_write);
	FD_ZERO(&fd_error);		
	FD_SET(m_socket, &fd_error);

	ULONG percent_sent = 0;
	ULONG total_sent = 0;

	char *dataPtr = (char *) data;

	do
	{
		FD_ZERO(&fd_write);		
		FD_SET(m_socket, &fd_write);

		if(select(m_socket, NULL, &fd_write, &fd_error, &timeout) == SOCKET_ERROR)
		{
			//AfxGetLog() << LOG_FORMAT("Select socket failed with error %d.", WSAGetLastError());
			return false;
		}

		if(FD_ISSET(m_socket, &fd_error))
		{
			//AfxGetLog() << LOG_FORMAT("Select socket returned error %d.", WSAGetLastError());
			return false;
		}

		if(!FD_ISSET(m_socket, &fd_write))
		{
			//AfxGetLog() << "Socket is not ready to send, waiting...";
			return false;
		}
		else
		{
			bytes_sent = send(m_socket, (const char *) dataPtr, size-total_sent, 0);

			if(bytes_sent == SOCKET_ERROR)
			{
				//AfxGetLog() << LOG_FORMAT("Send failed, error: %d.", WSAGetLastError());
				return false;
			}

			dataPtr+=bytes_sent;
			total_sent += bytes_sent;
		}
	}
	while(total_sent < size);

	return true;
}

bool Socket::IsConnected()
{
	fd_set fd_write;
	fd_set fd_error;
	
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 1000 * 100;

	FD_ZERO(&fd_write);		
	FD_SET(m_socket, &fd_write);
	FD_ZERO(&fd_error);		
	FD_SET(m_socket, &fd_error);

	if (select(m_socket, NULL, &fd_write, &fd_error, &timeout) == SOCKET_ERROR)
	{
		//AfxGetLog() << LOG_FORMAT("Select socket failed with error %d.", WSAGetLastError());
		return false;
	}

	if (FD_ISSET(m_socket, &fd_error))
	{
		//AfxGetLog() << LOG_FORMAT("Select socket returned error %d.", WSAGetLastError());
		int err = WSAGetLastError();
		return false;
	}

	if (!FD_ISSET(m_socket, &fd_write))
	{
		//AfxGetLog() << "Socket is not ready to send, waiting...";
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////
// Static methods

bool Socket::StartWSA()
{
	if(m_bWSAInitialized)
		return true;

    WSADATA wsaData;
	
	if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		//AfxGetLog() << LOG_FORMAT("WSAStartup failed with error: %d.", WSAGetLastError());
		return false;
	}

	//AfxGetLog() << "WSA initialized.";
	m_bWSAInitialized = true;

	return true;
}
