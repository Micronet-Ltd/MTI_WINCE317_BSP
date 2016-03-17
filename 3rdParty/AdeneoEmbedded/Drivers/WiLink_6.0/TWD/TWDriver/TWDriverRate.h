/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/
#ifndef TWDRIVERRATE_H
#define TWDRIVERRATE_H

/** \file  TWDriverRate.h 
 *  \brief TWDriver Rate APIs
 *
 *  \see
 */

/** \enum ERate
 * \brief Rate Types
 * 
 * \par Description
 * Driver's TX Control Frame Rate Format Type
 * 
 * \sa
 */
typedef enum
{
    DRV_RATE_AUTO       = 0,				/**< Auto							*/
    DRV_RATE_1M         = 1,				/**< 1M								*/
    DRV_RATE_2M         = 2,				/**< 2M								*/
    DRV_RATE_5_5M       = 3,				/**< 5.5M							*/
    DRV_RATE_11M        = 4,				/**< 11M							*/
    DRV_RATE_22M        = 5,				/**< 22M							*/
    DRV_RATE_6M         = 6,				/**< 6M								*/
    DRV_RATE_9M         = 7,				/**< 9M								*/	
    DRV_RATE_12M        = 8,				/**< 12M							*/	
    DRV_RATE_18M        = 9,				/**< 18M							*/
    DRV_RATE_24M        = 10,				/**< 24M							*/
    DRV_RATE_36M        = 11,				/**< 36M							*/
    DRV_RATE_48M        = 12,				/**< 48M							*/
    DRV_RATE_54M        = 13,				/**< 54M							*/
    DRV_RATE_MCS_0      = 14,				/**< 6.5M or  7.2					*/ 	
    DRV_RATE_MCS_1      = 15,				/**< 13.0M or 14.4					*/
    DRV_RATE_MCS_2      = 16,				/**< 19.5M or 21.7 					*/
    DRV_RATE_MCS_3      = 17,				/**< 26.0M or 28.9 					*/
    DRV_RATE_MCS_4      = 18,				/**< 39.0M or 43.3 					*/
    DRV_RATE_MCS_5      = 19,				/**< 52.0M or 57.8 					*/
    DRV_RATE_MCS_6      = 20,				/**< 58.5M or 65.0 					*/
    DRV_RATE_MCS_7      = 21,				/**< 65.0M or 72.2 					*/
    DRV_RATE_MAX        = DRV_RATE_MCS_7,	/**< Maximum Driver's Rate Type 	*/
    DRV_RATE_INVALID    = 0xFF				/**< Invalid Driver's Rate Type 	*/

} ERate;

#define RATE_TO_MASK(R)  (1 << ((R) - 1))

/** \enum ERateMask
 * \brief Driver rate mask
 * 
 * \par Description
 * 
 * \sa
 */
typedef enum
{
    DRV_RATE_MASK_AUTO          = DRV_RATE_AUTO,                  /**< 0x000000	*/
    DRV_RATE_MASK_1_BARKER      = RATE_TO_MASK(DRV_RATE_1M),      /**< 0x000001	*/
    DRV_RATE_MASK_2_BARKER      = RATE_TO_MASK(DRV_RATE_2M),      /**< 0x000002	*/
    DRV_RATE_MASK_5_5_CCK       = RATE_TO_MASK(DRV_RATE_5_5M),    /**< 0x000004	*/
    DRV_RATE_MASK_11_CCK        = RATE_TO_MASK(DRV_RATE_11M),     /**< 0x000008	*/
    DRV_RATE_MASK_22_PBCC       = RATE_TO_MASK(DRV_RATE_22M),     /**< 0x000010	*/
    DRV_RATE_MASK_6_OFDM        = RATE_TO_MASK(DRV_RATE_6M),      /**< 0x000020	*/
    DRV_RATE_MASK_9_OFDM        = RATE_TO_MASK(DRV_RATE_9M),      /**< 0x000040	*/
    DRV_RATE_MASK_12_OFDM       = RATE_TO_MASK(DRV_RATE_12M),     /**< 0x000080	*/
    DRV_RATE_MASK_18_OFDM       = RATE_TO_MASK(DRV_RATE_18M),     /**< 0x000100	*/
    DRV_RATE_MASK_24_OFDM       = RATE_TO_MASK(DRV_RATE_24M),     /**< 0x000200	*/
    DRV_RATE_MASK_36_OFDM       = RATE_TO_MASK(DRV_RATE_36M),     /**< 0x000400	*/
    DRV_RATE_MASK_48_OFDM       = RATE_TO_MASK(DRV_RATE_48M),     /**< 0x000800	*/
    DRV_RATE_MASK_54_OFDM       = RATE_TO_MASK(DRV_RATE_54M),     /**< 0x001000	*/
    DRV_RATE_MASK_MCS_0_OFDM    = RATE_TO_MASK(DRV_RATE_MCS_0),   /**< 0x002000	*/
    DRV_RATE_MASK_MCS_1_OFDM    = RATE_TO_MASK(DRV_RATE_MCS_1),   /**< 0x004000	*/
    DRV_RATE_MASK_MCS_2_OFDM    = RATE_TO_MASK(DRV_RATE_MCS_2),   /**< 0x008000	*/
    DRV_RATE_MASK_MCS_3_OFDM    = RATE_TO_MASK(DRV_RATE_MCS_3),   /**< 0x010000	*/
    DRV_RATE_MASK_MCS_4_OFDM    = RATE_TO_MASK(DRV_RATE_MCS_4),   /**< 0x020000	*/
    DRV_RATE_MASK_MCS_5_OFDM    = RATE_TO_MASK(DRV_RATE_MCS_5),   /**< 0x040000	*/
    DRV_RATE_MASK_MCS_6_OFDM    = RATE_TO_MASK(DRV_RATE_MCS_6),   /**< 0x080000	*/
    DRV_RATE_MASK_MCS_7_OFDM    = RATE_TO_MASK(DRV_RATE_MCS_7)    /**< 0x100000	*/

} ERateMask;

#define PBCC_BIT        0x00000080 /* BIT_7 */

#endif /* #define TWDRIVERRATE_H */
