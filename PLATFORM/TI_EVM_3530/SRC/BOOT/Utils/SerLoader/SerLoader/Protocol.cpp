// Protocol.cpp: implementation of the CProtocol class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Protocol.h"
#include "bootcmd.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
unsigned long CalcCS( const void *buf, int len)
{
	unsigned long cs=0;
	unsigned char *p=(unsigned char*)buf;
	while(len--)
	{
		cs+=*p++;
	}
	return cs;
}
unsigned long Ack2Error(unsigned long ack)
{
	unsigned long err;
	switch(ack)
	{
		case ACK_NO_ERROR:
			err = FOK;
		break;
		case ACK_WRONG_ARGUMENT:     
			err = FERROR_BAD_PARAMETERS;
		break;
		case ACK_ERROR_FLASH: 		
			err = FERROR_FLASHERROR;
		break;
		case ACK_UNKNOWN_CMD:     	
			err = FERROR_BAD_PARAMETERS;
		break;
		case ACK_CORRUPTED_CMD:    	
			err = FERROR_CORRUPTED_CMD;
		break;
		case ACK_IMAGE_NOT_BURNED:	
			err = FERROR_IMAGE_NOT_BURNED;
		break;
		case ACK_VERSION_NOT_FOUND:	
			err = FERROR_VERSION_NOT_FOUND;
		break;
		case ACK_GENERAL_ERROR:		
			err = FERROR_GENERAL;
		break;
		case ACK_PARTITION_ERROR:	
			err = FERROR_FLASHERROR;
		break;
		case ACK_BLOCK_STATUS_BAD:	
			err = FERROR_FLASHERROR;
		break;
		case ACK_NOT_EXIST:			
			err = FERROR_BADBINFILE;
		break;
		case ACK_NOT_IMPLEMENTED:	
			err = FERROR_NOT_SUPPORTED;
		break;
		case ACK_WRONG_SIZE:	
			err = FERROR_SIZE;
		break;
		default:
			err = FERROR_ACK;
		break;
	}
	return err;
}
CProtocol::~CProtocol()
{
}
DWORD CProtocol::Open( const TCHAR* DeviceName, HANDLE& hPort, CTransportNotify*	pProtocolNotifier ) 
{
	DWORD err;
	err = m_pTransport->Open( DeviceName, hPort, pProtocolNotifier );
	m_hPort = hPort;
	return err;
}
DWORD CProtocol::Close( HANDLE hPort )
{ 
	return m_pTransport->Close( hPort );		
}

DWORD CProtocol::Put( HANDLE hPort, DWORD Cmd, const BYTE* pData, DWORD DataSize, DWORD Param	) const
{
	if( !pData && DataSize )
		return FERROR_PARAM;

	DWORD err(0);
	BYTE Start = START_CHAR;
	ProtocolHeader* pHeader = (ProtocolHeader*)m_Buffer;
	
	DWORD TotalSize(0), CurSize(0);

	do
	{
		memset( pHeader, 0, sizeof(ProtocolHeader) );
		
		pHeader->HeaderSize	= sizeof(ProtocolHeader); 
		pHeader->Cmd		= Cmd; 
		pHeader->Param		= Param;

		if( DataSize - TotalSize > MAX_BUFFER_SIZE - sizeof(ProtocolHeader) )
		{
			pHeader->fContinueFlag	=  1;
			CurSize					= MAX_BUFFER_SIZE;
		}
		else
		{
			pHeader->fContinueFlag	= 0;
			CurSize					= DataSize - TotalSize + sizeof(ProtocolHeader);
		}

		if( pData && DataSize )
			memcpy( m_Buffer + sizeof(ProtocolHeader), pData + TotalSize, CurSize - sizeof(ProtocolHeader) );
		
		pHeader->DataLen	= CurSize - sizeof(ProtocolHeader);
		pHeader->DataCS		= CalcCS( m_Buffer + sizeof(ProtocolHeader), CurSize - sizeof(ProtocolHeader) );
		pHeader->HeaderCS	= CalcCS( pHeader, sizeof(ProtocolHeader) - 4 );

		err = m_pTransport->Put( hPort, &Start, 1 );
		
		if( err = PutWithAck( hPort, m_Buffer, CurSize ) )
			break;

		TotalSize += ( CurSize - sizeof(ProtocolHeader) ); 
	}
	while( TotalSize < DataSize );

	return err;
}
/////////////////////////////////////////////////////////////////////////////////////////
DWORD CProtocol::PutWithAck( HANDLE hPort, const BYTE* pData, DWORD DataSize ) const
{
	DWORD err(FOK), AckLen(0);
	ProtocolHeader	Ack = {0};

	if( err = m_pTransport->PutEsc( hPort, pData, DataSize ) )
		return err;

	if( err = GetStart( hPort, m_WaitTimeout ) )
		return err;

	if( err = m_pTransport->GetEsc( hPort, (BYTE*)&Ack, sizeof(Ack), AckLen, m_WaitTimeout ) )
		return err;//temp!!!! can be retry 

	if(FOK	!= CheckData( (BYTE*)&Ack, sizeof(Ack) ))
	{
		err = FERROR_DATA;
	}
	else if(ACK_NO_ERROR != Ack.Ack)
		err = Ack2Error(Ack.Ack);
	return err;
}
/////////////////////////////////////////////////////////////////////////////////////////
DWORD CProtocol::GetCmd( HANDLE hPort, BYTE* pData, DWORD DataSize, DWORD& DataOutSize ) const
{
	DWORD	err(0);
	DWORD	OutSize(0);

	if( err = m_pTransport->GetEsc( hPort, pData, DataSize, OutSize, m_WaitTimeout ) )
		return err;

	if( DataSize != OutSize )
		return FERROR_COMM;

	DataOutSize = OutSize;

	return err;
}
//////////////////////////////////////////////////////////////////////////////////////////
DWORD CProtocol::GetStart( HANDLE hPort, DWORD Timeout ) const
{
	DWORD	err = -1, OutSize;
	BYTE	strb = 0;
	DWORD	time = GetTickCount() + Timeout;
	while( GetTickCount() < time )
	{
		if( err = m_pTransport->GetEsc( hPort, &strb, 1, OutSize, Timeout ) )
			continue;
		if( START_CHAR == strb )
			break;
	}
	if( START_CHAR == strb )
		return 0;
	return FERROR_TIMEOUT;
}
DWORD CProtocol::ConnectAuto( HANDLE hPort )
{
	DWORD	err(1), OutSize(0);
	BYTE	doboot[] = "\r\nBOOT\r\n";
	char	okdo[] = "\r\nOK\r\n";
	BYTE	Buff[32] = {0};

	DWORD	time = GetTickCount() + m_WaitTimeout;

	while(err && GetTickCount() < time)
	{
		if(err = m_pTransport->Put(hPort, doboot, (DWORD)strlen((char*)doboot) ))
			return err;
		if(err = GetStart(hPort, 10))
			continue;

		if(err = GetCmd(hPort, Buff, 6, OutSize))
			break;

		if(err = strcmp((char*)Buff, okdo))
			err = -1;
		break;
	}
	return err;
}


DWORD CProtocol::ConnectOld( HANDLE hPort )
{
	DWORD err, OutSize(0);
	BYTE Start = START_CHAR;

	if( err = GetStart( hPort, m_WaitTimeout ) )
		return err;

	err = m_pTransport->Put( hPort, &Start, 1 );
	ProtocolHeader* pHeader = (ProtocolHeader*)m_Buffer;
	if( err = GetCmd( hPort, m_Buffer, sizeof(ProtocolHeader), OutSize ) )
		return err;

	if( pHeader->HeaderSize	!= sizeof(ProtocolHeader)	||
		pHeader->Cmd		!= CMD_SEND_BOOTME			||
		pHeader->HeaderCS	!= CalcCS(pHeader, sizeof(ProtocolHeader) - 4)	)
	{
		err = FERROR_READ;
		return err;
	}
	return err;
}

DWORD CProtocol::Get( HANDLE hPort, DWORD Cmd, BYTE* pData, DWORD DataSize, DWORD& GlobalOutSize, DWORD Param ) const
{
	GlobalOutSize = 0;

	if( !pData || !DataSize )//|| DataSize < sizeof(ProtocolHeader) )
		return FERROR_PARAM;

	BYTE Start = START_CHAR;
	DWORD err(0);
	ProtocolHeader* pHeader = (ProtocolHeader*)m_Buffer;
	
	DWORD	CurSize(0), OutSize(0), TotalDataSize(0);
	
	ProtocolHeader Header = {0};
	
	Header.Param		= Param;
	Header.HeaderSize	= sizeof(ProtocolHeader); 
	Header.Cmd			= Cmd; 
	Header.HeaderCS		= CalcCS( &Header, sizeof(ProtocolHeader) - 4 );
	
	err = m_pTransport->Put( hPort, &Start, 1 );

	if( err = m_pTransport->PutEsc( hPort, (BYTE*)&Header, sizeof(ProtocolHeader) ) )
		return err;

	do
	{
		OutSize = 0;
		CurSize = sizeof(ProtocolHeader);

		BYTE strb = 0;
		if( err = GetStart( hPort, m_WaitTimeout ) )
			break;
		
		if( err = GetCmd( hPort, m_Buffer, CurSize, OutSize ) )
			break;

		if( Header.Cmd		!= pHeader->Cmd		||
			0				== pHeader->DataLen	||
			DataSize		<  pHeader->DataLen	||
			pHeader->HeaderCS != CalcCS(pHeader, sizeof(ProtocolHeader) - 4)	)
		{
			if(ACK_NO_ERROR	!= pHeader->Ack)
				err = Ack2Error(pHeader->Ack);
			else
				err = FERROR_READ;
			break;
		}
		if(ACK_NO_ERROR	!= pHeader->Ack)
		{
			err = Ack2Error(pHeader->Ack);
			break;
		}

		CurSize	= pHeader->DataLen;
		OutSize = 0;
		////////////////////////
		if( err = GetCmd( hPort, m_Buffer + sizeof(ProtocolHeader), CurSize, OutSize ) )
			break;

		if(	err = CheckData( m_Buffer, OutSize + sizeof(ProtocolHeader) )	||
			OutSize != CurSize												)
		{
			err = FERROR_OVERFLOW;
			break;
		}

		memcpy( pData + TotalDataSize, m_Buffer + sizeof(ProtocolHeader), OutSize );
		
		TotalDataSize	+= OutSize;
	}
	while( TotalDataSize < DataSize && pHeader->fContinueFlag );

	GlobalOutSize = TotalDataSize;

	return err;
}
///////////////////////////////////////////////////////////////////////////////////////
/*DWORD CProtocol::GetCmd( HANDLE hPort, BYTE* pData, DWORD DataSize, DWORD& GlobalOutSize ) const
{
	if( !pData || !DataSize )//|| DataSize < sizeof(ProtocolHeader) )
		return FERROR_PARAM;

	DWORD err(0);

	err = m_pTransport->GetEsc( hPort, pData, DataSize, GlobalOutSize );

	return err;
}*/
/////////////////////////////////////////////////////////////////////////////////////////

DWORD CProtocol::PutCmd( HANDLE hPort, DWORD Cmd, DWORD Param	) const
{
	BYTE Start				= START_CHAR;
	ProtocolHeader Header	= {0};
	
	Header.HeaderSize	= sizeof(ProtocolHeader); 
	Header.Cmd			= Cmd;
	Header.Param		= Param;
	Header.HeaderCS		= CalcCS( &Header, sizeof(ProtocolHeader) - 4 );

	DWORD	err = m_pTransport->Put( hPort, &Start, 1 );
	err = m_pTransport->PutEsc( hPort, (BYTE*)&Header, sizeof(ProtocolHeader) );

	return err;
}
/////////////////////////////////////////////////////////////////////////////////////////
DWORD CProtocol::CheckData( BYTE* pBuffer, DWORD BufferLen ) const
{
	if( !pBuffer						|| 
		!BufferLen						|| 
		sizeof(ProtocolHeader) > BufferLen	)
	{
		return FERROR_DATA;
	}

	DWORD err(0);

	ProtocolHeader* pHeader = (ProtocolHeader*)pBuffer;
	
	if( pHeader->HeaderCS	!= CalcCS(pBuffer, sizeof(ProtocolHeader) - 4)	)//||
//		pHeader->DataLen	!= BufferLen - sizeof(ProtocolHeader)	)
	{
		return FERROR_DATA;
	}

	if(		BufferLen > sizeof(ProtocolHeader)	&&
			pHeader->DataLen					&& 
		(	pHeader->DataCS	 != CalcCS( pBuffer + sizeof(ProtocolHeader), BufferLen - sizeof(ProtocolHeader) ) ) )
	{
		return FERROR_DATA;
	}

	return FOK;
}
void CProtocol::ClearReceive(HANDLE hPort)
{
	m_pTransport->ClearReceive();
}
