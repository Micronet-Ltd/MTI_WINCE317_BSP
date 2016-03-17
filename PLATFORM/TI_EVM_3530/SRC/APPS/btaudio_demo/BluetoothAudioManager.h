#include <windows.h>
#include <bt_api.h>
#include <ag_hs_api.h>
#include "bt.h"

typedef void (*AudioGatewayEventHandler)(DWORD, DWORD, void*); 

static HANDLE	g_hBT = NULL;
static BT_ADDR	gBTAddr = 0ll;

void dummy_callback(DWORD, DWORD, void*) { };

class BluetoothAudioManager
{
	friend DWORD WINAPI AudioThread(LPVOID lpParam);

private:
	BOOL m_bIsActive;
	HANDLE m_hThr;
	AudioGatewayEventHandler m_eventCallback;
	void *m_pUserData;

public:
	BluetoothAudioManager()
	{
		m_pUserData = NULL;
		m_bIsActive = FALSE;
		m_eventCallback = &dummy_callback;

		if (findLastPaired(&gBTAddr))
		{
			g_hBT = btAudioOpen(gBTAddr);
			m_hThr = CreateThread(0, 0, AudioThread, this, 0, 0 );
		}
	};

	~BluetoothAudioManager()
	{
		Deactivate();
		btAudioClose();
		DWORD dwReason = WaitForSingleObject(m_hThr, 500);
		if (dwReason != WAIT_OBJECT_0)
			TerminateThread(m_hThr, -1);
	}

	void Activate() 
	{ 
		m_bIsActive = btAudioConnect(BT_AG_HS_ACL | BT_AG_HS_SCO);
	};

	void Deactivate() 
	{
		if (btAudioDisconnect(BT_AG_HS_ACL | BT_AG_HS_SCO))
			m_bIsActive = FALSE;
	};
	
	BOOL IsActive() 
	{ 
		return m_bIsActive; 
	};

	BOOL IsPaired()
	{
		return findLastPaired(&gBTAddr);
	};

	void RegisterEventCallback(AudioGatewayEventHandler fn, void *pUserData)
	{
		m_pUserData = pUserData;
		m_eventCallback = fn;
	}
};

DWORD WINAPI AudioThread(LPVOID lpParam)
{
	INT32			iRet;
	AGHS_MSG		AgMsg;
	BluetoothAudioManager *pThis = (BluetoothAudioManager *)lpParam;

	while(1)
	{
		iRet =  MIC_AGHSWaitForEvent(g_hBT, &AgMsg);

		switch(iRet)
		{
			case  AG_HS_OK:
			{
				switch(AgMsg.hsEvent)
				{
					case  EV_AG_HS_BUTTON_PRESS:
					case  EV_AG_HS_MIC_GAIN:
					case  EV_AG_HS_SPEAKER_GAIN:
					case  EV_AG_HS_CTRL:
					case  EV_AG_HS_AUDIO:
						pThis->m_eventCallback(AgMsg.hsEvent, AgMsg.dwParam, pThis->m_pUserData);
						break;
				
					default:
					{
						RETAILMSG(1, (L"[UNKNOWN AUDIO GATEWAY EVENT]\r\n"));
					} 
					break;
				}
			} 
			break;
 
			case  AG_HS_SERVICE_INVALID_PARAM:
			case  AG_HS_MSG_CREATE_ERROR:
			case  AG_HS_MSG_READ_ERROR:
				return -2;

			case  AG_HS_DEV_CLOSED:
				return 0;

			default: // unknown error
				return -3;
		}
	}

	return(0);
};

