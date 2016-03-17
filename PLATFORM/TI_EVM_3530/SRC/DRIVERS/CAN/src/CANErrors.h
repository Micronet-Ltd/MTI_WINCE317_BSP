#ifndef __CANERRORS_H
#define __CANERRORS_H


// Definitions ====================================================================================

// All CAN Specific Errors encoded in one single DWORD value
// There is two kinds of errors: CAN Bus Specific, MCP2515 Driver Specific
// The error status can be obtained by CANGetERROR(), if call to CANGetERROR() succeed
// MCP2515 Driver's internal value of error status is cleared so 
// additional call to CANGetERROR() returns 0 if no more error conditions in CAN or Driver


// ----------- Field Name ----------- Bit ---------- Explanation ----------------------------------

#define		CANERR_ABTF_TXB			1 << 0	// Transmission Abort Flag,
											// conditions for this error explained 
											// in MCP2515 Data Sheet (TXBnCTRL)

#define		CANERR_MLOA_TXB			1 << 1	// Message Lost Arbitration
											// conditions for this error explained 
											// in MCP2515 Data Sheet (TXBnCTRL)

#define		CANERR_TXERR_TXB		1 << 2	// Transmission Error Detected
											// conditions for this error explained 
											// in MCP2515 Data Sheet (TXBnCTRL)

#define		CANERR_OVR_RXB			1 << 3	// Overflow detected in Hardware
											// conditions for this error explained 
											// in MCP2515 Data Sheet (EFLG)

#define		CANERR_BUSOFF			1 << 4	// Bus-Off
											// conditions for this error explained 
											// in MCP2515 Data Sheet (EFLG)

#define		CANERR_PASSIVE_TX		1 << 5	// Error-Passive Transmit
											// conditions for this error explained 
											// in MCP2515 Data Sheet (EFLG)

#define		CANERR_PASSIVE_RX		1 << 6	// Error-Passive Receive
											// conditions for this error explained 
											// in MCP2515 Data Sheet (EFLG)

#define		CANERR_WARNIG_TX		1 << 7	// Error Warning Transmit
											// conditions for this error explained 
											// in MCP2515 Data Sheet (EFLG)

#define		CANERR_WARNIG_RX		1 << 8	// Error Warning Receive
											// conditions for this error explained 
											// in MCP2515 Data Sheet (EFLG)

#define		CANERR_ERRWARNING		1 << 9	// Error Warning
											// conditions for this error explained 
											// in MCP2515 Data Sheet (EFLG)

#define		CANERR_TXTIMEOUT		1 << 10 // Tx Timeout occur
											// WriteFile failed on Timeout

#define		CANERR_RXTIMEOUT		1 << 11 // Rx Timeout occur
											// ReadFile failed on Timeout

#define		CANERR_OVERFLOW			1 << 12 // Rx Queue is full and Data Overflow occurs.
											// Condition for this error is specific to MCP2515 Driver.
											// Internal Rx message queue was full and 
											// new message overrides previous received message.
											// The driver invokes SetLastError(ERROR_INTERNAL_ERROR) once 
											// for the next thread that calls ReadFile(...), and then 
											// resets CANERR_OVERFLOW condition.

#define		CANERR_ABORT			1 << 15 // All the I/O operations has been aborted because of
											// an application request to CloseHandle() for CAN device.
											// All waiting threads that can be released immediately
											// are released.

#define		CANERR_FATALERROR		1 << 31 // FATAL ERROR in Driver
											// indicates general error in driver caused by any hardware
											// failures like SPI interface



#endif //__CANERRORS_H