// WaveRecorder.cpp: implementation of the CWaveRecorder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WaveRecorder.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWaveRecorder::CWaveRecorder() :	m_hwo(0), 
									m_buff1(0), m_buff2(0), 
									m_RecBufferTime	( REC_BUFF_TIME ),	
									m_FileBigSize	( FILE_BIG_LEN )
{		
	memset( &m_WaveFormatEx, 0, sizeof(WAVEFORMATEX) );
	m_hStop			= CreateEvent( FALSE, TRUE,		FALSE,	_T("Record StopAll") );
	m_hStopWaiting	= CreateEvent( FALSE, TRUE,		FALSE,	_T("Record Stop") );
	m_hStart1		= CreateEvent( FALSE, FALSE,	FALSE,	_T("Record Start1") );
	m_hStart2		= CreateEvent( FALSE, FALSE,	FALSE,	_T("Record Start2") );
	m_hDoneEvent	= CreateEvent( FALSE, FALSE,	FALSE,	_T("Record Done"));

	m_EventsVector[0] = m_hStop;
	m_EventsVector[1] = m_hDoneEvent;

}

CWaveRecorder::~CWaveRecorder()
{
	Close();
		
	CloseHandle( m_hStop	);
	CloseHandle( m_hStopWaiting	);
	CloseHandle( m_hStart1		);	
	CloseHandle( m_hStart2		);	
	CloseHandle( m_hDoneEvent	);		
}
DWORD CWaveRecorder::WaitForAction()
{
	if( !m_hwo )//|| !m_pMgr )
		return -1;
	
	DWORD err(0);//, OutSize(0);
	TCHAR text[256];

//	TypeOfMsg		MsgType( MSGTYPE_UNKNOWN );
	MMRESULT mmres;
	
	HANDLE EventsVector1[2];
	EventsVector1[0] = m_hStopWaiting;
	EventsVector1[1] = m_hStart1;
	HANDLE EventsVector2[2];
	EventsVector2[0] = m_hStopWaiting;
	EventsVector2[1] = m_hStart2;
	DWORD Rv;
//	EXCEPTION_POINTERS EP = {0};
	__try
	{
		mmres = waveInAddBuffer( m_hwo, (WAVEHDR*)m_pWhdr1, sizeof(WAVEHDR));	
		if(mmres != MMSYSERR_NOERROR)
			return mmres;
		mmres = waveInAddBuffer( m_hwo, (WAVEHDR*)m_pWhdr2, sizeof(WAVEHDR));	
		if(mmres != MMSYSERR_NOERROR)
			return mmres;
		
		mmres = waveInStart( m_hwo );
		if(mmres != MMSYSERR_NOERROR)
			return mmres;

		while(1)
		{
//			Rv = WaitForMultipleObjects( 3, EventsVector, FALSE, INFINITE );
			Rv = WaitForMultipleObjects( 2, EventsVector1, FALSE, INFINITE );
			if( Rv == WAIT_OBJECT_0 )//stop
			{
				//err = waveInStop( m_hwo );
				break;
			}
			else if( Rv == WAIT_OBJECT_0 + 1 )
			{

				mmres = waveInAddBuffer( m_hwo, (WAVEHDR*)m_pWhdr1, sizeof(WAVEHDR));	
				if(mmres != MMSYSERR_NOERROR)
				{
					waveInReset( m_hwo );

					wsprintf( text, __T("waveInAddBuffer err %d, Rv = %d\r\n"), mmres, Rv );
					OutputDebugString( text );
					break;
				}
			}
			else //if( Rv != WAIT_OBJECT_0 + 1 )//error
			{
				waveInReset( m_hwo );
				
				wsprintf( text, __T("unknown Rv = %d\r\n"), Rv );
				OutputDebugString( text );
				break;
			}
//			else if( Rv == WAIT_OBJECT_0 + 2 )
			Rv = WaitForMultipleObjects( 2, EventsVector2, FALSE, INFINITE );
			if( Rv == WAIT_OBJECT_0 )//stop
			{
				//err = waveInStop( m_hwo );
				break;
			}
			else if( Rv == WAIT_OBJECT_0 + 1 )
			{
				mmres = waveInAddBuffer( m_hwo, (WAVEHDR*)m_pWhdr2, sizeof(WAVEHDR));	
				if(mmres != MMSYSERR_NOERROR)
				{
					waveInReset( m_hwo );

					wsprintf( text, __T("waveInAddBuffer err %d, Rv = %d\r\n"), mmres, Rv );
					OutputDebugString( text );
					break;
				}

			}
			else //if( Rv != WAIT_OBJECT_0 + 1 )//error
			{
				wsprintf( text, __T("unknown Rv = %d\r\n"), Rv );
				OutputDebugString( text );
				break;
			}
		}
	}//__try
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		//memcpy( &EP, GetExceptionInformation(), sizeof(EP) );
		err				= GetExceptionCode();
				//if( AbnormalTermination() )
				//{
				//}
	}
	
	return err;
}

