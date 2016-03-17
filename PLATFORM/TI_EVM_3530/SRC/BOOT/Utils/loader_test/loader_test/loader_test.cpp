// loader_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"



HANDLE hConnEvent;



//void __stdcall SetupCallback(SProgressCallbackParams *pParams)
void SetupCallback(SProgressCallbackParams *pParams)
{

	printf ("SetupC: MethodID= %x, reason = %x, position = %x,overallSize = %x \r\n", pParams->methodId, pParams->reason,pParams->position,pParams->overallSize);


	if (pParams->methodId ==PCID_Setup && pParams->reason ==PCRS_SetupComplete)
	{

		printf ("Connection is established. Event is signalled\r\n");
		SetEvent(hConnEvent);
	}
	
}


void OperationCallback(SProgressCallbackParams *pParams)
{

	printf ("OperationC: deviceIPSTR= %s \r\n", pParams->deviceIPSTR);
	printf ("OperationC: MethodID= %x, reason = %x, position = %x,overallSize = %x \r\n", pParams->methodId, pParams->reason,pParams->position,pParams->overallSize);
	
}




int _tmain(int argc, _TCHAR* argv[])
{

	DWORD ret;


	hConnEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	ret=LoaderSetup(SetupCallback);
	
	printf("Start Connection \r\n");
	DWORD dwReason = WaitForSingleObject(hConnEvent, INFINITE);
	printf ("Connected... \r\n");
	//Sleep(10000);

//	ret = LoaderProgram("C:\\WINCE600\\PLATFORM\\TI_EVM_3530\\target\\ARMV4I\\retail\\EBOOTNAND.bin",OperationCallback,NULL);
//	ret = LoaderProgram("C:\\WINCE600\\PLATFORM\\TI_EVM_3530\\target\\ARMV4I\\retail\\XLDRNAND.bin",NULL,NULL);
//	ret = LoaderProgram("C:\\WINCE600\\OSDesigns\\EVM_3530\\EVM_3530\\RelDir\\TI_EVM_3530_ARMV4I_Release\\nk.bin",NULL,NULL);
//	Sleep(1000);
//	ret = LoaderOSLoad("C:\\WINCE600\\OSDesigns\\EVM_3530\\EVM_3530\\RelDir\\TI_EVM_3530_ARMV4I_Release\\nk.bin",NULL,NULL);

/*	
	VersionInfo versionInfo;
	ret = LoaderGetVersion(&versionInfo,VERSION_EBOOT,NULL);
	ret = LoaderGetVersion(&versionInfo,VERSION_XLDR,NULL);
	ret = LoaderGetVersion(&versionInfo,VERSION_IMG,NULL);
//	ret = LoaderCleanRegistry(NULL);
	ret = LoaderFormatDataPartition(NULL);
	ret = LoaderOSLaunch(0);

    manufacture_block manuf_block;
	memset (manuf_block.PCB_catalog_number,61,RECORD_LENGTH);
	memset (manuf_block.PCB_identification,62,RECORD_LENGTH);
	memset (manuf_block.PCB_serial_number,63,RECORD_LENGTH);
	memset (manuf_block.device_catalog_number,64,RECORD_LENGTH);
	memset (manuf_block.device_identification,65,RECORD_LENGTH);
	memset (manuf_block.device_serial_number,66,RECORD_LENGTH);


	ret = LoaderSetParamBlock(&manuf_block ,NULL);
	ret = LoaderGetParamBlock(&manuf_block ,NULL);

	ret = LoaderProgram("C:\\WINCE600\\PLATFORM\\TI_EVM_3530\\target\\ARMV4I\\retail\\EBOOTNAND.bin",OperationCallback,NULL);
    ret = LoaderGetParamBlock(&manuf_block ,NULL);

	SWRightsBlock swRights;

	memset (swRights.ApplicationsLicense,71,RECORD_SW_RIGHTS_LENGTH);

	ret = LoaderSetSWRights(&swRights,NULL);

	ret = LoaderGetSWRights(&swRights,NULL);

	ret = LoaderFormatDataPartition(NULL);
	ret = LoaderCleanRegistry(NULL);


	HWVersions hwVersionInfo;
	ret = LoaderGetHWVersion(&hwVersionInfo,NULL);
*/


	
//	ret =  LoaderOSLaunch(NULL);
//	ret = LoaderProgram("C:\\WINCE600\\PLATFORM\\TI_EVM_3530\\target\\ARMV4I\\retail\\EBOOTNAND.bin",OperationCallback,NULL);
//	ret = LoaderOSLoad("C:\\WINCE600\\OSDesigns\\EVM_3530\\EVM_3530\\RelDir\\TI_EVM_3530_ARMV4I_Release\\nk.bin",NULL,NULL);

	FlashInformation		finfo;
	ret = LoaderGetFlashInfo(&finfo,NULL);
	PARTENTRY pinfo[NUM_PARTS]		= {0};
	ret = LoaderGetPartInfo(pinfo,NULL);
	//while(1);
	DWORD offset = (pinfo[0].Part_StartSector + pinfo[0].Part_TotalSectors) * finfo.wDataBytesPerSector;
	ret = LoaderDumpStore( 0, _T("c:\\tmp\\dump.dat"), &finfo, offset,	0);
//	ret = LoaderLoadBinFromFlash( _T("nk.bin"), 0);
	_getchar_nolock();
	ret = LoaderDisconnect();
	//while(1);
    return 0;

}

