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

#ifndef _DASFAUDIOPORT_H_
#define _DASFAUDIOPORT_H_

#include "audiostrmport.h"
#include "memtxapi.h"

// DASF Includes
//
#include "HwcodecAPI.h"

#define MAX_DASFBUFFER_COUNT        3

class DASFStream;
class DASFAudioStreamPort;

//------------------------------------------------------------------------------
// dasf routines
//
class DASF
{
protected:
    static HMODULE                          s_hHwCodecAdapter;

    //--------------------------------------------------------------------------
    // inline methods
    //
public:
    static BOOL IsDASFInitialized()         { return s_hHwCodecAdapter != NULL; }

public:

    //--------------------------------------------------------------------------
    // DASF specific methods
    //
    static BOOL Initialize(WCHAR const* szPort);
    static void Uninitialize();

    static void OnDASFCallback(PBYTE pAppBuf, DWORD *pActualSize, HANDLE hContext);

    static void OnDASFCallback2(PBYTE pAppBuf, DWORD *pActualSize, HANDLE hContext);

    static void OnDASFError(BOOL Status, HANDLE hContext);

    static BOOL RegisterDASFStream(DASFStream *pDASFStream);
    static BOOL UnregisterDASFStream(DASFStream *pStream);
    static BOOL StartDASFStream(DASFStream *pStream);
    static BOOL StopDASFStream(DASFStream *pStream);
    static BOOL InsertTransmitBuffer(PBYTE pBuffer, UINT nSize, DASFStream *pStream);
    static BOOL InsertCaptureBuffer(PBYTE pBuffer, UINT nSize, DASFStream *pStream);
    static BOOL SetGain(UINT nGain, DASFStream *pStream);
};

//------------------------------------------------------------------------------
// Direct memory transfer audio stream port interface definition
//
class DASFStream
{
public:
    typedef enum DASFStreamState {
        kUninitialized,
        kActive,
        kDeactivating,
        kInactive
        };

    DASFStream                             *m_pNext;
    DASFStream                             *m_pNextDelete;

    UINT                                    m_ffSetProcPermissions;
    BOOL                                    m_bDeleted;

    //--------------------------------------------------------------------------
    // static variables
    //
protected:
    static DASFStream                      *s_pDeleteList;
    static CRITICAL_SECTION                 s_csDASFStream;
    static HANDLE                           s_hDeleteSignal;
    static HANDLE                           s_hDeleteThread;

    //--------------------------------------------------------------------------
    // member variables
    //
protected:
    HANDLE                                  m_hStreamContext;
    UINT                                    m_idDASFStream;
    HANDLE                                  m_hDASFContext;
    BOOL                                    m_bOutput;
    DASFStreamState                         m_state;
    UINT                                    m_nNextDASFBuffer;
    LONG                                    m_nActiveDASFBufferCount;
    PBYTE                                   m_rgDASFBuffers[MAX_DASFBUFFER_COUNT];
    DASFAudioStreamPort                    *m_pDASFAudioStreamPort;


    //--------------------------------------------------------------------------
    // constructor and destructor
    //
protected:
    DASFStream():m_state(kUninitialized)    {};
    ~DASFStream()                           {};

    //--------------------------------------------------------------------------
    // helper routines
    //
protected:
    PBYTE GetNextBuffer();
    int ProcessBuffer(PBYTE pBuffer);
    BOOL InsertBuffer(PBYTE pBuffer);

    //--------------------------------------------------------------------------
    // static routines
    //
public:
    static DASFStream* CreateDASFStream();
    static BOOL InitializeDASFStream();
    static void DeleteDASFStream(DASFStream *pStream);
    static DWORD ISTDASFStreamDelete(void *pv);

    //--------------------------------------------------------------------------
    // public inline routines
    //
public:

    HANDLE GetStreamContext() const         { return m_hStreamContext; }
    HANDLE GetDASFContext() const           { return m_hDASFContext; }
    UINT GetDASFStreamId() const            { return m_idDASFStream; }
    BOOL IsOutputStream() const             { return m_bOutput; }
    BOOL IsInputStream() const              { return m_bOutput == FALSE; }
    BOOL GetActiveDASFBufferCount() const   { return m_nActiveDASFBufferCount; }
    BOOL IsDASFBufferSpaceAvailable() const { return m_nActiveDASFBufferCount < 2; }

    void IncrementActiveDASFBufferCount()
    {
        InterlockedIncrement(&m_nActiveDASFBufferCount);
    }

    void DecrementActiveDASFBufferCount()
    {
    if (InterlockedDecrement(&m_nActiveDASFBufferCount) == 0) StopStream();
    }

    PBYTE GetCurrentActiveBuffer() const
    {
        return m_rgDASFBuffers[(m_nNextDASFBuffer + 1) % MAX_DASFBUFFER_COUNT];
    }


    //--------------------------------------------------------------------------
    // routines
    //
public:

    void Uninitialize();
    BOOL Initialize(DASFAudioStreamPort *pDASFAudioStreamPort, HANDLE hStreamContext, BOOL bOutput, UINT nGain);

    BOOL StartStream();
    BOOL StopStream();
    BOOL ProcessDASFCallback();
    WAVEFORMATEX const* GetWaveFormat();
    DASFStreamState GetDASFStreamState() const { return m_state;}
    void SetDASFInfo(HANDLE hDASFContext, UINT idDASFStream, PBYTE *rgBuffer, int nCount);
    BOOL SetStreamGain(UINT nGain);
};


//------------------------------------------------------------------------------
// Direct memory transfer audio stream port interface definition
//
class DASFAudioStreamPort : public AudioStreamPort
{
    //--------------------------------------------------------------------------
    // member variables
    //
protected:
    DASFStream                             *m_pStreamsHead;
    DASFStream                             *m_pStreamsTail;

    int                                     m_nActiveRxStreams;
    int                                     m_nActiveTxStreams;

    CRITICAL_SECTION            m_cs;

public:
    AudioStreamPort_Client                 *m_pPortClient;

    //--------------------------------------------------------------------------
    // constructor/destructor
    //
public:
    DASFAudioStreamPort() :
        m_pPortClient(NULL), m_pStreamsHead(NULL), m_pStreamsTail(NULL),
        m_nActiveRxStreams(0), m_nActiveTxStreams(0)
        {
        InitializeCriticalSection(&m_cs);
        }

    ~DASFAudioStreamPort()
      {
        DeleteCriticalSection(&m_cs);
      }


    //--------------------------------------------------------------------------
    // protected methods
    //
protected:
    void Lock()             {EnterMutex();} //{ EnterCriticalSection(&m_cs); }
    void Unlock()          {ExitMutex();}   // { LeaveCriticalSection(&m_cs); }

    void InsertDASFStream(DASFStream *pStream);
    DASFStream* FindDASFStream(HANDLE hStreamContext);
    void RemoveDASFStream(DASFStream *pStream);

    //--------------------------------------------------------------------------
    // inline public methods
    //
public:

    virtual BOOL register_PORTHost(AudioStreamPort_Client *pClient)
    {
        m_pPortClient = pClient;
        return TRUE;
    }

    virtual void unregister_PORTHost(AudioStreamPort_Client *pClient)
    {
        m_pPortClient = NULL;
    }

    BOOL SendAudioStreamMessage(DWORD msg, void *pvData)
    {
        return m_pPortClient->OnAudioStreamMessage(this, msg, pvData);
    }

    BOOL StreamStarting(DASFStream *pStream);
    BOOL StreamStopped(DASFStream *pStream);

    //--------------------------------------------------------------------------
    // public methods
    //
public:

    virtual BOOL signal_Port(DWORD SignalId, HANDLE hStreamContext, DWORD dwContextData);
    virtual BOOL open_Port(WCHAR const* szPort, HANDLE hPlayPortConfigInfo, HANDLE hRecPortConfigInfo);

};




#endif //_DASFAUDIOPORT_H_


