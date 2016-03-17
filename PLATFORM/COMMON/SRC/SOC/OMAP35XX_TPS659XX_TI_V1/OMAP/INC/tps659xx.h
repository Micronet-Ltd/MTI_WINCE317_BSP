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
//  File: tps659xx.h
//
#ifndef __TPS659XX_H
#define __TPS659XX_H

//------------------------------------------------------------------------------

#define __TPS659XX_H
// generates a target message to control voltage regulators on T2

#define TWL_PSM(dev_grp, res_id, res_state)  \
            ((dev_grp << 13) | (res_id << 4) | (res_state << 0)) 

#define TWL_PBM(dev_grp, res_grp, res_typ2, res_typ1, res_state) \
            (((dev_grp) << 13) | (1 << 12) | ((res_grp) << 9) | \
             ((res_typ2) << 7) | ((res_typ1) << 4) | ((res_state) << 0))

// generates a target message to control voltage regulators on T2
#define TwlTargetMessage(dev_grp, res_id, res_state)  \
                           (UINT16)((dev_grp << 13) | (res_id << 4) | res_state)

// generates a Singular message sequence to control T2 resources
#define TwlSingularMsgSequence(dev_grp, res_id, res_state, delay, next_addr)  \
                                (UINT32)((dev_grp << 29) | (res_id << 20) | \
                                         (res_state << 16) | (delay << 8) | next_addr) 

// generates a Singular message sequence to control T2 resources
#define TwlBroadcastMsgSequence(dev_grp, res_grp, res_type2, res_type, res_state, delay, next_addr)  \
                                (UINT32)((dev_grp << 29) | (1 << 28) | (res_grp << 25) | \
                                         (res_type2 << 23) | (res_type << 20) | \
										 (res_state << 16) | (delay << 8) | next_addr) 

//------------------------------------------------------------------------------
// NOTE:
// the following values correspond to the interrupt array contained in
// the triton driver.  If changes are made to the following value 
// corresponding changes must be made in the triton driver.  Values
// are masked using the following format except for usb
//
// |27 26 25 24 | 23 22 21 20 | 19 18 17 16 | 15 14 13 12 | 11 10 9 8 | 7 6 5 4 | 3 2 1 0| 
//
// |  mask bit  |     register index        | SIH index   |    event array index         |

#define TWL_ARRAYINDEX(x)                       (x & 0xFFF)
#define TWL_SIHINDEX(x)                         ((x >> 12) & 0x0F)
#define TWL_REGISTERINDEX(x)                    ((x >> 16) & 0xFF)
#define TWL_MASKBIT(x)                          ((x >> 24) & 0x0F)

// power management
#define TWL_INTR_PWRON                          0x00005000
#define TWL_INTR_CHG_PRES                       0x01005001
#define TWL_INTR_USB_PRES                       0x02005002
#define TWL_INTR_RTC_IT                         0x03005003
#define TWL_INTR_HOT_DIE                        0x04005004
#define TWL_INTR_PWROK_TIMEOUT                  0x05005005
#define TWL_INTR_MBCHG                          0x06005006
#define TWL_INTR_SC_DETECT                      0x07005007

// bci
#define TWL_INTR_WOVF                           0x00002008
#define TWL_INTR_TMOVF                          0x01002009
#define TWL_INTR_ICHGHIGH                       0x0200200A
#define TWL_INTR_ICHGLOW                        0x0300200B
#define TWL_INTR_ICHGEOC                        0x0400200C
#define TWL_INTR_TBATOR2                        0x0500200D
#define TWL_INTR_TBATOR1                        0x0600200E
#define TWL_INTR_BATSTS                         0x0700200F
#define TWL_INTR_VBATLVL                        0x00012010
#define TWL_INTR_VBATOV                         0x01012011
#define TWL_INTR_VBUSOV                         0x02012012
#define TWL_INTR_ACCHGOV                        0x03012013
#define TWL_BATT_CONNECT                        0x00000040

// madc
#define TWL_INTR_MADC_RT                        0x00003014
#define TWL_INTR_MADC_SW1                       0x01003015
#define TWL_INTR_MADC_SW2                       0x02003016
#define TWL_INTR_MADC_USB                       0x03003017

// gpio
#define TWL_INTR_GPIO_0                         0x00000018
#define TWL_INTR_GPIO_1                         0x01000019
#define TWL_INTR_GPIO_2                         0x0200001A
#define TWL_INTR_GPIO_3                         0x0300001B
#define TWL_INTR_GPIO_4                         0x0400001C
#define TWL_INTR_GPIO_5                         0x0500001D
#define TWL_INTR_GPIO_6                         0x0600001E
#define TWL_INTR_GPIO_7                         0x0700001F
#define TWL_INTR_GPIO_8                         0x00010020
#define TWL_INTR_GPIO_9                         0x01010021
#define TWL_INTR_GPIO_10                        0x02010022
#define TWL_INTR_GPIO_11                        0x03010023
#define TWL_INTR_GPIO_12                        0x04010024
#define TWL_INTR_GPIO_13                        0x05010025
#define TWL_INTR_GPIO_14                        0x06010026
#define TWL_INTR_GPIO_15                        0x07010027
#define TWL_INTR_GPIO_16                        0x00020028
#define TWL_INTR_GPIO_17                        0x01020029

// keypad
#define TWL_INTR_ITKPI                          0x0000102A
#define TWL_INTR_ITLKI                          0x0100102B
#define TWL_INTR_ITTOIS                         0x0200102C
#define TWL_INTR_ITMIS                          0x0300102D

// usb
#define TWL_INTR_USB_RISE_IDGND                 0x0400402E
#define TWL_INTR_USB_RISE_SESSEND               0x0300402F
#define TWL_INTR_USB_RISE_SESSVALID             0x02004030
#define TWL_INTR_USB_RISE_VBUSVALID             0x01004031
#define TWL_INTR_USB_RISE_HOSTDISCONNECT        0x00004032
#define TWL_INTR_USB_FALL_IDGND                 0x04014033
#define TWL_INTR_USB_FALL_SESSEND               0x03014034
#define TWL_INTR_USB_FALL_SESSVALID             0x02014035
#define TWL_INTR_USB_FALL_VBUSVALID             0x01014036
#define TWL_INTR_USB_FALL_HOSTDISCONNECT        0x00014037

// USB_other
#define TWL_INTR_OTHER_RISE_VB_SESS_VLD         0x07024038
#define TWL_INTR_OTHER_RISE_DM_HI               0x06024039
#define TWL_INTR_OTHER_RISE_DP_HI               0x0502403A
#define TWL_INTR_OTHER_RISE_BDIS_ACON           0x0302403B
#define TWL_INTR_OTHER_RISE_MANU                0x0102403C
#define TWL_INTR_OTHER_RISE_ABNORMAL_STRESS     0x0002403D
#define TWL_INTR_OTHER_FALL_VB_SESS_VLD         0x0703403E
#define TWL_INTR_OTHER_FALL_DM_HI               0x0603403F
#define TWL_INTR_OTHER_FALL_DP_HI               0x05034040
#define TWL_INTR_OTHER_FALL_MANU                0x01034042
#define TWL_INTR_OTHER_FALL_ABNORMAL_STRESS     0x00034043

// carkit
#define TWL_INTR_CARKIT_CARDP                   0x00004044
#define TWL_INTR_CARKIT_CARINTDET               0x00004045
#define TWL_INTR_CARKIT_IDFLOAT                 0x00004046

#define TWL_INTR_CARKIT_PSM_ERR                 0x00004047
#define TWL_INTR_CARKIT_PH_ACC                  0x00004048
#define TWL_INTR_CARKIT_CHARGER                 0x00004049
#define TWL_INTR_CARKIT_USB_HOST                0x0000404A
#define TWL_INTR_CARKIT_USB_OTG                 0x0000404B
#define TWL_INTR_CARKIT_CARKIT                  0x0000404C
#define TWL_INTR_CARKIT_DISCONNECTED            0x0000404D

#define TWL_INTR_CARKIT_STOP_PLS_MISS           0x0000404E
#define TWL_INTR_CARKIT_STEREO_TO_MONO          0x0000404F
#define TWL_INTR_CARKIT_PHONE_UART              0x00004050
#define TWL_INTR_CARKIT_PH_NO_ACK               0x00004051

// id
#define TWL_INTR_ID_FLOAT                       0x00004052
#define TWL_INTR_ID_RES_440K                    0x00004053
#define TWL_INTR_ID_RES_200K                    0x00004054
#define TWL_INTR_ID_RES_100K                    0x00004055

#define TWL_MAX_INTR                            0x0056
#define TWL_KEYPAD_ROWS                         8

//------------------------------------------------------------------------------
// generic interrupt masks

#define TWL_SIH_CTRL_COR                (1 << 2)
#define TWL_SIH_CTRL_PENDDIS            (1 << 1)
#define TWL_SIH_CTRL_EXCLEN             (1 << 0)

// TWL_PWR_EDR1
#define TWL_RTC_IT_RISING               (1 << 7)
#define TWL_RTC_IT_FALLING              (1 << 6)
#define TWL_USB_PRES_RISING             (1 << 5)
#define TWL_USB_PRES_FALLING            (1 << 4)
#define TWL_CHG_PRES_RISING             (1 << 3)
#define TWL_CHG_PRES_FALLING            (1 << 2)
#define TWL_PWRON_RISING                (1 << 1)
#define TWL_PWRON_FALLING               (1 << 0)

// TWL_PWR_EDR2
#define TWL_SC_DETECT_RISING            (1 << 7)
#define TWL_SC_DETECT_FALLING           (1 << 6)
#define TWL_MBCHG_RISING                (1 << 5)
#define TWL_MBCHG_FALLING               (1 << 4)
#define TWL_PWROK_TIMEOUT_RISING        (1 << 3)
#define TWL_PWROK_TIMEOUT_FALLING       (1 << 2)
#define TWL_HOTDIE_RISING               (1 << 1)
#define TWL_HOTDIE_FALLING              (1 << 0)

// TWL_RTC_STATUS_REG
#define TWL_RTC_STATUS_POWER_UP         (1 << 7)  // reset event
#define TWL_RTC_STATUS_ALARM            (1 << 6)  // alarm event
#define TWL_RTC_STATUS_ONE_D_EVENT      (1 << 5)  // day event
#define TWL_RTC_STATUS_ONE_H_EVENT      (1 << 4)  // hour event 
#define TWL_RTC_STATUS_ONE_M_EVENT      (1 << 3)  // minute event
#define TWL_RTC_STATUS_ONE_S_EVENT      (1 << 2)  // second event
#define TWL_RTC_STATUS_RUN              (1 << 1)  // RTC run 

#define TWM_RTC_STATUS_TIME_EVENT       (0xF << 2)  // Any time (day,hour,min,sec) event 

// TWL_RTC_CTRL_REG 
#define TWL_RTC_CTRL_GET_TIME           (1 << 6)
#define TWL_RTC_CTRL_SET_32_COUNTER     (1 << 5)
#define TWL_RTC_CTRL_TEST_MODE          (1 << 4)
#define TWL_RTC_CTRL_MODE_12_24         (1 << 3)
#define TWL_RTC_CTRL_AUTO_COMP          (1 << 2)
#define TWL_RTC_CTRL_ROUND_30S          (1 << 1)
#define TWL_RTC_CTRL_RUN                (1 << 0)

// TWL_RTC_INTERRUPTS_REG
#define TWL_RTC_INTERRUPTS_EVERY_SECOND (0 << 0)
#define TWL_RTC_INTERRUPTS_EVERY_MINUTE (1 << 0)
#define TWL_RTC_INTERRUPTS_EVERY_HOUR   (2 << 0)
#define TWL_RTC_INTERRUPTS_EVERY_DAY    (3 << 0)
#define TWL_RTC_INTERRUPTS_IT_TIMER     (1 << 2)
#define TWL_RTC_INTERRUPTS_IT_ALARM     (1 << 3)


// TWL_KEYP_CTRL_REG
#define TWL_KBD_CTRL_NRESET             (1 << 0)
#define TWL_KBD_CTRL_NSOFT_MODE         (1 << 1)
#define TWL_KBD_CTRL_LK_EN              (1 << 2)
#define TWL_KBD_CTRL_TOE_EN             (1 << 3)
#define TWL_KBD_CTRL_TOLE_EN            (1 << 4)
#define TWL_KBD_CTRL_RP_EN              (1 << 5)
#define TWL_KBD_CTRL_KBD_ON             (1 << 6)

// TWL_KEYP_IMR1
#define TWL_KBD_INT_EVENT               (1 << 0)
#define TWL_KBD_INT_LONG_KEY            (1 << 1)
#define TWL_KBD_INT_TIMEOUT             (1 << 2)
#define TWL_KBD_INT_MISS                (1 << 3)

// TWL_CTRL_SWx
#define TWL_MADC_CTRL_SW_BUSY           (1 << 0)
#define TWL_MADC_CTRL_SW_EOC_SW1        (1 << 1)
#define TWL_MADC_CTRL_SW_EOC_RT         (1 << 2)
#define TWL_MADC_CTRL_SW_EOC_BCI        (1 << 3)
#define TWL_MADC_CTRL_SW_EOC_USB        (1 << 4)
#define TWL_MADC_CTRL_SW_TOGGLE         (1 << 5)

// TWL_OTG_CTRL, TWL_OTG_CTRL_SET, TWL_OTG_CTRL_CLR
#define TWL_OTG_CTRL_IDPULLUP           (1<<0)
#define TWL_OTG_CTRL_DPPULLDOWN         (1<<1)
#define TWL_OTG_CTRL_DMPULLDOWN         (1<<2)
#define TWL_OTG_CTRL_DISCHRGVBUS        (1<<3)
#define TWL_OTG_CTRL_CHRGVBUS           (1<<4)
#define TWL_OTG_CTRL_DRVVBUS            (1<<5)

// TWL_ID_STATUS
#define TWL_ID_STATUS_RES_GND           (1<<0)
#define TWL_ID_STATUS_RES_FLOAT         (1<<4)

// TWL_DEV_GROUP_x
#define TWL_DEV_GROUP_NONE              (0 << 5)
#define TWL_DEV_GROUP_P1                (1 << 5)
#define TWL_DEV_GROUP_P2                (2 << 5)
#define TWL_DEV_GROUP_P1P2              (3 << 5)
#define TWL_DEV_GROUP_P3                (4 << 5)
#define TWL_DEV_GROUP_P1P3              (5 << 5)
#define TWL_DEV_GROUP_P2P3              (6 << 5)
#define TWL_DEV_GROUP_ALL               (7 << 5)

// TWL_STS_HW_CONDITIONS
#define TWL_STS_USB                     (1 << 2)
#define TWL_STS_VBUS                    (1 << 7)

// Processor Groups 
#define TWL_PROCESSOR_GRP_NULL          0x0     // GROUP NULL
#define TWL_PROCESSOR_GRP1              0x01    // Applications devices
#define TWL_PROCESSOR_GRP2              0x02    // Modem Devices - ex:- N3G 
#define TWL_PROCESSOR_GRP3              0x04    // Peripherals 
#define TWL_PROCESSOR_GRP123            0x07

// Resource Groups
#define TWL_RES_GRP_NONE                0x0
#define TWL_RES_GRP_PP                  0x1
#define TWL_RES_GRP_RC                  0x2
#define TWL_RES_GRP_PP_RC               0x3
#define TWL_RES_GRP_PR                  0x4
#define TWL_RES_GRP_PP_PR               0x5
#define TWL_RES_GRP_RC_PR               0x6
#define TWL_RES_GRP_ALL                 0x7

// VMMC1 possible voltage values 

#define TWL_VMMC1_1P85                  0x00
#define TWL_VMMC1_2P85                  0x01
#define TWL_VMMC1_3P0                   0x02
#define TWL_VMMC1_3P15                  0x03

// VMMC2 possible voltage values 

#define TWL_VMMC2_1P85                  0x06
#define TWL_VMMC2_2P60                  0x08
#define TWL_VMMC2_2P85                  0x0A
#define TWL_VMMC2_3P00                  0x0B
#define TWL_VMMC2_3P15                  0x0C    // or 0x0D or 0x0E or 0x0F 

// VPLL2 possible voltage values
#define TWL_VPLL2_0P70                  0x0     // 0.70v
#define TWL_VPLL2_1P00                  0x1     // 1.00v
#define TWL_VPLL2_1P20                  0x2     // 1.20v
#define TWL_VPLL2_1P30                  0x3     // 1.30v
#define TWL_VPLL2_1P50                  0x4     // 1.50v
#define TWL_VPLL2_1P80                  0x5     // 1.80v
#define TWL_VPLL2_1P85                  0x6     // 1.85v
#define TWL_VPLL2_2P50                  0x7     // 2.50v
#define TWL_VPLL2_2P60                  0x8     // 2.60v
#define TWL_VPLL2_2P80                  0x9     // 2.80v
#define TWL_VPLL2_2P85                  0xA     // 2.85v
#define TWL_VPLL2_3P00                  0xB     // 3.00v
#define TWL_VPLL2_3P15                  0xC     // 3.15v

// VPLL1 possible voltage values
#define TWL_VPLL1_1P00                  0x0     // 1.00v
#define TWL_VPLL1_1P20                  0x1     // 1.20v
#define TWL_VPLL1_1P30                  0x2     // 1.30v
#define TWL_VPLL1_1P80                  0x3     // 1.80v
#define TWL_VPLL1_2P80                  0x4     // 2.80v
#define TWL_VPLL1_3P00                  0x5     // 3.00v

// VSIM possible voltage values
#define TWL_VSIM_1P00                   0x0     // 1.00v
#define TWL_VSIM_1P20                   0x1     // 1.20v
#define TWL_VSIM_1P30                   0x2     // 1.30v
#define TWL_VSIM_1P80                   0x3     // 1.80v
#define TWL_VSIM_2P80                   0x4     // 2.80v
#define TWL_VSIM_3P00                   0x5     // 3.00v

// VAUX4 possible voltage values
#define TWL_VAUX4_0P70                  0x0     // 0.70v
#define TWL_VAUX4_1P00                  0x1     // 1.00v
#define TWL_VAUX4_1P20                  0x2     // 1.20v
#define TWL_VAUX4_1P30                  0x3     // 1.30v
#define TWL_VAUX4_1P50                  0x4     // 1.50v
#define TWL_VAUX4_1P80                  0x5     // 1.80v
#define TWL_VAUX4_1P85                  0x6     // 1.85v
#define TWL_VAUX4_2P50                  0x7     // 2.50v
#define TWL_VAUX4_2P60                  0x8     // 2.60v
#define TWL_VAUX4_2P80                  0x9     // 2.80v
#define TWL_VAUX4_2P85                  0xA     // 2.85v
#define TWL_VAUX4_3P00                  0xB     // 3.00v
#define TWL_VAUX4_3P15                  0xC     // 3.15v

// VAUX3 possible voltage values
#define TWL_VAUX3_1P50                  0x0     // 1.50v
#define TWL_VAUX3_1P80                  0x1     // 1.80v
#define TWL_VAUX3_2P50                  0x2     // 2.50v
#define TWL_VAUX3_2P80                  0x3     // 2.80v
#define TWL_VAUX3_3P00                  0x4     // 3.00v

// VAUX2 possible voltage values
#define TWL_VAUX2_1P00                  0x1     // 1.00v
#define TWL_VAUX2_1P20                  0x2     // 1.20v
#define TWL_VAUX2_1P30                  0x3     // 1.30v
#define TWL_VAUX2_1P50                  0x4     // 1.50v
#define TWL_VAUX2_1P80                  0x5     // 1.80v
#define TWL_VAUX2_1P85                  0x6     // 1.85v
#define TWL_VAUX2_2P50                  0x7     // 2.50v
#define TWL_VAUX2_2P60                  0x8     // 2.60v
#define TWL_VAUX2_2P80                  0x9     // 2.80v
#define TWL_VAUX2_2P85                  0xA     // 2.85v
#define TWL_VAUX2_3P00                  0xB     // 3.00v
#define TWL_VAUX2_3P15                  0xC     // 3.15v

// VAUX1 possible voltage values
#define TWL_VAUX1_1P50                  0x0     // 1.50v
#define TWL_VAUX1_1P80                  0x1     // 1.80v
#define TWL_VAUX1_2P50                  0x2     // 2.50v
#define TWL_VAUX1_2P80                  0x3     // 2.80v
#define TWL_VAUX1_3P00                  0x4     // 3.00v

// VDAC possible voltage values
#define TWL_VDAC_1P20                   0x0     // 1.20v
#define TWL_VDAC_1P30                   0x1     // 1.30v
#define TWL_VDAC_1P80                   0x2     // 1.80v

// Power Source IDs 
#define TWL_VAUX1_RES_ID                1
#define TWL_VAUX2_RES_ID                2
#define TWL_VAUX3_RES_ID                3
#define TWL_VAUX4_RES_ID                4
#define TWL_VMMC1_RES_ID                5
#define TWL_VMMC2_RES_ID                6
#define TWL_VPLL1_RES_ID                7
#define TWL_VPLL2_RES_ID                8
#define TWL_VSIM_RES_ID                 9
#define TWL_VDAC_RES_ID                 10
#define TWL_VINTANA1_RES_ID             11
#define TWL_VINTANA2_RES_ID             12
#define TWL_VINTDIG_RES_ID              13
#define TWL_VIO_RES_ID                  14
#define TWL_VDD1_RES_ID                 15
#define TWL_VDD2_RES_ID                 16
#define TWL_VUSB_1V5_RES_ID             17
#define TWL_VUSB_1V8_RES_ID             18
#define TWL_VUSB_3V1_RES_ID             19
#define TWL_REGEN_RES_ID                21
#define TWL_NRESPWRON_RES_ID            22
#define TWL_CLKEN_RES_ID                23
#define TWL_SYSEN_RES_ID                24
#define TWL_HFCLKOUT_RES_ID             25
#define TWL_32KCLK_OUT_RES_ID           26
#define TWL_TRITON_RESET_RES_ID         27
#define TWL_MAIN_REF_RES_ID             28
#define TWL_NRES_PWRON_ID               22
#define TWL_CLKEN_ID                    23
#define TWL_SYSEN_ID                    24
#define TWL_HFCLKOUT_ID                 25
#define TWL_32KCLK_OUT_ID               26
#define TWL_TRITON_RESET                27
#define TWL_MAIN_REF_ID                 28
// Power Resource States
#define TWL_RES_OFF                     0x00  // Resource in OFF 
#define TWL_RES_SLEEP                   0x08  // Resource in SLEEP 
#define TWL_RES_ACTIVE                  0x0E  // Resource in ACTIVE 
#define TWL_RES_WRST                    0x0F  // Resource in Warm Reset


// Triton2 TPS659XX General purpose i2c addressing 
//------------------------------------------------------------------------------
// address group 0x4B

#define TWL_BACKUP_REG_A                0x004B0014
#define TWL_BACKUP_REG_B                0x004B0015
#define TWL_BACKUP_REG_C                0x004B0016
#define TWL_BACKUP_REG_D                0x004B0017
#define TWL_BACKUP_REG_E                0x004B0018
#define TWL_BACKUP_REG_F                0x004B0019
#define TWL_BACKUP_REG_G                0x004B001A
#define TWL_BACKUP_REG_H                0x004B001B

// interrupt registers
#define TWL_PWR_ISR1                    0x004B002E
#define TWL_PWR_IMR1                    0x004B002F
#define TWL_PWR_ISR2                    0x004B0030
#define TWL_PWR_IMR2                    0x004B0031
#define TWL_PWR_SIR                     0x004B0032
#define TWL_PWR_EDR1                    0x004B0033
#define TWL_PWR_EDR2                    0x004B0034
#define TWL_PWR_SIH_CTRL                0x004B0035

// power registers on/off & POR mode
#define TWL_CFG_P1_TRANSITION           0x004B0036
#define TWL_CFG_P2_TRANSITION           0x004B0037
#define TWL_CFG_P3_TRANSITION           0x004B0038
#define TWL_CFG_P123_TRANSITION         0x004B0039
#define TWL_STS_BOOT                    0x004B003A
#define TWL_CFG_BOOT                    0x004B003B
#define TWL_SHUNDAN                     0x004B003C
#define TWL_BOOT_BCI                    0x004B003D
#define TWL_CFG_PWRANA1                 0x004B003E
#define TWL_CFG_PWRANA2                 0x004B003F
#define TWL_BGAP_TRIM                   0x004B0040
#define TWL_BACKUP_MISC_STS             0x004B0041
#define TWL_BACKUP_MISC_CFG             0x004B0042
#define TWL_BACKUP_MISC_TST             0x004B0043
#define TWL_PROTECT_KEY                 0x004B0044
#define TWL_STS_HW_CONDITIONS           0x004B0045
#define TWL_P1_SW_EVENTS                0x004B0046
#define TWL_P2_SW_EVENTS                0x004B0047
#define TWL_P3_SW_EVENTS                0x004B0048
#define TWL_STS_P123_STATE              0x004B0049
#define TWL_PB_CFG                      0x004B004A
#define TWL_PB_WORD_MSB                 0x004B004B
#define TWL_PB_WORD_LSB                 0x004B004C
#define TWL_RESERVED_A                  0x004B004D
#define TWL_RESERVED_B                  0x004B004E
#define TWL_RESERVED_C                  0x004B004F
#define TWL_RESERVED_D                  0x004B0050
#define TWL_RESERVED_E                  0x004B0051
#define TWL_SEQ_ADD_W2P                 0x004B0052
#define TWL_SEQ_ADD_P2A                 0x004B0053
#define TWL_SEQ_ADD_A2W                 0x004B0054
#define TWL_SEQ_ADD_A2S                 0x004B0055
#define TWL_SEQ_ADD_S2A12               0x004B0056
#define TWL_SEQ_ADD_S2A3                0x004B0057
#define TWL_SEQ_ADD_WARM                0x004B0058
#define TWL_MEMORY_ADDRESS              0x004B0059
#define TWL_MEMORY_DATA                 0x004B005A

// pm receiver (un)secure mode
#define TWL_SC_CONFIG                   0x004B005B
#define TWL_SC_DETECT1                  0x004B005C
#define TWL_SC_DETECT2                  0x004B005D
#define TWL_WATCHDOG_CFG                0x004B005E
#define TWL_IT_CHECK_CFG                0x004B005F
#define TWL_VIBRATOR_CFG                0x004B0060
#define TWL_DCDC_GLOBAL_CFG             0x004B0061
#define TWL_VDD1_TRIM1                  0x004B0062
#define TWL_VDD1_TRIM2                  0x004B0063
#define TWL_VDD2_TRIM1                  0x004B0064
#define TWL_VDD2_TRIM2                  0x004B0065
#define TWL_VIO_TRIM1                   0x004B0066
#define TWL_VIO_TRIM2                   0x004B0067
#define TWL_MISC_CFG                    0x004B0068
#define TWL_LS_TST_A                    0x004B0069
#define TWL_LS_TST_B                    0x004B006A
#define TWL_LS_TST_C                    0x004B006B
#define TWL_LS_TST_D                    0x004B006C
#define TWL_BB_CFG                      0x004B006D
#define TWL_MISC_TST                    0x004B006E
#define TWL_TRIM1                       0x004B006F
#define TWL_TRIM2                       0x004B0070
#define TWL_DCDC_TIMEOUT                0x004B0071
#define TWL_VAUX1_DEV_GRP               0x004B0072
#define TWL_VAUX1_TYPE                  0x004B0073
#define TWL_VAUX1_REMAP                 0x004B0074
#define TWL_VAUX1_DEDICATED             0x004B0075
#define TWL_VAUX2_DEV_GRP               0x004B0076
#define TWL_VAUX2_TYPE                  0x004B0077
#define TWL_VAUX2_REMAP                 0x004B0078
#define TWL_VAUX2_DEDICATED             0x004B0079
#define TWL_VAUX3_DEV_GRP               0x004B007A
#define TWL_VAUX3_TYPE                  0x004B007B
#define TWL_VAUX3_REMAP                 0x004B007C
#define TWL_VAUX3_DEDICATED             0x004B007D
#define TWL_VAUX4_DEV_GRP               0x004B007E
#define TWL_VAUX4_TYPE                  0x004B007F
#define TWL_VAUX4_REMAP                 0x004B0080
#define TWL_VAUX4_DEDICATED             0x004B0081
#define TWL_VMMC1_DEV_GRP               0x004B0082
#define TWL_VMMC1_TYPE                  0x004B0083
#define TWL_VMMC1_REMAP                 0x004B0084
#define TWL_VMMC1_DEDICATED             0x004B0085
#define TWL_VMMC2_DEV_GRP               0x004B0086
#define TWL_VMMC2_TYPE                  0x004B0087
#define TWL_VMMC2_REMAP                 0x004B0088
#define TWL_VMMC2_DEDICATED             0x004B0089
#define TWL_VPLL1_DEV_GRP               0x004B008A
#define TWL_VPLL1_TYPE                  0x004B008B
#define TWL_VPLL1_REMAP                 0x004B008C
#define TWL_VPLL1_DEDICATED             0x004B008D
#define TWL_VPLL2_DEV_GRP               0x004B008E
#define TWL_VPLL2_TYPE                  0x004B008F
#define TWL_VPLL2_REMAP                 0x004B0090
#define TWL_VPLL2_DEDICATED             0x004B0091
#define TWL_VSIM_DEV_GRP                0x004B0092
#define TWL_VSIM_TYPE                   0x004B0093
#define TWL_VSIM_REMAP                  0x004B0094
#define TWL_VSIM_DEDICATED              0x004B0095
#define TWL_VDAC_DEV_GRP                0x004B0096
#define TWL_VDAC_TYPE                   0x004B0097
#define TWL_VDAC_REMAP                  0x004B0098
#define TWL_VDAC_DEDICATED              0x004B0099
#define TWL_VINTANA1_DEV_GRP            0x004B009A
#define TWL_VINTANA1_TYPE               0x004B009B
#define TWL_VINTANA1_REMAP              0x004B009C
#define TWL_VINTANA1_DEDICATED          0x004B009D
#define TWL_VINTANA2_DEV_GRP            0x004B009E
#define TWL_VINTANA2_TYPE               0x004B009F
#define TWL_VINTANA2_REMAP              0x004B00A0
#define TWL_VINTANA2_DEDICATED          0x004B00A1
#define TWL_VINTDIG_DEV_GRP             0x004B00A2
#define TWL_VINTDIG_TYPE                0x004B00A3
#define TWL_VINTDIG_REMAP               0x004B00A4
#define TWL_VINTDIG_DEDICATED           0x004B00A5
#define TWL_VIO_DEV_GRP                 0x004B00A6
#define TWL_VIO_TYPE                    0x004B00A7
#define TWL_VIO_REMAP                   0x004B00A8
#define TWL_VIO_CFG                     0x004B00A9
#define TWL_VIO_MISC_CFG                0x004B00AA
#define TWL_VIO_TEST1                   0x004B00AB
#define TWL_VIO_TEST2                   0x004B00AC
#define TWL_VIO_OSC                     0x004B00AD
#define TWL_VIO_RESERVED                0x004B00AE
#define TWL_VIO_VSEL                    0x004B00AF
#define TWL_VDD1_DEV_GRP                0x004B00B0
#define TWL_VDD1_TYPE                   0x004B00B1
#define TWL_VDD1_REMAP                  0x004B00B2
#define TWL_VDD1_CFG                    0x004B00B3
#define TWL_VDD1_MISC_CFG               0x004B00B4
#define TWL_VDD1_TEST1                  0x004B00B5
#define TWL_VDD1_TEST2                  0x004B00B6
#define TWL_VDD1_OSC                    0x004B00B7
#define TWL_VDD1_RESERVED               0x004B00B8
#define TWL_VDD1_VSEL                   0x004B00B9
#define TWL_VDD1_VMODE_CFG              0x004B00BA
#define TWL_VDD1_VFLOOR                 0x004B00BB
#define TWL_VDD1_VROOF                  0x004B00BC
#define TWL_VDD1_STEP                   0x004B00BD
#define TWL_VDD2_DEV_GRP                0x004B00BE
#define TWL_VDD2_TYPE                   0x004B00BF
#define TWL_VDD2_REMAP                  0x004B00C0
#define TWL_VDD2_CFG                    0x004B00C1
#define TWL_VDD2_MISC_CFG               0x004B00C2
#define TWL_VDD2_TEST1                  0x004B00C3
#define TWL_VDD2_TEST2                  0x004B00C4
#define TWL_VDD2_OSC                    0x004B00C5
#define TWL_VDD2_RESERVED               0x004B00C6
#define TWL_VDD2_VSEL                   0x004B00C7
#define TWL_VDD2_VMODE_CFG              0x004B00C8
#define TWL_VDD2_VFLOOR                 0x004B00C9
#define TWL_VDD2_VROOF                  0x004B00CA
#define TWL_VDD2_STEP                   0x004B00CB
#define TWL_VUSB1V5_DEV_GRP             0x004B00CC
#define TWL_VUSB1V5_TYPE                0x004B00CD
#define TWL_VUSB1V5_REMAP               0x004B00CE
#define TWL_VUSB1V8_DEV_GRP             0x004B00CF
#define TWL_VUSB1V8_TYPE                0x004B00D0
#define TWL_VUSB1V8_REMAP               0x004B00D1
#define TWL_VUSB3V1_DEV_GRP             0x004B00D2
#define TWL_VUSB3V1_TYPE                0x004B00D3
#define TWL_VUSB3V1_REMAP               0x004B00D4
#define TWL_VUSBCP_DEV_GRP              0x004B00D5
#define TWL_VUSBCP_TYPE                 0x004B00D6
#define TWL_VUSBCP_REMAP                0x004B00D7
#define TWL_VUSB_DEDICATED1             0x004B00D8
#define TWL_VUSB_DEDICATED2             0x004B00D9
#define TWL_REGEN_DEV_GRP               0x004B00DA
#define TWL_REGEN_TYPE                  0x004B00DB
#define TWL_REGEN_REMAP                 0x004B00DC
#define TWL_NRESPWRON_DEV_GRP           0x004B00DD
#define TWL_NRESPWRON_TYPE              0x004B00DE
#define TWL_NRESPWRON_REMAP             0x004B00DF
#define TWL_CLKEN_DEV_GRP               0x004B00E0
#define TWL_CLKEN_TYPE                  0x004B00E1
#define TWL_CLKEN_REMAP                 0x004B00E2
#define TWL_SYSEN_DEV_GRP               0x004B00E3
#define TWL_SYSEN_TYPE                  0x004B00E4
#define TWL_SYSEN_REMAP                 0x004B00E5
#define TWL_HFCLKOUT_DEV_GRP            0x004B00E6
#define TWL_HFCLKOUT_TYPE               0x004B00E7
#define TWL_HFCLKOUT_REMAP              0x004B00E8
#define TWL_32KCLKOUT_DEV_GRP           0x004B00E9
#define TWL_32KCLKOUT_TYPE              0x004B00EA
#define TWL_32KCLKOUT_REMAP             0x004B00EB
#define TWL_TRITON_RESET_DEV_GRP        0x004B00EC
#define TWL_TRITON_RESET_TYPE           0x004B00ED
#define TWL_TRITON_RESET_REMAP          0x004B00EE
#define TWL_MAINREF_DEV_GRP             0x004B00EF
#define TWL_MAINREF_TYPE                0x004B00F0
#define TWL_MAINREF_REMAP               0x004B00F1

// rtc
#define TWL_SECONDS_REG                 0x004B001C
#define TWL_MINUTES_REG                 0x004B001D
#define TWL_HOURS_REG                   0x004B001E
#define TWL_DAYS_REG                    0x004B001F
#define TWL_MONTHS_REG                  0x004B0020
#define TWL_YEARS_REG                   0x004B0021
#define TWL_WEEKS_REG                   0x004B0022
#define TWL_ALARM_SECONDS_REG           0x004B0023
#define TWL_ALARM_MINUTES_REG           0x004B0024
#define TWL_ALARM_HOURS_REG             0x004B0025
#define TWL_ALARM_DAYS_REG              0x004B0026
#define TWL_ALARM_MONTHS_REG            0x004B0027
#define TWL_ALARM_YEARS_REG             0x004B0028
#define TWL_RTC_CTRL_REG                0x004B0029
#define TWL_RTC_STATUS_REG              0x004B002A
#define TWL_RTC_INTERRUPTS_REG          0x004B002B
#define TWL_RTC_COMP_LSB_REG            0x004B002C
#define TWL_RTC_COMP_MSB_REG            0x004B002D

// secure registers
#define TWL_SECURED_REG_A               0x004B0000
#define TWL_SECURED_REG_B               0x004B0001
#define TWL_SECURED_REG_C               0x004B0002
#define TWL_SECURED_REG_D               0x004B0003
#define TWL_SECURED_REG_E               0x004B0004
#define TWL_SECURED_REG_F               0x004B0005
#define TWL_SECURED_REG_G               0x004B0006
#define TWL_SECURED_REG_H               0x004B0007
#define TWL_SECURED_REG_I               0x004B0008
#define TWL_SECURED_REG_J               0x004B0009
#define TWL_SECURED_REG_K               0x004B000A
#define TWL_SECURED_REG_L               0x004B000B
#define TWL_SECURED_REG_M               0x004B000C
#define TWL_SECURED_REG_N               0x004B000D
#define TWL_SECURED_REG_O               0x004B000E
#define TWL_SECURED_REG_P               0x004B000F
#define TWL_SECURED_REG_Q               0x004B0010
#define TWL_SECURED_REG_R               0x004B0011
#define TWL_SECURED_REG_S               0x004B0012
#define TWL_SECURED_REG_U               0x004B0013


//------------------------------------------------------------------------------
// address group 0x4A

// interrupts
#define TWL_BCIISR1A                    0x004A00B9
#define TWL_BCIISR2A                    0x004A00BA
#define TWL_BCIIMR1A                    0x004A00BB
#define TWL_BCIIMR2A                    0x004A00BC
#define TWL_BCIISR1B                    0x004A00BD
#define TWL_BCIISR2B                    0x004A00BE
#define TWL_BCIIMR1B                    0x004A00BF
#define TWL_BCIIMR2B                    0x004A00C0
#define TWL_BCISIR1                     0x004A00C1
#define TWL_BCISIR2                     0x004A00C2
#define TWL_BCIEDR1                     0x004A00C3
#define TWL_BCIEDR2                     0x004A00C4
#define TWL_BCIEDR3                     0x004A00C5
#define TWL_BCISIHCTRL                  0x004A00C6

// madc
#define TWL_CTRL1                       0x004A0000
#define TWL_CTRL2                       0x004A0001
#define TWL_RTSELECT_LSB                0x004A0002
#define TWL_RTSELECT_MSB                0x004A0003
#define TWL_RTAVERAGE_LSB               0x004A0004
#define TWL_RTAVERAGE_MSB               0x004A0005
#define TWL_SW1SELECT_LSB               0x004A0006
#define TWL_SW1SELECT_MSB               0x004A0007
#define TWL_SW1AVERAGE_LSB              0x004A0008
#define TWL_SW1AVERAGE_MSB              0x004A0009
#define TWL_SW2SELECT_LSB               0x004A000A
#define TWL_SW2SELECT_MSB               0x004A000B
#define TWL_SW2AVERAGE_LSB              0x004A000C
#define TWL_SW2AVERAGE_MSB              0x004A000D
#define TWL_BCI_USBAVERAGE              0x004A000E
#define TWL_ACQUISITION                 0x004A000F
#define TWL_USBREF_LSB                  0x004A0010
#define TWL_USBREF_MSB                  0x004A0011
#define TWL_CTRL_SW1                    0x004A0012
#define TWL_CTRL_SW2                    0x004A0013
#define TWL_MADC_TEST                   0x004A0014
#define TWL_GP_MADC_TEST1               0x004A0015
#define TWL_GP_MADC_TEST2               0x004A0016
#define TWL_RTCH0_LSB                   0x004A0017
#define TWL_RTCH0_MSB                   0x004A0018
#define TWL_RTCH1_LSB                   0x004A0019
#define TWL_RTCH1_MSB                   0x004A001A
#define TWL_RTCH2_LSB                   0x004A001B
#define TWL_RTCH2_MSB                   0x004A001C
#define TWL_RTCH3_LSB                   0x004A001D
#define TWL_RTCH3_MSB                   0x004A001E
#define TWL_RTCH4_LSB                   0x004A001F
#define TWL_RTCH4_MSB                   0x004A0020
#define TWL_RTCH5_LSB                   0x004A0021
#define TWL_RTCH5_MSB                   0x004A0022
#define TWL_RTCH6_LSB                   0x004A0023
#define TWL_RTCH6_MSB                   0x004A0024
#define TWL_RTCH7_LSB                   0x004A0025
#define TWL_RTCH7_MSB                   0x004A0026
#define TWL_RTCH8_LSB                   0x004A0027
#define TWL_RTCH8_MSB                   0x004A0028
#define TWL_RTCH9_LSB                   0x004A0029
#define TWL_RTCH9_MSB                   0x004A002A
#define TWL_RTCH10_LSB                  0x004A002B
#define TWL_RTCH10_MSB                  0x004A002C
#define TWL_RTCH11_LSB                  0x004A002D
#define TWL_RTCH11_MSB                  0x004A002E
#define TWL_RTCH12_LSB                  0x004A002F
#define TWL_RTCH12_MSB                  0x004A0030
#define TWL_RTCH13_LSB                  0x004A0031
#define TWL_RTCH13_MSB                  0x004A0032
#define TWL_RTCH14_LSB                  0x004A0033
#define TWL_RTCH14_MSB                  0x004A0034
#define TWL_RTCH15_LSB                  0x004A0035
#define TWL_RTCH15_MSB                  0x004A0036
#define TWL_GPCH0_LSB                   0x004A0037
#define TWL_GPCH0_MSB                   0x004A0038
#define TWL_GPCH1_LSB                   0x004A0039
#define TWL_GPCH1_MSB                   0x004A003A
#define TWL_GPCH2_LSB                   0x004A003B
#define TWL_GPCH2_MSB                   0x004A003C
#define TWL_GPCH3_LSB                   0x004A003D
#define TWL_GPCH3_MSB                   0x004A003E
#define TWL_GPCH4_LSB                   0x004A003F
#define TWL_GPCH4_MSB                   0x004A0040
#define TWL_GPCH5_LSB                   0x004A0041
#define TWL_GPCH5_MSB                   0x004A0042
#define TWL_GPCH6_LSB                   0x004A0043
#define TWL_GPCH6_MSB                   0x004A0044
#define TWL_GPCH7_LSB                   0x004A0045
#define TWL_GPCH7_MSB                   0x004A0046
#define TWL_GPCH8_LSB                   0x004A0047
#define TWL_GPCH8_MSB                   0x004A0048
#define TWL_GPCH9_LSB                   0x004A0049
#define TWL_GPCH9_MSB                   0x004A004A
#define TWL_GPCH10_LSB                  0x004A004B
#define TWL_GPCH10_MSB                  0x004A004C
#define TWL_GPCH11_LSB                  0x004A004D
#define TWL_GPCH11_MSB                  0x004A004E
#define TWL_GPCH12_LSB                  0x004A004F
#define TWL_GPCH12_MSB                  0x004A0050
#define TWL_GPCH13_LSB                  0x004A0051
#define TWL_GPCH13_MSB                  0x004A0052
#define TWL_GPCH14_LSB                  0x004A0053
#define TWL_GPCH14_MSB                  0x004A0054
#define TWL_GPCH15_LSB                  0x004A0055
#define TWL_GPCH15_MSB                  0x004A0056
#define TWL_BCICH0_LSB                  0x004A0057
#define TWL_BCICH0_MSB                  0x004A0058
#define TWL_BCICH1_LSB                  0x004A0059
#define TWL_BCICH1_MSB                  0x004A005A
#define TWL_BCICH2_LSB                  0x004A005B
#define TWL_BCICH2_MSB                  0x004A005C
#define TWL_BCICH3_LSB                  0x004A005D
#define TWL_BCICH3_MSB                  0x004A005E
#define TWL_BCICH4_LSB                  0x004A005F
#define TWL_BCICH4_MSB                  0x004A0060
#define TWL_MADC_ISR1                   0x004A0061
#define TWL_MADC_IMR1                   0x004A0062
#define TWL_MADC_ISR2                   0x004A0063
#define TWL_MADC_IMR2                   0x004A0064
#define TWL_MADC_SIR                    0x004A0065
#define TWL_MADC_EDR                    0x004A0066
#define TWL_MADC_SIH_CTRL               0x004A0067

// main charge
#define TWL_BCIMDEN                     0x004A0074
#define TWL_BCIMDKEY                    0x004A0075
#define TWL_BCIMSTATEC                  0x004A0076
#define TWL_BCIMSTATEP                  0x004A0077
#define TWL_BCIVBAT1                    0x004A0078
#define TWL_BCIVBAT2                    0x004A0079
#define TWL_BCITBAT1                    0x004A007A
#define TWL_BCITBAT2                    0x004A007B
#define TWL_BCIICHG1                    0x004A007C
#define TWL_BCIICHG2                    0x004A007D
#define TWL_BCIVAC1                     0x004A007E
#define TWL_BCIVAC2                     0x004A007F
#define TWL_BCIVBUS1                    0x004A0080
#define TWL_BCIVBUS2                    0x004A0081
#define TWL_BCIMFSTS2                   0x004A0082
#define TWL_BCIMFSTS3                   0x004A0083
#define TWL_BCIMFSTS4                   0x004A0084
#define TWL_BCIMFKEY                    0x004A0085
#define TWL_BCIMFEN1                    0x004A0086
#define TWL_BCIMFEN2                    0x004A0087
#define TWL_BCIMFEN3                    0x004A0088
#define TWL_BCIMFEN4                    0x004A0089
#define TWL_BCIMFTH1                    0x004A008A
#define TWL_BCIMFTH2                    0x004A008B
#define TWL_BCIMFTH3                    0x004A008C
#define TWL_BCIMFTH4                    0x004A008D
#define TWL_BCIMFTH5                    0x004A008E
#define TWL_BCIMFTH6                    0x004A008F
#define TWL_BCIMFTH7                    0x004A0090
#define TWL_BCIMFTH8                    0x004A0091
#define TWL_BCIMFTH9                    0x004A0092
#define TWL_BCITIMER1                   0x004A0093
#define TWL_BCITIMER2                   0x004A0094
#define TWL_BCIWDKEY                    0x004A0095
#define TWL_BCIWD                       0x004A0096
#define TWL_BCICTL1                     0x004A0097
#define TWL_BCICTL2                     0x004A0098
#define TWL_BCIVREF1                    0x004A0099
#define TWL_BCIVREF2                    0x004A009A
#define TWL_BCIIREF1                    0x004A009B
#define TWL_BCIIREF2                    0x004A009C
#define TWL_BCIPWM2                     0x004A009D
#define TWL_BCIPWM1                     0x004A009E
#define TWL_BCITRIM1                    0x004A009F
#define TWL_BCITRIM2                    0x004A00A0
#define TWL_BCITRIM3                    0x004A00A1
#define TWL_BCITRIM4                    0x004A00A2
#define TWL_BCIVREFCOMB1                0x004A00A3
#define TWL_BCIVREFCOMB2                0x004A00A4
#define TWL_BCIIREFCOMB1                0x004A00A5
#define TWL_BCIIREFCOMB2                0x004A00A6
#define TWL_BCIMNTEST1                  0x004A00A7
#define TWL_BCIMNTEST2                  0x004A00A8
#define TWL_BCIMNTEST3                  0x004A00A9

// pwm regisers
#define TWL_LEDEN                       0x004A00EE
#define TWL_PWMAON                      0x004A00EF
#define TWL_PWMAOFF                     0x004A00F0
#define TWL_PWMBON                      0x004A00F1
#define TWL_PWMBOFF                     0x004A00F2

#define TWL_PWM0ON                      0x004A00F8
#define TWL_PWM0OFF                     0x004A00F9
#define TWL_PWM1ON                      0x004A00FB
#define TWL_PWM1OFF                     0x004A00FC

// keypad 
#define TWL_KEYP_CTRL_REG               0x004A00D2
#define TWL_KEY_DEB_REG                 0x004A00D3
#define TWL_LONG_KEY_REG1               0x004A00D4
#define TWL_LK_PTV_REG                  0x004A00D5
#define TWL_TIME_OUT_REG1               0x004A00D6
#define TWL_TIME_OUT_REG2               0x004A00D7
#define TWL_KBC_REG                     0x004A00D8
#define TWL_KBR_REG                     0x004A00D9
#define TWL_KEYP_SMS                    0x004A00DA
#define TWL_FULL_CODE_7_0               0x004A00DB
#define TWL_FULL_CODE_15_8              0x004A00DC
#define TWL_FULL_CODE_23_16             0x004A00DD
#define TWL_FULL_CODE_31_24             0x004A00DE
#define TWL_FULL_CODE_39_32             0x004A00DF
#define TWL_FULL_CODE_47_40             0x004A00E0
#define TWL_FULL_CODE_55_48             0x004A00E1
#define TWL_FULL_CODE_63_56             0x004A00E2
#define TWL_KEYP_ISR1                   0x004A00E3
#define TWL_KEYP_IMR1                   0x004A00E4
#define TWL_KEYP_ISR2                   0x004A00E5
#define TWL_KEYP_IMR2                   0x004A00E6
#define TWL_KEYP_SIR                    0x004A00E7
#define TWL_KEYP_EDR                    0x004A00E8
#define TWL_KEYP_SIH_CTRL               0x004A00E9


//------------------------------------------------------------------------------
// address group 0x48

// usb
#define TWL_VENDOR_ID_LO                0x00480000
#define TWL_VENDOR_ID_HI                0x00480001
#define TWL_PRODUCT_ID_LO               0x00480002
#define TWL_PRODUCT_ID_HI               0x00480003
#define TWL_FUNC_CTRL                   0x00480004
#define TWL_FUNC_CTRL_SET               0x00480005
#define TWL_FUNC_CTRL_CLR               0x00480006
#define TWL_IFC_CTRL                    0x00480007
#define TWL_IFC_CTRL_SET                0x00480008
#define TWL_IFC_CTRL_CLR                0x00480009
#define TWL_OTG_CTRL                    0x0048000A
#define TWL_OTG_CTRL_SET                0x0048000B
#define TWL_OTG_CTRL_CLR                0x0048000C
#define TWL_USB_INT_EN_RISE             0x0048000D
#define TWL_USB_INT_EN_RISE_SET         0x0048000E
#define TWL_USB_INT_EN_RISE_CLR         0x0048000F
#define TWL_USB_INT_EN_FALL             0x00480010
#define TWL_USB_INT_EN_FALL_SET         0x00480011
#define TWL_USB_INT_EN_FALL_CLR         0x00480012
#define TWL_USB_INT_STS                 0x00480013
#define TWL_USB_INT_LATCH               0x00480014
#define TWL_DEBUG                       0x00480015
#define TWL_SCRATCH_REG                 0x00480016
#define TWL_SCRATCH_REG_SET             0x00480017
#define TWL_SCRATCH_REG_CLR             0x00480018
#define TWL_CARKIT_CTRL_SET             0x0048001A
#define TWL_CARKIT_CTRL                 0x00480019
#define TWL_CARKIT_CTRL_CLR             0x0048001B
#define TWL_CARKIT_INT_DELAY            0x0048001C
#define TWL_CARKIT_INT_EN               0x0048001D
#define TWL_CARKIT_INT_EN_SET           0x0048001E
#define TWL_CARKIT_INT_EN_CLR           0x0048001F
#define TWL_CARKIT_INT_STS              0x00480020
#define TWL_CARKIT_INT_LATCH            0x00480021
#define TWL_CARKIT_PLS_CTRL             0x00480022
#define TWL_CARKIT_PLS_CTRL_SET         0x00480023
#define TWL_CARKIT_PLS_CTRL_CLR         0x00480024
#define TWL_TRANS_POS_WIDTH             0x00480025
#define TWL_TRANS_NEG_WIDTH             0x00480026
#define TWL_RCV_PLTY_RECOVERY           0x00480027
#define TWL_MCPC_CTRL                   0x00480030
#define TWL_MCPC_CTRL_SET               0x00480031
#define TWL_MCPC_CTRL_CLR               0x00480032
#define TWL_MCPC_IO_CTRL                0x00480033
#define TWL_MCPC_IO_CTRL_SET            0x00480034
#define TWL_MCPC_IO_CTRL_CLR            0x00480035
#define TWL_MCPC_CTRL2                  0x00480036
#define TWL_MCPC_CTRL2_SET              0x00480037
#define TWL_MCPC_CTRL2_CLR              0x00480038
#define TWL_OTHER_FUNC_CTRL             0x00480080
#define TWL_OTHER_FUNC_CTRL_SET         0x00480081
#define TWL_OTHER_FUNC_CTRL_CLR         0x00480082
#define TWL_OTHER_IFC_CTRL              0x00480083
#define TWL_OTHER_IFC_CTRL_SET          0x00480084
#define TWL_OTHER_IFC_CTRL_CLR          0x00480085
#define TWL_OTHER_INT_EN_RISE           0x00480086
#define TWL_OTHER_INT_EN_RISE_SET       0x00480087
#define TWL_OTHER_INT_EN_RISE_CLR       0x00480088
#define TWL_OTHER_INT_EN_FALL           0x00480089
#define TWL_OTHER_INT_EN_FALL_SET       0x0048008A
#define TWL_OTHER_INT_EN_FALL_CLR       0x0048008B
#define TWL_OTHER_INT_STS               0x0048008C
#define TWL_OTHER_INT_LATCH             0x0048008D
#define TWL_ID_INT_EN_RISE              0x0048008E
#define TWL_ID_INT_EN_RISE_SET          0x0048008F
#define TWL_ID_INT_EN_RISE_CLR          0x00480090
#define TWL_ID_INT_EN_FALL              0x00480091
#define TWL_ID_INT_EN_FALL_SET          0x00480092
#define TWL_ID_INT_EN_FALL_CLR          0x00480093
#define TWL_ID_INT_STS                  0x00480094
#define TWL_ID_INT_LATCH                0x00480095
#define TWL_ID_STATUS                   0x00480096
#define TWL_CARKIT_SM_1_INT_EN          0x00480097
#define TWL_CARKIT_SM_1_INT_EN_SET      0x00480098
#define TWL_CARKIT_SM_1_INT_EN_CLR      0x00480099
#define TWL_CARKIT_SM_1_INT_STS         0x0048009A
#define TWL_CARKIT_SM_1_INT_LATCH       0x0048009B
#define TWL_CARKIT_SM_2_INT_EN          0x0048009C
#define TWL_CARKIT_SM_2_INT_EN_SET      0x0048009D
#define TWL_CARKIT_SM_2_INT_EN_CLR      0x0048009E
#define TWL_CARKIT_SM_2_INT_STS         0x0048009F
#define TWL_CARKIT_SM_2_INT_LATCH       0x004800A0
#define TWL_CARKIT_SM_CTRL              0x004800A1
#define TWL_CARKIT_SM_CTRL_SET          0x004800A2
#define TWL_CARKIT_SM_CTRL_CLR          0x004800A3
#define TWL_CARKIT_SM_CMD               0x004800A4
#define TWL_CARKIT_SM_CMD_SET           0x004800A5
#define TWL_CARKIT_SM_CMD_CLR           0x004800A6
#define TWL_CARKIT_SM_CMD_STS           0x004800A7
#define TWL_CARKIT_SM_STATUS            0x004800A8
#define TWL_CARKIT_SM_NEXT_STATUS       0x004800A9
#define TWL_CARKIT_SM_ERR_STATUS        0x004800AA
#define TWL_CARKIT_SM_CTRL_STATE        0x004800AB
#define TWL_POWER_CTRL                  0x004800AC
#define TWL_POWER_CTRL_SET              0x004800AD
#define TWL_POWER_CTRL_CLR              0x004800AE
#define TWL_OTHER_IFC_CTRL2             0x004800AF
#define TWL_OTHER_IFC_CTRL2_SET         0x004800B0
#define TWL_OTHER_IFC_CTRL2_CLR         0x004800B1
#define TWL_REG_CTRL_EN                 0x004800B2
#define TWL_REG_CTRL_EN_SET             0x004800B3
#define TWL_REG_CTRL_EN_CLR             0x004800B4
#define TWL_REG_CTRL_ERROR              0x004800B5
#define TWL_OTHER_FUNC_CTRL2            0x004800B8
#define TWL_OTHER_FUNC_CTRL2_SET        0x004800B9
#define TWL_OTHER_FUNC_CTRL2_CLR        0x004800BA
#define TWL_CARKIT_ANA_CTRL             0x004800BB
#define TWL_CARKIT_ANA_CTRL_SET         0x004800BC
#define TWL_CARKIT_ANA_CTRL_CLR         0x004800BD
#define TWL_VBUS_DEBOUNCE               0x004800C0
#define TWL_ID_DEBOUNCE                 0x004800C1
#define TWL_TPH_DP_CON_MIN              0x004800C2
#define TWL_TPH_DP_CON_MAX              0x004800C3
#define TWL_TCR_DP_CON_MIN              0x004800C4
#define TWL_TCR_DP_CON_MAX              0x004800C5
#define TWL_TPH_DP_PD_SHORT             0x004800C6
#define TWL_TPH_CMD_DLY                 0x004800C7
#define TWL_TPH_DET_RST                 0x004800C8
#define TWL_TPH_AUD_BIAS                0x004800C9
#define TWL_TCR_UART_DET_MIN            0x004800CA
#define TWL_TCR_UART_DET_MAX            0x004800CB
#define TWL_TPH_ID_INT_PW               0x004800CD
#define TWL_TACC_ID_INT_WAIT            0x004800CE
#define TWL_TACC_ID_INT_PW              0x004800CF
#define TWL_TPH_CMD_WAIT                0x004800D0
#define TWL_TPH_ACK_WAIT                0x004800D1
#define TWL_TPH_DP_DISC_DET             0x004800D2
#define TWL_VBAT_TIMER                  0x004800D3
#define TWL_CARKIT_4W_DEBUG             0x004800E0
#define TWL_CARKIT_5W_DEBUG             0x004800E1
#define TWL_TEST_CTRL_SET               0x004800EA
#define TWL_TEST_CTRL_CLR               0x004800EB
#define TWL_TEST_CARKIT_SET             0x004800EC
#define TWL_TEST_CARKIT_CLR             0x004800ED
#define TWL_TEST_POWER_SET              0x004800EE
#define TWL_TEST_POWER_CLR              0x004800EF
#define TWL_TEST_ULPI                   0x004800F0
#define TWL_TXVR_EN_TEST_SET            0x004800F2
#define TWL_TXVR_EN_TEST_CLR            0x004800F3
#define TWL_VBUS_EN_TEST                0x004800F4
#define TWL_ID_EN_TEST                  0x004800F5
#define TWL_PSM_EN_TEST_SET             0x004800F6
#define TWL_PSM_EN_TEST_CLR             0x004800F7
#define TWL_PHY_TRIM_CTRL               0x004800FC
#define TWL_PHY_PWR_CTRL                0x004800FD
#define TWL_PHY_CLK_CTRL                0x004800FE
#define TWL_PHY_CLK_CTRL_STS            0x004800FF


//------------------------------------------------------------------------------
// address group 0x49

// audio voice
#define TWL_CODEC_MODE                  0x00490001
#define TWL_OPTION                      0x00490002
#define TWL_MICBIAS_CTL                 0x00490004
#define TWL_ANAMICL                     0x00490005
#define TWL_ANAMICR                     0x00490006
#define TWL_AVADC_CTL                   0x00490007
#define TWL_ADCMICSEL                   0x00490008
#define TWL_DIGMIXING                   0x00490009
#define TWL_ATXL1PGA                    0x0049000A
#define TWL_ATXR1PGA                    0x0049000B
#define TWL_AVTXL2PGA                   0x0049000C
#define TWL_AVTXR2PGA                   0x0049000D
#define TWL_AUDIO_IF                    0x0049000E
#define TWL_VOICE_IF                    0x0049000F
#define TWL_ARXR1PGA                    0x00490010
#define TWL_ARXL1PGA                    0x00490011
#define TWL_ARXR2PGA                    0x00490012
#define TWL_ARXL2PGA                    0x00490013
#define TWL_VRXPGA                      0x00490014
#define TWL_VSTPGA                      0x00490015
#define TWL_VRX2ARXPGA                  0x00490016
#define TWL_AVDAC_CTL                   0x00490017
#define TWL_ARX2VTXPGA                  0x00490018
#define TWL_ARXL1_APGA_CTL              0x00490019
#define TWL_ARXR1_APGA_CTL              0x0049001A
#define TWL_ARXL2_APGA_CTL              0x0049001B
#define TWL_ARXR2_APGA_CTL              0x0049001C
#define TWL_ATX2ARXPGA                  0x0049001D
#define TWL_BT_IF                       0x0049001E
#define TWL_BTPGA                       0x0049001F
#define TWL_BTSTPGA                     0x00490020
#define TWL_EAR_CTL                     0x00490021
#define TWL_HS_SEL                      0x00490022
#define TWL_HS_GAIN_SET                 0x00490023
#define TWL_HS_POPN_SET                 0x00490024
#define TWL_PREDL_CTL                   0x00490025
#define TWL_PREDR_CTL                   0x00490026
#define TWL_PRECKL_CTL                  0x00490027
#define TWL_PRECKR_CTL                  0x00490028
#define TWL_HFL_CTL                     0x00490029
#define TWL_HFR_CTL                     0x0049002A
#define TWL_ALC_CTL                     0x0049002B
#define TWL_ALC_SET1                    0x0049002C
#define TWL_ALC_SET2                    0x0049002D
#define TWL_BOOST_CTL                   0x0049002E
#define TWL_SOFTVOL_CTL                 0x0049002F
#define TWL_DTMF_FREQSEL                0x00490030
#define TWL_DTMF_TONEXT1H               0x00490031
#define TWL_DTMF_TONEXT1L               0x00490032
#define TWL_DTMF_TONEXT2H               0x00490033
#define TWL_DTMF_TONEXT2L               0x00490034
#define TWL_DTMF_TONOFF                 0x00490035
#define TWL_DTMF_WANONOFF               0x00490036
#define TWL_I2S_RX_SCRAMBLE_H           0x00490037
#define TWL_I2S_RX_SCRAMBLE_M           0x00490038
#define TWL_I2S_RX_SCRAMBLE_L           0x00490039
#define TWL_APLL_CTL                    0x0049003A
#define TWL_DTMF_CTL                    0x0049003B
#define TWL_DTMF_PGA_CTL2               0x0049003C
#define TWL_DTMF_PGA_CTL1               0x0049003D
#define TWL_MISC_SET_1                  0x0049003E
#define TWL_PCMBTMUX                    0x0049003F
#define TWL_RX_PATH_SEL                 0x00490043
#define TWL_VDL_APGA_CTL                0x00490044
#define TWL_VIBRA_CTL                   0x00490045
#define TWL_VIBRA_SET                   0x00490046
#define TWL_VIBRA_PWM_SET               0x00490047
#define TWL_ANAMIC_GAIN                 0x00490048
#define TWL_MISC_SET_2                  0x00490049

// gpio
// Warning: The GPIODATAINx registers return incorrect values for pins that are outputs.
#define TWL_GPIODATAIN1                 0x00490098
#define TWL_GPIODATAIN2                 0x00490099
#define TWL_GPIODATAIN3                 0x0049009A
#define TWL_GPIODATADIR1                0x0049009B
#define TWL_GPIODATADIR2                0x0049009C
#define TWL_GPIODATADIR3                0x0049009D
#define TWL_GPIODATAOUT1                0x0049009E
#define TWL_GPIODATAOUT2                0x0049009F
#define TWL_GPIODATAOUT3                0x004900A0
#define TWL_CLEARGPIODATAOUT1           0x004900A1
#define TWL_CLEARGPIODATAOUT2           0x004900A2
#define TWL_CLEARGPIODATAOUT3           0x004900A3
#define TWL_SETGPIODATAOUT1             0x004900A4
#define TWL_SETGPIODATAOUT2             0x004900A5
#define TWL_SETGPIODATAOUT3             0x004900A6
#define TWL_GPIO_DEBEN1                 0x004900A7
#define TWL_GPIO_DEBEN2                 0x004900A8
#define TWL_GPIO_DEBEN3                 0x004900A9
#define TWL_GPIO_CTRL                   0x004900AA
#define TWL_GPIOPUPDCTR1                0x004900AB
#define TWL_GPIOPUPDCTR2                0x004900AC
#define TWL_GPIOPUPDCTR3                0x004900AD
#define TWL_GPIOPUPDCTR4                0x004900AE
#define TWL_GPIOPUPDCTR5                0x004900AF
#define TWL_GPIO_TEST                   0x004900B0
#define TWL_GPIO_ISR1A                  0x004900B1
#define TWL_GPIO_ISR2A                  0x004900B2
#define TWL_GPIO_ISR3A                  0x004900B3
#define TWL_GPIO_IMR1A                  0x004900B4
#define TWL_GPIO_IMR2A                  0x004900B5
#define TWL_GPIO_IMR3A                  0x004900B6
#define TWL_GPIO_ISR1B                  0x004900B7
#define TWL_GPIO_ISR2B                  0x004900B8
#define TWL_GPIO_ISR3B                  0x004900B9
#define TWL_GPIO_IMR1B                  0x004900BA
#define TWL_GPIO_IMR2B                  0x004900BB
#define TWL_GPIO_IMR3B                  0x004900BC
#define TWL_GPIO_SIR1                   0x004900BD
#define TWL_GPIO_SIR2                   0x004900BE
#define TWL_GPIO_SIR3                   0x004900BF
#define TWL_GPIO_EDR1                   0x004900C0
#define TWL_GPIO_EDR2                   0x004900C1
#define TWL_GPIO_EDR3                   0x004900C2
#define TWL_GPIO_EDR4                   0x004900C3
#define TWL_GPIO_EDR5                   0x004900C4
#define TWL_GPIO_SIH_CTRL               0x004900C5

// intbr
#define TWL_IDCODE_7_0                  0x00490085
#define TWL_IDCODE_15_8                 0x00490086
#define TWL_IDCODE_23_16                0x00490087
#define TWL_IDCODE_31_24                0x00490088
#define TWL_DIEID_7_0                   0x00490089
#define TWL_DIEID_15_8                  0x0049008A
#define TWL_DIEID_23_16                 0x0049008B
#define TWL_DIEID_31_24                 0x0049008C
#define TWL_DIEID_39_32                 0x0049008D
#define TWL_DIEID_47_40                 0x0049008E
#define TWL_DIEID_55_48                 0x0049008F
#define TWL_DIEID_63_56                 0x00490090
#define TWL_GPBR1                       0x00490091
#define TWL_PMBR1                       0x00490092
#define TWL_PMBR2                       0x00490093
#define TWL_GPPUPDCTR1                  0x00490094
#define TWL_GPPUPDCTR2                  0x00490095
#define TWL_GPPUPDCTR3                  0x00490096
#define TWL_UNLOCK_TEST_REG             0x00490097

// pih
#define TWL_PIH_ISR_P1                  0x00490081
#define TWL_PIH_ISR_P2                  0x00490082
#define TWL_PIH_SIR                     0x00490083

// test
#define TWL_AUDIO_TEST_CTL              0x0049004C
#define TWL_INT_TEST_CTL                0x0049004D
#define TWL_DAC_ADC_TEST_CTL            0x0049004E
#define TWL_RXTX_TRIM_IB                0x0049004F
#define TWL_CLD_CONTROL                 0x00490050
#define TWL_CLD_MODE_TIMING             0x00490051
#define TWL_CLD_TRIM_RAMP               0x00490052
#define TWL_CLD_TESTV_CTL               0x00490053
#define TWL_APLL_TEST_CTL               0x00490054
#define TWL_APLL_TEST_DIV               0x00490055
#define TWL_APLL_TEST_CTL2              0x00490056
#define TWL_APLL_TEST_CUR               0x00490057
#define TWL_DIGMIC_BIAS1_CTL            0x00490058
#define TWL_DIGMIC_BIAS2_CTL            0x00490059
#define TWL_RX_OFFSET_VOICE             0x0049005A
#define TWL_RX_OFFSET_AL1               0x0049005B
#define TWL_RX_OFFSET_AR1               0x0049005C
#define TWL_RX_OFFSET_AL2               0x0049005D
#define TWL_RX_OFFSET_AR2               0x0049005E
#define TWL_OFFSET1                     0x0049005F
#define TWL_OFFSET2                     0x00490060


// TWL_CFG_P1_TRANSITION
// TWL_CFG_P2_TRANSITION
// TWL_CFG_P3_TRANSITION
#define STARTON_SWBUG                   (1 << 7)            
#define STARTON_VBUS                    (1 << 5)
#define STARTON_VBAT                    (1 << 4)
#define STARTON_RTC                     (1 << 3)
#define STARTON_USB                     (1 << 2)
#define STARTON_CHG                     (1 << 1)
#define STARTON_PWON                    (1 << 0)

// TWL_CFG_BOOT
#define CK32K_LOWPWR_EN                 (1 << 7)
#define BOOT_CFG_SHIFT                  (4)
#define BOOT_CFG_MASK                   (7 << BOOT_CFG_SHIFT)
#define HIGH_PERF_SQ                    (1 << 3)
#define SLICER_BYPASS                   (1 << 2)
#define HFCLK_FREQ_SHIFT                (0)
#define HFCLK_FREQ_MASK                 (3 << HFCLK_FREQ_SHIFT)
#define HFCLK_FREQ_NOTPROGRAMMED        (0)
#define HFCLK_FREQ_19_2                 (1)
#define HFCLK_FREQ_26                   (2)
#define HFCLK_FREQ_38_4                 (3)

// TWL_GPBR1
#define MADC_HFCLK_EN                   (1 << 7)
#define MADC_3MHZ_EN                    (1 << 6)
#define BAT_MON_EN                      (1 << 5)
#define DEFAULT_MADC_CLK_EN             (1 << 4)
#define PWM1_ENABLE                     (1 << 3)
#define PWM0_ENABLE                     (1 << 2)
#define PWM1_CLK_ENABLE                 (1 << 1)
#define PWM0_CLK_ENABLE                 (1 << 0)

// TWL_PMBR1 
#define GPIO_PCM_DIGMICCLK(value)       ((value) << 6)

// TWL_CARKIT_ANA_CTRL_SET
    // ##### Michael, 4-Nov-2009
	// Add initialization of 'TWL_CARKIT_ANA_CTRL' register
	// Allow using ADCIN3-ADCIN6 as external A2Ds
#define DEFAULT_MADC_SEL_MCPC   (1 << 3)

// TWL_SW1AVERAGE_LSB
#define DEFAULT_MADC_GPCH_MASK	0x4

//------------------------------------------------------------------------------
// address group 0x12

// smartflex
#define TWL_VDD1_SR_CONTROL             0x00120000
#define TWL_VDD2_SR_CONTROL             0x00120001

//------------------------------------------------------------------------------
// address group 0xFF4A (logical address group)
#define TWL_LOGADDR_FULL_CODE_7_0       0xFF4A00DB

//-----------------------------------------------------------------------------
//   Union  : TRITON_PMB_MESSAGE
//      Defines the Power Management Bus Message 
//
typedef union {
    unsigned int    msgWord;
    unsigned char   msgByte[4];
} TRITON_PMB_MESSAGE;

//-----------------------------------------------------------------------------
//   Union  : TWL_PMB_ENTRY
//      entry for power message sequencer 
//
typedef struct {
    UINT16          msg;
    UINT8           delay;    
} TWL_PMB_ENTRY;

//------------------------------------------------------------------------------
#endif // __TPS659XX_H

