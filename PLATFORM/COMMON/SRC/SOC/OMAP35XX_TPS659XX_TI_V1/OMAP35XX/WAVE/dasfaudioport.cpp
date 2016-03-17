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
#include "dasfaudioport.h"

// DASF Includes
//
#include <mmddk.h>
#include <strmctxt.h>


#define DEFAULT_DASF_BUFFER_SIZE            (0x1000)
#define DASFSTREAM_DELETE_THREAD_PRIORITY   111

//------------------------------------------------------------------------------
// typedefs

// function definition for initializing the codec adapter
typedef HRESULT (*fnInitializeCodecEx)(
        const HWCODEC_PARAM_t* pCodecParams,
        HWCODEC_AppInfo_t *pwfx,
        STREAM_TYPE_e Type,
        HWCODEC_AppRegisterCb_t* pCb,
        HANDLE hCodecInstance,
        DWORD *pStreamID
        );

// function definition for starting DSP process
typedef HRESULT (*fnStartProcessEx)(
    HANDLE hInstance
    );

// function definition for stopping DSP process
typedef HRESULT (*fnStopProcessEx)(
    HANDLE hInstance
    );

// function definition for Sending playback data
typedef HRESULT (*fnSendDataEx)(
    PBYTE pInputBuffer,
    DWORD InputBufferSize,
    HANDLE hInstance
    );

// function definition for receiving recorded data
typedef HRESULT (*fnReceiveDataEx)(
    PBYTE pOutputBuffer,
    DWORD RequestedSize,
    HANDLE hInstance
    );

// function definition for sending SET GAIN command
typedef BOOL (*fnSetGainEx)(
    MM_COMMANDDATATYPE_t *pDataSet,
    HANDLE hInstance
    );

// function definition for sending control commands
typedef BOOL (*fnSendControlCommandEx)(
    MM_COMMANDDATATYPE_t *pDataSet,
    HANDLE hInstance
    );

// function definition for deinitializing the codec adapter
typedef HRESULT (*fnDeInitializeCodecEx)(
    HANDLE hInstance
    );

typedef struct {
    // function definition for initializing the codec adapter
    HRESULT (*InitializeCodecEx)(
            const HWCODEC_PARAM_t* pCodecParams,
            HWCODEC_AppInfo_t *pwfx,
            STREAM_TYPE_e Type,
            HWCODEC_AppRegisterCb_t* pCb,
            HANDLE hCodecInstance,
            DWORD *pStreamID
            );

    // function definition for starting DSP process
    HRESULT (*StartProcessEx)(
        HANDLE hInstance
        );

    // function definition for stopping DSP process
    HRESULT (*StopProcessEx)(
        HANDLE hInstance
        );

    // function definition for Sending playback data
    HRESULT (*SendDataEx)(
        PBYTE pInputBuffer,
        DWORD InputBufferSize,
        HANDLE hInstance
        );

    // function definition for receiving recorded data
    HRESULT (*ReceiveDataEx)(
        PBYTE pOutputBuffer,
        DWORD RequestedSize,
        HANDLE hInstance
        );

    // function definition for sending SET GAIN command
    BOOL (*SetGainEx)(
        MM_COMMANDDATATYPE_t *pDataSet,
        HANDLE hInstance
        );

    // function definition for sending control commands
    BOOL (*SendControlCommandEx)(
        MM_COMMANDDATATYPE_t *pDataSet,
        HANDLE hInstance
        );

    // function definition for deinitializing the codec adapter
    HRESULT (*DeInitializeCodecEx)(
        HANDLE hInstance
        );
    } HWCodecAdapterHooks_t;

//------------------------------------------------------------------------------
int
Mutex(
    BOOL bLock
    )
{
    bLock ? EnterMutex() : ExitMutex();
    return TRUE;
}


//-----------------------------------------------------------------------------
// global variables

static HWCodecAdapterHooks_t _HwCodecAdapterHooks = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    };

//------------------------------------------------------------------------------
// DASF static variables
//
HMODULE DASF::s_hHwCodecAdapter = NULL;

//------------------------------------------------------------------------------
// DASFStream static variables
//
HANDLE DASFStream::s_hDeleteSignal = NULL;
HANDLE DASFStream::s_hDeleteThread = NULL;
DASFStream *DASFStream::s_pDeleteList = NULL;
CRITICAL_SECTION DASFStream::s_csDASFStream;

//------------------------------------------------------------------------------
BOOL DASF::Initialize(WCHAR const* szPort)
{
    BOOL rc = FALSE;

    // We will link the HWCA dynamic library during run time
    //
    s_hHwCodecAdapter = ::LoadLibrary(szPort);
    if (s_hHwCodecAdapter == NULL)
        {
        RETAILMSG(0, (L"WAV:DASF LoadLibrary failed with error %d\r\n",GetLastError()));
        goto cleanUp;
        }

    // Get exported functions
    _HwCodecAdapterHooks.InitializeCodecEx = (fnInitializeCodecEx) GetProcAddress(
                                          (HMODULE)s_hHwCodecAdapter,
                                          L"InitializeCodecEx"
                                          );

    _HwCodecAdapterHooks.StartProcessEx = (fnStartProcessEx) GetProcAddress(
                                          (HMODULE)s_hHwCodecAdapter,
                                          L"StartProcessEx"
                                          );

    _HwCodecAdapterHooks.StopProcessEx = (fnStopProcessEx) GetProcAddress(
                                          s_hHwCodecAdapter,
                                          L"StopProcessEx"
                                          );

    _HwCodecAdapterHooks.SendDataEx = (fnSendDataEx) GetProcAddress(
                                          s_hHwCodecAdapter,
                                          L"SendDataEx"
                                          );

    _HwCodecAdapterHooks.ReceiveDataEx = (fnReceiveDataEx) GetProcAddress(
                                          s_hHwCodecAdapter,
                                          L"ReceiveDataEx"
                                          );

    _HwCodecAdapterHooks.SendControlCommandEx = (fnSendControlCommandEx)::GetProcAddress(
                                          s_hHwCodecAdapter,
                                          L"SendControlCommandEx"
                                          );

    _HwCodecAdapterHooks.SetGainEx = (fnSetGainEx)::GetProcAddress(
                                          s_hHwCodecAdapter,
                                          L"SetGainEx"
                                          );

    _HwCodecAdapterHooks.DeInitializeCodecEx = (fnDeInitializeCodecEx)::GetProcAddress(
                                          s_hHwCodecAdapter,
                                          L"DeInitializeCodecEx"
                                          );
        rc = TRUE;

cleanUp:
    return rc;
}
//------------------------------------------------------------------------------
void
DASF::Uninitialize()
{
    if (s_hHwCodecAdapter != NULL)
        {
        ::FreeLibrary(s_hHwCodecAdapter);
        s_hHwCodecAdapter = NULL;
        }
}

//------------------------------------------------------------------------------
BOOL
DASF::RegisterDASFStream(DASFStream *pDASFStream)
{
    WAVEFORMATEX const*pWaveFormat = pDASFStream->GetWaveFormat();

    BOOL bRet = FALSE;
    HRESULT hResult = S_OK;
    HWCODEC_PARAM_t  codecParam;
    HWCODEC_AppInfo_t appInfo;
    HWCODEC_AppRegisterCb_t Cb;
    DWORD idDASFStream =0;
    HANDLE hDASFContext = NULL;

    codecParam.Channels = pWaveFormat->nChannels;
    codecParam.SamplesPerSec = pWaveFormat->nSamplesPerSec;
    codecParam.BitsPerSample = pWaveFormat->wBitsPerSample;

    appInfo.hAppInstance = pDASFStream;  //pointer to DASFStream
    appInfo.InAllocSize = DEFAULT_DASF_BUFFER_SIZE;
    appInfo.OutAllocSize = DEFAULT_DASF_BUFFER_SIZE;

    // New multimedia changes whiche sends their buffer pointer
    // wave driver must set this to 1(Adapter sends its buffers ) or
    // 0(HW adapter receives the buffers from wave driver )
    //
    // GetHWBuffer option has been removed for 35xx, now buffers are always allocated and sent to
    // wave driver by hw adapter.
    //appInfo.GetHWBuffer = 1;
    appInfo.InNoOfHWBuffers = MAX_DASFBUFFER_COUNT;
    appInfo.OutNoOfHWBuffers = 0;

    Cb.pfnOutputCallback = DASF::OnDASFCallback2;
    Cb.pfnInputCallback = DASF::OnDASFCallback;
    Cb.pfnErrorNotifyCallback = DASF::OnDASFError;

    DEBUGMSG(ZONE_MDD, (TEXT("Calling DSP==>InitializeCodecEx()\r\n")));

    hResult = _HwCodecAdapterHooks.InitializeCodecEx(
                           &codecParam,
                           &appInfo,
                           pDASFStream->IsOutputStream() ? PCMDEC : PCMENC,
                           &Cb,
                           &hDASFContext,
                           &idDASFStream
                           );
    if ( hResult != S_OK )
    {
        DEBUGMSG(ZONE_MDD, (TEXT("Failed to initialize hw codec adapter\r\n")));
        goto cleanUp;
    }
    // Note in 35xx DASF input hw buffers implies playback buffers and output hw buffers implies
    //. recorded buffers
    //
    pDASFStream->SetDASFInfo(hDASFContext, idDASFStream,
        pDASFStream->IsOutputStream() ? (PBYTE*)appInfo.InBufArray : (PBYTE*)appInfo.OutBufArray,
        pDASFStream->IsOutputStream() ? appInfo.InNoOfHWBuffers : appInfo.OutNoOfHWBuffers
        );

    bRet = TRUE;

cleanUp:
    return bRet;
}

//------------------------------------------------------------------------------

BOOL
DASF::UnregisterDASFStream(DASFStream *pStream)
{
    BOOL bRet = FALSE;
    HRESULT hResult = S_OK;

    hResult = _HwCodecAdapterHooks.DeInitializeCodecEx(pStream->GetDASFContext());
    if ( hResult != S_OK )
    {
        DEBUGMSG(ZONE_MDD, (TEXT("Failed to start hw codec adapter\r\n")));
        goto cleanUp;
    }

    bRet = TRUE;

cleanUp:
return bRet;
}

//------------------------------------------------------------------------------

BOOL
DASF::StartDASFStream(DASFStream *pStream)
{
    BOOL bRet = FALSE;
    HRESULT hResult = S_OK;

    hResult = _HwCodecAdapterHooks.StartProcessEx(pStream->GetDASFContext());
    if ( hResult != S_OK )
    {
        DEBUGMSG(ZONE_MDD, (TEXT("Failed to start hw codec adapter\r\n")));
        goto cleanUp;
    }

    bRet = TRUE;

cleanUp:
return bRet;
}

//------------------------------------------------------------------------------

BOOL
DASF::StopDASFStream(DASFStream *pStream)
{
    BOOL bRet = FALSE;
    HRESULT hResult = S_OK;

    hResult = _HwCodecAdapterHooks.StopProcessEx(pStream->GetDASFContext());
    if ( hResult != S_OK )
    {
        DEBUGMSG(ZONE_MDD, (TEXT("Failed to stop hw codec adapter\r\n")));
        goto cleanUp;
    }

    bRet = TRUE;

cleanUp:
return bRet;
}

//------------------------------------------------------------------------------

BOOL
DASF::InsertTransmitBuffer(PBYTE pBuffer, UINT nSize, DASFStream *pStream)
{
    BOOL bRet = FALSE;
    HRESULT hResult = S_OK;

    hResult = _HwCodecAdapterHooks.SendDataEx(pBuffer, nSize, pStream->GetDASFContext());
    if ( hResult != S_OK )
    {
        DEBUGMSG(ZONE_MDD, (TEXT("Failed to transmit buffer to hw codec adapter\r\n")));
        goto cleanUp;
    }

    bRet = TRUE;

cleanUp:
return bRet;
}

//------------------------------------------------------------------------------

BOOL
DASF::InsertCaptureBuffer(PBYTE pBuffer, UINT nSize, DASFStream *pStream)
{
    BOOL bRet = FALSE;
    HRESULT hResult = S_OK;

    hResult = _HwCodecAdapterHooks.ReceiveDataEx(pBuffer, nSize, pStream->GetDASFContext());
    if ( hResult != S_OK )
    {
        DEBUGMSG(ZONE_MDD, (TEXT("Failed to receive buffer from hw codec adapter\r\n")));
        goto cleanUp;
    }

    bRet = TRUE;

cleanUp:
return bRet;
}

//------------------------------------------------------------------------------

BOOL
DASF::SetGain(UINT nGain, DASFStream *pStream)
{
    BOOL bRet = FALSE;
    MM_COMMANDDATATYPE_t dataGainIn;

    // Client can give a unique value if needed
    //
    dataGainIn.hComponent = pStream;

    // To mute use MM_CommandStreamMute value
    //
    dataGainIn.AM_Cmd = MM_CommandSWGain;

    // (m_dwGain);Q15 gain value (0x0000 up to 0x8000 with
    // 0x8000 representing +1)
    //
    dataGainIn.param1 = ((nGain & 0xFFFF)/2);

    dataGainIn.param2 = 0;
    dataGainIn.streamID = pStream->GetDASFStreamId();
    dataGainIn.pMDNGainRamp = NULL;
    dataGainIn.pRTMGainRamp = NULL;

    if (!_HwCodecAdapterHooks.SetGainEx(&dataGainIn, pStream->GetDASFContext()))
    {
        DEBUGMSG(ZONE_MDD, (TEXT("Failed to set the gain in hw codec adapter\r\n")));
        goto cleanUp;
    }

    bRet = TRUE;

cleanUp:
    return bRet;
}

//------------------------------------------------------------------------------
void
DASF::OnDASFCallback(PBYTE pAppBuf, DWORD *pActualSize, HANDLE hContext)
{
    DASFStream  *pDASFStream = (DASFStream*)hContext;

    DEBUGMSG(ZONE_MDD, (TEXT("OnDASFCallback+\r\n")));

    pDASFStream->ProcessDASFCallback();

    DEBUGMSG(ZONE_MDD, (TEXT("OnDASFCallback-\r\n")));
}

//------------------------------------------------------------------------------
void
DASF::OnDASFCallback2(PBYTE pAppBuf, DWORD *pActualSize, HANDLE hContext)
{
    DEBUGMSG(ZONE_MDD, (TEXT("OnDASFCallback2+\r\n")));

    OnDASFCallback(pAppBuf, pActualSize, hContext);

    DEBUGMSG(ZONE_MDD, (TEXT("OnDASFCallback2-\r\n")));
}
//------------------------------------------------------------------------------
void
DASF::OnDASFError(BOOL Status, HANDLE hContext)
{
}

//------------------------------------------------------------------------------
BOOL
DASFStream::InitializeDASFStream()
{
    BOOL rc = FALSE;

    // initialize critical sections
    InitializeCriticalSection(&s_csDASFStream);

    // create event object to signal delete thread
    s_hDeleteSignal = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (s_hDeleteSignal == NULL)
        {
        RETAILMSG(ZONE_ERROR, (L"DASFStream::InitializeDASFStream - "
            L"Failed to instantiate event object\r\n")
            );
        goto cleanUp;
        }

    // spawn thread
    s_hDeleteThread = CreateThread(NULL, 0, ISTDASFStreamDelete, 0, 0, NULL);
    if (s_hDeleteThread == NULL)
        {
        RETAILMSG(ZONE_ERROR, (L"DASFStream::InitializeDASFStream - "
            L"Failed to spawn delete thread\r\n")
            );
        goto cleanUp;
        }

    rc = TRUE;
cleanUp:
    if (rc == FALSE)
        {
        CloseHandle(s_hDeleteSignal);
        s_hDeleteSignal = NULL;
        }
    return rc;
}

//------------------------------------------------------------------------------
DWORD
DASFStream::ISTDASFStreamDelete(void *pv)
{
    DASFStream *pDASFStream;

    // set this thread to be a very low priority
    CeSetThreadPriority(GetCurrentThread(), DASFSTREAM_DELETE_THREAD_PRIORITY);

    // wait for event to get signaled
    while (TRUE)
        {
        WaitForSingleObject(s_hDeleteSignal, INFINITE);

        // synchronously delete DASFStream objects in the delete list
        EnterMutex();
        while (s_pDeleteList != NULL)
            {
            RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:ISTDASFStreamDelete()[pThis=0x%08X, dir=%d]\r\n",
            s_pDeleteList, s_pDeleteList->IsOutputStream()));

            pDASFStream = s_pDeleteList;
            pDASFStream->Uninitialize();

            s_pDeleteList = pDASFStream->m_pNextDelete;

            delete pDASFStream;
            }
        ExitMutex();

        }
}

//------------------------------------------------------------------------------
DASFStream*
DASFStream::CreateDASFStream()
{
    DASFStream *pDASFStream = NULL;

    // if a DASFStream wasn't found from the delete list then create a new one
    if (pDASFStream == NULL)
        {
        pDASFStream = new DASFStream();
        }

    return pDASFStream;
}

//------------------------------------------------------------------------------
void
DASFStream::DeleteDASFStream(DASFStream *pInsertDASFStream)
{
    DASFStream *pDASFStream;

    // Uninitialize DASF Stream first
    pInsertDASFStream->m_pNextDelete = NULL;

    // insert into delete list
    EnterMutex();
    RETAILMSG(0, (L"WAV:DeleteDASFStream()[pThis=0x%08X, dir=%d]\r\n",
            pInsertDASFStream, pInsertDASFStream->IsOutputStream()));
    if (s_pDeleteList != NULL)
        {
        // get to the end of the list
        pDASFStream = s_pDeleteList;
        while (pDASFStream->m_pNextDelete != NULL)
            {
            pDASFStream = pDASFStream->m_pNextDelete;
            }
        // insert into tail of list
        pDASFStream->m_pNextDelete = pInsertDASFStream;
        }
    else
        {
        // insert into head if list is empty
        s_pDeleteList = pInsertDASFStream;
        }
    //LeaveCriticalSection(&s_csDASFStream);
    ExitMutex();

    // signal there's objects to be deleted
    SetEvent(s_hDeleteSignal);
}

//------------------------------------------------------------------------------
BOOL
DASFStream::Initialize(DASFAudioStreamPort *pDASFAudioStreamPort,
    HANDLE hStreamContext, BOOL bOutput, UINT nGain
    )
{
    // initialize member variables
    m_pNext = NULL;
    m_pNextDelete = NULL;
    m_bOutput = bOutput;
    m_state = kInactive;
    m_nNextDASFBuffer = 0;
    m_hStreamContext = NULL;
    m_ffSetProcPermissions = 0;
    m_nActiveDASFBufferCount = 0;
    m_hStreamContext = hStreamContext;
    m_pDASFAudioStreamPort = pDASFAudioStreamPort;
    memset(m_rgDASFBuffers, 0, sizeof(m_rgDASFBuffers));

    // create new instance of DASF stream id
    DASF::RegisterDASFStream(this);

    // Set gain for this stream
    DASF::SetGain(nGain, this);

    return TRUE;
}

//------------------------------------------------------------------------------
void
DASFStream::Uninitialize()
{
    RETAILMSG(0, (L"WAV:+Uninitialize()[pThis=0x%08X, dir=%d]\r\n",
         this, IsOutputStream()));

    // unregister DASF from DASF
    DASF::StopDASFStream(this);
    DASF::UnregisterDASFStream(this);
    m_pDASFAudioStreamPort->StreamStopped(this);

    // reset member variables
    m_pNext = NULL;
    m_pNextDelete = NULL;
    m_bOutput = TRUE;
    m_hStreamContext = 0;
    m_nNextDASFBuffer = 0;
    m_hStreamContext = NULL;
    m_state = kUninitialized;
    m_ffSetProcPermissions = 0;
    m_nActiveDASFBufferCount = 0;
    m_pDASFAudioStreamPort = NULL;
    memset(m_rgDASFBuffers, 0, sizeof(m_rgDASFBuffers));

    RETAILMSG(0, (L"WAV:-Uninitialize()[pThis=0x%08X, dir=%d]\r\n",
         this, IsOutputStream()));

}

//------------------------------------------------------------------------------
PBYTE
DASFStream::GetNextBuffer()
{
    PBYTE pBuffer = NULL;
    if (m_state == kActive && IsDASFBufferSpaceAvailable())
        {
        pBuffer = m_rgDASFBuffers[m_nNextDASFBuffer];
        }

    return pBuffer;
}

//------------------------------------------------------------------------------
WAVEFORMATEX const*
DASFStream::GetWaveFormat()
{
    if (m_hStreamContext)
        {
        return ((StreamContext*)(m_hStreamContext))->GetWaveFormat();
        }
    return NULL;
}

//------------------------------------------------------------------------------
BOOL
DASFStream::InsertBuffer(PBYTE pBuffer)
{
    RETAILMSG(0, (L"WAV:+InsertBuffer(pBuffer=0x%08X)[pThis=0x%08X, dir=%d]\r\n",
        pBuffer, this, IsOutputStream()));

    if (pBuffer == NULL) return FALSE;

    // Need to check the state of the DASFStream
    if (GetDASFStreamState() != kActive) return FALSE;

    // push the buffer down to the DSP
    if (IsOutputStream())
        {
        RETAILMSG(0, (L"InsertBuffer: output stream\r\n"));
        DASF::InsertTransmitBuffer(pBuffer, DEFAULT_DASF_BUFFER_SIZE, this);
        }
    else
        {
        RETAILMSG(0, (L"InsertBuffer: input stream\r\n"));
        DASF::InsertCaptureBuffer(pBuffer, DEFAULT_DASF_BUFFER_SIZE, this);
        }

    // increment active buffer pointer and counters
    m_nNextDASFBuffer = (m_nNextDASFBuffer + 1) % MAX_DASFBUFFER_COUNT;
    IncrementActiveDASFBufferCount();

RETAILMSG(0, (L"WAV:-InsertBuffer()[pThis=0x%08X, dir=%d]\r\n",
        this, IsOutputStream()));
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
DASFStream::ProcessDASFCallback()
{
    RETAILMSG(0, (L"WAV:+ProcessDASFCallback()[pThis=0x%08X, dir=%d]\r\n",
        this, IsOutputStream()));

    PBYTE pBuffer;
    DecrementActiveDASFBufferCount();

    // check if this thread's permissions need to be set
    if (m_ffSetProcPermissions == 0)
        {
        CeSetThreadPriority(GetCurrentThread(), 110);
        //SetProcPermissions(-1);              //Deprecated for WM7
        m_ffSetProcPermissions = 1;
        }

    // notify of recorded data
    if (IsInputStream() == TRUE)
        {
        // process recorded data
        ProcessBuffer(GetCurrentActiveBuffer());
        }

    // clear buffer to remove audio artifacts
    memset(GetCurrentActiveBuffer(), 0, DEFAULT_DASF_BUFFER_SIZE);

    if (m_state == kActive)
        {
        // push next buffer down to DSP
        pBuffer = GetNextBuffer();
        if (IsOutputStream() == TRUE)
            {
            ProcessBuffer(pBuffer);
            }
        InsertBuffer(pBuffer);
        }

    RETAILMSG(0, (L"WAV:-ProcessDASFCallback()[pThis=0x%08X, dir=%d]\r\n",
        this, IsOutputStream()));
    return TRUE;
}

//------------------------------------------------------------------------------
int
DASFStream::ProcessBuffer(PBYTE pBuffer)
{
    RETAILMSG(0, (L"WAV:+ProcessBuffer(pBuffer=0x%08X)[pThis=0x%08X, dir=%d]\r\n",
        pBuffer, this, IsOutputStream()));

    int rc = 0;
    if (GetDASFStreamState() == kActive)
        {
        RETAILMSG(0, (L"WAV:GetDASFStreamState() == kActive\r\n"));

        // initialize audio data structure
        //
        ASPM_STREAM_DATA    StreamData;
        StreamData.pBuffer = pBuffer;
        StreamData.dwBufferSize = DEFAULT_DASF_BUFFER_SIZE;
        StreamData.hStreamContext = m_hStreamContext;

        // check if stream context is still active if not then we need to
        // shutdown this stream
        if (m_pDASFAudioStreamPort->SendAudioStreamMessage(
                    IsOutputStream()? ASPM_PROCESSDATA_TX : ASPM_PROCESSDATA_RX,
                    &StreamData
                    ) == FALSE)
            {
            // stream context has shutdown start the deactivation process
            // for this DASFStream
            StopStream();
            }
        else
            {
            rc = 1;
            }
        }

    RETAILMSG(0, (L"WAV:-ProcessBuffer()[pThis=0x%08X, dir=%d]\r\n",
        this, IsOutputStream()));
    return rc;
}

//------------------------------------------------------------------------------
BOOL
DASFStream::StartStream()
{
    RETAILMSG(0, (L"WAV:+StartStream()[pThis=0x%08X, dir=%d]\r\n",
        this, IsOutputStream()));

    PBYTE pBuffer;

    if (m_state == kActive)
        {
        goto cleanUp;
        }

    //EnterMutex();
    if (m_state == kInactive)
        {
        // signal to client we are about to start
        m_state = kActive;
        m_pDASFAudioStreamPort->StreamStarting(this);
        //ExitMutex();

        // calling into this routine can create a deadlock situation
        RETAILMSG(0, (L"StartStream: StartDASFStream\r\n"));
        DASF::StartDASFStream(this);

        //EnterMutex();

        // start stream by copying audio data into DASF buffer and
        // pushing it down to DASF
        pBuffer = GetNextBuffer();
        while (pBuffer != NULL)
            {
            if (IsOutputStream()== TRUE)
                {
                ProcessBuffer(pBuffer);
                }
            InsertBuffer(pBuffer);
            pBuffer = GetNextBuffer();
            }
        }
    m_state = kActive;
    //ExitMutex();

cleanUp:
RETAILMSG(0, (L"WAV:-StartStream()[pThis=0x%08X, dir=%d]\r\n",
        this, IsOutputStream()));
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
DASFStream::StopStream()
{
    RETAILMSG(0, (L"WAV:+StopStream()[pThis=0x%08X, dir=%d]\r\n",
            this, IsOutputStream()));

    EnterMutex();
    if (m_state != kInactive && m_state != kUninitialized)
        {
        m_state = kDeactivating;

        // check if it's safe to shutdown this now else wait for callback
        if (GetActiveDASFBufferCount() == 0)
            {
            // set state and remove from active list
            m_state = kInactive;
            m_hStreamContext = NULL;
            DeleteDASFStream(this);
            }
        }
    ExitMutex();

    RETAILMSG(0, (L"WAV:-StopStream()[pThis=0x%08X, dir=%d]\r\n",
        this, IsOutputStream()));
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
DASFStream::SetStreamGain(UINT nGain)
{
    RETAILMSG(0, (L"WAV:SetStreamGain() + \r\n"));

    if (m_state == kActive)
        {
        DASF::SetGain(nGain, this);
        }

    RETAILMSG(0, (L"WAV:SetStreamGain() - \r\n"));
    return TRUE;
}

//--------------------------------------------------------------------------
void
DASFStream::SetDASFInfo(HANDLE hDASFContext, UINT idDASFStream, PBYTE *rgBuffer, int nCount)
{
    m_hDASFContext = hDASFContext;
    m_idDASFStream = idDASFStream;
    nCount = min(nCount, MAX_DASFBUFFER_COUNT);
    for (int i=0; i<nCount;i++)
        {
        m_rgDASFBuffers[i] = rgBuffer[i];
        }
}

//------------------------------------------------------------------------------
void
DASFAudioStreamPort::InsertDASFStream(DASFStream *pStream)
{
    Lock();
    if (m_pStreamsHead == NULL)
        {
        m_pStreamsHead = pStream;
        }
    else
        {
        m_pStreamsTail->m_pNext = pStream;
        }

    m_pStreamsTail = pStream;
    pStream->m_pNext = NULL;
    Unlock();
}

//------------------------------------------------------------------------------
void
DASFAudioStreamPort::RemoveDASFStream(DASFStream *pStream)
{
    DASFStream *pPrev = NULL;
    DASFStream *pCurrent = m_pStreamsHead;

    Lock();
    while (pCurrent != NULL)
        {
        if (pCurrent == pStream)
            {
            if (pCurrent == m_pStreamsHead)
                {
                // removing from front
                m_pStreamsHead = m_pStreamsHead->m_pNext;
                }
            else if (pPrev != NULL)
                {
                // removing from middle or tail
                pPrev->m_pNext = pCurrent->m_pNext;
                }

            if (pCurrent == m_pStreamsTail)
                {
                // removing from tail
                m_pStreamsTail = pPrev;
                }

            break;
            }

        // next node
        pPrev = pCurrent;
        pCurrent = pCurrent->m_pNext;
        }
    Unlock();
}

//------------------------------------------------------------------------------
DASFStream*
DASFAudioStreamPort::FindDASFStream(HANDLE hStreamContext)
{
    Lock();
    DASFStream *pCurrent = m_pStreamsHead;
    while (pCurrent != NULL)
        {
        if (pCurrent->GetStreamContext() == hStreamContext) break;
        pCurrent = pCurrent->m_pNext;
        }
    Unlock();
    return pCurrent;
}

//------------------------------------------------------------------------------
BOOL
DASFAudioStreamPort::StreamStarting(DASFStream *pStream)
{
    Mutex(TRUE);
    Lock();
    if (pStream->IsOutputStream())
        {
        // only notify hw bridge on the activation of the first stream only
        ++m_nActiveTxStreams;
        if (m_nActiveTxStreams == 1)
            {
            SendAudioStreamMessage(ASPM_START_TX, NULL);
            }
        }
    else
        {
        // only notify hw bridge on the activation of the first stream only
        ++m_nActiveRxStreams;
        if (m_nActiveRxStreams == 1)
            {
            SendAudioStreamMessage(ASPM_START_RX, NULL);
            }
        }
    Unlock();
    Mutex(FALSE);

    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
DASFAudioStreamPort::StreamStopped(DASFStream *pStream)
{
    RETAILMSG(0, (L"WAV:StreamStopped() +\r\n"));

    // UNDONE:
    //  Need to check if pStream is in the active queu
    EnterMutex();
    Lock();
    if (pStream->IsOutputStream())
        {
        // only notify hw bridge on the activation of the first stream only
        --m_nActiveTxStreams;
        if (m_nActiveTxStreams == 0)
            {
            SendAudioStreamMessage(ASPM_STOP_TX, NULL);
            }
        }
    else
        {
        // only notify hw bridge on the activation of the first stream only
        --m_nActiveRxStreams;
        if (m_nActiveRxStreams == 0)
            {
            SendAudioStreamMessage(ASPM_STOP_RX, NULL);
            }
        }
    RemoveDASFStream(pStream);
    Unlock();
    ExitMutex();

    RETAILMSG(0, (L"WAV:StreamStopped() +\r\n"));

    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
DASFAudioStreamPort::signal_Port(
    DWORD SignalId,
    HANDLE hStreamContext,
    DWORD dwContextData
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+DMTAudioStreamPort::signal_Port(SignalId=%d)\r\n", SignalId)
        );

	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:+DMTAudioStreamPort::signal_Port(SignalId=%d)\r\n", SignalId)
        );

    BOOL bIsOutput;
    DASFStream *pStream;
    BOOL rc = TRUE;

    if (DASF::IsDASFInitialized() == FALSE)
        {
        rc = FALSE;
        goto cleanUp;
        }

    // route request to correct callback routine
    //
    switch (SignalId)
        {
        case ASPS_START_STREAM_TX:
        case ASPS_START_STREAM_RX:
            // check if we already have a stream w/ matching streamcontext
            bIsOutput = (SignalId == ASPS_START_STREAM_TX);

            RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:ASPS_START_STREAM(%d) hStrmCtxt=0x%X\r\n",
                bIsOutput, hStreamContext));
            if (m_nActiveRxStreams !=0)
                {
                bIsOutput = bIsOutput;
                }

            pStream = FindDASFStream(hStreamContext);
            if (pStream == NULL)
                {
                // create DASF Stream
                pStream = DASFStream::CreateDASFStream();
                if (pStream == FALSE)
                    {
                    rc = FALSE;
                    goto cleanUp;
                    }

                // initialize Stream
                StreamContext* pStreamContext = (StreamContext*)hStreamContext;
                pStream->Initialize(this, hStreamContext, bIsOutput, (UINT)pStreamContext->GetGain());


                // insert into queue
                InsertDASFStream(pStream);
                }

            // start DASF Stream
            pStream->StartStream();
            break;

        case ASPS_STOP_STREAM_TX:
        case ASPS_STOP_STREAM_RX:
            bIsOutput = (SignalId == ASPS_STOP_STREAM_TX);

            // find the DASF stream
            pStream = FindDASFStream(hStreamContext);
            RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:ASPS_STOP_STREAM(%d) hStrmCtxt=0x%X\r\n",
                bIsOutput, hStreamContext));
            if (pStream != NULL)
                {
                pStream->StopStream();
                }
            break;

        case ASPS_ABORT_STREAM_TX:
        case ASPS_ABORT_STREAM_RX:
            // find the DASF stream
            pStream = FindDASFStream(hStreamContext);
            if (pStream != NULL)
                {
                pStream->StopStream();
                }
            break;

        case ASPS_START_TX:
            SendAudioStreamMessage(ASPM_START_TX, NULL);
            break;

        case ASPS_START_RX:
            SendAudioStreamMessage(ASPM_START_RX, NULL);
            break;

        case ASPS_STOP_TX:
            if (m_nActiveTxStreams == 0)
                {
                SendAudioStreamMessage(ASPM_STOP_TX, NULL);
                }
            break;

        case ASPS_STOP_RX:
            if (m_nActiveRxStreams == 0)
                {
                SendAudioStreamMessage(ASPM_STOP_RX, NULL);
                }
            break;

        case ASPS_GAIN_STREAM_TX:
        case ASPS_GAIN_STREAM_RX:
            // find the DASF stream
            bIsOutput = (SignalId == ASPS_GAIN_STREAM_TX);

            StreamContext* pStreamContext = (StreamContext*)hStreamContext;
            pStream = FindDASFStream(hStreamContext);
            RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:ASPS_GAIN_STREAM(%d) hStrmCtxt=0x%X\r\n",
                bIsOutput, hStreamContext));
            if (pStream != NULL)
                {
                pStream->SetStreamGain((UINT)dwContextData);
                }
            break;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-DMTAudioStreamPort::signal_Port(SignalId=%d)\r\n", SignalId)
        );

	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:-DMTAudioStreamPort::signal_Port(SignalId=%d)\r\n", SignalId)
        );

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
// open_Port
//
BOOL
DASFAudioStreamPort::open_Port(WCHAR const* szPort, HANDLE hPlayPortConfigInfo, HANDLE hRecPortConfigInfo)
{
    return DASF::Initialize(szPort) && DASFStream::InitializeDASFStream();
}
//------------------------------------------------------------------------------


