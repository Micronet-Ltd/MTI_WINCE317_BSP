/*
==============================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
==============================================================
*/
#ifndef __SDMAPX_H__
#define __SDMAPX_H__

#include <windows.h>
#include <omap35xx.h>
#include <dma_utility.h>

#include "sdmaproxy.h"
#include "sdmapxclient.h"

//
// Today, all WinCE devices have a page size of 4kB. This can be verified using value in UserKInfo[KINX_PAGESIZE] array.
// The number of elements for SDMA is 2^24 max as the count elements can only be a 24bits value. See TRM Table9-43 for OMAP3.
// Hence, the MAX pages entries can only be (2^24 / 4096) i.e. 0x1000
// 
#define MAX_PFN_ENTRIES             0x00001000

//
// Define number of authorized clients
//
#define MAX_SDMAPX_CLIENTS          0x00000010


class SdmaPx
{
public:
    SdmaPx();
    virtual ~SdmaPx();
    // // //
    BOOL  Initialize(void);
    BOOL  CleanUp(void);
    BOOL  Configure(SdmaConfig_t *pdmaDataConfig, DWORD dwClientIdx);
    DWORD Copy(LPVOID pSource, LPVOID pDestination, DWORD dwClientIdx);
    // // //
    DWORD GetNumberOfClients(void) { return m_dwNumberOfClients; } 
    DWORD IncNumberOfClients(void) { InterlockedIncrement((LPLONG)&m_dwNumberOfClients); return m_dwNumberOfClients;}
    DWORD DecNumberOfClients(void) { InterlockedDecrement((LPLONG)&m_dwNumberOfClients); return m_dwNumberOfClients;}
    // // //
    BOOL  AddSdmaPxClient(DWORD dwClientID);
    BOOL  RemoveSdmaPxClient(DWORD dwClientID);
    INT32 GetSdmaPxClientIdx(DWORD dwClientID);    

private:
    HANDLE m_hEvent;
    HANDLE m_hDmaChannel;
    //DmaConfigInfo_t m_DmaSettings;
    DmaDataInfo_t m_dmaInfo;
    //SdmaConfig_t m_dmaConfig;
    // // //
    DWORD m_rgPFNsrc[MAX_PFN_ENTRIES];
    DWORD m_rgPFNdst[MAX_PFN_ENTRIES];      
    DWORD m_pageShift;    
    DWORD m_pageSize;
    DWORD m_pageMask;
    // // //
    DWORD m_dwNumberOfClients;
    SdmaPxClient* m_SdmaPxClient;
};

#endif /* sdmapx.h */
