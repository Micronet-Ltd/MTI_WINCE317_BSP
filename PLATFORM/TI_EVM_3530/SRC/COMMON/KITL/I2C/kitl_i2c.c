// Copyright (c) 2008 BSQUARE Corporation. All rights reserved.

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
//  File:  kitl_i2c.c
//
#include <windows.h>
#include <bsp.h>
#include <oal.h>
#include <oalex.h>
#include <i2c.h>
#include <omap35xx.h>
#include <oal_i2c.h>





static OAL_IFC_I2C      _i2cFnTable;
static BOOL             _bI2CFnTableInit = FALSE;

//-----------------------------------------------------------------------------
BOOL
OALI2CSetSlaveAddress(
    void           *hCtx, 
    UINT16          slaveAddress
    )
{
    ((I2CContext_t*)hCtx)->slaveAddress = slaveAddress;
    return TRUE;
}

//-----------------------------------------------------------------------------
void*
OALI2COpen(
    UINT            devId,
    void           *pData
    )
{    
    // get function pointers
    if (_bI2CFnTableInit == FALSE)
        {
        // get I2c function table from kernel
        if (!OEMIoControl(IOCTL_HAL_I2CCOPYFNTABLE, (void*)&_i2cFnTable,
                sizeof(OAL_IFC_I2C), NULL, 0, NULL))
            {
            KITL_RETAILMSG(ZONE_WARNING, ("OALI2COpen: "
                "Failed get i2c function table from kernel\r\n"
                ));
            return NULL;
            }
        _bI2CFnTableInit = TRUE;
        }

    return _i2cFnTable.fnI2COpen(devId, pData);
}

//------------------------------------------------------------------------------
void
OALI2CClose(
    void           *hI2C
    )
{
    _i2cFnTable.fnI2CClose(hI2C);
}

//------------------------------------------------------------------------------
UINT
OALI2CWrite(
    VOID           *hCtx,
    UINT32          subaddr,
    VOID           *pBuffer,
    UINT32          size
    )
{
    return _i2cFnTable.fnI2CWrite(hCtx, subaddr, pBuffer, size);
}

//------------------------------------------------------------------------------
UINT
OALI2CRead(
    VOID       *hCtx,
    UINT32      subaddr,
    VOID       *pBuffer,
    UINT32      size
    )
{
    return _i2cFnTable.fnI2CRead(hCtx, subaddr, pBuffer, size);
}

//------------------------------------------------------------------------------
