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

/** 
 * \file  osErr.h 
 * \brief Declare error codes returned by OS abstraction
 */

#ifndef __OS_ERR_H__
#define __OS_ERR_H__

typedef enum
{
    EOALERR_NO_ERR = 0,
    EOALERR_CU_WEXT_ERROR_CANT_ALLOCATE = -1,
    EOALERR_IPC_STA_ERROR_SENDING_WEXT = -2,
    EOALERR_IPC_EVENT_ERROR_EVENT_NOT_DEFINED = -3,
    EOALERR_IPC_EVENT_ERROR_EVENT_ALREADY_ENABLED = -4,
    EOALERR_IPC_EVENT_ERROR_EVENT_ALREADY_DISABLED = -5,
    EOALERR_IPC_WPA_ERROR_CANT_CONNECT_TO_SUPPL = -6,
    EOALERR_IPC_WPA_ERROR_CMD_TIMEOUT = -7,
    EOALERR_IPC_WPA_ERROR_CMD_FAILED = -8,
    EOALERR_MAX_ERROR = EOALERR_IPC_WPA_ERROR_CMD_FAILED
} EOALError;

typedef enum
{
	ECUERR_CU_ERROR = 								EOALERR_MAX_ERROR - 1,
	ECUERR_CU_CMD_ERROR_DEVICE_NOT_LOADED = 		EOALERR_MAX_ERROR - 2,
	ECUERR_WPA_CORE_ERROR_UNKNOWN_AUTH_MODE = 		EOALERR_MAX_ERROR - 3,
	ECUERR_WPA_CORE_ERROR_KEY_LEN_MUST_BE_SAME = 	EOALERR_MAX_ERROR - 4,
	ECUERR_WPA_CORE_ERROR_FAILED_CONNECT_SSID = 	EOALERR_MAX_ERROR - 5,
	ECUERR_WPA_CORE_ERROR_FAILED_DISCONNECT_SSID= 	EOALERR_MAX_ERROR - 6,
	ECUERR_CU_COMMON_ERROR = 						EOALERR_MAX_ERROR - 7,
	ECUERR_WPA_CORE_ERROR_IVALID_PIN = 				EOALERR_MAX_ERROR - 8,
	ECUERR_WPA_CORE_ERROR_CANT_ALLOC_PIN = 			EOALERR_MAX_ERROR - 9
} ECUError;


#endif /* __OS_ERR_H__ */

