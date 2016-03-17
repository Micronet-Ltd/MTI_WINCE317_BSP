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
//  Header:  bsp_tps659xx.h
//
#ifndef __BSP_TPS659XX_H
#define __BSP_TPS659XX_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Function:  InitTwlPower
//
//  Initializes the power grouping and voltage regulators on Triton2
//
VOID InitTwlPower();


//-----------------------------------------------------------------------------
//
//  Function: OALTWLxxx
//

void*
OALTritonOpen(
    );

VOID
OALTritonClose(
    void* hTWL
    );

BOOL
OALTritonWrite(
    void* hTWL,
    DWORD address,
    UCHAR data
    );


BOOL
OALTritonWriteRegs(
    void* hTwl,
    DWORD address,
    UCHAR *pData,
	DWORD  size
    );

BOOL
OALTritonRead(
    void* hTWL,
    DWORD address,
    UCHAR *pData
    );


BOOL
OALTritonReadRegs(
    void* hTwl,
    DWORD address,
    UCHAR *pData,
	DWORD  size
    );

BOOL 
SendPBMessage(
    void *hTwl,
    UCHAR power_res_id,
    UCHAR res_state
    );

#ifdef __cplusplus
}
#endif

#endif // __BSP_TPS659XX_H

