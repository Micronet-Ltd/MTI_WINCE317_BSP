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

#ifndef __SCAN_MNGR_TYPES_API_H__
#define __SCAN_MNGR_TYPES_API_H__


/** \file scanMngrTypes.h
 * \brief Scan Manager Types	\n
 * This file include public type definitions for the scan manager application level module,	\n
 * to be included both by driver and user-mode.
 * \n\n
 */
 
/* \
 * \date 01-Mar-2005
 */

/*
 ***********************************************************************
 *	Constant definitions.
 ***********************************************************************
 */

/** \def MAX_BAND_POLICY_CHANNLES
 * \brief Maximal number of channels per band policy
 */
#define MAX_BAND_POLICY_CHANNLES    30

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
/** \struct TScanProbReqParams
 * \brief Scan Prob Request Parameters
 * 
 * \par Description
 * This structure defines the probe request parameters used for active scan
 * 
 * \sa
 */ 
typedef struct
{
	TI_UINT8					numOfProbeReqs;	/**< Number of probe request frames to be transmitted on each channel	*/
	TI_UINT8 					txPowerDbm;     /**< Tx Power level of the probe request frame (1: strong power; 5: weak power), at which to transmit	*/
	ERateMask					bitrate;        /**< Probe request transmission bit rate	*/
} TScanProbReqParams;

/** \struct TScanBasicMethodParams
 * \brief Scan Basic Method Parameters
 * 
 * \par Description
 * This structure defines the argument used for the basic scan operation. 
 * The basic scan is a standard 802.11 scan, which can be active (sending a probe request frame on each channel) 
 * or passive (without sending probe request frames). For a passive scan, the probe request parameters are ignored
 * 
 * \sa
 */ 
typedef struct
{
	TI_UINT32						maxChannelDwellTime;	/**< Maximum time to stay on the channel if some frames are received but the early termination limit is not reached (microseconds)	*/
	TI_UINT32						minChannelDwellTime;    /**< Minimum time to stay on the channel if no activity at all was discovered (no frames are received) (microseconds)	*/
	EScanEtCondition    			earlyTerminationEvent;  /**< The cause for early termination	*/
	TI_UINT8						ETMaxNumberOfApFrames;  /**< Number of frames from the early termination frame types according to the early TerminationEvent setting, after which the scan is stopped on this channel	*/
	TScanProbReqParams	       		probReqParams;          /**< Probe request transmission parameters: number of frames, TX rate, and TX power level	*/
} TScanBasicMethodParams;

/** \struct TScanTidTriggeredMethodParams
 * \brief Scan Tid Triggered Method Parameters
 * 
 * \par Description
 * This structure defines the policy parameters for an AC triggered scan. 
 * The Scan is passive or active, in which a single-channel scan is triggered by transmission on the serving channel
 * 
 * \sa
 */ 
typedef struct 
{
	TScanBasicMethodParams			basicMethodParams;	/**< Parameters for the basic scan performed in the AC triggered scan process	*/
	TI_UINT8						triggeringTid;		/**< Quality-of-service (QoS) AC that triggers the scans in the AC triggered scan process	*/
} TScanTidTriggeredMethodParams;

/** \struct TScanSPSMethodParams
 * \brief Scan SPS Method Parameters
 * 
 * \par Description
 * This structure defines the policy parameters for an SPS. 
 * SPS is a passive scan that is timed accurately to beacon transmissions.
 * 
 * \sa
 */ 
typedef struct 
{
	EScanEtCondition    			earlyTerminationEvent;	/**< The cause for early termination */
	TI_UINT8				      	ETMaxNumberOfApFrames;  /**< Number of frames from the early termination frame types according to the early Termination Event setting, 
															* after which the scan is stopped on this channel	
															*/
	TI_UINT32					   	scanDuration;           /**< Time to spend on each channel (in usec) */
} TScanSPSMethodParams;

/** \struct TScanMethod
 * \brief Scan Method
 * 
 * \par Description
 * This structure defines the policy parameters for a scan method, which can be any type of scan
 * 
 * \sa
 */ 
typedef struct 
{
	EScanType					scanType;                           /**< Required scan type (active, passive, AC triggered, SPS)	*/
	/* Scan policy parameters */
	union
	{
		TScanBasicMethodParams 		    basicMethodParams;      	/**< scan parameters for normal scan 		*/
        TScanTidTriggeredMethodParams	TidTriggerdMethodParams; 	/**< scan parameters for Tid-Triggered scan */
        TScanSPSMethodParams			spsMethodParams;        	/**< scan parameters for SPS scan 			*/
	} method;
} TScanMethod;

/** \struct TScanBandPolicy
 * \brief Scan Band Policy
 * 
 * \par Description
 * This structure defines the parameters comprising a scan policy for a single band
 * 
 * \sa
 */ 
typedef struct
{
	ERadioBand 				band;	                            	/**< The band (2.4 / 5 GHz) 								*/
	TScanMethod				discoveryMethod;                    	/**< Scan method used to discover new BSS'es 				*/
	TScanMethod	    		trackingMethod;                     	/**< Scan method used to track previously discovered AP's	*/
	TScanMethod		    	immediateScanMethod;                	/**< Scan method used for immediate scan 					*/
	TI_INT8            		rxRSSIThreshold;                    	/**< Quality threshold for received frames 					*/
	TI_UINT8				numOfChannlesForDiscovery;          	/**< Number of channels to scan at each discovery attempt	*/
	TI_UINT8				numOfChannles;                      	/**< Number of channels to use on this band 				*/
	TI_UINT8	           	channelList[ MAX_BAND_POLICY_CHANNLES ];/**< All possible channels 									*/
                                                                    
} TScanBandPolicy;

/** \struct TScanPolicy
 * \brief Scan Policy
 * 
 * \par Description
 * This structure defines the parameters comprising scan policies on all bands
 * 
 * \sa
 */
typedef struct
{
    TI_UINT32  				normalScanInterval;                     	/**< Time interval (im msec) at which to perform
																		* continuous scan, when current BSS quality is
																		* considered "normal".
																		*/
    TI_UINT32	    		deterioratingScanInterval;              	/**< Time interval (in msec) at which to perform
																		* continuous scab, when current BSS quality is
																		* considered "deteriorating"
																		*/
    TI_UINT8				maxTrackFailures;                       	/**< The max number of track failures to keep
																		* an AP in the BSS list
																		*/
    TI_UINT8				BSSListSize;                            	/**< The number of APs in the BSS list */
    TI_UINT8				BSSNumberToStartDiscovery;              	/**< The number of APs in the BSS list at which
																		* discovery process is initiated
																		*/
    TI_UINT8 		    	numOfBands;                             	/**< Number of bands to scan */
    TScanBandPolicy			bandScanPolicy[ RADIO_BAND_NUM_OF_BANDS ];	/**< Bands' policies */
} TScanPolicy;

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


#endif /* __SCAN_MNGR_TYPES_API_H__ */

