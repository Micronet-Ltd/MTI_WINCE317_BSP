/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

  In addition to civil penalties for infringement of copyright under appli-
  cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
  of (a) the restrictions on circumvention of copyright protection systems
  found in 17 U.S.C. 1201 and (b) the protections for the integrity of
  copyright management information found in 17 U.S.C. 1202.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
  CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This module contains unit tests for byte order conversion operations.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltend.c $
    Revision 1.10  2009/06/27 22:31:00Z  garyp
    Updated to include dltests.h.
    Revision 1.9  2008/05/29 19:49:17Z  garyp
    Merged from the WinMobile branch.
    Revision 1.8.1.2  2008/05/29 19:49:17Z  garyp
    Removed typecasts which were causing faults on some systems.
    Revision 1.8  2007/11/03 23:31:12Z  Garyp
    Added the standard module header.
    Revision 1.7  2007/10/25 01:26:29Z  brandont
    Corrected warning for different parameter types when calling
    DclCopyLittleToNativeEndian64 when emulating 64-bit types.
    Revision 1.6  2007/09/10 15:50:47Z  peterb
    Updated the byte conversion tests to not pass type cast variables to
    memcpy funtions.
    Revision 1.5  2007/07/31 19:47:46Z  Garyp
    Clarified an ambiguous function name.
    Revision 1.4  2007/05/11 03:02:18Z  Garyp
    Modified to use a macro renamed for clarity.
    Revision 1.3  2006/10/05 23:05:01Z  Garyp
    General cleanup.  Removed unnecessary complexity in dealing with
    emulated 64-bit types.
    Revision 1.2  2006/02/26 20:57:14Z  Garyp
    Minor message cleanup -- nothing functional.
    Revision 1.1  2006/01/03 16:32:28Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include "dltests.h"

#define BE16        0x3587
#define LE16        0x8735
#define LE32HIGH    0xB8A7F6E5UL
#define LE32LOW     0xD4C3B2A1UL
#define BE32HIGH    0xA1B2C3D4UL
#define BE32LOW     0xE5F6A7B8UL


/*-------------------------------------------------------------------
    DclTestByteOrderConversion()

    Description
        This function invokes the unit tests for the byte-order
        conversion routines.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure
-------------------------------------------------------------------*/
DCLSTATUS DclTestByteOrderConversion(void)
{
    D_UINT16        uTestVar16;
    D_UINT32        ulTestVar32;
    D_UINT64        ullTestVar64;
    D_UINT8         ucTestArray[16];
    D_UINT16        uSetIndex;
    D_UINT16        uCheckIndex;
  #if DCL_BIG_ENDIAN
    D_UINT16        uResult16 = BE16;
    D_UINT32        ulResult32 = BE32HIGH;
  #else
    D_UINT16        uResult16 = LE16;
    D_UINT32        ulResult32 = LE32LOW;
  #endif
    D_UINT64        ullResult64;

  #if DCL_BIG_ENDIAN
    DclUint64AssignHiLo(&ullResult64, BE32HIGH, BE32LOW);
  #else
    DclUint64AssignHiLo(&ullResult64, LE32HIGH, LE32LOW);
  #endif

    DclPrintf("Testing byte order conversion...\n");

    /*  Check DclMakeLittleEndian functionality and watch for memory overruns
        or memory alignment issues.
    */
    DclPrintf("    DclMakeLittleEndian with D_UINT16\n");
    for(uSetIndex = 0;
        uSetIndex < (DCLDIMENSIONOF(ucTestArray) - sizeof(D_UINT16));
        uSetIndex++)
    {
        /*  Initialize the test case
        */
        DclMemSet(&ucTestArray[0], 0x77, DCLDIMENSIONOF(ucTestArray));
        uTestVar16 = LE16;
        DclMemCpy(&ucTestArray[uSetIndex], &uTestVar16, sizeof(uTestVar16));

        /*  Validate the conversion
        */
        DclMakeLittleEndian(&ucTestArray[uSetIndex], sizeof(uTestVar16));
        DclMemCpy(&uTestVar16, &ucTestArray[uSetIndex], sizeof(uTestVar16));
        if(uTestVar16 != uResult16)
        {
            DclPrintf("Error: DclMakeLittleEndian malfunction\n");
            return DCLSTAT_CURRENTLINE;
        }

        /*  Check for memory corruption
        */
        for(uCheckIndex = 0; uCheckIndex < uSetIndex; uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclMakeLittleEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
        for(uCheckIndex = uSetIndex + sizeof(uTestVar16);
            uCheckIndex < DCLDIMENSIONOF(ucTestArray); uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclMakeLittleEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
    }


    /*  Check DclMakeLittleEndian functionality and watch for memory overruns
        or memory alignment issues.
    */
    DclPrintf("    DclMakeLittleEndian with D_UINT32\n");
    for(uSetIndex = 0;
        uSetIndex < (DCLDIMENSIONOF(ucTestArray) - sizeof(D_UINT32));
        uSetIndex++)
    {
        /*  Initialize the test case
        */
        DclMemSet(&ucTestArray[0], 0x77, DCLDIMENSIONOF(ucTestArray));
        ulTestVar32 = LE32LOW;
        DclMemCpy(&ucTestArray[uSetIndex], &ulTestVar32, sizeof(ulTestVar32));

        /*  Validate the conversion
        */
        DclMakeLittleEndian(&ucTestArray[uSetIndex], sizeof(ulTestVar32));
        DclMemCpy(&ulTestVar32, &ucTestArray[uSetIndex], sizeof(ulTestVar32));
        if(ulTestVar32 != ulResult32)
        {
            DclPrintf("Error: DclMakeLittleEndian malfunction\n");
            return DCLSTAT_CURRENTLINE;
        }

        /*  Check for memory corruption
        */
        for(uCheckIndex = 0; uCheckIndex < uSetIndex; uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclMakeLittleEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
        for(uCheckIndex = uSetIndex + sizeof(ulTestVar32);
            uCheckIndex < DCLDIMENSIONOF(ucTestArray); uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclMakeLittleEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
    }

    /*  Check DclMakeLittleEndian64 functionality and watch for memory overruns
        or memory alignment issues.
    */
    DclPrintf("    DclMakeLittleEndian64 with D_UINT64\n");
    for(uSetIndex = 0;
        uSetIndex < (DCLDIMENSIONOF(ucTestArray) - sizeof(D_UINT64));
        uSetIndex++)
    {
        /*  Initialize the test case
        */
        DclMemSet(&ucTestArray[0], 0x77, DCLDIMENSIONOF(ucTestArray));

        DclUint64AssignHiLo(&ullTestVar64, LE32HIGH, LE32LOW);
        DclMemCpy(&ucTestArray[uSetIndex], &ullTestVar64, sizeof(ullTestVar64));

        /*  Validate the conversion
        */
        DclMakeLittleEndian64(&ucTestArray[uSetIndex]);
        DclMemCpy(&ullTestVar64, &ucTestArray[uSetIndex], sizeof(ullTestVar64));

        if(!DclUint64IsEqualUint64(&ullTestVar64, &ullResult64))
        {
            DclPrintf("Error: DclMakeLittleEndian64() failed, expected %llX got %llX\n", ullResult64, ullTestVar64);
            return DCLSTAT_CURRENTLINE;
        }

        /*  Check for memory corruption
        */
        for(uCheckIndex = 0; uCheckIndex < uSetIndex; uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclMakeLittleEndian64 memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
        for(uCheckIndex = uSetIndex + sizeof(ullTestVar64);
            uCheckIndex < DCLDIMENSIONOF(ucTestArray); uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclMakeLittleEndian64 memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
    }


    /*  Check DclMakeNativeEndian functionality and watch for memory overruns
        or memory alignment issues.
    */
    DclPrintf("    DclMakeNativeEndian with D_UINT16\n");
    for(uSetIndex = 0;
        uSetIndex < (DCLDIMENSIONOF(ucTestArray) - sizeof(D_UINT16));
        uSetIndex++)
    {
        /*  Initialize the test case
        */
        DclMemSet(&ucTestArray[0], 0x77, DCLDIMENSIONOF(ucTestArray));
        uTestVar16 = LE16;
        DclMemCpy(&ucTestArray[uSetIndex], &uTestVar16, sizeof(uTestVar16));

        /*  Validate the conversion
        */
        DclMakeNativeEndian(&ucTestArray[uSetIndex], sizeof(uTestVar16));
        DclMemCpy(&uTestVar16, &ucTestArray[uSetIndex], sizeof(uTestVar16));
        if(uTestVar16 != uResult16)
        {
            DclPrintf("Error: DclMakeNativeEndian malfunction\n");
            return DCLSTAT_CURRENTLINE;
        }

        /*  Check for memory corruption
        */
        for(uCheckIndex = 0; uCheckIndex < uSetIndex; uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclMakeNativeEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
        for(uCheckIndex = uSetIndex + sizeof(uTestVar16);
            uCheckIndex < DCLDIMENSIONOF(ucTestArray); uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclMakeNativeEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
    }


    /*  Check DclMakeNativeEndian functionality and watch for memory overruns
        or memory alignment issues.
    */
    DclPrintf("    DclMakeNativeEndian with D_UINT32\n");
    for(uSetIndex = 0;
        uSetIndex < (DCLDIMENSIONOF(ucTestArray) - sizeof(D_UINT32));
        uSetIndex++)
    {
        /*  Initialize the test case
        */
        DclMemSet(&ucTestArray[0], 0x77, DCLDIMENSIONOF(ucTestArray));
        ulTestVar32 = LE32LOW;
        DclMemCpy(&ucTestArray[uSetIndex], &ulTestVar32, sizeof(ulTestVar32));

        /*  Validate the conversion
        */
        DclMakeNativeEndian(&ucTestArray[uSetIndex], sizeof(ulTestVar32));
        DclMemCpy(&ulTestVar32, &ucTestArray[uSetIndex], sizeof(ulTestVar32));
        if(ulTestVar32 != ulResult32)
        {
            DclPrintf("Error: DclMakeNativeEndian malfunction\n");
            return DCLSTAT_CURRENTLINE;
        }

        /*  Check for memory corruption
        */
        for(uCheckIndex = 0; uCheckIndex < uSetIndex; uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclMakeNativeEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
        for(uCheckIndex = uSetIndex + sizeof(ulTestVar32);
            uCheckIndex < DCLDIMENSIONOF(ucTestArray); uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclMakeNativeEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
    }


    /*  Check DclMakeNativeEndian64 functionality and watch for memory overruns
        or memory alignment issues.
    */
    DclPrintf("    DclMakeNativeEndian64 with D_UINT64\n");
    for(uSetIndex = 0;
        uSetIndex < (DCLDIMENSIONOF(ucTestArray) - sizeof(D_UINT64));
        uSetIndex++)
    {
        /*  Initialize the test case
        */
        DclMemSet(&ucTestArray[0], 0x77, DCLDIMENSIONOF(ucTestArray));

        DclUint64AssignHiLo(&ullTestVar64, LE32HIGH, LE32LOW);
        DclMemCpy(&ucTestArray[uSetIndex], &ullTestVar64, sizeof(ullTestVar64));

        /*  Validate the conversion
        */
        DclMakeNativeEndian64(&ucTestArray[uSetIndex]);
        DclMemCpy(&ullTestVar64, &ucTestArray[uSetIndex], sizeof(ullTestVar64));

        if(!DclUint64IsEqualUint64(&ullTestVar64, &ullResult64))
        {
            DclPrintf("Error: DclMakeNativeEndian64() failed, expected %llX got %llX\n", ullResult64, ullTestVar64);
            return DCLSTAT_CURRENTLINE;
        }

        /*  Check for memory corruption
        */
        for(uCheckIndex = 0; uCheckIndex < uSetIndex; uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclMakeNativeEndian64 memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
        for(uCheckIndex = uSetIndex + sizeof(ullTestVar64);
            uCheckIndex < DCLDIMENSIONOF(ucTestArray); uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclMakeNativeEndian64 memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
    }


    /*  Check DclCopyLittleToNativeEndian functionality and watch for memory
        overruns or memory alignment issues.
    */
    DclPrintf("    DclCopyLittleToNativeEndian with D_UINT16\n");
    for(uSetIndex = 0;
        uSetIndex < (DCLDIMENSIONOF(ucTestArray) - sizeof(D_UINT16));
        uSetIndex++)
    {
        /*  Initialize the test case
        */
        DclMemSet(&ucTestArray[0], 0x77, DCLDIMENSIONOF(ucTestArray));
        uTestVar16 = LE16;

        /*  Validate the conversion
        */
        DclCopyLittleToNativeEndian(&ucTestArray[uSetIndex], &uTestVar16,
                                    sizeof(uTestVar16));
        DclMemCpy(&uTestVar16, &ucTestArray[uSetIndex], sizeof(uTestVar16));
        if(uTestVar16 != uResult16)
        {
            DclPrintf("Error: DclCopyLittleToNativeEndian malfunction\n");
            return DCLSTAT_CURRENTLINE;
        }

        /*  Check for memory corruption
        */
        for(uCheckIndex = 0; uCheckIndex < uSetIndex; uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclCopyLittleToNativeEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
        for(uCheckIndex = uSetIndex + sizeof(uTestVar16);
            uCheckIndex < DCLDIMENSIONOF(ucTestArray); uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclCopyLittleToNativeEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
    }


    /*  Check DclCopyLittleToNativeEndian functionality and watch for memory
        overruns or memory alignment issues.
    */
    DclPrintf("    DclCopyLittleToNativeEndian with D_UINT32\n");
    for(uSetIndex = 0;
        uSetIndex < (DCLDIMENSIONOF(ucTestArray) - sizeof(D_UINT32));
        uSetIndex++)
    {
        /*  Initialize the test case
        */
        DclMemSet(&ucTestArray[0], 0x77, DCLDIMENSIONOF(ucTestArray));
        ulTestVar32 = LE32LOW;

        /*  Validate the conversion
        */
        DclCopyLittleToNativeEndian(&ucTestArray[uSetIndex], &ulTestVar32,
                                    sizeof(ulTestVar32));
        DclMemCpy(&ulTestVar32, &ucTestArray[uSetIndex], sizeof(ulTestVar32));
        if(ulTestVar32 != ulResult32)
        {
            DclPrintf("Error: DclCopyLittleToNativeEndian malfunction\n");
            return DCLSTAT_CURRENTLINE;
        }

        /*  Check for memory corruption
        */
        for(uCheckIndex = 0; uCheckIndex < uSetIndex; uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclCopyLittleToNativeEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
        for(uCheckIndex = uSetIndex + sizeof(ulTestVar32);
            uCheckIndex < DCLDIMENSIONOF(ucTestArray); uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclCopyLittleToNativeEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
    }


    /*  Check DclCopyLittleToNativeEndian64 functionality and watch for memory
        overruns or memory alignment issues.
    */
    DclPrintf("    DclCopyLittleToNativeEndian64 with D_UINT64\n");
    for(uSetIndex = 0;
        uSetIndex < (DCLDIMENSIONOF(ucTestArray) - sizeof(D_UINT64));
        uSetIndex++)
    {
        /*  Initialize the test case
        */
        DclMemSet(&ucTestArray[0], 0x77, DCLDIMENSIONOF(ucTestArray));

        DclUint64AssignHiLo(&ullTestVar64, LE32HIGH, LE32LOW);

        /*  Validate the conversion
        */
        DclCopyLittleToNativeEndian64(&ucTestArray[uSetIndex], (const D_BUFFER*)&ullTestVar64);
        DclMemCpy(&ullTestVar64, &ucTestArray[uSetIndex], sizeof(ullTestVar64));

        if(!DclUint64IsEqualUint64(&ullTestVar64, &ullResult64))
        {
            DclPrintf("Error: DclCopyLittleToNativeEndian64() failed, expected %llX got %llX\n", ullResult64, ullTestVar64);
            return DCLSTAT_CURRENTLINE;
        }

        /*  Check for memory corruption
        */
        for(uCheckIndex = 0; uCheckIndex < uSetIndex; uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclCopyLittleToNativeEndian64 memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
        for(uCheckIndex = uSetIndex + sizeof(ullTestVar64);
            uCheckIndex < DCLDIMENSIONOF(ucTestArray); uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclCopyLittleToNativeEndian64 memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
    }


    /*  Check DclCopyNativeToLittleEndian functionality and watch for memory
        overruns or memory alignment issues.
    */
    DclPrintf("    DclCopyNativeToLittleEndian with D_UINT16\n");
    for(uSetIndex = 0;
        uSetIndex < (DCLDIMENSIONOF(ucTestArray) - sizeof(D_UINT64));
        uSetIndex++)
    {
        /*  Initialize the test case
        */
        DclMemSet(&ucTestArray[0], 0x77, DCLDIMENSIONOF(ucTestArray));
        uTestVar16 = LE16;

        /*  Validate the conversion
        */
        DclCopyNativeToLittleEndian(&ucTestArray[uSetIndex], &uTestVar16,
                                    sizeof(uTestVar16));
        DclMemCpy(&uTestVar16, &ucTestArray[uSetIndex], sizeof(uTestVar16));
        if(uTestVar16 != uResult16)
        {
            DclPrintf("Error: DclCopyNativeToLittleEndian malfunction\n");
            return DCLSTAT_CURRENTLINE;
        }

        /*  Check for memory corruption
        */
        for(uCheckIndex = 0; uCheckIndex < uSetIndex; uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclCopyNativeToLittleEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
        for(uCheckIndex = uSetIndex + sizeof(uTestVar16);
            uCheckIndex < DCLDIMENSIONOF(ucTestArray); uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclCopyNativeToLittleEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
    }


    /*  Check DclCopyNativeToLittleEndian functionality and watch for memory
        overruns or memory alignment issues.
    */
    DclPrintf("    DclCopyNativeToLittleEndian with D_UINT32\n");
    for(uSetIndex = 0;
        uSetIndex < (DCLDIMENSIONOF(ucTestArray) - sizeof(D_UINT32));
        uSetIndex++)
    {
        /*  Initialize the test case
        */
        DclMemSet(&ucTestArray[0], 0x77, DCLDIMENSIONOF(ucTestArray));
        ulTestVar32 = LE32LOW;

        /*  Validate the conversion
        */
        DclCopyNativeToLittleEndian(&ucTestArray[uSetIndex], &ulTestVar32,
                                    sizeof(ulTestVar32));
        DclMemCpy(&ulTestVar32, &ucTestArray[uSetIndex], sizeof(ulTestVar32));
        if(ulTestVar32 != ulResult32)
        {
            DclPrintf("Error: DclCopyNativeToLittleEndian malfunction\n");
            return DCLSTAT_CURRENTLINE;
        }

        /*  Check for memory corruption
        */
        for(uCheckIndex = 0; uCheckIndex < uSetIndex; uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclCopyNativeToLittleEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
        for(uCheckIndex = uSetIndex + sizeof(ulTestVar32);
            uCheckIndex < DCLDIMENSIONOF(ucTestArray); uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclCopyNativeToLittleEndian memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
    }


    /*  Check DclCopyNativeToLittleEndian64 functionality and watch for memory
        overruns or memory alignment issues.
    */
    DclPrintf("    DclCopyNativeToLittleEndian64 with D_UINT64\n");
    for(uSetIndex = 0;
        uSetIndex < (DCLDIMENSIONOF(ucTestArray) - sizeof(D_UINT64));
        uSetIndex++)
    {
        /*  Initialize the test case
        */
        DclMemSet(&ucTestArray[0], 0x77, DCLDIMENSIONOF(ucTestArray));

        DclUint64AssignHiLo(&ullTestVar64, LE32HIGH, LE32LOW);

        /*  Validate the conversion
        */
        DclCopyNativeToLittleEndian64(&ucTestArray[uSetIndex], &ullTestVar64);
        DclMemCpy(&ullTestVar64, &ucTestArray[uSetIndex], sizeof(ullTestVar64));

        if(!DclUint64IsEqualUint64(&ullTestVar64, &ullResult64))
        {
            DclPrintf("Error: DclCopyNativeToLittleEndian64() failed, expected %llX got %llX\n", ullResult64, ullTestVar64);
            return DCLSTAT_CURRENTLINE;
        }

        /*  Check for memory corruption
        */
        for(uCheckIndex = 0; uCheckIndex < uSetIndex; uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclCopyNativeToLittleEndian64 memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
        for(uCheckIndex = uSetIndex + sizeof(ullTestVar64);
            uCheckIndex < DCLDIMENSIONOF(ucTestArray); uCheckIndex++)
        {
            if(ucTestArray[uCheckIndex] != 0x77)
            {
                DclPrintf("Error: DclCopyNativeToLittleEndian64 memory corruption\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
    }


    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}

