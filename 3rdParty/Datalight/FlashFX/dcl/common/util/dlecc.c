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

    This is a generalized implementation of the ECC algorithm commonly used
    with NAND flash memory to correct single-bit errors and detect double-bit
    errors.  It is based on the original implementation, but is extended to
    allow sizes other than 256 bytes, and has some performance optimizations.

    A fairly good explanation (with nice pictures) of how this ECC algorithm
    works may be found in application notes at:

    http://www.samsung.com/Products/Semiconductor/Memory/appnote.htm#flashsoftware

    Briefly explained: the ECC works by treating the data to be protected as a
    bit stream.  Each bit is assigned an address, starting at zero.  The bits
    in the binary encoding of the address are used to select subsets of the
    data bits.  For each address bit, there are two subsets of the data bits:
    one set is all the data bits that have a one in their address at that bit
    position, the other is the data bits that have a zero there.

    A parity bit is generated for each subset: a one if the set of data bits
    has even parity, a zero if odd.  Thus there are two parity bits for each
    address bit.  The convention is to name the parity bits by the *value*
    (not bit position) of the address bit, and append a "'" to designate
    parity for the bits whose address bit is zero.  So, for example, P1 is the
    parity of bits 1, 3, 5, ..., P1' is the parity of bits 0, 2, 4, ..., P2 is
    the parity of bits 2, 3, 6, 7, ..., P4' is the parity of bits 0, 1, 2, 3,
    8, 9, 10, 11, ..., etc.

    Because Pn and Pn' are the parity for complementary sets of bits, if a
    single data bit is changed it will cause either Pn or Pn' to change.  So
    determining that a single bit error has occurred is easy: if the stored
    ECC differs from the ECC recalculated from the data bits in one bit of
    each Pn, Pn' pair, there has been single bit error.

    Locating the changed bit is also easy: its address is given by the
    exclusive or of the Pn bits in the stored ECC and the recalculated ECC.

    If the stored and recalculated ECCs differ in exactly one bit, there is a
    single bit error in the stored ECC itself.

    Any other difference between the stored and recalculated ECCs signifies a
    multiple bit error.

    How the data bits are addressed and where the parity bits are stored
    varies depending on the context in which this algorithm is used; the
    Samsung application notes described their standards.  Generally, each Pn
    is stored adjacent to its corresponding Pn'.

    This implementation uses a "canonical" form of storing the parity bits in
    a 32-bit unsigned integer.  Each Pn, Pn' pair is stored as two adjacent
    bits, with the Pn bit in the higher-order position.  The Pn, Pn' pairs are
    stored with lower n in lower-order position.  Thus the low-order bit of
    the 32-bit unsigned integer is P1', the next bit up is P1, then P2', all
    the way up to (potentially) P32768 in the highest-order bit.  Bits that
    are unused (corresponding to address bits beyond the size covered by the
    ECC) are all zeros.  Thus, for example, the canonical form of the ECC for
    256 bytes (2048 bits) is:

          31  30        22      21       20           3    2    1     0
        +---+---+-/ /-+---+-------+--------+-/ /-+----+-----+----+-----+
        | 0 | 0 | ... | 0 | P1024 | P1024' | ... | P2 | P2' | P1 | P1' |
        +---+---+-/ /-+---+-------+--------+-/ /-+----+-----+----+-----+

    Generally, the canonical form needs to have its bits rearranged into the
    format used on the media.  Often, this is described in terms of "column"
    and "line" parity.  Column parity is refers to the parity of bit positions
    within bytes or words, and line parity is the parity of sets of complete
    bytes or words.

    Note that byte order issues in the calculations can be resolved by swapping
    Pn, Pn' pairs (which is equivalent to inverting the corresponding address
    bit).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlecc.c $
    Revision 1.8  2010/10/11 22:08:18Z  glenns
    Fix erroneous bitmask in DclEccCorrect that caused the wrong
    bit to be corrected if the bit error is found in the high nibble of a
    byte. 
    Revision 1.7  2009/10/03 00:36:52Z  garyp
    Added DclEccCorrect().  Renamed DclEccGenerate() to DclEccCalculate()
    for consistency.  Slightly changed the behavior of DclEccCalculate() to 
    ensure that any unused high-order bits are set to zero.
    Revision 1.6  2009/02/26 22:34:09Z  keithg
    Added typecasts in auto assignments of ~0 to placate compiler warns.
    Revision 1.5  2007/12/18 04:07:04Z  brandont
    Updated function headers.
    Revision 1.4  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.3  2007/05/03 01:51:18Z  pauli
    Added an explicit cast to resolve a build warning with the ADS 1.2 tools.
    Revision 1.2  2007/03/10 04:55:04Z  Garyp
    Documentation/header cleanup.
    Revision 1.1  2007/02/06 22:42:48Z  billr
    Initial revision
    Based on flashfx/fmsl/bbm/ecc.c revision 1.2
---------------------------------------------------------------------------*/

#include <dcl.h>

#define MAX_ECC_DATA_LEN        (8192U)

/*  The All-Bit-XOR is the exclusive OR of all 8 data bits.  When the
    All-Bit-XOR is 1, the current line number (sector data position)
    is used in the line parity calculation.  Bit 6 (0x40) of each 8
    bit value in the table given below (abCPTable) represents the
    pre-computed All-Bit-XOR for that data byte.
*/
#define ALL_BIT_EOR             (0x40)

/*-------------------------------------------------------------------
    This is a table of pre-computed column parity values (CP0 - CP5).
    Each 8 bit data value (0 - 255) serves as an index into this
    table to get the corresponding column parity value for that data
    value.  The least significant 6 bits (0 - 5) are the column
    parity value.  Bit 6 is the all-bit-XOR (exclusive OR of all 8
    data bits D0 -D7) and determines whether or not the current line
    number (sector data position) is used in the line parity
    calculation.  Bit 7 is always zero.
-------------------------------------------------------------------*/
static const D_BYTE abCPTable[] =
{
    0x00, 0x55, 0x56, 0x03, 0x59, 0x0C, 0x0F, 0x5A,
    0x5A, 0x0F, 0x0C, 0x59, 0x03, 0x56, 0x55, 0x00,
    0x65, 0x30, 0x33, 0x66, 0x3C, 0x69, 0x6A, 0x3F,
    0x3F, 0x6A, 0x69, 0x3C, 0x66, 0x33, 0x30, 0x65,
    0x66, 0x33, 0x30, 0x65, 0x3F, 0x6A, 0x69, 0x3C,
    0x3C, 0x69, 0x6A, 0x3F, 0x65, 0x30, 0x33, 0x66,
    0x03, 0x56, 0x55, 0x00, 0x5A, 0x0F, 0x0C, 0x59,
    0x59, 0x0C, 0x0F, 0x5A, 0x00, 0x55, 0x56, 0x03,
    0x69, 0x3C, 0x3F, 0x6A, 0x30, 0x65, 0x66, 0x33,
    0x33, 0x66, 0x65, 0x30, 0x6A, 0x3F, 0x3C, 0x69,
    0x0C, 0x59, 0x5A, 0x0F, 0x55, 0x00, 0x03, 0x56,
    0x56, 0x03, 0x00, 0x55, 0x0F, 0x5A, 0x59, 0x0C,
    0x0F, 0x5A, 0x59, 0x0C, 0x56, 0x03, 0x00, 0x55,
    0x55, 0x00, 0x03, 0x56, 0x0C, 0x59, 0x5A, 0x0F,
    0x6A, 0x3F, 0x3C, 0x69, 0x33, 0x66, 0x65, 0x30,
    0x30, 0x65, 0x66, 0x33, 0x69, 0x3C, 0x3F, 0x6A,

    0x6A, 0x3F, 0x3C, 0x69, 0x33, 0x66, 0x65, 0x30,
    0x30, 0x65, 0x66, 0x33, 0x69, 0x3C, 0x3F, 0x6A,
    0x0F, 0x5A, 0x59, 0x0C, 0x56, 0x03, 0x00, 0x55,
    0x55, 0x00, 0x03, 0x56, 0x0C, 0x59, 0x5A, 0x0F,
    0x0C, 0x59, 0x5A, 0x0F, 0x55, 0x00, 0x03, 0x56,
    0x56, 0x03, 0x00, 0x55, 0x0F, 0x5A, 0x59, 0x0C,
    0x69, 0x3C, 0x3F, 0x6A, 0x30, 0x65, 0x66, 0x33,
    0x33, 0x66, 0x65, 0x30, 0x6A, 0x3F, 0x3C, 0x69,
    0x03, 0x56, 0x55, 0x00, 0x5A, 0x0F, 0x0C, 0x59,
    0x59, 0x0C, 0x0F, 0x5A, 0x00, 0x55, 0x56, 0x03,
    0x66, 0x33, 0x30, 0x65, 0x3F, 0x6A, 0x69, 0x3C,
    0x3C, 0x69, 0x6A, 0x3F, 0x65, 0x30, 0x33, 0x66,
    0x65, 0x30, 0x33, 0x66, 0x3C, 0x69, 0x6A, 0x3F,
    0x3F, 0x6A, 0x69, 0x3C, 0x66, 0x33, 0x30, 0x65,
    0x00, 0x55, 0x56, 0x03, 0x59, 0x0C, 0x0F, 0x5A,
    0x5A, 0x0F, 0x0C, 0x59, 0x03, 0x56, 0x55, 0x00
};

/*  Lookup table used when merging the complementary parity bits generated
    from the byte offsets.  Two sets of bits are calculated which must then
    be interleaved.  This table is indexed by a set of four adjacent bits,
    and generates an eight-bit result by inserting zero bits between the
    supplied bits, plus a high-order zero.
*/
static D_BYTE abMerge[16] =
{
    0x00, /* 0000 -> 0o0 o0o0 */
    0x01, /* 0001 -> 0o0 o0o1 */
    0x04, /* 0010 -> 0o0 o1o0 */
    0x05, /* 0011 -> 0o0 o1o1 */
    0x10, /* 0100 -> 0o1 o0o0 */
    0x11, /* 0101 -> 0o1 o0o1 */
    0x14, /* 0110 -> 0o1 o1o0 */
    0x15, /* 0111 -> 0o1 o1o1 */
    0x40, /* 1000 -> 1o0 o0o0 */
    0x41, /* 1001 -> 1o0 o0o1 */
    0x44, /* 1010 -> 1o0 o1o0 */
    0x45, /* 1011 -> 1o0 o1o1 */
    0x50, /* 1100 -> 1o1 o0o0 */
    0x51, /* 1101 -> 1o1 o0o1 */
    0x54, /* 1110 -> 1o1 o1o0 */
    0x55  /* 1111 -> 1o1 o1o1 */
};

#define EVENBITS    (0x55555555UL)


/*-------------------------------------------------------------------
    Public: DclEccCalculate()

    Generate an ECC in canonical form.  The generated ECC has two
    bits per significant bit of bit-level address, so the length
    in bytes is limited to 8K.

    Note that the canonical form used here will set any unused
    high-order bits to zero.

    Parameters:
        pData        - An array of data for which the ECC is to be
                       computed
        nLength      - The length of the data in bytes.  It must a
                       a power-of-two value not exceeding 8 KB.

    Return Value:
        Returns the ECC value in canonical form.
-------------------------------------------------------------------*/
D_UINT32 DclEccCalculate(
    const void     *pData,
    size_t          nLength)
{
    const D_BYTE   *p = pData;
    D_BYTE          bCP = (D_BYTE)~0;
    unsigned        nLP1 = (unsigned)~0;    /* Pn bits */
    unsigned        nLP0 = (unsigned)~0;    /* Pn' bits */
    unsigned        index;
    D_UINT32        ulEcc;

    /*  The parity bits initialized above are TRUE to indicate
        even parity, so they must all start as ones.
    */

    DclAssert(pData);
    DclAssert(nLength);
    DclAssert(nLength <= MAX_ECC_DATA_LEN);
    DclAssert(DCLISPOWEROF2(nLength));

    DclProfilerEnter("DclEccCalculate", 0, 0);

    for (index = 0; index < nLength; ++index)
    {
        /*  Accumulate pre-computed column parities (CP0 - CP5) from the ECC
            column parity table.  The currently selected sector data value is
            used as an index into the table.  The ALL_BIT_EOR flag is masked
            off later.
        */
        bCP ^= abCPTable[p[index]];

        /*  Check the All-Bit-EOR - exclusive OR of all 8 data bits (D0 - D7)
        */
        if (abCPTable[p[index]] & ALL_BIT_EOR)
        {
            /*  When the All-Bit-EOR is 1, the current line number (sector data
                position) is used in the line parity calculations.
            */
            nLP1 ^= index;
            nLP0 ^= ~index;
        }
    }

    /*  Combine the line and column parities.  This is done strictly by
        lookup, shifting and masking, no loops or conditionals.
    */
    ulEcc  = abMerge[(nLP1 >> 12) & 0xF] << 1;
    ulEcc |= abMerge[(nLP0 >> 12) & 0xF];

    ulEcc <<= 8;
    ulEcc |= abMerge[(nLP1 >>  8) & 0xF] << 1;
    ulEcc |= abMerge[(nLP0 >>  8) & 0xF];

    ulEcc <<= 8;
    ulEcc |= abMerge[(nLP1 >>  4) & 0xF] << 1;
    ulEcc |= abMerge[(nLP0 >>  4) & 0xF];

    ulEcc <<= 8;
    ulEcc |= abMerge[nLP1 & 0xF] << 1;
    ulEcc |= abMerge[nLP0 & 0xF];

    ulEcc <<= 6;
    ulEcc |= bCP & 0x3F;

    /*  Mask off any high-order bits which are not used
    */
    ulEcc &= (D_UINT32_MAX >> (32-((DclBitHighest(nLength) + 2) << 1)));

    DclProfilerLeave(0UL);

    return ulEcc;
}


/*-------------------------------------------------------------------
    Public: DclEccCorrect()

    Compare a calculated ECC with the stored ECC and correct a data
    stream or the stored ECC as appropriate.

    If any differences exist between the two ECCs, it is determined
    whether it is a correctable data error (i.e., all parity bit
    pairs have 1 error and 1 match), a correctable error in the ECC
    itself (i.e., only one bit is different), or an uncorrectable
    error.  If it is one of the correctable types, then the error
    is corrected in the data or ECC value, as appropriate.

    Note that the canonical form used here requires that any unused
    high-order bits be set to zero.

    Parameters:
        pData     - An array of the ECC data to be checked.
        nLength   - The data buffer length, which must be a power-
                    of-two value.
        pulOldECC - A pointer to the ECC which was originally
                    calculated for the data (in canonical form).
        ulNewECC  - The ECC for the supplied data bytes, in canonical
                    form.

    Return Value:
        Returns a DCLECCRESULT value which will be one of the
        following values:
          DCLECC_VALID         - The data and old ECC value had no
                                 errors.
          DCLECC_DATACORRECTED - The data was corrected.
          DCLECC_ECCCORRECTED  - The old ECC value was corrected.
          DCLECC_UNCORRECTABLE - The data and/or old ECC has
                                 uncorrectable errors.
-------------------------------------------------------------------*/
DCLECCRESULT DclEccCorrect(
    D_BUFFER       *pData,
    size_t          nLength,
    D_UINT32       *pulOldECC,
    D_UINT32        ulNewECC)
{
    DCLECCRESULT    iResult;
    D_UINT32        ulValidMask;

    DclAssert(pData);
    DclAssert(pulOldECC);
    DclAssert(nLength);
    DclAssert(nLength <= MAX_ECC_DATA_LEN);
    DclAssert(DCLISPOWEROF2(nLength));

    DclProfilerEnter("DclEccCorrect", 0, 0);

    /*  Mask off any high-order bits which are not used
    */
    ulValidMask = (D_UINT32_MAX >> (32-((DclBitHighest(nLength) + 2) << 1)));

    /*  Since any stray bits in the high-order unused area are
        probably indicative of a higher level munging error,
        assert so the programmer gets a "heads-up" warning.
    */
    DclAssert((ulNewECC & ~ulValidMask) == 0);
    DclAssert((*pulOldECC & ~ulValidMask) == 0);

    if(ulNewECC == *pulOldECC)
    {
        /*  Flag it as no error -- both the data and the ECC are valid.
        */
        iResult = DCLECC_VALID;
    }
    else
    {
        D_UINT32    ulECCTemp;

        ulECCTemp = ulNewECC ^ *pulOldECC;

        /*  If only a single bit is different between the new and old
            ECC values, then the error is in the old ECC value itself.
        */
        if(DCLISPOWEROF2(ulECCTemp))
        {
            /*  Correct the ECC code error by restoring the calculated ECC.
            */
            *pulOldECC = ulNewECC;

            /*  Flag it as a corrected error in the ECC.
            */
            iResult = DCLECC_ECCCORRECTED;
        }
        else
        {
            /*  Normalize the number by filling in any unused high-order
                bits with our even bits pattern.
            */
            ulECCTemp |= (EVENBITS & ~ulValidMask);

            /*  Determine whether the error is correctable.
            */
            if(((ulECCTemp ^ (ulECCTemp >> 1)) & EVENBITS) == EVENBITS)
            {
                D_UINT32    ulEvenBits = 0;
                unsigned    nIndex = 0;
                unsigned    nDataByteNumber;
                unsigned    nBitNumber;

                ulECCTemp ^= EVENBITS;
                ulECCTemp &= EVENBITS;

                /*  Extract the even bits, which will result in the bit
                    address of the bit in the data which must be flipped.
                */
                while(ulECCTemp)
                {
                    ulEvenBits |= ((ulECCTemp & 1) << nIndex);
                    ulECCTemp >>= 2;
                    nIndex++;
                }

                /*  Determine which byte to correct, and the bit within
                    that byte.
                */
                nDataByteNumber = ulEvenBits >> 3;
                nBitNumber = ulEvenBits & 7;

                /*  Perform error correction -- complement specified bit.
                */
                pData[nDataByteNumber] ^= (1 << nBitNumber);

                /*  Flag it as a corrected error in the data
                */
                iResult = DCLECC_DATACORRECTED;
            }
            else
            {
                /*  Flag it as an uncorrectable error.
                */
                iResult = DCLECC_UNCORRECTABLE;
            }
        }
    }

    DclProfilerLeave(0UL);

    return iResult;
}



