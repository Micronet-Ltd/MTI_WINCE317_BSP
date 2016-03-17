//-----------------------------------------------------------------------------
// 
// Sample Name: Mosquito Sample
// 
// Copyright (c) 1998-2000 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  This program demonstrates DirectDraw animation using overlays.

User's Guide
============
  To run the Mosquito application, you must have a display adapter that 
  supports overlays. On a computer with overlay support, the program creates a 
  large mosquito that flies around the screen. 

  Some cards have better overlay support in certain resolutions that others. If 
  you know your card has overlay support through DirectDraw, but the Mosquito 
  program is having problems creating or displaying the overlay, try switching 
  to a lower screen resolution or color depth and restarting the application.

  This app will also hog the applications keyboard focus. You will not be able
  to switch out of the app until you exit by pressing ESC or F12.

Programming Notes
=================
  The program creates a complex overlay surface and animates by flipping.

