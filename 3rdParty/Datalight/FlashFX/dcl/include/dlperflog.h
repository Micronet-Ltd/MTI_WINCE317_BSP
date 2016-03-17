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

    This header file defines the interface to the performance logging
    interface.

    This header contains the magic keyword "PrivateHeader" which prevents
    it from being included in the automated documentation.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlperflog.h $
    Revision 1.13  2009/06/25 00:28:05Z  garyp
    Updated to include dllog.h.
    Revision 1.12  2009/02/08 01:10:39Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.11  2008/05/03 19:54:04Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.10  2007/12/18 20:47:58Z  brandont
    Updated comments.
    Revision 1.9  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.8  2007/08/29 23:29:25Z  Garyp
    Changed some D_BUFFER types to char to avoid compiler warnings in
    some environments.
    Revision 1.7  2007/08/02 19:36:14Z  timothyj
    Added multiple include protection.
    Revision 1.6  2007/06/22 17:26:02Z  Garyp
    Refactored the close operation into separate write and copy functions.
    Updated to allow a "CategoreSuffix" value.
    Revision 1.5  2007/05/03 00:17:15Z  Garyp
    Added the ability to record 64-bit values.
    Revision 1.4  2007/04/03 19:14:59Z  Garyp
    Updated to support a PerfLog build number suffix.
    Revision 1.3  2007/02/01 22:44:44Z  Garyp
    Fixed problems with the previous checkin.
    Revision 1.2  2007/02/01 21:22:37Z  Garyp
    Added the ability to output signed integer values.
    Revision 1.1  2007/01/01 22:35:02Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/
#ifndef DLPERFLOG_H_INCLUDED
#define DLPERFLOG_H_INCLUDED


/*-------------------------------------------------------------------
    Type: DCLPERFLOGHANDLE

    Type for a performance log handle.
-------------------------------------------------------------------*/
typedef struct DCLPERFLOGINFO *DCLPERFLOGHANDLE;

#define PERFLOG_MAX_SUFFIX_LEN  (24)


#if DCLCONF_OUTPUT_ENABLED

#include <dllog.h>


/*-------------------------------------------------------------------
    Performance Logging Functions
-------------------------------------------------------------------*/

#define             DclPerfLogOpen      DCLFUNC(DclPerfLogOpen)
#define             DclPerfLogString    DCLFUNC(DclPerfLogString)
#define             DclPerfLogHex16     DCLFUNC(DclPerfLogHex16)
#define             DclPerfLogHex32     DCLFUNC(DclPerfLogHex32)
#define             DclPerfLogInt       DCLFUNC(DclPerfLogInt)
#define             DclPerfLogNum       DCLFUNC(DclPerfLogNum)
#define             DclPerfLogNum64     DCLFUNC(DclPerfLogNum64)
#define             DclPerfLogCopy      DCLFUNC(DclPerfLogCopy)
#define             DclPerfLogWrite     DCLFUNC(DclPerfLogWrite)
#define             DclPerfLogClose     DCLFUNC(DclPerfLogClose)

DCLPERFLOGHANDLE    DclPerfLogOpen(DCLLOGHANDLE hLog, const char *pszTestName, const char *pszTestCategory, const char *pszCatSuffix, const char *pszBldSuffix);
void                DclPerfLogString(DCLPERFLOGHANDLE hPerfLog, const char *pszDataName, const char *pszDataValue);
void                DclPerfLogHex16( DCLPERFLOGHANDLE hPerfLog, const char *pszDataName, D_UINT16 uDataValue);
void                DclPerfLogHex32( DCLPERFLOGHANDLE hPerfLog, const char *pszDataName, D_UINT32 ulDataValue);
void                DclPerfLogInt(   DCLPERFLOGHANDLE hPerfLog, const char *pszDataName, D_INT32 lDataValue);
void                DclPerfLogNum(   DCLPERFLOGHANDLE hPerfLog, const char *pszDataName, D_UINT32 ulDataValue);
void                DclPerfLogNum64( DCLPERFLOGHANDLE hPerfLog, const char *pszDataName, D_UINT64 ullDataValue);
DCLSTATUS           DclPerfLogCopy(  DCLPERFLOGHANDLE hPerfLog, unsigned nBufferLen, char *pDescBuffer, char *pDataBuffer);
void                DclPerfLogWrite( DCLPERFLOGHANDLE hPerfLog);
void                DclPerfLogClose( DCLPERFLOGHANDLE hPerfLog);


/*-------------------------------------------------------------------
    Macro: DCLPERFLOG_OPEN()

    This macro conditionally opens a performance log instance
    and writes header data out.  This must be paired with a
    call to DCLPERFLOG_CLOSE().

    Note that this is a virtual "open".  Neither this macro,
    nor any of the other performance logging macros (aside
    from DCLPERFLOG_WRITE) do any file I/O or write any output.
    These macros buffer the data until DCLPERFLOG_COPY() or
    DCLPERFLOG_WRITE() are called.

    Parameters:
        cond        - A condition to test, if TRUE, then the instance
                      will be created, if FALSE then the call will be
                      ignored.
        hLog        - The DCLLOGHANDLE to use when writing the
                      output.  This may be NULL to use the standard
                      output mechanisms.
        szName      - The test name.
        szCategory  - The test category.
        szCatSuffix - An optional suffix for the category field.
        szBldSuffix - An optional suffix for the build number field.

    Return Value:
        Returns a DCLPERFLOGHANDLE value if successful.  Returns
        NULL if (cond) evaluates to FALSE, or if the function
        failed.
-------------------------------------------------------------------*/
#define DCLPERFLOG_OPEN(cond, hLog, szName, szCategory, szCatSuffix, szBldSuffix)   \
    ((cond) ? (DclPerfLogOpen((hLog), (szName), (szCategory), (szCatSuffix), (szBldSuffix))) : (0))


/*-------------------------------------------------------------------
    Macro: DCLPERFLOG_STRING()

    This macro writes string data to the specified performance
    log.

    Parameters:
        hPerfLog   - The performance log handle returned by the
                     DCLPERFLOG_OPEN() macro.  If this value is
                     NULL, the function will be ignored.
        szDataName - The data name.
        szDataVal  - The data value to record.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define DCLPERFLOG_STRING(hPerfLog, szDataName, szDataVal)          \
    if(hPerfLog)                                                    \
        DclPerfLogString((hPerfLog), (szDataName), (szDataVal));


/*-------------------------------------------------------------------
    Macro: DCLPERFLOG_HEX16()

    This macro writes a 16-bit hex data value to the log.

    Parameters:
        hPerfLog   - The performance log handle returned by the
                     DCLPERFLOG_OPEN() macro.  If this value is
                     NULL, the function will be ignored.
        szDataName - The data name.
        uDataVal   - The data value to record.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define DCLPERFLOG_HEX16(hPerfLog, szDataName, uDataVal)            \
    if(hPerfLog)                                                    \
        DclPerfLogHex16((hPerfLog), (szDataName), (uDataVal));


/*-------------------------------------------------------------------
    Macro: DCLPERFLOG_HEX32()

    This macro writes a 32-bit hex data value to the log.

    Parameters:
        hPerfLog   - The performance log handle returned by the
                     DCLPERFLOG_OPEN() macro.  If this value is
                     NULL, the function will be ignored.
        szDataName - The data name.
        ulDataVal  - The data value to record.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define DCLPERFLOG_HEX32(hPerfLog, szDataName, ulDataVal)           \
    if(hPerfLog)                                                    \
        DclPerfLogHex32((hPerfLog), (szDataName), (ulDataVal));


/*-------------------------------------------------------------------
    Macro: DCLPERFLOG_INT()

    This macro writes a 32-bit signed decimal data value to the
    log.

    Parameters:
        hPerfLog   - The performance log handle returned by the
                     DCLPERFLOG_OPEN() macro.  If this value is
                     NULL, the function will be ignored.
        szDataName - The data name.
        lDataVal   - The data value to record.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define DCLPERFLOG_INT(hPerfLog, szDataName, lDataVal)              \
    if(hPerfLog)                                                    \
        DclPerfLogInt((hPerfLog), (szDataName), (lDataVal));


/*-------------------------------------------------------------------
    Macro: DCLPERFLOG_NUM()

    This macro writes a 32-bit unsigned decimal data value to the
    log.

    Parameters:
        hPerfLog   - The performance log handle returned by the
                     DCLPERFLOG_OPEN() macro.  If this value is
                     NULL, the function will be ignored.
        szDataName - The data name.
        ulDataVal  - The data value to record.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define DCLPERFLOG_NUM(hPerfLog, szDataName, ulDataVal)             \
    if(hPerfLog)                                                    \
        DclPerfLogNum((hPerfLog), (szDataName), (ulDataVal));


/*-------------------------------------------------------------------
    Macro: DCLPERFLOG_NUM64()

    This macro writes a 64-bit unsigned decimal data value to the
    log.

    Parameters:
        hPerfLog   - The performance log handle returned by the
                     DCLPERFLOG_OPEN() macro.  If this value is
                     NULL, the function will be ignored.
        szDataName - The data name.
        ullDataVal - The data value to record.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define DCLPERFLOG_NUM64(hPerfLog, szDataName, ullDataVal)          \
    if(hPerfLog)                                                    \
        DclPerfLogNum64((hPerfLog), (szDataName), (ullDataVal));


/*-------------------------------------------------------------------
    Macro: DCLPERFLOG_WRITE()

    This macro write performance log data to the device which
    was specified when the log was opened.

    Parameters:
        hPerfLog   - The performance log handle returned by the
                     DCLPERFLOG_OPEN() macro.  If this value is
                     NULL, the function will be ignored.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define DCLPERFLOG_WRITE(hPerfLog)                                  \
    if(hPerfLog)                                                    \
        DclPerfLogWrite(hPerfLog);


/*-------------------------------------------------------------------
    Macro: DCLPERFLOG_CLOSE()

    This macro closes the performance log instance opened with
    DCLPERFLOG_CLOSE().

    Parameters:
        hPerfLog   - The performance log handle returned by the
                     DCLPERFLOG_OPEN() macro.  If this value is
                     NULL, the function will be ignored.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define DCLPERFLOG_CLOSE(hPerfLog)                                  \
    if(hPerfLog)                                                    \
        DclPerfLogClose(hPerfLog);


#else

#define DCLPERFLOG_OPEN(cond, hLog, szName, szCategory, szCatSuffix, szBldSuffix)   (NULL)
#define DCLPERFLOG_STRING(hPerfLog, szDataName, szDataVal)
#define DCLPERFLOG_HEX16( hPerfLog, szDataName, uDataVal)
#define DCLPERFLOG_HEX32( hPerfLog, szDataName, ulDataVal)
#define DCLPERFLOG_INT(   hPerfLog, szDataName, lDataVal)
#define DCLPERFLOG_NUM(   hPerfLog, szDataName, ulDataVal)
#define DCLPERFLOG_NUM64( hPerfLog, szDataName, ullDataVal)
#define DCLPERFLOG_WRITE( hPerfLog)
#define DCLPERFLOG_CLOSE( hPerfLog)

#endif  /* DCLCONF_OUTPUT_ENABLED */



#endif  /* DLPERFLOG_H_INCLUDED */


