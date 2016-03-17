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
//     CTd.h
// 
// Abstract: Provides interface to UHCI host controller
// 
// Notes: 
//
#ifndef __CTD_H_
#define __CTD_H_

#include "td.h"
#include <cphysmem.hpp>
#define MAX_PHYSICAL_BLOCK 7
#define MAX_TRNAS_PER_ITD 8

#define TD_SETUP_PID 0x2d
#define TD_IN_PID 0x69
#define TD_OUT_PID 0xe1


class CNextLinkPointer  {
protected:
    volatile NextLinkPointer nextLinkPointer;
public:
    void * operator new(size_t stSize, CPhysMem * const pCPhysMem);
    void operator delete (void *pointer);
    CNextLinkPointer() {  nextLinkPointer.dwLinkPointer=1;}; // Invalid Zero Pointer.
    DWORD   SetDWORD(DWORD dwValue) { 
        DWORD dwReturn= nextLinkPointer.dwLinkPointer;
         nextLinkPointer.dwLinkPointer=dwValue;
        return dwReturn;
    }
    DWORD   GetDWORD() { return  nextLinkPointer.dwLinkPointer ;};
    
    BOOL    SetLinkValid(BOOL bTrue) {
        BOOL bReturn=( nextLinkPointer.lpContext.Terminate==0);
         nextLinkPointer.lpContext.Terminate=(bTrue?0:1);
        return bReturn;
    }
    BOOL    GetLinkValid() { return ( nextLinkPointer.lpContext.Terminate==0); };
    
    NEXTLINKTYPE   SetLinkType(NEXTLINKTYPE nLinkType) {
        NEXTLINKTYPE nReturn=(NEXTLINKTYPE) nextLinkPointer.lpContext.TypeSelect;
         nextLinkPointer.lpContext.TypeSelect=(DWORD)nLinkType;
        return nReturn;
    }
    NEXTLINKTYPE   GetLinkType() { return ((NEXTLINKTYPE)nextLinkPointer.lpContext.TypeSelect); };
    
    DWORD   SetPointer(DWORD dwPointer) {
        DWORD dwReturn= ( nextLinkPointer.lpContext.LinkPointer <<5);
        ASSERT((dwPointer & 0x1f) == 0 ); // Alignment check.
        nextLinkPointer.lpContext.LinkPointer = (dwPointer>>5);
        return dwReturn;
    }
    DWORD   GetPointer() {  return ( nextLinkPointer.lpContext.LinkPointer <<5); };

    BOOL SetNextPointer(DWORD dwPhysAddr,NEXTLINKTYPE nLinkType,BOOL bValid) {
        CNextLinkPointer cNext;
        cNext.SetLinkValid(bValid);
        cNext.SetLinkType(nLinkType);
        cNext.SetPointer(dwPhysAddr);
        nextLinkPointer.dwLinkPointer = cNext.GetDWORD();
        return TRUE;
    }
    CNextLinkPointer * GetNextLinkPointer(CPhysMem *pPhysMem) {
        ASSERT(pPhysMem);
        if (GetLinkValid() && pPhysMem ) {
            return (CNextLinkPointer * )pPhysMem->PaToVa(GetPointer());
        }
        else 
            return NULL;
    }
};

class CITransfer;
class CITD;
#define CITD_CHECK_FLAG_VALUE 0xc3a5f101
class CITD: public CNextLinkPointer, protected ITD{
public:
    CITD(CITransfer * pIsochTransfer);
    void ReInit(CITransfer * pIsochTransfer);
    ~CITD(){CheckStructure ();};
    DWORD IssueTransfer(DWORD dwNumOfTrans,PDWORD pdwTransLenArray, PDWORD pdwFrameAddrArray,BOOL bIoc,BOOL bIn);
    BOOL ActiveTrasfer() {CheckStructure ();
        for (DWORD dwCount =0; dwCount < MAX_TRNAS_PER_ITD; dwCount ++) {
            if (iTD_StatusControl[dwCount].iTD_SCContext.TransactionLength!=0)
                iTD_StatusControl[dwCount].iTD_SCContext.Active = 1;
        }
        return TRUE;
    }
    BOOL IsActive() { CheckStructure ();
        BOOL bReturn = FALSE;
        for (DWORD dwCount =0; dwCount < MAX_TRNAS_PER_ITD; dwCount ++) {
            if ( iTD_StatusControl[dwCount].iTD_SCContext.Active == 1)
                bReturn = TRUE;
        }
        return bReturn;
    }
    void SetIOC(BOOL bSet);
    DWORD GetPhysAddr() { return m_dwPhys; };
    DWORD SetPhysAddr (DWORD dwPhys) {
        CheckStructure ();
        DWORD dwReturn=m_dwPhys;
        m_dwPhys=dwPhys;
        return dwReturn;
    }
    BOOL CheckStructure () {
        ASSERT(m_CheckFlag==CITD_CHECK_FLAG_VALUE);
        return (m_CheckFlag==CITD_CHECK_FLAG_VALUE);
    }
private:
    CITransfer * m_pTrans;
    DWORD m_dwPhys;
    const DWORD m_CheckFlag;
    friend class CITransfer;
};
class CSITransfer ;
class CSITD;
#define CSITD_CHECK_FLAG_VALUE 0xc3a5f102
class CSITD : public CNextLinkPointer, SITD {
public:
    CSITD(CSITransfer * pTransfer,CSITD * pPrev);
    void ReInit(CSITransfer * pTransfer,CSITD * pPrev);
    ~CSITD() {CheckStructure ();;};
    DWORD IssueTransfer(DWORD dwPhysAddr, DWORD dwEndPhysAddr, DWORD dwLen,BOOL bIoc,BOOL bIn);
    DWORD GetPhysAddr() { CheckStructure ();return m_dwPhys; };
    DWORD SetPhysAddr (DWORD dwPhys) {
        CheckStructure ();
        DWORD dwReturn=m_dwPhys;
        m_dwPhys=dwPhys;
        return dwReturn;
    }
    void SetIOC(BOOL bSet) { CheckStructure ();sITD_TransferState.sITD_TSContext.IOC = (bSet?1:0); };
    BOOL CheckStructure () {
        ASSERT(m_CheckFlag==CSITD_CHECK_FLAG_VALUE);
        return (m_CheckFlag==CSITD_CHECK_FLAG_VALUE);
    }
private:
    CSITransfer  * m_pTrans;
    CSITD * m_pPrev;
    DWORD m_dwPhys;
    const DWORD m_CheckFlag;
    friend class CSITransfer;
};

#define CQTD_CHECK_FLAG_VALUE 0xc3a5f103
class CQTransfer;
class CQH;
class CQTD;
class CQTD :  public CNextLinkPointer,QTD {
public:
    CQTD(CQTransfer * pTransfer, CQH * pQh);
    ~CQTD() {CheckStructure ();};
    DWORD IssueTransfer(DWORD dwPID, BOOL bToggle1, DWORD dwTransLength, PPhysBufferArray pPhysBufferArray,BOOL bIoc);
    DWORD GetPhysAddr() {         CheckStructure ();return m_dwPhys; };
    CQTD *GetNextTD() {         CheckStructure ();return m_pNext; };
    CQTD *QueueNextTD(CQTD * pNextTD) ;
    void SetAltNextQTDPointer(DWORD dwPhysAddr) { 
        CheckStructure ();
        ASSERT((dwPhysAddr & 0x1f)== 0 ) ;// 32-byte alignment
        altNextQTDPointer.dwLinkPointer=dwPhysAddr;
    }
    CQTransfer * GetTransfer() {         CheckStructure ();return m_pTrans; };
    void CheckStructure () {
        ASSERT(m_CheckFlag==CQTD_CHECK_FLAG_VALUE);
    }
    void DeActiveTD() {
        CheckStructure ();
        qTD_Token.qTD_TContext.Active =0;// Deativate TD
        SetDWORD(1); // Physical Terminate Transfer Linker
    }
private:
    CQTransfer * const m_pTrans;
    CQH  * const m_pQh;
    CQTD * m_pNext;
    DWORD m_dwPhys;
    const DWORD m_CheckFlag;
    friend class CQTransfer;
};

class CPipe;
#define CQH_CHECK_FLAG_VALUE 0xc3a5f104
class CQH : public CNextLinkPointer, public QH {
public :
    CQH(CPipe *pPipe);
    ~CQH() {;};
    BOOL IsActive() {
        CheckStructure ();
        return ((nextQTDPointer.lpContext.Terminate == 0 || qTD_Overlay.qTD_Token.qTD_TContext.Active ==1) && 
                qTD_Overlay.qTD_Token.qTD_TContext.Halted ==0 );
    }
    void SetDeviceAddress(DWORD dwDeviceAddress) { 
        CheckStructure ();
        qH_StaticEndptState.qH_SESContext.DeviceAddress = dwDeviceAddress;
    }
    void SetReclamationFlag(BOOL bSet) {  CheckStructure ();qH_StaticEndptState.qH_SESContext.H=(bSet?1:0);  }
    BOOL GetReclamationFlag() {CheckStructure ();return (qH_StaticEndptState.qH_SESContext.H==1); }

    void SetDTC(BOOL bSet) {CheckStructure (); qH_StaticEndptState.qH_SESContext.DTC = (bSet?1:0); };
    void SetControlEnpt(BOOL bSet) { CheckStructure ();qH_StaticEndptState.qH_SESContext.C = (bSet?1:0);};
    void SetSMask(UCHAR SMask) { CheckStructure ();qH_StaticEndptState.qH_SESContext.UFrameSMask=SMask; };
    void SetCMask(UCHAR CMask) { CheckStructure ();qH_StaticEndptState.qH_SESContext.UFrameCMask=CMask; };
    void SetMaxPacketLength (DWORD dwMaxPacketLength) {CheckStructure (); qH_StaticEndptState.qH_SESContext.MaxPacketLength = (dwMaxPacketLength & 0x7ff); };
    void SetReLoad(DWORD dwCount) { CheckStructure ();qH_StaticEndptState.qH_SESContext.RL = dwCount; };
    void SetINT(BOOL bTrue) {CheckStructure (); qH_StaticEndptState.qH_SESContext.I=(bTrue?1:0); };

    BOOL QueueTD(CQTD * pCurTD);
    void InvalidNextTD() { 
        CheckStructure ();
        currntQTDPointer.dwLinkPointer = 0; 
        nextQTDPointer.lpContext.Terminate=1; 
        qTD_Overlay.altNextQTDPointer.dwLinkPointer=1;
        qTD_Overlay.qTD_Token.qTD_TContext.Active = 0; 
        qTD_Overlay.qTD_Token.qTD_TContext.Halted = 1;
        qTD_Overlay.qTD_Token.qTD_TContext.BytesToTransfer=0;

    };
    void ResetOverlayDataToggle() {
        qTD_Overlay.qTD_Token.qTD_TContext.DataToggle = 0 ;
    }
    DWORD GetPhysAddr() { CheckStructure ();return m_dwPhys; };
    CQH *GetNextQueueQHead(IN CPhysMem * const pCPhysMem) { 
        ASSERT(GetNextLinkPointer(pCPhysMem) == m_pNextQHead);
        CheckStructure ();
        return (CQH *) GetNextLinkPointer(pCPhysMem); };
    BOOL QueueQHead(CQH *pNextQH){
        CheckStructure ();
        m_pNextQHead = pNextQH;
        if (pNextQH) {
            return SetNextPointer(pNextQH->GetPhysAddr(), TYPE_SELECT_QH, TRUE);
        }
        else {
            SetDWORD(1);// ValidPointer;
            return TRUE;
        }
    }

    BOOL CheckStructure () {
        ASSERT(m_CheckFlag==CQH_CHECK_FLAG_VALUE);
        return (m_CheckFlag==CQH_CHECK_FLAG_VALUE);
    }
private:
    CPipe * const m_pPipe;
    DWORD   m_dwPhys;
    CQH *   m_pNextQHead;
    const DWORD m_CheckFlag;
};

#endif

