//-----------------------------------------------------------------------------
// 
// Sample Name: DDEx1 Sample
// 
// Copyright (c) 1998-2000 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------

Description
===========
  DDEx1 demonstrates the tasks required to initialize and run a DirectDraw 
  application.

User's Guide
============
  DDEx1 requires no user input. Press the F12 key or the ESC key to quit the 
  program.

Programming Notes
=================
  This program shows how to initialize DirectDraw and create a DirectDraw 
  surface. It creates a back surface and uses page flipping to alternately 
  display the contents of the front and back surfaces. Other techniques 
  demonstrated include color fills and using GDI functions on a DirectDraw 
  surface.

  Note: This sample requires hardware flipping support.  If the display
  driver doesn't support page flipping the sample will exit during
  initialization.
