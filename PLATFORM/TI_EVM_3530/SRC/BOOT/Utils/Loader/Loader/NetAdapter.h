#ifndef _NETADAPTER_H____
#define _NETADAPTER_H____

#include <windows.h>
#include <tchar.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Iphlpapi.h>
#include "LoaderAPI.h"



#define DEFAULT_FAMILY    AF_UNSPEC
#define DEFAULT_SOCKTYPE  SOCK_DGRAM

#define DEFAULT_PORT      990

#define SEND_CMD_ID    0
#define RECEIVE_CMD_ID 1




extern TCHAR  text[256];

#if defined(_DEBUG)
#define PRINT(x) wprintf(x)
#else
#define PRINT(x) OutputDebugString(x)
#endif



//////////////////////////////////////////////////////////////////////////////////////////
//	Desc:
//		Class wrapper for a single network adapter.  A listing of these adapters
//		can be built using the EnumNetworkAdapters(...) function prototyped
//		above.
//////////////////////////////////////////////////////////////////////////////////////////
class NetAdapter {

public:
		NetAdapter();
		~NetAdapter();
		BOOL  SetupConnection( IP_ADAPTER_INFO* pAdaptInfo,u_short portNo);

		DWORD DoConnect(void);
		DWORD CheckConnMsg( char* Buff,DWORD bufLen);
		DWORD SendConnAck();
		DWORD IOControl( DWORD dwIoControlCode, LPVOID lpInBuf,DWORD nInBufSize,LPVOID lpOutBuf,
						DWORD nOutBufSize, LPDWORD lpBytesReturned);
		DWORD ReceiveCmd( LPVOID lpInBuf,DWORD nInBufSize,LPVOID lpOutBuf,DWORD nOutBufSize, LPDWORD lpBytesReturned);
		
		DWORD SendCmd( LPVOID lpInBuf,DWORD nInBufSize,LPVOID lpOutBuf,DWORD nOutBufSize, LPDWORD lpBytesReturned);
		char* getRemoteIPString();
		BOOL isConnected();
		BOOL CloseConnection();

private:
	IP_ADAPTER_INFO m_adapterInfo;

    char             m_remoteIPStr[IP_STRING_LENGTH];
	sockaddr_in      m_remoteAddr;
	sockaddr_in      m_sourceAddr;
	u_short          m_portNo;
	int              m_remoteAddrSize;


	SOCKET          m_socket;
	DWORD           m_status;

	CRITICAL_SECTION	m_DeviceCritS;

};

/////////////////////////////////////////////
// Function Prototypes
//DWORD EnumNetworkAdapters( CLoader* lpBuffer, ULONG ulSzBuf, LPDWORD lpdwOutSzBuf );

#endif //_NETADAPTER_H____