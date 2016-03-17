/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2007 Datalight, Inc.
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

    See the FIM list, available on the Datalight web site, for a complete
    list of supported flash parts.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: devflash.h $
    Revision 1.5  2007/11/03 23:49:35Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2006/08/11 01:37:42Z  rickc
    Added ambx16 FIM
    Revision 1.3  2006/05/31 19:15:16Z  rickc
    Added asbx16 FIM
    Revision 1.2  2006/04/04 00:15:04Z  michaelm
    removed mention of fimlist.pdf and instead refer to website
    Revision 1.1  2006/02/01 00:45:36Z  Garyp
    Initial revision
    Revision 1.1  2005/05/10 17:57:58Z  Pauli
    Initial revision
    Revision 1.2  2005/05/10 18:57:58Z  garyp
    Added asux8.
    Revision 1.1  2005/04/25 02:54:26Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifdef FIMSCANMODE
#undef FIMSCANMODE
#endif

/*  If FFXFIM_ScanAllFIMs is defined, enable all the multithreaded
    FIMs except things like norram and norfile.
*/
#ifdef      FFXFIM_ScanAllFIMs
#define     FIMSCANMODE TRUE
#if D_DEBUG == 0
#error "The FFXFIM_ScanAllFIMs setting is only meaningful when doing a DEBUG build"
#endif

    #define  FFXFIM_asux8
    #define  FFXFIM_asu4x8
    #define  FFXFIM_ambx16
    #define  FFXFIM_amb2x16
    #define  FFXFIM_asb2x16
    #define  FFXFIM_asbx16
    #define  FFXFIM_iffx8
    #define  FFXFIM_iffx16
    #define  FFXFIM_iff2x16
    #define  FFXFIM_isfx16
    #define  FFXFIM_isf2x16
    #define  FFXFIM_isf4x8
#endif

/*  If FFXFIM_ScanAMDFIMs is defined, enable all the multithreaded
    FIMs except things like norram and norfile.
*/
#ifdef      FFXFIM_ScanAMDFIMs
#define     FIMSCANMODE TRUE
#if D_DEBUG == 0
#error "The FFXFIM_ScanAMDFIMs setting is only meaningful when doing a DEBUG build"
#endif

    #define  FFXFIM_asux8
    #define  FFXFIM_asu4x8
    #define  FFXFIM_ambx16
    #define  FFXFIM_amb2x16
    #define  FFXFIM_asb2x16
    #define  FFXFIM_asbx16

#endif

/*  If FFXFIM_ScanIntelFIMs is defined, enable all the multithreaded
    FIMs except things like norram and norfile.
*/
#ifdef      FFXFIM_ScanIntelFIMs
#define     FIMSCANMODE TRUE
#if D_DEBUG == 0
#error "The FFXFIM_ScanIntelFIMs setting is only meaningful when doing a DEBUG build"
#endif

    #define  FFXFIM_iffx8
    #define  FFXFIM_iffx16
    #define  FFXFIM_iff2x16
    #define  FFXFIM_isfx16
    #define  FFXFIM_isf2x16
    #define  FFXFIM_isf4x8
#endif


