// btaudio_demo.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "btaudio_demo.h"
#include <windows.h>
#include <commctrl.h>
#include "BluetoothAudioManager.h"
#include "MixerControl.h"
#include "Sound.h"

typedef enum HeadsetVolumeMode { RecordingGain = 0, PlaybackVolume };
// Global Variables:
HeadsetVolumeMode headsetVolumeMode = RecordingGain;
BluetoothAudioManager bthAudio;
MixerControl bthAudioOnOff, bthAudioRecGain, bthAudioRecMute, bthAudioPlayVol, bthAudioPlayMute;
Sound sound;
void *soundBuffer[2];

LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	INITCOMMONCONTROLSEX ctrl = {0};
	ctrl.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ctrl.dwICC = ICC_BAR_CLASSES;

	if (InitCommonControlsEx(&ctrl))
 		return DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)WndProc);
	
	return 1;
}

void AGCallback(DWORD dwEvent, DWORD dwParam, void *pUserData) 
{ 
	HWND hDlg = (HWND)pUserData;
	switch (dwEvent)
	{
	case EV_AG_HS_BUTTON_PRESS:
		SetDlgItemText(hDlg, IDC_STATIC_STATUS, L"Headset button pressed");
		break;

	case EV_AG_HS_MIC_GAIN:
		SetDlgItemText(hDlg, IDC_STATIC_STATUS, L"Headset mic gain changed");
		break;

	case EV_AG_HS_SPEAKER_GAIN:
		SetDlgItemText(hDlg, IDC_STATIC_STATUS, L"Headset speaker gain changed");
		break;

	case EV_AG_HS_AUDIO:
		SetDlgItemText(hDlg, IDC_STATIC_STATUS, dwParam ? L"Connected to headset" : L"Disconnected");
		break;

	default:
	//	DebugBreak();
		break;
	}
};

void SoundCallback(void *sender, void *buffer, void *userData)
{
	sound.Record(buffer == soundBuffer[0] ? soundBuffer[1] : soundBuffer[0]);
	sound.Play(buffer);
}

LRESULT CALLBACK WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
	{ 
	case WM_NOTIFY: 
		{
			LPNMHDR lpNotifyMsg = (LPNMHDR)lParam;
			int pos = SendDlgItemMessage(hDlg, wParam, TBM_GETPOS, 0 ,0);
			if (wParam == IDC_SLIDER_REC)
			{
				if (bthAudioRecGain.GetValue() != pos)
					bthAudioRecGain.SetValue(pos);
			}
			else if (wParam == IDC_SLIDER_PLAY)
			{
				if (bthAudioPlayVol.GetValue() != pos)
					bthAudioPlayVol.SetValue(pos);
			}
		}
		break;

	case WM_PAINT:
		break;

	case WM_INITDIALOG: 
		{
			SetDlgItemText(hDlg, IDC_STATIC_STATUS, L"");

			if (!bthAudio.IsPaired())
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_ACTIVATION), FALSE);
			
			bthAudio.RegisterEventCallback(AGCallback, hDlg);
			
			if (!bthAudioOnOff.CreateByShortName(TEXT("BthAudioOnOff")))
				SetDlgItemText(hDlg, IDC_STATIC_STATUS, L"BthAudio mixer control not available");

			if (!bthAudioRecGain.CreateByShortName(TEXT("BthAudioRecGain")))
				EnableWindow(GetDlgItem(hDlg, IDC_SLIDER_REC), FALSE);
			else
				SendDlgItemMessage(hDlg, IDC_SLIDER_REC, TBM_SETPOS, TRUE , bthAudioRecGain.GetValue());

			if (!bthAudioRecMute.CreateByShortName(TEXT("BthAudioRecMute")))
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_REC_MUTE), FALSE);

			if (!bthAudioPlayVol.CreateByShortName(TEXT("BthAudioPlayVol")))
				EnableWindow(GetDlgItem(hDlg, IDC_SLIDER_PLAY), FALSE);
			else
				SendDlgItemMessage(hDlg, IDC_SLIDER_PLAY, TBM_SETPOS, TRUE , bthAudioPlayVol.GetValue());

			if (!bthAudioPlayMute.CreateByShortName(TEXT("BthAudioPlayMut")))
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_PLAY_MUTE), FALSE);

			SendDlgItemMessage(hDlg, IDC_SLIDER_REC, TBM_SETRANGEMAX, TRUE , bthAudioRecGain.GetMaximum());
			SendDlgItemMessage(hDlg, IDC_SLIDER_PLAY, TBM_SETRANGEMAX, TRUE , bthAudioPlayVol.GetMaximum());

			soundBuffer[0] = sound.AllocateBuffer(8000 * 2);
			soundBuffer[1] = sound.AllocateBuffer(8000 * 2);
			if (soundBuffer[0] && soundBuffer[1])
			{
				sound.RegisterOnRecorded(SoundCallback, NULL);
			}
			else
				SetDlgItemText(hDlg, IDC_STATIC_STATUS, L"Out of memory");

		}
		return (INT_PTR)TRUE;  

	case WM_COMMAND: 
		switch (LOWORD(wParam))
		{
		case IDC_CHECK_ACTIVATION:
			if (!bthAudio.IsActive())
				bthAudio.Activate();
			else
				bthAudio.Deactivate();

			bthAudioOnOff.SetValue(bthAudio.IsActive());
			CheckDlgButton(hDlg, wParam, bthAudioOnOff.GetValue());
			SendDlgItemMessage(hDlg, IDC_SLIDER_REC, TBM_SETPOS, TRUE , bthAudioRecGain.GetValue());
			SendDlgItemMessage(hDlg, IDC_SLIDER_PLAY, TBM_SETPOS, TRUE , bthAudioPlayVol.GetValue());
			break;

		case IDC_CHECK_ACTIVATION2:
			if (IsDlgButtonChecked(hDlg, wParam))
			{
				sound.Open();
				sound.Record(soundBuffer[0]);
			}
			else
				sound.Close();

			break;

		case IDC_CHECK_REC_MUTE:
			bthAudioRecMute.SetValue(!bthAudioRecMute.GetValue());
			CheckDlgButton(hDlg, wParam, bthAudioRecMute.GetValue());
			break;

		case IDC_CHECK_PLAY_MUTE:
			bthAudioPlayMute.SetValue(!bthAudioPlayMute.GetValue());
			CheckDlgButton(hDlg, wParam, bthAudioPlayMute.GetValue());
			break;

		case IDC_BUTTON_EXIT:
		case IDCANCEL:
			sound.Close();
			sound.DeallocateBuffer(soundBuffer[1]);
			sound.DeallocateBuffer(soundBuffer[0]);
			bthAudioOnOff.SetValue(0);
			bthAudio.Deactivate();
			EndDialog(hDlg, LOWORD(wParam)); 
			return (INT_PTR)TRUE;
		}
		break; 
	case WM_DESTROY:
		break;

	case WM_CLOSE:
		break;

	} 
	
	return (INT_PTR)FALSE;
}

