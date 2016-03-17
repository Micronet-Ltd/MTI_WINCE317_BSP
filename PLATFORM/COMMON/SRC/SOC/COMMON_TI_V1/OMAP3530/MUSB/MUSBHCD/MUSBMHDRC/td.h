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
//     Td.h
// 
// Abstract: Provides interface to UHCI host controller
// 
// Notes: 
//
#ifndef __TD_H_
#define __TD_H_

#define MHCI_PAGESIZE 0x1000
#define MHCI_PAGEMASK 0xfff
#define MAX_QTD_PAGE_SIZE 4

#define MHCI_OFFSETOFPAGE(x) (x&MHCI_PAGEMASK)
#define MHCI_PAGEADDR(x) (x& ~MHCI_PAGEMASK)
typedef struct {
    DWORD Terminate:1;
    DWORD TypeSelect:2;
    DWORD Reserved:2;
    DWORD LinkPointer:27;
} LPContext;
//-------Type Select value ------
typedef enum { TYPE_SELECT_ITD=0,TYPE_SELECT_QH=1,TYPE_SELECT_SITD=2,TYPE_SELECT_FSTN=3} NEXTLINKTYPE;
//-------------------------------
typedef union {
    volatile LPContext lpContext;
    volatile DWORD     dwLinkPointer;
} NextLinkPointer;

//--------Type for ITD----------------------------
typedef struct {
    DWORD TransationOffset:12;
    DWORD PageSelect:3;
    DWORD InterruptOnComplete:1;
    DWORD TransactionLength:12;
    DWORD XactErr:1;
    DWORD BabbleDetected:1;
    DWORD DataBufferError:1;
    DWORD Active:1;
} ITD_SCContext;
typedef union {
    ITD_SCContext iTD_SCContext;
    DWORD         dwITD_StatusControl;
}ITD_StatusControl;


typedef struct {
    DWORD DeviceAddress:7;
    DWORD Reserved:1;
    DWORD EndPointNumber:4;
    DWORD BufferPointer:20;
} ITD_BPPContext1;
typedef struct {
    DWORD MaxPacketSize:11;
    DWORD Direction:1;
    DWORD BufferPointer:20;    
} ITD_BPPContext2;
typedef struct {
    DWORD Multi:2;
    DWORD Reserved:10;
    DWORD BufferPointer:20;    
} ITD_BPPContext3;
typedef union {
    ITD_BPPContext1 iTD_BPPContext1;
    ITD_BPPContext2 iTD_BPPContext2;
    ITD_BPPContext3 iTD_BPPContext3;
    DWORD           dwITD_BufferPagePointer;
} ITD_BufferPagePointer;


typedef struct {
    //NextLinkPointer         nextLinkPointer;
    volatile ITD_StatusControl       iTD_StatusControl[8];
    volatile ITD_BufferPagePointer   iTD_BufferPagePointer[7];
} ITD;


//--------Type for SITD----------------------------
typedef struct {
    DWORD DeviceAddress:7;
    DWORD Reserved:1;
    DWORD Endpt:4;
    DWORD Reserved2:4;
    DWORD HubAddress:7;
    DWORD Reserved3:1;
    DWORD PortNumber:7;
    DWORD Direction:1;
}SITD_CCContext;
typedef union {
    SITD_CCContext sITD_CCContext;
    DWORD dwSITD_CapChar;
} SITD_CapChar;

typedef struct {
    DWORD SplitStartMask:8;
    DWORD SplitCompletionMask:8;
    DWORD Reserved:16;
} SITD_MFSCContext;
typedef union  {
    SITD_MFSCContext sITD_MFSCContext;
    DWORD dwMicroFrameSchCtrl;
} MicroFrameSchCtrl;

typedef struct {
    DWORD Reserved:1;
    DWORD SlitXstate:1;
    DWORD MissedMicroFrame:1;
    DWORD XactErr:1;
    DWORD BabbleDetected:1;
    DWORD DataBufferError:1;
    DWORD ERR:1;
    DWORD Active:1;
    DWORD C_Prog_Mask:8;
    DWORD BytesToTransfer:10;
    DWORD Resevered1:4;
    DWORD PageSelect:1;
    DWORD IOC:1;
} SITD_TSContext;
typedef union {
    SITD_TSContext sITD_TSContext;
    DWORD dwSITD_TransferState;
} SITD_TransferState;

typedef struct {
    DWORD CurrentOffset:12;
    DWORD BufferPointer:20;
}SITD_BPPage0;
typedef struct {
    DWORD T_Count:3;
    DWORD TP:2;
    DWORD Reserved:7;
    DWORD BufferPointer:20;
}SITD_BPPage1;
typedef union {
    SITD_BPPage0    sITD_BPPage0;
    SITD_BPPage1    sITD_BPPage1;
    DWORD           dwSITD_BPPage;
}SITD_BPPage;

typedef struct {
    //NextLinkPointer         nextLinkPointer;
    volatile SITD_CapChar            sITD_CapChar;
    volatile MicroFrameSchCtrl       microFrameSchCtrl;
    volatile SITD_TransferState      sITD_TransferState;
    volatile SITD_BPPage             sITD_BPPage[2];
    volatile NextLinkPointer         backPointer;
} SITD;


//--------Type for QTD----------------------------
typedef union {
    LPContext lpContext;
    DWORD     dwLinkPointer;
} NextQTDPointer;
typedef struct {
    DWORD PingState:1;
    DWORD SplitXState:1;
    DWORD MisseduFrame:1;
    DWORD XactErr:1;
    DWORD BabbleDetected:1;
    DWORD DataBufferError:1;
    DWORD Halted:1;
    DWORD Active:1;
    DWORD PID:2;
    DWORD CEER:2;
    DWORD C_Page:3;
    DWORD IOC:1;
    DWORD BytesToTransfer:15;
    DWORD DataToggle:1;
} QTD_TConetext;
typedef union {
    QTD_TConetext   qTD_TContext;
    DWORD           dwQTD_Token;
}QTD_Token;

typedef struct {
    DWORD CurrentOffset:12;
    DWORD BufferPointer:20;
}QTD_BPContext;
typedef union {
    QTD_BPContext   qTD_BPContext;
    DWORD           dwQTD_BufferPointer;
} QTD_BufferPointer;
typedef struct {
    //NextQTDPointer      nextQTDPointer;
    volatile NextQTDPointer      altNextQTDPointer;
    volatile QTD_Token           qTD_Token;
    volatile QTD_BufferPointer   qTD_BufferPointer[5];
} QTD;
//--------Type for QH----------------------------
typedef struct {
// DWORD 1
    DWORD DeviceAddress:7;
    DWORD I:1;
    DWORD Endpt:4;
    DWORD ESP:2;
    DWORD DTC:1;
    DWORD H:1;
    DWORD MaxPacketLength:11;
    DWORD C:1;
    DWORD RL:4;
// DWORD 2
    DWORD UFrameSMask:8;
    DWORD UFrameCMask:8;
    DWORD HubAddr:7;
    DWORD PortNumber:7;
    DWORD Mult:2;
} QH_SESContext;
typedef union {
  QH_SESContext     qH_SESContext;
  DWORD             qH_StaticEndptState[2];
}QH_StaticEndptState;

typedef struct {
    //NextLinkPointer     qH_HorLinkPointer;
    volatile QH_StaticEndptState qH_StaticEndptState;
    volatile NextQTDPointer      currntQTDPointer;
    volatile NextQTDPointer      nextQTDPointer;
    volatile QTD                 qTD_Overlay;
} QH;
//--------Type for FSTN----------------------------
typedef struct {
    NextLinkPointer normalPathLinkPointer;
    NextLinkPointer backPathLinkPointer;
} FSTN;
#define MHCI_PAGE_SIZE 0x1000
#define MHCI_PAGE_ADDR_SHIFT 12
#define MHCI_PAGE_ADDR_MASK  0xFFFFF000
#define MHCI_PAGE_OFFSET_MASK 0xFFF

#define DEFAULT_BLOCK_SIZE MHCI_PAGE_SIZE
#define MAX_PHYSICAL_BLOCK 7

typedef struct {
    DWORD dwNumOfBlock;
    DWORD dwBlockSize;
    DWORD dwStartOffset;
    DWORD dwArrayBlockAddr[MAX_PHYSICAL_BLOCK];
} PhysBufferArray,*PPhysBufferArray;

    
#endif


