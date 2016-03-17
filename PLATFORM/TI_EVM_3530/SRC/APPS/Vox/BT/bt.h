#ifndef _BT_H
#define _BT_H

#include <bt_api.h>
//#include <winsock2.h>

#define RK_AUDIO_GATEWAY            _T("SOFTWARE\\Microsoft\\Bluetooth\\AudioGateway")
#define RK_AUDIO_GATEWAY_DEVICES    (RK_AUDIO_GATEWAY TEXT("\\Devices"))
#define RK_AG_SERVICE               _T("Services\\BTAGSVC")


#define BT_AG_HS_ACL           0x01 
#define BT_AG_HS_SCO           0x02 

// According to BT specifications
#define BT_VOL_MIN             0
#define BT_VOL_MAX             15

#ifdef __cplusplus
	extern "C" {
#endif


BOOL findLastPaired(BT_ADDR *bt);

HANDLE  btAudioOpen(BT_ADDR btAddr);
BOOL    btAudioClose(void);
BOOL    isBTAbsent(void);

BOOL    btAudioConnect(DWORD hsConnect);
BOOL    btAudioDisconnect(DWORD hsConnect);

BOOL  btSetSpkVol(USHORT vol);
BOOL  btSetMicVol(USHORT vol);
BOOL  btGetSpkVol(USHORT *pVol);
BOOL  btGetMicVol(USHORT *pVol);

#ifdef __cplusplus
	}
#endif	

#endif // _BT_H