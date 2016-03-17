#ifndef __CANBDIOCTL_H
#define __CANBDIOCTL_H


// IOCtl ==========================================================================================

// Configures Bit Timing according CAN Bus Specification,
// sets appropriate Baud Rate in Bits/Second
#define IOCTL_CAN_SET_BITTIMING \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 100, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_CAN_GET_BITTIMING \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 110, METHOD_BUFFERED, FILE_ANY_ACCESS )

// ------------------------------------------------------------------------------------------------

// Configures Mask for Rx Filters of specific Rx Buffer,
// achieves additional functionality of Rx Filters in packet filtering
#define IOCTL_CAN_SET_MASK \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 120, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_CAN_GET_MASK \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 130, METHOD_BUFFERED, FILE_ANY_ACCESS )

// ------------------------------------------------------------------------------------------------

// Configures Rx Filters of specific Rx Buffer,
// before incoming packet excepted it should pass this Filters with Mask
#define IOCTL_CAN_SET_FILTERS \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 140, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_CAN_GET_FILTERS \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 150, METHOD_BUFFERED, FILE_ANY_ACCESS )

// ------------------------------------------------------------------------------------------------

// Configures Time Outs for ReadFile/WriteFile functions,
// achieves functionality similar to Serial COM port using
#define IOCTL_CAN_SET_RWTIMEOUT \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 160, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_CAN_GET_RWTIMEOUT \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 170, METHOD_BUFFERED, FILE_ANY_ACCESS )

// ------------------------------------------------------------------------------------------------

// Configure Rx Buffer Operation Mode,
// specifies to MCP2515 how Mask and Filters should be applied on
// Standard ID and Extended ID so only valid messages will be excepted
#define IOCTL_CAN_SET_RXBOPMODE \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 180, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_CAN_GET_RXBOPMODE \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 190, METHOD_BUFFERED, FILE_ANY_ACCESS )

// ------------------------------------------------------------------------------------------------

// Reset to MCP2515,
// by CAN Specification, when multiple errors occur device should 
// switches itself out of the CAN bus, so Reset can be useful...
// After Reset, Driver will fetch Default Settings from Registry and
// applies them to MCP2515
#define IOCTL_CAN_MCP2515_RESET \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 200, METHOD_BUFFERED, FILE_ANY_ACCESS )

// ------------------------------------------------------------------------------------------------

// Get CAN Driver Error Status
#define IOCTL_CAN_GET_ERRORS \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 210, METHOD_BUFFERED, FILE_ANY_ACCESS )

// ------------------------------------------------------------------------------------------------

// Set MCP2515 Operation Mode
#define IOCTL_CAN_SET_OPMODE \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 220, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_CAN_GET_OPMODE \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 230, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_CAN_SET_ECHO \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 240, METHOD_BUFFERED, FILE_ANY_ACCESS )

// ------------------------------------------------------------------------------------------------

#endif //__CANBDIOCTL_H
