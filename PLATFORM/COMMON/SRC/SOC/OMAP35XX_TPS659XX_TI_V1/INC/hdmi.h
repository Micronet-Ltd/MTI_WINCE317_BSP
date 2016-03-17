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
//
//  File:  hdmi.h
//
#ifndef __HDMI_H
#define __HDMI_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define:  HDMI_DEVICE_NAME
//
#define HDMI_DEVICE_NAME        L"HDM1:"


// HDMI Connected States
#define HDMI_STATE_NOMONITOR    0       // No HDMI monitor connected
#define HDMI_STATE_CONNECTED    1       // HDMI monitor connected but powered off
#define HDMI_STATE_ON           2       // HDMI monitor connected and powered on


// HDMI EDID Length
#define HDMI_EDID_MAX_LENGTH    256

// HDMI EDID DTDs
#define HDMI_EDID_MAX_DTDS      4

// HDMI EDID DTD Tags
#define HDMI_EDID_DTD_TAG_MONITOR_NAME              0xFC
#define HDMI_EDID_DTD_TAG_MONITOR_SERIALNUM         0xFF
#define HDMI_EDID_DTD_TAG_MONITOR_LIMITS            0xFD


// HDMI EDID Extension Data Block Tags
#define HDMI_EDID_EX_DATABLOCK_TAG_MASK             0xE0
#define HDMI_EDID_EX_DATABLOCK_LEN_MASK             0x1F

#define HDMI_EDID_EX_DATABLOCK_AUDIO                0x20
#define HDMI_EDID_EX_DATABLOCK_VIDEO                0x40
#define HDMI_EDID_EX_DATABLOCK_VENDOR               0x60
#define HDMI_EDID_EX_DATABLOCK_SPEAKERS             0x80

// HDMI EDID Extenion Data Block Values: Video
#define HDMI_EDID_EX_VIDEO_NATIVE                   0x80
#define HDMI_EDID_EX_VIDEO_MASK                     0x7F
#define HDMI_EDID_EX_VIDEO_MAX                      35

#define HDMI_EDID_EX_VIDEO_640x480p_60Hz_4_3        1
#define HDMI_EDID_EX_VIDEO_720x480p_60Hz_4_3        2
#define HDMI_EDID_EX_VIDEO_720x480p_60Hz_16_9       3
#define HDMI_EDID_EX_VIDEO_1280x720p_60Hz_16_9      4
#define HDMI_EDID_EX_VIDEO_1920x1080i_60Hz_16_9     5
#define HDMI_EDID_EX_VIDEO_720x480i_60Hz_4_3        6
#define HDMI_EDID_EX_VIDEO_720x480i_60Hz_16_9       7
#define HDMI_EDID_EX_VIDEO_720x240p_60Hz_4_3        8
#define HDMI_EDID_EX_VIDEO_720x240p_60Hz_16_9       9
#define HDMI_EDID_EX_VIDEO_2880x480i_60Hz_4_3       10
#define HDMI_EDID_EX_VIDEO_2880x480i_60Hz_16_9      11
#define HDMI_EDID_EX_VIDEO_2880x480p_60Hz_4_3       12
#define HDMI_EDID_EX_VIDEO_2880x480p_60Hz_16_9      13
#define HDMI_EDID_EX_VIDEO_1440x480p_60Hz_4_3       14
#define HDMI_EDID_EX_VIDEO_1440x480p_60Hz_16_9      15
#define HDMI_EDID_EX_VIDEO_1920x1080p_60Hz_16_9     16
#define HDMI_EDID_EX_VIDEO_720x576p_50Hz_4_3        17
#define HDMI_EDID_EX_VIDEO_720x576p_50Hz_16_9       18
#define HDMI_EDID_EX_VIDEO_1280x720p_50Hz_16_9      19
#define HDMI_EDID_EX_VIDEO_1920x1080i_50Hz_16_9     20
#define HDMI_EDID_EX_VIDEO_720x576i_50Hz_4_3        21
#define HDMI_EDID_EX_VIDEO_720x576i_50Hz_16_9       22
#define HDMI_EDID_EX_VIDEO_720x288p_50Hz_4_3        23
#define HDMI_EDID_EX_VIDEO_720x288p_50Hz_16_9       24
#define HDMI_EDID_EX_VIDEO_2880x576i_50Hz_4_3       25
#define HDMI_EDID_EX_VIDEO_2880x576i_50Hz_16_9      26
#define HDMI_EDID_EX_VIDEO_2880x288p_50Hz_4_3       27
#define HDMI_EDID_EX_VIDEO_2880x288p_50Hz_16_9      28
#define HDMI_EDID_EX_VIDEO_1440x576p_50Hz_4_3       29
#define HDMI_EDID_EX_VIDEO_1440x576p_50Hz_16_9      30
#define HDMI_EDID_EX_VIDEO_1920x1080p_50Hz_16_9     31
#define HDMI_EDID_EX_VIDEO_1920x1080p_24Hz_16_9     32
#define HDMI_EDID_EX_VIDEO_1920x1080p_25Hz_16_9     33
#define HDMI_EDID_EX_VIDEO_1920x1080p_30Hz_16_9     34


//------------------------------------------------------------------------------

#define IOCTL_HDMI_CONNECTED        \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0700, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HDMI_READ_EDID        \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0701, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HDMI_ENABLE           \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0702, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HDMI_DISABLE          \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0703, METHOD_BUFFERED, FILE_ANY_ACCESS)


//
//  EDID - Extended Display ID Data structs
//

//  Video Descriptor Block
typedef struct
{
    UINT8   pixelClock[2];          // 54-55
    UINT8   horizActive;            // 56
    UINT8   horizBlanking;          // 57
    UINT8   horizHigh;              // 58
    UINT8   vertActive;             // 59
    UINT8   vertBlanking;           // 60
    UINT8   vertHigh;               // 61
    UINT8   horizSyncOffset;        // 62
    UINT8   horizSyncPulse;         // 63
    UINT8   vertSyncPulse;          // 64
    UINT8   syncPulseHigh;          // 65
    UINT8   horizImageSize;         // 66
    UINT8   vertImageSize;          // 67
    UINT8   imageSizeHigh;          // 68
    UINT8   horizBorder;            // 69
    UINT8   vertBorder;             // 70
    UINT8   miscSettings;           // 71
}
HDMI_EDID_DTD_VIDEO;


//  Monitor Limits Descriptor Block
typedef struct
{
    UINT8   pixelClock[2];          // 54-55
    UINT8   _reserved1;             // 56
    UINT8   blockType;              // 57
    UINT8   _reserved2;             // 58
    
    UINT8   minVertFreq;            // 59
    UINT8   maxVertFreq;            // 60
    UINT8   minHorizFreq;           // 61
    UINT8   maxHorizFreq;           // 62   
    UINT8   pixelClockMHz;          // 63
    
    UINT8   GTF[2];                 // 64-65
    UINT8   startHorizFreq;         // 66
    UINT8   C;                      // 67
    UINT8   M[2];                   // 68-69
    UINT8   K;                      // 70
    UINT8   J;                      // 71
}
HDMI_EDID_DTD_MONITOR;


//  Text Descriptor Block
typedef struct
{
    UINT8   pixelClock[2];          // 54-55
    UINT8   _reserved1;             // 56
    UINT8   blockType;              // 57
    UINT8   _reserved2;             // 58

    UCHAR   text[13];               // 59-71
}
HDMI_EDID_DTD_TEXT;
    

//  DTD Union
typedef union
{
    HDMI_EDID_DTD_VIDEO     Video;     
    HDMI_EDID_DTD_TEXT      MonitorName;   
    HDMI_EDID_DTD_TEXT      MonitorSerialNumber;
    HDMI_EDID_DTD_MONITOR   MonitorLimits;
}
HDMI_EDID_DTD;        


//  EDID struct
typedef struct
{
    UINT8   header[8];              // 00-07
    UINT8   manufacturerID[2];      // 08-09
    UINT8   productID[2];           // 10-11
    UINT8   serialNumber[4];        // 12-15
    UINT8   weekManufactured;       // 16
    UINT8   yearManufactured;       // 17
    UINT8   edidVersion;            // 18
    UINT8   edidRevision;           // 19

    UINT8   videoInDefinition;      // 20      
    UINT8   maxHorizImageSize;      // 21
    UINT8   maxVertImageSize;       // 22
    UINT8   displayGamma;           // 23
    UINT8   powerFeatures;          // 24
    UINT8   chromaInfo[10];         // 25-34       
    UINT8   timingI;                // 35
    UINT8   timingII;               // 36
    UINT8   timingIII;              // 37
    UINT8   stdTimings[16];         // 38-53
    
    HDMI_EDID_DTD   DTD[4];         // 72-125
    
    UINT8   extensionEDID;          // 126
    UINT8   checksum;               // 127

    UINT8   extensionTag;           // 00 (extensions follow EDID)
    UINT8   extentionRev;           // 01
    UINT8   offsetDTD;              // 02
    UINT8   numDTD;                 // 03

    UINT8   dataBlock[123];         // 04 - 126
    UINT8   extensionChecksum;      // 127
 }
HDMI_EDID;



//------------------------------------------------------------------------------
//
//  Functions: HDMIxxx
//
__inline HANDLE HDMIOpen()
{
    HANDLE  hHandle;
    
    //  Open handle to HDMI driver
    hHandle = CreateFile(HDMI_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
    return (hHandle == INVALID_HANDLE_VALUE) ? NULL : hHandle;
}

__inline VOID HDMIClose(HANDLE hContext)
{
    //  Close handle to HDMI driver
    if(hContext) CloseHandle(hContext);
}

__inline BOOL   HDMIConnected(HANDLE hContext, DWORD *pState)
{
    BOOL    bResult = FALSE;

    bResult = DeviceIoControl(hContext, 
                    IOCTL_HDMI_CONNECTED, 
                    NULL,
                    0,
                    pState,
                    sizeof(DWORD),
                    NULL,
                    NULL );
    
    return bResult; 
}

__inline BOOL   HDMIReadEDID(HANDLE hContext, DWORD dwLen, UCHAR *pBuffer)
{
    BOOL    bResult = FALSE;

    bResult = DeviceIoControl(hContext, 
                    IOCTL_HDMI_READ_EDID, 
                    NULL,
                    0,
                    pBuffer,
                    dwLen,
                    NULL,
                    NULL );
    
    return bResult; 
}
    
__inline BOOL   HDMIEnable(HANDLE hContext)
{
    BOOL    bResult = FALSE;

    bResult = DeviceIoControl(hContext, 
                    IOCTL_HDMI_ENABLE, 
                    NULL,
                    0,
                    NULL,
                    0,
                    NULL,
                    NULL );
    
    return bResult; 
}

__inline BOOL   HDMIDisable(HANDLE hContext)
{
    BOOL    bResult = FALSE;

    bResult = DeviceIoControl(hContext, 
                    IOCTL_HDMI_DISABLE, 
                    NULL,
                    0,
                    NULL,
                    0,
                    NULL,
                    NULL );
    
    return bResult; 
}



#ifdef __cplusplus
}
#endif

#endif
