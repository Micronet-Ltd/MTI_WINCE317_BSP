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
#include "sdmapx.h"
#include "sdmapxclient.h"

// If not received within 1s the DMA interrupt, consider failure.
// 1s arbitrary value.
#define DMA_IRQ_TIMEOUT             1000

// Cache helpers
#define SOURCE_CACHED               (1 << 0)
#define DESTINATION_CACHED          (1 << 1)
#define ISUNCACHEDADDRESS(x)        (((UINT)x & 0x20000000) != 0)

//
// Flush cache helper function
//
__inline
void
FlushCache(
    void* pSrc, 
    void* pDst, 
    int size,
    UINT ffCache
    )
{
    CacheRangeFlush(NULL, 0, 
        ffCache == SOURCE_CACHED ? CACHE_SYNC_WRITEBACK : CACHE_SYNC_DISCARD
        );
}

SdmaPx::SdmaPx()
{
    m_hEvent = NULL;
    m_hDmaChannel = NULL;
    
    // Reset SDMA internal settings.   
    //memset(&m_DmaSettings, 0, sizeof(DmaConfigInfo_t));    
    //m_DmaSettings.dmaPrio       = DMA_PRIORITY;
    //m_DmaSettings.interrupts    = DMA_CICR_BLOCK_IE;
    
    // Reset SDMA user config.
    //memset(&m_dmaConfig, 0, sizeof(SdmaConfig_t));
        
    m_pageShift = UserKInfo[KINX_PFN_SHIFT];    
    m_pageSize = UserKInfo[KINX_PAGESIZE];
    m_pageMask = m_pageSize - 1;
    
    // Reset number of clients
    m_dwNumberOfClients = 0;
    
    m_SdmaPxClient = NULL;
}

SdmaPx::~SdmaPx()
{
    if(m_SdmaPxClient != NULL)
    {
        delete [] m_SdmaPxClient;
    }
}

BOOL SdmaPx::Initialize(void)
{
    BOOL retVal = FALSE;
    
    m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if( (m_hEvent == NULL) || (INVALID_HANDLE_VALUE == m_hEvent) )
    {
        ERRORMSG(ZONE_ERROR, (TEXT("Failed to create the event for DMA interrupt\n")));
        goto cleanUp;
    }

    // Allocate DMA system channel
    m_hDmaChannel = DmaAllocateChannel(DMA_TYPE_SYSTEM);
    if (m_hDmaChannel == NULL)
    {
        ERRORMSG(ZONE_ERROR, (TEXT("Unable to allocate DMA channel\r\n")));
        goto cleanUp;
    }    
    
    // Register DMA for interrupts  
    if (DmaEnableInterrupts(m_hDmaChannel, m_hEvent) == FALSE)
    {
        ERRORMSG(ZONE_ERROR, (TEXT("Failed to register DMA for interrupts\r\n")));
        goto cleanUp;
    }  
    
    // Resets clients array
    m_SdmaPxClient = new SdmaPxClient [MAX_SDMAPX_CLIENTS];
    for(int i=0; i<MAX_SDMAPX_CLIENTS; i++)
    {
        m_SdmaPxClient[i].Init(-1);
    }
    
    retVal = TRUE;
cleanUp:    
    return retVal;
}

BOOL SdmaPx::AddSdmaPxClient(DWORD dwClientID)
{
    BOOL retVal = TRUE;
    DWORD dwClientIndex = GetNumberOfClients();
    
    if(dwClientIndex > MAX_SDMAPX_CLIENTS)
    {
        retVal = FALSE;
    }
    else
    {
        // Max number of authorizes clients hasn't yet been reached
        // Init specific client...
        IncNumberOfClients();
        retVal = m_SdmaPxClient[dwClientIndex].Init(dwClientID);
    }
    
    return retVal;
}

INT32 SdmaPx::GetSdmaPxClientIdx(DWORD dwClientID)
{
    INT32 retValIdx = -1; // ERROR
    
    for(int i=0; i<MAX_SDMAPX_CLIENTS; i++)
    {
        if(m_SdmaPxClient[i].GetID() == dwClientID)
        {
            // this is the index
            retValIdx = i;
            break;
        }
    }
    
    return retValIdx;
}

BOOL SdmaPx::RemoveSdmaPxClient(DWORD dwClientID)
{
    BOOL retVal = FALSE;
    
    for(int i=0; i<MAX_SDMAPX_CLIENTS; i++)
    {
        if(m_SdmaPxClient[i].GetID() == dwClientID)
        {
            // this is the index            
            m_SdmaPxClient[i].ResetID();
            DecNumberOfClients();
            retVal = TRUE;
            break;
        }
    }
    
    return retVal;
}

// Function gets the user DMA configuration information
BOOL SdmaPx::Configure(SdmaConfig_t *pdmaDataConfig, DWORD dwClientIdx)
{
    DWORD dwDataLength = 0;
    BOOL retVal = TRUE;
    DmaConfigInfo_t dmaSettings;
       
    // Check if length is < 2^24 as the count elements can only be a 24bits value. See TRM Table9-43 for OMAP3.
    // Increase the allowed size by 1, 2 or 4 times as the DMA copy uses 8bits, 16bits or 32bits elements.
    if((pdmaDataConfig->elementCount) > (DWORD)(0x01000000 << pdmaDataConfig->elementWidth))
    {
        retVal = FALSE;
        goto CleanUp;
    }
    // Same verification here for the frame count that is a 16bits value
    if((pdmaDataConfig->frameCount) > 0x00010000)
    {
        retVal = FALSE;
        goto CleanUp;
    }
    
    // Reset DMA settings structure
    memset(&dmaSettings, 0, sizeof(DmaConfigInfo_t));
    
    // Adapt user struct to internal API
    dmaSettings.elemSize      = pdmaDataConfig->elementWidth;
    dmaSettings.synchMode     = pdmaDataConfig->syncMode;
    dmaSettings.srcElemIndex  = pdmaDataConfig->srcElementIdx;
    dmaSettings.srcFrameIndex = pdmaDataConfig->srcFrameIdx;
    dmaSettings.srcAddrMode   = pdmaDataConfig->srcAddrMode;
    dmaSettings.dstElemIndex  = pdmaDataConfig->dstElementIdx;
    dmaSettings.dstFrameIndex = pdmaDataConfig->dstFrameIdx;
    dmaSettings.dstAddrMode   = pdmaDataConfig->dstAddrMode;
    dmaSettings.dmaPrio       = DMA_PRIORITY;
    dmaSettings.interrupts    = DMA_CICR_BLOCK_IE;
    
    // Data length
    dwDataLength = ( (pdmaDataConfig->elementCount ) * (pdmaDataConfig->frameCount) );
    
    //  Enable bursting for improved memory performance
    dmaSettings.elemSize |= DMA_CSDP_SRC_BURST_64BYTES_16x32_8x64 | DMA_CSDP_SRC_PACKED;
    dmaSettings.elemSize |= DMA_CSDP_DST_BURST_64BYTES_16x32_8x64 | DMA_CSDP_DST_PACKED;
    
    // Configure dma for data copy
    //retVal = DmaConfigure(m_hDmaChannel, &m_DmaSettings, 0, &m_dmaInfo); 
    m_SdmaPxClient[dwClientIdx].StoreConfig(&dmaSettings, dwDataLength, (pdmaDataConfig->elementCount ), (pdmaDataConfig->frameCount));

CleanUp:    
    return retVal;
}

BOOL SdmaPx::CleanUp(void)
{
    BOOL retVal = TRUE;
    
    DmaEnableInterrupts(m_hDmaChannel, NULL);
    
    if (m_hDmaChannel != NULL) DmaFreeChannel(m_hDmaChannel);
    if (m_hEvent != NULL) CloseHandle(m_hEvent);
    
    return retVal;
}

//
// Perform the actual DMA copy
// WARNING!! This function assumes the physical memory is contiguous.
// No check is performed for performance improvement. 
// Buffers passed-in come from CMEM and DISPLAY drivers.
//
DWORD SdmaPx::Copy(  LPVOID pSource,
                     LPVOID pDestination,
                     DWORD  dwClientIdx )
{
    DWORD dwRet = 0;    
    DWORD dwCause, dwStatus;   
    DWORD paSrc, paDst;
    BYTE ffCached = 0; 
    DmaConfigInfo_t dmaSettings = m_SdmaPxClient[dwClientIdx].GetConfig();
    DWORD dwDataLength = m_SdmaPxClient[dwClientIdx].GetDataLength();
    DWORD dwElementCount = m_SdmaPxClient[dwClientIdx].GetElementCount();
    DWORD dwFrameCount = m_SdmaPxClient[dwClientIdx].GetFrameCount();

    ASSERTMSG(L"Client must be configured before Copy I/O control is called !!!\n", m_SdmaPxClient[dwClientIdx].IsClientConfigured());
    
    // Configure SDMA for specific client
    // Need to call this first for length
    // memset() on DmaConfigInfo_t not needed as done in the SdmaPxClient::Init() method.
    // m_SdmaPxClient[dwClientIdx].GetConfig(&dmaSettings, &dwDataLength);
    if(DmaConfigure(m_hDmaChannel, &dmaSettings, 0, &m_dmaInfo) != TRUE)
    {
        ERRORMSG(ZONE_ERROR, (TEXT("ERROR! Unable to configure DMA for client\r\n")));    
        goto cleanUp;
    }
    
	// flush cache if necessary   
    if (ISUNCACHEDADDRESS(pSource) == FALSE)
    {
        ffCached |= SOURCE_CACHED;
    }    
    if (ISUNCACHEDADDRESS(pDestination) == FALSE)
    {
        ffCached |= DESTINATION_CACHED;
    } 
    if (ffCached & (SOURCE_CACHED | DESTINATION_CACHED))
    {
        FlushCache(pSource, pDestination, dwDataLength, ffCached);
    }

    // Retrieve base physical address of buffer to be copied and READ lock the pages on the length.
    if(LockPages(
                    (LPVOID)pSource,
                    (DWORD)dwDataLength,
                    m_rgPFNsrc,
                    LOCKFLAG_READ)  == FALSE)
    {
        ERRORMSG(ZONE_ERROR, (TEXT("LockPages call \"src\" failed. (error code=%d)\r\n"), GetLastError()));
        goto cleanUp;
    }
    // Not necessary to do the page shift on ARM platform as always 0. (ref. MSDN)
    // paSrc = (m_rgPFNsrc[0] << m_pageShift) + ((DWORD)pSource & m_pageMask);
    paSrc = m_rgPFNsrc[0] + ((DWORD)pSource & m_pageMask);
    
    
    // Retrieve base physical address of destination buffer and WRITE lock the pages on the length.
    if(LockPages(
                    (LPVOID)pDestination,
                    dwDataLength,
                    m_rgPFNdst,
                    LOCKFLAG_WRITE)  == FALSE)
     {
        ERRORMSG(ZONE_ERROR, (TEXT("LockPages \"dest\" call failed. (error code=%d)\r\n"), GetLastError()));
        goto cleanUp;
    }           
    // Not necessary to do the page shift on ARM platform as always 0. (ref. MSDN)
    // paDst = (m_rgPFNdst[0] << UserKInfo[KINX_PFN_SHIFT]) + ((DWORD)pDestination & m_pageMask);
    paDst = m_rgPFNdst[0] + ((DWORD)pDestination & m_pageMask);          
    
    // Configure Dest and Src buffers for DMA.   
    DmaSetSrcBuffer(&m_dmaInfo, (UINT8 *)pSource, paSrc);
    DmaSetDstBuffer(&m_dmaInfo, (UINT8 *)pDestination, paDst);
    
    // Watch out parameters in DmaSetElemenAndFrameCount. For e.g., shift right element count by 2 if you have bytes in input and you use 32bits element size.
    DmaSetElementAndFrameCount(&m_dmaInfo, dwElementCount, (UINT16)(dwFrameCount));
    
    // start dma
    DmaStart(&m_dmaInfo);
    
    // wait until we hit the end of buffer 
    // Wait for dma interrupt...  
    dwCause = WaitForSingleObject(m_hEvent, DMA_IRQ_TIMEOUT);         
     
    switch(dwCause)
    {
    case WAIT_OBJECT_0:
        {
            // Verify cause of interrupt was because we hit the end of block
            dwStatus = DmaGetStatus(&m_dmaInfo);
            if ((dwStatus & (dmaSettings.interrupts)) == 0)
            {
                ERRORMSG(ZONE_ERROR, (TEXT("Unexpected cause of interrupt\r\n")));
                break; 
            }

            DmaClearStatus(&m_dmaInfo, dwStatus);
            if (DmaInterruptDone(m_hDmaChannel) == FALSE)
            {
                ERRORMSG(ZONE_ERROR, (TEXT("ERROR! Unable to get status for dma interrupt\r\n")));        
                break; 
            }

			// Do the "good" client job
			DmaStop(&m_dmaInfo);
                        
        break;
        }        
        
    default:
        RETAILMSG(ZONE_ERROR, (TEXT("ERROR! didn't receive DMA interrupt\r\n")));
        break;
    }     
    
#if DEBUG_VERIFY_SDMA_COPY   
    //
    // Beware!! Bring a lot of overhead and can lead to bad display rendering.
    //
    NKDbgPrintfW(L"verify memory\r\n"); 
    if (memcmp(pSource, pDestination, dwDataLength) != 0)
    {
        NKDbgPrintfW(L"ERROR! memory doesn't match up\r\n");
        DebugBreak();
        goto cleanUp; 
    }
#endif
    
    dwRet = dwDataLength; // everything went fine obviously...
cleanUp:
    UnlockPages((LPVOID)pSource, dwDataLength);
    UnlockPages((LPVOID)pDestination, dwDataLength);   
    
    return dwRet;
}

