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
//  File:  oal_i2c.c
//
#include <windows.h>
#include <omap35xx.h>
#include <bsp_cfg.h>
#include <i2c.h>
#include <oal_i2c.h>


DWORD const I2CDefaultI2CTimeout = BSP_I2C_TIMEOUT_INIT;

//-----------------------------------------------------------------------------
I2CScaleTable_t _rgScaleTable[] = {
    { I2C_SS_PSC_VAL, I2C_SS_SCLL_VAL,  I2C_SS_SCLH_VAL }, // 100 KHz mode
    { I2C_FS_PSC_VAL, I2C_FS_SCLL_VAL,  I2C_FS_SCLH_VAL }, // 400 KHz mode (FS)
    { I2C_HS_PSC_VAL, I2C_1P6M_HSSCLL,  I2C_1P6M_HSSCLH }, // 1.6 MHz mode (HS)
    { I2C_HS_PSC_VAL, I2C_2P4M_HSSCLL,  I2C_2P4M_HSSCLH }, // 2.4 MHz mode (HS)
    { I2C_HS_PSC_VAL, I2C_3P2M_HSSCLL,  I2C_3P2M_HSSCLH }, // 3.2 MHz mode (HS)
};

//-----------------------------------------------------------------------------
I2CDevice_t _rgI2CDevice[] = 
{
    {
        BSP_I2C1_OA_INIT, BSP_I2C1_BAUDRATE_INIT,  BSP_I2C1_MAXRETRY_INIT, 
        BSP_I2C1_RX_THRESHOLD_INIT, BSP_I2C1_TX_THRESHOLD_INIT, NULL
    }, {
        BSP_I2C2_OA_INIT, BSP_I2C2_BAUDRATE_INIT,  BSP_I2C2_MAXRETRY_INIT,
        BSP_I2C2_RX_THRESHOLD_INIT, BSP_I2C2_TX_THRESHOLD_INIT, NULL
    }, {
        BSP_I2C3_OA_INIT, BSP_I2C3_BAUDRATE_INIT,  BSP_I2C3_MAXRETRY_INIT,
        BSP_I2C3_RX_THRESHOLD_INIT, BSP_I2C3_TX_THRESHOLD_INIT, NULL
    }
};

//-----------------------------------------------------------------------------