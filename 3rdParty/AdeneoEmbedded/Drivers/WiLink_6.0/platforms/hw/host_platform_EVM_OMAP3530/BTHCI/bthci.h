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
#ifndef _HCI_H
#define _HCI_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define:  HCI_xxx
//
//  BRF6300 power management packet/command codes
//
#define HCI_GO_TO_SLEEP_IND                 0x30
#define HCI_GO_TO_SLEEP_ACK                 0x31
#define HCI_WAKE_UP_IND                     0x32
#define HCI_WAKE_UP_ACK                     0x33

#define HCI_RESET                           0x0C03
#define HCI_READ_LOCAL_VERSION_INFORMATION  0x1001
#define HCI_READ_BUFFER_SIZE                0x1005
#define HCI_VS_SET_SLEEP_MODE_BRF6300               0xFD0C
#define HCI_VS_SET_SLEEP_MODE_BRF61XX               0xFF0C
#define HCI_VS_SET_JUST_WAKE_UP_TIMER       0xFF6C
#define HCI_VS_UPDATE_UART_HCI_BAUDRATE     0xFF36
#define HCI_VS_HCILL_PARAMETERS				0xFD2B

#define BRF_DEASSERTION_TIMEOUT				0x0000
#define BRF_INACTIVITY_TIMEOUT				0x0140
#define BRF_RETRANSMIT_TIMEOUT				0xffff
#define BRF_RTS_PULSE_WIDTH					0xff

#define COMMAND_COMPLETE_EVENT_CODE         0x0E


////////////////////////////////////////////////////////////////////////////////////////////
//FM Specific
#define HCC_GROUP_SHIFT(x)				((x) << 10)
#define HCC_GRP_VENDOR_SPECIFIC			(0x3F)

/* HCI Vendor-specific commands for FM */
#define HCIPP_I2C_FM_READ									(0x0133 | HCC_GROUP_SHIFT(HCC_GRP_VENDOR_SPECIFIC))
#define HCIPP_I2C_FM_READ_HW_REG							(0x0134 | HCC_GROUP_SHIFT(HCC_GRP_VENDOR_SPECIFIC))
#define HCIPP_I2C_FM_WRITE									(0x0135 | HCC_GROUP_SHIFT(HCC_GRP_VENDOR_SPECIFIC))
#define HCIPP_FM_POWER_MODE									(0x0137 | HCC_GROUP_SHIFT(HCC_GRP_VENDOR_SPECIFIC))
#define HCIPP_FM_SET_AUDIO_PATH								(0x0139 | HCC_GROUP_SHIFT(HCC_GRP_VENDOR_SPECIFIC))
#define HCIPP_FM_CHANGE_I2C_ADDR							(0x013A | HCC_GROUP_SHIFT(HCC_GRP_VENDOR_SPECIFIC))

//HCI EVENT for FM 
#define HCE_FM_EVENT		               0xF0
///////////////////////////////////////////////////////////////////////////////////////////////
#define TI_MANUFACTURER_ID                  13

//------------------------------------------------------------------------------
//
//  Type:  HCI_xxx_PACKET_HEADER
//
//  HCI UART headers
//
#pragma pack(push, 1)

typedef struct {
    UINT16 code;
    UINT8 length;
} HCI_COMMAND_PACKET_HEADER;

typedef struct {
    UINT16 connectionHandle;
    UINT16 length;
} HCI_ACL_PACKET_HEADER;

typedef struct {
    UINT16 connectionHandle;
    UINT8 length;
} HCI_SCO_PACKET_HEADER;

typedef struct {
    UINT8 code;
    UINT8 length;
} HCI_EVENT_PACKET_HEADER;

typedef struct {
    UINT8 type;
    union {
        HCI_COMMAND_PACKET_HEADER command;
        HCI_ACL_PACKET_HEADER acl;
        HCI_SCO_PACKET_HEADER sco;
        HCI_EVENT_PACKET_HEADER event;
    };
} HCI_PACKET_HEADER;

typedef struct {
    UINT8  type;
    UINT8  eventCode;
    UINT8  length;
    UINT8  commandPackets;
    UINT16 commandOpcode;
    UINT8  status;  
} HCI_PACKET_COMMAND_COMPLETE_EVENT;

typedef struct {
    UINT8  type;
    UINT8  eventCode;
    UINT8  length;
    UINT8  numHCICommandPackets;
    UINT16 commandOpcode;
    UINT8  status;    
    UINT8  hciVersion;
    UINT16 hciRevision;
    UINT8  lmpVersion;
    UINT16 manufacturerId;
    UINT16 lmpSubversion;
} HCI_PACKET_READ_LOCAL_VERSION_CC_EVENT;

typedef struct {
    UINT8  type;
    UINT8  eventCode;
    UINT8  length;
    UINT8  numHCICommandPackets;
    UINT16 commandOpcode;
    UINT8  status;    
    UINT16 aclDataPacketLength;
    UINT8  scoDataPacketLength;
    UINT16 totalNumAclDataPackets;
    UINT16 totalNumScoDataPackets;
} HCI_PACKET_READ_BUFFER_SIZE_CC_EVENT;

#pragma pack(pop)

//------------------------------------------------------------------------------
//
//  Type:  SCRIPT_HEADER
//
//  Script file header
//
//  The value 0x42535442 stands for (in ASCII) BTSB
#define SCRIPT_HEADER_MAGIC   0x42535442

typedef struct {
    ULONG magicNumber;
    ULONG version;
    UCHAR reserved[24];
} SCRIPT_HEADER;

//------------------------------------------------------------------------------
//
//  Type:  SCRIPT_ACTION_HEADER
//
//  Unified action header structure
//
typedef struct {
    USHORT code;
    USHORT size;
} SCRIPT_ACTION_HEADER;

//------------------------------------------------------------------------------
//
//  Define: ACTION_xxx
//
//  Script action codes...
//
#define ACTION_SEND_COMMAND             1   // Send out raw data (as is)
#define ACTION_WAIT_EVENT               2   // Wait for data
#define ACTION_SERIAL_PORT              3
#define ACTION_DELAY                    4
#define ACTION_RUN_SCRIPT               5
#define ACTION_REMARKS                  6

//------------------------------------------------------------------------------
// Structure for ACTION_SEND_COMMAND

typedef struct {
    UCHAR data[1];
} ACTION_COMMAND_PARAMS;

//------------------------------------------------------------------------------
// Structure for ACTION_WAIT_EVENT

typedef struct {
    ULONG timeToWait;               // in milliseconds
    ULONG sizeToWait;
    ULONG data[1];                  // data to wait for
} ACTION_WAIT_EVENT_PARAMS;

//------------------------------------------------------------------------------
// Structure for ACTION_SERIAL_PORT_PARAMETERS

typedef struct {
    ULONG baudRate;
    ULONG flowControl;
} ACTION_SERIAL_PORT_PARAMS;

// Flow Control Type
#define FCT_NONE            0
#define FCT_HARDWARE        1
#define DONT_CHANGE         0xFFFFFFFF  // For both baud rate and flow control

//------------------------------------------------------------------------------
// Structure for ACTION_DELAY

typedef struct {
    ULONG timeToWait;               // in milliseconds
} ACTION_DELAY_PARAMS;

//------------------------------------------------------------------------------
// Structure for ACTION_RUN_SCRIPT

typedef struct {
    CHAR fileName[1];
} ACTION_RUN_SCRIPT_PARAMS;

//------------------------------------------------------------------------------
// Structure for ACTION_REMARKS

typedef struct {
    CHAR remarks[1];
} ACTION_REMARKS_PARAMS;

//------------------------------------------------------------------------------
// Shut Down line on the PMIC GPIO13
#define BT_GPIO_SHUTDOWN					TRITON_GPIO(13)
#define IRQ_GPIO_144 272 // GPIO5 bit 16 (GPIO IRQs starts at 128)
#define BT_CTS_WAKEUP_IRQ            IRQ_GPIO_144
DWORD BT_CTS_WAKEUP_SysIntr;
HANDLE BT_CTS_WAKEUP_EVENT, BT_CTS_WAKEUP_THREAD;
BOOL  BT_ISRTerminating;

BOOL BT_WakeupEnabled;
BOOL btSleep_to_Awake_transition;

#ifdef __cplusplus
};
#endif

#endif // _HCI_H
