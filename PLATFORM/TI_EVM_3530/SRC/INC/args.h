//
//=============================================================================
//            Texas Instruments OMAP(TM) Platform Software
// (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
//
//  Use of this software is controlled by the terms and conditions found
// in the license agreement under which this software has been supplied.
//
//=============================================================================
//

//------------------------------------------------------------------------------
//
//  File:  args.h
//
//  This header file defines device structures and constant related to boot
//  configuration. BOOT_CFG structure defines layout of persistent device
//  information. It is used to control boot process. BSP_ARGS structure defines
//  information passed from boot loader to kernel HAL/OAL. Each structure has
//  version field which should be updated each time when structure layout
//  change.
//
#ifndef __ARGS_H
#define __ARGS_H

//------------------------------------------------------------------------------

#include <oal_args.h>
#include <oal_kitl.h>

#define MANUFACTURE_BLOCK_SIZE     1024
//------------------------------------------------------------------------------

#define BSP_ARGS_VERSION    8

typedef struct tag_card_ver
{
	unsigned int card_addr;
	unsigned int ver;
	unsigned int rev;
	unsigned int config;
}card_ver;

typedef struct tag_ram_tst_result
{
	unsigned int address;
	unsigned int address_dest;
	unsigned int value_written;
	unsigned int value_read;
	unsigned int err;
}ram_tst_result;

typedef union
{
	struct
	{
		//PRM_RSTST
		unsigned long prm_rstst		: 16;//low part of register
		//RTC_STATUS_REG
		unsigned long rtc_power_up	: 1; //1<<7	
		unsigned long rtc_alarm		: 1; //1<<6
		unsigned long rtc_event		: 1; //any 1d_event | 1h_event | 1m_event | 1s_event
		//RTC_INTERRUPTS_REG
		unsigned long rtc_it_timer	: 1; //1<<2
		unsigned long rtc_it_alarm	: 1; //1<<3
		unsigned long rtc_rsrv		: 3;
		//STS_HW_CONDITIONS
		unsigned long sts_pwon		: 1; //1<<0
		unsigned long sts_warmreset	: 1; //1<<3

		//STS_BOOT
		unsigned long watchdog_reset : 1;	//1<<5 
		//PWR_ISR1
		unsigned long isr_pwron		: 1;	//1<<0
		unsigned long isr_rtc_it	: 1;	//1<<3
		unsigned long aut_pin		: 1;	//automotive input
		unsigned long PoB			: 1;
		unsigned long overheat		: 1;
	};
	unsigned long bits;
}hw_boot_status;

typedef struct
{
	hw_boot_status HwBootStatus;
	UINT32         ThermalShutdownStatus;
}hw_boot_status_ex;

typedef struct
{
	IOCTL_HAL_GET_BSP_VERSION_OUT	Version;
	UINT32							BuildVersion;
}Sw_Version;
typedef struct {
    OAL_ARGS_HEADER header;				// 0
    BOOL updateMode;                    // 8 Should IPL run in update mode?
    BOOL coldBoot;                      // 12 Cold boot (erase registry)?
    UINT32 deviceID;                    // 16 Unique ID for development platform
    UINT32 imageLaunch;                 // 20 Image launch address
    OAL_KITL_ARGS kitl;                 // 24 KITL parameters
    BOOL bHiveCleanFlag;                // 64 TRUE = Clean hive at boot
	BOOL bFormatPartFlag;               // 68 TRUE = Format partion at boot
	UINT32 SwUpdateFlags;				// 72 SW updating flags (nk.bin, xips etc)
	UINT32 identLCD;					// 76 obsolet
	BYTE	ManufBlock[MANUFACTURE_BLOCK_SIZE];// 80 the copy of MANUFACTURE BLOCK
	IOCTL_HAL_GET_BSP_VERSION_OUT	EbootVersion;// 1104
	IOCTL_HAL_GET_BSP_VERSION_OUT	XldrVersion; // 1116
	card_ver main_board;		// 1128 card addr is 2
	card_ver display_board;		// 1144 card addr is 0
	card_ver wlan_board;		// 1160 card addr is 4
	card_ver canbus_board;		// 1176 card addr is 3
	ram_tst_result	ram_test_res; // 1192
	UINT8	RndisMacAddress[6];	// 1212
	UINT8	RndisMacPAD[2];		// 1218
    UINT32 oalFlags;                    // OAL flags
	hw_boot_status		hw_stat;
	card_ver gps_board;         // card addr is 6
	card_ver modem_board;       // card addr is 5
	UINT16	EthMacAddress[3];
	card_ver extaudio_board;         // card addr is 7
	Sw_Version	EbootVersionExt;		
	Sw_Version	XldrVersionExt;
	UINT16	WiFiMacAddress[3];
	UINT16  CdsParams[8];      //CPU Dynamic Speed Feature state and temperature thresholds/ Maximal frequency allowed by CDS in MHz
	UINT32  ThermalShutdown;   //State of bit 1 - Register G indicating Last Shutdown cause is MPU Thermal Shutdown
} BSP_ARGS;

//------------------------------------------------------------------------------
//--------------------------------------------
// Manufacture block structure and data
//---------------------------------------------
#define RECORD_LENGTH              48          

typedef struct tag_manufacture_block
{
	unsigned char PCB_catalog_number[RECORD_LENGTH];			// PCB part number, for production line use
	unsigned char PCB_identification[RECORD_LENGTH];		     // PCB configuration string for identifying the board, for production line use
	unsigned char PCB_serial_number[RECORD_LENGTH];			    // For production line use 
	unsigned char device_catalog_number[RECORD_LENGTH];			// MDT part number, printedon the device label 
	unsigned char device_identification[RECORD_LENGTH];		    // MDT configuration string for identifying the product. 
	                                                            //The system uses this string in startup for loading needed drivers 
	unsigned char device_serial_number [RECORD_LENGTH];			// MDT unique serial number of 6 digits, printed on the device label 

	unsigned char reserved[MANUFACTURE_BLOCK_SIZE-6*RECORD_LENGTH];
}manufacture_block;


#endif
