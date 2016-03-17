/*---------------------------------------------------------------------------
           Copyright (c) 1988-2002 Modular Software Systems, Inc.
                            All Rights Reserved.
---------------------------------------------------------------------------*/

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

    This header defines internal interfaces used by the viewer abstraction.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: winview.h $
    Revision 1.2  2009/06/28 02:46:24Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.3  2008/11/05 01:20:46Z  garyp
    Documentation updated.
    Revision 1.1.1.2  2008/08/25 18:00:39Z  garyp
    Added fields to facilitate caret support.
    Revision 1.1  2008/07/22 19:51:20Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

typedef struct sVIEWLINECACHE VIEWLINECACHE;


typedef struct sVIEWINST
{
    HWND            hWnd;
    D_UINT32        ulFlags;            /* Shared flags */
    ULONG           ulTopLine;
    UINT            uHorizOffset;
    UINT            uLinesPerPage;
    UINT            uTextHeight;
    HFONT           hFont;
    BYTE           *pTempDiskBuff;
    BYTE           *pTempLineBuff;
    BYTE           *pDisplayBuff;
    ULONG           ulDataSize;
    UINT            nPageWidth;
    UINT            nPageHeight;

    /*  LSA fields
    */
    HGLOBAL         hLSA;
    D_BUFFER       *pLSA;
    ULONG           ulLSAMaxCount;
    ULONG           ulLSACountUsed;
    ULONG           ulLSASize;

    /*  LDC fields
    */
    HGLOBAL         hLDC;
    BYTE           *pLDC;
    ULONG           ulLDCBufferSize;
    ULONG           ulLDCHeadOffset;
    ULONG           ulLDCTailOffset;

    /*  LCA fields
    */
    HGLOBAL         hLCA;
    VIEWLINECACHE  *pLCA;
    ULONG           ulLCAMaxEntries;
    ULONG           ulLCAHeadEntry;
    ULONG           ulLCAFirstLine;
    ULONG           ulLCALastLine;

    ULONG           ulLastTopLine;
    UINT            uLastHorizOffset;
    UINT            uLastLinesPerPage;
    UINT            uTabLen;
    UINT            uMaxDisplayLineLength;
/*    unsigned        fDataReadComplete : 1; */
    unsigned        fDynamicCacheSize : 1;
    unsigned        fNewLineFocus : 1;
    unsigned        nX;
    unsigned        nY;
} VIEWINST;



