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
//     USB2Lib.h
// 
// Abstract: Provides interface to UHCI host controller
// 
// Notes: 
//

#ifndef __USB2LIB_H_
#define __USB2LIB_H_
#include <sync.hpp>
// all times are in units of bytes
#define FS_BYTES_PER_MICROFRAME 188
#define MICROFRAMES_PER_FRAME   8
#define FS_SOF 6  // number of byte times allocated to an SOF packet at the beginning of a frame
//#define   MAXFRAMES   8   // scheduling window for budget tracking, periods longer than
#define MAXFRAMES   32  // scheduling window for budget tracking, periods longer than
                // this are reduced to this.  Also impacts space required for
                // tracking data structures.  Otherwise fairly arbitrary.

#define MAXMICROFRAMES  (MAXFRAMES * 8) 

// 4 byte sync, 4 byte split token, 1 byte EOP, 11 byte ipg, plus
// 4 byte sync, 3 byte regular token, 1 byte EOP, 11 byte ipg
#define HS_SPLIT_SAME_OVERHEAD 39
// 4 byte sync, 4 byte split token, 1 byte EOP, 11 byte ipg, plus
// 4 byte sync, 3 byte regular token, 1 byte EOP, 1 byte bus turn
#define HS_SPLIT_TURN_OVERHEAD 29
// 4 byte sync, 1 byte PID, 2 bytes CRC16, 1 byte EOP, 11 byte ipg
#define HS_DATA_SAME_OVERHEAD 19
// 4 byte sync, 1 byte PID, 2 bytes CRC16, 1 byte EOP, 1 byte bus turn
#define HS_DATA_TURN_OVERHEAD 9
// 4 byte sync, 1 byte PID, 1 byte EOP, 1 byte bus turn
#define HS_HANDSHAKE_OVERHEAD 7
//#define HS_MAX_PERIODIC_ALLOCATION    6000    // floor(0.8*7500)
#define HS_MAX_PERIODIC_ALLOCATION  7000    // floor(0.8*7500)

// This could actually be a variable based on an HC implementation
// some measurements have shown 3?us between transactions or about 3% of a microframe
// which is about 200+ byte times.  We'll use about half that for budgeting purposes.
#define HS_HC_THINK_TIME 100

// 4 byte sync, 3 byte regular token, 1 byte EOP, 11 byte ipg
#define HS_TOKEN_SAME_OVERHEAD 19
// 4 byte sync, 3 byte regular token, 1 byte EOP, 1 byte bus turn
#define HS_TOKEN_TURN_OVERHEAD 9

// TOKEN: 1 byte sync, 3 byte token, 3 bit EOP, 1 byte ipg
// DATA: 1 byte sync, 1 byte PID, 2 bytes CRC16, 3 bit EOP, 1 byte ipg
// HANDSHAKE: 1 byte sync, 1 byte PID, 3 bit EOP, 1 byte ipg
#define FS_ISOCH_OVERHEAD 9
#define FS_INT_OVERHEAD 13
//#define LS_INT_OVERHEAD (19*8)
#define LS_INT_OVERHEAD ((14 *8) + 5)
#define HUB_FS_ADJ 30 // periodic allocation at beginning of frame for use by hubs, maximum allowable is 60 bytes
#define FS_MAX_PERIODIC_ALLOCATION  (1157)  // floor(0.9*1500/1.16)
#define FS_BS_MAX_PERIODIC_ALLOCATION 1350 // floor(0.9*1500), includes bitstuffing allowance (for HC classic allocation)

// byte time to qualify as a large FS isoch transaction
//   673 = 1023/1.16 (i.e. 881) - 1microframe (188) - adj (30) or
//   1/2 of max allocation in this case 
// #define LARGEXACT (881-FS_BYTES_PER_MICROFRAME)
#define LARGEXACT (579)

typedef enum {bulk, control, interrupt, isoch} eptype;

#define HSSPEED 2
#define FSSPEED 1
#define LSSPEED 0
#define INDIR 0
#define OUTDIR 1
typedef struct _EndpointBuget
{

    // These fields have static information that is valid/constant as long as an
    // endpoint is configured
    USHORT max_packet;  // maximum number of data bytes allowed for this
                                // endpoint. 0-8 for LS_int, 0-64 for FS_int,
                                // 0-1023 for FS_isoch.
    USHORT  period;       // desired period of transactions, assumed to be a power of 2
    eptype      ep_type;
    UCHAR  type;
    UCHAR   direction;
    UCHAR   speed;
    UCHAR   moved_this_req; // 1 when this endpoint has been changed during this allocation request

    // These fields hold dynamically calculated information that changes as (other)
    // endpoints are added/removed.

    USHORT  calc_bus_time;  // bytes of FS/LS bus time this endpoint requires
                                // including overhead. This can be calculated once.

    USHORT  start_time;     // classic bus time at which this endpoint is budgeted to occupy the classic bus

    USHORT actual_period;   // requested period can be modified:
                                // 1. when period is greater than scheduling window (MAXFRAMES)
                                // 2. if period is reduced (not currently done by algorithm)

    UCHAR   start_frame;        // first bus frame that is allocated to this endpoint.
    CHAR    start_microframe;       // first bus microframe (in a frame) that can have a
                                // start-split for this ep.
                                // Complete-splits always start 2 microframes after a
                                // start-split.
    UCHAR   num_starts;     // the number of start splits.
    UCHAR   num_completes;  // the number of complete splits.
    /* The numbers above could be (better?) represented as bitmasks. */

    /* corner conditions above: coder beware!!
       patterns can have the last CS in the "next" frame
         This is indicated in this design when:
        (start_microframe + num_completes + 1) > 7
       patterns can have the first SS in the previous frame
             This is indicated in this design when:
                start_microframe = -1
    */


} EndpointBuget,*LPEndpointBuget;
typedef struct _microframe_rec
{
    DWORD   time_used;
} MICROFRAME_REC,FRAME_REC;

typedef struct _ENDPOINTBUGETLIST *LPEndpointBugetList;
typedef struct _ENDPOINTBUGETLIST {
    LPEndpointBugetList pNextEndpt;
    EndpointBuget endpt;    
}EndpointBugetList;

class TransactionTrasnlate;
class TransactionTrasnlate {
public:
    TransactionTrasnlate (const UCHAR uHubAddress,const UCHAR uPort,const PVOID ttContext,TransactionTrasnlate * pNextTT=NULL);
    ~TransactionTrasnlate();
    UCHAR GetHubAddress() { return m_uHubAddress; };
    UCHAR GetHubPort() { return m_uPort; };
    PVOID GetContext() { return m_ttContext; };
    BOOL AddedEp(LPEndpointBuget lEp);
    BOOL DeletedEp(LPEndpointBuget lEp);
    TransactionTrasnlate * SetNextTT(TransactionTrasnlate * pNextTT) { 
        TransactionTrasnlate * pReturn = m_pNextTT;
        m_pNextTT = pNextTT;
        return  pReturn;
    }
    TransactionTrasnlate * GetNextTT() { return  m_pNextTT; };
private:
    BOOL FindBestTimeSlot(USHORT start_time,USHORT time_duration,LPEndpointBugetList lpList,PDWORD pdwReturn);
    BOOL InsertEp(DWORD frameIndex,LPEndpointBuget ep);
    BOOL RemoveEp(DWORD frameIndex,LPEndpointBuget ep);
    const UCHAR m_uHubAddress;
    const UCHAR m_uPort;  
    const PVOID m_ttContext;
    DWORD m_dwThink_time;
    TransactionTrasnlate * m_pNextTT;
    LPEndpointBugetList TT_frame[MAXFRAMES];
    FRAME_REC frame_budget[MAXFRAMES];
};


class USB2lib :LockObject {
public:
    USB2lib() ;
    ~USB2lib();
    BOOL    Init(void) { return TRUE; };
    BOOL   AddedTt( UCHAR uHubAddress,UCHAR uPort);
    BOOL    DeleteTt( UCHAR uHubAddress,UCHAR uPort,BOOL ttContext);
    TransactionTrasnlate * GetTT( const UCHAR uHubAddress,const UCHAR uHubPort,BOOL ttContext);
    BOOL    AllocUsb2BusTime(IN const UCHAR uHubAddress,IN const UCHAR uHubPort,BOOL ttContext, LPEndpointBuget lpEndpointDescriptor);
    void    FreeUsb2BusTime(IN const UCHAR uHubAddress,IN const UCHAR uHubPort,BOOL ttContext, LPEndpointBuget lpEndpointDescriptor);
    UCHAR   GetSMASK(LPEndpointBuget lpEndpointDescriptor);
    UCHAR   GetCMASK(LPEndpointBuget lpEndpointDescriptor);
//
    UCHAR   GetNewPeriod(LPEndpointBuget lpEndpointDescriptor) const  { return (UCHAR) lpEndpointDescriptor->actual_period;};
    ULONG   GetScheduleOffset(LPEndpointBuget lpEndpointDescriptor) const { return  lpEndpointDescriptor->start_frame; };
    
    ULONG   GetAllocedBusTime(LPEndpointBuget lpEndpointDescriptor) const {  return  lpEndpointDescriptor->calc_bus_time;};
private:
    TransactionTrasnlate * pTTRoot;
    unsigned Add_bitstuff(unsigned bus_time) const;
    int     Compute_nonsplit_overhead(LPEndpointBuget ep);
    VOID ConvertBtoHFrame(UCHAR BFrame, UCHAR BUFrame, PUCHAR HFrame, PUCHAR HUFrame);
    DWORD   thinktime;
    DWORD   allocation_limit;   // maximum allocation allowed for this HC
    DWORD   speed;              // HS or FS
    MICROFRAME_REC HS_microframe_info[MAXFRAMES][MICROFRAMES_PER_FRAME];    // HS bus time allocated to
#define SIZE_OF_CMASK 8
    static UCHAR CMASKS [SIZE_OF_CMASK];
    static LONG m_lRef;
};

#endif

