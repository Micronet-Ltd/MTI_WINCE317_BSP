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

    This module contains tests for multibyte and wide character string
    conversions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltmbwc.c $
    Revision 1.6  2009/06/28 22:48:21Z  garyp
    Updated to include dltests.h.
    Revision 1.5  2009/05/08 17:09:53Z  jimmb
    changed unsigned to size_t to match the prototype.
    Revision 1.4  2009/02/21 04:11:59Z  brandont
    Added static declaration for local structures.
    Revision 1.3  2009/01/26 18:08:22Z  johnb
    Corrected string error in failure test
    Changed DCLSTAT_OUTOFRANGE to DCLSTAT_BADMULTIBYTECHAR
    Added test case for embedded null (0x00) in multibyte and wide
    char test cases.
    Revision 1.2  2009/01/21 19:03:51Z  johnb
    Updated DclWideCharToMultiByte and DclMultByteToWideChar
    to use char rather than D_UINT8.  Added new tests for invalid
    multibyte code sequences.
    Revision 1.1  2009/01/19 19:36:02Z  johnb
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapireq.h>
#include <dlstats.h>
#include <dltools.h>
#include <dlperflog.h>
#include "dltests.h"

#define MAX_UNICODE 0xFFFF

#define MAX_WIDECHAR_LEN 256
#define MAX_MULTIBYTE_LEN 1024

#define MAX_DESC_SIZE 80


static DCLSTATUS TestWCChars(void);
static DCLSTATUS TestWC2MBConv(void);
static DCLSTATUS TestMB2WCConv(void);

static DCLSTATUS TestWCConvErr(void);
static DCLSTATUS TestMBConvErr(void);
static DCLSTATUS TestOtherError(void);



typedef struct wide_char_test
{
    char        szDescription[MAX_DESC_SIZE];
    D_UINT16    wcSrcString[MAX_WIDECHAR_LEN];
    int         nSrcStringLen;
    char        mbDstString[MAX_MULTIBYTE_LEN];
    unsigned    uDstStringLen;
    DCLSTATUS   uExpectedResult;
}WCT;

typedef struct multbyte_test
{
    char        szDescription[MAX_DESC_SIZE];
    char        cSrcString[MAX_MULTIBYTE_LEN];
    int         nSrcStringLen;
    D_UINT16    wcDstString[MAX_WIDECHAR_LEN];
    unsigned    uDstStringLen;
    DCLSTATUS   uExpectedResult;
}MBT;

static MBT mb_fail_tests[] =
{
    {
        /*  test if source length of 0 fails -- expect return code of
            DCLSTAT_BADPARAMETER
        */
        {"Failure Test 1 "},
        {"\x41\xe2\x89\xa2\xce\x91\x2e\x00"},
        0,
        {0x0041, 0x2262, 0x0391, 0x002E, 0x0000},
        5,
        DCLSTAT_BADPARAMETER
    },
    {
        /*  Test if destination buffer is too small for data -- expect return
            code of DCLSTAT_LIMITREACHED
        */
        {"Failure Test 2 "},
        {"\x41\xe2\x89\xa2\xce\x91\x2e\x00"},
        8,
        {0x0041, 0x2262, 0x0391, 0x002E, 0x0000},
        2,
        DCLSTAT_LIMITREACHED
    },
    {
        /*  Test incomplete multibyte 3 byte sequence
        */
        {"Failure Test 3 "},
        {"\x41\xe2"},
        2,
        {0x0041, 0x0000},
        2,
        DCLSTAT_LIMITREACHED
    },
    {
        /*  Test incomplete multibyte 2 byte sequence
        */
        {"Failure Test 4 "},
        {"\x41\xc2"},
        2,
        {0x0041, 0x0000},
        2,
        DCLSTAT_LIMITREACHED
    },
    {
        /*  Test invalid multibyte 1 byte sequence
        */
        {"Failure Test 5 "},
        {"\\\xBF\\"},
        5,
        {0x0041, 0x0000, 0x0000, 0x0000},
        4,
        DCLSTAT_BADMULTIBYTECHAR
    },
    {
        /*  Test invalid multibyte 2 byte sequence
        */
        {"Failure Test 5 "},
        {"\\\xC0\xAE.\\"},
        5,
        {0x0041, 0x0000, 0x0000, 0x0000},
        4,
        DCLSTAT_BADMULTIBYTECHAR
    },
    {
        /*  Test invalid multibyte 3 byte sequence
        */
        {"Failure Test 6 "},
        {"\\.\xE0\x80\xAE\\"},
        6,
        {0x0041, 0x0000, 0x0000, 0x0000},
        4,
        DCLSTAT_BADMULTIBYTECHAR
    },
    {
        {"END"},
        {0x00}, 0,
        {0x0000}, 0,
        0
    }

};

static WCT wc_fail_tests[] =
{
    {
        /*  test if source length of 0 fails -- expect return code of
            DCLSTAT_BADPARAMETER
        */
        {"Failure Test 1 "},
        {0x0041, 0x2262, 0x0391, 0x002E, 0x0000},
        0,
        {"\x41\xe2\x89\xa2\xce\x91\x2e\x00"},
        8,
        DCLSTAT_BADPARAMETER
    },
    {
        /*  Test if destination buffer is too small for data -- expect return
            code of DCLSTAT_LIMITREACHED
        */
        {"Failure Test 2 "},
        {0x0041, 0x2262, 0x0391, 0x002E, 0x0000},
        5,
        {"\x41\xe2\x89\xa2\xce\x91\x2e\x00"},
        4,
        DCLSTAT_LIMITREACHED
    },
    {
        {"END"},
        {0x00}, 0,
        {0x0000}, 0,
        0
    }
};



static MBT mb_tests[] =
{
    {   {"A != <ALPHA> "},
        {"\x41\xe2\x89\xa2\xce\x91\x2e\x00"},
        8,
        {0x0041, 0x2262, 0x0391, 0x002E, 0x0000},
        5,
        DCLSTAT_SUCCESS
    },
    {
        {"Hi Mom <smiley> "},
        {"\x48\x69\x20\x4d\x6f\x6d\x20\xe2\x98\xba\x21\x00"},
        12,
        {0x0048, 0x0069, 0x0020, 0x004D, 0x006F, 0x006D, 0x0020, 0x263A, 0x0021,
         0x0000},
        10,
        DCLSTAT_SUCCESS
    },
    {
        {"nihongo "},
        {"\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E\x00"},
        10,
        {0x65E5, 0x672C, 0x8A9e, 0x0000},
        4,
        DCLSTAT_SUCCESS
    },
    {
        {"Embedded NULLS "},
        {"\xE6\x97\xA5\x00\xE6\x9C\xAC\x00\xE8\xAA\x9E\x00"},
        12,
        {0x65E5, 0x0000, 0x672C, 0x0000, 0x8A9e, 0x0000},
        6,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- English "},
        {"\x54\x68\x65\x20\x71\x75\x69\x63\x6b\x20\x62\x72"
         "\x6f\x77\x6e\x20\x66\x6f\x78\x20\x6a\x75\x6d\x70"
         "\x65\x64\x20\x6f\x76\x65\x72\x20\x74\x68\x65\x20"
         "\x6c\x61\x7a\x79\x20\x64\x6f\x67\x2e\x00"},
        -1,
        {0x0054, 0x0068, 0x0065, 0x0020, 0x0071, 0x0075, 0x0069, 0x0063, 0x006b,
         0x0020, 0x0062, 0x0072, 0x006f, 0x0077, 0x006e, 0x0020, 0x0066, 0x006f,
         0x0078, 0x0020, 0x006a, 0x0075, 0x006d, 0x0070, 0x0065, 0x0064, 0x0020,
         0x006f, 0x0076, 0x0065, 0x0072, 0x0020, 0x0074, 0x0068, 0x0065, 0x0020,
         0x006c, 0x0061, 0x007a, 0x0079, 0x0020, 0x0064, 0x006f, 0x0067, 0x002e,
         0x0000},
        46,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Chinese "},
        {"\xe5\xbf\xab\xe7\x9a\x84\xe6\xa3\x95\xe8\x89\xb2"
         "\xe7\x8b\x90\xe7\x8b\xb8\xe8\xb7\xb3\xe8\xbf\x87"
         "\xe4\xba\x86\xe6\x87\x92\xe6\x83\xb0\xe7\x8b\x97"
         "\xe3\x80\x82\x00"},
        -1,
        {0x5feb, 0x7684, 0x68d5, 0x8272, 0x72d0, 0x72f8, 0x8df3, 0x8fc7, 0x4e86,
         0x61d2, 0x60f0, 0x72d7, 0x3002, 0x000},
        14,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Dutch "},
        {"\x44\x65\x20\x73\x6e\x65\x6c\x6c\x65\x20\x62\x72"
         "\x75\x69\x6e\x65\x20\x76\x6f\x73\x20\x73\x70\x72"
         "\x6f\x6e\x67\x20\x6f\x76\x65\x72\x20\x64\x65\x20"
         "\x6c\x75\x69\x65\x20\x68\x6f\x6e\x64\x2e\x00"},
        -1,
        {0x0044, 0x0065, 0x0020, 0x0073, 0x006e, 0x0065, 0x006c, 0x006c, 0x0065,
         0x0020, 0x0062, 0x0072, 0x0075, 0x0069, 0x006e, 0x0065, 0x0020, 0x0076,
         0x006f, 0x0073, 0x0020, 0x0073, 0x0070, 0x0072, 0x006f, 0x006e, 0x0067,
         0x0020, 0x006f, 0x0076, 0x0065, 0x0072, 0x0020, 0x0064, 0x0065, 0x0020,
         0x006c, 0x0075, 0x0069, 0x0065, 0x0020, 0x0068, 0x006f, 0x006e, 0x0064,
         0x002e, 0x0000},
        47,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- French "},
        {"\x4c\x65\x20\x72\x65\x6e\x61\x72\x64\x20\x62\x72"
         "\x75\x6e\x20\x72\x61\x70\x69\x64\x65\x20\x61\x20"
         "\x73\x61\x75\x74\xc3\xa9\x20\x70\x61\x72\x2d\x64"
         "\x65\x73\x73\x75\x73\x20\x6c\x65\x20\x63\x68\x69"
         "\x65\x6e\x20\x70\x61\x72\x65\x73\x73\x65\x75\x78"
         "\x2e\x00"},
        -1,
        {0x004c, 0x0065, 0x0020, 0x0072, 0x0065, 0x006e, 0x0061, 0x0072, 0x0064,
         0x0020, 0x0062, 0x0072, 0x0075, 0x006e, 0x0020, 0x0072, 0x0061, 0x0070,
         0x0069, 0x0064, 0x0065, 0x0020, 0x0061, 0x0020, 0x0073, 0x0061, 0x0075,
         0x0074, 0x00e9, 0x0020, 0x0070, 0x0061, 0x0072, 0x002d, 0x0064, 0x0065,
         0x0073, 0x0073, 0x0075, 0x0073, 0x0020, 0x006c, 0x0065, 0x0020, 0x0063,
         0x0068, 0x0069, 0x0065, 0x006e, 0x0020, 0x0070, 0x0061, 0x0072, 0x0065,
         0x0073, 0x0073, 0x0065, 0x0075, 0x0078, 0x002e, 0x0000},
        61,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- German "},
        {"\x44\x65\x72\x20\x73\x63\x68\x6e\x65\x6c\x6c\x65"
         "\x20\x62\x72\x61\x75\x6e\x65\x20\x46\x75\x63\x68"
         "\x73\x20\x73\x70\x72\x61\x6e\x67\x20\xc3\xbc\x62"
         "\x65\x72\x20\x64\x65\x6e\x20\x66\x61\x75\x6c\x65"
         "\x6e\x20\x48\x75\x6e\x64\x2e\x00"},
        -1,
        {0x0044, 0x0065, 0x0072, 0x0020, 0x0073, 0x0063, 0x0068, 0x006e, 0x0065,
         0x006c, 0x006c, 0x0065, 0x0020, 0x0062, 0x0072, 0x0061, 0x0075, 0x006e,
         0x0065, 0x0020, 0x0046, 0x0075, 0x0063, 0x0068, 0x0073, 0x0020, 0x0073,
         0x0070, 0x0072, 0x0061, 0x006e, 0x0067, 0x0020, 0x00fc, 0x0062, 0x0065,
         0x0072, 0x0020, 0x0064, 0x0065, 0x006e, 0x0020, 0x0066, 0x0061, 0x0075,
         0x006C, 0x0065, 0x006e, 0x0020, 0x0048, 0x0075, 0x006e, 0x0064, 0x002e,
         0x0000},
        55,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Greek "},
        {"\xce\x97\x20\xce\xb3\xcf\x81\xce\xae\xce\xb3\xce"
         "\xbf\xcf\x81\xce\xb7\x20\xce\xba\xce\xb1\xcf\x86"
         "\xce\xb5\xcf\x84\xce\xb9\xce\xac\x20\xce\xb1\xce"
         "\xbb\xce\xb5\xcf\x80\xce\xbf\xcf\x8d\x20\xcf\x80"
         "\xce\xae\xce\xb4\xce\xb7\xcf\x83\xce\xb5\x20\xcf"
         "\x80\xce\xad\xcf\x81\xce\xb1\x20\xce\xb1\xcf\x80"
         "\xcf\x8c\x20\xcf\x84\xce\xbf\x20\xce\xbf\xce\xba"
         "\xce\xbd\xce\xb7\xcf\x81\xcf\x8c\x20\xcf\x83\xce"
         "\xba\xcf\x85\xce\xbb\xce\xaf\x2e\x00"},
        -1,
        {0x0397, 0x0020, 0x03b3, 0x03c1, 0x03ae, 0x03b3, 0x03bf, 0x03c1, 0x03b7,
         0x0020, 0x03ba, 0x03b1, 0x03c6, 0x03b5, 0x03c4, 0x03b9, 0x03ac, 0x0020,
         0x03b1, 0x03bb, 0x03b5, 0x03c0, 0x03bf, 0x03cd, 0x0020, 0x03c0, 0x03ae,
         0x03b4, 0x03b7, 0x03c3, 0x03b5, 0x0020, 0x03c0, 0x03ad, 0x03c1, 0x03b1,
         0x0020, 0x03b1, 0x03c0, 0x03cc, 0x0020, 0x03c4, 0x03bf, 0x0020, 0x03bf,
         0x03ba, 0x03bd, 0x03b7, 0x03c1, 0x03cc, 0x0020, 0x03c3, 0x03ba, 0x03c5,
         0x03bb, 0x03af, 0x002e, 0x0000},
        58,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Italian "},
        {"\x4c\x61\x20\x76\x6f\x6c\x70\x65\x20\x6d\x61\x72"
         "\x72\x6f\x6e\x65\x20\x72\x61\x70\x69\x64\x61\x20"
         "\x68\x61\x20\x73\x61\x6c\x74\x61\x74\x6f\x20\x73"
         "\x6f\x70\x72\x61\x20\x69\x6c\x20\x63\x61\x6e\x65"
         "\x20\x70\x69\x67\x72\x6f\x2e\x00"},
        -1,
        {0x004c, 0x0061, 0x0020, 0x0076, 0x006f, 0x006c, 0x0070, 0x0065, 0x0020,
         0x006d, 0x0061, 0x0072, 0x0072, 0x006f, 0x006e, 0x0065, 0x0020, 0x0072,
         0x0061, 0x0070, 0x0069, 0x0064, 0x0061, 0x0020, 0x0068, 0x0061, 0x0020,
         0x0073, 0x0061, 0x006c, 0x0074, 0x0061, 0x0074, 0x006f, 0x0020, 0x0073,
         0x006f, 0x0070, 0x0072, 0x0061, 0x0020, 0x0069, 0x006c, 0x0020, 0x0063,
         0x0061, 0x006e, 0x0065, 0x0020, 0x0070, 0x0069, 0x0067, 0x0072, 0x006f,
         0x002e, 0x0000},
        56,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Japanese"},
        {"\xe9\x80\x9f\xe3\x81\x84\xe8\x8c\xb6\xe8\x89\xb2"
         "\xe3\x81\xae\xe5\xad\xa4\xe3\x81\xaf\xe4\xb8\x8d"
         "\xe7\xb2\xbe\xe3\x81\xaa\xe7\x8a\xac\xe3\x82\x92"
         "\xe9\xa3\x9b\xe3\x81\xb3\xe8\xb6\x8a\xe3\x81\x97"
         "\xe3\x81\x9f\xe3\x80\x82\x00"},
        -1,
        {0x901f, 0x3044, 0x8336, 0x8272, 0x306e, 0x5b64, 0x306f, 0x4e0d, 0x7cbe,
         0x306a, 0x72ac, 0x3092, 0x98db, 0x3073, 0x8d8a, 0x3057, 0x305f, 0x3002,
         0x000},
        19,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Korean "},
        {"\xeb\xb9\xa0\xeb\xa5\xb8\x20\xea\xb0\x88\xec\x83"
         "\x89\x20\xec\x97\xac\xec\x9a\xb0\xeb\x8a\x94\x20"
         "\xea\xb2\x8c\xec\x9c\xbc\xeb\xa5\xb8\x20\xea\xb0"
         "\x9c\xec\x97\x90\x20\xeb\x9b\xb0\xec\x96\xb4\xec"
         "\x98\xac\xeb\x9e\x90\xeb\x8b\xa4\x2e\x00"},
        -1,
        {0xbe60, 0xb978, 0x0020, 0xac08, 0xc0c9, 0x0020, 0xc5ec, 0xc6b0, 0xb294,
         0x0020, 0xac8c, 0xc73c, 0xb978, 0x0020, 0xac1c, 0xc5d0, 0x0020, 0xb6f0,
         0xc5b4, 0xc62c, 0xb790, 0xb2e4, 0x002e, 0x000},
        24,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Portuguese "},
        {"\x41\x20\x72\x61\x70\x6f\x73\x61\x20\x6d\x61\x72"
         "\x72\x6f\x6d\x20\x72\xc3\xa1\x70\x69\x64\x61\x20"
         "\x73\x61\x6c\x74\x6f\x75\x20\x73\x6f\x62\x72\x65"
         "\x20\x6f\x20\x63\xc3\xa3\x6f\x20\x70\x72\x65\x67"
         "\x75\x69\xc3\xa7\x6f\x73\x6f\x2e\x00"},
        -1,
        {0x0041, 0x0020, 0x0072, 0x0061, 0x0070, 0x006f, 0x0073, 0x0061, 0x0020,
         0x006d, 0x0061, 0x0072, 0x0072, 0x006f, 0x006d, 0x0020, 0x0072, 0x00e1,
         0x0070, 0x0069, 0x0064, 0x0061, 0x0020, 0x0073, 0x0061, 0x006c, 0x0074,
         0x006f, 0x0075, 0x0020, 0x0073, 0x006f, 0x0062, 0x0072, 0x0065, 0x0020,
         0x006f, 0x0020, 0x0063, 0x00e3, 0x006f, 0x0020, 0x0070, 0x0072, 0x0065,
         0x0067, 0x0075, 0x0069, 0x00e7, 0x006f, 0x0073, 0x006f, 0x002e, 0x0000},
        54,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Russian "},
        {"\xd0\x91\xd1\x8b\xd1\x81\xd1\x82\xd1\x80\xd0\xb0"
         "\xd1\x8f\x20\xd0\xba\xd0\xbe\xd1\x80\xd0\xb8\xd1"
         "\x87\xd0\xbd\xd0\xb5\xd0\xb2\xd0\xb0\xd1\x8f\x20"
         "\xd0\xbb\xd0\xb8\xd1\x81\xd0\xb8\xd1\x86\xd0\xb0"
         "\x20\xd0\xbf\xd0\xbe\xd1\x81\xd0\xba\xd0\xb0\xd0"
         "\xba\xd0\xb0\xd0\xbb\xd0\xb0\x20\xd0\xbd\xd0\xb0"
         "\xd0\xb4\x20\xd0\xbb\xd0\xb5\xd0\xbd\xd0\xb8\xd0"
         "\xb2\xd0\xbe\xd0\xb9\x20\xd1\x81\xd0\xbe\xd0\xb1"
         "\xd0\xb0\xd0\xba\xd0\xbe\xd0\xb9\x2e\x00"},
        -1,
        {0x0411, 0x044b, 0x0441, 0x0442, 0x0440, 0x0430, 0x044f, 0x0020, 0x043a,
         0x043e, 0x0440, 0x0438, 0x0447, 0x043d, 0x0435, 0x0432, 0x0430, 0x044f,
         0x0020, 0x043b, 0x0438, 0x0441, 0x0438, 0x0446, 0x0430, 0x0020, 0x043f,
         0x043e, 0x0441, 0x043a, 0x0430, 0x043a, 0x0430, 0x043b, 0x0430, 0x0020,
         0x043d, 0x0430, 0x0434, 0x0020, 0x043b, 0x0435, 0x043d, 0x0438, 0x0432,
         0x043e, 0x0439, 0x0020, 0x0441, 0x043e, 0x0431, 0x0430, 0x043a, 0x043e,
         0x0439, 0x002e, 0x0000},
        57,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Spanish "},
        {"\x45\x6c\x20\x7a\x6f\x72\x72\x6f\x20\x6d\x61\x72"
         "\x72\xc3\xb3\x6e\x20\x72\xc3\xa1\x70\x69\x64\x6f"
         "\x20\x73\x61\x6c\x74\xc3\xb3\x20\x73\x6f\x62\x72"
         "\x65\x20\x65\x6c\x20\x70\x65\x72\x72\x6f\x20\x70"
         "\x65\x72\x65\x7a\x6f\x73\x6f\x2e\x00"},
        -1,
        {0x0045, 0x006c, 0x0020, 0x007a, 0x006f, 0x0072, 0x0072, 0x006f, 0x0020,
         0x006d, 0x0061, 0x0072, 0x0072, 0x00f3, 0x006e, 0x0020, 0x0072, 0x00e1,
         0x0070, 0x0069, 0x0064, 0x006f, 0x0020, 0x0073, 0x0061, 0x006c, 0x0074,
         0x00f3, 0x0020, 0x0073, 0x006f, 0x0062, 0x0072, 0x0065, 0x0020, 0x0065,
         0x006c, 0x0020, 0x0070, 0x0065, 0x0072, 0x0072, 0x006f, 0x0020, 0x0070,
         0x0065, 0x0072, 0x0065, 0x007a, 0x006f, 0x0073, 0x006f, 0x002e, 0x0000},
        54,
        DCLSTAT_SUCCESS
        },
    {
        {"END"},
        {0x00}, 0,
        {0x0000}, 0,
        0
    }
};

static WCT wc_tests[] =
{
    {
        {"A != <ALPHA> "},
        {0x0041, 0x2262, 0x0391, 0x002E, 0x0000},
        5,
        {"\x41\xe2\x89\xa2\xce\x91\x2e\x00"},
        8,
        DCLSTAT_SUCCESS
    },
    {
        {"Hi Mom <smiley> "},
        {0x0048, 0x0069, 0x0020, 0x004D, 0x006F, 0x006D, 0x0020, 0x263A, 0x0021},
        -1,
        {"\x48\x69\x20\x4d\x6f\x6d\x20\xe2\x98\xba\x21\x00"},
        12,
        DCLSTAT_SUCCESS
    },
    {
        {"nihongo "},
        {0x65E5, 0x672C, 0x8A9e, 0x0000},
        4,
        {"\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E\x00"},
        10,
        DCLSTAT_SUCCESS
    },
    {
        {"Embedded NULLS "},
        {0x65E5, 0x0000, 0x672C, 0x0000, 0x8A9e, 0x0000},
        6,
        {"\xE6\x97\xA5\x00\xE6\x9C\xAC\x00\xE8\xAA\x9E\x00"},
        12,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- English "},
        {0x0054, 0x0068, 0x0065, 0x0020, 0x0071, 0x0075, 0x0069, 0x0063, 0x006b,
         0x0020, 0x0062, 0x0072, 0x006f, 0x0077, 0x006e, 0x0020, 0x0066, 0x006f,
         0x0078, 0x0020, 0x006a, 0x0075, 0x006d, 0x0070, 0x0065, 0x0064, 0x0020,
         0x006f, 0x0076, 0x0065, 0x0072, 0x0020, 0x0074, 0x0068, 0x0065, 0x0020,
         0x006c, 0x0061, 0x007a, 0x0079, 0x0020, 0x0064, 0x006f, 0x0067, 0x002e,
         0x0000},
        -1,
        {"\x54\x68\x65\x20\x71\x75\x69\x63\x6b\x20\x62\x72"
         "\x6f\x77\x6e\x20\x66\x6f\x78\x20\x6a\x75\x6d\x70"
         "\x65\x64\x20\x6f\x76\x65\x72\x20\x74\x68\x65\x20"
         "\x6c\x61\x7a\x79\x20\x64\x6f\x67\x2e\x00"},
        46,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Chinese "},
        {0x5feb, 0x7684, 0x68d5, 0x8272, 0x72d0, 0x72f8, 0x8df3, 0x8fc7, 0x4e86,
         0x61d2, 0x60f0, 0x72d7, 0x3002, 0x000},
        -1,
        {"\xe5\xbf\xab\xe7\x9a\x84\xe6\xa3\x95\xe8\x89\xb2"
         "\xe7\x8b\x90\xe7\x8b\xb8\xe8\xb7\xb3\xe8\xbf\x87"
         "\xe4\xba\x86\xe6\x87\x92\xe6\x83\xb0\xe7\x8b\x97"
         "\xe3\x80\x82\x00"},
        40,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Dutch "},
        {0x0044, 0x0065, 0x0020, 0x0073, 0x006e, 0x0065, 0x006c, 0x006c, 0x0065,
         0x0020, 0x0062, 0x0072, 0x0075, 0x0069, 0x006e, 0x0065, 0x0020, 0x0076,
         0x006f, 0x0073, 0x0020, 0x0073, 0x0070, 0x0072, 0x006f, 0x006e, 0x0067,
         0x0020, 0x006f, 0x0076, 0x0065, 0x0072, 0x0020, 0x0064, 0x0065, 0x0020,
         0x006c, 0x0075, 0x0069, 0x0065, 0x0020, 0x0068, 0x006f, 0x006e, 0x0064,
         0x002e, 0x0000},
        -1,
        {"\x44\x65\x20\x73\x6e\x65\x6c\x6c\x65\x20\x62\x72"
         "\x75\x69\x6e\x65\x20\x76\x6f\x73\x20\x73\x70\x72"
         "\x6f\x6e\x67\x20\x6f\x76\x65\x72\x20\x64\x65\x20"
         "\x6c\x75\x69\x65\x20\x68\x6f\x6e\x64\x2e\x00"},
        47,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- French "},
        {0x004c, 0x0065, 0x0020, 0x0072, 0x0065, 0x006e, 0x0061, 0x0072, 0x0064,
         0x0020, 0x0062, 0x0072, 0x0075, 0x006e, 0x0020, 0x0072, 0x0061, 0x0070,
         0x0069, 0x0064, 0x0065, 0x0020, 0x0061, 0x0020, 0x0073, 0x0061, 0x0075,
         0x0074, 0x00e9, 0x0020, 0x0070, 0x0061, 0x0072, 0x002d, 0x0064, 0x0065,
         0x0073, 0x0073, 0x0075, 0x0073, 0x0020, 0x006c, 0x0065, 0x0020, 0x0063,
         0x0068, 0x0069, 0x0065, 0x006e, 0x0020, 0x0070, 0x0061, 0x0072, 0x0065,
         0x0073, 0x0073, 0x0065, 0x0075, 0x0078, 0x002e, 0x0000},
        -1,
        {"\x4c\x65\x20\x72\x65\x6e\x61\x72\x64\x20\x62\x72"
         "\x75\x6e\x20\x72\x61\x70\x69\x64\x65\x20\x61\x20"
         "\x73\x61\x75\x74\xc3\xa9\x20\x70\x61\x72\x2d\x64"
         "\x65\x73\x73\x75\x73\x20\x6c\x65\x20\x63\x68\x69"
         "\x65\x6e\x20\x70\x61\x72\x65\x73\x73\x65\x75\x78"
         "\x2e\x00"},
        62,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- German "},
        {0x0044, 0x0065, 0x0072, 0x0020, 0x0073, 0x0063, 0x0068, 0x006e, 0x0065,
         0x006c, 0x006c, 0x0065, 0x0020, 0x0062, 0x0072, 0x0061, 0x0075, 0x006e,
         0x0065, 0x0020, 0x0046, 0x0075, 0x0063, 0x0068, 0x0073, 0x0020, 0x0073,
         0x0070, 0x0072, 0x0061, 0x006e, 0x0067, 0x0020, 0x00fc, 0x0062, 0x0065,
         0x0072, 0x0020, 0x0064, 0x0065, 0x006e, 0x0020, 0x0066, 0x0061, 0x0075,
         0x006C, 0x0065, 0x006e, 0x0020, 0x0048, 0x0075, 0x006e, 0x0064, 0x002e,
         0x0000},
        -1,
        {"\x44\x65\x72\x20\x73\x63\x68\x6e\x65\x6c\x6c\x65"
         "\x20\x62\x72\x61\x75\x6e\x65\x20\x46\x75\x63\x68"
         "\x73\x20\x73\x70\x72\x61\x6e\x67\x20\xc3\xbc\x62"
         "\x65\x72\x20\x64\x65\x6e\x20\x66\x61\x75\x6c\x65"
         "\x6e\x20\x48\x75\x6e\x64\x2e\x00"},
        56,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Greek "},
        {0x0397, 0x0020, 0x03b3, 0x03c1, 0x03ae, 0x03b3, 0x03bf, 0x03c1, 0x03b7,
         0x0020, 0x03ba, 0x03b1, 0x03c6, 0x03b5, 0x03c4, 0x03b9, 0x03ac, 0x0020,
         0x03b1, 0x03bb, 0x03b5, 0x03c0, 0x03bf, 0x03cd, 0x0020, 0x03c0, 0x03ae,
         0x03b4, 0x03b7, 0x03c3, 0x03b5, 0x0020, 0x03c0, 0x03ad, 0x03c1, 0x03b1,
         0x0020, 0x03b1, 0x03c0, 0x03cc, 0x0020, 0x03c4, 0x03bf, 0x0020, 0x03bf,
         0x03ba, 0x03bd, 0x03b7, 0x03c1, 0x03cc, 0x0020, 0x03c3, 0x03ba, 0x03c5,
         0x03bb, 0x03af, 0x002e, 0x0000},
        -1,
        {"\xce\x97\x20\xce\xb3\xcf\x81\xce\xae\xce\xb3\xce"
         "\xbf\xcf\x81\xce\xb7\x20\xce\xba\xce\xb1\xcf\x86"
         "\xce\xb5\xcf\x84\xce\xb9\xce\xac\x20\xce\xb1\xce"
         "\xbb\xce\xb5\xcf\x80\xce\xbf\xcf\x8d\x20\xcf\x80"
         "\xce\xae\xce\xb4\xce\xb7\xcf\x83\xce\xb5\x20\xcf"
         "\x80\xce\xad\xcf\x81\xce\xb1\x20\xce\xb1\xcf\x80"
         "\xcf\x8c\x20\xcf\x84\xce\xbf\x20\xce\xbf\xce\xba"
         "\xce\xbd\xce\xb7\xcf\x81\xcf\x8c\x20\xcf\x83\xce"
         "\xba\xcf\x85\xce\xbb\xce\xaf\x2e\x00"},
        105,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Italian "},
        {0x004c, 0x0061, 0x0020, 0x0076, 0x006f, 0x006c, 0x0070, 0x0065, 0x0020,
         0x006d, 0x0061, 0x0072, 0x0072, 0x006f, 0x006e, 0x0065, 0x0020, 0x0072,
         0x0061, 0x0070, 0x0069, 0x0064, 0x0061, 0x0020, 0x0068, 0x0061, 0x0020,
         0x0073, 0x0061, 0x006c, 0x0074, 0x0061, 0x0074, 0x006f, 0x0020, 0x0073,
         0x006f, 0x0070, 0x0072, 0x0061, 0x0020, 0x0069, 0x006c, 0x0020, 0x0063,
         0x0061, 0x006e, 0x0065, 0x0020, 0x0070, 0x0069, 0x0067, 0x0072, 0x006f,
         0x002e, 0x0000},
        -1,
        {"\x4c\x61\x20\x76\x6f\x6c\x70\x65\x20\x6d\x61\x72"
         "\x72\x6f\x6e\x65\x20\x72\x61\x70\x69\x64\x61\x20"
         "\x68\x61\x20\x73\x61\x6c\x74\x61\x74\x6f\x20\x73"
         "\x6f\x70\x72\x61\x20\x69\x6c\x20\x63\x61\x6e\x65"
         "\x20\x70\x69\x67\x72\x6f\x2e\x00"},
        56,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Japanese "},
        {0x901f, 0x3044, 0x8336, 0x8272, 0x306e, 0x5b64, 0x306f, 0x4e0d, 0x7cbe,
         0x306a, 0x72ac, 0x3092, 0x98db, 0x3073, 0x8d8a, 0x3057, 0x305f, 0x3002,
         0x000},
        -1,
        {"\xe9\x80\x9f\xe3\x81\x84\xe8\x8c\xb6\xe8\x89\xb2"
         "\xe3\x81\xae\xe5\xad\xa4\xe3\x81\xaf\xe4\xb8\x8d"
         "\xe7\xb2\xbe\xe3\x81\xaa\xe7\x8a\xac\xe3\x82\x92"
         "\xe9\xa3\x9b\xe3\x81\xb3\xe8\xb6\x8a\xe3\x81\x97"
         "\xe3\x81\x9f\xe3\x80\x82\x00"},
        55,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Korean "},
        {0xbe60, 0xb978, 0x0020, 0xac08, 0xc0c9, 0x0020, 0xc5ec, 0xc6b0, 0xb294,
         0x0020, 0xac8c, 0xc73c, 0xb978, 0x0020, 0xac1c, 0xc5d0, 0x0020, 0xb6f0,
         0xc5b4, 0xc62c, 0xb790, 0xb2e4, 0x002e, 0x000},
        -1,
        {"\xeb\xb9\xa0\xeb\xa5\xb8\x20\xea\xb0\x88\xec\x83"
         "\x89\x20\xec\x97\xac\xec\x9a\xb0\xeb\x8a\x94\x20"
         "\xea\xb2\x8c\xec\x9c\xbc\xeb\xa5\xb8\x20\xea\xb0"
         "\x9c\xec\x97\x90\x20\xeb\x9b\xb0\xec\x96\xb4\xec"
         "\x98\xac\xeb\x9e\x90\xeb\x8b\xa4\x2e\x00"},
        58,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Portuguese "},
        {0x0041, 0x0020, 0x0072, 0x0061, 0x0070, 0x006f, 0x0073, 0x0061, 0x0020,
         0x006d, 0x0061, 0x0072, 0x0072, 0x006f, 0x006d, 0x0020, 0x0072, 0x00e1,
         0x0070, 0x0069, 0x0064, 0x0061, 0x0020, 0x0073, 0x0061, 0x006c, 0x0074,
         0x006f, 0x0075, 0x0020, 0x0073, 0x006f, 0x0062, 0x0072, 0x0065, 0x0020,
         0x006f, 0x0020, 0x0063, 0x00e3, 0x006f, 0x0020, 0x0070, 0x0072, 0x0065,
         0x0067, 0x0075, 0x0069, 0x00e7, 0x006f, 0x0073, 0x006f, 0x002e, 0x0000},
        -1,
        {"\x41\x20\x72\x61\x70\x6f\x73\x61\x20\x6d\x61\x72"
         "\x72\x6f\x6d\x20\x72\xc3\xa1\x70\x69\x64\x61\x20"
         "\x73\x61\x6c\x74\x6f\x75\x20\x73\x6f\x62\x72\x65"
         "\x20\x6f\x20\x63\xc3\xa3\x6f\x20\x70\x72\x65\x67"
         "\x75\x69\xc3\xa7\x6f\x73\x6f\x2e\x00"},
        57,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Russian "},
        {0x0411, 0x044b, 0x0441, 0x0442, 0x0440, 0x0430, 0x044f, 0x0020, 0x043a,
         0x043e, 0x0440, 0x0438, 0x0447, 0x043d, 0x0435, 0x0432, 0x0430, 0x044f,
         0x0020, 0x043b, 0x0438, 0x0441, 0x0438, 0x0446, 0x0430, 0x0020, 0x043f,
         0x043e, 0x0441, 0x043a, 0x0430, 0x043a, 0x0430, 0x043b, 0x0430, 0x0020,
         0x043d, 0x0430, 0x0434, 0x0020, 0x043b, 0x0435, 0x043d, 0x0438, 0x0432,
         0x043e, 0x0439, 0x0020, 0x0441, 0x043e, 0x0431, 0x0430, 0x043a, 0x043e,
         0x0439, 0x002e, 0x0000},
        -1,
        {"\xd0\x91\xd1\x8b\xd1\x81\xd1\x82\xd1\x80\xd0\xb0"
         "\xd1\x8f\x20\xd0\xba\xd0\xbe\xd1\x80\xd0\xb8\xd1"
         "\x87\xd0\xbd\xd0\xb5\xd0\xb2\xd0\xb0\xd1\x8f\x20"
         "\xd0\xbb\xd0\xb8\xd1\x81\xd0\xb8\xd1\x86\xd0\xb0"
         "\x20\xd0\xbf\xd0\xbe\xd1\x81\xd0\xba\xd0\xb0\xd0"
         "\xba\xd0\xb0\xd0\xbb\xd0\xb0\x20\xd0\xbd\xd0\xb0"
         "\xd0\xb4\x20\xd0\xbb\xd0\xb5\xd0\xbd\xd0\xb8\xd0"
         "\xb2\xd0\xbe\xd0\xb9\x20\xd1\x81\xd0\xbe\xd0\xb1"
         "\xd0\xb0\xd0\xba\xd0\xbe\xd0\xb9\x2e\x00"},
        106,
        DCLSTAT_SUCCESS
    },
    {
        {"TQBFJOLD -- Spanish "},
        {0x0045, 0x006c, 0x0020, 0x007a, 0x006f, 0x0072, 0x0072, 0x006f, 0x0020,
         0x006d, 0x0061, 0x0072, 0x0072, 0x00f3, 0x006e, 0x0020, 0x0072, 0x00e1,
         0x0070, 0x0069, 0x0064, 0x006f, 0x0020, 0x0073, 0x0061, 0x006c, 0x0074,
         0x00f3, 0x0020, 0x0073, 0x006f, 0x0062, 0x0072, 0x0065, 0x0020, 0x0065,
         0x006c, 0x0020, 0x0070, 0x0065, 0x0072, 0x0072, 0x006f, 0x0020, 0x0070,
         0x0065, 0x0072, 0x0065, 0x007a, 0x006f, 0x0073, 0x006f, 0x002e, 0x0000},
        -1,
        {"\x45\x6c\x20\x7a\x6f\x72\x72\x6f\x20\x6d\x61\x72"
         "\x72\xc3\xb3\x6e\x20\x72\xc3\xa1\x70\x69\x64\x6f"
         "\x20\x73\x61\x6c\x74\xc3\xb3\x20\x73\x6f\x62\x72"
         "\x65\x20\x65\x6c\x20\x70\x65\x72\x72\x6f\x20\x70"
         "\x65\x72\x65\x7a\x6f\x73\x6f\x2e\x00"},
        57,
        DCLSTAT_SUCCESS
        },
    {
        {"END"},
        {0x0000}, 0,
        {0x00}, 0,
        0
    }
};


DCLSTATUS DclTestMBWCConv(void)
{
    if (TestWCChars() != DCLSTAT_SUCCESS)
        return DCLSTAT_CURRENTLINE;

    if (TestWC2MBConv() != DCLSTAT_SUCCESS)
        return DCLSTAT_CURRENTLINE;

    if (TestMB2WCConv() != DCLSTAT_SUCCESS)
        return DCLSTAT_CURRENTLINE;

    if (TestWCConvErr() != DCLSTAT_SUCCESS)
        return DCLSTAT_CURRENTLINE;

    if (TestMBConvErr() != DCLSTAT_SUCCESS)
        return DCLSTAT_CURRENTLINE;

    if (TestOtherError() != DCLSTAT_SUCCESS)
        return DCLSTAT_CURRENTLINE;

    return DCLSTAT_SUCCESS;
}


static DCLSTATUS TestWCChars(void)
{
    D_UINT16    wcStartUnicodeBuf[MAX_WIDECHAR_LEN];
    D_UINT8     cLocUTF8Buf[MAX_MULTIBYTE_LEN];
    D_UINT16    wcLocUnicodeBuf[MAX_WIDECHAR_LEN];

    unsigned    uStartUnicodeBufLen;
    unsigned    uLocUTF8BufLen;
    unsigned    uLocUnicodeBufLen;

    size_t      uConvertedLength;

    unsigned    i, j;

    DCLSTATUS   dclStat = DCLSTAT_SUCCESS;

    /*  Verify that each wide character value can be converted to a multibyte
        character and converted back from multibyte character to the original
        wide character.
    */

    DclPrintf("Testing all wide characters -- ");
    i = 0;

    do
    {
        wcStartUnicodeBuf[0] = (D_UINT16)i;
        wcStartUnicodeBuf[1] = (D_UINT16)0;
        uStartUnicodeBufLen = 2;

        /*  determine how may bytes are needed to convert from wide char to
            multibyte.
        */
        dclStat = DclWideCharToMultiByte(wcStartUnicodeBuf, (int)uStartUnicodeBufLen,
                                            0, 0, &uConvertedLength);
        if (dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("Failed! Unable to determine multibyte length for 0x%04x. Status=%04x\n",
                            i, dclStat);
            break;
        }

        uLocUTF8BufLen = uConvertedLength;
        dclStat = DclWideCharToMultiByte(wcStartUnicodeBuf, (int)uStartUnicodeBufLen,
                                         (char *)cLocUTF8Buf, uLocUTF8BufLen, &uConvertedLength);
        if (dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("Failed! Unable to convert 0x%04x to multibyte. Status=%04x\n",
                            i, dclStat);
            break;
        }

        /*  determine how many wide chars are needed to convert from UTF8 to
            unicode with local function
        */
        dclStat = DclMultiByteToWideChar((char *)cLocUTF8Buf, (int)uLocUTF8BufLen, 0, 0, &uConvertedLength);
        if (dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("Failed! Unable to convert %04x back from multibyte. Status=%04x\n",
                            i, dclStat);
            break;
        }

        uLocUnicodeBufLen = uConvertedLength;
        dclStat = DclMultiByteToWideChar((char *)cLocUTF8Buf, (int)uLocUTF8BufLen, wcLocUnicodeBuf,
                                            uLocUnicodeBufLen, &uConvertedLength);
        if (dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("Failed! Unable to convert %04x back from multibyte. Status=%04x\n",
                            i, dclStat);
            break;
        }

        /*  compare original wide character string with converted string
        */
        j = 0;
        while (j < uConvertedLength)
        {
            if (wcLocUnicodeBuf[j] != wcStartUnicodeBuf[j])
                break;
            j++;
        }

        if (j != uConvertedLength)
        {
            DclPrintf("Failed!  Converted string does not match original!\n");
            dclStat = DCLSTAT_CURRENTLINE;
            break;
        }

        i++;
    } while (i != MAX_UNICODE);

    if (i == MAX_UNICODE || dclStat == DCLSTAT_SUCCESS)
    {
        DclPrintf("Success!\n");
    }

    return dclStat;
}

static DCLSTATUS TestWC2MBConv(void)
{
    char        cLocUTF8Buf[MAX_MULTIBYTE_LEN];
    unsigned    uLocUTF8BufLen;
    size_t      uConvertedLength;
    unsigned i, j;
    DCLSTATUS   dclStat = DCLSTAT_SUCCESS;

    /*  wide character string tests
    */
    i = 0;

    DclPrintf("Convert wide character strings to multibyte\n");

    while (wc_tests[i].nSrcStringLen != 0 || wc_tests[i].uDstStringLen != 0)
    {
        DclPrintf("\tTest case %s - ", wc_tests[i].szDescription);
        dclStat = DclWideCharToMultiByte(wc_tests[i].wcSrcString, wc_tests[i].nSrcStringLen, 0 ,0, &uConvertedLength );

        if (dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("Failed to determine length! Status=%04x\n", dclStat);
            break;
        }

        else if (uConvertedLength != wc_tests[i].uDstStringLen)
        {
            DclPrintf("Failed! Converted length does not match expected length! Status=%04x\n", dclStat);
            break;
        }

        else
        {
            uLocUTF8BufLen = uConvertedLength;
            dclStat = DclWideCharToMultiByte(wc_tests[i].wcSrcString, wc_tests[i].nSrcStringLen,
                                                cLocUTF8Buf, uLocUTF8BufLen, &uConvertedLength );
            if (dclStat != wc_tests[i].uExpectedResult)
            {
                DclPrintf("Failed to convert! Status = %04x\n", dclStat);
                break;
            }
            else if (uConvertedLength != uLocUTF8BufLen)
            {
                DclPrintf("Failed! Calculated Length(%d) does not match converted length(%d)\n",
                                                uLocUTF8BufLen, uConvertedLength);
                break;
            }
            else if (uConvertedLength != wc_tests[i].uDstStringLen)
            {
                DclPrintf("Failed! Converted length(%d) does not match expected length(%d)\n",
                                                uConvertedLength, wc_tests[i].uDstStringLen);
                break;
            }
            else
            {
                j = 0;

                while (j < uConvertedLength && wc_tests[i].mbDstString[j] == cLocUTF8Buf[j])
                {
                    j++;
                }

                if (j == uConvertedLength)
                    DclPrintf("Passed!\n");
                else
                    DclPrintf("Failed! Character mismatch at offset %d.  "
                                        "Expected %02x found %02x\n",
                                        j, wc_tests[i].mbDstString[j],
                                        cLocUTF8Buf[j]);
            }
        }

        i++;
    }

    return dclStat;
}


static DCLSTATUS TestMB2WCConv(void)
{
    D_UINT16    wcLocUnicodeBuf[MAX_WIDECHAR_LEN];
    unsigned    uLocUnicodeBufLen;
    size_t      uConvertedLength;
    unsigned    i, j;
    DCLSTATUS   dclStat = DCLSTAT_SUCCESS;

    /*  multibyte character string tests
    */
    i = 0;

    DclPrintf("Convert multibyte character strings to wide\n");

    while (mb_tests[i].nSrcStringLen != 0 || mb_tests[i].uDstStringLen != 0)
    {
        DclPrintf("\tTest case %s - ", mb_tests[i].szDescription);
        dclStat = DclMultiByteToWideChar(mb_tests[i].cSrcString, mb_tests[i].nSrcStringLen, 0 ,0, &uConvertedLength );

        if (dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("Failed to determine length! Status=%04x\n", dclStat);
        }
        else
        {

            if (uConvertedLength != mb_tests[i].uDstStringLen)
            {
                DclPrintf("Failed! Converted length(%d) does not match expected length(%d)\n", uConvertedLength, mb_tests[i].uDstStringLen);
            }

            uLocUnicodeBufLen = uConvertedLength;
            dclStat = DclMultiByteToWideChar(mb_tests[i].cSrcString, mb_tests[i].nSrcStringLen, wcLocUnicodeBuf, uLocUnicodeBufLen, &uConvertedLength );
            if (dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Failed to convert! Status = %04x\n", dclStat);
            }
            else if (uConvertedLength != uLocUnicodeBufLen)
            {
                DclPrintf("Failed! Calculated Length(%d) does not match converted length(%d)\n", uLocUnicodeBufLen, uConvertedLength);
            }
            else if (uConvertedLength != mb_tests[i].uDstStringLen)
            {
                DclPrintf("Failed! Converted length(%d) does not match expected length(%d)\n", uConvertedLength, mb_tests[i].uDstStringLen);
            }
            else
            {
                j = 0;

                while (j < uConvertedLength && mb_tests[i].wcDstString[j] == wcLocUnicodeBuf[j])
                {
                    j++;
                }

                if (j == uConvertedLength)
                {
                    DclPrintf("Passed!\n");
                }
                else
                    DclPrintf("Failed! Character mismatch at offset %d.  "
                                        "Expected %04x found %04x\n",
                                        j, mb_tests[i].wcDstString[j],
                                        wcLocUnicodeBuf[j]);
            }
        }

        i++;
    }


    return dclStat;

}

static DCLSTATUS TestWCConvErr(void)
{
    size_t      uConvertedLength;
    unsigned    i;
    DCLSTATUS   dclStat = DCLSTAT_CURRENTLINE;

    i = 0;

    DclPrintf("Testing wide character failure tests\n");

    while (wc_fail_tests[i].nSrcStringLen != 0 || wc_fail_tests[i].uDstStringLen != 0)
    {
        DclPrintf("\tTest case %s - ", wc_fail_tests[i].szDescription);
        dclStat = DclWideCharToMultiByte(wc_fail_tests[i].wcSrcString, wc_fail_tests[i].nSrcStringLen,
                                         wc_fail_tests[i].mbDstString, wc_fail_tests[i].uDstStringLen,
                                         &uConvertedLength);

        if (dclStat != wc_fail_tests[i].uExpectedResult)
        {
            DclPrintf("Failed!  Expected stats %04x -- received %04x\n",
                            wc_fail_tests[i].uExpectedResult, dclStat);
            dclStat = DCLSTAT_CURRENTLINE;
            break;
        }
        else
            DclPrintf("Success!\n");

        i++;
    }

    if (dclStat != DCLSTAT_SUCCESS)
        return DCLSTAT_SUCCESS;
    else
        return DCLSTAT_CURRENTLINE;
}

static DCLSTATUS TestMBConvErr(void)
{
    size_t      uConvertedLength;
    unsigned    i;
    DCLSTATUS   dclStat = DCLSTAT_SUCCESS;

    i = 0;

    DclPrintf("Testing multibyte character failure tests\n");

    while (mb_fail_tests[i].nSrcStringLen != 0 || mb_fail_tests[i].uDstStringLen != 0)
    {
        DclPrintf("\tTest case %s - ", mb_fail_tests[i].szDescription);
        dclStat = DclMultiByteToWideChar(mb_fail_tests[i].cSrcString, mb_fail_tests[i].nSrcStringLen,
                                         mb_fail_tests[i].wcDstString, mb_fail_tests[i].uDstStringLen,
                                         &uConvertedLength);

        if (dclStat != mb_fail_tests[i].uExpectedResult)
        {
            DclPrintf("Failed!  Expected stats %04x -- received %04x\n",
                            mb_fail_tests[i].uExpectedResult, dclStat);
            dclStat = DCLSTAT_CURRENTLINE;
            break;
        }
        else
            DclPrintf("Success!\n");

        i++;
    }

    if (dclStat != DCLSTAT_SUCCESS)
        return DCLSTAT_SUCCESS;
    else
        return DCLSTAT_CURRENTLINE;

}


static DCLSTATUS TestOtherError(void)
{
    char        cLocUTF8Buf[MAX_MULTIBYTE_LEN];
    D_UINT16    wcLocUnicodeBuf[MAX_WIDECHAR_LEN];
    unsigned    uLocUTF8BufLen = MAX_MULTIBYTE_LEN;
    unsigned    uLocUnicodeBufLen = MAX_WIDECHAR_LEN;
    size_t      uConvertedLength;
    DCLSTATUS   dclStat = DCLSTAT_SUCCESS;

    /*  Testing null source string pointers
    */
    DclPrintf("Testing MB2WC Null src pointer - ");
    dclStat = DclMultiByteToWideChar(NULL, (int)uLocUTF8BufLen, 0, 0, &uConvertedLength);

    if (dclStat == DCLSTAT_BADPARAMETER)
    {
        DclPrintf("Success!\n");
    }
    else
    {
        DclPrintf("Expected status %04x, received %04x\n", DCLSTAT_BADPARAMETER, dclStat);
    }

    DclPrintf("Testing MB2WC Null dst pointer - ");
    dclStat = DclMultiByteToWideChar(cLocUTF8Buf, (int)uLocUTF8BufLen, 0, uLocUnicodeBufLen, &uConvertedLength);
    if (dclStat == DCLSTAT_BADPARAMETER)
    {
        DclPrintf("Success!\n");
    }
    else
    {
        DclPrintf("Expected status %04x, received %04x\n", DCLSTAT_BADPARAMETER, dclStat);
    }

    DclPrintf("Testing WC2MB Null src pointer - ");
    dclStat = DclWideCharToMultiByte(NULL, (int)uLocUnicodeBufLen, 0, 0, &uConvertedLength);

    if (dclStat == DCLSTAT_BADPARAMETER)
    {
        DclPrintf("Success!\n");
    }
    else
    {
        DclPrintf("Expected status %04x, received %04x\n", DCLSTAT_BADPARAMETER, dclStat);
    }

    DclPrintf("Testing WC2MB Null dst pointer - ");
    dclStat = DclWideCharToMultiByte(wcLocUnicodeBuf, (int)uLocUnicodeBufLen, 0, uLocUTF8BufLen, &uConvertedLength);
    if (dclStat == DCLSTAT_BADPARAMETER)
    {
        DclPrintf("Success!\n");
    }
    else
    {
        DclPrintf("Expected status %04x, received %04x\n", DCLSTAT_BADPARAMETER, dclStat);
    }

    return DCLSTAT_SUCCESS;

}


