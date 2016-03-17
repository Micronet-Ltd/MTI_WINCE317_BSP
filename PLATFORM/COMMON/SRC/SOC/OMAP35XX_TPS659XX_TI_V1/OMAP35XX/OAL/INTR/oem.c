//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//------------------------------------------------------------------------------
//
//  File: oem.c
//
//  This file implements a standard implementation of OEMInterrupt functions
//  relating to enabling, disabling and finishing interrupts.
//
#include <windows.h>
#include <nkintr.h>
#include <oal_intrex.h>
#include <oal.h>
 

//#define INTR_TEST_POINT

// ##### Michael. Init GPIO_62 for test point purposes
// ##### Define GPIO_62 as GPIO in 'platform.c' of Xloader
#ifdef INTR_TEST_POINT
 #include <ceddkex.h>
 #include <gpio.h>
 #include "omap35xx.h"
 #include <interrupt_struct.h>

 #define GPIO_BITS_PER_BANK      (0x1F)
 #define GPIO_BANK(x)            (x >> 5)
 #define GPIO_BIT(x)             (x & GPIO_BITS_PER_BANK)
 #define TEST_BIT                GPIO_BIT(GPIO_62)
 #define TEST_BANK               GPIO_BANK(GPIO_62)
 #define TEST_MASK               (1 << (TEST_BIT))

 extern OMAP_INTR_CONTEXT const    *g_pIntr;

 #define TEST_GPIO_LOW           (CLRREG32(&g_pIntr->pGPIORegs[TEST_BANK]->DATAOUT, 1 << (TEST_BIT)))
 #define TEST_GPIO_HIGH          (SETREG32(&g_pIntr->pGPIORegs[TEST_BANK]->DATAOUT, 1 << (TEST_BIT)))
#else
 #define TEST_GPIO_LOW            
 #define TEST_GPIO_HIGH           
#endif /* INTR_TEST_POINT */


//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptEnable
//
//  This function enables the IRQ given its corresponding SysIntr value.
//  Function returns true if SysIntr is valid, else false.
//
BOOL OEMInterruptEnable(DWORD sysIntr, LPVOID pvData, DWORD cbData)
{
    BOOL rc = FALSE;
    const UINT32 *pIrqs;
    UINT32 count;

    OALMSG(OAL_INTR&&OAL_VERBOSE, 
        (L"+OEMInterruptEnable(%d, 0x%x, %d)\r\n", sysIntr, pvData, cbData
    ));

    // SYSINTR_VMINI & SYSINTR_TIMING are special cases
    if (sysIntr == SYSINTR_VMINI || sysIntr == SYSINTR_TIMING) {
        rc = TRUE;
        goto cleanUp;
    }

    // Obtain the SYSINTR's underlying IRQ number
    if (!OALIntrTranslateSysIntr(sysIntr, &count, &pIrqs)) {
        // Indicate invalid SysIntr
        OALMSG(OAL_ERROR, (
            L"ERROR: OEMInterruptEnable: IRQs are undefined for SysIntr %d\r\n", 
            sysIntr 
        ));
        goto cleanUp;
    }

    // Enable the interrupt
    if (pvData != NULL) OALIntrSetDataIrqs(count, pIrqs, pvData, cbData);
    rc = OALIntrEnableIrqs(count, pIrqs);

cleanUp:    
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OEMInterruptEnable(rc = 1)\r\n"));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptDisable(DWORD sysIntr)
//
//  This function disables the IRQ given its corresponding SysIntr value.
//
//
VOID OEMInterruptDisable(DWORD sysIntr)
{
    const UINT32 *pIrqs;
    UINT32 count;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+OEMInterruptDisable(%d)\r\n", sysIntr));

    // Obtain the SYSINTR's underlying IRQ number
    if (!OALIntrTranslateSysIntr(sysIntr, &count, &pIrqs)) {
        // Indicate invalid SysIntr
        OALMSG(OAL_ERROR, (
            L"ERROR: OEMInterruptEnable: IRQs are undefined for SysIntr %d\r\n", 
            sysIntr 
        ));
        goto cleanUp;
    }

    // Disable the interrupt
    OALIntrDisableIrqs(count, pIrqs);

cleanUp:
    // Indicate exit
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OEMInterruptDisable\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptMask
//
//  This function masks the IRQ given its corresponding SysIntr value.
//
//
VOID
OEMInterruptMask(
    DWORD sysIntr, 
    BOOL mask
    )
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"+OEMInterruptMask(%d, %d)\r\n", sysIntr, mask
        ));

    // Based on mask enable or disable
    if (mask)
        {
        OEMInterruptDisable(sysIntr);
        }
    else 
        {
        OEMInterruptEnable(sysIntr, NULL, 0);
        }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OEMInterruptMask\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function: OEMInterruptDone
//
//  OEMInterruptDone is called by the kernel when a device driver
//  calls InterruptDone(). The system is not preemtible when this
//  function is called.
//
VOID OEMInterruptDone(DWORD sysIntr)
{
    const UINT32 *pIrqs;
    UINT32 count;
    BOOL bPrevIntrState;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+OEMInterruptDone(%d)\r\n", sysIntr));

    // Make sure interrupts are disabled
    bPrevIntrState = INTERRUPTS_ENABLE(FALSE);
	TEST_GPIO_HIGH;
    
    if (OALIntrTranslateSysIntr(sysIntr, &count, &pIrqs)) {
        OALIntrDoneIrqs(count, pIrqs);
    }

    // Return interrupts to last state
	TEST_GPIO_LOW;
    INTERRUPTS_ENABLE(bPrevIntrState);

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OEMInterruptDone\r\n"));

}

//------------------------------------------------------------------------------
