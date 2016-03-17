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
//  File:  omap35xx_irq.h
//
//  This file defines names for IRQ. This names have no other role than make
//  code more readable. For SoC where device - IRQ mapping is defined by
//  silicon and it can't be changed by software.
//
#ifndef __OMAP35XX_IRQ_H
#define __OMAP35XX_IRQ_H


//------------------------------------------------------------------------------
//
//  Global: Flag for fake OEMIdle busy loop

extern volatile BOOL fInterruptFlag;


//------------------------------------------------------------------------------

#define OMAP_IRQ_MAXIMUM                320
#define OMAP_IRQ_PER_SYSINTR            5
#define OMAP_GPIO_BANK_COUNT            6

//------------------------------------------------------------------------------
//
//  Physical IRQs on the 35xx
//

#define IRQ_EMUINT                      0   // MPU emulation
#define IRQ_COMMRX                      1   // MPU emulation
#define IRQ_COMMTX                      2   // MPU emulation
#define IRQ_NMPU                        3   // MPU emulation
#define IRQ_MCBSP2_ST                   4   // Sidetone MCBSP2 overflow
#define IRQ_MCBSP3_ST                   5   // Sidetone MCBSP3 overflow
#define IRQ_SSM_ABORT                   6   // MPU subsystem secure 
                                            // state-machine abort 

#define IRQ_SYS_NIRQ                    7   // External source (active low)
#define IRQ_D2D_FW_STACKED              8   // Occurs when modem causes security 
                                            // violation and has been 
                                            // automatically put DEVICE_SECURITY 
                                            // [0] under reset.

#define IRQ_SMX_DBG                     9   // SMX error for debug
#define IRQ_SMX_APP                     10  // SMX error for application
#define IRQ_PRCM_MPU                    11  // PRCM module IRQ
#define IRQ_SDMA0                       12  // System DMA request 0
#define IRQ_SDMA1                       13  // System DMA request 1
#define IRQ_SDMA2                       14  // System DMA request 2
#define IRQ_SDMA3                       15  // System DMA request 3
#define IRQ_MCBSP1                      16  // McBSP module 1 IRQ 
#define IRQ_MCBSP2                      17  // McBSP module 2 IRQ 
#define IRQ_SR1                         18  // SmartReflex 1
#define IRQ_SR2                         19  // SmartReflex 2
#define IRQ_GPMC                        20  // General-purpose memory 
                                            // controller module

#define IRQ_GFX                         21  // 2D/3D graphics module
#define IRQ_MCBSP3                      22  // McBSP module 3
#define IRQ_MCBSP4                      23  // McBSP module 4
#define IRQ_CAM0                        24  // Camera interface request 0
#define IRQ_DSS                         25  // Display subsystem module
#define IRQ_MAIL_U0_MPU                 26  // Mailbox user 0 request
#define IRQ_MCBSP5                      27  // McBSP module 5 
#define IRQ_IVA2_MMU                    28  // IVA2 MMU
#define IRQ_GPIO1_MPU                   29  // GPIO module 1 
#define IRQ_GPIO2_MPU                   30  // GPIO module 2 
#define IRQ_GPIO3_MPU                   31  // GPIO module 3 
#define IRQ_GPIO4_MPU                   32  // GPIO module 4 
#define IRQ_GPIO5_MPU                   33  // GPIO module 5
#define IRQ_GPIO6_MPU                   34  // GPIO module 6
#define IRQ_zzzRESERVED35               35  // 
#define IRQ_WDT3                        36  // Watchdog timer module 3 
                                            // overflow
#define IRQ_GPTIMER1                    37  // GPTimer module 1
#define IRQ_GPTIMER2                    38  // GPTimer module 2
#define IRQ_GPTIMER3                    39  // GPTimer module 3
#define IRQ_GPTIMER4                    40  // GPTimer module 4
#define IRQ_GPTIMER5                    41  // GPTimer module 5
#define IRQ_GPTIMER6                    42  // GPTimer module 6
#define IRQ_GPTIMER7                    43  // GPTimer module 7
#define IRQ_GPTIMER8                    44  // GPTimer module 8
#define IRQ_GPTIMER9                    45  // GPTimer module 9
#define IRQ_GPTIMER10                   46  // GPTimer module 10
#define IRQ_GPTIMER11                   47  // GPTimer module 11
#define IRQ_SPI4                        48  // McSPI module 4
#define IRQ_SHA1MD52                    49  // SHA-1/MD5 crypto-accelerator 2
#define IRQ_FPKAREADY_N                 50  // PKA crypto-accelerator
#define IRQ_SHA1MD5                     51  // SHA-1/MD5 crypto-accelerator 1
#define IRQ_RNG                         52  // RNG module
#define IRQ_MG                          53  // MG function 
#define IRQ_MCBSP4_TX                   54  // McBSP module 4 transmit
#define IRQ_MCBSP4_RX                   55  // McBSP module 4 receive
#define IRQ_I2C1                        56  // I2C module 1
#define IRQ_I2C2                        57  // I2C module 2
#define IRQ_HDQ                         58  // HDQ/One-wire
#define IRQ_McBSP1_TX                   59  // McBSP module 1 transmit
#define IRQ_McBSP1_RX                   60  // McBSP module 1 receive
#define IRQ_I2C3                        61  // I2C module 3
#define IRQ_McBSP2_TX                   62  // McBSP module 2 transmit
#define IRQ_McBSP2_RX                   63  // McBSP module 2 receive
#define IRQ_FPKARERROR_N                64  // PKA crypto-accelerator
#define IRQ_SPI1                        65  // McSPI module 1
#define IRQ_SPI2                        66  // McSPI module 2
#define IRQ_SSI_P1_MPU0                 67  // Dual SSI port 1 request 0
#define IRQ_SSI_P1_MPU1                 68  // Dual SSI port 1 request 1
#define IRQ_SSI_P2_MPU0                 69  // Dual SSI port 2 request 0
#define IRQ_SSI_P2_MPU1                 70  // Dual SSI port 2 request 1
#define IRQ_SSI_GDD_MPU                 71  // Dual SSI GDD
#define IRQ_UART1                       72  // UART module 1
#define IRQ_UART2                       73  // UART module 2
#define IRQ_UART3                       74  // UART module 3 (also infrared)
#define IRQ_PBIAS                       75  // PBIASlite1 and 2
#define IRQ_OHCI                        76  // OHCI controller HSUSB MP Host Interrupt 
#define IRQ_EHCI                        77  // EHCI controller HSUSB MP Host Interrupt
#define IRQ_TLL                         78  // HSUSB MP TLL Interrupt
#define IRQ_RESERVED1                   79  // unassigned 
#define IRQ_UART4                       80  // UART module 4 (37xx only)
#define IRQ_MCBSP5_TX                   81  // McBSP module 5 transmit
#define IRQ_MCBSP5_RX                   82  // McBSP module 5 receive
#define IRQ_MMC1                        83  // MMC/SD module 1
#define IRQ_RESERVED3                   84  // Reserved
#define IRQ_RESERVED4                   85  // Reserved
#define IRQ_MMC2                        86  // MMC/SD module 2
#define IRQ_MPU_ICR                     87  // MPU ICR
#define IRQ_D2DFRINT                    88  // From 3G coprocessor hardware 
                                            // when used in stacked modem 
                                            // configuration

#define IRQ_MCBSP3_TX                   89  // McBSP module 3 transmit
#define IRQ_MCBSP3_RX                   90  // McBSP module 3 receive
#define IRQ_SPI3                        91  // McSPI module 3
#define IRQ_HSUSB_MC_NINT               92  // High-Speed USB OTG controller
#define IRQ_HSUSB_DMA_NINT              93  // High-Speed USB OTG DMA controller
#define IRQ_MMC3                           94    // MMC/SD module 3
#define IRQ_RESERVED95                  95  // 

#define MAX_IRQ_COUNT                   96  // Total number of IRQs 

//------------------------------------------------------------------------------
//
//  Software IRQs for kernel/driver interaction
//

#define IRQ_SW_RTC_QUERY                100         // Query to OAL RTC
#define IRQ_SW_CPUMONITOR               101
#define IRQ_SW_LAN						102
#define IRQ_SW_RESERVED_3               103
#define IRQ_SW_RESERVED_4               104
#define IRQ_SW_RESERVED_5               105
#define IRQ_SW_RESERVED_6               106
#define IRQ_SW_RESERVED_7               107
#define IRQ_SW_RESERVED_8               108
#define IRQ_SW_RESERVED_9               109

#define IRQ_SW_RESERVED_MAX             110


//------------------------------------------------------------------------------
//
//  GPIO virtual IRQs for flattening out the 2 level GPIO interrupts
//

//GPIO Bank 1
#define IRQ_GPIO_0                      128         // GPIO1 bit 0
#define IRQ_GPIO_1                      129         // GPIO1 bit 1
#define IRQ_GPIO_2                      130         // GPIO1 bit 2
#define IRQ_GPIO_3                      131         // GPIO1 bit 3
#define IRQ_GPIO_4                      132         // GPIO1 bit 4
#define IRQ_GPIO_5                      133         // GPIO1 bit 5
#define IRQ_GPIO_6                      134         // GPIO1 bit 6
#define IRQ_GPIO_7                      135         // GPIO1 bit 7
#define IRQ_GPIO_8                      136         // GPIO1 bit 8
#define IRQ_GPIO_9                      137         // GPIO1 bit 9
#define IRQ_GPIO_10                     138         // GPIO1 bit 10
#define IRQ_GPIO_11                     139         // GPIO1 bit 11
#define IRQ_GPIO_12                     140         // GPIO1 bit 12
#define IRQ_GPIO_13                     141         // GPIO1 bit 13
#define IRQ_GPIO_14                     142         // GPIO1 bit 14
#define IRQ_GPIO_15                     143         // GPIO1 bit 15
#define IRQ_GPIO_16                     144         // GPIO1 bit 16
#define IRQ_GPIO_17                     145         // GPIO1 bit 17
#define IRQ_GPIO_18                     146         // GPIO1 bit 18
#define IRQ_GPIO_19                     147         // GPIO1 bit 19
#define IRQ_GPIO_20                     148         // GPIO1 bit 20
#define IRQ_GPIO_21                     149         // GPIO1 bit 21
#define IRQ_GPIO_22                     150         // GPIO1 bit 22
#define IRQ_GPIO_23                     151         // GPIO1 bit 23
#define IRQ_GPIO_24                     152         // GPIO1 bit 24
#define IRQ_GPIO_25                     153         // GPIO1 bit 25
#define IRQ_GPIO_26                     154         // GPIO1 bit 26
#define IRQ_GPIO_27                     155         // GPIO1 bit 27
#define IRQ_GPIO_28                     156         // GPIO1 bit 28
#define IRQ_GPIO_29                     157         // GPIO1 bit 29
#define IRQ_GPIO_30                     158         // GPIO1 bit 30
#define IRQ_GPIO_31                     159         // GPIO1 bit 31

//GPIO Bank 2
#define IRQ_GPIO_32                     160         // GPIO2 bit 0
#define IRQ_GPIO_33                     161         // GPIO2 bit 1
#define IRQ_GPIO_34                     162         // GPIO2 bit 2
#define IRQ_GPIO_35                     163         // GPIO2 bit 3
#define IRQ_GPIO_36                     164         // GPIO2 bit 4
#define IRQ_GPIO_37                     165         // GPIO2 bit 5
#define IRQ_GPIO_38                     166         // GPIO2 bit 6
#define IRQ_GPIO_39                     167         // GPIO2 bit 7
#define IRQ_GPIO_40                     168         // GPIO2 bit 8
#define IRQ_GPIO_41                     169         // GPIO2 bit 9
#define IRQ_GPIO_42                     170         // GPIO2 bit 10
#define IRQ_GPIO_43                     171         // GPIO2 bit 11
#define IRQ_GPIO_44                     172         // GPIO2 bit 12
#define IRQ_GPIO_45                     173         // GPIO2 bit 13
#define IRQ_GPIO_46                     174         // GPIO2 bit 14
#define IRQ_GPIO_47                     175         // GPIO2 bit 15
#define IRQ_GPIO_48                     176         // GPIO2 bit 16
#define IRQ_GPIO_49                     177         // GPIO2 bit 17
#define IRQ_GPIO_50                     178         // GPIO2 bit 18
#define IRQ_GPIO_51                     179         // GPIO2 bit 19
#define IRQ_GPIO_52                     180         // GPIO2 bit 20
#define IRQ_GPIO_53                     181         // GPIO2 bit 21
#define IRQ_GPIO_54                     182         // GPIO2 bit 22
#define IRQ_GPIO_55                     183         // GPIO2 bit 23
#define IRQ_GPIO_56                     184         // GPIO2 bit 24
#define IRQ_GPIO_57                     185         // GPIO2 bit 25
#define IRQ_GPIO_58                     186         // GPIO2 bit 26
#define IRQ_GPIO_59                     187         // GPIO2 bit 27
#define IRQ_GPIO_60                     188         // GPIO2 bit 28
#define IRQ_GPIO_61                     189         // GPIO2 bit 29
#define IRQ_GPIO_62                     190         // GPIO2 bit 30
#define IRQ_GPIO_63                     191         // GPIO2 bit 31

//GPIO Bank 3
#define IRQ_GPIO_64                     192         // GPIO3 bit 0
#define IRQ_GPIO_65                     193         // GPIO3 bit 1
#define IRQ_GPIO_66                     194         // GPIO3 bit 2
#define IRQ_GPIO_67                     195         // GPIO3 bit 3
#define IRQ_GPIO_68                     196         // GPIO3 bit 4
#define IRQ_GPIO_69                     197         // GPIO3 bit 5
#define IRQ_GPIO_70                     198         // GPIO3 bit 6
#define IRQ_GPIO_71                     199         // GPIO3 bit 7
#define IRQ_GPIO_72                     200         // GPIO3 bit 8
#define IRQ_GPIO_73                     201         // GPIO3 bit 9
#define IRQ_GPIO_74                     202         // GPIO3 bit 10
#define IRQ_GPIO_75                     203         // GPIO3 bit 11
#define IRQ_GPIO_76                     204         // GPIO3 bit 12
#define IRQ_GPIO_77                     205         // GPIO3 bit 13
#define IRQ_GPIO_78                     206         // GPIO3 bit 14
#define IRQ_GPIO_79                     207         // GPIO3 bit 15
#define IRQ_GPIO_80                     208         // GPIO3 bit 16
#define IRQ_GPIO_81                     209         // GPIO3 bit 17
#define IRQ_GPIO_82                     210         // GPIO3 bit 18
#define IRQ_GPIO_83                     211         // GPIO3 bit 19
#define IRQ_GPIO_84                     212         // GPIO3 bit 20
#define IRQ_GPIO_85                     213         // GPIO3 bit 21
#define IRQ_GPIO_86                     214         // GPIO3 bit 22
#define IRQ_GPIO_87                     215         // GPIO3 bit 23
#define IRQ_GPIO_88                     216         // GPIO3 bit 24
#define IRQ_GPIO_89                     217         // GPIO3 bit 25
#define IRQ_GPIO_90                     218         // GPIO3 bit 26
#define IRQ_GPIO_91                     219         // GPIO3 bit 27
#define IRQ_GPIO_92                     220         // GPIO3 bit 28
#define IRQ_GPIO_93                     221         // GPIO3 bit 29
#define IRQ_GPIO_94                     222         // GPIO3 bit 30
#define IRQ_GPIO_95                     223         // GPIO3 bit 31

//GPIO Bank 4
#define IRQ_GPIO_96                     224         // GPIO4 bit 0
#define IRQ_GPIO_97                     225         // GPIO4 bit 1
#define IRQ_GPIO_98                     226         // GPIO4 bit 2
#define IRQ_GPIO_99                     227         // GPIO4 bit 3
#define IRQ_GPIO_100                    228         // GPIO4 bit 4
#define IRQ_GPIO_101                    229         // GPIO4 bit 5
#define IRQ_GPIO_102                    230         // GPIO4 bit 6
#define IRQ_GPIO_103                    231         // GPIO4 bit 7
#define IRQ_GPIO_104                    232         // GPIO4 bit 8
#define IRQ_GPIO_105                    233         // GPIO4 bit 9
#define IRQ_GPIO_106                    234         // GPIO4 bit 10
#define IRQ_GPIO_107                    235         // GPIO4 bit 11
#define IRQ_GPIO_108                    236         // GPIO4 bit 12
#define IRQ_GPIO_109                    237         // GPIO4 bit 13
#define IRQ_GPIO_110                    238         // GPIO4 bit 14
#define IRQ_GPIO_111                    239         // GPIO4 bit 15
#define IRQ_GPIO_112                    240         // GPIO4 bit 16
#define IRQ_GPIO_113                    241         // GPIO4 bit 17
#define IRQ_GPIO_114                    242         // GPIO4 bit 18
#define IRQ_GPIO_115                    243         // GPIO4 bit 19
#define IRQ_GPIO_116                    244         // GPIO4 bit 20
#define IRQ_GPIO_117                    245         // GPIO4 bit 21
#define IRQ_GPIO_118                    246         // GPIO4 bit 22
#define IRQ_GPIO_119                    247         // GPIO4 bit 23
#define IRQ_GPIO_120                    248         // GPIO4 bit 24
#define IRQ_GPIO_121                    249         // GPIO4 bit 25
#define IRQ_GPIO_122                    250         // GPIO4 bit 26
#define IRQ_GPIO_123                    251         // GPIO4 bit 27
#define IRQ_GPIO_124                    252         // GPIO4 bit 28
#define IRQ_GPIO_125                    253         // GPIO4 bit 29
#define IRQ_GPIO_126                    254         // GPIO4 bit 30
#define IRQ_GPIO_127                    255         // GPIO4 bit 21

//GPIO Bank 5
#define IRQ_GPIO_128                    256         // GPIO4 bit 0
#define IRQ_GPIO_129                    257         // GPIO5 bit 1
#define IRQ_GPIO_130                    258         // GPIO5 bit 2
#define IRQ_GPIO_131                    259         // GPIO5 bit 3
#define IRQ_GPIO_132                    260         // GPIO5 bit 4
#define IRQ_GPIO_133                    261         // GPIO5 bit 5
#define IRQ_GPIO_134                    262         // GPIO5 bit 6
#define IRQ_GPIO_135                    263         // GPIO5 bit 7
#define IRQ_GPIO_136                    264         // GPIO5 bit 8
#define IRQ_GPIO_137                    265         // GPIO5 bit 9
#define IRQ_GPIO_138                    266         // GPIO5 bit 10
#define IRQ_GPIO_139                    267         // GPIO5 bit 11
#define IRQ_GPIO_140                    268         // GPIO5 bit 12
#define IRQ_GPIO_141                    269         // GPIO5 bit 13
#define IRQ_GPIO_142                    270         // GPIO5 bit 14
#define IRQ_GPIO_143                    271         // GPIO5 bit 15
#define IRQ_GPIO_144                    272         // GPIO5 bit 16
#define IRQ_GPIO_145                    273         // GPIO5 bit 17
#define IRQ_GPIO_146                    274         // GPIO5 bit 18
#define IRQ_GPIO_147                    275         // GPIO5 bit 19
#define IRQ_GPIO_148                    276         // GPIO5 bit 20
#define IRQ_GPIO_149                    277         // GPIO5 bit 21
#define IRQ_GPIO_150                    278         // GPIO5 bit 22
#define IRQ_GPIO_151                    279         // GPIO5 bit 23
#define IRQ_GPIO_152                    280         // GPIO5 bit 24
#define IRQ_GPIO_153                    281         // GPIO5 bit 25
#define IRQ_GPIO_154                    282         // GPIO5 bit 26
#define IRQ_GPIO_155                    283         // GPIO5 bit 27
#define IRQ_GPIO_156                    284         // GPIO5 bit 28
#define IRQ_GPIO_157                    285         // GPIO5 bit 29
#define IRQ_GPIO_158                    286         // GPIO5 bit 30
#define IRQ_GPIO_159                    287         // GPIO5 bit 31

//GPIO Bank 6
#define IRQ_GPIO_160                    288         // GPIO4 bit 0
#define IRQ_GPIO_161                    289         // GPIO5 bit 1
#define IRQ_GPIO_162                    290         // GPIO5 bit 2
#define IRQ_GPIO_163                    291         // GPIO5 bit 3
#define IRQ_GPIO_164                    292         // GPIO5 bit 4
#define IRQ_GPIO_165                    293         // GPIO5 bit 5
#define IRQ_GPIO_166                    294         // GPIO5 bit 6
#define IRQ_GPIO_167                    295         // GPIO5 bit 7
#define IRQ_GPIO_168                    296         // GPIO5 bit 8
#define IRQ_GPIO_169                    297         // GPIO5 bit 9
#define IRQ_GPIO_170                    298         // GPIO5 bit 10
#define IRQ_GPIO_171                    299         // GPIO5 bit 11
#define IRQ_GPIO_172                    300         // GPIO5 bit 12
#define IRQ_GPIO_173                    301         // GPIO5 bit 13
#define IRQ_GPIO_174                    302         // GPIO5 bit 14
#define IRQ_GPIO_175                    303         // GPIO5 bit 15
#define IRQ_GPIO_176                    304         // GPIO5 bit 16
#define IRQ_GPIO_177                    305         // GPIO5 bit 17
#define IRQ_GPIO_178                    306         // GPIO5 bit 18
#define IRQ_GPIO_179                    307         // GPIO5 bit 19
#define IRQ_GPIO_180                    308         // GPIO5 bit 20
#define IRQ_GPIO_181                    309         // GPIO5 bit 21
#define IRQ_GPIO_182                    310         // GPIO5 bit 22
#define IRQ_GPIO_183                    311         // GPIO5 bit 23
#define IRQ_GPIO_184                    312         // GPIO5 bit 24
#define IRQ_GPIO_185                    313         // GPIO5 bit 25
#define IRQ_GPIO_186                    314         // GPIO5 bit 26
#define IRQ_GPIO_187                    315         // GPIO5 bit 27
#define IRQ_GPIO_188                    316         // GPIO5 bit 28
#define IRQ_GPIO_189                    317         // GPIO5 bit 29
#define IRQ_GPIO_190                    318         // GPIO5 bit 30
#define IRQ_GPIO_191                    319         // GPIO5 bit 31


//------------------------------------------------------------------------------

#endif // __OMAP35XX_IRQ_H

