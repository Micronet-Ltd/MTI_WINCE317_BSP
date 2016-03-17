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

    This module contains code to format a version number in the Datalight
    standard form.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlver.c $
    Revision 1.8  2010/12/01 01:05:27Z  garyp
    Minor twealk to allow a version number in the form "4.1a".
    Revision 1.7  2010/04/17 22:08:49Z  garyp
    Minor fix to ensure that a revision letter will only be displayed if a 
    subminor version number is specified.  Documentation updated.
    Revision 1.6  2009/05/20 00:37:36Z  garyp
    Updated to work with the new version number format, which uses all four
    bytes of the number for version information, and no longer includes flags
    for "Alpha" or "Beta".
    Revision 1.5  2009/05/02 17:25:19Z  garyp
    Fixed to not use indeterminate buffer lengths when calling DclLtoA().
    Revision 1.4  2007/12/18 04:28:33Z  brandont
    Updated function headers.
    Revision 1.3  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.2  2007/08/19 17:34:38Z  garyp
    Re-added this file to the project -- but different functionality.
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Protected: DclVersionFormat()

    Format a Datalight standand version number string.  The following
    are some examples of typical output forms:

        "1.0"
        "1.0a"
        "1.2"
        "1.2a"
        "1.2.2"
        "1.2.2a"
        "1.2.1 Build 123"
        "1.2.1a Build 123"
        "2.x.x"

    Some illegal values are:
        "1a"
        "1.2.0a"

    The version number value is a 32-bit segmented number where the
    high byte is the major part of the number, the next highest byte
    is the minor part of the number, the 3d byte is the subminor part
    of the number, and the lowest byte is a revision letter.

    The build number is included only if a pointer to it is supplied.

    Any numeric portion of the version number which has a value of 255
    will be output as an 'x'.  This is useful when version numbers for
    the next release are not yet known.

    The subminor portion of the version number will only be output
    if it is non-zero.  The subminor value may also be a revision 
    letter rather than a number.  Any value less than 0x41 (A) is 
    treated as a numeric value, and 0x41 and higher are treated as
    character values.  

    The revision letter of the version number will only be output
    if it is non-zero.  If the subminor portion of the version
    number is zero, then the revision letter must also be zero.
    The revision letter is normally always lower case.

    Parameters:
        pBuffer     - A pointer to the output buffer to use
        nBuffLen    - The output buffer length
        ulVersion   - The 32-bit version number value, in the
                      standard form as defined in xxver.h.
        pszBuildNum - An optional build number string.  May be NULL
                      if the build number is not to be processed.

    Return Value:
        Returns pBuffer if successful, or NULL if there was not
        enough buffer space (in which case the contents of pBuffer
        may be modified).
-------------------------------------------------------------------*/
char * DclVersionFormat(
    char           *pBuffer,
    unsigned        nBuffLen,
    D_UINT32        ulVersion,
    const char     *pszBuildNum)
{
    #define         MAXLEN  (4)             /* room enough for '255' plus a null */
    D_UINT8         major, minor, subminor, revision;
    char            szMajor[MAXLEN] = "x";
    char            szMinor[MAXLEN] = "x";
    char            szSubminor[MAXLEN+1] = ".x";   /* extra byte for the '.' */
    char            szRevision[MAXLEN] = {0};
    int             iLen;
    size_t          nLen;

    DclAssert(pBuffer);
    DclAssert(nBuffLen);

    major = (D_UINT8)(ulVersion >> 24) & 0xFF;
    minor = (D_UINT8)(ulVersion >> 16) & 0xFF;
    subminor = (D_UINT8)(ulVersion >>  8) & 0xFF;
    revision = (D_UINT8)ulVersion;

    /*  If any of the numeric components of the version number is 0xFF, output
        'x' rather than 255.  This is a placeholder for "I don't know yet",
        which is often the case when moving to a new version.
    */
    if(major != 0xFF)
    {
        nLen = sizeof(szMajor);
        DclLtoA(szMajor, &nLen, major, 0, 0);
    }

    if(minor != 0xFF)
    {
        nLen = sizeof(szMinor);
        DclLtoA(szMinor, &nLen, minor, 0, 0);
    }

    if(subminor == 0)
    {
        /*  The subminor value is zero -- the revision letter must also
            be zero.
        */            
        szSubminor[0] = 0;

        DclAssert(!revision);
    }
    else if(subminor < 'A')
    {
        /*  Format the subminor value is treated as a number, then append a 
            revision letter if specified.
        */            
        nLen = sizeof(szSubminor);
        DclLtoA(&szSubminor[1], &nLen, subminor, 0, 0);

        szRevision[0] = revision;
    }
    else if(subminor < 0xFF)
    {
        /*  Format the subminor value is treated as a letter, then append a
            revision letter if specified.  Technically this allows for two 
            revision letters, however that is not a standard use case.
        */            
        szSubminor[0] = subminor;
        szSubminor[1] = 0;
            szRevision[0] = revision;
    }

    iLen = DclSNPrintf(pBuffer, nBuffLen, "%s.%s%s%s", szMajor, szMinor, szSubminor, szRevision);
    if(iLen == -1)
        return NULL;

    if(pszBuildNum)
    {
        iLen = DclSNPrintf(pBuffer+iLen, nBuffLen-iLen, " Build %s", pszBuildNum);
        if(iLen == -1)
            return NULL;

        iLen = DclStrLen(pBuffer);
    }

    if(iLen == -1)
        return NULL;
    else
        return pBuffer;
}
