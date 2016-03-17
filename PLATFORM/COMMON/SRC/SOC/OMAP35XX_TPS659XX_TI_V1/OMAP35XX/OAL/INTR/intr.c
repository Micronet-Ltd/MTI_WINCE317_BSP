// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

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
//  File: intr.c
//
//  This file implement major part of interrupt module for OMAP7xx SoC.
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <bus.h>
#include <oal_prcm.h>
#include <oal_intrex.h>
#include <interrupt_struct.h>

// #define INTR_TEST_POINT

// ##### Michael. Init GPIO_62 for test point purposes
// ##### Define GPIO_62 as GPIO in 'platform.c' of Xloader
#ifdef INTR_TEST_POINT
 #include <ceddkex.h>
 #include <gpio.h>

 #define GPIO_BITS_PER_BANK      (0x1F)
 #define GPIO_BANK(x)            (x >> 5)
 #define GPIO_BIT(x)             (x & GPIO_BITS_PER_BANK)
 #define TEST_BIT                GPIO_BIT(GPIO_62)
 #define TEST_BANK               GPIO_BANK(GPIO_62)
 #define TEST_MASK               (1 << (TEST_BIT))

 #define TEST_GPIO_LOW           (CLRREG32(&s_intr.pGPIORegs[TEST_BANK]->DATAOUT, 1 << (TEST_BIT)))
 #define TEST_GPIO_HIGH          (SETREG32(&s_intr.pGPIORegs[TEST_BANK]->DATAOUT, 1 << (TEST_BIT)))
#else
 #define TEST_GPIO_LOW            
 #define TEST_GPIO_HIGH           
#endif /* INTR_TEST_POINT */

// ##### Michael, 6-Dec-2009
//Do not use Virtual IRQ #include <oal_viirq.h>

//------------------------------------------------------------------------------
//
//  Global: g_oalTimerIrq 
//
UINT32 g_oalTimerIrq = OAL_INTR_IRQ_UNDEFINED;

//------------------------------------------------------------------------------
//
//  Global: g_oalPrcmIrq 
//
UINT32 g_oalPrcmIrq = OAL_INTR_IRQ_UNDEFINED;

//------------------------------------------------------------------------------
//
//  Global: g_oalSmartReflex1 
//
UINT32 g_oalSmartReflex1 = OAL_INTR_IRQ_UNDEFINED;

//------------------------------------------------------------------------------
//
//  Global: g_oalSmartReflex2 
//
UINT32 g_oalSmartReflex2 = OAL_INTR_IRQ_UNDEFINED;

//------------------------------------------------------------------------------
//
//  Static: s_intr
//
//  This value contains virtual uncached address of interrupt controller
//  unit registers.
//
static OMAP_INTR_CONTEXT  s_intr;

//------------------------------------------------------------------------------
//
//  Static: g_pIntr
//
//  exposes pointer to interrupt structure.
//
OMAP_INTR_CONTEXT const *g_pIntr = &s_intr;


//------------------------------------------------------------------------------
//
//  Static: g_intcLxLevel
//
//  Following arrays contain interrupt routing, level and priority
//  initialization values for ILR interrupt controller registers.
//
static UINT32 s_icL1Level[] = {
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 0
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 2
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 4
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 6
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 8
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 10
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 12
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 14
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 16
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 18
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 20
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 22
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 24
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 26
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 28
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 30

    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 32/0
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 34/2
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI1,    // 36/4  gptimer1 prio=0
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 38/6
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 40/8
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 42/10
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 44/12
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 46/14
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 48/16
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 50/18
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 52/20
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 54/22
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 56/24
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 58/26
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 60/28
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 62/30

    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 64/0
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 66/2
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 68/4
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 70/6
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 72/8
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 74/10
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 76/12
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 78/14
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 80/16
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 82/18
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 84/20
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 86/22
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 88/24
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 90/26
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16,   // 92/28
    IC_ILR_IRQ|IC_ILR_PRI16, IC_ILR_IRQ|IC_ILR_PRI16    // 94/30
 };


//------------------------------------------------------------------------------
//
//  Function:  OALIntrInit
//
//  This function initialize OMAP interrupt subsystem. Implementation must
//  use its own mapping structure because general implementation limits
//  number of IRQ to 64 but OMAP35XX has 320 IRQs.
//
BOOL OALIntrInit()
{
    BOOL rc = FALSE;
    UINT32 i, mask;
    
    OALMSG(OAL_INTR&&OAL_FUNC, (L"+OALInterruptInit\r\n"));

    // Initialize interrupt mapping
    OALIntrMapInit();

    // Get interrupt controller and GPIO registers' virtual uncached addresses
    s_intr.pICLRegs = OALPAtoUA(OMAP_INTC_MPU_REGS_PA);
    s_intr.pGPIORegs[0] = OALPAtoUA(OMAP_GPIO1_REGS_PA);
    s_intr.pGPIORegs[1] = OALPAtoUA(OMAP_GPIO2_REGS_PA);
    s_intr.pGPIORegs[2] = OALPAtoUA(OMAP_GPIO3_REGS_PA);
    s_intr.pGPIORegs[3] = OALPAtoUA(OMAP_GPIO4_REGS_PA);
    s_intr.pGPIORegs[4] = OALPAtoUA(OMAP_GPIO5_REGS_PA);
    s_intr.pGPIORegs[5] = OALPAtoUA(OMAP_GPIO6_REGS_PA);

	OALMSG(0, (L"OALInterruptInit: OMAP_GPIO2_REGS UA = 0x%X\r\n", s_intr.pGPIORegs[1]));

    //Reset the MPU INTC and wait until reset is complete
    SETREG32(&s_intr.pICLRegs->INTC_SYSCONFIG, SYSCONFIG_SOFTRESET);
    while ((INREG32(&s_intr.pICLRegs->INTC_SYSSTATUS) & SYSSTATUS_RESETDONE) == 0);

    // Disable auto-idle for the interrupt controller
    CLRREG32(&s_intr.pICLRegs->INTC_SYSCONFIG, SYSCONFIG_AUTOIDLE);

    //Disable all interrupts and clear the ISR - for all for GPIO banks, too
    OUTREG32(&s_intr.pICLRegs->INTC_MIR_SET0, OMAP_MPUIC_MASKALL);
    OUTREG32(&s_intr.pICLRegs->INTC_MIR_SET1, OMAP_MPUIC_MASKALL);
    OUTREG32(&s_intr.pICLRegs->INTC_MIR_SET2, OMAP_MPUIC_MASKALL);

    //Assume all GPIO's are accessible
    
    //Reset and Disable interrupt/wakeup for all GPIOs
    for (i = 0; i < OMAP_GPIO_BANK_COUNT; i++)
        {
        //Disable interrupt/wakeup
        OUTREG32(&s_intr.pGPIORegs[i]->IRQENABLE1, 0x00000000);
        OUTREG32(&s_intr.pGPIORegs[i]->IRQENABLE2, 0x00000000);
        OUTREG32(&s_intr.pGPIORegs[i]->WAKEUPENABLE, 0x00000000);

        // clear irq status bits
        mask = INREG32(&s_intr.pGPIORegs[i]->IRQSTATUS1);
        OUTREG32(&s_intr.pGPIORegs[i]->IRQSTATUS1, mask);

        mask = INREG32(&s_intr.pGPIORegs[i]->IRQSTATUS2);
        OUTREG32(&s_intr.pGPIORegs[i]->IRQSTATUS2, mask);

        //Enable smart and auto idle for GPIO 
        //(We don't need to set INTC since INTC is always in smart mode)
        OUTREG32(&s_intr.pGPIORegs[i]->SYSCONFIG, 
            SYSCONFIG_AUTOIDLE| SYSCONFIG_ENAWAKEUP | SYSCONFIG_SMARTIDLE
            );
        
        }

#ifdef INTR_TEST_POINT
	// ##### Michael. Init GPIO_62 for test point purposes
	{
	 // Set Output direction
	 CLRREG32(&s_intr.pGPIORegs[TEST_BANK]->OE, TEST_MASK);

	 // Debounce disable
	 CLRREG32(&s_intr.pGPIORegs[TEST_BANK]->DEBOUNCENABLE, TEST_MASK);

	 // Clear Bit
	 CLRREG32(&s_intr.pGPIORegs[TEST_BANK]->DATAOUT, 1 << (TEST_BIT));
	}
#endif /* INTR_TEST_POINT */

    // clear any possible pending interrupts
    INREG32(&s_intr.pICLRegs->INTC_SIR_IRQ);
    INREG32(&s_intr.pICLRegs->INTC_SIR_FIQ);
   
    //Initialize interrupt routing, level and priority
    for (i = 0; i < 96; i++)
        {
        OUTREG32(&s_intr.pICLRegs->INTC_ILR[i], s_icL1Level[i]);
        }
    
    //Call board specific initializatrion
    rc = BSPIntrInit();

    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALInterruptInit(rc = %d)\r\n", rc));
    return rc;

}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrRequestIrq
//
//  This function returns IRQs for CPU/SoC devices based on their
//  physical address.
//
BOOL
OALIntrRequestIrqs(
    DEVICE_LOCATION *pDevLoc,
    UINT32 *pCount,
    UINT32 *pIrqs
    )
{
    BOOL rc = FALSE;

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrRequestIrqs(0x%08x->%d/%d/0x%08x/%d, 0x%08x, 0x%08x)\r\n",
        pDevLoc, pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc,
        pDevLoc->Pin, pCount, pIrqs
        ));

    // This shouldn't happen
    if (*pCount < 1) goto cleanUp;

    switch (pDevLoc->IfcType)
        {
        case Internal:
            /*
            switch ((ULONG)pDevLoc->LogicalLoc)
                {
                case OMAP35XX_MCSPI1_REGS_PA:
                    *pCount = 1;
                    pIrqs[0] = IRQ_SPI1;
                    rc = TRUE;
                    break;
                case OMAP35XX_MCSPI2_REGS_PA:
                    *pCount = 1;
                    pIrqs[0] = IRQ_SPI2;
                    rc = TRUE;
                    break;
                case OMAP35XX_UART1_REGS_PA:
                    *pCount = 1;
                    pIrqs[0] = IRQ_UART1;
                    rc = TRUE;
                    break;
                case OMAP35XX_UART2_REGS_PA:
                    *pCount = 1;
                    pIrqs[0] = IRQ_UART2;
                    rc = TRUE;
                    break;
                case OMAP35XX_UART3_REGS_PA:
                    *pCount = 1;
                    pIrqs[0] = IRQ_UART3;
                    rc = TRUE;
                    break;
                case OMAP35XX_UART4_REGS_PA:
                    *pCount = 1;
                    pIrqs[0] = IRQ_UART4;
                    rc = TRUE;
                    break;
                }
                */
            break;
        }

    if (!rc) rc = BSPIntrRequestIrqs(pDevLoc, pCount, pIrqs);

cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrRequestIrqs(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntrSetDataIrqs
//
void
OALIntrSetDataIrqs(
    UINT32 count,
    const UINT32 *pIrqs, 
    LPVOID pvData, 
    DWORD cbData
    )
{
    UINT32 i;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"+OALIntrSetDataIrqs(%d, 0x%08x, 0x%08X, %d)\r\n", 
        count, pIrqs, pvData, cbData
        ));

    for (i = 0; i < count; i++)
        {
        if (96 < pIrqs[i] && pIrqs[i] < IRQ_SW_RESERVED_MAX)
            {
            // call software irq handler
            // OALSWIntrSetDataIrq(pIrqs[i], pvData, cbData);
            }
        }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALIntrSetDataIrqs()\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntrEnableIrqs
//
BOOL
OALIntrEnableIrqs(
    UINT32 count,
    const UINT32 *pIrqs
    )
{
    BOOL rc = FALSE;
    UINT32 irq, i;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"+OALntrEnableIrqs(%d, 0x%08x)\r\n", count, pIrqs
        ));

    for (i = 0; i < count; i++)
        {
        irq = pIrqs[i];

        if (irq != OAL_INTR_IRQ_UNDEFINED)
            {
        if (irq < 32)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_CLEAR0, 1 << irq);
            }
        else if (irq < 64)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_CLEAR1, 1 << (irq - 32));
            }
        else if (irq < 96)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_CLEAR2, 1 << (irq - 64));
            }
        else if (irq < IRQ_SW_RESERVED_MAX )
            {
                // call software irq handler
                // OALSWIntrEnableIrq(irq);
            }
        else if (irq <= IRQ_GPIO_31)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_CLEAR0, 1<< IRQ_GPIO1_MPU);
            OUTREG32(&s_intr.pGPIORegs[0]->SETIRQENABLE1, 1<<(irq - IRQ_GPIO_0));   // unmask IRQ
            OUTREG32(&s_intr.pGPIORegs[0]->SETWAKEUPENA,  1<<(irq - IRQ_GPIO_0));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), TRUE);
            }
        else if (irq <= IRQ_GPIO_63)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_CLEAR0, 1<< IRQ_GPIO2_MPU);
            OUTREG32(&s_intr.pGPIORegs[1]->SETIRQENABLE1, 1<<(irq - IRQ_GPIO_32));  // unmask IRQ
            OUTREG32(&s_intr.pGPIORegs[1]->SETWAKEUPENA,  1<<(irq - IRQ_GPIO_32));        
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), TRUE);
            }
        else if (irq <= IRQ_GPIO_95)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_CLEAR0, 1<< IRQ_GPIO3_MPU);
            OUTREG32(&s_intr.pGPIORegs[2]->SETIRQENABLE1, 1<<(irq - IRQ_GPIO_64));  // unmask IRQ
            OUTREG32(&s_intr.pGPIORegs[2]->SETWAKEUPENA,  1<<(irq - IRQ_GPIO_64));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), TRUE);
            }
        else if (irq <= IRQ_GPIO_127)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_CLEAR1, 1<<(IRQ_GPIO4_MPU - IRQ_GPIO4_MPU));
            OUTREG32(&s_intr.pGPIORegs[3]->SETIRQENABLE1, 1<<(irq - IRQ_GPIO_96));  // unmask IRQ  
            OUTREG32(&s_intr.pGPIORegs[3]->SETWAKEUPENA,  1<<(irq - IRQ_GPIO_96));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), TRUE);
            }
        else if (irq <= IRQ_GPIO_159)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_CLEAR1, 1<<(IRQ_GPIO5_MPU - IRQ_GPIO4_MPU));
            OUTREG32(&s_intr.pGPIORegs[4]->SETIRQENABLE1, 1<<(irq - IRQ_GPIO_128));  // unmask IRQ
            OUTREG32(&s_intr.pGPIORegs[4]->SETWAKEUPENA,  1<<(irq - IRQ_GPIO_128));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), TRUE);
            }
        else if (irq <= IRQ_GPIO_191)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_CLEAR1, 1<<(IRQ_GPIO6_MPU - IRQ_GPIO4_MPU));
            OUTREG32(&s_intr.pGPIORegs[5]->SETIRQENABLE1, 1<<(irq - IRQ_GPIO_160));  // unmask IRQ
            OUTREG32(&s_intr.pGPIORegs[5]->SETWAKEUPENA,  1<<(irq - IRQ_GPIO_160));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), TRUE);
            }
            rc = TRUE;
            }
        }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALIntrEnableIrqs(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntrDisableIrqs
//
VOID
OALIntrDisableIrqs(
    UINT32 count,
    const UINT32 *pIrqs
    )
{
    UINT32 irq, i;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"+OALIntrDisableIrqs(%d, 0x%08x)\r\n", count, pIrqs
        ));

    for (i = 0; i < count; i++)
        {
        irq = pIrqs[i];

        if (irq != OAL_INTR_IRQ_UNDEFINED)
            {            
        if (irq < 32)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_SET0, 1 << irq);
            }
        else if (irq < 64)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_SET1, 1 << (irq - 32));
                }
        else if (irq < 96)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_SET2, 1 << (irq - 64));
            }
        else if (irq < IRQ_SW_RESERVED_MAX )
            {
                // call software irq handler
                // OALSWIntrDisableIrq(irq);
            }
        else if (irq <= IRQ_GPIO_31)
            {
            OUTREG32(&s_intr.pGPIORegs[0]->CLEARIRQENABLE1, 1<<(irq - IRQ_GPIO_0));   // mask IRQ
            OUTREG32(&s_intr.pGPIORegs[0]->CLEARWAKEUPENA,  1<<(irq - IRQ_GPIO_0));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), FALSE);
            }
        else if (irq <= IRQ_GPIO_63)
            {
            OUTREG32(&s_intr.pGPIORegs[1]->CLEARIRQENABLE1, 1<<(irq - IRQ_GPIO_32));  // mask IRQ
            OUTREG32(&s_intr.pGPIORegs[1]->CLEARWAKEUPENA,  1<<(irq - IRQ_GPIO_32));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), FALSE);
            }
        else if (irq <= IRQ_GPIO_95)
            {
            OUTREG32(&s_intr.pGPIORegs[2]->CLEARIRQENABLE1, 1<<(irq - IRQ_GPIO_64));  // mask IRQ
            OUTREG32(&s_intr.pGPIORegs[2]->CLEARWAKEUPENA,  1<<(irq - IRQ_GPIO_64));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), FALSE);
            }
        else if (irq <= IRQ_GPIO_127)
            {
            OUTREG32(&s_intr.pGPIORegs[3]->CLEARIRQENABLE1, 1<<(irq - IRQ_GPIO_96));  // mask IRQ  
            OUTREG32(&s_intr.pGPIORegs[3]->CLEARWAKEUPENA,  1<<(irq - IRQ_GPIO_96));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), FALSE);
            }
        else if (irq <= IRQ_GPIO_159)
            {
            OUTREG32(&s_intr.pGPIORegs[4]->CLEARIRQENABLE1, 1<<(irq - IRQ_GPIO_128));  // mask IRQ
            OUTREG32(&s_intr.pGPIORegs[4]->CLEARWAKEUPENA,  1<<(irq - IRQ_GPIO_128));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), FALSE);
            }
        else if (irq <= IRQ_GPIO_191)
            {
            OUTREG32(&s_intr.pGPIORegs[5]->CLEARIRQENABLE1, 1<<(irq - IRQ_GPIO_160));  // mask IRQ
            OUTREG32(&s_intr.pGPIORegs[5]->CLEARWAKEUPENA,  1<<(irq - IRQ_GPIO_160));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), FALSE);
            }
        }
        }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALIntrDisableIrqs\r\n"));
}
    

//------------------------------------------------------------------------------
//
//  Function:  OALIntrDoneIrqs
//
VOID
OALIntrDoneIrqs(
    UINT32 count,
    const UINT32 *pIrqs
    )
{
    BOOL rc = FALSE;
    UINT32 irq, i;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"+OALIntrDoneIrqs(%d, 0x%08x)\r\n", count, pIrqs
        ));

    for (i = 0; i < count; i++)
        {
        irq = pIrqs[i];

        if (irq != OAL_INTR_IRQ_UNDEFINED)
            {  
        if (irq < 32)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_CLEAR0, 1 << irq);
            }
        else if (irq < 64)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_CLEAR1, 1 << (irq - 32));
                }
        else if (irq < 96)
            {
            OUTREG32(&s_intr.pICLRegs->INTC_MIR_CLEAR2, 1 << (irq - 64));
            }
        else if (irq < IRQ_SW_RESERVED_MAX )
            {
                // call software irq handler
                // OALSWIntrDoneIrq(irq);
            }
        else if (irq <= IRQ_GPIO_31)
            {
            OUTREG32(&s_intr.pGPIORegs[0]->SETIRQENABLE1, 1<<(irq - IRQ_GPIO_0));   // unmask IRQ
            OUTREG32(&s_intr.pGPIORegs[0]->SETWAKEUPENA,  1<<(irq - IRQ_GPIO_0));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), TRUE);
            }
        else if (irq <= IRQ_GPIO_63)
            {
            OUTREG32(&s_intr.pGPIORegs[1]->SETIRQENABLE1, 1<<(irq - IRQ_GPIO_32));  // unmask IRQ
            OUTREG32(&s_intr.pGPIORegs[1]->SETWAKEUPENA,  1<<(irq - IRQ_GPIO_32));      
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), TRUE);
            }
        else if (irq <= IRQ_GPIO_95)
            {
            OUTREG32(&s_intr.pGPIORegs[2]->SETIRQENABLE1, 1<<(irq - IRQ_GPIO_64));  // unmask IRQ
            OUTREG32(&s_intr.pGPIORegs[2]->SETWAKEUPENA,  1<<(irq - IRQ_GPIO_64));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), TRUE);
            }
        else if (irq <= IRQ_GPIO_127)
            {
            OUTREG32(&s_intr.pGPIORegs[3]->SETIRQENABLE1, 1<<(irq - IRQ_GPIO_96));  // unmask IRQ  
            OUTREG32(&s_intr.pGPIORegs[3]->SETWAKEUPENA,  1<<(irq - IRQ_GPIO_96));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), TRUE);
            }
        else if (irq <= IRQ_GPIO_159)
            {
            OUTREG32(&s_intr.pGPIORegs[4]->SETIRQENABLE1, 1<<(irq - IRQ_GPIO_128));  // unmask IRQ
            OUTREG32(&s_intr.pGPIORegs[4]->SETWAKEUPENA,  1<<(irq - IRQ_GPIO_128));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), TRUE);
            }
        else if (irq <= IRQ_GPIO_191)
            {
            OUTREG32(&s_intr.pGPIORegs[5]->SETIRQENABLE1, 1<<(irq - IRQ_GPIO_160));  // unmask IRQ
            OUTREG32(&s_intr.pGPIORegs[5]->SETWAKEUPENA,  1<<(irq - IRQ_GPIO_160));
            OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), TRUE);
            }
            rc = TRUE;
        }
        }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALIntrDoneIrqs\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrIsIrqPending
//
//  This function checks if the given interrupt is pending.
//
BOOL
OALIntrIsIrqPending(
    UINT32 irq
    )
{
    BOOL rc = FALSE;

    if (irq < 32)
        {
        rc = INREG32(&s_intr.pICLRegs->INTC_ITR0) & (1 << irq);
        }
    else if (irq < 64)
        {
        rc = INREG32(&s_intr.pICLRegs->INTC_ITR1) & (1 << (irq - 32));
        }
    else if (irq < 96)
        {
        rc = INREG32(&s_intr.pICLRegs->INTC_ITR2) & (1 << (irq - 64));
        }
    else if (irq < IRQ_SW_RESERVED_MAX )
        {
            //  SW triggered interrupts only - none will be pending
            rc = FALSE;
        }
    else if (irq <= IRQ_GPIO_31)
        {
        rc = INREG32(&s_intr.pGPIORegs[0]->IRQSTATUS1) & (1 << (irq - IRQ_GPIO_0));
        }
    else if (irq <= IRQ_GPIO_63)
        {
        rc = INREG32(&s_intr.pGPIORegs[1]->IRQSTATUS1) & (1 << (irq - IRQ_GPIO_32));    
        }
    else if (irq <= IRQ_GPIO_95)
        {
        rc = INREG32(&s_intr.pGPIORegs[2]->IRQSTATUS1) & (1 << (irq - IRQ_GPIO_64));
        }
    else if (irq <= IRQ_GPIO_127)
        {
        rc = INREG32(&s_intr.pGPIORegs[3]->IRQSTATUS1) & (1 << (irq - IRQ_GPIO_96));
        }
    else if (irq <= IRQ_GPIO_159)
        {
        rc = INREG32(&s_intr.pGPIORegs[4]->IRQSTATUS1) & (1 << (irq - IRQ_GPIO_128));
        }
    else if (irq <= IRQ_GPIO_191)
        {
        rc = INREG32(&s_intr.pGPIORegs[5]->IRQSTATUS1) & (1 << (irq - IRQ_GPIO_160));
        }
    
    return (rc != 0);
}


//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptHandler
//
//  This is interrupt handler implementation.
//
extern void OALDrainWriteBuffer(void);
UINT32
OEMInterruptHandler(
    UINT32 ra
    )
{
    UINT32 irq = OAL_INTR_IRQ_UNDEFINED;
    UINT32 sysIntr = SYSINTR_NOP;
    UINT32 mask,status;
	UINT32 irqGpioMpu = 0;

    //TEST_GPIO_HIGH;

    if (g_oalILT.active) g_oalILT.interrupts++;

    // Get pending interrupt
    irq = INREG32(&s_intr.pICLRegs->INTC_SIR_IRQ);

    OALMSG(OAL_INTR, (L"OEMInterruptHandler(Irq %d)\r\n", irq));


	// In case of GPIO interrupts, mask appropriate GPIO register(s) too
	// Don't clear GPIO status register. Let Installable Interrupt verify
	// the GPIO Interrupt source!
	switch(irq)
	{
	 case IRQ_GPIO1_MPU:
	 {
		  irqGpioMpu = irq;
        // mask status with irq enabled GPIO's to make sure
        // only interrupts which generated a new interrupt is 
        // handled
        status = INREG32(&s_intr.pGPIORegs[0]->IRQSTATUS1);
        status &= INREG32(&s_intr.pGPIORegs[0]->IRQENABLE1);
        for (irq = IRQ_GPIO_0, mask = 1; mask != 0; mask <<= 1, irq++)
            {
            if ((mask & status) != 0) break;
            }
       // OUTPORT32(&s_intr.pGPIORegs[0]->IRQSTATUS1, mask);
       // OUTPORT32(&s_intr.pGPIORegs[0]->IRQSTATUS2, mask);
        OUTPORT32(&s_intr.pGPIORegs[0]->CLEARIRQENABLE1, mask);
        OUTPORT32(&s_intr.pGPIORegs[0]->CLEARWAKEUPENA, mask);
        OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), FALSE);
	 
	 } break;
	 
	 case IRQ_GPIO2_MPU:
	 {
          irqGpioMpu = irq;
        // mask status with irq enabled GPIO's to make sure
        // only interrupts which generated a new interrupt is 
        // handled
        status = INREG32(&s_intr.pGPIORegs[1]->IRQSTATUS1);
        status &= INREG32(&s_intr.pGPIORegs[1]->IRQENABLE1);
        for (irq = IRQ_GPIO_32, mask = 1; mask != 0; mask <<= 1, irq++)
            {
            if ((mask & status) != 0) break;
            }
        //OUTPORT32(&s_intr.pGPIORegs[1]->IRQSTATUS1, mask);
        //OUTPORT32(&s_intr.pGPIORegs[1]->IRQSTATUS2, mask);
        OUTPORT32(&s_intr.pGPIORegs[1]->CLEARIRQENABLE1, mask);
        OUTPORT32(&s_intr.pGPIORegs[1]->CLEARWAKEUPENA, mask);        
        OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), FALSE);
	 
	 } break;
	 
	 case IRQ_GPIO3_MPU:
	 {
          irqGpioMpu = irq;
        // mask status with irq enabled GPIO's to make sure
        // only interrupts which generated a new interrupt is 
        // handled
        status = INREG32(&s_intr.pGPIORegs[2]->IRQSTATUS1);
        status &= INREG32(&s_intr.pGPIORegs[2]->IRQENABLE1);
        for (irq = IRQ_GPIO_64, mask = 1; mask != 0; mask <<= 1, irq++)
            {
            if ((mask & status) != 0) break;
            }
        //OUTPORT32(&s_intr.pGPIORegs[2]->IRQSTATUS1, mask);
        //OUTPORT32(&s_intr.pGPIORegs[2]->IRQSTATUS2, mask);
        OUTPORT32(&s_intr.pGPIORegs[2]->CLEARIRQENABLE1, mask);
        OUTPORT32(&s_intr.pGPIORegs[2]->CLEARWAKEUPENA, mask);        
        OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), FALSE);
	 
	 } break;
	 
	 case IRQ_GPIO4_MPU:
	 {
		  irqGpioMpu = irq;
        // mask status with irq enabled GPIO's to make sure
        // only interrupts which generated a new interrupt is 
        // handled
        status = INREG32(&s_intr.pGPIORegs[3]->IRQSTATUS1);
        status &= INREG32(&s_intr.pGPIORegs[3]->IRQENABLE1);
        for (irq = IRQ_GPIO_96, mask = 1; mask != 0; mask <<= 1, irq++)
            {
            if ((mask & status) != 0) break;
            }
        //OUTPORT32(&s_intr.pGPIORegs[3]->IRQSTATUS1, mask);
        //OUTPORT32(&s_intr.pGPIORegs[3]->IRQSTATUS2, mask);
        OUTPORT32(&s_intr.pGPIORegs[3]->CLEARIRQENABLE1, mask);
        OUTPORT32(&s_intr.pGPIORegs[3]->CLEARWAKEUPENA, mask);        
        OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), FALSE);
	 
	 } break;
	 
	 case IRQ_GPIO5_MPU:
	 {
          irqGpioMpu = irq;
        // mask status with irq enabled GPIO's to make sure
        // only interrupts which generated a new interrupt is 
        // handled
        status = INREG32(&s_intr.pGPIORegs[4]->IRQSTATUS1);
        status &= INREG32(&s_intr.pGPIORegs[4]->IRQENABLE1);
        for (irq = IRQ_GPIO_128, mask = 1; mask != 0; mask <<= 1, irq++)
            {
            if ((mask & status) != 0) break;
            }
        //OUTPORT32(&s_intr.pGPIORegs[4]->IRQSTATUS1, mask);
        //OUTPORT32(&s_intr.pGPIORegs[4]->IRQSTATUS2, mask);
        OUTPORT32(&s_intr.pGPIORegs[4]->CLEARIRQENABLE1, mask);
        OUTPORT32(&s_intr.pGPIORegs[4]->CLEARWAKEUPENA, mask); 
        OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), FALSE);
	 
	 } break;
	 
	 case IRQ_GPIO6_MPU:
	 {
          irqGpioMpu = irq; 
        // mask status with irq enabled GPIO's to make sure
        // only interrupts which generated a new interrupt is 
        // handled
        status = INREG32(&s_intr.pGPIORegs[5]->IRQSTATUS1);
        status &= INREG32(&s_intr.pGPIORegs[5]->IRQENABLE1);
        for (irq = IRQ_GPIO_160, mask = 1; mask != 0; mask <<= 1, irq++)
            {
            if ((mask & status) != 0) break;
            }
        //OUTPORT32(&s_intr.pGPIORegs[5]->IRQSTATUS1, mask);
        //OUTPORT32(&s_intr.pGPIORegs[5]->IRQSTATUS2, mask);
        OUTPORT32(&s_intr.pGPIORegs[5]->CLEARIRQENABLE1, mask);
        OUTPORT32(&s_intr.pGPIORegs[5]->CLEARWAKEUPENA, mask);  
        OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), FALSE);
		 
	 } break;

	 default: 
	 {
		// Mask Interrupt in Interrupt Controller Register(s)
		if (irq < 32) 
			{
			SETPORT32(&s_intr.pICLRegs->INTC_MIR0, 1 << irq);
			} 
		else if (irq < 64) 
			{
             if( irq == IRQ_GPTIMER4 )
			 {
              //TEST_GPIO_HIGH; 
			 }
			SETPORT32(&s_intr.pICLRegs->INTC_MIR1, 1 << (irq - 32));
			}
		else if (irq < 96) 
			{
			SETPORT32(&s_intr.pICLRegs->INTC_MIR2, 1 << (irq - 64));
			}

	 }break;
	}

    // Acknowledge interrupt 
    OUTREG32(&s_intr.pICLRegs->INTC_CONTROL, IC_CNTL_NEW_IRQ);
	//OALDrainWriteBuffer();

    // Check if this is profiler IRQ
    if (irq == g_oalPerfTimerIrq && g_oalProfilerEnabled == TRUE)
    {
	  // Interrupt re-enables inside this function
        OALProfileTimerHit(ra);
    }


    // Check if this is timer IRQ
    if (irq == g_oalTimerIrq)
        {
        if (g_oalILT.active) g_oalILT.interrupts--;

        // Call timer interrupt handler
        sysIntr = OALTimerIntrHandler();

        // re-enable interrupts
        OALIntrDoneIrqs(1, &irq);
        }
    else if (irq == g_oalPrcmIrq)
        {
        // call prcm interrupt handler
        sysIntr = OALPrcmIntrHandler();

#if 0
        if (sysIntr != SYSINTR_NOP)
            {
            // sysIntr is a the GPIO irq number for which IO PAD
            // event occured, get the sysIntr for the irq
            sysIntr = OALIntrTranslateIrq(sysIntr);
            }

        OALIntrDoneIrqs(1, &irq);
#else
        if(sysIntr == SYSINTR_NOP)
			OALIntrDoneIrqs(1, &irq);
#endif
        }
    else if (irq == g_oalSmartReflex1)
        {
        // call prcm interrupt handler
        sysIntr = OALSmartReflex1Intr();

        // re-enable interrupts
        if (sysIntr == SYSINTR_NOP) OALIntrDoneIrqs(1, &irq);
        }
    else if (irq == g_oalSmartReflex2)
        {
        // call prcm interrupt handler
        sysIntr = OALSmartReflex2Intr();

        // re-enable interrupts
        if (sysIntr == SYSINTR_NOP) OALIntrDoneIrqs(1, &irq);
        }
    else if (irq != OAL_INTR_IRQ_UNDEFINED)
        {
         // ##### Michael, 24-Nov-2009
         // Add support for 
         //sysIntr = NKCallIntChain((UCHAR)g_IRQ2VIIRQ[irq]/* irq */); 

         if( irqGpioMpu )
		 {
		   sysIntr = NKCallIntChain((UCHAR)irqGpioMpu); 
		   // Now Clear GPIO status register
 		   OUTPORT32(&s_intr.pGPIORegs[irqGpioMpu-IRQ_GPIO1_MPU]->IRQSTATUS1, mask);
           OUTPORT32(&s_intr.pGPIORegs[irqGpioMpu-IRQ_GPIO1_MPU]->IRQSTATUS2, mask);
		 }
		 else
           sysIntr = NKCallIntChain((UCHAR)irq); 

		 if(sysIntr != SYSINTR_CHAIN)
  	      {
           //TEST_GPIO_LOW;

		   if(sysIntr == SYSINTR_NOP || sysIntr == SYSINTR_UNDEFINED || !NKIsSysIntrValid(sysIntr))
			return SYSINTR_NOP;
		   else
			return sysIntr;
	      }

         // We don't assume IRQ sharing, use static mapping
         sysIntr = OALIntrTranslateIrq(irq);
		 //TEST_GPIO_LOW;

 	     if(sysIntr == SYSINTR_NOP || sysIntr == SYSINTR_UNDEFINED || !NKIsSysIntrValid(sysIntr))
		  return SYSINTR_NOP;
		 else
		  return sysIntr;
        }

	//TEST_GPIO_LOW;

    return sysIntr;
}

//------------------------------------------------------------------------------
