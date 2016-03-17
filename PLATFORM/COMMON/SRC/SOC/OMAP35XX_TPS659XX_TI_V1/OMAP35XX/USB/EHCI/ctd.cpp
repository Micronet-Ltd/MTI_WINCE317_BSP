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
//     CTd.cpp
// 
// Abstract: Provides interface to UHCI host controller
// 
// Notes: 
//
#include <windows.h>
#include "ctd.h"
#include "trans.h"
#include "cpipe.h"
void * CNextLinkPointer::operator new(size_t stSize, CPhysMem * const pCPhysMem)
{
    PVOID pVirtAddr=0;
    PVOID pReturnLink=NULL;
    if (stSize<sizeof(PVOID))
        stSize = sizeof(PVOID);
    if (pCPhysMem && stSize ) {
        while (pVirtAddr==NULL) {
            if (!pCPhysMem->AllocateMemory( DEBUG_PARAM( TEXT("CNextLinkPointer")) stSize, (PUCHAR *)&pVirtAddr,CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK)) {
                pVirtAddr=NULL;
                break;
            }
            else {
                // Structure can not span 4k Page bound. refer EHCI 3. Note.
                DWORD dwPhysAddr =  pCPhysMem->VaToPa((PUCHAR)pVirtAddr);
                if ((dwPhysAddr & 0xfffff000)!= ((dwPhysAddr + stSize-1) & 0xfffff000)) {
                    // Cross Page bound. trash it.
                    pVirtAddr= NULL;
                };
            }
        }
    }
    return pVirtAddr;
}
void CNextLinkPointer::operator delete(void * /*pointer*/)
{
    ASSERT(FALSE); // Can not use this operator.
}

CITD::CITD(CITransfer * pIsochTransfer)
: m_pTrans(pIsochTransfer)
, m_CheckFlag(CITD_CHECK_FLAG_VALUE)
{
    ASSERT( (&(nextLinkPointer.dwLinkPointer))+ 15 == (&(iTD_BufferPagePointer[6].dwITD_BufferPagePointer)));// Check for Data Intergraty.
//    m_pNext=NULL;
    
    for (DWORD dwCount=0;dwCount<MAX_PHYSICAL_BLOCK;dwCount++) {
        iTD_BufferPagePointer[dwCount].dwITD_BufferPagePointer = 0;
        iTD_ExtendedBufferPagePointer[dwCount] = 0;
    }
    
    for (dwCount=0; dwCount<MAX_TRNAS_PER_ITD; dwCount++) 
        iTD_StatusControl[dwCount].dwITD_StatusControl=0;
    
    m_dwPhys = (m_pTrans?(m_pTrans->m_pCPipe->GetCPhysMem())-> VaToPa((PBYTE)this):0);
    
}
void CITD::ReInit(CITransfer * pIsochTransfer)
{
    nextLinkPointer.dwLinkPointer=1;
    m_pTrans =pIsochTransfer ;    
    for (DWORD dwCount=0;dwCount<MAX_PHYSICAL_BLOCK;dwCount++) 
        iTD_BufferPagePointer[dwCount].dwITD_BufferPagePointer = 0;
    
    for (dwCount=0; dwCount<MAX_TRNAS_PER_ITD; dwCount++) 
        iTD_StatusControl[dwCount].dwITD_StatusControl=0;
    
    m_dwPhys = (m_pTrans?(m_pTrans->m_pCPipe->GetCPhysMem())-> VaToPa((PBYTE)this):0);
}
void CITD::SetIOC(BOOL bSet)
{
    CheckStructure ();
    if (bSet) {
        for (int iCount = MAX_TRNAS_PER_ITD-1;iCount>0;iCount--)
            if (iTD_StatusControl[iCount].iTD_SCContext.TransactionLength!=0) {
                iTD_StatusControl[iCount].iTD_SCContext.InterruptOnComplete=1;
                break;
            }
    }
    else {
        for (int iCount =0;iCount< MAX_TRNAS_PER_ITD;iCount++)
            if (iTD_StatusControl[iCount].iTD_SCContext.TransactionLength!=0) 
                iTD_StatusControl[iCount].iTD_SCContext.InterruptOnComplete=0;
    }
}

DWORD CITD::IssueTransfer(DWORD dwNumOfTrans,PDWORD pdwTransLenArray, PDWORD pdwFrameAddrArray,BOOL bIoc,BOOL bIn)
{
    CheckStructure ();
    if (dwNumOfTrans ==NULL || dwNumOfTrans>MAX_TRNAS_PER_ITD ||pdwTransLenArray==NULL || pdwFrameAddrArray==NULL) {
        ASSERT(FALSE);
        return 0;
    }
    // Initial Buffer Pointer.
    for (DWORD dwCount=0;dwCount<MAX_PHYSICAL_BLOCK;dwCount++) 
        iTD_BufferPagePointer[dwCount].dwITD_BufferPagePointer = 0;
    
    DWORD dwCurBufferPtr=0;
    DWORD dwCurValidPage = ((DWORD)-1) &  EHCI_PAGE_ADDR_MASK;
    for (dwCount=0;dwCount<MAX_TRNAS_PER_ITD +1  && dwCount < dwNumOfTrans +1 && dwCurBufferPtr < MAX_PHYSICAL_BLOCK ;dwCount++) {
        if (dwCurValidPage !=  (*(pdwFrameAddrArray+dwCount)&EHCI_PAGE_ADDR_MASK) ) {
            dwCurValidPage = iTD_BufferPagePointer[dwCurBufferPtr].dwITD_BufferPagePointer 
                    = (*(pdwFrameAddrArray+dwCount) ) & EHCI_PAGE_ADDR_MASK;
            if (*(pdwTransLenArray +dwCount)==0) { // Endof Transfer
                break;
            }
            dwCurBufferPtr ++;
        }
    }
    USB_ENDPOINT_DESCRIPTOR endptDesc = m_pTrans->m_pCPipe->GetEndptDescriptor();
    
    iTD_BufferPagePointer[0].iTD_BPPContext1.DeviceAddress= m_pTrans->m_pCPipe->GetDeviceAddress();
    iTD_BufferPagePointer[0].iTD_BPPContext1.EndPointNumber=endptDesc.bEndpointAddress;
    iTD_BufferPagePointer[1].iTD_BPPContext2.MaxPacketSize=endptDesc.wMaxPacketSize & 0x7ff;
    iTD_BufferPagePointer[1].iTD_BPPContext2.Direction=(bIn?1:0);
    iTD_BufferPagePointer[2].iTD_BPPContext3.Multi=((endptDesc.wMaxPacketSize>>11) & 3)+1;
    ASSERT(((endptDesc.wMaxPacketSize>>11)&3)!=3);
    
    // Initial Transaction 
    dwCurValidPage = (*pdwFrameAddrArray) &  EHCI_PAGE_ADDR_MASK;
    dwCurBufferPtr=0;    
    for (dwCount=0; dwCount<MAX_TRNAS_PER_ITD; dwCount++) {
        iTD_StatusControl[dwCount].dwITD_StatusControl=0;
        if (dwCount < dwNumOfTrans) {
            if (dwCurValidPage != (*(pdwFrameAddrArray+dwCount)&EHCI_PAGE_ADDR_MASK)) {
                dwCurValidPage = *(pdwFrameAddrArray+dwCount)&EHCI_PAGE_ADDR_MASK;
                dwCurBufferPtr ++;                
            }
            iTD_StatusControl[dwCount].iTD_SCContext.TransactionLength = *(pdwTransLenArray+dwCount);
            iTD_StatusControl[dwCount].iTD_SCContext.TransationOffset  = (*(pdwFrameAddrArray+dwCount) & EHCI_PAGE_OFFSET_MASK);
            iTD_StatusControl[dwCount].iTD_SCContext.PageSelect = dwCurBufferPtr;
            iTD_StatusControl[dwCount].iTD_SCContext.Active = 1;
            if (dwCount == dwNumOfTrans -1 && bIoc )  { // if thiere is last one and interrupt on completion. do it.
                iTD_StatusControl[dwCount].iTD_SCContext.InterruptOnComplete=1;
            }
        }        
    }
    
    return (dwNumOfTrans<MAX_TRNAS_PER_ITD?dwNumOfTrans:MAX_TRNAS_PER_ITD);
    
};
CSITD::CSITD(CSITransfer * pTransfer,CSITD * pPrev)
: m_pTrans(pTransfer)
, m_CheckFlag(CSITD_CHECK_FLAG_VALUE)
{
    ASSERT((&(nextLinkPointer.dwLinkPointer))+ 6 == &(backPointer.dwLinkPointer)); // Check for Data Intergraty.
    sITD_CapChar.dwSITD_CapChar=0;
    microFrameSchCtrl.dwMicroFrameSchCtrl=0;
    sITD_TransferState.dwSITD_TransferState=0;
    sITD_BPPage[0].dwSITD_BPPage=0;
    sITD_BPPage[1].dwSITD_BPPage=0;
    sITD_ExtendedBPPage[0] =  sITD_ExtendedBPPage[1] = 0;
    backPointer.dwLinkPointer=1; // Invalid Back Link

    UCHAR S_Mask = (m_pTrans?m_pTrans->m_pCPipe->GetSMask():1);
    ASSERT(S_Mask!=0);
    if (S_Mask==0) // Start Mask has to be present
        S_Mask=1;
    microFrameSchCtrl.sITD_MFSCContext.SplitStartMask=S_Mask;
    microFrameSchCtrl.sITD_MFSCContext.SplitCompletionMask=(m_pTrans?m_pTrans->m_pCPipe->GetCMask():0);
    m_pPrev=pPrev;
    m_dwPhys =(m_pTrans?(m_pTrans->m_pCPipe->GetCPhysMem())-> VaToPa((PBYTE)this):0);
}
void CSITD::ReInit(CSITransfer * pTransfer,CSITD * pPrev)
{
    ASSERT( pTransfer!=NULL);
    nextLinkPointer.dwLinkPointer=1;
    m_pTrans=pTransfer;
    sITD_CapChar.dwSITD_CapChar=0;
    microFrameSchCtrl.dwMicroFrameSchCtrl=0;
    sITD_TransferState.dwSITD_TransferState=0;
    sITD_BPPage[0].dwSITD_BPPage=0;
    sITD_BPPage[1].dwSITD_BPPage=0;
    backPointer.dwLinkPointer=1; // Invalid Back Link

    UCHAR S_Mask = (m_pTrans?m_pTrans->m_pCPipe->GetSMask():1);
    ASSERT(S_Mask!=0);
    if (S_Mask==0) // Start Mask has to be present
        S_Mask=1;
    microFrameSchCtrl.sITD_MFSCContext.SplitStartMask=S_Mask;
    microFrameSchCtrl.sITD_MFSCContext.SplitCompletionMask=(m_pTrans?m_pTrans->m_pCPipe->GetCMask():0);
    m_pPrev=pPrev;
    m_dwPhys = (m_pTrans?(m_pTrans->m_pCPipe->GetCPhysMem())-> VaToPa((PBYTE)this):0);

}

#define MAX_SPLIT_TRANSFER_LENGTH 188
DWORD CSITD::IssueTransfer(DWORD dwPhysAddr, DWORD dwEndPhysAddr, DWORD dwLen,BOOL bIoc,BOOL bIn)
{
    CheckStructure ();
    if (dwPhysAddr==0 || dwLen==0||  dwLen > 1024) {
        ASSERT(FALSE);
        return 0;
    }
    sITD_BPPage[0].sITD_BPPage0.BufferPointer = ((dwPhysAddr) >> EHCI_PAGE_ADDR_SHIFT);
    sITD_BPPage[0].sITD_BPPage0.CurrentOffset = ((dwPhysAddr) & EHCI_PAGE_OFFSET_MASK);
    sITD_BPPage[1].sITD_BPPage1.BufferPointer = (dwEndPhysAddr) & EHCI_PAGE_ADDR_MASK;
    
    USB_ENDPOINT_DESCRIPTOR endptDesc = m_pTrans->m_pCPipe->GetEndptDescriptor();
    sITD_CapChar.sITD_CCContext.DeviceAddress = m_pTrans->m_pCPipe->GetDeviceAddress();
    sITD_CapChar.sITD_CCContext.Endpt = endptDesc.bEndpointAddress;
    sITD_CapChar.sITD_CCContext.HubAddress = m_pTrans->m_pCPipe->m_bHubAddress;
    sITD_CapChar.sITD_CCContext.PortNumber= m_pTrans->m_pCPipe->m_bHubPort;
    sITD_CapChar.sITD_CCContext.Direction =(bIn?1:0);

     // S-Mask and C-Mask has been initialized
     // Status will be inactive and DoStartSlit.

    sITD_TransferState.sITD_TSContext.BytesToTransfer = dwLen;
    sITD_TransferState.sITD_TSContext.PageSelect=0; // Always use first page first.
    sITD_TransferState.sITD_TSContext.IOC= (bIoc?1:0);

     //
    sITD_BPPage[1].sITD_BPPage1.TP=(dwLen>MAX_SPLIT_TRANSFER_LENGTH?1:0);
    DWORD dwSlitCount=( dwLen+MAX_SPLIT_TRANSFER_LENGTH-1) /MAX_SPLIT_TRANSFER_LENGTH;
    if (dwSlitCount>6) {
        ASSERT(FALSE);
        dwSlitCount=6;
     }
    if(bIn)
        dwSlitCount=1;
    sITD_BPPage[1].sITD_BPPage1.T_Count = dwSlitCount;
    sITD_TransferState.sITD_TSContext.Active=1;

    // Setup the back pointer if there is.
    if (m_pPrev==NULL || m_pPrev->GetPhysAddr()== 0)
        backPointer.dwLinkPointer=1;
    else {
        CNextLinkPointer backP;
        backP.SetNextPointer(m_pPrev->GetPhysAddr(), TYPE_SELECT_SITD, TRUE);
        backPointer.dwLinkPointer = backP.GetDWORD();        
    }
    return 1;
     
};

CQTD::CQTD( CQTransfer * pTransfer, CQH * pQh)
: m_pTrans(pTransfer)
, m_pQh(pQh)
, m_CheckFlag(CQTD_CHECK_FLAG_VALUE)
{
    ASSERT((&(nextLinkPointer.dwLinkPointer))+ 3 == &(qTD_BufferPointer[0].dwQTD_BufferPointer)); // Check for Data Intergraty.    
    m_pNext=NULL;
    altNextQTDPointer.dwLinkPointer=1;
    qTD_Token.dwQTD_Token=0;
    for (DWORD dwIndex=0;dwIndex<5; dwIndex++)
        qTD_BufferPointer[dwIndex].dwQTD_BufferPointer=0;
    nextLinkPointer.dwLinkPointer=1;
    altNextQTDPointer.dwLinkPointer=1;
    qTD_Token.dwQTD_Token=0;
    for (dwIndex=0;dwIndex<5;dwIndex++) {
        qTD_BufferPointer[dwIndex].dwQTD_BufferPointer = 0;
        qTD_ExtendedBufferPointer[dwIndex] = 0;
    }
    m_dwPhys = (m_pTrans->m_pCPipe->GetCPhysMem())-> VaToPa((PBYTE)this);
}
DWORD CQTD::IssueTransfer(DWORD dwPID, BOOL bToggle1, DWORD dwTransLength, PPhysBufferArray pPhysBufferArray,BOOL bIoc)
{
    CheckStructure ();
    if ( pPhysBufferArray ==NULL ) {
        ASSERT(FALSE);
        return 0;
    }
    ASSERT((pPhysBufferArray->dwBlockSize== dwTransLength + (pPhysBufferArray ->dwStartOffset & EHCI_PAGE_OFFSET_MASK)) ||
        pPhysBufferArray->dwBlockSize == EHCI_PAGE_SIZE);
    DWORD dwMaxPacketSize = (m_pTrans->m_pCPipe->GetEndptDescriptor()).wMaxPacketSize & 0x7ff;
    DWORD dwMaxPacketNumber=(EHCI_PAGE_SIZE* MAX_QTD_PAGE_SIZE)/dwMaxPacketSize;
    DWORD dwTotalTransfer=min(dwTransLength,dwMaxPacketNumber*dwMaxPacketSize);
    DWORD dwTotalPage = (pPhysBufferArray ->dwStartOffset + dwTransLength + EHCI_PAGE_SIZE -1)/EHCI_PAGE_SIZE;
    ASSERT(dwTotalPage<MAX_QTD_PAGE_SIZE+1);
    for (DWORD dwIndex=0;dwIndex<dwTotalPage && dwIndex<MAX_QTD_PAGE_SIZE+1;dwIndex++) {
        qTD_BufferPointer[dwIndex].dwQTD_BufferPointer = (pPhysBufferArray ->dwArrayBlockAddr[dwIndex] &  EHCI_PAGE_ADDR_MASK);
    }
    qTD_BufferPointer[0].qTD_BPContext.CurrentOffset = pPhysBufferArray ->dwStartOffset & EHCI_PAGE_OFFSET_MASK;
    qTD_Token.qTD_TContext.PID = (dwPID== TD_OUT_PID ?0:(dwPID==TD_SETUP_PID?2:1));
    qTD_Token.qTD_TContext.C_Page= 0;
    qTD_Token.qTD_TContext.IOC=((dwTotalTransfer < dwTransLength)?0:(bIoc?1:0));
    qTD_Token.qTD_TContext.BytesToTransfer=dwTotalTransfer;
    qTD_Token.qTD_TContext.DataToggle=(bToggle1?1:0);
    qTD_Token.qTD_TContext.Active = 1; 
    return dwTotalTransfer;

}
CQTD * CQTD::QueueNextTD(CQTD * pNextTD)
{
    CheckStructure ();
    CQTD * pReturn = m_pNext;
    m_pNext = pNextTD;
    SetNextPointer(pNextTD-> GetPhysAddr(),TYPE_SELECT_ITD,TRUE);
    return pReturn;
}
CQH::CQH(CPipe *pPipe)
    : m_pPipe(pPipe)
    , m_CheckFlag(CQH_CHECK_FLAG_VALUE)
{
    ASSERT((&(nextLinkPointer.dwLinkPointer))+ 1 == &(qH_StaticEndptState.qH_StaticEndptState[0])); // Check for Data Intergraty.
    ASSERT((&(qH_StaticEndptState.qH_StaticEndptState[0]))+ 4 == &(qTD_Overlay.altNextQTDPointer.dwLinkPointer)); // Check for Data Intergraty.
    PREFAST_DEBUGCHK( pPipe );
    CheckStructure ();
    m_pNextQHead = NULL;
    USB_ENDPOINT_DESCRIPTOR endptDesc =pPipe->GetEndptDescriptor();
    qH_StaticEndptState.qH_StaticEndptState[0]=0;
    qH_StaticEndptState.qH_StaticEndptState[1]=0;
    qH_StaticEndptState.qH_SESContext.DeviceAddress= pPipe->GetDeviceAddress();
    qH_StaticEndptState.qH_SESContext.I = 0;
    qH_StaticEndptState.qH_SESContext.Endpt =endptDesc.bEndpointAddress;
    qH_StaticEndptState.qH_SESContext.ESP = (pPipe->IsHighSpeed()?2:(pPipe->IsLowSpeed()?1:0));
    qH_StaticEndptState.qH_SESContext.DTC = 1; // Enable Data Taggle.
    qH_StaticEndptState.qH_SESContext.H=0;
    qH_StaticEndptState.qH_SESContext.MaxPacketLength =endptDesc.wMaxPacketSize & 0x7ff;
    qH_StaticEndptState.qH_SESContext.C  = 
        ((endptDesc.bmAttributes &  USB_ENDPOINT_TYPE_MASK)==USB_ENDPOINT_TYPE_CONTROL && pPipe->IsHighSpeed()!=TRUE ?1:0);
    qH_StaticEndptState.qH_SESContext.RL=0xf; // TODO Check this is value is correct or not.
// DWORD 2
    qH_StaticEndptState.qH_SESContext.UFrameSMask = pPipe->GetSMask();
    qH_StaticEndptState.qH_SESContext.UFrameCMask = pPipe->GetCMask();
    qH_StaticEndptState.qH_SESContext.HubAddr = pPipe->m_bHubAddress;
    qH_StaticEndptState.qH_SESContext.PortNumber = pPipe->m_bHubPort;
    qH_StaticEndptState.qH_SESContext.Mult = ((endptDesc.wMaxPacketSize>>11) & 3)+1;

    currntQTDPointer.dwLinkPointer = 0; 
    nextQTDPointer.dwLinkPointer = 1; // Terminate;
    memset((void *)&qTD_Overlay, 0 , sizeof(QTD));
    qTD_Overlay.altNextQTDPointer.dwLinkPointer = 1; // This is For short transfer 
    qTD_Overlay.qTD_Token.qTD_TContext.Halted = 1;
    
    m_dwPhys = (pPipe->GetCPhysMem())-> VaToPa((PBYTE)this);
//    m_pStaticQHead = NULL;
}

BOOL CQH::QueueTD(CQTD * pCurTD)
{
    CheckStructure ();
    if (pCurTD  && nextQTDPointer.lpContext.Terminate!=0 && qTD_Overlay.qTD_Token.qTD_TContext.Active == 0 ) { // Queue If they queue is not active.
        //memcpy((void *)&qTD_Overlay, pCurTD->GetQTDData(),sizeof(QTD));
        currntQTDPointer.dwLinkPointer =  0;
        ASSERT((pCurTD->GetPhysAddr() & 0x1f) == 0);
        nextQTDPointer.dwLinkPointer =pCurTD->GetPhysAddr(); // This will activate the TD trasnfer.
        qTD_Overlay.qTD_Token.qTD_TContext.Halted = 0;
        return TRUE;
    }
    else {
        ASSERT(FALSE);
        return FALSE;
    };
}


