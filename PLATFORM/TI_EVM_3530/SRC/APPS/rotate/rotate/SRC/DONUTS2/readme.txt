//-----------------------------------------------------------------------------
// 
// Sample Name: Space Donuts Sample
// 
// Copyright (c) 1998-2000 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  This simple game shows how to combine DirectDraw and waveform audio. 
  Although it demonstrates other DirectX components, it is 
  primarily intended to show how to animate multiple sprites.

User's Guide
============
  Input is from the keyboard. No other input device is supported.

  The commands are listed on the opening screen. All numbers must be entered 
  from the numeric keypad.

  Key	Command

  

  ESC, F12	Quit

  4	Turn left

  6	Turn right

  5 	Stop

  8	Accelerate forward

  2	Accelerate backward

  7 	Shield

  SPACEBAR Fire

  ENTER	Start game

  F1	Toggle trailing afterimage effect on/off

  F3	Turn audio on/off

  F5	Toggle frame rate display on/off

  F10	Main menu


  The game uses the following command line switches, which are case-sensitive:

  e	Use software emulation, not hardware acceleration

  t	Test mode, no input required

  x	Stress mode. Never stop if you can help it

  S	Turn sound off/on


Programming Notes
=================
  This game demonstrates many of the features of DirectDraw. It takes advantage 
  of hardware acceleration if it is supported by the driver. 

  The program requires less than 1 MB of video RAM. 


