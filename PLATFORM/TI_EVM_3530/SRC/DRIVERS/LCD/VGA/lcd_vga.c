// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky

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

#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <bsp.h>

#include <lcd.h>
#include "lcd_cfg.h"

////////////////////////////////////////////////////////////////////////
//  Defines
//

#define BSP_DPLL4_CLK 864000000 

UINT32 lcd_u_d;
UINT32 dispc_pixelformat;

UINT32 lcd_width;
UINT32 lcd_height;
UINT32 lcd_hsw;				// Horizontal Sync Width
UINT32 lcd_hfp;				// Horizontal Front Porch
UINT32 lcd_hbp;				// Horizontal Back Porch
UINT32 lcd_vsw;				// Vertical Sync Width
UINT32 lcd_vfp;				// Vertical Front Porch
UINT32 lcd_vbp;				// Vertical Back Porch
UINT32 lcd_logclkdiv;
UINT32 lcd_pixclkdiv;
UINT32 dss_clksel;
UINT32 lcd_loadmode;
UINT32 lcd_polfreq;
UINT32 lcd_default_color;
UINT32 lcd_trans_color;
UINT32 tv_default_color;
UINT32 tv_trans_color;

static BOOL bDVIEnabled = 0;
static OMAP_DISPC_REGS *g_pDispRegs;
extern card_ver g_cv;

//------------------------------------------------------------------------------
//
static void LcdPdd_LCD_Init()
{
	lcd_gpio_en(1);

	if(g_cv.config == 'C')
	{
		RETAILMSG(0, (L"LCD_VGA:%S, LCD 'C' configuration\r\n",__FUNCTION__));

		lcd_u_d = 1;
		dispc_pixelformat = DISPC_PIXELFORMAT_RGB16;

		lcd_width		= 480;
		lcd_height		= 272;
		lcd_hsw			= 41; //
		lcd_hfp			= 2; //
		lcd_hbp			= 2; //
		lcd_vsw			= 10; //
		lcd_vfp			= 2; //
		lcd_vbp			= 2; //
		lcd_logclkdiv	= 1;
		dss_clksel		= (16 << 0); //DSS1 = DPLL4/16 = 54 MHz;
		lcd_pixclkdiv	= 4; // pixel clock 54/7 = 7.71 MHz 6; // pixel clock 54/6 = 9; // pixel clock 54/4 = 13.5 MHz
		lcd_loadmode	= 0;
//		lcd_polfreq		= (DISPC_POL_FREQ_RF | DISPC_POL_FREQ_IVS | DISPC_POL_FREQ_IHS | DISPC_POL_FREQ_IPC | DISPC_POL_FREQ_ONOFF);
		lcd_polfreq		= (DISPC_POL_FREQ_RF | DISPC_POL_FREQ_IVS | DISPC_POL_FREQ_IHS | DISPC_POL_FREQ_ONOFF);
		lcd_default_color = 0;
		lcd_trans_color	  = 0;
		tv_default_color  = 0;
		tv_trans_color	= 0;
	}
	else
	{
		// 'B' - default
		lcd_u_d = 0;
		dispc_pixelformat = DISPC_PIXELFORMAT_RGB16;

		lcd_width		= 800;
		lcd_height		= 480;
		// 800+(47+1)+(39+1)+(39+1) = 928
		lcd_hsw			= 47;
		lcd_hfp			= 39;
		lcd_hbp			= 39;
		// 480+(2+1)+13+29 = 525
		lcd_vsw			= 2;
		lcd_vfp			= 13;
		lcd_vbp			= 29;
		lcd_logclkdiv	= 1;
		dss_clksel		= (12 << 0); //DSS1 = DPLL4/12 = 72 MHz;
		//dss_clksel		= (11 << 0); //DSS1 = DPLL4/11 = 78.(54) MHz;
		lcd_pixclkdiv	= 2; // pixel clock 72/2 = 36
		//lcd_pixclkdiv	= 2; // pixel clock 78.(54)/2 = 39.(27)
		lcd_loadmode	= 0;
		lcd_polfreq		= (DISPC_POL_FREQ_RF | DISPC_POL_FREQ_IVS | DISPC_POL_FREQ_IHS | DISPC_POL_FREQ_ONOFF);
		lcd_default_color = 0;
		lcd_trans_color	  = 0;
		tv_default_color  = 0;
		tv_trans_color	= 0;
	}
}
BOOL LcdPdd_LCD_Initialize(OMAP_DSS_REGS *pDSSRegs, OMAP_DISPC_REGS *pDispRegs, OMAP_RFBI_REGS *pRfbiRegs,
						   OMAP_VENC_REGS *pVencRegs, OMAP_PRCM_DSS_CM_REGS *pPrcmDssCM)
{
	// Store pointer to display controller for future use
	g_pDispRegs = pDispRegs;

	// Initialize LCD timing
	LcdPdd_LCD_Init();

	RETAILMSG(1, (L"LcdPdd_LCD_Initialize (CE-300): LCD is %s(%c)\r\n", (g_cv.config == 'A')?L"WINTEK 4.3 controlled via 3-wire":
													 (g_cv.config == 'B')?L"WINSTAR 7":
													 (g_cv.config == 'C')?L"Dastek 4.3":L"unknown",
													  g_cv.config));

	// setup the DSS1 clock divider - disable DSS1 clock, change divider, enable DSS clock
    OUTREG32(&pPrcmDssCM->CM_FCLKEN_DSS, INREG32(&pPrcmDssCM->CM_FCLKEN_DSS) & ~CM_CLKEN_DSS1);
    OUTREG32(&pPrcmDssCM->CM_CLKSEL_DSS, BSP_DSS_CLKSEL_TV | dss_clksel);
    OUTREG32(&pPrcmDssCM->CM_FCLKEN_DSS, INREG32(&pPrcmDssCM->CM_FCLKEN_DSS) | CM_CLKEN_DSS1);

	//  LCD control
 	OUTREG32(&pDispRegs->DISPC_CONTROL,
				DISPC_CONTROL_GPOUT1 |
				DISPC_CONTROL_GPOUT0 |
				DISPC_CONTROL_TFTDATALINES_24 |
				DISPC_CONTROL_STNTFT
				);
				
    //  LCD config
    OUTREG32(&pDispRegs->DISPC_CONFIG,
                DISPC_CONFIG_FUNCGATED | //DISPC_CONFIG_ACBIASGATED |
                DISPC_CONFIG_LOADMODE(lcd_loadmode) |
                DISPC_CONFIG_PALETTEGAMMATABLE
                );

    // Default Color
    OUTREG32(&pDispRegs->DISPC_DEFAULT_COLOR0, lcd_default_color);

    // Default Transparency Color
    OUTREG32(&pDispRegs->DISPC_TRANS_COLOR0, lcd_trans_color);

    // Timing logic for HSYNC signal
    OUTREG32(&pDispRegs->DISPC_TIMING_H, DISPC_TIMING_H_HSW(lcd_hsw) | DISPC_TIMING_H_HFP(lcd_hfp) | DISPC_TIMING_H_HBP(lcd_hbp));

    // Timing logic for VSYNC signal
    OUTREG32(&pDispRegs->DISPC_TIMING_V, DISPC_TIMING_V_VSW(lcd_vsw) | DISPC_TIMING_V_VFP(lcd_vfp) | DISPC_TIMING_V_VBP(lcd_vbp));

    // Signal configuration
    OUTREG32(&pDispRegs->DISPC_POL_FREQ, lcd_polfreq);

	RETAILMSG(0, (L"LcdPdd_LCD_Initialize PCLK divisor (%X <- %X)\r\n", INREG32(&pDispRegs->DISPC_DIVISOR), DISPC_DIVISOR_PCD(lcd_pixclkdiv) | DISPC_DIVISOR_LCD(lcd_logclkdiv)));
    // Configures the divisor
    OUTREG32(&pDispRegs->DISPC_DIVISOR, DISPC_DIVISOR_PCD(lcd_pixclkdiv) | DISPC_DIVISOR_LCD(lcd_logclkdiv));

    // Configures the panel size
    OUTREG32(&pDispRegs->DISPC_SIZE_LCD, DISPC_SIZE_LCD_LPP(lcd_height) | DISPC_SIZE_LCD_PPL(lcd_width));

	LcdPdd_SetPowerLevel(D0);

	return 1;
}
//------------------------------------------------------------------------------
BOOL LcdPdd_SetPowerLevel(DWORD dwPowerLevel)
{
	UINT32 read = 0;
	UINT16 control = 0;

	// Do nothing if LCD has not yet been initialized
	if(!g_pDispRegs)
		return 0;

	RETAILMSG(1, (L"LcdPdd_SetPowerLevel(D%d)\r\n", dwPowerLevel));
		
	// Power display up/down
	switch(dwPowerLevel)
	{
		case D0:
		case D1:
		case D2:
        {
			OUTREG32(&g_pDispRegs->DISPC_CONTROL,
						DISPC_CONTROL_GPOUT1 | DISPC_CONTROL_GPOUT0 | DISPC_CONTROL_TFTDATALINES_24 | DISPC_CONTROL_STNTFT);
            // Apply display configuration
            SETREG32(&g_pDispRegs->DISPC_CONTROL, DISPC_CONTROL_GOLCD);

			// LCD power enable sequence
			lcd_power_en(1);

			// Start scanning
			SETREG32( &g_pDispRegs->DISPC_CONTROL, DISPC_CONTROL_LCDENABLE);        
			
			// Delay for minimum 20uS
			LcdStall(50);
			
			// enable display buffer
			lcd_buff_en(1);
			
			LcdStall(10);

			break;
		}
//		case D2:
		case D3:
		case D4:
		{
            // disable DVI
//            LcdDviEnableControl(0);
		    // disable lcd buffer
	        lcd_buff_en(0);

			// LCD power disable sequence
		    lcd_power_en(0);
		    // Delay > 5 frames
		    LcdSleep(20*5);
		    // 3 - Stop all
            CLRREG32(&g_pDispRegs->DISPC_CONTROL, DISPC_CONTROL_LCDENABLE);
				
			break;
		}
	}
		
    return 1;
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
BOOL LcdPdd_TV_Initialize(OMAP_DSS_REGS *pDSSRegs, OMAP_DISPC_REGS *pDispRegs, OMAP_RFBI_REGS *pRfbiRegs, OMAP_VENC_REGS *pVencRegs)
{
	//  Enable TV out DSS control settings
	
#if BSP_TVOUT_COMPOSITE	
    SETREG32( &pDSSRegs->DSS_CONTROL,
                DSS_CONTROL_DAC_POWERDN_BGZ |
                DSS_CONTROL_DAC_DEMEN |
                DSS_CONTROL_VENC_CLOCK_4X_ENABLE |
                DSS_CONTROL_VENC_CLOCK_MODE_0
                );
	CLRREG32( &pDSSRegs->DSS_CONTROL,
				DSS_CONTROL_DAC_VENC_OUT_SEL 			//Enable Composite TVout
			);
#else
	SETREG32( &pDSSRegs->DSS_CONTROL,
                DSS_CONTROL_DAC_VENC_OUT_SEL |          // Enables S-video
                DSS_CONTROL_DAC_POWERDN_BGZ |
                DSS_CONTROL_DAC_DEMEN |
                DSS_CONTROL_VENC_CLOCK_4X_ENABLE |
                DSS_CONTROL_VENC_CLOCK_MODE_0
                );
#endif				


    // Default Color
    OUTREG32(&pDispRegs->DISPC_DEFAULT_COLOR1, tv_default_color);

    // Default Transparency Color
    OUTREG32(&pDispRegs->DISPC_TRANS_COLOR1, tv_trans_color);


    // Configures the TV size for NTSC interlaced (half height)
    // the LPP is programmed to (NTSC_HEIGHT+2)/2 for interlaced mode
    OUTREG32(&pDispRegs->DISPC_SIZE_DIG, DISPC_SIZE_DIG_LPP((NTSC_HEIGHT+2)/2) | DISPC_SIZE_DIG_PPL(NTSC_WIDTH));

    return 1;
}


//------------------------------------------------------------------------------
BOOL LcdPdd_GetMemory(DWORD *pVideoMemLen, DWORD *pVideoMemAddr)
{
    //  Return video memory parameters
    if(pVideoMemLen)
        *pVideoMemLen = IMAGE_DISPLAY_BUF_SIZE;

    if(pVideoMemAddr)
        *pVideoMemAddr = IMAGE_DISPLAY_BUF_PA;

    return 1;
}

//------------------------------------------------------------------------------
BOOL LcdPdd_LCD_GetMode(DWORD *pPixelFormat, DWORD *pWidth, DWORD *pHeight, DWORD *pPixelClock)
{
    //  Return default LCD parameters
	LcdPdd_LCD_Init();

    if(pPixelFormat)
        *pPixelFormat = dispc_pixelformat;

    if(pWidth)
        *pWidth = lcd_width;

    if(pHeight)
        *pHeight = lcd_height;

    if(pPixelClock)
        *pPixelClock = (BSP_DPLL4_CLK / dss_clksel) / lcd_pixclkdiv;

    return TRUE;
}

//------------------------------------------------------------------------------
BOOL LcdPdd_TV_GetMode(DWORD *pWidth, DWORD *pHeight, DWORD *pMode)
{
    //  Return default TV parameters
    if( pWidth )
        #if BSP_TVOUT_PAL
            *pWidth = PAL_WIDTH;
        #else
            *pWidth = NTSC_WIDTH;
        #endif
            
    if( pHeight )
        #if BSP_TVOUT_PAL
            *pHeight = PAL_HEIGHT;
        #else
            *pHeight = NTSC_HEIGHT;
        #endif

    if( pMode )
    {
        #if BSP_TVOUT_COMPOSITE
            *pMode = TVOUT_COMPOSITE;
        #else
            *pMode = TVOUT_SVIDEO;
        #endif
    }
        
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL LcdPdd_DVI_Select(BOOL bEnable)
{
    // only changes the select variable, does not perform the change
    bDVIEnabled = bEnable;
    return 1;
}

//------------------------------------------------------------------------------
BOOL LcdPdd_DVI_Enabled(void)
{
    return bDVIEnabled;
}
//------------------------------------------------------------------------------
DWORD LcdPdd_Get_PixClkDiv(void)
{
    return lcd_pixclkdiv;
}

