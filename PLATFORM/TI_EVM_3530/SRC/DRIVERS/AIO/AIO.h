// Copyright 2009 - 2013 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//   File:  AIODrvr.h
//
//   Automotive Input/Output Driver header
//  
//
//   Created by Anna Rayer  September 2009
//-----------------------------------------------------------------------------/*

#ifndef __AIO_H
#define __AIO_H

#ifdef __cplusplus
extern "C" {
#endif



//==============================================================================================
//            local definitions
//==============================================================================================


#define AIO_IDENTIFIER                          L"AIO1:"

#define AIO_QUEUE_NAME_LEN	                    64	           		// bytes



//==============================================================================================
//            IOControls definitions
//==============================================================================================

#define     IOCTL_ATSR_GETQUEUENAME                CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	    IOCTL_INPUT_REGISTER_STATE_CHANGE      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0801, METHOD_BUFFERED, FILE_ANY_ACCESS)
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
#define		IOCTL_INPUT_GET_PINS_NO                CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080D, METHOD_BUFFERED, FILE_ANY_ACCESS)


//==============================================================================================
//     AUTOMOTIVE INPUT PINS
//==============================================================================================


// AUTOMOTIVE INPUT PINS

#define INPUT_PIN_0                     (1 << 0)
#define INPUT_PIN_1                     (1 << 1)
#define INPUT_PIN_2                     (1 << 2)
#define INPUT_PIN_3                     (1 << 3)
#define INPUT_PIN_4                     (1 << 4)
#define INPUT_PIN_5                     (1 << 5)
#define INPUT_PIN_6                     (1 << 6)
#define INPUT_PIN_7                     (1 << 7)
#define INPUT_PIN_8                     (1 << 8)
#define INPUT_PIN_9                     (1 << 9)
#define INPUT_PIN_10                    (1 << 10)
#define INPUT_PIN_11                    (1 << 11)
#define INPUT_PIN_12                    (1 << 12)
#define INPUT_PIN_13                    (1 << 13)
#define INPUT_PIN_14                    (1 << 14)
#define INPUT_PIN_15                    (1 << 15)

//==============================================================================================
//     AUTOMOTIVE OUTPUT PINS
//==============================================================================================

#define OUTPUT_PIN_0                    (1 << 16)
#define OUTPUT_PIN_1                    (1 << 17)
#define OUTPUT_PIN_2                    (1 << 18)
#define OUTPUT_PIN_3                    (1 << 19)
#define OUTPUT_PIN_4                    (1 << 20)
#define OUTPUT_PIN_5                    (1 << 21)
#define OUTPUT_PIN_6                    (1 << 22)
#define OUTPUT_PIN_7                    (1 << 23)
#define OUTPUT_PIN_8                    (1 << 24)
#define OUTPUT_PIN_9                    (1 << 25)
#define OUTPUT_PIN_10                   (1 << 26)
#define OUTPUT_PIN_11                   (1 << 27)
#define OUTPUT_PIN_12                   (1 << 28)
#define OUTPUT_PIN_13                   (1 << 29)
#define OUTPUT_PIN_14                   (1 << 30)
#define OUTPUT_PIN_15                   (1 << 31)


/*
typedef enum {
	INVALID_NOTIFY_STATE = 0,
    OPENED,
    REGISTER,
	WAITING_FOR_EVENT
} INPUT_NOTIFY_STATE;
*/

#define OPENED_STATE       0x0001
#define REGISTERED_STATE   0x0002
#define START_NOTIFY_STATE 0x0004
#define WAITING_FOR_EVENT  0x0008

typedef struct _AIO_GET_NAME_CONTEXT
{
    UINT32     opErrorCode;                          // OUT: operation error code
//	TCHAR      adQueueName[AIO_QUEUE_NAME_LEN/2];    // OUT: 
	TCHAR      daQueueName[AIO_QUEUE_NAME_LEN/2]; 

} AIO_GET_NAME_CONTEXT, * PAIO_GET_NAME_CONTEXT;

#define MAX_STABLE_TIME                         2000

 #ifdef __cplusplus
}
#endif

#endif
