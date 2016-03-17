/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998-2010 Texas Instruments Incorporated           |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**          
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**     
**+----------------------------------------------------------------------+**
***************************************************************************/

#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__

//! The len in byte of a checksum
#define CHECKSUM_LEN	4

//! This typedef defines the byte representation of a checksum
typedef BYTE			Checksum_VALUE[CHECKSUM_LEN];

//! This typedef define the string representation of a checksum
#define BYTEOTOCHAR     2
#define ENDOFSTRINGCHAR 1
typedef TCHAR			Checksum_STRINGVALUE[CHECKSUM_LEN * BYTEOTOCHAR + ENDOFSTRINGCHAR];

//! Instance data of module
typedef struct
{
    DWORD currentChecksum;
} T_CHECKSUM_INSTANCE, *LPT_CHECKSUM_INSTANCE;

/*! Initializes an instance of this module
 *	  
 *  \param  instance pointer to memory for instance
 *	\return TRUE in case of success
 */ 
BOOL Checksum_Initialize(LPT_CHECKSUM_INSTANCE instance);

/*! Deinitializes an instance
 *	  
 *  \param  instance instance of this module to deinitialize
 *	\return TRUE in case of success
 */ 
BOOL Checksum_Deinitialize(LPT_CHECKSUM_INSTANCE instance);

/*! Resets the added data
 *	  
 *  \param  instance an instance of this module to reset
 *	\return TRUE in case of success
 */ 
BOOL Checksum_Reset(LPT_CHECKSUM_INSTANCE instance);

/*! Adds data for checksum
 *	  
 *  \param  instance an instance of this module
 *  \param  lpBuffer a pointer on a buffer to add for the checksum computation
 *  \param  dwLenBuffer the len for the data pointed by lpBuffer
 *	\return TRUE in case of success
 */ 
BOOL Checksum_AddData(const LPT_CHECKSUM_INSTANCE instance, const LPBYTE lpBuffer, DWORD dwLenBuffer);

/*! Adds data for checksum from a file
 *	  
 *  \param  instance an instance of this module
 *  \param  szfilePath the file to open
 *	\return TRUE in case of success
 */ 
BOOL Checksum_AddDataFromFile(const LPT_CHECKSUM_INSTANCE instance, LPCTSTR szfilePath);

/*! Returns the current checksum value from the previously added data
 *	  
 *  \param  instance an instance of this module
 *  \param  checksum the returned checksum
 *	\return TRUE in case of success
 */ 
BOOL Checksum_GetCurrentChecksum(const LPT_CHECKSUM_INSTANCE instance, Checksum_VALUE checksum);

/*! Tests if the added data corresponds to the checksum parameter
 *	  
 *  \param  instance an instance of this module
 *  \param  checksum the checksum to test
 *	\return TRUE in case of success
 */ 
BOOL Checksum_IsChecksumCorrect(const LPT_CHECKSUM_INSTANCE instance, const Checksum_VALUE checksum);

/*! Tests if two checksum parameters are identical
 *	  
 *  \param  checksum the first checksum to test
 *  \param  checksum the second checksum to test
 *	\return TRUE in case of success
 */ 
BOOL Checksum_IsChecksumIdentical(const Checksum_VALUE checksum1, const Checksum_VALUE checksum2);

/*!Transforms binary presentation of checksum to string representation.
 *	  
 *  \param  checksum The checksum to convert
 *  \param  checksumString the checksum converted in string
 *	\return TRUE in case of success
 */ 
BOOL Checksum_ChecksumToString(const Checksum_VALUE checksum, Checksum_STRINGVALUE checksumString);

/*! Transforms string representation of checksum to binary presentation.
 *	  
 *  \param  checksumString string representation of checksum
 *  \param  checksum binary presentation of checksum
 *	\return TRUE in case of success
 */ 
BOOL Checksum_StringToChecksum(const Checksum_STRINGVALUE checksumString, Checksum_VALUE checksum);

#endif // __CHECKSUM_H__
