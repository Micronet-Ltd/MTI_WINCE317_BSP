/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  args.c
//
//  This file implements the OEMArgsQuery handler.
//

#include <bsp.h>

//------------------------------------------------------------------------------
//  Static variables
//
//  These values must be generated uniquely for each WinMobile device.  On 35xx,
//  the eFuse key registers CONTROL_RPUB_KEY_H_x, CONTROL_RAND_KEY_x and 
//  CONTROL_CUST_KEY_x should be used to generate the UUID and HWEntropy values.
//  For EVM3530, these keys are all set to 0, so the Device ID set within the
//  EBOOT bootloader is used for unique identification of the EVM3530.
//  Another alternative is to use the one-time programmable area of NAND flash.
//
//  See the WinMobile 5.0 documentation on Hardware Device ID and Entropy

static UCHAR    s_deviceId[24];

//                         Manfacturer ID----  VarVer  Device ID------------------------------------- 
static GUID     s_uuid = { 0x00000000, 0x3024, 0x0801, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static DWORD    s_dwHWEntropy[2] = { 0, 0 };

    // Note: Bit 1 (Locally Administered Address bit) in MAC address
    // must not be set. UsbFn driver uses returned address as MAC address
    // on desktop (XP, W2K) side of virtual RNDIS network. On device
    // side same address with set LAA bit is used.

static UCHAR    s_rndisMAC[6] = { 0x00, 0x24, 0x30, 0xAB, 0x12, 0x34 };
static UCHAR    s_ethMAC[6] =	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static UCHAR    s_wifiMAC[6] =	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static UINT16   s_CdsParams[8] = { 87/*90°C*/,          81/*80°C*/,  77/*75°C*/, 
                                   0xFF/*Feature ON */, 1000/*MHz*/, 2 /*opm of 600MHz */, 0 ,0}; //Defaults for 3703

static  BOOL    s_bInitialized = FALSE;


//------------------------------------------------------------------------------
//
//  Function:  OEMArgsQuery
//
//  This function is called from other OAL modules to return boot arguments.
//  Boot arguments are typically placed in fixed memory location and they are
//  filled by boot loader. In case that boot arguments can't be located
//  the function should return NULL. The OAL module then must use
//  default values.
//
VOID* OALArgsQuery(UINT32 type)
{
    VOID *pData = NULL;
    BSP_ARGS *pArgs;

    OALMSG(OAL_ARGS&&OAL_FUNC, (L"+OALArgsQuery(%d)\r\n", type));

    // Get pointer to expected boot args location
    pArgs = OALPAtoUA(IMAGE_SHARE_ARGS_PA);

    // Check if there is expected signature
    if ((pArgs->header.signature != OAL_ARGS_SIGNATURE) ||
        (pArgs->header.oalVersion != OAL_ARGS_VERSION) ||
        (pArgs->header.bspVersion != BSP_ARGS_VERSION))
	{
		OALMSG(1, (L"OALArgsQuery: incorrect %x %x %x\r\n",	pArgs->header.signature,
																				pArgs->header.oalVersion,
																				pArgs->header.bspVersion));
		goto cleanUp;
	}

    //  Initialize settings
    if( s_bInitialized == FALSE )
    {
        int    i, count;
		UINT32 DieID_1;
        UCHAR  val;

		OMAP_IDCORE_REGS *pIdRegs;    
		pIdRegs = OALPAtoUA(OMAP_IDCORE_REGS_PA);

		if(pIdRegs)
			pArgs->deviceID = INREG32(&pIdRegs->DIE_ID_0);
		else
			pArgs->deviceID = 0;

		DieID_1 = pArgs->deviceID ^ 0xFFFFFFFF;
		s_uuid.Data4[0] = (UCHAR)(DieID_1 >> 24);
		s_uuid.Data4[1] = (UCHAR)(DieID_1 >> 16);
		s_uuid.Data4[2] = (UCHAR)(DieID_1 >> 8);
		s_uuid.Data4[3] = (UCHAR)(DieID_1);

		// Copy prefix for DEVICEID    
        count = sizeof(BSP_DEVICE_PREFIX) - 1;
        if (count > sizeof(s_deviceId)-1) count = sizeof(s_deviceId) -1;
        memset(s_deviceId, 0, sizeof(s_deviceId));
        memcpy(s_deviceId, BSP_DEVICE_PREFIX, count);

        // Append DeviceID as hex string    
        for( i = 0; i < 8 && (count < sizeof(s_deviceId) - 1); i++, count++)
        {
            val = (UCHAR)((pArgs->deviceID >> (28 - i*4)) & 0xF);
            s_deviceId[count] = val < 10 ? '0' + val : 'A' + val - 10;
        }
        
        //  Use DeviceID as last part of UUID
        s_uuid.Data4[4] = (UCHAR)(pArgs->deviceID >> 24);
        s_uuid.Data4[5] = (UCHAR)(pArgs->deviceID >> 16);
        s_uuid.Data4[6] = (UCHAR)(pArgs->deviceID >> 8);
        s_uuid.Data4[7] = (UCHAR)(pArgs->deviceID);

        //  Use DeviceID for HWEntropy value
        s_dwHWEntropy[0] = pArgs->deviceID;
        s_dwHWEntropy[1] = pArgs->deviceID ^ 0xFFFFFFFF;

        //  Use DeviceID for ActiveSync RNDIS MAC
/*        s_rndisMAC[2] = (pArgs->deviceID >> 24);
        s_rndisMAC[3] = (pArgs->deviceID >> 16);
        s_rndisMAC[4] = (pArgs->deviceID >> 8);
        s_rndisMAC[5] = (pArgs->deviceID);
*/
		memcpy(s_rndisMAC, pArgs->RndisMacAddress, sizeof(s_rndisMAC));
		memcpy(s_ethMAC, pArgs->EthMacAddress, sizeof(s_ethMAC));
		memcpy(s_wifiMAC, pArgs->WiFiMacAddress, sizeof(s_wifiMAC));
		memcpy(s_CdsParams, pArgs->CdsParams, sizeof(s_CdsParams));

		s_bInitialized = TRUE;
    }

    // Depending on required args    
    switch (type) 
    {
        case OAL_ARGS_QUERY_UPDATEMODE:
            pData = &pArgs->updateMode;
            break;

        case OAL_ARGS_QUERY_KITL:
			memcpy(pArgs->kitl.mac, pArgs->EthMacAddress, sizeof(pArgs->kitl.mac));
            pData = &pArgs->kitl;
            break;

        case OAL_ARGS_QUERY_COLDBOOT:
            pData = &pArgs->coldBoot;
            break;
		
		case OAL_ARGS_QUERY_HIVECLEAN:
			pData = &pArgs->bHiveCleanFlag;
		    break;

		case OAL_ARGS_QUERY_FORMATPART:
			pData = &pArgs->bFormatPartFlag;
			break;
    
        case OAL_ARGS_QUERY_DEVID:
            pData = s_deviceId;
            break;
        
        case OAL_ARGS_QUERY_UUID:
            pData = &s_uuid;
            break;

        case OAL_ARGS_QUERY_HWENTROPY:
            pData = s_dwHWEntropy;
            break;

        case OAL_ARGS_QUERY_RNDISMAC:
            pData = s_rndisMAC;
            break;
        case OAL_ARGS_QUERY_ETHMAC:
            pData = s_ethMAC;
            break;
		case OAL_ARGS_QUERY_SW_UPDATE:
			pData = &pArgs->SwUpdateFlags;
			break;
		case OAL_ARGS_QUERY_MUNUFACT:
			pData = pArgs->ManufBlock;
			break;
		case OAL_ARGS_QUERY_EBOOT_VERS:
			pData = &pArgs->EbootVersionExt;
			if(!pData || 0 == *(UINT32*)pData)//old ver
				pData = &pArgs->EbootVersion;
			break;
		case OAL_ARGS_QUERY_XLDR_VERS:
			pData = &pArgs->XldrVersionExt;
			if(!pData || 0 == *(UINT32*)pData)//old ver
				pData = &pArgs->XldrVersion;
			break;
		case OAL_ARGS_QUERY_MAIN_CFG:
			pData = &pArgs->main_board;
			break;
		case OAL_ARGS_QUERY_LCD_CFG:
			pData = &pArgs->display_board;
			break;
		case OAL_ARGS_QUERY_WLAN_CFG:
			pData = &pArgs->wlan_board;
			break;
		case OAL_ARGS_QUERY_CAN_CFG:
			pData = &pArgs->canbus_board;
			break;
        case OAL_ARGS_QUERY_OALFLAGS:
            pData = &pArgs->oalFlags;
            break;
		case OAL_ARGS_QUARY_HW_BOOT_STAT:
            pData = &pArgs->hw_stat;
            break;
		case OAL_ARGS_QUERY_GPS_CFG:
			pData = &pArgs->gps_board;
			break;
		case OAL_ARGS_QUERY_MODEM_CFG:
			pData = &pArgs->modem_board;
			break;
		case OAL_ARGS_QUERY_EXTAUDIO_CFG:
			pData = &pArgs->extaudio_board;
			break;
        case OAL_ARGS_QUERY_WIFIMAC:
			pData = s_wifiMAC;
			break;
		case OAL_ARGS_QUERY_CDSPARAMS:
			pData = s_CdsParams;
			break;
		case OAL_ARGS_QUERY_THERMALSHUTDOWN:
			pData = &pArgs->ThermalShutdown;
			break;
		default:
            pData = NULL;
            break;
    }

cleanUp:
    OALMSG(OAL_ARGS&&OAL_FUNC, (L"-OALArgsQuery(pData = 0x%08x)\r\n", pData));
    return pData;
}

//------------------------------------------------------------------------------
