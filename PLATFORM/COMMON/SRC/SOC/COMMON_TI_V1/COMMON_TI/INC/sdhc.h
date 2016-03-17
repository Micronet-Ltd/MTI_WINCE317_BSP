// All rights reserved ADENEO EMBEDDED 2010
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File: SDHC.H
//
//  SDHC driver definitions
//

#ifndef _SDHC_DEFINED
#define _SDHC_DEFINED

#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <devload.h>
#include "omap_types.h"
#include "omap_clocks.h"
#include "oal_clock.h"


#pragma warning(push)
#pragma warning(disable: 4127)
#pragma warning(disable: 4201)
#include <SDHCD.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4127)
#pragma warning(disable: 4100)
#pragma warning(disable: 4189)
#pragma warning(disable: 4201)
#pragma warning(disable: 6001)
#include <creg.hxx>
#pragma warning(pop)


#include "omap.h"
#include "omap_sdhc_regs.h"
#include "omap_sdma_utility.h"
#include "bsp_cfg.h"
#include "omap_sdma_regs.h"
#include "soc_cfg.h"
#include "omap_prcm_regs.h"

//#define ENABLE_RETAIL_OUTPUT

#define DEFAULT_PBIAS_VALUE  0x00000003

#define SDIO_DMA_ENABLED         // comment out this line to disable DMA support

#ifdef SDIO_DMA_ENABLED
#define SDIO_DMA_READ_ENABLED
#define SDIO_DMA_WRITE_ENABLED
#endif
#define CB_DMA_BUFFER 0x20000       // 128KB buffer

#define MMC_BLOCK_SIZE     0x200
#define MIN_MMC_BLOCK_SIZE 4
#define SD_IO_BUS_CONTROL_BUS_WIDTH_MASK 0x03

#ifndef SHIP_BUILD
#define STR_MODULE _T("SDHC!")
#define SETFNAME(name) LPCTSTR pszFname = STR_MODULE name _T(":")
#else
#define SETFNAME(name)
#endif
#define SD_REMOVE_TIMEOUT 200

#define EXT_MMCHS_STAT_CD_INSERT_INTR   0x80000000
#define EXT_MMCHS_STAT_CD_REMOVE_INTR   0x40000000
#define EXT_MMCHS_STAT_CD_INTR          (EXT_MMCHS_STAT_CD_INSERT_INTR  | EXT_MMCHS_STAT_CD_REMOVE_INTR)

#define SD_TIMEOUT 0
#define SD_INSERT 1
#define SD_REMOVE 2


typedef enum {
    CARD_REMOVED_STATE      = 0,
    COMMAND_TRANSFER_STATE,
    DATA_RECEIVE_STATE,
    DATA_TRANSMIT_STATE,
    CARDBUSY_STATE
} SDHCCONTROLLERIST_STATE, *PSDHCCONTROLLERIST_STATE;

typedef enum {
    SDHC_INTR_DISABLED      = 0,
    SDHC_MMC_INTR_ENABLED,
    SDHC_SDIO_INTR_ENABLED
} SDHCINTRENABLE, *PSDHCINTRENABLE;

typedef struct {
    DWORD               dwClockRate;
    SDHCINTRENABLE      eSDHCIntr;
    SD_INTERFACE_MODE   eInterfaceMode;
} SDHC_CONTROLLER_CONTEXT, *PSDHC_CONTROLLER_CONTEXT;

// SDHC hardware specific context
class CSDIOControllerBase
{
public:

    CSDIOControllerBase();
    ~CSDIOControllerBase();

    BOOL Init( LPCTSTR pszActiveKey );
    VOID FreeHostContext( BOOL fRegisteredWithBusDriver, BOOL fHardwareInitialized );
    VOID PowerDown();
    VOID PowerUp();
    // callback handlers
    static SD_API_STATUS SDHCDeinitialize(PSDCARD_HC_CONTEXT pHCContext);
    static SD_API_STATUS SDHCInitialize(PSDCARD_HC_CONTEXT pHCContext);
    static BOOLEAN SDHCCancelIoHandler( PSDCARD_HC_CONTEXT pHCContext,
                                        DWORD Slot,
                                        PSD_BUS_REQUEST pRequest );
    static SD_API_STATUS SDHCBusRequestHandler( PSDCARD_HC_CONTEXT pHCContext,
                                                DWORD Slot,
                                                PSD_BUS_REQUEST pRequest );
    static SD_API_STATUS SDHCSlotOptionHandler( PSDCARD_HC_CONTEXT pHCContext,
                                                DWORD SlotNumber,
                                                SD_SLOT_OPTION_CODE Option,
                                                PVOID pData,
                                                ULONG OptionSize );

    // SD driver callbacks implementation
    SD_API_STATUS SDHCInitializeImpl();
    SD_API_STATUS SDHCDeinitializeImpl();
    BOOLEAN SDHCCancelIoHandlerImpl( UCHAR Slot, PSD_BUS_REQUEST pRequest );
    SD_API_STATUS SDHCBusRequestHandlerImpl( PSD_BUS_REQUEST pRequest );
    SD_API_STATUS SDHCSlotOptionHandlerImpl( UCHAR SlotNumber,
                                             SD_SLOT_OPTION_CODE Option,
                                             PVOID pData,
                                             ULONG OptionSize );


    // platform specific functions
    virtual BOOL    InitializeHardware() = 0;
    virtual void    DeinitializeHardware() = 0;
    virtual BOOL    IsWriteProtected() = 0;
    virtual BOOL    SDCardDetect() = 0;
    virtual DWORD   SDHCCardDetectIstThreadImpl() = 0;
    virtual VOID    TurnCardPowerOn() = 0;
    virtual VOID    TurnCardPowerOff() = 0;
    virtual VOID    PreparePowerChange(CEDEVICE_POWER_STATE curPowerState, BOOL bInPowerHandler) = 0;
    virtual VOID    PostPowerChange(CEDEVICE_POWER_STATE curPowerState, BOOL bInPowerHandler) = 0;
    SD_API_STATUS   CSDIOControllerBase::SDHCBusRequestHandlerImpl_FastPath(PSD_BUS_REQUEST pRequest);
    SD_API_STATUS   CSDIOControllerBase::SDHCBusRequestHandlerImpl_NormalPath(PSD_BUS_REQUEST pRequest);

    // helper functions
    virtual BOOL    InterpretCapabilities();
    VOID    SetInterface(PSD_CARD_INTERFACE_EX pInterface);
    VOID    EnableSDHCInterrupts();
    VOID    DisableSDHCInterrupts();
    VOID    EnableSDIOInterrupts();
    VOID    AckSDIOInterrupt();
    VOID    DisableSDIOInterrupts();
    SD_API_STATUS SendCmdNoResp (DWORD cmd, DWORD arg);
    VOID    SendInitSequence();
    SD_API_STATUS SendCommand(PSD_BUS_REQUEST pRequest);
    SD_API_STATUS GetCommandResponse(PSD_BUS_REQUEST pRequest);
    BOOL    CommandCompleteHandler_FastPath(PSD_BUS_REQUEST pRequest);
    BOOL    SDIReceive(PBYTE pBuff, DWORD dwLen, BOOL FastPathMode);
    BOOL    SDITransmit(PBYTE pBuff, DWORD dwLen, BOOL FastPathMode);
#ifdef SDIO_DMA_ENABLED
    BOOL    SDIDMAReceive(PBYTE pBuff, DWORD dwLen, BOOL FastPathMode);
    BOOL    SDIDMATransmit(PBYTE pBuff, DWORD dwLen, BOOL FastPathMode);
#endif
    BOOL    SDIPollingReceive(PBYTE pBuff, DWORD dwLen);
    BOOL    SDIPollingTransmit(PBYTE pBuff, DWORD dwLen);
    VOID    SetSlotPowerState( CEDEVICE_POWER_STATE state );
    CEDEVICE_POWER_STATE GetSlotPowerState();

    VOID SoftwareReset(DWORD dwResetBits);

    // Interrupt handling methods
    VOID HandleRemoval(BOOL fCancelRequest);
    VOID HandleInsertion();

    VOID CardInterrupt(BOOL bInsert);
    BOOL HandleCardDetectInterrupt(DWORD dwStatus);

    VOID SetSDVSVoltage();
    VOID SetClockRate(PDWORD pdwRate);
    BOOL UpdateSystemClock( BOOL enable );
    VOID SystemClockOn(BOOL bInPowerHandler) {
        m_InternPowerState = D0;
        UpdateDevicePowerState(bInPowerHandler);
    }
    VOID SystemClockOff(BOOL bInPowerHandler) {
        m_InternPowerState = D4;
        UpdateDevicePowerState(bInPowerHandler);
    }
    VOID UpdateDevicePowerState(BOOL bInPowerHandler);
    BOOL SetPower(CEDEVICE_POWER_STATE dx);
    BOOL ContextRestore();

    inline DWORD Read_MMC_STAT();
    inline void Write_MMC_STAT( DWORD wVal );
    inline void Set_MMC_STAT( DWORD wVal );

    // IST functions
    static DWORD WINAPI SDHCControllerIstThread(LPVOID lpParameter);
    static DWORD WINAPI SDHCCardDetectIstThread(LPVOID lpParameter);
    static DWORD WINAPI DataTransferIstThread(LPVOID lpParameter);
    DWORD SDHCControllerIstThreadImpl();
    DWORD SDHCPowerTimerThreadImpl();
    static DWORD WINAPI SDHCPowerTimerThread(LPVOID lpParameter);

    static DWORD WINAPI SDHCPrintThread(LPVOID lpParameter);
    DWORD SDHCPrintThreadImpl();
    HANDLE               m_hPrintEvent;         // SDIO/controller print event
    HANDLE               m_hPrintIST;             // SDIO/controller print thread
    const static DWORD        m_cmdArrSize=32;
    DWORD              m_cmdArray[m_cmdArrSize];
    DWORD              m_cmdRdIndex;
    DWORD              m_cmdWrIndex;

#ifdef SDIO_DMA_ENABLED
    DmaDataInfo_t *      m_TxDmaInfo;
    HANDLE               m_hTxDmaChannel;       // TX DMA channel allocated by the DMA lib
    DmaDataInfo_t *      m_RxDmaInfo;
    HANDLE               m_hRxDmaChannel;       // RX DMA channel allocated by the DMA lib

    // DMA functions
    void SDIO_InitDMA(void);
    void SDIO_DeinitDMA(void);
    void SDIO_InitInputDMA(DWORD dwBlkCnt, DWORD dwBlkSize);
    void SDIO_InitOutputDMA(DWORD dwBlkCnt, DWORD dwBlkSize);
    void SDIO_StartInputDMA();
    void SDIO_StartOutputDMA();
    void SDIO_StopInputDMA();
    void SDIO_StopOutputDMA();
    void DumpDMARegs(int inCh);
#endif

    SD_API_STATUS CommandTransferCompleteHandler(PSD_BUS_REQUEST pRequest, DWORD dwIntrStatus, PSDHCCONTROLLERIST_STATE pNextState);
    SD_API_STATUS CardBusyCompletedHandler(PSD_BUS_REQUEST pRequest, DWORD dwIntrStatus, PSDHCCONTROLLERIST_STATE pNextState);

    SD_API_STATUS ReceiveHandler(PSD_BUS_REQUEST pRequest, PSDHCCONTROLLERIST_STATE peNextState);
    SD_API_STATUS TransmitHandler(PSD_BUS_REQUEST pRequest, PSDHCCONTROLLERIST_STATE peNextState);

#ifdef SDIO_DMA_ENABLED
    SD_API_STATUS DataReceiveCompletedHandler(PSD_BUS_REQUEST pRequest, DWORD dwIntrStatus, PSDHCCONTROLLERIST_STATE pNextState);
    SD_API_STATUS DataTransmitCompletedHandler(PSD_BUS_REQUEST pRequest, DWORD dwIntrStatus, PSDHCCONTROLLERIST_STATE pNextState);
    BOOL          StartDMATransmit(PBYTE pBuff, DWORD dwLen);
    BOOL          StartDMAReceive(PBYTE pBuff, DWORD dwLen);
#endif

    SD_API_STATUS ProcessCommandTransferStatus(PSD_BUS_REQUEST pRequest, SD_API_STATUS status, DWORD dwStatusOverwrite);
    SD_API_STATUS CheckIntrStatus(DWORD dwIntrStatus, DWORD *pOverwrite);

#ifdef DEBUG
    VOID DumpRegisters();
#else
    VOID DumpRegisters() {}
#endif

protected:
    VOID SetSDInterfaceMode(SD_INTERFACE_MODE eSDInterfaceMode);
    BOOL IsMultipleBlockReadSupported();
public:
    PSDCARD_HC_CONTEXT   m_pHCContext;                   // the host controller context
    HANDLE               m_hParentBus;                   // bus parent handle

    CRITICAL_SECTION     m_critSec;                      // used to synchronize access to SDIO controller registers
    CRITICAL_SECTION     m_powerCS;                      // used to synchronize access to SDIO controller registers
    BOOL                 m_fSDIOInterruptInService;      // TRUE - an SDIO interrupt has been detected and has
                                                        // not yet been acknowledge by the client
    CEDEVICE_POWER_STATE m_InternPowerState;            // current internal power state.
    CEDEVICE_POWER_STATE m_ActualPowerState;            // actual power state.
    BOOL                 m_bReinsertTheCard;            // force card insertion event
    DWORD                m_dwWakeupSources;             // possible wakeup sources (1 - SDIO, 2 - card insert/removal)
    DWORD                m_dwCurrentWakeupSources;      // current wakeup sources

    BOOL                 m_fCardPresent;                // a card is inserted and initialized
    BOOL                 m_fSDIOInterruptsEnabled;      // TRUE - indicates that SDIO interrupts are enabled

    BOOL                 m_fMMCMode;                    // if true, the controller assumed that the card inserted is MMC
    PBYTE                m_pDmaBuffer;                  // virtual address of DMA buffer
    PBYTE                m_pCachedDmaBuffer;            // virtual address of DMA buffer
    PHYSICAL_ADDRESS     m_pDmaBufferPhys;              // physical address of DMA buffer
    BOOL                 m_fDMATransfer;                // TRUE - the current request will use DMA for data transfer

    BOOL                 m_fAppCmdMode;                 // if true, the controller is in App Cmd mode
    HANDLE               m_hControllerISTEvent;         // SDIO/controller interrupt event
    HANDLE               m_htControllerIST;             // SDIO/controller interrupt thread
    HANDLE               m_hCardDetectEvent;            // card detect interrupt event
    HANDLE               m_htCardDetectIST;             // card detect interrupt thread

    DWORD                m_dwSDIOPriority;              // SDIO IST priority
    DWORD                m_dwCDPriority;                // CardDetect IST priority
    BOOL                 m_fDriverShutdown;             // controller shutdown
    INT                  m_dwControllerSysIntr;         // controller interrupt
    BOOL                 m_fInitialized;                // driver initialized
    BOOL                 m_fCardInitialized;            // Card Initialized
    DWORD                m_wCTOTimeout;                 // command time-out
    DWORD                m_wDTOTimeout;                 // data read time-out
    DWORD                m_dwMaxClockRate;              // host controller's clock base
    USHORT               m_usMaxBlockLen;               // max block length

    DWORD                m_dwMaxTimeout;                // timeout (in miliseconds) for read/write operations
    BOOL                 m_fFirstTime;                  // set to TRUE after a card is inserted to
                                                        // indicate that 80 clock cycles initialization
                                                        // must be done when the first command is issued
    BOOL                 m_fPowerDownChangedPower;      // did _PowerDown change the power state?

    OMAP_MMCHS_REGS *m_pbRegisters;                 // SDHC controller registers
    DWORD                m_LastCommand;

    HANDLE               m_hGPIO;                       // GPIO driver handle

    DWORD                m_dwMemBase;
    DWORD                m_dwMemLen;
    DWORD                m_dwSlot;
	DWORD				 m_dwDeviceID;
    DWORD                m_dwSDIOCard;
    DWORD                m_dwCPURev;
    DWORD                m_fastPathSDIO;
    DWORD                m_fastPathSDMEM;
    DWORD                m_fastPathReq;
    DWORD                m_LowVoltageSlot;
    DWORD                m_Sdio4BitDisable;
    DWORD                m_SdMem4BitDisable;
    DWORD                m_dwSDHighSpeedSupport;
    DWORD                m_dwSDClockMode;

    LONG                m_dwClockCnt;
    BOOL                m_bExitThread;
    HANDLE            m_hTimerEvent;
    HANDLE             m_hTimerThreadIST;
    DWORD              m_nNonSDIOActivityTimeout;
    DWORD              m_nSDIOActivityTimeout;
    CRITICAL_SECTION     m_pwrThrdCS;                 // used to synchronize access to m_dwClockCnt
    BOOL                m_bDisablePower;

    BOOL                 bRxDmaActive;
    BOOL                 bTxDmaActive;
    SD_TRANSFER_CLASS    m_TransferClass;

    DWORD                m_dwCardInsertedGPIOState;
    DWORD                m_dwCardWriteProtectedState;
    DWORD                m_dwCardDetectGPIO;
    DWORD                m_dwCardWPGPIO;

    VOID*                m_pCurrentRecieveBuffer;
    DWORD                m_dwCurrentRecieveBufferLength;

    DWORD                m_CardDetectInterruptStatus;
    BOOL                 m_bCommandPending;
    DWORD                m_dwDMABufferSize;
    SDHC_CONTROLLER_CONTEXT m_sContext;
};

typedef CSDIOControllerBase *PCSDIOControllerBase;

#define GET_PCONTROLLER_FROM_HCD(pHCDContext) \
    GetExtensionFromHCDContext(PCSDIOControllerBase, pHCDContext)

CSDIOControllerBase *CreateSDIOController();

#define SHC_INTERRUPT_ZONE              SDCARD_ZONE_0
#define SHC_SEND_ZONE                   SDCARD_ZONE_1
#define SHC_RESPONSE_ZONE               SDCARD_ZONE_2
#define SHC_RECEIVE_ZONE                SDCARD_ZONE_3
#define SHC_CLOCK_ZONE                  SDCARD_ZONE_4
#define SHC_TRANSMIT_ZONE               SDCARD_ZONE_5
#define SHC_SDBUS_INTERACT_ZONE         SDCARD_ZONE_6
#define SHC_BUSY_STATE_ZONE             SDCARD_ZONE_7
#define SHC_DVFS_ZONE                   SDCARD_ZONE_8

#define SHC_INTERRUPT_ZONE_ON           ZONE_ENABLE_0
#define SHC_SEND_ZONE_ON                ZONE_ENABLE_1
#define SHC_RESPONSE_ZONE_ON            ZONE_ENABLE_2
#define SHC_RECEIVE_ZONE_ON             ZONE_ENABLE_3
#define SHC_CLOCK_ZONE_ON               ZONE_ENABLE_4
#define SHC_TRANSMIT_ZONE_ON            ZONE_ENABLE_5
#define SHC_SDBUS_INTERACT_ZONE_ON      ZONE_ENABLE_6
#define SHC_BUSY_STATE_ZONE_ON          ZONE_ENABLE_7
#define SHC_DVFS_ZONE_ON                ZONE_ENABLE_8

#define SHC_CARD_CONTROLLER_PRIORITY    0x97
#define SHC_CARD_DETECT_PRIORITY        0x98

#define WAKEUP_SDIO                     1
#define WAKEUP_CARD_INSERT_REMOVAL      2

#define DMA_TX                          0
#define DMA_RX                          1
#define TIMERTHREAD_TIMEOUT_NONSDIO     1
#define TIMERTHREAD_TIMEOUT             2000
#define TIMERTHREAD_PRIORITY            252


//In Sandisk's TRM, the expected OCR register value returned from iNand should be 0xC0FF8080
//BIT31 indicates the device is ready, BIT30 indicates the High Capacity, 
//0x00FF8080 means the opertaing voltage range are VDD 2.7-3.6V and VDD 1.7-1.95V
#define SANDISK_EMMC_DEFAULT_OCR_VAL 0x40FF8080

#endif // _SDHC_DEFINED


