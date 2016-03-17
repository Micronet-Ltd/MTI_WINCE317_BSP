#ifndef _SWUPDATE_API_H
#define _SWUPDATE_API_H

typedef struct 
{
    DWORD   dwVersionMajor;
    DWORD   dwVersionMinor;       
    DWORD   dwVersionIncremental;       
} BspVersion;
typedef struct 
{
    DWORD   dwVersionMajor;
    DWORD   dwVersionMinor;       
    DWORD   dwVersionIncremental;       
	DWORD	dwVersionBuild;
} BspVersionExt;


typedef enum
{
	QUERY_MAIN_CFG = 1,
	QUERY_LCD_CFG,
	QUERY_WLAN_CFG,
	QUERY_CAN_CFG,
	QUERY_GPS_CFG,
	QUERY_MODEM_CFG,
	QUERY_HW_BOOT_STATUS,
	QUERY_EXTAUDIO_CFG
} HW_CONFIG_QUERY;

typedef struct
{
	unsigned int CardAddr;
	unsigned int Ver;
	unsigned int Rev;
	unsigned int Config;
}CardVer;

#ifdef __cplusplus
	extern "C" {
#endif

//------------------------------------------------------------------------------
// Function name	: MIC_GetSwUpdateFlags
// Description	    : Gets Sw Update flags. 
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
// Arguments         : pointer to Flags 
//                   : Size of Flags 
//------------------------------------------------------------------------------
BOOL MIC_GetSwUpdateFlags( VOID* pFlags, UINT32 Size );

//------------------------------------------------------------------------------
// Function name	: MIC_SetSwUpdateFlags
// Description	    : Set Sw Update flags. 
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
// Arguments         : pointer to Flags 
//                   : Size of Flags 
//------------------------------------------------------------------------------
BOOL MIC_SetSwUpdateFlags( VOID* pFlags, UINT32 Size );

//------------------------------------------------------------------------------
// Function name	: MIC_GetBspSwVerstion
// Description	    : Get current versions. 
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
// Arguments        : sw type: bsp, eboot, xloader.
//                  : ptr BspVersion structure 
//------------------------------------------------------------------------------
BOOL	MIC_GetBspSwVerstion( UINT SwType, BspVersion* pVersion );

//------------------------------------------------------------------------------
// Function name	: MIC_GetBspVersString
// Description	    : Get current bsp image version string. 
// Return type		: If the function succeeds, 0 is returned; otherwise, required length of string 
// Arguments        : pVersionString - Location of destination string buffer
//                  : MaxLength		 - Size of the destination string buffer in words
//------------------------------------------------------------------------------
DWORD	MIC_GetBspVersString( WCHAR* pVersionString, DWORD MaxLength );



//------------------------------------------------------------------------------
// Function name	: BOOL	MIC_5VSetValue
// Description	    : Set 5V output. 
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
// Arguments        : bEnable: if TRUE - then 5V output is activated,otherwise it's deactivated .
//------------------------------------------------------------------------------
BOOL	MIC_5VSetValue( BOOL bEnable );

//---------------------------------------------------------------------------------------
HANDLE	MIC_LightSenseOpen(void *QueueName, UINT32 len);
void	MIC_LightSenseClose(HANDLE hContext);
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Function name	: MIC_IsHWExist
// Description	    : Get current HW component version or HW boot status 
// Return type		:  If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
// Arguments        : HW_CONFIG_QUERY CfgType, pBuff - Location of destination value, Size - location size
/*
QUERY_XXXX_CFG return value into pBuff:
CardVer structure of XXXX card. If card is not present, CardVer filled with 0xFFFFFFFF

QUERY_HW_BOOT_STATUS return value into pBuff:
bits of prm_rstst (PRM_RSTST register) 0-15 bits
0 GLOBAL_COLD_RST				Power-up (cold) reset event.
1 GLOBAL_SW_RST					Global software reset event.
2								RESERVED
3 SECURITY_VIOL_RST				Security violation reset event.
4 MPU_WD_RST					MPU watchdog reset event.
5 SECURE_WD_RST					Secure watchdog reset event.
6 EXTERNAL_WARM_RST				External warm reset event.
7 VDD1_VOLTAGE_MANAGER_RST VDD1 VDD1 voltage manager reset event.
8 VDD2_VOLTAGE_MANAGER_RST VDD2 VDD2 voltage manager reset event.
9								Don't pay attention	
								IcePick reset event. This is a source of warm reset initiated by the emulation.
10								Don't pay attention	
								IceCrusher reset event. This is a source of warm initiated by the emulation.
11-15							RESERVED
/////////////////////////////////////////////////
//RTC_STATUS_REG register: 16-18 bits
16 rtc_power_up					RTC is lost a power
17 rtc_alarm					An alarm occured
18 rtc_event					Any alarm event (Don't pay attention)
//RTC_INTERRUPTS_REG register: 19-20 bits
19 rtc_it_timer					Enable periodic alarm
20 rtc_it_alarm					Enable one alarm (when the alarm value is reached)
21-23							RESERVED
//STS_HW_CONDITIONS register: 24 - 25 bits
24 sts_pwon						PWRON button 
25 sts_warmreset				Warmreset
//STS_BOOT register: bit 26
26 watchdog_reset				
//PWR_ISR1 register: 27 - 30 bits
27 isr_pwron					PWRON signal (inverted)
28 isr_rtc_it					RTC event
29 aut_pin						automotive input
30 PoB							Power Box is present
31								RESERVED
/////////////////////////////////////////////////
typedef union
{
	struct
	{
		//PRM_RSTST
		unsigned long prm_rstst		: 16;//low part of register
		//RTC_STATUS_REG
		unsigned long rtc_power_up	: 1; 	
		unsigned long rtc_alarm		: 1; 
		unsigned long rtc_event		: 1; //any 1d_event | 1h_event | 1m_event | 1s_event
		//RTC_INTERRUPTS_REG
		unsigned long rtc_it_timer	: 1; 
		unsigned long rtc_it_alarm	: 1; 
		unsigned long rtc_rsrv		: 3;//reserved
		//STS_HW_CONDITIONS
		unsigned long sts_pwon		: 1; 
		unsigned long sts_warmreset	: 1; 

		//STS_BOOT
		unsigned long watchdog_reset : 1;
		//PWR_ISR1
		unsigned long isr_pwron		: 1;	
		unsigned long isr_rtc_it	: 1;	
		unsigned long aut_pin		: 1;	//automotive input
		unsigned long PoB			: 1;
		unsigned long rsrv			: 1; //reserved
	};
	unsigned long bits;
}hw_boot_status;
*/
BOOL 	MIC_IsHWExist(HW_CONFIG_QUERY CfgType, void* pBuff, DWORD Size );

//------------------------------------------------------------------------------
// Function name	: MIC_GetBspSwVersionExt
// Description	    : Get current versions. 
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
// Arguments        : sw type: bsp, eboot, xloader.
//                  : ptr BspVersion structure 
//------------------------------------------------------------------------------
BOOL	MIC_GetBspSwVersionExt( UINT SwType, VOID* pVersion, DWORD Size, DWORD* SizeOut );

#ifdef __cplusplus
	}
#endif	

/////////// SwUpdateFlags //////////////////////////////
#define	SW_START_UPDATE		0x00000001	// to start sw update
#define	SW_END_UPDATE		0x00000002	// update is ended: if successful - check errors
#define	SW_CANNOT_OPEN		0x00010000	// boot cannot open file
#define	SW_CANNOT_READ		0x00020000	// boot cannot read file
#define	SW_DATA_ERROR		0x00040000	// file data error
#define	SW_FLASH_ERROR		0x00080000

#define BSP_VERSION			1
#define	EBOOT_VERSION		2
#define	XLDR_VERSION		3
//
typedef enum
{
	LIGHT_UNSPEC,	//=	AMBIENT_LIGHT_UNSPEC,
	LIGHT_LOW,		//=	AMBIENT_LIGHT_LOW,
	LIGHT_NORMAL,	//=	AMBIENT_LIGHT_NORMAL,
	LIGHT_HIGH,		//=	AMBIENT_LIGHT_HIGH,
	LIGHT_INVALID,	//=	AMBIENT_LIGHT_INVALID
}LIGHT_LEVEL;

#endif
