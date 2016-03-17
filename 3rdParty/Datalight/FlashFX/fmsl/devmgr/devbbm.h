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
                                Revision History
    $Log: devbbm.h $
    Revision 1.7  2009/01/18 08:43:26Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.6  2009/01/07 08:51:54Z  keithg
    Added conditional copmilation of FFXCONF_STATS_BBM;
    Revision 1.5  2008/12/19 02:10:48Z  keithg
    Added include for statistics, added BBM stats prototype.
    Revision 1.4  2008/12/18 04:26:47Z  keithg
    Changed function names to be more concise.
    Revision 1.3  2008/12/15 20:24:02Z  keithg
    Conditioned prototypes on BBMSUPPORT.
    Revision 1.2  2008/12/02 23:05:37Z  keithg
    Moved private data structure, added prototype for block replacement.
    Revision 1.1  2008/10/24 04:40:02Z  keithg
    Initial revision
---------------------------------------------------------------------------*/


#if FFXCONF_BBMSUPPORT

#if FFXCONF_STATS_BBM
 #include <fxstats.h>
#endif

FFXSTATUS FfxDevBbmDestroy(FFXFIMDEVICE *pfd);
FFXSTATUS FfxDevBbmCreate(FFXFIMDEVICE *pfd);
void FfxDevBbmQueryRawBlock(
    FFXFIMDEVICE *pfd,
    FFXIOR_DEV_GET_RAW_BLOCK_INFO *pinfo);
void FfxDevBbmRemapPage(
    FFXFIMDEVICE *pfd,
    D_UINT32 *pulPage,
    D_UINT32 *pulCount);
void FfxDevBbmRemapBlock(
    FFXFIMDEVICE *pfd,
    D_UINT32 *pulBlock,
    D_UINT32 *pulCount);
void FfxDevBbmMapBlockToRaw(
    FFXFIMDEVICE *pfd,
    D_UINT32 *pulBlock);
FFXIOSTATUS FfxDevBbmRelocateBlock(
    FFXFIMDEVICE *pfd,
    D_UINT32 ulOrgBlock,
    D_UINT32 ulPageCount);

#if FFXCONF_STATS_BBM
FFXSTATUS FfxDevBbmStatsQuery(
    FFXFIMDEVICE *pfd,
    FFXBBMSTATS *pbbmstats);
#endif

#endif

