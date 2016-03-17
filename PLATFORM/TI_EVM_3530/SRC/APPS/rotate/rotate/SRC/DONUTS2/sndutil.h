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
/*==========================================================================
 *
 *
 *  File:       sndutil.h
 *  Content:    Routines for dealing with sounds from resources
 *
 *
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//#############################################################################
//
// Data types
//
//#############################################################################

///////////////////////////////////////////////////////////////////////////////
//
// HSNDOBJ             Handle to a SNDOBJ object.
//
//  SNDOBJs are implemented in sndutil as an example layer built on top
//      of the waveform API.
//
//      A SNDOBJ is generally used to manage individual
//      sounds which need to be played multiple times concurrently.  A
//      SNDOBJ represents a set of wave streams and wave headers which
//      all refer to the same buffer memory.
//
///////////////////////////////////////////////////////////////////////////////
typedef struct _tag_SNDDEVICE   SNDDEVICE,      * HSNDDEVICE;
typedef struct _tag_SNDOBJ      SNDOBJ,         * HSNDOBJ;

//#############################################################################
//
// Data methods
//
//#############################################################################

///////////////////////////////////////////////////////////////////////////////
//
// SndObjCreate     Loads a SNDOBJ from a Win32 resource in
//		    the current application.
//
// Params:
//  pDevice     -- Sound Device object that will be used to create the SNDOBJ.
//
//  lpName      -- Name of WAV resource to load the data from.  Can be a
//                 resource id specified using the MAKEINTRESOURCE macro.
//
//  iConcurrent -- Integer representing the number of concurrent playbacks of
//                 to plan for.  Attempts to play more than this number will
//                 succeed but will restart the least recently played buffer
//                 even if it is not finished playing yet.
//
//  hInstance   -- Handle to the module to load from
//  fLooping    -- True if buffer should play continuously until stopped
//
// Returns an HSNDOBJ or NULL on error.
//
///////////////////////////////////////////////////////////////////////////////
HSNDOBJ SndObjCreate(HSNDDEVICE pDevice, 
                     HINSTANCE hInstance,
                     LPCTSTR lpName,
                     BOOL fLooping,
                     int iConcurrent
                     );

///////////////////////////////////////////////////////////////////////////////
//
// SndObjDestroy  Frees a SNDOBJ and releases all of its buffers.
//
// Params:
//  hSO         -- Handle to a SNDOBJ to free.
//
///////////////////////////////////////////////////////////////////////////////
void SndObjDestroy(HSNDOBJ hSO);

///////////////////////////////////////////////////////////////////////////////
//
// SndObjPlay	Plays a buffer in a SNDOBJ.
//
// Params:
//  hSO         -- Handle to a SNDOBJ to play a buffer from.
//
///////////////////////////////////////////////////////////////////////////////
BOOL SndObjPlay(HSNDOBJ hSO);

///////////////////////////////////////////////////////////////////////////////
//
// SndObjSetRate    Sets the relative playback rate for a sound object
//
// Params:
//  hSO         -- Handle to a SNDOBJ to play a buffer from.
//
//  dRate       -- Playback rate scale factor.
//                 1.0 - play at nominal rate
//                 0.5 - play at half speed
//                 1.1 0 play 10% above normal speed
///////////////////////////////////////////////////////////////////////////////
BOOL SndObjSetRate(HSNDOBJ hSO, DOUBLE dRate);


BOOL SndObjSetPan(HSNDOBJ pSndObject, DOUBLE dPan);


///////////////////////////////////////////////////////////////////////////////
//
// SndObjPlay	Plays a buffer in a SNDOBJ.
//
// Params:
//  hSO         -- Handle to a SNDOBJ to play a buffer from.
//
//  lPan        -- signed integer between [-10000,+10000], indicating pan left/right
///////////////////////////////////////////////////////////////////////////////
BOOL SndObjPlayPanned(HSNDOBJ hSO, LONG lPan);

///////////////////////////////////////////////////////////////////////////////
//
// SndObjStop	Stops one or more buffers in a SNDOBJ.
//
// Params:
//  hSO         -- Handle to a SNDOBJ to play a buffer from.
//
///////////////////////////////////////////////////////////////////////////////
BOOL SndObjStop(HSNDOBJ hSO);


///////////////////////////////////////////////////////////////////////////////
//
// SndObjGetDuratin	Retrieves the duration, in milliseconds, of the play time of SNDOBJ.
//
// Params:
//  hSO         -- Handle to a SNDOBJ to query
//
///////////////////////////////////////////////////////////////////////////////
DWORD SndObjGetDuration (HSNDOBJ hSO);

///////////////////////////////////////////////////////////////////////////////
//
// Sound Device Methods
//
///////////////////////////////////////////////////////////////////////////////


HSNDDEVICE SndCreateSndDevice(ULONG ulDeviceIndex);
void SndObjCloseDevice(HSNDDEVICE);




#ifdef __cplusplus
}
#endif

