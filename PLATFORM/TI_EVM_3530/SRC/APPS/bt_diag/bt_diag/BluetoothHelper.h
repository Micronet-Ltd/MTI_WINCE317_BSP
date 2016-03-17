#include <windows.h>
#include <bt_api.h>

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
} MIC_BTD_SERVICE;

typedef struct tag_BTH_CONTEXT
{
	UINT32 size; // sizeof(CONTEXT)
	UINT32 status; // operation status
	PVOID pBLOB;
	HANDLE Scan;
	HANDLE Lookup;
	UINT32 AbortInqiry;
	UINT32 AbortDiscovery;
}MIC_BTH_CONTEXT;

typedef struct tag_BT_DEVICEDETAILS
{
	BT_ADDR	b; // see MSDN
	unsigned	uClassOfDevice; // Class of Device see MSDN
	unsigned	uSrvcMask;	    // Services Mask
	unsigned 	channels[BTD_CLASSES];
	TCHAR		tszName[256];
}MIC_BT_DEVICEDETAILS;

typedef struct tag_BT_DEVICESLIST
{
	MIC_BT_DEVICEDETAILS *dev;
	tag_BT_DEVICESLIST	 *next;
}MIC_BT_DEVICESLIST;

typedef unsigned long (*PFN_BTH_INQUIRY_CALLBACK)(MIC_BT_DEVICESLIST *pvList, MIC_BT_DEVICEDETAILS *pDev);
typedef unsigned long (*PFN_BTH_DISCOVERY_CALLBACK)(MIC_BTD_SERVICE Srvc, MIC_BT_DEVICEDETAILS *pDev);

#define MUTEX_INQUIRY_NAME L"Eneter Inquiry"
#define BTHNS_ABORT_CURRENT_INQUIRY 0xfffffffd

#define HIDServiceClassID_UUID16	0x00001124
const UUID HIDServiceClassID_UUID			= {0x00001124, 0x0000, 0x1000, {0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}};
const UUID ActiveSyncServiceClassID_UUID	= {0x0350278F, 0x3DCA, 0x4e62, {0x83, 0x1D, 0xA4, 0x11, 0x65, 0xFF, 0x90, 0x6C}};

BOOL MIC_BthEnableScan(BOOL en, PVOID pContext);
BOOL MIC_BthInquiry(PVOID pContext, PFN_BTH_INQUIRY_CALLBACK pfnCallback);
BOOL MIC_BthCancelInquiry(PVOID pContext);
BOOL MIC_BthDesroyDevicesList(MIC_BT_DEVICESLIST *pList, PVOID pContext);
BOOL MIC_BthDiscoveryServices(MIC_BT_DEVICEDETAILS *pDev, PFN_BTH_DISCOVERY_CALLBACK pCallback, PVOID pContext);
BOOL MIC_BthAuthenticate(MIC_BT_DEVICEDETAILS *pDev, PVOID pContext, UINT8 *pin);
BOOL MIC_BthRevokePin(MIC_BT_DEVICEDETAILS *pDev, PVOID pContext);
BOOL MIC_BthCancelDiscovery(PVOID pContext);
BOOL Mic_BthIsAuthenticated(MIC_BT_DEVICEDETAILS *pDev, PVOID pContext);