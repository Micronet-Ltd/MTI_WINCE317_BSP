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

    This module contains code to fold a Unicode character to its
    lowercase counterpart.

    This solution is based on the Unicode case folding rules, as described
    at http://unicode.org/Public/UNIDATA/CaseFolding.txt.

    This algorithm supports the "common" (C) and "simple" (S) case-folding
    rules.

    If this function is updated to perform full case folding, other
    functions such as DclUtf8StringToLower() may need review to ensure
    functionality is not compromised.

    The output from this function has been validated to exactly match that
    described in the URL above, with regard to the C and S rules.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlutftolower.c $
    Revision 1.5  2010/09/02 20:09:22Z  johnb
    Changed iAdjust from D_INT16 to a D_INT32 and change its
    name to lAdjust
    Revision 1.4  2010/07/29 01:13:23Z  garyp
    Updated to build cleanly with Code Sourcery tools.
    Revision 1.3  2009/05/19 05:04:56Z  garyp
    Updated for AutoDoc.  Renamed a variable for clarity.
    Revision 1.2  2009/05/16 15:47:34Z  keithg
    Clarified comments.
    Revision 1.1  2009/05/15 02:00:10Z  keithg
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

typedef struct
{
    D_UINT32 ulStartCode;   /* First element handled by this range */
    D_UINT8  bCount;        /* Count of elements encapsulated in this range */
    D_UINT8	 bType;         /* Range type WC_* */
    D_INT32	 lAdjust;       /* Adjust value, lower case code, or sub-table index */
} RANGEDESCRIPTOR;

/*  Four basic types of range discriptors
*/
#define WC_FIXED        (0) /* Add/subtract a fixed value to get the lowercase equivalent */
#define WC_ALTERNATING  (1) /* Alternating lower/upper case values, always incrementing */
#define WC_SINGLE       (2) /* A single translation code embedded in the table itself */
#define WC_SUBTABLE     (3) /* A sub-table holds the direct translation values for the range */

static const D_UINT16   auLatinTable1[] =       /*  WC_LATIN_1 */
{
    0x0073, 0x0000, 0x0253, 0x0183, 0x0000, 0x0185, 0x0000, 0x0254,
    0x0188, 0x0000, 0x0256, 0x0257, 0x018C, 0x0000, 0x0000, 0x01DD,
    0x0259, 0x025B, 0x0192, 0x0000, 0x0260, 0x0263, 0x0000, 0x0269,
    0x0268, 0x0199, 0x0000, 0x0000, 0x0000, 0x026F, 0x0272, 0x0000,
    0x0275, 0x01A1, 0x0000, 0x01A3, 0x0000, 0x01A5, 0x0000, 0x0280,
    0x01A8, 0x0000, 0x0283, 0x0000, 0x0000, 0x01AD, 0x0000, 0x0288,
    0x01B0, 0x0000, 0x028A, 0x028B, 0x01B4, 0x0000, 0x01B6, 0x0000,
    0x0292, 0x01B9, 0x0000, 0x0000, 0x0000, 0x01BD, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x01C6, 0x01C6, 0x0000,
    0x01C9, 0x01C9, 0x0000, 0x01CC, 0x01CC
};

static const D_UINT16   auLatinTable2[] =       /*  WC_LATIN_2 */
{
    0x01F3, 0x01F3, 0x0000, 0x01F5, 0x0000, 0x0195, 0x01BF
};

static const D_UINT16   auLatinTable3[] =       /*  WC_LATIN_3 */
{
    0x2C65, 0x023C, 0x0000, 0x019A, 0x2C66, 0x0000, 0x0000, 0x0242,
    0x0000, 0x0180, 0x0289, 0x028C

};

static const D_UINT16   auLatinTable4[] =       /*  WC_LATIN_4 */
{
    0x0371, 0x0000, 0x0373, 0x0000, 0x0000, 0x0000, 0x0377
};

static const D_UINT16   auLatinTable5[] =       /*  WC_LATIN_5 */
{
    0x03AC, 0x0000, 0x03AD, 0x03AE, 0x03AF, 0x0000, 0x03CC, 0x0000,
    0x03CD, 0x03CE
};

static const D_UINT16   auGreekTable1[] =       /*  WC_GREEK_1 */
{
    0x03D7, 0x03B2, 0x03B8, 0x0000, 0x0000, 0x0000, 0x03C6, 0x03C0
};

static const D_UINT16   auGreekTable2[] =       /*  WC_GREEK_2 */
{
    0x03BA, 0x03C1, 0x0000, 0x0000, 0x03B8, 0x03B5, 0x0000, 0x03F8,
    0x0000, 0x03F2, 0x03FB, 0x0000, 0x0000, 0x037B, 0x037C, 0x037D
};

static const D_UINT16   auGreekTable3[] =       /*  WC_GREEK_3 */
{
    0x1F51, 0x0000, 0x1F53, 0x0000, 0x1F55, 0x0000, 0x1F57
};

static const D_UINT16   auGreekTable4[] =       /*  WC_GREEK_4 */
{
    0x1FB0, 0x1FB1, 0x1F70, 0x1F71, 0x1FB3, 0x0000, 0x03B9
};

static const D_UINT16   auGreekTable5[] =       /*  WC_GREEK_5 */
{
    0x1FD0, 0x1FD1, 0x1F76, 0x1F77
};

static const D_UINT16   auGreekTable6[] =       /*  WC_GREEK_6 */
{
    0x1FE0, 0x1FE1, 0x1F7A, 0x1F7B, 0x1FE5
};

static const D_UINT16   auGreekTable7[] =       /*  WC_GREEK_7 */
{
    0x1F78, 0x1F79, 0x1F7C, 0x1F7D, 0x1FF3
};

static const D_UINT16   auGreekTable8[] =       /*  WC_GREEK_8 */
{
    0x03C9, 0x0000, 0x0000, 0x0000, 0x006B, 0x00E5, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x214E,
};

static const D_UINT16   auLatinTable6[] =       /*  WC_LATIN_6 */
{
    0x2C61, 0x0000, 0x026B, 0x1D7D, 0x027D, 0x0000, 0x0000, 0x2C68,
    0x0000, 0x2C6A, 0x0000, 0x2C6C, 0x0000, 0x0251, 0x0271, 0x0250,
    0x0000, 0x0000, 0x2C73, 0x0000, 0x0000, 0x2C76
};

static const D_UINT16   auLatinTable7[] =       /*  WC_LATIN_7 */
{
    0xA77A, 0x0000, 0xA77C, 0x0000, 0x1D79
};

/*  These defines must directly match the appropriate indices
    into pauTables.  We use the extra indirection with the
    array of pointers, since the pointer value itself will
    not fit in the RANGEDESCRIPTOR structure without wasting
    a lot of space.
*/
#define WC_LATIN_1              (0)
#define WC_LATIN_2              (1)
#define WC_LATIN_3              (2)
#define WC_LATIN_4              (3)
#define WC_LATIN_5              (4)
#define WC_GREEK_1              (5)
#define WC_GREEK_2              (6)
#define WC_GREEK_3              (7)
#define WC_GREEK_4              (8)
#define WC_GREEK_5              (9)
#define WC_GREEK_6             (10)
#define WC_GREEK_7             (11)
#define WC_GREEK_8             (12)
#define WC_LATIN_6             (13)
#define WC_LATIN_7             (14)

static const D_UINT16 *pauTables[] =
{
    &auLatinTable1[0],                              /* WC_LATIN_1 */
    &auLatinTable2[0],                              /* WC_LATIN_2 */
    &auLatinTable3[0],                              /* WC_LATIN_3 */
    &auLatinTable4[0],                              /* WC_LATIN_4 */
    &auLatinTable5[0],                              /* WC_LATIN_5 */
    &auGreekTable1[0],                              /* WC_GREEK_1 */
    &auGreekTable2[0],                              /* WC_GREEK_2 */
    &auGreekTable3[0],                              /* WC_GREEK_3 */
    &auGreekTable4[0],                              /* WC_GREEK_4 */
    &auGreekTable5[0],                              /* WC_GREEK_5 */
    &auGreekTable6[0],                              /* WC_GREEK_6 */
    &auGreekTable7[0],                              /* WC_GREEK_7 */
    &auGreekTable8[0],                              /* WC_GREEK_8 */
    &auLatinTable6[0],                              /* WC_LATIN_6 */
    &auLatinTable7[0]                               /* WC_LATIN_7 */
};

static const RANGEDESCRIPTOR aRanges[] =
{
    /* Start  Count   Type              Adjust */
/*  {0x000041,  26, WC_FIXED,             +0x20},      A-Z (handled separately */
    {0x0000B5,   1, WC_SINGLE,           0x03BC},   /* MICRO SIGN */
    {0x0000C0,  23, WC_FIXED,             +0x20},   /* LATIN CAPITAL LETTERS */
    {0x0000D8,   7, WC_FIXED,             +0x20},   /* LATIN CAPITAL LETTERS */
    {0x000100,  47, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x000132,   6, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x000139,  15, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x00014A,  45, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x000178,   1, WC_SINGLE,           0x00FF},   /* LATIN CAPITAL LETTER Y WITH DIAERESIS */
    {0x000179,   5, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x00017F,  77, WC_SUBTABLE,     WC_LATIN_1},   /* LATIN CAPITAL LETTERS */
    {0x0001CD,  15, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x0001DE,  17, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x0001F1,   7, WC_SUBTABLE,     WC_LATIN_2},   /* LATIN CAPITAL LETTERS */
    {0x0001F8,  39, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x000220,   1, WC_SINGLE,           0x019E},   /* LATIN CAPITAL LETTER N WITH LONG RIGHT LEG */
    {0x000222,  17, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x00023A,  12, WC_SUBTABLE,     WC_LATIN_3},   /* LATIN CAPITAL LETTERS */
    {0x000246,   9, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x000345,   1, WC_SINGLE,           0x03B9},   /* COMBINING GREEK YPOGEGRAMMENI */
    {0x000370,   7, WC_SUBTABLE,     WC_LATIN_4},   /* LATIN CAPITAL LETTERS */
    {0x000386,  10, WC_SUBTABLE,     WC_LATIN_5},   /* LATIN CAPITAL LETTERS */
    {0x000391,  17, WC_FIXED,             +0x20},   /* GREEK CAPITAL LETTERS */
    {0x0003A3,   9, WC_FIXED,             +0x20},   /* GREEK CAPITAL LETTERS */
    {0x0003C2,   1, WC_SINGLE,           0x03C3},   /* GREEK SMALL LETTER FINAL SIGMA */
    {0x0003CF,   8, WC_SUBTABLE,     WC_GREEK_1},   /* GREEK SYMBOLS */
    {0x0003D8,  23, WC_ALTERNATING,           0},   /* GREEK/COPTIC CAPITAL LETTERS */
    {0x0003F0,  16, WC_SUBTABLE,     WC_GREEK_2},   /* GREEK SYMBOLS */
    {0x000400,  16, WC_FIXED,             +0x50},   /* CYRILLIC CAPITAL LETTERS */
    {0x000410,  32, WC_FIXED,             +0x20},   /* CYRILLIC CAPITAL LETTERS */
    {0x000460,  33, WC_ALTERNATING,           0},   /* CYRILLIC CAPITAL LETTERS */
    {0x00048A,  53, WC_ALTERNATING,           0},   /* CYRILLIC CAPITAL LETTERS */
    {0x0004C0,   1, WC_SINGLE,           0x04CF},   /* CYRILLIC LETTER PALOCHKA */
    {0x0004C1,  13, WC_ALTERNATING,           0},   /* CYRILLIC CAPITAL LETTERS */
    {0x0004D0,  83, WC_ALTERNATING,           0},   /* CYRILLIC CAPITAL LETTERS */
    {0x000531,  38, WC_FIXED,             +0x30},   /* ARMENIAN CAPITAL LETTERS */
    {0x0010A0,  38, WC_FIXED,           +0x1C60},   /* GEORGIAN CAPITAL LETTERS */
    {0x001E00, 149, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x001E9B,   1, WC_SINGLE,           0x1E61},   /* LATIN SMALL LETTER LONG S WITH DOT ABOVE */
    {0x001E9E,   1, WC_SINGLE,           0x00DF},   /* LATIN CAPITAL LETTER SHARP S */
    {0x001EA0,  95, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x001F08,   8, WC_FIXED,             -0x08},   /* GREEK CAPITAL LETTERS */
    {0x001F18,   6, WC_FIXED,             -0x08},   /* GREEK CAPITAL LETTERS */
    {0x001F28,   8, WC_FIXED,             -0x08},   /* GREEK CAPITAL LETTERS */
    {0x001F38,   8, WC_FIXED,             -0x08},   /* GREEK CAPITAL LETTERS */
    {0x001F48,   6, WC_FIXED,             -0x08},   /* GREEK CAPITAL LETTERS */
    {0x001F48,   6, WC_FIXED,             -0x08},   /* GREEK CAPITAL LETTERS */
    {0x001F59,   7, WC_SUBTABLE,     WC_GREEK_3},   /* GREEK CAPITAL LETTERS */
    {0x001F68,   8, WC_FIXED,             -0x08},   /* GREEK CAPITAL LETTERS */
    {0x001F88,   8, WC_FIXED,             -0x08},   /* GREEK CAPITAL LETTERS */
    {0x001F98,   8, WC_FIXED,             -0x08},   /* GREEK CAPITAL LETTERS */
    {0x001FA8,   8, WC_FIXED,             -0x08},   /* GREEK CAPITAL LETTERS */
    {0x001FB8,   7, WC_SUBTABLE,     WC_GREEK_4},   /* GREEK CAPITAL LETTERS */
    {0x001FC8,   4, WC_FIXED,             -0x56},   /* GREEK CAPITAL LETTERS */
    {0x001FCC,   1, WC_SINGLE,           0x1FC3},   /* GREEK CAPITAL LETTER ETA WITH PROSGEGRAMMENI */
    {0x001FD8,   4, WC_SUBTABLE,     WC_GREEK_5},   /* GREEK CAPITAL LETTERS */
    {0x001FE8,   5, WC_SUBTABLE,     WC_GREEK_6},   /* GREEK CAPITAL LETTERS */
    {0x001FF8,   5, WC_SUBTABLE,     WC_GREEK_7},   /* GREEK CAPITAL LETTERS */
    {0x002126,  13, WC_SUBTABLE,     WC_GREEK_8},   /* SIGNS */
    {0x002160,  16, WC_FIXED,             +0x10},   /* ROMAN NUMERALS */
    {0x002183,   1, WC_SINGLE,           0x2184},   /* ROMAN NUMERAL REVERSED ONE HUNDRED */
    {0x0024B6,  26, WC_FIXED,             +0x1A},   /* CIRCLED LATIN CAPITAL LETTERS */
    {0x002C00,  47, WC_FIXED,             +0x30},   /* GLAGOLITIC CAPITAL LETTERS */
    {0x002C60,  22, WC_SUBTABLE,     WC_LATIN_6},   /* LATIN CAPITAL LETTERS */
    {0x002C80,  99, WC_ALTERNATING,           0},   /* COPTIC CAPITAL LETTERS */
    {0x00A640,  31, WC_ALTERNATING,           0},   /* CYRILLIC CAPITAL LETTERS */
    {0x00A662,  11, WC_ALTERNATING,           0},   /* CYRILLIC CAPITAL LETTERS */
    {0x00A680,  23, WC_ALTERNATING,           0},   /* CYRILLIC CAPITAL LETTERS */
    {0x00A722,  13, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x00A732,  29, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x00A750,  31, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x00A779,   5, WC_SUBTABLE,     WC_LATIN_7},   /* LATIN CAPITAL LETTERS */
    {0x00A77E,   9, WC_ALTERNATING,           0},   /* LATIN CAPITAL LETTERS */
    {0x00A78B,   1, WC_SINGLE,           0xA78C},   /* LATIN CAPITAL LETTER SALTILLO */
    {0x00FF21,  26, WC_FIXED,             +0x20},   /* FULLWIDTH LATIN CAPITAL LETTERS */
    {0x010400,  40, WC_FIXED,             +0x28}    /* DESERET CAPITAL LETTERS */
};


/*-------------------------------------------------------------------
    Protected: DclUtfScalarToLower()

    Convert the supplied U+ code point to its lowercase equivalent.
    The same character will be returned if there is no translation.

    Note that this function does not perform any interpretation
    of the given scalar and will accept any scalar in the integral
    range.  This is wider than the Microsoft style UCS-2 or UTF-8
    character sets.

    Note that this function is locale-independent.  All characters
    are processed regardless of locale.  For locale relevant
    conversions, use OS specific functions.

    Parameters:
        ulScalar         - The Unicode scalar value to convert

    Return Value:
        Returns the converted character value, or the original
        value if no lowercase conversion is available.
-------------------------------------------------------------------*/
D_UINT32    DclUtfScalarToLower(
    D_UINT32    ulScalar)
{
    if(ulScalar >= 'A' && ulScalar <= 'Z')
    {
        /*  Performance optimization for ASCII A to Z
        */
        return ulScalar + 0x20;
    }
    else
    {
        int iStart = 0;
        int iCount = DCLDIMENSIONOF(aRanges);
        int iPos = iCount >> 1;

        /*  Initial binary search parameters initialized above.  Using
            "int" types to allow for simpler overflow checking.
        */

        while(TRUE)
        {
            DclAssert(iStart >= 0);
            DclAssert(iStart < (int)DCLDIMENSIONOF(aRanges));
            DclAssert(iCount > 0);
            DclAssert(iCount <= (int)DCLDIMENSIONOF(aRanges) - iStart);
            DclAssert(iPos >= iStart);
            DclAssert(iPos < iStart + iCount);

            if(ulScalar < aRanges[iPos].ulStartCode)
            {
                /*  If is not a part of this range and this is the last
                    range to search, then there is no mapping for this
                    code and we return the original value.
                */
                if((iCount == 1) || (iPos == iStart))
                    return ulScalar;

                DclAssert(iPos > iStart);

                /*  Narrow the search...
                */
                iCount = iPos - iStart;
                iPos = iStart + (iCount >> 1);
                continue;
            }

            if(ulScalar >= aRanges[iPos].ulStartCode + aRanges[iPos].bCount)
            {
                /*  If is not a part of this range and this is the last
                    range to search, then there is no mapping for this
                    code and we return the original value.
                */
                if((iCount == 1) || (iPos == iStart + iCount - 1))
                    return ulScalar;

                /*  Narrow the search...
                */
                iCount -= iPos - iStart + 1;
                iStart = iPos + 1;
                iPos = iStart + (iCount >> 1);
                continue;
            }

            /*  Found the table entry which encapsulates this code.
                Process it according to the type of entry.
            */
            switch(aRanges[iPos].bType)
            {
                case WC_FIXED:
                    return ulScalar + aRanges[iPos].lAdjust;

                case WC_SINGLE:
                    /*  For this style of range, we're storing an unsigned
                        value in a signed field, so we must use an extra
                        cast to ensure that the correct value is returned.
                    */
                    return (D_UINT32)(D_UINT16)aRanges[iPos].lAdjust;

                case WC_ALTERNATING:
                    /*  If the low bit of the source and destination codes
                        match, then the translated code is 1 more than the
                        original code.  If the low bits do not match, then
                        the code is unchanged (already is lowercase).
                    */
                    if((ulScalar & 1) == (aRanges[iPos].ulStartCode & 1))
                        return ulScalar + 1;
                    else
                        return ulScalar;

                case WC_SUBTABLE:
                {
                    const D_UINT16 *pauArray = pauTables[aRanges[iPos].lAdjust];
                    D_UINT16        uLower;

                    uLower = pauArray[ulScalar - aRanges[iPos].ulStartCode];

                    /*  If the array element is non-zero, then it is a valid
                        translation, so return it.  If it is zero, then it
                        is just a placeholder for a non-translatable character,
                        so return the original value.
                    */
                    if(uLower)
                        return (D_UINT32)uLower;
                    else
                        return ulScalar;
                }

                default:
                    DclProductionError();
            }
        }
    }
}


