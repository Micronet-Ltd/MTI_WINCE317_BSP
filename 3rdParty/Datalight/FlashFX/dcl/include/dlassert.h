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

    This header file contains macros which implement both DEBUG and RELEASE
    mode (production) asserts.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlassert.h $
    Revision 1.10  2010/11/23 18:26:17Z  garyp
    Made assert mode processing more flexible.
    Revision 1.9  2010/04/28 23:31:30Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.8  2009/11/06 07:09:23Z  keithg
    Added explicit cast of assert function calls to avoid inconsistent types
    use in the trinary operator.
    Revision 1.7  2009/08/27 20:42:27Z  billr
    Module name is a string literal (char, not D_BUFFER).
    Revision 1.6  2009/06/27 19:49:08Z  garyp
    Updated so asserts are implemented as a service.  Add the assert macros
    DclAssertRead/WritePtr().
    Revision 1.5  2009/01/22 21:49:30Z  billr
    Update copyright date.
    Revision 1.4  2009/01/12 20:38:15Z  billr
    DclAssert() and DclError() should always result in a valid, non-empty
    statement (which does nothing in a release build). This allows turning
    on compiler warnings that can catch things like "if (condition);"
    (note the spurious semicolon).
    Revision 1.3  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.2  2005/12/21 07:33:16Z  garyp
    Removed some dead code.
    Revision 1.1  2005/10/03 12:42:16Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLASSERT_H_INCLUDED
#define DLASSERT_H_INCLUDED


/*-------------------------------------------------------------------
    Default Memory Validation Service Settings
-------------------------------------------------------------------*/
#ifndef DCLCONF_MEMORYVALIDATION
#define DCLCONF_MEMORYVALIDATION        (FALSE)
#endif


typedef enum
{
    DCLASSERTMODE_LOWLIMIT = 0,
    DCLASSERTMODE_INVALID = 0,          /* Used only to provide a failure condition */
    DCLASSERTMODE_AUTODEBUGGERFAIL,     /* If possible, query to enter the debugger, but timeout after 10 seconds */
    DCLASSERTMODE_QUERYDEBUGGERFAIL,    /* If possible, query to enter the debugger, with no timeout */
    DCLASSERTMODE_DEBUGGERFAIL,         /* If possible, pop into the debugger */
    DCLASSERTMODE_FAIL,                 /* Fail (permanently put the thread to sleep) */
    DCLASSERTMODE_WARN,                 /* Provide a warning message only */
    DCLASSERTMODE_IGNORE,               /* Ignore the assert */
    DCLASSERTMODE_HIGHLIMIT,            /* High limit for legal states */
    DCLASSERTMODE_QUERY = -1            /* Used to query existing state, not a valid legal state to set */
} DCLASSERTMODE;

#define         DclAssertServiceInit    DCLFUNC(DclAssertServiceInit)
#define         DclAssertFired          DCLFUNC(DclAssertFired)
#define         DclAssertMode           DCLFUNC(DclAssertMode)

DCLSTATUS       DclAssertServiceInit(DCLINSTANCEHANDLE hDclInst);
DCLSTATUS       DclAssertFired(DCLINSTANCEHANDLE hDclInst, const char *pszModuleName, unsigned nLineNumber);
DCLASSERTMODE   DclAssertMode(DCLINSTANCEHANDLE hDclInst, DCLASSERTMODE nNewMode);


/*  DclProductionAssert() and DclProductionError() always exist
*/
#define DclProductionAssert(x)  ((x) ? ((void)0) : (void)DclAssertFired(NULL, __FILE__, __LINE__))
#define DclProductionError()                       (void)DclAssertFired(NULL, __FILE__, __LINE__)

#if D_DEBUG >= 1
  #define DclAssert(x)          ((x) ? ((void)0) : (void)DclAssertFired(NULL, __FILE__, __LINE__))
  #define DclError()                               (void)DclAssertFired(NULL, __FILE__, __LINE__)

  #if DCLCONF_MEMORYVALIDATION
    #define DclAssertReadPtr(p, l)  DclAssert(DclMemValBufferValidate(NULL, (p), (l), DCL_MVFLAG_READONLY) == DCLSTAT_SUCCESS)
    #define DclAssertWritePtr(p, l) DclAssert(DclMemValBufferValidate(NULL, (p), (l), 0) == DCLSTAT_SUCCESS)

  #else

    #define DclAssertReadPtr(p, l)  DclAssert(p)
    #define DclAssertWritePtr(p, l) DclAssert(p)
  #endif

#else

  #define DclAssert(x)              do { (void) 0; } while (0)
  #define DclError()                do { (void) 0; } while (0)
  #define DclAssertReadPtr(p, l)    do { (void) 0; } while (0)
  #define DclAssertWritePtr(p, l)   do { (void) 0; } while (0)
#endif


typedef struct
{
    DCLIOREQUEST    ior;
    const char     *pszModuleName;
    unsigned        nLineNumber;
} DCLREQ_ASSERT_FIRED;

typedef struct
{
    DCLIOREQUEST    ior;
    DCLASSERTMODE   nNewMode;
    DCLASSERTMODE   nOldMode;
} DCLREQ_ASSERT_MODE;


#endif /* DLASSERT_H_INCLUDED */

