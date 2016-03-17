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
 
/** \file   SdioDrv.c 
 *  \brief  The OMAP3530 WinCE SDIO driver (platform and OS dependent) 
 * 
 * The lower SDIO driver (BSP) for OMAP3530 on WinCE OS.
 * Provides all SDIO commands and read/write operation methods.
 *  
 *  \see    SdioDrv.h
 */

#include <windows.h>
#include <initguid.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <pm.h>
#include <oal_io.h>
#include <omap3530.h>
#include <oal.h>
#include <nkintr.h>
#include <dma_utility.h>

#include "omap_types.h"
#include "omap3530_config.h"
#include "omap3530_irq.h"
#include "sdiodrvdbg.h"
#include "SdioDrv.h"


/************************************************************************
 * Defines
 ************************************************************************/
#define TIWLAN_MMC_CONTROLLER              2
#define TIWLAN_MMC_CONTROLLER_BASE_SIZE    512
#define TIWLAN_MMC_MAX_DMA                 16384 //8192

#define OMAP_MMC_MASTER_CLOCK          96000000
/*
 *  HSMMC Host Controller Registers
 */
#define OMAP_HSMMC_SYSCONFIG           0x0010
#define OMAP_HSMMC_SYSSTATUS           0x0014
#define OMAP_HSMMC_CSRE                0x0024
#define OMAP_HSMMC_SYSTEST             0x0028
#define OMAP_HSMMC_CON                 0x002C
#define OMAP_HSMMC_BLK                 0x0104
#define OMAP_HSMMC_ARG                 0x0108
#define OMAP_HSMMC_CMD                 0x010C
#define OMAP_HSMMC_RSP10               0x0110
#define OMAP_HSMMC_RSP32               0x0114
#define OMAP_HSMMC_RSP54               0x0118
#define OMAP_HSMMC_RSP76               0x011C
#define OMAP_HSMMC_DATA                0x0120
#define OMAP_HSMMC_PSTATE              0x0124
#define OMAP_HSMMC_HCTL                0x0128
#define OMAP_HSMMC_SYSCTL              0x012C
#define OMAP_HSMMC_STAT                0x0130
#define OMAP_HSMMC_IE                  0x0134
#define OMAP_HSMMC_ISE                 0x0138
#define OMAP_HSMMC_AC12                0x013C
#define OMAP_HSMMC_CAPA                0x0140
#define OMAP_HSMMC_CUR_CAPA            0x0148
#define OMAP_HSMMC_REV                 0x01FC


#define VS18                           (1 << 26)
#define VS30                           (1 << 25)
#define SRA                            (1 << 24)
#define SDVS18                         (0x5 << 9)
#define SDVS30                         (0x6 << 9)
#define SDVSCLR                        0xFFFFF1FF
#define SDVSDET                        0x00000400  
#define SIDLE_MODE                     (0x2 << 3)   
#define AUTOIDLE                       0x1        
#define SDBP                           (1 << 8)     
#define DTO                            0xE        
#define ICE                            0x1        
#define ICS                            0x2        
#define CEN                            (1 << 2)     
#define CLKD_MASK                      0x0000FFC0 
#define IE_EN_MASK                     0x317F0137  
#define INIT_STREAM                    (1 << 1)     
#define DP_SELECT                      (1 << 21)    
#define DDIR                           (1 << 4)     
#define DMA_EN                         0x1        
#define MSBS                           (1 << 5)
#define BCE                            (1 << 1)     
#define ONE_BIT                        (~(0x2))
#define EIGHT_BIT                      (~(0x20))   
#define CC                             0x1        
#define TC                             0x02       
#define OD                             0x1        
#define BRW                            0x400      
#define BRR                            0x800      
#define BRE                            (1 << 11)    
#define BWE                            (1 << 10)    
#define SBGR                           (1 << 16)    
#define CT                             (1 << 17)    
#define SDIO_READ                      (1 << 31)    
#define SDIO_BLKMODE                   (1 << 27)    
#define OMAP_HSMMC_ERR                 (1 << 15)  /* Any error */
#define OMAP_HSMMC_CMD_TIMEOUT         (1 << 16)  /* Com mand response time-out */
#define OMAP_HSMMC_DATA_TIMEOUT        (1 << 20)  /* Data response time-out */
#define OMAP_HSMMC_CMD_CRC             (1 << 17)  /* Command CRC error */
#define OMAP_HSMMC_DATA_CRC            (1 << 21)  /* Date CRC error */
#define OMAP_HSMMC_CARD_ERR            (1 << 28)  /* Card ERR */
#define OMAP_HSMMC_STAT_CLEAR          0xFFFFFFFF 
#define INIT_STREAM_CMD                0x00000000 
#define INT_CLEAR                      0x00000000 
#define BLK_CLEAR                      0x00000000 


#define MMC1_ACTIVE_OVERWRITE          (1 << 31)                      
                                                                    
#define sdio_blkmode_regaddr           0x2000                       
#define sdio_blkmode_mask              0xFF00                       
                                                                    
#define IO_RW_DIRECT_MASK              0xF000FF00                   
#define IO_RW_DIRECT_ARG_MASK          0x80001A00                   
                                                                    
#define RMASK                          (MMC_RSP_MASK | MMC_RSP_CRC)   
#define MMC_TIMEOUT_MS                 20
#define MMCA_VSN_4                     4                            
                                                                    
#define VMMC1_DEV_GRP                  0x27                         
#define P1_DEV_GRP                     0x20                         
#define VMMC1_DEDICATED                0x2A                         
#define VSEL_3V                        0x02                         
#define VSEL_18V                       0x00                         
#define PBIAS_3V                       0x03                         
#define PBIAS_18V                      0x02                         
#define PBIAS_LITE                     0x04A0                       
#define PBIAS_CLR                      0x00                         

#define OMAP_MMC_REGS_BASE             OMAP_MMCHS2_REGS_PA /*IO_ADDRESS(OMAP_HSMMC2_BASE)*/

/* 
 * MMC Host controller read/write API's.
 */
#define OMAP_HSMMC_READ_OFFSET(base,offset) (*(volatile ULONG * const)((base) + (offset)))
#define OMAP_HSMMC_READ(base,reg)           (*(volatile ULONG * const)((base) + OMAP_HSMMC_##reg))
#define OMAP_HSMMC_WRITE(base, reg, val)    ((*(volatile ULONG * const)((base) + OMAP_HSMMC_##reg)) = (val))

#define OMAP_HSMMC_SEND_COMMAND(base, cmd, arg) do \
{ \
	OMAP_HSMMC_WRITE(base, ARG, arg); \
	OMAP_HSMMC_WRITE(base, CMD, cmd); \
} while (0)

#define OMAP_HSMMC_CMD52_WRITE     ((SD_IO_RW_DIRECT    << 24) | (OMAP_HSMMC_CMD_SHORT_RESPONSE << 16))
#define OMAP_HSMMC_CMD52_READ      (((SD_IO_RW_DIRECT   << 24) | (OMAP_HSMMC_CMD_SHORT_RESPONSE << 16)) | DDIR)
#define OMAP_HSMMC_CMD53_WRITE     (((SD_IO_RW_EXTENDED << 24) | (OMAP_HSMMC_CMD_SHORT_RESPONSE << 16)) | DP_SELECT)
#define OMAP_HSMMC_CMD53_READ      (((SD_IO_RW_EXTENDED << 24) | (OMAP_HSMMC_CMD_SHORT_RESPONSE << 16)) | DP_SELECT | DDIR)
#define OMAP_HSMMC_CMD53_READ_DMA  (OMAP_HSMMC_CMD53_READ  | DMA_EN)
#define OMAP_HSMMC_CMD53_WRITE_DMA (OMAP_HSMMC_CMD53_WRITE | DMA_EN)

/* Macros to build commands 52 and 53 in format according to SDIO spec */
#define SDIO_CMD52_READ(v1,v2,v3,v4)        (SDIO_RWFLAG(v1)|SDIO_FUNCN(v2)|SDIO_RAWFLAG(v3)| SDIO_ADDRREG(v4))
#define SDIO_CMD52_WRITE(v1,v2,v3,v4,v5)    (SDIO_RWFLAG(v1)|SDIO_FUNCN(v2)|SDIO_RAWFLAG(v3)| SDIO_ADDRREG(v4)|(v5))
#define SDIO_CMD53_READ(v1,v2,v3,v4,v5,v6)  (SDIO_RWFLAG(v1)|SDIO_FUNCN(v2)|SDIO_BLKM(v3)| SDIO_OPCODE(v4)|SDIO_ADDRREG(v5)|(v6&0x1ff))
#define SDIO_CMD53_WRITE(v1,v2,v3,v4,v5,v6) (SDIO_RWFLAG(v1)|SDIO_FUNCN(v2)|SDIO_BLKM(v3)| SDIO_OPCODE(v4)|SDIO_ADDRREG(v5)|(v6&0x1ff))
#define FORCEIDLE	0xFFFFFFE7

#define SDIODRV_MAX_LOOPS 50000

/************************************************************************
 * Types
 ************************************************************************/
typedef enum 
{
	AsyncRead = 0,
	AsyncWrite= 1
} AsyncDirection_e;

typedef struct OMAP3530_sdiodrv
{
    BOOL                 bRunDrv;
    CEDEVICE_POWER_STATE powerState;
    unsigned long        regs_base;
    HANDLE               hParentBus;
    HANDLE               IST;
    HANDLE               ist_event;
    DWORD                mmc_intr;
    void                 (*BusTxnCB)(void* BusTxnHandle, int status);
    void                *BusTxnHandle;
    /* DMA/Asyc stuff */
    HANDLE               dma_tx_channel;
    DmaDataInfo_t        dma_tx_info;
    OMAP_DMA_LC_REGS*    dma_rx_regs;
    HANDLE               dma_rx_channel;
    DmaDataInfo_t        dma_rx_info;
    char                 *dma_buffer;
    PHYSICAL_ADDRESS     dma_buffer_phys;
    AsyncDirection_e     async_direction;
    unsigned int         uBlkSize;
    unsigned int         uBlkSizeShift;
	BOOL                 Clock_Enable;
	
}  OMAP3530_sdiodrv_t;

static OMAP3530_sdiodrv_t g_drv;

int g_ssd_debug_level=SSD_DEBUGLEVEL_ERR;

void sdiodrv_clkset();
void sdiodrv_clkstop();
static void sdioDrv_ShutdownDma (void);
static int sdioDrv_InitDma (void);
static BOOL sdioDrv_SetPowerState (CEDEVICE_POWER_STATE ePowerState);
static int sdioDrv_InterruptDone (void);
static WINAPI sdiodrv_IST(LPVOID ptr);
static unsigned long sdioDrv_PollStatus (unsigned long uRegOffset, 
                                         unsigned long uStatusMask);
static void sdioDrv_StopClock (void);
static void sdioDrv_SetClock (unsigned int clock);
static int sdioDrv_ResetMmc (void);
static int sdioDrv_InitHw (void);
static int sdioDrv_SetDevicePowerState (void);
static int sdioDrv_SendCmd (unsigned int uCmdReg, unsigned int uArg);
static int sdioDrv_SendDataXferCmd (unsigned int uCmd, 
                                    unsigned int uArg, 
                                    unsigned int uLen, 
                                    unsigned int uStatusMask,
                                    unsigned int bBlkMode);
static int sdioDrv_DataXferSync (unsigned int  uCmd, 
                                 unsigned int  uArg, 
                                 unsigned char *pData, 
                                 unsigned int  uLen, 
                                 unsigned int  uStatusMask);

DWORD SDI_Open(DWORD context, DWORD accessCode, DWORD shareMode);
int SDI_Init (LPCTSTR szContext);
DWORD SDI_Read(DWORD context, VOID *pBuffer, DWORD size);
BOOL SDI_Close(DWORD context);
DWORD SDI_Write(DWORD context, VOID *pBuffer, DWORD size);
void SDI_Deinit (void);
BOOL SetDevicePowerState ( HANDLE , CEDEVICE_POWER_STATE , PVOID );
HANDLE CreateBusAccessHandle (LPCTSTR);
VOID MmUnmapIoSpace( PVOID, ULONG);
PVOID MmMapIoSpace( PHYSICAL_ADDRESS , ULONG, BOOLEAN );

/************************************************************************
 *
 *   Module functions implementation
 *
 ************************************************************************/

/*--------------------------------------------------------------------------------------*/

/*======================================================================================*/
/*----------------------------  Windows interface  -------------------------------------*/
/*======================================================================================*/

BOOL __stdcall DllEntry(HINSTANCE  hInstance,
                        ULONG      Reason,
                        LPVOID     pReserved)
{
   switch (Reason)
   {
      case DLL_PROCESS_ATTACH:
		  DisableThreadLibraryCalls((HMODULE) hInstance);
		  break;

	  default:
		  RETAILMSG(1,(TEXT("*********** DllEntry :: default case **********\n\n")));
   }
     
   return TRUE;
}

/*--------------------------------------------------------------------------------------*/

void SDI_Deinit (void)
{
    g_drv.bRunDrv = FALSE;

		
    if (g_drv.regs_base != 0)
    {
        MmUnmapIoSpace ((PVOID)g_drv.regs_base , TIWLAN_MMC_CONTROLLER_BASE_SIZE);
        g_drv.regs_base = 0;
    }
    
    if (g_drv.mmc_intr != 0)
    {
        KernelIoControl (IOCTL_HAL_RELEASE_SYSINTR, &g_drv.mmc_intr, sizeof(unsigned long), NULL, 0, NULL);
        g_drv.mmc_intr = 0;
    }
    if (g_drv.ist_event != NULL)
    {
        CloseHandle(g_drv.ist_event);
        g_drv.ist_event = NULL;
    }
    if (g_drv.IST != NULL) 
    {
        SetEvent (g_drv.ist_event);
        WaitForSingleObject (g_drv.IST, INFINITE); 
        CloseHandle (g_drv.IST);
        g_drv.IST = NULL;
    }
	
    if(g_drv.dma_buffer)
    {
        HalFreeCommonBuffer(NULL, 0, g_drv.dma_buffer_phys, g_drv.dma_buffer, FALSE);
        g_drv.dma_buffer = NULL;
    }

}



/*--------------------------------------------------------------------------------------*/

int SDI_Init (LPCTSTR szContext)
{

    PHYSICAL_ADDRESS PhysicalAddress;
    unsigned long    irq_mmc2 = IRQ_MMC2, thread_id;
    OMAP_SYSC_PADCONFS_REGS	*pConfig;
	OMAP_SYSC_GENERAL_REGS	*pConfigGeneral; 
    int              rc = 0;
	DMA_ADAPTER_OBJECT dma_adapter;

    g_ssd_debug_level = SSD_DEBUGLEVEL_ERR;

    g_drv.Clock_Enable = 1;
    memset (&g_drv,0,sizeof(g_drv));
    g_drv.bRunDrv = TRUE;

    /* Pull up the command and data lines */
	PhysicalAddress.QuadPart = OMAP_SYSC_PADCONFS_REGS_PA;
	pConfig = (OMAP_SYSC_PADCONFS_REGS*) MmMapIoSpace(PhysicalAddress, sizeof(OMAP_SYSC_PADCONFS_REGS), FALSE);

    if (pConfig == NULL)
    {
        PERR("sdiodrv_init() MmMapIoSpace FAILED to map CONTROL_MMC2_CMD_PAD_PA !!!\n");
        goto error_exit;
    }

    OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_CLK, (INPUT_ENABLE | PULL_UP | MUX_MODE_0));           /*MMC2_CLK*/
    OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_CMD, (INPUT_ENABLE | PULL_UP | MUX_MODE_0));            /*MMC2_CMD*/
    OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT0, (INPUT_ENABLE | PULL_UP | MUX_MODE_0));           /*MMC2_DAT0*/
    OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT1, (INPUT_ENABLE | PULL_UP | MUX_MODE_0));           /*MMC2_DAT1*/
    OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT2, (INPUT_ENABLE | PULL_UP | MUX_MODE_0));           /*MMC2_DAT2*/
    OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT3, (INPUT_ENABLE | PULL_UP | MUX_MODE_0));           /*MMC2_DAT3*/
    OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT7, (INPUT_ENABLE | PULL_UP | MUX_MODE_0));           /*MMC2_DAT7 is unused (clkin disable)*/
	MmUnmapIoSpace ((PVOID)pConfig, sizeof(OMAP_SYSC_PADCONFS_REGS));

    /* Init the MMC controller access */
    PhysicalAddress.QuadPart = OMAP_MMC_REGS_BASE; 
    g_drv.regs_base = (unsigned long)MmMapIoSpace(PhysicalAddress, TIWLAN_MMC_CONTROLLER_BASE_SIZE, FALSE);
    if (g_drv.regs_base == 0)
    {
        PERR("sdiodrv_init() MmMapIoSpace FAILED to map TIWLAN_MMC_CONTROLLER_BASE_ADDR !!!\n");
        goto error_exit;
    }    

    /* Configure the clock output for clock line */
	PhysicalAddress.QuadPart = OMAP_SYSC_GENERAL_REGS_PA;
	pConfigGeneral = (OMAP_SYSC_GENERAL_REGS*) MmMapIoSpace(PhysicalAddress, sizeof(OMAP_SYSC_GENERAL_REGS), FALSE);

    if (pConfigGeneral == NULL)
    {
        PERR("sdiodrv_init() MmMapIoSpace FAILED to map OMAP_SYSC_GENERAL_REGS_PA !!!\n");
        goto error_exit;
    }

	g_drv.hParentBus = CreateBusAccessHandle(szContext);
    if (g_drv.hParentBus == NULL) 
    {
        PERR("sdiodrv_init() CreateBusAccessHandle FAILED !!!\n");
        goto error_exit;
    }

	//input clock is copied from the module output clock
	OUTREG32(&pConfigGeneral->CONTROL_DEVCONF1, (DEVCONF1_MMCSDIO2ADPCLKISEL));         
	MmUnmapIoSpace ((PVOID)pConfigGeneral, sizeof(OMAP_SYSC_GENERAL_REGS));		

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, 
                         &irq_mmc2, 
                         sizeof(unsigned long), 
                         &g_drv.mmc_intr, 
                         sizeof(unsigned long), 
                         NULL))
    {
        PERR("sdiodrv_init() KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR) FAILED !!!\n");
        g_drv.mmc_intr = SYSINTR_UNDEFINED;
        goto error_exit;
    }
    /* allocate the interrupt event for the SDIO/controller interrupt */
    g_drv.ist_event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_drv.ist_event == NULL) 
    {
        PERR("sdiodrv_init() CreateEvent FAILED !!!\n");
        goto error_exit;
    }
    if (!InterruptInitialize(g_drv.mmc_intr, g_drv.ist_event, NULL, 0 )) 
    {
        PERR("sdiodrv_init() InterruptInitialize FAILED !!!\n");
        goto error_exit;
    }
    g_drv.IST = CreateThread(NULL, 0, sdiodrv_IST, NULL, 0, &thread_id);
    if (g_drv.IST == NULL) 
    {
        PERR("sdiodrv_init() CreateThread FAILED !!!\n");
        goto error_exit;
    }

	/* calling DMA_init is moved to sdioDrv_InitHw, to fix DMA problem while resuming after suspend */

	dma_adapter.ObjectSize    = sizeof(dma_adapter);
    dma_adapter.InterfaceType = Internal;
    dma_adapter.BusNumber     = 0;

    g_drv.dma_buffer = (char *)HalAllocateCommonBuffer(&dma_adapter, TIWLAN_MMC_MAX_DMA, &g_drv.dma_buffer_phys, FALSE);
    if( g_drv.dma_buffer == NULL )
    {
        PERR("sdioDrv_InitDma() HalAllocateCommonBuffer FAILED !!\n");
        return ERROR_1;
    }

    if (rc == 0) /* OK */
    {
        rc = 1;
    }
    return rc;

error_exit:
    SDI_Deinit ();
    return ERROR_1;
}

/*--------------------------------------------------------------------------------------*/

DWORD SDI_Open(DWORD context, DWORD accessCode, DWORD shareMode)
{
	PDEBUG1("%s() called\n",__FUNCTION__);

    return TRUE;
} 

/*--------------------------------------------------------------------------------------*/

BOOL SDI_Close(DWORD context)
{
	PDEBUG1("%s() called\n",__FUNCTION__);

    return TRUE;
} 

/*--------------------------------------------------------------------------------------*/

DWORD SDI_Read(DWORD context, VOID *pBuffer, DWORD size)
{
	PDEBUG1("%s() called\n",__FUNCTION__);

    return SUCCESS;
}

/*--------------------------------------------------------------------------------------*/

DWORD SDI_Write(DWORD context, VOID *pBuffer, DWORD size)
{
	PDEBUG1("%s() called\n",__FUNCTION__);

    return SUCCESS;
}

/*======================================================================================*/
/*----------------------------  Windows interface END  ---------------------------------*/
/*======================================================================================*/

static int sdioDrv_InitDma (void)
{
    int rc = 0;

    DmaConfigInfo_t dma_tx_settings = 
    {     
        DMA_CSDP_DATATYPE_S32,      /* element width               */   
        0,                          /* source element index        */
        0,                          /* source frame index          */
        DMA_CCR_SRC_AMODE_POST_INC, /* source addressing mode      */
        0,                          /* destination element index   */
        0,                          /* destination frame index     */
        DMA_CCR_DST_AMODE_CONST,    /* destination addressing mode */
        DMA_PRIORITY,               /* dma priority level          */
        DMA_SYNCH_TRIGGER_DST,      /* synch mode                  */
        DMA_SYNCH_FRAME,            /* synch mode                  */
        0                           /* dma interrupt mask          */
    };
    DmaConfigInfo_t dma_rx_settings =
    {
        DMA_CSDP_DATATYPE_S32,      /* element width               */ 
        0,                          /* source element index        */ 
        0,                          /* source frame index          */ 
        DMA_CCR_SRC_AMODE_CONST,    /* source addressing mode      */ 
        0,                          /* destination element index   */ 
        0,                          /* destination frame index     */ 
        DMA_CCR_DST_AMODE_POST_INC, /* destination addressing mode */ 
        DMA_PRIORITY,               /* dma priority level          */ 
        DMA_SYNCH_TRIGGER_SRC,      /* synch mode                  */ 
        DMA_SYNCH_FRAME,            /* synch mode                  */ 
        0                           /* dma interrupt mask          */
    };

	if ( g_drv.dma_tx_channel == NULL ) {
    g_drv.dma_tx_channel = DmaAllocateChannel(DMA_TYPE_SYSTEM);
	}
	if ( g_drv.dma_rx_channel == NULL ) {
    g_drv.dma_rx_channel = DmaAllocateChannel(DMA_TYPE_SYSTEM);
	}
    if (g_drv.dma_tx_channel == NULL || g_drv.dma_rx_channel == NULL)
    {
      PERR2("sdioDrv_InitDma() DmaAllocateChannel FAILED !! (tx_channel=0x%x, rx_channel=0x%x)\n", g_drv.dma_tx_channel, g_drv.dma_rx_channel);
      return ERROR_1;
    }
    DmaConfigure(g_drv.dma_tx_channel, &dma_tx_settings, SDMA_REQ_MMC2_TX, &g_drv.dma_tx_info);
    DmaConfigure(g_drv.dma_rx_channel, &dma_rx_settings, SDMA_REQ_MMC2_RX, &g_drv.dma_rx_info);

    DmaSetSrcBuffer(&g_drv.dma_tx_info, g_drv.dma_buffer, g_drv.dma_buffer_phys.LowPart);
    DmaSetDstBuffer(&g_drv.dma_tx_info, (UINT8 *)(g_drv.regs_base+OMAP_HSMMC_DATA), OMAP_MMCHS2_REGS_PA+OMAP_HSMMC_DATA);

    DmaSetSrcBuffer(&g_drv.dma_rx_info,(UINT8 *)(g_drv.regs_base+OMAP_HSMMC_DATA),OMAP_MMCHS2_REGS_PA+OMAP_HSMMC_DATA);
    DmaSetDstBuffer(&g_drv.dma_rx_info, g_drv.dma_buffer, g_drv.dma_buffer_phys.LowPart);

    g_drv.dma_rx_regs = (OMAP_DMA_LC_REGS*)DmaGetLogicalChannel(g_drv.dma_rx_channel);
    return rc;
  
} /* sdioDrv_InitDma() */

/*--------------------------------------------------------------------------------------*/

static void sdioDrv_ShutdownDma (void)
{
	if (g_drv.dma_tx_channel)
	{
		DmaFreeChannel(g_drv.dma_tx_channel);
		g_drv.dma_tx_channel = NULL;
	}
	if (g_drv.dma_rx_channel)
	{
		DmaFreeChannel(g_drv.dma_rx_channel);
		g_drv.dma_rx_channel = NULL;
	}

} /* sdioDrv_ShutdownDma() */

/*--------------------------------------------------------------------------------------*/
/*================================= End of DMA stuff ===================================*/
/*--------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------*/

static BOOL sdioDrv_SetPowerState (CEDEVICE_POWER_STATE ePowerState) 
{
    BOOL bRet = TRUE;

    switch (ePowerState) 
    {
      case D0:
      case D1:
      case D2:
            bRet = SetDevicePowerState (g_drv.hParentBus, ePowerState, NULL);
            if (bRet == FALSE)
            {
                PERR("sdioDrv_SetPowerState() SetDevicePowerState FAILED !!!\n");
            }        
            break;      
	  case D3:
	  case D4:
			bRet = SetDevicePowerState (g_drv.hParentBus, ePowerState, NULL);
            if (bRet == FALSE)
            {
				RETAILMSG(1,(TEXT("*********** sdioDrv_SetPowerState() SetDevicePowerState FAILED !!! **********\n\n")));
            }        
            break;      
    }
    return bRet;   
} /* sdioDrv_SetPowerState() */

/*--------------------------------------------------------------------------------------*/

static int sdioDrv_InterruptDone (void)
{
    int    status, loop_count=0;

	
    status = OMAP_HSMMC_READ(g_drv.regs_base, STAT); 
    OMAP_HSMMC_WRITE(g_drv.regs_base, ISE, 0);
    InterruptDone(g_drv.mmc_intr);    

    if (g_drv.async_direction == AsyncRead)   /* Async read */
    {
        while (g_drv.dma_rx_regs->CCR  & DMA_CCR_RD_ACTIVE)
        {
            if (loop_count ++ > 10000)
            {
                PERR("sdioDrv_InterruptDone() read channel active  !!!\n");
            }
        }
    }
    return status;

} /* sdiodrv_InterruptDone() */

/*--------------------------------------------------------------------------------------*/

static WINAPI sdiodrv_IST(LPVOID ptr)
{
    int iStatus;
    
    while (TRUE)
    {
        if (WaitForSingleObject (g_drv.ist_event, INFINITE) != WAIT_OBJECT_0) 
        {
            PERR("sdiodrv_IST() WaitForSingleObject FAILED  !!!\n");
            break;
        }
	    PDEBUG("sdiodrv_IST()\n");
        if (!g_drv.bRunDrv)
        {
            break;
        }
        iStatus = sdioDrv_InterruptDone ();
        if (g_drv.BusTxnCB != NULL)
        {
          if (iStatus & OMAP_HSMMC_ERR)
          {
            PERR1("sdiodrv_IST() sdioDrv_InterruptDone STAT=0x%x\n", iStatus);
          }
          g_drv.BusTxnCB (g_drv.BusTxnHandle, (iStatus & OMAP_HSMMC_ERR));
        }
    }

    return SUCCESS;
}

/*--------------------------------------------------------------------------------------*/

static unsigned long sdioDrv_PollStatus (unsigned long uRegOffset, 
                                         unsigned long uStatusMask)
{
	unsigned long uStatus  = 0, loops=0;

	do
    {
	    uStatus = OMAP_HSMMC_READ_OFFSET(g_drv.regs_base, uRegOffset);
		
	    if (uStatus & uStatusMask)
		{
            break;
		}
	} while (loops++ < SDIODRV_MAX_LOOPS);

   /* Read again to use the last updated status */
   uStatus = OMAP_HSMMC_READ_OFFSET(g_drv.regs_base, uRegOffset);

   return uStatus;
}

/*--------------------------------------------------------------------------------------*/
/*
 *  Disable clock to the card
 */
static void sdioDrv_StopClock (void)
{
	OMAP_HSMMC_WRITE(g_drv.regs_base, SYSCTL, OMAP_HSMMC_READ(g_drv.regs_base, SYSCTL) & ~CEN);
	if ((OMAP_HSMMC_READ(g_drv.regs_base, SYSCTL) & CEN) != 0x0)
    {
		PERR("MMC clock not stoped, clock freq can not be altered\n");
    }
}

/*--------------------------------------------------------------------------------------*/
static void sdioDrv_SetClock (unsigned int clock)
{
	unsigned short dsor = 0;
	unsigned long  regVal;
	int            status;

	PDEBUG1("sdioDrv_SetClock(%d)\n",clock);
	if (clock) 
    {
		/* Enable MMC_SD_CLK */
		dsor = OMAP_MMC_MASTER_CLOCK / clock;
		if (dsor < 1)
        {
            dsor = 1;
        }
		if ((unsigned int)(OMAP_MMC_MASTER_CLOCK / dsor) > clock)
        {
			dsor++;
        }
		if (dsor > 250)
        {
			dsor = 250;
        }
	}
	sdioDrv_StopClock ();
	regVal = OMAP_HSMMC_READ(g_drv.regs_base, SYSCTL);
	regVal = regVal & ~(CLKD_MASK);
	regVal = regVal | (dsor << 6);
	regVal = regVal | (DTO << 16);
	OMAP_HSMMC_WRITE(g_drv.regs_base, SYSCTL, regVal);
	OMAP_HSMMC_WRITE(g_drv.regs_base, SYSCTL, OMAP_HSMMC_READ(g_drv.regs_base, SYSCTL) | ICE);

    /* wait till the the clock is stable (ICS) bit is set */
	status  = sdioDrv_PollStatus (OMAP_HSMMC_SYSCTL, ICS);
	if(!(status & ICS)) 
    {
	    PERR1("sdioDrv_SetClock() clock not stable!! status=0x%x\n", status);
		
	}

	/* Enable clock to the card */
	OMAP_HSMMC_WRITE(g_drv.regs_base, SYSCTL, OMAP_HSMMC_READ(g_drv.regs_base, SYSCTL) | CEN);
}

/*--------------------------------------------------------------------------------------*/

/* Reset the SD system */
static int sdioDrv_ResetMmc (void)
{
    int status;
	unsigned long timeout;

   	OMAP_HSMMC_WRITE(g_drv.regs_base, SYSCTL, OMAP_HSMMC_READ(g_drv.regs_base, SYSCTL) | SRA);
	
	timeout = GetTickCount() + MMC_TIMEOUT_MS;
	while ((status = OMAP_HSMMC_READ(g_drv.regs_base, SYSCTL) &  SRA) && (GetTickCount() < timeout));
	if (status & SRA)
	{
	    PERR1("sdioDrv_ResetMmc() MMC reset FAILED!! status=0x%x\n", status);
		
	}

	return status;
}

/*--------------------------------------------------------------------------------------*/

static int sdioDrv_InitHw (void)
{
    int rc=0;
    BOOL bRet;

	/* Calling DMA_init is moved here from SDI_init to fix DMA problem while resuming after suspend*/
	/* DMA init is called here everytime, connectbus is called. DMA resources are free'd inside disconnectbus */
    
	rc = sdioDrv_InitDma ();
	if (rc != 0)
	{
	    PERR("sdiodrv_init() - sdioDrv_InitDma FAILED!!\n");
    }

    g_drv.powerState = D0;
    bRet =  sdioDrv_SetPowerState (D0);
    
    if (bRet == FALSE)
    {
        return ERROR_1;
    }

	sdioDrv_ResetMmc ();

	/* 1.8V */
	OMAP_HSMMC_WRITE(g_drv.regs_base, CAPA,      OMAP_HSMMC_READ(g_drv.regs_base,CAPA) | VS18);
	OMAP_HSMMC_WRITE(g_drv.regs_base, HCTL,      OMAP_HSMMC_READ(g_drv.regs_base,HCTL) | SDVS18);
	/* clock gating */
	OMAP_HSMMC_WRITE(g_drv.regs_base, SYSCONFIG, OMAP_HSMMC_READ(g_drv.regs_base,SYSCONFIG) | AUTOIDLE);
    OMAP_HSMMC_WRITE(g_drv.regs_base, SYSCONFIG, 0x308);
	/* bus power */
	OMAP_HSMMC_WRITE(g_drv.regs_base, HCTL,      OMAP_HSMMC_READ(g_drv.regs_base,HCTL) | SDBP);
	/* interrupts */
	OMAP_HSMMC_WRITE(g_drv.regs_base, ISE,       0);
	OMAP_HSMMC_WRITE(g_drv.regs_base, IE,        IE_EN_MASK);

	g_drv.Clock_Enable=1;

#ifdef SDIO_1_BIT /* see also in SdioAdapter.c */
    PERR("SDIO: setting 1 data lines\n");
    OMAP_HSMMC_WRITE(g_drv.regs_base, HCTL, (OMAP_HSMMC_READ(g_drv.regs_base,HCTL) & (ONE_BIT)));
#else
    OMAP_HSMMC_WRITE(g_drv.regs_base, HCTL, OMAP_HSMMC_READ(g_drv.regs_base,HCTL) | (1 << 1));
#endif


#ifdef FPGA1273_STAGE_
	printf("SDIO: setting 6Mhz clock\n");
	sdioDrv_SetClock (6000000);
#else
    sdioDrv_SetClock (24000000);
#endif


    /* send the init sequence */
  OMAP_HSMMC_WRITE(g_drv.regs_base, CON, OMAP_HSMMC_READ(g_drv.regs_base,CON) | INIT_STREAM);
  OMAP_HSMMC_SEND_COMMAND(g_drv.regs_base, 0, 0);
    sdioDrv_PollStatus (OMAP_HSMMC_STAT, CC);
  OMAP_HSMMC_WRITE(g_drv.regs_base, CON, OMAP_HSMMC_READ(g_drv.regs_base,CON) & ~INIT_STREAM);

    return rc;
}

/* sdioDrv_SetDevicePowerState () - This function is called when wifi is set to D3 state.
 * This function calls SetDevicePowerState (with D3) defined in sdioDrv_SetPowerState.
 */
static int sdioDrv_SetDevicePowerState (void)
{
    BOOL bRet;

	sdioDrv_ShutdownDma();
	/*force idle*/
	OMAP_HSMMC_WRITE(g_drv.regs_base, SYSCONFIG, ((OMAP_HSMMC_READ(g_drv.regs_base,SYSCONFIG) & FORCEIDLE)));
	/*autoidle*/
	OMAP_HSMMC_WRITE(g_drv.regs_base, SYSCONFIG, ((OMAP_HSMMC_READ(g_drv.regs_base,SYSCONFIG) | AUTOIDLE)));
	
	/*smartidle*/
	/*OMAP_HSMMC_WRITE(g_drv.regs_base, SYSCONFIG, ((OMAP_HSMMC_READ(g_drv.regs_base,SYSCONFIG) | (SIDLE_MODE) | (AUTOIDLE))));*/

	bRet =  sdioDrv_SetPowerState (D3);
    
    if (bRet == FALSE)
    {
        return ERROR_1;
    }
	return SUCCESS;
}

int sdioDrv_HwCheck()
{
    return sdioDrv_InitHw ();
}


/*--------------------------------------------------------------------------------------*/

int sdioDrv_ConnectBus (void *        fCbFunc, 
                        void *        hCbArg, 
                        unsigned int  uBlkSizeShift, 
                        unsigned int  uSdioThreadPriority, 
                        unsigned char **pDmaBufAddr,
                        unsigned int  *pDmaBufLen)
{
	int rc;
    
    g_drv.BusTxnCB      = fCbFunc;
    g_drv.BusTxnHandle  = hCbArg;
    g_drv.uBlkSizeShift = uBlkSizeShift;  
    g_drv.uBlkSize      = 1 << uBlkSizeShift;
    CeSetThreadPriority (g_drv.IST, uSdioThreadPriority);
    /* Provide the DMA buffer address to the upper layer so it will use it as the transactions host buffer. */
    rc = sdioDrv_InitHw ();
    if (pDmaBufAddr) 
    {
        *pDmaBufAddr = g_drv.dma_buffer;
        *pDmaBufLen  = TIWLAN_MMC_MAX_DMA;
    }
    return rc;
}

/*--------------------------------------------------------------------------------------*/

int sdioDrv_DisconnectBus (void)
{
	if (g_drv.Clock_Enable)
		return sdioDrv_SetDevicePowerState();
	else
		return SUCCESS;
}

/*--------------------------------------------------------------------------------------*/

static int sdioDrv_SendCmd (unsigned int uCmdReg, unsigned int uArg)
{
	
    OMAP_HSMMC_WRITE(g_drv.regs_base, STAT, OMAP_HSMMC_STAT_CLEAR);
	OMAP_HSMMC_SEND_COMMAND(g_drv.regs_base, uCmdReg, uArg);
	return sdioDrv_PollStatus (OMAP_HSMMC_STAT, CC);
	
}

/*--------------------------------------------------------------------------------------*/

int sdioDrv_ExecuteCmd (unsigned int uCmd, 
                        unsigned int uArg, 
                        unsigned int uRespType, 
                        void *       pResponse, 
                        unsigned int uLen)
{
	unsigned int uCmdReg   = 0;
	unsigned int uStatus   = 0;
	unsigned int uResponse = 0;
	
	
	PDEBUG2("sdioDrv_ExecuteCmd() starting cmd %02x arg %08x\n", (int)uCmd, (int)uArg);

	uCmdReg = (uCmd << 24) | (uRespType << 16) ;
	
	uStatus = sdioDrv_SendCmd (uCmdReg, uArg);

	if (!(uStatus & CC)) 
    {
	    PERR1("sdioDrv_ExecuteCmd() SDIO Command error status = 0x%x\n", uStatus);
		return ERROR_1;
	}
	if ((uLen > 0) && (uLen <= 4))
	{
	    uResponse = OMAP_HSMMC_READ(g_drv.regs_base, RSP10);
		memcpy (pResponse, (char *)&uResponse, uLen);
		PDEBUG1("sdioDrv_ExecuteCmd() response = 0x%x\n", uResponse);
		
	}
	
    return SUCCESS;
}

/*--------------------------------------------------------------------------------------*/

static int sdioDrv_SendDataXferCmd (unsigned int uCmd, 
                                    unsigned int uArg, 
                                    unsigned int uLen, 
                                    unsigned int uStatusMask,
                                    unsigned int bBlkMode)
{
    int status;

	if (bBlkMode)
    {
        OMAP_HSMMC_WRITE(g_drv.regs_base, BLK, (uLen << 16) | (g_drv.uBlkSize << 0));
        uCmd |= MSBS | BCE ;
    }
    else
    {
        OMAP_HSMMC_WRITE(g_drv.regs_base, BLK, uLen);
    }
	
	status = sdioDrv_SendCmd (uCmd, uArg);
	if (!(status & CC)) 
    {
	    PERR1("sdioDrv_SendDataXferCmd() SDIO Command error! STAT = 0x%x\n", status);
		return SUCCESS;
	}
	if(uStatusMask)
	{
		status = sdioDrv_PollStatus (OMAP_HSMMC_PSTATE, uStatusMask);
	}

	return status;
} 

/*--------------------------------------------------------------------------------------*/

static int sdioDrv_DataXferSync (unsigned int  uCmd, 
                                 unsigned int  uArg, 
                                 unsigned char *pData, 
                                 unsigned int  uLen, 
                                 unsigned int  uStatusMask)
{
	int iStatus;
	unsigned int   uBufEnd = (unsigned int)pData + (unsigned int)uLen;
    unsigned long  uData32;
	unsigned short *puData16;

	iStatus = sdioDrv_SendDataXferCmd (uCmd, uArg, uLen, uStatusMask, 0);
	if (!(iStatus & uStatusMask)) 
    {
	    return ERROR_1;
	}

   
	if (uStatusMask == BRE)  
    {
        if (((unsigned int)pData & 3) == 0)
        {
            /* Host address is 4 bytes aligned */
            for ( ; (unsigned int)pData < uBufEnd ; pData += sizeof(unsigned long))
            {
            	*((unsigned long *)pData) = OMAP_HSMMC_READ (g_drv.regs_base, DATA);
            }
        }
        else
        {
            /* Host address is 2 bytes aligned */
            for ( ; (unsigned int)pData < uBufEnd ; pData += sizeof(unsigned long))
            {
            	uData32 = OMAP_HSMMC_READ (g_drv.regs_base, DATA);
                *((unsigned short *)pData)       = (unsigned short)uData32;
                *((unsigned short *)(pData + 2)) = (unsigned short)(uData32 >> 16);

            }
        }
		
		iStatus  = sdioDrv_PollStatus (OMAP_HSMMC_STAT, TC);
		if (!(iStatus & TC)) 
		{
			PERR1("sdioDrv_DataXferSync() transfer error! STAT = 0x%x\n", iStatus);
            return -2;
		}
    }
    else /* Write operation */
    {
        if (((unsigned int)pData & 3) == 0)
        {
            /* Host address is 4 bytes aligned */
            for ( ; (unsigned int)pData < uBufEnd ; pData += sizeof(unsigned long))
            {
            	uData32 = *((unsigned long *)pData);
                /* Write next word (32 bit) */
                OMAP_HSMMC_WRITE (g_drv.regs_base, DATA, uData32);
            }
        }
        else
        {
            /* Host address is 2 bytes aligned */
            for ( ; (unsigned int)pData < uBufEnd ; pData += sizeof(unsigned long))
            {
                puData16 = (unsigned short *)&uData32;
            	*puData16 = *((unsigned short *)pData);
                puData16++;
                *puData16 = *((unsigned short *)pData + 2);

                /* Write next word (32 bit) */
                OMAP_HSMMC_WRITE (g_drv.regs_base, DATA, uData32);
            }
        }
		iStatus  = sdioDrv_PollStatus (OMAP_HSMMC_STAT, TC);
		if (!(iStatus & TC)) 
		{
			PERR1("sdioDrv_DataXferSync() transfer error! STAT = 0x%x\n", iStatus);
		return ERROR_2;
		}
    }

    return SUCCESS;
}

/*--------------------------------------------------------------------------------------*/

int sdioDrv_ReadSync (unsigned int uFunc, 
                      unsigned int uHwAddr, 
                      void *       pData, 
                      unsigned int uLen,
                      unsigned int bFixedAddr,
                      unsigned int bMore)
{
	unsigned int uCmdArg;
	int          iStatus;


	uCmdArg = SDIO_CMD53_READ(0, uFunc, 0, bFixedAddr, uHwAddr, uLen);

	iStatus = sdioDrv_DataXferSync (OMAP_HSMMC_CMD53_READ, uCmdArg, pData, uLen, BRE);
	if (iStatus != 0)
	{
		RETAILMSG(1,(TEXT("*********** sdioDrv_ReadSync : sdioDrv_DataXferSync :: FAILED **********\n\n")));
        PERR("sdioDrv_ReadSync() FAILED!!\n");
	}
	return iStatus;
}

/*--------------------------------------------------------------------------------------*/

int sdioDrv_ReadAsync (unsigned int uFunc, 
                       unsigned int uHwAddr, 
                       void *       pData, 
                       unsigned int uLen, 
                       unsigned int bBlkMode,
                       unsigned int bFixedAddr,
                       unsigned int bMore)
{
	unsigned int uCmdArg;
    unsigned int uNumBlks;
    unsigned int uDmaBlockCount;
    unsigned int uNumOfElem;
    unsigned int uDmaBufOffset;
    
    if (bBlkMode)
    {
        /* For block mode use number of blocks instead of length in bytes */
        uNumBlks = uLen >> g_drv.uBlkSizeShift;
        uDmaBlockCount = uNumBlks;
        /* due to the DMA config to 32Bit per element (OMAP_DMA_DATA_TYPE_S32) the division is by 4 */ 
        uNumOfElem = g_drv.uBlkSize >> 2;
    }
    else
    {	
        uNumBlks = uLen;
        uDmaBlockCount = 1;
        uNumOfElem = (uLen + 3) >> 2;
    }
    
    uCmdArg = SDIO_CMD53_READ(0, uFunc, bBlkMode, bFixedAddr, uHwAddr, uNumBlks);

    sdioDrv_SendDataXferCmd (OMAP_HSMMC_CMD53_READ_DMA, uCmdArg, uNumBlks, 0, bBlkMode);
    PDEBUG1("sdioDrv_ReadAsync() dma_ch=%d \n", 77/*SDMA_REQ_MMC2_RX*/);
    
    OMAP_HSMMC_WRITE(g_drv.regs_base, ISE, TC);
    g_drv.async_direction = AsyncRead;

    /* 
     * Update the DMA source address to the host buffer address.
     * Note: The upper layer uses the DMA buffer!!
     *       See also in sdioDrv_ConnectBus where this buffer address is provided upwards.
     */
    uDmaBufOffset = (unsigned int)pData - (unsigned int)g_drv.dma_buffer;
    DmaSetDstBuffer(&g_drv.dma_rx_info, pData, g_drv.dma_buffer_phys.LowPart + uDmaBufOffset);

    DmaSetElementAndFrameCount (&g_drv.dma_rx_info, uNumOfElem, uDmaBlockCount);
    DmaStart (&g_drv.dma_rx_info);
    
    /* Continued at sdiodrv_IST() after DMA transfer is finished */
    
    return SUCCESS;
}

/*--------------------------------------------------------------------------------------*/

int sdioDrv_WriteSync (unsigned int uFunc, 
                       unsigned int uHwAddr, 
                       void *       pData, 
                       unsigned int uLen,
                       unsigned int bFixedAddr,
                       unsigned int bMore)
{
	unsigned int uCmdArg;
	int          iStatus;

	uCmdArg = SDIO_CMD53_WRITE(1, uFunc, 0, bFixedAddr, uHwAddr, uLen);

	iStatus = sdioDrv_DataXferSync (OMAP_HSMMC_CMD53_WRITE, uCmdArg, pData, uLen, BWE);
	if (iStatus != 0)
	{
		RETAILMSG(1,(TEXT("*********** sdioDrv_WriteSync : sdioDrv_DataXferSync :: FAILED **********\n\n")));
	}
	return iStatus;
}

/*--------------------------------------------------------------------------------------*/

int sdioDrv_WriteAsync (unsigned int uFunc, 
                        unsigned int uHwAddr, 
                        void *       pData, 
                        unsigned int uLen, 
                        unsigned int bBlkMode,
                        unsigned int bFixedAddr,
                        unsigned int bMore)
{
	unsigned int uCmdArg;
    unsigned int uNumBlks;
    unsigned int uDmaBlockCount;
    unsigned int uNumOfElem;
    unsigned int uDmaBufOffset;
    
    if (bBlkMode)
    {
        /* For block mode use number of blocks instead of length in bytes */
        uNumBlks = uLen >> g_drv.uBlkSizeShift;
        uDmaBlockCount = uNumBlks;
        /* due to the DMA config to 32Bit per element (OMAP_DMA_DATA_TYPE_S32) the division is by 4 */ 
        uNumOfElem = g_drv.uBlkSize >> 2;
    }
    else
    {	
        uNumBlks = uLen;
        uDmaBlockCount = 1;
        uNumOfElem = (uLen + 3) >> 2;
    }

    /* 
     * Update the DMA source address to the host buffer address.
     * Note: The upper layer has already copied the data to the DMA buffer!!
     *       See also in sdioDrv_ConnectBus where this buffer address is provided upwards.
     */
    uDmaBufOffset = (unsigned int)pData - (unsigned int)g_drv.dma_buffer;
    DmaSetSrcBuffer(&g_drv.dma_tx_info, pData, g_drv.dma_buffer_phys.LowPart + uDmaBufOffset);

    uCmdArg = SDIO_CMD53_WRITE(1, uFunc, bBlkMode, bFixedAddr, uHwAddr, uNumBlks);

    sdioDrv_SendDataXferCmd (OMAP_HSMMC_CMD53_WRITE_DMA, uCmdArg, uNumBlks, 0, bBlkMode);
    OMAP_HSMMC_WRITE(g_drv.regs_base, ISE, TC);
    g_drv.async_direction = AsyncWrite;
    DmaSetElementAndFrameCount (&g_drv.dma_tx_info, uNumOfElem, uDmaBlockCount);
    DmaStart (&g_drv.dma_tx_info);
    
    /* Continued at sdiodrv_IST() after DMA transfer is finished */
    
    return SUCCESS;
}

/*--------------------------------------------------------------------------------------*/

int sdioDrv_ReadSyncBytes (unsigned int  uFunc, 
                           unsigned int  uHwAddr, 
                           unsigned char *pData, 
                           unsigned int  uLen, 
                           unsigned int  bMore)
{
	unsigned int uCmdArg;
	unsigned int i;
	int          iStatus;
	
    for (i = 0; i < uLen; i++) 
    {
        uCmdArg = SDIO_CMD52_READ(0, uFunc, 0, uHwAddr);

        iStatus = sdioDrv_SendCmd (OMAP_HSMMC_CMD52_READ, uCmdArg);

        if (!(iStatus & CC)) 
        {
			PERR1("sdioDrv_ReadSyncBytes() SDIO Command error status = 0x%x\n", iStatus);
            return ERROR_1;
        }
        else
        {
            *pData = (unsigned char)(OMAP_HSMMC_READ(g_drv.regs_base, RSP10));
        }

        pData++;
        uHwAddr++;
    }
	return SUCCESS;
}

/*--------------------------------------------------------------------------------------*/

int sdioDrv_WriteSyncBytes (unsigned int  uFunc, 
                            unsigned int  uHwAddr, 
                            unsigned char *pData, 
                            unsigned int  uLen, 
                            unsigned int  bMore)
{
	unsigned int uCmdArg;
	unsigned int i;
	int          iStatus;

	
    for (i = 0; i < uLen; i++) 
    {
        uCmdArg = SDIO_CMD52_WRITE(1, uFunc, 0, uHwAddr, *pData);

        iStatus = sdioDrv_SendCmd (OMAP_HSMMC_CMD52_WRITE, uCmdArg);

        if (!(iStatus & CC)) 
        {
	        PERR1("sdioDrv_WriteSyncBytes() SDIO Command error status = 0x%x\n", iStatus);
            return ERROR_1;
        }

        pData++;
        uHwAddr++;
    }
	return SUCCESS;
}

/*--------------------------------------------------------------------------------------*/
void sdiodrv_clkset()
{
	sdioDrv_SetPowerState (D0);
	OMAP_HSMMC_WRITE(g_drv.regs_base, SYSCONFIG, OMAP_HSMMC_READ(g_drv.regs_base,SYSCONFIG) | AUTOIDLE);
	OMAP_HSMMC_WRITE(g_drv.regs_base, SYSCONFIG, 0x308);
	g_drv.Clock_Enable = 1;

}

void sdiodrv_clkstop()
{
	OMAP_HSMMC_WRITE(g_drv.regs_base, SYSCONFIG, ((OMAP_HSMMC_READ(g_drv.regs_base,SYSCONFIG) & FORCEIDLE)));
	OMAP_HSMMC_WRITE(g_drv.regs_base, SYSCONFIG, ((OMAP_HSMMC_READ(g_drv.regs_base,SYSCONFIG) | AUTOIDLE)));
	sdioDrv_SetPowerState (D3);
	g_drv.Clock_Enable = 0;
}