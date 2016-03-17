#include "BluetoothHelper.h"
#include <winsock2.h>
#include <bt_sdp.h>
#include <bthapi.h>

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
			pcx->status = BthNsLookupServiceEnd(pcx->Scan);
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

unsigned short BthUUID(MIC_BTD_SERVICE service)
{
	if(service == BTD_MODEM)
		return DialupNetworkingServiceClassID_UUID16;
	else if((service == BTD_PRINTER) || (service == BTD_ASYNC))
		return SerialPortServiceClassID_UUID16;
	else if(service == BTD_LAP)
		return LANAccessUsingPPPServiceClassID_UUID16; //GenericNetworkingServiceClassID_UUID16;
	else if(service == BTD_PAN)
		return PANUServiceClassID_UUID16;
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
		memset(pDev->channels, 0xFF, sizeof(pDev->channels));

		RETAILMSG(1, (L"btsvc:: BtDiscoveryServices\r\n"));

		for(int i = BTD_MODEM; i < BTD_CLASSES; i++)
		{
			if(BthIsService(&pDev->b, (MIC_BTD_SERVICE)i))
			{
				pDev->channels[i] = BthRfCommChannel(pDev, (MIC_BTD_SERVICE)i);
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

BOOL Mic_BthIsAuthenticated(MIC_BT_DEVICEDETAILS *pDev, PVOID pContext)
{
	INT32 res = 0;
	MIC_BTH_CONTEXT *pcx = 0;

	__try
	{
		if (!pContext)
			__leave;
		
		pcx = (MIC_BTH_CONTEXT *)pContext;
		if (pcx->size < sizeof(MIC_BTH_CONTEXT))
			__leave;

		pcx->status = ERROR_INVALID_PARAMETER;

		if (!pDev)
			__leave;

		unsigned char key[16] = {0};
		pcx->status = BthGetLinkKey(&pDev->b, key);

		res = (pcx->status == ERROR_SUCCESS);
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
