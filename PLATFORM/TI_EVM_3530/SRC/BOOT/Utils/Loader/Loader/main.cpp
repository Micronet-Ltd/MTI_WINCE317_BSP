// Loader.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Loader.h"


#ifdef _MANAGED
  #pragma managed(push, off)
#endif


CLoader *pAppBootLoader = NULL;

int AttachCounter = 0;


BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			if(!pAppBootLoader)
			{

				AttachCounter++;
				wsprintf(text,__T("AttachCounter attach %d \r\n"),AttachCounter);
				PRINT(text);

				pAppBootLoader = new CLoader;
				if(!pAppBootLoader)
					return FALSE;

			}
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			if(pAppBootLoader)
			{
				//char buf[100];

				AttachCounter--;

				wsprintf(text,__T("AttachCounter detach %d \r\n"),AttachCounter);
				PRINT(text);

				if(AttachCounter == 0 && pAppBootLoader)
				{
					delete pAppBootLoader;
					pAppBootLoader = 0;
				}
			}
			break;
		}
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		default:
			break;

    }

    return TRUE;
}





// LOADER APIs


//--------------------------------------------------------------------------------------------------------
int LoaderSetup(ProgressCallback pfnProgressCallback)
{
	if(pAppBootLoader)
		return pAppBootLoader->SetupConnection(pfnProgressCallback);
	else
		return FERROR_GENERAL;
}


//--------------------------------------------------------------------------------------------------------

int LoaderGetHWVersion(HWVersions *pHWVersionInfo,char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderGetHWVersion(pHWVersionInfo,deviceIP);
	else
		return FERROR_GENERAL;

}

//--------------------------------------------------------------------------------------------------------

int LoaderGetVersion(VersionInfo *pVersionInfo,VersionType type,char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderGetVersion(pVersionInfo,type,deviceIP);
	else
		return FERROR_GENERAL;

}




//--------------------------------------------------------------------------------------------------------

int LoaderProgram(const char* binFile, ProgressCallback pfnProgressCallback,char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderProgram(binFile, pfnProgressCallback,deviceIP);
	else
		return FERROR_GENERAL;

}

//--------------------------------------------------------------------------------------------------------

int LoaderOSLoad(const char* binFile, ProgressCallback pfnProgressCallback,char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderOSLoad(binFile, pfnProgressCallback,deviceIP);
	else
		return FERROR_GENERAL;

}


//--------------------------------------------------------------------------------------------------------

int LoaderGetParamBlock(manufacture_block *prm, char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderGetParamBlock(prm,deviceIP);
	else
		return FERROR_GENERAL;

}

//--------------------------------------------------------------------------------------------------------

int LoaderSetParamBlock(manufacture_block *prm, char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderSetParamBlock(prm,deviceIP);
	else
		return FERROR_GENERAL;

}



//--------------------------------------------------------------------------------------------------------

int LoaderGetSWRights(SWRightsBlock *pswr, char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderGetSWRights(pswr,deviceIP);
	else
		return FERROR_GENERAL;

}

//--------------------------------------------------------------------------------------------------------

int LoaderSetSWRights(SWRightsBlock *pswr, char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderSetSWRights(pswr,deviceIP);
	else
		return FERROR_GENERAL;

}


//--------------------------------------------------------------------------------------------------------
int LoaderOSLaunch(char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderOSLaunch(deviceIP);
	else
		return FERROR_GENERAL;

}


//--------------------------------------------------------------------------------------------------------
int LoaderFormatDataPartition(char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderFormatDataPartition(deviceIP);
	else
		return FERROR_GENERAL;

}


//--------------------------------------------------------------------------------------------------------
int LoaderCleanRegistry(char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderCleanRegistry(deviceIP);
	else
		return FERROR_GENERAL;

}



//--------------------------------------------------------------------------------------------------------
int LoaderDisconnect()
{
	if(pAppBootLoader)
		return (pAppBootLoader->CloseConnection()==TRUE)? FOK:FERROR_GENERAL;
	else
		return FERROR_GENERAL;
	
}

//--------------------------------------------------------------------------------------------------------
int LoaderGetFlashInfo(FlashInformation* pFlashInfo, char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderGetFlashInfo(pFlashInfo, deviceIP);
	else
		return FERROR_GENERAL;
}
//--------------------------------------------------------------------------------------------------------
int LoaderGetPartInfo(PARTENTRY* pPartInfo, char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderGetPartInfo(pPartInfo, deviceIP);
	else
		return FERROR_GENERAL;
}
//--------------------------------------------------------------------------------------------------------

int LoaderDumpStore(ProgressCallback pfnProgressCallback, const char* FileName, const FlashInformation* pFlashInfo, unsigned long dp_offset,char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderDumpStore(pfnProgressCallback, FileName, pFlashInfo, dp_offset, deviceIP);
	else
		return FERROR_GENERAL;

}
int LoaderUpdateBmp(ProgressCallback pfnProgressCallback, const char* FileName, char* deviceIP)
{
	if(pAppBootLoader)
		return pAppBootLoader->LoaderUpdateBmp(pfnProgressCallback, FileName, deviceIP);
	else
		return FERROR_GENERAL;

}
#ifdef _MANAGED
 #pragma managed(pop)
#endif

