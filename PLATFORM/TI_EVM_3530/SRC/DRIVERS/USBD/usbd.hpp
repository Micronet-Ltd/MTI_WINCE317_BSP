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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

usbd.hpp

Abstract:  

Notes: 

--*/
#ifndef __USBD_HPP__
#define __USBD_HPP__

#ifndef __USBDOBJ_HPP__
#include "usbdobj.hpp"
#endif

#define UnusedParameter(x) x = x

#ifdef DEBUG
// Debug zone defs
#define ZONE_INIT       DEBUGZONE(0)
#define ZONE_LOADER     DEBUGZONE(1)
#define ZONE_CONFIG     DEBUGZONE(2)
#define ZONE_PIPE       DEBUGZONE(3)
#define ZONE_VENDOR     DEBUGZONE(4)
#define ZONE_CONTROL    DEBUGZONE(5)
#define ZONE_BULK       DEBUGZONE(6)
#define ZONE_INTERRUPT  DEBUGZONE(7)
#define ZONE_ISOCH      DEBUGZONE(8)
#define ZONE_API        DEBUGZONE(9)
#define ZONE_WARNING    DEBUGZONE(14)
#define ZONE_ERROR      DEBUGZONE(15)
#endif

extern LPCWSTR gcszUsbRegKey;
extern LPCWSTR gcszDriverIDs;
extern LPCWSTR gcszLoadClients;
extern LPCWSTR gcszDllName;

BOOL AddTransfer(SPipe * pPipe, STransfer * pTransfer);
BOOL LoadUSBClient(SDevice * pDev, BOOL * pfLoaded,
        LPCUSB_INTERFACE lpInterface);
BOOL InstallClientDriver(LPCWSTR szDriverName);
BOOL LoadDeviceDrivers(SDevice * pDev, BOOL * pfLoaded);
BOOL CloseUSBDevice(SDevice * pDev,DWORD dwCode);
void GetSettingString(LPTSTR pszString, DWORD cchString, DWORD dw1, DWORD dw2, DWORD dw3);
LPCUSB_ENDPOINT FindEndpoint(LPCUSB_INTERFACE lpInterface,
        UCHAR bEndpointAddress);
BOOL ReferencePipeHandle(SPipe *pPipe);
void DereferencePipeHandle(SPipe *pPipe);
BOOL ValidateDeviceHandle(SDevice *pDev);
BOOL ReferenceTransferHandle(STransfer *pTransfer);
void DereferenceTransferHandle(STransfer *pTransfer);
SPipe *GetPipeObject(SDevice *pDev);
void FreePipeObject(SPipe *pPipe);
void FreePipeObjectMem(SPipe *pPipe);
STransfer * GetTransferObject(SPipe *pPipe, DWORD dwIsochFrames);
void FreeTransferObject(STransfer *pTransfer);
void FreeTransferObjectMem(STransfer *pTransfer);
SWait * GetWaitObject();
BOOL FreeWaitObject(SWait * pWait);
BOOL ConvertToClientRegistry(LPTSTR lpStr, DWORD dwUnitSize, LPCUSB_DEVICE pDeviceInfo,LPCUSB_INTERFACE pInterface,BOOL fGroup1, BOOL fGroup2, BOOL fGroup3,PUSB_DRIVER_SETTINGS pSetting);


typedef BOOL (WINAPI *LPCLIENT_ATTACH_PROC)(USB_HANDLE, LPCUSB_FUNCS,
                                            LPCUSB_INTERFACE, LPCWSTR, LPBOOL,
                                            LPCUSB_DRIVER_SETTINGS, DWORD);
typedef BOOL (WINAPI *LPCLIENT_INSTALL_PROC)(LPCWSTR);

extern "C"
{
BOOL RegisterNotificationRoutine(USB_HANDLE hDevice,
                                 LPDEVICE_NOTIFY_ROUTINE lpNotifyRoutine,
                                 LPVOID lpvNotifyParameter);
BOOL UnRegisterNotificationRoutine(USB_HANDLE hDevice,
                                   LPDEVICE_NOTIFY_ROUTINE lpNotifyRoutine,
                                   LPVOID lpvNotifyParameter);


BOOL LoadGenericInterfaceDriver(USB_HANDLE hDevice,
                                LPCUSB_INTERFACE lpInterface);


BOOL TranslateStringDescr(LPCUSB_STRING_DESCRIPTOR lpStringDescr,
                          LPWSTR szString, DWORD cchStringLength);
LPCUSB_INTERFACE FindInterface(LPCUSB_DEVICE lpDeviceInfo,
                               UCHAR bInterfaceNumber, UCHAR bAlternateSetting);


BOOL GetFrameNumber(USB_HANDLE hDevice, LPDWORD lpdwFrameNumber);
BOOL GetFrameLength(USB_HANDLE hDevice, LPUSHORT lpuFrameLength);

BOOL TakeFrameLengthControl(USB_HANDLE hDevice);
BOOL SetFrameLength(USB_HANDLE hDevice, HANDLE hEvent, USHORT uFrameLength);
BOOL ReleaseFrameLengthControl(USB_HANDLE hDevice);


LPCUSB_DEVICE GetDeviceInfo(USB_HANDLE hDevice);


USB_TRANSFER IssueVendorTransfer(USB_HANDLE hDevice,
                                 LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                                 LPVOID lpvNotifyParameter, DWORD dwFlags,
                                 LPCUSB_DEVICE_REQUEST lpControlHeader,
                                 LPVOID lpvBuffer,
                                 ULONG uBufferPhysicalAddress);
USB_TRANSFER GetInterface(USB_HANDLE hDevice,
                          LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                          LPVOID lpvNotifyParameter, DWORD dwFlags,
                          UCHAR bInterfaceNumber, PUCHAR lpbAlternateSetting);
USB_TRANSFER SetInterface(USB_HANDLE hDevice,
                          LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                          LPVOID lpvNotifyParameter, DWORD dwFlags,
                          UCHAR bInterfaceNumber, UCHAR bAlternateSetting);
USB_TRANSFER GetDescriptor(USB_HANDLE hDevice,
                           LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                           LPVOID lpvNotifyParameter, DWORD dwFlags,
                           UCHAR bType, UCHAR bIndex, WORD wLanguage,
                           WORD wLength, LPVOID lpvBuffer);
USB_TRANSFER SetDescriptor(USB_HANDLE hDevice,
                           LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                           LPVOID lpvNotifyParameter, DWORD dwFlags,
                           UCHAR bType, UCHAR bIndex, WORD wLanguage,
                           WORD wLength, LPVOID lpvBuffer);
USB_TRANSFER SetFeature(USB_HANDLE hDevice,
                        LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                        LPVOID lpvNotifyParameter, DWORD dwFlags,
                        WORD wFeature, UCHAR bIndex);
USB_TRANSFER ClearFeature(USB_HANDLE hDevice,
                          LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                          LPVOID lpvNotifyParameter, DWORD dwFlags,
                          WORD wFeature, UCHAR bIndex);
USB_TRANSFER GetStatus(USB_HANDLE hDevice,
                       LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                       LPVOID lpvNotifyParameter, DWORD dwFlags, UCHAR bIndex,
                       LPWORD lpwStatus);
USB_TRANSFER SyncFrame(USB_HANDLE hDevice,
                       LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                       LPVOID lpvNotifyParameter, DWORD dwFlags,
                       UCHAR bEndpoint, LPWORD lpwFrame);


USB_PIPE OpenPipe(USB_HANDLE hDevice,
                  LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor);
BOOL AbortPipeTransfers(USB_PIPE hPipe, DWORD dwFlags);
BOOL ResetPipe(USB_PIPE hPipe);
BOOL ResetDefaultPipe(USB_HANDLE hDevice);
BOOL ClosePipe(USB_PIPE hPipe);
BOOL IsPipeHalted(USB_PIPE hPipe, LPBOOL lpfHalted);
BOOL IsDefaultPipeHalted(USB_HANDLE hDevice, LPBOOL lpbHalted);

USB_TRANSFER IssueControlTransfer(USB_PIPE hPipe,
                                  LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                                  LPVOID lpvNotifyParameter, DWORD dwFlags,
                                  LPCVOID lpvControlHeader, DWORD dwBufferSize,
                                  LPVOID lpvBuffer,
                                  ULONG uBufferPhysicalAddress);
USB_TRANSFER IssueBulkTransfer(USB_PIPE hPipe,
                               LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                               LPVOID lpvNotifyParameter, DWORD dwFlags,
                               DWORD dwBufferSize, LPVOID lpvBuffer,
                               ULONG uBufferPhysicalAddress);
USB_TRANSFER IssueInterruptTransfer(USB_PIPE hPipe,
                                    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                                    LPVOID lpvNotifyParameter, DWORD dwFlags,
                                    DWORD dwBufferSize, LPVOID lpvBuffer,
                                    ULONG uBufferPhysicalAddress);
USB_TRANSFER IssueIsochTransfer(USB_PIPE hPipe,
                                LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                                LPVOID lpvNotifyParameter, DWORD dwFlags,
                                DWORD dwStartingFrame, DWORD dwFrames,
                                LPCDWORD lpdwLengths, LPVOID lpvBuffer,
                                ULONG uBufferPhysicalAddress);


BOOL IsTransferComplete(USB_TRANSFER hTransfer);
BOOL GetTransferStatus(USB_TRANSFER hTransfer, LPDWORD lpdwBytesTransfered,
                       LPDWORD lpdwError);
BOOL GetIsochResults(USB_TRANSFER hTransfer, DWORD dwFrames,
                     LPDWORD lpdwBytesTransfered, LPDWORD lpdwErrors);
BOOL AbortTransfer(USB_TRANSFER hTransfer, DWORD dwFlags);
BOOL CloseTransfer(USB_TRANSFER hTransfer);
BOOL DisableDevice(USB_HANDLE hDevice, BOOL    fResetDevice, BYTE    bInterfaceNumber );

BOOL SuspendDevice(    USB_HANDLE hDevice, BYTE    bInterfaceNumber  );
BOOL ResumeDevice( USB_HANDLE hDevice, BYTE    bInterfaceNumber );


}

#endif
