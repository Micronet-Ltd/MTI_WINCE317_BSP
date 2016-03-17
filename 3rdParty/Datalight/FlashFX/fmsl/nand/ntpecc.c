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

    This module provides the means to do parameterized EDC using an extensible
    interface that can be used to set up EDC support for an arbitrary level
    of bit correction/detection.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntpecc.c $
    Revision 1.8  2010/04/29 00:04:23Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.7  2010/01/27 04:27:37Z  glenns
    Repair issues exposed by turning on a compiler option to warn of 
    possible data loss resulting from implicit typecasts between
    integer data types.
    Revision 1.6  2009/10/06 05:56:58Z  garyp
    Modified to use the revamped single-bit ECC calculation and
    correction functions.  Did <some> code cleanup to get closer
    to current standards -- much more to do.
    Revision 1.5  2009/04/03 18:55:54Z  glenns
    Fix Bugzilla #2560: add a typecast to prevent compiler warning.
    Revision 1.4  2009/04/02 17:25:53Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.3  2009/03/25 18:19:02Z  billr
    Add revision history.
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxiosys.h>
#include <ecc.h>
#include <errmanager.h>


/*-------------------------------------------------------------------
    Public: FfxParameterizedEccGenerate()

    This procedure is a wrapper that allows clients to generate
    a set of ECCs using parameterized EDC mechanism as outlined
    in "ecc.h". The parameterized ecc interface allows arbitrary
    EDC mechanisms to be connected to FlashFX using a structure
    that allows the characteristics of the ECC mechanism to be
    communicated to FlashFX. See "ecc.h" for details.

    Parameters:
        params - A structure used to provide relevant parameters
                 to an EDC mechanism regarding the generation
                 of ECCs.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxParameterizedEccGenerate(
    FX_ECC_PARAMS  *params)
{
    D_UINT32        ulDataLength;
    D_UINT32        ulNumSegments;
    FX_ECC_PARAMS   localEccParams = *params;

    /*  REFACTOR: The following local is placed here as a temporary storage
        location to put metadata in order to account for the current
        incarnation of the four-bit BCH EDC mechanism won't consume less
        than the number of bytes it's ECCProcessor indicates, which can happen.
        See the next "refactor" below.
    */
    D_UINT8         aucTemp[16];

    /*  IMPORTANT NOTE on the use of this procedure (PLEASE READ):

        Existing software algorithms for the legacy SLC spare area formats
        (SSFDC and OffsetZero) expect ECCs to be generated for an entire
        page at a time, even though the one-bit DCL ECC mechanism only
        consumes 256 bytes of main page data at a time. This procedure
        supports breaking the page up into segments so that legacy spare
        area formats may be supported by this mechanism. The existing
        call chains rely on the ECC hooks to do this, which are not used
        for parameterized ECC.

        The way this code does this is subject to a limitation. It expects
        both main page and metadata to exist contiguously in their respective
        buffers. This works for the SSFDC and OffsetZero formats because the
        main page data is provided in a contiguous buffer, and ECCs are not
        evaluated for metadata. Failure to observe this limitation will result
        in incorrectly-generated ECCs, and in the generalized case, metadata
        that needs to be covered by ECCs will probably *not* be contiguous
        in the native spare area format.

        Because of this, any client that wishes to use this procedure
        will have to do one of two things if the number of bytes evaluated
        by the ECC Processor is only a fraction of the page size, i.e the
        page has to be segmented:

         1. It will have to break the page and metadata up on its own so
            that only one segment of main page and metadata ECCs are
            generated per pass through this procedure, or...
         2. It will have to construct main page and metadata buffers that
            contain all segments of data and metadata in them contiguously,
            such that each main segment is exacly "ulNumDataBytesEvaluated"
            bytes long and each metadata segment is "ulNumMetaBytesEvaluated"
            bytes long, according to the parameters of the ECC Processor in
            use.

        To avoid unnecessary copying of data and allocation of stack space,
        it is *HIGHLY* recommended that the first of these options be used.
        It is the NTMs and the NTHelp mechanisms that know the details of
        spare area formats, and what sorts of games will have to be played
        in order to correctly evaluate ECCs for given pieces of main page
        and metadata. It is inappropriate to force this module to concern
        itself with spare area format details.
    */

    localEccParams.cmd = ECC_CMD_ENCODE;
    ulDataLength = localEccParams.data_buffer.uCount;

    /*  1. There better be an ECC processor.
        2. There better be enough room to store the ECCs generated.
    */
    DclAssert(localEccParams.pEccProcessor);
    DclAssert(localEccParams.pEccProcessor->ulNumDataBytesEvaluated);
    DclAssert((ulDataLength % localEccParams.pEccProcessor->ulNumDataBytesEvaluated) == 0);

    /*  REFACTOR: The following block of code is to account for the fact that
        the current incarnation of the four-bit BCH EDC mechanism we have
        doesn't account for the requested count in the metadata yet. In other
        words, it doesn't yet do what this block does.

        This clearly isn't the best way to do this either here or
        in the BCH code, but it works as a stopgap to get us where we need
        to be.
    */
    if ((localEccParams.metadata_buffer.buffer) &&
        (localEccParams.pEccProcessor->ulNumMetaBytesEvaluated >
         localEccParams.metadata_buffer.uCount))
    {
        unsigned i;

        for (i=0; i<localEccParams.metadata_buffer.uCount; i++)
            aucTemp[i] = localEccParams.metadata_buffer.buffer[localEccParams.metadata_buffer.uOffset+i];
        for (i=localEccParams.metadata_buffer.uCount;
             i<localEccParams.pEccProcessor->ulNumMetaBytesEvaluated;
             i++)
            aucTemp[i] = 0;
        localEccParams.metadata_buffer.buffer = (D_BUFFER *)&aucTemp[0];
        localEccParams.metadata_buffer.uCount = localEccParams.pEccProcessor->ulNumMetaBytesEvaluated;
        localEccParams.metadata_buffer.uOffset = 0;
    }
    /*  end REFACTOR
    */

    ulNumSegments = ulDataLength / localEccParams.pEccProcessor->ulNumDataBytesEvaluated;
    DclAssert(localEccParams.pEccProcessor->ulNumBytesForECC * ulNumSegments <=
              localEccParams.ulECCBufferByteCount);

    /*  Generate ECCs:
    */
    while (ulNumSegments)
    {
        (*(localEccParams.pEccProcessor->Calculate))(&localEccParams);
        ulNumSegments--;
        localEccParams.data_buffer.uOffset += localEccParams.pEccProcessor->ulNumDataBytesEvaluated;
        localEccParams.ecc_buffer += localEccParams.pEccProcessor->ulNumBytesForECC;
        if (localEccParams.metadata_buffer.buffer)
            localEccParams.metadata_buffer.uOffset+= localEccParams.pEccProcessor->ulNumMetaBytesEvaluated;
    }
    
    return;
}


/*-------------------------------------------------------------------
    Public: FfxParameterizedEccCorrectData()

    This procedure is a wrapper that allows clients to evaluate
    a set of ECCs and use them to correct a piece of data using
    the parameterized EDC mechanism as outlined in "ecc.h". The
    parameterized ecc interface allows arbitrary EDC mechanisms
    to be connected to FlashFX using a structure  that allows
    the characteristics of the ECC mechanism to be communicated
    to FlashFX. See "ecc.h" for details.

    Parameters:
        params - A structure used to provide relevant parameters
                 to an EDC mechanism regarding the generation
                 and evaluationof ECCs.
    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxParameterizedEccCorrectData(
    FX_ECC_PARAMS  *params)
{
    FX_ECC_PARAMS   localEccParams = *params;
    D_BUFFER       *pOrigBuffer;  /* See "REFACTOR" below. */
    D_UINT8         aucTemp[16];  /* See "REFACTOR" below. */

    /*  1. There better be an ECC processor.
    */
    DclAssert(localEccParams.pEccProcessor);

    /*  REFACTOR: The following block of code is to account for the fact that
        the current incarnation of the four-bit BCH EDC mechanism we have
        doesn't account for the requested count in the metadata yet. In other
        words, it doesn't yet do what this block does.

        This clearly isn't the best way to do this either here or
        in the BCH code, but it works as a stopgap to get us where we need
        to be.
    */
    pOrigBuffer = &(params->metadata_buffer.buffer[params->metadata_buffer.uOffset]);
    if ((params->metadata_buffer.buffer) &&
        (params->pEccProcessor->ulNumMetaBytesEvaluated >
         params->metadata_buffer.uCount))
    {
    unsigned i;

        for (i=0; i<params->metadata_buffer.uCount; i++)
            aucTemp[i] = params->metadata_buffer.buffer[params->metadata_buffer.uOffset+i];
        for (i=params->metadata_buffer.uCount;
             i<params->pEccProcessor->ulNumMetaBytesEvaluated;
             i++)
            aucTemp[i] = 0;
        localEccParams.metadata_buffer.buffer = (D_BUFFER *)&aucTemp[0];
        localEccParams.metadata_buffer.uCount = params->pEccProcessor->ulNumMetaBytesEvaluated;
        localEccParams.metadata_buffer.uOffset = 0;
    }

    /*  Perform the actual correction
    */
    localEccParams.cmd = ECC_CMD_VERIFY;
    (*(localEccParams.pEccProcessor->Calculate))(&localEccParams);

    /*  For the same reason outlined above in the "REFACTOR" note, copy the
        corrected data from the temporary buffer back into the original
        metadata buffer:
    */
    if ((params->metadata_buffer.buffer) &&
        (params->pEccProcessor->ulNumMetaBytesEvaluated >
         params->metadata_buffer.uCount))
    {
    unsigned i;

        for (i=0; i<params->metadata_buffer.uCount; i++)
            pOrigBuffer[i] = aucTemp[i];
    }
    return;
}


/*-------------------------------------------------------------------
    Public: FfxDefaultOneBitCalculate()

    This function performs the same functions as FfxEccCorrect()
    and FfxEccCalculate(), except that it that it takes as input a
    pointer to an ECCParams structure. It allows the parameterized
    ECC interface to use the existing one-bit correction mechanism
    in DCL to be used through the parameterized ECC interface.

    Parameters:
        params - A structure used to provide relevant parameters
                 to an EDC mechanism regarding the generation
                 and evaluationof ECCs.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxDefaultOneBitCalculate(
    FX_ECC_PARAMS  *params)
{
    switch (params->cmd)
    {
        case ECC_CMD_ENCODE:
        {
            D_BUFFER    abECC[BYTES_PER_ECC];
            D_BUFFER   *buffer;

            DclAssert(params->data_buffer.buffer);
            DclAssert(params->ecc_buffer);
            
            buffer = &((params->data_buffer.buffer)[params->data_buffer.uOffset]);
            
            FfxEccCalculate(buffer, abECC);
            
            params->ecc_buffer[0] = abECC[0];
            params->ecc_buffer[1] = abECC[1];
            params->ecc_buffer[2] = abECC[2];
            
            break;
        }
        
        case ECC_CMD_VERIFY:
        {
            D_BUFFER        abECC[BYTES_PER_ECC];
            DCLECCRESULT    iResult;
            D_UINT32        numBitErrors;
            D_BUFFER       *buffer;

            DclAssert(params->data_buffer.buffer);
            DclAssert(params->ecc_buffer);
            DclAssert(params->reference_ecc_buffer);
            
            buffer = &((params->data_buffer.buffer)[params->data_buffer.uOffset]);
            abECC[0] = params->ecc_buffer[0];
            abECC[1] = params->ecc_buffer[1];
            abECC[2] = params->ecc_buffer[2];
            
            iResult = FfxEccCorrect(buffer, params->reference_ecc_buffer, abECC);
            switch (iResult)
            {
                case DCLECC_VALID:
                    params->pIoStat->ffxStat = FFXSTAT_SUCCESS;
                    break;
                    
                case DCLECC_ECCCORRECTED:
                case DCLECC_DATACORRECTED:
                    numBitErrors = FfxErrMgrDecodeCorrectedBits(params->pIoStat);
                    numBitErrors++;
                    FfxErrMgrEncodeCorrectedBits(numBitErrors, params->pIoStat);
                    break;
                    
                default:
                    DclAssert(iResult == DCLECC_UNCORRECTABLE);
                    params->pIoStat->ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
                    break;
            }
            
            break;
        }
        
        default:
        {
            DclProductionError();
        }
    }
}

/*  The default one-bit ECC processor:
    (see "ecc.h" for details)
*/
static FX_EDC_PROCESSOR FfxDefaultOneBitECC =
{
    FfxDefaultOneBitCalculate,
    NULL,
    NULL,
    NULL,
    1,
    2,
    256,
    0,
    3,
    TRUE,
    NULL
};

/*---------------------------------------------------------------------------

            SECTION II: Four-bit ECC Generation/Evaluation

---------------------------------------------------------------------------*/

/*  REFACTOR: all the code between this comment and the comment below in
    this file that says "END BCH IMPLEMENTATION" should be placed in a
    separate module. This is because for products that don't use BCH, we
    don't want to bloat the code size with this stuff, and some linkers are
    not smart enough to remove unused code from a used object module.

    Further, this code was derived from an open-source implementation not
    written by Datalight, and the coding style does not measure up to
    Datalight coding standards. This should be repaired.
*/

/*---------------------------------------------------------------------------

                              Description

 *  Encode, decode and routines
 *  Originally based on bch4836.c by Robert Morelos-Zaragoza.
 *
 *  This program derivates from the original bch2.c, which was written
 *  to simulate the encoding/decoding of primitive binary BCH codes.
 *  Part of this program is adapted from a Reed-Solomon encoder/decoder
 *  program,  'rs.c', to the binary case.
 *
 *  rs.c by Simon Rockliff, University of Adelaide, 21/9/89
 *  bch2.c by Robert Morelos-Zaragoza, University of Hawaii, 5/19/92
 *
 * COPYRIGHT NOTICE: This computer program is free for non-commercial purposes.
 * You may implement this program for any non-commercial application. You may
 * also implement this program for commercial purposes, provided that you
 * obtain my written permission. Any modification of this program is covered
 * by this copyright.
 *
 * %%%% Copyright 1994 (c) Robert Morelos-Zaragoza. All rights reserved. %%%%%

---------------------------------------------------------------------------*/


/*  m = order of the field GF(2**10) = 10
    n = 2**10 - 1 = 1023
    t = 4 = error correcting capability
    d = 2*t + 1 = 9 = designed minimum distance
    k = n - deg(g(x)) = 512 = dimension
    p[] = coefficients of primitive polynomial used to generate GF(2**10)
    g[] = coefficients of generator polynomial, g(x)
    alpha_to [] = log table of GF(2**10)
    index_of[] = antilog table of GF(2**10)
    data[] = coefficients of data polynomial, i(x)
    bb[] = coefficients of redundancy polynomial ( x**(12) i(x) ) modulo g(x)
    numerr = number of errors
    errpos[] = error positions
    recd[] = coefficients of received polynomial
    decerror = number of decoding errors (in MESSAGE positions)
*/
#define LEN_ECCBITS 52
#define LEN_ECCREGION 64
#define LEN_DATABITS 4096
#define LEN_EXTRABITS 64

struct tagEdcCalcParams
{
    int m;
    int n;
    int k;
    int t;
    int d;
    char p[14];           /* irreducible polynomial */
    D_INT16 alpha_to[8192];
    D_INT16 index_of[8192];
    D_INT16 g[64];
    D_INT16 numerr;
    D_INT16 errpos[4160];
    int seed;
    D_INT16 elp[13][13];
    D_INT16 l[13];
    D_INT16 u_lu[13];
    D_INT16 s[13];
    D_INT16 root[200];
    D_INT16 loc[200];
    D_INT16 reg[201];
};

typedef D_INT16 aArray13[13];
typedef aArray13 *paArray13;

static void read_p(EDCCALCPARAMS *pEdcCalcParams)
/* Primitive polynomial of degree 13 */
{
char *p = pEdcCalcParams->p;

    p[0] = p[1] = p[3] = p[4] = p[13] = 1;
    p[2] = p[5] = p[6] = p[7] = p[8] = p[9] = 0;
}

static void generate_gf(EDCCALCPARAMS *pEdcCalcParams)
/*  generate GF(2**m) from the irreducible polynomial p(X) in p[0]..p[m]
    lookup tables:  index->polynomial form   alpha_to[] contains j=alpha**i;
    polynomial form -> index form  index_of[j=alpha**i] = i alpha=2 is the
    primitive element of GF(2**m)
*/
{
    register int    i, mask;
    register int    m = pEdcCalcParams->m;
    register int    n = pEdcCalcParams->n;
    D_INT16      *alpha_to = pEdcCalcParams->alpha_to;
    D_INT16      *index_of = pEdcCalcParams->index_of;
    char           *p = pEdcCalcParams->p;

    mask = 1;
    alpha_to[m] = 0;
    for (i = 0; i < m; i++) {
        alpha_to[i] = (D_INT16)mask;
        index_of[alpha_to[i]] = (D_INT16)i;
        if (p[i] != 0)
            alpha_to[m] ^= mask;
        mask <<= 1;
    }
    index_of[alpha_to[m]] = (D_INT16)m;
    mask >>= 1;
    for (i = m + 1; i < n; i++) {
        if (alpha_to[i - 1] >= mask)
          alpha_to[i] = alpha_to[m] ^ ((alpha_to[i - 1] ^ mask) << 1);
        else
          alpha_to[i] = alpha_to[i - 1] << 1;
        index_of[alpha_to[i]] = (D_INT16)i;
    }
    index_of[0] = -1;
}


static void gen_poly(
    EDCCALCPARAMS  *pEdcCalcParams)
{
    D_INT16        *g = pEdcCalcParams->g;

    /*  Use stored 53 bit polynomial instead :
        0    4    8    12   16   20   24   28   32   36   40   44   48   52
        1101 0101 0110 0001 1101 0101 1100 0010 0000 1100 0100 1010 0010 1
    */

    g[0] = g[1] = g[3] = 1;
    g[2] = 0;

    g[4] = g[6] = 0;
    g[5] = g[7] = 1;

    g[8] = g[11] = 0;
    g[9] = g[10] = 1;

    g[12] = g[13] = g[14] = 0;
    g[15] = 1;

    g[16] = g[17] = g[19] = 1;
    g[18] = 0;

    g[20] = g[22] = 0;
    g[21] = g[23] = 1;

    g[24] = g[25] = 1;
    g[26] = g[27] = 0;

    g[28] = g[29] = g[31] = 0;
    g[30] = 1;

    g[32] = g[33] = g[34] = g[35] = 0;

    g[36] = g[37] = 1;
    g[38] = g[39] = 0;

    g[40] = g[42] = g[43] = 0;
    g[41] = 1;

    g[44] = g[46] = 1;
    g[45] = g[47] = 0;

    g[48] = g[49] = g[51] = 0;
    g[50] = 1;

    g[52] = 1;

}


/*-------------------------------------------------------------------
    Local: encode_bch()

    Calculate redundant bits bb[], codeword is c(X) = data(X)*X**(n-k)+ bb(X)
 
    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void encode_bch(
    D_BUFFER       *pDataBuffer,
    unsigned        uDataLength,
    D_BUFFER       *pExtraBuffer,
    unsigned        uExtraLength,
    D_BUFFER       *pCodingBuffer,
    unsigned        uCodingLength,
    EDCCALCPARAMS  *pEdcCalcParams)
{
    D_INT16        *g = pEdcCalcParams->g;
    register int    i, j;
    register int    feedback;
    unsigned        uBitMask, uShiftValue;
    char            local_bb[64];

    for (i = 0; i < LEN_ECCREGION; i++)
        local_bb[i] = 0;

    /*  Process any Extra regions first
        TODO if value passed is LESS than max, pad region
    */
    i = uExtraLength;
    while(i)
    {
        i--;

        uBitMask = 0x001;
        uShiftValue = 0;
        while(uShiftValue < 8)
        {
            feedback = ((pExtraBuffer[i] & uBitMask) >> uShiftValue ) ^ local_bb[LEN_ECCBITS - 1];
            if( feedback != 0 )
            {
                for( j = LEN_ECCBITS - 1; j > 0; j-- )
                {
                    if( g[j] != 0 )
                    {
                        local_bb[j] = local_bb[j - 1] ^ feedback;
                    }
                    else
                    {
                        local_bb[j] = local_bb[j - 1];
                    }
                }
                local_bb[0] = g[0] && feedback;
            }
            else
            {
                for (j = LEN_ECCBITS - 1; j > 0; j--)
                {
                    local_bb[j] = local_bb[j - 1];
                }
                local_bb[0] = 0;
            }
            uBitMask <<= 1;
            uShiftValue++;
        }
    }


    /*  Process the Data region next
        TODO if value passed is LESS than max, pad region
    */
    i = uDataLength;
    while(i)
    {
        i--;

        uBitMask = 0x001;
        uShiftValue = 0;
        while(uShiftValue < 8)
        {
            feedback = ((pDataBuffer[i] & uBitMask) >> uShiftValue ) ^ local_bb[LEN_ECCBITS - 1];

            if( feedback != 0 )
            {
                for( j = LEN_ECCBITS - 1; j > 0; j-- )
                {
                    if( g[j] != 0 )
                    {
                        local_bb[j] = local_bb[j - 1] ^ feedback;
                    }
                    else
                    {
                        local_bb[j] = local_bb[j - 1];
                    }
                }
                local_bb[0] = g[0] && feedback;
            }
            else
            {
                for (j = LEN_ECCBITS - 1; j > 0; j--)
                {
                    local_bb[j] = local_bb[j - 1];
                }
                local_bb[0] = 0;
            }

            uBitMask <<= 1;
            uShiftValue++;
        }
    }


    /*  Convert the bits back into bytes and store in the passed ECC region
    */
    for (i = 0; i < 6; i++ )
    {
        pCodingBuffer[i] = (local_bb[(i<<3)]<<7)
                         + (local_bb[(i<<3)+1]<<6)
                         + (local_bb[(i<<3)+2]<<5)
                         + (local_bb[(i<<3)+3]<<4)
                         + (local_bb[(i<<3)+4]<<3)
                         + (local_bb[(i<<3)+5]<<2)
                         + (local_bb[(i<<3)+6]<<1)
                         + (local_bb[(i<<3)+7]);
    }
    /*  4 bit ECC requires 6.5 bytes; 7th byte is half full, 8th is empty
    */
    pCodingBuffer[6] = (local_bb[48]<<7)
                     + (local_bb[49]<<6)
                     + (local_bb[50]<<5)
                     + (local_bb[51]<<4);
    pCodingBuffer[7] = 0;
}


/*-------------------------------------------------------------------
    Local: decode_bch()

    Simon Rockliff's implementation of Berlekamp's algorithm.

    Assume we have received bits in recd[i], i=0..(n-1).

    Compute the 2*t syndromes by substituting alpha^i into rec(X) and
    evaluating, storing the syndromes in s[i], i=1..2t (leave s[0] zero) .
    Then we use the Berlekamp algorithm to find the error location polynomial
    elp[i].

    If the degree of the elp is >t, then we cannot correct all the errors, and
    we have detected an uncorrectable error pattern. We output the information
    bits uncorrected.

    If the degree of elp is <=t, we substitute alpha^i , i=1..n into the elp
    to get the roots, hence the inverse roots, the error location numbers.
    This step is usually called "Chien's search".

    If the number of errors located is not equal the degree of the elp, then
    the decoder assumes that there are more than t errors and cannot correct
    them, only detect them. We output the information bits uncorrected.
 
    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void decode_bch(
    D_BUFFER       *pDataBuffer,
    unsigned        uDataLength,
    D_BUFFER       *pExtraBuffer,
    unsigned        uExtraLength,
    D_BUFFER       *pCodingBuffer,
    unsigned        uCodingLength,
    D_UINT16       *pBitErrorCount,
    EDCCALCPARAMS  *pEdcCalcParams)
{
    D_INT16        *alpha_to = pEdcCalcParams->alpha_to;
    register int    n = pEdcCalcParams->n;
    paArray13       elp = (paArray13)pEdcCalcParams->elp;
    D_INT16        *index_of = pEdcCalcParams->index_of;
    D_INT16        *u_lu = pEdcCalcParams->u_lu;
    D_INT16        *l = pEdcCalcParams->l;
    D_INT16        *s = pEdcCalcParams->s;
    register int    t = pEdcCalcParams->t;
    D_INT16        *root = pEdcCalcParams->root;
    D_INT16        *reg = pEdcCalcParams->reg;
    D_INT16        *loc = pEdcCalcParams->loc;
    register int    i, j, u, q, t2, count = 0, syn_error = 0;
    D_INT16         d[13];
    D_INT16         recd_data_bit;
    unsigned        uBitMask, uShiftValue, uOffset;

    /*  These parameters may be needed later when this function is used for
        more than one level of EDC protection, but for now they are not used:
    */
    (void)uCodingLength;
    (void)uDataLength;
    (void)uExtraLength;

    t2 = 2 * t;
    *pBitErrorCount = 0;

    /* first form the syndromes
    */
    for (i = 1; i <= t2; i++)
    {
        s[i] = 0;
        for (j = 0; j < (LEN_DATABITS + LEN_ECCBITS + LEN_EXTRABITS); j++)
        {
            /*  Pull out a single received bit from one of the buffers
            */
            if (j < (LEN_ECCBITS))
            {
                uShiftValue = 7 - (j % 8);
                uBitMask = 0x01 << uShiftValue;

                recd_data_bit = (pCodingBuffer[(j / 8)] & uBitMask) >> uShiftValue;
            }
            else if (j < (LEN_DATABITS + LEN_ECCBITS) )
            {
                uOffset = j - (LEN_ECCBITS);

                uShiftValue = 7 - (uOffset % 8);
                uBitMask = 0x01 << uShiftValue;

                recd_data_bit = (pDataBuffer[(uOffset / 8)] & uBitMask) >> uShiftValue;
            }
            else
            {
                uOffset = j - (LEN_DATABITS + LEN_ECCBITS);

                uShiftValue = 7 - (uOffset % 8);
                uBitMask = 0x01 << uShiftValue;

                recd_data_bit = (pExtraBuffer[(uOffset / 8)] & uBitMask) >> uShiftValue;
            }

            if (recd_data_bit != 0)
            {
                s[i] ^= alpha_to[(i * j) % n];
            }
        }

        if (s[i] != 0)
        {
            syn_error = 1; /* set error flag if non-zero syndrome */
        }

/*  Note: If the code is used only for ERROR DETECTION, then
    exit program here indicating the presence of errors.
*/
        /* convert syndrome from polynomial form to index form  */
        s[i] = index_of[s[i]];
    }

    if (syn_error)
    {
        /*  Compute the error location polynomial via the Berlekamp
            iterative algorithm. Following the terminology of Lin and
            Costello's book :   d[u] is the 'mu'th discrepancy, where
            u='mu'+1 and 'mu' (the Greek letter!) is the step number
            ranging from -1 to 2*t (see L&C),  l[u] is the degree of
            the elp at that step, and u_l[u] is the difference between
            the step number and the degree of the elp.
        */
        /* initialise table entries
        */
        d[0] = 0;           /* index form */
        d[1] = s[1];        /* index form */
        elp[0][0] = 0;      /* index form */
        elp[1][0] = 1;      /* polynomial form */
        for (i = 1; i < t2; i++)
        {
            elp[0][i] = -1; /* index form */
            elp[1][i] = 0;  /* polynomial form */
        }
        l[0] = 0;
        l[1] = 0;
        u_lu[0] = -1;
        u_lu[1] = 0;
        u = 0;

        do
        {
            u++;
            if (d[u] == -1)
            {
                l[u + 1] = l[u];
                for (i = 0; i <= l[u]; i++)
                {
                    elp[u + 1][i] = elp[u][i];
                    elp[u][i] = index_of[elp[u][i]];
                }
            } 
            else
                /*  search for words with greatest u_lu[q] for
                    which d[q]!=0
                */
            {
                q = u - 1;
                while ((d[q] == -1) && (q > 0))
                    q--;
                
                /* have found first non-zero d[q]
                */
                if (q > 0)
                {
                    j = q;
                    do
                    {
                        j--;
                        if ((d[j] != -1) && (u_lu[q] < u_lu[j]))
                            q = j;
                    } while (j > 0);
                }

                /*  have now found q such that d[u]!=0 and
                    u_lu[q] is maximum
                */
                
                /* store degree of new elp polynomial
                */
                if (l[u] > l[q] + u - q)
                    l[u + 1] = l[u];
                else
                    l[u + 1] = l[q] + u - q;

                /* form new elp(x) */
                for (i = 0; i < t2; i++)
                    elp[u + 1][i] = 0;
                
                for (i = 0; i <= l[q]; i++)
                {
                    if (elp[q][i] != -1)
                    {
                        elp[u + 1][i + u - q] =
                                   alpha_to[(d[u] + n - d[q] + elp[q][i]) % n];
                    }
                }
                
                for (i = 0; i <= l[u]; i++)
                {
                    elp[u + 1][i] ^= elp[u][i];
                    elp[u][i] = index_of[elp[u][i]];
                }
            }
            u_lu[u + 1] = u - l[u + 1];

            /* form (u+1)th discrepancy
            */
            if (u < t2)
            {
                /* no discrepancy computed on last iteration
                */
                if (s[u + 1] != -1)
                    d[u + 1] = alpha_to[s[u + 1]];
                else
                    d[u + 1] = 0;

            /*  Note indentation misleading!!! */

                for (i = 1; i <= l[u + 1]; i++)
                  if ((s[u + 1 - i] != -1) && (elp[u + 1][i] != 0))
                  {
                    d[u + 1] ^= alpha_to[(s[u + 1 - i]
                                  + index_of[elp[u + 1][i]]) % n];
                  }
              /* put d[u+1] into index form */
              d[u + 1] = index_of[d[u + 1]];
            }
        } while ((u < t2) && (l[u + 1] <= t));

        u++;
        if (l[u] <= t) {/* Can correct errors */
            /* put elp into index form */
            for (i = 0; i <= l[u]; i++)
            {
                elp[u][i] = index_of[elp[u][i]];
            }

            /* Chien search: find roots of the error location polynomial */
            for (i = 1; i <= l[u]; i++)
            {
                reg[i] = elp[u][i];
            }
                count = 0;
            for (i = 1; i <= n; i++) {
                    q = 1;
                for (j = 1; j <= l[u]; j++)
                        if (reg[j] != -1) {
                            reg[j] = (reg[j] + j) % n;
                            q ^= alpha_to[reg[j]];
                        }
                if (!q) {   /* store root and error
                         * location number indices */
                    root[count] = (D_INT16)i;
                    loc[count] = n - i;
                        count++;
                }
            }

            if (count == l[u])
                /* no. roots = degree of elp hence <= t errors */
                for (i = 0; i < l[u]; i++)
                {
                    if (loc[i] < (LEN_ECCBITS))
                    {
                        uShiftValue = 7 - (loc[i] % 8);
                        uBitMask = 0x01 << uShiftValue;

                        pCodingBuffer[(loc[i] / 8)] ^= uBitMask;
                    }
                    else if (loc[i] < (LEN_DATABITS + LEN_ECCBITS) )
                    {
                        uOffset = loc[i] - (LEN_ECCBITS);

                        uShiftValue = 7 - (uOffset % 8);
                        uBitMask = 0x01 << uShiftValue;

                        pDataBuffer[(uOffset / 8)] ^= uBitMask;
                    }
                    else
                    {
                        uOffset = loc[i] - (LEN_DATABITS + LEN_ECCBITS);

                        uShiftValue = 7 - (uOffset % 8);
                        uBitMask = 0x01 << uShiftValue;

                        pExtraBuffer[(uOffset / 8)] ^= uBitMask;
                    }
                    *pBitErrorCount += 1;
                }
            else    /* elp has degree >t hence cannot solve */
            {
                *pBitErrorCount = D_UINT16_MAX;
                /*  printf("Incomplete decoding: errors detected\n"); */
            }
        }
        else
        {
            *pBitErrorCount = D_UINT16_MAX;
        }
    }
}

/*  END BCH IMPLEMENTATION
*/


/*-------------------------------------------------------------------
    Public: FfxInitializeBCH()

    Interface function that allows FlashFX to initialize the BCH
    implementation given in this module.

    Parameters:
        None

    Return Value:
        None
-------------------------------------------------------------------*/
D_BOOL FfxInitializeBCH(
    FX_EDC_PROCESSOR   *pEdcProcessor)
{
    EDCCALCPARAMS      *pEdcCalcParams;

    pEdcCalcParams = DclMemAllocZero(sizeof(*pEdcCalcParams));
    if (!pEdcCalcParams)
        return FALSE;

    /*  Initialize variables and arrays
    */
    pEdcCalcParams->m = 13;
    pEdcCalcParams->n = 8191;
    pEdcCalcParams->k = 4096;
    pEdcCalcParams->t = 4;
    pEdcCalcParams->d = 9;
    read_p(pEdcCalcParams);               /* read generator polynomial g(x) */
    generate_gf(pEdcCalcParams);          /* generate the Galois Field GF(2**m) */
    gen_poly(pEdcCalcParams);             /* Compute the generator polynomial of BCH code */
    pEdcProcessor->pEdcCalcParams = pEdcCalcParams;
    return TRUE;
}


void FfxDestroyBCH(
    FX_EDC_PROCESSOR *pEdcProcessor)
{
    if (pEdcProcessor)
    {
        if (pEdcProcessor->pEdcCalcParams)
            DclMemFree(pEdcProcessor->pEdcCalcParams);
        
        DclMemFree(pEdcProcessor);
    }
}


/*-------------------------------------------------------------------
    Public: FfxDefaultFourBitCalculate()

    This function provides the interface between FlashFX and the
    public-domain BCH four-bit correction algorithm.It uses the
    parameterized EDC mechanism.

    Parameters:
        params - A structure used to provide relevant parameters
                 to an EDC mechanism regarding the generation
                 and evaluationof ECCs.

    Return Value:
        None
-------------------------------------------------------------------*/
void FfxDefaultFourBitCalculate(
    FX_ECC_PARAMS  *params)
{
    D_UINT16        uBitErrorCount;

    switch (params->cmd)
    {
        case ECC_CMD_ENCODE:
        {
            encode_bch( &(params->data_buffer.buffer[params->data_buffer.uOffset]),
                        params->data_buffer.uCount,
                        &(params->metadata_buffer.buffer[params->metadata_buffer.uOffset]),
                        params->metadata_buffer.uCount,
                        params->ecc_buffer,
                        params->ulECCBufferByteCount,
                        params->pEccProcessor->pEdcCalcParams);
            break;
        }
        
        case ECC_CMD_VERIFY:
        {
            D_UINT32 ulNumBitErrors;

            decode_bch( &(params->data_buffer.buffer[params->data_buffer.uOffset]),
                        params->data_buffer.uCount,
                        &(params->metadata_buffer.buffer[params->metadata_buffer.uOffset]),
                        params->metadata_buffer.uCount,
                        params->reference_ecc_buffer,
                        params->ulECCBufferByteCount,
                        &uBitErrorCount,
                        params->pEccProcessor->pEdcCalcParams);

            if( uBitErrorCount )
            {
                if( uBitErrorCount > 4 )
                {
                    params->pIoStat->ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
                }
                else
                {
                    ulNumBitErrors = FfxErrMgrDecodeCorrectedBits(params->pIoStat);
                    ulNumBitErrors += uBitErrorCount;
                    FfxErrMgrEncodeCorrectedBits(ulNumBitErrors, params->pIoStat);
                    params->pIoStat->ffxStat = FFXSTAT_SUCCESS;
                }
            }
            else
            {
                params->pIoStat->ffxStat = FFXSTAT_SUCCESS;
            }
            break;
        }
        
        default:
        {
            DclProductionError();
        }
    }
}

/*  The four-bit ECC processor:
    (see "ecc.h" for details)
*/
static FX_EDC_PROCESSOR FfxDefaultFourBitECC =
{
    FfxDefaultFourBitCalculate,
    FfxInitializeBCH,
    NULL,
    FfxDestroyBCH,
    4,
    5,
    512,
    8,
    8,
    FALSE,
    NULL
};

/*  The smorgasbord of available ECC processors. Note that this has to be
    in order from low bit-correction ratings to high; otherwise the Acquire
    procedure may return a four-bit processor when only a one-bit processor
    is requested. This requirement could be deprecated through the use of
    a more intelligent Acquire procedure; see the REFACTOR comment below.
*/
static FX_EDC_PROCESSOR *apEdcProcessorList[] =
{
    &FfxDefaultOneBitECC,
    &FfxDefaultFourBitECC,
    NULL
};


/*-------------------------------------------------------------------
    Public: FfxGetIndexedEdcProcessor()

    This is a utility function that allows a client to enumerate
    all the EDC processors contained in the EDC Processor List. It
    is primarily for testing purposes. If there is no EDC processor
    at that index, NULL is returned.

    Parameters:
        ulIndex - A value specifying the desired index into the EDC
                  processor array. The indes is compared against the
                  array itself to be sure it is not out of bounds, and
                  if it is, NULL is returned.

    Return Value:
        A pointer to an ECC processor. See "ecc.h" for details.
-------------------------------------------------------------------*/
FX_EDC_PROCESSOR *FfxGetIndexedEdcProcessor(
    D_UINT32 ulIndex)
{
    D_UINT32 ulLocalIndex;

    /*  Find out how big the list is and check the requested index against
        that. If the requested index is out of bounds, return NULL.
    */
    for (ulLocalIndex=0; apEdcProcessorList[ulLocalIndex] != NULL; ulLocalIndex++);
    if (ulIndex < ulLocalIndex)
        return apEdcProcessorList[ulIndex];
    else
        return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxAcquireEccProcessor()

    This function allows FlashFX to peruse the available ECC
    processors and pick one that will serve the needs of a
    particular configuration.

    Parameters:
        uBitsOfCorreciton - A number specifying how powerful the
                            EDC must be.

    Return Value:
        A pointer to an ECC processor. See "ecc.h" for details.
-------------------------------------------------------------------*/
FX_EDC_PROCESSOR *FfxAcquireEdcProcessor(
    D_UINT32 ulBitsOfCorrection)
{
    D_UINT32 index;
  #if ALLOW_INFERIOR_EDC
    D_UINT32 ulBestChoice = D_UINT32_MAX;
    D_UINT32 ulCapabilityDifference = D_UINT32_MAX;
  #endif

    /*  REFACTOR: Right now, the selection of an ECC processor is
        done on the basis of the strength of the correction only.
        It does not at this time consider any other factors such
        as performance, memory footprint, the number of data bytes
        consumed, etc. As of now it isn't necessary to have all
        that stuff, but moving into the future it will be desirable
        to add such capability as new ECC processors become
        available.
    */
    for (index=0; apEdcProcessorList[index]!=NULL; index++)
    {
        D_UINT32 ulCurrentCapability;

        ulCurrentCapability = apEdcProcessorList[index]->ulCorrectionCapability;
        if (ulBitsOfCorrection <= ulCurrentCapability)
        {
            FX_EDC_PROCESSOR *pEdcProcessor;

            /*  Make a copy in memory so processor calculation parameters
                can be allocated, if necessary:
            */
            pEdcProcessor = DclMemAllocZero(sizeof(*pEdcProcessor));
            if (!pEdcProcessor)
                return NULL;
            *pEdcProcessor = *apEdcProcessorList[index];
            return pEdcProcessor;
        }

      #if ALLOW_INFERIOR_EDC
        else
        {
            if ((ulBitsOfCorrection - ulCurrentCapability) <
                ulCapabilityDifference)
            {
                ulCapabilityDifference = ulBitsOfCorrection - ulCurrentCapability;
                ulBestChoice = index;
            }
        }
      #endif

    }

  #if ALLOW_INFERIOR_EDC
    if (ulBestChoice < D_UINT32_MAX)
    {
        FX_EDC_PROCESSOR *pEdcProcessor;

        /*  Make a copy in memory so processor calculation parameters
            can be allocated, if necessary:
        */
        pEdcProcessor = DclMemAllocZero(sizeof(*pEdcProcessor));
        if (!pEdcProcessor)
            return NULL;
        DclPrintf("WARNING: EDC does not meet device requirements.\n");
        DclPrintf(" Protection level: %d bits. Device Requirement: %d bits.\n",
                    apEdcProcessorList[ulBestChoice]->ulCorrectionCapability,
                    ulBitsOfCorrection);
        *pEdcProcessor = *apEdcProcessorList[ulBestChoice];
        return pEdcProcessor;
    }
  #endif

    return NULL;
}

