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

#ifndef __STD_SDIO_BUS_DRV_H
#define __STD_SDIO_BUS_DRV_H

#include "TxnDefs.h"


/* @ */
/* Bus Width 1 or 4 */
//#define TISDBUS_BUS_WIDTH_4


/********************************************************************/
/*	SDIO bus driver defines                                         */
/********************************************************************/

/* Card Common Control Registers (CCCR) */

#define CCCR_SDIO_REVISION                  0x00
#define CCCR_SD_SPECIFICATION_REVISION      0x01
#define CCCR_IO_ENABLE                      0x02
#define CCCR_IO_READY                       0x03
#define CCCR_INT_ENABLE                     0x04
#define CCCR_INT_PENDING                    0x05
#define CCCR_IO_ABORT                       0x06
#define CCCR_BUS_INTERFACE_CONTOROL         0x07
#define CCCR_CARD_CAPABILITY	              0x08
#define CCCR_COMMON_CIS_POINTER             0x09 /*0x09-0x0B*/
#define CCCR_BUS_SUSPEND	                  0x0c
#define CCCR_FUNCTION_SELECT	              0x0d
#define CCCR_EXEC_FLAGS	                    0x0e
#define CCCR_READY_FLAGS	                  0x0f
#define CCCR_FNO_BLOCK_SIZE	                0x10 /*0x10-0x11*/
#define FN0_FBR2_REG_108                    0x210

#define FN0_FBR2_REG_108_BIT_MASK           0xFFF

#define FN0_CCCR_REG_32	                    0x64 
#define VDD_VOLTAGE_WINDOW                  0xffffc0

#define SYNC_ASYNC_LENGTH_THRESH (360)
#define TI_SDIO_BUS_DRV_CARD_GET_FUNCTIONS (100)
#define TISDBUS_SEND_RELATIVE_ADDR_RETRIES 2
#define SDHC_NAME   L"SHC2:"

#define IOCTL_SHC_SET_BULTIN_CARD_STATE_INSERT  CTL_CODE(65432, 2345, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SHC_SET_BULTIN_CARD_STATE_REMOVE  CTL_CODE(65432, 2346, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define CARD_MODULE_WAITING_TIME_SECONDS (60*3) /* 3 minutes */
#define WAIT_RELEASE (100)

#define MAX_RETRIES   10

#define SDHC_SLOT_NUMBER 1

#define SDHC_CLOCK_RATE 24000000

/********************************************************************/
/*	SDIO driver functions prototypes                                */
/********************************************************************/
int stdSdioBusDrv_ConnectBus     (void *fCbFunc, 
                                  void *hCbArg, 
                                  unsigned int uBlkSizeShift);

int stdSdioBusDrv_DisconnectBus  (void);

ETxnStatus stdSdioBusDrv_Transact  (unsigned int uFunc, 
                                    unsigned int uHwAddr, 
                                    void *       pData, 
                                    unsigned int uLen, 
							                      unsigned int  bDirection,
                                    unsigned int bBlkMode,
                                    unsigned int bFixedAddr,
                                    unsigned int bMore);

ETxnStatus stdSdioBusDrv_TransactBytes  (unsigned int  uFunc, 
								                         unsigned int  uHwAddr, 
								                         void *		     pData, 
								                         unsigned int  uLen, 
								                         unsigned int  bDirection,
								                         unsigned int  bMore);

/************************************************************************
 * Types
 ************************************************************************/
typedef struct sdioBusDrv
{
  PSDCARD_HC_CONTEXT hcContext;

} sdioHCDrv_t;


typedef struct sdiodrv
{
    SD_BUS_REQUEST      BusRequest;
    void                (*BusTxnCB)(void* BusTxnHandle, int status);
    void                *BusTxnHandle;
    unsigned int        uBlkSizeShift;
    unsigned int        uBlkSize;
}  sdioBusDrv_t;

/*
 * An IO control "TI_SDIO_BUS_DRV_CARD_GET_FUNCTIONS" which recieves a list of direct functions
 * of loaded API
 */
typedef struct {
    // See sdioAdapt_ConnectBus
    int 	   (*connectBus)  (void *        fCbFunc,
                               void *        hCbArg,
                               unsigned int  uBlkSizeShift);
    // See sdioAdapt_DisconnectBus
    int        (*disconnectBus)(void);
    // See sdioAdapt_Transact
    ETxnStatus (*transact     )(unsigned int  uFuncId,
                                unsigned int  uHwAddr,
                                void *        pHostAddr,
                                unsigned int  uLength,
                                unsigned int  bDirection,
                                unsigned int  bBlkMode,
                                unsigned int  bFixedAddr,
                                unsigned int  bMore);
    // See sdioAdapt_TransactBytes
    ETxnStatus (*transactBytes)(unsigned int  uFuncId,
                                unsigned int  uHwAddr,
                                void *        pHostAddr,
                                unsigned int  uLength,
                                unsigned int  bDirection,
                                unsigned int  bMore);
} stdSdioApi_t;

/************************************************************************
 * Globals
 ************************************************************************/
extern sdioHCDrv_t     g_hc_context;
extern sdioBusDrv_t    g_sdio_bus_drv;

#endif /* __STD_SDIO_BUS_DRV_H */
