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

    This header only needs to be included in those modules that need access
    to the vprintf() and vsprintf() variations on printf() and sprintf().

    The prototypes for these functions are kept separate (rather than being
    declared in dcl.h) to avoid ending up having to include stdarg.h in
    places where it really is not needed.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlprintf.h $
    Revision 1.13  2010/12/02 01:49:48Z  garyp
    Corrected so double percent signs are properly handled.
    Revision 1.12  2010/11/11 07:19:36Z  garyp
    Added support for the %T format specifier.
    Revision 1.11  2010/11/01 16:59:52Z  garyp
    Minor parameter type update.
    Revision 1.10  2010/11/01 03:52:14Z  garyp
    Refactored DclVSNPrintf() to use some public helper functions to allow
    access to printf() format string parsing functionality.
    Revision 1.9  2009/06/27 23:12:58Z  garyp
    Prototype update.
    Revision 1.8  2009/02/08 02:35:54Z  garyp
    Merged from the v4.0 branch.  Fixed DclVSNPrintf() so it is not conditioned
    on DCLCONF_OUTPUT_ENABLED.
    Revision 1.7  2008/05/03 19:54:05Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.6  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.5  2007/01/02 23:22:11Z  Garyp
    Added prototypes for new logging functions.
    Revision 1.4  2006/10/04 02:47:55Z  Garyp
    Added a DCLFUNC declaration for DclTraceVPrintf().
    Revision 1.3  2006/05/06 19:34:08Z  Garyp
    Minor prototype updates.  Commented out DclSNPrintf() as it is already
    handled in dlapiint.h.
    Revision 1.2  2006/03/03 22:25:01Z  johnb
    Added DclSNPrintf
    Revision 1.1  2005/10/02 04:44:48Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLPRINTF_H_INCLUDED
#define DLPRINTF_H_INCLUDED

#include <stdarg.h>

typedef enum
{
    DCLPRFMT_UNKNOWN = 0,
    DCLPRFMT_CHAR,                       /* %c       */
    DCLPRFMT_ANSISTRING,                 /* %s       */
    DCLPRFMT_UNICODESTRING,              /* %W       */
    DCLPRFMT_SIGNED16BIT,                /* %D       */
    DCLPRFMT_UNSIGNED16BIT,              /* %U       */
    DCLPRFMT_SIGNED32BIT,                /* %lD      */
    DCLPRFMT_UNSIGNED32BIT,              /* %lU      */
    DCLPRFMT_SIGNED64BIT,                /* %llD     */
    DCLPRFMT_UNSIGNED64BIT,              /* %llU     */
    DCLPRFMT_HEX16BIT,                   /* %X       */
    DCLPRFMT_HEX32BIT,                   /* %lX      */
    DCLPRFMT_HEX64BIT,                   /* %llX     */
    DCLPRFMT_POINTER,                    /* %P       */
    DCLPRFMT_TIME,                       /* %T       */
    DCLPRFMT_DOUBLEPERCENT               /* %%       */
} DCLPRINTTYPE;

typedef struct
{
    DCLPRINTTYPE    nType;              /* The DCLPRFMT_* type found */
    const char     *pszSpecifier;       /* Returns a pointer to the % sign */
    const char     *pVariableWidthChar; /* A pointer to any variable width character (NULL if not found) */
    size_t          nFillLen;
    char            cFillChar;
    unsigned        fLeftJustified : 1; 
    unsigned        fHasIllegalType : 1;/* TRUE if an illegal sequence was skipped over */
} DCLPRINTFORMAT;    


#define DclVSNPrintf                    DCLFUNC(DclVSNPrintf)
#define DclPrintfProcessFormatSegment   DCLFUNC(DclPrintfProcessFormatSegment)
#define DclPrintfParseFormatSpecifier   DCLFUNC(DclPrintfParseFormatSpecifier)
#define DclPrintfParseFormatType        DCLFUNC(DclPrintfParseFormatType)

int             DclVSNPrintf(char *pBuffer, int nBufferLen, const char *pszFmt, va_list arglist);
size_t          DclPrintfProcessFormatSegment(char *pBuffer, size_t nBufferLen, const char *pszFmt, DCLPRINTFORMAT *pFormat, size_t *pnSpecifierLen);
size_t          DclPrintfParseFormatSpecifier(char const *pszFmt, DCLPRINTFORMAT *pFormatType);
DCLPRINTTYPE    DclPrintfParseFormatType(const char *pszFmt, size_t *pnTypeLen);

#if DCLCONF_OUTPUT_ENABLED

#include <dllog.h>  /* necessary for DclLogVPrintf() */

#define DclVPrintf              DCLFUNC(DclVPrintf)
#define DclTraceVPrintf         DCLFUNC(DclTraceVPrintf)
#define DclLogVPrintf           DCLFUNC(DclLogVPrintf)

int     DclVPrintf(const char *pszFmt, va_list arglist);
int     DclTraceVPrintf(D_UINT32 ulFlags, D_UINT32 ulTraceMask, const char *pszFmt, va_list arglist);
int     DclLogVPrintf(DCLINSTANCEHANDLE hDclInst, DCLLOGHANDLE hLog, const char *pszFmt, va_list arglist);

#endif



#endif  /* DLPRINTF_H_INCLUDED */

