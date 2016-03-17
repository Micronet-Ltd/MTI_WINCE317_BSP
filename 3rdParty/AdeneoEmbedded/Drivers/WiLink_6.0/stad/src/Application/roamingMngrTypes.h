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

/****************************************************************************
 *                                                                          *
 *   MODULE:  Roaming Manager	    		                                *
 *   PURPOSE: Roaming Manager Module Types                              		*
 *                                                                          *
 ****************************************************************************/

#ifndef _ROAMING_MNGR_TYPES_H_
#define _ROAMING_MNGR_TYPES_H_

/** \file  roamingMngrTypes.h 
 *  \brief Internal Roaming Manager Types API
 */

/*-----------*/
/* Constants */
/*-----------*/
#define ROAMING_ENABLED 		1
#define ROAMING_DISABLED		0

#define ROAMING_OPERATIONAL_MODE_MANUAL    0
#define ROAMING_OPERATIONAL_MODE_AUTO 	   1


/*--------------*/
/* Enumerations */
/*--------------*/


/*----------*/
/* Typedefs */
/*----------*/

/** \struct roamingMngrConfig_t
 * \brief Roaming Manager Configuration
 * 
 * \par Description
 * This Structure defines the Roaming Manager Configuration Type
 * 
 * \sa
 */ 
typedef struct
{
	TI_UINT16	enableDisable; 					/**< Indicates if roaming is enabled/disabled: 
												* ROAMING_ENABLED: Enables roaming manager
												* ROAMING _DISABLED: Disables roaming manager
												* TI recommended default: roaming is disabled
												* Note: TI_BOOL is not used, beacuse of misdefinition between the Driver and CU
												*/
    TI_UINT16	lowPassFilterRoamingAttempt;	/**< Time to wait in seconds before roaming caused by low connection quality; 
												* TI recommended default: 30 seconds	
												*/
	TI_INT8		apQualityThreshold;				/**< Quality indicator (RSSI) to be used when comparing AP List matching quality
												* TI recommended default: 0
												*/
} roamingMngrConfig_t;

/** \struct roamingMngrThresholdsConfig_t
 * \brief Roaming Manager Thresholds Configuration
 * 
 * \par Description
 * This Structure defines the Roaming Manager Trigger Thresholds Configuration type	\n
 * Note - It is the applicatio's responsibility to change the threshold when Voice is On
 * 
 * \sa
 */ 
typedef struct
{
	TI_UINT8	dataRetryThreshold;						/**< Consecutive number of TX retries per roaming event; 
														* TI recommended default: 20
														*/  
    TI_UINT8	numExpectedTbttForBSSLoss;				/**< Number of expected TBTTs for BSS Loss event
														* TI recommended default: 10
														*/
	TI_UINT8	txRateThreshold;						/**< Low TX rate roaming even threshold; 
														* TI recommended default: 2 Mbps
														*/
	TI_INT8		lowRssiThreshold;						/**< When the low RSSI threshold is crossed, the low RSSI roaming event is initiated in dBm; 
														* TI recommended default: -80 dBm
														*/
	TI_UINT8	lowSnrThreshold;						/**< When the low SNR threshold is crossed, the low SNR roaming event is initiated. 
														* TI recommended default: 0
														*/
	TI_INT8		lowQualityForBackgroungScanCondition; 	/**< Indicator used to increase the background scan period when quality is low 
														* TI recommended default: -80
														*/
	TI_INT8		normalQualityForBackgroungScanCondition;/**< Indicator used to reduce the background scan period when quality is normal 
														* TI recommended default: -80
														*/	
} roamingMngrThresholdsConfig_t;

/** \struct roamingMngrConfigParams_t
 * \brief Roaming Manager Configuration Parameters
 * 
 * \par Description
 * 
 * \sa
 */ 
typedef struct
{
	roamingMngrConfig_t				roamingMngrConfig;				/**< Roaming Manager Configuration				*/
	roamingMngrThresholdsConfig_t	roamingMngrThresholdsConfig;	/**< Roaming Manager Thresholds Configuration	*/
} roamingMngrConfigParams_t;

/** \struct TUserDefinedQualityTrigger
 * \brief User Defined Quality Trigger
 * 
 * \par Description
 * This struct defines the User Defined Quality Trigger Parameters	\n
 * Used for setting Roaming User Defined Trigger
 * 
 * \sa
 */ 
typedef struct
{
    TI_INT16  iThreshold;	/**< Input Trigger Threshold. Units: dBm / dB ; Range: (-100 .. 100)						*/
    TI_UINT16 uPacing;		/**< Minimum delay between consecutive triggers. Units: milliseconds ; Range:  (0 .. 60000) */
    TI_UINT8  uMetric;		/**< 0 - RSSI Beacon, 1 - RSSI Packet, 2 - SNR Beacon, 3 - SNR Packet 						*/
    TI_UINT8  uType;		/**< 0 - Level, 1 - Edge 																	*/
    TI_UINT8  uDirection;	/**< 0 - Low, 1 - High, 2 - Bidirectional 													*/
    TI_UINT8  uHystersis;	/**< Hysteresis range around the threshold value. Units: dB ; Threshold range: (0 .. 255)	*/
    TI_UINT8  uIndex; 		/**< User Index (0 .. 1) 																	*/
    TI_UINT8  uEnable;		/**< User trigger Enable. 0 - Disable, 1 - Enable 											*/
    TI_UINT8  padding[2];  	/**< */ 	
    TI_UINT16 uClientID;    /**< the client ID that registered for the trigger event									*/
} TUserDefinedQualityTrigger;


#endif /*  _ROAMING_MNGR_TYPES_H_*/

