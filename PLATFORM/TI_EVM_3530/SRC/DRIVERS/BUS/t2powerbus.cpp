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

#include <windows.h>
#include <devload.h>
#include <cebuscfg.h>
#include <oal.h>
#include <oalex.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>

#include <initguid.h>
#include <powerbus.h>
#include <t2powerbus.h>
#include <initguid.h>
#include <twl.h>
#include <tps659xxusb.h>


// T2 power bus is a signleton
static T2PowerBus_t *_pT2PowerBus = NULL;


//-----------------------------------------------------------------------------
omapPowerBus_t* 
omapPowerBus_t::
CreatePowerBus(
    )
{
    // only create new instance of t2 power bus 
    // if there already isn't one
    //
    if (_pT2PowerBus == NULL)
        {
        _pT2PowerBus = new T2PowerBus_t();
        if (_pT2PowerBus->InitializePowerBus() == FALSE)
            {
            delete _pT2PowerBus;
            _pT2PowerBus = NULL;
            }
        }

    return _pT2PowerBus;
}

//-----------------------------------------------------------------------------
void
omapPowerBus_t::
DestroyPowerBus(
    omapPowerBus_t* pPowerBus
    )
{
    if (_pT2PowerBus != NULL)
        {
        _pT2PowerBus->DeinitializePowerBus();
        delete _pT2PowerBus;
        _pT2PowerBus = NULL;
        }
}

//-----------------------------------------------------------------------------
T2PowerBus_t::
T2PowerBus_t(
    )
{
    m_hTwl = NULL;
}
    
//-----------------------------------------------------------------------------
BOOL 
T2PowerBus_t::
InitializePowerBus(
    )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
void 
T2PowerBus_t::
DeinitializePowerBus(
    )
{
    if (m_hTwl != NULL)
        {
        TWLClose(m_hTwl);
        m_hTwl = NULL;
        }
}

//-----------------------------------------------------------------------------
BOOL 
T2PowerBus_t::
SendSingularPBMessage(
    UCHAR power_res_id,
    UCHAR res_state
    )
{
    UINT16 pb_message;
    UCHAR regval;
    BOOL rc = FALSE;

    // Enable I2C access to the Power Bus
    regval = 0x02;  
    if(!WriteTwlReg(TWL_PB_CFG, regval))
        goto cleanUp;

    // Form the message for VDAC
    pb_message = TwlTargetMessage(TWL_PROCESSOR_GRP1, power_res_id, res_state);

    // Extract the Message MSB
    regval = pb_message >> 8;
    if(!WriteTwlReg(TWL_PB_WORD_MSB, regval))
        goto cleanUp;

    // Extract the Message LSB
    regval = pb_message & 0x00FF;
    if(!WriteTwlReg(TWL_PB_WORD_LSB, regval))
        goto cleanUp;

    rc = TRUE;
cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
BOOL 
T2PowerBus_t::
SetT2USBXvcrActive(
    BOOL fActive
    )
{
    if (fActive)
        {
        // give time for power-on
        StallExecution(40);
        WakeT2USBPhyDPLL();
        }
    else
        {
        // if it has been asleep, wakeup the PHY before put to sleep again.
        // this is to avoid the PHY is actually sleeping and access the FUNC_CTRL
        // would return error
        // From USB analyzer, you may see SUSPEND/ALIVE and then SUSPEND again.  The problem
        // is due to enable phy and then disable again
        WakeT2USBPhyDPLL();

        // Now actual suspend the PHY
        // Jeffrey: testing
        WriteTwlReg(TWL_FUNC_CTRL_CLR, FUNC_CTRL_SUSPENDM);
        StallExecution(20);           
        }   
    
    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL 
T2PowerBus_t::
UpdateUSBVBusSleepOffState(
    UCHAR state
    )
{
    WriteTwlReg(TWL_CLKEN_REMAP, state);
    WriteTwlReg(TWL_VINTDIG_REMAP, state);
    WriteTwlReg(TWL_VUSB1V8_REMAP, state);
    WriteTwlReg(TWL_VUSB1V5_REMAP, state);
    WriteTwlReg(TWL_VUSB3V1_REMAP, state);
    WriteTwlReg(TWL_HFCLKOUT_REMAP, state);

    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL 
T2PowerBus_t::
WakeT2USBPhyDPLL(
    )
{
    UCHAR sts;
    UCHAR ctrl;
    BOOL fGetLock = FALSE;

    // Lock PHY: start
    if(ReadTwlReg(TWL_PHY_CLK_CTRL_STS, sts))
        {
        int iCount = 500;
        if (((sts & PHY_CLK_CTRL_STS_PHY_DPLL_LOCK) == 0) && 
             ReadTwlReg(TWL_PHY_CLK_CTRL, ctrl))
            {
            BOOL fSuccess;
            do 
                {
                ctrl |= PHY_CLK_CTRL_REQ_PHY_DPLL_CLK;
                WriteTwlReg(TWL_PHY_CLK_CTRL, ctrl);
                sts = 0x00;
                if (ReadTwlReg(TWL_PHY_CLK_CTRL_STS, sts) && 
                    (sts & PHY_CLK_CTRL_STS_PHY_DPLL_LOCK))
                {
                    fGetLock = TRUE;
                    break;
                }
                iCount--;
                StallExecution(50);
                fSuccess = ReadTwlReg(TWL_PHY_CLK_CTRL, ctrl);
                } while (fSuccess && (iCount > 0));
            }
        }
    
    // Lock PHY: end
    StallExecution(10);
    if (ReadTwlReg(TWL_PHY_CLK_CTRL, ctrl))
        {
        // clear PHY_DPLL_LOCK bit and let PHY state depend 
        // on the SUSPENDM bit in the FUNC_CTRL register
        ctrl &= ~PHY_CLK_CTRL_STS_PHY_DPLL_LOCK;
        WriteTwlReg(TWL_PHY_CLK_CTRL, ctrl);
        fGetLock = TRUE;
        }

    return fGetLock;
}

//-----------------------------------------------------------------------------
BOOL
T2PowerBus_t::
UsbFnAttach(
    BOOL state
    )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL 
T2PowerBus_t::
SelectPowerResVolt(
    DWORD res_vsel_reg_offset,
    UCHAR reg_data
    )
{
    return WriteTwlReg(res_vsel_reg_offset, reg_data) != FALSE;
}

//-----------------------------------------------------------------------------
BOOL 
T2PowerBus_t::
SetVmmc1PowerOnOff(
    UCHAR power_res_state
    )
{
    return (SendSingularPBMessage(TWL_VMMC1_RES_ID, power_res_state));
}

//-----------------------------------------------------------------------------
BOOL 
T2PowerBus_t::
SetVmmc2PowerOnOff(
    UCHAR power_res_state
    )
{
    return (SendSingularPBMessage(TWL_VMMC2_RES_ID, power_res_state));
}

//------------------------------------------------------------------------------
BOOL
T2PowerBus_t::
WriteTwlReg(
    DWORD address,
    UCHAR value
    )
{
    BOOL rc = FALSE;

    // If TWL isn't open, try to open it...
    if (m_hTwl == NULL)
        {
        m_hTwl = TWLOpen();
        if (m_hTwl == NULL) goto cleanUp;
        }

    // Call driver
    rc = TWLWriteRegs(m_hTwl, address, &value, sizeof(value));

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
BOOL
T2PowerBus_t::
ReadTwlReg(
    DWORD address,
    UCHAR& value
    )
{
    BOOL rc = FALSE;

    // If TWL isn't open, try to open it...
    if (m_hTwl == NULL)
        {
        m_hTwl = TWLOpen();
        if (m_hTwl == NULL) goto cleanUp;
        }

    // Call driver
    rc = TWLReadRegs(m_hTwl, address, &value, sizeof(value));

cleanUp:
    return rc;
}
    
//-----------------------------------------------------------------------------
DWORD 
T2PowerBus_t::
PreDevicePowerStateChange(
    DWORD devId, 
    CEDEVICE_POWER_STATE oldPowerState,
    CEDEVICE_POWER_STATE newPowerState
    )
{
    UCHAR state;
    DWORD rc = FALSE;

    if (newPowerState == oldPowerState) return rc;

    switch (devId)
        {
        case OMAP_DEVICE_HSOTGUSB:
            if (newPowerState <= D3)
                {
                state = TWL_RES_ACTIVE|(TWL_RES_ACTIVE << 4);
                UpdateUSBVBusSleepOffState(state);
                rc = TRUE;
                }
            
            if (newPowerState <= D2)
                {
                // Enable USB LDO's 
                SetT2USBXvcrActive(TRUE);
                // put LDO's in Active mode
                SendSingularPBMessage(TWL_VUSB_3V1_RES_ID, TWL_RES_ACTIVE);
                SendSingularPBMessage(TWL_VUSB_1V5_RES_ID, TWL_RES_ACTIVE);
                SendSingularPBMessage(TWL_VUSB_1V8_RES_ID, TWL_RES_ACTIVE);  
                }
            break;

        case OMAP_DEVICE_MMC1:
            if (newPowerState <= D3)
                {
                // assign MMC1 power resource to GROUP_NONE in order to turn off LDO
                //  (setting to OFF state alone doesn't turn off LDO)
                WriteTwlReg(TWL_VMMC1_DEV_GRP, TWL_DEV_GROUP_P1);
                rc = SendSingularPBMessage(TWL_VMMC1_RES_ID, TWL_RES_ACTIVE);
                }
            break;

        case OMAP_DEVICE_DSS:
            if (newPowerState <= D3)
                {
                SendSingularPBMessage(TWL_VAUX3_RES_ID, TWL_RES_ACTIVE);
                SendSingularPBMessage(TWL_VPLL2_RES_ID, TWL_RES_ACTIVE);
                rc = TRUE;
                }
            break;

        case OMAP_DEVICE_CAMERA:
            if (newPowerState <= D3)
                {
                rc = SendSingularPBMessage(TWL_VAUX2_RES_ID, TWL_RES_ACTIVE);
                }
            break;
#if 0            
        case OMAP_DEVICE_TVOUT:
            if (newPowerState <= D3)
                {
                rc = SendSingularPBMessage(TWL_VDAC_RES_ID, TWL_RES_ACTIVE);
                }
            break;
#endif     

        case OMAP_DEVICE_MMC2:
            if (newPowerState <= D3)
                {
                WriteTwlReg(TWL_VMMC2_DEV_GRP, TWL_DEV_GROUP_P1);
                rc = SendSingularPBMessage(TWL_VMMC2_RES_ID, TWL_RES_ACTIVE);
                }
            break;

        default:
            // signal we don't want notification for this device
            rc = -1;
        }
    
    return rc;
}

//-----------------------------------------------------------------------------
DWORD 
T2PowerBus_t::
PostDevicePowerStateChange(
    DWORD devId, 
    CEDEVICE_POWER_STATE oldPowerState,
    CEDEVICE_POWER_STATE newPowerState
    )
{
    UCHAR state;
    BOOL rc = FALSE;

    if (newPowerState == oldPowerState) return rc;
    
	switch (devId)
	{
        case OMAP_DEVICE_HSOTGUSB:
		{
            if(newPowerState >= D3)
			{
				SetT2USBXvcrActive(FALSE);
				// put LDO's in OFF mode
				SendSingularPBMessage(TWL_VUSB_3V1_RES_ID, TWL_RES_OFF);
				SendSingularPBMessage(TWL_VUSB_1V5_RES_ID, TWL_RES_OFF);
				SendSingularPBMessage(TWL_VUSB_1V8_RES_ID, TWL_RES_OFF);

				if(newPowerState == D4)
					state = TWL_RES_OFF|(TWL_RES_OFF << 4);
				else
					state = TWL_RES_ACTIVE|(TWL_RES_ACTIVE << 4);

				UpdateUSBVBusSleepOffState(state);
            }

            //
            //if (newPowerState == D4)
            //    {
            //    state = TWL_RES_ACTIVE|(TWL_RES_ACTIVE << 4);
            //    UpdateUSBVBusSleepOffState(state);
            //    }
   
			rc = 1;

			break;
		}
        case OMAP_DEVICE_MMC1:
            // assign MMC1 power resource to GROUP_NONE in order to turn off LDO
            //  (setting to OFF state alone doesn't turn off LDO)
            if (newPowerState == D4)
                {         
                WriteTwlReg(TWL_VMMC1_DEV_GRP, TWL_DEV_GROUP_NONE);
                rc = SendSingularPBMessage(TWL_VMMC1_RES_ID, TWL_RES_OFF);
                }
            break;

        case OMAP_DEVICE_DSS:
            if (newPowerState == D4)
                { 
                SendSingularPBMessage(TWL_VAUX3_RES_ID, TWL_RES_ACTIVE);
                SendSingularPBMessage(TWL_VPLL2_RES_ID, TWL_RES_ACTIVE);
                rc = TRUE;
                }
            break;

        case OMAP_DEVICE_CAMERA:
            if (newPowerState == D4)
                { 
                rc = SendSingularPBMessage(TWL_VAUX2_RES_ID, TWL_RES_ACTIVE);
                }
            break;

#if 0            
        case OMAP_DEVICE_TVOUT:
            if (newPowerState == D4)
                { 
                rc = SendSingularPBMessage(TWL_VDAC_RES_ID, TWL_RES_ACTIVE);
                }
            break;
#endif            

        case OMAP_DEVICE_MMC2:
            if (newPowerState == D4)
                { 
                WriteTwlReg(TWL_VMMC2_DEV_GRP, TWL_DEV_GROUP_NONE);
                rc = SendSingularPBMessage(TWL_VMMC2_RES_ID, TWL_RES_OFF);
                }
            break;

        default:
            // signal we don't want notification for this device
            rc = -1;
	}
    
    return rc;
}

//-----------------------------------------------------------------------------
