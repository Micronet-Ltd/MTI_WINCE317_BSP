/*++

Copyright (c) 2002-2003  Future Technology Devices International Ltd.

Module Name:

    emul.h

Abstract:

    VCP device driver for FTDI FT232BM
    Special emulation mode.

Environment:

    kernel & user mode

Revision History:

    23/12/02    awm     Created.
    07/01/03    awm     Changed emulation control word definition.
    10/01/03    awm     Changed FT_EMUL_SUBSTITUTION_MODE to fix
                        problem where substitution was always on.
    20/01/03    awm     Added FT_EMUL_DISABLE_SUB_IF_FC_OFF flag to control
                        disable substitution is flow control is off.
    03/04/03    awm     Added support for clearing modem status if break
                        detected.
    23/04/03    awm     Added Buffer to FTEMUL to support emulation mode
                        on closedown.
    25/04/03    awm     Added LSR to support break detection.


--*/


#ifndef _ftemul_h_
#define _ftemul_h_


//
// Default characters
//

#define FT_EMUL_DEF_ESC     0x10
#define FT_EMUL_DEF_XOFF    0x13
#define FT_EMUL_DEF_XON     0x11

//
// EscChar + x identifiers
//

#define FT_EMUL_ESC_XON     0x01
#define FT_EMUL_ESC_XOFF    0x02
#define FT_EMUL_ESC_SETDTR  0x03
#define FT_EMUL_ESC_CLRDTR  0x04
#define FT_EMUL_ESC_SETRTS  0x05
#define FT_EMUL_ESC_CLRRTS  0x06
#define FT_EMUL_ESC_SETDSR  0x07
#define FT_EMUL_ESC_CLRDSR  0x08
#define FT_EMUL_ESC_SETDCD  0x09
#define FT_EMUL_ESC_CLRDCD  0x0a
#define FT_EMUL_ESC_SETRI   0x0b
#define FT_EMUL_ESC_CLRRI   0x0c
#define FT_EMUL_ESC_SETCTS  0x0d
#define FT_EMUL_ESC_CLRCTS  0x0e

//
// Emulation control word substitution bits
//

#define FT_EMUL_FORCED_XONXOFF          (1)
#define FT_EMUL_SUB_XONXOFF             (1<<1)
#define FT_EMUL_SUB_DSR                 (1<<3)
#define FT_EMUL_SUB_RI                  (1<<5)
#define FT_EMUL_SUB_DCD                 (1<<7)
#define FT_EMUL_SUB_CTS                 (1<<9)
#define FT_EMUL_SUB_RTS                 (1<<11)
#define FT_EMUL_SUB_DTR                 (1<<13)
#define FT_EMUL_DISABLE_SUB_IF_FC_OFF   (1<<16)
#define FT_EMUL_FC_DISABLED             (1<<17)
#define FT_EMUL_CLEAR_MSR_IF_BREAK      (1<<18)

#define FT_EMUL_DEF_MODE    0

//
// Return codes
//

#define FT_EMUL_SUBSTITUTE_CODE     0x00000100

//
// macro defines substitution returns from state machine
//

#define FT_EMUL_RET_CODE(x)         (FT_EMUL_SUBSTITUTE_CODE | (x))

//
// substitution returns from state machine
//

#define FT_EMUL_RET_0               FT_EMUL_RET_CODE(0)
#define FT_EMUL_RET_ESC_XON         FT_EMUL_RET_CODE(FT_EMUL_ESC_XON)
#define FT_EMUL_RET_ESC_XOFF        FT_EMUL_RET_CODE(FT_EMUL_ESC_XOFF)
#define FT_EMUL_RET_DTR_ACTIVE      FT_EMUL_RET_CODE(FT_EMUL_ESC_SETDTR)
#define FT_EMUL_RET_DTR_INACTIVE    FT_EMUL_RET_CODE(FT_EMUL_ESC_CLRDTR)
#define FT_EMUL_RET_RTS_ACTIVE      FT_EMUL_RET_CODE(FT_EMUL_ESC_SETRTS)
#define FT_EMUL_RET_RTS_INACTIVE    FT_EMUL_RET_CODE(FT_EMUL_ESC_CLRRTS)
#define FT_EMUL_RET_DSR_ACTIVE      FT_EMUL_RET_CODE(FT_EMUL_ESC_SETDSR)
#define FT_EMUL_RET_DSR_INACTIVE    FT_EMUL_RET_CODE(FT_EMUL_ESC_CLRDSR)
#define FT_EMUL_RET_DCD_ACTIVE      FT_EMUL_RET_CODE(FT_EMUL_ESC_SETDCD)
#define FT_EMUL_RET_DCD_INACTIVE    FT_EMUL_RET_CODE(FT_EMUL_ESC_CLRDCD)
#define FT_EMUL_RET_RI_ACTIVE       FT_EMUL_RET_CODE(FT_EMUL_ESC_SETRI)
#define FT_EMUL_RET_RI_INACTIVE     FT_EMUL_RET_CODE(FT_EMUL_ESC_CLRRI)
#define FT_EMUL_RET_CTS_ACTIVE      FT_EMUL_RET_CODE(FT_EMUL_ESC_SETCTS)
#define FT_EMUL_RET_CTS_INACTIVE    FT_EMUL_RET_CODE(FT_EMUL_ESC_CLRCTS)

#define FT_EMUL_RET_ESC_ESC         FT_EMUL_RET_CODE(0xfe)
#define FT_EMUL_RET_ESC_ANY         FT_EMUL_RET_CODE(0xff)

//
// non-substitution return code
//

#define FT_EMUL_RET_1               1

//
// macro distinguishes substituted char return from state machine
//

#define FT_EMUL_SUBSTITUTED_CHAR(x) (((x) & FT_EMUL_SUBSTITUTE_CODE) != 0)


#define FT_EMUL_SUBSTITUTION_MODE(Ext)  \
    (((Ext)->EmulVars.SubMode & 0xfffcfffe) != 0)


//
// modem control requests
//

enum {
    FT_EMUL_BUILD_SET_DTR_REQUEST = 1,
    FT_EMUL_BUILD_CLR_DTR_REQUEST,
    FT_EMUL_BUILD_SET_RTS_REQUEST,
    FT_EMUL_BUILD_CLR_RTS_REQUEST
};

#define FT_EmulBuildSetDtrRequest(Ext)  \
            FT_EmulBuildModemCtrlRequest((Ext),FT_EMUL_BUILD_SET_DTR_REQUEST)

#define FT_EmulBuildClrDtrRequest(Ext)  \
            FT_EmulBuildModemCtrlRequest((Ext),FT_EMUL_BUILD_CLR_DTR_REQUEST)

#define FT_EmulBuildSetRtsRequest(Ext)  \
            FT_EmulBuildModemCtrlRequest((Ext),FT_EMUL_BUILD_SET_RTS_REQUEST)

#define FT_EmulBuildClrRtsRequest(Ext)  \
            FT_EmulBuildModemCtrlRequest((Ext),FT_EMUL_BUILD_CLR_RTS_REQUEST)

#define FT_EMUL_MODEM_CTRL_REQUEST_LEN  2

#define FT_EMUL_MODEM_CTRL_REQUEST(Ext) ((Ext)->EmulVars.ModemCtrl != 0)

#define FT_EMUL_MODEM_CTRL_MASK \
    (FT_EMUL_SUB_DSR | FT_EMUL_SUB_DCD | FT_EMUL_SUB_CTS | FT_EMUL_SUB_RI)

//
// macros for substitution enabled by flow control mode of operation
//

#define FT_EMUL_DISABLE_SUB_MASK    \
            (FT_EMUL_DISABLE_SUB_IF_FC_OFF | FT_EMUL_FC_DISABLED)

#define FT_EMUL_DISABLE_SUB_BY_FLOW(Emul) \
    (((Emul)->SubMode & FT_EMUL_DISABLE_SUB_MASK) == FT_EMUL_DISABLE_SUB_MASK)

#define FT_EMUL_SAVE_FLOW_CONTROL(Emul) \
    ((Emul)->SubMode & FT_EMUL_DISABLE_SUB_IF_FC_OFF)

#define FT_EMUL_SET_FLOW_DISABLED(Emul) \
    ((Emul)->SubMode |= FT_EMUL_FC_DISABLED)

#define FT_EMUL_CLEAR_FLOW_DISABLED(Emul) \
    ((Emul)->SubMode &= ~FT_EMUL_FC_DISABLED)

//
// macros for determining if substitution mode is enabled
//

#define FT_EMUL_SUB_XONXOFF_ENABLED(Emul)  \
    (((Emul)->SubMode & FT_EMUL_SUB_XONXOFF) && \
        !FT_EMUL_DISABLE_SUB_BY_FLOW(Emul))

#define FT_EMUL_SUB_DSR_ENABLED(Emul)  \
    (((Emul)->SubMode & FT_EMUL_SUB_DSR) && \
        !FT_EMUL_DISABLE_SUB_BY_FLOW(Emul))

#define FT_EMUL_SUB_DCD_ENABLED(Emul)  \
    (((Emul)->SubMode & FT_EMUL_SUB_DCD) && \
        !FT_EMUL_DISABLE_SUB_BY_FLOW(Emul))

#define FT_EMUL_SUB_CTS_ENABLED(Emul)  \
    (((Emul)->SubMode & FT_EMUL_SUB_CTS) && \
        !FT_EMUL_DISABLE_SUB_BY_FLOW(Emul))

#define FT_EMUL_SUB_RI_ENABLED(Emul)  \
    (((Emul)->SubMode & FT_EMUL_SUB_RI) && \
        !FT_EMUL_DISABLE_SUB_BY_FLOW(Emul))

#define FT_EMUL_SUB_DTR_ENABLED(Emul)  \
    (((Emul)->SubMode & FT_EMUL_SUB_DTR) && \
        !FT_EMUL_DISABLE_SUB_BY_FLOW(Emul))

#define FT_EMUL_SUB_RTS_ENABLED(Emul)  \
    (((Emul)->SubMode & FT_EMUL_SUB_RTS) && \
        !FT_EMUL_DISABLE_SUB_BY_FLOW(Emul))

//
// condition for using XON/XOFF flow control.
//
#define SerialForceXonXoff(Ext) \
    ((Ext)->EmulVars.SubMode & FT_EMUL_FORCED_XONXOFF)

//
// condition for clear modem status register if break detected
//
#define SerialClearMsrIfBreak(Ext) \
    ((Ext)->EmulVars.SubMode & FT_EMUL_CLEAR_MSR_IF_BREAK)

//
// condition for BREAK active
//
#define FT_EMUL_BREAK_ACTIVE(Emul)  \
    ((Emul)->LSR & SERIAL_LSR_BI)


//
// Emulation mode variables
//

typedef struct _ftemul {
    ULONG   SubMode;
    UCHAR   RxState;
    UCHAR   Esc;
    UCHAR   Xoff;
    UCHAR   Xon;
    UCHAR   TxState;
    UCHAR   ModemCtrl;
    UCHAR   MSR;
    UCHAR   MSRMask;
    UCHAR   LSR;
    UCHAR   Buffer[FT_EMUL_MODEM_CTRL_REQUEST_LEN+1];
	PUCHAR	pWriteBuffer;		// Our output buffer pointer
} FTEMUL, *PFTEMUL;


#endif /* _ftemul_h_ */

