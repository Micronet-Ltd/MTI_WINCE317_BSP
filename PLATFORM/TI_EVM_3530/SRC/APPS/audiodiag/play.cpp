// PlayWav.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "WaveFile.h"
#include "WavePlayer.h"
#include "WaveRecorder.h"

#define MYFILE TEXT("default.wav")
#define MAX_ERRMSG 256

#define		WITH_BT			0//without bluetouth

#define			MAX_VOLUME	0x7F
#define			MIN_VOLUME	0


CWavePlayer		g_pl;
CWaveRecorder	g_rec;
SCROLLINFO		si;

MMRESULT play(HWAVEOUT hwo, void *ev, WAVEHDR *whdr, wave_file *waveFile)
{
//	unsigned long old_vol;
	MMRESULT mmres;

//	mmres = waveOutGetVolume(hwo, &old_vol);
//	mmres = waveOutSetVolume(hwo, 0xFAFFFAFF);
//	if(mmres != MMSYSERR_NOERROR)
//		return mmres;
	
	// Play buffer
	waveFile->get(whdr->lpData, whdr->dwBufferLength);

	mmres = waveOutPrepareHeader(hwo, whdr, sizeof(WAVEHDR));	
	if(mmres != MMSYSERR_NOERROR)
		return mmres;

	mmres = waveOutWrite(hwo, whdr, sizeof(WAVEHDR));	
	if(mmres != MMSYSERR_NOERROR)
		return mmres;

	// Wait for audio to finish playing
	while(!(whdr->dwFlags & WHDR_DONE))
	{
		MSG msg;
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		WaitForSingleObject(ev, 10);
	}

	// Clean up
	mmres = waveOutUnprepareHeader(hwo, whdr, sizeof(WAVEHDR));	
	if(mmres != MMSYSERR_NOERROR)
		return mmres;

	return MMSYSERR_NOERROR;
}

MMRESULT record(HWAVEIN hwi, void *ev, WAVEHDR *whdr, unsigned long duration)
{
	MMRESULT mmres;

    // prepare the buffer for capture
    mmres = waveInPrepareHeader(hwi, whdr, sizeof(WAVEHDR));
	if(mmres != MMSYSERR_NOERROR)
		return mmres;

    // submit the buffer for capture
    mmres = waveInAddBuffer(hwi, whdr, sizeof(WAVEHDR));
	if(mmres != MMSYSERR_NOERROR)
		return mmres;

    mmres = waveInStart(hwi);
	if(mmres != MMSYSERR_NOERROR)
		return mmres;

	// Wait for audio to finish playing
	while(!(whdr->dwFlags & WHDR_DONE))
	{
		if(WAIT_OBJECT_0 != WaitForSingleObject(ev, duration + 1000))
		{
			mmres = waveInReset(hwi);
			break;
		}
	}

	waveInStop(hwi);

    // now clean up: unprepare the buffer
    mmres = waveInUnprepareHeader(hwi, whdr, sizeof(WAVEHDR));

	return mmres;
}

int WINAPI mainDlgProc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWAVEOUT hwo = 0;
	static HWAVEIN  hwi = 0;
//	static void *g_done = 0;
	static UINT devId = 0;
	static unsigned long volOut = 0;
	MMRESULT mmres;

	switch(msg)
	{
		case WM_INITDIALOG:
		{
/*
			WAVEFORMATEX wfx;
			wfx.cbSize = 0;
			wfx.wFormatTag = WAVE_FORMAT_PCM;
			wfx.wBitsPerSample = 16;
			wfx.nSamplesPerSec = 44100;
			wfx.nChannels = 1;
			wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
			wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

			for(devId = 0; devId < waveOutGetNumDevs(); devId++)
			{
				mmres = waveOutOpen(&hwo, devId, &wfx, 0, 0, CALLBACK_NULL);
				if(mmres == MMSYSERR_NOERROR)
					break;
			}
			if(mmres == MMSYSERR_NOERROR)
			{
				TCHAR vol[128];
				mmres = waveOutSetVolume((HWAVEOUT)devId, 0xE0FFE0FF);
				mmres = waveOutGetVolume((HWAVEOUT)devId, &volOut);
				mmres = waveOutClose(hwo);
				unsigned long volLeft = ((volOut & 0x0000FFFF) >> 9);
				unsigned long volRight = (((volOut & 0xFFFF0000) >>16) >> 9);
				volLeft &= 0x7F;
				volRight &= 0x7F;
				wsprintf(vol, L"L(R) %d(%d)", volLeft, volRight);

				//SendDlgItemMessage( w, IDC_SCROL_VOL, SBM_SETRANGE, 0, 0x7F );

				si.cbSize	= sizeof( si ); 
				si.fMask	= SIF_PAGE | SIF_POS | SIF_RANGE; //SIF_ALL; 
				si.nMin		= -127;//0; 
				si.nMax		= 0;//127; 
				si.nPage	= 5; 
				si.nPos		= -(int)volRight; 
				si.nTrackPos= 0;
				
				SetScrollInfo( GetDlgItem( w, IDC_SCROL_VOL ), SB_CTL, &si, TRUE ); 
				
				si.fMask	= SIF_POS;//for future use

				SetWindowText(GetDlgItem(w, IDC_VOLUME), vol);
			}

			SetDlgItemInt(w, IDC_EDIT_SR, 8000, 0);
*/			
			break;
		}
/*		case WM_VSCROLL:
		{
			mmres = waveOutGetVolume((HWAVEOUT)devId, &volOut);
			if(mmres == MMSYSERR_NOERROR)
			{
				TCHAR	vol[128];

				unsigned long volLeft = ((volOut & 0x0000FFFF) >> 9);
				unsigned long volRight = (((volOut & 0xFFFF0000) >>16) >> 9);
				
				long	Delta;

				switch( (int)LOWORD(wp) )
				{
					case	SB_LINEDOWN:
					{
						Delta = 1;
					}
					break;
					case	SB_LINEUP:
					{
						Delta = -1;
					}
					break;
					case	SB_PAGEUP:
					{
						Delta = -(long)si.nPage;
					}
					break;
					case	SB_PAGEDOWN:
					{
						Delta = si.nPage;
					}
					break;

					default:
						return 0;
						//break;
				}
				if( si.nMax < si.nPos + Delta )
					Delta = si.nMax - si.nPos;
				else if( si.nMin > si.nPos + Delta )
					Delta = si.nMin - si.nPos;

				volLeft		-= Delta;
				volRight	-= Delta;

				volLeft &= 0x7F;
				volRight &= 0x7F;
				volOut = (volLeft << 9) | ((volRight << 9) << 16);
				mmres = waveOutSetVolume((HWAVEOUT)devId, volOut);
				mmres = waveOutClose(hwo);

				wsprintf(vol, L"%d", volLeft );
				SetWindowText(GetDlgItem(w, IDC_VOLUME), vol);

				si.nPos += Delta; 
				SetScrollInfo( GetDlgItem( w, IDC_SCROL_VOL ), SB_CTL, &si, TRUE ); 
			}
			return 1;
		}
*/
		case WM_COMMAND:
		{
			switch(LOWORD(wp))
			{
/*				case IDC_VOLUP:
				{
					WAVEFORMATEX wfx;
					wfx.cbSize = 0;
					wfx.wFormatTag = WAVE_FORMAT_PCM;
					wfx.wBitsPerSample = 16;
					wfx.nSamplesPerSec = 44100;
					wfx.nChannels = 1;
					wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
					wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

					for(devId = 0; devId < waveOutGetNumDevs(); devId++)
					{
						mmres = waveOutOpen(&hwo, devId, &wfx, 0, 0, WAVE_FORMAT_QUERY);//CALLBACK_NULL);
						if(mmres == MMSYSERR_NOERROR)
							break;
					}
					if(mmres == MMSYSERR_NOERROR)
					{
						TCHAR vol[128];
						mmres = waveOutGetVolume((HWAVEOUT)devId, &volOut);
						unsigned long volLeft = ((volOut & 0x0000FFFF) >> 9);
						unsigned long volRight = (((volOut & 0xFFFF0000) >>16) >> 9);
						volLeft++;
						volRight++;
						volLeft &= 0x7F;
						volRight &= 0x7F;
						volOut = (volLeft << 9) | ((volRight << 9) << 16);
						mmres = waveOutSetVolume((HWAVEOUT)devId, volOut);
						mmres = waveOutClose(hwo);

						wsprintf(vol, L"Volume L(R) is %d(%d)", volLeft, volRight);
						SetWindowText(GetDlgItem(w, IDC_VOLUME), vol);
					}
					return 1;
				}
				case IDC_VOLDOWN:
				{
					WAVEFORMATEX wfx;
					wfx.cbSize = 0;
					wfx.wFormatTag = WAVE_FORMAT_PCM;
					wfx.wBitsPerSample = 16;
					wfx.nSamplesPerSec = 44100;
					wfx.nChannels = 1;
					wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
					wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

					for(devId = 0; devId < waveOutGetNumDevs(); devId++)
					{
						mmres = waveOutOpen(&hwo, devId, &wfx, 0, 0, WAVE_FORMAT_QUERY);//CALLBACK_NULL);
						if(mmres == MMSYSERR_NOERROR)
							break;
					}
					if(mmres == MMSYSERR_NOERROR)
					{
						TCHAR vol[128];
						mmres = waveOutGetVolume((HWAVEOUT)devId, &volOut);
						unsigned long volLeft = ((volOut & 0x0000FFFF) >> 9);
						unsigned long volRight = (((volOut & 0xFFFF0000) >>16) >> 9);
						volLeft--;
						volRight--;
						volLeft &= 0x7F;
						volRight &= 0x7F;
						volOut = (volLeft << 9) | ((volRight << 9) << 16);
						mmres = waveOutSetVolume((HWAVEOUT)devId, volOut);
						mmres = waveOutClose(hwo);

						wsprintf(vol, L"Volume L(R) is %d(%d)", volLeft, volRight);
						SetWindowText(GetDlgItem(w, IDC_VOLUME), vol);
					}
					return 1;
				}
				
*/
			/*case IDC_PO:
				{
					static int paused = 0;
					
					if( !paused )
					{
						g_pl.Pause();
					}
					else
					{	g_pl.Restart();
					}
					paused = !paused;
					return 1;
				}
				case IDC_STOP:
				{
					g_rec.Stop();
					g_pl.Stop();

					EnableWindow(GetDlgItem(w, IDC_PLAY),	1);
					EnableWindow(GetDlgItem(w, IDC_RECORD), 1);

					return 1;
				}
				case IDC_STOP_PLAY:
				{
					g_pl.Stop();
					g_rec.Stop();

					EnableWindow(GetDlgItem(w, IDC_PLAY),	1);
					EnableWindow(GetDlgItem(w, IDC_RECORD), 1);

					return 1;
				}
				case IDC_PLAY:
				{	
					EnableWindow(GetDlgItem(w, IDC_PLAY),	0);
					EnableWindow(GetDlgItem(w, IDC_RECORD), 0);

					OPENFILENAME ofn = {0};
					//wave_file waveFile;
					TCHAR file[256] = L"default.wav";
					TCHAR dir[256] = TEXT("\\doc\\");

					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = w;
					ofn.lpstrFilter = L"Waveform Audio(*.wav)\0*.wav\0\0";
					ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
					ofn.lpstrTitle  = _T("Choose Playing File");
					ofn.lpstrFile   = file;
					ofn.lpstrInitialDir = dir;
					ofn.nMaxFile    = sizeof(file);
					int ret = GetSaveFileName(&ofn);
					ret = GetLastError();
					//if(waveFile.open(file))
					//{
						g_pl.Start( file );

					//}
					//waveFile.close();
//					EnableWindow(GetDlgItem(w, IDC_PLAY), 1);
//					EnableWindow(GetDlgItem(w, IDC_RECORD), 1);
					return 1;
				}
				case IDC_RECORD:
				{
					EnableWindow(GetDlgItem(w, IDC_PLAY),	0);
					EnableWindow(GetDlgItem(w, IDC_RECORD), 0);

					DeleteFile(TEXT("\\doc\\rec.wav"));
					void *g_done = CreateEvent(0, 0, 0, TEXT("Record Done"));
					// set up the wave format structure
					WAVEFORMATEX wfx;
					wfx.cbSize = 0;
					wfx.wFormatTag = WAVE_FORMAT_PCM;
					wfx.wBitsPerSample = 16;
					wfx.nSamplesPerSec = GetDlgItemInt(w, IDC_EDIT_SR, 0, 0);
;
//					wfx.nSamplesPerSec = 11025; WAVE_FORMAT_1M08
//					wfx.nSamplesPerSec = 16000;
//					wfx.nSamplesPerSec = 22050;
//					wfx.nSamplesPerSec = 44100;
//					wfx.nSamplesPerSec = 48000;
					wfx.nChannels = (BST_CHECKED == SendMessage(GetDlgItem(w, IDC_CHECK_STEREO), BM_GETCHECK, 0, 0))?2:1;
					wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
					wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

					g_rec.Start( TEXT("\\doc\\rec.wav"), &wfx );

					return 1;
				}
*/
				case IDOK:
				{
					break;
				}
				case IDCANCEL:
					EndDialog(w, 1);
					return 1;
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
	return 0;
}
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
	int ret = DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, mainDlgProc);

//	g_rec.Stop();
//	g_pl.Stop();

	return 1;
	
	return 0;
}

