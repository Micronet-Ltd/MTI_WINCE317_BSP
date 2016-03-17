/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           H module gpio_isr
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   10-Dec-2009
 *
 *  Revision Histiory:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

#ifndef __OWR_ISR_H
#define __OWR_ISR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
  OWR_IDLE = 0,
  OWR_RESET_PRESENCE,

/*
  OWR_WRITE_0_SLOT,
  OWR_WRITE_1_SLOT,
*/
  OWR_WRITE_SLOTS,

  OWR_READ_SLOTS,

} OWR_LOW_LVL_PHASE;

// OWR_RESET_PRESENCE phase
typedef enum 
{
  OWR_RESET_BEGIN = 0,
  OWR_RESET_END,       // And Wait presence begin
  OWR_PRESENCE_END     // Signal to IST

} OWR_RESET_PRESENCE_STATES;

#if 0
// OWR_WRITE_0_SLOT phase
typedef enum 
{
  OWR_SLOT_0_BEGIN = 0,
  OWR_SLOT_0_END, 

} OWR_WRITE_0_SLOT_STATES;

// OWR_WRITE_0_SLOT phase
typedef enum 
{
  OWR_SLOT_1_BEGIN = 0,
  OWR_SLOT_1_END, 

} OWR_WRITE_1_SLOT_STATES;
#endif

typedef enum 
{
  OWR_WRITE_SLOT_BEGIN = 0,
  OWR_WRITE_SLOT_END, 

} OWR_WRITE_0_SLOT_STATES;

// OWR_READ_SLOT phase
typedef enum 
{
  OWR_READ_SLOT_BEGIN = 0,
  OWR_READ_SLOT_END, 

} OWR_READ_SLOT_STATES;


#define OWR_TIMER_LOG_LEN     (32*8)
#define OWR_ISR_MAX_DATA_LEN  32 // In bytes

typedef struct _OWR_TIMER_STATE 
{
	OWR_LOW_LVL_PHASE   phase;
    UINT32              state;    // Different for Each phase
	UINT32              cntBegin; // TCRR value (pulse beginning)
	UINT32				cntMatch;
	UINT32              cntEnd;   // TCRR value (pulse end)
    BOOL                endOp; 
	UINT8               readData[OWR_ISR_MAX_DATA_LEN]; // max 32 bytes for data read
	//UINT32              readData;
    UINT32              readDataBitLen; // Actual length for read data in bits
	UINT32              readDataBitCnt;  // current number of read data in bits
	UINT8               writeData[OWR_ISR_MAX_DATA_LEN]; // max 32 bytes for data write 
    UINT32              writeDataBitLen;  // Actual length for write data in bits 
    UINT32              writeDataBitCnt;  // current number of  written data in bits 
    BOOL                signalIST;
    UINT32              gptIntrCnt; 
	BYTE                log[OWR_TIMER_LOG_LEN]; 

} OWR_TIMER_STATE;


// OWR Line states
typedef enum 
{
  OWR_LINE_IMPULSE_BEGIN = 0,
  OWR_LINE_IMPULSE_END,      // And Wait presence begin
  
} OWR_LINE_STATE;

typedef struct _OWR_INPUT_STATE 
{
	OWR_LINE_STATE      lineState;
    UINT32              numOfPulses;
	UINT32              cntBegin; // TCRR value (pulse beginning)
	UINT32              cntEnd;   // TCRR value (pulse end)
    BOOL                signalIST;

} OWR_INPUT_STATE;


 BOOL  owrIsrSetResetPulse(void);
 BOOL  owrIsrSetDetectPulse(BOOL pulseDetect);
 BOOL  owrIsrWriteData(UINT8 data[], UINT32 len);
 //BOOL  owrIsrReadByte(UINT8 *readData);
 BOOL  owrIsrReadData(UINT32 len);

 extern UINT32 g_owrImpulseState;
 extern volatile OWR_TIMER_STATE  g_OwrGptState;
 extern volatile OWR_INPUT_STATE  g_OwrInputState;

 #ifdef __cplusplus
}
#endif

#endif //__OWR_ISR_H

