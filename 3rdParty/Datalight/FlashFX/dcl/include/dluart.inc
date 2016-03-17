;----------------------------------------------------------------------------
;              ----> DO NOT REMOVE THE FOLLOWING NOTICE <----
;
;                 Copyright (c) 1993 - 2007 Datalight, Inc.
;                      All Rights Reserved Worldwide.
;
; Datalight, Incorporated is a Washington State corporation located at:
;
;       21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
;       Bothell, WA  98021                Fax:  425-951-8094
;       USA                               Web:  http://www.datalight.com
;
; This software, including without limitation all source code and documen-
; tation, is the confidential, trade secret property of Datalight, Inc.
; and is protected under the copyright laws of the United States and other
; jurisdictions.  Patents may be pending.
;
; In addition to civil penalties for infringement of copyright under appli-
; cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
; of (a) the restrictions on circumvention of copyright protection systems
; found in 17 U.S.C. 1201 and (b) the protections for the integrity of
; copyright management information found in 17 U.S.C. 1202.
;
; U.S. Government Restricted Rights:  Use, duplication, reproduction, or
; transfer of this commercial product and accompanying documentation is
; restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
; License Agreement.
;
; IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
; A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
; CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
; IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
; OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
; BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
; PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.
;
; IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
; CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
; SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                               Description
;
;       This header contains information for dealing with UARTs.
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                               Revision History
; $Log: dluart.inc $
; Revision 1.2  2007/11/03 23:31:22Z  Garyp
; Added the standard module header.
; Revision 1.1  2005/01/17 02:15:00Z  Pauli
; Initial revision
; Revision 1.1  2005/01/17 02:15:00Z  Garyp
; Initial revision
; Revision 1.1  2005/01/17 01:15:00Z  pauli
; Initial revision
; Revision 1.1  2005/01/17 00:15:00Z  GaryP
; Initial revision
; Revision 1.4  2004/12/30 23:53:55Z  GaryP
; Updated to use indents of 4, hard tabs of 8 (if used), and a standard
; comment style.
; Revision 1.3  2004/02/01 01:30:42Z  garys
; Merge from FlashFXMT
; Revision 1.2.1.2  2004/02/01 01:30:42  garyp
; Minor cleanup to standardize the names.
; Revision 1.2  2003/06/12 07:12:28Z  garyp
; Added the proper header.
; 02/07/02 DE  Updated copyright notice for 2002.
; 01/03/01 gp  Copied from the WinLight project
;----------------------------------------------------------------------------

; Uart port offsets (8250/16450/16550)
UART_DATAREG            equ <0>         ; receive and transmit register  (!DLAB)
UART_INTMASK            equ <1>         ; interrupt enable mask register (!DLAB)
UART_INTID              equ <2>         ; interrupt ID register
UART_DATAFORMAT         equ <3>         ; data format register
UART_MODEMCONTROL       equ <4>         ; modem control register
UART_LINESTATUS         equ <5>         ; line status register
UART_MODEMSTATUS        equ <6>         ; modem status register
UART_SCRATCH            equ <7>         ; scratch pad register
UART_LSBDIVISOR         equ <0>         ; divisor latch register (LSB) (DLAB)
UART_MSBDIVISOR         equ <1>         ; divisor latch register (MSB) (DLAB)
UART_FIFOCONTROL        equ <2>         ; FIFO control register (16550)

; 16550 FIFO Control Register bit masks
FCR_ENABLEBOTH          equ <00000001b> ; enables Tx and Rx FIFO's
FCR_RXFIFORESET         equ <00000010b> ; clears the Rx FIFO
FCR_TXFIFORESET         equ <00000100b> ; clears the Tx FIFO
FCR_DMAMODESELECT       equ <00001000b> ; selectes DMA xfer mode
FCR_RESERVEDBITS        equ <00110000b> ; reserved
FCR_RXTHRESHOLD         equ <11000000b> ; int thresholds[] = {1,4,8,14}

; Data Format Register Flags
DFMT_DATABITS_5         equ <00000000b> ; 5 data bits
DFMT_DATABITS_6         equ <00000001b> ; 6 data bits
DFMT_DATABITS_7         equ <00000010b> ; 7 data bits
DFMT_DATABITS_8         equ <00000011b> ; 8 data bits
DFMT_STOPBITS_1         equ <00000000b> ; 1 stop bit
DFMT_STOPBITS_2         equ <00000100b> ; 2 stop bits
DFMT_PARITY_NONE        equ <00000000b> ; no parity
DFMT_PARITY_ODD         equ <00001000b> ; odd parity
DFMT_PARITY_EVEN        equ <00011000b> ; even parity
DFMT_PARITY_MARK        equ <00101000b> ; always on parity
DFMT_PARITY_SPACE       equ <00111000b> ; always off parity
DFMT_BREAK              equ <01000000b> ; break on
DFMT_DLAB               equ <10000000b> ; divisor latch access flag
DFMT_DLAB_MASK          equ <01111111b> ; MASK DLAB flag

; Interrupt ID values
INTID_PENDING           equ <00000001b> ; B0==0 => pending interrupt
INTID_CHANGESIGNAL      equ <00000000b> ; change of RS232 signal
INTID_CHANGETXDONE      equ <00000010b> ; transmit buffer/FIFO empty
INTID_CHANGERXDATA      equ <00000100b> ; received data (FIFO trigger)
INTID_CHANGEBREAK       equ <00000110b> ; serialization err (line status)
INTID_FIFOTIMEOUT       equ <00001100b> ; Rx FIFO timed out (16550)
INTID_FIFOSENABLED      equ <11000000b> ; both FIFOs enabled (16550)

; Line Status Register values
LSR_RXRD                equ <00000001b> ; receive data ready
LSR_OVERRUN             equ <00000010b> ; overrun error
LSR_PARITY              equ <00000100b> ; parity error
LSR_FRAME               equ <00001000b> ; framing error
LSR_BREAK               equ <00010000b> ; break
LSR_TBE                 equ <00100000b> ; transmit buffer empty
LSR_TXE                 equ <01000000b> ; transmit empty
LSR_FIFODATABAD         equ <10000000b> ; FIFO has bad data (16550)

; Modem Status Register bit masks
MSR_DELTACTS            equ <00000001b> ; change in Clear to Send
MSR_DELTADSR            equ <00000010b> ; change in Data Set Ready
MSR_DELTARI             equ <00000100b> ; change in Ring indicator
MSR_DELTADCD            equ <00001000b> ; change in Data carrier detect
MSR_CTS                 equ <00010000b> ; Clear to Send
MSR_DSR                 equ <00100000b> ; Data Set Ready
MSR_RI                  equ <01000000b> ; Ring indicator
MSR_DCD                 equ <10000000b> ; Data carrier detect

; Baudrate Divisors
BAUD_300                equ <384>       ; 115200/384 == 300
BAUD_1200               equ <96>        ; 115200/96 == 1200
BAUD_2400               equ <48>        ; etc...
BAUD_4800               equ <24>
BAUD_9600               equ <12>
BAUD_19200              equ <6>
BAUD_38400              equ <3>
BAUD_57600              equ <2>
BAUD_115200             equ <1>

; Interrupt Mask Register Flags
IMR_RXD                 equ <00000001b> ; receive data
IMR_TBE                 equ <00000010b> ; transmit buffer empty
IMR_ERRBRK              equ <00000100b> ; errors or break
IMR_STATE               equ <00001000b> ; RS-232 state change

; Modem Control Register Flags
MCR_DTR                 equ <00000001b> ; data terminal ready
MCR_RTS                 equ <00000010b> ; request to send
MCR_OUT1                equ <00000100b> ; general output 1
MCR_OUT2                equ <00001000b> ; general output 2 (enable ints)
MCR_LOOP                equ <00010000b> ; loopback check
