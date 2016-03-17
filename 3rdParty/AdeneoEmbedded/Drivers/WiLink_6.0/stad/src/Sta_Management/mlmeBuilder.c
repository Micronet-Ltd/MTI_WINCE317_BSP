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
 /** \file mlmeBuilder.c
 *  \brief 802.11 MLME Builder
 *
 *  \see mlmeBuilder.h
 */


/***************************************************************************/
/*																		   */
/*		MODULE:	mlmeBuilder.c											   */
/*    PURPOSE:	802.11 MLME Builder										   */
/*																	 	   */
/***************************************************************************/

#define __FILE_ID__  FILE_ID_67
#include "tidef.h"
#include "osApi.h"
#include "paramOut.h"
#include "report.h"
#include "802_11Defs.h"
#include "DataCtrl_Api.h"
#include "mlmeApi.h"
#include "mlmeSm.h"
#include "mlmeBuilder.h"
#include "TWDriver.h"
#include "connApi.h"
/* Constants */

/* Enumerations */

/* Typedefs */

/* Structures */

/* External data definitions */

/* External functions definitions */

/* Local function prototypes */

/* Functions */

TI_STATUS mlmeBuilder_sendFrame(TI_HANDLE hMlme, 
							 dot11MgmtSubType_e type, 
							 TI_UINT8   *pDataBuff, 
							 TI_UINT32  dataLen,
							 TI_UINT8	setWepOpt)
{
	mlme_t			*pHandle = (mlme_t*)hMlme;
	TI_STATUS		status;
    TTxCtrlBlk      *pPktCtrlBlk;
    TI_UINT8        *pPktBuffer;
	TMacAddr		daBssid, saBssid;
	dot11_mgmtHeader_t	*pDot11Header;

	/* Allocate a TxCtrlBlk and data buffer (large enough for the max management packet) */
    pPktCtrlBlk = TWD_txCtrlBlk_Alloc (pHandle->hTWD);
    pPktBuffer  = txCtrl_AllocPacketBuffer (pHandle->hTxCtrl, 
                                            pPktCtrlBlk, 
                                            MAX_MANAGEMENT_FRAME_BODY_LEN + WLAN_HDR_LEN);
    if (pPktBuffer == NULL)
    {
        TRACE0(pHandle->hReport, REPORT_SEVERITY_ERROR , ": No memory\n");
        TWD_txCtrlBlk_Free (pHandle->hTWD, pPktCtrlBlk);
        return TI_NOK;
    }
	
	pDot11Header = (dot11_mgmtHeader_t *)(pPktCtrlBlk->aPktHdr);

	status = mlmeBuilder_buildFrameCtrl (pHandle, type, (TI_UINT16 *)&pDot11Header->fc, setWepOpt);
	if (status != TI_OK)
	{
        txCtrl_FreePacket (pHandle->hTxCtrl, pPktCtrlBlk, TI_NOK);
		return TI_NOK;
	}

    status = ctrlData_getParamBssid(pHandle->hCtrlData, CTRL_DATA_CURRENT_BSSID_PARAM, daBssid);
	if (status != TI_OK)
	{
        txCtrl_FreePacket (pHandle->hTxCtrl, pPktCtrlBlk, TI_NOK);
		return TI_NOK;
	}

	/* copy destination mac address */
	MAC_COPY (pDot11Header->DA, daBssid);

    status = ctrlData_getParamMacAddr(pHandle->hCtrlData, saBssid);
	if (status != TI_OK)
	{
        txCtrl_FreePacket (pHandle->hTxCtrl, pPktCtrlBlk, TI_NOK);
		return TI_NOK;
	}

	/* copy source mac address */
	MAC_COPY (pDot11Header->SA, saBssid);

	/* copy BSSID (destination mac address) */
	MAC_COPY (pDot11Header->BSSID, daBssid);

	if (pDataBuff != NULL)
	{
		os_memoryCopy (pHandle->hOs, pPktBuffer, pDataBuff, dataLen);
	}

    /* Update packet parameters (start-time, length, pkt-type) */
    pPktCtrlBlk->tTxDescriptor.startTime = os_timeStampMs (pHandle->hOs);
    pPktCtrlBlk->tTxPktParams.uPktType   = TX_PKT_TYPE_MGMT;
    BUILD_TX_TWO_BUF_PKT_BDL (pPktCtrlBlk, (TI_UINT8 *)pDot11Header, WLAN_HDR_LEN, pPktBuffer, dataLen)

	/* Enqueue packet in the mgmt-queues and run the scheduler. */
	status = txMgmtQ_Xmit (pHandle->hTxMgmtQ, pPktCtrlBlk, TI_FALSE);

	return status;
}


TI_STATUS mlmeBuilder_buildFrameCtrl(mlme_t* pMlme, dot11MgmtSubType_e type, TI_UINT16* pFctrl, TI_UINT8 setWepOpt)
{
	TI_UINT16	fc = 0;

	switch (type)
	{
	case ASSOC_REQUEST:
		fc |= DOT11_FC_ASSOC_REQ;
		break;
	case ASSOC_RESPONSE:
		fc |= DOT11_FC_ASSOC_RESP;
		break;
	case RE_ASSOC_REQUEST:
		fc |= DOT11_FC_REASSOC_REQ;
		break;
	case RE_ASSOC_RESPONSE:
		fc |= DOT11_FC_REASSOC_RESP;
		break;
	case DIS_ASSOC:
		fc |= DOT11_FC_DISASSOC;
		break;
	case AUTH:
		fc |= DOT11_FC_AUTH;
		break;
	case DE_AUTH:
		fc |= DOT11_FC_DEAUTH;
		break;
	case ACTION:
		fc |= DOT11_FC_ACTION;
		break;
	default:
		*pFctrl = 0;
		return TI_NOK;
	}
	
	if (setWepOpt)
	{
		fc |= DOT11_FC_WEP;
	}

	COPY_WLAN_WORD(pFctrl, &fc); /* copy with endianess handling. */

	return TI_OK;
}

