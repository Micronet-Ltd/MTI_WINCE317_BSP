// WaveRecorder.cpp: implementation of the CWaveRecorder class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include <windows.h>
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
	m_hStop			= CreateEvent( NULL, TRUE,	FALSE,	NULL/*_T("Record StopAll")*/ );
	m_PlayThroughStop = CreateEvent( NULL, TRUE,	FALSE,	NULL/*_T("Record StopAll")*/ );
	m_hStopWaiting	= CreateEvent( NULL, TRUE,	FALSE,	NULL/*_T("Record Stop")*/ );
	m_PlayThroughStopWaiting = CreateEvent( NULL, TRUE,	FALSE,	NULL/*_T("Record Stop")*/ );
	m_hRecStarted   = CreateEvent( NULL, FALSE,	FALSE,	NULL/*_T("Record Started")*/ );
	m_PlayThroughRecStarted = CreateEvent( NULL, FALSE,	FALSE,	NULL/*_T("Record Started")*/ );
	m_hStart1		= CreateEvent( NULL, FALSE,	FALSE,	NULL/*_T("Record Start1")*/ );
	m_hStart2		= CreateEvent( NULL, FALSE,	FALSE,	NULL/*_T("Record Start2")*/ );
	m_PlayThroughStart1 = CreateEvent( NULL, FALSE,	FALSE,	NULL/*_T("Record Start1")*/ );
	m_PlayThroughStart2 = CreateEvent( NULL, FALSE,	FALSE,	NULL/*_T("Record Start1")*/ );
	m_hDoneEvent	= CreateEvent( NULL, FALSE,	FALSE,	NULL/*_T("Record Done")*/ );
	m_PlayThroughDoneEvent = CreateEvent( NULL, FALSE,	FALSE,	NULL/*_T("Record Done")*/ );

	m_EventsVector[0] = m_hStop;
	m_EventsVector[1] = m_hDoneEvent;

}

CWaveRecorder::~CWaveRecorder()
{
	Close();
		
	CloseHandle( m_hStop    	);
	CloseHandle( m_PlayThroughStop    	);
	CloseHandle( m_hStopWaiting	);
	CloseHandle( m_PlayThroughStopWaiting	);
	CloseHandle( m_hRecStarted  );	
	CloseHandle( m_PlayThroughRecStarted  );	
	CloseHandle( m_hStart1		);	
	CloseHandle( m_hStart2		);	
	CloseHandle( m_PlayThroughStart1		);	
	CloseHandle( m_PlayThroughStart2		);	
	CloseHandle( m_hDoneEvent	);		
	CloseHandle( m_PlayThroughDoneEvent	);
}
DWORD CWaveRecorder::WaitForAction()
{
	if( !m_hwo )//|| !m_pMgr )
	{
		RETAILMSG(1, (L"DIGVOICE:+%S, invalid handle, m_hwo\r\n",__FUNCTION__)); 
		return -1;
	}
	
	DWORD err(0);//, OutSize(0);
	//TCHAR text[256];

//	TypeOfMsg		MsgType( MSGTYPE_UNKNOWN );
	MMRESULT mmres;
	
	HANDLE EventsVector1[2];
	EventsVector1[0] = m_hStopWaiting;
	EventsVector1[1] = m_hStart1;
	HANDLE EventsVector2[2];
	EventsVector2[0] = m_hStopWaiting;
	EventsVector2[1] = m_hStart2;
	DWORD Rv;

	RETAILMSG(1, (L"DIGVOICE:+%S\r\n",__FUNCTION__)); 

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

		SetEvent( m_hRecStarted );

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

					RETAILMSG(1, (L"DIGVOICE:%S, waveInAddBuffer err %d, Rv = %d\r\n",__FUNCTION__, mmres, Rv)); 
					break;
				}
			}
			else //if( Rv != WAIT_OBJECT_0 + 1 )//error
			{
				waveInReset( m_hwo );
				RETAILMSG(1, (L"DIGVOICE:%S, unknown Rv = %d\r\n",__FUNCTION__, Rv)); 
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

				    RETAILMSG(1, (L"DIGVOICE:%S, waveInAddBuffer err %d, Rv = %d\r\n",__FUNCTION__, mmres, Rv)); 
					break;
				}

			}
			else //if( Rv != WAIT_OBJECT_0 + 1 )//error
			{
			    RETAILMSG(1, (L"DIGVOICE:%S, unknown Rv = %d\r\n",__FUNCTION__, Rv)); 
				break;
			}
		}
	}//__try
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		//memcpy( &EP, GetExceptionInformation(), sizeof(EP) );
		err				= GetExceptionCode();
		RETAILMSG(1, (L"DIGVOICE:%S, exception, err = %d\r\n",__FUNCTION__, err)); 
				//if( AbnormalTermination() )
				//{
				//}
	}

	RETAILMSG(1, (L"DIGVOICE:-%S, quit thread, err = %d\r\n",__FUNCTION__, err)); 
	return err;
}

DWORD CWaveRecorder::WaitForActionPlayThrough()
{
	if( !m_PlayThroughHwo )//|| !m_pMgr )
		return -1;
	
	DWORD err(0);//, OutSize(0);
	TCHAR text[256];

//	TypeOfMsg		MsgType( MSGTYPE_UNKNOWN );
	MMRESULT mmres;
	
	HANDLE EventsVector1[2];
	EventsVector1[0] = m_PlayThroughStopWaiting;
	EventsVector1[1] = m_PlayThroughStart1;
	HANDLE EventsVector2[2];
	EventsVector2[0] = m_PlayThroughStopWaiting;
	EventsVector2[1] = m_PlayThroughStart2;
	DWORD Rv;
//	EXCEPTION_POINTERS EP = {0};
	__try
	{
#if 0
		mmres = waveOutWrite( m_PlayThroughHwo, (WAVEHDR*)m_PlayThroughpWhdr1, sizeof(WAVEHDR));	
		if(mmres != MMSYSERR_NOERROR)
			return mmres;
		mmres = waveOutWrite( m_PlayThroughHwo, (WAVEHDR*)m_PlayThroughpWhdr2, sizeof(WAVEHDR));	
		if(mmres != MMSYSERR_NOERROR)
			return mmres;
#endif
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
				//Prepare(m_PlayThroughpWhdr1, TRUE, m_pWhdr1->lpData, m_BufferLength );

				mmres = waveOutWrite( m_PlayThroughHwo, (WAVEHDR*)m_PlayThroughpWhdr1, sizeof(WAVEHDR));	
				if(mmres != MMSYSERR_NOERROR)
				{
					waveOutReset( m_PlayThroughHwo );

					wsprintf( text, __T("waveOutWrite err %d, Rv = %d\r\n"), mmres, Rv );
					OutputDebugString( text );
					break;
				}

		        WaitForDone(m_PlayThroughpWhdr1, TRUE );
		        waveOutUnprepareHeader(m_PlayThroughHwo, (WAVEHDR*)m_PlayThroughpWhdr1, sizeof(WAVEHDR));	
			}
			else //if( Rv != WAIT_OBJECT_0 + 1 )//error
			{
				waveOutReset( m_PlayThroughHwo );

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
				//Prepare(m_PlayThroughpWhdr2, TRUE, m_pWhdr2->lpData, m_BufferLength );

				mmres = waveOutWrite( m_PlayThroughHwo, (WAVEHDR*)m_PlayThroughpWhdr2, sizeof(WAVEHDR));	
				if(mmres != MMSYSERR_NOERROR)
				{
					waveOutReset( m_PlayThroughHwo );

					wsprintf( text, __T("waveOutWrite err %d, Rv = %d\r\n"), mmres, Rv );
					OutputDebugString( text );
					break;
				}

		        WaitForDone(m_PlayThroughpWhdr2, TRUE );
		        waveOutUnprepareHeader(m_PlayThroughHwo, (WAVEHDR*)m_PlayThroughpWhdr2, sizeof(WAVEHDR));	
			}
			else //if( Rv != WAIT_OBJECT_0 + 1 )//error
			{
				waveOutReset( m_PlayThroughHwo );

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

