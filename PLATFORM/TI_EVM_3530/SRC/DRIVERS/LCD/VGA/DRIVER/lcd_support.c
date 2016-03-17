// Copyright (c) 2007 - 2013, Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky

#include <windows.h>
#include <bsp.h>
#include <initguid.h>
#include <gpio.h>

#include <..\lcd_cfg.h>

void *g_hGpio = (void *)-1;
card_ver g_cv = {0};

int lcd_gpio_en(int en)
{
	UINT32 in = OAL_ARGS_QUERY_LCD_CFG;

	if(en)
	{
		if((void *)-1 != g_hGpio)
			return 1;

		g_hGpio = GPIOOpen();

		if(g_hGpio == (void *)-1)
			return 0;

		if(g_cv.config == 0)
		{
			if(!KernelIoControl(IOCTL_HAL_QUERY_CARD_CFG, &in, sizeof(in), &g_cv, sizeof(g_cv), 0))
			{
				g_cv.card_addr	= -1;
				g_cv.ver		= -1;
				g_cv.rev		= -1;
				g_cv.config		= -1;
			}
		}

		// Assume GPIO are already initialized by platform setup code
		GPIOSetMode(g_hGpio, BSP_LCD_POWER_GPIO, GPIO_DIR_OUTPUT);
		GPIOSetMode(g_hGpio, BSP_LCD_BUFF_EN,	 GPIO_DIR_OUTPUT);

//		GPIOSetMode(g_hGpio, BSP_LCD_V3P3V_GPIO, GPIO_DIR_OUTPUT);
	}
	else
		CloseHandle(g_hGpio);

	return 1;
}

void lcd_power_en(int en)
{
	if(en)
		GPIOSetBit(g_hGpio, BSP_LCD_POWER_GPIO);
	else
		GPIOClrBit(g_hGpio, BSP_LCD_POWER_GPIO);
}

void lcd_buff_en(int en)
{
	if(en)
		GPIOClrBit(g_hGpio, BSP_LCD_BUFF_EN);
	else if(g_cv.config == 'B')
		GPIOSetBit(g_hGpio, BSP_LCD_BUFF_EN);
}

#if 0
// Screen should be in rotated to lanscape mode, 640x480 for use with DVI
void LcdDviEnableControl(BOOL bEnable)
{
    // g_gpioDviEnable == 0 -> disable, 1 -> enable
	if (bEnable == TRUE)
	{
        DWORD dwMode[2] = {g_gpioDviEnable, 0};
        // set pin to output
		DeviceIoControl(g_hGpio, IOCTL_GPIO_SETMODE, &dwMode, sizeof(dwMode), 0, 0, NULL, NULL);

		GPIOSetBit(g_hGpio, g_gpioDviEnable);
	}
	else
		GPIOClrBit(g_hGpio, g_gpioDviEnable);
}
#endif

void LcdStall(DWORD dwMicroseconds)
{
	StallExecution(dwMicroseconds);
}

void LcdSleep(DWORD dwMilliseconds)
{
	Sleep(dwMilliseconds);
}
