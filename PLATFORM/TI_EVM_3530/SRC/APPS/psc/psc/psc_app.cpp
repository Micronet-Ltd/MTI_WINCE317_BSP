// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
// Created by Anna Ryer
//
#include <windows.h>
#include <ceddkex.h>
#include <winbase.h>
#include <pm.h>
#include <pmpolicy.h>
//#include <msgqueue.h>
//#include <pnp.h>

#include <MicUserSdk.h>
#include <pkfuncs.h>
#include <pmimpl.h>
#include <pwsdef.h>
#include <bsp.h>
#include "..\\..\\IgnitionCpl\\IgnitionCpl.h"
#include "..\\..\\..\\drivers\\aio\\aio.h"

#define PSC_DEBUG 1

#define PWRMGR_PCS_KEY             PWRMGR_REG_KEY TEXT("\\PSC")
#define PWRMGR_TIMOUTS_KEY         PWRMGR_REG_KEY TEXT("\\Timeouts")
#define STATE_CHANGE_NOTIFY        TEXT("\0\0")
#define QUEUE_ENTRIES              10
#define MAX_NAMELEN                200
#define QUEUE_SIZE                (QUEUE_ENTRIES * (sizeof(POWER_BROADCAST) + MAX_NAMELEN))

//=========================================================

// local functions
//=========================================================


DWORD ReadPCSReg();
DWORD InitPSC();
DWORD DeinitPSC();
DWORD InitPowerSources();
DWORD DeInitPowerSources();
DWORD InputThread(VOID *pInfo);
DWORD RunPreventThread();
DWORD StopPreventThread();
DWORD PreventThread(VOID *);
DWORD PowerSourceChangeNotify(BOOL);
DWORD CalculatePowerSource();
DWORD InitPowerNotifications();
DWORD DeInitPowerNotifications();
void ProcessPowerNotification(HANDLE hMsgQ);
DWORD PowerThread(LPVOID pvParam);
DWORD GetInputsInfo();
DWORD CalculateCarAccState();


//=========================================================
// local defines
//=========================================================

//#define NUMBER_POWER_SOURCES 2
#define PREVENT_THREAD_CLOSE_TIMEOUT 5000
#define INPUT_THREAD_CLOSE_TIMEOUT 2000
#define POWER_THREAD_CLOSE_TIMEOUT 1000


//=========================================================
// local structures
//=========================================================
typedef enum {
	INVALID_TYPE = 0,
    IGNITION,
    EXT_POWER_SOURCE,
	CRADLE
} POWER_SOURCE_TYPE;

typedef struct _PCS_CONTEXT
{
	BOOL                ignSwtControlEnable;
	UINT32              ignSwtStableTime;
	UINT32              extSourceStableTime;
	UINT32              cradleStableTime;
	UINT32              performAction;
	UINT32              performActionDelay;
	UINT32              preventPollTime;
	TCHAR               szvpowerSource[256];
//	HANDLE              hGpio;
	HANDLE              hAIO;
	BYTE                ioBuffer[OPEN_BUFFER_LENGTH];
	HANDLE              hInputThread[3];
	HANDLE              hPreventThread;
	HANDLE              hAbortEvent;
	UINT32              ignitionState; // input[0] - ignition switch; input[1] - charging (on/out)
	UINT32              extSourceState;
	UINT32              cradleState;
	UINT32              powerSource;
	UINT32              carAcc;
	HANDLE              powerSourceChangeEvent;
	BOOL                preventQuit;
	HANDLE              hPowerNotifications;
	HANDLE              hPowerNotificationsQueue;
	HANDLE              hPowerThread;
	HANDLE				hTakePS;
	//DWORD               priority256;
	UINT32				shutDownAbsolute;
	UINT32				analogInputCheck;
} PCS_CONTEXT, *PPCS_CONTEXT;

typedef struct _PM_CONTEXT
{
	DWORD               ACUserIdle;
	DWORD               ACSystemIdle;
	DWORD               ACSuspend;
	DWORD               battUserIdle;
	DWORD               battSystemIdle;
	DWORD               battSuspend;
}PM_CONTEXT, *PPM_CONTEXT;


typedef struct _POWER_SOURCE_INFO{

	UINT32                  inputNo;
	POWER_SOURCE_TYPE       sourceType;           

}POWER_SOURCE_INFO, *PPOWER_SOURCE_INFO;


//=========================================================
// global variables
//=========================================================

PCS_CONTEXT g_pcsContext;
//PM_CONTEXT  g_pmContext;

static const DEVICE_REGISTRY_PARAM s_pcsInitRegParams[] = {
    {
        L"IgnitionSwitchControlEnable", PARAM_DWORD, FALSE, offset(PCS_CONTEXT, ignSwtControlEnable),
        fieldsize(PCS_CONTEXT, ignSwtControlEnable), (VOID*)0
    },
	{
        L"IgnStableTime", PARAM_DWORD, FALSE, offset(PCS_CONTEXT, ignSwtStableTime),
        fieldsize(PCS_CONTEXT, ignSwtStableTime), (VOID*)3
    },
	{
        L"ExtSourceStableTime", PARAM_DWORD, FALSE, offset(PCS_CONTEXT, extSourceStableTime),
        fieldsize(PCS_CONTEXT, extSourceStableTime), (VOID*)1
    },
	{
        L"CradleStableTime", PARAM_DWORD, FALSE, offset(PCS_CONTEXT, cradleStableTime),
        fieldsize(PCS_CONTEXT, cradleStableTime), (VOID*)1
    },
	{
        L"PerformAction", PARAM_DWORD, FALSE, offset(PCS_CONTEXT, performAction),
        fieldsize(PCS_CONTEXT, performAction), (VOID*)0
    },
	{
        L"PerformActionDelay", PARAM_DWORD, FALSE, offset(PCS_CONTEXT, performActionDelay),
        fieldsize(PCS_CONTEXT, performActionDelay), (VOID*)5
    },
	{
        L"PreventPollTime", PARAM_DWORD, FALSE, offset(PCS_CONTEXT, preventPollTime),
        fieldsize(PCS_CONTEXT, preventPollTime), (VOID*)1
    },

	{
        L"PowerSourceChange", PARAM_STRING, FALSE, offset(PCS_CONTEXT, szvpowerSource),
        fieldsize(PCS_CONTEXT, szvpowerSource), (VOID*)STATE_CHANGE_NOTIFY
    },

	{
        L"ShutDownAbsolute", PARAM_DWORD, FALSE, offset(PCS_CONTEXT, shutDownAbsolute),
        fieldsize(PCS_CONTEXT, shutDownAbsolute), (VOID*)0
    },
	{
        L"AnalogInputCheck", PARAM_DWORD, FALSE, offset(PCS_CONTEXT, analogInputCheck),
        fieldsize(PCS_CONTEXT, analogInputCheck), (VOID*)0
    }
};


static const DEVICE_REGISTRY_PARAM s_pmRegParams[] = {
	{
        L"ACUserIdle", PARAM_DWORD, FALSE, offset(PM_CONTEXT, ACUserIdle),
        fieldsize(PM_CONTEXT, ACUserIdle), (VOID*)0
    },
    {
        L"ACSystemIdle", PARAM_DWORD, FALSE, offset(PM_CONTEXT, ACSystemIdle),
        fieldsize(PM_CONTEXT, ACSystemIdle), (VOID*)0
    },
	{
        L"ACSuspend", PARAM_DWORD, FALSE, offset(PM_CONTEXT, ACSuspend),
        fieldsize(PM_CONTEXT, ACSuspend), (VOID*)0
    },
	{
        L"BattUserIdle", PARAM_DWORD, FALSE, offset(PM_CONTEXT, battUserIdle),
        fieldsize(PM_CONTEXT, battUserIdle), (VOID*)0
    },
	{
        L"BattSystemIdle", PARAM_DWORD, FALSE, offset(PM_CONTEXT, battSystemIdle),
        fieldsize(PM_CONTEXT, battSystemIdle), (VOID*)0
    },
	{
        L"BattSuspend", PARAM_DWORD, FALSE, offset(PM_CONTEXT, battSuspend),
        fieldsize(PM_CONTEXT, battSuspend), (VOID*)0
    }

};

static const POWER_SOURCE_INFO s_powerSourceInfo[] = {

	 { 0,   (POWER_SOURCE_TYPE)IGNITION},
};

//UINT32 GetShutDownAbsolute()
//{
//	UINT32				shutDownAbsolute = 0;	 
//	UINT32				number			= 0;
//	manufacture_block	mb				= {0};
//	char				str[2]			= {0};
//	char*				endptr;
//
//	HalQueryParamBlock( 0, 0, (void*)&mb, sizeof(mb), &number );
//	str[0] = mb.device_identification[CONF_BYTE];
//	number = strtol(str, &endptr, 16);
//	//bit 3 of byte 18 in catalog number 	
//	if( (0xFF				!= number) &&	//not exists
//		((1 << BIT_ENABLE)	&  number) &&
//		(MASK_EXISTS		&  number) )
//	{
//		shutDownAbsolute = 1;
//	}
//	RETAILMSG(1, (L"PSC: 0x%S - 0x%x - %d\r\n", str, number, shutDownAbsolute));
//	return shutDownAbsolute;
//}
//------------------------------------------------------------------------------
// Function name	: InitPSC
// Description	    : Denitialization of all used resources
// Return type		: DWORD 
// Argument         : NONE
//------------------------------------------------------------------------------
DWORD InitPSC()
{
	DWORD rc = 0;

	RETAILMSG(PSC_DEBUG, (L" + InitPSC\r\n"));
	memset (&g_pcsContext,0,sizeof(g_pcsContext));

	if (!ReadPCSReg())
	{ 
		RETAILMSG(1, (L"ERROR: InitPSCApps: ReadPCSReg failed\r\n"));
		return 0;	
	}
	RETAILMSG(1, (L"InitPSCApps: ReadPCSReg shutDownAbsolute %d\r\n", g_pcsContext.shutDownAbsolute));

	g_pcsContext.ignitionState  = INVALID_STATE;
    g_pcsContext.extSourceState = INVALID_STATE;
	g_pcsContext.cradleState    = INVALID_STATE;
	g_pcsContext.powerSource    = AC_LINE_UNKNOWN;

	g_pcsContext.hAbortEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (g_pcsContext.hAbortEvent == NULL)
	{
		RETAILMSG(1, (L"ERROR: InitPSCApps: Create Event failed\r\n"));
		return 0;
	}


	if (!InitPowerSources())
	{
		RETAILMSG(1, (L"ERROR: InitPSCApps: InitPowerSources failed\r\n"));
        DeinitPSC();
		return 0;
	}
	g_pcsContext.hAbortEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (g_pcsContext.hAbortEvent == NULL)
	{
		RETAILMSG(1, (L"ERROR: InitPSCApps: InitPowerSources failed\r\n"));
        DeinitPSC();
		return 0;
	}

	g_pcsContext.hTakePS = CreateEvent(0, 0, 0, L"Give Me Power Source");
	g_pcsContext.powerSourceChangeEvent = CreateEvent(NULL, TRUE, FALSE, g_pcsContext.szvpowerSource);
    if ((GetLastError() == ERROR_SUCCESS) || (GetLastError() == ERROR_ALREADY_EXISTS))
    {
        RETAILMSG(PSC_DEBUG, (TEXT("PSC:Power Source Change Event Create\r\n")));
		rc = 1;
    }
    else
    {
        RETAILMSG(1, (TEXT("PSC:Power Source Change Event failure\r\n")));

    }

	//g_pcsContext.shutDownAbsolute = GetShutDownAbsolute();

	RETAILMSG(1, (TEXT("PSC:PSC: shutDownAbsolute = %d\r\n"), g_pcsContext.shutDownAbsolute));
	InitPowerNotifications();

	// send the initial power source value;
	PowerSourceChangeNotify(TRUE);

/*
	if(g_pcsContext.carAcc)
	{
		RunPreventThread();
	}

	// set suspend shutdowm pm state. It means that shutdown will be implemented instead of suspend
	if(g_pcsContext.carAcc && g_pcsContext.performAction == SHUTDOWN)
	{
		PowerPolicyNotify(PPN_SUSPENDSHUTDOWN,1);
	}
*/

	RETAILMSG(PSC_DEBUG, (L" - InitPSC rc = %x \r\n",rc));

	return rc;
	
}




//------------------------------------------------------------------------------
// Function name	: DeinitPSC
// Description	    : DeInitialization of all used resources
// Return type		: DWORD  0 - failure; 1 - success 
// Argument         : NONE
//------------------------------------------------------------------------------
DWORD DeinitPSC()
{
	RETAILMSG(PSC_DEBUG, (L"DeinitPSC started\r\n"));
	DWORD rc = 1;


	if(DeInitPowerSources() == 0)
	{
		rc = 0;
		RETAILMSG(1, (TEXT("PSC:DeinitPSC ERROR: DeInitPowerSources() failed\r\n")));
	}
	if( StopPreventThread() == 0)
	{
		rc = 0;
		RETAILMSG(1, (TEXT("PSC:DeinitPSC ERROR: StopPreventThread() failed\r\n")));
	}
	if(g_pcsContext.powerSourceChangeEvent)
	{
		CloseHandle(g_pcsContext.powerSourceChangeEvent);
		g_pcsContext.powerSourceChangeEvent = NULL;
	}

	DeInitPowerNotifications();

	// release suspend shutdowm pm state. 
	if(g_pcsContext.ignSwtControlEnable && g_pcsContext.performAction == SHUTDOWN)
	{
		PowerPolicyNotify(PPN_SUSPENDSHUTDOWN,0);
	}

	memset (&g_pcsContext,0,sizeof(g_pcsContext));
	return rc;
	
}




//------------------------------------------------------------------------------
// Function name	: ReadPCSReg
// Description	    : Read PCS app registry values
// Return type		: DWORD  0 - failure; 1 - success 
// Argument         : NONE
//------------------------------------------------------------------------------
DWORD ReadPCSReg()
{
	static BOOL fBootPhase2Reopen = FALSE;

    if (!fBootPhase2Reopen) { // We need reopen the key if we migrate to Phase 2.
        HANDLE hevBootPhase2 = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T("SYSTEM/BootPhase2"));
        if (hevBootPhase2) {
            if( WaitForSingleObject(hevBootPhase2, 0) == WAIT_OBJECT_0) { // If Phase2 Signaled.
                
                fBootPhase2Reopen = TRUE;
            }
            CloseHandle(hevBootPhase2);
        }
        else
            fBootPhase2Reopen = TRUE; // If OpenEvent Failed, It meaning there is no Phase2 state. So We set it to by pass the checking.
    }


	// Read device parameters
	
    if(GetDeviceRegistryParams(PWRMGR_PCS_KEY, &g_pcsContext, dimof(s_pcsInitRegParams), s_pcsInitRegParams) != ERROR_SUCCESS)
	{
        RETAILMSG(1, (L"ERROR: ReadPCSReg: Failed read PCS registry parameters\r\n"));
		return 0;
	}

	/*
	g_pcsContext.ignSwtControlEnable = 1;
    g_pcsContext.ignSwtStableTime = 1;
	g_pcsContext.chargerStableTime = 1;
	g_pcsContext.performAction = 0;
	g_pcsContext.performActionDelay = 100;
	*/

	return 1;
}


//------------------------------------------------------------------------------
// Function name	: InitPowerSources
// Description	    : Initial Power sources values and threads
// Return type		: DWORD  0 - failure; 1 - success 
// Argument         : NONE
//------------------------------------------------------------------------------

DWORD InitPowerSources()
{
	DWORD rc  = 0;
	AIO_INPUT_CONTEXT inputContext;
	DWORD stableTime;

	RETAILMSG(PSC_DEBUG, (L"InitPowerSources started.\r\n"));
	
	
	*((DWORD*)g_pcsContext.ioBuffer) = OPEN_BUFFER_LENGTH;

	g_pcsContext.hAIO = MIC_AIOOpen(g_pcsContext.ioBuffer);

	if(g_pcsContext.hAIO==INVALID_HANDLE_VALUE)
	{
		RETAILMSG(1, (L"ERROR: InitIgnitionSources: MIC_AIOOpen failed\r\n"));
		g_pcsContext.hAIO = 0;
		return 0;
	}

	for( int i = 0; i < (sizeof(s_powerSourceInfo)/sizeof(s_powerSourceInfo[0])); i++)
	{
		// we are monitoring ignition switch source just in the case ignition switch control is enabled
		if(s_powerSourceInfo[i].sourceType != IGNITION || g_pcsContext.ignSwtControlEnable)
		{
	
			memset(&inputContext,0,sizeof(inputContext));
			inputContext.size = sizeof(AIO_INPUT_CONTEXT);

			inputContext.pinNo.mask = (0x01 << s_powerSourceInfo[i].inputNo);
			//inputContext.pinNo.AUT_IN_1 = 1;
			//inputContext.mask = (0x01<<(g_pcsContext.ignSwtSource[i]-1));

			if(s_powerSourceInfo[i].sourceType != CRADLE )
			{
				inputContext.mode = LOCKED;

				rc = MIC_AIOSetInputMode (g_pcsContext.hAIO, (LPVOID)&inputContext);

				if (!rc)
				{
					RETAILMSG(1, (L"ERROR: InitIgnitionSources: MIC_AIOSetInputMode failed error - 0x%x\r\n",inputContext.opErrorCode,s_powerSourceInfo[i].inputNo ));
					MIC_AIOClose(g_pcsContext.hAIO, g_pcsContext.ioBuffer);
					g_pcsContext.hAIO = 0;
					return 0;
				}
			}

			// get current source state

			rc = MIC_AIOGetInputInfo (g_pcsContext.hAIO, (LPVOID)&inputContext);

			if(s_powerSourceInfo[i].sourceType == IGNITION)
				g_pcsContext.ignitionState = inputContext.currentState;
			else if(s_powerSourceInfo[i].sourceType == EXT_POWER_SOURCE)
				g_pcsContext.extSourceState = inputContext.currentState;
			else
                g_pcsContext.cradleState = inputContext.currentState;

			// set stability time according to the registry value

			stableTime = (s_powerSourceInfo[i].sourceType == IGNITION)? 
				g_pcsContext.ignSwtStableTime: (s_powerSourceInfo[i].sourceType == EXT_POWER_SOURCE)?
				g_pcsContext.extSourceStableTime:g_pcsContext.cradleStableTime;
			


			//rc = MIC_AIOSetSignalStabilityTime(g_pcsContext.hAIO, (LPVOID)&inputContext,stableTime*1000);
			if(MAX_STABLE_TIME < stableTime)
				stableTime = MAX_STABLE_TIME;

			rc = MIC_AIOSetSignalStabilityTime(g_pcsContext.hAIO, (LPVOID)&inputContext,stableTime);
			if (!rc)
			{
				RETAILMSG(1, (L"ERROR: InitIgnitionSources: MIC_AIOSetSignalStabilityTime failed error - 0x%x - 0x%x \r\n",inputContext.opErrorCode,s_powerSourceInfo[i].inputNo ));
				MIC_AIOClose(g_pcsContext.hAIO, g_pcsContext.ioBuffer);
				g_pcsContext.hAIO = 0;
				return 0;
			}

			rc = MIC_AIORegisterForStateChange(g_pcsContext.hAIO, g_pcsContext.ioBuffer,(LPVOID)&inputContext,TRUE);
			if (!rc)
			{
				RETAILMSG(1, (L"ERROR: InitIgnitionSources: MIC_AIORegisterForStateChange failed error - 0x%x - 0x%x \r\n",inputContext.opErrorCode,s_powerSourceInfo[i].inputNo ));
				MIC_AIOClose(g_pcsContext.hAIO, g_pcsContext.ioBuffer);
				g_pcsContext.hAIO = 0;
				return 0;
			}

			g_pcsContext.hInputThread[i] = CreateThread(0, 0, InputThread, (LPVOID)&s_powerSourceInfo[i], 0, 0);
			if(g_pcsContext.hInputThread[i] == 0)
			{
				RETAILMSG(1, (L"ERROR: InitIgnitionSources: CreateThread failed\r\n",s_powerSourceInfo[i].inputNo));
				MIC_AIOClose(g_pcsContext.hAIO, g_pcsContext.ioBuffer);
				return 0;
			}

			rc = MIC_AIOStartStateChangeNotify(g_pcsContext.hAIO, g_pcsContext.ioBuffer,(LPVOID)&inputContext);
			if (!rc)
			{
				RETAILMSG(1, (L"ERROR: InitIgnitionSources: MIC_AIOStartStateChangeNotify failed error - 0x%x - 0x%x \r\n",inputContext.opErrorCode,s_powerSourceInfo[i].inputNo ));
				DeInitPowerSources();
				return 0;
			}
		}
	}


	

	
	RETAILMSG(PSC_DEBUG, (L"InitPowerSources ended\r\n"));
	
	
	return 1;
}



//------------------------------------------------------------------------------
// Function name	: DeInitPowerSources
// Description	    : DeInitial Power sources values and threads
// Return type		: DWORD  0 - failure; 1 - success 
// Argument         : NONE
//------------------------------------------------------------------------------
DWORD DeInitPowerSources()
{
	DWORD rc;
	DWORD dwReason;
	AIO_INPUT_CONTEXT inputContext;
	
	if(g_pcsContext.ignSwtControlEnable && g_pcsContext.hAIO)
	{
		for( int i = 0; i < (sizeof(s_powerSourceInfo)/sizeof(s_powerSourceInfo[0])); i++)
		{
				// we are monitoring ignition switch source just in the case ignition switch control is enabled
			if(s_powerSourceInfo[i].sourceType != IGNITION || g_pcsContext.ignSwtControlEnable)
			{


				memset(&inputContext,0,sizeof(inputContext));
				inputContext.size = sizeof(AIO_INPUT_CONTEXT);

				inputContext.pinNo.mask = (0x01 << s_powerSourceInfo[i].inputNo);

				//inputContext.pinNo.AUT_IN_1 = 1;
				//inputContext.mask = (0x01<<(g_pcsContext.ignSwtSource[i]-1));

			
				rc = MIC_AIOStopStateChangeNotify(g_pcsContext.hAIO, g_pcsContext.ioBuffer,(LPVOID)&inputContext);
				RETAILMSG(!rc, (L"ERROR: DeInitIgnitionSources: MIC_AIOStopStateChangeNotify failed error - 0x%x\r\n",inputContext.opErrorCode ));

				// it will signal the MIC_AIOWaitForStateChange thread
				rc = MIC_AIORegisterForStateChange(g_pcsContext.hAIO, g_pcsContext.ioBuffer,(LPVOID)&inputContext,FALSE);
				RETAILMSG(!rc, (L"ERROR: DeInitIgnitionSources: MIC_AIORegisterForStateChange failed error - 0x%x\r\n",inputContext.opErrorCode ));

				// terminate automotive input thread

				if (g_pcsContext.hInputThread[i])
				{

					//SetEvent(g_pcsContext.hAbortEvent);
					dwReason = WaitForSingleObject(g_pcsContext.hInputThread, INPUT_THREAD_CLOSE_TIMEOUT);
					if( dwReason != WAIT_OBJECT_0 )
					{
						TerminateThread(g_pcsContext.hInputThread[i], 0);
						RETAILMSG(1, (L"DeInitIgnitionSources: WaitForSingleObject(pData->hNotifyThread) # %d\r\n", dwReason));
					}
					CloseHandle(g_pcsContext.hInputThread[i]);
					g_pcsContext.hInputThread[i] = NULL;
				}

			
				inputContext.mode = FREE;

				rc = MIC_AIOSetInputMode (g_pcsContext.hAIO, (LPVOID)&inputContext);
				RETAILMSG(!rc, (L"ERROR: DeInitIgnitionSources: MIC_AIOSetInputMode failed error - 0x%x\r\n",inputContext.opErrorCode ));
			}
		}

		rc = MIC_AIOClose(g_pcsContext.hAIO, g_pcsContext.ioBuffer);
		g_pcsContext.hAIO = NULL;
	}

	return rc;
}


//------------------------------------------------------------------------------
// Function name	: InputThread
// Description	    : Thread to monitor power source value
// Return type		: DWORD  0 - failure; 1 - success 
// Argument         : PPOWER_SOURCE_INFO
//------------------------------------------------------------------------------

DWORD InputThread(VOID *pInfo)
{

	AIO_INPUT_CONTEXT  inputContext;

	PPOWER_SOURCE_INFO pInputInfo = (PPOWER_SOURCE_INFO)pInfo;

	UINT32             currentState;
	BOOL               quit = FALSE;
	DWORD              rc;
	BOOL               OnOff;

	memset(&inputContext,0,sizeof(inputContext));
	inputContext.size = sizeof(AIO_INPUT_CONTEXT);

	inputContext.pinNo.mask = (0x01 << pInputInfo->inputNo);
	RETAILMSG(0, (L"InputThread started for input pin =  - 0x%x\r\n",pInputInfo->inputNo ));
	
	if(g_pcsContext.ignSwtControlEnable && IGNITION == pInputInfo->sourceType && LOW == g_pcsContext.ignitionState)
	{
		if(g_pcsContext.performAction == SHUTDOWN || g_pcsContext.shutDownAbsolute)
			PowerPolicyNotify(PPN_SUSPENDSHUTDOWN,1);
		RunPreventThread();
	}
	//inputContext.pinNo.AUT_IN_1 = 1;

	if(g_pcsContext.hAIO)
	{
		while(!quit)
		{	
			rc = MIC_AIOWaitForStateChange(g_pcsContext.hAIO, g_pcsContext.ioBuffer,(LPVOID)&inputContext);
			RETAILMSG(!rc, (L"ERROR: AIOThread: MIC_AIOWaitForStateChange failed error - 0x%x\r\n",inputContext.opErrorCode ));
			if(!rc)
			{
				if(inputContext.opErrorCode == AIO_ERROR_SIGNAL_NOT_STABLE)
				{
					RETAILMSG (1,(TEXT("PSC:InputThread: AIO_ERROR_SIGNAL_NOT_STABLE ... thread Id = 0x%x  rc = 0x%x, error = 0x%x,cs = 0x%x,ps = 0x%x ,time = 0x%x\r\n"),GetCurrentThreadId(),rc,inputContext.opErrorCode,
					inputContext.currentState, inputContext.prevState, GetTickCount()));

					// Anna - need to be decided what to do here
					rc = MIC_AIOStartStateChangeNotify(g_pcsContext.hAIO, g_pcsContext.ioBuffer,(LPVOID)&inputContext);
				}
				else
				{
					RETAILMSG (1,(TEXT("PSC:AIOThread: quiting ... rc = 0x%x, error = 0x%x,cs = 0x%x,ps = 0x%x ,time = 0x%x\r\n"),rc,inputContext.opErrorCode,
					inputContext.currentState, inputContext.prevState, GetTickCount()));
					quit = TRUE;
				}
				
			}
			else
			{
				currentState =  (pInputInfo->sourceType == IGNITION)? g_pcsContext.ignitionState:
					(pInputInfo->sourceType == EXT_POWER_SOURCE)?g_pcsContext.extSourceState:g_pcsContext.cradleState;
				
				
				// we apparently missed any change state
				if(currentState == inputContext.currentState)
				{
					RETAILMSG (1,(TEXT("PSC: InputThread: AIOThread: switch state change missed... thread Id = 0x%x  rc = 0x%x, error = 0x%x,cs = 0x%x,ps = 0x%x ,time = 0x%x\r\n"),GetCurrentThreadId(),rc,inputContext.opErrorCode,
					inputContext.currentState, inputContext.prevState, GetTickCount()));
				}
				else
				{
					OnOff = FALSE;
					if(pInputInfo->sourceType == IGNITION)
					{
						OnOff = (g_pcsContext.ignitionState == HIGH && inputContext.currentState == LOW)? TRUE:FALSE;
						g_pcsContext.ignitionState = inputContext.currentState;
						RETAILMSG(0, (L"PSC: Ignition switch has been updated, new state is 0x%x\r\n",g_pcsContext.ignitionState));
					}
					else if(pInputInfo->sourceType == EXT_POWER_SOURCE)
					{
						g_pcsContext.extSourceState = inputContext.currentState;
						RETAILMSG(0, (L"PSC: extSourceState state has been updated, new state is 0x%x\r\n",g_pcsContext.extSourceState));
					}
					else
					{
						g_pcsContext.cradleState = inputContext.currentState;
						RETAILMSG(0, (L"PSC: cradle state has been updated, new state is 0x%x\r\n",inputContext.currentState));
					}

					PowerSourceChangeNotify(TRUE);
			



					//
					// ce-300: new ignition switch state is changed from on to off and ignition switch control is enabled
					if(OnOff && g_pcsContext.ignSwtControlEnable && g_pcsContext.ignitionState == LOW)
					{
						// let to Power Manager know to enter shutdown state instead of suspend
						if(g_pcsContext.performAction == SHUTDOWN || g_pcsContext.shutDownAbsolute)
							PowerPolicyNotify(PPN_SUSPENDSHUTDOWN,1);
						RunPreventThread();
					}
					else if (!(g_pcsContext.ignSwtControlEnable && g_pcsContext.ignitionState == LOW))
					{
						PowerPolicyNotify(PPN_SUSPENDSHUTDOWN,0);
						StopPreventThread();
					}

				}

			}


		}

	}

	return 1;
}


//------------------------------------------------------------------------------
// Function name	: RunPreventThread
// Description	    : Start Prevent thread
// Return type		: DWORD  0 - failure; 1 - success 
// Argument         : NONE
//------------------------------------------------------------------------------

DWORD RunPreventThread()
{
	g_pcsContext.preventQuit = FALSE;

	if(g_pcsContext.hPreventThread ==NULL)
	{
		g_pcsContext.hPreventThread = CreateThread(0, 0, PreventThread, 0, 0, 0);
		if(g_pcsContext.hPreventThread == 0)
		{
			RETAILMSG(1, (L"ERROR: RunPreventThread: CreateThread failed 0x%x\r\n",GetLastError()));
			return 0;
		}

	}
	return 1;

}

//------------------------------------------------------------------------------
// Function name	: StopPreventThread
// Description	    : Stop Prevent thread
// Return type		: DWORD  0 - failure; 1 - success 
// Argument         : NONE
//------------------------------------------------------------------------------

DWORD StopPreventThread()
{
	DWORD dwReason;

	if (g_pcsContext.hPreventThread)
	{

		g_pcsContext.preventQuit = TRUE;

		dwReason = WaitForSingleObject(g_pcsContext.hPreventThread, PREVENT_THREAD_CLOSE_TIMEOUT);
		if( dwReason != WAIT_OBJECT_0 )
		{
			TerminateThread(g_pcsContext.hPreventThread, 0);
			RETAILMSG(1, (L"StopPreventThread: WaitForSingleObject(pData->hNotifyThread) # %d\r\n", dwReason));
		}
		CloseHandle(g_pcsContext.hPreventThread);
		g_pcsContext.hPreventThread = NULL;
	}

	return 1;

}


//------------------------------------------------------------------------------
// Function name	: PreventThread
// Description	    : Thread to prevent device from entering suspend state
// Return type		: DWORD  0 - failure; 1 - success 
// Argument         : PPOWER_SOURCE_INFO
//------------------------------------------------------------------------------
#define	MAX_AI_VOLTS		8000
#define	MIN_AI_VOLTS		6000

DWORD PreventThread(VOID *)
{
	// read from registry battery system idle and suspend timeout values

	PM_CONTEXT  pmContext;
	DWORD userIdleTimeout,systemIdleTimeout, suspendTimeout;
	DWORD timeout;
	HANDLE hevSystemActivityTimer = NULL;
	DWORD endTime;

	DWORD	ret				= 0;	
	UINT16	ainVoltage[2]	= {0};
	HANDLE	hA2D			= 0;
	BOOL	fRestartTimeout	= 0;

	// read registry
	if(GetDeviceRegistryParams(PWRMGR_TIMOUTS_KEY, &pmContext, dimof(s_pmRegParams), s_pmRegParams) != ERROR_SUCCESS)
	{
        RETAILMSG(1, (L"ERROR: PreventThread: Failed read PM registry parameters\r\n"));
		return 0;
	}

	RETAILMSG(0, (L"RunPreventThread: started %d, shutDownAbsolute %d, analogInputCheck %d\r\n",GetTickCount(), g_pcsContext.shutDownAbsolute, g_pcsContext.analogInputCheck));
	userIdleTimeout   = (g_pcsContext.powerSource == AC_LINE_ONLINE)? pmContext.ACUserIdle  :pmContext.battUserIdle;
	systemIdleTimeout = (g_pcsContext.powerSource == AC_LINE_ONLINE)? pmContext.ACSystemIdle:pmContext.battSystemIdle;
	suspendTimeout    = (g_pcsContext.powerSource == AC_LINE_ONLINE)? pmContext.ACSuspend   :pmContext.battSuspend;


	hevSystemActivityTimer = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T("PowerManager/ActivityTimer/SystemActivity"));

	if(g_pcsContext.analogInputCheck)
	{
		hA2D			= MIC_AINOpen();
		RETAILMSG((0 == hA2D), (L"PreventThread: MIC_AINOpen failed. err %d", GetLastError()));
		if(hA2D)
			fRestartTimeout	= 1;
	}

	if(g_pcsContext.shutDownAbsolute || g_pcsContext.analogInputCheck ||
		((g_pcsContext.performActionDelay > (systemIdleTimeout + suspendTimeout)) && 
		(userIdleTimeout !=0 && systemIdleTimeout != 0 && suspendTimeout!= 0)))
	{
		timeout = g_pcsContext.preventPollTime;
		endTime = g_pcsContext.performActionDelay;

		if(0 == g_pcsContext.shutDownAbsolute && 0 == g_pcsContext.analogInputCheck)
			endTime -= (systemIdleTimeout + suspendTimeout);
		
		endTime = GetTickCount() + endTime * 1000;

		while(!g_pcsContext.preventQuit)
		{
			if(!fRestartTimeout && GetTickCount() > endTime)
				break;

			if(g_pcsContext.analogInputCheck && hA2D)
			{
				ainVoltage[0] = sizeof(ainVoltage);
				ainVoltage[1] = 0;

				ret = MIC_AINmVRead( hA2D, ainVoltage);
				
				RETAILMSG(0, (L"PreventThread: %d mV, ret = %d\r\n", ainVoltage[1], ret));

				if(MAX_AI_VOLTS < ainVoltage[1])//update wait timeout
				{
					RETAILMSG(0, (L"PreventThread: Stop timer\r\n"));
					fRestartTimeout = 1;
				}
				else if(MIN_AI_VOLTS > ainVoltage[1] && fRestartTimeout)
				{
					RETAILMSG(0, (L"PreventThread: Update timer\r\n"));
					endTime = g_pcsContext.performActionDelay * 1000 + GetTickCount();
					fRestartTimeout = 0;
				}
			}
		
			// Reset Activity Timeout to prevent entering system idle state
			if (hevSystemActivityTimer) 
			{
				SetEvent(hevSystemActivityTimer);
			}

			Sleep(timeout);
		}
	}
	if(hA2D)
		MIC_AINClose(hA2D);

	if(hevSystemActivityTimer)
		CloseHandle(hevSystemActivityTimer);

	if(!g_pcsContext.preventQuit && g_pcsContext.shutDownAbsolute)
		SetSystemPowerState(0, POWER_STATE_OFF, POWER_FORCE);

	RETAILMSG(0, (L"RunPreventThread: ended %d\r\n",GetTickCount()));
	return 1;
}



//------------------------------------------------------------------------------
// Function name	: PowerSourceChangeNotify
// Description	    : Calculate new power source state and car acc state 
//                  : and notify about it if changed
// Return type		: DWORD  0 - failure; 1 - success 
// Argument         : NONE
//------------------------------------------------------------------------------
DWORD PowerSourceChangeNotify(BOOL notify)
{
	SYSTEM_POWER_STATUS_EX2 psex2 = {0};

	DWORD newValue = CalculatePowerSource();
	BOOL  switchedOFF = FALSE; // TRUE just if ignition switch control is enabled and ignition switch is turned on->off.

	RETAILMSG(1, (L"PowerSourceChangeNotify oldValue power = %d,newValue = %d %d\r\n", g_pcsContext.powerSource, newValue, GetTickCount()));

	if(newValue != g_pcsContext.powerSource  && g_pcsContext.powerSourceChangeEvent)
	{
		if (g_pcsContext.powerSource == AC_LINE_ONLINE && newValue == AC_LINE_OFFLINE && g_pcsContext.ignSwtControlEnable)
			switchedOFF = TRUE;

		g_pcsContext.powerSource = newValue;
		SetEventData(g_pcsContext.powerSourceChangeEvent, newValue);
		SetEvent(g_pcsContext.powerSourceChangeEvent);

		if(notify)
		{
			// cause to schedule in order the battery driver will get the power source change event
			Sleep(0);

			// to cause the battery to update power source status
			GetSystemPowerStatusEx2(&psex2, sizeof(psex2), TRUE);
			PowerPolicyNotify(PPN_POWERCHANGE, 0);
		}

	}
/*
	newValue = CalculateCarAccState();
	RETAILMSG(1, (L"PowerSourceChangeNotify oldValue = %d,newValue = %d \r\n", g_pcsContext.carAcc, newValue));
	if(g_pcsContext.performAction == SHUTDOWN && newValue!=g_pcsContext.carAcc)
	{
		if (newValue && switchedOFF )
			PowerPolicyNotify(PPN_SUSPENDSHUTDOWN,1);
		else
			PowerPolicyNotify(PPN_SUSPENDSHUTDOWN,0);

		g_pcsContext.carAcc = newValue;
	}
*/

	return 1;
}

//------------------------------------------------------------------------------
// Function name	: CalculatePowerSource
// Description	    : Calculate new power source state
// Return type		: DWORD  0 - failure; 1 - success 
// Argument         : power source state
//------------------------------------------------------------------------------
DWORD CalculatePowerSource()
{
	DWORD powerSource = AC_LINE_UNKNOWN;

	if(g_pcsContext.ignSwtControlEnable)
	{
		if(g_pcsContext.ignitionState == LOW)
		{
           powerSource = AC_LINE_OFFLINE;
		}
		else if(g_pcsContext.ignitionState == HIGH)
		{
			powerSource = AC_LINE_ONLINE;
		}
	}
	else
	{
		powerSource = AC_LINE_ONLINE;
	}
	return powerSource;
}


//------------------------------------------------------------------------------
// Function name	: CalculateCarAccState
// Description	    : Calculate new car accumulator state
// Return type		: DWORD  1 - the power source is car's acc; 0 - otherwise 
// Argument         : NONE
//------------------------------------------------------------------------------
DWORD CalculateCarAccState()
{
	DWORD carAcc = -1;
	if(g_pcsContext.ignSwtControlEnable && g_pcsContext.ignitionState == LOW)
	{
		carAcc = 1;
	}
	else
       carAcc = 0;

	return carAcc;
}



//------------------------------------------------------------------------------
// Function name	: InitPowerNotifications
// Description	    : 
// Return type		: 
// Argument         : 
//------------------------------------------------------------------------------

DWORD InitPowerNotifications()
{
    DWORD rc = 0;
	DWORD dwErr;
	MSGQUEUEOPTIONS msgOptions = {0};
	RETAILMSG(TRUE, (TEXT("PSC:InitPowerNotifications: started \r\n")));

	// create a message queue for Power Manager notifications
    msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
    msgOptions.dwFlags = 0;
    msgOptions.dwMaxMessages = QUEUE_ENTRIES;
    msgOptions.cbMaxMessage = sizeof(POWER_BROADCAST) + MAX_NAMELEN;
    msgOptions.bReadAccess = TRUE;

    g_pcsContext.hPowerNotificationsQueue= CreateMsgQueue(NULL, &msgOptions);
    if (!g_pcsContext.hPowerNotificationsQueue) {
        DWORD dwErr = GetLastError();
        RETAILMSG(1, (TEXT("PSC:CreateMessageQueue ERROR:%d\n"), dwErr));
        return 0;
    }

    // request Power notifications
    g_pcsContext.hPowerNotifications = RequestPowerNotifications(g_pcsContext.hPowerNotificationsQueue, PBT_RESUME); // Flags
    if (!g_pcsContext.hPowerNotifications) {
        dwErr = GetLastError();
		RETAILMSG(TRUE, (TEXT("PSC:InitPowerNotifications: RequestPowerNotifications ERROR:%d\n"),dwErr));
        goto cleanUp;
    }

	// create the monitoring thread
    g_pcsContext.hPowerThread = CreateThread(NULL, 0, PowerThread, NULL, 0, NULL);
    if(!g_pcsContext.hPowerThread) {

        dwErr = GetLastError();
		RETAILMSG(TRUE, (TEXT("PSC:InitPowerNotifications: RequestPowerNotifications ERROR:%d\n"),dwErr));
        goto cleanUp;
    }

	rc = 1;

cleanUp:
	if(!rc)
	{
		RETAILMSG(1, (TEXT("PSC:%InitPowerNotifications failed.\r\n")));
		if(g_pcsContext.hPowerNotifications) StopPowerNotifications(g_pcsContext.hPowerNotifications);
		if(g_pcsContext.hPowerNotificationsQueue) CloseMsgQueue(g_pcsContext.hPowerNotificationsQueue);
		g_pcsContext.hPowerNotifications = NULL;
		g_pcsContext.hPowerNotificationsQueue  = NULL;

	}
   return rc;
}




//------------------------------------------------------------------------------
// Function name	: DeInitPowerNotifications
// Description	    : 
// Return type		: 
// Argument         : 
//------------------------------------------------------------------------------

DWORD DeInitPowerNotifications()
{
	DWORD dwReason;

	if(g_pcsContext.hPowerNotifications) 
		StopPowerNotifications(g_pcsContext.hPowerNotifications);
	if(g_pcsContext.hPowerNotificationsQueue) 
		CloseMsgQueue(g_pcsContext.hPowerNotificationsQueue);

	g_pcsContext.hPowerNotifications = NULL;
	g_pcsContext.hPowerNotificationsQueue  = NULL;

	dwReason = WaitForSingleObject(g_pcsContext.hPowerThread, POWER_THREAD_CLOSE_TIMEOUT);
	if( dwReason != WAIT_OBJECT_0 )
	{
		TerminateThread(g_pcsContext.hPowerThread, 0);
		RETAILMSG(1, (L"DeinitPowerNotifications: WaitForSingleObject %d\r\n", dwReason));
	}
	CloseHandle(g_pcsContext.hPowerThread);
	g_pcsContext.hPowerThread = NULL;

	return 1;
}

//------------------------------------------------------------------------------
// Function name	: PowerThread
// Description	    : 
// Return type		: 
// Argument         : 
//------------------------------------------------------------------------------
DWORD PowerThread(LPVOID pvParam)
{
    HANDLE                hEvents[2];
    UNREFERENCED_PARAMETER(pvParam);
	DWORD                 quit = FALSE;
	DWORD                 dwStatus;

    if(!g_pcsContext.hPowerNotificationsQueue || !g_pcsContext.hAbortEvent)
	{
		RETAILMSG(1, (TEXT("PSC:PowerThread failed. g_pcsContext.hPowerNotifications is not initialized yet. \r\n")));
		return 0;
	}

    CeSetThreadPriority(GetCurrentThread(), 220);//THREAD_PRIORITY_ABOVE_NORMAL);

    hEvents[0] = g_pcsContext.hPowerNotificationsQueue;
 //   hEvents[2] = ghevTerminate;
	hEvents[1] = g_pcsContext.hAbortEvent;

    while (!quit) 
    {

        // Block on our message queue.
        // This thread runs when the power manager writes a notification into the queue.
        dwStatus = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
        if(dwStatus == WAIT_OBJECT_0) {
            ProcessPowerNotification(g_pcsContext.hPowerNotificationsQueue);
        } else if(dwStatus == (WAIT_OBJECT_0 + 1)) {    
            RETAILMSG(1, (TEXT("PSC:PowerThread :Termination event signaled\r\n")));
			quit = TRUE;
        } else {
			RETAILMSG(1, (TEXT("PSC:WaitForMultipleObjects returned: %d (error %d)\r\n"),dwStatus, GetLastError()));
            quit = TRUE;
        }
    }

    return 0;
}



//------------------------------------------------------------------------------
// Function name	: ProcessPowerNotification
// Description	    : 
// Return type		: 
// Argument         : 
//------------------------------------------------------------------------------
void ProcessPowerNotification(HANDLE hMsgQ)
{

	SYSTEM_POWER_STATUS_EX2 psex2 = {0};

    union {
        UCHAR buf[QUEUE_SIZE];
        POWER_BROADCAST powerBroadcast;
    } u;

    int iBytesInQueue;
    DWORD dwFlags = 0;

    LPTSTR pszFname = _T("PMMON!ProcessPowerNotification");

    iBytesInQueue = 0;
    memset(u.buf, 0, QUEUE_SIZE);

    if ( !ReadMsgQueue(hMsgQ,
                       u.buf,
                       QUEUE_SIZE,
                       (LPDWORD)&iBytesInQueue,
                       INFINITE,    // Timeout
                       &dwFlags))
    {
        DWORD dwErr = GetLastError();
        RETAILMSG(1, (TEXT("PSC:%s: ReadMsgQueue: ERROR:%d\n"), pszFname, dwErr));
        return;
    } else if(iBytesInQueue >= sizeof(POWER_BROADCAST)) {
        //
        // process power notifications
        //
        PPOWER_BROADCAST pB = &u.powerBroadcast;
        dwFlags = pB->Flags;
        
        RETAILMSG(1, (TEXT("PSC:*** Power Notification @ Tick:%u \r\n"),GetTickCount()));
        switch (pB->Message) 
        {
			/*
        case PBT_TRANSITION:
            RETAILMSG(1, (TEXT("PSC:%s:\tPBT_TRANSITION to system power state: '%s'\n"), pszFname, pB->SystemPowerState));
            RETAILMSG(1, (TEXT("PSC:%s:\t0x%08x "), pszFname, dwFlags));
            {
                struct {
                    DWORD dwFlag;
                    LPCTSTR pszFlagName;
                } sFlagInfo[] = {
                    FLAGINFO(POWER_STATE_ON),
                    FLAGINFO(POWER_STATE_OFF),
                    FLAGINFO(POWER_STATE_CRITICAL),
                    FLAGINFO(POWER_STATE_BOOT),
                    FLAGINFO(POWER_STATE_IDLE),
                    FLAGINFO(POWER_STATE_SUSPEND),
                    FLAGINFO(POWER_STATE_RESET),
                    FLAGINFO(POWER_STATE_PASSWORD),
                };
                int i;

                // display each bit that's in the mask
                for(i = 0; i < dim(sFlagInfo); i++) {
                    if((dwFlags & sFlagInfo[i].dwFlag) != 0) {
                        RETAILMSG(1, (TEXT("PSC:%s "), sFlagInfo[i].pszFlagName));
                        dwFlags &= ~sFlagInfo[i].dwFlag;        // mask off the bit
                    }
                }
                if(dwFlags != 0) {
                    RETAILMSG(1, (TEXT("PSC:***UNKNOWN***")));
                }
            }
            RETAILMSG(1, (TEXT("PSC:\r\n")));
            break;
			*/
            
            case PBT_RESUME:
				{
					RETAILMSG(1, (TEXT("PSC: PBT_RESUME %d\r\n"), GetTickCount()));
	
				DWORD  battery_ok = 0;
				DWORD  retry_number = 0;

				GetInputsInfo();
				PowerSourceChangeNotify(TRUE);
				// if it's AC source we don't need to check
				if(g_pcsContext.powerSource == AC_LINE_ONLINE)
				{
					battery_ok = 1;
					//Sleep(100);
				}
				else
				{
				
					// if battery is low then battery driver will call POWER OF
					// so if we back to here so battery is ok.
					GetSystemPowerStatusEx2(&psex2, sizeof(psex2), TRUE);
					battery_ok = 1;

				}

				RETAILMSG(1, (TEXT("PSC:PBT_RESUME battery_ok = %d, retry_number = %d %d\r\n"),battery_ok,retry_number, GetTickCount()));
				WaitForSingleObject(g_pcsContext.hTakePS, 1000);
				if(battery_ok)
				{
					// workaround for the situation when 
					PowerPolicyNotify(PPN_RESUMETESTEND, 1);
					//PowerPolicyNotify(PPN_RESUMETESTEND, 1);
				}
				else
				{
					PowerPolicyNotify(PPN_RESUMETESTEND, 0);
					//PowerPolicyNotify(PPN_RESUMETESTEND, 0);
				}

                break;
				}
 /*               
            case PBT_POWERSTATUSCHANGE:
                RETAILMSG(1, (TEXT("PSC:%s:\tPBT_POWERSTATUSCHANGE\n"), pszFname));
                break;
                
            case PBT_POWERINFOCHANGE:
                {
                    PPOWER_BROADCAST_POWER_INFO ppbpi = (PPOWER_BROADCAST_POWER_INFO) pB->SystemPowerState;

                    RETAILMSG(1, (TEXT("PSC:%s:\tPBT_POWERINFOCHANGE\n"), pszFname));
                    RETAILMSG(1, (TEXT("PSC:%s:\t\tAC line status %u, battery flag %u, backup flag %u, %d levels\n"), pszFname,
                        ppbpi->bACLineStatus, ppbpi->bBatteryFlag, ppbpi->bBackupBatteryFlag, ppbpi->dwNumLevels));
                    RETAILMSG(1, (TEXT("PSC:%s:\t\tbattery life %d, backup life %d\n"), pszFname,
                        ppbpi->bBatteryLifePercent, ppbpi->bBackupBatteryLifePercent));
                    RETAILMSG(1, (TEXT("PSC:%s:\t\tlifetime 0x%08x, full lifetime 0x%08x\n"), pszFname, 
                        ppbpi->dwBatteryLifeTime, ppbpi->dwBatteryFullLifeTime));
                    RETAILMSG(1, (TEXT("PSC:%s:\t\tbackup lifetime 0x%08x, backup full lifetime 0x%08x\n"), pszFname, 
                        ppbpi->dwBackupBatteryLifeTime, ppbpi->dwBackupBatteryFullLifeTime));
                }
                break;
*/
                
            default:
                RETAILMSG(1, (TEXT("PSC:\tUnknown Message:%x \r\n"), pB->Message));
                break;
        }

    } else {
        RETAILMSG(1, (TEXT("PSC: Received short message: %d bytes\n"), iBytesInQueue));
        return;
    }
}

//------------------------------------------------------------------------------
// Function name	: GetInputsInfo
// Description	    : 
// Return type		: 
// Argument         : 
//------------------------------------------------------------------------------
DWORD GetInputsInfo()
{
	DWORD rc = 0;

	AIO_INPUT_CONTEXT inputContext;

	for( int i = 0; i < (sizeof(s_powerSourceInfo)/sizeof(s_powerSourceInfo[0])); i++)
	{
			// we are monitoring ignition switch source just in the case ignition switch control is enabled
			if(s_powerSourceInfo[i].sourceType != IGNITION || g_pcsContext.ignSwtControlEnable)
			{
		
				memset(&inputContext,0,sizeof(inputContext));
				inputContext.size = sizeof(AIO_INPUT_CONTEXT);

				inputContext.pinNo.mask = (0x01 << s_powerSourceInfo[i].inputNo);
				
				// get current source state

				rc = MIC_AIOGetInputInfo (g_pcsContext.hAIO, (LPVOID)&inputContext);

				if(s_powerSourceInfo[i].sourceType == IGNITION)
					g_pcsContext.ignitionState = inputContext.currentState;
				else if(s_powerSourceInfo[i].sourceType == EXT_POWER_SOURCE)
					g_pcsContext.extSourceState = inputContext.currentState;
				else
					g_pcsContext.cradleState = inputContext.currentState;

			}
	}
	return rc;
}




//-----------------------------------------------------------------------------------
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
int _tmain(int argc, _TCHAR* argv[])
{
	if(!InitPSC())
		return 0;

	WaitForSingleObject(g_pcsContext.hAbortEvent, INFINITE);

	DeinitPSC();


	return 0;
}

