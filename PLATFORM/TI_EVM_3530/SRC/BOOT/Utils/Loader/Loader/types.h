#ifndef __TYPES_H
#define __TYPES_H

#pragma pack(1)

typedef struct _PARTENTRY {
        BYTE            Part_BootInd;           // If 80h means this is boot partition
        BYTE            Part_FirstHead;         // Partition starting head based 0
        BYTE            Part_FirstSector;       // Partition starting sector based 1
        BYTE            Part_FirstTrack;        // Partition starting track based 0
        BYTE            Part_FileSystem;        // Partition type signature field
        BYTE            Part_LastHead;          // Partition ending head based 0
        BYTE            Part_LastSector;        // Partition ending sector based 1
        BYTE            Part_LastTrack;         // Partition ending track based 0
        DWORD           Part_StartSector;       // Logical starting sector based 0
        DWORD           Part_TotalSectors;      // Total logical sectors in partition
} PARTENTRY;
typedef PARTENTRY UNALIGNED *PPARTENTRY;

#pragma pack()


#endif //__TYPES_H