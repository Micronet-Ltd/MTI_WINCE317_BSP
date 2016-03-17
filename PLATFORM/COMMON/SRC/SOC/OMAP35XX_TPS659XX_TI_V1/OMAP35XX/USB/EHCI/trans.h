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
class CIsochronousPipe;
class CEhcd;
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
    DWORD   m_dwTransferID;
    static  DWORD m_dwGlobalTransferID;
    BOOL    m_fDoneTransferCalled ;
    
};
class CQTransfer : public CTransfer {
public:
    CQTransfer(IN CPipe *  const pCPipe, IN CPhysMem * const pCPhysMem,STransfer sTransfer) 
        : CTransfer(pCPipe,pCPhysMem,sTransfer)
    {   m_pCQTDList=NULL;   
        m_fCanceled = FALSE;
    };
    ~CQTransfer();
    BOOL AddTransfer () ;
    BOOL AbortTransfer();
    BOOL IsTransferDone();
    BOOL DoneTransfer();
    CQTD *GetCQTDList() { return m_pCQTDList; };
private:
    CQTD *m_pCQTDList;
    BOOL    m_fCanceled;
};

class CIsochTransfer : public CTransfer {
public:
    CIsochTransfer(IN CIsochronousPipe * const pCPipe, IN CEhcd * const pCEhcd,STransfer sTransfer) ;
    virtual ~CIsochTransfer() {
        ASSERT(m_dwSchedTDIndex<=m_dwArmedTDIndex);
        ASSERT(m_dwDequeuedTDIndex<=m_dwSchedTDIndex);
        ASSERT(m_dwArmedBufferIndex<=m_sTransfer.dwBufferSize);
    };
    virtual BOOL AbortTransfer()=0;
    virtual BOOL IsTransferDone(DWORD dwCurFrameIndex,DWORD dwCurMicroFrameIndex)=0;
    virtual BOOL ScheduleTD(DWORD dwCurFrameIndex,DWORD dwCurMicroFrameIndex)=0;
    virtual BOOL DoneTransfer(DWORD dwCurFrameIndex,DWORD dwCurMicroFrameIndex)=0;
    DWORD GetStartFrame() { return m_dwFrameIndexStart; };
    BOOL SetStartFrame(DWORD dwStartFrame) { 
        if (m_dwDequeuedTDIndex ==0 && m_dwSchedTDIndex==0) {
            m_dwFrameIndexStart= m_sTransfer.dwStartingFrame= dwStartFrame; 
            return TRUE;
        }
        else
            return FALSE;
    }
    CIsochronousPipe * const GetPipe() { return (CIsochronousPipe * const) m_pCPipe; };
    inline DWORD   GetMaxTransferPerItd();

protected:
    CEhcd * const m_pCEhcd;
    DWORD   m_dwNumOfTD;
    DWORD   m_dwSchedTDIndex;
    DWORD   m_dwDequeuedTDIndex;
    DWORD   m_dwFrameIndexStart;

    DWORD   m_dwArmedTDIndex;
    DWORD   m_dwArmedBufferIndex;
    DWORD   m_dwFirstError;
	DWORD	m_dwLastFrameIndex;
};
class CITransfer : public  CIsochTransfer {
public:
    CITransfer(IN CIsochronousPipe * const pCPipe, IN CEhcd * const pCEhcd,STransfer sTransfer); 
    ~CITransfer();
    BOOL AddTransfer () ;
    BOOL ArmTD();
    BOOL IsTransferDone(DWORD dwCurFrameIndex,DWORD dwCurMicroFrameIndex);
    BOOL ScheduleTD(DWORD dwCurFrameIndex,DWORD dwCurMicroFrameIndex);
    BOOL AbortTransfer();
    BOOL DoneTransfer(DWORD dwCurFrameIndex,DWORD dwCurMicroFrameIndex);
private:
    CITD **  m_pCITDList;
};
class CSITransfer : public  CIsochTransfer {
public:
    CSITransfer(IN  CIsochronousPipe * const pCPipe,IN CEhcd * const pCEhcd ,STransfer sTransfer); 
    ~CSITransfer();
    BOOL AddTransfer () ;
    BOOL ArmTD();
    BOOL IsTransferDone(DWORD dwCurFrameIndex,DWORD dwCurMicroFrameIndex);
    BOOL ScheduleTD(DWORD dwCurFrameIndex,DWORD dwCurMicroFrameIndex);
    BOOL AbortTransfer();
    BOOL DoneTransfer(DWORD dwCurFrameIndex,DWORD dwCurMicroFrameIndex);
private:
    CSITD **  m_pCSITDList;
};

#endif
