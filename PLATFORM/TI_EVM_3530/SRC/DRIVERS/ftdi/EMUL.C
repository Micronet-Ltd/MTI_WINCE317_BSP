/*++

Copyright (c) 2002-2003  Future Technology Devices International Ltd.

Module Name:

    emul.c

Abstract:

    VCP device driver for FTDI FT232BM
    Special emulation mode.

Environment:

    kernel & user mode

Revision History:

    23/12/02    awm     Created.
    07/01/03    awm     Modified to support extended modes.
    10/01/03    awm     Fixed problem where XON/XOFF always substituted.
    20/01/03    awm     Support disabling substitution by flow control.
    03/04/03    awm     Support clear MSR if break detected.
    25/04/03    awm     Maintain LSR to support break detection.

--*/



#ifdef WINCE
#include <windows.h>

#include <serhw.h>
#include "ftdi_ioctl.h"
#include "BUSBDBG.H"
#include "ftdi_debug.h"
//#include <serdbg.h>
//#include <celog.h> 
//#include <pkfuncs.h>
#else
//#include "precomp.h"
#endif
#include "emul.h"


//
// RX State machine
//

enum {
    FT_EMUL_RX_STATE_DEFAULT,
    FT_EMUL_RX_STATE_GETCHAR
};


//
// TX State machine
//

enum {
    FT_EMUL_TX_STATE_DEFAULT,
    FT_EMUL_TX_STATE_XON,
    FT_EMUL_TX_STATE_XOFF,
    FT_EMUL_TX_STATE_ESC
};


void FT_DebugPrint(
	PCHAR title,
	PUCHAR buf,
	ULONG len,
	ULONG DevNum
    );


BOOL
FT_InitEmulMode(
    PFTEMUL Emul,
    ULONG   SubMode,
	ULONG	BufferSize
    )
/*++

Routine Description:

    Initialize emulation mode variables.

Arguments:

    Emul    - Pointer to the emulation mode vars.
    SubMode - Substitution mode.

Return Value:

    None.

--*/
{
#ifndef WINCE
    SerialDump(SERTRACECALLS,("FT_InitEmulMode:: %x\n",SubMode));
#endif

	Emul->pWriteBuffer = (UCHAR *)FT_LocalAlloc(0, BufferSize);
	if(Emul->pWriteBuffer == NULL) {
		return FALSE;
	}

    Emul->SubMode = SubMode;

    if (SubMode & FT_EMUL_DISABLE_SUB_IF_FC_OFF)
        Emul->SubMode |= FT_EMUL_FC_DISABLED;
    else
        Emul->SubMode &= ~FT_EMUL_FC_DISABLED;

    Emul->RxState = FT_EMUL_RX_STATE_DEFAULT;
    Emul->TxState = FT_EMUL_TX_STATE_DEFAULT;

    Emul->ModemCtrl = 0;

    Emul->MSR = 0;

    Emul->MSRMask = 0xff;

    Emul->LSR = 0;

    if (SubMode & FT_EMUL_SUB_DSR)
        Emul->MSRMask ^= SERIAL_MSR_DSR;
    if (SubMode & FT_EMUL_SUB_DCD)
        Emul->MSRMask ^= SERIAL_MSR_DCD;
    if (SubMode & FT_EMUL_SUB_CTS)
        Emul->MSRMask ^= SERIAL_MSR_CTS;
    if (SubMode & FT_EMUL_SUB_RI)
        Emul->MSRMask ^= SERIAL_MSR_RI;

    if (SubMode) {
        Emul->Esc = FT_EMUL_DEF_ESC;
        Emul->Xoff = FT_EMUL_DEF_XOFF;
        Emul->Xon = FT_EMUL_DEF_XON;
    }

#ifndef WINCE
    SerialDump(SERTRACECALLS,("FT_InitEmulMode-- %x\n",Emul->SubMode));
#endif
	return TRUE;
}


VOID
FT_InitEmulChars(
    PFTEMUL Emul,
    UCHAR Esc,
    UCHAR Xoff,
    UCHAR Xon
    )
/*++

Routine Description:

    Initialize emulation mode characters.

Arguments:

    Emul    - Pointer to the emulation mode vars.
    Esc     - Escape character.
    Xoff    - XOFF character.
    Xon     - XON character.

Return Value:

    None.

--*/
{
    Emul->Esc = Esc;
    Emul->Xoff = Xoff;
    Emul->Xon = Xon;
}


ULONG
FT_EmulConvertRxChar(
    PFTEMUL Emul,
    PUCHAR  CurChar
    )
/*++

Routine Description:

    State machine to process receive characters.

Arguments:

    Emul    - Pointer to the emulation mode vars.
    CurChar - Pointer to current character.

Return Value:

    None.

--*/
{
    ULONG   rval;
    UCHAR   c = *CurChar;

    switch (Emul->RxState) {

    case FT_EMUL_RX_STATE_DEFAULT :

        //
        // IDLE state
        //
        // If current character is ESC character goto GETCHAR state.
        //

        if (c == Emul->Esc
         && !FT_EMUL_DISABLE_SUB_BY_FLOW(Emul)) {
            Emul->RxState = FT_EMUL_RX_STATE_GETCHAR;
            rval = FT_EMUL_RET_0;
        }
        else
            rval = FT_EMUL_RET_1;

        break;

    case FT_EMUL_RX_STATE_GETCHAR :

        //
        // GETCHAR state
        //
        // Process a character. Return a character or a status.
        // Goto IDLE state.
        //
        
        Emul->RxState = FT_EMUL_RX_STATE_DEFAULT;

        //
        // If substitute XON/XOFF mode
        //      If ESC 0x01, substitute XON.
        //      If ESC 0x02, substitute XOFF.
        //

        if (FT_EMUL_SUB_XONXOFF_ENABLED(Emul)) {
            if (c == FT_EMUL_ESC_XON) {
                *CurChar = Emul->Xon;
                rval = FT_EMUL_RET_ESC_XON;
                break;
            }
            else if (c == FT_EMUL_ESC_XOFF) {
                *CurChar = Emul->Xoff;
                rval = FT_EMUL_RET_ESC_XOFF;
                break;
            }
        }

        //
        // If substitute DSR mode
        //      If ESC 0x07, status DSR active.
        //      If ESC 0x08, status DSR inactive.
        //

        if (FT_EMUL_SUB_DSR_ENABLED(Emul)) {
            if (c == FT_EMUL_ESC_SETDSR) {
                rval = FT_EMUL_RET_DSR_ACTIVE;
                break;
            }
            else if (c == FT_EMUL_ESC_CLRDSR) {
                rval = FT_EMUL_RET_DSR_INACTIVE;
                break;
            }
        }

        //
        // If substitute DCD mode
        //      If ESC 0x09, status DCD active.
        //      If ESC 0x0a, status DCD inactive.
        //

        if (FT_EMUL_SUB_DCD_ENABLED(Emul)) {
            if (c == FT_EMUL_ESC_SETDCD) {
                rval = FT_EMUL_RET_DCD_ACTIVE;
                break;
            }
            else if (c == FT_EMUL_ESC_CLRDCD) {
                rval = FT_EMUL_RET_DCD_INACTIVE;
                break;
            }
        }

        //
        // If substitute RI mode
        //      If ESC 0x0b, status RI active.
        //      If ESC 0x0c, status RI inactive.
        //

        if (FT_EMUL_SUB_RI_ENABLED(Emul)) {
            if (c == FT_EMUL_ESC_SETRI) {
                rval = FT_EMUL_RET_RI_ACTIVE;
                break;
            }
            else if (c == FT_EMUL_ESC_CLRRI) {
                rval = FT_EMUL_RET_RI_INACTIVE;
                break;
            }
        }

        //
        // If substitute CTS mode
        //      If ESC 0x0d, status CTS active.
        //      If ESC 0x0e, status CTS inactive.
        //

        if (FT_EMUL_SUB_CTS_ENABLED(Emul)) {
            if (c == FT_EMUL_ESC_SETCTS) {
                rval = FT_EMUL_RET_CTS_ACTIVE;
                break;
            }
            else if (c == FT_EMUL_ESC_CLRCTS) {
                rval = FT_EMUL_RET_CTS_INACTIVE;
                break;
            }
        }

        //
        // If any substitution mode
        //      If ESC ESC, substitute ESC.
        //      If ESC Any, pass both characters.
        //

        if (c == Emul->Esc)
            rval = FT_EMUL_RET_ESC_ESC;
        else
            rval = FT_EMUL_RET_ESC_ANY;

        break;

    }

    return rval;

}


VOID
FT_EmulProcessMSR(
    PSERIAL_DEVICE_EXTENSION    Extension,
    UCHAR                       StatusByte
	)
{
    UCHAR ModemStatus;

	//
    // StatusByte contains Modem Status Register
	//

    StatusByte &= 0xf0;  // we are only interested in the status bits

    //
    // Build MSR
    // Bits 7-4 are returned in StatusByte, Bits 3-0 are the change bits.
    //

    ModemStatus = StatusByte;
    ModemStatus = (ModemStatus ^ Extension->SerialRegs.MSR) & 0xf0;
    ModemStatus >>= 4;
    ModemStatus |= StatusByte;

    if (ModemStatus != Extension->SerialRegs.MSR) {
        UCHAR DcdStatus;

        Extension->SerialRegs.MSR = ModemStatus;

        //
        // Check for disconnect (drop DCD). If we detect this condition
        // and there is data to process, we must defer handling the change
        // in DCD until the data has been processed.
        //
        DcdStatus = (ModemStatus & (SERIAL_MSR_DDCD|SERIAL_MSR_DCD));

        if (DcdStatus == SERIAL_MSR_DDCD) {
#ifndef WINCE
            SerialDump(SERDIAG1,("FT_EmulProcessMSR disconnect detected\n"));
#endif
            Extension->DeferModemUpdate = TRUE;
        }
        else {
#ifdef WINCE
//			pSerInfo->cIntStat |= MODEM_INT_MASK;
//			SetEvent(pSerInfo->hSerialEvent);
#else
            SerialHandleModemUpdate(Extension,FALSE);
#endif        
		}
    }

}




VOID
FT_EmulProcessLSR(
    PSERIAL_DEVICE_EXTENSION    Extension,
    UCHAR                       StatusByte
	)
{
	//
    // StatusByte contains Line Status Register
	//

    if (StatusByte != Extension->SerialRegs.LSR) {
        Extension->SerialRegs.LSR = StatusByte;
#ifndef WINCE
        SerialProcessLSR(Extension);
#endif
    }

    if ((Extension->SerialRegs.LSR & 0x60) == 0x60) {
        Extension->HoldingEmpty = TRUE;
#ifndef WINCE
        if (Extension->WriteLength) {
            Extension->EmptiedTransmit = TRUE;
        }
#endif
    }

}




VOID
FT_EmulProcessRxPacket(
    PSER_INFO pSerInfo,
    PUCHAR                      Buffer,
    ULONG                       BufSiz
	)
{
#define DATA_OFFSET 2
    PSERIAL_DEVICE_EXTENSION Extension = pSerInfo->pFTDICtxt;
    ULONG i;
    ULONG ConvChar;
    UCHAR ReceivedChar;
    UCHAR ModemStatus;
    BOOLEAN GotEventChar = FALSE;

	ASSERT(BufSiz >= DATA_OFFSET);

#if 0
    SerialDump(SERTRACECALLS,("Enter FT_EmulProcessRxPacket()\n"));
#endif


#ifndef WINCE
    KeAcquireSpinLock(&Extension->DpcLock,&OldIrql);
#endif

    //
    // If we have to clear MSR when break detected, check LSR first.
    //

    if (SerialClearMsrIfBreak(Extension)) {

        PFTEMUL Emul = &Extension->EmulVars;

        if (Buffer[1] & SERIAL_LSR_BI) {

#ifndef WINCE
			ULONG Mask = SERIAL_PURGE_TXABORT | SERIAL_PURGE_RXABORT | SERIAL_PURGE_RXCLEAR;
#endif

            if (!FT_EMUL_BREAK_ACTIVE(Emul)) {

                Emul->LSR = Buffer[1];

                if (FT_EMUL_SUB_DSR_ENABLED(Emul)) {
                    Emul->MSR &= ~SERIAL_MSR_DSR;
                }

                if (FT_EMUL_SUB_CTS_ENABLED(Emul)) {
                    Emul->MSR &= ~SERIAL_MSR_CTS;
                }

                if (FT_EMUL_SUB_DCD_ENABLED(Emul)) {
                    Emul->MSR &= ~SERIAL_MSR_DCD;
                }

                if (FT_EMUL_SUB_RI_ENABLED(Emul)) {
                    Emul->MSR &= ~SERIAL_MSR_RI;
                }

#ifndef WINCE
                KeReleaseSpinLock(&Extension->DpcLock,OldIrql);
                FT_SerialPurge(Extension,Mask);
                KeAcquireSpinLock(&Extension->DpcLock,&OldIrql);
#endif

            }

        }

        Emul->LSR = Buffer[1];

    }

	//
    // Buffer[0] contains Modem Status Register
	//

    ModemStatus = Buffer[0];

    if (FT_EMUL_SUBSTITUTION_MODE(Extension)) {
        ModemStatus &= Extension->EmulVars.MSRMask;
        ModemStatus |= Extension->EmulVars.MSR;
    }

    FT_EmulProcessMSR(Extension,ModemStatus);

	//
    // Buffer[1] contains Line Status Register
	//

    FT_EmulProcessLSR(Extension,Buffer[1]);

	if (BufSiz <= DATA_OFFSET) {

		//
		// this was just a status packet
		//

        if (Extension->DeferModemUpdate) {
#ifdef WINCE
			pSerInfo->cIntStat |= MODEM_INT_MASK;
			SetEvent(pSerInfo->hSerialEvent);
#else
            SerialHandleModemUpdate(Extension,FALSE);
#endif            
			Extension->DeferModemUpdate = FALSE;
        }

#ifndef WINCE
        KeReleaseSpinLock(&Extension->DpcLock,OldIrql);
#endif
		return;
	}

//    FT_DebugPrint("IN",Buffer,BufSiz,0);

	//
	// copy data to ISR buffer
	//

	i = DATA_OFFSET;

    do {

        ReceivedChar = Buffer[i++];

#ifndef WINCE
        Extension->PerfStats.ReceivedCount++;
        Extension->WmiPerfData.ReceivedCount++;
#endif

        ConvChar = FT_EmulConvertRxChar(&Extension->EmulVars,&ReceivedChar);

        if (FT_EMUL_SUBSTITUTED_CHAR(ConvChar)) {

#ifndef WINCE
SerialDump(SERDIAG1,("FT_EmulProcessRxPacket ConvChar:%x ReceivedChar:%x\n",ConvChar,ReceivedChar));
#endif

            switch (ConvChar) {

            case FT_EMUL_RET_0 :
                break;

            case FT_EMUL_RET_ESC_ANY :

                //
                // ESC + Any :- save both characters
                //

                // Check to see if we should note
                // the receive character or special
                // character event.
                //

#ifndef WINCE
                if (Extension->IsrWaitMask) {

                    if (Extension->IsrWaitMask & SERIAL_EV_RXCHAR) {
                        Extension->HistoryMask |= SERIAL_EV_RXCHAR;
                        GotEventChar = TRUE;
                    }

                    if ((Extension->IsrWaitMask & SERIAL_EV_RXFLAG) &&
                        (Extension->SpecialChars.EventChar == Extension->EmulVars.Esc)) {

                        Extension->HistoryMask |= SERIAL_EV_RXFLAG;

                        //
                        // we trigger on an event character when
                        // a). it is on its own in position 3
                        // b). it is in position 4 or above
                        //

                        if ((i == (DATA_OFFSET+1)) && (i == BufSiz)) {
                            GotEventChar = TRUE;   // 0x7e on its own in position 3
                        }
                        else if (i > (DATA_OFFSET+1)) {
                            GotEventChar = TRUE;   // 0x7e in position 4 or above
                        }

                    }

                }
#endif
                SerialPutChar(Extension,Extension->EmulVars.Esc);
#ifdef WINCE
				pSerInfo->cIntStat |= RX_INT_MASK;
				SetEvent(pSerInfo->hSerialEvent);
#endif

                //
                // FALL THROUGH TO ...
                //

            case FT_EMUL_RET_ESC_XOFF :
            case FT_EMUL_RET_ESC_XON :
            case FT_EMUL_RET_ESC_ESC :

                //
                // Check to see if we should note
                // the receive character or special
                // character event.
                //

#ifndef WINCE
                if (Extension->IsrWaitMask) {

                    if (Extension->IsrWaitMask & SERIAL_EV_RXCHAR) {
                        Extension->HistoryMask |= SERIAL_EV_RXCHAR;
                        GotEventChar = TRUE;
                    }

                    if ((Extension->IsrWaitMask & SERIAL_EV_RXFLAG) &&
                        (Extension->SpecialChars.EventChar == ReceivedChar)) {

                        Extension->HistoryMask |= SERIAL_EV_RXFLAG;

                        //
                        // we trigger on an event character when
                        // a). it is on its own in position 3
                        // b). it is in position 4 or above
                        //

                        if ((i == (DATA_OFFSET+1)) && (i == BufSiz)) {
                            GotEventChar = TRUE;   // 0x7e on its own in position 3
                        }
                        else if (i > (DATA_OFFSET+1)) {
                            GotEventChar = TRUE;   // 0x7e in position 4 or above
                        }

                    }

                }
#endif

                SerialPutChar(Extension,ReceivedChar);
#ifdef WINCE
				pSerInfo->cIntStat |= RX_INT_MASK;
				SetEvent(pSerInfo->hSerialEvent);
#endif

                break;

            case FT_EMUL_RET_DSR_ACTIVE :
                ModemStatus = Extension->SerialRegs.MSR;
                ModemStatus |= SERIAL_MSR_DSR;
                FT_EmulProcessMSR(Extension,ModemStatus);
                Extension->EmulVars.MSR |= SERIAL_MSR_DSR;
                break;

            case FT_EMUL_RET_DSR_INACTIVE :
                ModemStatus = Extension->SerialRegs.MSR;
                ModemStatus &= ~SERIAL_MSR_DSR;
                FT_EmulProcessMSR(Extension,ModemStatus);
                Extension->EmulVars.MSR &= ~SERIAL_MSR_DSR;
                break;

            case FT_EMUL_RET_DCD_ACTIVE :
                ModemStatus = Extension->SerialRegs.MSR;
                ModemStatus |= SERIAL_MSR_DCD;
                FT_EmulProcessMSR(Extension,ModemStatus);
                Extension->EmulVars.MSR |= SERIAL_MSR_DCD;
                break;

            case FT_EMUL_RET_DCD_INACTIVE :
                ModemStatus = Extension->SerialRegs.MSR;
                ModemStatus &= ~SERIAL_MSR_DCD;
                FT_EmulProcessMSR(Extension,ModemStatus);
                Extension->EmulVars.MSR &= ~SERIAL_MSR_DCD;
                break;

            case FT_EMUL_RET_RI_ACTIVE :
                ModemStatus = Extension->SerialRegs.MSR;
                ModemStatus |= SERIAL_MSR_RI;
                FT_EmulProcessMSR(Extension,ModemStatus);
                Extension->EmulVars.MSR |= SERIAL_MSR_RI;
                break;

            case FT_EMUL_RET_RI_INACTIVE :
                ModemStatus = Extension->SerialRegs.MSR;
                ModemStatus &= ~SERIAL_MSR_RI;
                FT_EmulProcessMSR(Extension,ModemStatus);
                Extension->EmulVars.MSR &= ~SERIAL_MSR_RI;
                break;

            case FT_EMUL_RET_CTS_ACTIVE :
                ModemStatus = Extension->SerialRegs.MSR;
                ModemStatus |= SERIAL_MSR_CTS;
                FT_EmulProcessMSR(Extension,ModemStatus);
                Extension->EmulVars.MSR |= SERIAL_MSR_CTS;
                break;

            case FT_EMUL_RET_CTS_INACTIVE :
                ModemStatus = Extension->SerialRegs.MSR;
                ModemStatus &= ~SERIAL_MSR_CTS;
                FT_EmulProcessMSR(Extension,ModemStatus);
                Extension->EmulVars.MSR &= ~SERIAL_MSR_CTS;
                break;

            default :
                break;

            }

        }
        else {

			//
			// here if we are doing as normal for all other data values
			//

#ifndef WINCE
            ReceivedChar &= Extension->ValidDataMask;

            if (!ReceivedChar &&
                (Extension->HandFlow.FlowReplace & SERIAL_NULL_STRIPPING)) {

                //
                // If what we got is a null character
                // and we're doing null stripping, then
                // we simply act as if we didn't see it.
                //

                goto ReceiveDoLineStatus;

            }

#endif
#ifndef WINCE
            if (
				(Extension->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT) &&
                ((ReceivedChar == Extension->SpecialChars.XonChar) ||
                 (ReceivedChar == Extension->SpecialChars.XoffChar))) {

                //
                // No matter what happens this character
                // will never get seen by the app.
                //

                if (ReceivedChar == Extension->SpecialChars.XoffChar) {

#ifndef WINCE
                    Extension->TXHolding |= SERIAL_TX_XOFF;
#endif

#ifndef WINCE
                    if ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
                         SERIAL_TRANSMIT_TOGGLE) {

                        SerialInsertQueueDpc(
                            &Extension->StartTimerLowerRTSDpc,
                            NULL,
                            NULL,
                            Extension
                            )?Extension->CountOfTryingToLowerRTS++:0;

                    }
#endif

                }
                else {

#ifndef WINCE
                    if (Extension->TXHolding & SERIAL_TX_XOFF) {

                        //
                        // We got the xon char **AND*** we
                        // were being held up on transmission
                        // by xoff.  Clear that we are holding
                        // due to xoff.  Transmission will
                        // automatically restart because of
                        // the code outside the main loop that
                        // catches problems chips like the
                        // SMC and the Winbond.
                        //

                        Extension->TXHolding &= ~SERIAL_TX_XOFF;

                    }
#endif

                }

                goto ReceiveDoLineStatus;

            }
#endif

            //
            // Check to see if we should note
            // the receive character or special
            // character event.
            //

#ifndef WINCE
            if (Extension->IsrWaitMask) {

                if (Extension->IsrWaitMask & SERIAL_EV_RXCHAR) {
                    Extension->HistoryMask |= SERIAL_EV_RXCHAR;
                    GotEventChar = TRUE;
                }

                if ((Extension->IsrWaitMask & SERIAL_EV_RXFLAG) &&
                    (Extension->SpecialChars.EventChar == ReceivedChar)) {

                    Extension->HistoryMask |= SERIAL_EV_RXFLAG;

                    //
                    // we trigger on an event character when
                    // a). it is on its own in position 3
                    // b). it is in position 4 or above
                    //

                    if ((i == (DATA_OFFSET+1)) && (i == BufSiz)) {
                        GotEventChar = TRUE;   // 0x7e on its own in position 3
                    }
                    else if (i > (DATA_OFFSET+1)) {
                        GotEventChar = TRUE;   // 0x7e in position 4 or above
                    }

                }

            }
#endif

            SerialPutChar(Extension,ReceivedChar);
#ifdef WINCE
			pSerInfo->cIntStat |= RX_INT_MASK;
			SetEvent(pSerInfo->hSerialEvent);
#endif

            //
            // If we're doing line status and modem
            // status insertion then we need to insert
            // a zero following the character we just
            // placed into the buffer to mark that this
            // was reception of what we are using to
            // escape.
            //

#ifndef WINCE		// This is OK as CE does not support the IOCTL_SERIAL_LSRMST_INSERT
            if (Extension->EscapeChar
             && (Extension->EscapeChar == ReceivedChar)) {
                SerialPutChar(Extension,SERIAL_LSRMST_ESCAPE);
            }
#endif

        } /* if ((ConvertedChar & 0xff00) != 0) */


//ReceiveDoLineStatus:    ;

		if (i == BufSiz) {

            //
            // No more characters, get out of the
            // loop.
            //

            break;

		}

    } while (TRUE);


#ifndef WINCE
    if (GotEventChar) {

        if (Extension->IrpMaskLocation &&
            Extension->HistoryMask) {

            *Extension->IrpMaskLocation = Extension->HistoryMask;
            Extension->IrpMaskLocation = NULL;
            Extension->HistoryMask = 0;

            Extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
            SerialInsertQueueDpc(
                &Extension->CommWaitDpc,
                NULL,
                NULL,
                Extension
                );

        }

    }
#endif

#ifndef WINCE
    if (Extension->DeferModemUpdate) {
        SerialHandleModemUpdate(Extension,FALSE);
        Extension->DeferModemUpdate = FALSE;
    }
#endif

#ifndef WINCE
    KeReleaseSpinLock(&Extension->DpcLock,OldIrql);
#endif

#if 0
    SerialDump(SERTRACECALLS,("Exit FT_EmulProcessRxPacket()\n"));
#endif

}


BOOLEAN
FT_EmulGetNextTxByte(PUCHAR Dest,PUCHAR Srce,PFTEMUL Emul)
/*++

Routine Description:

    State machine to process transmit characters.
    Most chars will pass through, but special chars
    will be converted to ESC char pairs.

Arguments:

    Emul    - Pointer to the emulation mode vars.
    Dest    - Pointer to storage for returned char.
    Srce    - Pointer to source char.

Return Value:

    BOOLEAN - TRUE is source char has been completely processed
              FALSE otherwise.

--*/
{
    BOOLEAN rval;
    UCHAR   rchar;
    UCHAR   c;

    switch (Emul->TxState) {

    default :

        c = *Srce;

        if (c == Emul->Xon
         && FT_EMUL_SUB_XONXOFF_ENABLED(Emul)) {
            rchar = Emul->Esc;
            Emul->TxState = FT_EMUL_TX_STATE_XON;
            rval = FALSE;
        }
        else if (c == Emul->Xoff
         && FT_EMUL_SUB_XONXOFF_ENABLED(Emul)) {
            rchar = Emul->Esc;
            Emul->TxState = FT_EMUL_TX_STATE_XOFF;
            rval = FALSE;
        }
        else if (c == Emul->Esc
         && !FT_EMUL_DISABLE_SUB_BY_FLOW(Emul)) {
            rchar = Emul->Esc;
            Emul->TxState = FT_EMUL_TX_STATE_ESC;
            rval = FALSE;
        }
        else {
            rchar = c;
            rval = TRUE;
        }

        break;

    case FT_EMUL_TX_STATE_XON :

        rchar = FT_EMUL_ESC_XON;
        Emul->TxState = FT_EMUL_TX_STATE_DEFAULT;
        rval = TRUE;
        break;

    case FT_EMUL_TX_STATE_XOFF :

        rchar = FT_EMUL_ESC_XOFF;
        Emul->TxState = FT_EMUL_TX_STATE_DEFAULT;
        rval = TRUE;
        break;

    case FT_EMUL_TX_STATE_ESC :

        rchar = Emul->Esc;
        Emul->TxState = FT_EMUL_TX_STATE_DEFAULT;
        rval = TRUE;
        break;

    }

    *Dest = rchar;

    return rval;

}


ULONG
FT_EmulCopyTxBytes(
    PUCHAR                      Dest,
    PUCHAR                      Srce,
    PULONG                      MaxChars,
    ULONG                       MaxTransferLen,
    PSERIAL_DEVICE_EXTENSION    DeviceExtension
    )
/*++

Routine Description:

    Control function to copy chars from IRP buffer to USB transfer buffer.
    The number of chars copied must be less than or equal to MaxTransferLen,
    and the function also keeps track of the number of IRP bytes processed.

Arguments:

    Dest            - Pointer to storage for returned chars.
    Srce            - Pointer to source buffer.
    MaxChars        - Pointer to maximum number of chars in source buffer.
    MaxTransferLen  - Maximum size of the USB request.
    DeviceExtension - Pointer to device extension.

Return Value:

    ULONG           - Number of bytes copied into the destination buffer.
                      MaxChars contains number of source chars actually
                      processed.

--*/
{
    ULONG   numTransferred = 0;
    ULONG   numProcessed = 0;
    ULONG   i;
    BOOLEAN rval;
    UCHAR   c;

#ifndef WINCE
    SerialDump(SERTRACECALLS,("FT_EmulCopyTxBytes::\n"));
    SerialDump(SERDIAG1,("Chars in request buffer: %d\n",*MaxChars));
#endif

    for (i = 0; i < *MaxChars; i++) {

        do {

            rval = FT_EmulGetNextTxByte(&c,Srce,&DeviceExtension->EmulVars);

            *Dest++ = c;
            numTransferred++;

            if (rval) {
                Srce++;
                numProcessed++;
            }

        }
//        while (!rval && numTransferred < MaxTransferLen);
        while (!rval);

//        if (numTransferred == MaxTransferLen)
//           break;

    }

    *MaxChars = numProcessed;

#ifndef WINCE
    SerialDump(SERDIAG1,("Chars processed: %d\n",*MaxChars));
    SerialDump(SERDIAG1,("Chars to transfer: %d\n",numTransferred));
    SerialDump(SERTRACECALLS,("FT_EmulCopyTxBytes--\n"));
#endif

    return numTransferred;
}


ULONG
FT_EmulBuildModemCtrlRequest(
    PUCHAR                      Buffer,
    ULONG                       BufferLen,
    ULONG                       RequestType,
    PSERIAL_DEVICE_EXTENSION    DeviceExtension
    )
/*++

Routine Description:

    Control function to setup USB transfer buffer for modem control requests.

Arguments:

    Buffer          - Pointer to storage for returned chars.
    BufferLen       - Number of chars in buffer.
    RequestType     - Type of request (SetDtr, ClrDtr, SetRts, ClrRts).
    DeviceExtension - Pointer to device extension.

Return Value:

    ULONG           - Number of bytes copied into the destination buffer.

--*/
{
#ifndef WINCE
    UCHAR Buf[FT_EMUL_MODEM_CTRL_REQUEST_LEN];
#endif
    ULONG rval = FT_EMUL_MODEM_CTRL_REQUEST_LEN;

#ifndef WINCE
    SerialDump(SERTRACECALLS,("FT_EmulBuildModemCtrlRequest::\n"));
#endif

    //
    // Check that there is enough room in buffer to store the request
    //

    if (BufferLen < FT_EMUL_MODEM_CTRL_REQUEST_LEN)
        return 0;

    *Buffer++ = DeviceExtension->EmulVars.Esc;

    switch (RequestType) {

    case FT_EMUL_BUILD_SET_DTR_REQUEST :
        *Buffer = FT_EMUL_ESC_SETDTR;
        break;

    case FT_EMUL_BUILD_CLR_DTR_REQUEST :
        *Buffer = FT_EMUL_ESC_CLRDTR;
        break;

    case FT_EMUL_BUILD_SET_RTS_REQUEST :
        *Buffer = FT_EMUL_ESC_SETRTS;
        break;

    case FT_EMUL_BUILD_CLR_RTS_REQUEST :
        *Buffer = FT_EMUL_ESC_CLRRTS;
        break;

    default :
        rval = 0;
        break;

    }

#ifndef WINCE
    SerialDump(SERTRACECALLS,("FT_EmulBuildModemCtrlRequest-- %d\n",rval));
#endif

    return rval;
}

