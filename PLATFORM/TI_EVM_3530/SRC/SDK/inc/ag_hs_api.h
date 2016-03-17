/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:          H Audio Gateway Headset API
 *  Author(s):      Michael Streshinsky
 *  Creation Date:  18-jan-2011
 *
 *  Revision Histiory:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

#ifndef _AG_HS_H
#define _AG_HS_H


/************** HEADER FILE INCLUDES ******************************************/
/************** GLOBAL CONSTANTS **********************************************/
// Headset events
// Command issued by HS to indicate that the button has been pressed
#define   EV_AG_HS_BUTTON_PRESS         0x00000001

// Command issued by the HS to report the current microphone gain level setting to
// the AG. <gain> is a decimal numeric constant, relating to a particular (implementation-
// dependent) volume level controlled by the HS (optional)
#define   EV_AG_HS_MIC_GAIN             0x00000002

// Command issued by the HS to report the current speaker gain level setting to the
// AG. <gain> is a decimal numeric constant, relating to a particular (implementation-
// dependent) volume level controlled by the HS. (optional)						 
#define   EV_AG_HS_SPEAKER_GAIN         0x00000004

// Bluetooth control connection has been updated;
#define   EV_AG_HS_CTRL                 0x00000008
			
// Bluetooth audio connection has been updated;
#define   EV_AG_HS_AUDIO                0x00000010


// Network (telephone line) events

// ringing
#define   EV_AG_NET_RING                0x00000100

// answer call
#define   EV_AG_NET_ANSWER              0x00000200

// ending cal 
#define   EV_AG_NET_END_CALL            0x00000400

// rejecting incoming cal
#define   EV_AG_NET_REJECT_INCALL       0x00000800 

#define  EV_AG_ALL_MASK            \
	     (                         \
           EV_AG_HS_BUTTON_PRESS |  \
           EV_AG_HS_MIC_GAIN     |  \
           EV_AG_HS_SPEAKER_GAIN |  \
           EV_AG_HS_CTRL         |  \
           EV_AG_HS_AUDIO        |  \
           EV_AG_NET_RING        |  \
           EV_AG_NET_ANSWER      |  \
           EV_AG_NET_END_CALL    |  \
           EV_AG_NET_REJECT_INCALL \
         )


/************** STRUCTURES, ENUMS, AND TYPEDEFS *******************************/
typedef enum
{
    AG_HS_OK = 0,
	AG_HS_DEV_SET_ERROR,
	AG_HS_DEV_OPEN_ERROR,
	AG_HS_SERVICE_OPEN_ERROR,
	AG_HS_SERVICE_CTRL_ERROR,
	AG_HS_SERVICE_INVALID_CMD,
	AG_HS_SERVICE_INVALID_PARAM,
	AG_HS_SERVICE_INVALID_STATE,
	AG_HS_MODULE_ABSENT,
	AG_HS_MODULE_OFF,
	AG_HS_INVALID_HANDLE_VALUE,
	AG_HS_OPEN_OVERFLOW,
	AG_HS_INVALID_BT_ADDR,
	AG_HS_MSG_CREATE_ERROR,
	AG_HS_LIB_LOAD_ERROR,
	AG_HS_PROC_ADDR_ERROR,
	AG_HS_REG_EVENT_ERROR,
	AG_HS_CREATE_EVENT_ERROR,
	AG_HS_MSG_READ_ERROR,
	AG_HS_DEV_CLOSED,

	AG_HS_MAX_ERRORS

} AG_HS_ERROR_CODES;

typedef enum 
{
    AG_CONNECT = 0,
    AG_AUDIO_ON,
    AG_AUDIO_OFF,
	AG_DISCONNECT,
	AG_SERVICE_START,
	AG_SERVICE_STOP,
    AG_SERVICE_STATUS,

    AG_GET_MIC_VOL,     // - retrieves the microphone volume of the headset or the hands-free device;
    AG_GET_SPEAKER_VOL, // - retrieves the speaker volume of the headset or the hands-free device;

    AG_SET_MIC_VOL,     // - sets the microphone volume of the headset or the hands-free device;
    AG_SET_SPEAKER_VOL, // - sets the speaker volume

    AG_GET_POWER_MODE,  // - headset power-save mode  
    AG_SET_POWER_MODE,     

} AGHSCMD_e;


typedef struct _AGHS_MSG
{
  DWORD   hsEvent;
  DWORD   dwParam;

} AGHS_MSG, *PAGHS_MSG;


/************** FUNCTION PROTOTYPES *******************************************/
#ifdef __cplusplus
	extern "C" {
#endif

/** 
 * @fn INT32  MIC_AGHSOpen(HANDLE *pHandle, BT_ADDR b)
 *  
 * @param    HANDLE *pHandle  - return HANDLE to device
 * @param    BT_ADDR b  - Bluetoot device address
 *
 * @return status of operation:
 *         AG_HS_OK - if Ok
 *         AG_HS_DEV_SET_ERROR - Failure
 *
 * @brief Set Bluetooth Headset device (only one) with given Bluetooth address
*/  
 INT32  MIC_AGHSOpen(HANDLE *pHandle, BT_ADDR b);


 INT32  MIC_AGHSClose(HANDLE Handle);


/** 
 * @fn INT32 MIC_AGHSControl(AGHSCmd_e cmd, VOID *pParam)
 *  
 * @param   AGHSCmd_e cmd - Headset AudioGateway Control command
 * @param   VOID *pParam  - Headset AudioGateway command's parameter
 *
 *  Command list
 *  cmd                * param In/Out/Type/ N/A * description
 *
 *  AG_CONNECT         *    N/A                 * Establish ALC (command) connecttion
 *  AG_AUDIO_ON        *    N/A                 * Establish SCO (Audio) connecttion
 *  AG_AUDIO_OFF       *    N/A                 * disconnect SCO (Audio)
 *  AG_DISCONNECT      *    N/A                 * disconnect ALC (command)
 *	AG_SERVICE_START   *    N/A                 *
 *	AG_SERVICE_STOP    *    N/A                 *
 *  AG_SERVICE_STATUS  *    In/DWORD            * see service.h
 *
 *  AG_GET_MIC_VOL     *    Out/USHORT          * retrieves the microphone volume of the headset or the hands-free device;
 *  AG_GET_SPEAKER_VOL *    Out/USHORT          * retrieves the speaker volume of the headset or the hands-free device;
 *
 *  AG_SET_MIC_VOL     *    In/USHORT           * sets the microphone volume of the headset or the hands-free device;
 *  AG_SET_SPEAKER_VOL *    In/USHORT           * sets the speaker volume
 *	
 *  AG_GET_POWER_MODE  *    Out/BOOL            * get headset power-save mode  
 *  AG_SET_POWER_MODE  *    In/BOOL             * set headset power-save mode  
 *
 * @return status of operation:
 *         0 - if Ok
 *         AG_HS_SERVICE_IVALID_PARAM
 *         AG_HS_SERVICE_IVALID_CMD
 *         AG_HS_SERVICE_OPEN_ERROR
 *         AG_HS_SERVICE_CTRL_ERROR 
 *        
 * @brief Control Bluetooth Headset device
 *    
 */  
INT32  MIC_AGHSControl(AGHSCMD_e cmd, VOID *pParam);

INT32  MIC_AGHSEventRegistering(HANDLE Handle, DWORD eventMask);

INT32  MIC_AGHSGetEvent(HANDLE Handle, DWORD *pEventMask);

INT32  MIC_AGHSWaitForEvent(HANDLE Handle, AGHS_MSG *pAgEvent);

#ifdef __cplusplus
	}
#endif	

/************** GLOBAL VARIABLES **********************************************/
/************** MACROS ********************************************************/

#endif /* _AG_HS_H */
