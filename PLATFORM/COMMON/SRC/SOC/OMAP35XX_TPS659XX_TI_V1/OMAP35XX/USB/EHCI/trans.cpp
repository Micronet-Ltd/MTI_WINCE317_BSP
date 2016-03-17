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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
// 
// Module Name:  
//     Trans.cpp
// 
// Abstract: Provides interface to UHCI host controller
// 
// Notes: 
//
#include <windows.h>
#include <Cphysmem.hpp>
#include "trans.h"
#include "cpipe.h"
#include "chw.h"
#include "cehcd.h"

#ifndef _PREFAST_
#pragma warning(disable: 4068) // Disable pragma warnings
#endif

#define ROOT_HUB_RESTART_EVENT_NAME TEXT("RootHubEventName")

DWORD CTransfer::m_dwGlobalTransferID=0;
CTransfer::CTransfer(IN CPipe * const pCPipe, IN CPhysMem * const pCPhysMem,STransfer sTransfer) 
    : m_sTransfer( sTransfer)
    , m_pCPipe(pCPipe)
    , m_pCPhysMem(pCPhysMem)
{
    m_pNextTransfer=NULL;
    m_paControlHeader=0;
    m_pAllocatedForControl=NULL;
    m_pAllocatedForClient=NULL;
    memcpy(&m_sTransfer, &sTransfer,sizeof(STransfer));
    m_DataTransferred =0 ;
    m_dwTransferID = m_dwGlobalTransferID++;
    m_fDoneTransferCalled = FALSE;

}
CTransfer::~CTransfer()
{
    if (m_pAllocatedForControl!=NULL) 
        m_pCPhysMem->FreeMemory( PUCHAR(m_pAllocatedForControl),m_paControlHeader,  CPHYSMEM_FLAG_NOBLOCK );
    if (m_pAllocatedForClient!=NULL)
        m_pCPhysMem->FreeMemory( PUCHAR(m_pAllocatedForClient), m_sTransfer.paBuffer,  CPHYSMEM_FLAG_NOBLOCK );
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE , (TEXT("CTransfer::~CTransfer() (this=0x%x,m_pAllocatedForControl=0x%x,m_pAllocatedForClient=0x%x)\r\n"),
        this,m_pAllocatedForControl,m_pAllocatedForClient));

}
BOOL CTransfer::Init(void)
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CTransfer::Init (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    // We must allocate the control header memory here so that cleanup works later.
    if (m_sTransfer.lpvControlHeader != NULL &&  m_pAllocatedForControl == NULL ) {
        // This must be a control transfer. It is asserted elsewhere,
        // but the worst case is we needlessly allocate some physmem.
        if ( !m_pCPhysMem->AllocateMemory(
                                   DEBUG_PARAM( TEXT("IssueTransfer SETUP Buffer") )
                                   sizeof(USB_DEVICE_REQUEST),
                                   &m_pAllocatedForControl,
                                   CPHYSMEM_FLAG_NOBLOCK ) ) {
            DEBUGMSG( ZONE_WARNING, (TEXT("CPipe(%s)::IssueTransfer - no memory for SETUP buffer\n"), m_pCPipe->GetPipeType() ) );
            m_pAllocatedForControl=NULL;
            return FALSE;
        }
        m_paControlHeader = m_pCPhysMem->VaToPa( m_pAllocatedForControl );
        DEBUGCHK( m_pAllocatedForControl != NULL && m_paControlHeader != 0 );

        __try {
            memcpy(m_pAllocatedForControl,m_sTransfer.lpvControlHeader,sizeof(USB_DEVICE_REQUEST));
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            // bad lpvControlHeader
            return FALSE;
        }
    }
#ifdef DEBUG
    if ( m_sTransfer.dwFlags & USB_IN_TRANSFER ) {
        // I am leaving this in for two reasons:
        //  1. The memset ought to work even on zero bytes to NULL.
        //  2. Why would anyone really want to do a zero length IN?
        DEBUGCHK( m_sTransfer.dwBufferSize > 0 &&
                  m_sTransfer.lpvBuffer != NULL );
        __try { // IN buffer, trash it
            memset( PUCHAR( m_sTransfer.lpvBuffer ), GARBAGE, m_sTransfer.dwBufferSize );
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
        }
    }
#endif // DEBUG

    if ( m_sTransfer.dwBufferSize > 0 && m_sTransfer.paBuffer == 0 ) { 

        // ok, there's data on this transfer and the client
        // did not specify a physical address for the
        // buffer. So, we need to allocate our own.

        if ( !m_pCPhysMem->AllocateMemory(
                                   DEBUG_PARAM( TEXT("IssueTransfer Buffer") )
                                   m_sTransfer.dwBufferSize,
                                   &m_pAllocatedForClient, 
                                   CPHYSMEM_FLAG_NOBLOCK ) ) {
            DEBUGMSG( ZONE_WARNING, (TEXT("CPipe(%s)::IssueTransfer - no memory for TD buffer\n"), m_pCPipe->GetPipeType() ) );
            m_pAllocatedForClient = NULL;
            return FALSE;
        }
        m_sTransfer.paBuffer = m_pCPhysMem->VaToPa( m_pAllocatedForClient );
        PREFAST_DEBUGCHK( m_pAllocatedForClient != NULL);
        PREFAST_DEBUGCHK( m_sTransfer.lpvBuffer!=NULL);
        DEBUGCHK(m_sTransfer.paBuffer != 0 );

        if ( !(m_sTransfer.dwFlags & USB_IN_TRANSFER) ) {
            __try { // copying client buffer for OUT transfer
                memcpy( m_pAllocatedForClient, m_sTransfer.lpvBuffer, m_sTransfer.dwBufferSize );
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                  // bad lpvClientBuffer
				RETAILMSG( 1, (TEXT("CQTransfer::Init bad lpvClientBuffer\r\n")));
                  return FALSE;
            }
        }
    }
    
    DEBUGMSG(  ZONE_TRANSFER && ZONE_VERBOSE , (TEXT("CQTransfer::Init (this=0x%x,id=0x%x),m_pAllocatedForControl=0x%x,m_pAllocatedForClient=0x%x\r\n"),
        this,m_dwTransferID,m_pAllocatedForControl,m_pAllocatedForClient));
    return AddTransfer();
}

CQTransfer::~CQTransfer()
{
    CQTD * pCurTD = m_pCQTDList;
    while (pCurTD!=NULL) {
         CQTD * pNextTD = pCurTD->GetNextTD();
         pCurTD->~CQTD();
         m_pCPhysMem->FreeMemory((PBYTE)pCurTD,m_pCPhysMem->VaToPa((PBYTE)pCurTD), CPHYSMEM_FLAG_HIGHPRIORITY |CPHYSMEM_FLAG_NOBLOCK );
         pCurTD = pNextTD;
    }
}
BOOL CQTransfer::AddTransfer() 
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer::AddTransfer (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    if (m_pCQTDList) { // Has been created. Somthing wrong.
        ASSERT(FALSE);
        return FALSE;
    }
    BOOL bDataToggle1= FALSE;
    CQTD * pStatusTD = NULL;
    if (m_paControlHeader!=NULL && m_sTransfer.lpvControlHeader!=NULL) { 
        // This is setup packet.
        if (m_pCQTDList = new(m_pCPhysMem) CQTD(this, ((CQueuedPipe * const)m_pCPipe)->GetQHead())) {            
            PhysBufferArray bufferArray;
            bufferArray.dwNumOfBlock=1;
            bufferArray.dwStartOffset=m_paControlHeader & EHCI_PAGE_OFFSET_MASK;
            bufferArray.dwBlockSize=min(bufferArray.dwStartOffset + sizeof(USB_DEVICE_REQUEST),EHCI_PAGE_SIZE );
            bufferArray.dwArrayBlockAddr[0]=(m_paControlHeader & EHCI_PAGE_ADDR_MASK);
            bufferArray.dwArrayBlockAddr[1]=((m_paControlHeader+sizeof(USB_DEVICE_REQUEST)) & EHCI_PAGE_ADDR_MASK ); // Terminate
            m_pCQTDList->IssueTransfer( TD_SETUP_PID, bDataToggle1, sizeof(USB_DEVICE_REQUEST),&bufferArray,TRUE);
            bDataToggle1 = !bDataToggle1;
        }
        else 
		{
			RETAILMSG( 1, (TEXT("CQTransfer::AddTransfer (this=0x%x,id=0x%x) Failed to allocate QTD setup packet\r\n"),this,m_dwTransferID));
            return FALSE;
		}
        // Status Packet
        pStatusTD = new(m_pCPhysMem) CQTD(this, ((CQueuedPipe * const)m_pCPipe)->GetQHead());
        if (pStatusTD) {            
            PhysBufferArray bufferArray;
            bufferArray.dwNumOfBlock=0;
            bufferArray.dwBlockSize=0;
            bufferArray.dwStartOffset=0;
            bufferArray.dwArrayBlockAddr[0]=0;
            bufferArray.dwArrayBlockAddr[1]=0;// Terminate
            pStatusTD->IssueTransfer( (m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0?TD_OUT_PID:TD_IN_PID,
                TRUE, 0 ,&bufferArray,TRUE);
        }
        else 
		{
			RETAILMSG( 1, (TEXT("CQTransfer::AddTransfer (this=0x%x,id=0x%x) Failed to allocate QTD Status Packet\r\n"),this,m_dwTransferID));
            return FALSE;
		}
    };
    CQTD * pPrevTD=m_pCQTDList;
    if ( m_pCQTDList==NULL  && m_sTransfer.dwBufferSize == 0 ) { // No Control but Zero Length
        ASSERT((m_sTransfer.dwFlags & USB_IN_TRANSFER)==0);// No meaning for IN Zero length packet.
        CQTD * pCurTD = new( m_pCPhysMem) CQTD(this, ((CQueuedPipe * const)m_pCPipe)->GetQHead());
        if (pCurTD) {            
            PhysBufferArray bufferArray;
            bufferArray.dwNumOfBlock=0;
            bufferArray.dwBlockSize=0;
            bufferArray.dwStartOffset=0;
            bufferArray.dwArrayBlockAddr[0]=0;
            bufferArray.dwArrayBlockAddr[1]=0;// Terminate
            pCurTD->IssueTransfer( (m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0? TD_IN_PID: TD_OUT_PID,
                bDataToggle1, 0 ,&bufferArray,TRUE);
            if (pPrevTD) {
                pPrevTD->QueueNextTD(pCurTD);
                pPrevTD=pCurTD;
            }
            else { // THis is First. So update m_pQTDList
                pPrevTD= m_pCQTDList = pCurTD;
            }
        }
        else {
            if ( pStatusTD) {
                pStatusTD->~CQTD();
                m_pCPhysMem->FreeMemory((PBYTE)pStatusTD,m_pCPhysMem->VaToPa((PBYTE)pStatusTD), CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK );
            }
            return FALSE;                
        }
    }
    else
    if (m_sTransfer.lpvBuffer &&  m_sTransfer.paBuffer && m_sTransfer.dwBufferSize) {
        DWORD dwCurPos=0;
        while ( dwCurPos< m_sTransfer.dwBufferSize) {
            CQTD * pCurTD = new( m_pCPhysMem) CQTD(this, ((CQueuedPipe * const)m_pCPipe)->GetQHead());
            ASSERT(pCurTD!=NULL);
            if (pCurTD==NULL) {
                // delete  pStatusTD;
				RETAILMSG( 1, (TEXT("CQTransfer::AddTransfer (this=0x%x,id=0x%x) Failed to allocate CQTD2\r\n"),this,m_dwTransferID));
                if ( pStatusTD) {
                    pStatusTD->~CQTD();
                    m_pCPhysMem->FreeMemory((PBYTE)pStatusTD,m_pCPhysMem->VaToPa((PBYTE)pStatusTD), CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK );
                }
                return FALSE;                
            }                
            DWORD dwCurPhysAddr=  m_sTransfer.paBuffer + dwCurPos;
            // We only can queue maximun 4 page for one TD and  Align with Packet Size.
            DWORD dwPacketSize= (m_pCPipe->GetEndptDescriptor()).wMaxPacketSize & 0x7ff;
            ASSERT(dwPacketSize!=0);
            DWORD dwCurLength = ((EHCI_PAGE_SIZE * MAX_QTD_PAGE_SIZE)/dwPacketSize)*dwPacketSize;            
            dwCurLength = min( m_sTransfer.dwBufferSize- dwCurPos,dwCurLength);
            PhysBufferArray bufferArray;
            bufferArray.dwNumOfBlock=  (dwCurLength + EHCI_PAGE_SIZE -1 ) /EHCI_PAGE_SIZE;
            ASSERT(bufferArray.dwNumOfBlock<=MAX_QTD_PAGE_SIZE && bufferArray.dwNumOfBlock!=0);
            bufferArray.dwStartOffset= dwCurPhysAddr & EHCI_PAGE_OFFSET_MASK;
            bufferArray.dwBlockSize = min (bufferArray.dwStartOffset + m_sTransfer.dwBufferSize- dwCurPos,EHCI_PAGE_SIZE);
            
            for (DWORD dwIndex=0;dwIndex<bufferArray.dwNumOfBlock+1 && dwIndex<5;dwIndex++) {
                bufferArray.dwArrayBlockAddr[dwIndex] = (dwCurPhysAddr & EHCI_PAGE_ADDR_MASK);
                dwCurPhysAddr += EHCI_PAGE_SIZE;
            }
            DWORD dwReturnLength = pCurTD->IssueTransfer( (m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0?TD_IN_PID:TD_OUT_PID,
                bDataToggle1, dwCurLength,&bufferArray,TRUE);
            ASSERT(dwReturnLength == dwCurLength);

            // Setup for Short Packet
            if ( pStatusTD) {
                pCurTD->SetAltNextQTDPointer(pStatusTD->GetPhysAddr());
            }
            
            DWORD dwNumOfPacket = (dwReturnLength + dwPacketSize-1)/dwPacketSize;
            
            dwCurPos += dwReturnLength;
            if (pPrevTD) {
                pPrevTD->QueueNextTD(pCurTD);
                pPrevTD=pCurTD;
            }
            else { // THis is First. So update m_pQTDList
                pPrevTD= m_pCQTDList = pCurTD;
            }
            if ((dwNumOfPacket & 1)!=0) // if it is odd packet number. Toggle the data toggle.
                bDataToggle1 = !bDataToggle1;
        }
    }
    // We have to append Status TD here.
    if (pStatusTD) { // This is setup packet.
        if (pPrevTD) {
            pPrevTD->QueueNextTD(pStatusTD);
            pPrevTD=pStatusTD;
        }
        else { // Something Wrong.
            ASSERT(FALSE);
			RETAILMSG( 1, (TEXT("CQTransfer::AddTransfer (this=0x%x,id=0x%x) Something Wrong.\r\n"),this,m_dwTransferID));
            //delete pCurTD;
            pStatusTD->~CQTD();
            m_pCPhysMem->FreeMemory((PBYTE)pStatusTD,m_pCPhysMem->VaToPa((PBYTE)pStatusTD), CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK  );
            return FALSE;
        }
    };
    return TRUE;    
}
BOOL CQTransfer::DoneTransfer()
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer::DoneTransfer (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    BOOL bIsTransDone = IsTransferDone();
    ASSERT(bIsTransDone == TRUE);
    if (bIsTransDone && !m_fDoneTransferCalled) {
        DWORD dwUsbError = USB_NO_ERROR;
        DWORD dwDataNotTransferred = 0;
        m_fDoneTransferCalled = TRUE;
        
        CQTD * pCurTD = m_pCQTDList;
        BOOL bReturn=TRUE;
        while ( pCurTD!=NULL) {
            pCurTD->CheckStructure();
            if (  pCurTD->qTD_Token.qTD_TContext.PID != 2) // Do not count Setup TD
                dwDataNotTransferred +=  pCurTD ->qTD_Token.qTD_TContext.BytesToTransfer ;
            if (pCurTD->qTD_Token.qTD_TContext.Halted==1) { // This Transfer Has been halted due to error.
                // This is error. We do not have error code for EHCI so generically we set STALL error.
                if (pCurTD->qTD_Token.qTD_TContext.BabbleDetected) {// Babble.
                    dwUsbError = USB_DATA_OVERRUN_ERROR ;
                }
                else if (pCurTD->qTD_Token.qTD_TContext.DataBufferError) {
                    dwUsbError = ((m_sTransfer.dwFlags &USB_IN_TRANSFER)!=0? USB_BUFFER_OVERRUN_ERROR : USB_BUFFER_UNDERRUN_ERROR);
                }
                else  if (dwUsbError == USB_NO_ERROR)
                    dwUsbError = USB_STALL_ERROR;
            }
            else
            if (pCurTD->qTD_Token.qTD_TContext.Active ==1) {
                if (dwUsbError == USB_NO_ERROR)
                    dwUsbError = USB_NOT_COMPLETE_ERROR;
                break;
            }
            pCurTD = pCurTD->GetNextTD();
        }
        ASSERT(dwDataNotTransferred <= m_sTransfer.dwBufferSize);
        if (dwDataNotTransferred >= m_sTransfer.dwBufferSize)
            dwDataNotTransferred = m_sTransfer.dwBufferSize;
        m_DataTransferred = m_sTransfer.dwBufferSize -  dwDataNotTransferred ;

        // We have to update the buffer when this is IN Transfer.
        if ((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=NULL && m_pAllocatedForClient!=NULL && m_sTransfer.dwBufferSize!=0) {
            __try { // copying client buffer for OUT transfer
                memcpy( m_sTransfer.lpvBuffer, m_pAllocatedForClient, m_sTransfer.dwBufferSize );
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                  // bad lpvBuffer.
                if (dwUsbError == USB_NO_ERROR)
                    dwUsbError = USB_CLIENT_BUFFER_ERROR;
            }
        }
        if (m_fCanceled && dwUsbError == USB_NO_ERROR) {
            dwUsbError = USB_CANCELED_ERROR ;
        }

        __try { // setting transfer status and executing callback function
            if (m_sTransfer.lpfComplete !=NULL)
                *m_sTransfer.lpfComplete = TRUE;
            if (m_sTransfer.lpdwError!=NULL)
                *m_sTransfer.lpdwError = dwUsbError;
            if (m_sTransfer.lpdwBytesTransferred)
                *m_sTransfer.lpdwBytesTransferred =  m_DataTransferred;
            if ( m_sTransfer.lpStartAddress ) {
                ( *m_sTransfer.lpStartAddress )(m_sTransfer.lpvNotifyParameter );
                m_sTransfer.lpStartAddress = NULL ; // Make sure only do once.
            }
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
              DEBUGMSG( ZONE_ERROR, (TEXT("CQueuedPipe(%s)::CheckForDoneTransfers - exception setting transfer status to complete\n"), m_pCPipe->GetPipeType() ) );
			  //Signal Event for restart of EHCI
			  RETAILMSG( 1, (TEXT("CQueuedPipe::Recover Root hub by power down and up\r\n") ));
			  //Signal detach to root hub
			  HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, ROOT_HUB_RESTART_EVENT_NAME);
			  if (hEvent != NULL)
			  {
				  SetEvent(hEvent);
				  CloseHandle(hEvent);
			  }
        }
        return (dwUsbError==USB_NO_ERROR);
    }
    else
        return TRUE;
}
BOOL CQTransfer::AbortTransfer()
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer::AbortTransfer (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    CQTD * pCurTD = m_pCQTDList;
    while ( pCurTD!=NULL) {
        pCurTD->DeActiveTD();
        pCurTD = pCurTD->GetNextTD();
    }
    m_fCanceled = TRUE;
    Sleep(2);// Make sure the shcdule has advanced. and current Transfer has completeded.
    return TRUE;
}
BOOL CQTransfer::IsTransferDone()
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer::IsTransferDone (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    if (m_pCQTDList==NULL) { // Has been created. Somthing wrong.
        return TRUE;
    }
    CQTD * pCurTD = m_pCQTDList;
    BOOL bReturn=TRUE;
    while ( pCurTD!=NULL) {
        ASSERT(pCurTD->m_pTrans == this);
        if (pCurTD->qTD_Token.qTD_TContext.Halted==1) { // This Transfer Has been halted due to error.
            break;
        }
        if (pCurTD->qTD_Token.qTD_TContext.Active == 1) { 
            bReturn = FALSE;
            break;
        }
        if (pCurTD ->GetLinkValid() == FALSE) { // No like connected. This Transfer is  aborted.
            break;
        }
        pCurTD = pCurTD ->GetNextTD();
    }
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer::IsTransferDone (this=0x%x) return %d \r\n"),this,bReturn));
    return bReturn;
}
CIsochTransfer::CIsochTransfer(IN CIsochronousPipe * const pCPipe,IN CEhcd * const pCEhcd,STransfer sTransfer) 
    : CTransfer(pCPipe,pCEhcd->GetPhysMem(),sTransfer)
    ,m_pCEhcd(pCEhcd)
{
    m_dwFrameIndexStart = m_sTransfer.dwStartingFrame;
    m_dwNumOfTD =0;
    m_dwSchedTDIndex=0;

    m_dwDequeuedTDIndex=0;
    
    m_dwArmedTDIndex =0;
    m_dwArmedBufferIndex=0;
    m_dwFirstError =  USB_NO_ERROR;
    m_dwLastFrameIndex = 0;

};

inline DWORD  CIsochTransfer::GetMaxTransferPerItd()
{   
    return (GetPipe())->GetMaxTransferPerItd(); 
};

CITransfer::CITransfer(IN CIsochronousPipe * const pCPipe, IN CEhcd * const pCEhcd,STransfer sTransfer)
    :CIsochTransfer (pCPipe, pCEhcd,sTransfer)
{
    m_pCITDList =0;
    ASSERT(GetMaxTransferPerItd()!=0);
    ASSERT(GetMaxTransferPerItd() <= MAX_TRNAS_PER_ITD);
}
CITransfer::~CITransfer()
{
    ASSERT(m_dwSchedTDIndex==m_dwNumOfTD || m_dwLastFrameIndex<m_dwNumOfTD);
    ASSERT(m_dwDequeuedTDIndex ==  m_dwNumOfTD||m_dwLastFrameIndex<m_dwNumOfTD );
    if (m_pCITDList && m_dwNumOfTD) {
        AbortTransfer();
        for (DWORD dwIndex=0;dwIndex<m_dwNumOfTD;dwIndex++)
            if (*(m_pCITDList + dwIndex)) {
                ASSERT((*(m_pCITDList + dwIndex) )->GetLinkValid() != TRUE); // Invalid Next Link
                //Free *(m_pCITDList + dwIndex);
                GetPipe()->FreeCITD(*(m_pCITDList + dwIndex));
                *(m_pCITDList + dwIndex) = NULL;
            }
        delete m_pCITDList;
    }
}
BOOL CITransfer::ArmTD()
{
    BOOL bAnyArmed = FALSE;
    if (m_pCITDList && m_dwArmedTDIndex<m_dwNumOfTD ) { // Something TD wait for Arm.
        DWORD dwCurDataPhysAddr = m_sTransfer.paBuffer + m_dwArmedBufferIndex;
        DWORD dwFrameIndex = m_dwArmedTDIndex * GetMaxTransferPerItd() ;
        while (dwFrameIndex< m_sTransfer.dwFrames && m_dwArmedTDIndex < m_dwNumOfTD) {
            *(m_pCITDList + m_dwArmedTDIndex) = GetPipe()->AllocateCITD( this);
            if (*(m_pCITDList + m_dwArmedTDIndex) == NULL) 
                break;
            DWORD dwTransLenArray[MAX_TRNAS_PER_ITD+1];
            DWORD dwFrameAddrArray[MAX_TRNAS_PER_ITD+1];
            for (DWORD dwTransIndex=0; dwTransIndex <  GetMaxTransferPerItd() && dwFrameIndex < m_sTransfer.dwFrames ; dwTransIndex ++) {
                dwTransLenArray[dwTransIndex]=  *(m_sTransfer.aLengths + dwFrameIndex);
                dwFrameAddrArray[dwTransIndex] = dwCurDataPhysAddr;
                dwCurDataPhysAddr +=dwTransLenArray[dwTransIndex];
                m_dwArmedBufferIndex += dwTransLenArray[dwTransIndex];
                dwFrameIndex ++;
            }
            dwTransLenArray[dwTransIndex]=  0 ;
            dwFrameAddrArray[dwTransIndex] = dwCurDataPhysAddr;
            
            if (dwTransIndex !=0) {
                (*(m_pCITDList + m_dwArmedTDIndex))->IssueTransfer( 
                    dwTransIndex,dwTransLenArray, dwFrameAddrArray,
                    dwFrameIndex>= m_sTransfer.dwFrames ,
                    (m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0);
                (*(m_pCITDList+ m_dwArmedTDIndex ))->SetIOC(TRUE); // Interrupt On every TD.
            }
            else {
                ASSERT(FALSE);
                break;
            }
            m_dwArmedTDIndex ++;
            bAnyArmed=TRUE;
        }
    }
    return bAnyArmed;
}
BOOL CITransfer::AddTransfer () 
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer::AddTransfer (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    if (m_dwNumOfTD!=0 || m_pCITDList != NULL) {
        ASSERT(FALSE);
        return FALSE;
    }
    m_dwNumOfTD = (m_sTransfer.dwFrames + GetMaxTransferPerItd()-1)/GetMaxTransferPerItd();
    m_dwSchedTDIndex=0;
    m_dwArmedBufferIndex=0;
    m_dwArmedTDIndex = 0 ;
    m_dwFirstError =  USB_NO_ERROR;
    if (m_sTransfer.lpvBuffer &&  m_sTransfer.paBuffer && m_sTransfer.dwBufferSize) {
        // Allocate space for CITD List
        m_pCITDList =(CITD **) new PVOID[m_dwNumOfTD];
        if (m_pCITDList!=NULL) {
            memset(m_pCITDList,0,sizeof(CITD *)*m_dwNumOfTD);
            ArmTD();
            return TRUE;
        }
    }
    ASSERT(FALSE);
    return FALSE;
}
BOOL CITransfer::ScheduleTD(DWORD dwCurFrameIndex,DWORD /*dwCurMicroFrameIndex*/)
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer::ScheduleTD (this=0x%x,id=0x%x,curFrameIndex=0x%x)\r\n"),this,m_dwTransferID, dwCurFrameIndex));
    BOOL bReturn = FALSE;
    ArmTD();
    if (m_dwSchedTDIndex < m_dwNumOfTD && m_pCITDList !=0 && m_pCEhcd ) {
        if ((long)(dwCurFrameIndex - m_dwFrameIndexStart) > (long)m_dwSchedTDIndex) {
            m_dwSchedTDIndex = dwCurFrameIndex - m_dwFrameIndexStart;
        }
        m_dwSchedTDIndex = min( m_dwSchedTDIndex , m_dwNumOfTD);
        
        DWORD EndShedTDIndex = dwCurFrameIndex + (m_pCEhcd->GetPeriodicMgr()).GetFrameSize()-1;
        DWORD dwNumTDCanSched;
        if ( (long)(EndShedTDIndex - m_dwFrameIndexStart ) >= 0)
            dwNumTDCanSched =EndShedTDIndex - m_dwFrameIndexStart ;
        else  // Too Early.
            dwNumTDCanSched = 0;
        dwNumTDCanSched= min(m_dwNumOfTD ,dwNumTDCanSched);
        dwNumTDCanSched = min (m_dwArmedTDIndex, dwNumTDCanSched);
        if (m_dwSchedTDIndex < dwNumTDCanSched) { // Do scudule those index.            
            for (DWORD dwIndex = m_dwSchedTDIndex ; dwIndex<dwNumTDCanSched; dwIndex++) {
                m_pCEhcd->PeriodQueueITD(*(m_pCITDList+dwIndex),m_dwFrameIndexStart + dwIndex );
                (*(m_pCITDList+dwIndex))->CheckStructure ();
                ASSERT((*(m_pCITDList+dwIndex))->m_pTrans == this);
            }
            m_dwSchedTDIndex = dwNumTDCanSched;
            bReturn = TRUE;
        }
    }
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer::ScheduleTD (this=0x%x) return %d\r\n"),this,bReturn));
    return bReturn;
}
BOOL CITransfer::IsTransferDone(DWORD dwCurFrameIndex,DWORD /*dwCurMicroFrameIndex*/)
{
    // Dequeue those TD has Transfered.
    m_dwLastFrameIndex = dwCurFrameIndex - m_dwFrameIndexStart;
    if ((long)(dwCurFrameIndex - m_dwFrameIndexStart)>= 1 ) {
        DWORD dwTransfered = min(dwCurFrameIndex - m_dwFrameIndexStart , m_dwSchedTDIndex);
        for (DWORD dwIndex=m_dwDequeuedTDIndex;dwIndex<dwTransfered && dwIndex<m_dwNumOfTD ; dwIndex++) {
            if ( *(m_pCITDList+dwIndex) != NULL ) {
                ASSERT((*(m_pCITDList+dwIndex))->m_pTrans == this);
                BOOL bResult =m_pCEhcd->PeriodDeQueueTD((*(m_pCITDList+dwIndex))->GetPhysAddr(),dwIndex + m_dwFrameIndexStart);
                ASSERT(bResult == TRUE);
                (*(m_pCITDList + dwIndex) )->SetLinkValid(FALSE);
                (*(m_pCITDList+ dwIndex))->CheckStructure ();
                DWORD dwFrameIndex = dwIndex * GetMaxTransferPerItd();
                
                for (DWORD dwTrans=0; dwTrans<GetMaxTransferPerItd() && dwFrameIndex< m_sTransfer.dwFrames; dwTrans++) {
                    DWORD dwTDError= USB_NO_ERROR;
                    if ((*(m_pCITDList + dwIndex))->iTD_StatusControl[dwTrans].iTD_SCContext.Active!=0) 
                        dwTDError = USB_NOT_COMPLETE_ERROR;
                    else
                    if ((*(m_pCITDList + dwIndex))->iTD_StatusControl[dwTrans].iTD_SCContext.XactErr!=0) 
                        dwTDError = USB_ISOCH_ERROR;
                    else
                    if ((*(m_pCITDList + dwIndex))->iTD_StatusControl[dwTrans].iTD_SCContext.BabbleDetected!=0) 
                        dwTDError = USB_STALL_ERROR;
                    else
                    if ((*(m_pCITDList + dwIndex))->iTD_StatusControl[dwTrans].iTD_SCContext.DataBufferError!=0) 
                        dwTDError = ((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0?USB_DATA_OVERRUN_ERROR:USB_DATA_UNDERRUN_ERROR);
                        
                    if (m_dwFirstError == USB_NO_ERROR   ) { // only update first time
                        m_dwFirstError = dwTDError;
                    }
                    
                    DWORD dwTransLength =(*(m_pCITDList + dwIndex))->iTD_StatusControl[dwTrans].iTD_SCContext.TransactionLength;
                    if (dwFrameIndex< m_sTransfer.dwFrames) {
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
                        __try { // setting isoch OUT status parameters
                            m_sTransfer.adwIsochErrors[ dwFrameIndex ] = dwTDError;
                            // Document said length is only update for Isoch IN 3.3.2
                            m_sTransfer.adwIsochLengths[ dwFrameIndex ] = ((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0?
                                    dwTransLength :  *(m_sTransfer.aLengths + dwFrameIndex ));
                            m_DataTransferred += m_sTransfer.adwIsochLengths[ dwFrameIndex ];
                        } __except( EXCEPTION_EXECUTE_HANDLER ) {
                        }
#pragma prefast(pop)
                    }
                    dwFrameIndex ++;

                }
                GetPipe()->FreeCITD(*(m_pCITDList + dwIndex));
                *(m_pCITDList + dwIndex) = NULL;
            }
        }
        m_dwDequeuedTDIndex = dwIndex;
    }
    BOOL bReturn = ((long)(dwCurFrameIndex-m_dwFrameIndexStart)>= (long)m_dwNumOfTD);
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer:::IsTransferDone (this=0x%x,curFrameIndex=0x%x) return %d \r\n"),this, dwCurFrameIndex,bReturn));
    return bReturn;
}
BOOL CITransfer::AbortTransfer()
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer::AbortTransfer (this=0x%x,id=0x%x) \r\n"),this,m_dwTransferID));
    for (DWORD dwIndex=m_dwDequeuedTDIndex;dwIndex < m_dwNumOfTD && dwIndex< m_dwSchedTDIndex; dwIndex++) {
        if ( *(m_pCITDList+dwIndex) != NULL) {
            BOOL bResult=m_pCEhcd->PeriodDeQueueTD((*(m_pCITDList+dwIndex))->GetPhysAddr(),dwIndex + m_dwFrameIndexStart);
            ASSERT(bResult == TRUE);
            (*(m_pCITDList + dwIndex) )->SetLinkValid(FALSE);
            
            DWORD dwFrameIndex = dwIndex * GetMaxTransferPerItd();            
            for (DWORD dwTrans=0; dwTrans<GetMaxTransferPerItd() && dwFrameIndex< m_sTransfer.dwFrames; dwTrans++) {
                if (dwFrameIndex< m_sTransfer.dwFrames) {
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
                    __try { // setting isoch OUT status parameters
                        m_sTransfer.adwIsochErrors[ dwFrameIndex ] = USB_NOT_COMPLETE_ERROR;;
                        m_sTransfer.adwIsochLengths[ dwFrameIndex ] = 0;
                    } __except( EXCEPTION_EXECUTE_HANDLER ) {
                    }
#pragma prefast(pop)
                }
                dwFrameIndex ++;
            }
            if (m_dwFirstError == USB_NO_ERROR   ) { // only update first time
                m_dwFirstError = USB_NOT_COMPLETE_ERROR;
            }
        }
    }
    m_dwDequeuedTDIndex = m_dwSchedTDIndex = m_dwNumOfTD;
    Sleep(2); // Make Sure EHCI nolong reference to those TD;
    return DoneTransfer(m_dwFrameIndexStart+m_dwNumOfTD, 0);
};
BOOL CITransfer::DoneTransfer(DWORD dwCurFrameIndex,DWORD dwCurMicroFrameIndex)
{
    BOOL bIsTransDone = IsTransferDone(dwCurFrameIndex,dwCurMicroFrameIndex);
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer::DoneTransfer (this=0x%x,id=0x%x,curFrameIndex=0x%x, bIsTransDone=%d \r\n")
        ,this, m_dwTransferID, dwCurFrameIndex,bIsTransDone));
    ASSERT(bIsTransDone == TRUE);
    if (bIsTransDone && !m_fDoneTransferCalled ) {
        DWORD dwUsbError = USB_NO_ERROR;
        DWORD dwDataNotTransferred = 0;
        DWORD dwFrameIndex =0 ;
        m_fDoneTransferCalled = TRUE;
        // We have to update the buffer when this is IN Transfer.
        if ((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=NULL && m_pAllocatedForClient!=NULL) {
            __try { // copying client buffer for OUT transfer
                memcpy( m_sTransfer.lpvBuffer, m_pAllocatedForClient, m_sTransfer.dwBufferSize );
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                  // bad lpvBuffer.
                if (dwUsbError == USB_NO_ERROR)
                    dwUsbError = USB_CLIENT_BUFFER_ERROR;
            }
        }
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
        __try { // setting transfer status and executing callback function
            if (m_sTransfer.lpfComplete !=NULL)
                *m_sTransfer.lpfComplete = TRUE;
            if (m_sTransfer.lpdwError!=NULL)
                *m_sTransfer.lpdwError = dwUsbError;
            if (m_sTransfer.lpdwBytesTransferred)
                *m_sTransfer.lpdwBytesTransferred =  m_DataTransferred;
            if ( m_sTransfer.lpStartAddress ) {
                ( *m_sTransfer.lpStartAddress )(m_sTransfer.lpvNotifyParameter );
                m_sTransfer.lpStartAddress = NULL ; // Make sure only do once.
            }
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
        }
#pragma prefast(pop)
        return (dwUsbError==USB_NO_ERROR);
    }
    else
        return TRUE;

}
CSITransfer::CSITransfer (IN  CIsochronousPipe * const pCPipe,IN CEhcd * const pCEhcd,STransfer sTransfer)
    :CIsochTransfer(pCPipe, pCEhcd,sTransfer)
{
    m_pCSITDList =0;
}
CSITransfer::~CSITransfer()
{
    ASSERT(m_dwSchedTDIndex==m_dwNumOfTD);
    ASSERT(m_dwDequeuedTDIndex ==  m_dwNumOfTD);
    if (m_pCSITDList && m_dwNumOfTD) {
        AbortTransfer();
        for (DWORD dwIndex=0;dwIndex<m_dwNumOfTD;dwIndex++)
            if (*(m_pCSITDList + dwIndex)) {
                ASSERT((*(m_pCSITDList + dwIndex) )->GetLinkValid() != TRUE); // Invalid Next Link
                GetPipe()->FreeCSITD(*(m_pCSITDList + dwIndex));
                *(m_pCSITDList + dwIndex) = NULL;
            }
        delete m_pCSITDList;
    }
}
BOOL CSITransfer::ArmTD()
{
    BOOL bAnyArmed = FALSE;
    if (m_pCSITDList && m_dwArmedTDIndex<m_dwNumOfTD ) { // Something TD wait for Arm.
        DWORD dwCurDataPhysAddr =   m_sTransfer.paBuffer + m_dwArmedBufferIndex ;
        CSITD * pPrevCSITD= (m_dwArmedTDIndex==0?NULL:*(m_pCSITDList + m_dwArmedTDIndex-1));
        while( m_dwArmedTDIndex < m_dwNumOfTD) {
            DWORD dwLength=  *(m_sTransfer.aLengths + m_dwArmedTDIndex);
            *(m_pCSITDList + m_dwArmedTDIndex) = GetPipe()->AllocateCSITD( this,pPrevCSITD);
            if (*(m_pCSITDList + m_dwArmedTDIndex) == NULL) {
                break;
            }
            else {
                pPrevCSITD = *(m_pCSITDList + m_dwArmedTDIndex);
                BOOL bReturn=(*(m_pCSITDList + m_dwArmedTDIndex))->IssueTransfer(dwCurDataPhysAddr,dwCurDataPhysAddr+ dwLength -1, dwLength,
                    TRUE,// Interrupt On Completion
                    (m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0);
                ASSERT(bReturn == TRUE);
                // Interrupt on any CITD completion
                (*(m_pCSITDList+ m_dwArmedTDIndex))->SetIOC(TRUE);
                dwCurDataPhysAddr += dwLength;
                m_dwArmedBufferIndex +=dwLength;
                m_dwArmedTDIndex ++ ;
                bAnyArmed = TRUE;
            }
        }
        
    }
    return bAnyArmed;
}
BOOL CSITransfer::AddTransfer() 
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CSITransfer::AddTransfer (this=0x%x,id=0x%xm_dwFrameIndexStart=%x)\r\n"),this,m_dwTransferID,m_dwFrameIndexStart));
    if (m_dwNumOfTD!=0 || m_pCSITDList != NULL) {
        ASSERT(FALSE);
        return FALSE;
    }
    m_dwNumOfTD = m_sTransfer.dwFrames ;
    m_dwSchedTDIndex=0;
    m_dwArmedTDIndex =0;
    m_dwArmedBufferIndex=0;
    m_dwFirstError =  USB_NO_ERROR;
    if (m_sTransfer.lpvBuffer &&  m_sTransfer.paBuffer && m_sTransfer.dwBufferSize) {
        // Allocate space for CITD List
        m_pCSITDList = (CSITD **)new PVOID[m_dwNumOfTD];
        if (m_pCSITDList!=NULL) {
            memset(m_pCSITDList,0,sizeof(CSITD *)*m_dwNumOfTD);
            ArmTD();
            return TRUE;
        }
    }
    ASSERT(FALSE);
    return FALSE;
}
BOOL CSITransfer::ScheduleTD(DWORD dwCurFrameIndex,DWORD /*dwCurMicroFrameIndex*/)
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CSITransfer::ScheduleTD (this=0x%x,id=0x%x,curFrameIndex=0x%x,m_dwFrameIndexStart=0x%x)\r\n"),this,m_dwTransferID, dwCurFrameIndex,m_dwFrameIndexStart));
    BOOL bReturn = FALSE;
    ArmTD();
    if (m_dwSchedTDIndex < m_dwNumOfTD && m_pCSITDList !=0 && m_pCEhcd) {
        if ((long)(dwCurFrameIndex - m_dwFrameIndexStart) > (long)m_dwSchedTDIndex) {
            m_dwSchedTDIndex = dwCurFrameIndex - m_dwFrameIndexStart;
        }
        m_dwSchedTDIndex = min( m_dwSchedTDIndex , m_dwNumOfTD);
        
        DWORD EndShedTDIndex = dwCurFrameIndex +  (m_pCEhcd->GetPeriodicMgr()).GetFrameSize()-1;
        DWORD dwNumTDCanSched;
        if ( (long)(EndShedTDIndex - m_dwFrameIndexStart ) >= 0)
            dwNumTDCanSched =EndShedTDIndex - m_dwFrameIndexStart ;
        else  // Too Early.
            dwNumTDCanSched = 0;
        dwNumTDCanSched= min(m_dwNumOfTD ,dwNumTDCanSched);        
        dwNumTDCanSched= min(m_dwArmedTDIndex ,dwNumTDCanSched);
        
        if (m_dwSchedTDIndex < dwNumTDCanSched) { // Do scudule those index.
            for (DWORD dwIndex = m_dwSchedTDIndex ; dwIndex<dwNumTDCanSched; dwIndex++) {
                m_pCEhcd->PeriodQueueSITD(*(m_pCSITDList+dwIndex),m_dwFrameIndexStart + dwIndex );
                (*(m_pCSITDList+dwIndex))->CheckStructure ();
                ASSERT((*(m_pCSITDList+dwIndex))->m_pTrans == this);
            }
            m_dwSchedTDIndex = dwNumTDCanSched;
            bReturn = TRUE;
        }
    }
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer::ScheduleTD (this=0x%x) return %d\r\n"),this,bReturn));
    return bReturn;
}
BOOL CSITransfer::IsTransferDone(DWORD dwCurFrameIndex,DWORD /*dwCurMicroFrameIndex*/)
{
    // Dequeue those TD has Transfered.
    m_dwLastFrameIndex = dwCurFrameIndex - m_dwFrameIndexStart;
    if ((long)(dwCurFrameIndex - m_dwFrameIndexStart)>= 1 ) {
        DWORD dwTransfered = min(dwCurFrameIndex - m_dwFrameIndexStart , m_dwSchedTDIndex);
        ASSERT(m_dwSchedTDIndex<=m_dwArmedTDIndex);
        for (DWORD dwIndex=m_dwDequeuedTDIndex;dwIndex<dwTransfered && dwIndex < m_dwNumOfTD ; dwIndex++) {
            if ( *(m_pCSITDList+dwIndex) != NULL && (*(m_pCSITDList + dwIndex) )->GetLinkValid()) {
                (*(m_pCSITDList+dwIndex))->CheckStructure ();
                ASSERT((*(m_pCSITDList+dwIndex))->m_pTrans == this);
                m_pCEhcd->PeriodDeQueueTD((*(m_pCSITDList+dwIndex))->GetPhysAddr(),dwIndex + m_dwFrameIndexStart);
                (*(m_pCSITDList + dwIndex) )->SetLinkValid(FALSE);
                
            }
            if (*(m_pCSITDList + dwIndex)!=NULL) {
                DWORD dwTDError = USB_NO_ERROR;
                (*(m_pCSITDList+dwIndex))->CheckStructure ();
                if ((*(m_pCSITDList +dwIndex))->sITD_TransferState.sITD_TSContext.Active!=0) 
                    dwTDError = USB_NOT_COMPLETE_ERROR;
                else
                if ((*(m_pCSITDList + dwIndex))->sITD_TransferState.sITD_TSContext.XactErr!=0) 
                    dwTDError = USB_ISOCH_ERROR;
                else
                if ((*(m_pCSITDList + dwIndex))->sITD_TransferState.sITD_TSContext.BabbleDetected!=0) 
                    dwTDError = USB_STALL_ERROR;
                else
                if ((*(m_pCSITDList + dwIndex))->sITD_TransferState.sITD_TSContext.DataBufferError!=0) 
                    dwTDError = ((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0?USB_DATA_OVERRUN_ERROR:USB_DATA_UNDERRUN_ERROR);
                else 
                if (((*(m_pCSITDList + dwIndex))->sITD_TransferState.dwSITD_TransferState & 0xff)!=0 )
                    dwTDError = USB_BIT_STUFFING_ERROR;
                
                if (m_dwFirstError == USB_NO_ERROR   ) { // only update first time
                    m_dwFirstError = dwTDError;
                }
                if (dwTDError!= USB_NO_ERROR) {
                    DEBUGMSG( ZONE_TRANSFER , (TEXT("CITransfer::DoneTransfer (this=0x%x, dwFrameIndex=%d) Error(%d) \r\n"),
                        this,dwIndex, dwTDError));
                }
                if (dwIndex< m_sTransfer.dwFrames) {
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
                    __try { // setting isoch OUT status parameters
                        m_sTransfer.adwIsochErrors[ dwIndex] = dwTDError;
                        m_sTransfer.adwIsochLengths[ dwIndex ] = *(m_sTransfer.aLengths + dwIndex) - (*(m_pCSITDList + dwIndex))->sITD_TransferState.sITD_TSContext.BytesToTransfer;
                        m_DataTransferred += m_sTransfer.adwIsochLengths[ dwIndex ];
                    } __except( EXCEPTION_EXECUTE_HANDLER ) {
                    }
#pragma prefast(pop)
                }

                GetPipe()->FreeCSITD(*(m_pCSITDList + dwIndex));
                *(m_pCSITDList + dwIndex) = NULL;
            }
        }
        m_dwDequeuedTDIndex = dwIndex;
    }
    BOOL bReturn= ((long)(dwCurFrameIndex-m_dwFrameIndexStart)>=(long)m_dwNumOfTD);
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer:::IsTransferDone (this=0x%x,curFrameIndex=0x%x) return %d \r\n"),this, dwCurFrameIndex,bReturn));
    return bReturn;
}
BOOL CSITransfer::AbortTransfer()
{
    for (DWORD dwIndex=m_dwDequeuedTDIndex;dwIndex < m_dwNumOfTD && dwIndex<m_dwSchedTDIndex ; dwIndex++) {
        if ( *(m_pCSITDList+dwIndex) != NULL ) {
            BOOL bResult =m_pCEhcd->PeriodDeQueueTD((*(m_pCSITDList+dwIndex))->GetPhysAddr(),dwIndex + m_dwFrameIndexStart);
            ASSERT(bResult == TRUE);
            (*(m_pCSITDList + dwIndex) )->SetLinkValid(FALSE);
            
            if (dwIndex< m_sTransfer.dwFrames) {
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
                __try { // setting isoch OUT status parameters
                    m_sTransfer.adwIsochErrors[ dwIndex] = USB_NOT_COMPLETE_ERROR;
                    m_sTransfer.adwIsochLengths[ dwIndex ] = 0;
                } __except( EXCEPTION_EXECUTE_HANDLER ) {
                }
#pragma prefast(pop)
            }
            if (m_dwFirstError == USB_NO_ERROR   ) { // only update first time
                m_dwFirstError = USB_NOT_COMPLETE_ERROR;
            }
            GetPipe()->FreeCSITD(*(m_pCSITDList + dwIndex));
            *(m_pCSITDList + dwIndex) = NULL;
        }
    }
    m_dwArmedTDIndex = m_dwDequeuedTDIndex = m_dwSchedTDIndex = m_dwNumOfTD;
    Sleep(2); // Make Sure EHCI nolong reference to those TD;
    return DoneTransfer(m_dwFrameIndexStart+m_dwNumOfTD, 0);
};
BOOL CSITransfer::DoneTransfer(DWORD dwCurFrameIndex,DWORD dwCurMicroFrameIndex)
{
    BOOL bIsTransDone = IsTransferDone(dwCurFrameIndex, dwCurMicroFrameIndex);
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer::DoneTransfer (this=0x%x,id=0x%x,curFrameIndex=0x%x, bIsTransDone=%d \r\n")
        ,this,m_dwTransferID, dwCurFrameIndex,bIsTransDone));
    ASSERT(bIsTransDone == TRUE);
    if (bIsTransDone && !m_fDoneTransferCalled) {
        m_fDoneTransferCalled = TRUE;
        // We have to update the buffer when this is IN Transfer.
        if ((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=NULL && m_pAllocatedForClient!=NULL) {
            __try { // copying client buffer for OUT transfer
                memcpy( m_sTransfer.lpvBuffer, m_pAllocatedForClient, m_sTransfer.dwBufferSize );
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                  // bad lpvBuffer.
                if (m_dwFirstError == USB_NO_ERROR)
                    m_dwFirstError = USB_CLIENT_BUFFER_ERROR;
            }
        }
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
        __try { // setting transfer status and executing callback function
            if (m_sTransfer.lpfComplete !=NULL)
                *m_sTransfer.lpfComplete = TRUE;
            if (m_sTransfer.lpdwError!=NULL)
                *m_sTransfer.lpdwError = m_dwFirstError;
            if (m_sTransfer.lpdwBytesTransferred)
                *m_sTransfer.lpdwBytesTransferred =  m_DataTransferred;
            if ( m_sTransfer.lpStartAddress ) {
                ( *m_sTransfer.lpStartAddress )(m_sTransfer.lpvNotifyParameter );
                m_sTransfer.lpStartAddress = NULL ; // Make sure only do once.
            }
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
        }
#pragma prefast(pop)
        return (m_dwFirstError==USB_NO_ERROR);
    }
    else
        return TRUE;

}



