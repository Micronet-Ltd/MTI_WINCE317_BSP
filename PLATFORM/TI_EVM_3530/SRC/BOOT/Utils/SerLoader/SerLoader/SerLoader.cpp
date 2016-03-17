// SerLoader.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "SerLoaderApi.h"
#include "Protocol.h"
#include "SerTransport.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

//CSerTransport*	pSerTr	= 0;
//CProtocol*		pProt	= 0;


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
//		pSerTr	= new CSerTransport;	
//		pProt	= new CProtocol(pSerTr);
	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	break;
	case DLL_PROCESS_DETACH:
//		delete pProt;
//		delete pSerTr;	
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

// This is an example of an exported variable
//SERLOADER_API int nSerLoader=0;
//
//// This is an example of an exported function.
//SERLOADER_API int fnSerLoader(void)
//{
//	return 42;
//}
//
//// This is the constructor of a class that has been exported.
//// see SerLoader.h for the class definition
//CSerLoader::CSerLoader()
//{
//	return;
//}
