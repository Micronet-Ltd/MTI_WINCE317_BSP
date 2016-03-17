/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
  IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
  OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
  BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
  PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.

  IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
  CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
  SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header describes configuration options and data structures which
    are shared by multiple modules for this project.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: project.h $
    Revision 1.3  2011/04/16 21:11:12Z  garyp
    Documentation clarified -- no functional changes.
    Revision 1.2  2010/07/06 01:35:45Z  garyp
    Updated to work with multiple chip selects.  Added M60 support.
    Revision 1.1  2009/12/19 01:49:18Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#define MAX_CHIPS       (1)     /* Maximum chips supported -- CS0 to CS(MAX_CHIPS-1) */

/*  These settings define the layout of the spare area
*/
#define ECC_OFFSET      (1)     /* Matches CSST default location for 8-bit NAND  */
#define ECC_LEN         (12)    /* 3 bytes for each 512-byte segment in a page */ 
#define FLAGS_OFFSET    (32)    /* Arbitrary */
#define TAG_OFFSET      (38)    /* Arbitrary */


/*  The following two defines are used within the project hooks modules.
    Note that USE_HARDWARE_ECC refers to the OMAP3xx on board hardware
    ECC.  If using M60 flash, set USE_HARDWARE_ECC to FALSE.
*/
#define USE_PREFETCH_MODE         TRUE  /* Must be TRUE for checkin */
#define USE_HARDWARE_ECC          TRUE  /* Must be TRUE for checkin */  


/*  These defines are passed to the NTM code via FFXMICRONPARAMS.
*/
#define SUPPORT_CACHE_MODE_READS  TRUE  /* Must be TRUE for checkin */  
#define SUPPORT_CACHE_MODE_WRITES TRUE  /* Must be TRUE for checkin */  
#define SUPPORT_DUAL_PLANE_OPS    TRUE  /* Must be TRUE for checkin */  


/*  The "M60" Settings are meaningful only if Micron M60 flash is being
    used, and they mark the chip/block boundary at which the M60 on-die
    ECC should be used.  This allows the blocks preceding that boundary
    to use the legacy 1-bit software ECC, as is required by a number
    of platform specific BOOTROMs and vendor flash programming tools.  
    Setting these values both to zero means that the M60 on-die ECC 
    will be used on the entire array (if the flash is M60).
*/
#define M60_EDC4_FIRST_CHIP       (0)   /* Must be 0 for checkin */
#define M60_EDC4_FIRST_BLOCK      (0)   /* Must be 0 for checkin */


struct sNTMHOOK
{
    unsigned            nChip;          /* The current chip select */
    unsigned            nValidChipMask; /* Mask of valid chips per hook instance */
    volatile D_UINT32  *pulGPMCBase;
  #if USE_PREFETCH_MODE  
    volatile D_UINT32  *apFIFO[MAX_CHIPS]; 
    D_BOOL              afFIFOInited[MAX_CHIPS]; 
  #endif
};



