#ifndef		_SER_TRANSPORT_H
#define		_SER_TRANSPORT_H

#include "SerialCommHelper.h"
#include "BaseTransport.h"


class	CTransportNotify;

class	CSerTransport : public CBaseTransport
{
public:
	CSerTransport() : m_pNotifier(0), m_hPort((HANDLE)-1) {}

	virtual ~CSerTransport() {}

	virtual DWORD Open( const TCHAR* DeviceName, HANDLE& hPort, CTransportNotify* pNotifier )
	{

		if( (HANDLE)-1 != (hPort = m_SerPort.Init( DeviceName, CBR_115200 )) )
		{
			m_pNotifier	= pNotifier;
			m_hPort = hPort;
			m_SerPort.Start();
			return 0;
		}

		return -1;
	}

	virtual DWORD Close(  HANDLE hPort )
	{
		if( (HANDLE)-1 == m_hPort || hPort != m_hPort )
			return -1;
		m_SerPort.UnInit();
		m_pNotifier = 0;

		return 0;
	}

	virtual DWORD GetEsc( HANDLE hPort,	BYTE* pBuffer, DWORD BufferSize, DWORD& OutSize, DWORD Timeout	)
	{
		if( (HANDLE)-1 == m_hPort || m_hPort != hPort || !pBuffer )
			return -1;
		std::string szTemp;
		
		BYTE	esc = 0;
		DWORD	err;
		BYTE	Tmp;

		OutSize = BufferSize;
		
		while(BufferSize--)
		{
			if( S_OK != (err = m_SerPort.Read_N( szTemp, 1, Timeout) ))
					break;
		
			Tmp = szTemp[0];
			if( ESC_CHAR == esc )
			{
				*pBuffer++ = Tmp + 2;
				esc = 0;
			}
			else if( ESC_CHAR == Tmp	)
			{
				esc = ESC_CHAR;//for next
				BufferSize++;
			}
			else
				*pBuffer++ = Tmp;
		}
		
		if( S_OK == err )
			return 0;

		OutSize = 0;
		return -1;
	}
	virtual DWORD PutEsc( HANDLE hPort, const	BYTE* pBuffer, DWORD BufferSize	)
	{
		if( (HANDLE)-1 != m_hPort && m_hPort == hPort && pBuffer )
		{
			DWORD Len = 0;
			for(DWORD i = 0; i < BufferSize; ++i )//temp!!!! check max size!!!
			{
				if( FROM_OEM_DEBUG_READ_NODATA	== pBuffer[i]	||
					FROM_OEM_DEBUG_COM_ERROR	== pBuffer[i]	||
					START_CHAR					== pBuffer[i]	||
					ESC_CHAR					== pBuffer[i]	)
				{
					m_Buffer[Len++] = ESC_CHAR;
					m_Buffer[Len++] = pBuffer[i] - 2;
				}
				else
					m_Buffer[Len++] = pBuffer[i];

			}

			if( S_OK == m_SerPort.Write(m_Buffer, Len) )
				return 0;
		}
		return -1;
	}

	virtual DWORD Put( HANDLE hPort, const	BYTE*	pBuffer, DWORD BufferSize ) 
	{ 
		if( (HANDLE)-1 != m_hPort && m_hPort == hPort && pBuffer )
		{
			if( S_OK == m_SerPort.Write(pBuffer, BufferSize) )
				return 0;
		}
		return -1;
	}
	virtual DWORD Get( HANDLE hPort, BYTE*	pBuffer, DWORD BufferSize, DWORD& OutSize )		
	{
		if( (HANDLE)-1 == m_hPort || m_hPort != hPort || !pBuffer )
			return -1;

		std::string szTemp;
		
		BYTE	esc = 0;
		DWORD	err;

		if( S_OK == (err = m_SerPort.Read_N( szTemp, BufferSize, 1000) ))//temp!!! time
		{
			OutSize = BufferSize;
		}

		return err;
	}
	virtual void ClearReceive() 
	{
		m_SerPort.ClearReceive();
	}

private:
	CTransportNotify*	m_pNotifier;
	CSerialCommHelper	m_SerPort;
	HANDLE				m_hPort;//for cheking
	mutable BYTE		m_Buffer[1024*4];
};

#endif