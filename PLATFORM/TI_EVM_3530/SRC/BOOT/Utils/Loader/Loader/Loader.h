#ifndef _LOADER_H____
#define _LOADER_H____
#include <windows.h>
#include <tchar.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Iphlpapi.h>
#include "BootCmd.h"
#include "LoaderAPI.h"
#include "NetAdapter.h"

#include "types.h"


//////////////////////////////////////////////////////////////////////////////////////////
//	Desc:
//		Class wrapper for a single network adapter.  A listing of these adapters
//		can be built using the EnumNetworkAdapters(...) function prototyped
//		above.
//////////////////////////////////////////////////////////////////////////////////////////
class CLoader {

public:
		CLoader();
		~CLoader();

		int SetupConnection(ProgressCallback pfnProgressCallback);
		BOOL  CloseConnection();

		int EnumNetworkAdapters( NetAdapter* lpBuffer, ULONG ulSzBuf, LPDWORD lpdwOutSzBuf );
		BOOL  BuildAdapterListing();

		int LoaderProgram(const char* binFile, ProgressCallback pfnProgressCallback,char* deviceIP);
		int LoaderOSLoad(const char* binFile, ProgressCallback pfnProgressCallback,char* deviceIP);

		int LoaderGetHWVersion(HWVersions *pHWVersionInfo,char* deviceIP);
		int LoaderGetVersion(VersionInfo *pVersionInfo,VersionType type,char* deviceIP);

		int LoaderGetParamBlock(manufacture_block *prm,char* deviceIP);
		int LoaderSetParamBlock(manufacture_block *prm,char* deviceIP);

		int LoaderSetSWRights(SWRightsBlock *pswr,char* deviceIP);
		int LoaderGetSWRights(SWRightsBlock *pswr,char* deviceIP);


		int LoaderFormatDataPartition(char* deviceIP);
		int LoaderCleanRegistry  (char* deviceIP);

		int LoaderOSLaunch(char* deviceIP);

		int LoaderGetFlashInfo(FlashInformation* pFlashInfo, char* deviceIP);
		int LoaderGetPartInfo(PARTENTRY* pPartInfo, char* deviceIP);

		int LoaderDumpStore(	ProgressCallback pfnProgressCallback, 
								const char*		FileName, 
								const FlashInformation*	pFlashInfo, 
								unsigned long		dp_offset,
								char*				deviceIP);

		int LoaderGetSector(	char* TxBuff, char* RxBuff, char* SectorBuff, DumpParams* params, char* deviceIP);
		
		int LoaderUpdateBmp(ProgressCallback pfnProgressCallback, const char* FileName, char* deviceIP);
		
		static DWORD WINAPI DoConnect(LPVOID lpParam);

		int		TestAdapter(char* deviceIP);
		void	SetProgress(ProgressCallback pfnProgressCallback, SProgressCallbackParams Params );//send copy of params

private:
	
	ProgressCallback m_pfnProgressCallback; // just for connection setup callbacks

	NetAdapter*      m_pAdapters;        // pointers to the array of network adapters
    DWORD            m_nAdaptersCount;
	HANDLE           m_ConnectThread;
	HANDLE           m_QuitConnEvent;

	CRITICAL_SECTION	m_AdapterCritS; // in the future it should be the list: 1 crit section for each adapter.

};

/////////////////////////////////////////////
// Function Prototypes
DWORD EnumNetworkAdapters( CLoader* lpBuffer, ULONG ulSzBuf, LPDWORD lpdwOutSzBuf );

#endif //_LOADER_H____