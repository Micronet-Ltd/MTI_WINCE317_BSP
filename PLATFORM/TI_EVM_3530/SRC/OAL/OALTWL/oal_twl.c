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
//
// oal_twl.c
//

#include <windows.h>
#include <bsp.h>
#include <i2c.h>
#include <oal_i2c.h>


//-----------------------------------------------------------------------------
//
//  Global:  hI2C
//
//  context handle to i2c driver.  Initialized in OALTritonOpen().
//
static void    *hI2C = NULL;

//-----------------------------------------------------------------------------
//
//  Global:  I2CHandle
//
//  context handle to i2c driver.  Initialized in OALTritonOpen().
//
I2CContext_t    I2CHandle;

//------------------------------------------------------------------------------

void*
OALTritonOpen(
    )
{
    //  Open handle to I2C and just return that
    hI2C = OALI2COpen(TPS659XX_I2C_BUS_ID, &I2CHandle);
    return (void*)1;
}

//------------------------------------------------------------------------------

VOID
OALTritonClose(
    void* hTwl
    )
{
    OALI2CClose(hI2C);
    hI2C = NULL;
}

//------------------------------------------------------------------------------

BOOL
OALTritonWrite(
    void* hTwl,
    DWORD address,
    UCHAR data
    )
{
    // write address and data in one operation
    if (hI2C == NULL) return FALSE;
    OALI2CSetSlaveAddress(hI2C, HIWORD(address));
    return OALI2CWrite(hI2C, LOWORD(address), &data, sizeof(UCHAR)) != -1;
}

//------------------------------------------------------------------------------

BOOL
OALTritonRead(
    void* hTwl,
    DWORD address,
    UCHAR *pData
    )
{
    // Set register address
    if (hI2C == NULL) return FALSE;
    OALI2CSetSlaveAddress(hI2C, HIWORD(address));
    return OALI2CRead(hI2C, LOWORD(address), pData, sizeof(UCHAR)) != -1;
}

//------------------------------------------------------------------------------



BOOL
OALTritonReadRegs(
    void* hTwl,
    DWORD address,
    UCHAR *pData,
	DWORD  size
    )
{
    // Set register address
    if (hI2C == NULL) return FALSE;
    OALI2CSetSlaveAddress(hI2C, HIWORD(address));
    return OALI2CRead(hI2C, LOWORD(address), pData, size) != size;

}

//------------------------------------------------------------------------------



BOOL
OALTritonWriteRegs(
    void* hTwl,
    DWORD address,
    UCHAR *pData,
	DWORD  size
    )
{
    // Set register address
    if (hI2C == NULL) return FALSE;
    OALI2CSetSlaveAddress(hI2C, HIWORD(address));
    return OALI2CWrite(hI2C, LOWORD(address), pData, size) != size;

}