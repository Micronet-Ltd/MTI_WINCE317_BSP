#ifndef _USBSER_API_H_
#define _USBSER_API_H_

#include <windows.h>

//==============================================================================================
//		enumerations
//==============================================================================================
typedef enum
{
	USB_SERIAL_DEVICE_TYPE_UNKNOWN = 0,
	
	USB_SERIAL_DEVICE_TYPE_INTERNAL_MODEM,	// for internal devices
	USB_SERIAL_DEVICE_TYPE_INTERNAL_GPS,	// do not use

	USB_SERIAL_DEVICE_TYPE_MODEM = 20,		// USB serial device types
	USB_SERIAL_DEVICE_TYPE_GPS,
	USB_SERIAL_DEVICE_TYPE_SERIAL_PORT

} USB_SERIAL_DEVICE_TYPE;

//==============================================================================================
//     structure definitions
//==============================================================================================

#pragma pack (push, 1)

// USB_SERIAL_DEVICE_INFO struct
typedef struct
{
	UINT8					UsbTier;
    UINT8					UsbPort;
	TCHAR					PortName[8];
	USB_SERIAL_DEVICE_TYPE	DeviceType;
} USB_SERIAL_DEVICE_INFO;

#pragma pack (pop)

//==============================================================================================
//     API functions
//==============================================================================================
#ifdef __cplusplus
	extern "C" {
#endif

	//------------------------------------------------------------------------------
	// Function name	: USB_SER_ENUM_PROC
	// Description	    : This function is an application-defined callback function 
	//					: that receives connected USB serial devices info as a result 
	//					: of a call to the MIC_EnumUsbSerialDevices function.
	//					: An application must register this callback function by 
	//					: passing its address to MIC_EnumUsbSerialDevices.
	//                  : 
	// Return value		: TRUE continues enumeration. FALSE stops enumeration.
	// Arguments        : pInfo: Pointer to USB_SERIAL_DEVICE_INFO structure
	//                  : pUserData: Specifies the application-defined value given in MIC_EnumUsbSerialDevices
    //------------------------------------------------------------------------------
	typedef BOOL (CALLBACK* USB_SER_ENUM_PROC)(USB_SERIAL_DEVICE_INFO *pInfo, LPVOID pUserData);

	//------------------------------------------------------------------------------
	// Function name	: MIC_EnumUsbSerialDevices
	// Description	    : This function enumerates all connected USB serial devices 
	//					: by passing the device info, in turn, to an application-defined 
	//					: callback function. MIC_EnumUsbSerialDevices continues until 
	//					: the last device is enumerated or the callback function returns FALSE.
	//                  : 
	// Return type		: Nonzero indicates success. Zero indicates failure. To get extended 
	//					: error information, call GetLastError
	// Arguments        : lpEnumFunc: Long pointer to an application-defined callback function.
	//                  : pUserData: Specifies an application-defined value to be passed 
	//					: to the callback function.
    //------------------------------------------------------------------------------
	BOOL MIC_EnumUsbSerialDevices(USB_SER_ENUM_PROC lpEnumFunc, LPVOID pUserData);

#ifdef __cplusplus
	}
#endif	

#endif //_USBSER_API_H_