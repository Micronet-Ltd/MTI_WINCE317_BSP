/*++

Copyright (c) 2000-2012 Future Technology Devices International Ltd.

Module Name:

    bulk_in.c

Abstract:

    Native USB device driver for FTDI FT8U245
    Bulk IN Endpoint

Environment:

    Kernel mode

Revision History:

	22/06/04	st		Adapt from Windows driver.
	05/07/04	st		Changed Timeouts for USB Bulk transfer
	13/08/04	st		Added support to reduce the bulk in buffer size if failed with out of memory error.
	23/08/04	st		Changes to the InTransferSize so it is in a per device basis
	25/11/04	st		Handled diconnect on open properly. Previously it was going into an infinitle loop.
	07/03/05	st		Changes to the DEBUG messages to make easier filtering.
	22/03/05	st		Changes to support 232R variable IN endpoint sizes.
	15/04/05	st		Changes to bulk in on read completion.
	22/06/05	st		Changes made to allow Modem Emulation mode to work. Some white space cleanup and removal of excess/unneeded code
	27/06/05	st		Comment additions.
	30/06/05	st		Changes made to allow BulkFlags to be altered. Changes made to allow Bulk Transfers to be synchronous or asynchronous
	17/08/05	st		Cleanup.
	19/09/05	st		New InRequest to deal with noisy devices. Return instead of exit thread (to allow porper cleanup).
	24/01/06	st		Debug change.
	30/08/06	st		Stall error handled properly.
	09/03/07	st		Cleanup.
	15/11/07	ma		Added #ifndef WINCE6 to remove proc permissions functions for CE 6 build. Added some extra debug.
	15/11/07	ma		Added more required #ifndef WINCE6.
	04/06/08	ma		Added hi-speed chip support. FT4232H and FT2232H.
	03/03/11	ma		Added a surprise remove flag to handle surprise disconnects better (in particular to allow WM_DEVICECHANGE messages to propagate).
						Added support for the FT232H.
	21/11/11	ma		Improvements for surprise disconnects.
	04/01/12	ma		Added an event to synchronise the bulk task with open; wait on an event to indicate that the bulk task is ready rather than an arbitrary 16ms.
						Also, if the bulk buffer is still allocated when the bulk task has been forcibly terminated, free the buffer.
	27/01/12	ma		Fix data loss in driver when exactly 256 bytes received in the buffer from the UART. 
						Data size was cast to a BYTE but had an exception for zero data (or 256 bytes) where data would be ignored and interrupt bit cleared.

--*/

#include <windows.h>
#include <serhw.h>
#include "ftdi_ioctl.h"
#include "BUSBDBG.H"
#include "ftdi_debug.h"
#include <serdbg.h>
#include <celog.h> 
#include <pkfuncs.h>

#define DATA_OFFSET                 2

#define PDEVICE_EXTENSION PFTDI_DEVICE

DWORD WINAPI
BulkInTask(
    IN PVOID Context
    );

void
FT_ProcessBulkIn(
    PSER_INFO pDevExt,
    UCHAR *buffer,
    ULONG BufSiz
    );

void
FT_ProcessBulkInEx(
	PSER_INFO Extension,
    UCHAR *Buffer,
    ULONG BufSiz,
	ULONG UsbPacketSize
    );

int
FT_CheckReadAndDequeue(
    PDEVICE_EXTENSION pDevExt
    );

void FT_DebugPrint(
    PCHAR title,
    PUCHAR buf,
    ULONG len,
    ULONG DevNum,
    PDEVICE_EXTENSION pDevExt
    );


NTSTATUS
InRequest(
    PVOID Buffer,
    ULONG Length,
    PULONG BytesRead,
    PDEVICE_EXTENSION pDevExt
    );

ULONG
FT_GetBytesPerTransfer(
	PFTDI_DEVICE pDevExt,
    ULONG TransferLength
    );
   
//
// Bulk IN states
//
enum {
    BULK_INIT,
    BULK_WAIT,
    BULK_CONFIGURE,
    BULK_IN,
    BULK_ERROR,
    BULK_STOP,
    BULK_EXIT
};

DWORD
FTDIReadTransferComplete(
   PVOID Context
   )
{
	HANDLE hEvent = (HANDLE)Context;
	DWORD dwErr = ERROR_SUCCESS;

	DEBUGMSG1(ZONE_FUNCTION, (TEXT("FTDIReadTransferComplete\n")));
	
//	DEBUGMSG2(ZONE_FUNCTION, TEXT("FTDIReadTransferComplete hEvent: 0x%X\n"), hEvent);

	if (hEvent) {

		//
		// The current operation completed, signal the event
		//
		if (!SetEvent(hEvent)) {
			dwErr = GetLastError();
			DEBUGMSG2(ZONE_ERROR, TEXT("*** SetEvent ERROR:%d ***\n"), dwErr);
		}
	} else {
		dwErr = ERROR_INVALID_HANDLE;
		SetLastError(dwErr);
		DEBUGMSG2( ZONE_ERROR, TEXT("*** FTDIReadTransferComplete ERROR:%d ***\n"), dwErr);
	}
	return dwErr;
}

DWORD WINAPI
BulkInTask(
    IN PVOID Context
    )

/*++

Routine Description:

    The BULK IN task.

Arguments:

    Context - pointer to device extension.

Return Value:

    None

--*/

{
    ULONG state;
    CHAR defaultBuffer[FT_USBD_PACKET_SIZE_HI];
    ULONG TransferLength;
    ULONG ActualTransferLength;
    ULONG BytesRead;
    ULONG nBytesToBeProcessed;
    ULONG iBytesToBeProcessed;
    ULONG usbPacketSize;
    PCHAR buffer;
	PFTDI_DEVICE pDevExt = NULL;
	PSER_INFO pHWHead;
	DWORD dwWaitReturn;
	DWORD dwErr = ERROR_SUCCESS;
	DWORD dwUsbErr = USB_NO_ERROR;
    NTSTATUS ntStatus;
    ULONG rxBytes;
	ULONG ulNewTransferSize;
	HANDLE aEvents[2];					// Our multiple events to wait on

	UCHAR latTimer;

	pHWHead = (PSER_INFO)Context;
	pDevExt = (PFTDI_DEVICE)pHWHead->pFTDICtxt;

    state  = BULK_INIT;
	
//	RETAILMSG(1, (TEXT("BULK IN STARTED\r\n")));

    while (state != BULK_EXIT) {

		DEBUGMSG2(ZONE_ERROR, TEXT("Entering Bulk state %d\n"),state);

        switch (state) {
        case BULK_INIT :
//			RETAILMSG(1,(TEXT("Bulk state: BULK_INIT\n")));
			buffer = NULL;
			pDevExt->RestartExpected = FALSE;
            state = BULK_CONFIGURE;				// unlike windows version
			break;

        case BULK_WAIT :
			/* This is where we wait if we are stopped */
//			RETAILMSG(1,(TEXT("Bulk state: BULK_WAIT\n")));

			aEvents[0] = pDevExt->hCloseReaderEvent;
			aEvents[1] = pDevExt->hRestartEvent;

			dwWaitReturn = WaitForMultipleObjects(2, aEvents, FALSE, INFINITE);

			if((dwWaitReturn - WAIT_OBJECT_0) == 0) {
                if (buffer && (buffer != defaultBuffer)) {
//					RETAILMSG(1, (TEXT("BULK Free buffer - normal\r\n")));
                    FT_LocalFree(buffer);
                    buffer = NULL;
					pDevExt->BulkBuffer = NULL;
                }
				
//				RETAILMSG(1, (TEXT("BULK close thread - normal\r\n")));
				DEBUGMSG1(ZONE_FUNCTION, (TEXT("Close BULK Task signalled\n")));
				return 0;
			}
			else if((dwWaitReturn - WAIT_OBJECT_0) == 1) {

				//
				// RESTART_EVENT
				//
				DEBUGMSG1(ZONE_FUNCTION, (TEXT("BULK_IN - got RESTART EVENT\n")));
				ResetEvent(pDevExt->hRestartEvent);
				pDevExt->RestartExpected = FALSE;
				state = BULK_IN;

				//
				// Give us a chance to reinitialise the device
				//
				// Sleep(1000); // removed to allow faster replug response - possibly required (comment deprecated gdm)
			}
			else {
								
                //
                // SHOULDN'T REACH HERE!
                //

				DEBUGMSG2(ZONE_ERROR, TEXT("BULK_WAIT Unhandled return code, Error = %d\n"), GetLastError());	// +++

				// Used to spin in WAIT state if we got here - now just close down!

				// Assume we're gone!
				pDevExt->Flags.SurpriseRemoved = TRUE;

				// Need to get rid our instance though...
				// Try to use the notify function to do this... (normally the system would provide this, but doesn't seem to sometimes)
				// deprecate call
//				DeviceNotify(pDevExt, USB_CLOSE_DEVICE, NULL, NULL, NULL, NULL);

				// Free our bulk buffer
				if (buffer && (buffer != defaultBuffer)) {
//					RETAILMSG(1, (TEXT("BULK Free buffer - shouldn't get here\r\n")));
                    FT_LocalFree(buffer);
                    buffer = NULL;
					pDevExt->BulkBuffer = NULL;
                }
				
//				RETAILMSG(1, (TEXT("BULK close thread - shouldn't get here\r\n")));
				DEBUGMSG1(ZONE_FUNCTION, (TEXT("Closing BULK Task\n")));
				return 0;
			}

			break;

        case BULK_CONFIGURE :
			/* Allocate the buffer etc... */
//			RETAILMSG(1,(TEXT("Bulk state: BULK_CONFIGURE\n")));
			DEBUGMSG1(ZONE_FUNCTION, (TEXT("BULK_CONFIGURE\n")));
			
			usbPacketSize = pDevExt->BulkIn.wMaxPacketSize;
			DEBUGMSG2(ZONE_FUNCTION, TEXT("UsbPacketSize = %d\n"), usbPacketSize);

			TransferLength = pDevExt->InParams.MaximumTransferSize;
			buffer = (UCHAR *)FT_LocalAlloc(0, TransferLength);

			if (!buffer) {
				// something has gone badly wrong, revert to default 
				buffer = defaultBuffer;
				TransferLength = usbPacketSize;	/*sizeof(defaultBuffer);*/
				pDevExt->InParams.MaximumTransferSize = TransferLength;
				pDevExt->InParams.CurrentTransferSize = TransferLength;
			}
			else {
				// save pointer in our device extension so we can free this if we have to kill the thread!
				pDevExt->BulkBuffer = buffer;
			}

			ActualTransferLength = FT_GetBytesPerTransfer(pDevExt, TransferLength);

			DEBUGMSG3(ZONE_FUNCTION, TEXT("BULK_IN CONFIGURE Allocated %d for actual transfer of %d\n"),
									TransferLength,ActualTransferLength);
			nBytesToBeProcessed = 0;

			//
			// transfer size changed, so setup deadman timeout.
			// 
#if 0 // deprecated call
			// FT_SetDeadmanTimeout(pDevExt);
#endif // 0
			
			SetEvent(pDevExt->hReaderConfiguredEvent);
			state = BULK_IN;
			break;

        case BULK_IN :
			/* The main reading event! */
	
			//RETAILMSG(1,(TEXT("Bulk State: BULK_IN\n")));

            if (pDevExt->TimeToTerminateThread) {
                state = BULK_WAIT;
                break;
            }

            if (pDevExt->TimeToSleep) {
                state = BULK_WAIT;
                break;
            }

            if (pDevExt->StopSignalled) {
                state = BULK_STOP;
                break;
            }

            if (pDevExt->ResetBuffer) {
				pDevExt->ReadBuffer.rbSize = FT_SIZE(pDevExt->InParams.MaximumTransferSize, pDevExt);
				pDevExt->ReadBuffer.rbPut = 0;
				pDevExt->ReadBuffer.rbGet = 0;
				pDevExt->ReadBuffer.rbBytesFree = pDevExt->ReadBuffer.rbSize;
				pDevExt->ResetBuffer = FALSE;
            }

            //
            // When the buffer is empty, setup the current transfer size
            //
            if (TransferLength != pDevExt->InParams.CurrentTransferSize) {
                TransferLength = pDevExt->InParams.CurrentTransferSize;
                ActualTransferLength = FT_GetBytesPerTransfer(pDevExt, TransferLength);
 				DEBUGMSG3(ZONE_FUNCTION, TEXT("BULK_IN CONFIGURE Allocated %d for actual transfer of %d\n"),
										TransferLength,ActualTransferLength);
                //
                // transfer size changed, so setup deadman timeout.
                // 

#if 0 // deprecated call
				// FT_SetDeadmanTimeout(pDevExt);
#endif // 0
				
				DEBUGMSG1(ZONE_FUNCTION, (TEXT("BULK_IN wait for hBufferMutex")));
 				DEBUGMSG1(ZONE_FUNCTION, (TEXT("BUFFER SIZE CHANGING\n")));
				dwWaitReturn = WaitForSingleObject(pDevExt->ReadBuffer.hBufferMutex, INFINITE);
				if(WAIT_OBJECT_0 == dwWaitReturn) {
					pDevExt->ReadBuffer.rbSize = FT_SIZE(pDevExt->InParams.MaximumTransferSize, pDevExt);
					pDevExt->ReadBuffer.rbPut = 0;
					pDevExt->ReadBuffer.rbGet = 0;
					pDevExt->ReadBuffer.rbBytesFree = pDevExt->ReadBuffer.rbSize;
				}
				else {
 					DEBUGMSG1(ZONE_ERROR, (TEXT("BULK_IN Wait for mutex failed\n")));
				}
 				DEBUGMSG1(ZONE_FUNCTION, (TEXT("BULK_IN releasing hBufferMutex")));
				ReleaseMutex(pDevExt->ReadBuffer.hBufferMutex);
            }


			/* Do all of your read requests from within the bulk task - that way you can fill up a huge read buffer
				without the mess of the timeout method you were initally implementing */
            if (pDevExt->ReadBuffer.rbBytesFree != pDevExt->ReadBuffer.rbSize) {

				rxBytes = pDevExt->ReadBuffer.rbSize - pDevExt->ReadBuffer.rbBytesFree;
		
#if 0
				// This isnt required for the VCP as the COM layer handles the timeouts etc.
				if (readIrp = FT_CheckReadAndDequeue(pDevExt)) {

  					DEBUGMSG1(ZONE_THREAD, (TEXT("BULK_IN completing read\n")));

                    FT_ProcessRead(pDevExt, readIrp);
                    FT_CompleteReadIrp(pDevExt);
                }				
#endif // 0				

			   /* Tell them there is data to read LAST of all */
				if(rxBytes) {
					pHWHead->cIntStat |= RX_INT_MASK;
					SetEvent(pHWHead->hSerialEvent);
 					//DEBUGMSG1(1, (TEXT("rxBytes Signalled\n")));
				}
            }

            //
            // Check if we are still processing the last request
            //

            if (nBytesToBeProcessed != 0
             && pDevExt->ReadBuffer.rbBytesFree >= nBytesToBeProcessed) {

                //
                // Process it - nBytesToBeProcessed and
                // iBytesToBeProcessed are valid at this point.
                //

                //
                // we must add status bytes so we can process
                // it properly
                //

                iBytesToBeProcessed -= 2;
                nBytesToBeProcessed += 2;

                buffer[iBytesToBeProcessed] = buffer[0];
                buffer[iBytesToBeProcessed+1] = buffer[1];

                FT_ProcessBulkInEx(
                    pHWHead,
                    &buffer[iBytesToBeProcessed],
                    nBytesToBeProcessed,
					usbPacketSize
                    );
                nBytesToBeProcessed = 0;
            }

            BytesRead = 0;

            if (pDevExt->ReadBuffer.rbBytesFree >= pDevExt->BulkIn.wMaxPacketSize) {
                ULONG TempLen;
                if (pDevExt->ReadBuffer.rbBytesFree >= ActualTransferLength)
                    TempLen = TransferLength;
                else {
                    TempLen = pDevExt->ReadBuffer.rbBytesFree;
                    TempLen &= (0xffffffff - usbPacketSize)+1;	//should result in 0xffffffc0;
                    //TempLen &= 0xffffffc0;
                }
				ntStatus = InRequest(
								buffer,
								TempLen,
								&BytesRead,
								pDevExt
								);

            }
            else if (pDevExt->ReadBuffer.rbBytesFree != 0 && nBytesToBeProcessed == 0) {

                //
                // There is some space left, so try to fill it.
                // We must request at least pDevExt->BulkIn.wMaxPacketSize,
                // so it may be necessary to process the data in
                // two stages.
                //
				ntStatus = InRequest(
								buffer,
								usbPacketSize, //obtained from pDevExt->BulkIn.wMaxPacketSize,
								&BytesRead,
								pDevExt
								);

                if (ntStatus == ERROR_SUCCESS
                 && ((BytesRead-2) > pDevExt->ReadBuffer.rbBytesFree)) {
                    //
                    // There is enough here to fill the buffer.
                    // Adjust BytesRead so that we fill the buffer,
                    // and save some for the next time.
                    //
                    iBytesToBeProcessed = pDevExt->ReadBuffer.rbBytesFree+2;
                    nBytesToBeProcessed = BytesRead - iBytesToBeProcessed;
                    BytesRead = iBytesToBeProcessed;
                }
                else
                    nBytesToBeProcessed = 0;

            }
            else {
				DEBUGMSG2(ZONE_ERROR, TEXT("WAITING FOR FREE SPACE - bytes free = %d\n"), pDevExt->ReadBuffer.rbBytesFree);	// +++

				// At this point, we need to make sure we're not stuck when the device is unplugged
				if (pDevExt->Flags.UnloadPending)
				{
					state = BULK_WAIT;
					break;
				}

				Sleep(0);		// Give up your time slice and wait for application to do some reading

				// At this point, we need to make sure we're not stuck when the device is unplugged
				// Occasionally, we can issue a vendor request to make sure we're still connected
				ntStatus = FT_GetLatencyTimer(pDevExt, &latTimer);
				if (ntStatus != ERROR_SUCCESS) {
					// we've probably been removed
					DEBUGMSG1(ZONE_ERROR, TEXT("Failed to get latency - surprise removed?\n"));	// +++
					// set the surprise removed flag and move to the WAIT state

					state = BULK_WAIT;
					break;
				}
				else {
					DEBUGMSG3(ZONE_ERROR, TEXT("Got latency timer = %d (status = %x)\n"), latTimer,ntStatus);	// +++
				}
            }

            if (ntStatus == ERROR_SUCCESS) {

                FT_ProcessBulkInEx(pHWHead,buffer,BytesRead,usbPacketSize);
				if((pDevExt->ConfigDataFlags & FTDI_BULK_IN_ON_GEN_FAILURE)) {
					pDevExt->dwGeneralFailure = 0;
				}

            }
			else {

				DEBUGMSG2(ZONE_ERROR, TEXT("BULK_ERROR =  %d\n"), ntStatus);
				state = BULK_ERROR;

			}
        
			break;

        case BULK_ERROR :

//			RETAILMSG(1,(TEXT("Bulk state: BULK_ERROR\n")));
			if(ntStatus == ERROR_OUTOFMEMORY) {

				ulNewTransferSize = 64;			// change to mininum transfer size - they can change the size in the registry setting
				pDevExt->InParams.CurrentTransferSize = ulNewTransferSize;
				DEBUGMSG2(ZONE_ERROR, TEXT("Setting In Size to %d\n"), ulNewTransferSize);
				state = BULK_IN;
//				RETAILMSG(1,(TEXT("Bulk state: BULK_ERROR memory\n")));

			}
			else if (ntStatus == ERROR_TIMEOUT) {
				// based on customer report unplugging a flash disk from a hub with
				// us connected is resulting in a timeout
				//
				// If this happens, move back to the BULK_IN state to retry rather than give up
				state = BULK_IN;
#if 0	// to recover to the BULK_IN state rather than BULK_WAIT on surprise removal
				timeoutCount++;
				if (timeoutCount == 2)
				{
					// if we process 2 timeouts in a row, we seem to get stuck in the bulk in state
					// particularly after a surprise remove; try to handle it here...
					// Assume we're gone in the case of 2 timeouts
					pDevExt->Flags.SurpriseRemoved = TRUE;
					state = BULK_WAIT;
				}
#endif // 0
//				RETAILMSG(1,(TEXT("Bulk state: BULK_ERROR timeout\n")));
			}
			else if(ntStatus == ERROR_GEN_FAILURE) {
				
				if((pDevExt->ConfigDataFlags & FTDI_BULK_IN_ON_GEN_FAILURE) && 
					(pDevExt->dwGeneralFailure++ < 50)) {
					// This to catch any recurrent general failure loops due to unplugs

					//
					// On initial install and first open certain devices have a ERROR_GEN_FAILURE on the 2nd
					// bulk in request. Previously we were using ERROR_GEN_FAILURE to detect a disconnect on open port
					// now if it is any other error than these 2 just go to the wait for close state.
					//
					state = BULK_IN;
				}
				else {
//					RETAILMSG(1,(TEXT("Bulk state: BULK_ERROR surprise disconnect\n")));
					DEBUGMSG1(ZONE_ERROR, (TEXT("Disconnect on open port\n")));
					pDevExt->Flags.SurpriseRemoved = TRUE;
					state = BULK_WAIT;
				}

			}
			else { // all other error codes...

//				RETAILMSG(1,(TEXT("Bulk state: BULK_ERROR unhandled (%x)\n"),ntStatus));
			
				// assume this is a disonnect or a dodgy device
				DEBUGMSG2(ZONE_ERROR, TEXT("Unhandled BULK_ERROR(%d)\n"), ntStatus);
				pDevExt->Flags.SurpriseRemoved = TRUE;
				state = BULK_WAIT;

			}
			break;
        
		case BULK_STOP :

            //
            // STOP request received, so prepare to go to WAIT state
            //

//			RETAILMSG(1,(TEXT("Bulk state: BULK_STOP\n")));
			DEBUGMSG1(ZONE_ERROR, (TEXT("BULK_IN STOP state entered\n")));
    
            //
            // cancel pending I/O
            //
#if 0 // deprecated
			while (readIrp = FT_DequeueReadIrp(pDevExt)) {
                FT_KdPrint(DBGLVL_DEFAULT,("BULK_IN STOP cancelling IRP 0x%x\n",readIrp));
                FT_CancelReadIrp(readIrp);
            }
#endif // 0
            pDevExt->RestartExpected = FALSE;		// should not get here

            state = BULK_WAIT;
			break;
		};
	}
//	RETAILMSG(1,(TEXT("Bulk state: BULK_EXIT\n")));

	return 0;
}

/*
	Most of the stuff that is done in the windows driver (like event char handling) is performed in the 
	upper layers in mdd. This might have a performance hit on when you detect an event but will wait until someone
	comes in with a problem.
 */
void
FT_ProcessBulkIn(
	PSER_INFO pSerInfo,
	UCHAR *Buffer,
	ULONG BufSiz
	)
{
    UCHAR ModemStatus;
    ULONG BytesToProcess;
    ULONG temp;
    PUCHAR Dest;
    PUCHAR DestEnd;
    PUCHAR BufferEnd;
    BOOLEAN GotEventChar = FALSE;
	PFTDI_DEVICE Extension;

	Extension = pSerInfo->pFTDICtxt;

	ASSERT(BufSiz >= DATA_OFFSET);

	//
    // Buffer[0] contains Modem Status Register
	//

    Buffer[0] &= 0xf0;  // we are only interested in the status bits

    //
    // Build MSR
    // Bits 7-4 are returned in Buffer[0], Bits 3-0 are the change bits.
    //

    ModemStatus = Buffer[0];
    ModemStatus = (ModemStatus ^ Extension->SerialRegs.MSR) & 0xf0;
    ModemStatus >>= 4;
    ModemStatus |= Buffer[0];

    if (ModemStatus != Extension->SerialRegs.MSR) {
        UCHAR DcdStatus;

		Extension->OldModemStatus = Extension->SerialRegs.MSR;
		Extension->SerialRegs.MSR = ModemStatus;

        //
        // Check for disconnect (drop DCD). If we detect this condition
        // and there is data to process, we must defer handling the change
        // in DCD until the data has been processed.
        //
        DcdStatus = (ModemStatus & (SERIAL_MSR_DDCD|SERIAL_MSR_DCD));

        if (DcdStatus == SERIAL_MSR_DDCD
         && BufSiz > DATA_OFFSET) {
            Extension->DeferModemUpdate = TRUE;
        }
        else {
			pSerInfo->cIntStat |= MODEM_INT_MASK;
			SetEvent(pSerInfo->hSerialEvent);
        }
    }

	//
    // Buffer[1] contains Line Status Register
	//

    if (Buffer[1] != Extension->SerialRegs.LSR) {
        Extension->SerialRegs.LSR = Buffer[1];
		pSerInfo->cIntStat |= LINE_STATUS_INT_MASK;
		SetEvent(pSerInfo->hSerialEvent);
    }

	if (BufSiz <= DATA_OFFSET) {
		//
		// this was just a status packet
		//

        if (Extension->DeferModemUpdate) {
			pSerInfo->cIntStat |= MODEM_INT_MASK;
			SetEvent(pSerInfo->hSerialEvent);
			Extension->DeferModemUpdate = FALSE;
        }

		return;
	}

//    FT_DebugPrint("IN",Buffer,BufSiz,0);

#ifndef WINCE // code not implemented in Windows CE
    //
    // Line status errors are only reset when a character is received.
    // So, for two consecutive line status errors, we have to check
    // the line status here, where we know that we have received
    // a character.
    //

    if (Buffer[1] &
            (SERIAL_LSR_OE|SERIAL_LSR_PE|SERIAL_LSR_FE|SERIAL_LSR_BI)) {
        SerialProcessLSR(Extension);
    }
#endif

    //
    // copy data to ISR buffer
    //

    Buffer += DATA_OFFSET;
    temp = BytesToProcess = (BufSiz-DATA_OFFSET);

    if ((Extension->ReadBuffer.rbPut + BytesToProcess) >= Extension->ReadBuffer.rbSize) {
        Dest = Extension->ReadBuffer.Buffer + Extension->ReadBuffer.rbPut;
        DestEnd = Extension->ReadBuffer.Buffer + Extension->ReadBuffer.rbSize;
        BytesToProcess -= (DestEnd-Dest);

		do {
			*Dest++ = *Buffer++;
		} while (Dest != DestEnd);

        Extension->ReadBuffer.rbPut = 0;
    }

    if (BytesToProcess) {
        Dest = Extension->ReadBuffer.Buffer + Extension->ReadBuffer.rbPut;
        BufferEnd = Buffer + BytesToProcess;
		DEBUGMSG2( ZONE_FUNCTION, TEXT("FT_ProcessBulkInEx BytesToProcess = %d\n"), BytesToProcess );

		do {
			*Dest++ = *Buffer++;
		} while (Buffer != BufferEnd);

        Extension->ReadBuffer.rbPut += BytesToProcess;
    }
        
	if(temp) {	/* We copied somthing over */
		pSerInfo->cIntStat |= RX_INT_MASK;
		SetEvent(pSerInfo->hSerialEvent);
	}

    Extension->ReadBuffer.rbBytesFree -= (BufSiz - DATA_OFFSET);

    if (Extension->DeferModemUpdate) {
		pSerInfo->cIntStat |= MODEM_INT_MASK;
		SetEvent(pSerInfo->hSerialEvent);
        Extension->DeferModemUpdate = FALSE;
    }
}

void
FT_ProcessBulkInEx(
	PSER_INFO pSerInfo,
	UCHAR *Buffer,
	ULONG BufSiz,
    ULONG UsbPacketSize
	)
{
    ULONG TempLen;
	PFTDI_DEVICE Extension;

	Extension = pSerInfo->pFTDICtxt;

    if (Extension->ReadBuffer.rbSize == 0) {

        //
        // Buffer not setup - don't wait for space!
        // Only process status bytes.
        //
        FT_ProcessBulkIn(pSerInfo,Buffer,2);
        return;
    }

    while (BufSiz) {

		//
		// This loop will do wMaxPacketSize per pass through
		//
		// effective replacement UsbPacketSize is Extension->BulkIn.wMaxPacketSize

		TempLen = BufSiz > UsbPacketSize ? UsbPacketSize : BufSiz;

#ifndef WINCE
        FT_WaitForSpaceAvailable(Extension,TempLen);
#endif

        if (Extension->TimeToSleep)

            //
            // quit because space available was signalled from SerialClose
            //
            break;

        if (!FT_EMUL_SUBSTITUTION_MODE(Extension)) {
            FT_ProcessBulkIn(pSerInfo,Buffer,TempLen);
		}
        else {
            FT_EmulProcessRxPacket(pSerInfo,Buffer,TempLen);
		}

        Buffer += TempLen;
        BufSiz -= TempLen;
    }

}

BOOL CheckObjectReturnCode(DWORD dwCode)
{
	switch(dwCode) {

	case WAIT_OBJECT_0:
		return TRUE;
	case WAIT_FAILED:
		return FALSE;
	default:
		return FALSE;
	};
}

VOID
FT_ProcessRead(
    PDEVICE_EXTENSION pDevExt,
    int Irp
    )
{
    PUCHAR systemVa;
    PUCHAR srce;
    PUCHAR srceEnd;
    ULONG transferLen;
    ULONG rxBytes;
	DWORD dwWaitReturn;

#ifndef WINCE6
	DWORD dwOldPerms;
#endif

	systemVa = pDevExt->ReadRequest.pBuffer;
    
    transferLen = pDevExt->ReadRequest.dwBufferLength;

	DEBUGMSG1(ZONE_FUNCTION, (TEXT("FT_ProcessRead\n")));

	DEBUGMSG1(ZONE_FUNCTION, (TEXT("FT_ProcessRead Wait for hBufferMutex\n")));

	dwWaitReturn = WaitForSingleObject(pDevExt->ReadBuffer.hBufferMutex, INFINITE);
	if(CheckObjectReturnCode(dwWaitReturn) == TRUE) {
		//
		// Optimize transfer by performing all calculations before copy.
		//

		rxBytes = pDevExt->ReadBuffer.rbSize - pDevExt->ReadBuffer.rbBytesFree;
		DEBUGMSG2(ZONE_FUNCTION, TEXT("FT_ProcessRead rxBytes=%d\n"),rxBytes);

		if (rxBytes < transferLen) {

			transferLen = rxBytes;

			//
			// We won't be able to transfer the requested bytes, so
			// adjust the TransferLength in the URB so that the completion
			// routine can setup the Information field in IoStatus.
			//

			DEBUGMSG1(ZONE_ERROR, (TEXT("FT_ProcessRead We won't be able to transfer the requested bytes\n")));	
			TEST_TRAP();
		}

		pDevExt->ReadBuffer.rbBytesFree += transferLen;

#ifndef WINCE6
		dwOldPerms = SetProcPermissions(pDevExt->ReadRequest.dwCurrPermissions);
#endif

		//
		// if copy wraps round read buffer, do the bit to the end of the buffer
		//
		if (pDevExt->ReadBuffer.rbGet + transferLen >= pDevExt->ReadBuffer.rbSize) {
			int i = 0;
			srce = pDevExt->ReadBuffer.Buffer + pDevExt->ReadBuffer.rbGet;
			srceEnd = pDevExt->ReadBuffer.Buffer + pDevExt->ReadBuffer.rbSize;
			transferLen -= (srceEnd-srce);
			do {
				*systemVa++ = *srce++;
				i++;
			} while (srce != srceEnd);
			pDevExt->ReadBuffer.rbGet = 0;
			pDevExt->ReadRequest.dwBytesTransferred += i;
//			DEBUGMSG2( ZONE_FUNCTION, TEXT("BulkCopied from %d to end\n"), pDevExt->ReadBuffer.rbGet);
		}

		if (transferLen) {

			srce = pDevExt->ReadBuffer.Buffer + pDevExt->ReadBuffer.rbGet;
			srceEnd = srce + transferLen;
//			DEBUGMSG2( ZONE_FUNCTION, TEXT("BulkCopied from %d to"), pDevExt->ReadBuffer.rbGet);

			do {
				*systemVa++ = *srce++;
			} while (srce != srceEnd);

			pDevExt->ReadBuffer.rbGet += transferLen;
			pDevExt->ReadRequest.dwBytesTransferred += transferLen;
//			DEBUGMSG2( ZONE_FUNCTION, TEXT(" %d\n"), pDevExt->ReadBuffer.rbGet);

		}
#ifndef WINCE6
		SetProcPermissions(dwOldPerms);
#endif

		DEBUGMSG2(ZONE_FUNCTION, TEXT("pDevExt->ReadRequest.dwBytesTransferred = %d\n"), pDevExt->ReadRequest.dwBytesTransferred);

		if (pDevExt->ReadBuffer.rbBytesFree == pDevExt->ReadBuffer.rbSize)
			pDevExt->EventMask &= ~FT_EVENT_RXCHAR;
	}

	DEBUGMSG1(ZONE_FUNCTION, (TEXT("FT_ProcessRead releasing hBufferMutex\n")));
	ReleaseMutex(pDevExt->ReadBuffer.hBufferMutex);
}

// This function isn't required for the VCP as the COM layer handles the timeouts etc.
int
FT_CheckReadAndDequeue(
    PDEVICE_EXTENSION pDevExt
    )
{
    ULONG rxBytes;
    ULONG transferLen;

	int readIrp = 0;
	/* Check to see if we have a read pending */
	if(pDevExt->ReadRequest.bReadPending == TRUE) {

		transferLen = pDevExt->ReadRequest.dwBufferLength;	// at the moment this will be less than 64k

		DEBUGMSG1(ZONE_FUNCTION, (TEXT("BULK_IN completing read\n")));

        //
        // check if read can be satisfied from data already in ReadBuffer
        //
        rxBytes = pDevExt->ReadBuffer.rbSize - pDevExt->ReadBuffer.rbBytesFree;

        if (rxBytes >= transferLen) {
			DEBUGMSG1(ZONE_FUNCTION, (TEXT("BULK_IN completing read2\n")));
			readIrp = 1;
        }
        else {
			DEBUGMSG3(ZONE_ERROR, TEXT("Cant satisfy request rxBytes = %d, tranLen = %d\n"), rxBytes, transferLen);
			readIrp = 0;
		}
	}

    return  readIrp;
}


void FT_DebugPrint(
	PCHAR title,
	PUCHAR buf,
	ULONG len,
    ULONG DevNum,
    PDEVICE_EXTENSION pDevExt
	)
{
#if DBG

	ULONG i;
	UCHAR c;
	UCHAR b[20];
	ULONG bi;
	
//if (len > 2) {
    KdPrint(("Dev %x - %s(%d):\n",pDevExt,title,len));

    i = 0;
	bi = 0;

	while (i < len) {
		c = *buf++;
		KdPrint(("%02x ",c));
		if ((c >= '0' && c <= '9')
		 || (c >= 'A' && c <= 'Z')
		 || (c >= 'a' && c <= 'z')
		 || (c >= 0x20 && c <= 0x41))
			b[bi] = c;
		else
			b[bi] = '.';
		++i;
		++bi;
		if ((i % 16) == 0) {
			b[bi] = '\0';
			KdPrint(("    %s\n",b));
			bi = 0;
		}
	}

	i %= 16;
	while (i < 16) {
		KdPrint(("   "));
		++i;
	}

	b[bi] = '\0';
	KdPrint(("    %s\n",b));
//}
#endif
}

ULONG
FT_GetBytesPerTransfer(
	PFTDI_DEVICE pDevExt,
    ULONG TransferLength
    )
{
    ULONG PacketsPerTransfer;

    PacketsPerTransfer = TransferLength / pDevExt->BulkIn.wMaxPacketSize;
    return TransferLength - (PacketsPerTransfer*2);
}


//
// Support for stopping and restarting task
//
#if 0
VOID
FT_InitializeBulkInStopCount(
    PDEVICE_EXTENSION   DeviceExtension
    )
{
    DeviceExtension->StopSignalled = FALSE;
	DeviceExtension->hRestartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if ( !DeviceExtension->hRestartEvent ) {
		DEBUGMSG2( ZONE_ERROR, TEXT("CreateEvent error:%d\n"), GetLastError() );
	}
}


VOID
FT_IncrementBulkInStopCount(
    PDEVICE_EXTENSION   DeviceExtension
    )
{
    DeviceExtension->StopSignalled = TRUE;
}


BOOL
FT_DecrementBulkInStopCount(
    PDEVICE_EXTENSION   DeviceExtension
    )
{
    BOOL rval = FALSE;

    if (DeviceExtension->StopSignalled
     && DeviceExtension->RestartExpected) {
        DeviceExtension->StopSignalled = FALSE;
		SetEvent(DeviceExtension->hRestartEvent);
        rval = TRUE;
    }

    return rval;
}
#endif

#ifndef WINCE

//
// Useful functions
//

VOID
FT_SetDeadmanTimeout(
    PDEVICE_EXTENSION   DeviceExtension
    )
{
    /*++

        Deadman timeout depends on usb transfer size (T),
        usb packet size (P), and latency timer (L) as follows

                        (T / (P-2)) * L

        Then add 5 seconds for luck! This ensures that
        we never setup a timeout less that 5 seconds
        which has been the default timeout till now.

    --*/

    DeviceExtension->DeadmanTimeout =
        (DeviceExtension->InParams.CurrentTransferSize /
            pDevExt->BulkIn.wMaxPacketSize-2) *
                DeviceExtension->DevContext.LatencyTime;

    DeviceExtension->DeadmanTimeout += FT_DEFAULT_DEADMAN_TIMEOUT;

    FT_KdPrint(
        DBGLVL_DEFAULT,
        ("FT_SetDeadmanTimeout: xfersize=%d lat=%d timeout=%d\n",
            DeviceExtension->InParams.CurrentTransferSize,
            DeviceExtension->DevContext.LatencyTime,
            DeviceExtension->DeadmanTimeout)
        );

}
#endif

NTSTATUS
InRequest(
	PVOID Buffer,
	ULONG Length,
	PULONG BytesRead,
    PDEVICE_EXTENSION pDevExt
	)
{
    NTSTATUS ntStatus;
	ULONG TransferBufferLength;
	DWORD dwUsbErr;

	DEBUGMSG2(ZONE_FUNCTION, TEXT("InRequest BulkIn.hPipe = 0x%X"),pDevExt->BulkIn.hPipe);

	ntStatus = IssueBulkTransfer(	
					pDevExt->UsbFuncs,
					pDevExt->BulkIn.hPipe,
					pDevExt->BulkIn.pCompletionRoutine,
					pDevExt->BulkIn.hEvent,
					pDevExt->BulkIn.dwBulkFlags,//(USB_IN_TRANSFER | USB_SHORT_TRANSFER_OK), // Flags
					Buffer, 
					0,
					Length,
					&TransferBufferLength,
					5000,
					&dwUsbErr
					);



//	if(Length > 2) {
//		DEBUGMSG3(ZONE_ERROR, TEXT("Bulk Read=%d, Length=%d\n"), TransferBufferLength, Length);
		DEBUGMSG3(ZONE_ERROR, TEXT("Bulk ntStatus=%d, Error=%d \n"), ntStatus, dwUsbErr);
//	}

	if(dwUsbErr == USB_STALL_ERROR) {

        //
        // Try to recover from STALL (USBD_HALTED)
        //

		DEBUGMSG1(ZONE_ERROR, (TEXT("nRequest RESETTING PIPE\n")));

		//
		// Do not set the ntStatus to anything else (will spoil backward compatibility
		//
        if(LResetPipe(pDevExt, &pDevExt->BulkIn) != ERROR_SUCCESS)
            ntStatus = STATUS_DEVICE_NOT_CONNECTED;

		//
		// Let the BULK_ERROR handle the other error codes
		//

//		if(Length > 2) {
//			DEBUGMSG3(ZONE_ERROR, TEXT("1Bulk Read=%d, Length=%d\n"), TransferBufferLength, Length);
//			DEBUGMSG3(ZONE_ERROR, TEXT("1Bulk ntStatus=%d, Error=%d \n"), ntStatus, dwUsbErr);
//		}
    }

	if (ntStatus == ERROR_SUCCESS) {
		*BytesRead = TransferBufferLength;
	}

    if (*BytesRead > 2) {
        DEBUGMSG3(ZONE_FUNCTION, TEXT("InRequest Requested:%d Read:%d\n"), Length, *BytesRead);
    }

	return ntStatus;
}

