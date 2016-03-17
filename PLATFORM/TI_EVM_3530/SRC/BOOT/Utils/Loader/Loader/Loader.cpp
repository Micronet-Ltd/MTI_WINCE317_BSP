// Loader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Iphlpapi.h>
#include "Loader.h"
#include "LoaderAPI.h"


#define     CONNECT_THREAD_CLOSE_TIMEOUT  5000



//------------------------------------------------------
//               CLoader class definition
//------------------------------------------------------

CLoader::CLoader()
{

	m_pfnProgressCallback = NULL;

	m_pAdapters =NULL;
    m_nAdaptersCount =0;
	m_ConnectThread = NULL;
	m_QuitConnEvent = NULL;

	InitializeCriticalSection( &m_AdapterCritS );

}

CLoader::~CLoader()
{

	m_pfnProgressCallback = NULL;

	CloseConnection();

	DeleteCriticalSection( &m_AdapterCritS );

	if(m_pAdapters)
	{
	 delete[]m_pAdapters;
     m_pAdapters=NULL;
	}

    m_nAdaptersCount =0;

}




/*-----------------------------------------------------------------------------------------
Function Name:         LoaderGetVersion

Description:           This function gets the given component version. 
                       The component is one of the following: IMG/EBOOT/XLDR according 
					   to the type argument. The version information includes component major,
					   minor and incremental version and it build date.

Parameters:           OUT: *  pVersionInfo - Pointer to the version information info.
                      It includes major,minor,incremental component version and its build date.
                      IN: type (VERSION_XLDR=0, VERSION_EBOOT, VERSION_IMG)  - what component version we want to get                            

                      IN. deviceIP – The IP address of the device to send the command to. 
					  At the first stage this argument is not relevant and even may be set to NULL.

Return Values:  FOK,  FERROR_VERSION_NOT_FOUND FERROR_IMAGE_NOT_BURNED FERROR_NETFAILURE,
                     FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,FERROR_NOTCONNECTED.


-----------------------------------------------------------------------------------------*/
int CLoader::LoaderGetVersionExt(VersionInfoExt *pVersionInfo,VersionType type,char* deviceIP)
{
	PRINT(__T("CLoader::LoaderGetVersion \r\n"));
					

	char TxBuff[sizeof(tCmdHeader)];
	char RxBuff [sizeof(tCmdHeader)+ sizeof(VersionInfoExt) -2];
	tCmdHeader *pTxCommandHeader = (tCmdHeader *)TxBuff;
	tCmdHeader *pRxCommandHeader = (tCmdHeader *)RxBuff;
	
	ULONG ReturnSize = 0;
	int Status = FOK;

	EnterCriticalSection( &m_AdapterCritS );
	
/*
	if (deviceIP!=NULL)
	{
		// for future use
		// I will choose the right adapter according to this string
		PRINT(__T("CLoader::LoaderGetVersion: not supported parameter \r\n "));
		Status = FERROR_NOT_SUPPORTED;
		goto Cleanup;
	}
*/
	if(!m_pAdapters)
	{
		PRINT(__T("CLoader::LoaderGetVersion: not called setup first.\r\n "));

		Status = FERROR_NOTOPENED;
		goto Cleanup;
	}

	// currently implementation is for only 1 adapter at the time.
	if (!m_pAdapters[0].isConnected())
	{
        PRINT(__T("CLoader::LoaderGetVersion: adapter is not connected yet.\r\n "));
		Status = FERROR_NOTCONNECTED;
		goto Cleanup;
	}

	pTxCommandHeader->ProtID =  LDR_PROTOCOL_ID;
	pTxCommandHeader->SeqNum =  0;
	pTxCommandHeader->Cmd	   =CMD_GET_VERSION_EXT;
	pTxCommandHeader->CmdAck =  NO_ACK;
	*((UINT16*)pTxCommandHeader->Data)   = (UINT16)type;

	if(m_pAdapters[0].IOControl(RECEIVE_CMD_ID, TxBuff, sizeof(tCmdHeader),RxBuff,sizeof(RxBuff),&ReturnSize))
	{
		wsprintf( text, __T("ERROR: CLoader::LoaderGetVersion fail in IoControl...%d \r\n"),GetLastError() );
		PRINT(text);

		Status = FERROR_NETFAILURE;
	}
	else
	{

		if (pRxCommandHeader->CmdAck == ACK_NO_ERROR && ReturnSize == sizeof(RxBuff))
		{
			memcpy(pVersionInfo,pRxCommandHeader->Data,sizeof(VersionInfoExt));
			Status = FOK;
		}
		else 
		{
			memset(pVersionInfo,0,sizeof(VersionInfoExt));
			Status = (pRxCommandHeader->CmdAck ==ACK_IMAGE_NOT_BURNED)? FERROR_IMAGE_NOT_BURNED:FERROR_VERSION_NOT_FOUND;
		}
	}
		
Cleanup:
	
	LeaveCriticalSection( &m_AdapterCritS );

	PRINT( __T("CLoader::LoaderGetVersion \r\n"));

	return Status;
}


int CLoader::LoaderGetVersion(VersionInfo *pVersionInfo,VersionType type,char* deviceIP)
{
	PRINT(__T("CLoader::LoaderGetVersion \r\n"));
					

	char TxBuff[sizeof(tCmdHeader)];
	char RxBuff [sizeof(tCmdHeader)+ sizeof(VersionInfo) -2];
	tCmdHeader *pTxCommandHeader = (tCmdHeader *)TxBuff;
	tCmdHeader *pRxCommandHeader = (tCmdHeader *)RxBuff;
	
	ULONG ReturnSize = 0;
	int Status = FOK;

	EnterCriticalSection( &m_AdapterCritS );
	
/*
	if (deviceIP!=NULL)
	{
		// for future use
		// I will choose the right adapter according to this string
		PRINT(__T("CLoader::LoaderGetVersion: not supported parameter \r\n "));
		Status = FERROR_NOT_SUPPORTED;
		goto Cleanup;
	}
*/
	if(!m_pAdapters)
	{
		PRINT(__T("CLoader::LoaderGetVersion: not called setup first.\r\n "));

		Status = FERROR_NOTOPENED;
		goto Cleanup;
	}

	// currently implementation is for only 1 adapter at the time.
	if (!m_pAdapters[0].isConnected())
	{
        PRINT(__T("CLoader::LoaderGetVersion: adapter is not connected yet.\r\n "));
		Status = FERROR_NOTCONNECTED;
		goto Cleanup;
	}

	/*
	memset(pVersionInfo,0,sizeof(SVersionInfo) );
	
	if((void *)-1 == m_DriverHandle)
	{
		if((Status = USBLoaderSetup(true,0)) != FOK)
		{
			return Status;
		}
	}
	*/

	
	pTxCommandHeader->ProtID =  LDR_PROTOCOL_ID;
	pTxCommandHeader->SeqNum =  0;
	pTxCommandHeader->Cmd	   =CMD_GET_VERSION;
	pTxCommandHeader->CmdAck =  NO_ACK;
	*((UINT16*)pTxCommandHeader->Data)   = (UINT16)type;

	if(m_pAdapters[0].IOControl(RECEIVE_CMD_ID, TxBuff, sizeof(tCmdHeader),RxBuff,sizeof(RxBuff),&ReturnSize))
	{
		wsprintf( text, __T("ERROR: CLoader::LoaderGetVersion fail in IoControl...%d \r\n"),GetLastError() );
		PRINT(text);

		Status = FERROR_NETFAILURE;
	}
	else
	{

		if (pRxCommandHeader->CmdAck == ACK_NO_ERROR && ReturnSize == sizeof(RxBuff))
		{
			memcpy(pVersionInfo,pRxCommandHeader->Data,sizeof(VersionInfo));
			Status = FOK;
		}
		else 
		{
			memset(pVersionInfo,0,sizeof(VersionInfo));
			Status = (pRxCommandHeader->CmdAck ==ACK_IMAGE_NOT_BURNED)? FERROR_IMAGE_NOT_BURNED:FERROR_VERSION_NOT_FOUND;
		}
	}
		
Cleanup:
	
	LeaveCriticalSection( &m_AdapterCritS );

	PRINT( __T("CLoader::LoaderGetVersion \r\n"));

	return Status;
}



/*-----------------------------------------------------------------------------------------
Function Name:         LoaderProgram

Description:           This function downloads the given bin file into the device RAM 
                       over the BOOT Interface communication layer and then burns it 
					   into the flash. The bin file may be one of the following: 
					   nk.bin/xldr.bin/eboot.bin

Parameters:           IN. binFile -  the file to be burned into the flash
                      IN. ProgressCallback – this function is called during the 
					  download process (each succesfull /unsuccesfull loader packet) 
					  and once when burning process is completed.
                      IN. deviceIP – The IP address of the device to send the cmd to.
					  At the first stage this argument is not relevant and even may be set to NULL.

Return Values:        FOK, FERROR_BADBINFILE, FERROR_FLASHERROR, FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,
                      FERROR_NOTOPENED,FERROR_NOTCONNECTED




-----------------------------------------------------------------------------------------*/

int CLoader::LoaderProgram(const char* binFile, ProgressCallback pfnProgressCallback,char* deviceIP)
{
	PRINT(__T("CLoader::LoaderProgram started \r\n"));
	
	char Buff[MAX_LDR_MSG_LENGTH];
	tCmdHeader *pCommandHeader = (tCmdHeader *)Buff;
	DWORD CurrentDataLen;
	DWORD TotalFileLength,CurrentReadenFileLen = 0;
	SProgressCallbackParams ProgressCallbackParams;
	ULONG ReturnSize = 0;
	int Status = FOK;

/*
	if (deviceIP!=NULL)
	{
		// for future use
		// I will choose the right adapter according to this string
		PRINT(__T("CLoader::LoaderProgram: not supported parameter \r\n "));
		Status = FERROR_NOT_SUPPORTED;
		goto Cleanup;
	}
*/


	EnterCriticalSection( &m_AdapterCritS );

	if(!m_pAdapters)
	{
		PRINT(__T("CLoader::LoaderProgram: not called setup first \r\n "));

		Status = FERROR_NOTOPENED;
		goto Cleanup;
	}

	// currently implementation is for only 1 adapter at the time.
	if (!m_pAdapters[0].isConnected())
	{

		PRINT(__T("CLoader::LoaderProgram: adapter is not connected yet. \r\n "));

		Status = FERROR_NOTCONNECTED;
		goto Cleanup;
	}



	//Open the file for getting the file size
	WCHAR* wBinFile = (WCHAR*)LocalAlloc (LPTR,sizeof(WCHAR)*(strlen(binFile)+1));
	mbstowcs(wBinFile,binFile,strlen(binFile));
	wBinFile[strlen(binFile)] = L'\0';

  //  m_pfnProgressCallback = pfnProgressCallback;
	HANDLE FileHandle = CreateFile(wBinFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
	if(FileHandle == INVALID_HANDLE_VALUE)
	{
		//Can not open the file 
		if(pfnProgressCallback)
		{
			//Send notification to the callback routine
			strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
            ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
			ProgressCallbackParams.methodId = PCID_DownLoad;
			ProgressCallbackParams.reason = PCRS_SetupFailed;
			ProgressCallbackParams.position =0;
			ProgressCallbackParams.overallSize =0;

			pfnProgressCallback(&ProgressCallbackParams);
		}

		wsprintf( text, __T("ERROR: LoaderProgram fail in create file...%d \r\n"),GetLastError() );
		PRINT(text);

		Status = FERROR_BADBINFILE;
		goto Cleanup;

	}
	//Get The Total file length
	TotalFileLength =  GetFileSize(FileHandle,NULL);
	CloseHandle(FileHandle);
	LocalFree (wBinFile);

	
	short Ack;
	//Read all the file and send it for burning to the device.
	FILE *Fop = fopen(binFile,"rb");
	if(Fop)
	{
		Ack = 0;

	    pCommandHeader->ProtID =LDR_PROTOCOL_ID;
		pCommandHeader->SeqNum =0;
		pCommandHeader->Cmd	   =CMD_BIN_DOWNLOAD;
		pCommandHeader->CmdAck =NO_ACK;

		while((CurrentDataLen = fread(Buff + sizeof(tCmdHeader)- 2, sizeof(char), sizeof(Buff) - sizeof(tCmdHeader)+2, Fop)))
		{	
			
			pCommandHeader->SeqNum ++;

			
			if(m_pAdapters[0].IOControl(SEND_CMD_ID, Buff, CurrentDataLen + sizeof(tCmdHeader)-2,&Ack,sizeof(Ack),&ReturnSize))
			{
				//The IOCTL Failed....
				if(pfnProgressCallback)
				{
					//Send notification to the callback routine
					strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
					 ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
					ProgressCallbackParams.methodId = PCID_DownLoad;
					ProgressCallbackParams.reason = PCRS_SetupFailed;
					ProgressCallbackParams.position =0;
			        ProgressCallbackParams.overallSize =0;
					pfnProgressCallback(&ProgressCallbackParams);
				}	
				//LOG(Log,debug_level::error)<<"USBLoaderProgram fail in DeviceIoControl "<<GetLastError()<<GOL;
				fclose(Fop);
				Status = FERROR_NETFAILURE;
				goto Cleanup;

			}
			else
			{
				//The IOCTL Success....
				if(ReturnSize == sizeof(Ack) && Ack == ACK_NO_ERROR)
				{
					CurrentReadenFileLen +=CurrentDataLen;
					//Addrress += CurrentDataLen;
					if(pfnProgressCallback)
					{
						//Send notification to the callback routine
						strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
						ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
						ProgressCallbackParams.methodId = PCID_DownLoad;
						ProgressCallbackParams.reason = PCRS_Progress;
						ProgressCallbackParams.overallSize = TotalFileLength;
						ProgressCallbackParams.position = CurrentReadenFileLen;
						pfnProgressCallback(&ProgressCallbackParams);
					}
				}
				else
				{
					//the device returns Wrong Ack.
					if(pfnProgressCallback)
					{
						strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
						ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
						ProgressCallbackParams.methodId = PCID_DownLoad;
						ProgressCallbackParams.reason = PCRS_SetupFailed;
						ProgressCallbackParams.position =0;
			            ProgressCallbackParams.overallSize =0;
						pfnProgressCallback(&ProgressCallbackParams);
					}	
					//char tt[20];
					//sprintf(tt," 0x%x ",Addrress);

					//LOG(Log,debug_level::note_ex)<<"IOCTL_SEND_COMMAND:"<<CurrentDataLen<<GOL;
			
					//LOG(Log,debug_level::error)<<"USBLoaderProgram return wrong Ack from the device "<<Ack<<GOL;
					fclose(Fop);

					Status = FERROR_NETFAILURE;
					goto Cleanup;
	
				}
			}	
		}
		fclose(Fop);
		if(TotalFileLength != CurrentReadenFileLen)
		{
			//The loop finshed before the end of the file.
			if(pfnProgressCallback)
			{
				strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
				ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
				ProgressCallbackParams.methodId = PCID_DownLoad;
				ProgressCallbackParams.reason = PCRS_SetupFailed;
				ProgressCallbackParams.position =0;
			    ProgressCallbackParams.overallSize =0;
				pfnProgressCallback(&ProgressCallbackParams);
			}	
			//LOG(Log,debug_level::error)<<"USBLoaderProgram fail in fread before the end of the file "<<GetLastError()<<GOL;
			Status = FERROR_BADBINFILE;
		}
		else
		{

			// need to burn it now

			pCommandHeader->ProtID =LDR_PROTOCOL_ID;
			pCommandHeader->SeqNum =1;
			pCommandHeader->Cmd	   =CMD_FLASH_BIN;
			pCommandHeader->CmdAck =NO_ACK;

			
			
			if(m_pAdapters[0].IOControl(SEND_CMD_ID,Buff,sizeof(tCmdHeader),&Ack,sizeof(Ack),&ReturnSize))
			{
				//The IOCTL Failed....
				if(pfnProgressCallback)
				{
					//Send notification to the callback routine
					strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
					ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
					ProgressCallbackParams.methodId = PCID_Burn;
					ProgressCallbackParams.reason = PCRS_SetupFailed;
					ProgressCallbackParams.position =0;
			        ProgressCallbackParams.overallSize =0;
					pfnProgressCallback(&ProgressCallbackParams);
				}	
				//LOG(Log,debug_level::error)<<"USBLoaderProgram fail in DeviceIoControl "<<GetLastError()<<GOL;

				Status = FERROR_NETFAILURE;
				goto Cleanup;
			}

			//
			if(ReturnSize == sizeof(Ack) && Ack == ACK_NO_ERROR)
			{
					//Finish to burn the new Image
					if(pfnProgressCallback)
					{
						//Send notification to the callback routine
						strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
						ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
						ProgressCallbackParams.methodId = PCID_Burn;
						ProgressCallbackParams.reason = PCRS_SetupComplete;
						ProgressCallbackParams.position =0;
			            ProgressCallbackParams.overallSize =0;
						pfnProgressCallback(&ProgressCallbackParams);
					}
			}
			else
			{
					//the device returns Wrong Ack.
					if(pfnProgressCallback)
					{
						strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
						ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
						ProgressCallbackParams.methodId = PCID_Burn;
						ProgressCallbackParams.reason = PCRS_SetupFailed;
						ProgressCallbackParams.position =0;
			            ProgressCallbackParams.overallSize =0;
						pfnProgressCallback(&ProgressCallbackParams);
					}	
					

					//LOG(Log,debug_level::note_ex)<<"IOCTL_SEND_COMMAND:"<<CurrentDataLen<<GOL;
			
					//LOG(Log,debug_level::error)<<"LoaderProgram return wrong Ack from the device "<<Ack<<GOL;

					Status = (Ack==ACK_ERROR_FLASH)? FERROR_FLASHERROR:FERROR_NETFAILURE;
					goto Cleanup;

			}
			
		}
	}
	else
	{
		//Cannot open the file 
		if(pfnProgressCallback)
		{
			//Send notification to the callback routine
			strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
			ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
			ProgressCallbackParams.methodId = PCID_DownLoad;
			ProgressCallbackParams.reason = PCRS_SetupFailed;
			ProgressCallbackParams.position =0;
			ProgressCallbackParams.overallSize =0;
			pfnProgressCallback(&ProgressCallbackParams);
		}
		//LOG(Log,debug_level::error)<<"LoaderProgram fail in create file "<<GetLastError()<<GOL;

		Status = FERROR_BADBINFILE;;
		goto Cleanup;
	}

Cleanup:
	LeaveCriticalSection( &m_AdapterCritS );

	//LOG(Log,debug_level::note_ex)<<"Exit LoaderProgram Status "<<Status<<GOL;
	return Status;
}



/////////////////////////////////////////////////////////
//	Desc:
//		Used to build the list of adapters.  Must be 
//		called again after each call that modifies 
//		the adapters information.  Currently this includes
//		the renewing and releasing of adapter information.
/////////////////////////////////////////////////////////
BOOL CLoader::BuildAdapterListing() {
	BOOL	bRet		= TRUE;
	DWORD	dwErr		= 0;
	ULONG	ulNeeded	= 0;

	SProgressCallbackParams ProgressCallbackParams;

	BOOL ret = TRUE;

	if (m_pAdapters)
		m_pAdapters[0].CloseConnection();

	EnterCriticalSection( &m_AdapterCritS );
	if (m_pAdapters)
	{
		//connection disconnected
		if(m_pfnProgressCallback)
		{
			//Send notification to the callback routine
			strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
			ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
			ProgressCallbackParams.methodId = PCID_Setup;
			ProgressCallbackParams.reason = PCRS_SetupFailed;
			ProgressCallbackParams.position =0;
	        ProgressCallbackParams.overallSize =0;
			m_pfnProgressCallback(&ProgressCallbackParams);
		}	
		delete []m_pAdapters;
		//LocalFree(m_pAdapters);
		m_pAdapters = NULL;
	}

	dwErr = EnumNetworkAdapters( m_pAdapters, 0, &ulNeeded );

	LeaveCriticalSection( &m_AdapterCritS );

	if( dwErr == ERROR_INSUFFICIENT_BUFFER ) {		
		m_nAdaptersCount	= ulNeeded / sizeof( NetAdapter );
		//m_pAdapters = new NetAdapter[ ulNeeded / sizeof(CLoader) ];
        m_pAdapters = new NetAdapter[ ulNeeded / sizeof(NetAdapter) ];	

		dwErr		= EnumNetworkAdapters( m_pAdapters, ulNeeded, &ulNeeded );
		if (dwErr)
			ret = FALSE;
		else if( ! m_pAdapters ) {
			PRINT( _T("No Network Adapters Found on System."));
			bRet = FALSE;
		}		
	}else{
		PRINT( _T("No Network Adapters Found on System."));
		bRet = FALSE;
	}

	return bRet;
}


////////////////////////////////////////////////////////////
//	Desc:
//		Builds a listing of all network adapters found
//		on the current system.  If there is not enough
//		space in the passed in buffer ERROR_INSUFFICIENT_BUFFER
//		is returned. If error lpdwOutSzBuf contains the
//		needed buffer size.  If no error it returned
//		used buffer size.
////////////////////////////////////////////////////////////
int CLoader::EnumNetworkAdapters( NetAdapter* pAdapters, ULONG ulSzBuf, LPDWORD lpdwOutSzBuf ) {	

	IP_ADAPTER_INFO* pAdptInfo	= NULL;
	IP_ADAPTER_INFO* pNextAd	= NULL;	
	ULONG ulLen					= 0;
	int nCnt					= 0;
	DWORD erradapt;
	SProgressCallbackParams ProgressCallbackParams;
	DWORD status = FOK;

	erradapt = ::GetAdaptersInfo( pAdptInfo, &ulLen );
	if( erradapt == ERROR_BUFFER_OVERFLOW ) {
		//pAdptInfo = ( IP_ADAPTER_INFO* )malloc( ulLen );
		 pAdptInfo = ( IP_ADAPTER_INFO* )LocalAlloc (LPTR,ulLen);

		erradapt = ::GetAdaptersInfo( pAdptInfo, &ulLen );		
	}

		// acquire a count of how many network adapters
		// we are going to need to work on so that
		// we can make sure we have adequate buffer space.
	pNextAd = pAdptInfo;
	while( pNextAd ) {
		// rndis related cards mac address start with 02.
		if (pNextAd->Address[0]!=0x02)
		{
			pNextAd = pNextAd->Next;
			continue;
		}
		nCnt++;
		pNextAd = pNextAd->Next;
	}

		// now that we have the count we need to determine if the caller passed in an
		// adequate buffer for us to store all the needed adapters.  we set the variable
		// to the needed number of bytes so the caller will now how many there are 
		// and how much space is needed to hold them.
	*lpdwOutSzBuf = nCnt * sizeof( NetAdapter);
	if( ulSzBuf < *lpdwOutSzBuf ) {		
		//delete( pAdptInfo );
		LocalFree(pAdptInfo);
		return ERROR_INSUFFICIENT_BUFFER;
	}

		// this needs to be reset for future use.
	nCnt = 0;

	if( erradapt == ERROR_SUCCESS ) {
			// initialize the pointer we use the move through
			// the list.
		pNextAd = pAdptInfo;

			// loop through for all available interfaces and setup an associated
			// CNetworkAdapter class.
		while( pNextAd ) {
			// rndis related cards mac address start with 02.
			if (pNextAd->Address[0]!=0x02)
			{
				pNextAd = pNextAd->Next;
				continue;
			}
			if (pAdapters[nCnt].SetupConnection( pNextAd ,DEFAULT_PORT) == TRUE)
			{

				//Setup succeeded
				if(m_pfnProgressCallback)
				{
					//Send notification to the callback routine
					strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
					ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
					ProgressCallbackParams.methodId = PCID_Setup;
					ProgressCallbackParams.reason = PCRS_SetupComplete;
					ProgressCallbackParams.position =0;
			        ProgressCallbackParams.overallSize =0;
					m_pfnProgressCallback(&ProgressCallbackParams);
				}	
			}
			else
			{
				//Setup failed
				if(m_pfnProgressCallback)
				{
					//Send notification to the callback routine
					strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
					ProgressCallbackParams.methodId = PCID_Setup;
					ProgressCallbackParams.reason = PCRS_SetupFailed;
					ProgressCallbackParams.position =0;
			        ProgressCallbackParams.overallSize =0;
					m_pfnProgressCallback(&ProgressCallbackParams);
					status = FERROR_NET_FAILED;
				}	
			}
				// move forward to the next adapter in the list so
				// that we can collect its information.
			pNextAd = pNextAd->Next;
			nCnt++;
		}
	}

		// free any memory we allocated from the heap before
		// exit. 
	//delete( pAdptInfo );	
	LocalFree(pAdptInfo);

	return status;
}


/* -----------------------------------------------------------------------------------------

Description:           This function startups the server. Its asynchronous function and it
                       returns immediately.  It starts the connection thread which is 
					   waiting for clients connection messages. When connection is established, 
					   the user callback function is called and the IP address of the connected 
					   client is passed as one of the callback function parameter. 

Parameters:            The user callback function which will be called when connection 
                       is established or broken.

Return Values:          FOK, FERROR_ALREADYOPEN, FERROR_SETUPFAILURE

-----------------------------------------------------------------------------------------*/

int CLoader::SetupConnection(ProgressCallback pfnProgressCallback)
{
	if(m_ConnectThread ||m_QuitConnEvent)
	{
		//LOG(Log,debug_level::error)<<"The Session is still opened"<<GOL;
		return FERROR_ALREADYOPEN;

	}

	m_pfnProgressCallback = pfnProgressCallback;
	m_QuitConnEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (!m_QuitConnEvent )
	{
		//LOG(Log,debug_level::error)<<"Couldn't create quit event"<<GOL;
		return FERROR_SETUPFAILURE;
	}


	m_ConnectThread = ::CreateThread(0, 0, DoConnect, (LPVOID)this, 0, 0 );
	if(!m_ConnectThread)
	{
		CloseHandle(m_QuitConnEvent);
        m_QuitConnEvent = NULL;
		//LOG(Log,debug_level::error)<<"Couldn't create quit event"<<GOL;
		return FERROR_SETUPFAILURE;
	}

	return FOK;
}


/*-----------------------------------------------------------
Function Name:         DoConnect

Description:           Its the connection thread 
-----------------------------------------------------------
*/
DWORD WINAPI CLoader::DoConnect(LPVOID lpParam)
{
	OVERLAPPED overlap;
	DWORD ret = 0;
	
	//HANDLE hand = ::WSACreateEvent();
	HANDLE hand = 0;
	overlap.hEvent = ::WSACreateEvent();

	HANDLE 	hEventHandles[2];
	DWORD   dwRetVal;

	CLoader *This = reinterpret_cast<CLoader *>( lpParam );


	hEventHandles[0] = This->m_QuitConnEvent;
    hEventHandles[1] = overlap.hEvent;


	do
	{
		  PRINT(__T("This->BuildAdapterListing() started ...\r\n"));

		  This->BuildAdapterListing();

		   PRINT(__T("This->BuildAdapterListing() ended ...\r\n"));

		  ret = ::NotifyAddrChange(&hand, &overlap);

		  if (ret != NO_ERROR)
		  {
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				wsprintf( text, __T("ERROR: NotifyAddrChange error...%d \r\n"),WSAGetLastError() );
				PRINT(text);

			  ret = 1;
			  break;
			}
		  }

		  dwRetVal = ::WaitForMultipleObjects(2, hEventHandles, FALSE, INFINITE);
		  
		  if(1 != (dwRetVal - WAIT_OBJECT_0))
		  {
			  PRINT(__T("event signalled to quit conn thread  ...\r\n"));
			  break;
		  }

		  PRINT(__T("IP Address table changed ...\r\n"));

		 
	}while(1);


//	::WSACloseEvent(hand);
	::WSACloseEvent(overlap.hEvent);

	hEventHandles[0] = 0;
	hEventHandles[1] = 0;

	 PRINT(__T("Connection thread exited ...\r\n"));

 return ret;
}


/* ------------------------------------------------------------------------------------
Function Name:         CloseConnection

Description:           This function closes all the connections and terminates server's 
                       communication thread. Its synchronous function and it returns when 
					   connection is closed and thread is terminated.  

Parameters:             None

Return Values:          FOK, FERROR_GENERAL;

------------------------------------------------------------------------------------*/

BOOL  CLoader::CloseConnection()
{
	DWORD dwReason;
	if (m_pAdapters)
		m_pAdapters[0].CloseConnection();


	// terminate connection thread and clean up
	if (m_ConnectThread  && m_QuitConnEvent)
	{
		SetEvent(m_QuitConnEvent);
		dwReason = WaitForSingleObject(m_ConnectThread, CONNECT_THREAD_CLOSE_TIMEOUT);
		if( dwReason != WAIT_OBJECT_0 )
		{
			TerminateThread(m_ConnectThread, 0);
			//LOG(Log,debug_level::error)<<"CLoader::CloseConnection dwReason:"<<dwReason<<GOL;
		}
		CloseHandle(m_ConnectThread);
		CloseHandle(m_QuitConnEvent);
		m_ConnectThread = NULL;
		m_QuitConnEvent = NULL;
	}
	
	return TRUE;

}

/* ------------------------------------------------------------------------------------------------------------
Function Name:         LoaderGetParamBlock

Description:           This function reads the Manufacture Parameters Block from flash memory. 
                       The Manufacture parameters block resides in the second last sector 
					   of the last boot block.

Parameters:           OUT: *prm – pointer to the parameter block to get the data.
                      IN. deviceIP – The IP address of the device to send the 
					  command to. At the first stage this argument is not relevant and even 
					  may be set to NULL.

Return Values:        FOK, FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,
                      FERROR_NOTCONNECTED, FERROR_FLASHERROR


------------------------------------------------------------------------------------------------------------*/
int CLoader::LoaderGetParamBlock(manufacture_block *prm,char* deviceIP)
{
	char TxBuff[sizeof(tCmdHeader)];
	char RxBuff [sizeof(tCmdHeader)+ sizeof(manufacture_block) -2];
	tCmdHeader *pTxCommandHeader = (tCmdHeader *)TxBuff;
	tCmdHeader *pRxCommandHeader = (tCmdHeader *)RxBuff;
	
	ULONG ReturnSize = 0;
	int Status = FOK;


/*
	if (deviceIP!=NULL)
	{
		// for future use
		// I will choose the right adapter according to this string
		PRINT(__T("CLoader::LoaderGetParamBlock: not supported parameter \r\n "));
		Status = FERROR_NOT_SUPPORTED;
		goto Cleanup;
	}
*/


	EnterCriticalSection( &m_AdapterCritS );

	if(!m_pAdapters)
	{
		PRINT(__T("LoaderGetParamBlock::LoaderGetParamBlock not called setup first\r\n"));
		Status= FERROR_NOTOPENED;
		goto Cleanup;

	}

	// currently implementation is for only 1 adapter at the time.
	if (!m_pAdapters[0].isConnected())
	{
		PRINT(__T("LoaderGetParamBlock::LoaderGetParamBlock adapter is not connected yet.t\r\n"));
		Status= FERROR_NOTCONNECTED;
		goto Cleanup;

	}

	
	
	pTxCommandHeader->ProtID =  LDR_PROTOCOL_ID;
	pTxCommandHeader->SeqNum =  0;
	pTxCommandHeader->Cmd	   =CMD_GET_MANUFACTURE;
	pTxCommandHeader->CmdAck =  NO_ACK;

	if(m_pAdapters[0].IOControl(RECEIVE_CMD_ID, TxBuff, sizeof(tCmdHeader),RxBuff,sizeof(RxBuff),&ReturnSize))
	{

		wsprintf( text, __T("ERROR: CLoader::LoaderGetParamBlock fail in IoControl...%d \r\n"),GetLastError() );
		PRINT(text);
		Status= FERROR_NETFAILURE;
		goto Cleanup;

	}

	if (pRxCommandHeader->CmdAck == ACK_NO_ERROR && ReturnSize == sizeof(RxBuff))
	{
		memcpy(prm,pRxCommandHeader->Data,sizeof(manufacture_block));
		Status = FOK;
	}
	else 
	{
		memset(prm,0,sizeof(manufacture_block));
		Status = (pRxCommandHeader->CmdAck ==ACK_ERROR_FLASH)? FERROR_FLASHERROR:FERROR_NETFAILURE;
	}
		
Cleanup:

	LeaveCriticalSection( &m_AdapterCritS );

	wsprintf( text, __T("CLoader::LoaderGetParamBlock status - %d \r\n"),Status );
	PRINT(text);
	return Status;
}

/*------------------------------------------------------------------------------------------------------------
Function Name:         LoaderSetParamBlock

Description:           This function burns the Manufacture Parameters Block into the flash memory. 
                       The Manufacture parameters block resides in the second last sector of the 
					   last boot block.

Parameters:            IN: *prm – pointer to the parameter block to be burned .
                       IN. deviceIP – The IP address of the device to send the command to. At
					   the first stage this argument is not relevant and even may be set to NULL.

Return Values:         FOK, FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,
                       FERROR_NOTCONNECTED, FERROR_FLASHERROR
------------------------------------------------------------------------------------------------------------*/

int CLoader::LoaderSetParamBlock(manufacture_block *prm,char* deviceIP)
{
	char RxBuff[sizeof(tCmdHeader)];
	char TxBuff [sizeof(tCmdHeader)+ sizeof(manufacture_block) -2];
	tCmdHeader *pTxCommandHeader = (tCmdHeader *)TxBuff;
	tCmdHeader *pRxCommandHeader = (tCmdHeader *)RxBuff;
	
	ULONG ReturnSize = 0;
	int Status = FOK;

/*
	if (deviceIP!=NULL)
	{
		// for future use
		// I will choose the right adapter according to this string
		PRINT(__T("CLoader::LoaderSetParamBlock: not supported parameter \r\n "));
		Status = FERROR_NOT_SUPPORTED;
		goto Cleanup;
	}
*/


	EnterCriticalSection( &m_AdapterCritS );

	if(!m_pAdapters)
	{
		PRINT(__T("LoaderGetParamBlock::LoaderSetParamBlock not called setup first\r\n"));
		Status= FERROR_NOTOPENED;
		goto Cleanup;

	}

	// currently implementation is for only 1 adapter at the time.
	if (!m_pAdapters[0].isConnected())
	{
		PRINT(__T("LoaderGetParamBlock::LoaderSetParamBlock adapter is not connected yet.t\r\n"));
		Status= FERROR_NOTCONNECTED;
		goto Cleanup;

	}
	
	pTxCommandHeader->ProtID =  LDR_PROTOCOL_ID;
	pTxCommandHeader->SeqNum =  1;
	pTxCommandHeader->Cmd	   =CMD_SET_MANUFACTURE;
	pTxCommandHeader->CmdAck =  NO_ACK;

	memcpy(pTxCommandHeader->Data, prm, sizeof(manufacture_block));

	short Ack = 0;

	if(m_pAdapters[0].IOControl(SEND_CMD_ID,TxBuff,sizeof(TxBuff),&Ack,sizeof(Ack),&ReturnSize))
	{
		wsprintf( text, __T("ERROR: CLoader::LoaderSetParamBlock fail in IoControl...%d \r\n"),GetLastError() );
		PRINT(text);
		Status= FERROR_NETFAILURE;
	}
	else if(ReturnSize != sizeof(Ack) || Ack != ACK_NO_ERROR)
	{
		Status = (Ack == ACK_ERROR_FLASH)?FERROR_FLASHERROR:FERROR_NETFAILURE;
	}


Cleanup:
	LeaveCriticalSection( &m_AdapterCritS );
	wsprintf( text, __T("CLoader::LoaderSetParamBlock status - %d \r\n"),Status );
	PRINT(text);
	return Status;
}




/* ------------------------------------------------------------------------------------------------------------
Function Name:         LoaderGetSWRights

Description:           This function reads the SW rights block from the flash memory.
                       The SW rights block resides in the third last sector of the 
					   last boot block.

Parameters:            OUT: *pswr – pointer to the sw rights block to get the data 
                       from the  flash.
                       IN. deviceIP – The IP address of the device to send the 
					   command to. At the first stage this argument is not 
					   relevant and even may be set to NULL.

Return Values:        FOK, FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,
                      FERROR_NOTCONNECTED, FERROR_FLASHERROR
------------------------------------------------------------------------------------------------------------*/

int CLoader::LoaderGetSWRights(SWRightsBlock *pswr,char* deviceIP)
{
	char TxBuff[sizeof(tCmdHeader)];
	char RxBuff [sizeof(tCmdHeader)+ sizeof(SWRightsBlock) -2];
	tCmdHeader *pTxCommandHeader = (tCmdHeader *)TxBuff;
	tCmdHeader *pRxCommandHeader = (tCmdHeader *)RxBuff;
	
	ULONG ReturnSize = 0;
	int Status = FOK;

/*
	if (deviceIP!=NULL)
	{
		// for future use
		// I will choose the right adapter according to this string
		PRINT(__T("CLoader::LoaderGetSWRights: not supported parameter \r\n "));
		Status = FERROR_NOT_SUPPORTED;
		goto Cleanup;
	}
*/


	EnterCriticalSection( &m_AdapterCritS );

	if(!m_pAdapters)
	{
		PRINT(__T("LoaderGetParamBlock::LoaderGetSWRights not called setup first\r\n"));
		Status= FERROR_NOTOPENED;
		goto Cleanup;

	}

	// currently implementation is for only 1 adapter at the time.
	if (!m_pAdapters[0].isConnected())
	{
		PRINT(__T("LoaderGetParamBlock::LoaderGetSWRights adapter is not connected yet.t\r\n"));
		Status= FERROR_NOTCONNECTED;
		goto Cleanup;

	}

	
	
	pTxCommandHeader->ProtID =  LDR_PROTOCOL_ID;
	pTxCommandHeader->SeqNum =  0;
	pTxCommandHeader->Cmd	   =CMD_GET_SW_RIGHTS;
	pTxCommandHeader->CmdAck =  NO_ACK;

	if(m_pAdapters[0].IOControl(RECEIVE_CMD_ID, TxBuff, sizeof(tCmdHeader),RxBuff,sizeof(RxBuff),&ReturnSize))
	{

		wsprintf( text, __T("ERROR: CLoader::LoaderGetParamBlock fail in IoControl...%d \r\n"),GetLastError() );
		PRINT(text);
		Status= FERROR_NETFAILURE;
		goto Cleanup;

	}

	if (pRxCommandHeader->CmdAck == ACK_NO_ERROR && ReturnSize == sizeof(RxBuff))
	{
		memcpy(pswr,pRxCommandHeader->Data,sizeof(SWRightsBlock));
		Status = FOK;
	}
	else 
	{
		memset(pswr,0,sizeof(SWRightsBlock));
		Status = (pRxCommandHeader->CmdAck ==ACK_ERROR_FLASH)? FERROR_FLASHERROR:FERROR_NETFAILURE;
	}
		
Cleanup:

	LeaveCriticalSection( &m_AdapterCritS );

	wsprintf( text, __T("CLoader::LoaderGetParamBlock status - %d \r\n"),Status );
	PRINT(text);
	return Status;
}

/*------------------------------------------------------------------------------------------------------------
Function Name:         LoaderSetSWRights

Description:          This function burns the SW rights block into the flash memory.
                      The SW rights block resides in the third last sector of the 
					  last boot block.

Parameters:           IN: * pswr – pointer to the SW rights block to be burned .
                      IN. deviceIP – The IP address of the device to send the command to. 
					  At the first stage this argument is not relevant and even may be set to NULL.

Return Values:        FOK, FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,FERROR_NOTCONNECTED, 
                      FERROR_FLASHERROR
------------------------------------------------------------------------------------------------------------*/
int CLoader::LoaderSetSWRights(SWRightsBlock *pswr,char* deviceIP)
{
	char RxBuff[sizeof(tCmdHeader)];
	char TxBuff [sizeof(tCmdHeader)+ sizeof(SWRightsBlock) -2];
	tCmdHeader *pTxCommandHeader = (tCmdHeader *)TxBuff;
	tCmdHeader *pRxCommandHeader = (tCmdHeader *)RxBuff;
	
	ULONG ReturnSize = 0;
	int Status = FOK;

/*
	if (deviceIP!=NULL)
	{
		// for future use
		// I will choose the right adapter according to this string
		PRINT(__T("CLoader::LoaderSetSWRights: not supported parameter \r\n "));
		Status = FERROR_NOT_SUPPORTED;
		goto Cleanup;
	}
*/

	EnterCriticalSection( &m_AdapterCritS );

	if(!m_pAdapters)
	{
		PRINT(__T("LoaderGetParamBlock::LoaderSetSWRights not called setup first\r\n"));
		Status= FERROR_NOTOPENED;
		goto Cleanup;

	}

	// currently implementation is for only 1 adapter at the time.
	if (!m_pAdapters[0].isConnected())
	{
		PRINT(__T("LoaderGetParamBlock::LoaderSetSWRights adapter is not connected yet.t\r\n"));
		Status= FERROR_NOTCONNECTED;
		goto Cleanup;

	}
	
	pTxCommandHeader->ProtID =  LDR_PROTOCOL_ID;
	pTxCommandHeader->SeqNum =  1;
	pTxCommandHeader->Cmd	   =CMD_SET_SW_RIGHTS;
	pTxCommandHeader->CmdAck =  NO_ACK;

	memcpy(pTxCommandHeader->Data, pswr, sizeof(SWRightsBlock));

	short Ack = 0;

	if(m_pAdapters[0].IOControl(SEND_CMD_ID,TxBuff,sizeof(TxBuff),&Ack,sizeof(Ack),&ReturnSize))
	{
		wsprintf( text, __T("ERROR: CLoader::LoaderSetParamBlock fail in IoControl...%d \r\n"),GetLastError() );
		PRINT(text);
		Status= FERROR_NETFAILURE;
	}
	else if(ReturnSize != sizeof(Ack) || Ack != ACK_NO_ERROR)
	{
		Status = (Ack == ACK_ERROR_FLASH)?FERROR_FLASHERROR:FERROR_NETFAILURE;
	}


Cleanup:
	LeaveCriticalSection( &m_AdapterCritS );
	wsprintf( text, __T("CLoader::LoaderSetParamBlock status - %d \r\n"),Status );
	PRINT(text);
	return Status;
}





//------------------------------------------------------------------------------------------------
// Downloads the nk.bin file into the device RAM and then launches it

//------------------------------------------------------------------------------------------------

int CLoader::LoaderOSLoad(const char* binFile, ProgressCallback pfnProgressCallback,char* deviceIP)
{
	//LOG(Log,debug_level::note_ex)<<"Enter USBLoaderProgram"<<GOL;
	
	char Buff[MAX_LDR_MSG_LENGTH];
	tCmdHeader *pCommandHeader = (tCmdHeader *)Buff;
	DWORD CurrentDataLen;
	DWORD TotalFileLength,CurrentReadenFileLen = 0;
	SProgressCallbackParams ProgressCallbackParams;
	ULONG ReturnSize = 0;
	int Status = FOK;

/*
	if (deviceIP!=NULL)
	{
		// for future use
		// I will choose the right adapter according to this string
		PRINT(__T("CLoader::LoaderOSLoad: not supported parameter \r\n "));
		Status = FERROR_NOT_SUPPORTED;
		goto Cleanup;
	}
*/

	EnterCriticalSection( &m_AdapterCritS );

	if(!m_pAdapters)
	{
		//LOG(Log,debug_level::error)<<"LoaderGetParamBlock not called setup first. "<<GOL;
		Status= FERROR_NOTOPENED;
		goto Cleanup;

	}

	// currently implementation is for only 1 adapter at the time.
	if (!m_pAdapters[0].isConnected())
	{
		//LOG(Log,debug_level::error)<<"LoaderGetParamBlock: adapter is not connected yet. "<<GOL;
		Status= FERROR_NOTCONNECTED;
		goto Cleanup;

	}
	


	//Open the file for getting the file size
	WCHAR* wBinFile = (WCHAR*)LocalAlloc (LPTR,sizeof(WCHAR)*(strlen(binFile)+1));
	mbstowcs(wBinFile,binFile,strlen(binFile));
	wBinFile[strlen(binFile)] = L'\0';

   // m_pfnProgressCallback = pfnProgressCallback;
	HANDLE FileHandle = CreateFile(wBinFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
	if(FileHandle == INVALID_HANDLE_VALUE)
	{
		//Can not open the file 
		if(pfnProgressCallback)
		{
			//Send notification to the callback routine
			strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
			ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
			ProgressCallbackParams.methodId = PCID_DownLoad;
			ProgressCallbackParams.reason = PCRS_SetupFailed;
			ProgressCallbackParams.position =0;
			ProgressCallbackParams.overallSize =0;
			pfnProgressCallback(&ProgressCallbackParams);
		}
		//LOG(Log,debug_level::error)<<"USBLoaderProgram fail in create file "<<GetLastError()<<GOL;
		Status= FERROR_BADBINFILE;
		LocalFree (wBinFile);
		goto Cleanup;

	}
	//Get The Total file length
	TotalFileLength =  GetFileSize(FileHandle,NULL);
	CloseHandle(FileHandle);
	LocalFree (wBinFile);

	
	short Ack;
	//Read all the file and send it for burning to the device.
	FILE *Fop = fopen(binFile,"rb");
	if(Fop)
	{
		Ack = 0;

	    pCommandHeader->ProtID =LDR_PROTOCOL_ID;
		pCommandHeader->SeqNum =0;
		pCommandHeader->Cmd	   =CMD_BIN_DOWNLOAD;
		pCommandHeader->CmdAck =NO_ACK;

		while((CurrentDataLen = fread(Buff + sizeof(tCmdHeader)- 2, sizeof(char), sizeof(Buff) - sizeof(tCmdHeader)+2, Fop)))
		{	
			
			pCommandHeader->SeqNum ++;

			
			if(m_pAdapters[0].IOControl(SEND_CMD_ID, Buff, CurrentDataLen + sizeof(tCmdHeader)-2,&Ack,sizeof(Ack),&ReturnSize))
			{
				//The IOCTL Failed....
				if(pfnProgressCallback)
				{
					//Send notification to the callback routine
					strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
					ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
					ProgressCallbackParams.methodId = PCID_DownLoad;
					ProgressCallbackParams.reason = PCRS_SetupFailed;
					ProgressCallbackParams.position =0;
			        ProgressCallbackParams.overallSize =0;
					pfnProgressCallback(&ProgressCallbackParams);
				}	
				//LOG(Log,debug_level::error)<<"USBLoaderProgram fail in DeviceIoControl "<<GetLastError()<<GOL;
				fclose(Fop);
				Status= FERROR_NETFAILURE;
				goto Cleanup;

			}
			else
			{
				//The IOCTL Success....
				if(ReturnSize == sizeof(Ack) && Ack == ACK_NO_ERROR)
				{
					CurrentReadenFileLen +=CurrentDataLen;
					//Addrress += CurrentDataLen;
					if(pfnProgressCallback)
					{
						//Send notification to the callback routine
						strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
						ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
						ProgressCallbackParams.methodId = PCID_DownLoad;
						ProgressCallbackParams.reason = PCRS_Progress;
						ProgressCallbackParams.overallSize = TotalFileLength;
						ProgressCallbackParams.position = CurrentReadenFileLen;
						pfnProgressCallback(&ProgressCallbackParams);
					}
				}
				else
				{
					//the device returns Wrong Ack.
					if(pfnProgressCallback)
					{
						strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
						ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
						ProgressCallbackParams.methodId = PCID_DownLoad;
						ProgressCallbackParams.reason = PCRS_SetupFailed;
						ProgressCallbackParams.position =0;
			            ProgressCallbackParams.overallSize =0;
						pfnProgressCallback(&ProgressCallbackParams);
					}	
					//char tt[20];
					//sprintf(tt," 0x%x ",Addrress);

					//LOG(Log,debug_level::note_ex)<<"IOCTL_SEND_COMMAND:"<<CurrentDataLen<<GOL;
			
					//LOG(Log,debug_level::error)<<"USBLoaderProgram return wrong Ack from the device "<<Ack<<GOL;
					fclose(Fop);
	                Status= FERROR_NETFAILURE;
					goto Cleanup;
				}
			}	
		}
		fclose(Fop);
		if(TotalFileLength != CurrentReadenFileLen)
		{
			//The loop finshed before the end of the file.
			if(pfnProgressCallback)
			{
				strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
				ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
				ProgressCallbackParams.methodId = PCID_DownLoad;
				ProgressCallbackParams.reason = PCRS_SetupFailed;
				ProgressCallbackParams.position =0;
			    ProgressCallbackParams.overallSize =0;
				pfnProgressCallback(&ProgressCallbackParams);
			}	
			//LOG(Log,debug_level::error)<<"USBLoaderProgram fail in fread before the end of the file "<<GetLastError()<<GOL;
			Status = FERROR_BADBINFILE;
		}
		else
		{

			// need to launch it now from the RAM

			pCommandHeader->ProtID =LDR_PROTOCOL_ID;
			pCommandHeader->SeqNum =1;
			pCommandHeader->Cmd	   =CMD_IMG_RAM_LAUNCH;
			pCommandHeader->CmdAck =NO_ACK;

			
			
			if(m_pAdapters[0].IOControl(SEND_CMD_ID,Buff,sizeof(tCmdHeader),&Ack,sizeof(Ack),&ReturnSize))
			{
				//The IOCTL Failed....
				if(pfnProgressCallback)
				{
					//Send notification to the callback routine
					strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
					ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
					ProgressCallbackParams.methodId = PCID_Lauch;
					ProgressCallbackParams.reason = PCRS_SetupFailed;
					ProgressCallbackParams.position =0;
			        ProgressCallbackParams.overallSize =0;
					pfnProgressCallback(&ProgressCallbackParams);
				}	
				//LOG(Log,debug_level::error)<<"USBLoaderProgram fail in DeviceIoControl "<<GetLastError()<<GOL;
				Status= FERROR_NETFAILURE;
				goto Cleanup;

			}

			//
			if(ReturnSize == sizeof(Ack) && Ack == ACK_NO_ERROR)
			{
					//Finish to launch new os image
					if(pfnProgressCallback)
					{
						//Send notification to the callback routine
						strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
						ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
						ProgressCallbackParams.methodId = PCID_Lauch;
						ProgressCallbackParams.reason = PCRS_SetupComplete;
						ProgressCallbackParams.position =0;
			            ProgressCallbackParams.overallSize =0;
						pfnProgressCallback(&ProgressCallbackParams);
					}
			}
			else
			{
					//the device returns Wrong Ack.
					if(pfnProgressCallback)
					{
						strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
						ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
						ProgressCallbackParams.methodId = PCID_Lauch;
						ProgressCallbackParams.reason = PCRS_SetupFailed;
						ProgressCallbackParams.position =0;
			            ProgressCallbackParams.overallSize =0;
						pfnProgressCallback(&ProgressCallbackParams);
					}	
					

					//LOG(Log,debug_level::note_ex)<<"IOCTL_SEND_COMMAND:"<<CurrentDataLen<<GOL;
			
					//LOG(Log,debug_level::error)<<"LoaderProgram return wrong Ack from the device "<<Ack<<GOL;

					Status= (Ack==ACK_IMAGE_NOT_BURNED)? FERROR_IMAGE_NOT_BURNED:FERROR_NETFAILURE;
					goto Cleanup;

			}
			
		}
	}
	else
	{
		//Cannot open the file 
		if(pfnProgressCallback)
		{
			//Send notification to the callback routine
			strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH,m_pAdapters[0].getRemoteIPString());
			ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';
			ProgressCallbackParams.methodId = PCID_DownLoad;
			ProgressCallbackParams.reason = PCRS_SetupFailed;
			ProgressCallbackParams.position =0;
			ProgressCallbackParams.overallSize =0;
			pfnProgressCallback(&ProgressCallbackParams);
		}
		//LOG(Log,debug_level::error)<<"LoaderProgram fail in create file "<<GetLastError()<<GOL;
		Status= FERROR_BADBINFILE;
		goto Cleanup;

	}

Cleanup:

    LeaveCriticalSection( &m_AdapterCritS );
	wsprintf( text, __T("CLoader::LoaderProgram status - %d \r\n"),Status );
	PRINT(text);
	return Status;
}



/* ------------------------------------------------------------------------------------------------------------
Function Name:         LoaderOSLaunch

Description:           This function reads the existing OS Image from the flash 
                       and then launches it.

Parameters:           IN. deviceIP – The IP address of the device to send the command to. 
                      At the first stage this argument is not relevant and even may be set to NULL

Return Values:        FOK, FERROR_NOT_BURNED, FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,
                      FERROR_NOTOPENED,FERROR_NOTCONNECTED


------------------------------------------------------------------------------------------------------------*/

int CLoader::LoaderOSLaunch(char* deviceIP)
{
	char TxBuff [sizeof(tCmdHeader)];
	tCmdHeader *pTxCommandHeader = (tCmdHeader *)TxBuff;

	
	ULONG ReturnSize = 0;
	int Status = FOK;

/*
	if (deviceIP!=NULL)
	{
		// for future use
		// I will choose the right adapter according to this string
		PRINT(__T("CLoader::LoaderOSLaunch: not supported parameter \r\n "));
		Status = FERROR_NOT_SUPPORTED;
		goto Cleanup;
	}
*/

	EnterCriticalSection( &m_AdapterCritS );

	if(!m_pAdapters)
	{
		//LOG(Log,debug_level::error)<<"LoaderGetParamBlock not called setup first. "<<GOL;
		Status= FERROR_NOTOPENED;
		goto Cleanup;

	}

	// currently implementation is for only 1 adapter at the time.
	if (!m_pAdapters[0].isConnected())
	{
		//LOG(Log,debug_level::error)<<"LoaderGetParamBlock: adapter is not connected yet. "<<GOL;
		Status= FERROR_NOTCONNECTED;
		goto Cleanup;

	}
	
	
	
	pTxCommandHeader->ProtID =  LDR_PROTOCOL_ID;
	pTxCommandHeader->SeqNum =  1;
	pTxCommandHeader->Cmd	   =CMD_IMG_FLASH_LAUNCH;
	pTxCommandHeader->CmdAck =  NO_ACK;

    short Ack;
	
	if(m_pAdapters[0].IOControl(SEND_CMD_ID,TxBuff,sizeof(TxBuff),&Ack,sizeof(Ack),&ReturnSize))
	{
		//LOG(Log,debug_level::error)<<"LoaderOSLaunch fail in DeviceIoControl "<<GetLastError()<<GOL;
		Status = FERROR_NETFAILURE;
	}

			//
	else if(ReturnSize != sizeof(Ack) && Ack != ACK_NO_ERROR)
	{
		
		Status = (Ack ==ACK_IMAGE_NOT_BURNED)? FERROR_IMAGE_NOT_BURNED:FERROR_NETFAILURE;
	}

Cleanup:
	LeaveCriticalSection( &m_AdapterCritS );

	wsprintf( text, __T("CLoader::LoaderOSLaunch status - %d \r\n"),Status );
	PRINT(text);
	return Status;
}


/*------------------------------------------------------------------------------------------------------------
Function Name:         LoaderFormatDataPartition

Description:           This function formats data partition. The boot sets the flag pArgs->bFormatPartFlag = TRUE, 
                       when getting CMD_FORMAT_PART command. This flag resides in BOOT<-> OS Image shared memory 
					   at physical address 0x80000000 + offset(bFormatPartFlag). 
					   The OS Image checks this flag at startup and formats Data partition if its set. 
					   So this function should be used with conjunction with LoaderOSLaunch() function 
					   to allow OS image to format Data Partition. 

Parameters:            IN. deviceIP – The IP address of the device to send the command to. 
                       At the first stage this argument is not relevant and even may be set to NULL.

Return Values:         FOK,  FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,FERROR_NOTCONNECTED.

------------------------------------------------------------------------------------------------------------*/

int CLoader::LoaderFormatDataPartition(char* deviceIP)
{
	char TxBuff [sizeof(tCmdHeader)];
	tCmdHeader *pTxCommandHeader = (tCmdHeader *)TxBuff;

	
	ULONG ReturnSize = 0;
	int Status = FOK;

/*
	if (deviceIP!=NULL)
	{
		// for future use
		// I will choose the right adapter according to this string
		PRINT(__T("CLoader::LoaderFormatDataPartition: not supported parameter \r\n "));
		Status = FERROR_NOT_SUPPORTED;
		goto Cleanup;
	}
*/

	EnterCriticalSection( &m_AdapterCritS );

	if(!m_pAdapters)
	{
		//LOG(Log,debug_level::error)<<"LoaderGetParamBlock not called setup first. "<<GOL;
		Status= FERROR_NOTOPENED;
		goto Cleanup;

	}

	// currently implementation is for only 1 adapter at the time.
	if (!m_pAdapters[0].isConnected())
	{
		//LOG(Log,debug_level::error)<<"LoaderGetParamBlock: adapter is not connected yet. "<<GOL;
		Status= FERROR_NOTCONNECTED;
		goto Cleanup;

	}
	
	
	
	pTxCommandHeader->ProtID =  LDR_PROTOCOL_ID;
	pTxCommandHeader->SeqNum =  1;
	pTxCommandHeader->Cmd	   =CMD_FORMAT_PART;
	pTxCommandHeader->CmdAck =  NO_ACK;

    short Ack;
	
	if(m_pAdapters[0].IOControl(SEND_CMD_ID,TxBuff,sizeof(TxBuff),&Ack,sizeof(Ack),&ReturnSize))
	{
		//LOG(Log,debug_level::error)<<"LoaderOSLaunch fail in DeviceIoControl "<<GetLastError()<<GOL;
		Status = FERROR_NETFAILURE;
	}

			//
	else if(ReturnSize != sizeof(Ack) && Ack != ACK_NO_ERROR)
	{
		
		Status = FERROR_NETFAILURE;
	}

Cleanup:
	LeaveCriticalSection( &m_AdapterCritS );

	wsprintf( text, __T("CLoader::LoaderFormatPartition status - %d \r\n"),Status );
	PRINT(text);
	return Status;
}


/* ------------------------------------------------------------------------------------------------------------
Function Name:         LoaderCleanRegistry

Description:           This function clean registry back to factory. 
                       The boot sets the flag pArgs->bHiveCleanFlag = TRUE, 
					   when getting CMD_ERASE_REGISTRY command. 
					   This flag resides in BOOT<-> OS Image shared memory at physical 
					   address 0x80000000 + offset(bHiveCleanFlag). The OS Image checks 
					   this flag at startup and clean registry if this flag is set. 
					   So this function should be used with conjunction with LoaderOSLaunch()
					   function  to allow OS image to clean the registry. 

Parameters:            IN. deviceIP – The IP address of the device to send the command to. 
                       At the first stage this argument is not relevant and even may be set to NULL.

Return Values:         FOK,  FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,FERROR_NOTCONNECTED.

------------------------------------------------------------------------------------------------------------*/

int CLoader::LoaderCleanRegistry(char* deviceIP)
{
	char TxBuff [sizeof(tCmdHeader)];
	tCmdHeader *pTxCommandHeader = (tCmdHeader *)TxBuff;

	
	ULONG ReturnSize = 0;
	int Status = FOK;

/*
	if (deviceIP!=NULL)
	{
		// for future use
		// I will choose the right adapter according to this string
		PRINT(__T("LoaderGetParamBlock::LoaderCleanRegistry not supported parameter\r\n"));
		Status= FERROR_NOT_SUPPORTED;
		goto Cleanup;
	}
*/

	EnterCriticalSection( &m_AdapterCritS );

	if(!m_pAdapters)
	{
		PRINT(__T("LoaderGetParamBlock::LoaderCleanRegistry not called setup first\r\n"));
		Status= FERROR_NOTOPENED;
		goto Cleanup;

	}

	// currently implementation is for only 1 adapter at the time.
	if (!m_pAdapters[0].isConnected())
	{
		PRINT(__T("LoaderGetParamBlock::LoaderCleanRegistry adapter is not connected yet.t\r\n"));
		Status= FERROR_NOTCONNECTED;
		goto Cleanup;

	}
	
	
	
	pTxCommandHeader->ProtID =  LDR_PROTOCOL_ID;
	pTxCommandHeader->SeqNum =  1;
	pTxCommandHeader->Cmd	   =CMD_ERASE_REGISTRY;
	pTxCommandHeader->CmdAck =  NO_ACK;

    short Ack;
	
	if(m_pAdapters[0].IOControl(SEND_CMD_ID,TxBuff,sizeof(TxBuff),&Ack,sizeof(Ack),&ReturnSize))
	{
		wsprintf( text, __T("ERROR: CLoader::LoaderCleanRegistry fail in IoControl...%d \r\n"),GetLastError() );
		PRINT(text);

		Status = FERROR_NETFAILURE;
	}

			//
	else if(ReturnSize != sizeof(Ack) && Ack != ACK_NO_ERROR)
	{
		
		Status = FERROR_NETFAILURE;
	}

Cleanup:
	LeaveCriticalSection( &m_AdapterCritS );

	wsprintf( text, __T("CLoader::LoaderCleanRegistry status - %d \r\n"),Status );
	PRINT(text);
	return Status;
}

/*----------------------------------------------------------------------------------------------
Function Name:         LoaderGetHWVersion

Description:           This function gets the CPU and Companion Chip versions. 

Parameters:           OUT: *  pHWVersionInfo - Pointer to the hw version information info.
                      It includes CPU and Companion Chip versions.
                  
                      IN. deviceIP – The IP address of the device to send the command to. 
					  At the first stage this argument is not relevant and even may be set to NULL.

Return Values:       FOK,  FERROR_VERSION_NOT_FOUND, FERROR_IMAGE_NOT_BURNED,FERROR_NETFAILURE,
                     FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,FERROR_NOTCONNECTED.


----------------------------------------------------------------------------------------------*/

int CLoader::LoaderGetHWVersion(HWVersions *pHWVersionInfo,char* deviceIP)
{
	PRINT(__T("CLoader::LoaderGetHWVersion \r\n"));
					

	char TxBuff[sizeof(tCmdHeader)];
	char RxBuff [sizeof(tCmdHeader)+ sizeof(HWVersions) -2];
	tCmdHeader *pTxCommandHeader = (tCmdHeader *)TxBuff;
	tCmdHeader *pRxCommandHeader = (tCmdHeader *)RxBuff;
	
	ULONG ReturnSize = 0;
	int Status = FOK;

	/*
	if (deviceIP!=NULL)
	{
		// for future use
		// I will choose the right adapter according to this string
		PRINT(__T("LoaderGetParamBlock::LoaderGetHWVersion not supported parameter\r\n"));
		Status= FERROR_NOT_SUPPORTED;
		goto Cleanup;
	}
	*/


	EnterCriticalSection( &m_AdapterCritS );

	if(!m_pAdapters)
	{
		PRINT(__T("LoaderGetParamBlock::LoaderGetHWVersion not called setup first\r\n"));
		Status= FERROR_NOTOPENED;
		goto Cleanup;

	}

	// currently implementation is for only 1 adapter at the time.
	if (!m_pAdapters[0].isConnected())
	{
		PRINT(__T("LoaderGetParamBlock::LoaderGetHWVersion adapter is not connected yet.t\r\n"));
		Status= FERROR_NOTCONNECTED;
		goto Cleanup;

	}
	


	
	pTxCommandHeader->ProtID =  LDR_PROTOCOL_ID;
	pTxCommandHeader->SeqNum =  0;
	pTxCommandHeader->Cmd	   =CMD_GET_HW_VERSION;
	pTxCommandHeader->CmdAck =  NO_ACK;


	if(m_pAdapters[0].IOControl(RECEIVE_CMD_ID, TxBuff, sizeof(tCmdHeader),RxBuff,sizeof(RxBuff),&ReturnSize))
	{
		wsprintf( text, __T("ERROR: CLoader::LoaderGetHWVersion fail in IoControl...%d \r\n"),GetLastError() );
		PRINT(text);

		Status = FERROR_NETFAILURE;
	}
	else
	{

		if (pRxCommandHeader->CmdAck == ACK_NO_ERROR && ReturnSize == sizeof(RxBuff))
		{
			memcpy(pHWVersionInfo,pRxCommandHeader->Data,sizeof(HWVersions));
			Status = FOK;
		}
		else 
		{
			memset(pHWVersionInfo,0,sizeof(HWVersions));
			Status = FERROR_NETFAILURE;
		}
	}
		
Cleanup:
	
	LeaveCriticalSection( &m_AdapterCritS );

	wsprintf( text, __T("CLoader::LoaderGetHWVersion status - %d \r\n"),Status );
	PRINT(text);

	return Status;
}


int CLoader::TestAdapter(char* deviceIP)
{
	/*
	if (deviceIP!=NULL)
	{
		// for future use
		// I will choose the right adapter according to this string
		PRINT(__T("LoaderGetParamBlock::LoaderGetHWVersion not supported parameter\r\n"));
		Status= FERROR_NOT_SUPPORTED;
		goto Cleanup;
	}
	*/

	if(!m_pAdapters)
	{
		PRINT(__T("LoaderGetParamBlock::LoaderGetHWVersion not called setup first\r\n"));
		return FERROR_NOTOPENED;
	}

	// currently implementation is for only 1 adapter at the time.
	if (!m_pAdapters[0].isConnected())
	{
		PRINT(__T("LoaderGetParamBlock::LoaderGetHWVersion adapter is not connected yet.t\r\n"));
		return FERROR_NOTCONNECTED;
	}

	return FOK;
}
int CLoader::LoaderGetFlashInfo(FlashInformation* pFlashInfo, char* deviceIP)
{
	PRINT(__T("CLoader::LoaderGetFlashInfo \r\n"));
	int Status = FOK;
					
	EnterCriticalSection( &m_AdapterCritS );
	if( Status = TestAdapter(deviceIP) )
	{
		LeaveCriticalSection( &m_AdapterCritS );
		return Status;	
	}

	char TxBuff[sizeof(tCmdHeader)];
	char RxBuff [MAX_LDR_MSG_LENGTH];

	tCmdHeader *pTxCommandHeader = (tCmdHeader *)TxBuff;
	tCmdHeader *pRxCommandHeader = (tCmdHeader *)RxBuff;
	
	ULONG ReturnSize = 0;

	
	pTxCommandHeader->ProtID =  LDR_PROTOCOL_ID;
	pTxCommandHeader->SeqNum =  0;
	pTxCommandHeader->Cmd	 =	CMD_GET_FLASH_INFO;
	pTxCommandHeader->CmdAck =  NO_ACK;


	if(m_pAdapters[0].IOControl(RECEIVE_CMD_ID, TxBuff, sizeof(tCmdHeader),RxBuff,(sizeof(FlashInformation) + sizeof(tCmdHeader) - 2),&ReturnSize))
	{
		wsprintf( text, __T("ERROR: CLoader::LoaderGetFlashInfo fail in IoControl...%d \r\n"),GetLastError() );
		PRINT(text);

		Status = FERROR_NETFAILURE;
	}
	else
	{

		if (pRxCommandHeader->CmdAck == ACK_NO_ERROR && ReturnSize == (sizeof(FlashInformation) + sizeof(tCmdHeader) - 2 ))
		{
			memcpy(pFlashInfo,pRxCommandHeader->Data,sizeof(FlashInformation));
			Status = FOK;
		}
		else 
		{
			Status = FERROR_NETFAILURE;
		}
	}
		
	LeaveCriticalSection( &m_AdapterCritS );

	wsprintf( text, __T("CLoader::LoaderGetFlashInfo status - %d \r\n"),Status );
	PRINT(text);

	return Status;
}


int CLoader::LoaderGetPartInfo(PARTENTRY* pPartInfo, char* deviceIP)
{
	PRINT(__T("CLoader::LoaderGetPartInfo \r\n"));
	int Status = FOK;
					
	EnterCriticalSection( &m_AdapterCritS );
	if( Status = TestAdapter(deviceIP) )
	{
		LeaveCriticalSection( &m_AdapterCritS );
		return Status;	
	}

	char TxBuff[sizeof(tCmdHeader)];
	char RxBuff [MAX_LDR_MSG_LENGTH];

	tCmdHeader *pTxCommandHeader = (tCmdHeader *)TxBuff;
	tCmdHeader *pRxCommandHeader = (tCmdHeader *)RxBuff;
	
	ULONG ReturnSize = 0;

	
	pTxCommandHeader->ProtID =  LDR_PROTOCOL_ID;
	pTxCommandHeader->SeqNum =  0;
	pTxCommandHeader->Cmd	 =	CMD_GET_PART_INFO;
	pTxCommandHeader->CmdAck =  NO_ACK;


	if(m_pAdapters[0].IOControl(RECEIVE_CMD_ID, TxBuff, sizeof(tCmdHeader),RxBuff,
								sizeof(PARTENTRY) * NUM_PARTS + sizeof(tCmdHeader) - 2,&ReturnSize))
	{
		wsprintf( text, __T("ERROR: CLoader::LoaderGetPartInfo fail in IoControl...%d \r\n"),GetLastError() );
		PRINT(text);

		Status = FERROR_NETFAILURE;
	}
	else
	{

		if (pRxCommandHeader->CmdAck == ACK_NO_ERROR && ReturnSize == (sizeof(PARTENTRY) * NUM_PARTS + sizeof(tCmdHeader) - 2 ))
		{
			memcpy(pPartInfo,pRxCommandHeader->Data,sizeof(PARTENTRY) * NUM_PARTS);
			Status = FOK;
		}
		else 
		{
			Status = FERROR_NETFAILURE;
		}
	}
		
	LeaveCriticalSection( &m_AdapterCritS );

	wsprintf( text, __T("CLoader::LoaderGetPartInfo status - %d \r\n"),Status );
	PRINT(text);

	return Status;
}


int CLoader::LoaderGetSector( char* TxBuff, char* RxBuff, char* SectorBuff, DumpParams* params, char* deviceIP)
{
	int Status = -1;
	if( !TxBuff || !RxBuff || !SectorBuff || !params )
		return Status;

	tCmdHeader *pTxCommandHeader = (tCmdHeader *)TxBuff;
	tCmdHeader *pRxCommandHeader = (tCmdHeader *)RxBuff;
	
	DWORD	TotalLength = 0;
	ULONG	ReturnSize = 0;
	DWORD	size = min( MAX_LDR_DATA_LENGTH, params->DumpSize );
	
	memcpy( pTxCommandHeader->Data,	params, sizeof(DumpParams) );

	pTxCommandHeader->ProtID =  LDR_PROTOCOL_ID;
	pTxCommandHeader->SeqNum =  0;
	pTxCommandHeader->Cmd	 =	CMD_GET_STORE_DUMP;
	pTxCommandHeader->CmdAck =  NO_ACK;

	while( TotalLength < params->DumpSize )
	{
		size = min( MAX_LDR_DATA_LENGTH, (params->DumpSize - TotalLength));
		
		if(m_pAdapters[0].IOControl(RECEIVE_CMD_ID, TxBuff, sizeof(tCmdHeader) + sizeof(DumpParams) - 2,
			RxBuff, size + sizeof(tCmdHeader) - 2,&ReturnSize))
		{
			wsprintf( text, __T("ERROR: CLoader::LoaderGetParamBlock failed in IoControl...%d \r\n"),GetLastError() );
			PRINT(text);
			return FERROR_NETFAILURE;
		}
	if( ReturnSize != (size + sizeof(tCmdHeader) - 2) )//maybe temp!!!!
	{
		Status = (pRxCommandHeader->CmdAck ==ACK_ERROR_FLASH)? FERROR_FLASHERROR:FERROR_NETFAILURE;
		break;
	}

		if (pRxCommandHeader->CmdAck != ACK_NO_ERROR )
		{
			Status = (pRxCommandHeader->CmdAck ==ACK_ERROR_FLASH)? FERROR_FLASHERROR:FERROR_NETFAILURE;
			break;
		}

		TotalLength += ReturnSize - sizeof(tCmdHeader) + 2;
		memcpy( SectorBuff + pRxCommandHeader->SeqNum * MAX_LDR_DATA_LENGTH,  
				pRxCommandHeader->Data, 
				ReturnSize - sizeof(tCmdHeader) + 2);
		Status = FOK;		
		pTxCommandHeader->SeqNum++;
	}//while

	return Status;
}
int CLoader::LoaderDumpStore(ProgressCallback pfnProgressCallback, const char* FileN, const FlashInformation* pFlashInfo, unsigned long dp_offset,char* deviceIP)
{
	if( !FileN || !pFlashInfo )
	{
//		LOG(Log,debug_level::error)<<"USBLoaderGetImgDifference fail in create file "<<GetLastError()<<GOL;
		return -1;//FERROR_BADBINFILE;
	}
	DWORD out_len;//

	TCHAR	FileName[MAX_PATH] = {0};
	mbstowcs_s( (size_t*)&out_len, FileName, MAX_PATH, FileN, strlen(FileN) );

	EnterCriticalSection( &m_AdapterCritS );

	int Status = -1;
	if( Status = TestAdapter(deviceIP) )
	{
		LeaveCriticalSection( &m_AdapterCritS );
		return Status;	
	}

	HANDLE hFile = CreateFile( FileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, 0);
	if( hFile == INVALID_HANDLE_VALUE)
	{
//		LOG(Log,debug_level::error)<<"USBLoaderGetImgDifference fail in create file "<<GetLastError()<<GOL;
		LeaveCriticalSection( &m_AdapterCritS );
		return FERROR_BADBINFILE;
	}


	char	RxBuff[MAX_LDR_MSG_LENGTH]	= {0};
	char	TxBuff[MAX_LDR_MSG_LENGTH]	= {0};
	
	char*	SectorBuff = new char[pFlashInfo->wDataBytesPerSector];

	DWORD	Total = ( pFlashInfo->dwNumBlocks - pFlashInfo->dwNumOfBadBlocks ) * pFlashInfo->dwBytesPerBlock;
	
	SProgressCallbackParams ProgressCallbackParams;
	if(pfnProgressCallback)
	{
		ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';//for everyone
		//Send notification to the callback routine
		//strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH - 1,m_pAdapters[0].getRemoteIPString());
		ProgressCallbackParams.methodId							= PCID_DownLoad;
		ProgressCallbackParams.reason							= PCRS_Progress;
		ProgressCallbackParams.position							= 0;
		ProgressCallbackParams.overallSize						= Total;
		SetProgress( pfnProgressCallback, ProgressCallbackParams);
	}

	DumpParams	Params = {0};

	Params.DumpSize		= pFlashInfo->wDataBytesPerSector;
	Params.DumpOffset	= dp_offset;
	Params.DumpFlag		= DUMP_START;

	while( Total > Params.DumpOffset )
	{
		if( FOK == ( Status = LoaderGetSector( TxBuff, RxBuff, SectorBuff, &Params, deviceIP ) ))
		{
			WriteFile( hFile, SectorBuff, pFlashInfo->wDataBytesPerSector, &out_len, 0 );
			Params.DumpOffset += pFlashInfo->wDataBytesPerSector;
			if( !(Params.DumpOffset % pFlashInfo->dwBytesPerBlock ) )//previous block 
			{
				ProgressCallbackParams.position		= Params.DumpOffset - pFlashInfo->wDataBytesPerSector;
				SetProgress( pfnProgressCallback, ProgressCallbackParams);
			}

			if( Total - Params.DumpOffset <= pFlashInfo->wDataBytesPerSector )
				Params.DumpFlag	= DUMP_STOP;
			else
				Params.DumpFlag	= 0;

		}
		else
		{
			ProgressCallbackParams.reason		= PCRS_SetupFailed;
			ProgressCallbackParams.position		= 0;
			ProgressCallbackParams.overallSize	= 0;
			SetProgress( pfnProgressCallback, ProgressCallbackParams);
			break;
		}
	}

	delete []SectorBuff;
	CloseHandle(hFile);
	LeaveCriticalSection( &m_AdapterCritS );

	wsprintf( text, __T("CLoader::LoaderGetParamBlock status - %d \r\n"),Status );
	PRINT(text);
	return Status;
}

void CLoader::SetProgress(ProgressCallback pfnProgressCallback, SProgressCallbackParams Params )//send params copy
{
	if(pfnProgressCallback)
	{
		pfnProgressCallback(&Params);
	}
}
int CLoader::LoaderUpdateBmp(ProgressCallback pfnProgressCallback, const char* FileN, char* deviceIP)
{
	PRINT(__T("CLoader::LoaderUpdateBmp \r\n"));
	int Status = FOK;
					
	if(!FileN)
	{
		wsprintf( text, __T("CLoader::LoaderUpdateBmp parameters error \r\n") );
		PRINT(text);
		return FERROR_BADBINFILE;
	}

	DWORD	RetSize, ReturnSize;
	DWORD	CurLen		= 0;
	short	Ack			= 0;
	TCHAR	FileName[MAX_PATH] = {0};

	mbstowcs_s( (size_t*)&RetSize, FileName, MAX_PATH, FileN, strlen(FileN) );
	
	HANDLE hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if( (HANDLE)-1 == hFile )
	{
		wsprintf( text, __T("CLoader::LoaderUpdateBmp cannot open file %s \r\n"), FileName );
		PRINT(text);
		return FERROR_BADBINFILE;
	}

	
	EnterCriticalSection( &m_AdapterCritS );
	if( Status = TestAdapter(deviceIP) )
	{
		LeaveCriticalSection( &m_AdapterCritS );
		CloseHandle(hFile);
		return Status;	
	}

	DWORD TotalLength	=  GetFileSize(hFile, 0);
	
	SProgressCallbackParams ProgressCallbackParams;

	ProgressCallbackParams.deviceIPSTR[IP_STRING_LENGTH -1] = '\0';//for everyone
	strcpy_s(ProgressCallbackParams.deviceIPSTR,IP_STRING_LENGTH - 1,m_pAdapters[0].getRemoteIPString());
	ProgressCallbackParams.methodId							= PCID_DownLoad;
	ProgressCallbackParams.reason							= PCRS_Progress;
	ProgressCallbackParams.position							= 0;
	ProgressCallbackParams.overallSize						= TotalLength;
	SetProgress( pfnProgressCallback, ProgressCallbackParams);


	char TxBuff[MAX_LDR_MSG_LENGTH];
//	char RxBuff [sizeof(tCmdHeader)];

	tCmdHeader *pCommandHeader = (tCmdHeader *)TxBuff;
//	tCmdHeader *pRxCommandHeader = (tCmdHeader *)RxBuff;

	pCommandHeader->ProtID = LDR_PROTOCOL_ID;
	pCommandHeader->SeqNum = 0;
	pCommandHeader->Cmd	   = CMD_BIN_DOWNLOAD;
	pCommandHeader->CmdAck = NO_ACK;

	while( TotalLength )
	{
		CurLen = min( MAX_LDR_DATA_LENGTH, TotalLength ); 
		if( !ReadFile(hFile, pCommandHeader->Data, CurLen, &RetSize, 0 ) )
		{
			Status = FERROR_BADBINFILE;
			ProgressCallbackParams.reason		= PCRS_SetupFailed;
			SetProgress( pfnProgressCallback, ProgressCallbackParams);
			break;
		}
		TotalLength -= RetSize;

		pCommandHeader->SeqNum ++;
			
		if(m_pAdapters[0].IOControl(SEND_CMD_ID, TxBuff, RetSize + sizeof(tCmdHeader)-2, &Ack, sizeof(Ack), &ReturnSize))
		{
			//The IOCTL Failed....
			Status								= FERROR_NETFAILURE;	
			ProgressCallbackParams.reason		= PCRS_SetupFailed;
			SetProgress( pfnProgressCallback, ProgressCallbackParams);
			break;
		}
		//The IOCTL Success....
		if(ReturnSize == sizeof(Ack) && Ack == ACK_NO_ERROR)
		{
			ProgressCallbackParams.position = ProgressCallbackParams.overallSize - TotalLength;
			SetProgress( pfnProgressCallback, ProgressCallbackParams);
		}
		else
		{
			//the device returns Wrong Ack.
			Status = FERROR_NETFAILURE;

			ProgressCallbackParams.reason		= PCRS_SetupFailed;
			ProgressCallbackParams.position		= 0;
            ProgressCallbackParams.overallSize	= 0;
			SetProgress( pfnProgressCallback, ProgressCallbackParams);
			break;
	
		}
	}	

	CloseHandle(hFile);

	if( FOK != Status )
	{
		LeaveCriticalSection( &m_AdapterCritS );
		return Status;
	}

//------------------
	pCommandHeader->ProtID	=  LDR_PROTOCOL_ID;
	pCommandHeader->SeqNum	=  0;
	pCommandHeader->Cmd		=	CMD_BURN_BITMAP;
	pCommandHeader->CmdAck	=  NO_ACK;

	if(m_pAdapters[0].IOControl(SEND_CMD_ID, TxBuff, sizeof(tCmdHeader), &Ack, sizeof(Ack), &ReturnSize))
	{
		wsprintf( text, __T("ERROR: CLoader::LoaderUpdateBmp fail in IoControl...%d \r\n"),GetLastError() );
		PRINT(text);

		Status = FERROR_NETFAILURE;

		ProgressCallbackParams.reason		= PCRS_SetupFailed;
		ProgressCallbackParams.position		= 0;
        ProgressCallbackParams.overallSize	= 0;
	}
	else
	{
		if(ReturnSize == sizeof(Ack) && Ack == ACK_NO_ERROR)
		{
			Status = FOK;

			ProgressCallbackParams.reason	= PCRS_SetupComplete;
			ProgressCallbackParams.position = ProgressCallbackParams.overallSize;
		}
		else 
		{
			Status = ACK_IMAGE_NOT_BURNED;
			ProgressCallbackParams.reason		= PCRS_SetupFailed;
			ProgressCallbackParams.position		= 0;
			ProgressCallbackParams.overallSize	= 0;
		}
	}
	SetProgress( pfnProgressCallback, ProgressCallbackParams);
		
	LeaveCriticalSection( &m_AdapterCritS );

	wsprintf( text, __T("CLoader::LoaderUpdateBmp status - %d \r\n"),Status );
	PRINT(text);

	return Status;
}


