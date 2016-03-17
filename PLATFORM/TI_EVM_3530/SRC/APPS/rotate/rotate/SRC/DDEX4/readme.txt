//-----------------------------------------------------------------------------
// 
// Sample Name: DDEx4 Sample
// 
// Copyright (c) 1998-2000 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  The DDEx4 program is an extension of DDEx3. It demonstrates a simple 
  animation technique.

User's Guide
============
  DDEx4 requires no user input. Press F12 or ESC to quit the program.

  This program requires at least 1.2 MB of video RAM.

Programming Notes
=================
  The DDEx4 program creates two off-screen surfaces. It loads a bitmap 
  containing a series of animation images, and a background image onto these 
  surfaces. To create the animation, it blits portions of the off-screen 
  surface containing the frame of animation to the back surface, then flips the 
  front and back surfaces.

  The blitting routines illustrate the use of a source color key to create a 
  sprite with a transparent background.

  Note: This sample requires hardware flipping support.  If the display
  driver doesn't support page flipping the sample will exit during
  initialization.
