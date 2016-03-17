//===================================================================
//
//	Module Name:	BTL
//	
//	File Name:		exfileio.c
//	
//	Description:	EXFAT file system i/o
//
//===================================================================

#include "eboot.h"

#include "exfileio.h"
#include "..\fatfs\filesys.h"



//----------------------------------------------------------------------------
//
// defines
//
//----------------------------------------------------------------------------
#define SECTOR_SIZE_M	2048

#define CURRENT_SECTOR_EOF_VALUE							0xffff
//#define DEBUG	1

// debug message enables
#ifdef DEBUG
	#define BOOTLOADER_DEBUG_DISPLAY_PARTITION_TABLE		1
	#define BOOTLOADER_DEBUG_DISPLAY_BPB					1
	#define BOOTLOADER_DEBUG_DISPLAY_DIRECTORY_ENTRIES		1
	#define BOOTLOADER_DEBUG_DISPLAY_CALLS					1
	#define BOOTLOADER_DEBUG_DUMP_SECTOR_ZERO				0
	#define BOOTLOADER_DEBUG_DISPLAY_ERRORS					1
#else
	#define BOOTLOADER_DEBUG_DISPLAY_PARTITION_TABLE		0
	#define BOOTLOADER_DEBUG_DISPLAY_BPB					0
	#define BOOTLOADER_DEBUG_DISPLAY_DIRECTORY_ENTRIES		0
	#define BOOTLOADER_DEBUG_DISPLAY_CALLS					0
	#define BOOTLOADER_DEBUG_DUMP_SECTOR_ZERO				0
	#define BOOTLOADER_DEBUG_DISPLAY_ERRORS					0
#endif

// This uses a sector buffer to speed up disk read by avoiding reads of the same sector
#define FILEIO_USE_FAT_SECTOR_BUFFER						1

// Enables the file system type from partition table to be overridden if
// the BPB specifies that the disk has less than 4096 clusters.
#define FF_CONTIGOUS  0x2                   // the file is contiguous - don't use FAT

//----------------------------------------------------------------------------
//
// private data
//
//----------------------------------------------------------------------------

static EXFILESYS_INFO FilesysInfo;

#if FILEIO_USE_FAT_SECTOR_BUFFER
	static UCHAR FatBuffer[SECTOR_SIZE_M];
	static UINT32 FatBufferSectorNumber = 0;
#endif

//----------------------------------------------------------------------------
//
// Debug Data Display function
//
//----------------------------------------------------------------------------

#ifdef DEBUG
    #if BOOTLOADER_DEBUG_DUMP_SECTOR_ZERO || BOOTLOADER_DEBUG_DUMP_ALL_SECTORS

	    static void DumpData(unsigned char *pData, int ByteCount)
	    {
		    unsigned long i, j, k;
		    unsigned char *pCh;
		
		    i = ByteCount;
		    pCh = pData;
		    while (i) 
		    {
			    OALMSG(OAL_INFO, (L"%x ", ByteCount - i));
			    k = (i < 16) ? i : 16;
			    for (j = 0; j < k; j++) 
				    OALMSG(OAL_INFO, (L"%x ", pCh[j]));
			    // align last line
			    if (k < 16) 
				    for (j = 0; j < 16 - k; j++) 
					    OALMSG(OAL_INFO, (L"   "));
			    for (j = 0; j < k; j++) 
			    {
				    if ((pCh[j] < ' ') || (pCh[j] > '~')) 
					    OALMSG(OAL_INFO, (L"."));
				    else 
					    OALMSG(OAL_INFO, (L"%c", pCh[j]));
			    }
			    OALMSG(OAL_INFO, (L"\r\n"));
			    i -= k;
			    pCh += k;
		    }
	    }
    #endif
#endif

static BOOL ByteIsPowerOfTwo(UINT8 b)
{
	return ( b == 1 || b == 2 || b == 4 || b == 8 || b == 16 || b == 32 || b == 64 || b == 128 || b == 256 );
}

/*
 *
 *  NAME: ExFileIoReadNextSector()
 *
 *  PARAMETERS:	pointer to file descriptor structure
 *              pointer to sector buffer (512 bytes)
 *
 *  DESCRIPTION: reads next sequential sector from file
 *
 *  RETURNS: 0 on success, error code on failure
 *
 */
static int ExFileIoReadNextSector(S_EXFILEIO_OPERATIONS_PTR pfileio_ops, PEXFILEHANDLE pFile, void * pSector) 
{
	UINT32 SectorNumber;
	UINT16 FatOffsetInSector, FatSectorNumber;

	// check if is this an attempt to read past the end of the file
	if (pFile->current_sector_in_cluster == CURRENT_SECTOR_EOF_VALUE)
	{
		#if BOOTLOADER_DEBUG_DISPLAY_CALLS
			OALMSG(OAL_INFO, (L"EXFATIO: ExFileIoReadNextSector EOF\r\n"));
		#endif
		return FILEIO_STATUS_READ_EOF;
	}
		
	// check for invalid current_cluster
	if (pFile->current_cluster < FAT32_CLUSTER_NEXT_START || (pFile->current_cluster > FAT32_CLUSTER_NEXT_END && pFile->current_cluster < FAT32_CLUSTER_LAST_START))
	{
		#if BOOTLOADER_DEBUG_DISPLAY_ERRORS
			OALMSG(OAL_INFO, (L"EXFATIO: ExFileIoReadNextSector failed, invalide current_cluster 0x%x\r\n", pFile->current_cluster));
		#endif
		return FILEIO_STATUS_READ_FAILED;
	}

	// calculate the sector number, used to read sector later
	// file data area starting sector
	SectorNumber = FilesysInfo.file_data_area_start;
	// plus first sector of current cluster 
	// Note: first 2 fat entries are reserved, making first cluster in file area = 2
	// ??? The reserved fat entry stuff is poorly documented, this code may be wrong
	#if BOOTLOADER_DEBUG_DISPLAY_ERRORS
		if (pFile->current_cluster < 2)
			OALMSG(OAL_INFO, (L"EXFATIO: ExFileIoReadNextSector() called with current_cluster < 2!\r\n"));
	#endif
	SectorNumber += (pFile->current_cluster - 2) * (FilesysInfo.sectors_per_cluster);
	// plus sector offset within cluster
	SectorNumber += pFile->current_sector_in_cluster;


	// read the requested sector
	if(!pfileio_ops->read_sector(pfileio_ops->drive_info, (FilesysInfo.partition_start + SectorNumber), pSector, 1))
	{
		#if BOOTLOADER_DEBUG_DISPLAY_ERRORS
			OALMSG(OAL_INFO, (L"EXFATIO: ExFileIoReadNextSector failed reading file or dir: LBA 0x%x, cluster 0x%x, sector in cluster 0x%x\r\n", (FilesysInfo.partition_start + SectorNumber), pFile->current_cluster - 2, pFile->current_sector_in_cluster));
		#endif
		return FILEIO_STATUS_READ_FAILED;
	}
	OALMSG(0, (L"#%X:\r\n%02X %02X %02X %02X %02X %02X %02X %02X\r\n", SectorNumber, 
	   ((BYTE*)pSector)[0], ((BYTE*)pSector)[1], ((BYTE*)pSector)[2], ((BYTE*)pSector)[3], ((BYTE*)pSector)[4], ((BYTE*)pSector)[5], ((BYTE*)pSector)[6], ((BYTE*)pSector)[7]));

	if(FF_CONTIGOUS & pFile->flags)
	{
		pFile->current_cluster++;
		return FILEIO_STATUS_OK;
	}

	// increment current_sector_in_cluster and check if this is the last sector in the cluster
	if (++(pFile->current_sector_in_cluster) >= (FilesysInfo.sectors_per_cluster))
	{
		// the current read is the last sector in the current cluster, 
		// get the next cluster number from the FAT

		// check to see if this was the last cluster, set flag so next read returns EOF
		if (pFile->current_cluster >= FAT32_CLUSTER_LAST_START)
		{
			pFile->current_sector_in_cluster = CURRENT_SECTOR_EOF_VALUE;
		}
		else
		{			
			FatOffsetInSector = (pFile->current_cluster * 4) % FilesysInfo.bytes_per_sector;
			FatSectorNumber = (pFile->current_cluster * 4) / FilesysInfo.bytes_per_sector;

			#if FILEIO_USE_FAT_SECTOR_BUFFER
			// if not already contained in FatBuffer,
			// read sector containing next fat entry, FatSectorNumber is an
			// offset from the start of the partition + reserved area
			if (FatBufferSectorNumber != FilesysInfo.partition_start + FilesysInfo.fat_offset + FatSectorNumber)
			{
				FatBufferSectorNumber = FilesysInfo.partition_start + FilesysInfo.fat_offset + FatSectorNumber;
				if( !pfileio_ops->read_sector(pfileio_ops->drive_info, FatBufferSectorNumber, FatBuffer, 1) )
				{
					#if BOOTLOADER_DEBUG_DISPLAY_ERRORS
						OALMSG(OAL_INFO, (L"EXFATIO: ExFileIoReadNextSector failed reading FAT: LBA 0x%x (cluster 0x%x, sector 0x%x)\r\n", FatBufferSectorNumber, pFile->current_cluster, pFile->current_sector_in_cluster));
					#endif
					
					FatBufferSectorNumber = 0;
					return FILEIO_STATUS_READ_FAILED;
				}
			}
						//  update current fat index
			pFile->current_cluster = (*(((UINT32 *)FatBuffer) + (FatOffsetInSector/4))) & FAT32_CLUSTER_MASK;
			OALMSG(0, (L"EXFATIO: current_cluster %x *FatBuffer %x FatBufferSectorNumber %x FatOffsetInSector %x\r\n", 
							pFile->current_cluster, (*(((UINT32 *)FatBuffer) + (FatOffsetInSector/4))), FatBufferSectorNumber, FatOffsetInSector));
			#else
			// read sector containing next fat entry, FatSectorNumber is an
			// offset from the start of the partition + reserved area
			if(!pfileio_ops->read_sector(pfileio_ops->drive_info, (FilesysInfo.partition_start + FilesysInfo.fat_offset + FatSectorNumber), pSector, 1))
			{
				#if BOOTLOADER_DEBUG_DISPLAY_ERRORS
					OALMSG(OAL_INFO, (L"EXFATIO: read_sector %u (FAT) failed\r\n", (FilesysInfo.partition_start + FilesysInfo.fat_offset + FatSectorNumber)));
				#endif
				return FILEIO_STATUS_READ_FAILED;
			}

			//  update current fat index
			pFile->current_cluster = *(((UINT32 *)pSector) + (FatOffsetInSector/4)) & FAT32_CLUSTER_MASK;
			#endif
					
			//  reset current_sector_in_cluster
			pFile->current_sector_in_cluster = 0;

			if (pFile->current_cluster >= FAT32_CLUSTER_LAST_START)
			{
				// no more clusters, signal end of file or directory
				pFile->current_sector_in_cluster = CURRENT_SECTOR_EOF_VALUE;
				OALMSG(OAL_INFO, (L"EXFATIO: read_sector %x \r\n", pFile->current_cluster));
			}
		}
	}

	// update sector count
	pFile->current_sector += 1;
	
	return FILEIO_STATUS_OK;
}

//----------------------------------------------------------------------------
//
// public functions
//
//----------------------------------------------------------------------------

void ExFileNameToDirEntry(LPCWSTR pFileName, PWSTR pName)
{
	int i;

    //OALMSG(1, (L"FileNameToDirEntry: \""));

	// copy name
	for (i = 0; i < MAX_FILE_NAME; i++)
	{
		pName[i] = pFileName[i]; 

		if( 0 == pName[i] )
		{
			break;
		}
		if( pName[i] >= L'a' && pName[i] <= L'z' )
			pName[i] -= 0x20; 
	}
}

/*
 *
 *  NAME: FileIoInit()
 *
 *  PARAMETERS:	pointer to base of ATA disk drive registers
 *
 *  DESCRIPTION: initializes fileio subsystem
 *
 *  RETURNS: 0 on success, error code on failure
 *
 */
int ExFileIoInit(S_EXFILEIO_OPERATIONS_PTR pfileio_ops) 
{
	UINT16 Sector[SECTOR_SIZE_M/sizeof(UINT16)];
	UINT8* pSector = (UINT8 *)Sector;
	int i;
	UINT32 boot_sector				= 0;
	UCHAR partition_type			= 0;

	int PartitionTableEntryCount	= 4;
	BPBEX	Bpbex					= {0};//iii
	
	const char FatName[] = "EXFAT";

	// initialize device driver
	#if BOOTLOADER_DEBUG_DISPLAY_CALLS
		OALMSG(OAL_INFO, (L"EXFATIO: FileIoInit() calling init.\r\n"));
	#endif

	if( !pfileio_ops->drive_info )// fallite already must be initialized 
	{
		OALMSG(OAL_INFO, (L"EXFATIO: init failed.\r\n"));
		return FILEIO_STATUS_INIT_FAILED;
	}

	#if BOOTLOADER_DEBUG_DISPLAY_CALLS
		OALMSG(OAL_INFO, (L"EXFATIO: FileIoInit() calling read_sector to get partition table\r\n"));
	#endif

	// read in candidate partition table sector
	if(!pfileio_ops->read_sector(pfileio_ops->drive_info, boot_sector, pSector, 1))
	{
		#if BOOTLOADER_DEBUG_DISPLAY_ERRORS
			OALMSG(OAL_INFO, (L"EXFATIO: read_sector %u (PT) failed\r\n", boot_sector));
		#endif
		return FILEIO_STATUS_INIT_FAILED;
	}

	#if BOOTLOADER_DEBUG_DUMP_SECTOR_ZERO || BOOTLOADER_DEBUG_DUMP_ALL_SECTORS
		DumpData((unsigned char *)pSector, 512);
	#endif
		
	// check for valid BPB (Bios Parameter Block) in sector zero (old DOS disk organization, no partition table)
	//if ( (*pSector == 0xe9 || *pSector == 0xeb) && (((PBOOT_SECTOR)pSector)->bpb.bytes_per_sector == 512) )
	//if ( (*pSector == 0xe9 || *pSector == 0xeb) && ((BYTE_STRUCT_2_READ(((PBOOT_SECTOR)pSector)->bpb.bytes_per_sector)) == 512) )
	if (   
		(*pSector == 0xe9 || *pSector == 0xeb) &&
		((BYTE_STRUCT_2_READ(((PBOOT_SECTOR)pSector)->bpb.bytes_per_sector)) == 512) &&
		ByteIsPowerOfTwo(((PBOOT_SECTOR)pSector)->bpb.sectors_per_cluster) &&
		(((PBOOT_SECTOR)pSector)->bpb.media_descriptor == 0xf8)
	)
	{
		#if BOOTLOADER_DEBUG_DISPLAY_BPB
				OALMSG(OAL_INFO, (L"EXFATIO: Active partition type is not supported (0x%x)\r\n", partition_type));
			#endif
			return FILEIO_STATUS_INIT_FAILED;
	}

	// assume that sector 0 contains a partition table
	#if BOOTLOADER_DEBUG_DISPLAY_PARTITION_TABLE
		for (i = 0; i < PartitionTableEntryCount; i++)
		{
			OALMSG(OAL_INFO, (L"EXFATIO: PartitionTable[%d] Flag:0x%x, Type:0x%x, Start:0x%X, Size:0x%X\r\n", 
				i,
				((PPARTITION_TABLE)pSector)->Entry[i].ActivePartitionFlag, 
				((PPARTITION_TABLE)pSector)->Entry[i].PartitionType, 
				((PPARTITION_TABLE)pSector)->Entry[i].PartitionStartLBA, 
				((PPARTITION_TABLE)pSector)->Entry[i].PartitionSize));
		}
	#endif

	// search the partition table for the active partition
	for (i = 0; i < PartitionTableEntryCount; i++)
	{
		if (((PPARTITION_TABLE)pSector)->Entry[i].ActivePartitionFlag == 0x80 || 
			((((PPARTITION_TABLE)pSector)->Entry[i].PartitionType != 0x00	) &&
			(((PPARTITION_TABLE)pSector)->Entry[i].PartitionType != 0x20	))		)//iii
		{
			boot_sector = ((PPARTITION_TABLE)pSector)->Entry[i].PartitionStartLBA;
			partition_type = ((PPARTITION_TABLE)pSector)->Entry[i].PartitionType;
			break;
		}
	}
	if (i == PartitionTableEntryCount)
	{
		#if BOOTLOADER_DEBUG_DISPLAY_CALLS
			OALMSG(OAL_INFO, (L"EXFATIO: FileIoInit() no active partition found\r\n"));
		#endif

		return FILEIO_STATUS_INIT_FAILED;
	}

	#if BOOTLOADER_DEBUG_DISPLAY_CALLS
		OALMSG(OAL_INFO, (L"EXFATIO: trying partition %d\r\n", i));
	#endif

	// check partition type
	switch (partition_type)
	{
		case 0x0b:
		case 0x0c:
			#if BOOTLOADER_DEBUG_DISPLAY_BPB
				OALMSG(OAL_INFO, (L"EXFATIO: Active partition type is FAT32\r\n"));
			#endif
			FilesysInfo.FatType = FAT_TYPE_FAT32;
			break;
		default:
			#if BOOTLOADER_DEBUG_DISPLAY_BPB
				OALMSG(OAL_INFO, (L"EXFATIO: Active partition type is not supported (0x%x)\r\n", partition_type));
			#endif
			return FILEIO_STATUS_INIT_FAILED;
			break;
	}					

	#if BOOTLOADER_DEBUG_DISPLAY_CALLS
		OALMSG(OAL_INFO, (L"EXFATIO: FileIoInit() calling read_sector %X to read MBR for active partition\r\n",	boot_sector ));
	#endif

	// read in the master boot record (MBR), fill in the BPB
	if(!pfileio_ops->read_sector(pfileio_ops->drive_info, boot_sector, pSector, 1))
	{
		#if BOOTLOADER_DEBUG_DISPLAY_ERRORS
			OALMSG(OAL_INFO, (L"EXFATIO: read_sector %u (MBR) failed\r\n", boot_sector));
		#endif
		return FILEIO_STATUS_INIT_FAILED;
	}
	
	memcpy( &Bpbex, pSector, sizeof(BPBEX));
	
	#if BOOTLOADER_DEBUG_DISPLAY_CALLS
		OALMSG(OAL_INFO, (L"EXFATIO: FileIoInit() copying BPB into FilesysInfo structure.\r\n"));

		OALMSG(OAL_INFO, (L"BR: VersionId   %c%c%c%c%c\r\n",Bpbex.VersionId[0],	Bpbex.VersionId[1],
															Bpbex.VersionId[2],	Bpbex.VersionId[3],Bpbex.VersionId[4] ));
		OALMSG(OAL_INFO, (L"BR: PartitionOffset		%X\r\n", Bpbex.PartitionOffset));
		OALMSG(OAL_INFO, (L"BR: PartitionLength		%X\r\n", Bpbex.PartitionLength));
		OALMSG(OAL_INFO, (L"BR: FATOffset			%X\r\n", Bpbex.FATOffset));
		OALMSG(OAL_INFO, (L"BR: FATLength			%X\r\n", Bpbex.FATLength));
		OALMSG(OAL_INFO, (L"BR: ClusterHeapOffset	%X\r\n", Bpbex.ClusterHeapOffset));
		OALMSG(OAL_INFO, (L"BR: ClusterCount		%X\r\n", Bpbex.ClusterCount));
		OALMSG(OAL_INFO, (L"BR: FirstClusterInRoot	%X\r\n", Bpbex.FirstClusterInRoot));
		OALMSG(OAL_INFO, (L"BR: SerialNumber		%X\r\n", Bpbex.SerialNumber));
		OALMSG(OAL_INFO, (L"BR: FSVersion			%X\r\n", Bpbex.FSVersion));
		OALMSG(OAL_INFO, (L"BR: ExtFlags			%X\r\n", Bpbex.ExtFlags));
		OALMSG(OAL_INFO, (L"BR: BytesPerSector      %X\r\n", Bpbex.BytesPerSector));
		OALMSG(OAL_INFO, (L"BR: SectorsPerCluster   %X\r\n", Bpbex.SectorsPerCluster));
		OALMSG(OAL_INFO, (L"BR: NumberOfFATs        %X\r\n", Bpbex.NumberOfFATs));
		OALMSG(OAL_INFO, (L"BR: DriveID				%X\r\n", Bpbex.DriveID));
		OALMSG(OAL_INFO, (L"BR: PercentInUse        %X\r\n", Bpbex.PercentInUse));
	#endif

	if( memcmp( FatName, Bpbex.VersionId, 5 ) )//"EXFAT" - must be written
	{
		#if BOOTLOADER_DEBUG_DISPLAY_BPB
			OALMSG(OAL_INFO, (L"EXFATIO: ERROR - it is not exFat\r\n"));
		#endif
		return FILEIO_STATUS_INIT_FAILED;
	}

	
	FilesysInfo.sectors_per_cluster		= 1 << Bpbex.SectorsPerCluster;
	FilesysInfo.bytes_per_sector		= 1 << Bpbex.BytesPerSector;
	FilesysInfo.total_sectors			= Bpbex.ClusterCount * FilesysInfo.sectors_per_cluster;
	FilesysInfo.file_data_area_start	= (LONG)Bpbex.PartitionOffset + Bpbex.ClusterHeapOffset; 
	FilesysInfo.root_directory_start	= Bpbex.FirstClusterInRoot;
	
	FilesysInfo.fat_offset				= Bpbex.FATOffset;
	
	if( ( 2 == Bpbex.NumberOfFATs ) && (Bpbex.ExtFlags & 0x1) )//if 2 FATs and second FAT is active
		FilesysInfo.fat_offset += Bpbex.FATLength;

	// check for partition type override due to small number of sectors
	if (FilesysInfo.total_sectors == 0)
	{
		#if BOOTLOADER_DEBUG_DISPLAY_BPB
			OALMSG(OAL_INFO, (L"EXFATIO: Disk has under 4096 clusters, switching to FAT12\r\n"));
		#endif
		return FILEIO_STATUS_INIT_FAILED;
	}

	// sanity check - sector size
	if (FilesysInfo.bytes_per_sector > SECTOR_SIZE_M)//size of buffer - can be changed
	{
		#if BOOTLOADER_DEBUG_DISPLAY_BPB
			OALMSG(OAL_INFO, (L"EXFATIO: ERROR bytes_per_sector = %X\r\n"));
		#endif
		return FILEIO_STATUS_INIT_FAILED;
	}

	#if BOOTLOADER_DEBUG_DISPLAY_CALLS
		OALMSG(OAL_INFO, (L"EXFATIO: FileIoInit() initializing data structures.\r\n"));
	#endif
	FilesysInfo.partition_start = boot_sector;

	return FILEIO_STATUS_OK;
}
//TargetFileName must be in UPPER case
UINT16 GetNameHash(WCHAR * TargetFileName)
{
	UINT16	Hash = 0;
	BYTE*	z;
	UINT32	Index = 0;

	while( TargetFileName[Index] )
	{
		// convert the characters one at a time
		z = (BYTE*)&(TargetFileName[Index]);

		// apply both bytes to the hash
		Hash	= 	( (Hash & 1) ? 0x8000 : 0 ) | ( Hash >> 1 );
		Hash	+= 	(WORD)(*z);
		z++;
		Hash	= 	( (Hash & 1) ? 0x8000 : 0 ) | ( Hash >> 1 );
		Hash	+= 	(WORD)(*z);

		Index++;
	}
	
	return Hash;
} 
/*
#define	IN_USE_BIT				0x80
#define	TYPE_CATEGORY_BIT		0X40
#define	TYPE_IMPORTANCE_BIT		0X20

#define	TYPE_CODE_MASK			0x1F //5 bits
*/
#define DIR_ENTRY_TYPE			0x85
#define STREAM_EXTN_TYPE		0xC0 //Stream Extension
#define FILE_NAME_TYPE			0xC1 

/*
 *
 *  NAME: FileIoOpen()
 *
 *  PARAMETERS:	pointer to FILEHANDLE structure, with name and extension fields filled in
 *
 *  DESCRIPTION: opens the file for sequential read
 *
 *  RETURNS: 0 on success, error code on failure
 *
 */
int ExFileIoOpen(S_EXFILEIO_OPERATIONS_PTR pfileio_ops, PEXFILEHANDLE pFile) 
{
	UINT16 Sector[SECTOR_SIZE_M/sizeof(UINT16)];
	void* pSector = (void*)Sector;

	UINT32	DataFirst = 0, DataLen = 0;
	int		status;
	int		found = 0, i;
	UINT16	DirEntryCount	= 0;
	BYTE	PrevEntryType	= 0;
	UINT16	PrevAttr		= 0; 
	UINT16	LastAttr		= 0; 
	UINT32	NameLen			= 0;

	WCHAR*	pName	= pFile->name;	

	StreamExtDirEntry*	pEntry;
	WORD				NameHash = GetNameHash( pFile->name );
	
	OALMSG(0, (L"ExFileIoOpen: FileName %s NameHash %X\r\n", pFile->name, NameHash));
	
	// setup to read first directory sector
#if FILEIO_USE_FAT_SECTOR_BUFFER
	FatBufferSectorNumber				= 0;
#endif
	pFile->current_cluster				= FilesysInfo.root_directory_start;
	pFile->current_sector_in_cluster	= 0;
	pFile->current_sector				= 0;
	pFile->flags						= 0;
	while(!found)
	{
		if( (status = ExFileIoReadNextSector(pfileio_ops, pFile, pSector)) != FILEIO_STATUS_OK )
		{
			#if BOOTLOADER_DEBUG_DISPLAY_ERRORS
					OALMSG(1, (L"ExFileIoOpen: read_sector %u (next DIR) failed\r\n", FilesysInfo.root_directory_start ));
			#endif
			return FILEIO_STATUS_OPEN_FAILED;
		}

		pEntry = (StreamExtDirEntry *)pSector;
		for( DirEntryCount = 0; DirEntryCount < FilesysInfo.bytes_per_sector / sizeof(GenDirEntry); DirEntryCount++ )
		{	
			if(!DataFirst) 
			{
				//Stream Extension Directory Entry: TypeCode Field = 0, TypeImportance Field = 0
				if( STREAM_EXTN_TYPE == pEntry->EntryType )
				{
					if( NameHash == pEntry->NameHash )//after that, check next entry file name
					{
						
						if( ( PrevEntryType != DIR_ENTRY_TYPE	) ||
							( PrevAttr & 0x10 )					)//0x10 is directory
						{
							OALMSG(0, (L"ExFileIoOpen: File is not found\r\n"));
							return FILEIO_STATUS_OPEN_FAILED;
						}
						
						DataFirst	= pEntry->FirstCluster;
						DataLen		= (UINT32)pEntry->ValidDataLength;
						LastAttr	= pEntry->GeneralSecondaryFlags; 

						OALMSG(0, (L"ExFileIoOpen: FLAGS: prev %x last %x\r\n", PrevAttr, LastAttr ));
						OALMSG(0, (L"ExFileIoOpen: pEntry: %x %x %x %x\r\n", 
							(UINT32)PrevAttr, (UINT32)pEntry->FirstCluster, (UINT32)pEntry->ValidDataLength, (UINT32)pEntry->GeneralSecondaryFlags  ));
					}
				}
			}
			else//NameHash found - compare file name
			{
				WCHAR wch;

				if( FILE_NAME_TYPE != pEntry->EntryType )
				{
					OALMSG(0, (L"File name entry is not found\r\n"));
					return FILEIO_STATUS_OPEN_FAILED;
				}
				OALMSG(0, (L"ExFileIoOpen: File Name Entry: flags %x type %x\r\n", pEntry->GeneralSecondaryFlags, pEntry->EntryType   ));
				for( i = 0; i < 15; ++i )//file name - from second WORD
				{
					wch = ((FileNameDirEntry*)pEntry)->FileName[i];
					if( wch >= L'a' && wch <= L'z' )
						wch -= 0x20; 

					if( *pName != wch )
					{
						OALMSG(0, (L"File name is not found\r\n"));
						return FILEIO_STATUS_OPEN_FAILED;
					}
					if( 0 == *pName++ && NameLen )
					{
						found = 1;
						break;
					}
					NameLen++;
				}
			}
			OALMSG(0, (	L"\r\n%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n"
			L"%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n", 
			((BYTE*)pEntry)[0], ((BYTE*)pEntry)[1], ((BYTE*)pEntry)[2], ((BYTE*)pEntry)[3], ((BYTE*)pEntry)[4], ((BYTE*)pEntry)[5], ((BYTE*)pEntry)[6], ((BYTE*)pEntry)[7],
			((BYTE*)pEntry)[8], ((BYTE*)pEntry)[9], ((BYTE*)pEntry)[10], ((BYTE*)pEntry)[11], ((BYTE*)pEntry)[12], ((BYTE*)pEntry)[13], ((BYTE*)pEntry)[14], ((BYTE*)pEntry)[15],
			((BYTE*)pEntry)[16], ((BYTE*)pEntry)[17], ((BYTE*)pEntry)[18], ((BYTE*)pEntry)[19], ((BYTE*)pEntry)[20], ((BYTE*)pEntry)[21], ((BYTE*)pEntry)[22], ((BYTE*)pEntry)[23],
			((BYTE*)pEntry)[24], ((BYTE*)pEntry)[25], ((BYTE*)pEntry)[26], ((BYTE*)pEntry)[27], ((BYTE*)pEntry)[28], ((BYTE*)pEntry)[29], ((BYTE*)pEntry)[30], ((BYTE*)pEntry)[31]));

			if(found)
				break;

			PrevEntryType	= pEntry->EntryType;
			PrevAttr		= ((GenDirEntry*)pEntry)->GeneralPrimeryFlags;
			pEntry++;
		}	
	}

	if(found) 
	{
		pFile->current_cluster				= DataFirst;
		pFile->file_size					= DataLen;
		pFile->current_sector_in_cluster	= 0;
		pFile->flags						= LastAttr;

		OALMSG(0,(L"File found: DataFirst 0x%x data length %x flags %x\r\n", DataFirst, DataLen, pFile->flags));		
		return FILEIO_STATUS_OK;
	}
	
	return FILEIO_STATUS_OPEN_FAILED;
}
/*
 *
 *  NAME: FileIoRead()
 *
 *	pFileio_ops	Pointer to S_FILEIO_OPERATIONS structure
 *	pFile       Pointer to FILEHANDLE structure with 'name' and 'extension' 
 *              fields already initialized using FileNameToDirEntry function.
 *  pDest       Pointer to destination for data
 *  Count       Number of bytes to read
 *
 *  DESCRIPTION: reads next sequential sector from file
 *
 *  RETURNS: 0 on success, error code on failure
 *
 */
int ExFileIoRead(S_EXFILEIO_OPERATIONS_PTR pfileio_ops, PEXFILEHANDLE pFile, void* pDest, DWORD Count)
{
	int status;
	//UINT8 * s;

//	while (Count)
//	{
		// create pointer to start of valid data in buffer
/*		s = pFile->buffer + (FilesysInfo.bytes_per_sector - pFile->bytes_in_buffer);	

		// if any bytes are in the buffer copy them to destination
		while (Count && pFile->bytes_in_buffer)
		{
			*((UINT8 *)pDest) = *s;
			((UINT8 *)pDest)++;
			s++;
			Count--;
			pFile->bytes_in_buffer--;
		}
*/
		// if Count is at least one sector size then read sector directly into caller's buffer
		while(Count)// >= FilesysInfo.bytes_per_sector)
		{
			OALMSG(0, (L"Count %02X\r\n", Count));
			if ((status = ExFileIoReadNextSector(pfileio_ops, pFile, (UINT8 *)pDest)) != FILEIO_STATUS_OK)
				return status;

			(UINT8 *)pDest += FilesysInfo.bytes_per_sector;
			if(Count < FilesysInfo.bytes_per_sector)
				Count = 0;
			else
				Count -= FilesysInfo.bytes_per_sector;
		}

		// if Count is not zero, read a sector into the file structure sector buffer
/*		if (Count)
		{
			OALMSG(0, (L"Count %02X\r\n", Count));
//			if ((status = ExFileIoReadNextSector(pfileio_ops, pFile, pFile->buffer)) != FILEIO_STATUS_OK)
			if ((status = ExFileIoReadNextSector(pfileio_ops, pFile, (UINT8 *)pDest)) != FILEIO_STATUS_OK)
				return status;
			//pFile->bytes_in_buffer = FilesysInfo.bytes_per_sector;
		}
//	}
*/	return FILEIO_STATUS_OK;
}

