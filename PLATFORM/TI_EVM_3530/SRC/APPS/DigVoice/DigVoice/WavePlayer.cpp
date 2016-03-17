// WavePlayer.cpp: implementation of the CWavePlayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WavePlayer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWavePlayer::CWavePlayer() : m_hwo(0)//, m_pWaveFile(0)
{
	m_hStop			= CreateEvent( FALSE, TRUE,		FALSE,	_T("Play Stop All") );
	m_hStopWaiting	= CreateEvent( FALSE, TRUE,		FALSE,	_T("Play Stop") );
	m_hStart1		= CreateEvent( FALSE, FALSE,	FALSE,	_T("Play Start1") );
	m_hStart2		= CreateEvent( FALSE, FALSE,	FALSE,	_T("Play Start2") );
	m_hDoneEvent	= CreateEvent( FALSE, FALSE,	FALSE,	_T("Play Done"));
}

CWavePlayer::~CWavePlayer()
{
	Close();

	CloseHandle( m_hStop		);
	CloseHandle( m_hStopWaiting	);
	CloseHandle( m_hStart1		);	
	CloseHandle( m_hStart2		);	
	CloseHandle( m_hDoneEvent	);		
}
DWORD CWavePlayer::WaitForAction()
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
		mmres = waveOutWrite( m_hwo, (WAVEHDR*)m_pWhdr1, sizeof(WAVEHDR));	
		if(mmres != MMSYSERR_NOERROR)
			return mmres;
		mmres = waveOutWrite( m_hwo, (WAVEHDR*)m_pWhdr2, sizeof(WAVEHDR));	
		if(mmres != MMSYSERR_NOERROR)
			return mmres;
		while(1)
		{
//			Rv = WaitForMultipleObjects( 3, EventsVector, FALSE, INFINITE );
			Rv = WaitForMultipleObjects( 2, EventsVector1, FALSE, INFINITE );
			if( Rv == WAIT_OBJECT_0 )//stop
			{
				break;
			}
			else if( Rv == WAIT_OBJECT_0 + 1 )
			{

				mmres = waveOutWrite( m_hwo, (WAVEHDR*)m_pWhdr1, sizeof(WAVEHDR));	
				if(mmres != MMSYSERR_NOERROR)
				{
					waveOutReset( m_hwo );

					wsprintf( text, __T("waveOutWrite err %d, Rv = %d\r\n"), mmres, Rv );
					OutputDebugString( text );
					break;
				}
			}
			else //if( Rv != WAIT_OBJECT_0 + 1 )//error
			{
				waveOutReset( m_hwo );

				wsprintf( text, __T("unknown Rv = %d\r\n"), Rv );
				OutputDebugString( text );
				break;
			}
//			else if( Rv == WAIT_OBJECT_0 + 2 )
			Rv = WaitForMultipleObjects( 2, EventsVector2, FALSE, INFINITE );
			if( Rv == WAIT_OBJECT_0 )//stop
			{
				break;
			}
			else if( Rv == WAIT_OBJECT_0 + 1 )
			{
				mmres = waveOutWrite( m_hwo, (WAVEHDR*)m_pWhdr2, sizeof(WAVEHDR));	
				if(mmres != MMSYSERR_NOERROR)
				{
					waveOutReset( m_hwo );

					wsprintf( text, __T("waveOutWrite err %d, Rv = %d\r\n"), mmres, Rv );
					OutputDebugString( text );
					break;
				}

			}
			else //if( Rv != WAIT_OBJECT_0 + 1 )//error
			{
				waveOutReset( m_hwo );

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

