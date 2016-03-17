/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/

/** \file  osentry.c 
 *  \brief This module defines unified interface to the OS specific sources and services.
 *
 *  \see  osentry.h 
 */


#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#if defined(BSP_EVM_DM_AM_37X)
#include <sdk_gpio.h>
#else
#include <gpio.h>
#endif
#include <ndis.h>
#include "ndis.h"               
#include "WlanDrvIf.h"
#include "osRgstry.h"
/*#include "osRgstry_extra.h"*/
#include "osrgstry_extra.h"
#include "ParamOut.h"
#include "osEntry.h"
#include "public_types.h"
#include "osDebug.h"
#include "osTIType.h"
#include "host_platform.h"
#ifdef WM_ZOOM2
#include "SdioAdapter.h"
#endif
#ifdef TI_DBG
#include "tracebuf_api.h"
#endif
#ifdef GEM_SUPPORTED
#include "gem_api.h"
#endif

#define NUMBER_OF_PACKETS                   100
#define NUMBER_OF_BUFFERS                   100

#ifdef WM_ZOOM2
/* Enable GPIO power line */
typedef struct{
	int GPIOPWR_LINE;
} PWR_STRUCT;
PWR_STRUCT pwrline;
#endif
/** \brief  Miniport ISR
 * 
 * \param  pfInterruptRecognized   	- Pointer to indicatonr which indicates if interrupt was recognized (True) or not (False)
 * \param  pfQueueDPC   			- Pointer to indicatonr which indicates if there is anything in the Deferred Procedure Call (DPC) Queue
 * \param  MiniportAdapterContext   - Handle to NDIS miniport adapter context in which the ISR will be handled
 * \return void
 * 
 * \par Description
 * This is the implementation of the miniport ISR (Interrupt Service Rutine). 
 * This ISR enables the TWD to handle Interrupt Requests in NDIS miniport adapter context.
 * 
 * \sa
 */ 
void MiniportISR(TI_BOOLEAN* pfInterruptRecognized, TI_BOOLEAN* pfQueueDPC, NDIS_HANDLE MiniportAdapterContext);
/** \brief  Miniport Halt
 * 
 * \param  MiniportAdapterContext   - Handle to halted NDIS miniport adapter context
 * \return void
 * 
 * \par Description
 * This function deallocates resources when miniport NDIS miniport adapter is removed, and halts the NDIS miniport adapter.
 * 
 * \sa
 */ 
void MiniportHalt(IN NDIS_HANDLE MiniportAdapterContext);
/** \brief  Miniport Shut Down
 * 
 * \param  MiniportAdapterContext   - Handle to shut down NDIS miniport adapter context
 * \return void
 * 
 * \par Description
 * This function currently does nothing
 * 
 * \sa
 */ 
void MiniportShutdown(IN NDIS_HANDLE MiniportAdapterContext);
/** \brief  Miniport Send
 * 
 * \param  MiniportAdapterContext   - Handle to send NDIS miniport adapter context
 * \param  PacketArray   			- Pointer to packets array to be sent
 * \param  NumberOfPackets   		- Number of packets to be sent
 * \return void
 * 
 * \par Description
 * This function sends multiple packets via NDIS miniport adapter
 * 
 * \sa
 */ 
void MiniportMultipleSend(NDIS_HANDLE MiniportAdapterContext, PPNDIS_PACKET PacketArray, TI_UINT32 NumberOfPackets);
/** \brief  Miniport Return Packet
 * 
 * \param  MiniportAdapterContext   - Handle to NDIS miniport adapter context
 * \param  Packet   				- Pointer to released packets
 * \return void
 * 
 * \par Description
 * This function releases a packet which was allocated from packets pool, and returns it to packets' free list
 * 
 * \sa
 */ 
void MiniportReturnPacket(NDIS_HANDLE  MiniportAdapterContext, PNDIS_PACKET Packet);
/** \brief  Miniport Reset
 * 
 * \param  pfAddressingReset   		- always set to TI_FALSE 
 * \param  MiniportAdapterContext   - Handle to NDIS miniport adapter context
 * \return NDIS_STATUS_SUCCESS
 * 
 * \par Description
 * This function currently does nothing
 * 
 * \sa
 */ 
NDIS_STATUS MiniportReset(TI_BOOLEAN* pfAddressingReset, IN NDIS_HANDLE MiniportAdapterContext);
/** \brief  Miniport Query Information
 * 
 * \param  MiniportAdapterContext 	- Handle to NDIS miniport adapter context
 * \param  Oid 						- The NDIS Object Identifier to process
 * \param  pInfoBuffer   			- Pointer to output information buffer in which the NDIS Request Query results are stored
 * \param  cbInfoBuffer   			- Pointer to length left in output information buffer
 * \param  pcbWritten   			- Pointer to length which was written in output information buffer
 * \param  pcbNeeded   				- Pointer which will contain the length needed to complete the request, if there is not enough space in the information buffer
 * \return NDIS_STATUS - status of the operation
 * 
 * \par Description
 * This function process a query request for NDIS information with OIDs that are specific to the Driver
 * 
 * \sa
 */ 
NDIS_STATUS MiniportQueryInformation(NDIS_HANDLE MiniportAdapterContext, 
									 NDIS_OID Oid, 
									 PVOID pInfoBuffer, 
									 ULONG cbInfoBuffer, 
									 PULONG pcbWritten, 
									 PULONG pcbNeeded);
/** \brief  Miniport Set Information
 * 
 * \param  MiniportAdapterHandle 	- Handle to NDIS miniport adapter context
 * \param  Oid 						- The NDIS Object Identifier to process
 * \param  pInfoBuffer   			- Pointer to input information buffer from which we take the value to set
 * \param  cbInfoBuffer   			- Length of input information buffer
 * \param  pcbRead   				- Pointer to length which was read from information buffer
 * \param  pcbNeeded   				- Pointer which will contain the length needed to complete the request, if there is not enough space to set all the information buffer
 * \return NDIS_STATUS - status of the operation
 * 
 * \par Description
 * This function process a set request from NDIS information with OIDs that are specific about the Driver
 * 
 * \sa
 */
NDIS_STATUS MiniportSetInformation(NDIS_HANDLE MiniportAdapterHandle, 
								   NDIS_OID Oid, 
								   PVOID pInfoBuffer, 
								   ULONG cbInfoBuffer, 
								   PULONG pcbRead, 
								   PULONG pcbNeeded);

#ifdef TI_DBG
/*
 * A couple msgs won't print early if we fail cause this isn't read or initialized from
 * registry till afterwards.  Driver just unloads quietly with no clue.  This gets over-
 * written from registry very early on.  Just can't be 0 or pAdapter alloc failure won't
 * be seen at all..
 */
unsigned long  TiDebugFlag = ((DBG_INIT << 16) | 
                              /* DBG_SEV_INFO |  */
                              DBG_SEV_LOUD | 
                              DBG_SEV_VERY_LOUD | 
                              /* DBG_SEV_WARNING |  */
                              DBG_SEV_ERROR | 
                              DBG_SEV_FATAL_ERROR
                             );

/* unsigned long  TiDebugFlag; */
#endif

/*--------------------------------------------------------------------------------------*/

static TI_STATUS InitializeInterrupt(TWlanDrvIfObjPtr pAdapter, TI_UINT32 *InterruptVector)
{
   
#ifdef WM_ZOOM2
	HANDLE  hGPIO;	
	NDIS_STRING IrqLine                     = NDIS_STRING_CONST( "IrqLine" );
	NDIS_STRING Wlan_PwrLine                = NDIS_STRING_CONST( "Wlan_PwrLine" );
	int WLAN_GPIO_INTERRUPT_LINE;

	int IRQ_LINE;

	// Configure Interrupt line
    PHYSICAL_ADDRESS PhysicalAddress;
    OMAP_SYSC_PADCONFS_REGS    *pConfig = NULL;

	PhysicalAddress.QuadPart = OMAP_SYSC_PADCONFS_REGS_PA;
	pConfig = (OMAP_SYSC_PADCONFS_REGS*) MmMapIoSpace(PhysicalAddress, sizeof(OMAP_SYSC_PADCONFS_REGS), FALSE);

	if (pConfig == NULL)
    {
        PRINTF(DBG_INIT_FATAL_ERROR,("TI_STATUS InitializeInterrupt() MmMapIoSpace FAILED to map CONTROL_MMC2_CMD_PAD_PA !!!\n"));
        return TI_NOK;
    }

	regReadIntegerParameter(pAdapter, &IrqLine,
                            0, 0,
                            200,
                            sizeof (int), 
                            (TI_UINT8 *)&IRQ_LINE);
    
	
	regReadIntegerParameter(pAdapter, &Wlan_PwrLine,
                            0, 0,
                            200,
                            sizeof (int), 
                            (TI_UINT8 *)&pwrline.GPIOPWR_LINE);
 
    hGPIO = GPIOOpen();
    if(hGPIO == NULL) 
    {
        PRINTF(DBG_INIT_FATAL_ERROR,("Error open GPIO handle.\n"));
        return TI_NOK;
    }

     /* set GPIO 149 pin to input direction */
    OUTREG16((unsigned short*)&pConfig->CONTROL_PADCONF_UART1_RTS, (INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_4));    /*UART1_RTS is CB_HOST_WL_IRQ*/
	MmUnmapIoSpace ((PVOID)pConfig, sizeof(OMAP_SYSC_PADCONFS_REGS));


    GPIOSetMode(hGPIO, IRQ_LINE, GPIO_DIR_OUTPUT);
    GPIOSetBit(hGPIO,  IRQ_LINE);
#ifdef FPGA1273_STAGE_
    GPIOSetMode(hGPIO, WLAN_GPIO_INTERRUPT_LINE, GPIO_DIR_INPUT | GPIO_INT_LOW_HIGH);
#else
#ifndef USE_IRQ_ACTIVE_HIGH
    GPIOSetMode(hGPIO, IRQ_LINE, GPIO_DIR_INPUT | GPIO_INT_HIGH_LOW);
#else
    GPIOSetMode(hGPIO, WLAN_GPIO_INTERRUPT_LINE, GPIO_DIR_INPUT | GPIO_INT_LOW_HIGH);
#endif
#endif

	if(!GPIOIoControl(

            hGPIO, 

            IOCTL_GPIO_GET_OMAP_HW_INTR,

            (UCHAR*)&IRQ_LINE,

            sizeof(IRQ_LINE),

            (UCHAR*)InterruptVector,

            sizeof(*InterruptVector),

            0,

            0))

        {

        PRINTF(DBG_INIT_FATAL_ERROR,("Error getting hardware interrupt.\n"));

        return TI_NOK;

        }
   GPIOClose(hGPIO);
#else
   hPlatform_Wlan_Intr_Enable(pAdapter, InterruptVector);
#endif

    return TI_OK;

} /* InitializeInterrupt() */

/*--------------------------------------------------------------------------------------*/
/* 
 * \fn     DllEntry
 * \brief  This function provides the DLL entry point required for the WinCE driver.
 *
 *	Since all network drivers are implemented as DLL's in WinCE, this entry point is needed.
 *
 * \note  don't really know what this function is used for, 
 * \note  it has basically been reproduced here from the WinCE sample driver. 
 * \note  Just safer to return TRUE than FALSE :-) 
 * \param  hDLL, handle identifying this DLL.
 * \param  dwReason, reason of invocation of this entry point.
 * \param  lpReserved, reserved param
 * \return  BOOL - Always TRUE
 */
BOOL __stdcall DllEntry( HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
{
    switch(dwReason) 
	{
    case DLL_PROCESS_ATTACH:
        PRINT(DBG_INIT_LOUD, "TI WLAN: DLL_PROCESS_ATTACH\n");
        DisableThreadLibraryCalls ((HMODULE)hDLL);
        break;
    case DLL_PROCESS_DETACH:
        PRINT(DBG_INIT_LOUD, "TI WLAN: DLL_PROCESS_DETACH\n");
        break;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------------------*/

/** \brief  Miniport Un-Initialize
 * 
 * \param  pAdapter 	- Pointer to WLAN Driver Adapter Object
 * \return void
 * 
 * \par Description
 * This function un-initializes NDIS miniport adapter by performing the follow:
 * Releasing all handled to files used by the adapter, including freeing all memory allocated to hold the files contents when they were opened.
 * Releasing the configuration handle, including freeing all memory allocated for it.
 * Releasing the handle to a block of packet pool and freeing all memory allocated for it.
 * Closing the Driver Thread object handle.
 * Freeing all memory allocated for WLAN Driver Adapter.
 * 
 * \sa
 */ 
VOID MiniportUnInitialize(TWlanDrvIfObjPtr pAdapter)
{
    pAdapter->bRunDrv = FALSE;

    if (pAdapter->FirmwareImageHandle)
    {    
        NdisCloseFile(pAdapter->FirmwareImageHandle);
    }
    if (pAdapter->EepromImageHandle)
    {
        NdisCloseFile(pAdapter->EepromImageHandle);
    }
    if (pAdapter->ConfigHandle)
    {
        NdisCloseConfiguration(pAdapter->ConfigHandle);
    }   
    if (pAdapter->PacketPoolHandle)
    {
        NdisFreePacketPool(pAdapter->PacketPoolHandle);
    }
    if (pAdapter->BufferPoolHandle)
    {
        NdisFreeBufferPool(pAdapter->BufferPoolHandle);
    }
    if (pAdapter->drvThread)
    {
        SetEvent(pAdapter->drvEvent);
        WaitForSingleObject(pAdapter->drvThread, INFINITE); 
        CloseHandle(pAdapter->drvThread);
    }
    if (pAdapter->drvEvent)
    {
        CloseHandle(pAdapter->drvEvent);
    }
    NdisFreeMemory((PVOID)pAdapter, (UINT)sizeof(TWlanDrvIfObj), 0);

} /* MiniportUnInitialize(pTWlanDrvIf) */

/*--------------------------------------------------------------------------------------*/

/* 
 * \fn     MiniportInitialize
 * \brief  tiacxwlnMiniportInitialize registers resources with the wrapper and starts the adapter.
 *
 * \param  OpenErrorStatus - Extra status bytes for opening token ring adapters.
 * \param  SelectedMediumIndex - Index of the media type chosen by the driver.
 * \param  MediumArray - Array of media types for the driver to chose from.
 * \param  MediumArraySize - Number of entries in the array.
 * \param  MiniportAdapterHandle - Handle for passing to the wrapper when referring to this adapter.
 * \param  ConfigurationHandle - A handle to pass to NdisOpenConfiguration.
 * \return  NDIS_STATUS - The status of the operation.
 */
#pragma NDIS_PAGEABLE_FUNCTION(MiniportInitialize)
NDIS_STATUS MiniportInitialize(
	OUT PNDIS_STATUS pStatus,
	OUT TI_UINT32* pSelectedMediumIndex,
	IN PNDIS_MEDIUM pMediumArray, 
	IN TI_UINT32 cMediumArrayEntries,
	IN NDIS_HANDLE hMiniportHandle,
	IN NDIS_HANDLE hConfigContext
	)
{
	NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    TWlanDrvIfObjPtr    pAdapter;
    ULONG               i;
    DWORD               dwInitStartTimestamp = GetTickCount();
    DWORD               dwInitEndTimestamp;
    TI_UINT32           InterruptVector;
    
	PRINT(DBG_INIT_LOUD, "TIWL: entryMiniportInitialize In\n");
  
	#ifdef TI_DBG
		tb_init(TB_OPTION_NONE);
	#endif
	/* Find the supported media in the list */
	for(i = 0; i<cMediumArrayEntries; i++) 
	{
		if(pMediumArray[i] == NdisMedium802_3) 
		{
			break;
		}
	}
	if(i == cMediumArrayEntries) 
	{
		Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
		PRINT(DBG_INIT_LOUD, "TIWL: entryMiniportInitialize Out NDIS_STATUS_UNSUPPORTED_MEDIA\n");
		return Status;
	}
	/* Return the selected medium index */
	*pSelectedMediumIndex = i;
	/* Allocate enough memory for the adapter context */
    pAdapter = os_memoryAlloc(NULL, sizeof(TWlanDrvIfObj));
	if(Status != NDIS_STATUS_SUCCESS) 
	{
        PRINT(DBG_INIT_LOUD, "TIWL: os_memoryAlloc failed!!!\n");
		return Status;
	}
	/* Zero out the adapter object space */
    os_memoryZero(pAdapter, pAdapter, sizeof(TWlanDrvIfObj));
	/* Save Miniport adapter handle for future use */
	pAdapter->MiniportHandle = hMiniportHandle;
	NdisOpenConfiguration(&Status, &pAdapter->ConfigHandle, hConfigContext);
	if(Status != NDIS_STATUS_SUCCESS) 
	{
		MiniportUnInitialize(pAdapter);
		PRINT(DBG_INIT_LOUD, "TIWL: Could'n open the device configuration key\n");
		return Status;
	}
	#ifdef TI_DBG
		regReadLastDbgState(pAdapter);
	#endif
	NdisMSetAttributesEx(hMiniportHandle, (NDIS_HANDLE) pAdapter, 0, NDIS_ATTRIBUTE_DESERIALIZE, NdisInterfaceInternal);
	pAdapter->LinkStatus = NdisMediaStateDisconnected;
	NdisAllocatePacketPool(&Status, &pAdapter->PacketPoolHandle, NUMBER_OF_PACKETS, 16);
    if(Status != NDIS_STATUS_SUCCESS) 
    {
        PRINT(DBG_INIT_LOUD, "TIWL: NdisAllocatePacketPool() failed!!!\n");
        MiniportUnInitialize(pAdapter);
        return Status;
    }
	NdisAllocateBufferPool(&Status, &pAdapter->BufferPoolHandle,	NUMBER_OF_BUFFERS);
    if(Status != NDIS_STATUS_SUCCESS) 
    {
        PRINT(DBG_INIT_LOUD, "TIWL: NdisAllocateBufferPool() failed!!!\n");
        MiniportUnInitialize(pAdapter);
        return Status;
    }
    pAdapter->drvEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pAdapter->drvEvent == NULL) 
    {
        PRINTF(DBG_PCI_RES_ERROR, ("TIWL: Could not create drvEvent(%X)\n", GetLastError()));
        MiniportUnInitialize(pAdapter);
        return NDIS_STATUS_FAILURE;
    }
    pAdapter->bRunDrv = TRUE;
    pAdapter->drvThread = CreateThread(NULL, 0, wlanDrvIf_DriverTask, (LPVOID)pAdapter, 0, NULL);
    if (pAdapter->drvThread == NULL) 
    {
        PRINTF(DBG_PCI_RES_ERROR, ("TIWL: Could not create drvThread(%X)\n", GetLastError()));
        MiniportUnInitialize(pAdapter);
        return NDIS_STATUS_FAILURE;
    }
    if (InitializeInterrupt(pAdapter, &InterruptVector) != TI_OK)
    {
        PRINT(DBG_INIT_LOUD, "TIWL: InitializeInterrupt() failed!!!\n");
        MiniportUnInitialize(pAdapter);
        return NDIS_STATUS_FAILURE;
    }
#ifdef WM_ZOOM2
	Get_GPIOPWR_LINE(pwrline.GPIOPWR_LINE);
#endif

    if (wlanDrvIf_Create(pAdapter) != TI_OK)
    {
        PRINTF(DBG_PCI_RES_ERROR, ("TIWL: wlanDrvIf_Create failed!!\n"));
        MiniportUnInitialize(pAdapter);
        return NDIS_STATUS_FAILURE;
    }

    Status = NdisMRegisterInterrupt(&pAdapter->Interrupt, pAdapter->MiniportHandle, InterruptVector,
                                    0, TRUE, FALSE, NdisInterruptLatched);
    if(Status != NDIS_STATUS_SUCCESS) 
    {
        PRINTF(DBG_PCI_RES_ERROR, ("TIWL: Could not register interrupt(%X)\n", Status));
        MiniportUnInitialize(pAdapter);
        return NDIS_STATUS_FAILURE;
    }
	PRINT(DBG_INIT_LOUD, "TIWL: entryMiniportInitialize wlanDrvIf_Start()\n");

#ifdef WM_ZOOM2
/* Perform check for hardware - See SdioAdapter.c */
#ifdef CHK_HW_PRESENT

    if(Hw_Check())
    {		
     	wlanDrvIf_Start(pAdapter);
    }
    else
    {		
		RETAILMSG(1,(TEXT("TIWL: WiFi Hardware Not Found. Please press key 'F5' to continue \n")));
        MiniportUnInitialize(pAdapter);
		return NDIS_STATUS_FAILURE;
    }
#endif
#else
    PRINT(DBG_INIT_LOUD, "TIWL: entryMiniportInitialize wlanDrvIf_Start()\n");
    if (wlanDrvIf_Start(pAdapter) != TI_OK)
    {
    WLAN_OS_REPORT(("wlanDrvIf_Start(): Failed\n"));
    return NDIS_STATUS_FAILURE;
    }
#endif    
    /* Set event load done */
    dwInitEndTimestamp = GetTickCount();
    pAdapter->InitTime = (dwInitEndTimestamp > dwInitStartTimestamp) ? 
        (dwInitEndTimestamp - dwInitStartTimestamp) : (((DWORD)-1 - dwInitStartTimestamp) + dwInitEndTimestamp);

	PRINT(DBG_INIT_LOUD, "TIWL: entryMiniportInitialize Good Init Out\n");

#ifdef GEM_SUPPORTED
	(VOID)GEM_InitializeHandler( pStatus,
								  pSelectedMediumIndex,
								  pMediumArray,
								  cMediumArrayEntries,
								  hMiniportHandle,
								  hConfigContext );
#endif

    return Status;

} /* MiniportInitialize() */

/*--------------------------------------------------------------------------------------*/

/* 
 * \fn     DriverEntry
 * \brief  This is the primary initialization routine for the TI WLAN driver.
 *
 *	It is simply responsible for the intializing the wrapper and registering
 *  the Miniport driver.  It then calls a system and architecture specific
 *  routine that will initialize and register each adapter.
 *
 * \param  DriverObject - Pointer to driver object created by the system.
 * \param  RegistryPath - Path to the parameters for this driver in the registry.
 * \return  NTSTATUS - The status of the operation.
 */
#if !defined(BSP_EVM_DM_AM_37X)
#include <oal.h>
#include <oalex.h>
#include <args.h>
#endif

#pragma NDIS_INIT_FUNCTION(DriverEntry)
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
	NDIS_STATUS						rc = NDIS_STATUS_SUCCESS;
	NDIS51_MINIPORT_CHARACTERISTICS Chars;
    NDIS_HANDLE                     NdisWrapper;
#if !defined(BSP_EVM_DM_AM_37X)
	card_ver						m_cv; // card vesion
	UINT32							in = OAL_ARGS_QUERY_WLAN_CFG;
#endif

	PRINT(DBG_INIT_LOUD, "TIWL: DriverEntry In\n");


	if(!KernelIoControl(IOCTL_HAL_QUERY_CARD_CFG, &in, sizeof(in), &m_cv, sizeof(m_cv), 0))
	{
		m_cv.card_addr	= -1;
		m_cv.ver		= -1;
		m_cv.rev		= -1;
		m_cv.config		= -1;
	}

	if(m_cv.ver == -1 || m_cv.ver < 300)
        return NDIS_STATUS_FAILURE;

	/* Initialize the wrapper */
	NdisMInitializeWrapper(&NdisWrapper, pDriverObject, pRegistryPath, NULL);
	/* Set up the characteristics structure */
	NdisZeroMemory(&Chars, sizeof(Chars));
	Chars.MajorNdisVersion			= 5;
	Chars.MinorNdisVersion			= 1;
    Chars.CheckForHangHandler       = NULL;
    Chars.InitializeHandler         = MiniportInitialize;
    Chars.HaltHandler               = MiniportHalt;
    Chars.HandleInterruptHandler    = NULL;
    Chars.ISRHandler                = MiniportISR;
    Chars.QueryInformationHandler   = MiniportQueryInformation;
	Chars.ReconfigureHandler		= NULL;
    Chars.ResetHandler              = MiniportReset;
	Chars.ReturnPacketHandler		= MiniportReturnPacket;
	Chars.SendPacketsHandler		= MiniportMultipleSend;
	Chars.TransferDataHandler		= NULL;
    Chars.SetInformationHandler     = MiniportSetInformation;
	Chars.AdapterShutdownHandler	= MiniportShutdown;
    /* Register the Miniport */
	rc = NdisMRegisterMiniport(NdisWrapper, &Chars, sizeof(Chars));
	if(rc != NDIS_STATUS_SUCCESS) 
	{
		/* Couldn't register miniport */
		NdisWriteErrorLogEntry(&NdisWrapper, NDIS_ERROR_CODE_DRIVER_FAILURE, 0, 0);
		NdisTerminateWrapper(NdisWrapper, NULL);
		return rc;
	}
	PRINT(DBG_INIT_LOUD, "TIWL: DriverEntry Out\n");

	return rc;

} /* DriverEntry() */

/*--------------------------------------------------------------------------------------*/

/*******************************************************************/
/*  Need to update drvMain_Sm() to close/unmap FW image/nvs files  */
/*******************************************************************/

