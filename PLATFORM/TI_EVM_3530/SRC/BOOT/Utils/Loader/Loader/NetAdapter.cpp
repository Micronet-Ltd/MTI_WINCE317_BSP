// Loader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// global variables
TCHAR  text[256];




NetAdapter::NetAdapter()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	
	wVersionRequested = MAKEWORD( 2, 2 );
 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
    /* Tell the user that we could not find a usable */
    /* WinSock DLL. 
     */
		wsprintf( text, __T("ERROR in WSAStartup %d \r\n"),err);
		PRINT(text);
		return;
    }

	::memset( &m_adapterInfo, 0, sizeof(m_adapterInfo));
	::memset( &m_remoteAddr, 0, sizeof(m_remoteAddr));
	::memset( &m_sourceAddr, 0, sizeof(m_sourceAddr));

	m_socket = INVALID_SOCKET;
	m_status = 0;
	m_remoteIPStr[0]='\0';
	m_portNo = 0;
	m_remoteAddrSize = 0;


	InitializeCriticalSection( &m_DeviceCritS );
}



NetAdapter::~NetAdapter()
{
	::memset( &m_adapterInfo, 0, sizeof(m_adapterInfo));
	if (m_socket!=INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
	// we need to wait all the network adaptors operations will complete
	EnterCriticalSection( &m_DeviceCritS );
	m_status = 0;
	WSACleanup();
	LeaveCriticalSection( &m_DeviceCritS );

	DeleteCriticalSection( &m_DeviceCritS );

}

BOOL NetAdapter::SetupConnection( IP_ADAPTER_INFO* pAdptInfo,u_short portNo)
{
	PIP_ADDR_STRING pAddressList;
	unsigned long sourceAddr;
	char szRemoteAddrString[128];
	sockaddr_in saSource;
	sockaddr_in ssRemoteAddr;
	int cbRemoteAddrSize, cbXfer, cbTotalRecvd;
	char pBuf[sizeof(tCmdHeader)];
	int err;
	USHORT ack;
	DWORD ret = 0;


	// setup the data members
	m_adapterInfo = *pAdptInfo;
	m_portNo = portNo;
	
	pAddressList = &(pAdptInfo->IpAddressList);

	do {
		wprintf(TEXT("\t IP Address ........ : %hs\n"),pAddressList->IpAddress.String);
		wprintf(TEXT("\t Subnet Mask ....... : %hs\n"), pAddressList->IpMask.String);
		sourceAddr = inet_addr(pAddressList->IpAddress.String);
		pAddressList = pAddressList->Next;
	 } while (pAddressList != NULL);

	
		
	m_socket = socket(AF_INET, SOCK_DGRAM, 0);

	// bind to the desktop source ip address of the rndis card
    if (m_socket != INVALID_SOCKET)
    {
		saSource.sin_family = AF_INET;
		saSource.sin_addr.s_addr = sourceAddr;
		saSource.sin_port = htons(m_portNo);


       if (bind(m_socket, (SOCKADDR *)&saSource, sizeof(saSource)) == SOCKET_ERROR)
		   closesocket(m_socket);
       else 
	   {

			if (getnameinfo((SOCKADDR *)&saSource, sizeof(saSource),
				  szRemoteAddrString, sizeof(szRemoteAddrString), NULL, 0, NI_NUMERICHOST) != 0)
				   szRemoteAddrString[0]='\0';
	

			wprintf(TEXT("SUCCESS - bind to the addresst %hs\r\n"), szRemoteAddrString);
			m_sourceAddr=saSource;
			m_status =0x01;
	   }
	}
	else
	{
		wprintf(TEXT("Failed to create socket %d\r\n"), WSAGetLastError());
		goto Cleanup;
	}

	

	do
	{
	//	cbTotalRecvd = 0;
		// do
		//{
			cbTotalRecvd = 0;
			cbRemoteAddrSize = sizeof(ssRemoteAddr);
			cbXfer = recvfrom(m_socket, pBuf + cbTotalRecvd, sizeof(pBuf) - cbTotalRecvd, 0,
				(SOCKADDR *)&ssRemoteAddr, &cbRemoteAddrSize);
			cbTotalRecvd += cbXfer;
	//	 } while(cbXfer > 0 && cbTotalRecvd < sizeof(pBuf));

		if(cbXfer == SOCKET_ERROR)
		{
			err = WSAGetLastError();
			wprintf(TEXT("ERROR: Couldn't receive the data! Error = %d\r\n"), err);
			goto Cleanup;
		}
		else if(cbXfer == 0)
		{
			wprintf(TEXT("ERROR: Didn't get all the expected data from the client!\r\n"));
			goto Cleanup;
		}

	    
		if (getnameinfo((SOCKADDR *)&ssRemoteAddr, cbRemoteAddrSize,
			szRemoteAddrString, sizeof(szRemoteAddrString), NULL, 0, NI_NUMERICHOST) != 0)
            szRemoteAddrString[0] ='\0';


		wprintf(TEXT("SUCCESS - Received %d bytes from client %hs\r\n"), cbTotalRecvd, szRemoteAddrString);

		//
		// check if connecting message had been received
		//

		if (CheckConnMsg(pBuf,cbTotalRecvd))
		{
        	//

			m_remoteAddr = ssRemoteAddr;
			m_remoteAddrSize = cbRemoteAddrSize;
			strcpy_s(m_remoteIPStr,IP_STRING_LENGTH,szRemoteAddrString);
			m_status = 0x02;

			char Buff[sizeof(tCmdHeader)];
			tCmdHeader *pCommandHeader = (tCmdHeader *)Buff;
			DWORD ReturnSize;

			pCommandHeader->ProtID =LDR_PROTOCOL_ID;
			pCommandHeader->SeqNum =0;
			pCommandHeader->Cmd	   =CMD_STOP_BOOTME;
			pCommandHeader->CmdAck =NO_ACK;

			ret = IOControl(SEND_CMD_ID, Buff, sizeof(tCmdHeader),&ack,sizeof(ack),&ReturnSize);

			//ret=SendCmd(CMD_STOP_BOOTME,&ack,1,NULL,0);
			if (ReturnSize == sizeof(ack) && ack ==ACK_NO_ERROR)
			{
				wprintf(TEXT("SendCmd Succeed ack = %d\r\n"), ack);
			}
			else
				m_status = 0x01;
		}

  } while(m_status!=0x02);

	return 1;

Cleanup:
	return 0;

}



DWORD NetAdapter::CheckConnMsg( char* Buff,DWORD bufLen)
{
	DWORD ret = 0;
	tCmdHeader *pCommandHeader = (tCmdHeader *)Buff;
	PRINT(__T("CLoader::CheckConnMsg. \r\n"));


	if (pCommandHeader->ProtID == LDR_PROTOCOL_ID && pCommandHeader->Cmd== CMD_SEND_BOOTME)
	{
		ret = 1;
	}
	
	return ret;
}



DWORD NetAdapter::IOControl( DWORD dwIoControlCode, LPVOID lpInBuf,DWORD nInBufSize,LPVOID lpOutBuf,
						DWORD nOutBufSize, LPDWORD lpBytesReturned)
{

	BOOL ret;
	EnterCriticalSection( &m_DeviceCritS );

	switch(dwIoControlCode)
	{
	case SEND_CMD_ID:
		ret = SendCmd(lpInBuf,nInBufSize,lpOutBuf,nOutBufSize,lpBytesReturned);
		break;

	case RECEIVE_CMD_ID:
		ret = ReceiveCmd(lpInBuf,nInBufSize,lpOutBuf,nOutBufSize,lpBytesReturned);
		break;
	};
	LeaveCriticalSection( &m_DeviceCritS );


	return ret;
}


DWORD NetAdapter::ReceiveCmd( LPVOID lpInBuf,DWORD nInBufSize,LPVOID lpOutBuf,DWORD nOutBufSize, LPDWORD lpBytesReturned)
{

	DWORD cbXfer, cbSent;
	tCmdHeader* pTxCommandHeader  = (tCmdHeader *)lpInBuf;
	tCmdHeader* pRxCommandHeader  = (tCmdHeader *)lpOutBuf;
	DWORD ret = FERROR_NET_FAILED;
	char szRemoteAddrString[128];


	*lpBytesReturned = 0;

	if (!lpInBuf || nInBufSize < sizeof(tCmdHeader) || nInBufSize > MAX_LDR_MSG_LENGTH ||!lpOutBuf || nOutBufSize > MAX_LDR_MSG_LENGTH )
	{
		wprintf(TEXT("Bad Parameters - %d\r\n"), nInBufSize);
		return FERROR_BAD_PARAMETERS;
	}

	// there is no adapter connected.
	if (m_status!=0x2 || m_socket == INVALID_SOCKET)
	{
		wprintf(TEXT("Failure - socket is not ready - %d\r\n"), m_status);
		return FERROR_NET_FAILED;
	}



	cbSent = sendto (m_socket, (char*)lpInBuf, nInBufSize, 0, (sockaddr*)&m_remoteAddr, sizeof(m_remoteAddr));

	if(cbSent == SOCKET_ERROR)
	{
		wprintf(TEXT("ERROR: Couldn't send the data! Error = %d\r\n"), WSAGetLastError());
		return FERROR_NET_FAILED;
	}

	// receive ack
	while( *lpBytesReturned < nOutBufSize )
	{
		cbXfer = recvfrom(m_socket, (char*)lpOutBuf + *lpBytesReturned, nOutBufSize - *lpBytesReturned, 0,(SOCKADDR *)&m_remoteAddr, &m_remoteAddrSize);
		
		if(cbXfer == SOCKET_ERROR)
		{
			//err = WSAGetLastError();
			wprintf(TEXT("ERROR: Couldn't receive the data! Error = %d\r\n"), WSAGetLastError());
			ret = FERROR_NET_FAILED;
			break;
		}
		
		if(cbXfer == 0)
		{
			wprintf(TEXT("ERROR: Didn't get all the expected data from the client!\r\n"));
			ret = FERROR_NET_FAILED;
			break;
		}

		*lpBytesReturned += cbXfer;

		if (*lpBytesReturned < nOutBufSize )//sizeof(tCmdHeader))
		{
			wprintf(TEXT("ERROR: The message size is wrong!\r\n"));
			continue;
		}

		ret = FOK;

		if (getnameinfo((SOCKADDR *)&m_remoteAddr, m_remoteAddrSize,
					szRemoteAddrString, sizeof(szRemoteAddrString), NULL, 0, NI_NUMERICHOST))
		{
			szRemoteAddrString[0] ='\0';
		}

		wprintf(TEXT("SUCCESS - Received %d bytes from client %hs\r\n"), cbXfer, szRemoteAddrString);
	}	

	return ret;
	
}



DWORD NetAdapter::SendCmd( LPVOID lpInBuf,DWORD nInBufSize,LPVOID lpOutBuf,DWORD nOutBufSize, LPDWORD lpBytesReturned)
{
	DWORD cbXfer,cbSent, cbTotalRecvd = 0;
	char  RxBuff[sizeof(tCmdHeader)];
	tCmdHeader* pTxCommandHeader  = (tCmdHeader *)lpInBuf;
	tCmdHeader* pRxCommandHeader;
	DWORD ret;
	char szRemoteAddrString[128];


	if (!lpInBuf || nInBufSize < sizeof(tCmdHeader) || nInBufSize > MAX_LDR_MSG_LENGTH  || !lpOutBuf || nOutBufSize < sizeof(USHORT))
	{
		wprintf(TEXT("Bad Parameters - %d\r\n"), nInBufSize);
		return FERROR_BAD_PARAMETERS;
	}

	// there is no adapter connected.
	if (m_status!=0x2 || m_socket == INVALID_SOCKET)
	{
		wprintf(TEXT("Failure - socket is not ready - %d\r\n"), m_status);
		return FERROR_NET_FAILED;
	}



	cbSent = sendto (m_socket, (char*)lpInBuf,nInBufSize, 0, (sockaddr*)&m_remoteAddr, sizeof(m_remoteAddr));

	if(cbSent == SOCKET_ERROR)
	{
		wprintf(TEXT("ERROR: Couldn't send the data! Error = %d\r\n"), WSAGetLastError());
		return FERROR_NET_FAILED;
	}
	
	ret = FOK;
	// receive ack
	while(sizeof(RxBuff) > cbTotalRecvd )
	{
		cbXfer = recvfrom(m_socket, RxBuff + cbTotalRecvd, sizeof(RxBuff) - cbTotalRecvd, 0,(SOCKADDR *)&m_remoteAddr, &m_remoteAddrSize);

		if(cbXfer == SOCKET_ERROR)
		{
			//err = WSAGetLastError();
			wprintf(TEXT("ERROR: Couldn't receive the data! Error = %d\r\n"), WSAGetLastError());
			ret = FERROR_NET_FAILED;
			break;
		}
		if(cbXfer == 0)
		{
			wprintf(TEXT("ERROR: Didn't get all the expected data from the client!\r\n"));
			ret = FERROR_NET_FAILED;
			break;
		}

		cbTotalRecvd += cbXfer;
	}



	if (getnameinfo((SOCKADDR *)&m_remoteAddr, m_remoteAddrSize,
		szRemoteAddrString, sizeof(szRemoteAddrString), NULL, 0, NI_NUMERICHOST) != 0)
           szRemoteAddrString[0] = '\0';


	wprintf(TEXT("Received %d bytes from client %hs\r\n"), cbTotalRecvd, szRemoteAddrString);

	pRxCommandHeader = (tCmdHeader *)RxBuff;
	
	*((USHORT*)lpOutBuf) = pRxCommandHeader->CmdAck;
	*lpBytesReturned = sizeof (USHORT);

	return ret;	
}



char* NetAdapter::getRemoteIPString()
{
	return m_remoteIPStr;
}

BOOL NetAdapter::isConnected()
{
	return (m_status == 0x02)?TRUE:FALSE;
}

BOOL NetAdapter::CloseConnection()
{
	if (m_socket!=INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	// we need to wait all the network adaptors operations will complete
	EnterCriticalSection( &m_DeviceCritS );
	m_status = 0;
	LeaveCriticalSection( &m_DeviceCritS );

	return TRUE;
}
//DWORD NetAdapter::Receive( LPVOID lpOutBuf,DWORD nOutBufSize, LPDWORD lpBytesReturned)
//{
//
//	DWORD cbXfer;
//	DWORD err;
//	char szRemoteAddrString[128];
//
//
//	if(!lpOutBuf || !nOutBufSize || nOutBufSize > MAX_LDR_MSG_LENGTH )
//	{
//		wprintf(TEXT("Bad Parameters - %s\r\n"), ( lpOutBuf ? _T("nOutBufSize") : _T("lpOutBuf") ));
//		return FERROR_BAD_PARAMETERS;
//	}
//
//	// there is no adapter connected.
//	if (m_status!=0x2 || m_socket == INVALID_SOCKET)
//	{
//		wprintf(TEXT("Failure - socket is not ready - %d\r\n"), m_status);
//		return FERROR_NET_FAILED;
//	}
//
//	cbXfer = recvfrom(m_socket, (char*)lpOutBuf, nOutBufSize, 0,(SOCKADDR *)&m_remoteAddr, &m_remoteAddrSize);
//
//	if(cbXfer == SOCKET_ERROR)
//	{
//		err = WSAGetLastError();
//		wprintf(TEXT("ERROR: Couldn't receive the data! Error = %d\r\n"), err);
//		return FERROR_NET_FAILED;
//	}
//	
//	if(cbXfer == 0)
//	{
//		wprintf(TEXT("ERROR: Didn't get all the expected data from the client!\r\n"));
//		return  FERROR_NET_FAILED;
//	}
//
//	//if (cbXfer < sizeof(tCmdHeader))
//	//{
//	//	wprintf(TEXT("ERROR: The message size is wrong!\r\n"));
//	//	continue;
//	//}
//	    
//	if (getnameinfo((SOCKADDR *)&m_remoteAddr, m_remoteAddrSize,
//		szRemoteAddrString, sizeof(szRemoteAddrString), NULL, 0, NI_NUMERICHOST) != 0)
//	{
//		szRemoteAddrString[0] ='\0';
//	}
//
//	wprintf(TEXT("SUCCESS - Received %d bytes from client %hs\r\n"), cbXfer, szRemoteAddrString);
//
//		
////	}while ( pRxCommandHeader->Cmd != pTxCommandHeader->Cmd );
//
//	*lpBytesReturned = cbXfer;
//
//	return FOK;
//}
//
//
