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
 /** \file mlmeBuilder.h
 *  \brief 802.11 MLME Builder
 *
 *  \see mlmeBuilder.c
 */


/***************************************************************************/
/*																		   */
/*		MODULE:	mlmeBuilder.h											   */
/*    PURPOSE:	802.11 MLME Builder										   */
/*																	 	   */
/***************************************************************************/

#ifndef _MLME_BUILDER_H
#define _MLME_BUILDER_H

#include "802_11Defs.h"

#include "paramOut.h"

#include "mlmeSm.h"

/* Constants */

/* Enumerations */

/* state machine states */

/* Typedefs */

/* Structures */

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

TI_STATUS mlmeBuilder_sendFrame(TI_HANDLE pMlme, 
							 dot11MgmtSubType_e type, 
							 TI_UINT8 *pDataBuff, 
							 TI_UINT32 dataLen, 
							 TI_UINT8 setWepOpt);

TI_STATUS mlmeBuilder_buildFrameCtrl(mlme_t* pMlme, 
								  dot11MgmtSubType_e type, 
								  TI_UINT16* pFctrl, 
								  TI_UINT8 setWepOpt);

#endif

