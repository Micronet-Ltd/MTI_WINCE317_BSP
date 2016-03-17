// All rights reserved ADENEO EMBEDDED 2010
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

#ifndef __DISPLAY_DRVESC_H__
#define __DISPLAY_DRVESC_H__


//------------------------------------------------------------------------------
//  Defines/Enums
//
//  Note - Private Display Driver DRVESC values may begin with value 100000
//         according to MSFT documentation


//-----------------------------------------------------------------------------
//
//  Define: DRVESC_TVOUT_ENABLE
//          DRVESC_TVOUT_DISABLE
//
//  Enable/disable TV out display
//

#define DRVESC_TVOUT_ENABLE         100000
#define DRVESC_TVOUT_DISABLE        100001


//-----------------------------------------------------------------------------
//
//  Define: DRVESC_TVOUT_GETSETTINGS
//          DRVESC_TVOUT_SETSETTINGS
//
//  Gets/Sets TV out settings using the TV out settings structure
//

#define DRVESC_TVOUT_GETSETTINGS    100002
#define DRVESC_TVOUT_SETSETTINGS    100003

#define TVOUT_SETTINGS_MIN_FILTER   0
#define TVOUT_SETTINGS_MAX_FILTER   0   // Only scaling supported. No Flicker filtering.

#define TVOUT_SETTINGS_MIN_ASPECT   1
#define TVOUT_SETTINGS_MAX_ASPECT   20
#define TVOUT_SETTINGS_DIFF_ASPECT  8

#define TVOUT_SETTINGS_MIN_RESIZE   80
#define TVOUT_SETTINGS_MAX_RESIZE   100

typedef struct {
    BOOL    bEnable;
    DWORD   dwFilterLevel;
    DWORD   dwAspectRatioW;    
    DWORD   dwAspectRatioH;    
    DWORD   dwResizePercentW;    
    DWORD   dwResizePercentH;    
    LONG    lOffsetW;    
    LONG    lOffsetH;    
} DRVESC_TVOUT_SETTINGS;

//-----------------------------------------------------------------------------
//
//  Define: DRVESC_DVI_ENABLE
//          DRVESC_DVI_DISABLE
//
//  Enable/disable DVI out
//

#define DRVESC_DVI_ENABLE         100004
#define DRVESC_DVI_DISABLE        100005
//-----------------------------------------------------------------------------
//
//  Define: DRVESC_GAPI_ENABLE
//          DRVESC_GAPI_DISABLE
//          DRVESC_GAPI_DRAMTOVRAM
//          DRVESC_GAPI_VRAMTODRAM
//
//  Game API support IOCTLs
//

#define DRVESC_GAPI_ENABLE          100010
#define DRVESC_GAPI_DISABLE         100011
#define DRVESC_GAPI_DRAMTOVRAM      100012
#define DRVESC_GAPI_VRAMTODRAM      100013


//-----------------------------------------------------------------------------
//
//  Define: DRVESC_HDMI_ENABLE
//          DRVESC_HDMI_DISABLE
//
//  Game API support IOCTLs
//

#define DRVESC_HDMI_ENABLE          100020
#define DRVESC_HDMI_DISABLE         100021



#endif //__DISPLAY_DRVESC_H__

