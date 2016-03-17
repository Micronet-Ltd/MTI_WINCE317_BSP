// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

#include <windows.h>
#include <bsp.h>
#include <gpio.h>

#include <..\lcd_cfg.h>

void	 *g_hGpio = (void *)-1;
BSP_ARGS *g_Args = 0;
card_ver g_cv = {0};

int lcd_gpio_en(int en)
{
	// Assume GPIO are already initialized by platform setup code
	if(en)
	{
		if(g_Args)
			return 1;

		g_Args = OALPAtoUA(IMAGE_SHARE_ARGS_PA);
		if(g_Args)
		{
			memcpy(&g_cv, &g_Args->display_board, sizeof(card_ver));
		}
		else
			return 0;
	}

	return 1;
}

void lcd_power_en(int en)
{
	OMAP_GPIO_REGS* pGpio = OALPAtoUA(OMAP_GPIO6_REGS_PA);

	if(en)
		SETREG32(&pGpio->DATAOUT, 1 << (BSP_LCD_POWER_GPIO % 32));
	else
		CLRREG32(&pGpio->DATAOUT, 1 << (BSP_LCD_POWER_GPIO % 32));
}

void lcd_buff_en(int en)
{
	OMAP_GPIO_REGS* pGpio = OALPAtoUA(OMAP_GPIO4_REGS_PA);

	if(en)
		CLRREG32(&pGpio->DATAOUT, 1 << (BSP_LCD_BUFF_EN % 32));
	else if(g_Args->display_board.config == 'B')
		SETREG32(&pGpio->DATAOUT, 1 << (BSP_LCD_BUFF_EN % 32));
}

#if 0
// Screen should be in rotated to lanscape mode, 640x480 for use with DVI
void LcdDviEnableControl(BOOL bEnable)
{
    // DVI enable bit in controlled by T2
    HANDLE hTwl;
    UINT8 regval;

    hTwl = OALTritonOpen();
    if (hTwl != NULL)
    {
	    OALTritonRead(hTwl, BSP_LCD_DVIENABLE_T2DIR, &regval);
        regval |= BSP_LCD_DVIENABLE_T2MASK;
	    OALTritonWrite(hTwl, BSP_LCD_DVIENABLE_T2DIR, regval);

	    OALTritonRead(hTwl, BSP_LCD_DVIENABLE_T2READ, &regval);
        if (bEnable)
            regval |= BSP_LCD_DVIENABLE_T2MASK;
        else
		    regval &= ~BSP_LCD_DVIENABLE_T2MASK;
	    OALTritonWrite(hTwl, BSP_LCD_DVIENABLE_T2WRITE, regval);
    }
    OALTritonClose(hTwl);
}
#endif

void LcdStall(DWORD dwMicroseconds)
{
	OALStall(dwMicroseconds);
}

void LcdSleep(DWORD dwMilliseconds)
{
	OALStall(1000 * dwMilliseconds);
}
