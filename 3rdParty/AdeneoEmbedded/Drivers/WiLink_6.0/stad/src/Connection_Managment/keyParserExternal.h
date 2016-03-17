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
/** \file keyParserExternal.h
 *  \brief WEP Key PArser Header file
 *
 *  \see keyParser.h
 */


/***************************************************************************/
/*																		   */
/*		MODULE:	keyParserExternal.h											   */
/*    PURPOSE:	External key parser											   */
/*																	 	   */
/***************************************************************************/

#ifndef _KEY_PARSER_EXTERNAL_H
#define _KEY_PARSER_EXTERNAL_H

#include "fsm.h"
#include "keyParser.h"

/* Constants */
#define MAX_EXT_KEY_DATA_LENGTH   32


#define EXT_KEY_TRANSMIT_MASK				0x80000000		/*< bit 31 of key index field */
#define EXT_KEY_PAIRWISE_GROUP_MASK			0x40000000		/*< bit 30 of key index field */
#define EXT_KEY_RSC_KEY_MASK				0x20000000		/*< bit 29 of key index field */
#define EXT_KEY_SUPP_AUTHENTICATOR_MASK		0x10000000		/*< bit 28 of key index field */
#define EXT_KEY_REMAIN_BITS_MASK			0x0FFFFF00		/*< remaining 20 bits in external key index */

#define WEP_KEY_LEN_40	 5  /* 40 bit (5 byte) key */
#define WEP_KEY_LEN_104	 13 /* 104 bit (13 byte) key */
#define WEP_KEY_LEN_232	 29 /* 232 bit (29 byte) key */

                                                      
/* Enumerations */                                    
                                                      
/* Typedefs */                                        
               



typedef struct
{
	TI_UINT32		Length;						    		/* The length of the whole structure */
	TI_UINT32		KeyIndex;								/* The key index to add or remove */
	TI_UINT8		macAddress[MAC_ADDR_LEN];	/**< Remote station MAC address*/
} keyParserExternal_keyRemoveDesc_t;

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

TI_STATUS keyParserExternal_config(struct _keyParser_t *pKeyParser);

TI_STATUS keyParserExternal_recv(struct _keyParser_t *pKeyParser,
						  TI_UINT8 *pPacket, TI_UINT32 packetLen);

TI_STATUS keyParserExternal_remove(struct _keyParser_t *pKeyParser, TI_UINT8 *pKeyData, TI_UINT32 keyDataLen);

#endif /*_KEY_PARSER_EXTERNAL_H*/

