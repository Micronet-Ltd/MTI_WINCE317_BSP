/*---------------------------------------------------------------------------
           Copyright (c) 1988-2002 Modular Software Systems, Inc.
                            All Rights Reserved.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

    This module contains the core functions that implement the heart of the
    Win32 viewer abstraction.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: winviewcore.c $
    Revision 1.3  2010/01/23 16:45:50Z  garyp
    Fixed to avoid "possible loss of data" warnings (no actual loss of data).
    Revision 1.2  2009/06/28 02:46:25Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.5  2008/11/05 01:20:48Z  garyp
    Documentation updated.
    Revision 1.1.1.4  2008/09/01 17:49:46Z  garyp
    Modified to use DclMemSet() rather than memset().
    Revision 1.1.1.3  2008/08/08 17:10:00Z  garyp
    Now use DrawText() rather than TextOut() to allow CE compatibility.
    Revision 1.1.1.2  2008/07/27 23:23:33Z  garyp
    Commented out CE specific code so we can build again for XP.
    Revision 1.1  2008/07/27 02:56:22Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <string.h>
#include <limits.h>

#include <dcl.h>
#include <winviewapi.h>
#include "winview.h"

#define  WVMAXLINELEN           (0x7FFF)    /* Max disaplayable length, anything longer is truncated */
#define  WVMAXPAGEWIDTH           (2048)
#define  WVAVGLINELEN               (60)
#define  WVMINMEMBUFFERSIZE       (4096U)
#define  WVDEFAULTMEMBUFFERSIZE  (65535U)
#define  WVMINLINEBUFFCOUNT        (100)
#define  WVDATAREADCHUNKSIZE      (4096)

#define  WVMAXLCAENTRYCOUNT     (ULONG_MAX / sizeof(VIEWLINECACHE))

#define  TRUNCATE_FLAG          (0x8000)

typedef struct
{
    ULONG       ulReadOffset;   /* Physical offset in the stream */
    unsigned    nLineLength;    /* Field should be eliminated -- just compare offsets for the previous and next record */
} VIEWLINESTATE;

struct sVIEWLINECACHE
{
    ULONG       ulOffset;
    D_UINT16    uLength;
};


#define ASC_LF  10
#define ASC_CR  13
#define ASC_TAB  9

static BOOL             AllocInstResources(     VIEWINST *pVI, ULONG ulCacheSize);
static void             DisplayPage(            VIEWINST *pVI, HWND hWnd);
static void             ReleaseInstResources(   VIEWINST *pVI);
static ULONG            PositionToNextLine(     VIEWINST *pVI, ULONG ulLine);
static void             CalcDisplayLineLength(  VIEWINST *pVI, D_BUFFER *pData, UINT uLen);
static BOOL             SaveLSARecord(          VIEWINST *pVI, ULONG ulLine, VIEWLINESTATE *pLS);
static void             RetrieveLSARecord(      VIEWINST *pVI, ULONG ulLine, VIEWLINESTATE *pLS);
static unsigned         GetTextLineSeg(         VIEWINST *pVI, D_BUFFER *pOutputBuff, ULONG ulLineNum, UINT uSkip, UINT uOutputLen);
static ULONG            CalcFreeLDCSpace(       VIEWINST *pVI);
static void             DeleteLDCLine(          VIEWINST *pVI, ULONG ulLine);
static ULONG            GetLineData(            VIEWINST *pVI, ULONG ulLineNum, UINT *puLength);
static void             AddLineToLDC(           VIEWINST *pVI, ULONG ulLine, unsigned nLen, D_BUFFER *pLineData);
static void             WriteLDCRecord(         VIEWINST *pVI, ULONG ulOffset, D_BUFFER *pLineData, UINT uRecordLen);
static ULONG            CalcLDCTailWriteOffset( VIEWINST *pVI, UINT uLen);
static ULONG            CalcLDCHeadWriteOffset( VIEWINST *pVI, UINT uLen);
static BOOL             ExpandLSA(              VIEWINST *pVI, ULONG ulNewMin);
static BOOL             ExpandLCA(              VIEWINST *pVI);
static void             AddLCARecord(           VIEWINST *pVI, ULONG ulLine, ULONG ulOffset, D_UINT16 uLen);
static ULONG            CalcLCARecordIndex(     VIEWINST *pVI, ULONG ulLine);
static void             LoadMultipleLines(      VIEWINST *pVI, ULONG ulNumLines);
static VIEWLINECACHE *  GetLCARecord(           VIEWINST *pVI, ULONG ulLine);
static BOOL             ValidateDataStructures( VIEWINST *pVI);
static unsigned         CalcAbsoluteLineLength(D_BUFFER *pBuff, unsigned nSize, unsigned fComplete);


            /*-------------------------------------------------*\
             *                                                 *
             *                Public Interface                 *
             *                                                 *
            \*-------------------------------------------------*/


/*-------------------------------------------------------------------
    Public: DclWinViewCreate()

    This function creates a view instance.

    Parameters:
        hWnd        - The window handle.
        ulFlags     - The flags to use, which may be one or more of
                      the following values:
                      VIEWFLAGS_DYNAMIC - Indicates that the data
                      is dynamically growing, and that the cursor
                      focus should be maintained at the end of the
                      data stream.
        uTabLen     - The number of characters which a hard tab
                      should represent.
        ulCacheSize - The size of the cache buffer to use when
                      reading data.

    Returns:
        Returns an HVIEW handle if successful, or NULL otherwise.
-------------------------------------------------------------------*/
HVIEW WINAPI DclWinViewCreate(
    HWND        hWnd,
    D_UINT32    ulFlags,
    UINT        uTabLen,
    ULONG       ulCacheSize)
{
    VIEWINST   *pVI;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, TRACEINDENT),
        "DclWinViewCreate() hWnd=%P Flags=%lX TabLen=%u CacheSize=%lX\n",
        hWnd, ulFlags, uTabLen, ulCacheSize));

    DclAssert(hWnd);

    pVI = DclMemAllocZero(sizeof(*pVI));
    if(pVI)
    {
        pVI->hWnd               = hWnd;
        pVI->ulTopLine          = 1;
        pVI->ulLastTopLine      = ULONG_MAX;
        pVI->uLastHorizOffset   = UINT_MAX;
        pVI->uLastLinesPerPage  = UINT_MAX;
        pVI->ulFlags            = ulFlags;
        pVI->uTabLen            = uTabLen;

        if(ulFlags & VIEWFLAGS_DYNAMIC)
            pVI->fNewLineFocus = TRUE;

        pVI->pTempDiskBuff = DclMemAlloc(WVDATAREADCHUNKSIZE);
        if(pVI->pTempDiskBuff)
        {
            pVI->pTempLineBuff = DclMemAlloc(WVMAXLINELEN);
            if(pVI->pTempLineBuff)
            {
                pVI->pDisplayBuff = DclMemAlloc(WVMAXPAGEWIDTH);
                if(pVI->pDisplayBuff)
                {
                    if(AllocInstResources(pVI, ulCacheSize))
                        goto CreateCleanup;
                }
            }
        }
    }

/* ErrorCleanup: */
    if(pVI->pTempDiskBuff)
        DclMemFree(pVI->pTempDiskBuff);

    if(pVI->pTempLineBuff)
        DclMemFree(pVI->pTempLineBuff);

    if(pVI->pDisplayBuff)
        DclMemFree(pVI->pDisplayBuff);

    DclMemFree(pVI);

    pVI = NULL;

  CreateCleanup:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, TRACEUNDENT),
        "DclWinViewCreate() returning %P\n", pVI));

    return (HVIEW)pVI;
}


/*-------------------------------------------------------------------
    Public: DclWinViewDestroy()

    This function destroys a view instance.

    Parameters:
        hView       - The view handle.

    Returns:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
BOOL WINAPI DclWinViewDestroy(
    HVIEW       hView)
{
    VIEWINST   *pVI = (VIEWINST*)hView;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
        "DclWinViewDestroy() hView=%P\n", hView));

    if(!pVI)
        return FALSE;

    DclAssert(ValidateDataStructures(pVI));

    /*  Release resources in reverse order of allocation
    */

    ReleaseInstResources(pVI);

    if(pVI->pTempDiskBuff)
        DclMemFree(pVI->pTempDiskBuff);

    if(pVI->pTempLineBuff)
        DclMemFree(pVI->pTempLineBuff);

    if(pVI->pDisplayBuff)
        DclMemFree(pVI->pDisplayBuff);

    DclMemFree(pVI);

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: DclWinViewPaint()

    This function updates the view window.

    Parameters:
        hView       - The view handle.
        hWnd        - The window handle.

    Returns:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
BOOL WINAPI DclWinViewPaint(
    HVIEW       hView,
    HWND        hWnd)
{
    VIEWINST   *pVI = (VIEWINST*)hView;

    DclAssert(hWnd);

    if(!pVI)
        return FALSE;

    DclAssert(ValidateDataStructures(pVI));

    DisplayPage(pVI, hWnd);

    return TRUE;
}


            /*-------------------------------------------------*\
             *                                                 *
             *                 Data Management                 *
             *                                                 *
            \*-------------------------------------------------*/


/*-------------------------------------------------------------------
    Returns the length, or UINT_MAX if we've reached the EOF.
-------------------------------------------------------------------*/
static unsigned GetTextLineSeg(
    VIEWINST   *pVI,
    D_BUFFER   *pOutputBuff,
    ULONG       ulLineNum,
    UINT        uSkip,
    UINT        uOutputLen)
{
    UINT        uInputLen;
    unsigned    nRetCount = 0;
    unsigned    nAbsOffset = 0;
    ULONG       ulOffset;
    D_BUFFER   *pInputBuff;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewCore:GetTextLineSeg() Line=%lu Skip=%u OutputLen=%u\n", ulLineNum, uSkip, uOutputLen));

    DclAssert(pVI);
    DclAssert(pOutputBuff);
    DclAssert(ulLineNum);
    DclAssert(uOutputLen);
    DclAssert(uSkip <= WVMAXLINELEN);
    DclAssert(ValidateDataStructures(pVI));

    if(/*pVI->fDataReadComplete &&*/ (ulLineNum > pVI->ulLSACountUsed))
       return UINT_MAX;

    ulOffset = GetLineData(pVI, ulLineNum, &uInputLen);
    if(ulOffset == ULONG_MAX)
       return UINT_MAX;

    uInputLen &= ~TRUNCATE_FLAG;

    DclAssert(pVI->ulLDCBufferSize > ulOffset);

    pInputBuff = pVI->pLDC + ulOffset;

    while(uInputLen && uOutputLen)
    {
        char  ch;

        DclAssert(ulOffset <= pVI->ulLDCBufferSize);

        if(ulOffset == pVI->ulLDCBufferSize)
        {
            /*  Wrap around to accommodate lines which are split between
                the head and tail of the buffer.
            */
            pInputBuff = pVI->pLDC;
            ulOffset = 0;
        }

        ch = *pInputBuff;

        if((ch == ASC_LF) || (ch == ASC_CR))
            break;

        if(ch == ASC_TAB && pVI->uTabLen)
        {
            UINT  uLength;

            uLength = pVI->uTabLen - (nAbsOffset % pVI->uTabLen);

            while(uLength && uOutputLen)
            {
                if(uSkip)
                {
                    uSkip--;
                }
                else
                {
                    *pOutputBuff = ' ';

                    pOutputBuff++;
                    uOutputLen--;
                    nRetCount++;
                }

                nAbsOffset++;
                uLength--;
            }

            pInputBuff++;
            uInputLen--;
            ulOffset++;

            continue;
        }

        if(uSkip)
        {
            uSkip--;
        }
        else
        {
            *pOutputBuff = ch;

            pOutputBuff++;
            uOutputLen--;

            nRetCount++;
        }

        nAbsOffset++;
        uInputLen--;
        pInputBuff++;
        ulOffset++;
    }

    return nRetCount;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static ULONG GetLineData(
    VIEWINST       *pVI,
    ULONG           ulLineNum,
    UINT           *puLength)
{
    VIEWLINECACHE  *pLC;

    DclAssert(pVI);
    DclAssert(ulLineNum);
    DclAssert(puLength);
    DclAssert(ValidateDataStructures(pVI));

    if(ulLineNum > pVI->ulLSACountUsed)
    {
        /*  If the line has not yet been read, read as many lines into
            the LSA as necessary until our target line is loaded.
        */
        LoadMultipleLines(pVI, ulLineNum - pVI->ulLSACountUsed);

        if(ulLineNum > pVI->ulLSACountUsed)
            return ULONG_MAX;
    }

    pLC = GetLCARecord(pVI, ulLineNum);

    *puLength = pLC->uLength;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewCore:GetLineData() Line=%lu returning Offset=%lu Len=%u\n",
        ulLineNum, pLC->ulOffset, pLC->uLength));

    return pLC->ulOffset;
}


/*-------------------------------------------------------------------
    Get the absolute line length, without expanding tabs, including
    the trailing CR and LF characters.

    It can handle handle lines that end with any combination of CR
    and LF characters, however, if a single CR or LF is encountered
    at the end of the available buffer, zero will be returned since
    the function cannot tell if a second terminator follows.

    Returns the line length, or zero if it could not be determined.
-------------------------------------------------------------------*/
static unsigned CalcAbsoluteLineLength(
    D_BUFFER   *pBuff,
    unsigned    nSize,
    unsigned    fComplete)
{
    unsigned    nLength = 0;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 3, 0),
        "ViewCore:CalcAbsoluteLineLength() P=%P Size=%u Complete=%u\n", pBuff, nSize, fComplete));

    DclAssert(pBuff);
    DclAssert(nSize);

    while(nSize)
    {
        nLength++;
        DclAssert(nLength);

        if(*pBuff == ASC_CR)
        {
            if(nSize == 1)
            {
                if(fComplete)
                    return nLength;
                else
                    return 0;
            }

            if(*(pBuff+1) == ASC_LF)
                return nLength+1;
            else
                return nLength;
        }

        if(*pBuff == ASC_LF)
        {
            if(nSize == 1)
            {
                if(fComplete)
                    return nLength;
                else
                    return 0;
            }

            if(*(pBuff+1) == ASC_CR)
                return nLength+1;
            else
                return nLength;
        }

        pBuff++;
        nSize--;
    }

    /*  Didn't find any terminators with certainty.  If we have read
        all the data, then just return the full length, otherwise
        return 0.
    */
    if(fComplete)
        return nLength;
    else
        return 0;
}


/*-------------------------------------------------------------------
    Read the next n lines.  This routine may read more lines than
    specified depending on the buffer sizes used and the relative
    location of the line breaks.

    Returns nothing.
-------------------------------------------------------------------*/
static void LoadMultipleLines(
    VIEWINST   *pVI,
    ULONG       ulNumLines)
{
    ULONG       ulReadOffset;
    D_BUFFER   *pBuff;
    unsigned    nRawSize;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, TRACEINDENT),
        "ViewCore:LoadMultipleLines() pVI=%P Count=%lU\n", pVI, ulNumLines));

    DclAssert(pVI);
    DclAssert(ulNumLines > 0);
/*    DclAssert(!pVI->fDataReadComplete); */
    DclAssert(ValidateDataStructures(pVI));

    if(pVI->ulLSACountUsed)
    {
        DclAssert(pVI->ulLSACountUsed >= pVI->ulLCALastLine);

        if(pVI->ulLSACountUsed == pVI->ulLCALastLine)
            DeleteLDCLine(pVI, pVI->ulLSACountUsed);

        pVI->ulLSACountUsed--;
    }

    ulReadOffset = PositionToNextLine(pVI, pVI->ulLSACountUsed);

    pBuff = pVI->pTempLineBuff;

    nRawSize = 0;

    while(/*!pVI->fDataReadComplete &&*/ ulNumLines)
    {
        unsigned    nBytesRead;
        unsigned    nChunkSize;
        unsigned    nBias;
        unsigned    fReadEverything = FALSE;

        DclAssert(WVMAXLINELEN > nRawSize);

        nBytesRead = SendMessage(pVI->hWnd, WM_WINVIEW_READDATA, WVMAXLINELEN - nRawSize, (LPARAM)pBuff + nRawSize);
        DclAssert(nBytesRead != UINT_MAX);

        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
            "ViewCore:LML() Read of %lu bytes returned %lU bytes\n", WVMAXLINELEN - nRawSize, nBytesRead));

        if(!nBytesRead)
            break;

        if(nBytesRead != (WVMAXLINELEN - nRawSize))
            fReadEverything = TRUE;

        nRawSize += nBytesRead;

        nBias = 0;

        nChunkSize = nRawSize;

        while(nRawSize)
        {
            unsigned        nLength;
            VIEWLINESTATE   ls;

            nLength = CalcAbsoluteLineLength(pBuff + nBias, nRawSize, fReadEverything);

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 3, 0),
                "ViewCore:LML() RawSize=%u Bias=%u AbsLen=%u\n", nRawSize, nBias, nLength));

            DclAssert(nLength <= WVMAXLINELEN);

            if(nLength == 0)
            {
                /*  If the length could not be determined and we have
                    a max length block in the buffer, we must write a
                    record indicating the line is truncated.
                */
                if(nBias == 0 && nRawSize == WVMAXLINELEN)
                {
                    ls.ulReadOffset = ulReadOffset;
                    ls.nLineLength = nRawSize;

                    if(nRawSize == WVMAXLINELEN)
                        ls.nLineLength |= TRUNCATE_FLAG;

                    pVI->ulLSACountUsed++;
                    DclAssert(pVI->ulLSACountUsed);

                    CalcDisplayLineLength(pVI, pBuff, nRawSize);

                    SaveLSARecord(pVI, pVI->ulLSACountUsed, &ls);

                    AddLineToLDC(pVI, pVI->ulLSACountUsed, ls.nLineLength, pBuff);

                    if(ulNumLines)
                        ulNumLines--;

                    /*  If we're at this position, but the file read is
                        not yet complete, it means that we just added a
                        line that was too long to handle.  We must seek
                        to the next line so that this routine can properly
                        continue at the top of the outer loop.
                    */
                    ulReadOffset = PositionToNextLine(pVI, pVI->ulLSACountUsed);

                    nRawSize = 0;

                    /*  Break out of the inner loop as we've processed
                        everything in the buffer.
                    */
                    break;
                }
                else
                {
                    /*  We get here if we could not determine the line
                        length, but we don't know whether it is a record
                        that needs to be truncated until we read more
                        data.  Break out of the inner loop and read more
                        data in the outer loop.
                    */
                    break;
                }
            }

            /*  Otherwise, we've got a normal line
            */
            ls.ulReadOffset = ulReadOffset;
            ls.nLineLength = nLength;

            pVI->ulLSACountUsed++;

            CalcDisplayLineLength(pVI, pBuff+nBias, nLength);

            SaveLSARecord(pVI, pVI->ulLSACountUsed, &ls);

            AddLineToLDC(pVI, pVI->ulLSACountUsed, nLength, pBuff+nBias);

            nRawSize     -= nLength;
            nBias        += nLength;
            ulReadOffset += nLength;

            if(ulNumLines)
                ulNumLines--;
        }

        /*  If we've processed at least as many lines as was requested,
            quit now.
        */
/*        if(ulNumLines == 0)
            break;*/

        if(nRawSize)
        {
            /*  If there is raw data remaining, move it to the beginning of
                the buffer.
            */
            memmove(pBuff, pBuff + (nChunkSize - nRawSize), nRawSize);
        }
    }

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, TRACEUNDENT),
        "ViewCore:LoadMultipleLines() returning\n"));

    return;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static ULONG SetAbsoluteStreamPosition(
    VIEWINST   *pVI,
    ULONG       ulPos)
{
    ULONG       ulResult;

    DclAssert(pVI);
    DclAssert(ValidateDataStructures(pVI));

    ulResult = (ULONG)SendMessage(pVI->hWnd, WM_WINVIEW_SETPOSITION, 0, ulPos);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewCore:SetAbsoluteStreamPosition() Pos=%lu returning %lu\n", ulPos, ulResult));

    return ulResult;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static ULONG SetRelativeStreamPosition(
    VIEWINST   *pVI,
    LONG        lPos)
{
    ULONG       ulResult;

    DclAssert(pVI);
    DclAssert(ValidateDataStructures(pVI));

    ulResult = (ULONG)SendMessage(pVI->hWnd, WM_WINVIEW_RELATIVEPOSITION, 0, lPos);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewCore:SetRelativeStreamPosition() Pos=%ld returning %lu\n", lPos, ulResult));

    return ulResult;
}


/*-------------------------------------------------------------------
    This function sets the current location in the raw data stream
    to be positioned at the specified line.  Essentially this is a
    virtual seek() -- and could result in a literal seek() operation
    depending on where the raw data is coming from.

    The position is set such that it follows the specified line.
    Therefore if ulLine is '0', the position is set to the start
    of the data stream (or the start of line 1).  If it is set to
    '1', it is set to the start of line 2, etc.
-------------------------------------------------------------------*/
static ULONG PositionToNextLine(
    VIEWINST       *pVI,
    ULONG           ulLine)
{
    VIEWLINESTATE   ls;

    DclAssert(pVI);
    DclAssert(ValidateDataStructures(pVI));

    /*  If zero is specified, then we want to seek to the beginning of the data
    */
    if(!ulLine)
        return SetAbsoluteStreamPosition(pVI, 0);

    RetrieveLSARecord(pVI, ulLine, &ls);

    if(!(ls.nLineLength & TRUNCATE_FLAG))
    {
        /*  If this line has not been truncated due to excessive length,
            simply set the position to follow it.
        */
        return SetAbsoluteStreamPosition(pVI, ls.ulReadOffset + ls.nLineLength);
    }

    DclAssert(ls.nLineLength == (WVMAXLINELEN & TRUNCATE_FLAG));

    SetAbsoluteStreamPosition(pVI, ls.ulReadOffset + WVMAXLINELEN);

    while(TRUE)
    {
        unsigned    nSizeRead;
        unsigned    nAbsLen;
        unsigned    fReadEverything = FALSE;

        nSizeRead = SendMessage(pVI->hWnd, WM_WINVIEW_READDATA, WVDATAREADCHUNKSIZE, (LPARAM)pVI->pTempDiskBuff);
        DclAssert(nSizeRead != UINT_MAX);

        if(nSizeRead != WVDATAREADCHUNKSIZE)
            fReadEverything = TRUE;

        nAbsLen = CalcAbsoluteLineLength(pVI->pTempDiskBuff, nSizeRead, fReadEverything);
        if(nAbsLen != 0)
        {
            /*  Found the terminator characters, back the file pointer up
                to the point where it points to the next line.
            */
            return SetRelativeStreamPosition(pVI, -(long)(nSizeRead-nAbsLen));
         }

        /*  If we found no terminator, but we've read all of the file,
            leave the file pointer at the end-of-file and return.
        */
        if(fReadEverything)
            break;

        /*  read another chunk
        */
    }

    /*  return the current position
    */
    return SetRelativeStreamPosition(pVI, 0);
}


            /*-------------------------------------------------*\
             *                                                 *
             *                Display Functions                *
             *                                                 *
            \*-------------------------------------------------*/


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void DisplayPage(
    VIEWINST   *pVI,
    HWND        hWnd)
{
    UINT        uTargLine;
    UINT        uNewHorOffset;
    unsigned    nLinesPerPage;
    ULONG       ulNewTop;

    DclAssert(pVI);
    DclAssert(hWnd);
    DclAssert(ValidateDataStructures(pVI));

    nLinesPerPage = SendMessage(pVI->hWnd, WM_WINVIEW_GETLINESPERPAGE, 0, 0L);

    uTargLine = nLinesPerPage;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, TRACEINDENT),
        "ViewCore:DisplayPage() TopLine=%lu TargLine=%u LSACount=%lu\n", pVI->ulTopLine, uTargLine, pVI->ulLSACountUsed));

    if(pVI->fNewLineFocus)
        ulNewTop = WINVIEW_MAXLINENUM;
    else
        ulNewTop = pVI->ulTopLine;

    if(ulNewTop + uTargLine > pVI->ulLSACountUsed)
    {
        LoadMultipleLines(pVI, (ulNewTop + uTargLine) - pVI->ulLSACountUsed);

        if(pVI->ulLSACountUsed > nLinesPerPage)
            ulNewTop = (pVI->ulLSACountUsed - nLinesPerPage) + 1;
        else
            ulNewTop = 1;

        if(ulNewTop + uTargLine > pVI->ulLSACountUsed)
            pVI->fNewLineFocus = TRUE;
    }

    uNewHorOffset = pVI->uHorizOffset;

    if((ulNewTop            == pVI->ulLastTopLine)      &&
       (uNewHorOffset       == pVI->uLastHorizOffset)   &&
       (nLinesPerPage       == pVI->uLastLinesPerPage))
    {
        pVI->ulTopLine      = pVI->ulLastTopLine;
        pVI->uHorizOffset   = pVI->uLastHorizOffset;
        pVI->uLinesPerPage  = pVI->uLastLinesPerPage;

/*        return; */
    }

    {
        PAINTSTRUCT ps;

        pVI->ulTopLine         = ulNewTop;
        pVI->ulLastTopLine     = ulNewTop;
        pVI->uHorizOffset      = uNewHorOffset;
        pVI->uLastHorizOffset  = uNewHorOffset;
        pVI->uLinesPerPage     = nLinesPerPage;
        pVI->uLastLinesPerPage = nLinesPerPage;

        BeginPaint(hWnd, &ps);
        {
            UINT    uLineOnPage;        /* relative to 0 */

            SelectObject(ps.hdc, pVI->hFont);

            for(uLineOnPage=0; uLineOnPage<uTargLine; uLineOnPage++)
            {
                UINT    uTextLength;

                uTextLength = GetTextLineSeg(pVI, pVI->pDisplayBuff, pVI->ulTopLine+uLineOnPage, pVI->uHorizOffset, WVMAXPAGEWIDTH);
                if(uTextLength == UINT_MAX)
                    break;

                if(uTextLength)
                {
                    WCHAR       tzText[WVMAXPAGEWIDTH];
                    unsigned    nWidth = DCLMIN(uTextLength, WVMAXPAGEWIDTH);
                    RECT        rc;
                    SIZE        siz;
                    unsigned    nChars;

                    DclOsAnsiToWcs(tzText, DCLDIMENSIONOF(tzText), pVI->pDisplayBuff, nWidth);

                    GetTextExtentExPoint(ps.hdc, tzText, nWidth,
                        pVI->nPageWidth, &nChars, NULL, &siz);

                    rc.left = 0;
                    rc.top = uLineOnPage * pVI->uTextHeight;
                    rc.right = pVI->nPageWidth-1;
                    rc.bottom = rc.top + pVI->uTextHeight;
                    DrawText(ps.hdc, tzText, nWidth, &rc, DT_SINGLELINE);

                    pVI->nX = siz.cx;
                    pVI->nY = uLineOnPage;
                }
                else
                {
                    pVI->nX = 0;
                    pVI->nY = uLineOnPage;
                }
            }
        }
        EndPaint(hWnd, &ps);
    }

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, TRACEUNDENT),
        "ViewCore:DisplayPage() returning\n"));

    return;
}


/*-------------------------------------------------------------------
    This function calculates how many characters a given line will
    take to display, taking into account tab character expansion.

    The maximum width line length is recorded for later use.
-------------------------------------------------------------------*/
static void CalcDisplayLineLength(
    VIEWINST   *pVI,
    D_BUFFER   *pData,
    UINT        uLen)
{
    UINT        uDispLen = 0;

    DclAssert(pVI);
    DclAssert(pData);
    DclAssert(ValidateDataStructures(pVI));

    if(pVI->uTabLen)
    {
        while(uLen)
        {
            if(*pData == '\t')
            {
                uDispLen += pVI->uTabLen - (uDispLen % pVI->uTabLen);
            }
            else
            {
                uDispLen++;
            }

            uLen--;
            pData++;
        }
    }
    else
    {
        uDispLen = uLen;
    }

    if(uDispLen > pVI->uMaxDisplayLineLength)
        pVI->uMaxDisplayLineLength = uDispLen;

    return;
}


            /*-------------------------------------------------*\
             *                                                 *
             *                  LDC Management                 *
             *                                                 *
            \*-------------------------------------------------*/


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void MakeSpaceAtLDCTail(
    VIEWINST   *pVI,
    UINT        uLineLength)
{
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewCore:MakeSpaceAtLDCTail() Length=%u\n", uLineLength));

    DclAssert(pVI);
    DclAssert(uLineLength);
    DclAssert(uLineLength <= WVMAXLINELEN);
    DclAssert(ValidateDataStructures(pVI));

    while(uLineLength > CalcFreeLDCSpace(pVI))
        DeleteLDCLine(pVI, pVI->ulLCAFirstLine);

    /*  Both the LDC and the LCA must have room for the new record.  We
        confirmed the LDC space first (above) since that is more likely
        to be constricted.  If after getting the necessary LDC space,
        we are still maxed out in the LCA table, either expand the table
        (if possible) or delete one entry.
    */
    if((pVI->ulLCALastLine - pVI->ulLCAFirstLine) >= pVI->ulLCAMaxEntries-1)
    {
        if(pVI->ulLCAMaxEntries != WVMAXLCAENTRYCOUNT)
            ExpandLCA(pVI);
        else
            DeleteLDCLine(pVI, pVI->ulLCAFirstLine);
    }

    return;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void MakeSpaceAtLDCHead(
    VIEWINST   *pVI,
    UINT        uLineLength)
{
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewCore:MakeSpaceAtLDCHead() Length=%u\n", uLineLength));

    DclAssert(pVI);
    DclAssert(uLineLength);
    DclAssert(uLineLength <= WVMAXLINELEN);
    DclAssert(ValidateDataStructures(pVI));

    while(uLineLength > CalcFreeLDCSpace(pVI))
        DeleteLDCLine(pVI, pVI->ulLCALastLine);

    /*  Both the LDC and the LCA must have room for the new record.  We
        confirmed the LDC space first (above) since that is more likely
        to be constricted.  If after getting the necessary LDC space,
        we are still maxed out in the LCA table, either expand the table
        (if possible) or delete one entry.
    */
    if((pVI->ulLCALastLine - pVI->ulLCAFirstLine) >= pVI->ulLCAMaxEntries-1)
    {
        if(pVI->ulLCAMaxEntries != WVMAXLCAENTRYCOUNT)
            ExpandLCA(pVI);
        else
            DeleteLDCLine(pVI, pVI->ulLCALastLine);
    }

    return;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void AddLineToLDC(
    VIEWINST   *pVI,
    ULONG       ulLine,
    unsigned    nLen,
    D_BUFFER   *pLineData)
{
    D_UINT16    uRecordLen;
    ULONG       ulLDCOffset;

    DclAssert(pVI);
    DclAssert(pLineData);
    DclAssert(ulLine);
    DclAssert(ulLine < pVI->ulLCAFirstLine || ulLine > pVI->ulLCALastLine);
    DclAssert(nLen);
    DclAssert(ValidateDataStructures(pVI));

    uRecordLen = nLen & ~TRUNCATE_FLAG;

    DclAssert(uRecordLen <= WVMAXLINELEN);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewCore:AddLineToLDC() Line=%lu Len=%u RecLen=%u\n", ulLine, nLen, uRecordLen));

    if(pVI->ulLCAFirstLine && (ulLine == pVI->ulLCAFirstLine-1))
    {
        MakeSpaceAtLDCHead(pVI, uRecordLen);

        ulLDCOffset = CalcLDCHeadWriteOffset(pVI, uRecordLen);

        WriteLDCRecord(pVI, ulLDCOffset, pLineData, uRecordLen);

        pVI->ulLDCHeadOffset = ulLDCOffset;
    }
    else if(pVI->ulLCALastLine && (ulLine == pVI->ulLCALastLine+1))
    {
        ULONG    ulBigOffset;

        MakeSpaceAtLDCTail(pVI, uRecordLen);

        ulLDCOffset = CalcLDCTailWriteOffset(pVI, uRecordLen);

        WriteLDCRecord(pVI, ulLDCOffset, pLineData, uRecordLen);

        ulBigOffset = ulLDCOffset + uRecordLen;

        if(ulBigOffset < pVI->ulLDCBufferSize)
            pVI->ulLDCTailOffset = ulBigOffset;
        else
            pVI->ulLDCTailOffset = ulBigOffset - pVI->ulLDCBufferSize;
    }
    else
    {
        /*  We've been asked to add a line to the LDC which is not
            sequential with what is already in there.  Toss everything
            out and start fresh.
        */
        pVI->ulLCAFirstLine = 0;
        pVI->ulLCALastLine = 0;
        pVI->ulLCAHeadEntry = 0;

        pVI->ulLDCHeadOffset = 0;
        pVI->ulLDCTailOffset = 0;

        WriteLDCRecord(pVI, 0, pLineData, uRecordLen);

        pVI->ulLDCHeadOffset = 0;
        pVI->ulLDCTailOffset = uRecordLen;

        ulLDCOffset = 0;
    }

    DclAssert(nLen < D_UINT16_MAX);
    AddLCARecord(pVI, ulLine, ulLDCOffset, (D_UINT16)nLen);

    if(pVI->ulLDCTailOffset > pVI->ulLDCHeadOffset)
    {
        DclAssert(ulLDCOffset >= pVI->ulLDCHeadOffset);
        DclAssert(ulLDCOffset < pVI->ulLDCTailOffset);
    }
    else
    {
        DclAssert((ulLDCOffset >= pVI->ulLDCHeadOffset) ||
                  (ulLDCOffset < pVI->ulLDCTailOffset));
    }

    return;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void WriteLDCRecord(
    VIEWINST   *pVI,
    ULONG       ulOffset,
    D_BUFFER   *pLineData,
    UINT        uRecordLen)
{
    D_BUFFER   *pNewRec;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewCore:WriteLDCRecord() Offset=%lu Len=%u\n", ulOffset, uRecordLen));

    DclAssert(pVI);
    DclAssert(pLineData);
    DclAssert(uRecordLen);
    DclAssert(uRecordLen <= WVMAXLINELEN);
    DclAssert(ValidateDataStructures(pVI));

    pNewRec = pVI->pLDC + ulOffset;

    /*  Does the record wrap around the end of the buffer back to
        the beginning?
    */
    if(uRecordLen > pVI->ulLDCBufferSize - ulOffset)
    {
        ULONG  ulFirstLen = pVI->ulLDCBufferSize - ulOffset;

        /*  Write the first part at the tail...
        */
        memcpy(pNewRec, pLineData, ulFirstLen);

        /*  ...and the remainder at the head
        */
        memcpy(pVI->pLDC, pLineData + ulFirstLen, uRecordLen - ulFirstLen);
    }
    else
    {
        memcpy(pNewRec, pLineData, uRecordLen);
    }

    return;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static ULONG CalcLDCTailWriteOffset(
    VIEWINST   *pVI,
    UINT        uLen)
{
    DclAssert(pVI);
    DclAssert(uLen);
    DclAssert(uLen <= WVMAXLINELEN);
    DclAssert(uLen <= CalcFreeLDCSpace(pVI));
    DclAssert(ValidateDataStructures(pVI));

    uLen = uLen;            /* avoid compiler warning */

    return pVI->ulLDCTailOffset;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static ULONG CalcLDCHeadWriteOffset(
    VIEWINST   *pVI,
    UINT        uLen)
{
    DclAssert(pVI);
    DclAssert(uLen);
    DclAssert(uLen <= WVMAXLINELEN);
    DclAssert(uLen <= CalcFreeLDCSpace(pVI));
    DclAssert(ValidateDataStructures(pVI));

    if(pVI->ulLDCHeadOffset >= uLen)
        return pVI->ulLDCHeadOffset - uLen;
    else
        return pVI->ulLDCBufferSize - (uLen - pVI->ulLDCHeadOffset);
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static ULONG CalcFreeLDCSpace(
    VIEWINST   *pVI)
{
    ULONG       ulFree;

    DclAssert(pVI);
    DclAssert(ValidateDataStructures(pVI));

    if(pVI->ulLDCHeadOffset <= pVI->ulLDCTailOffset)
        ulFree = pVI->ulLDCBufferSize - (pVI->ulLDCTailOffset - pVI->ulLDCHeadOffset);
    else
        ulFree = pVI->ulLDCBufferSize - ((pVI->ulLDCBufferSize - pVI->ulLDCHeadOffset) + pVI->ulLDCTailOffset);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewCore:CalcFreeLDCSpace() FreeSpace=%lu\n", ulFree));

    return ulFree;
}


/*-------------------------------------------------------------------
    Delete the specified line from the LDC.  Since LDC lines must
    be sequential, this routine may only be used to delete either
    the first line or the last line in the LDC.  The LCA pointers
    are adjusted accordingly.
-------------------------------------------------------------------*/
static void DeleteLDCLine(
    VIEWINST   *pVI,
    ULONG       ulLine)
{
    ULONG       ulIndex;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewCore:DeleteLDCLine() Line=%lu LDCFirst=%lu LDCLast=%lu\n",
        ulLine, pVI->ulLCAFirstLine, pVI->ulLCALastLine));

    DclAssert(pVI);
    DclAssert(ulLine);
    DclAssert(ulLine == pVI->ulLCAFirstLine || ulLine == pVI->ulLCALastLine);
    DclAssert(ValidateDataStructures(pVI));

    if(pVI->ulLCAFirstLine == pVI->ulLCALastLine)
    {
        /*  If the first and last line are equal, then there is only one
            line, so delete it and reset all the fields.
        */
        ulIndex = CalcLCARecordIndex(pVI, ulLine);

        /*  Clear the deleted entry
        */
        DclMemSet(&pVI->pLCA[ulIndex], 0, sizeof(VIEWLINECACHE));

        pVI->ulLCAFirstLine = 0;
        pVI->ulLCALastLine = 0;
        pVI->ulLCAHeadEntry = 0;

        pVI->ulLDCHeadOffset = 0;
        pVI->ulLDCTailOffset = 0;
    }
    else if(ulLine == pVI->ulLCAFirstLine)
    {
        ulIndex = CalcLCARecordIndex(pVI, ulLine+1);

        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
            "ViewCore:DeleteLDCLine() OldHeadOffset=%lu NewHeadOffset=%lu\n",
            pVI->ulLDCHeadOffset, pVI->pLCA[ulIndex].ulOffset));

        pVI->ulLDCHeadOffset = pVI->pLCA[ulIndex].ulOffset;

        ulIndex = CalcLCARecordIndex(pVI, ulLine);

        /*  Clear the deleted entry
        */
        DclMemSet(&pVI->pLCA[ulIndex], 0, sizeof(VIEWLINECACHE));

        pVI->ulLCAHeadEntry++;
        if(pVI->ulLCAHeadEntry >= pVI->ulLCAMaxEntries)
           pVI->ulLCAHeadEntry = 0;

        pVI->ulLCAFirstLine++;
    }
    else
    {
        /*  Must be deleting the last entry then -- asserted above.
        */
        ulIndex = CalcLCARecordIndex(pVI, ulLine);

        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
            "ViewCore:DeleteLDCLine() OldTailOffset=%lu NewTailOffset=%lu\n",
            pVI->ulLDCTailOffset, pVI->pLCA[ulIndex].ulOffset));

        pVI->ulLDCTailOffset = pVI->pLCA[ulIndex].ulOffset;

        /*  Clear the deleted entry
        */
        DclMemSet(&pVI->pLCA[ulIndex], 0, sizeof(VIEWLINECACHE));

        pVI->ulLCALastLine--;
    }

    return;
}


            /*-------------------------------------------------*\
             *                                                 *
             *                  LCA Management                 *
             *                                                 *
            \*-------------------------------------------------*/


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void AddLCARecord(
    VIEWINST   *pVI,
    ULONG       ulLine,
    ULONG       ulOffset,
    D_UINT16    uLen)
{
    ULONG       ulIndex;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewCore:AddLCARecord() Line=%lu Offset=%lu Len=%U\n",
        ulLine, ulOffset, uLen));

    DclAssert(ulLine);
    DclAssert(uLen);
    DclAssert(ulOffset < pVI->ulLDCBufferSize);
    DclAssert(ValidateDataStructures(pVI));

    /*  if there is nothing currently in the LCA buffer...
    */
    if(!pVI->ulLCAFirstLine && !pVI->ulLCALastLine)
    {
        pVI->pLCA[0].ulOffset = ulOffset;
        pVI->pLCA[0].uLength = uLen;

        pVI->ulLCAFirstLine = ulLine;
        pVI->ulLCALastLine = ulLine;

        pVI->ulLCAHeadEntry = 0;

        return;
    }

    DclAssert((ulLine == pVI->ulLCAFirstLine-1) || (ulLine == pVI->ulLCALastLine+1));

    if(ulLine == pVI->ulLCAFirstLine-1)
    {
        pVI->ulLCAFirstLine--;

        if(pVI->ulLCAHeadEntry)
            pVI->ulLCAHeadEntry--;
        else
            pVI->ulLCAHeadEntry = pVI->ulLCAMaxEntries-1;
    }
    else
    {
        pVI->ulLCALastLine++;
    }

    ulIndex = CalcLCARecordIndex(pVI, ulLine);

    pVI->pLCA[ulIndex].ulOffset = ulOffset;
    pVI->pLCA[ulIndex].uLength = uLen;

    return;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static BOOL ExpandLCA(
    VIEWINST   *pVI)
{
    ULONG       ulNewSize;
    HGLOBAL     hNew;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewCore:ExpandLCA() OldSize=%lu\n", pVI->ulLCAMaxEntries));

    DclAssert(pVI);
    DclAssert(ValidateDataStructures(pVI));

    /*  Expand the LCA by a factor of approximately 1/8th
    */
    ulNewSize = pVI->ulLCAMaxEntries + (pVI->ulLCAMaxEntries >> 3);

    if(ulNewSize > WVMAXLCAENTRYCOUNT)
       ulNewSize = WVMAXLCAENTRYCOUNT;

    /*  Impossible to expand?  return failure...
    */
    if(ulNewSize == pVI->ulLCAMaxEntries)
        return FALSE;

    GlobalUnlock(pVI->hLCA);

    hNew = GlobalReAlloc(pVI->hLCA, ulNewSize * sizeof(VIEWLINECACHE), GMEM_MOVEABLE|GMEM_ZEROINIT);
    if(!hNew)
    {
        DCLPRINTF(1, ("ViewCore:ExpandLCA() GlobalReAlloc() failed!\n"));

        /*  The docs state that the original handle is still valid if
            GlobalReAlloc() fails.
        */
        pVI->pLCA = GlobalLock(pVI->hLCA);
        DclAssert(pVI->pLCA);

        return FALSE;
    }

    pVI->hLCA = hNew;

    pVI->pLCA = GlobalLock(pVI->hLCA);
    DclAssert(pVI->pLCA);

    /*  If the existing data in the buffer is such that it wrapped around
        the end of the buffer back to the beginning, we must move the head
        data (which is at the end of the old buffer) to the end of the newly
        expanded buffer.
    */
    if((pVI->ulLCAMaxEntries - pVI->ulLCAHeadEntry) <= (pVI->ulLCALastLine - pVI->ulLCAFirstLine))
    {
        ULONG       ulDiffSize = ulNewSize - pVI->ulLCAMaxEntries;
        D_BUFFER   *pOld;
        D_BUFFER   *pNew;

        pOld = (D_BUFFER*)pVI->pLCA + (pVI->ulLCAHeadEntry * sizeof(VIEWLINECACHE));
        pNew = pOld + (ulDiffSize * sizeof(VIEWLINECACHE));

        memmove(pNew, pOld, (pVI->ulLCAMaxEntries - pVI->ulLCAHeadEntry) * sizeof(VIEWLINECACHE));

        pVI->ulLCAHeadEntry += ulDiffSize;
    }

    pVI->ulLCAMaxEntries = ulNewSize;

    DclAssert(ValidateDataStructures(pVI));

    return TRUE;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static ULONG CalcLCARecordIndex(
    VIEWINST   *pVI,
    ULONG       ulLine)
{
    ULONG       ulIndex;

    DclAssert(pVI);
    DclAssert(ulLine);
    DclAssert(ulLine <= pVI->ulLSACountUsed);
    DclAssert(ulLine >= pVI->ulLCAFirstLine && ulLine <= pVI->ulLCALastLine);

/*  Don't do this or we'll be in an endless loop...
    DclAssert(ValidateDataStructures(pVI));
*/

    ulIndex = ulLine - pVI->ulLCAFirstLine;

    ulIndex += pVI->ulLCAHeadEntry;

    if(ulIndex >= pVI->ulLCAMaxEntries)
        ulIndex -= pVI->ulLCAMaxEntries;

    DclAssert(ulIndex < pVI->ulLCAMaxEntries);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 3, 0),
        "ViewCore:CalcLCARecordIndex() Line=%lu returning Index=%lu (First=%lu Last=%lu Head=%lu Max=%lu)\n",
        ulLine, ulIndex, pVI->ulLCAFirstLine, pVI->ulLCALastLine, pVI->ulLCAHeadEntry, pVI->ulLCAMaxEntries));

    return ulIndex;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static VIEWLINECACHE* GetLCARecord(
    VIEWINST   *pVI,
    ULONG       ulLine)
{
    ULONG       ulIndex;

    DclAssert(pVI);
    DclAssert(ulLine);
    DclAssert(ValidateDataStructures(pVI));

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewCore:GetLCARecord() Line=%lu First=%lu Last=%lu\n", ulLine, pVI->ulLCAFirstLine, pVI->ulLCALastLine));

    if(ulLine < pVI->ulLCAFirstLine || ulLine > pVI->ulLCALastLine)
    {
        VIEWLINESTATE   ls;
        UINT            uLen;

        RetrieveLSARecord(pVI, ulLine, &ls);

        SetAbsoluteStreamPosition(pVI, ls.ulReadOffset);

        uLen = ls.nLineLength & ~TRUNCATE_FLAG;

        SendMessage(pVI->hWnd, WM_WINVIEW_READDATA, uLen, (LPARAM)pVI->pTempLineBuff);

        AddLineToLDC(pVI, ulLine, ls.nLineLength, pVI->pTempLineBuff);
    }

    ulIndex = CalcLCARecordIndex(pVI, ulLine);

    return &pVI->pLCA[ulIndex];
}


            /*-------------------------------------------------*\
             *                                                 *
             *                  LSA Management                 *
             *                                                 *
            \*-------------------------------------------------*/


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static BOOL SaveLSARecord(
    VIEWINST       *pVI,
    ULONG           ulLine,
    VIEWLINESTATE  *pLS)
{
    ULONG           ulMemOffset;

    DclAssert(pVI);
    DclAssert(pLS);
    DclAssert(ulLine);
    DclAssert(ValidateDataStructures(pVI));

    if(ulLine > pVI->ulLSAMaxCount)
    {
        if(!ExpandLSA(pVI, ulLine))
        {
            DclError();
            return FALSE;
        }
    }

    ulMemOffset = (ulLine-1) * sizeof(VIEWLINESTATE);

    memcpy(pVI->pLSA+ulMemOffset, pLS, sizeof(VIEWLINESTATE));

    return TRUE;
}


/*-------------------------------------------------------------------
    Return the VIEWLINESTATE structure defining where in the raw
    data stream a given line is positioned.

    Cannot fail -- must be called for lines which have already been
    read and indexed.
-------------------------------------------------------------------*/
static void RetrieveLSARecord(
    VIEWINST       *pVI,
    ULONG           ulLine,
    VIEWLINESTATE  *pLS)
{
    ULONG           ulMemOffset;

    DclAssert(pVI);
    DclAssert(pLS);
    DclAssert(ulLine);
    DclAssert(ulLine <= pVI->ulLSACountUsed);
    DclAssert(ValidateDataStructures(pVI));

    ulMemOffset = (ulLine-1) * sizeof(VIEWLINESTATE);

    memcpy(pLS, pVI->pLSA+ulMemOffset, sizeof(VIEWLINESTATE));

    return;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static BOOL ExpandLSA(
    VIEWINST   *pVI,
    ULONG       ulNewMin)
{
    ULONG       ulBuffSize;

    DclAssert(pVI);
    DclAssert(ulNewMin > pVI->ulLSAMaxCount);
    DclAssert(ValidateDataStructures(pVI));

    /*  Expand the LSA by a factor of approximately 1/8th, if possible
    */
    if(ULONG_MAX - ulNewMin < (pVI->ulLSAMaxCount >> 3))
        ulNewMin = ULONG_MAX;
    else
        ulNewMin += (pVI->ulLSAMaxCount >> 3);

    ulBuffSize = pVI->ulLSASize + ((ulNewMin - pVI->ulLSAMaxCount) * sizeof(VIEWLINESTATE));

    GlobalUnlock(pVI->hLSA);

    pVI->hLSA = GlobalReAlloc(pVI->hLSA, ulBuffSize, GMEM_MOVEABLE|GMEM_ZEROINIT);
    if(pVI->hLSA)
    {
        pVI->pLSA = GlobalLock(pVI->hLSA);
        DclAssert(pVI->pLSA);

        pVI->ulLSASize     = ulBuffSize;
        pVI->ulLSAMaxCount = ulNewMin;

        return TRUE;
    }

    return FALSE;
}


            /*-------------------------------------------------*\
             *                                                 *
             *                Instance Management              *
             *                                                 *
            \*-------------------------------------------------*/


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static BOOL AllocInstResources(
    VIEWINST   *pVI,
    ULONG       ulCacheSize)
{
    DclAssert(pVI);
    DclAssert(ValidateDataStructures(pVI));

    pVI->ulDataSize = SendMessage(pVI->hWnd, WM_WINVIEW_GETDATALEN, 0, 0);

    DclAssert(pVI->ulDataSize != ULONG_MAX);

    /*  LSA=Line State Array.  Allocate a global memory block that
        will contain an array of VIEWLINESTATE structures, one for
        each line in the file.
    */
    {
        pVI->ulLSAMaxCount = pVI->ulDataSize / WVAVGLINELEN;
        if(pVI->ulLSAMaxCount < WVMINLINEBUFFCOUNT)
            pVI->ulLSAMaxCount = WVMINLINEBUFFCOUNT;

        pVI->ulLSASize = pVI->ulLSAMaxCount * sizeof(VIEWLINESTATE);

        pVI->hLSA = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, pVI->ulLSASize);
        if(!pVI->hLSA)
            goto AIRCleanup;

        pVI->pLSA = GlobalLock(pVI->hLSA);
        DclAssert(pVI->pLSA);

        pVI->ulLSACountUsed = 0;
    }

    /*  LDC=Line Data Cache.  Allocate a global memory buffer to use
        for storing cached lines of text.
    */
    {
        if(ulCacheSize == 0)
        {
            ulCacheSize = WVDEFAULTMEMBUFFERSIZE;
            pVI->fDynamicCacheSize = TRUE;
        }
        else if(ulCacheSize < WVMINMEMBUFFERSIZE)
        {
            ulCacheSize = WVMINMEMBUFFERSIZE;
        }

        pVI->ulLDCBufferSize = ulCacheSize;

        pVI->hLDC = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, pVI->ulLDCBufferSize);
        if(!pVI->hLDC)
            goto AIRCleanup;

        pVI->pLDC = GlobalLock(pVI->hLDC);
        DclAssert(pVI->pLDC);

        pVI->ulLDCHeadOffset = 0;
        pVI->ulLDCTailOffset = 0;
    }

    /*  LCA=Line Cache Array.  Allocate a global memory buffer to use for
        storing an array of VIEWLINECACHE structures which point to line
        data in the LDC.
    */
    {
        pVI->ulLCAMaxEntries = pVI->ulLDCBufferSize / WVAVGLINELEN;

        pVI->hLCA = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, pVI->ulLCAMaxEntries * sizeof(VIEWLINECACHE));
        if(!pVI->hLCA)
            goto AIRCleanup;

        pVI->pLCA = GlobalLock(pVI->hLCA);
        DclAssert(pVI->pLCA);

        pVI->ulLCAFirstLine = 0;
        pVI->ulLCALastLine = 0;
    }

    return TRUE;

  AIRCleanup:
    ReleaseInstResources(pVI);

    return FALSE;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void ReleaseInstResources(
    VIEWINST   *pVI)
{
    DclAssert(pVI);
    DclAssert(ValidateDataStructures(pVI));

    if(pVI->hLCA)
    {
        GlobalUnlock(pVI->hLCA);
        GlobalFree(pVI->hLCA);
    }

    if(pVI->hLDC)
    {
        GlobalUnlock(pVI->hLDC);
        GlobalFree(pVI->hLDC);
    }

    if(pVI->hLSA)
    {
        GlobalUnlock(pVI->hLSA);
        GlobalFree(pVI->hLSA);
    }

    return;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static BOOL ValidateDataStructures(
    VIEWINST       *pVI)
{
/*    ULONG           ulLine;
    VIEWLINECACHE  *pLCA;
*/
    DclAssert(pVI);

    if(pVI->ulLCAFirstLine == 0)
        return TRUE;

    if(pVI->ulLCAFirstLine > pVI->ulLCALastLine)
        return FALSE;

    if(pVI->ulLCAHeadEntry >= pVI->ulLCAMaxEntries)
        return FALSE;
/*
    for(ulLine = pVI->ulLCAFirstLine; ulLine <= pVI->ulLCALastLine; ulLine++)
    {
        ULONG   ulIndex;

        ulIndex = CalcLCARecordIndex(pVI, ulLine);

        pLCA = &pVI->pLCA[ulIndex];

        if(pLCA->ulOffset >= pVI->ulLDCBufferSize)
            return FALSE;

        if((pLCA->uLength & ~TRUNCATE_FLAG) > WVMAXLINELEN)
            return FALSE;
    }
*/
    return TRUE;
}
