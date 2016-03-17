#ifndef _SER_LOADER_API
#define _SER_LOADER_API

#define MAX_BUFFER_SIZE 4*1024 //4KB
#include "bootcmd.h"
#include "swupdate.h"

enum ReturnValues
{
	FOK=0,               //Operation was successful
	FERROR_NOTOPENED ,   //Retuned by System API loader functions if they was called before LoaderSetup or after LoaderDisconnect functions
	FERROR_ALREADYOPEN,  //Returned by LoaderSetup  if previous session was not closed
	FERROR_BADBINFILE,   //The binary file being downloaded is corrupted or not exists.
	FERROR_DOWNLOADERROR, // Error when downloading to the device RAM
	FERROR_FLASHERROR,   // ERROR when reading/burning flash
	FERROR_NETFAILURE,   //the network was disconnected for some reason
	FERROR_SETUPFAILURE, //network setup failed
	FERROR_VERSION_NOT_FOUND, //LoaderGetVersion function failed to retrieve version info
	FERROR_IMAGE_NOT_BURNED,  //Image not burned
	FERROR_BAD_PARAMETERS,
	FERROR_NOT_SUPPORTED,
	FERROR_NOTCONNECTED,
	FERROR_NET_FAILED,
	FERROR_GENERAL,
	FERROR_PARAM	= 0x100,
	FERROR_SIZE,
	FERROR_COMM,
	FERROR_READ,
	FERROR_WRITE,
	FERROR_READ_LEN,
	FERROR_WRITE_LEN,
	FERROR_SYSTEM,
	FERROR_TIMEOUT,
	FERROR_ACK,
	FERROR_OVERFLOW,
	FERROR_DATA,
	FERROR_FAILED,
	FERROR_CORRUPTED_CMD
};

#ifdef __cplusplus
	extern "C" {
#endif
#ifdef SERLOADER_EXPORTS
#define SERLOADER_API __declspec(dllexport)
#else
#define SERLOADER_API __declspec(dllimport)
#endif

void		  SERLOADER_API LdrSetWaitTimeout(HANDLE hPort, unsigned long Timeout);
unsigned long SERLOADER_API LdrGetWaitTimeout(HANDLE hPort);

unsigned long SERLOADER_API LdrOpen(unsigned int PortNum, HANDLE* phPort );
unsigned long SERLOADER_API LdrClose(HANDLE hPort);
unsigned long SERLOADER_API LdrConnect(HANDLE hPort, DWORD Param);

unsigned long SERLOADER_API LdrGetParamBlock(HANDLE hPort, manufacture_block* mb);
unsigned long SERLOADER_API LdrSetParamBlock(HANDLE hPort, manufacture_block* mb);

unsigned long SERLOADER_API LdrGetVersion(HANDLE hPort, VersionInfo* pVersInfo, VersionType type);
unsigned long SERLOADER_API LdrGetVersionExt(HANDLE hPort, VOID* pVersInfo, DWORD Size, VersionType type);
unsigned long SERLOADER_API LdrGetHWVersion(HANDLE hPort, HWVersions* pVersInfo);

unsigned long SERLOADER_API LdrGetEtherMac(HANDLE hPort, unsigned short mac[] );
unsigned long SERLOADER_API LdrSetEtherMac(HANDLE hPort, unsigned short mac[] );

unsigned long SERLOADER_API LdrGetWiFiMac(HANDLE hPort, unsigned short mac[] );
unsigned long SERLOADER_API LdrSetWiFiMac(HANDLE hPort, unsigned short mac[] );

unsigned long SERLOADER_API LdrGetRndisMac(HANDLE hPort, unsigned short mac[] );
unsigned long SERLOADER_API LdrSetRndisMac(HANDLE hPort, unsigned short mac[] );

unsigned long SERLOADER_API LdrGetSwRights(HANDLE hPort, unsigned char sr[] );
unsigned long SERLOADER_API LdrSetSwRights(HANDLE hPort, unsigned char sr[] );

unsigned long SERLOADER_API LdrGetMaxFreq(HANDLE hPort, UINT16 * freq );
unsigned long SERLOADER_API LdrSetMaxFreq(HANDLE hPort, UINT16 * freq );

unsigned long SERLOADER_API LdrFormatFlash(HANDLE hPort);
unsigned long SERLOADER_API LdrFormatDataPartition(HANDLE hPort);
unsigned long SERLOADER_API LdrCleanRegistry(HANDLE hPort);
unsigned long SERLOADER_API LdrEraseFlash(HANDLE hPort, unsigned int Params);

unsigned long SERLOADER_API LdrGetRamTestResult(HANDLE hPort, ram_tst_result* tr);
//unsigned long SERLOADER_API LdrGetFlashTest(HANDLE hPort, unsigned char sr[] );
unsigned long SERLOADER_API LdrBurnSDFile(HANDLE hPort, sw_type_data type);

unsigned long SERLOADER_API LdrReset(HANDLE hPort);
unsigned long SERLOADER_API LdrExitLoaderMode(HANDLE hPort);

unsigned long SERLOADER_API LdrGetNandInfo(HANDLE hPort, NAND_INFO* pInfo);

#ifdef __cplusplus
	}
#endif

#endif	//_SER_LOADER_API
