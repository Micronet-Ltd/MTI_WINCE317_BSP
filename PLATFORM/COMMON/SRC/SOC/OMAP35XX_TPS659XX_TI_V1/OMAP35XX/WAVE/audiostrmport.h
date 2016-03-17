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

#ifndef _AUDIOSTRMPORT_H_
#define _AUDIOSTRMPORT_H_


//------------------------------------------------------------------------------
// Audio stream plug-in signals
//
#define ASPS_START_TX                   1
#define ASPS_STOP_TX                    (ASPS_START_TX + 1)
#define ASPS_START_RX                   (ASPS_STOP_TX + 1)
#define ASPS_STOP_RX                    (ASPS_START_RX + 1)
#define ASPS_ABORT_TX                   (ASPS_STOP_RX + 1)
#define ASPS_ABORT_RX                   (ASPS_ABORT_TX + 1)
#define ASPS_START_STREAM_TX            (ASPS_ABORT_RX + 1)
#define ASPS_START_STREAM_RX            (ASPS_START_STREAM_TX + 1)
#define ASPS_STOP_STREAM_TX             (ASPS_START_STREAM_RX + 1)
#define ASPS_STOP_STREAM_RX             (ASPS_STOP_STREAM_TX + 1)
#define ASPS_ABORT_STREAM_TX            (ASPS_STOP_STREAM_RX + 1)
#define ASPS_ABORT_STREAM_RX            (ASPS_ABORT_STREAM_TX + 1)
#define ASPS_GAIN_STREAM_TX             (ASPS_ABORT_STREAM_RX + 1)
#define ASPS_GAIN_STREAM_RX             (ASPS_GAIN_STREAM_TX + 1)
#define ASPS_PORT_RECONFIG              (ASPS_GAIN_STREAM_RX +1)


//------------------------------------------------------------------------------
// Audio stream plug-in messages
//
#define ASPM_START_TX                   WM_USER + 101
#define ASPM_STOP_TX                    (ASPM_START_TX + 1)
#define ASPM_START_RX                   (ASPM_STOP_TX + 1)
#define ASPM_STOP_RX                    (ASPM_START_RX + 1)
#define ASPM_PROCESSDATA_TX             (ASPM_STOP_RX + 1)
#define ASPM_PROCESSDATA_RX             (ASPM_PROCESSDATA_TX + 1)


//------------------------------------------------------------------------------
// Audio stream data structures
//
typedef struct
{
    LPBYTE      pBuffer;
    DWORD       dwBufferSize;
    HANDLE      hStreamContext;
}
ASPM_STREAM_DATA;


//------------------------------------------------------------------------------
// prototypes
//
class AudioStreamPort;

//------------------------------------------------------------------------------
// Audio stream port client interface definition
//
class AudioStreamPort_Client
{
    //--------------------------------------------------------------------------
    // public methods
    //
public:

    virtual BOOL OnAudioStreamMessage(AudioStreamPort *pPort,
                                      DWORD msg,
                                      void *pvData) = 0;
};


//------------------------------------------------------------------------------
// Audio stream port interface definition
//
class AudioStreamPort
{
    //--------------------------------------------------------------------------
    // public methods
    //
public:

    virtual BOOL signal_Port(DWORD SignalId,
                             HANDLE hStreamContext,
                             DWORD dwContextData) = 0;
    virtual BOOL open_Port(WCHAR const* szPort,
                           HANDLE hPlayPortConfigInfo,
                           HANDLE hRecPortConfigInfo) = 0;
    virtual BOOL register_PORTHost(AudioStreamPort_Client *pClient) = 0;
    virtual void unregister_PORTHost(AudioStreamPort_Client *pClient) = 0;
};

#endif  //_AUDIOSTRMPORT_H_
