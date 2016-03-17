// RCSerial.cpp: implementation of the CSerialCommHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "SerialCommHelper.h"
#include <Process.h>
#include <string>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define TMP_BUF_SIZE	1024 * 4

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void CSerialCommHelper::CloseAndCleanHandle(HANDLE& hHandle)
{

	CloseHandle( hHandle ) ;
	hHandle = (HANDLE)-1;

}
CSerialCommHelper::CSerialCommHelper() :	m_hThreadTerm(0),		
											m_hThread(0),			
											m_hThreadStarted(0),
											m_hDataRx(0),
											m_hWriteEvent(0),
											m_hCommPort((void*)-1)
{
	InitLock();
	m_eState = SS_UnInit;
}

CSerialCommHelper::~CSerialCommHelper()
{
	m_eState = SS_Unknown;
	DelLock();
}


HANDLE CSerialCommHelper::Init(std::tstring szPortName, DWORD dwBaudRate,BYTE byParity,BYTE byStopBits,BYTE byByteSize)
{
	try
	{
		m_hDataRx		= CreateEvent(0, 0, 0, 0);
		m_hWriteEvent	= CreateEvent(0, 1, 0, 0);

		m_hCommPort = CreateFile(szPortName.c_str(),	GENERIC_READ|GENERIC_WRITE, 0, 0,
														OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
		if( m_hCommPort == INVALID_HANDLE_VALUE )
		{
			CloseHandle(m_hCommPort);
			m_hCommPort = (HANDLE)-1;
			//ASSERT ( 0 );
			return m_hCommPort;
		}
		
		//now start to read but first we need to set the COM port settings and the timeouts
		if(!SetCommMask(m_hCommPort, EV_RXCHAR | EV_TXEMPTY))
		{
			//ASSERT(0);
			CloseHandle(m_hCommPort);
			m_hCommPort = (HANDLE)-1;
			return m_hCommPort;
		}

		DCB dcb = {0};
		
		dcb.DCBlength = sizeof(DCB);
		
		if (!GetCommState (m_hCommPort, &dcb))
		{
			CloseHandle(m_hCommPort);
			m_hCommPort = (HANDLE)-1;
			return (HANDLE)-1;//E_FAIL;
		}
		dcb.DCBlength		= sizeof(dcb);
		dcb.BaudRate		= dwBaudRate;
		dcb.ByteSize		= byByteSize;
		dcb.Parity			= byParity;
		if ( byStopBits == 1 )
			dcb.StopBits	= ONESTOPBIT;
		else if (byStopBits == 2 ) 
			dcb.StopBits	= TWOSTOPBITS;
		else 
			dcb.StopBits	= ONE5STOPBITS;
		dcb.fBinary           = 1;
		dcb.fOutxCtsFlow      = 0;
		dcb.fOutxDsrFlow      = 0;
		dcb.fDtrControl       = DTR_CONTROL_ENABLE;
		dcb.fRtsControl       = RTS_CONTROL_ENABLE;
		dcb.fDsrSensitivity   = 0;
		dcb.fTXContinueOnXoff = 0;
		dcb.fOutX             = 0;
		dcb.fInX              = 0;
		dcb.XoffLim			  = 1024;
		dcb.XoffLim			  = 512;
		dcb.XonChar			  = 17	;
		dcb.XoffChar		  = 19	;	
		dcb.fErrorChar        = 0;  
		dcb.ErrorChar         = 0x0D;  
		dcb.EvtChar			  = 0x0D;	
		dcb.fNull             = 0;
		dcb.fAbortOnError     = 0;
		
		if(!SetCommState(m_hCommPort, &dcb))
		{
	//		ASSERT(0);
			CloseHandle(m_hCommPort);
			m_hCommPort = (HANDLE)-1;
			return m_hCommPort;//E_FAIL;
		}

		COMMTIMEOUTS timeouts;
		
		timeouts.ReadIntervalTimeout					= MAXDWORD; 
		timeouts.ReadTotalTimeoutMultiplier		= 0;
		timeouts.ReadTotalTimeoutConstant		= 0;
		timeouts.WriteTotalTimeoutMultiplier	= 0;
		timeouts.WriteTotalTimeoutConstant		= 0;
		
		if(!SetCommTimeouts(m_hCommPort, &timeouts))
		{
			CloseHandle(m_hCommPort);
			m_hCommPort = (HANDLE)-1;
			return m_hCommPort;//E_FAIL;
		}
		PurgeComm( m_hCommPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );

		//create thread terminator event...
		m_hThreadTerm		= CreateEvent(0,0,0,0);
		m_hThreadStarted	= CreateEvent(0,0,0,0);
		
		m_hThread	= 	(HANDLE)_beginthreadex(0, 0, CSerialCommHelper::ThreadFn, (void*)this, 0, 0 );

		DWORD dwWait = WaitForSingleObject ( m_hThreadStarted , INFINITE );
		
		CloseHandle(m_hThreadStarted);
		m_hThreadStarted	= 0;

		m_abIsConnected = 1;		
	}
	catch(...)
	{
		//ASSERT(0);
		if( (HANDLE)-1 != m_hCommPort )
		{
			CloseHandle(m_hCommPort);
			m_hCommPort = (HANDLE)-1;
		}
	}
	
	if( (HANDLE)-1 != m_hCommPort )
	{
		m_eState = SS_Init;
	}
	return m_hCommPort;
}	
	
 
HRESULT CSerialCommHelper:: Start()
{
	m_eState = SS_Started;
	return S_OK;
}
HRESULT CSerialCommHelper:: Stop()
{
	m_eState = SS_Stopped;
	return S_OK;
}
HRESULT CSerialCommHelper:: UnInit()
{
	HRESULT hr = S_OK;
	try
	{
		m_abIsConnected = 0;
		SetCommMask( m_hCommPort, 0 );
		CancelIo(m_hCommPort);
		SignalObjectAndWait(m_hThreadTerm, m_hThread, INFINITE, FALSE);
		CloseHandle(m_hThreadTerm);
		m_hThreadTerm = 0;
		CloseHandle(m_hThread);	   
		m_hThread	= 0;
		CloseHandle(m_hDataRx);   
		m_hDataRx	= 0;
		CloseHandle(m_hWriteEvent);
		m_hWriteEvent = 0;
		CloseAndCleanHandle(m_hCommPort );
		m_theSerialBuffer.Clear();
	}
	catch(...)
	{
		//ASSERT(0);
		hr = E_FAIL;
	}
	if ( SUCCEEDED(hr)) 
		m_eState = SS_UnInit;
	return hr;
}

unsigned __stdcall CSerialCommHelper::ThreadFn(void*pvParam)
{

	CSerialCommHelper*	apThis		= (CSerialCommHelper*) pvParam ;
	BOOL				abContinue	= 1;
	DWORD				dwEventMask = EV_RXCHAR;
	
	OVERLAPPED ov;
	memset(&ov,0,sizeof(ov));

	ov.hEvent = CreateEvent(0, 0, 0, 0);
	
	HANDLE arHandles[2];
	arHandles[0] = apThis->m_hThreadTerm;

	DWORD dwWait;
	SetEvent(apThis->m_hThreadStarted);

	DWORD dwBytesRead	= 0;
	OVERLAPPED ovRead	= {0};
	ovRead.hEvent = CreateEvent(0, 0, 0, 0);

	char	szTmp[TMP_BUF_SIZE] = {0};
//	CString	dbg;

	while(abContinue)
	{
		BOOL abRet = WaitCommEvent(apThis->m_hCommPort, &dwEventMask, &ov);
		if( !abRet )
		{			
			//ASSERT( GetLastError() == ERROR_IO_PENDING);
			if( GetLastError() != ERROR_IO_PENDING )
				return -1;
		}
		
		arHandles[1] = ov.hEvent;

		dwWait = WaitForMultipleObjects(2, arHandles, FALSE, INFINITE);
		switch ( dwWait )
		{
			case WAIT_OBJECT_0:
			{
				_endthreadex(1);
			}
			break;
			case WAIT_OBJECT_0 + 1:
			{
				DWORD dwMask;
				if (GetCommMask(apThis->m_hCommPort,&dwMask) )
				{
					if ( dwMask == EV_TXEMPTY )
					{
						//AfxMessageBox(_T("Data sent"));
						//ResetEvent ( ov.hEvent );
						continue;
					}		
				}
				//read data here...
				DWORD iAccum = 0;
				
				apThis->m_theSerialBuffer.LockBuffer();
				
				try 
				{
					BOOL abRet = 0;
					
					DWORD	err;
					COMSTAT cs = {0};
					ClearCommError(apThis->m_hCommPort, &err, &cs );

					do
					{
						DWORD size = min(TMP_BUF_SIZE, cs.cbInQue);   
						//ResetEvent(ovRead.hEvent);

						abRet = ReadFile(apThis->m_hCommPort, szTmp, size, &dwBytesRead, &ovRead);
						if(!abRet ) 
						{
							abContinue = FALSE;
							break;
						}
						if ( dwBytesRead > 0 )
						{
							apThis->m_theSerialBuffer.AddData( szTmp, dwBytesRead );
//temp!!debug///////////////////////							
//							dbg.Format(_T("\r\n Added %u: "), dwBytesRead );
//							TCHAR db2[32];
//							for( DWORD i = 0; i < dwBytesRead; ++i )
//							{
//								wsprintf( db2, _T("%02x "), (BYTE)(szTmp[i]) );
//								dbg.Append(db2);
//							}
//							dbg.Append(_T("\r\n"));
//							OutputDebugString(dbg.GetBuffer(0));
///////////////////////////////////
							iAccum += dwBytesRead;
						}
						
						ClearCommError(apThis->m_hCommPort, &err, &cs );

					}while(cs.cbInQue);// dwBytesRead > 0 );
				}
				catch(...)
				{
					//ASSERT(0);
				}
				
				//if we are not in started state then we should flush the queue...( we would still read the data)
				if(apThis->GetCurrentState() != SS_Started ) 
				{
					iAccum  = 0;
					apThis->m_theSerialBuffer.Clear ();
				}
				
				apThis->m_theSerialBuffer.UnLockBuffer();
				
				if( iAccum > 0 )
				{
					apThis->SetDataReadEvent(); 
				}
				//ResetEvent ( ov.hEvent );
			}
			break;
		}//switch
	}
	CloseHandle(ovRead.hEvent );
	return 0;
}


HRESULT  CSerialCommHelper::CanProcess() const
{
	if(SS_Started == m_eState )
		return S_OK;
	return E_FAIL;
}

HRESULT CSerialCommHelper::Write(const BYTE* data, DWORD dwSize) const
{
	HRESULT hr = CanProcess();
	if( FAILED(hr)) 
		return hr;
	
//temp!!!	m_theSerialBuffer.Clear();//clear all received data
	
	int iRet = 0 ;
	OVERLAPPED ov = {0};

	ov.hEvent = m_hWriteEvent;

	DWORD dwBytesWritten = 0;

	iRet = WriteFile(m_hCommPort, data, dwSize, &dwBytesWritten, &ov);
	if(!iRet)
	{
		if( GetLastError() != ERROR_IO_PENDING )
			return S_FALSE;

		WaitForSingleObject(ov.hEvent ,INFINITE);
	}
 
	std::string szData((char*)data);
	
	return S_OK;
}

HRESULT CSerialCommHelper::Read_N(std::string& data, DWORD alCount, DWORD alTimeOut )
{
	HRESULT hr = CanProcess();
	
	if( FAILED(hr)) 
	{
		//ASSERT(0);
		return hr;
	}
	
	try
	{	 	
		std::string szTmp;

		DWORD iLocal =  m_theSerialBuffer.Read_N(szTmp, alCount, m_hDataRx);
		
		if( iLocal == alCount ) 
		{
			data = szTmp;
		}
		else
		{//there are either none or less bytes...
			DWORD iRead = 0;
			DWORD iRemaining = alCount - iLocal;
			while( 1 )
			{
				DWORD dwWait  = WaitForSingleObject( m_hDataRx, alTimeOut ) ;

				if( dwWait == WAIT_TIMEOUT ) 
				{
					data.erase ();
					hr = E_FAIL;
					return hr;
				}
				
				//ASSERT( dwWait == WAIT_OBJECT_0 );
				if(dwWait != WAIT_OBJECT_0)
					break;
				iRead =  m_theSerialBuffer.Read_N(szTmp, iRemaining, m_hDataRx);
				iRemaining -= iRead ;
				
				if( iRemaining  == 0) 
				{
					data = szTmp;
					return S_OK;
				}
			}
		}
	}
	catch(...)
	{
		//ASSERT ( 0  ) ;
	}
	return hr;
	
}
/*
HRESULT CSerialCommHelper::Read_Upto(std::string& data, char chTerminator, DWORD* alCount, DWORD alTimeOut)
{
	HRESULT hr = CanProcess();
	if ( FAILED(hr)) 
		return hr;

	try
	{
	 	std::string szTmp;
		szTmp.erase ();
		DWORD alBytesRead;
		
		BOOL abFound =  m_theSerialBuffer.Read_Upto(szTmp ,chTerminator,alBytesRead,m_hDataRx );

		if ( abFound ) 
		{
			data = szTmp ;
		}
		else
		{//there are either none or less bytes...
			DWORD iRead = 0;
			BOOL abContinue = 1;
			while(abContinue)
			{
				DWORD dwWait  = ::WaitForSingleObject ( m_hDataRx , alTimeOut ) ;
				
				if  ( dwWait == WAIT_TIMEOUT) 
				{
					data.erase ();
					hr = E_FAIL;
					return hr;

				}
   				
				abFound =  m_theSerialBuffer.Read_Upto(szTmp ,chTerminator,alBytesRead,m_hDataRx );
				
				if ( abFound ) 
				{
					data = szTmp;
					return S_OK;
				}
			}
		}
	}
	catch(...)
	{
		ASSERT ( 0  ) ;
	}
	return hr;
	
}
*/
/*-----------------------------------------------------------------------
	-- Reads all the data that is available in the local buffer.. 
	does NOT make any blocking calls in case the local buffer is empty
-----------------------------------------------------------------------*/
/*
HRESULT CSerialCommHelper::ReadAvailable(std::string& data)
{
	HRESULT hr = CanProcess();
	if ( FAILED(hr)) 
		return hr;
	try
	{
		std::string szTemp;
		BOOL abRet = m_theSerialBuffer.Read_Available(szTemp, m_hDataRx);

		data = szTemp;
	}
	catch(...)
	{
		ASSERT(0);
		hr = E_FAIL;
	}
	return hr;
}
*/




