//-----------------------------------------------------------------------------
// 
// Sample Name: DDEx2 Sample
// 
// Copyright (c) 1998-2000 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  The DDEx2 program is an extension of DDEx1 that adds a bitmap.

User's Guide
============
  DDEx2 requires no user input. Press F12 or ESC to quit the program.

Programming Notes
=================
  Routines in DDutil.cpp load a bitmap file and copy it to a DirectDraw 
  surface. 

  Note: This sample requires hardware flipping support.  If the display
  driver doesn't support page flipping the sample will exit during
  initialization.

