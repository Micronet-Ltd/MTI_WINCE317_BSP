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

    This module implements the performance logging interface.  Note that
    the functions in this module are not NOT typically called directly,
    but rather the macros defined in dlperflog.h are used instead.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlperflog.c $
    Revision 1.20  2010/11/01 03:58:32Z  garyp
    Renamed DclLogWrite() to DclLogWriteString() to avoid confusion with
    the new logging capability of handling binary logs.
    Revision 1.19  2009/06/19 23:27:48Z  garyp
    Updated for functions which now take a DCL Instance handle.
    Revision 1.18  2009/05/02 17:19:52Z  garyp
    Minor datatype changes.  Documentation updated.
    Revision 1.17  2008/05/27 17:09:54Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.16  2008/05/03 19:53:59Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.15  2007/12/18 20:50:35Z  brandont
    Updated function headers.
    Revision 1.14  2007/11/03 23:31:12Z  Garyp
    Added the standard module header.
    Revision 1.13  2007/08/30 20:31:41Z  Garyp
    Changed some D_BUFFER types to char to avoid compiler warnings in
    some environments.
    Revision 1.12  2007/08/02 19:44:20Z  timothyj
    Removed unused variable.
    Revision 1.11  2007/06/22 19:42:02Z  Garyp
    Refactored the close operation to support separate write and copy
    operations.  Added the ability to specify a "CategorySuffix" value.
    Modified the PerfLog stuff to use the new DCLPERFLOG_WRITE() functionality
    as well as handle the new "CategorySuffix" parameter.
    Revision 1.10  2007/05/03 00:17:15Z  Garyp
    Added the ability to record 64-bit values.
    Revision 1.9  2007/04/06 02:22:38Z  Garyp
    Reduced MINFIELDWIDTH to 5.
    Revision 1.8  2007/04/06 01:42:23Z  Garyp
    Improved error handling when truncating log data.  Reduced the odds of
    having log data truncated by using DclLogWrite() rather than DclLogPrintf().
    Revision 1.7  2007/04/02 17:36:29Z  Garyp
    Updated to support a PerfLog build number suffix.
    Revision 1.6  2007/02/22 19:52:58Z  Garyp
    Updated to initialize the notes field.
    Revision 1.5  2007/02/01 23:00:02Z  Garyp
    One more time...
    Revision 1.4  2007/02/01 22:44:45Z  Garyp
    Fixed problems with the previous checkin.
    Revision 1.3  2007/02/01 21:22:36Z  Garyp
    Added the ability to output signed integer values.
    Revision 1.2  2007/01/08 02:03:29Z  Garyp
    Added a notes field.
    Revision 1.1  2007/01/01 23:31:16Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#if DCLCONF_OUTPUT_ENABLED

#include <dlperflog.h>

#define PERFLOGBUFFLEN  (256)
#define MINFIELDWIDTH     (5)

struct DCLPERFLOGINFO
{
    DCLLOGHANDLE    hLog;
    char            szDescription[PERFLOGBUFFLEN];
    char            szData[PERFLOGBUFFLEN];
};

typedef struct DCLPERFLOGINFO DCLPERFLOGINFO;


/*-------------------------------------------------------------------
    Protected: DclPerfLogOpen()

    This function opens a performance log instance and writes
    header data out.  This must be paired with a call to
    DclPerfLogClose().

    Note that this is a virtual "open".  Neither this function,
    nor any of the other performance logging functions (aside
    from DclPerfLogClose) do any file I/O or write any output.
    These functions buffer the data until DclPerfLogClose() is
    called, at which time the data is written to the log file
    specified by hLog, using the DclLogWriteString() function.

    Parameters:
        hLog         - The DCLLOGHANDLE to use when writing the
                       output.  This may be NULL to use the standard
                       output mechanisms.
        pszTestName  - The test name.
        pszCategory  - The test category.
        pszCatSuffix - An optional suffix for the category field.
        pszBldSuffix - An optional suffix for the build number field.

    Return Value:
        If successful, returns a DCLPERFLOGHANDLE value, otherwise
        returns NULL.
-------------------------------------------------------------------*/
DCLPERFLOGHANDLE DclPerfLogOpen(
    DCLLOGHANDLE    hLog,
    const char     *pszTestName,
    const char     *pszCategory,
    const char     *pszCatSuffix,
    const char     *pszBldSuffix)
{
    #define         MINCATEGORYLEN  (16)
    DCLPERFLOGINFO *pDLI;
    size_t          nCatLen;
    size_t          nBldLen;
    char            szBuildNum[32];
    char            szCategory[64];

    DclAssert(pszTestName);
    DclAssert(pszCategory);

    pDLI = DclMemAllocZero(sizeof *pDLI);
    if(!pDLI)
        return NULL;

    pDLI->hLog = hLog;

    DclStrNCpy(szBuildNum, DCLSTRINGIZE(D_PRODUCTBUILD), sizeof(szBuildNum));
    if(pszBldSuffix)
        DclStrNCat(szBuildNum, pszBldSuffix, sizeof(szBuildNum) - DclStrLen(szBuildNum));
    nBldLen = DclStrLen(szBuildNum);

    DclAssert(DclStrLen(pszCategory) < sizeof(szCategory));
    DclStrNCpy(szCategory, pszCategory, sizeof(szCategory));
    if(pszCatSuffix)
        DclStrNCat(szCategory, pszCatSuffix, sizeof(szCategory) - DclStrLen(szCategory));
    nCatLen = DCLMAX(DclStrLen(szCategory), MINCATEGORYLEN);

    /*  Note that while it is nice from a readability standpoint to
        pad fields with spaces, we expressly do not do this for the
        "BuildNum" field because we need to be able to easily parse
        the data based on this field, and since a given build number
        could easily be a subset of another build number, using no
        padding allows the number to be uniquely parsed using the
        commas on either side, as in Bld=",1234,".

        On the other hand, we do pad the actual data fields since we
        will not be parsing things based on these values.
    */
    DclSNPrintf(pDLI->szDescription, sizeof(pDLI->szDescription),
        "PERFLOG_%s,%*s,%*s,  TestDate,  Notes", pszTestName, nCatLen, "Category", nBldLen, "Bld");

    /*  Note that a placeholder is used for the test date because we will
        fill that in later when the performance data is being processed.
        We don't want to use the date that the target system reports as
        it may be invalid.
    */
    DclSNPrintf(pDLI->szData, sizeof(pDLI->szData),
        "PERFLOG_%s,%*s,%s,-TestDate-,-Notes-", pszTestName, nCatLen, szCategory, szBuildNum);

    return pDLI;
}


/*-------------------------------------------------------------------
    Protected: DclPerfLogString()

    This function writes string data to the specified performance
    log.

    Parameters:
        hPerfLog    - The performance log handle returned by the
                      DclPerfLogOpen() function.
        pszDataName - The data name.
        pszDataVal  - The data value to record.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclPerfLogString(
    DCLPERFLOGHANDLE    hPerfLog,
    const char         *pszDataName,
    const char         *pszDataValue)
{
    size_t              nNameLen;
    size_t              nDataLen;
    size_t              nCurrentLen;

    DclAssert(hPerfLog);
    DclAssert(pszDataName);
    DclAssert(pszDataValue);

    nNameLen = DclStrLen(pszDataName);
    nDataLen = DclStrLen(pszDataValue);

    nDataLen = DCLMAX(nDataLen, nNameLen);
    nDataLen = DCLMAX(nDataLen, MINFIELDWIDTH);

    /*  Append this data name onto the existing string
    */
    nCurrentLen = DclStrLen(hPerfLog->szDescription);
    DclSNPrintf(&hPerfLog->szDescription[nCurrentLen], sizeof(hPerfLog->szDescription) - nCurrentLen,
        ",%*s", nDataLen, pszDataName);

    /*  Ensure that this buffer is ALWAYS null terminated
    */
    hPerfLog->szDescription[sizeof(hPerfLog->szDescription) - 1] = 0;

    /*  Append this data value onto the existing string
    */
    nCurrentLen = DclStrLen(hPerfLog->szData);
    DclSNPrintf(&hPerfLog->szData[nCurrentLen], sizeof(hPerfLog->szData) - nCurrentLen,
        ",%*s", nDataLen, pszDataValue);

    /*  Ensure that this buffer is ALWAYS null terminated
    */
    hPerfLog->szData[sizeof(hPerfLog->szData) - 1] = 0;

    return;
}


/*-------------------------------------------------------------------
    Protected: DclPerfLogHex16()

    This function writes 16-bit hex data to the specified
    performance log.

    Parameters:
        hPerfLog    - The performance log handle returned by the
                      DclPerfLogOpen() function.
        pszDataName - The data name.
        uDataVal    - The data value to record.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclPerfLogHex16(
    DCLPERFLOGHANDLE    hPerfLog,
    const char         *pszDataName,
    D_UINT16            uDataValue)
{
    char                achTempBuff[8];     /* long enough for "0x1234" */
    size_t              nLen = sizeof(achTempBuff);

    DclAssert(hPerfLog);
    DclAssert(pszDataName);

    achTempBuff[0] = '0';
    achTempBuff[1] = 'x';
    DclULtoA(&achTempBuff[2], &nLen, (D_UINT32)uDataValue, 16, 4, '0');

    DclPerfLogString(hPerfLog, pszDataName, achTempBuff);

    return;
}


/*-------------------------------------------------------------------
    Protected: DclPerfLogHex32()

    This function writes 32-bit hex data to the specified
    performance log.

    Parameters:
        hPerfLog    - The performance log handle returned by the
                      DclPerfLogOpen() function.
        pszDataName - The data name.
        ulDataVal   - The data value to record.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclPerfLogHex32(
    DCLPERFLOGHANDLE    hPerfLog,
    const char         *pszDataName,
    D_UINT32            ulDataValue)
{
    char                achTempBuff[12];     /* long enough for "0x12345678" */
    size_t              nLen = sizeof(achTempBuff);

    DclAssert(hPerfLog);
    DclAssert(pszDataName);

    achTempBuff[0] = '0';
    achTempBuff[1] = 'x';
    DclULtoA(&achTempBuff[2], &nLen, ulDataValue, 16, 8, '0');

    DclPerfLogString(hPerfLog, pszDataName, achTempBuff);

    return;
}


/*-------------------------------------------------------------------
    Protected: DclPerfLogInt()

    This function writes 32-bit signed numeric data to the
    specified performance log.

    Parameters:
        hPerfLog    - The performance log handle returned by the
                      DclPerfLogOpen() function.
        pszDataName - The data name.
        lDataVal    - The data value to record.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclPerfLogInt(
    DCLPERFLOGHANDLE    hPerfLog,
    const char         *pszDataName,
    D_INT32             lDataValue)
{
    char                achTempBuff[12];    /* long enough for D_UINT32_MAX in decimal */
    size_t              nLen = sizeof(achTempBuff);

    DclAssert(hPerfLog);
    DclAssert(pszDataName);

    DclLtoA(achTempBuff, &nLen, lDataValue, 0, ' ');

    DclPerfLogString(hPerfLog, pszDataName, achTempBuff);

    return;
}


/*-------------------------------------------------------------------
    Protected: DclPerfLogNum()

    This function writes 32-bit unsigned numeric data to the
    specified performance log.

    Parameters:
        hPerfLog    - The performance log handle returned by the
                      DclPerfLogOpen() function.
        pszDataName - The data name.
        ulDataVal   - The data value to record.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclPerfLogNum(
    DCLPERFLOGHANDLE    hPerfLog,
    const char         *pszDataName,
    D_UINT32            ulDataValue)
{
    char                achTempBuff[12];    /* long enough for D_UINT32_MAX in decimal */
    size_t              nLen = sizeof(achTempBuff);

    DclAssert(hPerfLog);
    DclAssert(pszDataName);

    DclULtoA(achTempBuff, &nLen, ulDataValue, 10, 0, ' ');

    DclPerfLogString(hPerfLog, pszDataName, achTempBuff);

    return;
}


/*-------------------------------------------------------------------
    Protected: DclPerfLogNum64()

    This function writes 64-bit unsigned numeric data to the
    specified performance log.

    Parameters:
        hPerfLog    - The performance log handle returned by the
                      DclPerfLogOpen() function.
        pszDataName - The data name.
        ullDataVal  - The data value to record.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclPerfLogNum64(
    DCLPERFLOGHANDLE    hPerfLog,
    const char         *pszDataName,
    D_UINT64            ullDataValue)
{
    char                achTempBuff[24];    /* long enough for D_UINT64_MAX in decimal */
    size_t              nLen = sizeof(achTempBuff);

    DclAssert(hPerfLog);
    DclAssert(pszDataName);

    DclULLtoA(achTempBuff, &nLen, &ullDataValue, 10, 0, ' ');

    DclPerfLogString(hPerfLog, pszDataName, achTempBuff);

    return;
}


/*-------------------------------------------------------------------
    Protected: DclPerfLogCopy()

    This function copies the performance log description and
    data lines to the supplied buffers.  The strings will be
    terminated with a \n and NULL, truncated if necessary.

    Parameters:
        hPerfLog     - The performance log handle returned by the
                       DclPerfLogOpen() function.
        nBufferLen   - The length of each of the two supplied
                       buffers.
        pDescBuffer  - A pointer to a buffer to contain the
                       PerfLog description information.
        pDataBuffer  - A pointer to a buffer to contain the
                       PerfLog data.

    Return Value:
        Returns DCLSTAT_SUCCESS or DCLSTAT_PERFLOG_TRUNCATED.
-------------------------------------------------------------------*/
DCLSTATUS DclPerfLogCopy(
    DCLPERFLOGHANDLE    hPerfLog,
    unsigned            nBufferLen,
    char               *pDescBuffer,
    char               *pDataBuffer)
{
    size_t              nCurrentLen;
    unsigned            fTruncated = FALSE;

    DclAssert(hPerfLog);
    DclAssert(nBufferLen);
    DclAssert(pDescBuffer);
    DclAssert(pDataBuffer);

    /*  Process the description line, truncating it if necessary
    */
    nCurrentLen = DclStrLen(hPerfLog->szDescription);

    DclAssert(nCurrentLen < sizeof(hPerfLog->szDescription));

    DclStrNCpy(pDescBuffer, hPerfLog->szDescription, nBufferLen);

    if(nCurrentLen >= (nBufferLen-1))
    {
        pDescBuffer[nBufferLen - 2] = '\n';
        pDescBuffer[nBufferLen - 1] = 0;
        fTruncated = TRUE;
    }
    else
    {
        pDescBuffer[nCurrentLen + 0] = '\n';
        pDescBuffer[nCurrentLen + 1] = 0;
    }

    /*  Process the data line, truncating it if necessary
    */
    nCurrentLen = DclStrLen(hPerfLog->szData);

    DclAssert(nCurrentLen < sizeof(hPerfLog->szData));

    DclStrNCpy(pDataBuffer, hPerfLog->szData, nBufferLen);

    if(nCurrentLen >= (nBufferLen-1))
    {
        pDataBuffer[nBufferLen - 2] = '\n';
        pDataBuffer[nBufferLen - 1] = 0;
        fTruncated = TRUE;
    }
    else
    {
        pDataBuffer[nCurrentLen + 0] = '\n';
        pDataBuffer[nCurrentLen + 1] = 0;
    }

    if(fTruncated)
        return DCLSTAT_PERFLOG_TRUNCATED;
    else
        return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Protected: DclPerfLogWrite()

    This function write performance log data to the device
    specified when the log was opened.

    Parameters:
        hPerfLog    - The performance log handle returned by the
                      DclPerfLogOpen() function.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclPerfLogWrite(
    DCLPERFLOGHANDLE    hPerfLog)
{
    size_t              nCurrentLen;
    unsigned            fTruncated = FALSE;

    DclAssert(hPerfLog);

    /*  Process the description line, truncating it if necessary
    */
    nCurrentLen = DclStrLen(hPerfLog->szDescription);

    DclAssert(nCurrentLen < sizeof(hPerfLog->szDescription));

    if(nCurrentLen == sizeof(hPerfLog->szDescription) - 1)
    {
        hPerfLog->szDescription[nCurrentLen - 1] = '\n';
        fTruncated = TRUE;
    }
    else
    {
        hPerfLog->szDescription[nCurrentLen + 0] = '\n';
        hPerfLog->szDescription[nCurrentLen + 1] = 0;
    }

    DclLogWriteString(NULL, hPerfLog->hLog, hPerfLog->szDescription);

    /*  Process the data line, truncating it if necessary
    */
    nCurrentLen = DclStrLen(hPerfLog->szData);

    DclAssert(nCurrentLen < sizeof(hPerfLog->szData));

    if(nCurrentLen == sizeof(hPerfLog->szData) - 1)
    {
        hPerfLog->szData[nCurrentLen - 1] = '\n';
        fTruncated = TRUE;
    }
    else
    {
        hPerfLog->szData[nCurrentLen + 0] = '\n';
        hPerfLog->szData[nCurrentLen + 1] = 0;
    }

    DclLogWriteString(NULL, hPerfLog->hLog, hPerfLog->szData);

    if(fTruncated)
        DclPrintf("Warning: PerfLog output was truncated!\n");

    return;
}


/*-------------------------------------------------------------------
    Protected: DclPerfLogClose()

    This function closes a performance log that was opened with
    DclPerfLogOpen().

    Parameters:
        hPerfLog    - The performance log handle returned by the
                      DclPerfLogOpen() function.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclPerfLogClose(
    DCLPERFLOGHANDLE    hPerfLog)
{
    DclAssert(hPerfLog);

    /*  Close this instance and release resources
    */
    DclMemFree(hPerfLog);

    return;
}


#endif  /* DCLCONF_OUTPUT_ENABLED */

