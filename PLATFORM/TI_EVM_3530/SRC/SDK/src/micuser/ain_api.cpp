#include <windows.h>
#include <MicUserSdk.h>
#include <bsp.h>

#include <ceddk.h>
#include <ceddkex.h>

#include <madc.h>
#include <initguid.h>
#include <gpio.h>

#define  ADC_DELTA     8  // 3 bits for 10-bit ADC

//------------------------------------------------------------------------------
//
//  Type:  DEVICE_CONTEXT_AIN
//
//  This structure is used to store AIN device context
//

typedef struct 
{
    HANDLE  hDevice;
	DWORD   devContextSize;
} MIC_DEVICE_CONTEXT_AIN;


//------------------------------------------------------------------------------

HANDLE  MIC_AINOpen(VOID)
{
   HANDLE hDevice;
   MIC_DEVICE_CONTEXT_AIN *pContext = NULL; 
   
    hDevice = CreateFile(MADC_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) 
	 return(INVALID_HANDLE_VALUE);

    // Allocate memory for our handler...
    pContext = (MIC_DEVICE_CONTEXT_AIN *)LocalAlloc(LPTR, sizeof(MIC_DEVICE_CONTEXT_AIN));
    if (pContext == NULL)
     {
      CloseHandle(hDevice);
      return(INVALID_HANDLE_VALUE);
     }
		
    // Save device handle
    pContext->hDevice = hDevice;
	pContext->devContextSize = sizeof(MIC_DEVICE_CONTEXT_AIN);
		
  return(pContext);

}


BOOL  MIC_AINClose(HANDLE hContext)
{
  BOOL   retClose;
  HLOCAL retFree;
  MIC_DEVICE_CONTEXT_AIN *pContext;
  
  if( ( (DWORD)hContext % (sizeof(HANDLE)) ) != 0 )
  {
	RETAILMSG (1,(TEXT("MIC_AINClose: Invalid 1st Parameter as alignment!\r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }

  pContext = (MIC_DEVICE_CONTEXT_AIN *)hContext; 

  // Check Hanlde
  if( IsBadReadPtr(pContext, sizeof(MIC_DEVICE_CONTEXT_AIN *) ) )
  {
	RETAILMSG (1,(TEXT("MIC_AINClose: Invalid 1st Parameter as pointer!\r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }

  if( pContext->devContextSize != sizeof(MIC_DEVICE_CONTEXT_AIN) )
  {
	RETAILMSG (1,(TEXT("MIC_AINClose: Invalid Parameter \r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }

    retClose = CloseHandle(pContext->hDevice);
	if( retClose == FALSE )
	{
     LocalFree(pContext);
	 return(FALSE);
	}
	 
    retFree = LocalFree(pContext);
	if( retFree != NULL )
	 return(FALSE);
	else
     return(TRUE);	
}


#define ADCIN0_MUTEX L"ADCIN0_Mutex"

BOOL SwitchToAnalogInput()
{
	HANDLE hGpio = GPIOOpen();
	if (!hGpio)
	{
		RETAILMSG(1, (L"AIN API : %S, GPIOOpen failure\r\n", __FUNCTION__));
		return FALSE;
	}

	GPIOClrBit(hGpio, T2_GPIO_2);
	GPIOClose(hGpio);
	return TRUE;

	/*
	HANDLE hDevice = CreateFile(GPIO_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) 
		return FALSE;

	DWORD id = T2_GPIO_2;
	BOOL ret = DeviceIoControl(hDevice, IOCTL_GPIO_CLRBIT, &id, sizeof(id), NULL, 0, NULL, NULL);
	CloseHandle(hDevice);
	return ret;
	*/
}

BOOL   MIC_AINRawRead(HANDLE hDevice, UINT16 rawdata[])
{
  MIC_DEVICE_CONTEXT_AIN   *pContext;
  DWORD                     inBuf, outBuf, outSize;
  HANDLE hMutex = NULL;

  if( ((DWORD)hDevice % (sizeof(HANDLE)) ) != 0 )
  {
	RETAILMSG (1,(TEXT("MIC_AINRawRead: Invalid 1st Parameter as alignment!\r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }

  pContext = (MIC_DEVICE_CONTEXT_AIN *)hDevice; 

  // Check Hanlde

  if( IsBadReadPtr(pContext, sizeof(MIC_DEVICE_CONTEXT_AIN *) ) )
  {
	RETAILMSG (1,(TEXT("MIC_AINRawRead: Invalid 1st Parameter as pointer!\r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }
   
  if( pContext->devContextSize != sizeof(MIC_DEVICE_CONTEXT_AIN) )
  {
	RETAILMSG (1,(TEXT("MIC_AINRawRead: Invalid 1st Parameter \r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }

  // Check 2nd parameter
  if( ( rawdata == NULL )|| ( rawdata[0] != 2*sizeof(UINT16) ) )
  {
	RETAILMSG (1,(TEXT("MIC_AINRawRead: Invalid 2nd Parameter \r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }

  if (IsCe300())
  {
	  inBuf = MADC_CHANNEL_0;
	  
	  hMutex = CreateMutex(NULL, FALSE, ADCIN0_MUTEX);	
	  if (!hMutex)
		  return (FALSE);

	  if (WaitForSingleObject(hMutex, INFINITE) != WAIT_OBJECT_0)
	  {
		  RETAILMSG (1,(TEXT("MIC_AINRawRead: Can't access the hardware\r\n")));
		  ReleaseMutex(hMutex); 
		  CloseHandle(hMutex); 
		  SetLastError(ERROR_ACCESS_DENIED);
		  return (FALSE);
	  }

	  if (!SwitchToAnalogInput())
	  {
		  RETAILMSG (1,(TEXT("MIC_AINRawRead: Can't switch to the analog input circuit\r\n")));
		   ReleaseMutex(hMutex); 
		   CloseHandle(hMutex);
		   SetLastError(ERROR_READ_FAULT);
		   return (FALSE);
	  }
  }
  else
	  inBuf = MADC_CHANNEL_3;
  
  // For test purposes only !
  //inBuf = MADC_CHANNEL_2;
   
   if ( !DeviceIoControl( pContext->hDevice, IOCTL_MADC_READVALUE, (VOID*)&inBuf,
                         sizeof(inBuf), (VOID*)&outBuf, sizeof(outBuf),
                         &outSize, NULL)
	  )
      {
		if (hMutex) 
		{ 
			ReleaseMutex(hMutex); 
			CloseHandle(hMutex); 
		}
       CloseHandle(pContext->hDevice);
       LocalFree(pContext);
       pContext = NULL;
       return(FALSE);
      }
		
   // If Ok
	if (hMutex) 
	{ 
		ReleaseMutex(hMutex); 
		CloseHandle(hMutex); 
	}
   rawdata[1] = (UINT16)outBuf;
   return(TRUE);
}



BOOL   MIC_AINmVRead(HANDLE hDevice, UINT16 mV[])
{
  MIC_DEVICE_CONTEXT_AIN         *pContext;
  DWORD                          outBuf, outSize;
  IOCTL_MADC_CONVERTTOVOLTS_IN   inBuf;
  UINT16                         rawdata[2];
  DWORD                          rawdata2;
  BOOL                           ret; 
   
  if( ((DWORD)hDevice % (sizeof(HANDLE)) ) != 0 )
  {
	RETAILMSG (1,(TEXT("MIC_AINmVRead: Invalid 1st Parameter as alignment!\r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }

  pContext = (MIC_DEVICE_CONTEXT_AIN *)hDevice; 
  // Check Hanlde
  if( IsBadReadPtr(pContext, sizeof(MIC_DEVICE_CONTEXT_AIN *) ) )
  {
	RETAILMSG (1,(TEXT("MIC_AINmVRead: Invalid 1st Parameter as pointer!\r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }

  if( pContext->devContextSize != sizeof(MIC_DEVICE_CONTEXT_AIN) )
  {
	RETAILMSG (1,(TEXT("MIC_AINmVRead: Invalid 1st Parameter \r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }

  // Check 2nd parameter
  if( ( mV == NULL )|| ( mV[0] != 2*sizeof(UINT16) ) )
  {
	RETAILMSG (1,(TEXT("MIC_AINmVRead: Invalid 2nd Parameter \r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }
 
  // Read ADC
  rawdata[0] = sizeof(rawdata);
  ret = MIC_AINRawRead(hDevice, rawdata);
  if( ret == FALSE )
    return(FALSE);

  if (IsCe300())
	  inBuf.mask = MADC_CHANNEL_0;
  else
	  inBuf.mask = MADC_CHANNEL_3;
  
  inBuf.count = 1;
  rawdata2 = (DWORD)rawdata[1];
  inBuf.pdwValues = (DWORD *)&rawdata2;

   
   if ( !DeviceIoControl( pContext->hDevice, IOCTL_MADC_CONVERTTOVOLTS, (VOID*)&inBuf,
                         sizeof(inBuf), (VOID*)&outBuf, sizeof(outBuf),
                         &outSize, NULL)
	  )
      {
       CloseHandle(pContext->hDevice);
       LocalFree(pContext);
       pContext = NULL;
       return(FALSE);
      }
	  
   // If Ok
   mV[1] = (UINT16)outBuf;
   return(TRUE);
}


BOOL   MIC_AINAvmVRead(HANDLE hDevice, UINT16 mV[], UINT8 nSamples)
{
  MIC_DEVICE_CONTEXT_AIN         *pContext;
  DWORD                          outBuf, outSize;
  IOCTL_MADC_CONVERTTOVOLTS_IN   inBuf;
  UINT16                         rawdata[2];
//  DWORD                          rawdata2;
  //BOOL                           ret; 
  INT32                          average, averageTry, sRead, i, accSamples;
  INT32                          average2, averageTry2, accSamples2;
  //for testing purposes
  INT32                          delta; 
  static TCHAR                   sBuff[132] = {0};
  HANDLE hMutex = NULL;
   
  if( ((DWORD)hDevice % (sizeof(HANDLE)) ) != 0 )
  {
	RETAILMSG (1,(TEXT("MIC_AINmVRead: Invalid 1st Parameter as alignment!\r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }

  pContext = (MIC_DEVICE_CONTEXT_AIN *)hDevice; 
  // Check Hanlde
  if( IsBadReadPtr(pContext, sizeof(MIC_DEVICE_CONTEXT_AIN *) ) )
  {
	RETAILMSG (1,(TEXT("MIC_AINmVRead: Invalid 1st Parameter as pointer!\r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }

  if( pContext->devContextSize != sizeof(MIC_DEVICE_CONTEXT_AIN) )
  {
	RETAILMSG (1,(TEXT("MIC_AINmVRead: Invalid 1st Parameter \r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }

  // Check 2nd parameter
  if( ( mV == NULL )|| ( mV[0] != 2*sizeof(UINT16) ) )
  {
	RETAILMSG (1,(TEXT("MIC_AINmVRead: Invalid 2nd Parameter \r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
  }
 
  // Read ADC
#if 0
  rawdata[0] = sizeof(rawdata);
  ret = MIC_AINRawRead(hDevice, rawdata);
  if( ret == FALSE )
    return(FALSE);
#endif
///-------------------------------------------------------

  rawdata[0] = sizeof(rawdata);
  BOOL isCe300 = IsCe300();
  if (isCe300)
  {
	  inBuf.mask = MADC_CHANNEL_0;
	  hMutex = CreateMutex(NULL, FALSE, ADCIN0_MUTEX);
	  if (!hMutex)
		  return (FALSE);
  }
  else
	  inBuf.mask = MADC_CHANNEL_3;

   average = 0;
   accSamples = 0;
   average2 = 0;
   accSamples2 = 0;

   delta = 0;

   for( i=0; i <= nSamples; i++ )
   {
     // read ADC
	if (isCe300)
	{
		if (WaitForSingleObject(hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			RETAILMSG (1,(TEXT("MIC_AINRawRead: Can't access the hardware\r\n")));
			ReleaseMutex(hMutex); 
			CloseHandle(hMutex); 
			SetLastError(ERROR_ACCESS_DENIED);
			return (FALSE);
		}

		if (!SwitchToAnalogInput())
		{
			RETAILMSG (1,(TEXT("MIC_AINRawRead: Can't switch to the analog input circuit\r\n")));
			ReleaseMutex(hMutex); 
			CloseHandle(hMutex);
			SetLastError(ERROR_READ_FAULT);
			return (FALSE);
		}
	}
     if( !DeviceIoControl( pContext->hDevice, IOCTL_MADC_READVALUE, (VOID*)&inBuf.mask,
                         sizeof(inBuf.mask), (VOID*)&outBuf, sizeof(outBuf),
                         &outSize, NULL)
	   )
       {
		if (hMutex) 
		{ 
			ReleaseMutex(hMutex); 
			CloseHandle(hMutex); 
		}
        CloseHandle(pContext->hDevice);
        LocalFree(pContext);
        pContext = NULL;
        return(FALSE);
       }

	if (hMutex) 
	{ 
		ReleaseMutex(hMutex); 
	}
    sRead = outBuf;

    if( i == 0 ) 
      average = sRead; 
	else
	{  
      if( i < nSamples )
	  {
        averageTry = (average + sRead)/2;
	    if( ((sRead - averageTry)>ADC_DELTA)||((averageTry - sRead)>ADC_DELTA) )
		{
          if( delta < abs(sRead - averageTry) )
            delta = abs(sRead - averageTry);
		}
	    else
	    {
          average = averageTry;
          accSamples++;
	    }
	  }
	}

	if( i == 1 )
      average2 = sRead; 
	else
	{
      if( i > 1 )
	  {
        averageTry2 = (average2 + sRead)/2;
	    if( ((sRead - averageTry2)>ADC_DELTA)||((averageTry2 - sRead)>ADC_DELTA) )
		{
          if( delta < abs(sRead - averageTry) )
            delta = abs(sRead - averageTry);
		}
	    else
	    {
          average2 = averageTry2;
          accSamples2++;
	    }
	  }
	}

   }
	if (hMutex) 
	{ 
		CloseHandle(hMutex);
		hMutex = 0;
	}
 
   // Make decision
   if( accSamples2 > accSamples )
     average = average2;

   // For testing
   if( delta > 0 )
   {
    i = 13;
	wsprintf( sBuff, L"MIC_AINAvmVRead: delta = %d, average = %d", delta, average );
	RETAILMSG (1,(TEXT("%s\r\n"), sBuff));
   }
///-------------------------------------------------------

  // Converting to mV
  if (isCe300)
	  inBuf.mask = MADC_CHANNEL_0;
  else
	  inBuf.mask = MADC_CHANNEL_3;

  inBuf.count = 1;
//  rawdata2 = (DWORD)rawdata[1];
  inBuf.pdwValues = (DWORD *)&average;

   
   if ( !DeviceIoControl( pContext->hDevice, IOCTL_MADC_CONVERTTOVOLTS, (VOID*)&inBuf,
                         sizeof(inBuf), (VOID*)&outBuf, sizeof(outBuf),
                         &outSize, NULL)
	  )
      {
       CloseHandle(pContext->hDevice);
       LocalFree(pContext);
       pContext = NULL;
       return(FALSE);
      }
	  
   // If Ok
   mV[1] = (UINT16)outBuf;
   return(TRUE);
}

