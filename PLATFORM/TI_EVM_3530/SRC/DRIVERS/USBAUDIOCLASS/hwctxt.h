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
#ifndef _INC_HWCTX_H_
#define _INC_HWCTX_H_

#pragma once
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.


Module Name:    HWCTXT.H

Abstract:       Platform dependent code for the mixing audio driver.

-*/
#include <WaveRtgCtrl.h>

#include "usbaudio.hpp"
#include "usbaudiodevice.hpp"

#include "DtpNotify.h"


#define AUDIO_DMA_NUMBER_PAGES  2
#define MAX_SUPPORTED_DEVICE    1

// Choose a DMA page size to make interrupt time a multiple of ms
#define AUDIO_NUM_FRAMES        32
#define AUDIO_DMA_PAGE_SIZE     (AUDIO_FRAME_SIZE*AUDIO_NUM_FRAMES)

class HardwareContext
{
protected:
    class AutoLock
    {
    public:
        AutoLock(HardwareContext *pHC) : m_pHC(pHC) {   m_pHC->Lock();    }
        ~AutoLock() {   m_pHC->Unlock();    }
    private:
        HardwareContext *m_pHC;
    };
private:
	TCHAR m_deviceName[32];
public:
    static BOOL CreateHWContext(DWORD Index);
    HardwareContext();
    ~HardwareContext();

    void Lock()   {EnterCriticalSection(&m_Lock);}
    void Unlock() {LeaveCriticalSection(&m_Lock);}

	TCHAR *GetDeviceName()
	{
		return m_deviceName;
	}

    DWORD GetNumInputDevices()
    {
        AutoLock lock(this);
        return min (m_pUsbAudioDevice?m_pUsbAudioDevice->GetNumInputs():MAX_SUPPORTED_DEVICE, MAX_SUPPORTED_DEVICE);
    }

    DWORD GetNumOutputDevices()
    {
        AutoLock lock(this);
        return min (m_pUsbAudioDevice?m_pUsbAudioDevice->GetNumOutputs():MAX_SUPPORTED_DEVICE, MAX_SUPPORTED_DEVICE);
    }

    DWORD GetNumMixerDevices()  { return 1; }

    DWORD GetNumInputChannels(UINT DeviceId)
    {
        AutoLock lock(this);
        return m_pUsbAudioDevice?m_pUsbAudioDevice->GetInput(DeviceId)->GetNumChannels():0;
    }
    DWORD GetNumOutputChannels(UINT DeviceId)
    {
        AutoLock lock(this);
        return m_pUsbAudioDevice?m_pUsbAudioDevice->GetOutput(DeviceId)->GetNumChannels():0;
    }

    DeviceContext *GetInputDeviceContext(UINT DeviceId)
    {
        return (DeviceContext *) m_InputDeviceContext[DeviceId];
    }

    DeviceContext *GetOutputDeviceContext(UINT DeviceId)
    {
        return (DeviceContext *) m_OutputDeviceContext[DeviceId];
    }

    BOOL Init(DWORD Index);
    BOOL Deinit();

    void PowerUp();
    void PowerDown();

    BOOL StartInputDMA(UINT DeviceId);
    BOOL StartOutputDMA(UINT DeviceId);

    void StopInputDMA(UINT DeviceId);
    void StopOutputDMA(UINT DeviceId);

    BOOL PmControlMessage (
                      DWORD  dwCode,
                      PBYTE  pBufIn,
                      DWORD  dwLenIn,
                      PBYTE  pBufOut,
                      DWORD  dwLenOut,
                      PDWORD pdwActualOut);

    void *GetMixerControlList() { return m_pMixerControlList; }
    void SetMixerControlList(void *pMixerControlList) { m_pMixerControlList = pMixerControlList; }

    BOOL Attach(USBAudioDevice* pUsbAudioDevice)
    {
        AutoLock lock(this);
        if (m_pUsbAudioDevice)
        {
            DEBUGMSG(ZONE_ERROR, (L"Can not reattach device[%p]\r\n", pUsbAudioDevice));
            return FALSE;
        }
        m_pUsbAudioDevice = pUsbAudioDevice;
        if (DoAttach()==FALSE)
        {
            m_pUsbAudioDevice = NULL;
            return FALSE;
        }
        return TRUE;
    }
    USBAudioDevice *Detach()
    {
        AutoLock lock(this);
        DoDetach();
        return static_cast<USBAudioDevice *>(InterlockedExchangePointer(&m_pUsbAudioDevice, NULL));

    }

    BOOL IsAttached()   {   return (m_pUsbAudioDevice!=NULL);   }

    DWORD OutputGetProperty(PWAVEPROPINFO pPropInfo);
    DWORD OutputSetProperty(PWAVEPROPINFO pPropInfo);
    DWORD InputGetProperty(PWAVEPROPINFO pPropInfo);
    DWORD InputSetProperty(PWAVEPROPINFO pPropInfo);

protected:
    // Device topology.
    DWORD GetOutputDeviceDescriptor(PDTP_DEVICE_DESCRIPTOR pDeviceDescriptor);
    DWORD GetInputDeviceDescriptor(PDTP_DEVICE_DESCRIPTOR pDeviceDescriptor);
    DWORD GetOutputEndpointDescriptor(DWORD dwIndex, PDTP_ENDPOINT_DESCRIPTOR pEndpointDescriptor);
    DWORD GetInputEndpointDescriptor(DWORD dwIndex, PDTP_ENDPOINT_DESCRIPTOR pEndpointDescriptor);

    DWORD NotifyEndpointsAttached(BOOL fAttached);

protected:
    virtual ULONG TransferInputBuffer(UINT DeviceId, PBYTE pBuffer, DWORD Size);
    virtual ULONG TransferOutputBuffer(UINT DeviceId, PBYTE pBuffer, DWORD Size);

    DWORD CommonGetProperty(PWAVEPROPINFO pPropInfo, BOOL fInput);
    DWORD CommonSetProperty(PWAVEPROPINFO pPropInfo, BOOL fInput);

    BOOL InitInterruptThread();
    void DeInitInterruptThread();

    void MapDMABuffers();
    void UnmapDMABuffers();

    void ResetDeviceContext();
    BOOL DoAttach();
    void DoDetach();

    CRITICAL_SECTION m_Lock;

    BOOL m_Initialized;
    BOOL m_InPowerHandler;

    USBAudioDevice* m_pUsbAudioDevice;

    InputDeviceContext** m_InputDeviceContext;
    OutputDeviceContext** m_OutputDeviceContext;

    // Which DMA channel are running
    BOOL* m_InputDMARunning;
    BOOL* m_OutputDMARunning;

    HANDLE* m_hOutputRunning;   // Handle to Audio Output event.
    HANDLE* m_hInputRunning;    // Handle to Audio Input event.

    HANDLE m_hUsbReadyOut[AUDIO_DMA_NUMBER_PAGES];
    USB_TRANSFER m_hTransferOut[AUDIO_DMA_NUMBER_PAGES];
    HANDLE m_hUsbReadyIn[AUDIO_DMA_NUMBER_PAGES];
    USB_TRANSFER m_hTransferIn[AUDIO_DMA_NUMBER_PAGES];

    //----------------------- Platform specific members ----------------------------------
    DWORD  m_OutputDMABuffer;   // Output DMA channel's current buffer
    DWORD  m_InputDMABuffer;    // Input DMA channel's current buffer
    //------------------------------------------------------------------------------------

    //DtpNotify* m_OutputDtpNotify;                            // Output device topology notifications.
    //DtpNotify* m_InputDtpNotify;                             // Input device topology notifications.
	DtpNotify m_OutputDtpNotify;                            // Output device topology notifications.
	DtpNotify m_InputDtpNotify;                             // Input device topology notifications.

    BOOL m_fEndpointsAttached;

    CEDEVICE_POWER_STATE m_DxState;

    void *m_pMixerControlList;

    // Gain-related APIs
public:
    DWORD       GetOutputGain (VOID);
    MMRESULT    SetOutputGain (DWORD dwVolume);
    DWORD       GetInputGain (VOID);
    MMRESULT    SetInputGain (DWORD dwVolume);
	DWORD       GetMicGain (VOID);
    MMRESULT    SetMicGain (DWORD dwGain);
	DWORD       GetMicGainMin (VOID);
	DWORD       GetMicGainMax (VOID);
	DWORD       GetMicGainRes (VOID);
	BOOL        IsMicAGCSupported();
	BOOL        GetMicAGC (VOID);
    MMRESULT    SetMicAGC (BOOL fEnable);

    BOOL        GetOutputMute (VOID);
    MMRESULT    SetOutputMute (BOOL fMute);
    BOOL        GetInputMute (VOID);
    MMRESULT    SetInputMute (BOOL fMute);

protected:
    void UpdateOutputGain(UINT DeviceId);
    void UpdateInputGain(UINT DeviceId);
    DWORD m_dwOutputGain;
    DWORD m_dwInputGain;
    BOOL  m_fInputMute;
    BOOL  m_fOutputMute;
};

extern HardwareContext *g_pHWContext;

#endif //~_INC_HWCTX_H_

