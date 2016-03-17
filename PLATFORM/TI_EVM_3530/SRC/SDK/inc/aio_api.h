#ifndef _AIO_API_H_
#define _AIO_API_H_

#include <windows.h>


//==============================================================================================
//            local definitions
//==============================================================================================


#define MAX_INPUT_PINS                          4


//==============================================================================================
//            errors definitions
//==============================================================================================

#define AIO_OK                               0

// common errors
#define AIO_INTERNAL_ERROR                    10
#define AIO_ERROR_INVALID_PARAMETER           11

// input errors
#define AIO_ERROR_ALREDY_REGISTERED           100
#define AIO_ERROR_SIGNAL_NOT_STABLE           101
#define AIO_ERROR_INPUT_WRONG_ACCESS          102
#define AIO_ERROR_INPUT_IS_LOCKED             103
#define AIO_ERROR_INVALID_HANDLE              104
#define AIO_ERROR_IS_NOT_REGISTERED           105
#define AIO_ERROR_WRONG_NOTIFICATION_STATE    106
#define AIO_ERROR_ALREADY_WAITING_FOR_EVENT   107


	 // output errors
#define AIO_ERROR_OUTPUT_IS_LOCKED            200
#define AIO_ERROR_OUTPUT_IS_ACCUMULATED       201
#define AIO_ERROR_OUTPUT_WRONG_ACCESS         202


//==============================================================================================
//     Bitfield for automotive pins inputs/outputs
//==============================================================================================
 
union AUT_PINS {
    UINT32 mask;

    struct {
        UINT32              AUT_IN_1     :  1;
		UINT32              AUT_IN_2     :  1;
		UINT32              AUT_IN_3     :  1;
		UINT32              AUT_IN_4     :  1;
		UINT32              AUT_IN_5     :  1;
		UINT32              AUT_IN_6     :  1;
		UINT32              AUT_IN_7     :  1;
		UINT32              AUT_IN_8     :  1;
		UINT32              AUT_IN_9     :  1;
		UINT32              AUT_IN_10    :  1;
		UINT32              AUT_IN_11    :  1;
		UINT32              AUT_IN_12    :  1;
		UINT32              AUT_IN_13    :  1;
		UINT32              AUT_IN_14    :  1;
		UINT32              AUT_IN_15    :  1;
		UINT32              AUT_IN_16    :  1;

		UINT32              AUT_OUT_1    :  1;
		UINT32              AUT_OUT_2    :  1;
		UINT32              AUT_OUT_3    :  1;
		UINT32              AUT_OUT_4    :  1;
		UINT32              AUT_OUT_5    :  1;
		UINT32              AUT_OUT_6    :  1;
		UINT32              AUT_OUT_7    :  1;
		UINT32              AUT_OUT_8    :  1;
		UINT32              AUT_OUT_9    :  1;
		UINT32              AUT_OUT_10   :  1;
		UINT32              AUT_OUT_11   :  1;
		UINT32              AUT_OUT_12   :  1;
		UINT32              AUT_OUT_13   :  1;
		UINT32              AUT_OUT_14   :  1;
		UINT32              AUT_OUT_15   :  1;
		UINT32              AUT_OUT_16   :  1;

    };
};


//==============================================================================================
//     enumerations
//==============================================================================================
typedef enum {
	INVALID = 0,
    OUTPUT_PIN,
    INPUT_PIN
} PIN_TYPE;


typedef enum{
	INVALID_STATE = -1,
	LOW,
	HIGH
}PIN_STATE;



typedef enum{
	FREE,
	LOCKED,
	ACCUMULATED
}PIN_MODE;




//==============================================================================================
//     structure definitions
//==============================================================================================
//------------------------------------------------------------------------------
// inputs
typedef struct _AIO_INPUT_CONTEXT
{
    UINT32     size;           // IN size of structure. Must  
                                 // be filled by user 
	AUT_PINS   pinNo;          // IN: pinID
	PIN_STATE  currentState;   // OUT: current pin state
 	PIN_STATE  prevState;      // OUT: previous pin state. 
	PIN_MODE   mode;           // IN/OUT: pin's mode - FREE and LOCKED for input
    UINT32     opErrorCode;    // OUT: operation error code
} AIO_INPUT_CONTEXT, * PAIO_INPUT_CONTEXT;


typedef struct _AIO_NOTIFY_INFO
{
	UINT32              state;                          
	HANDLE              hInQueue; 
//	HANDLE              hOutQueue;
	HANDLE              hEvent;
	CRITICAL_SECTION    cs;
}AIO_NOTIFY_INFO, *PAIO_NOTIFY_INFO;


typedef struct _AIO_NOTIFY_CONTEXT
{
	UINT32            size;
	AIO_NOTIFY_INFO   pinInfo[MAX_INPUT_PINS];              

} AIO_NOTIFY_CONTEXT, * PAIO_NOTIFY_CONTEXT;





#define OPEN_BUFFER_LENGTH            sizeof(AIO_NOTIFY_CONTEXT)     // in bytes

//------------------------------------------------------------------------------
//  outputs


typedef struct _AIO_OUTPUT_CONTEXT
{
    UINT32     size;           // IN size of structure. Must  
                                 // be filled by user 
	AUT_PINS   pinNo;          // IN: pinID
	PIN_STATE  state;          // IN/OUT: current pin state
 	PIN_MODE   mode;           // IN/OUT: pin's mode - FREE/LOCLED/ACCUMULATED. 
    UINT32     opErrorCode;    // OUT: operation error code
} AIO_OUTPUT_CONTEXT, * PAIO_OUTPUT_CONTEXT;




#ifdef __cplusplus
	extern "C" {
#endif

// ================================= General functions====================================

	//------------------------------------------------------------------------------
	// Function name	: MIC_AIOOpen
	// Description	    : This function returns handle for safe access to automotive pins. 
	//                  : This handle must be used for all automotive pins’ operations. 
	// Return type		: Pointer to valid handle if succeeded, INVALID_HANDLE_VALUE if failed.
	// Argument         : pContext :input buffer with size = OPEN_BUFFER_LENGTH (in bytes),the first
	//                  : DWORD (size)of this buffer should be filled by user to OPEN_BUFFER_LENGTH value.
	
    // Note             : The maximum number of handles opened at the same time is 32.
    //------------------------------------------------------------------------------
	HANDLE  MIC_AIOOpen(LPVOID);

    //------------------------------------------------------------------------------
	// Function name	: MIC_AIOClose
	// Description	    : This function destroys the given handle.
	// Return type		: 0 if succeeded, error number if failed (AIO_ERROR_INVALID_PARAMETER,AIO_INTERNAL_ERROR).
    //                  : if AIO_INTERNAL_ERROR is returned, use GetLastError() to get more detail error.
	// Argument         : handle – valid handle from XXX_AIOOpen,
	//                  : pContext  is a context getting from XXX_AIOOpen
   //------------------------------------------------------------------------------
	DWORD   MIC_AIOClose(HANDLE ,LPVOID);


//	BOOL    MIC_AIOGetInputState(HANDLE handle, LPVOID pInputContext);

	// ================================= Inputs====================================

    //------------------------------------------------------------------------------
	// Function name	: MIC_AIOGetInputInfo
	// Description	    : Returns the current state and mode of the given input pin
	// Return type		: not zero indicates success, zero - failure
	// Argument         : handle – valid handle from XXX_AIOOpen,
	//                  : pInputContext  is of PAIO_INPUT_CONTEXT type 
	//                  : IN: Size and pinNo. 
	//                  : OUT: currentState, mode  and opErrorCode.
	// NOTE             : The prevState field in the pInputContext structure is not relevant for
	//                  : this function.
    //------------------------------------------------------------------------------
	BOOL    MIC_AIOGetInputInfo (HANDLE handle, LPVOID pInputContext);

	//------------------------------------------------------------------------------
	// Function name	: MIC_AIOSetInputMode
	// Description	    : Sets mode of the given input pin (FREE,LOCKED).
	//                  : If pin is in LOCKED mode only LOCK caller is allowed
	//                  : to release pin back to free mode.
	// Return type		: not zero indicates success, zero - failure
	// Argument         : handle – valid handle from XXX_AIOOpen,
	//                  : pInputContext  is of PAIO_INPUT_CONTEXT type 
	//                  : IN: Size,pinNo and mode. 
	//                  : OUT: opErrorCode.
    //------------------------------------------------------------------------------

	BOOL    MIC_AIOSetInputMode(HANDLE handle, LPVOID pInputContext);

	//------------------------------------------------------------------------------
	// Function name	: MIC_AIORegisterForStateChange
	// Description	    : Registers/Deregisters to get notifications on the state change event 
	//                    on the given pin. 
	// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

	// Arguments         : IN: handle – valid handle from MIC_AIOOpen;
	//                   : IN  notifyContext is a valid context which has been get in MIC_AIOOpen(notifyContext);
	//                   : IN/OUT: inputContext  is of PAIO_INPUT_CONTEXT type 
	//                   : IN: Size and pinNo. 
	//                   : OUT: opErrorCode.
	//                   : IN: needToRegister  if TRUE - need to register, otherwise - to deregister; 
	//------------------------------------------------------------------------------
	BOOL    MIC_AIORegisterForStateChange(HANDLE handle, LPVOID notifyContext,LPVOID inputContext,BOOL needToRegister);


	//------------------------------------------------------------------------------
	// Function name	: MIC_AIOStartStateChangeNotify
	// Description	    : Starts sending notifications on the pin status change.
	// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
	// Arguments         : handle – valid handle from MIC_AIOOpen;
	//                   : IN  notifyContext is a valid context which has been get in MIC_AIOOpen(notifyContext);
	//                   : IN/OUT: pInputContext  is of PAIO_INPUT_CONTEXT type 
	//                   : IN: Size and pinNo. 
	//                   : OUT: opErrorCode.
	//------------------------------------------------------------------------------
	BOOL    MIC_AIOStartStateChangeNotify(HANDLE handle, LPVOID notifyContext,LPVOID inputContext);


	//------------------------------------------------------------------------------
	// Function name	: MIC_AIOStopStateChangeNotify
	// Description	    : Stops sending notifications on the pin status change.
	// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
	// Arguments         : handle – valid handle from MIC_AIOOpen;
	//                   : IN  notifyContext is a valid context which has been get in MIC_AIOOpen(notifyContext);
	//                   : IN/OUT: pInputContext  is of PAIO_INPUT_CONTEXT type 
	//                   : IN: Size and pinNo. 
	//                   : OUT: opErrorCode.
	//------------------------------------------------------------------------------
	BOOL    MIC_AIOStopStateChangeNotify(HANDLE handle, LPVOID notifyContext,LPVOID inputContext);


	//------------------------------------------------------------------------------
	// Function name	: MIC_AIOWaitForStateChange
	// Description	    : It synchronously waits for given pin status change 
	// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

	// Arguments         : handle – valid handle from MIC_AIOOpen;
	//                   : IN  notifyContext is a valid context which has been get in MIC_AIOOpen(notifyContext);
	//                   : IN/OUT: pInputContext  is of PAIO_INPUT_CONTEXT type 
	//                   : IN: Size and pinNo. 
	//                   : OUT: currentState,previousState and opErrorCode.
	//------------------------------------------------------------------------------
	BOOL    MIC_AIOWaitForStateChange(HANDLE handle, LPVOID pNotifyContext,LPVOID pInputContext);


	//------------------------------------------------------------------------------
	// Function name	: MIC_AIOSetSignalStabilityTime
	// Description	    : Sets pin's stability' time. The min stability time is 0,
	//                  : The max stability time is 2000
	//                   In free mode (default) everyone is allowed to use this function.  
	//                   In lock mode, the only "lock" caller handler is allowed to use this function. 

	// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

	// Arguments         : handle – valid handle from MIC_AIOOpen;
	//                   : inputContext  is of PAIO_INPUT_CONTEXT type 
	//                   : IN: Size,pinNo and 
	//                   : OUT: opErrorCode.
	//                   : IN:  stability time 
	//------------------------------------------------------------------------------
	BOOL    MIC_AIOSetSignalStabilityTime(HANDLE handle, LPVOID inputContext,DWORD  stabilityTime);


	
	//------------------------------------------------------------------------------
	// Function name	: MIC_AIOGetSignalStabilityTime
	// Description	    : Gets pin's stability' time. 

	// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

	// Arguments         : handle – valid handle from MIC_AIOOpen;
	//                   : inputContext  is of PAIO_INPUT_CONTEXT type 
	//                   : IN:   Size and pinNo 
	//                   : OUT:  opErrorCode.
	//                   : OUT:  stability time 
	//------------------------------------------------------------------------------
	BOOL    MIC_AIOGetSignalStabilityTime(HANDLE handle, LPVOID inputContext,DWORD* stabilityTime);


	// =================================  Outputs ====================================
	//------------------------------------------------------------------------------
	// Function name	: MIC_AIOSetOutputState
	// Description	    : Sets pin state according to the chosen mode. 
	//                   In free mode (default) everyone is allowed to use this function. 
	//                   In accumulated mode the output is a logical OR of all the handle states. 
	//                   In lock mode, the only "lock" caller handler is allowed to use this function. 

	// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

	// Arguments         : handle – valid handle from MIC_AIOOpen;
	//                   : context  is of PAIO_OUTPUT_CONTEXT type 
	//                   : IN: Size,pinNo and state. 
	//                   : OUT: opErrorCode.
	//------------------------------------------------------------------------------
	BOOL    MIC_AIOSetOutputState(HANDLE handle, LPVOID pOutputContext);


	//------------------------------------------------------------------------------
	// Function name	: MIC_AIOSetOutputMode
	// Description	    : Sets pin mode  
	//                  : Possible operations:
	//                  : FREE->LOCKED/ACCUMULATED - OK
	//                  : LOCKED -> FREE only "lock" caller handler ia allowed
	//                  : LOCKED -> LOCKED/ACCUMULATED - error
	//                  : ACCUMULATED-> FREE only "accumulated" caller handler ia allowed
	//                  : ACCUMULATED -> LOCKED - everyone is allowed
	//                  : ACCUMULATED ->ACCUMULATED - error
	// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
	//
	// Arguments         : handle – valid handle from MIC_AIOOpen;
	//                   : context  is of PAIO_OUTPUT_CONTEXT type 
	//                   : IN: Size,pinNo and mode. 
	//                   : OUT: opErrorCode.
	//------------------------------------------------------------------------------
	BOOL    MIC_AIOSetOutputMode (HANDLE handle, LPVOID pOutputContext);



	//------------------------------------------------------------------------------
	// Function name	: MIC_AIOGetOutputInfo
	// Description	    : Returns the current state and mode of the given output pin.   
	//
	// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
	//
	// Arguments         : handle – valid handle from MIC_AIOOpen;
	//                   : context  is of PAIO_OUTPUT_CONTEXT type 
	//                   : IN: Size,pinNo. 
	//                   : OUT: state,mode and opErrorCode.
	//------------------------------------------------------------------------------
	BOOL    MIC_AIOGetOutputInfo (HANDLE handle, LPVOID pOutputContext);




	#ifdef __cplusplus
		}
	#endif	

#endif //_AIO_API_H_