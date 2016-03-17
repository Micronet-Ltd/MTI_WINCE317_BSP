/* --------------------------------------------------------------
 Copyright (c) Micronet LTD. All rights reserved.
 History of Changes:
 Anna R.  03.2009.   File Created.

 File:  bsp_fmd.h

 This file contains platform internal fmd related functions.
 ---------------------------------------------------------------
 */


#ifndef __BSP_FMD_H
#define __BSP_FMD_H


//-----------------------------------------------------------------------------
typedef DWORD (*pfnFMD_ReadSectorNotify)();

//-----------------------------------------------------------------------------

void SetFMDNotify(pfnFMD_ReadSectorNotify fnFMD_ReadSectorNotify);

//-----------------------------------------------------------------------------

#endif

