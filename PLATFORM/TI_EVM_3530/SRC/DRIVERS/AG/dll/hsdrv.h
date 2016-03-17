#ifndef _HSDRV_H_
#define _HSDRV_H_


/************** HEADER FILE INCLUDES ******************************************/
#include <windows.h>

/************** GLOBAL CONSTANTS **********************************************/
#define     MAX_OPEN_HANDLERS                      32 
#define     SHARED_MEM_SCRATCH_PAD_SIZE            (32*4)
#define 	AGHS_QUEUE_NAME_LEN                    64

/************** STRUCTURES, ENUMS, AND TYPEDEFS *******************************/
typedef struct 
{
	BOOL                bRes; 
    DWORD               contextNum; //Optional...
	HANDLE              hMap;
	void                *pSharedMem;
    DWORD               processId; // current Process Id  
	DWORD               hsEventMask;
	HANDLE              hsQueue;
	TCHAR               hsQueueName[AGHS_QUEUE_NAME_LEN/2]; 
	
} BT_DEVICE_CONTEXT_AG_HS;


/************** FUNCTION PROTOTYPES *******************************************/
#ifdef __cplusplus
	extern "C" {
#endif

 BOOL                     BthAGSetRegEvent(BT_DEVICE_CONTEXT_AG_HS btContextIn, DWORD eventMask);
 BOOL                     BthAGGetRegEvent(BT_DEVICE_CONTEXT_AG_HS btContextIn, DWORD *pEventMask);
 BT_DEVICE_CONTEXT_AG_HS  *BthAGOpenDevice( void );
 INT32                    BthAGCloseDevice( HANDLE *pContext );
 BOOL                     BthAGWriteSharedMem(void *pSharedMem, DWORD offset, void *pData, DWORD length);
 BOOL                     BthAGReadSharedMem(void *pSharedMem, DWORD offset, void *pData, DWORD length);
 BOOL                     BthAGSendEvent(DWORD dwEvent, DWORD dwParam);

#ifdef __cplusplus
		}
#endif	



#endif //_HSDRV_H_