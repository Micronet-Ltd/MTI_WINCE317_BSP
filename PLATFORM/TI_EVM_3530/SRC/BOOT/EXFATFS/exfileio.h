//===================================================================
//
//	Module Name:	BOOTLOADER 
//	
//	File Name:		fileio.h
//	
//	Description:	FAT12/16/32 file system i/o for block devices
//
//===================================================================
// Copyright (c) 2007- 2009 BSQUARE Corporation. All rights reserved.
//===================================================================
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//

#ifndef _EXFILEIO_H
#define _EXFILEIO_H

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//
// File IO Data Structures
//
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MAX_FILE_NAME	255 //now in exFat

typedef struct _EXFILEHANDLE 
{
	WCHAR	name[MAX_FILE_NAME + 1]; //maximum 255 chars
	UINT32	file_size;				// Size of the file, in bytes

	// The following variables are used internally by the fileio routines
	UINT32 current_sector;	
	UINT32 current_cluster;
	UINT16 current_sector_in_cluster;
	UINT32* buffer;
	UINT32	bytes_in_buffer;

	UINT16	flags;
} EXFILEHANDLE, *PEXFILEHANDLE;

// S_FILEIO_OPERATIONS
// This data structure contains a set of pointers to functions that provide 
// access to a block device.  This structure needs to be initialized with the
// correct functions before calling FileIoInit.  The fileio routines use
// these function pointers to access the block device.
// Note that the implementation of these functions is custom for each device.
typedef struct exfileio_operations_t 
{
	// Pointer to an initialization function for the block device.  Function
	// takes a pointer to a data structure that describes the device.  
	// This function should initialize the device, making it ready for read
	// access.
	int (*init)(void *drive_info);

	// Pointer to a diagnostic function that can return information about the
	// block device.  This function takes a pointer to the device information
	// data structure, and a pointer to a user buffer with size equal to the 
	// sector size.  Note that the use of the user buffer is not specified.  
	// NOTE - This diagnostic function may not be called by all versions of 
	// the fileio library.
	int (*identify)(void *drive_info, void *Sector);

	// Pointer to a function that reads the specified logical sector into
	// the provided sector buffer.  Function also takes a pointer to the 
	// device specific information data structure.
	int (*read_sector)(void *drive_info, SECTOR_ADDR LogicalSector, void *pSector, unsigned long SectorCount);

	// Pointer to a device specific data structure.  This data structure 
	// must contain whatever information is needed by other device specific
	// functions.  The device information data structure must be initialized 
	// with the correct values, and this pointer must be initialized to the 
	// data structure before calling any of the fileio routines.
	void *drive_info;
} S_EXFILEIO_OPERATIONS, *S_EXFILEIO_OPERATIONS_PTR;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//
// File IO Function Return Codes
//
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#define FILEIO_STATUS_OK			0
#define FILEIO_STATUS_INIT_FAILED	1
#define FILEIO_STATUS_OPEN_FAILED	2
#define FILEIO_STATUS_READ_FAILED	3
#define FILEIO_STATUS_READ_EOF		4

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//
// File IO Public Functions
//
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// NAME: FileIoInit()
//
// DESCRIPTION: Initializes fileio subsystem on any FAT12/16/32 formatted block 
// device.  
//
// PARAMETERS:
//	fileio_ops	Pointer to a preinitialized S_FILEIO_OPERATIONS structure
//
// RETURNS: 	FILEIO_STATUS_OK on success, error code on failure
//
//----------------------------------------------------------------------------

int ExFileIoInit(S_EXFILEIO_OPERATIONS_PTR fileio_ops);

//----------------------------------------------------------------------------
//
// NAME: FileNameToDirEntry()
//
// DESCRIPTION: create 8+3 FAT file system directory entry strings from 8+3 
// file name.  This function must be used to create the proper filename root 
// and extension strings for use by FileIoOpen().  This function is typically
// used to initialize the 'name' and 'extension' fields of a FILEHANDLE structure.
//
// PARAMETERS:	
//	pFileName	Pointer to WCHAR string containing original file name
//	pName		Pointer to buffer that will contain resulting filename root.
//				This pointer is typically the 'name' field of a FILEHANDLE 
//				structure.
//	pExtension	Pointer to buffer that will contain resulting filename extension.
//				This pointer is typically the 'extension' field of a FILEHANDLE
//				structure.
//
// RETURNS: 	Nothing
//
//----------------------------------------------------------------------------

void ExFileNameToDirEntry(LPCWSTR pFileName, PWSTR pName );

//----------------------------------------------------------------------------
//
// NAME: FileIoOpen()
//
// DESCRIPTION: Opens the specified file for sequential read access.
//
// PARAMETERS:	
//	pFileio_ops	Pointer to S_FILEIO_OPERATIONS structure
//	pFile		Pointer to FILEHANDLE structure with 'name' and 'extension' 
//				fields already initialized using FileNameToDirEntry function.
//
// RETURNS: 	FILEIO_STATUS_OK on success, error code on failure
//
//----------------------------------------------------------------------------

int ExFileIoOpen(S_EXFILEIO_OPERATIONS_PTR pFileio_ops, PEXFILEHANDLE pFile);

//----------------------------------------------------------------------------
//
// NAME: FileIoRead()
//
// DESCRIPTION: Reads specified number of bytes from file into user buffer. 
//				File read pointer is saved in the FILEHANDLE structure, subsequent
//				calls to this function will continue reading from the previous
//				location.
//
// PARAMETERS:	
//	pFileio_ops	Pointer to S_FILEIO_OPERATIONS structure
//	pFile		Pointer to FILEHANDLE structure with 'name' and 'extension' 
//				fields already initialized using FileNameToDirEntry function.
//  pDest       Pointer to destination for data
//  Count       Number of bytes to read
//
// RETURNS: 	FILEIO_STATUS_OK on success, error code on failure
//
//----------------------------------------------------------------------------

int ExFileIoRead(S_EXFILEIO_OPERATIONS_PTR pFileio_ops, PEXFILEHANDLE pFile, void *pDest, DWORD Count);

// typedef for file handle, includes sector buffer
typedef struct _EXFILESYS_INFO {
//	BIOS_PARAMETER_BLOCK BiosParameterBlock;
	UINT32	partition_start;
	UINT32	root_directory_start;
	UINT32	file_data_area_start;
	UINT32	bytes_per_sector;
	UINT32	sectors_per_cluster;
	UINT32	fat_offset;
	UINT32	total_sectors;
	UINT32	FatType;
} EXFILESYS_INFO, *pEXFILESYS_INFO;


#pragma pack(1)
//
// exFAT BPB
//
typedef struct
{                                //  Offset    Size
    BYTE JmpBoot[3];             //  0         3
    UCHAR VersionId[8];          //  3         8
    UCHAR MustBeZero[53];        //  11        53
    ULONGLONG PartitionOffset;   //  64        8
    ULONGLONG PartitionLength;   //  72        8
    ULONG FATOffset;             //  80        4
    ULONG FATLength;             //  84        4
    ULONG ClusterHeapOffset;     //  88        4
    ULONG ClusterCount;          //  82        4
    ULONG FirstClusterInRoot;    //  96        4
    ULONG SerialNumber;          //  100       4
    USHORT FSVersion;            //  104       2
    USHORT ExtFlags;             //  106       2
    UCHAR BytesPerSector;        //  108       1
    UCHAR SectorsPerCluster;     //  109       1
    UCHAR NumberOfFATs;          //  110       1
    UCHAR DriveID;               //  111       1
    UCHAR PercentInUse;          //  112       1
    UCHAR Reserved[7];           //  113       7
} BPBEX, *PBPBEX;


typedef struct
{
	BYTE		EntryType;
	BYTE		SecondaryCount;
	WORD		SetChecksum;
	WORD		GeneralPrimeryFlags;
	BYTE		CustomDefined[14];
	ULONG		FirstCluster;
	ULONGLONG	DataLength;
} GenDirEntry;
//Stream Extension DirectoryEntry
typedef struct
{
	BYTE		EntryType;
	BYTE		GeneralSecondaryFlags;
	BYTE		Reserved1;
	BYTE		NameLength;
	WORD		NameHash;
	WORD		Reserved2;
	ULONGLONG	ValidDataLength;
	ULONG		Reserved3;
	ULONG		FirstCluster;
	ULONGLONG	DataLength;
} StreamExtDirEntry;

typedef struct
{
	BYTE		EntryType;
	BYTE		GeneralSecondaryFlags;
	WORD		FileName[15];
} FileNameDirEntry;
#pragma pack()

#endif
