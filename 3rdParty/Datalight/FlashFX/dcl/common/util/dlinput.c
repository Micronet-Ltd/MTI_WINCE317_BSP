/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
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

    This module contains general functions for performing character input.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlinput.c $
    Revision 1.5  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.4  2009/06/28 00:47:45Z  garyp
    Updated to use this feature as a service.
    Revision 1.3  2009/02/18 07:32:02Z  garyp
    Voided some unused parameters.
    Revision 1.2  2009/02/08 04:51:53Z  garyp
    Fixed a missing parameter.
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlservice.h>
#include <dlinstance.h>


/*-------------------------------------------------------------------
    Public: DclInputServiceInit()

    Initialize the input service.

    Parameters:
        hDclInst - The DCL instance handle.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclInputServiceInit(
    DCLINSTANCEHANDLE       hDclInst)
{
  #if DCL_OSFEATURE_CONSOLEINPUT
    static DCLSERVICEHEADER SrvInput[DCL_MAX_INSTANCES];
    static DCLINPUTINFO     ii[DCL_MAX_INSTANCES];
    DCLSERVICEHEADER       *pSrv;
    DCLSTATUS               dclStat;
    unsigned                nInstNum;

    /*  REMINDER! Because some environments are capable of restarting,
                  without reinitializing static data, this function must
                  explicitly ensure that any static data is manually
                  zero'd, rather than just expecting it to already be in
                  that state.
    */

    if(!hDclInst)
        return DCLSTAT_SERVICE_BADHANDLE;

    nInstNum = DclInstanceNumber(hDclInst);
    ii[nInstNum].pfnInputChar = DclOsInputChar;
    ii[nInstNum].pContext = NULL;

    pSrv = &SrvInput[nInstNum];
    dclStat = DclServiceHeaderInit(&pSrv, "DLINPUT", DCLSERVICE_INPUT, NULL, &ii[nInstNum], 0);
    if(dclStat == DCLSTAT_SUCCESS)
        dclStat = DclServiceCreate(hDclInst, pSrv);

    return dclStat;

  #else

    /*  Even if a feature is disabled at compile-time, a given project may
        still be using a default set of services which tries to initialize
        it.  Therefore, to avoid link errors, this "Init" function must
        exist.  Simply politely decline to initialize if this is the case.
        The DCLSTAT_FEATUREDISABLED status code is explicitly accommodated
        in the project creation process, and will not abort the process.
    */
    (void)hDclInst;
    return DCLSTAT_FEATUREDISABLED;

  #endif
}


#if DCL_OSFEATURE_CONSOLEINPUT

/*-------------------------------------------------------------------
    Public: DclInputChar()

    Obtain character input.

    When this function is used by common, OS independent code, it
    <must> be assumed that the OS interface may wait for a character,
    or even the [Enter] key before returning from this function.

    When this function is used from OS specific code, where the OS
    characteristics are known, it may return immediately with a
    DCLSTAT_INPUT_IDLE status.

    Parameters:
        hDclInst - The DCL instance handle.  May be NULL to use the
                   default DCL instance.
        puChar   - A pointer to a buffer to receive the character.
                   The contents of this field will only be modified
                   if DCLSTAT_SUCCESS is returned.
        nFlags   - This parameter is reserved for future use and must
                   be 0.

    Return Value:
        Returns one of the following DCLSTATUS values...
          DCLSTAT_SUCCESS         - A character was returned
          DCLSTAT_INPUT_DISABLED  - The input interface is disabled
          DCLSTAT_INPUT_IDLE      - There is no input ready
          DCLSTAT_INPUT_TERMINATE - The OS is terminating the program
          Other...
-------------------------------------------------------------------*/
DCLSTATUS DclInputChar(
    DCLINSTANCEHANDLE   hDclInst,
    D_UCHAR            *puChar,
    unsigned            nFlags)
{
    DCLINPUTINFO       *pII;
    DCLSTATUS           dclStat;

    DclAssert(puChar);
    DclAssert(nFlags == 0);

    dclStat = DclServiceData(hDclInst, DCLSERVICE_INPUT, (void**)&pII);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    if(!pII)
        return DCLSTAT_SERVICE_NOTINITIALIZED;

    if(!pII->pfnInputChar)
        return DCLSTAT_INPUT_DISABLED;

    /*  Get a character...
    */
    while(TRUE)
    {
        DCLSTATUS dclStat;

        dclStat = (*pII->pfnInputChar)(pII->pContext, puChar, nFlags);

        if(dclStat == DCLSTAT_INPUT_IDLE)
            DclOsSleep(1);
        else
            return dclStat;
    }
}


#endif  /* DCL_OSFEATURE_CONSOLEINPUT */

