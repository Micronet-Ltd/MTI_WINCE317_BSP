/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           Header inp_isr_dbg
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

/********************** INCLUDES **********************************************/
/********************** LOCAL CONSTANTS ***************************************/
/*********** LOCAL STRUCTURES, ENUMS, AND TYPEDEFS ****************************/
/****************** STATIC FUNCTION PROTOTYPES *********************************/
/*********************** LOCAL MACROS *****************************************/
/********************* GLOBAL VARIABLES ***************************************/
/********************* STATIC VARIABLES ***************************************/

#define OWRISRMSG(exp,p)    ((exp)?ISRDbgPintf p,1:0)

/******************** FUNCTION DEFINITIONS ************************************/

VOID ISRInitDebugSerial( VOID );


//------------------------------------------------------------------------------
//
//  Function:  ISRWriteDebugString
//
//  Output unicode string to debug serial port
//
VOID ISRWriteDebugString(UINT16 *string);


//------------------------------------------------------------------------------
//
//  Function:  ISRDbgPintf
//
//  Formatted Output unicode string to debug serial port
//
VOID ISRDbgPintf(LPCWSTR format, ...);

