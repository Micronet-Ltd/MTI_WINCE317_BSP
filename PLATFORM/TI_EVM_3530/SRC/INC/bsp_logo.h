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
//  Header:  bsp_logo.h
//
#ifndef __BSP_LOGO_H
#define __BSP_LOGO_H


//------------------------------------------------------------------------------
//
//  Function:  InitProgressData
//
//  This function initializes number of progres rectangles in the line
//                            and number of steps (ShowProgress function calls)
//                            to be done to display one more progress rectangle
//


BOOL SetProgressData(DWORD NoOfRectabgles, DWORD stepsNo);

//------------------------------------------------------------------------------
//
//  Function:  ShowProgress
//
//  This function shows the bootloader progress indication
//

BOOL ShowProgress();

//------------------------------------------------------------------------------
//
//  Function:  ShowLogo
//
//  This function shows the logo splash screen
//
VOID ShowLogo(UINT32 flashAddr, UINT32 offset);


//------------------------------------------------------------------------------
//
//  Function:  HideLogo
//
//  This function hides the logo splash screen
//
VOID HideLogo(VOID);


BOOL FillProgress( INT32 fOn );
BOOL ShowProgressLoop();
void TextWrite( int x, int y, UCHAR* Txt, UINT32 Mode, UINT32 ColorInd);
void TextWriteW( int x, int y, WCHAR* Txt, UINT32 Mode, UINT32 ColorInd);
DWORD TextPixLen( UCHAR* Text );
DWORD TextPixLenW( WCHAR* Text );

#pragma pack(1)
typedef struct _BMP_HEADER
{
    WORD    Type;
    DWORD   Size;
    DWORD   Reserved;
    DWORD   Offset;
    DWORD   headerSize;
    DWORD   Width;
    DWORD   Height;
    WORD    Planes;
    WORD    BitsPerPixel;
    DWORD   Compression;
    DWORD   SizeImage;
    DWORD   XPixelsPerMeter;
    DWORD   YPixelsPerMeter;
    DWORD   ColorsUsed;
    DWORD   ColorsImportant;
} BMP_HEADER;
#pragma pack()

typedef enum
{
	CL_BGROUND = 1,
	CL_TEXT,
	CL_PROGRESS,
	CL_PROGRESS_FR,
	CL_RED,
	CL_MAX
} ColorMode;
typedef enum
{
	TX_LEFT,
	TX_RIGHT	= 1,
	TX_CENTER	= 2,
	TX_VCENTER	= 4,
	TX_TOP		= 8,
	TX_BOTTOM	= 0x10
} TX_ALIGN;

#endif // __BSP_LOGO_H
