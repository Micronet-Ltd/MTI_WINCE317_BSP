
#ifndef		_BASE_TRANSPORT_H
#define		_BASE_TRANSPORT_H

class	CTransportNotify;

#define		ESC_CHAR					0xA5
#define		START_CHAR					0xB8
#define		FROM_OEM_DEBUG_READ_NODATA  0xFF
#define		FROM_OEM_DEBUG_COM_ERROR    0xFE

#define MAX_BUFFER_SIZE 4*1024 //4KB

class	CBaseTransport
{
public:
	virtual ~CBaseTransport() {}

	virtual DWORD Open( const TCHAR* DeviceName, HANDLE& hPort, CTransportNotify* pNotifier )	= 0;
	virtual DWORD Close(  HANDLE hPort )														= 0;

	virtual DWORD PutEsc( HANDLE hPort, const	BYTE* pBuffer, DWORD BufferSize					) {return -1;}
	virtual DWORD GetEsc( HANDLE hPort,			BYTE* pBuffer, DWORD BufferSize, DWORD& OutSize, DWORD Timeout	) {return -1;}
	virtual DWORD Put( HANDLE hPort, const	BYTE*	pBuffer,	DWORD BufferSize				) = 0;
	virtual DWORD Get( HANDLE hPort, 		BYTE*	pBuffer,	DWORD BufferSize, DWORD& OutSize) = 0;
	virtual void ClearReceive() {}
};

#endif