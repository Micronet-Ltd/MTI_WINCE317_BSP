// All rights reserved ADENEO EMBEDDED 2010
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
*(c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  pdd.c
//
//  This file contains USB function PDD implementation. Actual implementation
//  doesn't support ISO endpoints.
//
#pragma warning(push)
#pragma warning(disable : 4115 4189 4201 4214)
#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include <ceotgbus.h>
#include <oal.h>
#include <omap3530_musbd.h>
#include <omap3530_musbotg.h>
#include <omap3530.h>
#include <dma_utility.h>
#include <musbfnpdd.h>
#include <musbioctl.h>
#include <batt.h>
#pragma warning(pop)

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 4053 6320)

#define REG_VBUS_CHARGE_EVENT_NAME TEXT("USBChargerNotify")
#define REG_VBUS_CHARGE_B_TYPE_CONNECTOR TEXT("BTYPE")
#define REG_USBFN_DRV_PATH TEXT("Drivers\\BuiltIn\\MUSBOTG\\USBFN")
#define REG_USBFN_DEFAULT_FUNC_DRV_PATH TEXT("Drivers\\USB\\FunctionDrivers")
#define REG_USBFN_DEFAULT_FUNC_DRV TEXT("DefaultClientDriver")
#define REG_VBUS_CHARGE_EVENT_TYPE REG_SZ

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//

#define TX_TRANSFER 0
#define RX_TRANSFER 1

#ifdef DEBUG

//note most zones are defined in "usbfn.h"
#define DBG_INTERRUPTS         (1 << 8)
#define DBG_POWER              (1 << 9)
#define DBG_INFO               (1 << 10)
#define DBG_DVFS               (1 << 11)

#define ZONE_WARNING			DEBUGZONE(1)
#define ZONE_INTERRUPTS         DEBUGZONE(8)
#define ZONE_POWER              DEBUGZONE(9)
#define ZONE_INFO               DEBUGZONE(10)
#define ZONE_DVFS               DEBUGZONE(11)
#define ZONE_FUNCTION           DEBUGZONE(12)
#define ZONE_PDD                DEBUGZONE(15)

extern DBGPARAM dpCurSettings = {
    L"UsbFn", {
        L"Error",       L"Warning",     L"Init",        L"Transfer",
        L"Pipe",        L"Send",        L"Receive",     L"USB Events",
        L"Interrupts",  L"Power",       L"Info",        L"Dvfs",
        L"Function",    L"Comments",    L"",            L"PDD"
    },
    DBG_ERROR|DBG_INIT
};

#endif


#define DEBUG_PRT_TRANS 0
#define DEBUG_PRT_INFO 0
#if 0
#undef DEBUGMSG
#define DEBUGMSG(a, b)  RETAILMSG(a, b)

#undef ZONE_PDD
#undef ZONE_INFO
#undef ZONE_ERROR

#define ZONE_PDD 0
#define ZONE_FUNCTION 0
#define ZONE_INFO 1
#define ZONE_POWER 0
#define ZONE_DVFS 1
#define ZONE_ERROR 1

#endif


DWORD Device_ResetIRQ(PVOID pHSMUSBContext);
DWORD Device_ResumeIRQ(PVOID pHSMUSBContext);
DWORD Device_ProcessEP0(PVOID pHSMUSBContext);
DWORD Device_ProcessEPx_RX(PVOID pHSMUSBContext, DWORD endpoint);
DWORD Device_ProcessEPx_TX(PVOID pHSMUSBContext, DWORD endpoint);
DWORD Device_Disconnect(PVOID pHSMUSBContex);
DWORD Device_Suspend(PVOID pHSMUSBContex);
DWORD Device_ProcessDMA(PVOID pHSMUSBContext, UCHAR channel);


MUSB_FUNCS gc_MUsbFuncs = 
{
    &Device_ResetIRQ,
        &Device_ResumeIRQ,
        &Device_ProcessEP0,
        &Device_ProcessEPx_RX,
        &Device_ProcessEPx_TX,
        NULL,
        &Device_Disconnect,
        &Device_Suspend,
        &Device_ProcessDMA
};

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = 
{
    {
        L"RxDmaEp", PARAM_DWORD, FALSE, offset(MUsbFnPdd_t, rx0DmaEp),
            fieldsize(MUsbFnPdd_t, rx0DmaEp), (void *) - 1
    }, 
    {
        L"TxDmaEp", PARAM_DWORD, FALSE, offset(MUsbFnPdd_t, tx0DmaEp),
            fieldsize(MUsbFnPdd_t, tx0DmaEp), (void *) - 1
    }, 
    {
        L"RxDmaBuffer", PARAM_DWORD, FALSE, 
            offset(MUsbFnPdd_t, rx0DmaBufferSize),
            fieldsize(MUsbFnPdd_t, rx0DmaBufferSize), (void *)(8192)
    }, 
    {
        L"TxDmaBuffer", PARAM_DWORD, FALSE, 
            offset(MUsbFnPdd_t, tx0DmaBufferSize),
            fieldsize(MUsbFnPdd_t, tx0DmaBufferSize), (void *)10240
    },
    {
        L"EnableDMA", PARAM_DWORD, FALSE, 
            offset(MUsbFnPdd_t, enableDMA),
            fieldsize(MUsbFnPdd_t, enableDMA), (void *)1
    },
     { 
        L"DVFSOrder", PARAM_DWORD, FALSE, 
        offset(MUsbFnPdd_t, nDVFSOrder),
        fieldsize(MUsbFnPdd_t, nDVFSOrder), (VOID*)150
    } 
};

static void ResetDMAChannel(MUsbFnPdd_t *pPdd, UCHAR channel);
//------------------------------------------------------------------------------
//
//  Function:  PreDmaActivation
//
//  Called by dma read/write routines to synchronize dma activity
//  with current DVFS transitions.
//
static void PreDmaActivation(MUsbFnPdd_t *pPdd, DWORD endPoint, DWORD TxRx)
{
#ifdef DEBUG
	MUsbFnEp_t *pEP = &pPdd->ep[endPoint];
    STransfer *pTransfer = pEP->pTransfer;
#endif    

	UNREFERENCED_PARAMETER(TxRx);

    DEBUGMSG(ZONE_FUNCTION, (
        L"+PreDmaActivation\r\n"
    ));

    if (pPdd->ep[endPoint].dmaDVFSstate != DVFS_PREDMA)
        pPdd->ep[endPoint].dmaDVFSstate = DVFS_PREDMA;
    else
        RETAILMSG(1, (TEXT("!!! ERROR => PreDmaActiviation wrong for EP %d, state %d\r\n"),
            endPoint, pPdd->ep[endPoint].dmaDVFSstate));

    for(;;)
    {
    
        // this operation needs to be atomic to handle a corner case
        EnterCriticalSection(&pPdd->csDVFS);

        // check and wait for DVFS activity to complete
        if (pPdd->bDVFSActive == TRUE)
        {
            DWORD dwRet;

            DEBUGMSG(ZONE_FUNCTION, (L"*** DVFS in progress wait before doing DMA\r\n"));
            dwRet = WaitForSingleObject(pPdd->hDVFSActivityEvent, 0);
            if (dwRet != WAIT_TIMEOUT)
            {
                InterlockedIncrement(&pPdd->nActiveDmaCount);
                DEBUGMSG(ZONE_DVFS, (L"***PreDma (DVFS) DmaCount(%d) endpoint(%d) Tx0Rx1(%d) pTransfer(0x%x) pBuffer(0x%x) cBuffer(%d) cTransfer(%d)\r\n",
                    pPdd->nActiveDmaCount,endPoint, TxRx, pTransfer, pTransfer->pvBuffer, pTransfer->cbBuffer, pTransfer->cbTransferred));
                LeaveCriticalSection(&pPdd->csDVFS);
                break;
            }
        }
        else
        {
            InterlockedIncrement(&pPdd->nActiveDmaCount);
            DEBUGMSG(ZONE_DVFS, (L"***PreDma (NonDVFS) DmaCount(%d) endpoint(%d) Tx0Rx1(%d) pTransfer(0x%x) pBuffer(0x%x) cBuffer(%d) cTransfer(%d)\r\n",
                pPdd->nActiveDmaCount,endPoint, TxRx, pTransfer, pTransfer->pvBuffer, pTransfer->cbBuffer, pTransfer->cbTransferred));

            LeaveCriticalSection(&pPdd->csDVFS);
            break;
        }
        LeaveCriticalSection(&pPdd->csDVFS);  // hDVFSActivityEvent not signaled
        Sleep(1);
    }

}

//------------------------------------------------------------------------------
//
//  Function:  PostDmaDeactivation
//
//  Called by dma read/write routines to synchronize dma activity
//  with current DVFS transitions.
//
static void PostDmaDeactivation(MUsbFnPdd_t *pPdd, DWORD endPoint, DWORD TxRx)
{
#ifdef DEBUG	
	MUsbFnEp_t *pEP = &pPdd->ep[endPoint];
	STransfer *pTransfer = pEP->pTransfer;
#endif

	UNREFERENCED_PARAMETER(endPoint);
	UNREFERENCED_PARAMETER(TxRx);

    DEBUGMSG(ZONE_FUNCTION, (
        L"+PostDmaDeactivation\r\n"
    ));
    
    ASSERT(pPdd->nActiveDmaCount > 0);

    // this operation needs to be atomic to handle a corner case
    EnterCriticalSection(&pPdd->csDVFS);
    
    // check if all dma's are inactive and signal ack event if so
    InterlockedDecrement(&pPdd->nActiveDmaCount);
    if (pPdd->nActiveDmaCount < 0)
    {
        RETAILMSG(1, (L"***###PostDma error on EP %d\r\n", endPoint));
        RETAILMSG(1, (L"***###PostDma, nActiveDmaCount < 0 (0x%x)\r\n", pPdd->nActiveDmaCount));
        // this may be a race condition in which it stops DMA and then process back the
        if (pPdd->bDVFSAck == TRUE) 
            InterlockedIncrement(&pPdd->nActiveDmaCount);       
    }    

    if (pPdd->bDVFSActive == TRUE && pPdd->nActiveDmaCount <= 0 && pPdd->bDVFSAck == FALSE)
        {
        DEBUGMSG(ZONE_DVFS, (L"***PostDma finished all Dma's set hDVFSAckEvent EP %d, TxRx %d\r\n", endPoint, TxRx));        
        pPdd->bDVFSAck = TRUE;
        SetEvent(pPdd->hDVFSAckEvent);
        }

    DEBUGMSG(ZONE_DVFS, (L"***PostDma DmaCount(%d) endpoint(%d) Tx0Rx1(%d) pTransfer(0x%x) pBuffer(0x%x) cBuffer(%d) cTransfer(%d)\r\n",
            pPdd->nActiveDmaCount,endPoint, TxRx, pTransfer, pTransfer->pvBuffer, pTransfer->cbBuffer, pTransfer->cbTransferred));
    LeaveCriticalSection(&pPdd->csDVFS);

    DEBUGMSG(ZONE_FUNCTION, (
        L"-PostDmaDeactivation()\r\n"
        ));
}

//------------------------------------------------------------------------------
//
//  Function:  CopyDVFSHandles
//
//  Called by dma read/write routines to synchronize dma activity
//  with current DVFS transitions.
//
static void CopyDVFSHandles(MUsbFnPdd_t *pPdd, UINT processId, HANDLE hAckEvent, HANDLE hActivityEvent)
{
    DEBUGMSG(ZONE_FUNCTION, (
        L"+CopyDVFSHandles(0x%08x, 0x%08X, 0x%08X)\r\n", 
        pPdd, hAckEvent, hActivityEvent
    ));
        
    // make copies of event handles
    DuplicateHandle((HANDLE)processId, hAckEvent, 
        GetCurrentProcess(), &pPdd->hDVFSAckEvent, 
        0, FALSE, DUPLICATE_SAME_ACCESS
        );

    DuplicateHandle((HANDLE)processId, hActivityEvent, 
        GetCurrentProcess(), &pPdd->hDVFSActivityEvent, 
        0, FALSE, DUPLICATE_SAME_ACCESS
        );

    DEBUGMSG(ZONE_FUNCTION, (
        L"-CopyDVFSHandles()\r\n"
    ));
}

static BOOL SwitchReadDma2FIFO(MUsbFnPdd_t *pPdd)
{
    BOOL fSuccess = FALSE;        
    PCSP_MUSB_CSR_REGS pCsrRegs = pPdd->pUSBContext->pUsbCsrRegs;
    PHSMUSB_T pOTG =(PHSMUSB_T)(pPdd->pUSBContext);
    EnterCriticalSection(&pPdd->dmaCS);    
    if ((INREG32(&pOTG->pUsbDmaRegs->dma[MUSB_RX_DMA_CHN].Addr) != 0) &&
        ((INREG32(&pOTG->pUsbDmaRegs->dma[MUSB_RX_DMA_CHN].Addr) - pPdd->paDmaRx0Buffer) == 0))
    {        
        CLRREG16(&pCsrRegs->ep[pPdd->rx0DmaEp].RxCSR, RXCSR_P_AUTOCLEAR | RXCSR_P_DMAREQENAB | RXCSR_P_DMAREQMODE);
        fSuccess = TRUE;
    }
    LeaveCriticalSection(&pPdd->dmaCS);


    return fSuccess;
}
static BOOL IsReadDmaChannelActive(MUsbFnPdd_t *pPdd)
{
    PHSMUSB_T pOTG =(PHSMUSB_T)(pPdd->pUSBContext);
    BOOL fActive = FALSE;
    DWORD dmacntl = 0;

    EnterCriticalSection(&pPdd->dmaCS);
    dmacntl = INREG32(&pOTG->pUsbDmaRegs->dma[MUSB_RX_DMA_CHN].Cntl);
    if ((dmacntl & (pPdd->rx0DmaEp << 4)) && (dmacntl & 0x1))
        fActive = TRUE;        
    LeaveCriticalSection(&pPdd->dmaCS);
    return fActive;

}
static VOID PauseReadDma(MUsbFnPdd_t *pPdd, BOOL bHalt)
{
    PHSMUSB_T pOTG =(PHSMUSB_T)(pPdd->pUSBContext);
    if (bHalt)
    {
        if ((pPdd->nActiveDmaCount > 0) && IsReadDmaChannelActive(pPdd))
        {        
            if (SwitchReadDma2FIFO(pPdd))
            {
                DWORD cbTransferred = 0;
                pPdd->rx0DmaState = MODE_FIFO;
                // Before reset channel, please take a look how many bytes of data we have so far ...
                // it would be a critical problem                
                cbTransferred = (INREG32(&pOTG->pUsbDmaRegs->dma[MUSB_RX_DMA_CHN].Addr) - pPdd->paDmaRx0Buffer);                
                ResetDMAChannel(pPdd, MUSB_RX_DMA_CHN);
                DEBUGMSG(ZONE_DVFS, (TEXT("PauseReadDma success with remaining bytes = %d\r\n"), cbTransferred));
                if (cbTransferred)
                {                    
                    MUsbFnEp_t *pEP = &pPdd->ep[pPdd->rx0DmaEp];
                    STransfer *pTransfer = pEP->pTransfer;
                    if (pTransfer)
                    {
                        UCHAR *pBuffer =(UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;
                        if (pTransfer->dwBufferPhysicalAddress == 0)
                        {
                            memcpy(pBuffer, (UCHAR *)pPdd->pDmaRx0Buffer, cbTransferred);
                        }
                        pTransfer->cbTransferred += cbTransferred;
                        DEBUGMSG(ZONE_INFO, (TEXT("PauseReadDma =>>>> cbTransferred(%d), total transferred(%d), total buffer(%d)\r\n"),
                            cbTransferred, pTransfer->cbTransferred, pTransfer->cbBuffer));
                        // Actually, there are very high chance of race-condition, but so far, i didn't, still need to think of
                        // better way to handle that. If the data transferred is not multiple of packet size, it may be stopped
                        // due to middle of transfer or actually or data has been transferred.  Should we ack back?
                        // So, currently, notify MDD if (1) cbTransferred == cbBuffer                        
                        if (pTransfer->cbTransferred == pTransfer->cbBuffer) 
                        {
                            pEP->pTransfer = NULL;
                            pTransfer->dwUsbError = UFN_NO_ERROR;
                            DEBUGMSG(ZONE_INFO, (TEXT("ACK: CheckAndHaltDMA for EP %d\r\n"), pPdd->rx0DmaEp));
                            pPdd->pfnNotify(
                                pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);
                        }
                    }
                }

                InterlockedDecrement(&pPdd->nActiveDmaCount);
                if (pPdd->ep[pPdd->rx0DmaEp].dmaDVFSstate == DVFS_PREDMA)
                    pPdd->ep[pPdd->rx0DmaEp].dmaDVFSstate = DVFS_POSTDMA;                
    
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//  Function:  CheckAndHaltAllDma
//
//  Called by dma read/write routines to synchronize dma activity
//  with current DVFS transitions.
//
static VOID CheckAndHaltAllDma(MUsbFnPdd_t *pPdd, BOOL bHalt)
{
    
    DEBUGMSG(ZONE_FUNCTION, (
        L"+CheckAndHaltAllDma(0x%08x, %d)\r\n", 
        pPdd,bHalt
        ));

    EnterCriticalSection(&pPdd->csDVFS);

    PauseReadDma(pPdd, bHalt);
    if (pPdd->nActiveDmaCount == 0 && bHalt == TRUE && pPdd->bDVFSAck == FALSE)
    {
        DEBUGMSG(ZONE_DVFS, (L"*** no ActiveDMA when DVFS request comes: set hDVFSAckEvent\r\n"));        
        pPdd->bDVFSAck = TRUE;
        SetEvent(pPdd->hDVFSAckEvent);
    }  
    
    LeaveCriticalSection(&pPdd->csDVFS);

    DEBUGMSG(ZONE_FUNCTION, (L"-CheckAndHaltAllDma()\r\n"));
}

void 
memdump(unsigned char * data,   unsigned short num_bytes, unsigned short offset    )
{    
    unsigned short i,j,l;
    unsigned char tmp_str[100];
    unsigned char tmp_str1[10];
    for (i = 0; i < num_bytes; i += 16){
        unsigned short n ;
        tmp_str[0]='\0';
        n = i+offset ;
        for (j=0; j<4; j++) {
            l=n%16;
            if (l>=10)
            tmp_str[3-j]=(unsigned char)('A'+l-10);
            else
            tmp_str[3-j]=(unsigned char)(l+'0');
            n >>= 4 ;
        }
        tmp_str[4]='\0';
        strcat ( (char *)tmp_str, ": ");
        /*          Output the hex bytes        */
        for (j = i; j < (i+16); j++) {
            int m ;
            if (j < num_bytes)  {
                m=((unsigned int)((unsigned char)*(data+j)))/16 ;
                if (m>=10)
                    tmp_str1[0]='A'+(unsigned char)m-10;
                else
                    tmp_str1[0]=(unsigned char)m+'0';
                m=((unsigned int)((unsigned char)*(data+j)))%16 ;
                if (m>=10)
                    tmp_str1[1]='A'+(unsigned char)m-10;
                else
                    tmp_str1[1]=(unsigned char)m+'0';
                tmp_str1[2]='\0';
                strcat ((char *)tmp_str, (char *)tmp_str1);
                strcat ((char *)tmp_str, " ");
            }
            else {
                strcat((char *)tmp_str,"   ");
            }
        }
        strcat((char *)tmp_str, "  ");
        l=(unsigned short)strlen((char *)tmp_str);

        /*         * Output the ASCII bytes        */
        for (j = i; j < (i+16); j++){
            if (j < num_bytes){
                char c = *(data+j);
                if (c < ' ' || c > 'z')
                    c = '.';
                tmp_str[l++]=c;
            }
            else
                tmp_str[l++]=' ';
        }
        tmp_str[l++]='\r';        tmp_str[l++]='\n';        tmp_str[l++]='\0';
        RETAILMSG(1, (L"%S", tmp_str));    
    }
}

DWORD DBGPrtSetupPkt(PVOID pPkt)
{
#ifdef DEBUG
    USB_DEVICE_REQUEST* pSetup =(USB_DEVICE_REQUEST*)pPkt;
    
    DEBUGMSG(ZONE_FUNCTION, (L" "
        L"bmRequestType:0x%02x  bRequest:0x%02x   wValue:0x%04x   wIndex:0x%04x  wLength:0x%04x\r\n", 
        pSetup->bmRequestType, pSetup->bRequest, pSetup->wValue, pSetup->wIndex, pSetup->wLength
        ));
#else
	UNREFERENCED_PARAMETER(pPkt);	
#endif

    return ERROR_SUCCESS;
}

#ifdef DEBUG
VOID
prtEP0State(
    MUsbFnPdd_t *pPdd
)
{
    WCHAR *pwch;
    
    switch (pPdd->ep0State)
    {
        case EP0_ST_SETUP:
            pwch = L"EP0_ST_SETUP";
            break;
            
        case EP0_ST_SETUP_PROCESSED:
            pwch = L"EP0_ST_SETUP_PROCESSED";
            break;
            
        case EP0_ST_STATUS_IN:
            pwch = L"EP0_ST_STATUS_IN";
            break;
            
        case EP0_ST_STATUS_OUT:
            pwch = L"EP0_ST_STATUS_OUT";
            break;
            
        case EP0_ST_DATA_RX:
            pwch = L"EP0_ST_DATA_RX";
            break;
            
        case EP0_ST_DATA_TX:
            pwch = L"EP0_ST_DATA_TX";
            break;
            
        case EP0_ST_ERROR:
            pwch = L"EP0_ST_ERROR";
            break;
            
        default:
            pwch = L"invalid state";
            break;
    }
    
    DEBUGMSG(ZONE_INFO, (L"UsbFnPdd!prtEP0State: "
        L"%s\r\n",
        pwch
        ));
}
#else

//stub function out
#define prtEP0State(x)

#endif

VOID prtDescriptorInfo(
    const USB_DEVICE_DESCRIPTOR *pFullSpeedDeviceDesc,
    const UFN_CONFIGURATION *pFullSpeedConfig,
    const USB_CONFIGURATION_DESCRIPTOR *pFullSpeedConfigDesc
)
{
    UFN_INTERFACE *pIFC;
    UFN_ENDPOINT *pEP;
    DWORD ifc, epx;

	UNREFERENCED_PARAMETER(pFullSpeedDeviceDesc);
	UNREFERENCED_PARAMETER(pFullSpeedConfigDesc);

    for (ifc = 0; ifc < pFullSpeedConfig->Descriptor.bNumInterfaces; ifc++)
    {
        // For each endpoint in interface
        pIFC = &pFullSpeedConfig->pInterfaces[ifc];
        for (epx = 0; epx < pIFC->Descriptor.bNumEndpoints; epx++)
        {
            pEP = &pIFC->pEndpoints[epx];
            
            DEBUGMSG(ZONE_INFO, (TEXT("interface: %d  EP:%2.2d  direction:%s maxPacketSize:%d  Attributes:0x%x\r\n"),
                ifc,
                pEP->Descriptor.bEndpointAddress & 0x7F,
                ((pEP->Descriptor.bEndpointAddress & 0x80)? L"IN " : L"OUT"),
                pEP->Descriptor.wMaxPacketSize,
                pEP->Descriptor.bmAttributes));
        }
    }
}


UINT CalcFIFOAddr(DWORD endpoint, BOOL bRxDir)
{
    UINT fifoAddr;

    fifoAddr =  endpoint *(1024/8);

    if (bRxDir)
       fifoAddr +=512/8;

    return fifoAddr;
}

BOOL ReadFIFO(MUsbFnPdd_t *pPdd, UCHAR endpoint, void *pData, DWORD size)
{
    PCSP_MUSB_GEN_REGS pGenRegs = pPdd->pUSBContext->pUsbGenRegs;
    DWORD total  = size/4;
    DWORD remain = size%4;
    DWORD i;
    DWORD *pDword =(DWORD *)pData;

    volatile ULONG *pReg = (volatile ULONG *)&pGenRegs->fifo[endpoint];

    // Critical section would be handled outside
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ReadFIFO EP(%d): pData(0x%x) total (0x%x), remain (0x%x), size(0x%x)\r\n"), endpoint, pData, total, remain, size));

    // this is 32-bit align

    for (i = 0; i < total; i++)
    {
        *pDword++ = INREG32(pReg);
    }

    if (remain != 0)
    {
        UCHAR* pUCHAR = (UCHAR*) pDword;
        DWORD dwTemp = INREG32(pReg);
        
        while (remain--)
        {
            *pUCHAR++ = (UCHAR) (dwTemp & 0xFF);
            dwTemp>>=8;
        }
    }

    //RETAILMSG(1,(TEXT("Read fifo\r\n")));
    //memdump((UCHAR*)pData,(USHORT)size,0);
    //RETAILMSG(1,(TEXT("\r\n")));

    return TRUE;
}


//-------------------------------------------------------------------------
BOOL WriteFIFO(MUsbFnPdd_t *pPdd, UCHAR endpoint, void *pData, DWORD size)
{
    PCSP_MUSB_GEN_REGS pGenRegs = pPdd->pUSBContext->pUsbGenRegs;
    DWORD total  = size/4;
    DWORD remain = size%4;
    DWORD i;
    DWORD *pDword =(DWORD *)pData;
    volatile ULONG *pReg = (volatile ULONG *)&pGenRegs->fifo[endpoint];

    // Critical section would be handled outside
    DEBUGMSG(ZONE_FUNCTION, (TEXT("WriteFIFO: total (0x%x), remain (0x%x), size(0x%x)\r\n"), total, remain, size));

    //memdump((UCHAR*)pData,(USHORT)size,0);
    //RETAILMSG(1,(TEXT("\r\n")));

    // this is 32-bit align
    for (i = 0; i < total; i++)
    {
        OUTREG32(pReg, *pDword++);
    }
        
    // Set the pByte equal to the last bytes of data being transferred
    if (remain != 0)
    {
        // Pointer to the first byte of data
        USHORT *pWORD =(USHORT *)pDword;

        // Finally if there is remain
        if (remain & 0x2)  // either 2 or 3
        {
            // Write 2 bytes to there
            OUTREG16(pReg, *pWORD++);
        }

        if (remain & 0x1)
        {
            // Write 1 byte to there
            OUTREG8(pReg, *((UCHAR*)pWORD));
        }
    }
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: UpdateDevicePower
//
//  UpdateDevice according to self power state and PM power state.
//
static CEDEVICE_POWER_STATE
UpdateDevicePower(
    MUsbFnPdd_t *pPdd
)
{
    CEDEVICE_POWER_STATE ps;
    
    PREFAST_ASSERT(pPdd != NULL);
    ps = min(pPdd->pmPowerState, pPdd->selfPowerState);
    
    DEBUGMSG(ZONE_POWER, (L"UsbFnPdd!UpdateDevicePower: "
        L"Move from D%u to D%u\r\n",
        pPdd->actualPowerState , ps
        ));
    
    // Call bus driver to set power state
    SetDevicePowerState(pPdd->hParentBus, ps, NULL);
    pPdd->actualPowerState = ps;
    return ps;
}


//------------------------------------------------------------------------------
//
//  Function:  SetPowerState
//
static VOID
SetPowerState(
    MUsbFnPdd_t* pPdd,
    CEDEVICE_POWER_STATE ps
)
{
    PREFAST_ASSERT(pPdd != NULL);
    
    DEBUGMSG(ZONE_POWER, (L"UsbFnPdd!SetPowerState: "
        L"Try D%u -> D%u\r\n",
        ps, pPdd->pmPowerState
        ));

    if (ps != pPdd->pmPowerState)
    {
        pPdd->pmPowerState = ps;
        UpdateDevicePower(pPdd);
    }
}


//------------------------------------------------------------------------------
//
//  Function:  ContinueTxTransfer
//
//  This function sends next packet from transaction buffer. 
//
static DWORD WINAPI
ContinueTxTransfer(
    MUsbFnPdd_t *pPdd,
    DWORD endpoint
)
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    MUsbFnEp_t *pEP = &pPdd->ep[endpoint];
    PCSP_MUSB_CSR_REGS pCsrRegs = pPdd->pUSBContext->pUsbCsrRegs;
    STransfer *pTransfer = pEP->pTransfer;
    DWORD space, txcount;
    BOOL complete = FALSE;
    UCHAR *pBuffer;
    DWORD dwFlag;
    
    DEBUGMSG(DEBUG_PRT_INFO, (L"+UsbFnPdd!ContinueTxTransfer: "
        L"EP %d pTransfer 0x%08x (%d, %d, %d)\r\n",
        endpoint, pTransfer, pTransfer != NULL ? pTransfer->cbBuffer : 0,
        pTransfer != NULL ? pTransfer->cbTransferred : 0,
        pTransfer != NULL ? pTransfer->dwUsbError : -1
        ));

    if (pTransfer == NULL) 
    {
        DEBUGMSG(ZONE_ERROR, (L"ContinueTxTransfer:  error pTransfer is NULL\r\n"));
        return ERROR_INVALID_PARAMETER;
    }    
    
    __try
    {
       
        pBuffer =(UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;
        space = pTransfer->cbBuffer - pTransfer->cbTransferred;
                
        if (endpoint != 0)
        {
            if (INREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR) & TXCSR_P_TXPKTRDY)
            {
                DEBUGMSG(ZONE_INFO, (TEXT("endpoint(%d) cbBuffer(%d) cbTransferred(%d)\r\n"), endpoint, pTransfer->cbBuffer, pTransfer->cbTransferred));
                DEBUGMSG(ZONE_INFO, (TEXT("TXPKTRdy bit is set ..\r\n")));
                goto CleanUp;
            }
            else
            {
                // How many bytes we can send just now?
                txcount = pEP->maxPacketSize;
                if (txcount > space)
                    txcount = space;
                
                if (pPdd->ep[endpoint].dmaEnabled)
                {
                    // disable DMA related bits
                    CLRREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_AUTOSET | TXCSR_P_DMAREQENAB);
                    
                    // clear TXCSR_P_DMAREQMODE after clearing TXCSR_P_DMAREQENAB (per MUSBMHDRC programming guide)
                    CLRREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_DMAREQMODE);
                }
                
                DEBUGMSG(ZONE_FUNCTION, (TEXT("WriteFIFO EP %d, Size %d\r\n"), endpoint, txcount));
                //memdump((uchar *)pBuffer, (unsigned short)txcount, 0);
                // Write data to FIFO
                WriteFIFO(pPdd, (UCHAR) endpoint, pBuffer, txcount);
                
                // We transfered some data
                pTransfer->cbTransferred += txcount;                
                SETREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_TXPKTRDY);
            } 
        }
        else // endpoint == 0
        {
            // Zero endpoint: Zero length padding needed
            // if last packet is maxPacketSize and bytes transferred is not equal to max transfer size specified Setup transaction
            pEP->zeroLength =
                (space == pEP->maxPacketSize) &&
                (pPdd->setupCount > pTransfer->cbBuffer);
            
            if (pPdd->ep0State != EP0_ST_SETUP_PROCESSED)
            {
                DEBUGMSG(ZONE_INFO, (TEXT("Invalid State = %d\r\n"), pPdd->ep0State));
                goto CleanUp;
            }
            else
            {
                pPdd->ep0State = EP0_ST_DATA_TX;
                if (INREG16(&pCsrRegs->ep[endpoint].CSR.CSR0) & CSR0_P_TXPKTRDY)
                {
                    DEBUGMSG(ZONE_ERROR, (TEXT("endpoint(%d) cbBuffer(%d) cbTransferred(%d)\r\n"), endpoint, pTransfer->cbBuffer, pTransfer->cbTransferred));
                    DEBUGMSG(ZONE_ERROR, (TEXT("TXPKTRdy bit is set ..\r\n")));
                    goto CleanUp;
                }
                else
                {
                    // How many bytes we can send just now?
                    txcount = pEP->maxPacketSize;
                    if (txcount > space)
                        txcount = space;
                    
                    DEBUGMSG(ZONE_INFO, (L"ContinueTxTransfer: EP0_ST_DATA_TX space_left_to_send=%d txcount=%d\r\n", space, txcount));
                    
                    // Write data to FIFO
                    WriteFIFO(pPdd, EP0, pBuffer, txcount);
                    
                    // We transfered some data
                    pTransfer->cbTransferred += txcount;
                    
                    DEBUGMSG(ZONE_INFO, (L"UsbFnPdd!ContinueTxTransfer: "
                        L"Tx packet size: %d\r\n",
                        pTransfer->cbBuffer
                        ));
                                        
                    if (pTransfer->cbTransferred == pTransfer->cbBuffer)
                    {
                        DEBUGMSG(ZONE_INFO, (TEXT("DATAEND add\r\n")));
                        pPdd->ep0State = EP0_ST_STATUS_OUT;
                        DEBUGMSG(ZONE_INFO, (TEXT("Set OUT Msg => ep0State = 0x%x\r\n"), pPdd->ep0State));
                        dwFlag = CSR0_P_TXPKTRDY | CSR0_P_DATAEND;
                        pEP->pTransfer = NULL;
                        pTransfer->dwUsbError = UFN_NO_ERROR;
                        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);   
                        DEBUGMSG(ZONE_INFO, (TEXT("Device_ProcessEP0: Notify - transfer completed\r\n")));                        
                    }
                    else
                    {
                        dwFlag = CSR0_P_TXPKTRDY;
                    }                   
                    DEBUGMSG(ZONE_INFO, (TEXT("1. Set TxPktRdy - 0\r\n")));
                    SETREG16(&pCsrRegs->ep[endpoint].CSR.CSR0, dwFlag);                    
                }                
            }
        } // if (endpoint != 0)
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;
        complete = TRUE;
        DEBUGMSG(ZONE_ERROR, (L"UsbFnPdd!ContinueTxTransfer: UFN_CLIENT_BUFFER_ERROR"));
    }
    
    rc = ERROR_SUCCESS;
    
CleanUp:
    
    if (endpoint == 0)
        prtEP0State(pPdd);
    
    DEBUGMSG(ZONE_FUNCTION, (L"-ContinueTxTransfer\r\n"));
    
    return rc;
}

//------------------------------------------------------------------------------


static BOOL WINAPI
ProcessDMAChannel(MUsbFnPdd_t *pPdd, UCHAR endpoint, UCHAR channel, BOOL IsTx, DWORD paData, DWORD size, DWORD dwMaxPacket)
{
    PHSMUSB_T pOTG =(PHSMUSB_T)(pPdd->pUSBContext);
    DWORD dmacntl = 0;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ProcessDMAChannel  endpoint:%d   channel:%d\r\n"), endpoint, channel));
    
    //  Now actual configure the DMA channel
    //  DMA Mode (D2)
    EnterCriticalSection(&pPdd->dmaCS);
    if (size > dwMaxPacket)
        dmacntl |= DMA_CNTL_DMAMode;
    //  Burst Mode (D10-9)
    if (dwMaxPacket >= 64)
        dmacntl |= BURSTMODE_INCR16;
    else if (dwMaxPacket >= 32)
        dmacntl |= BURSTMODE_INCR8;
    else if (dwMaxPacket >= 16)
        dmacntl |= BURSTMODE_INCR4;
    else
        dmacntl |= BURSTMODE_UNSPEC;
    
    //  Direction (D1) - for sure it is Tx endpoint
    //  Interrupt Enable (D3)
    //  Enable DMA (D0)
    dmacntl |=(DMA_CNTL_Enable | DMA_CNTL_InterruptEnable);
    if (IsTx)
        dmacntl |= DMA_CNTL_Direction;
    
    //  Set endpoint number
    dmacntl |=(endpoint << 4);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("DMA Channel %d\r\n"), channel));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("DMA Addr (0x%x), Count (0x%x), Cntl(0x%x)\r\n"),
        paData, size, dmacntl));
    // Write address
    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Addr, (DWORD)paData);       
    // Write count
    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Count, (DWORD)size);
        
    // Write Control
    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Cntl, (DWORD)dmacntl);
    LeaveCriticalSection(&pPdd->dmaCS);
    
    return TRUE;
}

static void ResetDMAChannel(MUsbFnPdd_t *pPdd, UCHAR channel)
{
    PHSMUSB_T pOTG =(PHSMUSB_T)(pPdd->pUSBContext);

    EnterCriticalSection(&pPdd->dmaCS);
    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Addr, (DWORD)0x00);    
    // Write count
    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Count, (DWORD)0x00);
        
    // Write Control
    OUTREG32(&pOTG->pUsbDmaRegs->dma[channel].Cntl, (DWORD)0x00);

    pPdd->bRXIsUsingUsbDMA = FALSE;
    pPdd->bTXIsUsingUsbDMA = FALSE;

    LeaveCriticalSection(&pPdd->dmaCS);
}


//------------------------------------------------------------------------------

static DWORD WINAPI
ContinueTxDmaTransfer(
    MUsbFnPdd_t *pPdd,
    DWORD endpoint
)
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    MUsbFnEp_t *pEP = &pPdd->ep[endpoint];
    PCSP_MUSB_CSR_REGS pCsrRegs = pPdd->pUSBContext->pUsbCsrRegs;
    STransfer *pTransfer = pEP->pTransfer;
    DWORD size;
    BOOL complete = FALSE;
    UCHAR *pBuffer;
    
    
    DEBUGMSG(DEBUG_PRT_INFO, (L"UsbFnPdd!ContinueTxDmaTransfer: "
        L"EP %d pTransfer 0x%08x (%d, %d, %d)\r\n",
        endpoint, pTransfer, pTransfer != NULL ? pTransfer->cbBuffer : 0,
        pTransfer != NULL ? pTransfer->cbTransferred : 0,
        pTransfer != NULL ? pTransfer->dwUsbError : -1
        ));
    
    if (pTransfer == NULL)
        goto cleanUp;
    
    // Is this final interrupt of transfer?
    if ((pTransfer->cbTransferred == pTransfer->cbBuffer) && !pEP->zeroLength)
    {
        pTransfer->dwUsbError = UFN_NO_ERROR;
        complete = TRUE;
        rc = ERROR_SUCCESS;
        goto cleanUp;
    }
    
    // Get size and buffer position
    size = pTransfer->cbBuffer - pTransfer->cbTransferred;
    
    if (!pPdd->ep[endpoint].dmaEnabled || (size < pPdd->ep[endpoint].maxPacketSize))
    {
        return ContinueTxTransfer(pPdd, endpoint);
    }

    // If we are using PDD buffer we must check for maximal size
    if ((pTransfer->dwBufferPhysicalAddress == 0) &&
        (size > pPdd->tx0DmaBufferSize))
    {
        // send integer multiple of maxPacket size
        size = pEP->maxPacketSize *(pPdd->tx0DmaBufferSize/pEP->maxPacketSize);
        pEP->bTxDMAShortPacket = FALSE;

    }
    else
    {
        pEP->bTxDMAShortPacket = (size % pEP->maxPacketSize) ? FALSE : TRUE;
    }



    EnterCriticalSection(&pPdd->dmaCS);
    if(!pPdd->bRXIsUsingUsbDMA)
    {
        pPdd->bTXIsUsingUsbDMA = TRUE;
        LeaveCriticalSection(&pPdd->dmaCS);
    }
    else
    {
        LeaveCriticalSection(&pPdd->dmaCS);
        return ContinueTxTransfer(pPdd, endpoint);
    }
      
    __try
    {
        pBuffer =(UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;
        DEBUGMSG(ZONE_FUNCTION, (L"ContinueDmaTx: Transfer(0x%x), pBuffer(0x%x), cbTransferred(%d), size(%d)\r\n",
            pTransfer, pBuffer, pTransfer->cbTransferred, size));

        {
            if (size > 0)
            {
                memcpy(pPdd->pCachedDmaTx0Buffer, pBuffer, size);
                CacheRangeFlush(pPdd->pCachedDmaTx0Buffer, size, CACHE_SYNC_DISCARD);
            }

            PreDmaActivation(pPdd, endpoint, TX_TRANSFER);
            SETREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_AUTOSET | TXCSR_P_DMAREQENAB | TXCSR_P_DMAREQMODE);
            ProcessDMAChannel(pPdd, (UCHAR) endpoint, MUSB_TX_DMA_CHN, TRUE, pPdd->paDmaTx0Buffer, size, pEP->maxPacketSize);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;
        complete = TRUE;
        DEBUGMSG(ZONE_ERROR, (L"UsbFnPdd!ContinueTxTransfer: UFN_CLIENT_BUFFER_ERROR"));
    }
        
    // Update buffer and clear zero length flag
    pTransfer->cbTransferred += size;
    pEP->zeroLength = FALSE;
    
    rc = ERROR_SUCCESS;
    
    
    
cleanUp:
    
    if (complete)
    {
        DEBUGMSG(DEBUG_PRT_TRANS, (L"ContinueTxDmaTransfer: call pfnNotify COMPLETE endp:%d +\r\n", endpoint));  
        
        DEBUGMSG(ZONE_PDD, (L"UsbFnPdd!ContinueTxDmaTransfer: "
            L"EP %d pTransfer 0x%08x (%d, %d, %d) - done\r\n",
            endpoint, pTransfer, pTransfer != NULL ? pTransfer->cbBuffer : 0,
            pTransfer != NULL ? pTransfer->cbTransferred : 0,
            pTransfer != NULL ? pTransfer->dwUsbError : -1
            ));

        pEP->pTransfer = NULL;
        pTransfer->dwUsbError = UFN_NO_ERROR;
        DEBUGMSG(ZONE_INFO, (TEXT("ACK:ContinueTxDmaTransfer for EP %d\r\n"), endpoint));
        pPdd->pfnNotify(
            pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer
            );
    }
    
    return rc;
}

//------------------------------------------------------------------------------

static DWORD WINAPI
StartRxTransfer(
    MUsbFnPdd_t *pPdd,
    DWORD endpoint
)
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    MUsbFnEp_t *pEP = &pPdd->ep[endpoint];
    STransfer *pTransfer = pEP->pTransfer;
    PCSP_MUSB_CSR_REGS pCsrRegs = pPdd->pUSBContext->pUsbCsrRegs;
    PCSP_MUSB_GEN_REGS pGenRegs = pPdd->pUSBContext->pUsbGenRegs;
    BOOL bNotifyDataReady = FALSE;
    
    DEBUGMSG(ZONE_FUNCTION, (L"UsbFnPdd!StartRxTransfer: "
        L"EP %d pTransfer 0x%08x (%d, %d, %d)\r\n",
        endpoint, pTransfer, pTransfer != NULL ? pTransfer->cbBuffer : 0,
        pTransfer != NULL ? pTransfer->cbTransferred : 0,
        pTransfer != NULL ? pTransfer->dwUsbError : -1
        ));

    if (pTransfer == NULL) 
    {
        DEBUGMSG(ZONE_ERROR, (L"StartRxTransfer:  error pTransfer is NULL\r\n"));
        return ERROR_INVALID_PARAMETER;
    }

    if (endpoint != 0)
    {
        if (pPdd->intr_rx_data_avail &(1 << endpoint))
        {
            // rx FIFO has data.
            UCHAR *pBuffer;            
            DWORD space, maxSize;
            WORD rxcount;
            
            rxcount = INREG16(&pCsrRegs->ep[endpoint].Count.RxCount);
            if (rxcount > pPdd->ep[endpoint].maxPacketSize)
            {
                rxcount = 0;
            }
            
            pBuffer =(UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;               
            space = pTransfer->cbBuffer - pTransfer->cbTransferred;

            if(rxcount > space)
            {
                return ERROR_INVALID_PARAMETER;
            }
            // Get maxPacketSize
            maxSize = pPdd->ep[endpoint].maxPacketSize;
            
            __try
            {
                
                ReadFIFO(pPdd, (UCHAR)endpoint, pBuffer, (DWORD)rxcount);
                DEBUGMSG(ZONE_INFO, (L"Received:\r\n"));
                
                // We transfered some data
                pTransfer->cbTransferred += rxcount;
                
                // Is this end of transfer?
                if (rxcount < maxSize)
                {
                    // received last block
                    pTransfer->dwUsbError = UFN_NO_ERROR;
                    bNotifyDataReady = TRUE;
                    // Don't clear the RxPktRdy until it is ready to further received
                }
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;
            }
            
            // clear intr_rx_data_avail EP0 bit
            pPdd->intr_rx_data_avail &= ~(1 << endpoint);           

            pPdd->ep[endpoint].bLastRxUsedDMA = FALSE;
            
            if (bNotifyDataReady)
            {
                pEP->pTransfer = NULL;
                pTransfer->dwUsbError = UFN_NO_ERROR;
                DEBUGMSG(ZONE_INFO, (TEXT("ACK: StartRxTransfer for EP %d\r\n"), endpoint));
                pPdd->pfnNotify(
                    pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);
            }
        }
        else
        {
            if(pPdd->ep[endpoint].bLastRxUsedDMA == FALSE)
            {
            CLRREG16(&pCsrRegs->ep[endpoint].RxCSR, RXCSR_P_RXPKTRDY);
            }
            else
            {
                pPdd->ep[endpoint].bLastRxUsedDMA = FALSE;
            }

            SETREG16(&pGenRegs->IntrRxE, (1<<endpoint));
            DEBUGMSG(ZONE_FUNCTION, (TEXT("EP %d IssueTransfer(OUT) RxCSR=0x%x\r\n"), endpoint, INREG16(&pCsrRegs->ep[endpoint].RxCSR)));
        }
    }
    else // if (endpoint != 0)
    {
        if (pPdd->ep0State == EP0_ST_SETUP_PROCESSED)
        {
            pPdd->ep0State = EP0_ST_DATA_RX;
            
            if (pPdd->intr_rx_data_avail &(1 << EP0))
            {
                // rx FIFO has data.
                UCHAR *pBuffer;            
                DWORD space, maxSize;
                USHORT rxcount;
                
                rxcount = INREG8(&pCsrRegs->ep[EP0].Count.Count0);
                
                pBuffer =(UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;               
                space = pTransfer->cbBuffer - pTransfer->cbTransferred;
                // Get maxPacketSize
                maxSize = pPdd->ep[EP0].maxPacketSize;
                
                __try
                {
                    USHORT dwFlag = CSR0_P_SERVICEDRXPKTRDY;
                    
                    ReadFIFO(pPdd, (UCHAR)endpoint, pBuffer, (DWORD)rxcount);
                    DEBUGMSG(ZONE_INFO, (L"Received:\r\n"));
                    //memdump((UCHAR *)pBuffer, (USHORT)rxcount, 0);
                    
                    // We transfered some data
                    pTransfer->cbTransferred = pTransfer->cbBuffer - space;
                    
                    // Is this end of transfer?
                    if (rxcount < maxSize)
                    {
                        // received last block
                        pTransfer->dwUsbError = UFN_NO_ERROR;
                        pPdd->ep0State = EP0_ST_STATUS_IN;
                        bNotifyDataReady = TRUE;
                        dwFlag |= CSR0_P_DATAEND;
                    }
                    SETREG16(&pCsrRegs->ep[EP0].CSR.CSR0, dwFlag);
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;
                }
                
                // clear intr_rx_data_avail EP0 bit
                pPdd->intr_rx_data_avail &= ~(1 << EP0);            
                
                if (bNotifyDataReady)
                {
                    pEP->pTransfer = NULL;
                    pTransfer->dwUsbError = UFN_NO_ERROR;
                    pPdd->pfnNotify(
                        pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);
                }
            }
        }
    }       
    
    rc = ERROR_SUCCESS;
    
    if (endpoint == 0)
        prtEP0State(pPdd);
    DEBUGMSG(ZONE_FUNCTION, (L"-StartRxTransfer\r\n"));
    
    return rc;
}


//------------------------------------------------------------------------------

static DWORD WINAPI
StartRxDmaTransfer(
    MUsbFnPdd_t *pPdd,
    DWORD endpoint
)
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    MUsbFnEp_t *pEP = &pPdd->ep[endpoint];
    STransfer *pTransfer = pEP->pTransfer;
    DWORD size;
    DWORD dwDMAPhysAddr = pTransfer->dwBufferPhysicalAddress;
    PCSP_MUSB_CSR_REGS pCsrRegs = pPdd->pUSBContext->pUsbCsrRegs;
    PCSP_MUSB_GEN_REGS pGenRegs = pPdd->pUSBContext->pUsbGenRegs;
    UINT16 RxCsr = 0;
        
    
    DEBUGMSG(ZONE_FUNCTION, (L"UsbFnPdd!StartRxDmaTransfer: "
        L"EP %d pTransfer 0x%08x (%d, %d, %d)\r\n",
        endpoint, pTransfer, pTransfer != NULL ? pTransfer->cbBuffer : 0,
        pTransfer != NULL ? pTransfer->cbTransferred : 0,
        pTransfer != NULL ? pTransfer->dwUsbError : -1
        ));
    
    if (pTransfer == NULL)
        goto cleanUp;
    
    // Get size to be received
    size = pTransfer->cbBuffer - pTransfer->cbTransferred;
    
    if (!pPdd->ep[endpoint].dmaEnabled ||
        (size <= pPdd->ep[endpoint].maxPacketSize) ||
        (pPdd->rx0DmaState != MODE_DMA) ||
        !pPdd->bDMAForRX ||
        pPdd->intr_rx_data_avail)
    {
        return StartRxTransfer(pPdd, endpoint);
    }

    EnterCriticalSection(&pPdd->dmaCS);
    if (!pPdd->bTXIsUsingUsbDMA)
    {
        pPdd->bRXIsUsingUsbDMA = TRUE;
        LeaveCriticalSection(&pPdd->dmaCS);
    }
    else
    {
        LeaveCriticalSection(&pPdd->dmaCS);
        return StartRxTransfer(pPdd, endpoint);
    }

    pEP->dwRemainBuffer = 0;

    // If we are using PDD buffer we must check for maximal size
    if (dwDMAPhysAddr == 0)
    {
        if (size > pPdd->rx0DmaBufferSize)
        {
            pEP->dwRemainBuffer = size - pPdd->rx0DmaBufferSize;
            size = pPdd->rx0DmaBufferSize;
        }
        dwDMAPhysAddr = pPdd->paDmaRx0Buffer;
    }
    
    EnterCriticalSection(&pPdd->dmaCS);

    RxCsr = INREG16(&pCsrRegs->ep[endpoint].RxCSR);
    
    if(pEP->bMassStore)
    {
        RxCsr &= ~(RXCSR_P_DMAREQMODE);
        RxCsr |= (RXCSR_P_AUTOCLEAR | RXCSR_P_DMAREQENAB);
        CLRREG16(&pGenRegs->IntrRxE, (1<<endpoint));
    }
    else 
    {
        RxCsr |= (RXCSR_P_AUTOCLEAR | RXCSR_P_DMAREQENAB | RXCSR_P_DMAREQMODE);
        SETREG16(&pGenRegs->IntrRxE, (1<<endpoint));
    }
    // This is to avoid race condition, if the DVFS Notify comes after PreDmaActivation, 
    // If it is after CheckAndHaltDMA, we need to change to StartRxTransfer. 
    // Otherwise, we can use directly to call StartRxTransfer    
    if ((pPdd->bDVFSActive) && (pPdd->rx0DmaState == MODE_FIFO))
    {
        pPdd->bRXIsUsingUsbDMA = FALSE;
        StartRxTransfer(pPdd, endpoint);
    }
    else
    {
        PreDmaActivation(pPdd, endpoint, RX_TRANSFER);
        SETREG16(&pCsrRegs->ep[endpoint].RxCSR, RxCsr);

        pEP->dwRxDMASize = size;
        ProcessDMAChannel(pPdd, (UCHAR) endpoint, MUSB_RX_DMA_CHN, FALSE, dwDMAPhysAddr, size, pEP->maxPacketSize);

        if(pPdd->ep[endpoint].bLastRxUsedDMA == FALSE)
            CLRREG16(&pCsrRegs->ep[endpoint].RxCSR, RXCSR_P_RXPKTRDY);
        else
            pPdd->ep[endpoint].bLastRxUsedDMA = FALSE;
    }
    LeaveCriticalSection(&pPdd->dmaCS);
    
    rc = ERROR_SUCCESS;
    
cleanUp:
    
    return rc;
}



//------------------------- Interrupt Routine ----------------------------------
// 
//  Function:   Device_ResetIRQ
//
//  Routine description:
//
//      Handling of Reset interrupt
//
//  Arguments:
//      
//      pHSMUSBContext  :   Handle to HSMUSB_T
//      others          :   to be determined.
//
//  Return values:
//  
//      ERROR SUCCESS
//
DWORD Device_ResetIRQ(PVOID pHSMUSBContext)
{
    MUsbFnPdd_t *pPdd =((PHSMUSB_T) pHSMUSBContext)->pContext[DEVICE_CONTEXT];
    PCSP_MUSB_GEN_REGS pGenRegs =((PHSMUSB_T) pHSMUSBContext)->pUsbGenRegs;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+Device_ResetIRQ\r\n")));
    
    // initialize Endpoint0 setup state 
    pPdd->ep0State = EP0_ST_SETUP;
    
    pPdd->intr_rx_data_avail = 0;
    if (pPdd->enableDMA && (pPdd->rx0DmaEp != -1) && pPdd->ep[pPdd->rx0DmaEp].dmaEnabled)
        pPdd->rx0DmaState = MODE_DMA;
    
    pPdd->fPowerDown = FALSE;
    pPdd->dwUSBFNState = USBFN_IDLE;

    if ((pPdd->devState & MUSB_DEVSTAT_ATT) == 0)
    {
        // MUSB doesn't generate an attach interrrupt.  Interpret bus reset event as attach.
        pPdd->devState |= MUSB_DEVSTAT_ATT | MUSB_DEVSTAT_USB_RESET;
        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);
        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_ATTACH);
        if (INREG8(&pGenRegs->Power) & POWER_HSMODE)
        {
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_SPEED, BS_HIGH_SPEED);
        }
        else
        {
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_SPEED, BS_FULL_SPEED);
        }           
        // Tell MDD about reset...
        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_RESET);
    }
    else
        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_RESET);

    prtEP0State(pPdd);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-Device_ResetIRQ\r\n")));
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:   Device_ResumeIRQ
//
//  Routine description:
//
//      Handling of Resume interrupt
//
//  Arguments:
//
//      pHSMUSBContext  :    Handle of HSMUSB_T
//      others          :    to be determined.
//
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD Device_ResumeIRQ(PVOID pHSMUSBContext)
{
	UNREFERENCED_PARAMETER(pHSMUSBContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+Device_ResumeIRQ\r\n")));
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:   Device_ReadSetup
//
//  Routine description:
//
//      Read a Setup packet from host
//
//  Arguments:
//
//      pHSMUSBContext  :   Handle of HSMUSB_T
//      others          :   TBD
//
//  Return values:
//      
//      ERROR_SUCCESS
//
DWORD Device_ReadSetup(PVOID pHSMUSBContext)
{
    PHSMUSB_T pUSBContxt =(PHSMUSB_T) pHSMUSBContext;
    MUsbFnPdd_t *pPdd =((PHSMUSB_T) pHSMUSBContext)->pContext[DEVICE_CONTEXT];
    PCSP_MUSB_GEN_REGS pGenRegs = pUSBContxt->pUsbGenRegs;
    UINT32 data[2];
    
    DEBUGMSG(ZONE_FUNCTION, (L"Device_ReadSetup\r\n"));
    DEBUGMSG(ZONE_INFO, (L"Power register: 0x%02x\r\n", INREG8(&pGenRegs->Power)));
    
    // Read setup data
    data[0] = INREG32(&pGenRegs->fifo[EP0]);
    data[1] = INREG32(&pGenRegs->fifo[EP0]);
    
    DEBUGMSG(ZONE_INFO, (L"Device_ReadSetup: "
        L"%08x %08x\r\n", data[0], data[1]
        ));
    
    DBGPrtSetupPkt((PVOID)data);
    DEBUGMSG(ZONE_INFO, (TEXT("Device_ReadSetup => Notify\r\n")));
    pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_SETUP_PACKET, (DWORD)data);
    
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:   Device_ProcessEP0
//
//  Routine description:
//
//      Handling of endpoint 0 transfer
//
//  Arguments:
//
//      pHSMUSBContext  :   Handle of HSMUSB_T
//      others          :   TBD
//
//  Return values:
//      
//      ERROR_SUCCESS
//
DWORD Device_ProcessEP0(PVOID pHSMUSBContext)
{
    USB_DEVICE_REQUEST* pSetup = NULL;
    PHSMUSB_T pUSBContext =(PHSMUSB_T) pHSMUSBContext;
    PCSP_MUSB_GEN_REGS pGenRegs = pUSBContext->pUsbGenRegs;
    PCSP_MUSB_CSR_REGS pCsrRegs = pUSBContext->pUsbCsrRegs;
    MUsbFnPdd_t *pPdd = pUSBContext->pContext[DEVICE_CONTEXT];
    UINT16 csr0Val;
    UINT16 rxcount;
    DWORD data[2];
    
    DEBUGMSG(ZONE_FUNCTION, (L"+Device_ProcessEP0 state = %d\r\n", pPdd->ep0State));

    csr0Val = INREG16(&pCsrRegs->ep[EP0].CSR.CSR0);
    rxcount = INREG16(&pCsrRegs->ep[EP0].Count.Count0);
    DEBUGMSG(ZONE_INFO, (TEXT("Interrupt receive with CSR0 = 0x%x, rxcount = 0x%x\r\n"), csr0Val, rxcount));
    
    if (csr0Val & CSR0_P_SENTSTALL)
    {
        // MUSB finished sending STALL 
        // clear SENTSTALL 
        DEBUGMSG(ZONE_INFO, (TEXT("Stall\r\n")));
        CLRREG16(&pCsrRegs->ep[EP0].CSR.CSR0, CSR0_P_SENTSTALL);
        pPdd->ep0State = EP0_ST_SETUP;
        csr0Val = INREG16(&pCsrRegs->ep[EP0].CSR.CSR0);
    }
    
    if (csr0Val & CSR0_P_SETUPEND)
    {
        /* setup request ended "early" */
        // clear SETUPEND 
        DEBUGMSG(ZONE_INFO, (TEXT("SetupEnd\r\n")));
        if (pPdd->bSetAddress)
        {
            DEBUGMSG(ZONE_INFO, (L"SetAddress=%d\r\n", pPdd->ucAddress));
            OUTREG8(&pGenRegs->FAddr, pPdd->ucAddress); 
            pPdd->bSetAddress = FALSE;
        }
        
        SETREG16(&pCsrRegs->ep[EP0].CSR.CSR0, CSR0_P_SERVICEDSETUPEND);
        pPdd->ep0State = EP0_ST_SETUP;
        csr0Val = INREG16(&pCsrRegs->ep[EP0].CSR.CSR0);
        DEBUGMSG(ZONE_INFO, (TEXT("CSR0 = 0x%x\r\n"), csr0Val));
    }
    
    switch (pPdd->ep0State)
    {
        case EP0_ST_STATUS_IN:
            // dummy state in processEP0
            if (pPdd->bSetAddress)
            {
                DEBUGMSG(ZONE_INFO, (L"SetAddress=%d\r\n", pPdd->ucAddress));
                OUTREG8(&pGenRegs->FAddr, pPdd->ucAddress); 
                pPdd->bSetAddress = FALSE;
            }
            pPdd->ep0State = EP0_ST_SETUP;
            DEBUGMSG(ZONE_INFO, (TEXT("In EP0 Status IN state.  Change to EP0 SETUP state.\r\n")));
            
            // if received packet while in EP0_ST_STATUS_IN, continue process SETUP packet.
            
            
        case EP0_ST_SETUP:
            if (rxcount < 8)
                break;
            if (!(csr0Val & CSR0_P_RXPKTRDY))
                break;        
            data[0] = INREG32(&pGenRegs->fifo[EP0]);
            data[1] = INREG32(&pGenRegs->fifo[EP0]);        
            DEBUGMSG(ZONE_INFO, (L"EP0_ST_SETUP receives 0x%x 0x%x\r\n", data[0], data[1]));
            pSetup =(USB_DEVICE_REQUEST*)data;
            if ((pSetup->bmRequestType == 0) &&
                    (pSetup->bRequest == USB_REQUEST_SET_CONFIGURATION)){
                HKEY hkDevice;
                DWORD dwStatus;
                DWORD dwType, dwSize;
                DWORD dwbTypeConnector = 0;

                DEBUGMSG(ZONE_INFO, (L"EP0_ST_SETUP receives USB_REQUEST_SET_CONFIGURATION"));

                dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR) REG_USBFN_DRV_PATH, 0, KEY_ALL_ACCESS, &hkDevice);
                if(dwStatus != ERROR_SUCCESS) {
                       DEBUGMSG(ZONE_WARNING, (_T("UfnPdd_Init: OpenDeviceKey('%s') failed %u\r\n"), REG_USBFN_DRV_PATH, dwStatus));
                       goto cleanUp;
                }

                dwType = REG_DWORD;
                dwSize = sizeof(dwbTypeConnector);
                dwStatus = RegQueryValueEx(hkDevice, REG_VBUS_CHARGE_B_TYPE_CONNECTOR, NULL, &dwType, 
                          (LPBYTE) &dwbTypeConnector, &dwSize);

                if(dwStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
                    DEBUGMSG(ZONE_WARNING, (_T("UFNPDD_Init: RegQueryValueEx('%s', '%s') failed %u\r\n"),
                            REG_USBFN_DRV_PATH, REG_VBUS_CHARGE_B_TYPE_CONNECTOR, dwStatus));
                    RegCloseKey(hkDevice);
                    goto cleanUp;
                }

                RegCloseKey(hkDevice);

                if(dwbTypeConnector){
                    SetEventData( pPdd->hVbusChargeEvent, BATTERY_USBHOST_CONNECT );
                    SetEvent( pPdd->hVbusChargeEvent );
                    DEBUGMSG(ZONE_PDD, (_T("UFNPDD_Init: B-Type connector\r\n")));
                }else{
                    SetEventData( pPdd->hVbusChargeEvent,  BATTERY_USBHOST_DISCONNECT );  
                    SetEvent( pPdd->hVbusChargeEvent );
                    DEBUGMSG(ZONE_PDD, (_T("UFNPDD_Init: Not an B-Type connector\r\n")));
                }
            }
            else if ((pSetup->bmRequestType == (USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE)) &&
                    (pSetup->bRequest == USB_REQUEST_SET_FEATURE))
            {
                //Host Negotiation Protocol is enabled
                switch(pSetup->wValue)
                {
                    case USB_FEATURE_B_HNP_ENABLE:
                        pPdd->devState |= MUSB_DEVSTAT_B_HNP_ENABLE;
                        BusChildIoControl(pPdd->hParentBus,IOCTL_BUS_USBOTG_HNP_ENABLE,NULL,0);
                        break;

                    case USB_FEATURE_A_HNP_SUPPORT:
                        pPdd->devState |= MUSB_DEVSTAT_A_HNP_SUPPORT;
                        break;

                    case USB_FEATURE_A_ALT_HNP_SUPPORT:
                        pPdd->devState |= MUSB_DEVSTAT_A_ALT_HNP_SUPPORT;
                        break;
                    }
            }

            pPdd->ep0State = EP0_ST_SETUP_PROCESSED;

            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_SETUP_PACKET, (DWORD)data);        
            break;
            
        case EP0_ST_SETUP_PROCESSED:
            pPdd->intr_rx_data_avail |=(1 << EP0);
            break;
            
        case EP0_ST_DATA_RX:
            if (!(csr0Val & CSR0_P_RXPKTRDY))
                break;
            
            if (pPdd->ep[EP0].pTransfer == NULL) 
            {
                // set intr_rxdata_avail and when IssueTransfer() is called retrieve data
                //          from FIFO and set CSR0 ServicedRxPkt bit and check for end of packet to set CSR0 DATAEND bit.
                //          Will need critical section for accessing pTransfer and intr_rx_available
                pPdd->intr_rx_data_avail |=(1 << EP0);
                DEBUGMSG(ZONE_INFO, (L"Device_ProcessEP0:  error EP0_ST_DATA_RX pTransfer is NULL\r\n"));
                break;
            }
                        
            {
                MUsbFnEp_t *pEP = &pPdd->ep[EP0];
                STransfer *pTransfer = pEP->pTransfer;
                UCHAR *pBuffer;            
                DWORD space, remain, maxSize;
                
                pBuffer =(UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;               
                space = pTransfer->cbBuffer - pTransfer->cbTransferred;
                // Get maxPacketSize
                maxSize = pPdd->ep[EP0].maxPacketSize;
                
                __try
                {
                    USHORT dwFlag = CSR0_P_SERVICEDRXPKTRDY;
                    remain = rxcount;
                    
                    ReadFIFO(pPdd, (UCHAR)EP0, pBuffer, (DWORD)rxcount);
                    
                    DEBUGMSG(ZONE_INFO, (L"Device_ProcessEP0: EP0_ST_DATA_RX received data\r\n"));                    
                    
                    // We transfered some data
                    pTransfer->cbTransferred += rxcount;
                    
                    // Is this end of transfer?
                    if (rxcount < maxSize)
                    {
                        // received last block
                        pTransfer->dwUsbError = UFN_NO_ERROR;
                        pPdd->ep0State = EP0_ST_STATUS_IN;
                        pEP->pTransfer = NULL;
                        pPdd->pfnNotify(
                            pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);
                        dwFlag |= CSR0_P_DATAEND;
                    }
                    SETREG16(&pCsrRegs->ep[EP0].CSR.CSR0, dwFlag);
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;
                }
                
                // clear intr_rx_data_avail EP0 bit
                pPdd->intr_rx_data_avail &= ~(1 << EP0);            
            }
            break;
            
        case EP0_ST_DATA_TX:
            if (csr0Val & CSR0_P_TXPKTRDY)
                break;
            
            {
                MUsbFnEp_t *pEP = &pPdd->ep[EP0];
                STransfer *pTransfer = pEP->pTransfer;
                UCHAR *pBuffer;
                DWORD space, txcount;
                
                if (pTransfer == NULL) 
                {
                    DEBUGMSG(ZONE_INFO, (L"Device_ProcessEP0:  error EP0_ST_DATA_TX pTransfer is NULL\r\n"));
                    break;
                }
                
                __try
                {
                    DWORD dwFlag = 0;
                    pBuffer =(UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;
                    space = pTransfer->cbBuffer - pTransfer->cbTransferred;
                    
                    
                    // How many bytes we can send just now?
                    txcount = pEP->maxPacketSize;
                    if (txcount > space)
                        txcount = space;
                    
                    DEBUGMSG(ZONE_INFO, (L"Device_ProcessEP0: EP0_ST_DATA_TX bytes_left_to_send=%d sending=%d\r\n", space, txcount));
                    
                    // Write data to FIFO
                    WriteFIFO(pPdd, EP0, pBuffer, txcount);
                    
                    // We transfered some data
                    pTransfer->cbTransferred += txcount;
                    dwFlag = CSR0_P_TXPKTRDY;               
                    
                    if (pTransfer->cbTransferred == pTransfer->cbBuffer)
                    {
                        pPdd->ep0State = EP0_ST_STATUS_OUT;
                        pEP->pTransfer = NULL;
                        pTransfer->dwUsbError = UFN_NO_ERROR;
                        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);   
                        DEBUGMSG(ZONE_INFO, (TEXT("Device_Process0: final copy Notify - transfer completed\r\n")));
                        dwFlag |= CSR0_P_DATAEND;
                    }       
                    
                    DEBUGMSG(ZONE_INFO, (TEXT("2. Device_ProcessEP0 Set TxPktRdy - 0\r\n")));
                    SETREG16(&pCsrRegs->ep[EP0].CSR.CSR0, dwFlag);
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;
                    DEBUGMSG(ZONE_ERROR, (L"Device_ProcessEP0: EP0_ST_DATA_TX"));
                }
            }
            break;            
            
        case EP0_ST_STATUS_OUT:
            // dummy state in processEP0
            break;
            
        case EP0_ST_ERROR:
            if (csr0Val & CSR0_P_SENTSTALL)
            {
                DEBUGMSG(ZONE_INFO, (TEXT("ST_ERROR\r\n")));
                pPdd->ep0State = EP0_ST_SETUP;
                CLRREG16(&pCsrRegs->ep[EP0].CSR.CSR0, CSR0_P_SENTSTALL);
            }
            break;
            
        default:
            break;
        }
cleanUp:                
        prtEP0State(pPdd);
        DEBUGMSG(ZONE_FUNCTION, (L"-Device_ProcessEP0\r\n"));
        
        return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//  Function:   Device_ProcessEPx_RX
//
//  Routine description:
//
//      Handle of Endpoint (1-15) receive (OUT transaction)
//      This function is called when IntrRX of corresponding endpoint is set.
//      This indicates the corresponding endpoint has data in corresponding FIFO/DMA.
//
//  Arguments:
//
//      pHSMUSBContext  :   Handle of HSMUSB_T
//      endpoint        :   endpoint to be processed
//      others          :   TBD
//
//  Return values:
//
//      ERROR_SUCCESS
//


DWORD Device_ProcessEPx_RX(PVOID pHSMUSBContext, DWORD endpoint)
{
    DWORD rc = ERROR_SUCCESS;
    PHSMUSB_T pUSBContext =(PHSMUSB_T) pHSMUSBContext;
    PCSP_MUSB_CSR_REGS pCsrRegs = pUSBContext->pUsbCsrRegs;
    MUsbFnPdd_t *pPdd = pUSBContext->pContext[DEVICE_CONTEXT];
    UINT16 csrVal;
    MUsbFnEp_t *pEP = &pPdd->ep[endpoint];
    UINT16 rxcount;
    STransfer *pTransfer = pEP->pTransfer;
    
    DEBUGMSG(ZONE_FUNCTION, (L"+Device_ProcessEPx_RX: endpoint=%d\r\n", endpoint));
    csrVal = INREG16(&pCsrRegs->ep[endpoint].RxCSR);
    rxcount = INREG16(&pCsrRegs->ep[endpoint].Count.RxCount);

    
    if (csrVal & RXCSR_P_SENTSTALL)
    {
        // MUSB finished sending STALL 
        // clear SENTSTALL 
        DEBUGMSG(1, (TEXT("Stall\r\n")));
        if (pEP != NULL)
           pEP->stalled = FALSE;
        CLRREG16(&pCsrRegs->ep[endpoint].RxCSR, RXCSR_P_SENTSTALL);
        csrVal = INREG16(&pCsrRegs->ep[endpoint].RxCSR);
    }
    
    if (csrVal & RXCSR_P_OVERRUN)
    {
        // clear OVERRUN 
        DEBUGMSG(ZONE_ERROR, (TEXT("Overrun\r\n")));
        CLRREG16(&pCsrRegs->ep[endpoint].RxCSR, RXCSR_P_OVERRUN);
        if (pTransfer != NULL)
        {
           pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;
           pEP->pTransfer = NULL;
           pPdd->pfnNotify(
               pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);
        }

        pPdd->intr_rx_data_avail &=(1 << endpoint);
        return ERROR_SUCCESS;
    }
    
    if (!(csrVal & RXCSR_P_RXPKTRDY))
    {
        return ERROR_SUCCESS;
    }
    
    if (pTransfer == NULL) 
    {
        DEBUGMSG(ZONE_ERROR, (L"!!! Device_ProcessEPx_RX:  pTransfer is NULL\r\n"));
        pPdd->intr_rx_data_avail |= (1 << endpoint);
        return ERROR_SUCCESS;
    }

    if (rxcount > pPdd->ep[endpoint].maxPacketSize)
    {
        rxcount = 0;
    }

    __try
    {
        UCHAR *pBuffer;

        if ((pPdd->ep[endpoint].dmaEnabled) &&
            ((pTransfer->cbBuffer - pTransfer->cbTransferred) > pEP->maxPacketSize) &&
            pPdd->bRXIsUsingUsbDMA)
        {
            UCHAR channel = MUSB_RX_DMA_CHN;
            DWORD size;

            pBuffer =(UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;
            EnterCriticalSection(&pPdd->dmaCS);
            if (INREG32(&pUSBContext->pUsbDmaRegs->dma[channel].Addr) == 0x00)
            {
                if (pPdd->rx0DmaState == MODE_DMA)
                {
                    DEBUGMSG(ZONE_INFO, (L"EP %d for channel %d not setup yet\r\n", endpoint, channel));
                    size = 0;
                    pPdd->intr_rx_data_avail |= (1 << endpoint);
                    LeaveCriticalSection(&pPdd->dmaCS);
                    goto cleanUp;
                }
                else
                {
                    DEBUGMSG(ZONE_DVFS, (TEXT("Process FIFO even though size larger than maxpacket size (rxcount[%d])\r\n"),
                        rxcount));
                    LeaveCriticalSection(&pPdd->dmaCS);
                    goto PROCESS_FIFO;
                }
            }

            size = pEP->dwRxDMASize - INREG32(&pUSBContext->pUsbDmaRegs->dma[channel].Count);

            LeaveCriticalSection(&pPdd->dmaCS);

            DEBUGMSG(ZONE_FUNCTION, (L"Device_ProcessEPx_Rx:RXDMA count:%d,start(0x%x),cur(0x%x),pvBuffer(0x%x),cbTransferred(%d),cbBuffer(%d),pBuffer(0x%x)\r\n",
                size, pPdd->paDmaRx0Buffer, INREG32(&pUSBContext->pUsbDmaRegs->dma[channel].Addr), pTransfer->pvBuffer, pTransfer->cbTransferred, pTransfer->cbBuffer, pBuffer));


            if ((pTransfer->cbTransferred + size + rxcount) < pEP->maxPacketSize)
            {
                CLRREG16(&pCsrRegs->ep[endpoint].RxCSR, RXCSR_P_AUTOCLEAR | RXCSR_P_DMAREQENAB | RXCSR_P_DMAREQMODE);

                EnterCriticalSection(&pPdd->dmaCS);
                pPdd->bDMAForRX = FALSE;
                LeaveCriticalSection(&pPdd->dmaCS);
            }

            if ((size > 0) && (pTransfer->dwBufferPhysicalAddress == 0))
                memcpy(pBuffer, pPdd->pDmaRx0Buffer , size);

            // Update buffer and clear zero length flag
            pTransfer->cbTransferred += size;

            ResetDMAChannel(pPdd, channel);
            EnterCriticalSection(&pPdd->csDVFS);
            if ((pPdd->ep[endpoint].dmaDVFSstate == DVFS_PREDMA) && pPdd->ep[endpoint].dmaEnabled)
            {
                pPdd->ep[endpoint].dmaDVFSstate = DVFS_POSTDMA;
                PostDmaDeactivation(pPdd, endpoint, RX_TRANSFER);
            }
            LeaveCriticalSection(&pPdd->csDVFS);


            DEBUGMSG(ZONE_FUNCTION, (L"Device_ProcessEPx_Rx:Tot transfer(%d), rxcount(%d), pBuffer to ReadFIFO(0x%x)\r\n",
                pTransfer->cbTransferred, rxcount, ((UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred)));
        }

PROCESS_FIFO:

        if(rxcount== pEP->maxPacketSize)
        {
            EnterCriticalSection(&pPdd->dmaCS);
            pPdd->bDMAForRX = TRUE;
            LeaveCriticalSection(&pPdd->dmaCS);
        }

        if((pTransfer->cbTransferred + rxcount) <= pTransfer->cbBuffer)
        {
            if (rxcount)
            {
                pBuffer =(UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;
            ReadFIFO(pPdd, (UCHAR)endpoint, pBuffer, (DWORD)rxcount);

                // We transfered some data
                pTransfer->cbTransferred += rxcount;
            }

            // clear intr_rx_data_avail EP bit
            pPdd->intr_rx_data_avail &= ~(1 << endpoint);
        }
        else if (rxcount)
        {
            // set intr_rx_data_avail EP bit
            pPdd->intr_rx_data_avail |= (1 << endpoint);
        }

        // Is this end of transfer?
        // I guess the system doesn't send out the zero-length packet from host.
        if ((rxcount < pPdd->ep[endpoint].maxPacketSize) ||(rxcount == 0) || (pTransfer->cbTransferred == pTransfer->cbBuffer))
        {
            // received last block
            DEBUGMSG(DEBUG_PRT_TRANS, (L"Device_ProcessEPx_RX: call pfnNotify COMPLETE endp:%d +\r\n", endpoint));

            // May need to add Critical Section for DVFS
            pTransfer->dwUsbError = UFN_NO_ERROR;
            pEP->pTransfer = NULL;
            //memdump((uchar *)pTransfer->pvBuffer, (unsigned short) pTransfer->cbTransferred, 0);
            // Don't clear RXCSR_P_PXPKTRDY here, let the next time it start to do that.
            if ((pPdd->ep[endpoint].dmaEnabled) && (pPdd->rx0DmaState == MODE_FIFO))
                pPdd->rx0DmaState = MODE_DMA;

            pEP->bLastRxUsedDMA = FALSE;

            pPdd->pfnNotify(
                pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);
            DEBUGMSG(DEBUG_PRT_TRANS, (L"Device_ProcessEPx_RX: call pfnNotify COMPLETE -\r\n"));
        }
        else
        {
            rc = StartRxDmaTransfer(pPdd, endpoint);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;
    }
    return ERROR_SUCCESS;

cleanUp:

    DEBUGMSG(ZONE_FUNCTION, (L"-Device_ProcessEPx_RX\r\n"));

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//  Function:   Device_ProcessEPx_TX
//
//  Routine description:
//
//      Handle of Endpoint (1-15) transmit (IN transaction)
//      This function is called when IntrTX of corresponding endpoint is set.
//      This indicates the TX has been completed successfully.
//
//  Arguments:
//
//      pHSMUSBContext  :   Handle of HSMUSB_T
//      endpoint        :   endpoint to be processed
//      others          :   TBD
//
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD Device_ProcessEPx_TX(PVOID pHSMUSBContext, DWORD endpoint)
{
    PHSMUSB_T pUSBContext =(PHSMUSB_T) pHSMUSBContext;
    PCSP_MUSB_CSR_REGS pCsrRegs = pUSBContext->pUsbCsrRegs;
    MUsbFnPdd_t *pPdd = pUSBContext->pContext[DEVICE_CONTEXT];
    MUsbFnEp_t *pEP = &pPdd->ep[endpoint];
    STransfer *pTransfer = pEP->pTransfer;
    UINT16 csrVal;
    
    DEBUGMSG(DEBUG_PRT_INFO, (L"+Device_ProcessEPx_TX: endpoint=%d\r\n", endpoint));
    csrVal = INREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR);
    
    if (csrVal & TXCSR_P_SENTSTALL)
    {
        // MUSB finished sending STALL 
        // clear SENTSTALL 
        DEBUGMSG(ZONE_INFO, (TEXT("Stall\r\n")));

        pEP->stalled = FALSE;
        CLRREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_SENTSTALL);
        csrVal = INREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR);
    }
    
    if (csrVal & TXCSR_P_UNDERRUN)
    {
        // clear UNDERRUN 
        DEBUGMSG(ZONE_INFO, (TEXT("Underrun\r\n")));
        CLRREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_UNDERRUN);
    }
    
    // check if finished sending packet (TXPKTRDY bit clear when finished)
    if (csrVal & TXCSR_P_TXPKTRDY) 
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("TXCSR_P_TXPKTRDY\r\n")));
        return ERROR_SUCCESS;
    }
    
    if (pTransfer == NULL) 
    {
        DEBUGMSG(ZONE_INFO, (L"Device_ProcessEPx_TX:  pTransfer is NULL\r\n"));
        return ERROR_SUCCESS;
    }
    
    __try
    {
        UCHAR *pBuffer;
        DWORD space, txcount;

        pBuffer =(UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;
        space = pTransfer->cbBuffer - pTransfer->cbTransferred;


        // How many bytes we can send just now?
        txcount = pEP->maxPacketSize;
        if (txcount > space)
            txcount = space;

        DEBUGMSG(ZONE_INFO, (L"Device_ProcessEPx_TX: bytes_left_to_send=%d sending=%d\r\n", space, txcount));

        if (txcount)
        {
            // Write data to FIFO
            WriteFIFO(pPdd, (UCHAR) endpoint, pBuffer, txcount);

            // We transfered some data
            pTransfer->cbTransferred += txcount;
            SETREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_TXPKTRDY);
            DEBUGMSG(DEBUG_PRT_TRANS, (L"EPX_TX sent %d bytes\r\n", txcount));
        }
         // don't send zero byte data packet to see if USB mass storage works
        else if (((pTransfer->cbTransferred % pEP->maxPacketSize) == 0) && (!pEP->bZeroLengthSent) && (!pEP->bMassStore))
        {
            // send zero-length end of packet
            SETREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_TXPKTRDY);

            pEP->bZeroLengthSent = TRUE;
        }
        else
        {
            // ACK_COMPLETE
            DEBUGMSG(DEBUG_PRT_TRANS, (L"Device_ProcessEPx_TX: call pfnNotify COMPLETE endp:%d +\r\n", endpoint));
            // May need to add Critical Section for DVFS
            if (pPdd->ep[endpoint].dmaEnabled && pPdd->bTXIsUsingUsbDMA)
            {
                ResetDMAChannel(pPdd, MUSB_TX_DMA_CHN);
                EnterCriticalSection(&pPdd->csDVFS);
                if (pPdd->ep[endpoint].dmaDVFSstate == DVFS_PREDMA)
                {
                    pPdd->ep[endpoint].dmaDVFSstate = DVFS_POSTDMA;
                    PostDmaDeactivation(pPdd, endpoint, TX_TRANSFER);
                }
                LeaveCriticalSection(&pPdd->csDVFS);
            }

            pEP->pTransfer = NULL;
            pTransfer->dwUsbError = UFN_NO_ERROR;

            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);
            //DEBUGMSG(DEBUG_PRT_TRANS, (L"Device_ProcessEPx_TX: call pfnNotify COMPLETE -\r\n"));
            //DEBUGMSG(ZONE_INFO, (TEXT("Device_ProcessEPx_TX: ACK_COMPLETE\r\n")));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;
        DEBUGMSG(ZONE_ERROR, (L"Device_ProcessEPx_TX: EXCEPTION_EXECUTE_HANDLER"));
    }
    
    DEBUGMSG(ZONE_FUNCTION, (L"-Device_ProcessEPx_TX\r\n"));
    
    return ERROR_SUCCESS;
}
//------------------------------------------------------------------------------
//  Function:   Device_ProcessDMA
//
//  Routine description:
//
//      Handle the DMA interrupt 
//
//  Arguments:
//
//      pHSMUSBContext  :   Handle of HSMUSB_T
//      channel         :   DMA channel
//
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD Device_ProcessDMA(PVOID pHSMUSBContext, UCHAR channel)
{
    MUsbFnPdd_t *pPdd =((PHSMUSB_T) pHSMUSBContext)->pContext[DEVICE_CONTEXT];
    PCSP_MUSB_CSR_REGS pCsrRegs = pPdd->pUSBContext->pUsbCsrRegs;
    PHSMUSB_T pOTG =(PHSMUSB_T)(pPdd->pUSBContext);
    MUsbFnEp_t *pEP;
    STransfer *pTransfer;
    
    DEBUGMSG(DEBUG_PRT_INFO, (L"Device_ProcessDMA: channel %d\r\n", channel));
    
    if (channel == MUSB_TX_DMA_CHN)
    {
        DWORD endpoint = pPdd->tx0DmaEp;
        
        pEP = &pPdd->ep[endpoint];
        pTransfer = pEP->pTransfer;

        if (pTransfer == NULL) 
        {
            DEBUGMSG(ZONE_DVFS, (L"Device_ProcessDMA:  error pTransfer is NULL\r\n"));
            return ERROR_INVALID_PARAMETER;
        }    

        DEBUGMSG(ZONE_FUNCTION, (TEXT("Device_ProcessDMA TX, EP %d Cntl(0x%x) pTransfer(0x%x), pvBuffer(0x%x), cbBuffer(0x%x), cbTransferred(0x%x)\r\n"),             
            endpoint, INREG32(&pOTG->pUsbDmaRegs->dma[channel].Cntl), pTransfer, pTransfer->pvBuffer, pTransfer->cbBuffer, pTransfer->cbTransferred));
        
        DEBUGMSG(DEBUG_PRT_INFO, (TEXT("Device_ProcessDMA: cbBuffer=%d  cbTransferred=%d\r\n"), pTransfer->cbBuffer, pTransfer->cbTransferred));
        
        // Done using DMA controller.  disable DMA related bits.
        //  When controller finishes sending FIFO data, should generate interrupt and call IST.
        CLRREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_AUTOSET | TXCSR_P_DMAREQENAB);

        // clear TXCSR_P_DMAREQMODE after clearing TXCSR_P_DMAREQENAB (per MUSBMHDRC programming guide)
        CLRREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_DMAREQMODE);

        ResetDMAChannel(pPdd, MUSB_TX_DMA_CHN);

        EnterCriticalSection(&pPdd->csDVFS);
        if (pPdd->ep[endpoint].dmaDVFSstate == DVFS_PREDMA)
        {
            pPdd->ep[endpoint].dmaDVFSstate = DVFS_POSTDMA;
            PostDmaDeactivation(pPdd, endpoint, TX_TRANSFER);
        }
        LeaveCriticalSection(&pPdd->csDVFS);

        if ((pTransfer->cbBuffer - pTransfer->cbTransferred) == 0)
        {
            // DMA finished loading FIFO with last packet in pTransfer buffer.  Manually set TXPKTRDY to let MUSB transmit it.

            DEBUGMSG(DEBUG_PRT_INFO, (TEXT("Device_ProcessDMA: no more data to send.  There should be data in FIFO.  Set TXPKTRDY. \r\n")));

            // test don't set TXPKTRDY and see if USB mass storage works (no zero length data packets)
            if(pEP->bMassStore)
            {
                // call ContinueTxDmaTransfer() to see if more data left in pTransfer buffer
                ContinueTxDmaTransfer(pPdd, endpoint);
            }
            else
            {
                SETREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_TXPKTRDY);
            }
        }
        // set up for next block of data to transmit
        //            if ((pTransfer->cbBuffer - pTransfer->cbTransferred) > pPdd->ep[endpoint].maxPacketSize)
        else   // set up DMA to copy next block of data to FIFO
        {
            // call ContinueTxDmaTransfer() to see if more data left in pTransfer buffer
            if (pEP->bTxDMAShortPacket == TRUE)
            {
                SETREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_TXPKTRDY);
                pEP->bTxDMAShortPacket = FALSE;
            }
            else
            {
                ContinueTxDmaTransfer(pPdd, endpoint);
            }
        }

    }
    else if (channel == MUSB_RX_DMA_CHN)
    {
        DWORD endpoint				= pPdd->rx0DmaEp;
        DWORD size					= 0;
        UCHAR *pBuffer				= NULL;
        PCSP_MUSB_CSR_REGS pCsrRegs = pPdd->pUSBContext->pUsbCsrRegs;
        WORD rxcount				= 0;
        UINT16 csrVal				= 0;

        csrVal = INREG16(&pCsrRegs->ep[endpoint].RxCSR);

        pEP = &pPdd->ep[endpoint];
        pTransfer = pEP->pTransfer;

        if (pTransfer == NULL)
        {
            DEBUGMSG(ZONE_DVFS, (L"Device_ProcessDMA:  error pTransfer is NULL\r\n"));
            return ERROR_INVALID_PARAMETER;
        }

        DEBUGMSG(ZONE_INFO, (TEXT("Device_ProcessDMA RX, EP %d Cntl(0x%x) pTransfer(0x%x), pvBuffer(0x%x), cbBuffer(0x%x), cbTransferred(0x%x)\r\n"),
            endpoint, INREG32(&pOTG->pUsbDmaRegs->dma[channel].Cntl), pTransfer, pTransfer->pvBuffer, pTransfer->cbBuffer, pTransfer->cbTransferred));

        __try
        {
            pBuffer =(UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;
            EnterCriticalSection(&pPdd->dmaCS);

            size = (INREG32(&pOTG->pUsbDmaRegs->dma[channel].Addr) == 0x00) ?
                    0 : pEP->dwRxDMASize - INREG32(&pOTG->pUsbDmaRegs->dma[channel].Count);

            LeaveCriticalSection(&pPdd->dmaCS);
            // According to the specification, you get this interrupt because the DMA COUNT = 0
            rxcount = INREG16(&pCsrRegs->ep[endpoint].Count.RxCount);

            if ((size > 0) && (pTransfer->dwBufferPhysicalAddress == 0))
                memcpy(pBuffer, pPdd->pDmaRx0Buffer , size);

            // Now ResetDMAChannel
            ResetDMAChannel(pPdd, channel);
            EnterCriticalSection(&pPdd->csDVFS);
            if (pPdd->ep[endpoint].dmaDVFSstate == DVFS_PREDMA)
            {
                pPdd->ep[endpoint].dmaDVFSstate = DVFS_POSTDMA;
                PostDmaDeactivation(pPdd, endpoint, RX_TRANSFER);
            }
            LeaveCriticalSection(&pPdd->csDVFS);

        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;
            DEBUGMSG(ZONE_ERROR, (L"UsbFnPdd!Device_ProcessDMA: UFN_CLIENT_BUFFER_ERROR"));
        }

        // Update buffer and clear zero length flag
        pTransfer->cbTransferred += size;

        pEP->bLastRxUsedDMA = TRUE;

        // test.  don't buffer.  pass everything received from each interrupt
        if ((pEP->dwRemainBuffer > 0) && (size % pPdd->ep[endpoint].maxPacketSize == 0)
            && /*(!rxcount) &&*/ (pPdd->rx0DmaState == MODE_DMA))
        {
            StartRxDmaTransfer(pPdd, endpoint);
        }
        else
        {
            pEP->dwRemainBuffer = 0;
            DEBUGMSG(ZONE_FUNCTION, (TEXT("Device_ProcessDMA rxcount(%d), size(%d), ReadFIFO pTransfer(0x%x), pBuffer(0x%x), cbBuffer(0x%x), cbTransferred(0x%x)\r\n"),
                size, rxcount, pTransfer, pTransfer->pvBuffer, pTransfer->cbBuffer, pTransfer->cbTransferred));
            if((pTransfer->cbTransferred + rxcount) <= pTransfer->cbBuffer)
            {
                if(rxcount)
                {
                    pBuffer =(UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;
                    ReadFIFO(pPdd, (UCHAR)endpoint, pBuffer, (DWORD)rxcount);

                    // We transfered some data
                    pTransfer->cbTransferred += rxcount;
                }

                // clear intr_rx_data_avail EP bit
                pPdd->intr_rx_data_avail &= ~(1 << endpoint);
            }
            else if (rxcount)
            {
                // set intr_rx_data_avail EP bit
                pPdd->intr_rx_data_avail |= (1 << endpoint);
            }

            CLRREG16(&pCsrRegs->ep[endpoint].RxCSR, RXCSR_P_AUTOCLEAR | RXCSR_P_DMAREQENAB | RXCSR_P_DMAREQMODE);

            DEBUGMSG(ZONE_INFO, (L"Device_ProcessDMA: RX DMA buffer count:%d   FIFO count:%d\r\n", size, rxcount));
            {
                DEBUGMSG(DEBUG_PRT_TRANS, (L"Device_ProcessDMA: call pfnNotify COMPLETE endp:%d +\r\n", endpoint));

                DEBUGMSG(DEBUG_PRT_TRANS, (L"UsbFnPdd!Device_ProcessDMA: "
                    L"EP %d pTransfer 0x%08x (%d, %d, %d) - done\r\n",
                    endpoint, pTransfer, pTransfer != NULL ? pTransfer->cbBuffer : 0,
                    pTransfer != NULL ? pTransfer->cbTransferred : 0,
                    pTransfer != NULL ? pTransfer->dwUsbError : -1
                    ));
                DEBUGMSG(DEBUG_PRT_TRANS, (TEXT("ACK: ProcessDMATransfer for EP %d\r\n"), endpoint));
                pEP->pTransfer = NULL;
                pTransfer->dwUsbError = UFN_NO_ERROR;

                pPdd->pfnNotify(
                    pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer
                    );

            }
        }
    }

    return ERROR_SUCCESS;
}
//------------------------------------------------------------------------------
//  Function:   Device_Disconnect
//
//  Routine description:
//
//      Handle of Disconnect interrupt
//
//  Arguments:
//
//      pHSMUSBContext  :   Handle of HSMUSB_T
//      others          :   TBD
//
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD Device_Disconnect(PVOID pHSMUSBContext)
{
    MUsbFnPdd_t *pPdd =((PHSMUSB_T) pHSMUSBContext)->pContext[DEVICE_CONTEXT];
    PHSMUSB_T pOTG =(PHSMUSB_T)(pPdd->pUSBContext);    
    DEBUGMSG(ZONE_FUNCTION, (L"Device_Disconnect\r\n"));

    SetEventData( pPdd->hVbusChargeEvent,  BATTERY_USBHOST_DISCONNECT );  
    SetEvent( pPdd->hVbusChargeEvent );

    // We are not configured anymore
    // Let MDD process change
    pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);
    // Set internal power state to D3
    pPdd->devState &= ~(MUSB_DEVSTAT_ATT | 
                        MUSB_DEVSTAT_B_HNP_ENABLE | 
                        MUSB_DEVSTAT_A_HNP_SUPPORT |
                        MUSB_DEVSTAT_A_ALT_HNP_SUPPORT);

    pPdd->selfPowerState = D3;

    EnterCriticalSection(&pPdd->powerStateCS);
    pPdd->fPowerDown = TRUE;
    if(pPdd->dwUSBFNState == USBFN_ACTIVE)
    {
        LeaveCriticalSection(&pPdd->powerStateCS);
        WaitForSingleObject(pPdd->hPowerDownEvent,INFINITE);
    }
    else
    {
        LeaveCriticalSection(&pPdd->powerStateCS);
    }
    if ((pOTG->connect_status & CONN_DC) == 0x00)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("Client Mode Complete Disconnect\r\n")));
        pOTG->connect_status |= CONN_DC;
        SetEvent(pOTG->hSysIntrEvent);
    }
    
    return ERROR_SUCCESS;
}
//------------------------------------------------------------------------------
//  Function:   Device_Suspend
//
//  Routine description:
//
//      Handle of suspend interrupt
//
//  Arguments:
//
//      pHSMUSBContext  :   Handle of HSMUSB_T
//      others          :   TBD
//
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD Device_Suspend(PVOID pHSMUSBContext)
{
	UNREFERENCED_PARAMETER(pHSMUSBContext);

    return ERROR_INVALID_PARAMETER;
}

//----------------------- Microsoft UfnPDD -------------------------------------------------------
//
//  Function:  UfnPdd_PowerDown
//
//  Routine description:
//
//      This function is called in system suspend.
//
//  Arguments:
//
//      pPddContext:: Handle to HSUSBFNPDD_T
//
//  Return values:
//
//      Nothing
//
VOID WINAPI
UfnPdd_PowerDown(
    PVOID pPddContext
)
{
	UNREFERENCED_PARAMETER(pPddContext);

    DEBUGMSG(ZONE_INFO, (TEXT("+UfnPdd_PowerDown\r\n")));
#if 0
    MUsbFnPdd_t *pPdd = pPddContext;
    PHSMUSB_T pOTG = (PHSMUSB_T)(pPdd->pUSBContext);
    DWORD currtime;
    DWORD starttime;


    if (pOTG->operateMode != DEVICE_MODE)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("UfnPdd_PowerDown not in device mode, it is %d\r\n"), pOTG->operateMode));
        return;
    }

    /*
        Don't handle this musbotg.dll takes care of the clocks.
    */

    //CLRREG8(&pOTG->pUsbGenRegs->DevCtl, DEVCTL_SESSION);
    //RETAILMSG(1, (TEXT("DevCtl Power Down = 0x%x\r\n"), INREG8(&pOTG->pUsbGenRegs->DevCtl)));
    //CLRREG8(&pOTG->pUsbGenRegs->Power, POWER_SOFTCONN);
    //SETREG8(&pOTG->pUsbGenRegs->Power, POWER_EN_SUSPENDM);
    //RETAILMSG(1, (TEXT("Clear the Soft Conn\r\n")));
    pPdd->pfnEnUSBClock(FALSE);

    // Wait for one second
    starttime = GetTickCount();
    do {
        currtime = GetTickCount();
        if (currtime < starttime)
            starttime = currtime;
    } while (currtime - starttime < 1000);
#endif
    DEBUGMSG(ZONE_INFO, (TEXT("-UfnPdd_POwerDown\r\n")));
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_PowerUp
//  
//  Routine description:
//
//      This function is called in resume from suspend
//
//  Arguments:
//
//      pPddContext:: Handle to HSUSBFNPDD_T
//
//  Return values:
//
//      Nothing
//
VOID WINAPI
UfnPdd_PowerUp(
    PVOID pPddContext
)
{
	UNREFERENCED_PARAMETER(pPddContext);

    /*
        DONOT handle here musbotg.dll takes care of clocks.
    */
#if 0
    MUsbFnPdd_t *pPdd = pPddContext;
    PHSMUSB_T pOTG = (PHSMUSB_T)(pPdd->pUSBContext);

    if (pOTG->operateMode != DEVICE_MODE)
        return;

    DEBUGMSG(ZONE_POWER, (L"UfnPdd_PowerUp\r\n"));
    RETAILMSG(TRUE, (L"UfnPdd_PowerUp\r\n"));
    pPdd->pfnEnUSBClock(TRUE);

    SETREG8(&pOTG->pUsbGenRegs->Power, POWER_SOFTCONN);
#endif
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IssueTransfer
//
//  Routine description:
//
//      This function is the handling of transferring data from and to the host. 
//
//  Arguments:
//
//      pPddContext :: Handle to HSUSBFNPDD_T
//      endpoint    :: Endpoint to be used to transfer data
//      pTransfer   :: Pointer to STransfer passing from MDD, containing buffer to read/write.
//
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_IssueTransfer(
    VOID *pPddContext,
    DWORD endpoint,
    STransfer *pTransfer
)
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    MUsbFnPdd_t *pPdd = pPddContext;
    PHSMUSB_T pUSBContext =(PHSMUSB_T) pPdd->pUSBContext;
    PCSP_MUSB_CSR_REGS pCsrRegs = pUSBContext->pUsbCsrRegs;
    
    
    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_IssueTransfer\r\n"));
    
    EnterCriticalSection(&pPdd->powerStateCS);
    if(pPdd->fPowerDown == TRUE)
    {
        pTransfer->dwUsbError = UFN_CANCELED_ERROR;
        LeaveCriticalSection(&pPdd->powerStateCS);
        return ERROR_SUCCESS;

    }
    else
    {
        pPdd->dwUSBFNState = USBFN_ACTIVE;
        LeaveCriticalSection(&pPdd->powerStateCS);
    }

    if (pTransfer == NULL) 
    {
        DEBUGMSG(ZONE_ERROR, (L"UfnPdd_IssueTransfer:  error pTransfer is NULL\r\n"));
        return ERROR_INVALID_PARAMETER;
    }    
    
    // Save transfer for interrupt thread
    ASSERT(pPdd->ep[endpoint].pTransfer == NULL);
    pPdd->ep[endpoint].pTransfer = pTransfer;
    
    // If transfer buffer is NULL length must be zero
    if (pTransfer->pvBuffer == NULL)
        pTransfer->cbBuffer = 0;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("IssueTransfer: ep(%d), length(%d), (%s) max packet(%d)\r\n"), 
        endpoint, pTransfer->cbBuffer, (TRANSFER_IS_IN(pTransfer)? TEXT("IN"): TEXT("OUT")), pPdd->ep[endpoint].maxPacketSize));

    DEBUGCHK(pTransfer->dwUsbError == UFN_NOT_COMPLETE_ERROR);
    
    if ((endpoint == 0) &&(pPdd->ep0State == EP0_ST_SETUP_PROCESSED))
    {
        SETREG16(&pCsrRegs->ep[endpoint].CSR.CSR0, CSR0_P_SERVICEDRXPKTRDY);
    }
    
    // Depending on direction
    if (TRANSFER_IS_IN(pTransfer))
    {

       // don't send zeroLength data (see if USB mass storage works)
        if(pPdd->ep[endpoint].bMassStore)
        {
            pPdd->ep[endpoint].zeroLength = FALSE;
        }
        else
        {
            pPdd->ep[endpoint].zeroLength =(pTransfer->cbBuffer == 0);
        }

            rc = ContinueTxDmaTransfer(pPdd, endpoint);
        }
        else
        {
        pPdd->ep[endpoint].zeroLength = FALSE;
            rc = StartRxDmaTransfer(pPdd, endpoint);
        }

    EnterCriticalSection(&pPdd->powerStateCS);
    pPdd->dwUSBFNState = USBFN_IDLE;
    if(pPdd->fPowerDown == TRUE)
    {
        SetEvent(pPdd->hPowerDownEvent);
    }
    LeaveCriticalSection(&pPdd->powerStateCS);
    
    return rc;
    
    //    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_AbortTransfer
//
//  Routine description:
//
//      This routine is called when the system aborts the transferred.
//
//  Arguments:
//
//      pPddContext ::  Handle to HSUSBFNPDD_T
//      endpoint    ::  Endpoint in which the transfer to be aborted
//      pTransfer   ::  Pointer to STransfer passing from MDD, containing buffer to be aborted.
//
//  Return values:
//  
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_AbortTransfer(
    VOID *pPddContext,
    DWORD endpoint,
    STransfer *pTransfer
)
{
    MUsbFnPdd_t *pPdd = pPddContext;
    PCSP_MUSB_CSR_REGS pCsrRegs = pPdd->pUSBContext->pUsbCsrRegs;
    PHSMUSB_T pOTG = pPdd->pUSBContext;

    EnterCriticalSection(&pOTG->regCS);
    if(pOTG->bClockStatus)
    {
    if (endpoint == 0)
    {
        SETREG16(&pCsrRegs->ep[endpoint].CSR.CSR0, CSR0_P_FLUSHFIFO);
    }
    else
    {
        MUsbFnEp_t *pEP = &pPdd->ep[endpoint];

        if (pEP->dirRx)
            SETREG16(&pCsrRegs->ep[endpoint].RxCSR, RXCSR_P_FLUSHFIFO);
        else
            SETREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_FLUSHFIFO);
    }

    if ((pPdd->rx0DmaEp == endpoint) && (pPdd->ep[endpoint].dmaEnabled))
        ResetDMAChannel(pPdd, MUSB_RX_DMA_CHN);
    else if ((pPdd->tx0DmaEp == endpoint) && (pPdd->ep[endpoint].dmaEnabled))
        ResetDMAChannel(pPdd, MUSB_TX_DMA_CHN);
    }
    LeaveCriticalSection(&pOTG->regCS);

    EnterCriticalSection(&pPdd->csDVFS);
    if ((pPdd->ep[endpoint].dmaDVFSstate == DVFS_PREDMA) && pPdd->ep[endpoint].dmaEnabled)
    {
        MUsbFnEp_t *pEP = &pPdd->ep[endpoint];
        pEP->dmaDVFSstate = DVFS_POSTDMA;
        if (pEP->dirRx)
            PostDmaDeactivation(pPdd, endpoint, RX_TRANSFER);
        else
            PostDmaDeactivation(pPdd, endpoint, TX_TRANSFER);
    }
    LeaveCriticalSection(&pPdd->csDVFS);

    // Finish transfer
    pPdd->ep[endpoint].pTransfer = NULL;
    if (pTransfer != NULL)
    {
        pTransfer->dwUsbError = UFN_CANCELED_ERROR;
        DEBUGMSG(ZONE_INFO, (TEXT("UfnPdd_AbortTransfer for EP %d\r\n"), endpoint));
        pPdd->pfnNotify(
           pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer
           );
    }
    
    
    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_StallEndpoint
//
//  Routine description:
//
//      This function is called by MDD to set end point to stall mode (halted).
//
//  Arguments:
//
//      pPddContext ::  Handle to HSUSBFNPDD_T
//      endpoint    ::  The endpoint to which it is requested to stall
//
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_StallEndpoint(
    VOID *pPddContext,
    DWORD endpoint
)
{
    MUsbFnPdd_t *pPdd = pPddContext;
    MUsbFnEp_t *pEP = &pPdd->ep[endpoint];
    PCSP_MUSB_CSR_REGS pCsrRegs = pPdd->pUSBContext->pUsbCsrRegs;
    
    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_StallEndpoint\r\n"));

    DEBUGMSG(ZONE_INFO, (L"*** UfnPdd_StallEndpoint  endpoint %d\r\n",endpoint));
    
    if (endpoint == 0)
    {
        // Stall next EP0 transaction
        SETREG16(&pCsrRegs->ep[endpoint].CSR.CSR0, CSR0_P_SENDSTALL);
    }
    else
    {
        
        // stall end point
        if (pEP->dirRx)
            SETREG16(&pCsrRegs->ep[endpoint].RxCSR, RXCSR_P_SENDSTALL);        
        else
            SETREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_SENDSTALL);        
        
    }
    pEP->stalled = TRUE;
    
    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_ClearEndpointStall
//
//  Routine descriptions:
//
//      This function is called by MDD to clear end point stall mode (halted).
//
//  Arguments:
//
//      pPddContext ::  Handle to HSUSBFNPDD_T
//      endpoint    ::  The endpoint to which it is requested to stall
//
//  Return values:
//
//      ERROR_SUCCESS
//      
DWORD WINAPI
UfnPdd_ClearEndpointStall(
    VOID *pPddContext,
    DWORD endpoint
)
{
    MUsbFnPdd_t *pPdd = pPddContext;
    MUsbFnEp_t *pEP = &pPdd->ep[endpoint];
    PCSP_MUSB_CSR_REGS pCsrRegs = pPdd->pUSBContext->pUsbCsrRegs;
    UINT16 csrVal;
    
    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_ClearEndpointStall for EP %d\r\n", endpoint));
    
    if (endpoint == 0)
    {
        CLRREG16(&pCsrRegs->ep[endpoint].CSR.CSR0, CSR0_P_SENTSTALL | CSR0_P_SENDSTALL);        
    }
    else
    {
        // stall end point
        if (pEP->dirRx)
        {
            csrVal = INREG16(&pCsrRegs->ep[endpoint].RxCSR);

            if(csrVal & RXCSR_P_SENTSTALL)
            {
                csrVal &= ~(RXCSR_P_SENDSTALL | RXCSR_P_SENTSTALL);
                csrVal |= RXCSR_P_CLRDATATOG;

                OUTREG16(&pCsrRegs->ep[endpoint].RxCSR, csrVal);
            }
        }
        else
        {
            csrVal = INREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR);
                        					    
			if (csrVal & TXCSR_P_SENTSTALL)
            {
				csrVal &= ~(TXCSR_P_SENDSTALL | TXCSR_P_SENTSTALL);
                csrVal |= TXCSR_P_CLRDATATOG;

                OUTREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, csrVal);
            }
        }
        
    }
    pEP->stalled = FALSE;
    
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IsEndpointHalted
//
//  Routine descriptions:
//  
//      Check and see if the endpoint has been stalled.
//
//  Arguments:
//
//      pPddContext :: Handle to HSUSBFNPDD_T
//      endpoint    :: endpoint to check if it is halted.
//      pHalted     :: Pointer to which the status of endpoint to be returned.
//
DWORD WINAPI
UfnPdd_IsEndpointHalted(
    VOID *pPddContext,
    DWORD endpoint,
    BOOL *pHalted
)
{
   
    MUsbFnPdd_t *pPdd = pPddContext;
    MUsbFnEp_t *pEP = &pPdd->ep[endpoint];
    PCSP_MUSB_CSR_REGS pCsrRegs = pPdd->pUSBContext->pUsbCsrRegs;
    
    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_IsEndpointHalted\r\n"));
        
    if (endpoint == 0)
    {
        *pHalted = INREG16(&pCsrRegs->ep[endpoint].CSR.CSR0) & CSR0_P_SENDSTALL;
    }
    else
    {
        // stall end point
        if (pEP->dirRx)
        {
            *pHalted = INREG16(&pCsrRegs->ep[endpoint].RxCSR) & RXCSR_P_SENDSTALL;
        }
        else
        {
            *pHalted = INREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR) & TXCSR_P_SENDSTALL;
        }
    }
        
    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_SendControlStatusHandshake
//
//  Routine descriptions:
//
//      This function is the handling of the status phase in Control Transfer. 
//  
//  Arguments:
//  
//      pPddContext ::  Handle of HSUSBFNPDD_T
//      endpoint    ::  the endpoint to be processed.
//
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_SendControlStatusHandshake(
    VOID *pPddContext,
    DWORD endpoint
)
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    MUsbFnPdd_t *pPdd = pPddContext;
    PCSP_MUSB_CSR_REGS pCsrRegs = pPdd->pUSBContext->pUsbCsrRegs;
    
    DEBUGMSG(ZONE_FUNCTION, (L"+UfnPdd_SendControlStatusHandshake status = 0x%x\r\n", pPdd->ep0State));
    
    if (endpoint == 0)
    {
        if (pPdd->ep0State == EP0_ST_SETUP_PROCESSED)
        {
            DEBUGMSG(ZONE_INFO, (TEXT("SETUP_PROCESSED => STATUS_IN\r\n")));
            SETREG16(&pCsrRegs->ep[endpoint].CSR.CSR0, CSR0_P_SERVICEDRXPKTRDY | CSR0_P_DATAEND);
            // Send zero byte to ack
            SETREG16(&pCsrRegs->ep[endpoint].CSR.CSR0, CSR0_P_TXPKTRDY);
            pPdd->ep0State = EP0_ST_STATUS_IN;
        }

        // Send out zero byte data
        if (pPdd->ep0State == EP0_ST_STATUS_IN)
        {
            DEBUGMSG(ZONE_INFO, (L"Status In Phase\r\n"));
            DEBUGMSG(ZONE_INFO, (TEXT("1. Set TxPktRdy - 0\r\n")));
        }
        else if (pPdd->ep0State == EP0_ST_STATUS_OUT)
        {
            DEBUGMSG(ZONE_INFO, (L"Status Out Phase\r\n"));
        }
        else
            DEBUGMSG(ZONE_INFO, (L"SendControlStatusHandshake error with state %d\r\n", 
            pPdd->ep0State));
        if (pPdd->ep0State == EP0_ST_STATUS_OUT)
            pPdd->ep0State = EP0_ST_SETUP;
        DEBUGMSG(ZONE_INFO, (L"Complete\r\n"));
        rc = ERROR_SUCCESS;
        
        prtEP0State(pPdd);
    }
    else
        DEBUGMSG(ZONE_INFO, (L"!!! UfnPdd_SendControlStatusHandshake called with endpoint %d\r\n", endpoint));
    
    DEBUGMSG(ZONE_FUNCTION, (L"-UfnPdd_SendControlStatusHandshake\r\n"));
    
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_InitiateRemoteWakeup
//
//  Routine descriptions:
//  
//      This function is to handle the RemoteWakeUp request from MDD. It would
//      send the resume signal on the USB bus to wakeup the host.
//
//  Arguments:
//
//      pPddContext ::  Handle of HSUSBFNPDD_T
//
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_InitiateRemoteWakeup(
    VOID *pPddContext
)
{
	UNREFERENCED_PARAMETER(pPddContext);

    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_InitiateRemoteWakeup\r\n"));
    return ERROR_INVALID_PARAMETER;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IOControl
//
//  Routine descriptions:
//
//      IOControl called for UfnPdd request from MDD
//
//  Arguments:
//
//      pPddContext ::  Handle of HSUSBFNPDD_T
//      source      ::  IO Control Source
//      code        ::  IO Control Request
//      pInBuffer   ::  Buffer passing in the data from MDD
//      inSize      ::  size of pInBuffer
//      pOutBuffer  ::  Buffer which the data to be return to MDD
//      outSize     ::  Buffer size available
//      pOutSize    ::  actual buffer data size
//
DWORD WINAPI
UfnPdd_IOControl(
    VOID *pPddContext,
    IOCTL_SOURCE source,
    DWORD code,
    UCHAR *pInBuffer,
    DWORD inSize,
    UCHAR *pOutBuffer,
    DWORD outSize,
    DWORD *pOutSize
)
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    MUsbFnPdd_t *pPdd = pPddContext;
    UFN_PDD_INFO info;
    CE_BUS_POWER_STATE *pBusPowerState;

	UNREFERENCED_PARAMETER(pOutSize);

    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_IOControl\r\n"));
    
    switch (code)
    {

#if 0  // WM7 specific
        case IOCTL_UFN_PDD_SET_INTERFACE: {
            DWORD dwParam;
            DWORD dwInterface;
            DWORD dwAlternateSetting;

            if ( source != BUS_IOCTL ||
                 pOutBuffer != NULL || outSize != 0 ||
                 pInBuffer == NULL || inSize != sizeof(DWORD) ) {
                break;
            }

            dwParam = *(DWORD*)pInBuffer;
            dwInterface = UFN_GET_INTERFACE_NUMBER( dwParam );
            dwAlternateSetting = UFN_GET_INTERFACE_ALTERNATE_SETTING( dwParam );
            break;
        }

        case IOCTL_UFN_PDD_SET_CONFIGURATION: {
            DWORD dwConfiguration;

            if ( source != BUS_IOCTL ||
                 pOutBuffer != NULL || outSize != 0 ||
                 pInBuffer == NULL || inSize != sizeof(DWORD) ) {
                break;
            }

            dwConfiguration = *(DWORD*)pInBuffer;

            break;
        }
#endif
        case IOCTL_UFN_GET_PDD_INFO:
            if (source != BUS_IOCTL)
                break;
            if (pOutBuffer == NULL || outSize < sizeof(UFN_PDD_INFO))
                break;
            info.InterfaceType = Internal;
            info.BusNumber = 0;
            info.dwAlignment = sizeof(DWORD);
            if (!CeSafeCopyMemory(
                pOutBuffer, &info, sizeof(UFN_PDD_INFO)
                ))
                break;
            rc = ERROR_SUCCESS;
            break;
        case IOCTL_BUS_GET_POWER_STATE:
            if (source != MDD_IOCTL)
                break;
            if (pInBuffer == NULL || inSize < sizeof(CE_BUS_POWER_STATE))
                break;
            pBusPowerState =(CE_BUS_POWER_STATE*)pInBuffer;
            if (!CeSafeCopyMemory(
                pBusPowerState->lpceDevicePowerState, &pPdd->pmPowerState,
                sizeof(CEDEVICE_POWER_STATE)
                ))
                break;
            rc = ERROR_SUCCESS;
            break;
        case IOCTL_BUS_SET_POWER_STATE:
#if 0
            if (source == MDD_IOCTL)
                break;
            if (pInBuffer == NULL || inSize < sizeof(CE_BUS_POWER_STATE))
                break;
            pBusPowerState =(CE_BUS_POWER_STATE*)pInBuffer;
            if (!CeSafeCopyMemory(
                &devicePowerState, pBusPowerState->lpceDevicePowerState,
                sizeof(CEDEVICE_POWER_STATE)
                ))
                break;
            SetPowerState(pPdd , devicePowerState);
#endif
            rc = ERROR_SUCCESS;
            break;

#if 0
        case IOCTL_DVFS_OPMNOTIFY:
        {
            IOCTL_DVFS_OPMNOTIFY_IN *pData =(IOCTL_DVFS_OPMNOTIFY_IN*)pInBuffer;
            
            DEBUGMSG(ZONE_DVFS, (L"HSUSBFN: received dvfs notification (%d)\r\n",
                pData->notification)
                );
            
            // this operation should be atomic to handle a corner case
            EnterCriticalSection(&pPdd->csDVFS);
            
            // signal dvfs thread to stall SDRAM access
            if (pData->notification == kPreNotice)
            {
                pPdd->bDVFSActive = TRUE;
                pPdd->bDVFSAck = FALSE;
                
                // check and halt dma if active
                //
                DEBUGMSG(ZONE_DVFS, (L"HSUSBFN: Halting DMA for DVFS, "
                    L"active dma count=%d\r\n",
                    pPdd->nActiveDmaCount)
                    );
                CheckAndHaltAllDma(pPdd, TRUE);                
            }
            else if (pData->notification == kPostNotice)
            {
                pPdd->bDVFSActive = FALSE;
                pPdd->bDVFSAck = FALSE;
                                
                CheckAndHaltAllDma(pPdd, FALSE);
                DEBUGMSG(ZONE_DVFS, (L"HSUSBFN: Post-DVFS transition done\r\n"));
            }
            LeaveCriticalSection(&pPdd->csDVFS);
            rc = ERROR_SUCCESS;
        }
            break;

        case IOCTL_DVFS_INITINFO:
        {
            IOCTL_DVFS_INITINFO_OUT *pInitInfo =(IOCTL_DVFS_INITINFO_OUT*)pOutBuffer;
            pInitInfo->notifyMode = kAsynchronous;
            pInitInfo->notifyOrder = pPdd->nDVFSOrder;
            rc = ERROR_SUCCESS;
        }
            break;
        
        case IOCTL_DVFS_OPMINFO:
        {
            IOCTL_DVFS_OPMINFO_IN *pData =(IOCTL_DVFS_OPMINFO_IN*)pInBuffer;
            CopyDVFSHandles(pPdd, pData->processId, 
                pData->hAckEvent, pData->hOpmEvent
                );
            rc = ERROR_SUCCESS;
        }
            break;

        case IOCTL_DVFS_DETACH:
        {
            // close all handles
            if (pPdd->hDVFSAckEvent != NULL)
            {
                CloseHandle(pPdd->hDVFSAckEvent);
                pPdd->hDVFSAckEvent = NULL;
            }
            
            if (pPdd->hDVFSActivityEvent!= NULL)
            {
                CloseHandle(pPdd->hDVFSActivityEvent);
                pPdd->hDVFSActivityEvent = NULL;
            }

            pPdd->bDVFSAck = FALSE;
            rc = ERROR_SUCCESS;
        }
            break;

        case IOCTL_DVFS_HALTMODE:
        {
            if (pPdd->bDVFSActive == FALSE)
            {                
                IOCTL_DVFS_HALTMODE_IN *pData =(IOCTL_DVFS_HALTMODE_IN*)pInBuffer;
                pPdd->rxHaltMode = pData->rxMode;
                pPdd->txHaltMode = pData->txMode;
                rc = ERROR_SUCCESS;
            }
            break;
        }
#endif
    }
    
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_Deinit
//
//  Routine descriptions:
//  
//      De-init routine when it is unloaded.
//
//  Arguments:
//
//      pPddContext ::  Handle of HSUSBFNPDD_T
//  
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_Deinit(
    VOID *pPddContext
)
{
    MUsbFnPdd_t *pPdd = pPddContext;
    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_Deinit\r\n"));

    if( pPdd->hVbusChargeEvent != INVALID_HANDLE_VALUE )
    {
        CloseHandle(pPdd->hVbusChargeEvent);
        pPdd->hVbusChargeEvent = INVALID_HANDLE_VALUE;
    }    

    // Release DMA Rx0 buffer...
    if (pPdd->pDmaRx0Buffer != NULL)
    {
        FreePhysMem(pPdd->pDmaRx0Buffer);
        pPdd->pDmaRx0Buffer = NULL;
        pPdd->paDmaRx0Buffer = 0;
    }
    
    // Release DMA Tx0 buffer...
    if (pPdd->pDmaTx0Buffer != NULL)
    {
        FreePhysMem(pPdd->pDmaTx0Buffer);
        pPdd->pDmaTx0Buffer = NULL;
        pPdd->paDmaTx0Buffer = 0;
    }
    
    if (pPdd->pCachedDmaTx0Buffer != NULL)
    {
        VirtualFree(pPdd->pCachedDmaTx0Buffer, pPdd->tx0DmaBufferSize, MEM_RELEASE);
        pPdd->pCachedDmaTx0Buffer = NULL;
    }
    
    pPdd->bDVFSAck = FALSE;
    // release dvfs resources
    if (pPdd->hDVFSAckEvent != NULL) CloseHandle(pPdd->hDVFSAckEvent);
    if (pPdd->hDVFSActivityEvent != NULL) CloseHandle(pPdd->hDVFSActivityEvent);

    DeleteCriticalSection(&pPdd->csDVFS);


    DeleteCriticalSection(&pPdd->dmaCS);
    // Delete critical section
    DeleteCriticalSection(&pPdd->epCS);
    
    // Free PDD context
    LocalFree(pPdd);
    
    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_DeregisterDevice
//
//  Routine descriptions:
//
//      This function is called by MDD to move device to pre-registred state.
//  On OMAP730 we simply disable all end points.
//
//  Arguments:
//
//      pPddContext ::  Handle of HSUSBFNPDD_T
//  
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_DeregisterDevice(
    VOID *pPddContext
)
{
    MUsbFnPdd_t *pPdd = pPddContext;
    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_DeregisterDevice\r\n"));
    
    if (pPdd->tx0DmaEp != -1)
    {
        pPdd->ep[pPdd->tx0DmaEp].dmaEnabled = FALSE;
    }
    
    if (pPdd->rx0DmaEp != -1)
    {
        pPdd->ep[pPdd->rx0DmaEp].dmaEnabled = FALSE;
    }
    
    pPdd->tx0DmaEp = (DWORD)-1;
    pPdd->rx0DmaEp = (DWORD)-1;
        
    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_Stop
//
//  Routine descriptions:
//
//      This function is called before UfnPdd_DeregisterDevice. It should de-attach
//  device to USB bus (but we don't want disable interrupts because...)
//
//  Arguments:
//
//      pPddContext ::  Handle of HSUSBFNPDD_T
//  
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_Stop(
    VOID *pPddContext
)
{
    MUsbFnPdd_t *pPdd = pPddContext;    
    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_Stop\r\n"));
    pPdd->devState &= ~(MUSB_DEVSTAT_ATT | 
                        MUSB_DEVSTAT_B_HNP_ENABLE | 
                        MUSB_DEVSTAT_A_HNP_SUPPORT |
                        MUSB_DEVSTAT_A_ALT_HNP_SUPPORT);

    BusChildIoControl(pPdd->hParentBus,IOCTL_USBOTG_REQUEST_STOP,NULL,0);    

    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_DeinitEndpoint
//
//  Routine descriptions:
//
//      This function is called when pipe to endpoit is closed. We stall
//  endpoints there...
//
//  Arguments:
//
//      pPddContext ::  Handle of HSUSBFNPDD_T
//      endpoint    ::  Endpoint to be deinit.
//  
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_DeinitEndpoint(
    VOID *pPddContext,
    DWORD endpoint
)
{
    MUsbFnPdd_t *pPdd = pPddContext;
    PCSP_MUSB_GEN_REGS pGenRegs = pPdd->pUSBContext->pUsbGenRegs;
    PCSP_MUSB_CSR_REGS pCsrRegs = pPdd->pUSBContext->pUsbCsrRegs;
    PHSMUSB_T pOTG = pPdd->pUSBContext;
    DEBUGMSG(ZONE_INFO, (L"UfnPdd_DeinitEndpoint: endpoint %d\r\n", endpoint));

    EnterCriticalSection(&pOTG->regCS);
    if(pOTG->bClockStatus)
    {
    /* Flush the fifo, otherwise transmit error on the host during Device->=Host HNP */
    OUTREG16(&pGenRegs->Index, endpoint);  /* select endpoint to access rxFIFOxx registers */
    OUTREG16(&pCsrRegs->ep[endpoint].RxCSR, RXCSR_P_FLUSHFIFO | RXCSR_P_CLRDATATOG);
    OUTREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_FLUSHFIFO | TXCSR_P_CLRDATATOG);
    }
    LeaveCriticalSection(&pOTG->regCS);
    
    if (pPdd->tx0DmaEp == endpoint)
    {
        pPdd->ep[endpoint].dmaEnabled = FALSE;
        pPdd->tx0DmaEp = (DWORD)-1;
    }
    
    if (pPdd->rx0DmaEp == endpoint)
    {
        pPdd->ep[endpoint].dmaEnabled = FALSE;
        pPdd->rx0DmaEp = (DWORD)-1;
    }
    pPdd->ep[endpoint].dmaDVFSstate = DVFS_NODMA;
    
    // Done
    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_InitEndpoint
//
//  Routine descriptions:
//
//      This function is called when pipe to endpoint is created. By default, control endpoint
//  (endpoint 0) would be created during the driver is loaded. Other endpoints would be created
//  during the connection.
//
//  Arguments:
//  
//      pPddContext ::  Handle of HSUSBFNPDD_T
//      endpoint    ::  endpoint to be initialize
//      speed       ::  connection speed
//      pEPDesc     ::  pointer to endpoint descriptor
//      pReserved   ::  not used.
//      configValue ::  not used.
//      interfaceNumber     ::  not used
//      alternateSetting    ::  not used.
//
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_InitEndpoint(
    VOID *pPddContext,
    DWORD endpoint,
    UFN_BUS_SPEED speed,
    USB_ENDPOINT_DESCRIPTOR *pEPDesc,
    VOID *pReserved,
    UCHAR configValue,
    UCHAR interfaceNumber,
    UCHAR alternateSetting
)
{
    MUsbFnPdd_t *pPdd = pPddContext;
    PCSP_MUSB_GEN_REGS pGenRegs = pPdd->pUSBContext->pUsbGenRegs;
    PCSP_MUSB_CSR_REGS pCsrRegs = pPdd->pUSBContext->pUsbCsrRegs;
    UCHAR epTransferType;
    
	UNREFERENCED_PARAMETER(alternateSetting);
	UNREFERENCED_PARAMETER(interfaceNumber);
	UNREFERENCED_PARAMETER(configValue);
	UNREFERENCED_PARAMETER(pReserved);

    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_InitEndpoint: endpoint %d  speed: %d  MaxPacketSize: %d\r\n",
        endpoint, speed, pEPDesc->wMaxPacketSize));
    
    pPdd->ep[endpoint].maxPacketSize = pEPDesc->wMaxPacketSize;
    pPdd->ep[endpoint].dirRx =(pEPDesc->bEndpointAddress & 0x80) ? FALSE : TRUE;
    pPdd->ep[endpoint].dmaDVFSstate = DVFS_NODMA;
    pPdd->ep[endpoint].bMassStore = FALSE;
    epTransferType = pEPDesc->bmAttributes & 0x3;
    
    
    if (pPdd->pfnEnUSBClock)
        pPdd->pfnEnUSBClock(TRUE);
    EnterCriticalSection(&pPdd->epCS);
    OUTREG16(&pGenRegs->Index, endpoint);  /* select endpoint to access rxFIFOxx registers */
        
    switch (epTransferType)
    {
        case EP_TYPE_INTERRUPT:
            DEBUGMSG(ZONE_INFO, (L"Interrupt endpoint\r\n"));
            if (pPdd->ep[endpoint].dirRx)
            {
                OUTREG8(&pGenRegs->RxFIFOsz, MUSB_FIFOSZ_512_BYTE);
                OUTREG16(&pGenRegs->RxFIFOadd, CalcFIFOAddr(endpoint,pPdd->ep[endpoint].dirRx));
                OUTREG16(&pCsrRegs->ep[endpoint].RxMaxP, pPdd->ep[endpoint].maxPacketSize);
                OUTREG16(&pCsrRegs->ep[endpoint].RxCSR, RXCSR_P_FLUSHFIFO | RXCSR_P_CLRDATATOG);
                
                DEBUGMSG(ZONE_INFO, (L"RxFIFOsz=%d\r\n", INREG8(&pGenRegs->RxFIFOsz)));
                DEBUGMSG(ZONE_INFO, (L"RxFIFOadd=%d\r\n", INREG16(&pGenRegs->RxFIFOadd)));
                
                // Doesn't support PING in high speed mode
                if (speed == BS_HIGH_SPEED)
                    SETREG16(&pCsrRegs->ep[endpoint].RxCSR, RXCSR_P_DISNYET);
            }
            else
            {
                OUTREG8(&pGenRegs->TxFIFOsz, MUSB_FIFOSZ_512_BYTE);
                OUTREG16(&pGenRegs->TxFIFOadd, CalcFIFOAddr(endpoint,pPdd->ep[endpoint].dirRx));
                OUTREG16(&pCsrRegs->ep[endpoint].TxMaxP, pPdd->ep[endpoint].maxPacketSize);
                OUTREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_FLUSHFIFO | TXCSR_P_CLRDATATOG | TXCSR_P_FRCDATATOG);
                
                DEBUGMSG(ZONE_INFO, (L"TxFIFOsz=%d\r\n", INREG8(&pGenRegs->TxFIFOsz)));
                DEBUGMSG(ZONE_INFO, (L"TxFIFOadd=%d\r\n", INREG16(&pGenRegs->TxFIFOadd)));
            }
            break;
            
        case EP_TYPE_BULK:
            DEBUGMSG(ZONE_INFO, (L"Bulk endpoint\r\n"));
  
            // Bulk endpoints the of Mass Storage devices should not send zero length transfers
            if (speed == BS_HIGH_SPEED)
            {
                if(pPdd->HsEpMassStoreFlags & (1<<endpoint))
                {
                    pPdd->ep[endpoint].bMassStore = TRUE;
                }
            }
            else
            {
                if(pPdd->FsEpMassStoreFlags & (1<<endpoint))
                {
                    pPdd->ep[endpoint].bMassStore = TRUE;
                }
            }

            if (pPdd->ep[endpoint].dirRx)
            {
                OUTREG8(&pGenRegs->RxFIFOsz, MUSB_FIFOSZ_512_BYTE);
                OUTREG16(&pGenRegs->RxFIFOadd, CalcFIFOAddr(endpoint,pPdd->ep[endpoint].dirRx));
                OUTREG16(&pCsrRegs->ep[endpoint].RxMaxP, pPdd->ep[endpoint].maxPacketSize);
                OUTREG16(&pCsrRegs->ep[endpoint].RxCSR, RXCSR_P_FLUSHFIFO | RXCSR_P_CLRDATATOG);
                pPdd->rx0DmaState = MODE_FIFO;

                pPdd->ep[endpoint].bLastRxUsedDMA = FALSE;
                if (pPdd->enableDMA && (pPdd->rx0DmaEp == -1))
                {
                    pPdd->rx0DmaEp = endpoint;
                    pPdd->rx0DmaState = MODE_DMA;
                    pPdd->ep[endpoint].dmaEnabled = TRUE;
                    DEBUGMSG(ZONE_INFO, (TEXT("RX EP %d DMA Enable\r\n"), endpoint));
                }
                
                DEBUGMSG(ZONE_INFO, (L"RxFIFOsz=%d\r\n", INREG8(&pGenRegs->RxFIFOsz)));
                DEBUGMSG(ZONE_INFO, (L"RxFIFOadd=%d\r\n", INREG16(&pGenRegs->RxFIFOadd)));
            }
            else
            {
                OUTREG8(&pGenRegs->TxFIFOsz, MUSB_FIFOSZ_512_BYTE);
                OUTREG16(&pGenRegs->TxFIFOadd, CalcFIFOAddr(endpoint,pPdd->ep[endpoint].dirRx));
                OUTREG16(&pCsrRegs->ep[endpoint].TxMaxP, pPdd->ep[endpoint].maxPacketSize);
                OUTREG16(&pCsrRegs->ep[endpoint].CSR.TxCSR, TXCSR_P_FLUSHFIFO | TXCSR_P_CLRDATATOG);
                
                if (pPdd->enableDMA && (pPdd->tx0DmaEp == -1))
                {
                    pPdd->tx0DmaEp = endpoint;
                    pPdd->ep[endpoint].dmaEnabled = TRUE;
                    DEBUGMSG(ZONE_INFO, (TEXT("TX EP %d DMA Enable\r\n"), endpoint));
                }
                

                DEBUGMSG(ZONE_INFO, (L"TxFIFOsz=%d\r\n", INREG8(&pGenRegs->TxFIFOsz)));
                DEBUGMSG(ZONE_INFO, (L"TxFIFOadd=%d\r\n", INREG16(&pGenRegs->TxFIFOadd)));
            }
            break;
            
        case EP_TYPE_CTRL:
            DEBUGMSG(ZONE_INFO, (L"Control endpoint\r\n"));
            if (endpoint == 0)
            {
                OUTREG8(&pGenRegs->RxFIFOsz, MUSB_FIFOSZ_128_BYTE);
                OUTREG16(&pGenRegs->RxFIFOadd, 0);
                
                OUTREG8(&pGenRegs->TxFIFOsz, MUSB_FIFOSZ_128_BYTE);
                OUTREG16(&pGenRegs->TxFIFOadd, 256/8);
                
                DEBUGMSG(ZONE_INFO, (L"RxFIFOsz=%d\r\n", INREG8(&pGenRegs->RxFIFOsz)));
                DEBUGMSG(ZONE_INFO, (L"RxFIFOadd=%d\r\n", INREG16(&pGenRegs->RxFIFOadd)));
                
                DEBUGMSG(ZONE_INFO, (L"TxFIFOsz=%d\r\n", INREG8(&pGenRegs->TxFIFOsz)));
                DEBUGMSG(ZONE_INFO, (L"TxFIFOadd=%d\r\n", INREG16(&pGenRegs->TxFIFOadd)));
            }
            
        case EP_TYPE_ISOCH:
        default:
            DEBUGMSG(ZONE_INFO, (L"Unsupported endpoint Transfer type\r\n"));
            break;
    }
    
    
    LeaveCriticalSection(&pPdd->epCS);
    
    pPdd->pfnEnUSBClock(FALSE);
    
    
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_SetAddress
//
//  Routine descriptions:
//
//      This function is called by MDD when configuration process assigned address
//  to device. In this case, set the FADDR register when EP0 state machine transitions to proper state.
//
//  Arguments:
//  
//      pPddContext ::  Handle of HSUSBFNPDD_T
//      address     ::  Device address to be set when receiving from host.
//
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_SetAddress(
    VOID *pPddContext,
    UCHAR address
)
{
    MUsbFnPdd_t *pPdd = pPddContext;
    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_SetAddress\r\n"));
    
    pPdd->bSetAddress = TRUE;
    pPdd->ucAddress = address;
    
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_Start
//
//  Routine descriptions:
//
//      This function is called after UfnPdd_RegisterDevice. It should attach
//  device to USB bus.
//
//  Arguments:
//  
//      pPddContext ::  Handle to HSUSBFNPDD_T
//
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_Start(
    VOID *pPddContext
)
{
    MUsbFnPdd_t *pPdd = pPddContext;

    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_Start\r\n"));

    BusChildIoControl(pPdd->hParentBus,IOCTL_USBOTG_REQUEST_START,NULL,0);    

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_RegisterDevice
//
//  Routine descriptions:
//
//      This function is called by MDD after device configuration was sucessfully
//  verified by UfnPdd_IsEndpointSupportable and
//  UfnPdd_IsConfigurationSupportable. It should initialize hardware for given
//  configuration. Depending on hardware endpoints can be initialized later in
//  UfnPdd_InitEndpoint. For OMAP7xx it isn't a case, so we should do all
//  initialization there.
//
//  Arguments:
//      pPddContext - Handle of HSUSBFNPDD_T
//      pHighSpeedDeviceDesc - Pointer to High Speed USB Device descriptor
//      pHighSpeedConfig - Pointer to USB Configuration
//      pFullSpeedDeviceDesc - Pointer to Full Speed USB Device descriptor
//      pFullSpeedConfig - Pointer to USB Configuration
//      pFullSpeedConfigDesc - Pointer to USB Configuration descriptor
//  
//  Return:
//    ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_RegisterDevice(
    VOID *pPddContext,
    const USB_DEVICE_DESCRIPTOR *pHighSpeedDeviceDesc,
    const UFN_CONFIGURATION *pHighSpeedConfig,
    const USB_CONFIGURATION_DESCRIPTOR *pHighSpeedConfigDesc,
    const USB_DEVICE_DESCRIPTOR *pFullSpeedDeviceDesc,
    const UFN_CONFIGURATION *pFullSpeedConfig,
    const USB_CONFIGURATION_DESCRIPTOR *pFullSpeedConfigDesc,
    const UFN_STRING_SET *pStringSets,
    DWORD stringSets
)
{
    // NOTE: For now use FS configuration.                                                                                  //
    //     Final endpoint configuration (maxPackeSize, dirRX) is done later in UfnPdd_InitEndpoint()  //
        
    MUsbFnPdd_t *pPdd = pPddContext;
    UFN_INTERFACE *pIFC;
    UFN_ENDPOINT *pEP;
    DWORD ep;
    DWORD ifc, epx;

	UNREFERENCED_PARAMETER(pStringSets);
	UNREFERENCED_PARAMETER(stringSets);

    DEBUGMSG(ZONE_PDD, (L"UsbFnPdd_RegisterDevice\r\n"));
    
    DEBUGMSG(ZONE_INFO, (L"HS configuration: \r\n"));
    prtDescriptorInfo(pHighSpeedDeviceDesc, pHighSpeedConfig, pHighSpeedConfigDesc);
    
    DEBUGMSG(ZONE_INFO, (L"FS configuration: \r\n"));
    prtDescriptorInfo(pFullSpeedDeviceDesc, pFullSpeedConfig, pFullSpeedConfigDesc);
        
    // Remember self powered flag
    pPdd->selfPowered =(pFullSpeedConfig->Descriptor.bmAttributes & 0x20) != 0;
    
    pPdd->ep[EP0].maxPacketSize = pFullSpeedDeviceDesc->bMaxPacketSize0;
    
    // Configure Rx EPs
    for (ifc = 0; ifc < pFullSpeedConfig->Descriptor.bNumInterfaces; ifc++)
    {
        // For each endpoint in interface
        pIFC = &pFullSpeedConfig->pInterfaces[ifc];
        for (epx = 0; epx < pIFC->Descriptor.bNumEndpoints; epx++)
        {
            pEP = &pIFC->pEndpoints[epx];
            // If it is Tx EP skip it
            if ((pEP->Descriptor.bEndpointAddress & 0x80) != 0)
                continue;
            // Get EP address
            ep = pEP->Descriptor.bEndpointAddress & 0x0F;
            // Save max packet size & direction
            pPdd->ep[ep].maxPacketSize = pEP->Descriptor.wMaxPacketSize;
            pPdd->ep[ep].dirRx = TRUE;
            
            if ((pEP->Descriptor.bmAttributes & 0x03) == 0x01)
            {
                DEBUGMSG(ZONE_INFO, (L"!!! UfnPdd_RegisterDevice:  USB FN does not support ISOCH\r\n"));
            }
        }
    }
    
    // Configure Tx EPs
    for (ifc = 0; ifc < pFullSpeedConfig->Descriptor.bNumInterfaces; ifc++)
    {
        // For each endpoint in interface
        pIFC = &pFullSpeedConfig->pInterfaces[ifc];
        for (epx = 0; epx < pIFC->Descriptor.bNumEndpoints; epx++)
        {
            pEP = &pIFC->pEndpoints[epx];
            // If it is Rx EP skip it
            if ((pEP->Descriptor.bEndpointAddress & 0x80) == 0)
                continue;
            // Get EP address
            ep = pEP->Descriptor.bEndpointAddress & 0x0F;
            // Save max packet size & direction
            pPdd->ep[ep].maxPacketSize = pEP->Descriptor.wMaxPacketSize;
            pPdd->ep[ep].dirRx = FALSE;
            
            if ((pEP->Descriptor.bmAttributes & 0x03) == 0x01)
            {
                DEBUGMSG(ZONE_INFO, (L"!!! UfnPdd_RegisterDevice:  USB FN does not support ISOCH\r\n"));
            }
            
        }
    }

    // Handle Special case for Mass Storage Devices
    pPdd->FsEpMassStoreFlags = 0;
    for (ifc = 0; ifc < pFullSpeedConfig->Descriptor.bNumInterfaces; ifc++)
    {
        // For each endpoint in interface
        pIFC = &pFullSpeedConfig->pInterfaces[ifc];

        // check for mass storage device
        if(pIFC->Descriptor.bInterfaceClass == 0x08)
        {
            // do not complete packets with 0 that end are divizable by max packet size
            // this will prevent the CSW packet from being sent and will force the host to reset us
            
            // set a bit flag for endpoints
            for (epx = 0; epx < pIFC->Descriptor.bNumEndpoints; epx++)
            {
                pEP = &pIFC->pEndpoints[epx];
                // Get EP address
                ep = pEP->Descriptor.bEndpointAddress & 0x0F;
                // Save Bit Flag
                pPdd->FsEpMassStoreFlags |= 1 << ep;
            }
        }
    }
    pPdd->HsEpMassStoreFlags = 0;
    for (ifc = 0; ifc < pFullSpeedConfig->Descriptor.bNumInterfaces; ifc++)
    {
        // For each endpoint in interface
        pIFC = &pHighSpeedConfig->pInterfaces[ifc];

        // check for mass storage device
        if(pIFC->Descriptor.bInterfaceClass == 0x08)
        {
            // do not complete packets with 0 that end are divizable by max packet size
            // this will prevent the CSW packet form being sent and will force the host to reset us

            // set a bit flag for endpoints
            for (epx = 0; epx < pIFC->Descriptor.bNumEndpoints; epx++)
            {
                pEP = &pIFC->pEndpoints[epx];
                // Get EP address
                ep = pEP->Descriptor.bEndpointAddress & 0x0F;
                // Save Bit Flag
                pPdd->HsEpMassStoreFlags |= 1 << ep;
            }
        }
    }

    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IsEndpointSupportable
//
//  Routine descriptions:
//
//      This function is called by MDD to verify if EP can be supported on
//  hardware. It is called after UfnPdd_IsConfigurationSupportable. We must
//  verify configuration in this function, so we already know that EPs
//  are valid. Only information we can update there is maximal packet
//  size for EP0.
//
//  Arguments:
//
//      pPddContext ::  Handle of HSUSBFNPDD_T
//      endpoint    ::  Endpoint to be queried
//      speed       ::  connection speed
//      pEPDesc     ::  pointer to endpoint descriptor
//      configurationValue  ::
//      interfaceNumber     ::
//      alternateSetting    ::
//
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_IsEndpointSupportable(
    VOID *pPddContext,
    DWORD endpoint,
    UFN_BUS_SPEED speed,
    USB_ENDPOINT_DESCRIPTOR *pEPDesc,
    UCHAR configurationValue,
    UCHAR interfaceNumber,
    UCHAR alternateSetting
)
{
	UNREFERENCED_PARAMETER(pPddContext);
	UNREFERENCED_PARAMETER(speed);
	UNREFERENCED_PARAMETER(configurationValue);
	UNREFERENCED_PARAMETER(interfaceNumber);
	UNREFERENCED_PARAMETER(alternateSetting);

    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_IsEndpointSupportable\r\n"));
    
    // Update maximal packet size for EP0
    if (endpoint == 0)
    {
        DEBUGCHK(pEPDesc->wMaxPacketSize <= 64);
        DEBUGCHK(pEPDesc->bmAttributes == USB_ENDPOINT_TYPE_CONTROL);
        pEPDesc->wMaxPacketSize = 64;
    }
    
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IsConfigurationSupportable
//
//  Routine Descriptions:
//
//      This function is called before UfnPdd_RegisterDevice. It should verify
//  that USB device configuration can be supported on hardware. Function can
//  modify EP size and/or EP address.
//
//  For INVENTRA MUSB we should check if total descriptor size is smaller
/// than 1024 bytes. Unfortunately we don't get information
//  about EP0 max packet size. So we will assume maximal 64 byte size.
//
//  Arguments:
//
//      pPddContext ::  Handle of HSUSBFNPDD_T
//      speed       ::  connection speed
//      pConfig     ::  device configuration
//
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_IsConfigurationSupportable(
    VOID *pPddContext,
    UFN_BUS_SPEED speed,
    UFN_CONFIGURATION *pConfig
)
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    UFN_INTERFACE *pIFC;
    UFN_ENDPOINT *pEP;
    WORD ifc, epx, count;
    WORD size;

	UNREFERENCED_PARAMETER(pPddContext);
	UNREFERENCED_PARAMETER(speed);

    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_IsConfigurationSupportable\r\n"));
    
    // Clear number of end points
    count = 0;
    
    // For each interface in configuration
    for (ifc = 0; ifc < pConfig->Descriptor.bNumInterfaces; ifc++)
    {
        // For each endpoint in interface
        pIFC = &pConfig->pInterfaces[ifc];
        for (epx = 0; epx < pIFC->Descriptor.bNumEndpoints; epx++)
        {
            pEP = &pIFC->pEndpoints[epx];
            
            if (pEP->Descriptor.wMaxPacketSize > MAX_EPX_PKTSIZE) 
            {
                size = MAX_EPX_PKTSIZE;
            }
            else
            {
                size = pEP->Descriptor.wMaxPacketSize;
            }
            
            
            // Is it ISO end point?
            if ((pEP->Descriptor.bmAttributes & 0x03) == 0x01)
            {
                // Actual driver doesn't support ISO endpoints
                goto cleanUp;
            }
            // Update EP size
            pEP->Descriptor.wMaxPacketSize = size;
        }
        // Add number of end points to total count
        count = count +pIFC->Descriptor.bNumEndpoints;
    }
    
    // Can we support this configuration?
    if (count < USBD_EP_COUNT)
        rc = ERROR_SUCCESS;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_Init
//
//  Routine description:
//
//      This function is called by MDD on driver load. PDD should be created at this point
//
//  Arguments:
//
//      szActiveKey ::  Registry key info
//      pMddContext ::  Pointer to MDD context
//      pMddIfc     ::  Pointer to MDD_INTERFACE_INFO
//      pPddIfc     ::  Pointer to PDD_INTERFACE_INFO  
//      
//  Return values:
//
//      ERROR_SUCCESS
//
DWORD WINAPI
UfnPdd_Init(
    LPCTSTR szActiveKey,
    VOID *pMddContext,
    UFN_MDD_INTERFACE_INFO *pMddIfc,
    UFN_PDD_INTERFACE_INFO *pPddIfc
)
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    HMODULE hOTGInstance;
    LPMUSB_ATTACH_PROC lphAttachProc;
    BOOL bRet = FALSE;
    PHSMUSB_T pOTG;
    MUsbFnPdd_t *pPdd;
    HKEY hkDevice;
    DWORD dwStatus;
    DWORD dwType, dwSize;
    TCHAR szChargeEventName[MAX_PATH+1];
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("UfnPdd_Init called\r\n")));
    // Allocate PDD object
    pPdd = LocalAlloc(LPTR, sizeof(MUsbFnPdd_t));
    if (pPdd == NULL)
        goto cleanUp;
    
    // Initialize critical section
    InitializeCriticalSection(&pPdd->dmaCS);
    InitializeCriticalSection(&pPdd->epCS);
    
    InitializeCriticalSection(&pPdd->powerStateCS);
    pPdd->hPowerDownEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pPdd->hPowerDownEvent == NULL)
    {
        DEBUGMSG(TRUE, (TEXT("MUSBFN:Failed to create hPowerDownEvent\r\n")));
        goto cleanUp;
    }

    pPdd->devState = 0;
    
    // Read device parameters
    if (GetDeviceRegistryParams(
        szActiveKey, pPdd, dimof(s_deviceRegParams), s_deviceRegParams
        ) != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: "
            L"Failed read registry parameters\r\n"
            ));
        goto cleanUp;
    }

    // Set PM to Default
    pPdd->pmPowerState = D4;
    pPdd->selfPowerState = D0;
    pPdd->actualPowerState = D4;

    InitializeCriticalSection(&pPdd->csDVFS);

    // initialize dvfs variables
    pPdd->bDVFSActive = FALSE;
    pPdd->nActiveDmaCount = 0;
    pPdd->hDVFSAckEvent = NULL;
    // bDVFSAck is required to avoid duplicate ack DVFSAck event.
    // this is especially when DVFS happening and it is in the process of copying
    pPdd->bDVFSAck = FALSE;
    pPdd->hDVFSActivityEvent = NULL;

    
    // Open parent bus
    pPdd->hParentBus = CreateBusAccessHandle(szActiveKey);
    if (pPdd->hParentBus == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: "
            L"Failed open parent bus driver\r\n"
            ));
        goto cleanUp;
    }
    
    // Set hardware to standby mode
    pPdd->selfPowerState = D2;
    
    
    // Allocate DMA Rx0 buffer
    pPdd->pDmaRx0Buffer = AllocPhysMem(
        pPdd->rx0DmaBufferSize, PAGE_READWRITE | PAGE_NOCACHE, 0, 0,
        &pPdd->paDmaRx0Buffer
        );
    if (pPdd->pDmaRx0Buffer == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: "
            L"Failed allocate DMA Rx0 buffer (size %d)\r\n",
            pPdd->rx0DmaBufferSize
            ));
        goto cleanUp;
    }
    
    // Allocate DMA Tx0 buffer
    pPdd->pDmaTx0Buffer = AllocPhysMem(
        pPdd->tx0DmaBufferSize, PAGE_READWRITE | PAGE_NOCACHE, 0, 0,
        &pPdd->paDmaTx0Buffer
        );
    if (pPdd->pDmaTx0Buffer == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: "
            L"Failed allocate DMA Tx0 buffer (size %d)\r\n",
            pPdd->tx0DmaBufferSize
            ));
        goto cleanUp;
    }
        
    pPdd->pCachedDmaTx0Buffer = VirtualAlloc(NULL, pPdd->tx0DmaBufferSize, MEM_RESERVE, PAGE_READWRITE);
    if (pPdd->pCachedDmaTx0Buffer == NULL)
    {
        RETAILMSG(1, (L"ERROR: UfnPdd_Init: "
            L"Failed allocate virtual address for Tx0 buffer (size %d)\r\n",
            pPdd->tx0DmaBufferSize
            ));
        goto cleanUp;
    }

    if (!VirtualCopy(pPdd->pCachedDmaTx0Buffer, (PVOID)(pPdd->paDmaTx0Buffer>>8), pPdd->tx0DmaBufferSize, PAGE_READWRITE | PAGE_PHYSICAL))
    {
        RETAILMSG(1, (L"ERROR: UfnPdd_Init: "
            L"Failed virtual copy address for Tx0 buffer (size %d)\r\n",
            pPdd->tx0DmaBufferSize
            ));
        goto cleanUp;
    }

    // Get the OTG module handle
    hOTGInstance = GetModuleHandle(OTG_DRIVER);
    if (hOTGInstance == NULL)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("Failure to load %s\r\n"), OTG_DRIVER));
        return ERROR_GEN_FAILURE;
    }
    
    lphAttachProc =(LPMUSB_ATTACH_PROC)GetProcAddress(hOTGInstance, TEXT("OTGAttach"));
    if (lphAttachProc == NULL)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("Failure to get OTGAttach\r\n")));
        return ERROR_GEN_FAILURE;
    }
    
    DEBUGMSG(ZONE_INFO, (TEXT("AttachProc with 4 parameters\r\n")));
    bRet =(*lphAttachProc)(&gc_MUsbFuncs, DEVICE_MODE, (LPLPVOID)&pOTG);
    if (bRet == FALSE)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("Error in performing the attach procedure\r\n")));
        return ERROR_GEN_FAILURE;
    }

    pPdd->pfnEnUSBClock = (LPMUSB_USBCLOCK_PROC)GetProcAddress(hOTGInstance, TEXT("OTGUSBClock"));
    if (pPdd->pfnEnUSBClock == NULL)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("Failure to get OTGUSBClock\r\n")));
        return ERROR_GEN_FAILURE;
    }

    dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR) REG_USBFN_DRV_PATH, 0, KEY_ALL_ACCESS, &hkDevice);
    if(dwStatus != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_INFO, (_T("UfnPdd_Init: OpenDeviceKey('%s') failed %u\r\n"), szActiveKey, dwStatus));
        goto cleanUp;
    }

    wcscpy(szChargeEventName,TEXT(""));
    dwType = REG_SZ;
    dwSize = sizeof(szChargeEventName);
    dwStatus = RegQueryValueEx(hkDevice, REG_VBUS_CHARGE_EVENT_NAME, NULL, &dwType, 
        (LPBYTE) szChargeEventName, &dwSize);
    if(dwStatus != ERROR_SUCCESS || dwType != REG_SZ) {
        DEBUGMSG(ZONE_INFO, (_T("UFNPDD_Init: RegQueryValueEx('%s', '%s') failed %u\r\n"),
            REG_USBFN_DRV_PATH, REG_VBUS_CHARGE_EVENT_NAME, dwStatus));
        RegCloseKey(hkDevice);
        goto cleanUp;
    }
    RegCloseKey(hkDevice);
    
    DEBUGMSG(ZONE_ERROR,(L"USBHF RegQueryValueEx "L"Using Vbus Charge Event '%s'", szChargeEventName) );

    if (!dwStatus) 
    {
         pPdd->hVbusChargeEvent = CreateEvent( NULL, TRUE, FALSE, szChargeEventName );
         if ( pPdd->hVbusChargeEvent == NULL )
         {
            DEBUGMSG(ZONE_ERROR,(L"UfnPdd_Init() "
                    L"Failed to open Vbus Charge Event '%s'",
                    szChargeEventName));

            pPdd->hVbusChargeEvent = INVALID_HANDLE_VALUE;
         }
         else
         {
            DEBUGMSG(ZONE_ERROR,(L"UfnPdd_Init() "
                        L"Using Vbus Charge Event '%s'", szChargeEventName) );
         }
        // if event already exists, then handle to it will be returned
    }
    else
    {
        DEBUGMSG(ZONE_ERROR,(L"UfnPdd_Init() "
                L"Vbus Charge Event not defined ('%s' not set)", 
                REG_VBUS_CHARGE_EVENT_NAME));
    }
    
    pPdd->pUSBContext = pOTG;
    pPdd->bDMAForRX = FALSE;
    pPdd->bRXIsUsingUsbDMA = FALSE;
    pPdd->bTXIsUsingUsbDMA = FALSE;

    // Set PDD interface
    pPddIfc->dwVersion = UFN_PDD_INTERFACE_VERSION;
#if 0  // WM7 specific
    pPddIfc->dwCapabilities = UFN_PDD_CAPS_SUPPORTS_FULL_SPEED | UFN_PDD_CAPS_SUPPORTS_HIGH_SPEED
                         | UFN_PDD_CAPS_SUPPORTS_MULTIPLE_CONFIGURATIONS
                         | UFN_PDD_CAPS_SUPPORTS_ALTERNATE_INTERFACES
                         | UFN_PDD_CAPS_REUSABLE_ENDPOINTS;
#else
    pPddIfc->dwCapabilities = UFN_PDD_CAPS_SUPPORTS_FULL_SPEED | UFN_PDD_CAPS_SUPPORTS_HIGH_SPEED;

#endif
    pPddIfc->dwEndpointCount = USBD_EP_COUNT;
    // need to uncoment
    pPddIfc->pvPddContext = pPdd;
    pPddIfc->pfnDeinit = UfnPdd_Deinit;
    pPddIfc->pfnIsConfigurationSupportable = UfnPdd_IsConfigurationSupportable;
    pPddIfc->pfnIsEndpointSupportable = UfnPdd_IsEndpointSupportable;
    pPddIfc->pfnInitEndpoint = UfnPdd_InitEndpoint;
    pPddIfc->pfnRegisterDevice = UfnPdd_RegisterDevice;
    pPddIfc->pfnDeregisterDevice = UfnPdd_DeregisterDevice;
    pPddIfc->pfnStart = UfnPdd_Start;
    pPddIfc->pfnStop = UfnPdd_Stop;
    pPddIfc->pfnIssueTransfer = UfnPdd_IssueTransfer;
    pPddIfc->pfnAbortTransfer = UfnPdd_AbortTransfer;
    pPddIfc->pfnDeinitEndpoint = UfnPdd_DeinitEndpoint;
    pPddIfc->pfnStallEndpoint = UfnPdd_StallEndpoint;
    pPddIfc->pfnClearEndpointStall = UfnPdd_ClearEndpointStall;
    pPddIfc->pfnSendControlStatusHandshake = UfnPdd_SendControlStatusHandshake;
    pPddIfc->pfnSetAddress = UfnPdd_SetAddress;
    pPddIfc->pfnIsEndpointHalted = UfnPdd_IsEndpointHalted;
    pPddIfc->pfnInitiateRemoteWakeup = UfnPdd_InitiateRemoteWakeup;
    pPddIfc->pfnPowerDown = UfnPdd_PowerDown;
    pPddIfc->pfnPowerUp = UfnPdd_PowerUp;
    pPddIfc->pfnIOControl = UfnPdd_IOControl;
    
    // Save MDD context & notify function
    pPdd->pMddContext = pMddContext;
    pPdd->pfnNotify = pMddIfc->pfnNotify;
    
    // pOTG->counter++;
    // Should set to pPdd object
    pOTG->pContext[DEVICE_CONTEXT] = pPdd;
    // DEBUGMSG(1, (TEXT("Counter in device module = %d\r\n"), pOTG->counter));
    SetEvent(pOTG->hReadyEvents[DEVICE_CONTEXT]);
    // Done
    rc = ERROR_SUCCESS;

cleanUp:
   
    return rc;
}


//------------------------------------------------------------------------------
//  Function : UfnPdd_DllEntry
//
//  Routine descriptions:
//
//      DLL entry point
//
//  Arguments:
//  
//      hDllHandle  ::  DLL handle
//      reason      ::  reason for this call
//      pReserved   ::  not used.
//
//  Return values:
//
//      TRUE
//
extern BOOL
UfnPdd_DllEntry(
    HANDLE hDllHandle,
    DWORD reason,
    VOID *pReserved
)
{
	UNREFERENCED_PARAMETER(hDllHandle);
	UNREFERENCED_PARAMETER(reason);
	UNREFERENCED_PARAMETER(pReserved);

    return TRUE;
}
