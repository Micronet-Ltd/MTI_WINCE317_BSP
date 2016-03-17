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
/*++


Module Name:

    usb2lib.cpp

Abstract:

    interface to usb2lib, usb2 low/full speed scheduling algorithms

Environment:

    kernel or user mode only

Notes:

Revision History:

--*/

#pragma warning(push)
#pragma warning(disable : 4510 4512 4610)
#include <windows.h>
#include "usb2lib.h"
#pragma warning(pop)

TransactionTrasnlate::TransactionTrasnlate (const UCHAR uHubAddress,const UCHAR uPort,const PVOID ttContext,TransactionTrasnlate * pNextTT)
    : m_uHubAddress(uHubAddress)
    , m_uPort(uPort)
    , m_ttContext(ttContext)
{
    m_pNextTT =  pNextTT;
    m_dwThink_time =1;
    for (DWORD dwIndex=0; dwIndex<MAXFRAMES; dwIndex++) {
        TT_frame[dwIndex]=NULL;
        frame_budget[dwIndex].time_used=0;;
    }
}
TransactionTrasnlate::~TransactionTrasnlate ()
{
    for (DWORD dwIndex=0; dwIndex<MAXFRAMES; dwIndex++) {
        LPEndpointBugetList pCurList = TT_frame[dwIndex];
        if (pCurList!=NULL) {
            LPEndpointBugetList pNextList = pCurList ->pNextEndpt;
            delete pCurList;
            pCurList = pNextList;
        }
        TT_frame[dwIndex]=NULL;
    }
};
BOOL TransactionTrasnlate::AddedEp(LPEndpointBuget ep)
{
    BOOL bReturn = TRUE;
    if (ep) {
       // split or nonsplit FS/LS speed allocation
        // classic allocation
        // split allocation
        unsigned int min_used = frame_budget[0].time_used;

        if (ep->period > MAXFRAMES)
            ep->actual_period = MAXFRAMES;
        else
            ep->actual_period = ep->period;

        // Look at all candidate frames for this period to find the one with min
        // allocated bus time.  
        //
        for (unsigned int i=1; i < ep->actual_period ; i++) {
            if (frame_budget[i].time_used < min_used) {
                min_used = frame_budget[i].time_used;
                ep->start_frame = (UCHAR)i;
            }
        }
        //***
        //*** 2. Calculate classic time required
        //***

        // Calculate classic overhead
        DWORD overhead;
        if (ep->ep_type == isoch) {
            if (ep->speed == FSSPEED)
                overhead = FS_ISOCH_OVERHEAD + m_dwThink_time;
            else {
                ASSERT(FALSE);
                return FALSE;
            }
        } 
        else  { // interrupt
            if (ep->speed == FSSPEED)
                overhead = FS_INT_OVERHEAD + m_dwThink_time;
            else
                overhead = LS_INT_OVERHEAD + m_dwThink_time;
        }

        // Classic bus time, NOT including bitstuffing overhead (in FS byte times) since we do best case budget
        ep->calc_bus_time = (USHORT)(ep->max_packet * (ep->speed!=LSSPEED?1:8) + overhead);

        USHORT latest_start = FS_SOF + HUB_FS_ADJ;  // initial start time must be after the SOF transaction

        for (i=0; ep->start_frame + i < MAXFRAMES && bReturn==TRUE; i += ep->actual_period) {
            DWORD t=0;
            if (FindBestTimeSlot(latest_start,ep->calc_bus_time,TT_frame[ep->start_frame + i],&t)) {
                ASSERT(t>=latest_start);
                // update latest start time as required
                if (t > latest_start)
                    latest_start = (USHORT)t;
            }
            else { // Runout the slot this one.
                ASSERT(FALSE);
                ep->calc_bus_time = 0;
                bReturn= FALSE;
                break;
            }
        } // end of for loop looking for latest start time

        if (bReturn==TRUE) {
            // Set the start time for the new endpoint
            ep->start_time = latest_start;

            if ((ep->start_time + ep->calc_bus_time) > FS_MAX_PERIODIC_ALLOCATION)  {
            //      error("start time %d past end of frame", ep->start_time + ep->calc_bus_time);
                ep->calc_bus_time = 0;
                return FALSE;
            }
            BOOL bRet=TRUE;
            for (i=0; ep->start_frame + i < MAXFRAMES; i += ep->actual_period) {
                bRet &= InsertEp(ep->start_frame + i, ep);
                ASSERT(bRet==TRUE);
                frame_budget[0].time_used += ep->calc_bus_time;
            }
            if (bRet == FALSE) {
                for (i=0; ep->start_frame + i < MAXFRAMES; i += ep->actual_period) {
                    RemoveEp(ep->start_frame + i, ep);
                    frame_budget[0].time_used -= ep->calc_bus_time;
                }
            }
            bReturn = bRet;
        }

    }
    ASSERT(bReturn);
    return bReturn;
}
BOOL  TransactionTrasnlate::DeletedEp(LPEndpointBuget ep)
{
    BOOL bReturn = TRUE;
    for (unsigned int i=0; ep->start_frame + i < MAXFRAMES; i += ep->actual_period) {
        if (RemoveEp(ep->start_frame + i, ep))
            frame_budget[0].time_used -= ep->calc_bus_time;
        else{
            ASSERT(FALSE);
            bReturn=FALSE;
        }
    }
    return bReturn;    
}

BOOL TransactionTrasnlate::FindBestTimeSlot(USHORT start_time,USHORT time_duration,LPEndpointBugetList lpList,PDWORD pdwReturn)
{
    while (lpList) {
        // Check this is no overlap.
        if (lpList->endpt.start_time  >= start_time +  time_duration) { // Empty Slot has been found.
            break;
        }
        else if (lpList->endpt.start_time +  lpList->endpt.calc_bus_time <= start_time) { // Have not reach yet . Continue
            lpList = lpList ->pNextEndpt;
        }
        else { // We have overlap. Let us move slot later.
            start_time = lpList->endpt.start_time +  lpList->endpt.calc_bus_time;
            lpList =  lpList ->pNextEndpt;
        }
    }
    *pdwReturn = start_time;
    return TRUE;
}
BOOL TransactionTrasnlate::InsertEp(DWORD frameIndex,LPEndpointBuget ep)
{
    if (frameIndex>=MAXFRAMES || ep == NULL) {
        ASSERT(FALSE);
        return FALSE;
    }
    BOOL bReturn = FALSE;
    EndpointBugetList * pNewEpList = new EndpointBugetList;
    if (pNewEpList) {
        pNewEpList->endpt = *ep;
        // find out where is not
        EndpointBugetList * pPrevNode=NULL;
        EndpointBugetList * pCurNode = TT_frame[frameIndex];
        bReturn = TRUE;
        while (pCurNode) {
            if (pCurNode->endpt.start_time +  pCurNode->endpt.calc_bus_time <= ep->start_time) { // Continue
                pPrevNode = pCurNode;
                pCurNode =pCurNode->pNextEndpt;
            }
            else if (pCurNode->endpt.start_time >= ep->start_time+ep->calc_bus_time) { // Find hole
                bReturn=TRUE;
                break;
            }
            else {// This is really bad. Someone try to inserted something that has overlap.
                ASSERT(FALSE);
                bReturn = FALSE;
                break;
            }
        }
        if (bReturn) { // We reached last one.
            if (pPrevNode) { // Not first.
                pNewEpList->pNextEndpt = pPrevNode ->pNextEndpt;
                pPrevNode->pNextEndpt = pNewEpList;
            } else { // This is first.
                pNewEpList->pNextEndpt = TT_frame[frameIndex];
                TT_frame[frameIndex] = pNewEpList;
            }
        }
        else // Fails we have to clean it.
            delete pNewEpList;
    }
    return bReturn;
};
BOOL TransactionTrasnlate::RemoveEp(DWORD frameIndex,LPEndpointBuget ep)
{
    if (frameIndex>=MAXFRAMES || ep == NULL) {
        ASSERT(FALSE);
        return FALSE;
    }
    BOOL bReturn=FALSE;
    EndpointBugetList * pPrevNode=NULL;
    EndpointBugetList * pCurNode = TT_frame[frameIndex];
    while (pCurNode) {
        if (pCurNode->endpt.start_time +  pCurNode->endpt.calc_bus_time <= ep->start_time) { // Continue
            pPrevNode = pCurNode;
            pCurNode =pCurNode->pNextEndpt;
        }
        else if ( pCurNode->endpt.start_time == ep->start_time && pCurNode->endpt.calc_bus_time == ep->calc_bus_time) {
            bReturn=TRUE;
            break;
        }
        else { // Either overlap or behind, We can not find this one.
            bReturn=FALSE;
            break;            
        }
    };
    if (bReturn == TRUE && pCurNode!=NULL) {
        if (pPrevNode) // Not first one.
            pPrevNode->pNextEndpt=pCurNode->pNextEndpt;
        else
            TT_frame[frameIndex]= pCurNode->pNextEndpt;
        delete pCurNode;
    }
    else
        ASSERT(FALSE);
    return bReturn;

}

USB2lib::USB2lib()
{
    // allocate at TT to test with
    //myHC.tthead = (PTT) malloc(sizeof(TT));
    Lock();
    pTTRoot = NULL;
    thinktime = HS_HC_THINK_TIME;
    allocation_limit = HS_MAX_PERIODIC_ALLOCATION;
    speed = HSSPEED;

    for (int i=0; i<MAXFRAMES; i++) {
        for (int j=0; j < MICROFRAMES_PER_FRAME; j++) {
            HS_microframe_info[i][j].time_used = 0;
        }
    }
    Unlock();
}
USB2lib::~USB2lib()
{
    Lock();
    TransactionTrasnlate * pCurTT= pTTRoot;
    while (pCurTT) {
        TransactionTrasnlate * pNextTT = pCurTT->GetNextTT();
        delete pCurTT;
        pCurTT = pNextTT;
    }
    Unlock();
}
LONG USB2lib::m_lRef = 0;
BOOL USB2lib::AddedTt( UCHAR uHubAddress,UCHAR uPort)
{
    BOOL bReturn = FALSE;
    Lock();
    if (GetTT( uHubAddress,uPort, TRUE) == NULL) {//Verify
        TransactionTrasnlate * pNewTT = new TransactionTrasnlate(uHubAddress,uPort, pTTRoot);
        if ( pNewTT) {
            pTTRoot=pNewTT;
            bReturn=TRUE;
        }
    }
    else
        bReturn=TRUE;
    Unlock();
    return bReturn;
        
}
BOOL USB2lib::DeleteTt( UCHAR uHubAddress,UCHAR uPort, BOOL ttContext)
{
	UNREFERENCED_PARAMETER(ttContext);
	
	BOOL bReturn = FALSE;
    Lock();
    TransactionTrasnlate * pPrevTT= NULL;
    TransactionTrasnlate * pCurTT = pTTRoot;

    while ( pCurTT!=NULL ) {
        if (pCurTT->GetHubAddress()==uHubAddress && pCurTT->GetHubPort() == uPort)
            break;
        else {
            pPrevTT = pCurTT;
            pCurTT = pCurTT->GetNextTT();
        }
    }
    if (pCurTT) { // We found one matched.
        if (pPrevTT) { // Not First One.
            pPrevTT ->SetNextTT(pCurTT->GetNextTT());
            delete pCurTT;
        }
        else { // First one
            pTTRoot = pCurTT->GetNextTT();
            delete pCurTT;
        }
        bReturn=TRUE;
     }
    Unlock();
    return bReturn;
}
TransactionTrasnlate * USB2lib::GetTT( const UCHAR uHubAddress,const UCHAR uHubPort, const BOOL pContext )
{
	UNREFERENCED_PARAMETER(pContext);

    Lock();
    TransactionTrasnlate * pFoundTT = pTTRoot;
    while (pFoundTT) {
        if (pFoundTT->GetHubAddress() == uHubAddress && pFoundTT->GetHubPort() == uHubPort)
            break;
        else
            pFoundTT = pFoundTT->GetNextTT();
    }
    Unlock();
    return pFoundTT;
}

unsigned USB2lib::Add_bitstuff(unsigned bus_time) const
{
    // Bit stuffing is 16.6666% extra.
    // But we'll calculate bitstuffing as 16% extra with an add of a 4bit
    // shift (i.e.  value + value/16) to avoid floats.
    return (bus_time + (bus_time>>4));
}


/*******

    Compute_nonsplit_overhead

********/
int USB2lib::Compute_nonsplit_overhead(LPEndpointBuget ep)
{
    if (ep->speed == HSSPEED)
    {
        if (ep->direction == OUTDIR)
        {
            if (ep->ep_type == isoch)
            {
                return HS_TOKEN_SAME_OVERHEAD + HS_DATA_SAME_OVERHEAD + thinktime;
            } else // interrupt
            {
                return HS_TOKEN_SAME_OVERHEAD + HS_DATA_SAME_OVERHEAD +
                    HS_HANDSHAKE_OVERHEAD + thinktime;
            }
        } else
        { // IN
            if (ep->ep_type == isoch)
            {
                return HS_TOKEN_TURN_OVERHEAD + HS_DATA_TURN_OVERHEAD + thinktime;
                
            } else // interrupt
            {
                return HS_TOKEN_TURN_OVERHEAD + HS_DATA_TURN_OVERHEAD +
                    HS_HANDSHAKE_OVERHEAD + thinktime;
            }
        }  // end of IN overhead calculations
    } else  if (ep->speed == FSSPEED)
    {
        if (ep->ep_type == isoch)
        {
            return FS_ISOCH_OVERHEAD + thinktime;
        } else // interrupt
        {
            return FS_INT_OVERHEAD + thinktime;
        }
    } else  // LS
    {
        return LS_INT_OVERHEAD + thinktime;
    }
}

int USB2lib::AllocUsb2BusTime( IN const UCHAR uHubAddress,IN const UCHAR uHubPort,const BOOL ttContext, LPEndpointBuget ep)
{
    int retv;
    unsigned   changed_eps, i, min_used;
    //PEndpoint curr_ep, last_ep, p;

    changed_eps = 0;

    retv = 1;
    Lock();
    // OVERVIEW of algorithm steps:
    //  1. Determine starting frame # for period
    //  2. Calculate classic time required
    //  3. For all period frames, find the latest starting time so we can check the classic allocation later
    //  4. Process each frame data structure for endpoint period in budget window
    //  5.   Now check allocation for each frame using shift adjustment based on latest start time
    //  6a.  Now move isoch endpoints, insert new isoch and then move interrupt endpoints
    //  6b.  Now insert new interrupt and move rest of interrupt endpoints
    //  7.   Allocate HS bus time
    //  8.   Allocate classic bus time


    //***
    //*** 1. Determine starting frame # for period
    //***



    // Also remember the maximum frame time allocation since it will be used to pass the allocation check.

    // Find starting frame number for reasonable balance of all classic frames

    ep->start_frame = 0;
    ep->start_microframe = 0;
    ep->num_completes = 0;
    ep->num_starts = 0;

    // check that this endpoint isn't already allocated
    if (ep->calc_bus_time)
    {
        DEBUGMSG(1,(TEXT("endpoint already allocated\r\n")));
        ASSERT(FALSE);
        Unlock();
        return 0;
    }

    // handle nonsplit HS allocation
    if (ep->speed == HSSPEED) {

        min_used = HS_microframe_info[0][0].time_used;

        if (ep->period > MAXFRAMES*MICROFRAMES_PER_FRAME)
            ep->actual_period = MAXFRAMES*MICROFRAMES_PER_FRAME;
        else
            ep->actual_period = ep->period;

        // Look at all candidate frames for this period to find the one with min
        // allocated bus time.  
        //
        for (i=1; i < ep->actual_period; i++){
            if (HS_microframe_info[i/MICROFRAMES_PER_FRAME][i % MICROFRAMES_PER_FRAME].time_used < min_used)
            {
                min_used = HS_microframe_info[i/MICROFRAMES_PER_FRAME][i % MICROFRAMES_PER_FRAME].time_used;
                ep->start_frame = (UCHAR)(i / MICROFRAMES_PER_FRAME);
                ep->start_microframe = (UCHAR)(i % MICROFRAMES_PER_FRAME);
            }
        }

        // compute and allocate HS bandwidth
        ep->calc_bus_time = (USHORT)(Compute_nonsplit_overhead(ep) + Add_bitstuff(ep->max_packet));
        for (i = (ep->start_frame*MICROFRAMES_PER_FRAME) + ep->start_microframe;
                i < MAXFRAMES*MICROFRAMES_PER_FRAME;
                i += ep->actual_period){
            HS_microframe_info[i/MICROFRAMES_PER_FRAME][i % MICROFRAMES_PER_FRAME].time_used +=ep->calc_bus_time;
            if (HS_microframe_info[i/MICROFRAMES_PER_FRAME][i % MICROFRAMES_PER_FRAME].time_used >HS_MAX_PERIODIC_ALLOCATION)
                retv = 0;
        }
        if (! retv)  // if allocation failed, deallocate
        {
            for (i = (ep->start_frame*MICROFRAMES_PER_FRAME) + ep->start_microframe;
                    i < MAXFRAMES*MICROFRAMES_PER_FRAME;
                    i += ep->actual_period){
                HS_microframe_info[i/MICROFRAMES_PER_FRAME][i % MICROFRAMES_PER_FRAME].time_used -= ep->calc_bus_time;
            }
        }
        Unlock();
        return retv;
    }
    
 
    // above handles all speeds, the rest of this code is for split transaction processing

    //***
    //*** 2. Calculate classic time required
    //***
    TransactionTrasnlate * pTT=GetTT( uHubAddress,uHubPort,ttContext);
    if (pTT== NULL || pTT->AddedEp(ep)!=TRUE) {
        Unlock();
        return 0;
    }
    
    if ((ep->start_time + ep->calc_bus_time) > FS_MAX_PERIODIC_ALLOCATION)
    {
//      error("start time %d past end of frame", ep->start_time + ep->calc_bus_time);
        ep->calc_bus_time = 0;
        Unlock();
        return 0;
    }

    ep->start_microframe = (CHAR)((ep->start_time /  FS_BYTES_PER_MICROFRAME) - 1);
    UCHAR lastcs = (UCHAR)(( (ep->start_time + ep->calc_bus_time) / FS_BYTES_PER_MICROFRAME) + 1);

    // determine number of splits (starts and completes)
    if (ep->direction == OUTDIR) {
        if (ep->ep_type == isoch)
        {
            ep->num_starts = (UCHAR)((ep->max_packet / FS_BYTES_PER_MICROFRAME) + 1);
            ep->num_completes = 0;
        } else // interrupt
        {
            ep->num_starts = 1;
            ep->num_completes = 2;
            if (ep->start_microframe + 1 < 6)
                ep->num_completes++;
        }
    } else { // IN
        if (ep->ep_type == isoch) {
            ep->num_starts = 1;
            ep->num_completes = lastcs - (ep->start_microframe + 1);
            if (lastcs <= 6)
            {
                if ((ep->start_microframe + 1) == 0)
                    ep->num_completes++;
                else
                    ep->num_completes += 2;  // this can cause one CS to be in the next frame
            }
            else if (lastcs == 7)
            {
                if ((ep->start_microframe + 1) != 0)
                    ep->num_completes++;  // only one more CS if late in the frame.
            }

        } else // interrupt
        {
            ep->num_starts = 1;
            ep->num_completes = 2;
            if (ep->start_microframe + 1 < 6)
                ep->num_completes++;
        }
    }  // end of IN
    Unlock();
    return retv;
}

void USB2lib::FreeUsb2BusTime(IN const UCHAR uHubAddress,IN const UCHAR uHubPort,const BOOL ttContext, LPEndpointBuget ep)
{
    Lock();
    if (ep && ep->calc_bus_time!=0){
    // handle nonsplit HS allocation
        if (ep->speed == HSSPEED) {
            for (DWORD i = (ep->start_frame*MICROFRAMES_PER_FRAME) + ep->start_microframe;
                   i < MAXFRAMES*MICROFRAMES_PER_FRAME;
                   i += ep->actual_period){
                if (HS_microframe_info[i/MICROFRAMES_PER_FRAME][i % MICROFRAMES_PER_FRAME].time_used >= ep->calc_bus_time)
                    HS_microframe_info[i/MICROFRAMES_PER_FRAME][i % MICROFRAMES_PER_FRAME].time_used -= ep->calc_bus_time;
                else {
                    ASSERT(FALSE);
                    HS_microframe_info[i/MICROFRAMES_PER_FRAME][i % MICROFRAMES_PER_FRAME].time_used =0;
                }
            }
        }
        else {
            TransactionTrasnlate * pTT=GetTT( uHubAddress,uHubPort,ttContext);
            if (pTT!=NULL)
                pTT->DeletedEp(ep);
        }
     }
    Unlock();
}


UCHAR USB2lib::GetSMASK(LPEndpointBuget Ep)
{
    UCHAR       tmp = 0;


    if(Ep->speed == HSSPEED) {
//DBGPRINT(("in GetSMASK StartUFrame on High Speed Endpoint = 0x%x\n", Ep->start_microframe));
        tmp |= 1 << Ep->start_microframe;
    } else {
        ULONG       ilop;
        UCHAR       HFrame;         // H (Host) frame for endpoint
        UCHAR       HUFrame;        // H (Host) micro frame for endpoint
        // For Full and Low Speed Endpoints 
        // the budgeter returns a bframe. Convert to HUFrame to get SMASK
        ConvertBtoHFrame((UCHAR)Ep->start_frame, (UCHAR)Ep->start_microframe, &HFrame, &HUFrame);

        for(ilop = 0; ilop < Ep->num_starts; ilop++) {
            tmp |= 1 << HUFrame++;
        }
    }

    return tmp;
};
UCHAR USB2lib::CMASKS[SIZE_OF_CMASK] = 
{       0x1c,       // Start HUFRAME 0  
        0x38,       // Start HUFRAME 1
        0x70,       // Start HUFRAME 2
        0xE0,       // Start HUFRAME 3
        0xC1,       // Start HUFRAME 4
        0x83,       // Start HUFRAME 5
        0x07,       // Start HUFRAME 6
        0x0E        // Start HUFRAME 7
};


UCHAR
USB2lib::GetCMASK(LPEndpointBuget Ep)
{

    if(Ep->speed == HSSPEED) {
        return 0;
    } else if(Ep->ep_type == interrupt) {
        UCHAR       HFrame;         // H (Host) frame for endpoint
        UCHAR       HUFrame;        // H (Host) micro frame for endpoint

        ConvertBtoHFrame((UCHAR)Ep->start_frame, (UCHAR)Ep->start_microframe, 
            &HFrame, &HUFrame);

        return CMASKS[HUFrame];
    } else {
        // Split ISO!
        UCHAR       HFrame;         // H (Host) frame for endpoint
        UCHAR       HUFrame;        // H (Host) micro frame for endpoint
        UCHAR       tmp = 0;
        ULONG       NumCompletes;

        if(Ep->direction == OUTDIR) {
            // Split iso out -- NO complete splits
            return 0;
        }
        ConvertBtoHFrame((UCHAR)Ep->start_frame, (UCHAR)Ep->start_microframe, 
            &HFrame, &HUFrame);

        HUFrame += 2;  
        NumCompletes = Ep->num_completes;

        //      ASSERT(NumCompletes > 0);

        //
        //  Set all CMASKS bits to be set at the end of the frame
        // 
        for(;  HUFrame < 8; HUFrame++) {
            tmp |= 1 <<  HUFrame;
            NumCompletes--; 
            if(!NumCompletes){
                break;
            }
        }

        //
        // Now set all CMASKS bits to be set at the end of the 
        // frame I.E. for the next frame wrap condition
        // 
        while(NumCompletes) {
            tmp |= 1 << (HUFrame - 8); 
            NumCompletes--;
        }

//DBGPRINT(("in GetCMASK HFRAME = 0x%x HUFRAME 0x%x\n", HFrame, HUFrame));
        return tmp;
    }
}
VOID 
USB2lib::ConvertBtoHFrame(UCHAR BFrame, UCHAR BUFrame, PUCHAR HFrame, PUCHAR HUFrame)
{
    // The budgeter returns funky values that we have to convert to something
    // that the host controller understands.
    // If bus micro frame is -1, that means that the start split is scheduled 
    // in the last microframe of the previous bus frame.
    // to convert to hframes, you simply change the microframe to 0 and 
    // keep the bus frame (see one of the tables in the host controller spec 
    // eg 4-17.
    if(BUFrame == 0xFF) {
        *HUFrame = 0;
        *HFrame = BFrame;
    }
        
    // if the budgeter returns a value in the range from 0-6
    // we simply add one to the bus micro frame to get the host 
    // microframe
    if(BUFrame >= 0 && BUFrame <= 6) {
        *HUFrame = BUFrame + 1;
        *HFrame = BFrame;
    }

    // if the budgeter returns a value of 7 for the bframe
    // then the HUframe = 0 and the HUframe = buframe +1
    if(BUFrame == 7) {
        *HUFrame = 0;
        *HFrame = BFrame + 1;
    }
}

