/*---------------------------------------------------------------------------
   This file contains the typical modifications needed to modify the
   default ethernet boot loader in Windows CE to load an NK.BIN file
   off a FlashFX disk.
---------------------------------------------------------------------------*/

/* FlashFX Includes and defines
*/
#include <fxloader.h>

BOOL FFXIsBootable(void);
BOOL FFXReadBinFile(void);

/* NK.BIN Header
*/
const unsigned char   NKHeaderBytes[] = 
{'B','0','0','0','F','F','\n'};

/* The NK.BIN header is 7 characters in Length
*/
#define SIZE_OF_SYNC_BYTES  7


/* This indicates whether or not we've found, opened, and have
   an NK.BIN image ready to read from a FlashFX disk.
*/
static BOOL bFFXDownload = FALSE;
static FXLOADERHANDLE	hLoader = NULL;
static FXLOADERFILEHANDLE		hFile	= NULL;


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
BOOL OEMPlatformInit (void)
{
   /* If we've been told to, attempt to read NK.BIN from the FlashFX
      disk, call FFXIsBootable() to locate and open NK.BIN on the
      disk.  If we fail to find the OS image, or we're not supposed
      to boot from the disk, continue with the normal EBOOT process.

      If we find NK.BIN, call FFXReadBinFile to read the image from
      the disk and jump to it.  If we are unable to read a valid
      image from the disk, continue with the normal EBOOT process.
   */
   if(FFXIsBootable())
      if(FFXReadBinFile())
         return TRUE;

   /* Standard ethernet download process
   */

   return(TRUE);
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
BOOL OEMReadData(
	DWORD		cbData,
	LPBYTE	pbData)
{
   if(bFFXDownload)
   {
		if(FfxLoaderFileRead(hLoader, hFile, cbData, pbData) != cbData)
			return FALSE;
   }
   else
   {
      if(!EbootEtherReadData(cbData, pbData))
			return FALSE;
   }

   g_dwOffset += cbData;

   if (g_dwLength)
   {
	   int cPercent;
		
      cPercent = g_dwOffset * 100 / g_dwLength;
      DrawProgress (cPercent, FALSE);
   }

   return TRUE;
}


/*-------------------------------------------------------------------
	Parameters:
      nothing

	Notes:
		Determines if NK.BIN exists on the FlashFX Disk, and if so
      opens the file for reading.

	Returns:
		TRUE - If NK.BIN is on the media, and is opened for reading.
		FALSE - If NK.BIN is not found on the media.
-------------------------------------------------------------------*/
BOOL FFXIsBootable(void)
{
   EdbgOutputDebugString("FFX: Loader Initializing\n");

	/* Initialize the FlashFX loader for device 0, partition 0
	*/
	hLoader = FfxLoaderInitialize(0, 0, 0);
	if(!hLoader)
	{
      EdbgOutputDebugString("FFX: Loader initialize failed\n");
		return FALSE;
	}
	else
	{
      EdbgOutputDebugString("FFX: Opening NK.BIN.\n");
		hFile = FfxLoaderFileOpen(hLoader, "NK.BIN");
		if(!hFile)
		{
         EdbgOutputDebugString("FFX: Cannot open file NK.BIN\n");
			FfxLoaderDeinitialize(hLoader);
			hLoader = NULL;
			return FALSE;
		}

      EdbgOutputDebugString("FFX: NK.BIN is open for reading.\n");
		bFFXDownload = TRUE;
		return TRUE;
	}
}


/*-------------------------------------------------------------------
	Parameters:
      nothing

	Notes:
		Attempts to read the NK.BIN file into memory, validates it, and
      if all is well, jumps to it.  The only way this function should
      return is if something fails, and we want to continue on with
      the "normal" boot method.

	Returns:
      FALSE - If we have a corrupted NK.BIN on the media (fails
              checksums) then we want to be able to boot using the
              "normal" method.
-------------------------------------------------------------------*/
BOOL FFXReadBinFile(void)
{
	unsigned int	buf[3];
	unsigned char	cBuf[SIZE_OF_SYNC_BYTES];
	unsigned int	cbRecord, ulImageLength;
	unsigned int	ulCheckSum, ulCheckOff;
	unsigned int	Vaddr, ulChkSum, ulImageStart;
	unsigned char	*pCheckSum;
	unsigned int   *pStartAddr = 0;
	int				nResult;
	int				i;

   EdbgOutputDebugString("FFX: Reading image from FlashFX Disk\n");
	
   /* Start with the header
   */
	nResult = OEMReadData(SIZE_OF_SYNC_BYTES, cBuf);
	if (!nResult) 
	{
      EdbgOutputDebugString("FFX: Failed Header Read\n");
      goto ErrorCleanup;
   }

   /* Check to make sure we have a valid header!
	   first six bytes should be "B000FF"  
	*/
	for (i = 0; i < SIZE_OF_SYNC_BYTES; i++) 
	{
		if (cBuf[i] != NKHeaderBytes[i]) 
		{
          EdbgOutputDebugString("FFX: Invalid NK.BIN Header!\n");
          goto ErrorCleanup;
		}
	}

   /* Next, get the start address, and length for the image
   */
	nResult = OEMReadData(2 * sizeof(unsigned int), (unsigned char *)buf);
	if (!nResult) 
	{
      EdbgOutputDebugString("FFX: Failed Reading Image parameters\n");
      goto ErrorCleanup;
	}

#if D_DEBUG
   EdbgOutputDebugString("FFX: Starting Address:0x%X, Length of Image:%u\r\n", buf[0], buf[1]);
   EdbgOutputDebugString ("Contents of buf (0-1): %x %x\r\n", buf[0], buf[1]);
#endif

	ulImageStart  = buf[0]; /* starting address */
	ulImageLength = buf[1]; /* number of bytes */

	/* copy all records */
	do 
	{ 
	   /* Get the next record from the image! Start, Length, and Checksum */
		nResult = OEMReadData(3 * sizeof(unsigned int), (unsigned char *)buf);
		if (!nResult) 
		{
         EdbgOutputDebugString("FFX: Failed Reading Image parameters\n");
         goto ErrorCleanup;
		}

		Vaddr = buf[0]; /* starting address */
		cbRecord = buf[1]; /* number of bytes */
		ulChkSum = buf[2]; /* checksum */

      #if D_DEBUG
         EdbgOutputDebugString("FFX: Starting Address:0x%X, Length:%u, Checksum:0x%X\r\n", buf[0], buf[1], buf[2]);
      #endif

      if (Vaddr && cbRecord)
		{ /* copy directly to destination */
			nResult = OEMReadData((unsigned int)cbRecord, (unsigned char *)Vaddr);
			if (!nResult) 
			{
            EdbgOutputDebugString("FFX: Failed OEMReadData #3, Result was %d\n",nResult);
            return FALSE;
         }

         #if D_DEBUG
            EdbgOutputDebugString("FFX: OEMReadData succeeded\n");
         #endif

			ulCheckSum=0; /* calculate checksum */

			pCheckSum = (unsigned char *)Vaddr; 

			for (ulCheckOff=0; ulCheckOff < cbRecord; ulCheckOff++) 
			{
				ulCheckSum+= *pCheckSum++;
			}
			if (ulCheckSum != ulChkSum) 
			{
            EdbgOutputDebugString("FFX: Failed CheckSum\n");
            EdbgOutputDebugString("FFX: Checksum S/B: 0x%X, Is: 0x%X\r\n", ulChkSum, ulCheckSum);
            goto ErrorCleanup;
			}

         #if D_DEBUG
            EdbgOutputDebugString("FFX: CheckSum is correct.\n");
         #endif
		}
	} while (Vaddr); /* until last record */

   #ifndef D_DEBUG
      EdbgOutputDebugString(".\r\n");
   #endif

	FfxLoaderDeinitialize(hLoader);
	hLoader = NULL;
   EdbgOutputDebugString("FFX: Loader deinitialized\n");

   EdbgOutputDebugString("FFX: Launching Windows CE...\n");
   ((PFN_LAUNCH)(ulImageStart))();

   /* Adding the return to placate the compiler.  If we do the above
		launch, we're never going to get here... 
   */
   return TRUE;

ErrorCleanup:
   bFFXDownload = FALSE;
	
	FfxLoaderDeinitialize(hLoader);
	hLoader = NULL;
	
   EdbgOutputDebugString("FFX: Loader deinitialized\n");
	
   return FALSE;
}
