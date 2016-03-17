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

#ifdef SECURE_BOOTLOADER
#include <sbl.h>

// One or more public keys for validation of images.
// Provided by the platform's boot loader at link time.
extern const BYTE  *g_rgpbPublicKeys[];
extern const DWORD  g_rgdwKeyLengths[];
extern const WORD   g_wNumPublicKeys;

// Buffer to hold packet signature (output of hash algorithm).
// SHA-1 produces a 160-bit hash (20 bytes) but some algorithms use 128 bytes.
#define MAX_HASH_LENGTH 128
static BYTE          g_rgpbSignature[MAX_HASH_LENGTH];
static PUBLICKEYDATA g_keyData;
#endif // SECURE_BOOTLOADER


// Global buffer to contain the 7-byte image header.
// Unsigned .nb0 files don't have a header, so the data
// read into this buffer must be copied back into the 
// destination memory after probing for image type.
#define BL_HDRSIG_SIZE 7
static BYTE g_hdr[BL_HDRSIG_SIZE];

// Only provide download manifest to the OAL once per session
static BOOL g_fOEMNotified = FALSE;


#define SPIN_FOREVER        while (1)

ROMHDR * volatile const pTOC = (ROMHDR *)-1;     // Gets replaced by RomLoader with real address

BOOLEAN g_bBINDownload = TRUE;

static DownloadManifest g_DownloadManifest;
static BYTE g_downloadFilesRemaining = 1;


static BOOL KernelRelocate    (ROMHDR *const pTOC);
	   BOOL DownloadImage     (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr);
static BOOL IsKernelRegion    (DWORD   dwRegionStart, DWORD   dwRegionLength);

#ifdef SECURE_BOOTLOADER
static BOOL InitSecureBootLoader();
static BOOL DownloadSignedBin (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr);
static BOOL DownloadSignedNB0 (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr);
static VOID PrintPacketData   (PACKETDATA packetData);
#else
BOOL DownloadBin       (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr);
static BOOL DownloadNB0       (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr);
#endif

	   BOOL GetImageType      ();
static BOOL CheckImageManifest();
	   BOOL WriteImageToFlash ();
       BOOL ResetGlobalVariables();
	   BOOL SetDownloadedFlag(BOOL flag);
	   BOOL GetDownloadedFlag();

static VOID DumpMem (PBYTE Ptr, ULONG Len);
static VOID ComputeChecksum();


#define CURRENT_VERSION_MAJOR       1
#define CURRENT_VERSION_MINOR       4


#ifdef SECURE_BOOTLOADER

// List of public keys that may be used to verify images.
// Provided at link time by the platform.
extern PUBLICKEYDATA KeyData;

const unsigned char NKSignon[] = {
    "\nMicrosoft Windows CE Secure Bootloader Common Library Version %d.%d Built "
    __DATE__ " " __TIME__ "\r\n"
};

#else

const unsigned char NKSignon[] = {
    "\nMicrosoft Windows CE Bootloader Common Library Version %d.%d Built "
    __DATE__ " " __TIME__ "\r\n"
};

#endif // SECURE_BOOTLOADER

DWORD g_dwROMOffset;

PFN_OEMVERIFYMEMORY     g_pOEMVerifyMemory;
PFN_OEMREPORTERROR      g_pOEMReportError;
PFN_OEMCHECKSIGNATURE   g_pOEMCheckSignature;
PFN_OEMMULTIBINNOTIFY   g_pOEMMultiBINNotify;


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
#ifndef SHIP_BUILD
// output banner
    KITLOutputDebugString (NKSignon, CURRENT_VERSION_MAJOR, CURRENT_VERSION_MINOR);
#endif
    // (3) initialize platform (clock, drivers, transports, etc)
    if (!OEMPlatformInit ())
    {
        // spin forever
        HALT (BLERR_PLATINIT);
    }

    // (4) call OEM specific pre-download function
    switch (dwAction = OEMPreDownload ())
    {
    case BL_DOWNLOAD:
        // (5) download image
        if (!DownloadImage (&dwImageStart, &dwImageLength, &dwLaunchAddr))
        {
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
#ifndef SHIP_BUILD
            KITLOutputDebugString ("ROMHDR at Address %Xh\r\n", dwImageStart + ROM_SIGNATURE_OFFSET + sizeof (DWORD)); // right after signature
#endif
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
    default:
        // ERROR! spin forever
        HALT (BLERR_INVALIDCMD);
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


BL_IMAGE_TYPE GetImageType()
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
#ifndef SHIP_BUILD
        KITLOutputDebugString("\r\nBL_IMAGE_TYPE_MANIFEST\r\n\r\n");
#endif
		rval =  BL_IMAGE_TYPE_MANIFEST;
    }
    else if (!memcmp (g_hdr, "X000FF\x0A", BL_HDRSIG_SIZE))
    {
#ifndef SHIP_BUILD
        KITLOutputDebugString("\r\nBL_IMAGE_TYPE_MULTIXIP\r\n\r\n");
#endif
		rval =  BL_IMAGE_TYPE_MULTIXIP;
    }
    else if (!memcmp (g_hdr, "B000FF\x0A", BL_HDRSIG_SIZE))
    {
#ifndef SHIP_BUILD
        KITLOutputDebugString("\r\nBL_IMAGE_TYPE_BIN\r\n\r\n");
#endif
		rval =  BL_IMAGE_TYPE_BIN;
    }
    else if (!memcmp (g_hdr, "S000FF\x0A", BL_HDRSIG_SIZE))
    {
#ifndef SHIP_BUILD
        KITLOutputDebugString("\r\nBL_IMAGE_TYPE_SIGNED_BIN\r\n\r\n");
#endif
		rval =  BL_IMAGE_TYPE_SIGNED_BIN;
    }
    else if (!memcmp (g_hdr, "R000FF\x0A", BL_HDRSIG_SIZE))
    {
#ifndef SHIP_BUILD
        KITLOutputDebugString("\r\nBL_IMAGE_TYPE_SIGNED_NB0\r\n\r\n");
#endif
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
		HALT (BLERR_MAGIC);
        return FALSE;
    }

    // read region descriptions (start address and length).
    //
    if (!OEMReadData (sizeof (DWORD), (LPBYTE) &g_DownloadManifest.dwNumRegions) ||
        !OEMReadData ((g_DownloadManifest.dwNumRegions * sizeof(RegionInfo)), (LPBYTE) &g_DownloadManifest.Region[0]))
    {
        KITLOutputDebugString("\r\nERROR: Unable to read download manifest information.\r\n");
		HALT (BLERR_MAGIC);
        return FALSE;
    }

    // verify the packet checksum.
    //
    if (!VerifyChecksum((g_DownloadManifest.dwNumRegions * sizeof(RegionInfo)), (LPBYTE) &g_DownloadManifest.Region[0], dwRecChk))
    {
        KITLOutputDebugString ("\r\nERROR: Download manifest packet failed checksum verification.\r\n");
		HALT (BLERR_CHECKSUM);
        return FALSE;
    }

    return TRUE;
}


BOOL DownloadImage (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr)
{
    BOOL        rval = TRUE;
    DWORD       dwImageType;

    *pdwImageStart = *pdwImageLength = *pdwLaunchAddr = 0;

#ifdef SECURE_BOOTLOADER
    InitSecureBootLoader();
#endif

    //
    // Download each region (multiple can be sent)
    //
    do
    {
        dwImageType = GetImageType();
#ifndef SHIP_BUILD
		KITLOutputDebugString("DownloadImage\r\n*****Image type\r\n");
#endif        
        switch(dwImageType) 
        {
            case BL_IMAGE_TYPE_MANIFEST:
                // Platform Builder sends a manifest to indicate the following 
                // data consists of multiple .bin files /OR/ one .nb0 file.
                if (!CheckImageManifest()) {
                    HALT (BLERR_MAGIC);
                    return (FALSE);   
                }

                // Continue with download of next file
                // +1 to account for the manifest
                g_downloadFilesRemaining = (BYTE)(g_DownloadManifest.dwNumRegions + 1);
                continue;

#ifdef SECURE_BOOTLOADER
            case BL_IMAGE_TYPE_BIN:
#ifndef SHIP_BUILD
                KITLOutputDebugString("\r\n**\r\n");
                KITLOutputDebugString("** ERROR: This boot loader does not support unsigned .bin images.\r\n");
                KITLOutputDebugString("**        Image must be signed with a recognized private key.\r\n");
                KITLOutputDebugString("**\r\n");
#endif
				HALT (BLERR_SIGNATURE);
                return (FALSE);

            case BL_IMAGE_TYPE_SIGNED_BIN:
                rval &= DownloadSignedBin( pdwImageStart, pdwImageLength, pdwLaunchAddr );
                break;

            case BL_IMAGE_TYPE_SIGNED_NB0:
                rval &= DownloadSignedNB0( pdwImageStart, pdwImageLength, pdwLaunchAddr );
                break;

#else // not SECURE_BOOTLOADER

            case BL_IMAGE_TYPE_BIN:
#ifndef SHIP_BUILD
				KITLOutputDebugString("DownloadImage binary\r\n");
#endif
				rval &= DownloadBin( pdwImageStart, pdwImageLength, pdwLaunchAddr );
                break;

            case BL_IMAGE_TYPE_SIGNED_BIN:
#ifndef SHIP_BUILD
                KITLOutputDebugString("\r\n**\r\n");
                KITLOutputDebugString("** ERROR: This boot loader does not support signed .bin images.\r\n");
                KITLOutputDebugString("**\r\n");
#endif
				HALT (BLERR_SIGNATURE);
                return (FALSE);

            case BL_IMAGE_TYPE_SIGNED_NB0:
#ifndef SHIP_BUILD
                KITLOutputDebugString("\r\n**\r\n");
                KITLOutputDebugString("** ERROR: This boot loader does not support signed .nb0 images.\r\n");
                KITLOutputDebugString("**\r\n");
#endif
				HALT (BLERR_SIGNATURE);
                return (FALSE);
#endif // SECURE_BOOTLOADER

            case BL_IMAGE_TYPE_MULTIXIP:
#ifndef SHIP_BUILD
                KITLOutputDebugString("\r\n**\r\n");
                KITLOutputDebugString("** ERROR: The X000FF packet is an old-style multi-bin download manifest and it's no longer supported.\r\n");
                KITLOutputDebugString("** Please update your Platform Builder installation in you want to download multiple files.\r\n");
                KITLOutputDebugString("**\r\n");
#endif
				HALT (BLERR_MAGIC);
                return (FALSE);

            case BL_IMAGE_TYPE_UNKNOWN:
#ifdef SECURE_BOOTLOADER
#ifndef SHIP_BUILD
                KITLOutputDebugString("\r\n**\r\n");
                KITLOutputDebugString("** ERROR: Unrecognized image type (possibly unsigned .nb0).\r\n");
                KITLOutputDebugString("**        This boot loader does not support unsigned images.\r\n");
                KITLOutputDebugString("**        Image must be signed with a recognized private key.\r\n");
                KITLOutputDebugString("**\r\n");
#endif
				HALT (BLERR_MAGIC);
                return (FALSE);
#else
                // Assume files without a "type" header (e.g. raw data) are unsigned .nb0
                rval &= DownloadNB0( pdwImageStart, pdwImageLength, pdwLaunchAddr );
                break;
#endif                

            default:
                // should never get here
				KITLOutputDebugString("DownloadImage nothing\r\n");
				return (FALSE);
                
        }
    }
    while (--g_downloadFilesRemaining);

    
	if(rval)
	{
		ComputeChecksum();
		rval &= WriteImageToFlash();
	}
    return rval;
}


#ifdef SECURE_BOOTLOADER
static BOOL InitSecureBootLoader()
{
    // Initialize public keys for signature validation
    g_keyData.rgpbPublicKeys  = g_rgpbPublicKeys;       // pointer to array of public keys        
    g_keyData.rgdwKeyLengths  = g_rgdwKeyLengths;       // pointer to array of public key lengths 
    g_keyData.wNumPublicKeys  = g_wNumPublicKeys;       // number of public keys in rgpbPublicKeys
    g_keyData.wMinSearchIndex = 0;                      // starting array index of search         
    g_keyData.wMaxSearchIndex = g_wNumPublicKeys - 1;   // ending array index of search           

#ifndef SHIP_BUILD
    KITLOutputDebugString("Secure Boot Loader includes %d public keys\r\n",
		g_keyData.wNumPublicKeys);
#endif
        
    return TRUE;
}
#endif


#ifdef SECURE_BOOTLOADER
static BOOL DownloadSignedBin (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr)
{
    RegionInfo *pCurDownloadFile;
    BOOL        fIsFlash = FALSE;
    LPBYTE      lpDest = NULL;
    DWORD       dwImageStart, dwImageLength;
    DWORD       dwSignedDataLength, dwSigLength;
    BYTE        bFlags;
    DWORD       dwRecNum = 0;
    DWORD       dwRecAddr, dwRecLen, dwRecChk;
    DWORD       dwPacketNum = 1;
    DWORD       dwChunksRemaining = 0;
    DWORD       dwChunkOffset = 0;
    HRESULT     hr;
    PACKETDATA  packetData;

    g_bBINDownload = TRUE;


    if (!OEMReadData (sizeof (DWORD), (LPBYTE) &dwImageStart)   ||
        !OEMReadData (sizeof (DWORD), (LPBYTE) &dwImageLength))
    {
        KITLOutputDebugString ("Unable to read image start/length\r\n");
		HALT (BLERR_MAGIC);
        return (FALSE);
    }


    // If Platform Builder didn't provide a manifest (i.e., we're
    // only downloading a single .bin file), manufacture a manifest so we
    // can notify the OEM.
    //
    if (!g_DownloadManifest.dwNumRegions)
    {
        g_DownloadManifest.dwNumRegions             = 1;
        g_DownloadManifest.Region[0].dwRegionStart  = dwImageStart;
        g_DownloadManifest.Region[0].dwRegionLength = dwImageLength;
    }

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
    if (g_pOEMVerifyMemory && !g_pOEMVerifyMemory (pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionLength))
    {
        KITLOutputDebugString ("!OEMVERIFYMEMORY: Invalid image\r\n");
		HALT (BLERR_OEMVERIFY);
        return (FALSE);
    }

#ifdef DEBUG
    // Clearing memory ensures no garbage between sparse .bin records, so that 
    // our post-download checksum will be accurate.
    memset( (LPVOID) OEMMapMemAddr(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart),
                0, pCurDownloadFile->dwRegionLength );
#endif

    // Check for flash image. Start erasing if it is.
    // This is risky iff:
    //      We're downloading the bootloader
    //      OEM has implemented OEMStartEraseFlash()/OEMContinueEraseFlash()
    //      Download fails (e.g. image has been tampered with)
    //
    // In this scenario only, the device will be unbootable and require
    // JTAG recovery.
    //
    // The OEM should implement the flash functions to skip erase if the image
    // is a bootloader (i.e. defer erase until OEMFinishEraseFlash()). 
    // That prevents bricking the device.  
    //
    // It's safe to erase flash and/or flush RAM to flash when download the OS.
    // In fact this improves download speed, and lets you download images
    // much larger than available RAM.
    if ((fIsFlash = OEMIsFlashAddr (pCurDownloadFile->dwRegionStart)) 
        && !OEMStartEraseFlash (pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionLength))
    {
        KITLOutputDebugString ("Invalid flash address/length\r\n");
		HALT (BLERR_FLASHADDR);
        return (FALSE);
    }

    //------------------------------------------------------------------------
    //  Signed files are prefixed with a 16-byte random seed, which
    //  is included in the hash.
    //------------------------------------------------------------------------
    if (!OEMReadData (RANDOM_SEED_LENGTH, (LPBYTE) &packetData.bRandomSeed))
    {
        KITLOutputDebugString ("ERROR: Failed to read random seed at start of signed .bin file\r\n");
		HALT (BLERR_MAGIC);
        return (FALSE);
    }


    //------------------------------------------------------------------------
    //  Download signed packets (.bin records or record chunks)
    //------------------------------------------------------------------------

    while ( OEMReadData (sizeof (DWORD), (LPBYTE) &dwSignedDataLength) &&
            OEMReadData (sizeof (DWORD), (LPBYTE) &dwSigLength)        &&
            OEMReadData (sizeof (BYTE),  (LPBYTE) &bFlags) )
    {
#ifdef DEBUG
        KITLOutputDebugString("\r\n------------------------------------------------------------------------------\r\n");
        KITLOutputDebugString(" <> Packet [ %d ] dwSignedDataLength = 0x%x, dwSigLen = 0x%x, bFlags = 0x%x\r\n", 
            dwPacketNum, dwSignedDataLength, dwSigLength, bFlags);
#endif

        // Check if what we just read was a chunk header instead of a
        // signed packet header.
        //
        // If a .bin record is larger than the <packet size> parameter to 
        // ImageHash.exe, it will be broken into multiple chunks.
        //
        // This theoretically allows download of .bin records that are larger
        // than the bootloader's available RAM.  However BLCommon doesn't 
        // implement that.  The OEM can do it by creatively using 
        // OEMMapMemAddr() and OEMContinueEraseFlash() to periodically flush 
        // the RAM buffer to flash.

        if (bFlags & SBL_FLAG_CHUNK_HEADER)
        {
            // It's a chunk header; we must reinterpret the data we just read:
            //
            //  field 0: ignore             (always zero for a chunk header)
            //  field 1: dwChunksRemaining  (multiple chunks = one .bin record)
            //  field 2: bFlags             (no change)
            dwChunksRemaining = dwSigLength;
            dwChunkOffset     = 0;
#ifdef DEBUG
            KITLOutputDebugString(" <> Record chunk (dwChunksRemaining = %d)\r\n", dwChunksRemaining);
#endif             
            
            // Read the signed packet header
            if ( !(OEMReadData (sizeof (DWORD), (LPBYTE) &dwSignedDataLength) &&
                   OEMReadData (sizeof (DWORD), (LPBYTE) &dwSigLength)        &&
                   OEMReadData (sizeof (BYTE),  (LPBYTE) &bFlags)))
            {
                KITLOutputDebugString ("ERROR: Failed to read signed packet header %d, ABORT!\r\n", dwPacketNum);
				HALT (BLERR_CORRUPTED_DATA);
                return (FALSE);
            }
#ifdef DEBUG
            KITLOutputDebugString(" <> Packet [ %d ] dwSignedDataLength = 0x%x, dwSigLen = 0x%x, bFlags = 0x%x\r\n", 
                dwPacketNum, dwSignedDataLength, dwSigLength, bFlags);
#endif            
        }


        // Read the .bin record header
        if ( !(OEMReadData (sizeof (DWORD), (LPBYTE) &dwRecAddr) &&
               OEMReadData (sizeof (DWORD), (LPBYTE) &dwRecLen)  &&
               OEMReadData (sizeof (DWORD), (LPBYTE) &dwRecChk)) )
        {
            KITLOutputDebugString ("ERROR: Failed to read .bin record header %d, ABORT!\r\n", dwPacketNum);
			HALT (BLERR_CORRUPTED_DATA);
            return (FALSE);
        }
#ifdef DEBUG
        KITLOutputDebugString(" <> Record [ %d ] dwRecAddr = 0x%x, dwRecLen = 0x%x, dwRecChk = 0x%x\r\n", 
            dwRecNum, dwRecAddr, dwRecLen, dwRecChk);
#endif

        if (bFlags & SBL_FLAG_END_FILE)
        {
            // last packet always has unused signature block, read it!
            ASSERT(dwSignedDataLength == 0);
            ASSERT(dwSigLength <= sizeof(g_rgpbSignature));
            OEMReadData(dwSigLength, (LPBYTE) &g_rgpbSignature);
#ifndef SHIP_BUILD
            KITLOutputDebugString("Reached last record of signed .bin file\r\n");
#endif
			break;
        }

        // map the record address (FLASH data is cached, for example)
        // add offset if this is a record chunk
        lpDest = OEMMapMemAddr (pCurDownloadFile->dwRegionStart, dwRecAddr + dwChunkOffset);

        // read record (or record chunk)
        if (!OEMReadData (dwSignedDataLength, lpDest))
        {
            KITLOutputDebugString ("ERROR: Data packet %d corrupted, ABORT!\r\n", dwPacketNum);
			HALT (BLERR_CORRUPTED_DATA);
            return (FALSE);
        }
#ifdef DEBUG
        else
        {
            KITLOutputDebugString ("Read 0x%x bytes into cache: 0x%x (final address: 0x%x)\r\n", 
                dwSignedDataLength,
                lpDest,
                dwRecAddr + dwChunkOffset);   
                
            DumpMem( lpDest, 64 );                
        }
#endif
        
        
        // Check for last chunk in a fragmented .bin record
        if (0 != dwChunksRemaining) 
        {
            if (--dwChunksRemaining > 0) 
            {
                dwChunkOffset += dwSignedDataLength;
            } 
            else 
            {
                dwChunkOffset = 0;  
            }
#ifdef DEBUG
            KITLOutputDebugString("dwChunksRemaining = %d, offset = 0x%x\r\n", 
                dwChunksRemaining, dwChunkOffset);
#endif            
        }
        

        if (0 == dwChunksRemaining) 
        {
            LPBYTE cacheAddress = OEMMapMemAddr (pCurDownloadFile->dwRegionStart, dwRecAddr);
            
            // The packet signature makes this redundant, but we check anyway.
            if (!VerifyChecksum (dwRecLen, cacheAddress, dwRecChk))
            {
                HALT (BLERR_CHECKSUM);
                return (FALSE);
            }

            dwRecNum++; 
        }

        // Look for ROMHDR to compute ROM offset.  NOTE: romimage guarantees that the record containing
        // the TOC signature and pointer will always come before the record that contains the ROMHDR contents.
        //
        if (dwRecLen == sizeof(ROMHDR) && (*(LPDWORD) OEMMapMemAddr(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE))
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
#ifndef SHIP_BUILD
                KITLOutputDebugString("rom_offset=0x%x.\r\n", g_dwROMOffset); 
#endif
			}
        }

        // read packet signature
        ASSERT(dwSigLength <= sizeof(g_rgpbSignature));
        if (!OEMReadData (dwSigLength, (LPBYTE) &g_rgpbSignature))
        {
            KITLOutputDebugString ("ERROR: Failed to read signature for packet %d, ABORT!\r\n", dwPacketNum);
			HALT (BLERR_SIGNATURE);
            return (FALSE);
        }
    
        //------------------------------------------------------------------------
        //  Validate signature for this record or chunk
        //------------------------------------------------------------------------

        packetData.pbData       = lpDest;               // data to verify               
        packetData.dwDataLength = dwSignedDataLength;   // length of data in bytes              
        packetData.pbSig        = g_rgpbSignature;      // signature to verify               
        packetData.dwSigLength  = dwSigLength;          // length of signature in bytes                
        packetData.dwRecAddress = dwRecAddr;            // record address              
        packetData.dwRecLength  = dwRecLen;             // record length   
        packetData.dwRecCheck   = dwRecChk;             // record checksum  
        packetData.dwSequenceNumber = dwPacketNum;      // packet sequence number           
        packetData.bFlags       = bFlags;               // Flags

        PrintPacketData( packetData );

        hr = SBL_VerifyPacket( &packetData, &g_keyData );

        if (FAILED(hr)) {
            KITLOutputDebugString("ERROR: Signature validation failed for packet %d with error 0x%x, ABORT!\r\n", dwPacketNum, hr);
			HALT (BLERR_SIGNATURE);
            return (FALSE);
        }
        
        OEMShowProgress (dwPacketNum++);

        if (fIsFlash)
        {
            OEMContinueEraseFlash ();
        }
    }  // while( signed packets remaining )
    

    //------------------------------------------------------------------------
    //  Determine the image entry point
    //------------------------------------------------------------------------

    // Does this .bin file contain a TOC?
    if (*(LPDWORD) OEMMapMemAddr(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE)
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
    else if (g_DownloadManifest.dwNumRegions == 1)
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

#ifndef SHIP_BUILD
    KITLOutputDebugString("\r\n");
    KITLOutputDebugString("            ******************************************************\r\n");
    KITLOutputDebugString("            *                                                    *\r\n");
    KITLOutputDebugString("            *   This image was signed with a valid private key   *\r\n");
    KITLOutputDebugString("            *                                                    *\r\n");
    KITLOutputDebugString("            ******************************************************\r\n");
    KITLOutputDebugString("\r\n");

    KITLOutputDebugString("Downloaded %d records\r\n", dwRecNum);
#endif
    if (fIsFlash)
    {
        // finish the flash erase
        if (!OEMFinishEraseFlash())
        {
            HALT (BLERR_FLASH_ERASE);
            return (FALSE);
        }

    }

#ifndef SHIP_BUILD
    KITLOutputDebugString("ImageStart = 0x%x, ImageLength = 0x%x, LaunchAddr = 0x%x\r\n",
        *pdwImageStart, *pdwImageLength, *pdwLaunchAddr);
#endif

    return TRUE;
}
#endif // SECURE_BOOTLOADER



#ifdef SECURE_BOOTLOADER
static BOOL DownloadSignedNB0 (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr)
{
    RegionInfo *pCurDownloadFile;
    LPBYTE      lpDest = NULL;
    DWORD       dwPacketNum = 1;
    DWORD       dwSignedDataLength, dwSigLength;
    BYTE        bFlags;
    PACKETDATA  packetData;
    HRESULT     hr;
            
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
		HALT (BLERR_OEMVERIFY);
        return (FALSE);
    }


    //------------------------------------------------------------------------
    //  Signed files are prefixed with a 16-byte random seed, which
    //  is included in the hash.
    //------------------------------------------------------------------------

    if (!OEMReadData (RANDOM_SEED_LENGTH, (LPBYTE) &packetData.bRandomSeed))
    {
        KITLOutputDebugString ("ERROR: Didn't find random seed at start of signed .nb0 file\r\n");
		HALT (BLERR_MAGIC);
        return (FALSE);
    } else {
#ifdef DEBUG
        int i;
        KITLOutputDebugString("\r\nSigned file random seed = ");
        for (i = 0; i < RANDOM_SEED_LENGTH; i++) {
            KITLOutputDebugString("%x ", packetData.bRandomSeed[i]);   
        }
        KITLOutputDebugString("\r\n\r\n");
#endif        
    }


    //------------------------------------------------------------------------
    //  Download signed packets of raw data (.nb0)
    //------------------------------------------------------------------------

    lpDest = OEMMapMemAddr (pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart);

    while ( OEMReadData (sizeof (DWORD), (LPBYTE) &dwSignedDataLength) &&
            OEMReadData (sizeof (DWORD), (LPBYTE) &dwSigLength)        &&
            OEMReadData (sizeof (BYTE),  (LPBYTE) &bFlags) )
    {
#ifdef DEBUG
        KITLOutputDebugString("\r\n------------------------------------------------------------------------------\r\n");
        KITLOutputDebugString(" <> Packet [ %d ] dwDataLen = 0x%x, dwSigLen = 0x%x, bFlags = 0x%x\r\n", 
            dwPacketNum, dwSignedDataLength, dwSigLength, bFlags);
#endif

        // check bFlags
        if (bFlags & SBL_FLAG_END_FILE) 
        {
            // last packet always has unused signature block, read it!
            ASSERT(dwSignedDataLength == 0);
            ASSERT(dwSigLength <= sizeof(g_rgpbSignature));
            OEMReadData (dwSigLength, (LPBYTE) &g_rgpbSignature);
            break;
        } 
        else if (!(bFlags & SBL_FLAG_NORMAL)) 
        {
            KITLOutputDebugString("ERROR: bad bFlags 0x%x, ABORT!\r\n", bFlags);
			HALT (BLERR_CORRUPTED_DATA);
            return (FALSE);
        }
        
        // read packet data
        if (!OEMReadData (dwSignedDataLength, lpDest))
        {
            KITLOutputDebugString ("ERROR: failed when reading raw binary file.\r\n");
			HALT (BLERR_CORRUPTED_DATA);
            return (FALSE);
        }
        
        // read packet signature
        ASSERT(dwSigLength <= sizeof(g_rgpbSignature));
        if (!OEMReadData (dwSigLength, (LPBYTE) &g_rgpbSignature))
        {
            KITLOutputDebugString ("ERROR: Failed to read signature for packet %d, ABORT!\r\n", dwPacketNum);
			HALT (BLERR_SIGNATURE);
            return (FALSE);
        }
    
        //------------------------------------------------------------------------
        //  Validate the signature for this packet
        //------------------------------------------------------------------------

        packetData.pbData       = lpDest;               // data to verify               
        packetData.dwDataLength = dwSignedDataLength;   // length of data in bytes              
        packetData.pbSig        = g_rgpbSignature;      // signature to verify               
        packetData.dwSigLength  = dwSigLength;          // length of signature in bytes                
        packetData.dwRecAddress = 0;                    // record address              
        packetData.dwRecLength  = 0;                    // record length   
        packetData.dwRecCheck   = 0;                    // record checksum  
        packetData.dwSequenceNumber = dwPacketNum;      // packet sequence number           
        packetData.bFlags       = bFlags;               // Flags

        PrintPacketData( packetData );

        hr = SBL_VerifyPacket( &packetData, &g_keyData );

        if (FAILED(hr)) {
            KITLOutputDebugString("ERROR: Signature validation failed for packet %d with error 0x%x, ABORT!\r\n", dwPacketNum, hr);
			HALT (BLERR_SIGNATURE);
            return (FALSE);
        }

        lpDest += dwSignedDataLength;
        
        OEMShowProgress (dwPacketNum++);

    } // end while(packets remaining)


#ifndef SHIP_BUILD
    KITLOutputDebugString("\r\n");
    KITLOutputDebugString("            ******************************************************\r\n");
    KITLOutputDebugString("            *                                                    *\r\n");
    KITLOutputDebugString("            *   This image was signed with a valid private key   *\r\n");
    KITLOutputDebugString("            *                                                    *\r\n");
    KITLOutputDebugString("            ******************************************************\r\n");
    KITLOutputDebugString("\r\n");
#endif
    //------------------------------------------------------------------------
    // PB sends the file size for .nb0 files (headers and all).
    // Subtract the overhead of headers/footers; this is the true image size.
    //------------------------------------------------------------------------

    pCurDownloadFile->dwRegionLength -= BL_HDRSIG_SIZE;
    pCurDownloadFile->dwRegionLength -= RANDOM_SEED_LENGTH;
    pCurDownloadFile->dwRegionLength -= (dwPacketNum) * SBL_PACKET_HEADER_SIZE;
    pCurDownloadFile->dwRegionLength -= (dwPacketNum) * dwSigLength;

#ifndef SHIP_BUILD
    KITLOutputDebugString("Image size after stripping signed headers/footers for %d packets: 0x%x\r\n",
        dwPacketNum,
        pCurDownloadFile->dwRegionLength);
#endif

    //------------------------------------------------------------------------
    //  Determine the image entry point
    //------------------------------------------------------------------------

    *pdwImageStart  = pCurDownloadFile->dwRegionStart;
    *pdwLaunchAddr  = pCurDownloadFile->dwRegionStart;
    *pdwImageLength = pCurDownloadFile->dwRegionLength;

#ifndef SHIP_BUILD
    KITLOutputDebugString("ImageStart = 0x%x, ImageLength = 0x%x, LaunchAddr = 0x%x\r\n",
        *pdwImageStart, *pdwImageLength, *pdwLaunchAddr);
#endif

    //------------------------------------------------------------------------
    // If this is a flash image, erase flash.
    // For signed images, we don't do this until the entire image has been
    // validated.
    //------------------------------------------------------------------------
    if (OEMIsFlashAddr(pCurDownloadFile->dwRegionStart))
    {
        BOOL rval = TRUE;

        rval &= OEMStartEraseFlash(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionLength);
                OEMContinueEraseFlash();
        rval &= OEMFinishEraseFlash();

        if (!rval)
        {
            KITLOutputDebugString ("Invalid flash address/length or flash operation failed\r\n");
			HALT (BLERR_FLASHADDR);
            return (FALSE);
        }
    }
    
    return TRUE;
}
#endif // SECURE_BOOTLOADER



BOOL DownloadBin (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr)
{
    RegionInfo *pCurDownloadFile;
    BOOL        fIsFlash = FALSE;
    LPBYTE      lpDest = NULL;
    DWORD       dwImageStart, dwImageLength, dwRecAddr, dwRecLen, dwRecChk;
    DWORD       dwRecNum = 0;

    g_bBINDownload = TRUE;

    if (!OEMReadData (sizeof (DWORD), (LPBYTE) &dwImageStart)
        || !OEMReadData (sizeof (DWORD), (LPBYTE) &dwImageLength))
    {
        KITLOutputDebugString ("Unable to read image start/length\r\n");
		//HALT (BLERR_MAGIC);
        return (FALSE);
    }

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

#ifdef DEBUG
    // Clearing memory ensures no garbage between sparse .bin records, so that 
    // our post-download checksum will be accurate.
    memset( (LPVOID) OEMMapMemAddr(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart),
                0, pCurDownloadFile->dwRegionLength );
#endif


    //------------------------------------------------------------------------
    //  Download .bin records
    //------------------------------------------------------------------------

    while ( OEMReadData (sizeof (DWORD), (LPBYTE) &dwRecAddr)  &&
            OEMReadData (sizeof (DWORD), (LPBYTE) &dwRecLen)   &&
            OEMReadData (sizeof (DWORD), (LPBYTE) &dwRecChk) )
    {
#ifdef DEBUG
        KITLOutputDebugString(" <> Record [ %d ] dwRecAddr = 0x%x, dwRecLen = 0x%x\r\n", 
            dwRecNum, dwRecAddr, dwRecLen);
#endif

        // last record of .bin file uses sentinel values for address and checksum.
        if (!dwRecAddr && !dwRecChk)
        {
            break;
        }

        // map the record address (FLASH data is cached, for example)
        lpDest = OEMMapMemAddr (pCurDownloadFile->dwRegionStart, dwRecAddr);

        // read data block
        if (!OEMReadData (dwRecLen, lpDest))
        {
            KITLOutputDebugString ("****** Data record %d corrupted, ABORT!!! ******\r\n", dwRecNum);
			//HALT (BLERR_CORRUPTED_DATA);
            return (FALSE);
        }

        if (!VerifyChecksum (dwRecLen, lpDest, dwRecChk))
        {
            KITLOutputDebugString ("****** Checksum failure on record %d, ABORT!!! ******\r\n", dwRecNum);
			//HALT (BLERR_CHECKSUM);
            return (FALSE);
        }

        // Look for ROMHDR to compute ROM offset.  NOTE: romimage guarantees that the record containing
        // the TOC signature and pointer will always come before the record that contains the ROMHDR contents.
        //
        if (dwRecLen == sizeof(ROMHDR) && (*(LPDWORD) OEMMapMemAddr(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE))
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
#ifndef SHIP_BUILD
                KITLOutputDebugString("rom_offset=0x%x.\r\n", g_dwROMOffset); 
#endif
			}
        }

        // verify partial checksum
        OEMShowProgress (dwRecNum++);

        if (fIsFlash)
        {
            OEMContinueEraseFlash ();
        }
    }  // while( records remaining )
    

    //------------------------------------------------------------------------
    //  Determine the image entry point
    //------------------------------------------------------------------------

    // Does this .bin file contain a TOC?
    if (*(LPDWORD) OEMMapMemAddr(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE)
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
    else if (g_DownloadManifest.dwNumRegions == 1)
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

#ifndef SHIP_BUILD
    KITLOutputDebugString("ImageStart = 0x%x, ImageLength = 0x%x, LaunchAddr = 0x%x\r\n",
        *pdwImageStart, *pdwImageLength, *pdwLaunchAddr);
#endif
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
		HALT (BLERR_OEMVERIFY);
        return (FALSE);
    }

    // check for flash image. Start erasing if it is.
    if ((fIsFlash = OEMIsFlashAddr (pCurDownloadFile->dwRegionStart)) 
        && !OEMStartEraseFlash (pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionLength))
    {
        KITLOutputDebugString ("Invalid flash address/length\r\n");
		HALT (BLERR_FLASHADDR);
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
    memcpy(lpDest, g_hdr, BL_HDRSIG_SIZE);
    lpDest += BL_HDRSIG_SIZE;

    if (!OEMReadData ((pCurDownloadFile->dwRegionLength - BL_HDRSIG_SIZE), lpDest))
    {
        KITLOutputDebugString ("ERROR: failed when reading raw binary file.\r\n");
		HALT (BLERR_CORRUPTED_DATA);
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
            HALT (BLERR_FLASH_ERASE);
            return (FALSE);
        }
    }

#ifndef SHIP_BUILD
    KITLOutputDebugString("ImageStart = 0x%x, ImageLength = 0x%x, LaunchAddr = 0x%x\r\n",
        *pdwImageStart, *pdwImageLength, *pdwLaunchAddr);
#endif
    return TRUE;
}


BOOL WriteImageToFlash()
{
    BOOL  bFlash;
    DWORD i;
    
#ifndef SHIP_BUILD
    KITLOutputDebugString("\r\nCompleted file(s):\r\n");
    KITLOutputDebugString("-------------------------------------------------------------------------------\r\n");
#endif

    for (i = 0; i < g_DownloadManifest.dwNumRegions; i++)
    {
        RegionInfo *pRegion = &g_DownloadManifest.Region[i];

        bFlash = OEMIsFlashAddr( pRegion->dwRegionStart );
        
#ifndef SHIP_BUILD
        KITLOutputDebugString("[%d]: Address=0x%x  Length=0x%x  Name=\"%s\" Target=%s\r\n",
            i, 
            pRegion->dwRegionStart, 
            pRegion->dwRegionLength, 
            pRegion->szFileName,
            (bFlash ? "FLASH" : "RAM"));
#endif
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


#ifdef SECURE_BOOTLOADER
VOID PrintPacketData( PACKETDATA packetData ) 
{
#ifdef DEBUG
    DWORD i;
    
    KITLOutputDebugString("packetData.pbData        = 0x%x\r\n", packetData.pbData);
    
    KITLOutputDebugString("pbData = [");
    for (i = 0; i < packetData.dwDataLength; i++) {
        KITLOutputDebugString("%x ", packetData.pbData[i]);   
        
        if (20 < i && i < packetData.dwDataLength) {
            KITLOutputDebugString("...");
            break;
        }
    }
    KITLOutputDebugString("]\r\n\r\n");
    
    KITLOutputDebugString("packetData.dwDataLength  = 0x%x\r\n", packetData.dwDataLength);
    KITLOutputDebugString("packetData.pbSig         = 0x%x\r\n", packetData.pbSig       );
    KITLOutputDebugString("packetData.dwSigLength   = 0x%x\r\n", packetData.dwSigLength );
    KITLOutputDebugString("packetData.dwRecAddress  = 0x%x\r\n", packetData.dwRecAddress);
    KITLOutputDebugString("packetData.dwRecLength   = 0x%x\r\n", packetData.dwRecLength );
    KITLOutputDebugString("packetData.dwRecCheck    = 0x%x\r\n", packetData.dwRecCheck  );
    KITLOutputDebugString("packetData.dwSeqNum      = 0x%x\r\n", packetData.dwSequenceNumber);
    KITLOutputDebugString("packetData.bFlags        = 0x%x\r\n", packetData.bFlags      );
    KITLOutputDebugString("\r\n");
    
    KITLOutputDebugString("Signature = [");
    for (i = 0; i < sizeof(g_rgpbSignature); i++) {
        KITLOutputDebugString("%x ", g_rgpbSignature[i]);   
    }
    KITLOutputDebugString("]\r\n\r\n");
#endif
}
#endif



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

BOOL ResetGlobalVariables()
{
	memset(&g_DownloadManifest, 0, sizeof(g_DownloadManifest));
	g_fOEMNotified = 0;
	g_downloadFilesRemaining = 1;
	return TRUE;
}
BOOL SetDownloadedFlag(BOOL flag)
{
	g_bBINDownload = flag;
	return TRUE;
}

BOOL GetDownloadedFlag()
{
	return g_bBINDownload;
}


char Nib2HexChar (BYTE Nibble)
{
    if (Nibble < 0x0a) {
		return Nibble+'0';
    } else if (Nibble < 0x10) {
		return Nibble-0x0a+'A';
    } else {
		return '?';
    }
}


VOID DumpMem (PBYTE Ptr, ULONG Len)
{
#ifdef DEBUG
	ULONG i,j;
	char OutString[256];
	int Index;

    KITLOutputDebugString("0x%x - 0x%x\r\n", Ptr, Ptr + Len);
	for (j=0; j < Len; j+= 16) {
		Index = 0;
		// Print out a leader
        KITLOutputDebugString("0x%x ", j);
        Index = 0;        
		
		for (i=0; i < 16; i++) {
			if (i+j < Len) {
				OutString[Index++] = Nib2HexChar ((BYTE)(Ptr[i+j]>>4));
				OutString[Index++] = Nib2HexChar ((BYTE)(Ptr[i+j] & 0x0f));
			} else {
				OutString[Index++] = ' ';
				OutString[Index++] = ' ';
			}
			if (i == 7) {
				OutString[Index++] = '-';
			} else {
				OutString[Index++] = ' ';
			}
		}
		for (i=0; i < 2; i++)
			OutString[Index++] = ' ';

		for (i=0; i < 16; i++) {
			if ((i+j) >=  Len) {
				OutString[Index++] = ' ';
			} else if ((Ptr[i+j] < ' ') || (Ptr[i+j] >= 0x7f)) {
				OutString[Index++] = '.';
			} else {
				OutString[Index++] = (CHAR)Ptr[i+j];
			}
		}
		OutString[Index] = '\0';
		KITLOutputDebugString("%s\r\n", OutString);
	}
#endif DEBUG	
}

