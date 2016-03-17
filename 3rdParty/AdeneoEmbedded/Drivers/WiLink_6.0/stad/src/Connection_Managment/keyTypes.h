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
/** \file eapolParserMainInternal.h
 * \brief EAPOL frame parser internal definitions header file.
 *
 *
 * \see  eapolParseerMain.c,  eapolBuilderMain.h,  eapolBuilderMain.c, eapolParserMain.h
*/


/****************************************************************************
 *                                                                          *
 *   MODULE:                                                                *
 *   PURPOSE:                                                               *
 *   CREATOR: Alexander Sirotkin.                                           *
 *            Demiurg@ti.com                                                *
 *                                                                          *
 ****************************************************************************/

#ifndef  _KEY_TYPES_H
#define  _KEY_TYPES_H

#include "tidef.h"

/* Constatnts */


#define SESSION_KEY_LEN				16

/* Structures */

typedef struct
{
	TI_UINT32	keyId;                          /**< Decoded key Id */
	TI_UINT32	keyLen;							/**< Key length */
	char	*pData;							/**< Pointer to the material to derive the key from */
} encodedKeyMaterial_t;


#endif /*  _EAPOL_PARSER_INTERNAL_H*/
