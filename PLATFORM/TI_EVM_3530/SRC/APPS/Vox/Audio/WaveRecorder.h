// WaveRecorder.h: interface for the CWaveRecorder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVERECORDER_H__43836CF1_EF08_44C4_8992_27E85F38BEB3__INCLUDED_)
#define AFX_WAVERECORDER_H__43836CF1_EF08_44C4_8992_27E85F38BEB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define  PLAY_THROUGH  1

//#include <windows.h>
#include "wavefile.h"
#include <windowsx.h>
#include <commctrl.h>
#include <Mmreg.h>
#include "Mixer.h"

#define REC_BUFF_TIME		300
#define FILE_BIG_LEN		0x40000000 //1024*1024
#define VU_METER            1 

typedef BOOL (*ENDRECCALLBACK)(DWORD);
//extern "C" BOOL getExtMicMute(BOOL *pMicMute);

class CWaveRecorder  
{
public:
	CWaveRecorder();
	virtual ~CWaveRecorder();

	ENDRECCALLBACK pfnEndRecCallBack;

#ifdef  VU_METER
	HWND   m_hDlg;
	DWORD  m_PlayRecBar;
#endif


DWORD  Init(ENDRECCALLBACK pfnRecCallBack )
{
  if( pfnRecCallBack == NULL )
   return(-1);

  pfnEndRecCallBack = pfnRecCallBack;

  return(0);
}

unsigned int Open( WAVEFORMATEX* format )
{
//	WAVEHDR whdr;
	unsigned int err(0);

	ResetEvent( m_hDoneEvent	);
	ResetEvent( m_hStart1 );
	ResetEvent( m_hStart2 );

	m_hwo = 0;

	err = waveInOpen( &m_hwo, 0, format, (DWORD_PTR)m_hDoneEvent, 0, CALLBACK_EVENT);

	if( m_hwo == 0 )
     return -1;

//	err = waveInOpen( &m_hwo, WAVE_MAPPER, format, (DWORD_PTR)m_hDoneEvent, 0, CALLBACK_EVENT);
	
	if( err != MMSYSERR_NOERROR )
	{
		m_hwo = 0;
	}
	return	err;
}


// Used for Audio Play-through
unsigned int Open( WAVEFORMATEX* format, BOOL bPlbThrough )
{
	unsigned int err(0);

	ResetEvent( m_PlayThroughStart1 );
	ResetEvent( m_PlayThroughStart2 );
	ResetEvent( m_PlayThroughDoneEvent );

	m_PlayThroughHwo = 0;

	// Verify Wave Format
    if (  (format->nChannels!=1) && (format->nChannels!=2) )
        return -1;

    if (  (format->wBitsPerSample!=8) && (format->wBitsPerSample!=16) )
        return -1;

    if (format->nSamplesPerSec < 100 ||  format->nSamplesPerSec > 96000)
        return -1;

	// Some Wave converters puts wrong format tag...
	if( format->wFormatTag == WAVE_FORMAT_EXTENSIBLE )
	  format->wFormatTag = WAVE_FORMAT_PCM;

    if (format->wFormatTag != WAVE_FORMAT_PCM)
        return -1;

	for( unsigned int devId = 0; devId < waveOutGetNumDevs(); devId++)
	{
		err = waveOutOpen( &m_PlayThroughHwo, devId, format, (unsigned long)m_PlayThroughDoneEvent, 0, CALLBACK_EVENT | WAVE_FORMAT_DIRECT);
		if(err == MMSYSERR_NOERROR)
			break;
	}
	if( err != MMSYSERR_NOERROR )
	{
		m_PlayThroughHwo = 0;
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
	//int mute;

	err = Open( &m_WaveFormatEx );

	//getExtMicMute(&mute);
	//getMicMute(&mute);
	if( !err )
	{
		err = record( m_hwo );
	}

	// Wait for 'm_hRecStarted'
	while( WAIT_TIMEOUT == WaitForSingleObject(m_hRecStarted, INFINITE ) );
	return err; 
}


// Used for Audio Play-through
unsigned int Start( WAVEFORMATEX* pFormat )
{
	unsigned int	errP(0);
	unsigned int	err(0);

	memcpy( &m_WaveFormatEx, pFormat, sizeof(WAVEFORMATEX) );

	err = Open( &m_WaveFormatEx );
	errP = Open( &m_WaveFormatEx, TRUE );
	
	if( !errP && !err)
	{
		errP = record( m_hwo, TRUE );
	}

	// Wait for 'm_hRecStarted'
	while( WAIT_TIMEOUT == WaitForSingleObject(m_hRecStarted, INFINITE ) );
	return errP; 
}


void	Stop()
{
	unsigned int err(0);

	SetEvent( m_hStop );
	SetEvent( m_PlayThroughStop );
#if 0
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
#endif
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


// Used for Audio Play-through
unsigned int record( HWAVEIN hwo, BOOL bPlbThrough )
{
	unsigned int	err(0);
	int             playThroughPriority;
	BOOL            bRet;

	ResetEvent( m_hStopWaiting	);
	ResetEvent( m_hStop			);

	ResetEvent( m_PlayThroughStopWaiting	);
	ResetEvent( m_PlayThroughStop );

	if( bPlbThrough )
	{
	 m_WorkingThread	= CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)WaitForActionThreadProc,this,CREATE_SUSPENDED,0);
	 if( !m_WorkingThread )
	 {
		StopWork(bPlbThrough);
	 }

	 m_PrepThread	= CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)PrepActionPlayThroughThreadProc,this,0,0);
	 if( !m_PrepThread )
	 {
		StopWork(bPlbThrough);
	 }

//#ifdef  PLAY_THROUGH
	 m_PlayThroughWorkingThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)WaitForActionPlayThroughThreadProc,this,CREATE_SUSPENDED,0);
	 if( !m_PlayThroughWorkingThread )
	 {
		StopWork(bPlbThrough);
	 }

	 playThroughPriority = CeGetThreadPriority(m_PlayThroughWorkingThread);

	 bRet = CeSetThreadPriority(m_PlayThroughWorkingThread, 200);
//#endif

	}

	return err;
}

unsigned int StopWork()
{
	unsigned int err(0);
	unsigned int fileErr(0);

	RETAILMSG(1, (L"DIGVOICE:+%S\r\n",__FUNCTION__)); 

//	if( m_hStopWaiting )
//		SetEvent( m_hStopWaiting );

	if( m_hwo )
	{

//		err = waveInReset( m_hwo );
//		err = waveInStop( m_hwo );

		if( m_WorkingThread )
		{
            //ResumeThread( m_WorkingThread ); 
			SetEvent( m_hStopWaiting );

			WaitForSingleObject( m_WorkingThread, INFINITE );
			CloseHandle( m_WorkingThread );
			m_WorkingThread = NULL;
		}
	

		if( m_pWhdr1 )
		{
		 if( m_pWhdr1->dwFlags & WHDR_DONE && m_pWhdr1->dwBytesRecorded	==	m_pWhdr1->dwBufferLength )
			fileErr = m_WaveFile.put( m_pWhdr1->lpData, m_pWhdr1->dwBytesRecorded );
		
			err = waveInUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr1, sizeof(WAVEHDR));	
			m_pWhdr1 = 0;
		}

		if( m_pWhdr2 && ( fileErr != 0 ) )
		{
		 if( m_pWhdr2->dwFlags & WHDR_DONE && m_pWhdr2->dwBytesRecorded	==	m_pWhdr2->dwBufferLength )
 		  m_WaveFile.put( m_pWhdr2->lpData, m_pWhdr2->dwBytesRecorded );

		  err = waveInUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr2, sizeof(WAVEHDR));
		  m_pWhdr2 = 0;
		}
	
		m_WaveFile.close();
		
		Close();
	}

	RETAILMSG(1, (L"DIGVOICE:-%S\r\n",__FUNCTION__, err)); 
	return err;
}

// Used for Audio Play-through
unsigned int StopWork(BOOL bPlbThrough)
{
	unsigned int err(0);
	//unsigned int fileErr(0);

//	if( m_hStopWaiting )
//		SetEvent( m_hStopWaiting );

	if( m_hwo )
	{

//		err = waveInReset( m_hwo );
//		err = waveInStop( m_hwo );

		if( m_WorkingThread )
		{
            //ResumeThread( m_WorkingThread ); 
			SetEvent( m_hStopWaiting );

			WaitForSingleObject( m_WorkingThread, INFINITE );
			CloseHandle( m_WorkingThread );
			m_WorkingThread = NULL;
		}
	

		if( m_pWhdr1 )
		{
			err = waveInUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr1, sizeof(WAVEHDR));	
			m_pWhdr1 = 0;
		}

		if( m_pWhdr2 )
		{
		  err = waveInUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr2, sizeof(WAVEHDR));
		  m_pWhdr2 = 0;
		}
	
		Close();
	}

	if( m_PlayThroughStopWaiting )
		SetEvent( m_PlayThroughStopWaiting );
	
	if( m_PlayThroughHwo )
	{
		//if( err = waveOutReset( m_hwo ) )
		//{

		//}
		if( m_PlayThroughWorkingThread )
		{
			ResumeThread( m_PlayThroughWorkingThread );

			WaitForSingleObject( m_PlayThroughWorkingThread, INFINITE );
			CloseHandle( m_PlayThroughWorkingThread );
			m_PlayThroughWorkingThread = NULL;
		}

		if( m_PlayThroughpWhdr1 )
		{
			waveOutUnprepareHeader(m_PlayThroughHwo, (WAVEHDR*)m_PlayThroughpWhdr1, sizeof(WAVEHDR));	
			m_PlayThroughpWhdr1 = 0;
		}
		if( m_PlayThroughpWhdr2 )
		{
			waveOutUnprepareHeader(m_PlayThroughHwo, (WAVEHDR*)m_PlayThroughpWhdr2, sizeof(WAVEHDR));	
			m_PlayThroughpWhdr2 = 0;
		}
		
		Close(bPlbThrough);
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

unsigned int Close(BOOL bPlbThrough)
{
	unsigned int err(0);

	if( m_PlayThroughHwo )
	{
		err = waveOutClose(m_PlayThroughHwo);
		if( err != MMSYSERR_NOERROR )
          waveOutReset(m_PlayThroughHwo);

		m_PlayThroughHwo = 0;

		delete []m_PlayThroughBuff1;
		delete []m_PlayThroughBuff2;
		m_PlayThroughBuff1 = 0;
		m_PlayThroughBuff2 = 0;

	}

	return err;
}

unsigned int Prepare( WAVEHDR *pWhdr, BOOL bPlbThrough, LPSTR lpData, int len )
{ 
	if( !pWhdr )
		return 1;

	memcpy(pWhdr->lpData, lpData, len ); 
	pWhdr->dwBufferLength = len;
	pWhdr->dwFlags = 0;

	unsigned int mmres = waveOutPrepareHeader( m_PlayThroughHwo, pWhdr, sizeof(WAVEHDR));	
	
//	if(mmres != MMSYSERR_NOERROR)
	return mmres;
}

unsigned int Prepare( WAVEHDR *pWhdr )
{ 

	if( !pWhdr )
		return -1;

	pWhdr->dwFlags			= 0;	
//	pWhdr->dwLoops			= 0;
	pWhdr->dwBytesRecorded	= 0;

	memset( pWhdr->lpData, 0, pWhdr->dwBufferLength );

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

void WaitForDone( WAVEHDR* pWhdr, BOOL bPlbThrough )
{
	if( pWhdr && m_PlayThroughDoneEvent )
	{
		while(!(pWhdr->dwFlags & WHDR_DONE))
		{
			WaitForSingleObject( m_PlayThroughDoneEvent, INFINITE );
		}
	}
}


UINT32 intSqrt(UINT64 n)
{
    UINT64 a;
    for (a=0;n>=(2*a)+1;n-=(2*a++)+1);
    return (UINT32)a;
}


DWORD PrepAction()
{
	if( !m_hwo )
		return -1;

	//TCHAR text[256];

	unsigned int	err(0);
	unsigned int	fileErr(0);
//	unsigned long old_vol;
	WAVEHDR		whdr1 = {0};
	WAVEHDR		whdr2 = {0};

	//UINT32      avgVol_quarterSec = 0;
	//UINT32      samplesPerQuarterSec, sampleCount;
  
	RETAILMSG(1, (L"DIGVOICE:+%S\r\n",__FUNCTION__)); 
	
	memset(&whdr1, 0, sizeof(WAVEHDR));

	
	m_BufferLength = (m_RecBufferTime * m_WaveFormatEx.nSamplesPerSec / 1000) * m_WaveFormatEx.nBlockAlign;

#ifdef VU_METER
	unsigned int	barPosition;
	INT16       *pWavData = NULL;
	UINT64       sqSum = 0ll;
	UINT32       rms;

	int step = m_BufferLength>>1 ; // divide by 2 == m_WaveFormatEx.nBlockAlign
	step = step >> 8; // divide by 256  
	int maxIndex = step << 8; // * 256

#endif
	//samplesPerQuarterSec = m_WaveFormatEx.nSamplesPerSec >> 2;  // nSamplesPerSec / 4

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

	while( WAIT_TIMEOUT == WaitForSingleObject( m_hStop, 0 ) )
//	while( WAIT_TIMEOUT == WaitForSingleObject( m_WorkingThread, 0 ) )
	{

		if( err = WaitForDone( m_pWhdr1 ) )
		{
			//unpr
			waveInReset( m_hwo );
			break;
		}

		fileErr = m_WaveFile.put( m_pWhdr1->lpData, m_pWhdr1->dwBytesRecorded );
		if( err = waveInUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr1, sizeof(WAVEHDR)) )
		{
			RETAILMSG(1, (L"DIGVOICE:%S, waveInUnprepareHeader err %d\r\n",__FUNCTION__, err)); 
			//break;
		}
		
		if( fileErr == 0 )
         break;

		GlobalLen += m_pWhdr1->dwBufferLength;

#ifdef VU_METER
		pWavData = (INT16 *) m_pWhdr1->lpData;
		sqSum = 0ll;

		for( int i = 0; i < maxIndex; i = i + step )
		{
          sqSum = sqSum +  (UINT64)(pWavData[i] * pWavData[i]);
		}

        sqSum = sqSum >> 8;
		rms = intSqrt(sqSum);

		//rms = fp_log10(rms, 0);
	    barPosition = (rms * 100)>>13;
		SendDlgItemMessage(m_hDlg, m_PlayRecBar, PBM_SETPOS, barPosition, 0);
#endif
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
			WaitForDone( m_pWhdr1 );
			m_pWhdr1 = 0;
			break;
		}
		
		if( err = WaitForDone( m_pWhdr2 ) )
		{
			waveInReset( m_hwo );
			break;
		}


		fileErr = m_WaveFile.put( m_pWhdr2->lpData, m_pWhdr2->dwBytesRecorded );
		if( err = waveInUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr2, sizeof(WAVEHDR)) )
		{
			RETAILMSG(1, (L"DIGVOICE:%S, waveInUnprepareHeader err %d\r\n",__FUNCTION__, err)); 
		//break;
		}

		if( fileErr == 0 )
         break;

		GlobalLen += m_pWhdr2->dwBufferLength;

#ifdef VU_METER
		pWavData = (INT16 *) m_pWhdr2->lpData;
		sqSum = 0ll;

		for( int i = 0; i < maxIndex; i = i + step )
		{
          sqSum = sqSum +  (UINT64)(pWavData[i] * pWavData[i]);
		}

        sqSum = sqSum >> 8;
		rms = intSqrt(sqSum);

		//rms = fp_decibels(rms, 0);
	    barPosition = (rms * 100)>>13;
		SendDlgItemMessage(m_hDlg, m_PlayRecBar, PBM_SETPOS, barPosition, 0);
#endif

		
		if( GlobalLen < m_FileBigSize )
		{
			err = Prepare( m_pWhdr2 );

			SetEvent( m_hStart2 );
		}
		else
		{
			WaitForDone( m_pWhdr2 );
			m_pWhdr2 = 0;
			break;
		}
	}
#if 0
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
#endif

#ifdef VU_METER
	SendDlgItemMessage(m_hDlg, m_PlayRecBar, PBM_SETPOS, 0, 0);
#endif
	StopWork();

	RETAILMSG(1, (L"DIGVOICE:-%S, quit thread, err = %d\r\n",__FUNCTION__, err)); 
	return err;
}


//
DWORD PrepAction(BOOL bPlbThrough)
{
	if( !m_hwo || !m_PlayThroughHwo )
		return -1;

	TCHAR text[256];

	unsigned int	err(0);
	//unsigned int	fileErr(0);
//	unsigned long old_vol;
	WAVEHDR		whdr1 = {0};
	WAVEHDR		whdr2 = {0};

	WAVEHDR		whdrPlayThrough1 = {0};
	WAVEHDR		whdrPlayThrough2 = {0};

	//UINT32      avgVol_quarterSec = 0;
	//UINT32      samplesPerQuarterSec, sampleCount;

	
	memset(&whdr1, 0, sizeof(WAVEHDR));
	memset(&whdrPlayThrough1, 0, sizeof(WAVEHDR));

	
	m_BufferLength = (m_RecBufferTime * m_WaveFormatEx.nSamplesPerSec / 1000) * m_WaveFormatEx.nBlockAlign;

#ifdef VU_METER
	unsigned int	barPosition;
	INT16       *pWavData = NULL;
	UINT64       sqSum = 0ll;
	UINT32       rms;

	int step = m_BufferLength>>1 ; // divide by 2 == m_WaveFormatEx.nBlockAlign
	step = step >> 8; // divide by 256  
	int maxIndex = step << 8; // * 256

#endif
	//samplesPerQuarterSec = m_WaveFormatEx.nSamplesPerSec >> 2;  // nSamplesPerSec / 4

	//m_buff1 = new char[m_BufferLength];
	//m_buff2 = new char[m_BufferLength];
	m_buff1 = new char[m_BufferLength]; 
	m_buff2 = new char[m_BufferLength]; 

	m_PlayThroughBuff1 = new char[m_BufferLength];
    m_PlayThroughBuff2 = new char[m_BufferLength];

	whdr1.dwBufferLength	= m_BufferLength;//for a test
	whdrPlayThrough1.dwBufferLength	= m_BufferLength;//for a test
	
	//whdr.dwUser			= 0;
	//whdr.dwFlags			= 0;
	//whdr.dwLoops			= 0;
	//whdr.dwBytesRecorded	= 0;
	//whdr.lpNext				= 0;
	//whdr.reserved			= 0;
	
	m_pWhdr1	= &whdr1;
	m_pWhdr2	= &whdr2;

    m_PlayThroughpWhdr1 = &whdrPlayThrough1;
    m_PlayThroughpWhdr2 = &whdrPlayThrough2;

	memcpy( m_pWhdr2, m_pWhdr1, sizeof(WAVEHDR) );
	memcpy( m_PlayThroughpWhdr2, m_PlayThroughpWhdr1, sizeof(WAVEHDR) );
	
	m_pWhdr1->lpData = m_buff1;
	m_pWhdr2->lpData = m_buff2;

	m_PlayThroughpWhdr1->lpData = m_PlayThroughBuff1;
	m_PlayThroughpWhdr2->lpData = m_PlayThroughBuff2;


//	unsigned int		FileLen		= FILE_MAX_LEN;
	
	unsigned int		GlobalLen(0);//, DataLen;

	err = Prepare( m_pWhdr1 );
	err = Prepare( m_pWhdr2 );//, &DataLen );

	//err = Prepare( m_pWhdr1, TRUE, m_BufferLength );
	//err = Prepare( m_pWhdr2, TRUE, m_BufferLength );
#ifdef  PLAY_THROUGH
	//Prepare(m_PlayThroughpWhdr1, TRUE, m_pWhdr1->lpData, m_BufferLength );
	//Prepare(m_PlayThroughpWhdr2, TRUE, m_pWhdr2->lpData, m_BufferLength );
#endif

	ResumeThread( m_WorkingThread );
//#ifdef  PLAY_THROUGH
	ResumeThread( m_PlayThroughWorkingThread );
//#endif

	while( WAIT_TIMEOUT == WaitForSingleObject( m_hStop, 0 ) )
//	while( WAIT_TIMEOUT == WaitForSingleObject( m_WorkingThread, 0 ) )
	{

		if( err = WaitForDone( m_pWhdr1 ) )
		{
			//unpr
			waveInReset( m_hwo );
			break;
		}

		//fileErr = m_WaveFile.put( m_pWhdr1->lpData, m_pWhdr1->dwBytesRecorded );
		// Prepare buffer for Playback
//#ifdef  PLAY_THROUGH
		Prepare(m_PlayThroughpWhdr1, TRUE, m_pWhdr1->lpData, m_BufferLength );
		SetEvent( m_PlayThroughStart1 );
//#endif

		if( err = waveInUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr1, sizeof(WAVEHDR)) )
		{
			wsprintf( text, __T("waveInUnprepareHeader err %d\r\n"), err );
			OutputDebugString( text );
			//break;
		}
		
		//if( fileErr == 0 )
        // break;

		GlobalLen += m_pWhdr1->dwBufferLength;

#ifdef VU_METER
		pWavData = (INT16 *) m_pWhdr1->lpData;
		sqSum = 0ll;

		for( int i = 0; i < maxIndex; i = i + step )
		{
          sqSum = sqSum +  (UINT64)(pWavData[i] * pWavData[i]);
		}

        sqSum = sqSum >> 8;
		rms = intSqrt(sqSum);

		//rms = fp_log10(rms, 0);
	    barPosition = (rms * 100)>>13;
		SendDlgItemMessage(m_hDlg, m_PlayRecBar, PBM_SETPOS, barPosition, 0);
#endif

//#ifdef  PLAY_THROUGH
		//WaitForDone(m_PlayThroughpWhdr1, TRUE );
		//waveOutUnprepareHeader(m_PlayThroughHwo, (WAVEHDR*)m_PlayThroughpWhdr1, sizeof(WAVEHDR));	
//#endif

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
			WaitForDone( m_pWhdr1 );
			m_pWhdr1 = 0;
			break;
		}
		
		if( err = WaitForDone( m_pWhdr2 ) )
		{
			waveInReset( m_hwo );
			break;
		}

//#ifdef  PLAY_THROUGH
		Prepare(m_PlayThroughpWhdr2, TRUE, m_pWhdr2->lpData, m_BufferLength );
		SetEvent( m_PlayThroughStart2 );
//#endif

		//fileErr = m_WaveFile.put( m_pWhdr2->lpData, m_pWhdr2->dwBytesRecorded );
		if( err = waveInUnprepareHeader(m_hwo, (WAVEHDR*)m_pWhdr2, sizeof(WAVEHDR)) )
		{
			wsprintf( text, __T("waveInUnprepareHeader err %d\r\n"), err );
			OutputDebugString( text );
		//break;
		}

		//if( fileErr == 0 )
         //break;

		GlobalLen += m_pWhdr2->dwBufferLength;

#ifdef VU_METER
		pWavData = (INT16 *) m_pWhdr2->lpData;
		sqSum = 0ll;

		for( int i = 0; i < maxIndex; i = i + step )
		{
          sqSum = sqSum +  (UINT64)(pWavData[i] * pWavData[i]);
		}

        sqSum = sqSum >> 8;
		rms = intSqrt(sqSum);

		//rms = fp_decibels(rms, 0);
	    barPosition = (rms * 100)>>13;
		SendDlgItemMessage(m_hDlg, m_PlayRecBar, PBM_SETPOS, barPosition, 0);
#endif

//#ifdef  PLAY_THROUGH
		//WaitForDone(m_PlayThroughpWhdr2, TRUE );
		//waveOutUnprepareHeader(m_PlayThroughHwo, (WAVEHDR*)m_PlayThroughpWhdr2, sizeof(WAVEHDR));	
//#endif
		
		if( GlobalLen < m_FileBigSize )
		{
			err = Prepare( m_pWhdr2 );

			SetEvent( m_hStart2 );
		}
		else
		{
			WaitForDone( m_pWhdr2 );
			m_pWhdr2 = 0;
			break;
		}
	}
#if 0
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
#endif

#ifdef VU_METER
	SendDlgItemMessage(m_hDlg, m_PlayRecBar, PBM_SETPOS, 0, 0);
#endif
	StopWork(TRUE);

	return err;
}

	DWORD WaitForAction();
	//This method is thread routine for starting the wait for a transfer.
	static DWORD WINAPI  WaitForActionThreadProc(LPVOID lpParameter)
	{
		CWaveRecorder	*This = (CWaveRecorder*)lpParameter;
		
		// Check if the parameter passed to thread main function is valid.
		DWORD Ret = This->WaitForAction();
		This->pfnEndRecCallBack(Ret);
		return Ret;
 	}

	DWORD  WaitForActionPlayThrough();

	static DWORD WINAPI  WaitForActionPlayThroughThreadProc(LPVOID lpParameter)
	{
		CWaveRecorder	*This = (CWaveRecorder*)lpParameter;
		
		// Check if the parameter passed to thread main function is valid.
		DWORD Ret = This->WaitForActionPlayThrough();
		This->pfnEndRecCallBack(Ret);
		return Ret;
 	}


	static DWORD WINAPI  PrepActionThreadProc(LPVOID lpParameter)
	{
		CWaveRecorder	*This = (CWaveRecorder*)lpParameter;
		
		// Check if the parameter passed to thread main function is valid.
		DWORD Ret = This->PrepAction();
		return Ret;
 	}

	static DWORD WINAPI  PrepActionPlayThroughThreadProc(LPVOID lpParameter)
	{
		CWaveRecorder	*This = (CWaveRecorder*)lpParameter;
		
		// Check if the parameter passed to thread main function is valid.
		DWORD Ret = This->PrepAction(TRUE);
		return Ret;
 	}

private:
	//HWAVEIN	m_hwo;
	wav_file		m_WaveFile;
	
	WAVEHDR*		m_pWhdr1;	
	WAVEHDR*		m_pWhdr2;	
	
	HANDLE			m_WorkingThread;
	HANDLE			m_PrepThread;

	HANDLE			m_hStop;
	HANDLE			m_hStopWaiting;
	HANDLE	   	    m_hRecStarted;
	HANDLE			m_hStart1;
	HANDLE			m_hStart2;
	//HANDLE			m_hDoneEvent;
	WAVEFORMATEX	m_WaveFormatEx;

	char*			m_buff1;
	char*			m_buff2;

	unsigned int	m_BufferLength;

	unsigned int	m_RecBufferTime;//RECORD_TIME		3000
	unsigned int	m_FileBigSize;	//FILE_MAX_LEN
	HANDLE			m_EventsVector[2];

	// Play soaund being recorded while it is recording or
	// Play-through channel
	HWAVEOUT	m_PlayThroughHwo;
	WAVEHDR*	m_PlayThroughpWhdr1;	
	WAVEHDR*	m_PlayThroughpWhdr2;	
	
	HANDLE		m_PlayThroughWorkingThread;
	//HANDLE		m_PrepThread;

	HANDLE		m_PlayThroughStop;
	HANDLE		m_PlayThroughStopWaiting;
	HANDLE	    m_PlayThroughRecStarted;
	HANDLE		m_PlayThroughStart1;
	HANDLE		m_PlayThroughStart2;
	HANDLE		m_PlayThroughDoneEvent;

	char*		m_PlayThroughBuff1;
	char*		m_PlayThroughBuff2;

	//mutable char		m_buff1[BUFF_SIZE];
	//mutable char		m_buff2[BUFF_SIZE];

public:
	HWAVEIN	        m_hwo;
	HANDLE			m_hDoneEvent;

};




#endif // !defined(AFX_WAVERECORDER_H__43836CF1_EF08_44C4_8992_27E85F38BEB3__INCLUDED_)
