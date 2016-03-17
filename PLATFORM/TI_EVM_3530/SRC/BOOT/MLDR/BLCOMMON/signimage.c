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
#include <windows.h>
#include <halether.h>
#include <minasn1.h>
#include <mincrypt.h>
#include <traverse.h>

// Matches BIB file EXTENSION area (note BIB file names are forced to lower case).
const unsigned char CatSigExtName[] = "cat_sig";

// Secure loader globals
extern DWORD g_dwROMOffset;
DWORD g_hHash;
BYTE g_hashCtx[MINCRYPT_MAX_HASH_CONTEXT_LEN];
BYTE rgbOID_Attr[] =
//OID (1.3.6.1.4.1.311.12.2.1) has the following encoding:
// "1.3.6.1.4.1.311.12.2.1"
{0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x0C, 0x02, 0x01};

BYTE  rgbManufacturerTag[] = 
//"Manufacturer" as a BMP string
{ 0x00, 0x4D, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x75, 0x00, 0x66, 0x00, 0x61, 0x00, 0x63, 0x00, 
0x74, 0x00, 0x75, 0x00, 0x72, 0x00, 0x65, 0x00, 0x72};

BYTE  rgbModelTag[] = 
//"Model" as a BMP string
{0x00, 0x4D, 0x00, 0x6F, 0x00, 0x64, 0x00, 0x65, 0x00, 0x6C};


BOOL UpdateHash(DWORD dwAddr, DWORD dwSize)
{
    CRYPT_DER_BLOB ScratchBlob;

    ScratchBlob.cbData = dwSize;
    ScratchBlob.pbData = (BYTE *)dwAddr;

    if (MinCryptUpdateHashMemory(CALG_SHA1, g_hHash, 1, &ScratchBlob))
    {
        return(FALSE);
    }

    return(TRUE);
}


BOOL CheckSignature(DWORD dwStoreBase, DWORD dwRunBase, BOOL fTestSignature,
                    OPTIONAL IN BYTE* pbDeviceMakeData, OPTIONAL IN DWORD dwDeviceMakeSize,
                    OPTIONAL IN BYTE* pbDeviceModelData, OPTIONAL IN DWORD dwDeviceModelSize)
{
#define REBASE_ADDR(x)   (x + (dwStoreBase - dwRunBase))
    DWORD dwpTOC = 0;
    ROMHDR *pROMHdr = NULL;
    EXTENSION *pExt = NULL;
    BYTE *pSig = NULL;
    DWORD dwSigLen = 0;
    DWORD cbHash = 0;
    DWORD cbAttr = 0;
    LONG rglErr = 0;
    LONG lStatus = 0;
    BYTE rgbHash[MINCRYPT_MAX_HASH_LEN];
    CRYPT_HASH_BLOB rgHashBlob;
    MAP_CAT_INFO rgMapCatInfo;
    CRYPT_DATA_BLOB rgExtensionTagName;
    CRYPT_DATA_BLOB rgExtensionValue;
    CRYPT_DATA_BLOB rgAttrEncodedOIDBlob;


    // Check for TOC signature.
    //
    if (*(LPDWORD)(dwStoreBase + ROM_SIGNATURE_OFFSET) != ROM_SIGNATURE)
    {
        KITLOutputDebugString ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
        KITLOutputDebugString ("! ERROR: Did not find pTOC signature.  ABORTING. !\r\n");
        KITLOutputDebugString ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");

        return(FALSE);
    }

    // Get pointer to TOC.
    //
    dwpTOC = *(LPDWORD)(dwStoreBase + ROM_SIGNATURE_OFFSET + sizeof(ULONG));

    // Locate the catalog file signature record via the ROMHDR extension pointer.
    //
    pROMHdr = (ROMHDR *)REBASE_ADDR(dwpTOC);

    if (!pROMHdr->pExtensions)
    {
        KITLOutputDebugString("ERROR: No extensions list (NULL pointer).\r\n");
        return(FALSE);
    }

    pExt = (EXTENSION *)REBASE_ADDR((DWORD)pROMHdr->pExtensions);

    while(pExt && memcmp(pExt->name, CatSigExtName, strlen(CatSigExtName)))
    {
        if (!pExt->pNextExt)
        {
            pExt = NULL;
            break;
        }

        pExt = (EXTENSION *)REBASE_ADDR((DWORD)pExt->pNextExt);
    }

    if (!pExt)
    {
        KITLOutputDebugString("ERROR: No signature record in extensions list (an extension block hasn't been reserved in this image)?\r\n");
        return(FALSE);
    }

    // Get location and length of the signature.
    //
    dwSigLen = pExt->length;
    pSig     = (BYTE *)REBASE_ADDR((DWORD)pExt->pdata);
    if (!dwSigLen)
    {
        KITLOutputDebugString("ERROR: Signature length is zero.  Was this image stamped with a signature?\r\n");
        return(FALSE);
    }
    
    KITLOutputDebugString("INFO: Found signature (Address=0x%x  Length=0x%x).\r\n", (DWORD)pSig, dwSigLen);

    // Hash memory initialization.
    //
    // MinCryptCreateHashMemoryMTH() is the thread-safe version of 
    // MinCryptCreateHashMemory().  We're running in a boot loader without 
    // threading, but it's recommended to use this API to be future-proof.
    //
    // The other MinCrypt APIs were thread-safe to begin with, so there are no 
    // xxxMTH() equivalents.
    //
    // The hash context is filled in by MinCrypt and used for its own purposes; 
    // just make sure it doesn't get modified/invalidated until we're finished 
    // using g_hHash.
    //
    MinCryptCreateHashMemoryMTH(CALG_SHA1, g_hashCtx, &g_hHash);

    // Traverse the image and compute the hash.
    //
    if (!SigProcessImage((ROMHDR *)dwpTOC, (dwStoreBase - dwRunBase), UpdateHash))
    {
        return(FALSE);
    }

    // Finish computing the hash.
    //
    MinCryptGetHashParam(CALG_SHA1, g_hHash, rgbHash, &cbHash);
    
    rgHashBlob.cbData = cbHash;
    rgHashBlob.pbData = rgbHash;

    memset(&rgMapCatInfo, 0, sizeof(MAP_CAT_INFO));
    
    rgMapCatInfo.FileBlob.cbData = dwSigLen;
    rgMapCatInfo.FileBlob.pbData = pSig;

    rgAttrEncodedOIDBlob.cbData = sizeof(rgbOID_Attr);
    rgAttrEncodedOIDBlob.pbData = rgbOID_Attr;

    // Compute a CAT record for the hashed data and compare against the CAT record stamped in the image.
    //
    lStatus = MinCryptVerifyHashInCatalogBlob(CALG_SHA1, 1, &rgHashBlob, 1, &rgMapCatInfo, fTestSignature, &rglErr);
    if (lStatus || rglErr)
    {
        KITLOutputDebugString("ERROR: MinCryptVerifyHashInCatalogs returned 0x%x (rglErr=0x%x).\r\n", lStatus, rglErr);
        return(FALSE);
    }

    // Optionally verify the Make attribute.    
    if (pbDeviceMakeData && dwDeviceMakeSize)
    {
        // Now check the Make string
        rgExtensionTagName.cbData = sizeof(rgbManufacturerTag);
        rgExtensionTagName.pbData = rgbManufacturerTag;
       
        rgExtensionValue.cbData = dwDeviceMakeSize;
        rgExtensionValue.pbData = pbDeviceMakeData;
        
        lStatus = MinCryptVerifyExtension(rgMapCatInfo, rgAttrEncodedOIDBlob, rgExtensionTagName, rgExtensionValue);
        if (lStatus)
        {
            KITLOutputDebugString("ERROR: MinCryptVerifyExtension failed to verify manufacturer\r\n");
            return (FALSE);
        }
    }
    
    // Optionally verify the Model attribute.    
    if (pbDeviceModelData && dwDeviceModelSize)
    {
        rgExtensionTagName.cbData = sizeof(rgbModelTag);
        rgExtensionTagName.pbData = rgbModelTag;
       
        rgExtensionValue.cbData = dwDeviceModelSize;
        rgExtensionValue.pbData = pbDeviceModelData;
    
        lStatus = MinCryptVerifyExtension(rgMapCatInfo, rgAttrEncodedOIDBlob, rgExtensionTagName, rgExtensionValue);
        if (lStatus)
        {
            KITLOutputDebugString("ERROR: MinCryptVerifyExtension failed to verify Model\r\n");
            return (FALSE);
        }
    }

    KITLOutputDebugString("INFO: Signature check passed!\r\n");
    
    return(TRUE);
}

