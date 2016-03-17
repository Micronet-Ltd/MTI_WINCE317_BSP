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

    This module contains functions for manipulating an array of environment
    strings.  The environment size is dynamic and will grow as needed.  It
    will not shrink.  This code is not currently designed to be thread-safe
    in the sense that multiple threads may not access the same environment.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlenviron.c $
    Revision 1.8  2010/11/01 04:08:18Z  garyp
    Added support for the predefined variables RANDOM8, RANDOM16, and RANDOM32.
    No longer use a private memmove() implementation.  Updated to use
    size_t where appropriate.
    Revision 1.7  2009/11/07 23:10:46Z  garyp
    Relaxed some parameter validation.
    Revision 1.6  2009/03/23 01:07:49Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.5.1.2  2009/03/23 01:07:49Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.5  2007/12/18 04:51:50Z  brandont
    Updated function headers.
    Revision 1.4  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.3  2007/06/24 02:23:20Z  Garyp
    Modified so DclEnvVarAdd() will delete a variable if the value string is
    NULL.
    Revision 1.2  2007/05/17 18:36:08Z  garyp
    Fixed to initialize the environment signature.
    Revision 1.1  2007/05/14 17:34:48Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#define DEFAULT_ENV_LENGTH  (128)

#define ENVIRONMENT_SIG     "DENV"

typedef struct DCLENVIRONMENT
{
    char        acSignature[4];     /* Signature = "DENV"                   */
    unsigned    nBufferSize;        /* The allocated buffer size            */
    unsigned    nBufferUsed;        /* The total size currently used by string (not including this header) */
    char       *pszBuffer;          /* A pointer to the environment data    */
    char        szRandomBuff[10];   /* Large enough for a 32-bit random number */
} DCLENVIRONMENT;


/*-------------------------------------------------------------------
    Public: DclEnvCreate()

    Create a DCL environment construct.  Note that the environment
    is intended for use by single-threaded access.  Multithreaded
    accesses to the environment will likely result in corruption.

    Parameters:
        nEnvLen   - The initial environment length to use.  If this
                    value is zero, then it will be automatically
                    determined by the length of the pszImport array,
                    if specified, or an arbitrary default value of
                    128 will be used.
        pszImport - An optional pointer to an array of environment
                    strings to import into the new environment.  If
                    the length of this array is greater than nEnvLen,
                    and nEnvLen is not zero, the function will fail.

    Return Value:
        The environment handle if successful, or NULL otherwise.
-------------------------------------------------------------------*/
DCLENVHANDLE DclEnvCreate(
    size_t          nEnvLen,
    const char     *pszImport)
{
    size_t          nImportLen = 0;
    DCLENVIRONMENT *pEnv;

    if(pszImport)
    {
        const char *pszTmp = pszImport;

        while(TRUE)
        {
            size_t nVarLen = DclStrLen(pszTmp) + 1;

            nImportLen += nVarLen;

            if(nVarLen == 1)
                break;

            pszTmp += nVarLen;
        }
    }

    if(!nEnvLen)
    {
        if(nImportLen)
            nEnvLen = nImportLen << 1;
        else
            nEnvLen = DEFAULT_ENV_LENGTH;
    }
    else
    {
        if(nEnvLen < nImportLen)
            return NULL;
    }

    pEnv = DclMemAllocZero(sizeof *pEnv);
    if(!pEnv)
        return NULL;

    pEnv->pszBuffer = DclMemAlloc(nEnvLen);
    if(!pEnv->pszBuffer)
    {
        DclMemFree(pEnv);
        return NULL;
    }

    DclMemCpy(pEnv->acSignature, ENVIRONMENT_SIG, sizeof pEnv->acSignature);
    pEnv->nBufferSize = nEnvLen;

    if(pszImport)
    {
        DclMemCpy(pEnv->pszBuffer, pszImport, nImportLen);
        pEnv->nBufferUsed = nImportLen;
    }
    else
    {
        pEnv->pszBuffer[0] = 0;
        pEnv->nBufferUsed = 1;
    }

    return (DCLENVHANDLE)pEnv;
}


/*-------------------------------------------------------------------
    Public: DclEnvDestroy()

    Create a DCL environment construct.

    Parameters:
        hEnv      - The environment handle.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclEnvDestroy(
    DCLENVHANDLE    hEnv)
{
    DclAssert(hEnv);
    DclAssert(hEnv->pszBuffer[hEnv->nBufferUsed-1] == 0);

    DclMemFree(hEnv->pszBuffer);
    DclMemFree(hEnv);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclEnvDisplay()

    Display the contents of the environment.

    Parameters:
        hEnv      - The environment handle.
        pszMatch  - An optional pointer to a string to match

    Return Value:
        Returns DCLSTAT_SUCCESS if at least one matching string was
        displayed, or DCLSTAT_FAILURE if there were no matches.
-------------------------------------------------------------------*/
DCLSTATUS DclEnvDisplay(
    DCLENVHANDLE    hEnv,
    const char     *pszMatch)
{
    const char     *pStr;
    size_t          nMatchLen = 0;
    unsigned        nMatchCount = 0;
    size_t          nMatchSize = 0;

    DclAssert(hEnv);
    DclAssert(hEnv->pszBuffer[hEnv->nBufferUsed-1] == 0);

    if(pszMatch)
        nMatchLen = DclStrLen(pszMatch);

    pStr = hEnv->pszBuffer;

    while(*pStr)
    {
        if(nMatchLen)
        {
            if(DclStrNICmp(pStr, pszMatch, nMatchLen) == 0)
            {
                DclPrintf("%s\n", pStr);

                nMatchCount++;
                nMatchSize += DclStrLen(pStr) + 1;
            }
        }
        else
        {
            DclPrintf("%s\n", pStr);

            nMatchCount++;
            nMatchSize += DclStrLen(pStr) + 1;
        }

        pStr += DclStrLen(pStr) + 1;
    }

    DclPrintf("Found %u matching strings using %u of %u total available bytes.\n",
        nMatchCount, nMatchSize, hEnv->nBufferSize-1);

    if(nMatchCount)
        return DCLSTAT_SUCCESS;
    else
        return DCLSTAT_FAILURE;
}


/*-------------------------------------------------------------------
    Public: DclEnvSubstitute()

    Perform environment variable substitution on the specified
    string.  The substitution is done in place.  If there is not
    enough buffer space, the results are undefined.

    Parameters:
        hEnv       - The shell instance being operated on.
        pszBuffer  - A buffer in which the input is stored.
        uMaxLen    - The buffer length

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclEnvSubstitute(
    DCLENVHANDLE    hEnv,
    char           *pszBuffer,
    size_t          nMaxLen)
{
    size_t          nStrLen;    /* Current string size including the terminating null */

    DclAssert(hEnv);
    DclAssert(pszBuffer);
    DclAssert(nMaxLen);

    nStrLen = DclStrLen(pszBuffer) + 1;

    if(nStrLen > nMaxLen)
        return DCLSTAT_ENV_SUBSTPARAMS;

    while(*pszBuffer)
    {
        char   *pEnd;
        size_t  nVarLen;

        DclAssert(nMaxLen);

        if(*pszBuffer != '%')
        {
            pszBuffer++;
            nMaxLen--;
            continue;
        }

        pEnd = DclStrChr(pszBuffer+1, '%');
        if(!pEnd)
            return DCLSTAT_ENV_SUBSTSYNTAX;

        /*  Change the ending percent to a null
        */
        *pEnd = 0;

        nVarLen = DclStrLen(pszBuffer+1);
        if(nVarLen == 0)
        {
            /*  We found double percent signs, so simply remove one
                of them, and shorten the whole string by one byte.
            */
            DclStrCpy(pszBuffer+1, pszBuffer+2);
            nStrLen--;

            pszBuffer++;
            nMaxLen--;
            continue;
        }
        else
        {
            const char *pValue;

            /*  Modify nVarLen so it includes the leading and trailing
                percent signs.
            */
            nVarLen += 2;

            pValue = DclEnvVarLookup(hEnv, pszBuffer+1);
            if(!pValue)
            {
                /*  The environment variable was not found, so quietly
                    remove it from the string.
                */
                DclStrCpy(pszBuffer, pszBuffer+nVarLen);

                nStrLen -= nVarLen;
                continue;
            }
            else
            {
                size_t  nValueLen = DclStrLen(pValue);

                if((nStrLen - nVarLen) + nValueLen > nMaxLen)
                    return DCLSTAT_ENV_SUBSTOVERFLOW;

                /*  Safely shift the remainder of the data left or right as
                    needed, and stuff in the value from the environment.
                */
                DclMemMove(pszBuffer+nValueLen, pszBuffer+nVarLen, DclStrLen(pszBuffer+nVarLen) + 1);
                DclMemCpy(pszBuffer, pValue, nValueLen);

                nStrLen = (nStrLen - nVarLen) + nValueLen;
                continue;
            }
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclEnvVarAdd()

    Add an environment variable.  Note that the pszVar and
    pszValue strings should not contain any leading or trailing
    white space.  pszVar may not contain an equal '=' sign.

    Parameters:
        hEnv     - The enviroment handle.
        pszVar   - A pointer to the null-terminated variable name
                   to add.  Environment variable names are not
                   case-sensitive, though case is preserved.
        pszValue - A pointer to the null-terminated value.  If this
                   is an empty string, pszVar will be removed from
                   the environment, if it exists.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclEnvVarAdd(
    DCLENVHANDLE    hEnv,
    const char     *pszVar,
    const char     *pszValue)
{
    size_t          nLen;
    size_t          nOldLen = 0;
    const char     *pOld;

    DclAssert(hEnv);
    DclAssert(pszVar);
    DclAssert(*pszVar);
    DclAssert(pszValue);
    DclAssert(hEnv->pszBuffer[hEnv->nBufferUsed-1] == 0);

    /*  If this is an empty string, remove the variable from the environment
    */
    if(!(*pszValue))
    {
        DclEnvVarDelete(hEnv, pszVar);
        return DCLSTAT_SUCCESS;
    }

    /*  NOTE:  This sequence is designed so that if the environment
               cannot be expanded (can't allocate memory), any original
               value in the environment will remain unchanged.
    */

    nLen = DclStrLen(pszVar) + 1;        /* +1 for the '=' */

    /*  Find the length of any original string
    */
    pOld = DclEnvVarLookup(hEnv, pszVar);
    if(pOld)
        nOldLen = nLen + DclStrLen(pOld) + 1;

    /*  Place in nLen the full length of the new var=value string
    */
    nLen += DclStrLen(pszValue) + 1;     /* +1 for the null-terminator   */

    /*  If there is not enough room in the environment, even after
        any original string is deleted...
    */
    if(hEnv->nBufferSize - (hEnv->nBufferUsed - nOldLen) < nLen)
    {
        unsigned    nNewSize = hEnv->nBufferSize + (hEnv->nBufferSize >> 1) + nLen;
        char       *pBuff = DclMemAlloc(nNewSize);

        if(!pBuff)
            return DCLSTAT_MEMALLOCFAILED;

        DclMemCpy(pBuff, hEnv->pszBuffer, hEnv->nBufferUsed);

        DclMemFree(hEnv->pszBuffer);

        hEnv->pszBuffer = pBuff;
        hEnv->nBufferSize = nNewSize;
    }

    /*  Delete the variable if it previously existed
    */
    DclEnvVarDelete(hEnv, pszVar);

    DclAssert(hEnv->nBufferSize - hEnv->nBufferUsed >= nLen);

    DclStrCpy(&hEnv->pszBuffer[hEnv->nBufferUsed-1], pszVar);
    DclStrCat(&hEnv->pszBuffer[hEnv->nBufferUsed-1], "=");
    DclStrCat(&hEnv->pszBuffer[hEnv->nBufferUsed-1], pszValue);

    hEnv->nBufferUsed += nLen;

    /*  Add the double null terminator
    */
    hEnv->pszBuffer[hEnv->nBufferUsed - 1] = 0;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclEnvVarDelete()

    Delete an environment variable by name.

    Parameters:
        hEnv    - The enviroment handle.
        pszVar  - A pointer to the null-terminated variable name
                  to delete.  Environment variable names are not
                  case-sensitive, though case is preserved.

    Return Value:
        Returns the length of the VAR=value data (including the NULL)
        which was removed from the environment.  Returns zero if the
        variable did not exist in the environment.
-------------------------------------------------------------------*/
size_t DclEnvVarDelete(
    DCLENVHANDLE    hEnv,
    const char     *pszVar)
{
    size_t          nVarLen;
    char           *pStr;

    DclAssert(hEnv);
    DclAssert(pszVar);
    DclAssert(*pszVar);
    DclAssert(hEnv->pszBuffer[hEnv->nBufferUsed-1] == 0);

    nVarLen = DclStrLen(pszVar);

    pStr = hEnv->pszBuffer;

    while(*pStr)
    {
        if((DclStrNICmp(pStr, pszVar, nVarLen) == 0) && (pStr[nVarLen] == '='))
        {
            ptrdiff_t   nPrevious = pStr - hEnv->pszBuffer;
            size_t      nLen = DclStrLen(pStr) + 1;

            DclMemCpy(pStr, pStr+nLen, hEnv->nBufferUsed - (nPrevious + nLen));

            hEnv->nBufferUsed -= nLen;

            DclAssert(hEnv->pszBuffer[hEnv->nBufferUsed-1] == 0);

            return nLen;
        }

        pStr += DclStrLen(pStr) + 1;
    }

    return 0;
}


/*-------------------------------------------------------------------
    Public: DclEnvVarLookup()

    Look up an environment variable by name.  The keywords RANDOM8,
    RANDOM16, and RANDOM32 are supported and will return a random
    number in the range which will fit into an 8, 16, or 32-bit
    variable, respectively.  These values are returned in shared
    static buffer which is private to the environment instance, so
    they must be used before another request for a random number
    re-uses the buffer.

    Parameters:
        hEnv    - The enviroment handle.
        pszVar  - A pointer to the null-terminated variable name
                  to look up.  Environment variable names are not
                  case-sensitive, though case is preserved.

    Return Value:
        A pointer to the environment string value, or NULL if the
        variable was not found.
-------------------------------------------------------------------*/
const char * DclEnvVarLookup(
    DCLENVHANDLE    hEnv,
    const char     *pszVar)
{
    size_t          nVarLen;
    const char     *pStr;

    DclAssert(hEnv);
    DclAssert(pszVar);
    DclAssert(*pszVar);
    DclAssert(hEnv->pszBuffer[hEnv->nBufferUsed-1] == 0);

    if(DclStrICmp(pszVar, "RANDOM8") == 0)
    {
        D_UINT8 bRand = (D_UINT8)DclRand64(NULL);
        DclSNPrintf(hEnv->szRandomBuff, sizeof(hEnv->szRandomBuff), "%u", bRand);
        return hEnv->szRandomBuff;
    }
    else if(DclStrICmp(pszVar, "RANDOM16") == 0)
    {
        D_UINT16 uRand = (D_UINT16)DclRand64(NULL);
        DclSNPrintf(hEnv->szRandomBuff, sizeof(hEnv->szRandomBuff), "%U", uRand);
        return hEnv->szRandomBuff;
    }
    else if(DclStrICmp(pszVar, "RANDOM32") == 0)
    {
        D_UINT32 ulRand = (D_UINT32)DclRand64(NULL);
        DclSNPrintf(hEnv->szRandomBuff, sizeof(hEnv->szRandomBuff), "%lU", ulRand);
        return hEnv->szRandomBuff;
    }

    nVarLen = DclStrLen(pszVar);

    pStr = hEnv->pszBuffer;

    while(*pStr)
    {
        if((DclStrNICmp(pStr, pszVar, nVarLen) == 0) && (pStr[nVarLen] == '='))
        {
            return &pStr[nVarLen+1];
        }

        pStr += DclStrLen(pStr) + 1;
    }

    return NULL;
}

