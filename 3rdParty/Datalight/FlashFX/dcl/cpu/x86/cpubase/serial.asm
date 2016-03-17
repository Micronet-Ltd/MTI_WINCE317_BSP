;----------------------------------------------------------------------------
;              ----> DO NOT REMOVE THE FOLLOWING NOTICE <----
;
;                 Copyright (c) 1993 - 2009 Datalight, Inc.
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
;   This module contains rudimentary routines to push information out a
;   standard 8250 serial port.  There is no flow control or buffering, and
;   is not interrupt driven.
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                               Revision History
;   $Log: serial.asm $
;   Revision 1.3  2009/02/07 23:39:38Z  garyp
;   No longer use a relative include path (now handled in the make file).
;   Revision 1.2  2007/11/03 23:31:15Z  Garyp
;   Added the standard module header.
;   Revision 1.1  2005/10/02 01:31:22Z  Pauli
;   Initial revision
;   Revision 1.2  2004/05/28 17:14:58Z  garyp
;   Hacked so this code does not compile if using a flat memory model.
;   Revision 1.1  2004/02/17 21:19:38Z  garyp
;   Initial revision
;----------------------------------------------------------------------------
include dcl.inc
include dluart.inc
include dlx86.inc
include io.inc

ModuleProlog

if FLATMODEL eq FALSE

public _DclCpuSerialGetChar
public _DclCpuSerialPutChar
public _DclCpuSerialInit


;--------------------------------------------------------------------
;       DclCpuSerialGetChar() - Get a character from the serial port
;
;       This function gets a character from the specified serial
;       port.  The port must have been initialized with the
;       DclCpuSerialInit() function.
;
;       The following prototype is used:
;
;       D_UCHAR DclCpuSerialGetChar(D_UINT16 uIOPort);
;
;       Note: No flow control!
;--------------------------------------------------------------------
_DclCpuSerialGetChar    proc
        push    bp
        mov     bp, sp
        mov     dx, ss:[bp+4]           ; port number into DX

  WaitForData:
        INPORT  al,UART_LINESTATUS
        test    al,LSR_RXRD
        jz      WaitForData

        INPORT  al,UART_DATAREG
        xor     ah,ah

        pop     bp
        ret
_DclCpuSerialGetChar    endp


;--------------------------------------------------------------------
;       DclCpuSerialPutChar() - Put a character out the serial port
;
;       This function puts the specified character out the specified
;       serial port.  The port must have been initialized with the
;       DclCpuSerialInit() function.
;
;       The following prototype is used:
;
;       void DclCpuSerialPutChar(D_UINT16 uIOPort, D_UCHAR uChar);
;
;       Note: No flow control!
;--------------------------------------------------------------------
_DclCpuSerialPutChar    proc
        push    bp
        mov     bp, sp
        mov     dx, ss:[bp+4]           ; port number into DX
        mov     ax, ss:[bp+6]
        mov     ah,al                   ; char in AH

  WaitForBuffer:
        INPORT  al,UART_LINESTATUS
        test    al,LSR_TBE
        jz      WaitForBuffer

        OUTPORT UART_DATAREG,ah

        pop     bp
        ret
_DclCpuSerialPutChar    endp


;--------------------------------------------------------------------
;       DclCpuSerialInit() - Initialize the serial port
;
;       This function initializes the specified serial port so that
;       it can be used with the DclCpuSerialPutChar() function.
;
;       The following prototype is used:
;
;       DCLSTATUS DclCpuSerialInit(D_UINT16 uIOPort, D_UINT16 uBaudRateDivisor);
;
;       The baud rate divisor values can be found in dluart.inc.
;
;       Note: No flow control!
;--------------------------------------------------------------------
_DclCpuSerialInit       proc
        push    bp
        mov     bp, sp
        mov     dx, ss:[bp+4]           ; port number into DX
        mov     cx, ss:[bp+6]           ; baud rate divisor into CX

        OUTPORT UART_INTID,        INTID_CHANGESIGNAL
        OUTPORT UART_DATAFORMAT,   DFMT_DLAB
        OUTPORT UART_MSBDIVISOR,   ch
        OUTPORT UART_LSBDIVISOR,   cl
        OUTPORT UART_DATAFORMAT,   DFMT_DATABITS_8      ; n,8,1
        OUTPORT UART_MODEMCONTROL, <MCR_DTR + MCR_RTS>

        xor     ax, ax                  ; always successful for now
        pop     bp
        ret
_DclCpuSerialInit       endp


else

.data
; Define this simply to avoid assembler warnings
public	bDclSerialData
bDclSerialData	db  0

endif 	; FLATMODEL eq FALSE



end
