// WaveRecorder.h: interface for the CWaveRecorder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVERECORDER_H__43836CF1_EF08_44C4_8992_27E85F38BEB3__INCLUDED_)
#define AFX_WAVERECORDER_H__43836CF1_EF08_44C4_8992_27E85F38BEB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include <windows.h>
#include "wavefile.h"
#include <windowsx.h>

#define REC_BUFF_TIME		3000
#define FILE_BIG_LEN		1024*1024

class CWaveRecorder  
{
public:
	CWaveRecorder();
	virtual ~CWaveRecorder();

unsigned int Open( WAVEFORMATEX* format )
{
//	WAVEHDR whdr;
	unsigned int err(0);

	ResetEvent( m_hDoneEvent	);
	ResetEvent( m_hStart1 );
	ResetEvent( m_hStart2 );
	m_hwo = 0;

	for( unsigned int devId = 0; devId < waveInGetNumDevs(); devId++)
	{
		err = waveInOpen( &m_hwo, devId, format, (DWORD_PTR)m_hDoneEvent, 0, CALLBACK_EVENT);
		if(err == MMSYSERR_NOERROR)
		{
			break;
		}
	}
//	err = waveInOpen( &m_hwo, WAVE_MAPPER, format, (DWORD_PTR)m_hDoneEvent, 0, CALLBACK_EVENT);
	
	if( err != MMSYSERR_NOERROR )
	{
		m_hwo = 0;
	}
	return	err;
}
void InitParams( unsigned int RecBufferTime, unsigned int FileBigSize )
{
	m_RecBufferTime	= RecBufferTime;
	m_FileBigSize	= FileBigSize;	
}
unsigned int Start( const TCHAR* FileName, WAVEFORMATEX* pFormat )
{
	memcpy( &m_WaveFormatEx, pFormat, sizeof(WAVEFORMATEX) );
	if( !m_WaveFile.create( FileName, pFormat ) )
		return -1;
	
	return Start( &m_WaveFile );
}
unsigned int Start( wav_file* waveFile )
{
	unsigned int	err(0);

	err = Open( &m_WaveFormatEx );
	
	if( !err )
	{
		err = record( m_hwo );
	}
	return err; 
}
void	Stop()
{
	unsigned int err(0);

	if( m_hwo )
	{
//		err = waveInStop( m_hwo );

//		SetEvent( m_hStop );
		SetEvent( m_hStopWaiting );

		unsigned int err(0);
	
		if( m_PrepThread )
		{
			err = waveInReset( m_hwo );
			err = waveInStop( m_hwo );

			WaitForSingleObject( m_PrepThread, INFINITE );
		
			CloseHandle( m_PrepThread );
			m_PrepThread = NULL;
		}
	}
}
unsigned int record( HWAVEIN hwo )
{
	unsigned int	err(0);
	

	ResetEvent( m_hStopWaiting	);
	ResetEvent( m_hStop			);

	m_WorkingThread	= CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)WaitForActionThreadProc,this,CREATE_SUSPENDED,0);
	if( !m_WorkingThread )
	{
		StopWork();
	}
	m_PrepThread	= CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)PrepActionThreadProc,this,0,0);
	if( !m_PrepThread )
	{
		StopWork();
	}

	return err;
}
unsigned int StopWork()
{
	unsigned int err(0);

//	if( m_hStopWaiting )
//		SetEvent( m_hStopWaiting );

	if( m_hwo )
	{

//		err = waveInReset( m_hwo );
//		err = waveInStop( m_hwo );

		if( m_WorkingThread )
		{
			WaitForSingleObject( m_WorkingThread, INFINITE );
			CloseHandle( m_WorkingThread );
			m_WorkingThread = NULL;
		}
	

		if( m_pWhdr1 )
		{
	//		m_WaveFile.put( m_pWhdr1->lpData, m_pWhdr1->dwBufferLength );
			err = waveInUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr1, sizeof(WAVEHDR));	
			
			
			m_pWhdr1 = 0;
		}
		if( m_pWhdr2 )
		{
	//		m_WaveFile.put( m_pWhdr2->lpData, m_pWhdr2->dwBufferLength );
			err = waveInUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr2, sizeof(WAVEHDR));	
			
			
			m_pWhdr2 = 0;
		}
	
		m_WaveFile.close();
		
		Close();
	}

	
	return err;
}
unsigned int Close()
{
	unsigned int err(0);

	if( m_hwo )
	{
		delete []m_buff1;
		delete []m_buff2;
		m_buff1 = 0;
		m_buff2 = 0;

		if( WAVERR_STILLPLAYING == ( err = waveInClose( m_hwo ) ) )
		{
			waveInReset( m_hwo );
			waveInClose( m_hwo );
		}
		m_hwo = 0;
	}

	return err;
}
unsigned int Prepare( WAVEHDR *pWhdr )
{ 

	if( !pWhdr )
		return -1;

	pWhdr->dwFlags			= 0;	
//	pWhdr->dwLoops			= 0;
	pWhdr->dwBytesRecorded	= 0;

//	memset( pWhdr->lpData, 0, pWhdr->dwBufferLength );

	unsigned int err = waveInPrepareHeader( m_hwo, pWhdr, sizeof(WAVEHDR));	
	
//	if(mmres != MMSYSERR_NOERROR)
	return err;
}
unsigned int WaitForDone( WAVEHDR* pWhdr )
{
	unsigned int ret(0);

	if( pWhdr && m_hDoneEvent )
	{
		if( !(pWhdr->dwFlags & WHDR_DONE) )
		{
			if(ret = WaitForSingleObject(m_hDoneEvent, m_RecBufferTime + 1000))
			{
				if( WAIT_OBJECT_0 == ret )
				{
					while( !(pWhdr->dwFlags & WHDR_DONE) )
						Sleep(0);
				}
				else
				{
					return -1;
				}
			}
			//ret = WaitForMultipleObjects( 2, m_EventsVector, 0, m_RecBufferTime + 4000);

			//switch( ret )
			//{
			//	case WAIT_OBJECT_0://stop
			//		return 1;
			//	case WAIT_OBJECT_0 + 1://done
			//		while( !(pWhdr->dwFlags & WHDR_DONE) )
			//			Sleep(0);
			//		break;					
			//	default: //timeout or error
			//		return -1;//failed
			//}
		}
	}

	return 0;
}

DWORD PrepAction()
{

	if( !m_hwo )
		return -1;

	TCHAR text[256];

	unsigned int	err(0);
//	unsigned long old_vol;
	WAVEHDR		whdr1 = {0};
	WAVEHDR		whdr2 = {0};
	
	memset(&whdr1, 0, sizeof(WAVEHDR));

	
	m_BufferLength = (m_RecBufferTime * m_WaveFormatEx.nSamplesPerSec / 1000) * m_WaveFormatEx.nBlockAlign;

	//m_buff1 = new char[m_BufferLength];
	//m_buff2 = new char[m_BufferLength];
	m_buff1 = new char[m_BufferLength]; 
	m_buff2 = new char[m_BufferLength]; 
	whdr1.dwBufferLength	= m_BufferLength;//for a test
	//whdr.dwUser			= 0;
	//whdr.dwFlags			= 0;
	//whdr.dwLoops			= 0;
	//whdr.dwBytesRecorded	= 0;
	//whdr.lpNext				= 0;
	//whdr.reserved			= 0;
	
	m_pWhdr1	= &whdr1;
	m_pWhdr2	= &whdr2;

	memcpy( m_pWhdr2, m_pWhdr1, sizeof(WAVEHDR) );
	
	m_pWhdr1->lpData = m_buff1;
	m_pWhdr2->lpData = m_buff2;


//	unsigned int		FileLen		= FILE_MAX_LEN;
	
	unsigned int		GlobalLen(0);//, DataLen;

	err = Prepare( m_pWhdr1 );
	err = Prepare( m_pWhdr2 );//, &DataLen );

	ResumeThread( m_WorkingThread );

//	while( WAIT_TIMEOUT == WaitForSingleObject( m_hStop, 0 ) )
	while( WAIT_TIMEOUT == WaitForSingleObject( m_WorkingThread, 0 ) )
	{

		if( err = WaitForDone( m_pWhdr1 ) )
		{
			//unpr
			waveInReset( m_hwo );
			break;
		}

		m_WaveFile.put( m_pWhdr1->lpData, m_pWhdr1->dwBytesRecorded );
		if( err = waveInUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr1, sizeof(WAVEHDR)) )
		{
			wsprintf( text, __T("waveInUnprepareHeader err %d\r\n"), err );
			OutputDebugString( text );
			//break;
		}
		
		GlobalLen += m_pWhdr1->dwBufferLength;

//		if( WAIT_TIMEOUT != WaitForSingleObject( m_hStop, 0 ) )
		if( WAIT_TIMEOUT != WaitForSingleObject( m_WorkingThread, 0 ) )
			break;

		if( GlobalLen < m_FileBigSize )
		{
			err = Prepare( m_pWhdr1 );
			SetEvent( m_hStart1 );
		}
		else
		{
//			WaitForDone( m_pWhdr2 );
//			m_pWhdr1 = 0;
			break;
		}
		
		if( err = WaitForDone( m_pWhdr2 ) )
		{
			waveInReset( m_hwo );
			break;
		}


		m_WaveFile.put( m_pWhdr2->lpData, m_pWhdr2->dwBytesRecorded );
		if( err = waveInUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr2, sizeof(WAVEHDR)) )
		{
			wsprintf( text, __T("waveInUnprepareHeader err %d\r\n"), err );
			OutputDebugString( text );
		//break;
		}

		GlobalLen += m_pWhdr2->dwBufferLength;
		
		if( GlobalLen < m_FileBigSize )
		{
			err = Prepare( m_pWhdr2 );

			SetEvent( m_hStart2 );
		}
		else
		{
			break;
		}
	}
	if( m_pWhdr1 )
	{
		if( m_pWhdr1->dwFlags & WHDR_DONE		&&
			m_pWhdr1->dwBytesRecorded	==	m_pWhdr1->dwBufferLength	)
			m_WaveFile.put( m_pWhdr1->lpData, m_pWhdr1->dwBytesRecorded );
		
		err = waveInUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr1, sizeof(WAVEHDR));
		m_pWhdr1 = 0;

	}
	if( m_pWhdr2 )
	{
		if( m_pWhdr2->dwFlags & WHDR_DONE		&&
			m_pWhdr2->dwBytesRecorded	==	m_pWhdr2->dwBufferLength	)
			m_WaveFile.put( m_pWhdr2->lpData, m_pWhdr2->dwBytesRecorded );

		err = waveInUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr2, sizeof(WAVEHDR));
		m_pWhdr2 = 0;
	}

	StopWork();

	return err;
}

	DWORD WaitForAction();
	//This method is thread routine for starting the wait for a transfer.
	static DWORD WINAPI  WaitForActionThreadProc(LPVOID lpParameter)
	{
		CWaveRecorder	*This = (CWaveRecorder*)lpParameter;
		
		// Check if the parameter passed to thread main function is valid.
		DWORD Ret = This->WaitForAction();
		return Ret;
 	}

	static DWORD WINAPI  PrepActionThreadProc(LPVOID lpParameter)
	{
		CWaveRecorder	*This = (CWaveRecorder*)lpParameter;
		
		// Check if the parameter passed to thread main function is valid.
		DWORD Ret = This->PrepAction();
		return Ret;
 	}

private:
	HWAVEIN	m_hwo;
	wav_file		m_WaveFile;
	
	WAVEHDR*		m_pWhdr1;	
	WAVEHDR*		m_pWhdr2;	
	
	HANDLE			m_WorkingThread;
	HANDLE			m_PrepThread;

	HANDLE			m_hStop;
	HANDLE			m_hStopWaiting;
	HANDLE			m_hStart1;
	HANDLE			m_hStart2;
	HANDLE			m_hDoneEvent;
	WAVEFORMATEX	m_WaveFormatEx;

	char*			m_buff1;
	char*			m_buff2;

	unsigned int	m_BufferLength;

	unsigned int	m_RecBufferTime;//RECORD_TIME		3000
	unsigned int	m_FileBigSize;	//FILE_MAX_LEN
	HANDLE			m_EventsVector[2];

};

#endif // !defined(AFX_WAVERECORDER_H__43836CF1_EF08_44C4_8992_27E85F38BEB3__INCLUDED_)
