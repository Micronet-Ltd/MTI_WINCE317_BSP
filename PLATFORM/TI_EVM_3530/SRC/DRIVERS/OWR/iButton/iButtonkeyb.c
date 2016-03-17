/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           iButton Keyb simulation High-Level C module
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   24-Jan-2010
 *
 *  Revision History:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

/********************** INCLUDES **********************************************/

#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
//#include <initguid.h>
//#include <gpio.h>
//#include <bus.h>
#include "omap35xx.h"
//#include "oal.h"
//#include <oal.h>
//#include <oalex.h>
//#include <omap35xx.h>
//#include "oal_io.h"
//#include "oal_memory.h"
//#include <oal_prcm.h>
//#include <aio_api.h>

//#include "..\gpio_isr\gpio_isr.h"
//#include "..\gpio_isr\gpt_isr.h"
#include "owrdrv.h"
#include "iButton.h"

/********************** LOCAL CONSTANTS ***************************************/
#define   VK_KEY_0    	0x30 //('0')
#define   VK_KEY_1    	0x31 //('1')
#define   VK_KEY_2    	0x32 //('2')
#define   VK_KEY_3    	0x33 //('3')
#define   VK_KEY_4    	0x34 //('4')
#define   VK_KEY_5    	0x35 //('5')
#define   VK_KEY_6    	0x36 //('6')
#define   VK_KEY_7    	0x37 //('7')
#define   VK_KEY_8    	0x38 //('8')
#define   VK_KEY_9    	0x39 //('9')

/*********** LOCAL STRUCTURES, ENUMS, AND TYPEDEFS ****************************/

/****************** STATIC FUNCTION PROTOTYPES *********************************/

/*********************** LOCAL MACROS *****************************************/
/********************* GLOBAL VARIABLES ***************************************/


/********************* STATIC VARIABLES ***************************************/

ASCII_TO_VKEY_CONV_s  IBTNKBD_ASCIIToVkeysTable_00000409[] =
{
/* "sp ", */ VK_SPACE, IBTNKBD_NO_REG,
/* " ! ", */ VK_KEY_1, IBTNKBD_SHIFT, // !
/* " \" ", */ VK_BACKSLASH, IBTNKBD_SHIFT, // "\"
/* " # ", */  VK_KEY_3, IBTNKBD_SHIFT,
/* " $ ", */  VK_KEY_4, IBTNKBD_SHIFT,
/* " % ", */  VK_KEY_5, IBTNKBD_SHIFT,
/* " & ", */  VK_KEY_7, IBTNKBD_SHIFT,
/* " ' ", */  VK_APOSTROPHE, IBTNKBD_NO_REG, // '
/* " ( ", */  VK_KEY_9, IBTNKBD_SHIFT,
/* " ) ", */  VK_KEY_0, IBTNKBD_SHIFT,
/* " * ", */  VK_KEY_8, IBTNKBD_SHIFT, //*
/* " + ", */  VK_EQUAL, IBTNKBD_SHIFT, // =
/* " ,", */   VK_COMMA, IBTNKBD_NO_REG, //,-
/* " - ", */  VK_HYPHEN, IBTNKBD_NO_REG,
/* " . ", */  VK_PERIOD, IBTNKBD_NO_REG,
/* " / ", */  VK_SLASH, IBTNKBD_NO_REG,  // /
/* " 0 ", */  VK_KEY_0, IBTNKBD_NO_REG,
/* " 1 ", */  VK_KEY_1, IBTNKBD_NO_REG,
/* " 2 ", */  VK_KEY_2, IBTNKBD_NO_REG,
/* " 3 ", */  VK_KEY_3, IBTNKBD_NO_REG,
/* " 4 ", */  VK_KEY_4, IBTNKBD_NO_REG,
/* " 5 ", */  VK_KEY_5, IBTNKBD_NO_REG,
/* " 6 ", */  VK_KEY_6, IBTNKBD_NO_REG,
/* " 7 ", */  VK_KEY_7, IBTNKBD_NO_REG,
/* " 8 ", */  VK_KEY_8, IBTNKBD_NO_REG,
/* " 9 ", */  VK_KEY_9, IBTNKBD_NO_REG,
/* " : ", */  VK_SEMICOLON, IBTNKBD_SHIFT, // :
/* " ; ", */  VK_SEMICOLON, IBTNKBD_NO_REG, // ;
/* " < ", */  VK_COMMA, IBTNKBD_SHIFT,  // ,<
/* " = ", */  VK_EQUAL, IBTNKBD_NO_REG, // =
/* " > ", */  VK_PERIOD, IBTNKBD_SHIFT, //>
/* " ? ", */  VK_SLASH, IBTNKBD_SHIFT, // ?
/* " @ ", */  VK_KEY_2, IBTNKBD_SHIFT,

};


ASCII_TO_VKEY_CONV_s *g_piButnTranslTbl = NULL;


#define NIBBLE   0x0F
UINT8 hextab[] = "0123456789ABCDEF";

void byte2hex (UINT8 data, UINT8 buf[]);


/******************** FUNCTION DEFINITIONS ************************************/

void byte2hex (UINT8 data, UINT8 buf[])
{
   buf[0] = hextab[(data >> 4) & NIBBLE];

   /* turn off last 4 low order bits */
   buf[1] = hextab [data & NIBBLE];

   buf[2] = '\0';
   return;
}
/* End of byte2hex */

BOOL createIbtnTranslTable(void)
{
 HKL     hkl;

 RETAILMSG(OWR_DRV_DBG, (L"+createIbtnTranslTable\r\n"));

#if 0
  hkl = GetKeyboardLayout(0); // Doesn't work !

  RETAILMSG(OWR_DRV_DBG, (L"+createIbtnTranslTable: hkl = 0x%X\r\n", hkl));

  // meanwhile support only US Keyboard
  if( LOWORD(hkl) != MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US) )
  {
   RETAILMSG(OWR_DRV_DBG, (L"createIbtnTranslTable fail, lID = 0x%X\r\n", LOWORD(hkl)));
   return(FALSE);
  }
  else
  {
   RETAILMSG(OWR_DRV_DBG, (L"createIbtnTranslTable Ok, lID = 0x%X\r\n", LOWORD(hkl)));
   g_piButnTranslTbl = (ASCII_TO_VKEY_CONV_s *)&IBTNKBD_ASCIIToVkeysTable_00000409[0];
   return(TRUE);
  }
#endif

 RETAILMSG(OWR_DRV_DBG, (L"-createIbtnTranslTable\r\n"));
 g_piButnTranslTbl = (ASCII_TO_VKEY_CONV_s *)&IBTNKBD_ASCIIToVkeysTable_00000409[0];
 return(TRUE);

}



// ASCII_TO_VKEY_CONV_s

BOOL convertAsciiToVirt( const UINT8 aStr[], ASCII_TO_VKEY_CONV_s vStr[], DWORD len )
{
  DWORD    i;

  if( ( aStr == NULL )||( vStr == NULL )|| (g_piButnTranslTbl == NULL) || ( len > MAX_PATH) )
   return(FALSE);

  for( i = 0; i < len; i++ )
  {

	  if( ( aStr[i] >= '!' )&& (aStr[i] <= '@' ) )
	  {
	   vStr[i].vkey = g_piButnTranslTbl[aStr[i] - ' '].vkey;
	   vStr[i].kbdReg = g_piButnTranslTbl[aStr[i] - ' '].kbdReg;
	  }
	  else if( ( aStr[i] >= 'A' )&& (aStr[i] <= 'Z' ) )
	  {
		vStr[i].vkey = aStr[i]; 
		vStr[i].kbdReg = IBTNKBD_SHIFT;
	  }
	  else if( ( aStr[i] >= 'a' )&& (aStr[i] <= 'z' ) )
	  {
		vStr[i].vkey = aStr[i]; 
		vStr[i].kbdReg = IBTNKBD_NO_REG;
	  }
	  else
	  {
		vStr[i].vkey = 0x00; 
		vStr[i].kbdReg = IBTNKBD_NO_REG;

		return(FALSE);
	  }
  }

  return(TRUE);
}


BOOL checkiButtonPrefixnSuffix( TCHAR prefixNsuffix[] )
{
  INT32    i;
  BOOL     rPrefix;

  //if( pOWR_OMAP == NULL )
   //return(FALSE);

  //if( pOWR_OMAP->owriButton.iButtonMode != i_BUTTON_EXT_MODE )
  //    return(FALSE);

    rPrefix = TRUE;

	for( i = 0; i < 3; i++ )
	{
	  if( prefixNsuffix[i] == 0 )
	   {
        break;
	   }

	  if( ( prefixNsuffix[i] < _T('!') /* '!' */ ) ||
          ( prefixNsuffix[i] > _T('z') /* '~' */ ) ||
          ( ( prefixNsuffix[i] > _T('Z'))&&( prefixNsuffix[i] < _T('a')) )
		 )
	   {
         rPrefix = FALSE;  
	   }
	}

	if( rPrefix == FALSE )
	{
      for( i = 0; i < 4; i++ )
       prefixNsuffix[i] = 0;

	  return(FALSE);
	}

	if( i == 3 ) // No null-termination
	 prefixNsuffix[3] = 0;

	return(TRUE);
}


BOOL owriButtonSendKeyStroke(OMAP_GPIO_ONE_WIRE *pOWR_OMAP, UINT8 romdata[])
{
  INT32                 i, j;
  //BOOL     rPrefix, rSuffix;
  UINT8                 ibAsciiBuf[32];
  ASCII_TO_VKEY_CONV_s  ibVkeyBuf[32];
  UINT8    buf[4];

	// pCxt->omapGpio1Wire.owriButton.iButtonMode, pCxt->omapGpio1Wire.owriButton.iButtonPrefix, pCxt->omapGpio1Wire.owriButton.iButtonSuffix
	// Analize prefix and suffix fields

   RETAILMSG(OWR_DRV_DBG, (L"owriButtonSendKeyStroke: ROM data = \r\n"));

   for (i=7; i>=0; i--)
    RETAILMSG( OWR_DRV_DBG, (L"[0x%02X]", romdata[i] )); 

   RETAILMSG(OWR_DRV_DBG, (L"\r\n"));

    RETAILMSG(OWR_DRV_DBG, (L"owriButtonSendKeyStroke: prefix = %s, suffix = %s\r\n", pOWR_OMAP->owriButton.iButtonPrefix, pOWR_OMAP->owriButton.iButtonSuffix));

	RETAILMSG(OWR_DRV_DBG, (L"owriButtonSendKeyStroke: ibAsciiBuf = "));
    i = 0;
	while( i < 3 )
	{
	  if( pOWR_OMAP->owriButton.iButtonPrefix[i] != 0 )
		ibAsciiBuf[i] = (UINT8) pOWR_OMAP->owriButton.iButtonPrefix[i];
	  else
       break;

	  RETAILMSG( OWR_DRV_DBG, (L"[0x%02X]", ibAsciiBuf[i] ));

	  i++;
	}

    ibAsciiBuf[i] = 0;

	
	j = i;
	i = 7;
	RETAILMSG(OWR_DRV_DBG, (L"j = %d\r\n", j));

    while( i >= 0 ) 
	{
	  byte2hex(romdata[i], buf);
	  RETAILMSG( OWR_DRV_DBG, (L"i = %d, j = %d, [0x%02X], [0x%02X-0x%02X]\r\n", i, j, romdata[i], buf[0], buf[1] ));
      ibAsciiBuf[j] = buf[0];
	  ibAsciiBuf[j+1] = buf[1];

	  j = j + 2;
	  i--;
	}
   
	ibAsciiBuf[j] = 0;

	RETAILMSG(OWR_DRV_DBG, (L"j = %d\r\n", j));

	//RETAILMSG(OWR_DRV_DBG, (L"owriButtonSendKeyStroke: prefix + ROM = %s\r\n", ibAsciiBuf ));

   //bugggg...... !!!!!

	i = 0;
    while( i < 3 )
	{
	  if( pOWR_OMAP->owriButton.iButtonSuffix[i] != 0 )
		ibAsciiBuf[j] = (UINT8) pOWR_OMAP->owriButton.iButtonSuffix[i];
	  else
       break;

	  RETAILMSG( OWR_DRV_DBG, (L"[0x%02X]", ibAsciiBuf[j] ));
	  i++;
	  j++;
	}

	ibAsciiBuf[j] = 0;
	RETAILMSG(OWR_DRV_DBG, (L"j = %d\r\n", j));

	//RETAILMSG(OWR_DRV_DBG, (L"owriButtonSendKeyStroke:  %s\r\n", ibAsciiBuf));

	if( convertAsciiToVirt(ibAsciiBuf, ibVkeyBuf, j ) )
	{
	#if OWR_DRV_DBG == 1
		RETAILMSG(OWR_DRV_DBG, (L"owriButtonSendKeyStroke: Virtual keys:\r\n"));
		for( i = 0; i < j; i++ )
		{
			RETAILMSG(OWR_DRV_DBG, (L"[0x%02X:%01X]\r\n", ibVkeyBuf[i].vkey, ibVkeyBuf[i].kbdReg ));
		}
		RETAILMSG(OWR_DRV_DBG, (L"\r\n"));
	#endif

		for( i = 0; i < j; i++ )
		{
		 if( ibVkeyBuf[i].kbdReg == IBTNKBD_SHIFT )
		 {
		  keybd_event(VK_SHIFT, MapVirtualKey(VK_SHIFT, 0), 0, 0);
		  keybd_event( ibVkeyBuf[i].vkey, MapVirtualKey(ibVkeyBuf[i].vkey, 0), 0, 0);
		  keybd_event( ibVkeyBuf[i].vkey, MapVirtualKey(ibVkeyBuf[i].vkey, 0), KEYEVENTF_KEYUP, 0);
		  keybd_event(VK_SHIFT, MapVirtualKey(VK_SHIFT, 0), KEYEVENTF_KEYUP,0);
		 }
		 else
		 {
		  keybd_event( ibVkeyBuf[i].vkey, MapVirtualKey(ibVkeyBuf[i].vkey, 0), 0, 0);
		  keybd_event( ibVkeyBuf[i].vkey, MapVirtualKey(ibVkeyBuf[i].vkey, 0), KEYEVENTF_KEYUP, 0);
		 }
		}
	}
	else
	{
     RETAILMSG(OWR_DRV_DBG, (L"owriButtonSendKeyStroke: convertAsciiToVirt failure\r\n"));
	 return(FALSE);
	}

	return(TRUE);
}



