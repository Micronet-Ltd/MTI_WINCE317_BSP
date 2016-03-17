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
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
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

/** \file  TWDriverRadio.c 
 *  \brief TI WLAN BIT
 *
 *  \see   TWDriver.h 
 */

#define __FILE_ID__  FILE_ID_119
#include "TWDriver.h"
#include "osApi.h"
#include "TWDriverInternal.h"
#include "CmdBld.h"
/*****************************************************************************
*                                                                            *
*                       Static functions                                     *
*                                                                            *
******************************************************************************/



/*****************************************************************************
*                                                                            *
*                       API functions	                                     *
*                                                                            *
******************************************************************************/
/****************************************************************************************
 *                        TWDriverTestCB                                    
 ****************************************************************************************/
void TWDriverTestCB(TI_HANDLE hTWD, 
					TI_STATUS eStatus, 
					TI_HANDLE pTestCmdParams)
{
	TTwd *pTWD = (TTwd *)hTWD;

	if (pTWD->pRadioCb != NULL)
	{
	os_memoryCopy(NULL, pTWD->pRadioCb, &pTWD->testCmd.testCmd_u, sizeof(pTWD->testCmd.testCmd_u));
	}

	if (pTWD->fRadioCb != NULL)
	{
	pTWD->fRadioCb(pTWD->hRadioCb, eStatus, pTWD->pRadioCb);
}
}

/****************************************************************************************
 *                        TWDriverTest                                    
 ****************************************************************************************/
TI_STATUS TWDriverTest(TI_HANDLE hTWD, 
					   TestCmdID_enum eTestCmd, 
					   void* pTestCmdParams, 
					   TTestCmdCB fCb, 
					   TI_HANDLE hCb)
{
	TTwd *pTWD = (TTwd *)hTWD;

	/* check parameters */
	if (( hTWD == NULL ) ||
		( eTestCmd >= MAX_TEST_CMD_ID ) ||
		( fCb == NULL ) ||
		( hCb == NULL ))
	{
		return (TI_NOK);
	}

	pTWD->testCmd.testCmdId = eTestCmd;

	if (pTestCmdParams != NULL)
	{
	os_memoryCopy(NULL, &pTWD->testCmd.testCmd_u, pTestCmdParams, sizeof(pTWD->testCmd.testCmd_u));
	}

	pTWD->fRadioCb = fCb;
	pTWD->pRadioCb = pTestCmdParams;
	pTWD->hRadioCb = hCb;

	return(cmdBld_CmdTest (pTWD->hCmdBld, 
						   (TI_HANDLE)TWDriverTestCB, 
						   hTWD, 
						   &pTWD->testCmd));
}
