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
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky, Igor Lantsman
//
//
//  File:  bsp_logo.c
//


//------------------------------------------------------------------------------
//
// includes
//

#include <bsp.h>
#include <bsp_logo.h>
#include <lcd.h>

//------------------------------------------------------------------------------
//
// prototypes
//
void reset_display_controller( void );
void disable_dss( void );
UINT32 enable_lcd_power( void );
UINT32 enable_lcd_backlight( void );
void configure_dss( UINT32 framebuffer );
void display_lcd_image( void );
void lcd_config(UINT32 framebuffer);
void lcd_shutdown(void);
UINT32 disable_lcd_power(void);
UINT32 disable_lcd_backlight(void);

extern card_ver g_cv;

//------------------------------------------------------------------------------
//
// defines
//

#define BYTES_PER_PIXEL             3
#define DELAY_COUNT                 100 
#define LOGO_GFX_ATTRIBUTES         (DISPC_GFX_ATTR_GFXENABLE | DISPC_GFX_ATTR_GFXFORMAT(DISPC_PIXELFORMAT_RGB24))           // RGB24 packed, enabled

#define BSP_LCD_CONFIG              (DISPC_CONFIG_FUNCGATED | DISPC_CONFIG_LOADMODE(2))

#define BSP_GFX_POS                 (DISPC_GFX_POS_GFXPOSY(g_dwLogoPosY) | DISPC_GFX_POS_GFXPOSX(g_dwLogoPosX))
#define BSP_GFX_SIZE                (DISPC_GFX_SIZE_GFXSIZEY(g_dwLogoHeight) | DISPC_GFX_SIZE_GFXSIZEX(g_dwLogoWidth))
#define BSP_GFX_FIFO_THRESHOLD      (DISPC_GFX_FIFO_THRESHOLD_LOW(192) | DISPC_GFX_FIFO_THRESHOLD_HIGH(252))
#define BSP_GFX_ROW_INC             0x00000001
#define BSP_GFX_PIXEL_INC           0x00000001
#define BSP_GFX_WINDOW_SKIP         0x00000000

DWORD   g_dwLogoPosX;
DWORD   g_dwLogoPosY;

DWORD   g_dwLogoWidth;
DWORD   g_dwLogoHeight;

DWORD  g_dwProgressWidth      = 0;   // progress rectangle width in pixels
DWORD  g_dwStepsInRectangle   = 0;   // how many times ShowProgress function should be called to display one more progress rectangle


//#define PROGRESS_1_MIN_X		101 
//#define PROGRESS_1_WIDTH		278
//#define PROGRESS_1_MIN_Y		62 //from bottom
#define PROGRESS_1_HEIGHT		17

UINT32  g_framebuffer		= 0;
UINT32	g_LcdWidth			= 0;
UINT32	g_LcdHeight			= 0;
UINT32  g_ProgressBarWidth	= 278;
UINT32  g_ProgressBarMinY	= 210;

typedef struct _tRgb
{
	UCHAR	RColor;
	UCHAR	GColor;
	UCHAR	BColor;
} Rgb;

//font parameters
UINT32	g_CharWidth		= 8;
UINT32	g_CharHeight	= 16;
UINT32	g_StartChar		= 0x20;
UINT32	g_LastChar		= 0x7E;
UINT	g_CharByteLen	= 16;

////-
const UCHAR g_font[] = 
{//from vgafixr.fnt	
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x18,0x3C,0x3C,0x3C,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x66,0x66,0x66,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x36,0x36,0x7F,0x36,0x36,0x36,0x7F,0x36,0x36,0x00,0x00,0x00,0x00,
0x00,0x18,0x18,0x3C,0x66,0x60,0x30,0x18,0x0C,0x06,0x66,0x3C,0x18,0x18,0x00,0x00,
0x00,0x00,0x70,0xD8,0xDA,0x76,0x0C,0x18,0x30,0x6E,0x5B,0x1B,0x0E,0x00,0x00,0x00,
0x00,0x00,0x00,0x38,0x6C,0x6C,0x38,0x60,0x6F,0x66,0x66,0x3B,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x18,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x0C,0x18,0x18,0x30,0x30,0x30,0x30,0x30,0x18,0x18,0x0C,0x00,0x00,
0x00,0x00,0x00,0x30,0x18,0x18,0x0C,0x0C,0x0C,0x0C,0x0C,0x18,0x18,0x30,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x36,0x1C,0x7F,0x1C,0x36,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x1C,0x0C,0x18,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x1C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x06,0x06,0x0C,0x0C,0x18,0x18,0x30,0x30,0x60,0x60,0x00,0x00,0x00,
0x00,0x00,0x00,0x1E,0x33,0x37,0x37,0x33,0x3B,0x3B,0x33,0x1E,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x0C,0x1C,0x7C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x3C,0x66,0x66,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x3C,0x66,0x66,0x06,0x1C,0x06,0x66,0x66,0x3C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x30,0x30,0x36,0x36,0x36,0x66,0x7F,0x06,0x06,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x7E,0x60,0x60,0x60,0x7C,0x06,0x06,0x0C,0x78,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x1C,0x18,0x30,0x7C,0x66,0x66,0x66,0x66,0x3C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x7E,0x06,0x0C,0x0C,0x18,0x18,0x30,0x30,0x30,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x3C,0x66,0x66,0x76,0x3C,0x6E,0x66,0x66,0x3C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x3C,0x66,0x66,0x66,0x66,0x3E,0x0C,0x18,0x38,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1C,0x1C,0x00,0x00,0x00,0x1C,0x1C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1C,0x1C,0x00,0x00,0x00,0x1C,0x1C,0x0C,0x18,0x00,0x00,
0x00,0x00,0x00,0x06,0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x06,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x60,0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x60,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x3C,0x66,0x66,0x0C,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x7E,0xC3,0xC3,0xCF,0xDB,0xDB,0xCF,0xC0,0x7F,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x18,0x3C,0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x7C,0x66,0x66,0x66,0x7C,0x66,0x66,0x66,0x7C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x3C,0x66,0x66,0x60,0x60,0x60,0x66,0x66,0x3C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x78,0x6C,0x66,0x66,0x66,0x66,0x66,0x6C,0x78,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x7E,0x60,0x60,0x60,0x7C,0x60,0x60,0x60,0x7E,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x7E,0x60,0x60,0x60,0x7C,0x60,0x60,0x60,0x60,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x3C,0x66,0x66,0x60,0x60,0x6E,0x66,0x66,0x3E,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x3C,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x06,0x06,0x06,0x06,0x06,0x06,0x66,0x66,0x3C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x66,0x66,0x6C,0x6C,0x78,0x6C,0x6C,0x66,0x66,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x63,0x63,0x77,0x6B,0x6B,0x6B,0x63,0x63,0x63,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x63,0x63,0x73,0x7B,0x6F,0x67,0x63,0x63,0x63,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x3C,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x7C,0x66,0x66,0x66,0x7C,0x60,0x60,0x60,0x60,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x3C,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x0C,0x06,0x00,0x00,
0x00,0x00,0x00,0x7C,0x66,0x66,0x66,0x7C,0x6C,0x66,0x66,0x66,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x3C,0x66,0x60,0x30,0x18,0x0C,0x06,0x66,0x3C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x63,0x63,0x63,0x6B,0x6B,0x6B,0x36,0x36,0x36,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x66,0x66,0x34,0x18,0x18,0x2C,0x66,0x66,0x66,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x7E,0x06,0x06,0x0C,0x18,0x30,0x60,0x60,0x7E,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x3C,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x3C,0x3C,
0x00,0x00,0x00,0x60,0x60,0x30,0x30,0x18,0x18,0x0C,0x0C,0x06,0x06,0x00,0x00,0x00,
0x00,0x00,0x00,0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x3C,
0x00,0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,
0x00,0x38,0x18,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x3C,0x06,0x06,0x3E,0x66,0x66,0x3E,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x66,0x7C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x06,0x06,0x3E,0x66,0x66,0x66,0x66,0x66,0x3E,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x3C,0x66,0x66,0x7E,0x60,0x60,0x3C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x1E,0x30,0x30,0x30,0x7E,0x30,0x30,0x30,0x30,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x3E,0x66,0x66,0x66,0x66,0x66,0x3E,0x06,0x06,0x7C,0x7C,
0x00,0x00,0x00,0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00,
0x00,0x00,0x18,0x18,0x00,0x78,0x18,0x18,0x18,0x18,0x18,0x7E,0x00,0x00,0x00,0x00,
0x00,0x00,0x0C,0x0C,0x00,0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x78,0x78,
0x00,0x00,0x00,0x60,0x60,0x66,0x66,0x6C,0x78,0x6C,0x66,0x66,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x78,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x7E,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x7E,0x6B,0x6B,0x6B,0x6B,0x6B,0x63,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x66,0x7C,0x60,0x60,0x60,0x60,
0x00,0x00,0x00,0x00,0x00,0x3E,0x66,0x66,0x66,0x66,0x66,0x3E,0x06,0x06,0x06,0x06,
0x00,0x00,0x00,0x00,0x00,0x66,0x6E,0x70,0x60,0x60,0x60,0x60,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x3E,0x60,0x60,0x3C,0x06,0x06,0x7C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x30,0x30,0x7E,0x30,0x30,0x30,0x30,0x30,0x1E,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x66,0x66,0x3E,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x63,0x6B,0x6B,0x6B,0x6B,0x36,0x36,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x0C,0x18,0xF0,0xF0,
0x00,0x00,0x00,0x00,0x00,0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x0C,0x18,0x18,0x18,0x30,0x60,0x30,0x18,0x18,0x18,0x0C,0x00,0x00,
0x00,0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x00,0x00,0x00,0x30,0x18,0x18,0x18,0x0C,0x06,0x0C,0x18,0x18,0x18,0x30,0x00,0x00,
0x00,0x00,0x00,0x71,0xDB,0x8E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

//-------------------------------------------------------------------------------
VOID	InitLcdParams()
{
	LcdPdd_GetMemory( NULL, &g_framebuffer );

		 //  Get the LCD width and height
	LcdPdd_LCD_GetMode(NULL, &g_LcdWidth, &g_LcdHeight, 0);

	g_ProgressBarWidth	= 58 * g_LcdWidth / 100;
	g_ProgressBarMinY	= 77 * g_LcdHeight / 100;
}
//-------------------------------------------------------------------------------
VOID DrawHLine(int y, int xStart, int xStop, Rgb* colour)
{
    int x;
	UCHAR* FlatFrameBuffer = (UCHAR*)g_framebuffer;

    for(x = xStart; x < xStop; x++)
    {
        FlatFrameBuffer[BYTES_PER_PIXEL*(g_LcdWidth*y + x)]		= colour->BColor;
        FlatFrameBuffer[BYTES_PER_PIXEL*(g_LcdWidth*y + x) + 1]	= colour->GColor;
        FlatFrameBuffer[BYTES_PER_PIXEL*(g_LcdWidth*y + x) + 2]	= colour->RColor;
    }
}
//-------------------------------------------------------------------------------
VOID DrawVLine(int x, int yStart, int yStop, Rgb* colour)
{
    int y;
	UCHAR* FlatFrameBuffer = (UCHAR*)g_framebuffer;
    
    for(y = yStart; y < yStop; y++)
    {
        FlatFrameBuffer[BYTES_PER_PIXEL*(g_LcdWidth*y + x)]		= colour->BColor;
        FlatFrameBuffer[BYTES_PER_PIXEL*(g_LcdWidth*y + x) + 1]	= colour->GColor;
        FlatFrameBuffer[BYTES_PER_PIXEL*(g_LcdWidth*y + x) + 2]	= colour->RColor;
   }
}
//-------------------------------------------------------------------------------
VOID DrawBox(int x, int y, int w, int h, Rgb* colour)
{
    for(; h >= 0; h--)
    {
		DrawHLine(y + h, x, x + w, colour);
    }
}
//-------------------------------------------------------------------------------
VOID DrawFrame(int x, int y, int w, int h, int weight, Rgb* colour)
{
	int i;
	
	DrawBox(x - weight, y - weight,w + weight * 2, weight, colour);
	DrawBox(x - weight, y + h,		w + weight * 2, weight, colour);
	
	for( i = 0; i < weight; ++i )
	{
		DrawVLine(x + i - weight,	y, y + h, colour);
		DrawVLine(x + w + i,		y, y + h, colour);
	}
}
//-------------------------------------------------------------------------------
VOID TextAlign( int* px, int* py, UINT32 Len, UINT32 Mode)
{
	if(Mode & TX_RIGHT)
	{
		*px = g_LcdWidth - Len - *px;
	}	
	else if(Mode & TX_CENTER )
	{
		*px = (g_LcdWidth - Len)/2;
	}
	if(Mode & TX_BOTTOM)
	{
		*py = g_LcdHeight - g_CharHeight - *py;
	}
	else if(Mode & TX_VCENTER)
	{
		*py = (g_LcdHeight - g_CharHeight)/2;
	}
	//TX_TOP	TX_LEFT - do nothing
}
//-------------------------------------------------------------------------------
DWORD TextPixLen( UCHAR* Text )
{
	return strlen(Text) * g_CharWidth;
}
//-------------------------------------------------------------------------------
DWORD TextPixLenW( WCHAR* Text )
{
	return wcslen(Text) * g_CharWidth;
}
//-------------------------------------------------------------------------------
void DrawChar(const UCHAR* cc, int xStart, int y, Rgb* colour)
{
	UCHAR Mask;
	DWORD i, j, x;
	UCHAR* FlatFrameBuffer = (UCHAR*)g_framebuffer;

	for( j = 0; j < g_CharHeight; ++j )
	{
		Mask = 0x80;
		x	= xStart;

		for( i = 0; i < g_CharWidth; ++i )//for g_CharWidth <= 8 only
		{
			if(Mask & cc[j])
			{
				FlatFrameBuffer[BYTES_PER_PIXEL*(g_LcdWidth*(y + j) + x)]		= colour->BColor;
				FlatFrameBuffer[BYTES_PER_PIXEL*(g_LcdWidth*(y + j) + x) + 1]	= colour->GColor;
				FlatFrameBuffer[BYTES_PER_PIXEL*(g_LcdWidth*(y + j) + x) + 2]	= colour->RColor;
			}
			Mask >>= 1;
			x += 1;
		}
	}
}
//-------------------------------------------------------------------------------
VOID GetColor( UINT32 Cmode, Rgb* rgb )
{
	switch(Cmode)
	{
		case 	CL_BGROUND:
		{
			rgb->BColor = 0;//165;    //  Blue
			rgb->GColor = 0;//109;    //  Green
			rgb->RColor = 0;//57;    //  Red
		}
		break;
		case	CL_TEXT:
		{
			rgb->BColor = 255;    //  Blue
			rgb->GColor = 255;    //  Green
			rgb->RColor = 255;    //  Red
		}
		break;
		case	CL_PROGRESS:
		{		
			rgb->BColor = 192;//0;    //  Blue
			rgb->GColor = 192;//255;    //  Green
			rgb->RColor = 192;//128;    //  Red
		}
		break;
		case	CL_PROGRESS_FR:
		{		
			rgb->BColor = 180;	//0;//200;	//255;    //100;//0;    //  Blue
			rgb->GColor = 115;	//128;//10;	//255;    //100;//255;    //  Green
			rgb->RColor = 45;	//0;//80;	//255;    //100;//128;    //  Red
		}
		break;
		case	CL_RED:
		{		
			rgb->BColor = 0;    //  Blue
			rgb->GColor = 0;    //  Green
			rgb->RColor = 255;  //  Red
		}
		break;
	}
}
//----------------------------------------------------------------------------------------------------
const UCHAR* CharPtr( UCHAR ch )
{
	const UCHAR* pChar = pChar = g_font;//space

	if(	ch >= g_StartChar && ch <= g_LastChar )//if char is not into font table - skip 
		pChar += (ch - g_StartChar) * g_CharByteLen; 

	return pChar;
}
//----------------------------------------------------------------------------------------------------
void TextWriteW(int x, int y, WCHAR* Txt, UINT32 Mode, UINT32 ColorInd)
{
	DWORD	PixLen, Len, i = 0;
	Rgb		color;

	InitLcdParams();

	PixLen = TextPixLenW(Txt);
	TextAlign(&x, &y, PixLen, Mode);

	GetColor( CL_BGROUND, &color );
	DrawBox(x, y, PixLen, g_CharHeight, &color);

	if(0 == ColorInd)
		ColorInd = CL_TEXT;
	GetColor( ColorInd, &color );

	Len = wcslen(Txt);
	for(i = 0; i < Len; ++i )
	{
		DrawChar(CharPtr((UCHAR)Txt[i]), x + i * g_CharWidth, y, &color);
	}
}
//----------------------------------------------------------------------------------------------------
void TextWrite(int x, int y, UCHAR* Txt, UINT32 Mode, UINT32 ColorInd)
{
	DWORD	PixLen, Len, i = 0;
	Rgb		color;

	InitLcdParams();

	PixLen = TextPixLen(Txt);
	TextAlign(&x, &y, PixLen, Mode);

	GetColor( CL_BGROUND, &color );
	DrawBox(x, y, PixLen, g_CharHeight, &color);

	if(0 == ColorInd)
		ColorInd = CL_TEXT;
	
	GetColor( ColorInd, &color );

	Len = strlen(Txt);
	for(i = 0; i < Len; ++i )
	{
		DrawChar(CharPtr(Txt[i]), x + i * g_CharWidth, y, &color);
	}
}
//------------------------------------------------------------------------------
//
//  Function:  InitProgressData
//
//  This function initializes Progress rectangle size
//
BOOL  SetProgressData(DWORD NoOfRectabgles, DWORD stepsNo)
{
	OALMSG(0, (L"SetProgressData: NoOfRectabgles = %d, stepsNo = %d \r\n", NoOfRectabgles,stepsNo));

	InitLcdParams();

	if(NoOfRectabgles)
		FillProgress(0);//maybe temp!!! place

	g_dwProgressWidth    = (NoOfRectabgles) ? ( g_ProgressBarWidth / NoOfRectabgles ) : 0;
	g_dwStepsInRectangle = stepsNo;

	return TRUE;
}
//------------------------------------------------------------------------------
BOOL FillProgress( INT32 fOn )
{
	Rgb		color;

	InitLcdParams();
	
	if(1 == fOn)
	{
		GetColor(CL_PROGRESS, &color);
	}
	else
	{
		GetColor(CL_BGROUND, &color);
	}

	DrawBox((g_LcdWidth - g_ProgressBarWidth) / 2, g_ProgressBarMinY, g_ProgressBarWidth, PROGRESS_1_HEIGHT, &color);

	if(-1 == fOn)//erase only
		return 1;

	//temp!!! maybe frame's another color
	GetColor(CL_PROGRESS_FR, &color);
	DrawFrame((g_LcdWidth - g_ProgressBarWidth) / 2, g_ProgressBarMinY, g_ProgressBarWidth, PROGRESS_1_HEIGHT, 2, &color);

	return 1;
}
//------------------------------------------------------------------------------
BOOL ShowProgressLoop()
{   
	static DWORD dwStepNo		= 0;
	static DWORD dwRectangleNo	= 0;

	Rgb		color;

	// failed if Progress data is not initialized
	if (!g_dwProgressWidth || !g_dwStepsInRectangle)
	{
		dwStepNo		= 0;		
		dwRectangleNo	= 0;

		OALMSG(0, (L"ShowProgressLoop clean. \r\n"));
		return FALSE;
	}

	dwStepNo++;
	
	if(dwStepNo >= g_dwStepsInRectangle)
	{	
		DWORD Height, MinY, MinX = 0, Width = 0;

		dwStepNo = 0;
		dwRectangleNo++;

		InitLcdParams();

		MinY	= g_ProgressBarMinY;
		Height	= PROGRESS_1_HEIGHT;
		
		MinX	= (g_LcdWidth - g_ProgressBarWidth) / 2 + g_dwProgressWidth * (dwRectangleNo - 1 );
		Width	= g_dwProgressWidth;
		
		if( g_ProgressBarWidth <= g_dwProgressWidth * dwRectangleNo )
		{
			Width = g_dwProgressWidth + g_ProgressBarWidth - g_dwProgressWidth * dwRectangleNo;
			dwRectangleNo = 0;
		}

		GetColor(CL_BGROUND, &color);
		DrawBox((g_LcdWidth - g_ProgressBarWidth) / 2, MinY, g_ProgressBarWidth, Height, &color);
		GetColor(CL_PROGRESS, &color);
		DrawBox(MinX, MinY, Width, Height, &color);
	}

	return TRUE;
}
//------------------------------------------------------------------------------
//
//  Function:  ShowProgress
//
//  This function shows the bootloader progress indication
//

BOOL ShowProgress()
{   
	static DWORD	dwStepNo		= 0;
	static DWORD	dwRectangleNo	= 0;
	Rgb				color;

	// failed if Progress data is not initialized
	if (!g_dwProgressWidth || !g_dwStepsInRectangle)
	{
		OALMSG(0, (L"Error: Progress data is not initialized yet. \r\n"));
		return FALSE;
	}

	dwStepNo++;
	
	if (dwStepNo == g_dwStepsInRectangle)
	{	
		DWORD Height, MinY, MinX = 0, Width = 0;

		dwStepNo = 0;
		dwRectangleNo++;

		InitLcdParams();
	
		Height	= PROGRESS_1_HEIGHT;
		MinY		= g_ProgressBarMinY;
		
		MinX	= (g_LcdWidth - g_ProgressBarWidth) / 2;
		Width	= g_dwProgressWidth * dwRectangleNo;

		if( g_ProgressBarWidth < Width )
		{
			Width = g_ProgressBarWidth;
		}
		GetColor(CL_PROGRESS, &color);
		DrawBox(MinX, MinY, Width, Height, &color);
	}
	return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ShowLogo
//
//  This function shows the logo splash screen
//
VOID ShowLogo(UINT32 flashAddr, UINT32 offset)
{
    HANDLE  hFlash = NULL;
    PUCHAR  pChar;
    ULONG   y, i;
    DWORD   Line = 0;
    DWORD   Junk = 0;

	BMP_HEADER		bmp;
	Rgb				color;
	static BOOL		fFirst	= 0;
  
	//  Get the LCD width and height//  Get the video memory
	InitLcdParams();
    pChar = (PUCHAR)g_framebuffer;

	//fill always
	GetColor(CL_BGROUND, &color);
	DrawBox(0, 0, g_LcdWidth, g_LcdHeight, &color);
   
	if (flashAddr != -1)
	{
		if( flashAddr ) //0 - for RAM
		{// Open flash storage
			hFlash = OALFlashStoreOpen(flashAddr);
		}
		if( !flashAddr || hFlash != NULL )
		{
			//  The LOGO reserved NAND flash region contains the BMP file
			if( flashAddr )
				OALFlashStoreBufferedRead( hFlash, offset, (UCHAR*)&bmp, sizeof(bmp), FALSE );
			else
				memcpy( (UCHAR*)&bmp, (UCHAR*)offset, sizeof(bmp) );

			//  Check for 'BM' signature
			if( bmp.Type			== 0x4D42							&&
				bmp.BitsPerPixel	== BYTES_PER_PIXEL * 8				&&
				bmp.Width			<= g_LcdWidth						&&
				bmp.Height			<= g_LcdHeight						)  

			{
				g_dwLogoPosX   = (g_LcdWidth - bmp.Width)/2;
				g_dwLogoPosY   = 0;//(LcdHeight - bmp.Height)/2;

				Line = bmp.Width * BYTES_PER_PIXEL;			//1 + ((bmp.Width * bmp.BitsPerPixel - 1) >> 3;
				Junk = (~(bmp.Width * bmp.BitsPerPixel - 1) >> 3) & 3;
			
				if( flashAddr )
				{
					//read the whole file
					OALFlashStoreBufferedRead(	hFlash, offset, (UCHAR*)IMAGE_DISPLAY_BUF_PA + (IMAGE_DISPLAY_BUF_SIZE>>1), 
												(Line + Junk) * bmp.Height + bmp.Offset, 0);

					OALFlashStoreClose(hFlash);
					offset = IMAGE_DISPLAY_BUF_PA + (IMAGE_DISPLAY_BUF_SIZE>>1);

				}

				offset	+= bmp.Offset;
				i		= bmp.Height;
				y		= g_dwLogoPosY + bmp.Height;

				while(i--)
				{
					y -= 1;
					memcpy(	(VOID*)(pChar + BYTES_PER_PIXEL * (g_LcdWidth * y + g_dwLogoPosX)), (UCHAR*)offset, Line);
					offset += Line + Junk;
				}
			}
		}
	}
	//to show whole screen
	g_dwLogoPosX   = 0;
	g_dwLogoPosY   = 0;
	g_dwLogoWidth  = g_LcdWidth;
	g_dwLogoHeight = g_LcdHeight;

 	OALMSG(0, (L"LcdHeight=%d,LcdWidth =%d \r\n",g_LcdHeight, g_LcdWidth));

    if(!fFirst)
	{
		lcd_config(g_framebuffer);
		fFirst = 1;
	}
}	

//------------------------------------------------------------------------------
//
//  Function:  HideLogo
//
//  This function hides the logo splash screen
//
VOID HideLogo(VOID)
{
    lcd_shutdown();
}

//------------------------------------------------------------------------------
//
//  Function:  reset_display_controller
//
//  This function resets the Display Sub System on omap24xx
//
void reset_display_controller( void )
{
    
    UINT32 reg_val;
    UINT16 count;
    UINT32 timeout;
    UINT32 fclk, iclk;
    OMAP_PRCM_DSS_CM_REGS *pPrcmRegs = OALPAtoUA(OMAP_PRCM_DSS_CM_REGS_PA);
    OMAP_DISPC_REGS  *pDisplayRegs = OALPAtoUA(OMAP_DISC1_REGS_PA);
    
     //OALMSG(OAL_INFO, (L"reset_display_controller+\r\n"));

    // enable all display clocks
    fclk = INREG32(&pPrcmRegs->CM_FCLKEN_DSS);
    iclk = INREG32(&pPrcmRegs->CM_ICLKEN_DSS);

    OUTREG32(&pPrcmRegs->CM_FCLKEN_DSS, (fclk | CM_CLKEN_DSS1 | CM_CLKEN_DSS2 | CM_CLKEN_TV));
    OUTREG32(&pPrcmRegs->CM_ICLKEN_DSS, (iclk | CM_CLKEN_DSS));
  
    // disable the display controller
    disable_dss();

    // reset the display controller
    OUTREG32(&pDisplayRegs->DISPC_SYSCONFIG, DISPC_SYSCONFIG_SOFTRESET);
    
    // wait until reset completes OR timeout occurs
    timeout=10000;
    while(!((reg_val=INREG32(&pDisplayRegs->DISPC_SYSSTATUS)) & DISPC_SYSSTATUS_RESETDONE) && (timeout > 0))
    {
        // delay
        for(count=0;count<DELAY_COUNT;++count);
        timeout--;
    }

    if(!(reg_val & DISPC_SYSSTATUS_RESETDONE))
    {
        // OALMSG(OAL_INFO, (L"reset_display_controller: DSS reset timeout\r\n"));
    }
    
    reg_val=INREG32(&pDisplayRegs->DISPC_SYSCONFIG);
    reg_val &=~(DISPC_SYSCONFIG_SOFTRESET);
    OUTREG32(&pDisplayRegs->DISPC_SYSCONFIG,reg_val);


    // restore old clock settings
    OUTREG32(&pPrcmRegs->CM_FCLKEN_DSS, fclk);
    OUTREG32(&pPrcmRegs->CM_ICLKEN_DSS, iclk);
    
     //OALMSG(OAL_INFO, (L"reset_display_controller-\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  lcd_config
//
//  This function configures the LCD
//
void lcd_config(UINT32 framebuffer)
{
    disable_lcd_backlight();
    reset_display_controller();

    // Enable LCD clocks
	// LCD power up sequence
	// t2 = 60 ms before first data typical for segcom panel
	OALStall(60*1000);

    enable_lcd_power();

    // Configure the DSS registers
    configure_dss(framebuffer);
      
    // Display data on LCD
    display_lcd_image() ;
	
	// LCD power up sequence
	// t3 = 200 ms before first data
	OALStall(300*1000);

#if BSP_DVI_ENABLE
    // DVI is selected, disable backlight
    disable_lcd_backlight();
#else
    // Turn on backlight last
    enable_lcd_backlight();
#endif
}

//------------------------------------------------------------------------------
//
//  Function:  lcd_shutdown
//
//  This function disables the backlight and power of the LCD controller
//
void lcd_shutdown()
{
    disable_lcd_backlight();
//    LcdPdd_SetPowerLevel(D4);
//    disable_lcd_power();
}   

//------------------------------------------------------------------------------
//
//  Function:  disable_dss
//
//  This function disables the Display Sub System on omap24xx
//
void disable_dss( void )
{
     //OALMSG(OAL_INFO, (L"disable_dss+\r\n"));

     //OALMSG(OAL_INFO, (L"disable_dss-\r\n"));
}
     
//------------------------------------------------------------------------------
//
//  Function:  enable_lcd_power
//
//  This function enables the power for the LCD controller
//
UINT32 enable_lcd_power( void )
{
    OMAP_PRCM_DSS_CM_REGS *pPrcmRegs = OALPAtoUA(OMAP_PRCM_DSS_CM_REGS_PA);
    
     //OALMSG(OAL_INFO, (L"enable_lcd_power+\r\n"));

    SETREG32(&pPrcmRegs->CM_FCLKEN_DSS, (CM_CLKEN_DSS1 | CM_CLKEN_DSS2));
    SETREG32(&pPrcmRegs->CM_ICLKEN_DSS, (CM_CLKEN_DSS));

     //OALMSG(OAL_INFO, (L"enable_lcd_power-\r\n"));

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  disable_lcd_power
//
//  This function disables the power for the LCD controller
//
UINT32 disable_lcd_power( void )
{
    OMAP_PRCM_DSS_CM_REGS *pPrcmRegs = OALPAtoUA(OMAP_PRCM_DSS_CM_REGS_PA);

     //OALMSG(OAL_INFO, (L"disable_lcd_power+\r\n"));

    CLRREG32(&pPrcmRegs->CM_FCLKEN_DSS, (CM_CLKEN_DSS1 | CM_CLKEN_DSS2));
    CLRREG32(&pPrcmRegs->CM_ICLKEN_DSS, (CM_CLKEN_DSS));
    
     //OALMSG(OAL_INFO, (L"disable_lcd_power-\r\n"));
    return ERROR_SUCCESS;
}
      
//------------------------------------------------------------------------------
//
//  Function:  enable_lcd_backlight
//
//  This function enables the backlight for the LCD controller
//
UINT32 enable_lcd_backlight( void )
{
	int i;
    void* hTwl;
    
     //OALMSG(OAL_INFO, (L"enable_lcd_backlight+\r\n"));

    // Enable LEDA on TPS659XX
    hTwl = OALTritonOpen();

    // The hardware design is completely backwards.  
    // In order to get 100% brightness, the LEDPWM must 
    // be disabled.
    // Clear LEDAON, LEDAPWM
    OALTritonWrite(hTwl, TWL_PWMAON, 0);
    OALTritonWrite(hTwl, TWL_PWMBON, 0);
    OALTritonWrite(hTwl, TWL_PWMAOFF, 2);
    OALTritonWrite(hTwl, TWL_PWMBOFF, 2);
    OALTritonWrite(hTwl, TWL_LEDEN, 0x11);
	for(i = 1; i < 100; i+=2)
	{
		OALTritonWrite(hTwl, TWL_PWMAOFF, i);
		OALTritonWrite(hTwl, TWL_PWMBOFF, i);
		OALStall(20*1000);
	}
    OALTritonWrite(hTwl, TWL_PWMAOFF, 127);
    OALTritonWrite(hTwl, TWL_PWMBOFF, 127);

	OALTritonClose(hTwl);
    
     //OALMSG(OAL_INFO, (L"enable_lcd_backlight-\r\n"));
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  disable_lcd_backlight
//
//  This function disables the backlight for the LCD controller
//
UINT32 disable_lcd_backlight( void )
{
    void* hTwl;

     //OALMSG(OAL_INFO, (L"disable_lcd_backlight+\r\n"));
    // Enable LEDA on TPS659XX
    hTwl = OALTritonOpen();

    // The hardware design is completely backwards.  In order
    // to disable the LED control signal, the LEDPWM signal must 
    // be enabled 100%
    // Set LEDAON, LEDAPWM
    OALTritonWrite(hTwl, TWL_LEDEN, 0x00);
    // Set PWM registers to same value to trigger 100% duty cycle
    OALTritonWrite(hTwl, TWL_PWMAOFF, 0x00);
    OALTritonWrite(hTwl, TWL_PWMAON, 0x00);
    
    OALTritonClose(hTwl);
     //OALMSG(OAL_INFO, (L"disable_lcd_backlight-\r\n"));
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  configure_dss
//
//  This function configures the Display Sub System on omap35xx
//
void configure_dss( UINT32 framebuffer )
{
    OMAP_DSS_REGS   *pDSSRegs = OALPAtoUA(OMAP_DSS1_REGS_PA);
    OMAP_DISPC_REGS *pDisplayRegs = OALPAtoUA(OMAP_DISC1_REGS_PA);
    OMAP_PRCM_DSS_CM_REGS* pPrcmDssCMRegs = OALPAtoUA(OMAP_PRCM_DSS_CM_REGS_PA);
    
    // OALMSG(OAL_INFO, (L"configure_dss+\r\n"));

    //  Configure the clock source
    OUTREG32( &pDSSRegs->DSS_CONTROL, 
                DSS_CONTROL_DISPC_CLK_SWITCH_DSS1_ALWON |
                DSS_CONTROL_DSI_CLK_SWITCH_DSS1_ALWON
                );
    
    //  Configure interconnect parameters
    OUTREG32( &pDSSRegs->DSS_SYSCONFIG, DISPC_SYSCONFIG_AUTOIDLE );
    OUTREG32( &pDisplayRegs->DISPC_SYSCONFIG, DISPC_SYSCONFIG_AUTOIDLE|SYSCONFIG_NOIDLE|SYSCONFIG_NOSTANDBY );

    // Not enabling any interrupts
    OUTREG32( &pDisplayRegs->DISPC_IRQENABLE, 0x00000000 );
    
    
    //  Configure the LCD
    LcdPdd_LCD_Initialize( pDSSRegs, pDisplayRegs, NULL, NULL, pPrcmDssCMRegs );
    
    //  Over-ride default LCD config
    OUTREG32(&pDisplayRegs->DISPC_CONFIG,BSP_LCD_CONFIG);
    
    
    // Configure Graphics Window
    //--------------------------
    
    OUTREG32(&pDisplayRegs->DISPC_GFX_BA0 ,framebuffer );
 
    // configure the position of graphics window
    OUTREG32(&pDisplayRegs->DISPC_GFX_POSITION,BSP_GFX_POS);
    
    // size of graphics window
    OUTREG32(&pDisplayRegs->DISPC_GFX_SIZE,BSP_GFX_SIZE);
    
    // GW Enabled, RGB24 packed, Little Endian
    OUTREG32(&pDisplayRegs->DISPC_GFX_ATTRIBUTES,LOGO_GFX_ATTRIBUTES);
    
    OUTREG32(&pDisplayRegs->DISPC_GFX_FIFO_THRESHOLD,BSP_GFX_FIFO_THRESHOLD);
    OUTREG32(&pDisplayRegs->DISPC_GFX_ROW_INC,BSP_GFX_ROW_INC); 
    OUTREG32(&pDisplayRegs->DISPC_GFX_PIXEL_INC,BSP_GFX_PIXEL_INC); 
    OUTREG32(&pDisplayRegs->DISPC_GFX_WINDOW_SKIP,BSP_GFX_WINDOW_SKIP);

    // OALMSG(OAL_INFO, (L"configure_dss-\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  display_lcd_image
//
//  This function displays the image in the frame buffer on the LCD
//
void display_lcd_image( void )
{
    UINT8  count, timeout = DELAY_COUNT  ;
    UINT16 ctrl;
    OMAP_DISPC_REGS *pDisplayRegs = OALPAtoUA(OMAP_DISC1_REGS_PA);
    
    // Apply display configuration
    SETREG32(&pDisplayRegs->DISPC_CONTROL,DISPC_CONTROL_GOLCD);
    
    // wait for configuration to take effect
    do
    {
        for(count=0;count<DELAY_COUNT;++count);
        ctrl=INREG16(&pDisplayRegs->DISPC_CONTROL);
        timeout-- ;
    }
    while((ctrl & DISPC_CONTROL_GOLCD) && (timeout > 0));
    
    // Power up and start scanning
    LcdPdd_SetPowerLevel(D0);   
    
     //OALMSG(OAL_INFO, (L"display_lcd_image-\r\n"));
}

//------------------------------------------------------------------------------
//
// end of bsp_logo.c
