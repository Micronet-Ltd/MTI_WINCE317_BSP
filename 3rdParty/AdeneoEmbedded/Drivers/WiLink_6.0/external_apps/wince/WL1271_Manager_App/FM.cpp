/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998-2010 Texas Instruments Incorporated           |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**          
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**     
**+----------------------------------------------------------------------+**
***************************************************************************/
 
/** \file   FM.cpp 
 *  \brief  WL1271 FM device management
 *
 *  \see    FM.h
 */

#include <windows.h>
#include "FM.h"

BOOL FM_Init(void)
{
	DEBUGMSG(1, (TEXT("++FM_Init\r\n")));
	DEBUGMSG(1, (TEXT("--FM_Init\r\n")));
	return TRUE;
}

BOOL FM_SaveStartupState(int save)
{
	return 1;
}


BOOL FM_GetStatus(void)
{
	DEBUGMSG(1, (TEXT("++FM_GetStatus\r\n")));
	DEBUGMSG(1, (TEXT("--FM_GetStatus\r\n")));
	return FALSE;
}

BOOL FM_TurnOn(void)
{
	DEBUGMSG(1, (TEXT("++FM_TurnOn\r\n")));
	DEBUGMSG(1, (TEXT("--FM_TurnOn\r\n")));
	return TRUE;
}

BOOL FM_TurnOff(void)
{
	DEBUGMSG(1, (TEXT("++FM_TurnOff\r\n")));
	DEBUGMSG(1, (TEXT("--FM_TurnOff\r\n")));	
	return TRUE;
}

BOOL FM_DeInit(void)
{
	DEBUGMSG(1, (TEXT("++FM_DeInit\r\n")));
	DEBUGMSG(1, (TEXT("--FM_DeInit\r\n")));
	return TRUE;
}