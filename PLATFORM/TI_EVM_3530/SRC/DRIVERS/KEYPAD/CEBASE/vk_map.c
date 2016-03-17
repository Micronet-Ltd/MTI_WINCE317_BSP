// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
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
// by Vladimir Zatulovsky
//
//
//  File: vk_map.c
//
//  This file containts definition for keyboard row/column to virtual key
//  mapping.
//
#include <windows.h>
#include <winuserm.h>
#include <keypad.h>

#ifndef dimof
#define dimof(x)            (sizeof(x)/sizeof(x[0]))
#endif

#if KEYPAD_ROWS < 6 || KEYPAD_COLUMNS < 6
    #error g_keypadVK dimensions do not match KEYPAD_ROWS and/or KEYPAD_COLUMNS
#endif

const UCHAR g_keypadVK_Default[KEYPAD_ROWS * KEYPAD_COLUMNS] = {
//  Column 0	Column 1	Column 2	Column 3	Column 4	Column 5	Column 6	Column 7
      VK_DOWN,	  VK_DOWN,	  VK_DOWN,	  VK_DOWN,    VK_DOWN,	  VK_DOWN,    VK_DOWN,    VK_DOWN,     	// Row 0
		    0,		    0,		    0,		    0, 		    0,     	    0,          0,          0,     	// Row 1
     VK_RIGHT,	 VK_RIGHT,	 VK_RIGHT,	 VK_RIGHT,	 VK_RIGHT,   VK_RIGHT,	 VK_RIGHT,	 VK_RIGHT,     	// Row 2
      VK_LEFT,    VK_LEFT,    VK_LEFT,	  VK_LEFT, 	  VK_LEFT,    VK_LEFT,    VK_LEFT,    VK_LEFT,		// Row 3
	 VK_SPACE,   VK_SPACE,   VK_SPACE,	 VK_SPACE,	 VK_SPACE,	 VK_SPACE,	 VK_SPACE,	 VK_SPACE,		// Row 4
	VK_ESCAPE,	VK_ESCAPE,	VK_ESCAPE,	VK_ESCAPE,	VK_ESCAPE,	VK_ESCAPE,	VK_ESCAPE,	VK_ESCAPE,      // Row 5
		VK_UP,		VK_UP,		VK_UP,		VK_UP,		VK_UP,		VK_UP,		VK_UP,		VK_UP,      // Row 6
    VK_RETURN,	VK_RETURN,	VK_RETURN,	VK_RETURN,	VK_RETURN,	VK_RETURN,	VK_RETURN,	VK_RETURN,		// Row 7
 };

const UCHAR g_powerVK[1] = {
//  Column 0
    VK_OFF // Row 0
 };

//------------------------------------------------------------------------------
const KEYPAD_REMAP g_keypadRemap = { 0, NULL };

const KEYPAD_REMAP g_powerRemap = { 0, 0 };

//------------------------------------------------------------------------------

const KEYPAD_REPEAT_ITEM g_repeatItems_Default[] = {
    { VK_UP,     1000, 200, FALSE, NULL },
    { VK_RIGHT,  1000, 200, FALSE, NULL },
    { VK_LEFT,   1000, 200, FALSE, NULL },
    { VK_DOWN,   1000, 200, FALSE, NULL },
    { VK_RETURN, 1000, 200, FALSE, NULL }
//	{ VK_ESCAPE, 1000, 200, FALSE, NULL },
//  { VK_SPACE,  1000, 200, FALSE, NULL }
};
UINT32 def_rep = dimof(g_repeatItems_Default);

KEYPAD_REPEAT g_keypadRepeat = {(USHORT)dimof(g_repeatItems_Default), (KEYPAD_REPEAT_ITEM *)g_repeatItems_Default};
const KEYPAD_REPEAT g_powerRepeat = { 0, 0 };

//------------------------------------------------------------------------------

const KEYPAD_EXTRASEQ g_keypadExtraSeq = { 0, NULL };

//------------------------------------------------------------------------------


KEYPAD_LONGPRESS_ITEM g_longPressItems[] = {
	{VK_OFF, VK_ESCAPE, KEYPAD_LONG_PRESS_DEFAULT},
	{VK_OFF1, VK_SPACE, KEYPAD_LONG_PRESS_DEFAULT}
};

KEYPAD_LONGPRESS g_keypadLongPressAllEnable		=	{dimof(g_longPressItems), g_longPressItems };
KEYPAD_LONGPRESS g_keypadLongPressOffEnable		=	{1, g_longPressItems };
KEYPAD_LONGPRESS g_keypadLongPressSuspendEnable =	{1, &g_longPressItems[1] };
KEYPAD_LONGPRESS g_keypadLongPressOffDisable	=	{0, 0 };



//------------------------------------------------------------------------------