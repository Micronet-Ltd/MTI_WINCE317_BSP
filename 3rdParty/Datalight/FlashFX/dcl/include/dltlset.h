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

    This header defines the legitimate values for D_TOOLSETNUM.

    This information MUST match that found in dltlset.inc and dltlinit.h, as
    well as the settings found in all the various toolset.mak/toolinit.bat
    files.

    The numbering scheme used is as follows:

    1) The 1000's digit indicates the vendor as well as the major
       product category:
          1000 - Borland
          2000 - Microsoft
          3000 - ARM
          4000 - Diab
          5000 - GNU
          6000 - Green Hills
          7000 - Altium Tasking
          8000 - Mentor
          9000 - Marvell
         10000 - Texas Instruments
    2) The 100's and 10's digits indicate the version number.
    3) The 1's digit is used for further differentiation as needed.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltlset.h $
    Revision 1.13  2010/06/24 14:18:59Z  jimmb
    Name police - Bad programmer - no donut - renamed to reflect GNU
    Revision 1.12  2010/06/21 15:20:06Z  jimmb
    Added the GNU linux tool abstraction for RTOS
    Revision 1.11  2010/06/18 15:04:37Z  jimmb
    Completed adding the Intel compiler descriptions for DCL
    Revision 1.10  2010/04/28 23:30:36Z  garyp
    Added support for the TICCS ToolSet abstraction.
    Revision 1.9  2009/02/08 01:13:27Z  garyp
    Merged from the v4.0 branch.  Added support for the MARVELLSDT and
    MSWCE ToolSet abstractions.
    Revision 1.8  2008/03/11 21:56:02Z  jeremys
    Added WDK support.
    Revision 1.7  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.6  2007/06/15 01:00:37Z  pauli
    Replaced the poorly named gnunu toolset with gnucs.
    Revision 1.5  2007/03/18 18:05:54Z  Garyp
    Added the "msvs" ToolSet.
    Revision 1.4  2006/12/06 23:55:38Z  Garyp
    Added Microtec ToolSet support.
    Revision 1.3  2006/11/02 19:54:07Z  Garyp
    Renamed the Green Hills toolset from ghsarm36 to ghs.
    Revision 1.2  2006/10/07 01:03:26Z  Garyp
    Added support for the RVDS30 ToolSet abstraction.
    Revision 1.1  2005/10/02 00:29:48Z  Pauli
    Initial revision
    Revision 1.9  2005/06/21 21:29:15Z  billr
    More meaningful name for GCC 3.x toolset that doesn't conflict with
    historical gnu.h.
    Revision 1.8  2005/06/21 20:03:43Z  billr
    Add toolset for GCC 3.x (3.4 at least).
    Revision 1.7  2005/06/20 20:54:30Z  Pauli
    Added toolset for Nucleus GNU tools.
    Revision 1.6  2005/06/10 16:53:56Z  PaulI
    XP Merge
    Revision 1.5  2005/04/24 07:13:56Z  GaryP
    Added support for new and renamed ToolSets.
    Revision 1.4.1.2  2005/03/31 21:06:51Z  garyp
    Added the MSWINDDK ToolSet.
    Revision 1.4  2005/02/27 06:47:47Z  GaryP
    Let's try a forward slash.
    Revision 1.3  2005/02/27 00:44:54Z  GaryP
    Experimenting with the path to the toolset headers to mollify the Borland
    preprocessor.
    Revision 1.2  2005/02/22 03:19:48Z  GaryP
    Prefixed the toolset names with "TS_".  Modified to include the toolset
    specific header.
    Revision 1.1  2005/01/25 23:47:28Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLTLSET_H_INCLUDED
#define DLTLSET_H_INCLUDED

#define TS_BC4X         1400  /* Borland C 4.x                               */
#define TS_DEVTOOLS     1520  /* DL DevTools (BC 5.2)                        */
#define TS_MSWCE4       2400  /* Microsoft Windows CE.net 4.2 Command Line (deprecated use "mswce")  */
#define TS_MSWCEPB      2401  /* Microsoft Windows CE Platform Builder 3 & 4 */
#define TS_MSWCE        2402  /* Microsoft Windows CE 4, 5 and 6, WinMobile Command Line */
#define TS_MSVC6        2600  /* Microsoft Visual C 6                        */
#define TS_MSVS         2620  /* Microsoft Visual Studio 6, 7, and 8         */
#define TS_MSWINDDK     2700  /* Microsoft Windows XP DDK                    */
#define TS_MSWDK        2701  /* Microsoft Windows XP WDK                    */
#define TS_ADS12        3120  /* ARM Developer's Suite 1.2                   */
#define TS_RVDS30       3300  /* ARM RealView Developer's Suite 3.0          */
#define TS_DIABVX5X     4500  /* WindRiver Diab tools for VxWorks 5.x        */
#define TS_DIABVX6X     4505  /* WindRiver Diab tools for VxWorks 6.x        */
#define TS_GNUVX5X      5290  /* GNU tools for VxWorks 5.x                   */
#define TS_GNUVX6X      5295  /* GNU tools for VxWorks 6.x                   */
#define TS_ICCVX6X      5296  /* Intel compiler a superset of GNU         */
#define TS_GNURTOS      5297  /* GNU tools for RTOS                          */
#define TS_GCC3X        5300  /* GCC 3.x (3.4 at least)                      */
#define TS_GNUCS	    5400  /* Code Sourcery ARM GNU Tools                 */
#define TS_GHS          6360  /* Green Hills ARM tools (v4.2.3 tested)       */
#define TS_TASKING      7152  /* Altium Tasking v1.5r2                       */
#define TS_MICROTEC     8500  /* Mentor Graphics Microtec tools              */
#define TS_MARVELLSDT   9210  /* Marvell SDT v2.1                            */
#define TS_TICCS       10000  /* Texas Instruments Code Composer Studio      */

#endif  /* DLTLSET_H_INCLUDED */


