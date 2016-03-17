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

    Header file that describes the UTF-8 Case Folding data structure.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltutf_casedata.h $
    Revision 1.2  2009/05/26 21:06:37Z  keithg
    Corrected name of array holding UTF-8 test case data.
    Revision 1.1  2009/05/26 17:25:34Z  keithg
    Initial revision
    Revision 1.4  2009/05/16 22:24:45Z  keithg
    Removed private test structure declaration, added test case characters.
    Revision 1.3  2009/05/16 00:09:10Z  keithg
    Added upper case characters and simple ascii string for test cases.
    Revision 1.2  2009/05/15 01:05:20Z  keithg
    Added extern data declaration for negative tests of UTF case folding.
    Revision 1.1  2009/05/14 19:03:24Z  keithg
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLTFUTF_STRINGS_H_INCLUDED
#define DLTFUTF_STRINGS_H_INCLUDED

struct _tagUnicodeCaseData {
    D_UINT32 ulUpperCase;
    char      cType;
    D_UINT32 ulLowerCase;
};

extern const struct _tagUnicodeCaseData gcasUnicodeCaseTestData[];
extern const D_UINT32 gcaulUnicodeFullCaseChars[];
extern const char * gcaszUtf8ICanEatGlass[];

/*  This is a special case test string.  With UTF case folding, the
    'case' and 'CASE' translations will *not* be the same in that the
    Greek captial letter IOTA is translated to the small letter Iota
    with a tonos accent mark.  In a case insensitive or a sensitive
    state, these are two different files.
*/
#define _Greek_CASE "\xce\xa0\xce\x95\xce\xa1\xce\x99\xce\xa0\xce\xa4\xce\xa9\xce\xa3\xce\x97"
#define _Greek_case "\xcf\x80\xce\xb5\xcf\x81\xce\xaf\xcf\x80\xcf\x84\xcf\x89\xcf\x83\xce\xb7"

#define _Greek_UPPER   "\xce\x91\xce\x92\xce\x93\xce\x94"
#define _Greek_lower   "\xce\xb1\xce\xb2\xce\xb3\xce\xb4"
#define _Greek_MiXed   "\xce\x91\xce\xb2\xce\x93\xce\xb4"


#define TEST_ASCII_LC_BOUNDARY_STRING \
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz0123456789"

#define TEST_UTF8_1B_UPPER_CHAR  "\x58"
#define TEST_UTF8_1B_LOWER_CHAR  "\x78"
#define TEST_UTF8_2B_UPPER_CHAR  "\xd3\x9e"
#define TEST_UTF8_2B_LOWER_CHAR  "\xd3\x9f"
#define TEST_UTF8_3B_UPPER_CHAR  "\xef\xbc\xb8"
#define TEST_UTF8_3B_LOWER_CHAR  "\xef\xbd\x98"

#define TEST_UTF8_UPPER_SPEC_CHAR "\xc5\xbf"
#define TEST_UTF8_LOWER_SPEC_CHAR "\x73"


#endif



