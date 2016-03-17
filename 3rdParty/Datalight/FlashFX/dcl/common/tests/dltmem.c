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

    This module contains unit tests for memset/memcpy() operations.

    Note that this code uses internal static wrapper functions for all the
    various interfaces we are testing.  This is done to avoid problems when
    macros are used to remap functions (and particularly when the remapping
    is done purely to add a typecast to a parameter).  For some reason this
    seems to cause grief with the PFNMEMSET/PFNMEMCPY typedefs used below.

    For an example, see the VxWorks port where DclMemSet() is specially
    defined as shown below:

    #define DclMemSet(ptr, val, len) memset((ptr), (D_UCHAR)(val), (len))

    Using this definition, without using the special wrapper functions will
    cause the build to fail (using the GNUVX6X ToolSet).

    While this will skew the performance results somewhat, we are treating
    all the tested functions identically, therefore they should all skew
    evenly.

    ToDo:
    - Add support for testing memcmp().
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltmem.c $
    Revision 1.19  2010/07/29 00:49:22Z  garyp
    Reduced a buffer size so the memset() tests run faster.
    Revision 1.18  2010/01/23 16:45:49Z  garyp
    Fixed to avoid "possible loss of data" warnings (no actual loss of data).
    Revision 1.17  2009/10/16 22:30:47Z  garyp
    Minor datatype and messaging changes -- no functional changes.
    Revision 1.16  2009/06/28 01:53:52Z  garyp
    Moved the Memory Tracking tests to a separate module.
    Revision 1.15  2008/07/23 23:02:26Z  keithg
    Added DclMemMove test.
    Revision 1.14  2008/05/29 19:50:46Z  garyp
    Merged from the WinMobile branch.
    Revision 1.13.1.2  2008/05/29 19:50:46Z  garyp
    Cleaned up some function headers -- no functional changes.
    Revision 1.13  2007/11/03 23:31:12Z  Garyp
    Added the standard module header.
    Revision 1.12  2007/10/15 17:28:31Z  Garyp
    Modified DclMemPoolVerify() to take an fQuiet parameter.
    Revision 1.11  2007/10/05 20:46:04Z  pauli
    Added memory tracking tests.
    Revision 1.10  2007/07/31 20:31:37Z  Garyp
    Fixed a broken message.
    Revision 1.9  2007/06/24 20:25:23Z  Garyp
    Modified the PerfLog stuff to use the new DCLPERFLOG_WRITE() functionality
    as well as handle the new "CategorySuffix" parameter.  Cleaned up some
    error messages and used some more descriptive variable names.
    Revision 1.8  2007/05/20 16:46:06Z  Garyp
    Enhanced some diagnostics messages.
    Revision 1.7  2007/04/03 19:14:22Z  Garyp
    Updated to support a PerfLog build number suffix.
    Revision 1.6  2007/02/11 01:37:03Z  Garyp
    Decreased the COPYBUFFLEN value so the test runs faster on slow hardware.
    Revision 1.5  2007/02/04 20:15:16Z  Garyp
    Fixed so the "SmallBuffLen" value is the same as DCL_ALIGNSIZE, to prevent
    asserts when doing aligned memset performance tests on small buffers.
    Revision 1.4  2007/01/28 02:56:10Z  Garyp
    Added PerfLog support.
    Revision 1.3  2006/08/23 23:36:16Z  brandont
    Fixed warning for ulClib variable used uninitialized in DclTestMemFuncs.
    Revision 1.2  2006/08/07 23:14:17Z  Garyp
    Added performance tests for memcpy(), and synonyms.
    Revision 1.1  2006/05/06 23:12:42Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include <dlperflog.h>
#include "dltests.h"

/*  Note that SMALLBUFFLEN can't be any smaller than DCL_ALIGNSIZE or
    the "aligned" functions will fail because the length is not evenly
    divisible by DCL_ALIGNSIZE.
*/
#define FILLCHAR        0xAB
#define SETCHAR         0xCD    /* high bit intentionally set here */
#define COPYBUFFLEN      (64)
#define SETBUFFLEN      (128)
#define SMALLBUFFLEN    (DCL_ALIGNSIZE)
#define BIGBUFFLEN      (512)
#define PERFSECS        (1)

/*-----------------------------------------------
    This macro returns TRUE if y is within range
    of x by a margin of 1/32d of x.  x and y
    must be unsigned values.
-----------------------------------------------*/
#define WITHIN_RANGE(x,y) (((y) > ((x) - ((x)>>5))) && ((y) < ((x) + ((x)>>5))))

typedef void*(*PFNMEMSET)(void *pBuffer, int chr, size_t len);
typedef void*(*PFNMEMCPY)(void *pDest, const void *pSrc, size_t len);

static DCLSTATUS TestMemSet(  PFNMEMSET pfnMemSet, const char *pszName, unsigned uAlign, unsigned fPerf);
static DCLSTATUS TestMemCpy(  PFNMEMCPY pfnMemCpy, const char *pszName, unsigned uAlign, unsigned fPerf);
static DCLSTATUS TestMemMove(  PFNMEMCPY pfnMemCpy);
static DCLSTATUS MemSetHelper(PFNMEMSET pfnMemSet, const char *pszName, D_BUFFER *pBuffer, unsigned uHead, unsigned uLength, unsigned uTail);
static DCLSTATUS MemCpyHelper(PFNMEMCPY pfnMemCpy, const char *pszName, D_BUFFER *pBuffer, const D_BUFFER *pSrc, unsigned uHead, unsigned uLength, unsigned uTail);
static D_UINT32  MemSetPerf(      PFNMEMSET pfnMemSet, const char *pszName, unsigned fMisaligned, unsigned fPerfLog, const char *pszPerfLogSuffix);
static D_UINT32  MemSetPerfHelper(PFNMEMSET pfnMemSet, D_BUFFER *pBuffer, unsigned uLength);
static D_UINT32  MemCpyPerf(      PFNMEMCPY pfnMemCpy, const char *pszName, unsigned fMisaligned, unsigned fPerfLog, const char *pszPerfLogSuffix);
static D_UINT32  MemCpyPerfHelper(PFNMEMCPY pfnMemCpy, D_BUFFER *pDest, D_BUFFER *pSrc, unsigned uLength);
static void *    LocalMemSet(          void *pDst, int chr, size_t nLen);
static void *    LocalDclMemSet(       void *pDst, int chr, size_t nLen);
static void *    LocalDclMemSetAligned(void *pDst, int chr, size_t nLen);
static void *    LocalMemCpy(          void *pDst, const void *pSrc, size_t nLen);
static void *    LocalDclMemCpy(       void *pDst, const void *pSrc, size_t nLen);
static void *    LocalDclMemCpyAligned(void *pDst, const void *pSrc, size_t nLen);
static void *    LocalDclMemMove(          void *pDst, const void *pSrc, size_t nLen);


/*-------------------------------------------------------------------
    DclTestMemFuncs()

    This function invokes the unit tests for the memset/cpy/cmp
    functions.

    Parameters:

    Return:
        Returns a DCLSTATUS value indicating success or failure
-------------------------------------------------------------------*/
DCLSTATUS DclTestMemFuncs(
    D_BOOL      fTestClib,
    D_BOOL      fPerf,
    D_BOOL      fPerfLog,
    const char *pszPerfLogSuffix)
{
    DCLSTATUS   dclStat;

    DclPrintf("Testing Mem Functions...\n");

    if((dclStat = TestMemSet(LocalDclMemSet, "DclMemSet", 1, fPerf)) != DCLSTAT_SUCCESS)
    {
        DclPrintf("DclMemSet() test failed, status=%lX\n", dclStat);
        return dclStat;
    }

    if((dclStat = TestMemCpy(LocalDclMemCpy, "DclMemCpy", 1, fPerf)) != DCLSTAT_SUCCESS)
    {
        DclPrintf("DclMemCpy() test failed, status=%lX\n", dclStat);
        return dclStat;
    }

    if((dclStat = TestMemCpy(LocalDclMemMove, "DclMemMove", 1, fPerf)) != DCLSTAT_SUCCESS)
    {
        DclPrintf("DclMemCpy() test failed, status=%lX\n", dclStat);
        return dclStat;
    }

    if((dclStat = TestMemMove(LocalDclMemMove)) != DCLSTAT_SUCCESS)
    {
        DclPrintf("DclMemMove() test failed, status=%lX\n", dclStat);
        return dclStat;
    }

    if((dclStat = TestMemSet(LocalDclMemSetAligned, "DclMemSetAligned", DCL_ALIGNSIZE, fPerf)) != DCLSTAT_SUCCESS)
    {
        DclPrintf("DclMemSetAligned() test failed, status=%lX\n", dclStat);
        return dclStat;
    }

    if((dclStat = TestMemCpy(LocalDclMemCpyAligned, "DclMemCpyAligned", DCL_ALIGNSIZE, fPerf)) != DCLSTAT_SUCCESS)
    {
        DclPrintf("DclMemCpyAligned() test failed, status=%lX\n", dclStat);
        return dclStat;
    }

    if(fTestClib)
    {
        if((dclStat = TestMemSet(LocalMemSet, "memset", 1, fPerf)) != DCLSTAT_SUCCESS)
        {
            DclPrintf("memset() test failed, status=%lX\n", dclStat);
            return dclStat;
        }

        if((dclStat = TestMemCpy(LocalMemCpy, "memcpy", 1, fPerf)) != DCLSTAT_SUCCESS)
        {
            DclPrintf("memcpy() test failed, status=%lX\n", dclStat);
            return dclStat;
        }
    }

    if(fPerf)
    {
        D_UINT32    ulMapped;
        D_UINT32    ulClib;
        D_UINT32    ulAligned;

        /*-------------------
                memset
        -------------------*/

        ulMapped = MemSetPerf(LocalDclMemSet, "DclMemSet", TRUE, fPerfLog, pszPerfLogSuffix);

        if(fTestClib)
        {
            ulClib = MemSetPerf(LocalMemSet, "memset", TRUE, fPerfLog, pszPerfLogSuffix);
            if(WITHIN_RANGE(ulClib, ulMapped))
            {
                DclPrintf("      NOTE: It appears as if DclMemSet() is mapped to the C library memset()\n");
            }
            else if(ulMapped < ulClib)
            {
                DclPrintf("      WARNING: DclMemSet() is significantly slower than memset().\n");
                DclPrintf("               Consider mapping DclMemSet() to use memset().\n");
            }

        }

        ulAligned = MemSetPerf(LocalDclMemSetAligned, "DclMemSetAligned", FALSE, fPerfLog, pszPerfLogSuffix);

        if(ulAligned < ulMapped && !WITHIN_RANGE(ulMapped, ulAligned))
        {
            DclPrintf("      WARNING: DclMemSetAligned() is slower than DclMemSet()!  Compiler flags\n");
            DclPrintf("               may need tuning, or remap DclMemSetAligned() to use memset() or\n");
            DclPrintf("               DclMemSet().\n");
        }

        /*-------------------
                memcpy
        -------------------*/

        ulMapped = MemCpyPerf(LocalDclMemCpy, "DclMemCpy", TRUE, fPerfLog, pszPerfLogSuffix);
        if(fTestClib)
        {
            ulClib = MemCpyPerf(LocalMemCpy, "memcpy", TRUE, fPerfLog, pszPerfLogSuffix);
            if(WITHIN_RANGE(ulClib, ulMapped))
            {
                DclPrintf("      NOTE: It appears as if DclMemCpy() is mapped to the C library memcpy()\n");
            }
            else if(ulMapped < ulClib)
            {
                DclPrintf("      WARNING: DclMemCpy() is significantly slower than memcpy().\n");
                DclPrintf("               Consider mapping DclMemCpy() to use memcpy().\n");
            }
        }

        ulAligned = MemCpyPerf(LocalDclMemCpyAligned, "DclMemCpyAligned", FALSE, fPerfLog, pszPerfLogSuffix);

        if(ulAligned < ulMapped && !WITHIN_RANGE(ulMapped, ulAligned))
        {
            DclPrintf("      WARNING: DclMemCpyAligned() is slower than DclMemCpy()!  Compiler flags\n");
            DclPrintf("               may need tuning, or remap DclMemCpyAligned() to use memcpy() or\n");
            DclPrintf("               DclMemCpy().\n");
        }
    }

    /*  all tests passed
    */
    DclPrintf("    OK\n");
    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestMemSet()

    This function invokes the unit tests for the memset()
    functions.

    Parameters:

    Return:
        Returns a DCLSTATUS value indicating success or failure
-------------------------------------------------------------------*/
static DCLSTATUS TestMemSet(
    PFNMEMSET           pfnMemSet,
    const char         *pszName,
    unsigned            uAlign,
    unsigned            fPerf)
{
    DCLALIGNEDBUFFER    (buffer, data, SETBUFFLEN);
    unsigned            tot, offset, tail;
    DCLSTATUS           dclStat;
    DCLTIMESTAMP        t;
    D_UINT32            ulTotal = 0;
    D_UINT32            ulElapsed;

    DclAssert(pfnMemSet);
    DclAssert(pszName);
    DclAssert(uAlign);

    DclPrintf("    Testing %s()...\n", pszName);

    t = DclTimeStamp();

    for(tot = uAlign; tot <= SETBUFFLEN; tot += uAlign)
    {
        for(offset = uAlign; offset <= tot; offset += uAlign)
        {
            for(tail = 0; tail < offset; tail += uAlign)
            {
                /*  Verify that we really did the math right
                */
                DclAssert((tot-offset) + (offset-tail) + tail == tot);

                dclStat = MemSetHelper(pfnMemSet, pszName, buffer.data, tot-offset, offset-tail, tail);
                if(dclStat != DCLSTAT_SUCCESS)
                    return dclStat;

                ulTotal += (offset-tail);
            }
        }
    }

    if(fPerf)
    {
        ulElapsed = DclTimePassed(t);
        if(!ulElapsed)
            ulElapsed++;

        DclPrintf("      Set and verified %lU bytes in %lU milliseconds (%lU bytes per ms)\n",
            ulTotal, ulElapsed, ulTotal/ulElapsed);
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestMemCpy()

    This function invokes the unit tests for the memcpy()
    functions.

    Parameters:

    Return:
        Returns a DCLSTATUS value indicating success or failure
-------------------------------------------------------------------*/
static DCLSTATUS TestMemCpy(
    PFNMEMCPY           pfnMemCpy,
    const char         *pszName,
    unsigned            uAlign,
    unsigned            fPerf)
{
    #define             SRCOFFSET   (DCL_ALIGNSIZE*8)
    #define             SRCBUFFLEN  (COPYBUFFLEN+SRCOFFSET)
    DCLALIGNEDBUFFER    (buffer, data, COPYBUFFLEN);
    DCLALIGNEDBUFFER    (src, data, SRCBUFFLEN);
    unsigned            tot, offset, tail, q;
    DCLSTATUS           dclStat;
    DCLTIMESTAMP        t;
    D_UINT32            ulTotal = 0;
    D_UINT32            ulElapsed;

    DclAssert(pfnMemCpy);
    DclAssert(pszName);
    DclAssert(uAlign);

    DclPrintf("    Testing %s()...\n", pszName);

    t = DclTimeStamp();

    /*  Initialize the source buffer
    */
    for(q = 0; q < SRCBUFFLEN; q++)
        src.data[q] = (unsigned char)q;

    for(tot = uAlign; tot <= COPYBUFFLEN; tot += uAlign)
    {
        for(offset = uAlign; offset <= tot; offset += uAlign)
        {
            for(tail = 0; tail < offset; tail += uAlign)
            {
                /*  Verify that we really did the math right
                */
                DclAssert((tot-offset) + (offset-tail) + tail == tot);

                for(q = 0; q < SRCOFFSET; q += uAlign)
                {
                    dclStat = MemCpyHelper(pfnMemCpy, pszName,
                                        buffer.data, &src.data[q], tot-offset, offset-tail, tail);
                    if(dclStat != DCLSTAT_SUCCESS)
                        return dclStat;

                    ulTotal += (offset-tail);
                }
            }
        }
    }

    if(fPerf)
    {
        ulElapsed = DclTimePassed(t);
        if(!ulElapsed)
            ulElapsed++;

        DclPrintf("      Copied and verified %lU bytes in %lU milliseconds (%lU bytes per ms)\n",
            ulTotal, ulElapsed, ulTotal/ulElapsed);
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestMemMove()

    This function completes tests for the memmove() with an
    overlapped buffer.  The primary functional tests are completed
    with the TestMemCpy function.

    Parameters:

    Return:
        Returns a DCLSTATUS value indicating success or failure
-------------------------------------------------------------------*/
static DCLSTATUS TestMemMove(
    PFNMEMCPY   pfnMemMove)
{
    #define     MEMMOVE_TEST_SIZE   100
    #define     MEMMOVE_TEST_OFFSET 5
    D_UINT8    *pBuffer;
    D_UINT8    *pSrc, *pDst;
    D_UINT8     ii;

    DclAssert(pfnMemMove);
    DclAssert(MEMMOVE_TEST_SIZE + MEMMOVE_TEST_OFFSET <= D_UINT8_MAX);

    DclPrintf("    Testing DclMemMove() w/overlapped buffers...\n");

    pBuffer = DclMemAlloc(MEMMOVE_TEST_SIZE + MEMMOVE_TEST_OFFSET);

    /*  Test Case 1: Copy from buffer[x] into buffer[x+1]
        failure of this test indicates the copy went the
        wrong direction.
    */
    for(ii=0; ii<MEMMOVE_TEST_SIZE+MEMMOVE_TEST_OFFSET; ++ii)
    {
        pBuffer[ii] = ii;
    }

    pDst = pBuffer + MEMMOVE_TEST_OFFSET;
    pSrc = pBuffer;
    pfnMemMove(pDst, pSrc, MEMMOVE_TEST_SIZE);

    /*  Ensure the destination buffer is correct
    */
    for(ii=0; ii<MEMMOVE_TEST_SIZE; ++ii)
    {
        if(pDst[ii] != ii)
            return DCLSTAT_FAILURE;
    }


    /*  Test Case 2: Copy from buffer[x+1] into buffer[x]
        failure indicates
    */
    for(ii=0; ii<MEMMOVE_TEST_SIZE+MEMMOVE_TEST_OFFSET; ++ii)
    {
        pBuffer[ii] = ii;
    }

    pDst = pBuffer;
    pSrc = pBuffer + MEMMOVE_TEST_OFFSET;
    pfnMemMove(pDst, pSrc, MEMMOVE_TEST_SIZE);

    /*  Ensure the destination buffer is correct
    */
    for(ii=0; ii<MEMMOVE_TEST_SIZE; ++ii)
    {
        if(pDst[ii] != MEMMOVE_TEST_OFFSET+ii)
            return DCLSTAT_FAILURE;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    MemSetHelper()

    This function implements the meat of the code for testing
    memset().

    Parameters:
        pfnMemSet - A pointer to the memset() function to use
        pszName   - The name of the memset() function being used
        pBuffer   - The data buffer to use
        uHead     - The offset to skip into the specified buffer
        uLength   - The length of data to memset()
        uTail     - The count of bytes to be untouched at the
                    end of the buffer.

    Return:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS MemSetHelper(
    PFNMEMSET       pfnMemSet,
    const char     *pszName,
    D_BUFFER       *pBuffer,
    unsigned        uHead,
    unsigned        uLength,
    unsigned        uTail)
{
    unsigned        i;

    DclAssert(pfnMemSet);
    DclAssert(pszName);
    DclAssert(pBuffer);
    DclAssert(uLength);

    DCLPRINTF(3, ("MemSetHelper() \"%s\" Head=%u Len=%u Tail=%u\n",
        pszName, uHead, uLength, uTail));

    /*  Initialize the buffer for the total length with FILLCHAR
    */
    for(i=0; i<uHead+uLength+uTail; i++)
        pBuffer[i] = FILLCHAR;

    /*  Issue the specified memset() command
    */
    if((*pfnMemSet)(pBuffer+uHead, SETCHAR, uLength) != pBuffer+uHead)
    {
        DclPrintf("    %s failed, incorrect pointer returned. Head=%u Len=%u Tail=%u\n",
            pszName, uHead, uLength, uTail);
        return DCLSTAT_CURRENTLINE;
    }

    /*  Verify that the head (leading portion) is still valid.
    */
    for(i=0; i<uHead; i++)
    {
        if(pBuffer[i] != FILLCHAR)
        {
            DclPrintf("    %s failed at offset %u, %02x should be %02x. Head=%u Len=%u Tail=%u\n",
                pszName, i, pBuffer[i], FILLCHAR, uHead, uLength, uTail);
            return DCLSTAT_CURRENTLINE;
        }
    }

    /*  Verify that the data we memset() is valid.
    */
    for(i=i; i<uHead+uLength; i++)
    {
        if(pBuffer[i] != SETCHAR)
        {
            DclPrintf("    %s failed at offset %u, %02x should be %02x. Head=%u Len=%u Tail=%u\n",
                pszName, i, pBuffer[i], SETCHAR, uHead, uLength, uTail);
            return DCLSTAT_CURRENTLINE;
        }
    }

    /*  Verify that the tail (trailing portion) is still valid.
    */
    for(i=i; i<uHead+uLength+uTail; i++)
    {
        if(pBuffer[i] != FILLCHAR)
        {
            DclPrintf("    %s failed at offset %u, %02x should be %02x. Head=%u Len=%u Tail=%u\n",
                pszName, i, pBuffer[i], FILLCHAR, uHead, uLength, uTail);
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    MemCpyHelper()

    This function implements the meat of the code for testing
    memcpy().

    Parameters:
        pfnMemSet - A pointer to the memcpy() function to use
        pszName   - The name of the memcpy() function being used
        pBuffer   - The output data buffer to use
        pSrc      - The source data to copy
        uHead     - The offset to skip into the specified buffer
        uLength   - The length of data to memcpy()
        uTail     - The count of bytes to be untouched at the
                    end of the buffer.

    Return:
        Returns a DCLSTATUS value indicating success or failure
-------------------------------------------------------------------*/
static DCLSTATUS MemCpyHelper(
    PFNMEMCPY       pfnMemCpy,
    const char     *pszName,
    D_BUFFER       *pBuffer,
    const D_BUFFER *pSrc,
    unsigned        uHead,
    unsigned        uLength,
    unsigned        uTail)
{
    unsigned        i, j;

    DclAssert(pfnMemCpy);
    DclAssert(pszName);
    DclAssert(pBuffer);
    DclAssert(pSrc);
    DclAssert(uLength);

    DCLPRINTF(3, ("MemCpyHelper() \"%s\" Head=%u Len=%u Tail=%u\n",
        pszName, uHead, uLength, uTail));

    /*  Initialize the buffer for the total length with FILLCHAR
    */
    for(i=0; i<uHead+uLength+uTail; i++)
        pBuffer[i] = FILLCHAR;

    /*  Issue the specified memcpy() command
    */
    if((*pfnMemCpy)(pBuffer+uHead, pSrc, uLength) != pBuffer+uHead)
    {
        DclPrintf("    %s failed, incorrect pointer returned. Head=%u Len=%u Tail=%u\n",
            pszName, uHead, uLength, uTail);
        return DCLSTAT_CURRENTLINE;
    }

    /*  Verify that the head (leading portion) is still valid.
    */
    for(i=0; i<uHead; i++)
    {
        if(pBuffer[i] != FILLCHAR)
        {
            DclPrintf("    %s failed at offset %u, %02x should be %02x. Head=%u Len=%u Tail=%u\n",
                pszName, i, pBuffer[i], FILLCHAR, uHead, uLength, uTail);
            return DCLSTAT_CURRENTLINE;
        }
    }

    /*  Verify that the data we memset() is valid.
    */
    for(i=i, j=0; i<uHead+uLength; i++, j++)
    {
        if(pBuffer[i] != pSrc[j])
        {
            DclPrintf("    %s failed at offset %u, %02x should be %02x. Head=%u Len=%u Tail=%u\n",
                pszName, i, pBuffer[i], pSrc[j], uHead, uLength, uTail);
            return DCLSTAT_CURRENTLINE;
        }
    }

    /*  Verify that the tail (trailing portion) is still valid.
    */
    for(i=i; i<uHead+uLength+uTail; i++)
    {
        if(pBuffer[i] != FILLCHAR)
        {
            DclPrintf("    %s failed at offset %u, %02x should be %02x. Head=%u Len=%u Tail=%u\n",
                pszName, i, pBuffer[i], FILLCHAR, uHead, uLength, uTail);
            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    MemSetPerf()

    This function implements the meat of the code for testing
    memset().

    Parameters:

    Return:
        Returns the sum composite rate value for all tests (not
        including the misaligned tests).
-------------------------------------------------------------------*/
static D_UINT32 MemSetPerf(
    PFNMEMSET           pfnMemSet,
    const char         *pszName,
    unsigned            fMisaligned,
    unsigned            fPerfLog,
    const char         *pszPerfLogSuffix)
{
    DCLALIGNEDBUFFER    (buffer, data, BIGBUFFLEN+DCL_ALIGNSIZE);
    D_UINT32            ulRate;
    D_UINT32            ulReturn = 0;
    DCLPERFLOGHANDLE    hPerfLog;

    DclAssert(pfnMemSet);
    DclAssert(pszName);

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perfdcl.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    hPerfLog = DCLPERFLOG_OPEN(fPerfLog, NULL, "DCL", pszName, NULL, pszPerfLogSuffix);

    DclPrintf("    Performance testing %s()\n", pszName);
    DCLPERFLOG_NUM(hPerfLog, "SmallBuffSize", SMALLBUFFLEN);
    DCLPERFLOG_NUM(hPerfLog, "LargeBuffSize", BIGBUFFLEN);

    ulRate = MemSetPerfHelper(pfnMemSet, buffer.data, SMALLBUFFLEN);
    DclPrintf("      Small buffer (%u bytes) aligned,         %9lU bytes per ms\n", SMALLBUFFLEN, ulRate);
    ulReturn += ulRate;

    DCLPERFLOG_NUM(hPerfLog, "SmallBuffAligned", ulRate);

    if(fMisaligned)
    {
        ulRate = MemSetPerfHelper(pfnMemSet, &buffer.data[1], SMALLBUFFLEN);
        DclPrintf("      Small buffer misaligned,                %9lU bytes per ms\n", ulRate);
        DCLPERFLOG_NUM(hPerfLog, "SmallBuffMisaligned", ulRate);
    }

    ulRate = MemSetPerfHelper(pfnMemSet, buffer.data, BIGBUFFLEN);
    DclPrintf("      Large buffer (%u bytes) aligned,       %9lU bytes per ms\n", BIGBUFFLEN, ulRate);
    ulReturn += ulRate;

    DCLPERFLOG_NUM(hPerfLog, "LargeBuffAligned", ulRate);

    if(fMisaligned)
    {
        ulRate = MemSetPerfHelper(pfnMemSet, &buffer.data[1], BIGBUFFLEN);
        DclPrintf("      Large buffer misaligned,                %9lU bytes per ms\n", ulRate);
        DCLPERFLOG_NUM(hPerfLog, "LargeBuffMisaligned", ulRate);
    }

    DCLPERFLOG_WRITE(hPerfLog);
    DCLPERFLOG_CLOSE(hPerfLog);

    return ulReturn;
}


/*-------------------------------------------------------------------
    MemSetPerfHelper()

    This function implements the meat of the code for performance
    testing memset().

    Parameters:

    Return:
        Returns the bytes per millisecond memset rate.
-------------------------------------------------------------------*/
static D_UINT32 MemSetPerfHelper(
    PFNMEMSET       pfnMemSet,
    D_BUFFER       *pBuffer,
    unsigned        uLength)
{
    DCLTIMER        t;
    D_UINT32        ulTotal = 0;
    D_UINT32        ulElapsed;

    DclAssert(pfnMemSet);
    DclAssert(pBuffer);

    DclTimerSet(&t, PERFSECS*1000);

    while(!DclTimerExpired(&t))
    {
        (*pfnMemSet)(pBuffer, SETCHAR, uLength);
        ulTotal += uLength;
    }

    ulElapsed = DclTimerElapsed(&t);
    if(!ulElapsed)
        ulElapsed++;

    DCLPRINTF(1, ("      Set %lU total bytes (%U at a time) in %lU milliseconds\n",
        ulTotal, uLength, ulElapsed));

    return ulTotal / ulElapsed;
}


/*-------------------------------------------------------------------
    MemCpyPerf()

    This function implements the code for performance testing
    of memcpy().

    Parameters:

    Return:
        Returns the sum composite rate value for all tests (not
        including the misaligned tests).
-------------------------------------------------------------------*/
static D_UINT32 MemCpyPerf(
    PFNMEMCPY           pfnMemCpy,
    const char         *pszName,
    unsigned            fMisaligned,
    unsigned            fPerfLog,
    const char         *pszPerfLogSuffix)
{
    DCLALIGNEDBUFFER    (srcbuff, data, BIGBUFFLEN+DCL_ALIGNSIZE);
    DCLALIGNEDBUFFER    (destbuff, data, BIGBUFFLEN+DCL_ALIGNSIZE);
    D_UINT32            ulRate;
    D_UINT32            ulReturn = 0;
    DCLPERFLOGHANDLE    hPerfLog;

    DclAssert(pfnMemCpy);
    DclAssert(pszName);

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perfdcl.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    hPerfLog = DCLPERFLOG_OPEN(fPerfLog, NULL, "DCL", pszName, NULL, pszPerfLogSuffix);
    DCLPERFLOG_NUM(hPerfLog, "SmallBuffSize", SMALLBUFFLEN);
    DCLPERFLOG_NUM(hPerfLog, "LargeBuffSize", BIGBUFFLEN);

    DclPrintf("    Performance testing %s()\n", pszName);

    ulRate = MemCpyPerfHelper(pfnMemCpy, destbuff.data, srcbuff.data, SMALLBUFFLEN);
    DclPrintf("      Small buffer (%u bytes) aligned,         %9lU bytes per ms\n", SMALLBUFFLEN, ulRate);
    ulReturn += ulRate;

    DCLPERFLOG_NUM(hPerfLog, "SmallBuffAligned", ulRate);

    if(fMisaligned)
    {
        ulRate = MemCpyPerfHelper(pfnMemCpy, &destbuff.data[1], &srcbuff.data[1], SMALLBUFFLEN);
        DclPrintf("      Small buffer misaligned dest[1] src[1], %9lU bytes per ms\n", ulRate);

        DCLPERFLOG_NUM(hPerfLog, "SmallBuffMisaligned11", ulRate);

        ulRate = MemCpyPerfHelper(pfnMemCpy, &destbuff.data[1], &srcbuff.data[3], SMALLBUFFLEN);
        DclPrintf("      Small buffer misaligned dest[1] src[3], %9lU bytes per ms\n", ulRate);

        DCLPERFLOG_NUM(hPerfLog, "SmallBuffMisaligned13", ulRate);
    }

    ulRate = MemCpyPerfHelper(pfnMemCpy, destbuff.data, srcbuff.data, BIGBUFFLEN);
    DclPrintf("      Large buffer (%u bytes) aligned,       %9lU bytes per ms\n", BIGBUFFLEN, ulRate);
    ulReturn += ulRate;

    DCLPERFLOG_NUM(hPerfLog, "LargeBuffAligned", ulRate);

    if(fMisaligned)
    {
        ulRate = MemCpyPerfHelper(pfnMemCpy, &destbuff.data[1], &srcbuff.data[1], BIGBUFFLEN);
        DclPrintf("      Large buffer misaligned dest[1] src[1], %9lU bytes per ms\n", ulRate);

        DCLPERFLOG_NUM(hPerfLog, "LargeBuffMisaligned11", ulRate);

        ulRate = MemCpyPerfHelper(pfnMemCpy, &destbuff.data[1], &srcbuff.data[3], BIGBUFFLEN);
        DclPrintf("      Large buffer misaligned dest[1] src[3], %9lU bytes per ms\n", ulRate);

        DCLPERFLOG_NUM(hPerfLog, "LargeBuffMisaligned13", ulRate);
    }

    DCLPERFLOG_WRITE(hPerfLog);
    DCLPERFLOG_CLOSE(hPerfLog);

    return ulReturn;
}


/*-------------------------------------------------------------------
    MemCpyPerfHelper()

    This function implements the meat of the code for performance
    testing memcpy().

    Parameters:

    Return:
        Returns the bytes per millisecond memcpy rate.
-------------------------------------------------------------------*/
static D_UINT32 MemCpyPerfHelper(
    PFNMEMCPY       pfnMemCpy,
    D_BUFFER       *pDest,
    D_BUFFER       *pSrc,
    unsigned        uLength)
{
    DCLTIMER        t;
    D_UINT32        ulTotal = 0;
    D_UINT32        ulElapsed;

    DclAssert(pfnMemCpy);
    DclAssert(pDest);
    DclAssert(pSrc);

    DclTimerSet(&t, PERFSECS*1000);

    while(!DclTimerExpired(&t))
    {
        (*pfnMemCpy)(pDest, pSrc, uLength);
        ulTotal += uLength;
    }

    ulElapsed = DclTimerElapsed(&t);
    if(!ulElapsed)
        ulElapsed++;

    DCLPRINTF(1, ("      Copied %lU total bytes (%U at a time) in %lU milliseconds\n",
        ulTotal, uLength, ulElapsed));

    return ulTotal / ulElapsed;
}


/*-------------------------------------------------------------------
    This is a simple wrapper used to avoid problems which may occur
    when the functions are remapped with macros.
-------------------------------------------------------------------*/
static void * LocalMemSet(
    void           *pDst,
    int             chr,
    size_t          nLen)
{
    return memset(pDst, chr, nLen);
}

/*-------------------------------------------------------------------
    This is a simple wrapper used to avoid problems which may occur
    when the functions are remapped with macros.
-------------------------------------------------------------------*/
static void * LocalDclMemSet(
    void           *pDst,
    int             chr,
    size_t          nLen)
{
    return DclMemSet(pDst, chr, nLen);
}

/*-------------------------------------------------------------------
    This is a simple wrapper used to avoid problems which may occur
    when the functions are remapped with macros.
-------------------------------------------------------------------*/
static void * LocalDclMemSetAligned(
    void           *pDst,
    int             chr,
    size_t          nLen)
{
    return DclMemSetAligned(pDst, chr, nLen);
}

/*-------------------------------------------------------------------
    This is a simple wrapper used to avoid problems which may occur
    when the functions are remapped with macros.
-------------------------------------------------------------------*/
static void * LocalMemCpy(
    void           *pDst,
    const void     *pSrc,
    size_t          nLen)
{
    return memcpy(pDst, pSrc, nLen);
}

/*-------------------------------------------------------------------
    This is a simple wrapper used to avoid problems which may occur
    when the functions are remapped with macros.
-------------------------------------------------------------------*/
static void * LocalDclMemCpy(
    void           *pDst,
    const void     *pSrc,
    size_t          nLen)
{
    return DclMemCpy(pDst, pSrc, nLen);
}

/*-------------------------------------------------------------------
    This is a simple wrapper used to avoid problems which may occur
    when the functions are remapped with macros.
-------------------------------------------------------------------*/
static void * LocalDclMemMove(
    void           *pDst,
    const void     *pSrc,
    size_t          nLen)
{
    return DclMemMove(pDst, pSrc, nLen);
}

/*-------------------------------------------------------------------
    This is a simple wrapper used to avoid problems which may occur
    when the functions are remapped with macros.
-------------------------------------------------------------------*/
static void * LocalDclMemCpyAligned(
    void           *pDst,
    const void     *pSrc,
    size_t          nLen)
{
    return DclMemCpyAligned(pDst, pSrc, nLen);
}


