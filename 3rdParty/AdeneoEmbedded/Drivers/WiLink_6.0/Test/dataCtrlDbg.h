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
#ifndef __DATA_CTRL_DBG_H__
#define __DATA_CTRL_DBG_H__


/* RX/TX debug functions */
typedef enum
{
/* Tx debug functions */
/*	0	*/	TX_RX_DBG_FUNCTIONS,
/*	1	*/	PRINT_TX_CTRL_INFO,
/*	2	*/	PRINT_TX_CTRL_COUNTERS,
/*	3	*/	PRINT_TX_DATA_QUEUE_INFO,
/*	4	*/	PRINT_TX_DATA_QUEUE_COUNTERS,
/*	5	*/	PRINT_TX_MGMT_QUEUE_INFO,
/*	6	*/	PRINT_TX_MGMT_QUEUE_COUNTERS,
/*	7	*/	PRINT_TX_CTRL_BLK_INFO,
/*	8	*/	PRINT_TX_HW_QUEUE_INFO,
/*	9	*/	PRINT_TX_XFER_INFO,
/*	10	*/	PRINT_TX_RESULT_INFO,
/*	11	*/	PRINT_TX_DATA_CLSFR_TABLE,
/*	20	*/	RESET_TX_CTRL_COUNTERS          = 20,
/*	21	*/	RESET_TX_DATA_QUEUE_COUNTERS,
/*	22	*/	RESET_TX_DATA_CLSFR_TABLE,
/*	23	*/	RESET_TX_MGMT_QUEUE_COUNTERS,
/*	24	*/	RESET_TX_RESULT_COUNTERS,
/*	25	*/	RESET_TX_XFER_COUNTERS,
    

/* Rx debug functions */
/*	50	*/	PRINT_RX_BLOCK                  = 50,
/*	51	*/	PRINT_RX_COUNTERS,
/*	52	*/	RESET_RX_COUNTERS,
/*	53	*/	PRINT_RX_THROUGHPUT_START,
/*	54	*/	PRINT_RX_THROUGHPUT_STOP

} ERxTxDbgFunc;


/* debg functions */
typedef enum
{
/*	0	*/	CTRL_PRINT_DBG_FUNCTIONS,
/*	1	*/	CTRL_PRINT_CTRL_BLOCK,
/*	2	*/	CTRL_PRINT_TX_PARAMETERS,
/*	3	*/	CTRL_SET_CTS_TO_SELF

} ECtrlDbgFunc;


void rxTxDebugFunction (TI_HANDLE hRxTxHandle, TI_UINT32 funcType, void *pParam);
void ctrlDebugFunction (TI_HANDLE hCtrlData, TI_UINT32 funcType, void *pParam);


#endif /* __DATA_CTRL_DBG_H__*/


