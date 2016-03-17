// DigVoice.cpp : Defines the entry point for the application.
//

//#include "stdafx.h"
#include "Audio.h"
#include <windows.h>
//#include <service.h>
#include <commctrl.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <bt_api.h>

#include "WavePlayer.h"
#include "WaveRecorder.h"
#include "Mixer.h"
#ifdef BT_AG
 #include "bt.h"
#endif /* BT_AG */

#include <MicUserSdk.h>
#include "Ag.h"

#define MAX_LOADSTRING 100

#define NUM_OF_EDIT_CHARS  32

//#define DEF_TEST           0

//==============================================================================================
//     enumerations
//==============================================================================================
typedef enum
{
	FILE_SILENT = 0,
    FILE_PLAYING,
	FILE_PAUSE,

} PLAY_STATE;

typedef enum
{
	REC_OFF = 0,
    REC_ON,

} REC_STATE;


typedef enum
{
	INT_MIC = 0,
    EXT_MIC,

	NUM_OF_MIC,

} MIC_TYPE_e;



PLAY_STATE gPlayState = FILE_SILENT;
REC_STATE  gRecState = REC_OFF;

HINSTANCE       g_hInst;
HICON           g_hBTIcon = NULL;

HWND            g_hDlg;
CWavePlayer		g_pl;
CWaveRecorder	g_rec;
OPENFILENAME    ofnPlay = {0};
OPENFILENAME    ofnRec = {0};
BOOL            gPlayFileOpenStatus = FALSE;
BOOL            gRecFileOpenStatus = FALSE;
TCHAR           myTest[32];
TCHAR           gEditLine[NUM_OF_EDIT_CHARS];

MIC_TYPE_e      gExtMic = INT_MIC;

#ifdef BT_AG
 BOOL            gBTPresent = FALSE;
 BT_ADDR         gBTAddr = 0ll;
 HANDLE          g_hBT = NULL;
#endif /* BT_AG */

HANDLE g_hThr = NULL;

  static UINT16 /* micAnalogGain, */ sMicAnalogGainMin, sMicAnalogGainMax;
  static int /*micDigitalGain, */ sMicDigitalGainMin, sMicDigitalGainMax;
  

DWORD WINAPI AudioThread(LPVOID lpParam);

DWORD WINAPI RunnerThread(LPVOID lpParam);

LRESULT CALLBACK mainDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
BOOL  EndPlayBack( DWORD playerRet );


static void updateMicAnalogGainInSlider(HWND hDlg, HWND hDlgSliderMicAnalogGain, int *pMicAnalogGainCurPos);
static void updateMicDigitalGainInSlider(HWND hDlg, HWND hDlgSliderMicDigitalGain, int *pMicDigitalGainCurPos);

BOOL  gDigSliderNotifyOmit = FALSE;
BOOL  gAnSliderNotifyOmit = FALSE;


BOOL  EndPlayBack( DWORD playerRet )
{
  // WM_USER 
  SendMessage(g_hDlg, WM_USER, 0 ,0); 
  return(TRUE);
}

BOOL  EndRecording( DWORD recRet )
{
  // WM_USER 
  SendMessage(g_hDlg, WM_USER+1, 0 ,0); 
  return(TRUE);
}


void FAR PASCAL lineCallbackFunc(DWORD hDevice,DWORD dwMsg, DWORD dwCallbackInstance,DWORD dwParam1,DWORD dwParam2, DWORD dwParam3)
{
 // NKDbgPrintfW(L"lineCallbackFunc( %d , %d , %d , %d , %d , %d
 // )\n",hDevice,dwMsg,dwCallbackInstance,dwParam1,dwParam2,dwParam3);
 return;
}



LRESULT CALLBACK mainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lparam)
{
  static HWND hDlgSlider, hDlgMasterSlider; 
  static HWND hDlgSliderAnalogGain; 
  static HWND hDlgSliderMicAnalogGain; 
  static HWND hDlgSliderMicDigitalGain; 

  DWORD  numOfSliderTicks;
  static int count = 0;
  static int maxRange, minRange;
  static int maxRangeAnalogGain, minRangeAnalogGain;
  static int curPos;
  static int curPosAGain;
  static UINT16 aGain, aGainMin, aGainMax;

  static HWAVEOUT hwo = 0;
  //static HWAVEIN  hwi = 0;
  static UINT devId = 0;
  static unsigned long volOut = 0;
  static unsigned long volLeft, volRight;
  static unsigned long masterVolLeft, masterVolRight;
  static unsigned long lVolOut1, lVolOut2;
  static WAVEFORMATEX  wfxRec;

  // Microphone Analog/Digital gain sliders
  //static int    micAnalogGainMaxRange, micAnalogGainMinRange;
  //static int    micDigitalGainMaxRange, micDigitalGainMinRange;
  static int    micAnalogGainCurPos[NUM_OF_MIC], micDigitalGainCurPos[NUM_OF_MIC];
  

  MMRESULT mmres;

  count++;

  switch(msg)
  {
    case WM_INITDIALOG:
	{
     TCHAR         myTest[32]; 
	 WAVEFORMATEX  wfx;
	 BOOL          bRet;
	 BOOL          bExtAudio;
	 //BOOL          micMute;
	 LONG          iRet;
	 DWORD         ret;
	 DWORD         AgServiceStatus;
	 MIXER_ERROR_CODES  mixErr;
	 DWORD         dwDstLine, dwSrcLine;
	 //HANDLE        hBT;
#if 0
	DWORD btInputBuffer[OPEN_BUFFER_LENGTH] = {0};
	AIO_OUTPUT_CONTEXT Local_output1 = {0};
	HANDLE hIO = NULL;


    //bRet = MIC_5VSetValue(0);
	//bRet = MIC_5VSetValue(1);
	//bRet = MIC_5VSetValue(0);
	 btInputBuffer[0] = OPEN_BUFFER_LENGTH;
	 hIO = MIC_AIOOpen(btInputBuffer);

	 // Read current state
	 Local_output1.size = sizeof(Local_output1);
	 Local_output1.pinNo.AUT_OUT_1 = 1;

	 bRet = MIC_AIOGetOutputInfo(hIO, &Local_output1);

	 // Set LOW
	 Local_output1.state = LOW;
	 bRet = MIC_AIOSetOutputState(hIO, &Local_output1);
	 bRet = MIC_AIOGetOutputInfo(hIO, &Local_output1);

	 // Set HIGH
	 Local_output1.state = HIGH;
	 bRet = MIC_AIOSetOutputState(hIO, &Local_output1);
	 bRet = MIC_AIOGetOutputInfo(hIO, &Local_output1);

	 // Set LOW
	 Local_output1.state = LOW;
	 bRet = MIC_AIOSetOutputState(hIO, &Local_output1);
	 bRet = MIC_AIOGetOutputInfo(hIO, &Local_output1);

	 bRet = MIC_AIOClose(hIO, btInputBuffer);
#endif

	 g_hDlg = hDlg;

     mixErr = enumMixer();
	 
     g_pl.Init(EndPlayBack); 
	 g_rec.Init(EndRecording);
#ifdef  VU_METER
	 g_rec.m_PlayRecBar = IDC_REC_PROGRESS_BAR;
#endif
	 g_pl.m_PlayProgressBar = IDC_PLAY_PROGRESS_BAR;
     g_pl.m_PlayProgressText = IDC_PLAY_PROGRESS;

     hDlgSlider = GetDlgItem(hDlg, IDC_SLIDER_PLAY_VOL);
	 hDlgMasterSlider = GetDlgItem(hDlg, IDC_SLIDER_PLAY_MASTER_VOL);
	 //hDlgSliderAnalogGain = GetDlgItem(hDlg, IDC_SLIDER_AGAIN);
	 
	 hDlgSliderMicAnalogGain  = GetDlgItem(hDlg, IDC_SLIDER_MIC_ANALOG_GAIN);  
     hDlgSliderMicDigitalGain = GetDlgItem(hDlg, IDC_SLIDER_MIC_DIGITAL_GAIN); 

	 //numOfSliderTicks = SendDlgItemMessage(hDlg, IDC_SLIDER_PLAY_VOL, TBM_GETNUMTICS, 0 ,0);
	 numOfSliderTicks = SendMessage(hDlgSlider, TBM_GETNUMTICS, 0 ,0);
	 maxRange = SendMessage(hDlgSlider, TBM_GETRANGEMAX, 0 ,0); 
	 minRange = SendMessage(hDlgSlider, TBM_GETRANGEMIN, 0 ,0); 
	 maxRangeAnalogGain = SendMessage(hDlgSliderAnalogGain, TBM_GETRANGEMAX, 0 ,0); 
	 minRangeAnalogGain = SendMessage(hDlgSliderAnalogGain, TBM_GETRANGEMIN, 0 ,0); 

	 curPos = SendDlgItemMessage(hDlg, IDC_SLIDER_PLAY_VOL, TBM_GETPOS, 0 ,0);
	 //curPosAGain = SendDlgItemMessage(hDlg, IDC_SLIDER_AGAIN, TBM_GETPOS, 0 ,0); 

     //micAnalogGainMaxRange  = SendMessage(hDlgSliderMicAnalogGain, TBM_GETRANGEMAX, 0 ,0); 
     //micAnalogGainMinRange  = SendMessage(hDlgSliderMicAnalogGain, TBM_GETRANGEMAX, 0 ,0); 
     //micDigitalGainMaxRange = SendMessage(hDlgSliderMicDigitalGain, TBM_GETRANGEMAX, 0 ,0); 
	 //micDigitalGainMinRange = SendMessage(hDlgSliderMicDigitalGain, TBM_GETRANGEMAX, 0 ,0); 
 

	 //if(mmres == MMSYSERR_NOERROR)
	  
        DWORD  volVerify;
     	mmres = waveOutGetVolume(0/*hwo*/, &volOut);


		//mmres = waveOutClose(hwo);
		//mmres = waveOutSetVolume(hwo, volOut);

		//BOOL bRes = setSpkMute(TRUE);
		//mmres = waveOutGetVolume(0/*hwo*/, &volOut);

		masterVolLeft = (volOut & 0x0000FFFF);
		masterVolRight = ((volOut & 0xFFFF0000) >>16);

		// Master Volume
		volOut = masterVolLeft | (masterVolRight << 16);

		curPos = ( (masterVolLeft * maxRange)/0x0000FFFF );

		wsprintf( myTest, _T("%d%%"), curPos );
		SetDlgItemText( hDlg, IDC_MASTER_VOLUME_BOX, myTest );

		SendDlgItemMessage(hDlg, IDC_SLIDER_PLAY_MASTER_VOL, TBM_SETPOS, TRUE , curPos);

		// Local Volume
		volLeft = masterVolLeft/2; //Set 50% from Master Volume
		volRight = masterVolRight/2;

		volOut = volLeft | (volRight << 16);

		curPos = ( (volLeft * maxRange)/0x0000FFFF );

		wsprintf( myTest, _T("%d%%"), curPos );
		SetDlgItemText( hDlg, IDC_VOLUME_BOX, myTest );

		SendDlgItemMessage(hDlg, IDC_SLIDER_PLAY_VOL, TBM_SETPOS, TRUE , curPos);

		SendDlgItemMessage(hDlg, IDC_PLAY_PROGRESS_BAR, PBM_SETPOS, 0, 0);
	    wsprintf( myTest, _T("--:--/--:--"));
	    SetDlgItemText( hDlg, IDC_PLAY_PROGRESS, myTest );


#ifdef BT_AG
	  //if( isBTAbsent() )
	  //  EnableWindow( GetDlgItem( hDlg, IDC_CHECK_BT_HS ), FALSE );

	  // disable BT because BT playback problem (high voice distortion)
      EnableWindow( GetDlgItem( hDlg, IDC_CHECK_BT_HS ), FALSE );

	  //bRet = findLastPaired(&gBTAddr);
#else
       EnableWindow( GetDlgItem( hDlg, IDC_CHECK_BT_HS ), FALSE );  
#endif /* BT_AG */

#if 0
      ICONINFO  iconinfo;
	  //bRet = GetIconInfo( IDC_APPSTARTING, iconinfo);
	  HICON hIconLarge, hIconSmall, hIcon, hPlay;
	 
     
	  g_hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_ICON), IMAGE_ICON, 16, 16, 0 ); 
	  hIconLarge = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_ICON), IMAGE_ICON, 32, 32, 0 ); 

	  hPlay = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON, 16, 16, 0 ); 

	  //ret = SendDlgItemMessage(hDlg, IDC_BT, STM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hBTIcon);
	  //ret = SendDlgItemMessage(hDlg, IDC_BT, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIconLarge);
      ret = SendDlgItemMessage(hDlg, IDC_BT, STM_SETIMAGE, IMAGE_ICON, (LPARAM)NULL);
      //Sleep(5000);
	  //ret = SendDlgItemMessage(hDlg, IDC_BT, STM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hBTIcon);

	  ret = SendDlgItemMessage(hDlg, IDC_PLAY_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)NULL);
	  //ret = SendDlgItemMessage(hDlg, IDC_BUTTON_PLAY, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hPlay);

	  DestroyIcon(g_hBTIcon);
	  DestroyIcon(hIconLarge);
	  DestroyIcon(hPlay);
#endif

      // Check if Ext Audio Present
     if( queryExtAudio(&bExtAudio) )
	 {
      BOOL  bMute;   
      //turnExtAudioOnOff(FALSE);
      if( bExtAudio == TRUE )
	  {
       
	   gExtMic = EXT_MIC;

       SendDlgItemMessage(hDlg, IDC_CHECK_EXT_AUDIO, BM_SETCHECK, (WPARAM)BST_CHECKED , 0); 
	   if( getExtMicMute(&bMute) )
	   {
        if( bMute == TRUE )
         SendDlgItemMessage(hDlg, IDC_RECORD_MUTE, BM_SETCHECK, (WPARAM)BST_CHECKED , 0); 
		else
         SendDlgItemMessage(hDlg, IDC_RECORD_MUTE, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);  
	   }
	  }
	  else
	  {
       SendDlgItemMessage(hDlg, IDC_CHECK_EXT_AUDIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0); 
	   if( getMicMute(&bMute) )
	   {
        if( bMute == TRUE )
         SendDlgItemMessage(hDlg, IDC_RECORD_MUTE, BM_SETCHECK, (WPARAM)BST_CHECKED , 0); 
		else
         SendDlgItemMessage(hDlg, IDC_RECORD_MUTE, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);  
	   }
	  }
	 }
	 else
	 {
       gExtMic = INT_MIC; 
       SendDlgItemMessage(hDlg, IDC_CHECK_EXT_AUDIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);
	   EnableWindow( GetDlgItem( hDlg, IDC_CHECK_EXT_AUDIO ), FALSE );
	 }

		updateMicAnalogGainInSlider(hDlg, hDlgSliderMicAnalogGain, &micAnalogGainCurPos[gExtMic]);
		wsprintf( myTest, _T("%d"), sMicAnalogGainMax );
		SetDlgItemText(hDlg, IDC_VOL_MIC_ANALOG_MAX, myTest);

		updateMicDigitalGainInSlider(hDlg, hDlgSliderMicDigitalGain, &micDigitalGainCurPos[gExtMic]);
		wsprintf( myTest, _T("%d"), sMicDigitalGainMax );
		SetDlgItemText(hDlg, IDC_VOL_MIC_DIGITAL_MAX, myTest);

      /* 
      bRet = findLastPaired(&gBTAddr);
      if( bRet == TRUE )
	  {
        g_hBT = btAudioOpen(gBTAddr);
		g_hThr = CreateThread(0, 0, AudioThread, &g_hBT, 0, 0 );
	  }
	  */

	  if( g_hBT == NULL )
	   EnableWindow( GetDlgItem( hDlg, IDC_CHECK_BT_HS ), FALSE );

	  //btAudioClose();
      //g_hBT = NULL;
	
     BOOL  bMute;
	 bRet = getMicMute(&bMute);
	 bRet = getExtMicMute(&bMute);
	 bRet = getVoiceInMute(&bMute, FALSE);
	 bRet = getVoiceInMute(&bMute, TRUE);
	 bRet = getVoiceOutMute(&bMute, FALSE);
     bRet = getVoiceOutMute(&bMute, TRUE);
	 bRet = getBTPlaybackOutMute(&bMute);

	 EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_PLAY ), FALSE );
	 EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP ), FALSE );

	 EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_RECORDING ), TRUE );
	 EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP_REC ), FALSE );
	 //EnableWindow( GetDlgItem( hDlg, IDC_SLIDER_PLAY_VOL ), FALSE );

	 HMIXER hMixer = openMixer(hDlg);


	  //g_hThr = CreateThread(0, 0, RunnerThread, &g_hDlg, 0, 0 );

     return TRUE; 
	} 

	case WM_NOTIFY:
	{
     LPNMHDR  lpNotifyMsg;
	 int      pos;
	 TCHAR    myTest[32];

     lpNotifyMsg = (LPNMHDR)lparam;

	 if( lpNotifyMsg->hwndFrom == hDlgSlider )
	 {
      DWORD  volVerify;
      pos = SendDlgItemMessage(hDlg, IDC_SLIDER_PLAY_VOL, TBM_GETPOS, 0 ,0);

	  if( pos != curPos )
	  {
        curPos = pos;

		wsprintf( myTest, _T("%d%%"), curPos );
		SetDlgItemText( hDlg, IDC_VOLUME_BOX, myTest );

        volLeft = (curPos * 0x0000FFFF) / maxRange;
		volRight = volLeft;

		volOut = volLeft | (volRight << 16);
		//mmres = waveOutSetVolume((HWAVEOUT)devId, volOut); 
		if( g_pl.m_hwo )
		{
		 mmres = waveOutSetVolume(g_pl.m_hwo, volOut); 
		 mmres = waveOutGetVolume(g_pl.m_hwo, &volVerify);
		
		 if( volOut != volVerify )
		 {
          RETAILMSG(1, (L"DIGVOICE:%S, waveOutSetVolume fails\r\n",__FUNCTION__)); 
		 }
		}
	  }
	 }

	 if( lpNotifyMsg->hwndFrom == hDlgMasterSlider )
	 {
      DWORD  volVerify;
      pos = SendDlgItemMessage(hDlg, IDC_SLIDER_PLAY_MASTER_VOL, TBM_GETPOS, 0 ,0);

	  if( pos != curPos )
	  {
        curPos = pos;

		wsprintf( myTest, _T("%d%%"), curPos );
		SetDlgItemText( hDlg, IDC_MASTER_VOLUME_BOX, myTest );

        masterVolLeft = (curPos * 0x0000FFFF) / maxRange;
		masterVolRight = masterVolLeft;

		volOut = masterVolLeft | (masterVolRight << 16);
		//mmres = waveOutSetVolume((HWAVEOUT)devId, volOut); 
		mmres = waveOutSetVolume((HWAVEOUT)devId, volOut); 
		mmres = waveOutGetVolume((HWAVEOUT)devId, &volVerify);
		if( volOut != volVerify )
		{
         RETAILMSG(1, (L"DIGVOICE:%S, waveOutSetVolume fails\r\n",__FUNCTION__)); 
		}
	  }
	 }

#if 1
	 if( ( lpNotifyMsg->hwndFrom == hDlgSliderMicAnalogGain ) && ( gAnSliderNotifyOmit == FALSE) )
	 {
      int     OutPos;
	  UINT16  aGain;
	  DWORD   dwStep, dwTicks, dwRangeMin, dwRangeMax, rmnd;

	  OutPos = SendMessage(hDlgSliderMicAnalogGain, TBM_GETPOS, 0 , 0);

	  if( OutPos != micAnalogGainCurPos[gExtMic] )
	  {
        micAnalogGainCurPos[gExtMic] = OutPos;

		// TBM_GETRANGEMIN 
		dwRangeMin = SendMessage(hDlgSliderMicAnalogGain, TBM_GETRANGEMIN, 0 , 0);
		dwRangeMax = SendMessage(hDlgSliderMicAnalogGain, TBM_GETRANGEMAX, 0 , 0);
		dwTicks = SendMessage(hDlgSliderMicAnalogGain, TBM_GETNUMTICS, 0 , 0);  
        dwStep =  (dwRangeMax - dwRangeMin)/(dwTicks-1);
 
		rmnd = micAnalogGainCurPos[gExtMic] % dwStep;
		if( rmnd != 0 )
        {
          if( rmnd < (dwStep/2) )
		  {
           aGain = ( (UINT16)micAnalogGainCurPos[gExtMic]/dwStep) * dwStep;
		  }
		  else
		  {
           aGain = ( ((UINT16)micAnalogGainCurPos[gExtMic]/dwStep) + 1) * dwStep;
		  }
		}
		else
         aGain = micAnalogGainCurPos[gExtMic];

		wsprintf( myTest, _T("%d"), aGain );
	    SetDlgItemText( hDlg, IDC_VOLUME_MIC_ANALOG, myTest );

		if( gExtMic == INT_MIC )
		  putMicAnalogGain(aGain);
		else
          putExtMicAnalogGain(aGain);

	  }

	 }

	 if( ( lpNotifyMsg->hwndFrom == hDlgSliderMicDigitalGain ) && ( gDigSliderNotifyOmit == FALSE) )
	 {
      int     OutPos, OutPosOld;
	  int     dGain;
	  int     micDigitalGainCurPosOld;

	  OutPos = SendMessage(hDlgSliderMicDigitalGain, TBM_GETPOS, 0 , 0);

	  OutPosOld = OutPos;
	  micDigitalGainCurPosOld = micDigitalGainCurPos[gExtMic];

	  if( OutPos != micDigitalGainCurPos[gExtMic] )
	  {
        micDigitalGainCurPos[gExtMic] = OutPos;

		//dGain = (micDigitalGainCurPos * (sMicDigitalGainMax - sMicDigitalGainMin)) / micDigitalGainMaxRange;
		//dGain += sMicDigitalGainMin;
		dGain = micDigitalGainCurPos[gExtMic]; 
		wsprintf( myTest, _T("%d"), dGain );
	    SetDlgItemText( hDlg, IDC_VOLUME_MIC_DIGITAL, myTest );

		RETAILMSG(1, (L"DIGVOICE:%S, dGain = %d\r\n",__FUNCTION__, dGain)); 

		if( gExtMic == INT_MIC )
		  putMicDigitalGain(dGain);
		else
         putExtMicDigitalGain(dGain);
	  }

	 }
#endif
     return TRUE; 
	}

	case WM_COMMAND:
	{
      switch( LOWORD(wParam) )
	   {
		 case IDCANCEL:
			//btAudioClose();
		    WaitForSingleObject(g_hThr, INFINITE);
		    //CloseHandle(g_hThr);
            g_hBT = NULL;

            g_pl.Stop();
			g_rec.Stop();
			deEnumMixer();
			//waveOutClose(hwo);
			closeMixer();
			CloseHandle(g_hThr);
			g_hThr = 0;
			EndDialog(hDlg, LOWORD(wParam));
			// Close Com port too ...
			//CloseHandle( g_hThr );
            //CloseHandle(g_hCom);

		 return TRUE;

		 //////////////////////////// PLAY Controls ////////////////////////
         case IDC_BUTTON_FILE:
		  {
			BOOL         ret;
  		    //wave_file waveFile;
			TCHAR file[256] = L"*.wav";
			TCHAR dir[256] = TEXT("\\.");

			//EnableWindow( GetDlgItem(hDlg, IDC_BUTTON_FILE), 0);

			ofnPlay.lStructSize = sizeof(ofnPlay);
			ofnPlay.hwndOwner = hDlg;
			ofnPlay.lpstrFilter = L"Waveform Audio(*.wav)\0*.wav\0\0";
			ofnPlay.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
			ofnPlay.lpstrTitle  = _T("Choose Playing File");
			ofnPlay.lpstrFile   = file;
			ofnPlay.lpstrInitialDir = dir;
			ofnPlay.nMaxFile    = sizeof(file);
			ret = GetSaveFileName(&ofnPlay);
			if( ret != FALSE )
             gPlayFileOpenStatus = TRUE;
			else
             gPlayFileOpenStatus = FALSE;

  		    // Button debounce
            Sleep(100);

		    if( gPlayFileOpenStatus == TRUE )
			{
             EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_PLAY ), TRUE );
             EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP ), TRUE );
			}

           return TRUE;
		  }

		 case IDC_BUTTON_PLAY:
		  {
           TCHAR    myTest[32];
           
		   EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_PLAY ), FALSE );

		   switch( gPlayState )
		   {
		    case  FILE_SILENT:
			 {
               if( gPlayFileOpenStatus == TRUE )
			   {
                 gPlayState = FILE_PLAYING;
				 g_pl.m_hDlg = hDlg;
				 EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_FILE ), FALSE );
                 g_pl.Start( ofnPlay.lpstrFile );
				 if( g_pl.m_hwo )
				   mmres = waveOutSetVolume(g_pl.m_hwo, volOut); 
				 //EnableWindow( GetDlgItem( hDlg, IDC_SLIDER_PLAY_VOL ), TRUE );
				 //gPlayState = FILE_SILENT;
			   }
               
			 } break;

		    case  FILE_PLAYING:
			 {
               if( gPlayFileOpenStatus == TRUE )
			   {
                 gPlayState = FILE_PAUSE;
				 g_pl.Pause();

 		         wsprintf( myTest, _T("Pause") );
		         SetDlgItemText( hDlg, IDC_BUTTON_PLAY, myTest );
			   }
              
			 } break;

		    case  FILE_PAUSE:
			 {
               if( gPlayFileOpenStatus == TRUE )
			   {
                 gPlayState = FILE_PLAYING;
				 g_pl.Restart();
				 //gPlayState = FILE_SILENT;

 		         wsprintf( myTest, _T("Play") );
		         SetDlgItemText( hDlg, IDC_BUTTON_PLAY, myTest );
			   }
              
			 } break;

			default: break;             
		   }

  		    // Button debounce
            Sleep(100);

		   EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_PLAY ), TRUE );

           return TRUE;
		  }

  	     case IDC_BUTTON_STOP:
		  {
            TCHAR    myTest[32];

			
               if( gPlayFileOpenStatus == TRUE )
			   {
                 if( gPlayState == FILE_PAUSE )
				 {
  		          wsprintf( myTest, _T("Play") );
		          SetDlgItemText( hDlg, IDC_BUTTON_PLAY, myTest );
				  // release pause state
				  g_pl.Restart();
				 }

				 EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP ), FALSE );
				 EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_PLAY ), FALSE );
				 //EnableWindow( GetDlgItem( hDlg, IDC_SLIDER_PLAY_VOL ), FALSE );
                 gPlayState = FILE_SILENT;
				 g_pl.Stop();

  		         // Button debounce
                 Sleep(100);

				 EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_FILE ), TRUE );

 		         wsprintf( myTest, _T("Play") );
		         SetDlgItemText( hDlg, IDC_BUTTON_PLAY, myTest );
			   }

           return TRUE;
		  }

		 case  IDC_PLAYBACK_MUTE:
		  {
           BOOL  bRet;
		   //unsigned long lVolOut1, lVolOut2;

		   if( g_pl.m_hwo )
		   {
            if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
			{
             waveOutGetVolume(g_pl.m_hwo, &lVolOut1);
             // Mute Playback
			  //bRet = setSpkMute(TRUE);
			  waveOutSetVolume(g_pl.m_hwo, 0);
			}
			else
			{
             // Un-Mute Playback
			 // bRet = setSpkMute(FALSE);
              waveOutSetVolume(g_pl.m_hwo, lVolOut1);
			}
		   }
			return TRUE;
		  }

		 case  IDC_PLAYBACK_MASTER_MUTE:
		  {
           BOOL  bRet;
		   
            if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
			{
             waveOutGetVolume(0, &lVolOut2);
             // Mute Playback
			  //bRet = setSpkMute(TRUE);
			  waveOutSetVolume(0, 0);
			}
			else
			{
             // Un-Mute Playback
			 // bRet = setSpkMute(FALSE);
              waveOutSetVolume(0, lVolOut2);
			}

			return TRUE;
		  }

        //////////////////////////// Record Controls ////////////////////////////
#if 1
         case IDC_BUTTON_REC_FILE:
		  {
			BOOL         ret;
  		    //wave_file waveFile;
			TCHAR recfile[256] = L"*.wav";
			TCHAR recdir[256] = TEXT("\\.");

			//EnableWindow( GetDlgItem(hDlg, IDC_BUTTON_FILE), 0);

					ofnRec.lStructSize = sizeof(ofnRec);
					ofnRec.hwndOwner = hDlg;
					ofnRec.lpstrFilter = L"Waveform Audio(*.wav)\0*.wav\0\0";
					ofnRec.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
					ofnRec.lpstrTitle  = _T("Choose Record File");
					ofnRec.lpstrFile   = recfile;
					ofnRec.lpstrInitialDir = recdir;
					ofnRec.nMaxFile    = sizeof(recfile);
					ret = GetSaveFileName(&ofnRec);
					if( ret != FALSE )
                     gRecFileOpenStatus = TRUE;
					else
                     gRecFileOpenStatus = FALSE;

  		     // Button debounce
             Sleep(100);

             if( gRecFileOpenStatus == TRUE )
			 {
	          EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_RECORDING ), TRUE );
	          EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP_REC ), TRUE );
			 }

					//ret = GetLastError();
           return TRUE;
		  }

        //IDC_BUTTON_RECORDING
         case IDC_BUTTON_RECORDING:
		  {
           EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP_REC ), FALSE );

		   switch(gRecState)
		   {
		     case REC_OFF: 
			  {
        		if( gRecFileOpenStatus == TRUE ) 
		         {
		          EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_REC_FILE ), FALSE );
		         }

				EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_RECORDING ), FALSE );

				wfxRec.cbSize = 0;
				wfxRec.wFormatTag = WAVE_FORMAT_PCM;
				wfxRec.wBitsPerSample = 16;
				//wfxRec.nSamplesPerSec = GetDlgItemInt(w, IDC_EDIT_SR, 0, 0);
				wfxRec.nSamplesPerSec = 44100;
				wfxRec.nChannels = 1; /*(BST_CHECKED == SendMessage(GetDlgItem(w, IDC_CHECK_STEREO), BM_GETCHECK, 0, 0))?2:1;*/
				wfxRec.nBlockAlign = wfxRec.nChannels * wfxRec.wBitsPerSample / 8;
				wfxRec.nAvgBytesPerSec = wfxRec.nBlockAlign * wfxRec.nSamplesPerSec;

#ifdef  VU_METER
				g_rec.m_hDlg = hDlg;
#endif
				if( gRecFileOpenStatus == TRUE )
				  g_rec.Start( ofnRec.lpstrFile, &wfxRec );
				else
				{
                 // g_rec.Start(&wfxRec);
				}

				updateMicAnalogGainInSlider(hDlg, hDlgSliderMicAnalogGain, &micAnalogGainCurPos[gExtMic]);
				updateMicDigitalGainInSlider(hDlg, hDlgSliderMicDigitalGain, &micDigitalGainCurPos[gExtMic]);
                
				gRecState = REC_ON;

			  } break;

		     case REC_ON: 
			  {

			  } break;

			 default: break;
		   }


  		   // Button debounce
           Sleep(100);
           EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP_REC ), TRUE );

           return TRUE;
		  }
 
         // IDC_BUTTON_STOP_REC
         case IDC_BUTTON_STOP_REC:
		  {
           gRecState = REC_OFF;
		   //gRecFileOpenStatus = FALSE;
           g_rec.Stop();
		   EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_REC_FILE ), TRUE );
		   if( gRecFileOpenStatus == TRUE )
 	         EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_RECORDING ), FALSE );
		   else
             EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_RECORDING ), TRUE );

		   gRecFileOpenStatus = FALSE;

	       EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP_REC ), FALSE );

		   return TRUE;
		  }
#endif

		 case IDC_RECORD_MUTE:
		  {
           BOOL  bRet;
           BOOL  bExtMic = FALSE;

		   bRet = queryExtAudio(&bExtMic);

           if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
		   {
			 if( bExtMic )
               bRet = setExtMicMute(TRUE); 
			 else
               bRet = setMicMute(TRUE); 
		   }
		   else
		   {
			 if( bExtMic )
               bRet = setExtMicMute(FALSE); 
			 else
               bRet = setMicMute(FALSE); 
		   }

	  	   //updateMicAnalogGainInSlider(hDlg, hDlgSliderMicAnalogGain, &micAnalogGainCurPos[gExtMic]);
		   //updateMicDigitalGainInSlider(hDlg, hDlgSliderMicDigitalGain, &micDigitalGainCurPos[gExtMic]);

		   return TRUE;
		  }

		  
      
#if 1
		 case IDC_CHECK_EXT_AUDIO: 
		  {
            BOOL  bMute;
           //BOOL  bret;
		   //LONG         ret;
		   //HICON        hBTIcon;

		   
           if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
		   {
              //hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_ICON), IMAGE_ICON, 16, 16, 0 ); 

             //ret = SendDlgItemMessage(hDlg, IDC_BT, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);
			 //if( ret == 0 )
             //  ret = GetLastError();

             if( turnExtAudioOnOff(TRUE) )
			 {
              gExtMic = EXT_MIC;
	  	      updateMicAnalogGainInSlider(hDlg, hDlgSliderMicAnalogGain, &micAnalogGainCurPos[gExtMic]);
			  updateMicDigitalGainInSlider(hDlg, hDlgSliderMicDigitalGain, &micDigitalGainCurPos[gExtMic]);

			   if( getExtMicMute(&bMute) )
			   {
				if( bMute == TRUE )
				 SendDlgItemMessage(hDlg, IDC_RECORD_MUTE, BM_SETCHECK, (WPARAM)BST_CHECKED , 0); 
				else
                 SendDlgItemMessage(hDlg, IDC_RECORD_MUTE, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0); 
			   }
			 }
			 else
			 {
               SendDlgItemMessage(hDlg, IDC_CHECK_EXT_AUDIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);
			   EnableWindow( GetDlgItem( hDlg, IDC_CHECK_EXT_AUDIO ), FALSE );
			 }
		   }
		   else
		   {
             //hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_ICON_GREY), IMAGE_ICON, 16, 16, 0 );  
             //ret = SendDlgItemMessage(hDlg, IDC_BT, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);


             gExtMic = INT_MIC;
             turnExtAudioOnOff(FALSE);  

			 updateMicAnalogGainInSlider(hDlg, hDlgSliderMicAnalogGain, &micAnalogGainCurPos[gExtMic] );
			 updateMicDigitalGainInSlider(hDlg, hDlgSliderMicDigitalGain, &micDigitalGainCurPos[gExtMic]);

			   if( getMicMute(&bMute) )
			   {
				if( bMute == TRUE )
				 SendDlgItemMessage(hDlg, IDC_RECORD_MUTE, BM_SETCHECK, (WPARAM)BST_CHECKED , 0); 
				else
                 SendDlgItemMessage(hDlg, IDC_RECORD_MUTE, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0); 
			   }
		   }

		   //DestroyIcon(hBTIcon);
           return TRUE;  
		  }
#endif

#ifdef BT_AG
         //////////////////////////// Bluetooth Playback
         case IDC_CHECK_BT_HS: 
		  {
           LONG         ret;
           BOOL          bRet;
		   HICON        hBTIcon = NULL;
		   //HANDLE       hBT;

		   
           if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
		   {
             hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_ICON), IMAGE_ICON, 16, 16, 0 ); 

             EnableWindow( GetDlgItem( hDlg, IDC_CHECK_BT_HS ), FALSE );
			 EnableWindow( GetDlgItem( hDlg, IDC_CHECK_BT_HS ), TRUE );

             if( g_hBT != NULL )
			 {
              bRet = btAudioConnect(BT_AG_HS_ACL|BT_AG_HS_SCO);
			  if( bRet == TRUE )
			  {
			   ret = SendDlgItemMessage(hDlg, IDC_BT, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);
			   turnBTPlbOnOff(TRUE);
			  }
			  else
               SendDlgItemMessage(hDlg, IDC_CHECK_BT_HS, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);
			 }
			 else
			 {
              SendDlgItemMessage(hDlg, IDC_CHECK_BT_HS, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);
			 }
  	       }
		   else
		   {
			 turnBTPlbOnOff(FALSE);   
			 bRet = btAudioDisconnect(BT_AG_HS_ACL|BT_AG_HS_SCO);

             hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_ICON_GREY), IMAGE_ICON, 16, 16, 0 );  
             ret = SendDlgItemMessage(hDlg, IDC_BT, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);
			// ret = SendDlgItemMessage(hDlg, IDC_BT, STM_SETIMAGE, IMAGE_ICON, (LPARAM)NULL);
		   }

		   DestroyIcon(hBTIcon);

           return TRUE;  
		  }
#endif /* BT_AG */

	   }

     return TRUE; 
	}

	case MM_MIXM_LINE_CHANGE:
	{
      HMIXER  hMixer;
	  DWORD  dwControlID;
	  DWORD  dwLineID;

	  hMixer = (HMIXER)wParam;
	  dwLineID = lparam;

	  return TRUE; 
	}

	case MM_MIXM_CONTROL_CHANGE:
	{
      HMIXER  hMixer;
	  DWORD   dwControlID;
	  DWORD   dwLineID;

	  hMixer = (HMIXER)wParam;
	  dwControlID = lparam;

	  return TRUE; 
	}

	// End PlayBack
	case WM_USER:
	{
      // End Playback
      gPlayState = FILE_SILENT;
	  EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_PLAY ), FALSE );
	  EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP ), FALSE );
	  EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_FILE ), TRUE );
	  //EnableWindow( GetDlgItem( hDlg, IDC_SLIDER_PLAY_VOL ), FALSE );
      return TRUE; 
	}

	// End Recording, epecially need when rec file size limitation occur
	case WM_USER+1:
	{
      // End Recording
      RETAILMSG(1, (L"DIGVOICE:%S, End Recording, gRecState = %d\r\n",__FUNCTION__,gRecState));  
      if( gRecState == REC_ON )
	  {
		gRecState = REC_OFF;

		//EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_REC_FILE ), TRUE );
 	    EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_RECORDING ), FALSE );
	    EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP_REC ), FALSE );
	  }
      return TRUE; 
	}

  } //switch(msg)

  return FALSE;
}


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	int  ret = -1;
	INITCOMMONCONTROLSEX slideBarControl;

    //ret = ListMixerDevices();
    // Store global instance handle.

//#if (DEF_TEST == 0)  
//    g_hInst = (HINSTANCE)0;
//#else
//	g_hInst = (HINSTANCE)1;
//#endif

//#ifdef  DEF_TEST
//    g_hInst = hInstance;
//#endif

    g_hInst = hInstance;

	slideBarControl.dwSize = sizeof(INITCOMMONCONTROLSEX);
	slideBarControl.dwICC = ICC_BAR_CLASSES;

	if( InitCommonControlsEx(&slideBarControl) )
 	 ret = DialogBox(hInstance, MAKEINTRESOURCE(IDD_PLAY_RECORD_DIALOG), 0, (DLGPROC)mainDlgProc);

	if( ret == -1 )
	{
     ret =	GetLastError();
	}

	return 0;
}





#if 0
VOID shadeRecButtons(HWND hDlg, BOOL bShade)
{
  EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_REC_FILE ), !bShade );
  EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_RECORDING ), !bShade );
  EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP_REC ), !bShade );
}
#endif


#if 1
static void updateMicAnalogGainInSlider(HWND hDlg, HWND hDlgSliderMicAnalogGain, int *pMicAnalogGainCurPos)
{
 UINT16   micAnalogGain;
 BOOL     bRet;
 
        gAnSliderNotifyOmit = TRUE;

        if( gExtMic == INT_MIC )
          bRet = getMicAnalogGain(&micAnalogGain, &sMicAnalogGainMin, &sMicAnalogGainMax);
		else
          bRet = getExtMicAnalogGain(&micAnalogGain, &sMicAnalogGainMin, &sMicAnalogGainMax);

		if( bRet )
		{
          if( (sMicAnalogGainMax - sMicAnalogGainMin) != 0 )
		  {
           SendMessage(hDlgSliderMicAnalogGain, TBM_SETRANGEMIN, TRUE , sMicAnalogGainMin);  
		   SendMessage(hDlgSliderMicAnalogGain, TBM_SETRANGEMAX, TRUE , sMicAnalogGainMax);  

		   // test
		   if( gExtMic == EXT_MIC )
		   {
             SendMessage(hDlgSliderMicAnalogGain, TBM_SETTICFREQ, 10 , 0);  
		   }
		   else
		   {
             SendMessage(hDlgSliderMicAnalogGain, TBM_SETTICFREQ, 1 , 0);  
		   }

		   *pMicAnalogGainCurPos = micAnalogGain;
           SendMessage(hDlgSliderMicAnalogGain, TBM_SETPOS, TRUE , *pMicAnalogGainCurPos); 

		   wsprintf( myTest, _T("%d"), micAnalogGain );
		   SetDlgItemText( hDlg, IDC_VOLUME_MIC_ANALOG, myTest );
		  }
			wsprintf( myTest, _T("%d"), sMicAnalogGainMax );
			SetDlgItemText(hDlg, IDC_VOL_MIC_ANALOG_MAX, myTest);

			wsprintf( myTest, _T("%d"), sMicAnalogGainMin );
			SetDlgItemText(hDlg, IDC_VOL_MIC_ANALOG_MIN, myTest);
		}

		gAnSliderNotifyOmit = FALSE;
}
#endif

#if 1
static void updateMicDigitalGainInSlider(HWND hDlg, HWND hDlgSliderMicDigitalGain, int *pMicDigitalGainCurPos)
{
 int      micDigitalGain;
 INT16    micDigitalGainS, micDigitalGainMinS, micDigitalGainMaxS;
 UINT16   micDigitalGainU, micDigitalGainMinU, micDigitalGainMaxU;
 BOOL     bRet;
 
        gDigSliderNotifyOmit = TRUE;

        if( gExtMic == INT_MIC )
		{
          bRet = getMicDigitalGain(&micDigitalGainU, &micDigitalGainMinU, &micDigitalGainMaxU);
		  micDigitalGain = micDigitalGainU;
		  sMicDigitalGainMin = micDigitalGainMinU;
          sMicDigitalGainMax = micDigitalGainMaxU;
		}
		else
		{
          bRet = getExtMicDigitalGain(&micDigitalGainS, &micDigitalGainMinS, &micDigitalGainMaxS);
		  micDigitalGain = micDigitalGainS;
		  sMicDigitalGainMin = micDigitalGainMinS;
          sMicDigitalGainMax = micDigitalGainMaxS;
		}

		if( bRet )
		{
          if( (sMicDigitalGainMax - sMicDigitalGainMin) != 0 )
		  {
           SendMessage(hDlgSliderMicDigitalGain, TBM_SETRANGEMIN, TRUE , sMicDigitalGainMin);  
		   SendMessage(hDlgSliderMicDigitalGain, TBM_SETRANGEMAX, TRUE , sMicDigitalGainMax);  

		   *pMicDigitalGainCurPos = micDigitalGain;
           SendMessage(hDlgSliderMicDigitalGain, TBM_SETPOS, TRUE , *pMicDigitalGainCurPos); 

		   wsprintf( myTest, _T("%d"), micDigitalGain );
		   SetDlgItemText( hDlg, IDC_VOLUME_MIC_DIGITAL, myTest );
		  }

			wsprintf( myTest, _T("%d"), sMicDigitalGainMax );
			SetDlgItemText(hDlg, IDC_VOL_MIC_DIGITAL_MAX, myTest);

			wsprintf( myTest, _T("%d"), sMicDigitalGainMin );
			SetDlgItemText(hDlg, IDC_VOL_MIC_DIGITAL_MIN, myTest);
		}

  gDigSliderNotifyOmit = FALSE;
}
#endif


DWORD WINAPI AudioThread(LPVOID lpParam)
{
 //DWORD       dwEvent; 
 //VOICE_LINE  *pVoiceLine;
 //HANDLE      hAgHS;
 INT32       iRet;
 //BOOL        bRet;
 //LONG        ret;
 AGHS_MSG    AgMsg;
 HICON       hBTIcon = NULL;
 AG_HS_EVENTS  AgEvent;
 //LINEMESSAGE lineMsg; 
 //VMODEM_CONTEXT_s  *pContext;

 //pContext = (VMODEM_CONTEXT_s *)lpParam;
 HANDLE        hBT = *(HANDLE *)lpParam;

   //hAgHS = *(HANDLE *)lpParam;


   while(1)
   {
    RETAILMSG(1, (L"DIGVOICE:%S, Enter  'MIC_AGHSWaitForEvent'\r\n",__FUNCTION__));
    iRet =  MIC_AGHSWaitForEvent( hBT, &AgMsg);
    //RETAILMSG(1, (L"DIGVOICE:%S, ret = %d, Event Msg = %d , %d\r\n",__FUNCTION__, iRet, AgMsg.hsEvent, AgMsg.dwParam));

	switch(iRet)
	{
	 case  AG_HS_OK:
	  {
       RETAILMSG(1, (L"DIGVOICE:%S, Valid Event = 0x%X ",__FUNCTION__, AgMsg.hsEvent));
	   AgEvent = (AG_HS_EVENTS)-1;

	   switch(AgMsg.hsEvent)
	   {
	    case  EV_AG_HS_BUTTON_PRESS:
	     {
           AgEvent = AG_HS_BUTTON_PRESS;
           RETAILMSG(1, (L"[BUTTON_PRESS]"));
		 } break;
		case  EV_AG_HS_MIC_GAIN:
	     {
           AgEvent = AG_HS_MIC_GAIN;
           RETAILMSG(1, (L"[MIC_GAIN]"));
		 } break;
		case  EV_AG_HS_SPEAKER_GAIN:
	     {
           AgEvent = AG_HS_SPEAKER_GAIN;
           RETAILMSG(1, (L"[SPEAKER_GAIN]"));
		 } break;
		case  EV_AG_HS_CTRL:
	     {
           AgEvent = AG_HS_CTRL;
           RETAILMSG(1, (L"[HS_CTRL]"));
		 } break;

		case  EV_AG_HS_AUDIO:
	     {
           AgEvent = AG_HS_AUDIO;
           RETAILMSG(1, (L"[HS_AUDIO]\r\n"));
		   //EnterCriticalSection(&gcsBTVoice);

		   //LeaveCriticalSection(&gcsBTVoice);

		 } break;

		default:
	     {
           RETAILMSG(1, (L"[UNKNOWN!]"));
		 } break;
	   }

	   if( AgEvent != (AG_HS_EVENTS)-1 )
	   {
        RETAILMSG(1, (L"DIGVOICE:%S, AG event sent: ev = %d, param = %d\r\n",__FUNCTION__, AgEvent, AgMsg.dwParam));
        /*
	    if( PostThreadMessage(pContext->destThreadId, WM_APP+1, (WPARAM)AgEvent, (LPARAM)AgMsg.dwParam) )
		  {
            RETAILMSG(1, (L"DIGVOICE:%S, AG event sent: ev = %d, param = %d\r\n",__FUNCTION__, AgEvent, AgMsg.dwParam));
		  }
		else
		  {
            RETAILMSG(1, (L"DIGVOICE:%S, AG event sent failure\r\n",__FUNCTION__));
		  }
		  */
	   }

        RETAILMSG(1, (L" param = %d\r\n", AgMsg.dwParam));

	  } break;

	 case  AG_HS_SERVICE_INVALID_PARAM:
	 case  AG_HS_MSG_CREATE_ERROR:
	 case  AG_HS_MSG_READ_ERROR:
	  {
       RETAILMSG(1, (L"DIGVOICE:%S, Wait Event Error = %d\r\n",__FUNCTION__, iRet));      
	   return(-2);
	  } break;

	 case  AG_HS_DEV_CLOSED:
	  {
       RETAILMSG(1, (L"DIGVOICE:%S, Dev Hanlde Closed!\r\n",__FUNCTION__));      
	   return(0);
	  } break;

	 default:
	  {
       RETAILMSG(1, (L"DIGVOICE:%S, Unknown Wait Event = %d\r\n",__FUNCTION__, iRet));      
	   return(-3);
	  } break;
	}

   }
  

  return(0);
}


// For testing purposes only. without proper quitting
#if 0
DWORD WINAPI RunnerThread(LPVOID lpParam)
{
 HWND          hDlg = *(HWND *)lpParam;
 TCHAR         thrStr[80];
 UINT32        thrCnt = 0;

 while(1)
  {
   Sleep(500);

   wsprintf( thrStr, _T("Play/Record (%d)"), thrCnt );
   SetWindowText(hDlg, thrStr);
   thrCnt++;
  }

  return(0);
}
#endif // 0