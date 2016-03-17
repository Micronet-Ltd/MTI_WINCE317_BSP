/*++

Copyright (c) 2001-2012  Future Technology Devices International Ltd.

Module Name:

    ftdi_ioctl.h

Abstract:

    Native USB device driver for FTDI FT8U2XX
    Serial port routines

Environment:

    kernel mode only

Revision History:

	24/05/04	st		Adapted from Microsoft sample
	13/08/04	st		Custom Ioctl for the D2XX interface - different values from the D2XX driver.
	23/08/04	st		Added flags for the DTR/RTS settings in the registry as well as the InTransferSize stuff added to the structure.
	25/11/04	st		Added code for the baud rate aliasing.
	09/03/05	st		Added flag to ftdi structure to allow baud rates to be calculated when config data is not present.
	22/03/05	st		Changes to support 232R variable IN endpoint size.
	07/06/05	st		Added function headers for the Purge internal buffer and the notify on removal. 
						Also added members to the device structure to allow support of the notify on removal.
	23/06/05	st		Changes required for modem emuation.
	27/06/05	st		Some additions for placing a sleep in with the BULK In task. Additions for the ActiveSync monitor thread.
	30/06/05	st		Additions to allow completion function to be set for asynchronous/synchronous calls. Addition to allow bulk flags to be set.
	19/09/05	st		Changed descriptions of some structure definitions to be more in line with windows.
						Added a required members to the ftdi_device structure.
	24/01/06	st		Removed LineStatus variable from serial structure.
	05/04/06	st		232R chip detection macros added.
	06/07/07	ma		Added FTDI_INTERFACE_CLASS (needed to create registry keys for CE 5.0).
	04/06/08	ma		Added hi-speed chip support. FT4232H and FT2232H.
	16/10/08	ma		Modified interface setup for FT2232H and FT4232H - port B and greater were selecting wrong interface number.
	21/10/09	ma		Larger buffer for serial number - this led to persistant port not working for some devices.
	03/03/11	ma		Added a surprise remove flag to handle surprise disconnects better (in particular to allow WM_DEVICECHANGE messages to propagate).
						Added support for the FT232H.
	28/10/11	ma		Added EX definitions.
	09/11/11	ma		Added \ characters for multi-line macro.
	21/11/11	ma		Improvements for surprise disconnects.
	04/01/12	ma		Added an event to synchronise the bulk task with open; wait on an event to indicate that the bulk task is ready rather than an arbitrary 16ms.
						Also, if the bulk buffer is still allocated when the bulk task has been forcibly terminated, free the buffer.


--*/

#ifndef ___FT_IOCTL_H__
#define ___FT_IOCTL_H__
#include <windows.h>
#include <devload.h>
#include <usbdi.h>
#include <pegdser.h>
#include <pegdpar.h>
#include <serhw.h>
#include "emul.h"
#include "serial.h"
#include "ftdi_ser.h"
#include <usbclient.h>
#include "usbser_api.h"


#define USB_ENDPOINT_DIRECTION_MASK               0x80

#define USBD_PIPE_DIRECTION_IN(pipeInformation) ((pipeInformation)->EndpointAddress & \
                                                  USB_ENDPOINT_DIRECTION_MASK) 
#define FTDI_RESET_PIPE_RETRIES 50

//
// Maximum transfer size over USB. This is the absolute maximum
// number of bytes per transfer, and for IN requests it includes
// the status bytes. We have to force it to 64k because USB2
// can't handle anything bigger. USB1 can handle bigger transfer
// sizes, but until we find a way to distinguish USB2 controllers,
// 64k will have to do.
//
#define FT_USBD_MAXIMUM_TRANSFER_SIZE   (1024*64)

//
// Default transfer size over USB. This value is required for
// backward compatibility, but the transfer size can be set
// to anything up to FT_USBD_MAXIMUM_TRANSFER_SIZE.
//
#define FT_USBD_DEFAULT_TRANSFER_SIZE   (4096)

//
// Default packet size over USB. This value is standard for
// BULK endpoints.
//
#define FT_USBD_PACKET_SIZE             64
#define FT_USBD_PACKET_SIZE_HI			512
//
// Adjust buffer size to an equivalent size that is optimized for
// maximum throughput. This takes into consideration the status bytes
// that are inserted every 64 bytes. So we always get less than we
// seem to ask for, but an equivalent definition for user code can
// mask this. Using this reduces the number of IN requests over USB.
//
#define FT_SIZE(x, Ext)                 ((x) - (((x)/(Ext)->BulkIn.wMaxPacketSize)*2))

//
// Read buffer optinized for maximum throughput.
//
#define FT_READ_BUFFER_SIZE             FT_SIZE(FT_USBD_MAXIMUM_TRANSFER_SIZE)

#define BULK_BUFFER_SIZE	0x1000		// 4k
#define USER_BUFFER_SIZE	0x10000		// 64k


// vendor command codes
#define FTDI_RESET              0x00
#define FTDI_MODEM_CTRL         0x01
#define FTDI_SET_FLOW_CONTROL   0x02
#define FTDI_SET_BAUD_RATE      0x03
#define FTDI_SET_DATA           0x04
#define FTDI_GET_MODEM_STATUS   0x05
#define FTDI_SET_EVENT_CHAR     0x06
#define FTDI_SET_ERROR_CHAR     0x07

// rev2
#define FTDI_SET_LATENCY_TIMER  0x09
#define FTDI_GET_LATENCY_TIMER  0x0a
#define FTDI_SET_BIT_MODE       0x0b
#define FTDI_GET_BIT_MODE       0x0c

#define FTDI_READ_EE			0x90
#define FTDI_WRITE_EE			0x91
#define FTDI_ERASE_EE			0x92


//
// FTDI Flow Control
//

#define FT_FLOW_NONE            0
#define FT_FLOW_RTS_CTS         0x0100
#define FT_FLOW_DTR_DSR         0x0200
#define FT_FLOW_XON_XOFF        0x0400


//
// Registry settings to ignore and set DTR and initial settings
//
#define DTR_IGNORE_ALL_SET		0x00000001
#define RTS_IGNORE_ALL_SET		0x00000002
#define DTR_INITIAL_STATE_ON	0x00000004
#define RTS_INITIAL_STATE_ON	0x00000008
#define DTR_IGNORE_ON_CLOSE		0x00000010
#define RTS_IGNORE_ON_CLOSE		0x00000020


//
// Purge rx and tx buffers
//
#define FT_PURGE_RX             1
#define FT_PURGE_TX             2


//
// Modem Status Flags
//
#define FT_CTS_ON               0x10
#define FT_DSR_ON               0x20
#define FT_RING_ON              0x40
#define FT_DCD_ON               0x80

//
// BREAK is controlled using the FTDI_SET_DATA request.
// Default data characteristics are 8 data bits, 1 stop bit, no parity.
//
#define FTDI_BREAK_PARAM_DEFAULT    0x0008

//
// BREAK on is bit 14 in wValue parameter of FTDI_SET_DATA request.
//
#define FTDI_BREAK_OFF              0x0000
#define FTDI_BREAK_ON               0x4000

//
// EEROM contents
//

#define FT_MAX_E2DATA_INDEX         63  // WORD INDEX

#define FT_DESCRIPTION_INDEX        16  // BYTE INDEX
#define FT_SERIAL_NUMBER_INDEX      18  // BYTE INDEX


#define FT_MAX_SERIAL_NUMBER_LEN    32  // 8
#define FT_MAX_DESCRIPTION_LEN      64


#define FT_IOCTL_INDEX  0x0200


//
// FTDI Baud Rates
//

enum {
    FTDI_CBR_300,
    FTDI_CBR_600,
    FTDI_CBR_1200,
    FTDI_CBR_2400,
    FTDI_CBR_4800,
    FTDI_CBR_9600,
    FTDI_CBR_19200,
    FTDI_CBR_38400,
    FTDI_CBR_57600,
    FTDI_CBR_115200,
    FTDI_CBR_230400,
    FTDI_CBR_460800,
    FTDI_CBR_921600,
    FTDI_CBR_1843200,
    FTDI_CBR_14400,
    NUM_FTDI_BAUD_RATES
};


#define IOCTL_FT_SET_LATENCY_TIMER      CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                 FT_IOCTL_INDEX+0, \
                                                 METHOD_BUFFERED, \
                                                 FILE_ANY_ACCESS)

#define IOCTL_FT_GET_LATENCY_TIMER      CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                 FT_IOCTL_INDEX+1, \
                                                 METHOD_BUFFERED, \
                                                 FILE_ANY_ACCESS)

#define IOCTL_FT_SET_BIT_MODE			CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                 FT_IOCTL_INDEX+2, \
                                                 METHOD_BUFFERED, \
                                                 FILE_ANY_ACCESS)

#define IOCTL_FT_GET_BIT_MODE			CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                 FT_IOCTL_INDEX+3, \
                                                 METHOD_BUFFERED, \
                                                 FILE_ANY_ACCESS)

#define IOCTL_FT_GET_DEVICE_INFO		CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                 FT_IOCTL_INDEX+4, \
                                                 METHOD_BUFFERED, \
                                                 FILE_ANY_ACCESS)

#define IOCTL_FT_SET_RTS				CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                 FT_IOCTL_INDEX+5, \
                                                 METHOD_BUFFERED, \
                                                 FILE_ANY_ACCESS)

#define IOCTL_FT_CLR_RTS				CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                 FT_IOCTL_INDEX+6, \
                                                 METHOD_BUFFERED, \
                                                 FILE_ANY_ACCESS)

#define IOCTL_FT_SET_DTR				CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                 FT_IOCTL_INDEX+7, \
                                                 METHOD_BUFFERED, \
                                                 FILE_ANY_ACCESS)

#define IOCTL_FT_CLR_DTR				CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                 FT_IOCTL_INDEX+8, \
                                                 METHOD_BUFFERED, \
                                                 FILE_ANY_ACCESS)

#define IOCTL_FT_SET_DEVICE_EVENT		 CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                 FT_IOCTL_INDEX+9, \
                                                 METHOD_BUFFERED, \
                                                 FILE_ANY_ACCESS)

#define IOCTL_FT_GET_DEVICE_EVENT_INFO	 CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                 FT_IOCTL_INDEX+10, \
                                                 METHOD_BUFFERED, \
                                                 FILE_ANY_ACCESS)

// Platform depended
#define IOCTL_FT_GET_SET_DEVICE_TYPE	 CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                 FT_IOCTL_INDEX+20, \
                                                 METHOD_BUFFERED, \
                                                 FILE_ANY_ACCESS)

#define IOCTL_FT_GET_USB_DEVICE_EXT_INFO	 CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                 FT_IOCTL_INDEX+21, \
                                                 METHOD_BUFFERED, \
                                                 FILE_ANY_ACCESS)

// +++ NOTE ANY MORE WILL HAVE TO BE ADDED HERE ON TO AVOID CODE OUT THERE BREAKING



//
//  define struct containing info used for USBD transfer
//

typedef struct _ft_usb_params {
    ULONG MaximumTransferSize;
    ULONG CurrentTransferSize;
} FT_USB_PARAMS, *PFT_USB_PARAMS;

typedef struct _TFtWriteEeParams {
	USHORT Address;
	UCHAR lValue;
	UCHAR hValue;
} TFtWriteEeParams;


//
// This structure is used to set read and write timeouts.
//

typedef struct _TFtTimeouts {
    ULONG ReadTimeout;
    ULONG WriteTimeout;
} TFtTimeouts;


//
// This structure is also used to set read and write timeouts.
//

typedef struct _TFtTimeoutsEx {
    ULONG ReadIntervalTimeout;          /* Maximum time between read chars. */
    ULONG ReadTotalTimeoutMultiplier;   /* Multiplier of characters.        */
    ULONG ReadTotalTimeoutConstant;     /* Constant in milliseconds.        */
    ULONG WriteTotalTimeoutMultiplier;  /* Multiplier of characters.        */
    ULONG WriteTotalTimeoutConstant;    /* Constant in milliseconds.        */
} TFtTimeoutsEx;


//
// This structure used to set line parameters.
//

typedef struct _TFtLineControl {
    UCHAR StopBits;
    UCHAR Parity;
    UCHAR WordLength;
} TFtLineControl;

//
// This structure used to set flow control parameters.
//

typedef struct _TFtFlowControl {
    USHORT Control;
    UCHAR XonChar;
    UCHAR XoffChar;
} TFtFlowControl;

//
// This structure used by IOCTL_FT_SET_BAUD_RATE
//

typedef struct _TFtBaudRate {
    ULONG BaudRate;
} TFtBaudRate;

//
// This structure used by IOCTL_FT_GET_CHARS and IOCTL_FT_SET_CHARS
//

typedef struct _TFtSpecialChars {
    UCHAR EventChar;
    UCHAR EventCharEnabled;
	UCHAR ErrorChar;
    UCHAR ErrorCharEnabled;
    UCHAR XonChar;
    UCHAR XoffChar;
} TFtSpecialChars;

//
// This structure used by IOCTL_FT_SET_EVENT_NOTIFICATION
//

#define FT_EVENT_RXCHAR         1
#define FT_EVENT_MODEM_STATUS   2

typedef struct _TFtEventNotify {
    ULONG Ref;
    ULONG Mask;
    PVOID Param;
} TFtEventNotify;

//
// This structure used by IOCTL_FT_GET_STATUS
//

typedef struct _TFtStatus {
    ULONG RxBytes;
    ULONG TxBytes;
    ULONG EventDWord;
} TFtStatus;

//
// This structure used to set bit mode.
//

typedef struct _TFtBitMode {
    UCHAR Mask;
    UCHAR Enable;
} TFtBitMode;

//
// This structure used to set USB parameters.
//

typedef struct _TFtSetUSBParameters {
    ULONG InTransferSize;
    ULONG OutTransferSize;
} TFtUSBParameters;


//
// This structure used by IOCTL_FT_GET_DEVICE_INFO
//

typedef struct _TFtDeviceInfo {
	ULONG Status;
    ULONG Type;
	ULONG ID;
	PVOID Dummy;
	char SerialNumber[16];
	char Description[64];
} TFtDeviceInfo;

//
// This structure used by Read Requests
//

typedef struct _FTReadRequest {
	BOOL	bReadPending;
    PUCHAR	pBuffer;
	DWORD	dwBufferLength;
	DWORD	dwBytesTransferred;
	HANDLE	hReadBytesSatisfied;
	DWORD	dwCurrPermissions;		// from GetCurrentPermissions

} FTReadRequest;

#define FT_USB_CLOSE_DEVICE		0x00000001

//
// Used by the FT_RegisterNotification routine
//
typedef struct _FTNotifyParams {
    DWORD dwRef;
	DWORD dwMask;
	PVOID pvParam;
} FTNotifyParams;

// device types
#define FT8U232AM	0x40360010
#define FT232BM		0x40360011
#define FT232R      0x40360012
#define FT8U100AX	0x40383720
#define FT2232      0x40360100
#define FT2232H     0x40360101
#define FT4232H     0x40360110
#define FT232H      0x40360140

//
// These structures are used by IOCTL_FT_CLEAR_ERROR
//

typedef struct _FTCOMSTAT {
    ULONG fCtsHold : 1;
    ULONG fDsrHold : 1;
    ULONG fRlsdHold : 1;
    ULONG fXoffHold : 1;
    ULONG fXoffSent : 1;
    ULONG fEof : 1;
    ULONG fTxim : 1;
    ULONG fReserved : 25;
    ULONG cbInQue;
    ULONG cbOutQue;
} FTCOMSTAT, *LPFTCOMSTAT;

typedef struct _TFtClearError {
    ULONG Errors;
    FTCOMSTAT Comstat;
} TFtClearError;

//
// This structure is used by IOCTL_FT_SET_STATE and IOCTL_FT_GET_STATE
//

#define WORD USHORT
#define BYTE UCHAR

typedef struct _FTDCB {
    ULONG DCBlength;      /* sizeof(FTDCB)                   */
    ULONG BaudRate;       /* Baudrate at which running       */
    ULONG fBinary: 1;     /* Binary Mode (skip EOF check)    */
    ULONG fParity: 1;     /* Enable parity checking          */
    ULONG fOutxCtsFlow:1; /* CTS handshaking on output       */
    ULONG fOutxDsrFlow:1; /* DSR handshaking on output       */
    ULONG fDtrControl:2;  /* DTR Flow control                */
    ULONG fDsrSensitivity:1; /* DSR Sensitivity              */
    ULONG fTXContinueOnXoff: 1; /* Continue TX when Xoff sent */
    ULONG fOutX: 1;       /* Enable output X-ON/X-OFF        */
    ULONG fInX: 1;        /* Enable input X-ON/X-OFF         */
    ULONG fErrorChar: 1;  /* Enable Err Replacement          */
    ULONG fNull: 1;       /* Enable Null stripping           */
    ULONG fRtsControl:2;  /* Rts Flow control                */
    ULONG fAbortOnError:1; /* Abort all reads and writes on Error */
    ULONG fDummy2:17;     /* Reserved                        */
    WORD wReserved;       /* Not currently used              */
    WORD XonLim;          /* Transmit X-ON threshold         */
    WORD XoffLim;         /* Transmit X-OFF threshold        */
    BYTE ByteSize;        /* Number of bits/byte, 4-8        */
    BYTE Parity;          /* 0-4=None,Odd,Even,Mark,Space    */
    BYTE StopBits;        /* 0,1,2 = 1, 1.5, 2               */
    char XonChar;         /* Tx and Rx X-ON character        */
    char XoffChar;        /* Tx and Rx X-OFF character       */
    char ErrorChar;       /* Error replacement char          */
    char EofChar;         /* End of Input character          */
    char EvtChar;         /* Received Event character        */
    WORD wReserved1;      /* Fill for now.                   */
} FTDCB, *LPFTDCB;

typedef struct _FT_DEV_CONTEXT {

    ULONG           BaudRate;
    TFtLineControl  LineControl;
    TFtFlowControl  FlowControl;
    TFtSpecialChars SpecialChars;
    TFtBitMode      BitMode;
    UCHAR           LatencyTime;
    UCHAR           MCR;

} FT_DEV_CONTEXT, *PFT_DEV_CONTEXT;

typedef struct _FLAGS {
	UCHAR    Open           : 1; // bits 0
	UCHAR    UnloadPending  : 1; // bits 1
	UCHAR    SurpriseRemoved : 1; // bits 2
	UCHAR    Reserved       : 5; // bits 3-7
} FLAGS, *PFLAGS;

typedef struct _USBTIMEOUTS {
    DWORD PortStatusTimeout;
    DWORD DeviceIdTimeout;
    DWORD SoftResetTimeout;
} USBTIMEOUTS, *PUSBTIMEOUTS;

// Our notion of a Pipe
typedef struct _PIPE {
	USB_PIPE	hPipe;				// USB Pipe handle received from the stack
	UCHAR		EndpointAddress;	// Endpoint's Address
	USHORT		wMaxPacketSize;		// Endpoint's wMaxPacketSize
	HANDLE		hEvent;				// Completion Event
	DWORD		dwBulkFlags;		// Flags for the bulk transfer
	DWORD		(*pCompletionRoutine)(PVOID);	// Pointer to completion Event - to allow configurable synchronous/asynchronous bulk transfers 
} USBD_PIPE_INFORMATION, *PUSBD_PIPE_INFORMATION;

typedef struct _FTREADBUFFER {
    ULONG rbSize;					// size for receive buffer
    ULONG rbPut;					// put index for receive buffer
    ULONG rbGet;					// get index for receive buffer
    ULONG rbBytesFree;				// space remaining in ReadBuffer
	//
	//	A read request size (if it has not been satisfied
	//
	ULONG ReadBytesRequested;

	//
	// Read Event - set when we have got our requested size
	//
	UCHAR * Buffer;					// the buffer
	HANDLE hBufferMutex;			// Mutex for exclusive access to the user buffer

} FTREADBUFFER, *PFTREADBUFFER;

typedef struct _FTAPPEVENTINFO {
	HANDLE AppEvent;				// the Event to signal
	DWORD dwCurrPermissions;		// from GetCurrentPermissions
} FTAPPEVENTINFO, *PFTAPPEVENTINFO;

typedef struct _FTDI_DEVICE {
	ULONG Sig;						// We use a Signature to be sure (Some drivers read 
									// the context from the registry - so this is a safe check)
	CRITICAL_SECTION Lock;			// sync object for this instance
	LPTSTR ActivePath;				// path to HKLM\Drivers\Active\xx
	HANDLE hStreamDevice;			// Handle for Stream interface.
	HANDLE hUsbDevice;				// USB handle to the device
	LPCUSB_FUNCS UsbFuncs;			// USBD Function table
	//
	// Fields from USB_INTERFACE that we need
	//
	UCHAR bInterfaceNumber;
	UCHAR bAlternateSetting;		// ??

	USHORT ConfigIndex;				// ??

	USBD_PIPE_INFORMATION BulkOut;
	USBD_PIPE_INFORMATION BulkIn;
	HANDLE hEP0Event;				// completion signal used for endpoint 0
	HANDLE hCloseEvent;
	HANDLE hReconnectEvent;
	FLAGS Flags;
    
	//
    // Holds the timeout controls for the device.  This value
    // is set by the Ioctl processing.
    //

    TFtTimeouts Timeouts;
    
	//
    // Holds the timeout controls for the device.  This value
    // is set by the Ioctl processing.
    //

    TFtTimeoutsEx TimeoutsEx;


	USBTIMEOUTS UsbTimeouts;

	//
	// BulkIn thread variables
	//
	HANDLE hReaderThread;
	DWORD dwReaderThreadID;
	HANDLE hCloseReaderEvent;			// signal to close the Thread
	HANDLE hReaderConfiguredEvent;		// signal that the Thread has been set up
	FTREADBUFFER ReadBuffer;			// Larger buffer to hold what we have returned from the Bulk
	FTReadRequest ReadRequest;
	BOOL ResetBuffer;					// reset out Larger in buffer

	UCHAR *BulkBuffer;					// pointer to our local bulk data buffer - used to free it if we have to kill the thread!

	FT_DEV_CONTEXT	DevContext;
	USHORT FlowControl;
	USHORT BreakOnParam;

	//
    // Copy of serial device registers
    //

	SERIAL_REGS SerialRegs;	// This replaces ModemStatus

	//
	// So we can tell if there are any changes in modem status for events
	//

    UCHAR OldModemStatus;

	//
    // struct used to hold comms parameters
    //

    FTDCB DCB;
	
    //
    // Communications errors
    //

    ULONG Errors;

	// Our USB interface
	LPCUSB_INTERFACE	pUsbInterface;
	LPCUSB_DEVICE		pUsbDevice;

	//
    // Serial number string 
    //

    CHAR SerialNumber[FT_MAX_SERIAL_NUMBER_LEN+1];

    //
    // Device description string 
    //

    CHAR Description[FT_MAX_DESCRIPTION_LEN+1];

    //
    // Struct used to control event notification
    //

    TFtEventNotify EventNotification;

    //
    // currently signalled events
    //

    ULONG EventMask;
	
    //
    // Struct used to hold special characters
    //

    TFtSpecialChars SpecialChars;

	//
    // Reference for event passed in IOCTL_FT_SET_EVENT_NOTIFICATION.
    //

#ifdef WINCE
	FTAPPEVENTINFO AppEventInfo;
#else
    PKEVENT AppEvent;
#endif
	
    //
    // Miscellaneous flags
    //

    ULONG MiscFlags;

    //
    // current wait event mask
    //

    ULONG WaitMask;
	
	//
	// Close down thread
	//

	BOOLEAN TimeToTerminateThread;

	//
	// Make thread sleep
	//

	BOOLEAN TimeToSleep;

    //
    // true when stop-in-task request is received
    //
    BOOLEAN StopSignalled;

    //
    // true when in-task is waiting after a stop-in-task request is received
    //
    BOOLEAN RestartExpected;

	HANDLE hRestartEvent;		// For the stop in task

    //
    // IN EP USB parameters
    //

    FT_USB_PARAMS   InParams;
	
    //
    // OUT EP USB parameters
    //

    FT_USB_PARAMS   OutParams;

	DWORD dwPID;		// for registry key name

	//
	// Config data for extra special special settings in the registry
	//
	DWORD dwConfigDataFlags;

	/*
	 Not real easy to implement this in the driver as the upper mdd.c has no access to our lower private
	 variables (in the ftdi structure). It is in the COM_Write function that calculates the timeouts.
	*/
//	DWORD dwMinWriteTimeOut;

//	DWORD dwMinReadTimeOut;

	//
	// Bulk In Transfer Size
	//
	DWORD dwInTransferSize;
	//
	// Bulk Out Transfer Size
	//
	DWORD dwOutTransferSize;

	//
    // Copy of flags from ConfigData struct
    //
    ULONG ConfigDataFlags;

	//
    // Baud rate table
    //
    ULONG BaudRates[NUM_FTDI_BAUD_RATES];
	BOOLEAN bIsValidBaudRateTable;			// do we have one in the registry

	//
	// For our FT_RegisterNotifyRoutine Params
	//
	CRITICAL_SECTION NotifyLock;

	//
	// Current Permissions so we can call the function pointer
	//
	DWORD dwNotifyPermissions;

	//
	// used when the user calls FT_GetDeviceEventInfo
	//
	DWORD dwCurrentNotifyStatus;

	//
	// our FT_RegisterNotifyRoutine Params
	//
	FTNotifyParams Notify;

    //
    // DeferModemUpdate set TRUE in BULK_IN to defer modem status handling
    // until read data has been processed.
    //
    BOOLEAN DeferModemUpdate;

    //
    // This is only accessed at interrupt level.  It keeps track
    // of whether the holding register is empty.
    //
    BOOLEAN HoldingEmpty;

	//
    // Emulation mode variables.
    //
    FTEMUL EmulVars;

	//
    // This variable is only accessed at interrupt level.  Whenever
    // a wait is initiated this variable is set to false.
    // Whenever any kind of character is written it is set to true.
    // Whenever the write queue is found to be empty the code that
    // is processing that completing irp will synchonize with the interrupt.
    // If this synchronization code finds that the variable is true and that
    // there is a wait on the transmit queue being empty then it is
    // certain that the queue was emptied and that it has happened since
    // the wait was initiated.
    //
    BOOLEAN EmptiedTransmit;

	//
	// Our current device head - to allow us to set the interrupt flags
	//
	PSER_INFO pHWHead;

	//
	// Our bulk in sleep function pointer - possible fix for Thales and phillips crap host
	//
	void (*mySleep)(DWORD);
	DWORD dwSleepAfterBulkIn;

	//
	// ActiveSync monitoring thread related variables
	//
	UCHAR ActiveSyncMonitorBit;
	DWORD ActiveSyncSleep;
	BOOL bActiveSyncKeepRunning;
	HANDLE hActiveSyncThread;

	//
	// Required for the UnloadPending flag to reject Access on an unplug
	//
	PVOID pMddHead;

	//
    // Number of retries allowed for reset pipe request.
    //

    ULONG ResetPipeRetries;

	//
	// For surprise unplug / recurrant unplug loop catch
	//
	DWORD dwGeneralFailure;

	HANDLE hVspDevice;
	USB_SERIAL_DEVICE_INFO usbDeviceInfo;
} FTDI_DEVICE, *PFTDI_DEVICE;

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
#define FTDI_BULK_IN_ON_GEN_FAILURE				    0x00000200
#define FTDI_PERSISTANT_DEVICE_INSTANCE			    0x00000400

//
// FTDI config data defines ConfigData field in Registry
//
typedef struct _FTDI_CONFIG_DATA {
    struct {
        ULONG Flags;
        UCHAR BaudRates[NUM_FTDI_BAUD_RATES*sizeof(ULONG)];
    } ConfigData;
    ULONG RxBuffer;
    ULONG TxBuffer;
    ULONG MinReadTimeout;
    ULONG MinWriteTimeout;
    ULONG TerminateDelay;
    ULONG LatencyTimer;
    ULONG EmulationMode;
} FTDI_CONFIG_DATA;

//
// condition for using 32 bit baud rate divisors.
//
#define SerialUseBmBaudRates(Ext) \
    ((Ext)->ConfigDataFlags & FTDI_CD_FLAGS_BM_BAUD_RATES)

//
// IOCTL_FT_SET_EVENT_MASK, IOCTL_FT_WAIT_EVENT
//

#define D2XX_EV_RXCHAR          0x0001  // Any Character received
#define D2XX_EV_RXFLAG          0x0002  // Received certain character
#define D2XX_EV_TXEMPTY         0x0004  // Transmitt Queue Empty
#define D2XX_EV_CTS             0x0008  // CTS changed state
#define D2XX_EV_DSR             0x0010  // DSR changed state
#define D2XX_EV_RLSD            0x0020  // RLSD changed state
#define D2XX_EV_BREAK           0x0040  // BREAK received
#define D2XX_EV_ERR             0x0080  // Line status error occurred
#define D2XX_EV_RING            0x0100  // Ring signal detected


//
// Error Flags
//

#define CE_RXOVER           0x0001  // Receive Queue overflow
#define CE_OVERRUN          0x0002  // Receive Overrun Error
#define CE_RXPARITY         0x0004  // Receive Parity Error
#define CE_FRAME            0x0008  // Receive Framing error
#define CE_BREAK            0x0010  // Break Detected
#define CE_TXFULL           0x0100  // TX Queue is full
#define CE_PTO              0x0200  // LPTx Timeout
#define CE_IOE              0x0400  // LPTx I/O Error
#define CE_DNS              0x0800  // LPTx Device not selected
#define CE_OOP              0x1000  // LPTx Out-Of-Paper
#define CE_MODE             0x8000  // Requested mode unsupported

#ifdef __cplusplus
extern "C" {
#endif

SHORT FT_GetDivisor(
    ULONG rate,
    USHORT *divisor,
    USHORT *ext_div,
    ULONG *actual,
    SHORT *accuracy,
    SHORT *plus,
    ULONG bm
    );

SHORT FT_GetDivisorHi(
    ULONG rate,
    USHORT *divisor,
    USHORT *ext_div,
    ULONG *actual,
    SHORT *accuracy,
    SHORT *plus
    );

DWORD
FT_VendorRequest(
	UCHAR RequestCode,
	USHORT wValue,
	USHORT wIndex,
	PVOID Buffer,
	USHORT Length,
	ULONG TransferFlags,
    PFTDI_DEVICE pUsbFTDI
    );

DWORD
FT_SetBaudRate(
    PFTDI_DEVICE pUsbFTDI,
    ULONG BaudRate
    );

DWORD
FT_SetDivisor(
    PFTDI_DEVICE pUsbFTDI,
    USHORT Divisor
    );

DWORD
FT_SetLineControl(
    PFTDI_DEVICE pUsbFTDI,
    TFtLineControl *LineControl
    );

DWORD
FT_SetFlowControl(
    PFTDI_DEVICE pUsbFTDI,
    TFtFlowControl *FlowControl
    );

DWORD
FT_SetDtr(
	PSER_INFO pHWHead
    );

DWORD
FT_ClrDtr(
	PSER_INFO pHWHead
    );

DWORD
FT_SetRts(
	PSER_INFO pHWHead
    );

DWORD
FT_ClrRts(
	PSER_INFO pHWHead
    );

DWORD
FT_SetChars(
    PFTDI_DEVICE pUsbFTDI,
    TFtSpecialChars *SpecialChars
    );

DWORD
FT_GetModemStatus(
    PFTDI_DEVICE pUsbFTDI
    );

VOID
FT_PurgeInternalBuffer(
	PFTDI_DEVICE pUsbFTDI
    );

DWORD
FT_Purge(
    PFTDI_DEVICE pUsbFTDI,
    ULONG Mask
    );

DWORD
FT_SetBreak(
    PFTDI_DEVICE pUsbFTDI,
    USHORT OnOrOff
    );


// rev2

DWORD
FT_SetLatencyTimer(
    PFTDI_DEVICE pUsbFTDI,
    UCHAR LatencyTime
    );

DWORD
FT_GetLatencyTimer(
    PFTDI_DEVICE pUsbFTDI,
    PUCHAR pTimer 
    );

DWORD
FT_SetBitMode(
    PFTDI_DEVICE pUsbFTDI,
    TFtBitMode *BitMode
    );

DWORD
FT_GetBitMode(
    PFTDI_DEVICE pUsbFTDI,
    PUCHAR pMode 
    );

PCHAR
FT_StrCpy(
    PCHAR d,
    PCHAR s
    );

ULONG
FT_StrLen(
    PCHAR s
    );

DWORD
FT_GetDeviceSerialNumber(
	PFTDI_DEVICE pUsbFTDI
	);
								
DWORD
FT_GetDeviceDescription(
	PFTDI_DEVICE pUsbFTDI
	);

VOID
FT_CompleteReadIrp(
    PFTDI_DEVICE pUsbFTDI
    );


PCHAR
FT_CopyWStrToStr(
    PCHAR Dest,
    USHORT DestLen,
    PUSHORT WStr,
    USHORT Len
    );

void StringToWchar(PWCHAR pDest, char * pSrc, ULONG Len);

void
FT_ProcessBulkInEx(
    PSER_INFO pHWObj,
    UCHAR *Buffer,
    ULONG BufSiz,
    ULONG UsbPacketSize
    );

BOOL
CleanupOpenDevice(
	PFTDI_DEVICE pUsbFTDI
	);

DWORD
FT_SetState(
	PFTDI_DEVICE pUsbFTDI
    );

DWORD
FT_ResetHardware(
	PFTDI_DEVICE pUsbFTDI
	);

DWORD
FT_SetDeviceEvent(
	PFTDI_DEVICE pUsbFTDI,
	FTNotifyParams * pNotifyParams
	);

VOID
FT_ProcessRead(
    PFTDI_DEVICE pUsbFTDI,
    int Irp
    );

VOID
FT_InitializeBulkInStopCount(
    PFTDI_DEVICE pUsbFTDI
    );

VOID
FT_IncrementBulkInStopCount(
    PFTDI_DEVICE pUsbFTDI
    );

BOOL
FT_DecrementBulkInStopCount(
    PFTDI_DEVICE pUsbFTDI
    );

DWORD
ReadRequest(
   PFTDI_DEVICE pUsbFTDI,
   PUCHAR pBuffer,
   ULONG  BufferLength
   );

DWORD
IoErrorHandler(
   PFTDI_DEVICE pUsbPrn,
   PUSBD_PIPE_INFORMATION hPipe,
   UCHAR EndpointAddress,
   USB_ERROR dwUsbError
   );

#ifdef __cplusplus
}
#endif

#ifdef WINCE
DWORD WINAPI
#else
void
#endif
BulkInTask(
    IN PVOID Context
    );
//
// Port A and B interface definitions
//
//#define FT2232_INTERFACE_A                  1
//#define FT2232_INTERFACE_A_SUFFIX           "A"
//#define FT2232_INTERFACE_A_PRODUCT_SUFFIX   " " FT2232_INTERFACE_A_SUFFIX

//#define FT2232_INTERFACE_B                  2
//#define FT2232_INTERFACE_B_SUFFIX           "B"
//#define FT2232_INTERFACE_B_PRODUCT_SUFFIX   " " FT2232_INTERFACE_B_SUFFIX

//
// FT2232C/FT2232H/FT4232H
//

//
// Interface definitions for ports A,B,C,D
//
#define FT2232_INTERFACE_A					1
#define FT2232_INTERFACE_A_SUFFIX			"A"
#define FT2232_INTERFACE_A_PRODUCT_SUFFIX	" " FT2232_INTERFACE_A_SUFFIX

#define FT2232_INTERFACE_B					2
#define FT2232_INTERFACE_B_SUFFIX			"B"
#define FT2232_INTERFACE_B_PRODUCT_SUFFIX	" " FT2232_INTERFACE_B_SUFFIX

#define FT2232_INTERFACE_C					3
#define FT2232_INTERFACE_C_SUFFIX			"C"
#define FT2232_INTERFACE_C_PRODUCT_SUFFIX	" " FT2232_INTERFACE_C_SUFFIX

#define FT2232_INTERFACE_D					4
#define FT2232_INTERFACE_D_SUFFIX			"D"
#define FT2232_INTERFACE_D_PRODUCT_SUFFIX	" " FT2232_INTERFACE_D_SUFFIX


//
// macro to identify FT232EX
//
// FT232EX device if bcdDevice == 0x1000
//
#define BCD_FT232EX			0x1000
#define IS_FT232EX(Ext)		(((Ext)->pUsbDevice->Descriptor.bcdDevice & 0xFF00) == 0x1000)

//
// macro to identify FT232H
//
// FT232H device if bcdDevice == 0x0900
//
#define BCD_FT232H			0x0900
#define IS_FT232H(Ext)	(((Ext)->pUsbDevice->Descriptor.bcdDevice & 0xFF00) == 0x0900)


//
// macro to identify FT4232H
//
// FT4232H device if bcdDevice == 0x0800
//
#define BCD_FT4232H			0x0800
#define IS_FT4232H(Ext)	(((Ext)->pUsbDevice->Descriptor.bcdDevice & 0xFF00)  == 0x0800)

//
// macro to identify FT2232H
//
// FT2232H device if bcdDevice == 0x0700
//
#define BCD_FT2232H			0x0700
#define IS_FT2232H(Ext)	(((Ext)->pUsbDevice->Descriptor.bcdDevice & 0xFF00)  == 0x0700)

//
// macro to identify FT232R (rev6 device)
//
// FT232R device if bcdDevice == 0x0600
//
#define BCD_FT232R			0x0600
#define IS_FT232R(Ext)   (((Ext)->pUsbDevice->Descriptor.bcdDevice & 0xFF00)  == 0x0600)

//
// macro to identify 2232 device
//
// 2232 device if bcdDevice == 0x0500
//
#define BCD_FT2232			0x0500
#define IS_FT2232(Ext) (((Ext)->pUsbDevice->Descriptor.bcdDevice & 0xFF00)  == 0x0500)

//
// macro to identify rev4 device
//
// rev4 device if bcdDevice == 0x0400
//           or bcdDevice == 0x0200 AND iSerialNumber == 0
//
#define BCD_FT232B			0x0400
#define IS_FT232B(Ext)   ((((Ext)->pUsbDevice->Descriptor.bcdDevice & 0xFF00)  == 0x0400) || \
                               (((Ext)->pUsbDevice->Descriptor.bcdDevice & 0xFF00)  == 0x0200 &&  \
                                (Ext)->pUsbDevice->Descriptor.iSerialNumber == 0))

//
// macro to identify BM device
//
// BM device if 2232 or REV4
//
#define IS_BM_DEVICE(Ext)   (IS_FT232B((Ext)) || IS_FT232R((Ext)) || IS_FT2232((Ext)) \
								|| IS_FT2232H((Ext)) || IS_FT4232H((Ext)) || IS_FT232H((Ext)) \
								|| IS_FT232EX((Ext)))


//
// macro to identify multi-interface devices
//
#define IS_MULTI_IF_DEVICE(Ext)	(IS_FT2232((Ext)) || IS_FT2232H((Ext)) || IS_FT4232H((Ext)))

//
// macro to identify hi-speed capable devices
//
#define IS_HI_SPEED_DEVICE(Ext)	(IS_FT2232H((Ext)) || IS_FT4232H((Ext)) || IS_FT232H((Ext)))


//
// macro to detect FT8U232AM
//
// AM device if bcdDevice == 0x0200 AND iSerialNumber != 0
//
#define BCD_FT8U232AM			0x0200
#define IS_FT8U232AM(Ext)       (((Ext)->pUsbDevice->Descriptor.bcdDevice & 0xFF00) == 0x0200 && \
                             (Ext)->pUsbDevice->Descriptor.iSerialNumber != 0)


//
//
// 232 device if bcdDevice >= 0x0200
//
#define IS_FT8U232(Ext)			((Ext)->pUsbDevice->Descriptor.bcdDevice >= 0x0200)

//
// macro to detect FT8U232AM
//
#define IS_FT8U100AX(Ext)       ((Ext)->pUsbDevice->Descriptor.bcdDevice == 0x0001)

//
// miscellaneous flags
//
#define D2XX_MF_USE_CLASSIC_EVENTS  1
#define D2XX_MF_SURPRISE_REMOVED    2
#define D2XX_MF_BULK_IN_WAITING     4


//
// macro determines if we are using Classic event handling
//
#define USING_CLASSIC_EVENTS(Ext)   \
    (((Ext)->MiscFlags & D2XX_MF_USE_CLASSIC_EVENTS) != 0)

//
// macro determines if we have been surprise removed
//
#define BEEN_SURPRISE_REMOVED(Ext)   \
    (((Ext)->MiscFlags & D2XX_MF_SURPRISE_REMOVED) != 0)

//
// macro determines if BULK IN is in its wait state
//
#define IS_BULK_IN_WAITING(Ext)   \
    (((Ext)->MiscFlags & D2XX_MF_BULK_IN_WAITING) != 0)

#define USB_COM_SIG		'IDTF'


#define VALID_CONTEXT( p ) \
	( p && USB_COM_SIG == p->Sig )

#define ACCEPT_IO( p ) \
   ( VALID_CONTEXT( p ) && \
     p->Flags.Open && \
    !p->Flags.UnloadPending && \
    !p->Flags.SurpriseRemoved )

#define DRIVER_NAME   TEXT("FTDI_SER.DLL")

#define FTDI_VENDOR_ID		0x0403
#define FTDI_PRODUCT_ID		0x6001

// Needed to create keys for loading properly in CE 5.0?
#define FTDI_DEVICE_CLASS			0x0000
#define FTDI_INTERFACE_CLASS		0x00FF
#define FTDI_INTERFACE_SUBCLASS		0x00FF
#define FTDI_INTERFACE_PROTOCOL		0x00FF

//
// these should be removed in the code if you can 'g' past these successfully
//
#define TEST_TRAP() { \
   NKDbgPrintfW( TEXT("%s: Code Coverage Trap in: ftdi_ser, Line: %d\n"), DRIVER_NAME, TEXT(__FILE__), __LINE__); \
   DebugBreak();  \
}


#define DUMP_USB_DEVICE_DESCRIPTOR( d ) { \
   DEBUGMSG1( ZONE_INIT, (TEXT("USB_DEVICE_DESCRIPTOR:\n"))); \
   DEBUGMSG1( ZONE_INIT, (TEXT("----------------------\n"))); \
   DEBUGMSG2( ZONE_INIT, TEXT("bLength: 0x%x\n"), d.bLength );   \
   DEBUGMSG2( ZONE_INIT, TEXT("bDescriptorType: 0x%x\n"), d.bDescriptorType );  \
   DEBUGMSG2( ZONE_INIT, TEXT("bcdUSB: 0x%x\n"), d.bcdUSB );  \
   DEBUGMSG2( ZONE_INIT, TEXT("bDeviceClass: 0x%x\n"), d.bDeviceClass );  \
   DEBUGMSG2( ZONE_INIT, TEXT("bDeviceSubClass: 0x%x\n"), d.bDeviceSubClass );  \
   DEBUGMSG2( ZONE_INIT, TEXT("bDeviceProtocol: 0x%x\n"), d.bDeviceProtocol );  \
   DEBUGMSG2( ZONE_INIT, TEXT("bMaxPacketSize0: 0x%x\n"), d.bMaxPacketSize0 );  \
   DEBUGMSG2( ZONE_INIT, TEXT("idVendor: 0x%x\n"), d.idVendor ); \
   DEBUGMSG2( ZONE_INIT, TEXT("idProduct: 0x%x\n"), d.idProduct );  \
   DEBUGMSG2( ZONE_INIT, TEXT("bcdDevice: 0x%x\n"), d.bcdDevice );  \
   DEBUGMSG2( ZONE_INIT, TEXT("iManufacturer: 0x%x\n"), d.iManufacturer );   \
   DEBUGMSG2( ZONE_INIT, TEXT("iProduct: 0x%x\n"), d.iProduct ); \
   DEBUGMSG2( ZONE_INIT, TEXT("iSerialNumber: 0x%x\n"), d.iSerialNumber );   \
   DEBUGMSG2( ZONE_INIT, TEXT("bNumConfigurations: 0x%x\n"), d.bNumConfigurations );  \
   DEBUGMSG1( ZONE_INIT, (TEXT("\n")));  \
}

#define DUMP_USB_CONFIGURATION_DESCRIPTOR( c ) { \
   DEBUGMSG1( ZONE_INIT, (TEXT("USB_CONFIGURATION_DESCRIPTOR:\n"))); \
   DEBUGMSG1( ZONE_INIT, (TEXT("-----------------------------\n"))); \
   DEBUGMSG2( ZONE_INIT, TEXT("bLength: 0x%x\n"), c.bLength ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bDescriptorType: 0x%x\n"), c.bDescriptorType ); \
   DEBUGMSG2( ZONE_INIT, TEXT("wTotalLength: 0x%x\n"), c.wTotalLength ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bNumInterfaces: 0x%x\n"), c.bNumInterfaces ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bConfigurationValue: 0x%x\n"), c.bConfigurationValue ); \
   DEBUGMSG2( ZONE_INIT, TEXT("iConfiguration: 0x%x\n"), c.iConfiguration ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bmAttributes: 0x%x\n"), c.bmAttributes ); \
   DEBUGMSG2( ZONE_INIT, TEXT("MaxPower: 0x%x\n"), c.MaxPower ); \
   DEBUGMSG1( ZONE_INIT, (TEXT("\n"))); \
}

#define DUMP_USB_INTERFACE_DESCRIPTOR( i, _index ) { \
   DEBUGMSG1( ZONE_INIT, (TEXT("USB_INTERFACE_DESCRIPTOR[%d]:\n"), _index )); \
   DEBUGMSG1( ZONE_INIT, (TEXT("-------------------------\n"))); \
   DEBUGMSG2( ZONE_INIT, TEXT("bLength: 0x%x\n"), i.bLength ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bDescriptorType: 0x%x\n"), i.bDescriptorType ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bInterfaceNumber: 0x%x\n"), i.bInterfaceNumber ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bAlternateSetting: 0x%x\n"), i.bAlternateSetting ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bNumEndpoints: 0x%x\n"), i.bNumEndpoints ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bInterfaceClass: 0x%x\n"), i.bInterfaceClass ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bInterfaceSubClass: 0x%x\n"), i.bInterfaceSubClass ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bInterfaceProtocol: 0x%x\n"), i.bInterfaceProtocol ); \
   DEBUGMSG2( ZONE_INIT, TEXT("iInterface: 0x%x\n"), i.iInterface ); \
   DEBUGMSG1( ZONE_INIT, (TEXT("\n"))); \
}

#define DUMP_USB_ENDPOINT_DESCRIPTOR( e ) { \
   DEBUGMSG1( ZONE_INIT, (TEXT("USB_ENDPOINT_DESCRIPTOR:\n"))); \
   DEBUGMSG1( ZONE_INIT, (TEXT("-----------------------------\n"))); \
   DEBUGMSG2( ZONE_INIT, TEXT("bLength: 0x%x\n"), e.bLength ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bDescriptorType: 0x%x\n"), e.bDescriptorType ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bEndpointAddress: 0x%x\n"), e.bEndpointAddress ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bmAttributes: 0x%x\n"), e.bmAttributes ); \
   DEBUGMSG2( ZONE_INIT, TEXT("wMaxPacketSize: 0x%x\n"), e.wMaxPacketSize ); \
   DEBUGMSG2( ZONE_INIT, TEXT("bInterval: 0x%x\n"), e.bInterval );\
   DEBUGMSG1( ZONE_INIT, (TEXT("\n"))); \
}

#define FT_MCR_DTR  0x01
#define FT_MCR_RTS  0x02


#define MANUAL_RESET_EVENT TRUE
#define AUTO_RESET_EVENT   FALSE



//
// Default Timeout values
//
#define GET_PORT_STATUS_TIMEOUT		2000
#define GET_DEVICE_ID_TIMEOUT		2000
//#define SOFT_RESET_TIMEOUT			2000
#define SOFT_RESET_TIMEOUT			20

#define READ_TIMEOUT_INTERVAL		250
#define READ_TIMEOUT_MULTIPLIER		10
#define READ_TIMEOUT_CONSTANT		100
//#define WRITE_TIMEOUT_MULTIPLIER	50
//#define WRITE_TIMEOUT_CONSTANT		1000

#define USB_ERROR  DWORD
#define PUSB_ERROR LPDWORD


#define PSERIAL_DEVICE_EXTENSION PFTDI_DEVICE

#ifdef __cplusplus
extern "C" {
#endif

ULONG
SerialHandleModemUpdate(
    PSERIAL_DEVICE_EXTENSION Extension,
    BOOLEAN DoingTX
    );

UCHAR
SerialProcessLSR(
    PSERIAL_DEVICE_EXTENSION Extension
    );

VOID
SerialPutChar(
    PSERIAL_DEVICE_EXTENSION Extension,
    UCHAR CharToPut
    );

/* emul.c */

BOOL
FT_InitEmulMode(
    PFTEMUL Emul,
    ULONG   SubMode,
	ULONG	BufferSize
    );

VOID
FT_InitEmulChars(
    PFTEMUL Emul,
    UCHAR Esc,
    UCHAR Xoff,
    UCHAR Xon
    );

VOID
FT_EmulProcessRxPacket(
    PSER_INFO pSerInfo,
    PUCHAR                      Buffer,
    ULONG                       BufSiz
    );

ULONG
FT_EmulCopyTxBytes(
    PUCHAR                      Dest,
    PUCHAR                      Srce,
    PULONG                      MaxChars,
    ULONG                       MaxTransferLen,
    PSERIAL_DEVICE_EXTENSION    DeviceExtension
    );

ULONG
FT_EmulBuildModemCtrlRequest(
    PUCHAR                      Buffer,
    ULONG                       BufferLen,
    ULONG                       RequestType,
    PSERIAL_DEVICE_EXTENSION    DeviceExtension
    );

//
// ActiveSync monitor
//
VOID 
ConfigureActiveSyncMonitor(
		PFTDI_DEVICE pDevice
		);

VOID TerminateActiveSyncMonitor(
		PFTDI_DEVICE pUsbFTDI
		);

//
// For configurable synchronous/asynchronous commands
//
DWORD
FTDIReadTransferComplete(
   PVOID    Context
   );

DWORD
FTDIWriteTransferComplete(
   PVOID    Context
   );

//
// Device unloading function
//
VOID
SetDeviceUnloading(
	PFTDI_DEVICE pUsbFTDI
	);

VOID
SetDeviceLoading(
	PFTDI_DEVICE pUsbFTDI
	);
//
// After an unplug/replug or power down/up to restore the settings
//
BOOL
FT_RestoreDeviceSettings(
	PFTDI_DEVICE pUsbFTDI
	);

NTSTATUS
LResetPipe(
    PFTDI_DEVICE       DeviceExtension,
    PUSBD_PIPE_INFORMATION  PipeInfo
    );

BOOL WINAPI 
DeviceNotify(
   LPVOID lpvNotifyParameter,
   DWORD dwCode,
   LPDWORD * dwInfo1,
   LPDWORD * dwInfo2,
   LPDWORD * dwInfo3,
   LPDWORD * dwInfo4
   );

#ifdef __cplusplus
}
#endif

#endif
