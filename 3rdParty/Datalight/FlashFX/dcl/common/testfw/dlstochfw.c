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
    Stochastic test framework.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlstochfw.c $
    Revision 1.7  2010/04/28 23:31:28Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.6  2009/10/07 01:01:22Z  garyp
    Documentation updated.  Restored some missing revision history.
    Revision 1.5  2009/09/02 15:03:22Z  johnbr
    Corrected a coding standard error in the function header comments.
    Revision 1.4  2009/07/30 20:37:38Z  johnbr
    Changes for the power cycling tests.
    Revision 1.3  2009/06/25 21:55:44Z  garyp
    Updated to use the new DclSignOn() calling convention.
    Revision 1.2  2009/01/28 16:21:52Z  keithg
    Now builds cleanly with DCLCONF_OUTPUT_ENABLED disabled.
    Revision 1.1  2009/01/22 13:16:14  johnbr
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiint.h>
#include <dlprintf.h>
#include <dltestfw.h>
#include <dlstochfw.h>

/*
   Stochastic Tool Globals
 */
static DCLFTPFSFD            ghFtpFileHandle;
static DCLFTPFSINSTANCE      gpsInstance;
static enum DclStochLogMode  geLoggingMode;

/*-------------------------------------------------------------------
    Protected: DclStochUseFtpLogging()

    Tell the Stoch tool to use the DAF module FTP file system as a
    logging destination.  This will open a file and pass the file
    handle into the pParameters structure for use when DclStochExecute()
    is initiated.  This should be called before DclStochExecute() for
    obvious reasons.

    Note:  The FTP server is opened even if the user specifies 'No Logging'
    We do this so that the automated tests can get to the FTP server for
    token exchange if needed.

    Parameters:
        psParameters - The parameter structure for DclStochExecute().
        szFileName   - The name to write the log file to.
        szIpAddress  - The IP address for the FTP server.
        szUserName   - The username on the FTP server. May be
                       "anonymous" if the server supports it.
        szPassword   - The password for szUserName on the FTP
                       server.  If the username "anonymous",
                       then this should be your email address.

    Return Value:
        0 if there were no errors or any other value indicating
        that a problem occured and logging will be done to
        stdout.  If no options that require an FTP connection
        are set, then no errors are possible and we'll return
        0, with the instance and ftpFile variables set to NULL.
-------------------------------------------------------------------*/
int DclStochUseFtpLogging (
    RelStochParameters_t       * psParameters,
    char                       * szFileName,
    char                       * szRename,
    char                       * szIpAddress,
    char                       * szUserName,
    char                       * szPassword)
{
    DCLFTPFSINSTANCE  psInstance;
    DCLFTPFSFD        psFileHandle;
    int               iFTPErrorCount;

    /* First, save the logging mode, and clear the FTP instance variables
    */
    geLoggingMode   = psParameters->eLoggingMode;
    gpsInstance     = NULL;
    ghFtpFileHandle       = NULL;

    if ( psParameters->eTokenExchangeEnabled == DLSTOCH_TOKENSENABLED ||
         psParameters->eLoggingMode == DLSTOCH_FTPOUT )
    {
        /* Now, create an FTP Instance.  Return if error exists after ten
           retries.
        */
        iFTPErrorCount = 0;
        do
        {
            psInstance = DclFtpFsInstanceCreate(
                    szIpAddress,
                    szUserName,
                    szPassword );
            if ( psInstance == NULL)
            {
                iFTPErrorCount ++;
                DclOsSleep( 1000 );
            }
        } while (psInstance == NULL && iFTPErrorCount <= 10);
        if (psInstance == NULL)
        {
            DclPrintf( "No FTP Connection possible, exiting\n" );
            return -2;
        }

        if (psParameters->eLoggingMode == DLSTOCH_FTPOUT)
        {
            /* If the old log exists, then rename it.  If it doesn't
               exist, then no harm done.
            */
            DclFtpFsRename( psInstance, szFileName, szRename );

            /* Open the specified file.  Return on error.
            */
            psFileHandle = DclFtpFsCreate(
                    psInstance,
                    szFileName );
            if (psFileHandle == NULL)
            {
                DclPrintf( "Log file create failed\n");
                return -3;   /* General error until a better one exists */
            }

            ghFtpFileHandle = psFileHandle;
        }

        gpsInstance = psInstance;
    }
    return 0;
}


/*-------------------------------------------------------------------
    Protected: DclStochSendSimpleToken()

    Send a simple token to the FTP server.  This token consists of a
    zero-length file with the filename of the form:

        <DCL_OSNAME>_<string supplied>.tok

    The FTP master software looks for these tokens as a means of
    determining whether the machine is alive, and capable of
    running the Monkey.  Currently, the master strips the section
    to the left of the first '_' and uses it to refer to the sending
    machine.

    The <string> part should be one of the following:
    ID           - tells the master that we're alive and can be told to
                   run tests.  This token will be renamed to *.sav
                   then the master program has parsed it.
    Monkey       - received by this program, and thus isn't usable.
                   tells this program to run the Monkey for a LONG
                   time.
    LogDone      - tells the master that it can parse the log output
                   for errors.  This will be deleted by the master
                   when the parse operation is complete.

    Parameters:
        szTokenDir  - The directory to drop the token in.  Currently,
                      everything is dropped in SQA_Logs on 10.0.10.10,
                      but this (and this comment) will change.
        szSystemName- The name of system we're running.  This corresponds
                      to the name field in the Master parameter file on
                      the controller machine.  This must match exactly
                      to that field and the match is case sensitive
        szTokenName - The name of the token to send.  This cannot be
                      "Monkey", as that is reserved to the Master program.

    Return Value:
        0 on a valid exit with the token thought to be sent, or
        any other value to indicate an error.
-------------------------------------------------------------------*/
int DclStochSendSimpleToken(
    char * szTokenDir,
    char * szSystemType,
    char * szTokenName )
{
    char         szToken[64];
    DCLFTPFSFD   psFileHandle;
    int          iStatus = 1;

    DclStrCpy( szToken, szTokenDir );
    DclStrCat( szToken, szSystemType );
    DclStrCat( szToken, "_" );
    DclStrCat( szToken, szTokenName );
    DclStrCat( szToken, ".tok" );

    DclPrintf( "Dropping token %s\n", szToken );

    psFileHandle = DclFtpFsCreate( gpsInstance, szToken );
    if ( psFileHandle != NULL )
    {
        iStatus = DclFtpFsClose( psFileHandle );
    }

    return iStatus;
}


/*-------------------------------------------------------------------
    Protected: DclStochFetchSimpleToken()

    Fetch a simple token from the FTP server.  This token consists of a
    zero-length file with the filename of the form:

        <DCL_OSNAME>_<string supplied>.tok

    The FTP master software uses these tokens as a means of
    determining whether the machine is alive, and capable of
    running the Monkey. This function looks for a token with
    a given name/DCL_OSNAME and returns TRUE iff the token
    exists, or FALSE if the token cannot be found.

    Parameters:
        szTokenDir  - The directory to drop the token in.  Currently,
                      everything is dropped in SQA_Logs on 10.0.10.10,
                      but this (and this comment) will change.
        szSystemName- The name of system we're running.  This corresponds
                      to the name field in the Master parameter file on
                      the controller machine.  This must match exactly
                      to that field and the match is case sensitive
        szTokenName - The name of the token to send.  This cannot be
                      Monkey, as that is reserved to the Master program.

    Return Value:
        0 on a valid exit with the token thought to be sent, or
        any other value to indicate an error.
-------------------------------------------------------------------*/
int DclStochFetchSimpleToken(
    char * szTokenDir,
    char * szSystemType,
    char * szTokenName )
{
    char         szToken[64];
    DCLFSSTAT    sStatBlock;
    int          iStatus = 1;

    DclStrCpy( szToken, szTokenDir );
    DclStrCat( szToken, szSystemType );
    DclStrCat( szToken, "_" );
    DclStrCat( szToken, szTokenName );
    DclStrCat( szToken, ".tok" );

    iStatus = DclFtpFsStat( gpsInstance, szToken, &sStatBlock );

    return iStatus == 0;
}


/*-------------------------------------------------------------------
    Protected: DclStochDeleteSimpleToken()

    Delete a simple token from the FTP server.  This token consists of a
    zero-length file with the filename of the form:

        <DCL_OSNAME>_<string supplied>.tok

    The FTP master software uses these tokens as a means of
    determining whether the machine is alive, and capable of
    running the Monkey. This function looks for a token with
    a given name/DCL_OSNAME and returns TRUE iff the token
    exists, or FALSE if the token cannot be found.

    Parameters:
        szTokenDir  - The directory to drop the token in.  Currently,
                      everything is dropped in SQA_Logs on 10.0.10.10,
                      but this (and this comment) will change.
        szSystemName- The name of system we're running.  This corresponds
                      to the name field in the Master parameter file on
                      the controller machine.  This must match exactly
                      to that field and the match is case sensitive
        szTokenName - The name of the token to send.  This cannot be
                      Monkey, as that is reserved to the Master program.

    Return Value:
        0 on a valid exit with the token thought to be deleted, or
        any other value to indicate an error.
-------------------------------------------------------------------*/
int DclStochDeleteSimpleToken(
    char * szTokenDir,
    char * szSystemType,
    char * szTokenName )
{
    char         szToken[64];
    int          iStatus = 1;

    DclStrCpy( szToken, szTokenDir );
    DclStrCat( szToken, szSystemType );
    DclStrCat( szToken, "_" );
    DclStrCat( szToken, szTokenName );
    DclStrCat( szToken, ".tok" );

    iStatus = DclFtpFsDelete( gpsInstance, szToken );

    return iStatus == 0;
}


/*-------------------------------------------------------------------
    Protected: DclStochReadSimpleToken()

    Read a simple token from the FTP server.  This token consists of a
    short file with the filename of the form:

        <DCL_OSNAME>_<string supplied>.tok

    The file contains a single 32-bit integer which will be read in
    if the file exists.  If the file doesn't exist, then the integer
    will not be read or written to.

    The FTP master software uses these tokens as a means of
    determining whether the machine is alive, and capable of
    running the Monkey. This function looks for a token with
    a given name/DCL_OSNAME and returns TRUE iff the token
    exists, or FALSE if the token cannot be found.

    Parameters:
        szTokenDir  - The directory to drop the token in.  Currently,
                      everything is dropped in SQA_Logs on 10.0.10.10,
                      but this (and this comment) will change.
        szSystemName- The name of system we're running.  This corresponds
                      to the name field in the Master parameter file on
                      the controller machine.  This must match exactly
                      to that field and the match is case sensitive
        szTokenName - The name of the token to send.  This cannot be
                      Monkey, as that is reserved to the Master program.
        pulInteger  - A pointer to the 32-bit object which will hold the
                      value in the file if the file exists.

    Return Value:
        0 on a valid exit with the token thought to be sent, or
        any other value to indicate an error.
-------------------------------------------------------------------*/
int DclStochReadSimpleToken(
    char     * szTokenDir,
    char     * szSystemName,
    char     * szTokenName,
    D_UINT32 * pulInteger)
{
    char         szToken[64];
    char         asBuffer[64];
    DCLFSSTAT    sStatBlock;
    int          iStatus = 1;  /* compiler warning initialization */
    DCLFTPFSFD   file;
    D_UINT32     ulRead;

    DclStrCpy( szToken, szTokenDir );
    DclStrCat( szToken, szSystemName );
    DclStrCat( szToken, "_" );
    DclStrCat( szToken, szTokenName );
    DclStrCat( szToken, ".tok" );

    iStatus = DclFtpFsStat( gpsInstance, szToken, &sStatBlock );
    if (iStatus != 0)
        return FALSE;
    if (pulInteger == NULL )   /* if the pointer is bogus, return now */
        return FALSE;

    /* We have a file, read the first line and convert it to an
       integer.
    */
    file = DclFtpFsOpen( gpsInstance, szToken );
    if (!file)
        return FALSE;

    iStatus = DclFtpFsRead( file,
                            asBuffer,
                            sizeof(asBuffer),
                            &ulRead );

    * pulInteger = (D_UINT32)DclAtoL( asBuffer );
    return TRUE;
}


/*-------------------------------------------------------------------
    Protected: DclStochLogLine()

    A standard way to log a line of text. This is a wrapper function
    that may get more complex in the future, or it could be replaced
    by DclPrintf.

    Parameters:
        szFormat - Format for the display data.
        ...      - Arguments for the format string.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclStochLogLine(
    char * szFormat,
    ...)
{
#if DCLCONF_OUTPUT_ENABLED
    va_list    arg_ptr;
    char       szLogBuffer[256];
    D_UINT32   ulTransfered;

    /* If we're not logging, then exit immediately.
    */
    if ( geLoggingMode == DLSTOCH_NONE )
        return;

    /* If we're using stdout logging, then send the data and be done.
       with it.
    */
    va_start(arg_ptr, szFormat) ;

    if ( geLoggingMode == DLSTOCH_STDOUT)
    {
        DclVPrintf(szFormat, arg_ptr);
        return;
    }

    if ( geLoggingMode == DLSTOCH_FTPOUT)
    {
        /* We're logging to the FTP Server.  Format the data and send as needed.
        */
        DclVSNPrintf(
                szLogBuffer,
                sizeof(szLogBuffer)-1,
                szFormat,
                arg_ptr );
        DclFtpFsWrite(
                ghFtpFileHandle,
                szLogBuffer,
                (D_UINT32)DclStrLen(szLogBuffer),
                & ulTransfered);
    }
#else

    (void) szFormat;

#endif
    return;
}


/*-------------------------------------------------------------------
    Protected: DclStochRand()

    A standard way to generate a random number.

    Parameters:
        None.

    Return Value:
        A pseudo-random number in the form of an unsigned long.  This
        is generated by DclRand(), using the seed, below.
-------------------------------------------------------------------*/

/* The universal random number seed for the Stoch tool.  It is set
   by DclStochExecute() directly (from the psParameters values) and
   accessed after that point by the DclStochRand() function.
*/
static D_UINT32 gulDclStochRandomSeed;

unsigned long DclStochRand(void)
{
    return (unsigned long)DclRand( &gulDclStochRandomSeed );
}


/*-------------------------------------------------------------------
    Protected: DclStochExecute()

    By convention the log file for the Monkey will be output to STDOUT
    on whatever system the Monkey is running on.  The log will include
    two to three lines per case.  The first is of the form:

    \t<testNumber>\t<time>\t<test related info>

    The testNumber and time fields are output before the test is entered.
    The <test related info> area is intended to be used by the testCase
    to output the things it will attempt to do.  For instance, the file
    Monkey will display the file/directory information and what the
    testCase is intending to do with the item.

    The last line will be of the form:

    <pass|FAIL|Skip>\t<Test related information>

    Other lines will contain general information from the test case.

    Parameters:
        szTitle      - A user specified title for use in logging.
        testCases    - A non-null pointer to an array of type
                       RelStochCoreTestCase_t. The last element of the array must
                       have a NULL test pointer value. The frequency value
                       gives the relative frequency of a test call.  A zero
                       frequency means that the entry will not be called.
        pGlobals     - A pointer to a global structure that will be passed
                       to all testCases and call-backs.  This should be
                       allocated and coherent when the Monkey is entered.
                       It may be NULL, and the NULL will be passed in
                       blindly.
        psParameters - The parameter structure for this instance of the
                       Monkey.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclStochExecute(
    const char             * szTitle,
    RelStochCoreTestCase_t * pTestCases,
    RelStochGlobals_t      * pGlobals,
    RelStochParameters_t   * psParameters)
{
    unsigned long      i;
    unsigned long      j;
    unsigned long      ulRandom;
    unsigned long      ulCount;
    unsigned long      ulTotalFreq;
    unsigned long      ulFreqSum;
    unsigned long      ulCurrentCaseIndex = 0;
    unsigned long      nIterations;
    unsigned long      ulCheckerInterval;
    unsigned long      ulCheckerSingle;
    D_BOOL             fStopOnCheckError;
    eDCLTESTFWSTATUS   eTestResult;
    RelStochCoreStatistics_t   sRunStats = {0, 0, 0, 0};
/*    RelStochCoreValidator_t    pValidator; */
    eDCLTESTFWSTATUS   eValidatorResult;

    /*
     * First, verify the parameters
     */
    if(pTestCases == NULL)
    {
        DclStochLogLine("No test case list supplied\n");
        return;
    }

    /* Pull out the parameters into local variables, note that gulDclStochRandomSeed
       is a global variable, defined at the top of this module. It should
       be renamed to a Monkey-specific name for use by DclRand in the test cases.
    */
    nIterations          = psParameters->nIterations;
    ulCheckerInterval    = psParameters->ulCheckerInterval;
    ulCheckerSingle      = psParameters->ulCheckerSingle;
    gulDclStochRandomSeed= psParameters->ulRandomSeed;
/*    pValidator           = psParameters->pValidator; */
    fStopOnCheckError    = psParameters->fStopOnCheckError;

    DclStochLogLine("%s\n", szTitle);
    DclSignOn(FALSE);
    DclStochLogLine("Iterations               %ld\n",  nIterations);
    DclStochLogLine("Checker Interval         %ld\n",  ulCheckerInterval);
    DclStochLogLine("Checker Singleton        %ld\n",  ulCheckerSingle);
    DclStochLogLine("Random Number Seed       %ld\n",  gulDclStochRandomSeed);
    DclStochLogLine("Stop on Validator Error  %s\n\n", fStopOnCheckError ? "TRUE" : "FALSE");

    /*
     * Second, count the test cases and the frequency values.  These
     * will be used to determine the which test case to run on any
     * given iteration.
     */
    i = 0;
    ulCount = 0;
    ulTotalFreq = 0;

    while(pTestCases[i].pfTestFunction != NULL)
    {
        ulCount ++;
        ulTotalFreq += pTestCases[i].ulRelativeFreq;
        i++;
    }

    if(ulCount == 0)
    {
        DclStochLogLine("No available tests in the test list\n");
        return;
    }
    if(ulTotalFreq == 0)
    {
        DclStochLogLine("No tests will run with this list\n");
        return;
    }

    /*
     * Start the call loop.
     */
    for(i=1; i <= nIterations; ++i)
    {
        ulRandom = DclStochRand() % ulTotalFreq;
        ulFreqSum = 0;
        for(j=0; j<ulCount; ++j)
        {
            ulCurrentCaseIndex = j;
            ulFreqSum += pTestCases[j].ulRelativeFreq;
            if(ulFreqSum > ulRandom)
                break;
        }

        eTestResult = pTestCases[ulCurrentCaseIndex].pfTestFunction(
                        pGlobals,
                        &pTestCases[ulCurrentCaseIndex],
                        i);

        /*
         * Build the test statistics block.
         */
        sRunStats.ulTestsRun ++;
        if(eTestResult == DCL_TESTFW_PASSED)
        {
            sRunStats.ulTestsPassed ++;
        }
        else
        {
            if(eTestResult == DCL_TESTFW_FAILED){
                sRunStats.ulTestsFailed ++;
            }
            else
            {
                sRunStats.ulTestsSkipped ++;
            }
        }

        /* Try the Validator.  This currently runs every N iterations.
        */
        if(((i % ulCheckerInterval) == 0) || (i == ulCheckerSingle))
        {
            eValidatorResult = DCL_TESTFW_PASSED;
            if(psParameters->pValidator != NULL)
                eValidatorResult = (psParameters->pValidator)(pGlobals, i);
            if(eValidatorResult != DCL_TESTFW_PASSED && fStopOnCheckError)
                break;
        }
    }

    DclStochLogLine("\n\nRun Statistics\n\n");
    DclStochLogLine("Tests Passed         %ld\n", sRunStats.ulTestsPassed);
    DclStochLogLine("Tests Failed         %ld\n", sRunStats.ulTestsFailed);
    DclStochLogLine("Tests Skipped        %ld\n", sRunStats.ulTestsSkipped);
    DclStochLogLine("Tests Run            %ld\n", sRunStats.ulTestsRun);
    DclStochLogLine("\n\n");
    DclStochLogLine("Testing Complete\n");
    return;
}
