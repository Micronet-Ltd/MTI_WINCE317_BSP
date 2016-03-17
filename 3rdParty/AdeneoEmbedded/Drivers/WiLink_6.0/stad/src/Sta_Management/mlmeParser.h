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

#ifndef _MLME_PARSER_H
#define _MLME_PARSER_H

#include "802_11Defs.h"
#include "paramOut.h"
#include "mlmeApi.h"
#include "mlmeSm.h"

/* Constants */

#define FRAME_CTRL_PROTOCOL_VERSION_MASK	0x03
#define FRAME_CTRL_TYPE_MASK				0x0C
#define FRAME_CTRL_SUB_TYPE_MASK			0xF0

/* Enumerations */

/* state machine states */

/* Typedefs */

/* Structures */

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

TI_STATUS mlmeParser_getFrameType(mlme_t *pMlme, 
							   TI_UINT16* pFrameCtrl, 
							   dot11MgmtSubType_e *pType);

TI_STATUS mlmeParser_readRates(mlme_t *pMlme, 
							TI_UINT8 *pData, 
							TI_UINT32 dataLen, 
							TI_UINT32 *pReadLen, 
							dot11_RATES_t *pRates);

TI_STATUS mlmeParser_readERP(mlme_t *pMlme,
                             TI_UINT8 *pData,
                             TI_UINT32 dataLen,
                             TI_UINT32 *pReadLen,
                             TI_BOOL *useProtection,
                             EPreamble *barkerPreambleMode);

TI_STATUS mlmeParser_readSsid(mlme_t *pMlme, 
						   TI_UINT8 *pData, 
						   TI_UINT32 dataLen, 
						   TI_UINT32 *pReadLen, 
						   dot11_SSID_t *pSsid);

TI_STATUS mlmeParser_readFhParams(mlme_t *pMlme, 
							   TI_UINT8 *pData, 
							   TI_UINT32 dataLen, 
							   TI_UINT32 *pReadLen, 
							   dot11_FH_PARAMS_t *pFhParams);

TI_STATUS mlmeParser_readDsParams(mlme_t *pMlme, 
							   TI_UINT8 *pData, 
							   TI_UINT32 dataLen, 
							   TI_UINT32 *pReadLen, 
							   dot11_DS_PARAMS_t *pDsParams);

TI_STATUS mlmeParser_readCfParams(mlme_t *pMlme, 
							   TI_UINT8 *pData, 
							   TI_UINT32 dataLen, 
							   TI_UINT32 *pReadLen, 
							   dot11_CF_PARAMS_t *pCfParams);

TI_STATUS mlmeParser_readIbssParams(mlme_t *pMlme, 
								 TI_UINT8 *pData, 
								 TI_UINT32 dataLen, 
								 TI_UINT32 *pReadLen, 
								 dot11_IBSS_PARAMS_t *pIbssParams);

TI_STATUS mlmeParser_readTim(mlme_t *pMlme, 
						  TI_UINT8 *pData, 
						  TI_UINT32 dataLen, 
						  TI_UINT32 *pReadLen, 
						  dot11_TIM_t *pTim);

TI_STATUS mlmeParser_readCountry(mlme_t *pMlme,
								 TI_UINT8 *pData,
								 TI_UINT32 dataLen,
								 TI_UINT32 *pReadLen,
								 dot11_COUNTRY_t *countryIE);

TI_STATUS mlmeParser_readWMEParams(mlme_t *pMlme,
								   TI_UINT8 *pData,
								   TI_UINT32 dataLen,
								   TI_UINT32 *pReadLen,
								   dot11_WME_PARAM_t *pWMEParamIE, 
								   assocRsp_t *assocRsp);

TI_STATUS mlmeParser_readPowerConstraint(mlme_t *pMlme,
										 TI_UINT8 *pData,
										 TI_UINT32 dataLen,
										 TI_UINT32 *pReadLen,
										 dot11_POWER_CONSTRAINT_t *powerConstraintIE);

TI_STATUS mlmeParser_readChannelSwitch(mlme_t *pMlme,
									   TI_UINT8 *pData,
									   TI_UINT32 dataLen,
									   TI_UINT32 *pReadLen,
									   dot11_CHANNEL_SWITCH_t *channelSwitch,
                                       TI_UINT8 channel);

TI_STATUS mlmeParser_readTPCReport(mlme_t *pMlme,
								   TI_UINT8 *pData,
								   TI_UINT32 dataLen,
								   TI_UINT32 *pReadLen,
								   dot11_TPC_REPORT_t	*TPCReport);

#ifdef XCC_MODULE_INCLUDED
TI_STATUS mlmeParser_readCellTP(mlme_t *pMlme, 
								TI_UINT8 *pData, 
								TI_UINT32 dataLen, 
								TI_UINT32 *pReadLen, 
								dot11_CELL_TP_t *cellTP);
#endif

TI_STATUS mlmeParser_readQuiet(mlme_t *pMlme,
							   TI_UINT8 *pData,
							   TI_UINT32 dataLen,
							   TI_UINT32 *pReadLen,
							   dot11_QUIET_t *quiet);

TI_STATUS mlmeParser_readChallange(mlme_t *pMlme, 
								TI_UINT8 *pData, 
								TI_UINT32 dataLen, 
								TI_UINT32 *pReadLen, 
								dot11_CHALLENGE_t *pChallange);


TI_STATUS mlmeParser_readRsnIe(mlme_t *pMlme, 
                               TI_UINT8 *pData, 
                               TI_UINT32 dataLen, 
                               TI_UINT32 *pReadLen, 
                               dot11_RSN_t *pRsnIe);

TI_STATUS mlmeParser_readQosCapabilityIE(mlme_t *pMlme,
										 TI_UINT8 *pData, 
										 TI_UINT32 dataLen, 
										 TI_UINT32 *pReadLen, 
										 dot11_QOS_CAPABILITY_IE_t *QosCapParams);

TI_STATUS mlmeParser_readHtCapabilitiesIE (mlme_t *pMlme,
										  TI_UINT8 *pData, 
										  TI_UINT32 dataLen, 
										  TI_UINT32 *pReadLen, 
										  Tdot11HtCapabilitiesUnparse *pHtCapabilities);

TI_STATUS mlmeParser_readHtInformationIE (mlme_t *pMlme,
                                          TI_UINT8 *pData, 
                                          TI_UINT32 dataLen, 
                                          TI_UINT32 *pReadLen, 
                                          Tdot11HtInformationUnparse *pHtInformation);


#endif

