//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  
    blcommon.c
    
Abstract:  
    Bootloader common main module. This file contains the C BootloaderMain
    function for the boot loader.    NOTE: The firmware "entry" point (the real
    entry point) is _StartUp in init assembler file.

    The Windows CE boot loader is the code that is executed on a Windows CE
    development system at power-on reset and loads the Windows CE
    operating system. The boot loader also provides code that monitors
    the behavior of a Windows CE platform between the time the boot loader
    starts running and the time the full operating system debugger is 
    available. Windows CE OEMs are supplied with sample boot loader code 
    that runs on a particular development platform and CPU.
    
Functions:


Notes: 

    PACKET: a signed blob of data.  Contains ONE of the following:
        array of bytes from a raw .nb0 image
        one .bin record
        one .bin record chunk

    RECORD: a .bin record created by romimage.  

    CHUNK:  a fragment of a .bin record.  Chunks are created if a record is
            larger than the PACKET size.

--*/
#include <windows.h>
#include <halether.h>
#include <blcommon.h>
#include "..\eboot.h"
#include <bsp_logo.h>

// Global buffer to contain the 7-byte image header.
// Unsigned .nb0 files don't have a header, so the data
// read into this buffer must be copied back into the 
// destination memory after probing for image type.
#define BL_HDRSIG_SIZE 7
#define MLDR_PATTERN  "\r\nD:\\>"
static BYTE g_hdr[BL_HDRSIG_SIZE];

// Only provide download manifest to the OAL once per session
static BOOL g_fOEMNotified = FALSE;


#define SPIN_FOREVER        while (1)

ROMHDR * volatile const pTOC = (ROMHDR *)-1;     // Gets replaced by RomLoader with real address
static DownloadManifest g_DownloadManifest;
static BOOLEAN g_bBINDownload = 1;
static BYTE g_downloadFilesRemaining = 1;


static BOOL KernelRelocate    (ROMHDR *const pTOC);
static BOOL DownloadImage     (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr);
static BOOL IsKernelRegion    (DWORD   dwRegionStart, DWORD   dwRegionLength);

static BOOL DownloadBin       (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr);
static BOOL DownloadNB0       (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr);

static BOOL GetImageType      ();
static BOOL CheckImageManifest();
static BOOL WriteImageToFlash ();
VOID OEMPlatformDeinit();

static VOID ComputeChecksum();
extern BOOL DownloadBmp();

#define CURRENT_VERSION_MAJOR       1
#define CURRENT_VERSION_MINOR       0

const unsigned char NKSignon[] = {
    "\nMicronet Munufacturing SD Card Loader Version %d.%d Built "__DATE__ " " __TIME__ "\r\n"
};

DWORD g_dwROMOffset;

PFN_OEMVERIFYMEMORY     g_pOEMVerifyMemory;
PFN_OEMREPORTERROR      g_pOEMReportError;
PFN_OEMCHECKSIGNATURE   g_pOEMCheckSignature;
PFN_OEMMULTIBINNOTIFY   g_pOEMMultiBINNotify;

UINT32	g_y = 30;

static void HALT (DWORD dwReason)
{
    if (g_pOEMReportError)
    {
        g_pOEMReportError (dwReason, 0);
    }
    SPIN_FOREVER;
}


void BootloaderMain (void)
{
    DWORD dwAction;   
    DWORD dwpToc = 0;
    DWORD dwImageStart = 0, dwImageLength = 0, dwLaunchAddr = 0;
    BOOL bDownloaded = FALSE;

	UINT32 x = 30;
    // relocate globals to RAM
    if (!KernelRelocate (pTOC))
    {
        // spin forever
        HALT (BLERR_KERNELRELOCATE);
    }

    // (1) Init debug support. We can use OEMWriteDebugString afterward.
    if (!OEMDebugInit ())
    {
        // spin forever
        HALT (BLERR_DBGINIT);
    }

    // output banner
    KITLOutputDebugString (NKSignon, CURRENT_VERSION_MAJOR, CURRENT_VERSION_MINOR);

	KITLOutputDebugString (MLDR_PATTERN "Connected...\r\n");

    // (3) initialize platform (clock, drivers, transports, etc)
    if (!OEMPlatformInit ())
    {
        // spin forever
        HALT (BLERR_PLATINIT);
    }

    // system ready, preparing for download

    // (4) call OEM specific pre-download function
	while(1)
	{
		switch (dwAction = OEMPreDownload ())
		{
		case BL_DOWNLOAD:
		{
			// (5) download image
			memset(&g_DownloadManifest, 0, sizeof(g_DownloadManifest));
			g_fOEMNotified = 0;
			bDownloaded = 0;
			g_downloadFilesRemaining = 1;
			if (!DownloadImage (&dwImageStart, &dwImageLength, &dwLaunchAddr))
			{
				TextWriteW( x + 300, g_y, L"ERROR", TX_LEFT, CL_TEXT );
				// error already reported in DownloadImage
				SPIN_FOREVER;
			}
			bDownloaded = TRUE;

			// Check for pTOC signature ("CECE") here, after image in place
			if (*(LPDWORD) OEMMapMemAddr (dwImageStart, dwImageStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE)
			{
				dwpToc = *(LPDWORD) OEMMapMemAddr (dwImageStart, dwImageStart + ROM_SIGNATURE_OFFSET + sizeof(ULONG));
				// need to map the content again since the pointer is going to be in a fixup address
				dwpToc = (DWORD) OEMMapMemAddr (dwImageStart, dwpToc + g_dwROMOffset);

				KITLOutputDebugString ("ROMHDR at Address %Xh\r\n", dwImageStart + ROM_SIGNATURE_OFFSET + sizeof (DWORD)); // right after signature
			}
//			OEMPlatformDeinit();
//			OEMPlatformInit();

			TextWriteW( x + 300, g_y, L"OK", TX_LEFT, CL_TEXT );
			break;
		}
			// fall through
		case BL_JUMP:
			// Before jumping to the image, optionally check the image signature.
			// NOTE: if we haven't downloaded the image by now, we assume that it'll be loaded from local storage in OEMLaunch (or it
			// already resides in RAM from an earlier download), and in this case, the image start address might be 0.  This means 
			// that the image signature routine will need to find the image in storage or in RAM to validate it.  Since the OEM"s 
			// OEMLaunch function will need to do this anyways, we trust that it's within their abilities to do it here.
			//
			if (g_bBINDownload && g_pOEMCheckSignature)
			{
				if (!g_pOEMCheckSignature(dwImageStart, g_dwROMOffset, dwLaunchAddr, bDownloaded))
					HALT(BLERR_CAT_SIGNATURE);
			}
			// (5) final call to launch the image. never returned
			OEMLaunch (dwImageStart, dwImageLength, dwLaunchAddr, (const ROMHDR *)dwpToc);
			// should never return
			// fall through
			// spin forever
			HALT (BLERR_INVALIDCMD);
			break;
		case BL_DOWNLOAD_BMP:
			if(!DownloadBmp() )
				TextWriteW( x + 300, g_y, L"ERROR", TX_LEFT, CL_TEXT );
			else
				TextWriteW( x + 300, g_y, L"OK", TX_LEFT, CL_TEXT );
		break;

		default:
			// ERROR! spin forever
			continue;
		}
	}
}


//
// KernelRelocate: move global variables to RAM
//
static BOOL KernelRelocate (ROMHDR *const pTOC)
{
    ULONG loop;
    COPYentry *cptr;
    if (pTOC == (ROMHDR *const) -1)
    {
        return (FALSE); // spin forever!
    }
    // This is where the data sections become valid... don't read globals until after this
    for (loop = 0; loop < pTOC->ulCopyEntries; loop++)
    {
        cptr = (COPYentry *)(pTOC->ulCopyOffset + loop*sizeof(COPYentry));
        if (cptr->ulCopyLen)
            memcpy((LPVOID)cptr->ulDest,(LPVOID)cptr->ulSource,cptr->ulCopyLen);
        if (cptr->ulCopyLen != cptr->ulDestLen)
            memset((LPVOID)(cptr->ulDest+cptr->ulCopyLen),0,cptr->ulDestLen-cptr->ulCopyLen);
    }
    return (TRUE);
}

static BOOL VerifyChecksum (DWORD cbRecord, LPBYTE pbRecord, DWORD dwChksum)
{
    // Check the CRC
    DWORD dwCRC = 0;
    DWORD i;
    for (i = 0; i < cbRecord; i++)
        dwCRC += *pbRecord ++;

    if (dwCRC != dwChksum)
        KITLOutputDebugString ("ERROR: Checksum failure (expected=0x%x  computed=0x%x)\r\n", dwChksum, dwCRC);

    return (dwCRC == dwChksum);
}


static BL_IMAGE_TYPE GetImageType()
{
    BL_IMAGE_TYPE rval = BL_IMAGE_TYPE_UNKNOWN;

    // read the 7 byte "magic number"
    //
    if (!OEMReadData (BL_HDRSIG_SIZE, g_hdr))
    {
        KITLOutputDebugString ("\r\nERROR: Unable to read image signature.\r\n");
        rval =  BL_IMAGE_TYPE_NOT_FOUND;
    }

    
    // The N000FF packet indicates a manifest, which is constructed by Platform 
    // Builder when we're downloading multiple .bin files or an .nb0 file.
    //
    if (!memcmp (g_hdr, "N000FF\x0A", BL_HDRSIG_SIZE))
    {
        KITLOutputDebugString("\r\nBL_IMAGE_TYPE_MANIFEST\r\n\r\n");
        rval =  BL_IMAGE_TYPE_MANIFEST;
    }
    else if (!memcmp (g_hdr, "X000FF\x0A", BL_HDRSIG_SIZE))
    {
        KITLOutputDebugString("\r\nBL_IMAGE_TYPE_MULTIXIP\r\n\r\n");
        rval =  BL_IMAGE_TYPE_MULTIXIP;
    }
    else if (!memcmp (g_hdr, "B000FF\x0A", BL_HDRSIG_SIZE))
    {
        KITLOutputDebugString("\r\nBL_IMAGE_TYPE_BIN\r\n\r\n");
        rval =  BL_IMAGE_TYPE_BIN;
    }
    else if (!memcmp (g_hdr, "S000FF\x0A", BL_HDRSIG_SIZE))
    {
        KITLOutputDebugString("\r\nBL_IMAGE_TYPE_SIGNED_BIN\r\n\r\n");
        rval =  BL_IMAGE_TYPE_SIGNED_BIN;
    }
    else if (!memcmp (g_hdr, "R000FF\x0A", BL_HDRSIG_SIZE))
    {
        KITLOutputDebugString("\r\nBL_IMAGE_TYPE_SIGNED_NB0\r\n\r\n");
        rval =  BL_IMAGE_TYPE_SIGNED_NB0;
    }
    else
    {
        KITLOutputDebugString("\r\nBL_IMAGE_TYPE_UNKNOWN\r\n\r\n");
        rval =  BL_IMAGE_TYPE_UNKNOWN;
    }

    return rval;  
}


static BOOL CheckImageManifest()
{
    DWORD dwRecChk;

    // read the packet checksum.
    //
    if (!OEMReadData (sizeof (DWORD), (LPBYTE) &dwRecChk))
    {
        KITLOutputDebugString("\r\nERROR: Unable to read download manifest checksum.\r\n");
        //HALT (BLERR_MAGIC);
        return FALSE;
    }

    // read region descriptions (start address and length).
    //
    if (!OEMReadData (sizeof (DWORD), (LPBYTE) &g_DownloadManifest.dwNumRegions) ||
        !OEMReadData ((g_DownloadManifest.dwNumRegions * sizeof(RegionInfo)), (LPBYTE) &g_DownloadManifest.Region[0]))
    {
        KITLOutputDebugString("\r\nERROR: Unable to read download manifest information.\r\n");
        //HALT (BLERR_MAGIC);
        return FALSE;
    }

    // verify the packet checksum.
    //
    if (!VerifyChecksum((g_DownloadManifest.dwNumRegions * sizeof(RegionInfo)), (LPBYTE) &g_DownloadManifest.Region[0], dwRecChk))
    {
        KITLOutputDebugString ("\r\nERROR: Download manifest packet failed checksum verification.\r\n");
        //HALT (BLERR_CHECKSUM);
        return FALSE;
    }

    return TRUE;
}


static BOOL DownloadImage(LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr)
{
    BOOL        rval = TRUE;
    DWORD       dwImageType;

    *pdwImageStart = *pdwImageLength = *pdwLaunchAddr = 0;

	KITLOutputDebugString(MLDR_PATTERN "Preparing for download.\r\n");

    //
    // Download each region (multiple can be sent)
    //
    do
    {
        dwImageType = GetImageType();

		KITLOutputDebugString("*****Image type\r\n");
        
        switch(dwImageType) 
        {
            case BL_IMAGE_TYPE_MANIFEST:
                // Platform Builder sends a manifest to indicate the following 
                // data consists of multiple .bin files /OR/ one .nb0 file.
                if (!CheckImageManifest()) {
                    //HALT (BLERR_MAGIC);
                    return (FALSE);   
                }

                // Continue with download of next file
                // +1 to account for the manifest
                g_downloadFilesRemaining = (BYTE)(g_DownloadManifest.dwNumRegions + 1);
                continue;
            case BL_IMAGE_TYPE_BIN:
				KITLOutputDebugString(MLDR_PATTERN "Download Image binary: started.\r\n");
                rval &= DownloadBin( pdwImageStart, pdwImageLength, pdwLaunchAddr );
				KITLOutputDebugString(MLDR_PATTERN "Download Image binary: completed. Status = %s.\r\n",
					                                rval == 0 ? "Failed" : "Succeeded");
                break;
            case BL_IMAGE_TYPE_SIGNED_BIN:
                KITLOutputDebugString("\r\n**\r\n");
                KITLOutputDebugString(MLDR_PATTERN "** ERROR: This boot loader does not support signed .bin images.\r\n");
                KITLOutputDebugString("**\r\n");
                //HALT (BLERR_SIGNATURE);
                return (FALSE);

            case BL_IMAGE_TYPE_SIGNED_NB0:
                KITLOutputDebugString("\r\n**\r\n");
                KITLOutputDebugString(MLDR_PATTERN "** ERROR: This boot loader does not support signed .nb0 images.\r\n");
                KITLOutputDebugString("**\r\n");
                //HALT (BLERR_SIGNATURE);
                return (FALSE);
            case BL_IMAGE_TYPE_MULTIXIP:
                KITLOutputDebugString("\r\n**\r\n");
                KITLOutputDebugString(MLDR_PATTERN "** ERROR: The X000FF packet is an old-style multi-bin download manifest and it's no longer supported.\r\n");
                KITLOutputDebugString("** Please update your Platform Builder installation in you want to download multiple files.\r\n");
                KITLOutputDebugString("**\r\n");
                //HALT (BLERR_MAGIC);
                return (FALSE);

            case BL_IMAGE_TYPE_UNKNOWN:
                // Assume files without a "type" header (e.g. raw data) are unsigned .nb0
                rval &= DownloadNB0( pdwImageStart, pdwImageLength, pdwLaunchAddr );
                break;
            default:
                // should never get here
				KITLOutputDebugString(MLDR_PATTERN "DownloadImage nothing.\r\n");
                return (FALSE);
                
        }
    }
    while (--g_downloadFilesRemaining);

    if(rval)
	{
		ComputeChecksum();
		KITLOutputDebugString(MLDR_PATTERN "Preparing for burn.\r\n");
		rval &= WriteImageToFlash();
	}
	//KITLOutputDebugString(MLDR_PATTERN "Completed to burn. Status = %d\r\n",rval);

    return rval;
}

static BOOL DownloadBin (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr)
{
    RegionInfo *pCurDownloadFile;
    BOOL        fIsFlash = FALSE;
    LPBYTE      lpDest = NULL;
    DWORD       dwImageStart, dwImageLength, dwRecAddr, dwRecLen, dwRecChk, dwImageDawnlodedPart;
    DWORD       dwRecNum = 0;
	DWORD       dwImagePercent,i;

    g_bBINDownload = TRUE;

    if (!OEMReadData(sizeof(DWORD), (LPBYTE) &dwImageStart) || !OEMReadData(sizeof(DWORD), (LPBYTE)&dwImageLength))
    {
        KITLOutputDebugString ("Unable to read image start/length\r\n");
        //HALT(BLERR_MAGIC);
        return (FALSE);
    }

	KITLOutputDebugString("****image start/length %x/%x\r\n", dwImageStart, dwImageLength);

    // If Platform Builder didn't provide a manifest (i.e., we're only 
    // downloading a single .bin file), manufacture a manifest so we
    // can notify the OEM.
    //
    if (!g_DownloadManifest.dwNumRegions)
    {
        g_DownloadManifest.dwNumRegions             = 1;
        g_DownloadManifest.Region[0].dwRegionStart  = dwImageStart;
        g_DownloadManifest.Region[0].dwRegionLength = dwImageLength;
    }

	KITLOutputDebugString("****image has %x regions\r\n", g_DownloadManifest.dwNumRegions);
    // Provide the download manifest to the OEM.
    //
    if (!g_fOEMNotified && g_pOEMMultiBINNotify)
    {
        g_pOEMMultiBINNotify((PDownloadManifest)&g_DownloadManifest);
        g_fOEMNotified = TRUE;
    }

    // Locate the current download manifest entry (current download file).
    //
    pCurDownloadFile = &g_DownloadManifest.Region[g_DownloadManifest.dwNumRegions - g_downloadFilesRemaining];

    // give the OEM a chance to verify memory
    if(g_pOEMVerifyMemory && !g_pOEMVerifyMemory (pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionLength))
    {
        KITLOutputDebugString ("!OEMVERIFYMEMORY: Invalid image\r\n");
        //HALT (BLERR_OEMVERIFY);
        return (FALSE);
    }

    // check for flash image. Start erasing if it is.
    if((fIsFlash = OEMIsFlashAddr(pCurDownloadFile->dwRegionStart)) &&
		!OEMStartEraseFlash (pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionLength))
    {
        KITLOutputDebugString ("Invalid flash address/length\r\n");
        //HALT (BLERR_FLASHADDR);
        return (FALSE);
    }

#ifdef DEBUG
    // Clearing memory ensures no garbage between sparse .bin records, so that 
    // our post-download checksum will be accurate.
    memset((LPVOID) OEMMapMemAddr(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart), 0, pCurDownloadFile->dwRegionLength);
#endif


    //------------------------------------------------------------------------
    //  Download .bin records
    //------------------------------------------------------------------------

	KITLOutputDebugString ("Start image %s download\r\n ",pCurDownloadFile->szFileName);
	dwImageDawnlodedPart = 0;
	dwImagePercent = 0; 
	i=0;
	//
    while(OEMReadData(sizeof(DWORD), (LPBYTE)&dwRecAddr) && OEMReadData(sizeof(DWORD), (LPBYTE)&dwRecLen) && OEMReadData(sizeof(DWORD), (LPBYTE)&dwRecChk))
    {
#ifdef DEBUG
        KITLOutputDebugString(" <> Record [ %d ] dwRecAddr = 0x%x, dwRecLen = 0x%x, dwImagePercent = 0x%x, dwImageDawnlodedPart = 0x%x\r\n", dwRecNum, dwRecAddr, dwRecLen,dwImagePercent,dwImageDawnlodedPart);
#endif

        // last record of .bin file uses sentinel values for address and checksum.
        if(!dwRecAddr && !dwRecChk)
        {
            break;
        }

        // map the record address (FLASH data is cached, for example)
        lpDest = OEMMapMemAddr(pCurDownloadFile->dwRegionStart, dwRecAddr);

        // read data block
        if(!OEMReadData (dwRecLen, lpDest))
        {
            KITLOutputDebugString ("****** Data record %d corrupted, ABORT!!! ******\r\n", dwRecNum);
            //HALT (BLERR_CORRUPTED_DATA);
            return (FALSE);
        }

        if(!VerifyChecksum (dwRecLen, lpDest, dwRecChk))
        {
            KITLOutputDebugString ("****** Checksum failure on record %d, ABORT!!! ******\r\n", dwRecNum);
            //HALT (BLERR_CHECKSUM);
            return (FALSE);
        }

        // Look for ROMHDR to compute ROM offset.  NOTE: romimage guarantees that the record containing
        // the TOC signature and pointer will always come before the record that contains the ROMHDR contents.
        //
        if(dwRecLen == sizeof(ROMHDR) && (*(LPDWORD)OEMMapMemAddr(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE))
        {
            DWORD dwTempOffset = (dwRecAddr - *(LPDWORD)OEMMapMemAddr(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart + ROM_SIGNATURE_OFFSET + sizeof(ULONG)));
            ROMHDR *pROMHdr = (ROMHDR *)lpDest;

            // Check to make sure this record really contains the ROMHDR.
            //
            if ((pROMHdr->physfirst == (pCurDownloadFile->dwRegionStart - dwTempOffset)) &&
                (pROMHdr->physlast  == (pCurDownloadFile->dwRegionStart - dwTempOffset + pCurDownloadFile->dwRegionLength)) &&
                (DWORD)(HIWORD(pROMHdr->dllfirst << 16) <= pROMHdr->dlllast) &&
                (DWORD)(LOWORD(pROMHdr->dllfirst << 16) <= pROMHdr->dlllast))
            {
                g_dwROMOffset = dwTempOffset;
                KITLOutputDebugString("rom_offset=0x%x.\r\n", g_dwROMOffset); 
            }
        }

        // verify partial checksum
        OEMShowProgress (dwRecNum++);
		// check what % of the image already downloaded
		dwImageDawnlodedPart+=dwRecLen;


		if (dwImageDawnlodedPart >= dwImagePercent)
		{
			KITLOutputDebugString(MLDR_PATTERN "Download in PROGRESS -- %d percent.\r\n", (i*20)); 
			i++; 
			dwImagePercent =(dwImageLength*(2*i))/10;
		}
		

        if(fIsFlash)
        {
            OEMContinueEraseFlash ();
        }
    }  // while( records remaining )
	KITLOutputDebugString ("Image download completed\r\n");
    

    //------------------------------------------------------------------------
    //  Determine the image entry point
    //------------------------------------------------------------------------
	//
    // Does this .bin file contain a TOC?
	//
    if(*(LPDWORD)OEMMapMemAddr(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE)
    {
        // Contain the kernel?
        if (IsKernelRegion(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionLength))
        {
            *pdwImageStart  = pCurDownloadFile->dwRegionStart;
            *pdwImageLength = pCurDownloadFile->dwRegionLength;
            *pdwLaunchAddr  = dwRecLen;
        }
    }
    // No TOC - not made by romimage.  
    else if(g_DownloadManifest.dwNumRegions == 1)
    {
        *pdwImageStart  = pCurDownloadFile->dwRegionStart;
        *pdwImageLength = pCurDownloadFile->dwRegionLength;
        *pdwLaunchAddr  = dwRecLen;
    }
    else
    {
        // If we're downloading more than one .bin file, it's probably 
        // chain.bin which doesn't have a TOC (and which isn't
        // going to be downloaded on its own) and we should ignore it.
    }


    if (fIsFlash)
    {
        // finish the flash erase
        if (!OEMFinishEraseFlash())
        {
            //HALT (BLERR_FLASH_ERASE);
            return (FALSE);
        }

    }

    KITLOutputDebugString("ImageStart = 0x%x, ImageLength = 0x%x, LaunchAddr = 0x%x\r\n", *pdwImageStart, *pdwImageLength, *pdwLaunchAddr);

    return TRUE;
}

static BOOL DownloadNB0 (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr)
{
    RegionInfo *pCurDownloadFile;
    BOOL        fIsFlash = FALSE;
    LPBYTE      lpDest = NULL;

    g_bBINDownload = FALSE;


    // Provide the download manifest to the OEM.  This gives the OEM the
    // opportunity to provide start addresses for the .nb0 files (which 
    // don't contain placement information like .bin files do).
    if (!g_fOEMNotified && g_pOEMMultiBINNotify)
    {
        g_pOEMMultiBINNotify((PDownloadManifest)&g_DownloadManifest);
        g_fOEMNotified = TRUE;
    }

    // Locate the current download manifest entry (current download file).
    //
    pCurDownloadFile = &g_DownloadManifest.Region[g_DownloadManifest.dwNumRegions - g_downloadFilesRemaining];

    // give the OEM a chance to verify memory
    if (g_pOEMVerifyMemory && !g_pOEMVerifyMemory (pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionLength))
    {
        KITLOutputDebugString ("!OEMVERIFYMEMORY: Invalid image\r\n");
        //HALT (BLERR_OEMVERIFY);
        return (FALSE);
    }

    // check for flash image. Start erasing if it is.
    if ((fIsFlash = OEMIsFlashAddr (pCurDownloadFile->dwRegionStart)) 
        && !OEMStartEraseFlash (pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionLength))
    {
        KITLOutputDebugString ("Invalid flash address/length\r\n");
        //HALT (BLERR_FLASHADDR);
        return (FALSE);
    }


    //------------------------------------------------------------------------
    //  Download the file
    //
    //  If we're downloading an UNSIGNED .nb0 file, we've already read the 
    //  start of the file in GetImageType().
    //  Copy what we've read so far to the destination, then finish downloading.
    //------------------------------------------------------------------------
    lpDest = OEMMapMemAddr (pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart);
    if(0 == lpDest)
		return 0;

	memcpy(lpDest, g_hdr, BL_HDRSIG_SIZE);
    lpDest += BL_HDRSIG_SIZE;

    if (!OEMReadData ((pCurDownloadFile->dwRegionLength - BL_HDRSIG_SIZE), lpDest))
    {
        KITLOutputDebugString ("ERROR: failed when reading raw binary file.\r\n");
        //HALT (BLERR_CORRUPTED_DATA);
        return (FALSE);
    }


    //------------------------------------------------------------------------
    //  Determine the image entry point
    //------------------------------------------------------------------------

    *pdwImageStart  = pCurDownloadFile->dwRegionStart;
    *pdwLaunchAddr  = pCurDownloadFile->dwRegionStart;
    *pdwImageLength = pCurDownloadFile->dwRegionLength;


    if (fIsFlash)
    {
        // finish the flash erase
        if (!OEMFinishEraseFlash())
        {
            //HALT (BLERR_FLASH_ERASE);
            return (FALSE);
        }
    }

    KITLOutputDebugString("ImageStart = 0x%x, ImageLength = 0x%x, LaunchAddr = 0x%x\r\n", *pdwImageStart, *pdwImageLength, *pdwLaunchAddr);

    return TRUE;
}

static BOOL WriteImageToFlash()
{
    BOOL  bFlash;
    DWORD i;
    
    KITLOutputDebugString("\r\nCompleted file(s):\r\n");
    KITLOutputDebugString("-------------------------------------------------------------------------------\r\n");

    for (i = 0; i < g_DownloadManifest.dwNumRegions; i++)
    {
        RegionInfo *pRegion = &g_DownloadManifest.Region[i];

        bFlash = OEMIsFlashAddr( pRegion->dwRegionStart );
        
        KITLOutputDebugString("[%d]: Address=0x%x  Length=0x%x  Name=\"%s\" Target=%s\r\n",
            i, 
            pRegion->dwRegionStart, 
            pRegion->dwRegionLength, 
            pRegion->szFileName,
            (bFlash ? "FLASH" : "RAM"));

        if (bFlash) 
        {
            if (!OEMWriteFlash (pRegion->dwRegionStart, pRegion->dwRegionLength))
            {
                //HALT (BLERR_FLASH_WRITE);
                return FALSE;
            }
        }
    }

    return TRUE;    
}

static VOID ComputeChecksum()
{
#ifdef DEBUG
    RegionInfo *pRegion;
    DWORD       dwRegionLength;
    DWORD       dwChecksum;
    BYTE       *pbCache;
    DWORD       i;
    
    for (i = 0; i < g_DownloadManifest.dwNumRegions; i++)
    {
        pRegion         = &g_DownloadManifest.Region[i];
        pbCache         = (LPBYTE) OEMMapMemAddr( pRegion->dwRegionStart, pRegion->dwRegionStart );
        dwRegionLength  = pRegion->dwRegionLength;
        dwChecksum      = 0;

        KITLOutputDebugString("Computing checksum: image start = 0x%x, len = 0x%x\r\n", 
            pbCache, dwRegionLength);

        while(dwRegionLength--) {
            dwChecksum += *pbCache++;
        }

        KITLOutputDebugString("Checksum = 0x%x (0x%x bytes)\r\n", dwChecksum, pRegion->dwRegionLength);
    }
#endif
}


/*
    @func   BOOLEAN | IsKernelRegion | Determines if the expanded BIN file provided contains the kernel image.
    @rdesc  TRUE if the region contains the kernel image, FALSE if it doesn't.
    @comm   <l Download Image> 
    @xref   
    @notes  dwCurrentBase is the base address where the BIN records are currently stored (this can be a RAM, a RAM
            file cache, or flash).  dwImageStoreBase is the images base storage address (this is the base address of potentially
            multiple BIN regions and can be in RAM or flash) and is used to translate addresses to the file cache area.
            dwROMOffset is the difference between the address where the BIN records are stored versus where they're fixed-up
            to run from (for example, an image may be stored in flash, but fixed-up to run in RAM).
*/
static BOOL IsKernelRegion(DWORD dwRegionStart, DWORD dwRegionLength)
{
	DWORD dwCacheAddress = 0;
	ROMHDR *pROMHeader;
	DWORD dwNumModules = 0;
	TOCentry *plTOC;

    if (dwRegionStart == 0 || dwRegionLength == 0)
        return(FALSE);

    if (*(LPDWORD) OEMMapMemAddr (dwRegionStart, dwRegionStart + ROM_SIGNATURE_OFFSET) != ROM_SIGNATURE)
        return (FALSE);

    // A pointer to the ROMHDR structure lives just past the ROM_SIGNATURE (which is a longword value).  Note that
    // this pointer is remapped since it might be a flash address (image destined for flash), but is actually cached
    // in RAM.
    //
    dwCacheAddress = *(LPDWORD) OEMMapMemAddr (dwRegionStart, dwRegionStart + ROM_SIGNATURE_OFFSET + sizeof(ULONG));
    pROMHeader     = (ROMHDR *) OEMMapMemAddr (dwRegionStart, dwCacheAddress + g_dwROMOffset);

    // Make sure sure are some modules in the table of contents.
    //
    if ((dwNumModules = pROMHeader->nummods) == 0)
        return (FALSE);

	// Locate the table of contents and search for the kernel executable and the TOC immediately follows the ROMHDR.
	//
    plTOC = (TOCentry *)(pROMHeader + 1);

	while(dwNumModules--) {
		LPBYTE pFileName = OEMMapMemAddr(dwRegionStart, (DWORD)plTOC->lpszFileName + g_dwROMOffset);
		if (!strcmp(pFileName, "nk.exe")) {
			return TRUE;
		}
		++plTOC;
	}
	return FALSE;
}
