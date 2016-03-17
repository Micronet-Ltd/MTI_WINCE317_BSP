#ifndef _BOOT_CMD_H
#define _BOOT_CMD_H

typedef struct _CmdHeader
{
	USHORT  ProtID; // Protocol identifier ("EDBG" on the wire)
	USHORT  Cmd;    // CMD identifier
	USHORT  SeqNum; // For detection of dropped packets   
	USHORT  CmdAck; // ack for each cmd
	UCHAR   Data[2];// Cmd specific data starts here (format is determined by 
                    // Cmd, len is determined by UDP packet size)	
}tCmdHeader, *PtCmdHeader;



//--------------------------------------------
// Acknowledge types
//---------------------------------------------

enum AckType
{
	ACK_NO_ERROR=0,
	NO_ACK,                         

	ACK_WRONG_ARGUMENT,             
	ACK_ERROR_FLASH, 			
	ACK_UNKNOWN_CMD,     			
	ACK_CORRUPTED_CMD,    			
    ACK_IMAGE_NOT_BURNED,
	ACK_VERSION_NOT_FOUND,
	ACK_GENERAL_ERROR,                
	ACK_PARTITION_ERROR,
	ACK_BLOCK_STATUS_BAD,
	ACK_NOT_EXIST,
	ACK_WRONG_SIZE,
	ACK_NOT_IMPLEMENTED=100
};


//--------------------------------------------
// Loader protocol identifier
//---------------------------------------------
#define LDR_PROTOCOL_ID     33

//--------------------------------------------
// Boot interface (cmds)
//---------------------------------------------

#define CMD_FLASH_BIN        1
#define CMD_FORMAT_PART      2
#define CMD_GET_VERSION		 3
#define CMD_BIN_DOWNLOAD	 4
#define CMD_IMG_RAM_LAUNCH	 5
#define CMD_IMG_FLASH_LAUNCH 6
#define CMD_GET_MANUFACTURE	 7
#define CMD_SET_MANUFACTURE	 8
#define CMD_STOP_BOOTME      9
#define CMD_SEND_BOOTME      11
#define CMD_ERASE_REGISTRY   12
#define CMD_GET_HW_VERSION   13
#define CMD_SET_SW_RIGHTS    14
#define CMD_GET_SW_RIGHTS    15
#define CMD_GET_FLASH_INFO		16
#define CMD_GET_PART_INFO		17
#define CMD_GET_STORE_DUMP		18
#define CMD_BURN_BITMAP			19
#define CMD_GET_RNDIS_MAC		20
#define CMD_SET_RNDIS_MAC		21
#define CMD_GET_ETHER_MAC		22
#define CMD_SET_ETHER_MAC		23
#define	CMD_RESET_DEVICE		24
#define	CMD_FORMAT_FLASH		25
#define CMD_GET_RAM_TEST_RESULT	26
#define CMD_GET_FLASH_TEST		27
#define CMD_BURN_SD_FILE		28
#define CMD_GET_VERSION_EXT		29
#define CMD_GET_WIFI_MAC		30
#define CMD_SET_WIFI_MAC		31
#define	CMD_ERASE_FLASH			32
#define	CMD_GET_MAX_FREQ		33
#define	CMD_SET_MAX_FREQ		34
#define CMD_GET_NAND_INFO		35

//--------------------------------------------
// Manufacture block structure and data
//---------------------------------------------
#define MANUFACTURE_BLOCK_SIZE     1024
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



//--------------------------------------------
// SW rights structure and data
//---------------------------------------------
#define SW_RIGHTS_BLOCK_SIZE           96
#define RECORD_SW_RIGHTS_LENGTH        96          



typedef struct tag_SWRightsBlock
{
	unsigned char ApplicationsLicense[RECORD_SW_RIGHTS_LENGTH];

}SWRightsBlock;


//--------------------------------------------------------------------
// HW (CPU and Companion chip) vesrions for CMD_GET_HW_VERSION cmd
//--------------------------------------------------------------------

typedef struct tag_HWVersions
{
	// 0 = "ES1.0", 1 = "ES2.0", 2 = "ES2.1", 3 = "ES2.0, ID determined using CRC", 4 = "ES2.1, ID determined using CRC", 0xFF = UNKNOWN
	UINT8 CPUVersion;
	// 0 = "ES1.0" ,0x10 = "ES2.x", 0x40 = "ES3.0" ,0x50 = "ES3.1", 0xFF = "Unknown"
	UINT8 CoChipVersion;
}HWVersions;





//--------------------------------------------
// LDR protocol specific data
//--------------------------------------------

#define LDR_PORT             990
#define MAX_LDR_MSG_LENGTH   1460
#define MAX_LDR_DATA_LENGTH		(MAX_LDR_MSG_LENGTH - sizeof(tCmdHeader) + 2)


//---------------------------------------------------------------
// Version Type  and Version Data for CMD_GET_VERSION interface
//---------------------------------------------------------------
enum VersionType
{
	VERSION_XLDR=0,          
	VERSION_EBOOT, 
	VERSION_IMG
};

#define BUILD_DATE_STR_LENTGH	20
#define VERS_STRING_LEN			64


typedef struct tag_VersionInfo
{
	///Version Info....
	UINT32 majorVersion;
	UINT32 minorVersion;
	UINT32 incrementalVersion;
	char   buildDate[BUILD_DATE_STR_LENTGH];
	WCHAR  versionString[VERS_STRING_LEN];
}VersionInfo;

typedef struct tag_VersionInfoExt
{
	VersionInfo	Version;
	UINT32		BuildVersion;
}VersionInfoExt;

#define		NUM_PARTS		4

#define		DUMP_START		1
#define		DUMP_STOP		2

typedef struct tag_DumpParams
{
	UINT32 DumpOffset;
	UINT32 DumpSize;
	UINT32 DumpFlag;

}DumpParams;

typedef struct _FlashInformation
{
    DWORD		flashType;
    DWORD       dwNumBlocks;
    DWORD       dwBytesPerBlock;
    WORD        wSectorsPerBlock;
    WORD        wDataBytesPerSector;
	DWORD		dwNumOfBadBlocks;

}FlashInformation, *PFlashInformation;


#pragma pack( push, 1 )
typedef struct _ProtocolHeader
{
	unsigned long HeaderSize;
	unsigned long Cmd;
	unsigned long Ack;
	unsigned long fContinueFlag;
	unsigned long DataLen;
	unsigned long Param;
	unsigned long DataCS;
	unsigned long HeaderCS;
} ProtocolHeader;
#pragma pack()

typedef struct tag_ram_tst_result
{
	unsigned int address;
	unsigned int address_dest;
	unsigned int value_written;
	unsigned int value_read;
	unsigned int err;
}ram_tst_result;
/* moved to swupdate.h
typedef enum
{
	sw_type_xldr	= 0x01,
	sw_type_eboot	= 0x02,
	sw_type_mnfr	= 0x04,
	sw_type_logo	= 0x08,
	sw_type_nk		= 0x10
} sw_type_data;
*/
//  NAND Geometry info structure
typedef struct {
    UINT8  manufacturerId;
    UINT8  deviceId;
    UINT32 blocks;
    UINT32 sectorsPerBlock;
    UINT32 sectorSize;
    BOOL   wordData;
} NAND_INFO;

#endif