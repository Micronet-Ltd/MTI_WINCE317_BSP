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

    This header defines the interface to the Service API.

    ToDo: Break down services by class, so that each product can have its
          own set of services -- including OEM specific services.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlservice.h $
    Revision 1.7  2010/04/18 21:25:33Z  garyp
    Added support for DCLREQ_SERVICE_REQUEST.
    Revision 1.6  2010/01/08 00:47:20Z  garyp
    Undid the "const" change from the previous revision.
    Revision 1.5  2010/01/07 02:43:04Z  garyp
    Updated so that the instance private data pointer is considered "const"
    so far as the service framework is concerned.  Added prototypes.
    Revision 1.4  2009/11/08 03:51:12Z  garyp
    Added DCLSERVICE_PROFILER.
    Revision 1.3  2009/10/06 19:33:19Z  keithg
    Added DCLSERVICE_RAND enum.
    Revision 1.2  2009/06/23 20:22:02Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.2  2009/01/10 02:56:52Z  garyp
    Updated to support the DclServiceIoctl() function.
    Revision 1.1  2008/12/08 18:23:22Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLSERVICE_H_INCLUDED
#define DLSERVICE_H_INCLUDED

/*-------------------------------------------------------------------
    This list enumerates all the service numbers, both for Datalight
    internal services, and for OEM specific services.  This list must
    not contain super-high values, as the ahServices[] array in the
    DCLINSTANCEDATA structure is dimensioned with DCLSERVICE_HIGHLIMIT.
-------------------------------------------------------------------*/
typedef enum
{
    DCLSERVICE_LOWLIMIT = 0,
    DCLSERVICE_ASSERT,
    DCLSERVICE_OUTPUT,
    DCLSERVICE_INPUT,
    DCLSERVICE_TIMER,
    DCLSERVICE_HRTIME,
    DCLSERVICE_MEMTRACK,
    DCLSERVICE_MEMVALIDATION,
    DCLSERVICE_LOG,
    DCLSERVICE_RAND,
    DCLSERVICE_PROFILER,
    DCLSERVICE_BLOCKDEV,
    DCLSERVICE_OEM_1,           /* The "OEM" services must be last */
    DCLSERVICE_OEM_2,
    DCLSERVICE_OEM_3,
    DCLSERVICE_OEM_4,
    DCLSERVICE_OEM_5,
    DCLSERVICE_OEM_6,
    DCLSERVICE_OEM_7,
    DCLSERVICE_OEM_8,
    DCLSERVICE_HIGHLIMIT
} DCLSERVICE;

typedef struct
{
    DCLIOREQUEST    ior;
    DCLIOREQUEST   *pSubRequest;
} DCLREQ_SERVICE;


typedef DCLSTATUS(*DCLPFNSERVICEIOCTL)(DCLSERVICEHANDLE hService, DCLREQ_SERVICE *pReq, void *pPrivateData);


typedef struct sDCLSERVICEHEADER
{
    unsigned                    nStrucLen;      /* Structure length */
    const char                 *pszName;        /* short name */
    DCLSERVICE                  nType;          /* DCLSERVICE_* types */
    DCLPFNSERVICEIOCTL          pfnIoctl;       /* Optional IOCTL function */
    void                       *pPrivateData;   /* Optional associated data */
    unsigned                    nFlags;         /* DCLSERVICEFLAGS_* defined below */
    unsigned                    nMaxRecurse;    /* Max times the service can recursively call itself (0=none) */
    unsigned                    nRecurseCount;  /* Current times recursed into */
    PDCLMUTEX                   pServiceMutex;
    struct sDCLSERVICEHEADER   *pNext;          /* Next in the overall service chain */
    struct sDCLSERVICEHEADER   *pPrevByType;    /* Previous service of this type */
    DCLINSTANCEHANDLE           hDclInst;       /* DCL Instance which owns the service */
} DCLSERVICEHEADER;


#define DCLSERVICEFLAG_MASKVALID    (0x00000003U)   /* Mask for valid user flags */
#define DCLSERVICEFLAG_THREADSAFE   (0x00000001U)   /* The service is inherently thread-safe */
#define DCLSERVICEFLAG_SLAVE        (0x00000002U)   /* The service is a slave of another */
#define DCLSERVICEFLAG_MASKINTERNAL (0xE0000000U)
#define DCLSERVICEFLAG_AUTOALLOCED  (0x20000000U)   /* INTERNAL: Memory must be automatically freed */
#define DCLSERVICEFLAG_REGISTERED   (0x40000000U)   /* INTERNAL: Service has been registered */
#define DCLSERVICEFLAG_PROJECT      (0x80000000U)   /* INTERNAL: Denotes services auto created at project init time */
#define DCLSERVICEFLAG_MASKRESERVED (~(DCLSERVICEFLAG_MASKVALID | DCLSERVICEFLAG_MASKINTERNAL))

#define DCLDECLARESERVICE(var, pName, typ, pfnIO, pPriv, Flags)                 \
    DCLSERVICEHEADER var = {sizeof(DCLSERVICEHEADER), pName, typ, pfnIO, pPriv, Flags}


#define     DclServiceHeaderInit    DCLFUNC(DclServiceHeaderInit)
#define     DclServiceCreate        DCLFUNC(DclServiceCreate)
#define     DclServiceDestroy       DCLFUNC(DclServiceDestroy)
#define     DclServiceDestroyAll    DCLFUNC(DclServiceDestroyAll)
#define     DclServiceIsInitialized DCLFUNC(DclServiceIsInitialized)
#define     DclServiceIsInChain     DCLFUNC(DclServiceIsInChain)
#define     DclServicePointer       DCLFUNC(DclServicePointer)
#define     DclServiceIoctl         DCLFUNC(DclServiceIoctl)
#define     DclServiceIoctlPrevious DCLFUNC(DclServiceIoctlPrevious)
#define     DclServiceData          DCLFUNC(DclServiceData)
#define     DclServiceDataPrevious  DCLFUNC(DclServiceDataPrevious)
#define     DclServiceRegister      DCLFUNC(DclServiceRegister)
#define     DclServiceDeregister    DCLFUNC(DclServiceDeregister)
#define     DclServiceValidate      DCLFUNC(DclServiceValidate)

DCLSTATUS   DclServiceHeaderInit(   DCLSERVICEHEADER **ppService, const char *pszName, DCLSERVICE nServiceType, DCLPFNSERVICEIOCTL pfnIoctl, void *pPrivateData, unsigned nFlags);
DCLSTATUS   DclServiceCreate(       DCLINSTANCEHANDLE hDclInst, DCLSERVICEHEADER *pService);
DCLSTATUS   DclServiceDestroy(      DCLSERVICEHEADER *pService);
DCLSTATUS   DclServiceDestroyAll(   DCLINSTANCEHANDLE hDclInst, D_BOOL fTransient);
DCLSTATUS   DclServiceIsInitialized(DCLINSTANCEHANDLE hDclInst, DCLSERVICE nServiceType);
D_BOOL      DclServiceIsInChain(    DCLSERVICEHANDLE hService, DCLSERVICEHANDLE hSearchService);
DCLSTATUS   DclServicePointer(      DCLINSTANCEHANDLE hDclInst, DCLSERVICE nServiceType, DCLSERVICEHANDLE *phService);
DCLSTATUS   DclServiceIoctl(        DCLINSTANCEHANDLE hDclInst, DCLSERVICE nServiceType, DCLIOREQUEST *pRequest);
DCLSTATUS   DclServiceIoctlPrevious(DCLSERVICEHANDLE hService, DCLREQ_SERVICE *pReq);
DCLSTATUS   DclServiceData(         DCLINSTANCEHANDLE hDclInst, DCLSERVICE nServiceType, void **ppServiceData);
DCLSTATUS   DclServiceDataPrevious( DCLSERVICEHANDLE hService, void **ppServiceData);
DCLSTATUS   DclServiceRegister(     DCLSERVICEHANDLE hService);
DCLSTATUS   DclServiceDeregister(   DCLSERVICEHANDLE hService);
DCLSTATUS   DclServiceValidate(     DCLSERVICEHANDLE hService);


typedef struct
{
    DCLIOREQUEST            ior;
    DCLSERVICE              nServiceType;
    DCLIOREQUEST           *pSubReq;
    DCLSTATUS               dclStat;
} DCLREQ_SERVICE_REQUEST;


#endif  /* DLSERVICE_H_INCLUDED */

