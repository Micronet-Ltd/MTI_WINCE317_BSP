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
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlcmdln.h $
    Revision 1.6  2011/05/19 16:53:40Z  garyp
    Fixed a nested comment problem from the previous checkin.
    Revision 1.5  2011/05/18 06:12:13Z  garyp
    Added support for DCL_CMDTYPE_UINT32A argument types.
    Revision 1.4  2011/05/13 02:53:53Z  garyp
    Added the DCL_CMDTYPE_SIZEKB argument type.
    Revision 1.3  2010/08/30 00:44:49Z  garyp
    Major update to remove bugs.  Options can now be strings rather than
    single characters.  Now supports a "Boolean" argument type. Now
    supports string arguments with embedded spaces which are surrounded
    by quotes or single ticks.
    Revision 1.2  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/06/26 02:35:50Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLCMDLN_H_INCLUDED
#define DLCMDLN_H_INCLUDED


#define DCL_CMDTYPE_UINT32  1       /* Unsigned 32-bit value */
#define DCL_CMDTYPE_UINT32A 2       /* An array of unsigned 32-bit values */
#define DCL_CMDTYPE_STRING  3       /* String value */
#define DCL_CMDTYPE_BOOL    4       /* Boolean value */
#define DCL_CMDTYPE_SIZEKB  5       /* Unsigned 32-bit value, returned as a KB count  */
/* #define DCL_CMDTYPE_UINT64  6       FUTURE: Unsigned 64-bit value */
/* #define DCL_CMDTYPE_UINT64A 7       FUTURE: An array of unsigned 64-bit values */

/*  Command flags
*/
#define DCL_CMDFLAG_RANGE           0x0001
#define DCL_CMDFLAG_RANGEPOW2       0x0002
#define DCL_CMDFLAG_REQUIRED        0x0004
#define DCL_CMDFLAG_MASK_VALID      0x0007

#define DCLCMDLINEINIT(pCmd, pRes, nLo, nHi) (pCmd).pResult = (pRes);                 \
                                             (pCmd).nResultLen = DCLDIMENSIONOF(*(pRes)); \
                                             (pCmd).v.Range.ullLow = (nLo);            \
                                             (pCmd).v.Range.ullHigh = (nHi);


/*  Command-line line argument numeric range checked
*/
typedef struct
{
    D_UINT64       ullLow;
    D_UINT64       ullHigh;
} DCLCMDLINERANGE;

/*  Describes command line options and arguments
*/
typedef struct
{
    const char *pszName;    /* Option name                                  */
    unsigned    nType;      /* DCL_CMDTYPE_*                                */
    unsigned    nFlags;     /* DCL_CMDFLAGS_* values                        */
    void       *pResult;    /* A pointer to the buffer in which store the results */
    size_t      nResultLen; /* Length of pResult (only for "STRING" and "UINT32A" types)  */
    union
    {
        DCLCMDLINERANGE Range;  /* RANGE and RANGEPOW2 validation           */
    } v;                        /* Validation union                         */
    D_BOOL      fValid;     /* TRUE if the option was found and is valid    */
} DCLCMDLINE;

#define DclCmdLineProcessCmdLine    DCLFUNC(DclCmdLineProcessCmdLine)

/*  Function prototypes
*/
DCLSTATUS DclCmdLineProcessCmdLine(DCLCMDLINE *pOrgOptions, int argc, const char *argv[], const char **pBadArg);


#endif /* #ifndef DLCMDLN_H_INCLUDED */
