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
/** \file keyParser.c
 * \brief KEY parser module implementation.
 *
 * \see keyParser.h
*/

/****************************************************************************
 *                                                                          *
 *   MODULE:	KEY parser                                                  *
 *   PURPOSE:   KEY parser implementation									*
 *                                                                          *
 ****************************************************************************/

#define __FILE_ID__  FILE_ID_33
#include "osApi.h"
#include "report.h"
#include "rsnApi.h"

#include "keyParser.h" 
#include "keyParserExternal.h"
#include "keyParserWep.h"

TI_STATUS keyParserNone_config(keyParser_t *pKeyParser);

/**
*
* Function  - Init KEY Parser module.
*
* \b Description: 
*
* Called by RSN Manager. 
* Registers the function 'rsn_keyParserRecv()' at the distributor to receive KEY frames upon receiving a KEY_RECV event.
*
* \b ARGS:
*
*  
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure. 
*
*/

keyParser_t* keyParser_create(TI_HANDLE hOs)
{
	keyParser_t 		*pKeyParser;

	/* allocate key parser context memory */
	pKeyParser = (keyParser_t*)os_memoryCAlloc(hOs, 1, sizeof(keyParser_t));
	if (pKeyParser == NULL)
	{
		return NULL;
	}

	pKeyParser->hOs = hOs;

	return pKeyParser;
}

/**
*
* Function  - Init KEY Parser module.
*
* \b Description: 
*
* Called by RSN Manager. 
* Registers the function 'rsn_keyParserRecv()' at the distributor to receive KEY frames upon receiving a KEY_RECV event.
*
* \b ARGS:
*
*  
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure. 
*
*/

TI_STATUS keyParser_unload(struct _keyParser_t *pKeyParser)
{
	/* free key parser context memory */
	os_memoryFree(pKeyParser->hOs, pKeyParser, sizeof(keyParser_t));

	return TI_OK;
}

/**
*
* Function  - Init KEY Parser module.
*
* \b Description: 
*
* Called by RSN Manager. 
* Registers the function 'rsn_keyParserRecv()' at the distributor to receive KEY frames upon receiving a KEY_RECV event.
*
* \b ARGS:
*
*  
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure. 
*
*/

TI_STATUS keyParser_config(struct _keyParser_t *pKeyParser,
						TRsnPaeConfig *pPaeConfig,
                        struct _unicastKey_t *pUcastKey,
                        struct _broadcastKey_t *pBcastKey,
                        struct _mainKeys_t *pParent,
						TI_HANDLE hReport,
						TI_HANDLE hOs,
                        TI_HANDLE hCtrlData)
{
    
    pKeyParser->pParent = pParent;
    pKeyParser->pUcastKey = pUcastKey;
    pKeyParser->pBcastKey = pBcastKey;
    pKeyParser->pPaeConfig = pPaeConfig;

    pKeyParser->hReport = hReport;
	pKeyParser->hOs = hOs;
    pKeyParser->hCtrlData = hCtrlData;

	keyParserExternal_config(pKeyParser);
	
	return TI_OK;
}



TI_STATUS keyParserNone_config(keyParser_t *pKeyParser)
{
	pKeyParser->recv = NULL;
	pKeyParser->replayReset = NULL;
	
	return TI_OK;
	
}


TI_STATUS keyParser_nop(keyParser_t *pKeyParser)
{
TRACE0(pKeyParser->hReport, REPORT_SEVERITY_INFORMATION, "KEY_PARSER: nop \n");
	
	return TI_OK;
}


