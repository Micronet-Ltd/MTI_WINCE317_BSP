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
/** \file regulatoryDomainApi.h
 *  \brief regulatoryDomain module interface header file
 *
 *  \see regulatoryDomain.c & regulatoryDomain.h
 */

/***************************************************************************/
/*                                                                          */
/*    MODULE:   regulatoryDomainApi.h                                       */
/*    PURPOSE:  regulatoryDomain module interface header file               */
/*                                                                          */
/***************************************************************************/
#ifndef __REGULATORY_DOMAIN_API_H__
#define __REGULATORY_DOMAIN_API_H__


#include "802_11Defs.h"
#include "regulatoryDomain.h"
#include "DrvMainModules.h"

TI_HANDLE regulatoryDomain_create(TI_HANDLE hOs);

void      regulatoryDomain_init (TStadHandlesList *pStadHandles);

TI_STATUS regulatoryDomain_SetDefaults (TI_HANDLE  hRegulatoryDomain,
                                        regulatoryDomainInitParams_t *pRegulatoryDomainInitParams);
/**
 * \brief	Set Regulatory Domain Parameter 
 * 
 * \param  hRegulatoryDomain	-	Handle to the regulatory domain object 
 * \param  pParam				-	Pointer to the input parameter
 * \return TI_OK on success, TI_NOK otherwise
 * 
 * \par Description
 * Configure channel validity information to the regulatory domain object.
 * called by the following:
 *	- config mgr in order to set a parameter receiving to the OS abstraction layer.
 * 	- From inside the driver
 * 
 * \sa	
 */ 
TI_STATUS regulatoryDomain_setParam(TI_HANDLE hRegulatoryDomain, paramInfo_t *pParam);
/**
 * \brief	Get Regulatory Domain Parameter 
 * 
 * \param  hRegulatoryDomain	-	Handle to the regulatory domain object 
 * \param  pParam				-	Pointer to the output parameter
 * \return TI_OK on success, TI_NOK otherwise
 * 
 * \par Description
 * Retrieves channel validity information from the regulatory domain object.
 * Called by the following:
 *	- Configuration Manager in order to get a parameter from the OS abstraction layer.
 *	- From inside the driver	 
 * 
 * \sa	
 */ 
TI_STATUS regulatoryDomain_getParam(TI_HANDLE hRegulatoryDomain, paramInfo_t *pParam);

TI_STATUS regulatoryDomain_destroy(TI_HANDLE hRegulatoryDomain);

#endif /* __REGULATORY_DOMAIN_API_H__*/


