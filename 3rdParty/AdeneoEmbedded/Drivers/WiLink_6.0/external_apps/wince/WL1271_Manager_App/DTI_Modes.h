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
**|     Copyright (c) 1998-2010 Texas Instruments Incorporated           |**
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

#ifndef __DTI_MODES_H
#define __DTI_MODES_H

#include "tidef.h"
#include "TWDriverScan.h"

// Global Vars for Scan Configuration
static TI_UINT32		gDTIScanMode; //2100 : Immediate Scan, 2101 : Discovery, 2102 : Tracking

typedef struct
{
	TI_UINT32						maxChannelDwellTime;	/**< Maximum time to stay on the channel if some frames are received but the early termination limit is not reached (microseconds)	*/
	TI_UINT32						minChannelDwellTime;    /**< Minimum time to stay on the channel if no activity at all was discovered (no frames are received) (microseconds)	*/
	EScanEtCondition    			earlyTerminationEvent;  /**< The cause for early termination	*/
	TI_UINT8						ETMaxNumberOfApFrames;  /**< Number of frames from the early termination frame types according to the early TerminationEvent setting, after which the scan is stopped on this channel	*/

	TI_UINT8						numOfProbeReqs;	/**< Number of probe request frames to be transmitted on each channel	*/
	TI_UINT8 						txPowerDbm;     /**< Tx Power level of the probe request frame (1: strong power; 5: weak power), at which to transmit	*/
	ERateMask						bitrate;        /**< Probe request transmission bit rate	*/

	TI_UINT8						triggeringTid;		/**< Quality-of-service (QoS) AC that triggers the scans in the AC triggered scan process	*/

	TI_UINT32					   	scanDuration;           /**< Time to spend on each channel (in usec) */
	EScanType						scanType;                           /**< Required scan type (active, passive, AC triggered, SPS)	*/
	ERadioBand						bandIndex;
} TScanAppData;

typedef struct {
	TScanAppData	discData;
	TScanAppData	trackData;
	TScanAppData	immScanData;
} DTI_Mode_t;

static ERadioBand gDTIBandIndex;

static DTI_Mode_t gDTIScanAppData[2];

INT_PTR CALLBACK ScanWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#endif