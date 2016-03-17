/*++

Copyright (c) 2000-2003 Future Technology Devices International Ltd.

Module Name:

    serial.h

Abstract:

    Type definitions and data for the serial port driver

Environment:

    Kernel mode

Revision History:

    07/11/00    awm     Added BulkInBufferSize and BulkOutBufferSize fields
                        to device extension.
    07/12/00    awm     Added MinReadTimeout field to device extension.
    15/01/01    awm     Added DeviceMutex to device extension.
    23/02/01    awm     Added BusFlowControl to device extension.
    28/02/01    awm     Added compile-time option DEFAULT_BAUD_RATE_144000
                        to incorporate special for LG Telecom into mainline.
    27/03/01    awm     Added MinWriteTimeout field to device extension.
    13/07/01    awm     Added TasksStarted event to device extension.
    19/07/01    awm     Added RestoreD0Event to device extension.
    21/09/01    awm     Added RetainPowerOnClose to device extension.
    28/09/01    awm     Added WriteIrpInUsbd to device extension.
    04/10/01    awm     Added WriteIrpInUsbdLen to device extension.
    10/10/01    awm     Removed DeviceMutex to bus driver.
    13/11/01    awm     Added TxImmediateAvailable to device extension.
    18/12/01    awm     Added SerialPrinter to device extension.
    16/01/02    awm     Added NotD0 and ConfigDataFlags to device extension,
                        SerialPoweringDown and new config data flag for
                        Spyrus hibernation problem.
    20/02/02    awm     Added new config data flag to enable use of
                        SERIAL_EV_EVENT2 as surprise removal indication.
    25/02/02    awm     Added BulkOutTerminateDelay to device extension.
    28/06/02    awm     Changes references for REQUEST_GET_DEVICE_INFO to
                        REQUEST_GET_DEVICE_PIPE_INFO.
    18/10/02    awm     Support for latency timer.
    24/10/02    awm     Change baud rate table to support 32-bit divisors.
    16/12/02    awm     Added flag to force XON/XOFF flow control.
    24/12/02    awm     Added emulation mode vars to device extension.
    07/01/03    awm     Removed flag to force XON/XOFF flow control because
                        this is now handled in emul module.
    19/03/03    awm     Added device context to restore state after
                        suspend/resume.
    22/05/03    awm     Added FTDI_CD_FLAGS_SET_RTS_ON_CLOSE.
    22/07/03    awm     Added IoctlQueue and CurrentIoctlIrp to device
                        extension and ConfigData flag for asynchronous
                        vendor commands.
    23/09/03    awm     Added AttemptClrRTS flag to device extension.
    03/10/03    awm     Added FTDI_CD_FLAGS_SET_DTR_ON_OPEN.
    28/10/03    awm     Added FTDI_CD_FLAGS_DONT_QUEUE_VENDOR_REQUESTS.


--*/

/*++

Copyright (c) 1990, 1991, 1992, 1993 - 1997 Microsoft Corporation

Module Name :

    serial.h

Abstract:

    Type definitions and data for the serial port driver

Author:

    Anthony V. Ercolano                 April 8, 1991

--*/

#ifndef _serial_h_
#define _serial_h_

#define USING_FTDI_SERIAL_DEVICE
#ifndef WINCE
#define WIN2000_BUILD
#endif
//
// Special for LG Telecom
// #define for unsupported baud rates defualt to 144000 baud
//
#undef DEFAULT_BAUD_RATE_144000


#ifdef USING_FTDI_SERIAL_DEVICE

#ifndef WINCE
//#include "task.h"
#endif

//
// Minimum value for ReadTotalTimeoutConstant
// Used in SerialStartRead()
//
#define MIN_READ_TOTAL_TIMEOUT_CONSTANT 1000L

//
// Minimum value for WriteTotalTimeoutConstant
// Used in SerialStartWrite()
//
#define MIN_WRITE_TOTAL_TIMEOUT_CONSTANT 1000L

//
// copy of serial device registers (required for some IOCTLs)
// 
typedef struct _SerialRegs {

   UCHAR FCR;
   UCHAR LCR;
   UCHAR MCR;
   UCHAR LSR;
   UCHAR MSR;

} SERIAL_REGS;

#define CONFIGURATION_INFORMATION_SERIAL_COUNT	4

typedef BOOLEAN (*PFT_SYNCHRONIZE_ROUTINE)(IN PVOID SynchronizeContext);

//
// Flags for ConfigData
//
#define FTDI_CD_FLAGS_SET_DTR_ON_OPEN               0x00000002
#define FTDI_CD_FLAGS_ASYNC_VENDOR_IF               0x00000004
#define FTDI_CD_FLAGS_SET_RTS_ON_CLOSE              0x00000008
#define FTDI_CD_FLAGS_BM_BAUD_RATES                 0x00000010
#define FTDI_CD_FLAGS_EV2_AS_REMOVAL                0x00000020
#define FTDI_CD_FLAGS_POFF_CANCEL                   0x00000040
#define FTDI_CD_FLAGS_SERIAL_PRINTER                0x00000080
#define FTDI_CD_FLAGS_DONT_QUEUE_VENDOR_REQUESTS    0x00000100


#endif

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'XMOC')
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,'XMOC')
#endif


//
// The following definition is used to include/exclude changes made for power
// support in the driver.  If non-zero the support is included.  If zero the
// support is excluded.
//

#define POWER_SUPPORT   1

//
// The following is used to tell the serial driver to perform legacy detection
// and initialization until PnP functionality can be fully implemented.
//
// #define FAKE_IT         1

#define RM_PNP_CODE


#if DBG
#define SERDIAG1              ((ULONG)0x00000001)
#define SERDIAG2              ((ULONG)0x00000002)
#define SERDIAG3              ((ULONG)0x00000004)
#define SERDIAG4              ((ULONG)0x00000008)
#define SERDIAG5              ((ULONG)0x00000010)
#define SERIRPPATH            ((ULONG)0x00000020)
#define SERINITCODE           ((ULONG)0x00000040)
#define SERTRACECALLS         ((ULONG)0x00000040)
#define SERDIAGX              ((ULONG)0x00000080)
#define SERPNPPOWER           ((ULONG)0x00000100)
#define SERMYDIAG             ((ULONG)0x00000200)
#define SERFLOW               ((ULONG)0x20000000)
#define SERERRORS             ((ULONG)0x40000000)
#define SERBUGCHECK           ((ULONG)0x80000000)
#define SERDBGALL             ((ULONG)0xFFFFFFFF)

#define SER_DBG_DEFAULT       SERDBGALL


extern ULONG SerialDebugLevel;
#define SerialDump(LEVEL,STRING) \
        do { \
            ULONG _level = (LEVEL); \
            if (SerialDebugLevel & _level) { \
                DbgPrint STRING; \
            } \
            if (_level == SERBUGCHECK) { \
                ASSERT(FALSE); \
            } \
        } while (0)
#else
#define SerialDump(LEVEL,STRING) do {NOTHING;} while (0)
#endif


//
// Some default driver values.  We will check the registry for
// them first.
//
#define SERIAL_UNINITIALIZED_DEFAULT    1234567
#define SERIAL_FORCE_FIFO_DEFAULT       1
#define SERIAL_RX_FIFO_DEFAULT          8
#define SERIAL_TX_FIFO_DEFAULT          14
#define SERIAL_PERMIT_SHARE_DEFAULT     0
#define SERIAL_LOG_FIFO_DEFAULT         0


//
// This define gives the default Object directory
// that we should use to insert the symbolic links
// between the NT device name and namespace used by
// that object directory.
#define DEFAULT_DIRECTORY L"DosDevices"

//
// For the above directory, the serial port will
// use the following name as the suffix of the serial
// ports for that directory.  It will also append
// a number onto the end of the name.  That number
// will start at 1.
#define DEFAULT_SERIAL_NAME L"MOM"
//
//
// This define gives the default NT name for
// for serial ports detected by the firmware.
// This name will be appended to Device prefix
// with a number following it.  The number is
// incremented each time encounter a serial
// port detected by the firmware.  Note that
// on a system with multiple busses, this means
// that the first port on a bus is not necessarily
// \Device\Serial0.
//
#define DEFAULT_NT_SUFFIX L"Serial"

//
// This value - which could be redefined at compile
// time, define the stride between registers
//
#if !defined(SERIAL_REGISTER_STRIDE)
#define SERIAL_REGISTER_STRIDE 1
#endif

//
// Offsets from the base register address of the
// various registers for the 8250 family of UARTS.
//
#define RECEIVE_BUFFER_REGISTER    ((ULONG)((0x00)*SERIAL_REGISTER_STRIDE))
#define TRANSMIT_HOLDING_REGISTER  ((ULONG)((0x00)*SERIAL_REGISTER_STRIDE))
#define INTERRUPT_ENABLE_REGISTER  ((ULONG)((0x01)*SERIAL_REGISTER_STRIDE))
#define INTERRUPT_IDENT_REGISTER   ((ULONG)((0x02)*SERIAL_REGISTER_STRIDE))
#define FIFO_CONTROL_REGISTER      ((ULONG)((0x02)*SERIAL_REGISTER_STRIDE))
#define LINE_CONTROL_REGISTER      ((ULONG)((0x03)*SERIAL_REGISTER_STRIDE))
#define MODEM_CONTROL_REGISTER     ((ULONG)((0x04)*SERIAL_REGISTER_STRIDE))
#define LINE_STATUS_REGISTER       ((ULONG)((0x05)*SERIAL_REGISTER_STRIDE))
#define MODEM_STATUS_REGISTER      ((ULONG)((0x06)*SERIAL_REGISTER_STRIDE))
#define DIVISOR_LATCH_LSB          ((ULONG)((0x00)*SERIAL_REGISTER_STRIDE))
#define DIVISOR_LATCH_MSB          ((ULONG)((0x01)*SERIAL_REGISTER_STRIDE))
#define SERIAL_REGISTER_SPAN       ((ULONG)(7*SERIAL_REGISTER_STRIDE))

//
// If we have an interrupt status register this is its assumed
// length.
//
#define SERIAL_STATUS_LENGTH       ((ULONG)(1*SERIAL_REGISTER_STRIDE))

//
// Bitmask definitions for accessing the 8250 device registers.
//

//
// These bits define the number of data bits trasmitted in
// the Serial Data Unit (SDU - Start,data, parity, and stop bits)
//
#define SERIAL_DATA_LENGTH_5 0x00
#define SERIAL_DATA_LENGTH_6 0x01
#define SERIAL_DATA_LENGTH_7 0x02
#define SERIAL_DATA_LENGTH_8 0x03


//
// These masks define the interrupts that can be enabled or disabled.
//
//
// This interrupt is used to notify that there is new incomming
// data available.  The SERIAL_RDA interrupt is enabled by this bit.
//
#define SERIAL_IER_RDA   0x01

//
// This interrupt is used to notify that there is space available
// in the transmitter for another character.  The SERIAL_THR
// interrupt is enabled by this bit.
//
#define SERIAL_IER_THR   0x02

//
// This interrupt is used to notify that some sort of error occured
// with the incomming data.  The SERIAL_RLS interrupt is enabled by
// this bit.
#define SERIAL_IER_RLS   0x04

//
// This interrupt is used to notify that some sort of change has
// taken place in the modem control line.  The SERIAL_MS interrupt is
// enabled by this bit.
//
#define SERIAL_IER_MS    0x08


//
// These masks define the values of the interrupt identification
// register.  The low bit must be clear in the interrupt identification
// register for any of these interrupts to be valid.  The interrupts
// are defined in priority order, with the highest value being most
// important.  See above for a description of what each interrupt
// implies.
//
#define SERIAL_IIR_RLS      0x06
#define SERIAL_IIR_RDA      0x04
#define SERIAL_IIR_CTI      0x0c
#define SERIAL_IIR_THR      0x02
#define SERIAL_IIR_MS       0x00

//
// This bit mask get the value of the high two bits of the
// interrupt id register.  If this is a 16550 class chip
// these bits will be a one if the fifo's are enbled, otherwise
// they will always be zero.
//
#define SERIAL_IIR_FIFOS_ENABLED 0xc0

//
// If the low bit is logic one in the interrupt identification register
// this implies that *NO* interrupts are pending on the device.
//
#define SERIAL_IIR_NO_INTERRUPT_PENDING 0x01



//
// These masks define access to the fifo control register.
//

//
// Enabling this bit in the fifo control register will turn
// on the fifos.  If the fifos are enabled then the high two
// bits of the interrupt id register will be set to one.  Note
// that this only occurs on a 16550 class chip.  If the high
// two bits in the interrupt id register are not one then
// we know we have a lower model chip.
//
//
#define SERIAL_FCR_ENABLE     ((UCHAR)0x01)
#define SERIAL_FCR_RCVR_RESET ((UCHAR)0x02)
#define SERIAL_FCR_TXMT_RESET ((UCHAR)0x04)

//
// This set of values define the high water marks (when the
// interrupts trip) for the receive fifo.
//
#define SERIAL_1_BYTE_HIGH_WATER   ((UCHAR)0x00)
#define SERIAL_4_BYTE_HIGH_WATER   ((UCHAR)0x40)
#define SERIAL_8_BYTE_HIGH_WATER   ((UCHAR)0x80)
#define SERIAL_14_BYTE_HIGH_WATER  ((UCHAR)0xc0)

//
// These masks define access to the line control register.
//

//
// This defines the bit used to control the definition of the "first"
// two registers for the 8250.  These registers are the input/output
// register and the interrupt enable register.  When the DLAB bit is
// enabled these registers become the least significant and most
// significant bytes of the divisor value.
//
#define SERIAL_LCR_DLAB     0x80

//
// This defines the bit used to control whether the device is sending
// a break.  When this bit is set the device is sending a space (logic 0).
//
// Most protocols will assume that this is a hangup.
//
#define SERIAL_LCR_BREAK    0x40

//
// These defines are used to set the line control register.
//
#define SERIAL_5_DATA       ((UCHAR)0x00)
#define SERIAL_6_DATA       ((UCHAR)0x01)
#define SERIAL_7_DATA       ((UCHAR)0x02)
#define SERIAL_8_DATA       ((UCHAR)0x03)
#define SERIAL_DATA_MASK    ((UCHAR)0x03)

#define SERIAL_1_STOP       ((UCHAR)0x00)
#define SERIAL_1_5_STOP     ((UCHAR)0x04) // Only valid for 5 data bits
#define SERIAL_2_STOP       ((UCHAR)0x04) // Not valid for 5 data bits
#define SERIAL_STOP_MASK    ((UCHAR)0x04)

#define SERIAL_NONE_PARITY  ((UCHAR)0x00)
#define SERIAL_ODD_PARITY   ((UCHAR)0x08)
#define SERIAL_EVEN_PARITY  ((UCHAR)0x18)
#define SERIAL_MARK_PARITY  ((UCHAR)0x28)
#define SERIAL_SPACE_PARITY ((UCHAR)0x38)
#define SERIAL_PARITY_MASK  ((UCHAR)0x38)

//
// These masks define access the modem control register.
//

//
// This bit controls the data terminal ready (DTR) line.  When
// this bit is set the line goes to logic 0 (which is then inverted
// by normal hardware).  This is normally used to indicate that
// the device is available to be used.  Some odd hardware
// protocols (like the kernel debugger) use this for handshaking
// purposes.
//
#define SERIAL_MCR_DTR      0x01

//
// This bit controls the ready to send (RTS) line.  When this bit
// is set the line goes to logic 0 (which is then inverted by the normal
// hardware).  This is used for hardware handshaking.  It indicates that
// the hardware is ready to send data and it is waiting for the
// receiving end to set clear to send (CTS).
//
#define SERIAL_MCR_RTS      0x02

//
// This bit is used for general purpose output.
//
#define SERIAL_MCR_OUT1     0x04

//
// This bit is used for general purpose output.
//
#define SERIAL_MCR_OUT2     0x08

//
// This bit controls the loopback testing mode of the device.  Basically
// the outputs are connected to the inputs (and vice versa).
//
#define SERIAL_MCR_LOOP     0x10


//
// These masks define access to the line status register.  The line
// status register contains information about the status of data
// transfer.  The first five bits deal with receive data and the
// last two bits deal with transmission.  An interrupt is generated
// whenever bits 1 through 4 in this register are set.
//

//
// This bit is the data ready indicator.  It is set to indicate that
// a complete character has been received.  This bit is cleared whenever
// the receive buffer register has been read.
//
#define SERIAL_LSR_DR       0x01

//
// This is the overrun indicator.  It is set to indicate that the receive
// buffer register was not read befor a new character was transferred
// into the buffer.  This bit is cleared when this register is read.
//
#define SERIAL_LSR_OE       0x02

//
// This is the parity error indicator.  It is set whenever the hardware
// detects that the incoming serial data unit does not have the correct
// parity as defined by the parity select in the line control register.
// This bit is cleared by reading this register.
//
#define SERIAL_LSR_PE       0x04

//
// This is the framing error indicator.  It is set whenever the hardware
// detects that the incoming serial data unit does not have a valid
// stop bit.  This bit is cleared by reading this register.
//
#define SERIAL_LSR_FE       0x08

//
// This is the break interrupt indicator.  It is set whenever the data
// line is held to logic 0 for more than the amount of time it takes
// to send one serial data unit.  This bit is cleared whenever the
// this register is read.
//
#define SERIAL_LSR_BI       0x10

//
// This is the transmit holding register empty indicator.  It is set
// to indicate that the hardware is ready to accept another character
// for transmission.  This bit is cleared whenever a character is
// written to the transmit holding register.
//
#define SERIAL_LSR_THRE     0x20

//
// This bit is the transmitter empty indicator.  It is set whenever the
// transmit holding buffer is empty and the transmit shift register
// (a non-software accessable register that is used to actually put
// the data out on the wire) is empty.  Basically this means that all
// data has been sent.  It is cleared whenever the transmit holding or
// the shift registers contain data.
//
#define SERIAL_LSR_TEMT     0x40

//
// This bit indicates that there is at least one error in the fifo.
// The bit will not be turned off until there are no more errors
// in the fifo.
//
#define SERIAL_LSR_FIFOERR  0x80


//
// These masks are used to access the modem status register.
// Whenever one of the first four bits in the modem status
// register changes state a modem status interrupt is generated.
//

//
// This bit is the delta clear to send.  It is used to indicate
// that the clear to send bit (in this register) has *changed*
// since this register was last read by the CPU.
//
#define SERIAL_MSR_DCTS     0x01

//
// This bit is the delta data set ready.  It is used to indicate
// that the data set ready bit (in this register) has *changed*
// since this register was last read by the CPU.
//
#define SERIAL_MSR_DDSR     0x02

//
// This is the trailing edge ring indicator.  It is used to indicate
// that the ring indicator input has changed from a low to high state.
//
#define SERIAL_MSR_TERI     0x04

//
// This bit is the delta data carrier detect.  It is used to indicate
// that the data carrier bit (in this register) has *changed*
// since this register was last read by the CPU.
//
#define SERIAL_MSR_DDCD     0x08

//
// This bit contains the (complemented) state of the clear to send
// (CTS) line.
//
#define SERIAL_MSR_CTS      0x10

//
// This bit contains the (complemented) state of the data set ready
// (DSR) line.
//
#define SERIAL_MSR_DSR      0x20

//
// This bit contains the (complemented) state of the ring indicator
// (RI) line.
//
#define SERIAL_MSR_RI       0x40

//
// This bit contains the (complemented) state of the data carrier detect
// (DCD) line.
//
#define SERIAL_MSR_DCD      0x80

//
// This should be more than enough space to hold then
// numeric suffix of the device name.
//
#define DEVICE_NAME_DELTA 20


//
// Up to 16 Ports Per card.  However for sixteen
// port cards the interrupt status register must me
// the indexing kind rather then the bitmask kind.
//
//
#define SERIAL_MAX_PORTS_INDEXED (16)
#define SERIAL_MAX_PORTS_NONINDEXED (8)

//
// Default xon/xoff characters.
//
#define SERIAL_DEF_XON 0x11
#define SERIAL_DEF_XOFF 0x13

//
// Reasons that recption may be held up.
//
#define SERIAL_RX_DTR       ((ULONG)0x01)
#define SERIAL_RX_XOFF      ((ULONG)0x02)
#define SERIAL_RX_RTS       ((ULONG)0x04)
#define SERIAL_RX_DSR       ((ULONG)0x08)

//
// Reasons that transmission may be held up.
//
#define SERIAL_TX_CTS       ((ULONG)0x01)
#define SERIAL_TX_DSR       ((ULONG)0x02)
#define SERIAL_TX_DCD       ((ULONG)0x04)
#define SERIAL_TX_XOFF      ((ULONG)0x08)
#define SERIAL_TX_BREAK     ((ULONG)0x10)

//
// These values are used by the routines that can be used
// to complete a read (other than interval timeout) to indicate
// to the interval timeout that it should complete.
//
#define SERIAL_COMPLETE_READ_CANCEL ((LONG)-1)
#define SERIAL_COMPLETE_READ_TOTAL ((LONG)-2)
#define SERIAL_COMPLETE_READ_COMPLETE ((LONG)-3)

//
// These are default values that shouldn't appear in the registry
//
#define SERIAL_BAD_VALUE ((ULONG)-1)

#if DBG

//
// Page locking seems to differ between WIN98 and WIN2000,
// and we reflect this in the following definitions.
//

#ifdef WIN2000_BUILD

#define SerialLockPagableSectionByHandle(_secHandle) \
{ \
    MmLockPagableSectionByHandle((_secHandle)); \
    InterlockedIncrement(&SerialGlobals.PAGESER_Count); \
}

#define SerialUnlockPagableImageSection(_secHandle) \
{ \
   InterlockedDecrement(&SerialGlobals.PAGESER_Count); \
   MmUnlockPagableImageSection(_secHandle); \
}

#else

#define SerialLockPagableSectionByHandle(_secHandle) \
{ \
    InterlockedIncrement(&SerialGlobals.PAGESER_Count); \
}

#define SerialUnlockPagableImageSection(_secHandle) \
{ \
   InterlockedDecrement(&SerialGlobals.PAGESER_Count); \
}

#endif

#define SERIAL_LOCKED_PAGED_CODE() \
    if ((KeGetCurrentIrql() > APC_LEVEL)  \
    && (SerialGlobals.PAGESER_Count == 0)) { \
    KdPrint(("SERIAL: Pageable code called at IRQL %d without lock \n", \
             KeGetCurrentIrql())); \
        ASSERT(FALSE); \
        }

#else

#ifdef WIN2000_BUILD

#define SerialLockPagableSectionByHandle(_secHandle) \
{ \
    MmLockPagableSectionByHandle((_secHandle)); \
}

#define SerialUnlockPagableImageSection(_secHandle) \
{ \
   MmUnlockPagableImageSection(_secHandle); \
}

#else

#define SerialLockPagableSectionByHandle(_secHandle)

#define SerialUnlockPagableImageSection(_secHandle)

#endif

#define SERIAL_LOCKED_PAGED_CODE()

#endif // DBG



#define SerialRemoveQueueDpc(_dpc, _pExt) \
{ \
  if (KeRemoveQueueDpc((_dpc))) { \
     InterlockedDecrement(&(_pExt)->DpcCount); \
  } \
}

#define SERIAL_PNPACCEPT_OK                 0x0L
#define SERIAL_PNPACCEPT_REMOVING           0x1L
#define SERIAL_PNPACCEPT_STOPPING           0x2L
#define SERIAL_PNPACCEPT_STOPPED            0x4L
#define SERIAL_PNPACCEPT_SURPRISE_REMOVING  0x8L

#define SERIAL_PNP_ADDED                    0x0L
#define SERIAL_PNP_STARTED                  0x1L
#define SERIAL_PNP_QSTOP                    0x2L
#define SERIAL_PNP_STOPPING                 0x3L
#define SERIAL_PNP_QREMOVE                  0x4L
#define SERIAL_PNP_REMOVING                 0x5L
#define SERIAL_PNP_RESTARTING               0x6L

#define SERIAL_FLAGS_CLEAR                  0x0L
#define SERIAL_FLAGS_STARTED                0x1L
#define SERIAL_FLAGS_STOPPED                0x2L
#define SERIAL_FLAGS_BROKENHW               0x4L


//
// Reads the divisor latch register.  The divisor latch register
// is used to control the baud rate of the 8250.
//
// As with all of these routines it is assumed that it is called
// at a safe point to access the hardware registers.  In addition
// it also assumes that the data is correct.
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
// DesiredDivisor - A pointer to the 2 byte word which will contain
//                  the value of the divisor.
//
#define READ_DIVISOR_LATCH(BaseAddress,PDesiredDivisor)           \
do                                                                \
{                                                                 \
    PUCHAR Address = BaseAddress;                                 \
    PSHORT PDivisor = PDesiredDivisor;                            \
    UCHAR LineControl;                                            \
    UCHAR Lsb;                                                    \
    UCHAR Msb;                                                    \
    LineControl = READ_PORT_UCHAR(Address+LINE_CONTROL_REGISTER); \
    WRITE_PORT_UCHAR(                                             \
        Address+LINE_CONTROL_REGISTER,                            \
        (UCHAR)(LineControl | SERIAL_LCR_DLAB)                    \
        );                                                        \
    Lsb = READ_PORT_UCHAR(Address+DIVISOR_LATCH_LSB);             \
    Msb = READ_PORT_UCHAR(Address+DIVISOR_LATCH_MSB);             \
    *PDivisor = Lsb;                                              \
    *PDivisor = *PDivisor | (((USHORT)Msb) << 8);                 \
    WRITE_PORT_UCHAR(                                             \
        Address+LINE_CONTROL_REGISTER,                            \
        LineControl                                               \
        );                                                        \
} while (0)

//
// This macro reads the interrupt enable register.
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
#define READ_INTERRUPT_ENABLE(BaseAddress)                     \
    (READ_PORT_UCHAR((BaseAddress)+INTERRUPT_ENABLE_REGISTER))

//
// This macro writes the interrupt enable register.
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
// Values - The values to write to the interrupt enable register.
//
#define WRITE_INTERRUPT_ENABLE(BaseAddress,Values)                \
do                                                                \
{                                                                 \
    WRITE_PORT_UCHAR(                                             \
        BaseAddress+INTERRUPT_ENABLE_REGISTER,                    \
        Values                                                    \
        );                                                        \
} while (0)

//
// This macro disables all interrupts on the hardware.
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
//
#ifndef USING_FTDI_SERIAL_DEVICE
#define DISABLE_ALL_INTERRUPTS(BaseAddress)       \
do                                                \
{                                                 \
    WRITE_INTERRUPT_ENABLE(BaseAddress,0);        \
} while (0)
#else
#define DISABLE_ALL_INTERRUPTS(BaseAddress)       \
do                                                \
{                                                 \
} while (0)
#endif

//
// This macro enables all interrupts on the hardware.
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
//
#ifndef USING_FTDI_SERIAL_DEVICE
#define ENABLE_ALL_INTERRUPTS(BaseAddress)        \
do                                                \
{                                                 \
    WRITE_INTERRUPT_ENABLE(                       \
        (BaseAddress),                            \
        (UCHAR)(SERIAL_IER_RDA | SERIAL_IER_THR | \
                SERIAL_IER_RLS | SERIAL_IER_MS)   \
        );                                        \
} while (0)
#else
#define ENABLE_ALL_INTERRUPTS(BaseAddress)        \
do                                                \
{                                                 \
} while (0)
#endif                                                  \

//
// This macro reads the interrupt identification register
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
// Note that this routine potententially quites a transmitter
// empty interrupt.  This is because one way that the transmitter
// empty interrupt is cleared is to simply read the interrupt id
// register.
//
//
#define READ_INTERRUPT_ID_REG(BaseAddress)                          \
    (READ_PORT_UCHAR((BaseAddress)+INTERRUPT_IDENT_REGISTER))

//
// This macro reads the modem control register
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
//
#define READ_MODEM_CONTROL(BaseAddress)                          \
    (READ_PORT_UCHAR((BaseAddress)+MODEM_CONTROL_REGISTER))

//
// This macro reads the modem status register
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
//
#define READ_MODEM_STATUS(BaseAddress)                          \
    (READ_PORT_UCHAR((BaseAddress)+MODEM_STATUS_REGISTER))

//
// This macro reads a value out of the receive buffer
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
//
#define READ_RECEIVE_BUFFER(BaseAddress)                          \
    (READ_PORT_UCHAR((BaseAddress)+RECEIVE_BUFFER_REGISTER))

//
// This macro reads the line status register
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
//
#define READ_LINE_STATUS(BaseAddress)                          \
    (READ_PORT_UCHAR((BaseAddress)+LINE_STATUS_REGISTER))

//
// This macro writes the line control register
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
//
#define WRITE_LINE_CONTROL(BaseAddress,NewLineControl)           \
do                                                               \
{                                                                \
    WRITE_PORT_UCHAR(                                            \
        (BaseAddress)+LINE_CONTROL_REGISTER,                     \
        (NewLineControl)                                         \
        );                                                       \
} while (0)

//
// This macro reads the line control register
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
//
#define READ_LINE_CONTROL(BaseAddress)           \
    (READ_PORT_UCHAR((BaseAddress)+LINE_CONTROL_REGISTER))


//
// This macro writes to the transmit register
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
// TransmitChar - The character to send down the wire.
//
//
#define WRITE_TRANSMIT_HOLDING(BaseAddress,TransmitChar)       \
do                                                             \
{                                                              \
    WRITE_PORT_UCHAR(                                          \
        (BaseAddress)+TRANSMIT_HOLDING_REGISTER,               \
        (TransmitChar)                                         \
        );                                                     \
} while (0)

//
// This macro writes to the transmit FIFO register
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
// TransmitChars - Pointer to the characters to send down the wire.
//
// TxN - number of charactes to send.
//
//
#define WRITE_TRANSMIT_FIFO_HOLDING(BaseAddress,TransmitChars,TxN)  \
do                                                             \
{                                                              \
    WRITE_PORT_BUFFER_UCHAR(                                   \
        (BaseAddress)+TRANSMIT_HOLDING_REGISTER,               \
        (TransmitChars),                                       \
        (TxN)                                                  \
        );                                                     \
} while (0)

//
// This macro writes to the control register
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
// ControlValue - The value to set the fifo control register too.
//
//
#define WRITE_FIFO_CONTROL(BaseAddress,ControlValue)           \
do                                                             \
{                                                              \
    WRITE_PORT_UCHAR(                                          \
        (BaseAddress)+FIFO_CONTROL_REGISTER,                   \
        (ControlValue)                                         \
        );                                                     \
} while (0)

//
// This macro writes to the modem control register
//
// Arguments:
//
// BaseAddress - A pointer to the address from which the hardware
//               device registers are located.
//
// ModemControl - The control bits to send to the modem control.
//
//
#define WRITE_MODEM_CONTROL(BaseAddress,ModemControl)          \
do                                                             \
{                                                              \
    WRITE_PORT_UCHAR(                                          \
        (BaseAddress)+MODEM_CONTROL_REGISTER,                  \
        (ModemControl)                                         \
        );                                                     \
} while (0)


#define DEVICE_OBJECT_NAME_LENGTH       128
#define SYMBOLIC_NAME_LENGTH            128
#define SERIAL_PNP_ID_STR               L"*PNP0501"
#define SERIAL_PNP_MULTI_ID_STR         L"*PNP0502"
#define SERIAL_DEVICE_MAP               L"SERIALCOMM"

//
// Return values for mouse detection callback
//

#define SERIAL_FOUNDPOINTER_PORT   1
#define SERIAL_FOUNDPOINTER_VECTOR 2

#define SerialCompleteRequest(PDevExt, PIrp, PriBoost) \
   { \
      IoCompleteRequest((PIrp), (PriBoost)); \
      SerialIRPEpilogue((PDevExt)); \
   }

#define SERIAL_WMI_GUID_LIST_SIZE 5

//
// Default delay of 100 msecs for termination delay.
//
#define FT_TERMINATE_DELAY      100

//
// Default 16 msec latency timer.
//
#define FT_DEFAULT_LATENCY_TIMER    16

//
// condition for cancelling write and ioctl requests when powering down
//
#define SerialPoweringDown(Ext) \
    (((Ext)->ConfigDataFlags & FTDI_CD_FLAGS_POFF_CANCEL) && (Ext)->NotD0)

//
// condition for using SERIAL_EV_EVENT2 as surprise removal indicator.
//
#define SerialEnableSurpriseRemovalEvent(Ext) \
    ((Ext)->ConfigDataFlags & FTDI_CD_FLAGS_EV2_AS_REMOVAL)

//
// macro to set surprise removal indicator.
//
#ifndef WINCE
#define SerialSetSurpriseRemovalEvent(Ext) \
    if (((Ext)->IsrWaitMask) \
     && ((Ext)->IsrWaitMask & SERIAL_EV_EVENT2)) \
        (Ext)->HistoryMask |= SERIAL_EV_EVENT2
#endif

//
// condition for using 32 bit baud rate divisors.
//
#define SerialUseBmBaudRates(Ext) \
    ((Ext)->ConfigDataFlags & FTDI_CD_FLAGS_BM_BAUD_RATES)

//
// condition for setting RTS on close.
//
#define SerialSetRTSOnClose(Ext) \
    ((Ext)->ConfigDataFlags & FTDI_CD_FLAGS_SET_RTS_ON_CLOSE)

//
// condition for using asynchronous vendor interface.
//
#define SerialUseAsyncVendorInterface(Ext) \
    ((Ext)->ConfigDataFlags & FTDI_CD_FLAGS_ASYNC_VENDOR_IF)

//
// condition for setting DTR on open.
//
#define SerialSetDTROnOpen(Ext) \
    ((Ext)->ConfigDataFlags & FTDI_CD_FLAGS_SET_DTR_ON_OPEN)


#endif

