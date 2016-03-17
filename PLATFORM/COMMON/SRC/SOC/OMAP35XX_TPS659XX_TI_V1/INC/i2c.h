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
//  File:  i2c.h
//
#ifndef __I2C_H
#define __I2C_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// i2c baud index
//
#define SLOWSPEED_MODE          0       // corresponds to 100 khz
#define FULLSPEED_MODE          1       // corresponds to 400 khz
#define HIGHSPEED_MODE_1P6      2       // corresponds to 1.6 mhz
#define HIGHSPEED_MODE_2P4      3       // corresponds to 2.4 mhz
#define HIGHSPEED_MODE_3P2      4       // corresponds to 3.2 mhz
#define MAX_SCALE_ENTRY         5       // number of entries in s_ScaleTable

//-----------------------------------------------------------------------------
// defines subaddress mode for a i2c client
//
#define I2C_SUBADDRESS_MODE_0   0
#define I2C_SUBADDRESS_MODE_8   1
#define I2C_SUBADDRESS_MODE_16  2
#define I2C_SUBADDRESS_MODE_24  3
#define I2C_SUBADDRESS_MODE_32  4

//------------------------------------------------------------------------------
//
//  Define:  IOCTL_DDK_GET_OAL_I2C_IFC
//
//  This IOCTL code is used to obtain the function pointers to the OAL
//  i2c functions.
//
#define IOCTL_DDK_GET_OAL_I2C_IFC \
    CTL_CODE(FILE_DEVICE_I2C, 0x0100, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    void      (*fnI2CLock)(UINT idI2C);
    void      (*fnI2CUnlock)(UINT idI2C);    
    HANDLE    (*fnI2COpen)(UINT devId, void *pData);
    void      (*fnI2CClose)(void *pData);
    UINT      (*fnI2CWrite)(void *hCtx, UINT32 subAddr, VOID *pBuffer, UINT32 size);
    UINT      (*fnI2CRead)(void *hCtx, UINT32 subAddr, VOID *pBuffer, UINT32 size);
} OAL_IFC_I2C;


//------------------------------------------------------------------------------
//
//  Global:  I2CGetDeviceIdFromMembase
//
//  returns the i2c device identifier based on a memBase value
//
UINT 
I2CGetDeviceIdFromMembase(
    UINT memBase
    );
    

//------------------------------------------------------------------------------
//
//  Global:  I2COpen
//
//  returns a i2c context handle
//
HANDLE 
I2COpen(
    UINT devId
    );

//------------------------------------------------------------------------------
//
//  Global:  I2CSetSlaveAddress
//
//  sets the slave address for an i2c context handle
//
void 
I2CSetSlaveAddress(
    HANDLE      hI2C,
    DWORD       slaveAddress
    );

//------------------------------------------------------------------------------
//
//  Global:  I2CSetSubAddressMode
//
//  sets the address mode for an i2c context handle
//
void 
I2CSetSubAddressMode(
    HANDLE      hI2C,
    DWORD       subAddressMode
    );

//------------------------------------------------------------------------------
//
//  Global:  I2CSetBaudIndex
//
//  sets the baud index for an i2c context handle
//
void 
I2CSetBaudIndex(
    HANDLE      hI2C,
    DWORD       baudIndex
    );

//------------------------------------------------------------------------------
//
//  Global:  I2CSetTimeout
//
//  sets the transaction timeout for an i2c context handle 
//
void 
I2CSetTimeout(
    HANDLE      hI2C,
    DWORD       timeOut
    );

//------------------------------------------------------------------------------
//
//  Global:  I2CClose
//
//  closes the i2c context handle 
//
void
I2CClose(
    HANDLE hContext
    );

//------------------------------------------------------------------------------
//
//  Global:  I2CRead
//
//  reads from an i2c device
//
DWORD
I2CRead(
    HANDLE hContext, 
    DWORD subaddr,
    VOID *pBuffer, 
    DWORD size
    );

//------------------------------------------------------------------------------
//
//  Global:  I2CWrite
//
//  writes to an i2c device
//
DWORD 
I2CWrite(
    HANDLE hContext, 
    DWORD subaddr,
    const VOID *pBuffer, 
    DWORD size
    );

//------------------------------------------------------------------------------
//
//  Global:  I2CLock
//
//  locks the i2c bus, needed only for atomic multiple transaction accesses
//
void I2CLock(HANDLE hContext);

//------------------------------------------------------------------------------
//
//  Global:  I2CUnlock
//
//  unlocks the i2c bus, needed only for atomic multiple transaction accesses
//
void I2CUnlock(HANDLE hContext);



//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
