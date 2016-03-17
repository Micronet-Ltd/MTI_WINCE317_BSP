// WavePlayer.h: interface for the CWavePlayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVEPLAYER_H__2199305A_7B3E_4A49_86DA_4FC1092D3EAE__INCLUDED_)
#define AFX_WAVEPLAYER_H__2199305A_7B3E_4A49_86DA_4FC1092D3EAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wavefile.h"

#define		BUFF_SIZE		32*1024

class CWavePlayer  
{
public:
	CWavePlayer();
	virtual ~CWavePlayer();


unsigned int	Pause()
{
	unsigned int err(0);

	if( m_hwo )
	{
		err = waveOutPause( m_hwo );
	}

	return err;
}
unsigned int Restart()
{
	int err(0);

	if( m_hwo )
	{
		err = waveOutRestart( m_hwo );
	}

	return err;
}

unsigned int Prepare( WAVEHDR *pWhdr, wav_file *waveFile, unsigned int* pLen )
{ 
	if( !pWhdr )
		return 1;

	*pLen = waveFile->get( pWhdr->lpData, pWhdr->dwBufferLength );
	
	if( pWhdr->dwBufferLength != (unsigned int)*pLen ) //must be last chunk
		pWhdr->dwBufferLength	= *pLen;
	
	else if( !*pLen )
		return 0;//	StopWork();
	
	pWhdr->dwFlags = 0;

	unsigned int mmres = waveOutPrepareHeader( m_hwo, pWhdr, sizeof(WAVEHDR));	
	
//	if(mmres != MMSYSERR_NOERROR)
	return mmres;
}
unsigned int StopWork()
{
	unsigned int err(0);

	if( m_hStopWaiting )
		SetEvent( m_hStopWaiting );
	
	if( m_hwo )
	{
		//if( err = waveOutReset( m_hwo ) )
		//{

		//}
		if( m_WorkingThread )
		{
			ResumeThread( m_WorkingThread );

			WaitForSingleObject( m_WorkingThread, INFINITE );
			CloseHandle( m_WorkingThread );
			m_WorkingThread = NULL;
		}
		if( m_pWhdr1 )
		{
			waveOutUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr1, sizeof(WAVEHDR));	
			m_pWhdr1 = 0;
		}
		if( m_pWhdr2 )
		{
			waveOutUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr2, sizeof(WAVEHDR));	
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
		while( WAVERR_STILLPLAYING == ( err = waveOutClose( m_hwo ) ) )
			Sleep( 10 );
		m_hwo = 0;
	}

	return err;
}
unsigned int Open( WAVEFORMATEX* format )
{
//	WAVEHDR whdr;
	unsigned int err(0);

	ResetEvent( m_hDoneEvent	);
	m_hwo = 0;

	for( unsigned int devId = 0; devId < waveOutGetNumDevs(); devId++)
	{
		err = waveOutOpen( &m_hwo, devId, format, (unsigned long)m_hDoneEvent, 0, CALLBACK_EVENT | WAVE_FORMAT_DIRECT);
		if(err == MMSYSERR_NOERROR)
			break;
	}
	if( err != MMSYSERR_NOERROR )
	{
		m_hwo = 0;
	}
	return	err;
}
//
void	Stop()
{
	SetEvent( m_hStop );
	if( m_PrepThread )
	{
		SetEvent( m_hStopWaiting );

		WaitForSingleObject( m_PrepThread, INFINITE );
		CloseHandle( m_PrepThread );
		m_PrepThread = NULL;
	}
}

unsigned int Start( const TCHAR* FileName )
{
	if( m_WaveFile.open( FileName ) )
		return -1;
	
	return Start( &m_WaveFile );
}
unsigned int Start( wav_file *waveFile )
{
//	m_hwo = hwo;
	unsigned int	err(0);
//	MMRESULT mmres;
	err = Open( waveFile->get_format() );
	if( !err )
	{
		err = play( m_hwo );
	}
	return err; 
}

protected:

unsigned int play( HWAVEOUT hwo )
{
	unsigned int	err(0);
	
	ResetEvent( m_hStopWaiting );
	ResetEvent( m_hStop );
	ResetEvent( m_hStart1 );
	ResetEvent( m_hStart2 );

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

void WaitForDone( WAVEHDR* pWhdr )
{
	if( pWhdr && m_hDoneEvent )
	{
		while(!(pWhdr->dwFlags & WHDR_DONE))
		{
			WaitForSingleObject( m_hDoneEvent, INFINITE );
		}
	}
}
DWORD PrepAction()
{

	if( !m_hwo )
		return -1;

	unsigned int	err(0);
//	unsigned unsigned int old_vol;
	WAVEHDR		whdr1 = {0};
	WAVEHDR		whdr2 = {0};
	
	memset(&whdr1, 0, sizeof(WAVEHDR));

	
	whdr1.dwBufferLength	= BUFF_SIZE;//for a test
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


	unsigned int		FileLen		= m_WaveFile.length();
	
	unsigned int		GlobalLen(0), DataLen;

	err = Prepare( m_pWhdr1, &m_WaveFile, &DataLen );
	if( !DataLen )
		StopWork();
	GlobalLen += DataLen;

	err = Prepare( m_pWhdr2, &m_WaveFile, &DataLen );
	if( !DataLen )
		StopWork();
			
	GlobalLen += DataLen;

	ResumeThread( m_WorkingThread );

	while( WAIT_TIMEOUT == WaitForSingleObject( m_hStop, 0 ) )
	{

		WaitForDone( m_pWhdr1 );

		err = waveOutUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr1, sizeof(WAVEHDR));	
		
		if( GlobalLen < FileLen )
		{
			err = Prepare( m_pWhdr1, &m_WaveFile, &DataLen );
			if( !DataLen )
			{
				WaitForDone( m_pWhdr2 );
				break;
			}
			SetEvent( m_hStart1 );
			
			GlobalLen += DataLen;
		}
		else
		{
			m_pWhdr1 = 0;
			break;
		}
		
		WaitForDone( m_pWhdr2 );

		err = waveOutUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr2, sizeof(WAVEHDR));	
		
		if( GlobalLen < FileLen )
		{
			err = Prepare( m_pWhdr2, &m_WaveFile, &DataLen );

			SetEvent( m_hStart2 );
			
			GlobalLen += DataLen;
		}
		else
		{
			//WaitForDone( m_pWhdr1 );
			m_pWhdr2 = 0;
			break;
		}
//		if( GlobalLen >= FileLen )//EOF
//			break;
	}


	StopWork();

	return 0;
}

private:

	DWORD WaitForAction();
	//This method is thread routine for starting the wait for a transfer.
	static DWORD WINAPI  WaitForActionThreadProc(LPVOID lpParameter)
	{
		CWavePlayer	*This = (CWavePlayer*)lpParameter;
		
		// Check if the parameter passed to thread main function is valid.
		DWORD Ret = This->WaitForAction();
		return Ret;
 	}

	static DWORD WINAPI  PrepActionThreadProc(LPVOID lpParameter)
	{
		CWavePlayer	*This = (CWavePlayer*)lpParameter;
		
		// Check if the parameter passed to thread main function is valid.
		DWORD Ret = This->PrepAction();
		return Ret;
 	}
	HWAVEOUT	m_hwo;
	wav_file	m_WaveFile;
	
	WAVEHDR*	m_pWhdr1;	
	WAVEHDR*	m_pWhdr2;	
	
	HANDLE		m_WorkingThread;
	HANDLE		m_PrepThread;

	HANDLE		m_hStop;
	HANDLE		m_hStopWaiting;
	HANDLE		m_hStart1;
	HANDLE		m_hStart2;
	HANDLE		m_hDoneEvent;

	mutable char		m_buff1[BUFF_SIZE];
	mutable char		m_buff2[BUFF_SIZE];
};

#endif // !defined(AFX_WAVEPLAYER_H__2199305A_7B3E_4A49_86DA_4FC1092D3EAE__INCLUDED_)
