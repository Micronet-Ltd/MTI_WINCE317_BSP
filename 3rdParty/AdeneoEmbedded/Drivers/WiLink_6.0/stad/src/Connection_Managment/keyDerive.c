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
/** \file keyDeriveSM.c
 * \brief station unicast key SM implementation
 *
 * \see keyDeriveSM.h
*/

/****************************************************************************
 *                                                                          *
 *   MODULE:	station unicast key SM		                                *
 *   PURPOSE:   station unicast key SM implementation						*
 *                                                                          *
 ****************************************************************************/

#define __FILE_ID__  FILE_ID_28
#include "osApi.h"
#include "report.h"
#include "rsnApi.h"

#include "keyDerive.h"
#include "keyDeriveWep.h"
#include "keyDeriveTkip.h"
#include "keyDeriveAes.h"
#ifdef XCC_MODULE_INCLUDED
#include "keyDeriveCkip.h"
#endif

/**
*
* Function  - Init KEY Parser module.
*
* \b Description: 
*
* Called by RSN Manager. 
* Registers the function 'rsn_KeyDeriveRecv()' at the distributor to receive KEY frames upon receiving a KEY_RECV event.
*
* \b ARGS:
*
*  
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure. 
*
*/

keyDerive_t* keyDerive_create(TI_HANDLE hOs)
{
	keyDerive_t 		*pKeyDerive;

	/* allocate key parser context memory */
	pKeyDerive = (keyDerive_t*)os_memoryAlloc(hOs, sizeof(keyDerive_t));
	if (pKeyDerive == NULL)
	{
		return NULL;
	}

	os_memoryZero(hOs, pKeyDerive, sizeof(keyDerive_t));

	pKeyDerive->hOs = hOs;

	return pKeyDerive;
}

/**
*
* Function  - Init KEY Parser module.
*
* \b Description: 
*
* Called by RSN Manager. 
* Registers the function 'rsn_KeyDeriveRecv()' at the distributor to receive KEY frames upon receiving a KEY_RECV event.
*
* \b ARGS:
*
*  
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure. 
*
*/

TI_STATUS keyDerive_unload(struct _keyDerive_t *pKeyDerive)
{
	/* free key parser context memory */
	os_memoryFree(pKeyDerive->hOs, pKeyDerive, sizeof(keyDerive_t));

	return TI_OK;
}

/**
*
* Function  - Init KEY Parser module.
*
* \b Description: 
*
* Called by RSN Manager. 
* Registers the function 'rsn_KeyDeriveRecv()' at the distributor to receive KEY frames upon receiving a KEY_RECV event.
*
* \b ARGS:
*
*  
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure. 
*
*/

TI_STATUS keyDerive_config(struct _keyDerive_t *pKeyDerive,
						ECipherSuite cipher,
						struct _mainKeys_t *pMainKeys,
						TI_HANDLE hReport,
						TI_HANDLE hOs)
{

	TI_STATUS		status = TI_NOK;

	pKeyDerive->hReport = hReport;
	pKeyDerive->hOs = hOs;
	pKeyDerive->pMainKeys = pMainKeys;

    switch (cipher)
	{
    case TWD_CIPHER_NONE:
		status = keyDeriveNone_config(pKeyDerive);
        break;
	case TWD_CIPHER_WEP:
    case TWD_CIPHER_WEP104:
		status = keyDeriveWep_config(pKeyDerive);
		break;
	case TWD_CIPHER_TKIP:
		status = keyDeriveTkip_config(pKeyDerive);
		break;
#ifdef XCC_MODULE_INCLUDED
	case TWD_CIPHER_CKIP: 
  	status = keyDeriveCkip_config(pKeyDerive);
		break;
#endif

	case TWD_CIPHER_AES_CCMP:
		status = keyDeriveAes_config(pKeyDerive);
		break;
	default:
		return TI_NOK;
	}

	return status;
}



