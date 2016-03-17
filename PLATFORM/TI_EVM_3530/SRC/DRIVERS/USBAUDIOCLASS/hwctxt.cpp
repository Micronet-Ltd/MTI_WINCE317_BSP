//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
/*
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:    HWCTXT.CPP

Abstract:               Platform dependent code for the mixing audio driver.

Notes:                  The following file contains all the hardware specific code
for the mixing audio driver.  This code's primary responsibilities
are:

* Initialize audio hardware (including codec chip)
* Schedule DMA operations (move data from/to buffers)
* Handle audio interrupts

All other tasks (mixing, volume control, etc.) are handled by the "upper"
layers of this driver.
*/

#include "wavemain.h"
#include "ceddk.h"
#include "hwctxt.h"
#include "devctxt.h"
#include <WaveTopologyGuids.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

HardwareContext *g_pHWContext           = NULL;

BOOL HardwareContext::CreateHWContext(DWORD Index)
{
	HKEY hKey;
    DWORD dwType;
    DWORD cbData = 0;

    if (g_pHWContext)
    {
        return TRUE;
    }

    g_pHWContext = new HardwareContext;
    if (!g_pHWContext)
    {
        return FALSE;
    }

	LONG lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCWSTR)Index, 0, 0, &hKey);
    if (ERROR_SUCCESS == lStatus)
	{	
		if (RegQueryValueEx(hKey, TEXT("Name"), NULL, &dwType, NULL, &cbData ) == ERROR_SUCCESS && dwType == REG_SZ && cbData > 0 && cbData <= sizeof(g_pHWContext->m_deviceName))
		{
			RegQueryValueEx(hKey, TEXT("Name"), NULL, &dwType, (LPBYTE)(g_pHWContext->m_deviceName), &cbData);
			RegCloseKey(hKey);
		}
	}

    return g_pHWContext->Init(Index);
}

HardwareContext::HardwareContext():
m_pUsbAudioDevice(NULL),
m_InputDeviceContext(NULL),
m_OutputDeviceContext(NULL),
m_InputDMARunning(NULL),
m_OutputDMARunning(NULL),
m_hOutputRunning(NULL),
m_hInputRunning(NULL),
//m_OutputDtpNotify(NULL),
//m_InputDtpNotify(NULL),
m_pMixerControlList(NULL)
{
    DEBUGMSG(1, (TEXT("HardwareContext(%p)\n"), this));

	InitializeCriticalSection(&m_Lock);
    m_Initialized=FALSE;
	memset(m_deviceName, 0, sizeof(m_deviceName));
}

HardwareContext::~HardwareContext()
{
    DWORD NumInputs = GetNumInputDevices();
    DWORD NumOutputs = GetNumOutputDevices();
    for (DWORD i = 0; i < NumInputs; i++ ) 
    {
        delete m_InputDeviceContext[i];
        m_InputDMARunning[i] = FALSE;
    }

    for (DWORD i = 0; i < NumOutputs; i++ ) 
    {
        delete m_OutputDeviceContext[i];
        m_OutputDMARunning[i] = FALSE;
    }

    delete []m_InputDeviceContext;
    delete []m_InputDMARunning;
    delete []m_hInputRunning;
    delete []m_OutputDeviceContext;
    delete []m_OutputDMARunning;
    delete []m_hOutputRunning;

	//delete m_OutputDtpNotify;
	//delete m_InputDtpNotify;

    DeleteCriticalSection(&m_Lock);
}

BOOL HardwareContext::Init(DWORD Index)
{
    if (m_Initialized) 
    {
        goto cleanUp;
    }

	//m_OutputDtpNotify	= new DtpNotify();
	//m_InputDtpNotify	= new DtpNotify();

    m_fEndpointsAttached = FALSE;

    DWORD NumInputs = GetNumInputDevices();
    DWORD NumOutputs = GetNumOutputDevices();
    
    m_InputDeviceContext = new InputDeviceContext*[NumInputs];
    m_InputDMARunning = new BOOL[NumInputs];
    m_hInputRunning = new HANDLE[NumInputs];
    m_OutputDeviceContext = new OutputDeviceContext*[NumOutputs];
    m_OutputDMARunning = new BOOL[NumOutputs];
    m_hOutputRunning = new HANDLE[NumOutputs];
          
    for (DWORD i = 0; i < NumInputs; i++ ) 
    {
        m_InputDeviceContext[i] = new InputDeviceContext(i, this);
        m_InputDMARunning[i] = FALSE;
    }

    for (DWORD i = 0; i < NumOutputs; i++ ) 
    {
        m_OutputDeviceContext[i] = new OutputDeviceContext(i, this);
        m_OutputDMARunning[i] = FALSE;
    }

    // Initialize the state/status variables
    m_InPowerHandler    = FALSE;
    m_InputDMABuffer    = 0;
    m_OutputDMABuffer   = 0;

    // Map the DMA buffers into driver's virtual address space
    MapDMABuffers();
    
    // Configure the Codec
    m_dwOutputGain = 0xFFFFFFFF;
    m_dwInputGain  = 0xFFFFFFFF;
    m_fInputMute  = FALSE;
    m_fOutputMute = FALSE;

    // Initialize the mixer controls in mixerdrv.cpp
    InitMixerControls(this);

    m_Initialized = TRUE;

cleanUp:
    return m_Initialized;
}

BOOL HardwareContext::Deinit()
{
    UnmapDMABuffers();
    return TRUE;
}

VOID HardwareContext::MapDMABuffers()
{
    // Do any DMA buffer related setup here
    return;
}

VOID HardwareContext::UnmapDMABuffers()
{
    // Do any cleanup related to DMA buffers here
    return;
}

VOID HardwareContext::PowerUp() 
{
    return;
}

VOID HardwareContext::PowerDown()
{
    return;
}

BOOL HardwareContext::DoAttach()
{
    BOOL rc = FALSE;
    AutoLock lock(this);

    for (DWORD i = 0; i < GetNumInputDevices(); i++ ) 
    {
        m_InputDeviceContext[i]->SetBaseSampleRate(m_pUsbAudioDevice->GetInput(i)->GetSampleRate());
        m_InputDMARunning[i] = FALSE;
        m_pUsbAudioDevice->GetInput(i)->SetDeviceId(i);
    }

    for (DWORD i = 0; i < GetNumOutputDevices(); i++ ) 
    {
        m_OutputDeviceContext[i]->SetBaseSampleRate(m_pUsbAudioDevice->GetOutput(i)->GetSampleRate());
        m_OutputDMARunning[i] = FALSE;
        m_pUsbAudioDevice->GetOutput(i)->SetDeviceId(i);
    }

    // Initialize the interrupt thread
    if (!InitInterruptThread())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("WAVEDEV.DLL:HardwareContext::DoAttach() - Failed to initialize interrupt thread.\r\n")));
        goto cleanUp;
    }

    NotifyEndpointsAttached(TRUE);

    rc = TRUE;
cleanUp:
    return rc;
}

void HardwareContext::DoDetach()
{
    AutoLock lock(this);
    
    ASSERT(m_pUsbAudioDevice!=NULL);
    
    // Reset the device context.
    ResetDeviceContext();

    // Stop all of input/output transffers.
    for (DWORD i = 0; i < GetNumOutputDevices(); i++) 
    {
        m_pUsbAudioDevice->GetOutput(i)->Stop();
    }

    for (DWORD i = 0; i < GetNumInputDevices(); i++) 
    {
        m_pUsbAudioDevice->GetInput(i)->Stop();
    }

    // Deinit Interrupt Thread.
    DeInitInterruptThread();

    NotifyEndpointsAttached(FALSE);
}

inline VOID HardwareContext::UpdateOutputGain(UINT DeviceId)
{
    m_OutputDeviceContext[DeviceId]->SetGain(m_fOutputMute ? 0 : m_dwOutputGain);
}

inline VOID HardwareContext::UpdateInputGain(UINT DeviceId)
{
    m_InputDeviceContext[DeviceId]->SetGain(m_fInputMute ? 0 : m_dwInputGain);
}

MMRESULT HardwareContext::SetOutputGain(DWORD dwGain)
{
    m_dwOutputGain = dwGain;
    UpdateOutputGain(0);
    return MMSYSERR_NOERROR;
}

MMRESULT HardwareContext::SetOutputMute(BOOL fMute)
{
    m_fOutputMute = fMute;
    UpdateOutputGain(0);
    return MMSYSERR_NOERROR;
}

DWORD HardwareContext::GetOutputGain(VOID)
{
    return m_dwOutputGain;
}

BOOL HardwareContext::GetOutputMute(VOID)
{
    return m_fOutputMute;
}

BOOL HardwareContext::GetInputMute(VOID)
{
    return m_fInputMute;
}

MMRESULT HardwareContext::SetInputMute(BOOL fMute)
{
    m_fInputMute = fMute;
    UpdateInputGain(0);
    return MMSYSERR_NOERROR;
}

DWORD HardwareContext::GetInputGain(VOID)
{
    return m_dwInputGain;
}

MMRESULT HardwareContext::SetInputGain(DWORD dwGain)
{
    m_dwInputGain = dwGain;
    UpdateInputGain(0);
    return MMSYSERR_NOERROR;
}

DWORD HardwareContext::GetMicGain(VOID)
{
	if (GetNumInputDevices())
		return m_pUsbAudioDevice->GetInput(0)->GetMicGain();

	return 0;
}

MMRESULT HardwareContext::SetMicGain(DWORD dwGain)
{
    if (GetNumInputDevices())
		m_pUsbAudioDevice->GetInput(0)->SetMicGain(dwGain);

    return MMSYSERR_NOTSUPPORTED;
}

DWORD HardwareContext::GetMicGainMin (VOID)
{
	if (GetNumInputDevices())
		return m_pUsbAudioDevice->GetInput(0)->GetMicGainMin();

	return 0;
}

DWORD HardwareContext::GetMicGainMax (VOID)
{
	if (GetNumInputDevices())
		return m_pUsbAudioDevice->GetInput(0)->GetMicGainMax();

	return 0;
}

DWORD HardwareContext::GetMicGainRes (VOID)
{
	if (GetNumInputDevices())
		return m_pUsbAudioDevice->GetInput(0)->GetMicGainRes();

	return 0;
}

BOOL HardwareContext::IsMicAGCSupported()
{
	if (GetNumInputDevices())
		return m_pUsbAudioDevice->GetInput(0)->IsMicAGCSupported();

	return FALSE;
}

BOOL HardwareContext::GetMicAGC(VOID)
{
	if (GetNumInputDevices())
		return m_pUsbAudioDevice->GetInput(0)->GetMicAGC();

	return FALSE;
}

MMRESULT HardwareContext::SetMicAGC(BOOL fEnable)
{
	if (GetNumInputDevices())
		m_pUsbAudioDevice->GetInput(0)->SetMicAGC(fEnable);

	return MMSYSERR_NOTSUPPORTED;
}

BOOL HardwareContext::StartOutputDMA(UINT DeviceId)
{
    AutoLock lock(this);
    if (m_pUsbAudioDevice)
    {
        m_pUsbAudioDevice->GetOutput(DeviceId)->Start();
        return TRUE;
    }
    return FALSE;
}

VOID HardwareContext::StopOutputDMA(UINT DeviceId)
{
    AutoLock lock(this);
    if (m_pUsbAudioDevice)
    {
        m_pUsbAudioDevice->GetOutput(DeviceId)->Stop();
    }
    return;
}

BOOL HardwareContext::StartInputDMA(UINT DeviceId)
{
    AutoLock lock(this);
    if (m_pUsbAudioDevice)
    {
        m_pUsbAudioDevice->GetInput(DeviceId)->Start();
        return TRUE;
    }
    return FALSE;
}

VOID HardwareContext::StopInputDMA(UINT DeviceId)
{
    AutoLock lock(this);
    if (m_pUsbAudioDevice)
    {
        m_pUsbAudioDevice->GetInput(DeviceId)->Stop();
    }
    return;
}

// Initializes the IST for handling DMA interrupts.
BOOL HardwareContext::InitInterruptThread()
{
    BOOL rc = FALSE;
    AutoLock lock(this);

    for (DWORD i = 0; i < GetNumOutputDevices(); i++) 
    {
        m_hOutputRunning[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!m_hOutputRunning[i]) 
        {
            ERRMSG("Unable to create interrupt event");
            goto cleanUp;
        }
    }

    for (DWORD i = 0; i < GetNumInputDevices(); i++) 
    {
        m_hInputRunning[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!m_hInputRunning[i]) 
        {
            ERRMSG("Unable to create interrupt event");
            goto cleanUp;
        }
    }

    for(int i = 0; i < AUDIO_DMA_NUMBER_PAGES; i++) 
    {
        m_hUsbReadyOut[i] = CreateEvent(NULL, TRUE, TRUE, NULL);
        if (!m_hUsbReadyOut[i]) 
        {
            ERRMSG("Unable to create interrupt event");
            goto cleanUp;
        }
        
        m_hUsbReadyIn[i] = CreateEvent(NULL, TRUE, TRUE, NULL);
        if (!m_hUsbReadyIn[i]) 
        {
            ERRMSG("Unable to create interrupt event");
            goto cleanUp;
        }
    }

    for (DWORD i = 0; i < GetNumInputDevices(); i++) 
    {
        m_pUsbAudioDevice->GetInput(i)->SetDeviceContext(m_InputDeviceContext[i]);
        m_pUsbAudioDevice->GetInput(i)->InitAudioThread();        
    }
    
    for (DWORD i = 0; i < GetNumOutputDevices(); i++) 
    {
        m_pUsbAudioDevice->GetOutput(i)->SetDeviceContext(m_OutputDeviceContext[i]);
        m_pUsbAudioDevice->GetOutput(i)->InitAudioThread();
    }

    rc = TRUE;

cleanUp:
    return rc;
}

void HardwareContext::DeInitInterruptThread()
{
    AutoLock lock(this);
    for (DWORD i = 0; i < GetNumInputDevices(); i++) 
    {
        m_pUsbAudioDevice->GetInput(i)->DeInitAudioThread();    
    }
    
    for (DWORD i = 0; i < GetNumOutputDevices(); i++) 
    {
        m_pUsbAudioDevice->GetOutput(i)->DeInitAudioThread();
    }

    for (DWORD i = 0; i < GetNumOutputDevices(); i++) 
    {
        CloseHandle(m_hOutputRunning[i]);
    }

    for (DWORD i = 0; i < GetNumInputDevices(); i++) 
    {
        CloseHandle(m_hInputRunning[i]);
    }

    for(int i = 0; i < AUDIO_DMA_NUMBER_PAGES; i++) 
    {
        CloseHandle(m_hUsbReadyOut[i]);        
        CloseHandle(m_hUsbReadyIn[i]);
    }
}

// =========== Helper Functions =========

// Retrieves the next "mixed" audio buffer of data to DMA into the output channel.
// Returns number of bytes needing to be transferred.
ULONG HardwareContext::TransferOutputBuffer(UINT DeviceId, PBYTE pBuffer, DWORD Size)
{
    ULONG BytesTransferred = 0;

    PBYTE pBufferStart = pBuffer;
    PBYTE pBufferEnd = pBufferStart + Size;
    PBYTE pBufferLast;

    TRANSFER_STATUS TransferStatus = {0};

    pBufferLast = m_OutputDeviceContext[DeviceId]->TransferBuffer(pBufferStart, pBufferEnd,&TransferStatus);
    BytesTransferred = pBufferLast-pBufferStart;

    StreamContext::ClearBuffer(pBufferLast,pBufferEnd);

    return BytesTransferred;
}

// Retrieves the chunk of recorded sound data and inputs it into an audio buffer for potential "mixing".
// Returns number of bytes needing to be transferred.
ULONG HardwareContext::TransferInputBuffer(UINT DeviceId, PBYTE pBuffer, DWORD Size)
{
    ULONG BytesTransferred = 0;

    PBYTE pBufferStart = pBuffer;
    PBYTE pBufferEnd = pBufferStart + Size;
    PBYTE pBufferLast;

    pBufferLast = m_InputDeviceContext[DeviceId]->TransferBuffer(pBufferStart, pBufferEnd,NULL);
    BytesTransferred = pBufferLast-pBufferStart;

    return BytesTransferred;
}

VOID HardwareContext::ResetDeviceContext()
{
    for (DWORD i = 0; i < GetNumInputDevices(); i++) 
    {
        m_InputDeviceContext[i]->ResetDeviceContext();    
    }
    
    for (DWORD i = 0; i < GetNumOutputDevices(); i++) 
    {
        m_OutputDeviceContext[i]->ResetDeviceContext();    
    }
}

// Power management routine.
BOOL HardwareContext::PmControlMessage (
    DWORD  dwCode,
    PBYTE  pBufIn,
    DWORD  dwLenIn,
    PBYTE  pBufOut,
    DWORD  dwLenOut,
    PDWORD pdwActualOut)
{
    BOOL rc = FALSE;

    switch (dwCode) 
    {
        // Return device specific power capabilities.
        case IOCTL_POWER_CAPABILITIES:
            {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES)pBufOut;

                // Check arguments.
                if (ppc && (dwLenOut >= sizeof(POWER_CAPABILITIES)) && pdwActualOut) 
                {
                    // Clear capabilities structure.
                    memset(ppc, 0, sizeof(POWER_CAPABILITIES));

                    // Set power capabilities. Supports D0 and D4.
                    ppc->DeviceDx = DX_MASK(D0) | DX_MASK(D4);

                    DEBUGMSG(ZONE_FUNCTION, (TEXT("WAVE: IOCTL_POWER_CAPABILITIES = 0x%x\r\n"), ppc->DeviceDx));

                    // Update returned data size.
                    *pdwActualOut = sizeof(*ppc);
                    rc = TRUE;
                } 
                else 
                {
                    DEBUGMSG(ZONE_ERROR, (TEXT( "WAVE: Invalid parameter.\r\n" )));
                }

                break;
            }

            // Indicate if the device is ready to enter a new device power state.
        case IOCTL_POWER_QUERY:
            {
                PCEDEVICE_POWER_STATE pDxState = (PCEDEVICE_POWER_STATE)pBufOut;

                // Check arguments.
                if (pDxState && (dwLenOut >= sizeof(CEDEVICE_POWER_STATE)) && pdwActualOut) 
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("WAVE: IOCTL_POWER_QUERY = %d\r\n"), *pDxState));

                    // Check for any valid power state.
                    if (VALID_DX (*pDxState)) 
                    {
                        *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                        rc = TRUE;
                    } 
                    else 
                    {
                        DEBUGMSG(ZONE_ERROR, (TEXT( "WAVE: IOCTL_POWER_QUERY invalid power state.\r\n")));
                    }
                } 
                else 
                {
                    DEBUGMSG(ZONE_ERROR, (TEXT( "WAVE: IOCTL_POWER_QUERY invalid parameter.\r\n")));
                }
                break;
            }

            // Request a change from one device power state to another.
        case IOCTL_POWER_SET:
            {
                PCEDEVICE_POWER_STATE pDxState = (PCEDEVICE_POWER_STATE)pBufOut;

                // Check arguments.
                if (pDxState && (dwLenOut >= sizeof(CEDEVICE_POWER_STATE))) 
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("WAVE: IOCTL_POWER_SET = %d.\r\n"), *pDxState));

                    // Check for any valid power state.
                    if (VALID_DX(*pDxState)) 
                    {
                        Lock();

                        if (*pDxState == D4) 
                        {
                            PowerDown();
                        } 
                        else 
                        {
                            PowerUp();
                        }

                        m_DxState = *pDxState;
                        Unlock();
                        rc = TRUE;
                    } 
                    else 
                    {
                        DEBUGMSG(ZONE_ERROR, (TEXT("WAVE: IOCTL_POWER_SET invalid power state.\r\n")));
                    }
                } 
                else 
                {
                    DEBUGMSG(ZONE_ERROR, (TEXT( "WAVE: IOCTL_POWER_SET invalid parameter.\r\n")));
                }

                break;
            }

            // Return the current device power state.
        case IOCTL_POWER_GET:
            {
                DEBUGMSG(ZONE_FUNCTION, (TEXT("WAVE: IOCTL_POWER_GET -- not supported!\r\n")));
                break;
            }

        default:
            DEBUGMSG(ZONE_WARN, (TEXT("WAVE: Unknown IOCTL_xxx(0x%0.8X) \r\n"), dwCode));
            break;
    }

    return rc;
}

DWORD HardwareContext::CommonGetProperty(PWAVEPROPINFO pPropInfo, BOOL fInput)
{
    DWORD mmRet = MMSYSERR_NOTSUPPORTED;

    // Device topology
    if (IsEqualGUID(MM_PROPSET_DEVTOPOLOGY, *pPropInfo->pPropSetId))
    {
        switch (pPropInfo->ulPropId)
        {
        case MM_PROP_DEVTOPOLOGY_DEVICE_DESCRIPTOR:
            {
                if ((NULL == pPropInfo->pvPropData) ||
                    (pPropInfo->cbPropData < sizeof(DTP_DEVICE_DESCRIPTOR)))
                {
                    mmRet = MMSYSERR_INVALPARAM;
                    break;
                }

                PDTP_DEVICE_DESCRIPTOR pDeviceDescriptor;
                pDeviceDescriptor = (PDTP_DEVICE_DESCRIPTOR) pPropInfo->pvPropData;

                // Get the device descriptor.
                if (fInput)
                {
                    mmRet = GetInputDeviceDescriptor(pDeviceDescriptor);
                }
                else
                {
                    mmRet = GetOutputDeviceDescriptor(pDeviceDescriptor);
                }

                if (MMSYSERR_NOERROR == mmRet)
                {
                    *pPropInfo->pcbReturn = sizeof(DTP_DEVICE_DESCRIPTOR);
                }
            }
            break;

        case MM_PROP_DEVTOPOLOGY_ENDPOINT_DESCRIPTOR:
            {
                if ((NULL == pPropInfo->pvPropParams) ||
                    (pPropInfo->cbPropParams != sizeof(DWORD)) ||
                    (NULL == pPropInfo->pvPropData) ||
                    (pPropInfo->cbPropData < sizeof(DTP_ENDPOINT_DESCRIPTOR)))
                {
                    mmRet = MMSYSERR_INVALPARAM;
                    goto Error;
                }

                DWORD dwIndex = *((PDWORD) pPropInfo->pvPropParams);
                PDTP_ENDPOINT_DESCRIPTOR pEndpointDescriptor;
                pEndpointDescriptor = (PDTP_ENDPOINT_DESCRIPTOR) pPropInfo->pvPropData;

                // Get the endpoint descriptor.
                if (fInput)
                {
                    mmRet = GetInputEndpointDescriptor(
                                dwIndex,
                                pEndpointDescriptor);
                }
                else
                {
                    mmRet = GetOutputEndpointDescriptor(
                                dwIndex,
                                pEndpointDescriptor);
                }

                if (MMSYSERR_NOERROR == mmRet)
                {
                    *pPropInfo->pcbReturn = sizeof(DTP_ENDPOINT_DESCRIPTOR);
                }
            }
            break;

        default:
            break;
        }
    }

Error:

    return mmRet;
}

DWORD HardwareContext::CommonSetProperty(PWAVEPROPINFO pPropInfo, BOOL fInput)
{
    DWORD mmRet = MMSYSERR_NOTSUPPORTED;
    HANDLE hClientProc = NULL;

#if 0
    // Routing control
    if (IsEqualGUID(MM_PROPSET_RTGCTRL, *pPropInfo->pPropSetId))
    {
        switch (pPropInfo->ulPropId)
        {
        case MM_PROP_RTGCTRL_ENDPOINT_ROUTING:
            {
                if ((NULL == pPropInfo->pvPropData) ||
                    (pPropInfo->cbPropData < sizeof(RTGCTRL_ENDPOINT_ROUTING)))
                {
                    mmRet = MMSYSERR_INVALPARAM;
                    goto Error;
                }

                PRTGCTRL_ENDPOINT_ROUTING pEndpointRouting;
                pEndpointRouting = (PRTGCTRL_ENDPOINT_ROUTING) pPropInfo->pvPropData;

                // Check the media type to route.
                switch (pEndpointRouting->MediaType)
                {
                case RTGCTRL_MEDIATYPE_SYSTEM:
                    {
                        // Route system audio using the selected endpoints.
                        if (fInput)
                        {
                            mmRet = RouteSystemInputEndpoints(pEndpointRouting);
                        }
                        else
                        {
                            mmRet = RouteSystemOutputEndpoints(pEndpointRouting);
                        }
                    }
                    break;


                default:
                    break;
                }
            }
            break;

        default:
            break;
        }
    }
    // Device topology
    else 
#endif  //  0
    if (IsEqualGUID(MM_PROPSET_DEVTOPOLOGY, *pPropInfo->pPropSetId))
    {
        switch (pPropInfo->ulPropId)
        {
        case MM_PROP_DEVTOPOLOGY_EVTMSG_REGISTER:
            {
                if ((NULL == pPropInfo->pvPropData) ||
                    (pPropInfo->cbPropData != sizeof(HANDLE)))
                {
                    mmRet = MMSYSERR_INVALPARAM;
                    goto Error;
                }

                // Get client message queue handle.
                HANDLE hMsgQueue = *((HANDLE *) pPropInfo->pvPropData);

                // Get client process.
                hClientProc = OpenProcess(0, FALSE, GetCallerVMProcessId());
                if (NULL == hClientProc)
                {
                    mmRet = MMSYSERR_ERROR;
                    goto Error;
                }

                // Register for device topology messages.
                if (fInput)
                {
                    mmRet = m_InputDtpNotify.RegisterDtpMsgQueue(hMsgQueue, hClientProc);
                }
                else
                {
                    mmRet = m_OutputDtpNotify.RegisterDtpMsgQueue(hMsgQueue, hClientProc);
                }
            }
            break;

        case MM_PROP_DEVTOPOLOGY_EVTMSG_UNREGISTER:
            {
                if ((NULL == pPropInfo->pvPropData) ||
                    (pPropInfo->cbPropData != sizeof(HANDLE)))
                {
                    mmRet = MMSYSERR_INVALPARAM;
                    goto Error;
                }

                // Get client message queue handle.
                HANDLE hMsgQueue = *((HANDLE *) pPropInfo->pvPropData);

                // Get client process.
                hClientProc = OpenProcess(0, FALSE, GetCallerVMProcessId());
                if (NULL == hClientProc)
                {
                    mmRet = MMSYSERR_ERROR;
                    goto Error;
                }

                // Unregister the message queue.
                if (fInput)
                {
                    mmRet = m_InputDtpNotify.UnregisterDtpMsgQueue(hMsgQueue, hClientProc);
                }
                else
                {
                    mmRet = m_OutputDtpNotify.UnregisterDtpMsgQueue(hMsgQueue, hClientProc);
                }
            }
            break;

        default:
            break;
        }
    }

Error:

    if (hClientProc != NULL)
    {
        CloseHandle(hClientProc);
    }

    return mmRet;
}

DWORD HardwareContext::OutputGetProperty(PWAVEPROPINFO pPropInfo)
{
    return CommonGetProperty(pPropInfo, FALSE);
}

DWORD HardwareContext::OutputSetProperty(PWAVEPROPINFO pPropInfo)
{
    return CommonSetProperty(pPropInfo, FALSE);
}

DWORD HardwareContext::InputGetProperty(PWAVEPROPINFO pPropInfo)
{
    return CommonGetProperty(pPropInfo, TRUE);
}

DWORD HardwareContext::InputSetProperty(PWAVEPROPINFO pPropInfo)
{
    return CommonSetProperty(pPropInfo, TRUE);
}

//------------------------------------------------------------------------------
//
//  Device topology
//

enum
{
    WAVEOUT_EPIDX_DEFAULT = 0,
};

static const DTP_ENDPOINT_DESCRIPTOR c_rgOutputEndpointDescriptor[] =
{
    // Index 0 - Default
    {
        WAVEOUT_EPIDX_DEFAULT,              // Endpoint index
        WAVEOUT_ENDPOINT_DEFAULT,           // Endpoint GUID
        TRUE,                               // Removable
        FALSE,                              // Unattached - To be set during query
    },
};

static const DTP_DEVICE_DESCRIPTOR c_OutputDeviceDescriptor =
{
    WAVE_DEVCLASS_USBAUDIO,                 // Device class GUID
    _countof(c_rgOutputEndpointDescriptor)  // Number of endpoints
};

DWORD
HardwareContext::GetOutputDeviceDescriptor(
    PDTP_DEVICE_DESCRIPTOR pDeviceDescriptor
    )
{
    memcpy(pDeviceDescriptor, &c_OutputDeviceDescriptor,
        sizeof(*pDeviceDescriptor));
    return MMSYSERR_NOERROR;
}

DWORD
HardwareContext::GetOutputEndpointDescriptor(
    DWORD dwIndex,
    PDTP_ENDPOINT_DESCRIPTOR pEndpointDescriptor
    )
{
    DWORD mmRet = MMSYSERR_INVALPARAM;

    if (dwIndex < _countof(c_rgOutputEndpointDescriptor))
    {
        memcpy(pEndpointDescriptor, &c_rgOutputEndpointDescriptor[dwIndex],
            sizeof(*pEndpointDescriptor));

        // Set the attached state of removable endpoints.
        if (pEndpointDescriptor->fRemovable)
        {
            switch (pEndpointDescriptor->dwIndex)
            {
            case WAVEOUT_EPIDX_DEFAULT:
                pEndpointDescriptor->fAttached = m_fEndpointsAttached;
                break;
            default:
                break;
            }
        }

        mmRet = MMSYSERR_NOERROR;
    }

    return mmRet;
}

enum
{
    WAVEIN_EPIDX_DEFAULT = 0,
};

static DTP_ENDPOINT_DESCRIPTOR c_rgInputEndpointDescriptor[] =
{
    // Index 0 - Default
    {
        WAVEIN_EPIDX_DEFAULT,               // Endpoint index
        WAVEIN_ENDPOINT_DEFAULT,            // Endpoint GUID
        TRUE,                               // Removable
        FALSE,                              // Unattached - To be set during query
    },
};

static const DTP_DEVICE_DESCRIPTOR c_InputDeviceDescriptor =
{
    WAVE_DEVCLASS_USBAUDIO,                 // Device class GUID
    _countof(c_rgInputEndpointDescriptor)   // Number of endpoints
};

DWORD
HardwareContext::GetInputDeviceDescriptor(
    PDTP_DEVICE_DESCRIPTOR pDeviceDescriptor
    )
{
    memcpy(pDeviceDescriptor, &c_InputDeviceDescriptor,
        sizeof(*pDeviceDescriptor));
    return MMSYSERR_NOERROR;
}

DWORD
HardwareContext::GetInputEndpointDescriptor(
    DWORD dwIndex,
    PDTP_ENDPOINT_DESCRIPTOR pEndpointDescriptor
    )
{
    DWORD mmRet = MMSYSERR_INVALPARAM;

    if (dwIndex < _countof(c_rgInputEndpointDescriptor))
    {
        memcpy(pEndpointDescriptor, &c_rgInputEndpointDescriptor[dwIndex],
            sizeof(*pEndpointDescriptor));

        // Set the attached state of removable endpoints.
        if (pEndpointDescriptor->fRemovable)
        {
            switch (pEndpointDescriptor->dwIndex)
            {
            case WAVEIN_EPIDX_DEFAULT:
                pEndpointDescriptor->fAttached = m_fEndpointsAttached;
                break;
            default:
                break;
            }
        }

        mmRet = MMSYSERR_NOERROR;
    }

    return mmRet;
}

DWORD
HardwareContext::NotifyEndpointsAttached(
    BOOL fAttached
    )
{
    if (m_fEndpointsAttached != fAttached)
    {
        m_fEndpointsAttached = fAttached;

        // Send endpoint notifications for both output and input.
        DTP_EVTMSG_ENDPOINT_CHANGE msg = {0};
        msg.hdr.dwEvtType = DTP_EVT_ENDPOINT_CHANGE;
        msg.hdr.cbMsg = sizeof(msg);
        msg.fAttached = fAttached;

        msg.dwIndex = WAVEOUT_EPIDX_DEFAULT;
        m_OutputDtpNotify.SendDtpNotifications(&msg, sizeof(msg));

        msg.dwIndex = WAVEIN_EPIDX_DEFAULT;
        m_InputDtpNotify.SendDtpNotifications(&msg, sizeof(msg));
    }

    return MMSYSERR_NOERROR;
}

//#ifdef __cplusplus
// }
//#endif

