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
//     Trans.cpp
// 
// Abstract: Provides interface to UHCI host controller
// 
// Notes: 
//

#pragma warning(push)
#pragma warning(disable : 4510 4512 4610)
#include <windows.h>
#include <Cphysmem.hpp>
#include "trans.h"
#include "cpipe.h"
#include "chw.h"
#include "cmhcd.h"
#pragma warning(pop)

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

#ifndef _PREFAST_
#pragma warning(disable: 4068) // Disable pragma warnings
#endif

#undef ZONE_DEBUG
#define ZONE_DEBUG 0

DWORD CTransfer::m_dwGlobalTransferID=0;

extern "C" void 
memdodump(unsigned char * data,     unsigned short num_bytes, unsigned short offset    )
{    
    unsigned short i,j,l;
    unsigned char tmp_str[100]; 
    unsigned char tmp_str1[10];    
    for (i = 0; i < num_bytes; i += 16)    {        
        unsigned short n ;      
        tmp_str[0]='\0';        
        n = i+offset ;        
        for (j=0; j<4; j++) {
            l=n%16;
            if (l>=10)              
                tmp_str[3-j]=(unsigned char)('A'+l-10);         
            else                
                tmp_str[3-j]=(unsigned char)(l+'0');            
            n >>= 4 ;       
        }       
        tmp_str[4]='\0';        
        strcat ( (char *)tmp_str, ": ");        
        /*          Output the hex bytes        */        
        for (j = i; j < (i+16); j++) {          
            int m ;            
            if (j < num_bytes)  {  
                m=((unsigned int)((unsigned char)*(data+j)))/16 ;               
                if (m>=10)                  
                    tmp_str1[0]='A'+(unsigned char)m-10;                
                else                    
                    tmp_str1[0]=(unsigned char)m+'0';               
                m=((unsigned int)((unsigned char)*(data+j)))%16 ;               
                if (m>=10)                  
                    tmp_str1[1]='A'+(unsigned char)m-10;                
                else                    
                    tmp_str1[1]=(unsigned char)m+'0';               
                tmp_str1[2]='\0';               
                strcat ((char *)tmp_str, (char *)tmp_str1);             
                strcat ((char *)tmp_str, " ");            
            }            
            else {                
                strcat((char *)tmp_str,"   ");            
            }        
        }        
        strcat((char *)tmp_str, "  ");      
        l=(unsigned short)strlen((char *)tmp_str);        

        /*         * Output the ASCII bytes        */        
        for (j = i; j < (i+16); j++){            
            if (j < num_bytes){
                char c = *(data+j);                
                if (c < ' ' || c > 'z') 
                    c = '.';                
                tmp_str[l++]=c;            
            }            
            else            
                tmp_str[l++]=' ';
        }       
        tmp_str[l++]='\r';        tmp_str[l++]='\n';        tmp_str[l++]='\0';     
        RETAILMSG(1, (L"%S", tmp_str));    
    }
}

//************************************************************************************************
CTransfer::CTransfer(IN CPipe * const pCPipe, IN CPhysMem * const pCPhysMem,STransfer sTransfer) 
    : m_sTransfer( sTransfer)
    , m_pCPipe(pCPipe)
    , m_pCPhysMem(pCPhysMem)
//
//  Purpose: CTransfer Constructor
//
//  Parameter:  pCPipe - Pointer to the pipe class being used
//              pCPhyMem - Pointer to physical memory class
//              sTransfer - Transfer object passing from above.
//
//*************************************************************************************************
{
    m_pNextTransfer=NULL;
    m_paControlHeader=0;
    m_pAllocatedForControl=NULL;
    m_pAllocatedForClient=NULL;
    memcpy(&m_sTransfer, &sTransfer,sizeof(STransfer));
    m_DataTransferred =0 ;
    m_dwTransferID = m_dwGlobalTransferID++;
}

//**************************************************************************************************
CTransfer::~CTransfer()
//
//  Purpose: Destructor for CTransfer
//
//**************************************************************************************************
{    
    if (m_pAllocatedForControl!=NULL) 
        m_pCPhysMem->FreeMemory( PUCHAR(m_pAllocatedForControl),m_paControlHeader,  CPHYSMEM_FLAG_NOBLOCK );
    if (m_pAllocatedForClient!=NULL)
        m_pCPhysMem->FreeMemory( PUCHAR(m_pAllocatedForClient), m_sTransfer.paBuffer,  CPHYSMEM_FLAG_NOBLOCK );
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE , (TEXT("CTransfer::~CTransfer() (this=0x%x,m_pAllocatedForControl=0x%x,m_pAllocatedForClient=0x%x)\r\n"),
        this,m_pAllocatedForControl,m_pAllocatedForClient));

}
//***************************************************************************************************
BOOL CTransfer::Init(void)
//
//  Purpose: Initialize of the CTransfer object. This is called when it is to do the transfer. It allocates
//  buffer and put the data into CPhysMem, create the link-list and then send out.
//
//  Parameter: Nil
//
//  Return: TRUE - success, FALSE - failure
//
//******************************************************************************************************
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
#if 0
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
#endif

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
                if ((m_sTransfer.dwBufferSize == 31) && (*m_pAllocatedForClient != 0x55))
                    RETAILMSG(1, (TEXT("Invalid Transfer OUT\r\n")));
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                  // bad lpvClientBuffer
                  return FALSE;
            }
        }
    }
    
    DEBUGMSG(  ZONE_TRANSFER && ZONE_VERBOSE , (TEXT("CQTransfer::Init (this=0x%x,id=0x%x),m_pAllocatedForControl=0x%x,m_pAllocatedForClient=0x%x\r\n"),
        this,m_dwTransferID,m_pAllocatedForControl,m_pAllocatedForClient));    
    return AddTransfer();
}

//****************************************************************************************************
CQTransfer::~CQTransfer()
//
//  Purpose:    Destructor of CQTransfer
//
//  Parameter:  Nil
//
//  Return: Nil
//
//****************************************************************************************************
{    
    CQTD * pCurTD = m_pCQTDList;
    while (pCurTD!=NULL) {
         CQTD * pNextTD = pCurTD->GetNextTD();
         pCurTD->~CQTD();         
         delete pCurTD;
         pCurTD = pNextTD;
    }
}

//*****************************************************************************************************
BOOL CQTransfer::AddTransfer() 
//
//  Purpose: Create the CQTransfer List to be transferred to the client side
//           On completion the m_pCQTDList should contain list of TD to be transferred.
//
//  Parameter: Nil
//
//  Return: Nil
//
//******************************************************************************************************
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer::AddTransfer (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    if (m_pCQTDList) { // Has been created. Somthing wrong.
        DEBUGMSG(ZONE_ERROR, (TEXT("CQTransfer:: Failure to add since m_pCQTDList is full\r\n")));
        ASSERT(FALSE);
        return FALSE;
    }
    BOOL bDataToggle1= FALSE;
    CQTD * pStatusTD = NULL;
    DWORD dwPacketSize= (m_pCPipe->GetEndptDescriptor()).wMaxPacketSize & 0x7ff;
    if (m_paControlHeader!=NULL && m_sTransfer.lpvControlHeader!=NULL) { 
        // This is setup packet.        
        if (m_pCQTDList = new CQTD(this, (CQH *)(((CQueuedPipe * const)m_pCPipe->GetQHead())))) {            
            m_pCPipe->GetQHead()->Lock();
            m_pCQTDList->SetBuffer((DWORD)m_paControlHeader, (DWORD)m_pAllocatedForControl, sizeof(USB_DEVICE_REQUEST));
            m_pCQTDList->SetTotTfrSize(0);
            m_pCQTDList->SetCurTfrSize(0);
            m_pCQTDList->SetTDType(TD_SETUP);
            m_pCQTDList->SetToggle(bDataToggle1);
            m_pCQTDList->SetStatus(STATUS_WAIT_TRANSFER);            
            m_pCQTDList->SetPacketSize(dwPacketSize);
            m_pCPipe->GetQHead()->UnLock();
            bDataToggle1 = !bDataToggle1;
        }
        else 
            return FALSE;

        // Status Packet        
        pStatusTD = new CQTD(this, ((CQueuedPipe * const)m_pCPipe)->GetQHead());
        if (pStatusTD) {
            m_pCPipe->GetQHead()->Lock();
            pStatusTD->SetBuffer(0, 0, 0);
            pStatusTD->SetTotTfrSize(0);
            pStatusTD->SetCurTfrSize(0);
            pStatusTD->SetTDType(((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0?TD_STATUS_OUT:TD_STATUS_IN));
            pStatusTD->SetToggle(TRUE);
            pStatusTD->SetStatus(STATUS_WAIT_TRANSFER);
            pStatusTD->SetPacketSize(dwPacketSize);
            m_pCPipe->GetQHead()->UnLock();
        }
        else 
            return FALSE;
    };
    CQTD * pPrevTD=m_pCQTDList;
    if ( m_pCQTDList==NULL  && m_sTransfer.dwBufferSize == 0 ) { // No Control but Zero Length
        //ASSERT((m_sTransfer.dwFlags & USB_IN_TRANSFER)==0);// No meaning for IN Zero length packet.        
        CQTD * pCurTD = new CQTD(this, ((CQueuedPipe * const)m_pCPipe)->GetQHead());
        if (pCurTD) {           
            m_pCPipe->GetQHead()->Lock();
            pCurTD->SetBuffer(0, 0, 0);
            pCurTD->SetTotTfrSize(0);
            pCurTD->SetCurTfrSize(0);
            pCurTD->SetTDType(((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0? TD_DATA_IN: TD_DATA_OUT));
            pCurTD->SetToggle(bDataToggle1);
            pCurTD->SetStatus(STATUS_WAIT_TRANSFER);
            pCurTD->SetPacketSize(dwPacketSize);
            if (pPrevTD) {
                pPrevTD->QueueNextTD(pCurTD);
                pPrevTD=pCurTD;
            }
            else { // THis is First. So update m_pQTDList
                pPrevTD= m_pCQTDList = pCurTD;
            }
            m_pCPipe->GetQHead()->UnLock();
        }
        else {
            if ( pStatusTD) {
                pStatusTD->~CQTD();
                delete pStatusTD;
            }
            return FALSE;                
        }
    }
    else
    if (m_sTransfer.lpvBuffer &&  m_sTransfer.paBuffer && m_sTransfer.dwBufferSize) {                
        BOOL  bZeroLength = FALSE;        
        
        // If this is an OUT transaction and total length is multiple of packet size
        if (((m_sTransfer.dwFlags & USB_IN_TRANSFER)==0) && (m_sTransfer.dwBufferSize%dwPacketSize==0))
            bZeroLength = TRUE;

        // We just setup one single CQTD here, let the WriteFIFO handle that.                
        CQTD * pCurTD = new CQTD(this, ((CQueuedPipe * const)m_pCPipe)->GetQHead());
        ASSERT(pCurTD!=NULL);
        if (pCurTD==NULL) {
            // delete  pStatusTD;
            if ( pStatusTD) {
                pStatusTD->~CQTD();
                delete pStatusTD;
            }
            return FALSE;                
        }                
        DWORD dwCurPhysAddr=  m_sTransfer.paBuffer;
        DWORD dwCurVirtAddr = (m_pAllocatedForClient != NULL)?(DWORD)m_pAllocatedForClient:(DWORD)m_sTransfer.lpvBuffer;
        m_pCPipe->GetQHead()->Lock();
        pCurTD->SetBuffer(dwCurPhysAddr, dwCurVirtAddr, m_sTransfer.dwBufferSize);
        pCurTD->SetTotTfrSize(0);
        pCurTD->SetCurTfrSize(0);
        pCurTD->SetTDType(((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0?TD_DATA_IN:TD_DATA_OUT));
        pCurTD->SetToggle(bDataToggle1);
        pCurTD->SetStatus(STATUS_WAIT_TRANSFER);
        pCurTD->SetPacketSize(dwPacketSize);
        if (pPrevTD) {
            pPrevTD->QueueNextTD(pCurTD);
            pPrevTD=pCurTD;
        }
        else { // THis is First. So update m_pQTDList
            pPrevTD= m_pCQTDList = pCurTD;
        }
        m_pCPipe->GetQHead()->UnLock();

    } // end of else if 
    // We have to append Status TD here.
    if (pStatusTD) { // This is setup packet.
        m_pCPipe->GetQHead()->Lock();
        if (pPrevTD) {
            pPrevTD->QueueNextTD(pStatusTD);
            pPrevTD=pStatusTD;
        }
        else { // Something Wrong.
            ASSERT(FALSE);
            //delete pCurTD;
            pStatusTD->~CQTD();
            m_pCPipe->GetQHead()->UnLock();
            delete pStatusTD;
            return FALSE;
        }
        m_pCPipe->GetQHead()->UnLock();
    };
    return TRUE;    
}

//***************************************************************************************************************
BOOL CQTransfer::DoneTransfer()
//
//  Purpose: To notify back to upper layer on the completion and status of the transfer
//
//  Parameter: Nil
//
//  Return: TRUE - complete, FALSE - not yet
//
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer::DoneTransfer (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    BOOL bIsTransDone = IsTransferDone();
    ASSERT(bIsTransDone == TRUE);
    if (bIsTransDone) {
        DWORD dwUsbError = USB_NO_ERROR;
        DWORD dwDataTransferred = 0;        
        CQTD * pCurTD = m_pCQTDList;
        m_pCPipe->GetQHead()->Lock();
        while ( pCurTD!=NULL) {
            DWORD dwStatus = pCurTD->GetStatus();
            DWORD dwTDType = pCurTD->GetTDType();

            if (dwTDType != TD_SETUP ) // Do not count Setup TD
                dwDataTransferred += pCurTD->m_cbTransferred;
            if (dwStatus == STATUS_HALT) { // This Transfer Has been halted due to error.
                // This is error. We do not have error code for MHCI so generically we set STALL error.
                if (dwUsbError == USB_NO_ERROR)
                    dwUsbError = USB_STALL_ERROR;
            }
            else if (dwStatus == STATUS_ABORT) {
                if (dwUsbError == USB_NO_ERROR)
                    dwUsbError = USB_CANCELED_ERROR;
            }
            else if (dwStatus == STATUS_NOT_ENOUGH_BUFFER) {
                if (dwUsbError == USB_NO_ERROR)
                    dwUsbError = USB_CLIENT_BUFFER_ERROR;
            }
            else
            if ((dwStatus == STATUS_WAIT_TRANSFER) || (dwStatus == STATUS_WAIT_RESPONSE)) {
                if (dwUsbError == USB_NO_ERROR)
                    dwUsbError = USB_NOT_COMPLETE_ERROR;
                break;
            }
            pCurTD = pCurTD->GetNextTD();
        }        
        
        UCHAR ep = m_pCPipe->GetMappedEndPoint();
        UCHAR isIn = (m_sTransfer.dwFlags & USB_IN_TRANSFER)? 1:0;
        DWORD dwPacketSize= (m_pCPipe->GetEndptDescriptor()).wMaxPacketSize & 0x7ff;
        if (ep == 0)
            m_pCPipe->m_pCMhcd->UnlockEP0(m_pCPipe->GetReservedDeviceAddr());
        else 
        {
            // Set Data Toggle here after the current transaction has been completed
            UCHAR dataToggle = m_pCPipe->m_pCMhcd->GetCurrentToggleBit(m_pCPipe->GetMappedEndPoint(), isIn);            
            if (dataToggle != 0xff)
                m_pCPipe->SetDataToggle(dataToggle);          
            // Release the endpoint only if the transfer size is smaller than the max packet size;
            {
                UCHAR channel = m_pCPipe->m_pCMhcd->DeviceInfo2Channel(m_pCPipe);                
                m_pCPipe->m_pCMhcd->ReleaseDMAChannel(m_pCPipe, channel);
                m_pCPipe->m_pCMhcd->ReleasePhysicalEndPoint(m_pCPipe, TRUE, FALSE);

                // We release it only when it is fully complete. Otherwise, there may be problem.
                if ((dwUsbError == USB_NO_ERROR) && (dwDataTransferred < dwPacketSize))
                {   
                    DEBUGMSG(ZONE_VERBOSE,(TEXT("CQTransfer::DoneTransfer ReleasePhysicalEndPoint and ReleaseDMAChannel:%d\r\n"),channel));
                }
            }
        }

        m_pCPipe->GetQHead()->UnLock();

        // We have to update the buffer when this is IN Transfer.
        if ((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=NULL && m_pAllocatedForClient!=NULL && m_sTransfer.dwBufferSize!=0) {
            __try { // copying client buffer for OUT transfer
                memcpy( m_sTransfer.lpvBuffer, m_pAllocatedForClient, m_sTransfer.dwBufferSize );
                //RETAILMSG(1, (TEXT("Receive Buffer size %d\r\n"), m_sTransfer.dwBufferSize));
                //memdump((UCHAR *)m_sTransfer.lpvBuffer, (USHORT)m_sTransfer.dwBufferSize, 0);
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                  // bad lpvBuffer.
                if (dwUsbError == USB_NO_ERROR)
                    dwUsbError = USB_CLIENT_BUFFER_ERROR;
            }
        }
        __try { // setting transfer status and executing callback function
            if (m_sTransfer.lpfComplete !=NULL)
                *m_sTransfer.lpfComplete = TRUE;
            if (m_sTransfer.lpdwError!=NULL)
                *m_sTransfer.lpdwError = dwUsbError;
            if (m_sTransfer.lpdwBytesTransferred)
                *m_sTransfer.lpdwBytesTransferred =  dwDataTransferred;
            if ( m_sTransfer.lpStartAddress ) {
                //UCHAR index;
                //if ((m_sTransfer.dwFlags & USB_IN_TRANSFER) && (dwDataTransferred == 13))
                //    index = *((UCHAR *)(m_sTransfer.lpvBuffer)+4);                

                DEBUGMSG(ZONE_HCD, (TEXT("Notify ep(0x%x) (%s) dev %d above with error(0x%x), dwDataTransferred(%d), size(%d)\r\n"), 
                    ep, (m_sTransfer.dwFlags & USB_IN_TRANSFER)? TEXT("IN"):TEXT("OUT"), m_pCPipe->GetReservedDeviceAddr(), dwUsbError, dwDataTransferred, m_sTransfer.dwBufferSize));
                memdump((UCHAR *)m_sTransfer.lpvBuffer, (USHORT)dwDataTransferred, 0);
                ( *m_sTransfer.lpStartAddress )(m_sTransfer.lpvNotifyParameter );
                m_sTransfer.lpStartAddress = NULL ; // Make sure only do once.
            }
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
              DEBUGMSG( ZONE_ERROR, (TEXT("CQueuedPipe(%s)::CheckForDoneTransfers - exception setting transfer status to complete\n"), m_pCPipe->GetPipeType() ) );
        }        

        return (dwUsbError==USB_NO_ERROR);
    }
    else
        return TRUE;
}

//***********************************************************************************************************
BOOL CQTransfer::AbortTransfer()
//
//  Purpose: Abort the current transfer
//
//  Parameter:  Nil
//
//  Return: TRUE
//
{
    DEBUGMSG(ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer::AbortTransfer (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    CQTD * pCurTD = m_pCQTDList;

    m_pCPipe->GetQHead()->Lock();
    while ( pCurTD!=NULL) {
        DEBUGMSG(ZONE_TRANSFER, (TEXT("AbortTransfer for pCurTD 0x%x\r\n"), pCurTD));
        pCurTD->DeActiveTD();
        pCurTD = pCurTD->GetNextTD();
    }
    m_pCPipe->GetQHead()->UnLock();

    Sleep(2);// Make sure the shcdule has advanced. and current Transfer has completeded.
    return TRUE;
}

//***********************************************************************************************
DWORD CQTransfer::ProcessResponse(DWORD dwReason, CQTD *pCurTD)
//
//  Purpose: Process the response coming from interrupt
//
//  Parameter: dwReason - Reason for calling
//             pCurTD - current process TD
//
//  Return: error code
{
    DWORD dwReturn = USB_NO_ERROR;
    if (dwReason == STATUS_PROCESS_INTR)
    {
        // In case of TD_DATA_IN transaction, we have to 
        // read the data from FIFO.
        DWORD dwType = pCurTD->GetTDType();        
        switch (dwType) {
            case TD_DATA_IN:                
                {
                    DWORD dwMore = MUSB_READ_SUCCESS;
                    // Read the data into the pCurTD.                    
                    dwReturn = pCurTD->ReadDataIN(&dwMore);

                    if (dwReturn == USB_NO_ERROR)
                    {
                        if (dwMore == MUSB_READ_MORE_DATA)
                            pCurTD->SetStatus(STATUS_WAIT_MORE_DATA);
                        else if (dwMore == MUSB_READ_SUCCESS)
                        {
                            if (pCurTD->GetError() & ERR_BUFFER_ERROR)                                                           
                                pCurTD->SetStatus(STATUS_NOT_ENOUGH_BUFFER);
                            else
                                pCurTD->SetStatus(STATUS_COMPLETE);
                        }
                        else if (dwMore == MUSB_WAIT_DMA_COMPLETE)
                            DEBUGMSG(ZONE_HCD, (TEXT("Wait DMA Complete\r\n")));
                    }
                    else if (dwReturn == USB_STALL_ERROR)
                    {
                        DEBUGMSG(ZONE_ERROR, (TEXT("Endpoint halt\r\n")));
                        pCurTD->SetStatus(STATUS_HALT);
                    }
                    else if (dwReturn == USB_DEVICE_NOT_RESPONDING_ERROR)
                    {
                        DEBUGMSG(ZONE_ERROR, (TEXT("Device not responding\r\n")));
                        pCurTD->SetStatus(STATUS_ERROR);
                    }
                    else
                    {
                        DEBUGMSG(ZONE_TRANSFER, (TEXT("ReadDataIn return 0x%x\r\n"), dwReturn));
                        pCurTD->SetStatus(STATUS_WAIT_RESPONSE);
                    }
                }                
                break;

            case TD_STATUS_OUT:
            case TD_SETUP:                    
                {
                    // Need to check the result here
                    dwReturn = m_pCPipe->m_pCMhcd->CheckTxCSR(m_pCPipe->GetMappedEndPoint());
                    if (dwReturn == USB_NO_ERROR)
                    {
                        pCurTD->SetTotTfrSize(pCurTD->GetCurTfrSize() + pCurTD->GetTotTfrSize());
                        pCurTD->SetStatus(STATUS_COMPLETE);                    
                    }
                    else if (dwReturn == USB_STALL_ERROR)
                    {
                        RETAILMSG(1, (TEXT("Tx Stall at EP %d\r\n"), m_pCPipe->GetMappedEndPoint()));
                        pCurTD->SetStatus(STATUS_HALT);
                    }
                    else if (dwReturn == USB_DEVICE_NOT_RESPONDING_ERROR)
                    {
                        DEBUGMSG(ZONE_ERROR, (TEXT("Device not responding\r\n")));
                        pCurTD->SetStatus(STATUS_ERROR);
                    }
                }
                break;

            case TD_DATA_OUT:
                {
                    // Need to check the result here
                    dwReturn = m_pCPipe->m_pCMhcd->CheckTxCSR(m_pCPipe->GetMappedEndPoint());
                    if (dwReturn == USB_NO_ERROR)
                    {
                        DWORD dwTfrSize = pCurTD->GetCurTfrSize();                    
                        pCurTD->SetTotTfrSize(pCurTD->GetCurTfrSize() + pCurTD->GetTotTfrSize());
                        if ((dwTfrSize%pCurTD->GetPacketSize() != 0)  || 
                                (pCurTD->GetTotTfrSize() >= pCurTD->GetDataSize()))                                               
                            pCurTD->SetStatus(STATUS_COMPLETE);
                        else
                        {
                            pCurTD->SetStatus(STATUS_SEND_MORE_DATA);
                        }
                        DEBUGMSG(ZONE_TRANSFER, (TEXT("ACK Data OUT ep %d, size %d, status %d\r\n"),
                                m_pCPipe->GetMappedEndPoint(), dwTfrSize, pCurTD->GetStatus()));
                    }
                    else if (dwReturn == USB_STALL_ERROR)
                    {
                        RETAILMSG(1, (TEXT("Tx Stall at EP %d\r\n"), m_pCPipe->GetMappedEndPoint()));
                        pCurTD->SetStatus(STATUS_HALT);
                    }
                    else if (dwReturn == USB_DEVICE_NOT_RESPONDING_ERROR)
                    {
                        DEBUGMSG(ZONE_ERROR, (TEXT("Device not responding\r\n")));
                        pCurTD->SetStatus(STATUS_ERROR);
                    }
                }
                break;

            case TD_STATUS_IN:                    
                {
                    dwReturn = m_pCPipe->m_pCMhcd->CheckRxCSR(m_pCPipe->GetMappedEndPoint());
                    pCurTD->ClearRxPktRdy();
                    if (dwReturn == USB_NO_ERROR)
                        pCurTD->SetStatus(STATUS_COMPLETE);                    
                    else if (dwReturn == USB_STALL_ERROR)
                        pCurTD->SetStatus(STATUS_HALT);
                    else
                        pCurTD->SetStatus(STATUS_ERROR);
                }
                break;
            default:
                break;
            }
    }
    else // Reason for call
        pCurTD->SetStatus(STATUS_COMPLETE); // Jeffrey: need to check here            

    return dwReturn;
}
//************************************************************************************************
BOOL CQTransfer::ProcessEP(DWORD dwReason, BOOL isDMAIntr)
//
//  Purpose: Process the request and then set the status accordingly.
//
//  Parameter: dwReason - Reason for calling
//
//  Return: TRUE: success, FALSE: failure
//
{
    if (m_pCQTDList==NULL) { // Has been created. Somthing wrong.
        DEBUGMSG(ZONE_ERROR, (TEXT("CQTransfer::ProcessEP:m_pCQTDList == NULL\r\n")));
        return TRUE;
    }
    CQTD * pCurTD = m_pCQTDList;
    DWORD dwReturn = USB_NO_ERROR;    
    m_pCPipe->GetQHead()->Lock();
    // Find back the current working one.
    while (pCurTD!=NULL) {
        ASSERT(pCurTD->m_pTrans == this);
        DWORD dwStatus = pCurTD->GetStatus();
        // Condition:
        // (1) Status = STATUS_WAIT_RESPONSE & Interrupt NOT from DMA 
        // (2) Status = STATUS_WAIT_DMA_0_RD_FIFO_COMPLETE & Interrupt from DMA
        // (3) Status = STATUS_WAIT_DMA_1_RD_FIFO_COMPLETE & Interrupt from DMA
        // (4) Status = STATUS_WAIT_DMA_WR_2_FIFO_TXPKT_RDY & Interrupt from DMA
        DEBUGMSG(ZONE_TRANSFER, (TEXT("CQTransfer::ProcessEP status (0x%x), isDMAIntr(%d)\r\n"),
            dwStatus, isDMAIntr));
        if (((dwStatus == STATUS_WAIT_RESPONSE) && (isDMAIntr == FALSE)) ||
            (((dwStatus == STATUS_WAIT_DMA_0_RD_FIFO_COMPLETE)|| (dwStatus == STATUS_WAIT_DMA_1_RD_FIFO_COMPLETE)) &&
                (isDMAIntr == TRUE)) ||                
            ((dwStatus == STATUS_WAIT_DMA_WR_2_FIFO_TXPKT_RDY) && (isDMAIntr == TRUE)))
        {
            dwReturn = ProcessResponse(dwReason, pCurTD);
            break;
        }

        if (((dwStatus == STATUS_WAIT_DMA_WR_RESPONSE)||(dwStatus == STATUS_WAIT_DMA_WR_2_FIFO_TXPKT_NOTRDY)) && isDMAIntr)
        {                
            m_pCPipe->m_pCMhcd->SendOutDMA(m_pCPipe->GetMappedEndPoint(),pCurTD);
            break;
        }

        // Abnormal receive of EP interrupt
        if ((dwStatus == STATUS_WAIT_DMA_1_RD_FIFO_COMPLETE) && (isDMAIntr == 0))
        {
            RETAILMSG(1, (TEXT("This is to read the remaining data by dumping the value\r\n")));
            m_pCPipe->m_pCMhcd->DumpRxCSR(m_pCPipe->GetMappedEndPoint());
            RETAILMSG(1, (TEXT("Debug here\r\n")));
        }
        pCurTD = pCurTD ->GetNextTD();
    }
    m_pCPipe->GetQHead()->UnLock();

    //DEBUGMSG(ZONE_HCD, (TEXT("CQTransfer:ProcessEP return %s\r\n"), ((dwReturn == USB_NO_ERROR)?TEXT("TRUE"):TEXT("FALSE"))));
    return ((dwReturn == USB_NO_ERROR)? TRUE: FALSE);
}
//************************************************************************************************
BOOL CQTransfer::IsTransferDone()
//
//  Purpose: Check if the CQTDs in the m_pCQTDList has been completed
//
//  Parameter:  Nil
//
//  Return: TRUE - complete, FALSE - not yet
//
{
    DEBUGMSG(ZONE_TRANSFER, (TEXT("CQTransfer::IsTransferDone (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    if (m_pCQTDList==NULL) { // Has been created. Somthing wrong.
        return TRUE;
    }
    CQTD * pCurTD = m_pCQTDList;
    BOOL bReturn=TRUE;

    DEBUGMSG(ZONE_TRANSFER, (TEXT("IsTransferDone:pCurTD = 0x%x\r\n"),pCurTD));
    m_pCPipe->GetQHead()->Lock();
    while ( pCurTD!=NULL) {
        ASSERT(pCurTD->m_pTrans == this);
        DWORD dwStatus = pCurTD->GetStatus();
        DEBUGMSG(ZONE_TRANSFER, (TEXT("pCurTD(0x%x) return status = %d\r\n"), pCurTD, dwStatus));
        if (dwStatus == STATUS_HALT || dwStatus == STATUS_ABORT || dwStatus == STATUS_ERROR) { // This Transfer Has been halted due to error.            
            DEBUGMSG(ZONE_TRANSFER, (TEXT("IsTransferDone with error = %d\r\n"), dwStatus));
            break;
        }
        if (dwStatus == STATUS_WAIT_TRANSFER || ((dwStatus > STATUS_ACTIVE_START) && (dwStatus < STATUS_ACTIVE_END))) { 
            bReturn = FALSE;
            break;
        }
        pCurTD = pCurTD ->GetNextTD();
    }
    m_pCPipe->GetQHead()->UnLock();

    DEBUGMSG(ZONE_TRANSFER, (TEXT("CQTransfer::IsTransferDone (this=0x%x) return %d \r\n"),this,bReturn));
    return bReturn;
}

