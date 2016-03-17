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

    This module contains the default OS Services functions for managing the
    standard time tick.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ostick.c $
    Revision 1.3  2010/07/27 20:37:42Z  garyp
    Removed some unused code.
    Revision 1.2  2010/07/06 01:32:52Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.1  2009/11/06 17:35:50Z  garyp
    Initial revision
---------------------------------------------------------------------------*/
#include <windows.h>
#include <oal_memory.h>
#include <oal_timer.h>

#include <dcl.h>
#include <dlcpuapi.h>
#include <hwomap35x.h>

#define TIME_CALL_DUMPSIZE         (0)  /* non-zero to debug tick history */

#if TIME_CALL_DUMPSIZE
    static unsigned nTimeCallDumpIndex = 0;
    static D_UINT32 ulRegCallDump[TIME_CALL_DUMPSIZE];
#endif



/*-------------------------------------------------------------------
    Public: DclOsTickCount()

    Get the current system tick count.  The tick resolution can be
    determined by calling DclOsTickResolution().  It must be assumed
    that the tick count will roll over at some point.

    Parameters:
        None.

    Return Value:
        Returns the current system tick value.
-------------------------------------------------------------------*/
D_UINT32 DclOsTickCount(void)
{
    D_UINT32                    ulTicks;
/*
    static unsigned             fVirgin = TRUE;
    static volatile D_UINT32   *pulOSCR0 = NULL;
*/

    ulTicks = OALGetTickCount();

#if 0
    if(fVirgin)
    {
        fVirgin = FALSE;

/*      if(DclCpuIsMmuOn())
	        pulOSCR0 = (volatile UINT32*)OALPAtoVA(OMAP35X_TIMER_GP1, FALSE);
        else */
            pulOSCR0 = (volatile UINT32*)(OMAP35X_TIMER_GP1);
    }

    ulTicks = *pulOSCR0;
#endif

  #if TIME_CALL_DUMPSIZE
    {
    	ulRegCallDump[nTimeCallDumpIndex] = ulTicks;
    	nTimeCallDumpIndex++;
    	if(nTimeCallDumpIndex >= TIME_CALL_DUMPSIZE)
            nTimeCallDumpIndex = 0;
    }
  #endif

    return ulTicks;
}


/*-------------------------------------------------------------------
    Public: DclOsTickModulus()

    Get the system tick modulus.  This is the number of legitimate
    tick values.  If the tick rolls over, then the tick modulus must
    be zero.

    Parameters:
        None.

    Return Value:
        Returns the system tick modulus.
-------------------------------------------------------------------*/
D_UINT32 DclOsTickModulus(void)
{
    /*  Ticker wraps so return a modulus of 0
    */
    return 0L;
}


/*-------------------------------------------------------------------
    Public: DclOsTickResolution()

    Get the system tick resolution in microseconds per tick.

    Parameters:
        None.

    Return Value:
        Returns the system tick resolution.
-------------------------------------------------------------------*/
D_UINT32 DclOsTickResolution(void)
{
    /*  Ticks on Windows CE are in milliseconds (though the
        ticker may not run at that frequency).
    */
    return 1000L;
}


#if TIME_CALL_DUMPSIZE

/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
void DumpTimeCalls()
{
	unsigned ii = 0;

	for(ii = 0; ii < TIME_CALL_DUMPSIZE; ii++)
	{
		if(nTimeCallDumpIndex == 0)
			nTimeCallDumpIndex = TIME_CALL_DUMPSIZE - 1;
		else
			nTimeCallDumpIndex--;

		DclPrintf("DclOsTickCount() DumpTimeCalls() %u %lX\n",
            nTimeCallDumpIndex, ulRegCallDump[nTimeCallDumpIndex]);
	}
}

#endif


