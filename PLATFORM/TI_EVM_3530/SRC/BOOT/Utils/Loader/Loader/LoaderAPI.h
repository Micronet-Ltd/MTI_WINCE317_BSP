#ifndef _BOOT_LOADER
#define _BOOT_LOADER

#include "bootcmd.h"
#include "types.h"

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LOADER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LOADER_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef __cplusplus
	extern "C" {
#endif
#ifdef LOADER_EXPORTS
#define LOADER_API __declspec(dllexport)
#else
#define LOADER_API __declspec(dllimport)
#endif



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
	FERROR_GENERAL

};


enum MethoId {PCID_Setup, PCID_DownLoad, PCID_Burn,PCID_Lauch};

enum CallbackReason 
{
	PCRS_SetupComplete,//Setup operation completed, position and overallSize are ignored
	PCRS_SetupFailed,  //Setup operation failed, position and overallSize are ignored
	PCRS_Progress,     //Indicates progress of opetations
	PCRS_Removed       //Setup operation failed, position and overallSize are ignored
};



#define IP_STRING_LENGTH 36

struct SProgressCallbackParams
{
	

	//Device remote IP address.
	char  deviceIPSTR[IP_STRING_LENGTH];

	//Method ID which called to the call back function
	//Possible values: PCID_Setup,PCID_Program,PCID_Verify,PCID_RAMTest,PCID_DOCFormat
	int methodId;
	
	//Reason The callback was called
	////Possible values: PCRS_SetupComplete,PCRS_SetupFailed,PCRS_Progress
	int reason;
	
	//Position of operation (in bytes) 
	int position;
	
	//Size of media (in bytes)
	int overallSize;
};



typedef void (*ProgressCallback)(SProgressCallbackParams *pParams);

/*
Function Name:         LoaderSetup

Description:           This function startups the server. Its asynchronous function and it
                       returns immediately.  It starts the connection thread which is 
					   waiting for clients connection messages. When connection is established, 
					   the user callback function is called and the IP address of the connected 
					   client is passed as one of the callback function parameter. 

Parameters:            The user callback function which will be called when connection 
                       is established or broken.

Return Values:          FOK, FERROR_ALREADYOPEN, FERROR_SETUPFAILURE

*/

int LOADER_API LoaderSetup(ProgressCallback pfnProgressCallback);


/*
Function Name:         LoaderDisconnect

Description:           This function closes all the connections and terminates server's 
                       communication thread. Its synchronous function and it returns when 
					   connection is closed and thread is terminated.  

Parameters:             None

Return Values:          FOK, FERROR_GENERAL;

*/
int LOADER_API LoaderDisconnect();


/*
Function Name:         LoaderProgram

Description:           This function downloads the given bin file into the device RAM 
                       over the BOOT Interface communication layer and then burns it 
					   into the flash. The bin file may be one of the following: 
					   nk.bin/xldr.bin/eboot.bin

Parameters:           IN. binFile -  the file to be burned into the flash
                      IN. ProgressCallback – this function is called during the 
					  download process (each succesfull /unsuccesfull loader packet) 
					  and once when burning process is completed.
                      IN. deviceIP – The IP address of the device to send the cmd to.
					  At the first stage this argument is not relevant and even may be set to NULL.

Return Values:        FOK, FERROR_BADBINFILE, FERROR_FLASHERROR, FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,
                      FERROR_NOTOPENED,FERROR_NOTCONNECTED




*/


int LOADER_API LoaderProgram(const char* binFile, ProgressCallback pfnProgressCallback,char* deviceIP);



/*
Function Name:         LoaderOSLaunch

Description:           This function reads the existing OS Image from the flash 
                       and then launches it.

Parameters:           IN. deviceIP – The IP address of the device to send the command to. 
                      At the first stage this argument is not relevant and even may be set to NULL

Return Values:        FOK, FERROR_NOT_BURNED, FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,
                      FERROR_NOTOPENED,FERROR_NOTCONNECTED


*/

int LOADER_API LoaderOSLaunch(char* deviceIP);




/*
Function Name:         LoaderGetParamBlock

Description:           This function reads the Manufacture Parameters Block from flash memory. 
                       The Manufacture parameters block resides in the second last sector 
					   of the last boot block.

Parameters:           OUT: *prm – pointer to the parameter block to get the data.
                      IN. deviceIP – The IP address of the device to send the 
					  command to. At the first stage this argument is not relevant and even 
					  may be set to NULL.

Return Values:        FOK, FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,
                      FERROR_NOTCONNECTED, FERROR_FLASHERROR


*/

int LOADER_API LoaderGetParamBlock(manufacture_block *prm, char* deviceIP);

/*
Function Name:         LoaderSetParamBlock

Description:           This function burns the Manufacture Parameters Block into the flash memory. 
                       The Manufacture parameters block resides in the second last sector of the 
					   last boot block.

Parameters:            IN: *prm – pointer to the parameter block to be burned .
                       IN. deviceIP – The IP address of the device to send the command to. At
					   the first stage this argument is not relevant and even may be set to NULL.

Return Values:         FOK, FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,
                       FERROR_NOTCONNECTED, FERROR_FLASHERROR
*/

int LOADER_API LoaderSetParamBlock(manufacture_block *prm, char* deviceIP);


/*
Function Name:         LoaderGetSWRights

Description:           This function reads the SW rights block from the flash memory.
                       The SW rights block resides in the third last sector of the 
					   last boot block.

Parameters:            OUT: *pswr – pointer to the sw rights block to get the data 
                       from the  flash.
                       IN. deviceIP – The IP address of the device to send the 
					   command to. At the first stage this argument is not 
					   relevant and even may be set to NULL.

Return Values:        FOK, FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,
                      FERROR_NOTCONNECTED, FERROR_FLASHERROR
*/


int LOADER_API LoaderGetSWRights(SWRightsBlock *pswr, char* deviceIP);


/*
Function Name:         LoaderSetSWRights

Description:          This function burns the SW rights block into the flash memory.
                      The SW rights block resides in the third last sector of the 
					  last boot block.

Parameters:           IN: * pswr – pointer to the SW rights block to be burned .
                      IN. deviceIP – The IP address of the device to send the command to. 
					  At the first stage this argument is not relevant and even may be set to NULL.

Return Values:        FOK, FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,FERROR_NOTCONNECTED, 
                      FERROR_FLASHERROR
*/

int LOADER_API LoaderSetSWRights(SWRightsBlock *pswr, char* deviceIP);



/*
Function Name:         LoaderGetVersion

Description:           This function gets the given component version. 
                       The component is one of the following: IMG/EBOOT/XLDR according 
					   to the type argument. The version information includes component major,
					   minor and incremental version and it build date.

Parameters:           OUT: *  pVersionInfo - Pointer to the version information info.
                      It includes major,minor,incremental component version and its build date.
                      IN: type (VERSION_XLDR=0, VERSION_EBOOT, VERSION_IMG)  - what component version we want to get                            

                      IN. deviceIP – The IP address of the device to send the command to. 
					  At the first stage this argument is not relevant and even may be set to NULL.

Return Values:  FOK,  FERROR_VERSION_NOT_FOUND FERROR_IMAGE_NOT_BURNED FERROR_NETFAILURE,
                     FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,FERROR_NOTCONNECTED.


*/


int LOADER_API LoaderGetVersion(VersionInfo *pVersionInfo,VersionType type,char* deviceIP);
int LOADER_API LoaderGetVersionExt(VersionInfoExt *pVersionInfo,VersionType type,char* deviceIP);

/*
Function Name:         LoaderGetHWVersion

Description:           This function gets the CPU and Companion Chip versions. 

Parameters:           OUT: *  pHWVersionInfo - Pointer to the hw version information info.
                      It includes CPU and Companion Chip versions.
                  
                      IN. deviceIP – The IP address of the device to send the command to. 
					  At the first stage this argument is not relevant and even may be set to NULL.

Return Values:       FOK,  FERROR_VERSION_NOT_FOUND, FERROR_IMAGE_NOT_BURNED,FERROR_NETFAILURE,
                     FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,FERROR_NOTCONNECTED.


*/


int LOADER_API LoaderGetHWVersion (HWVersions *pHWVersionInfo, char* deviceIP);


/*
Function Name:         LoaderFormatDataPartition

Description:           This function formats data partition. The boot sets the flag pArgs->bFormatPartFlag = TRUE, 
                       when getting CMD_FORMAT_PART command. This flag resides in BOOT<-> OS Image shared memory 
					   at physical address 0x80000000 + offset(bFormatPartFlag). 
					   The OS Image checks this flag at startup and formats Data partition if its set. 
					   So this function should be used with conjunction with LoaderOSLaunch() function 
					   to allow OS image to format Data Partition. 

Parameters:            IN. deviceIP – The IP address of the device to send the command to. 
                       At the first stage this argument is not relevant and even may be set to NULL.

Return Values:         FOK,  FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,FERROR_NOTCONNECTED.

*/

int LOADER_API LoaderFormatDataPartition (char* deviceIP);

/*
Function Name:         LoaderCleanRegistry

Description:           This function clean registry back to factory. 
                       The boot sets the flag pArgs->bHiveCleanFlag = TRUE, 
					   when getting CMD_ERASE_REGISTRY command. 
					   This flag resides in BOOT<-> OS Image shared memory at physical 
					   address 0x80000000 + offset(bHiveCleanFlag). The OS Image checks 
					   this flag at startup and clean registry if this flag is set. 
					   So this function should be used with conjunction with LoaderOSLaunch()
					   function  to allow OS image to clean the registry. 

Parameters:            IN. deviceIP – The IP address of the device to send the command to. 
                       At the first stage this argument is not relevant and even may be set to NULL.

Return Values:         FOK,  FERROR_NETFAILURE,FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,FERROR_NOTCONNECTED.

*/
int LOADER_API LoaderCleanRegistry (char* deviceIP);

int LOADER_API LoaderGetFlashInfo(FlashInformation* pFlashInfo, char* deviceIP);
int LOADER_API LoaderGetPartInfo(PARTENTRY* pPartInfo, char* deviceIP);

int LOADER_API	LoaderDumpStore(ProgressCallback pfnProgressCallback, const char* FileName, const FlashInformation*	pFlashInfo, unsigned long dp_offset,char* deviceIP);
int LOADER_API	LoaderUpdateBmp(ProgressCallback pfnProgressCallback, const char* pPartInfo, char* deviceIP);





#if 0

//This function  downloads the os bin file into the device RAM and then launches it.
//Returns code of failure:
//FOK, FERROR_BADBINFILE, FERROR_IMAGE_NOT_BURNED, FERROR_NETFAILURE FERROR_NOT_SUPPORTED,FERROR_NOTOPENED,FERROR_NOTCONNECTED
int LOADER_API LoaderOSLoad(const char* binFile, ProgressCallback pfnProgressCallback,char* deviceIP);
#endif



#ifdef __cplusplus
	}
#endif
#endif 