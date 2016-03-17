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

#include "wavemain.h"
#include "usbaudiodevice.hpp"
#include "usbaudio.hpp"

// TODO: include "wavetopologyguids.h" instead
// {20375F48-7756-40dc-9520-28E96DE6B2E3}
DEFINE_GUID(WAVE_DEVCLASS_USBAUDIO, 
    0x20375f48, 0x7756, 0x40dc, 0x95, 0x20, 0x28, 0xe9, 0x6d, 0xe6, 0xb2, 0xe3);


// ----------------------------------------------------------------------------
//Global functions
// ----------------------------------------------------------------------------

BOOL USBDeviceAttach
(
    USB_HANDLE hDevice,                         // @parm [IN] - USB device handle
    LPCUSB_FUNCS lpUsbFuncs,                    // @parm [IN] - Pointer to USBDI function table
    LPCUSB_INTERFACE lpInterface,               // @parm [IN] - If client is being loaded as an interface
    LPCWSTR szUniqueDriverId,                   // @parm [IN] - Contains client driver id string
    LPBOOL fAcceptControl,                      // @parm [OUT]- Filled in with TRUE if we accept control
    LPCUSB_DRIVER_SETTINGS lpDriverSettings,    // @parm [IN] - Contains pointer to USB_DRIVER_SETTINGS
    DWORD dwUnused)                             // @parm [IN] - Reserved for use with future versions of USBD
{
    DEBUGMSG(ZONE_INIT, (TEXT("Device Attach\n")));

    PREFAST_ASSERT(fAcceptControl!=NULL)
    *fAcceptControl = FALSE;

    USBAudioDevice * pUsbDevice = new USBAudioDevice(hDevice, lpUsbFuncs, lpInterface, szUniqueDriverId, lpDriverSettings);
    if (pUsbDevice && pUsbDevice->Init())
    {
        *fAcceptControl = TRUE;
        DEBUGMSG(ZONE_INIT, (TEXT(">USBAudioDevice* = %p\n"), pUsbDevice));
    }
    else if (pUsbDevice)
    {
        delete pUsbDevice;
    }

    return TRUE;
};

INT compareDecreasing (const VOID * a, const VOID * b)
{
    DWORD x = *(DWORD*)a;
    DWORD y = *(DWORD*)b;
    if (x > y)
    {
        return -1;
    } else if (x < y)
    {
        return 1;
    } else
    {
        return 0;
    }
}

VOID CallAudioThread(USBAudioPipe *AudioPipe)
{
    AudioPipe->AudioThread();
}

// ----------------------------------------------------------------------------
// class USBAudioDeviceTable
// ----------------------------------------------------------------------------

USBAudioDeviceTable::USBAudioDeviceTable()
    : m_NumDevices(0), m_NumInputs(0), m_NumOutputs(0)
{
    InitializeCriticalSection(&m_Lock);
}

USBAudioDeviceTable::~USBAudioDeviceTable()
{
    DeleteCriticalSection(&m_Lock);
    // TODO: Delete all devices
}

BOOL USBAudioDeviceTable::AddDevice(USBAudioDevice* AudioDevice)
{
    if (AudioDevice == NULL)
    {
        return FALSE;
    }

    m_Devices[m_NumDevices++] = AudioDevice;
    m_NumInputs += AudioDevice->GetNumInputs();
    m_NumOutputs += AudioDevice->GetNumOutputs();

    return TRUE;
}

BOOL USBAudioDeviceTable::RemoveDevice(USBAudioDevice* AudioDevice)
{
    if (AudioDevice == NULL)
    {
        return FALSE;
    }

    return TRUE;
}

DWORD USBAudioDeviceTable::GetNumDevices()
{
    return m_NumDevices;
}

USBAudioDevice* USBAudioDeviceTable::GetDevice(DWORD Index)
{
    if (Index >= m_NumDevices)
    {
        return NULL;
    }
    else
    {
        return m_Devices[Index];
    }
}

DWORD USBAudioDeviceTable::GetNumInputs()
{
    return m_NumInputs;
}

DWORD USBAudioDeviceTable::GetNumOutputs()
{
    return m_NumOutputs;
}

USBAudioInput* USBAudioDeviceTable::GetInput(DWORD Index)
{
    DWORD count = 0;

    for (DWORD i = 0; i < m_NumDevices; i++)
    {
        DWORD nextcount = count + m_Devices[i]->GetNumInputs();
        if (count <= Index && Index < nextcount)
        {
            return m_Devices[i]->GetInput(Index - count);
        }
    }

    return NULL;
}

USBAudioOutput* USBAudioDeviceTable::GetOutput(DWORD Index)
{
    DWORD count = 0;

    for (DWORD i = 0; i < m_NumDevices; i++)
    {
        DWORD nextcount = count + m_Devices[i]->GetNumOutputs();
        if (count <= Index && Index < nextcount)
        {
            return m_Devices[i]->GetOutput(Index - count);
        }
    }

    return NULL;
}

// ----------------------------------------------------------------------------
//class USBAudioDevice
// ----------------------------------------------------------------------------

BOOL USBAudioDevice::Init()
{
    BOOL bReturn = FALSE;
    ASSERT(m_hDriver==NULL);
    DEBUGMSG(m_dwDebugZone,(TEXT("+UsbClientDevice::Init\n")));

    Lock();
    if (m_szUniqueDriverId &&m_hSetupComleteEvent!=NULL && Attach() &&
            RegisterNotificationRoutine(m_hUsb, UsbDeviceNotifyStub, this))
    {
        ASSERT(m_hDriver == NULL);
        TCHAR ClientRegistryPath[MAX_PATH];
        if (GetClientRegistryPath(ClientRegistryPath, MAX_PATH, m_szUniqueDriverId))
        {
            if (g_pHWContext==NULL)
            {
                ActivateDeviceEx(ClientRegistryPath, NULL, 0, NULL);
            }
            ASSERT(g_pHWContext!=NULL);
            bReturn = g_pHWContext->Attach(this);
        }
    }
    Unlock();
    DEBUGMSG(m_dwDebugZone,(TEXT("-UsbClientDevice::Init return %d\n"), bReturn));

    m_bDeviceSatisfied = FALSE;

    return bReturn;
}

BOOL USBAudioDevice::IsThisInterfaceSupported(LPCUSB_INTERFACE usbInterface)
{
    BOOL rc = FALSE;

    if (!usbInterface || usbInterface->Descriptor.bInterfaceClass != USB_AUD_INTERFACE_CLASS)
    {
        goto cleanUp;
    }

    if (usbInterface->Descriptor.bInterfaceSubClass == USB_AUD_CONTROL_INTERFACE_SUBCLASS &&
         usbInterface->Descriptor.bNumEndpoints == 0)
    {
        m_bDeviceSatisfied = TRUE;

#ifdef DEBUG
        BYTE aSubTypes[256];

        // dump terminal/unit descriptors
        DEBUGMSG(ZONE_USB_PARSE, (TEXT("------------------------------------------------\n")));
        LPBYTE curr = (LPBYTE) usbInterface->lpvExtended;
        LPUSB_AUDIO_CSI_DESCRIPTOR header = (LPUSB_AUDIO_CSI_DESCRIPTOR) curr;
        DUMP_USB_AUDIO_CSI_DESCRIPTOR((*header));
        LPBYTE last = curr + header->wTotalLength;
        curr += header->bLength;

        while (curr < last)
        {
            LPUSB_AUDIO_DUMMY_DESCRIPTOR dummy = (LPUSB_AUDIO_DUMMY_DESCRIPTOR)curr;

            PRINT_UNIT_DESCRIPTOR(dummy->bDescriptorSubType);
            aSubTypes[dummy->bUnitID] = dummy->bDescriptorSubType;

            if (dummy->bDescriptorSubType == OUTPUT_TERMINAL)
            {
                LPUSB_AUDIO_OTD_DESCRIPTOR otd = (LPUSB_AUDIO_OTD_DESCRIPTOR) curr;
                PRINT_TERMINAL_TYPE(otd->wTerminalType);
                DEBUGMSG(ZONE_USB_PARSE, (TEXT("bTerminalID = 0x%x\n"), otd->bTerminalID));
                DEBUGMSG(ZONE_USB_PARSE, (TEXT("bAssocTerminal = 0x%x\n"), otd->bAssocTerminal));
            }
            else if (dummy->bDescriptorSubType == INPUT_TERMINAL)
            {
                LPUSB_AUDIO_ITD_DESCRIPTOR itd = (LPUSB_AUDIO_ITD_DESCRIPTOR) curr;
                PRINT_TERMINAL_TYPE(itd->wTerminalType);
                DEBUGMSG(ZONE_USB_PARSE, (TEXT("bTerminalID = 0x%x\n"), itd->bTerminalID));
                DEBUGMSG(ZONE_USB_PARSE, (TEXT("bAssocTerminal = 0x%x\n"), itd->bAssocTerminal));
            }

            curr += dummy->bLength;
        }
        DEBUGMSG(ZONE_USB_PARSE, (TEXT("------------------------------------------------\n")));
#endif
    }
    else if (usbInterface->Descriptor.bInterfaceSubClass == USB_AUD_STREAMING_INTERFACE_SUBCLASS)
    {
        goto cleanUp;
    }
    else
    {
        goto cleanUp;
    }

    //
    // walk the interfaces searching for best fit
    //
    LPCUSB_DEVICE pDevice = GetDeviceInfo();
    DWORD dwNumInterfaces = pDevice->lpActiveConfig->dwNumInterfaces;
    LPCUSB_INTERFACE pUsbInterface = pDevice->lpActiveConfig->lpInterfaces;

    for (DWORD dwIndex = 0; dwIndex < dwNumInterfaces;)
    {
        // find best alternate setting
        DWORD dwCurrInterfaceNr = pUsbInterface->Descriptor.bInterfaceNumber;
        LPCUSB_INTERFACE pBestAlternateInterface = pUsbInterface;
        DWORD dwBestNumChannels = 0;
        DWORD dwBestBPS = 0;            // bytes per sample
        BOOL bFound = FALSE;

        do
        {
            if (pUsbInterface->Descriptor.bInterfaceClass == USB_AUD_INTERFACE_CLASS &&
                pUsbInterface->Descriptor.bInterfaceSubClass == USB_AUD_STREAMING_INTERFACE_SUBCLASS &&
                pUsbInterface->Descriptor.bNumEndpoints == 1)
            {
                LPUSB_AUDIO_AS_INTERFACE_DESCRIPTOR pASDescriptor = (LPUSB_AUDIO_AS_INTERFACE_DESCRIPTOR)pUsbInterface->lpvExtended;

                // TODO: correct the following line:
                LPUSB_AUDIO_AS_TYPE1_FORMAT_DESCRIPTOR pFmtDescriptor = (LPUSB_AUDIO_AS_TYPE1_FORMAT_DESCRIPTOR)(&pASDescriptor[1]);
#ifdef DEBUG
                DEBUGMSG(1, (TEXT("dwCurrInterfaceNr=%d pUsbInterface=0x%x Interface Nr=%d\n"), dwCurrInterfaceNr, pUsbInterface, pUsbInterface->Descriptor.bInterfaceNumber));
                DUMP_USB_AUDIO_AS_INTERFACE_DESCRIPTOR((*pASDescriptor));
                DUMP_USB_AUDIO_AS_TYPE1_FORMAT_DESCRIPTOR((*pFmtDescriptor));
                DEBUGMSG(1, (TEXT("----------------------------------------------------------\n")));
#endif
                if ((pASDescriptor->wFormatTag == 1 || pASDescriptor->wFormatTag == 2) && // PCM8 or PCM16
                    dwBestNumChannels <= pFmtDescriptor->bNumChannels && pFmtDescriptor->bNumChannels <= MAXCHANNELS &&
                    dwBestBPS <= pFmtDescriptor->bSubFrameSize && pFmtDescriptor->bSubFrameSize <= sizeof(HWSAMPLE))
                {
                    dwBestNumChannels = pFmtDescriptor->bNumChannels;
                    dwBestBPS = pFmtDescriptor->bSubFrameSize;
                    pBestAlternateInterface = pUsbInterface;

                    bFound = TRUE;
                }
            }

            pUsbInterface++; dwIndex++;
        }
        while (dwIndex < dwNumInterfaces && dwCurrInterfaceNr == pUsbInterface->Descriptor.bInterfaceNumber);

        if (bFound)
        {
            // Add line
            PCUSB_ENDPOINT pEndpoint = pBestAlternateInterface->lpEndpoints;
            ASSERT((pEndpoint->Descriptor.bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_ISOCHRONOUS);

            UsbAsyncClassPipe* USBPipe = new UsbAsyncClassPipe(*pEndpoint, this, 2, 0);
            USBPipe->Init();
            USBPipe->ResetTransferQueue();

            USBAudioPipe* lpAudioPipe;
            if (USB_ENDPOINT_DIRECTION_OUT(pEndpoint->Descriptor.bEndpointAddress))
            {
                lpAudioPipe = new USBAudioOutput();
                AddOutput((USBAudioOutput*) lpAudioPipe);
                DEBUGMSG(1, (TEXT("Adding Output!\n")));
            }
            else
            {
                lpAudioPipe = new USBAudioInput();
				((USBAudioInput*)lpAudioPipe)->ParseDescriptor(usbInterface->lpvExtended);
                AddInput((USBAudioInput*) lpAudioPipe);
                DEBUGMSG(1, (TEXT("Adding Input!\n")));
            }

            SetInterface(pBestAlternateInterface->Descriptor.bInterfaceNumber, pBestAlternateInterface->Descriptor.bAlternateSetting);
            lpAudioPipe->SetAudioDevice(this);

            // TODO: Add supported rates
            LPUSB_AUDIO_AS_INTERFACE_DESCRIPTOR pASDescriptor = (LPUSB_AUDIO_AS_INTERFACE_DESCRIPTOR)pBestAlternateInterface->lpvExtended;
            // TODO: correct the following line:
            LPUSB_AUDIO_AS_TYPE1_FORMAT_DESCRIPTOR pFmtDescriptor = (LPUSB_AUDIO_AS_TYPE1_FORMAT_DESCRIPTOR)(&pASDescriptor[1]);

            PBYTE pTable = (PBYTE)&(pFmtDescriptor->tSamFreq);
            DWORD SampleRate = USBAUD_MAX_SAMPLERATE;

            if (pFmtDescriptor->bSamFreqType == 0) {  // continuous sampling frequency
                DWORD tLowerSamFreq = (pTable[2] << 16) | (pTable[1] << 8) | pTable[0];
                DWORD tUpperSamFreq = (pTable[5] << 16) | (pTable[4] << 8) | pTable[3];
                SampleRate = tUpperSamFreq < SampleRate ? tUpperSamFreq : SampleRate;
                lpAudioPipe->AddSupportedRate(SampleRate);
                DEBUGMSG(1, (TEXT("Supported sample rates from %i to %i\n"), tLowerSamFreq, tUpperSamFreq));
            }
            else
            {   // discrete sampling frequencies
                DWORD NumRates = (pFmtDescriptor->bLength - 8) / 3;
                DWORD *pRate = new DWORD[NumRates];
                PBYTE pCurr = pTable;

                for (DWORD i = 0; i < NumRates; i++, pCurr += 3)
                {
                    pRate[i] =  (((DWORD) pCurr[2]) << 16) | (((DWORD) pCurr[1]) << 8) | ((DWORD) pCurr[0]);
                    lpAudioPipe->AddSupportedRate(pRate[i]);
                    DEBUGMSG(1, (TEXT("Supported sample rate[%i]: %i\n"), i,  pRate[i]));
                }
                qsort (pRate, NumRates, sizeof(DWORD), compareDecreasing);
                for (DWORD i = 0; i < NumRates; i++)
                {
                    if (pRate[i] <= SampleRate)
                    {
                        SampleRate = pRate[i];
                        break;
                    }
                }

                delete [] pRate;
            }

            DEBUGMSG(1, (TEXT("Selected audio settings: %i BPS, %i channels, %i Hz\n"), dwBestBPS,  dwBestNumChannels, SampleRate));

            lpAudioPipe->SetBytesPerSample(dwBestBPS);
            lpAudioPipe->SetNumChannels(dwBestNumChannels);
            lpAudioPipe->SetUSBPipe(USBPipe);
            lpAudioPipe->SetEndpointNum(pEndpoint->Descriptor.bEndpointAddress);
            lpAudioPipe->SetSampleRate(SampleRate);
			if (USB_ENDPOINT_DIRECTION_IN(pEndpoint->Descriptor.bEndpointAddress))
				((USBAudioInput*)lpAudioPipe)->GetCurrentValues();
        }

        //pUsbInterface++; dwIndex++;
    }

    rc = TRUE;

cleanUp:
    return rc;
}

BOOL USBAudioDevice::IsClientDriverSatisfied()
{
    return m_bDeviceSatisfied;
}

BOOL USBAudioDevice::Attach()
{
    DEBUGMSG(m_dwDebugZone, (TEXT("UsbClientDevice::Attach\n")));
    BOOL bReturn = FALSE;

    if (m_lpInputInterface != NULL)
    {   // This driver is loaded by Interface
        if (IsThisInterfaceSupported(m_lpInputInterface))
        {
            DEBUGMSG(m_dwDebugZone, (TEXT("UsbClientDevice::Attach(InterfaceOnly): Found Supported Interface (bInterfaceNumber=%d, bAlternateSetting=%d\n"),
                m_lpInputInterface->Descriptor.bInterfaceNumber, m_lpInputInterface->Descriptor.bAlternateSetting));
            bReturn = TRUE;
        }
    }

	AdvertiseInterface(TRUE);
    return bReturn;
}

BOOL USBAudioDevice::Detach()
{
    DEBUGMSG(m_dwDebugZone,(TEXT("UsbClientDevice::Detach")));
	AdvertiseInterface(FALSE);
    Lock();
    ASSERT(g_pHWContext!=NULL);
    g_pHWContext->Detach();
    Unlock();
    return TRUE;
}

BOOL ConvertStringToGuid(LPCTSTR GuidString, GUID *Guid )
{
    UINT Data4[8];
    int  Count;
    BOOL Ok = FALSE;
    LPWSTR GuidFormat = L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}";

	if (!GuidString || !Guid)
		return Ok;

    __try 
    {

      if (_stscanf_s(GuidString, GuidFormat, &Guid->Data1, 
        &Guid->Data2, &Guid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3], 
        &Data4[4], &Data4[5], &Data4[6], &Data4[7]) == 11) 
      {

          for (Count = 0; Count < (sizeof(Data4) / sizeof(Data4[0])); Count++) 
          {
              Guid->Data4[Count] = (UCHAR) Data4[Count];
          }
      }

      Ok = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) 
    {
        RETAILMSG(1, (TEXT("Exception in convertstringtoguid\r\n")));
    }

    return Ok;
}

USBAudioDevice::USBAudioDevice(
    USB_HANDLE hUsb,
    LPCUSB_FUNCS UsbFuncsPtr,
    LPCUSB_INTERFACE lpInputInterface,
    LPCWSTR szUniqueDriverId,
    LPCUSB_DRIVER_SETTINGS lpDriverSettings,
    DWORD dwDebugZone
    )
    : UsbClientDevice(hUsb, UsbFuncsPtr, lpInputInterface, szUniqueDriverId, lpDriverSettings, dwDebugZone),
    m_NumInputs(0),
    m_NumOutputs(0)
#if 0
    ,
    m_AudioControl(NULL)
#endif
{
	HKEY hKey;
    DWORD dwType;
    DWORD cbData = 0;

	memset((LPVOID)&IClass, 0, sizeof(IClass)); 
	LONG lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, CLIENT_REGKEY_SZ, 0, 0, &hKey);
    if (ERROR_SUCCESS == lStatus)
	{	
		if (RegQueryValueEx(hKey, ICLASS_NAME_SZ, NULL, &dwType, NULL, &cbData ) == ERROR_SUCCESS && dwType == REG_MULTI_SZ && cbData > 0)
		{
			LPCTSTR lpValue = new TCHAR[cbData];
			if (lpValue)
			{
				DWORD dwStrLen = cbData;
				if (RegQueryValueEx( hKey, ICLASS_NAME_SZ, NULL, &dwType, (LPBYTE)lpValue, &dwStrLen ) == ERROR_SUCCESS && dwType == REG_MULTI_SZ && dwStrLen == cbData )
					ConvertStringToGuid(lpValue, &IClass);

				delete [] lpValue;
			}

			RegCloseKey(hKey);
		}
	}
}

USBAudioDevice::~USBAudioDevice()
{
    // delete all audio pipes
    for(DWORD i = 0; i < m_NumInputs; i++)
    {
        delete m_Inputs[i];
        m_Inputs[i] = NULL;
    }
    for(DWORD i = 0; i < m_NumOutputs; i++)
    {
        delete m_Outputs[i];
        m_Outputs[i] = NULL;
    }
}

DWORD USBAudioDevice::GetNumInputs()
{
    return m_NumInputs;
}

DWORD USBAudioDevice::GetNumOutputs()
{
    return m_NumOutputs;
}

USBAudioInput* USBAudioDevice::GetInput(DWORD Index)
{
    if (Index >= m_NumInputs)
    {
        return NULL;
    }
    else
    {
        return m_Inputs[Index];
    }
}

USBAudioOutput* USBAudioDevice::GetOutput(DWORD Index)
{
    if (Index >= m_NumOutputs)
    {
        return NULL;
    }
    else
    {
        return m_Outputs[Index];
    }
}

BOOL USBAudioDevice::AddInput(USBAudioInput* Input)
{
    if (m_NumInputs == USBAUD_MAX_LINES)
    {
        return FALSE;
    }
    else
    {
        m_Inputs[m_NumInputs++] = Input;
        return TRUE;
    }
}

BOOL USBAudioDevice::AddOutput(USBAudioOutput* Output)
{
    if (m_NumOutputs >= USBAUD_MAX_LINES)
    {
        return FALSE;
    }
    else
    {
        m_Outputs[m_NumOutputs++] = Output;
        return TRUE;
    }
}

VOID USBAudioDevice::AdvertiseInterface(BOOL fAdd)
{
	::AdvertiseInterface(&WAVE_DEVCLASS_USBAUDIO, _T("HDS1:"), fAdd);
	/*
	RETAILMSG(!IClass.Data1, (TEXT("USBAudioDevice::AdvertiseInterface, Invalid IClass value!\n")));

	if (IClass.Data1 && g_pHWContext)
	{
		::AdvertiseInterface(&IClass, g_pHWContext->GetDeviceName(), fAdd);
	}
	*/
}

// ----------------------------------------------------------------------------
// class USBAudioPipe
// ----------------------------------------------------------------------------

DMA_ADAPTER_OBJECT USBAudioPipe::m_Adapter = { sizeof(DMA_ADAPTER_OBJECT), Internal, 0 };

USBAudioPipe::USBAudioPipe() : m_BytesPerSample(sizeof(HWSAMPLE)), m_NumChannels(MAXCHANNELS), m_NumSampleRates(0),
    m_AudioDevice(NULL), m_CurrBufferIndex(0), m_hUSBPipe(NULL), m_CurrSampleRate(USBAUD_MAX_SAMPLERATE),
    m_hAudioRunning(NULL), m_AudioPlaying(FALSE), m_DeviceId(0), m_hAudioThread(NULL), m_AudioThreadExit(FALSE), m_DeviceContext(NULL),
    m_hDetachEvent(NULL)
{
    m_hUsbTransfer[0] = m_hUsbTransfer[1] = NULL;
    m_Buffer[0] = m_Buffer[1] = NULL;

    m_hTransferDone[0] = CreateEvent(NULL, TRUE, TRUE, NULL);   // signalled initially
    m_hTransferDone[1] = CreateEvent(NULL, TRUE, TRUE, NULL);   // signalled initially
    m_hAudioRunning = CreateEvent(NULL, TRUE, FALSE, NULL);     // unsignalled initially
    m_hDetachEvent = CreateEvent(NULL, FALSE, FALSE, NULL);     // unsignalled initially

    m_Lengths[0] = new DWORD[USBAUD_FRAMES];
    m_Lengths[1] = new DWORD[USBAUD_FRAMES];

    ReallocateBuffers();
}

USBAudioPipe::~USBAudioPipe()
{
    for(int i = 0; i < 2; i++)
    {
        delete m_Buffer[i];
        delete m_Lengths[i];
        CloseHandle(m_hTransferDone[i]);
    }

    CloseHandle(m_hAudioRunning);
    CloseHandle(m_hDetachEvent);
    m_hDetachEvent = NULL;
    
    delete m_hUSBPipe;
    m_hUSBPipe = NULL;
}

WORD USBAudioPipe::GetEndpointNum()
{
    return m_EndpointNum;
}

USBAudioDevice* USBAudioPipe::GetAudioDevice()
{
    return m_AudioDevice;
}

DWORD USBAudioPipe::GetNumSupportedRates()
{
    return m_NumSampleRates;
}

DWORD* USBAudioPipe::GetSupportedRates()
{
    return m_SampleRates;
}

DWORD USBAudioPipe::GetRate(DWORD Index)
{
    if (Index >= m_NumSampleRates)
    {
        return 0;
    }
    else
    {
        return m_SampleRates[Index];
    }
}

BOOL USBAudioPipe::SetRate(DWORD Index, DWORD Hz)
{
    if (Index < m_NumSampleRates)
    {
        m_SampleRates[Index] = Hz;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

DWORD USBAudioPipe::GetNumChannels()
{
    return m_NumChannels;
}

DWORD USBAudioPipe::GetBytesPerSample()
{
    return m_BytesPerSample;
}

DWORD USBAudioPipe::GetSampleRate()
{
    return m_CurrSampleRate;
}

UsbAsyncClassPipe* USBAudioPipe::GetUSBPipe()
{
    return m_hUSBPipe;
}

HANDLE USBAudioPipe::GetTransferEvent()
{
    return m_hUSBPipe->GetCurrentWaitObjectHandle();
}

PBYTE USBAudioPipe::GetBuffer()
{
    return m_Buffer[m_CurrBufferIndex];
}

DWORD USBAudioPipe::GetBufferSize()
{
    return (m_CurrSampleRate * m_BytesPerSample * m_NumChannels * USBAUD_FRAMES) / 1000;
}

VOID USBAudioPipe::SetEndpointNum(WORD Num)
{
    m_EndpointNum = Num;
}

VOID USBAudioPipe::SetAudioDevice(USBAudioDevice* AudioDevice)
{
    m_AudioDevice = AudioDevice;
}

BOOL USBAudioPipe::AddSupportedRate(DWORD Rate)
{
    if (m_NumSampleRates >= USBAUD_MAX_RATES)
    {
        return FALSE;
    }
    else
    {
        m_SampleRates[m_NumSampleRates++] = Rate;
        return TRUE;
    }
}

VOID USBAudioPipe::SetNumChannels(DWORD Num)
{
    m_NumChannels = Num;
}

VOID USBAudioPipe::SetBytesPerSample(DWORD Bps)
{
    m_BytesPerSample = Bps;
}

// Assumption: sample rate is valid, buffers are not being used
VOID USBAudioPipe::SetSampleRate(DWORD Rate)
{
    m_CurrSampleRate = Rate;
    if (m_hUSBPipe != NULL)
    {
        USB_DEVICE_REQUEST ControlHeader;
        BYTE aData[3];

        // set sampling frequency
        ControlHeader.bmRequestType = USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_CLASS | USB_REQUEST_FOR_ENDPOINT;
        ControlHeader.bRequest = SET_CUR;
        ControlHeader.wValue = SAMPLING_FREQ_CONTROL << 8;
        ControlHeader.wIndex = m_EndpointNum;   // endpoint number
        ControlHeader.wLength = 3;              // length of data

        aData[0] = (BYTE) (Rate & 0xFF);
        aData[1] = (BYTE) ((Rate >> 8) & 0xFF);
        aData[2] = (BYTE) ((Rate >> 16) & 0xFF);

        BOOL bRc = m_AudioDevice->IssueVendorTransfer(USB_OUT_TRANSFER | USB_SEND_TO_ENDPOINT, //flags
                                                          &ControlHeader,
                                                          aData, 0);
        if (!bRc)
        {
            DEBUGMSG(1, (TEXT("IssueVendorTransfer error: %d\n"), GetLastError()));
        }
        else
        {
            DEBUGMSG(1, (TEXT("Sampling frequency set successfully!\n")));
        }

        //
        // get sampling frequency
        //
        ControlHeader.bmRequestType = USB_REQUEST_DEVICE_TO_HOST | USB_REQUEST_CLASS | USB_REQUEST_FOR_ENDPOINT;
        ControlHeader.bRequest = GET_CUR;
        ControlHeader.wValue = SAMPLING_FREQ_CONTROL << 8;
        ControlHeader.wIndex = 1;     // endpoint
        ControlHeader.wLength = 3;    // length of data

        aData[0] = 0x00;
        aData[1] = 0x00;
        aData[2] = 0x00;

        bRc = m_AudioDevice->IssueVendorTransfer(USB_IN_TRANSFER | USB_SEND_TO_ENDPOINT, //flags
                                                      &ControlHeader,
                                                      aData, 0);

        DWORD dwCurFreq = (((DWORD)aData[2])<< 16) | (((DWORD)aData[1])<< 8) | ((DWORD)aData[0]);

        if (!bRc)
        {
            DEBUGMSG(1, (TEXT("IssueVendorTransfer error:%d\n"), GetLastError()));
        }
        else
        {
            DEBUGMSG(1, (TEXT("Sampling frequency read successfully: %i\n"), dwCurFreq));
        }
    }

    ReallocateBuffers();
}

VOID USBAudioPipe::SetUSBPipe(UsbAsyncClassPipe* Pipe)
{
    m_hUSBPipe = Pipe;
}

BOOL USBAudioPipe::CloseAllArmedTransfer()
{
    if (m_hUSBPipe == NULL)
    {
        return TRUE;
    }
    else
    {
        return m_hUSBPipe->CloseAllArmedTransfer();
    }
}

BOOL USBAudioPipe::CloseFrontArmedTransfer()
{
    if (m_hUSBPipe == NULL)
    {
        return TRUE;
    }
    else
    {
        return m_hUSBPipe->CloseFrontArmedTransfer();
    }
}

VOID USBAudioPipe::Reset()
{
    m_hUSBPipe->WaitForTransferComplete(100, 0, NULL);
    m_hUSBPipe->CloseAllArmedTransfer();
    m_hUSBPipe->ResetTransferQueue();
}

VOID USBAudioPipe::SetDeviceContext(DeviceContext* Device)
{
    m_DeviceContext = Device;
}

VOID USBAudioPipe::Start()
{
    if (!m_AudioPlaying)
    {
        m_AudioPlaying = TRUE;
        SetEvent(m_hAudioRunning);
    }
}

VOID USBAudioPipe::Stop()
{
    if (m_AudioPlaying)
    {
        ResetEvent(m_hAudioRunning);
        m_AudioPlaying = FALSE;
        Reset();
    }
}

VOID USBAudioPipe::SetDeviceId(DWORD DeviceId)
{
    m_DeviceId = DeviceId;
}

DWORD USBAudioPipe::GetDeviceId()
{
    return m_DeviceId;
}

BOOL USBAudioPipe::InitAudioThread()
{
    m_hAudioThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE)CallAudioThread,
                                    this,
                                    0,
                                    NULL);
    if (!m_hAudioThread)
    {
        DEBUGMSG(1, (TEXT("Unable to create audio thread\n")));
        return FALSE;
    }

    // Bump up the priority since the interrupts must be serviced immediately.
    CeSetThreadPriority(m_hAudioThread, USBAUD_THREAD_PRIORITY);
    return TRUE;
}

VOID USBAudioPipe::DeInitAudioThread()
{
    if (m_hAudioThread != NULL) {
        m_AudioThreadExit = TRUE;
        SetEvent(m_hDetachEvent);
        WaitForSingleObject(m_hAudioThread, INFINITE);
        CloseHandle(m_hAudioThread);
    }
}

VOID USBAudioPipe::AudioThread()
{
    DEBUGMSG(1, (TEXT("USBAudioPipe::AudioThread()\n")));

    DWORD dwRunThread = 0;
    // Detach needs to be handled first if both are signalled.
    HANDLE hRunThread[ 2 ] = {m_hDetachEvent, m_hAudioRunning};
    while (!m_AudioThreadExit)
    {
        dwRunThread = WaitForMultipleObjects(2, hRunThread, FALSE, INFINITE);
        if(dwRunThread == (WAIT_OBJECT_0 + 1))
        {
            // At this point is still possible to get m_hDetachEvent at the same time as a transfer
            // This will cause a hang in TransferBuffer below as the TransferComplete event
            // supercedes the detach event
            if (m_hUSBPipe->WaitForTransferComplete(INFINITE, 1, &m_hDetachEvent) == FALSE)
            {
                // Got m_hDetachEvent
                DEBUGMSG (1, (TEXT("USBAudioPipe::AudioThread() detach event detected!\r\n")));
                continue;
            }

            DWORD BytesTransferred;

            // render audio stream to buffer
            PBYTE pBufferStart = GetBuffer();
            PBYTE pBufferEnd = pBufferStart + GetBufferSize();
            PBYTE pBufferLast = pBufferStart;
            TRANSFER_STATUS TransferStatus = {0};
            pBufferLast = m_DeviceContext->TransferBuffer(pBufferStart, pBufferEnd, &TransferStatus);
            BytesTransferred = pBufferLast - pBufferStart;

            if (BytesTransferred == 0)
            {
                Stop();
                continue;
            }

            // transfer stream via USB
            CloseFrontArmedTransfer();
            TransferBuffer(BytesTransferred);
        }
        else if(dwRunThread == WAIT_OBJECT_0)
        {
            // Got m_hDetachEvent
            DEBUGMSG (1, (TEXT("USBAudioPipe::AudioThread() detach event detected!\r\n")));
            continue;
        }
        else
        {
            DEBUGMSG (1, (TEXT("USBAudioPipe::AudioThread() fail in WaitForMultipleObjects\r\n")));
            continue;
        }
    }
}

// Assumption: buffers are not being used
VOID USBAudioPipe::ReallocateBuffers()
{
    for(int i = 0; i < 2; i++)
    {
        delete m_Buffer[i];
        m_Buffer[i] = new BYTE[GetBufferSize()];
        m_BufferSize[i] = GetBufferSize();
    }
}

BOOL USBAudioPipe::GetUsbRequest(BYTE RequestCode, BYTE FeatureUnit, BYTE ControlSelector, BYTE Channel, BYTE *out, BYTE outLen)
{
	USB_DEVICE_REQUEST ControlHeader;
	BOOL bRc = FALSE;
	if (m_hUSBPipe)
	{
		ControlHeader.bmRequestType = USB_REQUEST_DEVICE_TO_HOST | USB_REQUEST_CLASS | USB_REQUEST_FOR_INTERFACE;
        ControlHeader.bRequest = RequestCode;
        ControlHeader.wValue = (ControlSelector << 8) + Channel;
        ControlHeader.wIndex = FeatureUnit << 8; // TODO: interface is always 0?
        ControlHeader.wLength = outLen;    // length of data

        bRc = m_AudioDevice->IssueVendorTransfer(USB_IN_TRANSFER | USB_SEND_TO_ENDPOINT, //flags
                                                 &ControlHeader,
                                                 out, 0);
	}

	RETAILMSG(0, (TEXT("USBAudioPipe::GetUsbRequest(bRequest=0x%x, wValue=0x%x, wIndex=0x%x) returned %s\n"), RequestCode, ControlHeader.wValue, ControlHeader.wIndex, bRc ? L"OK" : L"ERROR"));
	return bRc;
}

BOOL USBAudioPipe::SetUsbRequest(BYTE RequestCode, BYTE FeatureUnit, BYTE ControlSelector, BYTE Channel, BYTE *in, BYTE inLen)
{
	USB_DEVICE_REQUEST ControlHeader;
	BOOL bRc = FALSE;
	if (m_hUSBPipe)
	{
		ControlHeader.bmRequestType = USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_CLASS | USB_REQUEST_FOR_INTERFACE;
        ControlHeader.bRequest = RequestCode;
        ControlHeader.wValue = (ControlSelector << 8) + Channel;
        ControlHeader.wIndex = FeatureUnit << 8; // TODO: interface is always 0?
        ControlHeader.wLength = inLen;     

		bRc = m_AudioDevice->IssueVendorTransfer(USB_OUT_TRANSFER | USB_SEND_TO_ENDPOINT, //flags
                                                 &ControlHeader,
                                                 in, 0);
	}

	RETAILMSG(0, (TEXT("USBAudioPipe::SetUsbRequest(bRequest=0x%x, wValue=0x%x, wIndex=0x%x) returned %s\n"), RequestCode, ControlHeader.wValue, ControlHeader.wIndex, bRc ? L"OK" : L"ERROR"));
	return bRc;
}

// ----------------------------------------------------------------------------
// class USBAudioInput : public USBAudioPipe
// ----------------------------------------------------------------------------

USBAudioInput::USBAudioInput()
{
	m_MicFeatureUnitId = 0;
	m_IsMicAGCSupported = FALSE;
	m_IsMicVolumeSupported = FALSE;
	m_MicVolumeChannel = 0;
	m_MicAGCChannel = 0;
	m_MicVolumeMin = 0;
	m_MicVolumeMax = 0;
	m_MicVolumeRes = 0;
}

USBAudioInput::~USBAudioInput()
{
}

VOID USBAudioInput::Start()
{
    if(!m_AudioPlaying)
    {
        BOOL success;
        DWORD count = 0;
        do
        {
            // transfer stream via USB
            success = TransferBuffer(GetBufferSize());
            count++;
        }
        while (success && count < 2);

        USBAudioPipe::Start();
    }
}

BOOL USBAudioInput::TransferBuffer(DWORD NumBytes)
{
    BOOL bRc = FALSE;

    if (m_hUSBPipe != NULL  && NumBytes != 0)
    {
        DWORD FrameSize = m_CurrSampleRate / 1000 * m_BytesPerSample * m_NumChannels;
        DWORD dwFrames = USBAUD_FRAMES;
        LPVOID lpvBuffer = m_Buffer[m_CurrBufferIndex];

        for (DWORD i = 0; i < dwFrames; i++)
        {
            m_Lengths[m_CurrBufferIndex][i] = FrameSize;
        }


        DEBUGMSG(1, (TEXT("TransferBuffer (Input): NumBytes = %i, dwFrames = %i, FrameSize = %i\n"),
            NumBytes, dwFrames, FrameSize));

        LPDWORD lpdwLengths = m_Lengths[m_CurrBufferIndex];

        bRc = m_hUSBPipe->IsochTransfer(USB_START_ISOCH_ASAP | USB_NO_WAIT | USB_IN_TRANSFER, // transfer flags
                                           0,               // starting frame
                                           dwFrames,        // number of frames
                                           lpdwLengths,     // lengths
                                           lpvBuffer,       // data buffer
                                           NULL,            // physical address
                                           0);
        if (!bRc)
        {
            DEBUGMSG(1, (TEXT("IsochTransfer failed!\n")));
        }
        else
        {
            m_CurrBufferIndex ^= 1;     // flip buffer
        }
    }

    return bRc;
}

DWORD USBAudioInput::GetMicGain()
{
	signed short steps = (m_MicVolumeMax + (0 - m_MicVolumeMin)) / m_MicVolumeRes;
	signed short gain;
	GetUsbRequest(GET_CUR, m_MicFeatureUnitId, VOLUME_CONTROL, m_MicVolumeChannel, (LPBYTE)&gain, sizeof(gain));
	RETAILMSG(0, (TEXT("* USBAudioInput::GetMicGain returned 0x%x\n"), gain));
	return ((gain - m_MicVolumeMin) / m_MicVolumeRes) * 100 / steps;
}

VOID USBAudioInput::SetMicGain(DWORD Rate)
{
	signed short steps = (m_MicVolumeMax + (0 - m_MicVolumeMin)) / m_MicVolumeRes;
	signed short gain = ((((short)Rate * steps) / 100) * m_MicVolumeRes) + m_MicVolumeMin;
	/*BOOL b = */SetUsbRequest(SET_CUR, m_MicFeatureUnitId, VOLUME_CONTROL, m_MicVolumeChannel, (LPBYTE)&gain, sizeof(gain));
	//RETAILMSG(0, (TEXT("* USBAudioInput::SetMicGain(0x%X) returned %d\n"), gain, b));
}

BOOL USBAudioInput::GetMicAGC()
{
	BYTE state;
	GetUsbRequest(GET_CUR, m_MicFeatureUnitId, AUTOMATIC_GAIN_CONTROL, m_MicAGCChannel, (LPBYTE)&state, sizeof(state));
	RETAILMSG(0, (TEXT("* USBAudioInput::GetMicAGC returned 0x%x\n"), state));
	return (BOOL)state;
}

VOID USBAudioInput::SetMicAGC(BOOL Enable)
{
	BYTE state = Enable ? 1 : 0;
	/*BOOL b = */SetUsbRequest(SET_CUR, m_MicFeatureUnitId, AUTOMATIC_GAIN_CONTROL, m_MicAGCChannel, (LPBYTE)&state, sizeof(state));
	//RETAILMSG(0, (TEXT("* USBAudioInput::SetMicAGC(0x%X) returned %d\n"), Enable, b));
}

BOOL USBAudioInput::IsMicAGCSupported()
{
	return m_IsMicAGCSupported;
}

DWORD USBAudioInput::GetMicGainMin() 
{ 
	return 0; 
}

DWORD USBAudioInput::GetMicGainMax() 
{ 
	return 100; 
}

DWORD USBAudioInput::GetMicGainRes() 
{ 
	return 1; 
}

LPUSB_AUDIO_DUMMY_DESCRIPTOR FindUnit(const LPCVOID Descriptor, BYTE UnitID)
{
	LPBYTE curr = (LPBYTE)Descriptor;
	LPUSB_AUDIO_CSI_DESCRIPTOR header = (LPUSB_AUDIO_CSI_DESCRIPTOR)curr;
    LPBYTE last = curr + header->wTotalLength;
    curr += header->bLength;
	while (curr < last)
	{
		LPUSB_AUDIO_DUMMY_DESCRIPTOR dummy = (LPUSB_AUDIO_DUMMY_DESCRIPTOR)curr;

		if (dummy->bUnitID == UnitID)
			return dummy;

		curr += dummy->bLength;
	}

	return NULL;
}

LPUSB_AUDIO_DUMMY_TD_DESCRIPTOR FindFirstTerminal(const LPCVOID Descriptor, BYTE SubType, WORD TerminalType)
{
	LPBYTE curr = (LPBYTE)Descriptor;
	LPUSB_AUDIO_CSI_DESCRIPTOR header = (LPUSB_AUDIO_CSI_DESCRIPTOR)curr;
    LPBYTE last = curr + header->wTotalLength;
    curr += header->bLength;
	while (curr < last)
	{
		LPUSB_AUDIO_DUMMY_DESCRIPTOR dummy = (LPUSB_AUDIO_DUMMY_DESCRIPTOR)curr;

		// TODO: check parameters validity
		if (dummy->bDescriptorSubType == SubType)
		{
			LPUSB_AUDIO_DUMMY_TD_DESCRIPTOR terminal = (LPUSB_AUDIO_DUMMY_TD_DESCRIPTOR)dummy;
			if (terminal->wTerminalType == TerminalType)
				return terminal;
		}

		curr += dummy->bLength;
	}

	return NULL;
}

VOID USBAudioInput::ParseDescriptor(const LPCVOID Descriptor)
{
	LPUSB_AUDIO_DUMMY_DESCRIPTOR dummy;
	LPUSB_AUDIO_OTD_DESCRIPTOR ot = (LPUSB_AUDIO_OTD_DESCRIPTOR)FindFirstTerminal(Descriptor, OUTPUT_TERMINAL, TERMINAL_STREAMING);
	if (ot)
	{
		BOOL done = FALSE;
		BYTE src = ot->bSourceID;
		while ((dummy = FindUnit(Descriptor, src)) && !done)
		{
			if (dummy->bDescriptorSubType == MIXER_UNIT)
			{
				LPUSB_AUDIO_MIXER_DESCRIPTOR mixer = (LPUSB_AUDIO_MIXER_DESCRIPTOR)dummy;
				if (mixer->bNumInPins)
					src = ((PBYTE)&(mixer->baSourceID))[0];
				else
					break;
			}
			else if (dummy->bDescriptorSubType == SELECTOR_UNIT)
			{
				LPUSB_AUDIO_SELECTOR_DESCRIPTOR selector = (LPUSB_AUDIO_SELECTOR_DESCRIPTOR)dummy;
				if (selector->bNumInPins)
					src = ((PBYTE)&(selector->baSourceID))[0];
				else
					break;
			}
			else if (dummy->bDescriptorSubType == FEATURE_UNIT)
				{
					LPUSB_AUDIO_FEATURE_DESCRIPTOR feature = (LPUSB_AUDIO_FEATURE_DESCRIPTOR)dummy;
					m_MicFeatureUnitId = feature->bUnitID;
					RETAILMSG(1, (TEXT("\nUSBAudioInput::ParseDescriptor, m_MicFeatureUnitId = 0x%x\n"), m_MicFeatureUnitId));

					if (feature->bControlSize)
					{
						int numLogicalChannels = (feature->bLength - 7) / feature->bControlSize;
						for (int i = 0; i < numLogicalChannels; i++)
						{
							BYTE mask = ((PBYTE)&(feature->bmaControls))[i * feature->bControlSize];
							// VOLUME_CONTROL
							if ((mask & 0x02) && !m_IsMicVolumeSupported) 
							{
								m_IsMicVolumeSupported = TRUE;
								m_MicVolumeChannel = i;
							}
							// AUTOMATIC_GAIN_CONTROL
							if ((mask & 0x40) && !m_IsMicAGCSupported) 
							{
								m_IsMicAGCSupported = TRUE;
								m_MicAGCChannel = i;
							}

							RETAILMSG(1, (TEXT("\nUSBAudioInput::ParseDescriptor, ch = 0x%x, mask = 0x%x\n"), i, mask));
						}

						RETAILMSG(1, (TEXT("\nUSBAudioInput::ParseDescriptor, mic volume is %s, ch = %d\n"), m_IsMicVolumeSupported ? L"supported" : L"not supported", m_MicVolumeChannel));
						RETAILMSG(1, (TEXT("\nUSBAudioInput::ParseDescriptor, mic AGC is %s, ch = %d\n"), m_IsMicAGCSupported ? L"supported" : L"not supported", m_MicAGCChannel));
					}

					done = TRUE;
					break;
				}
			else
				break;
		}
	}
}

VOID USBAudioInput::GetCurrentValues()
{
	BYTE value[2] = {0};

	if (!GetUsbRequest(GET_MIN, m_MicFeatureUnitId, VOLUME_CONTROL, m_MicVolumeChannel, (LPBYTE)&m_MicVolumeMin, sizeof(m_MicVolumeMin)) ||
		!GetUsbRequest(GET_MAX, m_MicFeatureUnitId, VOLUME_CONTROL, m_MicVolumeChannel, (LPBYTE)&m_MicVolumeMax, sizeof(m_MicVolumeMax)) ||
		!GetUsbRequest(GET_RES, m_MicFeatureUnitId, VOLUME_CONTROL, m_MicVolumeChannel, (LPBYTE)&m_MicVolumeRes, sizeof(m_MicVolumeRes)))
	{
		m_MicVolumeMin = 0;
		m_MicVolumeMax = 1;
		m_MicVolumeRes = 1;
	}
	else if (m_MicVolumeMin == m_MicVolumeMax || m_MicVolumeMax == m_MicVolumeRes)
	{
		m_MicVolumeMin = 0;
		m_MicVolumeMax = 1;
		m_MicVolumeRes = 1;
	}

	RETAILMSG(1, (TEXT("USBAudioInput::GetCurrentValues, MIC min=0x%x, max=0x%x, res=0x%x\n"), m_MicVolumeMin, m_MicVolumeMax, m_MicVolumeRes));
}

// ----------------------------------------------------------------------------
// class USBAudioOutput : public USBAudioPipe
// ----------------------------------------------------------------------------

USBAudioOutput::USBAudioOutput()
{
}

USBAudioOutput::~USBAudioOutput()
{
}

VOID USBAudioOutput::Start()
{
    if(!m_AudioPlaying)
    {
        BOOL success;
        DWORD BytesTransferred;
        DWORD count = 0;
        do
        {
            // render audio stream to buffer
            PBYTE pBufferStart = GetBuffer();
            PBYTE pBufferEnd = pBufferStart + GetBufferSize();
            PBYTE pBufferLast = pBufferStart;
            TRANSFER_STATUS TransferStatus = {0};
            pBufferLast = m_DeviceContext->TransferBuffer(pBufferStart, pBufferEnd, &TransferStatus);
            BytesTransferred = pBufferLast - pBufferStart;

            if (BytesTransferred == 0)
            {
                Stop();
                break;
            }

            // transfer stream via USB
            success = TransferBuffer(BytesTransferred);
            count++;
        }
        while (success && count < 2);

        USBAudioPipe::Start();
    }
}

BOOL
USBAudioOutput::TransferBuffer(DWORD NumBytes)
{
    BOOL bRc = FALSE;

    if (m_hUSBPipe != NULL && NumBytes != 0)
    {
        DWORD FrameSize = m_CurrSampleRate / 1000 * m_BytesPerSample * m_NumChannels;
        DWORD NumFrames = (NumBytes + FrameSize - 1)  / FrameSize;  // round up
        DWORD dwFrames = USBAUD_FRAMES < NumFrames ? USBAUD_FRAMES : NumFrames;
        LPVOID lpvBuffer = m_Buffer[m_CurrBufferIndex];

        for (DWORD i = 0; i < dwFrames; i++)
        {
            m_Lengths[m_CurrBufferIndex][i] = FrameSize;
        }
        m_Lengths[m_CurrBufferIndex][dwFrames-1] = NumBytes - (dwFrames-1) * FrameSize; // last frame can be shorter

        DEBUGMSG( 1, (TEXT("TransferBuffer (Output): NumBytes = %i, dwFrames = %i, FrameSize = %i, Last frame = %i\n"),
            NumBytes, NumFrames, FrameSize, NumBytes - (dwFrames-1) * FrameSize));

        LPDWORD lpdwLengths = m_Lengths[m_CurrBufferIndex];

        bRc = m_hUSBPipe->IsochTransfer(USB_START_ISOCH_ASAP | USB_NO_WAIT,         // transfer flags
                                                   0,                               // starting frame
                                                   dwFrames,                        // number of frames
                                                   lpdwLengths,                     // lengths
                                                   lpvBuffer,                       // data buffer
                                                   NULL,                            // physical address
                                                   0);
        if (!bRc)
        {
            DEBUGMSG( 1, (TEXT("IsochTransfer failed!\n")));
        }
        else
        {
            m_CurrBufferIndex ^= 1;     // flip buffer
        }
    }

    return bRc;
}
