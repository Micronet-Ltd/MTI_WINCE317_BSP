// Copyright 2009 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//   File: OWRDrvr.cpp
//
//   ONE-WIRE Interface Driver
//  
//
//   Created by Michael Streshinsky
//-----------------------------------------------------------------------------/*



#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <initguid.h>
#include <gpio.h>
#include <bus.h>
#include "omap35xx.h"
//#include "oal.h"
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include "oal_io.h"
#include "oal_memory.h"
#include <oal_prcm.h>
#include <owr_api.h>

//#include "..\gpio_isr\gpio_isr.h"
//#include "..\gpio_isr\gpt_isr.h"
#include "owrdrv.h"
#include "iButton.h"
//#include "nkintr.h"


#ifndef SHIP_BUILD

#undef ZONE_ERROR
#undef ZONE_WARN
#undef ZONE_FUNCTION
#undef ZONE_INFO

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INFO           DEBUGZONE(3)

#if DEBUG
//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"AIO", {
        L"Errors",      L"Warnings",    L"Function",    L"Info",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    ZONE_ERROR|ZONE_WARN
};
#endif

#endif




//==============================================================================================
//            Local Definitions
//==============================================================================================


#define  OWR_DEVICE_COOKIE                     'owrS'

#define  OWR_QUEUE_NAME		                     L"OWR queue name"
#define  READ_QUEUE_NAME                           L"R"
#define  WRITE_QUEUE_NAME                          L"W"

#define  OWR_REG_KEY			            	L"Drivers\\BuiltIn\\OWR"
#define  OWR_REG_KEY_iBUTTON	            	L"Drivers\\BuiltIn\\OWR\\iButton"
#define  OWR_REG_KEY_TEMPR_SENS	            	L"Drivers\\BuiltIn\\OWR\\temprSens"
#define  OWR_QUEUE_NAME_LEN	                    64	           		   // bytes

//#pragma optimize("",off)

typedef enum{
	INVALID_STA = -1,
	LO,
	HIG
}NOTIFY_STAT;
//==============================================================================================
//            Local enumerations
//==============================================================================================


//------------------------------------------------------------------------------

typedef enum{
	UNREGISTERED = 0,
	REGISTERED,
    NOTIFY
} NOTIFY_STATE;


typedef enum
{
 OWR_PUBLIC_ACCESS = 0, // when no process with private access, all - public
 OWR_PRIVATE_ACCESS,    // one has exclusive access, processes have denied access
 OWR_DENIED_ACCESS,
    
} OWR_ACCESS_MODE;



BOOL WINAPI DllEntry ( HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved );


//==============================================================================================
//            structure definitions
//==============================================================================================


 //HANDLE                  g_IsrHandle_inp;
 //HANDLE                  g_IsrHandle_gpt;


typedef struct _OWR_INIT_CONTEXT
{
	UINT32              cookie;
	OWR_DEVICE_TYPE     owrDeviceType;
    OMAP_GPIO_ONE_WIRE  omapGpio1Wire;
	HANDLE	            hOwrISTThread;		// Interrupt Service Thread
    HANDLE	            hOwrISTDummyThread;		// Interrupt Service Thread
	BOOL                quitThread;
	DWORD               priority256;
	CRITICAL_SECTION    OpenCS;	        // @field Protects Open Linked List
	UINT32				OpenContexts[32];
} OWR_INIT_CONTEXT, *POWR_INIT_CONTEXT;



//------------------------------------------------------------------------------
typedef struct __OWR_OPEN_CONTEXT {

    POWR_INIT_CONTEXT pOWRHead;                // @field Pointer back to our APD_INIT_CONTEXT
	HANDLE            hwQueue;
	OWR_DEVICE_TYPE   owrDeviceType;
	TCHAR             owrQueueName[OWR_QUEUE_NAME_LEN/2];
	BOOL              register4notify;
	OWR_ACCESS_MODE   accessMode;
	UINT32            State;

} OWR_OPEN_CONTEXT, *POWR_OPEN_CONTEXT;





//==============================================================================================
//            Device registry parameters
//==============================================================================================

static const DEVICE_REGISTRY_PARAM s_OwrDeviceInitRegParams[] = 
{
    {
        L"deviceType", PARAM_DWORD, FALSE, offset(OWR_INIT_CONTEXT, owrDeviceType),
        fieldsize(OWR_INIT_CONTEXT, owrDeviceType), (VOID*)0
    },

    {
        L"Priority256", PARAM_DWORD, FALSE, offset(OWR_INIT_CONTEXT, priority256),
        fieldsize(OWR_INIT_CONTEXT, priority256), (VOID*)250
    }
};


//------------------------------------------------------------------------------
static const DEVICE_REGISTRY_PARAM s_OwrDeviceiButtonRegParams[] = {

	{
        L"mode", PARAM_DWORD, FALSE, offset(ONE_WIRE_iBUTTON, iButtonMode),
        fieldsize(ONE_WIRE_iBUTTON, iButtonMode), (VOID*)0
    },

	{
        L"prefix", PARAM_DWORD, FALSE, offset(ONE_WIRE_iBUTTON, iButtonPrefix),
        fieldsize(ONE_WIRE_iBUTTON, iButtonPrefix), (VOID*)0
    },

	{
        L"suffix", PARAM_DWORD, FALSE, offset(ONE_WIRE_iBUTTON, iButtonSuffix),
        fieldsize(ONE_WIRE_iBUTTON, iButtonSuffix), (VOID*)0
    }

};



/* Meanwhile hardcoded, may be put in Registry? */
OMAP_GPT_ISR_INFO s_OMAP_GPTimer = 
{
 4,
 0,
 0,
 0,
 0,
 0,
 NULL,                  //OMAP_GPTIMER3_REGS_PA,
 NULL,                  //OMAP_INTC_MPU_REGS_PA,
 0,
 NULL,
 0,
 0,
 NULL
};

OMAP_GPIO_ISR_INFO s_OWR_OMAP_GPIO = 
{
 NULL,
 GPIO_96, // CE 300
 GPIO_97,
 0,
 NULL,
 0,
 NULL,
 { 0,0,0,}
};

#if 0
OMAP_GPIO_ONE_WIRE s_OWR_OMAP = 
{
  s_OWR_OMAP_GPIO,
  s_OMAP_GPTimer,
  NULL,
  NULL,
  NULL
};
#endif

DWORD  g_outBuf = 0;
UINT8  g_romValue[8];
UINT8  g_iButtonPresence = FALSE;
BOOL   g_pulseDetect = FALSE;

//==============================================================================================
//           Local Functions
//==============================================================================================

DWORD IST_OWR_DETECT_Thread( LPVOID pContext );
DWORD IST_OWR_TEST_Thread( LPVOID pContext );
DWORD IST_OWR_TEST_Dummy_Thread( LPVOID pContext );

BOOL OWR_Deinit ( DWORD dwContext );
BOOL OWR_Close( DWORD Handle );

static BOOL owrAttachmentNotify(OWR_INIT_CONTEXT   *pInitCtx, OWR_EVENT_TYPE iButtonEvent);
static BOOL owrAttachmentInstanceNotify(POWR_OPEN_CONTEXT  pOpenCtx, OWR_EVENT_TYPE iButtonEvent);

static BOOL getOwrIsrParams(OMAP_GPIO_ONE_WIRE *pOWR_OMAP, OWR_INPUT_STATE *pOWRInputState, OWR_TIMER_STATE *pOWRGptState);



// ****************************************************************
//
//      @doc EXTERNAL
//      @func           HANDLE | OWR_Init | AIO driver initialization.
//
//      @parm           LPCTSTR  | Identifier | Port identifier.  The device loader
//                              passes in the registry key that contains information
//                              about the active device.
//
//      @remark         This routine is called at device load time in order
//                              to perform any initialization.   Typically the init
//                              routine does as little as possible, postponing memory
//                              allocation and device power-on to Open time.
//
//       @rdesc         Returns a pointer to the ioc head which is passed into
//                              the OWR_OPEN and OWR_DEINIT entry points as a device handle.
DWORD OWR_Init ( LPCTSTR pContext )
{
	OWR_INIT_CONTEXT *pCxt =0;
	int i;
	LPVOID              lpMsgBuf;
	DWORD               inBuf, dwLen;

	//OMAP_GPIO_REGS*    pGpio = OALPAtoUA(OMAP_GPIO1_REGS_PA);


    //DEBUGMSG( ZONE_FUNCTION, ( _T ( "AIO: +OWR_Init, pContext=%s\r\n" ), pContext ) );
	RETAILMSG(OWR_DRV_DBG, (L"+OWR_Init.\r\n"));

	// allocate AIO context object
	if( (pCxt = (OWR_INIT_CONTEXT *)LocalAlloc(LPTR, sizeof(OWR_INIT_CONTEXT))) == NULL )
	{
		return 0;
	}

	memset(pCxt, 0, sizeof(OWR_INIT_CONTEXT));

	 // Set cookie
    pCxt->cookie = OWR_DEVICE_COOKIE;


	pCxt->hOwrISTThread = NULL;
    pCxt->hOwrISTDummyThread = NULL;
	pCxt->quitThread = FALSE; 
	//pCxt->omapGpio1Wire.owrOmapGpio = s_OWR_OMAP_GPIO;
	memcpy( (char *)&pCxt->omapGpio1Wire.owrOmapGpio, (const char *)&s_OWR_OMAP_GPIO, sizeof(OMAP_GPIO_ISR_INFO) );
	//pCxt->omapGpio1Wire.owrOmapTimer = s_OMAP_GPTimer;
	memcpy( (char *)&pCxt->omapGpio1Wire.owrOmapTimer, (const char *)&s_OMAP_GPTimer, sizeof(OMAP_GPT_ISR_INFO) );
    pCxt->omapGpio1Wire.owrResetPresenceEvent = NULL;
    pCxt->omapGpio1Wire.owrWriteEvent = NULL;
    pCxt->omapGpio1Wire.owrReadEvent = NULL;

	
	// set all the open contexts to be null

	for(i = 0; i < (sizeof(pCxt->OpenContexts)/sizeof(pCxt->OpenContexts[0])); i++)
	{
		pCxt->OpenContexts[i] = 0;
	}

	
    // Read device parameters
    if(GetDeviceRegistryParams(pContext, pCxt, dimof(s_OwrDeviceInitRegParams), s_OwrDeviceInitRegParams) != ERROR_SUCCESS)
	{
        RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Init: Failed read OWR driver init registry parameters\r\n"));
        OWR_Deinit((DWORD)pCxt);
		return 0;
	}

	RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: reg: owrDeviceType = %d, priority = %d\r\n", pCxt->owrDeviceType, pCxt->priority256));

	if( pCxt->owrDeviceType == i_BUTTON )
	{
  	    
	 // read iButton registry parameters
		if(GetDeviceRegistryParams(OWR_REG_KEY_iBUTTON, &pCxt->omapGpio1Wire.owriButton, dimof(s_OwrDeviceiButtonRegParams), s_OwrDeviceiButtonRegParams) != ERROR_SUCCESS)
		{
			RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Init: Failed read OWR driver iButton registry parameters\r\n"));
			//OWR_Deinit((DWORD)pCxt);
			//return 0;
		}

		// iButtonMode
		RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: iButton reg: mode = %d, prefix = %s, suffix = %s\r\n", pCxt->omapGpio1Wire.owriButton.iButtonMode, pCxt->omapGpio1Wire.owriButton.iButtonPrefix, pCxt->omapGpio1Wire.owriButton.iButtonSuffix));

		if( pCxt->omapGpio1Wire.owriButton.iButtonMode == i_BUTTON_EXT_MODE )
		{
			if( checkiButtonPrefixnSuffix(pCxt->omapGpio1Wire.owriButton.iButtonPrefix) )
              RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: iButton reg: prefix OK\r\n"));
			else
              RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: iButton reg: prefix error\r\n"));

			if( checkiButtonPrefixnSuffix(pCxt->omapGpio1Wire.owriButton.iButtonSuffix) )
              RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: iButton reg: Suffix OK\r\n"));
			else
              RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: iButton reg: Suffix error\r\n"));

		}

		iButtonInit();
	}

	InitializeCriticalSection(&(pCxt->OpenCS));


	// Open gpio driver
	if( owrGPIOInit(&pCxt->omapGpio1Wire.owrOmapGpio) )
	{
	 RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: Gpio driver opened successfully, GPIO Ok\r\n"));
	}
	else
	{
	 RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Init: " L"Failed to open Gpio driver \r\n"));
	 //OWR_Deinit((DWORD)pCxt);
	 return(NULL);
	}

    // Init GPTimer 4  //s_OMAP_GPTimer
	RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: Start Init GpTimer\r\n"));
	if( gpTimerInit(&pCxt->omapGpio1Wire.owrOmapTimer) )
	{
	 RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: GPTimer driver opened successfully, GPTimer Ok\r\n"));
	}
	else
	{
	 RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Init: " L"Failed to open GPTimer driver \r\n"));
	 //OWR_Deinit((DWORD)pCxt);
	 return(NULL);
	}



#if 1
	// Multiple instance ???
	// s_OWR_OMAP_GPIO.gpioModule.gpioModuleIrq 
	/*g_IsrHandle_inp*/ pCxt->omapGpio1Wire.owrIsrHandle_gpio = LoadIntChainHandler(L"inp_isr.dll", L"ISRHandler", (BYTE)pCxt->omapGpio1Wire.owrOmapGpio.gpioModule.gpioModuleIrq );
	if( pCxt->omapGpio1Wire.owrIsrHandle_gpio == NULL )
     {

		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			0, // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);

		RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Init: LoadIntChainHandler for inp_isr.dll Failed: %s\r\n", (LPCTSTR)lpMsgBuf));
        //SetLastError(ERROR_INVALID_HANDLE);
        return(0);
     }
	else
	{
#if 1
		// Inform SysIntr to ISR.
		//inBuf = 0x11223344;
		static OMAP_GPIO_ISR_INFO  l_OWRGpioInfo;

		if( !KernelLibIoControl(pCxt->omapGpio1Wire.owrIsrHandle_gpio, IOCTL_ISR_GPIO_CFG, (LPVOID)&pCxt->omapGpio1Wire.owrOmapGpio, sizeof(OMAP_GPIO_ISR_INFO), (LPVOID)&l_OWRGpioInfo, sizeof(OMAP_GPIO_ISR_INFO), &dwLen) )
		{
     	 RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Init: KernelLibIoControl for inp_isr.dll Failed\r\n"));
         //SetLastError(ERROR_INVALID_HANDLE);
         return(0);
		}
		else
		{

         RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: KernelLibIoControl for inp_isr.dll pass OK, bIRQ = %d\r\n", (BYTE)pCxt->omapGpio1Wire.owrOmapGpio.gpioModule.gpioModuleIrq));
		 memcpy((char*)&pCxt->omapGpio1Wire.owrOmapGpio, (const char*)&l_OWRGpioInfo, sizeof(OMAP_GPIO_ISR_INFO));

#if 1
	RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: KernelLibIoControl return: dwLen = %d, hGpio = 0x%X, input = %d, output = %d, irqNum = %d, gpioReg = 0x%X, sysIntr = %d, hEvent = 0x%X, group Num = %d, IRQ = %d, PA = 0x%X\r\n",
		               dwLen, 
                       pCxt->omapGpio1Wire.owrOmapGpio.hGpio,
					   pCxt->omapGpio1Wire.owrOmapGpio.input,
                       pCxt->omapGpio1Wire.owrOmapGpio.output,
                       pCxt->omapGpio1Wire.owrOmapGpio.irqNum,
                       pCxt->omapGpio1Wire.owrOmapGpio.pGpio,
                       pCxt->omapGpio1Wire.owrOmapGpio.sysIntr,
                       pCxt->omapGpio1Wire.owrOmapGpio.hGPIOEvent,
			           pCxt->omapGpio1Wire.owrOmapGpio.gpioModule.gpioModuleNum, 
					   pCxt->omapGpio1Wire.owrOmapGpio.gpioModule.gpioModuleIrq,
					   pCxt->omapGpio1Wire.owrOmapGpio.gpioModule.gpioModulePA
					   ));
#endif
		}
#endif
	}
#endif


	if( (ResetEvent(pCxt->omapGpio1Wire.owrOmapGpio.hGPIOEvent)) == NULL )
	{
     RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Init: " L"Failed to Reset hGPIOEvent\r\n"));
	 return(0);
	}
	else
	{
     RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: hGPIOEvent = 0x%X\r\n", pCxt->omapGpio1Wire.owrOmapGpio.hGPIOEvent));
	}
	

	/*g_IsrHandle_gpt*/ pCxt->omapGpio1Wire.owrIsrHandle_gpt = LoadIntChainHandler(L"inp_isr.dll", L"ISRHandler", (BYTE)pCxt->omapGpio1Wire.owrOmapTimer.gpTimerIrQNum /*IRQ_GPTIMER3*/);
	if( pCxt->omapGpio1Wire.owrIsrHandle_gpt == NULL )
    {

		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			0, // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);

		RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Init: LoadIntChainHandler for gptIsr.dll Failed: %s\r\n", (LPCTSTR)lpMsgBuf));
        //SetLastError(ERROR_INVALID_HANDLE);
        return(0);
    }
	else
	{
		static OMAP_GPT_ISR_INFO  l_OMAP_GPTimer;

        RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: LoadIntChainHandler for gptIsr.dll Ok\r\n"));

		// Inform SysIntr to ISR.
		inBuf = 0x55667788;
		if( !KernelLibIoControl(pCxt->omapGpio1Wire.owrIsrHandle_gpt, IOCTL_ISR_TIMER_CFG, (LPVOID)&pCxt->omapGpio1Wire.owrOmapTimer, sizeof(OMAP_GPT_ISR_INFO), (LPVOID)&l_OMAP_GPTimer, sizeof(OMAP_GPT_ISR_INFO), &dwLen) )
		{
     	 RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Init: KernelLibIoControl for gptIsr.dll Failed\r\n"));
         //SetLastError(ERROR_INVALID_HANDLE);
         return(0);
		}
		else
		{

         RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: KernelLibIoControl for gptIsr.dll pass OK, inBuf = 0x%X\r\n", inBuf));
		 memcpy((char*)&pCxt->omapGpio1Wire.owrOmapTimer, (const char*)&l_OMAP_GPTimer, sizeof(OMAP_GPT_ISR_INFO));
#if 1
	RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: KernelLibIoControl return: dwLen = %d, num = %d, irq = %d, gptReg = 0x%X, ICLReg = 0x%X, sysIntr = %d, hEvent = 0x%X, DummyIrQNum = %d, dummySysIntr = %d, hGPTDummyEvent = 0x%X\r\n",
		               dwLen, 
                       pCxt->omapGpio1Wire.owrOmapTimer.gpTimerNum,
					   pCxt->omapGpio1Wire.owrOmapTimer.gpTimerIrQNum,
                       pCxt->omapGpio1Wire.owrOmapTimer.pGpt,
                       pCxt->omapGpio1Wire.owrOmapTimer.pICL,
                       pCxt->omapGpio1Wire.owrOmapTimer.sysIntr,
                       pCxt->omapGpio1Wire.owrOmapTimer.hGPTEvent,
                       pCxt->omapGpio1Wire.owrOmapTimer.gpTimerDummyIrQNum,
                       pCxt->omapGpio1Wire.owrOmapTimer.dummySysIntr,
                       pCxt->omapGpio1Wire.owrOmapTimer.hGPTDummyEvent
					   ));
#endif

		}

	}


	if( (ResetEvent(pCxt->omapGpio1Wire.owrOmapTimer.hGPTEvent)) == NULL )
	{
     RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Init: Failed to Reset hGPTEvent\r\n"));
	 return(0);
	}
	else
	{
     RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: hGPTEvent = 0x%X\r\n", pCxt->omapGpio1Wire.owrOmapTimer.hGPTEvent));
	}

	RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: hGPTDummyEvent = 0x%X\r\n", pCxt->omapGpio1Wire.owrOmapTimer.hGPTDummyEvent));

	if( (ResetEvent(pCxt->omapGpio1Wire.owrOmapTimer.hGPTDummyEvent)) == NULL )
	{
     RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Init: Failed to Reset hGPTDummyEvent\r\n"));
	 return(0);
	}
	/*
	else
	{
     RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: hGPTDummyEvent = 0x%X\r\n", pCxt->omapGpio1Wire.owrOmapTimer.hGPTDummyEvent));
	}
	*/


    if( (GPTInterruptInitialize( (OMAP_GPT_ISR_INFO *)&pCxt->omapGpio1Wire.owrOmapTimer ) ) == FALSE )
	{
     RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: gpTimerInit failed\r\n"));
	}

   //CalibrateStallCounter();

   // unmask interrupt
   GPIOInterruptMask(pCxt->omapGpio1Wire.owrOmapGpio.hGpio, pCxt->omapGpio1Wire.owrOmapGpio.input, FALSE);

	// Read iButton when start
   RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: iButton read begin\r\n"));

   // IST_OWR_DETECT_Thread
#if 1
	pCxt->hOwrISTThread = CreateThread(NULL, 0, IST_OWR_DETECT_Thread, pCxt, 0, NULL);
	if( ( pCxt->hOwrISTThread == NULL ))
	{
     RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Init: " L"Failed to CreateThread IST OWR Detect Thread\r\n"));
	}
	CeSetThreadPriority(pCxt->hOwrISTThread, 250);
#endif 

#if 0
	// IST_OWR_TEST_Thread
	pCxt->hOwrISTThread = CreateThread(NULL, 0, IST_OWR_TEST_Thread, pCxt, 0, NULL);
	if( ( pCxt->hOwrISTThread == NULL ))
	{
     RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Init: Failed to CreateThread IST_OWR_TEST_Thread\r\n"));
	}
	CeSetThreadPriority(pCxt->hOwrISTThread, 200);

#endif 

	//DWORD IST_OWR_TEST_Dummy_Thread( LPVOID pContext )
	pCxt->hOwrISTDummyThread = CreateThread(NULL, 0, IST_OWR_TEST_Dummy_Thread, pCxt, 0, NULL);
	if( ( pCxt->hOwrISTDummyThread == NULL ))
	{
     RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Init: Failed to CreateThread IST_OWR_TEST_Dummy_Thread\r\n"));
	}
	CeSetThreadPriority(pCxt->hOwrISTDummyThread, 120);

    RETAILMSG(OWR_DRV_DBG, (_T("\r\n-OWR_Init\r\n")));
	return (DWORD)pCxt;
}


//------------------------------------------------------------------------------
// Function name	: OWR_Deinit
// Description	    : DeInitialization of all used resources
// Return type		: BOOL  
// Argument         : DWORD dwContext
//------------------------------------------------------------------------------
BOOL OWR_Deinit ( DWORD dwContext )
{
	//DEBUGMSG ( ZONE_FUNCTION, ( _T ( "IOC: +OWR_Deinit, dwContext=%d\r\n" ), dwContext ) );
    OWR_INIT_CONTEXT *pInitCtx = (OWR_INIT_CONTEXT *)dwContext;
	OMAP_GPIO_ONE_WIRE *pOWR_OMAP = (OMAP_GPIO_ONE_WIRE *)&pInitCtx->omapGpio1Wire;
	POWR_OPEN_CONTEXT pOpenCtx;
	int i;

	RETAILMSG(OWR_DRV_DBG, (_T("+OWR: OWR_Deinit\r\n")));

	 // Check if we get correct context
    if(!pInitCtx || (pInitCtx->cookie != OWR_DEVICE_COOKIE))
	{
        RETAILMSG(OWR_DRV_DBG, (L"OWR: OWR_Deinit: Incorrect context parameter\r\n"));
        return 0;
	}

	EnterCriticalSection(&(pInitCtx->OpenCS));
	for(i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
	{
		pOpenCtx = (POWR_OPEN_CONTEXT )pInitCtx->OpenContexts[i];
		if (pOpenCtx)
		{
			OWR_Close((DWORD)pOpenCtx);
		}
	}

	// to close all the existed threads
	if( ( pOWR_OMAP->owrOmapGpio.hGPIOEvent != NULL )&&( pInitCtx->hOwrISTThread != NULL ) )
	{
	 pInitCtx->quitThread = TRUE;
     SetEvent(pOWR_OMAP->owrOmapGpio.hGPIOEvent);
	 WaitForSingleObject(pInitCtx->hOwrISTThread, INFINITE);
	 CloseHandle(pInitCtx->hOwrISTThread);
	 pInitCtx->hOwrISTThread = NULL;

	 owrGPIODeInit(&pOWR_OMAP->owrOmapGpio);
	 gpTimerDeInit(&pOWR_OMAP->owrOmapTimer);
	}

    //hGPTDummyEvent
	if( ( pOWR_OMAP->owrOmapTimer.hGPTDummyEvent != NULL )&&( pInitCtx->hOwrISTDummyThread != NULL ) )
	{
      pInitCtx->quitThread = TRUE;
	  SetEvent(pOWR_OMAP->owrOmapTimer.hGPTDummyEvent);
	  WaitForSingleObject(pInitCtx->hOwrISTDummyThread, INFINITE);
	  CloseHandle(pInitCtx->hOwrISTDummyThread);
	  pInitCtx->hOwrISTDummyThread = NULL;
	}

	LeaveCriticalSection(&(pInitCtx->OpenCS));
	DeleteCriticalSection(&(pInitCtx->OpenCS));


	LocalFree(pInitCtx);

	//DEBUGMSG( ZONE_FUNCTION, ( _T ( "APD: -OWR_Deinit\r\n" ) ) );
	RETAILMSG(OWR_DRV_DBG, (_T("-OWR: OWR_Deinit\r\n")));

    return TRUE;
}


//------------------------------------------------------------------------------
// Function name	: APD_IOControl
// Description	    : IOControl ... :-/
// Return type		: BOOL   IOC_IOControl 
// Argument         : DWORD  hHandle
// Argument         : DWORD  dwIoControlCode
// Argument         : PBYTE  pInBuf
// Argument         : DWORD  nInBufSize
// Argument         : PBYTE  pOutBuf
// Argument         : DWORD  nOutBufSize
// Argument         : PDWORD pBytesReturned
//------------------------------------------------------------------------------
BOOL OWR_IOControl ( DWORD  hHandle, 
					 DWORD  dwIoControlCode, 
					 PBYTE  pInBuf, 
					 DWORD  nInBufSize, 
					 PBYTE  pOutBuf,
					 DWORD  nOutBufSize,
					 PDWORD pBytesReturned )
{

	POWR_OPEN_CONTEXT pOpenCtx = (POWR_OPEN_CONTEXT)hHandle;
	POWR_INIT_CONTEXT pInitCtx = (POWR_INIT_CONTEXT)pOpenCtx->pOWRHead;
	//DWORD*            pBuff;
	BOOL bRet = TRUE;
	

	// Check if we get correct context
    if(!pInitCtx || (pInitCtx->cookie != OWR_DEVICE_COOKIE))
	{
        RETAILMSG(OWR_DRV_DBG, (L"OWR: OWR_IOControl: Incorrect context parameter\r\n"));
        //SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
 
	EnterCriticalSection(&(pInitCtx->OpenCS));
	switch (dwIoControlCode)
	{
		case IOCTL_OWR_GET_DEVICE_CONTEXT:
		{

			DWORD                              errorCode = 0;
			size_t                             len;
			//TCHAR                              name[OWR_QUEUE_NAME_LEN/2];
			OWR_GET_DEVICE_CONTEXT               nameContext;
			//POWR_GET_DEVICE_CONTEXT              pBuff;
			HRESULT                            nameValid;

			if(!pOutBuf || nOutBufSize < sizeof(OWR_GET_DEVICE_CONTEXT))
			{
				RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_OWR_GET_DEVICE_CONTEXT: OWR_IOControl: Incorrect parameter\r\n"));
				//SetLastError(ERROR_INVALID_PARAMETER);
				bRet = FALSE;
			}
			else
			{
             //pInitCtx->owrDeviceType
             nameContext.owrDeviceType = pInitCtx->owrDeviceType;

             // Validate name
			 nameValid = StringCchLength(pOpenCtx->owrQueueName, OWR_QUEUE_NAME_LEN/2, &len);
			 if( ( nameValid == S_OK )&& (len < OWR_QUEUE_NAME_LEN/2) )
			 {
               nameContext.opErrorCode = OWR_OK;
			   memcpy(nameContext.owrQueueName, pOpenCtx->owrQueueName, sizeof(pOpenCtx->owrQueueName));
               CeSafeCopyMemory (pOutBuf, &nameContext, sizeof(nameContext));
			 }
			 else
			 {
               nameContext.opErrorCode = ERROR_INVALID_NAME;
			   RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_OWR_GET_DEVICE_CONTEXT: OWR_IOControl: ERROR_INVALID_NAME\r\n"));
			 }

	         if(errorCode)
	  	   	  bRet = FALSE;

			}

			break;

		}


		case IOCTL_INPUT_REGISTER_NOTIFY:
		{
			DWORD             errorCode = 0;
			BOOL              needToRegister;
			static UINT32     s_registrationSt = FALSE;

			if(!pInBuf || nInBufSize < sizeof(BOOL) ||!pOutBuf || nOutBufSize < sizeof(DWORD))
			{
				RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_INPUT_REGISTER_NOTIFY: OWR_IOControl: Incorrect context parameter\r\n"));
				//SetLastError(ERROR_INVALID_PARAMETER);
				bRet = FALSE;
			}
			else
			{
				CeSafeCopyMemory (&needToRegister, pInBuf, sizeof(BOOL));

				// And relevant for iButton device only!
				if( pOpenCtx->accessMode != OWR_DENIED_ACCESS )
				{
					if(pInitCtx->owrDeviceType == i_BUTTON)
					{
						if( pOpenCtx->register4notify )
						{
						  if( needToRegister )
						  {
							errorCode =  OWR_ERROR_ALREDY_REGISTERED;
							RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_INPUT_REGISTER_NOTIFY: OWR_ERROR_ALREDY_REGISTERED for instance = 0x%08X\r\n", pOpenCtx));
						  }
						  else
						  {
							pOpenCtx->register4notify = needToRegister; // Unregistering
							RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_INPUT_REGISTER_NOTIFY: Unregistering for instance = 0x%08X\r\n", pOpenCtx));
						  }
						}
						else
						{
						  if( needToRegister )  
						  {
							pOpenCtx->register4notify = needToRegister; // Registering 
							RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_INPUT_REGISTER_NOTIFY: Registering for instance = 0x%08X\r\n", pOpenCtx));

							// If iButton was atatched before 1st registration, sent 
							// notification event !
							if( g_iButtonPresence == TRUE )
							{
							 OMAP_GPIO_ONE_WIRE *pOWR_OMAP = (OMAP_GPIO_ONE_WIRE *)&pInitCtx->omapGpio1Wire;

							 if( iButtonCheckPresence(pOWR_OMAP) )
							 {
 							  RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_INPUT_REGISTER_NOTIFY: iButton attached for instance = 0x%08X\r\n", pOpenCtx));
							  owrAttachmentNotify(pInitCtx, OWR_I_BUTTON_ATTACH);
							 }
							}
						  }
						  else
						  {
						   errorCode =  OWR_ERROR_ALREDY_UNREGISTERED;
						   RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_INPUT_REGISTER_NOTIFY: OWR_ERROR_ALREDY_UNREGISTERED for instance = 0x%08X\r\n", pOpenCtx));
						  }
						}
					}
					else
					{
                     errorCode =  OWR_ERROR_INVALID_ACCESS;
					 RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_INPUT_REGISTER_NOTIFY: OWR_ERROR_INVALID_ACCESS for instance = 0x%08X\r\n", pOpenCtx));
					}
				}
				else
				{
				   errorCode =  OWR_ERROR_ACCESS_DENIED;
  			       RETAILMSG(OWR_DRV_DBG, (L"ERROR: OWR_Open: Access denied! for instance = 0x%08X\r\n", pOpenCtx));
				}

				CeSafeCopyMemory (pOutBuf, &errorCode, sizeof(errorCode));

	         if(errorCode)
	  	   	  bRet = FALSE;

	         //SetLastError(errorCode);
			}

			break;
		}


		case IOCTL_INPUT_SET_ACCESS:
		{
			DWORD             errorCode = 0;
			OWR_ACCESS_MODE   accessMode;
			BOOL              bAccessMode; 
			INT32             i;  

			if(!pInBuf || nInBufSize < sizeof(BOOL) ||!pOutBuf || nOutBufSize < sizeof(DWORD))
			{
				RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_INPUT_SET_ACCESS: OWR_IOControl: Incorrect context parameter\r\n"));
				//SetLastError(ERROR_INVALID_PARAMETER);
				bRet = FALSE;
			}
			else
			{
				POWR_OPEN_CONTEXT pOpenCtxEnum;

				// OWR_ERROR_INPUT_WRONG_ACCESS
				CeSafeCopyMemory (&bAccessMode, pInBuf, sizeof(OWR_ACCESS_MODE));

				accessMode = (OWR_ACCESS_MODE)bAccessMode; 

				switch(accessMode)
				{
				  case OWR_PUBLIC_ACCESS:
					  {
					    if( pOpenCtx->accessMode == OWR_PRIVATE_ACCESS)
						{
                          //Release private access, set access mode for all
                          //processes to PUBLIC 
							for(i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
							{
				             pOpenCtxEnum = (POWR_OPEN_CONTEXT )pInitCtx->OpenContexts[i];
				             if(pOpenCtxEnum)
							 {
                               pOpenCtxEnum->accessMode = OWR_PUBLIC_ACCESS;
							   RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_INPUT_SET_ACCESS: Release private access for instance = 0x%08X\r\n", pOpenCtxEnum));
							 }
							}

						}
						else
						{
                          RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_INPUT_SET_ACCESS: OWR_ERROR_ACCESS_DENIED for instance = 0x%08X\r\n", pOpenCtx));
                          errorCode =  OWR_ERROR_ACCESS_DENIED; 
						}

					  } break;

				  case OWR_PRIVATE_ACCESS:
					  {
                        if( pOpenCtx->accessMode == OWR_PUBLIC_ACCESS)
						{
                          // Set access mode for this process to PRIVATE
                          // and for all other processes to DENIED
							for(i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
							{
				             pOpenCtxEnum = (POWR_OPEN_CONTEXT )pInitCtx->OpenContexts[i];
				             if( (pOpenCtxEnum)&& (pOpenCtxEnum != pOpenCtx) )
							  {
								owrAttachmentInstanceNotify(pOpenCtxEnum, OWR_I_BUTTON_ERROR);
                                pOpenCtxEnum->accessMode = OWR_DENIED_ACCESS;
								// And notify withh 'OWR_I_BUTTON_ERROR' event
								RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_INPUT_SET_ACCESS: set OWR_DENIED_ACCESS for instance = 0x%08X\r\n", pOpenCtxEnum));
							  }
							}

							pOpenCtx->accessMode = OWR_PRIVATE_ACCESS; 
							RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_INPUT_SET_ACCESS: set OWR_PRIVATE_ACCESS for instance = 0x%08X\r\n", pOpenCtx));
						}
						else
						{
                          errorCode =  OWR_ERROR_INVALID_ACCESS;  
                          RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_INPUT_SET_ACCESS: OWR_ERROR_INVALID_ACCESS for instance = 0x%08X\r\n", pOpenCtx));
						}

					  } break;

				  case OWR_DENIED_ACCESS:
					  {
                        errorCode =  OWR_ERROR_INVALID_ACCESS;
						RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_INPUT_SET_ACCESS: OWR_ERROR_INVALID_ACCESS for instance = 0x%08X\r\n", pOpenCtx));
					  } break;

				  default:  
					  {
                        errorCode =  OWR_ERROR_INVALID_ACCESS;
					  } break;

				}

				CeSafeCopyMemory (pOutBuf, &errorCode, sizeof(errorCode));

	         if(errorCode)
	  	   	  bRet = FALSE;

			 //SetLastError(errorCode);
			}

			break;
		}


		case IOCTL_OUTPUT_IBUTTON_READ:
		{
			//AUT_PINS          pinNo;
			//DWORD             errorCode;
			INT32             i ;
			OWR_IBUTTON_CONTEXT  iButtonContext;
			//UINT8                romValue[8];

			if(!pOutBuf || nOutBufSize < sizeof(OWR_IBUTTON_CONTEXT))
			{
				RETAILMSG(OWR_DRV_DBG, (L"OWR: IOCTL_OUTPUT_IBUTTON_READ: OWR_IOControl: Incorrect parameter\r\n"));
				//SetLastError(ERROR_INVALID_PARAMETER);
				iButtonContext.opErrorCode = OWR_ERROR_INVALID_IO_PARAMETER;
				bRet = FALSE;
			}
			else
			{
                if( pOpenCtx->accessMode != OWR_DENIED_ACCESS )
				{
					//if( iButtonRead(pCxt, romValue, OWR_IBUTTON_POLLING) )
                    // Read iButton data from cache! 
					if( g_iButtonPresence == TRUE )
					{
                     iButtonContext.opErrorCode = 0; 
                     memcpy( iButtonContext.romValue, g_romValue, 8 ); 

					  RETAILMSG( OWR_DRV_DBG, (L"OWR_IOControl: iButtonRead: read data Result:\r\n"));
					  for (i=7; i>=0; i--)
					   RETAILMSG( OWR_DRV_DBG, (L"[0x%02X]", iButtonContext.romValue[i] )); 

					  RETAILMSG( OWR_DRV_DBG, (L"\r\n"));
					}
					else
					{
                     iButtonContext.opErrorCode = (UINT32)OWR_ERROR_READ_FAULT;
					 RETAILMSG(OWR_DRV_DBG, (L"OWR_IOControl: iButtonRead Failure\r\n"));
					}
				}
				else
				  iButtonContext.opErrorCode = (UINT32)OWR_ERROR_ACCESS_DENIED;

	         if(iButtonContext.opErrorCode)
	  	   	  bRet = FALSE;

			  //SetLastError(iButtonContext.opErrorCode);

			  CeSafeCopyMemory (pOutBuf, &iButtonContext, sizeof(iButtonContext));
			}

			break;
		}


		//IOCTL_OUTPUT_OW
		case IOCTL_OUTPUT_OW:
		{
			//AUT_PINS          pinNo;
			//DWORD             state,mode;
			//DWORD             errorCode;
			DWORD             inBuf, /*outBuf,*/ dwLen;

			inBuf = 13;
			//dwLen = 0;
            if( KernelLibIoControl(pInitCtx->omapGpio1Wire.owrIsrHandle_gpio, IOCTL_ISR_GPIO_INFO, NULL, 0, (LPVOID)&g_outBuf, sizeof(g_outBuf), (LPDWORD)&dwLen) )
			{
     		 RETAILMSG(OWR_DRV_DBG, (L"OWR_IOControl: OW out KernelLibIoControl, outBuf = 0x%X, len = %d\r\n", g_outBuf, dwLen));
			 //RETAILMSG(OWR_DRV_DBG, (L"OWR_IOControl: OW out KernelLibIoControl, outBuf Addr = 0x%X\r\n", &g_outBuf));
			}

            if( KernelLibIoControl(pInitCtx->omapGpio1Wire.owrIsrHandle_gpt, IOCTL_ISR_TIMER_INFO, NULL, 0, (LPVOID)&g_outBuf, sizeof(g_outBuf), (LPDWORD)&dwLen) )
			{
     		 RETAILMSG(OWR_DRV_DBG, (L"OWR_IOControl: OW out KernelLibIoControl, outBuf = 0x%X, len = %d\r\n", g_outBuf, dwLen));
			 //RETAILMSG(OWR_DRV_DBG, (L"OWR_IOControl: OW out KernelLibIoControl, outBuf Addr = 0x%X\r\n", &g_outBuf));
			}

			break;
		}

		case IOCTL_OUTPUT_OW_TEST:
		{
			//AUT_PINS          pinNo;
			//DWORD             state,mode;
			//DWORD             errorCode;
			DWORD             inBuf;
			INT32             i ;
			UINT8             romValue[8];

			inBuf = 1;
			//dwLen = 0;
			if( iButtonRead(&pInitCtx->omapGpio1Wire, romValue) )
			{
			  RETAILMSG(0/*OWR_DRV_DBG*/, (L"OWR_IOControl: OW Test #1 iButtonRead Ok:  "));
			  //RETAILMSG( 0/*OWR_DRV_DBG*/, (L"iButtonRead: read data Result:\r\n"));
			  for (i=7; i>=0; i--)
			   RETAILMSG( 0/*OWR_DRV_DBG*/, (L"[0x%02X]", romValue[i] )); 

			  RETAILMSG( 0/*OWR_DRV_DBG*/, (L"\r\n"));

			}
			else
			{
             RETAILMSG(OWR_DRV_DBG, (L"OWR_IOControl: OW Test #1 iButtonRead Failure\r\n"));
			}
#if 0
            if( KernelLibIoControl(pInitCtx->omapGpio1Wire.owrIsrHandle_gpio, IOCTL_ISR_OWR_RESET_PRESENCE, NULL, 0, NULL, 0, NULL) )
			{
		     //owrIsrSetResetPulse();
   			 RETAILMSG(OWR_DRV_DBG, (L"OWR_IOControl: OW Test #1 KernelLibIoControl, inBuf = %d\r\n", inBuf));
			}
#endif

			break;
		}

		default:
		{
			//SetLastError(ERROR_INVALID_PARAMETER);
			bRet = FALSE;
			break;
		}
    }

	LeaveCriticalSection(&(pInitCtx->OpenCS));
	                                

	//DEBUGMSG( ZONE_FUNCTION, ( _T ( "AIO: -OWR_IOControl, bRet=%d\r\n" ), bRet ) );

    return bRet;
}


//------------------------------------------------------------------------------
// Function name	: OWR_Open
// Description	    : This function returns handle for safe access to automotive pins.
//                    This handle must be used for all automotive pins’ operations. 
// Return type		: DWORD  
// Argument         : DWORD dwContext

//------------------------------------------------------------------------------
DWORD OWR_Open ( DWORD dwContext, DWORD dwAccess, DWORD dwShareMode )
{
	//DEBUGMSG( ZONE_FUNCTION, ( TEXT ( "AIO: +OWR_Open\r\n" ) ) );
	RETAILMSG( OWR_DRV_DBG, ( TEXT ( "OWR: +OWR_Open\r\n" ) ) );

	OWR_INIT_CONTEXT *pInitCtx = (OWR_INIT_CONTEXT *)dwContext;
	POWR_OPEN_CONTEXT pOpenCtx;

	MSGQUEUEOPTIONS		msgOptions;
	DWORD				rc = 0;
	//LPVOID              lpMsgBuf;
	//DWORD               inBuf, dwLen;

//	TCHAR               QueueName[OWR_QUEUE_NAME_LEN/2];


	 // Return NULL if OWR_Init failed.
    if ( !pInitCtx ) {
        RETAILMSG(OWR_DRV_DBG,(TEXT("OWR_Open: Open attempted on uninited device!\r\n")));
        //SetLastError(ERROR_INVALID_HANDLE);
        return(0);
    }



	EnterCriticalSection(&(pInitCtx->OpenCS));

	for(int i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
	{
		if(pInitCtx->OpenContexts[i] == 0)
		{
			// Create device handle

			 // OK, lets allocate an open structure
			pOpenCtx    =  (POWR_OPEN_CONTEXT)LocalAlloc(LPTR, sizeof(OWR_OPEN_CONTEXT));
			
			if(!pOpenCtx)
			{
				RETAILMSG(OWR_DRV_DBG, (L"OWR_Open: Failed allocate open context driver structure\r\n"));

				//SetLastError(ERROR_INVALID_PARAMETER);
				rc = 0;
				break;
			}

			
			memset(pOpenCtx, 0, sizeof(OWR_OPEN_CONTEXT));

			// Inherit device type
			pOpenCtx->owrDeviceType = pInitCtx->owrDeviceType;

			memset(&msgOptions, 0, sizeof(msgOptions));
			msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
			msgOptions.dwFlags = 0;
			msgOptions.cbMaxMessage = sizeof(OWR_INPUT_CONTEXT);
			msgOptions.bReadAccess = 0;

			// initialize input specific data
			memset(pOpenCtx->owrQueueName, 0, sizeof(pOpenCtx->owrQueueName));
			wsprintf(pOpenCtx->owrQueueName, L"%s%d%s", OWR_QUEUE_NAME, i, WRITE_QUEUE_NAME); 

			// create write msg queue
			pOpenCtx->hwQueue = CreateMsgQueue(pOpenCtx->owrQueueName, &msgOptions);
			RETAILMSG(OWR_DRV_DBG, (L"OWR_Open:msg queue created: %s\r\n", pOpenCtx->owrQueueName));


			//memset(&pOpenCtx->mode, 0, sizeof(pOpenCtx->mode)); 

            pOpenCtx->register4notify = FALSE;
			pOpenCtx->accessMode = OWR_PUBLIC_ACCESS;

			pOpenCtx->pOWRHead = pInitCtx;  // pointer back to our parent
     
			pInitCtx->OpenContexts[i] = (UINT32)pOpenCtx;
			rc = (DWORD)pOpenCtx;
			break;
		}
		else
		{
         // Check if some process is already in PRIVATE access mode, deny opening!
		 pOpenCtx = (POWR_OPEN_CONTEXT )pInitCtx->OpenContexts[i];
		 if( pOpenCtx->accessMode != OWR_PUBLIC_ACCESS )
		 {
			RETAILMSG(OWR_DRV_DBG, (L"OWR_Open: Access denied!\r\n"));
			//SetLastError(ERROR_ACCESS_DENIED);
			rc = 0;
			break;
		 }
		}
	}

    LeaveCriticalSection(&(pInitCtx->OpenCS));


    RETAILMSG( OWR_DRV_DBG, (TEXT ("OWR: -OWR_Open, handle = 0x%08X\r\n"), pOpenCtx ) );
    return rc;

}

//------------------------------------------------------------------------------
// Function name	: OWR_Close
// Description	    : ...
// Return type		: BOOL 
// Argument         : DWORD Handle
//------------------------------------------------------------------------------
BOOL OWR_Close( DWORD Handle )
{
	//DEBUGMSG( ZONE_FUNCTION, ( TEXT ( "AIO: +OWR_Close\r\n" ) ) );
	RETAILMSG( OWR_DRV_DBG, ( TEXT ( "OWR: +OWR_Close\r\n" ) ) );

	POWR_OPEN_CONTEXT pOpenCtx = (POWR_OPEN_CONTEXT)Handle;
	POWR_OPEN_CONTEXT pOpenCtxEnum;
	POWR_INIT_CONTEXT pInitCtx = (POWR_INIT_CONTEXT)pOpenCtx->pOWRHead;
	int i;
	//DWORD state;
	BOOL  rc = FALSE;
	BOOL  release = FALSE;


	if ( !pInitCtx ) {
        RETAILMSG(OWR_DRV_DBG, (TEXT("OWR_Close: pInitCtx == NULL!!\r\n")));
        //SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

	EnterCriticalSection(&(pInitCtx->OpenCS));


	// If process was registered for notification - send msg about closing
    if( (pOpenCtx->register4notify) && (pOpenCtx->accessMode != OWR_DENIED_ACCESS) )
	{
	 OWR_INPUT_CONTEXT inputContext;

	 inputContext.size =  sizeof(OWR_INPUT_CONTEXT),
	 inputContext.opErrorCode = ERROR_INVALID_HANDLE;

	 // ??? doubt. Probably read queue was already closed by API close function
	 if(pOpenCtx->hwQueue)
	  {
		WriteMsgQueue(pOpenCtx->hwQueue, &inputContext, sizeof(inputContext), 10, 0);
		Sleep(0);
	  }

	}

	if(pOpenCtx->hwQueue)
	{
	 CloseMsgQueue(pOpenCtx->hwQueue);
	}

	// If this process has Private access, all other processes must be released !!!
    if( pOpenCtx->accessMode == OWR_PRIVATE_ACCESS )
     release = TRUE;

	for(i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
	{
		if(pInitCtx->OpenContexts[i] == (UINT32)pOpenCtx)
		{
			pOpenCtx->pOWRHead = NULL;
			LocalFree(pOpenCtx);
			pInitCtx->OpenContexts[i] = 0;
			rc = TRUE;
			break;
		}
	}

	// If this process has Private access, all other processes must be released !!!
	if( ( release == TRUE )&& (rc == TRUE) )
	{
     for(i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
	 {
	  if( pInitCtx->OpenContexts[i] != 0 )
	  {
       pOpenCtxEnum = (POWR_OPEN_CONTEXT )pInitCtx->OpenContexts[i]; 
	   pOpenCtxEnum->accessMode = OWR_PUBLIC_ACCESS;
	  }
	 }
	}

	LeaveCriticalSection(&(pInitCtx->OpenCS));

	//DEBUGMSG( ZONE_FUNCTION, ( TEXT ( "AIO: -OWR_Close\r\n" ) ) );
    RETAILMSG( OWR_DRV_DBG, ( TEXT ( "OWR: -OWR_Close\r\n" ) ) );
    return rc;
}

//------------------------------------------------------------------------------
// Function name	: OWR_Read
// Description	    : ...
// Return type		: DWORD OWR_Read 
// Argument         : DWORD Handle
// Argument         : LPVOID pBuffer
// Argument         : DWORD dwNumBytes
//------------------------------------------------------------------------------
DWORD OWR_Read ( DWORD Handle, LPVOID pBuffer, DWORD dwNumBytes )
{
	//DEBUGMSG( ZONE_FUNCTION, ( TEXT ( "AIO: +OWR_Read\r\n" ) ) );
    return (DWORD) -1;
}
	                            

//------------------------------------------------------------------------------
// Function name	: {
// Description	    : ...
// Return type		: DWORD { 
// Argument         : DWORD Handle
// Argument         : LPCVOID pBuffer
// Argument         : DWORD dwNumBytes
//------------------------------------------------------------------------------
DWORD OWR_Write ( DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes )
{
	//DEBUGMSG( ZONE_FUNCTION, ( TEXT ( "AIO: +OWR_WRITE\r\n" ) ) );
    return (DWORD) -1;
}


//------------------------------------------------------------------------------
// Function name	: OWR_Seek
// Description	    : ...
// Return type		: DWORD OWR_Seek 
// Argument         : DWORD Handle
// Argument         : long lDistance
// Argument         : DWORD dwMoveMethod
//------------------------------------------------------------------------------
DWORD OWR_Seek ( DWORD Handle, long lDistance, DWORD dwMoveMethod )
{
	//DEBUGMSG( ZONE_FUNCTION, ( TEXT ( "AIO: +OWR_Seek\r\n" ) ) );
	// ???
	//DEBUGMSG( ZONE_FUNCTION, ( TEXT ( "AIO: -OWR_Seek\r\n" ) ) );
    return (DWORD) -1;
}


//------------------------------------------------------------------------------
// Function name	: OWR_PowerUp
// Description	    : ...
// Return type		: void OWR_PowerUp 
// Argument         : DWORD hDeviceContext
//------------------------------------------------------------------------------
void OWR_PowerUp ( DWORD hDeviceContext )
{
	//DEBUGMSG( ZONE_FUNCTION, ( TEXT ( "AIO: +OWR_PowerUp\r\n" ) ) );
	//
	//DEBUGMSG( ZONE_FUNCTION, ( TEXT ( "AIO: -OWR_PowerUp\r\n" ) ) );
}


//------------------------------------------------------------------------------
// Function name	: OWR_PowerDown
// Description	    : ...
// Return type		: void OWR_PowerDown 
// Argument         : DWORD hDeviceContext
//------------------------------------------------------------------------------
void OWR_PowerDown ( DWORD hDeviceContext )
{
	//DEBUGMSG( ZONE_FUNCTION, ( TEXT ( "AIO: +OWR_PowerDown\r\n" ) ) );
	//
	//DEBUGMSG( ZONE_FUNCTION, ( TEXT ( "AIO: -OWR_PowerDown\r\n" ) ) );
}


// Function name	: DllEntry 
// Description	    : ...
// Return type		: BOOL WINAPI 
// Argument         : HANDLE hInstDll
// Argument         : DWORD dwReason
// Argument         : LPVOID lpvReserved
BOOL WINAPI DllEntry ( HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved )
{
	//DEBUGMSG( ZONE_FUNCTION, ( _T ( "AIO: +DllEntry\r\n" ) ) );
    RETAILMSG( OWR_DRV_DBG, ( TEXT ( "OWR: +DllEntry\r\n" ) ) );

    switch ( dwReason ) 
	{
    case DLL_PROCESS_ATTACH:

		DisableThreadLibraryCalls ( ( HINSTANCE )hInstDll );

		DEBUGREGISTER ( ( HINSTANCE )hInstDll );

        //DEBUGMSG( ZONE_FUNCTION, ( TEXT ( "AIO: DLL_PROCESS_ATTACH\r\n" ) ) );
		RETAILMSG( OWR_DRV_DBG, ( TEXT ( "OWR: DLL_PROCESS_ATTACH\r\n" ) ) );

        break;

    case DLL_PROCESS_DETACH:
    
        //DEBUGMSG( ZONE_FUNCTION, ( TEXT ( "AIO: DLL_PROCESS_DETACH\r\n" ) ) );
		RETAILMSG( OWR_DRV_DBG, ( TEXT ( "OWR: DLL_PROCESS_DETACH\r\n" ) ) );

        break;
    }


	//DEBUGMSG( ZONE_FUNCTION, ( _T ( "AIO: -DllEntry\r\n" ) ) );
	RETAILMSG( OWR_DRV_DBG, ( TEXT ( "OWR: -DllEntry\r\n" ) ) );

	return TRUE;
}


// Later, put in Registry
#define  IBUTTON_SCAN_PERIOD             250   // in milliseconds
#define  IBUTTON_SCAN_TRIALS             2     //


DWORD IST_OWR_TEST_Thread( LPVOID pContext )
{
  OWR_INIT_CONTEXT    *pInitCtx = (OWR_INIT_CONTEXT *)pContext;
  OMAP_GPIO_ONE_WIRE  *pOWR_OMAP = (OMAP_GPIO_ONE_WIRE *)&pInitCtx->omapGpio1Wire;
  BOOL                ret;
  DWORD         dwSize;
#if 0
  IOCTL_HAL_PRCMDEVICEENABLECLOCKS_IN  s_pcrmDevEnClks = {0,0};
  IOCTL_PRCM_CLOCK_SET_SOURCECLOCK_IN  s_pParentClk;
#endif

  RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_TEST_Thread: Started\r\n"));

  WaitForAPIReady(SH_WMGR, INFINITE );

  RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_TEST_Thread: APIReady\r\n"));
  for( dwSize = 0; dwSize < 5; dwSize++ )
  {
   RETAILMSG(OWR_DRV_DBG, (L"%d\r\n", dwSize));
   Sleep(1000);
  }

  while(1)
  {
#if 0
	//gpTimerInit(&pOWR_OMAP->owrOmapTimer);
    //update Timer clocks!
	s_pParentClk.clkId = pOWR_OMAP->owrOmapTimer.gpTimer_clkId; //  kGPT3_ALWON_FCLK;
    s_pParentClk.newParentClkId = pOWR_OMAP->owrOmapTimer.gpTimer_newParentClkId;  // kSYS_CLK;
	if( !KernelIoControl(IOCTL_PRCM_CLOCK_SET_SOURCECLOCK, (LPVOID)&s_pParentClk, sizeof(s_pParentClk), NULL, 0, &dwSize) )
	{
     RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_TEST_Thread: PrcmClockSetParent  failure\r\n"));
	}

	s_pcrmDevEnClks.devId = pOWR_OMAP->owrOmapTimer.gpTimer_devId;  //OMAP_DEVICE_GPTIMER3;
	s_pcrmDevEnClks.bEnable = TRUE;
	if( !KernelIoControl(IOCTL_HAL_PRCMDEVICEENABLECLOCKS, (LPVOID)&s_pcrmDevEnClks, sizeof(s_pcrmDevEnClks), NULL, 0, &dwSize) )
	{
     RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_TEST_Thread: PrcmDeviceEnableClocks  failure\r\n"));
	}
#endif

    ret = owrResetPresence(pOWR_OMAP);
	//WaitForSingleObject(pOWR_OMAP->owrOmapTimer.hGPTEvent, 400);
	Sleep(500);
  }
}


DWORD IST_OWR_TEST_Dummy_Thread( LPVOID pContext )
{
  OWR_INIT_CONTEXT    *pInitCtx = (OWR_INIT_CONTEXT *)pContext;
  OMAP_GPIO_ONE_WIRE  *pOWR_OMAP = (OMAP_GPIO_ONE_WIRE *)&pInitCtx->omapGpio1Wire;
  DWORD               dwReason;


  RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_TEST_Dummy_Thread: Started\r\n"));

  while(1)
  {
   dwReason = WaitForSingleObject(pOWR_OMAP->owrOmapTimer.hGPTDummyEvent, INFINITE);

   if( pInitCtx->quitThread == TRUE )
     return(0);

   StallExecution(600);
   RETAILMSG(0, (L"DE%d\r\n", dwReason ));   
  }

  return(0);
}

DWORD IST_OWR_DETECT_Thread( LPVOID pContext )
{
    OWR_INIT_CONTEXT   *pInitCtx = (OWR_INIT_CONTEXT *)pContext;
	OMAP_GPIO_ONE_WIRE *pOWR_OMAP = (OMAP_GPIO_ONE_WIRE *)&pInitCtx->omapGpio1Wire;
	POWR_OPEN_CONTEXT  pOpenCtx;
	//OWR_INPUT_STATE   lOWRInputState;
	//OWR_TIMER_STATE   lOWRGptState;
	DWORD             dwLen, dwReason,succsRead, seqRead, readTimeout;
	DWORD             iButtonScanPeriod, dwStartTick, dwEndTick, scanTrials;
	INT32             i; 
	//BOOL              pulseDetect;
	BOOL              kbEvent;
    static DWORD      pulseDetectCnt = 0;
	UINT8             romValue[8];
	DWORD             threadPriority;
	static UINT32     threadCnt = 0;


   
	RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: Started\r\n"));
	RETAILMSG(0, (L"IST_OWR_DETECT_Thread: Started\r\n"));
    WaitForAPIReady(SH_WMGR, INFINITE );

	RETAILMSG(0, (L"SYSTEM/GweApiSetReady\r\n"));

 

   if( pInitCtx->owrDeviceType == i_BUTTON )
   {
     if( iButtonRead(&pInitCtx->omapGpio1Wire, g_romValue) )
	 {
       g_iButtonPresence = TRUE;

//#if OWR_DRV_DBG == 1
	   RETAILMSG(0 /*OWR_DRV_DBG*/, (L"OWR_Init: iButton present - "));
 	   for(i=7; i>=0; i--)
		 RETAILMSG( 0 /*OWR_DRV_DBG*/, (L"[0x%02X]", g_romValue[i] )); 
//#endif /* OWR_DRV_DBG == 1 */
	 }
	 else
	 {
//#if OWR_DRV_DBG == 1
	   RETAILMSG(OWR_DRV_DBG, (L"OWR_Init: iButton CRC Error:\r\n"));
 	   for(i=7; i>=0; i--)
		 RETAILMSG(OWR_DRV_DBG, (L"[0x%02X]", g_romValue[i] )); 

	   RETAILMSG(OWR_DRV_DBG, (L"\r\n"));
//#endif /* OWR_DRV_DBG == 1 */
       g_iButtonPresence = FALSE; 
	 }
   }


    // Enable GPIO Input interrupt, ready to detect impulse
    g_pulseDetect = TRUE;
	if( !KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpio, IOCTL_ISR_OWR_DETECT_PULSE, (LPVOID)&g_pulseDetect, sizeof(BOOL), NULL, 0, (LPDWORD)&dwLen) )
	{
	 RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: KernelLibIoControl, IOCTL_ISR_OWR_DETECT_PULSE failure\r\n"));
	 //return(FALSE);
	}

	threadPriority = CeGetThreadPriority(GetCurrentThread ());

	if(g_iButtonPresence)
		iButtonScanPeriod = IBUTTON_SCAN_PERIOD;
	else
		iButtonScanPeriod = INFINITE;

  RETAILMSG(0, (L"IST_OWR_DETECT_Thread: Scan period %x\r\n", iButtonScanPeriod)); 
	scanTrials = 0;

	while(1)
	{
   
	//	RETAILMSG(0, (L"OWR_GPIO # %d mode = 0x%X\r\n", pOWR_OMAP->owrOmapGpio.input, GPIOGetMode(pOWR_OMAP->owrOmapGpio.hGpio, pOWR_OMAP->owrOmapGpio.input) ));

     dwReason = WaitForSingleObject(pOWR_OMAP->owrOmapGpio.hGPIOEvent, iButtonScanPeriod /*INFINITE*/);
     dwStartTick = GetTickCount();  

	 switch(dwReason)
	 {
	   case WAIT_OBJECT_0:
		{
         if( pInitCtx->quitThread == TRUE )
          return(0);

		 EnterCriticalSection(&(pInitCtx->OpenCS));

		 // Reset trials attempts
		 scanTrials = 0;

         // Disable GPIO Input interrupt, ready to detect impulse
		g_pulseDetect = FALSE;
		if( !KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpio, IOCTL_ISR_OWR_DETECT_PULSE, (LPVOID)&g_pulseDetect, sizeof(BOOL), NULL, 0, (LPDWORD)&dwLen) )
		{
		 RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: KernelLibIoControl, IOCTL_ISR_OWR_DETECT_PULSE failure\r\n"));
		 LeaveCriticalSection(&(pInitCtx->OpenCS));
		 return(FALSE);
		}

         // GPIOSetMode(pOWR_OMAP->owrOmapGpio.hGpio, pOWR_OMAP->owrOmapGpio.input, GPIO_DIR_INPUT);

		  readTimeout = 5;
          CeSetThreadPriority(GetCurrentThread(), 113);
          pulseDetectCnt++;
		  RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: Input Impulse detected # %d\r\n", pulseDetectCnt)); 
		  Sleep(readTimeout);
          GPIOInterruptDone(pOWR_OMAP->owrOmapGpio.hGpio, pOWR_OMAP->owrOmapGpio.input);
           
		  
		  succsRead = 0;
		  seqRead = 0;
		  for( i = 0; i < 20; i++ )
		  {
            if( iButtonRead(pOWR_OMAP, romValue) )
			{
			 succsRead++;
			 seqRead++;
			 readTimeout = 5;
			}
			else
			{
             seqRead = 0;
			 readTimeout = 35;
			}

			if( seqRead >= 1 )
              break;

			Sleep(5);
		  }

		  CeSetThreadPriority(GetCurrentThread(), threadPriority);

		  if( seqRead >= 1 )
		  {

#if OWR_DRV_DBG == 1
			  RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: iButtonRead Ok: "));
			  for (i=7; i>=0; i--)
			   RETAILMSG( OWR_DRV_DBG, (L"[0x%02X]", romValue[i] )); 

			  RETAILMSG( OWR_DRV_DBG, (L"\r\n"));
#endif /* OWR_DRV_DBG == 1 */

		  // Notify about iButton attach event
			
			 // Protect, if during 1000 ms timeout, iButton was changed!
			 //  Send de-attach event
             if( ( g_iButtonPresence == TRUE )&& ( memcmp(romValue, g_romValue, 8) ) )
			 {
              g_iButtonPresence = FALSE; 
    		  memset(g_romValue, 0, 8);
		      owrAttachmentNotify(pInitCtx, OWR_I_BUTTON_DEATTACH);	  
			 }
            
			 // Send attach event before
			 // Update global iButton cache
			 memcpy( g_romValue, romValue, 8);
			 g_iButtonPresence = TRUE;

			 owrAttachmentNotify(pInitCtx, OWR_I_BUTTON_ATTACH);	  

			 
			 if( pOWR_OMAP->owriButton.iButtonMode == i_BUTTON_EXT_MODE )
			 {
				 kbEvent = TRUE;
				 for(i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
				 {
				   pOpenCtx = (POWR_OPEN_CONTEXT )pInitCtx->OpenContexts[i];
				   if( (pOpenCtx) && 
					   ( (pOpenCtx->accessMode == OWR_DENIED_ACCESS) || (pOpenCtx->accessMode == OWR_PRIVATE_ACCESS) )
				     )
				   {
					 kbEvent = FALSE;
					 break;
				   }
				 }
			 }
			 else
              kbEvent = FALSE;

			 if( kbEvent )
			  owriButtonSendKeyStroke(pOWR_OMAP, romValue);


			// delay of 500 ms after iButton was read
			// Sleep(500);
		  }
		  else
		  {
            // Sometimes deattach event can come...
            if( g_iButtonPresence == TRUE )
			{
             g_iButtonPresence = FALSE; 
			 memset(g_romValue, 0, 8);
			 // Notify about de-attachment
			 owrAttachmentNotify(pInitCtx, OWR_I_BUTTON_DEATTACH);
			}

            RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: iButtonRead Failure, succsRead = %d\r\n", succsRead));
		  }

          if(g_iButtonPresence)
		  {
           dwEndTick = GetTickCount();
		   if( IBUTTON_SCAN_PERIOD >(dwEndTick - dwStartTick) )
		    iButtonScanPeriod = IBUTTON_SCAN_PERIOD - (dwEndTick - dwStartTick);
		   else
            iButtonScanPeriod = 0; 
		  }
		  else
		  {
            iButtonScanPeriod = INFINITE;
			g_pulseDetect = TRUE;
			if( !KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpio, IOCTL_ISR_OWR_DETECT_PULSE, (LPVOID)&g_pulseDetect, sizeof(BOOL), NULL, 0, (LPDWORD)&dwLen) )
			{
			 RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: KernelLibIoControl, IOCTL_ISR_OWR_DETECT_PULSE failure\r\n"));
			 LeaveCriticalSection(&(pInitCtx->OpenCS));
			 return(FALSE);
			}
		  }

           // "but 1000ms was not yet expired, so before return g_pulseDetect TRUE, 
		   // wait remaining time here up to IBUTTON_SCAN_PERIOD ms"
//		   dwEndTick = GetTickCount();

//		   if( IBUTTON_SCAN_PERIOD >(dwEndTick - dwStartTick) )
//             Sleep(IBUTTON_SCAN_PERIOD - (dwEndTick - dwStartTick));
           
//		   iButtonScanPeriod = 0;

		 LeaveCriticalSection(&(pInitCtx->OpenCS));

		} break;


	   case  WAIT_TIMEOUT:
	    {
          BOOL    iButtonPresence, iButtonReadSt;
//#error "Use number of retries (at least 2) before confirm deattaching"
// If all of them no response from 'ButtonCheckPresence'  - then deattach
// If 'ButtonCheckPresence' response - read physically iButton
// If 'ButtonCheckPresence' response OK, but the number is different - confirm deattach



          EnterCriticalSection(&(pInitCtx->OpenCS));

            // Check iButton presence
		    //RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: 1 sec timeout, iButton presence = %d\r\n", g_iButtonPresence));

		    //g_pulseDetect = TRUE;
            if( g_iButtonPresence == TRUE )
			{
             g_pulseDetect = FALSE; 
             scanTrials++;
			 iButtonPresence = iButtonCheckPresence(pOWR_OMAP);


			 switch(scanTrials)
			 {
			  case 1:
			   {
                if( iButtonPresence == TRUE )
				{
  				 RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: scanTrials = %d, iButtonPresence TRUE\r\n", scanTrials));
                 //g_pulseDetect = FALSE; //iButton still attached
				 scanTrials = 0;
				}
				else
				{
                 RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: scanTrials = %d, iButtonPresence FALSE\r\n", scanTrials));                 
				}

			   } break;

			  case 2:
			   {
				iButtonReadSt = FALSE;

				if( iButtonPresence == TRUE )
				{
				  RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: scanTrials = %d, iButtonPresence TRUE\r\n", scanTrials));
                 // Physically read iButton and compare with previous value
                 // If read OK and value is equal to previous, confirm that iButton is
                 // still attached
				  if( iButtonRead(pOWR_OMAP, romValue) )
				  {
                   RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: scanTrials = %d, iButtonRead OK\r\n", scanTrials));
                   if( !memcmp(romValue, g_romValue, 8) )
					 iButtonReadSt = TRUE;
				  }
                  
				}

			    if( iButtonReadSt == TRUE )
				{
				  //g_pulseDetect = FALSE; //iButton still attached
                  RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: scanTrials = %d, iButtonPresence TRUE\r\n", scanTrials));   
				}
				else
				{
			     RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: scanTrials = %d, iButton de-attached\r\n", scanTrials));

			     g_iButtonPresence = FALSE;
			     memset(g_romValue, 0, 8);
			     owrAttachmentNotify(pInitCtx, OWR_I_BUTTON_DEATTACH);
				 g_pulseDetect = TRUE;
				}

				scanTrials = 0;

			   } break;

			  default:
			   {
				RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: scanTrials = %d, Error state\r\n", scanTrials));
				LeaveCriticalSection(&(pInitCtx->OpenCS));
				return(FALSE);
			   } break;

			 }
			 
			}
			else
			 g_pulseDetect = TRUE;

			//scanTrials = 0;
			 // Enable GPIO Input interrupt, ready to detect impulse
		     // Only when deattach status set, enable impulse detection
          if(g_iButtonPresence)
		  {
           dwEndTick = GetTickCount();
		   if( IBUTTON_SCAN_PERIOD >(dwEndTick - dwStartTick) )
		    iButtonScanPeriod = IBUTTON_SCAN_PERIOD - (dwEndTick - dwStartTick);
		   else
            iButtonScanPeriod = 0; 

		   RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: iButtonScanPeriod = %d\r\n", iButtonScanPeriod));
		  }
		  else
		  {
            iButtonScanPeriod = INFINITE;
			g_pulseDetect = TRUE;
			if( !KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpio, IOCTL_ISR_OWR_DETECT_PULSE, (LPVOID)&g_pulseDetect, sizeof(BOOL), NULL, 0, (LPDWORD)&dwLen) )
			{
			 RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: KernelLibIoControl, IOCTL_ISR_OWR_DETECT_PULSE failure\r\n"));
			 LeaveCriticalSection(&(pInitCtx->OpenCS));
			 return(FALSE);
			}
		  }

#if 0
			  if(g_pulseDetect == TRUE)
			   {
				if( !KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpio, IOCTL_ISR_OWR_DETECT_PULSE, (LPVOID)&g_pulseDetect, sizeof(BOOL), NULL, 0, (LPDWORD)&dwLen) )
				{
				 RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: KernelLibIoControl, IOCTL_ISR_OWR_DETECT_PULSE failure\r\n"));
				 LeaveCriticalSection(&(pInitCtx->OpenCS));
				 return(FALSE);
				}
				iButtonScanPeriod = INFINITE;
                RETAILMSG(0, (L"IST_OWR_DETECT_Thread: detach %x\r\n", iButtonScanPeriod)); 
			   }
#endif

		  LeaveCriticalSection(&(pInitCtx->OpenCS));

//		   dwEndTick = GetTickCount();
//		   if( IBUTTON_SCAN_PERIOD >(dwEndTick - dwStartTick) )
//            iButtonScanPeriod = IBUTTON_SCAN_PERIOD - (dwEndTick - dwStartTick);
//           else
//		    iButtonScanPeriod = 0;

	    } break;
 
	   default:
	    {
         RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: WaitForSingleObject, return error = %d\r\n", dwReason)); 
         //RETAILMSG(OWR_DRV_DBG, (L"IST_OWR_DETECT_Thread: HANDLES = 0x%X, 0x%X\r\n", pOWR_OMAP->owrOmapGpio.hGPIOEvent, pOWR_OMAP->owrOmapTimer.hGPTEvent)); 
		 return(1);
		} break;

	 }


	   threadCnt++;
	} // while(1)

  return(0);
}



static BOOL owrAttachmentNotify(OWR_INIT_CONTEXT   *pInitCtx, OWR_EVENT_TYPE iButtonEvent)
{
  INT32              i;  
  POWR_OPEN_CONTEXT  pOpenCtx;

  RETAILMSG(OWR_DRV_DBG, (L"+owrAttachmentNotify\r\n"));

  for(i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
  {
    pOpenCtx = (POWR_OPEN_CONTEXT )pInitCtx->OpenContexts[i];
	owrAttachmentInstanceNotify(pOpenCtx, iButtonEvent ); 
  }

  RETAILMSG(OWR_DRV_DBG, (L"-owrAttachmentNotify\r\n"));
  return(TRUE);
}


static BOOL owrAttachmentInstanceNotify(POWR_OPEN_CONTEXT  pOpenCtx, OWR_EVENT_TYPE iButtonEvent)
{
	if (pOpenCtx)
	{
		if( pOpenCtx->register4notify && pOpenCtx->accessMode != OWR_DENIED_ACCESS )
		{
		 OWR_INPUT_CONTEXT inputContext;

		 inputContext.size =  sizeof(OWR_INPUT_CONTEXT);
         switch(iButtonEvent)
		 {
		  case OWR_I_BUTTON_DEATTACH: inputContext.opErrorCode = OWR_OK; break;
		  case OWR_I_BUTTON_ATTACH: inputContext.opErrorCode = OWR_OK; break;
		  case OWR_I_BUTTON_ERROR: inputContext.opErrorCode = OWR_ERROR_ACCESS_DENIED; break;
		  default: inputContext.opErrorCode = OWR_OK;   break; 
		 }
		 //inputContext.opErrorCode = OWR_OK;
		 inputContext.oneWireDatalength = 0;
		 inputContext.oneWireEventType = (UINT32)iButtonEvent;

		 if(pOpenCtx->hwQueue)
		  {
		   // Send attach/de-attach event
		   RETAILMSG(OWR_DRV_DBG, (L"owrAttachmentNotify: attachment event = %d to instance = 0x%08X\r\n", iButtonEvent, pOpenCtx));
		   WriteMsgQueue(pOpenCtx->hwQueue, &inputContext, sizeof(inputContext), INFINITE, 0);
		  }
		}
    }
 return(TRUE);
}



static BOOL getOwrIsrParams(OMAP_GPIO_ONE_WIRE *pOWR_OMAP, OWR_INPUT_STATE *pOWRInputState, OWR_TIMER_STATE *pOWRGptState)
{
  DWORD dwLen;

	if( !KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpio, IOCTL_ISR_GPIO_GET_DATA, NULL, 0, (LPVOID)pOWRInputState, sizeof(OWR_INPUT_STATE), (LPDWORD)&dwLen) )
	{
	 RETAILMSG(OWR_DRV_DBG, (L"getOwrIsrParams: KernelLibIoControl, IOCTL_ISR_GPIO_GET_DATA failure\r\n"));
	 return(FALSE);
	}

	if( !KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpt, IOCTL_ISR_TIMER_GET_DATA, NULL, 0, (LPVOID)pOWRGptState, sizeof(OWR_TIMER_STATE), (LPDWORD)&dwLen) )
	{
	 RETAILMSG(OWR_DRV_DBG, (L"getOwrIsrParams: KernelLibIoControl, IOCTL_ISR_TIMER_GET_DATA failure\r\n"));
	 return(FALSE);
	}

  	 return(TRUE);
}


BOOL owrResetPresence(OMAP_GPIO_ONE_WIRE *pOWR_OMAP)
{
 OWR_INPUT_STATE   lOWRInputState;
 OWR_TIMER_STATE   lOWRGptState;
 DWORD             dwReason;
 DWORD             dwStartTickCount = 0;
 DWORD             dwStopTickCount = 0;
 //DWORD             dwTimerCnt, dwLen;

  RETAILMSG(OWR_DRV_DBG, (L"owrResetPresence: Started\r\n"));

    dwStartTickCount = GetTickCount();

    if( !KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpio, IOCTL_ISR_OWR_RESET_PRESENCE, NULL, 0, NULL, 0, NULL) )
	{
	 RETAILMSG(OWR_DRV_DBG, (L"owrResetPresence: IOCTL_ISR_OWR_RESET_PRESENCE start Failure\r\n"));
	 return(FALSE);
	}

	// Wait event of RESET_PRESENCE finish
    dwReason = WaitForSingleObject(pOWR_OMAP->owrOmapTimer.hGPTEvent, 400);
	dwStopTickCount = GetTickCount();
	//if( dwStopTickCount - dwStartTickCount > 150 )
	//{
    // RETAILMSG(1, (L"Reset Wait = %d\r\n", (dwStopTickCount - dwStartTickCount) ));   
	//}

	if( dwReason != WAIT_OBJECT_0 )
	{
      RETAILMSG(OWR_DRV_DBG, (L"owrResetPresence: IOCTL_ISR_OWR_RESET_PRESENCE event Failure\r\n")); 
#if 0 // Test
	  if( !getOwrIsrParams(pOWR_OMAP, &lOWRInputState, &lOWRGptState) )
        return(FALSE);

	  RETAILMSG(0, (L"Imp: [%d - %d]\r\n", lOWRGptState.cntBegin, lOWRGptState.cntEnd ));
#endif
	  RETAILMSG(0, (L"Reset Wait(ErT) = %d\r\n", (dwStopTickCount - dwStartTickCount) ));   
	  return(FALSE);
	}
	else
	{
      GPTInterruptDone(&pOWR_OMAP->owrOmapTimer); 

	  // Get parameters of RESET_PRESENCE
	  if( !getOwrIsrParams(pOWR_OMAP, &lOWRInputState, &lOWRGptState) )
	  {
        RETAILMSG(OWR_DRV_DBG, (L"owrResetPresence: getOwrIsrParams Failure\r\n"));
        return(FALSE);
	  }

	  RETAILMSG(0, (L"GPT4 = %d\r\n", lOWRGptState.gptIntrCnt ));

	   //Sleep(50);
	   // Verify
		if( ( lOWRGptState.phase == OWR_RESET_PRESENCE )&&
            ( lOWRGptState.endOp == TRUE )&&
            ((lOWRGptState.cntEnd - lOWRGptState.cntBegin) > 12346) && // 480*25.72  - Reset Pulse
			( lOWRInputState.numOfPulses == 1 ) &&
			( (lOWRInputState.cntBegin - lOWRGptState.cntEnd) > 386) &&// 15 * 25.72 - presence begin min
			( (lOWRInputState.cntBegin - lOWRGptState.cntEnd) < 3858 /*1544*/) && // (60)150 * 25.72 - presence begin max
			( (lOWRInputState.cntEnd - lOWRInputState.cntBegin) > 1543) && // 60 * 25.72 - lenght of presence pulse begin min
			( (lOWRInputState.cntEnd - lOWRInputState.cntBegin) < 6173)  // 240 * 25.72 - lenght of presence pulse begin min
		  )
		{
         RETAILMSG(OWR_DRV_DBG, (L"owrResetPresence: Reset/Presence OK\r\n"));
		 RETAILMSG(OWR_DRV_DBG, (L"owrResetPresence: Reset/Presence : Imp + waiting, begin = %d, match = %d, end = %d\r\n", lOWRGptState.cntBegin, lOWRGptState.cntMatch, lOWRGptState.cntEnd ));
		 RETAILMSG(OWR_DRV_DBG, (L"owrResetPresence: Presence : cnt = %d, begin = %d, end = %d\r\n", lOWRInputState.numOfPulses, lOWRInputState.cntBegin, lOWRInputState.cntEnd )); 				 
#if OWR_DRV_DBG == 0
		 RETAILMSG(0, (L"Ok: [%d - %d][%d - %d]\r\n", lOWRGptState.cntBegin, lOWRGptState.cntEnd, lOWRInputState.cntBegin, lOWRInputState.cntEnd ));
#endif
		 RETAILMSG(0, (L"Reset Wait(Ok) = %d\r\n", (dwStopTickCount - dwStartTickCount) ));   
		}
		else
		{
		 RETAILMSG(OWR_DRV_DBG, (L"owrResetPresence: Reset/Presence error: Imp + waiting, begin = %d, match = %d, end = %d\r\n", lOWRGptState.cntBegin, lOWRGptState.cntMatch, lOWRGptState.cntEnd ));
		 RETAILMSG(OWR_DRV_DBG, (L"owrResetPresence: Presence Imp error: cnt = %d, begin = %d, end = %d\r\n", lOWRInputState.numOfPulses, lOWRInputState.cntBegin, lOWRInputState.cntEnd )); 				 
#if OWR_DRV_DBG == 0
		 RETAILMSG(0, (L"Er: [%d - %d - %d][%d - %d] T = %d\r\n", lOWRGptState.cntBegin, lOWRGptState.cntMatch, lOWRGptState.cntEnd, lOWRInputState.cntBegin, lOWRInputState.cntEnd, (dwStopTickCount - dwStartTickCount) ));
#endif
		 RETAILMSG(0, (L"Reset Wait(Er) = %d\r\n", (dwStopTickCount - dwStartTickCount) ));   
		 return(FALSE);
		}
	} // Wait event of RESET_PRESENCE finish

  return(TRUE);
}


BOOL owrWrite(OMAP_GPIO_ONE_WIRE *pOWR_OMAP, UINT8 data[], UINT32 dataLen)
{
 OWR_INPUT_STATE   lOWRInputState;
 OWR_TIMER_STATE   lOWRGptState;
 DWORD             dwReason, dwLen;

    RETAILMSG(OWR_DRV_DBG, (L"owrWrite: Started\r\n"));

   // Launch ROM command
	if( KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpt, IOCTL_ISR_OWR_ROM_CMD, (LPVOID)data, dataLen, NULL, 0, (LPDWORD)&dwLen) )
	{
	 RETAILMSG(OWR_DRV_DBG, (L"owrWrite: start Ok\r\n"));
	 //RETAILMSG(OWR_DRV_DBG, (L"OWR_IOControl: OW out KernelLibIoControl, outBuf Addr = 0x%X\r\n", &g_outBuf));
	}
	else
	{
     RETAILMSG(1/*OWR_DRV_DBG*/, (L"owrWrite: start Failure\r\n"));
	 return(FALSE);
	}

	// Wait event of OWR_ROM_CMD finish
    dwReason = WaitForSingleObject(pOWR_OMAP->owrOmapTimer.hGPTEvent, 400/*50*/);
	if( dwReason != WAIT_OBJECT_0 )
	{
      RETAILMSG(1/*OWR_DRV_DBG*/, (L"owrWrite: event Failure\r\n")); 
	  return(FALSE);
	}
	else
	{
      GPTInterruptDone(&pOWR_OMAP->owrOmapTimer); 

      // Get parameters of OWR_ROM_CMD
	  if( !getOwrIsrParams(pOWR_OMAP, &lOWRInputState, &lOWRGptState) )
	  {
        RETAILMSG(1/*OWR_DRV_DBG*/, (L"owrWrite: getOwrIsrParams Failure\r\n"));
        return(FALSE);
	  }

	  if( ( lOWRGptState.phase == OWR_WRITE_SLOTS )&&
            ( lOWRGptState.endOp == TRUE )
		)
	  {
        RETAILMSG(OWR_DRV_DBG, (L"owrWrite: Write Finished OK\r\n"));
	  }
	  else
	  {
		 RETAILMSG(1/*OWR_DRV_DBG*/, (L"owrWrite: Write Failure\r\n"));
		 return(FALSE);
	  }
	} // Wait event of OWR_ROM_CMD finish

 return(TRUE);
}


BOOL owrRead(OMAP_GPIO_ONE_WIRE *pOWR_OMAP, UINT8 data[], UINT32 dataLen)
{
 OWR_INPUT_STATE   lOWRInputState;
 OWR_TIMER_STATE   lOWRGptState;
 DWORD             dwReason, dwLen, readDataLen;

    RETAILMSG(OWR_DRV_DBG, (L"owrRead: Started\r\n"));

    // For iButton recieve 8 bytes
    readDataLen = dataLen;
	if( KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpt, IOCTL_ISR_OWR_READ, (LPVOID)&readDataLen, sizeof(DWORD), NULL, 0, (LPDWORD)&dwLen) )
	{
	 RETAILMSG(OWR_DRV_DBG, (L"owrRead: start Ok\r\n"));
	 //RETAILMSG(OWR_DRV_DBG, (L"OWR_IOControl: OW out KernelLibIoControl, outBuf Addr = 0x%X\r\n", &g_outBuf));
	}
	else
	{
	 RETAILMSG(1/*OWR_DRV_DBG*/, (L"owrRead: start Failure\r\n"));
	 return(FALSE);
	}

	// Wait event of OWR_READ finish
    dwReason = WaitForSingleObject(pOWR_OMAP->owrOmapTimer.hGPTEvent, 400/*100*/);
	if( dwReason != WAIT_OBJECT_0 )
	{
      RETAILMSG(1/*OWR_DRV_DBG*/, (L"owrRead: event Failure\r\n")); 
	  return(FALSE);
	}
	else
	{
      GPTInterruptDone(&pOWR_OMAP->owrOmapTimer); 

      // Get parameters of OWR_READ
	  if( !getOwrIsrParams(pOWR_OMAP, &lOWRInputState, &lOWRGptState) )
	  {
        RETAILMSG(1/*OWR_DRV_DBG*/, (L"owrRead: getOwrIsrParams Failure\r\n"));
        return(FALSE);
	  }

	  if( ( lOWRGptState.phase == OWR_READ_SLOTS )&&
            ( lOWRGptState.endOp == TRUE )
		)
	  {
	   //RETAILMSG(OWR_DRV_DBG, (L"owrRead: Finished OK\r\n"));
		// For iButton recieve 8 bytes
		//dwRomCmd = 8;
		// get data
		if(KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpt, IOCTL_ISR_TIMER_GET_DATA, NULL, 0, (LPVOID)&lOWRGptState, sizeof(OWR_TIMER_STATE), (LPDWORD)&dwLen) )
		{
         memcpy( data, lOWRGptState.readData, readDataLen ); 

		 //RETAILMSG(OWR_DRV_DBG, (L"OWR_IOControl: OW out KernelLibIoControl, outBuf Addr = 0x%X\r\n", &g_outBuf));
		}
		else
		{
		 RETAILMSG(1/*OWR_DRV_DBG*/, (L"owrRead: Finish Failure\r\n"));
		 return(FALSE);
		}
	  }
	  else
	  {
	   RETAILMSG(1/*OWR_DRV_DBG*/, (L"owrRead: Failure\r\n"));
	   return(FALSE);
	  }
	}

 RETAILMSG(OWR_DRV_DBG, (L"owrRead: Finish Ok\r\n"));
 return(TRUE);
}
//#pragma optimize("",on)




