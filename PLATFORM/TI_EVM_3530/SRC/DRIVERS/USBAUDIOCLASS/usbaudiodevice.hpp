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
#pragma once

#include <windows.h>
#include <usbdi.h>

#include <ntcompat.h>
#include <CeDDK.h>

#include "usbclient.h"
#include "usbserv.h"

#pragma pack()

// ----------------------------------------------------------------------------
// Defines
// ----------------------------------------------------------------------------

class USBAudioDeviceTable;
class USBAudioDevice;
class USBAudioPipe;
class USBAudioInput;
class USBAudioOutput;

class DeviceContext;

// frames per transfer
#define USBAUD_FRAMES           25
// maximum number of input/output lines per device
#define USBAUD_MAX_LINES        16
// maximum number of sample rates
#define USBAUD_MAX_RATES        64
// maximum number of devices
#define USBAUD_MAX_DEVICES      64
// maximum sample rate
#define USBAUD_MAX_SAMPLERATE   16000
// minimum sample rate
#define USBAUD_MIN_SAMPLERATE   8000
// USB Audio Thread priority
#define USBAUD_THREAD_PRIORITY  110

// ----------------------------------------------------------------------------
// Declarations
// ----------------------------------------------------------------------------

class USBAudioDeviceTable 
{
public:
    USBAudioDeviceTable();
    ~USBAudioDeviceTable();

    VOID Lock()   { EnterCriticalSection(&m_Lock); }
    VOID Unlock() { LeaveCriticalSection(&m_Lock); }

    BOOL AddDevice(USBAudioDevice* AudioDevice); 
    BOOL RemoveDevice(USBAudioDevice* AudioDevice);

    DWORD GetNumDevices();
    USBAudioDevice* GetDevice(DWORD Index);
    
    DWORD GetNumInputs();
    DWORD GetNumOutputs();
    USBAudioInput* GetInput(DWORD Index);
    USBAudioOutput* GetOutput(DWORD Index);

private:
    DWORD m_NumDevices;
    DWORD m_NumInputs;
    DWORD m_NumOutputs;

    CRITICAL_SECTION m_Lock;

    USBAudioDevice* m_Devices[USBAUD_MAX_DEVICES];
};

// ----------------------------------------------------------------------------

class USBAudioDevice : public UsbClientDevice 
{
// Usage scenario:
//
// (1) Constructor
// (2) Add inputs and outputs
// (3) Destructor
public:
    virtual BOOL Init();
    virtual BOOL IsThisInterfaceSupported(LPCUSB_INTERFACE usbInterface);
    virtual BOOL IsClientDriverSatisfied();
    virtual BOOL Attach();
    virtual BOOL Detach();

public:
    USBAudioDevice(USB_HANDLE hUsb, LPCUSB_FUNCS UsbFuncsPtr,LPCUSB_INTERFACE lpInputInterface, LPCWSTR szUniqueDriverId,LPCUSB_DRIVER_SETTINGS lpDriverSettings,DWORD dwDebugZone = 0 );
    ~USBAudioDevice();
    
    DWORD GetNumInputs();
    DWORD GetNumOutputs();

    USBAudioInput* GetInput(DWORD Index);
    USBAudioOutput* GetOutput(DWORD Index);

    USB_HANDLE GetUSBDevice() { return m_hUsb; }

    BOOL AddInput(USBAudioInput* Input);
    BOOL AddOutput(USBAudioOutput* Output);
    
private:
    static BOOL WINAPI UsbDeviceNotifyStub( LPVOID lpvNotifyParameter, DWORD dwCode,LPDWORD * dwInfo1, LPDWORD * dwInfo2,
           LPDWORD * dwInfo3,LPDWORD * dwInfo4)
    {
        return ((USBAudioDevice *)lpvNotifyParameter)->UsbDeviceNotify(dwCode,dwInfo1, dwInfo2,dwInfo3,dwInfo4);
    }

	VOID AdvertiseInterface(BOOL fAdd);
    
	GUID IClass;
    DWORD m_NumInputs;
    DWORD m_NumOutputs;

    USBAudioInput* m_Inputs[USBAUD_MAX_LINES];
    USBAudioOutput* m_Outputs[USBAUD_MAX_LINES];

    BOOL m_bDeviceSatisfied;
};

// ----------------------------------------------------------------------------

class USBAudioPipe 
{
//  Usage scenario:
//  
//  (1) Constructor
//  (2) Set audio device
//  (3) Add supported rates
//  (4) Set bytes per sample
//  (5) Set number of channels
//  (6) Set current sample rate (doesn't set sample rate on actual USB device)
//  (7) Set USB pipe
//  (8) Wait for event to be signalled
//  (9) Call transfer
// (10) Go to (8)
// (11) Desctructor
public:
    USBAudioPipe();
    ~USBAudioPipe();

    WORD GetEndpointNum();
    USBAudioDevice* GetAudioDevice();
    DWORD GetNumSupportedRates();
    DWORD* GetSupportedRates();
    DWORD GetRate(DWORD Index);
    BOOL SetRate(DWORD Index, DWORD Hz);
    DWORD GetNumChannels();
    DWORD GetBytesPerSample();
    DWORD GetSampleRate();
	BOOL GetUsbRequest(BYTE RequestCode, BYTE FeatureUnit, BYTE ControlSelector, BYTE Channel, BYTE *out, BYTE outLen);
	BOOL SetUsbRequest(BYTE RequestCode, BYTE FeatureUnit, BYTE ControlSelector, BYTE Channel, BYTE *in, BYTE inLen);
    UsbAsyncClassPipe* GetUSBPipe();

    HANDLE GetTransferEvent();
    PBYTE GetBuffer();
    DWORD GetBufferSize();
    virtual BOOL TransferBuffer(DWORD NumBytes) = 0;

    VOID SetEndpointNum(WORD Num);
    VOID SetAudioDevice(USBAudioDevice* AudioDevice);
    BOOL AddSupportedRate(DWORD Rate);
    VOID SetNumChannels(DWORD Num);
    VOID SetBytesPerSample(DWORD Bps);
    VOID SetSampleRate(DWORD Rate);
    VOID SetUSBPipe(UsbAsyncClassPipe* Pipe);

    BOOL CloseAllArmedTransfer();
    BOOL CloseFrontArmedTransfer();

    VOID Reset();
    
    VOID SetDeviceContext(DeviceContext* Device);
    
    virtual VOID Start();
    VOID Stop();
    VOID SetDeviceId(DWORD DeviceId);
    DWORD GetDeviceId();
    BOOL InitAudioThread();
    void DeInitAudioThread();
    VOID AudioThread();

    static DWORD WINAPI TransferDone(LPVOID lpvNotifyParameter) 
    {
        HANDLE hEvent = (HANDLE) lpvNotifyParameter;
        SetEvent(hEvent);

        return TRUE;
    }
   
protected:
    VOID ReallocateBuffers();

    WORD m_EndpointNum;
    DWORD m_BytesPerSample;
    DWORD m_CurrSampleRate;
    DWORD m_NumChannels;
    DWORD m_NumSampleRates;
    DWORD m_SampleRates[USBAUD_MAX_RATES];
    USBAudioDevice* m_AudioDevice;
    HANDLE m_hTransferDone[2];
    USB_HANDLE m_hUsbTransfer[2];

    static DMA_ADAPTER_OBJECT m_Adapter;
    PHYSICAL_ADDRESS m_PhysicalAddress[2];
    DWORD m_BufferSize[2];
    BYTE* m_Buffer[2];
    
    DWORD* m_Lengths[2];
    int m_CurrBufferIndex;
    UsbAsyncClassPipe* m_hUSBPipe;

    HANDLE m_hAudioRunning;
    BOOL m_AudioPlaying;
    DWORD m_DeviceId;
    DeviceContext* m_DeviceContext;
    BOOL m_AudioThreadExit;
    HANDLE m_hAudioThread;
    HANDLE m_hDetachEvent;
};

// ----------------------------------------------------------------------------

class USBAudioInput : public USBAudioPipe 
{
private:
	BYTE m_MicFeatureUnitId;
	BOOL m_IsMicVolumeSupported;
	BOOL m_IsMicAGCSupported;
	BYTE m_MicVolumeChannel;
	BYTE m_MicAGCChannel;
	
	signed short m_MicVolumeMin;
	signed short m_MicVolumeMax;
	signed short m_MicVolumeRes;

public:
    USBAudioInput();
    ~USBAudioInput();

    VOID Start();
    BOOL TransferBuffer(DWORD NumBytes);
	DWORD GetMicGain();
	VOID  SetMicGain(DWORD Value);
	BOOL  GetMicAGC();
	VOID  SetMicAGC(BOOL Enable);
	BOOL  IsMicAGCSupported();
	VOID  ParseDescriptor(const LPCVOID Descriptor);
	VOID  GetCurrentValues();
	DWORD GetMicGainMin();
	DWORD GetMicGainMax();
	DWORD GetMicGainRes();
};

// ----------------------------------------------------------------------------

class USBAudioOutput : public USBAudioPipe 
{
public:
    USBAudioOutput();
    ~USBAudioOutput();

    VOID Start();
    BOOL TransferBuffer(DWORD NumBytes);
};

// ----------------------------------------------------------------------------


