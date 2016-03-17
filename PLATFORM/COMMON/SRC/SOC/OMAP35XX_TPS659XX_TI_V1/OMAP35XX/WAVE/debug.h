//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  Debugging definitions.
//

#ifndef __DEBUG_H__
#define __DEBUG_H__

//#ifdef DEBUGMSG
//#undef DEBUGMSG
//#define DEBUGMSG(x,y) RETAILMSG(1,y)
//#endif

#ifndef SHIP_BUILD

EXTERN_C  DBGPARAM    dpCurSettings;

#define ZONE_HWBRIDGE   DEBUGZONE(0)
#define ZONE_IOCTL      DEBUGZONE(1)
#define ZONE_VERBOSE    DEBUGZONE(2)
#define ZONE_IRQ        DEBUGZONE(3)
#define ZONE_WODM       DEBUGZONE(4)
#define ZONE_WIDM       DEBUGZONE(5)
#define ZONE_PDD        DEBUGZONE(6)
#define ZONE_MDD        DEBUGZONE(7)
#define ZONE_MIXER      DEBUGZONE(8)
#define ZONE_RIL        DEBUGZONE(9)
#define ZONE_UNUSED     DEBUGZONE(10)
#define ZONE_MODEM      DEBUGZONE(11)
#define ZONE_PM         DEBUGZONE(12)
#define ZONE_FUNCTION   DEBUGZONE(13)
#define ZONE_WARN       DEBUGZONE(14)
#ifndef ZONE_ERROR
#define ZONE_ERROR      DEBUGZONE(15)
#endif
#endif // !SHIP_BUILD


#endif //__DEBUG_H__

