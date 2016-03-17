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
//     cpipe.h
// 
// Abstract: Implements class for managing open pipes for UHCI
//
//                             CPipe (ADT)
//                           /             \
//                  CQueuedPipe (ADT)       CIsochronousPipe (not supported)
//                /         |       \ 
//              /           |         \
//   CControlPipe    CInterruptPipe    CBulkPipe
// 
// Notes: 
// 
#ifndef __CPIPE_H_
#define __CPIPE_H_
#include <globals.hpp>
#include <pipeabs.hpp>
#include "ctd.h"
#include "usb2lib.h"
class CPhysMem;
class CMhcd;

// According to MUSBMHDRC controller speed setting
enum {
    UNUSED_SPEED,
    HIGH_SPEED,
    FULL_SPEED,
    LOW_SPEED
};

// Define the transfer mode
enum {
    TRANSFER_FIFO,
    TRANSFER_DMA0,
    TRANSFER_DMA1
};
typedef struct STRANSFER STransfer;
class CTransfer ;
class CPipe : public CPipeAbs {
public:
    // ****************************************************
    // Public Functions for CPipe
    // ****************************************************

    CPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
           IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
           IN const UCHAR bDeviceAddress,
           IN const UCHAR bHubAddress,IN const UCHAR bHubPort,IN const BOOL ttContext,
           IN CMhcd * const pCMhcd);

    virtual ~CPipe();

    virtual HCD_REQUEST_STATUS  OpenPipe( void ) = 0;

    virtual HCD_REQUEST_STATUS  ClosePipe( void ) = 0;

    virtual HCD_REQUEST_STATUS AbortTransfer( 
                                IN const LPTRANSFER_NOTIFY_ROUTINE lpCancelAddress,
                                IN const LPVOID lpvNotifyParameter,
                                IN LPCVOID lpvCancelId ) = 0;

    HCD_REQUEST_STATUS IsPipeHalted( OUT LPBOOL const lpbHalted );
    UCHAR   GetSMask(){ return  m_bFrameSMask; };
    UCHAR   GetCMask() { return m_bFrameCMask; };
    BOOL    IsHighSpeed() { return m_fIsHighSpeed; };
    BOOL    IsLowSpeed() { return m_fIsLowSpeed; };
    virtual CPhysMem * GetCPhysMem();
    virtual HCD_REQUEST_STATUS  ScheduleTransfer( void ) = 0;
    virtual BOOL    CheckForDoneTransfers( void ) = 0;
    virtual BOOL    ProcessEP(DWORD dwStatus, BOOL isDMAIntr) = 0;
    virtual const DWORD   GetTransferType(void) const = 0;
#ifdef DEBUG
    virtual const TCHAR*  GetPipeType( void ) const = 0;
#endif // DEBUG


    virtual void ClearHaltedFlag( void );
    USB_ENDPOINT_DESCRIPTOR GetEndptDescriptor() { return m_usbEndpointDescriptor;};
    UCHAR GetDeviceAddress() { return m_bDeviceAddress; };
    UCHAR GetSpeed() {return (UCHAR)(m_fIsHighSpeed? HIGH_SPEED : (m_fIsLowSpeed? LOW_SPEED: FULL_SPEED));};
    BOOL  SetReservedDeviceAddr(UCHAR bReservedDeviceAddr) { m_bReservedDeviceAddr = bReservedDeviceAddr; return TRUE;};
    UCHAR GetReservedDeviceAddr() { return m_bReservedDeviceAddr;};
    void  SetTxFIFO(DWORD addr, DWORD size) { m_tx_fifo_addr = addr; m_tx_fifo_size = size; };
    void  SetRxFIFO(DWORD addr, DWORD size) { m_rx_fifo_addr = addr; m_rx_fifo_size = size; };    
    UCHAR   GetMappedEndPoint(void) { return m_mappedEndpoint; };
    void    SetMappedEndPoint(UCHAR mappedEP) { m_mappedEndpoint = mappedEP;};
    UCHAR   GetDataToggle(void);
    void    SetDataToggle(UCHAR dataToggle);
    BOOL    IsDataToggle(void);
    HANDLE  GetEPTransferEvent(void) { return m_hEPTransferEvent;};
    HANDLE  GetDMATransferEvent(void) { return m_hDMATransferEvent;};
    UCHAR   GetTransferMode(void) { return m_ucTransferMode;};
    void    SetTransferMode(UCHAR mode) { m_ucTransferMode = mode;};
    //void  CalculateFIFO(void);
    void ResetEndPoint(void);

#ifdef DEBUG
    const TCHAR* GetControllerName( void ) const
    {
        static const TCHAR* cszDeviceType = TEXT("MUSB");
        return cszDeviceType; 
    }
#endif // DEBUG

    virtual CQH *  GetQHead() = 0;    
    
    // ****************************************************
    // Public Variables for CPipe
    // ****************************************************
    UCHAR const m_bHubAddress;
    UCHAR const m_bHubPort;
    //UCHAR m_bTierNumber;
    //UCHAR m_bPortNo;         
    CMhcd * const m_pCMhcd;
    BOOL const m_TTContext;

private:
    // ****************************************************
    // Private Functions for CPipe
    // ****************************************************
    UCHAR m_ucTransferMode;
    UCHAR m_ucDataToggle;
    BOOL  m_bProcessDataToggle;    
    UCHAR m_bReservedDeviceAddr;   

protected:
    // ****************************************************
    // Protected Functions for CPipe
    // ****************************************************
    virtual BOOL    AreTransferParametersValid( const STransfer *pTransfer = NULL ) const = 0;

    
    // pipe specific variables
    UCHAR   m_bFrameSMask;
    UCHAR   m_bFrameCMask;
    CRITICAL_SECTION        m_csPipeLock;           // crit sec for this specific pipe's variables
    USB_ENDPOINT_DESCRIPTOR m_usbEndpointDescriptor; // descriptor for this pipe's endpoint
    UCHAR                   m_bDeviceAddress;       // Device Address that assigned by HCD.
    BOOL                    m_fIsLowSpeed;          // indicates speed of this pipe
    BOOL                    m_fIsHighSpeed;         // Indicates speed of this Pipe;
    BOOL                    m_fIsHalted;            // indicates pipe is halted        
    DWORD                   m_tx_fifo_addr;
    DWORD                   m_rx_fifo_addr;
    DWORD                   m_tx_fifo_size;
    DWORD                   m_rx_fifo_size;
    UCHAR                   m_mappedEndpoint;
    HANDLE                  m_hEPTransferEvent;
    HANDLE                  m_hDMATransferEvent;

};
class CQueuedPipe : public CPipe
{

public:
    // ****************************************************
    // Public Functions for CQueuedPipe
    // ****************************************************
    CQueuedPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                 IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                 IN const UCHAR bDeviceAddress,
                 IN const UCHAR bHubAddress,IN const UCHAR bHubPort,IN const BOOL ttContext,
                 IN CMhcd * const pCMhcd);
    virtual ~CQueuedPipe();

    inline const int GetTdSize( void ) const { return sizeof(CQTD); };

    HCD_REQUEST_STATUS  IssueTransfer( 
                                IN const UCHAR address,
                                IN LPTRANSFER_NOTIFY_ROUTINE const lpfnCallback,
                                IN LPVOID const lpvCallbackParameter,
                                IN const DWORD dwFlags,
                                IN LPCVOID const lpvControlHeader,
                                IN const DWORD dwStartingFrame,
                                IN const DWORD dwFrames,
                                IN LPCDWORD const aLengths,
                                IN const DWORD dwBufferSize,     
                                IN_OUT LPVOID const lpvBuffer,
                                IN const ULONG paBuffer,
                                IN LPCVOID const lpvCancelId,
                                OUT LPDWORD const adwIsochErrors,
                                OUT LPDWORD const adwIsochLengths,
                                OUT LPBOOL const lpfComplete,
                                OUT LPDWORD const lpdwBytesTransferred,
                                OUT LPDWORD const lpdwError ) ;

    HCD_REQUEST_STATUS AbortTransfer( 
                                IN const LPTRANSFER_NOTIFY_ROUTINE lpCancelAddress,
                                IN const LPVOID lpvNotifyParameter,
                                IN LPCVOID lpvCancelId );
    CQH *  GetQHead() { return m_pPipeQHead; };
    HCD_REQUEST_STATUS  ScheduleTransfer( void );
    BOOL    CheckForDoneTransfers(void );
    BOOL    ProcessEP(DWORD dwStatus, BOOL isDMA);
    virtual void ClearHaltedFlag( void );
    // ****************************************************
    // Public Variables for CQueuedPipe
    // ****************************************************

protected:
    // ****************************************************
    // Private Functions for CQueuedPipe
    // ****************************************************
    void  AbortQueue( void ); 
    // ****************************************************
    // Private Variables for CQueuedPipe
    // ****************************************************

    // ****************************************************
    // Protected Functions for CQueuedPipe
    // ****************************************************
    CQH *   m_pPipeQHead;

    // ****************************************************
    // Protected Variables for CQueuedPipe
    // ****************************************************
    //BOOL         m_fIsReclamationPipe; // indicates if this pipe is participating in bandwidth reclamation
//    UCHAR        m_dataToggle;         // Transfer data toggle.
    // WARNING! These parameters are treated as a unit. They
    // can all be wiped out at once, for example when a 
    // transfer is aborted.
    CQTransfer *             m_pUnQueuedTransfer;      // ptr to last transfer in queue
    CQTransfer *             m_pQueuedTransfer;
    
};

class CBulkPipe : public CQueuedPipe
{
public:
    // ****************************************************
    // Public Functions for CBulkPipe
    // ****************************************************
    CBulkPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,IN const BOOL ttContext,
               IN CMhcd * const pCMhcd);
    ~CBulkPipe();

    HCD_REQUEST_STATUS  OpenPipe( void );
    HCD_REQUEST_STATUS  ClosePipe( void );
    // ****************************************************
    // Public variables for CBulkPipe
    // ****************************************************
    const DWORD   GetTransferType(void) const
    {
        return USB_ENDPOINT_TYPE_BULK;
    }
#ifdef DEBUG
    const TCHAR*  GetPipeType( void ) const
    {
        return TEXT("Bulk");
    }
#endif // DEBUG
private:
    // ****************************************************
    // Private Functions for CBulkPipe
    // ****************************************************

    BOOL   AreTransferParametersValid( const STransfer *pTransfer = NULL ) const;

    // ****************************************************
    // Private variables for CBulkPipe
    // ****************************************************
};
class CControlPipe : public CQueuedPipe
{
public:
    // ****************************************************
    // Public Functions for CBulkPipe
    // ****************************************************
    CControlPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,IN const BOOL ttContext,
               IN CMhcd * const pCMhcd);
    ~CControlPipe();
    void ChangeMaxPacketSize( IN const USHORT wMaxPacketSize );
    HCD_REQUEST_STATUS  OpenPipe( void );
    HCD_REQUEST_STATUS  ClosePipe( void );
    // ****************************************************
    // Public variables for CBulkPipe
    // ****************************************************
    const DWORD   GetTransferType(void) const
    {
        return USB_ENDPOINT_TYPE_CONTROL;
    }

#ifdef DEBUG
    const TCHAR*  GetPipeType( void ) const
    {
        return TEXT("Control");
    }
#endif // DEBUG    
    HCD_REQUEST_STATUS  IssueTransfer( 
                                IN const UCHAR address,
                                IN LPTRANSFER_NOTIFY_ROUTINE const lpfnCallback,
                                IN LPVOID const lpvCallbackParameter,
                                IN const DWORD dwFlags,
                                IN LPCVOID const lpvControlHeader,
                                IN const DWORD dwStartingFrame,
                                IN const DWORD dwFrames,
                                IN LPCDWORD const aLengths,
                                IN const DWORD dwBufferSize,     
                                IN_OUT LPVOID const lpvBuffer,
                                IN const ULONG paBuffer,
                                IN LPCVOID const lpvCancelId,
                                OUT LPDWORD const adwIsochErrors,
                                OUT LPDWORD const adwIsochLengths,
                                OUT LPBOOL const lpfComplete,
                                OUT LPDWORD const lpdwBytesTransferred,
                                OUT LPDWORD const lpdwError ) ;

private:
    // ****************************************************
    // Private Functions for CBulkPipe
    // ****************************************************

    BOOL   AreTransferParametersValid( const STransfer *pTransfer = NULL ) const;

    // ****************************************************
    // Private variables for CBulkPipe
    // ****************************************************
};
class CInterruptPipe : public CQueuedPipe
{
public:
    // ****************************************************
    // Public Functions for CInterruptPipe
    // ****************************************************
CInterruptPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,IN const BOOL ttContext,
               IN CMhcd * const pCMhcd);
~CInterruptPipe();

    HCD_REQUEST_STATUS  OpenPipe( void );
    HCD_REQUEST_STATUS  ClosePipe( void );
    const DWORD   GetTransferType(void) const
    {
        return USB_ENDPOINT_TYPE_INTERRUPT;
    }

#ifdef DEBUG
    const TCHAR*  GetPipeType( void ) const
    {
        static const TCHAR* cszPipeType = TEXT("Interrupt");
        return cszPipeType;
    }
#endif // DEBUG    

private:
    // ****************************************************
    // Private Functions for CInterruptPipe
    // ****************************************************


    void                UpdateInterruptQHTreeLoad( IN const UCHAR branch,
                                                   IN const int   deltaLoad );

    BOOL                AreTransferParametersValid( const STransfer *pTransfer = NULL ) const;


    HCD_REQUEST_STATUS  AddTransfer( CQTransfer *pTransfer );

    EndpointBuget m_EndptBuget;

    BOOL m_bSuccess;
    // ****************************************************
    // Private variables for CInterruptPipe
    // ****************************************************
};
#endif
