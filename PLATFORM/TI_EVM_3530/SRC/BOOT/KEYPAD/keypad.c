/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
/*
================================================================================

 Copyright (c) 2013, Micronet Corporation. All rights reserved.

 History of Changes:
 Anna R.              03.2009  Adaptation to M5000 platform.
 Vladimir Zatulovsky CE317, Oct - 2013 
================================================================================
*/
//  File:  keypad.c
//

#include <windows.h>
#include <bsp.h>
#include <bsp_keypad.h>
#include <keypad.h>

#define MAX_KEYPAD_COLUMNS 0

//------------------------------------------------------------------------------
//
UINT64 OALReadKeyPad() 
{
    UINT64 keysDown = 0;
	UINT32 to;
    void   *hTWL;
	UINT8  matrix[KEYPAD_ROWS];
    UCHAR  nCol, row, key;
	UINT8  state;

	BOOL ret;

    hTWL = OALTritonOpen();
    if(hTWL)
    {
		// reset keypad
		//
		OALTritonWrite(hTWL, TWL_KEYP_CTRL_REG, 0x0 );
		to = 100 + OALGetTickCount();
		while(OALGetTickCount() < to);
         
		OALTritonWrite(hTWL, TWL_KEYP_CTRL_REG, TWL_KBD_CTRL_KBD_ON | TWL_KBD_CTRL_NRESET |TWL_KBD_CTRL_NSOFT_MODE);

		to = 100 + OALGetTickCount();
		while(OALGetTickCount() < to);

		OALTritonWrite(hTWL, TWL_KEY_DEB_REG, 0x04);
		OALTritonWrite(hTWL, TWL_KEYP_IMR1, 0x0f);

		to = OALGetTickCount() + 100;
		do
		{
			OALTritonRead(hTWL, TWL_KEYP_SMS, &key);
			if(to < OALGetTickCount())
			{
				OALTritonClose(hTWL);
				return 0;
			}
		}while(!(key & 1));

		// read  amount of rows...
		ret = OALTritonReadRegs(hTWL, TWL_FULL_CODE_7_0, matrix, sizeof(matrix));

		for(row = 0; row < KEYPAD_ROWS; row++)
		{
			state = matrix[row];
			if(!state)
				continue;

			for(nCol = 0; nCol < KEYPAD_COLUMNS; nCol++)
			{
				if(state & (1 << nCol))
					keysDown |=  ((UINT64)1 << (nCol * MAX_KEYPAD_COLUMNS + row));
		   }
		}
	}
		
    // We are done with Triton
    OALTritonClose(hTWL);
	
	OALMSG(0, (L"keysDown = %08x%08x\r\n", (UINT32)(keysDown >> 32), (UINT32)keysDown));
	
	return keysDown;
}