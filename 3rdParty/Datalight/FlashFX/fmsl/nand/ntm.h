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
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

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

    This header contains symbols, structures, types that are used internally
    by the NTM layer.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntm.h $
    Revision 1.16  2010/08/09 17:02:25Z  glenns
    Backed out previous change as NativePageRead needs to be an
    optional interface and is not present in all NTMs.
    Revision 1.15  2010/08/06 22:40:42Z  glenns
    Add declaration of NativePageRead interface function.
    Revision 1.14  2009/10/14 00:48:47Z  keithg
    Added BENDSWAP from the now obsolete FXIO.H module.
    Revision 1.13  2009/07/24 23:18:17Z  garyp
    Merged from the v4.0 branch.  Added support for IORequest().
    Revision 1.12  2009/04/09 02:29:11Z  garyp
    Renamed the public functions so they fit smoothly into the automated
    documentation system.  No other functional changes.
    Revision 1.11  2008/03/23 20:38:54Z  Garyp
    Prototype updates.
    Revision 1.10  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.9  2007/02/13 22:28:16Z  timothyj
    Changed 'ulPage' parameter to some NTM functions to 'ulStartPage' for
    consistency.  Changed D_UINT16 uCount parameters to D_UIN32 ulCount, to
    allow the call tree all the way up through the IoRequest to avoid having
    to range check (and/or split) requests.  Removed corresponding casts.
    Revision 1.8  2007/02/05 15:38:53Z  timothyj
    Updated interfaces to use blocks and pages instead of linear byte offsets.
    Revision 1.7  2006/11/08 18:01:29Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.6  2006/03/05 03:23:55Z  Garyp
    Added RawPageRead/Write() support.
    Revision 1.5  2006/02/26 02:47:51Z  Garyp
    Prototype updates.
    Revision 1.4  2006/02/17 23:37:38Z  Garyp
    Added a typedef.
    Revision 1.3  2006/02/08 19:43:58Z  Garyp
    Updated to no longer use the EXTMEDIAINFO structure.
    Revision 1.2  2006/01/11 02:14:54Z  Garyp
    Eliminated support for IsWriteProtected() and GetChipInfo().
    Revision 1.1  2005/12/02 01:16:30Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef NTM_H_INCLUDED
#define NTM_H_INCLUDED

#include <fimdev.h>


/*  Swaps byte order of a 16-bit unsigned int on Big Endian sytems to
    make output match hardware expectations of Little Endian input.

*/
#if !DCL_BIG_ENDIAN

#define BENDSWAP16(x)   (x)
#else

/*  An unsafe macro in want of a ROTATE operator...
*/
#define BENDSWAP16(x)   ((D_UINT16)((((x) >> 8) & 0x00FF) | ((D_UINT16)(x) << 8)))
#endif


typedef struct tagNTMDATA NTMDATA;

#ifdef NEW_NTM_FUNCTION_NAMES

static NTMHANDLE    FfxNtmCreate(FFXDEVHANDLE hDev, const NTMINFO **ppNtmInfo);
static void         FfxNtmDestroy(         NTMHANDLE hNTM);
static FFXIOSTATUS  FfxNtmPageRead(        NTMHANDLE hNTM, D_UINT32 ulStartPage, D_BUFFER *pBuffer, D_BUFFER *pSpare, D_UINT32 ulCount, unsigned nTagWidth, D_BOOL fUseEcc);
static FFXIOSTATUS  FfxNtmPageWrite(       NTMHANDLE hNTM, D_UINT32 ulStartPage, const D_BUFFER *pBuffer, const D_BUFFER *pTags, D_UINT32 ulCount, unsigned nTagWidth, D_BOOL fUseEcc);
static FFXIOSTATUS  FfxNtmRawPageRead(     NTMHANDLE hNTM, D_UINT32 ulStartPage, D_BUFFER *pBuffer, D_BUFFER *pSpare, D_UINT32 ulCount);
static FFXIOSTATUS  FfxNtmRawPageWrite(    NTMHANDLE hNTM, D_UINT32 ulStartPage, const D_BUFFER *pBuffer, const D_BUFFER *pSpare, D_UINT32 ulCount);
static FFXIOSTATUS  FfxNtmTagRead(         NTMHANDLE hNTM, D_UINT32 ulStartPage, D_UINT32 ulCount, D_BUFFER *pcHidden, unsigned nTagWidth);
static FFXIOSTATUS  FfxNtmTagWrite(        NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pTags, unsigned nTagWidth);
static FFXIOSTATUS  FfxNtmSpareRead(       NTMHANDLE hNTM, D_UINT32 ulPage, D_BUFFER *pSpare);
static FFXIOSTATUS  FfxNtmSpareWrite(      NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pSpare);
static FFXIOSTATUS  FfxNtmBlockErase(      NTMHANDLE hNTM, D_UINT32 ulBlock);
static FFXIOSTATUS  FfxNtmGetPageStatus(   NTMHANDLE hNTM, D_UINT32 ulPage);
static FFXIOSTATUS  FfxNtmGetBlockStatus(  NTMHANDLE hNTM, D_UINT32 ulBlock);
static FFXIOSTATUS  FfxNtmSetBlockStatus(  NTMHANDLE hNTM, D_UINT32 ulBlock, D_UINT32 ulBlockStatus);
static FFXSTATUS    FfxNtmParameterGet(    NTMHANDLE hNTM, FFXPARAM id, void *pBuffer, D_UINT32 ulBuffLen);
static FFXSTATUS    FfxNtmParameterSet(    NTMHANDLE hNTM, FFXPARAM id, const void *pBuffer, D_UINT32 ulBuffLen);
static FFXIOSTATUS  FfxNtmIORequest(       NTMHANDLE hNTM, FFXIOREQUEST *pIOR);

#else

/*  NOTE: Identical to the previous list, but without the "FfxNtm" prefix.
          Once all the NTMs have been updated to use the new name, this
          list can go away.

    NOTE: "Hidden" was renamed to "Tag" in the first list.  NTMs
          adapted to the new names need to change HiddenRead/Write()
          to FfxNtmTagRead/Write().
*/
static NTMHANDLE    Create(FFXDEVHANDLE hDev, const NTMINFO **ppNtmInfo);
static void         Destroy(         NTMHANDLE hNTM);
static FFXIOSTATUS  PageRead(        NTMHANDLE hNTM, D_UINT32 ulStartPage, D_BUFFER *pBuffer, D_BUFFER *pSpare, D_UINT32 ulCount, unsigned nTagWidth, D_BOOL fUseEcc);
static FFXIOSTATUS  PageWrite(       NTMHANDLE hNTM, D_UINT32 ulStartPage, const D_BUFFER *pBuffer, const D_BUFFER *pTags, D_UINT32 ulCount, unsigned nTagWidth, D_BOOL fUseEcc);
static FFXIOSTATUS  RawPageRead(     NTMHANDLE hNTM, D_UINT32 ulStartPage, D_BUFFER *pBuffer, D_BUFFER *pSpare, D_UINT32 ulCount);
static FFXIOSTATUS  RawPageWrite(    NTMHANDLE hNTM, D_UINT32 ulStartPage, const D_BUFFER *pBuffer, const D_BUFFER *pSpare, D_UINT32 ulCount);
static FFXIOSTATUS  HiddenRead(      NTMHANDLE hNTM, D_UINT32 ulStartPage, D_UINT32 ulCount, D_BUFFER *pcHidden, unsigned nTagWidth);
static FFXIOSTATUS  HiddenWrite(     NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pTags, unsigned nTagWidth);
static FFXIOSTATUS  SpareRead(       NTMHANDLE hNTM, D_UINT32 ulPage, D_BUFFER *pSpare);
static FFXIOSTATUS  SpareWrite(      NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pSpare);
static FFXIOSTATUS  BlockErase(      NTMHANDLE hNTM, D_UINT32 ulBlock);
static FFXIOSTATUS  GetPageStatus(   NTMHANDLE hNTM, D_UINT32 ulPage);
static FFXIOSTATUS  GetBlockStatus(  NTMHANDLE hNTM, D_UINT32 ulBlock);
static FFXIOSTATUS  SetBlockStatus(  NTMHANDLE hNTM, D_UINT32 ulBlock, D_UINT32 ulBlockStatus);
static FFXSTATUS    ParameterGet(    NTMHANDLE hNTM, FFXPARAM id, void *pBuffer, D_UINT32 ulBuffLen);
static FFXSTATUS    ParameterSet(    NTMHANDLE hNTM, FFXPARAM id, const void *pBuffer, D_UINT32 ulBuffLen);
static FFXIOSTATUS  IORequest(       NTMHANDLE hNTM, FFXIOREQUEST *pIOR);

#endif

#endif  /* NTM_H_INCLUDED */

