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
//     Trans.h
// 
// Abstract: Provides interface to UHCI host controller
// 
// Notes: 
//
#ifndef __TRNAS_H_
#define __TRNAS_H_
#include <Cphysmem.hpp>
#include <ctd.h>
class CPipe;
class CMhcd;
typedef struct STRANSFER {
    // These are the IssueTransfer parameters
    IN LPTRANSFER_NOTIFY_ROUTINE lpStartAddress;
    IN LPVOID lpvNotifyParameter;
    IN DWORD dwFlags;
    IN LPCVOID lpvControlHeader;
    IN DWORD dwStartingFrame;
    IN DWORD dwFrames;
    IN LPCDWORD aLengths;
    IN DWORD dwBufferSize;
    IN_OUT LPVOID lpvBuffer;
    IN ULONG paBuffer;
    IN LPCVOID lpvCancelId;
    OUT LPDWORD adwIsochErrors;
    OUT LPDWORD adwIsochLengths;
    OUT LPBOOL lpfComplete;
    OUT LPDWORD lpdwBytesTransferred;
    OUT LPDWORD lpdwError ;
} STransfer ;

class CTransfer ;
class CTransfer {
public:
    CTransfer(IN CPipe * const cPipe, IN CPhysMem * const pCPhysMem,STransfer sTransfer) ;
    virtual ~CTransfer();
    CPipe * const m_pCPipe;
    CPhysMem * const m_pCPhysMem;
    CTransfer * GetNextTransfer(void) { return  m_pNextTransfer; };
    void SetNextTransfer(CTransfer * pNext) {  m_pNextTransfer= pNext; };
    virtual BOOL Init(void);
    virtual BOOL AddTransfer () =0;
    STransfer GetSTransfer () { return m_sTransfer; };
    DWORD GetPermissions() { return m_dwCurrentPermissions;};
    void  DoNotCallBack() {
        m_sTransfer.lpfComplete = NULL;
        m_sTransfer.lpdwError = NULL;
        m_sTransfer.lpdwBytesTransferred = NULL;
        m_sTransfer.lpStartAddress = NULL;
    }
protected:
    CTransfer * m_pNextTransfer;
    PBYTE   m_pAllocatedForControl;
    PBYTE   m_pAllocatedForClient;
    DWORD   m_paControlHeader;
    STransfer m_sTransfer;
    DWORD   m_DataTransferred;
    DWORD   m_dwCurrentPermissions ;
    DWORD   m_dwTransferID;
    static  DWORD m_dwGlobalTransferID;
    
};
class CQTransfer : public CTransfer {
public:
    CQTransfer(IN CPipe *  const pCPipe, IN CPhysMem * const pCPhysMem,STransfer sTransfer) 
        : CTransfer(pCPipe,pCPhysMem,sTransfer)
    {   m_pCQTDList=NULL;   };
    ~CQTransfer();
    BOOL AddTransfer () ;
    BOOL AbortTransfer();
    BOOL IsTransferDone();
    BOOL DoneTransfer();
    BOOL ProcessEP(DWORD dwStatus, BOOL isDMA);
    DWORD ProcessResponse(DWORD dwReason, CQTD *pCurTD);
    CQTD *GetCQTDList() { return m_pCQTDList; };
private:
    CQTD *m_pCQTDList;
};


#endif
