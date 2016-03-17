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

    This module contains command-line parsing functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlcmdln.c $
    Revision 1.9  2011/05/16 19:27:04Z  garyp
    Added support for DCL_CMDTYPE_UINT32A argument types.
    Revision 1.8  2011/05/13 02:53:52Z  garyp
    Added the DCL_CMDTYPE_SIZEKB argument type.
    Revision 1.7  2010/09/13 20:01:25Z  jimmb
    Initialized to satisfy Rvds ARM compiler
    Revision 1.6  2010/09/03 18:22:57Z  garyp
    Fixed so no arguments is not treated as a help request.  Enhanced
    string processing to work better in environments where even quoted
    arguments are broken up.  Added error messages.
    Revision 1.5  2010/08/30 14:45:04Z  garyp
    Fixed a couple character processing problems in the previous rev.
    Revision 1.4  2010/08/30 00:44:49Z  garyp
    Major update to remove bugs.  Options can now be strings rather than single
    characters.  Now supports a "Boolean" argument type.  Now supports string 
    arguments with embedded spaces which are surrounded by quotes or single
    ticks.
    Revision 1.3  2007/12/18 04:51:51Z  brandont
    Updated function headers.
    Revision 1.2  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/06/26 17:19:38Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlcmdln.h>


#if DCL_PATHSEPCHAR == '/'
  #define SWITCHCHAR_PRIMARY    '-'     
  #define SWITCHCHAR_ALTERNATE  '-'
#else
  #define SWITCHCHAR_PRIMARY    '/'
  #define SWITCHCHAR_ALTERNATE  '-'
#endif
 
#define ISWHITE(c)  (((c) == ' ') || ((c) == '\t'))

static const char * EatWhiteSpace(const char *pC);
static D_BOOL       IsSwitchChar(char c);
static DCLSTATUS    GetOption(DCLCMDLINE *paCmdLine, const char *argv, const char **ppszNextArg);
static DCLSTATUS    ValidateRange32(const DCLCMDLINE *pCmdLine, D_UINT32 ulResult, const char *pStart, const char *pEnd);


/*-------------------------------------------------------------------
    Public: DclCmdLineProcessCmdLine()

    Parse command line options.  The options are defined in an array
    of DCLCMDLINE structures.  The options string(s) may be specified
    individually in the argc/argv[] array, or as one single options
    string, with argc being set to 1 (a mix is also supported).

    All options begin with a switch character which is environment
    specific.

    The options "?" and "/?" (or "-?") are implicitly defined, and if
    found anywhere in the option strings, will result in the status
    code DCLSTAT_HELPREQUEST being returned.  Whenever this status 
    code is returned, all option processing is aborted, and none of
    the option data should be considered to be valid, even though
    some of the options may have been successfully processed.

    DCL_CMDTYPE_UINT32 Options:
        A UINT32 option has the form /OptionName:nnnn, and
        can optionally use the flags DCL_CMDFLAG_RANGE and 
        DCL_CMDFLAG_RANGEPOW2.  The resulting value is placed
        in the paCmdLine->pResult field, which *MUST* point to
        a D_UINT32 field.

        For UINT32 options with range validation, the paCmdLine->v.Range
        structure must be filled in with the range data.

    DCL_CMDTYPE_SIZEKB Options:
        A SIZEKB option has the form /OptionName:nnnn[suffix],
        and can optionally use the flags DCL_CMDFLAG_RANGE and 
        DCL_CMDFLAG_RANGEPOW2.  The resulting value is placed in the
        paCmdLine->pResult field, which *MUST* point to a D_UINT32 field.
        Suffixes of "KB", "MB", and "GB" are are supported, as are hex
        numbers prefixed with "0x".  The value specified is a count of KB
        if no suffix is specified, otherwise the result is translated
        into KB.

        For SIZEKB options with range validation, the paCmdLine->v.Range
        structure must be filled in with the range data.

    DCL_CMDTYPE_BOOL Options:
        A BOOL option has the for /OptionName.  The resulting value 
        is placed in the paCmdLine->pResult field, which *MUST* point
        to a D_BOOL field.

    DCL_CMDTYPE_STRING Options:
        A STRING option has the form /OptionName:string, /OptionName:"long
        string", or /OptionName:'long string'.

        For STRING options the paCmdLine->pResult field must specify
        a buffer of the length paCmdLine->nLen in size. If the buffer
        is not large enough for the string value, an error will be 
        returned.

    For all options, upon completion, each option's paCmdLine->fValid
    field will be set to TRUE or FALSE to indicate that an option was
    specified and its data is valid.  If this value is set to FALSE,
    none of that option's results fields will be valid.

    The optional flags values are:
        DCL_CMDFLAG_REQUIRED  - The option is required.
        DCL_CMDFLAG_RANGE     - The UINT32 or SIZEKB argument must fall
                                in a range.
        DCL_CMDFLAG_RANGEPOW2 - The UINT32 or SIZEKB argument must fall
                                in a range and be a power-of-two value.
   
    Parameters:
        paCmdLine    - A pointer to an array of DCLCMDLINE structures,
                       with the final structure containing a NULL in
                       the pszName field.
        argc         - The number of elements in argv[].
        argv         - An array of argument string pointers.
        ppszBadArg   - The location in which to store a pointer to
                       argument data which is bad.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclCmdLineProcessCmdLine(
    DCLCMDLINE     *paCmdLine,
    int             argc,
    const char     *argv[],
    const char    **ppszBadArg)
{
    DCLCMDLINE     *pCmdLine;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    char           *pAllocStart = NULL;
    const char     *pszArg;

    /*  validate parameters
    */
    DclAssert(paCmdLine);
    DclAssert(argv);

    /*  Clear the "valid" flag for all options
    */
    pCmdLine = paCmdLine;
    while(pCmdLine->pszName)
    {
        pCmdLine->fValid = FALSE;
       
        pCmdLine++;
    }

    if(!argc)
        goto Cleanup;

    /*  If there is more than one argument, build a single argument
        string.  This solves the problem of string arguments with
        white space, which, depending on the OS environment, may 
        come in as a single argument, or as multiple arguments.
    */        
    if(argc > 1)
    {
        int     iArg;
        size_t  nLen = 0;
        
        /*  Get the total space for all args
        */
        for(iArg = 0; iArg < argc; iArg++)
            nLen += DclStrLen(argv[iArg]) + 1;

        pAllocStart = DclMemAlloc(nLen);
        if(!pAllocStart)
            return DCLSTAT_OUTOFMEMORY;

        /*  Build a single argument string
        */
        *pAllocStart = 0;
        for(iArg = 0; iArg < argc; iArg++)
        {
            DclStrCat(pAllocStart, argv[iArg]);

            if(iArg < argc-1)
                DclStrCat(pAllocStart, " ");
        }

        pszArg = pAllocStart;
    }
    else
    {
        pszArg = argv[0];
    }
    
    while(*pszArg)
    {
        const char     *pszNextArg = NULL; /* initialized to satisfy Rvds ARM compiler */
        
        pszArg = EatWhiteSpace(pszArg);
        if(!*pszArg)
            break;

        if(*pszArg == '?')
        {
            dclStat = DCLSTAT_HELPREQUEST;
            break;
        }
    
        if(!IsSwitchChar(*pszArg))
        {
            dclStat = DCLSTAT_CMDLINE_BADOPTION;
            goto ErrorExit;
        }

        pszArg++;

        if(*pszArg == '?')
        {
            dclStat = DCLSTAT_HELPREQUEST;
            break;
        }

        if(!*pszArg)
        {
            pszArg--;
            dclStat = DCLSTAT_CMDLINE_BADOPTION;
            goto ErrorExit;
        }

        dclStat = GetOption(paCmdLine, pszArg, &pszNextArg);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            pszArg--;
            
          ErrorExit:
            if(ppszBadArg)
            {
                if(argc == 1)
                {
                    /*  If argc is 1, then this is the easy case
                        because we did not move the arguments into
                        a temporary buffer.
                    */    
                    *ppszBadArg = pszArg;
                }
                else
                {
                    /*  If argc is >1, we have a complex case, because
                        we've made a copy of the args, and we can't 
                        return a pointer into a buffer we're about to
                        free up.  Therefore find the correct location
                        in the argv[] array of the bad parameter.
                    */                        
                    int         ii;
                    ptrdiff_t   nRemaining = pszArg - pAllocStart;

                    for(ii = 0; ii < argc; ii++)
                    {
                        size_t nLen = DclStrLen(argv[ii]);
                        
                        if(nLen > (size_t)nRemaining)
                        {
                            *ppszBadArg = argv[ii] + nRemaining;
                            break;
                        }
                        else
                        {
                            /*  Should never get into this clause if we are
                                on the last argument...
                            */    
                            DclAssert(ii != argc - 1);
                            
                            /*  Because we added a single space between each
                                argv, and because a space itself cannot be
                                the cause of a "bad argument", nRemaining
                                at this point should always be non-zero.
                                Assert it so, and adjust to skip the space.
                            */                                
                            nRemaining -= nLen;

                            DclAssert(nRemaining);

                            nRemaining--;
                        }
                    }
                }
            }

            break;
        }

        pszArg = pszNextArg;
    }

  Cleanup:

    if(dclStat == DCLSTAT_SUCCESS)
    {
        pCmdLine = paCmdLine;
        while(pCmdLine->pszName)
        {
            /*  Ensure that all the required arguments were found
            */
            if((pCmdLine->nFlags & DCL_CMDFLAG_REQUIRED) && !pCmdLine->fValid)
            {
                DclPrintf("\nError: Required option \"%c%s\" is missing\n\n", SWITCHCHAR_PRIMARY, pCmdLine->pszName);

                if(ppszBadArg)
                    *ppszBadArg = NULL;
                 
                dclStat = DCLSTAT_CMDLINE_OPTIONMISSING;
                break;
            }
            
            pCmdLine++;
        }
    }
    
    /*  Free any allocated memory
    */
    if(pAllocStart)
        DclMemFree(pAllocStart);
    
    return dclStat;
}


/*-------------------------------------------------------------------
    Local: EatWhiteSpace()

    Increment the pointer past any white space (spaces and tabs).

    Parameter:

    Return Value:
        Returns the (possibly) updated pointer.
-------------------------------------------------------------------*/
static const char * EatWhiteSpace(
    const char *pC)
{
    while(ISWHITE(*pC))
        pC++;

    return pC;
}


/*-------------------------------------------------------------------
    Local: IsSwitchChar()

    Return TRUE if the specified character is a switch character.

    Parameters:

    Return Value:
        Return TRUE if the specified character is a switch character.
-------------------------------------------------------------------*/
static D_BOOL IsSwitchChar(
    char c)
{
    return ((c == SWITCHCHAR_PRIMARY) || (c == SWITCHCHAR_ALTERNATE));
}


/*-------------------------------------------------------------------
    Local: RangeCheck()

    Range check this value.

    Parameters:

    Return Value:
        Returns TRUE if OK or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL RangeCheck(
    D_UINT32                ulValue,
    const DCLCMDLINERANGE  *pRange)
{
    return (ulValue >= pRange->ullLow && ulValue <= pRange->ullHigh);
}


/*-------------------------------------------------------------------
    Local: FindOption()

    Find this option in the options list.

    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS FindOption(
    DCLCMDLINE     *paCmdLine,
    const char     *pszArg,
    DCLCMDLINE    **ppCmdLine)
{
    /*  validate parameters
    */
    DclAssertWritePtr(paCmdLine, 0);
    DclAssertReadPtr(pszArg, 0);

    while(paCmdLine->pszName)
    {
        size_t  nLen;
        size_t  nOptLen;
        
        nLen = DclStrLen(paCmdLine->pszName);
        nOptLen = DclStrLen(pszArg);

        if( (DclStrNICmp(pszArg, paCmdLine->pszName, nLen) == 0)
            &&
                ((nOptLen == nLen)
                    ||
                ((nOptLen > nLen) 
                && 
                    ((pszArg[nLen] == ':') || ISWHITE(pszArg[nLen])) ) ) )
        {
            if(paCmdLine->fValid)
                return DCLSTAT_CMDLINE_OPTIONDUPLICATED;

            *ppCmdLine = paCmdLine;
            return DCLSTAT_SUCCESS;
        }

        /*  try next option
        */
        paCmdLine++;
    }

    return DCLSTAT_CMDLINE_BADOPTION;
}


/*-------------------------------------------------------------------
    Local: GetOption()

    Get the options or string, etc.. from this command line argument.

    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS GetOption(
    DCLCMDLINE     *paCmdLine,
    const char     *pszArg,
    const char    **ppszNextArg)
{
    DCLCMDLINE     *pCmdLine = NULL; /* Init'd to placate a picky compiler */
    DCLSTATUS       dclStat;
    const char     *pszOriginal = pszArg;

    /*  Validate parameters
    */
    DclAssertWritePtr(paCmdLine, 0);
    DclAssertReadPtr(pszArg, 0);
    DclAssertWritePtr(ppszNextArg, sizeof(*ppszNextArg));

    /*  Find the option the corresponds to this argument
    */
    dclStat = FindOption(paCmdLine, pszArg, &pCmdLine);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    pszArg += DclStrLen(pCmdLine->pszName);

    /*  Act on that option (may include range checking, etc...)
    */
    switch(pCmdLine->nType)
    {
        case DCL_CMDTYPE_BOOL:
        {
            pCmdLine->fValid = TRUE;

            *(D_BOOL*)pCmdLine->pResult = TRUE;
 
            *ppszNextArg = pszArg;

            return DCLSTAT_SUCCESS;
        }
        
        case DCL_CMDTYPE_UINT32:
        {
            const char *pStr;
                        
            if(*pszArg != ':')
                return DCLSTAT_CMDLINE_BADSYNTAX;

            pszArg++;

            if(!*pszArg)
                return DCLSTAT_CMDLINE_BADSYNTAX;

            pStr = DclNtoUL(pszArg, (D_UINT32*)pCmdLine->pResult);
            if(!pStr || (*pStr != 0 && !ISWHITE(*pStr)))
                return DCLSTAT_CMDLINE_OPTIONMALFORMED;
 

            dclStat = ValidateRange32(pCmdLine, *(D_UINT32*)pCmdLine->pResult, pszOriginal, pStr);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;
            
            
            pCmdLine->fValid = TRUE;
 
            *ppszNextArg = pStr;

            return DCLSTAT_SUCCESS;
        }

        case DCL_CMDTYPE_UINT32A:
        {
            const char *pStr = pszArg;
            size_t      nRemaining = pCmdLine->nResultLen;
            D_UINT32   *pulResult = (D_UINT32*)pCmdLine->pResult;

            /*  The count must be initialized to a non-zero value.
            */
            if(!nRemaining)
                return DCLSTAT_CMDLINE_BADDEFINITION;

            do
            {
                if(*pStr != ':')
                    return DCLSTAT_CMDLINE_BADSYNTAX;

                pStr++;

                if(!*pStr)
                    return DCLSTAT_CMDLINE_BADSYNTAX;

                pStr = DclNtoUL(pStr, pulResult);
                if( !(pStr || !*pStr || ISWHITE(*pStr) || (*pStr == ':' && nRemaining > 1)) )
                    return DCLSTAT_CMDLINE_OPTIONMALFORMED;
     
                dclStat = ValidateRange32(pCmdLine, *pulResult, pszOriginal, pStr);
                if(dclStat != DCLSTAT_SUCCESS)
                    return dclStat;

                pulResult++;
                nRemaining--;
            }            
            while(nRemaining);
            
            pCmdLine->fValid = TRUE;
 
            *ppszNextArg = pStr;

            return DCLSTAT_SUCCESS;
        }

        case DCL_CMDTYPE_SIZEKB:
        {
            const char *pStr;
                        
            if(*pszArg != ':')
                return DCLSTAT_CMDLINE_BADSYNTAX;

            pszArg++;

            if(!*pszArg)
                return DCLSTAT_CMDLINE_BADSYNTAX;

            pStr = DclSizeToULKB(pszArg, (D_UINT32*)pCmdLine->pResult);
            if(!pStr || (*pStr != 0 && !ISWHITE(*pStr)))
                return DCLSTAT_CMDLINE_OPTIONMALFORMED;

            dclStat = ValidateRange32(pCmdLine, *(D_UINT32*)pCmdLine->pResult, pszOriginal, pStr);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;
            
            pCmdLine->fValid = TRUE;
 
            *ppszNextArg = pStr;

            return DCLSTAT_SUCCESS;
        }

        case DCL_CMDTYPE_STRING:
        {
            ptrdiff_t   nLen;
            const char *pEnd;
            char        cQuoteChar = 0;
                        
            if(*pszArg != ':')
                return DCLSTAT_CMDLINE_BADSYNTAX;

            pszArg++;

            if(!*pszArg)
                return DCLSTAT_CMDLINE_BADSYNTAX;

            if(*pszArg == '\'' || *pszArg == '"')
            {
                cQuoteChar = *pszArg;
                
                pszArg++;

                /*  Find the end of the string value, taking the quotes
                    into account.
                */                    
                pEnd = pszArg;
                while(*pEnd && (*pEnd != cQuoteChar))
                    pEnd++;

                if(!*pEnd)
                    return DCLSTAT_CMDLINE_MISMATCHEDQUOTES;

                nLen = pEnd - pszArg;

                /*  Note that when using quoted strings, is <IS> legal to
                    specify an empty string, therefore, nLen could be zero.
                */                    
            }
            else
            {
                pEnd = pszArg;
                while(*pEnd && !ISWHITE(*pEnd))
                    pEnd++;

                nLen = pEnd - pszArg;

                /*  Empty strings are not valid unless quoted.
                */
                if(!nLen)
                    return DCLSTAT_CMDLINE_STRINGEMPTY;
            }

            /*  Ensure there is room in the buffer
            */
            if(nLen >= (ptrdiff_t)pCmdLine->nResultLen)
                return DCLSTAT_CMDLINE_STRINGTOOLONG;

            DclStrNCpy(pCmdLine->pResult, pszArg, nLen);
            ((char*)pCmdLine->pResult)[nLen] = 0;

            pCmdLine->fValid = TRUE;

            /*  Adjust to point to the next argument.
            */
            pszArg += nLen;

            if(cQuoteChar)
                pszArg++;

            *ppszNextArg = pszArg;

            return DCLSTAT_SUCCESS;
        }
        
        /*  Not a recognized type
        */
        default:
            DclProductionError();
            return DCLSTAT_CMDLINE_BADOPTION;
    }
}


/*-------------------------------------------------------------------
    Local: ValidateRange32()

    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS ValidateRange32(
    const DCLCMDLINE   *pCmdLine,
    D_UINT32            ulResult,
    const char         *pStart,
    const char         *pEnd)
{
    if((pCmdLine->nFlags & DCL_CMDFLAG_RANGEPOW2) &&
        (!RangeCheck(ulResult, &pCmdLine->v.Range) || !DCLISPOWEROF2(ulResult)))
    {
        ptrdiff_t   iLen = (pEnd - pStart) + 1;
        char *      pTemp = DclMemAlloc(iLen);

        if(pTemp)
        {
            DclStrNCpy(pTemp, pStart, iLen);
            pTemp[iLen-1] = 0;
        }

        /*  In the unlikely event that pTemp failed to allocate, don't
            worry about it in  this printf call since it internally
            will safely accommodate NULL pointers.
        */                    
        DclPrintf("\nError: \"%c%s\" is not a power-of-two value in the range of %llU to %llU\n\n",
            SWITCHCHAR_PRIMARY, pTemp, 
            VA64BUG(pCmdLine->v.Range.ullLow), 
            VA64BUG(pCmdLine->v.Range.ullHigh));

        if(pTemp)
            DclMemFree(pTemp);
        
        return DCLSTAT_CMDLINE_OUTOFRANGE;
    }
    
    if((pCmdLine->nFlags & DCL_CMDFLAG_RANGE) && !RangeCheck(ulResult, &pCmdLine->v.Range))
    {
        ptrdiff_t   iLen = (pEnd - pStart) + 1;
        char *      pTemp = DclMemAlloc(iLen);

        if(pTemp)
        {
            DclStrNCpy(pTemp, pStart, iLen);
            pTemp[iLen-1] = 0;
        }
        
        /*  In the unlikely event that pTemp failed to allocate, don't
            worry about it in  this printf call since it internally
            will safely accommodate NULL pointers.
        */                    
        DclPrintf("\nError: \"%c%s\" is not in the range of %llU to %llU\n\n",
            SWITCHCHAR_PRIMARY, pTemp, 
            VA64BUG(pCmdLine->v.Range.ullLow), 
            VA64BUG(pCmdLine->v.Range.ullHigh));
        
        if(pTemp)
            DclMemFree(pTemp);
        
        return DCLSTAT_CMDLINE_OUTOFRANGE;
    }

    return DCLSTAT_SUCCESS;
}            

