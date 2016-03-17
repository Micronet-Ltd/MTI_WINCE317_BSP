// lap.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <bt_api.h>
#include <bthapi.h>
#include <initguid.h>
#include <bt_sdp.h>
#include "lap.h"
#include <windows.h>
#include <commctrl.h>

#define MAX_LOADSTRING 100


// Global Variables:
HINSTANCE			g_hInst;			// current instance

// Forward declarations of functions included in this code module:
#ifndef WIN32_PLATFORM_WFSP
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
#endif // !WIN32_PLATFORM_WFSP

#define MUTEX_INQUIRY_NAME L"Eneter Inquiry"
//#define MUTEX_DISCOVERY_NAME L"Eneter Discovery"

typedef enum
{
	BTD_INVALID		= -1,
	BTD_MODEM		=  0,
	BTD_PRINTER,
	BTD_LAP,
	BTD_OBEX_FTP,
	BTD_OBEX_OPP,
	BTD_HEADSET,
	BTD_ASYNC,
	BTD_HID,
	BTD_PAN,
	BTD_HANDSFREE,
	BTD_CLASSES
}MIC_BTD_SERVICE;

typedef struct tagBTH_CONTEXT
{
	UINT32 size; // sizeof(CONTEXT)
	UINT32 status; // operation status
	PVOID pBLOB;
	HANDLE Scan;
	HANDLE Lookup;
	UINT32 AbortInqiry;
	UINT32 AbortDiscovery;
	HANDLE clients[BTD_CLASSES];
}MIC_BTH_CONTEXT;

typedef struct tag_BT_DEVICEDETAILS
{
	BT_ADDR	b; // see MSDN
	unsigned	uClassOfDevice; // Class of Device see MSDN
	unsigned	uSrvcMask;	    // Services Mask
	unsigned 	cannels[BTD_CLASSES];
	TCHAR		tszName[256];
}MIC_BT_DEVICEDETAILS;

typedef struct tag_BT_DEVICESLIST
{
	MIC_BT_DEVICEDETAILS *dev;
	tag_BT_DEVICESLIST	 *next;
}MIC_BT_DEVICESLIST;

typedef unsigned long (*PFN_BTH_INQUIRY_CALLBACK)(MIC_BT_DEVICESLIST *pvList, MIC_BT_DEVICEDETAILS *pDev);
typedef unsigned long (*PFN_BTH_DISCOVERY_CALLBACK)(MIC_BTD_SERVICE Srvc, MIC_BT_DEVICEDETAILS *pDev);

TCHAR *bt_dev_reg[] = 
{
	TEXT("SOFTWARE\\Microsoft\\Bluetooth\\device\\modem"),
	TEXT("SOFTWARE\\Microsoft\\Bluetooth\\device\\printer"),
	TEXT("SOFTWARE\\Microsoft\\Bluetooth\\device\\lan_access"),
	TEXT("SOFTWARE\\Microsoft\\Bluetooth\\device\\obex_ftp"),
	TEXT("SOFTWARE\\Microsoft\\Bluetooth\\device\\obex_opp"),
	TEXT("SOFTWARE\\Microsoft\\Bluetooth\\device\\headset"),
	TEXT("SOFTWARE\\Microsoft\\Bluetooth\\device\\activesync"),
	TEXT("SOFTWARE\\Microsoft\\Bluetooth\\device\\hid"),
	TEXT("SOFTWARE\\Microsoft\\Bluetooth\\device\\pan"),
	TEXT("SOFTWARE\\Microsoft\\Bluetooth\\device\\handsfree")
};

TCHAR *bt_dev_name[] = 
{
	TEXT("Dialup networking"),
	TEXT("Printer"),
	TEXT("LAN Access"),
	TEXT("OBEX File Transfer"),
	TEXT("OBEX Object Push"),
	TEXT("Headset"),
	TEXT("Activesync"),
	TEXT("Hid"),
	TEXT("Private Networking"),
	TEXT("Handsfree")
};

BOOL MIC_BthEnableScan(BOOL en, PVOID pContext)
{
	INT32 res = 0;
	MIC_BTH_CONTEXT *pcx = 0;

	__try
	{
		if(!pContext)
			__leave;
		pcx = (MIC_BTH_CONTEXT *)pContext;
		if(pcx->size < sizeof(MIC_BTH_CONTEXT))
			__leave;

		pcx->status = ERROR_INVALID_PARAMETER;

		en = (en)?3:0;

		pcx->status = BthWriteScanEnableMask(en);
		if(ERROR_SUCCESS == pcx->status)
			res = 1;
	}
	__finally
	{
	}

	return res;
}

BOOL MIC_BthInquiry(PVOID pContext, PFN_BTH_INQUIRY_CALLBACK pfnCallback)
{
	INT32 res = 0;
	HANDLE Mutex;
	UINT32 Err = WSASERVICE_NOT_FOUND;
	WSAQUERYSET wsaq;

	MIC_BTH_CONTEXT *pcx = 0;
	MIC_BT_DEVICEDETAILS Dev = {0};
	MIC_BT_DEVICESLIST *pList = 0;

	__try
	{
		if(!pContext)
			__leave;

		pcx = (MIC_BTH_CONTEXT *)pContext;
		if(pcx->size < sizeof(MIC_BTH_CONTEXT))
			__leave;

		pcx->status = ERROR_INVALID_PARAMETER;

		if(!pcx->pBLOB)
			__leave;

		pList = (MIC_BT_DEVICESLIST *)pcx->pBLOB;
		memset(pList, 0, sizeof(MIC_BT_DEVICESLIST));

		Mutex = CreateMutex(0, 0, MUTEX_INQUIRY_NAME);
		if(!Mutex)
			__leave;

		RETAILMSG(1, (L"btsvc:: BtStartInquiry\r\n"));

		memset(&wsaq, 0, sizeof(wsaq));

		wsaq.dwSize      = sizeof(wsaq);
		wsaq.dwNameSpace = NS_BTH;
		wsaq.lpcsaBuffer = 0;

		Err = WaitForSingleObject(Mutex, INFINITE);
		if(WAIT_OBJECT_0 != Err)
		{
			pcx->status = GetLastError();
			__leave;
		}
		pcx->status = BthNsLookupServiceBegin(&wsaq, LUP_CONTAINERS , &pcx->Scan);
		
		while((pcx->status == ERROR_SUCCESS) && pcx->Scan)
		{
			union {
				CHAR buf[5000];
				double __unused;	// ensure proper alignment
			};

			LPWSAQUERYSET pwsaRes = (LPWSAQUERYSET)buf;
			DWORD dwSize  = sizeof(buf);

			memset(pwsaRes, 0, sizeof(WSAQUERYSET));
			pwsaRes->dwSize      = sizeof(WSAQUERYSET);
			pwsaRes->dwNameSpace = NS_BTH;
			pwsaRes->lpBlob      = 0;

			pcx->status = BthNsLookupServiceNext(pcx->Scan, LUP_RETURN_NAME | LUP_RETURN_ADDR | LUP_RETURN_BLOB, &dwSize, pwsaRes);
			if((pcx->status == ERROR_SUCCESS) && (pwsaRes->dwNumberOfCsAddrs == 1))
			{
				memset(&Dev, 0, sizeof(Dev));
				Dev.uClassOfDevice = ((BthInquiryResult *)pwsaRes->lpBlob->pBlobData)->cod;
				Dev.b = ((SOCKADDR_BTH *)pwsaRes->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr;
				if(pwsaRes->lpszServiceInstanceName && *(pwsaRes->lpszServiceInstanceName))
				{
					int l = (_tcslen(pwsaRes->lpszServiceInstanceName) < (sizeof(Dev.tszName)/sizeof(Dev.tszName[0]) - 1))?
						_tcslen(pwsaRes->lpszServiceInstanceName):(sizeof(Dev.tszName)/sizeof(Dev.tszName[0]) - 1);
						_tcsncpy(Dev.tszName, pwsaRes->lpszServiceInstanceName, l);
				}
				if(pfnCallback)
				{

					Err = pfnCallback(pList, &Dev);
					if(Err)
					{
						pcx->status = ERROR_SUCCESS;
						res = 1;
						__leave;
					}
				}
			}
		}

		if(pcx->AbortInqiry)
		{
			pcx->status = ERROR_OPERATION_ABORTED;
			pcx->AbortInqiry = 0;
		}
		else
		{
			if(pcx->status == SOCKET_ERROR && GetLastError() == WSA_E_NO_MORE)
				pcx->status = ERROR_SUCCESS;
		}

		res = 1;
	}
	__finally
	{
		if(pcx->Scan)
			BthNsLookupServiceEnd(pcx->Scan);
		pcx->Scan = 0;

		if(Mutex)
		{
			ReleaseMutex(Mutex);
			CloseHandle(Mutex);
		}
	}

	return res;
}
BOOL MIC_BthCancelInquiry(PVOID pContext)
{
	INT32 res = 0;
	MIC_BTH_CONTEXT *pcx = 0;

	__try
	{
		if(!pContext)
			__leave;
		
		pcx = (MIC_BTH_CONTEXT *)pContext;
		
		if(pcx->size < sizeof(MIC_BTH_CONTEXT))
			__leave;

		pcx->status = ERROR_INVALID_PARAMETER;

		if(pcx->Scan)
		{
			pcx->AbortInqiry = 1;
			pcx->status = BthNsLookupServiceEnd((HANDLE)BTHNS_ABORT_CURRENT_INQUIRY);
		}
		res = 1;
	}
	__finally
	{
	}

	return res;
}

BOOL MIC_BthDesroyDevicesList(MIC_BT_DEVICESLIST *pList, PVOID pContext)
{
	MIC_BT_DEVICESLIST *pvl = pList, *pvlt;
	do
	{
		if(!pvl)
			break;
		if(pvl->dev)
		{
			delete pvl->dev;
			pvl->dev = 0;
		}

		if(pvl->next)
		{
			if(pvl == pList)
				pvl = pvl->next;
			else
			{
				pvlt = pvl->next;
				delete pvl;
				pvl = pvlt;
			}
			continue;
		}
		break;
	}while(pvl);

	return 1;
}
#define HIDServiceClassID_UUID16	0x00001124
const UUID HIDServiceClassID_UUID			= {0x00001124, 0x0000, 0x1000, {0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}};
const UUID ActiveSyncServiceClassID_UUID	= {0x0350278F, 0x3DCA, 0x4e62, {0x83, 0x1D, 0xA4, 0x11, 0x65, 0xFF, 0x90, 0x6C}};

unsigned short BthUUID(MIC_BTD_SERVICE service)
{
	if(service == BTD_MODEM)
		return DialupNetworkingServiceClassID_UUID16;
	else if((service == BTD_PRINTER) || (service == BTD_ASYNC))
		return SerialPortServiceClassID_UUID16;
	else if(service == BTD_LAP)
		return LANAccessUsingPPPServiceClassID_UUID16; //GenericNetworkingServiceClassID_UUID16;
	else if(service == BTD_OBEX_FTP)
		return OBEXFileTransferServiceClassID_UUID16;
	else if(service == BTD_OBEX_OPP)
		return OBEXObjectPushServiceClassID_UUID16; //GenericFileTransferServiceClassID_UUID16
	else if(service == BTD_HEADSET)
		return HeadsetServiceClassID_UUID16;
	else if(service == BTD_HANDSFREE)
        return HandsfreeServiceClassID_UUID16;
	else if(service == BTD_HID)
		return HIDServiceClassID_UUID16;

	return 0;
}

int BthIsRfcommUuid(NodeData *pNode)
{
	if(pNode->type != SDP_TYPE_UUID)
		return 0;

	if(pNode->specificType == SDP_ST_UUID16)
		return (pNode->u.uuid16 == RFCOMM_PROTOCOL_UUID16);
	else if(pNode->specificType == SDP_ST_UUID32)
		return (pNode->u.uuid32 == RFCOMM_PROTOCOL_UUID16);
	else if(pNode->specificType == SDP_ST_UUID128)
		return (0 == memcmp(&RFCOMM_PROTOCOL_UUID, &pNode->u.uuid128, sizeof(GUID)));

	return 0;
}

int BthGetChannelNode(NodeData *pChannelNode)
{
	if(pChannelNode->specificType == SDP_ST_UINT8)
		return pChannelNode->u.uint8;
	else if(pChannelNode->specificType == SDP_ST_INT8)
		return pChannelNode->u.int8;
	else if(pChannelNode->specificType == SDP_ST_UINT16)
		return pChannelNode->u.uint16;
	else if(pChannelNode->specificType == SDP_ST_INT16)
		return pChannelNode->u.int16;
	else if(pChannelNode->specificType == SDP_ST_UINT32)
		return pChannelNode->u.uint32;
	else if(pChannelNode->specificType == SDP_ST_INT32)
		return pChannelNode->u.int32;

	return 0;
}

unsigned long BthIsService(BT_ADDR *pb, MIC_BTD_SERVICE service)
{
	int iRet = 0;

	BTHNS_RESTRICTIONBLOB RBlob;

	memset(&RBlob, 0, sizeof(RBlob));

	RBlob.type = SDP_SERVICE_SEARCH_REQUEST;
	RBlob.numRange = 1;
	RBlob.pRange[0].minAttribute = SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST;
	RBlob.pRange[0].maxAttribute = SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST;
	RBlob.uuids[0].uuidType = SDP_ST_UUID16;
	RBlob.uuids[0].u.uuid16 = BthUUID(service);

	BLOB blob;
	blob.cbSize = sizeof(RBlob);
	blob.pBlobData = (BYTE *)&RBlob;

	SOCKADDR_BTH	sa;

	memset(&sa, 0, sizeof(sa));

	*(BT_ADDR *)(&sa.btAddr) = *pb;
	sa.addressFamily = AF_BT;

	CSADDR_INFO		csai;

	memset (&csai, 0, sizeof(csai));
	csai.RemoteAddr.lpSockaddr = (sockaddr *)&sa;
	csai.RemoteAddr.iSockaddrLength = sizeof(sa);

	WSAQUERYSET		wsaq;

	memset (&wsaq, 0, sizeof(wsaq));
	wsaq.dwSize      = sizeof(wsaq);
	wsaq.dwNameSpace = NS_BTH;
//	wsaq.lpServiceClassId = &uuid;
	wsaq.lpBlob      = &blob;
	wsaq.lpcsaBuffer = &csai;

	HANDLE hLookup;
	iRet = BthNsLookupServiceBegin(&wsaq, 0, &hLookup); //LUP_RES_SERVICE

	if(ERROR_SUCCESS == iRet)
	{
		union
		{
			CHAR buf[5000];
			double __unused;
		};

		LPWSAQUERYSET pwsaResults = (LPWSAQUERYSET) buf;
		DWORD dwSize  = sizeof(buf);

		memset(pwsaResults,0,sizeof(WSAQUERYSET));
		pwsaResults->dwSize      = sizeof(WSAQUERYSET);
		pwsaResults->dwNameSpace = NS_BTH;
		pwsaResults->lpBlob      = 0;

		iRet = BthNsLookupServiceNext(hLookup, 0, &dwSize, pwsaResults);

		if(iRet == ERROR_SUCCESS)
		{
			iRet = BthNsLookupServiceNext(hLookup,0,&dwSize,pwsaResults);
			if(iRet == SOCKET_ERROR && GetLastError() == WSA_E_NO_MORE)
			{
				RETAILMSG(1, (L"btsvc:: %d service found on %X%X\r\n", service, (DWORD)((LONGLONG)(*pb)>>32), (DWORD)(*pb)));
				iRet = ERROR_SUCCESS;
			}
		}

		if(iRet != ERROR_SUCCESS)
		{
			RETAILMSG(1, (L"btsvc:: Lookup %d service failed\r\n", service));
		}

		BthNsLookupServiceEnd(hLookup);
	}

	return (iRet == ERROR_SUCCESS);
}


HRESULT BthServiceAndAttributeSearch( unsigned char	*szResponse,		// in - response returned from SDP ServiceAttribute query
												 unsigned long	cbResponse,			// in - length of response
												 ISdpRecord		***pppSdpRecords,	// out - array of pSdpRecords
												 unsigned long	*pNumRecords		// out - number of elements in pSdpRecords
											   )
{
    *pppSdpRecords = 0;
    *pNumRecords = 0;
    ISdpStream *pIStream = 0;

	HRESULT hres = CoCreateInstance(__uuidof(SdpStream), 0, CLSCTX_INPROC_SERVER, __uuidof(ISdpStream), (void **)&pIStream);

    if(FAILED(hres))
        return hres;  

	ULONG ulError;

    hres = pIStream->Validate(szResponse, cbResponse, &ulError);

    if(SUCCEEDED(hres))
	{
        hres = pIStream->VerifySequenceOf(szResponse,cbResponse, SDP_TYPE_SEQUENCE, 0, pNumRecords);

        if(SUCCEEDED(hres) && *pNumRecords > 0)
		{
			*pppSdpRecords = (ISdpRecord **)CoTaskMemAlloc(sizeof(ISdpRecord *) * (*pNumRecords));

            if(pppSdpRecords)
			{
                hres = pIStream->RetrieveRecords(szResponse, cbResponse, *pppSdpRecords, pNumRecords);

                if(!SUCCEEDED(hres))
				{
					CoTaskMemFree(*pppSdpRecords);
                    *pppSdpRecords = 0;
                    *pNumRecords = 0;
                }
            }
            else
                hres = E_OUTOFMEMORY;
        }
    }

    if(pIStream)
	{
        pIStream->Release();
        pIStream = 0;
    }

	return hres;
}

unsigned long BthChannel(void *pvData, int dataSize, unsigned char *chnl)
{
	ISdpRecord **pRecordArg;
	int		   i,cRecordArg	= 0;

	*chnl = 0;

	HRESULT hr = BthServiceAndAttributeSearch((unsigned char *)pvData, dataSize, &pRecordArg, (unsigned long *)&cRecordArg);

	if(FAILED(hr))
		return HRESULT_CODE(hr);

	for(i = 0; (!*chnl) && (i < cRecordArg); i++)
	{
	    ISdpRecord *pRecord		 = pRecordArg[i];    // particular record to examine in this loop
	    NodeData   protocolList;					 // contains SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST data, if available

		hr = pRecord->GetAttribute(SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST, &protocolList) || (protocolList.type != SDP_TYPE_CONTAINER);
		if(ERROR_SUCCESS != HRESULT_CODE(hr))
			continue;
		
		ISdpNodeContainer *pRecordContainer = protocolList.u.container;
		int				  cProtocols		= 0;
		NodeData		  protocolDescriptor;	// information about a specific protocol (i.e. L2CAP, RFCOMM, ...)

		pRecordContainer->GetNodeCount((unsigned long *)&cProtocols);
		for(int j = 0; (!*chnl) && (j < cProtocols); j++)
		{
			pRecordContainer->GetNode(j, &protocolDescriptor);

			if(protocolDescriptor.type != SDP_TYPE_CONTAINER)
				continue;

			ISdpNodeContainer *pProtocolContainer = protocolDescriptor.u.container;
			int				  cProtocolAtoms	  = 0;

			pProtocolContainer->GetNodeCount((unsigned long *)&cProtocolAtoms);

			for(int k = 0; (!*chnl) && (k < cProtocolAtoms); k++)
			{
				NodeData nodeAtom;  // individual data element, such as what protocol this is or RFCOMM channel id.

				pProtocolContainer->GetNode(k, &nodeAtom);

				if(BthIsRfcommUuid(&nodeAtom))
				{
					if(k+1 == cProtocolAtoms) // misformatted response.  Channel ID should follow RFCOMM uuid
						break;

					NodeData channelID;
					pProtocolContainer->GetNode(k+1, &channelID);

					*chnl = (unsigned char)BthGetChannelNode(&channelID);
					break; // formatting error
				}
			}
		}
	}

    for(i = 0; i < cRecordArg; i++) 
		pRecordArg[i]->Release();

	CoTaskMemFree(pRecordArg);

	return (*chnl)?ERROR_SUCCESS:ERROR_ACCESS_DENIED;
}

unsigned long BthVerifyRecord(int rec, void *pStream, int cStream, unsigned long *subClass)
{
	int i, iErr = ERROR_SUCCESS;
	int cRecordArg = 0;
	ISdpRecord **pRecordArg;

	*subClass = 0;

	iErr = BthServiceAndAttributeSearch((unsigned char	*)pStream, cStream, &pRecordArg, (unsigned long *)&cRecordArg);
	if(ERROR_SUCCESS != HRESULT_CODE(iErr))
		return iErr;

	int fFound = 0;

	for(i = 0; i < cRecordArg; i++)
	{
	    ISdpRecord *pRecord = pRecordArg[i];
	    NodeData  nodeAttrib;

		iErr = pRecord->GetAttribute(rec, &nodeAttrib);
        if(ERROR_SUCCESS != HRESULT_CODE(iErr))
			continue;

		fFound = 1;
        *subClass = nodeAttrib.u.uint8;
		break;
	}

    for(i = 0; i < cRecordArg; i++) 
		pRecordArg[i]->Release();

	CoTaskMemFree(pRecordArg);

	return HRESULT_CODE(iErr);
}

unsigned long BthSDP(BT_ADDR b, int *c, MIC_BTD_SERVICE service, unsigned long *devSubclass)
{
	unsigned char *ppsdp = 0;
	unsigned int pcsdp = 0;
	*devSubclass = 0;

	int iRet = 0;

	*c = 0;//0xFE;

	for(int i = 0; i < (((service == BTD_ASYNC) && (*c == 0)) ? 2 : 1); ++i)
	{
		BTHNS_RESTRICTIONBLOB RBlob;

		memset(&RBlob, 0, sizeof(RBlob));

		RBlob.type = SDP_SERVICE_SEARCH_ATTRIBUTE_REQUEST;
		RBlob.numRange = 1;

		if(service == BTD_HID)
		{
			RBlob.pRange[0].minAttribute = 0;
			RBlob.pRange[0].maxAttribute = 0xffff;

			RBlob.uuids[0].uuidType = SDP_ST_UUID128;
			RBlob.uuids[0].u.uuid128 = HIDServiceClassID_UUID;
		}
		else if((service == BTD_ASYNC) && (i == 0))
		{
			RBlob.pRange[0].minAttribute = 0;
			RBlob.pRange[0].maxAttribute = 0xffff;

			RBlob.uuids[0].uuidType = SDP_ST_UUID128;
			RBlob.uuids[0].u.uuid128 = ActiveSyncServiceClassID_UUID;
		}
		else
		{
			RBlob.pRange[0].minAttribute = SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST;
			RBlob.pRange[0].maxAttribute = SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST;

			RBlob.uuids[0].uuidType = SDP_ST_UUID16;
			RBlob.uuids[0].u.uuid16 = BthUUID(service);
		}

		BLOB blob;
		blob.cbSize = sizeof(RBlob);
		blob.pBlobData = (BYTE *)&RBlob;

		SOCKADDR_BTH	sa;

		memset(&sa, 0, sizeof(sa));

		*(BT_ADDR *)(&sa.btAddr) = b;
		sa.addressFamily = AF_BT;

		CSADDR_INFO		csai;

		memset (&csai, 0, sizeof(csai));
		csai.RemoteAddr.lpSockaddr = (sockaddr *)&sa;
		csai.RemoteAddr.iSockaddrLength = sizeof(sa);

		WSAQUERYSET		wsaq;

		memset (&wsaq, 0, sizeof(wsaq));
		wsaq.dwSize      = sizeof(wsaq);
		wsaq.dwNameSpace = NS_BTH;
		wsaq.lpBlob      = &blob;
		wsaq.lpcsaBuffer = &csai;

		HANDLE hLookup;
		iRet = BthNsLookupServiceBegin (&wsaq, 0, &hLookup); //LUP_RES_SERVICE

		if(ERROR_SUCCESS == iRet)
		{
			union
			{
				CHAR buf[5000];
				double __unused;
			};

			LPWSAQUERYSET pwsaResults = (LPWSAQUERYSET) buf;
			DWORD dwSize  = sizeof(buf);

			memset(pwsaResults,0,sizeof(WSAQUERYSET));
			pwsaResults->dwSize      = sizeof(WSAQUERYSET);
			pwsaResults->dwNameSpace = NS_BTH;
			pwsaResults->lpBlob      = NULL;

			iRet = BthNsLookupServiceNext(hLookup, 0, &dwSize, pwsaResults);
			if(iRet == ERROR_SUCCESS)
			{	// Success - got the stream
				if(service == BTD_HID)
				{
					if(BthVerifyRecord(SDP_ATTRIB_HID_DEVICE_SUBCLASS, pwsaResults->lpBlob->pBlobData, pwsaResults->lpBlob->cbSize, devSubclass))
					{
						ppsdp = (unsigned char *)LocalAlloc(LMEM_FIXED, pwsaResults->lpBlob->cbSize);
						if(ppsdp)
						{
							memcpy (ppsdp, pwsaResults->lpBlob->pBlobData, pwsaResults->lpBlob->cbSize);
							pcsdp = pwsaResults->lpBlob->cbSize;
						}
					}
				}
				else
				{
					if(ERROR_SUCCESS != BthChannel(pwsaResults->lpBlob->pBlobData, pwsaResults->lpBlob->cbSize, (unsigned char *)c))
						*c = 0xFE;
					else
					{
						BthNsLookupServiceEnd(hLookup);
						break;
					}
				}
			}

			BthNsLookupServiceEnd(hLookup);
		}
	}

	return iRet;
}

unsigned long BthRfCommChannel(MIC_BT_DEVICEDETAILS *pDev, MIC_BTD_SERVICE s)
{
	DWORD subclass = 0;
	int c = 0xFE;
	int iErr;


	if(!pDev)
		return c;

	iErr = BthSDP(pDev->b, &c, s, &subclass);

	if(ERROR_SUCCESS != iErr)
		c = 0xFE;

	return c;
}

BOOL MIC_BthDiscoveryServices(MIC_BT_DEVICEDETAILS *pDev, PFN_BTH_DISCOVERY_CALLBACK pCallback, PVOID pContext)
{
	INT32 res = 0;
	UINT32 Err = WSASERVICE_NOT_FOUND;
	HANDLE Mutex;
	MIC_BTH_CONTEXT *pcx = 0;

	__try
	{
		CoInitializeEx(0, COINIT_MULTITHREADED);

		if(!pContext)
			__leave;

		pcx = (MIC_BTH_CONTEXT *)pContext;
		if(pcx->size < sizeof(MIC_BTH_CONTEXT))
		{
			pcx = 0;
			__leave;
		}

		Mutex = CreateMutex(0, 0, MUTEX_INQUIRY_NAME);
		if(!Mutex)
		{
			pcx->status = GetLastError();
			__leave;
		}

		Err = WaitForSingleObject(Mutex, INFINITE);
		if(WAIT_OBJECT_0 != Err)
		{
			pcx->status = GetLastError();
			__leave;
		}

		pcx->status = ERROR_INVALID_PARAMETER;

		if(!pDev)
			__leave;


		pcx->Lookup = (HANDLE)pDev;
		pDev->uSrvcMask = 0;
		memset(pDev->cannels, 0xFF, sizeof(pDev->cannels));

		RETAILMSG(1, (L"btsvc:: BtDiscoveryServices\r\n"));

		for(int i = BTD_MODEM; i < BTD_CLASSES; i++)
		{
			if(BthIsService(&pDev->b, (MIC_BTD_SERVICE)i))
			{
				pDev->cannels[i] = BthRfCommChannel(pDev, (MIC_BTD_SERVICE)i);
				pDev->uSrvcMask |= 1 << i;

				if(pCallback)
				{
					if(pCallback((MIC_BTD_SERVICE)i, pDev))
					{
						pcx->status = ERROR_SUCCESS;
						res = 1;
						__leave;
					}
				}
			}
			if(pcx->AbortDiscovery)
			{
				pcx->status = ERROR_OPERATION_ABORTED;
				pcx->AbortDiscovery = 0;
				break;
			}
		}

		if(pcx->status == SOCKET_ERROR && GetLastError() == WSA_E_NO_MORE)
			pcx->status = ERROR_SUCCESS;

		res = 1;
	}
	__finally
	{
		CoUninitialize();
		if(pcx)
			pcx->Lookup = 0;
		if(Mutex)
		{
			ReleaseMutex(Mutex);
			CloseHandle(Mutex);
		}
	}

	return res;
}

BOOL MIC_BthCancelDiscovery(PVOID pContext)
{
	INT32 res = 0;
	MIC_BTH_CONTEXT *pcx = 0;

	__try
	{
		if(!pContext)
			__leave;
		pcx = (MIC_BTH_CONTEXT *)pContext;
		if(pcx->size < sizeof(MIC_BTH_CONTEXT))
			__leave;
		if(pcx->Lookup)
		{
			pcx->AbortDiscovery = 1;
		}
		pcx->status = ERROR_SUCCESS;
		res = 1;
	}
	__finally
	{
	}

	return res;
}

BOOL MIC_BthAuthenticate(MIC_BT_DEVICEDETAILS *pDev, PVOID pContext, UINT8 *pin)
{
	INT32 res = 0;
	MIC_BTH_CONTEXT *pcx = 0;

	__try
	{
		if(!pContext)
			__leave;
		pcx = (MIC_BTH_CONTEXT *)pContext;
		if(pcx->size < sizeof(MIC_BTH_CONTEXT))
			__leave;

		pcx->status = ERROR_INVALID_PARAMETER;

		if(!pDev)
			__leave;

		pcx->status = BthPairRequest(&pDev->b, strlen((char *)pin), pin);

		res = (pcx->status == ERROR_SUCCESS);
	}
	__finally
	{
	}

	return res;
}

BOOL MIC_BthRevokePin(MIC_BT_DEVICEDETAILS *pDev, PVOID pContext)
{
	INT32 res = 0;
	MIC_BTH_CONTEXT *pcx = 0;

	__try
	{
		if(!pContext)
			__leave;
		
		pcx = (MIC_BTH_CONTEXT *)pContext;
		if(pcx->size < sizeof(MIC_BTH_CONTEXT))
			__leave;

		pcx->status = ERROR_INVALID_PARAMETER;

		if(!pDev)
			__leave;

		pcx->status = BthRevokePIN(&pDev->b);
		if(pcx->status == ERROR_SUCCESS)
			pcx->status = BthRevokeLinkKey(&pDev->b);

		res = (pcx->status == ERROR_SUCCESS);
	}
	__finally
	{
	}

	return res;
}

UINT32 BthCreateDevice(MIC_BTD_SERVICE s, MIC_BT_DEVICEDETAILS *pDev, PVOID pContext)
{
    HKEY hk = 0;
    DWORD dwDisp;
	INT32 res = 0, port, imtu;
    unsigned int uiportflags = 0;
	MIC_BTH_CONTEXT *pcx = 0;
	TCHAR buf[512] = {0};

	__try
	{
		if(!pContext)
			__leave;
		
		pcx = (MIC_BTH_CONTEXT *)pContext;
		if(pcx->size < sizeof(MIC_BTH_CONTEXT))
			__leave;

		pcx->status = ERROR_INVALID_PARAMETER;

		if(!pDev)
			__leave;

		if(!(pDev->uSrvcMask & (1 << s)))
			__leave;

		if(pcx->clients[s] != (void *)-1)
			__leave;

		wsprintf(buf, L"%s\\%04x%08x", bt_dev_reg[s], GET_NAP(pDev->b), GET_SAP(pDev->b));
		if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, buf, 0, KEY_ALL_ACCESS, &hk))
			__leave;

		DWORD dw;
		DWORD dwSize = sizeof(dw);
		DWORD dwType;
		if((ERROR_SUCCESS != RegQueryValueEx(hk, L"channel", NULL, &dwType, (BYTE *)&dw, &dwSize)) ||
							 (dwType != REG_DWORD) || (dwSize > sizeof(dw)))
			__leave;

		if(pDev->cannels[s] != dw)
			__leave;

		dwSize = sizeof(buf);

		if((ERROR_SUCCESS != RegQueryValueEx(hk, L"device", NULL, &dwType, (BYTE *)buf, &dwSize)) ||
							 (dwType != REG_SZ) || (dwSize > sizeof(buf)))
			__leave;

//		Is the bt_addr valid?
//		if(!GetBA(szString, &b))
//			__leave;

		dwSize = sizeof(dw);

		if((ERROR_SUCCESS != RegQueryValueEx(hk, L"authenticate", NULL, &dwType, (BYTE *)&dw, &dwSize)) ||
							 (dwType != REG_DWORD) || (dwSize != sizeof(dw)) || (!dw))
			uiportflags &=  ~RFCOMM_PORT_FLAGS_AUTHENTICATE;
		else
			uiportflags |=  RFCOMM_PORT_FLAGS_AUTHENTICATE;

		dwSize = sizeof(dw);

		if((ERROR_SUCCESS != RegQueryValueEx(hk, L"encrypt", NULL, &dwType, (BYTE *)&dw, &dwSize)) ||
							 (dwType != REG_DWORD) || (dwSize != sizeof(dw)) || (!dw))
			uiportflags &=  ~RFCOMM_PORT_FLAGS_ENCRYPT;
		else
			uiportflags |=  RFCOMM_PORT_FLAGS_ENCRYPT;

		dwSize = sizeof(dw);

		if((ERROR_SUCCESS != RegQueryValueEx(hk, L"port", NULL, &dwType, (BYTE *)&dw, &dwSize)) ||
							 (dwType != REG_DWORD) || (dwSize > sizeof(dw)) || (dw < 0) || (dw > 9))
			__leave;

		port = dw;

		dw = 0;
		dwSize = sizeof(dw);

		if((ERROR_SUCCESS != RegQueryValueEx(hk, L"mtu", NULL, &dwType, (BYTE *)&dw, &dwSize)) ||
							 (dwType != REG_DWORD) || (dwSize != sizeof(dw)) || (! dw))
				dw = 0;

		imtu = (int)dw;

		RegCloseKey (hk);

//		MIC_BthDisconnect(s, pContext);

		PORTEMUPortParams pp;
		memset(&pp, 0, sizeof(pp));

		pp.device  = pDev->b;
		pp.channel = pDev->cannels[s];
		pp.imtu    = imtu;
		pp.uiportflags = uiportflags;


		wsprintf(buf, L"software\\microsoft\\bluetooth\\device\\ports\\%s", bt_dev_name[s]);

		if(ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE, buf, 0, NULL, 0, KEY_WRITE, NULL, &hk, &dwDisp))
			__leave;

		RegSetValueEx(hk, L"dll", 0, REG_SZ, (BYTE *)L"btd.dll", sizeof(L"btd.dll"));
		wsprintf(buf, L"BSP");
		dwSize = (1  + _tcslen(buf)) * sizeof(buf[0]);
		RegSetValueEx(hk, L"prefix", 0, REG_SZ, (BYTE *)buf, dwSize);
		RegSetValueEx(hk, L"index", 0, REG_DWORD, (BYTE *)&port, sizeof(port));

		dw = (DWORD)&pp;
		RegSetValueEx(hk, L"context", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));

		if((s == BTD_MODEM) || (s == BTD_LAP) || (s== BTD_ASYNC))
		{
			HKEY hk2;

			if(ERROR_SUCCESS != RegCreateKeyEx(hk, L"unimodem", 0, NULL, 0, KEY_WRITE, NULL, &hk2, &dwDisp))
				__leave;

			if(dwDisp == REG_CREATED_NEW_KEY)
			{
				RegSetValueEx(hk2, L"friendlyname", 0, REG_SZ, (BYTE *)bt_dev_name[s], (_tcslen(bt_dev_name[s]) + 1) * sizeof(bt_dev_name[s][0]));
				RegSetValueEx (hk2, L"tsp", 0, REG_SZ, (BYTE *)L"unimodem.dll", sizeof(L"unimodem.dll"));
				dw = (s == BTD_MODEM)?1:0;
				RegSetValueEx(hk2, L"devicetype", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));
			}
			RegCloseKey (hk2);
		}

		wsprintf(buf, L"software\\microsoft\\bluetooth\\device\\ports\\%s", bt_dev_name[s]);
		pcx->clients[s] = ActivateDevice(buf, 0);

		//		pcx->clients[s] = RegisterBluetoothCOMPort (L"COM", port, &pp);


/*
    RASENTRY RasEntry;

    RasEntry.dwSize = sizeof(RASENTRY);
    DWORD cb = sizeof(RASENTRY);
    RasGetEntryProperties (NULL, L"", &RasEntry, &cb, NULL, NULL);

    RasEntry.dwfOptions &= ~(RASEO_SpecificNameServers|RASEO_SpecificIpAddr|
                                    RASEO_IpHeaderCompression|RASEO_SwCompression|RASEO_UseCountryAndAreaCodes);

    wcscpy (RasEntry.szDeviceType, RASDT_Direct);
    wcscpy (RasEntry.szDeviceName, L"bluetooth_lap");

    RasSetEntryProperties(NULL, L"Bluetooth LAN Access", &RasEntry, sizeof(RasEntry), NULL, 0);

    RASDIALPARAMS   RasDialParams;
    memset((char *)&RasDialParams, 0, sizeof(RasDialParams));

    RasDialParams.dwSize = sizeof(RASDIALPARAMS);
    wcscpy (RasDialParams.szEntryName, L"Bluetooth LAN Access");

    wcscpy (RasDialParams.szUserName, L"guest");
//    wcscpy (RasDialParams.szPassword, L"guest");

    RasSetEntryDialParams(NULL, &RasDialParams, FALSE);
*/
		pcx->status = GetLastError();

		res = (pcx->status == ERROR_SUCCESS);
	}
	__finally
	{
		if(hk)
		{
			RegCloseKey (hk);
		}
	}

    return res;
}

UINT32 MIC_BthConnect(MIC_BTD_SERVICE s, UINT32 port, MIC_BT_DEVICEDETAILS *pDev, PVOID pContext)
{
	INT32 res = 0;
	MIC_BTH_CONTEXT *pcx = 0;
    HKEY hk;
    DWORD dwDisp;
	TCHAR buf[512] = {0};
    int fEncrypt = 0; // for future use
    int fAuth = 0;// for future use
    int fLan = 1;// for future use

	__try
	{
		if(!pContext)
			__leave;
		
		pcx = (MIC_BTH_CONTEXT *)pContext;
		if(pcx->size < sizeof(MIC_BTH_CONTEXT))
			__leave;

		pcx->status = ERROR_INVALID_PARAMETER;

		if(!pDev)
			__leave;

		if(!(pDev->uSrvcMask & (1 << s)))
			__leave;
		if(pDev->cannels[s] == -1)
			__leave;

		wsprintf(buf, L"%s\\%04x%08x", bt_dev_reg[s], GET_NAP(pDev->b), GET_SAP(pDev->b));
		if(ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, buf, 0, NULL, 0, KEY_WRITE, NULL, &hk, &dwDisp))
		{
			if(dwDisp == REG_CREATED_NEW_KEY)
			{
				wsprintf(buf, L"%04x%08x", GET_NAP(pDev->b), GET_SAP(pDev->b));
				RegSetValueEx(hk, L"device", 0, REG_SZ, (BYTE *)buf, (_tcslen(buf) + 1) * sizeof(buf[0]));
				RegSetValueEx(hk, L"channel", 0, REG_DWORD, (BYTE *)&pDev->cannels[s], sizeof(pDev->cannels[s]));
				RegSetValueEx(hk, L"port", 0, REG_DWORD, (BYTE *)&port, sizeof(port));

				RegSetValueEx(hk, L"encrypt", 0, REG_DWORD, (BYTE *)&fEncrypt, sizeof(fEncrypt));
				RegSetValueEx(hk, L"authenticate", 0, REG_DWORD, (BYTE *)&fAuth, sizeof(fAuth));
				RegSetValueEx(hk, L"lan", 0, REG_DWORD, (BYTE *)&fLan, sizeof(fLan));
			}
			RegCloseKey(hk);
		}

		pcx->status = BthCreateDevice(s, pDev, pContext);

		res = (pcx->status == ERROR_SUCCESS);
	}
	__finally
	{
	}

    return res;
}
void MIC_BthDisconnect(MIC_BTD_SERVICE s, PVOID pContext)
{
	INT32 res = 0;
	MIC_BTH_CONTEXT *pcx = 0;

	__try
	{
		if(!pContext)
			__leave;
		
		pcx = (MIC_BTH_CONTEXT *)pContext;
		if(pcx->size < sizeof(MIC_BTH_CONTEXT))
			__leave;

		pcx->status = ERROR_INVALID_PARAMETER;


		if(pcx->clients[s] == (void *)-1)
			__leave;

		DeregisterBluetoothCOMPort(pcx->clients[s]);
		pcx->clients[s] = (void *)-1;

		res = (pcx->status == ERROR_SUCCESS);
	}
	__finally
	{
	}

    return;
}

HWND notify = 0;
UINT32 InquiryCallback(MIC_BT_DEVICESLIST *pvList, MIC_BT_DEVICEDETAILS *pDev)
{
	MIC_BT_DEVICESLIST *btdl;
	MIC_BT_DEVICEDETAILS *btd;
	MIC_BT_DEVICESLIST *pvl = pvList;
	do
	{
		if(!pvl)
			break;
		if(!pvl->dev)
		{
			btd = new MIC_BT_DEVICEDETAILS;
			if(btd)
			{
				pvl->dev = btd;
				memcpy(pvl->dev, pDev, sizeof(MIC_BT_DEVICEDETAILS));
				pvl->next = 0;
				TCHAR buf[512];
				wsprintf(buf, L"%s(%04x%08x)", pvl->dev->tszName, GET_NAP(pvl->dev->b), GET_SAP(pvl->dev->b));
				int res = SendMessage(notify, LB_ADDSTRING, 0, (LPARAM)buf);
				res = SendMessage(notify, LB_SETITEMDATA, res, (LPARAM)pvl->dev);
				//if(!res)
				//	res = GetLastError();
//				MSG msg ; 
//				while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) 
//					DispatchMessage(&msg); 
				break;
			}
		}
		if(pvl->next)
		{
			pvl = pvl->next;
			continue;
		}

		btdl = new MIC_BT_DEVICESLIST;
		btdl->dev = 0;
		btdl->next = 0;
		pvl->next = btdl;
		pvl = pvl->next;
	}while(pvl);

	return 0;
}

UINT32 DiscoveryCallBack(MIC_BTD_SERVICE s, MIC_BT_DEVICEDETAILS *pDev)
{
	TCHAR sz[16] = {0};
	int stop = 0;

	if(s == BTD_MODEM)
		_tcscpy(sz, L"DUN");
	else if(s == BTD_PRINTER)
		_tcscpy(sz, L"PRN");
	else if(s == BTD_ASYNC)
		_tcscpy(sz, L"SER");
	else if(s == BTD_LAP)
		_tcscpy(sz, L"LAP");
	else if(s == BTD_OBEX_FTP)
		_tcscpy(sz, L"OFTP");
	else if(s == BTD_OBEX_OPP)
		_tcscpy(sz, L"OPUSH");
	else if(s == BTD_HEADSET)
		_tcscpy(sz, L"HS");
	else if(s == BTD_HANDSFREE)
		_tcscpy(sz, L"HF");

	TCHAR buf[512];
	int res = SendMessage(notify, LB_GETCURSEL, 0, 0);
	SendMessage(notify, LB_GETTEXT, res, (LPARAM)buf);
	if(s == BTD_LAP)
	{
		wsprintf(buf, L"%s(%04x%08x) LAN Access Point is found(%d)", pDev->tszName, GET_NAP(pDev->b), GET_SAP(pDev->b), pDev->cannels[s]);
		stop = 1;
	}
	else
	{
		wsprintf(buf, L"%s %s(%d)", buf, sz, pDev->cannels[s]);
	}
	SendMessage(notify, LB_DELETESTRING, res, 0);
	SendMessage(notify, LB_INSERTSTRING, res, (LPARAM)buf);
	SendMessage(notify, LB_SETCURSEL, res, 0);
	res = SendMessage(notify, LB_SETITEMDATA, res, (LPARAM)pDev);

	return stop;
}

HWND g_Wnd;
HANDLE g_evQuit = 0;
HANDLE g_evDoInquery = 0;
HANDLE g_evDoDiscovery = 0;
MIC_BTH_CONTEXT mbc = {0};
MIC_BT_DEVICESLIST mbdl = {0};

DWORD WINAPI BtWorkThread(LPVOID lpvParam)
{
	HANDLE ev[] = {g_evQuit, g_evDoInquery, g_evDoDiscovery};

	int quit = 0;
	while(!quit)
	{
	DWORD dwStat = WaitForMultipleObjects(sizeof(ev)/sizeof(ev[0]), ev, 0, INFINITE);
		switch(dwStat)
		{
			case (WAIT_OBJECT_0 + 0):
			{
				quit = 1;
				break;
			}
			case (WAIT_OBJECT_0 + 1):
			{
				SendMessage(notify, LB_RESETCONTENT, 0, 0);
				SetWindowText(g_Wnd, L"LAN Access Datat Teminal (Inquiring...)");
				mbc.pBLOB = &mbdl;
				int res = MIC_BthInquiry(&mbc, (PFN_BTH_INQUIRY_CALLBACK)InquiryCallback);
				if(!res)
					SetWindowText(g_Wnd, L"LAN Access Datat Teminal (Inquiring is failed)");
				else
				{
					if(mbc.status == ERROR_OPERATION_ABORTED)
						SetWindowText(g_Wnd, L"LAN Access Datat Teminal (Inquiring is aborted)");
					else
						SetWindowText(g_Wnd, L"LAN Access Datat Teminal");
				}
				break;
			}
			case(WAIT_OBJECT_0 + 2):
			{
				int res;
				SetWindowText(g_Wnd, L"LAN Access Datat Teminal (Discovering...)");
				res = SendMessage(notify, LB_GETCURSEL, 0, 0);
				if(res >= 0)
				{
					TCHAR buf[512];
					SendMessage(g_Wnd, LB_GETTEXT, res, (LPARAM)buf);

					MIC_BT_DEVICEDETAILS *dev = (MIC_BT_DEVICEDETAILS *)SendMessage(notify, LB_GETITEMDATA, res, 0);
					if(dev)
					{
						wsprintf(buf, L"%s(%04x%08x)", dev->tszName, GET_NAP(dev->b), GET_SAP(dev->b));
						SendMessage(notify, LB_DELETESTRING, res, 0);
						SendMessage(notify, LB_INSERTSTRING, res, (LPARAM)buf);
						SendMessage(notify, LB_SETCURSEL, res, 0);
						res = SendMessage(notify, LB_SETITEMDATA, res, (LPARAM)dev);
					}
					res = MIC_BthDiscoveryServices(dev, (PFN_BTH_DISCOVERY_CALLBACK)DiscoveryCallBack, &mbc);
				}
				else
				{
					res = 0;
				}
				if(!res)
					SetWindowText(g_Wnd, L"LAN Access Datat Teminal (Discovering is failed)");
				else
				{
					if(mbc.status == ERROR_OPERATION_ABORTED)
						SetWindowText(g_Wnd, L"LAN Access Datat Teminal (Discovering is aborted)");
					else
						SetWindowText(g_Wnd, L"LAN Access Datat Teminal");
				}
				break;
			}
		}
	}
	return 1;
}
BOOL CALLBACK Pin(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static MIC_BT_DEVICEDETAILS *dev = 0;
    switch(uMsg)
	{
		case WM_INITDIALOG:
        {
            SetForegroundWindow(hWnd);
            SetWindowText(GetDlgItem (hWnd, IDC_PIN), L"");
            SetFocus(GetDlgItem (hWnd, IDC_PIN));
            WCHAR szString[256];
			int res = SendMessage(notify, LB_GETCURSEL, 0, 0);
			dev = (MIC_BT_DEVICEDETAILS *)SendMessage(notify, LB_GETITEMDATA, res, 0);
            wsprintf(szString, L"Enter pin code for %04x%08x device", GET_NAP(dev->b), GET_SAP(dev->b));
            SetWindowText(hWnd, szString);
			return 0;
        }
		case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
				case IDOK:
                {
					WCHAR szPin[64] = {0};
                    WCHAR *pszPIN = szPin;

                    GetWindowText(GetDlgItem (hWnd, IDC_PIN), szPin, 64);

					unsigned char pin[16] = {0};
                    int cPin = 0;

                    while((*pszPIN) && (cPin < 15))
                        pin[cPin++] = (unsigned char)*(pszPIN++);
					pin[cPin] = 0;

                    if(cPin)
                        MIC_BthAuthenticate(dev, &mbc, pin);

                    EndDialog(hWnd, 0);

					return 1;
                }
				case IDCANCEL:
				{
                    MIC_BthRevokePin(dev, &mbc);
                    EndDialog (hWnd, 0);
					return 1;
                }
            }
			break;
        }
    }

    return 0;
}

int WINAPI mainDlgProc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
	INT32 res;

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			g_Wnd = w;
			g_evQuit = CreateEvent(0, 0, 0, 0);
			g_evDoInquery = CreateEvent(0, 0, 0, 0);
			g_evDoDiscovery = CreateEvent(0, 0, 0, 0);
			notify = GetDlgItem(w, IDC_DEV);
			mbc.size = sizeof(mbc);
			memset(&mbc.clients, -1, sizeof(mbc.clients));
			res = MIC_BthEnableScan(0, &mbc);
			res = MIC_BthEnableScan(1, &mbc);
			CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)BtWorkThread, 0, 0, 0));

			break;
		}
		case WM_COMMAND:
		{
			switch(LOWORD(wp))
			{
				case IDR_INQ:
				{
					MIC_BthCancelInquiry(&mbc);
					MIC_BthDesroyDevicesList(&mbdl, &mbc);
					SetEvent(g_evDoInquery);
					break;
				}
				case IDR_DISC:
				{
					MIC_BthCancelInquiry(&mbc);
					SetEvent(g_evDoDiscovery);
					break;
				}
				case IDC_PAIR:
				{
					DialogBox(g_hInst, MAKEINTRESOURCE(IDD_PIN), w, Pin);
					break;
				}
				case IDR_LAP_CONNECT:
				{
					static int connected = 0;

					if(connected)
					{
						MIC_BthDisconnect(BTD_LAP, &mbc);

						SetWindowText(GetDlgItem(w, IDR_LAP_CONNECT), L"Create LAN Access");

						connected = 0;
					}
					else
					{
						res = SendMessage(notify, LB_GETCURSEL, 0, 0);
						if(res >= 0)
						{
							MIC_BT_DEVICEDETAILS *dev = (MIC_BT_DEVICEDETAILS *)SendMessage(notify, LB_GETITEMDATA, res, 0);
							MIC_BthConnect(BTD_LAP, 4, dev, &mbc);
							SetWindowText(GetDlgItem(w, IDR_LAP_CONNECT), L"Destroy LAN Access");
							connected = 1;
						}
					}
					break;
				}
				case IDOK:
				{
					DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), w, About);
					break;
				}
				case IDCANCEL:
					SetEvent(g_evQuit);
					MIC_BthCancelInquiry(&mbc);
					MIC_BthDisconnect(BTD_LAP, &mbc);
					MIC_BthDesroyDevicesList(&mbdl, &mbc);
					CloseHandle(g_evQuit);
					CloseHandle(g_evDoInquery);
					CloseHandle(g_evDoDiscovery);
					EndDialog(w, 1);
					return 1;
				default:
					break;
			}
		}
		default:
			break;
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	g_hInst = hInstance;
	return (int) DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, mainDlgProc);
}


#ifndef WIN32_PLATFORM_WFSP
// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
#ifndef SHELL_AYGSHELL
            RECT rectChild, rectParent;
            int DlgWidth, DlgHeight;	// dialog width and height in pixel units
            int NewPosX, NewPosY;

            // trying to center the About dialog
            if (GetWindowRect(hDlg, &rectChild)) 
            {
                GetClientRect(GetParent(hDlg), &rectParent);
                DlgWidth	= rectChild.right - rectChild.left;
                DlgHeight	= rectChild.bottom - rectChild.top ;
                NewPosX		= (rectParent.right - rectParent.left - DlgWidth) / 2;
                NewPosY		= (rectParent.bottom - rectParent.top - DlgHeight) / 2;
				
                // if the About box is larger than the physical screen 
                if (NewPosX < 0) NewPosX = 0;
                if (NewPosY < 0) NewPosY = 0;
                SetWindowPos(hDlg, 0, NewPosX, NewPosY,
                    0, 0, SWP_NOZORDER | SWP_NOSIZE);
            }
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            {
                // Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
            }
#endif // SHELL_AYGSHELL

            return (INT_PTR)TRUE;

        case WM_COMMAND:
#ifndef SHELL_AYGSHELL
            if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            if (LOWORD(wParam) == IDOK)
#endif
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return (INT_PTR)TRUE;

    }
    return (INT_PTR)FALSE;
}
#endif // !WIN32_PLATFORM_WFSP
