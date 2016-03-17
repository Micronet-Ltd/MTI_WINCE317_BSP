//-----------------------------------------------------------------------------
// Copyright 2010 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//  File:  gsm_api.c
//
//  This file contains GSM Modem component API code.
//
//  Created by Anna Rayer    2010
//-----------------------------------------------------------------------------

#include <windows.h>
#include <devload.h>
#include <string.h>
#include <MicUserSdk.h>
#include <gprs.h>
#include <comm.h>
#include <gsm_api.h>
#include <gsm710_api.h>
#include <oal.h>
#include <oalex.h>
#include <args.h>

#define GSM_DBG_PRN                           0

//------------------------------------------------------------------------------
// Function name	: MIC_GSMPower
// Description	    : This function turns on/off the GSM modem
// Return type		: 0 if succeeded, error number if failed.
// Argument         : power – 0 if we want to turn off the Modem and 1 if we want 
//                  : to turn on the modem.
//------------------------------------------------------------------------------
INT32  MIC_GSMPower(INT32 power)
{
	DWORD error;
	UINT32 rc = GSM_OK;
	HANDLE hPort;
	HKEY hKey1 = NULL;
	BOOL isMuxActivated = FALSE;
  
	RETAILMSG(GSM_DBG_PRN, (L"GSM API:+%S, power = %d\r\n",__FUNCTION__, power));

	// we need to check if MUX is activated. In this case we can't turn off the modem

	if(!power)
	{

		LONG lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, DRIVER_MUX07_10_REG_KEY1, 0, 0, &hKey1) ;
		if (ERROR_SUCCESS == lStatus)
		{	

			DWORD dwPortIndex;
			DWORD dwSize = sizeof(DWORD);
			DWORD dwValType = REG_DWORD;
			lStatus = RegQueryValueEx(hKey1, TEXT("Index"), NULL, &dwValType, (LPBYTE)&dwPortIndex, &dwSize);
			if (ERROR_SUCCESS == lStatus)
			{			
				TCHAR szCtrlPort[16];
				wsprintf(szCtrlPort, TEXT("COM%d:"), dwPortIndex);
				hPort = CreateFile(szCtrlPort, DEVACCESS_BUSNAMESPACE, 0, 0, 0, 0, 0);
				if (hPort != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hPort);
					RETAILMSG(GSM_DBG_PRN, (L"GSM API:+%S.Error: Mux is activated, so we don't turn off the modem\r\n",__FUNCTION__, power));
					return GSM_ERROR_CONTROLLED_BY_MUX;
				}
			}
		}

		hPort = CreateFile(BTMODEM_PREFIX, 0, 0, NULL, 0, 0, NULL);
		if (hPort == INVALID_HANDLE_VALUE)
			return GSM_ERROR_CONTROLLED_BY_BTMODEM;

		CloseHandle(hPort);
	}


	

	// Open the serial port.
	hPort = CreateFile(GPRS_IDENTIFIER, 0, 0, NULL, 0, 0, NULL);

	if (hPort == INVALID_HANDLE_VALUE)
	{
		RETAILMSG(1, (L"GSM API:+%S. Error: Open Modem handle failed error = %d\r\n",__FUNCTION__, GetLastError ()));
		return GSM_COMPORT_ERROR;
	}

	
	BOOL ret = DeviceIoControl(hPort, IOCTL_SET_MODEM_POWER_STATE, &power,
	        sizeof(power), &error, sizeof(DWORD), NULL, NULL);
	if(!ret)
		rc = GSM_DRIVER_ERROR;
	else
		rc = error;

	CloseHandle(hPort);
	
 
	RETAILMSG(GSM_DBG_PRN, (L"GSM API:-%S, rc = %d\r\n",__FUNCTION__, rc));
    return rc;
}


//------------------------------------------------------------------------------
// Function name	: MIC_GSMGetPowerStatus
// Description	    : This function returns modem's power status.0 - if modem
//           		: is turned off, 1- if modem is turned on.
// Argument         : OUT: returns 0 - if modem is turned off, 1- if modem is turned on.
// Return type		: 0 if succeeded, error number if failed. Use GetLastError() 
//                  : to get detail error.
//------------------------------------------------------------------------------
INT32  MIC_GSMGetPowerStatus(UINT32* power)
{
	UINT32 rc = GSM_OK;
	DWORD dwNumberOfBytes;
  
	// Open the serial port.
	HANDLE hPort = CreateFile(GPRS_IDENTIFIER, 0, 0, NULL, 0, 0, NULL);

	if (hPort == INVALID_HANDLE_VALUE)
	{
		RETAILMSG (1,(TEXT("MIC_GSMPower. Error: Open Modem handle failed error = %d \r\n"),GetLastError ()));
		return GSM_COMPORT_ERROR;
	}

	
	BOOL ret = DeviceIoControl(hPort, IOCTL_GET_MODEM_POWER_STATE,0,0, power,sizeof(UINT32), &dwNumberOfBytes, NULL);
	if(!ret)
		rc = GSM_DRIVER_ERROR;

	CloseHandle(hPort);
	
 
    return rc;
}

//------------------------------------------------------------------------------
// Function name	: MIC_GSMVoice
// Description	    : This function turn on/off the GSM modem Voice
// Return type		: 0 if succeeded, error number if failed
// Argument  1      : modemVoiceOn = 1 - On, 0 - Off
// Argument  2      : GSMAudioProfile, 0 ... 3, See table below:
// Notes            : Modem should be Off when this function is called.
//                    And any thread listening to Modem Com port should be closed.
//                    --------------------------------- 
//                    |  Number |  Description        |
//                    |   0     |   standard          |
//                    |   1     |   office            | 
//                    |   2     |   open air          |
//                    |   3     |   small /medium car | 
//                    ---------------------------------
//------------------------------------------------------------------------------
INT32  MIC_GSMVoice(HANDLE hCom, INT32 modemVoiceOn, INT32 GSMAudioProfile)
{
 BOOL       ret;
 INT32      iRet; 
 UINT32     power;
 char       atCmd[32];
 char       audioProfileNum[2];
 int        len, mode, stat, timeout;
 char       *pCREGStr;
 char       respStr[MAX_SIZE_TO_READ];

 RETAILMSG(GSM_DBG_PRN, (L"GSM API:+%S, hCom = 0x%X, modemVoiceOn = %d, GSMAudioProfile = %d\r\n",__FUNCTION__, hCom, modemVoiceOn, GSMAudioProfile));

 if( hCom == INVALID_HANDLE_VALUE )
  {
   RETAILMSG (1,(TEXT("MIC_GSMVoice Error: Invalid handle = 0x%X\r\n"), hCom ));
   return(GSM_ERROR_INVALID_PARAMETER);
  }


 if( ( modemVoiceOn != 1 )&&( modemVoiceOn != 0 ) )
  {
   RETAILMSG (1,(TEXT("MIC_GSMVoice Error: Invalid modem On/Off parameter = %d \r\n"), modemVoiceOn ));
   return(GSM_ERROR_INVALID_PARAMETER);
  }

 if( ( modemVoiceOn == 1 ) && ((GSMAudioProfile < 0)||(GSMAudioProfile > 3)) )
  {
   RETAILMSG (1,(TEXT("MIC_GSMVoice Error: Invalid Audio Profile = %d \r\n"), GSMAudioProfile ));
   return(GSM_ERROR_INVALID_PARAMETER);
  }

 // Check Modem Power status. If Modem Power is Off - exit.
 iRet = MIC_GSMGetPowerStatus(&power);
 if( ( iRet != GSM_OK )||(power != 1) )
 {
   RETAILMSG(1, (L"GSM API:%S, Modem Power is Off, iRet = %d, power = %d\r\n",__FUNCTION__, iRet, power));
   return(GSM_ERROR_MODEM_ALREADY_OFF);
 }

 if( modemVoiceOn == 1 )
 {

	 // Check presence of SIM card, first of all
     if( !sendATCmd(hCom, "AT#QSS?\r\n", respStr) )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#QSS? cmd failed\r\n")));
		return(GSM_ERROR_AT_CMD);
	  }
	 else
	 {
       // GSM_DBG_PRN 
	   pCREGStr = strstr(respStr, "#QSS:");
	   if( pCREGStr != NULL )
	   {
        // Skip mode field
        pCREGStr = strchr(respStr, ',');
		iRet = 0;
		stat = 0;
		if( pCREGStr != NULL )
		{
          while( (pCREGStr[iRet] != '0')&& (pCREGStr[iRet] != '1') )
		  {
            if( ( iRet < MAX_SIZE_TO_READ )&&(pCREGStr[iRet] != '\r')&&
				(pCREGStr[iRet] != '\n')&& (pCREGStr[iRet] != '\0') 
			   )
		     iRet++;
			else
             stat = -1; // nod found, parsing failed 
		  }

		  if( stat == -1 )
		  {
		   RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#QSS? parsing failed 1\r\n")));
		   return(GSM_ERROR_AT_CMD);
		  }
		  
          stat = pCREGStr[iRet] - '0';
		  if( ( stat != 0 )&&(stat != 1) )
		  {
		   RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#QSS? parsing failed 2\r\n")));
		   return(GSM_ERROR_AT_CMD);
		  }

         if( stat == 0 )
		 {
          RETAILMSG (1,(TEXT("MIC_GSMVoice Error: SIM NOT INSERTED\r\n")));
		  return(GSM_ERROR_SIM_NOT_INSERTED);
		 }

		}
	   }
	 }

	 // Config as GSM Modem Voice as Slave
     if( !sendATCmd(hCom, "AT#DVI=1,1,0\r\n", respStr) )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#DVI cmd failed\r\n")));
		return(GSM_ERROR_AT_CMD);
	  }

	 // disables the headset sidetone
	 if( !sendATCmd(hCom, "AT#SHFSD=0\r\n", respStr) )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#SHFSD cmd failed\r\n")));
		return(GSM_ERROR_AT_CMD);
	  }

	 // disables the handset sidetone
	 if( !sendATCmd(hCom, "AT#SHSSD=0\r\n", respStr) )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#SHSSD cmd failed\r\n")));
		return(GSM_ERROR_AT_CMD);
	  }

	 //strcpy_s(atCmd, 32, "AT#PSEL=");
	 strcpy(atCmd, "AT#PSEL=");
	 audioProfileNum[0] = (char)GSMAudioProfile + '0';
	 audioProfileNum[1] = '\0';
	 //strcat_s(atCmd, 32, audioProfileNum);
	 strcat(atCmd, audioProfileNum);
	 //strcat_s(atCmd, 32, "\r\n");
	 strcat(atCmd, "\r\n");

	 if( !sendATCmd(hCom, atCmd, respStr) )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#PSEL cmd failed, cmd = %S\r\n"), atCmd));
		return(GSM_ERROR_AT_CMD);
	  }

	BOOL isEchoCancellerExists = FALSE;
	card_ver cv = {0};
	UINT32 in = OAL_ARGS_QUERY_MODEM_CFG;

	HANDLE hProxy = CreateFile(L"PXY1:", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hProxy != INVALID_HANDLE_VALUE)
	{
		if (DeviceIoControl(hProxy, IOCTL_HAL_QUERY_CARD_CFG, &in, sizeof(in), &cv, sizeof(cv), 0, 0))
		{
			if (cv.ver != -1 && cv.rev != -1 && cv.config == 'A')
				isEchoCancellerExists = TRUE;
		}

		CloseHandle(hProxy);
	}
	
	RETAILMSG (1,(TEXT("isEchoCancellerExists = %d\r\n"), isEchoCancellerExists));
	RETAILMSG (1,(TEXT("GPRS modem ver.%d.%d%c\r\n"), cv.ver, cv.rev, (UINT8)cv.config));

	if (isEchoCancellerExists)	// configure the ForteMedia chip
	{
		if (!sendATCmd(hCom, "AT#GPIO=9,0,1\r", respStr))	// BP to high
		{
			RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#GPIO=9,0,1 failed\r\n")));
			return(GSM_ERROR_AT_CMD);
		}
		if (!sendATCmd(hCom, "AT#GPIO=7,1,1\r", respStr))	// RST to low
		{
			RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#GPIO=7,1,1 failed\r\n")));
			return(GSM_ERROR_AT_CMD);
		}
		if (!sendATCmd(hCom, "AT#GPIO=6,0,1\r", respStr))	// PWDN to high
		{
			RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#GPIO=6,0,1 failed\r\n")));
			return(GSM_ERROR_AT_CMD);
		}
		if (!sendATCmd(hCom, "AT#GPIO=7,0\r", respStr))		// RST to high
		{
			RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#GPIO=7,0 failed\r\n")));
			return(GSM_ERROR_AT_CMD);
		}
	}
	else
	{
	 // enables echo canceller for handsfree mode
	 if( !sendATCmd(hCom, "AT#SHFEC=1\r\n", respStr) )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#SHFEC failed\r\n")));
		return(GSM_ERROR_AT_CMD);
	  }

	 // enables Noise Reduction
	 if( !sendATCmd(hCom, "AT#SHFNR=1\r\n", respStr) )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#SHFNR failed\r\n")));
		return(GSM_ERROR_AT_CMD);
	  }

	 if( !sendATCmd(hCom, "AT#SHSNR=1\r\n", respStr) )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#SHFNR failed\r\n")));
		return(GSM_ERROR_AT_CMD);
	  }

	 // enables echo canceller for handset mode
	 if( !sendATCmd(hCom, "AT#SHSEC=1\r\n", respStr) )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#SHSEC failed\r\n")));
		return(GSM_ERROR_AT_CMD);
	  }

	 //enables automatic gain control for handsfree mode
	 if( !sendATCmd(hCom, "AT#SHFAGC=1\r\n", respStr) )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#SHFAGC failed\r\n")));
		return(GSM_ERROR_AT_CMD);
	  }

	 //enables automatic gain control for handset mode
	 if( !sendATCmd(hCom, "AT#SHSAGC=1\r\n", respStr) )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoice Error: AT#SHSAGC failed\r\n")));
		return(GSM_ERROR_AT_CMD);
	  }
	}
	 //GSM_DBG_PRN
	 //Show current Audio codec mode
#if 0
	 if( !sendATCmd(hCom, "AT#CODEC?\r\n", respStr) )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoice Error:AT#CODEC? failed\r\n")));
		return(GSM_ERROR_AT_CMD);
	  }
	 else
	 {
      RETAILMSG (GSM_DBG_PRN,(TEXT("MIC_GSMVoice: current audio codec mode = %S\r\n"), respStr));
	 }

	 //Show the range of available values
	 if( !sendATCmd(hCom, "AT#CODEC=?\r\n", respStr) )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoice Error:AT#CODEC=? failed\r\n")));
		return(GSM_ERROR_AT_CMD);
	  }
	 else
	 {
      RETAILMSG (GSM_DBG_PRN,(TEXT("MIC_GSMVoice:available audio codec modes = %S\r\n"), respStr));  
	 }
#endif

	 mode = 0;
	 stat = 0;
	 timeout = 0;

	 RETAILMSG(1, (L"GSM API:%S, Wait for network registration ",__FUNCTION__));

	 do
	 {
		 // Wait for network registration
		 if( !sendATCmd(hCom, "at+creg?\r\n", respStr) )
		 {
			RETAILMSG (GSM_DBG_PRN,(TEXT("MIC_GSMVoice Error:at+creg? failed\r\n")));
			return(GSM_ERROR_AT_CMD);
		  }
		 else
		 {
		  //RETAILMSG (GSM_DBG_PRN,(TEXT("MIC_GSMVoice: network searching state = %S\r\n"), respStr));
		  RETAILMSG(1, (L"."));
		 }

		 Sleep(1000);
		 timeout++;

		 // "+CREG: 0,2"
		 // Search for 'mode' and 'stat'
		 pCREGStr = strstr(respStr, "+CREG: ");
         if( pCREGStr != NULL )
		 {
		  mode = pCREGStr[7] - '0';
		  stat = pCREGStr[9] - '0';
		  RETAILMSG (GSM_DBG_PRN,(TEXT("MIC_GSMVoice: mode = %d, stat = %d\r\n"), mode, stat));
		 }
		 else
          break;

         if( timeout > 60 )
          break;

	 } while( (stat != 1)&&(stat !=5) );

	 if( (stat == 1)||(stat == 5) )
       RETAILMSG (1,(TEXT("\r\nMIC_GSMVoice: network is Ok, stat = %d\r\n"), stat));
	 else
	 {
       RETAILMSG (1,(TEXT("\r\nMIC_GSMVoice: network not found, stat = %d\r\n"), stat));
	   return(GSM_REGISTRATION_ERROR);
	 }

 }

 
 RETAILMSG(GSM_DBG_PRN, (L"GSM API:-%S\r\n",__FUNCTION__));
 return(GSM_OK);
}



//------------------------------------------------------------------------------
// Function name	: MIC_GSMVoiceCMD
// Description	    : This function implements gsm modem voice commands
// Return type		: 0 if succeeded, error number if failed 
// Argument 1       : GSM_VOICE_CMD voice command
// Argument 2       : string in case of DIAL command
// Notes            : Modem should be On when this function is called.
//                    This function doesn't check the Modem response.
//------------------------------------------------------------------------------
INT32  MIC_GSMVoiceCMD(HANDLE  hCom, GSM_VOICE_CMD voiceCMD, const char *pCmdStr)
{
  char   atCmd[MAX_SIZE_TO_READ];
  //char   respStr[MAX_SIZE_TO_READ];


  RETAILMSG(GSM_DBG_PRN, (L"GSM API:+%S, voiceCMD = %d, CmdStr = %S\r\n",__FUNCTION__, voiceCMD, pCmdStr));

  if( hCom == INVALID_HANDLE_VALUE )
  {
   RETAILMSG (1,(TEXT("MIC_GSMVoiceCMD Error: Invalid Handle\r\n")));
   return(GSM_ERROR_INVALID_PARAMETER);
  }

  switch(voiceCMD)
  {
   case GSM_VOICE_DIAL:
	{
	 // Check the validity of cmdStr
	 if( pCmdStr == NULL )
	 {
	  return(GSM_ERROR_INVALID_PARAMETER);
	 }

	 // Check the length of cmdStr
	 if( strnlen(pCmdStr, MAX_SIZE_TO_READ) == MAX_SIZE_TO_READ )
	 {
	  return(GSM_ERROR_INVALID_PARAMETER);
	 }
      
	// strcpy_s(atCmd, MAX_SIZE_TO_READ, "ATD");
	 strcpy(atCmd, "ATD");
	 //if( strcat_s(atCmd, MAX_SIZE_TO_READ, pCmdStr) )
	 // return(GSM_ERROR_INVALID_PARAMETER);

	 strcat(atCmd, pCmdStr);

	 //if( strcat_s(atCmd, MAX_SIZE_TO_READ, ";\r\n") )
	 // return(GSM_ERROR_INVALID_PARAMETER);

	 strcat(atCmd, ";\r\n");

	 RETAILMSG(GSM_DBG_PRN, (L"GSM API:%S, dial string = %S\r\n",__FUNCTION__, atCmd));

     if( !sendATCmdNoResp(hCom, atCmd) )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoiceCMD DIAL Error: %S failed\r\n"), atCmd));
		return(GSM_ERROR_AT_CMD);
	  }


	} break;

   case GSM_VOICE_HANG_UP:
	{
	 // Hang Up "ATH\r\n"
     RETAILMSG(GSM_DBG_PRN, (L"GSM API:%S Hang Up\r\n",__FUNCTION__));
     if( !sendATCmdNoResp(hCom, "ATH\r\n") )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoiceCMD Hang Up Error\r\n")));
		return(GSM_ERROR_AT_CMD);
	  }

	} break;

   case	GSM_VOICE_ANSWER:
	{
	 // Answer "ATA\r\n"
     RETAILMSG(GSM_DBG_PRN, (L"GSM API:%S Answer\r\n",__FUNCTION__));
     if( !sendATCmdNoResp(hCom, "ATA\r\n") )
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoiceCMD Answer Error\r\n")));
		return(GSM_ERROR_AT_CMD);
	  }

	} break;

   default:
	  {
		RETAILMSG (1,(TEXT("MIC_GSMVoiceCMD Error: Invalid 1st parameter = %d\r\n"), voiceCMD ));
		return(GSM_ERROR_INVALID_PARAMETER);
	  } break;
  }

  RETAILMSG(GSM_DBG_PRN, (L"GSM API:-%S\r\n",__FUNCTION__));
  return(0);
}



//------------------------------------------------------------------------------
// Function name	: MIC_GSMConfig
// Description	    : This function configures the gsm modem.
// Return type		: 0 if succeeded, error number if failed 
// Argument         : 
//------------------------------------------------------------------------------
UINT32  MIC_GSMConfig(HANDLE hCom, UINT32 flag)
{

	UINT32 rc = GSM_OK;
  
#if 0
	RETAILMSG(GSM_DBG_PRN, (L"GSM API:+%S, flag = %d\r\n",__FUNCTION__, flag));
	// Open the serial port.
	HANDLE hPort = CreateFile(GPRS_IDENTIFIER, 0, 0, NULL, 0, 0, NULL);

	if (hPort == INVALID_HANDLE_VALUE)
	{
		RETAILMSG (1,(TEXT("MIC_GSMPower. Error: Open Modem handle failed error = %d \r\n"),GetLastError ()));
		return GSM_ERROR_COMPORT;
	}
#endif
	
	BOOL ret = DeviceIoControl(hCom, IOCTL_SET_MODEM_CONFIG, &flag,
	        sizeof(flag), 0, 0, NULL, NULL
	        );
	if(!ret)
		rc = GSM_DRIVER_ERROR;

#if 0
	CloseHandle(hPort);
#endif	
 
	RETAILMSG(GSM_DBG_PRN, (L"GSM API:-%S, rc = %d\r\n",__FUNCTION__, rc));
    return rc;

}


