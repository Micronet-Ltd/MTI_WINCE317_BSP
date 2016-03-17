/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           One Wire Low-Level header
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   17-Dec-2008
 *
 *  Revision History:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

#ifndef IBUTTON_H
#define IBUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

/************** HEADER FILE INCLUDES ******************************************/
/************** GLOBAL CONSTANTS **********************************************/
/************** STRUCTURES, ENUMS, AND TYPEDEFS *******************************/

/*
typedef enum 
{
  OWR_IBUTTON_POLLING = 0,
  OWR_IBUTTON_EVENT,
  
} OWR_IBUTTON_MODE;
*/

typedef enum
{
	IBTNKBD_NO_REG = 0,
	IBTNKBD_SHIFT,

} IBTN_KBD_REG;

typedef struct _ASCII_TO_VKEY_CONV_s
{
  UINT8 vkey;
  IBTN_KBD_REG  kbdReg;

} ASCII_TO_VKEY_CONV_s;

/************** FUNCTION PROTOTYPES *******************************************/

BOOL iButtonInit(void);
BOOL iButtonRead(OMAP_GPIO_ONE_WIRE *pOWR_OMAP, UINT8 romValue[]);
BOOL iButtonCheckPresence(OMAP_GPIO_ONE_WIRE *pOWR_OMAP);
BOOL checkiButtonPrefixnSuffix( TCHAR prefixNsuffix[] );
BOOL createIbtnTranslTable(void);
BOOL owriButtonSendKeyStroke(OMAP_GPIO_ONE_WIRE *pOWR_OMAP, UINT8 romdata[]);

/************** GLOBAL VARIABLES **********************************************/
extern BOOL   g_pulseDetect;
/************** MACROS ********************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* IBUTTON_H */
