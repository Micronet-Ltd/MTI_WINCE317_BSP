#ifndef _OWR_API_H_
#define _OWR_API_H_

#include <windows.h>



//==============================================================================================
//            local definitions
//==============================================================================================


#define OWR_QUEUE_NAME_LEN	                    64	           		// bytes


//==============================================================================================
//            errors definitions
//==============================================================================================

#define OWR_OK                               0

// common errors
#define OWR_INTERNAL_ERROR                    10  // Internal error
#define OWR_ERROR_INVALID_PARAMETER           11
#define OWR_ERROR_INVALID_DEVICE              12  // Internal error
#define OWR_ERROR_INVALID_IO_PARAMETER        13  // Internal error
#define OWR_ERROR_READ_FAULT                  14
#define OWR_ERROR_ACCESS_DENIED               15  // access cannot be granted (some other application got private access)
#define OWR_ERROR_ALREDY_REGISTERED           16
#define OWR_ERROR_INVALID_ACCESS              17  // Invalid rule for PUBLIC/PRIVATE access changing
#define OWR_ERROR_CLOSED_HANDLE               18
#define OWR_ERROR_NOTIFICATION                19  // MIC_OneWireWaitForEvent internal error
#define OWR_ERROR_ALREDY_UNREGISTERED         20  


//==============================================================================================
//     enumerations
//==============================================================================================
typedef enum
{
    OWR_I_BUTTON_DEATTACH = 0,
	OWR_I_BUTTON_ATTACH = 1,
	OWR_I_BUTTON_ERROR = 2,

} OWR_EVENT_TYPE;


//==============================================================================================
//     structure definitions
//==============================================================================================
//------------------------------------------------------------------------------

typedef struct _OWR_INPUT_CONTEXT
{
  UINT32     size;               // IN size of structure. Must be filled by user 
  UINT32     oneWireDatalength;  // IN: The length of One-Wire device data object
  UINT32     oneWireEventType;   // OUT: iButton Attach(1) or De-attach(0) event 
  UINT32     opErrorCode;        // OUT: operation error code

} OWR_INPUT_CONTEXT, * POWR_INPUT_CONTEXT;



//------------------------------------------------------------------------------

typedef struct _oneWire_ROM_Id_t
{
  BYTE    familyCode;
  BYTE    SerialNumber[6];
  BYTE    CRC;

} ONEWIRE_ROM_ID_T;


// Each member is null-terminated ('\0') ASCII string, consists of hexadecimal 
//digit characters ['0'-'9''A'-'F'].
typedef struct _oneWire_ROM_Id_ASCII_t
{
  BYTE    familyCode[3];
  BYTE    SerialNumber[13];
  BYTE    CRC[3];

} ONEWIRE_ROM_ID_ASCII_T;



#ifdef __cplusplus
	extern "C" {
#endif

// ================================= General functions====================================

//------------------------------------------------------------------------------
// Function name	: MIC_OWROpen
// Description	    : This function returns handle for safe access to One-Wire. This handle 
//                    must be used for all One-Wire operations.  This API supports multiple  
//                    accesses. Each time when this function is called, it returns valid handle.
// Return type		: Pointer to valid handle if succeeded, INVALID_HANDLE_VALUE if failed.  
//------------------------------------------------------------------------------

HANDLE  MIC_OWROpen(VOID);


//------------------------------------------------------------------------------
// Function name	: MIC_OWRClose
// Description	    : This function destroys the given handle.
// Return type		: TRUE if succeeded, FALSE if failed
// Argument         : handle – valid handle from XXX_OWROpen;
//------------------------------------------------------------------------------

BOOL  MIC_OWRClose(HANDLE handle);



//------------------------------------------------------------------------------
// Function name	: MIC_OneWireRead
// Description	    : Read 1-Wire data. 
//                   In case of iButton, it reads its ROM (8-byte) value
//                   In case of temp sensor, it reads the temperature data

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

// Arguments         : handle – valid handle from MIC_OWROpen;
//                   : context  is of POWR_INPUT_CONTEXT type, where:
//                          : IN: oneWireDatalength ( sizeof(ONEWIRE_ROM_ID_T) )
//                          : OUT: opErrorCode. Possible return codes for this function:
//
//                          OWR_OK 
//                          OWR_ERROR_INVALID_PARAMETER
//                          OWR_ERROR_INVALID_DEVICE
//                          OWR_ERROR_INVALID_IO_PARAMETER
//                          OWR_ERROR_READ_FAULT
//                          OWR_ERROR_ACCESS_DENIED
//                    : OUT: 'pOneWireData' - pointer to 'ONEWIRE_ROM_ID_T' object 
//------------------------------------------------------------------------------

BOOL  MIC_OneWireRead(HANDLE handle, POWR_INPUT_CONTEXT pInputContext, LPVOID pOneWireData );



//------------------------------------------------------------------------------
// Function name	: MIC_OneWireReadASCII
// Description	    : Read 1-Wire data. 
//                   In case of iButton, it reads its ROM (8-byte) value
//                   This function read iButton number as strings, see structure ' ONEWIRE_ROM_ID_ASCII_T'

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

// Arguments         : handle – valid handle from MIC_OWROpen;
//                   : context  is of POWR_INPUT_CONTEXT type, where:
//                          : IN: oneWireDatalength ( sizeof(ONEWIRE_ROM_ID_ASCII_T) )
//                          : OUT: opErrorCode. Possible return codes for this function:
//
//                          OWR_OK 
//                          OWR_ERROR_INVALID_PARAMETER
//                          OWR_ERROR_INVALID_DEVICE
//                          OWR_ERROR_INVALID_IO_PARAMETER
//                          OWR_ERROR_READ_FAULT
//                          OWR_ERROR_ACCESS_DENIED
//                    : OUT: 'pOneWireData' - pointer to 'ONEWIRE_ROM_ID_ASCII_T' object 
//------------------------------------------------------------------------------

BOOL  MIC_OneWireReadASCII(HANDLE handle, POWR_INPUT_CONTEXT pInputContext, LPVOID pOneWireData );



//------------------------------------------------------------------------------
// Function name	: MIC_OneWireEventRegistering
// Description	    : This function permit event notification for process with given handle.. 

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

//Parameters:         handle – valid handle from MIC_OneWireOpen;
//                    pInputContext – pointer to ONE_WIRE_INPUT_CONTEXT  structure, which filled the 'size' by user. Other fields are not relevant here.
//                    BOOL needToRegister – TRUE for Register, FALSE - unregister
//                    'opErrorCode' member filled by Driver with appropriate error code if error occur.
//                    Possible return codes for this function:

//                     OWR_OK
//                     OWR_ERROR_ALREDY_REGISTERED
//                     OWR_ERROR_INVALID_ACCESS
//                     OWR_ERROR_ACCESS_DENIED
//------------------------------------------------------------------------------

BOOL  MIC_OneWireEventRegistering(HANDLE handle, 
								  POWR_INPUT_CONTEXT pInputContext, 
                                  BOOL needToRegister);



//------------------------------------------------------------------------------
// Function name	: MIC_OneWirePrivateAccess
// Description	    : This function grants/release exclusive access for process with such 
//                    handle. Once successfully granted, other processes cannot to access 
//                    One-Wire devices.

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

//Parameters:         handle        –      valid handle from MIC_OneWireOpen;
//                    pInputContext –      pointer to ONE_WIRE_INPUT_CONTEXT  tructure, 
//                                         which filled the 'size' by User,  'oneWireEventType' and  
//                    'oneWireDatalength'  members are not relevant here.
//                    BOOL privateAccess – TRUE for set, FALSE – for release
//                    'opErrorCode'        member filled by Driver with appropriate error code if error occur.
//                                         Possible return codes for this function:

//                                          OWR_OK
//                                          OWR_ERROR_ACCESS_DENIED
//                                          OWR_ERROR_INVALID_ACCESS
//------------------------------------------------------------------------------

BOOL  MIC_OneWirePrivateAccess( HANDLE handle, 
                                POWR_INPUT_CONTEXT pInputContext,
                                BOOL privateAccess);



//------------------------------------------------------------------------------
// Function name	: MIC_OneWireWaitForEvent
// Description	    : This function grants/release exclusive access for process with such 
//                    handle. Once successfully granted, other processes cannot to access 
//                    One-Wire devices.

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

//Parameters:         handle        – valid handle from MIC_OneWireOpen;
//                    pInputContext – pointer to ONE_WIRE_INPUT_CONTEXT  tructure, 
//                                    which filled the 'size' by User,  'oneWireEventType' member 
//                                    returned by Driver. 'oneWireDatalength' is not relevant here.
//                    'opErrorCode'    member filled by Driver with appropriate error   
//                                         Possible return codes for this function:

//                                         OWR_OK
//                                         OWR_ERROR_CLOSED_HANDLE
//                                         OWR_ERROR_NOTIFICATION
//------------------------------------------------------------------------------

BOOL  MIC_OneWireWaitForEvent( HANDLE handle, POWR_INPUT_CONTEXT pInputContext);


	#ifdef __cplusplus
		}
	#endif	

#endif //_OWR_API_H_