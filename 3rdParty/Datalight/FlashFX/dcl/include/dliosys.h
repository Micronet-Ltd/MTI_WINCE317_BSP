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
  jurisdictions. 

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

  Notwithstanding the foregoing, Licensee acknowledges that the software may
  be distributed as part of a package containing, and/or in conjunction with
  other source code files, licensed under so-called "open source" software 
  licenses.  If so, the following license will apply in lieu of the terms set
  forth above:

  Redistribution and use of this software in source and binary forms, with or
  without modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions, and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions, and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY DATALIGHT "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
  CHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE
  DISCLAIMED.  IN NO EVENT SHALL DATALIGHT BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEG-
  LIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header defines the types and symbols used for the DCL I/O requestor
    interface.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dliosys.h $
    Revision 1.22  2011/04/21 02:26:43Z  garyp
    Added support for the FileSpy functionality.
    Revision 1.21  2011/01/03 22:08:57Z  billr
    Update copyright date.
    Revision 1.20  2011/01/03 22:06:48Z  billr
    Fix warnings having to do with mixing enumerated types.
    Revision 1.19  2010/10/26 22:13:31Z  garyp
    Renamed DclLogWrite() to DclLogWriteString() to avoid confusion with
    the new logging capability of handling binary logs.
    Revision 1.18  2010/07/15 01:15:03Z  garyp
    Added the DCLREQ_VERSIONCHECK structure which is now shared
    by multiple products.
    Revision 1.17  2010/07/15 01:14:06Z  garyp
    Added functionality to reset the profiler data.
    Revision 1.16  2010/04/18 21:14:27Z  garyp
    Did some refactoring of the DCL system info functions to make them
    more useful.
    Revision 1.15  2010/02/13 20:37:40Z  garyp
    Updated to use the correct license wording.
    Revision 1.14  2010/02/10 23:38:54Z  billr
    Changed the headers to reflect the shared or public license. [jimmb]
    Revision 1.13  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.12  2009/11/16 02:10:08Z  garyp
    Added support for DCLIOFUNC_PROFILER_MEMPOOLSTATE.
    Revision 1.11  2009/11/08 16:21:06Z  garyp
    Updated to support the profiler toggle command.
    Revision 1.10  2009/10/18 00:41:44Z  garyp
    Added support for DCLIOFUNC_TRACE_TOGGLE.
    Revision 1.9  2009/09/03 17:31:16Z  garyp
    Added DCLIOSTATUS.
    Revision 1.8  2009/06/25 00:27:27Z  garyp
    Added function codes for various services.
    Revision 1.7  2009/02/08 01:07:19Z  garyp
    Merged from the v4.0 code base.  Added requestor support for trace enable/
    disable.  Moved the definition of DLOSREQHANDLE into this file so it is
    available to code being built outside the standard build process.  Added
    requestor functions.  Added a macro to declare and initialize a requestor
    packet.
    Revision 1.6  2008/05/23 02:15:52Z  garyp
    Updated the stats interface to use a device name rather than a disk number.
    Added interfaces for controlling the profiler.
    Revision 1.5.1.2  2008/05/23 02:15:52Z  garyp
    Updated the stats interface to use a device name rather than a disk number.
    Added interfaces for controlling the profiler.
    Revision 1.5  2008/03/22 19:09:20Z  Garyp
    Minor data type changes.
    Revision 1.4  2007/12/13 23:57:21Z  Garyp
    Added a "SubType" field to the DCLREQ_STATS structure,
    Revision 1.3  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.2  2007/08/29 23:29:23Z  Garyp
    Changed some D_BUFFER types to char to avoid compiler warnings in
    some environments.
    Revision 1.1  2007/06/22 20:44:02Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLIOSYS_H_INCLUDED
#define DLIOSYS_H_INCLUDED


#define MAX_DEVICE_NAME_LEN         (16)


/*-------------------------------------------------------------------
    DCLIO_* Macros

    These macros are used for defining and manipulating I/O request
    function numbers.

    These numbers must be unique across products, and (at least for
    the time being) fit in an 'unsigned' field, which could be only
    16-bits wide.  The high 4 bits are used for the PRODUCTNUM_*
    value from dlprod.h (normalized to be zero-based), and the low
    12 bits are used for the actual function number.
-------------------------------------------------------------------*/
#define DCLIO_FUNCTIONMASK          (0x0FFF)
#define DCLIO_PRODUCTMASK           (0xF000)
#define DCLIO_PRODUCTSHIFT          (12)

#define DCLIO_MAKEFUNCTION(prod, num) ((((prod)-1) << DCLIO_PRODUCTSHIFT) | (num))
#define DCLIO_GETPRODUCT(func)        ((((func) & DCLIO_PRODUCTMASK) >> DCLIO_PRODUCTSHIFT) + 1)

typedef enum
{
    DCLIOFUNC_RESERVED              = DCLIO_MAKEFUNCTION(PRODUCTNUM_DCL, 0x000),

    DCLIOFUNC_SYSTEM_INFO           = DCLIO_MAKEFUNCTION(PRODUCTNUM_DCL, 0x080),
    DCLIOFUNC_STATS_RESET           = DCLIO_MAKEFUNCTION(PRODUCTNUM_DCL, 0x100),
    DCLIOFUNC_STATS_REQUEST,

    DCLIOFUNC_PROFILER_ENABLE       = DCLIO_MAKEFUNCTION(PRODUCTNUM_DCL, 0x200),
    DCLIOFUNC_PROFILER_DISABLE,
    DCLIOFUNC_PROFILER_RESET,
    DCLIOFUNC_PROFILER_SUMMARY,
    DCLIOFUNC_PROFILER_TOGGLE,
    DCLIOFUNC_PROFILER_MEMPOOLSTATE,

    DCLIOFUNC_ASSERT_FIRED          = DCLIO_MAKEFUNCTION(PRODUCTNUM_DCL, 0x240),
    DCLIOFUNC_ASSERT_MODE,

    DCLIOFUNC_TRACE_ENABLE          = DCLIO_MAKEFUNCTION(PRODUCTNUM_DCL, 0x280),
    DCLIOFUNC_TRACE_DISABLE,
    DCLIOFUNC_TRACE_TOGGLE,

    DCLIOFUNC_LOG_OPEN              = DCLIO_MAKEFUNCTION(PRODUCTNUM_DCL, 0x2C0),
    DCLIOFUNC_LOG_CLOSE,
    DCLIOFUNC_LOG_WRITE,
    DCLIOFUNC_LOG_WRITESTRING,
    DCLIOFUNC_LOG_FLUSH,
    DCLIOFUNC_LOG_MODE,

    DCLIOFUNC_SERVICE_CREATE        = DCLIO_MAKEFUNCTION(PRODUCTNUM_DCL, 0x300),
    DCLIOFUNC_SERVICE_DESTROY,
    DCLIOFUNC_SERVICE_REGISTER,
    DCLIOFUNC_SERVICE_DEREGISTER,
    DCLIOFUNC_SERVICE_DISPATCH,
    DCLIOFUNC_SERVICE_REQUEST,

    DCLIOFUNC_MEMTRACK_BLOCKVERIFY  = DCLIO_MAKEFUNCTION(PRODUCTNUM_DCL, 0x400),
    DCLIOFUNC_MEMTRACK_POOLVERIFY,
    DCLIOFUNC_MEMTRACK_POOLDUMP,
    DCLIOFUNC_MEMTRACK_STATS,
    DCLIOFUNC_MEMTRACK_ERRORMODE,

    DCLIOFUNC_MEMVAL_ERRORMODE      = DCLIO_MAKEFUNCTION(PRODUCTNUM_DCL, 0x480),
    DCLIOFUNC_MEMVAL_SUMMARY,
    DCLIOFUNC_MEMVAL_BUFFERADD,
    DCLIOFUNC_MEMVAL_BUFFERREMOVE,
    DCLIOFUNC_MEMVAL_BUFFERVALIDATE,

    DCLIOFUNC_FILESPY_TRACEMASK     = DCLIO_MAKEFUNCTION(PRODUCTNUM_DCL, 0x500),

    DCLIOFUNC_HIGHLIMIT
} DCLIOFUNC;



/*-------------------------------------------------------------------
    DCLIOREQUEST

    Note that ioFunc is an int rather than the enumerated type
    DCLIOFUNC because constants from other enumerated types are also
    stored there, and some compilers in some environments will warn
    about this.
-------------------------------------------------------------------*/
typedef struct
{
    int             ioFunc;
    D_UINT32        ulReqLen;
} DCLIOREQUEST;


/*---------------------------------------------------------
    Version
---------------------------------------------------------*/
typedef struct
{
    DCLIOREQUEST    ior;
    D_UINT32        ulSignature;
    D_UINT32        ulVersion;
    char            szBuildNum[DCL_MAX_BUILDNUM_LENGTH+1];
} DCLREQ_VERSIONCHECK;


/*---------------------------------------------------------
    Stats Interface
---------------------------------------------------------*/
typedef struct
{
    DCLIOREQUEST    ior;
    char            szDeviceName[MAX_DEVICE_NAME_LEN];
} DCLREQ_STATSRESET;

typedef struct
{
    DCLIOREQUEST    ior;
    DCLSTATUS       ResultStatus;
    unsigned        nType;
    unsigned        nSubType;
    unsigned        nBufferLen;
    char           *pDescBuffer;
    char           *pDataBuffer;
    const char     *pszCatSuffix;
    const char     *pszBldSuffix;
    char            szDeviceName[MAX_DEVICE_NAME_LEN];
} DCLREQ_STATS;

    
/*---------------------------------------------------------
    Profiler 
---------------------------------------------------------*/
typedef struct
{
    DCLIOREQUEST    ior;
    DCLSTATUS       dclStat;
} DCLREQ_PROFILER_ENABLE;

typedef struct
{
    DCLIOREQUEST    ior;
    DCLSTATUS       dclStat;
} DCLREQ_PROFILER_DISABLE;

typedef struct
{
    DCLIOREQUEST    ior;
    DCLSTATUS       dclStat;
} DCLREQ_PROFILER_RESET;

typedef struct
{
    DCLIOREQUEST    ior;
    D_BOOL          fReset;
    D_BOOL          fShort;
    D_BOOL          fAdjustOverhead;
    DCLSTATUS       dclStat;
} DCLREQ_PROFILER_SUMMARY;

typedef struct
{
    DCLIOREQUEST    ior;
    D_BOOL          fSummaryReset;
    DCLSTATUS       dclStat;
} DCLREQ_PROFILER_TOGGLE;

typedef struct
{
    DCLIOREQUEST    ior;
    D_BOOL          fNewValidationEnabled;
    D_BOOL          fOldValidationEnabled;
} DCLREQ_PROFILER_MEMPOOLSTATE;


/*---------------------------------------------------------
    Tracing 
---------------------------------------------------------*/
typedef struct
{
    DCLIOREQUEST    ior;
    unsigned        nLevel;
} DCLREQ_TRACEENABLE;

typedef struct
{
    DCLIOREQUEST    ior;
} DCLREQ_TRACEDISABLE;

typedef struct
{
    DCLIOREQUEST    ior;
} DCLREQ_TRACETOGGLE;


/*---------------------------------------------------------
    Tracing 
---------------------------------------------------------*/
typedef struct
{
    DCLIOREQUEST    ior;
    D_UINT32        ulNewTraceMask;
    D_UINT32        ulOldTraceMask;
    DCLSTATUS       dclStat;
} DCLREQ_FILESPY_TRACEMASK;


/*-------------------------------------------------------------------
    DCLDECLAREREQUESTPACKET()

    This macro is used to declare a DCLREQ_* structure and fill
    in the function name, structure length, and ensure that the
    remainder of the structure is zero'd out.

    The structure name must have the form:  DCLREQ_typ_func
    The function name must have the form:   DCLIOFUNC_typ_func

    Parameters:
        typ  - The structure type
        func - The function name
        var  - The variable name to use for the structure

    Returns:
        Nothing.
-------------------------------------------------------------------*/
#define DCLDECLAREREQUESTPACKET(typ, func, var)                     \
    DCLREQ_ ## typ ## _ ## func var = {{DCLIOFUNC_ ## typ ## _ ## func, sizeof(var)}}


/*-------------------------------------------------------------------
    Type: DCLIOSTATUS

    Contains the results of an I/O request.  The DCLIOSTATUS structure
    has the following layout:

	<pre>
    {{+-----------------------------------------+}}
    {{|                                         |}}
    {{|  typedef struct                         |}}
    {{|  {                                      |}}
    {{|      D_UINT32    ulCount;               |}}
    {{|      DCLSTATUS   dclStat;               |}}
    {{|  } DCLIOSTATUS;                         |}}
    {{|                                         |}}
    {{+-----------------------------------------+}}
	</pre>

    *ulcount* - The count of elements which were successfully
                transferred.
    *dclStat* - The standard DCLSTATUS value.

   The following methods are used to determine the state of
   an operation:

   1) If dclStat == DCLSTAT_SUCCESS, then the operation was entirely
      successful.  ulCount will indicate the count of elements
      transferred, however examining this value is not necessary.

   2) If dclStat != DCLSTAT_SUCCESS, then the operation <may> have
      been partially successful.  ulCount will indicate the number
      of elements that were transferred successfully, if any.  The
      element indicated by ulCount+1 will be the element where the
      error occurred.  For write operations, it should be assumed
      that the element where the I/O error occurred was modified.
-------------------------------------------------------------------*/
typedef struct sDCLIOSTATUS
{
    D_UINT32    ulCount;            /* Number of elements transferred   */
    DCLSTATUS   dclStat;            /* Standard values from dlstatus.h  */
} DCLIOSTATUS;


/*-------------------------------------------------------------------
    Function Prototypes
-------------------------------------------------------------------*/
#define     DclIoctlDispatch    DCLFUNC(DclIoctlDispatch)
#define     DclDecodeIOStatus    DCLFUNC(DclDecodeIOStatus)

DCLSTATUS   DclIoctlDispatch(DCLIOREQUEST *pReqHdr);
const char *DclDecodeIOStatus(DCLIOSTATUS *pIOStatus);




#endif /* DLIOSYS_H_INCLUDED */

