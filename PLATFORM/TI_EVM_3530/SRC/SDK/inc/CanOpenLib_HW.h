/*
       h t t p : / / 
        _____          _   _                                      
       / ____|   /\   | \ | |                                     
      | |       /  \  |  \| | ___  _ __   ___ _ __    _ __  _   _ 
      | |      / /\ \ | . ` |/ _ \| '_ \ / _ \ '_ \  | '_ \| | | |
      | |____ / ____ \| |\  | (_) | |_) |  __/ | | |_| | | | |_| |
       \_____/_/    \_\_| \_|\___/| .__/ \___|_| |_(_)_| |_|\__,_|
                                  | |                             
                                  |_|                              

                      CANopen API (C++/C#) by CANopen.nu.
            Copyright (C) 2009 Ulrik Hagstrom (ulrik@canopen.nu)

  CANopen API (C++/C#) is available under a dual license: 
  
  1. CANopen API (C++/C#) can be used under the GNU General Public License, 
  version 3.0. This license allows for free use, but also requires that your 
  own code using CANopen API (C++/C#) be released under the same license. 
  For details, please read version 3.0 of the GPL (see gpl-3.0.txt).

  2. CANopen API (C++/C#) can be used under the CANopen API (C++/C#) 
  Commercial License. This license allows for the use of CANopen API (C++/C#) 
  in closed-source projects. This license is available for a very reasonable 
  fee and also includes technical support. For details, please read the 
  CANopen API (C++/C#) Commercial License (see license.txt).

*/

#include "CANopenLibDefines.h"

#ifndef CANOPENLIB_HW_H
#define CANOPENLIB_HW_H
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#ifdef CANLIB32_EXPORTS
#define CANOPENLIB_HW_API  EXTERN_C //__declspec(dllexport) 
#else
#define CANOPENLIB_HW_API  EXTERN_C //__declspec(dllimport)
#endif

// Valid baud rates for function 'canPortBitrateSet'
typedef enum 		
{ 
 CANBR_10KBPS  = 10000,
 CANBR_20KBPS  = 20000,
 CANBR_50KBPS  = 50000,
 CANBR_100KBPS = 100000,
 CANBR_125KBPS = 125000,
 CANBR_250KBPS = 250000,
 CANBR_500KBPS = 500000,
 CANBR_800KBPS = 800000,
 CANBR_1MBPS   = 1000000,
  
}CAN_BAUD_RATES;


/***************
*
*   CANopen-lib hardware port plugin DLL (canopenlib_hw.dll).
*
****************/

typedef int      canPortHandle;

//
// CAN informaiton flags.
//
#define CAN_MSG_RTR              0x0001      // Message is a remote request
#define CAN_MSG_EXT              0x0002      // Message has a standard ID


//------------------------------------------------------------------------------
// Function name	: canPortLibraryInit
// Description	    : This function do nothing, for backward compatibility only
// Return type		: canOpenStatus
// Argument         : none
//------------------------------------------------------------------------------

CANOPENLIB_HW_API   canOpenStatus        __stdcall canPortLibraryInit(void);


//------------------------------------------------------------------------------
// Function name	: canPortOpen
// Description	    : This function opens CAN port, returns Handle
// Return type		: canOpenStatus, canPortHandle *handle
// Argument         : port number (0 supported now), pointer to Handle
//------------------------------------------------------------------------------

CANOPENLIB_HW_API   canOpenStatus        __stdcall canPortOpen( int port, canPortHandle *handle );



//------------------------------------------------------------------------------
// Function name	: canPortClose
// Description	    : This function closes CAN port
// Return type		: canOpenStatus
// Argument         : Handle from canPortOpen
//------------------------------------------------------------------------------

CANOPENLIB_HW_API   canOpenStatus        __stdcall canPortClose( canPortHandle handle );



//------------------------------------------------------------------------------
// Function name	: canPortBitrateSet
// Description	    : This function sets bit rate for CAN port
// Return type		: canOpenStatus
// Argument         : Handle from canPortOpen, bitrate
//                    use only supported bitrates from enum 'CAN_BAUD_RATES'
//
// Note             : before calling this functions set CAN bus in configuration mode
//                    using 'canPortGoBusOff' function. 
//------------------------------------------------------------------------------

CANOPENLIB_HW_API   canOpenStatus        __stdcall canPortBitrateSet( canPortHandle handle, int bitrate );


//------------------------------------------------------------------------------
// Function name	: canPortEcho
// Description	    : This function enables/disables transmit echo
// Return type		: canOpenStatus, bool
// Argument         : Handle from canPortOpen, enables/disable parameter
//                     
// Notes            : If echo enabled, received frames will be send back
//                    to transmitter
//------------------------------------------------------------------------------

CANOPENLIB_HW_API   canOpenStatus        __stdcall canPortEcho( canPortHandle handle, bool enabled );


//------------------------------------------------------------------------------
// Function name	: canPortGoBusOn
// Description	    : This function turns CAN bus On
// Return type		: canOpenStatus
// Argument         : Handle from canPortOpen
//                    
// Notes            : Sets CAN controller in Operational Mode
//                    In this mode, transmit and receiving are enabled
//------------------------------------------------------------------------------

CANOPENLIB_HW_API   canOpenStatus        __stdcall canPortGoBusOn( canPortHandle handle );


//------------------------------------------------------------------------------
// Function name	: canPortGoBusOff
// Description	    : This function turns CAN bus Off
// Return type		: canOpenStatus
// Argument         : Handle from canPortOpen
//                    
// Notes            : Sets CAN controller in Configuration Mode
//                    In this mode, transmit and receiving are disabled,
//                    but enable configuration settings
//------------------------------------------------------------------------------

CANOPENLIB_HW_API   canOpenStatus        __stdcall canPortGoBusOff( canPortHandle handle );


//------------------------------------------------------------------------------
// Function name	: canPortWrite
// Description	    : This function transmts CAN message (frame)
// Return type		: canOpenStatus
// Argument         : Handle from canPortOpen
//                    frame id,
//                    *msg - pointer to Data for transmit  
//                     dlc - datalength
//                     flags - CAN_MSG_RTR or/and CAN_MSG_EXT which define
//                     if frame is remote or not, if frame is standard or extended
//
// Notes            : CAN bus should be On when calling this function
//------------------------------------------------------------------------------

CANOPENLIB_HW_API   canOpenStatus        __stdcall canPortWrite(canPortHandle handle,
                                                                long id,
                                                                void *msg,
                                                                unsigned int dlc,
                                                                unsigned int flags);



//------------------------------------------------------------------------------
// Function name	: canPortRead
// Description	    : This function reads CAN message (frame)
// Return type		: canOpenStatus
// Argument         : Handle from canPortOpen
//                    pointer to receiving frame id,
//                    *msg - pointer to receiving Data 
//                    *dlc - pointer to datalength
//                    *flags - pointer to flag CAN_MSG_RTR or/and CAN_MSG_EXT which define
//                     if frame is remote or not, if frame is standard or extended
//
// Notes            : CAN bus should be On when calling this function
//------------------------------------------------------------------------------

CANOPENLIB_HW_API   canOpenStatus        __stdcall canPortRead(canPortHandle handle,
                                                                long *id,
                                                                void *msg,
                                                                unsigned int *dlc,
                                                                unsigned int *flags);

#endif
