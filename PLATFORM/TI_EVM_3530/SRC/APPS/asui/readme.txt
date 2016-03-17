/**
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

Copyright (c) 1995-2001 Microsoft Corporation.  All rights reserved.

Abstract:
	Windows CE Bluetooth application sample

**/
This is a sample ActiveSync driver application which illustrates the use
of the client-side Windows CE Bluetooth port emulator, device discovery
and client-side SDP queries.

Note, that as a sample this application was never intended for production use.
It has not been exhaustively tested and may contain deficiencies that must
be resolved for an end user product.

This sample compiles to an application PHONEUI.EXE.

When started, ASUI discovers if a phone has been selected, and if it was,
creates a COM port associated with its parameters. It then creates an icon
in the shell notification area.

When the icon is pressed, ASUI presents its main dialog box that lets
user discover devices and then select device to set as an ActiveSync partner,
and sync to that device.

In process it runs inquiry, name queries and SDP queries.

