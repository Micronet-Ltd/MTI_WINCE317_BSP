/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module contains code to generate random numbers.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlrand.c $
    Revision 1.13  2011/09/12 07:32:09Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.12  2010/02/02 19:29:03Z  keithg
    Replaced missing pointer initialization cut from prior revision.
    Revision 1.11  2010/02/01 16:58:21Z  keithg
    Tempered the number generator's return value to produce a more
    acceptable distribution of random bits.
    Revision 1.10  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.9  2009/11/11 02:23:52Z  keithg
    Clarified comments, corrected error that caused the global seed to
    not get set on subsequent calls resulting in the same random word.
    Revision 1.8  2009/10/19 19:58:02Z  garyp
    Documentation updates -- no functional changes.
    Revision 1.7  2009/10/14 23:33:26Z  keithg
    Added 64bit rand function, removed initialization from DclRand() and
    placed it into the service initialization fixing bug 705.
    Revision 1.6  2007/12/18 19:22:58Z  brandont
    Updated function headers.
    Revision 1.5  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.4  2006/10/13 00:57:18Z  Garyp
    Modified to allow the initial and subsequent seeds to be zero.  Added the
    ability to use an internal seed.
    Revision 1.3  2006/10/11 21:11:57Z  brandont
    Removed assert for passing a seed of zero which should work fine
    and could potentially be returned by this function for future calls.
    Revision 1.2  2006/01/09 05:04:29Z  Garyp
    Debug code updated.
    Revision 1.1  2005/11/25 23:13:26Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlservice.h>
#include <dlinstance.h>

/*  This is the global seed used by the random number generator when
    non-reproducible sequences are requested.  Specifically this is
    used when the caller has not provided a seed to either DclRand()
    or DclRand64() function.
*/
static D_UINT64 ullGlobalRandomNumberSeed = UINT64SUFFIX(0x0123456789ABCDEF);


/*-------------------------------------------------------------------
    Private: DclRandomServiceInit()

    Initialize the random number generator service.  This is needed
	only to seed the generator for those applications requiring a
	unreproducible sequence.

    Parameters:
        hDclInst - The DCL instance handle.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclRandomServiceInit(
    DCLINSTANCEHANDLE       hDclInst)
{
    static DCLSERVICEHEADER SrvRand[DCL_MAX_INSTANCES];
    DCLSERVICEHEADER       *pSrv;
    DCLSTATUS               dclStat;

    if(!hDclInst)
    {
        hDclInst = DclInstanceHandle(0);
        if(!hDclInst)
            return DCLSTAT_TIMER_INSTANCEHANDLEFAILED;
    }

    /*  All that is needed is the setup of the random number seed.
        Use the system tick count for the time being, the high
        resolution timer should be more 'random', but is not available
        in all systems.
    */
    ullGlobalRandomNumberSeed = DclOsTickCount();
    dclStat = DCLSTAT_SUCCESS;


    pSrv = &SrvRand[DclInstanceNumber(hDclInst)];
    dclStat = DclServiceHeaderInit(&pSrv, "DLRAND", DCLSERVICE_RAND, NULL, NULL, 0);
    if(dclStat == DCLSTAT_SUCCESS)
        dclStat = DclServiceCreate(hDclInst, pSrv);

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclRand64()

    Generate a 64-bit pseudo-random number.

    Parameters:
        pullSeed - A pointer to the seed to use.  Set this value to
                   NULL to use the internal global seed value.  This
                   may result in non-deterministic results in a 
                   multithreaded environment.

    Return Value:
        A pseudo-random number in the range [0, D_UINT64_MAX].
-------------------------------------------------------------------*/
D_UINT64 DclRand64(
    D_UINT64       *pullSeed)
{
    /*  The period of this random number generator is 2^64  (1.8 x 1019)
        These parameters are the same as the default one-stream SPRNG
        lcg64 generator and it satisfies the requirements for a maximal
        period.

        The tempering value is used an an AND mask and is specifically
        selected to favor the distribution of lower bits.
    */
    const D_UINT64  ullA = UINT64SUFFIX(2862933555777941757);
    const D_UINT64  ullC = UINT64SUFFIX(3037000493);
    const D_UINT64  ullT = UINT64SUFFIX(4921441182957829599);
    D_UINT64        ullN;
    D_UINT64       *pullSeedPtr;
    D_UINT64        ullLocalSeed;

    if(pullSeed)
    {
        ullLocalSeed = *pullSeed;
        pullSeedPtr = pullSeed;
    }
    else
    {
        ullLocalSeed = ullGlobalRandomNumberSeed;
        pullSeedPtr = &ullGlobalRandomNumberSeed;
    }

    ullN = *pullSeedPtr = ullLocalSeed * ullA + ullC;

    /*  The Linear congruential generator used above produces good
        psuedo-random 64-bit number sequences, however, as with any
        LCG, the period of the lower order bits is much shorter
        resulting in alternately odd/even pairs in bit zero.

        The result of the LGC above is tempered below with a series
        of XOR and shift operations to produce a more acceptable
        equidistribution of bits throughout the 64-bit range.
    */

    ullN ^= ullN >> 21 & ullT;
    ullN ^= ullN >> 43 & ullT;
    ullN ^= ullN << 23 & ~ullT;
    ullN ^= ullN << 31 & ~ullT;

    return ullN;
}


/*-------------------------------------------------------------------
    Public: DclRand()

	Generate a 32-bit pseudo-random number. 

    Parameters:
        pulSeed - A pointer to the seed to use.  Set this value to
                  NULL to use the internal global seed value.  This
                  may result in non-deterministic results in a 
                  multithreaded environment.

    Return Value:
        A pseudo-random number in the range [0, D_UINT32_MAX].
-------------------------------------------------------------------*/
D_UINT32 DclRand(
    D_UINT32   *pulSeed)
{
    D_UINT64   *pullSeedPtr;
    D_UINT64    ullLocalSeed;
    D_UINT64    ullN;

    if(pulSeed)
    {
        ullLocalSeed = *pulSeed;
        pullSeedPtr = &ullLocalSeed;
    }
    else
    {
        pullSeedPtr = NULL;
    }

    ullN = DclRand64(pullSeedPtr);

    if(pulSeed)
    {
        *pulSeed = (D_UINT32) *pullSeedPtr;
    }

    return (D_UINT32) ullN;
}



