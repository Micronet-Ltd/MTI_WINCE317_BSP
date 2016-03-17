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
/** \file keyParser.h
 *  \brief key parser API
 *
 *  \see keyParser.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  Key Parser	                                            *
 *   PURPOSE: key Parser module API                                    *
 *                                                                          *
 ****************************************************************************/

#ifndef _KEY_PARSER_H
#define _KEY_PARSER_H

#include "rsnApi.h"
#include "keyTypes.h"

/* Constants */
#define MAX_REPLAY_COUNTER_LEN		8

/* Enumerations */

/* Typedefs */

typedef struct _keyParser_t	keyParser_t;

/* RSN admission control prototypes */

typedef TI_STATUS (*keyParser_recv_t)(struct _keyParser_t *pKeyParser, TI_UINT8 *pKeyData, TI_UINT32 keyDataLen);
typedef TI_STATUS (*keyParser_config_t)(struct _keyParser_t *pKeyParser, TI_HANDLE hReport, TI_HANDLE hOs);
typedef TI_STATUS (*keyParser_replayReset_t)(struct _keyParser_t *pKeyParser);
typedef TI_STATUS (*keyParser_remove_t)(struct _keyParser_t *pKeyParser, TI_UINT8 *pKeyData, TI_UINT32 keyDataLen);

/* Structures */

struct _keyParser_t
{
	TI_UINT8					replayCounter[MAX_REPLAY_COUNTER_LEN];

	struct _mainKeys_t		*pParent;
	struct _unicastKey_t	*pUcastKey;
	struct _broadcastKey_t	*pBcastKey;

	TRsnPaeConfig         *pPaeConfig;

	TI_HANDLE 				hReport;
	TI_HANDLE 				hOs;
    TI_HANDLE               hCtrlData;

	keyParser_config_t		config;
	keyParser_recv_t		recv;
	keyParser_replayReset_t replayReset;
	keyParser_remove_t		remove;
};

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

keyParser_t* keyParser_create(TI_HANDLE hOs);

TI_STATUS keyParser_config(struct _keyParser_t *pKeyParser,
						TRsnPaeConfig *pPaeConfig,
                        struct _unicastKey_t *pUcastKey,
                        struct _broadcastKey_t *pBcastKey,
                        struct _mainKeys_t *pParent,
						TI_HANDLE hReport,
						TI_HANDLE hOs,
                        TI_HANDLE hCtrlData);

TI_STATUS keyParser_unload(struct _keyParser_t *pKeyParser);

TI_STATUS keyParser_nop(keyParser_t *pKeyParser);

#endif /*  _KEY_PARSER_H*/
