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
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

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

    This module contains the general ioctl dispatcher routines used by
    typical device drivers.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvioctl.c $
    Revision 1.14  2009/11/02 23:39:49Z  garyp
    Added profiler instrumentation.  
    Revision 1.13  2009/03/31 19:17:23Z  davidh
    Function Headers updated for AutoDoc.
    Revision 1.12  2009/02/09 02:42:54Z  garyp
    Merged from the v4.0 branch.  Cleaned up the order in which things are
    dispatched to better support generic IOCTLs.
    Revision 1.11  2008/03/22 23:44:49Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.10  2008/01/13 07:26:28Z  keithg
    Function header updates to support autodoc.
    Revision 1.9  2007/12/26 02:11:07Z  Garyp
    Moved the dispatch logic for the FML and lower layers (including the
    version check) to the function FfxFmlIoctlMasterDispatch(), to allow the
    IOCTL interface to be used even if the higher layers of FlashFX are not
    included or enabled.
    Revision 1.8  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.7  2007/08/03 02:50:11Z  garyp
    Updated to use tje new 32-bit version number.
    Revision 1.6  2007/07/31 01:26:48Z  Garyp
    Added support for dispatching generic IOCTL requests.
    Revision 1.5  2006/09/16 19:26:56Z  Garyp
    Modified to handle build numbers which are now strings rather than numbers.
    Revision 1.4  2006/02/20 22:22:17Z  Garyp
    Re-enabled the external API stuff.
    Revision 1.3  2006/02/13 03:15:24Z  Garyp
    Temporarily disabled -- work-in-progress.
    Revision 1.2  2006/02/07 06:37:56Z  Garyp
    Updated to use new device/disk handle model.
    Revision 1.1  2005/10/02 01:29:06Z  Pauli
    Initial revision
    Revision 1.5  2005/02/13 19:04:26Z  GaryP
    Minor debug code changes.
    Revision 1.4  2004/12/30 17:32:42Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.3  2004/12/01 22:21:43Z  GaryP
    Fixed the file number.
    Revision 1.2  2004/12/01 01:06:12Z  PaulI
    Set the FILENUM for this module.
    Revision 1.1  2004/11/29 18:51:42Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxapireq.h>


/*-------------------------------------------------------------------
    Public: FfxDriverIoctlMasterDispatch()

    This function is the Driver Framework master dispatcher.

    Note that this function only explicitly handles requests for
    things that pertain to the higher layers of FlashFX -- VBF and
    above.  Lower layers are processed by the FML master dispatch
    function.

    Parameters:
        pReq - A pointer to the request block for the function.

    Return Value:
        Returns an FFXSTATUS value indicating the status of the
        request.  Note that this status value indicates whether the
        IOCTL request and interface are working properly, NOT whether
        the actual function call was successful.  If this function
        returns FFXSTAT_SUCCESS, then the returned request structure
        must be examined to see if the actual API function was
        successful.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverIoctlMasterDispatch(
    FFXIOREQUEST       *pReq)
{
    FFXSTATUS           ffxStat;

    if(!pReq)
    {
        DclError();
        return FFXSTAT_APIBADREQUESTPOINTER;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_EXTAPI, 2, 0),
        "FfxDriverIoctlMasterDispatch() Function 0x%x\n", pReq->ioFunc));

    DclProfilerEnter("FfxDriverIoctlMasterDispatch", 0, 0);

    ffxStat = FfxDriverGenericIoctlDispatch(pReq);
    if(ffxStat == DCLSTAT_UNSUPPORTEDFUNCTION)
    {
        FXIOFUNC nFunc = pReq->ioFunc;
        
        if(nFunc >= FXIOFUNC_EXTDRVRFW_MIN && nFunc <= FXIOFUNC_EXTDRVRFW_MAX)
        {
            ffxStat = FfxDriverIoctlDispatch(pReq);
        }
      #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
        else if(nFunc >= FXIOFUNC_EXTVBF_MIN && nFunc <= FXIOFUNC_EXTVBF_MAX)
        {
            ffxStat = FfxVbfIoctlDispatch(pReq);
        }
      #endif
        else if(nFunc >= FXIOFUNC_EXTFML_MIN && nFunc <= FXIOFUNC_EXTFML_MAX)
        {
            /*  Even though the FML can handle the generic requests as well,
                we exclude them using the checks above, since we don't want
                them to be processed twice.
            */
            ffxStat = FfxFmlIoctlDispatch(pReq);
        }
    }
    
    DclProfilerLeave(0);

    return ffxStat;
}

