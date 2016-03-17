// All rights reserved ADENEO EMBEDDED 2010

#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#include <nkintr.h>

#include "am3517.h"
#include "oal_clock.h"
#include "sdk_padcfg.h"
#include "am3517_usb.h"
#include "am3517_usbcdma.h"
#pragma warning(pop)

typedef CSL_CppiRegs CppiRegs;

typedef struct _UsbModule
{
    UINT16            nHdCount;
    UINT8             cbHdSize;
    VOID             *pvPool;
    PHYSICAL_ADDRESS  paPool;
    ULONG             cbPoolSize;
    PVOID             pvLinkingRam;
    PHYSICAL_ADDRESS  paLinkingRam;
    ULONG             cbLinkingRamSize;
    VOID (* volatile  callback)(VOID *);
    VOID             *param;
} UsbModule;

typedef struct _DeviceContext
{
    CRITICAL_SECTION csLock;
    DWORD            dwIrqVal;
    DWORD            dwSysIntr;
    HANDLE           hIntrEvent;
    HANDLE           hIntrThread;
    int              nIntrThreadPriority;
    BOOL             fIntrThreadClosing;
    CppiRegs        *pCppiRegs;
    VOID            *pvTdPool;
    PHYSICAL_ADDRESS paTdPool;
    ULONG            cbTdPoolSize;
    BOOL             fTdPoolInitialised;
    UINT             nNextUsbModule;
    UsbModule        usb[2];
} DeviceContext;


#ifdef DEBUG

#undef ZONE_ERROR
#undef ZONE_INIT

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_INFO           DEBUGZONE(2)
#define ZONE_VERBOSE        DEBUGZONE(3)
#define ZONE_FUNCTION       DEBUGZONE(4)
#define ZONE_INIT           DEBUGZONE(5)
#define ZONE_IST            DEBUGZONE(6)
#define ZONE_IOCTL          DEBUGZONE(7)

DBGPARAM dpCurSettings = {
    L"USBCDMA Driver", {
        L"Errors",      L"Warnings",    L"Info",        L"Verbose",
        L"Function",    L"Init",        L"IST",         L"IOCTL",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x00b7 |  0xFFFF
};

#endif


static DWORD WINAPI IntrThread      (LPVOID lpParameter);
static void         Lock            (void);
static void         Unlock          (void);
static BOOL         PoolInit        (void);
static void         PoolDeinit      (void);
static BOOL         TdPoolInit      (void);
static void         TdPoolDeinit    (void);
static BOOL         TdFreeQueueInit (void);
static void         QueuePush       (BYTE nQueue, void *pDescriptor);
static BOOL         IsPow2          (unsigned n);
static unsigned     Log2            (unsigned n);
static VOID         ConfigureScheduler(VOID);

static BOOL g_scheduleRx[USB_CPPI_MAX_RX_CHANNELS] = {FALSE};
static BOOL g_scheduleTx[USB_CPPI_MAX_TX_CHANNELS] = {FALSE};

static DWORD         g_hDc = 0;
static DeviceContext g_Dc =
{
    /* csLock              */ { 0 },
    /* dwIrqVal            */ (DWORD)-1, // INTC_IRQ_USB_CDMA_INT,
    /* dwSysIntr           */ (DWORD)SYSINTR_UNDEFINED,
    /* hIntrEvent          */ NULL,
    /* hIntrThread         */ NULL,
    /* nIntrThreadPriority */ 101,
    /* fIntrThreadClosing  */ FALSE,
    /* pCppiRegs           */ NULL,
    /* pvTdPool            */ NULL,
    /* paTdPool            */ { 0 },
    /* cbTdPoolSize        */ USB_CPPI_TD_POOL_SIZE,
    /* fTdPoolInitialised  */ FALSE,
    /* nNextUsbModule      */ 0,
    /* usb[]               */
    {
        {   /* usb[0]          */
            /* nHdCount        */ 0,
            /* cbHdSize        */ 0,
            /* pvPool          */ NULL,
            /* paPool          */ { 0 },
            /* cbPoolSize      */ 0,
            /* pvLinkingRam    */ NULL,
            /* paLinkingRam    */ { 0 },
            /* cbLinkingRamSize */ 0,
            /* callback         */ NULL,
            /* param            */ NULL
        },
        {   /* usb[1]          */
            /* nHdCount        */ 0,
            /* cbHdSize        */ 0,
            /* pvPool          */ NULL,
            /* paPool          */ { 0 },
            /* cbPoolSize      */ 0,
            /* pvLinkingRam    */ NULL,
            /* paLinkingRam    */ { 0 },
            /* cbLinkingRamSize */ 0,
            /* callback         */ NULL,
            /* param            */ NULL
        }
    }
};


BOOL WINAPI DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
	UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DEBUGMSG(ZONE_INIT, (_T("USBCDMA: DLL_PROCESS_ATTACH\r\n")));
            DEBUGREGISTER(hInstDll);
            DisableThreadLibraryCalls((HMODULE)hInstDll);
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INIT, (_T("USBCDMA: DLL_PROCESS_DETACH\r\n")));
            break;
    }
    return TRUE;
}


DWORD UCD_Init (
	LPCTSTR pContext,
	DWORD dwBusContext);

BOOL UCD_Deinit(
	DWORD hDeviceContext);

BOOL UCD_Close(
    DWORD hOpenContext)
{ 
	UNREFERENCED_PARAMETER(hOpenContext);

	return TRUE;
}

DWORD UCD_Read(
	DWORD hOpenContext, 
	LPVOID pBuffer, 
	DWORD Count)
{ 
	UNREFERENCED_PARAMETER(hOpenContext);
	UNREFERENCED_PARAMETER(pBuffer);
	UNREFERENCED_PARAMETER(Count);

	return 0;
}

DWORD UCD_Write(
	DWORD hOpenContext, 
	LPCVOID pBuffer, 
	DWORD Count)
{ 
	UNREFERENCED_PARAMETER(hOpenContext);
	UNREFERENCED_PARAMETER(pBuffer);
	UNREFERENCED_PARAMETER(Count);

	return 0;
}

DWORD UCD_Seek(
    DWORD hOpenContext, 
	LONG Amount, 
	WORD Type)
{ 
	UNREFERENCED_PARAMETER(hOpenContext);
	UNREFERENCED_PARAMETER(Amount);
	UNREFERENCED_PARAMETER(Type);

	return 0; 
}

DWORD UCD_Open(
	DWORD hDeviceContext, 
	DWORD AccessCode, 
	DWORD ShareMode) 
{ 
	UNREFERENCED_PARAMETER(AccessCode);
	UNREFERENCED_PARAMETER(ShareMode);

	return hDeviceContext; 
}

BOOL UCD_IOControl(
    DWORD hOpenContext,
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut)
{
	UNREFERENCED_PARAMETER(hOpenContext);
	UNREFERENCED_PARAMETER(dwCode);
	UNREFERENCED_PARAMETER(pBufIn);
	UNREFERENCED_PARAMETER(dwLenIn);
	UNREFERENCED_PARAMETER(dwLenOut);
	UNREFERENCED_PARAMETER(pBufOut);

    if (pdwActualOut != NULL)
        pdwActualOut = 0;

    return TRUE;
}

HANDLE USBCDMA_RegisterUsbModule(
    UINT16 nHdCount,
    UINT8 cbHdSize,
    PHYSICAL_ADDRESS *ppaHdPool,
    VOID **ppvHdPool,
    VOID (*callback)(VOID *),
    VOID *param
    )
{
    HANDLE hUsbModule = NULL;
    UsbModule *pUsbModule = NULL;
    int i;

    DEBUGMSG(ZONE_INIT || ZONE_VERBOSE,
    (L"+USBCDMA_RegisterUsbModule: %u HDs, %u bytes, 0x%08x, 0x%08x, 0x%08x, 0x%08x\r\n",
        nHdCount,
        cbHdSize,
        ppaHdPool,
        ppvHdPool,
        callback,
        param));

    // Check driver state
    if (g_hDc == 0) 
	{
        ERRORMSG(1,
            (L" USBCDMA_RegisterUsbModule: ERROR - Driver not initialised\r\n"));
        goto done_unlocked;
    }

    Lock();

    // Check input parameters

    if (!IsPow2(nHdCount) || (nHdCount < 32) || (nHdCount > 4096)) 
	{
        ERRORMSG(1,
            (L" USBCDMA_RegisterUsbModule: ERROR - Invalid nHdCount %u\r\n",
            nHdCount));
        goto done;
    }

    if (!IsPow2(cbHdSize) || (cbHdSize < 32)) 
	{
        ERRORMSG(1,
            (L" USBCDMA_RegisterUsbModule: ERROR - Invalid cbHdSize %u\r\n",
            cbHdSize));
        goto done;
    }

    if (ppaHdPool == NULL) 
	{
        ERRORMSG(1,
            (L" USBCDMA_RegisterUsbModule: ERROR - Invalid ppaHdPool 0x%08x\r\n",
            ppaHdPool));
        goto done;
    }

    if (ppvHdPool == NULL) 
	{
        ERRORMSG(1,
            (L" USBCDMA_RegisterUsbModule: ERROR - Invalid ppvHdPool 0x%08x\r\n",
            ppvHdPool));
        goto done;
    }

    if (callback == NULL) 
	{
        ERRORMSG(1,
            (L" USBCDMA_RegisterUsbModule: ERROR - Invalid callback 0x%08x\r\n",
            callback));
        goto done;
    }

    // Initialise [OUT] parameters
    (*ppaHdPool).QuadPart = 0;
    (*ppvHdPool) = NULL;

    // Check driver state
    if (g_Dc.nNextUsbModule >= 2) 
	{
        ERRORMSG(1,
            (L" USBCDMA_RegisterUsbModule: ERROR - %u modules already registered\r\n",
            g_Dc.nNextUsbModule));
        goto done;
    }

    pUsbModule = &g_Dc.usb[g_Dc.nNextUsbModule];

    // Check the USB module state is uninitialised
    DEBUGCHK(pUsbModule->nHdCount == 0);
    DEBUGCHK(pUsbModule->cbHdSize == 0);
    DEBUGCHK(pUsbModule->pvPool == NULL);
    DEBUGCHK(pUsbModule->paPool.QuadPart == 0);
    DEBUGCHK(pUsbModule->cbPoolSize == 0);
    DEBUGCHK(pUsbModule->pvLinkingRam == NULL);
    DEBUGCHK(pUsbModule->paLinkingRam.QuadPart == 0);
    DEBUGCHK(pUsbModule->cbLinkingRamSize == 0);
    //DEBUGCHK(pUsbModule->callback == NULL);
    //DEBUGCHK(pUsbModule->param == NULL);

    // Allocate the USB module's host descriptor pool
    {
		DMA_ADAPTER_OBJECT Adapter;
		VOID *pvPool;
		PHYSICAL_ADDRESS paPool;
		ULONG cbPoolSize = nHdCount * cbHdSize;

		// The first module to register allocates additional space at the end of its host descriptor
		// pool for the shared teardown descriptor pool
		if (g_Dc.nNextUsbModule == 0)
		{
			cbPoolSize += g_Dc.cbTdPoolSize;
		}

		Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
		Adapter.InterfaceType = Internal;
		Adapter.BusNumber = 0;

		pvPool = HalAllocateCommonBuffer(
			&Adapter,
			cbPoolSize,
			&paPool,
			FALSE);

		if (pvPool == NULL) 
		{
			ERRORMSG(1,
				(L" USBCDMA_RegisterUsbModule: ERROR - Failed to allocate %u bytes for the descriptor pool\r\n", 
				cbPoolSize));
			goto done;
		}

		// Set the USB module's pool parameters
		pUsbModule->nHdCount   = nHdCount;
		pUsbModule->cbHdSize   = cbHdSize;
		pUsbModule->pvPool     = pvPool;
		pUsbModule->paPool     = paPool;
		pUsbModule->cbPoolSize = cbPoolSize;
    }

    // Save information about the shared teardown descriptor memory area
    if (g_Dc.nNextUsbModule == 0) {
        g_Dc.pvTdPool = (VOID *)((UINT)pUsbModule->pvPool + (nHdCount * cbHdSize));
        g_Dc.paTdPool.LowPart = pUsbModule->paPool.LowPart + (nHdCount * cbHdSize);
    }

    // Initialise the shared teardown descriptor pool when the first module registers
    if (g_Dc.nNextUsbModule == 0)
	{
        if (!TdPoolInit())
		{
            goto done;
		}
	}
    // Allocate the USB module's linking RAM
    {
    DMA_ADAPTER_OBJECT Adapter;
    VOID *pvLinkingRam;
    PHYSICAL_ADDRESS paLinkingRam;
    ULONG cbLinkingRamSize = nHdCount * sizeof(UINT32);

    // The first module to register allocates additional linking RAM for the teardown descriptors
    if (g_Dc.nNextUsbModule == 0)
	{
        cbLinkingRamSize += (USB_CPPI_TD_COUNT * sizeof(UINT32));
	}

    Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
    Adapter.InterfaceType = Internal;
    Adapter.BusNumber = 0;

    pvLinkingRam = HalAllocateCommonBuffer(
        &Adapter,
        cbLinkingRamSize,
        &paLinkingRam,
        FALSE);

    if (pvLinkingRam == NULL) 
	{
        ERRORMSG(1,
            (L" USBCDMA_RegisterUsbModule: ERROR - Failed to allocate %u bytes for the linking RAM\r\n", 
            cbLinkingRamSize));
        goto done;
    }

    // Set the USB module's linking RAM parameters
    pUsbModule->pvLinkingRam     = pvLinkingRam;
    pUsbModule->paLinkingRam     = paLinkingRam;
    pUsbModule->cbLinkingRamSize = cbLinkingRamSize;
    }

    // Set the USB module's callback parameters
    pUsbModule->callback = callback;
    pUsbModule->param    = param;

    // Setup the USB CPPI linking RAM registers
    if (g_Dc.nNextUsbModule == 0) {
        g_Dc.pCppiRegs->LRAM0BASE = pUsbModule->paLinkingRam.LowPart;
        g_Dc.pCppiRegs->LRAM0SIZE = pUsbModule->cbLinkingRamSize;
    }
    else {
        g_Dc.pCppiRegs->LRAM1BASE = pUsbModule->paLinkingRam.LowPart;
    }

    // Setup the USB CPPI memory region registers
    if (g_Dc.nNextUsbModule == 0) {
        g_Dc.pCppiRegs->QMMEMREGION[0].QMEMRBASE = pUsbModule->paPool.LowPart;
        g_Dc.pCppiRegs->QMMEMREGION[0].QMEMRCTRL =
            ((0                         ) << 16) | /* Start index */
            ((Log2(cbHdSize)         - 5) <<  8) | /* Desc. size  */
            ((Log2(nHdCount   )      - 5) <<  0);  /* Desc. count */

        g_Dc.pCppiRegs->QMMEMREGION[1].QMEMRBASE = g_Dc.paTdPool.LowPart;
        g_Dc.pCppiRegs->QMMEMREGION[1].QMEMRCTRL =
            ((nHdCount                  ) << 16) | /* Start index */
            ((USB_CPPI_TD_SIZE_POW2  - 5) <<  8) | /* Desc. size  */
            ((USB_CPPI_TD_COUNT_POW2 - 5) <<  0);  /* Desc. count */
    }
    else {
        UINT16 index = g_Dc.usb[0].nHdCount + USB_CPPI_TD_COUNT;
        g_Dc.pCppiRegs->QMMEMREGION[2].QMEMRBASE = pUsbModule->paPool.LowPart;
        g_Dc.pCppiRegs->QMMEMREGION[2].QMEMRCTRL =
            ((index                     ) << 16) | /* Start index */
            ((Log2(cbHdSize)         - 5) <<  8) | /* Desc. size  */
            ((Log2(nHdCount)         - 5) <<  0);  /* Desc. count */
    }

    if (!TdFreeQueueInit())
	{
        goto done;
	}

    // Finalise [OUT] parameters
    (*ppaHdPool) = pUsbModule->paPool;
    (*ppvHdPool) = pUsbModule->pvPool;

    g_Dc.nNextUsbModule++;
    hUsbModule = (HANDLE)pUsbModule;

    // Start CDMA scheduler based on initial values
    // (Tx enabled, Rx disabled)
    for(i=0;i<USB_CPPI_MAX_TX_CHANNELS;i++)
        g_scheduleTx[i] = TRUE;
    for(i=0;i<USB_CPPI_MAX_RX_CHANNELS;i++)
        g_scheduleRx[i] = FALSE;
    ConfigureScheduler();

done:
    Unlock();

done_unlocked:
    if (hUsbModule == NULL)
	{
        USBCDMA_DeregisterUsbModule((HANDLE)pUsbModule);
	}

    DEBUGMSG(ZONE_INIT || ZONE_VERBOSE,
        (L"-USBCDMA_RegisterUsbModule: %s\r\n",
        (hUsbModule != NULL) ?
            L"SUCCEEDED" :
            L"FAILED"));

    return hUsbModule;
}

BOOL USBCDMA_DeregisterUsbModule(
    HANDLE hUsbModule
    )
{
    BOOL fRC = FALSE;
    UsbModule *pUsbModule;
    int i;

    DEBUGMSG(ZONE_INIT || ZONE_VERBOSE,
        (L"+USBCDMA_DeregisterUsbModule: 0x%08x\r\n",
        hUsbModule));

    // Check driver state
    if (g_hDc == 0) 
	{
        ERRORMSG(1, (L" USBCDMA_DeregisterUsbModule: ERROR - Driver not initialised\r\n"));
        goto done_unlocked;
    }

    Lock();

    // Stop CDMA scheduler
    for(i=0;i<USB_CPPI_MAX_TX_CHANNELS;i++)
        g_scheduleTx[i] = FALSE;
    for(i=0;i<USB_CPPI_MAX_RX_CHANNELS;i++)
        g_scheduleRx[i] = FALSE;
    ConfigureScheduler();

    // Check input parameters
    if ((hUsbModule != (HANDLE)&g_Dc.usb[0]) && (hUsbModule != (HANDLE)&g_Dc.usb[1])) 
	{
        ERRORMSG(1,
            (L" USBCDMA_DeregisterUsbModule: ERROR - Invalid handle 0x%08x\r\n",
            hUsbModule));
        goto done;
    }

    pUsbModule = (UsbModule *)hUsbModule;

    // Check module state
    if (pUsbModule->callback == NULL) 
	{
        ERRORMSG(1,
            (L" USBCDMA_DeregisterUsbModule: ERROR - USB module not registered 0x%08x\r\n",
            pUsbModule));
        goto done;
    }

    // Deinitialise the shared teardown descriptor pool when the last module deregisters
    if (pUsbModule == &g_Dc.usb[0])
	{
        TdPoolDeinit();
	}

    // Free the USB module's resources
    {
		DMA_ADAPTER_OBJECT Adapter;
		Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
		Adapter.InterfaceType = Internal;
		Adapter.BusNumber = 0;

		// Free the USB module's linking RAM
		HalFreeCommonBuffer(
			&Adapter,
			pUsbModule->cbLinkingRamSize,
			pUsbModule->paLinkingRam,
			pUsbModule->pvLinkingRam,
			FALSE);

		// Reset the USB module's linking RAM parameters
		pUsbModule->cbLinkingRamSize      = 0;
		pUsbModule->paLinkingRam.QuadPart = 0;
		pUsbModule->pvLinkingRam          = NULL;

		// Free the USB module's descriptor pool
		HalFreeCommonBuffer(
			&Adapter,
			pUsbModule->cbPoolSize,
			pUsbModule->paPool,
			pUsbModule->pvPool,
			FALSE);

		// Clear the USB module's pool parameters
		pUsbModule->cbPoolSize      = 0;
		pUsbModule->paPool.QuadPart = 0;
		pUsbModule->pvPool          = NULL;
		pUsbModule->cbHdSize        = 0;
		pUsbModule->nHdCount        = 0;
    }

    g_Dc.nNextUsbModule--;

    fRC = TRUE;

done:
    Unlock();

done_unlocked:
    DEBUGMSG(ZONE_INIT || ZONE_VERBOSE,
        (L"-USBCDMA_DeregisterUsbModule: %s\r\n",
        fRC ?
            L"SUCCEEDED" :
            L"FAILED"));

    return fRC;
}

VOID USBCDMA_KickCompletionCallback(
    HANDLE hUsbModule
    )
{
    UsbModule *pUsbModule = NULL;

    DEBUGMSG(ZONE_IST,
        (L"+USBCDMA_KickCompletionCallback: 0x%08x\r\n",
        hUsbModule));

    // Check driver state
    if (g_hDc == 0) 
	{
        ERRORMSG(1,
            (L" USBCDMA_KickCompletionCallback: ERROR - Driver not initialised\r\n"));
        goto done;
    }

    // This function is thread-safe so we don't bother locking here

    // Check input parameters
    if ((hUsbModule != (HANDLE)&g_Dc.usb[0]) && (hUsbModule != (HANDLE)&g_Dc.usb[1])) 
	{
        ERRORMSG(1,
            (L" USBCDMA_KickCompletionCallback: ERROR - Invalid handle 0x%08x\r\n",
            hUsbModule));
        goto done;
    }

    pUsbModule = (UsbModule *)hUsbModule;

    // Check module state
    if (pUsbModule->callback == NULL) 
	{
        ERRORMSG(1,
            (L" USBCDMA_KickCompletionCallback: ERROR - USB module not registered 0x%08x\r\n",
            pUsbModule));
        goto done;
    }

    SetEvent(g_Dc.hIntrEvent);

done:
    DEBUGMSG(ZONE_IST,
        (L"-USBCDMA_KickCompletionCallback\r\n"));
}

UINT32 USBCDMA_DescriptorVAtoPA(
    HANDLE hUsbModule,
    void *va)
{
    UINT32 pa = 0;

    // Check driver state
    if (g_hDc == 0) 
	{
        ERRORMSG(1,
            (L" USBCDMA_DescriptorVAtoPA: ERROR - Driver not initialised\r\n"));
        goto done;
    }

    // This function is thread-safe so we don't bother locking here

    // Check input parameters
    if ((hUsbModule != (HANDLE)&g_Dc.usb[0]) && (hUsbModule != (HANDLE)&g_Dc.usb[1])) 
	{
        ERRORMSG(1,
            (L" USBCDMA_DescriptorVAtoPA: ERROR - Invalid handle 0x%08x\r\n",
            hUsbModule));
        goto done;
    }

    if (va != NULL) 
	{
        UINT32 vaPoolBase  = (UINT32)g_Dc.pvTdPool;
        UINT32 vaPoolLimit = (UINT32)g_Dc.pvTdPool + g_Dc.cbTdPoolSize;

        if (((UINT32)va < vaPoolBase) || ((UINT32)va > vaPoolLimit))
		{
            ERRORMSG(1,
                (L" USBCDMA_DescriptorVAtoPA: Virtual address is outside TD pool - 0x%08x\r\n",
                va));
		}
        else
		{
            pa = g_Dc.paTdPool.LowPart + ((UINT32)va - vaPoolBase);
		}
    }

done:
    return pa;
}


VOID USBCDMA_ConfigureScheduleRx( /* Returns nothing */
    UINT32  chanNum,              /* channel to configure */
    BOOL enable                   /* enable endpoint in scheduler */
    )
{
    // Check input parameter
    if(chanNum >= USB_CPPI_MAX_RX_CHANNELS)
    {
        // invalid parameter
        return;
    }

    // Check current state - only need to reconfigure if
    // required state is different
    if(g_scheduleRx[chanNum] != enable)
    {
        // update scheduler list
        g_scheduleRx[chanNum] = enable;
        // reconfigure scheduler
        ConfigureScheduler();
    }
}

VOID USBCDMA_ConfigureScheduleTx( /* Returns nothing */
    UINT32  chanNum,              /* channel to configure */
    BOOL enable                   /* enable endpoint in scheduler */
    )
{
    // Check input parameter
    if(chanNum >= USB_CPPI_MAX_TX_CHANNELS)
    {
        // invalid parameter
        return;
    }

    // Check current state - only need to reconfigure if
    // required state is different
    if(g_scheduleTx[chanNum] != enable)
    {
        // update scheduler list
        g_scheduleTx[chanNum] = enable;
        // reconfigure scheduler
        ConfigureScheduler();
    }
}

#define SCHEDULE_TABLE_ENTRIES      (USB_CPPI_MAX_TX_CHANNELS + USB_CPPI_MAX_RX_CHANNELS + 3)/4
VOID ConfigureScheduler( /* Returns nothing */
    VOID                         /* takes no parameters */
    )
{
    UINT32 max_entry = max(USB_CPPI_MAX_TX_CHANNELS,
                           USB_CPPI_MAX_RX_CHANNELS);
    UINT32 entryCount = 0;
    UINT32 scheduleTable[SCHEDULE_TABLE_ENTRIES] = {0};
    UINT32 scheduleBits;
    UINT32 i;

    // disable scheduler while we update
    g_Dc.pCppiRegs->DMA_SCHED_CTRL = 0;

    // update schedule register values based on 
    // g_scheduleTx and g_scheduleRx
    for(i=0; i<max_entry; i++)
    {
        // add Tx to the schedule, if enabled
        if((i < USB_CPPI_MAX_TX_CHANNELS)&&(g_scheduleTx[i] == TRUE))
        {
            scheduleBits = (i << ((entryCount * 8) % 32));
            scheduleTable[(entryCount/4)] |= scheduleBits;
            entryCount++;
        }
        // add Rx to the schedule, if enabled
        if((i < USB_CPPI_MAX_RX_CHANNELS)&&(g_scheduleRx[i] == TRUE))
        {
            scheduleBits = ((0x80 + i) << ((entryCount * 8) % 32));
            scheduleTable[(entryCount/4)] |= scheduleBits;
            entryCount++;
        }
    }

    // update schedule
    for(i=0;i<SCHEDULE_TABLE_ENTRIES;i++)
    {
        DEBUGMSG(ZONE_VERBOSE,(L"ConfigureScheduler: scheduleTable[%d]=0x%x\r\n", i, scheduleTable[i]));
        g_Dc.pCppiRegs->CDMASTWORD[i] = scheduleTable[i];
    }

    // update schedule control register
    g_Dc.pCppiRegs->DMA_SCHED_CTRL = 0x80000000 | (entryCount - 1);
    DEBUGMSG(ZONE_VERBOSE,(L"ConfigureScheduler: DMA_SCHED_CTRL = 0x%x\r\n", g_Dc.pCppiRegs->DMA_SCHED_CTRL));
}

void * USBCDMA_DescriptorPAtoVA(
    HANDLE hUsbModule,
    UINT32 pa)
{
    void *va = NULL;

    // Check driver state
    if (g_hDc == 0) 
	{
        ERRORMSG(1,
            (L" USBCDMA_DescriptorPAtoVA: ERROR - Driver not initialised\r\n"));
        goto done;
    }

    // This function is thread-safe so we don't bother locking here

    // Check input parameters
    if ((hUsbModule != (HANDLE)&g_Dc.usb[0]) && (hUsbModule != (HANDLE)&g_Dc.usb[1])) 
	{
        ERRORMSG(1,
            (L" USBCDMA_DescriptorPAtoVA: ERROR - Invalid handle 0x%08x\r\n",
            hUsbModule));
        goto done;
    }

    if (pa != 0) 
	{
        UINT32 paPoolBase  = g_Dc.paTdPool.LowPart;
        UINT32 paPoolLimit = g_Dc.paTdPool.LowPart + g_Dc.cbTdPoolSize;

        if ((pa < paPoolBase) || (pa > paPoolLimit))
		{
            ERRORMSG(1,
                (L" USBCDMA_DescriptorPAtoVA: Physical address is outside TD pool - 0x%08x\r\n",
                pa));
		}
        else
		{
            va = (void *)(((UINT32)g_Dc.pvTdPool) + (pa - paPoolBase));
		}
    }

done:
    return va;
}

DWORD UCD_Init(LPCTSTR pContext, DWORD dwBusContext)
{
    PHYSICAL_ADDRESS pa;

	UNREFERENCED_PARAMETER(pContext);
	UNREFERENCED_PARAMETER(dwBusContext);

    DEBUGMSG(ZONE_INIT || ZONE_VERBOSE,
        (L"+UCD_Init\r\n"));

    if (g_hDc != 0)
	{
        goto done_unlocked;
	}

    InitializeCriticalSection(&g_Dc.csLock);

    Lock();

    pa.LowPart = AM3517_CPPI_REGS_PA;
    g_Dc.pCppiRegs = (CppiRegs *)MmMapIoSpace(pa, sizeof(CppiRegs), FALSE);
    if (g_Dc.pCppiRegs == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (L" UCD_Init: ERROR - Failed to map CPPI register space\r\n"));
        goto done;
    }

    DEBUGCHK(g_Dc.hIntrEvent == NULL);
    g_Dc.hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_Dc.hIntrEvent == NULL) 
	{
        DEBUGMSG(ZONE_ERROR,
            (L" UCD_Init: ERROR - CreateEvent failed\r\n"));
        goto done;
    }

    // AM35x : The CDMA controller does not have a dedicated interrupt.
    // Therefore, the host and function drivers check the pending
    // queue register(s) on a USB controller interrupt and process
    // the completion queues if they indicate a DMA transfer has completed.
    // However, the event and thread in this driver is still required to 
    // process teardown completion.
#if 0
	{
    DWORD BytesRet = 0;

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &g_Dc.dwIrqVal, sizeof(DWORD),
            &g_Dc.dwSysIntr, sizeof(DWORD), &BytesRet))
    {
        DEBUGMSG(ZONE_ERROR,
            (L" UCD_Init: ERROR - Failed to request SYSINTR for IRQ%u\r\n",
            g_Dc.dwIrqVal));
        goto done;
    }

    InterruptDisable(g_Dc.dwSysIntr);
    if (!InterruptInitialize(g_Dc.dwSysIntr, g_Dc.hIntrEvent, NULL, 0)) {
        DEBUGMSG(ZONE_ERROR,
            (L" UCD_Init - ERROR - InterruptInitialize failed\r\n"));
        goto done;
    }
	}
#endif

    DEBUGCHK(g_Dc.hIntrThread == NULL);
    DEBUGCHK(g_Dc.fIntrThreadClosing == FALSE);
    g_Dc.hIntrThread = CreateThread(NULL, 0, IntrThread, NULL, 0, NULL);
    if (g_Dc.hIntrThread == NULL) 
	{
        DEBUGMSG(ZONE_ERROR,
            (L" UCD_Init: ERROR - CreateThread failed\r\n"));
        goto done;
    }
    CeSetThreadPriority(g_Dc.hIntrThread, g_Dc.nIntrThreadPriority);

    g_hDc = (DWORD)&g_Dc;

done:
    Unlock();

done_unlocked:
    if (g_hDc == 0)
	{
        UCD_Deinit((DWORD)&g_Dc);
	}

    DEBUGMSG(ZONE_INIT || ZONE_VERBOSE,
        (L"-UCD_Init: %s\r\n",
        g_hDc ?
            L"SUCCEEDED" :
            L"FAILED"));

    return g_hDc;
}

BOOL UCD_Deinit(DWORD hDeviceContext)
{
    DEBUGMSG(ZONE_INIT || ZONE_VERBOSE,
        (L"+UCD_Deinit\r\n"));

    if (hDeviceContext != (DWORD)&g_Dc) {
        DEBUGMSG(ZONE_ERROR,
            (L" UCD_Deinit: ERROR - Invalid handle 0x%08x\r\n",
            hDeviceContext));
        goto done;
    }

    Lock();

    // Ensure that any registered modules are deregistered
    while (g_Dc.nNextUsbModule)
        USBCDMA_DeregisterUsbModule((HANDLE)&g_Dc.usb[--g_Dc.nNextUsbModule]);

    g_Dc.fIntrThreadClosing = TRUE;

    if (g_Dc.hIntrEvent) {
        SetEvent(g_Dc.hIntrEvent);
        if (g_Dc.hIntrThread) {
            if (WaitForSingleObject(g_Dc.hIntrThread, 5000) != WAIT_OBJECT_0 ) {
                DEBUGCHK(0);
#pragma warning(push)
#pragma warning(disable:6258)
                TerminateThread(g_Dc.hIntrThread, (DWORD)-1);
#pragma warning(pop)
            }
            CloseHandle(g_Dc.hIntrThread);
            g_Dc.hIntrThread = NULL;
        }
        InterruptDisable(g_Dc.dwSysIntr);
        CloseHandle(g_Dc.hIntrEvent);
        g_Dc.hIntrEvent = NULL;
    }
    else
        InterruptDisable(g_Dc.dwSysIntr);

    if (g_Dc.pCppiRegs) 
	{
        MmUnmapIoSpace(g_Dc.pCppiRegs, sizeof(CppiRegs));
        g_Dc.pCppiRegs = NULL;
    }

    g_hDc = 0;

    Unlock();

    DeleteCriticalSection(&g_Dc.csLock);

done:
    DEBUGMSG(ZONE_INIT || ZONE_VERBOSE,
        (L"-UCD_Deinit: %s\r\n",
        (g_hDc == 0) ?
            L"SUCCEEDED" :
            L"FAILED"));

    return (g_hDc == 0);
}

DWORD IntrThread(LPVOID lpParameter)
{
	UNREFERENCED_PARAMETER(lpParameter);

    DEBUGMSG(ZONE_INIT || ZONE_VERBOSE,
        (L"+USBCDMA: IntrThread\n"));

    while (!g_Dc.fIntrThreadClosing)
    {
        WaitForSingleObject(g_Dc.hIntrEvent, INFINITE);

        DEBUGMSG(ZONE_IST && ZONE_VERBOSE,
            (L"+USBCDMA: IntrThread - ISR\r\n"));

        if (g_Dc.fIntrThreadClosing)
		{
            break;
		}

		while (((g_Dc.pCppiRegs->PEND1) & USB_CPPI_PEND1_QMSK_HOST) ||
			   ((g_Dc.pCppiRegs->PEND2) & USB_CPPI_PEND2_QMSK_HOST) ||
			   ((g_Dc.pCppiRegs->PEND2) & USB_CPPI_PEND2_QMSK_FN)	)
        {
            if (g_Dc.usb[0].callback)
			{
                g_Dc.usb[0].callback(g_Dc.usb[0].param);
			}

            if (g_Dc.usb[1].callback)
			{
                g_Dc.usb[1].callback(g_Dc.usb[1].param);
			}
        }

        // In AM3517, this thread is not directly
        // handling interrupts.
#if 0
        InterruptDone(g_Dc.dwSysIntr);
#endif

        DEBUGMSG(ZONE_IST && ZONE_VERBOSE,
            (L"-USBCDMA: IntrThread - ISR\r\n"));
    }

    DEBUGMSG(ZONE_INIT || ZONE_VERBOSE,
        (L"-USBCDMA: IntrThread\n"));

    return 0;
}

void Lock(void)
{
    EnterCriticalSection(&g_Dc.csLock);
}

void Unlock(void)
{
    LeaveCriticalSection(&g_Dc.csLock);
}

BOOL TdPoolInit(void)
{
    /* Only initialise once */
    if (g_Dc.fTdPoolInitialised)
        return TRUE;

    DEBUGMSG(ZONE_INIT,
        (L"+TdPoolInit: %u TDs\r\n",
        USB_CPPI_TD_COUNT));

    Lock();
    DEBUGCHK(g_Dc.pvTdPool != NULL);
    {
    unsigned n;
    TEARDOWN_DESCRIPTOR* pTd = g_Dc.pvTdPool;
    for (n = 0; n < USB_CPPI_TD_COUNT; n++, pTd++) {
        pTd->DescInfo = (UINT32)(USB_CPPI41_DESC_TYPE_TEARDOWN << USB_CPPI41_DESC_TYPE_SHIFT);

        DEBUGMSG(ZONE_INIT && ZONE_VERBOSE,
            (L"TD %04u: PAddr 0x%08x VAddr 0x%08x\r\n",
            n,
            USBCDMA_DescriptorVAtoPA((HANDLE)&g_Dc.usb[0], pTd),
            pTd));
    }
    }
    g_Dc.fTdPoolInitialised = TRUE;
    Unlock();

    DEBUGMSG(ZONE_INIT,
        (L"-TdPoolInit: %s\r\n",
        g_Dc.fTdPoolInitialised ?
            L"SUCCEEDED" :
            L"FALIED"));

    return g_Dc.fTdPoolInitialised;
}

void TdPoolDeinit(void)
{
    /* Only deinitialise once */
    if (g_Dc.fTdPoolInitialised == FALSE)
        return;

    DEBUGMSG(ZONE_INIT,
        (L"+TdPoolDeinit\r\n"));

    Lock();
    g_Dc.fTdPoolInitialised = FALSE;
    g_Dc.paTdPool.QuadPart = 0;
    g_Dc.pvTdPool = NULL;
    Unlock();

    DEBUGMSG(ZONE_INIT,
        (L"-TdPoolDeinit\r\n"));
}

BOOL TdFreeQueueInit(void)
{
    BOOL fInitialised = FALSE;

    DEBUGMSG(ZONE_INIT,
        (L"+TdFreeQueueInit\r\n"));

    /* Teardown descriptor pool must be initialised before teardown free queue */
    TdPoolInit();

    Lock();

    g_Dc.pCppiRegs->TDFDQ =
        (USB_CPPI_XXCMPL_QMGR << 12) |
        (USB_CPPI_TDFREE_QNUM << 0);

    {
		unsigned n;
		TEARDOWN_DESCRIPTOR *pTd = g_Dc.pvTdPool;
		for (n = 0; n < USB_CPPI_TD_COUNT; n++, pTd++)
		{
			QueuePush(USB_CPPI_TDFREE_QNUM, pTd);
		}
    }

    fInitialised = TRUE;

    Unlock();

    DEBUGMSG(ZONE_INIT,
        (L"-TdFreeQueueInit: %s\r\n",
        fInitialised ?
            L"SUCCEEDED" :
            L"FALIED"));

    return fInitialised;
}

void QueuePush(BYTE nQueue, void *pDescriptor)
{
    UINT32 value = 0;

    if (pDescriptor != NULL)
    {
        UINT32 addr  = USBCDMA_DescriptorVAtoPA((HANDLE)&g_Dc.usb[0],pDescriptor);
        UINT32 size  = 0;

        UINT32 type = (*(UINT32*)pDescriptor & USB_CPPI41_DESC_TYPE_MASK) >> USB_CPPI41_DESC_TYPE_SHIFT;
        switch (type)
        {
        case USB_CPPI41_DESC_TYPE_TEARDOWN: /* Teardown descriptor */
            size = (USB_CPPI_TD_SIZE - 24) / 4;
            break;

        default:
            ERRORMSG(TRUE,
                (L"Invalid descriptor type %u\r\n",
                type));
        }

        value = ((addr & QMGR_QUEUE_N_REG_D_DESC_ADDR_MASK) |
                 (size & QMGR_QUEUE_N_REG_D_DESCSZ_MASK));
    }

    g_Dc.pCppiRegs->QMQUEUEMGMT[nQueue].QCTRLD = value;

    DEBUGMSG(ZONE_INFO && 0, (L"QueuePush: Queue %u, Value 0x%08x\r\n", nQueue, value));
}

BOOL IsPow2(unsigned n)
{
    return ((unsigned int)(1 << Log2(n)) == n);
}

unsigned Log2(unsigned n)
{
    unsigned log2 = 0;

    while (n != 0) 
	{
        n >>= 1;
        log2++;
    }

    if (log2 > 0)
	{
        log2--;
	}

    return log2;
}
