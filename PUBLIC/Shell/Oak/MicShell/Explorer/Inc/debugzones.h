//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//

// Debug Zones
#ifdef DEBUG  
  #define ZONE_FATAL      DEBUGZONE(0x00)
  #define ZONE_WARNING    DEBUGZONE(0x01)
  #define ZONE_VERBOSE    DEBUGZONE(0x02)
  #define ZONE_INFO       DEBUGZONE(0x03)
#else
  #define ZONE_FATAL      0
  #define ZONE_WARNING    0
  #define ZONE_VERBOSE    0
  #define ZONE_INFO       0
#endif // DEBUG
