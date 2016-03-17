// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

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
//  File:  cfg.c
//
//  This file implements functions used to load/save EBOOT configuration info.
//
#include <eboot.h>
#include <bsp_logo.h>

//------------------------------------------------------------------------------

BOOL
BLReadBootCfg(
    BOOT_CFG *pBootCfg
    )
{
    //OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLReadBootCfg: Not supported for SD FILE boot\r\n"));
    return FALSE;
}

//------------------------------------------------------------------------------

BOOL
BLWriteBootCfg(
    BOOT_CFG *pBootCfg
    )
{
    OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLWriteBootCfg: Not supported for SD FILE boot\r\n"));
    return FALSE;
}

//------------------------------------------------------------------------------



BOOL
BLReadManufactureCfg(
    BYTE *pManufCfg
    )
{
	OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLReadManufactureCfg: Not supported for SD FILE boot\r\n"));
    return FALSE;
}

//------------------------------------------------------------------------------
BOOL
BLWriteManufactureCfg(
    BYTE *pManufCfg
    )
{
	OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLWriteManufactureCfg: Not supported for SD FILE boot\r\n"));
    return FALSE;
}


//------------------------------------------------------------------------------



BOOL
BLReadSWRights(
    BYTE *pSWRights
    )
{
	OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLWriteSWRights: Not supported for SD FILE boot\r\n"));
    return FALSE;
}

//------------------------------------------------------------------------------
BOOL
BLWriteSWRights(
    BYTE *pSWRights
    )
{
	OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLWriteSWRights: Not supported for SD FILE boot\r\n"));
    return FALSE;
}


//------------------------------------------------------------------------------

BOOL
BLReserveBootBlocks(
    BOOT_CFG *pBootCfg
    )
{
    // Nothing to do...
    return TRUE;
}



//------------------------------------------------------------------------------

BOOL
BLShowLogo(
    )
{
    //  Show the bootloader splashscreen, use -1 for flashaddr to force default display
    ShowLogo(-1, 0);

    return TRUE;
}

//------------------------------------------------------------------------------

