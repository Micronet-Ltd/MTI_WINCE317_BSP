/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

    This module contains unit tests for date/time operations.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltdate.c $
    Revision 1.10  2010/01/23 16:45:49Z  garyp
    Fixed to avoid "possible loss of data" warnings (no actual loss of data).
    Revision 1.9  2009/06/27 22:31:00Z  garyp
    Updated to include dltests.h.
    Revision 1.8  2007/11/03 23:31:12Z  Garyp
    Added the standard module header.
    Revision 1.7  2007/08/06 19:25:08Z  jimmb
    Fixed the indentation around the latest change.
    Revision 1.6  2007/08/06 16:37:18Z  jimmb
    Move the declaration and initialization of  dtPrevious outside
    the "for (uYear...)" loop.
    Revision 1.5  2007/08/03 17:11:50Z  jimmb
    Used the MACRO to define the D_TIME variable.
    Revision 1.4  2007/08/02 17:38:36Z  jimmb
    Added variable initialization.
    Revision 1.3  2007/05/11 03:01:52Z  Garyp
    Modified to use a macro renamed for clarity.
    Revision 1.2  2007/02/27 23:49:29Z  Garyp
    Updated to properly calculate leap years for 100 and 400 year boundaries,
    as well as to handle everything up to the year 10000.
    Revision 1.1  2006/02/03 01:02:50Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include "dltests.h"

#define MAXTESTYEAR             (10001)
#define SECONDS_PER_DAY         (60 * 60 * 24)
#define MICROSECS_PER_SECOND    (1000000)

#define FEBRUARY    (1)         /*  Month number relative to zero */

static const D_UINT8 uDaysPerMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*  Leap years occur every 4 years, but not at 100 year boundaries,
    unless it is a 400 year boundary.  It has been argued that every
    4000 years should again not be a leap year, but likely that dabate
    will not be settled until long after this software is obsolete.
    Therefore, the year 2000 is a leap year, but the year 2100 is not.
*/
#define ISLEAPYEAR(y) ( (!((y) % 4)) &&     \
                        ( (!((y) % 400)) || ((y) % 100) ) ? TRUE : FALSE )


/*-------------------------------------------------------------------
    DclTestDateTime()

    Description
        This function invokes the unit tests for the date/time
        routines.

    Parameters
        None

    Return Value
        Returns a DCLSTATUS value indicating success or failure
-------------------------------------------------------------------*/
DCLSTATUS DclTestDateTime(void)
{
    D_TIME          ullTime;
    D_UINT16        uIndex;
    D_UINT64        ullMicrosecsPerDay;
    DCLDATETIME     dtNew;
    static const DCLDATETIME adtTestCases[] =
    { /* yyyy  mm  dd  hh  mm  ss   ms   us  */
        {1970,  0,  1,  0,  0,  0,   0,   0},
        {1970, 11, 31,  0,  0,  0,   0,   0},
        {1971,  0,  1,  0,  0,  0,   0,   0},
        {1971, 11, 31,  0,  0,  0,   0,   0},
        {1972,  0,  1,  0,  0,  0,   0,   0},
        {1972, 11, 31,  0,  0,  0,   0,   0},
        {1973,  0,  1,  0,  0,  0,   0,   0},
        {1973, 11, 31,  0,  0,  0,   0,   0},
        {1999,  0,  1,  0,  0,  0,   0,   0},
        {1999, 11, 31,  0,  0,  0,   0,   0},
        {2000,  0,  1,  0,  0,  0,   0,   0},
        {2000, 11, 31,  0,  0,  0,   0,   0},
        {2001,  0,  1,  0,  0,  0,   0,   0},
        {2001, 11, 31,  0,  0,  0,   0,   0},
        {2099,  0,  1,  0,  0,  0,   0,   0},
        {2099, 11, 31,  0,  0,  0,   0,   0},
        {2100,  0,  1,  0,  0,  0,   0,   0},
        {2100, 11, 31,  0,  0,  0,   0,   0},
        {2101,  0,  1,  0,  0,  0,   0,   0},
        {2101, 11, 31,  0,  0,  0,   0,   0},
        {1970,  1,  1,  0,  0,  0,   0,   0},
        {1978,  3, 28, 19, 37, 14,   0, 999},
        {1975,  1,  1,  1,  1,  2, 999,   0},
        {1970, 11, 30, 22, 33, 26, 999, 999},
        {1974,  3,  1,  1,  1,  2,   0,   1},
        {1974,  2, 28,  1,  1,  2,   1,   0},
        {1974,  2, 29,  1,  1,  2,   1,   1},
        {1994,  8, 23, 18, 30,  0,   1, 999},
        {1999,  1,  1,  0,  0,  0, 999,   1},
        {1999,  3, 28, 19, 37, 14,   0,   0},
        {1999,  1,  1,  1,  1,  2,   0,   0},
        {1999, 11, 30, 22, 33, 26,   0,   0},
        {1999,  3,  1,  1,  1,  2,   0,   0},
        {1999,  2, 28,  1,  1,  2,   0,   0},
        {1999,  2, 29,  1,  1,  2,   0,   0},
        {1999,  8, 23, 18, 30,  0,   0,   0},
        {2000,  1,  1,  0,  0,  0,   0,   0},
        {2000,  3, 28, 19, 37, 14,   0,   0},
        {2000,  1,  1,  1,  1,  2,   0,   0},
        {2000, 11, 30, 22, 33, 26,   0,   0},
        {2000,  3,  1,  1,  1,  2,   0,   0},
        {2000,  2, 28,  1,  1,  2,   0,   0},
        {2000,  2, 29,  1,  1,  2,   0,   0},
        {2000,  8, 23, 18, 30,  0,   0,   0},
        {2001,  1,  1,  0,  0,  0,   0,   0},
        {2001,  3, 28, 19, 37, 14,   0,   0},
        {2001,  1,  1,  1,  1,  2,   0,   0},
        {2001, 11, 30, 22, 33, 26,   0,   0},
        {2001,  3,  1,  1,  1,  2,   0,   0},
        {2001,  2, 28,  1,  1,  2,   0,   0},
        {2001,  2, 29,  1,  1,  2,   0,   0},
        {2001,  8, 23, 18, 30,  0,   0,   0},
        {1970,  1,  1,  0,  0,  0,   0,   0},
        {1970,  1,  1,  0,  0,  1,   0,   0},
        {1970,  1,  1,  0,  1,  0,   0,   0},
        {1970,  1,  1,  0,  1,  1,   0,   0},
        {1970,  1,  1,  1,  0,  0,   0,   0},
        {1970,  1,  1,  1,  0,  1,   0,   0},
        {1970,  1,  1,  1,  1,  0,   0,   0},
        {1970,  1,  1,  1,  1,  1,   0,   0},
        {1970,  1,  1,  0,  0, 59,   0,   0},
        {1970,  1,  1,  0, 59,  0,   0,   0},
        {1970,  1,  1,  0, 59, 59,   0,   0},
        {1970,  1,  1, 23,  0,  0,   0,   0},
        {1970,  1,  1, 23,  0, 59,   0,   0},
        {1970,  1,  1, 23, 59,  0,   0,   0},
        {1970,  1,  1, 23, 59, 59,   0,   0}
    };
    static const D_UINT16   uNumCases = DCLDIMENSIONOF(adtTestCases);
    static const D_UINT16   auLeapYears[] = { 1972, 1996, 2000, 2004, 2096, 2104, 2396, 2400, 2404};
    static const D_UINT16   auNonLeapYears[] = { 1970, 1999, 2001, 2099, 2100, 2101, 2399, 2401};

    /*  Test converting between encoded and decoded time.
    */
    DclPrintf("Testing date and time...\n");

    DclUint64AssignUint32(&ullMicrosecsPerDay, SECONDS_PER_DAY);
    DclUint64MulUint32(&ullMicrosecsPerDay, MICROSECS_PER_SECOND);

    /*  Test converting between encoded and decoded time.
    */
    DclPrintf("    Validating conversion between encoded and decoded forms\n");
    for (uIndex = 0; uIndex < uNumCases; uIndex++)
    {
        DclDateTimeEncode(&adtTestCases[uIndex], &ullTime);
        DclDateTimeDecode(&ullTime, &dtNew);

        if (dtNew.uYear        != adtTestCases[uIndex].uYear         ||
            dtNew.uMonth       != adtTestCases[uIndex].uMonth        ||
            dtNew.uDay         != adtTestCases[uIndex].uDay          ||
            dtNew.uHour        != adtTestCases[uIndex].uHour         ||
            dtNew.uMinute      != adtTestCases[uIndex].uMinute       ||
            dtNew.uSecond      != adtTestCases[uIndex].uSecond       ||
            dtNew.uMillisecond != adtTestCases[uIndex].uMillisecond  ||
            dtNew.uMicrosecond != adtTestCases[uIndex].uMicrosecond)
        {
            DclPrintf("Test %U Failed\n", uIndex);
            return DCLSTAT_CURRENTLINE;
        }
    }

    /*  Test leap years
    */
    DclPrintf("    Validating leap years\n");
    for (uIndex = 0; uIndex < DCLDIMENSIONOF(auLeapYears); uIndex++)
    {
        DCLDATETIME dt = { 0,  1, 28,  0,  0,  0,   0,   0};
        D_TIME      ullYear1;
        D_TIME      ullYear2;
        D_TIME      ullYear3;

        dt.uYear = auLeapYears[uIndex];

        if (!DclDateTimeEncode(&dt, &ullYear1))
        {
            DclPrintf("Encoding 2/28 failed!  Test %U, year %U\n", uIndex, auLeapYears[uIndex]);
            return DCLSTAT_CURRENTLINE;
        }

        dt.uDay = 29;
        if (!DclDateTimeEncode(&dt, &ullYear2))
        {
            DclPrintf("Encoding 2/29 failed!  Test %U, year %U\n", uIndex, auLeapYears[uIndex]);
            return DCLSTAT_CURRENTLINE;
        }

        DclUint64AddUint64(&ullYear1, &ullMicrosecsPerDay);
        if (!DclUint64IsEqualUint64(&ullYear1, &ullYear2))
        {
            DclPrintf("Test 2/29 failed!  Test %U, year %U\n", uIndex, auLeapYears[uIndex]);
            return DCLSTAT_CURRENTLINE;
        }

        /*  Change to 3/1 (Months are relative to 0, but days
            are relative to 1).
        */
        dt.uMonth = 2;
        dt.uDay = 1;
        if (!DclDateTimeEncode(&dt, &ullYear3))
        {
            DclPrintf("Encoding 3/1 failed!  Test %U, year %U\n", uIndex, auLeapYears[uIndex]);
            return DCLSTAT_CURRENTLINE;
        }

        DclUint64AddUint64(&ullYear2, &ullMicrosecsPerDay);
        if (!DclUint64IsEqualUint64(&ullYear2, &ullYear3))
        {
            DclPrintf("Test 3/1 failed!  Test %U, year %U\n", uIndex, auLeapYears[uIndex]);
            return DCLSTAT_CURRENTLINE;
        }
    }

    /*  Test non-leap years
    */
    DclPrintf("    Validating non-leap years\n");
    for (uIndex = 0; uIndex < DCLDIMENSIONOF(auNonLeapYears); uIndex++)
    {
        DCLDATETIME dt = {0,  1, 28,  0,  0,  0,   0,   0};
        D_TIME      ullYear1;
        D_TIME      ullYear2;

        dt.uYear = auNonLeapYears[uIndex];

        if (!DclDateTimeEncode(&dt, &ullYear1))
        {
            DclPrintf("Encoding 2/28 failed!  Test %U, year %U\n", uIndex, auNonLeapYears[uIndex]);
            return DCLSTAT_CURRENTLINE;
        }

        /*  Change to 3/1 (Months are relative to 0, but days
            are relative to 1).
        */
        dt.uMonth = 2;
        dt.uDay = 1;
        if (!DclDateTimeEncode(&dt, &ullYear2))
        {
            DclPrintf("Encoding 3/1 failed!  Test %U, year %U\n", uIndex, auNonLeapYears[uIndex]);
            return DCLSTAT_CURRENTLINE;
        }

        DclUint64AddUint64(&ullYear1, &ullMicrosecsPerDay);
        if (!DclUint64IsEqualUint64(&ullYear1, &ullYear2))
        {
            DclPrintf("Test failed!  Test %U, year %U\n", uIndex, auNonLeapYears[uIndex]);
            return DCLSTAT_CURRENTLINE;
        }
    }

    DclPrintf("    Encoding and decoding every day from 1970 to %u\n", MAXTESTYEAR);
    {
        unsigned    fFirstYear = TRUE;
        D_UINT16    uYear;
        D_TIME      dtPrevious;

        DclUint64AssignUint32(&dtPrevious,0);

        for (uYear = 1970; uYear <= MAXTESTYEAR; uYear++)
        {
            D_UINT8     uMonth;

            for (uMonth = 0; uMonth < 12; uMonth++)
            {
                D_UINT16    uMaxDay;
                D_UINT8     uDay;

                if (uMonth == FEBRUARY && ISLEAPYEAR(uYear))
                    uMaxDay = uDaysPerMonth[FEBRUARY] + 1;
                else
                    uMaxDay = uDaysPerMonth[uMonth];

                for (uDay = 1; uDay <= uMaxDay; uDay++)
                {
                    DCLDATETIME dtCompare;
                    DCLDATETIME dt = {0, 0, 0, 0, 0, 0, 0, 0};
                    D_TIME      dtEncoded;

                    dt.uYear = uYear;
                    dt.uMonth = uMonth;
                    dt.uDay = uDay;

                    if (!DclDateTimeEncode(&dt, &dtEncoded))
                    {
                        DclPrintf("Encoding failed!  Year/Month/Day %U/%U/%U\n",
                                  dt.uYear, dt.uMonth+1, dt.uDay);
                        return DCLSTAT_CURRENTLINE;
                    }

                    if (fFirstYear)
                    {
                        DclUint64AssignHiLo(&dtPrevious, 0, 0);

                        if (!DclUint64IsEqualUint64(&dtEncoded, &dtPrevious))
                        {
                            DclPrintf("First year encoding invalid!\n");
                            return DCLSTAT_CURRENTLINE;
                        }

                        fFirstYear = FALSE;
                    }
                    else
                    {
                        DclUint64AddUint64(&dtPrevious, &ullMicrosecsPerDay);
                        if (!DclUint64IsEqualUint64(&dtEncoded, &dtPrevious))
                        {
                            DclPrintf("Progression failed!  Year/Month/Day %U/%U/%U\n",
                                      dt.uYear, dt.uMonth+1, dt.uDay);
                            return DCLSTAT_CURRENTLINE;
                        }
                    }

                    if (!DclDateTimeDecode(&dtEncoded, &dtCompare))
                    {
                        DclPrintf("Decoding failed!  Year/Month/Day %U/%U/%U\n",
                                  dt.uYear, dt.uMonth+1, dt.uDay);
                        return DCLSTAT_CURRENTLINE;
                    }

                    if (dt.uYear        != dtCompare.uYear           ||
                        dt.uMonth       != dtCompare.uMonth          ||
                        dt.uDay         != dtCompare.uDay            ||
                        dt.uHour        != dtCompare.uHour           ||
                        dt.uMinute      != dtCompare.uMinute         ||
                        dt.uSecond      != dtCompare.uSecond         ||
                        dt.uMillisecond != dtCompare.uMillisecond    ||
                        dt.uMicrosecond != dtCompare.uMicrosecond)
                    {
                        DclPrintf("Encode/decode comparison failed!  Year/Month/Day %U/%U/%U\n",
                                  dt.uYear, dt.uMonth+1, dt.uDay);
                        return DCLSTAT_CURRENTLINE;
                    }
                }
            }
        }
    }

    /*  all tests passed
    */
    DclPrintf("    OK\n");
    return DCLSTAT_SUCCESS;
}

