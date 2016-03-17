// Protocol.h: interface for the CProtocol class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROTOCOL_H__1DAFDAC5_F77B_408A_B110_F98B64CB6814__INCLUDED_)
#define AFX_PROTOCOL_H__1DAFDAC5_F77B_408A_B110_F98B64CB6814__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "BaseTransport.h"
#include "SerLoaderApi.h"

class CTransportNotify;
unsigned long CalcCS( const void *buf, int len);

class CProtocol
{
public:
	
	CProtocol( CBaseTransport* pTransport ) :	m_pTransport( pTransport ),
												m_WaitTimeout(10000)	{}
	
	virtual ~CProtocol();

	DWORD	Put( HANDLE hPort, DWORD Cmd, const	BYTE* pData, DWORD DataSize, DWORD Param = 0		) const;
	DWORD	Get( HANDLE hPort, DWORD Cmd,		BYTE* pData, DWORD DataMaxSize, DWORD& OutSize, DWORD Param = 0	) const;
	DWORD	PutCmd( HANDLE hPort, DWORD cmd, DWORD Param						) const;
	DWORD	GetCmd( HANDLE hPort, BYTE* pData, DWORD DataMaxSize, DWORD& OutSize		) const;

	DWORD	Open( const TCHAR* DeviceName, HANDLE& hPort, CTransportNotify*	m_pProtocolNotifier = NULL );
	DWORD	Close( HANDLE hPort );
	DWORD	ConnectAuto( HANDLE hPort );
	DWORD	ConnectOld( HANDLE hPort );
	DWORD	GetStart( HANDLE hPort, DWORD Timeout ) const;

	DWORD	GetWaitTimeout() const			{ return m_WaitTimeout; }
	void	SetWaitTimeout( DWORD Timeout ) { m_WaitTimeout = Timeout; }
//	DWORD	GetJunk( HANDLE hPort, BYTE* pData, DWORD DataMaxSize, DWORD& OutSize )//temp!!!
//	{
//		return m_Transport->Get( hPort, pData, DataMaxSize, OutSize );
//	}
	void ClearReceive(HANDLE hPort);

	HANDLE			m_hPort;//temp!!! work around
	CBaseTransport*	m_pTransport;
private:
	DWORD	CheckData( BYTE* pBuffer, DWORD DataLen ) const;
//	DWORD	GetWithAck(	HANDLE hPort, 		BYTE* pData, DWORD DataSize, DWORD& DataOutSize ) const;
	DWORD	PutWithAck( HANDLE hPort, const	BYTE* pData, DWORD DataSize )	const;
	DWORD	GetCmdWithAck(	HANDLE hPort, 		BYTE* pData, DWORD DataSize, DWORD& DataOutSize ) const;
	DWORD	PutCmdWithAck( HANDLE hPort, const	BYTE* pData, DWORD DataSize )	const;
	
//	CBaseTransport*	m_pTransport;
	
	mutable BYTE	m_Buffer[MAX_BUFFER_SIZE];
	DWORD			m_WaitTimeout;
	CProtocol( const CProtocol& );//
};

#endif // !defined(AFX_PROTOCOL_H__1DAFDAC5_F77B_408A_B110_F98B64CB6814__INCLUDED_)
