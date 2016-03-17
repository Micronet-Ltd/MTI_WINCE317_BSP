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

    This module contains tests for UTF-8 string manipulation.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltutf8.c $
    Revision 1.12  2009/07/01 23:09:49Z  keithg
    Corrected error reporting code in the test case for invalid UTF-8 sequences.
    Revision 1.11  2009/06/28 22:48:21Z  garyp
    Updated to include dltests.h.
    Revision 1.10  2009/05/26 21:07:09Z  keithg
    Corrected variable names for the shared UTF-8 case data.
    Revision 1.9  2009/05/26 15:56:00Z  keithg
    Moved dltutf_casedata.h into the public include tree.
    Revision 1.8  2009/05/20 19:48:49Z  keithg
    Added tests for DclUtf8StrICmp() and additional cases with differing
    string lengths.
    Revision 1.7  2009/05/18 00:53:31Z  keithg
    Added test cases for UTF-8 string tolower and strnicmp functions.
    Revision 1.6  2009/05/15 01:42:41Z  keithg
    Updated to use the renamed UTF to lower functions.
    Revision 1.5  2009/05/15 01:03:28Z  keithg
    Added negative cases for UTF case folding.
    Revision 1.4  2009/05/15 00:40:18Z  keithg
    Added test case for unicode lower case functionality.
    Revision 1.3  2009/05/14 23:07:40Z  keithg
    Added comments with references to the Unicode Standard and
    introduced cases for invalid, short, and long conditions.
    Revision 1.2  2009/05/08 21:43:03Z  jimmb
    Changed unsigned to size_t to match the prototype
    Revision 1.1  2009/05/08 17:54:34Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapireq.h>
#include <dlstats.h>
#include <dltools.h>
#include <test/dltutf_casedata.h>
#include "dltests.h"

#define UTF8_MAX_SCALAR    0x10ffffU
#define UTF8_MAX_SEQUENCE  4

struct _tagUtfStringTestData {
    char *szString1;
    size_t nString1Len;
    char *szString2;
    size_t nString2Len;
    char *szWorking;
    size_t nWorkingLen;
    char *szExpected;
    size_t nExpectedLen;

    size_t nExpectedString1Processed;
    size_t nExpectedString2Processed;
    int    iExpectedCompare;
    unsigned uExpectedCompareFlags;
};



static D_BOOL CheckUtfStringNICmp(struct _tagUtfStringTestData *pct)
{
    size_t nString1Processed;
    size_t nString2Processed;
    unsigned uCompareFlags;
    int iResult;

    uCompareFlags = 0;
    iResult = DclUtf8StrNICmp(
        pct->szString1, pct->szString2,
        pct->nString1Len, pct->nString2Len,
        &nString1Processed, &nString2Processed, &uCompareFlags);

    DclPrintf("    DclUtf8StrNICmp(): \"%s\" AND \"%s\"", pct->szString1, pct->szString2);

    if(nString1Processed != pct->nExpectedString1Processed)
    {
        DclPrintf("\nFailed, unexpected length processed, expected %2d got %2d!\n",
            pct->nExpectedString1Processed, nString1Processed);
        return FALSE;
    }
    if(nString2Processed != pct->nExpectedString2Processed)
    {
        DclPrintf("\nFailed, unexpected length processed, expected %2d got %2d!\n",
            pct->nExpectedString1Processed, nString1Processed);
        return FALSE;
    }
    if(iResult != pct->iExpectedCompare)
    {
        DclPrintf("\nFailed, unexpected compare result, expected %2d got %2d!\n",
            pct->iExpectedCompare, iResult);
        return FALSE;
    }
    if(pct->uExpectedCompareFlags != uCompareFlags)
    {
        DclPrintf("\nFailed, unexpected compare flags, expected %2d got %2d!\n",
            pct->uExpectedCompareFlags, uCompareFlags);
        return FALSE;
    }

    DclPrintf(" OK\n");
    return TRUE;
}



static D_BOOL CheckUtfStringICmp(struct _tagUtfStringTestData *pct)
{
    int iResult;

    iResult = DclUtf8StrICmp(pct->szString1, pct->szString2);

    DclPrintf("    DclUtf8StrICmp(): \"%s\" AND \"%s\"", pct->szString1, pct->szString2);
    if(iResult != pct->iExpectedCompare)
    {
        DclPrintf("\nFailed, unexpected compare result, expected %2d got %2d!\n",
            pct->iExpectedCompare, iResult);
        return FALSE;
    }

    DclPrintf(" OK\n");
    return TRUE;
}



static D_BOOL CheckUtfStringToLower(struct _tagUtfStringTestData *pct)
{
    size_t nIn;
    size_t nOut;

    DclUtf8StringToLower(
        pct->szString1, pct->nString1Len,
        pct->szWorking, pct->nWorkingLen,
        &nIn, &nOut);

    DclPrintf("    To lower: %s --> %s\n", pct->szString1, pct->szWorking);

    if(nIn > pct->nString1Len)
    {
        DclPrintf("Failed, incorect input length reported %d expected <= %d\n", nIn, pct->nString1Len);
        return FALSE;
    }
    if(nOut > pct->nWorkingLen)
    {
        DclPrintf("Failed, incorect output length reported %d expected <= %d)\n", nOut, pct->nWorkingLen);
        return FALSE;
    }
    if(nOut != pct->nExpectedLen)
    {
        DclPrintf("Failed, output length was %d, but expected %d\n", nOut, pct->nExpectedLen);
        return FALSE;
    }

    if(DclMemCmp(pct->szExpected, pct->szWorking, pct->nExpectedLen) != 0)
    {
        DclPrintf("Failed, string compare failed!\n");
        return FALSE;
    }

    return TRUE;
}



DCLSTATUS DclTestUtf8(void)
{
    unsigned    nn, ii;
    unsigned    nSize;
    char        acBuff[10];
    size_t      nProcessed;
    D_UINT32    ulUtfScalar;
    D_UINT32    ulUCS4New;

    /*  Quiet this test down a bit by only printing about 1 out of
        each 100+ test cases in the list.  Change to 1 to display
        all test case data.
    */
    const unsigned mm = 97;

    DclPrintf("Testing UTF-8 Functions...\n");

    DclPrintf("  Verify consistency of valid UTF-8 boundary scalars...\n");

    for(ulUtfScalar = 0; ulUtfScalar <= UTF8_MAX_SCALAR; ulUtfScalar++)
    {

        /*  Verify encoding of valid sequences prior to and after
            each boundary condition.

            0x00..0x7f          single byte sequences
            0x80..0x7ff         two byte sequences
            0x800..0xffff       three byte sequences
            0x10000..0x10ffff   last valid UTF-8 sequence
            0xd800..0xdfff      surrogate pairs (invalid utf-8 scalars)
        */
        if(ulUtfScalar == 0x08)
            ulUtfScalar = 0x78;
        if(ulUtfScalar == 0x88)
            ulUtfScalar = 0x7F8;
        if(ulUtfScalar == 0x808)
            ulUtfScalar = 0xD7F8;
        if(ulUtfScalar == 0xD800)
            ulUtfScalar = 0xE000;
        if(ulUtfScalar == 0xE008)
            ulUtfScalar = 0x10000;
        if(ulUtfScalar == 0x10008)
            ulUtfScalar = 0x10FFF8;

        nSize = DclUtf8FromScalar(ulUtfScalar, acBuff, sizeof(acBuff));
        if(!nSize)
        {
            DclPrintf("Conversion of %lX to UTF-8 failed!\n", ulUtfScalar);
            return DCLSTAT_CURRENTLINE;
        }

        DclPrintf("    %lX %02x", ulUtfScalar, (D_UINT8)acBuff[0]);
        for(ii=1; ii<nSize; ii++)
            DclPrintf(" %02x", (D_UINT8)acBuff[ii]);
        DclPrintf("\n");

        ulUCS4New = DclUtf8ToScalar(acBuff, sizeof(acBuff), &nProcessed);

        if(ulUCS4New != ulUtfScalar)
        {
            DclPrintf("Return conversion failed, Expected %lX, got %lX!\n", ulUtfScalar, ulUCS4New);
            return DCLSTAT_CURRENTLINE;
        }

        if(nProcessed != nSize)
        {
            DclPrintf("Return conversion process length for %lX is bad, Expected %u, got %u!\n", nSize, nProcessed);
            return DCLSTAT_CURRENTLINE;
        }
    }

    DclPrintf("  Verify detection of invalid UTF-8 scalars...\n");
    {
        const D_UINT32 aulInvalidScalars[] = {
            0xd800,
            0xd801,
            0xdffe,
            0xdfff,
            0x110000,
            0x110001,
            0x7fffffff,
            0x80000000,
            0x80000001,
            0xfffffffe,
            0xffffffff,
        };

        for(nn=0; nn<DCLDIMENSIONOF(aulInvalidScalars); ++nn)
        {
            ulUtfScalar = aulInvalidScalars[nn];
            nSize = DclUtf8FromScalar(ulUtfScalar, acBuff, sizeof(acBuff));

            DclPrintf("    Invalid scalar %lX produced sequence of %d bytes.\n", ulUtfScalar, nSize);

            if(nSize != 0)
            {
                DclPrintf("Sequence encoded from an invalid scalar of %lX to UTF-8!\n", ulUtfScalar);
                return DCLSTAT_CURRENTLINE;
            }
        }
    }

    DclPrintf("  Verify detection of invalid UTF-8 sequences...\n");
    {
        char * aszInvalidUtfString[] = {

            /*  Example ill-formed UTF-8 sequences from:
                See: The Unicode Standard, 3.9 Unicode Encoding Forms, D92

                Test values based on examples by:
                Markus Kuhn <http://www.cl.cam.ac.uk/~mgk25/> - 2003-02-19
            */

            /* Sequences starting with continuation bytes*/
            "\x80",
            "\x81",
            "\xbe",
            "\xbf",

            /*  Strings of continuation characters */
            "\x80\x80",
            "\x80\x80\x80",
            "\x80\x80\x80\x80",
            "\x80\x80\x80\x80\x80",
            "\x80\x80\x80\x80\x80\x80",
            "\x80\x80\x80\x80\x80\x80\x80",

            /* Start bytes with no following continuation bytes */
            "\xc1\x21\x21",
            "\xe1\x21\x21\x21",
            "\xf1\x21\x21\x21\x21",

            /* Starts with a missing continuation */
            "\xc1\x21",
            "\xe1\x80\x21",
            "\xf1\x80\x80\x21",

            /* Over long encodings for a single byte values must fail */
            "\xc0\xa1",
            "\xe0\x80\xa1",
            "\xf0\x80\x80\xa1",
            "\xf8\x80\x80\x80\xa1",
            "\xfc\x80\x80\x80\x80\xa1",

            /* Maximal over long sequence */
            "\xc1\xbf",
            "\xe0\x9f\xbf",
            "\xf0\x8f\xbf\xbf",

            /* There is no UTF-8 encoding for NULL */
            "\xc0\x80",
            "\xe0\x80\x80",
            "\xf0\x80\x80\x80",

            /*  Invalid byte sequences */
            "\xfe\xff",
            "\xff\xfe",
            "\xc2\xf5",
            "\xc2\xc0",
        };

        /*  Check all above sequences...
            Each should produce a zero length character and report an
            invalid scalar.
        */
        for(nn=0; nn< DCLDIMENSIONOF(aszInvalidUtfString); ++nn)
        {
            ulUtfScalar = DclUtf8ToScalar(aszInvalidUtfString[nn],
                    DclStrLen(aszInvalidUtfString[nn]), &nProcessed);

            DclPrintf("    Invalid sequence of ");
            for(ii=0; aszInvalidUtfString[nn][ii]; ii++)
                DclPrintf(" %02x", (D_UINT8)aszInvalidUtfString[nn][ii]);
            DclPrintf(" was transformed to a scalar of 0x%lX.\n", ulUtfScalar);

            if(ulUtfScalar != D_UINT32_MAX && nProcessed != 0)
            {
                DclPrintf("Scalar (%lU) decoded from an invalid sequence of %lD bytes: ", ulUtfScalar, nProcessed);

                for(ii=0; ii<UTF8_MAX_SEQUENCE; ii++)
                    DclPrintf(" %02x", (D_UINT8)aszInvalidUtfString[nn][ii]);

                DclPrintf(" to %lX!\n", ulUtfScalar);
                return DCLSTAT_CURRENTLINE;
            }
        }
    }

    DclPrintf("  Verify detection of missing UTF-8 code bytes...\n");
    {
        char * szString1String;

        /* Missing sequence bytes from 1F000 MAHJONG TILE EAST WIND */
        szString1String = "\xf0\x9f\x80\x80";
        for(nn=1; nn<4; ++nn)
        {
            ulUtfScalar = DclUtf8ToScalar(szString1String, nn, &nProcessed);

            DclPrintf("    Decode a scalar from short sequence of: ");
            for(ii=0; ii<nn; ii++)
                DclPrintf(" %02x", (D_UINT8)szString1String[ii]);
            DclPrintf(" to 0x%lX.\n", ulUtfScalar);

            if(ulUtfScalar != D_UINT32_MAX && nProcessed != 0)
            {
                DclPrintf("DclUtf8CodeLength returned a valid length for an short UTF-8 sequence...\n");
                return DCLSTAT_CURRENTLINE;
            }
        }
    }

    /********************************************
        Test Cases for DclUtfScalarToLower()
     ********************************************/

    DclPrintf("  Verify Unicode tolower ...\n");
    {
        /* Loop through the entire list of cases */
        for(nn=0; gcasUnicodeCaseTestData[nn].ulUpperCase; ++nn)
        {
            ulUtfScalar = DclUtfScalarToLower(
                gcasUnicodeCaseTestData[nn].ulUpperCase);

            if(nn % mm == 0)
            {
                DclPrintf("    Scalar 0x%06lX was folded to lowercase 0x%06lX.\n",
                    gcasUnicodeCaseTestData[nn].ulUpperCase, ulUtfScalar);
            }

            if(ulUtfScalar != gcasUnicodeCaseTestData[nn].ulLowerCase)
            {
                DclPrintf("Failed, expected scalar is 0x%06lX!\n",
                    gcasUnicodeCaseTestData[nn].ulLowerCase);
                return DCLSTAT_CURRENTLINE;
            }

            ulUtfScalar = DclUtfScalarToLower(
                gcasUnicodeCaseTestData[nn].ulLowerCase);

            if(nn % mm == 0)
            {
                DclPrintf("    Lowercase scalar 0x%06lX was folded to 0x%06lX.\n",
                    gcasUnicodeCaseTestData[nn].ulLowerCase, ulUtfScalar);
            }

            if(ulUtfScalar != gcasUnicodeCaseTestData[nn].ulLowerCase)
            {
                DclPrintf("Failed to lowercase scalar, expected 0x%06lX!\n",
                    gcasUnicodeCaseTestData[nn].ulLowerCase);
                return DCLSTAT_CURRENTLINE;
            }
        }
    }

    DclPrintf("  Validate Unicode full case folding tolower (negative test)...\n");
    {
        /*  Loop through the entire list of full folding cases
            Each case should map to itself.
        */
        for(nn=0; gcaulUnicodeFullCaseChars[nn]; ++nn)
        {
            ulUtfScalar = DclUtfScalarToLower(gcaulUnicodeFullCaseChars[nn]);

            if(nn % mm == 0)
            {
                DclPrintf("    Uppercase scalar 0x%06lX was folded to 0x%06lX.\n",
                    gcaulUnicodeFullCaseChars[nn], ulUtfScalar);
            }

            if(ulUtfScalar != gcaulUnicodeFullCaseChars[nn])
            {
                DclPrintf("Failed to lowercase scalar, expected 0x%06lX!\n",
                    gcaulUnicodeFullCaseChars[nn]);
                return DCLSTAT_CURRENTLINE;
            }
        }
    }

    /********************************************
        Test Cases for DclUtfStrToLower()
     ********************************************/

    DclPrintf("  Unicode string to tolower checks...\n");
    {
        struct _tagUtfStringTestData ct;
        char szWorkingString[37];

        /* Setup the test's workspace */
        ct.szWorking = szWorkingString;

        /* No string changes */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz";
        ct.nString1Len     = 26;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^");
        ct.nWorkingLen  = 26;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz";
        ct.nExpectedLen = 26;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* All characters should be changed */
        ct.szString1       = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        ct.nString1Len     = 26;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^");
        ct.nWorkingLen  = 26;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz";
        ct.nExpectedLen = 26;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Mixed case string */
        ct.szString1       = "AbCdEfGhIjKlMNoPqRsTuVwXyZ";
        ct.nString1Len     = 26;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^");
        ct.nWorkingLen  = 26;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz";
        ct.nExpectedLen = 26;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Buffer overrun check */
        ct.szString1       = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        ct.nString1Len     = 13;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^");
        ct.nWorkingLen  = 13;
        ct.szExpected   = "abcdefghijklm^^^^^^^^^^^^^";
        ct.nExpectedLen = 13;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;
        if(DclMemCmp(ct.szExpected, ct.szWorking, 26) != 0)
        {
            DclPrintf("Buffer overrun detected!\n");
            return DCLSTAT_CURRENTLINE;
        }

        /* Buffer underrun check */
        ct.szString1     = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        ct.nString1Len   = 13;
        ct.szString1    += ct.nString1Len;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^");
        ct.nWorkingLen  = 13;
        ct.szWorking   += ct.nWorkingLen;
        ct.szExpected   = "^^^^^^^^^^^^^nopqrstuvwxyz";
        ct.nExpectedLen = 13;
        ct.szExpected  += ct.nExpectedLen;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        ct.szString1   -= ct.nString1Len;
        ct.szWorking   -= ct.nWorkingLen;
        ct.szExpected  -= ct.nExpectedLen;
        if(DclMemCmp(ct.szExpected, ct.szWorking, 26) != 0)
        {
            DclPrintf("Buffer overrun detected!\n");
            return DCLSTAT_CURRENTLINE;
        }

        /* Two byte UTF-8 character () embedded */
        ct.szString1       = "abcdefghijklm"TEST_UTF8_2B_UPPER_CHAR"nopqrstuvwxyz";
        ct.nString1Len     = 28;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^22");
        ct.nWorkingLen  = 28;
        ct.szExpected   = "abcdefghijklm"TEST_UTF8_2B_LOWER_CHAR"nopqrstuvwxyz";
        ct.nExpectedLen = 28;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Two byte UTF-8 character () start */
        ct.szString1       = TEST_UTF8_2B_UPPER_CHAR"abcdefghijklmnopqrstuvwxyz";
        ct.nString1Len     = 28;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^22");
        ct.nWorkingLen  = 28;
        ct.szExpected   = TEST_UTF8_2B_LOWER_CHAR"abcdefghijklmnopqrstuvwxyz";
        ct.nExpectedLen = 28;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Two byte UTF-8 character () end */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_UPPER_CHAR;
        ct.nString1Len     = 28;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^22");
        ct.nWorkingLen  = 28;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_LOWER_CHAR;
        ct.nExpectedLen = 28;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Two byte UTF-8 character () SHORT */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_UPPER_CHAR;
        ct.nString1Len     = 27;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^22");
        ct.nWorkingLen  = 28;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_LOWER_CHAR;
        ct.nExpectedLen = 26;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;


        /* Three byte UTF-8 character () */
        ct.szString1       = "abcdefghijklm"TEST_UTF8_3B_UPPER_CHAR"nopqrstuvwxyz";
        ct.nString1Len     = 29;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^333");
        ct.nWorkingLen  = 29;
        ct.szExpected   = "abcdefghijklm"TEST_UTF8_3B_LOWER_CHAR"nopqrstuvwxyz";
        ct.nExpectedLen = 29;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;


        /* Three byte UTF-8 character () start */
        ct.szString1       = TEST_UTF8_3B_UPPER_CHAR"abcdefghijklmnopqrstuvwxyz";
        ct.nString1Len     = 29;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^333");
        ct.nWorkingLen  = 29;
        ct.szExpected   = TEST_UTF8_3B_LOWER_CHAR"abcdefghijklmnopqrstuvwxyz";
        ct.nExpectedLen = 29;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Three byte UTF-8 character () end */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_UPPER_CHAR;
        ct.nString1Len     = 29;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^333");
        ct.nWorkingLen  = 29;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_LOWER_CHAR;
        ct.nExpectedLen = 29;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Three byte UTF-8 character () SHORT by 1 bytes */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_UPPER_CHAR;
        ct.nString1Len     = 28;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^333");
        ct.nWorkingLen  = 28;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_LOWER_CHAR;;
        ct.nExpectedLen = 26;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Three byte UTF-8 character () SHORT by 2 bytes */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_UPPER_CHAR;
        ct.nString1Len     = 27;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^333");
        ct.nWorkingLen  = 27;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_LOWER_CHAR;
        ct.nExpectedLen = 26;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Subsequent UTF-8 characters (NO CHANGES) */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_LOWER_CHAR TEST_UTF8_3B_LOWER_CHAR;
        ct.nString1Len     = 31;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^22333");
        ct.nWorkingLen  = 31;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_LOWER_CHAR TEST_UTF8_3B_LOWER_CHAR;
        ct.nExpectedLen = 31;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Subsequent UTF-8 characters (One lower and one upper) */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_UPPER_CHAR TEST_UTF8_3B_LOWER_CHAR;
        ct.nString1Len     = 31;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^22333");
        ct.nWorkingLen  = 31;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_LOWER_CHAR TEST_UTF8_3B_LOWER_CHAR;
        ct.nExpectedLen = 31;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Subsequent UTF-8 characters (Both upper) */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_UPPER_CHAR TEST_UTF8_3B_UPPER_CHAR;
        ct.nString1Len     = 31;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^22333");
        ct.nWorkingLen  = 31;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_LOWER_CHAR TEST_UTF8_3B_LOWER_CHAR;
        ct.nExpectedLen = 31;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Subsequent UTF-8 characters (Both upper, one ASCII between) */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_UPPER_CHAR "A" TEST_UTF8_3B_UPPER_CHAR;
        ct.nString1Len     = 32;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^22Z333");
        ct.nWorkingLen  = 32;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_LOWER_CHAR "a" TEST_UTF8_3B_LOWER_CHAR;
        ct.nExpectedLen = 32;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Subsequent UTF-8 characters (Both upper, one ASCII between) */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_UPPER_CHAR TEST_UTF8_3B_UPPER_CHAR "A";
        ct.nString1Len     = 32;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^22333Z");
        ct.nWorkingLen  = 32;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_LOWER_CHAR TEST_UTF8_3B_LOWER_CHAR "a";
        ct.nExpectedLen = 32;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Subsequent UTF-8 characters */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_UPPER_CHAR TEST_UTF8_3B_UPPER_CHAR;
        ct.nString1Len     = 31;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^22333");
        ct.nWorkingLen  = 31;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_LOWER_CHAR TEST_UTF8_3B_LOWER_CHAR;
        ct.nExpectedLen = 31;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Subsequent UTF-8 characters short by 1 byte */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_UPPER_CHAR TEST_UTF8_3B_UPPER_CHAR;
        ct.nString1Len     = 30;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^22333");
        ct.nWorkingLen  = 31;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_LOWER_CHAR TEST_UTF8_3B_LOWER_CHAR;
        ct.nExpectedLen = 28;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Subsequent UTF-8 characters short by 2 byte */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_UPPER_CHAR TEST_UTF8_3B_UPPER_CHAR;
        ct.nString1Len     = 29;
        DclStrCpy(ct.szWorking, "abcdefghijklmnopqrstuvwxyz22333");
        ct.nWorkingLen  = 31;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_LOWER_CHAR TEST_UTF8_3B_LOWER_CHAR;
        ct.nExpectedLen = 28;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /* Subsequent UTF-8 characters short by 3 byte */
        ct.szString1       = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_UPPER_CHAR TEST_UTF8_3B_UPPER_CHAR;
        ct.nString1Len     = 28;
        DclStrCpy(ct.szWorking, "^^^^^^^^^^^^^^^^^^^^^^^^^^22333");
        ct.nWorkingLen  = 31;
        ct.szExpected   = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_LOWER_CHAR TEST_UTF8_3B_LOWER_CHAR;
        ct.nExpectedLen = 28;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;

        /*  The _SPEC_ UTF character has an upper case that is larger
            than the lower case - exercise this case.
        */
        ct.szString1       = "abcdef"TEST_UTF8_UPPER_SPEC_CHAR"ghijklmnopqrstuvwxyz";
        ct.nString1Len     = 28;
        DclStrCpy(ct.szWorking, "123456789012345678901234567");
        ct.nWorkingLen  = 27;
        ct.szExpected   = "abcdef"TEST_UTF8_LOWER_SPEC_CHAR"ghijklmnopqrstuvwxyz";
        ct.nExpectedLen = 27;
        if(!CheckUtfStringToLower(&ct))
            return DCLSTAT_CURRENTLINE;
    }


    /********************************************
        Test Cases for DclUtfStrNICmp() && DclUtf8StrICmp()
     ********************************************/

    DclPrintf("  Unicode string case insensitive N-bound compare checks...\n");
    {
        struct _tagUtfStringTestData ct;

        /* Basic compare check success */
        ct.szString1     = "abcdefghijklmnopqrstuvwxyz";
        ct.nString1Len  = 26;
        ct.szString2     = "abcdefghijklmnopqrstuvwxyz";
        ct.nString2Len  = 26;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 26;
        ct.nExpectedString2Processed = 26;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        ct.szString1     = "abcdef1";
        ct.nString1Len  = 7;
        ct.szString2     = "abcdef2";
        ct.nString2Len  = 7;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 6;
        ct.nExpectedString2Processed = 6;
        ct.iExpectedCompare = -1;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        ct.szString1     = "abcdef2";
        ct.nString1Len  = 7;
        ct.szString2     = "abcdef1";
        ct.nString2Len  = 7;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 6;
        ct.nExpectedString2Processed = 6;
        ct.iExpectedCompare = 1;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        ct.szString1     = "abcdefghijklmnopqrstuvwxyz";
        ct.nString1Len  = 26;
        ct.szString2     = "";
        ct.nString2Len  = 1;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 0;
        ct.nExpectedString2Processed = 0;
        ct.iExpectedCompare = 1;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        ct.szString1     = "";
        ct.nString1Len  = 1;
        ct.szString2     = "abcdefghijklmnopqrstuvwxyz";
        ct.nString2Len  = 26;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 0;
        ct.nExpectedString2Processed = 0;
        ct.iExpectedCompare = -1;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Basic compare check success */
        ct.szString1     = "abcdefghijklmnopqrstuvwxyz";
        ct.nString1Len  = 26;
        ct.szString2     = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        ct.nString2Len  = 26;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 26;
        ct.nExpectedString2Processed = 26;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Basic compare check success */
        ct.szString1     = "ABCDEFGHIJKLM";
        ct.nString1Len  = 13;
        ct.szString2     = "abcdefghijklmnopqrstuvwxyz";
        ct.nString2Len  = 26;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 13;
        ct.nExpectedString2Processed = 13;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        ct.iExpectedCompare = -1;
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Basic compare check success */
        ct.szString1     = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        ct.nString1Len  = 26;
        ct.szString2     = "abcdefghijklm";
        ct.nString2Len  = 13;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 13;
        ct.nExpectedString2Processed = 13;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        ct.iExpectedCompare = 1;
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Basic compare check success */
        ct.szString1     = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        ct.nString1Len  = 26;
        ct.szString2     = "abcdefghijklmnopqrstuvwxyz";
        ct.nString2Len  = 26;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 26;
        ct.nExpectedString2Processed = 26;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Single two byte unicode character, embedded */
        ct.szString1     = "abcdefghijklm"TEST_UTF8_2B_UPPER_CHAR"nopqrstuvwxyz";
        ct.nString1Len  = 28;
        ct.szString2     = "abcdefghijklm"TEST_UTF8_2B_LOWER_CHAR"nopqrstuvwxyz";
        ct.nString2Len  = 28;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 28;
        ct.nExpectedString2Processed = 28;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Single two byte unicode character, end */
        ct.szString1     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_UPPER_CHAR;
        ct.nString1Len  = 28;
        ct.szString2     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_LOWER_CHAR;
        ct.nString2Len  = 28;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 28;
        ct.nExpectedString2Processed = 28;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Single two byte unicode character, start */
        ct.szString1     = TEST_UTF8_2B_UPPER_CHAR"abcdefghijklmnopqrstuvwxyz";
        ct.nString1Len  = 28;
        ct.szString2     = TEST_UTF8_2B_LOWER_CHAR"abcdefghijklmnopqrstuvwxyz";
        ct.nString2Len  = 28;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 28;
        ct.nExpectedString2Processed = 28;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Single three byte unicode character, embedded */
        ct.szString1     = "abcdefghijklm"TEST_UTF8_3B_UPPER_CHAR"nopqrstuvwxyz";
        ct.nString1Len  = 29;
        ct.szString2     = "abcdefghijklm"TEST_UTF8_3B_LOWER_CHAR"nopqrstuvwxyz";
        ct.nString2Len  = 29;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 29;
        ct.nExpectedString2Processed = 29;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Single three byte unicode character, end */
        ct.szString1     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_UPPER_CHAR;
        ct.nString1Len  = 29;
        ct.szString2     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_LOWER_CHAR;
        ct.nString2Len  = 29;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 29;
        ct.nExpectedString2Processed = 29;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Single three byte unicode character, start */
        ct.szString1     = TEST_UTF8_3B_LOWER_CHAR"abcdefghijklmnopqrstuvwxyz";
        ct.nString1Len  = 29;
        ct.szString2     = TEST_UTF8_3B_UPPER_CHAR"abcdefghijklmnopqrstuvwxyz";
        ct.nString2Len  = 29;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 29;
        ct.nExpectedString2Processed = 29;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Three and Two byte unicode character, ASCII  */
        ct.szString1     = "abcdefghijklM"TEST_UTF8_3B_UPPER_CHAR TEST_UTF8_2B_UPPER_CHAR"Nopqrstuvwxyz";
        ct.nString1Len  = 31;
        ct.szString2     = "abcdefghijklm"TEST_UTF8_3B_LOWER_CHAR TEST_UTF8_2B_UPPER_CHAR"nopqrstuvwxyz";
        ct.nString2Len  = 31;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 31;
        ct.nExpectedString2Processed = 31;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Two and three byte unicode character, ASCII  */
        ct.szString1     = "abcdefghijklM" TEST_UTF8_2B_UPPER_CHAR TEST_UTF8_3B_UPPER_CHAR"Nopqrstuvwxyz";
        ct.nString1Len  = 31;
        ct.szString2     = "abcdefghijklm"TEST_UTF8_2B_LOWER_CHAR TEST_UTF8_3B_UPPER_CHAR"nopqrstuvwxyz";
        ct.nString2Len  = 31;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 31;
        ct.nExpectedString2Processed = 31;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* COMPARE (SEMI) FAILURE CASES */

        /* Appended three byte unicode character SHORT compare, success case */
        ct.szString1     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_UPPER_CHAR;
        ct.nString1Len  = 29;
        ct.szString2     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_UPPER_CHAR;
        ct.nString2Len  = 29;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 29;
        ct.nExpectedString2Processed = 29;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Appended three byte unicode character SHORT compare, fail case */
        ct.szString1     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_LOWER_CHAR;
        ct.nString1Len  = 28;
        ct.szString2     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_UPPER_CHAR;
        ct.nString2Len  = 28;
        ct.uExpectedCompareFlags = DCL_UTF8_PARTIAL_BUFFER_1 | DCL_UTF8_PARTIAL_BUFFER_2;
        ct.nExpectedString1Processed = 26;
        ct.nExpectedString2Processed = 26;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Appended three byte unicode character SHORT compare, fail case */
        ct.szString1     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_LOWER_CHAR;
        ct.nString1Len  = 27;
        ct.szString2     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_UPPER_CHAR;
        ct.nString2Len  = 27;
        ct.uExpectedCompareFlags = DCL_UTF8_PARTIAL_BUFFER_1 | DCL_UTF8_PARTIAL_BUFFER_2;
        ct.nExpectedString1Processed = 26;
        ct.nExpectedString2Processed = 26;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Appended three byte unicode character one string SHORT compare, fail case */
        ct.szString1     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_LOWER_CHAR;
        ct.nString1Len  = 27;
        ct.szString2     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_UPPER_CHAR;
        ct.nString2Len  = 29;
        ct.uExpectedCompareFlags = DCL_UTF8_PARTIAL_BUFFER_1;
        ct.nExpectedString1Processed = 26;
        ct.nExpectedString2Processed = 26;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Appended three byte unicode character one string SHORT compare, fail case */
        ct.szString1     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_LOWER_CHAR;
        ct.nString1Len  = 29;
        ct.szString2     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_UPPER_CHAR;
        ct.nString2Len  = 27;
        ct.uExpectedCompareFlags = DCL_UTF8_PARTIAL_BUFFER_2;
        ct.nExpectedString1Processed = 26;
        ct.nExpectedString2Processed = 26;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Appended three byte unicode character SHORT compare, success case */
        ct.szString1     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_UPPER_CHAR;
        ct.nString1Len  = 26;
        ct.szString2     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_UPPER_CHAR;
        ct.nString2Len  = 26;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 26;
        ct.nExpectedString2Processed = 26;
        ct.iExpectedCompare = 0;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Single mixed string character compare, failure case */
        ct.szString1     = "abcdefghijklm"TEST_UTF8_2B_UPPER_CHAR"nopqrstuvwxyz";
        ct.nString1Len  = 28;
        ct.szString2     = "abcdefghijklmn"TEST_UTF8_2B_UPPER_CHAR"opqrstuvwxyz";
        ct.nString2Len  = 28;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 13;
        ct.nExpectedString2Processed = 13;
        ct.iExpectedCompare = 1;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Single mixed string character compare, failure case */
        ct.szString1     = "abcdefghijklmn"TEST_UTF8_2B_UPPER_CHAR"opqrstuvwxyz";
        ct.nString1Len  = 28;
        ct.szString2     = "abcdefghijklm"TEST_UTF8_2B_UPPER_CHAR"opqrstuvwxyz";
        ct.nString2Len  = 28;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 13;
        ct.nExpectedString2Processed = 13;
        ct.iExpectedCompare = -1;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Single mixed string character compare, failure case */
        ct.szString1     = "abcdefghijklm"TEST_UTF8_3B_UPPER_CHAR"nopqrstuvwxyz";
        ct.nString1Len  = 29;
        ct.szString2     = "abcdefghijklmn"TEST_UTF8_3B_UPPER_CHAR"opqrstuvwxyz";
        ct.nString2Len  = 29;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 13;
        ct.nExpectedString2Processed = 13;
        ct.iExpectedCompare = 1;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Single mixed string character compare, failure case */
        ct.szString1     = "abcdefghijklmn"TEST_UTF8_3B_UPPER_CHAR"opqrstuvwxyz";
        ct.nString1Len  = 29;
        ct.szString2     = "abcdefghijklm"TEST_UTF8_3B_UPPER_CHAR"nopqrstuvwxyz";
        ct.nString2Len  = 29;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 13;
        ct.nExpectedString2Processed = 13;
        ct.iExpectedCompare = -1;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Added three byte and two byte unicode character compare, failure case */
        ct.szString1     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_UPPER_CHAR;
        ct.nString1Len  = 29;
        ct.szString2     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_UPPER_CHAR "*";
        ct.nString2Len  = 29;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 26;
        ct.nExpectedString2Processed = 26;
        ct.iExpectedCompare = 1;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

        /* Added three byte and twqo byte unicode character compare, failure case */
        ct.szString1     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_2B_UPPER_CHAR "*";
        ct.nString1Len  = 29;
        ct.szString2     = "abcdefghijklmnopqrstuvwxyz"TEST_UTF8_3B_UPPER_CHAR;
        ct.nString2Len  = 29;
        ct.uExpectedCompareFlags = 0;
        ct.nExpectedString1Processed = 26;
        ct.nExpectedString2Processed = 26;
        ct.iExpectedCompare = -1;
        if(!CheckUtfStringNICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }
        if(!CheckUtfStringICmp(&ct))
        {
            return DCLSTAT_CURRENTLINE;
        }

    }

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}



