/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

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

    This module contains routines for encoding and decoding date/time values.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dldate.c $
    Revision 1.13  2010/12/22 00:03:29Z  garyp
    Documentation corrections -- no functional changes.
    Revision 1.12  2010/08/29 19:49:01Z  garyp
    Added DclTimeFormatUS().
    Revision 1.11  2010/08/04 00:10:31Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.10  2009/06/27 23:24:47Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.9  2009/04/10 02:00:51Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.8  2007/12/18 20:17:26Z  brandont
    Updated function headers.
    Revision 1.7  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.6  2007/10/30 21:10:19Z  Garyp
    Added DclTimeFormat().
    Revision 1.5  2007/10/01 19:05:26Z  pauli
    Rearranged the order of type qualifiers.
    Revision 1.4  2007/02/19 23:09:32Z  Garyp
    Added a number of new tests.
    Revision 1.3  2006/10/05 23:44:29Z  Garyp
    Modified to use a corrected macro name.
    Revision 1.2  2006/05/28 22:19:11Z  Garyp
    Corrected a typo in the month names.  Minor cleanup -- nothing functional.
    Revision 1.1  2006/02/08 00:10:56Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

static D_BOOL ValidateDateTime(const DCLDATETIME *pDateTime);

#define SECONDS_PER_MINUTE      (60)
#define SECONDS_PER_HOUR        (SECONDS_PER_MINUTE * 60)
#define SECONDS_PER_DAY         (SECONDS_PER_HOUR * 24)
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

#define LEAPDAYS_BEFORE_YEAR(y)     (((y) / 4) - ((y) / 100) + ((y) / 400))

#define BASELINE_YEAR               (1970)
#define BASELINE_LEAPDAYS           (LEAPDAYS_BEFORE_YEAR(BASELINE_YEAR))

#define DAYS_PER_YEAR               (365)
#define DAYS_PER_PRESIDENT          ((DAYS_PER_YEAR * 4) + 1)
#define DAYS_PER_CENTURY            ((DAYS_PER_PRESIDENT * 25) - 1)
#define DAYS_PER_QUADCENTURY        ((DAYS_PER_CENTURY * 4) + 1)

#define DAYS_BEFORE_BASELINE_YEAR   (((BASELINE_YEAR-1) * DAYS_PER_YEAR) + BASELINE_LEAPDAYS)


/*-------------------------------------------------------------------
    Public: DclDateTimeString()

    Convert a date/time to a human readable string.

    The output is in the following format

    <pre>
        Jul 05 13:08:27 2003
    </pre>

    If the pTime parameter is NULL, the current date/time is used.

    Parameters:
        pTime        - A pointer to the raw date/time value
        pBuffer      - A buffer to receive the formatted string
        nBuffLen     - The size of the buffer

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL DclDateTimeString(
    const D_TIME       *pTime,
    char               *pBuffer,
    size_t              nBuffLen)
{
    int                 iResult;
    D_TIME              ullTime;
    DCLDATETIME         sDateTime;
    static const char   szMonths[][4] = {"Jan", "Feb", "Mar", "Apr",
                                         "May", "Jun", "Jul", "Aug",
                                         "Sep", "Oct", "Nov", "Dec"};
    /*  Validate the parameters
    */
    if(!pBuffer)
    {
        DclError();
        return FALSE;
    }

    /*  Use the current time if one is not given.
    */
    if(!pTime)
    {
        DclOsGetDateTime(&ullTime);
        pTime = &ullTime;
    }

    /*  Convert the time
    */
    DclDateTimeDecode(pTime, &sDateTime);

    /*  format the date/time string
    */
    iResult = DclSNPrintf(pBuffer, nBuffLen, "%s %02u %02u:%02u:%02u %04u",
                          szMonths[sDateTime.uMonth], sDateTime.uDay,
                          sDateTime.uHour, sDateTime.uMinute,
                          sDateTime.uSecond, sDateTime.uYear);
    if(iResult < 0)
        return FALSE;
    else
        return TRUE;
}


/*-------------------------------------------------------------------
    Public: DclDateTimeDecode()

    Decode a date/time value to the individual parts -- year, month,
    day, hour, minute, second, millisecond, and microsecond.

    Parameters:
        pTime        - A pointer to the raw date/time value
        pDateTime    - A buffer to receive the decoded date/time

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL DclDateTimeDecode(
    const D_TIME   *pTime,
    DCLDATETIME    *pDateTime)
{
    D_UINT64        ullTime;
    D_UINT64        ullDays;
    D_UINT32        ulYears = 0;
    D_UINT32        ulTotalDays;
    D_UINT32        ulTotalSeconds;
    D_UINT32        ulSeconds;
    D_UINT32        ulLowMicro;
    D_UINT8         uIndex;

    /*  Validate the parameters
    */
    DclAssert(pTime);
    DclAssert(pDateTime);
    if(!pTime || !pDateTime)
    {
        return FALSE;
    }

    /*  Grab the microseconds and milliseconds values now, before
        scaling the time value down to seconds.
    */
    ullTime = *pTime;
    DclUint64ModUint32(&ullTime, 1000000);
    ulLowMicro = DclUint32CastUint64(&ullTime);
    pDateTime->uMicrosecond = (D_UINT16)(ulLowMicro % 1000);
    ulLowMicro /= 1000;
    pDateTime->uMillisecond = (D_UINT16)(ulLowMicro % 1000);

    /*  Convert microseconds into seconds
    */
    DclUint64AssignUint64(&ullTime, pTime);
    DclUint64DivUint32(&ullTime, 1000000);

    /*  Calculate the number of full days
    */
    DclUint64AssignUint64(&ullDays, &ullTime);
    DclUint64DivUint32(&ullDays, SECONDS_PER_DAY);
    ulTotalDays = DclUint32CastUint64(&ullDays);

    /*  Calculate the seconds into the current day
    */
    DclUint64ModUint32(&ullTime, SECONDS_PER_DAY);
    ulTotalSeconds = DclUint32CastUint64(&ullTime);

    /*  Make total days relative to January 1, year 1
    */
    ulTotalDays += DAYS_BEFORE_BASELINE_YEAR;

    if(ulTotalDays >= DAYS_PER_QUADCENTURY)
    {
        D_UINT32    ulQuadCenturies = ulTotalDays / DAYS_PER_QUADCENTURY;

        ulYears += (ulQuadCenturies * 400);
        ulTotalDays -= (ulQuadCenturies * DAYS_PER_QUADCENTURY);
    }

    if(ulTotalDays >= DAYS_PER_CENTURY)
    {
        D_UINT32    ulCenturies = ulTotalDays / DAYS_PER_CENTURY;

        if(ulCenturies == 4)
        {
            DclAssert(ulTotalDays == DAYS_PER_QUADCENTURY-1);
            ulCenturies--;
        }

        ulYears += (ulCenturies * 100);
        ulTotalDays -= (ulCenturies * DAYS_PER_CENTURY);
    }

    if(ulTotalDays >= DAYS_PER_PRESIDENT)
    {
        D_UINT32    ulPresidents = ulTotalDays / DAYS_PER_PRESIDENT;

        ulYears += (ulPresidents * 4);
        ulTotalDays -= (ulPresidents * DAYS_PER_PRESIDENT);
    }

    if(ulTotalDays >= DAYS_PER_YEAR)
    {
        D_UINT32    ulYrs = ulTotalDays / DAYS_PER_YEAR;

        if(ulYrs == 4)
        {
            DclAssert(ulTotalDays == DAYS_PER_PRESIDENT-1);
            ulYrs--;
        }

        ulYears += ulYrs;
        ulTotalDays -= (ulYrs * DAYS_PER_YEAR);
    }

    /*  No year 0
    */
    ulYears++;

    DclAssert(ulYears >= BASELINE_YEAR);
    DclAssert(ulTotalDays < DAYS_PER_YEAR + 1);
    DclAssert(ISLEAPYEAR(ulYears) || ulTotalDays < DAYS_PER_YEAR);

    pDateTime->uYear = (D_UINT16)ulYears;

    /*  Calculate the month
    */
    for(uIndex = 0; uIndex < 12; uIndex++)
    {
        D_UINT8 uMonthDays = uDaysPerMonth[uIndex];

        /*  For February, if it is a leap year, adjust...
        */
        if((uIndex == FEBRUARY) && ISLEAPYEAR(pDateTime->uYear))
            uMonthDays++;

        if(ulTotalDays < uMonthDays)
        {
            /*  Found the proper month...
            */
            pDateTime->uMonth = uIndex;
            pDateTime->uDay = (D_UINT8)(ulTotalDays + 1);
            break;
        }

        /*  Keep subtracting days until we find the right month
        */
        ulTotalDays -= uMonthDays;
    }

    /*  Calculate the seconds in the current day
    */
    if(ulTotalSeconds > 0)
        ulSeconds = ulTotalSeconds % SECONDS_PER_DAY;
    else
        ulSeconds = 0;

    /*  determine hour, minute and second for this day
    */
    if(ulSeconds > 0)
    {
        pDateTime->uHour = (D_UINT8)(ulSeconds / SECONDS_PER_HOUR);
        ulSeconds -= pDateTime->uHour * SECONDS_PER_HOUR;
    }
    else
    {
        pDateTime->uHour = 0;
    }

    if(ulSeconds > 0)
    {
        pDateTime->uMinute = (D_UINT8)(ulSeconds / 60);
        ulSeconds -= pDateTime->uMinute * 60;
    }
    else
    {
        pDateTime->uMinute = 0;
    }

    if(ulSeconds > 0)
        pDateTime->uSecond = (D_UINT8)(ulSeconds);
    else
        pDateTime->uSecond = 0;

    /*  Validate the decoded time (should not be necessary to do this
        validation for normal operations, since any D_TIME value should
        be able to be properly decoded).
    */
    DclAssert(ValidateDateTime(pDateTime));

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: DclDateTimeEncode()

    Encode a date/time value from the individual parts -- year, month,
    day, hour, minute, second, millisecond, and microsecond.

    Parameters:
        pDateTime    - A pointer to the date/time in component form
        pTime        - A buffer to receive the encoded date/time

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL DclDateTimeEncode(
    const DCLDATETIME  *pDateTime,
    D_TIME             *pTime)
{
    D_UINT32            ulTotalDays = 0;
    D_UINT32            ulTotalSeconds;
    D_UINT32            ulExtraMicros;
    D_UINT8             uIndex;
    D_UINT16            uYear;

    /*  Validate the parameters
    */
    DclAssert(pDateTime);
    DclAssert(pTime);
    if(!pDateTime || !pTime || !ValidateDateTime(pDateTime))
    {
        return FALSE;
    }

    uYear = pDateTime->uYear - 1;

    if(uYear >= 400)
    {
        ulTotalDays += ((uYear / 400) * DAYS_PER_QUADCENTURY);
        uYear %= 400;
    }

    if(uYear >= 100)
    {
        ulTotalDays += ((uYear / 100) * DAYS_PER_CENTURY);
        uYear %= 100;
    }

    if(uYear >= 4)
    {
        ulTotalDays += ((uYear / 4) * DAYS_PER_PRESIDENT);
        uYear %= 4;
    }

    if(uYear >= 1)
    {
        ulTotalDays += (uYear * DAYS_PER_YEAR);
    }

    DclAssert(ulTotalDays >= DAYS_BEFORE_BASELINE_YEAR);

    ulTotalDays -= DAYS_BEFORE_BASELINE_YEAR;

    for(uIndex = 0; uIndex < pDateTime->uMonth; uIndex++)
    {
        ulTotalDays += uDaysPerMonth[uIndex];

        /*  For February, if it is a leap year, set 29 days
        */
        if((uIndex == FEBRUARY) && ISLEAPYEAR(pDateTime->uYear))
            ulTotalDays++;
    }

    ulTotalDays += pDateTime->uDay - 1;

    /*  Determine the number of seconds up to this day
    */
    DclUint64AssignUint32(pTime, ulTotalDays);
    DclUint64MulUint32(pTime, SECONDS_PER_DAY);

    /*  Add the number of seconds for this day
    */
    ulTotalSeconds = (pDateTime->uHour * SECONDS_PER_HOUR) +
                      (pDateTime->uMinute * 60) + pDateTime->uSecond;

    DclUint64AddUint32(pTime, ulTotalSeconds);

    /*  Convert 32-bit seconds into 64-bit microseconds
    */
    DclUint64MulUint32(pTime, 1000000);

    /*  Add in the millisecond and microsecond portions
    */
    ulExtraMicros = ((D_UINT32)pDateTime->uMillisecond * 1000) + pDateTime->uMicrosecond;

    DclUint64AddUint32(pTime, ulExtraMicros);

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: DclTimeFormat()

    Convert a count of relative seconds and formats it as a string.

    The output is in the form -- "days:hh:mm:ss".

    "days" is a variable length field, and will be "0" if the
    count represents less than one day.  The other fields will
    always be two characters long.

    Parameters:
        ullSeconds  - The count of seconds to use
        pBuffer     - A buffer to receive the formatted string
        nBuffLen    - The size of the buffer

    Return Value:
        Returns a pointer to the output buffer if successful, or
        NULL if the output buffer is not long enough (in which case
        it may have been modified).
-------------------------------------------------------------------*/
char * DclTimeFormat(
    D_UINT64        ullSeconds,
    char           *pBuffer,
    size_t          nBuffLen)
{
    D_UINT64        ullDays, ullHours, ullMinutes;
    int             iLen;

    DclAssert(pBuffer);
    DclAssert(nBuffLen);

    /*  Calculate full days
    */
    ullDays = ullSeconds;
    DclUint64DivUint32(&ullDays, SECONDS_PER_DAY);

    /*  Calculate full hours
    */
    ullHours = ullSeconds;
    DclUint64ModUint32(&ullHours, SECONDS_PER_DAY);
    ullMinutes = ullHours;
    DclUint64DivUint32(&ullHours, SECONDS_PER_HOUR);

    /*  Calculate full minutes
    */
    DclUint64ModUint32(&ullMinutes, SECONDS_PER_HOUR);
    ullSeconds = ullMinutes;
    DclUint64DivUint32(&ullMinutes, SECONDS_PER_MINUTE);

    /*  Remainder is seconds
    */
    DclUint64ModUint32(&ullSeconds, SECONDS_PER_MINUTE);

    iLen = DclSNPrintf(pBuffer, nBuffLen, "%llU:%02llU:%02llU:%02llU",
        VA64BUG(ullDays), VA64BUG(ullHours), VA64BUG(ullMinutes), VA64BUG(ullSeconds));

    if(iLen == -1)
        return NULL;
    else
        return pBuffer;
}


/*-------------------------------------------------------------------
    Public: DclTimeFormatUS()

    Take a count of relative microseconds, round it to the nearest
    millisecond, and format it as a string.

    The output is in the form -- "days:hh:mm:ss.mmm".

    "days" is a variable length field, and will be "0" if the count
    represents less than one day.  The mm, hh, and ss fields will
    always be two characters long.  The mmm milliseconds field will
    always be 3 character long.

    Parameters:
        ullMicrosecs - The count of microseconds to use
        pBuffer      - A buffer to receive the formatted string
        nBuffLen     - The size of the buffer

    Return Value:
        Returns a pointer to the output buffer if successful, or
        NULL if the output buffer is not long enough (in which case
        it may have been modified).
-------------------------------------------------------------------*/
char * DclTimeFormatUS(
    D_UINT64        ullMicrosecs,
    char           *pBuffer,
    size_t          nBuffLen)
{
    D_UINT64        ullMillisecs;
    size_t          nLen;
    char           *pStr;
    
    /*  Convert US to MS, rounding as appropriate
    */
    ullMillisecs = (ullMicrosecs + 500) / 1000;

    if(!DclTimeFormat(ullMillisecs / 1000, pBuffer, nBuffLen))
        return NULL;

    /*  Make sure there's room for the .nnn, and don't forget about the 
        terminator.
    */        
    nLen = DclStrLen(pBuffer);
    if(nLen + 4 >= nBuffLen)
        return NULL;

    pStr = pBuffer + nLen;
    nBuffLen -= nLen;

    DclSNPrintf(pStr, nBuffLen, ".%03u", ullMillisecs % 1000);

    return pBuffer;
}


/*-------------------------------------------------------------------
    Local: ValidateDateTime()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL ValidateDateTime(
    const DCLDATETIME  *pDateTime)
{
    DclAssert(pDateTime);

    if(pDateTime->uYear < 1970)
        return FALSE;

    if(pDateTime->uMonth > 11)
        return FALSE;

    if(pDateTime->uDay < 1)
        return FALSE;

    if(pDateTime->uMonth == FEBRUARY && ISLEAPYEAR(pDateTime->uYear))
    {
        if(pDateTime->uDay > uDaysPerMonth[FEBRUARY] + 1)
            return FALSE;
    }
    else
    {
        if(pDateTime->uDay > uDaysPerMonth[pDateTime->uMonth])
            return FALSE;
    }

    if(pDateTime->uHour > 23)
        return FALSE;

    if(pDateTime->uMinute > 59)
        return FALSE;

    if(pDateTime->uSecond > 59)
        return FALSE;

    if(pDateTime->uMillisecond > 999)
        return FALSE;

    if(pDateTime->uMicrosecond > 999)
        return FALSE;

    return TRUE;
}
