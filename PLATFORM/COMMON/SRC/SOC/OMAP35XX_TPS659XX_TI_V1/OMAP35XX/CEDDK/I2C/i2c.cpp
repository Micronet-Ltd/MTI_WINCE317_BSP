/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
//
//  File:  i2c.cpp
//
//  This file contains DDK library implementation for platform specific
//  i2c operations.
//  
#include <windows.h>
#include <types.h>
#include <oal.h>
#include <oalex.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>
#include <i2c.h>
#include <oal_i2c.h>
#include <gpio.h>

#define I2C_CEDDK_COOKIE        ('i2cH')
#define ValidateI2C()           ((_bI2CFnTableInit == FALSE) ? I2CInitialize() : TRUE)

//-----------------------------------------------------------------------------
struct I2CContextEx_t : public I2CContext_t
{
    UINT                cookie;
	CRITICAL_SECTION	*cs;
};


//-----------------------------------------------------------------------------
static OAL_IFC_I2C      _i2cFnTable;
static BOOL         _bI2CFnTableInit = 0;
CRITICAL_SECTION	_cs_i2c[3];

//-----------------------------------------------------------------------------
static BOOL I2CInitialize()
{
    if(!_bI2CFnTableInit)
	{
        // get clock ref counter table from kernel
        if(KernelIoControl(IOCTL_HAL_I2CCOPYFNTABLE, (void*)&_i2cFnTable, sizeof(OAL_IFC_I2C), 0, 0, 0))
		{
			InitializeCriticalSection(&_cs_i2c[0]);
			InitializeCriticalSection(&_cs_i2c[1]);
			InitializeCriticalSection(&_cs_i2c[2]);
			_bI2CFnTableInit = 1;
		}
	}

    return _bI2CFnTableInit;
}



//-----------------------------------------------------------------------------
UINT I2CGetDeviceIdFromMembase(UINT memBase)
{
    switch(memBase)
	{
        case OMAP_I2C1_REGS_PA:
            return OMAP_DEVICE_I2C1;
        case OMAP_I2C2_REGS_PA:
            return OMAP_DEVICE_I2C2;
        case OMAP_I2C3_REGS_PA:
            return OMAP_DEVICE_I2C3;
	}

    return OMAP_DEVICE_NONE;
}
    

//-----------------------------------------------------------------------------
HANDLE I2COpen(UINT devId)
{
    I2CContextEx_t *pContext = 0;

    if(!ValidateI2C())
		return 0;
    
	switch(devId)
	{
        case OMAP_DEVICE_I2C1:
        case OMAP_DEVICE_I2C2:
        case OMAP_DEVICE_I2C3:
		{
			if(OMAP_DEVICE_I2C3 == devId)//enable ext i2c idI2C
			{
				HANDLE	hGpio;
				DWORD	rc		= 0;
				DWORD	GpioEn	= GPIO_177;
				hGpio = CreateFile(GPIO_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
				
				if(INVALID_HANDLE_VALUE != hGpio)
				{
					rc = DeviceIoControl(hGpio, IOCTL_GPIO_SETBIT, &GpioEn, sizeof(GpioEn), 0, 0, 0, 0 );
				    CloseHandle(hGpio);
				}
				RETAILMSG(0,(L"I2COpen: hGpio = %x, rc = %x, err = %d\r\n", hGpio, rc, GetLastError()));
				if(0 == rc)
					return 0;
				StallExecution(1);
			}
            pContext = new I2CContextEx_t();
            if(pContext)
			{
                pContext->cookie = I2C_CEDDK_COOKIE;
                _i2cFnTable.fnI2COpen(devId, pContext);
				pContext->cs = &_cs_i2c[devId - OMAP_DEVICE_I2C1];
			}
		}
		break;
	}

    return pContext;
}


//-----------------------------------------------------------------------------
void I2CSetSlaveAddress(HANDLE hContext, DWORD slaveAddress)
{
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;

    if(pContext->cookie == I2C_CEDDK_COOKIE)
	{
        pContext->slaveAddress = slaveAddress;
	}
}

//-----------------------------------------------------------------------------
void I2CSetSubAddressMode(HANDLE hContext, DWORD subAddressMode)
{
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;

    if(pContext->cookie == I2C_CEDDK_COOKIE)
	{
        pContext->subAddressMode = subAddressMode;
	}
}

//-----------------------------------------------------------------------------
void I2CSetBaudIndex(HANDLE hContext, DWORD baudIndex)
{
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;

    if(pContext->cookie == I2C_CEDDK_COOKIE)
	{
        pContext->baudIndex = baudIndex;
	}
}

//-----------------------------------------------------------------------------
void I2CSetTimeout(HANDLE hContext, DWORD timeOut)
{
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;

    if(pContext->cookie == I2C_CEDDK_COOKIE)
	{
        pContext->timeOut = timeOut;
	}
}


//-----------------------------------------------------------------------------
void I2CClose(HANDLE hContext)
{
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;
    if(pContext->cookie == I2C_CEDDK_COOKIE)
	{
		if(OMAP_DEVICE_I2C3 == pContext->idI2C + OMAP_DEVICE_I2C1)//disable ext i2c 
		{
			HANDLE	hGpio;
			DWORD	rc		= 0;
			DWORD	GpioEn	= GPIO_177;
			
			hGpio = CreateFile(GPIO_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);

			if(INVALID_HANDLE_VALUE != hGpio)
			{
				rc = DeviceIoControl(hGpio, IOCTL_GPIO_CLRBIT, &GpioEn,sizeof(GpioEn), 0, 0, NULL, NULL );
			    CloseHandle(hGpio);
			}
			RETAILMSG(0,(L"I2CClose: hGpio = %x, rc = %x err = %d\r\n", hGpio, rc, GetLastError()));
			StallExecution(1);
		}
        delete pContext;
	}
}

//-----------------------------------------------------------------------------
DWORD I2CRead(HANDLE hContext, DWORD subaddr, VOID *pBuffer, DWORD size)
{
    DWORD result = -1;
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;
    if(pContext->cookie == I2C_CEDDK_COOKIE && ValidateI2C())
    {
        result = _i2cFnTable.fnI2CRead(pContext, subaddr, pBuffer, size);
	}

    return result;
}
    
//-----------------------------------------------------------------------------
DWORD I2CWrite(HANDLE hContext, DWORD subaddr, const VOID *pBuffer, DWORD size)
{
    DWORD result = -1;
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;
    if (pContext->cookie == I2C_CEDDK_COOKIE && ValidateI2C())
        {
        result = _i2cFnTable.fnI2CWrite(pContext, subaddr, (VOID*)pBuffer, size);
        }
    return result;
}

//-----------------------------------------------------------------------------
void I2CLock(HANDLE hContext)
{
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;
	//EnterCriticalSection(pContext->cs);
    if (pContext->cookie == I2C_CEDDK_COOKIE && ValidateI2C())
        {
        _i2cFnTable.fnI2CLock(pContext->idI2C);
        }
}

//-----------------------------------------------------------------------------
void I2CUnlock(HANDLE hContext)
{
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;
	//LeaveCriticalSection(pContext->cs);
    if (pContext->cookie == I2C_CEDDK_COOKIE && ValidateI2C())
        {
        _i2cFnTable.fnI2CUnlock(pContext->idI2C);
        }
}


//-----------------------------------------------------------------------------
