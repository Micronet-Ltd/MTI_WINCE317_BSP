/*++

Copyright (c) 2001-2008  Future Technology Devices International Ltd.

Module Name:

    baud.c

Abstract:

    Native USB device driver for FTDI FT8U2XX
    Utility routines

Environment:

    kernel mode only

Revision History:

    27/11/01    awm     Incorporated from D2XX driver.
    11/01/02    awm     Fixed divide-by-zero problem.
    15/10/02    awm     Support for BM.
    30/07/04    awm     Ensure BM fractions only applied to BM device.
	13/08/04	st		Changes for AM baud rate conversion.
	04/06/08	ma		Added hi-speed chip support.


--*/


#ifdef WINCE
#include <windows.h>
#else
#include "precomp.h"
#endif


//
// definitions for baud rate divisor calculations
//

#define FT_CLOCK_RATE      3000000L
#define FT_CLOCK_RATE_HI	12000000L

#define FT_SUB_INT_0_0     0x0000
#define FT_SUB_INT_0_125   0xc000
#define FT_SUB_INT_0_25    0x8000
#define FT_SUB_INT_0_5     0x4000
#define FT_SUB_INT_MASK    0xc000

//
// BM definitions
//
#define FT_SUB_INT_0_375   0x0000
#define FT_SUB_INT_0_625   0x4000
#define FT_SUB_INT_0_75    0x8000
#define FT_SUB_INT_0_875   0xc000


SHORT FT_CalcDivisor(ULONG rate,USHORT *divisor,USHORT *ext_div,ULONG bm);
SHORT FT_CalcDivisorHi(ULONG rate,USHORT *divisor,USHORT *ext_div);
ULONG FT_CalcBaudRate(USHORT divisor,USHORT ext_div,ULONG bm);
ULONG FT_CalcBaudRateHi(USHORT divisor,USHORT ext_div);

SHORT FT_GetDivisor(
    ULONG rate,
    USHORT *divisor,
    USHORT *ext_div,
    ULONG *actual,
    SHORT *accuracy,
    SHORT *plus,
    ULONG bm
    )
/*++

Routine Description:

    Get divisor for baud rate. This function can optionally return
    the actual baud rate and accuracy that the divisor represents.
    The accuracy must be within the limits +/- 3% for the divisor
    to be usable.

Arguments:

    rate     - baud rate
    divisor  - pointer to storage to hold divisor
    ext_div  - pointer to storage to hold external divisor
    actual   - optional pointer to storage to hold actual baud rate
    accuracy - optional pointer to storage to hold accuracy value
    plus     - optional pointer to storage to hold +/- indication
    bm       - if non-zero use of BM fractional parts is allowed

Return Value:

    -1  Invalid baud rate - unable to calculate divisor
    0   Calculated baud rate outwith limit
    1   OK

--*/
{
    ULONG temp_actual;
    SHORT temp_accuracy;
    SHORT temp_mod;
    SHORT temp_plus;
    SHORT rval;

    if (!divisor || !ext_div)
        return 0;   // must have pointer to divisor and ext_div

    if ((rval = FT_CalcDivisor(rate,divisor,ext_div,bm)) == -1)
        return -1;  // unable to calculate baud rate (<= 183 baud?)

    if (rval == 0)
        *divisor = (*divisor & ~FT_SUB_INT_MASK) + 1;

    temp_actual = FT_CalcBaudRate(*divisor,*ext_div,bm);

    if (rate > temp_actual) {
        temp_accuracy = ((SHORT)((rate*100)/temp_actual))-100;
        temp_mod = (SHORT)(((rate%temp_actual)*100) % temp_actual);
        temp_plus = 0;
    }
    else {
        temp_accuracy = ((SHORT)((temp_actual*100)/rate))-100;
        temp_mod = (SHORT)(((temp_actual%rate)*100) % rate);
        temp_plus = 1;
    }

    if (actual)
        *actual = temp_actual;

    if (accuracy)
        *accuracy = temp_accuracy;

    if (plus)
        *plus = temp_plus;

    if (temp_accuracy < 3)
        rval = 1;
    else if (temp_accuracy == 3 && temp_mod == 0)
        rval = 1;
    else
        rval = 0;

    return rval;
}


SHORT FT_CalcDivisor(ULONG rate,USHORT *divisor,USHORT *ext_div,ULONG bm)
/*++

Routine Description:

    Calculates divisor for baud rate. Used in conjunction with
    FT_GetDivisor, if the modifier can't be rounded down,
    it is the responsibility of FT_GetDivisor to round up
    to the next value.

Arguments:

    rate     - baud rate
    divisor  - pointer to storage to hold divisor
    ext_div  - pointer to storage to hold external divisor
    bm       - if non-zero use of BM fractional parts is allowed

Return Value:

    -1  Not calculable
    0   Not available
    1   OK

--*/
{
    USHORT t;
    USHORT modifier;
    SHORT ok = 1;

    if (rate == 0)
        return -1;  // can't calculate divisor

    if ((FT_CLOCK_RATE/rate) & 0xffffc000)
        return -1;  // overflow, so can't calculate divisor

    *divisor = (USHORT) (FT_CLOCK_RATE / rate);
    *ext_div = 0;

    // 3000000 is a special case, and must return 0
    if (*divisor == 1) {
        t = (USHORT) (((FT_CLOCK_RATE % rate) * 100) / rate);
        if (t <= 3)
            *divisor = 0;
    }

    if (*divisor == 0)
        return ok;

    t = (USHORT) (((FT_CLOCK_RATE % rate) * 100) / rate);

    if (!bm) {

        if (t <= 6)
            modifier = FT_SUB_INT_0_0;
        else if (t <= 18)
            modifier = FT_SUB_INT_0_125;
        else if (t <= 37)
            modifier = FT_SUB_INT_0_25;
        else if (t <= 75)
            modifier = FT_SUB_INT_0_5;
        else {
            modifier = 0;
            ok = 0;
        }

    }
    else {

        if (t <= 6)
            modifier = FT_SUB_INT_0_0;
        else if (t <= 18)
            modifier = FT_SUB_INT_0_125;
        else if (t <= 31)
            modifier = FT_SUB_INT_0_25;
        else if (t <= 43) {
            modifier = FT_SUB_INT_0_375;
            *ext_div = 1;
        }
        else if (t <= 56)
            modifier = FT_SUB_INT_0_5;
        else if (t <= 68) {
            modifier = FT_SUB_INT_0_625;
            *ext_div = 1;
        }
        else if (t <= 81) {
            modifier = FT_SUB_INT_0_75;
            *ext_div = 1;
        }
        else if (t <= 93) {
            modifier = FT_SUB_INT_0_875;
            *ext_div = 1;
        }
        else {
            modifier = 0;
            ok = 0;
        }

    }

    *divisor |= modifier;

    return ok;
}


ULONG FT_CalcBaudRate(USHORT divisor,USHORT ext_div,ULONG bm)
/*++

Routine Description:

    Calculates baud rate from divisor.

Arguments:

    divisor  - the divisor
    ext_div  - the external divisor
    bm       - if non-zero use of BM fractional parts is allowed

Return Value:

    Baud rate

--*/
{
    ULONG rate;

    if (divisor == 0)
        return FT_CLOCK_RATE;

    rate = (divisor & ~FT_SUB_INT_MASK) * 100;

    if (!bm) {

        switch (divisor & FT_SUB_INT_MASK) {
        default :
            break;
        case FT_SUB_INT_0_125 :
            rate += 12;
            break;
        case FT_SUB_INT_0_25 :
            rate += 25;
            break;
        case FT_SUB_INT_0_5 :
            rate += 50;
            break;
        }

    }
    else {

        if (ext_div == 0) {
            switch (divisor & FT_SUB_INT_MASK) {
            default :
                break;
            case FT_SUB_INT_0_125 :
                rate += 12;
                break;
            case FT_SUB_INT_0_25 :
                rate += 25;
                break;
            case FT_SUB_INT_0_5 :
                rate += 50;
                break;
            }
        }
        else {
            switch (divisor & FT_SUB_INT_MASK) {
            default :
                break;
            case FT_SUB_INT_0_375 :
                rate += 37;
                break;
            case FT_SUB_INT_0_625 :
                rate += 62;
                break;
            case FT_SUB_INT_0_75 :
                rate += 75;
                break;
            case FT_SUB_INT_0_875 :
                rate += 87;
                break;
            }
        }

    }

    rate = (FT_CLOCK_RATE*100)/rate;

    return rate;
}

SHORT FT_GetDivisorHi(
    ULONG rate,
    USHORT *divisor,
    USHORT *ext_div,
    ULONG *actual,
    SHORT *accuracy,
    SHORT *plus
    )
/*++

Routine Description:

    Get divisor for baud rate. This function can optionally return
    the actual baud rate and accuracy that the divisor represents.
    The accuracy must be within the limits +/- 3% for the divisor
    to be usable.

Arguments:

    rate     - baud rate
    divisor  - pointer to storage to hold divisor
    ext_div  - pointer to storage to hold external divisor
    actual   - optional pointer to storage to hold actual baud rate
    accuracy - optional pointer to storage to hold accuracy value
    plus     - optional pointer to storage to hold +/- indication

Return Value:

    -1  Invalid baud rate - unable to calculate divisor
    0   Calculated baud rate outwith limit
    1   OK

--*/
{
    ULONG temp_actual;
    SHORT temp_accuracy;
    SHORT temp_mod;
    SHORT temp_plus;
    SHORT rval;

    if (!divisor || !ext_div)
        return 0;   // must have pointer to divisor and ext_div

    if ((rval = FT_CalcDivisorHi(rate,divisor,ext_div)) == -1)
        return -1;  // unable to calculate baud rate (<= 183 baud?)

    if (rval == 0)
        *divisor = (*divisor & ~FT_SUB_INT_MASK) + 1;

    temp_actual = FT_CalcBaudRateHi(*divisor,*ext_div);

    if (rate > temp_actual) {
        temp_accuracy = ((SHORT)((rate*100)/temp_actual))-100;
        temp_mod = (SHORT)(((rate%temp_actual)*100) % temp_actual);
        temp_plus = 0;
    }
    else {
        temp_accuracy = ((SHORT)((temp_actual*100)/rate))-100;
        temp_mod = (SHORT)(((temp_actual%rate)*100) % rate);
        temp_plus = 1;
    }

    if (actual)
        *actual = temp_actual;

    if (accuracy)
        *accuracy = temp_accuracy;

    if (plus)
        *plus = temp_plus;

    if (temp_accuracy < 3)
        rval = 1;
    else if (temp_accuracy == 3 && temp_mod == 0)
        rval = 1;
    else
        rval = 0;

    return rval;
}


SHORT FT_CalcDivisorHi(ULONG rate,USHORT *divisor,USHORT *ext_div)
/*++

Routine Description:

    Calculates divisor for baud rate. Used in conjunction with
    FT_GetDivisor, if the modifier can't be rounded down,
    it is the responsibility of FT_GetDivisor to round up
    to the next value.

Arguments:

    rate     - baud rate
    divisor  - pointer to storage to hold divisor
    ext_div  - pointer to storage to hold external divisor

Return Value:

    -1  Not calculable
    0   Not available
    1   OK

--*/
{
    USHORT t;
    USHORT modifier;
    SHORT ok = 1;

    if (rate == 0)
        return -1;  // can't calculate divisor

    if ((FT_CLOCK_RATE_HI/rate) & 0xffffc000)
        return -1;  // overflow, so can't calculate divisor

    *ext_div = 0x0002;

    //
    // 12000000 and 8000000 are special cases
    //
    if (rate >= 11640000 && rate <= 12360000) {
        *divisor = 0;
        return ok;
    }

    if (rate >= 7760000 && rate <= 8240000) {
        *divisor = 1;
        return ok;
    }

    *divisor = (USHORT) (FT_CLOCK_RATE_HI / rate);
    *ext_div = 0x0002;

    // 12000000 is a special case, and must return 0
    if (*divisor == 1) {
        t = (USHORT) (((FT_CLOCK_RATE_HI % rate) * 100) / rate);
        if (t <= 3)
            *divisor = 0;
    }

    if (*divisor == 0)
        return ok;

    t = (USHORT) (((FT_CLOCK_RATE_HI % rate) * 100) / rate);

	if (t <= 6)
		modifier = FT_SUB_INT_0_0;
    else if (t <= 18)
        modifier = FT_SUB_INT_0_125;
    else if (t <= 31)
        modifier = FT_SUB_INT_0_25;
    else if (t <= 43) {
		modifier = FT_SUB_INT_0_375;
		*ext_div |= 1;
    }
    else if (t <= 56)
		modifier = FT_SUB_INT_0_5;
    else if (t <= 68) {
		modifier = FT_SUB_INT_0_625;
		*ext_div |= 1;
    }
    else if (t <= 81) {
		modifier = FT_SUB_INT_0_75;
        *ext_div |= 1;
	}
	else if (t <= 93) {
		modifier = FT_SUB_INT_0_875;
        *ext_div |= 1;
	}
	else {
		modifier = 0;
        ok = 0;
	}

    *divisor |= modifier;

    return ok;
}


ULONG FT_CalcBaudRateHi(USHORT divisor,USHORT ext_div)
/*++

Routine Description:

    Calculates baud rate from divisor.

Arguments:

    divisor  - the divisor
    ext_div  - the external divisor

Return Value:

    Baud rate

--*/
{
    ULONG rate;

    if (divisor == 0)
        return FT_CLOCK_RATE_HI;

    if (divisor == 1)
        return 8000000;

    rate = (divisor & ~FT_SUB_INT_MASK) * 100;

	ext_div &= 0xfffd;

	if (ext_div == 0) {
		switch (divisor & FT_SUB_INT_MASK) {
		default :
			break;
		case FT_SUB_INT_0_125 :
			rate += 12;
			break;
		case FT_SUB_INT_0_25 :
			rate += 25;
			break;
		case FT_SUB_INT_0_5 :
			rate += 50;
			break;
		}
	}
	else {
		switch (divisor & FT_SUB_INT_MASK) {
		default :
			break;
		case FT_SUB_INT_0_375 :
			rate += 37;
			break;
		case FT_SUB_INT_0_625 :
			rate += 62;
			break;
		case FT_SUB_INT_0_75 :
			rate += 75;
			break;
		case FT_SUB_INT_0_875 :
			rate += 87;
			break;
		}
	}

    rate = (FT_CLOCK_RATE_HI*100)/rate;

    return rate;
}










