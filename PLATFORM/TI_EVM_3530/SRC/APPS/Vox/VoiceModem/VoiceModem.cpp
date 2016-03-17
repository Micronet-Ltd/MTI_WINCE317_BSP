// DigVoice.cpp : Defines the entry point for the application.
//

//#include "stdafx.h"
#include "VoiceModem.h"
#include <windows.h>
#include <service.h>
#include <commctrl.h>
#include <commdlg.h>
#include <pm.h>
#include <mmsystem.h>
#include <MicUserSdk.h>

#include "Mixer.h"
#include "modem.h"
#include "Ag.h"
#include "Line.h"
#include "bt.h"
#include "MuxMdm.h"
//#include "ag_hs_api.h"

#define MAX_LOADSTRING 100

#define NUM_OF_EDIT_CHARS  32


BT_ADDR             gvBTAddr = 0ll;
CRITICAL_SECTION    gcsBTVoice;            // Critical Section for Network Module

//==============================================================================================
//     enumerations
//==============================================================================================

typedef enum
{
	INT_MIC = 0,
    EXT_MIC,

	NUM_OF_MIC,

} MIC_TYPE_e;


HINSTANCE       g_hInst;
HANDLE          g_hBTAg = NULL;


HWND            g_hDlg;
TCHAR           myTest[32];


BOOL            gBtOn = FALSE;
BOOL            gBtConnect = FALSE;
BOOL            gBtConnectPrev = FALSE;

TCHAR           gEditLine[NUM_OF_EDIT_CHARS];

  static INT16  /*voiceInGain,*/ sVoiceInGainMin, sVoiceInGainMax;
  static INT16  /*voiceOutGain,*/ sVoiceOutGainMin, sVoiceOutGainMax;
  

VMODEM_CONTEXT_s  gVModem = 
{
 NULL,
 NULL,
 0,
 NULL,
 FALSE,
};

HANDLE g_hAgThr = NULL;
HANDLE g_hVModemThr = NULL;

LRESULT CALLBACK mainDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
BOOL  EndPlayBack( DWORD playerRet );

VOID shadeModemCheckBox(HWND hDlg, BOOL bShade);
VOID shadeMuxCheckBox(HWND hDlg, BOOL bShade);
VOID shadeModemDialButtons(HWND hDlg, BOOL bShade);
VOID shadeModemAPButtons(HWND hDlg, BOOL bShade);
VOID shadeModemBTButton(HWND hDlg, BOOL bShade);


static void updateVoiceInSlider(HWND hDlg, HWND hDlgSliderVoiceInGain, int *pCurPos, int voiceInMaxR);
static void updateVoiceOutSlider(HWND hDlg, HWND hDlgSliderVoiceOutGain, int *pCurPos, int voiceOutMaxR);
static void updateBTMicSlider(HWND hDlg, HWND hDlgSliderBTMic, int *pCurPos);
static void updateBTSpkSlider(HWND hDlg, HWND hDlgSliderBTSpk, int *pCurPos);
static VOID shadeBTSliders(HWND hDlg, BOOL bShade);


static VOID shadeModemBTConnectButton(HWND hDlg, BOOL bShade);

static BOOL  vModemAudioConnect(BOOL bGui);
static BOOL  vModemAudioDisconnect(BOOL bGui);
static BOOL  vModemDial(void);
static BOOL  vModemAnswer(void);
static BOOL  vModemhangUp(void);

//typedef BOOL                      (WINAPI *BTHAGNETREGEVENT)( DWORD, DWORD );

HWND g_hDlgSliderVoiceInGain; 
HWND g_hDlgSliderVoiceOutGain; 
HWND g_hDlgSliderMicAnalogGain; 
HWND g_hDlgSliderMicDigitalGain; 

HWND g_hDlgSliderBTMic;
HWND g_hDlgSliderBTSpk;

  static int    voiceInMaxRange, voiceInMinRange;
  static int    voiceOutMaxRange, voiceOutMinRange;
  static int    voiceInCurPos, voiceOutCurPos;
  static USHORT    btMicCurPos = 0;
  static USHORT    btSpkCurPos = 0;

BOOL  gVoiceInSliderNotifyOmit = FALSE;
BOOL  gVoiceOutSliderNotifyOmit = FALSE;


LRESULT CALLBACK mainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lparam)
{
  static HWND hDlgSlider; 

  static int count = 0;
  static int maxRange, minRange;
  static int curPos;

  static HWAVEOUT hwo = 0;
  //static HWAVEIN  hwi = 0;
  static UINT devId = 0;
  static unsigned long volOut = 0;
  static unsigned long volLeft, volRight;
  static WAVEFORMATEX  wfxRec;

  //Voice In/Out Volume Sliders

  // Microphone Analog/Digital gain sliders
  static int    micAnalogGainMaxRange, micAnalogGainMinRange;
  static int    micDigitalGainMaxRange, micDigitalGainMinRange;
  static int    micAnalogGainCurPos, micDigitalGainCurPos;


  count++;

  switch(msg)
  {
    case WM_INITDIALOG:
	{
     //TCHAR         myTest[32]; 
	 //WAVEFORMATEX  wfx;
	 BOOL          bRet;
	 MIXER_ERROR_CODES  mixErr;
	 //BOOL          micMute;
	 //LONG          iRet;
	 //DWORD         ret;
	 //DWORD         AgServiceStatus;
	 //DWORD         eventMask;
	 //BT_ADDR        bt;
	 //HANDLE         hAgHS; 
	 HICON   hBTIcon = NULL;




#if 0
#define  BT_PCM
   INT16  voiceInGain;
   INT16  voiceOutGain;
   BOOL   micMute;
  
  #ifndef  BT_PCM
	 bRet = turnVoiceOnOff(TRUE, 0);

	 bRet = putVoiceInGain(17, 0);
	 bRet = getVoiceInGain(&voiceInGain, &sVoiceInGainMin, &sVoiceInGainMax, 1);

	 bRet = putVoiceOutGain(513, 0);
	 bRet = getVoiceOutGain(&voiceOutGain, &sVoiceOutGainMin, &sVoiceOutGainMax);

	 bRet = setMicMute(1);
	 bRet = getMicMute(&micMute);

	 bRet = setMicMute(0);
	 bRet = getMicMute(&micMute);

     bRet = putMicAnalogGain(14);
	 bRet = getMicAnalogGain(&micAnalogGain, &sMicAnalogGainMin, &sMicAnalogGainMax);

	 bRet = putMicDigitalGain(18);
	 bRet = getMicDigitalGain(&micDigitalGain, &sMicDigitalGainMin, &sMicDigitalGainMax);
  #else
	 bRet = turnVoiceOnOff(TRUE, 1);

	 bRet = putVoiceInGain(-3, 1);
	 bRet = getVoiceInGain(&voiceInGain, &sVoiceInGainMin, &sVoiceInGainMax, 1);

	 bRet = putVoiceOutGain(-6, 1);
	 bRet = getVoiceOutGain(&voiceOutGain, &sVoiceOutGainMin, &sVoiceOutGainMax, 1);

	 bRet = setBTVoiceInMute(1);
	 bRet = getBTVoiceInMute(&micMute);

	 bRet = setBTVoiceInMute(0);
	 bRet = getBTVoiceInMute(&micMute);

	 bRet = setBTVoiceOutMute(1);
	 bRet = getBTVoiceOutMute(&micMute);

	 bRet = setBTVoiceOutMute(0);
	 bRet = getBTVoiceOutMute(&micMute);

  #endif
#endif

	 //getVoiceOutGain(&voiceOutGain, &sVoiceOutGainMin, &sVoiceOutGainMax);
	 //setMicMute(1);
	 //getMicMute(&micMute);

	// HKEY hk;
	 //LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\device", 0, KEY_READ, &hk);

	 //BthEnumDevices (NULL, EnumCallback);

	 //HANDLE  hServ;
	 //DWORD   dwErr;
	 //HINSTANCE  hInstance;
     //HMODULE    hModule;
     //WIN32_FIND_DATA fd;
     //HANDLE hFind;

#if 0
     hFind = FindFirstFile (TEXT("\\Windows\\btagsvc.*"), &fd);

     if (hFind != INVALID_HANDLE_VALUE) {

            do {

                  // show file name
                 
            } while (FindNextFile (hFind, &fd));

            FindClose (hFind);

     }


	 hModule = GetModuleHandle(L"\\Windows\\BTAGSVC.DLL");
	 if( hModule == NULL )
      dwErr = GetLastError();

	 hInstance = LoadLibrary( L"BTAGSVC.DLL" );
	 if( hInstance == NULL )
      dwErr = GetLastError();

	 hServ = ActivateService(L"BTAGSVC",0);
	 if( hServ == NULL )
      dwErr = GetLastError();
#endif

	 g_hDlg = hDlg;
	 gVModem.hDlg = g_hDlg;
	 mixErr = enumMixer();

	 bRet = isModemExists();
	 if( bRet == FALSE )
	 {
       SendDlgItemMessage(hDlg, IDC_VOICE_MODEM, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);
	   SendDlgItemMessage(hDlg, IDC_MUX_ON, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);   
	   shadeModemCheckBox(hDlg, TRUE);
	   shadeMuxCheckBox(hDlg, TRUE);
	 }
	 else
	 {
      shadeModemCheckBox(hDlg, FALSE);  
	  InitMux(&gVModem);
	 }

	 

	 // Workaround
	 turnVoiceOnOff(0, 0);

	 EnableWindow( GetDlgItem( hDlg, IDC_CALL_KEY ), FALSE );
	 EnableWindow( GetDlgItem( hDlg, IDC_END_KEY ), FALSE );

	 //SendDlgItemMessage(hDlg, IDC_CALL_KEY_EDIT_BOX, EN_UPDATE, (WPARAM)0, (LPARAM)0); 

        
		shadeModemDialButtons(hDlg, TRUE);
		
		

     g_hDlgSliderVoiceInGain  = GetDlgItem(hDlg, IDC_SLIDER_VOICE_VOL_IN);  
     g_hDlgSliderVoiceOutGain = GetDlgItem(hDlg, IDC_SLIDER_VOICE_VOL_OUT); 
	 
     g_hDlgSliderBTMic =  GetDlgItem(hDlg, IDC_SLIDER_BT_MIC); 
	 g_hDlgSliderBTSpk =  GetDlgItem(hDlg, IDC_SLIDER_BT_SPK); 

     voiceInMaxRange = SendMessage(g_hDlgSliderVoiceInGain, TBM_GETRANGEMAX, 0 ,0); 
     voiceInMinRange = SendMessage(g_hDlgSliderVoiceInGain, TBM_GETRANGEMIN, 0 ,0); 

     voiceOutMaxRange = SendMessage(g_hDlgSliderVoiceOutGain, TBM_GETRANGEMAX, 0 ,0); 
     voiceOutMinRange = SendMessage(g_hDlgSliderVoiceOutGain, TBM_GETRANGEMIN, 0 ,0); 

	 SendMessage(g_hDlgSliderBTMic, TBM_SETRANGEMIN, TRUE ,BT_VOL_MIN); 
     SendMessage(g_hDlgSliderBTMic, TBM_SETRANGEMAX, TRUE ,BT_VOL_MAX); 
     SendMessage(g_hDlgSliderBTSpk, TBM_SETRANGEMIN, TRUE ,BT_VOL_MIN); 
	 SendMessage(g_hDlgSliderBTSpk, TBM_SETRANGEMAX, TRUE ,BT_VOL_MAX); 

	 wsprintf( myTest, _T("%d"), BT_VOL_MIN );
	 SetDlgItemText(hDlg, IDC_BT_MIC_MIN, myTest);

	 wsprintf( myTest, _T("%d"), BT_VOL_MAX );
	 SetDlgItemText(hDlg, IDC_BT_MIC_MAX, myTest);

	 wsprintf( myTest, _T("%d"), BT_VOL_MIN );
	 SetDlgItemText(hDlg, IDC_BT_SPK_MIN, myTest);

	 wsprintf( myTest, _T("%d"), BT_VOL_MAX );
	 SetDlgItemText(hDlg, IDC_BT_SPK_MAX, myTest);

	 shadeBTSliders(hDlg, TRUE);

	 SendDlgItemMessage(hDlg, IDC_AP_3, BM_SETCHECK, (WPARAM)BST_CHECKED , 0);

	 modemSetAudioProfile(3);
	 //SendDlgItemMessage(hDlg, IDC_RADIO_2, BM_SETSTYLE, BS_AUTORADIOBUTTON , MAKELPARAM(TRUE, 0));

      // Check if Ext Audio Present
	 BOOL  bExtAudio;

     if( queryExtAudio(&bExtAudio) )
	 {
      //BOOL  bMute;   
      //turnExtAudioOnOff(FALSE);
      if( bExtAudio == TRUE )
	  {
       //IDC_EXT_AUDIO
	   //gExtMic = EXT_MIC; 
       SendDlgItemMessage(hDlg, IDC_EXT_AUDIO, BM_SETCHECK, (WPARAM)BST_CHECKED , 0); 
	  }
	  else
	  {
       SendDlgItemMessage(hDlg, IDC_EXT_AUDIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0); 
	  }
	 }
	 else
	 {
       //gExtMic = INT_MIC; 
       SendDlgItemMessage(hDlg, IDC_EXT_AUDIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);
	   EnableWindow( GetDlgItem( hDlg, IDC_EXT_AUDIO ), FALSE );
	 }

		updateVoiceInSlider(hDlg, g_hDlgSliderVoiceInGain, &voiceInCurPos, voiceInMaxRange);
		updateVoiceOutSlider(hDlg, g_hDlgSliderVoiceOutGain, &voiceOutCurPos, voiceOutMaxRange);

	  if( isBTAbsent() )
	  {
	    //shadeModemBTButton(hDlg, TRUE);
        shadeModemBTConnectButton(hDlg, TRUE);  

        hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_VOICE_OFF_ICON), IMAGE_ICON, 16, 16, 0 ); 
		SendDlgItemMessage(hDlg, IDC_BT_VOICE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);
		DestroyIcon(hBTIcon);
	  }
	  else
	  {
        //shadeModemBTButton(hDlg, FALSE);

		bRet = findLastPaired(&gvBTAddr);
        if( bRet == TRUE )
		{
         //gBtOn = TRUE;
         g_hBTAg = btAudioOpen(gvBTAddr);
		}

		if( g_hBTAg != NULL )
		{
		  hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_VOICE_ON_ICON), IMAGE_ICON, 16, 16, 0 ); 
		  SendDlgItemMessage(g_hDlg, IDC_BT_VOICE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);
		  DestroyIcon(hBTIcon);
		  
		  gVModem.hAgHS = g_hBTAg;
		  gBtOn = TRUE;
          g_hAgThr = CreateThread(0, 0, AgThread, &gVModem, 0, 0 );
		  // Need close thread on BT Off !
		}

		shadeModemBTConnectButton(hDlg, FALSE);
	  }
	  
	  g_hVModemThr = CreateThread(0, 0, VoiceModemThread, &gVModem, 0, &gVModem.destThreadId );
      
	  //IDD_VOICE_MODEM_DIALOG
	  //bRet = SetDlgItemText(hDlg, IDD_VOICE_MODEM_DIALOG, L"Ver 1.00");
	  //SendDlgItemMessage(hDlg, IDC_CALL_KEY_EDIT_BOX, EN_UPDATE, (WPARAM)0, (LPARAM)0); 
	  bRet = SetWindowText (hDlg, L"Voice Modem, Ver 1.13");

#if 0
      // Check if Ext Audio Present
		 if( turnExtAudioOnOff(TRUE) )
		 {
		  turnExtAudioOnOff(FALSE);
		 }
		 else
		 {
		   //SendDlgItemMessage(hDlg, IDC_CHECK_EXT_AUDIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);
		   //EnableWindow( GetDlgItem( hDlg, IDC_CHECK_EXT_AUDIO ), FALSE );
		 }
#endif
      return TRUE; 
	} 

	case WM_NOTIFY:
	{
     LPNMHDR  lpNotifyMsg;
	 //int      pos;
	 TCHAR    myTest[32];

     lpNotifyMsg = (LPNMHDR)lparam;

	 if( ( lpNotifyMsg->hwndFrom == g_hDlgSliderVoiceInGain ) && ( gVoiceInSliderNotifyOmit == FALSE) )
	 {
      int     InPos;
	  INT16  InGain;

	  InPos = SendMessage(g_hDlgSliderVoiceInGain, TBM_GETPOS, 0 , 0);

	  if( InPos != voiceInCurPos )
	  {
        voiceInCurPos = InPos;

		//InGain = sVoiceInGainMin + ((voiceInCurPos * (sVoiceInGainMax - sVoiceInGainMin)) / voiceInMaxRange);
		InGain = voiceInCurPos;
		wsprintf( myTest, _T("%d"), InGain );
	    SetDlgItemText( hDlg, IDC_VOLUME_VOICE_IN, myTest );
		putVoiceInGain(InGain, gBtConnect);
	  }

	 }

	 if( ( lpNotifyMsg->hwndFrom == g_hDlgSliderVoiceOutGain ) && ( gVoiceOutSliderNotifyOmit == FALSE) )
	 {
      int     OutPos;
	  INT16  OutGain;

	  OutPos = SendMessage(g_hDlgSliderVoiceOutGain, TBM_GETPOS, 0 , 0);

	  if( OutPos != voiceOutCurPos )
	  {
        voiceOutCurPos = OutPos;

		//OutGain = sVoiceOutGainMin + ((voiceOutCurPos * (sVoiceOutGainMax - sVoiceOutGainMin)) / voiceOutMaxRange);
		OutGain = voiceOutCurPos;
		wsprintf( myTest, _T("%d"), OutGain );
	    SetDlgItemText( hDlg, IDC_VOLUME_VOICE_OUT, myTest );
		putVoiceOutGain(OutGain, gBtConnect);
	  }

	 }

	 if( lpNotifyMsg->hwndFrom == g_hDlgSliderBTMic )
	 {
      USHORT  micPos;

	   micPos = (USHORT)SendMessage(g_hDlgSliderBTMic, TBM_GETPOS, 0 , 0);
       if( micPos != btMicCurPos )
	   {
        btMicCurPos = micPos; 
		wsprintf( myTest, _T("%d"), micPos );
	    SetDlgItemText( hDlg, IDC_BT_MIC, myTest );
		btSetMicVol(micPos);
	   }
	 }

	 if( lpNotifyMsg->hwndFrom == g_hDlgSliderBTSpk)
	 {
      USHORT  spkPos;

	   spkPos = (USHORT)SendMessage(g_hDlgSliderBTSpk, TBM_GETPOS, 0 , 0);
       if( spkPos != btSpkCurPos )
	   {
        btSpkCurPos = spkPos; 
		wsprintf( myTest, _T("%d"), spkPos );
	    SetDlgItemText( hDlg, IDC_BT_SPK, myTest );
		btSetSpkVol(spkPos);
	   }
	 }

     return TRUE; 
	}

	case WM_COMMAND:
	{
      switch( LOWORD(wParam) )
	   {
		 case IDCANCEL:
			
			btAudioClose();
		    WaitForSingleObject(g_hAgThr, INFINITE);
		    CloseHandle(g_hAgThr);
		    g_hAgThr = NULL;
 		    gvBTAddr = 0ll;

			DeInitMux();
			// Thread  exit too ?
            CloseHandle(g_hAgThr);
			turnVoiceOnOff(0, 0);
			modemOff(&gVModem);

			deEnumMixer();

			PostThreadMessage(gVModem.destThreadId, WM_QUIT,0,0);
			WaitForSingleObject(g_hVModemThr, INFINITE);
			g_hVModemThr = NULL;

			EndDialog(hDlg, LOWORD(wParam));
			DeleteCriticalSection(&gcsBTVoice);
			// Close Com port too ...
			//CloseHandle( g_hAgThr );

		 return TRUE;


        /////////////////////// MODEM Controls ////////////////////////////////// 
		 case  IDC_VOICE_MODEM:
		  {
            BOOL micMute;

                 if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
				 {
                    EnterCriticalSection(&gcsBTVoice);

					if( gVModem.bMux == FALSE )
					{
						shadeMuxCheckBox(hDlg, TRUE);
						shadeModemCheckBox(hDlg, TRUE); 
						shadeModemAPButtons(hDlg, TRUE);
						//shadeModemBTButton(hDlg, TRUE);
						
						if( modemOn(&gVModem) )
						{
						  turnVoiceOnOff(1, gBtConnect);

  						  shadeModemDialButtons(hDlg, FALSE);
						  //shadeModemBTButton(hDlg, FALSE);

	 					  updateVoiceInSlider(hDlg, g_hDlgSliderVoiceInGain, &voiceInCurPos, voiceInMaxRange);
						  updateVoiceOutSlider(hDlg, g_hDlgSliderVoiceOutGain, &voiceOutCurPos, voiceOutMaxRange);
						  //getMicMute(&micMute);
						}
						else
						{
						 shadeModemAPButtons(hDlg, FALSE);
						 shadeMuxCheckBox(hDlg, FALSE);
						 SendDlgItemMessage(hDlg, IDC_VOICE_MODEM, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);
						}

						shadeModemCheckBox(hDlg, FALSE); 
					}
					else  // with MUX
					{
                      MUX_ACTIVATION_ST_e  muxStatus;
					  BOOL                 errStatus = FALSE;

					  shadeMuxCheckBox(hDlg, TRUE);
					  shadeModemCheckBox(hDlg, TRUE); 
					  shadeModemAPButtons(hDlg, TRUE);
					  //shadeModemBTButton(hDlg, TRUE);

					  muxStatus = ActivateMux();
					  switch(muxStatus)
					  {
					   case  MUX_START_ACTIVATING: break;
					   //case  MUX_ACTIVATION_IN_PROGRESS:  // by other Application
					   case  MUX_ACTIVATED:
						{
                          if( modemOnMux(&gVModem) )
						  {
						   turnVoiceOnOff(1, gBtConnect);

  						   shadeModemDialButtons(hDlg, FALSE);
						   //shadeModemBTButton(hDlg, FALSE);

	 					   updateVoiceInSlider(hDlg, g_hDlgSliderVoiceInGain, &voiceInCurPos, voiceInMaxRange);
						   updateVoiceOutSlider(hDlg, g_hDlgSliderVoiceOutGain, &voiceOutCurPos, voiceOutMaxRange);
						   shadeModemCheckBox(hDlg, FALSE);
						  }
						  else
						  {
                           errStatus = TRUE;
						  }
						} break;

					   case  MUX_ACTIVATION_ERR: errStatus = TRUE; break;
					   default:  errStatus = TRUE; break;
					  }

					  if( errStatus == TRUE )
					  {
						 shadeModemAPButtons(hDlg, FALSE);
						 shadeMuxCheckBox(hDlg, FALSE);
						 SendDlgItemMessage(hDlg, IDC_VOICE_MODEM, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);
						 shadeModemCheckBox(hDlg, FALSE);
					  }
					}

  		            // Button debounce
                    Sleep(100);

					LeaveCriticalSection(&gcsBTVoice);
				 }
				 else
				 {
                    EnterCriticalSection(&gcsBTVoice);

					shadeMuxCheckBox(hDlg, TRUE);
                    shadeModemCheckBox(hDlg, TRUE);
					shadeModemDialButtons(hDlg, TRUE);
					//shadeModemBTButton(hDlg, TRUE);
					
					if( modemOff(&gVModem) )
					{
                      shadeModemAPButtons(hDlg, FALSE);
					  
					  if( turnVoiceOnOff(0, gBtConnect) )
						wsprintf( myTest, _T("Voice Off") );
					  else
                        wsprintf( myTest, _T("Voice Off failure") );

					  getMicMute(&micMute);
					}

					shadeModemCheckBox(hDlg, FALSE);
					//shadeModemBTButton(hDlg, FALSE);
					shadeMuxCheckBox(hDlg, FALSE);

					LeaveCriticalSection(&gcsBTVoice);

  		            // Button debounce
                    Sleep(100);
				 }

                 //SetDlgItemText( hDlg, IDC_STATUS1, myTest );

		   return TRUE;
		  }

		 case IDC_VOICE_IN_MUTE:
		  {
           BOOL  bRet;
		   INT16 lVoiceInGain1, lVoiceInGain2;

		    bRet = getVoiceInGain(&lVoiceInGain1, &sVoiceInGainMin, &sVoiceInGainMax, gBtConnect);

			if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
			 {
              bRet =  setVoiceInMute(TRUE, gBtConnect);
			 }
			else
			 {
              bRet =  setVoiceInMute(FALSE, gBtConnect);
			 }

			bRet = getVoiceInGain(&lVoiceInGain2, &sVoiceInGainMin, &sVoiceInGainMax, gBtConnect);

			if( lVoiceInGain1 != lVoiceInGain2 )
			{
              RETAILMSG(1, (L"DIGVOICE:%S, VoiceInGain changed!\r\n",__FUNCTION__));      
			}

		    return TRUE;
		  }
		   
           //getVoiceOutGain(&voiceOutGain, &sVoiceOutGainMin, &sVoiceOutGainMax, gBtConnect)
		 case IDC_VOICE_OUT_MUTE:
		  {
           BOOL  bRet;
		   INT16 lVoiceOutGain1, lVoiceOutGain2;

		    bRet = getVoiceOutGain(&lVoiceOutGain1, &sVoiceOutGainMin, &sVoiceOutGainMax, gBtConnect);

			if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
			 {
              bRet =  setVoiceOutMute(TRUE, gBtConnect);
			 }
			else
			 {
              bRet =  setVoiceOutMute(FALSE, gBtConnect);
			 }

			bRet = getVoiceOutGain(&lVoiceOutGain2, &sVoiceOutGainMin, &sVoiceOutGainMax, gBtConnect);

			if( lVoiceOutGain1 != lVoiceOutGain2 )
			{
              RETAILMSG(1, (L"DIGVOICE:%S, VoiceOutGain changed!\r\n",__FUNCTION__));      
			}

		    return TRUE;
		  }

		 case IDC_EXT_AUDIO: 
		  {
           //BOOL  bret;
		   //LONG         ret;
		   //HICON        hBTIcon;

		   
           if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
		   {
             //hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_ICON), IMAGE_ICON, 16, 16, 0 ); 

             //ret = SendDlgItemMessage(hDlg, IDC_BT, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);
			 //if( ret == 0 )
             //  ret = GetLastError();
             if( !gBtConnect )
			 {
				 if( turnExtAudioOnOff(TRUE) )
				 {
                    if( isModemOn() )
					{
                      turnVoiceOnOff(1, FALSE);
					}
				 }
				 else
				 {
				   SendDlgItemMessage(hDlg, IDC_EXT_AUDIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);
				   EnableWindow( GetDlgItem( hDlg, IDC_EXT_AUDIO ), FALSE );
				 }
			 }
		   }
		   else
		   {
			 if( !gBtConnect )
			 {
               if( turnExtAudioOnOff(FALSE) )
			   {
                    if( isModemOn() )
					{
                      turnVoiceOnOff(0, FALSE); 
                      turnVoiceOnOff(1, FALSE);
					}
			   }
			 }
             //hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_ICON_GREY), IMAGE_ICON, 16, 16, 0 );  
             //ret = SendDlgItemMessage(hDlg, IDC_BT, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);
		   }

		   //DestroyIcon(hBTIcon);
           return TRUE;  
		  }

#if 0
		 case IDC_BT_ON_OFF:
		  {
           BOOL    bRet;
		   LONG    ret;
		   HICON   hBTIcon = NULL;

		   EnterCriticalSection(&gcsBTVoice);

            if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
			 {
              
              if( !gBtOn )
			  {
               shadeModemCheckBox(hDlg, TRUE); 
			   //shadeModemBTButton(hDlg, TRUE);

               if( isModemOn() )
			   {
                
                //EnableWindow( GetDlgItem( hDlg, IDC_CALL_KEY ), FALSE );
	            //EnableWindow( GetDlgItem( hDlg, IDC_END_KEY ), FALSE );

                hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_VOICE_OFF_ICON), IMAGE_ICON, 16, 16, 0 ); 
				ret = SendDlgItemMessage(hDlg, IDC_BT_VOICE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);
				DestroyIcon(hBTIcon);
			   }

			    ....
				if( g_hBTAg != NULL )
				{
                  //hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_VOICE_CONNECT_ICON), IMAGE_ICON, 16, 16, 0 ); 
                  gBtOn = TRUE;
				  //gBtConnect = TRUE;
				  
				  //gVModem.hAgHS = g_hBTAg;
                  //g_hAgThr = CreateThread(0, 0, AgThread, &gVModem, 0, 0 );
				  // Need close thread on BT Off !

				  //ret = SendDlgItemMessage(hDlg, IDC_BT_VOICE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);
				  //DestroyIcon(hBTIcon);

				  shadeModemBTConnectButton(g_hDlg, FALSE);
				  //SendDlgItemMessage(hDlg, HS_AUDIO, BM_SETCHECK, (WPARAM)BST_CHECKED , 0);
				}
				else
				{
                 gBtOn = FALSE;
                 SendDlgItemMessage(hDlg, IDC_BT_ON_OFF, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);
				 hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_VOICE_OFF_ICON), IMAGE_ICON, 16, 16, 0 ); 
				 ret = SendDlgItemMessage(hDlg, IDC_BT_VOICE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);
				 DestroyIcon(hBTIcon);
				}

               shadeModemCheckBox(hDlg, FALSE);
			   //shadeModemBTButton(hDlg, FALSE);
			  }
			 }
			else
			 {
              if( gBtOn )
			  {
                //shadeModemBTButton(hDlg, TRUE);

				  gBtOn = FALSE;
				  gBtConnect = FALSE;
				  gBtConnectPrev = FALSE;

				  //gBtOn = FALSE;
				  //gBtConnect = FALSE;
				  shadeModemBTConnectButton(hDlg, TRUE);
				  SendDlgItemMessage(hDlg, HS_AUDIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);

                hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_VOICE_OFF_ICON), IMAGE_ICON, 16, 16, 0 ); 
				ret = SendDlgItemMessage(hDlg, IDC_BT_VOICE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);
				DestroyIcon(hBTIcon);

                if( isModemOn() )
				{
				  turnVoiceOnOff(0, TRUE);
                  turnVoiceOnOff(1, FALSE);
                  
	 			  updateVoiceInSlider(hDlg, g_hDlgSliderVoiceInGain, &voiceInCurPos, voiceInMaxRange);
				  updateVoiceOutSlider(hDlg, g_hDlgSliderVoiceOutGain, &voiceOutCurPos, voiceOutMaxRange);
				  //updateMicAnalogGainInSlider(hDlg, g_hDlgSliderMicAnalogGain, &micAnalogGainCurPos, micAnalogGainMaxRange);
				  //updateMicDigitalGainInSlider(hDlg, g_hDlgSliderMicDigitalGain, &micDigitalGainCurPos, micDigitalGainMaxRange);

                  //EnableWindow( GetDlgItem( hDlg, IDC_CALL_KEY ), TRUE );
	              //EnableWindow( GetDlgItem( hDlg, IDC_END_KEY ), TRUE );
				}

				//shadeModemBTButton(hDlg, FALSE);
				
			  }
			 }

            LeaveCriticalSection(&gcsBTVoice);

            return TRUE;  
		  }
#endif //0

         case HS_AUDIO:
		  {
           HICON   hBTIcon = NULL;
		   int     micGain, spkGain;
		   //LONG    ret;

		   shadeModemBTConnectButton(g_hDlg, TRUE); 

           EnterCriticalSection(&gcsBTVoice);
           if( gBtOn )  
		   {
             if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
			 {
              vModemAudioConnect(TRUE);
			  updateBTMicSlider(g_hDlg, g_hDlgSliderBTMic, (int *)&micGain);
			  updateBTSpkSlider(g_hDlg, g_hDlgSliderBTSpk, (int *)&spkGain);
			 }
			 else
			 {
              vModemAudioDisconnect(TRUE);
			 }
		   }
		   LeaveCriticalSection(&gcsBTVoice);

  		   // Button debounce
           Sleep(100);
		   shadeModemBTConnectButton(g_hDlg, FALSE); 

           return TRUE;  
		  }

		 case  IDC_AP_0:
		  {
           shadeModemAPButtons(hDlg, TRUE);

           if(IsDlgButtonChecked(hDlg, IDC_AP_0) == BST_CHECKED)
			 modemSetAudioProfile(0);

  		   // Button debounce
           Sleep(100);
		   shadeModemAPButtons(hDlg, FALSE);

           return TRUE;
		  }

		 case  IDC_AP_1:
		  {
           shadeModemAPButtons(hDlg, TRUE);

           if(IsDlgButtonChecked(hDlg, IDC_AP_1) == BST_CHECKED)
             modemSetAudioProfile(1);

  		   // Button debounce
           Sleep(100);
		   shadeModemAPButtons(hDlg, FALSE);

           return TRUE;
		  }

		 case  IDC_AP_2:
		  {
           shadeModemAPButtons(hDlg, TRUE);

           if(IsDlgButtonChecked(hDlg, IDC_AP_2) == BST_CHECKED)
             modemSetAudioProfile(2);

  		   // Button debounce
           Sleep(100);
		   shadeModemAPButtons(hDlg, FALSE);

           return TRUE;
		  }

		 case  IDC_AP_3:
		  {
           shadeModemAPButtons(hDlg, TRUE);

           if(IsDlgButtonChecked(hDlg, IDC_AP_3) == BST_CHECKED)
             modemSetAudioProfile(3);

  		   // Button debounce
           Sleep(100);
		   shadeModemAPButtons(hDlg, FALSE);

           return TRUE;
		  }


		 case  IDC_CALL_KEY:
		  {
		 	 // TCHAR editLine[32];
			  //LONG  len;
			  //size_t origsize; 
			  size_t convertedChars = 0;
			  //char   ATstring[NUM_OF_EDIT_CHARS-2];

			  shadeModemDialButtons(hDlg, TRUE);

			  EnterCriticalSection(&gcsBTVoice);
			  if( gBtConnect )
			  {
                LeaveCriticalSection(&gcsBTVoice);
                return TRUE;
			  }

			  PostThreadMessage(gVModem.destThreadId, WM_APP+2, (WPARAM)VMODEM_CALL_KEY, (LPARAM)0);
#if 0			  
              EnableWindow( GetDlgItem( hDlg, IDC_CALL_KEY ), FALSE );
	          EnableWindow( GetDlgItem( hDlg, IDC_END_KEY ), FALSE );

			  *(WORD *)gEditLine = NUM_OF_EDIT_CHARS - sizeof(TCHAR);
			  len = SendDlgItemMessage(hDlg, IDC_DIAL_EDIT_BOX, EM_GETLINE, 0 , (LPARAM)gEditLine);
			  //IDC_DIAL_EDIT_BOX

			  origsize = wcslen(gEditLine) + 1;
              wcstombs_s(&convertedChars, ATstring, origsize, gEditLine, _TRUNCATE);

			  //MIC_GSMVoiceCMD(g_hCom, GSM_VOICE_DIAL, ATstring);
			  modemVoiceDial(ATstring);

              EnableWindow( GetDlgItem( hDlg, IDC_END_KEY ), TRUE );
	          EnableWindow( GetDlgItem( hDlg, IDC_CALL_KEY ), TRUE );
#endif
			  LeaveCriticalSection(&gcsBTVoice);

  		      // Button debounce
			  shadeModemDialButtons(hDlg, FALSE);
              Sleep(100);

			  return TRUE;
		  }


		 case  IDC_END_KEY:
		  {
			  shadeModemDialButtons(hDlg, TRUE);

			  EnterCriticalSection(&gcsBTVoice);
			  if( gBtConnect )
			  {
                LeaveCriticalSection(&gcsBTVoice); 
                return TRUE;
			  }

			  PostThreadMessage(gVModem.destThreadId, WM_APP+2, (WPARAM)VMODEM_END_KEY, (LPARAM)0);
#if 0
              EnableWindow( GetDlgItem( hDlg, IDC_CALL_KEY ), FALSE );
	          EnableWindow( GetDlgItem( hDlg, IDC_END_KEY ), FALSE );

			  RETAILMSG(1, (L"DIGVOICE:%S, start Hang-Up\r\n",__FUNCTION__));

			  //MIC_GSMVoiceCMD(g_hCom, GSM_VOICE_HANG_UP, NULL);
			  modemVoiceHangUp();

			  RETAILMSG(1, (L"DIGVOICE:%S, stop Hang-Up\r\n",__FUNCTION__));

              EnableWindow( GetDlgItem( hDlg, IDC_END_KEY ), TRUE );
	          EnableWindow( GetDlgItem( hDlg, IDC_CALL_KEY ), TRUE );
#endif
			  LeaveCriticalSection(&gcsBTVoice);

  		      // Button debounce
			  shadeModemDialButtons(hDlg, FALSE);
              Sleep(100);

			  return TRUE;
		  }

#if 0
		 case  IDC_ANSWER:
		  {
			  EnterCriticalSection(&gcsBTVoice);
			  if( gBtConnect )
			  {
                LeaveCriticalSection(&gcsBTVoice);
                return TRUE;
			  }

	          EnableWindow( GetDlgItem( hDlg, IDC_ANSWER ), FALSE );
              EnableWindow( GetDlgItem( hDlg, IDC_CALL_KEY ), FALSE );
	          EnableWindow( GetDlgItem( hDlg, IDC_END_KEY ), FALSE );

			  //MIC_GSMVoiceCMD(g_hCom, GSM_VOICE_ANSWER, NULL);
			  modemVoiceAnswer();

	          EnableWindow( GetDlgItem( hDlg, IDC_END_KEY ), TRUE );
              EnableWindow( GetDlgItem( hDlg, IDC_CALL_KEY ), TRUE );
	          EnableWindow( GetDlgItem( hDlg, IDC_ANSWER ), TRUE );

			  LeaveCriticalSection(&gcsBTVoice);
			  return TRUE;
		  }
#endif
#if 0
		 case IDC_CHECK_EXT_AUDIO: 
		  {
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
              gExtMic = TRUE;
	  	      updateMicAnalogGainInSlider(hDlg, hDlgSliderMicAnalogGain, &micAnalogGainCurPos, micAnalogGainMaxRange);
			  updateMicDigitalGainInSlider(hDlg, hDlgSliderMicDigitalGain, &micDigitalGainCurPos, micDigitalGainMaxRange);
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


             gExtMic = FALSE;
             turnExtAudioOnOff(FALSE);  

			 updateMicAnalogGainInSlider(hDlg, hDlgSliderMicAnalogGain, &micAnalogGainCurPos, micAnalogGainMaxRange);
			 updateMicDigitalGainInSlider(hDlg, hDlgSliderMicDigitalGain, &micDigitalGainCurPos, micDigitalGainMaxRange);
		   }

		   //DestroyIcon(hBTIcon);
           return TRUE;  
		  }
#endif
		 case IDC_MUX_ON: 
		  {
           EnterCriticalSection(&gcsBTVoice);

           shadeModemCheckBox(hDlg, TRUE); 
           shadeMuxCheckBox(hDlg, TRUE);

           if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
		   {
             //MuxTurnOn(MUX_ACTIVATE);
              if( isModemOn() )
			  {
				SendDlgItemMessage(hDlg, IDC_MUX_ON, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);   
			  }
			  else
			  {
			   gVModem.bMux = TRUE;
			  }
		   }
		   else
		   {
             //MuxTurnOn(MUX_DEACTIVATE);
              if( isModemOn() )
			  {
				SendDlgItemMessage(hDlg, IDC_MUX_ON, BM_SETCHECK, (WPARAM)BST_CHECKED , 0);   
			  }
			  else
			  {
			   gVModem.bMux = FALSE;
			  }
		   }

		   // Button debounce
           Sleep(100);

		   shadeMuxCheckBox(hDlg, FALSE);
		   shadeModemCheckBox(hDlg, FALSE); 

		   LeaveCriticalSection(&gcsBTVoice);

           return TRUE;  
		  }


	   }

     return TRUE; 
	}

	case WM_USER:
	{
     // WPARAM wParam, LPARAM lparam
		  RETAILMSG(1, (L"DIGVOICE:%S, type = %d, value = %d\r\n",__FUNCTION__, wParam, lparam ));

			if (wParam == GSM710MESSAGETYPE_CONNECTIONSTATE)
			 {
               if( lparam == 1 )
			   {
                 EnterCriticalSection(&gcsBTVoice);

                 if( modemOnMux(&gVModem) )
				 {
						  turnVoiceOnOff(1, gBtConnect);

  						  shadeModemDialButtons(hDlg, FALSE);
						  //shadeModemBTButton(hDlg, FALSE);

	 					  updateVoiceInSlider(hDlg, g_hDlgSliderVoiceInGain, &voiceInCurPos, voiceInMaxRange);
						  updateVoiceOutSlider(hDlg, g_hDlgSliderVoiceOutGain, &voiceOutCurPos, voiceOutMaxRange);

				 }
				 else
				 {
				 	     shadeModemAPButtons(hDlg, FALSE);
						 shadeMuxCheckBox(hDlg, FALSE);
						 SendDlgItemMessage(hDlg, IDC_VOICE_MODEM, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);
				 }

				 shadeModemCheckBox(hDlg, FALSE);
				 LeaveCriticalSection(&gcsBTVoice);
                 RETAILMSG(1, (L"DIGVOICE:%S, GSM710MUX driver activated.\r\n",__FUNCTION__));
			   }
			   else
			   {
                    EnterCriticalSection(&gcsBTVoice);

					shadeMuxCheckBox(hDlg, TRUE);
                    shadeModemCheckBox(hDlg, TRUE);
					shadeModemDialButtons(hDlg, TRUE);
					//shadeModemBTButton(hDlg, TRUE);
					
					if( modemOff(&gVModem) )
					{
                      shadeModemAPButtons(hDlg, FALSE);
					  
					  turnVoiceOnOff(0, gBtConnect);
					  //getMicMute(&micMute);
					}

					shadeModemCheckBox(hDlg, FALSE);
					//shadeModemBTButton(hDlg, FALSE);
					shadeMuxCheckBox(hDlg, FALSE);

					SendDlgItemMessage(hDlg, IDC_VOICE_MODEM, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);

					LeaveCriticalSection(&gcsBTVoice);
                 
                 RETAILMSG(1, (L"DIGVOICE:%S, GSM710MUX driver deactivated.\r\n",__FUNCTION__));
			   }
			 }
			else if (wParam == GSM710MESSAGETYPE_MODEMSTATE)
			 {
				RETAILMSG(1, (L"DIGVOICE:%S, Please turn on the GPRS modem.\r\n",__FUNCTION__));
				if( Mic_GSM710ModemOn(TRUE) )
				{
                 SendModemOnToMux(); 
				}
				else
				{
                  EnterCriticalSection(&gcsBTVoice);

                  modemOff(&gVModem);
				  shadeModemAPButtons(hDlg, FALSE);
				  shadeMuxCheckBox(hDlg, FALSE);
				  shadeModemCheckBox(hDlg, FALSE);
				  SendDlgItemMessage(hDlg, IDC_VOICE_MODEM, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);

				  LeaveCriticalSection(&gcsBTVoice);
				}
			 }
			else if (wParam == GSM710MESSAGETYPE_POWERSTATE)
			{
				CEDEVICE_POWER_STATE power_state = (CEDEVICE_POWER_STATE)lparam;
				RETAILMSG(1, (L"DIGVOICE:%S, Power state was changed to %d\r\n",__FUNCTION__, power_state));
			}
			else if (wParam == GSM710MESSAGETYPE_CONNECTIONERROR)
			{
				RETAILMSG(1, (L"DIGVOICE:%S, Connection error = %\r\n",__FUNCTION__, lparam));
			}

      return TRUE; 
	}

	case WM_TIMER:
    {
      if(wParam == VMODEM_TIMEOUT_EV_0)
	  {
	    RETAILMSG(1, (L"DIGVOICE:%S, WM_TIMER : VMODEM_TIMEOUT_EV_0\r\n",__FUNCTION__));

        PostThreadMessage(gVModem.destThreadId, WM_APP+3, (WPARAM)wParam, (LPARAM)0);
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

	slideBarControl.dwSize = sizeof(INITCOMMONCONTROLSEX);
	slideBarControl.dwICC = ICC_BAR_CLASSES;

	g_hInst = hInstance;
	InitializeCriticalSection(&gcsBTVoice);

	// IDD_VOICE_MODEM_DIALOG
	if( InitCommonControlsEx(&slideBarControl) )
 	 ret = DialogBox(hInstance, MAKEINTRESOURCE(IDD_VOICE_MODEM_DIALOG), 0, (DLGPROC)mainDlgProc);

	if( ret == -1 )
	{
     ret =	GetLastError();
	}

	return 0;
}






VOID shadeModemCheckBox(HWND hDlg, BOOL bShade)
{
  EnableWindow( GetDlgItem( hDlg, IDC_VOICE_MODEM ), !bShade );
}

VOID shadeMuxCheckBox(HWND hDlg, BOOL bShade)
{
  EnableWindow( GetDlgItem( hDlg, IDC_MUX_ON ), !bShade );
}


VOID shadeModemDialButtons(HWND hDlg, BOOL bShade)
{

  EnableWindow( GetDlgItem( hDlg, IDC_CALL_KEY ), !bShade );
  EnableWindow( GetDlgItem( hDlg, IDC_END_KEY ), !bShade );

  EnableWindow( GetDlgItem( hDlg, IDC_SLIDER_VOICE_VOL_IN ), !bShade );
  EnableWindow( GetDlgItem( hDlg, IDC_SLIDER_VOICE_VOL_OUT ), !bShade );
  EnableWindow( GetDlgItem( hDlg, IDC_VOLUME_VOICE_IN ), !bShade );
  EnableWindow( GetDlgItem( hDlg, IDC_VOLUME_VOICE_OUT ), !bShade );
}

VOID shadeModemAPButtons(HWND hDlg, BOOL bShade)
{
  EnableWindow( GetDlgItem( hDlg, IDC_AP_0 ), !bShade );
  EnableWindow( GetDlgItem( hDlg, IDC_AP_1 ), !bShade );
  EnableWindow( GetDlgItem( hDlg, IDC_AP_2 ), !bShade );
  EnableWindow( GetDlgItem( hDlg, IDC_AP_3 ), !bShade );
}

VOID shadeModemBTButton(HWND hDlg, BOOL bShade)
{
  EnableWindow( GetDlgItem( hDlg, IDC_BT_ON_OFF ), !bShade );
}

static VOID shadeModemBTConnectButton(HWND hDlg, BOOL bShade)
{
  EnableWindow( GetDlgItem( hDlg, HS_AUDIO ), !bShade );
}

static VOID shadeBTSliders(HWND hDlg, BOOL bShade)
{
  EnableWindow( GetDlgItem( hDlg, IDC_SLIDER_BT_MIC ), !bShade );
  EnableWindow( GetDlgItem( hDlg, IDC_SLIDER_BT_SPK ), !bShade );
}


static void updateVoiceInSlider(HWND hDlg, HWND hDlgSliderVoiceInGain, int *pCurPos, int voiceInMaxR)
{
  INT16 voiceInGain;
  BOOL  mute;

   gVoiceInSliderNotifyOmit = TRUE;

		//voiceGain, voiceGainMin, voiceGainMax
		if( getVoiceInGain(&voiceInGain, &sVoiceInGainMin, &sVoiceInGainMax, gBtConnect) )
		{
          if( (sVoiceInGainMax - sVoiceInGainMin) != 0 )
		  {
           SendMessage(hDlgSliderVoiceInGain, TBM_SETRANGEMIN, TRUE , sVoiceInGainMin);  
		   SendMessage(hDlgSliderVoiceInGain, TBM_SETRANGEMAX, TRUE , sVoiceInGainMax);  

           //*pCurPos = ( (voiceInGain - sVoiceInGainMin) * voiceInMaxR)/(sVoiceInGainMax - sVoiceInGainMin);
		   //*pCurPos = (voiceInGain - sVoiceInGainMin);
		   *pCurPos = voiceInGain;
           SendMessage(hDlgSliderVoiceInGain, TBM_SETPOS, TRUE , *pCurPos); 

		   wsprintf( myTest, _T("%d"), voiceInGain );
		   SetDlgItemText( hDlg, IDC_VOLUME_VOICE_IN, myTest );
		  }

			wsprintf( myTest, _T("%d"), sVoiceInGainMax );
			SetDlgItemText(hDlg, IDC_VOL_IN_MAX, myTest);

			wsprintf( myTest, _T("%d"), sVoiceInGainMin );
			SetDlgItemText(hDlg, IDC_VOL_IN_MIN, myTest);
		}

		if( getVoiceInMute(&mute, gBtConnect) )
		{
          if( mute )
           SendDlgItemMessage(hDlg, IDC_VOICE_IN_MUTE, BM_SETCHECK, (WPARAM)BST_CHECKED , 0);
		  else
           SendDlgItemMessage(hDlg, IDC_VOICE_IN_MUTE, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);  
		}

    gVoiceInSliderNotifyOmit = FALSE;
}


static void updateVoiceOutSlider(HWND hDlg, HWND hDlgSliderVoiceOutGain, int *pCurPos, int voiceOutMaxR)
{
	INT16 voiceOutGain;
	BOOL  mute;

	gVoiceOutSliderNotifyOmit = TRUE;

		//voiceGain, voiceGainMin, voiceGainMax
		if( getVoiceOutGain(&voiceOutGain, &sVoiceOutGainMin, &sVoiceOutGainMax, gBtConnect) )
		{
          if( (sVoiceOutGainMax - sVoiceOutGainMin) != 0 )
		  {
           SendMessage(hDlgSliderVoiceOutGain, TBM_SETRANGEMIN, TRUE , sVoiceOutGainMin);  
		   SendMessage(hDlgSliderVoiceOutGain, TBM_SETRANGEMAX, TRUE , sVoiceOutGainMax);  

           //*pCurPos = ( (voiceOutGain - sVoiceOutGainMin) * voiceOutMaxR)/(sVoiceOutGainMax - sVoiceOutGainMin);
		   //*pCurPos = (voiceOutGain - sVoiceOutGainMin);
		   *pCurPos = voiceOutGain;
           SendMessage(hDlgSliderVoiceOutGain, TBM_SETPOS, TRUE , *pCurPos); 

		   wsprintf( myTest, _T("%d"), voiceOutGain );
		   SetDlgItemText( hDlg, IDC_VOLUME_VOICE_OUT, myTest );
		  }

			wsprintf( myTest, _T("%d"), sVoiceOutGainMax );
			SetDlgItemText(hDlg, IDC_VOL_OUT_MAX, myTest);

			wsprintf( myTest, _T("%d"), sVoiceOutGainMin );
			SetDlgItemText(hDlg, IDC_VOL_OUT_MIN, myTest);
		}

		if( getVoiceOutMute(&mute, gBtConnect) )
		{
          if( mute )
           SendDlgItemMessage(hDlg, IDC_VOICE_OUT_MUTE, BM_SETCHECK, (WPARAM)BST_CHECKED , 0);
		  else
           SendDlgItemMessage(hDlg, IDC_VOICE_OUT_MUTE, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);  
		}

   gVoiceOutSliderNotifyOmit = FALSE;
}


static void updateBTMicSlider(HWND hDlg, HWND hDlgSliderBTMic, int *pCurPos)
{
  USHORT vol;

  if( btGetMicVol(&vol) )
  {
   if( vol <= 15 )
   {
	*pCurPos = vol;
    SendMessage(hDlgSliderBTMic, TBM_SETPOS, TRUE , *pCurPos); 

	wsprintf( myTest, _T("%d"), vol );
	SetDlgItemText( hDlg, IDC_BT_MIC, myTest );
   }
  }
}

static void updateBTSpkSlider(HWND hDlg, HWND hDlgSliderBTSpk, int *pCurPos)
{
  USHORT vol;

  if( btGetSpkVol(&vol) )
  {
   if( vol <= 15 )
   {
	*pCurPos = vol;
    SendMessage(hDlgSliderBTSpk, TBM_SETPOS, TRUE , *pCurPos); 

	wsprintf( myTest, _T("%d"), vol );
	SetDlgItemText( hDlg, IDC_BT_SPK, myTest );
   }
  }
}

static BOOL  vModemAudioConnect(BOOL bGui)
{
  HICON   hBTIcon = NULL;

              if( gBtConnect == FALSE )
			  {
                shadeModemBTConnectButton(g_hDlg, TRUE); 
				gBtConnect = TRUE;

                if( btAudioConnect(BT_AG_HS_ACL|BT_AG_HS_SCO) )
				{
                  //gBtConnect = TRUE;
				  hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_VOICE_CONNECT_ICON), IMAGE_ICON, 16, 16, 0 ); 
				  SendDlgItemMessage(g_hDlg, IDC_BT_VOICE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);
				  DestroyIcon(hBTIcon);

				   if( isModemOn() )
				   {
	                
					EnableWindow( GetDlgItem( g_hDlg, IDC_CALL_KEY ), FALSE );
					EnableWindow( GetDlgItem( g_hDlg, IDC_END_KEY ), FALSE );

					//hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_VOICE_OFF_ICON), IMAGE_ICON, 16, 16, 0 ); 
					//ret = SendDlgItemMessage(hDlg, IDC_BT_VOICE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);
					//DestroyIcon(hBTIcon);


					   turnVoiceOnOff(0, FALSE);
					   turnVoiceOnOff(1, TRUE);
 					   updateVoiceInSlider(g_hDlg, g_hDlgSliderVoiceInGain, &voiceInCurPos, voiceInMaxRange);
					   updateVoiceOutSlider(g_hDlg, g_hDlgSliderVoiceOutGain, &voiceOutCurPos, voiceOutMaxRange);

				   }
                  SendDlgItemMessage(g_hDlg, HS_AUDIO, BM_SETCHECK, (WPARAM)BST_CHECKED , 0);
				  shadeBTSliders(g_hDlg, FALSE);
				}
				else
				{
                  gBtConnect = FALSE;
                  SendDlgItemMessage(g_hDlg, HS_AUDIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);
				}
				shadeModemBTConnectButton(g_hDlg, FALSE);
			  }
 
          if( bGui == TRUE )
            gBtConnectPrev = gBtConnect;

  return(TRUE);
}


static BOOL  vModemAudioDisconnect(BOOL bGui)
{
 HICON   hBTIcon = NULL;

              if( gBtConnect == TRUE )
			  {
                shadeModemBTConnectButton(g_hDlg, TRUE);
                gBtConnect = FALSE;
                btAudioDisconnect(BT_AG_HS_ACL|BT_AG_HS_SCO);
				hBTIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BT_VOICE_ON_ICON), IMAGE_ICON, 16, 16, 0 ); 
				SendDlgItemMessage(g_hDlg, IDC_BT_VOICE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hBTIcon);
				DestroyIcon(hBTIcon);

				SendDlgItemMessage(g_hDlg, HS_AUDIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED , 0);
				shadeBTSliders(g_hDlg, TRUE);

                if( isModemOn() )
				{
				  turnVoiceOnOff(0, TRUE);
                  turnVoiceOnOff(1, FALSE);
                  
	 			  updateVoiceInSlider(g_hDlg, g_hDlgSliderVoiceInGain, &voiceInCurPos, voiceInMaxRange);
				  updateVoiceOutSlider(g_hDlg, g_hDlgSliderVoiceOutGain, &voiceOutCurPos, voiceOutMaxRange);
				  //updateMicAnalogGainInSlider(hDlg, g_hDlgSliderMicAnalogGain, &micAnalogGainCurPos, micAnalogGainMaxRange);
				  //updateMicDigitalGainInSlider(hDlg, g_hDlgSliderMicDigitalGain, &micDigitalGainCurPos, micDigitalGainMaxRange);

                  EnableWindow( GetDlgItem( g_hDlg, IDC_CALL_KEY ), TRUE );
	              EnableWindow( GetDlgItem( g_hDlg, IDC_END_KEY ), TRUE );
				}

				shadeModemBTConnectButton(g_hDlg, FALSE);
			  }

          if( bGui == TRUE )
            gBtConnectPrev = gBtConnect;

  return(TRUE);
}

static BOOL vModemDial(void)
{
			  LONG  len;
			  size_t origsize; 
			  size_t convertedChars = 0;
			  char   ATstring[NUM_OF_EDIT_CHARS-2];

              EnableWindow( GetDlgItem( g_hDlg, IDC_CALL_KEY ), FALSE );
	          EnableWindow( GetDlgItem( g_hDlg, IDC_END_KEY ), FALSE );

			  *(WORD *)gEditLine = NUM_OF_EDIT_CHARS - sizeof(TCHAR);
			  len = SendDlgItemMessage(g_hDlg, IDC_DIAL_EDIT_BOX, EM_GETLINE, 0 , (LPARAM)gEditLine);
			  //IDC_DIAL_EDIT_BOX

			  origsize = wcslen(gEditLine) + 1;
              wcstombs_s(&convertedChars, ATstring, origsize, gEditLine, _TRUNCATE);

			  //MIC_GSMVoiceCMD(g_hCom, GSM_VOICE_DIAL, ATstring);
			  modemVoiceDial(ATstring);

              EnableWindow( GetDlgItem( g_hDlg, IDC_END_KEY ), TRUE );
	          EnableWindow( GetDlgItem( g_hDlg, IDC_CALL_KEY ), TRUE );

  return(TRUE);
}

static BOOL  vModemAnswer(void)
{
	          EnableWindow( GetDlgItem( g_hDlg, IDC_ANSWER ), FALSE );
              EnableWindow( GetDlgItem( g_hDlg, IDC_CALL_KEY ), FALSE );
	          EnableWindow( GetDlgItem( g_hDlg, IDC_END_KEY ), FALSE );

			  //MIC_GSMVoiceCMD(g_hCom, GSM_VOICE_ANSWER, NULL);
			  modemVoiceAnswer();

	          EnableWindow( GetDlgItem( g_hDlg, IDC_END_KEY ), TRUE );
              EnableWindow( GetDlgItem( g_hDlg, IDC_CALL_KEY ), TRUE );
	          EnableWindow( GetDlgItem( g_hDlg, IDC_ANSWER ), TRUE );

  return(TRUE);
}

static BOOL  vModemhangUp(void)
{
              EnableWindow( GetDlgItem( g_hDlg, IDC_CALL_KEY ), FALSE );
	          EnableWindow( GetDlgItem( g_hDlg, IDC_END_KEY ), FALSE );

			  RETAILMSG(1, (L"DIGVOICE:%S, start Hang-Up\r\n",__FUNCTION__));

			  //MIC_GSMVoiceCMD(g_hCom, GSM_VOICE_HANG_UP, NULL);
			  modemVoiceHangUp();

			  RETAILMSG(1, (L"DIGVOICE:%S, stop Hang-Up\r\n",__FUNCTION__));

              EnableWindow( GetDlgItem( g_hDlg, IDC_END_KEY ), TRUE );
	          EnableWindow( GetDlgItem( g_hDlg, IDC_CALL_KEY ), TRUE );

  return(TRUE);
}

DWORD WINAPI VoiceModemThread(LPVOID lpParam)
{
  MSG               Msg;
  VMODEM_CONTEXT_s  *pContext;
  VMODEM_STATES     vModemState = VMODEM_IDLE;
  VMODEM_EVENTS     vModemEvent = (VMODEM_EVENTS)-1;
  AG_HS_STATES      AgHsState = AG_HS_OFF; 
  AG_HS_EVENTS      AgHsEvent = (AG_HS_EVENTS)-1;
  UINT              timerId = 0; 
  DWORD             AgServiceStatus;
  //BOOL              bBTCtrlActive   = FALSE;
  //BOOL              bBTAudioActive = FALSE;
  //BOOL              bPrevBTAudioActive = FALSE; 

  pContext = (VMODEM_CONTEXT_s *)lpParam;

  // g_hVModemThr
  RETAILMSG(1, (L"DIGVOICE:+%S, Thread Id = 0x%X, Handle(get)= 0x%X, handle(Create)= 0x%X\r\n",__FUNCTION__, GetCurrentThreadId(), GetCurrentThread(), g_hVModemThr ));

  while(GetMessage(&Msg, NULL, 0, 0))
  {
   RETAILMSG(1, (L"DIGVOICE:%S, msg = 0x%X , Event = %d\r\n",__FUNCTION__, Msg.message, Msg.wParam ));
 
   switch(Msg.message)
   {
    case  WM_APP:
	 {
       vModemEvent = (VMODEM_EVENTS)Msg.wParam;

	   switch(vModemEvent)
	   {
	     case VMODEM_OK:
		  {
            if( vModemState == VMODEM_DIALING )
			{
             vModemState = VMODEM_CONNECTED;
			 RETAILMSG(1, (L"DIGVOICE:%S, vModemState = VMODEM_CONNECTED\r\n",__FUNCTION__));
			}

		  } break;

		 case VMODEM_DIAL:
		  {
            if( vModemState == VMODEM_IDLE )
			{
             vModemState = VMODEM_DIALING;
			 RETAILMSG(1, (L"DIGVOICE:%S, vModemState = VMODEM_DIALING\r\n",__FUNCTION__));
			}

		  } break;

		 case VMODEM_HANGUP:
		  {
            if( vModemState == VMODEM_CONNECTED )
			{
             vModemState = VMODEM_IDLE;
			 RETAILMSG(1, (L"DIGVOICE:%S, vModemState = VMODEM_IDLE\r\n",__FUNCTION__));
			}

		  } break;

		 case VMODEM_ANSWER:
		  {
            if( vModemState == VMODEM_RINGING )
			{
             vModemState = VMODEM_CONNECTED;

			   if( timerId != 0 )
			   {
				 timerId = 0; 
				 KillTimer(pContext->hDlg, VMODEM_TIMEOUT_EV_0);
			   }

			 RETAILMSG(1, (L"DIGVOICE:%S, vModemState = VMODEM_CONNECTED\r\n",__FUNCTION__));
			}

		  } break;

	     case VMODEM_RING:
	      {
           // Ring timeout 
           RETAILMSG(1, (L"DIGVOICE:%S, RING\r\n",__FUNCTION__));  

		   //MIC_AGHSControl(AG_CONNECT, &AgServiceStatus);
#if 0
		   if( AgHsState >= AG_HS_ACL )
             MIC_AGHSControl(AG_RING, &AgServiceStatus);
#endif
		   if( timerId != 0 )
		   {
             timerId = 0; 
             KillTimer(pContext->hDlg, VMODEM_TIMEOUT_EV_0);
		   }

		   timerId = SetTimer(pContext->hDlg, VMODEM_TIMEOUT_EV_0, 6000, NULL);

           if( vModemState == VMODEM_IDLE ) 
		   {
            vModemState = VMODEM_RINGING;
			//vModemAudioConnect(FALSE);
			RETAILMSG(1, (L"DIGVOICE:%S, vModemState = VMODEM_RINGING\r\n",__FUNCTION__));
		   }
	      } break;

		 case VMODEM_NO_CARRIER:
 	      {
            vModemState = VMODEM_IDLE;
			RETAILMSG(1, (L"DIGVOICE:%S, NO CARRIER, vModemState = VMODEM_IDLE\r\n",__FUNCTION__));  
	      } break;

	   }

	 } break;

    case  WM_APP+1:  // Audio Gateway messages
	 {
       AgHsEvent = (AG_HS_EVENTS)Msg.wParam; 

	   switch(AgHsEvent)
	    {
	       case AG_HS_BUTTON_PRESS:
			 {
		      RETAILMSG(1, (L"DIGVOICE:%S, AG_BUTTON_PRESS, state = %d\r\n",__FUNCTION__, vModemState));  

			   if( vModemState == VMODEM_RINGING ) 
			   {
				 // Send Answer Command ! 
				 vModemAnswer();
				 RETAILMSG(1, (L"DIGVOICE:%S, AG_HS_BUTTON_PRESS, call vModemAnswer\r\n",__FUNCTION__));  
				 //vModemState = VMODEM_CONNECTED;
			   }

			   if( vModemState == VMODEM_CONNECTED )
			   {
                 vModemhangUp();  
				 vModemAudioDisconnect(TRUE);
				 RETAILMSG(1, (L"DIGVOICE:%S, AG_HS_BUTTON_PRESS, call vModemAudioDisconnect\r\n",__FUNCTION__));  
			   }

			 } break;

	       case AG_HS_MIC_GAIN: 
			 {
               RETAILMSG(1, (L"DIGVOICE:%S, AG_HS Mic gain = %d\r\n",__FUNCTION__, Msg.lParam));  
               updateBTMicSlider(pContext->hDlg, g_hDlgSliderBTMic, (int *)&Msg.lParam);
			 }break;

	       case AG_HS_SPEAKER_GAIN:
			 {
               RETAILMSG(1, (L"DIGVOICE:%S, AG_HS Mic gain = %d\r\n",__FUNCTION__, Msg.lParam));  
               updateBTSpkSlider(pContext->hDlg, g_hDlgSliderBTSpk, (int *)&Msg.lParam);
			 }break;

	       case AG_HS_CTRL:
			 {

              if( Msg.lParam == 0 )
			   {
                 if( AgHsState != AG_HS_OFF )
				 {
                   AgHsState = AG_HS_OFF; 
				   RETAILMSG(1, (L"DIGVOICE:%S, ACL Off, call vModemAudioDisconnect\r\n",__FUNCTION__));  
				   vModemAudioDisconnect(FALSE);
				 }
			   }
			  else
			   {
                 if( AgHsState == AG_HS_OFF )
				 {
                   AgHsState = AG_HS_ACL; 
				   RETAILMSG(1, (L"DIGVOICE:%S, ACL On\r\n",__FUNCTION__));

				   if( Msg.lParam == 1 ) // HS initiated connection
				   {
                    if( vModemState == VMODEM_CONNECTED )
					{
                     RETAILMSG(1, (L"DIGVOICE:%S, HS initiated connection, Call vModemAudioConnect\r\n",__FUNCTION__));  
                     if( vModemAudioConnect(FALSE) )
                       AgHsState = AG_HS_SCO;
					}
				   }
				 }
			   }

			 } break;

	       case AG_HS_AUDIO:
			 {

              if( Msg.lParam == 0 )
			   {
                 if( AgHsState == AG_HS_SCO )
				 {
                   AgHsState = AG_HS_ACL; 
				   //vModemAudioDisconnect(FALSE);
				   //shadeModemBTConnectButton(g_hDlg, TRUE); 
				   //RETAILMSG(1, (L"DIGVOICE:%S, SCO Off, call vModemAudioDisconnect\r\n",__FUNCTION__));
				 }
			   }
			  else
			   {
                 if( AgHsState == AG_HS_ACL )
				 {
                   AgHsState = AG_HS_SCO; 
				   RETAILMSG(1, (L"DIGVOICE:%S, SCO On, call vModemAudioConnect\r\n",__FUNCTION__));
				   vModemAudioConnect(FALSE);
				   //shadeModemBTConnectButton(g_hDlg, FALSE); 
				 }
			   }

			 } break;

	       default: break;
	    }

	 } break;

    case  WM_APP+2:
	 {
        vModemEvent = (VMODEM_EVENTS)Msg.wParam;

        switch(vModemEvent)
		{
		  case VMODEM_CALL_KEY:
			 {
			   if( vModemState == VMODEM_RINGING ) 
			   {
				 // Send Answer Command ! 
				 vModemAnswer();
				 RETAILMSG(1, (L"DIGVOICE:%S, VMODEM_CALL_KEY, call vModemAnswer\r\n",__FUNCTION__));  
				 //vModemState = VMODEM_CONNECTED;
			   }

			   if( vModemState == VMODEM_IDLE )
			   {
				 // Send dial Command ! 
				 vModemDial();
				 RETAILMSG(1, (L"DIGVOICE:%S, VMODEM_CALL_KEY, call vModemDial\r\n",__FUNCTION__));  
				 //vModemState = VMODEM_CONNECTED;
			   }

			 } break;

		  case VMODEM_END_KEY:
			 {
			   if( ( vModemState == VMODEM_RINGING )|| (vModemState == VMODEM_CONNECTED) )
			   {
				 // Send Reject (Hang-Up) Command ! 
				 vModemhangUp();
				 RETAILMSG(1, (L"DIGVOICE:%S, VMODEM_END_KEY, call vModemhangUp\r\n",__FUNCTION__));  
				 //vModemState = VMODEM_IDLE;
			   }

			 } break;

		  default: break;
		}

 	 } break;

	 case  WM_APP+3:
	 {
       vModemEvent = (VMODEM_EVENTS)Msg.wParam;

       if( vModemEvent == VMODEM_TIMEOUT_EV_0 )
	   {
            if( vModemState == VMODEM_RINGING )
			{
             vModemState = VMODEM_IDLE;

			   if( timerId != 0 )
			   {
				 timerId = 0; 
				 KillTimer(pContext->hDlg, VMODEM_TIMEOUT_EV_0);
			   }

			 RETAILMSG(1, (L"DIGVOICE:%S, VMODEM_RINGING, Ringing Timeout Event\r\n",__FUNCTION__));
			}
	   }

 	 } break;    

	default: break;
   }

  } // while
 

 RETAILMSG(1, (L"DIGVOICE:-%S\r\n",__FUNCTION__));
 return(0);
}