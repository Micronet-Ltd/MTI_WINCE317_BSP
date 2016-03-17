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

/** \file  buf.c
 *  \brief WinMobile buf implementation.
 *
 *  \see   
 */

#include "tidef.h"
#include "osApi.h"
#include "RxBuf.h"
#include "report.h"
#include <ndis.h>
#include <stdio.h>


/*--------------------------------------------------------------------------------------*/
/* 
 * Allocate BUF Rx packets.
 * Add 16 bytes before the data buffer for WSPI overhead!
 */
void* RxBufAlloc (TI_HANDLE hOs, TI_UINT32 len, PacketClassTag_e ePacketClassTag)
{
    void* pBuf;
    TI_UINT32   alloc_len = len + WSPI_PAD_BYTES + PAYLOAD_ALIGN_PAD_BYTES;
    

    pBuf  = os_memoryAlloc(hOs, alloc_len);
    if (pBuf == NULL)
    {
		return NULL;
	}

    return pBuf;
}

/*--------------------------------------------------------------------------------------*/

void  RxBufFree(TI_HANDLE hOs, void* pBuf)		
{
    TI_UINT32 alloc_len;

    /* Get the original length as received from the FW in the Rx descriptor */
    alloc_len = (TI_UINT32)(*(TI_UINT16 *)pBuf);

    /* Multiply by 4 for bytes length, and add padding as added in RxBufAlloc() */
    alloc_len = (alloc_len << 2) + WSPI_PAD_BYTES + PAYLOAD_ALIGN_PAD_BYTES;

    /* align the buffer to lowest address due to Rx QOS payload alignment */ 
    (TI_UINT32)pBuf &= ~(TI_UINT32)0x3;

    os_memoryFree(hOs, (PVOID)pBuf, alloc_len);    
}




