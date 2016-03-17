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
#ifndef __SDMAPXCLIENT_H__
#define __SDMAPXCLIENT_H__

#include <windows.h>
#include <omap35xx.h>
#include <dma_utility.h>

class SdmaPxClient
{
public:
    SdmaPxClient();
    virtual ~SdmaPxClient();
    // // //
    BOOL  Init(DWORD dwClientID);
    BOOL  StoreConfig(DmaConfigInfo_t *pdmaDataConfig, DWORD dwDataLengthClient, DWORD dwElementCount, DWORD dwFrameCount);
    DmaConfigInfo_t GetConfig(void) { return m_dmaConfigInfoClient; }
    DWORD GetDataLength(void) { return m_dwDataLengthClient; }
    DWORD GetElementCount(void) { return m_dwElementCount; }
    DWORD GetFrameCount(void) { return m_dwFrameCount; }
    // // //
    DWORD GetID(void) { return m_dwClientID; }
    void ResetID(void) { m_dwClientID = -1; return; }
    // // //
    BOOL  IsClientConfigured(void) { return m_bConfigured; }

private:
    DWORD m_dwClientID;
    DWORD m_dwDataLengthClient;
    DWORD m_dwElementCount;
    DWORD m_dwFrameCount;
    DmaConfigInfo_t m_dmaConfigInfoClient;
    // // //
    BOOL m_bConfigured;
};

#endif /* sdmapxclient.h */
