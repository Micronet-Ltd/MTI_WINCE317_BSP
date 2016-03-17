// All rights reserved ADENEO EMBEDDED 2010
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
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
// 
// Module Name:  
//     chw.h
// 
// Abstract: Provides interface to UHCI host controller
// 
// Notes: 
//

#ifndef __CHW_H__
#define __CHW_H__
#include <usb200.h>
#include <sync.hpp>
#include <hcd.hpp>
#include "cpipe.h"
#include "ctd.h"
#include <omap3530_musbotg.h>
#include <omap3530_musbhcd.h>

#ifdef DUMP_MEMORY
#define memdump(x, y, z)  memdodump(x, y, z)
#else
#define memdump(x, y, z)
#endif

class CHW;
class CMhcd;

typedef struct _LOCK_DEVICE {
    UCHAR               ucLockEP;
    CRITICAL_SECTION    hLockCS;
} LOCK_DEVICE;

typedef struct _PIPE_LIST_ELEMENT {
    CPipe*                      pPipe;
    struct _PIPE_LIST_ELEMENT * pNext;
} PIPE_LIST_ELEMENT, *PPIPE_LIST_ELEMENT;

typedef struct _PIPE_LIST_INFO {
    UCHAR                       deviceAddr;
    struct _PIPE_LIST_INFO      *pNext;
} PIPE_LIST_INFO, *PPIPE_LIST_INFO;

typedef struct _EVENT_INFO {
    BOOL        IsDMA;
    CPipe *     pPipe;
    UCHAR       ucReservedDeviceAddr;
    UCHAR       ucDeviceEndPoint;
} EVENT_INFO, *PEVENT_INFO;

typedef struct _EP_INFO {
    USHORT      usPrevDevInfo;
    USHORT      usDeviceInfo;
} EP_INFO, *PEP_INFO;
//-----------------------------------Dummy Queue Head for static QHEad ---------------
class CDummyPipe : public CPipe
{

public:
    // ****************************************************
    // Public Functions for CQueuedPipe
    // ****************************************************
    CDummyPipe(IN CPhysMem * const pCPhysMem);
    virtual ~CDummyPipe() {;};

//    inline const int GetTdSize( void ) const { return sizeof(TD); };

    HCD_REQUEST_STATUS  IssueTransfer( 
                                IN const UCHAR /*address*/,
                                IN LPTRANSFER_NOTIFY_ROUTINE const /*lpfnCallback*/,
                                IN LPVOID const /*lpvCallbackParameter*/,
                                IN const DWORD /*dwFlags*/,
                                IN LPCVOID const /*lpvControlHeader*/,
                                IN const DWORD /*dwStartingFrame*/,
                                IN const DWORD /*dwFrames*/,
                                IN LPCDWORD const /*aLengths*/,
                                IN const DWORD /*dwBufferSize*/,     
                                IN_OUT LPVOID const /*lpvBuffer*/,
                                IN const ULONG /*paBuffer*/,
                                IN LPCVOID const /*lpvCancelId*/,
                                OUT LPDWORD const /*adwIsochErrors*/,
                                OUT LPDWORD const /*adwIsochLengths*/,
                                OUT LPBOOL const /*lpfComplete*/,
                                OUT LPDWORD const /*lpdwBytesTransferred*/,
                                OUT LPDWORD const /*lpdwError*/ )  
        { return requestFailed;};

    virtual HCD_REQUEST_STATUS  OpenPipe( void )
        { return requestFailed;};

    virtual HCD_REQUEST_STATUS  ClosePipe( void ) 
        { return requestFailed;};

    virtual HCD_REQUEST_STATUS IsPipeHalted( OUT LPBOOL const /*lpbHalted*/ )
        {   
            ASSERT(FALSE);
            return requestFailed;
        };

    virtual void ClearHaltedFlag( void ) {;};    
    
    HCD_REQUEST_STATUS AbortTransfer( 
                                IN const LPTRANSFER_NOTIFY_ROUTINE /*lpCancelAddress*/,
                                IN const LPVOID /*lpvNotifyParameter*/,
                                IN LPCVOID /*lpvCancelId*/ )
        {return requestFailed;};

    // ****************************************************
    // Public Variables for CQueuedPipe
    // ****************************************************
    virtual CPhysMem * GetCPhysMem() {return m_pCPhysMem;};

private:
    // ****************************************************
    // Private Functions for CQueuedPipe
    // ****************************************************
    void  AbortQueue( void ) { ; };
    HCD_REQUEST_STATUS  ScheduleTransfer( void ) { return requestFailed;};

    // ****************************************************
    // Private Variables for CQueuedPipe
    // ****************************************************
    IN CPhysMem * const m_pCPhysMem;
protected:
    // ****************************************************
    // Protected Functions for CQueuedPipe
    // ****************************************************
#ifdef DEBUG
    const TCHAR*  GetPipeType( void ) const
    {
        static const TCHAR* cszPipeType = TEXT("Dummy");
        return cszPipeType;
    }
#endif // DEBUG

    virtual BOOL    AreTransferParametersValid( const STransfer * /*pTransfer = NULL*/ )  const { return FALSE;};

    BOOL    CheckForDoneTransfers( void) { return FALSE; };
    BOOL    ProcessEP(DWORD dwStatus, BOOL isDMA)
    {
        UNREFERENCED_PARAMETER(isDMA);
        UNREFERENCED_PARAMETER(dwStatus); 
        return FALSE; 
    };

};

#if 0
#define INDEX(p) ((p == 0)? 0:(USB_ENDPOINT_DIRECTION_OUT(p)? (USB_ENDPOINT(p)*2): ((USB_ENDPOINT(p)*2)-1)))
#define R_RX_INDEX(p) ((p == 0)? 0: (p+1)/2)
#define R_TX_INDEX(p) ((p == 0)? 0: p/2)
#else
#define INDEX(p) USB_ENDPOINT(p)
#define R_RX_INDEX(p) p
#define R_TX_INDEX(p) p
#endif

#define MAX_DIR 2
#define DIR_IN 0
#define DIR_OUT 1
class CBusyPipeList : public LockObject {
public:
    CBusyPipeList(DWORD dwFrameSize) { m_FrameListSize=dwFrameSize;};
    ~CBusyPipeList() {DeInit();};
    BOOL Init();
    void DeInit();
    BOOL AddToBusyPipeList( IN CPipe * const pPipe, IN const BOOL fHighPriority );
    void RemoveFromBusyPipeList( IN CPipe * const pPipe );
    BOOL SignalDisconnectComplete(PVOID pContext);
    void SignalCheckForDoneTransfers( IN const UCHAR endpoint, IN const UCHAR ucIsOut);
    BOOL SignalCheckForDoneDMA( IN const UCHAR channel);
    HANDLE           m_hEP2Handles[HOST_MAX_EPNUM][MAX_DIR];
    HANDLE           m_hDMA2Handles[DMA_MAX_CHANNEL];
    void    SetSignalDisconnectACK(BOOL fAck) { m_SignalDisconnectACK = fAck;};

private:
    // ****************************************************
    // Private Functions for CPipe
    // ****************************************************
    static ULONG CALLBACK CheckForDoneTransfersThreadStub( IN PVOID pContext);
    ULONG CheckForDoneTransfersThread();
private:    
    DWORD   m_FrameListSize ;
    // ****************************************************
    // Private Variables for CPipe
    // ****************************************************
    // CheckForDoneTransfersThread related variables
    BOOL             m_fCheckTransferThreadClosing; // signals CheckForDoneTransfersThread to exit      
    HANDLE           m_hEP0CheckForDoneTransfersEvent; // event for CheckForDoneTransfersThread    
    PPIPE_LIST_INFO  m_PipeListInfoEP0CheckForDoneTransfersCount;
    HANDLE           m_hUpdateEPEvent;
    HANDLE           m_hCheckForDoneTransfersThread; // thread for handling done transfers        
    PPIPE_LIST_ELEMENT m_pBusyPipeList;    
    BOOL             m_SignalDisconnectACK;
#ifdef DEBUG
    int              m_debug_numItemsOnBusyPipeList;
#endif // DEBUG    
};

// this class is an encapsulation of UHCI hardware registers.
class CHW : public CHcd {
public:   
    // ****************************************************
    // public Functions
    // ****************************************************

    // 
    // Hardware Init/Deinit routines
    //
    CHW(IN CPhysMem * const pCPhysMem,
                              //IN CUhcd * const pHcd,
                              IN LPVOID pvUhcdPddObject, 
                              IN DWORD dwSysIntr  );
    ~CHW(); 
    virtual BOOL    Initialize();
    virtual void    DeInitialize( void );
    virtual void    SignalCheckForDoneTransfers( IN const UCHAR endpoint, IN const UCHAR ucIsOut ) { 
            m_cBusyPipeList.SignalCheckForDoneTransfers(endpoint, ucIsOut);
        };

    virtual void    SignalCheckForDoneDMA( IN const UCHAR channel ) { 
            m_cBusyPipeList.SignalCheckForDoneDMA(channel);
        };
    
    UCHAR    AcquireDMAChannel(CPipe *pPipe);        
    BOOL     ReleaseDMAChannel(CPipe *pPipe, UCHAR channel);

    USHORT   Channel2DeviceInfo(UCHAR channel);
    UCHAR    DeviceInfo2Channel(CPipe *pPipe);

    UCHAR    AcquirePhysicalEndPoint(CPipe *pPipe);
    BOOL     ReleasePhysicalEndPoint(CPipe *pPipe, BOOL fForce, BOOL fClearAll);

    UCHAR    GetCurrentToggleBit(UCHAR mappedEP, UCHAR isIn);
    
    void   EnterOperationalState(void);

    BOOL GetFrameNumber( OUT LPDWORD lpdwFrameNumber );

    BOOL GetFrameLength( OUT LPUSHORT lpuFrameLength );
    
    BOOL SetFrameLength( IN HANDLE hEvent,
                                IN USHORT uFrameLength );
    
    BOOL StopAdjustingFrame( void );


    void   ResetEndPoint(UCHAR endpoint);
    void   DumpRxCSR(UCHAR endpoint);

    //
    // Root Hub Queries
    //
    BOOL DidPortStatusChange( IN const UCHAR port );

    BOOL GetPortStatus( IN const UCHAR port,
                               OUT USB_HUB_AND_PORT_STATUS& rStatus );

    BOOL RootHubFeature( IN const UCHAR port,
                                IN const UCHAR setOrClearFeature,
                                IN const USHORT feature );

    BOOL ResetAndEnablePort( IN const UCHAR port );

    void DisablePort( IN const UCHAR port );

    virtual BOOL WaitForPortStatusChange (HANDLE m_hHubChanged);

    BOOL ConfigEP(IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor, UCHAR mappedEP,
        UCHAR bDeviceAddress, UCHAR bHubAddress, UCHAR bHubPort, int speed, UCHAR ucTransferMode, BOOL bClrTog);
    BOOL InitFIFO(IN CPipe * const pPipe);
    BOOL UpdateDataToggle(IN CPipe * const pPipe, BOOL fReset, BOOL fUpdate);
    BOOL ProcessTD(IN const UCHAR endpoint, void *pTmp);
    BOOL SendOutDMA(IN const UCHAR endpoint, void *pQTD);
    BOOL WriteFIFO(void *pContext, IN const UCHAR endpoint, void *pData, DWORD size);
    DWORD ReadFIFO(void *pContext, IN const UCHAR endpoint, void *pData, DWORD size, int *pRet);    
    BOOL WriteDMA(void * pContext, UCHAR endpoint, UCHAR channel, void *ppData, 
        DWORD size, DWORD dwMaxPacket, void *pQTD);
    DWORD ReadDMA(void *pContext, IN const UCHAR endpoint, IN const UCHAR channel, 
        void *pBuff, DWORD size, DWORD dwMaxPacket, CQTD *pCur);
    BOOL RestoreRxConfig(void *pContext, IN const UCHAR endpoint);
    DWORD CheckDMAResult(void *pContext, IN const UCHAR channel);
    DWORD GetRxCount(void *pContext, IN const UCHAR endpoint);
    BOOL ClearDMAChannel(UCHAR channel);
    BOOL ProcessDMAChannel(void *pContext, UCHAR endpoint, UCHAR channel, 
        BOOL IsTx, void *ppData, DWORD size, DWORD dwMaxPacket);
    void SetDeviceAddress(UCHAR endpoint, UCHAR ucDeviceAddress, UCHAR ucHubAddress, UCHAR ucHubPort, UCHAR out);
    DWORD CheckRxCSR(UCHAR MappedEP);
    DWORD CheckTxCSR(UCHAR MappedEP);
    BOOL IsHostConnect(void);
    BOOL IsDMASupport(void);
    DWORD GetDMAMode(void);
    BOOL LockEP0(UCHAR address);
    void UnlockEP0(UCHAR address);
    BOOL IsDeviceLockEP0(UCHAR address);
    void PrintRxTxCSR(UCHAR MappedEP);

    //
    // Miscellaneous bits
    //
    // PowerCallback
    VOID PowerMgmtCallback( IN BOOL fOff );

private:
    // ****************************************************
    // private Functions
    // ****************************************************
    
    static DWORD CALLBACK CeResumeThreadStub( IN PVOID context );
    DWORD CeResumeThread();
    //static DWORD CALLBACK UsbInterruptThreadStub( IN PVOID context );
    //DWORD UsbInterruptThread();

    //static DWORD CALLBACK UsbAdjustFrameLengthThreadStub( IN PVOID context );
    //DWORD UsbAdjustFrameLengthThread();

    VOID    SuspendHostController();
    VOID    ResumeHostController();

    //
    // ****************************************************
    // Private Variables
    // ****************************************************
    
    REGISTER    m_capBase;
    DWORD       m_NumOfPort;

    CBusyPipeList m_cBusyPipeList;
    // internal frame counter variables
    CRITICAL_SECTION m_csFrameCounter;
        
    HANDLE   m_hUsbHubConnectEvent;
    HANDLE   m_hUsbHubDisconnectEvent;

    // frame length adjustment variables
    // note - use LONG because we need to use InterlockedTestExchange
    HANDLE   m_hAdjustDoneCallbackEvent;

    BOOL    m_bDoResume;
    USHORT  m_DMAChannel[MAX_DMA_CHANNEL];
    CRITICAL_SECTION m_csDMAChannel;
    HANDLE  m_hLockDMAAccess;
    EP_INFO m_EndPoint[HOST_MAX_EPNUM][MAX_DIR];
    DWORD   m_EndPointInUseCount;
    CRITICAL_SECTION m_csEndPoint;
    USHORT  m_fifo_avail_addr; // FIFO start 
    USHORT  m_intr_rx_avail;
    LOCK_DEVICE     m_LockEP0DeviceAddress;
public:
    BOOL    SignalHubChangeEvent(BOOL fConnect);
    void    SetSignalDisconnectACK(BOOL fAck) { m_cBusyPipeList.SetSignalDisconnectACK(fAck); };
private:
    // initialization parameters for the IST to support CE resume
    // (resume from fully unpowered controller).
    //CUhcd    *m_pHcd;
    CPhysMem *m_pMem;
    LPVOID    m_pPddContext;
    BOOL g_fPowerUpFlag ;
    BOOL g_fPowerResuming ;
public:
    BOOL GetPowerUpFlag() { return g_fPowerUpFlag; };
    BOOL SetPowerUpFlag(BOOL bFlag) { return (g_fPowerUpFlag=bFlag); };
    BOOL GetPowerResumingFlag() { return g_fPowerResuming ; };
    BOOL SetPowerResumingFlag(BOOL bFlag) { return (g_fPowerResuming=bFlag) ; };
    CPhysMem * GetPhysMem() { return m_pMem; };
    DWORD GetNumberOfPort() { return m_NumOfPort; };
    //Bridge To its Instance.
    BOOL AddToBusyPipeList( IN CPipe * const pPipe, IN const BOOL fHighPriority ) {  return m_cBusyPipeList.AddToBusyPipeList(pPipe,fHighPriority);};
    void RemoveFromBusyPipeList( IN CPipe * const pPipe ) { m_cBusyPipeList.RemoveFromBusyPipeList(pPipe); };           
    BOOL SignalDisconnectComplete(void) { return (m_cBusyPipeList.SignalDisconnectComplete(GetOTGContext()));};
    void SetRxDataAvail(UCHAR endpoint) { m_intr_rx_avail |= (1<<endpoint);};
    BOOL GetRxDataAvail(UCHAR endpoint) { return ((m_intr_rx_avail & (1<<endpoint))? TRUE: FALSE);};
    void ClrRxDataAvail(UCHAR endpoint) { (m_intr_rx_avail &=~(1<<endpoint));};    
};


#endif

