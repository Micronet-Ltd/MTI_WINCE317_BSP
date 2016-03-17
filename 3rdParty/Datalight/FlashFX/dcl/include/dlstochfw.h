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
    The Monkey engine definitions and prototypes.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlstochfw.h $
    Revision 1.4  2010/01/08 19:11:14Z  brandont
    Added the szLogFile member to the TagRelStochParameters_s structure.
    Revision 1.3  2009/09/02 20:45:13Z  johnbr
    Fix the definition of the type RelStochGlobals_t so that it functions
    correctly as an opaque type.
    Revision 1.2  2009/08/20 21:33:31Z  johnbr
    Changes for the power cycling tests.
    Revision 1.1  2009/01/22 00:07:58Z  johnbr
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLSTOCHFW_H_INCLUDED
#define DLSTOCHFW_H_INCLUDED

/* The logging enumeration.  This corresponds to the -f parameter in
   the command line-based tools.  These evaluate as follows:

   DLSTOCH_STDOUT - logs to standard out.  Typically, this runs to
                    the command line window and is moderately fast.
   DLSTOCH_FTPOUT - logs using the DAF FTP file system.  This allows
                    the Stoch tool to reliably log to a server
                    independant of the current run.  It is very
                    slow.
   DCSTOCH_NONE   - no logging is done in this mode.  You can't
                    "log" faster than this.
*/
enum DclStochLogMode
{
    DLSTOCH_STDOUT   = 1,
    DLSTOCH_FTPOUT   = 2,
    DLSTOCH_NONE     = 3,

    DLSTOCH_LAST
};

/* The token exchange enumeration.  This corresponds to the -t parameter in
   the command line-based tols.
*/
enum DclStochTokenExchange
{
    DLSTOCH_NOEXCHANGE = 0,
    DLSTOCH_TOKENSENABLED = 1
};

/* The type for the Monkey testcase globals.  The actual type for this
   is included by the main() function (to setup the globals for use)
   and by the test cases (for actual use).

   The Monkey framework accepts the pointer and passes it to the test
   cases.  It knows NOTHING about the contents of the structure and
   cannot examine or modify the contents.
*/
typedef struct TagRelStochGlobals_s  RelStochGlobals_t;

/* The validator function.  This is called every few iterations or
   at a specified iteration, according to the parameters on the
   command line.  The Core Monkey uses /v and /h for these respectively.
 */
typedef eDCLTESTFWSTATUS (*RelStochCoreValidator_t)(
        RelStochGlobals_t  * pGlobals,
        unsigned long        ulIteration);

/* The Monkey engine needs a number of parameters that make calling
   engine unweildly.  This structure contains the actual parameters
   used to describe a run.  All of them should have defaults set in
   the main() function, with the actual values entered on the
   command line.
 */
typedef struct TagRelStochParameters_s {
    D_UINT32                    ulCheckerSingle;
    D_UINT32                    ulCheckerInterval;
    D_BOOL                      fStopOnCheckError;
    D_UINT32                    ulRandomSeed;
    D_UINT32                    nIterations;
    RelStochCoreValidator_t     pValidator;
    enum DclStochTokenExchange  eTokenExchangeEnabled;
    enum DclStochLogMode        eLoggingMode;
    char                        szSystemName[32];
    char                        szUsername[32];
    char                        szPassword[32];
    char                        szLogFile[255];
} RelStochParameters_t;

struct TagRelStochCoreTestCase_s;

/* The description of a testcase.  It is possible that the pfTestcase
   parameter could be removed.  The function result is DCL_TESTFW_PASSED if
   the test case passed.
 */
typedef eDCLTESTFWSTATUS (*RelStochTestCasePtr)(
        RelStochGlobals_t             * pGlobals,
        struct TagRelStochCoreTestCase_s  * pTestCase,
        unsigned long                       ulIteration);


/* The pTestFunction refers to the test case function.
   pszShortName is used to find the test cases in the log file
   pszDescrtiption is a long description used for verbose logging.
   TODO:  Add verbose logging.
 */
typedef struct TagRelStochCoreTestCase_s {
    RelStochTestCasePtr   pfTestFunction;
    char                    * szDescription;
    unsigned long             ulRelativeFreq;
} RelStochCoreTestCase_t;

/* The results statistics.  In the ideal run, there are no failures and no
   skips.
 */
typedef struct TagRelStochCoreStatistics_s {
    unsigned long   ulTestsRun;
    unsigned long   ulTestsPassed;
    unsigned long   ulTestsFailed;
    unsigned long   ulTestsSkipped;
} RelStochCoreStatistics_t;


/* Monkey engine prototypes
 */
void DclStochLogLine(
    char * szFormat, ...);
unsigned long DclStochRand(void);
void DclStochExecute(
    const char * szTitle,
    RelStochCoreTestCase_t     * pTestCases,
    RelStochGlobals_t      * pGlobals,
    RelStochParameters_t * psParameters);

/* FTP Logging and Token passing functions
 */
int DclStochUseFtpLogging (
    RelStochParameters_t * psParameters,
    char                 * szFileName,
    char                 * szRename,
    char                 * szIpAddress,
    char                 * szUserName,
    char                 * szPassword);
int DclStochSendSimpleToken(
    char * szTokenDir,
    char * szSystemType,
    char * szTokenName );
int DclStochFetchSimpleToken(
    char * szTokenDir,
    char * szSystemType,
    char * szTokenName );
int DclStochDeleteSimpleToken(
    char * szTokenDir,
    char * szSystemType,
    char * szTokenName );
int DclStochReadSimpleToken(
    char     * szTokenDir,
    char     * szSystemName,
    char     * szTokenName,
    D_UINT32 * pulInteger);

#endif /* #ifndef DLSTOCHFW_H_INCLUDED */
