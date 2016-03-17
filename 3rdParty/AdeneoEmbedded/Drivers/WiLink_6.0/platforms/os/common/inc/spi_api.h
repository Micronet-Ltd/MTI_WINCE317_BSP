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

/*
 * inc/spi_api.h
 *
 * Header for SPI API test interface
 *
 */

#ifndef SPI_API_H
#define SPI_API_H


typedef void (*request_callback_t) (void *data, int status);


typedef enum 
{
    SPI_OK                =  0,
    SPI_TXN_COMPLETE      =  0,
    SPI_TXN_PENDING       =  1,
    SPI_ERR_UNKNOWN       = -1,
    SPI_ERR_BUS_BUSY      = -2,
    SPI_ERR_QUEUE_FULL    = -3,
    SPI_ERR_ALLOC_MEM     = -4,
    SPI_ERR_ASYNC_TIMEOUT = -5, 
    SPI_ERR_WRONG_LENGTH  = -6,

} SPI_Status_e; 


void* SPI_Open         (void* hOS);
int   SPI_Close        (void* hSPI);
int   SPI_Write        (void* hSPI, TI_UINT8 *data, TI_UINT32 length, request_callback_t fCb, void *pCb, int more);                     
int   SPI_Read         (void* hSPI, TI_UINT8 *data, TI_UINT32 length, request_callback_t fCb, void *pCb, int more);                      
int   SPI_WriteRead    (void* hSPI, TI_UINT8 *data, TI_UINT32 length, TI_UINT8 *rdata, TI_UINT32 rlength, request_callback_t fCb, void* pCb, int more);
int   SPI_WriteSync    (void* hSPI, TI_UINT8 *data, TI_UINT32 length, int more);
int   SPI_ReadSync     (void* hSPI, TI_UINT8 *data, TI_UINT32 length);


#endif /* SPI_API_H */

