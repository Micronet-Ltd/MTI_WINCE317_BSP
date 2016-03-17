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
//     hcd.hpp
// 
// Abstract:  Chcd implements the abstract HCDI interface. It mostly
//            just passes requests on to other objects, which
//            do the real work.
//     
// Notes: 
//
#ifndef __HCD_HPP_
#define __HCD_HPP_
class CHcd;
#include <cdevice.hpp>
#include <cphysmem.hpp>
#include "cpipe.h"
#include "omap3530_musbotg.h"

typedef enum _HCD_STATE{
    USBHCD_IDLE = 0,
    USBHCD_ACTIVE
}HCD_STATE, *PHCD_STATE;

class CHcd : public  LockObject, public CDeviceGlobal
{
public:
    // ****************************************************
    // Public Functions for CUhcd
    // ****************************************************
    CHcd( );
    // These functions are called by the HCDI interface
    virtual ~CHcd();
    virtual BOOL DeviceInitialize(void)=0;
    virtual void DeviceDeInitialize( void )=0;   
    virtual BOOL    SignalHubChangeEvent(BOOL fConnect)=0;    
    
    HCD_STATE   m_fHCDState;
    HANDLE      m_hPowerDownEvent;
    BOOL        m_fPowerDown;
//    virtual BOOL GetFrameNumber( OUT LPDWORD lpdwFrameNumber )=0;
//    virtual BOOL GetFrameLength( OUT LPUSHORT lpuFrameLength )=0;
//    virtual BOOL SetFrameLength( IN HANDLE hEvent,IN USHORT uFrameLength )=0;
//    virtual BOOL StopAdjustingFrame( void )=0;
    BOOL OpenPipe( IN UINT address,
                   IN LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                   OUT LPUINT lpPipeIndex );
    
    BOOL ClosePipe( IN UINT address,
                    IN UINT pipeIndex );
    
    BOOL IssueTransfer( IN UINT address,
                        IN UINT pipeIndex,
                        IN LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                        IN LPVOID lpvNotifyParameter,
                        IN DWORD dwFlags,
                        IN LPCVOID lpvControlHeader,
                        IN DWORD dwStartingFrame,
                        IN DWORD dwFrames,
                        IN LPCDWORD aLengths,
                        IN DWORD dwBufferSize,     
                        IN_OUT LPVOID lpvBuffer,
                        IN ULONG paBuffer,
                        IN LPCVOID lpvCancelId,
                        OUT LPDWORD adwIsochErrors,
                        OUT LPDWORD adwIsochLengths,
                        OUT LPBOOL lpfComplete,
                        OUT LPDWORD lpdwBytesTransfered,
                        OUT LPDWORD lpdwError );
    
    BOOL AbortTransfer( IN UINT address,
                        IN UINT pipeIndex,
                        IN LPTRANSFER_NOTIFY_ROUTINE lpCancelAddress,
                        IN LPVOID lpvNotifyParameter,
                        IN LPCVOID lpvCancelId );

    BOOL IsPipeHalted( IN UINT address,
                       IN UINT pipeIndex,
                       OUT LPBOOL lpbHalted );

    BOOL ResetPipe( IN UINT address,
                    IN UINT pipeIndex );
    
    virtual VOID PowerMgmtCallback( IN BOOL fOff )=0;
    CRootHub* GetRootHub() { return m_pCRootHub;};
    CRootHub* SetRootHub(CRootHub* pRootHub) ;
    virtual BOOL DisableDevice( IN const UINT address, 
                                  IN const BOOL fReset ) ;
    
    virtual BOOL SuspendResume( IN const UINT address,
                                  IN const BOOL fSuspend );

    // Abstract for RootHub Function.
    virtual BOOL DidPortStatusChange( IN const UCHAR port )=0;
    virtual BOOL GetPortStatus( IN const UCHAR port,
                               OUT USB_HUB_AND_PORT_STATUS& rStatus )=0;
    virtual BOOL RootHubFeature( IN const UCHAR port,
                                IN const UCHAR setOrClearFeature,
                                IN const USHORT feature )=0;
    virtual BOOL ResetAndEnablePort( IN const UCHAR port )=0;
    virtual void DisablePort( IN const UCHAR port )=0;
    virtual BOOL WaitForPortStatusChange (HANDLE /*m_hHubChanged*/) { return FALSE; };    
    virtual BOOL    SuspendHC() { return FALSE; }; // Default does not support it function.
    virtual BOOL    InitFIFO(IN CPipe * const pPipe)=0;
    virtual BOOL    UpdateDataToggle(IN CPipe * const pPipe, BOOL fReset, BOOL fUpdate)=0;
    PVOID   GetOTGContext() { return m_pOTG;};
    void    SetOTGContext(PVOID pOTG) { m_pOTG = pOTG;};    
    virtual void    SetSignalDisconnectACK(BOOL fAck) = 0;

    // ****************************************************
    // Public Variables for Chcd
    // ****************************************************
    virtual BOOL ConfigEP(IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor, 
        UCHAR mappedEP, UCHAR bDevAddr, UCHAR bHubAddr, UCHAR bHubPort, int speed, UCHAR transferMode, BOOL bClrTog)=0;
    virtual BOOL ProcessTD(IN const UCHAR endpoint, void *pQTD)=0;
    virtual BOOL SendOutDMA(IN const UCHAR endpoint, void *pQTD)=0;
    virtual BOOL WriteFIFO(void *pContext, IN const UCHAR endpoint, void *pData, DWORD size) = 0;
    virtual DWORD ReadFIFO(void *pOTGContext, IN const UCHAR endpoint, void *pData, DWORD size, int *pRet) = 0;
    virtual BOOL WriteDMA(void * pContext, UCHAR endpoint, UCHAR channel, void *ppData, 
        DWORD size, DWORD dwMaxPacket, void *pQTD) = 0;
    virtual void SetDeviceAddress(UCHAR endpoint, UCHAR ucDeviceAddress, 
        UCHAR ucHubAddress, UCHAR ucHubPort, UCHAR out) = 0;
    virtual BOOL IsHostConnect(void) = 0;
    virtual BOOL IsDMASupport(void) = 0;   
    virtual DWORD GetDMAMode(void)= 0;
    virtual BOOL LockEP0(UCHAR address) = 0;
    virtual void UnlockEP0(UCHAR address) = 0;
    virtual BOOL IsDeviceLockEP0(UCHAR address) = 0;
    virtual BOOL GetFrameNumber( OUT LPDWORD lpdwFrameNumber )=0;

    virtual BOOL GetFrameLength( OUT LPUSHORT lpuFrameLength )=0;
    
    virtual BOOL SetFrameLength( IN HANDLE hEvent,
                                IN USHORT uFrameLength )=0;
    
    virtual BOOL StopAdjustingFrame( void ) = 0;

    BOOL BusChildIoControl(DWORD dwIoControl,LPVOID lpBuffer,DWORD nBufferSize);
    BOOL BusIoControl(DWORD dwIoControl,LPVOID lpInBuffer,DWORD nInBufferSize,LPVOID lpOutBuffer,DWORD nOutBufferSize,LPDWORD lpdReturnByte);

    // SignalHubStatusChange is member function for HCD to signal a hub status change.
    // It is implemented specially for CExternalHub which provides another way to process
    // HubStatusChangeThread besides waiting for interrupt pipe signal.
    virtual void SignalHubStatusChange() {
        if (m_pCRootHub!=NULL)
            m_pCRootHub->SignalHubStatusChange();
    }

    // no public variables

protected:
    virtual BOOL ResumeNotification ()  {
        Lock();
        BOOL fReturn = FALSE;
        if (m_pCRootHub) {
            fReturn = m_pCRootHub->ResumeNotification();
            m_pCRootHub->NotifyOnSuspendedResumed(FALSE);
            m_pCRootHub->NotifyOnSuspendedResumed(TRUE);
        }
        Unlock();
        return fReturn;
    }
private:
    // ****************************************************
    // Private Functions for CUhcd
    // ****************************************************

    // ****************************************************
    // Private Variables for CUhcd
    // ****************************************************
    CRootHub*       m_pCRootHub;            // pointer to CRootHub object, which represents                                            
                                            // the built-in hardware USB ports
    PVOID           m_pOTG;                 // pointer to PHSMUSB_T    
    HANDLE          m_hParentBusDriver;
};


CHcd * CreateHCDObject(IN LPVOID pvUhcdPddObject,
                     IN CPhysMem * pCPhysMem,
                     IN LPCWSTR szDriverRegistryKey,
                     IN REGISTER portBase,
                     IN DWORD dwSysIntr);
    
extern "C"
{

BOOL HcdGetFrameNumber(LPVOID lpvHcd, LPDWORD lpdwFrameNumber);
BOOL HcdGetFrameLength(LPVOID lpvHcd, LPUSHORT lpuFrameLength);
BOOL HcdSetFrameLength(LPVOID lpvHcd, HANDLE hEvent, USHORT uFrameLength);
BOOL HcdStopAdjustingFrame(LPVOID lpvHcd);


BOOL HcdOpenPipe(LPVOID lpvHcd, UINT iDevice,
                 LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                 LPUINT lpiEndpointIndex);
BOOL HcdClosePipe(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex);
BOOL HcdResetPipe(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex);
BOOL HcdIsPipeHalted(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex,
        LPBOOL lpbHalted);


BOOL HcdIssueTransfer(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex,
                      LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                      LPVOID lpvNotifyParameter, DWORD dwFlags,
                      LPCVOID lpvControlHeader, DWORD dwStartingFrame,
                      DWORD dwFrames, LPCDWORD aLengths, DWORD dwBufferSize,
                      LPVOID lpvBuffer, ULONG paBuffer, LPCVOID lpvCancelId,
                      LPDWORD adwIsochErrors, LPDWORD adwIsochLengths,
                      LPBOOL lpfComplete, LPDWORD lpdwBytesTransfered,
                      LPDWORD lpdwError);

BOOL HcdAbortTransfer(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex,
                      LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                      LPVOID lpvNotifyParameter, LPCVOID lpvCancelId);

}

#endif

