/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           H module OWR
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   1-Dec-2009
 *
 *  Revision Histiory:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

#ifndef __OWR_H
#define __OWR_H

#ifdef __cplusplus
extern "C" {
#endif


//==============================================================================================
//            local definitions
//==============================================================================================


#define OWR_IDENTIFIER                          L"OWR1:"

#define 	OWR_QUEUE_NAME_LEN   64


//==============================================================================================
//            IOControls definitions
//==============================================================================================

#define     IOCTL_OWR_GET_DEVICE_CONTEXT                CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	    IOCTL_INPUT_REGISTER_NOTIFY           CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_OUTPUT_IBUTTON_READ             CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_INPUT_SET_ACCESS                CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0803, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*
#define		IOCTL_INPUT_GET_STATE                  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_INPUT_SET_STABLE_TIME            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_INPUT_SET_MODE                   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_OUTPUT_SET_MODE                  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0805, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_OUTPUT_SET_STATE                 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_OUTPUT_GET_INFO                  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0807, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	    IOCTL_INPUT_DEREGISTER_STATE_CHANGE    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0808, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_INPUT_GET_STABLE_TIME            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0809, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_INPUT_GET_INFO                   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080A, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_INPUT_START_CHANGE_EVENT         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080B, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_INPUT_STOP_CHANGE_EVENT          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080C, METHOD_BUFFERED, FILE_ANY_ACCESS)
*/
#define		IOCTL_OUTPUT_OW                        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080D, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_OUTPUT_OW_TEST                   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080E, METHOD_BUFFERED, FILE_ANY_ACCESS)


/*
typedef enum {
	INVALID_NOTIFY_STATE = 0,
    OPENED,
    REGISTER,
	WAITING_FOR_EVENT
} INPUT_NOTIFY_STATE;
*/

/*
#define OPENED_STATE       0x0001
#define REGISTERED_STATE   0x0002
#define START_NOTIFY_STATE 0x0004
#define WAITING_FOR_EVENT  0x0008
*/

typedef enum{
	i_BUTTON = 0,
	TEMPR_SENS1,

} OWR_DEVICE_TYPE;


typedef struct _OWR_GET_DEVICE_CONTEXT
{
    UINT32           opErrorCode;                          // OUT: operation error code
    OWR_DEVICE_TYPE  owrDeviceType;
	TCHAR            owrQueueName[OWR_QUEUE_NAME_LEN/2]; 

} OWR_GET_DEVICE_CONTEXT, * POWR_GET_DEVICE_CONTEXT;


typedef struct _OWR_IBUTTON_CONTEXT
{
    UINT32     opErrorCode;                          // OUT: operation error code
	UINT8      romValue[8]; 

} OWR_IBUTTON_CONTEXT, *POWR_IBUTTON_CONTEXT;


//------------------------------------------------------------------------------


 #ifdef __cplusplus
}
#endif

#endif
