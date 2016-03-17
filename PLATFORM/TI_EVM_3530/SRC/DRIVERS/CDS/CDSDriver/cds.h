#ifndef __CDS_H
#define __CDS_H


#include <windows.h>
#include <omap35xx.h>
#include "MicUserSdk.h"

#define CDS_REG_KEY				L"\\Drivers\\BuiltIn\\CDS"
#define CDS_STATS_BREACH_FLAG   1
#define CDS_STATS_MAX_TJ_FLAG	2
#define CDS_BOOT_PARAM_SIZE     8
#define CDS_BIT_FEATURE_EN      1 //bit 0 indicates feature enabling

//From Technical Reference of D/AM37x
//                      DM3730 DM3725 AM3715 AM3703
// ARM/IVA 800/600MHz   0x0C00 0x4C00 0x1C00 0x5C00
// ARM/IVA 1000/800 MHz 0x0E00 0x4E00 0x1E00 0x5E00
#define OMAP_CHIP_ID_SPEED_MASK     0xF00
#define OMAP_CHIP_ID_800MHZ         0xC00
#define OMAP_CHIP_ID_1000MHZ        0xE00


extern "C" {
	DWORD ISRHandler( DWORD InstanceIndex );
	DWORD CreateInstance( void );
	void DestroyInstance( DWORD InstanceIndex );
	BOOL  IOControl( DWORD InstanceIndex, DWORD IoControlCode, LPVOID pInBuf, DWORD InBufSize, LPVOID pOutBuf, DWORD OutBufSize, LPDWORD pBytesReturned );
	DWORD CDS_Init(LPCTSTR szContext, LPCVOID pBusContext);
	BOOL  CDS_Deinit(DWORD context);
	DWORD CDS_Open(DWORD pHead, DWORD AccessCode, DWORD ShareMode);
	BOOL  CDS_Close(DWORD context);
	BOOL  CDS_IOControl(DWORD context, DWORD code, UCHAR *pInBuffer, DWORD inSize, UCHAR *pOutBuffer,DWORD outSize, DWORD *pOutSize);
	BOOL WINAPI DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);
};

typedef struct __CDS_OPEN_INFO CDS_OPEN_INFO, *PCDS_OPEN_INFO;

//------------------------------------------------------------------------------
//  Local Structures

typedef struct __CDS_STATISTICS{
	INT16  MaximalADCValue;
	DWORD  NumTjBreaches;
	DWORD  UpdateStats; //Flags for CDS to update statistics
}CDS_STATISTICS, *PCDS_STATISTICS;

typedef struct __CDS_HW_INFO{
    BOOL             intrThreadExit;
	DWORD            priority256;
	DWORD            tShutSysIntr;
	TCHAR*           queueNameTpModule;
	UINT32           queueNameLenTpModule;
	HKEY		     hRegKey;
	HANDLE           hTwl;
	HANDLE           hGpio;
	HANDLE			 hTshutEvent;
	HANDLE           hTshutThread;
	HANDLE           hTpModule;
	HANDLE           hTpMsgQueue;
	HANDLE           hTpThread;
	HANDLE           hInitDoneEvent;  //Used by users of CDS to insure initialization of CDS is completed
	TEMPSENSOR_DATA  CcTempSensMsg;
	PCDS_OPEN_INFO   pAccessOwner;
	CDS_STATISTICS   CdsStats;
} CDS_HW_INFO, *PCDS_HW_INFO;

typedef struct __CDS_OPEN_INFO{
    PCDS_HW_INFO  pCdsHead;    // @field Pointer back to our CDS_HW_INFO
    DWORD         AccessCode;  // @field What permissions was this opened with
    DWORD         ShareMode;   // @field What Share Mode was this opened with
} CDS_OPEN_INFO, *PCDS_OPEN_INFO;

//Required by CDS ISR
typedef struct __CDS_GPT_INFO{
	OMAP_INTC_MPU_REGS *       pICL;       //Pointer to MPU registers for interrupt mask. Interrupt handler
	OMAP_GPTIMER_REGS*         pGpt;
	OMAP_PRCM_MPU_CM_REGS*     pPrcmMpuCM; //MPU clock configuration
    OMAP_PRCM_IVA2_CM_REGS*    pPrcmIvaCM; //MPU clock corresponding voltage configuration
    OMAP_PRCM_GLOBAL_PRM_REGS* pPrcmGblPRM;
	OMAP_PRCM_IVA2_PRM_REGS*   pPrcmIva2PRM;
    OMAP_SYSC_GENERAL_REGS*    pSysGenReg; //Temperature register Sensor is inside here

	BOOL   BlockOneGhz;
	DWORD  SysIntr;           //System Interrupt number
	DWORD  AtoDReadIntCon;    //CDSISr context read temperature
	DWORD  MaximalAdcWRead;   //Maximal value (above MJT) read from BGAPTS ADC
	UINT   TimerIrQNum;       //IRQ Number for interrput enabling.
	UINT   EntryNumber;       //Keep track of how many times is the GPTimer interrupt is called
	UINT   CurrentOpm;        //Current operation mode of MPU 2=600MHz, 3=800MHz, 4=1000MHz
	INT16  FeatStat;          //Feature status in bit 0. 0-off 1-on.
	INT16  CCTempRead;        //Companion Chip temperature
	INT16  MJT;               //Maximal Junction Temperature
	INT16  IT;                //Intermediate Temperature
	INT16  LTT;               //Lowest Threshold Temperature
	INT16  MaxSpeed;          //Maximal allowed MPU speed
	INT16  CurrSpeed;         //Current MPU speed. Initialy set by boot.
	UINT32 AtoDRead;          //CDS IOCTL Read temperature
	HANDLE hTimer;            //Event set when interrupt routine returns system einterrupt value indicating CDS has some job to do.
	HANDLE hThreadTimer;      //Thread waiting for event from timer. Update OPM.
	PCDS_STATISTICS  pCdsStat;
    CRITICAL_SECTION cs;
}CDS_GPT_INFO, *PCDS_GPT_INFO;

#define USER_IOCTL(X)                         (IOCTL_KLIB_USER + (X))
#define IOCTL_CDS_ISR_CONFIG                   USER_IOCTL(10)
#define IOCTL_CDS_ISR_GET_STATUS               USER_IOCTL(11)
#define IOCTL_CDS_ISR_SET_OPM                  USER_IOCTL(12)


#endif __CDS_H
