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
//     CTd.h
// 
// Abstract: Provides interface to host controller
// 
// Notes: 
//
#ifndef __CTD_H_
#define __CTD_H_

#define ERR_BUFFER_ERROR    0x1
enum {
    STATUS_PROCESS_INTR,
    STATUS_PROCESS_ERROR,
    STATUS_PROCESS_STALL,
    STATUS_PROCESS_NAKTIMEOUT,
    STATUS_PROCESS_ABORT
};

enum {    
    STATUS_IDLE,
    STATUS_WAIT_TRANSFER,
    STATUS_ACTIVE_START,
    STATUS_WAIT_RESPONSE,
    STATUS_WAIT_MORE_DATA,
    STATUS_SEND_MORE_DATA,
    STATUS_PROCESS_DMA,
    STATUS_WAIT_DMA_WR_RESPONSE,
    STATUS_WAIT_DMA_WR_2_FIFO_TXPKT_RDY,
    STATUS_WAIT_DMA_WR_2_FIFO_TXPKT_NOTRDY,
    STATUS_WAIT_DMA_0_RD_FIFO_COMPLETE,
    STATUS_WAIT_DMA_1_RD_FIFO_COMPLETE,
    STATUS_ACTIVE_END,
    STATUS_HALT,
    STATUS_ABORT,
    STATUS_ERROR,        
    STATUS_NOT_ENOUGH_BUFFER,
    STATUS_COMPLETE,
};

enum {
    TD_IDLE,
    TD_SETUP,
    TD_DATA_IN,
    TD_DATA_OUT,
    TD_STATUS_IN,
    TD_STATUS_OUT
};

class CQTransfer;
class CQH;
class CQTD;
class CQTD {
public:
    CQTD(CQTransfer * pTransfer, CQH * pQh);
    ~CQTD() {};
    // Function: IssueTransfer: Perform the transfer of data on the transfer descriptor
    DWORD IssueTransfer(DWORD dwPID, BOOL bToggle1, DWORD dwTransLength);
    // Function: GetNextTD: Return the next TD in the list
    CQTD *GetNextTD() {return m_pNext; };
    // Function: QueueNextTD: Put the new TD into the queue list
    CQTD *QueueNextTD(CQTD * pNextTD) ;
    // Function: GetTransfer: Get the transfer object
    CQTransfer * GetTransfer() {return m_pTrans; };
    void SetBuffer(DWORD dwPAData, DWORD dwVAData, DWORD cbData) { m_dwPAData=dwPAData; m_dwVAData=dwVAData; m_cbData=cbData;};
    void SetTDType(DWORD type) { m_dwTDType = type;};
    void SetToggle(BOOL bToggle) { m_bDataToggle = bToggle;};
    void SetStatus(DWORD status) { m_dwStatus = status;};
    void SetTotTfrSize(DWORD size) { m_cbTransferred = size;};
    void SetCurTfrSize(DWORD size) { m_cbCurTransferred = size;};
    void SetPacketSize(DWORD dwPacketSize) {m_dwPacketSize = dwPacketSize;};
    void SetError(DWORD error) { m_dwError = error; };
    DWORD GetStatus() { return m_dwStatus; };
    DWORD GetError() { return m_dwError; };
    DWORD GetTDType() { return m_dwTDType; };
    DWORD GetDataSize() { return m_cbData; };
    DWORD GetPAData() { return m_dwPAData; };
    DWORD GetVAData() { return m_dwVAData; };
    DWORD GetTotTfrSize() { return m_cbTransferred; };
    DWORD GetCurTfrSize() { return m_cbCurTransferred;};
    DWORD GetPacketSize() { return m_dwPacketSize;};    
    void DeActiveTD();
    DWORD  ReadDataIN(DWORD *pdwMore);
    void ClearRxPktRdy(void);
    void DumpReg(void);
    CQH *GetQH() { return m_pQh;};

private:
    CQTransfer * const m_pTrans;
    CQH  * const m_pQh;
    CQTD * m_pNext;
    // m_state provides what phase of the data being transferred, it can be
    // TD_SETUP, TD_DATA_IN, TD_DATA_OUT, TD_STATUS_IN, TD_STATUS_OUT
    DWORD   m_dwTDType; 
    // Address to the data to be transferred IN/OUT in physical address
    DWORD  m_dwPAData; 
    // Address to the data to be transferred IN/OUT in virtual address
    DWORD  m_dwVAData;
    // Size of the data to be transferred IN/OUT
    DWORD   m_cbData;
    // Size of the data actually recevied
    DWORD   m_cbTransferred;
    // Size of the data transferred in this batch
    DWORD   m_cbCurTransferred;
    // Toggle bit - it may need to use
    BOOL    m_bDataToggle;
    // Status of the transfer
    DWORD   m_dwStatus;
    // Packet Size
    DWORD   m_dwPacketSize;
    BOOL m_bSendDataSize;
    DWORD   m_dwError;  // bit 0 - buffer error

    friend class CQTransfer;
};

class CPipe;
class CQH  {
public :
    CQH(CPipe *pPipe);
    ~CQH() { DeleteCriticalSection(&m_csCQTD); };
    // Function: IsActive: Check if all the CQTD objects in the list has been completed.
    BOOL IsActive();
    // Function: SetDeviceAddress: Set the device address, should be FAddr or TxHubAddr/RxHubAddr
    void SetDeviceAddress(UCHAR ucDeviceAddress, UCHAR outdir);
    // Function: IssueTransfer: Put the pCurTD into Queue head and then start to process it by putting
    // one by one into FIFO
    DWORD IssueTransfer(CQTD * pCurTD);
    // Function: InvalidNextTD: Invalidate the TD in case of some errors, should stop the continual of processing
    // the transfer.
    void InvalidNextTD() {;};
    
    void Lock() { EnterCriticalSection(&m_csCQTD); };
    void UnLock() { LeaveCriticalSection(&m_csCQTD); };
    void InitLock() { InitializeCriticalSection(&m_csCQTD);};
    // Set Max Packet Size
    void SetMaxPacketLength(DWORD dwMaxPacketSize);
//    UCHAR GetEndPoint() { return m_bEndPointAddress; };
    CPipe *GetPipe() { return m_pPipe;};
    void ClearTDList() { m_pCQHTDList = NULL;};
private:
    CPipe * const m_pPipe;
    CQTD  * m_pCQHTDList;
    UCHAR   m_bEndPointAddress;
    // Handle for CQTD critical section, this is used to control the access in case of multiple acess of
    // the CQTD. It should be put in CQH as only 1 CQH is available for each endpoint connection.
    CRITICAL_SECTION  m_csCQTD; 
};

#endif

