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
/** \file measurementSrvDbgPrint.c
 *  \brief This file include variuos measurement SRV debug print facilities
 *  \
 *  \date 23-December-2005
 */

#define __FILE_ID__  FILE_ID_111
#include "report.h"
#include "TWDriver.h"
#include "MeasurementSrvSM.h"
#include "MeasurementSrv.h"
#include "measurementSrvDbgPrint.h"


/** 
 * \\n
 * \date 23-December-2005\n
 * \brief Prints a measurement request.\n
 *
 * Function Scope \e Public.\n
 * \param hMeasurementSRV - handle to the measurement SRV object.\n
 * \param pMsrRequest - the measurement request.\n
 */
void measurementSRVPrintRequest( TI_HANDLE hMeasurementSRV, TMeasurementRequest *pMsrRequest )
{
#ifdef TI_DBG    
	measurementSRV_t* pMeasurementSRV = (measurementSRV_t*)hMeasurementSRV;
	TI_INT32 i;

TRACE0( pMeasurementSRV->hReport, REPORT_SEVERITY_INFORMATION, "Measurement request:\n");
TRACE5( pMeasurementSRV->hReport, REPORT_SEVERITY_INFORMATION, "band: %d, channel:%d, TX power level: %d, start time: %x-%x\n", pMsrRequest->band, pMsrRequest->channel, pMsrRequest->txPowerDbm, INT64_HIGHER(pMsrRequest->startTime), INT64_LOWER(pMsrRequest->startTime));
	for ( i = 0; i < pMsrRequest->numberOfTypes; i++ )
	{
		measurementSRVPrintTypeRequest( hMeasurementSRV, &(pMsrRequest->msrTypes[ i ]));
	}
#endif /* TI_DBG */

}

/** 
 * \\n
 * \date 23-December-2005\n
 * \brief Prints a measurement type request.\n
 *
 * Function Scope \e Public.\n
 * \param hMeasurementSRV - handle to the measurement SRV object.\n
 * \param pMsrTypeRequest - the measurement type request.\n
 */
void measurementSRVPrintTypeRequest( TI_HANDLE hMeasurementSRV, TMeasurementTypeRequest* pMsrTypeRequest )
{
#ifdef TI_DBG    
	measurementSRV_t* pMeasurementSRV = (measurementSRV_t*)hMeasurementSRV;

TRACE4( pMeasurementSRV->hReport, REPORT_SEVERITY_INFORMATION, "Measurement type request: type: %d, duration:%d, scan mode: %d, reserved: %d", pMsrTypeRequest->msrType, pMsrTypeRequest->duration, pMsrTypeRequest->scanMode, pMsrTypeRequest->reserved);

#endif /* TI_DBG */
}

