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

#ifndef __SENSORPROPERTIES_H
#define __SENSORPROPERTIES_H

// ------------------------------------------------------------------------
static CSPROPERTY_STEPPING_LONG BrightnessRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        -10000,             // Minimum in (IRE * 100) units
        10000               // Maximum in (IRE * 100) units
    }
};

const static LONG BrightnessDefault = 750;

static CSPROPERTY_MEMBERSLIST BrightnessMembersList [] = 
{
    {
        /*CSPROPERTY_MEMBERSHEADER*/
        {    
            CSPROPERTY_MEMBER_RANGES,                /*MembersFlags*/
            sizeof (CSPROPERTY_STEPPING_LONG),       /*MembersSize*/
            _countof (BrightnessRangeAndStep),   /*MembersCount*/
            0                                        /*flags 0 or CSPROPERTY_MEMBER_FLAG_DEFAULT*/
        },
        /*Members*/
        (PVOID) BrightnessRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (BrightnessDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &BrightnessDefault,
    }    
};

static CSPROPERTY_VALUES BrightnessValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    _countof (BrightnessMembersList),
    BrightnessMembersList
};

// ------------------------------------------------------------------------
// The contrast value is expressed as a gain factor multiplied by 100. 
static CSPROPERTY_STEPPING_LONG ContrastRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        0,                  // Minimum as a gain factor multiplied by 100
        10000               // Maximum as a gain factor multiplied by 100
    }
};

const static LONG ContrastDefault = 100;

static CSPROPERTY_MEMBERSLIST ContrastMembersList [] = 
{
    {
        /*CSPROPERTY_MEMBERSHEADER*/
        {    
            CSPROPERTY_MEMBER_RANGES,                /*MembersFlags*/
            sizeof (CSPROPERTY_STEPPING_LONG),       /*MembersSize*/
            _countof (ContrastRangeAndStep),     /*MembersCount*/
            0                                        /*flags 0 or CSPROPERTY_MEMBER_FLAG_DEFAULT*/
        },
        /*Members*/
        (PVOID) ContrastRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (ContrastDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &ContrastDefault,
    }    
};

static CSPROPERTY_VALUES ContrastValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    _countof (ContrastMembersList),
    ContrastMembersList
};

// ------------------------------------------------------------------------

DEFINE_CSPROPERTY_TABLE(VideoProcAmpProperties)
{
    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_VIDEOPROCAMP_BRIGHTNESS,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &BrightnessValues,                      // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),

    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_VIDEOPROCAMP_CONTRAST,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &ContrastValues,                        // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    )
};

// ------------------------------------------------------------------------
// Array of all of the property sets supported by the adapter
// ------------------------------------------------------------------------

DEFINE_CSPROPERTY_SET_TABLE(AdapterPropertyTable)
{
    DEFINE_CSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_VIDEOPROCAMP,
        _countof(VideoProcAmpProperties),
        VideoProcAmpProperties,
        0, 
        NULL
    )
};

#define NUMBER_OF_ADAPTER_PROPERTY_SETS (_countof (AdapterPropertyTable))

// ----------------------------------------------------------------------------
// Default Video Control Caps 
// ----------------------------------------------------------------------------
ULONG DefaultVideoControlCaps[] = {
    0x0                               /*CAPTURE*/,
    CS_VideoControlFlag_ExternalTriggerEnable | CS_VideoControlFlag_Trigger | CS_VideoControlFlag_Sample_Scanned_Notification /*STILL*/,
    0x0                              /*PREVIEW*/
    };
#if 0
// ----------------------------------------------------------------------------
// Metadata
// ----------------------------------------------------------------------------
const CS_PROPERTYITEM Metadata[] =
{
    {CS_PROPID_MAKE, 32, CS_PROPITEM_TYPE_ASCII, (ULONG)"Placeholder for 'Make' tag data"},
    {CS_PROPID_MODEL, 33, CS_PROPITEM_TYPE_ASCII, (ULONG)"Placeholder for 'Model' tag data"},
    {CS_PROPID_MAKER_NOTE, 36, CS_PROPITEM_TYPE_UNDEFINED, (ULONG)"Placeholder for 'MakerNote' tag data"},
    {CS_PROPID_SENSING_METHOD, sizeof(SHORT), CS_PROPITEM_TYPE_SHORT, (ULONG)"\x01\x00"},
    {CS_PROPID_CFA_PATTERN, 37, CS_PROPITEM_TYPE_UNDEFINED, (ULONG)"Placeholder for 'CFAPattern' tag data"}
};

#define NUMBER_OF_PROPERTYITEMS (_countof (Metadata))
#endif
#endif