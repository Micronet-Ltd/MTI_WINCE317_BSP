//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
#include <windows.h>
#include <pm.h>
#include <oal.h>
#include <ceddkex.h>
#include <bt_buffer.h>
#include <bt_hcip.h>
#include <bt_os.h>
#include <bt_debug.h>
#include <bt_tdbg.h>
#include <winbase.h>
#include "bthci.h"

#include <ceddk.h>
#include <omap35xx_base_regs.h>  // base register PA 
#include <omap35xx.h>
#include <omap35xx_irq.h>
#include <nkintr.h>
#include <constants.h>

#include <initguid.h>
#include <gpio.h>

#include <oalex.h>
#include <args.h>

#define BT_HCI_SIZEOFARRAY(a)  (sizeof(a) / sizeof(a[0]))

#define PACKET_BODY_OFFSET              3
#define PACKET_BODY_SIZE              257
//------------------------------------------------------------------------------

#define PACKET_SIZE_R                   (64 * 1024 + 128)
#define PACKET_SIZE_W                   (64 * 1024)

// Read/WritePacket should never fail, but just in case there is a hardware
// problem we will timeout after 5 second.
#define DEFAULT_PACKET_TIMEOUT            5000
// used to thrash the unwanted packet.
#define THRASH_SIZE 16

//------------------------------------------------------------------------------

#define DEBUG_READ_BUFFER_HEADER        4
#define DEBUG_WRITE_BUFFER_HEADER       8
#define DEBUG_READ_BUFFER_TRAILER       1
#define DEBUG_WRITE_BUFFER_TRAILER      3


//------------------------------------------------------------------------------
DECLARE_DEBUG_VARS();
//------------------------------------------------------------------------------

typedef enum {
    BT_SLEEP = 0,
    BT_SLEEP_TO_AWAKE,
    BT_AWAKE_TO_SLEEP,
    BT_AWAKE
} BT_POWER_MODE;

typedef enum {
    REQUEST_NONE = 0,
    REQUEST_SEND,
/*  _HOST_INITIATED_BT_SLEEP_OR_AWAKE_, is not defined, so this part of the code will be ignored by the Compiler */
#ifdef _HOST_INITIATED_BT_SLEEP_OR_AWAKE_
/*  The host is not setting this request anywhere, when the host has some data to send, SendPacketRequest sends it*/
    REQUEST_AWAKE,
/*  The host is not setting this request anywhere, the host cannot send sleep request, In future, if the sleep is sent by the host, this can be used.*/
    REQUEST_SLEEP,
#endif
    REQUEST_RELEASE_BUFFER
} REQUEST;

typedef struct {

    LPWSTR  comPortName;
    LPWSTR  startScript;
    DWORD   baud;
    DWORD   priority256;
    DWORD   wakeUpTimer;
    DWORD   cmdTimeout;
    DWORD   resetDelay;
    DWORD   specV10a;
    DWORD   specV11;
    DWORD   writeTimeout;
    DWORD   flags;
    DWORD   drift;
    DWORD   resetGpio;
    DWORD   driverTimeout;

    HANDLE  hCom;

    HANDLE  hGpio;
    OMAP_SYSC_PADCONFS_REGS *pPadConfig;
	OMAP_SYSC_PADCONFS_WKUP_REGS *pPadConfig2;
    BOOL    opened;
    BOOL    exitDispatchThread;
    HANDLE  hDispatchThread;

    BT_POWER_MODE btPowerMode;

    BOOL    startRequest;
    REQUEST request;
    HANDLE  hRequestDoneEvent;

    UCHAR*  pWritePacket;
    DWORD   writePacketSize;

    HANDLE  hReadBufferEvent;
    HANDLE  hReadPacketEvent;
    UCHAR*  pReadBuffer;
    DWORD   readBufferSize;
    DWORD   readPacketSize;

    CRITICAL_SECTION writeCS;
    CRITICAL_SECTION readCS;

    HCI_TransportCallback pfnCallback;
} HCI_CONTEXT;

static HCI_CONTEXT g_hci;

//------------------------------------------------------------------------------
//  Device registry parameters
static const DEVICE_REGISTRY_PARAM g_deviceRegParams[] = {
    {
        L"Name", PARAM_STRING, FALSE, offset(HCI_CONTEXT, comPortName),
        0, L"COM2:"
    }, {
        L"Script", PARAM_STRING, FALSE, offset(HCI_CONTEXT, startScript),
        0, L"brf61_%d.%d.%d.bts"
    }, {
        L"Baud", PARAM_DWORD, FALSE, offset(HCI_CONTEXT, baud),
        fieldsize(HCI_CONTEXT, baud), (VOID*)115200
    }, {
        L"Priority256", PARAM_DWORD, FALSE, offset(HCI_CONTEXT, priority256),
        fieldsize(HCI_CONTEXT, priority256), (VOID*)120
    }, {
        L"WakeUpTimer", PARAM_DWORD, FALSE, offset(HCI_CONTEXT, wakeUpTimer),
        fieldsize(HCI_CONTEXT, wakeUpTimer), (VOID*)2000
    }, {
        L"CmdTimeout", PARAM_DWORD, FALSE, offset(HCI_CONTEXT, cmdTimeout),
        fieldsize(HCI_CONTEXT, cmdTimeout), (VOID*)8000
    }, {
        L"ResetDelay", PARAM_DWORD, FALSE, offset(HCI_CONTEXT, resetDelay),
        fieldsize(HCI_CONTEXT, resetDelay), (VOID*)300
    }, {
        L"SpecV10a", PARAM_DWORD, FALSE, offset(HCI_CONTEXT, specV10a),
        fieldsize(HCI_CONTEXT, specV10a), (VOID*)0
    }, {
        L"SpecV11", PARAM_DWORD, FALSE, offset(HCI_CONTEXT, specV11),
        fieldsize(HCI_CONTEXT, specV11), (VOID*)0
    }, {
        // This is the WriteTimeout used by HCI. It is different from the one used by the driver.
        // A better name for this parameter might have been CommandTimeout.
        L"WriteTimeout", PARAM_DWORD, FALSE, offset(HCI_CONTEXT, writeTimeout),
        fieldsize(HCI_CONTEXT, writeTimeout), (VOID*)HCI_DEFAULT_WRITE_TIMEOUT
    }, {
        L"Flags", PARAM_DWORD, FALSE, offset(HCI_CONTEXT, flags),
        fieldsize(HCI_CONTEXT, flags), (VOID*)0
    }, {
        L"Drift", PARAM_DWORD, FALSE, offset(HCI_CONTEXT, drift),
        fieldsize(HCI_CONTEXT, drift), (VOID*)HCI_DEFAULT_DRIFT
    }, {
        L"ResetGpio", PARAM_DWORD, FALSE, offset(HCI_CONTEXT, resetGpio),
        fieldsize(HCI_CONTEXT, resetGpio), (VOID*)-1
    }, {
        L"DriverTimeout", PARAM_DWORD, FALSE, offset(HCI_CONTEXT, driverTimeout),
        fieldsize(HCI_CONTEXT, driverTimeout), (VOID*)DEFAULT_PACKET_TIMEOUT
    }
};

//------------------------------------------------------------------------------
//  Function Definitions

//------------------------------------------------------------------------------
#if DBG_DUMPBUFFERS
void DumpBuffer(UCHAR *pBuffer, DWORD size)
{
    WCHAR szBuffer[64], *pszPos = szBuffer;
    DWORD count;

    count = 0;
    while (count < size) {
        if ((count & 0x0F) == 0) {
            wsprintf(szBuffer, L"%04x:", count);
            pszPos = &szBuffer[5];
        }
        wsprintf(pszPos, L" %02x", pBuffer[count]);
        pszPos += 3;
        if ((count & 0x0F) == 0x0F) {
            wsprintf(pszPos, L"\r\n");
            OutputDebugString(szBuffer);
        }
        count++;
    }
    if ((count & 0x0F) != 0) {
        wsprintf(pszPos, L"\r\n");
        OutputDebugString(szBuffer);
    }
}
#endif

//------------------------------------------------------------------------------
static void bt_enable_wake(CEDEVICE_POWER_STATE ps)
{
    switch(ps)
    {
        case D0:
        case D1:
        case D2:
		{
            if(g_hci.hGpio)
            {
                // Disable wake by CTS
				InterruptMask(BT_CTS_WAKEUP_SysIntr, 1);
                GPIOSetMode(g_hci.hGpio, UART2_CTS, GPIO_DIR_INPUT);
                GPIOClrBit(g_hci.hGpio, UART2_RTS);
            }
            break;
		}
        case D3:
        case D4:
		{
            if(g_hci.hGpio)
            {
                // Pullup RTS to disallow BT chip to send while we sleep
                GPIOSetMode(g_hci.hGpio, UART2_RTS, GPIO_DIR_OUTPUT );
                GPIOSetBit(g_hci.hGpio, UART2_RTS);

                GPIOSetMode(g_hci.hGpio, UART2_CTS, GPIO_DIR_INPUT | GPIO_INT_HIGH | GPIO_INT_LOW_HIGH );
                /* Enable wake CTS interrupt */
                InterruptMask(BT_CTS_WAKEUP_SysIntr, FALSE);
            }
            break;
		}
        default:
            break;
    }

    return;
}

static void Uart2GpioMode(HCI_CONTEXT *pHCI)
{        
	RETAILMSG(1, (L"HCI: Uart2GpioMode: %d, \r\n", BT_CTS_WAKEUP_SysIntr));
	// Enable GPIO Interrupt
	if (BT_CTS_WAKEUP_SysIntr != 0)
		InterruptMask(BT_CTS_WAKEUP_SysIntr, FALSE);
     		 	
    //Set RTS to GPIO_141
	OUTREG16(&g_hci.pPadConfig->CONTROL_PADCONF_MCBSP3_DR,   (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));  // GPIO_141
	if(g_hci.hGpio) { 
		GPIOSetMode(g_hci.hGpio, UART2_RTS, GPIO_DIR_OUTPUT );
		GPIOSetBit(g_hci.hGpio, UART2_RTS);
	}

	GPIOSetMode(g_hci.hGpio, UART2_CTS, GPIO_DIR_INPUT | GPIO_INT_HIGH | GPIO_INT_LOW_HIGH );	//GPIO_INT_LOW_HIGH
	// Mux Uart CTS-> GPIO_140
	RETAILMSG(1, (L"HCI: Uart2GpioMode: UART 2 GPIO \r\n")); 
	OUTREG16(&g_hci.pPadConfig->CONTROL_PADCONF_MCBSP3_DX,   (INPUT_ENABLE  | PULL_INACTIVE | MUX_MODE_4));  // GPIO_140
	// enable wake
	bt_enable_wake(D3);  
}

static void Gpio2UartMode(HCI_CONTEXT *pHCI)
{		
	// awake
	bt_enable_wake(D0);

	// Disable Interrupt, only when interrupt is already registered
	if (BT_CTS_WAKEUP_SysIntr != 0)
		InterruptMask(BT_CTS_WAKEUP_SysIntr, TRUE);
	if(g_hci.hGpio) 
		GPIOClrBit(g_hci.hGpio, UART2_RTS);

	//Give GPIO_141 back to RTS with Mode 0, pulldown, enabled     	
	OUTREG16(&g_hci.pPadConfig->CONTROL_PADCONF_MCBSP3_DR,   (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_1));  // UART2_RTS

	// Mux GPIO_140 -> UART CTS
	RETAILMSG(1, (L"HCI: Gpio2UartMode: GPIO 2 UART\r\n")); 
	OUTREG16(&g_hci.pPadConfig->CONTROL_PADCONF_MCBSP3_DX,   (INPUT_ENABLE  | PULL_INACTIVE | MUX_MODE_1));  // UART2_CTS
}

//------------------------------------------------------------------------------
static void BT_WakeupInterruptThread()
{
    DWORD TimeOut=INFINITE;
    TCHAR szPowerState[MAX_PATH];
    DWORD dwStateFlags;
    DWORD dwErr;

    while (!BT_ISRTerminating)
    {
        WaitForSingleObject(BT_CTS_WAKEUP_EVENT, TimeOut);
        if(BT_WakeupEnabled)
        {
            RETAILMSG(0, (L"***Wakeup IRQ***\r\n"));
            btSleep_to_Awake_transition = TRUE;
            bt_enable_wake(D0);
            dwErr = GetSystemPowerState(szPowerState,BT_HCI_SIZEOFARRAY(szPowerState),&dwStateFlags);
            if (ERROR_SUCCESS != dwErr)
            {
                RETAILMSG(TRUE, (TEXT("HCI:IST!GetSystemPowerState:ERROR:%d\n"), dwErr));
            }
            else
            {
                if((wcscmp(szPowerState,L"resuming")==0) || (wcscmp(szPowerState,L"useridle")==0)
			 || (wcscmp(szPowerState,L"backlightoff")==0))
                {
                    SetSystemPowerState(NULL,POWER_STATE_ON,POWER_FORCE);
                }		
            }
            BT_WakeupEnabled = FALSE;
	    
        }
        else
        {
            RETAILMSG(0, (L"***UART Mode :Wakeup IRQ***\r\n"));
            Sleep(10);
        }

        InterruptDone(BT_CTS_WAKEUP_SysIntr);
    }
}

//------------------------------------------------------------------------------
static BOOL BT_InitInterruptThread(void)
{
    BT_ISRTerminating = FALSE;
    BT_CTS_WAKEUP_EVENT = CreateEvent( NULL, FALSE, FALSE, NULL);

    DWORD i = BT_CTS_WAKEUP_IRQ;

    if (BT_CTS_WAKEUP_EVENT == NULL)
    {
        RETAILMSG(TRUE, (L"ERROR:Failed create interrupt event\r\n"));
    }

    if(g_hci.hGpio != NULL)
    {
        if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &i, sizeof(i), &BT_CTS_WAKEUP_SysIntr, sizeof(BT_CTS_WAKEUP_SysIntr), NULL))
        {
            RETAILMSG(TRUE, (L"BT_IntThread: IRQ -> SYSINTR translation failed\r\n"));
            return FALSE;
        }
        if (BT_CTS_WAKEUP_SysIntr == SYSINTR_UNDEFINED)
        { 
		    RETAILMSG(TRUE, (L"BT_IntThread: IRQ -> SYSINTR failed: get SYSINTR_UNDEFINED\r\n"));
		    return FALSE;
        }

        //Enable wakeup from BRF
        if(KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &BT_CTS_WAKEUP_SysIntr, sizeof( BT_CTS_WAKEUP_SysIntr ), NULL, 0, NULL))
		{
            RETAILMSG(TRUE, (L"set BRF as Wakeup source\r\n"));
		}
        InterruptInitialize(BT_CTS_WAKEUP_SysIntr, BT_CTS_WAKEUP_EVENT, NULL, 0);
    }

     BT_CTS_WAKEUP_THREAD  = CreateThread( NULL,
                                            0,
                                            (LPTHREAD_START_ROUTINE)BT_WakeupInterruptThread,
                                            NULL,
                                            0,
                                            NULL);
    if(BT_CTS_WAKEUP_THREAD == NULL)
    {
        RETAILMSG(TRUE, (L"BT_CTS_WAKEUP_THREAD failed\r\n"));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
static void BT_ReleaseInterruptThread(void)
{
    BT_ISRTerminating = TRUE;

    if(BT_CTS_WAKEUP_THREAD != NULL)
    {
        // Set event to wake it
        SetEvent(BT_CTS_WAKEUP_EVENT);
        // Wait until thread exits
        WaitForSingleObject(BT_CTS_WAKEUP_THREAD, INFINITE);
        CloseHandle(BT_CTS_WAKEUP_THREAD);
        BT_CTS_WAKEUP_THREAD = NULL;
    }

    // Release interrupt
	if (BT_CTS_WAKEUP_SysIntr != 0)
    {
		InterruptDisable(BT_CTS_WAKEUP_SysIntr);
		
		//Disable wakeup from BRF
		if(KernelIoControl(IOCTL_HAL_DISABLE_WAKE, &BT_CTS_WAKEUP_SysIntr, sizeof( BT_CTS_WAKEUP_SysIntr ), NULL, 0, NULL))
		{
			RETAILMSG(TRUE, (L"set BRF as Wakeup source\r\n"));
		}

        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &BT_CTS_WAKEUP_SysIntr, sizeof(BT_CTS_WAKEUP_SysIntr), NULL, 0, NULL);
        BT_CTS_WAKEUP_SysIntr = 0;
    }

    if(BT_CTS_WAKEUP_EVENT != NULL)
    {
        CloseHandle(BT_CTS_WAKEUP_EVENT);
        BT_CTS_WAKEUP_EVENT = NULL;
    }

}

//------------------------------------------------------------------------------
static BOOL
WriteData(
    HCI_CONTEXT *pHCI,
    UCHAR *pBuffer,
    DWORD size
    )
{
    DWORD count;

    while (size > 0)
    {
         if (!WriteFile(pHCI->hCom, pBuffer, size, &count, NULL)) break;

          if (count == 0)
            break;

          pBuffer += count;
          size -= count;
    }

    // Did we write all data?
    return (size == 0);
}

//------------------------------------------------------------------------------
static BOOL
ReadData(
    HCI_CONTEXT *pHCI,
    UCHAR *pBuffer,
    DWORD size
    )
{
    DWORD count, mask;

    while (size > 0)
    {
        if (!ReadFile(pHCI->hCom, pBuffer, size, &count, NULL)) break;
        // If there is no data to be read wait for read event...
        if (count == 0)
        {
            // We must get read event...
            do
            {
                WaitCommEvent(pHCI->hCom, &mask, NULL);
            }
            while (mask == 0);
        }
        else
        {
            pBuffer += count;
            size -= count;
        }
    }

    // Did we read all data?
    return (size == 0);
}

//------------------------------------------------------------------------------
//
//  Function:  SendPacketRequest
//
//  This function process REQUEST_SEND request. First it make sure that BT chip
//  is awake. Then it sends packet data to BT chip.
//
static VOID
SendPacketRequest(
    HCI_CONTEXT *pHCI
    )
{
    DWORD size;
    UCHAR *pPacket, cmd;

    // Before we send packet BT chip must be awake
    if (pHCI->btPowerMode == BT_SLEEP && btSleep_to_Awake_transition == FALSE)
    {
        BT_WakeupEnabled = FALSE;   //disable ISR
        bt_enable_wake(D0);
        // Set new mode...
        pHCI->btPowerMode = BT_SLEEP_TO_AWAKE;
        // Send packet...
        cmd = HCI_WAKE_UP_IND;
        if (!WriteData(pHCI, &cmd, sizeof(cmd)))
        {
            pHCI->exitDispatchThread = FALSE;
            RETAILMSG(0, (L"HCI: SendPacketRequest(): Send WakeUpInd failed\r\n"));
        }

        RETAILMSG(0, (L"HCI: SendPacketRequest(): HCILL is BT_SLEEP_TO_AWAKE, Send WakeUpInd finished\r\n"));

    }
    else if (pHCI->btPowerMode == BT_SLEEP && btSleep_to_Awake_transition == TRUE)
    {
        // Wait for UART to move into D0 state
        // Sooner or later we will get another WAKEUP_IND from BT chip again as first got missed because of D3-D0 Transition
        // and before it could retry the host also send WAKEUP_Ind and host state goes for toss as it won't get ACK back now
        // To avoid that condition a new flag is defined which will avoid that condition
        Sleep(1);
        RETAILMSG(0, (L"HCI: SendPacketRequest(): UART is in transition to D0, sleep(1)\r\n"));
    }
    else if (pHCI->btPowerMode != BT_AWAKE)
    {
        // Sooner or later we should get ACK from BT chip
        Sleep(1);
        RETAILMSG(0, (L"HCI: SendPacketRequest(): HCILL is Not BT_AWAKE, NOT BT_SLEEP, sleep(1)\r\n"));
    }
    else
    {
        RETAILMSG(0, (L"HCI: SendPacketRequest: HCILL is BT_AWAKE, send data %d\r\n", GetTickCount()));

        // Send packet over serial port
        pPacket = pHCI->pWritePacket;
        size = pHCI->writePacketSize;
        if (!WriteData(pHCI, pPacket, size))
        {
            pHCI->exitDispatchThread = FALSE;
        }

        RETAILMSG(0, (L"HCI: SendPacketRequest(): Send %02x\r\n", *pPacket));

        // We can release request
        pHCI->startRequest = FALSE;
        pHCI->request = REQUEST_NONE;
        pHCI->pWritePacket = NULL;
        pHCI->writePacketSize = 0;
        RETAILMSG(0, (L"HCI: SendPacketRequest: Sent %d\r\n", GetTickCount()));
        SetEvent(pHCI->hRequestDoneEvent);
    }
}
#ifdef _HOST_INITIATED_BT_SLEEP_OR_AWAKE_
//------------------------------------------------------------------------------
//
//  Function:  WakeUpRequest
//
//  This function starts BT chip wakeup sequence. This function is unused,
//  as the SendPacketRequest() sends the wake up request.
//
static VOID
WakeUpRequest(
    HCI_CONTEXT *pHCI
    )
{
    UCHAR cmd;
    DWORD size, count;

    switch (pHCI->btPowerMode)
    {
        case BT_AWAKE:
            // We are done...
            pHCI->startRequest = FALSE;
            pHCI->request = REQUEST_NONE;
            RETAILMSG(0, (L"HCI: WakeUpRequest(): SetEvent(hRequestDoneEvent)\r\n"));
            SetEvent(pHCI->hRequestDoneEvent);
            break;
        case BT_SLEEP:
            BT_WakeupEnabled = FALSE;   //disable ISR
            bt_enable_wake(D0);
            // Set new mode...
            pHCI->btPowerMode = BT_SLEEP_TO_AWAKE;
            // Send packet...
            cmd = HCI_WAKE_UP_IND;
            size = sizeof(cmd);
            while (size > 0)
            {
                if (!WriteFile(pHCI->hCom, &cmd, size, &count, NULL)) break;
                size -= count;
            }

           RETAILMSG(0, (L"HCI: WakeUpRequest(): Send WakeUpInd\r\n"));

            // We start request...
            pHCI->startRequest = FALSE;
            break;
        case BT_AWAKE_TO_SLEEP:
        case BT_SLEEP_TO_AWAKE:
            // Sooner or later we should get ACK from BT chip
            Sleep(1);
            break;
    }
}

//------------------------------------------------------------------------------
//
//  Function:  SleepRequest
//
//  This function starts BT chip sleep sequence. This function is unused, as the host can never initiate the sleep
//
static VOID
SleepRequest(
    HCI_CONTEXT *pHCI
    )
{
    UCHAR cmd;
    DWORD size, count;

    switch (pHCI->btPowerMode)
    {
        case BT_SLEEP:
            // We are done...
            pHCI->startRequest = FALSE;
            pHCI->request = REQUEST_NONE;
            RETAILMSG(0, (L"HCI:SleepRequest(): SetEvent(hRequestDoneEvent)\r\n"));
            SetEvent(pHCI->hRequestDoneEvent);
            break;
        case BT_AWAKE:
            // Set new mode...
            pHCI->btPowerMode = BT_AWAKE_TO_SLEEP;
            // Send packet...
            cmd = HCI_GO_TO_SLEEP_IND;
            size = sizeof(cmd);
            while (size > 0)
            {
                if (!WriteFile(pHCI->hCom, &cmd, size, &count, NULL)) break;
                size -= count;
            }

           RETAILMSG(0, (L"HCI:SleepRequest(): Send GoToSleepInd\r\n"));

            // We start request...
            pHCI->startRequest = FALSE;
            break;
        case BT_AWAKE_TO_SLEEP:
        case BT_SLEEP_TO_AWAKE:
            // Sooner or later we should get ACK from BT chip
            Sleep(1);
            break;
    }
}
#endif
//------------------------------------------------------------------------------
//
//  Function:  ReleaseBufferRequest
//
//  This function return zero packet if there is pending read request. It
//  is used from HCI_CloseConnection to release pending HCI_ReadPacket.
//
static VOID
ReleaseBufferRequest(
    HCI_CONTEXT *pHCI
    )
{
    if (WaitForSingleObject(pHCI->hReadBufferEvent, 100) == WAIT_OBJECT_0)
    {
        pHCI->readPacketSize = 0;
        pHCI->pReadBuffer = NULL;
        pHCI->readBufferSize = 0;
        PulseEvent(pHCI->hReadPacketEvent);
    }
#ifdef _FM_
    if(!pHCI->FMOpened)
    {
#endif
    // We are done...
    pHCI->startRequest = FALSE;
    pHCI->request = REQUEST_NONE;
    RETAILMSG(1, (L"HCI:ReleaseBufferRequest:\r\n"));
    SetEvent(pHCI->hRequestDoneEvent);
#ifdef _FM_
   }
#endif
}

//------------------------------------------------------------------------------
//
//  Function:  ProcessReceivePacket
//
//  This function receives packet from BT chip. When packet doesn't fit to
//  buffer (or buffer isn't ready) packet is trashed.
//
static VOID
ProcessReceivePacket(
    HCI_CONTEXT *pHCI,
    UCHAR packetType,
    DWORD headerSize
    )
{
    UCHAR *pBuffer;
    DWORD bufferSize, bodySize, packetSize;
    HCI_PACKET_HEADER *pHeader, header;
    DWORD size, count;
    BOOL ignorePacket = FALSE;


    // Start with zero packet length
    packetSize = 0;

    RETAILMSG(0, (L"HCI:ProcessReceivePacket(): waiting for pHCI->hReadBufferEvent \r\n"));
    // We must wait for read buffer
    WaitForSingleObject(pHCI->hReadBufferEvent, INFINITE);
    RETAILMSG(0, (L"HCI:ProcessReceivePacket(): Triggered for pHCI->hReadBufferEvent \r\n"));
    // We get buffer for packet
    pBuffer = pHCI->pReadBuffer;
    bufferSize = pHCI->readBufferSize;

    // Read packet header, use our header when buffer is too small
    if (headerSize > bufferSize)
    {
        pBuffer = (UCHAR*)&header;
        bufferSize = sizeof(header);
        ignorePacket = TRUE;
    }

    // We already have HCI packet type
    pHeader = (HCI_PACKET_HEADER*)pBuffer;
    pHeader->type = packetType;
    pBuffer += sizeof(packetType);

    // Read header
    if (!ReadData(pHCI, pBuffer, headerSize)) goto cleanUp;
    pBuffer += headerSize;

    // Get packet body size from header
    switch (packetType)
    {
        case EVENT_PACKET:
            bodySize = pHeader->event.length;
            break;
        case DATA_PACKET_ACL:
            bodySize = pHeader->acl.length;
            break;
        case DATA_PACKET_SCO:
            bodySize = pHeader->sco.length;
            break;
        default:
            // This should not happen...
            goto cleanUp;
    }

    // Check if packet fits to buffer
    size = sizeof(packetType) + headerSize + bodySize;
    if (ignorePacket || size > bufferSize)
    {
        while (bodySize > 0)
        {
            count = bodySize > THRASH_SIZE ? THRASH_SIZE : bodySize;
            if (!ReadData(pHCI, pBuffer, bodySize)) goto cleanUp;
            bodySize -= count;
        }
        goto cleanUp;
    }

    // Read packet body
    if (!ReadData(pHCI, pBuffer, bodySize)) goto cleanUp;

    // This is final packet size
    packetSize = sizeof(packetType) + headerSize + bodySize;

cleanUp:
    // Signal received packet
    if (!ignorePacket)
    {
        pHCI->readPacketSize = packetSize;
        pHCI->pReadBuffer = NULL;
        pHCI->readBufferSize = 0;
        RETAILMSG(0, (L"HCI:ProcessReceivePacket(): setting pHCI->hReadPacketEvent \r\n"));
        SetEvent(pHCI->hReadPacketEvent);
    }
}

//------------------------------------------------------------------------------
//
//  Function:  ProcessWakeUpInd
//
//  This function process HCI_WAKE_UP_IND pseudo-packet. BT chip indicates
//  it desided awake.
//
static VOID
ProcessWakeUpInd(
                 HCI_CONTEXT *pHCI
    )
{
    UCHAR reply;
    if ((pHCI->btPowerMode == BT_SLEEP) ||
        (pHCI->btPowerMode == BT_SLEEP_TO_AWAKE))
    {
        /* 
         * It is seen that the host may get wakeup ind while UARt is going
         * to D3 state, to avoid this race conidtion it is safe set the UART
         * state to D0 before the BT driver sends the wake up ack to BT chip.
         */
        /* Disable ISR handling */
        BT_WakeupEnabled = FALSE;

        bt_enable_wake(D0);

        /* Ack indication to BT chip */
        reply = HCI_WAKE_UP_ACK;
        if (!WriteData(pHCI, &reply, sizeof(reply)))
        {
            pHCI->exitDispatchThread = TRUE;
            ASSERT(FALSE);
            return;
        }

        RETAILMSG(0, (L"HCI: ProcessWakeUpInd(): Send WakeUpAck\r\n"));

        // Done...
        pHCI->btPowerMode = BT_AWAKE;
        // UART state transition done by now
        btSleep_to_Awake_transition = FALSE;
        //None can ever set this request. None can ever wait on this request. For future reference
#ifdef _HOST_INITIATED_BT_SLEEP_OR_AWAKE_
        if (pHCI->request == REQUEST_AWAKE)
        {
            pHCI->request = REQUEST_NONE;
            RETAILMSG(0, (L"HCI: ProcessWakeUpInd(): SetEven(hRequestDoneEvent)\r\n"));
            SetEvent(pHCI->hRequestDoneEvent);
        }
#endif
    }
    else
    {
             ASSERT(FALSE);
    }

    return;
}

//------------------------------------------------------------------------------
//
//  Function:  ProcessWakeUpAck
//
//  This function process HCI_WAKE_UP_ACK pseudo-packet. It indicates
//  BT chip accepted HCI_WAKE_UP_IND.
//
static VOID
ProcessWakeUpAck(
    HCI_CONTEXT *pHCI
    )
{
    ASSERT(pHCI->btPowerMode == BT_SLEEP_TO_AWAKE);

    pHCI->btPowerMode = BT_AWAKE;
    //None can ever set this request. None can ever wait on this request. For future reference
#ifdef _HOST_INITIATED_BT_SLEEP_OR_AWAKE_
    if (pHCI->request == REQUEST_AWAKE)
    {
        pHCI->request = REQUEST_NONE;
        RETAILMSG(0, (L"HCI: ProcessWakeUpAck(): SetEven(hRequestDoneEvent)\r\n"));
        SetEvent(pHCI->hRequestDoneEvent);
    }
#endif
}

//------------------------------------------------------------------------------
//
//  Function:  ProcessSleepInd
//
//  This function process HCI_GO_TO_SLEEP_IND pseudo-packet. BT chip indicates
//  it desided go to sleep mode.
//
static VOID
ProcessSleepInd(HCI_CONTEXT *pHCI)
{
    UCHAR reply;

    if ((pHCI->btPowerMode == BT_AWAKE) ||
        (pHCI->btPowerMode == BT_AWAKE_TO_SLEEP))
    {

        if (pHCI->btPowerMode == BT_AWAKE)
        {
            // Ack indication to BT chip
            reply = HCI_GO_TO_SLEEP_ACK;
            if (!WriteData(pHCI, &reply, sizeof(reply)))
            {
                pHCI->exitDispatchThread = TRUE;
                //IFDBG(DebugOut(DEBUG_ERROR, L"ERROR!HCI: ProcessSleepInd(): failed to call WriteData()\r\n"));
                ASSERT(FALSE);
                return;
            }

        RETAILMSG(0, (L"HCI: ProcessSleepInd(): Send GoToSleepAck\r\n"));

        }
        // Done...
        pHCI->btPowerMode = BT_SLEEP;

        bt_enable_wake(D3);

        BT_WakeupEnabled = TRUE;    //enable ISR

        //: None can ever set this request. None can ever wait on this request. For future reference
#ifdef _HOST_INITIATED_BT_SLEEP_OR_AWAKE_
        if (pHCI->request == REQUEST_SLEEP)
        {
            pHCI->request = REQUEST_NONE;
            RETAILMSG(0, (L"HCI: ProcessSleepInd(): SetEvent(hRequestDoneEvent)\r\n"));
            SetEvent(pHCI->hRequestDoneEvent);
        }
#endif
    }
    else
    {
        ASSERT(FALSE);
    }

    return;
}

#ifdef _HOST_INITIATED_BT_SLEEP_OR_AWAKE_
//------------------------------------------------------------------------------
//
//  Function:  ProcessSleepAck
//
//  This function process HCI_GO_TO_SLEEP_ACK pseudo-packet. It indicates
//  BT chip accepted HCI_GO_TO_SLEEP_IND. 
//   This function is not called as host cannot initiate the sleep request
//
static VOID
ProcessSleepAck(
    HCI_CONTEXT *pHCI
    )
{
    ASSERT(pHCI->btPowerMode == BT_AWAKE_TO_SLEEP);

    // Set BT chip power state
    pHCI->btPowerMode = BT_SLEEP;

    //Review : None can ever set this request. None can ever wait on this request. For future reference
    if (pHCI->request == REQUEST_SLEEP)
    {
        pHCI->request = REQUEST_NONE;
        RETAILMSG(0, (L"HCI: ProcessSleepAck(): SetEvent(hRequestDoneEvent)\r\n"));
        SetEvent(pHCI->hRequestDoneEvent);
    }
}
#endif

//------------------------------------------------------------------------------
//
//  Function:  DispatchThread
//
//  This is dispatch thread function. All communication with BT chip is made
//  from this thread. It serialize read/write communication with BT chip. But
//  because of buffered serial port driver there should not be a penalty at
//  all. On other side it simplify 4-wire BT chip protocol implementation.
//
//  Return value of the Dispatch thread was not used, so made it a void function.
static DWORD
DispatchThread(
    VOID* pContext
    )
{
    HCI_CONTEXT *pHCI = (HCI_CONTEXT*)pContext;
    UCHAR packetType;
    DWORD count, mask = 0;

    // While we are not asked to exit thread
    while (!pHCI->exitDispatchThread)
    {
        RETAILMSG(0,(TEXT("HCI: +DispatchThread\r\n")));

        // If there is pending request process it
        if (pHCI->startRequest)
        {
            switch (pHCI->request)
            {
                case REQUEST_SEND:
                    RETAILMSG(0, (L"HCI:DispatchThread: REQUEST_SEND\r\n"));
                    SendPacketRequest(pHCI);
                    break;
#ifdef _HOST_INITIATED_BT_SLEEP_OR_AWAKE_
//None can ever set this request. None can ever wait on this request. For future reference
                case REQUEST_AWAKE:
                    RETAILMSG(0, (L"HCI:DispatchThread: REQUEST_AWAKE\r\n"));
                    WakeUpRequest(pHCI);
                    break;
                case REQUEST_SLEEP:
                    RETAILMSG(0, (L"HCI:DispatchThread: REQUEST_SLEEP\r\n"));
                    SleepRequest(pHCI);
                    break;
#endif
                case REQUEST_RELEASE_BUFFER:
                    RETAILMSG(1, (L"HCI:DispatchThread: REQUEST_RELEASE_BUFFER\r\n"));
                    ReleaseBufferRequest(pHCI);
                    break;
                default:
                    pHCI->startRequest = FALSE;
                    pHCI->request = REQUEST_NONE;
                    RETAILMSG(0, (L"HCI:DispatchThread: REQUEST_NONE\r\n"));
                    break;
            }
        }

        RETAILMSG(0, (L"HCI:DispatchThread: pHCI->startRequest %d, pHCI->request %d\r\n",pHCI->startRequest,pHCI->request));

        do
        {
            // Try read packet type
            count = sizeof(packetType);
            if (!ReadFile(pHCI->hCom, &packetType, count, &count, NULL))
            {
                count = 0;
                break;
            }

            // When we don't get packet type, wait for event
            if (count == 0) WaitCommEvent(pHCI->hCom, &mask, NULL);

            // If mask is zero, thread is wakeup by request
            if (mask == 0) break;

        } while (count < sizeof(packetType));

        // If there is no packet type it is request...
        if (count == 0) continue;

        // Depending on packet type, call the functions. It is ordered according to frequency where 
        //the event_packet has highest frequency, then ACL, WAKE_UP_IND for responsiveness, and then SLEEP_IND 
        //and WAKE_UP_ACK. SCO packets are rarely received.
        switch (packetType)
        {
            case HCI_WAKE_UP_IND:

                RETAILMSG(0, (L"HCI:DispatchThread: Recv WakeUpInd\r\n"));

                ProcessWakeUpInd(pHCI);
                break;
            case HCI_GO_TO_SLEEP_IND:

                RETAILMSG(0, (L"HCI:DispatchThread: Recv GoToSleepInd\r\n"));

                ProcessSleepInd(pHCI);
                break;
            case EVENT_PACKET:

                RETAILMSG(0, (L"HCI:DispatchThread: Recv %02x\r\n", packetType));

                ProcessReceivePacket(
                    pHCI, packetType, sizeof(HCI_EVENT_PACKET_HEADER)
                    );

                break;

            case DATA_PACKET_ACL:

                 RETAILMSG(0, (L"HCI:DispatchThread: Recv %02x\r\n", packetType));

                ProcessReceivePacket(
                    pHCI, packetType, sizeof(HCI_ACL_PACKET_HEADER)
                    );
                break;

            case HCI_WAKE_UP_ACK:

                RETAILMSG(0, (L"HCI:DispatchThread: Recv WakeUpAck\r\n"));

                ProcessWakeUpAck(pHCI);
                break;

            case DATA_PACKET_SCO:

                RETAILMSG(0, (L"HCI:DispatchThread: Recv %02x\r\n", packetType));

                ProcessReceivePacket(
                    pHCI, packetType, sizeof(HCI_SCO_PACKET_HEADER)
                    );
                break;
            
#ifdef _HOST_INITIATED_BT_SLEEP_OR_AWAKE_  
//None can ever set this request. None can ever wait on this request. For future reference
            case HCI_GO_TO_SLEEP_ACK:

                RETAILMSG(0, (L"HCI:DispatchThread: Recv GoToSleepAck\r\n"));

                ProcessSleepAck(pHCI);
                break;
#endif
                  
            default:
                ASSERT(FALSE);

                RETAILMSG(0, (L"ERROR! HCI:DispatchThread: Recv %02x\r\n", packetType));

                pHCI->exitDispatchThread = TRUE;
                break;
            }

        }
     return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
// Return Value of the function was not used, so changed the return type to void.
static void
ExecuteRequest(
    HCI_CONTEXT *pHCI,
    REQUEST request
    )
{
    // One request at time...
    EnterCriticalSection(&pHCI->writeCS);

    ASSERT(!pHCI->startRequest && pHCI->request == REQUEST_NONE);

    // Set request
    pHCI->request = request;
    pHCI->startRequest = TRUE;

    // This should release WaitCommMask
    SetCommMask(pHCI->hCom, EV_RXCHAR);

    // Wait until request is done (should we timeout?)...
    WaitForSingleObject(pHCI->hRequestDoneEvent, INFINITE);

    // We are done...
    LeaveCriticalSection(&pHCI->writeCS);

}

//------------------------------------------------------------------------------
static BOOL
WritePacket(
    HCI_CONTEXT *pHCI,
    UCHAR *pBuffer,
    DWORD size,
    DWORD timeout
    )
{
    BOOL rc = FALSE;

    EnterCriticalSection(&pHCI->writeCS);

    // Let do some checks
    ASSERT(pHCI->pWritePacket == NULL);
    ASSERT(!pHCI->startRequest && pHCI->request == REQUEST_NONE);

    // Submit request to dispatch thread
    pHCI->pWritePacket = pBuffer;
    pHCI->writePacketSize = size;
    pHCI->request = REQUEST_SEND;
    pHCI->startRequest = TRUE;

    // This should release WaitCommMask
    SetCommMask(pHCI->hCom, EV_RXCHAR);

#if DBG_DUMPBUFFERS
	DumpBuffer( pBuffer, size );
#endif
    // Wait until request is done...
    DWORD dwWait = WaitForSingleObject(pHCI->hRequestDoneEvent, timeout);


    if (dwWait != WAIT_OBJECT_0)
    {
        switch (dwWait)
        {
            case WAIT_FAILED:
                RETAILMSG(1, (L"ERROR: HCI!WritePacket: "
                    L"WaitForSingleObject failed on hRequestDoneEvent (0x%08x)\r\n",
                    GetLastError()
                    ));
                break;
            case WAIT_TIMEOUT:
                RETAILMSG(1, (L"ERROR: HCI!WritePacket: "
                    L"WaitForSingleObject timeout on hRequestDoneEvent\r\n"
                    ));
                break;
        }

        goto cleanUp;
    }

    // Done...
    rc = TRUE;

cleanUp:
    LeaveCriticalSection(&pHCI->writeCS);
    return rc;
}

//------------------------------------------------------------------------------
static DWORD
ReadPacket(
    HCI_CONTEXT *pHCI,
    UCHAR *pBuffer,
    DWORD size,
    DWORD timeout
    )
{
    DWORD rc;

    EnterCriticalSection(&pHCI->readCS);
    // Submit packet to receive thread
    ASSERT(pHCI->pReadBuffer == NULL);
    pHCI->pReadBuffer = pBuffer;
    pHCI->readBufferSize = size;
    if (!SetEvent(pHCI->hReadBufferEvent))
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!ReadPacket: "
            L"SetEvent failed on hReadBufferEvent (0x%08x)\r\n",
            GetLastError()
            ));
        goto cleanUp;
    }
     RETAILMSG(0,(TEXT("HCI ReadPacket(): set pHCI->hReadBufferEvent\r\n")));
    // Wait for packet
    rc = WaitForSingleObject(pHCI->hReadPacketEvent, timeout);
    switch (rc)
    {
        case WAIT_FAILED:
            IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!ReadPacket: "
                L"WaitForSingleObject failed on hReadPacketEvent (0x%08x)\r\n",
                GetLastError()
                ));
            size = 0;
            break;
        case WAIT_TIMEOUT:
            IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!ReadPacket: "
                L"WaitForSingleObject timeout on hReadPacketEvent \r\n"
                ));
            size = 0;
            break;
        case WAIT_OBJECT_0:
            // Get received packet size
            size = pHCI->readPacketSize;
            break;
    }

    // Clear buffer from HCI context (just to catch errors)
    pHCI->pReadBuffer = NULL;
    pHCI->readBufferSize = 0;
    pHCI->readPacketSize = 0;

cleanUp:
    LeaveCriticalSection(&pHCI->readCS);
    return size;
}

//------------------------------------------------------------------------------
static BOOL
RunScript(
    HCI_CONTEXT *pHCI,
    LPCWSTR fileName
    )
{
    BOOL rc = FALSE;
    HANDLE hFile= INVALID_HANDLE_VALUE;
    SCRIPT_HEADER header;
    SCRIPT_ACTION_HEADER action;
    ACTION_SERIAL_PORT_PARAMS *pSerialParams;
    ACTION_DELAY_PARAMS *pDelayParams;
    DCB dcb;
    UCHAR buffer[MAX_PATH * sizeof(WCHAR)];
    DWORD count;

    RETAILMSG(1,(TEXT("+RunScript:%s\n"),fileName));
    // Add path to file name and convert to unicode...
    if (FAILED(StringCbPrintf((LPWSTR)buffer, sizeof(buffer), L"Windows\\%s",
            fileName)))
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!RunScript: "
            L"Failed extend file name '%s'\r\n", fileName
            ));
        goto cleanUp;
    }

    // Open script file, exit when it is missing
    hFile = CreateFile(
        (LPWSTR)buffer, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL
        );
    if (hFile == INVALID_HANDLE_VALUE)
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!RunScript: "
            L"Failed open script file name '%s'\r\n", buffer
            ));
        RETAILMSG(1,(TEXT("Failed open script file name '%s'\r\n"),buffer));
        goto cleanUp;
    }

    // Read script header
    if (!ReadFile(hFile, &header, sizeof(header), &count, NULL) ||
        (count < sizeof(header)))
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!RunScript: "
            L"Failed read header from script file\r\n"
            ));
        RETAILMSG(1,(TEXT("Failed read header from script file\r\n")));
        goto cleanUp;
    }

    // Header magic must match
    if (header.magicNumber != SCRIPT_HEADER_MAGIC)
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!RunScript: "
            L"Script magic number/cookie %08x doesn't match\r\n"
            ));
        goto cleanUp;
    }

    while (TRUE)
    {
        // Read action header
        if (!ReadFile(hFile, &action, sizeof(action), &count, NULL))
        {
            IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!RunScript: "
                L"Failed read action from script file\r\n"
                ));
            RETAILMSG(1,(TEXT("Failed read action from script file \r\n")));
            goto cleanUp;
        }

        // If we didn't get full action header assume end of script
        if (count < sizeof(action))
        { 
            RETAILMSG(1,(TEXT("HCI: RunScript(): reach the end of script\r\n"))); 
			break;
		}

        // Make sure that parameters fit to buffer
        if (action.size > sizeof(buffer))
        {
            IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!RunScript: "
                L"Action parameters size too big (%d)\r\n", action.size
                ));
            goto cleanUp;
        }

        // Read buffer
            if (!ReadFile(hFile, buffer, action.size, &count, NULL) || (count < action.size))
            {
            IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!RunScript: "
                L"Failed read action parameters (read %d from %d)\r\n",
                count, action.size
                ));
            RETAILMSG(1,(TEXT("Failed read action parameters  \r\n")));
            goto cleanUp;
            }

        switch (action.code)
        {
            case ACTION_SEND_COMMAND:
                // Write packet
                RETAILMSG(0,(TEXT("HCI: RunScript(): ACTION_SEND_COMMAND\r\n")));
                if (!WritePacket(pHCI, buffer, count, pHCI->driverTimeout))
                {
                        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!RunScript: " L"Failed write HCI packet to Bluetooth device\r\n"));
                        RETAILMSG(1,(TEXT("Failed write HCI packet to Bluetooth device  \r\n")));
                        goto cleanUp;
                }
                count = ReadPacket(pHCI, buffer, sizeof(buffer), pHCI->driverTimeout);
                if (count == 0)
                {
                        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!RunScript: "L"Failed read HCI packet from Bluetooth device\r\n"));
                    RETAILMSG(1,(TEXT("Failed read HCI packet from Bluetooth device  \r\n")));
                        goto cleanUp;
                }
                RETAILMSG(0,(TEXT("HCI RunScript(): Response from BT module:")));
                //DumpBuffer( buffer, count);
                break;

            case ACTION_SERIAL_PORT:
                // Check action parameters
                RETAILMSG(0,(TEXT("HCI: RunScript(): ACTION_SERIAL_PORT\r\n")));
                if (count < sizeof(ACTION_SERIAL_PORT_PARAMS))
                {
                    IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!RunScript: "
                        L"Parameters for ACTION_SERIAL_PORT too small (%d)\r\n",
                        count
                        ));
                    RETAILMSG(1,(TEXT("Parameters for ACTION_SERIAL_PORT too small  \r\n")));

                    goto cleanUp;
                }
                pSerialParams = (ACTION_SERIAL_PORT_PARAMS*)&buffer[0];
                // Get actual serial port parameters
                dcb.DCBlength = sizeof(dcb);
                if (!GetCommState(pHCI->hCom, &dcb))
                {
                    IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!RunScript: "
                        L"Failed get serial port parameters\r\n"
                        ));
                    goto cleanUp;
                }
                // Update the communication parameters structure
                if (pSerialParams->baudRate != DONT_CHANGE)
                {
                    dcb.BaudRate = pSerialParams->baudRate;
                }
                if (pSerialParams->flowControl != DONT_CHANGE)
                {

                    switch (pSerialParams->flowControl)
                    {
                        case FCT_NONE:
                            dcb.fOutxCtsFlow = 0;
                            dcb.fRtsControl = RTS_CONTROL_ENABLE;
                            break;
                        case FCT_HARDWARE:
                            dcb.fOutxCtsFlow = 1;
                            dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
                            break;

                    }

                }
                // Update
                if (!SetCommState(pHCI->hCom, &dcb))
                {
                    IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!RunScript: "
                        L"Failed set serial port parameters\r\n"
                        ));
            RETAILMSG(1,(TEXT("Failed set serial port parameters \r\n")));

                    goto cleanUp;
                }
                break;

            case ACTION_DELAY:
                // Check action parameters
                RETAILMSG(0,(TEXT("HCI: RunScript(): ACTION_DELAY\r\n")));
                if (count < sizeof(ACTION_DELAY_PARAMS))
                {
                    IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!RunScript: "
                        L"Parameters for ACTION_SERIAL_PORT too small (%d)\r\n",
                        count
                        ));
                        RETAILMSG(1,(TEXT("Parameters for ACTION_SERIAL_PORT too small\r\n")));

                    goto cleanUp;
                }
                pDelayParams = (ACTION_DELAY_PARAMS*)&buffer[0];
                // Do it...
                Sleep(pDelayParams->timeToWait);
                break;

            case ACTION_RUN_SCRIPT:
                // Convert name to unicode
                RETAILMSG(1,(TEXT("HCI: RunScript(): ACTION_RUN_SCRIPT\r\n")));
                if (action.size < sizeof(buffer))
                {
                //  Added fialure Handler.
                if (FAILED(StringCbPrintf(
                    (LPWSTR)(buffer + action.size),
                        sizeof(buffer) - action.size, L"%hs", buffer)))
                {
                        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!RunScript: "
                           L"Failed ACTION_RUN_SCRIPT \r\n"));
                        goto cleanUp;
                }
                // Recourse
                if (!RunScript(pHCI, (LPWSTR)(buffer + action.size)))
                {
                    goto cleanUp;
                }
                }       
                break;

            case ACTION_REMARKS:
            case ACTION_WAIT_EVENT:
            default:
                RETAILMSG(0,(TEXT("HCI: RunScript(): ACTION_REMARKS or ACTION_WAIT_EVENT or others \r\n")));
                break;
        }
    }

    // Done
    rc = TRUE;

cleanUp:
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

    RETAILMSG(1,(TEXT("HCI: -RunScript(): %d\r\n"), rc));
    return rc;
}

//------------------------------------------------------------------------------
static BOOL
StartScript(
    HCI_CONTEXT *pHCI
    )
{
    BOOL rc = FALSE;
    HCI_PACKET_HEADER header;
    UCHAR buffer[32];
    WCHAR fileName[64];
    HCI_PACKET_HEADER *pHeader = (HCI_PACKET_HEADER*)buffer;
    USHORT manufacturerID, version;
    USHORT projectType, minorVersion, majorVersion;
  
    // First get BT version
    header.type = COMMAND_PACKET;
    header.command.code = HCI_READ_LOCAL_VERSION_INFORMATION;
    header.command.length = 0;
    if (!WritePacket(
            pHCI, (UCHAR*)&header,
            sizeof(UCHAR) + sizeof(HCI_COMMAND_PACKET_HEADER), pHCI->driverTimeout
            ))
    {
        goto cleanUp;
    }

    // Get response...
	//  The return value of the function stored in a temp variable.
    if ((ReadPacket(pHCI, buffer, sizeof(buffer), pHCI->driverTimeout) == 0) ||
        (pHeader->type != EVENT_PACKET) ||
        (pHeader->event.code != COMMAND_COMPLETE_EVENT_CODE) ||
        (pHeader->event.length < 12))
    {
        goto cleanUp;
    }

    // Check for TI's id
    manufacturerID = MAKEWORD(buffer[11], buffer[12]);
    if (manufacturerID != TI_MANUFACTURER_ID)
    {
        goto cleanUp;
    }

    // Get project & versions
    version = MAKEWORD(buffer[13], buffer[14]);
    projectType  = (version & 0x7C00) >> 10;
    minorVersion = (version & 0x007F);
    majorVersion = (version & 0x0380) >> 7;
    if ((version & 0x8000) != 0) majorVersion |= 0x0008;

    // Create file name
    // Added failure Handler.
    if (FAILED(StringCbPrintf(
        fileName, sizeof(fileName), pHCI->startScript,
        projectType, majorVersion, minorVersion
        )))
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI!StartScript: "
            L"Failed extend file name \r\n"));
        goto cleanUp;
    }

	RETAILMSG(TRUE, (L"TiWi firmware: %s\r\n", pHCI->startScript)); 
    // Run script engine...
    if (!RunScript(pHCI, fileName))
       goto cleanUp;

    // Wait for while to receive "HCILL_GOTO_SLEEP_IND_MSG" from BRF
    Sleep(3);       // if the delays is too long, OS will send HCI_RESET cmd to reset HCI trans layer

    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
void
StartHardware(void)
{
	RETAILMSG(1, (L"StartHardware:\r\n"));
	if(g_hci.hGpio != NULL) {
		RETAILMSG(1, (L"StartHardware: BT_EN\r\n"));

		GPIOSetMode(g_hci.hGpio, BT_GPIO_SHUTDOWN, GPIO_DIR_OUTPUT );
		GPIOSetBit(g_hci.hGpio, BT_GPIO_SHUTDOWN);
		Sleep(150);

		//// Disable Interrupt, only when interrupt is already registered
		if (BT_CTS_WAKEUP_SysIntr != 0)
			InterruptMask(BT_CTS_WAKEUP_SysIntr, TRUE);

		GPIOClrBit(g_hci.hGpio, UART2_RTS);

		// GPIO_141 back to RTS     	
		OUTREG16(&g_hci.pPadConfig->CONTROL_PADCONF_MCBSP3_DR,   (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_1));  // UART2_RTS

		//GPIO_140 -> UART CTS
		OUTREG16(&g_hci.pPadConfig->CONTROL_PADCONF_MCBSP3_DX,   (INPUT_ENABLE  | PULL_INACTIVE | MUX_MODE_1));  // GPIO_140
	}

	// Set UART to Power D0 - awake
	if (g_hci.hCom != NULL) 
	{
		Gpio2UartMode(&g_hci);
		bt_enable_wake(D0);
	}
}

//------------------------------------------------------------------------------
//  Failure case hhandled, and return value chnaged during failure
int HCI_StartHardware(void)
{
	RETAILMSG(1, (L"HCI_StartHardware\r\n"));
	
	StartHardware();

	if (! g_hci.pfnCallback) {
        IFDBG(DebugOut (DEBUG_HCI_INIT, L"-HCI_StartHardware (not registered)\n"));
        return FALSE;
    }

    return ERROR_SUCCESS == g_hci.pfnCallback (DEVICE_UP, NULL);
}

//------------------------------------------------------------------------------
void
StopHardware(void)
{
	RETAILMSG(1, (L"StopHardware:\r\n"));
	if(g_hci.hGpio != NULL) {
		RETAILMSG(1, (L"StopHardware: BT_DIS\r\n"));
		GPIOSetMode(g_hci.hGpio, BT_GPIO_SHUTDOWN, GPIO_DIR_OUTPUT );
		GPIOClrBit(g_hci.hGpio, BT_GPIO_SHUTDOWN);
		StallExecution(150*1000);

		//Set RTS to GPIO_141
		OUTREG16(&g_hci.pPadConfig->CONTROL_PADCONF_MCBSP3_DR,   (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));  // GPIO_141
		GPIOSetMode(g_hci.hGpio, UART2_RTS, GPIO_DIR_OUTPUT );
		GPIOSetBit(g_hci.hGpio, UART2_RTS);

		GPIOSetMode(g_hci.hGpio, UART2_CTS, GPIO_DIR_INPUT | GPIO_INT_HIGH | GPIO_INT_LOW_HIGH );
		// Mux Uart CTS-> GPIO_140
		RETAILMSG(1, (L"StopHardware: CTS to GPIO\r\n"));
		OUTREG16(&g_hci.pPadConfig->CONTROL_PADCONF_MCBSP3_DX,   (INPUT_ENABLE  | PULL_INACTIVE | MUX_MODE_4));  // GPIO_140
	}

	// Set UART to Power D3 - sleep
	if (g_hci.hCom != NULL)
	{
		Uart2GpioMode(&g_hci);
		RETAILMSG(1, (L"StopHardware: Suspend Serial port\r\n"));
		bt_enable_wake(D3);
	}
}

//------------------------------------------------------------------------------
//  Failure case hhandled, and return value chnaged during failure
int HCI_StopHardware(void)
{
	RETAILMSG(1, (L"+HCI_StopHardware\r\n"));

	StopHardware();

	return ERROR_SUCCESS == g_hci.pfnCallback (DEVICE_DOWN, NULL);
}

//------------------------------------------------------------------------------
int
HCI_SetCallback(
    HCI_TransportCallback pfnCallback
    )
{
    g_hci.pfnCallback = pfnCallback;

    if (pfnCallback != NULL)
    {
       DebugInit();
    }
    else
    {
        DebugDeInit();
    }
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
int
HCI_OpenConnection(
    )
{
    int rc = FALSE;
    COMMTIMEOUTS commTimeOuts;
    DCB dcb;
    PHYSICAL_ADDRESS pa;

    IFDBG(DebugOut(DEBUG_HCI_INIT, L"+HCI_OpenConnection\r\n"));
    RETAILMSG(TRUE, (L" +++ HCI_OpenConnection +++\r\n"));

	// Cleanup 
	ZeroMemory(&g_hci, sizeof(g_hci));
	BT_CTS_WAKEUP_SysIntr = 0;
	BT_WakeupEnabled = FALSE;
	
    // Read parameters from registry
    if (GetDeviceRegistryParams(
            L"Software\\Microsoft\\Bluetooth\\Transports\\BuiltIn\\1", &g_hci,
            dimof(g_deviceRegParams), g_deviceRegParams
            ) != ERROR_SUCCESS)
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI_OpenConnection: "
            L"Failed read HCI registry parameters\r\n"
            ));
        RETAILMSG(TRUE, (L" ERROR:- Failed read HCI registry parameters \r\n"));
        goto cleanUp;
    }


    RETAILMSG(1, (L"Opening port %s (baudrate %d)\r\n", g_hci.comPortName, g_hci.baud));

    g_hci.hCom = CreateFile(
        g_hci.comPortName, GENERIC_READ|GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
        );

    if (g_hci.hCom == INVALID_HANDLE_VALUE)
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI_OpenConnection: "
            L"Failed open UART HCI interface (GetLastError() = 0x%08x)\r\n",
            GetLastError()
            ));
        g_hci.hCom = NULL;
        goto cleanUp;
    }

	// Set UART to Power D0 - awake
	bt_enable_wake(D0);

    //Open GPIO device and start hardware
    pa.QuadPart = OMAP_SYSC_PADCONFS_REGS_PA;
    g_hci.pPadConfig = (OMAP_SYSC_PADCONFS_REGS *)MmMapIoSpace(pa, sizeof(OMAP_SYSC_PADCONFS_REGS), FALSE);
	pa.QuadPart = OMAP_SYSC_PADCONFS_WKUP_REGS_PA;
    g_hci.pPadConfig2 = (OMAP_SYSC_PADCONFS_WKUP_REGS *)MmMapIoSpace(pa, sizeof(OMAP_SYSC_PADCONFS_WKUP_REGS), FALSE);
    if(g_hci.pPadConfig == NULL || g_hci.pPadConfig2 == NULL) {
    	RETAILMSG(1, (L" OMAP2430 BRF6300! OMAP2430_PADCONFS1_REGS:Failed to map PADCONFS1 registers (pa = 0x%08x)\r\n", pa.LowPart));
        goto cleanUp; 
    }

    //set MUX register: Mode 0, Pulldown, enabled
	MASKREG32(&(g_hci.pPadConfig2->CONTROL_PADCONF_SYS_32K), BIT20|BIT19|BIT18|BIT17|BIT16, BIT19); 



    g_hci.hGpio = GPIOOpen();
    if(g_hci.hGpio == NULL)
    {
        RETAILMSG(1, (L"ERROR: HCI_OpenConnection: "L"Failed open GPIO interface (GetLastError() = 0x%08x)\r\n",GetLastError()));
        goto cleanUp;
    }

	GPIOSetMode(g_hci.hGpio, UART2_CTS, GPIO_DIR_INPUT | GPIO_INT_HIGH | GPIO_INT_LOW_HIGH );
	RETAILMSG(0,(TEXT("mode: 0x%x\r\n"),GPIOGetMode(g_hci.hGpio,UART2_CTS)));

	// Enable hardware
    StartHardware();


    // Purge any information in the buffer
    if (!PurgeComm(
            g_hci.hCom, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR
        ))
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI_OpenConnection: "
            L"PurgeComm failed on HCI UART (GetLastError() = 0x%08x)\r\n",
            GetLastError()
            ));
        goto cleanUp;
    }

    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = g_hci.baud;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fOutxCtsFlow = TRUE;
    dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fTXContinueOnXoff = FALSE;
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;
    dcb.fAbortOnError = TRUE;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.XonChar = 0x11;
    dcb.XoffChar = 0x13;
	// value divided by 10 due to a mdd limitation.
    dcb.XonLim = 300;
    dcb.XoffLim = 900;
    if (!SetCommState(g_hci.hCom, &dcb))
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI_OpenConnection: "
            L"SetCommState failed on HCI UART (GetLastError() = 0x%08x)\r\n",
            GetLastError()
            ));
        goto cleanUp;
    }


    // Set serial port timeouts. We want don't want to wait
    // in ReadFile (instead we will use WaitCommEvent).
    commTimeOuts.ReadIntervalTimeout = MAXDWORD;
    commTimeOuts.ReadTotalTimeoutMultiplier = MAXDWORD;
    commTimeOuts.ReadTotalTimeoutConstant = 0;
    commTimeOuts.WriteTotalTimeoutMultiplier = 1;
    commTimeOuts.WriteTotalTimeoutConstant = 100;
    if (!SetCommTimeouts(g_hci.hCom, &commTimeOuts))
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI_OpenConnection: "
            L"SetCommTimeouts failed on HCI UART (GetLastError() = 0x%08x)\r\n",
            GetLastError()
            ));
        goto cleanUp;
    }

    // Dispatch thread uses comm events
    SetCommMask(g_hci.hCom, EV_RXCHAR);

    //Setup wakeup mode
    if(!BT_InitInterruptThread())
        goto cleanUp;

    // No read buffer, write packet or request
    g_hci.pReadBuffer = NULL;
    g_hci.readBufferSize = 0;
    g_hci.readPacketSize = 0;
    g_hci.pWritePacket = NULL;
    g_hci.writePacketSize = 0;
    g_hci.startRequest = FALSE;
    g_hci.request = REQUEST_NONE;

    // We expect BT is awake now
    g_hci.btPowerMode = BT_AWAKE;
    // Added by NBS 
	//g_hci.btPowerMode = BT_SLEEP;
	

    InitializeCriticalSection(&g_hci.writeCS);
    InitializeCriticalSection(&g_hci.readCS);
    g_hci.hReadBufferEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_hci.hReadBufferEvent == NULL)
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI_OpenConnection: "
            L"CreateEvent failed (GetLastError() = 0x%08x)\r\n",
            GetLastError()
            ));
        goto cleanUp;
    }

    g_hci.hReadPacketEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_hci.hReadPacketEvent == NULL)
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI_OpenConnection: "
            L"CreateEvent failed (GetLastError() = 0x%08x)\r\n",
            GetLastError()
            ));
        goto cleanUp;
    }

    g_hci.hRequestDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_hci.hRequestDoneEvent == NULL)
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI_OpenConnection: "
            L"CreateEvent failed (GetLastError() = 0x%08x)\r\n",
            GetLastError()
            ));
        goto cleanUp;
    }

    // Start dispatch thread
    g_hci.exitDispatchThread = FALSE;
    g_hci.hDispatchThread = CreateThread(
        NULL, 0, DispatchThread, &g_hci, 0, NULL
        );
    if (g_hci.hDispatchThread == NULL)
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI_OpenConnection: "
            L"CreateThread failed (GetLastError() = 0x%08x)\r\n",
            GetLastError()
            ));
        goto cleanUp;
    }

    // Set read thread priority
    CeSetThreadPriority(g_hci.hDispatchThread, g_hci.priority256);
#if 0
	HCI_PACKET_HEADER header;
    UCHAR buffer[32];
	HCI_PACKET_COMMAND_COMPLETE_EVENT *pHeader = (HCI_PACKET_COMMAND_COMPLETE_EVENT*)buffer;

	RETAILMSG(TRUE, (L"*** INIT ***\r\n"));
	// First get BT version
    header.type = COMMAND_PACKET;
	header.commandWithParams.header.code = HCI_VS_SET_PCM_LOOPBACK_ENABLE;
    header.commandWithParams.header.length = 1;
	header.commandWithParams.params.par0 = 0;
    if (!WritePacket(
            &g_hci, (UCHAR*)&header,
            sizeof(UCHAR) + sizeof(HCI_COMMAND_PACKET_HEADER) + sizeof(UINT8), g_hci.driverTimeout
            ))
    {
        RETAILMSG(TRUE, (L"WritePacket fail\r\n"));
    }

    // Get response...
	//  The return value of the function stored in a temp variable.
    if ((ReadPacket(&g_hci, buffer, sizeof(buffer), g_hci.driverTimeout) == 0) ||
        (pHeader->type != EVENT_PACKET) ||
		(pHeader->eventCode != COMMAND_COMPLETE_EVENT_CODE) ||
        (pHeader->status > 0))
    {
        RETAILMSG(TRUE, (L"ReadPacket fail\r\n"));
    }
#endif

    // Done
    rc = TRUE;
    g_hci.opened = TRUE;
cleanUp:
    if (!rc) HCI_CloseConnection();
    RETAILMSG(TRUE, (L" --- HCI_OpenConnection ---\r\n"));
    IFDBG(DebugOut(DEBUG_HCI_INIT, L"-HCI_OpenConnection(%d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
VOID
HCI_CloseConnection(
    )
{
    RETAILMSG(TRUE, (L" +++ HCI_CloseConnection +++\r\n"));

    // Dispatch thread must have higher priority
    ASSERT(CeGetThreadPriority(GetCurrentThread()) > (int)g_hci.priority256);

    IFDBG(DebugOut(DEBUG_HCI_INIT, L"+HCI_CloseConnection\r\n"));

    // If hComm is NULL we don't open connection at all
    if (g_hci.hCom == NULL) goto cleanUp;

    // Stop the BT hardware
    StopHardware();

   SetEvent(g_hci.hReadPacketEvent);
   SetEvent(g_hci.hRequestDoneEvent);

    // If connection is opened, we need to start close connection actions
    if (g_hci.opened)
    {
        // Send request for buffer release, it will release HCI_ReadPacket
        ExecuteRequest(&g_hci, REQUEST_RELEASE_BUFFER);
    }

    RETAILMSG(1, (L" HCI_CloseConnection: Closing Wakeup mode +\r\n"));
    //close wakeup mode
   BT_ReleaseInterruptThread();

    // Set exit flag before we close COM port
    g_hci.exitDispatchThread = TRUE;

    // Close COM port
    CloseHandle(g_hci.hCom);
    g_hci.hCom = NULL;

    RETAILMSG(1, (L" HCI_CloseConnection: Closed Wakeup mode & closed com port +\r\n"));

    // If thread exist wait until it exists
    if (g_hci.hDispatchThread != NULL)
    {
        WaitForSingleObject(g_hci.hDispatchThread, INFINITE);
        CloseHandle(g_hci.hDispatchThread);
        g_hci.hDispatchThread = NULL;
    }
    if (g_hci.hReadBufferEvent != NULL)
    {
        CloseHandle(g_hci.hReadBufferEvent);
        g_hci.hReadBufferEvent = NULL;
    }
    if (g_hci.hReadPacketEvent != NULL)
    {
        CloseHandle(g_hci.hReadPacketEvent);
        g_hci.hReadPacketEvent = NULL;
    }
    if (g_hci.hRequestDoneEvent != NULL)
    {
        CloseHandle(g_hci.hRequestDoneEvent);
        g_hci.hRequestDoneEvent = NULL;
    }
    DeleteCriticalSection(&g_hci.writeCS);
    Sleep(100);
    DeleteCriticalSection(&g_hci.readCS);

    RETAILMSG(TRUE, (L" HCI_CloseConnection: Stop hardware and closing the GPIO +\r\n"));
    
    if(g_hci.hGpio != NULL)
    {
        GPIOClose(g_hci.hGpio);
        g_hci.hGpio = NULL;
    }
    RETAILMSG(1, (L" HCI_CloseConnection: Unmapping the memory +\r\n"));

	MmUnmapIoSpace((VOID *)g_hci.pPadConfig, sizeof(OMAP_SYSC_PADCONFS_REGS));
	MmUnmapIoSpace((VOID *)g_hci.pPadConfig2, sizeof(OMAP_SYSC_PADCONFS_WKUP_REGS));

	// Close COM port
    CloseHandle(g_hci.hCom);
    g_hci.hCom = NULL;
	
    g_hci.opened = FALSE;

cleanUp:
    RETAILMSG(TRUE, (L" --- HCI_CloseConnection ---\r\n"));
    IFDBG(DebugOut(DEBUG_HCI_INIT, L"-HCI_CloseConnection\r\n"));
}

//------------------------------------------------------------------------------
int
HCI_ReadHciParameters(
    HCI_PARAMETERS *pParms
    )
{
    BOOL rc = FALSE;

    // Check size
    if (pParms->uiSize < sizeof (*pParms)) goto cleanUp;
    memset(pParms, 0, sizeof(*pParms));

    pParms->uiSize             = sizeof(*pParms);
    pParms->fInterfaceVersion  = HCI_INTERFACE_VERSION_1_1;
    pParms->iMaxSizeRead       = PACKET_SIZE_R;
    pParms->iMaxSizeWrite      = PACKET_SIZE_W;
    pParms->iWriteBufferHeader = 4;
    pParms->iReadBufferHeader  = 4;
    pParms->uiWriteTimeout     = g_hci.writeTimeout;
    pParms->uiResetDelay       = g_hci.resetDelay;
    pParms->uiFlags            = g_hci.flags;
    pParms->uiDriftFactor      = g_hci.drift;
    if (g_hci.specV10a)
    {
        pParms->fHardwareVersion = HCI_HARDWARE_VERSION_V_1_0_A;
    }
    else if (g_hci.specV11)
    {
        pParms->fHardwareVersion = HCI_HARDWARE_VERSION_V_1_1;
    }
    else
    {
        pParms->fHardwareVersion = HCI_HARDWARE_VERSION_V_1_0_B;
    }
#if defined (DEBUG) || defined (_DEBUG)
    pParms->iReadBufferHeader   = DEBUG_READ_BUFFER_HEADER;
    pParms->iReadBufferTrailer  = DEBUG_READ_BUFFER_TRAILER;
    pParms->iWriteBufferHeader  = DEBUG_WRITE_BUFFER_HEADER;
    pParms->iWriteBufferTrailer = DEBUG_WRITE_BUFFER_TRAILER;
#endif
    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

int
HCI_WritePacket(
    HCI_TYPE eType,
    BD_BUFFER *pPacket
    )
{
    int rc = FALSE;

    IFDBG(DebugOut(DEBUG_HCI_TRANSPORT,
        L"HCI_WritePacket type 0x%02x length %d\r\n",
        eType, BufferTotal(pPacket)
        ));
    //DumpBuffer(pPacket->pBuffer + pPacket->cStart, BufferTotal(pPacket));

#if defined(DEBUG) || defined(_DEBUG)
    ASSERT(pPacket->cStart == DEBUG_WRITE_BUFFER_HEADER);
    ASSERT(pPacket->cSize - pPacket->cEnd >= DEBUG_WRITE_BUFFER_TRAILER);
#endif

    ASSERT ((pPacket->cStart & 0x3) == 0);

    // Make sure that hardware is started...
    if (!g_hci.opened)
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"HCI_WritePacket - not active\r\n"));
        goto cleanUp;
    }

    // Check if we get packet with right size
    if (((int)BufferTotal(pPacket) > PACKET_SIZE_W) || (pPacket->cStart < 1))
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI_WritePacket: "
            L"Packet too big (%d, max %d), or no space for header!\r\n",
            BufferTotal(pPacket), PACKET_SIZE_W
            ));
        goto cleanUp;
    }


    // Set packet type to buffer
    pPacket->pBuffer[--pPacket->cStart] = (UCHAR)eType;
    // Write packet...
    if (!WritePacket(
            &g_hci, pPacket->pBuffer + pPacket->cStart,
            BufferTotal(pPacket), g_hci.driverTimeout
            ))
        {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI_WritePacket: "
            L"WritePacket type 0x%02x length %d failed\r\n",
            eType, BufferTotal(pPacket)
            ));
        goto cleanUp;
        }

    IFDBG(DebugOut(DEBUG_HCI_TRANSPORT, L"HCI_WritePacket: "
        L"DONE type 0x%02x length %d\r\n", eType, BufferTotal(pPacket)
        ));

    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
int
HCI_ReadPacket(
    HCI_TYPE *peType,
    BD_BUFFER *pPacket
    )
{
    int rc = FALSE;
    UCHAR *pBuffer;
    HCI_PACKET_COMMAND_COMPLETE_EVENT *pEventPacket;
    DWORD size;

    IFDBG(DebugOut(DEBUG_HCI_TRANSPORT, L"+HCI_ReadPacket\r\n"));
    //RETAILMSG(TRUE, (L" +++ HCI_ReadPacket +++\r\n"));

    if (!g_hci.opened)
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI_ReadPacket: "
            L"Connection is not active\r\n"
            ));
        goto cleanUp;
    }

    pPacket->cStart = PACKET_BODY_OFFSET;
    pPacket->cEnd = pPacket->cSize + PACKET_BODY_OFFSET;
    if (BufferTotal(pPacket) < PACKET_BODY_SIZE)
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI_ReadPacket: "
            L"Buffer too small (%d bytes, should be 260)\r\n",
            BufferTotal(pPacket)
            ));
        goto cleanUp;
    }


    RETAILMSG(0, (
        L"HCI_ReadPacket  - buffer size %3d start %3d end %d\r\n",
        pPacket->cEnd - pPacket->cStart, pPacket->cStart, pPacket->cEnd
    ));


    // Read packet
    size = ReadPacket(
        &g_hci, pPacket->pBuffer + pPacket->cStart,
        BufferTotal(pPacket), INFINITE
        );

    // If returned size is zero, something bad happen...
    if (size == 0)
    {
        IFDBG(DebugOut(DEBUG_ERROR, L"ERROR: HCI_ReadPacket: "
            L"ReadPacket return zero length packet\r\n"
            ));
        goto cleanUp;
    }

    // Fix packet start & end
    pPacket->cEnd = pPacket->cStart + size;
    pPacket->cStart++;

    // We need detect EVENT packet which is response
    // for HCI_RESET command packet.
    pBuffer = &pPacket->pBuffer[pPacket->cStart - 1];
    pEventPacket = (HCI_PACKET_COMMAND_COMPLETE_EVENT*)pBuffer;
    if ((pEventPacket->type == EVENT_PACKET) &&
        (pEventPacket->eventCode == COMMAND_COMPLETE_EVENT_CODE) &&
        (pEventPacket->commandOpcode == HCI_RESET))
    {
        // Run initialization script
		RETAILMSG(TRUE, (L" *** HCI_RESET ***\r\n"));
            if( !StartScript(&g_hci) )
            {
                RETAILMSG(TRUE, (L" --- StartScript Failed For Reset ---\r\n"));
                goto cleanUp;
            }
#if 0
		// packet to send
		HCI_PACKET_HEADER packet = {0};
		// receive buffer
		UCHAR buffer[32] = {0};
		HCI_PACKET_COMMAND_COMPLETE_EVENT *pHeader = (HCI_PACKET_COMMAND_COMPLETE_EVENT*)buffer;
	  
		packet.type = COMMAND_PACKET;
		packet.commandWithParams.header.code = HCI_VS_WRITE_CODEC_CONFIG;
		packet.commandWithParams.header.length = sizeof(HCI_VS_CODEC_CONFIG);
		// Codec config
		packet.commandWithParams.params.codecConfig.PCM_clock_rate = 32 * 8; // 32 bits * 8 kHz
		packet.commandWithParams.params.codecConfig.PCM_clock_direction = 0; // master mode
		packet.commandWithParams.params.codecConfig.Frame_sync_frequency = 8000; //8 kHz
		packet.commandWithParams.params.codecConfig.Frame_sync_duty_cycle = 0; // 50% of Fsync period
		packet.commandWithParams.params.codecConfig.Frame_sync_edge = 0; // Driven/sampled at rising edge
		packet.commandWithParams.params.codecConfig.Frame_sync_polarity = 1; // Active low
		packet.commandWithParams.params.codecConfig.Channel1_data_out_size = 16; //bits per channel
		packet.commandWithParams.params.codecConfig.Channel2_data_out_size = 16;
		packet.commandWithParams.params.codecConfig.Channel1_data_out_offset = 1;
		packet.commandWithParams.params.codecConfig.Channel2_data_out_offset = 17;
		packet.commandWithParams.params.codecConfig.Channel1_data_out_edge = 0; // Data driven at rising edge
		packet.commandWithParams.params.codecConfig.Channel2_data_out_edge = 0; // Data driven at rising edge
		packet.commandWithParams.params.codecConfig.Channel1_data_in_size = 16;
		packet.commandWithParams.params.codecConfig.Channel2_data_in_size = 16;
		packet.commandWithParams.params.codecConfig.Channel1_data_in_offset = 1;
		packet.commandWithParams.params.codecConfig.Channel2_data_in_offset = 17;
		packet.commandWithParams.params.codecConfig.Channel1_data_in_edge = 0; // Data sampled at falling edge
		packet.commandWithParams.params.codecConfig.Channel2_data_in_edge = 0; // Data sampled at falling edge

		if (!WritePacket(
				&g_hci, (UCHAR*)&packet,
				sizeof(UCHAR) + sizeof(HCI_COMMAND_PACKET_HEADER) + sizeof(HCI_VS_CODEC_CONFIG), g_hci.driverTimeout
				))
		{
			RETAILMSG(1, (L"HCI : %S WritePacket fail\r\n", __FUNCTION__));
			goto cleanUp;
		}

		// Get response...
		//  The return value of the function stored in a temp variable.
		if ((ReadPacket(&g_hci, buffer, sizeof(buffer), g_hci.driverTimeout) == 0) ||
			(pHeader->type != EVENT_PACKET) ||
			(pHeader->eventCode != COMMAND_COMPLETE_EVENT_CODE) ||
			(pHeader->status > 0))
		{
			RETAILMSG(1, (L"HCI : %S, ReadPacket fail, status = %d\r\n", __FUNCTION__, pHeader->status));
			//goto cleanUp;
		}
#endif
    }

    // Set packet type
    *peType = (HCI_TYPE)pBuffer[0];

#if DBG_DUMPBUFFERS
    RETAILMSG(TRUE, (
        L"HCI_ReadPacket  - type %02x size %3d start %3d end %d\r\n",
        *peType, pPacket->cEnd - pPacket->cStart, pPacket->cStart, pPacket->cEnd
    ));
    DumpBuffer(pPacket->pBuffer + pPacket->cStart, BufferTotal(pPacket));
#endif

    // We are done
    rc = TRUE;

    IFDBG(DebugOut(DEBUG_HCI_TRANSPORT, L"-HCI_ReadPacket rc:%d\r\n", rc));
    RETAILMSG(0, (L" --- HCI_ReadPacket ---\r\n"));
cleanUp:
    return rc;
}



