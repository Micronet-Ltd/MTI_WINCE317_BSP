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
 
/** \file   SdioAdapter.c 
 *  \brief  The SDIO driver adapter. Platform dependent. 
 * 
 * An adaptation layer between the lower SDIO driver (in BSP) and the upper SdioBusDrv.
 * Used for issuing all SDIO transaction types towards the lower SDIO-driver.
 * Makes the decision whether to use Sync or Async transaction, and reflects it to the caller
 *     by the return value and calling its callback in case of Async.
 *  
 *  \see    SdioAdapter.h, SdioDrv.c & h
 */

#include <windows.h>
#include "sdiodrvdbg.h"
#include "TxnDefs.h"
#include "SdioAdapter.h"
#include "sdiodrv.h"
#include "host_platform.h"
#include "bmtrace_api.h"

int sdioDrv_HwCheck();
void sdiodrv_clkset();
void sdiodrv_clkstop();


#ifdef SDIO_1_BIT /* see also in SdioDrv.c */
#define SDIO_BITS_CODE   0x80 /* 1 bits */
#else
#define SDIO_BITS_CODE   0x82 /* 4 bits */
#endif

#define HW_FOUND 1
#define HW_NOT_FOUND 0

/************************************************************************
 * Defines
 ************************************************************************/
/* Sync/Async Threshold */
#ifdef FULL_ASYNC_MODE
#define SYNC_ASYNC_LENGTH_THRESH	0     /* Use Async for all transactions */
#else
#define SYNC_ASYNC_LENGTH_THRESH	360   /* Use Async for transactions longer than this threshold (in bytes) */
#endif

#define MAX_RETRIES                 10

/* For block mode configuration */
#define FN0_FBR2_REG_108                    0x210
#define FN0_FBR2_REG_108_BIT_MASK           0xFFF 


int g_ssd_debug_level=SSD_DEBUGLEVEL_ERR;
/* Perform check for hardware */
int Hw_Check()
{
   unsigned char  uByte;
   int iStatus; 
	
    
   if(hPlatform_DevicePowerOn(1))
   {
		RETAILMSG(1, (L" TURNING ON HARDWARE :: FAILED " ));
   }
	
   Sleep (150);

   iStatus = sdioDrv_HwCheck();
   iStatus = sdioDrv_ExecuteCmd (SD_IO_GO_IDLE_STATE, 0, MMC_RSP_NONE, &uByte, sizeof(uByte));
   if(iStatus){
	return HW_NOT_FOUND;
   }
    	    
   iStatus = sdioDrv_ExecuteCmd (SDIO_CMD5, 0, MMC_RSP_R4, &uByte, sizeof(uByte));
   if(iStatus == -1){
	return HW_NOT_FOUND;
   }
   return HW_FOUND;
}


int sdioAdapt_ConnectBus (void *        fCbFunc,
                          void *        hCbArg,
                          unsigned int  uBlkSizeShift,
                          unsigned int  uSdioThreadPriority,
                          unsigned char **pRxDmaBufAddr,
                          unsigned int  *pRxDmaBufLen,
                          unsigned char **pTxDmaBufAddr,
                          unsigned int  *pTxDmaBufLen)
{
	unsigned char  uByte;
    unsigned long  uLong;
    unsigned long  uCount = 0;
    unsigned int   uBlkSize = 1 << uBlkSizeShift;
	int            iStatus;
    unsigned char  *pDmaBufAddr;
    unsigned int   uDmaBufLen;


    if (uBlkSize < SYNC_ASYNC_LENGTH_THRESH) 
    {
        PERR2("%s(): Block-Size (%d) should be bigger than SYNC_ASYNC_LENGTH_THRESH!!\n", __FUNCTION__, uBlkSize);
    }

    /* Init SDIO driver and HW */
    iStatus = sdioDrv_ConnectBus (fCbFunc, hCbArg, uBlkSizeShift,uSdioThreadPriority, &pDmaBufAddr, &uDmaBufLen);
	if (iStatus) { return iStatus; }

    /* Provide the DMA buffer to the upper later (currently same buffer is used for both Tx and Rx) */
    *pRxDmaBufAddr = *pTxDmaBufAddr = pDmaBufAddr;
    *pRxDmaBufLen  = *pTxDmaBufLen  = uDmaBufLen;
  
    /* Send commands sequence: 0, 5, 3, 7 */
	iStatus = sdioDrv_ExecuteCmd (SD_IO_GO_IDLE_STATE, 0, MMC_RSP_NONE, &uByte, sizeof(uByte));
	if (iStatus) { return iStatus; }
	iStatus = sdioDrv_ExecuteCmd (SDIO_CMD5, VDD_VOLTAGE_WINDOW, MMC_RSP_R4, &uByte, sizeof(uByte));
	if (iStatus) { return iStatus; }
	iStatus = sdioDrv_ExecuteCmd (SD_IO_SEND_RELATIVE_ADDR, 0, MMC_RSP_R6, &uLong, sizeof(uLong));
	if (iStatus) { return iStatus; }
	iStatus = sdioDrv_ExecuteCmd (SD_IO_SELECT_CARD, uLong, MMC_RSP_R6, &uByte, sizeof(uByte));
	if (iStatus) { return iStatus; }

    /* NOTE:
     * =====
     * Each of the following loops is a workaround for a HW bug that will be solved in PG1.1 !!
     * Each write of CMD-52 to function-0 should use it as follows:
     * 1) Write the desired byte using CMD-52
     * 2) Read back the byte using CMD-52
     * 3) Write two dummy bytes to address 0xC8 using CMD-53
     * 4) If the byte read in step 2 is different than the written byte repeat the sequence
     */

    /* set device side bus width to 4 bit (for 1 bit write 0x80 instead of 0x82) */
    do
    {
        uByte = SDIO_BITS_CODE;
        iStatus = sdioDrv_WriteSyncBytes (TXN_FUNC_ID_CTRL, CCCR_BUS_INTERFACE_CONTOROL, &uByte, 1, 1);
        if (iStatus) { return iStatus; }

        iStatus = sdioDrv_ReadSyncBytes (TXN_FUNC_ID_CTRL, CCCR_BUS_INTERFACE_CONTOROL, &uByte, 1, 1);
        if (iStatus) { return iStatus; }
        
        iStatus = sdioDrv_WriteSync (TXN_FUNC_ID_CTRL, 0xC8, &uLong, 2, 1, 1);
        if (iStatus) { return iStatus; }

        uCount++;

    } while ((uByte != SDIO_BITS_CODE) && (uCount < MAX_RETRIES));


    uCount = 0;

    /* allow function 2 */
    do
    {
        uByte = 4;
        iStatus = sdioDrv_WriteSyncBytes (TXN_FUNC_ID_CTRL, CCCR_IO_ENABLE, &uByte, 1, 1);
        if (iStatus) { return iStatus; }

        iStatus = sdioDrv_ReadSyncBytes (TXN_FUNC_ID_CTRL, CCCR_IO_ENABLE, &uByte, 1, 1);
        if (iStatus) { return iStatus; }
        
        iStatus = sdioDrv_WriteSync (TXN_FUNC_ID_CTRL, 0xC8, &uLong, 2, 1, 1);
        if (iStatus) { return iStatus; }

        uCount++;

    } while ((uByte != 4) && (uCount < MAX_RETRIES));


#ifdef SDIO_IN_BAND_INTERRUPT

    uCount = 0;

    do
    {
        uByte = 3;
        iStatus = sdioDrv_WriteSyncBytes (TXN_FUNC_ID_CTRL, CCCR_INT_ENABLE, &uByte, 1, 1);
        if (iStatus) { return iStatus; }

        iStatus = sdioDrv_ReadSyncBytes (TXN_FUNC_ID_CTRL, CCCR_INT_ENABLE, &uByte, 1, 1);
        if (iStatus) { return iStatus; }
        
        iStatus = sdioDrv_WriteSync (TXN_FUNC_ID_CTRL, 0xC8, &uLong, 2, 1, 1);
        if (iStatus) { return iStatus; }

        uCount++;

    } while ((uByte != 3) && (uCount < MAX_RETRIES));


#endif

    uCount = 0;
    
    /* set block size for SDIO block mode */
    do
    {
        uLong = uBlkSize;
        iStatus = sdioDrv_WriteSync (TXN_FUNC_ID_CTRL, FN0_FBR2_REG_108, &uLong, 2, 1, 1);
        if (iStatus) { return iStatus; }

        iStatus = sdioDrv_ReadSync (TXN_FUNC_ID_CTRL, FN0_FBR2_REG_108, &uLong, 2, 1, 1);
        if (iStatus) { return iStatus; }
        
        iStatus = sdioDrv_WriteSync (TXN_FUNC_ID_CTRL, 0xC8, &uLong, 2, 1, 1);
        if (iStatus) { return iStatus; }

        uCount++;

    } while (((uLong & FN0_FBR2_REG_108_BIT_MASK) != uBlkSize) && (uCount < MAX_RETRIES));


    if (uCount >= MAX_RETRIES)
    {
        /* Failed to write CMD52_WRITE to function 0 */
        return (int)uCount;
    }

	return iStatus;
}


int sdioAdapt_DisconnectBus (void)
{
    return sdioDrv_DisconnectBus ();
}

ETxnStatus sdioAdapt_Transact (unsigned int  uFuncId,
                               unsigned int  uHwAddr,
                               void *        pHostAddr,
                               unsigned int  uLength,
                               unsigned int  bDirection,
                               unsigned int  bBlkMode,
                               unsigned int  bFixedAddr,
                               unsigned int  bMore)
{
    int iStatus;

    /* If transction length is below threshold, use Sync methods */
    if (uLength < SYNC_ASYNC_LENGTH_THRESH) 
    {
        /* Call read or write Sync method */
        if (bDirection) 
        {
            CL_TRACE_START_L2();
            iStatus = sdioDrv_ReadSync (uFuncId, uHwAddr, pHostAddr, uLength, bFixedAddr, bMore);
            CL_TRACE_END_L2("tiwlan_drv.ko", "INHERIT", "SDIO", ".ReadSync");
        }
        else 
        {
            CL_TRACE_START_L2();
            iStatus = sdioDrv_WriteSync (uFuncId, uHwAddr, pHostAddr, uLength, bFixedAddr, bMore);
            CL_TRACE_END_L2("tiwlan_drv.ko", "INHERIT", "SDIO", ".WriteSync");
        }

        /* If failed return ERROR, if succeeded return COMPLETE */
        if (iStatus) 
        {
            return TXN_STATUS_ERROR;
        }
        return TXN_STATUS_COMPLETE;
    }

    /* If transction length is above threshold, use Async methods */
    else 
    {
        /* Call read or write Async method */
        if (bDirection) 
        {
            CL_TRACE_START_L2();
            iStatus = sdioDrv_ReadAsync (uFuncId, uHwAddr, pHostAddr, uLength, bBlkMode, bFixedAddr, bMore);
            CL_TRACE_END_L2("tiwlan_drv.ko", "INHERIT", "SDIO", ".ReadAsync");
        }
        else 
        {
            CL_TRACE_START_L2();
            iStatus = sdioDrv_WriteAsync (uFuncId, uHwAddr, pHostAddr, uLength, bBlkMode, bFixedAddr, bMore);
            CL_TRACE_END_L2("tiwlan_drv.ko", "INHERIT", "SDIO", ".WriteAsync");
        }

        /* If failed return ERROR, if succeeded return PENDING */
        if (iStatus) 
        {
            return TXN_STATUS_ERROR;
        }
        return TXN_STATUS_PENDING;
    }
}
         
ETxnStatus sdioAdapt_TransactBytes (unsigned int  uFuncId,
                                    unsigned int  uHwAddr,
                                    void *        pHostAddr,
                                    unsigned int  uLength,
                                    unsigned int  bDirection,
                                    unsigned int  bMore)
{
    int iStatus;

	if(bMore == 1)
	{
		sdiodrv_clkset(); 
	}
	
    /* Call read or write bytes Sync method */
    if (bDirection) 
    {
        iStatus = sdioDrv_ReadSyncBytes (uFuncId, uHwAddr, pHostAddr, uLength, bMore);
    }
    else 
    {
        iStatus = sdioDrv_WriteSyncBytes (uFuncId, uHwAddr, pHostAddr, uLength, bMore);
    }

	if(bMore == 0)
	{
		sdiodrv_clkstop();
	}
    /* If failed return ERROR, if succeeded return COMPLETE */
    if (iStatus) 
    {
        return TXN_STATUS_ERROR;
    }
    return TXN_STATUS_COMPLETE;
}




