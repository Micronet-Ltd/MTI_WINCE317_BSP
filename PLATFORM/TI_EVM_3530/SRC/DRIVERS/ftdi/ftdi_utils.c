/*++

Copyright (c) 2001-2012  Future Technology Devices International Ltd.

Module Name:

    ftdi_ser.c

Abstract:

    Native USB device driver for FTDI FT8U2XX
    Serial port routines

Environment:

    kernel mode only

Revision History:

	24/05/04	st		Adapted from Microsoft sample
	13/08/04	st		Dual chip support.
	23/08/04	st		Added checks in the Set and Clear RTS/DTR.
	25/11/04	st		Alias baud rate code added. 2232C serial number code added.
	09/03/05	st		Added check in the SetBaudRate function if we have a vaid baud rate table.
	07/06/05	st		Added FT_PurgeInternalBuffer and FT_SetDeviceEvent functions.
	23/06/05	st		Changes to module to support modem emulation (mainly to the FT_SetDTR etc...)
	30/06/05	st		Configurable sync/async changes.
	19/09/05	st		Restore device data added (for unplug/replug). Removed all NT_STATUS macro calls.
	24/01/06	st		Altered the get modem status function.
	04/06/08	ma		Added hi-speed chip support. FT4232H and FT2232H.
	20/10/09	ma		Add ability to alias hi-speed baud rates. 
						Send set baud rate request 3 times if enumerated in hi-speed mode to allow time for settling before next write.
	03/03/11	ma		Added a surprise remove flag to handle surprise disconnects better (in particular to allow WM_DEVICECHANGE messages to propagate).
						Added support for the FT232H.
	14/12/11	ma		Use IS_HI_SPEED macro for baud rates.
	25/04/11	gdm		Fix FT_GetStringDescriptor handling of abstracted CLIIssueVendorTransfer return value.


--*/
#include "ftdi_ioctl.h"
#include "ftdi_debug.h"
#include "BUSBDBG.H"
#include <serdbg.h>
#include <celog.h>

#if defined(LOG_DEBUG) || defined(DEBUG)
TCHAR gtcBuf[1024];
TCHAR gtcBufTime[1024];

#ifndef DEBUG
	unsigned long ulDebugFlags = ZONE_ERROR;

/*	#define ZONE_INIT		DEBUGZONE(0)
#define ZONE_OPEN		DEBUGZONE(1)
#define ZONE_READ		DEBUGZONE(2)
#define ZONE_WRITE		DEBUGZONE(3)
#define ZONE_CLOSE		DEBUGZONE(4)
#define ZONE_IOCTL		DEBUGZONE(5)
#define ZONE_THREAD		DEBUGZONE(6)
#define ZONE_EVENTS		DEBUGZONE(7)
#define ZONE_CRITSEC	DEBUGZONE(8)
#define ZONE_FLOW		DEBUGZONE(9)
#define ZONE_IR			DEBUGZONE(10)
#define ZONE_USR_READ	DEBUGZONE(11)
#define ZONE_ALLOC		DEBUGZONE(12)
#define ZONE_FUNCTION	DEBUGZONE(13)
#define ZONE_WARN		DEBUGZONE(14)
#define ZONE_ERROR		DEBUGZONE(15)*/
#endif

#endif

#define OUT_USB_TRANSFER	(USB_OUT_TRANSFER | USB_SHORT_TRANSFER_OK)
//#define IN_USB_TRANSFER	(USB_IN_TRANSFER | USB_SHORT_TRANSFER_OK)
#define IN_USB_TRANSFER	(USB_IN_TRANSFER)

// STATUS_SUCCESS and ERROR_SUCCESS are equivalent (0L)
DWORD 
IoErrorHandler( 
   PFTDI_DEVICE pUsbFTDI,
   PUSBD_PIPE_INFORMATION hPipe, // if NULL then uses default endpoint,
   UCHAR EndpointAddress,
   USB_ERROR dwUsbErr
   );
/*++
Class specific command:
   SOFT_RESET

Return: TRUE if successful, else FALSE.

--*/
BOOL
SoftReset( 
   PFTDI_DEVICE pUsbFTDI
   )
{
	USB_DEVICE_REQUEST ControlHeader;
	DWORD  dwBytesTransferred = 0;
	DWORD  dwUsbErr = USB_NO_ERROR;
    DWORD  dwErr = ERROR_SUCCESS;
    USHORT usStatus = 0;
    BOOL bRc = TRUE;

    DEBUGMSG1( ZONE_FUNCTION, (TEXT(">SoftReset\n")));

    if ( !VALID_CONTEXT( pUsbFTDI) ) {
		DEBUGMSG1( ZONE_ERROR, (TEXT("Invalid parameter\n")));
		bRc = FALSE;
	} else {
		ControlHeader.bmRequestType = USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_CLASS;
        ControlHeader.bRequest = FTDI_RESET;
        ControlHeader.wValue   = 0;				// Reset SIO
        ControlHeader.wIndex   = pUsbFTDI->bInterfaceNumber;
        ControlHeader.wLength  = 0;

        dwErr = IssueVendorTransfer(	 
					pUsbFTDI->UsbFuncs,
					pUsbFTDI->hUsbDevice,
					DefaultTransferComplete,
					pUsbFTDI->hEP0Event,
					(USB_OUT_TRANSFER | USB_SHORT_TRANSFER_OK),
					&ControlHeader,
					NULL, 0,
					&dwBytesTransferred,
					pUsbFTDI->UsbTimeouts.SoftResetTimeout,
					&dwUsbErr 
					);

        if(ERROR_SUCCESS != dwErr || USB_NO_ERROR != dwUsbErr) {
            //RETAILMSG(1, (TEXT("Soft Reset IVT ERROR:%d 0x%x\n"), dwErr, dwUsbErr));
            DEBUGMSG3( ZONE_ERROR, TEXT("IssueVendorTransfer ERROR:%d 0x%x\n"), dwErr, dwUsbErr);
            IoErrorHandler(pUsbFTDI, NULL, 0, dwUsbErr);
        }
    }

    DEBUGMSG2(ZONE_FUNCTION, TEXT("<SoftReset:%d\n"), bRc);

    return bRc;
}

NTSTATUS
FT_ResetPipe(
    PFTDI_DEVICE       DeviceExtension,
    PUSBD_PIPE_INFORMATION  PipeInfo
    )
{
	NTSTATUS ntStatus = ~ERROR_SUCCESS;

	if (PipeInfo) {
		// reset the pipe in the USB stack
		if (ResetPipe(DeviceExtension->UsbFuncs, PipeInfo->hPipe, 0))
			// clear the stall on the device
			ntStatus = ClearOrSetFeature(	
							DeviceExtension->UsbFuncs,
							DeviceExtension->hUsbDevice,
							DefaultTransferComplete,
							DeviceExtension->hEP0Event,
							USB_SEND_TO_ENDPOINT,
							USB_FEATURE_ENDPOINT_STALL,
							PipeInfo->EndpointAddress,
							1000,
							FALSE 
							);
	}
	return ntStatus;
}

NTSTATUS
LResetPipe(
    PFTDI_DEVICE       DeviceExtension,
    PUSBD_PIPE_INFORMATION  PipeInfo
    )
{
    NTSTATUS ntStatus;
    ULONG i;

    //
    // Try to recover from USBD_STATUS_ENDPOINT_HALTED
    //
	DEBUGMSG1(ZONE_ERROR,(TEXT("ResetPipe RESETTING PIPE\n")));

	Sleep(100);			// to let the device notify get a chance to setup variables.

    for (i = 0; !DeviceExtension->Flags.UnloadPending && i < DeviceExtension->ResetPipeRetries; i++) {

        ntStatus = FT_ResetPipe(DeviceExtension, PipeInfo);

        if (ntStatus == ERROR_SUCCESS)
            break;

        DEBUGMSG3(ZONE_FUNCTION, TEXT("ResetPipe %s PIPE ERROR 0x%x\n"), USBD_PIPE_DIRECTION_IN(PipeInfo) ? "IN" : "OUT",ntStatus);

    }

    if (ntStatus != ERROR_SUCCESS) {

        //
        // We have failed to reset the pipe, so as a last resort
        // try to reset the device.
        //

#if 0
        //
        // No, don't reset device here. Instead, return the error and
        // let the application reset the port via FT_ResetPort.
        //
        ntStatus = FT_ResetDevice(deviceObject);
#endif
    }

    return ntStatus;
}


//
// returns Win32 error
//
DWORD 
IoErrorHandler( 
   PFTDI_DEVICE	pUsbFTDI,
   PUSBD_PIPE_INFORMATION hPipe, // if NULL then uses default endpoint,
   UCHAR EndpointAddress,
   USB_ERROR dwUsbErr
   )
{
	DWORD dwErr = ERROR_SUCCESS;

	DEBUGMSG3(ZONE_FUNCTION,TEXT(">IoErrorHandler(%p, 0x%x)\n"), hPipe, dwUsbErr);

	if ( VALID_CONTEXT( pUsbFTDI ) ) {
		switch (dwUsbErr) {

		case USB_STALL_ERROR:
			DEBUGMSG1(ZONE_ERROR,(TEXT("USB_STALL_ERROR\n")));
			if (hPipe) {
				// reset the pipe in the USB stack
				if (ResetPipe( pUsbFTDI->UsbFuncs, hPipe, 0))
					// clear the stall on the device
					dwErr = ClearOrSetFeature(	
								pUsbFTDI->UsbFuncs,
								pUsbFTDI->hUsbDevice,
								DefaultTransferComplete,
								pUsbFTDI->hEP0Event,
								USB_SEND_TO_ENDPOINT,
								USB_FEATURE_ENDPOINT_STALL,
								EndpointAddress,
								1000,
								FALSE 
								);
			} 
			else {
				// reset the default endpoint
				dwErr = ResetDefaultEndpoint(pUsbFTDI->UsbFuncs, pUsbFTDI->hUsbDevice);
			}
            break;

		case USB_DEVICE_NOT_RESPONDING_ERROR:
			DEBUGMSG1(ZONE_ERROR,(TEXT("USB_DEVICE_NOT_RESPONDING_ERROR\n")));
            // try a SoftReset
			if ( !SoftReset(pUsbFTDI) ) {
				// If the device is not responding and we can not soft reset then assume it's dead.
				// WCE can't reset the port yet.
				DEBUGMSG1( ZONE_ERROR, (TEXT("Dead device\n")));
				dwErr = ERROR_INVALID_HANDLE;
			}
			break;

		default:
			DEBUGMSG2(ZONE_ERROR,TEXT("IoErrorHandler: Unhandled error: 0x%x\n"), dwUsbErr);
			break;
		}

	} else {
		dwErr = ERROR_INVALID_PARAMETER;
		DEBUGMSG1(ZONE_ERROR,(TEXT("Invalid Parameter\n")));
	}

	DEBUGMSG2(ZONE_FUNCTION,TEXT("<IoErrorHandler:%d\n"), dwErr);

	return dwErr;
}

void StringToWchar(PWCHAR pDest, char * pSrc, ULONG Len)
{
    WCHAR *d = pDest;
    WCHAR w;
    char c;

    while (Len--) {
        c = *pSrc++;
        w = (WCHAR)c;
        *d++ = w;
        if (w == 0)
            break;
    }
}

DWORD
FT_VendorRequest(
	UCHAR RequestCode,
	USHORT wValue,
	USHORT wIndex,
	PVOID Buffer,
	USHORT Length,
	ULONG TransferFlags,
	PFTDI_DEVICE pUsbFTDI
	)
{
	USB_DEVICE_REQUEST ControlHeader;
	DWORD  dwBytesTransferred = 0;
	DWORD  dwUsbErr = USB_NO_ERROR;
    DWORD  dwErr = ERROR_SUCCESS;
    USHORT usStatus = 0;
	DWORD status = ERROR_SUCCESS;
	INT tempLength;
//	USB_TRANSFER val;
//	TCHAR ErrorText[128];
//	char cErrorText[128];
//	LPIsSUE_VENDOR_TRANSFER pVendorTransfer;

	/* This needs to change for vendor DEVICE to HOST for all the Gets */

	if(TransferFlags == IN_USB_TRANSFER) {
		ControlHeader.bmRequestType = USB_REQUEST_DEVICE_TO_HOST | USB_REQUEST_VENDOR;
//		ControlHeader.bmRequestType = 0;
		tempLength = 0;
	}
	else if(TransferFlags == OUT_USB_TRANSFER) {
		ControlHeader.bmRequestType = USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_VENDOR;
//		ControlHeader.bmRequestType = 0;
		tempLength = Length;
	}
	else {		/* Unknown Request type */
		return ~ERROR_SUCCESS; // ????
	}
	ControlHeader.bRequest		= RequestCode;
	ControlHeader.wValue		= wValue;
	ControlHeader.wIndex		= wIndex;
	ControlHeader.wLength		= Length;

	dwErr = IssueVendorTransfer(	
				pUsbFTDI->UsbFuncs,
				pUsbFTDI->hUsbDevice,
				DefaultTransferComplete,
				pUsbFTDI->hEP0Event,
				TransferFlags,
				&ControlHeader,
				Buffer, tempLength,
				&dwBytesTransferred,
				pUsbFTDI->UsbTimeouts.SoftResetTimeout,
				&dwUsbErr 
				);

	if ( ERROR_SUCCESS != dwErr || USB_NO_ERROR != dwUsbErr) {
        //RETAILMSG(1, (TEXT("Vendor Request IVT ERROR:%d 0x%x\n"), dwErr, dwUsbErr));
		DEBUGMSG3( ZONE_ERROR, TEXT("IssueVendorTransfer ERROR:%d 0x%x\n"), dwErr, dwUsbErr);

		if ((dwUsbErr == USB_DEVICE_NOT_RESPONDING_ERROR) || (dwErr == ERROR_INVALID_HANDLE)) {
			// possibly been surprise removed!
			// return our invalid handle error code
            //RETAILMSG(1, (TEXT("Vendor Request device gone\n")));
			DEBUGMSG1( ZONE_ERROR, TEXT("IssueVendorTransfer device gone\n"));
			pUsbFTDI->Flags.SurpriseRemoved = TRUE;
			if (dwErr == ERROR_SUCCESS)
				dwErr = ERROR_DEVICE_NOT_CONNECTED;
			return dwErr;
		}

//		sprintf(cErrorText, "dwErr(%d),dwUsbErr(%d)", dwErr, dwUsbErr);
//		StringToWchar(ErrorText, cErrorText, strlen(cErrorText));
//		MessageBox( NULL, ErrorText, TEXT("Error"), MB_OK | MB_ICONINFORMATION );

		dwErr = IoErrorHandler(pUsbFTDI, NULL, 0, dwUsbErr);
		if(dwErr != ERROR_SUCCESS)
			status = GetLastError();
	}
	return status;			/* Really not sure what to do here */
}


DWORD
FT_SetBaudRate(
    PFTDI_DEVICE pUsbFTDI,
    ULONG BaudRate
    )
{
	DWORD ntStatus = ERROR_SUCCESS;
	INT Index;
    SHORT rateOk = 1;
	ULONG bm;
	USHORT wValue;
    USHORT wIndex = 0;
	ULONG ulTemp;
	UCHAR count = 1;
	UCHAR i;

	bm = IS_BM_DEVICE(pUsbFTDI);

	/* set baud rate into wValue */

    switch (BaudRate) {
    case 300 :
        Index = FTDI_CBR_300;
        break;
    case 600 :
        Index = FTDI_CBR_600;
        break;
    case 1200 :
        Index = FTDI_CBR_1200;
        break;
    case 2400 :
        Index = FTDI_CBR_2400;
        break;
    case 4800 :
        Index = FTDI_CBR_4800;
        break;
    case 9600 :
        Index = FTDI_CBR_9600;
        break;
    case 14400 :
        Index = (IS_FT8U232(pUsbFTDI)) ? FTDI_CBR_14400 : FTDI_CBR_115200;
        break;
    case 19200 :
        Index = FTDI_CBR_19200;
        break;
    case 38400 :
        Index = FTDI_CBR_38400;
        break;
    case 57600 :
        Index = FTDI_CBR_57600;
        break;
    case 115200 :
        Index = FTDI_CBR_115200;
        break;
    case 230400 :
        Index = (IS_FT8U232(pUsbFTDI)) ? FTDI_CBR_230400 : FTDI_CBR_115200;
        break;
    case 460800 :
        Index = (IS_FT8U232(pUsbFTDI)) ? FTDI_CBR_460800 : FTDI_CBR_115200;
        break;
    case 921600 :
        Index = (IS_FT8U232(pUsbFTDI)) ? FTDI_CBR_921600 : FTDI_CBR_115200;
        break;
    default :
        Index = 0xffffffff;
        break;
    }
    
    if ((pUsbFTDI->bIsValidBaudRateTable == TRUE) && (Index != 0xffffffff)) {
		wValue = (USHORT)(pUsbFTDI->BaudRates[Index] & 0xFFFF);
		wIndex = (USHORT)((pUsbFTDI->BaudRates[Index] & 0xFFFF0000) >> 16);
    }
    else {
		// Try to set the baud rate
		if (IS_HI_SPEED_DEVICE(pUsbFTDI) && (BaudRate >= 1200))
			rateOk = FT_GetDivisorHi(BaudRate,&wValue,&wIndex,NULL,NULL,NULL);
		else
			rateOk = FT_GetDivisor(BaudRate,&wValue,&wIndex,NULL,NULL,NULL,bm);

		if (rateOk != 1)
			ntStatus = STATUS_UNSUCCESSFUL;
    }

	if(ntStatus == STATUS_UNSUCCESSFUL) {
        //
        // default to 115200 baud
        //
		wValue = (USHORT)(pUsbFTDI->BaudRates[FTDI_CBR_115200] & 0xFFFF);
		wIndex = (USHORT)((pUsbFTDI->BaudRates[FTDI_CBR_115200] & 0xFFFF0000) >> 16);
	}

    //
    // For FT2232, ls byte of wIndex has to hold interface.
    // So shift into ms byte of wIndex.
    //
    if (IS_MULTI_IF_DEVICE(pUsbFTDI) || IS_FT232H(pUsbFTDI) || IS_FT232EX(pUsbFTDI)) {
		// Our ext_div value from FT_GetDivisorHi/FT_GetDivisor should be in the upper byte
		// for FT2232D, FT2232H, FT4232H and FT232H devices
		// NOT FT232R or FT232B devices which expect it in the lower byte!
		wIndex <<= 8;
        wIndex &= 0xff00;
        wIndex |= pUsbFTDI->bInterfaceNumber;
    }

	// If we are a hi-speed device, we need to be able to alias up to 12Mbaud so check for 
	// the highest bit in the divisor value being set.
	if (IS_HI_SPEED_DEVICE(pUsbFTDI)) {
		// if we have a valid baud rate table entry...
		if (Index != 0xFFFFFFFF) {
			ulTemp = pUsbFTDI->BaudRates[Index];
			if (ulTemp & 0x80000000) {
				// bit is set to alias hi-speed baud rates 
				// turn off baud rate divider to use 12Mbaud as base
				wIndex |= 0x0200;
//				FT_KdPrint( DBGLVL_DEFAULT,("SetBaudRate - aliasing hi-speed baud rate, turning off divid by 2.5\n"));
			}
		}
	}

    if (rateOk == 1) {
		// Check for needing to delay hi-speed chips to allow baud rate to settle
		// Note: only need this if enumerated by a hi-speed host!
		// Otherwise we run on 1ms frames and this allows sufficient time for baud rate to settle
		if (pUsbFTDI->BulkIn.wMaxPacketSize == FT_USBD_PACKET_SIZE_HI) 
			count = 3;	// issue request 3 times to cause a delay of 375us - this allows 
						// enough time for baud rate to stabilise before accepting any writes
		else
			count = 1;


		for (i = 0; i < count; i++) {

			ntStatus = FT_VendorRequest(
							FTDI_SET_BAUD_RATE,
							wValue,
							(USHORT)(wIndex),
							NULL,
							0,
							OUT_USB_TRANSFER,
							pUsbFTDI
							);
		}
        pUsbFTDI->DevContext.BaudRate = BaudRate;
    }

//    FT_KdPrint(DBGLVL_DEFAULT,("FT_SetBaudRate Baud=%ld wValue=%04x ntStatus=0x%x\n",
//                                BaudRate,wValue,ntStatus));

    return ntStatus;
}


DWORD
FT_SetDivisor(
    PFTDI_DEVICE pUsbFTDI,
    USHORT Divisor
    )
{
    USHORT wValue = Divisor;
    DWORD ntStatus;
	
    ntStatus = FT_VendorRequest(
                    FTDI_SET_BAUD_RATE,
                    wValue,
                    pUsbFTDI->bInterfaceNumber,
                    NULL,
                    0,
                    OUT_USB_TRANSFER,
                    pUsbFTDI
                    );

    return ntStatus;
}


DWORD
FT_SetLineControl(
    PFTDI_DEVICE pUsbFTDI,
    TFtLineControl *LineControl
    )
{
	DWORD ntStatus = STATUS_SUCCESS;
	USHORT wValue;
	
	/* set number of data bits into wValue */

    wValue = LineControl->WordLength;

	/* set parity into wValue */

    wValue |= (LineControl->Parity)<<8;

	/* set number of stop bits into wValue */

    wValue |= (LineControl->StopBits)<<11;

    /* save wValue in case we get a BREAK on */

    pUsbFTDI->BreakOnParam = wValue;

    ntStatus = FT_VendorRequest(
                    FTDI_SET_DATA,
                    wValue,
                    pUsbFTDI->bInterfaceNumber,
                    NULL,
                    0,
                    OUT_USB_TRANSFER,
                    pUsbFTDI
                    );

    if (ntStatus == ERROR_SUCCESS) {
        pUsbFTDI->DevContext.LineControl = *LineControl;
    }

//    FT_KdPrint(DBGLVL_DEFAULT,
//               ("FT_SetLineControl (%d,%d,%d) wValue=%04x ntStatus=0x%x\n",
//               LineControl->WordLength,
//               LineControl->StopBits,
//               LineControl->Parity,
//               wValue,
//               ntStatus));

    return ntStatus;
}


DWORD
FT_SetFlowControl(
    PFTDI_DEVICE pUsbFTDI,
    TFtFlowControl *FlowControl

    )
{
	DWORD ntStatus = STATUS_SUCCESS;
    USHORT wValue = 0;
    USHORT wIndex = FlowControl->Control;

    if (wIndex == FT_FLOW_XON_XOFF) {
        wValue = ((USHORT) FlowControl->XoffChar) << 8;
        wValue |= ((USHORT) FlowControl->XonChar) & 0x00ff;
		pUsbFTDI->SpecialChars.XonChar = FlowControl->XonChar;
 		pUsbFTDI->SpecialChars.XoffChar = FlowControl->XoffChar;
	}

	if (FT_EMUL_SAVE_FLOW_CONTROL(&pUsbFTDI->EmulVars)) {
        if (wIndex == FT_FLOW_NONE) {
            FT_EMUL_SET_FLOW_DISABLED(&pUsbFTDI->EmulVars);
        }
        else {
            FT_EMUL_CLEAR_FLOW_DISABLED(&pUsbFTDI->EmulVars);
        }
    }

    ntStatus = FT_VendorRequest(
                    FTDI_SET_FLOW_CONTROL,
                    wValue,
                    (USHORT)(pUsbFTDI->bInterfaceNumber | wIndex),
                    NULL,
                    0,
                    OUT_USB_TRANSFER,
                    pUsbFTDI
                    );

    if (ntStatus == ERROR_SUCCESS) {
		PFT_DEV_CONTEXT pDevCtxt = &pUsbFTDI->DevContext;
        pUsbFTDI->FlowControl = wIndex;
        pDevCtxt->FlowControl = *FlowControl;
    }

    return ntStatus;
}


DWORD
FT_SetDtr(
	PSER_INFO pHWHead
    )
{
	DWORD ntStatus = STATUS_SUCCESS;
    USHORT wValue = 0x0101;
    USHORT wIndex = 0;
	PFTDI_DEVICE pUsbFTDI;

	pUsbFTDI = pHWHead->pFTDICtxt;

	if(pUsbFTDI->dwConfigDataFlags & DTR_IGNORE_ALL_SET) {
		return ntStatus;
	}

	if (!FT_EMUL_SUB_DTR_ENABLED(&pUsbFTDI->EmulVars)) {
		ntStatus = FT_VendorRequest(
						FTDI_MODEM_CTRL,
						wValue,
						pUsbFTDI->bInterfaceNumber,
						NULL,
						0,
						OUT_USB_TRANSFER,
						pUsbFTDI
						);
	}
	else {
	    PUCHAR Buffer;
		DWORD dwBytesTransferred;
		DWORD dwUsbErr;

		EnterCriticalSection(&(pHWHead->TransmitCritSec));

		//
		// Build a modem emulation packet and send
		//
		Buffer = pUsbFTDI->EmulVars.pWriteBuffer;
		*Buffer++ = FT_EMUL_DEF_ESC;
        *Buffer = FT_EMUL_ESC_SETDTR;

		//
		// Transfer it
		//
		dwBytesTransferred = 0;
		ntStatus = IssueBulkTransfer(
					pUsbFTDI->UsbFuncs,
					pUsbFTDI->BulkOut.hPipe,
					pUsbFTDI->BulkOut.pCompletionRoutine,
					pUsbFTDI->BulkOut.hEvent,
					pUsbFTDI->BulkOut.dwBulkFlags,//(USB_OUT_TRANSFER),				// Flags | USB_SHORT_TRANSFER_OK failed previously
					pUsbFTDI->EmulVars.pWriteBuffer,	// transfer buffer
					0,
					2,
					&dwBytesTransferred,
					5000,
					&dwUsbErr 
					);

		LeaveCriticalSection(&(pHWHead->TransmitCritSec));
	}

    if (ntStatus == ERROR_SUCCESS) {
        PFT_DEV_CONTEXT pDevCtxt = &pUsbFTDI->DevContext;
        pDevCtxt->MCR |= FT_MCR_DTR;
    }

//    FT_KdPrint(DBGLVL_DEFAULT,("FT_SetDtr\n"));

    return ntStatus;
}


DWORD
FT_ClrDtr(
	PSER_INFO pHWHead
    )
{
	DWORD ntStatus = STATUS_SUCCESS;
    USHORT wValue = 0x0100;
    USHORT wIndex = 0;
	PFTDI_DEVICE pUsbFTDI;

	pUsbFTDI = pHWHead->pFTDICtxt;
	
	if(pUsbFTDI->dwConfigDataFlags & DTR_IGNORE_ALL_SET) {
		return ntStatus;
	}

	if (!FT_EMUL_SUB_DTR_ENABLED(&pUsbFTDI->EmulVars)) {
		ntStatus = FT_VendorRequest(
						FTDI_MODEM_CTRL,
						wValue,
						pUsbFTDI->bInterfaceNumber,
						NULL,
						0,
						OUT_USB_TRANSFER,
						pUsbFTDI
						);
	}
	else {
	    PUCHAR Buffer;
		DWORD dwBytesTransferred;
		DWORD dwUsbErr;

		EnterCriticalSection(&(pHWHead->TransmitCritSec));

		//
		// Build a modem emulation packet and send
		//
		Buffer = pUsbFTDI->EmulVars.pWriteBuffer;
		*Buffer++ = FT_EMUL_DEF_ESC;
        *Buffer = FT_EMUL_ESC_CLRDTR;

		//
		// Transfer it
		//
		dwBytesTransferred = 0;
		ntStatus = IssueBulkTransfer(
					pUsbFTDI->UsbFuncs,
					pUsbFTDI->BulkOut.hPipe,
					pUsbFTDI->BulkOut.pCompletionRoutine,
					pUsbFTDI->BulkOut.hEvent,
					pUsbFTDI->BulkOut.dwBulkFlags,//(USB_OUT_TRANSFER),				// Flags | USB_SHORT_TRANSFER_OK failed previously
					pUsbFTDI->EmulVars.pWriteBuffer,	// transfer buffer
					0,
					2,
					&dwBytesTransferred,
					5000,
					&dwUsbErr 
					);

		LeaveCriticalSection(&(pHWHead->TransmitCritSec));
	}

    if (ntStatus == ERROR_SUCCESS) {
        PFT_DEV_CONTEXT pDevCtxt = &pUsbFTDI->DevContext;
        pDevCtxt->MCR |= ~FT_MCR_DTR;
    }

//    FT_KdPrint(DBGLVL_DEFAULT,("FT_ClrDtr\n"));

    return ntStatus;
}


DWORD
FT_SetRts(
	PSER_INFO pHWHead
    )
{
	DWORD ntStatus = STATUS_SUCCESS;
    USHORT wValue = 0x0202;
    USHORT wIndex = 0;
	PFTDI_DEVICE pUsbFTDI;

	pUsbFTDI = pHWHead->pFTDICtxt;

	if(pUsbFTDI->dwConfigDataFlags & RTS_IGNORE_ALL_SET) {
		return ntStatus;
	}

	if (!FT_EMUL_SUB_RTS_ENABLED(&pUsbFTDI->EmulVars)) {
		ntStatus = FT_VendorRequest(
						FTDI_MODEM_CTRL,
						wValue,
						pUsbFTDI->bInterfaceNumber,
						NULL,
						0,
						OUT_USB_TRANSFER,
						pUsbFTDI
						);
	}
	else {
	    PUCHAR Buffer;
		DWORD dwBytesTransferred;
		DWORD dwUsbErr;

		EnterCriticalSection(&(pHWHead->TransmitCritSec));

		//
		// Build a modem emulation packet and send
		//
		Buffer = pUsbFTDI->EmulVars.pWriteBuffer;
		*Buffer++ = FT_EMUL_DEF_ESC;
        *Buffer = FT_EMUL_ESC_SETRTS;

		//
		// Transfer it
		//
		dwBytesTransferred = 0;
		ntStatus = IssueBulkTransfer(
					pUsbFTDI->UsbFuncs,
					pUsbFTDI->BulkOut.hPipe,
					pUsbFTDI->BulkOut.pCompletionRoutine,
					pUsbFTDI->BulkOut.hEvent,
					pUsbFTDI->BulkOut.dwBulkFlags,//(USB_OUT_TRANSFER),				// Flags | USB_SHORT_TRANSFER_OK failed previously
					pUsbFTDI->EmulVars.pWriteBuffer,	// transfer buffer
					0,
					2,
					&dwBytesTransferred,
					5000,
					&dwUsbErr 
					);

		LeaveCriticalSection(&(pHWHead->TransmitCritSec));
	}

    if (ntStatus == ERROR_SUCCESS) {
		PFT_DEV_CONTEXT pDevCtxt = &pUsbFTDI->DevContext;
        pDevCtxt->MCR |= FT_MCR_RTS;
    }

//    FT_KdPrint(DBGLVL_DEFAULT,("FT_SetRts\n"));

    return ntStatus;
}

DWORD
FT_ClrRts(
	PSER_INFO pHWHead
    )
{
	DWORD ntStatus = STATUS_SUCCESS;
    USHORT wValue = 0x0200;
    USHORT wIndex = 0;
	PFTDI_DEVICE pUsbFTDI;

	pUsbFTDI = pHWHead->pFTDICtxt;

	if(pUsbFTDI->dwConfigDataFlags & RTS_IGNORE_ALL_SET) {
		return ntStatus;
	}
	
	if (!FT_EMUL_SUB_RTS_ENABLED(&pUsbFTDI->EmulVars)) {
		ntStatus = FT_VendorRequest(
						FTDI_MODEM_CTRL,
						wValue,
						pUsbFTDI->bInterfaceNumber,
						NULL,
						0,
						OUT_USB_TRANSFER,
						pUsbFTDI
						);
	}
	else {
	    PUCHAR Buffer;
		DWORD dwBytesTransferred;
		DWORD dwUsbErr;

		EnterCriticalSection(&(pHWHead->TransmitCritSec));

		//
		// Build a modem emulation packet and send
		//
		Buffer = pUsbFTDI->EmulVars.pWriteBuffer;
		*Buffer++ = FT_EMUL_DEF_ESC;
        *Buffer = FT_EMUL_ESC_CLRRTS;

		//
		// Transfer it
		//
		dwBytesTransferred = 0;
		ntStatus = IssueBulkTransfer(
					pUsbFTDI->UsbFuncs,
					pUsbFTDI->BulkOut.hPipe,
					pUsbFTDI->BulkOut.pCompletionRoutine,
					pUsbFTDI->BulkOut.hEvent,
					pUsbFTDI->BulkOut.dwBulkFlags,//(USB_OUT_TRANSFER),				// Flags | USB_SHORT_TRANSFER_OK failed previously
					pUsbFTDI->EmulVars.pWriteBuffer,	// transfer buffer
					0,
					2,
					&dwBytesTransferred,
					5000,
					&dwUsbErr 
					);

		LeaveCriticalSection(&(pHWHead->TransmitCritSec));
	}

    if (ntStatus == ERROR_SUCCESS) {
		PFT_DEV_CONTEXT pDevCtxt = &pUsbFTDI->DevContext;
        pDevCtxt->MCR |= ~FT_MCR_RTS;
    }

//    FT_KdPrint(DBGLVL_DEFAULT,("FT_ClrRts\n"));

    return ntStatus;
}


DWORD
FT_SetChars(
	PFTDI_DEVICE pUsbFTDI,
    TFtSpecialChars *SpecialChars
    )
{
	DWORD ntStatus = STATUS_SUCCESS;
    USHORT wIndex = 0;
    USHORT wValue;

    //
    // do event char first
    //

    wValue = ((USHORT) SpecialChars->EventChar) & 0x00ff;
    if (SpecialChars->EventCharEnabled)
        wValue |= 0x0100;


//    FT_KdPrint(
//        DBGLVL_DEFAULT,
//        ("FT_SetEventChar wValue=0x%x wIndex=0x%x\n",wValue,wIndex)
//        );

    ntStatus = FT_VendorRequest(
                    FTDI_SET_EVENT_CHAR,
                    wValue,
                    pUsbFTDI->bInterfaceNumber,
                    NULL,
                    0,
                    OUT_USB_TRANSFER,
                    pUsbFTDI
                    );

    if (ntStatus != ERROR_SUCCESS)
        return ntStatus;

    //
    // now do the error char
    //

    wValue = ((USHORT) SpecialChars->ErrorChar) & 0x00ff;
    if (SpecialChars->ErrorCharEnabled)
        wValue |= 0x0100;


//    FT_KdPrint(
//        DBGLVL_DEFAULT,
//        ("FT_SetErrorChar wValue=0x%x wIndex=0x%x\n",wValue,wIndex)
//        );

    ntStatus = FT_VendorRequest(
                    FTDI_SET_ERROR_CHAR,
                    wValue,
                    pUsbFTDI->bInterfaceNumber,
                    NULL,
                    0,
                    OUT_USB_TRANSFER,
                    pUsbFTDI
                    );

	if (ntStatus == ERROR_SUCCESS) {
		PFT_DEV_CONTEXT pDevCtxt = &pUsbFTDI->DevContext;
		pDevCtxt->SpecialChars = *SpecialChars;
	}

    return ntStatus;
}


DWORD
FT_GetModemStatus(
	PFTDI_DEVICE pUsbFTDI
    )
{
    DWORD ntStatus;
    USHORT buf;
    USHORT c;

    ntStatus = FT_VendorRequest(
                   FTDI_GET_MODEM_STATUS,
                   0,
                   pUsbFTDI->bInterfaceNumber,
                   (void **) &buf,
                   sizeof(USHORT),
                   IN_USB_TRANSFER,
                   pUsbFTDI
                   );

//    FT_KdPrint(
//        DBGLVL_DEFAULT,
//        ("FT_GetModemStatus buf=0x%x ntStatus=%x\n",buf,ntStatus)
//        );

    if (ntStatus == ERROR_SUCCESS) {
		DEBUGMSG2( ZONE_FUNCTION, TEXT("FT_GetModemStatus buf =%x\n"), buf );
        c = buf & 0x00f0;
        pUsbFTDI->SerialRegs.MSR = (UCHAR) c;
        c = buf >> 8;
        c &= 0x00ff;
        pUsbFTDI->SerialRegs.LSR = (UCHAR) c;
    }
    else {
		DEBUGMSG2( ZONE_ERROR, TEXT("FT_GetModemStatus Error ntStatus=%x\n"), ntStatus );
//        FT_KdPrint(
//            DBGLVL_DEFAULT,
//            ("FT_GetModemStatus Error ntStatus=%x\n",ntStatus)
//            );
    }

    return ntStatus;
}

VOID
FT_PurgeRead(
	PFTDI_DEVICE pUsbFTDI
    )
{
 /*   PIRP    readIrp;

    while (readIrp = FT_DequeueReadIrp(pDevExt)) {

        FT_KdPrint(
            DBGLVL_DEFAULT,
            ("FT_PurgeRead cancelling read readIrp=%x\n",readIrp)
            );

        KeCancelTimer(&pDevExt->ReadRequestTimer);

        readIrp->IoStatus.Status = STATUS_CANCELLED;

        FT_CancelReadIrp(readIrp);

    }
*/
//	pUsbFTDI->ResetBuffer = TRUE;

}

VOID
FT_PurgeInternalBuffer(
	PFTDI_DEVICE pUsbFTDI
    )
{
	DWORD dwWaitReturn;

	dwWaitReturn = WaitForSingleObject(pUsbFTDI->ReadBuffer.hBufferMutex, INFINITE);
	if(WAIT_OBJECT_0 == dwWaitReturn) {
		pUsbFTDI->ReadBuffer.rbGet = 0;
		pUsbFTDI->ReadBuffer.rbPut = 0;
		pUsbFTDI->ReadBuffer.rbBytesFree = pUsbFTDI->ReadBuffer.rbSize;
	}
	ReleaseMutex(pUsbFTDI->ReadBuffer.hBufferMutex);
}

DWORD
FT_Purge(
	PFTDI_DEVICE pUsbFTDI,
    ULONG Mask
    )
{
    DWORD ntStatus;
//    KIRQL oldIrql;
    USHORT wValue;
    USHORT wIndex = 0;
    ULONG i;

    //
    // purge rx and tx separately for safety!
    //

    if (Mask & FT_PURGE_RX) {

        wValue = FT_PURGE_RX;

        for (i = 0; i < 6; i++) {

            //
            // 6 resets required to ensure receive fifo is purged
            //

            ntStatus = FT_VendorRequest(
                            FTDI_RESET,
                            wValue,
                            pUsbFTDI->bInterfaceNumber,
                            NULL,
                            0,
                            OUT_USB_TRANSFER,
                            pUsbFTDI
                            );

        }

//        FT_KdPrint(DBGLVL_DEFAULT,("FT_PurgeRx Mask=0x%x\n",Mask));

        if (ntStatus != ERROR_SUCCESS)
            return ntStatus;

//        pDevExt = DeviceObject->DeviceExtension;

//        KeAcquireSpinLock(
//            &pDevExt->DpcLock,
//            &oldIrql
//            );
// +++ Some sort of spinlock required ???
        pUsbFTDI->ReadBuffer.rbGet = 0;
        pUsbFTDI->ReadBuffer.rbPut = 0;
        pUsbFTDI->ReadBuffer.rbBytesFree = pUsbFTDI->ReadBuffer.rbSize;		// +++ unsure about this

//        KeReleaseSpinLock(
//            &pDevExt->DpcLock,
//            oldIrql
//           );

        //
        // Purge read queue for good measure
        //

        FT_PurgeRead(pUsbFTDI);	// +++ this is canceling our read queue (IN Task) as well - need to work out how to do - should be easy

    }

    if (Mask & FT_PURGE_TX) {

        wValue = FT_PURGE_TX;

        ntStatus = FT_VendorRequest(
                        FTDI_RESET,
                        wValue,
                        pUsbFTDI->bInterfaceNumber,
                        NULL,
                        0,
                        OUT_USB_TRANSFER,
                        pUsbFTDI
                        );

//        FT_KdPrint(DBGLVL_DEFAULT,("FT_PurgeTx Mask=0x%x\n",Mask));

        if (ntStatus != ERROR_SUCCESS)
            return ntStatus;

    }

    return ntStatus;
}


DWORD
FT_SetBreak(
	PFTDI_DEVICE pUsbFTDI,
    USHORT OnOrOff
    )
{
	DWORD ntStatus = STATUS_SUCCESS;
	USHORT wValue;
	
    /* restore saved wValue */

	wValue = pUsbFTDI->BreakOnParam;

    wValue |= OnOrOff;

    ntStatus = FT_VendorRequest(
                    FTDI_SET_DATA,
                    wValue,
                    pUsbFTDI->bInterfaceNumber,
                    NULL,
                    0,
                    OUT_USB_TRANSFER,
                    pUsbFTDI
                    );

//    FT_KdPrint(
//        DBGLVL_DEFAULT,
//        ("FT_SetBreak wValue=%04x ntStatus=0x%x\n",
//        wValue,
//        ntStatus)
//        );

    return ntStatus;
}


DWORD
FT_SetLatencyTimer(
	PFTDI_DEVICE pUsbFTDI,
    UCHAR LatencyTime
    )
{
	DWORD ntStatus = STATUS_SUCCESS;
	USHORT wValue;

	if (!IS_BM_DEVICE(pUsbFTDI)) {
		return STATUS_UNSUCCESSFUL;
	}

	
    /* restore saved wValue */

    wValue = (USHORT) LatencyTime;
    wValue &= 0x00ff;

    ntStatus = FT_VendorRequest(
                    FTDI_SET_LATENCY_TIMER,
                    wValue,
                    pUsbFTDI->bInterfaceNumber,
                    NULL,
                    0,
                    OUT_USB_TRANSFER,
                    pUsbFTDI
                    );

    if (ntStatus == ERROR_SUCCESS) {
        pUsbFTDI->DevContext.LatencyTime = LatencyTime;

        //
        // latency timer changed, so setup deadman timeout.
        //
#if 0 // deprecated
        FT_SetDeadmanTimeout(pDevExt);
#endif // 0
    }

//    FT_KdPrint(
//        DBGLVL_DEFAULT,
//        ("FT_SetLatencyTimer wValue=%04x ntStatus=0x%x\n",
//        wValue,
//        ntStatus)
//        );

    return ntStatus;
}


DWORD
FT_GetLatencyTimer(
	PFTDI_DEVICE pUsbFTDI,
    PUCHAR pTimer 
    )
{
    DWORD ntStatus;
    UCHAR buf;

	if (!IS_BM_DEVICE(pUsbFTDI)) {
		return STATUS_UNSUCCESSFUL;
	}

    ntStatus = FT_VendorRequest(
                   FTDI_GET_LATENCY_TIMER,
                   0,
                   pUsbFTDI->bInterfaceNumber,
                   (void **) &buf,
                   sizeof(UCHAR),
                   IN_USB_TRANSFER,
                   pUsbFTDI
                   );

    if (ntStatus == ERROR_SUCCESS) {
        *pTimer = buf;
    }

//    FT_KdPrint(
//        DBGLVL_DEFAULT,
//        ("FT_GetLatencyTimer ntStatus=%x *pTimer=%d\n",ntStatus,*pTimer)
//        );

    return ntStatus;
}


DWORD
FT_SetBitMode(
	PFTDI_DEVICE pUsbFTDI,
    TFtBitMode *BitMode
    )
{
	DWORD ntStatus = STATUS_SUCCESS;
	USHORT wValue;
    USHORT Temp;

	if (!IS_BM_DEVICE(pUsbFTDI)) {
		return STATUS_UNSUCCESSFUL;
	}

    /* restore saved wValue */

    wValue = (USHORT) BitMode->Enable;
    wValue <<= 8;

    Temp = (USHORT) BitMode->Mask;
    Temp &= 0x00ff;

    wValue |= Temp;

    ntStatus = FT_VendorRequest(
                    FTDI_SET_BIT_MODE,
                    wValue,
                    pUsbFTDI->bInterfaceNumber,
                    NULL,
                    0,
                    OUT_USB_TRANSFER,
                    pUsbFTDI
                    );

    if (ntStatus == ERROR_SUCCESS) {
        pUsbFTDI->DevContext.BitMode = *BitMode;
    }

//    FT_KdPrint(
//        DBGLVL_DEFAULT,
//        ("FT_SetBitModeTimer wValue=%04x ntStatus=0x%x\n",
//        wValue,
//        ntStatus)
//        );

    return ntStatus;
}


DWORD
FT_GetBitMode(
	PFTDI_DEVICE pUsbFTDI,
    PUCHAR pMode 
    )
{
    DWORD ntStatus;
    UCHAR buf;

	if (!IS_BM_DEVICE(pUsbFTDI)) {
		return STATUS_UNSUCCESSFUL;
	}

    ntStatus = FT_VendorRequest(
                   FTDI_GET_BIT_MODE,
                   0,
                   pUsbFTDI->bInterfaceNumber,
                   (void **) &buf,
                   sizeof(UCHAR),
                   IN_USB_TRANSFER,
                   pUsbFTDI
                   );

    if (ntStatus == ERROR_SUCCESS) {
        *pMode = buf;
    }

//    FT_KdPrint(
//        DBGLVL_DEFAULT,
//        ("FT_GetBitMode ntStatus=%x *pMode=%x\n",ntStatus,*pMode)
//        );

    return ntStatus;
}


DWORD
FT_SetState(
	PFTDI_DEVICE pUsbFTDI
    )
{
    DWORD ntStatus = 0;

    return ntStatus;
}



DWORD
FT_ResetDevice(
	PFTDI_DEVICE pUsbFTDI
    )
/*++

Routine Description:
        Checks port status; if OK, return success and  do no more;
        If bad, attempt reset

Arguments:

    DeviceObject - pointer to the device object for this instance of the 82930
                    device.


Return Value:

    NT status code

--*/
{
    DWORD ntStatus = STATUS_SUCCESS;
//    ULONG portStatus;

//    FT_KdPrint(5,("Enter FT_ResetDevice()\n"));
    
    //
    // Check the port state, if it is disabled we will need 
    // to re-enable it
    //
//    ntStatus = FT_GetPortStatus(pUsbFTDI, &portStatus);

	/*  ST you are supposed to check if the devce needs a reset or not - how do you do this ????
    if (NT_SUCCESS(ntStatus) && !(portStatus & USBD_PORT_ENABLED) &&
        portStatus & USBD_PORT_CONNECTED) {
        //
        // port is disabled, attempt reset
        //
//                FT_KdPrint( DBGLVL_DEFAULT,("FT_ResetDevice() will reset\n"));
//        ntStatus = FT_ResetParentPort(pUsbFTDI);
    }*/
    return ntStatus;
}


VOID
FT_GetStringDescriptor(
	PFTDI_DEVICE pUsbFTDI,
    UCHAR Index,
    PUCHAR Buffer,
    WORD MaxLen
    )
{
	USB_DEVICE_REQUEST ControlHeader;
    PWCHAR Descriptor = NULL;
    DWORD ntStatus;
    WORD DescSiz;
	DWORD  dwBytesTransferred = 0;
	DWORD  dwUsbErr = USB_NO_ERROR;
    DWORD  dwErr = ERROR_SUCCESS;
    USHORT usStatus = 0;
	DWORD status = ERROR_SUCCESS;
	int len;

//    if (Index != 0) {

        /* allow for descriptor length and type, and null terminator */
        DescSiz = (MaxLen+2) * sizeof(WCHAR);
        Descriptor = (PWCHAR)FT_LocalAlloc(LMEM_ZEROINIT,DescSiz);

        if (!Descriptor)
            return;

		/* This needs to change for vendor DEVICE to HOST for all the Gets */
		ControlHeader.bmRequestType = USB_REQUEST_DEVICE_TO_HOST |
				USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE;

		ControlHeader.bRequest		= USB_REQUEST_GET_DESCRIPTOR;
		ControlHeader.wValue		= ((WORD)USB_STRING_DESCRIPTOR_TYPE) << 8 | Index;
		ControlHeader.wIndex		= 0;
		ControlHeader.wLength		= DescSiz;
			
		ntStatus = IssueVendorTransfer(	
						pUsbFTDI->UsbFuncs,
						pUsbFTDI->hUsbDevice,
						DefaultTransferComplete,
						pUsbFTDI->hEP0Event,
						(USB_IN_TRANSFER | USB_SHORT_TRANSFER_OK),
						&ControlHeader,
						Descriptor, 0,
						&dwBytesTransferred,
						pUsbFTDI->UsbTimeouts.SoftResetTimeout,
						&dwUsbErr 
						);

	    if (ntStatus == ERROR_SUCCESS) {
            PUCHAR Ptr = (PUCHAR) Descriptor;
            USHORT DescBytes = ((USHORT) *Ptr) - 2;

//			if(Index != 0) {
			// If pucLanguage is NULL, we're only trying to gett the size
			// If index is 0, we could still need to append interface identifiers
			// and force an empty serial number when it is disabled.
//			if(pucLanguage != NULL) {
			FT_CopyWStrToStr(Buffer,(USHORT)MaxLen,Descriptor+1,DescBytes);
			len = strlen(Buffer);

			if ((Index == 0) & (pUsbFTDI->pUsbDevice->Descriptor.iSerialNumber == 0)) {
				// Serial number is disabled
				// We previously returned garbage here
				// Also did not append the interface identifier for multi-interface 
				// chips as is done on Windows driver...
				Buffer[0] = (char)NULL;
				Buffer[1] = (char)NULL;
				len = 0;
			}

			if(IS_MULTI_IF_DEVICE(pUsbFTDI)) {
				if(pUsbFTDI->bInterfaceNumber == FT2232_INTERFACE_A) {
					// Add  A
					if(Index == pUsbFTDI->pUsbDevice->Descriptor.iProduct) {
						Buffer[len++] = ' ';
					}
					Buffer[len++] = 'A';
					//Buffer[len++] = (char)FT2232_INTERFACE_A_SUFFIX;
					Buffer[len++] = (char)NULL;
				}
				else if (pUsbFTDI->bInterfaceNumber == FT2232_INTERFACE_B){
					// Add  B
					if(Index == pUsbFTDI->pUsbDevice->Descriptor.iProduct) {
						Buffer[len++] = ' ';
					}
					Buffer[len++] = 'B';
					//Buffer[len++] = (char)FT2232_INTERFACE_B_SUFFIX;
					Buffer[len++] = (char)NULL;
				}
				if (IS_FT4232H(pUsbFTDI)) {
					if (pUsbFTDI->bInterfaceNumber == FT2232_INTERFACE_C) {
						// Add  C
						if(Index == pUsbFTDI->pUsbDevice->Descriptor.iProduct) {
							Buffer[len++] = ' ';
						}
						Buffer[len++] = 'C';
						//Buffer[len++] = (char)FT2232_INTERFACE_C_SUFFIX;
						Buffer[len++] = (char)NULL;
					}
					else if (pUsbFTDI->bInterfaceNumber == FT2232_INTERFACE_D) {
						// Add  D
						if(Index == pUsbFTDI->pUsbDevice->Descriptor.iProduct) {
							Buffer[len++] = ' ';
						}
						Buffer[len++] = 'D';
						//Buffer[len++] = (char)FT2232_INTERFACE_D_SUFFIX;
						Buffer[len++] = (char)NULL;
					}
				}
			}
			else {
				if (Index == 0) {
					memcpy(Buffer, Descriptor, MaxLen);
				}
			}
//			}			
	    }
		else {
			DEBUGMSG2(ZONE_ERROR, TEXT("FT_GetStringDescriptor error %d\n"), ntStatus);
            *Buffer = 0;
		}
        FT_LocalFree(Descriptor);
//    }
}


DWORD
FT_GetDescriptor(
				 PFTDI_DEVICE pUsbFTDI,
				 PCHAR pBuffer,
				 WORD bIndex, 
				 WORD Length
				 )

{
	USB_DEVICE_REQUEST ControlHeader;
	DWORD  dwBytesTransferred = 0;
	DWORD  dwUsbErr = USB_NO_ERROR;
    DWORD  dwErr = ERROR_SUCCESS;
    USHORT usStatus = 0;
	DWORD status = ERROR_SUCCESS;
//	USB_TRANSFER val;

	/* This needs to change for vendor DEVICE to HOST for all the Gets */
	ControlHeader.bmRequestType = USB_REQUEST_DEVICE_TO_HOST |
            USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE;

	ControlHeader.bRequest		= USB_REQUEST_GET_DESCRIPTOR;
	ControlHeader.wValue		= ((WORD)USB_STRING_DESCRIPTOR_TYPE) << 8 | bIndex;
	ControlHeader.wIndex		= 0;
	ControlHeader.wLength		= Length;
	
/*	val =  USBDIssueVendorTransfer(pUsbFTDI->hUsbDevice,
							NULL,
							NULL,
							USB_OUT_TRANSFER,
							&ControlHeader,
							(PVOID)&Buffer,
							0l);

	if(val)
		return 0;
	return 1;*/
	
	return IssueVendorTransfer(	
				pUsbFTDI->UsbFuncs,
				pUsbFTDI->hUsbDevice,
				DefaultTransferComplete,
				pUsbFTDI->hEP0Event,
				USB_IN_TRANSFER ,
				&ControlHeader,
				pBuffer, 0,
				&dwBytesTransferred,
				pUsbFTDI->UsbTimeouts.SoftResetTimeout,
				&dwUsbErr 
				);
}

DWORD
FT_GetDeviceSerialNumber(
						PFTDI_DEVICE pUsbFTDI
						)
{

	FT_GetStringDescriptor(
							pUsbFTDI,
							pUsbFTDI->pUsbDevice->Descriptor.iSerialNumber,
							pUsbFTDI->SerialNumber,
							FT_MAX_SERIAL_NUMBER_LEN+1
							);
	return ERROR_SUCCESS;


}
								
DWORD
FT_GetDeviceDescription(
						PFTDI_DEVICE pUsbFTDI
						)
{
	FT_GetStringDescriptor(
							pUsbFTDI,
							pUsbFTDI->pUsbDevice->Descriptor.iProduct,
							pUsbFTDI->Description,
							FT_MAX_DESCRIPTION_LEN+1
							);
	return ERROR_SUCCESS;
	
}

DWORD
FT_ResetHardware(
				 PFTDI_DEVICE pUsbFTDI
				 )
{
	DWORD dwStatus;
	DWORD dwWaitReturn;
	DWORD dwBytesTransferred	= 0;
	DWORD dwUsbErr				= USB_NO_ERROR;
	USB_DEVICE_REQUEST ControlHeader;
	DWORD TransferFlags = OUT_USB_TRANSFER;

	ControlHeader.bmRequestType = USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_VENDOR;
	ControlHeader.bRequest		= FTDI_RESET;
	ControlHeader.wValue		= 0;
	ControlHeader.wIndex		= pUsbFTDI->bInterfaceNumber;
	ControlHeader.wLength		= 0;

	ResetEvent(pUsbFTDI->hEP0Event);	/* This is the only place that uses this so there wont be a problem */
	dwStatus = IssueVendorTransfer(	
					pUsbFTDI->UsbFuncs,
					pUsbFTDI->hUsbDevice,
					DefaultTransferComplete,
					pUsbFTDI->hEP0Event,
					TransferFlags,
					&ControlHeader,
					NULL, 0,
					&dwBytesTransferred,
					pUsbFTDI->UsbTimeouts.SoftResetTimeout,
					&dwUsbErr 
					);

	if (ERROR_SUCCESS != dwStatus || USB_NO_ERROR != dwUsbErr) {
            //RETAILMSG(1, (TEXT("Reset Hardware IVT ERROR:%d 0x%x\n"), dwStatus, dwUsbErr));
		DEBUGMSG3( ZONE_ERROR, TEXT("IssueVendorTransfer ERROR:%d 0x%x\n"), dwStatus, dwUsbErr);
		dwStatus = IoErrorHandler(pUsbFTDI, NULL, 0, dwUsbErr);
		return dwStatus;
	}

	dwWaitReturn = WaitForSingleObject(pUsbFTDI->hEP0Event, 5000); // 5 sec timeout
	switch (dwWaitReturn) {
	case WAIT_OBJECT_0:
		DEBUGMSG1( ZONE_FUNCTION, (TEXT("FT_ResetHardware OK\n")));
		dwStatus = ERROR_SUCCESS;
		break;
	case WAIT_FAILED:
		DEBUGMSG2( ZONE_ERROR, TEXT("FT_ResetHardware error:%d\n"), GetLastError());
		break;
	default:
		DEBUGMSG2( ZONE_ERROR, TEXT("Unhandled FT_ResetHardware WaitReason:%d\n"), dwWaitReturn );
		break;
	};

	return dwStatus;
}

DWORD
FT_SetDeviceEvent(
				  PFTDI_DEVICE pUsbFTDI,
				  FTNotifyParams * pNotifyParams
				  )
{
	EnterCriticalSection(&pUsbFTDI->NotifyLock);

	pUsbFTDI->Notify.dwMask = pNotifyParams->dwMask;
	pUsbFTDI->Notify.dwRef = pNotifyParams->dwRef;
	pUsbFTDI->Notify.pvParam = pNotifyParams->pvParam;

#ifndef WINCE6
	pUsbFTDI->dwNotifyPermissions = GetCurrentPermissions();
#endif

	DEBUGMSG3(ZONE_ERROR, TEXT("FT_SetDeviceEvent:%p, event %p\n"), pUsbFTDI, pUsbFTDI->Notify.pvParam);

	LeaveCriticalSection(&pUsbFTDI->NotifyLock);

	return 0;
}

/*++

Routine Description:
        On an unplug replug if the user wants a persistant COM port then this must be called to return the device
		to its original state.

Arguments:

    pUsbFTDI - pointer to the device instance
       
Return Value:

    TRUE: Success, FALSE otherwise

--*/
BOOL
FT_RestoreDeviceSettings(
	PFTDI_DEVICE pUsbFTDI
	)
{
	//
	// Restore Baud rate
	//
	FT_SetBaudRate(pUsbFTDI, pUsbFTDI->DevContext.BaudRate);

	//
	// Restore flow control
	//
	FT_SetFlowControl(pUsbFTDI, &pUsbFTDI->DevContext.FlowControl);

	//
	// Restore Data characteristics
	//
	FT_SetLineControl(pUsbFTDI, &pUsbFTDI->DevContext.LineControl);

	//
	// Modem control
	//
	if(pUsbFTDI->DevContext.MCR & FT_MCR_DTR) {
		FT_SetDtr(pUsbFTDI->pHWHead);
	}
	if(pUsbFTDI->DevContext.MCR & FT_MCR_RTS) {
		FT_SetRts(pUsbFTDI->pHWHead);
	}

	//
	// Event character - I dont really understand this part of the driver - I suspect this will be why dial up is slow
	// ask andy when he has a minute
	//
	FT_SetChars(pUsbFTDI, &pUsbFTDI->SpecialChars);

	return TRUE;
}

