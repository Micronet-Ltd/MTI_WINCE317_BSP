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
/** \file admCtrlWpa.h
 *  \brief Admission control API
 *
 *  \see admCtrl.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  Admission Control	    		                                *
 *   PURPOSE: Admission Control Module API                              	*
 *                                                                          *
 ****************************************************************************/

#ifndef _ADM_CTRL_WPA_H_
#define _ADM_CTRL_WPA_H_

/* Constants */

/* Enumerations */

/* Typedefs */


/* RSN admission control prototypes */


/* Structures */



#define MAX_WPA_UNICAST_SUITES        (TWD_CIPHER_CKIP+1)

#define WPA_OUI_MAX_VERSION           0x1
#define WPA_OUI_DEF_TYPE              0x1
#define WPA_OUI_MAX_TYPE			  0x2

#define WPA_GROUP_4_UNICAST_CAPABILITY_MASK  	0x0002
#define WPA_REPLAY_COUNTERS_CAPABILITY_MASK 	0x000c
#define WPA_REPLAY_GROUP4UNI_CAPABILITY_SHIFT 	1
#define WPA_REPLAY_COUNTERS_CAPABILITY_SHIFT 	2

#define WPA_IE_MIN_LENGTH 				 	6
#define WPA_IE_GROUP_SUITE_LENGTH 		 	10
#define WPA_IE_MIN_PAIRWISE_SUITE_LENGTH 	16
#define WPA_IE_MIN_DEFAULT_LENGTH 			24
#define WPA_IE_MIN_KEY_MNG_SUITE_LENGTH(pairwiseCnt) (18+4*pairwiseCnt)

typedef enum 
{
	WPA_IE_KEY_MNG_NONE				= 0,		/**< no key management available */
	WPA_IE_KEY_MNG_801_1X			= 1,		/**< "802.1X" key management - WPA default*/
	WPA_IE_KEY_MNG_PSK_801_1X		= 2,		/**< "WPA PSK */
	WPA_IE_KEY_MNG_CCKM			    = 3,		/**< WPA CCKM */
	WPA_IE_KEY_MNG_NA			    = 4			/**< NA */
} keyMngSuite_e;


#define	MAX_WPA_KEY_MNG_SUITES   	(WPA_IE_KEY_MNG_CCKM+1)


typedef struct
{

	TI_UINT8  				elementid;	   /* WPA information element id is 0xDD */	   
	TI_UINT8  				length;			   
    TI_UINT8  				oui[DOT11_OUI_LEN - 1];
	TI_UINT8  				ouiType;
    TI_UINT16 				version;
    TI_UINT8 				groupSuite[DOT11_OUI_LEN];
    TI_UINT16 				pairwiseSuiteCnt;
	TI_UINT8				pairwiseSuite[DOT11_OUI_LEN];
	TI_UINT16 				authKeyMngSuiteCnt;
	TI_UINT8				authKeyMngSuite[DOT11_OUI_LEN];
    TI_UINT16				capabilities;
} wpaIePacket_t;


/* WPA capabilities structure */
typedef struct
{
    ECipherSuite 		broadcastSuite;
    TI_UINT16 			unicastSuiteCnt;
	ECipherSuite		unicastSuite[MAX_WPA_UNICAST_SUITES];
	TI_UINT16 			KeyMngSuiteCnt;
	ERsnKeyMngSuite	    KeyMngSuite[MAX_WPA_KEY_MNG_SUITES];
    TI_UINT8			bcastForUnicatst;
	TI_UINT8			replayCounters;
    TI_BOOL             XCCKp;
    TI_BOOL             XCCMic;

} wpaIeData_t;


/* External data definitions */

/* External functions definitions */

/* Function prototypes */

TI_STATUS admCtrlWpa_getInfoElement(admCtrl_t *pAdmCtrl, TI_UINT8 *pIe, TI_UINT32 *pLength);

TI_STATUS admCtrlWpa_setSite(admCtrl_t *pAdmCtrl, TRsnData *pRsnData, TI_UINT8 *pAssocIe, TI_UINT8 *pAssocIeLen);

TI_STATUS admCtrlWpa_evalSite(admCtrl_t *pAdmCtrl, TRsnData *pRsnData, TRsnSiteParams *pRsnSiteParams, TI_UINT32 *pEvaluation);

#endif /*  _ADM_CTRL_WPA_H_*/

