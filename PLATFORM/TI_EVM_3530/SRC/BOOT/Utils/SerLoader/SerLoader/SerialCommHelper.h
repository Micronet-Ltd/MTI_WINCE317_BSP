// SerialCommHelper.h: interface for the CSerialCommHelper class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "SerialBuffer.h"
//#include "DebugDump.h"
//#include <map>
#include <TCHAR.H>

#ifndef _UNICODE
#define		tstring		string	
#else
#define		tstring		wstring
#endif

typedef enum tagSERIAL_STATE
{
	SS_Unknown,
	SS_UnInit,
	SS_Init,
	SS_Started ,
	SS_Stopped ,
	
} SERIAL_STATE;

class CSerialCommHelper  
{
public:
	CSerialCommHelper();
	virtual ~CSerialCommHelper();
	HANDLE	GetWaitForEvent() {return m_hDataRx;} 

	inline void		LockThis()			{ EnterCriticalSection ( &m_csLock );		}	
	inline void		UnLockThis()		{ LeaveCriticalSection (&m_csLock);			}
	inline void		InitLock()			{ InitializeCriticalSection (&m_csLock );	}
	inline void		DelLock()			{ DeleteCriticalSection (&m_csLock );		}
 	inline BOOL		IsInputAvailable()
	{
		LockThis (); 
		BOOL abData = ( !m_theSerialBuffer.IsEmpty() ) ;
		UnLockThis (); 
		return abData;
	} 
	inline BOOL		IsConnection()		{ return m_abIsConnected ;}
 	inline void		SetDataReadEvent()	{ SetEvent ( m_hDataRx );	}
	

	HRESULT			Read_N		(std::string& data, DWORD alCount, DWORD alTimeOut);
	HRESULT			Write(const BYTE* data, DWORD dwSize) const;
	HANDLE			Init( std::tstring szPortName= _T("COM1"), DWORD dwBaudRate = 9600,BYTE byParity = 0,BYTE byStopBits = 1,BYTE byByteSize  = 8);
	HRESULT			Start();
	HRESULT			Stop();
	HRESULT			UnInit();

//	HRESULT			Read_Upto	(std::string& data, char chTerminator, DWORD* alCount, DWORD alTimeOut) ;
//	HRESULT			ReadAvailable(std::string& data);

	SERIAL_STATE GetCurrentState() {return m_eState;}
	void	CloseAndCleanHandle(HANDLE& hHandle) ;
	void	ClearReceive()
	{
		m_theSerialBuffer.Clear();
	}
	static unsigned __stdcall ThreadFn(void*pvParam);

	HRESULT  CanProcess() const;
private:
	SERIAL_STATE	m_eState;
	HANDLE	m_hCommPort;
	HANDLE	m_hThreadTerm ;
	HANDLE	m_hThread;
	HANDLE	m_hThreadStarted;
	HANDLE	m_hDataRx;
	HANDLE	m_hWriteEvent;

	BOOL	m_abIsConnected;
	
	mutable CSerialBuffer m_theSerialBuffer;
	CRITICAL_SECTION m_csLock;	
};


