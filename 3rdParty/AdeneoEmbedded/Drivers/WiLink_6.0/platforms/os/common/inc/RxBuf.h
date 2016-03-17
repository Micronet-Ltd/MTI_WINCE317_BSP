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

/***************************************************************************/
/*																		   */
/*	  MODULE:	buf.h												       */
/*    PURPOSE:	manages the allocation/free and field access of the BUF    */
/*																		   */
/***************************************************************************/
#ifndef _BUF_H_
#define _BUF_H_

#include "tidef.h"
#include "queue.h"
#include "public_descriptors.h"



#define WSPI_PAD_BYTES          	16     /* Add padding before data buffer for WSPI overhead */
#define PAYLOAD_ALIGN_PAD_BYTES   	4      /* Add an extra word for alignment the MAC payload in case of QoS MSDU */



/**
 * \brief Buffer for Tx/Rx packets 
 */ 
typedef void BUF, *PBUF;

/* Packet types */


/**
 * \def RX_BUF_DATA
 * \brief Macro which gets a pointer to BUF packet header and returns the pointer to the start address of the WLAN packet's data
 */
#define RX_BUF_DATA(pBuf)   ((void*)((TI_UINT8 *)pBuf + sizeof(RxIfDescriptor_t)))
/**
 * \def RX_BUF_LEN
 * \brief Macro which gets a pointer to BUF packet header and returns the buffer length (without Rx Descriptor) of the WLAN packet
 */
#define RX_BUF_LEN(pBuf)    ( (((RxIfDescriptor_t *)(pBuf))->length << 2) -  \
                              ((RxIfDescriptor_t *)(pBuf))->extraBytes -     \
                              sizeof(RxIfDescriptor_t) )

/**
 * \def RX_ETH_PKT_DATA
 * \brief Macro which gets a pointer to BUF packet header and returns the pointer to the start address of the ETH packet's data
 */
#define RX_ETH_PKT_DATA(pBuf)   *((void **)(((TI_UINT32)pBuf + sizeof(RxIfDescriptor_t) + 2) & ~3))
/**
 * \def RX_ETH_PKT_LEN
 * \brief Macro which gets a pointer to BUF packet header and returns the buffer length (without Rx Descriptor) of the ETH packet
 */
#define RX_ETH_PKT_LEN(pBuf)    *((TI_UINT32 *)(((TI_UINT32)pBuf + sizeof(RxIfDescriptor_t) + 6) & ~3))


/** \brief BUF Allocation
 * 
 * \param  hOs		- OS module object handle
 * \param  len		- Length of allocated WBUF
 * \param  ePacketClassTag	- The RX packet type (used only in EMP) 
 * \return On success: Pointer to WBUF	;	Otherwise: NULL
 * 
 * \par Description
 * This function allocates BUF element for Tx/Rx packet
 * 
 * \sa
 */ 
BUF* RxBufAlloc         (TI_HANDLE hOs, TI_UINT32 len, PacketClassTag_e ePacketClassTag);


/** \brief BUF Free
 * 
 * \param  hOs		- OS module object handle
 * \param  pWbuf	- Pointer to WBUF which was previously created by user
 * \return void
 * 
 * \par Description
 * This function frees the memory allocated for BUF element
 * 
 * \sa
 */ 
void  RxBufFree          (TI_HANDLE hOs, void* pBuf);


/** \brief BUF Free
 * 
 * \param  hOs		- OS module object handle
 * \param  pWbuf	- Pointer to WBUF which was previously created by user
 * \return void
 * 
 * \par Description
 * This function increment the start address of data held in BUF element in len.
 * 
 * \sa
 */ 
void  RxBufReserve       (TI_HANDLE hOs, void* pBuf, TI_UINT32 len); 

#endif

