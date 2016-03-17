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
/***************************************************************************/
/*                                                                         */
/*    MODULE:   MibDbg.c                                                 */
/*    PURPOSE:  MIB debug implementation                                    */
/*                                                                         */
/***************************************************************************/

#include "MibDbg.h"
#include "TWDriver.h"
#include "report.h"
#include "osApi.h"

/*
 ***********************************************************************
 *	Internal functions definitions
 ***********************************************************************
 */

static void mibDbgGetArpIpTable(TI_HANDLE hTWD)
{
	TI_STATUS status = TI_OK;
    TMib mib;
    TI_INT32 i;

	/* init */
	mib.aMib = MIB_arpIpAddressesTable;
	mib.aData.ArpIpAddressesTable.FilteringEnable = 0;
	for ( i = 0 ; i < IP_V4_ADDR_LEN ; i++ )
	{
		mib.aData.ArpIpAddressesTable.addr[i] = 0;
	}

	status = TWD_ReadMib(hTWD,NULL,NULL,(void*)&mib);
	if (status != TI_OK)
	{
		WLAN_OS_REPORT(("Get ARP Table failed\n"));
		return;
	}

	/* print ARP Table */
    WLAN_OS_REPORT(("ARP IP Table:\n"));
    WLAN_OS_REPORT(("FilteringEnable: %s (%d)\n", 
				   (mib.aData.ArpIpAddressesTable.FilteringEnable)?"Enable":"Disable",
				   mib.aData.ArpIpAddressesTable.FilteringEnable));
    WLAN_OS_REPORT(("ArpIpAddress: %d.%d.%d.%d\n", 
					mib.aData.ArpIpAddressesTable.addr[0],
					mib.aData.ArpIpAddressesTable.addr[1],
					mib.aData.ArpIpAddressesTable.addr[2],
					mib.aData.ArpIpAddressesTable.addr[3]));
}

static void mibDbgGetGroupAddressTable(TI_HANDLE hTWD)
{
	TI_STATUS status = TI_OK;
    TMib mib;
    TI_INT32 i,j;

	/* init */
	mib.aMib 										= MIB_dot11GroupAddressesTable;
	mib.aData.GroupAddressTable.bFilteringEnable 	= 0;
	mib.aData.GroupAddressTable.nNumberOfAddresses 	= 0;
    for ( i = 0 ; i < MIB_MAX_MULTICAST_GROUP_ADDRS ; i++ )
	{
		for ( j = 0 ; j < MAC_ADDR_LEN ; j++ )
		{
			mib.aData.GroupAddressTable.aGroupTable[i][j]	= 0;
		}
	}

	status = TWD_ReadMib(hTWD,NULL,NULL,(void*)&mib);
	if (status != TI_OK)
	{
		WLAN_OS_REPORT(("Get Group Address Table failed\n"));
		return;
	}

	/* print Group Address Table */
    WLAN_OS_REPORT(("Group addresses Table:\n"));
    WLAN_OS_REPORT(("FilteringEnable: %s (%d)\n", 
				   (mib.aData.GroupAddressTable.bFilteringEnable)?"Enable":"Disable",
				   mib.aData.GroupAddressTable.bFilteringEnable));
    WLAN_OS_REPORT(("nNumberOfAddresses: %d\n", 
				   mib.aData.GroupAddressTable.nNumberOfAddresses));

    WLAN_OS_REPORT(("Group addresses: \n"));
    for ( i = 0 ; i < MIB_MAX_MULTICAST_GROUP_ADDRS ; i++ )
		WLAN_OS_REPORT(("%x:%x:%x:%x:%x:%x\n",     
						mib.aData.GroupAddressTable.aGroupTable[i][0],
						mib.aData.GroupAddressTable.aGroupTable[i][1],
						mib.aData.GroupAddressTable.aGroupTable[i][2],
						mib.aData.GroupAddressTable.aGroupTable[i][3],
						mib.aData.GroupAddressTable.aGroupTable[i][4],
						mib.aData.GroupAddressTable.aGroupTable[i][5]));
}

static void mibDbgGetCounterTable(TI_HANDLE hTWD)
{
	TI_STATUS status = TI_OK;
    TMib mib;

	/* init */
	mib.aMib 								= MIB_countersTable;
	mib.aData.CounterTable.FCSErrorCount	= 0;
	mib.aData.CounterTable.PLCPErrorCount	= 0;
	mib.aData.CounterTable.SeqNumMissCount	= 0;

	status = TWD_ReadMib(hTWD,NULL,NULL,(void*)&mib);
	if (status != TI_OK)
	{
		WLAN_OS_REPORT(("Get Counter Table failed\n"));
		return;
	}

	/* print Counter Table */
    WLAN_OS_REPORT(("FCS error count= %d \nPLCP  error count = %d \nSeq error count= %d\n",     
					mib.aData.CounterTable.FCSErrorCount,
					mib.aData.CounterTable.PLCPErrorCount,
					mib.aData.CounterTable.SeqNumMissCount));

}

static void mibDbgModifyCtsToSelf(TI_HANDLE hTWD, void* pParam)
{
	TI_STATUS status = TI_OK;
    TMib mib;

	if (pParam == NULL) 
	{
		WLAN_OS_REPORT(("Modify CTS To Self failed: No Parameter received\n"));
		return;
	}

	/* init */
	mib.aMib					= MIB_ctsToSelf;
	mib.aData.CTSToSelfEnable 	= 0;
	mib.aData.CTSToSelfEnable 	= *(TI_UINT32*)pParam;

	status = TWD_WriteMib(hTWD, &mib);
	if (status != TI_OK)
	{
		WLAN_OS_REPORT(("Modify CTS To Self failed\n"));
		return;
	}
	WLAN_OS_REPORT(("Modify CTS To Self OK\n"));

	WLAN_OS_REPORT(("CtsToSelf = %s (%d)\n",
				   mib.aData.CTSToSelfEnable?"Enable":"Disable",
				   mib.aData.CTSToSelfEnable));
	WLAN_OS_REPORT(("CTS to self: [0 - Disable, 1 - Enable]\n"));
}

static void mibDbgGetCtsToSelf(TI_HANDLE hTWD)
{
	TI_STATUS status = TI_OK;
    TMib mib;

	/* init */
	mib.aMib					= MIB_ctsToSelf;
	mib.aData.CTSToSelfEnable 	= 0;

	status = TWD_ReadMib(hTWD,NULL,NULL,(void*)&mib);
	if (status != TI_OK)
	{
		WLAN_OS_REPORT(("Get CTS To Self failed\n"));
		return;
	}

	WLAN_OS_REPORT(("CtsToSelf = %s (%d)\n",
				   mib.aData.CTSToSelfEnable?"Enable":"Disable",
				   mib.aData.CTSToSelfEnable));
	WLAN_OS_REPORT(("CTS to self: [0 - Disable, 1 - Enable]\n"));
}

static void mibDbgSetMaxRxLifetime(TI_HANDLE hTWD, void* pParam)
{
	TI_STATUS status = TI_OK;
    TMib mib;

	if (pParam == NULL) 
	{
		WLAN_OS_REPORT(("Set Maximum Rx Life Time failed: No Parameter received\n"));
		return;
	}

	/* init */
	mib.aMib 						= MIB_dot11MaxReceiveLifetime;
	mib.aData.MaxReceiveLifeTime 	= *(TI_UINT32*)pParam;

	status = TWD_WriteMib(hTWD, &mib);
	if (status != TI_OK)
	{
		WLAN_OS_REPORT(("Set Maximum Rx Life Time failed\n"));
		return;
	}
}

/*
 ***********************************************************************
 *	External functions definitions
 ***********************************************************************
 */
void mibDbgPrintFunctions(void)
{
	WLAN_OS_REPORT(("\n          MIB Dbg Functions   \n"));
	WLAN_OS_REPORT(("--------------------------------------\n"));

	WLAN_OS_REPORT(("2101 - Print ARP Table\n"));
	WLAN_OS_REPORT(("2102 - Print Group Address Table\n"));
	WLAN_OS_REPORT(("2103 - Print Counter Table\n"));
	WLAN_OS_REPORT(("2104 - Print Modify CTS to Self\n"));
	WLAN_OS_REPORT(("2105 - Print Max RX Life Time\n"));
}

void MibDebugFunction(TI_HANDLE hTWD ,TI_UINT32 funcType, void* pParam)
{
	if (hTWD == NULL) 
	{
		return;
	}

	switch (funcType)
	{
	case DBG_MIB_PRINT_HELP:
		mibDbgPrintFunctions();
		break;
	case DBG_MIB_GET_ARP_TABLE:
		mibDbgGetArpIpTable(hTWD);
		break;
	case DBG_MIB_GET_GROUP_ADDRESS_TABLE:
		mibDbgGetGroupAddressTable(hTWD);
		break;
	case DBG_MIB_GET_COUNTER_TABLE:
		mibDbgGetCounterTable(hTWD);
		break;
	case DBG_MIB_MODIFY_CTS_TO_SELF:
		mibDbgModifyCtsToSelf(hTWD, pParam);
		break;
	case DBG_MIB_GET_CTS_TO_SELF:
		mibDbgGetCtsToSelf(hTWD);
		break;
	case DBG_MIB_SET_MAX_RX_LIFE_TIME:
		mibDbgSetMaxRxLifetime(hTWD, pParam);
		break;
	default:
        WLAN_OS_REPORT(("MIB Debug: Invalid function type in MIB Debug function: %d\n", funcType));
		break;
	}
}
