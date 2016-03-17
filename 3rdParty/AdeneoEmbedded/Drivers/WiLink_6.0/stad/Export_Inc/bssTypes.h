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
/** \file bssTypes.h
 *  \brief This file include public type definitions for Driver-wide BSS information
 *  \
 *  \date 05-April-2005
 */

#ifndef __BSS_TYPES_API_H__
#define __BSS_TYPES_API_H__

#include "TWDriver.h"
/*
 ***********************************************************************
 *	Constant definitions.
 ***********************************************************************
 */
#define MAX_NUM_OF_NEIGHBOR_APS     30
#define MAX_SIZE_OF_BSS_TRACK_LIST  16


/*
 ***********************************************************************
 *	Enumeration definitions.
 ***********************************************************************
 */
 /** \enum resultFrameType_e
 * \brief Type of result frame
 * 
 * \par Description
 * Enumerates the different types for a result frame
 * 
 * \sa TMib
 */
typedef enum
{
/*	0	*/	SCAN_RFT_BEACON,         	/**< Result frame is a beacon */
/*	1	*/	SCAN_RFT_PROBE_RESPONSE    	/**< Result frame is a probe response */

} resultFrameType_e;

/*
 ***********************************************************************
 *	Structure definitions.
 ***********************************************************************
 */
/** \struct bssEntry_t
 * \brief	BSS entry
 * 
 * \par Description
 * This structure contains a single BSS entry. 
 * E.g. it holds one AP of the BSS list. 
 * 
 * \sa	bssList_t	
 */ 
typedef struct
{
	/* values in beacon with fixed length */
    TMacAddr               BSSID;                  /**< BSSID of this entry */
    TI_UINT64              lastRxTSF;              /**< TSF of last received frame */
    TI_UINT16              beaconInterval;         /**< Beacon interval of this AP */
    TI_UINT16              capabilities;           /**< capabilities of this AP */

	/* IE's in beacon */
    TI_UINT8               DTIMPeriod;             /**< DTIm period (in beacon interval quantas */
	resultFrameType_e	   resultType;             /**< The type of frame in pBuffer */
    TI_UINT16              bufferLength;           /**< length of rest of beacon (or probe response) buffer */
    TI_UINT8*              pBuffer;                /**< rest of beacon (or probe response) buffer */

	/* Information from other sources */
    ERadioBand             band;                   /**< band on which the AP transmits */
    TI_UINT8               channel;                /**< channel on which the AP transmits */
    TI_UINT8               rxRate;                 /**< Rate at which last frame was received */
    TI_UINT32              lastRxHostTimestamp;    /**< 
													* the host timestamp (in milliseconds) at which last frame 
													* was received
													*/
    TI_INT8                RSSI;                   /**< average RSSI */
    TI_INT8                lastRSSI;               /** last given RSSI */
	TI_BOOL                bNeighborAP;            /**< Indicates whether this is a neighbor AP */
} bssEntry_t;

/** \struct bssList_t
 * \brief BSS List
 * 
 * \par Description
 * This structure holds the BSS list. E.g. it holds the AP ESS. 
 * This list is filled by the scan manager and is used by the 
 * Roaming Manager to select the best AP to roam to.
 * 
 * \sa	bssEntry_t
 */ 
typedef struct
{
	TI_UINT8 			   numOfEntries;                               /**< Number of entries in the BSS list */
	bssEntry_t             BSSList[ MAX_SIZE_OF_BSS_TRACK_LIST ];      /**< Pointer to the first entry in the BSS list */
} bssList_t;

/** \struct neighborAP_t
 * \brief	Neighbor AP 
 * 
 * \par Description
 * This structure contains information on one Neighbor AP.	\n 
 * A Neighbor AP is set by the Roaming Manager for the Scan Manager, 
 * and the Scan Manager discovers and tracks the AP. 
 * Neighbor APs have higher priority in the discovery process than the 
 * Channel List configured in the Scan Policy
 * 
 * \sa	neighborAPList_t
 */ 
typedef struct
{
	TMacAddr	    	    BSSID;                          /**< The BSSID (MAC address) of this Neighbor AP */
	TI_UINT8				channel;                        /**< Neighbor AP channel (on which the AP transmits) */
	ERadioBand			    band;                           /**< Neighbor AP band (2.4/5 GHz) (the band used by the AP) */
} neighborAP_t;

/** \struct neighborAPList_t
 * \brief	list of Neighbor APs
 * 
 * \par Description
 * This structure holds a list of all neighbor APs
 * 
 * \sa	neighborAP_t
 */ 
typedef struct
{
	TI_UINT8				numOfEntries;                           /**< Number of entries in Neighbor AP list */
	neighborAP_t            APListPtr[ MAX_NUM_OF_NEIGHBOR_APS ];   /**< Pointer to Neighbor AP list */
} neighborAPList_t;


#endif
