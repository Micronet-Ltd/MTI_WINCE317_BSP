/*
================================================================================

 Copyright (c) 2009, Micronet Corporation. All rights reserved.

 History of Changes:
 Anna R.       March 2010      Created          
================================================================================
*/
//
//  File:  hdq.c
//

//#include <windows.h>
#include <bsp.h>
#include <oal_prcm.h>
#include <omap35xx.h>

#ifndef SHIP_BUILD
	#define HDQ_DEBUG         1
#else
	#define HDQ_DEBUG         0
#endif

#define BREAK_TIMEOUT     5              // in msec
#define RX_TIMEOUT        5              // in msec
#define TX_TIMEOUT        5              // in msec

//-----------------------------------------------------------------------------
// hdq context
//
typedef struct {
    OMAP_HDQ_1WIRE_REGS*    pHDQRegs;    
    DWORD                   breakTimeout;
	DWORD                   rxTimeout;
	DWORD                   txTimeout;
} HDQContext_t;



HDQContext_t             sDevice;
//------------------------------------------------------------------------------
//
//  Function:  HDQ_Init
//
//  Called by device manager to initialize device.
//
BOOL OALHDQInit(OMAP_HDQ_1WIRE_REGS* pHDQRegs)

{

	// Enable interface and functional clock of HDQ module
	PrcmDeviceEnableClocks(OMAP_DEVICE_HDQ, TRUE);

	//OALPAtoUA(OMAP_PRCM_CORE_CM_REGS_PA);

	sDevice.pHDQRegs     = pHDQRegs;
	sDevice.breakTimeout = BREAK_TIMEOUT;
	sDevice.rxTimeout    = RX_TIMEOUT;
	sDevice.txTimeout    = TX_TIMEOUT;


	// Enable the clock.
    SETREG32(&sDevice.pHDQRegs->CTRL_STATUS, HDQ_CTRL_CLOCK_ENABLE);
    
    // Set to HDQ mode as default
    CLRREG32(&sDevice.pHDQRegs->CTRL_STATUS, HDQ_MODE_1WIRE);
    
    // Clear the interrupt
   // INREG32(&sDevice.pHDQRegs->INT_STATUS);



	  // release clocks
    PrcmDeviceEnableClocks(OMAP_DEVICE_HDQ, FALSE);



    return TRUE;
}




//------------------------------------------------------------------------------
//
//  Function:  TransmitBreak
//
//  sends a break command to the slave device.
//
void TransmitBreak()
{
    DWORD status;
	DWORD time;
    
   OALMSG(HDQ_DEBUG, (L"+TransmitBreak() time = %x \r\n",OALGetTickCount()));

    // set break command to slave
    OUTREG32(&sDevice.pHDQRegs->CTRL_STATUS, 
        HDQ_CTRL_INITIALIZE | HDQ_CTRL_GO | HDQ_CTRL_CLOCK_ENABLE | 
        HDQ_CTRL_INTERRUPT_MASK);

	// OALMSG(1, (L"+TransmitBreak() CTRL_STATUS = %x \r\n",INREG32(&sDevice.pHDQRegs->CTRL_STATUS)));


	// wait till HDQ_CTRL_GO is cleared and breakTimeout is over to make sure the interrupt status
	// register has the right value

	time = OALGetTickCount()+ sDevice.breakTimeout;
	while ((INREG32(&sDevice.pHDQRegs->CTRL_STATUS)& HDQ_CTRL_GO) == 1);
	while (OALGetTickCount()<time);

	  // Clear interrupt.
    status = INREG32(&sDevice.pHDQRegs->INT_STATUS);

   
    // disable clock and reset to 0
  //  OUTREG32(&sDevice.pHDQRegs->CTRL_STATUS, HDQ_CTRL_INTERRUPT_MASK);

	OALMSG(HDQ_DEBUG, (L"-TransmitBreak() time = %x \r\n",OALGetTickCount()));

}



//------------------------------------------------------------------------------
//
//  Function:  TransmitCommand
//
//  sends a command to the slave device.
//
BOOL 
TransmitCommand(
    UINT8 cmd,
    DWORD *pStatus
    )
{
    DWORD mask;
	DWORD status;
    BOOL rc = FALSE;
	DWORD time;

    OMAP_HDQ_1WIRE_REGS *pHDQRegs = sDevice.pHDQRegs;


	OALMSG(HDQ_DEBUG, (L"+TransmitCommand() time = %x, cmd = %x \r\n",OALGetTickCount(),cmd));


    // setup mask
    mask = HDQ_CTRL_GO | HDQ_CTRL_WRITE | HDQ_CTRL_CLOCK_ENABLE | HDQ_CTRL_INTERRUPT_MASK ;
    
    // write the command
    OUTREG32(&pHDQRegs->TX_DATA, cmd);

    // send signal to write
    OUTREG32(&pHDQRegs->CTRL_STATUS, mask);

	// wait till HDQ_CTRL_GO is cleared + timeout to be sure the interrupt status register  has the proper value

	time = OALGetTickCount()+ sDevice.txTimeout;;
	while ((INREG32(&sDevice.pHDQRegs->CTRL_STATUS)& HDQ_CTRL_GO) == 1);
	while (OALGetTickCount()<time);

	// Clear interrupt.
    *pStatus = INREG32(&sDevice.pHDQRegs->INT_STATUS);

    OALMSG(HDQ_DEBUG, (L"TransmitCommand: *pStatus = 0x%02X\r\n", *pStatus));
    
    // Verify interrupt source
    if ((*pStatus & HDQ_INT_TX_COMPLETE) == 0) 
    {

		OALMSG(HDQ_DEBUG, (L"ERROR: TransmitCommand: TX complete expected (0x%x)\r\n", *pStatus));
        TransmitBreak();
        goto cleanUp;
     }
    rc = TRUE;
    
cleanUp:

	OALMSG(HDQ_DEBUG, (L"-TransmitCommand(cmd=0x%02X, rc = 0x%x)\r\n", cmd,rc));
    return rc;

}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Function:  HDQ_Read
//
//  This function reads from the specified address data in HDQ mode.
//
BOOL  OALHDQRead( UINT8 address, void* pBuffer, DWORD size)
{

	OMAP_HDQ_1WIRE_REGS *pHDQRegs;
	DWORD status,i;
	DWORD mask,time;
	UINT8 *pData = (UINT8*)pBuffer;
	BOOL rc = FALSE;

	pHDQRegs = sDevice.pHDQRegs;
	OALMSG(HDQ_DEBUG, (L"+OALHDQRead() pBuffer = %x, size = %x \r\n",pBuffer,size));

		// Enable interface and functional clock of HDQ module
	PrcmDeviceEnableClocks(OMAP_DEVICE_HDQ, TRUE);
	//OMAP_PRCM_CORE_CM_REGS_PA


	// Clear the interrupt.
    status = INREG32(&sDevice.pHDQRegs->INT_STATUS);

	TransmitBreak();
	
    if (TransmitCommand(address, &status) == FALSE)
    {
		OALMSG(HDQ_DEBUG, (L"ERROR:TransmitCommand failed \r\n"));
		goto cleanUp;

    }


	// setup mask for read
    mask = HDQ_CTRL_GO | HDQ_CTRL_READ |HDQ_CTRL_CLOCK_ENABLE |HDQ_CTRL_INTERRUPT_MASK ; 

    for (i = 0; i < size; ++i)
    {

		if ((status & HDQ_INT_RX_COMPLETE) == 0)
        {

						// initiate receive
			OUTREG32(&pHDQRegs->CTRL_STATUS, mask);


			// wait till HDQ_CTRL_GO is cleared + timeout to be sure the interrupt status register  has the proper value
			time = OALGetTickCount()+ sDevice.rxTimeout;;
			while ((INREG32(&sDevice.pHDQRegs->CTRL_STATUS)& HDQ_CTRL_GO) == 1);
			while (OALGetTickCount()<time);

			status = INREG32(&sDevice.pHDQRegs->INT_STATUS);

            
        }

		 // Verify interrupt source
        if ((status & HDQ_INT_RX_COMPLETE) == 0) 
        {

			OALMSG(HDQ_DEBUG, (L"ERROR: HDQ_Read: RX complete expected (0x%02x)\r\n", status));
            TransmitBreak();
            goto cleanUp;
         }

		// read data
        pData[i] = (UINT8)INREG32(&pHDQRegs->RX_DATA);
        OALMSG(HDQ_DEBUG, (L"INFO: HDQ_Read: " L"Read data[%d] = 0x%02X, status = %x\r\n", i, pData[i] , status));
        // reset status to force read request
        status = 0;

	}

	  // disable clock
    OUTREG32(&pHDQRegs->CTRL_STATUS, HDQ_CTRL_INTERRUPT_MASK);

	    // Done
    rc = TRUE;

cleanUp:

	PrcmDeviceEnableClocks(OMAP_DEVICE_HDQ, FALSE);
	OALMSG(HDQ_DEBUG, (L"-OALHDQRead(rc = 0x%x)\r\n",rc));

	return rc;
}



#if 0

DWORD OALHDQReset()
{
	OMAP_HDQ_1WIRE_REGS *pHDQRegs;


	pHDQRegs = sDevice.pHDQRegs;

	PrcmDeviceEnableClocks(OMAP_DEVICE_HDQ, TRUE);

	SETREG32(&pHDQRegs->SYS_CONFIG, HDQ_SYSCONFIG_SOFT_RESET);
	while ((INREG32(&sDevice.pHDQRegs->SYS_STATUS)& HDQ_SYSSTATUS_RESET_DONE) == 0)
	{};

	PrcmDeviceEnableClocks(OMAP_DEVICE_HDQ, FALSE);
	return 1;
}
#endif