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

    The functions in this module are used internally by Datalight products.
    Typically these functions are used rather than the similarly named,
    general Standard C library functions.  Typically this is done for
    compatibility, portability, and code size reasons.

    These functions are NOT intended to be complete, 100% ANSI C compatible
    implementations, but rather are designed address specific needs.

    See the specific function headers for more information about departures
    from the ANSI standard, and missing or extended functionality.

    ToDo:
      - Add support for an emulated D_INT64 type.
      - Add support for pointers which are <not> 32 bits wide.
      - The functions should be changed to use size_t types rather than ints.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlsprint.c $
    Revision 1.29  2011/03/23 22:57:48Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.28  2010/12/02 02:07:44Z  garyp
    Corrected so double percent signs are properly handled.
    Revision 1.27  2010/11/13 01:58:24Z  garyp
    Added support for the %T format specifier.  Removed some dead code.
    Revision 1.26  2010/11/01 16:59:53Z  garyp
    Minor parameter type update.
    Revision 1.25  2010/10/29 21:26:03Z  garyp
    Refactored DclVSNPrintf() to use some public helper functions to allow
    access to printf() format string parsing functionality.
    Revision 1.24  2010/08/02 17:17:52Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.23  2010/06/02 16:28:02Z  garyp
    Reverted the 16-bit datatype handling to its original implementation
    circa rev 1.20.
    Revision 1.22  2010/06/02 15:18:06Z  garyp
    Partial re-implementation of the previous revision.  Updated to call
    va_arg() with respect to the DCL_VA_ARG_DATATYPE_MINIMUM_IS_INT setting.
    Revision 1.21  2010/06/02 00:05:47Z  garyp
    Changed to now use fixed with data-types with va_args() where we
    could not previously -- and hope no old compilers break.
    Revision 1.20  2010/04/28 23:31:28Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.19  2009/11/05 02:14:20Z  garyp
    Fixed a bug which could cause output to be truncated should a 
    string exceed its fill length.
    Revision 1.18  2009/05/02 17:35:55Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.17  2009/05/02 17:13:22Z  garyp
    Added a conditional feature to normalize all pointers displayed with the
    "%P" format specifier, to allow diffing logs to be simplified.  Minor
    datatype updates.  Laid some groundwork for supporting pointers which
    are not 32-bits wide.
    Revision 1.16  2009/02/08 01:09:03Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.15  2007/12/18 19:23:18Z  brandont
    Updated function headers.
    Revision 1.14  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.13  2007/11/01 19:54:18Z  Garyp
    Removed the inclusion of limits.h.
    Revision 1.12  2007/08/01 21:05:48Z  Garyp
    Updated the handling of wide character string conversion so that they
    will properly be truncated if the output buffer is not long enough.
    Documentation updated.
    Revision 1.11  2006/10/18 22:24:09Z  joshuab
    Remove stray va_end(), DclVSNPrintf should not be calling it.
    Revision 1.10  2006/06/03 03:32:13Z  Garyp
    Updated to support emulated 64-bit types (but not an emulated D_INT64).
    Revision 1.9  2006/05/06 19:25:53Z  Garyp
    Fixed DclVSNPrintf() to better handle buffer overflow conditions, properly
    display signed numbers when a pad character is used (though still broken for
    64-bit signed numbers with a pad char), and to correctly return -1 if the
    buffer fills up and there is no room for the null-terminator.
    Revision 1.8  2006/03/21 00:33:57Z  Garyp
    Fixed to properly pad 64-bit hex numbers.
    Revision 1.7  2006/03/17 23:37:19Z  joshuab
    Added width processing for %W format strings (UNICODE environments only.
    Revision 1.6  2006/02/19 01:48:28Z  Garyp
    Modified so that if %W is encountered in a non-Unicode environment, the
    string is treated as an ANSI string.
    Revision 1.5  2006/01/26 18:47:00Z  Pauli
    Updated to use "null" if passed a null ptr as a string parameter.
    Revision 1.4  2006/01/10 08:02:17Z  Garyp
    Updated to support '*' to allow variable length field widths.  Documentation
    updated.
    Revision 1.3  2006/01/04 02:06:51Z  Garyp
    Updated to use some renamed settings.
    Revision 1.2  2005/12/13 20:32:59Z  billr
    It takes a lot of 'F's to fill 64 bits.
    Revision 1.1  2005/10/20 23:06:10Z  Pauli
    Initial revision
    Revision 1.3  2005/08/03 19:17:18Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.2  2005/07/31 00:39:59Z  Garyp
    Updated to support 64-bit output (when native), as well as work on systems
    where a long is 64-bits.
    Revision 1.1  2005/07/08 12:13:00Z  pauli
    Initial revision
    Revision 1.2  2005/06/13 19:43:42Z  PaulI
    Modified to be snprintf() and vsnprintf() compatible and use counted buffers
    to avoid buffer overflow.  Modified the Unicode string support to simply
    display nothing if the pointer is null.
    Revision 1.1  2005/04/11 03:48:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <stdarg.h>

#include <dcl.h>
#include <dlprintf.h>

/*  Set this to a non-zero count value to enable the pointer normalization
    feature, which causes all values printed with the %P format specifier
    to be displayed as 0xFFseqnum, where 'seqnum' is an index into the
    array used to store the pointers which have been processed.  Extremely
    useful when comparing log output from different runs.
*/
#define MAX_NORMALIZED_POINTERS     (0)    /* Must be 0 for checkin */

#if MAX_NORMALIZED_POINTERS
    static const void * NormalizePointer(const void *pNormalize);
#endif

/*  Our output handlers are written for the Datalight standard
    fixed width data types.  Map the standard ANSI C data types
    onto our handlers.  Currently this code has the following
    requirements:

    1) shorts must be either 16 or 32 bits
    2) ints must be either 16 or 32 bits
    3) longs must be between 32 or 64 bits
*/
#if (USHRT_MAX == 0xFFFF)
  #define MAPSHORT          DCLPRFMT_SIGNED16BIT
  #define MAPUSHORT         DCLPRFMT_UNSIGNED16BIT
  #define MAPHEXUSHORT      DCLPRFMT_HEX16BIT
#elif (USHRT_MAX == 0xFFFFFFFF)
  #define MAPSHORT          DCLPRFMT_SIGNED32BIT
  #define MAPUSHORT         DCLPRFMT_UNSIGNED32BIT
  #define MAPHEXUSHORT      DCLPRFMT_HEX32BIT
#else
  #error "DCL: The 'short' data type does not have a 16 or 32-bit width"
#endif

#if (UINT_MAX == 0xFFFF)
  #define MAPINT            DCLPRFMT_SIGNED16BIT
  #define MAPUINT           DCLPRFMT_UNSIGNED16BIT
  #define MAPHEXUINT        DCLPRFMT_HEX16BIT
#elif (UINT_MAX == 0xFFFFFFFF)
  #define MAPINT            DCLPRFMT_SIGNED32BIT
  #define MAPUINT           DCLPRFMT_UNSIGNED32BIT
  #define MAPHEXUINT        DCLPRFMT_HEX32BIT
#else
  #error "DCL: The 'int' data type does not have a 16 or 32-bit width"
#endif

#if (ULONG_MAX == 0xFFFFFFFF)
  #define MAPLONG           DCLPRFMT_SIGNED32BIT
  #define MAPULONG          DCLPRFMT_UNSIGNED32BIT
  #define MAPHEXULONG       DCLPRFMT_HEX32BIT
#else
  /*  We've run into unusual environments where "longs" are 40-bits wide.
      In this event, map them to 64-bit types so no data is lost. 
  */      
  #if (ULONG_MAX <= UINT64SUFFIX(0xFFFFFFFFFFFFFFFF))
    #define MAPLONG         DCLPRFMT_SIGNED64BIT
    #define MAPULONG        DCLPRFMT_UNSIGNED64BIT
    #define MAPHEXULONG     DCLPRFMT_HEX64BIT
  #endif
#endif

#ifndef MAPLONG
  #error "DCL: The 'long' data type is not between 32 and 64 bits wide"
#endif


/*-------------------------------------------------------------------
    Public: DclVSNPrintf()

    Format arguments into a string using a subset of the ANSI C
    vsprintf() functionality, with several extensions to support
    Datalight specific requirements.

    This function is modeled after the Microsoft _vsnprint()
    extension to the ANSI C vsprintf() function, and requires
    a buffer length to be specified so that overflow is avoided.

    The following ANSI standard formatting codes are supported.
      %c         - Format a character
      %s         - Format a null-terminated ANSI string
      %hd  %hu   - Format a signed/unsigned short
      %d   %u    - Format a signed/unsigned integer
      %ld  %lu   - Format a signed/unsigned long
      %hx        - Format a short in hex
      %x         - Format an integer in hex
      %lx        - Format a long in hex

    The following Datalight specific formatting codes are supported.
      %W         - Format a Unicode string
      %P         - Format a pointer (32-bit hex value)
      %D   %U    - Format a fixed 16-bit signed/unsigned integer
      %lD  %lU   - Format a fixed 32-bit signed/unsigned integer
      %llD %llU  - Format a fixed 64-bit signed/unsigned integer
      %X         - Format a fixed 16-bit integer in hex
      %lX        - Format a fixed 32-bit integer in hex
      %llX       - Format a fixed 64-bit integer in hex
      %T         - Format a D_TIME value (passed by address)

    Fill characters and field widths are supported per the ANSI
    standard, as is left justification with the '-' character.

    The only supported fill characters are '0' and ' '.

    '*' is supported to specify variable length field widths.

    Unicode String Support:
      In environments where Unicode is supported, the format
      strings must still be ANSI.  Any Unicode string parameters
      are converted to ANSI in the output buffer.

      If %W is used in an environment where DCL_OSFEATURE_UNICODE
      is FALSE, the string will be treated as a standard ANSI
      string.  This allows common code which may be compiled for
      both ANSI and Unicode environments to properly display
      strings where the type is not known.

    Special behavior for hex numbers:
    - If a fill length is not specified, the output number will
      always be prefixed with a "0x", and number will consume the
      full potential width (4, 8, or 16 characters for 16, 32, or
      64-bit numbers), and the number will be padded with leading
      zeros as needed.

    - Note that hexadecimal displays are always uppercase with the
      exception of the leading "0x".

    Formatting the time with %T:
      The %T format specifier allows a D_TIME value to be output
      in the form: "Jul 05 13:08:27 2003".  The D_TIME value must
      be passed by address.  If a NULL pointer is passed instead
      the current time will be queried with DclOsDateTimeGet().

    Unsupported behaviors:
      - Precision is not supported.
      - Floating point is not supported.

    Errata:
    - Systems where pointers are something other than 32-bits
      will get undefined behavior when using %P.

    - Displaying 64-bit signed numbers with a leading fill
      character will result in the sign being displayed to the
      left of the fill characters (need a DclLLtoA function).

    - ANSI string parameters which have a null pointer will
      display the text "null", however Unicode string parameters
      which have a null pointer will display nothing.

    - There is a subtle difference in the return value for this
      function versus the Microsoft implementation.  In the
      Microsoft version, if the buffer exactly fills up, but
      there is no room for a null-terminator, the return value
      will be the length of the buffer.  In this code, -1 will
      be returned when this happens.

    - When using left justified strings, the only supported fill
      character is a space, regardless of what may be specified.
      It is not clear if this is ANSI standard or just the way
      the Microsoft function works, but we emulate the Microsoft
      behavior.

    Parameters:
        pBuffer    - A pointer to the output buffer
        nBufferLen - The output buffer length
        pszFmt     - A pointer to the null terminated ANSI format
                     string
        arglist    - Variable argument list

    Return Value:
        Returns the length output, or -1 if the buffer filled up.
        If -1 is returned, the output buffer may not be null-
        terminated.
-------------------------------------------------------------------*/
int DclVSNPrintf(
    char           *pBuffer,
    int             nBufferLen,
    const char     *pszFmt,
    va_list         arglist)
{
    char           *pPos = pBuffer;
    size_t          nRemaining = (size_t)nBufferLen;

    DclAssert(nBufferLen);
    DclAssertWritePtr(pBuffer, nBufferLen);
    DclAssertReadPtr(pszFmt, 0);

    while(*pszFmt && nRemaining)
    {
        DCLPRINTFORMAT  Format;
        size_t          nSpecifierLen;
        size_t          nWidth;

        /*  Process the next segment of the format string, outputting
            any non-format specifiers, as output buffer space allows,
            and return information about the next format specifier.
        */
        nWidth = DclPrintfProcessFormatSegment(pPos, nRemaining, pszFmt, &Format, &nSpecifierLen);
        if(nWidth)
        {
            DclAssert(nWidth <= nRemaining);
            
            pPos        += nWidth;
            nRemaining  -= nWidth;
        }
        
        /*  If no specifier was found, or if the output buffer is
            full, we're done -- get out.
        */                
        if(!nSpecifierLen || !nRemaining)
            break;

        /*  Otherwise, the math should add up for these things...
        */
        DclAssert(Format.pszSpecifier == pszFmt + nWidth);

        /*  Point past the specifier, to the next piece of the format string.
        */
        pszFmt = Format.pszSpecifier + nSpecifierLen;

        if(Format.pVariableWidthChar)
            Format.nFillLen = va_arg(arglist, int);

        switch(Format.nType)
        {
            case DCLPRFMT_DOUBLEPERCENT:
                /*  Nothing to do.  A single percent has already been
                    output, and we just finished skipping past the 
                    second percent.
                */
                break;

            case DCLPRFMT_CHAR:
                *pPos++ = (char)va_arg(arglist, int);
                nRemaining--;
                break;

            /*----------------->  16-bit int handling  <-----------------
             *
             *  Fixed with 16-bit values will be promoted to "int" by
             *  the compiler, so we must retrieve them using "int" when
             *  calling va_arg().  Once we've done that, we immediately
             *  put the value into the desired data type.
             *---------------------------------------------------------*/

            case DCLPRFMT_SIGNED16BIT:
            {
                D_INT16 nNum = (D_INT16)va_arg(arglist, int);

                pPos += DclLtoA(pPos, &nRemaining, (D_INT32)nNum, Format.nFillLen, Format.cFillChar);

                break;
            }

            case DCLPRFMT_UNSIGNED16BIT:
            {
                D_UINT16 uNum = (D_UINT16)va_arg(arglist, unsigned);

                pPos += DclULtoA(pPos, &nRemaining, (D_UINT32)uNum, 10, Format.nFillLen, Format.cFillChar);
                break;
            }

            case DCLPRFMT_HEX16BIT:
            {
                D_UINT16 uNum = (D_UINT16)va_arg(arglist, unsigned);
              
                /*  Special behavior -- for hex numbers where a fill
                    length is not specified, always prefix with "0x",
                    and force a full fill length with '0' as the fill
                    char.
                */
                if(!Format.nFillLen)
                {
                    *pPos++ = '0';
                    nRemaining--;
                    if(!nRemaining)
                        break;
                    *pPos++ = 'x';
                    nRemaining--;
                    if(!nRemaining)
                        break;
                    Format.nFillLen = 4;
                    Format.cFillChar = '0';
                }

                pPos += DclULtoA(pPos, &nRemaining, (D_UINT32)uNum, 16, Format.nFillLen, Format.cFillChar);
                break;
            }

            case DCLPRFMT_SIGNED32BIT:
            {
                D_INT32 lNum = va_arg(arglist, D_INT32);

                pPos += DclLtoA(pPos, &nRemaining, lNum, Format.nFillLen, Format.cFillChar);
                break;
            }

            case DCLPRFMT_UNSIGNED32BIT:
            {
                D_UINT32 ulNum = va_arg(arglist, D_UINT32);

                pPos += DclULtoA(pPos, &nRemaining, ulNum, 10, Format.nFillLen, Format.cFillChar);
                break;
            }

            case DCLPRFMT_POINTER:
            {
                const void * ptr = va_arg(arglist, void *);

              #if MAX_NORMALIZED_POINTERS
                ptr = NormalizePointer(ptr);
              #endif

                /*  Prefix with 0x and pad with zeros.  Have no method
                    currently for abstracting the pointer width -- assumes
                    32-bits for now.
                */
                *pPos++ = '0';
                nRemaining--;
                if(!nRemaining)
                    break;
                *pPos++ = 'x';
                nRemaining--;
                if(!nRemaining)
                    break;
                Format.nFillLen = 8;
                Format.cFillChar = '0';

                pPos += DclULtoA(pPos, &nRemaining, (D_UINTPTR)ptr, 16, Format.nFillLen, Format.cFillChar);
                break;
            }

            case DCLPRFMT_HEX32BIT:
            {
                D_UINT32 ulNum = va_arg(arglist, D_UINT32);

                /*  Special behavior -- for hex numbers where a fill
                    length is not specified, always prefix with "0x",
                    and force a full fill length with '0' as the fill
                    char.
                */
                if(!Format.nFillLen)
                {
                    *pPos++ = '0';
                    nRemaining--;
                    if(!nRemaining)
                        break;
                    *pPos++ = 'x';
                    nRemaining--;
                    if(!nRemaining)
                        break;
                    Format.nFillLen = 8;
                    Format.cFillChar = '0';
                }

                pPos += DclULtoA(pPos, &nRemaining, ulNum, 16, Format.nFillLen, Format.cFillChar);
                break;
            }

            /*----------------->  64-bit int handling  <-----------------
             *
             *  We must pass standard C data types to va_arg() rather
             *  than our special data types.  We know that for this
             *  environment definition of long is always 32-bits wide
             *  (preprocessor logic at the top of this module).
             *---------------------------------------------------------*/

          #if !DCL_NATIVE_64BIT_SUPPORT
            case DCLPRFMT_SIGNED64BIT:
                /*  We don't have the necessary macros to handle emulated
                    signed 64-bit support, however we don't currently use
                    D_INT64 types anywhere in the code (as of 6/1/2006).
                */
                DclProductionError();
                break;
          #else
            case DCLPRFMT_SIGNED64BIT:
            {
              #if VAARG64_BROKEN
                D_INT64 llNum = *(va_arg(arglist, const D_INT64*)); 
              #else
                D_INT64 llNum = va_arg(arglist, D_INT64);
              #endif

                if(llNum < 0)
                {
                    *pPos++ = '-';
                    nRemaining--;
                    llNum = -llNum;
                    if(Format.nFillLen)
                        Format.nFillLen--;
                }
                pPos += DclULLtoA(pPos, &nRemaining, (D_UINT64*)&llNum, 10, Format.nFillLen, Format.cFillChar);
                break;
            }
          #endif

            case DCLPRFMT_UNSIGNED64BIT:
            {
              #if VAARG64_BROKEN
                D_UINT64 ullNum = *(va_arg(arglist, const D_UINT64*)); 
              #else
                D_UINT64 ullNum = va_arg(arglist, D_UINT64); 
              #endif

                pPos += DclULLtoA(pPos, &nRemaining, &ullNum, 10, Format.nFillLen, Format.cFillChar);
                break;
            }

            case DCLPRFMT_HEX64BIT:
            {
              #if VAARG64_BROKEN
                D_UINT64 ullNum = *(va_arg(arglist, const D_UINT64*)); 
              #else
                D_UINT64 ullNum = va_arg(arglist, D_UINT64);
              #endif

                /*  Special behavior -- for hex numbers where a fill
                    length is not specified, always prefix with "0x",
                    and force a full fill length with '0' as the fill
                    char.
                */
                if(!Format.nFillLen)
                {
                    *pPos++ = '0';
                    nRemaining--;
                    if(!nRemaining)
                        break;
                    *pPos++ = 'x';
                    nRemaining--;
                    if(!nRemaining)
                        break;
                    Format.nFillLen = 16;
                    Format.cFillChar = '0';
                }

                pPos += DclULLtoA(pPos, &nRemaining, &ullNum, 16, Format.nFillLen, Format.cFillChar);
                break;
            }

            case DCLPRFMT_TIME:
            {
                char            szTime[32];
                size_t          nLen;
                D_TIME          Time;
                const D_TIME   *pTime = va_arg(arglist, const D_TIME*); 

                if(pTime)
                {
                    Time = *pTime;
                }
                else
                {
                    /*  If a NULL pointer is supplied, use the current time.
                    */
                    if(!DclOsGetDateTime(&Time))
                        Time = 0;
                }
   
                if(!DclDateTimeString(&Time, szTime, sizeof(szTime)))
                {
                    DclError();
                    szTime[0] = 0;
                }
                
                nLen = DclStrLen(szTime);

                if(nRemaining >= nLen)
                {
                    DclStrCpy(pPos, szTime);
                    pPos += nLen;
                    nRemaining -= nLen;
                }
                break;
            }

          #if DCL_OSFEATURE_UNICODE
            case DCLPRFMT_UNICODESTRING:
            {
                char *pArg = va_arg(arglist, char *);

                /*  Only if a valid pointer is supplied...
                */
                if(pArg)
                {
                    size_t nLen;

                    /*  Calculate how much buffer space we will need,
                        NOT including the terminating null.
                    */
                    nLen = DclOsWcsToAnsi(NULL, 0, (D_WCHAR*)pArg, -1) - 1;

                    if(Format.nFillLen)
                    {
                        if(!Format.fLeftJustified)
                        {
                            /*  So long as we are not left justifying, fill
                                as many characters as is necessary to make
                                the string right justified.
                            */
                            while(nRemaining && Format.nFillLen > nLen)
                            {
                                *pPos++ = Format.cFillChar;
                                nRemaining--;
                                Format.nFillLen--;
                            }
                        }

                        /*  Convert as many characters as we have space for
                            into the output buffer.  This will not include
                            any terminating null since we already removed
                            that from the length.
                        */
                        nLen = DclOsWcsToAnsi(pPos, nRemaining, (D_WCHAR*)pArg, DCLMIN(nLen, nRemaining));
                        pPos        += nLen;
                        nRemaining  -= nLen;
                        if(Format.nFillLen >= nLen)
                            Format.nFillLen -= nLen;
                        else
                            Format.nFillLen = 0;

                        /*  If there is any space left to fill, do it (the
                            string must have been left justified).
                        */
                        while(nRemaining && Format.nFillLen > 0)
                        {
                            /*  This is NOT a typo -- when using left
                                justified strings, spaces are the only
                                allowed fill character.  See the errata.
                            */
                            *pPos++ = ' ';
                            nRemaining--;
                            Format.nFillLen--;
                        }
                    }
                    else
                    {
                        /*  No fill characters, just move up to as many
                            characters as we have space for in the output
                            buffer.
                        */
                        nLen = DclOsWcsToAnsi(pPos, nRemaining, (D_WCHAR*)pArg, DCLMIN(nLen, nRemaining));
                        pPos        += nLen;
                        nRemaining  -= nLen;
                    }
                }
                break;
            }
          #else
            case DCLPRFMT_UNICODESTRING:
                /*  Unicode is not supported, so fall into the ANSI
                    string case....
                */
          #endif

            case DCLPRFMT_ANSISTRING:
            {
                char *pArg = va_arg(arglist, char *);
                if(!pArg)
                    pArg = "null";

                if(Format.nFillLen)
                {
                    if(!Format.fLeftJustified)
                    {
                        size_t nLen = DclStrLen(pArg);

                        /*  So long as we are not left justifying, fill
                            as many characters as is necessary to make
                            the string right justified.
                        */
                        while(nRemaining && Format.nFillLen > nLen)
                        {
                            *pPos++ = Format.cFillChar;
                            nRemaining--;
                            Format.nFillLen--;
                        }
                    }

                    /*  Move as many characters as we have space for
                        into the output buffer.
                    */
                    while(nRemaining && *pArg)
                    {
                        *pPos++ = *pArg++;
                        nRemaining--;
                        if(Format.nFillLen)
                            Format.nFillLen--;
                    }

                    /*  If there is any space left to fill, do it (the
                        string must have been left justified).
                    */
                    while(nRemaining && Format.nFillLen > 0)
                    {
                        /*  This is NOT a typo -- when using left
                            justified strings, spaces are the only
                            allowed fill character.  See the errata.
                        */
                        *pPos++ = ' ';
                        nRemaining--;
                        Format.nFillLen--;
                    }
                }
                else
                {
                    /*  No fill characters, just move up to as many
                        characters as we have space for in the output
                        buffer.
                    */
                    while(nRemaining && *pArg)
                    {
                        *pPos++ = *pArg++;
                        nRemaining--;
                    }
                }
                break;
            }

            default:
                DclError();
                break;
        }
    }

    /*  If there is space, tack on a null and return the output
        length processed, not including the null.
    */
    if(nRemaining)
    {
        *pPos = 0;

        return (int)(pPos - pBuffer);
    }
    else
    {
        /*  Not enough space, just return -1, with no null termination
        */
        return -1;
    }
}


/*-------------------------------------------------------------------
    Public: DclSNPrintf()

    Format arguments into a string using a subset of the ANSI C
    vsprintf() functionality, with several extensions to support
    Datalight specific requirements.

    This function is modeled after the Microsoft _snprint()
    extension to the ANSI C sprintf() function, and allows
    a buffer length to be specified so that overflow is avoided.

    See DclVSNPrintf() for the list of supported types.

    Parameters:
        pBuffer    - A pointer to the output buffer
        nBufferLen - The output buffer length
        pszFmt     - A pointer to the null terminated format string
        ...        - Variable argument list

    Return Value:
        Returns the length output, or -1 if the buffer filled up.
        If -1 is returned, the output buffer may not be null-
        terminated.
-------------------------------------------------------------------*/
int DclSNPrintf(
    char           *pBuffer,
    int             nBufferLen,
    const char     *pszFmt,
    ...)
{
    int             iLen;
    va_list         arglist;

    DclAssert(pBuffer);
    DclAssert(pszFmt);

    va_start(arglist, pszFmt);

    /*  call our workhorse function
    */
    iLen = DclVSNPrintf(pBuffer, nBufferLen, pszFmt, arglist);

    va_end(arglist);

    return iLen;
}


/*-------------------------------------------------------------------
    Protected: DclPrintfProcessFormatSegment()

    Process the next segment of the format string, outputting any
    non-format specifiers, as output buffer space allows, and 
    return information about the next format specifier.

    *Note* -- If the returned value is the same as the supplied
              nBufferLen, the output buffer will not be null-
              terminated.  In all other cases, the result will
              be null-terminated.  The returned length will never
              include the null in the count.

    Parameters:
        pBuffer        - The output buffer.
        nBufferLen     - The output buffer length.
        pszFmt         - The format string to process.
        pFormat        - The DCLPRINTFORMAT structure to fill.
        pnSpecifierLen - Returns the length of any format specifier
                         string, or zero if no specifier was found.
 
    Return Value:
        Returns the count of characters from pszFmt which were 
        processed and copied to pBuffer.  
        - If zero is returned and *pnSpecifierLen is non-zero, then
          a format specifier string was found at the start of pszFmt.
        - If non-zero is returned and *pnSpecifierLen is zero, then
          no format specifier string was found, and the entire pszFmt
          string was copied to pBuffer (or as much as will fit).
-------------------------------------------------------------------*/
size_t DclPrintfProcessFormatSegment(
    char           *pBuffer,
    size_t          nBufferLen,
    const char     *pszFmt,
    DCLPRINTFORMAT *pFormat,
    size_t         *pnSpecifierLen)
{
    size_t          nWidth = 0;

    DclAssert(nBufferLen);
    DclAssertReadPtr(pszFmt, 0);
    DclAssertWritePtr(pBuffer, nBufferLen);
    DclAssertWritePtr(pFormat, sizeof(*pFormat));
    DclAssertWritePtr(pnSpecifierLen, sizeof(*pnSpecifierLen));

    /*  Find the next format specifier string, and information about it.
    */
    *pnSpecifierLen = DclPrintfParseFormatSpecifier(pszFmt, pFormat);

    if(!*pnSpecifierLen)
    {
        /*  If no specifier was found at all, then simply output the
            full length of the string, or as much as will fit.
         */                
        nWidth = DCLMIN(nBufferLen, DclStrLen(pszFmt));
        
        DclMemCpy(pBuffer, pszFmt, nWidth);
    }
    else
    {
        /*  If we encountered a double percent, skip past one of them so
            it is copied into the output buffer.
        */  
        if(pFormat->nType == DCLPRFMT_DOUBLEPERCENT)
        {
            pFormat->pszSpecifier++;

            /*  A double percent specifier always has a length of 
                two.  Since we're processing one of those percent
                signs, reduce the length to one.  Assert it so.
            */    
            DclAssert(*pnSpecifierLen == 2);
             
            (*pnSpecifierLen)--;
        }

        /*  So long as the specifier is not the very first thing in the 
            format string...
        */    
        if(pFormat->pszSpecifier != pszFmt)
        {
            /*  A specifier was found, but there is other data preceding
                it.  Copy as much as allowed to the output buffer.
            */                
            nWidth = DCLMIN(nBufferLen, (size_t)(pFormat->pszSpecifier - pszFmt));
             
            DclMemCpy(pBuffer, pszFmt, nWidth);
        }
    }
    
    /*  If there is room in the output buffer, null-terminate whatever is
        there.  But note that the returned length never includes the null.
    */        
    if(nWidth < nBufferLen)
        pBuffer[nWidth] = 0;

    return nWidth;
}


/*-------------------------------------------------------------------
    Protected: DclPrintfParseFormatSpecifier()

    Parse the specified format string for a valid DclVSNPrintf()
    format sequence, and return information about it.

    Parameters:
        pszFmt      - The format string to process.
        pFormatType - The DCLPRINTFORMAT structure to fill.  The data
                      is only valid if a non-zero length is returned.
 
    Return Value:
        Returns the length of the full format specifier string, 
        starting at pFormatType->pszSpecifier.  Returns zero if
        a valid specifier was not found.
-------------------------------------------------------------------*/
size_t DclPrintfParseFormatSpecifier(
    char const     *pszFmt,
    DCLPRINTFORMAT *pFormatType)
{
    D_BOOL          fContainsIllegalSequence = FALSE;
    
    DclAssertReadPtr(pszFmt, 0);
    DclAssertWritePtr(pFormatType, sizeof(*pFormatType));

    while(*pszFmt)
    {
        size_t      nTypeLen;
        
        /*  general output
        */
        if(*pszFmt != '%')
        {
            pszFmt++;
            continue;
        }

        DclMemSet(pFormatType, 0, sizeof(*pFormatType));

        /*  Record the location of the start of the format sequence
        */
        pFormatType->pszSpecifier = pszFmt;

        pszFmt++;
        
        if(*pszFmt == '-')
        {
            pFormatType->fLeftJustified = TRUE;
            pszFmt++;
        }

        if(*pszFmt == '0')
        {
            pFormatType->cFillChar = '0';
            pszFmt++;
        }
        else
        {
            pFormatType->cFillChar = ' ';
        }

        if(*pszFmt == '*')
        {
            pFormatType->pVariableWidthChar = pszFmt;
            pszFmt++;
        }
        else if(DclIsDigit(*pszFmt))
        {
            pFormatType->nFillLen = DclAtoI(pszFmt);
            while(DclIsDigit(*pszFmt))
                pszFmt++;
        }

        pFormatType->nType = DclPrintfParseFormatType(pszFmt, &nTypeLen);
        if(pFormatType->nType != DCLPRFMT_UNKNOWN)
        {
            /*  Even though we are returning successfully, keep track of
                whether an illegal sequence was encountered and skipped.
            */                
            pFormatType->fHasIllegalType = fContainsIllegalSequence;
    
            return (pszFmt - pFormatType->pszSpecifier) + nTypeLen;
        }

        /*  In the case of an unrecognized type string, simply ignore
            it entirely.  Reset the pointer to the position following
            the percent sign, so it is not found again.
        */            
        fContainsIllegalSequence = TRUE;
        pszFmt = pFormatType->pszSpecifier + 1;
    }

    return 0;
}


/*-------------------------------------------------------------------
    Protected: DclPrintfParseFormatType()

    Parse a DclPrintf() format type string to determine the proper
    data type.

    Parameters:
        pszFmt      - The format string to process.  This must be a
                      pointer to the character following any width
                      or justification characters.
        pnTypeLen   - The location in which to store the type length.
                      The value will be 0 if DCLPRFMT_UNKNOWN is
                      returned.

    Return Value:
        Returns the DCLPRFMT_* type value, or DCLPRFMT_UNKNOWN if the
        type is not recognized.
-------------------------------------------------------------------*/
DCLPRINTTYPE DclPrintfParseFormatType(
    const char     *pszFmt,
    size_t         *pnTypeLen)
{
    DCLPRINTTYPE    fmtType = DCLPRFMT_UNKNOWN;
    const char     *pszOriginal = pszFmt;

    DclAssertReadPtr(pszFmt, 0);
    DclAssertWritePtr(pnTypeLen, sizeof(*pnTypeLen));

    switch(*pszFmt)
    {
        case 0:
            /*  Just get out if we have run out of input
            */
            break;
            
        case '%':
            fmtType = DCLPRFMT_DOUBLEPERCENT;
            break;
            
        case 'c':
            fmtType = DCLPRFMT_CHAR;
            break;
            
        case 's':
            fmtType = DCLPRFMT_ANSISTRING;
            break;
            
        case 'W':
            fmtType = DCLPRFMT_UNICODESTRING;
            break;
            
        case 'P':
            fmtType = DCLPRFMT_POINTER;
            break;
            
        case 'd':
            fmtType = MAPINT;
            break;
            
        case 'D':
            fmtType = DCLPRFMT_SIGNED16BIT;
            break;
            
        case 'u':
            fmtType = MAPUINT;
            break;
            
        case 'U':
            fmtType = DCLPRFMT_UNSIGNED16BIT;
            break;
            
        case 'x':
            fmtType = MAPHEXUINT;
            break;
            
        case 'X':
            fmtType = DCLPRFMT_HEX16BIT;
            break;
            
        case 'T':
            fmtType = DCLPRFMT_TIME;
            break;
            
        case 'h':
            pszFmt++;
            switch(*pszFmt)
            {
                case 0:
                    /*  Just get out if we have run out of input
                    */
                    break;
                    
                case 'd':
                    fmtType = MAPSHORT;
                    break;
                    
                case 'u':
                    fmtType = MAPUSHORT;
                    break;
                    
                case 'x':
                    fmtType = MAPHEXUSHORT;
                    break;
             }
            break;
            
        case 'l':
            pszFmt++;
            switch(*pszFmt)
            {
                case 0:
                    /*  Just get out if we have run out of input
                    */
                    break;
                    
                case 'd':
                    fmtType = MAPLONG;
                    break;
                    
                case 'u':
                    fmtType = MAPULONG;
                    break;
                    
                case 'x':
                    fmtType = MAPHEXULONG;
                    break;
                    
                case 'D':
                    fmtType = DCLPRFMT_SIGNED32BIT;
                    break;
                    
                case 'U':
                    fmtType = DCLPRFMT_UNSIGNED32BIT;
                    break;
                    
                case 'X':
                    fmtType = DCLPRFMT_HEX32BIT;
                    break;
                    
                case 'l':
                    pszFmt++;
                    switch(*pszFmt)
                    {
                        case 0:
                            /*  Just get out if we have run out of input
                            */
                            break;
                            
                        case 'd':
                        case 'D':
                            fmtType = DCLPRFMT_SIGNED64BIT;
                            break;
                            
                        case 'u':
                        case 'U':
                            fmtType = DCLPRFMT_UNSIGNED64BIT;
                            break;
                            
                        case 'X':
                            fmtType = DCLPRFMT_HEX64BIT;
                            break;
                    }
                    break;
            }
            break;
    }

    if(fmtType != DCLPRFMT_UNKNOWN)
        *pnTypeLen = (pszFmt+1) - pszOriginal;
    else
        *pnTypeLen = 0;

    return fmtType;
}


#if MAX_NORMALIZED_POINTERS

/*-------------------------------------------------------------------
    Local: NormalizePointer()

    This function normalizes a pointer value by storing it in a
    static array, and returning a pseudo pointer value consisting
    of the index into that array.  If the table fills up, the
    original pointer will be returned, unmodified.  A normalized
    pointer has the form: 0xFFseqnum.

    NULL pointers and pointers which are all 0xFFs are ignored.

    Parameters:
        pNormalize - The pointer to normalize.

    Return Value:
        Returns the normalized pointer value.  The value may or may
        not be modified.
-------------------------------------------------------------------*/
static const void * NormalizePointer(
    const void         *pNormalize)
{
    static const void * apNormalized[MAX_NORMALIZED_POINTERS];
    unsigned            nn;

    /*  Do nothing with NULL and all 0xFF pointers.
    */
    if(!pNormalize || (D_UINTPTR)pNormalize == D_UINT32_MAX)
        return pNormalize;

    for(nn=0; nn<MAX_NORMALIZED_POINTERS; nn++)
    {
        /*  If we found a match, return the normalized pointer value
        */
        if(apNormalized[nn] == pNormalize)
            return (const void*)(D_UINTPTR)(0xFF000000 | nn);

        /*  If we found a blank slot, break out of the loop since we
            know there can be nothing else.
        */
        if(apNormalized[nn] == NULL)
            break;
    }

    /*  If there is no more room in the table, just return the
        original pointer.
    */
    if(nn >= MAX_NORMALIZED_POINTERS)
        return pNormalize;

    apNormalized[nn] = pNormalize;

    return (const void*)(D_UINTPTR)(0xFF000000 | nn);
}

#endif

