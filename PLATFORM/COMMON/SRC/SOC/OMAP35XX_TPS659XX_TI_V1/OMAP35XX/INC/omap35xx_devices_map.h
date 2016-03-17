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
//  File:  omap35xx_devices_map.h
//
//  Configuration file for the device list
//
//  This file is used by omap35xxbus.cpp for s_DeviceAddressMap    
//  This file should only  contain the DEVICE_ADDRESS_MAP array.
//
//  DeviceAddress    Device id
//------------------------------------------------------------------------------

    {        OMAP_GPIO1_REGS_PA,               OMAP_DEVICE_GPIO1       },
    {        OMAP_GPIO2_REGS_PA,               OMAP_DEVICE_GPIO2       },
    {        OMAP_GPIO3_REGS_PA,               OMAP_DEVICE_GPIO3       },
    {        OMAP_GPIO4_REGS_PA,               OMAP_DEVICE_GPIO4       },
    {        OMAP_GPIO5_REGS_PA,               OMAP_DEVICE_GPIO5       },
    {        OMAP_GPIO6_REGS_PA,               OMAP_DEVICE_GPIO6       },
    {        OMAP_GPTIMER12_REGS_PA,           OMAP_DEVICE_GPTIMER12   },
    {        OMAP_GPTIMER2_REGS_PA,            OMAP_DEVICE_GPTIMER2    },
    {        OMAP_GPTIMER3_REGS_PA,            OMAP_DEVICE_GPTIMER3    },
    {        OMAP_GPTIMER4_REGS_PA,            OMAP_DEVICE_GPTIMER4    },
    {        OMAP_GPTIMER5_REGS_PA,            OMAP_DEVICE_GPTIMER5    },
    {        OMAP_GPTIMER6_REGS_PA,            OMAP_DEVICE_GPTIMER6    },
    {        OMAP_GPTIMER7_REGS_PA,            OMAP_DEVICE_GPTIMER7    },
    {        OMAP_GPTIMER8_REGS_PA,            OMAP_DEVICE_GPTIMER8    },
    {        OMAP_GPTIMER9_REGS_PA,            OMAP_DEVICE_GPTIMER9    },
    {        OMAP_GPTIMER10_REGS_PA,           OMAP_DEVICE_GPTIMER10   },
    {        OMAP_GPTIMER11_REGS_PA,           OMAP_DEVICE_GPTIMER11   },
    {        OMAP_MCBSP1_REGS_PA,              OMAP_DEVICE_MCBSP1      },
    {        OMAP_MCBSP2_REGS_PA,              OMAP_DEVICE_MCBSP2      },
    {        OMAP_MCBSP3_REGS_PA,              OMAP_DEVICE_MCBSP3      },
    {        OMAP_MCBSP4_REGS_PA,              OMAP_DEVICE_MCBSP4      },
    {        OMAP_MCBSP5_REGS_PA,              OMAP_DEVICE_MCBSP5      },
    {        OMAP_UART1_REGS_PA,               OMAP_DEVICE_UART1       },
    {        OMAP_UART2_REGS_PA,               OMAP_DEVICE_UART2       },
    {        OMAP_I2C1_REGS_PA,                OMAP_DEVICE_I2C1        },
    {        OMAP_I2C2_REGS_PA,                OMAP_DEVICE_I2C2        },
    {        OMAP_I2C3_REGS_PA,                OMAP_DEVICE_I2C3        },
    {        OMAP_MCSPI1_REGS_PA,              OMAP_DEVICE_MCSPI1      },
    {        OMAP_MCSPI2_REGS_PA,              OMAP_DEVICE_MCSPI2      },
    {        OMAP_MCSPI3_REGS_PA,              OMAP_DEVICE_MCSPI3      },
    {        OMAP_MCSPI4_REGS_PA,              OMAP_DEVICE_MCSPI4      },
    {        OMAP_MMCHS1_REGS_PA,              OMAP_DEVICE_MMC1        },
    {        OMAP_MMCHS2_REGS_PA,              OMAP_DEVICE_MMC2        },
    {        OMAP_MMCHS3_REGS_PA,              OMAP_DEVICE_MMC3        },
    {        OMAP_HSUSB_REGS_PA,               OMAP_DEVICE_HSOTGUSB    },
    {        OMAP_GPTIMER1_REGS_PA,            OMAP_DEVICE_GPTIMER1    },
    {        OMAP_UART3_REGS_PA,               OMAP_DEVICE_UART3       },
    {        OMAP_HDQ_1WIRE_REGS_PA,           OMAP_DEVICE_HDQ         },
    {        OMAP_SGX_REGS_PA,                 OMAP_DEVICE_SGX         },
    {        OMAP_SSI_REGS_PA,                 OMAP_DEVICE_SSI         },
    {        OMAP_SDRC_REGS_PA,                OMAP_DEVICE_SDRC        },
    {        OMAP_WDOG1_REGS_PA,               OMAP_DEVICE_WDT1        },
    {        OMAP_WDOG2_REGS_PA,               OMAP_DEVICE_WDT2        },
    {        OMAP_WDOG3_REGS_PA,               OMAP_DEVICE_WDT3        },
    {        OMAP_32KSYNC_REGS_PA,             OMAP_DEVICE_32KSYNC     },
    {        OMAP_CONFIG_REGS_PA,              OMAP_DEVICE_OMAPCTRL    },
    {        OMAP_DSS1_REGS_PA,                OMAP_DEVICE_DSS         },
    {        OMAP_CAMISP_REGS_PA,              OMAP_DEVICE_CAMERA      },
    {        OMAP_MLB1_REGS_PA,                OMAP_DEVICE_MAILBOXES   },
    {        OMAP_USBHOST1_REGS_LA,            OMAP_DEVICE_USBHOST1    },
    {        OMAP_USBHOST2_REGS_LA,            OMAP_DEVICE_USBHOST2    },
    {        OMAP_USBHOST3_REGS_LA,            OMAP_DEVICE_USBHOST3    },
    {        OMAP_USBTLL_REGS_PA,              OMAP_DEVICE_USBTLL      },
    {        OMAP_CAMCSI2_RECEIVER_REGS_PA,    OMAP_DEVICE_CSI2        },
    {        OMAP_USIM_REG_PA,                 OMAP_DEVICE_USIM        },
    {        OMAP_VRFB_REGS_PA,                OMAP_DEVICE_VRFB        },
    {        OMAP35XX_GENERIC,                 OMAP_DEVICE_GENERIC     },
    {        OMAP_VENC1_REGS_PA,               OMAP_DEVICE_TVOUT       }, 
    {        OMAP_SMARTREFLEX1_PA,             OMAP_DEVICE_SR1         },
    {        OMAP_SMARTREFLEX2_PA,             OMAP_DEVICE_SR2         },
    {        OMAP_PRCM_IVA2_CM_REGS_PA,        OMAP_DEVICE_DSP         },
    {        OMAP_UART4_REGS_PA,               OMAP_DEVICE_UART4       }, /* 37xx only */
#if 0
    {        OMAP_2D_REGS_PA,                  OMAP_DEVICE_2D          },
    {        OMAP_3D_REGS_PA,                  OMAP_DEVICE_3D          },
    {        OMAP_MSPRO_REGS_PA,               OMAP_DEVICE_MSPRO       },
    {        OMAP_DES2_REGS_PA,                OMAP_DEVICE_DES2        },
    {        OMAP_SHA12_REGS_PA,               OMAP_DEVICE_SHA12       },
    {        OMAP_AES2_REGS_PA,                OMAP_DEVICE_AES2        },
    {        OMAP_ICR_REGS_PA,                 OMAP_DEVICE_ICR         },
    {        OMAP_DES1_REGS_PA,                OMAP_DEVICE_DES1        },
    {        OMAP_SHA11_REGS_PA,               OMAP_DEVICE_SHA11       },
    {        OMAP_RNG_REGS_PA,                 OMAP_DEVICE_RNG         },
    {        OMAP_AES1_REGS_PA,                OMAP_DEVICE_AES1        },
    {        OMAP_PKA_REGS_PA,                 OMAP_DEVICE_PKA         },
    {        OMAP_D2D_REGS_PA,                 OMAP_DEVICE_D2D         },
#endif
    {        0,                                OMAP_DEVICE_NONE        }

