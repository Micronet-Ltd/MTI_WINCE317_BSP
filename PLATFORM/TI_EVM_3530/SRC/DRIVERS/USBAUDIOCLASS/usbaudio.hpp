//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft
// premium shared source license agreement under which you licensed
// this source code. If you did not accept the terms of the license
// agreement, you are not authorized to use this source code.
// For the terms of the license, please see the license agreement
// signed by you and Microsoft.
// THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//
// Module Name:
//      usbaudio.hpp
//
// Abstract:
//      USB Client Driver for Audio Device Class.
//

#pragma once

#include <windows.h>
#include <devload.h>
#include <usbdi.h>
#include <ntcompat.h>
#include "usbclient.h"
#include "debug.h"

#define OUTPUT_SAMPLERATE  (48000)
#define INPUT_SAMPLERATE   (48000)
#define AUDIO_FRAME_SIZE   192

#define DRIVER_NAME   TEXT("USBAUDIO.DLL")
#define DEVICE_PREFIX TEXT("WAV")

#define CLASS_NAME_SZ    TEXT("Audio_Class")
#define CLIENT_REGKEY_SZ TEXT("Drivers\\USB\\ClientDrivers\\Audio_Class")
#define LOAD_REGKEY_SZ TEXT("Drivers\\USB\\LoadClients\\Default\\Default\\1\\Audio_Class")
#define ICLASS_NAME_SZ   TEXT("IClass")

//
// USB Audio Interface Descriptor
//
#define USB_AUD_INTERFACE_CLASS                     0x01
#define USB_AUD_CONTROL_INTERFACE_SUBCLASS          0x01
#define USB_AUD_STREAMING_INTERFACE_SUBCLASS        0x02
#define USB_AUD_MIDISTREAMING_INTERFACE_SUBCLASS    0x03

//
// Default Timeout values
//
//#define GET_PORT_STATUS_TIMEOUT     2000
//#define GET_DEVICE_ID_TIMEOUT       2000
//#define SOFT_RESET_TIMEOUT          2000
//
//#define READ_TIMEOUT_INTERVAL       250
//#define READ_TIMEOUT_MULTIPLIER     10
//#define READ_TIMEOUT_CONSTANT       100
//#define WRITE_TIMEOUT_MULTIPLIER    50
//#define WRITE_TIMEOUT_CONSTANT      1000

//
// registry strings
//
//#define GET_PORT_STATUS_TIMEOUT_SZ      TEXT("PortStatusTimeout")
//#define GET_DEVICE_ID_TIMEOUT_SZ        TEXT("DeviceIdTimeout")
//#define SOFT_RESET_TIMEOUT_SZ           TEXT("SoftResetTimeout")
//
//#define READ_TIMEOUT_MULTIPLIER_SZ      TEXT("ReadTimeoutMultiplier")
//#define READ_TIMEOUT_CONSTANT_SZ        TEXT("ReadTimeoutConstant")
//#define WRITE_TIMEOUT_MULTIPLIER_SZ     TEXT("WriteTimeoutMultiplier")
//#define WRITE_TIMEOUT_CONSTANT_SZ       TEXT("WriteTimeoutConstant")

//
// USB_DRIVER_SETTINGS
//
#define DRIVER_SETTINGS \
            sizeof(USB_DRIVER_SETTINGS),  \
            USB_NO_INFO,   \
            USB_NO_INFO,   \
            USB_NO_INFO,   \
            USB_NO_INFO,   \
            USB_NO_INFO,   \
            USB_NO_INFO,   \
            USB_AUD_INTERFACE_CLASS,      \
            USB_AUD_STREAMING_INTERFACE_SUBCLASS,   \
            USB_NO_INFO

//
// Our notion of a Pipe
//
typedef struct _PIPE 
{    
    USB_PIPE hPipe;             // USB Pipe handle received from the stack
    UCHAR    bIndex;            // Endpoint's Address
    USHORT   wMaxPacketSize;    // Endpoint's wMaxPacketSize
    HANDLE   hEvent;            // Completion Event
} PIPE, *PPIPE;

//
// USB Audio Context
//
#define USB_AUD_SIG         0x41425355      // "USBA" tag
#define MANUAL_RESET_EVENT  TRUE
#define AUTO_RESET_EVENT    FALSE

typedef struct _USBAUD_CONTEXT 
{
    ULONG Sig;                          // We use a Signature (defined above) since we must read
                                        // our context pointer out of the registry
    CRITICAL_SECTION Lock;              // sync object for this instance
    LPTSTR ActivePath;                  // path to HKLM\Drivers\Active\xx
    HANDLE hStreamDevice;               // Handle for Stream interface.
    HANDLE hUsbDevice;                  // USB handle to the device
    LPCUSB_FUNCS   UsbFuncs;            // USBD Function table
    UCHAR bInterfaceNumber;             // Fields from USB_INTERFACE that we need (audio output)
    UCHAR bAlternateSetting;
    PIPE IsoOut;                        // Iso OUT Pipe
    PIPE IsoIn;                         // Iso IN Pipe
    HANDLE hEP0Event;                   // completion signal used for endpoint 0
    HANDLE hCloseEvent;                 // Signals the device has been closed.    
    HANDLE htDevice;                    // Device Thread Handle
} USBAUD_CONTEXT, *PUSBAUD_CONTEXT;

// Is the pointer a valid PUSBPRN_CONTEXT
#define VALID_CONTEXT( p ) \
   ( p && USB_AUD_SIG == p->Sig )

// USB Audio Class Structures
#define CS_UNDEFINED                    0x20
#define CS_DEVICE                       0x21
#define CS_CONFIGURATION                0x22
#define CS_STRING                       0x23
#define CS_INTERFACE                    0x24
#define CS_ENDPOINT                     0x25

#define AS_DESCRIPTOR_UNDEFINED         0x00
#define AS_GENERAL                      0x01
#define FORMAT_TYPE                     0x02
#define FORMAT_SPECIFIC                 0x03

#pragma pack(1)

typedef struct _USB_AUDIO_AS_INTERFACE_DESCRIPTOR 
{
    const BYTE bLength;                             // Size of this descriptor in bytes: 7
    const BYTE bDescriptorType;                     // CS_INTERFACE descriptor type.
    const BYTE bDescriptorSubType;                  // AS_GENERAL descriptor subtype.
    const BYTE bTerminalLink;                       // The Terminal ID of the Terminal to which the endpoint of this interface is connected.
    const BYTE bDelay;                              // Delay introduced by the data path.
    const WORD wFormatTag;                          // The Audio Data Format that has to be used to communicate with this interface.
} USB_AUDIO_AS_INTERFACE_DESCRIPTOR, *PUSB_AUDIO_AS_INTERFACE_DESCRIPTOR, *LPUSB_AUDIO_AS_INTERFACE_DESCRIPTOR;


typedef struct _USB_AUDIO_AS_TYPE1_FORMAT_DESCRIPTOR 
{
    const BYTE bLength;                             // Size of this descriptor, in bytes: 8+(ns*3)
    const BYTE bDescriptorType;                     // CS_INTERFACE descriptor type.
    const BYTE bDescriptorSubType;                  // FORMAT_TYPE descriptor subtype.
    const BYTE bFormatType;                         // FORMAT_TYPE_I. Constant identifying the Format Type the AudioStreaming interface is using.
    const BYTE bNumChannels;                        // Indicates the number of physical channels in the audio data stream.
    const BYTE bSubFrameSize;                       // The number of bytes occupied by one audio subframe. Can be 1, 2, 3 or 4.
    const BYTE bBitResolution;                      // The number of effectively used bits from the available bits in an audio subframe.
    const BYTE bSamFreqType;                        // Indicates how the sampling frequency can be programmed
                                                    //      0:      Continuous sampling frequency
                                                    // 1..255:      The number of discrete sampling frequencies supported by the
                                                    //              isochronous data endpoint of the AudioStreaming interface (ns)
    PBYTE tSamFreq;                                 // Frequency Table (see P. 10 in USB Audio Data Formats Spec)
} USB_AUDIO_AS_TYPE1_FORMAT_DESCRIPTOR, *PUSB_AUDIO_AS_TYPE1_FORMAT_DESCRIPTOR, *LPUSB_AUDIO_AS_TYPE1_FORMAT_DESCRIPTOR;

// Class-Specific AC Interface Descriptor
//
// The class-specific AC interface descriptor is a concatenation of all the descriptors that are used to fully
// describe the audio function, i.e. all Unit Descriptors (UDs) and Terminal Descriptors (TDs).
#define AC_DESCRIPTOR_UNDEFINED             0x00
#define CSI_HEADER                          0x01
#define INPUT_TERMINAL                      0x02
#define OUTPUT_TERMINAL                     0x03
#define MIXER_UNIT                          0x04
#define SELECTOR_UNIT                       0x05
#define FEATURE_UNIT                        0x06
#define PROCESSING_UNIT                     0x07
#define EXTENSION_UNIT                      0x08

#define PRINT_UNIT_DESCRIPTOR( d )     { WCHAR descr[9][32] = { \
                                            TEXT("AC_DESCRIPTOR_UNDEFINED"), \
                                            TEXT("CSI_HEADER"), \
                                            TEXT("INPUT_TERMINAL"), \
                                            TEXT("OUTPUT_TERMINAL"), \
                                            TEXT("MIXER_UNIT"), \
                                            TEXT("SELECTOR_UNIT"), \
                                            TEXT("FEATURE_UNIT"), \
                                            TEXT("PROCESSING_UNIT"), \
                                            TEXT("EXTENSION_UNIT") }; \
                                        DEBUGMSG( ZONE_USB_PARSE, ((0 <= d && d <= 9) ? descr[d] : TEXT("UNKNOWN_UNIT"))); }

// USB Terminal Types
#define TERMINAL_UNDEFINED                  0x0100
#define TERMINAL_STREAMING                  0x0101
#define TERMINAL_VENDOR_SPECIFIC            0x01FF

// Input Terminal Types
#define IT_UNDEFINED                        0x0200
#define IT_MICROPHONE                       0x0201
#define IT_DESKTOP_MICROPHONE               0x0202
#define IT_PERSONAL_MICROPHONE              0x0203
#define IT_OMNIDIRECTIONAL_MICROPHONE       0x0204
#define IT_MICROPHONE_ARRAY                 0x0205
#define IT_PROCESSING_MICROPHONE_ARRAY      0x0206

// Output Terminal Types
#define OT_UNDEFINED                        0x0300
#define OT_SPEAKER                          0x0301
#define OT_HEADPHONES                       0x0302
#define OT_HEAD_MOUNTET_DISPLAY_AUDIO       0x0303
#define OT_DESKTOP_SPEAKER                  0x0304
#define OT_ROOM_SPEAKER                     0x0305
#define OT_COMMUNICAITON_SPEAKER            0x0306
#define OT_LOW_FREQUENCY_EFFECTS_SPEAKER    0x0307

// Bi-directional Terminal Types
#define BI_UNDEFINED                        0x0400
#define BI_HANDSET                          0x0401
#define BI_HEADSET                          0x0402
#define BI_SPEAKERPHONE_NO_ECHO_REDUCTION   0x0403
#define BI_SPEAKERPHONE_ECHO_SUPPRESSING    0x0404
#define BI_SPEAKERPHONE_ECHO_CANCELLING     0x0405

// Telephony Terminal Types
#define TEL_UNDEFINED                       0x0500
#define TEL_PHONE_LINE                      0x0501
#define TEL_TELEPHONE                       0x0502
#define TEL_DOWN_LINE_PHONE                 0x0503

// External Terminal Types
#define EXT_UNDEFINED                       0x0600
#define EXT_ANALOG_CONNECTOR                0x0601
#define EXT_DIGITAL_AUDIO_INTERFACE         0x0602
#define EXT_LINE_CONNECTOR                  0x0603
#define EXT_LEGACY_AUDIO_CONNECTOR          0x0604
#define EXT_SPDIF_INTERFACE                 0x0605
#define EXT_1394_DA_STREAM                  0x0606
#define EXT_1394_DV_STREAM_SOUNDTRACK       0x0607

// Embedded Terminal Types
#define EMB_UNDEFINED                       0x0700
#define EMB_LEVEL_CALIBRATION_NOISE         0x0701
#define EMB_EQUALIZATION_NOISE              0x0702
#define EMB_CD_PLAYER                       0x0703
#define EMB_DAT                             0x0704
#define EMB_DCC                             0x0705
#define EMB_MINIDISK                        0x0706
#define EMB_ANALOG_TAPE                     0x0707
#define EMB_PHONOGRAPH                      0x0708
#define EMB_VCR_AUDIO                       0x0709
#define EMB_VIDEO_DISC_AUDIO                0x070A
#define EMB_DVD_AUDIO                       0x070B
#define EMB_TV_TUNER_AUDIO                  0x070C 
#define EMB_SATELLITE_RECEIVER_AUDIO        0x070D
#define EMB_CABLE_TUNER_AUDIO               0x070E
#define EMB_DSS_AUDIO                       0x070F
#define EMB_RADIO_RECEIVER                  0x0710
#define EMB_RADIO_TRANSMITTER               0x0711
#define EMB_MULTITRACK_RECORDER             0x0712
#define EMB_SYNTHESIZER                     0x0713

#define PRINT_TERMINAL_TYPE( d )     { WCHAR descr[][64] = { \
                                            TEXT("TERMINAL_UNDEFINED"), \
                                            TEXT("TERMINAL_STREAMING"), \
                                            TEXT("TERMINAL_VENDOR_SPECIFIC"), \
                                            TEXT("IT_UNDEFINED"), \
                                            TEXT("IT_MICROPHONE"), \
                                            TEXT("IT_DESKTOP_MICROPHONE"), \
                                            TEXT("IT_PERSONAL_MICROPHONE"), \
                                            TEXT("IT_OMNIDIRECTIONAL_MICROPHONE"), \
                                            TEXT("IT_MICROPHONE_ARRAY"), \
                                            TEXT("IT_PROCESSING_MICROPHONE_ARRAY"), \
                                            TEXT("OT_UNDEFINED"), \
                                            TEXT("OT_SPEAKER"), \
                                            TEXT("OT_HEADPHONES"), \
                                            TEXT("OT_HEAD_MOUNTET_DISPLAY_AUDIO"), \
                                            TEXT("OT_DESKTOP_SPEAKER"), \
                                            TEXT("OT_ROOM_SPEAKER"), \
                                            TEXT("OT_COMMUNICAITON_SPEAKER"), \
                                            TEXT("OT_LOW_FREQUENCY_EFFECTS_SPEAKER"), \
                                            TEXT("BI_UNDEFINED"), \
                                            TEXT("BI_HANDSET"), \
                                            TEXT("BI_HEADSET"), \
                                            TEXT("BI_SPEAKERPHONE_NO_ECHO_REDUCTION"), \
                                            TEXT("BI_SPEAKERPHONE_ECHO_SUPPRESSING"), \
                                            TEXT("BI_SPEAKERPHONE_ECHO_CANCELLING"), \
                                            TEXT("TEL_UNDEFINED"), \
                                            TEXT("TEL_PHONE_LINE"), \
                                            TEXT("TEL_TELEPHONE"), \
                                            TEXT("TEL_DOWN_LINE_PHONE"), \
                                            TEXT("EXT_UNDEFINED"), \
                                            TEXT("EXT_ANALOG_CONNECTOR"), \
                                            TEXT("EXT_DIGITAL_AUDIO_INTERFACE"), \
                                            TEXT("EXT_LINE_CONNECTOR"), \
                                            TEXT("EXT_LEGACY_AUDIO_CONNECTOR"), \
                                            TEXT("EXT_SPDIF_INTERFACE"), \
                                            TEXT("EXT_1394_DA_STREAM"), \
                                            TEXT("EXT_1394_DV_STREAM_SOUNDTRACK"), \
                                            TEXT("EMB_UNDEFINED"), \
                                            TEXT("EMB_LEVEL_CALIBRATION_NOISE"), \
                                            TEXT("EMB_EQUALIZATION_NOISE"), \
                                            TEXT("EMB_CD_PLAYER"), \
                                            TEXT("EMB_DAT"), \
                                            TEXT("EMB_DCC"), \
                                            TEXT("EMB_MINIDISK"), \
                                            TEXT("EMB_ANALOG_TAPE"), \
                                            TEXT("EMB_PHONOGRAPH"), \
                                            TEXT("EMB_VCR_AUDIO"), \
                                            TEXT("EMB_VIDEO_DISC_AUDIO"), \
                                            TEXT("EMB_DVD_AUDIO"), \
                                            TEXT("EMB_TV_TUNER_AUDIO"), \
                                            TEXT("EMB_SATELLITE_RECEIVER_AUDIO"), \
                                            TEXT("EMB_CABLE_TUNER_AUDIO"), \
                                            TEXT("EMB_DSS_AUDIO"), \
                                            TEXT("EMB_RADIO_RECEIVER"), \
                                            TEXT("EMB_RADIO_TRANSMITTER"), \
                                            TEXT("EMB_MULTITRACK_RECORDER"), \
                                            TEXT("EMB_SYNTHESIZER") }; \
                                         int shifts[] = {0, 0, 3, 10, 18, 24, 28, 36 }; \
                                         DEBUGMSG( ZONE_USB_PARSE, (descr[(d & 0xFF) + shifts[(d >> 8) & 0xFF]])); }


// Class-Specific AC Interface Header Descriptor (P. 37)
typedef struct _USB_AUDIO_CSI_DESCRIPTOR 
{
    const BYTE bLength;                     // Size of this descriptor, in bytes: 8+n
    const BYTE bDescriptorType;             // CS_INTERFACE descriptor type.
    const BYTE bDescriptorSubType;          // CSI_HEADER descriptor subtype.
    const WORD bcdADC;                      // Audio Device Class Specification Release Number in Binary-Coded Decimal.
    const WORD wTotalLength;                // Total number of bytes returned for the class-specific AudioControl interface
                                            //descriptor. Includes the combined length of this descriptor header and
                                            //all Unit andTerminal descriptors.
    const BYTE bInCollection;               // The number of AudioStreaming and MIDIStreaming interfaces in the Audio
                                            // Interface Collection to which this AudioControl interface belongs: n

    PBYTE baInterfaceNr;                    //baInterfaceNr(1) ... baInterfaceNr(n)
                                            // Interface numbers of AudioStreaming or MIDIStreaming interfaces in the Collection.

} USB_AUDIO_CSI_DESCRIPTOR , *PUSB_AUDIO_CSI_DESCRIPTOR , *LPUSB_AUDIO_CSI_DESCRIPTOR ;

// Input Terminal Descriptor (P. 38)
typedef struct _USB_AUDIO_ITD_DESCRIPTOR 
{
    const BYTE bLength;                     // Size of this descriptor, in bytes: 12
    const BYTE bDescriptorType;             // CS_INTERFACE descriptor type.
    const BYTE bDescriptorSubType;          // INPUT_TERMINAL descriptor subtype.
    const BYTE bTerminalID;                 // Constant uniquely identifying the Terminal within the audio function.
                                            // This value is used in all requests to address this Terminal.
    const WORD wTerminalType;               // Constant characterizing the type of Terminal.
    const BYTE bAssocTerminal;              // ID of the Output Terminal to which this Input Terminal is associated.
    const BYTE bNumChannels;                // Number of logical output channels in the Terminal’s output audio channel cluster.
    const WORD wChannelConfig;              // Describes the spatial location of the logical channels.
    const BYTE iChannelNames;               // Index of a string descriptor, describing the name of the first logical channel.
    const BYTE iTerminal;                   // Index of a string descriptor, describing the Input Terminal.
} USB_AUDIO_ITD_DESCRIPTOR , *PUSB_AUDIO_ITD_DESCRIPTOR , *LPUSB_AUDIO_ITD_DESCRIPTOR ;

// Output Terminal Descriptor (P. 39)
typedef struct _USB_AUDIO_OTD_DESCRIPTOR 
{
    const BYTE bLength;                     // Size of this descriptor, in bytes: 9
    const BYTE bDescriptorType;             // CS_INTERFACE descriptor type.
    const BYTE bDescriptorSubType;          // OUTPUT_TERMINAL descriptor subtype.
    const BYTE bTerminalID;                 // Constant uniquely identifying the Terminal within the audio function. This
                                            // value is used in all requests to address this Terminal.
    const WORD wTerminalType;               // Constant characterizing the type of Terminal.
    const BYTE bAssocTerminal;              // Constant, identifying the Input Terminal to which this Output Terminal is associated.
    const BYTE bSourceID;                   // ID of the Unit or Terminal to which this Terminal is connected.
    const BYTE iTerminal;                   // Index of a string descriptor, describing the Input Terminal.
} USB_AUDIO_OTD_DESCRIPTOR , *PUSB_AUDIO_OTD_DESCRIPTOR , *LPUSB_AUDIO_OTD_DESCRIPTOR ;

typedef struct _USB_AUDIO_DUMMY_TD_DESCRIPTOR 
{
    const BYTE bLength;                     // Size of this descriptor, in bytes: 9
    const BYTE bDescriptorType;             // CS_INTERFACE descriptor type.
    const BYTE bDescriptorSubType;          // OUTPUT_TERMINAL descriptor subtype.
    const BYTE bTerminalID;                 // Constant uniquely identifying the Terminal within the audio function.
	const WORD wTerminalType;               // Constant characterizing the type of Terminal.
} USB_AUDIO_DUMMY_TD_DESCRIPTOR , *PUSB_AUDIO_DUMMY_TD_DESCRIPTOR , *LPUSB_AUDIO_DUMMY_TD_DESCRIPTOR ;

// Mixer Unit Descriptor (P. 41)
typedef struct _USB_AUDIO_MIXER_DESCRIPTOR 
{
    const BYTE bLength;                     // Size of this descriptor, in bytes: 10+p+N
    const BYTE bDescriptorType;             // CS_INTERFACE descriptor type.
    const BYTE bDescriptorSubType;          // MIXER_UNIT descriptor subtype.
    const BYTE bUnitID;                     // Constant uniquely identifying the Unit within the audio function.
                                                              // This value is used in all requests to address this Unit.
    const BYTE bNumInPins;                  // Number of Input Pins of this Unit: p
    PBYTE baSourceID;                       // baSourceID(1) ... baSourceID(p)
                                            // ID of the Unit or Terminal to which the first Input Pin of this Mixer Unit is connected.
    const BYTE bNumChannels;                // Number of logical output channels in the Mixer’s output audio channel cluster.
    const WORD wChannelConfig;              // Describes the spatial location of the logical channels.
    const BYTE iChannelNames;               // Index of a string descriptor, describing the name of the first logical channel.
    PBYTE bmControls;                       // bmControls(1) ... bmControls(N)
                                            // Bit map indicating which are programmable.
    const BYTE iMixer;                      // Index of a string descriptor, describing the Mixer Unit.
} USB_AUDIO_MIXER_DESCRIPTOR , *PUSB_AUDIO_MIXER_DESCRIPTOR , *LPUSB_AUDIO_MIXER_DESCRIPTOR ;

// Selector Unit Descriptor (P. 43)
typedef struct _USB_AUDIO_SELECTOR_DESCRIPTOR 
{
    const BYTE bLength;                     // Size of this descriptor, in bytes: 6+p
    const BYTE bDescriptorType;             // CS_INTERFACE descriptor type.
    const BYTE bDescriptorSubType;          // SELECTOR_UNIT descriptor subtype.
    const BYTE bUnitID;                     // Constant uniquely identifying the Unit within the audio function.
                                            // This value is used in all requests to address this Unit.
    const BYTE bNumInPins;                  // Number of Input Pins of this Unit: p
    PBYTE baSourceID;                       // baSourceID(1) ... baSourceID(p)
                                            // ID of the Unit or Terminal to which the first Input Pin of this Selector Unit is connected.
    const BYTE iSelector;                   // Index of a string descriptor, describing the Selector Unit.
} USB_AUDIO_SELECTOR_DESCRIPTOR , *PUSB_AUDIO_SELECTOR_DESCRIPTOR , *LPUSB_AUDIO_SELECTOR_DESCRIPTOR ;

// Feature Unit Descriptor (P. 43)
typedef struct _USB_AUDIO_FEATURE_DESCRIPTOR 
{
    const BYTE bLength;                     // Size of this descriptor, in bytes: 7+(ch+1)*n
    const BYTE bDescriptorType;             // CS_INTERFACE descriptor type.
    const BYTE bDescriptorSubType;          // FEATURE_UNIT descriptor subtype.
    const BYTE bUnitID;                     // Constant uniquely identifying the Unit within the audio function.
                                            // This value is used in all requests to address this Unit.
    const BYTE bSourceID;                   // ID of the Unit or Terminal to which this Feature Unit is connected.
    const BYTE bControlSize;                // Size in bytes of an element of the bmaControls() array: n
    PBYTE bmaControls;                      // bmaControls(0) ... bmaControls(n*ch)
                                            // bmaControls(i*n)...bmaControls((i+1)*n) is a bit mask for channel i,
                                            // where channel i=0 is the master channel.
                                            // A bit set to 1 indicates that the mentioned Control is supported for logical channel i.
                                            // Available controls are:
                                            //  D0: Mute
                                            //  D1: Volume
                                            //  D2: Bass
                                            //  D3: Mid
                                            //  D4: Treble
                                            //  D5: Graphic Equalizer
                                            //  D6: Automatic Gain
                                            //  D7: Delay
                                            //  D8: Bass Boost
                                            //  D9: Loudness
                                            //  D10..(n*8-1): Reserved
    const BYTE iFeature;                    // Index of a string descriptor, describing the Feature Unit.
} USB_AUDIO_FEATURE_DESCRIPTOR , *PUSB_AUDIO_FEATURE_DESCRIPTOR, *LPUSB_AUDIO_FEATURE_DESCRIPTOR ;

// Dummy Unit Descriptor
typedef struct _USB_AUDIO_DUMMY_DESCRIPTOR 
{
    const BYTE bLength;                         // Size of this descriptor, in bytes
    const BYTE bDescriptorType;                 // CS_INTERFACE descriptor type.
    const BYTE bDescriptorSubType;              // *_UNIT descriptor subtype.
    const BYTE bUnitID;                         // Constant uniquely identifying the Unit within the audio function.
                                                // This value is used in all requests to address this Unit.
} USB_AUDIO_DUMMY_DESCRIPTOR , *PUSB_AUDIO_DUMMY_DESCRIPTOR , *LPUSB_AUDIO_DUMMY_DESCRIPTOR ;

// Associated Interfaces Descriptor (directly follows the entity descriptor to which it's associated to)
typedef struct _USB_AUDIO_ASSOC_INTERFACE_DESCRIPTOR 
{
    const BYTE bLength;                         // Size of this descriptor, in bytes: 4+x
    const BYTE bDescriptorType;                 // CS_INTERFACE descriptor type.
    const BYTE bDescriptorSubType;              // ASSOC_INTERFACE descriptor subtype.
    const BYTE bInterfaceNr;                    // The interface number of the associated interface.
    // x bytes of "Association-specific extension to the open-ended descriptor."
} USB_AUDIO_ASSOC_INTERFACE_DESCRIPTOR , *PUSB_AUDIO_ASSOC_INTERFACE_DESCRIPTOR , *LPUSB_AUDIO_ASSOC_INTERFACE_DESCRIPTOR;

//
// Audio Controls
//

// Audio Class-Specific Request Codes
#define REQUEST_CODE_UNDEFINED          0x00
#define SET_CUR                         0x01
#define GET_CUR                         0x81
#define SET_MIN                         0x02
#define GET_MIN                         0x82
#define SET_MAX                         0x03
#define GET_MAX                         0x83
#define SET_RES                         0x04
#define GET_RES                         0x84
#define SET_MEM                         0x05
#define GET_MEM                         0x85
#define GET_STAT                        0xFF

// Terminal Control Selectors
#define TE_CONTROL_UNDEFINED            0x00
#define COPY_PROTECT_CONTROL            0x01

// Feature Unit Control Selectors
#define FU_CONTROL_UNDEFINED            0x00
#define MUTE_CONTROL                    0x01
#define VOLUME_CONTROL                  0x02
#define BASS_CONTROL                    0x03
#define MID_CONTROL                     0x04
#define TREBLE_CONTROL                  0x05
#define GRAPHIC_EQUALIZER_CONTROL       0x06
#define AUTOMATIC_GAIN_CONTROL          0x07
#define DELAY_CONTROL                   0x08
#define BASS_BOOST_CONTROL              0x09
#define LOUDNESS_CONTROL                0x0A

// Endpoint Control Selectors
#define EP_CONTROL_UNDEFINED            0x00
#define SAMPLING_FREQ_CONTROL           0x01
#define PITCH_CONTROL                   0x02

//
// DEBUG
//
#ifndef SHIP_BUILD
#if DEBUG

//#ifndef ZONE_INIT
//#define ZONE_ERR                DEBUGZONE(0)
//#define ZONE_WARN               DEBUGZONE(1)
//#define ZONE_INIT               DEBUGZONE(2)
//#define ZONE_TRACE              DEBUGZONE(3)
//#endif
//
//#define ZONE_AUD_INIT           DEBUGZONE(4)
//#define ZONE_AUD_READ           DEBUGZONE(5)
//#define ZONE_AUD_WRITE          DEBUGZONE(6)
//#define ZONE_AUD_IOCTL          DEBUGZONE(7)
//
//#define ZONE_USB_PARSE          DEBUGZONE(8)
//#define ZONE_USB_INIT           DEBUGZONE(9)
//#define ZONE_USB_CONTROL        DEBUGZONE(10)
//#define ZONE_USB_ISO            DEBUGZONE(11)
//
//#define ZONE_USBCLIENT          DEBUGZONE(15)

//
// these should be removed in the code if you can 'g' past these successfully
//
#define TEST_TRAP() { \
   NKDbgPrintfW( TEXT("%s: Code Coverage Trap in: %s, Line: %d\n"), DRIVER_NAME, TEXT(__FILE__), __LINE__); \
   DebugBreak();  \
}

#define DUMP_USB_DEVICE_DESCRIPTOR( d ) { \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("USB_DEVICE_DESCRIPTOR:\n"))); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("----------------------\n"))); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bLength: 0x%x\n"), d.bLength ));   \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bDescriptorType: 0x%x\n"), d.bDescriptorType ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bcdUSB: 0x%x\n"), d.bcdUSB ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bDeviceClass: 0x%x\n"), d.bDeviceClass ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bDeviceSubClass: 0x%x\n"), d.bDeviceSubClass ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bDeviceProtocol: 0x%x\n"), d.bDeviceProtocol ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bMaxPacketSize0: 0x%x\n"), d.bMaxPacketSize0 ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("idVendor: 0x%x\n"), d.idVendor )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("idProduct: 0x%x\n"), d.idProduct ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bcdDevice: 0x%x\n"), d.bcdDevice ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("iManufacturer: 0x%x\n"), d.iManufacturer ));   \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("iProduct: 0x%x\n"), d.iProduct )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("iSerialNumber: 0x%x\n"), d.iSerialNumber ));   \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bNumConfigurations: 0x%x\n"), d.bNumConfigurations ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("\n")));  \
}

#define DUMP_USB_CONFIGURATION_DESCRIPTOR( c ) { \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("USB_CONFIGURATION_DESCRIPTOR:\n"))); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("-----------------------------\n"))); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bLength: 0x%x\n"), c.bLength )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bDescriptorType: 0x%x\n"), c.bDescriptorType )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("wTotalLength: 0x%x\n"), c.wTotalLength )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bNumInterfaces: 0x%x\n"), c.bNumInterfaces )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bConfigurationValue: 0x%x\n"), c.bConfigurationValue )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("iConfiguration: 0x%x\n"), c.iConfiguration )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bmAttributes: 0x%x\n"), c.bmAttributes )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("MaxPower: 0x%x\n"), c.MaxPower )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("\n"))); \
}

#define DUMP_USB_INTERFACE_DESCRIPTOR( i, _index ) { \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("USB_INTERFACE_DESCRIPTOR[%d]:\n"), _index )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("-------------------------\n"))); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bLength: 0x%x\n"), i.bLength )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bDescriptorType: 0x%x\n"), i.bDescriptorType )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bInterfaceNumber: 0x%x\n"), i.bInterfaceNumber )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bAlternateSetting: 0x%x\n"), i.bAlternateSetting )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bNumEndpoints: 0x%x\n"), i.bNumEndpoints )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bInterfaceClass: 0x%x\n"), i.bInterfaceClass )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bInterfaceSubClass: 0x%x\n"), i.bInterfaceSubClass )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bInterfaceProtocol: 0x%x\n"), i.bInterfaceProtocol )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("iInterface: 0x%x\n"), i.iInterface )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("\n"))); \
}

#define DUMP_USB_ENDPOINT_DESCRIPTOR( e ) { \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("USB_ENDPOINT_DESCRIPTOR:\n"))); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("-----------------------------\n"))); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bLength: 0x%x\n"), e.bLength )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bDescriptorType: 0x%x\n"), e.bDescriptorType )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bEndpointAddress: 0x%x\n"), e.bEndpointAddress )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bmAttributes: 0x%x\n"), e.bmAttributes )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("wMaxPacketSize: 0x%x\n"), e.wMaxPacketSize )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bInterval: 0x%x\n"), e.bInterval ));\
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("\n"))); \
}


#define DUMP_USB_AUDIO_AS_INTERFACE_DESCRIPTOR( d ) { \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("USB_AUDIO_AS_INTERFACE_DESCRIPTOR:\n"))); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("----------------------\n"))); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bLength: 0x%x\n"), d.bLength ));   \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bDescriptorType: 0x%x\n"), d.bDescriptorType ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bDescriptorSubType: 0x%x\n"), d.bDescriptorSubType ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bTerminalLink: 0x%x\n"), d.bTerminalLink ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bDelay: 0x%x\n"), d.bDelay ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("wFormatTag: 0x%x\n"), d.wFormatTag )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("\n")));  \
}


#define DUMP_USB_AUDIO_AS_TYPE1_FORMAT_DESCRIPTOR( d ) { \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("USB_AUDIO_AS_TYPE1_FORMAT_DESCRIPTOR:\n"))); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("----------------------\n"))); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bLength: 0x%x\n"), d.bLength ));   \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bDescriptorType: 0x%x\n"), d.bDescriptorType ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bDescriptorSubType: 0x%x\n"), d.bDescriptorSubType ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bFormatType: 0x%x\n"), d.bFormatType ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bNumChannels: 0x%x\n"), d.bNumChannels ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bSubFrameSize: 0x%x\n"), d.bSubFrameSize )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bBitResolution: 0x%x\n"), d.bBitResolution )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bSamFreqType: 0x%x\n"), d.bSamFreqType )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("\n")));  \
}

#define DUMP_USB_AUDIO_CSI_DESCRIPTOR( d ) { \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("USB_USB_AUDIO_CSI_DESCRIPTOR:\n"))); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("----------------------\n"))); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bLength: 0x%x\n"), d.bLength ));   \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bDescriptorType: 0x%x\n"), d.bDescriptorType ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bDescriptorSubType: 0x%x\n"), d.bDescriptorSubType ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bcdADC: 0x%x\n"), d.bcdADC ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("wTotalLength: 0x%x\n"), d.wTotalLength ));  \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("bInCollection: 0x%x\n"), d.bInCollection )); \
   DEBUGMSG( ZONE_USB_PARSE, (TEXT("\n")));  \
}

#else
#define TEST_TRAP()
#define DUMP_USB_DEVICE_DESCRIPTOR( p )
#define DUMP_USB_CONFIGURATION_DESCRIPTOR( c )
#define DUMP_USB_INTERFACE_DESCRIPTOR( i, _index )
#define DUMP_USB_ENDPOINT_DESCRIPTOR( e )
#define DUMP_USB_AUDIO_CSI_DESCRIPTOR( d )
#define DUMP_USB_AUDIO_AS_INTERFACE_DESCRIPTOR( d )
#define DUMP_USB_AUDIO_AS_TYPE1_FORMAT_DESCRIPTOR( d )

#endif // DEBUG

#else
#define TEST_TRAP()
#define DUMP_USB_DEVICE_DESCRIPTOR( p )
#define DUMP_USB_CONFIGURATION_DESCRIPTOR( c )
#define DUMP_USB_INTERFACE_DESCRIPTOR( i, _index )
#define DUMP_USB_ENDPOINT_DESCRIPTOR( e )
#define DUMP_USB_AUDIO_CSI_DESCRIPTOR( d )
#define DUMP_USB_AUDIO_AS_INTERFACE_DESCRIPTOR( d )
#define DUMP_USB_AUDIO_AS_TYPE1_FORMAT_DESCRIPTOR( d )

#endif //~SHIP_BUILD



