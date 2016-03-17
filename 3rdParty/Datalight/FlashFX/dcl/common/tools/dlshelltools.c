/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

    This module implements the Datalight shell commands which are generally
    classified as "tools".
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlshelltools.c $
    Revision 1.1  2010/04/18 21:19:38Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlshell.h>

#if DCLCONF_COMMAND_SHELL
#if DCLCONF_SHELL_TOOLS

#include <dltools.h>
#include <dlapireq.h>
#include <dlservice.h>
#include "dlshl.h"

static DCLSTATUS    ShellCmd_MemChain( DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    ShellCmd_MemDump(  DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    ShellCmd_MemVal(   DCLSHELLHANDLE hShell, int argc, char **argv);
static D_BOOL       ParseNumericValue(const char *pszNum, D_UINT32 *pulNum);

static const DCLSHELLCOMMAND aToolCmds[] =
{
    {DEBUGGINGCLASS, "MemChain", "Display the memory control block chain",         ShellCmd_MemChain},
    {DEBUGGINGCLASS, "MemDump",  "Display memory contents in hexadecimal form",    ShellCmd_MemDump},
    {DEBUGGINGCLASS, "MemVal",   "Send commands to the memory validation service", ShellCmd_MemVal}
};


/*-------------------------------------------------------------------
    Protected: DclShellAddToolCommands()

    Add generic tool commands to the specified shell instance.

    Parameters:
        hShell      - The handle of the shell instance to use.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclShellAddToolCommands(
    DCLSHELLHANDLE  hShell)
{
    unsigned        nn;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;

    DclAssert(hShell);

    for(nn = 0; nn < DCLDIMENSIONOF(aToolCmds); nn++)
    {
        dclStat = DclShellAddCommand(hShell, &aToolCmds[nn]);
        if(dclStat != DCLSTAT_SUCCESS)
            break;
    }

    return dclStat;
}
			

/*-------------------------------------------------------------------
    Dump Memory Chain
-------------------------------------------------------------------*/
static DCLSTATUS ShellCmd_MemChain(
    DCLSHELLHANDLE          hShell,
    int                     argc,
    char                  **argv)
{
    D_UINT32                ulLen = 0;
    DCLSTATUS               dclStat;
    DCLDECLAREREQUESTPACKET (MEMTRACK, POOLDUMP, dump);	/* DCLREQ_MEMTRACK_POOLDUMP */

    /*  Explicitly unused parameter 
    */
    ((void)hShell);

    if(argc < 2 || argc > 3 || ISHELPREQUEST())
        goto MemChainSyntax;

    if(!ParseNumericValue(argv[1], &ulLen))
        return DCLSTAT_SHELLSYNTAXERROR;

    dump.nLength = (unsigned)ulLen;

    if(argc == 3)
    {
        if(DclStrNICmp(argv[2], "/REQ:", 5) == 0)
        {
            D_UINT32    ulHandle;

            if(!ParseNumericValue(&argv[2][5], &ulHandle))
                goto MemChainSyntax;

            dclStat = DclOsRequestorDispatch(ulHandle, &dump.ior);
        }
        else
        {
            DclPrintf("Syntax error in \"%s\"\n", argv[2]);
            return DCLSTAT_FAILURE;
        }
    }
    else
    {
        dclStat = DclServiceIoctl(hShell->params.hDclInst, DCLSERVICE_MEMTRACK, &dump.ior);
    }

    if(dclStat == DCLSTAT_SERVICE_NOTREGISTERED)
        DclPrintf("Memory tracking is not enabled\n");
    else if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("The %s command failed, Status=%lX\n", argv[0], dclStat);

    return dclStat;

  MemChainSyntax:

    DclPrintf("This command displays memory control block chain, and optionally displays the\n");
    DclPrintf("first n bytes of each block.\n\n");
    DclPrintf("Syntax:  %s Length [/Req:handle]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("       Length - Is the number of bytes from each block to display, or zero to\n");
    DclPrintf("                disable dumping the memory contents.\n");
    DclPrintf("  /Req:handle - The optional Requestor Handle to which the command will be\n");
    DclPrintf("                sent.  The ReqHandle can be obtained using the 'ReqOpen'\n");
    DclPrintf("                command.  Not all environments support the requestor interface.\n\n");
    DclPrintf("If a requestor handle is not specified, the command will apply locally (useful\n");
    DclPrintf("primarily in a monolithic environment).\n\n");
    DclPrintf("The %s command only functions when DCLCONF_MEMTRACKING is enabled in\n", argv[0]);
    DclPrintf("dclconf.h.\n\n");

    return DCLSTAT_SHELLSYNTAXERROR;
}


/*-------------------------------------------------------------------
    Dump Memory
-------------------------------------------------------------------*/
static DCLSTATUS ShellCmd_MemDump(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    const char     *pParam;

    /*  Explicitly unused parameter
    */
    ((void)hShell);

    pParam = argv[1];

    if(argc == 2 && !ISHELPREQUEST())
    {
        D_UINT32        ulAddress;
        D_UINT32        ulLength;
        D_UINT16        uFlags = HEXDUMP_UINT8;
        D_UINT16        uWidth = 32;
        char            szTitle[32];

        pParam = DclNtoUL(pParam, &ulAddress);
        if(*pParam == ':')
        {
            pParam++;

            pParam = DclNtoUL(pParam, &ulLength);
            if(pParam != NULL)
            {
                if(*pParam == ',')
                {
                    pParam++;

                    if(DclToUpper(*pParam) == 'B')
                    {
                        uFlags = HEXDUMP_UINT8;
                        pParam++;
                    }
                    else if(DclToUpper(*pParam) == 'W')
                    {
                        uFlags = HEXDUMP_UINT16;
                        uWidth = 16;
                        pParam++;
                    }
                    else if(DclToUpper(*pParam) == 'D')
                    {
                        uFlags = HEXDUMP_UINT32;
                        uWidth = 8;
                        pParam++;
                    }
                    else
                    {
                        /*  Must have two commas in a row if they do not change
                            the default format, but do want to change the width.
                        */
                        if(*pParam != ',')
                            goto MemDumpSyntax;
                    }
                }

                if(*pParam == ',')
                {
                    pParam++;

                    if(!DclIsDigit(*pParam))
                        goto MemDumpSyntax;

                    uWidth = (D_UINT16)DclAtoL(pParam);

                    while(DclIsDigit(*pParam))
                        pParam++;

                    if(*pParam != 0)
                        goto MemDumpSyntax;
                }

                DclSNPrintf(szTitle, sizeof(szTitle), "Dumping memory at %lX\n", ulAddress);

                DclHexDump(szTitle, uFlags, uWidth, ulLength, (const void*)ulAddress);

                return DCLSTAT_SUCCESS;
            }
        }
    }

  MemDumpSyntax:

    DclPrintf("This command displays memory in hexadecimal format.\n\n");
    DclPrintf("Syntax:  %s Address:Length[,b|w|d][,Width]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Address - The memory address to dump\n");
    DclPrintf("  Length  - The total length to dump, in bytes\n");
    DclPrintf("  b|w|d   - Format as bytes, words, or dwords (defaults to bytes)\n");
    DclPrintf("  Width   - Elements to display on each line (defaults to 32 bytes worth)\n\n");
    DclPrintf("The address and length parameters may be specified in hex or decimal.  Hex\n");
    DclPrintf("numbers must be prefixed with '0x'.\n\n");

    return DCLSTAT_FAILURE;
}


/*-------------------------------------------------------------------
    Interface to the Memory Validation Service
-------------------------------------------------------------------*/
static DCLSTATUS ShellCmd_MemVal(
    DCLSHELLHANDLE          hShell,
    int                     argc,
    char                  **argv)
{
    DCLSTATUS               dclStat;
    int                     argnum = 1;
    D_UINT32                ulHandle = 0;
    DCLDECLAREREQUESTPACKET (MEMVAL, SUMMARY, summary); /* DCLREQ_MEMVAL_SUMMARY */

    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    if(argc > 3 || ISHELPREQUEST())
        goto MemValSyntax;

    while(argnum < argc)
    {
        if(DclStrNICmp(argv[argnum], "/RESET", 6) == 0)
        {
            summary.fReset = TRUE;
            argnum++;
            continue;
        }

        if(DclStrNICmp(argv[argnum], "/REQ:", 5) == 0)
        {
            if(!ParseNumericValue(&argv[argnum][5], &ulHandle))
                goto MemValSyntax;

            argnum++;
            continue;
        }

        DclPrintf("Syntax error in \"%s\"\n", argv[argnum]);
        return DCLSTAT_FAILURE;
    }

    if(ulHandle)
        dclStat = DclOsRequestorDispatch(ulHandle, &summary.ior);
    else
        dclStat = DclServiceIoctl(hShell->params.hDclInst, DCLSERVICE_MEMVALIDATION, &summary.ior);

    if(dclStat == DCLSTAT_SERVICE_NOTREGISTERED)
        DclPrintf("Memory validation is not enabled\n");
    else if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("The %s command failed, Status=%lX\n", argv[0], dclStat);

    return dclStat;

  MemValSyntax:

    DclPrintf("This command displays/resets summary information for the memory validation service.\n");
    DclPrintf("Syntax:  %s [/Reset /Req:handle]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("       /Reset - Reset any resettable counters to zero after retrieving the\n");
    DclPrintf("                summary information.\n");
    DclPrintf("  /Req:handle - The optional Requestor Handle to which the command will be\n");
    DclPrintf("                sent.  The ReqHandle can be obtained using the 'ReqOpen'\n");
    DclPrintf("                command.  Not all environments support the requestor interface.\n\n");
    DclPrintf("If a requestor handle is not specified, the command will apply locally (useful\n");
    DclPrintf("primarily in a monolithic environment).\n\n");
    DclPrintf("The %s command only functions when DCLCONF_MEMVALIDATION is enabled in\n", argv[0]);
    DclPrintf("dclconf.h.\n\n");

    return DCLSTAT_SHELLSYNTAXERROR;
}


/*-------------------------------------------------------------------
    Local: ParseNumericValue

    This helper function parses the specified ASCII string for
    a numeric value which may be in hex or decimal format.  Hex
    numbers must be prefixed with 0x.

    This function will display a syntax error message if the
    parse fails.

    Parameters:
        pszNum  - A pointer to the ASCII number to parse
        pulNum  - The buffer in which to store the result

    Return:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL ParseNumericValue(
    const char *pszNum,
    D_UINT32   *pulNum)
{
    const char *pStr;

    pStr = DclNtoUL(pszNum, pulNum);

    /*  If the function failed, or the number is NOT followed immediately
        by a NULL, then report the error.
    */
    if(!pStr || *pStr != 0)
    {
        DclPrintf("Syntax error, \"%s\" not recognized as a numeric value\n", pszNum);
        return FALSE;
    }

    return TRUE;
}



#endif  /* DCLCONF_SHELL_TOOLS */
#endif  /* DCLCONF_COMMAND_SHELL */

