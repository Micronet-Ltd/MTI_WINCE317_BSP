#include "stdafx.h"
#include "boot_i.h"

srv_param sp = {INVALID_SOCKET, 0, {0}};

unsigned long __stdcall socket_srv(void *pv_param)
{
	unsigned sock_type = SOCK_DGRAM;//SOCK_STREAM;
    int remote_addr_len, cbXfer, cbTotalRecvd;
    SOCKET s, ss;
    WSADATA wsaData;
	((srv_param *)pv_param)->Port = 980;
    unsigned int Port = ((srv_param *)pv_param)->Port;
	char szPort[] = "980";
    SOCKADDR_IN sin;
    SOCKADDR_IN sinc;
//    ADDRINFO ai, *addr_info = 0;
	FILE *file = 0;


	RECT	rect;
	GetClientRect( sp.w, &rect );

	int	TotalSent = 0;
	wsprintf( sp.TxTotal, TEXT("total Tx = %d "), TotalSent );
	wsprintf( sp.StatusTx, TEXT("Connecting"));
	InvalidateRect( sp.w, &rect, TRUE );

	if(WSAStartup(MAKEWORD(1,1), &wsaData))
	{
        // WSAStartup failed
        return -1;
    }

	__try
	{
/*
		memset(&ai, 0, sizeof(ai));
		ai.ai_family = AF_UNSPEC;
		ai.ai_socktype = sock_type;
		ai.ai_flags = AI_NUMERICHOST | AI_PASSIVE;

		if(getaddrinfo(0, szPort, &ai, &addr_info))
		{
			__leave;
		}
*/
		s = ss = INVALID_SOCKET;

		// Get a available address to serve on
		//

		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY; //INADDR_BROADCAST
		sin.sin_port = htons((USHORT)Port);


		// Create a serving sockets
		//

        ss = socket(AF_INET, sock_type, (sock_type == SOCK_STREAM)?IPPROTO_TCP:IPPROTO_UDP);
		if(ss == INVALID_SOCKET)
		{
			wsprintf( sp.StatusTx, TEXT("func socket failed err = %d"), WSAGetLastError() );
			InvalidateRect( sp.w, &rect, TRUE );
			__leave;
		}
		if(bind(ss, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
		{
			wsprintf( sp.StatusTx, TEXT("func bind failed err = %d"), WSAGetLastError() );
			InvalidateRect( sp.w, &rect, TRUE );
			__leave;
		}
		// Wait for incomming data/connections
		//

/*		if(listen(ss, SOMAXCONN) == SOCKET_ERROR)
		{
			wsprintf( sp.StatusTx, TEXT("func listen failed err = %d"), WSAGetLastError() );
			InvalidateRect( sp.w, &rect, TRUE );
			__leave;
		}
*/
		fd_set sock_set;
		FD_ZERO(&sock_set);

		if(select(1, &sock_set, 0, 0, 0))
			__leave;

		remote_addr_len = sizeof(sinc);
		// proceed for connected socket
		if(sock_type == SOCK_STREAM)
		{
			// TCP data available on socket
			s = accept(ss, (SOCKADDR*)&sinc, &remote_addr_len);
			if(s == INVALID_SOCKET) 
			{
				wsprintf( sp.StatusTx, TEXT("func accept failed err = %d"), WSAGetLastError() );
				InvalidateRect( sp.w, &rect, TRUE );
				__leave;
			}
		}
		else
		{
			// UDP data available on socket
			s = ss;
		}

		wsprintf( sp.StatusTx, TEXT("Connected %d.%d.%d.%d"), sinc.sin_addr.S_un.S_un_b.s_b1, sinc.sin_addr.S_un.S_un_b.s_b2,sinc.sin_addr.S_un.S_un_b.s_b3,sinc.sin_addr.S_un.S_un_b.s_b4);
		InvalidateRect( sp.w, &rect, TRUE );
		// Tx data to a client
		//

		OPENFILENAME ofn = {0};
		TCHAR name[256] = TEXT("default.txt");
		TCHAR dir[256] = TEXT("\\WINCE420\\PROJECTS\\net701core\\RelDir\\DRAGONBALLRelease");

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = sp.w;
		ofn.lpstrFilter = TEXT("Text files(*.txt)\0*.txt\0\0");
		ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
		ofn.lpstrTitle  = TEXT("Choose File");
		ofn.lpstrFile   = name;
		ofn.lpstrInitialDir = dir;
		ofn.nMaxFile    = sizeof(name);
		GetSaveFileName(&ofn);


		char pBuf[BUFFER_SIZE];

		cbXfer = send(s, (char *)&pBuf, sizeof(pBuf), 0);
		if(cbXfer == SOCKET_ERROR)
		{
			wsprintf( sp.StatusTx, TEXT("Cannot send prolog = %d"), WSAGetLastError() );
			InvalidateRect( sp.w, &rect, TRUE );
			__leave;
		}

		file = _tfopen(name, TEXT("rb"));

		int tc = GetTickCount();
		cbXfer = 0;
		do{
			cbTotalRecvd = fread(pBuf, 1, sizeof(pBuf), file);
			while(cbTotalRecvd)
			{
				cbXfer = send(s, (char *)pBuf, cbTotalRecvd, 0);

				if(cbXfer == SOCKET_ERROR)
				{
					wsprintf( sp.StatusTx, TEXT("func send failed err = %d"), WSAGetLastError() );
					InvalidateRect( sp.w, &rect, TRUE );
					__leave;
				}
				TotalSent += cbXfer;
				cbTotalRecvd -= cbXfer;
				wsprintf( sp.TxTotal, TEXT("total Tx = %d "), TotalSent );
				InvalidateRect( sp.w, &rect, TRUE );
			}
		}while(!feof(file));
		wsprintf( sp.StatusTx, TEXT("Completely  sent deu %d ms"), GetTickCount() - tc);
		InvalidateRect( sp.w, &rect, TRUE );
	}
	__finally
	{
//		if(AddrInfo)
//			freeaddrinfo(AddrInfo);

		if(file)
			fclose(file);
		if(s != INVALID_SOCKET)
		{
			shutdown(s, SD_BOTH);
			closesocket(s);
		}
		if(ss != INVALID_SOCKET)
			closesocket(ss);
		WSACleanup();
		((srv_param *)pv_param)->s = INVALID_SOCKET;
	}

	return 0;
}
