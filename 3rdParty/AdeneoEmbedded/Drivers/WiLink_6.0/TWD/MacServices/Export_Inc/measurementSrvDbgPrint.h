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
/** \file MeasurementSrv.h
 *  \brief This file include definitions for the measurmeent SRV module debug print functions.
 *  \
 *  \date 23-december-2005
 */

#ifndef __MEASUREMENT_SRV__SBG_PRINT_H__
#define __MEASUREMENT_SRV__SBG_PRINT_H__

/*
 ***********************************************************************
 *	Constant definitions.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *	Enums.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *	Typedefs.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *	Structure definitions.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *	External data definitions.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *	External functions definitions
 ***********************************************************************
 */
/** 
 * \\n
 * \date 23-December-2005\n
 * \brief Prints a measurement request.\n
 *
 * Function Scope \e Public.\n
 * \param hMeasurementSRV - handle to the measurement SRV object.\n
 * \param pMsrRequest - the measurement request.\n
 */
void measurementSRVPrintRequest( TI_HANDLE hMeasurementSRV, TMeasurementRequest *pMsrRequest );

/** 
 * \\n
 * \date 23-December-2005\n
 * \brief Prints a measurement type request.\n
 *
 * Function Scope \e Public.\n
 * \param hMeasurementSRV - handle to the measurement SRV object.\n
 * \param pMsrTypeRequest - the measurement type request.\n
 */
void measurementSRVPrintTypeRequest( TI_HANDLE hMeasurementSRV, TMeasurementTypeRequest* pMsrTypeRequest );

#endif /* __MEASUREMENT_SRV__SBG_PRINT_H__ */

