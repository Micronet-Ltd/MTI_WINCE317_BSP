// DigVoice.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DigVoice.h"
#include <windows.h>
#include <service.h>
#include <commctrl.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <MicUserSdk.h>

#include "WavePlayer.h"
#include "WaveRecorder.h"

#include "Voice.h"
#include "comm.h"

#include "bt.h"
#include "btagpub.h"

#include "tapi.h"
#include "unimodem.h"

#include "ag_hs_api.h"

#define MAX_LOADSTRING 100

#define NUM_OF_EDIT_CHARS  32

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
	INT_MIC = 0,
    EXT_MIC,

	NUM_OF_MIC,

} MIC_TYPE_e;


typedef struct VOICE_LINE_tag
{
  HLINEAPP                *phLine;
  LINEINITIALIZEEXPARAMS  *pLineInitEx;
  
}VOICE_LINE;

PLAY_STATE gPlayState = FILE_SILENT;

HANDLE          g_hCom = INVALID_HANDLE_VALUE;
TCHAR           g_ComPort[8] = L"COM0:";

HWND            g_hDlg;
CWavePlayer		g_pl;
CWaveRecorder	g_rec;
OPENFILENAME    ofnPlay = {0};
OPENFILENAME    ofnRec = {0};
BOOL            gPlayFileOpenStatus = FALSE;
BOOL            gRecFileOpenStatus = FALSE;
TCHAR           myTest[32];
BOOL            gModemOn = FALSE;
int             modemAudioProfile = -1;

BOOL            gBtOn = FALSE;
BOOL            gExtMic = FALSE;

TCHAR           gEditLine[NUM_OF_EDIT_CHARS];

  static INT16  /*voiceInGain,*/ sVoiceInGainMin, sVoiceInGainMax;
  static INT16  /*voiceOutGain,*/ sVoiceOutGainMin, sVoiceOutGainMax;
  static UINT16 /* micAnalogGain, */ sMicAnalogGainMin, sMicAnalogGainMax;
  static int /*micDigitalGain, */ sMicDigitalGainMin, sMicDigitalGainMax;

  static WCHAR szBufString[BTENUM_MAX_NAME];

BOOL modemOn(HWND hDlg);
BOOL modemOnTest(HWND hDlg);
BOOL modemOff(HWND hDlg);

HANDLE g_hThr = NULL;
DWORD WINAPI RunnerThread(LPVOID lpParam);

LRESULT CALLBACK mainDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
BOOL  EndPlayBack( DWORD playerRet );

VOID shadeModemCheckBox(HWND hDlg, BOOL bShade);
VOID shadeModemDialButtons(HWND hDlg, BOOL bShade);
VOID shadeModemAPButtons(HWND hDlg, BOOL bShade);
VOID shadeModemBTButton(HWND hDlg, BOOL bShade);
VOID shadeRecButtons(HWND hDlg, BOOL bShade);

static void updateVoiceInSlider(HWND hDlg, HWND hDlgSliderVoiceInGain, int *pCurPos, int voiceInMaxR);
static void updateVoiceOutSlider(HWND hDlg, HWND hDlgSliderVoiceOutGain, int *pCurPos, int voiceOutMaxR);
static void updateMicAnalogGainInSlider(HWND hDlg, HWND hDlgSliderMicAnalogGain, int *pMicAnalogGainCurPos, int micAnalogGainMaxRange);
static void updateMicDigitalGainInSlider(HWND hDlg, HWND hDlgSliderMicDigitalGain, int *pMicDigitalGainCurPos, int micDigitalGainMaxRange);

#if 0
//BOOL putMicDigitalGain(UINT16 micDigitalGain);
BOOL   (*pFnPutMicDigitalGain[NUM_OF_MIC])(UINT16 micDigitalGain) = 
{
  putMicDigitalGain,
  putExtMicDigitalGain,
};

// BOOL getMicDigitalGain(UINT16 *pMicDigitalGain, UINT16 *pMicDigitalGainMin, UINT16 *pMicDigitalGainMax);
BOOL   (*pFngetMicDigitalGain[NUM_OF_MIC])(int *pMicDigitalGain, int *pMicDigitalGainMin, int *pMicDigitalGainMax) = 
{
	getMicDigitalGain,
	getExtMicDigitalGain,
};
#endif

//typedef BOOL                      (WINAPI *BTHAGNETREGEVENT)( DWORD, DWORD );

static HLINEAPP hLine;
static LINEINITIALIZEEXPARAMS LineInitEx;
DWORD dwNumdevs=0;
DWORD dwApiVers=TAPI_CURRENT_VERSION;
DWORD DevID;
LINEDEVCAPS LineDevCaps;
DWORD dwRetVal = ERROR_NOT_FOUND;
VOICE_LINE  gVoiceLine = {NULL, NULL};


static BOOL AGControl(AGHSCMD_e cmd)
{
 //int ret;
 DWORD ioctl;

         switch(cmd)
		 {
		  case AG_CONNECT:        ioctl = IOCTL_AG_OPEN_CONTROL; break;
		  case AG_AUDIO_ON:       ioctl = IOCTL_AG_OPEN_AUDIO; break;
		  case AG_AUDIO_OFF:      ioctl = IOCTL_AG_CLOSE_AUDIO; break;
		  case AG_DISCONNECT:     ioctl = IOCTL_AG_CLOSE_CONTROL; break;
		  case AG_SERVICE_START:  ioctl = IOCTL_SERVICE_START; break;
		  case AG_SERVICE_STOP:   ioctl = IOCTL_SERVICE_STOP; break;
		  default: return 0;
		 }

		 HANDLE h = CreateFile(L"BAG0:",0,0,NULL,OPEN_EXISTING,0,NULL);
		 if(INVALID_HANDLE_VALUE == h) 
		  {
			//wprintf(L"Error calling CreateFile on Audio Gateway.\r\n");
			return 0;
		  }
		 else
		  {
			BOOL fStatus = DeviceIoControl(h,ioctl,NULL,0,NULL,0,NULL,NULL);
			CloseHandle(h);
			 if (FALSE == fStatus) 
			  {
				//wprintf(L"Operation failed: %d.\n", GetLastError());
				  return 0;
			  }
			 else 
			  {
					//wprintf(L"Operation successful.\n"); 
				  // Get name
				  //ret = BthRemoteNameQuery(&pb->b, BTENUM_MAX_NAME, &cRequired, szBufString);
				  return(TRUE);
			  }
		  }

}

static int EnumCallback (void *pContext, BTDEV *pb) 
{
	int ret;
	unsigned int cod;
	unsigned short handleALC;
    unsigned int cRequired;
	unsigned char mode;
	unsigned char pin[4] = {'0','0','0','0'};



	ret = BthGetRemoteCOD(&pb->b, &cod);
	if( ret != ERROR_SUCCESS )
	{
     //ret = BthRemoteNameQuery(&pb->b, BTENUM_MAX_NAME, &cRequired, szBufString);
	 //ret = BthAuthenticate(&pb->b);
      ret = BthGetCurrentMode(&pb->b, &mode); 
	  ret = BthCreateACLConnection(&pb->b, &handleALC);
	  
	  if( ret == ERROR_SUCCESS )
	   {
         //ret = BthPairRequest(&pb->b, 4, pin); 
		 ret = AGControl(AG_SERVICE_START);
		 ret = AGControl(AG_CONNECT);
	   }
#if 0   
	 ret = BthPairRequest(&pb->b, 4, pin); 
	 if( ret == ERROR_SUCCESS )
	  {
       ret = BthGetCurrentMode(&pb->b, &mode);
	   ret = BthRemoteNameQuery(&pb->b, BTENUM_MAX_NAME, &cRequired, szBufString);
	  }
#endif

     return 1;

	}
   else
   {
    ret = BthRemoteNameQuery(&pb->b, BTENUM_MAX_NAME, &cRequired, szBufString);
	if( ret == ERROR_SUCCESS )
	{
     ret = AGControl(AG_CONNECT);
	 if( ret == TRUE )
	  return 0; // Device found
	 else
      return 1;  
	}
	else
     return 1;  
   }
}


BOOL  EndPlayBack( DWORD playerRet )
{
  // WM_USER 
  SendMessage(g_hDlg, WM_USER, 0 ,0); 
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
  static HWND hDlgSlider; 
  static HWND hDlgSliderVoiceInGain; 
  static HWND hDlgSliderVoiceOutGain; 
  static HWND hDlgSliderMicAnalogGain; 
  static HWND hDlgSliderMicDigitalGain; 

  DWORD  numOfSliderTicks;
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
  static int    voiceInMaxRange, voiceInMinRange;
  static int    voiceOutMaxRange, voiceOutMinRange;
  static int    voiceInCurPos, voiceOutCurPos;

  // Microphone Analog/Digital gain sliders
  static int    micAnalogGainMaxRange, micAnalogGainMinRange;
  static int    micDigitalGainMaxRange, micDigitalGainMinRange;
  static int    micAnalogGainCurPos, micDigitalGainCurPos;


  MMRESULT mmres;

  count++;

  switch(msg)
  {
    case WM_INITDIALOG:
	{
     TCHAR         myTest[32]; 
	 WAVEFORMATEX  wfx;
	 BOOL          bRet;
	 //BOOL          micMute;
	 LONG          iRet;
	 DWORD         AgServiceStatus;
	 BT_ADDR bt;
	 HANDLE         hAgHS; 


#if 0
	 bRet =	AGControl(AG_CONNECT);

	 bRet =	AGControl(AG_DISCONNECT);
	 bRet =	AGControl(AG_SERVICE_STOP);
	 bRet =	AGControl(AG_SERVICE_START);
	 bRet =	AGControl(AG_CONNECT);

	 gBtOn = TRUE;
	 modemAudioProfile = 3;
	 modemOn(hDlg);
#endif
	 //bRet =	AGControl(AG_SERVICE_STOP);
	 //bRet =	AGControl(AG_SERVICE_START);

     //bRet =	AGControl(AG_CONNECT);

	 //g_hThr = CreateThread(0, 0, RunnerThread, NULL, 0, 0 );

	 // 1st, Stop AGService

     //BtAgSvcCallTest();

//   bRet =	AGControl(AG_SERVICE_STOP);


#if 0
  iRet = MIC_AGHSControl(AG_SERVICE_STATUS, &AgServiceStatus);
  if( iRet != AG_HS_OK )
  {
	RETAILMSG(1, (L"DIGVOICE:%S, Ag Service Control Failure, ret = %d\r\n",__FUNCTION__, iRet));
  }
  else
  {
	RETAILMSG(1, (L"DIGVOICE:%S, Ag Service Status = 0x%X\r\n",__FUNCTION__, AgServiceStatus));
  }

  switch(AgServiceStatus)
  {
    case  SERVICE_STATE_OFF:
	case  SERVICE_STATE_UNINITIALIZED:
		{
          iRet = MIC_AGHSControl(AG_SERVICE_START, &AgServiceStatus);
		} break;

	case  SERVICE_STATE_ON:
		{
         iRet == AG_HS_OK;
		} break;

	case  SERVICE_STATE_STARTING_UP:
	case  SERVICE_STATE_SHUTTING_DOWN:
	case  SERVICE_STATE_UNLOADING:
	case  SERVICE_STATE_UNKNOWN:
	default:
		{
         iRet = -1;   
		} break; 

  }

   
  if( iRet == AG_HS_OK )
  {
   bt = 0x1234567812345678;
   iRet = MIC_AGHSOpen(&hAgHS, bt);
  }

  if( iRet == AG_HS_OK )
   iRet = MIC_AGHSClose(hAgHS);
#endif

    //CardVer cv = {0};

    //bRet = MIC_IsHWExist(QUERY_EXTAUDIO_CFG, &cv, sizeof(cv));
    //iRet = GetLastError();


#if 0
	 HANDLE h = CreateFile(L"BAG0:",0,0,NULL,OPEN_EXISTING,0,NULL);
	 if(INVALID_HANDLE_VALUE != h) 
	 {
	  BOOL fStatus = DeviceIoControl(h, /*IOCTL_AG_CLOSE_AUDIO*/ IOCTL_SERVICE_START, NULL,0,NULL,0,NULL,NULL);
 
	  if( fStatus )
	  {
       int vol = 10;
       //IOCTL_AG_SET_SPEAKER_VOL 
		fStatus = DeviceIoControl(h, IOCTL_AG_SET_SPEAKER_VOL, (LPVOID)&vol, sizeof(vol), NULL,0,NULL,NULL);
	  }

	  CloseHandle(h);
	 }
#endif


#if 0
   HKEY hk;
   WCHAR wszName[MAX_PATH];
   WCHAR wszPortName[32];
   BOOL fUseDefaultLine = TRUE;
	DWORD    cSubKeys=0;               /* number of subkeys       */ 
	DWORD    cbMaxSubKey=0;            /* longest subkey size     */ 
	DWORD    cchMaxClass=0;            /* longest class string    */ 
	DWORD    cValues=0;                /* number of values for key    */ 
	DWORD    cchMaxValue=0;            /* longest value name          */ 
	DWORD    cbMaxValueData=0;         /* longest value data          */ 
	LONG     hRes;
	DWORD    dwLen;
	CE_REGISTRY_INFO   regInfo;
	TCHAR     szRegKey[255];

       if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, RK_AUDIO_GATEWAY, 0, 0, &hk)) 
	   {
        DWORD cdwBytes = MAX_PATH;
        if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("TapiLineName"), 0, NULL, (PBYTE)wszName, &cdwBytes)) 
		{
            fUseDefaultLine = FALSE;
        }

		  RegCloseKey(hk);

          StringCchPrintf(szRegKey, sizeof(szRegKey), TEXT("ExtModems\\%s"), wszName);
		  hRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegKey, 0, 0, &hk);
		  hRes = RegQueryValueEx(hk, _T("Port"), 0, NULL, (PBYTE)wszPortName, &cdwBytes);


          hRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, NULL, 0, 0, &hk);
		  hRes = RegQueryValueEx(hk, _T("Port"), 0, NULL, (PBYTE)wszName, &cdwBytes);

         RegCloseKey(hk);
       }

#endif

#if 0
DWORD  foundLine;
DWORD dwVer=0;
DWORD retPower;
HLINE hmLine;

retPower = MIC_GSMPower(1);

ZeroMemory(&LineInitEx,sizeof(LINEINITIALIZEEXPARAMS));
LineInitEx.dwTotalSize=sizeof(LINEINITIALIZEEXPARAMS);
LineInitEx.dwNeededSize=0;
LineInitEx.dwCompletionKey=0;
LineInitEx.dwOptions= LINEINITIALIZEEXOPTION_USEEVENT; //LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;
LineInitEx.dwUsedSize=0;
//LineInitEx.Handles.hEvent = ; TAPI creates an Event !

LONG r=lineInitializeEx(&hLine, NULL,NULL,NULL,&dwNumdevs,&dwApiVers,&LineInitEx);

for( DevID = 0; DevID < dwNumdevs; DevID++ )
{
LINEEXTENSIONID Ext;
ZeroMemory(&Ext,sizeof(LINEEXTENSIONID));

r=lineNegotiateAPIVersion(hLine,DevID,0x00010000,0x00030000,&dwVer,&Ext);


//DWORD dwAdrId=0;
//r=lineGetAddressID(hmLine,&dwAdrId,LINEADDRESSMODE_DIALABLEADDR,TEXT("02101794097+"),13);
//LINEADDRESSCAPS adrcaps;
//ZeroMemory(&adrcaps,sizeof(LINEADDRESSCAPS));
//adrcaps.dwTotalSize=sizeof(LINEADDRESSCAPS);
//r=lineGetAddressCaps(hLine,DevID,dwAdrId,dwVer,NULL,&adrcaps);
//if(adrcaps.dwAddressFeatures & LINEADDRFEATURE_SETUPCONF)
//{
//}
//if(adrcaps.dwCallFeatures & LINECALLFEATURE_DIAL)
//{
//}
//if(adrcaps.dwCallFeatures & LINECALLFEATURE_ADDTOCONF)
//{
//}
//if(adrcaps.dwCallFeatures & LINECALLFEATURE_PREPAREADDCONF)
//{
//}
if( r == ERROR_SUCCESS )
{
	LineDevCaps.dwTotalSize = sizeof(LineDevCaps);
	r = lineGetDevCaps(hLine, DevID, dwVer, 0, &LineDevCaps);
	if( r == ERROR_SUCCESS )
	 {
	  LINEDEVCAPS* pLineDevCaps = (LINEDEVCAPS*) new BYTE[LineDevCaps.dwNeededSize];
		  if (pLineDevCaps) 
		  {
			pLineDevCaps->dwTotalSize = LineDevCaps.dwNeededSize;
			if (ERROR_SUCCESS == lineGetDevCaps(hLine, DevID, dwVer, 0, pLineDevCaps)) {
				if (0 == wcscmp((WCHAR*)((BYTE*)pLineDevCaps + pLineDevCaps->dwLineNameOffset),  L"GSM Voice Modem")) 
				{
					//  HLINEAPP                *phLine;
					//   LINEINITIALIZEEXPARAMS  *pLineInitEx

                    // lineDevSpecific
					// allocate and set up pUniMdmChgDevCfg fields 
					UNIMDM_CHG_DEVCFG UniMdmChgDevCfg;
					LPVARSTRING       pDevConfig; 
					LPCWSTR           szDeviceClass = L"tapi/line";
                    DWORD dwAdrId=0;
					DWORD dwLen; 
					LONG  devSpecRet; 
					DWORD baudrate, bytesize, parity, stopbits, waitbong, mdmOpt, timeout, termopt;
					LPCWSTR dialMod;

                    r=lineOpen(hLine,DevID,&hmLine,dwVer,NULL,0,
						         LINECALLPRIVILEGE_OWNER | LINECALLPRIVILEGE_MONITOR, 
                                 /*LINEMEDIAMODE_INTERACTIVEVOICE*/ LINEMEDIAMODE_DATAMODEM, NULL);

                    //r=lineGetAddressID(hmLine,&dwAdrId,LINEADDRESSMODE_DIALABLEADDR,TEXT("02101794097+"),13);

					UniMdmChgDevCfg.dwCommand = UNIMDM_CMD_GET_DEVCFG;
					
					//UniMdmChgDevCfg.lpDevConfig = pDevConfig;
					UniMdmChgDevCfg.lpszDeviceClass = szDeviceClass;

					pDevConfig = (LPVARSTRING) new BYTE[sizeof(VARSTRING)];
					pDevConfig->dwTotalSize =  sizeof(VARSTRING);
					r = lineGetDevConfig(DevID, pDevConfig, szDeviceClass);
					dwLen = pDevConfig->dwNeededSize;
					 if( pDevConfig->dwTotalSize < dwLen )
					 {
                       delete[] pDevConfig;
                       pDevConfig = (LPVARSTRING) new BYTE[dwLen];
					   r = lineGetDevConfig(DevID, pDevConfig, szDeviceClass);
					 }

                    UniMdmChgDevCfg.lpDevConfig = pDevConfig;
					UniMdmChgDevCfg.lpDevConfig->dwTotalSize = dwLen;


					UniMdmChgDevCfg.dwOption = UNIMDM_OPT_BAUDRATE;
					devSpecRet = lineDevSpecific( hmLine, 0, NULL, &UniMdmChgDevCfg, sizeof(UNIMDM_CHG_DEVCFG));
					if( devSpecRet > 0 )
                     baudrate = UniMdmChgDevCfg.dwValue;

					UniMdmChgDevCfg.dwOption = UNIMDM_OPT_BYTESIZE;
					devSpecRet = lineDevSpecific( hmLine, 0, NULL, &UniMdmChgDevCfg, sizeof(UNIMDM_CHG_DEVCFG));
					if( devSpecRet > 0 )
                     bytesize = UniMdmChgDevCfg.dwValue;

					UniMdmChgDevCfg.dwOption = UNIMDM_OPT_PARITY;
					devSpecRet = lineDevSpecific( hmLine, 0, NULL, &UniMdmChgDevCfg, sizeof(UNIMDM_CHG_DEVCFG));
					if( devSpecRet > 0 )
                     parity = UniMdmChgDevCfg.dwValue;

					UniMdmChgDevCfg.dwOption = UNIMDM_OPT_STOPBITS;
					devSpecRet = lineDevSpecific( hmLine, 0, NULL, &UniMdmChgDevCfg, sizeof(UNIMDM_CHG_DEVCFG));
					if( devSpecRet > 0 )
                     stopbits = UniMdmChgDevCfg.dwValue;

					UniMdmChgDevCfg.dwOption = UNIMDM_OPT_WAITBONG;
					devSpecRet = lineDevSpecific( hmLine, 0, NULL, &UniMdmChgDevCfg, sizeof(UNIMDM_CHG_DEVCFG));
					if( devSpecRet > 0 )
                     waitbong = UniMdmChgDevCfg.dwValue;

					UniMdmChgDevCfg.dwOption = UNIMDM_OPT_MDMOPTIONS;
					devSpecRet = lineDevSpecific( hmLine, 0, NULL, &UniMdmChgDevCfg, sizeof(UNIMDM_CHG_DEVCFG));
					if( devSpecRet > 0 )
                     mdmOpt = UniMdmChgDevCfg.dwValue;

					UniMdmChgDevCfg.dwOption = UNIMDM_OPT_TIMEOUT;
					devSpecRet = lineDevSpecific( hmLine, 0, NULL, &UniMdmChgDevCfg, sizeof(UNIMDM_CHG_DEVCFG));
					if( devSpecRet > 0 )
                     timeout = UniMdmChgDevCfg.dwValue;

#if 0
					UniMdmChgDevCfg.dwOption = UNIMDM_OPT_TERMOPTIONS;
					devSpecRet = lineDevSpecific( hmLine, 0, NULL, &UniMdmChgDevCfg, sizeof(UNIMDM_CHG_DEVCFG));
					if( devSpecRet > 0 )
                     termopt = UniMdmChgDevCfg.dwValue;

					UniMdmChgDevCfg.dwOption = UNIMDM_OPT_DIALMOD;
					devSpecRet = lineDevSpecific( hmLine, 0, NULL, &UniMdmChgDevCfg, sizeof(UNIMDM_CHG_DEVCFG));
					if( devSpecRet > 0 )
                     dialMod = (LPCWSTR)UniMdmChgDevCfg.dwValue;
#endif


						DWORD dwSize = sizeof(VARSTRING) +1024 ;
						DWORD dwReturn = 1;
						LPVARSTRING lpVarString = NULL;
						LPCWSTR     portName;
						while(dwReturn)
						{
						 lpVarString = (LPVARSTRING)LocalAlloc(LPTR,dwSize);
						  if(!lpVarString) break;

						 lpVarString->dwTotalSize = dwSize;
						 dwReturn = lineGetID(hmLine,0,0,LINECALLSELECT_LINE, lpVarString, L"comm/datamodem/portname");

						 if(dwReturn == LINEERR_STRUCTURETOOSMALL)
						  lpVarString->dwNeededSize = dwSize*2;

						 if(lpVarString->dwNeededSize <= lpVarString->dwTotalSize ) 
						  break;

						 dwSize = lpVarString->dwNeededSize;
						 LocalFree(lpVarString);
						 dwReturn =1;
						}

						portName = ((LPCWSTR)(LPBYTE)lpVarString + lpVarString->dwStringOffset);  

					//r=lineClose(hmLine);
					delete[] pDevConfig;

					//gVoiceLine.phLine = &hLine;
					//gVoiceLine.pLineInitEx = &LineInitEx;

					//g_hThr = CreateThread(0, 0, RunnerThread, &gVoiceLine, 0, 0 );
					dwRetVal = ERROR_SUCCESS;
					//*puiCellLine = DevID;
					//g_Data.dwAPIVersion = dwVersion;
					delete[] pLineDevCaps;

					foundLine = DevID;
					break;
				}
			}

			delete[] pLineDevCaps;
		}

	 }
}

//r=lineClose(hmLine);
}

r=lineShutdown(hLine);
r=lineClose(hmLine);

#if 0
if(dwRetVal != ERROR_SUCCESS )
  r=lineShutdown(hLine);
else
{
 //r=lineOpen(hLine,foundLine,&hmLine,dwVer,NULL,foundLine,LINECALLPRIVILEGE_OWNER | LINECALLPRIVILEGE_MONITOR, 
//           LINEMEDIAMODE_INTERACTIVEVOICE /*LINEMEDIAMODE_DATAMODEM*/, NULL);

 r=lineClose(hmLine);
}
#endif

#endif

#if 0
#define  BT_PCM

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
	// dump(0);

	 //getVoiceOutGain(&voiceOutGain, &sVoiceOutGainMin, &sVoiceOutGainMax);
	 //setMicMute(1);
	 //getMicMute(&micMute);

	// HKEY hk;
	 //LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\device", 0, KEY_READ, &hk);

	 //BthEnumDevices (NULL, EnumCallback);

#if 0
	 BT_ADDR bt;

	 BASEBAND_CONNECTION  bConnection[10];
     int                  cConnectionsReturned, ret;
	 HANDLE               hAgHS; 

	 //ret = findLastPaired(&bt);

	 //ret = BthGetBasebandConnections(10, bConnection, &cConnectionsReturned);
	 //ret = AGConnect(); 
    //count = BthDelAgDevices();
    //count = BthSetAgDevices(&bt, EnumCallback);
	//bRet =	AGControl(AG_SERVICE_START);
	ret = MIC_AGHSOpen(&hAgHS, bt);

#endif

	 g_hDlg = hDlg;

	 EnableWindow( GetDlgItem( hDlg, IDC_DIAL ), FALSE );
	 EnableWindow( GetDlgItem( hDlg, IDC_HANG_UP ), FALSE );
	 EnableWindow( GetDlgItem( hDlg, IDC_ANSWER ), FALSE );

	 //SendDlgItemMessage(hDlg, IDC_DIAL_EDIT_BOX, EN_UPDATE, (WPARAM)0, (LPARAM)0); 

      //  g_hCom = __OpenComPort(g_ComPort);

	 //modemOff(g_hDlg);
        shadeModemCheckBox(hDlg, TRUE);
		shadeModemDialButtons(hDlg, TRUE);
		shadeModemBTButton(hDlg, TRUE);

#if 0
		if( modemOff(hDlg) )
		{
          shadeModemAPButtons(hDlg, FALSE);

		  if( turnVoiceOnOff(0, gBtOn) )
			wsprintf( myTest, _T("Voice Off") );
		  else
            wsprintf( myTest, _T("Voice Off failure") );
		}
#endif
		shadeModemCheckBox(hDlg, FALSE);

		//modemAudioProfile = 3;
		//modemOnTest(hDlg);

     g_pl.Init(EndPlayBack); 

     hDlgSlider = GetDlgItem(hDlg, IDC_SLIDER_PLAY_VOL);
     hDlgSliderVoiceInGain  = GetDlgItem(hDlg, IDC_SLIDER_VOICE_VOL_IN);  
     hDlgSliderVoiceOutGain = GetDlgItem(hDlg, IDC_SLIDER_VOICE_VOL_OUT); 
	 
	 hDlgSliderMicAnalogGain  = GetDlgItem(hDlg, IDC_SLIDER_MIC_ANALOG_GAIN);  
     hDlgSliderMicDigitalGain = GetDlgItem(hDlg, IDC_SLIDER_MIC_DIGITAL_GAIN); 

	 //numOfSliderTicks = SendDlgItemMessage(hDlg, IDC_SLIDER_PLAY_VOL, TBM_GETNUMTICS, 0 ,0);
	 numOfSliderTicks = SendMessage(hDlgSlider, TBM_GETNUMTICS, 0 ,0);
	 maxRange = SendMessage(hDlgSlider, TBM_GETRANGEMAX, 0 ,0); 
	 minRange = SendMessage(hDlgSlider, TBM_GETRANGEMIN, 0 ,0); 
	 curPos = SendDlgItemMessage(hDlg, IDC_SLIDER_PLAY_VOL, TBM_GETPOS, 0 ,0);

     voiceInMaxRange = SendMessage(hDlgSliderVoiceInGain, TBM_GETRANGEMAX, 0 ,0); 
     voiceInMinRange = SendMessage(hDlgSliderVoiceInGain, TBM_GETRANGEMIN, 0 ,0); 

     voiceOutMaxRange = SendMessage(hDlgSliderVoiceOutGain, TBM_GETRANGEMAX, 0 ,0); 
     voiceOutMinRange = SendMessage(hDlgSliderVoiceOutGain, TBM_GETRANGEMIN, 0 ,0); 

     micAnalogGainMaxRange  = SendMessage(hDlgSliderMicAnalogGain, TBM_GETRANGEMAX, 0 ,0); 
     micAnalogGainMinRange  = SendMessage(hDlgSliderMicAnalogGain, TBM_GETRANGEMAX, 0 ,0); 
     micDigitalGainMaxRange = SendMessage(hDlgSliderMicDigitalGain, TBM_GETRANGEMAX, 0 ,0); 
	 micDigitalGainMinRange = SendMessage(hDlgSliderMicDigitalGain, TBM_GETRANGEMAX, 0 ,0); 
 

	 SendDlgItemMessage(hDlg, IDC_AP_3, BM_SETCHECK, (WPARAM)BST_CHECKED , 0);

	 // meanwhile disable BT
	 EnableWindow( GetDlgItem( hDlg, IDC_CHECK_BT_HS ), FALSE );

	 modemAudioProfile = 3;
	 //SendDlgItemMessage(hDlg, IDC_RADIO_2, BM_SETSTYLE, BS_AUTORADIOBUTTON , MAKELPARAM(TRUE, 0));

            wfx.cbSize = 0;
			wfx.wFormatTag = WAVE_FORMAT_PCM;
			wfx.wBitsPerSample = 16;
			wfx.nSamplesPerSec = 44100;
			wfx.nChannels = 1;
			wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
			wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	 for(devId = 0; devId < waveOutGetNumDevs(); devId++)
	  {
		mmres = waveOutOpen(&hwo, devId, &wfx, 0, 0, CALLBACK_NULL);
		if(mmres == MMSYSERR_NOERROR)
			break;
	  }

	 if(mmres == MMSYSERR_NOERROR)
	  {
     	mmres = waveOutGetVolume((HWAVEOUT)devId, &volOut);
		//mmres = waveOutClose(hwo);

		volLeft = (volOut & 0x0000FFFF);
		volRight = ((volOut & 0xFFFF0000) >>16);

		curPos = ( (volLeft * maxRange)/0x0000FFFF );

		wsprintf( myTest, _T("%d"), curPos );
		SetDlgItemText( hDlg, IDC_VOLUME_BOX, myTest );

		SendDlgItemMessage(hDlg, IDC_SLIDER_PLAY_VOL, TBM_SETPOS, TRUE , curPos);

		//updateVoiceInSlider(HWND hDlg, HWND hDlgSliderVoiceInGain
		updateVoiceInSlider(hDlg, hDlgSliderVoiceInGain, &voiceInCurPos, voiceInMaxRange);
		updateVoiceOutSlider(hDlg, hDlgSliderVoiceOutGain, &voiceOutCurPos, voiceOutMaxRange);


		updateMicAnalogGainInSlider(hDlg, hDlgSliderMicAnalogGain, &micAnalogGainCurPos, micAnalogGainMaxRange);
		wsprintf( myTest, _T("%d"), sMicAnalogGainMax );
		SetDlgItemText(hDlg, IDC_VOL_MIC_ANALOG_MAX, myTest);

		updateMicDigitalGainInSlider(hDlg, hDlgSliderMicDigitalGain, &micDigitalGainCurPos, micDigitalGainMaxRange);
		wsprintf( myTest, _T("%d"), sMicDigitalGainMax );
		SetDlgItemText(hDlg, IDC_VOL_MIC_DIGITAL_MAX, myTest);

	  }

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
      pos = SendDlgItemMessage(hDlg, IDC_SLIDER_PLAY_VOL, TBM_GETPOS, 0 ,0);

	  if( pos != curPos )
	  {
        curPos = pos;

		wsprintf( myTest, _T("%d"), curPos );
		SetDlgItemText( hDlg, IDC_VOLUME_BOX, myTest );

        volLeft = (curPos * 0x0000FFFF) / maxRange;
		volRight = volLeft;

		volOut = volLeft | (volRight << 16);
		mmres = waveOutSetVolume((HWAVEOUT)devId, volOut);

	  }
	 }

	 if( lpNotifyMsg->hwndFrom == hDlgSliderVoiceInGain )
	 {
      int     InPos;
	  INT16  InGain;

	  InPos = SendMessage(hDlgSliderVoiceInGain, TBM_GETPOS, 0 , 0);

	  if( InPos != voiceInCurPos )
	  {
        voiceInCurPos = InPos;

		//InGain = sVoiceInGainMin + ((voiceInCurPos * (sVoiceInGainMax - sVoiceInGainMin)) / voiceInMaxRange);
		InGain = voiceInCurPos;
		wsprintf( myTest, _T("%d"), InGain );
	    SetDlgItemText( hDlg, IDC_VOLUME_VOICE_IN, myTest );
		putVoiceInGain(InGain, gBtOn);
	  }

	 }

	 if( lpNotifyMsg->hwndFrom == hDlgSliderVoiceOutGain )
	 {
      int     OutPos;
	  INT16  OutGain;

	  OutPos = SendMessage(hDlgSliderVoiceOutGain, TBM_GETPOS, 0 , 0);

	  if( OutPos != voiceOutCurPos )
	  {
        voiceOutCurPos = OutPos;

		//OutGain = sVoiceOutGainMin + ((voiceOutCurPos * (sVoiceOutGainMax - sVoiceOutGainMin)) / voiceOutMaxRange);
		OutGain = voiceOutCurPos;
		wsprintf( myTest, _T("%d"), OutGain );
	    SetDlgItemText( hDlg, IDC_VOLUME_VOICE_OUT, myTest );
		putVoiceOutGain(OutGain, gBtOn);
	  }

	 }

	 if( lpNotifyMsg->hwndFrom == hDlgSliderMicAnalogGain )
	 {
      int     OutPos;
	  UINT16  aGain;
	  DWORD   dwStep, dwTicks, dwRangeMin, dwRangeMax, rmnd;

	  OutPos = SendMessage(hDlgSliderMicAnalogGain, TBM_GETPOS, 0 , 0);

	  if( OutPos != micAnalogGainCurPos )
	  {
        micAnalogGainCurPos = OutPos;

		// TBM_GETRANGEMIN 
		dwRangeMin = SendMessage(hDlgSliderMicAnalogGain, TBM_GETRANGEMIN, 0 , 0);
		dwRangeMax = SendMessage(hDlgSliderMicAnalogGain, TBM_GETRANGEMAX, 0 , 0);
		dwTicks = SendMessage(hDlgSliderMicAnalogGain, TBM_GETNUMTICS, 0 , 0);  
        dwStep =  (dwRangeMax - dwRangeMin)/(dwTicks-1);
 
		rmnd = micAnalogGainCurPos % dwStep;
		if( rmnd != 0 )
        {
          if( rmnd < (dwStep/2) )
		  {
           aGain = ( (UINT16)micAnalogGainCurPos/dwStep) * dwStep;
		  }
		  else
		  {
           aGain = ( ((UINT16)micAnalogGainCurPos/dwStep) + 1) * dwStep;
		  }
		}
		else
         aGain = micAnalogGainCurPos;

		//SendMessage(hDlgSliderMicAnalogGain, TBM_SETPOS, TRUE , aGain); 

		//aGain = (micAnalogGainCurPos * (sMicAnalogGainMax - sMicAnalogGainMin)) / micAnalogGainMaxRange;
		//aGain += sMicAnalogGainMin;
		
		wsprintf( myTest, _T("%d"), aGain );
	    SetDlgItemText( hDlg, IDC_VOLUME_MIC_ANALOG, myTest );

		if( gExtMic == FALSE )
		  putMicAnalogGain(aGain);
		else
          putExtMicAnalogGain(aGain);
	  }

	 }

	 if( lpNotifyMsg->hwndFrom == hDlgSliderMicDigitalGain )
	 {
      int     OutPos;
	  int     dGain;

	  OutPos = SendMessage(hDlgSliderMicDigitalGain, TBM_GETPOS, 0 , 0);

	  if( OutPos != micDigitalGainCurPos )
	  {
        micDigitalGainCurPos = OutPos;

		//dGain = (micDigitalGainCurPos * (sMicDigitalGainMax - sMicDigitalGainMin)) / micDigitalGainMaxRange;
		//dGain += sMicDigitalGainMin;
		dGain = micDigitalGainCurPos; 
		wsprintf( myTest, _T("%d"), dGain );
	    SetDlgItemText( hDlg, IDC_VOLUME_MIC_DIGITAL, myTest );

		if( gExtMic == FALSE )
		  putMicDigitalGain(dGain);
		else
         putExtMicDigitalGain(dGain);
	  }

	 }

     return TRUE; 
	}

	case WM_COMMAND:
	{
      switch( LOWORD(wParam) )
	   {
		 case IDCANCEL:
            CloseHandle(g_hThr);
            g_pl.Stop();
			g_rec.Stop();
			turnVoiceOnOff(0, 0);
			modemOff(hDlg);
			CloseHandle(g_hCom);
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

					//ret = GetLastError();
           return TRUE;
		  }

		 case IDC_BUTTON_PLAY:
		  {
           TCHAR    myTest[32];
           
		   switch( gPlayState )
		   {
		    case  FILE_SILENT:
			 {
               if( gPlayFileOpenStatus == TRUE )
			   {
                 gPlayState = FILE_PLAYING;
                 g_pl.Start( ofnPlay.lpstrFile );
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
				 gPlayState = FILE_SILENT;

 		         wsprintf( myTest, _T("Play") );
		         SetDlgItemText( hDlg, IDC_BUTTON_PLAY, myTest );
			   }
              
			 } break;

			default: break;             
		   }

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
				 }

                 gPlayState = FILE_SILENT;
				 g_pl.Stop();

 		         wsprintf( myTest, _T("Play") );
		         SetDlgItemText( hDlg, IDC_BUTTON_PLAY, myTest );
			   }


           return TRUE;
		  }


        //////////////////////////// Record Controls ////////////////////////////
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

					//ret = GetLastError();
           return TRUE;
		  }

        //IDC_BUTTON_RECORDING
         case IDC_BUTTON_RECORDING:
		  {
 					wfxRec.cbSize = 0;
					wfxRec.wFormatTag = WAVE_FORMAT_PCM;
					wfxRec.wBitsPerSample = 16;
					//wfxRec.nSamplesPerSec = GetDlgItemInt(w, IDC_EDIT_SR, 0, 0);
					wfxRec.nSamplesPerSec = 44100;
//					wfxRec.nSamplesPerSec = 11025; WAVE_FORMAT_1M08
//					wfxRec.nSamplesPerSec = 16000;
//					wfxRec.nSamplesPerSec = 22050;
//					wfxRec.nSamplesPerSec = 44100;
//					wfxRec.nSamplesPerSec = 48000;
					wfxRec.nChannels = 1; /*(BST_CHECKED == SendMessage(GetDlgItem(w, IDC_CHECK_STEREO), BM_GETCHECK, 0, 0))?2:1;*/
					wfxRec.nBlockAlign = wfxRec.nChannels * wfxRec.wBitsPerSample / 8;
					wfxRec.nAvgBytesPerSec = wfxRec.nBlockAlign * wfxRec.nSamplesPerSec;

					g_rec.Start( ofnRec.lpstrFile, &wfxRec );

					updateMicAnalogGainInSlider(hDlg, hDlgSliderMicAnalogGain, &micAnalogGainCurPos, micAnalogGainMaxRange);
					updateMicDigitalGainInSlider(hDlg, hDlgSliderMicDigitalGain, &micDigitalGainCurPos, micDigitalGainMaxRange);

           return TRUE;
		  }
 
         // IDC_BUTTON_STOP_REC
         case IDC_BUTTON_STOP_REC:
		  {
           g_rec.Stop();

		   return TRUE;
		  }


        /////////////////////// MODEM Controls ////////////////////////////////// 
		 case  IDC_VOICE_MODEM:
		  {
            BOOL micMute;

                 if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
				 {
                    shadeModemCheckBox(hDlg, TRUE); 
					shadeModemAPButtons(hDlg, TRUE);
					shadeModemBTButton(hDlg, TRUE);
					
					if( modemOn(hDlg) )
					{
                      if( turnVoiceOnOff(1, gBtOn) )
						wsprintf( myTest, _T("Voice On") );
					  else
                        wsprintf( myTest, _T("Voice On failure") );

  				      shadeModemDialButtons(hDlg, FALSE);
					  shadeModemBTButton(hDlg, FALSE);
					  shadeRecButtons(hDlg, TRUE);

	 				  updateVoiceInSlider(hDlg, hDlgSliderVoiceInGain, &voiceInCurPos, voiceInMaxRange);
					  updateVoiceOutSlider(hDlg, hDlgSliderVoiceOutGain, &voiceOutCurPos, voiceOutMaxRange);
					  updateMicAnalogGainInSlider(hDlg, hDlgSliderMicAnalogGain, &micAnalogGainCurPos, micAnalogGainMaxRange);
					  updateMicDigitalGainInSlider(hDlg, hDlgSliderMicDigitalGain, &micDigitalGainCurPos, micDigitalGainMaxRange);

					  getMicMute(&micMute);
					}
					else
                     shadeModemAPButtons(hDlg, FALSE);

                    shadeModemCheckBox(hDlg, FALSE); 
					//myOpenCloseCom(hDlg, wParam);
				 }
				 else
				 {
                    //myOpenCloseCom(hDlg, wParam);
                    shadeModemCheckBox(hDlg, TRUE);
					shadeModemDialButtons(hDlg, TRUE);
					shadeModemBTButton(hDlg, TRUE);
					
					if( modemOff(hDlg) )
					{
                      shadeModemAPButtons(hDlg, FALSE);
					  
					  if( turnVoiceOnOff(0, gBtOn) )
						wsprintf( myTest, _T("Voice Off") );
					  else
                        wsprintf( myTest, _T("Voice Off failure") );

					  getMicMute(&micMute);
					}

					shadeModemCheckBox(hDlg, FALSE);
					shadeRecButtons(hDlg, FALSE);
				 }

                 //SetDlgItemText( hDlg, IDC_STATUS1, myTest );

		   return TRUE;
		  }

         case IDC_CHECK_BT_HS: 
		  {
           BOOL  bret;
           if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
		   {
             if( gModemOn == FALSE )
			 {
				bret = AGControl(AG_SERVICE_START);
                bret = AGControl(AG_CONNECT);
			 	bret = AGControl(AG_AUDIO_ON);

                turnBTPlbOnOff(TRUE); 
			 }
		   }
		   else
		   {
             if( gModemOn == FALSE )
			 {
               turnBTPlbOnOff(FALSE); 

			   AGControl(AG_AUDIO_OFF);
               AGControl(AG_DISCONNECT);
			   AGControl(AG_SERVICE_STOP);
			 }
		   }

           return TRUE;  
		  }


		 case IDC_CHECK_EXT_AUDIO: 
		  {
           //BOOL  bret;
           if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
		   {
             turnExtAudioOnOff(TRUE);
             gExtMic = TRUE;
	  	     updateMicAnalogGainInSlider(hDlg, hDlgSliderMicAnalogGain, &micAnalogGainCurPos, micAnalogGainMaxRange);
			 updateMicDigitalGainInSlider(hDlg, hDlgSliderMicDigitalGain, &micDigitalGainCurPos, micDigitalGainMaxRange);
		   }
		   else
		   {
             gExtMic = FALSE;
             turnExtAudioOnOff(FALSE);  

			 updateMicAnalogGainInSlider(hDlg, hDlgSliderMicAnalogGain, &micAnalogGainCurPos, micAnalogGainMaxRange);
			 updateMicDigitalGainInSlider(hDlg, hDlgSliderMicDigitalGain, &micDigitalGainCurPos, micDigitalGainMaxRange);

			 if( gModemOn == TRUE )
			 {
              turnVoiceOnOff(0, gBtOn); 
              turnVoiceOnOff(1, gBtOn);
			 }
		   }

           return TRUE;  
		  }

		 case IDC_BT_ON_OFF:
		  {
            if(IsDlgButtonChecked(hDlg, wParam) == BST_CHECKED)
			 {
              shadeModemBTButton(hDlg, TRUE);
              if( !gBtOn )
			  {
               EnableWindow( GetDlgItem( hDlg, IDC_DIAL ), FALSE );
	           EnableWindow( GetDlgItem( hDlg, IDC_HANG_UP ), FALSE );
	           EnableWindow( GetDlgItem( hDlg, IDC_ANSWER ), FALSE );

				if( g_hCom != INVALID_HANDLE_VALUE )
				{
				 CloseHandle(g_hCom);
				 g_hCom = INVALID_HANDLE_VALUE;
				}

				AGControl(AG_SERVICE_START);

                if( AGControl(AG_CONNECT) )
				{
					AGControl(AG_AUDIO_ON);
                  //if( AGControl(AG_AUDIO_ON) )
				  //{
					if( gModemOn == TRUE )
					{
					  turnVoiceOnOff(0, FALSE);
					  turnVoiceOnOff(1, TRUE);

					  gBtOn = TRUE;
	 				  updateVoiceInSlider(hDlg, hDlgSliderVoiceInGain, &voiceInCurPos, voiceInMaxRange);
					  updateVoiceOutSlider(hDlg, hDlgSliderVoiceOutGain, &voiceOutCurPos, voiceOutMaxRange);
					  updateMicAnalogGainInSlider(hDlg, hDlgSliderMicAnalogGain, &micAnalogGainCurPos, micAnalogGainMaxRange);
					  updateMicDigitalGainInSlider(hDlg, hDlgSliderMicDigitalGain, &micDigitalGainCurPos, micDigitalGainMaxRange);
					}
				  //}
				}
			  }

			  if( !gBtOn )
			  {
               EnableWindow( GetDlgItem( hDlg, IDC_DIAL ), TRUE );
	           EnableWindow( GetDlgItem( hDlg, IDC_HANG_UP ), TRUE );
	           EnableWindow( GetDlgItem( hDlg, IDC_ANSWER ), TRUE );
			  }

			  shadeModemBTButton(hDlg, FALSE);
			 }
			else
			 {
              if( gBtOn )
			  {
                if( gModemOn == TRUE )
				{
                  shadeModemBTButton(hDlg, TRUE);

				  AGControl(AG_AUDIO_OFF);
                  AGControl(AG_DISCONNECT);
				  AGControl(AG_SERVICE_STOP);
                  //AGControl(AG_AUDIO_OFF);
                 
				  // Now open COM0 for Voice Modem!
				  if( g_hCom == INVALID_HANDLE_VALUE )
				  {
                   g_hCom = __OpenComPort(g_ComPort);  
                   if( g_hCom == INVALID_HANDLE_VALUE )
				   {
                    RETAILMSG(1, (L"DIGVOICE:%S, re-opening COM0 failure\r\n",__FUNCTION__));
                    return TRUE; //???
				   }
				  }

				  turnVoiceOnOff(0, TRUE);
                  turnVoiceOnOff(1, FALSE);
                  gBtOn = FALSE;
	 			  updateVoiceInSlider(hDlg, hDlgSliderVoiceInGain, &voiceInCurPos, voiceInMaxRange);
				  updateVoiceOutSlider(hDlg, hDlgSliderVoiceOutGain, &voiceOutCurPos, voiceOutMaxRange);
				  updateMicAnalogGainInSlider(hDlg, hDlgSliderMicAnalogGain, &micAnalogGainCurPos, micAnalogGainMaxRange);
				  updateMicDigitalGainInSlider(hDlg, hDlgSliderMicDigitalGain, &micDigitalGainCurPos, micDigitalGainMaxRange);

                  EnableWindow( GetDlgItem( hDlg, IDC_DIAL ), TRUE );
	              EnableWindow( GetDlgItem( hDlg, IDC_HANG_UP ), TRUE );
	              EnableWindow( GetDlgItem( hDlg, IDC_ANSWER ), TRUE );

				  shadeModemBTButton(hDlg, FALSE);
				}
			  }
			 }

            return TRUE;  
		  }

		 case  IDC_AP_0:
		  {
           if(IsDlgButtonChecked(hDlg, IDC_AP_0) == BST_CHECKED)
             modemAudioProfile = 0;
		   //else
           //  modemAudioProfile = -1;

           return TRUE;
		  }

		 case  IDC_AP_1:
		  {
           if(IsDlgButtonChecked(hDlg, IDC_AP_1) == BST_CHECKED)
             modemAudioProfile = 1;
		   //else
           //  modemAudioProfile = -1;

           return TRUE;
		  }

		 case  IDC_AP_2:
		  {
           if(IsDlgButtonChecked(hDlg, IDC_AP_2) == BST_CHECKED)
             modemAudioProfile = 2;
		   //else
           //  modemAudioProfile = -1;

           return TRUE;
		  }

		 case  IDC_AP_3:
		  {
           if(IsDlgButtonChecked(hDlg, IDC_AP_3) == BST_CHECKED)
             modemAudioProfile = 3;
		   //else
           //  modemAudioProfile = -1;

           return TRUE;
		  }


		 case  IDC_DIAL:
		  {
		 	 // TCHAR editLine[32];
			  LONG  len;
			  size_t origsize; 
			  size_t convertedChars = 0;
			  char   ATstring[NUM_OF_EDIT_CHARS-2];

			  if( gBtOn )
                return TRUE;

              EnableWindow( GetDlgItem( hDlg, IDC_DIAL ), FALSE );
	          EnableWindow( GetDlgItem( hDlg, IDC_HANG_UP ), FALSE );
	          EnableWindow( GetDlgItem( hDlg, IDC_ANSWER ), FALSE );

			  *(WORD *)gEditLine = NUM_OF_EDIT_CHARS - sizeof(TCHAR);
			  len = SendDlgItemMessage(hDlg, IDC_DIAL_EDIT_BOX, EM_GETLINE, 0 , (LPARAM)gEditLine);
			  //IDC_DIAL_EDIT_BOX

			  origsize = wcslen(gEditLine) + 1;
              wcstombs_s(&convertedChars, ATstring, origsize, gEditLine, _TRUNCATE);

			  MIC_GSMVoiceCMD(g_hCom, GSM_VOICE_DIAL, ATstring);

              EnableWindow( GetDlgItem( hDlg, IDC_DIAL ), TRUE );
	          EnableWindow( GetDlgItem( hDlg, IDC_HANG_UP ), TRUE );
	          EnableWindow( GetDlgItem( hDlg, IDC_ANSWER ), TRUE );

			  return TRUE;
		  }

#if 0
		 case  IDC_BUTTON_TEST:
		  {
			  LONG  len;
			  RECT  eRect; 
			  //HWND  hDlgEditBox; 
			  size_t origsize; 
			  size_t convertedChars = 0;
			  char   ATstring[NUM_OF_EDIT_CHARS-2];
			  

			  //WORD  numOfEditChar;

			  //EM_GETLIMITTEXT 
              len = SendDlgItemMessage(hDlg, IDC_DIAL_EDIT_BOX, EM_GETLIMITTEXT, (WPARAM)0, (LPARAM)0); 
			  len = SendDlgItemMessage(hDlg, IDC_DIAL_EDIT_BOX, EM_GETRECT, (WPARAM)0, (LPARAM)&eRect); 

			  //hDlgEditBox = GetDlgItem( hDlg, IDC_DIAL_EDIT_BOX );
			  //len = SendMessage(hDlgEditBox, EM_GETLINE, 0, (LPARAM)gEditLine);
              //numOfEditChar = 8;

			  *(WORD *)gEditLine = NUM_OF_EDIT_CHARS - sizeof(TCHAR);
  			  len = SendDlgItemMessage(hDlg, IDC_DIAL_EDIT_BOX, EM_GETLINE, (WPARAM)0, (LPARAM)gEditLine);

			  origsize = wcslen(gEditLine) + 1;
              wcstombs_s(&convertedChars, ATstring, origsize, gEditLine, _TRUNCATE);


           return TRUE;
		  }
#endif

		 case  IDC_HANG_UP:
		  {
			  if( gBtOn )
                return TRUE;

			  EnableWindow( GetDlgItem( hDlg, IDC_HANG_UP ), FALSE );
              EnableWindow( GetDlgItem( hDlg, IDC_DIAL ), FALSE );
	          EnableWindow( GetDlgItem( hDlg, IDC_ANSWER ), FALSE );

			  RETAILMSG(1, (L"DIGVOICE:%S, start Hang-Up\r\n",__FUNCTION__));

			  MIC_GSMVoiceCMD(g_hCom, GSM_VOICE_HANG_UP, NULL);

			  RETAILMSG(1, (L"DIGVOICE:%S, stop Hang-Up\r\n",__FUNCTION__));

	          EnableWindow( GetDlgItem( hDlg, IDC_ANSWER ), TRUE );
              EnableWindow( GetDlgItem( hDlg, IDC_DIAL ), TRUE );
			  EnableWindow( GetDlgItem( hDlg, IDC_HANG_UP ), TRUE );

			  return TRUE;
		  }

		 case  IDC_ANSWER:
		  {
			  if( gBtOn )
                return TRUE;

	          EnableWindow( GetDlgItem( hDlg, IDC_ANSWER ), FALSE );
              EnableWindow( GetDlgItem( hDlg, IDC_DIAL ), FALSE );
	          EnableWindow( GetDlgItem( hDlg, IDC_HANG_UP ), FALSE );

			  MIC_GSMVoiceCMD(g_hCom, GSM_VOICE_ANSWER, NULL);

	          EnableWindow( GetDlgItem( hDlg, IDC_HANG_UP ), TRUE );
              EnableWindow( GetDlgItem( hDlg, IDC_DIAL ), TRUE );
	          EnableWindow( GetDlgItem( hDlg, IDC_ANSWER ), TRUE );

			  return TRUE;
		  }

	   }

     return TRUE; 
	}

	case WM_USER:
	{
      gPlayState = FILE_SILENT;

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

	if( InitCommonControlsEx(&slideBarControl) )
 	 ret = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, (DLGPROC)mainDlgProc);

	if( ret == -1 )
	{
     ret =	GetLastError();
	}

	return 0;
}



BOOL modemOnTest(HWND hDlg)
{
  INT32    ret;

  if( gModemOn == TRUE )
   return(TRUE);

  if( g_hCom == INVALID_HANDLE_VALUE )
  {
    g_hCom = __OpenComPortTest(g_ComPort);
    if( g_hCom == INVALID_HANDLE_VALUE )
     return(FALSE);
  }

		if( modemAudioProfile != -1 )
		{
		 ret = MIC_GSMVoice(g_hCom, 1, modemAudioProfile);
		}
		else
		{
         gModemOn = FALSE;
    	 return(FALSE);
		}

		if( ret == 0 )
		{
		  gModemOn = TRUE;
		  return(TRUE);
		}
		else
		{
         gModemOn = FALSE;
    	 return(FALSE);
		}

}



BOOL modemOn(HWND hDlg)
{
  INT32    retPower, ret;
  BOOL     bRet;
  //char     respStr[128];

  if( gModemOn == TRUE )
   return(TRUE);
   
  if( modemAudioProfile == -1 )
  {
    gModemOn = FALSE;
    return(FALSE);
  }

  //INT32  MIC_GSMPower(INT32 power);
  retPower = MIC_GSMPower(1);
  //ret = 0;

  if( gBtOn )
  {
   // If Bluetooth is On, temporary stop AudioGateway service for release com0
	 bRet = AGControl(AG_SERVICE_STOP);

     //if( retPower == GSM_OK )
	   g_hCom = __OpenComPort(g_ComPort);
	 //else
     // g_hCom = __OpenComPort(L"COM6:");
   
	 if( g_hCom == INVALID_HANDLE_VALUE )
	 {
       MIC_GSMPower(0);
       return(FALSE);
	 }

	 ret = MIC_GSMVoice(g_hCom, 1, modemAudioProfile);
	 if( ret == ERROR_SUCCESS )
	  {
	   //bRet = sendATCmdApp(g_hCom, "AT#DIALMODE=2", respStr);

       // Close COM0, let AG Service to use it
       CloseHandle(g_hCom);
	   g_hCom = INVALID_HANDLE_VALUE;

       // Start AG Service 
       bRet = AGControl(AG_SERVICE_START);
	   bRet = AGControl(AG_CONNECT);
	   //bRet = AGControl(AG_AUDIO_ON);
	   gModemOn = TRUE;
	   return(TRUE);
	  }
	 else
	  {
       // no need AG Service if cannot to start modem ??
       // Service already stoped here.
       gModemOn = FALSE;
       CloseHandle(g_hCom);
	   g_hCom = INVALID_HANDLE_VALUE;
	   MIC_GSMPower(0);
       return(FALSE);
	  }
  }
  else
  {
	  //if( retPower == GSM_OK )
	   g_hCom = __OpenComPort(g_ComPort);
	  //else
	  // g_hCom = __OpenComPort(L"COM6:");

	  if( g_hCom == INVALID_HANDLE_VALUE )
	   {
		MIC_GSMPower(0);
		return(FALSE);
	   }

	 ret = MIC_GSMVoice(g_hCom, 1, modemAudioProfile);
	 if( ret == ERROR_SUCCESS )
	  {
#if 0
        // Test ATZ influence
        bRet = sendATCmdApp(g_hCom, "ATZ\r\n", respStr);  
		bRet = sendATCmdApp(g_hCom, "AT#DVI?\r\n", respStr);  

        bRet = sendATCmdApp(g_hCom, "AT+FCLASS=8\r\n", respStr);  
        bRet = sendATCmdApp(g_hCom, "ATX0\r\n", respStr);  
		bRet = sendATCmdApp(g_hCom, "AT&D0\r\n", respStr);
        bRet = sendATCmdApp(g_hCom, "AT#DIALMODE=0\r\n", respStr); 
#endif
	   gModemOn = TRUE;
	   return(TRUE);
	  }
	 else
	  {
       gModemOn = FALSE;
       CloseHandle(g_hCom);
	   g_hCom = INVALID_HANDLE_VALUE;
	   MIC_GSMPower(0);
       return(FALSE);
	  }
  }
}


BOOL modemOff(HWND hDlg)
{
  INT32    ret;
  BOOL     bRet;

    if( gBtOn )
	{
		 // Stop AGService
     bRet = AGControl(AG_SERVICE_STOP); 
	 // Open COM Port here ! 
	 g_hCom = __OpenComPort(g_ComPort);
	}

  
  //INT32  MIC_GSMPower(INT32 power);
  ret = MIC_GSMVoice(g_hCom, 0, 3);

  ret = MIC_GSMPower(0);

  gModemOn = FALSE;


    if( g_hCom != INVALID_HANDLE_VALUE )
	{
     CloseHandle(g_hCom);
	 g_hCom = INVALID_HANDLE_VALUE;
	}

	if( ( ret != 0  )&& ( ret != GSM_ERROR_CONTROLLED_BY_MUX ) )
	{
		//wsprintf( myTest, _T("GSM Modem turn off failure") );
		//SetDlgItemText( hDlg, IDC_STATUS1, myTest );
		return(FALSE);
	}
	else
	{
		//wsprintf( myTest, _T("GSM Modem turned off") );
		//SetDlgItemText( hDlg, IDC_STATUS1, myTest );
		return(TRUE);
	}
}


VOID shadeModemCheckBox(HWND hDlg, BOOL bShade)
{
  EnableWindow( GetDlgItem( hDlg, IDC_VOICE_MODEM ), !bShade );
}

VOID shadeModemDialButtons(HWND hDlg, BOOL bShade)
{

  EnableWindow( GetDlgItem( hDlg, IDC_DIAL ), !bShade );
  EnableWindow( GetDlgItem( hDlg, IDC_HANG_UP ), !bShade );
  EnableWindow( GetDlgItem( hDlg, IDC_ANSWER ), !bShade );

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

VOID shadeRecButtons(HWND hDlg, BOOL bShade)
{
  EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_REC_FILE ), !bShade );
  EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_RECORDING ), !bShade );
  EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP_REC ), !bShade );
}



static void updateVoiceInSlider(HWND hDlg, HWND hDlgSliderVoiceInGain, int *pCurPos, int voiceInMaxR)
{
  INT16 voiceInGain;

		//voiceGain, voiceGainMin, voiceGainMax
		if( getVoiceInGain(&voiceInGain, &sVoiceInGainMin, &sVoiceInGainMax, gBtOn) )
		{
          if( (sVoiceInGainMax - sVoiceInGainMin) != 0 )
		  {
           SendMessage(hDlgSliderVoiceInGain, TBM_SETRANGEMIN, TRUE , sVoiceInGainMin);  
		   SendMessage(hDlgSliderVoiceInGain, TBM_SETRANGEMAX, TRUE , sVoiceInGainMax);  

           //*pCurPos = ( (voiceInGain - sVoiceInGainMin) * voiceInMaxR)/(sVoiceInGainMax - sVoiceInGainMin);
		   *pCurPos = (voiceInGain - sVoiceInGainMin);
           SendMessage(hDlgSliderVoiceInGain, TBM_SETPOS, TRUE , *pCurPos); 

		   wsprintf( myTest, _T("%d"), voiceInGain );
		   SetDlgItemText( hDlg, IDC_VOLUME_VOICE_IN, myTest );
		  }

			wsprintf( myTest, _T("%d"), sVoiceInGainMax );
			SetDlgItemText(hDlg, IDC_VOL_IN_MAX, myTest);

			wsprintf( myTest, _T("%d"), sVoiceInGainMin );
			SetDlgItemText(hDlg, IDC_VOL_IN_MIN, myTest);
		}
}


static void updateVoiceOutSlider(HWND hDlg, HWND hDlgSliderVoiceOutGain, int *pCurPos, int voiceOutMaxR)
{
	INT16 voiceOutGain;

		//voiceGain, voiceGainMin, voiceGainMax
		if( getVoiceOutGain(&voiceOutGain, &sVoiceOutGainMin, &sVoiceOutGainMax, gBtOn) )
		{
          if( (sVoiceOutGainMax - sVoiceOutGainMin) != 0 )
		  {
           SendMessage(hDlgSliderVoiceOutGain, TBM_SETRANGEMIN, TRUE , sVoiceOutGainMin);  
		   SendMessage(hDlgSliderVoiceOutGain, TBM_SETRANGEMAX, TRUE , sVoiceOutGainMax);  

           //*pCurPos = ( (voiceOutGain - sVoiceOutGainMin) * voiceOutMaxR)/(sVoiceOutGainMax - sVoiceOutGainMin);
		   *pCurPos = (voiceOutGain - sVoiceOutGainMin);
           SendMessage(hDlgSliderVoiceOutGain, TBM_SETPOS, TRUE , *pCurPos); 

		   wsprintf( myTest, _T("%d"), voiceOutGain );
		   SetDlgItemText( hDlg, IDC_VOLUME_VOICE_OUT, myTest );
		  }

			wsprintf( myTest, _T("%d"), sVoiceOutGainMax );
			SetDlgItemText(hDlg, IDC_VOL_OUT_MAX, myTest);

			wsprintf( myTest, _T("%d"), sVoiceOutGainMin );
			SetDlgItemText(hDlg, IDC_VOL_OUT_MIN, myTest);
		}
}

static void updateMicAnalogGainInSlider(HWND hDlg, HWND hDlgSliderMicAnalogGain, int *pMicAnalogGainCurPos, int micAnalogGainMaxRange)
{
 UINT16   micAnalogGain;
 BOOL     bRet;
 
        if( gExtMic == FALSE )
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
		   if( gExtMic == TRUE )
		   {
             SendMessage(hDlgSliderMicAnalogGain, TBM_SETTICFREQ, 10 , 0);  
		   }
		   else
		   {
             SendMessage(hDlgSliderMicAnalogGain, TBM_SETTICFREQ, 1 , 0);  
		   }

           //*pMicAnalogGainCurPos = ( (micAnalogGain - sMicAnalogGainMin) * micAnalogGainMaxRange)/(sMicAnalogGainMax - sMicAnalogGainMin);
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
}

static void updateMicDigitalGainInSlider(HWND hDlg, HWND hDlgSliderMicDigitalGain, int *pMicDigitalGainCurPos, int micDigitalGainMaxRange)
{
 int      micDigitalGain;
 INT16    micDigitalGainS, micDigitalGainMinS, micDigitalGainMaxS;
 UINT16   micDigitalGainU, micDigitalGainMinU, micDigitalGainMaxU;
 BOOL     bRet;
 
        if( gExtMic == FALSE )
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

           //*pMicDigitalGainCurPos = ( (micDigitalGain - sMicDigitalGainMin) * micDigitalGainMaxRange)/(sMicDigitalGainMax - sMicDigitalGainMin);
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

}



DWORD WINAPI RunnerThread(LPVOID lpParam)
{
 DWORD       dwRet; 
 VOICE_LINE  *pVoiceLine;
 HANDLE      hLineEvent;
 long        lineRet;
 LINEMESSAGE lineMsg; 

   pVoiceLine = (VOICE_LINE *)lpParam;

   hLineEvent = pVoiceLine->pLineInitEx->Handles.hEvent;

   while(1)
   {
    dwRet = WaitForSingleObject( hLineEvent , INFINITE );
	switch(dwRet)
	{
	 case  WAIT_OBJECT_0:
	  {
        lineRet = lineGetMessage(*pVoiceLine->phLine, &lineMsg, INFINITE);
	  } break;

	 case WAIT_FAILED: break;
	 case WAIT_TIMEOUT: break;
	 default: break;
	}
   }

  return(0);
}
