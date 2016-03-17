//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//

#include <windows.h>
#include "btagpub.h"


void usage()
{
    wprintf(L"btagconfig [-audio {on,off}]\r\n");
    wprintf(L"           [-disconnect]\r\n");
    wprintf(L"           [-connect]\r\n");
    wprintf(L"           [-autoanswer]\r\n");
    wprintf(L"           [-inband {on,off}]\r\n");
    wprintf(L"           [-powersave {on,off,query}]\r\n");
    wprintf(L"           [-outvol {n}]\r\n");
    wprintf(L"           [-invol {n}]\r\n");
    wprintf(L"\r\n");
    wprintf(L"      -audio:       Turn Bluetooth audio on/off\r\n");
    wprintf(L"      -disconnect:  Close AG control connection\r\n");
    wprintf(L"      -outvol:      Set speaker volume (range: 0-15)\r\n");
    wprintf(L"      -invol:       Set mic volume (range: 0-15)\r\n");
    wprintf(L"      -powersave:   Turn power-save mode on/off\r\n");
    wprintf(L"      -autoanswer:  Turn auto-answer mode on\r\n");
    wprintf(L"      -connect:     Open AG control connection\r\n");
    wprintf(L"      -inband:      Turn inband ring on/off\r\n");
}

int wmain(int argc, WCHAR **argv)
{

    HANDLE h = CreateFile(L"BAG0:",0,0,NULL,OPEN_EXISTING,0,NULL);
    if (INVALID_HANDLE_VALUE == h) {
        wprintf(L"Error calling CreateFile on Audio Gateway.\r\n");
        return 0;
    }

    if (argc == 2) {
        if (0 == wcscmp(argv[1], L"-disconnect")) {
            wprintf(L"Closing Bluetooth control connection...\n");
            BOOL fStatus = DeviceIoControl(h,IOCTL_AG_CLOSE_CONTROL,NULL,0,NULL,0,NULL,NULL);
            if (FALSE == fStatus) {
                wprintf(L"Operation failed: %d.\n", GetLastError());
            }
            else {
                wprintf(L"Operation successful.\n"); 
            }
            
            CloseHandle(h);
            return 0;
        } else if (0 == wcscmp(argv[1], L"-connect")) {
            wprintf(L"Opening Bluetooth control connection...\n");
            BOOL fStatus = DeviceIoControl(h,IOCTL_AG_OPEN_CONTROL,NULL,0,NULL,0,NULL,NULL);
            if (FALSE == fStatus) {
                wprintf(L"Operation failed: %d.\n", GetLastError());
            }
            else {
                wprintf(L"Operation successful.\n"); 
            }
            
            CloseHandle(h);
            return 0;                
        } else if (0 == wcscmp(argv[1], L"-autoanswer")) {
            DWORD dwData = 1;
            wprintf(L"Setting auto-answer option...\n");            
            BOOL fStatus = DeviceIoControl(h,IOCTL_AG_SET_USE_HF_AUDIO,(LPVOID)&dwData,sizeof(dwData),NULL,0,NULL,NULL);
            if (FALSE == fStatus) {
                wprintf(L"Operation failed: %d.\n", GetLastError());
            }
            else {
                wprintf(L"Operation successful.\n"); 
            }
            
            CloseHandle(h);
            return 0;
        }
    }
    else if (argc == 3) {
        if (0 == wcscmp(argv[1], L"-audio")) {
            if (0 == wcscmp(argv[2], L"on")) {
                wprintf(L"Opening Bluetooth audio...\n");
                
                BOOL fStatus = DeviceIoControl(h,IOCTL_AG_OPEN_AUDIO,NULL,0,NULL,0,NULL,NULL);
                if (FALSE == fStatus) {
                    wprintf(L"Operation failed: %d.\n", GetLastError());
                }
                else {
                    wprintf(L"Operation successful.\n");
                }
            
                CloseHandle(h);
                return 0;
            }
            else if (0 == wcscmp(argv[2], L"off")) {
                wprintf(L"Closing Bluetooth audio...\n");
                
                BOOL fStatus = DeviceIoControl(h,IOCTL_AG_CLOSE_AUDIO,NULL,0,NULL,0,NULL,NULL);
                if (FALSE == fStatus) {
                    wprintf(L"Operation failed: %d.\n", GetLastError());
                }
                else {
                    wprintf(L"Operation successful.\n");    
                }
                
                CloseHandle(h);
                return 0;
            }
        }        
        else if (0 == wcscmp(argv[1], L"-inband")) {
            if (0 == wcscmp(argv[2], L"on")) {
                wprintf(L"Turning on inband ring tones...\n");

                DWORD dwData = 1;
                BOOL fStatus = DeviceIoControl(h,IOCTL_AG_SET_INBAND_RING,(LPVOID)&dwData,sizeof(dwData),NULL,0,NULL,NULL);
                if (FALSE == fStatus) {
                    wprintf(L"Operation failed: %d.\n", GetLastError());
                }
                else {
                    wprintf(L"Operation successful.\n");
                }
            
                CloseHandle(h);
                return 0;
            }
            else if (0 == wcscmp(argv[2], L"off")) {
                wprintf(L"Turning off inband ring tones...\n");

                DWORD dwData = 0;
                BOOL fStatus = DeviceIoControl(h,IOCTL_AG_SET_INBAND_RING,(LPVOID)&dwData,sizeof(dwData),NULL,0,NULL,NULL);
                if (FALSE == fStatus) {
                    wprintf(L"Operation failed: %d.\n", GetLastError());
                }
                else {
                    wprintf(L"Operation successful.\n");    
                }
                
                CloseHandle(h);
                return 0;
            }            
        }
        else if (0 == wcscmp(argv[1], L"-powersave")) {
            if (0 == wcscmp(argv[2], L"on")) {
                wprintf(L"Turning on Power-save mode...\n");

                DWORD dwData = 1;
                
                BOOL fStatus = DeviceIoControl(h,IOCTL_AG_SET_POWER_MODE,(LPVOID)&dwData,sizeof(dwData),NULL,0,NULL,NULL);
                if (FALSE == fStatus) {
                    wprintf(L"Operation failed: %d.\n", GetLastError());
                }
                else {
                    wprintf(L"Operation successful.\n");    
                }
            
                CloseHandle(h);
                return 0;
            }
            else if (0 == wcscmp(argv[2], L"off")) {
                wprintf(L"Turning off Power-save mode...\n");

                DWORD dwData = 0;
                
                BOOL fStatus = DeviceIoControl(h,IOCTL_AG_SET_POWER_MODE,(LPVOID)&dwData,sizeof(dwData),NULL,0,NULL,NULL);
                if (FALSE == fStatus) {
                    wprintf(L"Operation failed: %d.\n", GetLastError());
                }
                else {
                    wprintf(L"Operation successful.\n");    
                }
                
                CloseHandle(h);
                return 0;
            }
            else if (0 == wcscmp(argv[2], L"query")) {
                wprintf(L"Getting Power-save mode...\n");

                DWORD dwData = 0;
                
                BOOL fStatus = DeviceIoControl(h,IOCTL_AG_GET_POWER_MODE,NULL,0,(LPVOID)&dwData,sizeof(dwData),NULL,NULL);
                if (FALSE == fStatus) {
                    wprintf(L"Operation failed: %d.\n", GetLastError());
                }
                else {
                    wprintf(L"Operation successful.  Result=%d\n", dwData?1:0);    
                }
                
                CloseHandle(h);
                return 0;
            }
        }
        else if (0 == wcscmp(argv[1], L"-outvol")) {
            int vol = _wtoi(argv[2]);

            wprintf(L"Setting speaker volume to %d...\n", vol);
            
            BOOL fStatus = DeviceIoControl(h,IOCTL_AG_SET_SPEAKER_VOL,(LPVOID)&vol,sizeof(vol),NULL,0,NULL,NULL);
            if (FALSE == fStatus) {
                wprintf(L"Operation failed: %d.\n", GetLastError());
            }
            else {
                wprintf(L"Operation successful.\n");    
            }
            
            CloseHandle(h);
            return 0;
        }
        else if (0 == wcscmp(argv[1], L"-invol")) {
            int vol = _wtoi(argv[2]);

            wprintf(L"Setting microphone volume to %d...\r\n", vol);

            BOOL fStatus = DeviceIoControl(h,IOCTL_AG_SET_MIC_VOL,(LPVOID)&vol,sizeof(vol),NULL,0,NULL,NULL);
            if (FALSE == fStatus) {
                wprintf(L"Operation failed: %d.\n", GetLastError());
            }
            else {
                wprintf(L"Operation successful.\n");    
            }
            
            CloseHandle(h);
            return 0;
        }
    }
    
    CloseHandle(h);

    usage();
    return 0;
}

