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
#include <omap35xx.h>
#include "wavemain.h"
#include "debug.h"
#include "dmtaudioport.h"

EXTERN_C int PopulateTxBuffer(void* pStart, void* pData, unsigned int dwLength);
EXTERN_C int PopulateRxBuffer(void* pStart, void* pData, unsigned int dwLength);
EXTERN_C int TxCommand(ExternalDrvrCommand cmd, void* pData, PortConfigInfo_t* pPortConfigInfo);
EXTERN_C int RxCommand(ExternalDrvrCommand cmd, void* pData, PortConfigInfo_t* pPortConfigInfo);
EXTERN_C int Mutex(BOOL bLock, DWORD dwTime, void* pData);

//------------------------------------------------------------------------------
//  Called by AudioStream port to populate buffer with out-going audio data
//
int
PopulateTxBuffer(
    void* pStart,
    void* pData,
    unsigned int dwLength
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV::+PopulateTxBuffer(pStart=0x%08X\r\n)",
        pStart));

	RETAILMSG(0 /*WAVE_AUDIO_DBG*/,(L"WAV:+%S\r\n",__FUNCTION__));

    // local variable init
    //
    ASSERT(pData);
    DMTAudioStreamPort *pThis = (DMTAudioStreamPort*)pData;

    // initialize audio data structure
    //
    ASPM_STREAM_DATA    StreamData;
    StreamData.pBuffer = (LPBYTE)pStart;
    StreamData.dwBufferSize = dwLength;
    StreamData.hStreamContext = NULL;

	RETAILMSG(0 /*WAVE_AUDIO_DBG*/,(L"WAV:%S, start = 0x%X, len = %d \r\n",__FUNCTION__, StreamData.pBuffer, StreamData.dwBufferSize));

    // notify port client of incoming audio data
    //
    int nRet = 0;
    nRet = pThis->m_pPortClient->OnAudioStreamMessage(pThis,
                                    ASPM_PROCESSDATA_TX, &StreamData);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV::-PopulateTxBuffer()\r\n"));

	RETAILMSG(0 /*WAVE_AUDIO_DBG*/,(L"WAV:-%S, ret = %d\r\n",__FUNCTION__, nRet));
    return nRet;
}


//------------------------------------------------------------------------------
//  Called by AudioStream port to process in-coming audio data
//
int
PopulateRxBuffer(
    void* pStart,
    void* pData,
    unsigned int dwLength
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV::+PopulateRxBuffer(pStart=0x%08X\r\n)",
        pStart));

    RETAILMSG(0,(L"WAV:+%S\r\n",__FUNCTION__));

    // local variable init
    //
    ASSERT(pData);
    DMTAudioStreamPort *pThis = (DMTAudioStreamPort*)pData;

    // initialize audio data structure
    //
    ASPM_STREAM_DATA    StreamData;
    StreamData.pBuffer = (LPBYTE)pStart;
    StreamData.dwBufferSize = dwLength;
    StreamData.hStreamContext = NULL;

    // notify port client of incoming audio data
    //
    int nRet = 0;
    nRet = pThis->m_pPortClient->OnAudioStreamMessage(pThis,
                                    ASPM_PROCESSDATA_RX, &StreamData);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV::-PopulateRxBuffer()\r\n"));

	RETAILMSG(0,(L"WAV:-%S\r\n",__FUNCTION__));

    return nRet;
}


//------------------------------------------------------------------------------
int
TxCommand(
    ExternalDrvrCommand cmd,
    void* pData,
    PortConfigInfo_t* pPortConfigInfo
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+TxCommand(cmd=%d)", cmd)
        );

	RETAILMSG(0 /*WAVE_AUDIO_DBG*/,(L"WAV:+%S, TxCommand, cmd = %d\r\n",__FUNCTION__, cmd));

    int nRet = 0;
    ASSERT(pData);
    DMTAudioStreamPort *pThis = (DMTAudioStreamPort*)pData;
    switch (cmd)
        {
        case kExternalDrvrDx_Stop:
            nRet = pThis->m_pPortClient->OnAudioStreamMessage(pThis,
                        ASPM_STOP_TX, NULL);
            break;

        case kExternalDrvrDx_Start:
            nRet = pThis->m_pPortClient->OnAudioStreamMessage(pThis,
                        ASPM_START_TX, NULL);
            break;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-TxCommand(cmd=%d)", cmd)
        );

	RETAILMSG(0 /*WAVE_AUDIO_DBG*/,(L"WAV:-%S\r\n",__FUNCTION__));
    return nRet;
}


//------------------------------------------------------------------------------
int
RxCommand(
    ExternalDrvrCommand cmd,
    void* pData,
    PortConfigInfo_t* pPortConfigInfo
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+RxCommand(cmd=%d)", cmd)
        );

	RETAILMSG(0 /*WAVE_AUDIO_DBG*/,(L"WAV:+%S, RxCommand, cmd = %d\r\n",__FUNCTION__, cmd));

    int nRet = 0;
    ASSERT(pData);
    DMTAudioStreamPort *pThis = (DMTAudioStreamPort*)pData;
    switch (cmd)
        {
        case kExternalDrvrDx_Stop:
            nRet = pThis->m_pPortClient->OnAudioStreamMessage(pThis,
                        ASPM_STOP_RX, NULL);
            break;

        case kExternalDrvrDx_Start:
            nRet = pThis->m_pPortClient->OnAudioStreamMessage(pThis,
                        ASPM_START_RX, NULL);
            break;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-RxCommand(cmd=%d)", cmd)
        );

	RETAILMSG(0 /*WAVE_AUDIO_DBG*/,(L"WAV:-%S\r\n",__FUNCTION__));
    return nRet;
}


//------------------------------------------------------------------------------
int
Mutex(
    BOOL bLock,
    DWORD dwTime,
    void* pData
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+Mutex(bLock=%d)", bLock)
        );

    bLock ? EnterMutex() : ExitMutex();

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-Mutex(bLock=%d)", bLock)
        );

    return TRUE;
}


//------------------------------------------------------------------------------
BOOL
DMTAudioStreamPort::signal_Port(
    DWORD SignalId,
    HANDLE hStreamContext,
    DWORD dwContextData
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+DMTAudioStreamPort::signal_Port(SignalId=%d)\r\n", SignalId)
        );

	RETAILMSG(0 /*WAVE_AUDIO_DBG*/,
        (L"WAV:+DMTAudioStreamPort(ver1)::signal_Port(SignalId=%d)\r\n", SignalId)
        );

    BOOL bSuccess = FALSE;
    if (m_hDMTPort == NULL)
        {
        RETAILMSG(ZONE_WARN,
            (L"WAV:ERROR! - attempting to send audio data to invalid port\r\n")
            );
		RETAILMSG(0 /*WAVE_AUDIO_DBG*/,
            (L"WAV:ERROR! - attempting to send audio data to invalid port\r\n")
            );

	    RETAILMSG(0 /*WAVE_AUDIO_DBG*/,
            (L"WAV:-DMTAudioStreamPort::signal_Port(SignalId=%d)\r\n", SignalId)
            );

		return bSuccess;
        //goto cleanup;
        }

    // route request to correct callback routine
    //
    switch (SignalId)
        {
        case ASPS_START_TX:
            m_fnTxCommand(kExternalDrvrDx_Start, m_pCallbackData, (PortConfigInfo_t*)&m_PlayPortConfigInfo);
            break;

        case ASPS_STOP_TX:
            m_fnTxCommand(kExternalDrvrDx_Stop, m_pCallbackData, NULL);
            break;

        case ASPS_START_RX:
            m_fnRxCommand(kExternalDrvrDx_Start, m_pCallbackData, (PortConfigInfo_t*)&m_RecPortConfigInfo);
            break;

        case ASPS_STOP_RX:
            m_fnRxCommand(kExternalDrvrDx_Stop, m_pCallbackData, NULL);
            break;

        case ASPS_ABORT_TX:
            m_fnTxCommand(kExternalDrvrDx_ImmediateStop, m_pCallbackData, NULL);
            break;

        case ASPS_ABORT_RX:
            m_fnRxCommand(kExternalDrvrDx_ImmediateStop, m_pCallbackData, NULL);
            break;

        case ASPS_PORT_RECONFIG:
            m_fnTxCommand(kExternalDrvrDx_Reconfig, m_pCallbackData, NULL);
            break;

		default:
  	           RETAILMSG(0 /*WAVE_AUDIO_DBG*/,(L"WAV:DMTAudioStreamPort::signal_Port(SignalId=%d - unsupported!)\r\n", SignalId));
			break;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-DMTAudioStreamPort::signal_Port(SignalId=%d)\r\n", SignalId)
        );

	RETAILMSG(0 /*WAVE_AUDIO_DBG*/,
        (L"WAV:-DMTAudioStreamPort::signal_Port(SignalId=%d)\r\n", SignalId)
        );

//cleanup:
    return bSuccess;
}


//------------------------------------------------------------------------------
BOOL
DMTAudioStreamPort::open_Port(
    WCHAR const* szPort,
    HANDLE hPlayPortConfigInfo,
    HANDLE hRecPortConfigInfo
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+DMTAudioStreamPort::open_Port(szPort=%s)",
        szPort)
        );

    // check for valid stream port name and a stream port isn't already
    // opened
    //
    ASSERT(szPort);
    ASSERT(m_hDMTPort == NULL);

    // copy the port configuration info to be sent to the mcbsp port
    //
    if (hPlayPortConfigInfo)
        {
        memcpy(&m_PlayPortConfigInfo,
            hPlayPortConfigInfo, sizeof(PortConfigInfo_t)
            );
        }
    if (hRecPortConfigInfo)
        {
        memcpy(&m_RecPortConfigInfo,
            hRecPortConfigInfo, sizeof(PortConfigInfo_t)
            );
        }

    // attempt to open the stream port and set member variable
    //
    BOOL bSuccess = TRUE;
    m_hDMTPort = CreateFile(szPort, 0, 0, NULL, 0, 0, NULL);
    if (m_hDMTPort == INVALID_HANDLE_VALUE)
        {
        RETAILMSG(ZONE_ERROR,
            (L"DMTAudioStreamPort::open_Port - failed err=0x%08X",
            ::GetLastError())
            );

        m_hDMTPort = NULL;
        goto cleanup;
        };

    // register call back routines
    //
    bSuccess = RegisterDirectMemoryTransferCallbacks();
    if (bSuccess == FALSE)
        {
        ::CloseHandle(m_hDMTPort);
        m_hDMTPort = NULL;
        }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-DMTAudioStreamPort::open_Port(szPort=%s)",
        szPort)
        );

cleanup:
    return bSuccess;
}


//------------------------------------------------------------------------------
BOOL
DMTAudioStreamPort::RegisterDirectMemoryTransferCallbacks()
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+DMTAudioStreamPort::RegisterDirectMemoryTransferCallbacks()")
        );

    EXTERNAL_DRVR_DATA_TRANSFER_IN wavDx;
    EXTERNAL_DRVR_DATA_TRANSFER_OUT wavDxOut;

    wavDx.pInData = this;
    wavDx.pfnInTxCommand = TxCommand;
    wavDx.pfnInRxCommand = RxCommand;
    wavDx.pfnInTxPopulateBuffer = PopulateTxBuffer;
    wavDx.pfnInRxPopulateBuffer = PopulateRxBuffer;
    wavDx.pfnMutexLock = Mutex;

    BOOL bSuccess;
    bSuccess = DeviceIoControl(m_hDMTPort,
        IOCTL_EXTERNAL_DRVR_REGISTER_TRANSFERCALLBACKS,
        (UCHAR*)&wavDx,
        sizeof(EXTERNAL_DRVR_DATA_TRANSFER_IN),
        (UCHAR*)&wavDxOut,
        sizeof(EXTERNAL_DRVR_DATA_TRANSFER_OUT),
        NULL,
        NULL
        );

    // if registeration is successfull save callback routines and data
    //
    if (bSuccess == TRUE)
        {
        m_pCallbackData = wavDxOut.pOutData;
        m_fnRxCommand = wavDxOut.pfnOutRxCommand;
        m_fnTxCommand = wavDxOut.pfnOutTxCommand;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-DMTAudioStreamPort::RegisterDirectMemoryTransferCallbacks()")
        );

    return bSuccess;
}


//------------------------------------------------------------------------------
BOOL
DMTAudioStreamPort::UnregisterDirectMemoryTransferCallbacks()
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+DMTAudioStreamPort::UnregisterDirectMemoryTransferCallbacks()")
        );

    BOOL bSuccess = TRUE;
    if (m_pCallbackData)
        {
        bSuccess = DeviceIoControl(m_hDMTPort,
            IOCTL_EXTERNAL_DRVR_UNREGISTER_TRANSFERCALLBACKS,
            NULL,
            0,
            NULL,
            0,
            NULL,
            NULL
            );

        ::CloseHandle(m_hDMTPort);

        m_pCallbackData = NULL;
        m_fnRxCommand = NULL;
        m_fnTxCommand = NULL;
        m_hDMTPort = NULL;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-DMTAudioStreamPort::UnregisterDirectMemoryTransferCallbacks()")
        );

    return bSuccess;
}

